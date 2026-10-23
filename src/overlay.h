// overlay -- the VR menu's pictures on screen: OpenXR QUAD layers appended to the game's xrEndFrame (on top of its picture),
// one swapchain per picture (menu, keyboard, pointer dot). The game's own layers and graphics are never changed.
//   * Only for a game whose session uses D3D11 (ETS2 / ATS do): the device comes from the game's xrCreateSession
//     (XrGraphicsBindingD3D11KHR). Any other graphics API: no menu pictures (logged once), the controls work as before.
//   * The pictures come finished (vr_menu.h Painter, a background thread): already at the size they have in the headset (a
//     small panel shimmered when the compositor squeezed a big picture -- Virtual Desktop ignored mipmaps), in the swapchain's
//     channel order (Rgba). So the game's xrEndFrame only copies: the changed area (QuadIn::dirty, e.g. two keys under a moving
//     pointer) via UpdateSubresource with a box on the device's immediate context -- into each of the swapchain's images (they
//     rotate) the area changed since that image was last written (Sc::pending). Nothing changed: no call at all; the runtime
//     keeps showing the last released image. Each swapchain has the panel's FULL picture size (QuadIn::cw / ch) and a smaller
//     picture uses its top-left part (the layer's imageRect): a new shrink step costs no new swapchain in a frame (one did,
//     ~5 ms on Virtual Desktop); only another full size (the keyboard full <-> compact) makes a new one. One level, no mips.
//   * Any failure (create / acquire / wait / release, or the runtime refusing the frame with our layers): the pictures are off
//     for this session (logged); PadInput then re-sends the game's frame unchanged.
#pragma once
#include <array>
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
    int             pw = 0, ph = 0;    // pixels (the picture)
    int             cw = 0, ch = 0;    // the swapchain's size (the largest picture of this slot; 0 = pw / ph)
    const uint32_t* px = nullptr;      // premultiplied, top-down, the swapchain's channel order (Rgba)
    uint64_t        version = 0;       // uploaded again when it changes
    int             dirty[4] = {};     // x0, y0, x1, y1 of px changed since the last call (empty: x1 <= x0)
};

class Overlay {
public:
    static constexpr int kSlots = 3;
    Overlay(XrSession s, ID3D11Device* dev, const OverlayDispatch& d);   // AddRefs the device
    ~Overlay();                       // destroys the swapchains (unless Abandon) and releases the device
    void Abandon() { abandoned_ = true; }   // the session / instance is gone already: no OpenXR calls in the destructor
    bool Dead() const { return dead_; }
    void Kill(const char* why);      // pictures off for this session
    bool Rgba();                     // the swapchains are R8G8B8A8 (else B8G8R8A8): the pictures' channel order
    // At the game's xrEndFrame: upload what changed, then the layers to append (valid until the next call). submit = false:
    // only make the swapchains and upload (ahead of time, while the panels are hidden), no layers.
    void Frame(const std::vector<QuadIn>& quads, XrSpace space, std::vector<const XrCompositionLayerBaseHeader*>& out,
               bool submit = true);

private:
    struct Sc {
        XrSwapchain sc = XR_NULL_HANDLE;
        int w = 0, h = 0;
        std::vector<ID3D11Texture2D*> img;
        std::vector<std::array<int, 4>> pending; // per swapchain image: the area not yet written into it
        int iw = 0, ih = 0;            // the picture in it now (top-left part)
        uint64_t uploaded = ~0ull;
        bool released = false;         // an image was released once (it may be shown)
    };
    bool Format();
    bool Create(int slot, int w, int h);
    bool Upload(int slot, const QuadIn& q);

    XrSession       s_;
    ID3D11Device*   dev_ = nullptr;
    OverlayDispatch d_;
    int64_t         format_ = 0;
    bool            dead_ = false, abandoned_ = false, formatTried_ = false;
    Sc              sc_[kSlots];
    std::vector<XrCompositionLayerQuad> layers_;

    uint64_t        frames_ = 0;
};

}  // namespace ck
