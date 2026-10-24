// pad_map -- VR CONTROLLERS -> keys, mouse and driving (config.ini). The pure part: the button table, the config.ini key
// names, the output parser ("H", "Shift+F", "mouse_left", "wheel_up", "gas" ...) and the per-frame Mapper (thresholds with
// hysteresis, the modifier, stick -> mouse, air mouse = the aim's yaw / pitch change while a hold button is down, steering
// and gas / brake). No Windows input calls: output.h sends the Mapper's events with SendInput, scs_input.h hands the driving
// values to the game. Header only, so tests\pad_map_test.cpp checks it without a game.
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
// kVk + a virtual key: a key with no plain scan code, sent as that virtual key (Pause: its scan sequence is E1 1D 45;
// the ETS2 / ATS DLAA mod reads its hotkeys with GetAsyncKeyState, so the virtual key is what it sees -- the user,
// 2026-10-24: "the DLAA mod captures logs of pause").
constexpr uint16_t kExt = 0x100;
constexpr uint16_t kVk = 0x200;
// kGas / kBrake: driving pedals -- not a key: how far the button is pressed (a trigger / grip / stick direction: analog, any
// other button: full) goes to the game as the gas / brake pedal (Sink::Drive, scs_input.h)
// kKeyboard: a press shows / hides the VR keyboard (vr_menu.h; Mapper::TakeKeyboardToggles)
enum OutKind : uint8_t { kNone, kKeys, kMouseL, kMouseR, kMouseM, kWheelUp, kWheelDown, kGas, kBrake, kKeyboard };
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
        { "scrolllock", 0x46 }, { "printscreen", 0x37 | kExt }, { "prtsc", 0x37 | kExt },
        { "pause", 0x13 | kVk },  // (VK_PAUSE: see kVk)
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
    if (!_stricmp(buf, "keyboard")) { o->kind = kKeyboard; return true; }
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
        "num5", "num6", "num7", "num8", "num9", "numdot", "numplus", "numminus", "nummul", "numdiv", "numenter", "scrolllock", "printscreen", "pause",
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
    case kKeyboard: strncpy_s(buf, cap, "keyboard", _TRUNCATE); return;
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
    // Two modifiers = four layers (2026-10-24, the user: "no modifier, modifier 1, modifier 2, both"): modBtn alone =
    // modMap, modBtn2 alone = modMap2, both held = modMapBoth. An empty entry in the layer held = the button's normal map
    // (never another layer's: what a button does with both grips is only what modMapBoth says). See OutOf.
    int   modBtn = kRGrip;
    Out   modMap[kBtnCount];
    int   modBtn2 = kLGrip;                            // -1 = no second modifier
    Out   modMap2[kBtnCount];
    Out   modMapBoth[kBtnCount];
    // driving -- analog values for the game's own input plugin door (scs_input.h), not keys. Steering: steerStick's x (its
    // left / right directions are then not buttons, and a mouse on that stick is off). The stick (dead zone, two zones --
    // StickShape --, times steerMax and the speed factor) is the wheel's TARGET place; the wheel glides there (Mapper::Step):
    //   - stick held (pushed further OR eased): time constant steerFollowTime -- easing the stick = a smaller angle that
    //     holds, and a wobbling thumb near the edge is smoothed (the user's drive log 2026-10-23 19:57: holds at ~0.9 wobble
    //     0.1-0.25 per 0.1 s; an unsmoothed ease made every wobble a twitch);
    //   - let go: back to straight at steerCenterTime (default 0 = at the wheel speed limit: the user's choice, "more
    //     realistic");
    //   - the stick on the OTHER side: straight first, at the speed limit times the push (no glide: with a centre time a
    //     small angle crept to straight for over a second while the stick was pushed the other way -- the user's drive log
    //     2026-10-23 20:55: "the buttons weren't steering"), then out the other way. A thumb slipping past the middle as it
    //     lets go never throws the wheel past straight;
    //   - never faster than steerRate full turns (centre -> full lock) per second, times the push while held (at least
    //     0.3): a small push turns slowly, a full push fast (the user's thumb is mostly 0 or ~0.9: taps);
    //   - times longer and the limit lower at speed (/ and * sqrt(speed factor)): calmer on the highway.
    // (2026-10-23, Fable's design: the old "turn speed" style kept turning while the stick was eased, and only the dead zone
    //  brought it back.) Gas / brake: any button mapped to "gas" / "brake" (map or modMap), see kGas.
    bool  drive = true;                                // off: no steering, gas / brake buttons send nothing, the steering
                                                       //   stick's left / right are buttons again (the menu's one switch)
    int   steerStick = 1;                              // 0 off, 1 left stick, 2 right stick
    float steerDeadzone = 0.10f;                       // stick, before steering starts
    // two zones (2026-10-23: the stick's throw is short -- 75 % of it was two turns of the wheel at low speed): the first
    // steerFineZone of the push (after the dead zone) turns the wheel up to steerFine (small, careful turns: following the
    // road), the rest up to the edge turns it on up to full (hard turns). steerFine >= steerFineZone = even all the way.
    float steerFine = 0.2f;                            // the wheel at the end of the fine zone (share of steerMax)
    float steerFineZone = 0.8f;                        // the share of the push that is the fine zone
    // the stick counts as pushed all the way from here on (the user's log: at the rim it read 0.93-0.95 nearly always, 1.0
    // rarely -- "all the way" gave half lock)
    float steerFullAt = 0.95f;
    float steerMax = 1.0f;                             // full stick = this much of the wheel (0.2 .. 1)
    float steerFollowTime = 0.4f;                      // s, toward the stick while it is held (0 = at once)
    float steerCenterTime = 0.0f;                      // s, back to straight when let go (0 = at the speed limit)
    float steerRate = 0.5f;                            // full turns per second at most (0 = no limit)
    // the limit while the wheel goes back toward straight (let go, or the stick on the other side): the user 2026-10-23
    // 22:54 log, a swing from full lock to the other side took 4 s at steerRate -- "super slow". 0 = no limit.
    float steerBackRate = 2.0f;                        // full turns per second
    // speed-sensitive steering (the truck's speed from the game, SetSpeed): up to 10 km/h the stick steers fully; at 90 km/h
    // and faster the wheel turns at most steerAtSpeed of that; the return and the rate limit go sqrt(steerAtSpeed) as fast.
    // 1 = off.
    float steerAtSpeed = 0.3f;
    bool  steerSpeedSens = true;                       // the switch (the Driving tab's On / Off)
    // what steers (2026-10-24, the user: "three different options to choose from"): 0 the stick (above), 1 a two-hand wheel
    // (hold both grips; the tilt of the line between the hands is the wheel's turn), 2 pointing with one hand (the aim's
    // yaw: straight ahead = straight). In 1 / 2 the stick still steers when pushed (it wins: a quick correction). The wheel
    // follows the hands with steerAimSmooth (no speed limit: the hands are the wheel); speed-sensitive as the stick. Let go
    // (grips open / the point button released) = back to straight as the stick's let go. Point HOLDS the wheel where it is
    // while that hand points at a panel (menu / keyboard: SetHands) or the air mouse is on (the user 2026-10-24: typing on
    // the keyboard in a bend must not throw the wheel straight); with steerPointBtn it lets go while that button is not
    // held. The wheel lets go while a hand holds a panel (the menu's grab).
    int   steerWith = 0;
    float steerWheelAngle = 90.0f;                     // degrees of hand tilt to full lock (wheel)
    float steerPointAngle = 35.0f;                     // degrees of pointing left / right to full lock (point)
    float steerAimDeadzone = 2.0f;                     // degrees around straight that do not steer (wheel and point)
    float steerAimSmooth = 0.08f;                      // s, the wheel following the hands (0 = at once)
    int   steerPointHand = 1;                          // 0 left, 1 right
    int   steerPointBtn = -1;                          // -1 = always (but the pauses above); a button = only while held
    // wheel / point: the stick steers too (it wins while pushed). Off: only the hands steer, and the steering stick's left /
    // right are buttons again (the user 2026-10-24: "an option to keep thumbstick enabled might be good")
    bool  steerStickToo = true;
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

// driving: the stick's x (-1 left .. +1 right) as the wanted steering after the dead zone and the two zones; a trigger as a pedal
// (0 .. 1)
// the steering of a stick at (x, y): a slanted push counts by its whole length while it is within ~50 degrees of sideways
// (|x| * 1.6 >= the length), steeper ones by 1.6 x. The user's log 2026-10-23 20:3x: right turns stopped at 0.65-0.9 of x --
// the thumb pushed up and to the right --, left turns reached the edge.
inline float SteerX(float x, float y) {
    const float ax = x < 0 ? -x : x;
    float len = std::sqrt(x * x + y * y);
    if (len > 1.0f) len = 1.0f;
    const float v = ax * 1.6f < len ? ax * 1.6f : len;
    return x < 0 ? -v : v;
}
// the push: 0 in the dead zone .. 1 from steerFullAt on (no sign)
inline float StickPush(float x, const Config& c) {
    const float a = x < 0 ? -x : x;
    const float top = c.steerFullAt > c.steerDeadzone + 0.05f && c.steerFullAt <= 1.0f ? c.steerFullAt : 1.0f;
    if (a <= c.steerDeadzone || c.steerDeadzone >= top) return 0.0f;
    return ((a > top ? top : a) - c.steerDeadzone) / (top - c.steerDeadzone);
}
inline float StickShape(float x, const Config& c) {   // -1 .. 1 after the dead zone, the full point and the two zones
    const float k = StickPush(x, c);
    if (k <= 0.0f) return 0.0f;
    const float z = c.steerFineZone > 0.05f && c.steerFineZone < 0.99f ? c.steerFineZone : 0.8f;
    const float fine = c.steerFine < 0.0f ? 0.0f : c.steerFine > z ? z : c.steerFine;
    float v;
    if (k <= z) {
        v = fine * k / z;                              // the fine zone: even, small
    } else {                                           // the hard zone: on at the fine zone's slope, bending up to full at the
        const float u = (k - z) / (1.0f - z);          //   edge (fine = zone: b = 0, even all the way)
        const float s0 = fine / z * (1.0f - z), b = 1.0f - fine - s0;
        v = fine + s0 * u + b * u * u;
    }
    return x < 0 ? -v : v;
}
inline float SpeedFactor(float kmh, const Config& c) {   // 1 slow .. steerAtSpeed fast; speed unknown (< 0, NaN) = 1
    if (!(kmh >= 0.0f) || !c.steerSpeedSens || c.steerAtSpeed >= 1.0f) return 1.0f;
    const float t = kmh <= 10.0f ? 0.0f : kmh >= 90.0f ? 1.0f : (kmh - 10.0f) / 80.0f;
    return 1.0f + (c.steerAtSpeed - 1.0f) * t;
}
inline int SteerStickOf(const Config& c) {              // the stick that steers now (0 none)
    return c.drive && (c.steerWith == 0 || c.steerStickToo) ? c.steerStick : 0;
}
inline float SteerOf(float x, const Config& c) { return StickShape(x, c) * c.steerMax; }   // the target at low speed
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

// the layer held now: 0 none, 1 modifier 1, 2 modifier 2, 3 both (want = the buttons held)
inline int LayerOf(const Config& c, const bool* want) {
    const bool m1 = c.modBtn >= 0 && c.modBtn < kBtnCount && want[c.modBtn];
    const bool m2 = c.modBtn2 >= 0 && c.modBtn2 < kBtnCount && c.modBtn2 != c.modBtn && want[c.modBtn2];
    return (m1 ? 1 : 0) | (m2 ? 2 : 0);
}
inline Out* LayerMap(Config& c, int layer) {           // the layer's own entries (layer 1 .. 3); nullptr for 0
    return layer == 1 ? c.modMap : layer == 2 ? c.modMap2 : layer == 3 ? c.modMapBoth : nullptr;
}
inline const Out* LayerMap(const Config& c, int layer) {
    return layer == 1 ? c.modMap : layer == 2 ? c.modMap2 : layer == 3 ? c.modMapBoth : nullptr;
}
inline bool IsModifier(const Config& c, int b) { return b >= 0 && (b == c.modBtn || b == c.modBtn2); }
// what button b sends in a layer: the layer's entry, or its normal map when that is empty (a modifier keeps its own job)
inline const Out& OutOf(const Config& c, int layer, int b) {
    const Out* t = LayerMap(c, layer);
    return t && !IsModifier(c, b) && t[b].kind != kNone ? t[b] : c.map[b];
}

class Mapper {
public:
    // active = send anything at all (enabled, session focused, the game window in front). Inactive = everything held is
    // released once. dt = seconds since the last Step.
    void Step(const Frame& f, bool active, double dt, const Config& c, Sink& s) {
        bool want[kBtnCount] = {};
        if (active) Want(f, c, want);
        const int layer = LayerOf(c, want);
        for (int b = 0; b < kBtnCount; ++b) {
            if (want[b] == down_[b]) continue;
            down_[b] = want[b];
            if (b == c.airBtn) {                       // the air mouse button sends nothing itself
                if (want[b]) airLatch_ = !airLatch_;   // (toggle mode: each press switches)
                continue;
            }
            if (want[b]) {                             // a modifier held: the layer's entry for this button, if it has one
                sent_[b] = OutOf(c, layer, b);
                notPedal_[b] = sent_[b].kind != kGas && sent_[b].kind != kBrake;   // (a click: no gas until let go)
                if (sent_[b].kind == kKeyboard) ++kbToggles_;
                Send(sent_[b], true, s);
            } else {
                Send(sent_[b], false, s);              // (released as it was pressed, whatever the modifier does now)
            }
        }
        if (!active || !c.airToggle) airLatch_ = false;
        // air mouse
        const int ah = c.airHand & 1;
        const bool airBtnOn = c.airBtn >= 0 && c.airBtn < kBtnCount && (c.airToggle ? airLatch_ : down_[c.airBtn]);
        const bool air = active && airBtnOn && f.aim[ah] && !(aimOn_ && c.steerWith == 1);   // (not while both hands hold the
                                                                                          //  wheel; pointing pauses for it)
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
            const int sh = SteerStickOf(c) - 1;
            float x = (sh == 0 || sh == 1) && f.hand[sh] ? f.sx[sh] : 0.0f;
            float y = (sh == 0 || sh == 1) && f.hand[sh] ? f.sy[sh] : 0.0f;
            if (!(x == x)) x = 0.0f;                   // (a NaN stick would stay in the wheel for good)
            if (!(y == y)) y = 0.0f;
            stickX_ = x; stickY_ = y;
            x = SteerX(x, y);
            const float t = (float)(dt > 0.0 ? (dt < 0.1 ? dt : 0.1) : 0.0);   // (a hitch = one bounded step)
            const float cur = drive_[0];
            // faster = less wheel (the target range times sf), and a calmer return / rate limit (times sqrt(sf): the user's
            // drive logs moved the wheel at most ~0.13 / s at 85-92 km/h). Speeding up in a bend with the stick held keeps
            // the range the bend began with (range_), so the bend stays; easing the stick (past a little thumb jitter),
            // letting go or steering the other way takes the range of the speed now, and the wheel glides in.
            const float sf = SpeedFactor(speed_, c), rf = std::sqrt(sf);
            // (the hold rule and the speed limit go by the push, not the wheel shape: the shape is flat in the fine zone and
            //  steep in the hard zone, the thumb's jitter is the same everywhere)
            float shape = StickShape(x, c), push = StickPush(x, c);
            float av = 0.0f;                           // the hands (steerWith 1 / 2): only while the stick is not pushed
            const bool aimed = AimSteer(f, c, t, sf, cur, &av) && shape == 0.0f;
            if (aimed) { shape = av; push = av < 0.0f ? -av : av; }
            if (shape == 0.0f || shape * holdSign_ < 0.0f || sf >= range_ || push < holdPeak_ - 0.03f) {
                range_ = sf;
                holdPeak_ = push;
            } else if (push > holdPeak_) {
                holdPeak_ = push;
            }
            holdSign_ = shape;
            float tgt = shape * c.steerMax * range_;
            const bool other = !aimed && tgt * cur < 0.0f;   // the other side: straight first, no glide (see Config)
            if (other) tgt = 0.0f;
            const bool letGo = tgt == 0.0f && !other && !aimed;
            const float tau = aimed ? c.steerAimSmooth : other ? 0.0f : (letGo ? c.steerCenterTime : c.steerFollowTime) / rf;
            const float a = t <= 0.0f ? 0.0f : tau > 0.0f ? t / (tau + t) : 1.0f;   // first order, stable for any step
            float step = (tgt - cur) * a;
            const bool back = step * cur < 0.0f;       // toward straight: the faster back limit
            const float rate = aimed ? 0.0f : back ? c.steerBackRate : c.steerRate;   // (the hands: no limit)
            if (rate > 0.0f) {
                const float m = rate * rf * t * (letGo ? 1.0f : push > 0.3f ? push : 0.3f);
                step = step > m ? m : step < -m ? -m : step;
                if (back && (cur + step) * cur < 0.0f) step = -cur;   // (back stops at straight; out the other way next step)
            }
            float st = cur + step;
            if (tgt == 0.0f && st < 0.002f && st > -0.002f) st = 0.0f;   // (the glide's tail: exactly straight)
            dv[0] = sh == 0 || sh == 1 || c.steerWith == 1 || c.steerWith == 2 ? st : 0.0f;
            for (int b = 0; b < kBtnCount; ++b) {      // gas / brake: the most pressed button mapped to each
                if (b == c.airBtn) continue;
                const float v = Pressed(f, c, b);
                if (notPedal_[b]) {                    // pressed as a key / click (e.g. grip + trigger = click, then the
                    if (v <= 0.0f && !down_[b]) notPedal_[b] = false;   // grip let go first): no gas until fully let go
                    continue;
                }
                const Out& o = OutOf(c, layer, b);
                if (o.kind != kGas && o.kind != kBrake) continue;
                float& d = dv[o.kind == kGas ? 1 : 2];
                if (v > d) d = v;
            }
        }
        if (!active) for (bool& n : notPedal_) n = false;
        if (!active || !c.drive) { range_ = 0.0f; holdPeak_ = holdSign_ = 0.0f; aimOn_ = false; aimLost_ = 0.0f; }
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
    void SetSpeed(float kmh) { speed_ = kmh; }          // the truck's speed (km/h, < 0 = not known): speed-sensitive steering
    float StickX() const { return stickX_; }            // the steering stick's x / y at the last Step (log = 1: the drive line)
    float StickY() const { return stickY_; }
    int TakeKeyboardToggles() { const int n = kbToggles_; kbToggles_ = 0; return n; }   // "keyboard" presses since the last call
    // the VR menu's use of each hand this frame (before Step): pointing at a panel (point steering pauses), holding one
    // (a grab / a trigger on it: the wheel lets go)
    void SetHands(bool pointL, bool pointR, bool holdL, bool holdR) {
        handPoint_[0] = pointL; handPoint_[1] = pointR; handHold_[0] = holdL; handHold_[1] = holdR;
    }
    bool AimSteering() const { return aimOn_; }        // the hands steer now (wheel held / pointing)

private:
    // steering with the hands (Config::steerWith): true + *v (-1 .. 1, the wheel's share before steerMax and the speed range)
    // while they steer. Tracking lost for a moment (< 0.3 s) keeps the last value. sf = the speed factor now, cur = the
    // wheel now (the wheel's grab is relative: grabbing does not move it).
    bool AimSteer(const Frame& f, const Config& c, float t, float sf, float cur, float* v) {
        bool want = false, seen = false;
        double ang = 0.0, lock = 1.0;
        if (c.steerWith == 1) {                        // the wheel: both grips held, no hand on a panel
            const float g = aimOn_ ? kReleaseAt : kPressAt;
            want = f.hand[0] && f.hand[1] && f.grip[0] > g && f.grip[1] > g && !handHold_[0] && !handHold_[1];
            if (want && f.pos[0] && f.pos[1]) {        // left hand higher = a right turn, whichever way the player faces
                const double dx = f.p[1][0] - f.p[0][0], dz = f.p[1][2] - f.p[0][2];
                ang = std::atan2(f.p[0][1] - f.p[1][1], std::sqrt(dx * dx + dz * dz)) * 57.2957795;
                seen = true;
            }
            lock = c.steerWheelAngle;
        } else if (c.steerWith == 2) {                 // pointing: that hand's yaw, unless it is busy elsewhere
            const int h = c.steerPointHand & 1;
            want = f.hand[h] && (c.steerPointBtn < 0 || c.steerPointBtn >= kBtnCount || down_[c.steerPointBtn]);
            if (want && (handPoint_[h] || handHold_[h] || airOn_)) {   // busy on a panel / the air mouse: the wheel holds
                if (!aimOn_) return false;
                aimLost_ = 0.0f;
                *v = aimLast_;
                return true;
            }
            if (want && f.aim[h]) {
                double yaw, pitch;
                YawPitch(f.q[h], &yaw, &pitch);
                ang = yaw * 57.2957795;
                seen = true;
            }
            lock = c.steerPointAngle;
        }
        if (!want) { aimOn_ = false; aimLost_ = 0.0f; return false; }
        if (lock < 5.0) lock = 5.0;
        if (!seen) {                                   // tracking lost: hold a moment, then let go
            aimLost_ += t;
            if (!aimOn_ || aimLost_ > 0.3f) { aimOn_ = false; return false; }
            *v = aimLast_;
            return true;
        }
        aimLost_ = 0.0f;
        if (!aimOn_) {                                 // the grab: the wheel keeps its place (point: absolute, ahead = straight)
            const float k = c.steerMax * sf;
            aimRef_ = c.steerWith == 1 && k > 0.01f ? ang - cur / k * lock : 0.0;
            aimOn_ = true;
        }
        double d = ang - aimRef_;
        const double dz = c.steerAimDeadzone < lock * 0.5 ? c.steerAimDeadzone : lock * 0.5;
        const double m = d < 0 ? -d : d;
        d = m <= dz ? 0.0 : (d < 0 ? -1.0 : 1.0) * (m - dz) / (lock - dz);
        aimLast_ = (float)(d > 1.0 ? 1.0 : d < -1.0 ? -1.0 : d);
        *v = aimLast_;
        return true;
    }
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
    bool   notPedal_[kBtnCount] = {};                  // pressed as a key / click: not a pedal until fully let go
    int    kbToggles_ = 0;
    float  speed_ = -1.0f;                             // km/h (SetSpeed); < 0 = not known
    Out    sent_[kBtnCount];                           // what each held button sent when it went down
    bool   airOn_ = false;
    bool   airLatch_ = false;                          // toggle mode: the air mouse is switched on
    double yaw_ = 0.0, pitch_ = 0.0;
    double accX_ = 0.0, accY_ = 0.0;
    float  drive_[3] = {};                             // steer, throttle, brake last sent
    float  stickX_ = 0.0f, stickY_ = 0.0f;             // the steering stick's x / y (StickX / StickY)
    float  range_ = 0.0f;                              // steering: the speed range of the bend held now (see Step)
    float  holdPeak_ = 0.0f, holdSign_ = 0.0f;         // ... the most push in it (StickPush), and its side
    bool   aimOn_ = false;                             // the hands steer (AimSteer)
    double aimRef_ = 0.0;                              // ... the angle that is straight (wheel: set at the grab)
    float  aimLast_ = 0.0f, aimLost_ = 0.0f;           // ... the last value, and how long tracking has been lost (s)
    bool   handPoint_[2] = {}, handHold_[2] = {};      // the VR menu's use of each hand (SetHands)
};

}  // namespace VrPad
