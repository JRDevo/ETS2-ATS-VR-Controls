// fake_game -- an OpenXR "game" for the end-to-end test (tests/run_e2e.ps1), through the real OpenXR loader. By default it does
// what ATS / ETS2 do: one action set "input" (priority 0) with a boolean "select" and "menu" action for both hands, suggested
// ONLY for khr/simple_controller, attached at session start, synced every frame. It prints "game select <hand> down/up at <t>"
// when its own select action changes, so the test can check that the game keeps its buttons next to the layer's.
// After its first sync it READS its input every frame (synced this frame or not) and counts reads where its action was inactive
// ("game input inactive reads: N" at the end): a sync by someone else that left its set out shows up there.
//   fake_game <seconds> [--no-input] [--no-sync] [--touch] [--sync-every N] [--stall t0-t1] [--no-destroy-session]
//     --no-input    no action set at all (the layer attaches its own after VRC_TEST_SELF_ATTACH_S)
//     --no-sync     attaches but never calls xrSyncActions (the layer syncs its own set)
//     --touch       also suggests oculus/touch_controller itself (the layer merges into it, no translation)
//     --sync-every  syncs only every Nth frame
//     --stall       no sync from t0 to t1 seconds (a loading screen / hitch)
//     --no-destroy-session  ends with xrDestroyInstance only (no xrEndSession / xrDestroySession)
//     --d3d11       a D3D11 session (WARP device, XR_KHR_D3D11_enable) like ETS2 / ATS: the VR menu can draw
//     --eyecapture  also what VR EyeCapture's layer (above ours) adds: action set "eyecapture_vr_hotkey" with both stick clicks,
//                   suggested on Touch, attached with the game's set
//     --scs <dll>   what ETS2 / ATS's plugin loader does first: loads <dll> (the layer's file) and calls its scs_input_init with
//                   a register_device of ours; each frame asks the device for its events (first_in_frame, first_after_activation
//                   at the first) and prints "scs <input> <value> at <t>" when a value changed ("scs events per frame:
//                   min .. max" at the end); "scs same module" = the OpenXR loader got the same module. It also calls
//                   scs_telemetry_init and feeds "truck.speed" every frame: --scs-speed <km/h> (default 0)
// Exit code 0 = every OpenXR call the game made succeeded.
#include <windows.h>
#include <d3d11.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <openxr/openxr.h>
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr_platform.h>

static int g_bad = 0;
#define CK(call) do { XrResult r_ = (call); if (XR_FAILED(r_)) { printf("FAILED %s -> %d\n", #call, (int)r_); ++g_bad; } } while (0)

// the SCS SDK input API, the game's side (layouts as in scssdk_input*.h, x64)
struct ScsEvent { uint32_t input_index; union { uint8_t value_bool; float value_float; float sizing[6]; }; };
typedef int32_t (__stdcall* ScsEventCb)(ScsEvent*, uint32_t, void*);
struct ScsDeviceInput { const char* name; const char* display_name; uint32_t value_type; uint32_t padding; };
struct ScsDevice { const char* name; const char* display_name; uint32_t type; uint32_t input_count; const ScsDeviceInput* inputs;
                   void* context; void* active_cb; ScsEventCb event_cb; };
struct ScsInit { const char* game_name; const char* game_id; uint32_t game_version; uint32_t padding; void* log;
                 int32_t (__stdcall* register_device)(const ScsDevice*); };
static ScsDevice g_scsDev = {};
static const char* g_scsNames[8] = {};
static int32_t __stdcall ScsRegister(const ScsDevice* d) {
    g_scsDev = *d;
    for (uint32_t i = 0; i < d->input_count && i < 8; ++i) g_scsNames[i] = d->inputs[i].name;
    printf("scs device %s (%s) type %u, %u inputs\n", d->name, d->display_name, d->type, d->input_count);
    return 0;
}

// the telemetry API, the game's side: the channels the plugin asks for; "truck.speed" is fed each frame (--scs-speed)
struct ScsValue { uint32_t type; uint32_t padding; union { float value_float; double sizing[7]; }; };
typedef void (__stdcall* ScsChannelCb)(const char*, uint32_t, const ScsValue*, void*);
struct ScsTelemetryInit { const char* game_name; const char* game_id; uint32_t game_version; uint32_t padding; void* log;
                          void* reg_event; void* unreg_event;
                          int32_t (__stdcall* reg_channel)(const char*, uint32_t, uint32_t, uint32_t, ScsChannelCb, void*);
                          void* unreg_channel; };
static ScsChannelCb g_speedCb = nullptr;
static void* g_speedCtx = nullptr;
static int32_t __stdcall ScsRegChannel(const char* name, uint32_t, uint32_t type, uint32_t, ScsChannelCb cb, void* ctx) {
    printf("scs channel %s type %u\n", name, type);
    if (!strcmp(name, "truck.speed")) { g_speedCb = cb; g_speedCtx = ctx; }
    return 0;
}

static double Now() {
    static LARGE_INTEGER f = [] { LARGE_INTEGER x; QueryPerformanceFrequency(&x); return x; }();
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart / (double)f.QuadPart;
}

int main(int argc, char** argv) {
    const double seconds = argc > 1 ? atof(argv[1]) : 5.0;
    bool noInput = false, noSync = false, touch = false, noDestroySession = false, d3d = false, eyecap = false;
    int syncEvery = 1;
    double stall0 = -1, stall1 = -1;
    const char* scsDll = nullptr;
    float scsSpeed = 0.0f;                               // km/h the telemetry reports (--scs-speed)
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--no-input")) noInput = true;
        if (!strcmp(argv[i], "--no-sync")) noSync = true;
        if (!strcmp(argv[i], "--touch")) touch = true;
        if (!strcmp(argv[i], "--no-destroy-session")) noDestroySession = true;
        if (!strcmp(argv[i], "--d3d11")) d3d = true;
        if (!strcmp(argv[i], "--eyecapture")) eyecap = true;
        if (!strcmp(argv[i], "--sync-every") && i + 1 < argc) syncEvery = atoi(argv[++i]) > 0 ? atoi(argv[i]) : 1;
        if (!strcmp(argv[i], "--stall") && i + 1 < argc) sscanf_s(argv[++i], "%lf-%lf", &stall0, &stall1);
        if (!strcmp(argv[i], "--scs") && i + 1 < argc) scsDll = argv[++i];
        if (!strcmp(argv[i], "--scs-speed") && i + 1 < argc) scsSpeed = (float)atof(argv[++i]);
    }
    setvbuf(stdout, nullptr, _IONBF, 0);
    HMODULE scsMod = nullptr;
    if (scsDll) {                                        // (before OpenXR, like the game: its plugins load at start)
        scsMod = LoadLibraryA(scsDll);
        typedef int32_t (__stdcall* Init)(uint32_t, const void*);
        Init init = scsMod ? reinterpret_cast<Init>(GetProcAddress(scsMod, "scs_input_init")) : nullptr;
        ScsInit p = { "Fake Truck Simulator", "fake", (1u << 16), 0, nullptr, ScsRegister };
        const int32_t r = init ? init((1u << 16), &p) : -99;
        printf("scs init %s -> %d\n", scsDll, (int)r);
        typedef int32_t (__stdcall* TInit)(uint32_t, const void*);
        TInit tinit = scsMod ? reinterpret_cast<TInit>(GetProcAddress(scsMod, "scs_telemetry_init")) : nullptr;
        ScsTelemetryInit tp = { "Fake Truck Simulator", "fake", (1u << 16), 0, nullptr, nullptr, nullptr, ScsRegChannel, nullptr };
        const int32_t tr = tinit ? tinit((1u << 16) | 1u, &tp) : -99;
        printf("scs telemetry init -> %d\n", (int)tr);
    }
    bool scsFirst = true;
    float scsLast[8] = { -9, -9, -9, -9, -9, -9, -9, -9 };   // the last value printed per input
    int scsMin = 99, scsMax = -1;                            // events per frame
    int frame = 0, inactive = 0;
    bool synced = false;
    XrInstanceCreateInfo ic{XR_TYPE_INSTANCE_CREATE_INFO};
    strcpy_s(ic.applicationInfo.applicationName, "fake_game");
    strcpy_s(ic.applicationInfo.engineName, "prism3d-ish");
    ic.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0);
    const char* ext[1] = { XR_KHR_D3D11_ENABLE_EXTENSION_NAME };
    if (d3d) { ic.enabledExtensionCount = 1; ic.enabledExtensionNames = ext; }
    XrInstance inst = XR_NULL_HANDLE;
    XrResult r = xrCreateInstance(&ic, &inst);
    if (XR_FAILED(r)) { printf("xrCreateInstance failed %d\n", (int)r); return 2; }
    if (scsMod) printf("scs same module: %s\n", GetModuleHandleA("ets2_ats_vr_controls.dll") == scsMod ? "yes" : "no");
    XrSystemGetInfo sg{XR_TYPE_SYSTEM_GET_INFO};
    sg.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemId sys = 0;
    CK(xrGetSystem(inst, &sg, &sys));

    XrPath hands[2] = {};
    CK(xrStringToPath(inst, "/user/hand/left", &hands[0]));
    CK(xrStringToPath(inst, "/user/hand/right", &hands[1]));
    XrActionSet set = XR_NULL_HANDLE, ecSet = XR_NULL_HANDLE;
    XrAction select = XR_NULL_HANDLE, menu = XR_NULL_HANDLE;
    if (!noInput) {
        XrActionSetCreateInfo sc{XR_TYPE_ACTION_SET_CREATE_INFO};
        strcpy_s(sc.actionSetName, "input");
        strcpy_s(sc.localizedActionSetName, "Input");
        CK(xrCreateActionSet(inst, &sc, &set));
        XrActionCreateInfo ac{XR_TYPE_ACTION_CREATE_INFO};
        ac.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        ac.countSubactionPaths = 2;
        ac.subactionPaths = hands;
        strcpy_s(ac.actionName, "select");
        strcpy_s(ac.localizedActionName, "Select");
        CK(xrCreateAction(set, &ac, &select));
        strcpy_s(ac.actionName, "menu");
        strcpy_s(ac.localizedActionName, "Menu");
        CK(xrCreateAction(set, &ac, &menu));
        auto path = [&](const char* s) { XrPath p = 0; CK(xrStringToPath(inst, s, &p)); return p; };
        XrActionSuggestedBinding sb[4] = { {select, path("/user/hand/left/input/select/click")}, {select, path("/user/hand/right/input/select/click")},
                                           {menu, path("/user/hand/left/input/menu/click")}, {menu, path("/user/hand/right/input/menu/click")} };
        XrInteractionProfileSuggestedBinding ip{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
        ip.interactionProfile = path("/interaction_profiles/khr/simple_controller");
        ip.countSuggestedBindings = 4;
        ip.suggestedBindings = sb;
        CK(xrSuggestInteractionProfileBindings(inst, &ip));
        if (touch) {
            XrActionSuggestedBinding tb[3] = { {select, path("/user/hand/left/input/trigger/value")}, {select, path("/user/hand/right/input/trigger/value")},
                                               {menu, path("/user/hand/left/input/menu/click")} };
            ip.interactionProfile = path("/interaction_profiles/oculus/touch_controller");
            ip.countSuggestedBindings = 3;
            ip.suggestedBindings = tb;
            CK(xrSuggestInteractionProfileBindings(inst, &ip));
        }
    }
    if (eyecap) {                                          // VR EyeCapture's layer (it runs above ours, so we see it like the game)
        XrActionSetCreateInfo sc{XR_TYPE_ACTION_SET_CREATE_INFO};
        strcpy_s(sc.actionSetName, "eyecapture_vr_hotkey");
        strcpy_s(sc.localizedActionSetName, "EyeCapture");
        CK(xrCreateActionSet(inst, &sc, &ecSet));
        XrAction b0 = XR_NULL_HANDLE, b1 = XR_NULL_HANDLE;
        XrActionCreateInfo ac{XR_TYPE_ACTION_CREATE_INFO};
        ac.actionType = XR_ACTION_TYPE_BOOLEAN_INPUT;
        strcpy_s(ac.actionName, "b0");
        strcpy_s(ac.localizedActionName, "b0");
        CK(xrCreateAction(ecSet, &ac, &b0));
        strcpy_s(ac.actionName, "b1");
        strcpy_s(ac.localizedActionName, "b1");
        CK(xrCreateAction(ecSet, &ac, &b1));
        auto path = [&](const char* s) { XrPath p = 0; CK(xrStringToPath(inst, s, &p)); return p; };
        // like the real one: its own two clicks + the game's simple_controller bindings translated (select -> trigger, menu)
        XrActionSuggestedBinding eb[5] = { {b0, path("/user/hand/left/input/thumbstick/click")}, {b1, path("/user/hand/right/input/thumbstick/click")},
                                           {select, path("/user/hand/left/input/trigger/value")}, {select, path("/user/hand/right/input/trigger/value")},
                                           {menu, path("/user/hand/left/input/menu/click")} };
        XrInteractionProfileSuggestedBinding ip{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
        ip.interactionProfile = path("/interaction_profiles/oculus/touch_controller");
        ip.countSuggestedBindings = noInput ? 2 : 5;
        ip.suggestedBindings = eb;
        CK(xrSuggestInteractionProfileBindings(inst, &ip));
    }
    XrSessionCreateInfo sci{XR_TYPE_SESSION_CREATE_INFO};
    sci.systemId = sys;
    ID3D11Device* dev = nullptr;
    XrGraphicsBindingD3D11KHR gb{XR_TYPE_GRAPHICS_BINDING_D3D11_KHR};
    if (d3d) {
        if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &dev, nullptr, nullptr))) {
            printf("D3D11CreateDevice (WARP) failed\n");
            return 2;
        }
        gb.device = dev;
        sci.next = &gb;
    }
    XrSession ses = XR_NULL_HANDLE;
    CK(xrCreateSession(inst, &sci, &ses));
    if (!noInput) {
        XrSessionActionSetsAttachInfo ai{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
        const XrActionSet both[2] = { set, ecSet };
        ai.countActionSets = eyecap ? 2 : 1;
        ai.actionSets = both;
        CK(xrAttachSessionActionSets(ses, &ai));
    }
    const double t0 = Now();
    bool running = false, prev[2] = {};
    while (Now() - t0 < seconds) {
        XrEventDataBuffer ev{XR_TYPE_EVENT_DATA_BUFFER};
        while (xrPollEvent(inst, &ev) == XR_SUCCESS) {
            if (ev.type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
                const auto* sc = reinterpret_cast<XrEventDataSessionStateChanged*>(&ev);
                if (sc->state == XR_SESSION_STATE_READY) {
                    XrSessionBeginInfo bi{XR_TYPE_SESSION_BEGIN_INFO};
                    bi.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                    CK(xrBeginSession(ses, &bi));
                    running = true;
                }
            }
            ev = XrEventDataBuffer{XR_TYPE_EVENT_DATA_BUFFER};
        }
        if (!running) { Sleep(5); continue; }
        XrFrameState fs{XR_TYPE_FRAME_STATE};
        CK(xrWaitFrame(ses, nullptr, &fs));
        CK(xrBeginFrame(ses, nullptr));
        ++frame;
        const double t = Now() - t0;
        const bool stalled = t >= stall0 && t < stall1;
        if (!noInput && !noSync && !stalled && frame % syncEvery == 0) {
            XrActiveActionSet as{set, XR_NULL_PATH};
            XrActionsSyncInfo si{XR_TYPE_ACTIONS_SYNC_INFO};
            si.countActiveActionSets = 1;
            si.activeActionSets = &as;
            const XrResult rs = xrSyncActions(ses, &si);
            if (XR_FAILED(rs)) { printf("FAILED xrSyncActions -> %d\n", (int)rs); ++g_bad; }
            synced = true;
        }
        if (!noInput && !noSync && synced) {               // the game reads its input EVERY frame, synced this frame or not
            for (int h = 0; h < 2; ++h) {
                XrActionStateGetInfo gi{XR_TYPE_ACTION_STATE_GET_INFO};
                gi.action = select;
                gi.subactionPath = hands[h];
                XrActionStateBoolean st{XR_TYPE_ACTION_STATE_BOOLEAN};
                CK(xrGetActionStateBoolean(ses, &gi, &st));
                if (!st.isActive) ++inactive;                   // its set was left out of a sync (by someone else)
                const bool down = st.isActive && st.currentState;
                if (down != prev[h]) printf("game select %s %s at %.2f\n", h ? "right" : "left", down ? "down" : "up", Now() - t0);
                prev[h] = down;
            }
        }
        if (g_speedCb) {                                 // telemetry first (the game's physics step), in m/s
            ScsValue v = {};
            v.type = 5;
            v.value_float = scsSpeed / 3.6f;
            g_speedCb("truck.speed", 0xFFFFFFFFu, &v, g_speedCtx);
        }
        if (g_scsDev.event_cb) {                         // the game's main thread: the device's events of this frame
            uint32_t flags = 1u | (scsFirst ? 2u : 0u);
            scsFirst = false;
            int n = 0;
            for (; n < 16; ++n) {
                ScsEvent e = {};
                if (g_scsDev.event_cb(&e, flags, g_scsDev.context) != 0) break;
                flags = 0;
                if (e.input_index < 8 && e.value_float != scsLast[e.input_index]) {   // (printed only when it changed)
                    scsLast[e.input_index] = e.value_float;
                    printf("scs %s %.3f at %.2f\n", g_scsNames[e.input_index] ? g_scsNames[e.input_index] : "?",
                           (double)e.value_float, Now() - t0);
                }
            }
            scsMin = n < scsMin ? n : scsMin;
            scsMax = n > scsMax ? n : scsMax;
        }
        XrFrameEndInfo fe{XR_TYPE_FRAME_END_INFO};
        fe.displayTime = fs.predictedDisplayTime;
        fe.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        CK(xrEndFrame(ses, &fe));
    }
    if (!noDestroySession) {
        CK(xrEndSession(ses));
        CK(xrDestroySession(ses));
    }
    CK(xrDestroyInstance(inst));
    if (dev) dev->Release();
    if (g_scsDev.event_cb) printf("scs events per frame: %d .. %d\n", scsMin, scsMax);
    printf("game input inactive reads: %d\n", inactive);
    printf("fake_game done, %d failed call(s)\n", g_bad);
    return g_bad ? 1 : 0;
}
