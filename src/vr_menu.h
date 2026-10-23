// vr_menu -- phase 2: the mod's own VR menu and the on-screen keyboard. Two flat panels that float in the room (OpenXR quad
// layers drawn by overlay.h on top of the game's picture; the game's graphics are never touched), so they never meet the DLAA
// mod's in-picture menu (Delete): nothing here uses a keyboard key.
//   * Open / close: hold ALL menu_combo buttons (default both grips + the left stick click; NOT both stick clicks: that is VR
//     EyeCapture's record combo) for menu_hold seconds (1 s). Nothing is held back: each button does its own job until the last
//     one of the combo goes down; from then on all of them are taken out of the frame until all are let go (ComboGate).
//   * Point with either controller (its aim ray); a dot shows where. Its trigger presses what it points at. While a controller
//     points at a panel, its trigger reaches neither the game nor the trigger's mapping (PadInput hides it).
//   * Menu panel: tabs Buttons (every controller button and what it sends: point at one, then pick a key on the keyboard or a
//     mouse button / none from the bar; "Normal" or "With modifier": what a button sends while the modifier button -- chosen
//     there, default the right grip -- is held, e.g. right grip + B = right click), Mouse (stick mouse, air mouse
//     hold / toggle, speeds), Panels (sizes, bring here), Driving (the stick that steers: left / right / off, its dead zone,
//     curve, how far full stick turns the wheel, right trigger = brake).
//   * Keyboard panel: a full-size keyboard (main block, F-keys, arrows / navigation, number pad) as one flat block (no title
//     bar: its - / + / x are above the number pad), or COMPACT (Panels tab / keyboard_layout): the main block only (Esc /
//     F-keys down to Enter, Shift, Ctrl) with only its x, above Backspace (its size: Panels tab). Switching keeps the key size (the
//     panel's width in metres follows its pixel width). A key goes down on the
//     trigger and up when it is let go. Shift / Ctrl / Alt latch: they go with the next key. While a button is being set, the
//     keyboard picks its key instead (with the latched modifiers: shift+f).
//   * Move: point at a panel and hold the GRIP (or the trigger on its top bar) and move the controller: it hangs on the
//     controller and follows every turn, tilt and twist of the hand; that controller's stick up / down pushes it away / pulls
//     it closer meanwhile. While a controller
//     points at a panel its grip is the menu's (no air mouse); while it holds one, its stick too. Size: the - / + on the bar.
//     Panels tab: size and distance of each. Look tab: colour theme, see-through (opacity) of each panel, pointer smoothing.
//   * Pointer smoothing (off / low / medium / high): both aim poses go through a PoseSmoother (hand shake out, fast moves
//     follow) -- the menu's pointer AND the air mouse (the smoothed aim is written back into the frame). A hovered element
//     also keeps the pointer until it is 8 px outside it (no jumping between two keys at their edge).
//   * The keyboard is its own panel: the combo opens / closes only the menu; the keyboard stays where it is (shown or not,
//     also at the next game start: keyboard_show) until its x -- e.g. fixed at the side of the cab, always there.
//   * Light: the game's threads never draw. A background thread at low priority (Painter, vr_menu.cpp) draws the panels (GDI),
//     shrinks them to the size they have in the headset (Output: an area filter in linear light, 10 % steps with hysteresis;
//     none at 80 % or more), puts them in the swapchain's channel order and publishes them (Picture: the pixels + the area
//     changed since the overlay last took it); it also writes config.ini. The game's xrBeginFrame only lays out and posts a job
//     (the whole panel when what it shows changed -- ContentKey per panel -- else only the elements whose hover / press look
//     changed); its xrEndFrame only copies the changed area to the swapchain (overlay.h). Both panels are drawn once when the
//     game starts, so opening the menu draws nothing.
//   * Every change is written to config.ini at once (menu_place / keyboard_place in the room's LOCAL space: a recenter moves
//     them with the room).
// Pure logic + GDI drawing into memory: no OpenXR / D3D here (tests/vr_menu_test.cpp drives it with made-up frames).
#pragma once
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "config.h"
#include "pad_map.h"
#include "xr_math.h"

namespace ck {

// all buttons of a combo held for `hold` seconds. Before the last one goes down each passes through; from then on all are
// taken out of the frame until every one is let go (so the Mapper sees them released).
struct ComboGate {
    std::vector<int> btns = { VrPad::kLGrip, VrPad::kRGrip, VrPad::kLStick };
    double hold = 1.0;
    // masks the combo buttons in f while the combo is held; true = it was just held for `hold` seconds
    bool Step(VrPad::Frame& f, double now);
private:
    bool   on_ = false;              // all went down: the combo owns the buttons
    double t0_ = 0;
    bool   fired_ = false;
};
bool BtnDown(const VrPad::Frame& f, int b);   // kLTrig .. kMenu (not the stick directions)
void BtnSet(VrPad::Frame& f, int b, bool on);

class MenuCanvas;   // vr_menu.cpp: the GDI picture of one panel (kept between redraws)
class Painter;      // vr_menu.cpp: the background thread that draws / shrinks / publishes the pictures and saves config.ini

class VrMenu {
public:
    enum { kPanMenu, kPanKb, kPanCount };
    struct Elem { int kind = 0, arg = 0, x = 0, y = 0, w = 0, h = 0; std::string text, sub; bool on = false, dim = false; };
    struct Panel {
        bool     show = false;
        Pose     pose;                // LOCAL space; the quad faces +z
        double   width = 0.6;         // metres (height from the pixel aspect)
        int      pw = 0, ph = 0;      // pixels
        std::vector<Elem>     el;
        double   Height() const { return width * ph / pw; }
    };
    // a panel's picture as the overlay gets it (PictureLocked): the size it is shown at, the swapchain's channel order,
    // premultiplied, top-down; version + 1 on every change; dirty = the area changed since the overlay last took it
    struct Picture { std::vector<uint32_t> px; int w = 0, h = 0; uint64_t version = 0; int dirty[4] = {}; };

    // cfg: the start settings; path: the config.ini that changes are written to (empty = not saved)
    VrMenu(const Config& cfg, const std::wstring& path);
    ~VrMenu();

    // Every frame with fresh controller states (PadInput::BeginFrame). f: this frame's controllers in LOCAL space (changed: what
    // the menu uses is taken out before the Mapper sees it). live: input may be sent (else everything held is let go).
    // headOk / head: the head (VIEW) pose in LOCAL. Keyboard keys go to `sink`.
    void Update(VrPad::Frame& f, bool live, bool headOk, const Pose& head, double now, VrPad::Sink& sink);
    void ReleaseAll(VrPad::Sink& sink);           // let go of a held keyboard key / a held panel

    const VrPad::Config& Pad() const { return pad_; }   // the mapping in use (the menu edits it)
    bool AnyShown() const { return pan_[0].show || pan_[1].show; }
    bool HandOnPanel(int h) const { return h >= 0 && h < 2 && (hov_[h].pan >= 0 || press_[h].pan >= 0 || grab_[h].pan >= 0 || own_[h]); }
    bool Pending() const { return kbPending_; }  // the keyboard waits for the head pose to be placed
    const MenuLook& Look() const { return look_; }
    const Panel& Pan(int i) const { return pan_[i]; }
    // xrEndFrame: the size panel p is shown at, from its width in the headset (pixels; 0 = unknown) and the swapchain's channel
    // order; true = the picture already has that size (Painter works on it otherwise)
    bool Output(int p, double wantPx, bool rgba);
    std::unique_lock<std::mutex> LockPictures();  // hold it while reading / taking a picture
    Picture& PictureLocked(int p);
    void Flush();                                 // tests: wait until the painter is done
    const std::vector<uint32_t>& FullPicture(int p);   // tests: the full-size BGRA picture (after Flush)
    // the pointer dot of hand h: shown + its pose (on the panel, facing out)
    bool Cursor(int h, Pose* p) const;
    const std::vector<uint32_t>& CursorPixels() const { return cursorPx_; }
    static constexpr int kCursorPx = 64;
    static constexpr double kCursorM = 0.014;     // metres

    // tests: the world point of an element's centre (false = not on a shown panel)
    bool ElemCentre(int pan, int kind, int arg, V3* out) const;
    // tests / log: a short state line
    std::string Describe() const;
    // element kinds (public for the tests)
    enum Kind { eNone, eBar, eClose, eSmaller, eBigger, eKbBtn, eTab, eRow, ePickOut, ePickCancel, eSet, eKey, eHere, eInfo };
    void Open(bool headOk, const Pose& head);     // as the combo does (tests)
    static int KeyIndex(const char* name);        // tests: the eKey arg of a keyboard key ("f1", "shift"); -1 = none
    void Close();                                 // the menu (the keyboard stays as it is)

private:
    struct Ref { int pan = -1, kind = eNone, arg = 0; };
    struct Press { int pan = -1, kind = eNone, arg = 0; bool drag = false; Pose grab; uint16_t key = 0; uint16_t mods[3] = {}; int nm = 0; };
    void Place(int p, bool headOk, const Pose& head, bool force);
    void Layout(int p);
    void LayoutMenu();
    void LayoutKb();
    void Draw(int p);                               // the whole panel
    bool DrawChanged(int p);                        // only the elements whose hover / press look changed; false = Draw needed
    static void DrawElem(MenuCanvas& c, int theme, const Elem& e, int st);   // (on the painter's thread)
    int  ElemState(int p, const Elem& e) const;     // 1 hovered, 2 pressed
    friend class Painter;
    void Redraw(int p);
    void RedrawAll() { Redraw(kPanMenu); Redraw(kPanKb); }
    bool Hit(const VrPad::Frame& f, int h, Ref* r, double* u, double* v);
    std::string ContentKey(int p) const;            // everything panel p's look depends on but hover / press
    const Elem* Find(int p, int kind, int arg) const;
    void Activate(int h, const Ref& r, const VrPad::Frame& f, VrPad::Sink& sink);
    void Release(int h, VrPad::Sink& sink);
    void SetValue(int arg);
    void Push(int p, double metres);              // the panel along the line from the head: + = farther
    void ShowKb(bool on);
    void SetKbCompact(bool on);
    void Pick(const VrPad::Out& o);
    void Save();
    void SavePlaces();

    VrPad::Config pad_;
    ComboGate     gate_;
    std::wstring  path_;
    Panel         pan_[kPanCount];
    PanelPlace    place_[kPanCount];
    int           tab_ = 0;               // 0 Buttons, 1 Mouse, 2 Panels, 3 Look, 4 Driving
    int           pickBtn_ = -1;          // the button being set (a key / mouse button is picked next)
    bool          pickMod_ = false;       // ... its entry for while the modifier is held
    int           layer_ = 0;             // Buttons tab: 0 normal, 1 with the modifier held
    MenuLook      look_;
    bool          kbPending_ = false;     // keyboard_show without a saved place: shown at the first frame with the head
    struct Grab { int pan = -1; Pose off; };
    Grab          grab_[2];               // a panel held by the grip
    bool          gripDown_[2] = {};
    double        lastNow_ = -1;
    PoseSmoother  smooth_[2];
    bool          latch_[3] = {};         // Shift, Ctrl, Alt latched (left / right alike)
    Ref           hov_[2];
    Press         press_[2];
    bool          trigDown_[2] = {};
    bool          own_[2] = {};           // a trigger pressed on a panel stays the menu's until it is let go (also after the
                                          //   press closed the panel: no click leaks into the game)
    bool          aimOk_[2] = {};
    Pose          aim_[2];
    double        hitT_[2] = {};
    bool          lastHeadOk_ = false;
    Pose          lastHead_;
    bool          needRedraw_ = false;
    std::unique_ptr<Painter> painter_;
    double        scale_[kPanCount] = {};           // Output: the shrink step in use (0 = none yet)
    int           scaleOf_[kPanCount] = {};         // ... for this picture width
    std::string   content_[kPanCount];              // ContentKey at the last full draw
    std::vector<uint8_t> elState_[kPanCount];       // ElemState of each element as drawn
    std::vector<uint32_t> cursorPx_;
    bool          saveDue_ = false;
    std::string   status_;                // the bar's last message ("Saved", "Set: A = enter")
};

}  // namespace ck
