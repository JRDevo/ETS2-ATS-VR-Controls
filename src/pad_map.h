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
// kGas / kBrake: driving pedals -- not a key: how far the button is pressed (a trigger / grip / stick direction: analog, any
// other button: full) goes to the game as the gas / brake pedal (Sink::Drive, scs_input.h)
enum OutKind : uint8_t { kNone, kKeys, kMouseL, kMouseR, kMouseM, kWheelUp, kWheelDown, kGas, kBrake };
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
        { "scrolllock", 0x46 },   // (no "pause": its scan sequence is E1 1D 45; a plain 0x45 is NumLock)
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
    if (!_stricmp(buf, "gas") || !_stricmp(buf, "throttle") || !_stricmp(buf, "accelerate")) { o->kind = kGas; return true; }
    if (!_stricmp(buf, "brake")) { o->kind = kBrake; return true; }
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

// the config.ini name of scan code `code` ("f", "f1", "enter", "[", "num7" ...); nullptr = none
inline const char* NameOfScan(uint16_t code) {
    static const char* const kAll[] = {
        "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x",
        "y", "z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10",
        "f11", "f12", "space", "enter", "esc", "tab", "backspace", "up", "down", "left", "right", "insert", "delete", "home", "end",
        "pageup", "pagedown", "shift", "rshift", "ctrl", "rctrl", "alt", "ralt", "capslock", "num0", "num1", "num2", "num3", "num4",
        "num5", "num6", "num7", "num8", "num9", "numdot", "numplus", "numminus", "nummul", "numdiv", "numenter", "scrolllock",
        "-", "=", "[", "]", ";", "'", "`", "\\", ",", ".", "/" };
    for (const char* n : kAll) if (ScanOfName(n) == code) return n;
    return nullptr;
}

// the config.ini text of `o` ("none", "mouse_left", "shift+f" ...) into buf (cap >= 32)
inline void OutToString(const Out& o, char* buf, size_t cap) {
    if (!cap) return;
    buf[0] = 0;
    switch (o.kind) {
    case kMouseL: strncpy_s(buf, cap, "mouse_left", _TRUNCATE); return;
    case kMouseR: strncpy_s(buf, cap, "mouse_right", _TRUNCATE); return;
    case kMouseM: strncpy_s(buf, cap, "mouse_middle", _TRUNCATE); return;
    case kWheelUp: strncpy_s(buf, cap, "wheel_up", _TRUNCATE); return;
    case kWheelDown: strncpy_s(buf, cap, "wheel_down", _TRUNCATE); return;
    case kGas: strncpy_s(buf, cap, "gas", _TRUNCATE); return;
    case kBrake: strncpy_s(buf, cap, "brake", _TRUNCATE); return;
    case kKeys: break;
    default: strncpy_s(buf, cap, "none", _TRUNCATE); return;
    }
    for (int i = 0; i < o.n; ++i) {
        const char* n = NameOfScan(o.code[i]);
        if (i) strncat_s(buf, cap, "+", _TRUNCATE);
        strncat_s(buf, cap, n ? n : "?", _TRUNCATE);
    }
}

struct Config {
    Out   map[kBtnCount];
    int   mouseStick = 2;                              // 0 off, 1 left stick, 2 right stick moves the mouse
    float mouseSpeed = 1800.0f;                        // pixels / s at full stick
    float deadzone = 0.15f;                            // stick
    int   airBtn = kRGrip;                             // held = air mouse (the button's own mapping is not sent); -1 = off
    int   airHand = 1;                                 // the hand whose aim moves the mouse (follows airBtn's hand)
    float airGain = 25.0f;                             // pixels per degree of aim turn
    bool  airToggle = false;                           // false: air mouse while airBtn is held; true: one press on, the next off
    // the modifier: while modBtn is held, a button that has a modMap entry sends that instead of its map (e.g. right grip +
    // A / B = left / right click, so one controller does both clicks). -1 = no modifier. The modifier button keeps its own
    // job (the air mouse on the right grip). What a press sends is decided when it goes down (and released the same way).
    int   modBtn = kRGrip;
    Out   modMap[kBtnCount];
    // driving -- analog values for the game's own input plugin door (scs_input.h), not keys. Steering: steerStick's x (its
    // left / right directions are then not buttons, and a mouse on that stick is off), in one of two styles (steerMode):
    //   wheel (0): the stick's position is the wheel's position; the wheel follows it at most steerSpeed full turns (centre ->
    //              full lock) per second, so a flick does not throw the wheel.
    //   rate  (1): the stick's push is how fast the wheel turns -- a little = slowly, full = full lock in steerTurnTime s;
    //              let go: the wheel goes back to the middle (full lock -> straight in steerReturnTime s).
    // Both: dead zone + curve on the stick, the wheel never past steerMax. Gas / brake: any button mapped to "gas" / "brake"
    // (map or modMap), see kGas.
    bool  drive = true;                                // off: no steering, gas / brake buttons send nothing, the steering
                                                       //   stick's left / right are buttons again (the menu's one switch)
    int   steerStick = 1;                              // 0 off, 1 left stick, 2 right stick
    float steerDeadzone = 0.10f;                       // stick, before steering starts
    float steerCurve = 1.0f;                           // 1 = straight; more = finer near the centre
    float steerMax = 1.0f;                             // full stick = this much of the wheel (0.2 .. 1)
    float steerSpeed = 0.5f;                           // wheel: centre -> full lock per second at most (0 = at once)
    int   steerMode = 1;                               // 0 wheel, 1 rate
    float steerTurnTime = 5.0f;                        // rate: full stick turns centre -> full lock in this many seconds
    float steerReturnTime = 3.0f;                      // rate: let go, full lock -> straight in this many seconds
};
inline Config DefaultConfig() {
    Config c;
    // (the user's own mapping, made the default 2026-10-23. Driving: left stick = steering, right trigger = gas, left trigger
    //  = brake. The mouse with the right hand: right stick or air mouse (hold the right grip) to point, A = left click; right
    //  grip + A or + right trigger = left click, + B = right click (drag the map with the grip held).)
    ParseOut("gas", &c.map[kRTrig]);
    ParseOut("brake", &c.map[kLTrig]);
    ParseOut("mouse_left", &c.map[kA]);
    ParseOut("esc", &c.map[kB]);
    ParseOut("[", &c.map[kX]);                         // ETS2 / ATS default: left indicator
    ParseOut("]", &c.map[kY]);                         // right indicator
    ParseOut("h", &c.map[kRStick]);                    // horn
    ParseOut("f", &c.map[kLStick]);                    // hazard lights
    // kMenu: none -- ETS2 / ATS use the menu button themselves (their only other controller input is the trigger)
    ParseOut("up", &c.map[kRUp]);                      // (used when the right stick does not move the mouse: mouse_stick)
    ParseOut("down", &c.map[kRDown]);
    ParseOut("left", &c.map[kRLeft]);
    ParseOut("right", &c.map[kRRight]);
    ParseOut("mouse_left", &c.modMap[kA]);             // right grip (air mouse) + A = left click
    ParseOut("mouse_left", &c.modMap[kRTrig]);         // ... + right trigger = left click (the trigger alone is the gas)
    ParseOut("mouse_right", &c.modMap[kB]);            // right grip (air mouse) + B = right click: drag the map
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
    bool  pos[2] = {};                                 // aim position valid + tracked (the VR menu's pointer)
    double p[2][3] = {};                               // aim position (metres) in the same space
};

// what the Mapper asks for (vr_controls.cpp: SendInput)
struct Sink {
    virtual void Key(uint16_t code, bool down) = 0;
    virtual void MouseButton(int which, bool down) = 0;   // 0 left, 1 right, 2 middle
    virtual void Wheel(int notches) = 0;                  // + up
    virtual void Move(int dx, int dy) = 0;
    virtual void Drive(float steer, float throttle, float brake) { (void)steer; (void)throttle; (void)brake; }   // only on change
    virtual ~Sink() {}
};

// driving: the stick's x (-1 left .. +1 right) as the wanted steering after the dead zone and curve; a trigger as a pedal
// (0 .. 1)
inline float StickShape(float x, const Config& c) {   // -1 .. 1 after the dead zone and curve
    const float a = x < 0 ? -x : x;
    if (a <= c.steerDeadzone || c.steerDeadzone >= 1.0f) return 0.0f;
    const float k = ((a > 1.0f ? 1.0f : a) - c.steerDeadzone) / (1.0f - c.steerDeadzone);
    const float v = std::pow(k, c.steerCurve);
    return x < 0 ? -v : v;
}
inline int SteerStickOf(const Config& c) { return c.drive ? c.steerStick : 0; }   // the stick that steers now (0 none)
inline float SteerOf(float x, const Config& c) { return StickShape(x, c) * c.steerMax; }   // wheel style: the wheel position
inline float PedalOf(float t) {
    constexpr float kRest = 0.05f;                     // a resting finger: no throttle
    if (t <= kRest) return 0.0f;
    const float v = (t - kRest) / (1.0f - kRest);
    return v > 1.0f ? 1.0f : v;
}

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
        const bool modOn = c.modBtn >= 0 && c.modBtn < kBtnCount && want[c.modBtn];
        for (int b = 0; b < kBtnCount; ++b) {
            if (want[b] == down_[b]) continue;
            down_[b] = want[b];
            if (b == c.airBtn) {                       // the air mouse button sends nothing itself
                if (want[b]) airLatch_ = !airLatch_;   // (toggle mode: each press switches)
                continue;
            }
            if (want[b]) {                             // the modifier held: its entry for this button, if it has one
                sent_[b] = modOn && b != c.modBtn && c.modMap[b].kind != kNone ? c.modMap[b] : c.map[b];
                Send(sent_[b], true, s);
            } else {
                Send(sent_[b], false, s);              // (released as it was pressed, whatever the modifier does now)
            }
        }
        if (!active || !c.airToggle) airLatch_ = false;
        // air mouse
        const int ah = c.airHand & 1;
        const bool airBtnOn = c.airBtn >= 0 && c.airBtn < kBtnCount && (c.airToggle ? airLatch_ : down_[c.airBtn]);
        const bool air = active && airBtnOn && f.aim[ah];
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
        const int ms = SteerStickOf(c) == c.mouseStick ? 0 : c.mouseStick;   // (steering owns that stick)
        if (active && (ms == 1 || ms == 2) && f.hand[ms - 1] && dt > 0.0 && dt < 0.25) {
            const int h = ms - 1;
            const double x = f.sx[h], y = f.sy[h];
            const double len = std::sqrt(x * x + y * y);
            if (len > c.deadzone) {
                const double k = ((len > 1.0 ? 1.0 : len) - c.deadzone) / (1.0 - c.deadzone);   // 0 .. 1 after the dead zone
                                                                                                 // (diagonals reach ~1.4: capped)
                const double sp = c.mouseSpeed * k * k * dt / len;                             // squared curve: fine near the centre
                accX_ += x * sp;
                accY_ -= y * sp;
            }
        }
        // driving: sent only when a value changed (inactive = all 0, at once)
        float dv[3] = {};
        if (active && c.drive) {
            const int sh = c.steerStick - 1;
            const float x = (sh == 0 || sh == 1) && f.hand[sh] ? f.sx[sh] : 0.0f;
            const float t = (float)(dt > 0.0 && dt < 0.25 ? dt : 0.011);
            const float cur = drive_[0];
            float st;
            if (c.steerMode == 1) {                    // rate: the push turns the wheel; let go = back to the middle
                const float r = StickShape(x, c);
                if (r != 0.0f) {
                    st = cur + r * t / (c.steerTurnTime > 0.1f ? c.steerTurnTime : 0.1f);
                } else {
                    const float step = t / (c.steerReturnTime > 0.1f ? c.steerReturnTime : 0.1f);
                    st = cur > step ? cur - step : cur < -step ? cur + step : 0.0f;
                }
                st = st > c.steerMax ? c.steerMax : st < -c.steerMax ? -c.steerMax : st;
            } else {                                   // wheel: the stick's position, reached at most steerSpeed / s
                st = SteerOf(x, c);
                if (c.steerSpeed > 0.0f) {
                    const float step = c.steerSpeed * t;
                    st = st > cur + step ? cur + step : st < cur - step ? cur - step : st;
                }
            }
            dv[0] = sh == 0 || sh == 1 ? st : 0.0f;
            for (int b = 0; b < kBtnCount; ++b) {      // gas / brake: the most pressed button mapped to each
                if (b == c.airBtn) continue;
                const Out& o = modOn && b != c.modBtn && c.modMap[b].kind != kNone ? c.modMap[b] : c.map[b];
                if (o.kind != kGas && o.kind != kBrake) continue;
                const float v = Pressed(f, c, b);
                float& d = dv[o.kind == kGas ? 1 : 2];
                if (v > d) d = v;
            }
        }
        if (dv[0] != drive_[0] || dv[1] != drive_[1] || dv[2] != drive_[2]) {
            for (int i = 0; i < 3; ++i) drive_[i] = dv[i];
            s.Drive(dv[0], dv[1], dv[2]);
        }
        if (!active) { accX_ = accY_ = 0.0; return; }
        const int mx = (int)accX_, my = (int)accY_;
        if (mx || my) { s.Move(mx, my); accX_ -= mx; accY_ -= my; }
    }
    bool Down(int b) const { return b >= 0 && b < kBtnCount && down_[b]; }
    bool AirOn() const { return airOn_; }
    float Driving(int i) const { return i >= 0 && i < 3 ? drive_[i] : 0.0f; }   // the last steer / gas / brake sent

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
            if (c.mouseStick != h + 1) {               // that stick is not the mouse: its directions are buttons
                const int u = h ? kRUp : kLUp;
                ana(u + 0, f.sy[h]);
                ana(u + 1, -f.sy[h]);
                if (SteerStickOf(c) != h + 1) {        // (left / right: not while that stick steers)
                    ana(u + 2, -f.sx[h]);
                    ana(u + 3, f.sx[h]);
                }
            }
        }
    }
    // how far button b is pressed, 0 .. 1 (gas / brake): a trigger / grip as a pedal, a stick direction after the dead zone,
    // any other button full; 0 for a stick direction that is not a button now (the mouse / the steering)
    static float Pressed(const Frame& f, const Config& c, int b) {
        const int h = HandOfBtn(b);
        if (!f.hand[h]) return 0.0f;
        switch (b) {
        case kLTrig: case kRTrig: return PedalOf(f.trig[h]);
        case kLGrip: case kRGrip: return PedalOf(f.grip[h]);
        case kX: case kA: return f.prim[h] ? 1.0f : 0.0f;
        case kY: case kB: return f.sec[h] ? 1.0f : 0.0f;
        case kLStick: case kRStick: return f.click[h] ? 1.0f : 0.0f;
        case kMenu: return f.menu[0] ? 1.0f : 0.0f;
        default: break;
        }
        const int d = (b - kLUp) % 4;                  // up, down, left, right
        if (c.mouseStick == h + 1 || (d >= 2 && SteerStickOf(c) == h + 1)) return 0.0f;
        const float v = d == 0 ? f.sy[h] : d == 1 ? -f.sy[h] : d == 2 ? -f.sx[h] : f.sx[h];
        if (v <= c.deadzone || c.deadzone >= 1.0f) return 0.0f;
        const float k = ((v > 1.0f ? 1.0f : v) - c.deadzone) / (1.0f - c.deadzone);
        return k > 1.0f ? 1.0f : k;
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
    Out    sent_[kBtnCount];                           // what each held button sent when it went down
    bool   airOn_ = false;
    bool   airLatch_ = false;                          // toggle mode: the air mouse is switched on
    double yaw_ = 0.0, pitch_ = 0.0;
    double accX_ = 0.0, accY_ = 0.0;
    float  drive_[3] = {};                             // steer, throttle, brake last sent
};

}  // namespace VrPad
