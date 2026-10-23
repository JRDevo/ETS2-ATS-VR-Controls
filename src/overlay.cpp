// overlay -- see overlay.h.
#include "overlay.h"
#include <windows.h>
#include <d3d11.h>
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr_platform.h>
#include <cstdio>
#include "log.h"

namespace ck {
namespace {
const char* FormatName(int64_t f) {
    return f == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ? "R8G8B8A8_UNORM_SRGB" : f == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ? "B8G8R8A8_UNORM_SRGB"
         : f == DXGI_FORMAT_R8G8B8A8_UNORM ? "R8G8B8A8_UNORM" : f == DXGI_FORMAT_B8G8R8A8_UNORM ? "B8G8R8A8_UNORM" : "?";
}
const char* const kSlotName[Overlay::kSlots] = { "menu", "keyboard", "pointer" };
}  // namespace

bool OverlayDispatch::Complete() const {
    return EnumerateSwapchainFormats && CreateSwapchain && DestroySwapchain && EnumerateSwapchainImages && AcquireSwapchainImage &&
           WaitSwapchainImage && ReleaseSwapchainImage;
}

Overlay::Overlay(XrSession s, ID3D11Device* dev, const OverlayDispatch& d) : s_(s), dev_(dev), d_(d) {
    if (dev_) dev_->AddRef();
}

Overlay::~Overlay() {
    if (!abandoned_)
        for (Sc& c : sc_) if (c.sc != XR_NULL_HANDLE) d_.DestroySwapchain(c.sc);
    if (dev_) dev_->Release();
}

void Overlay::Kill(const char* why) {
    if (dead_) return;
    dead_ = true;
    LOGW("VR menu pictures off for this session: %s (the controls still work)", why);
}

bool Overlay::Format() {
    if (formatTried_) return format_ != 0;
    formatTried_ = true;
    uint32_t n = 0;
    std::vector<int64_t> f;
    XrResult r = d_.EnumerateSwapchainFormats(s_, 0, &n, nullptr);
    if (XR_SUCCEEDED(r) && n) { f.resize(n); r = d_.EnumerateSwapchainFormats(s_, n, &n, f.data()); }
    // an _SRGB format first: the compositor reads a plain 8-bit image as LINEAR (washed out colours)
    static const int64_t kPref[4] = { DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, DXGI_FORMAT_R8G8B8A8_UNORM,
                                      DXGI_FORMAT_B8G8R8A8_UNORM };
    for (int64_t want : kPref) {
        for (int64_t x : f) if (x == want) { format_ = want; break; }
        if (format_) break;
    }
    if (XR_FAILED(r) || !format_) { Kill("no 8-bit RGBA swapchain format offered by the runtime"); return false; }
    LOGI("VR menu: swapchain format %s", FormatName(format_));
    return true;
}

bool Overlay::Rgba() {
    Format();
    return format_ == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB || format_ == DXGI_FORMAT_R8G8B8A8_UNORM;
}

bool Overlay::Create(int slot, int w, int h) {
    if (!Format()) return false;
    Sc& c = sc_[slot];
    XrSwapchainCreateInfo ci{XR_TYPE_SWAPCHAIN_CREATE_INFO};
    ci.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
    ci.format = format_;
    ci.sampleCount = 1;
    ci.width = (uint32_t)w;
    ci.height = (uint32_t)h;
    ci.faceCount = 1;
    ci.arraySize = 1;
    ci.mipCount = 1;                          // (the picture already has its size in the headset: overlay.h)
    XrResult r = d_.CreateSwapchain(s_, &ci, &c.sc);
    if (XR_FAILED(r)) {                       // some runtimes refuse TRANSFER_DST: without it (UpdateSubresource needs no flag)
        ci.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        r = d_.CreateSwapchain(s_, &ci, &c.sc);
    }
    if (XR_FAILED(r)) {
        c.sc = XR_NULL_HANDLE;
        char b[96];
        snprintf(b, sizeof(b), "xrCreateSwapchain (%s %dx%d) failed (%d)", kSlotName[slot], w, h, (int)r);
        Kill(b);
        return false;
    }
    uint32_t n = 0;
    r = d_.EnumerateSwapchainImages(c.sc, 0, &n, nullptr);
    std::vector<XrSwapchainImageD3D11KHR> im(n, XrSwapchainImageD3D11KHR{XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR});
    if (XR_SUCCEEDED(r) && n)
        r = d_.EnumerateSwapchainImages(c.sc, n, &n, reinterpret_cast<XrSwapchainImageBaseHeader*>(im.data()));
    if (XR_FAILED(r) || !n || !im[0].texture) { Kill("xrEnumerateSwapchainImages failed"); return false; }
    c.w = w;
    c.h = h;
    for (const auto& i : im) c.img.push_back(i.texture);
    c.pending.assign(c.img.size(), std::array<int, 4>{ 0, 0, w, h });   // every image: all of it still to write
    LOGI("VR menu: %s swapchain %dx%d (%u images)", kSlotName[slot], w, h, n);
    return true;
}

bool Overlay::Upload(int slot, const QuadIn& q) {
    Sc& c = sc_[slot];
    // the changed area goes into every image's "still to write"; the next image gets its own (they rotate). A picture of
    // another size (a shrink step): all of it, in every image
    int all[4] = { 0, 0, q.pw, q.ph };
    const int* d = q.dirty;
    if (c.iw != q.pw || c.ih != q.ph) { c.iw = q.pw; c.ih = q.ph; d = all; }
    if (d[2] > d[0] && d[3] > d[1])
        for (auto& pd : c.pending) {
            if (pd[2] <= pd[0] || pd[3] <= pd[1]) { pd = { d[0], d[1], d[2], d[3] }; continue; }
            pd[0] = pd[0] < d[0] ? pd[0] : d[0]; pd[1] = pd[1] < d[1] ? pd[1] : d[1];
            pd[2] = pd[2] > d[2] ? pd[2] : d[2]; pd[3] = pd[3] > d[3] ? pd[3] : d[3];
        }
    c.uploaded = q.version;
    bool any = false;
    for (const auto& pd : c.pending) any = any || (pd[2] > pd[0] && pd[3] > pd[1]);
    if (!any && c.released) return true;               // nothing changed: nothing to send
    uint32_t idx = 0;
    XrSwapchainImageAcquireInfo ai{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
    XrResult r = d_.AcquireSwapchainImage(c.sc, &ai, &idx);
    if (XR_FAILED(r)) { Kill("xrAcquireSwapchainImage failed"); return false; }
    XrSwapchainImageWaitInfo wi{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
    wi.timeout = 100000000;            // 100 ms
    r = d_.WaitSwapchainImage(c.sc, &wi);
    if (r != XR_SUCCESS) { Kill("xrWaitSwapchainImage failed or timed out"); return false; }   // (an image we may not release)
    if (idx < c.img.size() && c.img[idx]) {
        std::array<int, 4>& pd = c.pending[idx];
        pd[2] = pd[2] > q.pw ? q.pw : pd[2];             // (only the picture: the swapchain may be bigger)
        pd[3] = pd[3] > q.ph ? q.ph : pd[3];
        if (pd[2] > pd[0] && pd[3] > pd[1]) {
            ID3D11DeviceContext* ctx = nullptr;
            dev_->GetImmediateContext(&ctx);
            if (ctx) {
                const D3D11_BOX box{ (UINT)pd[0], (UINT)pd[1], 0, (UINT)pd[2], (UINT)pd[3], 1 };
                ctx->UpdateSubresource(c.img[idx], 0, &box, q.px + (size_t)pd[1] * q.pw + pd[0], (UINT)q.pw * 4, 0);
                ctx->Release();
            }
        }
        pd = { 0, 0, 0, 0 };
    }
    XrSwapchainImageReleaseInfo ri{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
    r = d_.ReleaseSwapchainImage(c.sc, &ri);
    if (XR_FAILED(r)) { Kill("xrReleaseSwapchainImage failed"); return false; }
    c.released = true;
    return true;
}

void Overlay::Frame(const std::vector<QuadIn>& quads, XrSpace space, std::vector<const XrCompositionLayerBaseHeader*>& out,
                    bool submit) {
    out.clear();
    layers_.clear();
    if (dead_ || !dev_ || space == XR_NULL_HANDLE) return;
    layers_.reserve(quads.size());
    for (const QuadIn& q : quads) {
        if (q.slot < 0 || q.slot >= kSlots || !q.px || q.pw < 1 || q.ph < 1) continue;
        Sc& c = sc_[q.slot];
        const int cw = q.cw > 0 ? q.cw : q.pw, ch = q.ch > 0 ? q.ch : q.ph;
        if (q.pw > cw || q.ph > ch) continue;
        if (c.sc != XR_NULL_HANDLE && (c.w != cw || c.h != ch)) {   // another full size (the keyboard full <-> compact)
            d_.DestroySwapchain(c.sc);
            c = Sc();
        }
        if (c.sc == XR_NULL_HANDLE && !Create(q.slot, cw, ch)) return;
        if ((c.uploaded != q.version || !c.released || c.iw != q.pw || c.ih != q.ph) && !Upload(q.slot, q)) return;
        if (!c.released || !submit) continue;
        XrCompositionLayerQuad l{XR_TYPE_COMPOSITION_LAYER_QUAD};
        l.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;   // premultiplied alpha
        l.space = space;
        l.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
        l.subImage.swapchain = c.sc;
        l.subImage.imageRect.offset = { 0, 0 };
        l.subImage.imageRect.extent = { c.iw, c.ih };   // (the picture: the top-left part)
        l.subImage.imageArrayIndex = 0;
        l.pose.orientation = { (float)q.pose.q.x, (float)q.pose.q.y, (float)q.pose.q.z, (float)q.pose.q.w };
        l.pose.position = { (float)q.pose.p.x, (float)q.pose.p.y, (float)q.pose.p.z };
        l.size = { (float)q.w, (float)q.h };
        layers_.push_back(l);
    }
    for (const XrCompositionLayerQuad& l : layers_) out.push_back(reinterpret_cast<const XrCompositionLayerBaseHeader*>(&l));
    if (!out.empty() && frames_++ == 0) LOGI("VR menu: first frame with the menu's %zu quad layer(s)", out.size());
}

}  // namespace ck
