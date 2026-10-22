// Phase 70: VR CONTROLLERS -> keys and mouse (dlaa.ini vr_controls / vr_ctl_*). The pure part: the button table, the ini key
// names, the output parser ("H", "Shift+F", "mouse_left", "wheel_up" ...) and the per-frame Mapper (thresholds with
// hysteresis, stick -> mouse, air mouse = the aim's yaw / pitch change while a hold button is down). No Windows input calls:
// vr_controls.cpp sends the Mapper's events with SendInput. Header only, so tests\vr_pad_test.cpp checks it without a game.
#pragma once
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace VrPad {

// the controller "buttons" a dlaa.ini line can map (hand 0 = left, 1 = right)
enum Btn {
    kLTrig, kRTrig, kLGrip, kRGrip,                   // analog, pressed above kPressAt (released below kReleaseAt)
    kX, kY, kA, kB,                                   // X / Y left, A / B right (Vive / WMR / simple: none)
    kLStick, kRStick,                                 // stick (Vive: trackpad) click
    kMenu,                                            // left menu button (the right one belongs to the runtime)
    kLUp, kLDown, kLLeft, kLRight,                    // stick directions (off while that stick moves the mouse)
    kRUp, kRDown, kRLeft, kRRight,
    kBtnCount
};
inline const char* const* BtnNames() {               // config.ini: <name> = <output>
    static const char* const k[kBtnCount] = { "ltrigger", "rtrigger", "lgrip", "rgrip", "x", "y", "a", "b", "lstick", "rstick",
                                              "menu", "lup", "ldown", "lleft", "lright", "rup", "rdown", "rleft", "rright" };
    return k;
}
inline int BtnOfName(const char* s) {
    for (int i = 0; i < kBtnCount; ++i) if (!_stricmp(s, BtnNames()[i])) return i;
    return -1;
}

constexpr float kPressAt = 0.75f, kReleaseAt = 0.55f;   // trigger / grip / stick direction hysteresis

// what one button does. Keys = up to 3 scan codes (set 1; + kExt for an E0 key) pressed in order, released in reverse.
constexpr uint16_t kExt = 0x100;
enum OutKind : uint8_t { kNone, kKeys, kMouseL, kMouseR, kMouseM, kWheelUp, kWheelDown };
struct Out { uint8_t kind = kNone; uint8_t n = 0; uint16_t code[3] = {}; };

// scan code of one key name (case-insensitive); 0 = unknown
inline uint16_t ScanOfName(const char* s) {
    if (!s || !*s) return 0;
    if (!s[1]) {                                       // one character
        const char c = (char)toupper((unsigned char)s[0]);
        static const char kRow1[] = "QWERTYUIOP", kRow2[] = "ASDFGHJKL", kRow3[] = "ZXCVBNM";
        for (int i = 0; kRow1[i]; ++i) if (c == kRow1[i]) return (uint16_t)(0x10 + i);
        for (int i = 0; kRow2[i]; ++i) if (c == kRow2[i]) return (uint16_t)(0x1E + i);
        for (int i = 0; kRow3[i]; ++i) if (c == kRow3[i]) return (uint16_t)(0x2C + i);
        if (c >= '1' && c <= '9') return (uint16_t)(0x02 + (c - '1'));
        switch (c) {
        case '0': return 0x0B; case '-': return 0x0C; case '=': return 0x0D; case '[': return 0x1A; case ']': return 0x1B;
        case ';': return 0x27; case '\'': return 0x28; case '`': return 0x29; case '\\': return 0x2B; case ',': return 0x33;
        case '.': return 0x34; case '/': return 0x35;
        default: return 0;
        }
    }
    if ((s[0] == 'F' || s[0] == 'f') && s[1] >= '1' && s[1] <= '9') {   // F1 .. F12
        const int n = atoi(s + 1);
        if (n >= 1 && n <= 10) return (uint16_t)(0x3B + n - 1);
        if (n == 11) return 0x57;
        if (n == 12) return 0x58;
        return 0;
    }
    struct Named { const char* name; uint16_t code; };
    static const Named kNamed[] = {
        { "space", 0x39 }, { "enter", 0x1C }, { "return", 0x1C }, { "esc", 0x01 }, { "escape", 0x01 }, { "tab", 0x0F },
        { "backspace", 0x0E }, { "up", 0x48 | kExt }, { "down", 0x50 | kExt }, { "left", 0x4B | kExt }, { "right", 0x4D | kExt },
        { "insert", 0x52 | kExt }, { "delete", 0x53 | kExt }, { "home", 0x47 | kExt }, { "end", 0x4F | kExt },
        { "pageup", 0x49 | kExt }, { "pgup", 0x49 | kExt }, { "pagedown", 0x51 | kExt }, { "pgdn", 0x51 | kExt },
        { "shift", 0x2A }, { "lshift", 0x2A }, { "rshift", 0x36 }, { "ctrl", 0x1D }, { "lctrl", 0x1D }, { "rctrl", 0x1D | kExt },
        { "alt", 0x38 }, { "lalt", 0x38 }, { "ralt", 0x38 | kExt }, { "capslock", 0x3A },
        { "num0", 0x52 }, { "num1", 0x4F }, { "num2", 0x50 }, { "num3", 0x51 }, { "num4", 0x4B }, { "num5", 0x4C },
        { "num6", 0x4D }, { "num7", 0x47 }, { "num8", 0x48 }, { "num9", 0x49 }, { "numdot", 0x53 }, { "numplus", 0x4E },
        { "numminus", 0x4A }, { "nummul", 0x37 }, { "numdiv", 0x35 | kExt }, { "numenter", 0x1C | kExt },
        { "pause", 0x45 }, { "scrolllock", 0x46 },
    };
    for (const Named& k : kNamed) if (!_stricmp(s, k.name)) return k.code;
    return 0;
}

// "none" / "" / "H" / "Shift+F" / "mouse_left" / "mouse_right" / "mouse_middle" / "wheel_up" / "wheel_down". False = not
// understood (o = none). Spaces are ignored; "+" joins up to 3 keys (the "=" key is the plus key of a US board; or "numplus").
inline bool ParseOut(const char* in, Out* o) {
    *o = Out();
    char buf[96];
    size_t n = 0;
    for (const char* p = in; *p && n + 1 < sizeof(buf); ++p)
        if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != '"') buf[n++] = *p;
    buf[n] = 0;
    if (!n || !_stricmp(buf, "none") || !_stricmp(buf, "off") || !strcmp(buf, "0")) return true;
    if (!_stricmp(buf, "mouse_left")   || !_stricmp(buf, "mouse1")) { o->kind = kMouseL; return true; }
    if (!_stricmp(buf, "mouse_right")  || !_stricmp(buf, "mouse2")) { o->kind = kMouseR; return true; }
    if (!_stricmp(buf, "mouse_middle") || !_stricmp(buf, "mouse3")) { o->kind = kMouseM; return true; }
    if (!_stricmp(buf, "wheel_up"))   { o->kind = kWheelUp; return true; }
    if (!_stricmp(buf, "wheel_down")) { o->kind = kWheelDown; return true; }
    Out r;
    r.kind = kKeys;
    char* part = buf;
    for (;;) {
        char* plus = part[0] ? strchr(part + 1, '+') : nullptr;   // (part + 1: a lone "+" is never a separator at the start)
        if (plus) *plus = 0;
        const uint16_t c = ScanOfName(part);
        if (!c || r.n >= 3) return false;
        r.code[r.n++] = c;
        if (!plus) break;
        part = plus + 1;
    }
    *o = r;
    return true;
}

struct Config {
    Out   map[kBtnCount];
    int   mouseStick = 1;                              // 0 off, 1 left stick, 2 right stick moves the mouse
    float mouseSpeed = 900.0f;                         // pixels / s at full stick
    float deadzone = 0.15f;                            // stick
    int   airBtn = kRGrip;                             // held = air mouse (the button's own mapping is not sent); -1 = off
    int   airHand = 1;                                 // the hand whose aim moves the mouse (follows airBtn's hand)
    float airGain = 25.0f;                             // pixels per degree of aim turn
};
inline Config DefaultConfig() {
    Config c;
    ParseOut("mouse_left", &c.map[kRTrig]);
    ParseOut("mouse_right", &c.map[kLTrig]);
    ParseOut("enter", &c.map[kA]);
    ParseOut("esc", &c.map[kB]);
    ParseOut("[", &c.map[kX]);                         // ETS2 / ATS default: left indicator
    ParseOut("]", &c.map[kY]);                         // right indicator
    ParseOut("h", &c.map[kRStick]);                    // horn
    ParseOut("f", &c.map[kLStick]);                    // hazard lights
    // kMenu: none -- ETS2 / ATS use the menu button themselves (their only other controller input is the trigger)
    ParseOut("up", &c.map[kRUp]);
    ParseOut("down", &c.map[kRDown]);
    ParseOut("left", &c.map[kRLeft]);
    ParseOut("right", &c.map[kRRight]);
    return c;
}
inline int HandOfBtn(int b) {
    switch (b) {
    case kLTrig: case kLGrip: case kX: case kY: case kLStick: case kMenu: case kLUp: case kLDown: case kLLeft: case kLRight: return 0;
    default: return 1;
    }
}

// one frame of controller state (OpenXR: actions with /user/hand/left and /user/hand/right subaction paths)
struct Frame {
    bool  hand[2] = {};                                // the hand's actions are active (controller on and bound)
    float trig[2] = {}, grip[2] = {};
    bool  prim[2] = {}, sec[2] = {}, click[2] = {}, menu[2] = {};   // prim: X / A, sec: Y / B
    float sx[2] = {}, sy[2] = {};                      // stick, +x right, +y up
    bool  aim[2] = {};                                 // aim orientation valid + tracked
    double q[2][4] = {};                               // aim orientation (x, y, z, w) in the game's space
};

// what the Mapper asks for (vr_controls.cpp: SendInput)
struct Sink {
    virtual void Key(uint16_t code, bool down) = 0;
    virtual void MouseButton(int which, bool down) = 0;   // 0 left, 1 right, 2 middle
    virtual void Wheel(int notches) = 0;                  // + up
    virtual void Move(int dx, int dy) = 0;
    virtual ~Sink() {}
};

// yaw (+ right) and pitch (+ up) in radians of the -z axis of orientation q (OpenXR: -z forward, +x right, +y up)
inline void YawPitch(const double q[4], double* yaw, double* pitch) {
    const double x = q[0], y = q[1], z = q[2], w = q[3];
    const double fx = -2.0 * (x * z + w * y), fy = -2.0 * (y * z - w * x), fz = -(1.0 - 2.0 * (x * x + y * y));
    *yaw = std::atan2(fx, -fz);
    const double s = fy < -1.0 ? -1.0 : (fy > 1.0 ? 1.0 : fy);
    *pitch = std::asin(s);
}

class Mapper {
public:
    // active = send anything at all (enabled, session focused, the game window in front). Inactive = everything held is
    // released once. dt = seconds since the last Step.
    void Step(const Frame& f, bool active, double dt, const Config& c, Sink& s) {
        bool want[kBtnCount] = {};
        if (active) Want(f, c, want);
        for (int b = 0; b < kBtnCount; ++b) {
            if (want[b] == down_[b]) continue;
            down_[b] = want[b];
            if (b == c.airBtn) continue;               // the air mouse hold button sends nothing itself
            Send(c.map[b], want[b], s);
        }
        // air mouse
        const int ah = c.airHand & 1;
        const bool air = active && c.airBtn >= 0 && c.airBtn < kBtnCount && down_[c.airBtn] && f.aim[ah];
        if (air) {
            double yaw, pitch;
            YawPitch(f.q[ah], &yaw, &pitch);
            if (airOn_) {
                double dy = yaw - yaw_;
                if (dy > 3.14159265358979) dy -= 6.28318530717959;
                if (dy < -3.14159265358979) dy += 6.28318530717959;
                accX_ += dy * 57.2957795 * c.airGain;
                accY_ -= (pitch - pitch_) * 57.2957795 * c.airGain;
            }
            yaw_ = yaw; pitch_ = pitch; airOn_ = true;
        } else {
            airOn_ = false;
        }
        // stick mouse
        const int ms = c.mouseStick;
        if (active && (ms == 1 || ms == 2) && f.hand[ms - 1] && dt > 0.0 && dt < 0.25) {
            const int h = ms - 1;
            const double x = f.sx[h], y = f.sy[h];
            const double len = std::sqrt(x * x + y * y);
            if (len > c.deadzone) {
                const double k = (len > 1.0 ? 1.0 : len - c.deadzone) / (1.0 - c.deadzone);   // 0 .. 1 after the dead zone
                const double sp = c.mouseSpeed * k * k * dt / len;                             // squared curve: fine near the centre
                accX_ += x * sp;
                accY_ -= y * sp;
            }
        }
        if (!active) { accX_ = accY_ = 0.0; return; }
        const int mx = (int)accX_, my = (int)accY_;
        if (mx || my) { s.Move(mx, my); accX_ -= mx; accY_ -= my; }
    }
    bool Down(int b) const { return b >= 0 && b < kBtnCount && down_[b]; }

private:
    void Want(const Frame& f, const Config& c, bool* w) const {
        auto ana = [&](int b, float v) { w[b] = down_[b] ? v > kReleaseAt : v > kPressAt; };
        for (int h = 0; h < 2; ++h) {
            if (!f.hand[h]) continue;
            ana(h ? kRTrig : kLTrig, f.trig[h]);
            ana(h ? kRGrip : kLGrip, f.grip[h]);
            w[h ? kA : kX] = f.prim[h];
            w[h ? kB : kY] = f.sec[h];
            w[h ? kRStick : kLStick] = f.click[h];
            if (!h) w[kMenu] = f.menu[0];
            if (c.mouseStick != h + 1) {               // that stick is not the mouse: its four directions are buttons
                const int u = h ? kRUp : kLUp;
                ana(u + 0, f.sy[h]);
                ana(u + 1, -f.sy[h]);
                ana(u + 2, -f.sx[h]);
                ana(u + 3, f.sx[h]);
            }
        }
    }
    static void Send(const Out& o, bool down, Sink& s) {
        switch (o.kind) {
        case kKeys:
            if (down) for (int i = 0; i < o.n; ++i) s.Key(o.code[i], true);
            else      for (int i = o.n - 1; i >= 0; --i) s.Key(o.code[i], false);
            break;
        case kMouseL: s.MouseButton(0, down); break;
        case kMouseR: s.MouseButton(1, down); break;
        case kMouseM: s.MouseButton(2, down); break;
        case kWheelUp:   if (down) s.Wheel(1); break;
        case kWheelDown: if (down) s.Wheel(-1); break;
        default: break;
        }
    }
    bool   down_[kBtnCount] = {};
    bool   airOn_ = false;
    double yaw_ = 0.0, pitch_ = 0.0;
    double accX_ = 0.0, accY_ = 0.0;
};

}  // namespace VrPad
