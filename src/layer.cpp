// layer -- the OpenXR API layer XR_APILAYER_JRDEVO_ets2_ats_vr_controls (implicit; the loader loads it into every OpenXR program).
// Only programs named in config.ini 'apps' (default eurotrucks2.exe, amtrucks.exe) are touched: in every other program the layer
// hands out the next layer's functions unchanged and writes no log. Hooked (pad_input.h says why each one):
//   xrDestroyInstance                     forget the instance
//   xrCreateSession / xrDestroySession    per-session state (our spaces are destroyed before the game's session)
//   xrPollEvent                           the session state (input only while FOCUSED)
//   xrWaitFrame                           the predicted display time (aim poses)
//   xrBeginFrame                          the per-frame tick: read the controllers, send keys / mouse
//   xrCreateActionSet / xrCreateAction    the game's input (priorities, which set an action belongs to)
//   xrSuggestInteractionProfileBindings   the game's bindings (merged with ours at its attach)
//   xrAttachSessionActionSets / xrSyncActions   our action set appended to the game's
//   xrEndFrame                            the VR menu's quad layers after the game's (only while it is shown)
//   xrGetActionStateBoolean / Float       the game's trigger reads released while it points at the VR menu
// With log = 1 its own cost is measured (CPU time in our part of xrBeginFrame + xrEndFrame, not the game's or the runtime's)
// and logged once a minute: "performance: ..." -- so anyone can see in the log how light it is. log = 0 (the default): no log
// file at all and nothing measured.
#include <windows.h>
#include <d3d11.h>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <openxr/openxr.h>
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr_platform.h>
#include <openxr/openxr_loader_negotiation.h>
#include "config.h"
#include "log.h"
#include "pad_input.h"

namespace ck {
namespace {

const char* const kLayerName = VRC_LAYER_NAME;

struct InstanceData {
    XrInstance                              handle = XR_NULL_HANDLE;
    PFN_xrGetInstanceProcAddr               nextGipa = nullptr;
    PFN_xrDestroyInstance                   DestroyInstance = nullptr;
    PFN_xrCreateSession                     CreateSession = nullptr;
    PFN_xrDestroySession                    DestroySession = nullptr;
    PFN_xrPollEvent                         PollEvent = nullptr;
    PFN_xrWaitFrame                         WaitFrame = nullptr;
    PFN_xrBeginFrame                        BeginFrame = nullptr;
    PFN_xrCreateActionSet                   CreateActionSet = nullptr;
    PFN_xrCreateAction                      CreateAction = nullptr;
    PFN_xrSuggestInteractionProfileBindings SuggestInteractionProfileBindings = nullptr;
    PFN_xrAttachSessionActionSets           AttachSessionActionSets = nullptr;
    PFN_xrSyncActions                       SyncActions = nullptr;
    PFN_xrEndFrame                          EndFrame = nullptr;
    PFN_xrGetActionStateBoolean             GetActionStateBoolean = nullptr;
    PFN_xrGetActionStateFloat               GetActionStateFloat = nullptr;
    std::unique_ptr<PadInput>               pad;
};

std::once_flag            g_initOnce;
bool                      g_on = false;     // this program is in 'apps' and enabled = 1
Config                    g_cfg;
std::wstring              g_dllDir, g_exe;
std::mutex                g_lock;           // the maps below
std::map<XrInstance, std::unique_ptr<InstanceData>> g_instances;
std::map<XrSession, InstanceData*>   g_sessions;
std::map<XrActionSet, InstanceData*> g_sets;
PFN_xrGetInstanceProcAddr g_anyNextGipa = nullptr;
bool                      g_measure = false;  // log = 1: the layer's own cost per frame is measured and logged

// the layer's own CPU time per frame (see the header); one frame = our part of xrBeginFrame + our part of xrEndFrame
struct Cost {
    std::mutex m;
    double begin = 0, t0 = -1, sum = 0, max = 0, bSum = 0, bMax = 0, eSum = 0, eMax = 0;
    uint64_t frames = 0, menuFrames = 0;
};
Cost g_cost;
void CostBegin(double s) { std::lock_guard<std::mutex> lk(g_cost.m); g_cost.begin = s; }
void CostEnd(double s, bool menu) {
    std::lock_guard<std::mutex> lk(g_cost.m);
    const double now = NowS(), f = g_cost.begin + s;
    g_cost.begin = 0;
    if (g_cost.t0 < 0) g_cost.t0 = now;
    g_cost.sum += f;
    if (f > g_cost.max) g_cost.max = f;
    g_cost.bSum += f - s;
    if (f - s > g_cost.bMax) g_cost.bMax = f - s;
    g_cost.eSum += s;
    if (s > g_cost.eMax) g_cost.eMax = s;
    static int slowLogs = 0;                           // a single frame over 3 ms: named (the first 10 per game)
    if (f > 0.003 && slowLogs < 10) {
        ++slowLogs;
        LOGI("performance: one frame took %.2f ms (controllers + menu %.2f, pictures to the headset %.2f)%s", 1000.0 * f,
             1000.0 * (f - s), 1000.0 * s, slowLogs == 10 ? " -- no more of these lines" : "");
    }
    ++g_cost.frames;
    if (menu) ++g_cost.menuFrames;
    if (now - g_cost.t0 >= 60.0 && g_cost.frames) {
        const double n = (double)g_cost.frames;
        LOGI("performance: the layer's own work took %.3f ms per frame on average (most %.2f ms) over %llu frames in %.0f s "
             "[controllers + menu %.3f ms avg / %.2f most, pictures to the headset %.3f / %.2f]; VR menu / keyboard shown in "
             "%llu of them", 1000.0 * g_cost.sum / n, 1000.0 * g_cost.max, (unsigned long long)g_cost.frames, now - g_cost.t0,
             1000.0 * g_cost.bSum / n, 1000.0 * g_cost.bMax, 1000.0 * g_cost.eSum / n, 1000.0 * g_cost.eMax,
             (unsigned long long)g_cost.menuFrames);
        g_cost.t0 = now;
        g_cost.sum = g_cost.max = g_cost.bSum = g_cost.bMax = g_cost.eSum = g_cost.eMax = 0;
        g_cost.frames = g_cost.menuFrames = 0;
    }
}

void EnsureInit() {
    std::call_once(g_initOnce, [] {
        HMODULE self = nullptr;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           reinterpret_cast<LPCWSTR>(&EnsureInit), &self);
        wchar_t p[MAX_PATH] = L"";
        GetModuleFileNameW(self, p, MAX_PATH);
        g_dllDir = p;
        const size_t s = g_dllDir.find_last_of(L"\\/");
        if (s != std::wstring::npos) g_dllDir.resize(s);
        wchar_t e[MAX_PATH] = L"";
        GetModuleFileNameW(nullptr, e, MAX_PATH);
        g_exe = e;
        const size_t es = g_exe.find_last_of(L"\\/");
        if (es != std::wstring::npos) g_exe = g_exe.substr(es + 1);
        g_cfg = LoadConfig(g_dllDir);
        g_on = g_cfg.enabled && AppAllowed(g_cfg, g_exe);
        if (!g_on) return;                           // not our game: no log file, nothing touched
        g_measure = g_cfg.log;                       // (log = 0: no file, and the per-frame cost is not even measured)
        if (g_cfg.log) LogInit(g_dllDir, g_exe);
        LOGI("ETS2 ATS VR Controls " VRC_VERSION " (%s) loaded into this game", kLayerName);
        LOGI("config: %ls", g_cfg.loadedFrom.empty() ? L"(none, defaults)" : g_cfg.loadedFrom.c_str());
        for (const std::string& n : g_cfg.notes) LOGW("config: %s", n.c_str());
        LOGI("mapping: %s", DescribePad(g_cfg.pad).c_str());
    });
}

InstanceData* FindInstanceLocked(XrInstance i) {
    auto it = g_instances.find(i);
    return it == g_instances.end() ? nullptr : it->second.get();
}
InstanceData* FindSession(XrSession s) {
    std::lock_guard<std::mutex> lk(g_lock);
    auto it = g_sessions.find(s);
    return it == g_sessions.end() ? nullptr : it->second;
}
InstanceData* FindInstance(XrInstance i) {
    std::lock_guard<std::mutex> lk(g_lock);
    return FindInstanceLocked(i);
}

// ------------------------------------------------------------------------------------------------ hooks

XrResult XRAPI_CALL Hook_DestroyInstance(XrInstance instance) {
    PFN_xrDestroyInstance next = nullptr;
    std::unique_ptr<InstanceData> dead;
    {
        std::lock_guard<std::mutex> lk(g_lock);
        auto it = g_instances.find(instance);
        if (it != g_instances.end()) {
            next = it->second->DestroyInstance;
            InstanceData* id = it->second.get();
            for (auto s = g_sessions.begin(); s != g_sessions.end();) s = s->second == id ? g_sessions.erase(s) : std::next(s);
            for (auto s = g_sets.begin(); s != g_sets.end();) s = s->second == id ? g_sets.erase(s) : std::next(s);
            dead = std::move(it->second);
            g_instances.erase(it);
        }
    }
    if (!next) return XR_ERROR_HANDLE_INVALID;
    if (dead && dead->pad) dead->pad->ReleaseAll();   // a game that skips xrDestroySession: nothing stays held
    LOGI("instance %p destroyed", (void*)instance);
    LogFlush();
    return next(instance);   // (dead is freed after the call returns)
}

XrResult XRAPI_CALL Hook_CreateSession(XrInstance instance, const XrSessionCreateInfo* ci, XrSession* session) {
    InstanceData* id = FindInstance(instance);
    if (!id || !id->CreateSession) return XR_ERROR_HANDLE_INVALID;
    const XrResult r = id->CreateSession(instance, ci, session);
    if (XR_SUCCEEDED(r) && session) {
        {
            std::lock_guard<std::mutex> lk(g_lock);
            g_sessions[*session] = id;
        }
        ID3D11Device* dev = nullptr;                   // the game's graphics: the VR menu draws only into D3D11 swapchains
        for (const XrBaseInStructure* b = ci ? static_cast<const XrBaseInStructure*>(ci->next) : nullptr; b; b = b->next)
            if (b->type == XR_TYPE_GRAPHICS_BINDING_D3D11_KHR) dev = reinterpret_cast<const XrGraphicsBindingD3D11KHR*>(b)->device;
        if (id->pad) id->pad->SessionCreated(*session, dev);
        LOGI("session %p created", (void*)*session);
    }
    return r;
}

XrResult XRAPI_CALL Hook_DestroySession(XrSession session) {
    InstanceData* id = FindSession(session);
    if (!id || !id->DestroySession) return XR_ERROR_HANDLE_INVALID;
    if (id->pad) id->pad->SessionDestroyed(session);
    LogFlush();
    {
        std::lock_guard<std::mutex> lk(g_lock);
        g_sessions.erase(session);
    }
    return id->DestroySession(session);
}

XrResult XRAPI_CALL Hook_PollEvent(XrInstance instance, XrEventDataBuffer* ev) {
    InstanceData* id = FindInstance(instance);
    if (!id || !id->PollEvent) return XR_ERROR_HANDLE_INVALID;
    const XrResult r = id->PollEvent(instance, ev);
    if (r == XR_SUCCESS && ev && ev->type == XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED && id->pad) {
        const auto* sc = reinterpret_cast<const XrEventDataSessionStateChanged*>(ev);
        id->pad->SessionState(sc->session, sc->state);
    }
    return r;
}

XrResult XRAPI_CALL Hook_WaitFrame(XrSession session, const XrFrameWaitInfo* wi, XrFrameState* fs) {
    InstanceData* id = FindSession(session);
    if (!id || !id->WaitFrame) return XR_ERROR_HANDLE_INVALID;
    const XrResult r = id->WaitFrame(session, wi, fs);
    if (XR_SUCCEEDED(r) && fs && id->pad) id->pad->FrameWaited(session, fs->predictedDisplayTime);
    return r;
}

XrResult XRAPI_CALL Hook_BeginFrame(XrSession session, const XrFrameBeginInfo* bi) {
    InstanceData* id = FindSession(session);
    if (!id || !id->BeginFrame) return XR_ERROR_HANDLE_INVALID;
    if (id->pad) {
        const double t = g_measure ? NowS() : 0.0;
        try { id->pad->BeginFrame(session); } catch (...) { LOGE("frame tick failed (exception) -- skipped"); }
        if (g_measure) CostBegin(NowS() - t);
    }
    return id->BeginFrame(session, bi);
}

XrResult XRAPI_CALL Hook_EndFrame(XrSession session, const XrFrameEndInfo* fe) {
    InstanceData* id = FindSession(session);
    if (!id || !id->EndFrame) return XR_ERROR_HANDLE_INVALID;
    if (id->pad && fe) {
        std::vector<const XrCompositionLayerBaseHeader*> layers;
        XrFrameEndInfo mine{};
        bool ours = false;
        const double t = g_measure ? NowS() : 0.0;
        try { ours = id->pad->EndFrame(session, fe, &mine, layers); } catch (...) { LOGE("menu layers failed (exception) -- skipped"); }
        if (g_measure) CostEnd(NowS() - t, ours);
        if (ours) {
            const XrResult r = id->EndFrame(session, &mine);
            if (XR_SUCCEEDED(r)) return r;
            id->pad->EndFrameRefused(session, r);      // the game's own frame below
        }
    }
    return id->EndFrame(session, fe);
}

XrResult XRAPI_CALL Hook_GetActionStateBoolean(XrSession session, const XrActionStateGetInfo* gi, XrActionStateBoolean* st) {
    InstanceData* id = FindSession(session);
    if (!id || !id->GetActionStateBoolean) return XR_ERROR_HANDLE_INVALID;
    const XrResult r = id->GetActionStateBoolean(session, gi, st);
    if (XR_SUCCEEDED(r) && id->pad) id->pad->FilterBool(session, gi, st);
    return r;
}

XrResult XRAPI_CALL Hook_GetActionStateFloat(XrSession session, const XrActionStateGetInfo* gi, XrActionStateFloat* st) {
    InstanceData* id = FindSession(session);
    if (!id || !id->GetActionStateFloat) return XR_ERROR_HANDLE_INVALID;
    const XrResult r = id->GetActionStateFloat(session, gi, st);
    if (XR_SUCCEEDED(r) && id->pad) id->pad->FilterFloat(session, gi, st);
    return r;
}

XrResult XRAPI_CALL Hook_CreateActionSet(XrInstance instance, const XrActionSetCreateInfo* ci, XrActionSet* set) {
    InstanceData* id = FindInstance(instance);
    if (!id || !id->CreateActionSet) return XR_ERROR_HANDLE_INVALID;
    const XrResult r = id->CreateActionSet(instance, ci, set);
    if (XR_SUCCEEDED(r) && set) {
        {
            std::lock_guard<std::mutex> lk(g_lock);
            g_sets[*set] = id;
        }
        if (id->pad) id->pad->ActionSetCreated(*set, ci);
    }
    return r;
}

XrResult XRAPI_CALL Hook_CreateAction(XrActionSet set, const XrActionCreateInfo* ci, XrAction* action) {
    InstanceData* id = nullptr;
    {
        std::lock_guard<std::mutex> lk(g_lock);
        auto it = g_sets.find(set);
        id = it == g_sets.end() ? nullptr : it->second;
    }
    if (!id || !id->CreateAction) return XR_ERROR_HANDLE_INVALID;
    const XrResult r = id->CreateAction(set, ci, action);
    if (XR_SUCCEEDED(r) && action && id->pad) id->pad->ActionCreated(*action, set);
    return r;
}

XrResult XRAPI_CALL Hook_SuggestInteractionProfileBindings(XrInstance instance, const XrInteractionProfileSuggestedBinding* sb) {
    InstanceData* id = FindInstance(instance);
    if (!id || !id->SuggestInteractionProfileBindings) return XR_ERROR_HANDLE_INVALID;
    const XrResult r = id->SuggestInteractionProfileBindings(instance, sb);
    if (XR_SUCCEEDED(r) && id->pad) {
        try { id->pad->Suggested(sb); } catch (...) { LOGE("suggestion bookkeeping failed (exception)"); }
    }
    return r;
}

XrResult XRAPI_CALL Hook_AttachSessionActionSets(XrSession session, const XrSessionActionSetsAttachInfo* ai) {
    InstanceData* id = FindSession(session);
    if (!id || !id->AttachSessionActionSets) return XR_ERROR_HANDLE_INVALID;
    if (id->pad) {
        try { return id->pad->Attach(session, ai); } catch (...) { LOGE("attach bookkeeping failed (exception) -- forwarding as is"); }
    }
    return id->AttachSessionActionSets(session, ai);
}

XrResult XRAPI_CALL Hook_SyncActions(XrSession session, const XrActionsSyncInfo* si) {
    InstanceData* id = FindSession(session);
    if (!id || !id->SyncActions) return XR_ERROR_HANDLE_INVALID;
    if (id->pad) {
        try { return id->pad->Sync(session, si); } catch (...) { LOGE("sync bookkeeping failed (exception) -- forwarding as is"); }
    }
    return id->SyncActions(session, si);
}

// ------------------------------------------------------------------------------------------------ dispatch

XrResult XRAPI_CALL Layer_GetInstanceProcAddr(XrInstance instance, const char* name, PFN_xrVoidFunction* function);

#define HOOK(fn) { "xr" #fn, (PFN_xrVoidFunction)Hook_##fn }
struct HookEntry { const char* name; PFN_xrVoidFunction fn; };
const HookEntry kHooks[] = {
    HOOK(DestroyInstance), HOOK(CreateSession), HOOK(DestroySession), HOOK(PollEvent), HOOK(WaitFrame), HOOK(BeginFrame),
    HOOK(CreateActionSet), HOOK(CreateAction), HOOK(SuggestInteractionProfileBindings), HOOK(AttachSessionActionSets),
    HOOK(SyncActions), HOOK(EndFrame), HOOK(GetActionStateBoolean), HOOK(GetActionStateFloat),
};
#undef HOOK

XrResult XRAPI_CALL Layer_GetInstanceProcAddr(XrInstance instance, const char* name, PFN_xrVoidFunction* function) {
    if (!name || !function) return XR_ERROR_VALIDATION_FAILURE;
    if (strcmp(name, "xrGetInstanceProcAddr") == 0) {
        *function = (PFN_xrVoidFunction)Layer_GetInstanceProcAddr;
        return XR_SUCCESS;
    }
    PFN_xrGetInstanceProcAddr next = nullptr;
    bool known = false;
    {
        std::lock_guard<std::mutex> lk(g_lock);
        InstanceData* id = FindInstanceLocked(instance);
        known = id != nullptr;
        next = id ? id->nextGipa : g_anyNextGipa;
    }
    if (!next) { *function = nullptr; return XR_ERROR_HANDLE_INVALID; }
    const XrResult r = next(instance, name, function);
    if (XR_FAILED(r) || !*function || !known) return r;
    for (const HookEntry& h : kHooks)
        if (strcmp(name, h.name) == 0) { *function = h.fn; break; }
    return r;
}

XrResult XRAPI_CALL Layer_CreateApiLayerInstance(const XrInstanceCreateInfo* info, const XrApiLayerCreateInfo* li, XrInstance* instance) {
    EnsureInit();
    if (!li || li->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_CREATE_INFO || !li->nextInfo ||
        li->nextInfo->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_NEXT_INFO || !li->nextInfo->nextCreateApiLayerInstance ||
        !li->nextInfo->nextGetInstanceProcAddr) {
        LOGE("xrCreateApiLayerInstance: invalid loader structures");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
    if (strcmp(li->nextInfo->layerName, kLayerName) != 0) {
        LOGE("xrCreateApiLayerInstance: chain names '%s', expected %s", li->nextInfo->layerName, kLayerName);
        return XR_ERROR_INITIALIZATION_FAILED;
    }
    XrApiLayerCreateInfo nextLi = *li;
    nextLi.nextInfo = li->nextInfo->next;
    const XrResult r = li->nextInfo->nextCreateApiLayerInstance(info, &nextLi, instance);
    if (XR_FAILED(r)) {
        if (g_on) LOGI("xrCreateInstance (next in chain) returned %d -- layer idle", (int)r);
        return r;
    }
    PFN_xrGetInstanceProcAddr nextGipa = li->nextInfo->nextGetInstanceProcAddr;
    if (!g_on) {                                     // not our game: every function is the next layer's own
        std::lock_guard<std::mutex> lk(g_lock);
        g_anyNextGipa = nextGipa;
        return r;
    }
    try {
        auto id = std::make_unique<InstanceData>();
        id->handle = *instance;
        id->nextGipa = nextGipa;
#define RESOLVE(fn) id->nextGipa(*instance, "xr" #fn, reinterpret_cast<PFN_xrVoidFunction*>(&id->fn))
        RESOLVE(DestroyInstance); RESOLVE(CreateSession); RESOLVE(DestroySession); RESOLVE(PollEvent); RESOLVE(WaitFrame);
        RESOLVE(BeginFrame); RESOLVE(CreateActionSet); RESOLVE(CreateAction); RESOLVE(SuggestInteractionProfileBindings);
        RESOLVE(AttachSessionActionSets); RESOLVE(SyncActions); RESOLVE(EndFrame); RESOLVE(GetActionStateBoolean);
        RESOLVE(GetActionStateFloat);
#undef RESOLVE
        PadDispatch d;
        d.CreateActionSet = id->CreateActionSet;
        d.CreateAction = id->CreateAction;
        d.SuggestInteractionProfileBindings = id->SuggestInteractionProfileBindings;
        d.AttachSessionActionSets = id->AttachSessionActionSets;
        d.SyncActions = id->SyncActions;
#define RESOLVE_D(fn) id->nextGipa(*instance, "xr" #fn, reinterpret_cast<PFN_xrVoidFunction*>(&d.fn))
        RESOLVE_D(StringToPath); RESOLVE_D(PathToString); RESOLVE_D(GetActionStateBoolean); RESOLVE_D(GetActionStateFloat);
        RESOLVE_D(GetActionStateVector2f); RESOLVE_D(GetCurrentInteractionProfile); RESOLVE_D(CreateActionSpace);
        RESOLVE_D(CreateReferenceSpace); RESOLVE_D(LocateSpace); RESOLVE_D(DestroySpace);
#undef RESOLVE_D
        OverlayDispatch od;
#define RESOLVE_O(fn) id->nextGipa(*instance, "xr" #fn, reinterpret_cast<PFN_xrVoidFunction*>(&od.fn))
        RESOLVE_O(EnumerateSwapchainFormats); RESOLVE_O(CreateSwapchain); RESOLVE_O(DestroySwapchain);
        RESOLVE_O(EnumerateSwapchainImages); RESOLVE_O(AcquireSwapchainImage); RESOLVE_O(WaitSwapchainImage);
        RESOLVE_O(ReleaseSwapchainImage);
#undef RESOLVE_O
        id->pad = std::make_unique<PadInput>(*instance, d, g_cfg, g_cfg.loadedFrom.empty() ? g_dllDir + L"\\config.ini" : g_cfg.loadedFrom, od);
        if (!id->pad->Enabled()) id->pad.reset();
        LOGI("instance %p created: app '%s' (engine '%s') -- %s", (void*)*instance, info ? info->applicationInfo.applicationName : "?",
             info ? info->applicationInfo.engineName : "?",
             id->pad ? "our controls are added when the game attaches its input" : "controllers OFF (see above)");
        std::lock_guard<std::mutex> lk(g_lock);
        g_anyNextGipa = id->nextGipa;
        g_instances[*instance] = std::move(id);
    } catch (...) {
        LOGE("instance bookkeeping failed -- the layer passes everything through");
    }
    return r;
}

}  // namespace
}  // namespace ck

// ------------------------------------------------------------------------------------------------ export

extern "C" __declspec(dllexport) XrResult XRAPI_CALL xrNegotiateLoaderApiLayerInterface(const XrNegotiateLoaderInfo* loaderInfo,
                                                                                         const char* layerName,
                                                                                         XrNegotiateApiLayerRequest* req) {
    using namespace ck;
    EnsureInit();
    if (!loaderInfo || !req || loaderInfo->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO ||
        loaderInfo->structVersion != XR_LOADER_INFO_STRUCT_VERSION || loaderInfo->structSize != sizeof(XrNegotiateLoaderInfo) ||
        req->structType != XR_LOADER_INTERFACE_STRUCT_API_LAYER_REQUEST || req->structVersion != XR_API_LAYER_INFO_STRUCT_VERSION ||
        req->structSize != sizeof(XrNegotiateApiLayerRequest)) {
        LOGE("negotiate: unexpected loader structures");
        return XR_ERROR_INITIALIZATION_FAILED;
    }
    if (layerName && strcmp(layerName, kLayerName) != 0) {
        LOGE("negotiate: asked for layer '%s'", layerName);
        return XR_ERROR_INITIALIZATION_FAILED;
    }
    if (loaderInfo->minInterfaceVersion > XR_CURRENT_LOADER_API_LAYER_VERSION ||
        loaderInfo->maxInterfaceVersion < XR_CURRENT_LOADER_API_LAYER_VERSION) {
        LOGE("negotiate: loader interface %u..%u unsupported", loaderInfo->minInterfaceVersion, loaderInfo->maxInterfaceVersion);
        return XR_ERROR_INITIALIZATION_FAILED;
    }
    XrVersion api = XR_CURRENT_API_VERSION;
    if (api > loaderInfo->maxApiVersion) api = loaderInfo->maxApiVersion;
    if (api < loaderInfo->minApiVersion) api = loaderInfo->minApiVersion;
    req->layerInterfaceVersion = XR_CURRENT_LOADER_API_LAYER_VERSION;
    req->layerApiVersion = api;
    req->getInstanceProcAddr = Layer_GetInstanceProcAddr;
    req->createApiLayerInstance = Layer_CreateApiLayerInstance;
    if (g_on) LOGI("negotiate: OK (layer API %u.%u)", XR_VERSION_MAJOR(api), XR_VERSION_MINOR(api));
    return XR_SUCCESS;
}

BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID) {
    return TRUE;   // nothing here (loader lock); setup runs on the first negotiate call
}
