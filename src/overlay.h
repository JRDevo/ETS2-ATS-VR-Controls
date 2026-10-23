// overlay -- the VR menu's pictures on screen: OpenXR QUAD layers appended to the game's xrEndFrame (on top of its picture),
// one swapchain per picture (menu, keyboard, pointer dot). The game's own layers and graphics are never changed.
//   * Only for a game whose session uses D3D11 (ETS2 / ATS do): the device comes from the game's xrCreateSession
//     (XrGraphicsBindingD3D11KHR). Any other graphics API: no menu pictures (logged once), the controls work as before.
//   * A picture is uploaded only when it changed (CPU pixels -> UpdateSubresource on the device's immediate context, at the
//     game's xrEndFrame = its render thread); the runtime keeps showing the last released image of a swapchain.
//   * Mipmaps: a panel seen smaller than its picture (a 1600 px keyboard 0.3 m wide) shimmers without them, so each swapchain
//     has a mip chain (down to ~16 px) built on the CPU at upload (2x2 box filter in linear light). A runtime that refuses
//     mipCount > 1 gets one level (as before).
//   * Any failure (create / acquire / wait / release, or the runtime refusing the frame with our layers): the pictures are off
//     for this session (logged); PadInput then re-sends the game's frame unchanged.
#pragma once
#include <cstdint>
#include <vector>
#include <openxr/openxr.h>
#include "xr_math.h"

struct ID3D11Device;
struct ID3D11Texture2D;

namespace ck {

struct OverlayDispatch {
    PFN_xrEnumerateSwapchainFormats EnumerateSwapchainFormats = nullptr;
    PFN_xrCreateSwapchain           CreateSwapchain = nullptr;
    PFN_xrDestroySwapchain          DestroySwapchain = nullptr;
    PFN_xrEnumerateSwapchainImages  EnumerateSwapchainImages = nullptr;
    PFN_xrAcquireSwapchainImage     AcquireSwapchainImage = nullptr;
    PFN_xrWaitSwapchainImage        WaitSwapchainImage = nullptr;
    PFN_xrReleaseSwapchainImage     ReleaseSwapchainImage = nullptr;
    bool Complete() const;
};

// one picture to show this frame
struct QuadIn {
    int             slot = 0;          // swapchain: 0 menu, 1 keyboard, 2 pointer dot (both dots share it)
    Pose            pose;              // in `space`
    double          w = 0, h = 0;      // metres
    int             pw = 0, ph = 0;    // pixels
    const uint32_t* px = nullptr;      // BGRA premultiplied, top-down
    uint64_t        version = 0;       // uploaded again when it changes
};

class Overlay {
public:
    static constexpr int kSlots = 3;
    Overlay(XrSession s, ID3D11Device* dev, const OverlayDispatch& d);   // AddRefs the device
    ~Overlay();                       // destroys the swapchains (unless Abandon) and releases the device
    void Abandon() { abandoned_ = true; }   // the session / instance is gone already: no OpenXR calls in the destructor
    bool Dead() const { return dead_; }
    void Kill(const char* why);      // pictures off for this session
    // At the game's xrEndFrame: upload what changed, then the layers to append (valid until the next call).
    void Frame(const std::vector<QuadIn>& quads, XrSpace space, std::vector<const XrCompositionLayerBaseHeader*>& out);

private:
    struct Sc {
        XrSwapchain sc = XR_NULL_HANDLE;
        int w = 0, h = 0;
        std::vector<ID3D11Texture2D*> img;
        uint32_t mips = 1;
        uint64_t uploaded = ~0ull;
        bool released = false;         // an image was released once (it may be shown)
    };
    bool Create(int slot, int w, int h);
    bool Upload(int slot, const QuadIn& q);

    XrSession       s_;
    ID3D11Device*   dev_ = nullptr;
    OverlayDispatch d_;
    int64_t         format_ = 0;
    bool            dead_ = false, abandoned_ = false, formatTried_ = false;
    Sc              sc_[kSlots];
    std::vector<XrCompositionLayerQuad> layers_;
    std::vector<uint32_t> conv_, mip_[2];
    uint64_t        frames_ = 0;
};

}  // namespace ck
