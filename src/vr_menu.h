// vr_menu -- phase 2: the mod's own VR menu and the on-screen keyboard. Two flat panels that float in the room (OpenXR quad
// layers drawn by overlay.h on top of the game's picture; the game's graphics are never touched), so they never meet the DLAA
// mod's in-picture menu (Delete): nothing here uses a keyboard key.
//   * Open / close: hold BOTH menu_combo buttons (default the two stick clicks) for menu_hold seconds (1 s). A short press of
//     one of them alone still does its own job (horn / hazards): it is held back up to 0.25 s, then sent (ComboGate).
//   * Point with either controller (its aim ray); a dot shows where. Its trigger presses what it points at. While a controller
//     points at a panel, its trigger reaches neither the game nor the trigger's mapping (PadInput hides it).
//   * Menu panel: tabs Buttons (every controller button and what it sends: point at one, then pick a key on the keyboard or a
//     mouse button / none from the bar), Mouse (stick mouse, air mouse hold / toggle, speeds), Panels (sizes, bring here).
//   * Keyboard panel: a full-size keyboard (main block, F-keys, arrows / navigation, number pad). A key goes down on the
//     trigger and up when it is let go. Shift / Ctrl / Alt latch: they go with the next key. While a button is being set, the
//     keyboard picks its key instead (with the latched modifiers: shift+f).
//   * Move: point at a panel's top bar, hold the trigger and move the controller (it hangs on the controller, kept level).
//     Size: the - / + on the bar. Every change is written to config.ini at once (menu_place / keyboard_place in the room's LOCAL
//     space: a recenter moves them with the room).
// Pure logic + GDI drawing into memory: no OpenXR / D3D here (tests/vr_menu_test.cpp drives it with made-up frames).
#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include "config.h"
#include "pad_map.h"
#include "xr_math.h"

namespace ck {

// both buttons of a combo held for `hold` seconds; one alone passes through (a tap at its release, a hold after 0.25 s)
struct ComboGate {
    int    a = VrPad::kLStick, b = VrPad::kRStick;
    double hold = 1.0;
    // masks the combo buttons in f as needed; true = the combo was just completed
    bool Step(VrPad::Frame& f, double now);
private:
    int    st_ = 0;                  // 0 idle, 1 one down (waiting for the other), 2 one passes through, 3 both (the combo)
    int    first_ = -1;
    double t0_ = 0;
    bool   fired_ = false;
    int    tapBtn_ = -1;
    double tapUntil_ = 0;
};
bool BtnDown(const VrPad::Frame& f, int b);   // kLTrig .. kMenu (not the stick directions)
void BtnSet(VrPad::Frame& f, int b, bool on);

class MenuCanvas;   // vr_menu.cpp: the GDI picture of one panel (kept between redraws)

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
        std::vector<uint32_t> px;     // BGRA, premultiplied, top-down
        uint64_t version = 0;         // + 1 on every redraw
        double   Height() const { return width * ph / pw; }
    };

    // cfg: the start settings; path: the config.ini that changes are written to (empty = not saved)
    VrMenu(const Config& cfg, const std::wstring& path);
    ~VrMenu();

    // Every frame with fresh controller states (PadInput::BeginFrame). f: this frame's controllers in LOCAL space (changed: what
    // the menu uses is taken out before the Mapper sees it). live: input may be sent (else everything held is let go).
    // headOk / head: the head (VIEW) pose in LOCAL. Keyboard keys go to `sink`.
    void Update(VrPad::Frame& f, bool live, bool headOk, const Pose& head, double now, VrPad::Sink& sink);
    void ReleaseAll(VrPad::Sink& sink);           // let go of a held keyboard key

    const VrPad::Config& Pad() const { return pad_; }   // the mapping in use (the menu edits it)
    bool AnyShown() const { return pan_[0].show || pan_[1].show; }
    bool HandOnPanel(int h) const { return h >= 0 && h < 2 && (hov_[h].pan >= 0 || press_[h].pan >= 0); }
    const Panel& Pan(int i) const { return pan_[i]; }
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
    void Close();

private:
    struct Ref { int pan = -1, kind = eNone, arg = 0; };
    struct Press { int pan = -1, kind = eNone, arg = 0; bool drag = false; Pose grab; uint16_t key = 0; uint16_t mods[3] = {}; int nm = 0; };
    void Place(int p, bool headOk, const Pose& head, bool force);
    void Layout(int p);
    void LayoutMenu();
    void LayoutKb();
    void Draw(int p);                               // the whole panel
    bool DrawChanged(int p);                        // only the elements whose hover / press look changed; false = Draw needed
    void DrawElem(MenuCanvas& c, const Elem& e, int st);
    int  ElemState(int p, const Elem& e) const;     // 1 hovered, 2 pressed
    void Publish(int p);
    void Redraw(int p);
    void RedrawAll() { Redraw(kPanMenu); Redraw(kPanKb); }
    bool Hit(const VrPad::Frame& f, int h, Ref* r, double* u, double* v);
    std::string ContentKey() const;                 // everything a panel's look depends on but hover / press
    const Elem* Find(int p, int kind, int arg) const;
    void Activate(int h, const Ref& r, const VrPad::Frame& f, VrPad::Sink& sink);
    void Release(int h, VrPad::Sink& sink);
    void SetValue(int arg);
    void Pick(const VrPad::Out& o);
    void Save();
    void SavePlaces();

    VrPad::Config pad_;
    ComboGate     gate_;
    std::wstring  path_;
    Panel         pan_[kPanCount];
    PanelPlace    place_[kPanCount];
    int           tab_ = 0;               // 0 Buttons, 1 Mouse, 2 Panels
    int           pickBtn_ = -1;          // the button being set (a key / mouse button is picked next)
    bool          kbWanted_ = false;      // the keyboard comes back with the menu
    bool          latch_[3] = {};         // Shift, Ctrl, Alt latched (left / right alike)
    Ref           hov_[2];
    Press         press_[2];
    bool          trigDown_[2] = {};
    bool          aimOk_[2] = {};
    Pose          aim_[2];
    double        hitT_[2] = {};
    bool          lastHeadOk_ = false;
    Pose          lastHead_;
    bool          needRedraw_ = false;
    std::unique_ptr<MenuCanvas> cv_[kPanCount];
    std::string   content_[kPanCount];              // ContentKey at the last full draw
    std::vector<uint8_t> elState_[kPanCount];       // ElemState of each element as drawn
    std::vector<uint32_t> cursorPx_;
    double        saveAt_ = -1;
    std::string   status_;                // the bar's last message ("Saved", "Set: A = enter")
};

}  // namespace ck
