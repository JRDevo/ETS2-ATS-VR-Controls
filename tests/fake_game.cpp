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
// Exit code 0 = every OpenXR call the game made succeeded.
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <openxr/openxr.h>

static int g_bad = 0;
#define CK(call) do { XrResult r_ = (call); if (XR_FAILED(r_)) { printf("FAILED %s -> %d\n", #call, (int)r_); ++g_bad; } } while (0)

static double Now() {
    static LARGE_INTEGER f = [] { LARGE_INTEGER x; QueryPerformanceFrequency(&x); return x; }();
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart / (double)f.QuadPart;
}

int main(int argc, char** argv) {
    const double seconds = argc > 1 ? atof(argv[1]) : 5.0;
    bool noInput = false, noSync = false, touch = false, noDestroySession = false;
    int syncEvery = 1;
    double stall0 = -1, stall1 = -1;
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "--no-input")) noInput = true;
        if (!strcmp(argv[i], "--no-sync")) noSync = true;
        if (!strcmp(argv[i], "--touch")) touch = true;
        if (!strcmp(argv[i], "--no-destroy-session")) noDestroySession = true;
        if (!strcmp(argv[i], "--sync-every") && i + 1 < argc) syncEvery = atoi(argv[++i]) > 0 ? atoi(argv[i]) : 1;
        if (!strcmp(argv[i], "--stall") && i + 1 < argc) sscanf_s(argv[++i], "%lf-%lf", &stall0, &stall1);
    }
    int frame = 0, inactive = 0;
    bool synced = false;
    setvbuf(stdout, nullptr, _IONBF, 0);
    XrInstanceCreateInfo ic{XR_TYPE_INSTANCE_CREATE_INFO};
    strcpy_s(ic.applicationInfo.applicationName, "fake_game");
    strcpy_s(ic.applicationInfo.engineName, "prism3d-ish");
    ic.applicationInfo.apiVersion = XR_MAKE_VERSION(1, 0, 0);
    XrInstance inst = XR_NULL_HANDLE;
    XrResult r = xrCreateInstance(&ic, &inst);
    if (XR_FAILED(r)) { printf("xrCreateInstance failed %d\n", (int)r); return 2; }
    XrSystemGetInfo sg{XR_TYPE_SYSTEM_GET_INFO};
    sg.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemId sys = 0;
    CK(xrGetSystem(inst, &sg, &sys));

    XrPath hands[2] = {};
    CK(xrStringToPath(inst, "/user/hand/left", &hands[0]));
    CK(xrStringToPath(inst, "/user/hand/right", &hands[1]));
    XrActionSet set = XR_NULL_HANDLE;
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
    XrSessionCreateInfo sci{XR_TYPE_SESSION_CREATE_INFO};
    sci.systemId = sys;
    XrSession ses = XR_NULL_HANDLE;
    CK(xrCreateSession(inst, &sci, &ses));
    if (!noInput) {
        XrSessionActionSetsAttachInfo ai{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
        ai.countActionSets = 1;
        ai.actionSets = &set;
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
    printf("game input inactive reads: %d\n", inactive);
    printf("fake_game done, %d failed call(s)\n", g_bad);
    return g_bad ? 1 : 0;
}
