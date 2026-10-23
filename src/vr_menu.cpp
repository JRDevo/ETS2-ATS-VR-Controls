// vr_menu -- see vr_menu.h.
#include "vr_menu.h"
#include <windows.h>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include "log.h"

namespace ck {
namespace {

constexpr double kPi = 3.14159265358979;
constexpr double kDeg = kPi / 180.0;
constexpr int kMenuW = 1000, kMenuH = 800, kKbW = 1600, kKbH = 450;
// keyboard: every gap between key groups is kGap of a key; the full keyboard is 22.3 x 6.1 key units in kKbW pixels; the
// compact one (the main block only, 15 key units) keeps the same key size, so it is narrower in pixels and in metres
constexpr double kGap = 0.15;
constexpr double kKbUnit = (kKbW - 16) / (23.0 - 2 * (0.5 - kGap));
constexpr int kKbCompactW = (int)(16 + kKbUnit * 15.0 + 0.5);

// ---- the keyboard (units of one key; x 0..15 main block, 15.5..18.5 navigation, 19..23 number pad)
struct KeyDef { const char* label; const char* name; float x, y, w, h; };
const KeyDef kKeys[] = {
    { "Esc", "esc", 0, 0, 1, 1 },
    { "F1", "f1", 2, 0, 1, 1 }, { "F2", "f2", 3, 0, 1, 1 }, { "F3", "f3", 4, 0, 1, 1 }, { "F4", "f4", 5, 0, 1, 1 },
    { "F5", "f5", 6.5f, 0, 1, 1 }, { "F6", "f6", 7.5f, 0, 1, 1 }, { "F7", "f7", 8.5f, 0, 1, 1 }, { "F8", "f8", 9.5f, 0, 1, 1 },
    { "F9", "f9", 11, 0, 1, 1 }, { "F10", "f10", 12, 0, 1, 1 }, { "F11", "f11", 13, 0, 1, 1 }, { "F12", "f12", 14, 0, 1, 1 },
    { "ScrLk", "scrolllock", 16.5f, 0, 1, 1 },
    { "`", "`", 0, 1.25f, 1, 1 }, { "1", "1", 1, 1.25f, 1, 1 }, { "2", "2", 2, 1.25f, 1, 1 }, { "3", "3", 3, 1.25f, 1, 1 },
    { "4", "4", 4, 1.25f, 1, 1 }, { "5", "5", 5, 1.25f, 1, 1 }, { "6", "6", 6, 1.25f, 1, 1 }, { "7", "7", 7, 1.25f, 1, 1 },
    { "8", "8", 8, 1.25f, 1, 1 }, { "9", "9", 9, 1.25f, 1, 1 }, { "0", "0", 10, 1.25f, 1, 1 }, { "-", "-", 11, 1.25f, 1, 1 },
    { "=", "=", 12, 1.25f, 1, 1 }, { "Backspace", "backspace", 13, 1.25f, 2, 1 },
    { "Ins", "insert", 15.5f, 1.25f, 1, 1 }, { "Home", "home", 16.5f, 1.25f, 1, 1 }, { "PgUp", "pageup", 17.5f, 1.25f, 1, 1 },
    { "/", "numdiv", 20, 1.25f, 1, 1 }, { "*", "nummul", 21, 1.25f, 1, 1 }, { "-", "numminus", 22, 1.25f, 1, 1 },
    { "Tab", "tab", 0, 2.25f, 1.5f, 1 }, { "Q", "q", 1.5f, 2.25f, 1, 1 }, { "W", "w", 2.5f, 2.25f, 1, 1 },
    { "E", "e", 3.5f, 2.25f, 1, 1 }, { "R", "r", 4.5f, 2.25f, 1, 1 }, { "T", "t", 5.5f, 2.25f, 1, 1 },
    { "Y", "y", 6.5f, 2.25f, 1, 1 }, { "U", "u", 7.5f, 2.25f, 1, 1 }, { "I", "i", 8.5f, 2.25f, 1, 1 },
    { "O", "o", 9.5f, 2.25f, 1, 1 }, { "P", "p", 10.5f, 2.25f, 1, 1 }, { "[", "[", 11.5f, 2.25f, 1, 1 },
    { "]", "]", 12.5f, 2.25f, 1, 1 }, { "\\", "\\", 13.5f, 2.25f, 1.5f, 1 },
    { "Del", "delete", 15.5f, 2.25f, 1, 1 }, { "End", "end", 16.5f, 2.25f, 1, 1 }, { "PgDn", "pagedown", 17.5f, 2.25f, 1, 1 },
    { "7", "num7", 19, 2.25f, 1, 1 }, { "8", "num8", 20, 2.25f, 1, 1 }, { "9", "num9", 21, 2.25f, 1, 1 },
    { "+", "numplus", 22, 2.25f, 1, 2 },
    { "Caps", "capslock", 0, 3.25f, 1.75f, 1 }, { "A", "a", 1.75f, 3.25f, 1, 1 }, { "S", "s", 2.75f, 3.25f, 1, 1 },
    { "D", "d", 3.75f, 3.25f, 1, 1 }, { "F", "f", 4.75f, 3.25f, 1, 1 }, { "G", "g", 5.75f, 3.25f, 1, 1 },
    { "H", "h", 6.75f, 3.25f, 1, 1 }, { "J", "j", 7.75f, 3.25f, 1, 1 }, { "K", "k", 8.75f, 3.25f, 1, 1 },
    { "L", "l", 9.75f, 3.25f, 1, 1 }, { ";", ";", 10.75f, 3.25f, 1, 1 }, { "'", "'", 11.75f, 3.25f, 1, 1 },
    { "Enter", "enter", 12.75f, 3.25f, 2.25f, 1 },
    { "4", "num4", 19, 3.25f, 1, 1 }, { "5", "num5", 20, 3.25f, 1, 1 }, { "6", "num6", 21, 3.25f, 1, 1 },
    { "Shift", "shift", 0, 4.25f, 2.25f, 1 }, { "Z", "z", 2.25f, 4.25f, 1, 1 }, { "X", "x", 3.25f, 4.25f, 1, 1 },
    { "C", "c", 4.25f, 4.25f, 1, 1 }, { "V", "v", 5.25f, 4.25f, 1, 1 }, { "B", "b", 6.25f, 4.25f, 1, 1 },
    { "N", "n", 7.25f, 4.25f, 1, 1 }, { "M", "m", 8.25f, 4.25f, 1, 1 }, { ",", ",", 9.25f, 4.25f, 1, 1 },
    { ".", ".", 10.25f, 4.25f, 1, 1 }, { "/", "/", 11.25f, 4.25f, 1, 1 }, { "Shift", "rshift", 12.25f, 4.25f, 2.75f, 1 },
    { "\xE2\x86\x91", "up", 16.5f, 4.25f, 1, 1 },
    { "1", "num1", 19, 4.25f, 1, 1 }, { "2", "num2", 20, 4.25f, 1, 1 }, { "3", "num3", 21, 4.25f, 1, 1 },
    { "Enter", "numenter", 22, 4.25f, 1, 2 },
    { "Ctrl", "ctrl", 0, 5.25f, 1.5f, 1 }, { "Alt", "alt", 1.5f, 5.25f, 1.5f, 1 }, { "Space", "space", 3, 5.25f, 9, 1 },
    { "Alt", "ralt", 12, 5.25f, 1.5f, 1 }, { "Ctrl", "rctrl", 13.5f, 5.25f, 1.5f, 1 },
    { "\xE2\x86\x90", "left", 15.5f, 5.25f, 1, 1 }, { "\xE2\x86\x93", "down", 16.5f, 5.25f, 1, 1 },
    { "\xE2\x86\x92", "right", 17.5f, 5.25f, 1, 1 },
    { "0", "num0", 19, 5.25f, 2, 1 }, { ".", "numdot", 21, 5.25f, 1, 1 },
};
constexpr int kKeyCount = (int)(sizeof(kKeys) / sizeof(kKeys[0]));
// Shift / Ctrl / Alt (either side) = latch 0 / 1 / 2; -1 = not a modifier
int ModOf(const char* name) {
    if (!strcmp(name, "shift") || !strcmp(name, "rshift")) return 0;
    if (!strcmp(name, "ctrl") || !strcmp(name, "rctrl")) return 1;
    if (!strcmp(name, "alt") || !strcmp(name, "ralt")) return 2;
    return -1;
}
const char* const kModName[3] = { "shift", "ctrl", "alt" };

const char* BtnLabel(int b) {
    static const char* const k[VrPad::kBtnCount] = { "Left trigger", "Right trigger", "Left grip", "Right grip", "X", "Y", "A", "B",
        "Left stick click", "Right stick click", "Left menu", "Left stick up", "Left stick down", "Left stick left",
        "Left stick right", "Right stick up", "Right stick down", "Right stick left", "Right stick right" };
    return b >= 0 && b < VrPad::kBtnCount ? k[b] : "?";
}
// "shift+f" -> "Shift + F", "mouse_left" -> "Mouse left", "none" -> "-"
std::string Pretty(const VrPad::Out& o) {
    char raw[64];
    VrPad::OutToString(o, raw, sizeof(raw));
    if (o.kind == VrPad::kNone) return "-";
    if (o.kind != VrPad::kKeys) {
        std::string s = raw;
        for (char& c : s) if (c == '_') c = ' ';
        s[0] = (char)toupper((unsigned char)s[0]);
        return s;
    }
    std::string out, part;
    for (const char* p = raw;; ++p) {
        if ((*p == '+' && !part.empty()) || !*p) {
            for (const KeyDef& k : kKeys)
                if (part == k.name) { part = k.name[0] == 'n' && k.name[1] == 'u' ? std::string("Num ") + k.label : k.label; break; }
            if (part.size() == 1) part[0] = (char)toupper((unsigned char)part[0]);
            out += (out.empty() ? "" : " + ") + part;
            part.clear();
            if (!*p) break;
            continue;
        }
        part += *p;
    }
    return out;
}

// ---- GDI drawing into a top-down 32-bit DIB
std::wstring Wide(const std::string& s) {
    if (s.empty()) return std::wstring();
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    std::wstring w((size_t)n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], n);
    return w;
}
}  // namespace

class MenuCanvas {
public:
    MenuCanvas(int w, int h) : w_(w), h_(h) {
        BITMAPINFO bi{};
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        bi.bmiHeader.biWidth = w;
        bi.bmiHeader.biHeight = -h;
        bi.bmiHeader.biPlanes = 1;
        bi.bmiHeader.biBitCount = 32;
        bi.bmiHeader.biCompression = BI_RGB;
        dc_ = CreateCompatibleDC(nullptr);
        bmp_ = CreateDIBSection(dc_, &bi, DIB_RGB_COLORS, &bits_, nullptr, 0);
        if (dc_ && bmp_) old_ = SelectObject(dc_, bmp_);
        if (dc_) SetBkMode(dc_, TRANSPARENT);
    }
    ~MenuCanvas() {
        if (dc_ && old_) SelectObject(dc_, old_);
        for (int i = 0; i < 2; ++i) for (auto& f : fonts_[i]) if (f.second) DeleteObject(f.second);
        if (bmp_) DeleteObject(bmp_);
        if (dc_) DeleteDC(dc_);
    }
    bool Ok() const { return dc_ && bmp_ && bits_; }
    int W() const { return w_; }
    int H() const { return h_; }
    void Round(int x, int y, int w, int h, int r, COLORREF c) {
        HBRUSH b = CreateSolidBrush(c);
        HGDIOBJ ob = SelectObject(dc_, b), op = SelectObject(dc_, GetStockObject(NULL_PEN));
        if (r > 0) RoundRect(dc_, x, y, x + w + 1, y + h + 1, r, r);
        else Rectangle(dc_, x, y, x + w + 1, y + h + 1);
        SelectObject(dc_, op);
        SelectObject(dc_, ob);
        DeleteObject(b);
    }
    void Text(int x, int y, int w, int h, const std::string& s, int px, bool bold, COLORREF c, UINT align) {
        if (s.empty()) return;
        SelectObject(dc_, Font(px, bold));
        SetTextColor(dc_, c);
        RECT r{ x, y, x + w, y + h };
        const std::wstring ws = Wide(s);
        DrawTextW(dc_, ws.c_str(), (int)ws.size(), &r, align | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    // the picture with `alpha` (0..255) everywhere, premultiplied (GDI leaves alpha 0)
    void CopyTo(std::vector<uint32_t>& out, int alpha) {
        GdiFlush();
        out.assign(static_cast<const uint32_t*>(bits_), static_cast<const uint32_t*>(bits_) + (size_t)w_ * h_);
        if (alpha >= 255) { for (uint32_t& p : out) p |= 0xFF000000u; return; }
        const uint32_t a = (uint32_t)(alpha < 0 ? 0 : alpha);
        for (uint32_t& p : out)
            p = (a << 24) | ((((p >> 16) & 0xFFu) * a / 255u) << 16) | ((((p >> 8) & 0xFFu) * a / 255u) << 8) | ((p & 0xFFu) * a / 255u);
    }
private:
    HFONT Font(int px, bool bold) {
        for (auto& f : fonts_[bold ? 1 : 0]) if (f.first == px) return f.second;
        HFONT f = CreateFontW(-px, 0, 0, 0, bold ? FW_SEMIBOLD : FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, L"Segoe UI");
        fonts_[bold ? 1 : 0].push_back({ px, f });
        return f;
    }
    int w_, h_;
    HDC dc_ = nullptr;
    HBITMAP bmp_ = nullptr;
    HGDIOBJ old_ = nullptr;
    void* bits_ = nullptr;
    std::vector<std::pair<int, HFONT>> fonts_[2];
};

namespace {

// the colour themes (config.ini theme = ThemeNames()): background, bar, element, hover, selected, pressed, text, dim text, accent
struct Theme { COLORREF bg, bar, elem, hover, on, press, text, dim, accent; };
const Theme kThemes[kThemeCount] = {
    { RGB(24, 27, 33), RGB(38, 43, 52), RGB(50, 56, 66), RGB(66, 88, 120), RGB(46, 108, 186), RGB(96, 156, 236), RGB(232, 234, 237), RGB(128, 136, 148), RGB(242, 169, 59) },   // dark
    { RGB(14, 28, 52), RGB(22, 44, 80), RGB(30, 58, 102), RGB(46, 86, 146), RGB(0, 120, 212), RGB(80, 170, 255), RGB(230, 240, 255), RGB(130, 150, 180), RGB(255, 196, 64) },  // blue
    { RGB(58, 60, 64), RGB(76, 79, 84), RGB(92, 96, 102), RGB(116, 122, 132), RGB(70, 130, 200), RGB(120, 170, 230), RGB(245, 245, 245), RGB(170, 174, 180), RGB(255, 190, 80) },   // grey
    { RGB(232, 234, 238), RGB(206, 210, 218), RGB(250, 250, 252), RGB(200, 220, 245), RGB(60, 130, 220), RGB(30, 100, 200), RGB(28, 30, 36), RGB(110, 116, 126), RGB(200, 110, 0) },   // light
    { RGB(0, 0, 0), RGB(20, 20, 20), RGB(34, 34, 34), RGB(70, 70, 70), RGB(200, 140, 0), RGB(255, 190, 40), RGB(255, 255, 255), RGB(150, 150, 150), RGB(255, 190, 40) },          // black
    { RGB(18, 34, 26), RGB(28, 52, 40), RGB(38, 70, 54), RGB(56, 100, 78), RGB(40, 150, 90), RGB(90, 200, 130), RGB(232, 245, 236), RGB(130, 160, 140), RGB(250, 200, 70) },      // green
};
const char* const kThemeLabel[kThemeCount] = { "Dark", "Blue", "Grey", "Light", "Black", "Green" };

}  // namespace

// ------------------------------------------------------------------------------------------------ the combo gate

bool BtnDown(const VrPad::Frame& f, int b) {
    using namespace VrPad;
    switch (b) {
    case kLTrig: return f.trig[0] > kPressAt;
    case kRTrig: return f.trig[1] > kPressAt;
    case kLGrip: return f.grip[0] > kPressAt;
    case kRGrip: return f.grip[1] > kPressAt;
    case kX: return f.prim[0];
    case kY: return f.sec[0];
    case kA: return f.prim[1];
    case kB: return f.sec[1];
    case kLStick: return f.click[0];
    case kRStick: return f.click[1];
    case kMenu: return f.menu[0];
    default: return false;
    }
}
void BtnSet(VrPad::Frame& f, int b, bool on) {
    using namespace VrPad;
    const float v = on ? 1.0f : 0.0f;
    switch (b) {
    case kLTrig: f.trig[0] = v; break;
    case kRTrig: f.trig[1] = v; break;
    case kLGrip: f.grip[0] = v; break;
    case kRGrip: f.grip[1] = v; break;
    case kX: f.prim[0] = on; break;
    case kY: f.sec[0] = on; break;
    case kA: f.prim[1] = on; break;
    case kB: f.sec[1] = on; break;
    case kLStick: f.click[0] = on; break;
    case kRStick: f.click[1] = on; break;
    case kMenu: f.menu[0] = on; break;
    default: break;
    }
}

bool ComboGate::Step(VrPad::Frame& f, double now) {
    if (btns.empty()) return false;
    bool all = true, any = false;
    for (int b : btns) { const bool d = BtnDown(f, b); all = all && d; any = any || d; }
    if (!on_ && all) { on_ = true; t0_ = now; fired_ = false; }
    if (on_ && !any) on_ = false;
    if (!on_) return false;
    for (int b : btns) BtnSet(f, b, false);
    if (!fired_ && all && now - t0_ >= hold) { fired_ = true; return true; }
    return false;
}

// ------------------------------------------------------------------------------------------------ the menu

VrMenu::~VrMenu() = default;

VrMenu::VrMenu(const Config& cfg, const std::wstring& path) : pad_(cfg.pad), path_(path) {
    gate_.btns = cfg.menuCombo;
    gate_.hold = cfg.menuHold;
    place_[kPanMenu] = cfg.menuPlace;
    place_[kPanKb] = cfg.kbPlace;
    pan_[kPanMenu].pw = kMenuW; pan_[kPanMenu].ph = kMenuH; pan_[kPanMenu].width = 0.6;
    pan_[kPanKb].pw = kKbW; pan_[kPanKb].ph = kKbH; pan_[kPanKb].width = 0.8;
    look_ = cfg.look;
    if (look_.theme < 0 || look_.theme >= kThemeCount) look_.theme = 0;
    if (look_.kbCompact) { pan_[kPanKb].pw = kKbCompactW; pan_[kPanKb].width *= (double)kKbCompactW / kKbW; }   // (same key size)
    for (int p = 0; p < kPanCount; ++p)
        if (place_[p].set) { pan_[p].width = place_[p].width; Place(p, false, Pose(), false); }
    if (look_.kbShow) {                                 // the keyboard stays from the last game (at its place; else in front of
        if (place_[kPanKb].set) pan_[kPanKb].show = true;   //   the head at the first frame that has the head)
        else kbPending_ = true;
    }
    // the pointer dot: white with a dark ring, soft edge, premultiplied alpha
    cursorPx_.resize((size_t)kCursorPx * kCursorPx);
    for (int y = 0; y < kCursorPx; ++y)
        for (int x = 0; x < kCursorPx; ++x) {
            const double dx = x + 0.5 - kCursorPx / 2.0, dy = y + 0.5 - kCursorPx / 2.0, r = std::sqrt(dx * dx + dy * dy);
            auto cover = [](double edge, double rr) { const double k = edge - rr + 0.5; return k < 0 ? 0.0 : k > 1 ? 1.0 : k; };
            const double outer = cover(26.0, r), inner = cover(18.0, r);
            const double a = outer * 0.9 + inner * 0.1;              // ring 0.9 opaque, the inside fully
            const double white = inner;                              // the inside white, the ring dark
            const int c = (int)(255.0 * white + 0.5), al = (int)(255.0 * a + 0.5);
            const int pc = c * al / 255;
            cursorPx_[(size_t)y * kCursorPx + x] = ((uint32_t)al << 24) | ((uint32_t)pc << 16) | ((uint32_t)pc << 8) | (uint32_t)pc;
        }
}

int VrMenu::KeyIndex(const char* name) {
    for (int k = 0; k < kKeyCount; ++k) if (!strcmp(kKeys[k].name, name)) return k;
    return -1;
}

const VrMenu::Elem* VrMenu::Find(int p, int kind, int arg) const {
    for (const Elem& e : pan_[p].el) if (e.kind == kind && e.arg == arg) return &e;
    return nullptr;
}

void VrMenu::Place(int p, bool headOk, const Pose& head, bool force) {
    Panel& pn = pan_[p];
    PanelPlace& pl = place_[p];
    if (!pl.set || force) {
        double yaw = 0;
        V3 hp{ 0, 0, 0 };
        if (headOk) {
            V3 fw = QRot(head.q, V3{ 0, 0, -1 });
            double pitch;
            YawPitchOf(fw, &yaw, &pitch);
            hp = head.p;
        }
        const double pitch = p == kPanMenu ? -8.0 * kDeg : -38.0 * kDeg, dist = p == kPanMenu ? 0.75 : 0.55;
        const V3 dir = QRot(QYawPitch(yaw, pitch), V3{ 0, 0, -1 });
        const V3 at = hp + dir * dist;
        pl.set = true;
        pl.x = at.x; pl.y = at.y; pl.z = at.z;
        pl.yaw = yaw / kDeg; pl.pitch = pitch / kDeg;
        pl.width = pn.width;
    }
    pn.width = pl.width;
    pn.pose.p = V3{ pl.x, pl.y, pl.z };
    pn.pose.q = QYawPitch(pl.yaw * kDeg, pl.pitch * kDeg);
}

void VrMenu::Open(bool headOk, const Pose& head) {
    Place(kPanMenu, headOk, head, false);
    pan_[kPanMenu].show = true;
    status_.clear();
    RedrawAll();
    LOGI("VR menu open (%s)", Describe().c_str());
}

void VrMenu::ShowKb(bool on) {
    if (on && !pan_[kPanKb].show) Place(kPanKb, lastHeadOk_, lastHead_, false);
    if (!on) pickBtn_ = -1;
    pan_[kPanKb].show = on;
    kbPending_ = false;
    if (look_.kbShow != on) { look_.kbShow = on; Save(); }
}

void VrMenu::SetKbCompact(bool on) {
    if (look_.kbCompact == on) return;
    Panel& pn = pan_[kPanKb];
    const int npw = on ? kKbCompactW : kKbW;
    pn.width *= (double)npw / pn.pw;                    // the same key size: the panel narrows / widens with its pixels
    pn.pw = npw;
    look_.kbCompact = on;
    for (int h = 0; h < 2; ++h) if (grab_[h].pan == kPanKb) grab_[h] = Grab();
    if (place_[kPanKb].set) SavePlaces(); else Save();
    needRedraw_ = true;
    LOGI("VR menu: keyboard %s (%d px, %.2f m wide)", on ? "compact" : "full", npw, pn.width);
}

void VrMenu::Push(int p, double metres) {
    Panel& pn = pan_[p];
    const V3 from = lastHeadOk_ ? lastHead_.p : V3{ 0, 0, 0 };
    const V3 v = pn.pose.p - from;
    const double len = Len(v);
    if (len < 1e-6) return;
    double nl = len + metres;
    nl = nl < 0.2 ? 0.2 : nl > 3.0 ? 3.0 : nl;
    pn.pose.p = from + v * (nl / len);
    SavePlaces();
}

void VrMenu::Close() {
    pan_[kPanMenu].show = false;
    pickBtn_ = -1;
    for (bool& l : latch_) l = false;
    LOGI("VR menu closed");
}

void VrMenu::Layout(int p) { if (p == kPanMenu) LayoutMenu(); else LayoutKb(); }

void VrMenu::LayoutMenu() {
    std::vector<Elem>& el = pan_[kPanMenu].el;
    el.clear();
    auto add = [&](int kind, int arg, int x, int y, int w, int h, const std::string& t, bool on = false, bool dim = false,
                   const std::string& sub = std::string()) {
        Elem e;
        e.kind = kind; e.arg = arg; e.x = x; e.y = y; e.w = w; e.h = h; e.text = t; e.on = on; e.dim = dim; e.sub = sub;
        el.push_back(e);
    };
    const bool picking = pickBtn_ >= 0;
    add(eBar, 0, 0, 0, kMenuW, 72, picking ? std::string("Set ") + BtnLabel(pickBtn_) + ": pick a key" : "ETS2 ATS VR Controls", picking);
    add(eSmaller, 0, 600, 11, 64, 50, "\xE2\x88\x92");
    add(eBigger, 0, 672, 11, 64, 50, "+");
    add(eKbBtn, 0, 744, 11, 170, 50, "Keyboard", pan_[kPanKb].show);
    add(eClose, 0, 924, 11, 64, 50, "\xC3\x97");
    static const char* const kTabs[4] = { "Buttons", "Mouse", "Panels", "Look" };
    for (int t = 0; t < 4; ++t) add(eTab, t, 24 + t * 241, 84, 229, 56, kTabs[t], tab_ == t);
    char b[96];
    if (tab_ == 0) {
        static const int kLeft[10] = { VrPad::kLTrig, VrPad::kLGrip, VrPad::kX, VrPad::kY, VrPad::kLStick, VrPad::kMenu,
                                       VrPad::kLUp, VrPad::kLDown, VrPad::kLLeft, VrPad::kLRight };
        static const int kRight[9] = { VrPad::kRTrig, VrPad::kRGrip, VrPad::kA, VrPad::kB, VrPad::kRStick, VrPad::kRUp,
                                       VrPad::kRDown, VrPad::kRLeft, VrPad::kRRight };
        for (int col = 0; col < 2; ++col)
            for (int i = 0; i < (col ? 9 : 10); ++i) {
                const int btn = col ? kRight[i] : kLeft[i];
                std::string val = Pretty(pad_.map[btn]);
                bool dim = false;
                if (btn == pad_.airBtn) { val = "air mouse"; dim = true; }
                else if (btn >= VrPad::kLUp && pad_.mouseStick == (btn >= VrPad::kRUp ? 2 : 1)) { val = "moves the mouse"; dim = true; }
                bool combo = false;
                for (int cb : gate_.btns) combo = combo || cb == btn;
                add(eRow, btn, col ? 508 : 24, 156 + i * 54, 468, 50, BtnLabel(btn), pickBtn_ == btn, dim,
                    combo ? val + "  (+menu)" : val);
            }
        if (picking) {
            static const int kOuts[6] = { VrPad::kMouseL, VrPad::kMouseR, VrPad::kMouseM, VrPad::kWheelUp, VrPad::kWheelDown, VrPad::kNone };
            static const char* const kOutNames[6] = { "Mouse L", "Mouse R", "Mouse M", "Wheel up", "Wheel dn", "None" };
            for (int i = 0; i < 6; ++i) add(ePickOut, kOuts[i], 24 + i * 138, 716, 130, 64, kOutNames[i]);
            add(ePickCancel, 0, 852, 716, 124, 64, "Cancel");
        } else {
            add(eInfo, 0, 24, 716, 952, 64, status_.empty() ? "Point at a button, pull the trigger, then pick its key." : status_);
        }
    } else if (tab_ == 1) {
        auto row = [&](int r, const char* label) { add(eInfo, 100 + r, 24, 162 + r * 88, 440, 70, label); };
        auto opts = [&](int r, int base, std::initializer_list<const char*> names, int cur) {
            const int n = (int)names.size(), w = (506 - (n - 1) * 10) / n;
            int i = 0;
            for (const char* nm : names) { add(eSet, base + i, 470 + i * (w + 10), 162 + r * 88, w, 70, nm, i == cur); ++i; }
        };
        auto plusMinus = [&](int r, int base, const std::string& value) {
            add(eSet, base, 470, 162 + r * 88, 90, 70, "\xE2\x88\x92");
            add(eInfo, 200 + r, 570, 162 + r * 88, 306, 70, value);
            add(eSet, base + 1, 886, 162 + r * 88, 90, 70, "+");
        };
        row(0, "Stick that moves the mouse");
        opts(0, 0, { "Left", "Right", "Off" }, pad_.mouseStick == 1 ? 0 : pad_.mouseStick == 2 ? 1 : 2);
        row(1, "Stick mouse speed");
        snprintf(b, sizeof(b), "%.0f px/s", (double)pad_.mouseSpeed);
        plusMinus(1, 10, b);
        row(2, "Stick dead zone");
        snprintf(b, sizeof(b), "%.2f", (double)pad_.deadzone);
        plusMinus(2, 20, b);
        row(3, "Air mouse button");
        opts(3, 30, { "Right grip", "Left grip", "Off" }, pad_.airBtn == VrPad::kRGrip ? 0 : pad_.airBtn == VrPad::kLGrip ? 1 : pad_.airBtn < 0 ? 2 : -1);
        row(4, "Air mouse works while");
        opts(4, 40, { "Button held", "Toggled on" }, pad_.airToggle ? 1 : 0);
        row(5, "Air mouse speed");
        snprintf(b, sizeof(b), "%.0f px/deg", (double)pad_.airGain);
        plusMinus(5, 50, b);
        add(eInfo, 0, 24, 716, 952, 64, status_.empty() ? "Changes work at once and are saved." : status_);
    } else if (tab_ == 2) {
        auto row = [&](int r, const char* label) { add(eInfo, 100 + r, 24, 156 + r * 76, 440, 64, label); };
        auto plusMinus = [&](int r, int base, const std::string& value, const char* minus, const char* plus) {
            const bool glyph = !strcmp(minus, "\xE2\x88\x92");
            add(eSet, base, 470, 156 + r * 76, glyph ? 90 : 130, 64, minus);
            add(eInfo, 200 + r, glyph ? 570 : 610, 156 + r * 76, glyph ? 306 : 226, 64, value);
            add(eSet, base + 1, glyph ? 886 : 846, 156 + r * 76, glyph ? 90 : 130, 64, plus);
        };
        const V3 from = lastHeadOk_ ? lastHead_.p : V3{ 0, 0, 0 };
        row(0, "Menu size");
        snprintf(b, sizeof(b), "%.2f m wide", pan_[kPanMenu].width);
        plusMinus(0, 60, b, "\xE2\x88\x92", "+");
        row(1, "Menu distance");
        snprintf(b, sizeof(b), "%.2f m", Len(pan_[kPanMenu].pose.p - from));
        plusMinus(1, 62, b, "Closer", "Farther");
        row(2, "Keyboard size");
        snprintf(b, sizeof(b), "%.2f m wide", pan_[kPanKb].width);
        plusMinus(2, 70, b, "\xE2\x88\x92", "+");
        row(3, "Keyboard distance");
        if (pan_[kPanKb].show) snprintf(b, sizeof(b), "%.2f m", Len(pan_[kPanKb].pose.p - from));
        else snprintf(b, sizeof(b), "(hidden)");
        plusMinus(3, 72, b, "Closer", "Farther");
        row(4, "Keyboard");
        add(eSet, 80, 470, 460, 119, 64, "Show", pan_[kPanKb].show);
        add(eSet, 81, 599, 460, 119, 64, "Hide", !pan_[kPanKb].show);
        add(eSet, 82, 738, 460, 114, 64, "Full", !look_.kbCompact);
        add(eSet, 83, 862, 460, 114, 64, "Compact", look_.kbCompact);
        add(eHere, 0, 24, 540, 952, 64, "Bring the menu and the keyboard in front of me");
        std::string combo;
        for (int cb : gate_.btns) combo += (combo.empty() ? "" : " + ") + std::string(BtnLabel(cb));
        snprintf(b, sizeof(b), " for %.1f s", gate_.hold);
        add(eInfo, 300, 24, 612, 952, 46, "Menu open / close: hold " + combo + b);
        add(eInfo, 301, 24, 656, 952, 46, "Move: point at a panel, hold the grip, move. Its stick: farther / closer.");
        add(eInfo, 0, 24, 716, 952, 64, status_.empty() ? "The keyboard stays when the menu closes (its x hides it)." : status_);
    } else {
        auto row = [&](int r, const char* label) { add(eInfo, 100 + r, 24, 162 + r * 88, 440, 70, label); };
        row(0, "Colours");
        for (int i = 0; i < kThemeCount; ++i)
            add(eSet, 100 + i, 470 + (i % 3) * 172, 162 + (i / 3) * 80, 162, 70, kThemeLabel[i], look_.theme == i);
        row(2, "Menu see-through");
        add(eSet, 90, 470, 338, 90, 70, "\xE2\x88\x92");
        snprintf(b, sizeof(b), "%d %% solid", (int)(look_.menuOpacity * 100.0f + 0.5f));
        add(eInfo, 202, 570, 338, 306, 70, b);
        add(eSet, 91, 886, 338, 90, 70, "+");
        row(3, "Keyboard see-through");
        add(eSet, 92, 470, 426, 90, 70, "\xE2\x88\x92");
        snprintf(b, sizeof(b), "%d %% solid", (int)(look_.kbOpacity * 100.0f + 0.5f));
        add(eInfo, 203, 570, 426, 306, 70, b);
        add(eSet, 93, 886, 426, 90, 70, "+");
        row(4, "Pointer smoothing");
        static const char* const kSm[4] = { "Off", "Low", "Medium", "High" };
        for (int i = 0; i < 4; ++i) add(eSet, 110 + i, 470 + i * 129, 514, 119, 70, kSm[i], look_.smoothing == i);
        add(eInfo, 302, 24, 596, 952, 56, "Smoothing calms a shaky hand (menu pointer and air mouse).");
        add(eInfo, 0, 24, 716, 952, 64, status_.empty() ? "- = more see-through, + = more solid." : status_);
    }
}

void VrMenu::LayoutKb() {
    std::vector<Elem>& el = pan_[kPanKb].el;
    el.clear();
    auto add = [&](int kind, int arg, int x, int y, int w, int h, const std::string& t, bool on = false) {
        Elem e;
        e.kind = kind; e.arg = arg; e.x = x; e.y = y; e.w = w; e.h = h; e.text = t; e.on = on;
        el.push_back(e);
    };
    // one flat block, no title bar: - / + / x sit in the empty space above the number pad (compact: only x, above Backspace;
    // its size is set in the Panels tab); move it with the grip anywhere. Picking a key for a button: the whole keyboard gets an accent
    // frame (the menu's bar says which button). kKeys is a real keyboard's layout (gaps of 1 / 0.5 key between the groups);
    // here every gap between groups is only kGap of a key: still visible, less wasted room. 2 px of gap around each key.
    const bool compact = look_.kbCompact;
    const double u = kKbUnit, x0 = 8, y0 = 8;
    auto mapX = [&](double kx, double ky) {
        if (kx >= 19) return kx - 1.0 + 2 * kGap;                       // number pad
        if (kx >= 15.5) return kx - 0.5 + kGap;                         // navigation / arrows
        if (ky != 0 || kx >= 15) return kx;
        if (kx >= 11) return kx - 2.0 + 3 * kGap;                       // F9 .. F12
        if (kx >= 6.5) return kx - 1.5 + 2 * kGap;                      // F5 .. F8
        if (kx >= 2) return kx - 1.0 + kGap;                            // F1 .. F4
        return kx;
    };
    auto at = [&](int kind, int arg, double kx, double ky, double kw, double kh, const std::string& t, bool on = false) {
        const double ux = mapX(kx, ky), uy = ky >= 1 ? ky - (0.25 - kGap * 2.0 / 3.0) : ky;   // F-row gap 0.1 of a key
        add(kind, arg, (int)(x0 + ux * u + 2), (int)(y0 + uy * u + 2), (int)(kw * u - 4), (int)(kh * u - 4), t, on);
    };
    if (compact) {                                       // only x, in the free top-row corner above Backspace (no - / +:
        add(eClose, 0, (int)(x0 + 14 * u + 2), (int)(y0 + 2), (int)(u - 4), (int)(u - 4), "\xC3\x97");   //   Panels tab)
    } else {
        at(eSmaller, 0, 19, 0, 1, 1, "\xE2\x88\x92");
        at(eBigger, 0, 20, 0, 1, 1, "+");
        at(eClose, 0, 22, 0, 1, 1, "\xC3\x97");
    }
    for (int k = 0; k < kKeyCount; ++k) {
        const KeyDef& d = kKeys[k];
        if (compact && d.x >= 15) continue;              // compact: no navigation / arrows / number pad
        const int m = ModOf(d.name);
        at(eKey, k, d.x, d.y, d.w, d.h, d.label, m >= 0 && latch_[m]);
    }
}

void VrMenu::Draw(int p) {
    const Theme& T = kThemes[look_.theme];
    Panel& pn = pan_[p];
    if (!cv_[p] || cv_[p]->W() != pn.pw || cv_[p]->H() != pn.ph) cv_[p] = std::make_unique<MenuCanvas>(pn.pw, pn.ph);
    MenuCanvas& c = *cv_[p];
    if (!c.Ok()) return;
    c.Round(0, 0, pn.pw, pn.ph, 0, p == kPanKb && pickBtn_ >= 0 ? T.accent : T.bg);   // (picking: an accent frame)
    if (p == kPanKb && pickBtn_ >= 0) c.Round(5, 5, pn.pw - 11, pn.ph - 11, 0, T.bg);
    elState_[p].assign(pn.el.size(), 0);
    for (size_t i = 0; i < pn.el.size(); ++i) {
        elState_[p][i] = (uint8_t)ElemState(p, pn.el[i]);
        DrawElem(c, pn.el[i], elState_[p][i]);
    }
    Publish(p);
}

int VrMenu::ElemState(int p, const Elem& e) const {
    int st = 0;
    if (e.kind == eBar) return 0;                       // the bar never changes its look (a shaky pointer on it made it blink)
    for (int h = 0; h < 2; ++h) {
        if (hov_[h].pan == p && hov_[h].kind == e.kind && hov_[h].arg == e.arg) st |= 1;
        if (press_[h].pan == p && press_[h].kind == e.kind && press_[h].arg == e.arg) st |= 2;
    }
    return st;
}

// A pointer moving over the keys changes two elements: only those are drawn again (a full panel is ~2 ms of GDI).
bool VrMenu::DrawChanged(int p) {
    const Theme& T = kThemes[look_.theme];
    Panel& pn = pan_[p];
    if (!cv_[p] || elState_[p].size() != pn.el.size()) return false;
    MenuCanvas& c = *cv_[p];
    bool any = false;
    for (size_t i = 0; i < pn.el.size(); ++i) {
        const Elem& e = pn.el[i];
        const int st = ElemState(p, e);
        if (st == elState_[p][i]) continue;
        if (e.kind == eBar) return false;            // the bar lies under its buttons: all again
        elState_[p][i] = (uint8_t)st;
        c.Round(e.x, e.y, e.w, e.h, 0, p == kPanMenu && e.y < 72 ? T.bar : T.bg);
        DrawElem(c, e, st);
        any = true;
    }
    if (any) Publish(p);
    return true;
}

void VrMenu::Publish(int p) {
    const float op = p == kPanMenu ? look_.menuOpacity : look_.kbOpacity;
    cv_[p]->CopyTo(pan_[p].px, (int)(op * 255.0f + 0.5f));
    ++pan_[p].version;
}

void VrMenu::Redraw(int p) {
    Layout(p);
    Draw(p);
    content_[p] = ContentKey();
}

void VrMenu::DrawElem(MenuCanvas& c, const Elem& e, int st) {
    const Theme& T = kThemes[look_.theme];
    const bool hov = (st & 1) != 0, prs = (st & 2) != 0;
    switch (e.kind) {
    case eBar:
        c.Round(e.x, e.y, e.w, e.h, 0, T.bar);
        c.Text(e.x + 24, e.y, e.w - 24, e.h, e.text, 34, true, e.on ? T.accent : T.text, DT_LEFT);
        break;
    case eInfo: {
        const bool label = e.arg >= 100 && e.arg < 200, value = e.arg >= 200 && e.arg < 300;
        c.Text(e.x + (value ? 0 : 4), e.y, e.w, e.h, e.text, label ? 28 : value ? 30 : 26, value, label || value ? T.text : T.dim,
               value ? DT_CENTER : DT_LEFT);
        break;
    }
    case eRow: {
        c.Round(e.x, e.y, e.w, e.h, 12, prs ? T.press : e.on ? T.on : hov ? T.hover : T.elem);
        c.Text(e.x + 16, e.y, 200, e.h, e.text, 24, false, e.dim ? T.dim : T.text, DT_LEFT);
        c.Text(e.x + 210, e.y, e.w - 226, e.h, e.sub, 26, true, e.dim ? T.dim : T.text, DT_RIGHT);
        break;
    }
    case eKey: {
        c.Round(e.x, e.y, e.w, e.h, 8, prs ? T.press : e.on ? T.on : hov ? T.hover : T.elem);
        const bool one = e.text.size() == 1 || (unsigned char)e.text[0] >= 0x80;   // a letter / digit / arrow: bigger
        c.Text(e.x, e.y, e.w, e.h, e.text, one ? 30 : 21, true, T.text, DT_CENTER);
        break;
    }
    default: {
        c.Round(e.x, e.y, e.w, e.h, 12, prs ? T.press : e.on ? T.on : hov ? T.hover : T.elem);
        const bool glyph = e.text == "+" || e.text == "\xE2\x88\x92" || e.text == "\xC3\x97";   // + - x: bigger
        c.Text(e.x, e.y, e.w, e.h, e.text, glyph ? 40 : e.kind == ePickOut ? 24 : 28,
               true, T.text, DT_CENTER);
        break;
    }
    }
}

bool VrMenu::Hit(const VrPad::Frame&, int h, Ref* r, double* uo, double* vo) {
    if (!aimOk_[h]) return false;
    const V3 o = aim_[h].p, d = QRot(aim_[h].q, V3{ 0, 0, -1 });
    double best = 1e9;
    bool any = false;
    for (int p = 0; p < kPanCount; ++p) {
        const Panel& pn = pan_[p];
        if (!pn.show || pn.el.empty()) continue;
        double t, u, v;
        if (!RayQuad(o, d, pn.pose, pn.width, pn.Height(), 0.0, &t, &u, &v) || t >= best || t > 5.0) continue;
        best = t;
        any = true;
        r->pan = p; r->kind = eNone; r->arg = 0;
        *uo = u; *vo = v;
        const int x = (int)(u * pn.pw), y = (int)(v * pn.ph);
        // the hovered element keeps the pointer up to 8 px outside it -- not the menu's top bar: it lies under its own
        // buttons, and keeping it made - / + / Keyboard / x on it unreachable
        if (hov_[h].pan == p && hov_[h].kind != eNone && hov_[h].kind != eBar) {
            const Elem* e = Find(p, hov_[h].kind, hov_[h].arg);
            if (e && x >= e->x - 8 && y >= e->y - 8 && x < e->x + e->w + 8 && y < e->y + e->h + 8) {
                r->kind = e->kind;
                r->arg = e->arg;
                continue;
            }
        }
        for (size_t i = pn.el.size(); i-- > 0;) {       // the last drawn is on top
            const Elem& e = pn.el[i];
            if (x < e.x || y < e.y || x >= e.x + e.w || y >= e.y + e.h) continue;
            if (e.kind == eInfo) break;                  // text: nothing to press
            r->kind = e.kind;
            r->arg = e.arg;
            break;
        }
    }
    if (any) hitT_[h] = best;
    return any;
}

bool VrMenu::Cursor(int h, Pose* out) const {
    if (h < 0 || h > 1 || !aimOk_[h] || hov_[h].pan < 0) return false;
    const Panel& pn = pan_[hov_[h].pan];
    const V3 o = aim_[h].p, d = QRot(aim_[h].q, V3{ 0, 0, -1 });
    const V3 n = QRot(pn.pose.q, V3{ 0, 0, 1 });
    out->q = pn.pose.q;
    out->p = o + d * hitT_[h] + n * 0.004;
    return true;
}

bool VrMenu::ElemCentre(int p, int kind, int arg, V3* out) const {
    const Panel& pn = pan_[p];
    const Elem* e = pn.show ? Find(p, kind, arg) : nullptr;
    if (!e) return false;
    const double u = (e->x + e->w * 0.5) / pn.pw, v = (e->y + e->h * 0.5) / pn.ph;
    *out = pn.pose.p + QRot(pn.pose.q, V3{ (u - 0.5) * pn.width, (0.5 - v) * pn.Height(), 0 });
    return true;
}

std::string VrMenu::Describe() const {
    char b[256];
    snprintf(b, sizeof(b), "menu %s %.2f m at %.2f %.2f %.2f, keyboard %s %.2f m, tab %d, picking %s", pan_[0].show ? "shown" : "hidden",
             pan_[0].width, pan_[0].pose.p.x, pan_[0].pose.p.y, pan_[0].pose.p.z, pan_[1].show ? "shown" : "hidden", pan_[1].width, tab_,
             pickBtn_ >= 0 ? VrPad::BtnNames()[pickBtn_] : "-");
    return b;
}

void VrMenu::Save() { saveAt_ = 0; }   // written at the end of this Update (see Update)

void VrMenu::SavePlaces() {
    for (int p = 0; p < kPanCount; ++p) {
        if (!place_[p].set) continue;
        const Panel& pn = pan_[p];
        double yaw, pitch;
        YawPitchOf(QRot(pn.pose.q, V3{ 0, 0, -1 }), &yaw, &pitch);
        place_[p].x = pn.pose.p.x; place_[p].y = pn.pose.p.y; place_[p].z = pn.pose.p.z;
        place_[p].yaw = yaw / kDeg; place_[p].pitch = pitch / kDeg;
        place_[p].width = pn.width;
    }
    Save();
}

void VrMenu::Pick(const VrPad::Out& o) {
    if (pickBtn_ < 0) return;
    pad_.map[pickBtn_] = o;
    status_ = std::string("Set: ") + BtnLabel(pickBtn_) + " = " + Pretty(o);
    LOGI("VR menu: %s = %s", VrPad::BtnNames()[pickBtn_], Pretty(o).c_str());
    pickBtn_ = -1;
    for (bool& l : latch_) l = false;
    Save();
}

void VrMenu::SetValue(int arg) {
    auto mul = [](float& v, bool up, float lo, float hi) { v = up ? v * 1.15f : v / 1.15f; v = v < lo ? lo : v > hi ? hi : v; };
    switch (arg) {
    case 0: case 1: case 2: pad_.mouseStick = arg == 0 ? 1 : arg == 1 ? 2 : 0; break;
    case 10: case 11: mul(pad_.mouseSpeed, arg == 11, 50.0f, 10000.0f); break;
    case 20: case 21: {
        float d = pad_.deadzone + (arg == 21 ? 0.05f : -0.05f);
        pad_.deadzone = d < 0.0f ? 0.0f : d > 0.9f ? 0.9f : d;
        break;
    }
    case 30: case 31: case 32:
        pad_.airBtn = arg == 30 ? VrPad::kRGrip : arg == 31 ? VrPad::kLGrip : -1;
        if (pad_.airBtn >= 0) pad_.airHand = VrPad::HandOfBtn(pad_.airBtn);
        break;
    case 40: case 41: pad_.airToggle = arg == 41; break;
    case 50: case 51: mul(pad_.airGain, arg == 51, 1.0f, 500.0f); break;
    case 62: case 63: Push(kPanMenu, arg == 63 ? 0.05 : -0.05); return;
    case 72: case 73: if (pan_[kPanKb].show) Push(kPanKb, arg == 73 ? 0.05 : -0.05); return;
    case 90: case 91: case 92: case 93: {                // see-through, in steps of 10 %
        float& o = arg < 92 ? look_.menuOpacity : look_.kbOpacity;
        o += (arg & 1) ? 0.1f : -0.1f;
        o = o < 0.2f ? 0.2f : o > 1.0f ? 1.0f : o;
        o = (float)(int)(o * 10.0f + 0.5f) / 10.0f;
        needRedraw_ = true;
        break;
    }
    case 100: case 101: case 102: case 103: case 104: case 105: look_.theme = arg - 100; needRedraw_ = true; break;
    case 110: case 111: case 112: case 113: look_.smoothing = arg - 110; break;
    case 60: case 61: case 70: case 71: {
        Panel& pn = pan_[arg < 70 ? kPanMenu : kPanKb];
        pn.width *= (arg & 1) ? 1.12 : 1 / 1.12;
        pn.width = pn.width < 0.25 ? 0.25 : pn.width > 2.0 ? 2.0 : pn.width;
        SavePlaces();
        return;
    }
    case 82: case 83: SetKbCompact(arg == 83); return;
    case 80: ShowKb(true); return;
    case 81: ShowKb(false); return;
    default: return;
    }
    status_ = "Saved.";
    Save();
}

void VrMenu::Activate(int h, const Ref& r, const VrPad::Frame&, VrPad::Sink& sink) {
    Press& ps = press_[h];
    ps = Press();
    ps.pan = r.pan; ps.kind = r.kind; ps.arg = r.arg;
    Panel& pn = pan_[r.pan];
    static const char* const kKindName[] = { "-", "top bar", "close", "smaller", "bigger", "keyboard button", "tab", "button row",
                                             "pick", "pick cancel", "setting", "key", "bring here", "text" };
    LOGI("VR menu: %s hand presses %s %s %d", h ? "right" : "left", r.pan == kPanMenu ? "menu" : "keyboard", kKindName[r.kind],
         r.kind == eKey ? (int)VrPad::ScanOfName(kKeys[r.arg].name) : r.arg);
    switch (r.kind) {
    case eBar:
        ps.drag = true;
        ps.grab = PMul(PInv(aim_[h]), pn.pose);
        break;
    case eSmaller: case eBigger:
        pn.width *= r.kind == eBigger ? 1.12 : 1 / 1.12;
        pn.width = pn.width < 0.25 ? 0.25 : pn.width > 2.0 ? 2.0 : pn.width;
        SavePlaces();
        break;
    case eClose:
        if (r.pan == kPanMenu) Close();
        else ShowKb(false);
        break;
    case eKbBtn:
        ShowKb(!pan_[kPanKb].show);
        break;
    case eTab: tab_ = r.arg; status_.clear(); break;
    case eRow:
        if (r.arg == pad_.airBtn) { status_ = std::string(BtnLabel(r.arg)) + " is the air mouse button (Mouse tab)."; break; }
        if (r.arg >= VrPad::kLUp && pad_.mouseStick == (r.arg >= VrPad::kRUp ? 2 : 1)) {
            status_ = "That stick moves the mouse (Mouse tab).";
            break;
        }
        pickBtn_ = pickBtn_ == r.arg ? -1 : r.arg;
        for (bool& l : latch_) l = false;
        if (pickBtn_ >= 0 && !pan_[kPanKb].show) ShowKb(true);
        break;
    case ePickOut: { VrPad::Out o; o.kind = (uint8_t)r.arg; Pick(o); break; }
    case ePickCancel: pickBtn_ = -1; for (bool& l : latch_) l = false; break;
    case eSet: SetValue(r.arg); break;
    case eHere:
        Place(kPanMenu, lastHeadOk_, lastHead_, true);
        if (pan_[kPanKb].show) Place(kPanKb, lastHeadOk_, lastHead_, true);
        SavePlaces();
        break;
    case eKey: {
        const KeyDef& d = kKeys[r.arg];
        const int m = ModOf(d.name);
        if (m >= 0) { latch_[m] = !latch_[m]; break; }
        const uint16_t code = VrPad::ScanOfName(d.name);
        if (!code) break;
        if (pickBtn_ >= 0) {
            VrPad::Out o;
            o.kind = VrPad::kKeys;
            for (int i = 0; i < 3 && o.n < 2; ++i) if (latch_[i]) o.code[o.n++] = VrPad::ScanOfName(kModName[i]);
            o.code[o.n++] = code;
            Pick(o);
            break;
        }
        for (int i = 0; i < 3; ++i)
            if (latch_[i]) { ps.mods[ps.nm] = VrPad::ScanOfName(kModName[i]); sink.Key(ps.mods[ps.nm], true); ++ps.nm; }
        ps.key = code;
        sink.Key(code, true);
        break;
    }
    default: break;
    }
}

void VrMenu::Release(int h, VrPad::Sink& sink) {
    Press& ps = press_[h];
    if (ps.key) {
        sink.Key(ps.key, false);
        for (int i = ps.nm - 1; i >= 0; --i) sink.Key(ps.mods[i], false);
        if (ps.nm) for (bool& l : latch_) l = false;
    }
    if (ps.drag) SavePlaces();
    ps = Press();
}

void VrMenu::ReleaseAll(VrPad::Sink& sink) {
    bool held = false;
    for (int h = 0; h < 2; ++h) {
        if (press_[h].pan >= 0) Release(h, sink);
        held = held || grab_[h].pan >= 0;
        grab_[h] = Grab();
        gripDown_[h] = false;
    }
    if (held) SavePlaces();
}

void VrMenu::Update(VrPad::Frame& f, bool live, bool headOk, const Pose& head, double now, VrPad::Sink& sink) {
    lastHeadOk_ = headOk;
    if (headOk) lastHead_ = head;
    const double dt = lastNow_ < 0 ? 0.0 : (now - lastNow_ > 0.1 ? 0.1 : now - lastNow_);
    lastNow_ = now;
    if (kbPending_ && headOk) ShowKb(true);
    if (!live) {
        ReleaseAll(sink);
        hov_[0] = hov_[1] = Ref();
        trigDown_[0] = trigDown_[1] = false;
    } else {
        if (gate_.Step(f, now)) {
            if (pan_[kPanMenu].show) Close();
            else Open(headOk, head);
        }
        for (int h = 0; h < 2; ++h) {
            aimOk_[h] = f.hand[h] && f.aim[h] && f.pos[h];
            if (aimOk_[h]) {
                aim_[h].q = QNorm(Q4{ f.q[h][0], f.q[h][1], f.q[h][2], f.q[h][3] });
                aim_[h].p = V3{ f.p[h][0], f.p[h][1], f.p[h][2] };
                // hand-shake smoothing: rest cut-off (Hz) and its rise per deg/s; written back for the air mouse too
                static const double kMinCut[4] = { 0, 2.5, 1.2, 0.6 }, kBeta[4] = { 0, 0.04, 0.03, 0.02 };
                if (look_.smoothing > 0 && look_.smoothing < 4) {
                    aim_[h] = smooth_[h].Step(aim_[h], dt, kMinCut[look_.smoothing], kBeta[look_.smoothing]);
                    f.q[h][0] = aim_[h].q.x; f.q[h][1] = aim_[h].q.y; f.q[h][2] = aim_[h].q.z; f.q[h][3] = aim_[h].q.w;
                } else {
                    smooth_[h].Reset();
                }
            } else {
                smooth_[h].Reset();
            }
            Press& ps = press_[h];
            Grab& g = grab_[h];
            if (g.pan >= 0 && (!pan_[g.pan].show || !aimOk_[h])) { g = Grab(); SavePlaces(); }
            if (g.pan >= 0) {                                     // its stick pushes the held panel away / pulls it closer
                const double len = Len(g.off.p);
                const double sy = std::fabs(f.sy[h]) > 0.2 ? f.sy[h] : 0.0;
                if (len > 1e-6 && sy != 0.0) {
                    double nl = len + sy * 0.8 * dt;
                    nl = nl < 0.15 ? 0.15 : nl > 3.0 ? 3.0 : nl;
                    g.off.p = g.off.p * (nl / len);
                }
            }
            const Pose* hold = g.pan >= 0 ? &g.off : ps.drag ? &ps.grab : nullptr;
            const int heldPan = g.pan >= 0 ? g.pan : ps.drag ? ps.pan : -1;
            if (hold && aimOk_[h] && pan_[heldPan].show) {      // the panel hangs on the controller, kept level (no roll)
                const Pose np = PMul(aim_[h], *hold);
                double yaw, pitch;
                YawPitchOf(QRot(np.q, V3{ 0, 0, -1 }), &yaw, &pitch);
                pan_[heldPan].pose.p = np.p;
                pan_[heldPan].pose.q = QYawPitch(yaw, pitch);
            }
            Ref r;
            double u, v;
            if (!AnyShown() || !Hit(f, h, &r, &u, &v)) r = Ref();
            hov_[h] = r;
            const bool grip = gripDown_[h] ? f.grip[h] > VrPad::kReleaseAt : f.grip[h] > VrPad::kPressAt;
            if (grip && !gripDown_[h] && r.pan >= 0 && g.pan < 0 && !ps.drag) {
                g.pan = r.pan;
                g.off = PMul(PInv(aim_[h]), pan_[r.pan].pose);
                LOGI("VR menu: %s hand grabs the %s", h ? "right" : "left", r.pan == kPanMenu ? "menu" : "keyboard");
            }
            if (!grip && g.pan >= 0) { g = Grab(); SavePlaces(); }
            gripDown_[h] = grip;
            if (r.pan >= 0 || g.pan >= 0) f.grip[h] = 0.0f;                         // pointing at / holding a panel: no air mouse
            if (g.pan >= 0) { f.sx[h] = f.sy[h] = 0.0f; }                            // holding: the stick moves the panel
            const bool down = trigDown_[h] ? f.trig[h] > VrPad::kReleaseAt : f.trig[h] > VrPad::kPressAt;
            if (down && !trigDown_[h] && r.pan >= 0) Activate(h, r, f, sink);
            if (!down && press_[h].pan >= 0) Release(h, sink);
            if (press_[h].pan >= 0 && !pan_[press_[h].pan].show) Release(h, sink);   // (its panel was closed by that press)
            trigDown_[h] = down;
            if (hov_[h].pan >= 0 || press_[h].pan >= 0) f.trig[h] = 0.0f;           // this trigger belongs to the menu now
        }
    }
    if (AnyShown()) {                                   // (closed: nothing drawn)
        const std::string ck = ContentKey();
        for (int p = 0; p < kPanCount; ++p)
            if (pan_[p].show && (needRedraw_ || ck != content_[p] || !DrawChanged(p))) Redraw(p);
    }
    needRedraw_ = false;
    if (saveAt_ >= 0) {
        saveAt_ = -1;
        if (SaveConfigValues(path_, MenuValues(pad_, place_[kPanMenu], place_[kPanKb], look_)))
            LOGI("VR menu: settings saved to %ls", path_.c_str());
        else if (!path_.empty()) {
            LOGW("VR menu: could not write %ls -- the change works until the game ends", path_.c_str());
            status_ = "Could not save config.ini (the change works until the game ends).";
        }
        needRedraw_ = true;
    }
}

std::string VrMenu::ContentKey() const {
    char b[160];
    snprintf(b, sizeof(b), "%d%d|%d|%d|%d%d%d|%.3f,%.3f|", pan_[0].show, pan_[1].show, tab_, pickBtn_, latch_[0], latch_[1], latch_[2],
             pan_[0].width, pan_[1].width);
    char l[64];
    snprintf(l, sizeof(l), "%d|%.2f|%.2f|%d|%d|", look_.theme, look_.menuOpacity, look_.kbOpacity, look_.smoothing, look_.kbCompact);
    return std::string(b) + l + status_ + "|" + DescribePad(pad_);
}

}  // namespace ck
