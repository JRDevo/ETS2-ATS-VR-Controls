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
        if (key == "air_speed")   { if (Num(val, 1, 500, &d)) c.pad.airGain = (float)d; else bad("a number (pixels per degree)"); continue; }
        if (key == "air_mode") {
            if (lv == "hold") c.pad.airToggle = false;
            else if (lv == "toggle") c.pad.airToggle = true;
            else bad("use hold or toggle");
            continue;
        }
        if (key == "vr_menu") { c.menu = lv != "0" && lv != "off" && lv != "no"; continue; }
        if (key == "menu_hold") { if (Num(val, 0.3, 5, &d)) c.menuHold = (float)d; else bad("a number of seconds (0.3 .. 5)"); continue; }
        if (key == "menu_combo") {
            const size_t plus = lv.find('+');
            const int b1 = plus == std::string::npos ? -1 : VrPad::BtnOfName(Trim(lv.substr(0, plus)).c_str());
            const int b2 = plus == std::string::npos ? -1 : VrPad::BtnOfName(Trim(lv.substr(plus + 1)).c_str());
            if (b1 < 0 || b2 < 0 || b1 == b2 || b1 >= VrPad::kLUp || b2 >= VrPad::kLUp) { bad("two different buttons with + (lstick+rstick); no stick directions"); continue; }
            c.menuBtnA = b1;
            c.menuBtnB = b2;
            continue;
        }
        if (key == "menu_place" || key == "keyboard_place") {
            PanelPlace p;
            double v[6] = {};
            int n = 0;
            size_t a = 0;
            bool ok = true;
            while (ok && a <= val.size() && n < 7) {
                size_t comma = val.find(',', a);
                if (comma == std::string::npos) comma = val.size();
                const std::string one = Trim(val.substr(a, comma - a));
                if (n < 6) ok = Num(one, -1000, 1000, &v[n]);
                ++n;
                a = comma + 1;
            }
            if (!ok || n != 6 || v[5] < 0.1 || v[5] > 3.0) { bad("x, y, z, yaw, pitch, width (6 numbers; width 0.1 .. 3 m)"); continue; }
            p.set = true;
            p.x = v[0]; p.y = v[1]; p.z = v[2]; p.yaw = v[3]; p.pitch = v[4]; p.width = v[5];
            (key == "menu_place" ? c.menuPlace : c.kbPlace) = p;
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
            else bad("not a key name / mouse_left / mouse_right / mouse_middle / wheel_up / wheel_down / none");
            continue;
        }
        c.notes.push_back("line " + std::to_string(lineNo) + ": unknown key '" + key + "' -- ignored");
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
    snprintf(b, sizeof(b), "%.3f, %.3f, %.3f, %.1f, %.1f, %.3f", p.x, p.y, p.z, p.yaw, p.pitch, p.width);
    return b;
}

std::vector<std::pair<std::string, std::string>> MenuValues(const VrPad::Config& p, const PanelPlace& menu, const PanelPlace& kb) {
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
    snprintf(b, sizeof(b), "%.0f", (double)p.airGain);
    kv.push_back({ "air_speed", b });
    if (menu.set) kv.push_back({ "menu_place", PlaceString(menu) });
    if (kb.set) kv.push_back({ "keyboard_place", PlaceString(kb) });
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
            lines.push_back("# ---- saved by the VR menu (in the headset: hold both stick clicks 1 s) ------------------------------------------");
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
        char v[48];
        switch (o.kind) {
        case VrPad::kMouseL: snprintf(v, sizeof(v), "mouse_left"); break;
        case VrPad::kMouseR: snprintf(v, sizeof(v), "mouse_right"); break;
        case VrPad::kMouseM: snprintf(v, sizeof(v), "mouse_middle"); break;
        case VrPad::kWheelUp: snprintf(v, sizeof(v), "wheel_up"); break;
        case VrPad::kWheelDown: snprintf(v, sizeof(v), "wheel_down"); break;
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
    return s + tail;
}

}  // namespace ck
