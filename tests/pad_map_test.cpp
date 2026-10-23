// Unit test: VR CONTROLLERS -> keys / mouse (src/pad_map.h). (1) key names -> scan codes and the output parser
// (combos, mouse, wheel, junk). (2) the Mapper: press / release edges, trigger hysteresis, combos released in reverse, stick
// directions vs the mouse stick, stick mouse speed and dead zone, the air mouse (yaw right = +x, pitch up = -y, its hold button
// sends nothing), everything released when inactive, driving (left stick = steering with dead zone / curve / max, left
// trigger = throttle, the left trigger's mapping and stick directions not used, sent only on change, 0 when inactive). No game, no Windows input. Build: the pad_map_test target.
// Exit code 0 = every check passed.
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include "pad_map.h"

static int g_checks = 0, g_fails = 0;
#define CHECK(c, ...) do { ++g_checks; if (!(c)) { ++g_fails; if (g_fails <= 40) { printf("   FAIL: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)

struct Rec : VrPad::Sink {
    std::vector<std::string> ev;
    int mx = 0, my = 0;
    void Key(uint16_t c, bool d) override { char b[32]; snprintf(b, sizeof(b), "k%03x%s", c, d ? "v" : "^"); ev.push_back(b); }
    void MouseButton(int w, bool d) override { char b[32]; snprintf(b, sizeof(b), "m%d%s", w, d ? "v" : "^"); ev.push_back(b); }
    void Wheel(int n) override { char b[32]; snprintf(b, sizeof(b), "w%+d", n); ev.push_back(b); }
    void Move(int dx, int dy) override { mx += dx; my += dy; }
    void Drive(float s, float t, float b) override { drv[0] = s; drv[1] = t; drv[2] = b; ++drives; }
    float drv[3] = {};
    int drives = 0;
    std::string Take() { std::string s; for (auto& e : ev) { if (!s.empty()) s += ' '; s += e; } ev.clear(); return s; }
};

static void QuatYawPitch(double yawDeg, double pitchDeg, double q[4]) {   // yaw about +y (right = negative angle), then pitch about +x
    const double ya = -yawDeg * 3.14159265358979 / 360.0, pa = pitchDeg * 3.14159265358979 / 360.0;
    const double qy[4] = { 0, std::sin(ya), 0, std::cos(ya) }, qx[4] = { std::sin(pa), 0, 0, std::cos(pa) };
    // q = qy * qx
    q[0] = qy[3] * qx[0] + qy[0] * qx[3] + qy[1] * qx[2] - qy[2] * qx[1];
    q[1] = qy[3] * qx[1] - qy[0] * qx[2] + qy[1] * qx[3] + qy[2] * qx[0];
    q[2] = qy[3] * qx[2] + qy[0] * qx[1] - qy[1] * qx[0] + qy[2] * qx[3];
    q[3] = qy[3] * qx[3] - qy[0] * qx[0] - qy[1] * qx[1] - qy[2] * qx[2];
}

// the mapping these checks were written for (the defaults before 2026-10-23): trigger = left click, left trigger = right click,
// A = Enter, left stick = mouse at 900 px/s, right grip + right trigger = right click
static VrPad::Config OldDefaults() {
    VrPad::Config c = VrPad::DefaultConfig();
    VrPad::ParseOut("mouse_left", &c.map[VrPad::kRTrig]);
    VrPad::ParseOut("mouse_right", &c.map[VrPad::kLTrig]);
    VrPad::ParseOut("enter", &c.map[VrPad::kA]);
    c.mouseStick = 1;
    c.mouseSpeed = 900.0f;
    c.steerStick = 0;
    c.modMap[VrPad::kA] = c.modMap[VrPad::kB] = VrPad::Out();
    VrPad::ParseOut("mouse_right", &c.modMap[VrPad::kRTrig]);
    return c;
}

int main() {
    using namespace VrPad;
    printf("(1) names and parser\n");
    CHECK(ScanOfName("H") == 0x23 && ScanOfName("h") == 0x23, "H");
    CHECK(ScanOfName("Q") == 0x10 && ScanOfName("A") == 0x1E && ScanOfName("Z") == 0x2C && ScanOfName("M") == 0x32, "letters");
    CHECK(ScanOfName("1") == 0x02 && ScanOfName("0") == 0x0B, "digits");
    CHECK(ScanOfName("[") == 0x1A && ScanOfName("]") == 0x1B, "brackets");
    CHECK(ScanOfName("F1") == 0x3B && ScanOfName("F10") == 0x44 && ScanOfName("F11") == 0x57 && ScanOfName("f12") == 0x58, "F keys");
    CHECK(ScanOfName("F13") == 0 && ScanOfName("F0") == 0, "bad F keys");
    CHECK(ScanOfName("Delete") == (0x53 | kExt) && ScanOfName("UP") == (0x48 | kExt), "extended");
    CHECK(ScanOfName("space") == 0x39 && ScanOfName("Esc") == 0x01 && ScanOfName("numenter") == (0x1C | kExt), "named");
    CHECK(ScanOfName("bogus") == 0 && ScanOfName("") == 0, "unknown");
    Out o;
    CHECK(ParseOut(" none ", &o) && o.kind == kNone, "none");
    CHECK(ParseOut("", &o) && o.kind == kNone, "empty");
    CHECK(ParseOut("Shift + F", &o) && o.kind == kKeys && o.n == 2 && o.code[0] == 0x2A && o.code[1] == 0x21, "combo");
    CHECK(ParseOut("ctrl+alt+delete", &o) && o.n == 3 && o.code[2] == (0x53 | kExt), "3 combo");
    CHECK(!ParseOut("a+b+c+d", &o) && o.kind == kNone, "4 keys refused");
    CHECK(!ParseOut("shift+", &o), "trailing +");
    CHECK(!ParseOut("nokey", &o) && o.kind == kNone, "junk refused");
    CHECK(ParseOut("mouse_left", &o) && o.kind == kMouseL, "mouse_left");
    CHECK(ParseOut("\"wheel_down\"\r\n", &o) && o.kind == kWheelDown, "wheel_down quoted");
    CHECK(BtnOfName("RGRIP") == kRGrip && BtnOfName("menu") == kMenu && BtnOfName("zz") == -1, "button names");

    printf("(2) mapper\n");
    {   // the modifier (default: right grip + right trigger = right click; the grip stays the air mouse)
        Config c = OldDefaults();
        ParseOut("f1", &c.modMap[kA]);
        Mapper m; Rec r; Frame f;
        f.hand[0] = f.hand[1] = true;
        f.trig[1] = 1.0f;
        m.Step(f, true, 0.011, c, r);
        f.trig[1] = 0.0f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m0v m0^", "trigger alone: left click");
        f.grip[1] = 1.0f;                              // the modifier (also the air mouse button: sends nothing itself)
        m.Step(f, true, 0.011, c, r);
        f.trig[1] = 1.0f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m1v", "grip held + trigger: right click down");
        f.grip[1] = 0.0f;                              // the grip let go first: the trigger still releases what it pressed
        m.Step(f, true, 0.011, c, r);
        f.trig[1] = 0.0f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m1^", "released as pressed (right), whatever the modifier does meanwhile");
        f.grip[1] = 1.0f; f.prim[1] = true;            // A with the modifier: its own entry
        m.Step(f, true, 0.011, c, r);
        f.prim[1] = false; f.grip[1] = 0.0f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "k03bv k03b^", "grip + A = F1 (modifier_keys)");
        f.grip[1] = 1.0f; f.sec[1] = true;             // B has no entry: as normal (Esc)
        m.Step(f, true, 0.011, c, r);
        f.sec[1] = false; f.grip[1] = 0.0f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "k001v k001^", "grip + B without an entry = B as normal");
        c.modBtn = -1;                                 // no modifier: the grip changes nothing
        f.grip[1] = 1.0f; f.trig[1] = 1.0f;
        m.Step(f, true, 0.011, c, r);
        f.grip[1] = 0.0f; f.trig[1] = 0.0f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m0v m0^", "modifier off: the trigger is the left click");
    }
    {   // the defaults since 2026-10-23: A = left click, B = Esc, right grip (air mouse, the modifier) + A / B = left / right
        // click, + right trigger = left click; the right trigger alone = gas, the left trigger = brake
        Config c = DefaultConfig();
        Mapper m; Rec r; Frame f;
        f.hand[0] = f.hand[1] = true;
        f.prim[1] = true; m.Step(f, true, 0.011, c, r);
        f.prim[1] = false; m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m0v m0^", "A = left click");
        f.sec[1] = true; m.Step(f, true, 0.011, c, r);
        f.sec[1] = false; m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "k001v k001^", "B alone = Esc");
        f.grip[1] = 1.0f; m.Step(f, true, 0.011, c, r);
        f.sec[1] = true; m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m1v", "right grip + B = right click down (drag the map)");
        f.sec[1] = false; m.Step(f, true, 0.011, c, r);
        f.prim[1] = true; m.Step(f, true, 0.011, c, r);
        f.prim[1] = false; f.grip[1] = 0.0f; m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m1^ m0v m0^", "... up; right grip + A = left click");
        f.trig[1] = 1.0f; m.Step(f, true, 0.011, c, r);
        CHECK(r.Take().empty() && r.drv[1] == 1.0f, "right trigger alone = gas (no click)");
        f.trig[1] = 0.0f; m.Step(f, true, 0.011, c, r);
        f.grip[1] = 1.0f; m.Step(f, true, 0.011, c, r);
        f.trig[1] = 1.0f; m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m0v" && r.drv[1] == 0.0f, "right grip + right trigger = left click, no gas");
        f.trig[1] = 0.0f; f.grip[1] = 0.0f; m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m0^", "... released");
        f.trig[0] = 1.0f; m.Step(f, true, 0.011, c, r);
        CHECK(r.Take().empty() && r.drv[2] == 1.0f, "left trigger = brake");
    }
    {   // edges, hysteresis, the hold button, combos
        Config c = OldDefaults();
        ParseOut("shift+f", &c.map[kX]);
        ParseOut("delete", &c.map[kMenu]);
        Mapper m; Rec r; Frame f;
        f.hand[0] = f.hand[1] = true;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take().empty(), "idle frame sends nothing");
        f.trig[1] = 0.8f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m0v", "right trigger -> left click down");
        f.trig[1] = 0.6f;                              // inside the hysteresis: still down
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take().empty(), "hysteresis keeps it down");
        f.trig[1] = 0.5f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "m0^", "release below 0.55");
        f.trig[1] = 0.7f;                              // below the press point: not pressed again
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take().empty(), "0.7 does not press");
        f.prim[0] = true;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "k02av k021v", "combo down in order");
        f.prim[0] = false;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "k021^ k02a^", "combo up in reverse");
        f.grip[1] = 1.0f;                              // the air mouse hold button: nothing sent
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take().empty() && m.Down(kRGrip), "hold button sends nothing");
        f.menu[0] = true; f.sec[1] = true;
        m.Step(f, true, 0.011, c, r);
        const std::string t = r.Take();
        CHECK(t == "k001v k153v", "B (esc) + menu (delete) -- got %s", t.c_str());
        m.Step(f, false, 0.011, c, r);                 // inactive: all released
        const std::string u = r.Take();
        CHECK(u == "k001^ k153^", "inactive releases all -- got %s", u.c_str());
        m.Step(f, false, 0.011, c, r);
        CHECK(r.Take().empty(), "released only once");
    }
    {   // stick directions vs the mouse stick
        Config c = OldDefaults();                    // left stick = mouse, right stick = arrows
        Mapper m; Rec r; Frame f;
        f.hand[0] = f.hand[1] = true;
        f.sx[1] = 0.9f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "k14dv", "right stick right -> Right arrow");
        f.sx[1] = 0.0f; f.sy[1] = -0.9f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "k150v k14d^", "down pressed, right released");
        f.sy[1] = 0.0f;
        f.sx[0] = 1.0f;                                // left stick full right for 1 s at 100 Hz = mouseSpeed px
        for (int i = 0; i < 100; ++i) m.Step(f, true, 0.01, c, r);
        const std::string t = r.Take();
        CHECK(t == "k150^", "only the down release (left stick sends no keys) -- got %s", t.c_str());
        CHECK(std::abs(r.mx - (int)c.mouseSpeed) <= 2 && r.my == 0, "full stick 1 s = %d px (got %d, %d)", (int)c.mouseSpeed, r.mx, r.my);
        r.mx = r.my = 0;
        f.sx[0] = 0.1f;                                // inside the dead zone
        for (int i = 0; i < 100; ++i) m.Step(f, true, 0.01, c, r);
        CHECK(r.mx == 0 && r.my == 0, "dead zone");
        f.sx[0] = 0.0f; f.sy[0] = 0.575f;              // half way after the dead zone: quarter speed, up = -y
        for (int i = 0; i < 100; ++i) m.Step(f, true, 0.01, c, r);
        CHECK(r.mx == 0 && std::abs(r.my + (int)(c.mouseSpeed * 0.25f)) <= 2, "half stick up = quarter speed (got %d)", r.my);
        r.mx = r.my = 0;
        f.sx[0] = 1.0f; f.sy[0] = 1.0f;                // diagonal past unit length (Touch reaches ~1.4): capped at full speed
        for (int i = 0; i < 100; ++i) m.Step(f, true, 0.01, c, r);
        const double diag = std::sqrt((double)r.mx * r.mx + (double)r.my * r.my);
        CHECK(std::abs(diag - c.mouseSpeed) <= 3 && r.mx > 0 && r.my < 0, "diagonal 1 s = %d px total (got %.1f: %d, %d)", (int)c.mouseSpeed, diag, r.mx, r.my);
        f.sx[0] = 0.0f; f.sy[0] = 0.0f;
        c.mouseStick = 0;                             // left stick = arrows? no: only the right has default keys -> left dirs send nothing
        r.mx = r.my = 0;
        f.sy[0] = 0.9f;
        m.Step(f, true, 0.01, c, r);
        CHECK(r.Take().empty() && r.mx == 0 && r.my == 0 && m.Down(kLUp), "mouse off: left up is a button (unmapped)");
    }
    {   // air mouse
        Config c = OldDefaults();
        Mapper m; Rec r; Frame f;
        f.hand[0] = f.hand[1] = true;
        f.aim[1] = true;
        QuatYawPitch(0, 0, f.q[1]);
        m.Step(f, true, 0.011, c, r);
        CHECK(r.mx == 0 && r.my == 0, "no move without the hold button");
        f.grip[1] = 1.0f;
        QuatYawPitch(0, 0, f.q[1]);
        m.Step(f, true, 0.011, c, r);                  // start: no move
        CHECK(r.mx == 0 && r.my == 0, "start frame does not move");
        QuatYawPitch(2, 0, f.q[1]);                    // 2 degrees right
        m.Step(f, true, 0.011, c, r);
        CHECK(std::abs(r.mx - (int)(2 * c.airGain)) <= 1 && r.my == 0, "2 deg right = +%d px (got %d, %d)", (int)(2 * c.airGain), r.mx, r.my);
        r.mx = r.my = 0;
        QuatYawPitch(2, 1, f.q[1]);                    // 1 degree up
        m.Step(f, true, 0.011, c, r);
        CHECK(std::abs(r.my + (int)c.airGain) <= 1 && std::abs(r.mx) <= 1, "1 deg up = -%d px (got %d, %d)", (int)c.airGain, r.mx, r.my);
        r.mx = r.my = 0;
        f.grip[1] = 0.0f;                              // let go, turn, hold again: no jump
        QuatYawPitch(30, 1, f.q[1]);
        m.Step(f, true, 0.011, c, r);
        f.grip[1] = 1.0f;
        m.Step(f, true, 0.011, c, r);
        CHECK(r.mx == 0 && r.my == 0, "no jump after re-grab");
        double y, p;
        QuatYawPitch(179, 0, f.q[1]); m.Step(f, true, 0.011, c, r);
        r.mx = 0;
        QuatYawPitch(-179, 0, f.q[1]); m.Step(f, true, 0.011, c, r);   // across +-180: +2 deg
        CHECK(std::abs(r.mx - (int)(2 * c.airGain)) <= 2, "wrap at 180 (got %d)", r.mx);
        QuatYawPitch(-40, 25, f.q[1]); YawPitch(f.q[1], &y, &p);
        CHECK(std::abs(y * 57.2957795 + 40) < 0.01 && std::abs(p * 57.2957795 - 25) < 0.01, "YawPitch (%.3f, %.3f)", y * 57.29578, p * 57.29578);
        CHECK(r.Take().empty(), "air mouse sends no buttons");
    }
    printf("(3) driving\n");
    {
        Config c = DefaultConfig();                    // left stick x = steering; here: left trigger = gas, curve 2, no modifier keys
        c.steerSpeed = 0.0f;                           // (at once: the values below are exact)
        c.steerCurve = 2.0f;
        ParseOut("gas", &c.map[kLTrig]);
        c.map[kRTrig] = Out();
        for (Out& mo : c.modMap) mo = Out();
        const Config dc = DefaultConfig();
        CHECK(dc.steerStick == 1 && dc.map[kRTrig].kind == kGas && dc.map[kLTrig].kind == kBrake && dc.map[kLGrip].kind == kNone &&
              dc.modMap[kRTrig].kind == kMouseL && dc.steerSpeed == 0.5f && dc.steerCurve == 1.0f,
              "defaults: left stick steers, right trigger = gas (+ right grip: left click), left trigger = brake, curve 1, speed 0.5");
        Out po;
        CHECK(ParseOut("gas", &po) && po.kind == kGas && ParseOut("Throttle", &po) && po.kind == kGas && ParseOut("brake", &po) && po.kind == kBrake,
              "gas / throttle / brake parse");
        char ob[32];
        OutToString(po, ob, sizeof(ob));
        CHECK(!strcmp(ob, "brake"), "brake written back (%s)", ob);
        CHECK(SteerOf(0.05f, c) == 0.0f && SteerOf(-0.1f, c) == 0.0f, "steering dead zone");
        CHECK(std::fabs(SteerOf(1.0f, c) - 1.0f) < 1e-5f && std::fabs(SteerOf(-1.0f, c) + 1.0f) < 1e-5f, "full stick = full wheel");
        CHECK(std::fabs(SteerOf(0.55f, c) - 0.25f) < 1e-4f, "half way after the dead zone: 0.5^2 (%.4f)", (double)SteerOf(0.55f, c));
        CHECK(SteerOf(1.4f, c) == 1.0f, "a diagonal over 1 is capped");
        Config m = c; m.steerMax = 0.5f; m.steerCurve = 1.0f;
        CHECK(std::fabs(SteerOf(-1.0f, m) + 0.5f) < 1e-5f && std::fabs(SteerOf(0.55f, m) - 0.25f) < 1e-4f, "steer_max / curve 1");
        CHECK(PedalOf(0.04f) == 0.0f && PedalOf(1.0f) == 1.0f && std::fabs(PedalOf(0.525f) - 0.5f) < 1e-4f, "pedal: rest, full, half");
        Mapper mp; Rec r; Frame f;
        f.hand[0] = f.hand[1] = true;
        mp.Step(f, true, 0.011, c, r);
        CHECK(r.drives == 0, "nothing sent while nothing moves");
        f.sx[0] = 1.0f; f.trig[0] = 1.0f;
        mp.Step(f, true, 0.011, c, r);
        CHECK(r.drives == 1 && r.drv[0] == 1.0f && r.drv[1] == 1.0f && r.drv[2] == 0.0f, "full right + full gas");
        CHECK(r.Take().empty() && r.mx == 0 && r.my == 0, "no key from the gas trigger or the steering stick, no mouse");
        mp.Step(f, true, 0.011, c, r);
        CHECK(r.drives == 1, "the same values are not sent again");
        f.sx[0] = 0.0f; f.trig[0] = 0.0f;
        mp.Step(f, true, 0.011, c, r);
        CHECK(r.drives == 2 && r.drv[0] == 0.0f && r.drv[1] == 0.0f, "stick let go = straight, trigger let go = roll");
        f.sy[0] = 1.0f;                                // the steering stick's up / down are still buttons
        ParseOut("x", &c.map[kLUp]);
        mp.Step(f, true, 0.011, c, r);
        f.sy[0] = 0.0f; mp.Step(f, true, 0.011, c, r);
        CHECK(r.Take() == "k02dv k02d^", "steering stick up = its key (X)");
        Config b = c;                                  // brake on the left grip, gas also on the left stick up (analog)
        ParseOut("brake", &b.map[kLGrip]);
        ParseOut("gas", &b.map[kLUp]);
        f.grip[0] = 0.525f;
        mp.Step(f, true, 0.011, b, r);
        CHECK(std::fabs(r.drv[2] - 0.5f) < 1e-4f && r.Take().empty(), "left grip = brake, half pressed: 0.5, no key");
        f.grip[0] = 0.0f; f.sy[0] = 0.575f;            // (stick dead zone 0.15: (0.575 - 0.15) / 0.85 = 0.5)
        mp.Step(f, true, 0.011, b, r);
        CHECK(std::fabs(r.drv[1] - 0.5f) < 1e-4f && r.drv[2] == 0.0f, "left stick up = gas, analog (%.3f)", (double)r.drv[1]);
        f.sy[0] = 0.0f; f.trig[0] = 0.3f; f.prim[0] = true;
        ParseOut("gas", &b.map[kX]);
        mp.Step(f, true, 0.011, b, r);
        CHECK(r.drv[1] == 1.0f && r.Take().empty(), "two gas buttons: the most pressed one (X = full)");
        f.prim[0] = false; f.trig[0] = 0.0f; mp.Step(f, true, 0.011, b, r);
        Config md = c;                                 // with the modifier: right grip + right trigger = brake
        ParseOut("brake", &md.modMap[kRTrig]);
        ParseOut("mouse_left", &md.map[kRTrig]);
        f.trig[1] = 0.4f;                              // (not pressed as a button yet: the modifier decides already)
        mp.Step(f, true, 0.011, md, r);
        CHECK(r.drv[2] == 0.0f && r.Take().empty(), "without the modifier the trigger is a click (not pressed yet)");
        f.grip[1] = 1.0f;
        mp.Step(f, true, 0.011, md, r);
        CHECK(r.drv[2] > 0.3f && r.Take().empty(), "right grip + right trigger = brake (%.3f)", (double)r.drv[2]);
        f.grip[1] = 0.0f; f.trig[1] = 0.0f;
        f.sx[0] = -0.7f; f.trig[0] = 0.6f;
        mp.Step(f, true, 0.011, c, r);
        mp.Step(f, false, 0.011, c, r);
        CHECK(r.drv[0] == 0.0f && r.drv[1] == 0.0f && r.drv[2] == 0.0f, "inactive (game not in front): all 0");
        Config off = c; off.steerStick = 0; ParseOut("wheel_up", &off.map[kLTrig]);
        Mapper mo; Rec ro; Frame g;
        g.hand[0] = g.hand[1] = true;
        g.trig[0] = 1.0f; g.sx[0] = 1.0f;
        mo.Step(g, true, 0.011, off, ro);
        CHECK(ro.drives == 0 && ro.Take() == "w+1", "steer = off, left trigger = wheel up: no driving at all");
        Config ml = c; ml.mouseStick = 1;              // the mouse on the steering stick: steering wins
        Mapper mm; Rec rm; Frame h;
        h.hand[0] = true; h.sx[0] = 1.0f;
        mm.Step(h, true, 0.011, ml, rm);
        mm.Step(h, true, 0.011, ml, rm);
        CHECK(rm.mx == 0 && rm.drv[0] == 1.0f, "mouse_stick = steer stick: it steers, no mouse");
        Config rs = c; rs.steerStick = 2;              // the right stick steers (the mouse moves to nothing on it)
        Mapper mr; Rec rr; Frame k;
        k.hand[0] = k.hand[1] = true; k.sx[1] = -1.0f;
        mr.Step(k, true, 0.011, rs, rr);
        CHECK(rr.drv[0] == -1.0f && rr.mx == 0 && rr.Take().empty(), "steer = right: the right stick steers");
    }
    {   // steering speed: the wheel follows the stick at most steerSpeed full turns per second
        Config c = DefaultConfig();
        c.steerSpeed = 2.0f; c.steerCurve = 1.0f; c.steerDeadzone = 0.0f;
        Mapper mp; Rec r; Frame f;
        f.hand[0] = true; f.sx[0] = 1.0f;
        mp.Step(f, true, 0.1, c, r);
        CHECK(std::fabs(r.drv[0] - 0.2f) < 1e-4f, "0.1 s after a flick: 0.2 of the wheel (%.3f)", (double)r.drv[0]);
        for (int i = 0; i < 4; ++i) mp.Step(f, true, 0.1, c, r);
        CHECK(r.drv[0] == 1.0f, "0.5 s: full");
        f.sx[0] = 0.0f;
        mp.Step(f, true, 0.1, c, r);
        CHECK(std::fabs(r.drv[0] - 0.8f) < 1e-4f, "let go: back toward the middle at the same speed (%.3f)", (double)r.drv[0]);
        mp.Step(f, false, 0.1, c, r);
        CHECK(r.drv[0] == 0.0f, "inactive: 0 at once");
    }
    printf("%d checks, %d failed\n", g_checks, g_fails);
    return g_fails ? 1 : 0;
}
