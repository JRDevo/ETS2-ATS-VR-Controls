// config -- see config.h.
#include "config.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cwctype>

namespace ck {
namespace {

std::string Trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) ++a;
    while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r' || s[b - 1] == '\n')) --b;
    return s.substr(a, b - a);
}
std::string Lower(std::string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}
bool Num(const std::string& v, double lo, double hi, double* out) {
    char* end = nullptr;
    const double d = strtod(v.c_str(), &end);
    if (end == v.c_str() || *end) return false;
    *out = d < lo ? lo : (d > hi ? hi : d);
    return true;
}

}  // namespace

Config ParseConfig(const std::string& text) {
    Config c;
    c.apps = { L"eurotrucks2.exe", L"amtrucks.exe" };
    size_t pos = 0;
    int lineNo = 0;
    bool mod2Set = false;                              // (modifier2 in the file: a clash with modifier is then noted)
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string::npos) nl = text.size();
        const std::string line = Trim(text.substr(pos, nl - pos));
        pos = nl + 1;
        ++lineNo;
        if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[') continue;
        const size_t eq = line.find('=');
        if (eq == std::string::npos) { c.notes.push_back("line " + std::to_string(lineNo) + ": no '=' -- ignored"); continue; }
        const std::string key = Lower(Trim(line.substr(0, eq)));
        // a comment after the value: from '#', or from a ';' after a space ("x = ;" is the ; key, "a = enter ; ok" a comment)
        std::string raw = line.substr(eq + 1);
        const size_t hash = raw.find('#');
        if (hash != std::string::npos) raw.resize(hash);
        raw = Trim(raw);
        for (size_t k = 1; k < raw.size(); ++k)
            if (raw[k] == ';' && (raw[k - 1] == ' ' || raw[k - 1] == '\t')) { raw.resize(k); break; }
        const std::string val = Trim(raw);
        const std::string lv = Lower(val);
        auto bad = [&](const char* why) { c.notes.push_back("line " + std::to_string(lineNo) + ": " + key + " = " + val + " -- " + why); };
        double d = 0;
        if (key == "enabled") { c.enabled = lv != "0" && lv != "off" && lv != "no"; continue; }
        if (key == "log") { c.log = lv == "1" || lv == "on" || lv == "yes"; continue; }
        if (key == "log_inputs") { c.logInputs = lv == "1" || lv == "on" || lv == "yes"; continue; }
        if (key == "apps") {
            c.apps.clear();
            size_t a = 0;
            while (a <= val.size()) {
                size_t comma = val.find(',', a);
                if (comma == std::string::npos) comma = val.size();
                const std::string one = Lower(Trim(val.substr(a, comma - a)));
                if (!one.empty()) c.apps.push_back(std::wstring(one.begin(), one.end()));
                a = comma + 1;
            }
            if (c.apps.empty()) bad("empty: the layer stays off in every program");
            continue;
        }
        if (key == "mouse_stick") {
            if (lv == "left") c.pad.mouseStick = 1;
            else if (lv == "right") c.pad.mouseStick = 2;
            else if (lv == "off" || lv == "none" || lv == "0") c.pad.mouseStick = 0;
            else bad("use left, right or off");
            continue;
        }
        if (key == "mouse_speed") { if (Num(val, 50, 10000, &d)) c.pad.mouseSpeed = (float)d; else bad("a number (pixels per second)"); continue; }
        if (key == "deadzone")    { if (Num(val, 0, 0.9, &d)) c.pad.deadzone = (float)d; else bad("a number 0 .. 0.9"); continue; }
        if (key == "steer") {
            if (lv == "left") c.pad.steerStick = 1;
            else if (lv == "right") c.pad.steerStick = 2;
            else if (lv == "off" || lv == "none" || lv == "0") c.pad.steerStick = 0;
            else bad("use left, right or off");
            continue;
        }
        if (key == "drive") { c.pad.drive = lv != "0" && lv != "off" && lv != "no"; continue; }
        // (the steering styles before 2026-10-23: no longer used -- a note, so a user's old tuning is not lost without a word)
        if (key == "steer_mode" || key == "steer_turn_time" || key == "steer_return_time" || key == "steer_speed") {
            bad("no longer used (2026-10-23: the wheel glides to the stick's place; see steer_follow_time, steer_center_time, "
                "steer_max_speed)");
            continue;
        }
        if (key == "steer_speed_sens") { c.pad.steerSpeedSens = lv != "0" && lv != "off" && lv != "no"; continue; }
        if (key == "steer_at_speed") { if (Num(val, 0.1, 1, &d)) c.pad.steerAtSpeed = (float)d; else bad("0.1 .. 1 (1 = off)"); continue; }
        if (key == "steer_follow_time") { if (Num(val, 0, 2, &d)) c.pad.steerFollowTime = (float)d; else bad("seconds 0 .. 2 (0 = at once)"); continue; }
        if (key == "steer_center_time") { if (Num(val, 0, 10, &d)) c.pad.steerCenterTime = (float)d; else bad("seconds 0 .. 10 (0 = at once)"); continue; }
        if (key == "steer_max_speed") { if (Num(val, 0, 10, &d)) c.pad.steerRate = (float)d; else bad("full turns per second 0 .. 10 (0 = no limit)"); continue; }
        if (key == "steer_with") {                      // what steers: the stick, a two-hand wheel, pointing with one hand
            if (lv == "stick") c.pad.steerWith = 0;
            else if (lv == "wheel") c.pad.steerWith = 1;
            else if (lv == "point") c.pad.steerWith = 2;
            else bad("stick, wheel or point");
            continue;
        }
        if (key == "steer_wheel_angle") { if (Num(val, 10, 180, &d)) c.pad.steerWheelAngle = (float)d; else bad("degrees 10 .. 180"); continue; }
        if (key == "steer_point_angle") { if (Num(val, 10, 90, &d)) c.pad.steerPointAngle = (float)d; else bad("degrees 10 .. 90"); continue; }
        if (key == "steer_aim_deadzone") { if (Num(val, 0, 10, &d)) c.pad.steerAimDeadzone = (float)d; else bad("degrees 0 .. 10"); continue; }
        if (key == "steer_aim_smooth") { if (Num(val, 0, 1, &d)) c.pad.steerAimSmooth = (float)d; else bad("seconds 0 .. 1 (0 = at once)"); continue; }
        if (key == "steer_point_hand") {
            if (lv == "left") c.pad.steerPointHand = 0;
            else if (lv == "right") c.pad.steerPointHand = 1;
            else bad("left or right");
            continue;
        }
        if (key == "steer_wheel_grip") {
            if (lv == "both") c.pad.steerWheelGrip = 0;
            else if (lv == "left") c.pad.steerWheelGrip = 1;
            else if (lv == "right") c.pad.steerWheelGrip = 2;
            else if (lv == "either") c.pad.steerWheelGrip = 3;
            else bad("both, left, right or either");
            continue;
        }
        if (key == "steer_stick_too") { c.pad.steerStickToo = lv != "0" && lv != "off" && lv != "no"; continue; }
        if (key == "steer_point_button") {
            if (lv == "off" || lv == "none" || lv == "0") { c.pad.steerPointBtn = -1; continue; }
            const int b = VrPad::BtnOfName(lv.c_str());
            if (b < 0 || b >= VrPad::kLUp) { bad("a button name such as lgrip, or none (= always)"); continue; }
            c.pad.steerPointBtn = b;
            continue;
        }
        if (key == "steer_back_speed") { if (Num(val, 0, 10, &d)) c.pad.steerBackRate = (float)d; else bad("full turns per second 0 .. 10 (0 = no limit)"); continue; }
        if (key == "steer_deadzone") { if (Num(val, 0, 0.5, &d)) c.pad.steerDeadzone = (float)d; else bad("a number 0 .. 0.5"); continue; }
        if (key == "steer_curve") { bad("no longer used (2026-10-23: see steer_fine, steer_fine_zone)"); continue; }
        if (key == "steer_fine") { if (Num(val, 0.05, 0.95, &d)) c.pad.steerFine = (float)d; else bad("a number 0.05 .. 0.95"); continue; }
        if (key == "steer_full_at") { if (Num(val, 0.7, 1, &d)) c.pad.steerFullAt = (float)d; else bad("a number 0.7 .. 1"); continue; }
        if (key == "steer_fine_zone") { if (Num(val, 0.5, 0.95, &d)) c.pad.steerFineZone = (float)d; else bad("a number 0.5 .. 0.95"); continue; }
        if (key == "steer_max")   { if (Num(val, 0.2, 1, &d)) c.pad.steerMax = (float)d; else bad("a number 0.2 .. 1"); continue; }
        if (key == "air_speed")   { if (Num(val, 1, 500, &d)) c.pad.airGain = (float)d; else bad("a number (pixels per degree)"); continue; }
        if (key == "air_mode") {
            if (lv == "hold") c.pad.airToggle = false;
            else if (lv == "toggle") c.pad.airToggle = true;
            else bad("use hold or toggle");
            continue;
        }
        if (key == "theme") {
            int t = -1;
            for (int i = 0; i < kThemeCount; ++i) if (lv == ThemeNames()[i]) t = i;
            if (t < 0) bad("dark, blue, grey, light, black or green"); else c.look.theme = t;
            continue;
        }
        if (key == "menu_opacity" || key == "keyboard_opacity") {
            if (!Num(val, 0.2, 1.0, &d)) { bad("a number 0.2 .. 1 (1 = solid)"); continue; }
            (key == "menu_opacity" ? c.look.menuOpacity : c.look.kbOpacity) = (float)d;
            continue;
        }
        if (key == "pointer_smoothing") {
            int t = -1;
            for (int i = 0; i < 4; ++i) if (lv == SmoothNames()[i]) t = i;
            if (t < 0) bad("off, low, medium or high"); else c.look.smoothing = t;
            continue;
        }
        if (key == "keyboard_layout") {
            if (lv == "full") c.look.kbCompact = false;
            else if (lv == "compact") c.look.kbCompact = true;
            else bad("full or compact");
            continue;
        }
        if (key == "keyboard_show") { c.look.kbShow = lv == "1" || lv == "on" || lv == "yes"; continue; }
        if (key == "vr_menu") { c.menu = lv != "0" && lv != "off" && lv != "no"; continue; }
        if (key == "menu_hold") { if (Num(val, 0.3, 5, &d)) c.menuHold = (float)d; else bad("a number of seconds (0.3 .. 5)"); continue; }
        if (key == "menu_combo") {
            std::vector<int> combo;
            bool ok = true;
            size_t a = 0;
            while (ok && a <= lv.size()) {
                size_t plus = lv.find('+', a);
                if (plus == std::string::npos) plus = lv.size();
                const int b = VrPad::BtnOfName(Trim(lv.substr(a, plus - a)).c_str());
                for (int x : combo) ok = ok && x != b;
                ok = ok && b >= 0 && b < VrPad::kLUp;
                combo.push_back(b);
                a = plus + 1;
            }
            if (!ok || combo.size() < 2 || combo.size() > 4) { bad("2 to 4 different buttons with + (lgrip+rgrip+lstick); no stick directions"); continue; }
            c.menuCombo = combo;
            continue;
        }
        if (key == "menu_place" || key == "keyboard_place") {
            PanelPlace p;
            double v[7] = {};
            int n = 0;
            size_t a = 0;
            bool ok = true;
            while (ok && a <= val.size() && n < 8) {
                size_t comma = val.find(',', a);
                if (comma == std::string::npos) comma = val.size();
                const std::string one = Trim(val.substr(a, comma - a));
                if (n < 7) ok = Num(one, -1000, 1000, &v[n]);
                ++n;
                a = comma + 1;
            }
            if (!ok || (n != 6 && n != 7) || v[5] < 0.1 || v[5] > 3.0) { bad("x, y, z, yaw, pitch, width, roll (6 or 7 numbers; width 0.1 .. 3 m)"); continue; }
            p.set = true;
            p.x = v[0]; p.y = v[1]; p.z = v[2]; p.yaw = v[3]; p.pitch = v[4]; p.width = v[5]; p.roll = n == 7 ? v[6] : 0.0;
            (key == "menu_place" ? c.menuPlace : c.kbPlace) = p;
            continue;
        }
        if (key == "modifier" || key == "modifier2") {  // the two modifiers (see VrPad::Config: four layers)
            int& m = key == "modifier" ? c.pad.modBtn : c.pad.modBtn2;
            if (key == "modifier2") mod2Set = true;
            if (lv == "off" || lv == "none" || lv == "0") { m = -1; continue; }
            const int b = VrPad::BtnOfName(lv.c_str());
            if (b < 0 || b >= VrPad::kLUp) { bad("a button name such as rgrip, or off"); continue; }
            m = b;
            continue;
        }
        if (key == "modifier_keys" || key == "modifier2_keys" || key == "modifier_both_keys") {
            // "rtrigger:mouse_right | a:shift+f" (all of them; none = no entries)
            VrPad::Out* t = key == "modifier_keys" ? c.pad.modMap : key == "modifier2_keys" ? c.pad.modMap2 : c.pad.modMapBoth;
            for (int i = 0; i < VrPad::kBtnCount; ++i) t[i] = VrPad::Out();
            if (lv == "none" || lv.empty()) continue;
            size_t a = 0;
            while (a <= val.size()) {
                size_t bar = val.find('|', a);
                if (bar == std::string::npos) bar = val.size();
                const std::string item = Trim(val.substr(a, bar - a));
                a = bar + 1;
                if (item.empty()) continue;
                const size_t colon = item.find(':');
                const int b = colon == std::string::npos ? -1 : VrPad::BtnOfName(Lower(Trim(item.substr(0, colon))).c_str());
                VrPad::Out o;
                if (b < 0 || !VrPad::ParseOut(item.substr(colon + 1).c_str(), &o)) { bad(("'" + item + "': button:key").c_str()); continue; }
                t[b] = o;
            }
            continue;
        }
        if (key == "air_mouse") {
            if (lv == "off" || lv == "none" || lv == "0") { c.pad.airBtn = -1; continue; }
            const int b = VrPad::BtnOfName(lv.c_str());
            if (b < 0 || b >= VrPad::kLUp) { bad("a button name such as rgrip, or off"); continue; }
            c.pad.airBtn = b;
            c.pad.airHand = VrPad::HandOfBtn(b);
            continue;
        }
        const int b = VrPad::BtnOfName(key.c_str());
        if (b >= 0) {
            VrPad::Out o;
            if (VrPad::ParseOut(val.c_str(), &o)) c.pad.map[b] = o;
            else bad("not a key name / mouse_left / mouse_right / mouse_middle / wheel_up / wheel_down / gas / brake / keyboard / none");
            continue;
        }
        c.notes.push_back("line " + std::to_string(lineNo) + ": unknown key '" + key + "' -- ignored");
    }
    // the two modifiers: not the same button, and never both stick clicks (VR EyeCapture records on that pair)
    if (c.pad.modBtn2 >= 0 && c.pad.modBtn2 == c.pad.modBtn) {   // (an old file with modifier = lgrip: quietly off)
        if (mod2Set) c.notes.push_back("modifier2 is the same button as modifier -- modifier2 off");
        c.pad.modBtn2 = -1;
    }
    if ((c.pad.modBtn == VrPad::kLStick && c.pad.modBtn2 == VrPad::kRStick) || (c.pad.modBtn == VrPad::kRStick && c.pad.modBtn2 == VrPad::kLStick)) {
        c.notes.push_back("modifier + modifier2 = both stick clicks (VR EyeCapture's record combo) -- modifier2 off");
        c.pad.modBtn2 = -1;
    }
    return c;
}

Config LoadConfig(const std::wstring& dllDir) {
    wchar_t env[MAX_PATH];
    std::wstring path = GetEnvironmentVariableW(L"VRC_CONFIG", env, MAX_PATH) > 0 ? std::wstring(env) : dllDir + L"\\config.ini";
    std::string text;
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"rb") == 0 && f) {
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) text.append(buf, n);
        fclose(f);
        if (text.size() >= 3 && (unsigned char)text[0] == 0xEF && (unsigned char)text[1] == 0xBB && (unsigned char)text[2] == 0xBF)
            text.erase(0, 3);
        Config c = ParseConfig(text);
        c.loadedFrom = path;
        return c;
    }
    Config c = ParseConfig(std::string());
    c.notes.push_back("no config.ini -- the defaults are used");
    return c;
}

std::string PlaceString(const PanelPlace& p) {
    char b[160];
    snprintf(b, sizeof(b), "%.3f, %.3f, %.3f, %.1f, %.1f, %.3f, %.1f", p.x, p.y, p.z, p.yaw, p.pitch, p.width, p.roll);
    return b;
}

std::vector<std::pair<std::string, std::string>> MenuValues(const VrPad::Config& p, const PanelPlace& menu, const PanelPlace& kb,
                                                            const MenuLook& look) {
    std::vector<std::pair<std::string, std::string>> kv;
    char b[64];
    for (int i = 0; i < VrPad::kBtnCount; ++i) {
        VrPad::OutToString(p.map[i], b, sizeof(b));
        kv.push_back({ VrPad::BtnNames()[i], b });
    }
    kv.push_back({ "mouse_stick", p.mouseStick == 1 ? "left" : p.mouseStick == 2 ? "right" : "off" });
    snprintf(b, sizeof(b), "%.0f", (double)p.mouseSpeed);
    kv.push_back({ "mouse_speed", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.deadzone);
    kv.push_back({ "deadzone", b });
    kv.push_back({ "air_mouse", p.airBtn >= 0 ? VrPad::BtnNames()[p.airBtn] : "off" });
    kv.push_back({ "air_mode", p.airToggle ? "toggle" : "hold" });
    kv.push_back({ "modifier", p.modBtn >= 0 ? VrPad::BtnNames()[p.modBtn] : "off" });
    kv.push_back({ "modifier2", p.modBtn2 >= 0 ? VrPad::BtnNames()[p.modBtn2] : "off" });
    const struct { const char* key; const VrPad::Out* t; } layers[] = {
        { "modifier_keys", p.modMap }, { "modifier2_keys", p.modMap2 }, { "modifier_both_keys", p.modMapBoth } };
    for (const auto& l : layers) {
        std::string mk;
        for (int i = 0; i < VrPad::kBtnCount; ++i) {
            if (l.t[i].kind == VrPad::kNone) continue;
            VrPad::OutToString(l.t[i], b, sizeof(b));
            mk += (mk.empty() ? "" : " | ") + std::string(VrPad::BtnNames()[i]) + ":" + b;
        }
        kv.push_back({ l.key, mk.empty() ? "none" : mk });
    }
    snprintf(b, sizeof(b), "%.0f", (double)p.airGain);
    kv.push_back({ "air_speed", b });
    kv.push_back({ "drive", p.drive ? "1" : "0" });
    kv.push_back({ "steer", p.steerStick == 1 ? "left" : p.steerStick == 2 ? "right" : "off" });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerDeadzone);
    kv.push_back({ "steer_deadzone", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerFine);
    kv.push_back({ "steer_fine", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerFineZone);
    kv.push_back({ "steer_fine_zone", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerFullAt);
    kv.push_back({ "steer_full_at", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerMax);
    kv.push_back({ "steer_max", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerFollowTime);
    kv.push_back({ "steer_follow_time", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerCenterTime);
    kv.push_back({ "steer_center_time", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerRate);
    kv.push_back({ "steer_max_speed", b });
    snprintf(b, sizeof(b), "%.2f", (double)p.steerAtSpeed);
    kv.push_back({ "steer_at_speed", b });
    kv.push_back({ "steer_speed_sens", p.steerSpeedSens ? "1" : "0" });
    kv.push_back({ "steer_with", p.steerWith == 1 ? "wheel" : p.steerWith == 2 ? "point" : "stick" });
    snprintf(b, sizeof(b), "%.0f", (double)p.steerWheelAngle);
    kv.push_back({ "steer_wheel_angle", b });
    snprintf(b, sizeof(b), "%.0f", (double)p.steerPointAngle);
    kv.push_back({ "steer_point_angle", b });
    kv.push_back({ "steer_stick_too", p.steerStickToo ? "1" : "0" });
    static const char* const kGrip[4] = { "both", "left", "right", "either" };
    kv.push_back({ "steer_wheel_grip", kGrip[p.steerWheelGrip & 3] });
    kv.push_back({ "steer_point_hand", p.steerPointHand ? "right" : "left" });
    if (menu.set) kv.push_back({ "menu_place", PlaceString(menu) });
    if (kb.set) kv.push_back({ "keyboard_place", PlaceString(kb) });
    kv.push_back({ "theme", ThemeNames()[look.theme < 0 || look.theme >= kThemeCount ? 0 : look.theme] });
    snprintf(b, sizeof(b), "%.2f", (double)look.menuOpacity);
    kv.push_back({ "menu_opacity", b });
    snprintf(b, sizeof(b), "%.2f", (double)look.kbOpacity);
    kv.push_back({ "keyboard_opacity", b });
    kv.push_back({ "keyboard_show", look.kbShow ? "1" : "0" });
    kv.push_back({ "keyboard_layout", look.kbCompact ? "compact" : "full" });
    kv.push_back({ "pointer_smoothing", SmoothNames()[look.smoothing < 0 || look.smoothing > 3 ? 2 : look.smoothing] });
    return kv;
}

bool SaveConfigValues(const std::wstring& path, const std::vector<std::pair<std::string, std::string>>& kv) {
    if (path.empty()) return false;
    std::string text;
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"rb") == 0 && f) {
        char buf[4096];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), f)) > 0) text.append(buf, n);
        fclose(f);
    }
    const bool crlf = text.find("\r\n") != std::string::npos;
    std::vector<std::string> lines;
    size_t pos = 0;
    while (pos < text.size()) {
        size_t nl = text.find('\n', pos);
        if (nl == std::string::npos) nl = text.size();
        std::string l = text.substr(pos, nl - pos);
        if (!l.empty() && l.back() == '\r') l.pop_back();
        lines.push_back(l);
        pos = nl + 1;
    }
    bool header = false;
    for (const std::string& l : lines) header = header || l.find("# ---- saved by the VR menu") == 0;
    for (const auto& e : kv) {
        bool done = false;
        for (std::string& l : lines) {
            const std::string t = Trim(l);
            if (t.empty() || t[0] == '#' || t[0] == ';') continue;
            const size_t eq = t.find('=');
            if (eq == std::string::npos || Lower(Trim(t.substr(0, eq))) != e.first) continue;
            l = e.first + " = " + e.second;
            done = true;
            break;
        }
        if (done) continue;
        if (!header) {
            if (!lines.empty() && !Trim(lines.back()).empty()) lines.push_back("");
            lines.push_back("# ---- saved by the VR menu (in the headset: hold both grips + the left stick click 1 s) ----------------------------");
            header = true;
        }
        lines.push_back(e.first + " = " + e.second);
    }
    std::string out;
    for (const std::string& l : lines) out += l + (crlf ? "\r\n" : "\n");
    const std::wstring tmp = path + L".tmp";
    FILE* w = nullptr;
    if (_wfopen_s(&w, tmp.c_str(), L"wb") || !w) return false;
    const bool ok = fwrite(out.data(), 1, out.size(), w) == out.size();
    fclose(w);
    if (!ok || !MoveFileExW(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) { DeleteFileW(tmp.c_str()); return false; }
    return true;
}

bool AppAllowed(const Config& c, const std::wstring& exeName) {
    std::wstring e;
    for (wchar_t ch : exeName) e += (wchar_t)towlower(ch);
    for (const std::wstring& a : c.apps) if (a == L"*" || a == e) return true;
    return false;
}

std::string DescribePad(const VrPad::Config& p) {
    std::string s;
    for (int b = 0; b < VrPad::kBtnCount; ++b) {
        if (b == p.airBtn) { s += std::string(VrPad::BtnNames()[b]) + "=air mouse (held) "; continue; }
        const VrPad::Out& o = p.map[b];
        if (o.kind == VrPad::kNone) continue;
        if ((p.mouseStick == 1 && b >= VrPad::kLUp && b <= VrPad::kLRight) || (p.mouseStick == 2 && b >= VrPad::kRUp)) continue;
        if (VrPad::SteerStickOf(p) == (b >= VrPad::kRUp ? 2 : 1) && b >= VrPad::kLUp && (b - VrPad::kLUp) % 4 >= 2) continue;
        char v[48];
        switch (o.kind) {
        case VrPad::kMouseL: snprintf(v, sizeof(v), "mouse_left"); break;
        case VrPad::kMouseR: snprintf(v, sizeof(v), "mouse_right"); break;
        case VrPad::kMouseM: snprintf(v, sizeof(v), "mouse_middle"); break;
        case VrPad::kWheelUp: snprintf(v, sizeof(v), "wheel_up"); break;
        case VrPad::kWheelDown: snprintf(v, sizeof(v), "wheel_down"); break;
        case VrPad::kGas: snprintf(v, sizeof(v), "gas"); break;
        case VrPad::kBrake: snprintf(v, sizeof(v), "brake"); break;
        case VrPad::kKeyboard: snprintf(v, sizeof(v), "keyboard"); break;
        default: {
            int k = snprintf(v, sizeof(v), "scan");
            for (int i = 0; i < o.n && k > 0 && k < (int)sizeof(v); ++i) k += snprintf(v + k, sizeof(v) - k, "%s%03x", i ? "+" : " ", o.code[i]);
        }
        }
        s += std::string(VrPad::BtnNames()[b]) + "=" + v + " ";
    }
    char tail[160];
    snprintf(tail, sizeof(tail), "| mouse stick %s %.0f px/s, dead zone %.2f | air mouse %s %.0f px/deg",
             p.mouseStick == 1 ? "left" : p.mouseStick == 2 ? "right" : "off", (double)p.mouseSpeed, (double)p.deadzone,
             p.airBtn >= 0 ? VrPad::BtnNames()[p.airBtn] : "off", (double)p.airGain);
    if (p.airToggle) strncat_s(tail, sizeof(tail), " (toggle)", _TRUNCATE);
    std::string mod;
    const char* const layerName[3] = { "modifier", "modifier2", "both" };
    for (int l = 1; l <= 3; ++l) {
        const int m = l == 1 ? p.modBtn : l == 2 ? p.modBtn2 : (p.modBtn >= 0 && p.modBtn2 >= 0 ? 0 : -1);
        if (m < 0) { if (l < 3) mod += std::string(" | no ") + layerName[l - 1]; continue; }
        mod += std::string(" | ") + layerName[l - 1] + (l < 3 ? std::string(" ") + VrPad::BtnNames()[m] : std::string()) + ":";
        const VrPad::Out* t = VrPad::LayerMap(p, l);
        for (int b = 0; b < VrPad::kBtnCount; ++b) {
            if (t[b].kind == VrPad::kNone) continue;
            char v[48];
            VrPad::OutToString(t[b], v, sizeof(v));
            mod += std::string(" ") + VrPad::BtnNames()[b] + "=" + v;
        }
    }
    char dr[260] = " | no steering";
    if (!p.drive) snprintf(dr, sizeof(dr), " | driving off");
    else if (p.steerStick)
        snprintf(dr, sizeof(dr), " | driving: %s stick steers (dead zone %.2f, fine %.2f up to %.2f of the push, full at %.2f, max %.2f, follow %.2f s, back to the middle %.2f s, at most %.2f turns / s, at 90 km/h %.2f%s)",
                 p.steerStick == 1 ? "left" : "right", (double)p.steerDeadzone, (double)p.steerFine, (double)p.steerFineZone, (double)p.steerFullAt, (double)p.steerMax,
                 (double)p.steerFollowTime, (double)p.steerCenterTime, (double)p.steerRate, (double)p.steerAtSpeed,
                 p.steerSpeedSens ? "" : " off");
    std::string aim;
    if (p.drive && p.steerWith == 1) {
        char a[120];
        static const char* const kGrip[4] = { "both grips", "the left grip", "the right grip", "either grip" };
        snprintf(a, sizeof(a), " | wheel (held with %s): %.0f deg to full lock, dead zone %.0f deg, smooth %.2f s, stick too %s",
                 kGrip[p.steerWheelGrip & 3], (double)p.steerWheelAngle, (double)p.steerAimDeadzone, (double)p.steerAimSmooth,
                 p.steerStickToo ? "on" : "off");
        aim = a;
        // a grip that is the wheel does nothing else: say what that switches off (the log's mapping line)
        const int wg = VrPad::WheelGripMask(p);
        for (int h = 0; h < 2; ++h) {
            if (!(wg & (1 << h))) continue;
            const int g = h ? VrPad::kRGrip : VrPad::kLGrip;
            if (g == p.airBtn) aim += std::string(" -- ") + VrPad::BtnNames()[g] + " is the wheel: no air mouse";
            if (g == p.modBtn) aim += std::string(" -- ") + VrPad::BtnNames()[g] + " is the wheel: no modifier 1 layer";
            if (g == p.modBtn2) aim += std::string(" -- ") + VrPad::BtnNames()[g] + " is the wheel: no modifier 2 layer";
        }
        if (p.steerWheelGrip == 0 && p.modBtn >= 0 && p.modBtn2 >= 0 &&
            ((p.modBtn == VrPad::kLGrip && p.modBtn2 == VrPad::kRGrip) || (p.modBtn == VrPad::kRGrip && p.modBtn2 == VrPad::kLGrip)))
            aim += " -- both grips are the wheel: no Both layer";
    } else if (p.drive && p.steerWith == 2) {
        char a[160];
        snprintf(a, sizeof(a), " | point to steer (%s hand, %s): %.0f deg to full lock, dead zone %.0f deg, smooth %.2f s, stick too %s",
                 p.steerPointHand ? "right" : "left", p.steerPointBtn >= 0 ? VrPad::BtnNames()[p.steerPointBtn] : "always",
                 (double)p.steerPointAngle, (double)p.steerAimDeadzone, (double)p.steerAimSmooth, p.steerStickToo ? "on" : "off");
        aim = a;
    }
    return s + tail + mod + dr + aim;
}

}  // namespace ck
