// overlay -- see overlay.h.
#include "overlay.h"
#include <windows.h>
#include <d3d11.h>
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr_platform.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include "log.h"

namespace ck {
namespace {
const char* FormatName(int64_t f) {
    return f == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB ? "R8G8B8A8_UNORM_SRGB" : f == DXGI_FORMAT_B8G8R8A8_UNORM_SRGB ? "B8G8R8A8_UNORM_SRGB"
         : f == DXGI_FORMAT_R8G8B8A8_UNORM ? "R8G8B8A8_UNORM" : f == DXGI_FORMAT_B8G8R8A8_UNORM ? "B8G8R8A8_UNORM" : "?";
}
const char* const kSlotName[Overlay::kSlots] = { "menu", "keyboard", "pointer" };

// sRGB <-> linear for the mip filter (8-bit in, 8-bit out; a 4096-step table back)
const float* ToLin() {
    static float t[256];
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 256; ++i) { const double c = i / 255.0; t[i] = (float)(c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4)); }
        init = true;
    }
    return t;
}
const uint8_t* ToSrgb() {
    static uint8_t t[4096];
    static bool init = false;
    if (!init) {
        for (int i = 0; i < 4096; ++i) {
            const double l = i / 4095.0, c = l <= 0.0031308 ? l * 12.92 : 1.055 * std::pow(l, 1 / 2.4) - 0.055;
            t[i] = (uint8_t)(c * 255.0 + 0.5);
        }
        init = true;
    }
    return t;
}
// one mip level down: 2x2 box (the last row / column of an odd size counts twice), colour in linear light, alpha linear
void Half(const uint32_t* in, int w, int h, std::vector<uint32_t>& out, int* ow, int* oh) {
    const int nw = w > 1 ? w / 2 : 1, nh = h > 1 ? h / 2 : 1;
    out.resize((size_t)nw * nh);
    const float* lin = ToLin();
    const uint8_t* srgb = ToSrgb();
    for (int y = 0; y < nh; ++y)
        for (int x = 0; x < nw; ++x) {
            const int x0 = x * 2, y0 = y * 2, x1 = x0 + 1 < w ? x0 + 1 : x0, y1 = y0 + 1 < h ? y0 + 1 : y0;
            const uint32_t p[4] = { in[(size_t)y0 * w + x0], in[(size_t)y0 * w + x1], in[(size_t)y1 * w + x0], in[(size_t)y1 * w + x1] };
            float c[3] = {};
            uint32_t a = 0;
            for (uint32_t q : p) {
                c[0] += lin[q & 0xFF]; c[1] += lin[(q >> 8) & 0xFF]; c[2] += lin[(q >> 16) & 0xFF];
                a += q >> 24;
            }
            uint32_t r = (uint32_t)((a + 2) / 4) << 24;
            for (int k = 0; k < 3; ++k) r |= (uint32_t)srgb[(int)(c[k] * 0.25f * 4095.0f + 0.5f)] << (8 * k);
            out[(size_t)y * nw + x] = r;
        }
    *ow = nw;
    *oh = nh;
}
// The area filter (each target pixel = the average of the source pixels it covers, colour in linear light). Weights per
// axis are built once per size pair.
struct Axis {
    std::vector<int> first, count, off;
    std::vector<float> w;
};
void MakeAxis(int src, int dst, Axis& a) {
    a.first.assign((size_t)dst, 0); a.count.assign((size_t)dst, 0); a.off.assign((size_t)dst, 0); a.w.clear();
    const double k = (double)src / dst;               // source pixels per target pixel (>= 1)
    for (int j = 0; j < dst; ++j) {
        const double s0 = j * k, s1 = (j + 1) * k;
        int i0 = (int)s0, i1 = (int)std::ceil(s1) - 1;
        if (i1 >= src) i1 = src - 1;
        if (i1 < i0) i1 = i0;
        a.first[(size_t)j] = i0;
        a.count[(size_t)j] = i1 - i0 + 1;
        a.off[(size_t)j] = (int)a.w.size();
        for (int i = i0; i <= i1; ++i) {
            const double lo = i > s0 ? i : s0, hi = i + 1 < s1 ? i + 1 : s1;
            a.w.push_back((float)((hi - lo) / k));
        }
    }
}
// target pixels [x0, x1) x [y0, y1) of `dst` (dw wide) from `src` (sw x sh)
void Shrink(const uint32_t* src, int sw, int sh, uint32_t* dst, int dw, int dh, int x0, int y0, int x1, int y1) {
    static Axis ax, ay;
    static int aw[4] = {};
    if (aw[0] != sw || aw[1] != dw) { MakeAxis(sw, dw, ax); aw[0] = sw; aw[1] = dw; }
    if (aw[2] != sh || aw[3] != dh) { MakeAxis(sh, dh, ay); aw[2] = sh; aw[3] = dh; }
    const float* lin = ToLin();
    const uint8_t* srgb = ToSrgb();
    std::vector<float> row((size_t)(x1 - x0) * 4), acc((size_t)(x1 - x0) * 4);
    for (int y = y0; y < y1; ++y) {
        std::fill(acc.begin(), acc.end(), 0.0f);
        for (int r = 0; r < ay.count[(size_t)y]; ++r) {
            const int sy = ay.first[(size_t)y] + r;
            const float wy = ay.w[(size_t)(ay.off[(size_t)y] + r)];
            const uint32_t* sr = src + (size_t)sy * sw;
            for (int x = x0; x < x1; ++x) {
                float c0 = 0, c1 = 0, c2 = 0, ca = 0;
                const int f = ax.first[(size_t)x], n = ax.count[(size_t)x];
                const float* wx = &ax.w[(size_t)ax.off[(size_t)x]];
                for (int i = 0; i < n; ++i) {
                    const uint32_t q = sr[f + i];
                    c0 += wx[i] * lin[q & 0xFF]; c1 += wx[i] * lin[(q >> 8) & 0xFF]; c2 += wx[i] * lin[(q >> 16) & 0xFF];
                    ca += wx[i] * (float)(q >> 24);
                }
                float* o = &acc[(size_t)(x - x0) * 4];
                o[0] += wy * c0; o[1] += wy * c1; o[2] += wy * c2; o[3] += wy * ca;
            }
        }
        uint32_t* dr = dst + (size_t)y * dw;
        for (int x = x0; x < x1; ++x) {
            const float* o = &acc[(size_t)(x - x0) * 4];
            auto enc = [&](float v) { const int i = (int)(v * 4095.0f + 0.5f); return (uint32_t)srgb[i < 0 ? 0 : i > 4095 ? 4095 : i]; };
            const int a = (int)(o[3] + 0.5f);
            dr[x] = ((uint32_t)(a < 0 ? 0 : a > 255 ? 255 : a) << 24) | (enc(o[2]) << 16) | (enc(o[1]) << 8) | enc(o[0]);
        }
    }
    (void)row;
}
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

bool Overlay::Create(int slot, int w, int h) {
    if (!formatTried_) {
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
    }
    if (!format_) return false;
    Sc& c = sc_[slot];
    XrSwapchainCreateInfo ci{XR_TYPE_SWAPCHAIN_CREATE_INFO};
    ci.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_TRANSFER_DST_BIT;
    ci.format = format_;
    ci.sampleCount = 1;
    ci.width = (uint32_t)w;
    ci.height = (uint32_t)h;
    ci.faceCount = 1;
    ci.arraySize = 1;
    uint32_t mips = 1;                        // a mip chain down to ~16 px (see overlay.h)
    for (int mw = w, mh = h; mips < 8 && mw >= 32 && mh >= 32; mw /= 2, mh /= 2) ++mips;
    ci.mipCount = mips;
    XrResult r = d_.CreateSwapchain(s_, &ci, &c.sc);
    if (XR_FAILED(r)) {                       // some runtimes refuse TRANSFER_DST: without it (UpdateSubresource needs no flag)
        ci.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT | XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
        r = d_.CreateSwapchain(s_, &ci, &c.sc);
    }
    if (XR_FAILED(r) && ci.mipCount > 1) {   // no mipmaps on this runtime: one level, as before
        LOGW("VR menu: the runtime refuses a %u-level mip chain (%d) -- one level (small panels may shimmer)", ci.mipCount, (int)r);
        ci.mipCount = 1;
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
    c.mips = ci.mipCount;
    for (const auto& i : im) c.img.push_back(i.texture);
    LOGI("VR menu: %s swapchain %dx%d (%u images, %u mip levels)", kSlotName[slot], w, h, n, c.mips);
    return true;
}

bool Overlay::Upload(int slot, const QuadIn& q, bool full) {
    Sc& c = sc_[slot];
    // the picture at the swapchain's size: shrunk (the changed area only, unless `full`), or as it is
    const uint32_t* pic = q.px;
    if (c.w != q.pw || c.h != q.ph) {
        if (c.shrunk.size() != (size_t)c.w * c.h || c.srcW != q.pw || c.srcH != q.ph) { c.shrunk.assign((size_t)c.w * c.h, 0); full = true; }
        c.srcW = q.pw;
        c.srcH = q.ph;
        int x0 = 0, y0 = 0, x1 = c.w, y1 = c.h;
        if (!full) {
            if (q.dirty[2] <= q.dirty[0] || q.dirty[3] <= q.dirty[1]) { x1 = y1 = 0; }
            else {
                const double kx = (double)c.w / q.pw, ky = (double)c.h / q.ph;
                x0 = (int)(q.dirty[0] * kx) - 1; y0 = (int)(q.dirty[1] * ky) - 1;
                x1 = (int)std::ceil(q.dirty[2] * kx) + 1; y1 = (int)std::ceil(q.dirty[3] * ky) + 1;
                x0 = x0 < 0 ? 0 : x0; y0 = y0 < 0 ? 0 : y0; x1 = x1 > c.w ? c.w : x1; y1 = y1 > c.h ? c.h : y1;
            }
        }
        if (x1 > x0 && y1 > y0) Shrink(q.px, q.pw, q.ph, c.shrunk.data(), c.w, c.h, x0, y0, x1, y1);
        pic = c.shrunk.data();
    }
    uint32_t idx = 0;
    XrSwapchainImageAcquireInfo ai{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
    XrResult r = d_.AcquireSwapchainImage(c.sc, &ai, &idx);
    if (XR_FAILED(r)) { Kill("xrAcquireSwapchainImage failed"); return false; }
    XrSwapchainImageWaitInfo wi{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
    wi.timeout = 100000000;            // 100 ms
    r = d_.WaitSwapchainImage(c.sc, &wi);
    if (r != XR_SUCCESS) { Kill("xrWaitSwapchainImage failed or timed out"); return false; }   // (an image we may not release)
    const uint32_t* src = pic;
    if (format_ == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB || format_ == DXGI_FORMAT_R8G8B8A8_UNORM) {   // BGRA -> RGBA
        conv_.assign(pic, pic + (size_t)c.w * c.h);
        for (uint32_t& p : conv_) p = (p & 0xFF00FF00u) | ((p >> 16) & 0xFFu) | ((p & 0xFFu) << 16);
        src = conv_.data();
    }
    if (idx < c.img.size() && c.img[idx]) {
        ID3D11DeviceContext* ctx = nullptr;
        dev_->GetImmediateContext(&ctx);
        if (ctx) {
            ctx->UpdateSubresource(c.img[idx], 0, nullptr, src, (UINT)c.w * 4, 0);
            const uint32_t* lv = src;
            int lw = c.w, lh = c.h;
            for (uint32_t m = 1; m < c.mips; ++m) {   // the smaller copies (mip levels), each from the one before
                std::vector<uint32_t>& dst = mip_[m & 1];
                Half(lv, lw, lh, dst, &lw, &lh);
                ctx->UpdateSubresource(c.img[idx], m, nullptr, dst.data(), (UINT)lw * 4, 0);
                lv = dst.data();
            }
            ctx->Release();
        }
    }
    XrSwapchainImageReleaseInfo ri{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
    r = d_.ReleaseSwapchainImage(c.sc, &ri);
    if (XR_FAILED(r)) { Kill("xrReleaseSwapchainImage failed"); return false; }
    c.released = true;
    c.uploaded = q.version;
    return true;
}

void Overlay::Frame(const std::vector<QuadIn>& quads, XrSpace space, std::vector<const XrCompositionLayerBaseHeader*>& out) {
    out.clear();
    layers_.clear();
    if (dead_ || !dev_ || space == XR_NULL_HANDLE) return;
    layers_.reserve(quads.size());
    for (const QuadIn& q : quads) {
        if (q.slot < 0 || q.slot >= kSlots || !q.px || q.pw < 1 || q.ph < 1) continue;
        Sc& c = sc_[q.slot];
        // the size it is shown at: shrunk to its size in the headset, in 10 % steps (each step a new swapchain), changed only
        // when the wanted size leaves -20 % .. +15 % of the current one (walking around does not keep resizing it)
        double scale = 1.0;
        if (q.wantPx > 0) {
            double want = q.wantPx / q.pw;
            want = want < 0.2 ? 0.2 : want > 1.0 ? 1.0 : want;
            scale = c.scale;
            if (scale <= 0 || want > scale * 1.15 || want < scale / 1.25) {
                scale = 1.0;
                while (scale / 1.1 >= want) scale /= 1.1;   // the smallest step that is still at least as big as wanted
            }
        }
        int tw = (int)(q.pw * scale + 0.5), th = (int)(q.ph * scale + 0.5);
        tw = tw < 16 ? 16 : tw; th = th < 16 ? 16 : th;
        bool full = false;
        if (c.sc != XR_NULL_HANDLE && (c.w != tw || c.h != th)) {   // a new size (shrink step, or the keyboard full <-> compact)
            d_.DestroySwapchain(c.sc);
            c = Sc();
        }
        if (c.sc == XR_NULL_HANDLE) {
            if (!Create(q.slot, tw, th)) return;
            full = true;
            if (tw != q.pw) LOGI("VR menu: %s shown at %d x %d (%.0f %% of its %d x %d picture: its size in the headset)",
                                 kSlotName[q.slot], tw, th, scale * 100.0, q.pw, q.ph);
        }
        c.scale = scale;
        if ((c.uploaded != q.version || full) && !Upload(q.slot, q, full)) return;
        if (!c.released) continue;
        XrCompositionLayerQuad l{XR_TYPE_COMPOSITION_LAYER_QUAD};
        l.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;   // premultiplied alpha
        l.space = space;
        l.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
        l.subImage.swapchain = c.sc;
        l.subImage.imageRect.offset = { 0, 0 };
        l.subImage.imageRect.extent = { c.w, c.h };
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
