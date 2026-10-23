// mock_runtime -- a tiny OpenXR runtime for the end-to-end test (tests/run_e2e.ps1): no graphics, no headset. It implements what
// the fake game and the layer call: instance / system / session (state events up to FOCUSED at xrBeginSession), frames, paths,
// action sets / actions, suggestions, attach, sync, action states, action + reference spaces, xrLocateSpace,
// xrGetCurrentInteractionProfile.
//
// Like a real runtime: a suggestion REPLACES the earlier one for its profile; binding paths are checked against the inputs each
// accepted profile really has (a wrong path = XR_ERROR_PATH_UNSUPPORTED for the whole call); hp/mixed_reality_controller is NOT
// accepted (a runtime without that extension); one attach per session, suggestions refused after the first attach; the profile
// in use = oculus/touch_controller when it was suggested (the "controllers" are Quest Touch), else the first suggested one.
// Pressed inputs come from VRC_MOCK_INPUT = "<t0>-<t1>:<input path>=<value>;..." (seconds since xrCreateSession). Values:
// "1" for a button, "0.9" for a trigger, "x,y" for a stick, "yaw0,pitch0,yaw1,pitch1" (degrees, linear over the interval) for
// an aim pose. VRC_MOCK_REPORT = a file that gets one line per event worth checking ("profile ...", "suggest ...", "attach ...").
// D3D11 (XR_KHR_D3D11_enable, for the VR menu): swapchains of real D3D11 textures on the game's device (R8G8B8A8_TYPELESS for
// the _SRGB format, as real runtimes do), acquire / wait / release, and xrEndFrame checks the layers like a runtime (at most 16;
// a quad's swapchain must have a released image, its space must exist) and reports "endframe quads N" on each change. With
// VRC_MOCK_DUMP = a folder, each quad picture is written there as quad_<w>x<h>.bmp after it changed (the last released image).
#include <windows.h>
#include <d3d11.h>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <openxr/openxr.h>
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr_platform.h>
#include <openxr/openxr_loader_negotiation.h>

namespace {

std::mutex g_m;
std::vector<std::string> g_paths(1);   // XrPath = index (0 = XR_NULL_PATH)
XrPath PathOf(const std::string& s) {
    for (size_t i = 1; i < g_paths.size(); ++i) if (g_paths[i] == s) return (XrPath)i;
    g_paths.push_back(s);
    return (XrPath)(g_paths.size() - 1);
}
std::string StrOf(XrPath p) { return p > 0 && p < g_paths.size() ? g_paths[(size_t)p] : std::string(); }

void Report(const char* fmt, ...) {
    char path[MAX_PATH];
    if (!GetEnvironmentVariableA("VRC_MOCK_REPORT", path, MAX_PATH)) return;
    FILE* f = nullptr;
    if (fopen_s(&f, path, "ab") || !f) return;
    va_list a;
    va_start(a, fmt);
    vfprintf(f, fmt, a);
    va_end(a);
    fputs("\n", f);
    fclose(f);
}

// the inputs each accepted profile has (both hands unless noted), from the OpenXR 1.0 spec
struct Prof { const char* path; std::vector<std::string> left, right; };
std::vector<Prof> MakeProfiles() {
    auto both = [](std::vector<std::string> v) { return v; };
    std::vector<std::string> touchCommon = { "squeeze/value", "trigger/value", "trigger/touch", "thumbstick", "thumbstick/x", "thumbstick/y",
                                             "thumbstick/click", "thumbstick/touch", "thumbrest/touch", "grip/pose", "aim/pose" };
    std::vector<std::string> tl = touchCommon, tr = touchCommon;
    for (const char* s : { "x/click", "x/touch", "y/click", "y/touch", "menu/click" }) tl.push_back(s);
    for (const char* s : { "a/click", "a/touch", "b/click", "b/touch", "system/click" }) tr.push_back(s);
    std::vector<std::string> index = { "system/click", "system/touch", "a/click", "a/touch", "b/click", "b/touch", "squeeze/value",
                                       "squeeze/force", "trigger/click", "trigger/value", "trigger/touch", "thumbstick", "thumbstick/x",
                                       "thumbstick/y", "thumbstick/click", "thumbstick/touch", "trackpad", "trackpad/x", "trackpad/y",
                                       "trackpad/force", "trackpad/touch", "grip/pose", "aim/pose" };
    std::vector<std::string> vive = { "system/click", "squeeze/click", "menu/click", "trigger/click", "trigger/value", "trackpad",
                                      "trackpad/x", "trackpad/y", "trackpad/click", "trackpad/touch", "grip/pose", "aim/pose" };
    std::vector<std::string> wmr = { "menu/click", "squeeze/click", "trigger/value", "thumbstick", "thumbstick/x", "thumbstick/y",
                                     "thumbstick/click", "trackpad", "trackpad/x", "trackpad/y", "trackpad/click", "trackpad/touch",
                                     "grip/pose", "aim/pose" };
    std::vector<std::string> simple = { "select/click", "menu/click", "grip/pose", "aim/pose" };
    return { { "/interaction_profiles/oculus/touch_controller", tl, tr }, { "/interaction_profiles/valve/index_controller", both(index), both(index) },
             { "/interaction_profiles/htc/vive_controller", vive, vive }, { "/interaction_profiles/microsoft/motion_controller", wmr, wmr },
             { "/interaction_profiles/khr/simple_controller", simple, simple } };
}
const std::vector<Prof>& Profiles() { static const std::vector<Prof> p = MakeProfiles(); return p; }
const Prof* FindProf(const std::string& p) {
    for (const Prof& x : Profiles()) if (p == x.path) return &x;
    return nullptr;
}
bool ValidBinding(const Prof& pr, const std::string& b) {
    for (int h = 0; h < 2; ++h) {
        const std::string top = h ? "/user/hand/right/" : "/user/hand/left/";
        if (b.compare(0, top.size(), top) != 0) continue;
        const std::string rest = b.substr(top.size());
        if (rest == "output/haptic") return true;
        if (rest.compare(0, 6, "input/") != 0) return false;
        for (const std::string& s : h ? pr.right : pr.left) if (rest.substr(6) == s) return true;
        return false;
    }
    return false;
}

struct ActionSet { std::string name; uint32_t priority = 0; bool attached = false; };
struct Action { XrActionSet set; XrActionType type; std::vector<XrPath> sub; std::string name; };
struct State { float f = 0; bool b = false; XrVector2f v{0, 0}; bool active = false; };
struct Session {
    double created = 0;
    bool attached = false, focused = false;
    std::string profile;                                  // in use (after attach)
    std::set<XrActionSet> activeSets;                     // of the last sync
    std::map<std::pair<XrAction, XrPath>, State> states;  // (action, subaction path or 0)
    std::set<XrActionSet> attachedSets;
    uint64_t syncs = 0, soloSyncs = 0;                    // soloSyncs: a sync that left an attached "input" set (the game's) inactive
    int unfocusStep = 0;                                  // VRC_MOCK_UNFOCUS: 0 not yet, 1 unfocused, 2 focused again
    ID3D11Device* dev = nullptr;                          // XrGraphicsBindingD3D11KHR (AddRef'd)
    int lastQuads = -1;
};
struct Swapchain {
    XrSession ses; uint32_t w = 0, h = 0; int64_t format = 0;
    std::vector<ID3D11Texture2D*> tex;
    uint32_t next = 0; int acquired = -1, waited = -1, lastReleased = -1;
    uint64_t releases = 0, dumped = 0;
};
struct Space { XrSession ses; XrAction action = XR_NULL_HANDLE; XrPath sub = XR_NULL_PATH; };

uint64_t g_next = 0x100;
std::map<uint64_t, ActionSet> g_sets;
std::map<uint64_t, Action> g_actions;
std::map<uint64_t, Session> g_sessions;
std::map<uint64_t, Space> g_spaces;
std::map<uint64_t, Swapchain> g_swapchains;
std::map<XrPath, std::vector<XrActionSuggestedBinding>> g_suggest;   // per profile
bool g_anyAttached = false;
std::vector<std::pair<XrSession, XrSessionState>> g_events;
XrTime g_time = 1000000000;

double Now() {
    static LARGE_INTEGER f = [] { LARGE_INTEGER x; QueryPerformanceFrequency(&x); return x; }();
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart / (double)f.QuadPart;
}

// scripted value of input `path` at t seconds; false = not pressed / not moved
bool Scripted(const std::string& path, double t, std::vector<double>& vals) {
    static std::vector<char> spec(32768);              // (a long script: the performance test switches tabs for a minute)
    spec[0] = 0;
    if (GetEnvironmentVariableA("VRC_MOCK_INPUT", spec.data(), (DWORD)spec.size()) >= spec.size()) spec[0] = 0;
    std::string s = spec.data();
    size_t pos = 0;
    while (pos < s.size()) {
        size_t semi = s.find(';', pos);
        if (semi == std::string::npos) semi = s.size();
        const std::string item = s.substr(pos, semi - pos);
        pos = semi + 1;
        double t0 = 0, t1 = 0;
        char p[256] = "", v[128] = "";
        if (sscanf_s(item.c_str(), "%lf-%lf:%255[^=]=%127s", &t0, &t1, p, (unsigned)sizeof(p), v, (unsigned)sizeof(v)) != 4) continue;
        if (path != p || t < t0 || t > t1) continue;
        vals.clear();
        char* c = v;
        while (*c) { vals.push_back(strtod(c, &c)); if (*c == ',') ++c; else break; }
        if (vals.size() == 4) {                           // aim ramp
            const double k = t1 > t0 ? (t - t0) / (t1 - t0) : 0.0;
            vals = { vals[0] + (vals[2] - vals[0]) * k, vals[1] + (vals[3] - vals[1]) * k };
        }
        return true;
    }
    return false;
}

// the input paths bound to `a` for subaction path `sub` (0 = any hand) in the session's profile
std::vector<std::string> BoundPaths(const Session& s, XrAction a, XrPath sub) {
    std::vector<std::string> out;
    auto it = g_suggest.find(PathOf(s.profile));
    if (it == g_suggest.end()) return out;
    const std::string top = sub ? StrOf(sub) + "/" : std::string();
    for (const XrActionSuggestedBinding& b : it->second) {
        if (b.action != a) continue;
        const std::string bp = StrOf(b.binding);
        if (!top.empty() && bp.compare(0, top.size(), top) != 0) continue;
        out.push_back(bp);
    }
    return out;
}

// ------------------------------------------------------------------------------------------------ functions
XrResult XRAPI_CALL EnumerateInstanceExtensionProperties(const char*, uint32_t cap, uint32_t* n, XrExtensionProperties* p) {
    if (n) *n = 1;
    if (!cap) return XR_SUCCESS;
    if (!p) return XR_ERROR_VALIDATION_FAILURE;
    strcpy_s(p[0].extensionName, XR_KHR_D3D11_ENABLE_EXTENSION_NAME);
    p[0].extensionVersion = 1;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL CreateInstance(const XrInstanceCreateInfo*, XrInstance* i) { *i = (XrInstance)(uintptr_t)0x1; return XR_SUCCESS; }
XrResult XRAPI_CALL DestroyInstance(XrInstance) { return XR_SUCCESS; }
XrResult XRAPI_CALL GetInstanceProperties(XrInstance, XrInstanceProperties* p) {
    p->runtimeVersion = XR_MAKE_VERSION(1, 0, 0);
    strcpy_s(p->runtimeName, "CK mock runtime");
    return XR_SUCCESS;
}
XrResult XRAPI_CALL GetSystem(XrInstance, const XrSystemGetInfo*, XrSystemId* s) { *s = 1; return XR_SUCCESS; }
XrResult XRAPI_CALL StringToPath(XrInstance, const char* s, XrPath* p) {
    std::lock_guard<std::mutex> lk(g_m);
    *p = PathOf(s);
    return XR_SUCCESS;
}
XrResult XRAPI_CALL PathToString(XrInstance, XrPath p, uint32_t cap, uint32_t* n, char* buf) {
    std::lock_guard<std::mutex> lk(g_m);
    if (!p || p >= g_paths.size()) return XR_ERROR_PATH_INVALID;
    const std::string& s = g_paths[(size_t)p];
    *n = (uint32_t)s.size() + 1;
    if (cap == 0) return XR_SUCCESS;
    if (cap < *n) return XR_ERROR_SIZE_INSUFFICIENT;
    memcpy(buf, s.c_str(), *n);
    return XR_SUCCESS;
}
XrResult XRAPI_CALL CreateSession(XrInstance, const XrSessionCreateInfo* ci, XrSession* s) {
    std::lock_guard<std::mutex> lk(g_m);
    const uint64_t h = g_next++;
    g_sessions[h].created = Now();
    for (const XrBaseInStructure* b = static_cast<const XrBaseInStructure*>(ci->next); b; b = b->next)
        if (b->type == XR_TYPE_GRAPHICS_BINDING_D3D11_KHR) {
            g_sessions[h].dev = reinterpret_cast<const XrGraphicsBindingD3D11KHR*>(b)->device;
            if (g_sessions[h].dev) g_sessions[h].dev->AddRef();
        }
    *s = (XrSession)h;
    g_events.push_back({*s, XR_SESSION_STATE_IDLE});
    g_events.push_back({*s, XR_SESSION_STATE_READY});
    return XR_SUCCESS;
}
XrResult XRAPI_CALL DestroySession(XrSession s) {
    std::lock_guard<std::mutex> lk(g_m);
    for (auto it = g_spaces.begin(); it != g_spaces.end();) it = it->second.ses == s ? g_spaces.erase(it) : std::next(it);
    auto ss = g_sessions.find((uint64_t)s);
    if (ss != g_sessions.end())
        Report("syncs %llu solo %llu", (unsigned long long)ss->second.syncs, (unsigned long long)ss->second.soloSyncs);
    int left = 0;
    for (auto it = g_swapchains.begin(); it != g_swapchains.end();) {
        if (it->second.ses != s) { ++it; continue; }
        ++left;
        for (ID3D11Texture2D* t : it->second.tex) t->Release();
        it = g_swapchains.erase(it);
    }
    if (left) Report("session destroyed with %d swapchain(s) still alive", left);
    if (ss != g_sessions.end() && ss->second.dev) ss->second.dev->Release();
    g_sessions.erase((uint64_t)s);
    return XR_SUCCESS;
}
XrResult XRAPI_CALL BeginSession(XrSession s, const XrSessionBeginInfo*) {
    std::lock_guard<std::mutex> lk(g_m);
    g_events.push_back({s, XR_SESSION_STATE_SYNCHRONIZED});
    g_events.push_back({s, XR_SESSION_STATE_VISIBLE});
    g_events.push_back({s, XR_SESSION_STATE_FOCUSED});
    return XR_SUCCESS;
}
XrResult XRAPI_CALL EndSession(XrSession) { return XR_SUCCESS; }
XrResult XRAPI_CALL RequestExitSession(XrSession) { return XR_SUCCESS; }
XrResult XRAPI_CALL PollEvent(XrInstance, XrEventDataBuffer* ev) {
    std::lock_guard<std::mutex> lk(g_m);
    if (g_events.empty()) return XR_EVENT_UNAVAILABLE;
    auto e = g_events.front();
    g_events.erase(g_events.begin());
    auto* sc = reinterpret_cast<XrEventDataSessionStateChanged*>(ev);
    sc->type = XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED;
    sc->next = nullptr;
    sc->session = e.first;
    sc->state = e.second;
    sc->time = g_time;
    auto it = g_sessions.find((uint64_t)e.first);
    if (it != g_sessions.end()) it->second.focused = e.second == XR_SESSION_STATE_FOCUSED;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL WaitFrame(XrSession s, const XrFrameWaitInfo*, XrFrameState* fs) {
    Sleep(11);
    std::lock_guard<std::mutex> lk(g_m);
    // VRC_MOCK_UNFOCUS = "t0-t1": the session drops to VISIBLE at t0 and is FOCUSED again at t1 (seconds since xrCreateSession)
    char uf[64] = "";
    double u0 = 0, u1 = 0;
    auto ss = g_sessions.find((uint64_t)s);
    if (ss != g_sessions.end() && GetEnvironmentVariableA("VRC_MOCK_UNFOCUS", uf, sizeof(uf)) && sscanf_s(uf, "%lf-%lf", &u0, &u1) == 2) {
        const double t = Now() - ss->second.created;
        if (ss->second.unfocusStep == 0 && t >= u0) { ss->second.unfocusStep = 1; g_events.push_back({s, XR_SESSION_STATE_VISIBLE}); }
        if (ss->second.unfocusStep == 1 && t >= u1) { ss->second.unfocusStep = 2; g_events.push_back({s, XR_SESSION_STATE_FOCUSED}); }
    }
    g_time += 11111111;
    fs->predictedDisplayTime = g_time;
    fs->predictedDisplayPeriod = 11111111;
    fs->shouldRender = XR_TRUE;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL BeginFrame(XrSession, const XrFrameBeginInfo*) { return XR_SUCCESS; }
void DumpLocked(Session& ss, Swapchain& sc) {
    char dir[MAX_PATH];
    if (!GetEnvironmentVariableA("VRC_MOCK_DUMP", dir, MAX_PATH) || !ss.dev || sc.lastReleased < 0) return;
    D3D11_TEXTURE2D_DESC d{};
    sc.tex[(size_t)sc.lastReleased]->GetDesc(&d);
    d.Usage = D3D11_USAGE_STAGING; d.BindFlags = 0; d.CPUAccessFlags = D3D11_CPU_ACCESS_READ; d.MiscFlags = 0;
    ID3D11Texture2D* st = nullptr;
    if (FAILED(ss.dev->CreateTexture2D(&d, nullptr, &st))) return;
    ID3D11DeviceContext* ctx = nullptr;
    ss.dev->GetImmediateContext(&ctx);
    ctx->CopyResource(st, sc.tex[(size_t)sc.lastReleased]);
    D3D11_MAPPED_SUBRESOURCE m{};
    if (SUCCEEDED(ctx->Map(st, 0, D3D11_MAP_READ, 0, &m))) {
        char path[MAX_PATH + 64];
        snprintf(path, sizeof(path), "%s\\quad_%ux%u.bmp", dir, sc.w, sc.h);
        FILE* f = nullptr;
        if (!fopen_s(&f, path, "wb") && f) {
            BITMAPFILEHEADER fh{};
            BITMAPINFOHEADER ih{};
            ih.biSize = sizeof(ih); ih.biWidth = (LONG)sc.w; ih.biHeight = -(LONG)sc.h; ih.biPlanes = 1; ih.biBitCount = 32;
            fh.bfType = 0x4D42; fh.bfOffBits = sizeof(fh) + sizeof(ih); fh.bfSize = fh.bfOffBits + sc.w * sc.h * 4;
            fwrite(&fh, sizeof(fh), 1, f);
            fwrite(&ih, sizeof(ih), 1, f);
            std::vector<uint8_t> row(sc.w * 4);
            for (uint32_t y = 0; y < sc.h; ++y) {
                memcpy(row.data(), static_cast<const uint8_t*>(m.pData) + (size_t)y * m.RowPitch, row.size());
                for (uint32_t x = 0; x < sc.w; ++x) std::swap(row[x * 4], row[x * 4 + 2]);   // RGBA -> BGRA
                fwrite(row.data(), 1, row.size(), f);
            }
            fclose(f);
        }
        ctx->Unmap(st, 0);
    }
    const uint32_t lv = d.MipLevels > 2 ? 2 : 0;          // also mip level 2 (a quarter of the size): the filter can be looked at
    if (lv && SUCCEEDED(ctx->Map(st, lv, D3D11_MAP_READ, 0, &m))) {
        const uint32_t w = sc.w >> lv ? sc.w >> lv : 1, h = sc.h >> lv ? sc.h >> lv : 1;
        char path[MAX_PATH + 64];
        snprintf(path, sizeof(path), "%s\\quad_%ux%u_mip2.bmp", dir, sc.w, sc.h);
        FILE* f = nullptr;
        if (!fopen_s(&f, path, "wb") && f) {
            BITMAPFILEHEADER fh{};
            BITMAPINFOHEADER ih{};
            ih.biSize = sizeof(ih); ih.biWidth = (LONG)w; ih.biHeight = -(LONG)h; ih.biPlanes = 1; ih.biBitCount = 32;
            fh.bfType = 0x4D42; fh.bfOffBits = sizeof(fh) + sizeof(ih); fh.bfSize = fh.bfOffBits + w * h * 4;
            fwrite(&fh, sizeof(fh), 1, f);
            fwrite(&ih, sizeof(ih), 1, f);
            std::vector<uint8_t> row(w * 4);
            for (uint32_t y = 0; y < h; ++y) {
                memcpy(row.data(), static_cast<const uint8_t*>(m.pData) + (size_t)y * m.RowPitch, row.size());
                for (uint32_t x = 0; x < w; ++x) std::swap(row[x * 4], row[x * 4 + 2]);
                fwrite(row.data(), 1, row.size(), f);
            }
            fclose(f);
        }
        ctx->Unmap(st, lv);
    }
    ctx->Release();
    st->Release();
}
XrResult XRAPI_CALL EndFrame(XrSession s, const XrFrameEndInfo* fe) {
    std::lock_guard<std::mutex> lk(g_m);
    auto ss = g_sessions.find((uint64_t)s);
    if (ss == g_sessions.end()) return XR_ERROR_HANDLE_INVALID;
    if (fe->layerCount > 16) { Report("endframe REFUSED (%u layers)", fe->layerCount); return XR_ERROR_LAYER_LIMIT_EXCEEDED; }
    int quads = 0;
    for (uint32_t i = 0; i < fe->layerCount; ++i) {
        const XrCompositionLayerBaseHeader* l = fe->layers[i];
        if (!l) return XR_ERROR_LAYER_INVALID;
        if (l->type != XR_TYPE_COMPOSITION_LAYER_QUAD) continue;
        const auto* q = reinterpret_cast<const XrCompositionLayerQuad*>(l);
        auto sc = g_swapchains.find((uint64_t)q->subImage.swapchain);
        if (sc == g_swapchains.end() || sc->second.releases == 0 || !g_spaces.count((uint64_t)q->space)) {
            Report("endframe REFUSED (quad %u: bad swapchain / no released image / bad space)", i);
            return XR_ERROR_LAYER_INVALID;
        }
        ++quads;
        if (sc->second.dumped != sc->second.releases) { sc->second.dumped = sc->second.releases; DumpLocked(ss->second, sc->second); }
    }
    if (quads != ss->second.lastQuads) { ss->second.lastQuads = quads; Report("endframe quads %d", quads); }
    return XR_SUCCESS;
}
XrResult XRAPI_CALL EnumerateSwapchainFormats(XrSession, uint32_t cap, uint32_t* n, int64_t* f) {
    *n = 2;
    if (!cap) return XR_SUCCESS;
    if (cap < 2) return XR_ERROR_SIZE_INSUFFICIENT;
    f[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    f[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL CreateSwapchain(XrSession s, const XrSwapchainCreateInfo* ci, XrSwapchain* out) {
    std::lock_guard<std::mutex> lk(g_m);
    auto ss = g_sessions.find((uint64_t)s);
    if (ss == g_sessions.end()) return XR_ERROR_HANDLE_INVALID;
    if (!ss->second.dev) return XR_ERROR_GRAPHICS_DEVICE_INVALID;
    if (ci->format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB && ci->format != DXGI_FORMAT_R8G8B8A8_UNORM) return XR_ERROR_SWAPCHAIN_FORMAT_UNSUPPORTED;
    Swapchain sc;
    sc.ses = s; sc.w = ci->width; sc.h = ci->height; sc.format = ci->format;
    D3D11_TEXTURE2D_DESC d{};
    d.Width = ci->width; d.Height = ci->height; d.MipLevels = ci->mipCount ? ci->mipCount : 1; d.ArraySize = 1;
    d.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
    d.SampleDesc.Count = 1; d.Usage = D3D11_USAGE_DEFAULT; d.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    for (int i = 0; i < 3; ++i) {
        ID3D11Texture2D* t = nullptr;
        if (FAILED(ss->second.dev->CreateTexture2D(&d, nullptr, &t))) return XR_ERROR_RUNTIME_FAILURE;
        sc.tex.push_back(t);
    }
    const uint64_t h = g_next++;
    g_swapchains[h] = sc;
    *out = (XrSwapchain)h;
    Report("swapchain %ux%u mips %u", ci->width, ci->height, d.MipLevels);
    return XR_SUCCESS;
}
XrResult XRAPI_CALL DestroySwapchain(XrSwapchain sc) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_swapchains.find((uint64_t)sc);
    if (it == g_swapchains.end()) return XR_ERROR_HANDLE_INVALID;
    for (ID3D11Texture2D* t : it->second.tex) t->Release();
    g_swapchains.erase(it);
    return XR_SUCCESS;
}
XrResult XRAPI_CALL EnumerateSwapchainImages(XrSwapchain sc, uint32_t cap, uint32_t* n, XrSwapchainImageBaseHeader* im) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_swapchains.find((uint64_t)sc);
    if (it == g_swapchains.end()) return XR_ERROR_HANDLE_INVALID;
    *n = (uint32_t)it->second.tex.size();
    if (!cap) return XR_SUCCESS;
    if (cap < *n) return XR_ERROR_SIZE_INSUFFICIENT;
    auto* d = reinterpret_cast<XrSwapchainImageD3D11KHR*>(im);
    for (uint32_t i = 0; i < *n; ++i) {
        if (d[i].type != XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR) return XR_ERROR_VALIDATION_FAILURE;
        d[i].texture = it->second.tex[i];
    }
    return XR_SUCCESS;
}
XrResult XRAPI_CALL AcquireSwapchainImage(XrSwapchain sc, const XrSwapchainImageAcquireInfo*, uint32_t* idx) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_swapchains.find((uint64_t)sc);
    if (it == g_swapchains.end()) return XR_ERROR_HANDLE_INVALID;
    if (it->second.acquired >= 0) return XR_ERROR_CALL_ORDER_INVALID;
    it->second.acquired = (int)it->second.next;
    it->second.next = (it->second.next + 1) % (uint32_t)it->second.tex.size();
    *idx = (uint32_t)it->second.acquired;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL WaitSwapchainImage(XrSwapchain sc, const XrSwapchainImageWaitInfo*) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_swapchains.find((uint64_t)sc);
    if (it == g_swapchains.end()) return XR_ERROR_HANDLE_INVALID;
    if (it->second.acquired < 0 || it->second.waited >= 0) return XR_ERROR_CALL_ORDER_INVALID;
    it->second.waited = it->second.acquired;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL ReleaseSwapchainImage(XrSwapchain sc, const XrSwapchainImageReleaseInfo*) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_swapchains.find((uint64_t)sc);
    if (it == g_swapchains.end()) return XR_ERROR_HANDLE_INVALID;
    if (it->second.waited < 0) return XR_ERROR_CALL_ORDER_INVALID;
    it->second.lastReleased = it->second.waited;
    it->second.acquired = it->second.waited = -1;
    ++it->second.releases;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL CreateReferenceSpace(XrSession s, const XrReferenceSpaceCreateInfo*, XrSpace* sp) {
    std::lock_guard<std::mutex> lk(g_m);
    const uint64_t h = g_next++;
    g_spaces[h] = Space{s};
    *sp = (XrSpace)h;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL CreateActionSpace(XrSession s, const XrActionSpaceCreateInfo* ci, XrSpace* sp) {
    std::lock_guard<std::mutex> lk(g_m);
    const uint64_t h = g_next++;
    g_spaces[h] = Space{s, ci->action, ci->subactionPath};
    *sp = (XrSpace)h;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL DestroySpace(XrSpace sp) {
    std::lock_guard<std::mutex> lk(g_m);
    g_spaces.erase((uint64_t)sp);
    return XR_SUCCESS;
}
XrResult XRAPI_CALL LocateSpace(XrSpace sp, XrSpace, XrTime, XrSpaceLocation* loc) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_spaces.find((uint64_t)sp);
    if (it == g_spaces.end()) return XR_ERROR_HANDLE_INVALID;
    loc->pose = XrPosef{{0, 0, 0, 1}, {0, 0, 0}};
    loc->locationFlags = XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT |
                         XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
    if (it->second.action == XR_NULL_HANDLE) return XR_SUCCESS;
    auto ss = g_sessions.find((uint64_t)it->second.ses);
    if (ss == g_sessions.end()) return XR_ERROR_HANDLE_INVALID;
    const double t = Now() - ss->second.created;
    for (const std::string& bp : BoundPaths(ss->second, it->second.action, it->second.sub)) {
        std::vector<double> v;
        if (!Scripted(bp, t, v) || v.size() < 2) continue;
        // yaw about +y (right = -angle), then pitch about +x
        const double ya = -v[0] * 3.14159265358979 / 360.0, pa = v[1] * 3.14159265358979 / 360.0;
        const double qy[4] = { 0, std::sin(ya), 0, std::cos(ya) }, qx[4] = { std::sin(pa), 0, 0, std::cos(pa) };
        loc->pose.orientation.x = (float)(qy[3] * qx[0] + qy[0] * qx[3] + qy[1] * qx[2] - qy[2] * qx[1]);
        loc->pose.orientation.y = (float)(qy[3] * qx[1] - qy[0] * qx[2] + qy[1] * qx[3] + qy[2] * qx[0]);
        loc->pose.orientation.z = (float)(qy[3] * qx[2] + qy[0] * qx[1] - qy[1] * qx[0] + qy[2] * qx[3]);
        loc->pose.orientation.w = (float)(qy[3] * qx[3] - qy[0] * qx[0] - qy[1] * qx[1] - qy[2] * qx[2]);
    }
    return XR_SUCCESS;
}
XrResult XRAPI_CALL CreateActionSet(XrInstance, const XrActionSetCreateInfo* ci, XrActionSet* s) {
    std::lock_guard<std::mutex> lk(g_m);
    const uint64_t h = g_next++;
    g_sets[h] = ActionSet{ ci->actionSetName, ci->priority };
    *s = (XrActionSet)h;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL DestroyActionSet(XrActionSet) { return XR_SUCCESS; }
XrResult XRAPI_CALL CreateAction(XrActionSet set, const XrActionCreateInfo* ci, XrAction* a) {
    std::lock_guard<std::mutex> lk(g_m);
    if (!g_sets.count((uint64_t)set)) return XR_ERROR_HANDLE_INVALID;
    if (g_sets[(uint64_t)set].attached) return XR_ERROR_ACTIONSETS_ALREADY_ATTACHED;
    for (auto& kv : g_actions) if (kv.second.set == set && kv.second.name == ci->actionName) return XR_ERROR_NAME_DUPLICATED;
    const uint64_t h = g_next++;
    Action act{ set, ci->actionType, {}, ci->actionName };
    for (uint32_t i = 0; i < ci->countSubactionPaths; ++i) act.sub.push_back(ci->subactionPaths[i]);
    g_actions[h] = act;
    *a = (XrAction)h;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL DestroyAction(XrAction) { return XR_SUCCESS; }
XrResult XRAPI_CALL SuggestBindings(XrInstance, const XrInteractionProfileSuggestedBinding* sb) {
    std::lock_guard<std::mutex> lk(g_m);
    const std::string prof = StrOf(sb->interactionProfile);
    if (g_anyAttached) { Report("suggest %s REFUSED (already attached)", prof.c_str()); return XR_ERROR_ACTIONSETS_ALREADY_ATTACHED; }
    const Prof* pr = FindProf(prof);
    if (!pr) { Report("suggest %s REFUSED (profile not supported)", prof.c_str()); return XR_ERROR_PATH_UNSUPPORTED; }
    for (uint32_t i = 0; i < sb->countSuggestedBindings; ++i) {
        const std::string b = StrOf(sb->suggestedBindings[i].binding);
        if (!ValidBinding(*pr, b)) { Report("suggest %s REFUSED (bad binding %s)", prof.c_str(), b.c_str()); return XR_ERROR_PATH_UNSUPPORTED; }
        if (!g_actions.count((uint64_t)sb->suggestedBindings[i].action)) return XR_ERROR_HANDLE_INVALID;
    }
    // VRC_MOCK_REFUSE_MERGED = a profile: any suggestion for it that carries the layer's actions is refused (the restore path)
    char refuse[256] = "";
    if (GetEnvironmentVariableA("VRC_MOCK_REFUSE_MERGED", refuse, sizeof(refuse)) && prof == refuse) {
        for (uint32_t i = 0; i < sb->countSuggestedBindings; ++i)
            if (g_sets[(uint64_t)g_actions[(uint64_t)sb->suggestedBindings[i].action].set].name == "ets2_ats_vr_controls") {
                Report("suggest %s REFUSED (test: merged)", prof.c_str());
                return XR_ERROR_PATH_UNSUPPORTED;
            }
    }
    g_suggest[sb->interactionProfile].assign(sb->suggestedBindings, sb->suggestedBindings + sb->countSuggestedBindings);
    std::string list;
    for (uint32_t i = 0; i < sb->countSuggestedBindings; ++i)
        list += " " + g_actions[(uint64_t)sb->suggestedBindings[i].action].name + "=" + StrOf(sb->suggestedBindings[i].binding);
    Report("suggest %s %u:%s", prof.c_str(), sb->countSuggestedBindings, list.c_str());
    return XR_SUCCESS;
}
XrResult XRAPI_CALL AttachSessionActionSets(XrSession s, const XrSessionActionSetsAttachInfo* ai) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_sessions.find((uint64_t)s);
    if (it == g_sessions.end()) return XR_ERROR_HANDLE_INVALID;
    if (it->second.attached) return XR_ERROR_ACTIONSETS_ALREADY_ATTACHED;
    std::string names;
    for (uint32_t i = 0; i < ai->countActionSets; ++i) {
        auto st = g_sets.find((uint64_t)ai->actionSets[i]);
        if (st == g_sets.end()) return XR_ERROR_HANDLE_INVALID;
        st->second.attached = true;
        it->second.attachedSets.insert(ai->actionSets[i]);
        names += " " + st->second.name + "(" + std::to_string(st->second.priority) + ")";
    }
    it->second.attached = true;
    g_anyAttached = true;
    const XrPath touch = PathOf("/interaction_profiles/oculus/touch_controller");
    char want[256] = "";                                  // VRC_MOCK_PROFILE: the "controllers" are this profile instead of Touch
    if (GetEnvironmentVariableA("VRC_MOCK_PROFILE", want, sizeof(want)) && g_suggest.count(PathOf(want))) it->second.profile = want;
    else if (g_suggest.count(touch)) it->second.profile = StrOf(touch);
    else if (!g_suggest.empty()) it->second.profile = StrOf(g_suggest.begin()->first);
    Report("attach%s", names.c_str());
    Report("profile %s", it->second.profile.empty() ? "none" : it->second.profile.c_str());
    return XR_SUCCESS;
}
XrResult XRAPI_CALL SyncActions(XrSession s, const XrActionsSyncInfo* si) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_sessions.find((uint64_t)s);
    if (it == g_sessions.end()) return XR_ERROR_HANDLE_INVALID;
    Session& ss = it->second;
    for (uint32_t i = 0; i < si->countActiveActionSets; ++i) {
        auto st = g_sets.find((uint64_t)si->activeActionSets[i].actionSet);
        if (st == g_sets.end()) return XR_ERROR_HANDLE_INVALID;
        if (!st->second.attached || !ss.attached) return XR_ERROR_ACTIONSET_NOT_ATTACHED;
    }
    ss.activeSets.clear();
    for (uint32_t i = 0; i < si->countActiveActionSets; ++i) ss.activeSets.insert(si->activeActionSets[i].actionSet);
    ++ss.syncs;
    for (XrActionSet a : ss.attachedSets)
        if (g_sets[(uint64_t)a].name == "input" && !ss.activeSets.count(a)) { ++ss.soloSyncs; break; }
    ss.states.clear();
    if (!ss.focused) return XR_SESSION_NOT_FOCUSED;
    const double t = Now() - ss.created;
    for (auto& kv : g_actions) {
        const XrAction a = (XrAction)kv.first;
        if (!ss.activeSets.count(kv.second.set)) continue;
        std::vector<XrPath> subs = kv.second.sub;
        subs.push_back(XR_NULL_PATH);
        for (XrPath sub : subs) {
            State st;
            for (const std::string& bp : BoundPaths(ss, a, sub)) {
                st.active = true;
                std::vector<double> v;
                if (!Scripted(bp, t, v) || v.empty()) continue;
                if (kv.second.type == XR_ACTION_TYPE_BOOLEAN_INPUT) st.b = st.b || v[0] >= 0.5;
                else if (kv.second.type == XR_ACTION_TYPE_FLOAT_INPUT) st.f = (float)(v[0] > st.f ? v[0] : st.f);
                else if (kv.second.type == XR_ACTION_TYPE_VECTOR2F_INPUT && v.size() >= 2) st.v = XrVector2f{(float)v[0], (float)v[1]};
            }
            ss.states[{a, sub}] = st;
        }
    }
    return XR_SUCCESS;
}
template <class T> XrResult GetState(XrSession s, const XrActionStateGetInfo* gi, T* out, XrActionType type) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_sessions.find((uint64_t)s);
    auto ac = g_actions.find((uint64_t)gi->action);
    if (it == g_sessions.end() || ac == g_actions.end()) return XR_ERROR_HANDLE_INVALID;
    if (ac->second.type != type) return XR_ERROR_ACTION_TYPE_MISMATCH;
    if (!g_sets[(uint64_t)ac->second.set].attached || !it->second.attached) return XR_ERROR_ACTIONSET_NOT_ATTACHED;
    auto st = it->second.states.find({gi->action, gi->subactionPath});
    const State s0 = st == it->second.states.end() ? State() : st->second;
    out->isActive = s0.active ? XR_TRUE : XR_FALSE;
    out->changedSinceLastSync = XR_FALSE;
    out->lastChangeTime = 0;
    if constexpr (std::is_same<T, XrActionStateBoolean>::value) out->currentState = s0.b ? XR_TRUE : XR_FALSE;
    else if constexpr (std::is_same<T, XrActionStateFloat>::value) out->currentState = s0.f;
    else out->currentState = s0.v;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL GetActionStateBoolean(XrSession s, const XrActionStateGetInfo* gi, XrActionStateBoolean* o) {
    return GetState(s, gi, o, XR_ACTION_TYPE_BOOLEAN_INPUT);
}
XrResult XRAPI_CALL GetActionStateFloat(XrSession s, const XrActionStateGetInfo* gi, XrActionStateFloat* o) {
    return GetState(s, gi, o, XR_ACTION_TYPE_FLOAT_INPUT);
}
XrResult XRAPI_CALL GetActionStateVector2f(XrSession s, const XrActionStateGetInfo* gi, XrActionStateVector2f* o) {
    return GetState(s, gi, o, XR_ACTION_TYPE_VECTOR2F_INPUT);
}
XrResult XRAPI_CALL GetActionStatePose(XrSession, const XrActionStateGetInfo*, XrActionStatePose* o) { o->isActive = XR_TRUE; return XR_SUCCESS; }
XrResult XRAPI_CALL GetCurrentInteractionProfile(XrSession s, XrPath, XrInteractionProfileState* o) {
    std::lock_guard<std::mutex> lk(g_m);
    auto it = g_sessions.find((uint64_t)s);
    if (it == g_sessions.end()) return XR_ERROR_HANDLE_INVALID;
    o->interactionProfile = it->second.profile.empty() ? XR_NULL_PATH : PathOf(it->second.profile);
    return XR_SUCCESS;
}

XrResult XRAPI_CALL GetInstanceProcAddr(XrInstance, const char* name, PFN_xrVoidFunction* fn);
#define E(n, f) { n, (PFN_xrVoidFunction)f }
const struct { const char* name; PFN_xrVoidFunction fn; } kFns[] = {
    E("xrGetInstanceProcAddr", GetInstanceProcAddr), E("xrEnumerateInstanceExtensionProperties", EnumerateInstanceExtensionProperties),
    E("xrCreateInstance", CreateInstance), E("xrDestroyInstance", DestroyInstance), E("xrGetInstanceProperties", GetInstanceProperties),
    E("xrGetSystem", GetSystem), E("xrStringToPath", StringToPath), E("xrPathToString", PathToString), E("xrCreateSession", CreateSession),
    E("xrDestroySession", DestroySession), E("xrBeginSession", BeginSession), E("xrEndSession", EndSession),
    E("xrRequestExitSession", RequestExitSession), E("xrPollEvent", PollEvent), E("xrWaitFrame", WaitFrame), E("xrBeginFrame", BeginFrame),
    E("xrEndFrame", EndFrame), E("xrCreateReferenceSpace", CreateReferenceSpace), E("xrCreateActionSpace", CreateActionSpace),
    E("xrDestroySpace", DestroySpace), E("xrLocateSpace", LocateSpace), E("xrCreateActionSet", CreateActionSet),
    E("xrDestroyActionSet", DestroyActionSet), E("xrCreateAction", CreateAction), E("xrDestroyAction", DestroyAction),
    E("xrSuggestInteractionProfileBindings", SuggestBindings), E("xrAttachSessionActionSets", AttachSessionActionSets),
    E("xrSyncActions", SyncActions), E("xrGetActionStateBoolean", GetActionStateBoolean), E("xrGetActionStateFloat", GetActionStateFloat),
    E("xrGetActionStateVector2f", GetActionStateVector2f), E("xrGetActionStatePose", GetActionStatePose),
    E("xrGetCurrentInteractionProfile", GetCurrentInteractionProfile),
    E("xrEnumerateSwapchainFormats", EnumerateSwapchainFormats), E("xrCreateSwapchain", CreateSwapchain),
    E("xrDestroySwapchain", DestroySwapchain), E("xrEnumerateSwapchainImages", EnumerateSwapchainImages),
    E("xrAcquireSwapchainImage", AcquireSwapchainImage), E("xrWaitSwapchainImage", WaitSwapchainImage),
    E("xrReleaseSwapchainImage", ReleaseSwapchainImage),
};
#undef E
XrResult XRAPI_CALL GetInstanceProcAddr(XrInstance, const char* name, PFN_xrVoidFunction* fn) {
    if (GetEnvironmentVariableA("VRC_MOCK_TRACE", nullptr, 0)) fprintf(stderr, "mock gipa %s\n", name);
    for (const auto& f : kFns) if (strcmp(name, f.name) == 0) { *fn = f.fn; return XR_SUCCESS; }
    *fn = nullptr;
    return XR_ERROR_FUNCTION_UNSUPPORTED;
}

}  // namespace

extern "C" __declspec(dllexport) XrResult XRAPI_CALL xrNegotiateLoaderRuntimeInterface(const XrNegotiateLoaderInfo* li,
                                                                                       XrNegotiateRuntimeRequest* rq) {
    if (!li || !rq || li->structType != XR_LOADER_INTERFACE_STRUCT_LOADER_INFO || rq->structType != XR_LOADER_INTERFACE_STRUCT_RUNTIME_REQUEST)
        return XR_ERROR_INITIALIZATION_FAILED;
    rq->runtimeInterfaceVersion = XR_CURRENT_LOADER_RUNTIME_VERSION;
    rq->runtimeApiVersion = XR_MAKE_VERSION(1, 0, 0) > li->maxApiVersion ? li->maxApiVersion : XR_MAKE_VERSION(1, 0, 0);
    if (rq->runtimeApiVersion < li->minApiVersion) rq->runtimeApiVersion = li->minApiVersion;
    rq->getInstanceProcAddr = GetInstanceProcAddr;
    return XR_SUCCESS;
}
