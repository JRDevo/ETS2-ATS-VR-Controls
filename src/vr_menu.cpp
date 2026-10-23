// vr_menu -- see vr_menu.h.
#include "vr_menu.h"
#include <windows.h>
#include <cctype>
#include <cmath>
#include <condition_variable>
#include <cstdio>
#include <cstring>
#include <deque>
#include <initializer_list>
#include <atomic>
#include <thread>
#include <utility>
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
    // fit = true: a smaller font (down to 60 %) when the text is wider than the box minus 12 px, before "..." is used
    void Text(int x, int y, int w, int h, const std::string& s, int px, bool bold, COLORREF c, UINT align, bool fit = false) {
        if (s.empty()) return;
        const std::wstring wt = Wide(s);
        for (int min = px * 6 / 10; fit && px > min; px -= 2) {
            SelectObject(dc_, Font(px, bold));
            SIZE sz{};
            if (!GetTextExtentPoint32W(dc_, wt.c_str(), (int)wt.size(), &sz) || sz.cx <= w - 12) break;
        }
        SelectObject(dc_, Font(px, bold));
        SetTextColor(dc_, c);
        RECT r{ x, y, x + w, y + h };
        const std::wstring ws = Wide(s);
        DrawTextW(dc_, ws.c_str(), (int)ws.size(), &r, align | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    // the picture's area [x0, x1) x [y0, y1) into `out` (w x h) with `alpha` (0..255) everywhere, premultiplied (GDI leaves
    // alpha 0); `out` of another size: all of it
    void CopyTo(std::vector<uint32_t>& out, int alpha, int x0, int y0, int x1, int y1) {
        GdiFlush();
        if (out.size() != (size_t)w_ * h_) { out.assign((size_t)w_ * h_, 0); x0 = y0 = 0; x1 = w_; y1 = h_; }
        x0 = x0 < 0 ? 0 : x0; y0 = y0 < 0 ? 0 : y0; x1 = x1 > w_ ? w_ : x1; y1 = y1 > h_ ? h_ : y1;
        const uint32_t a = (uint32_t)(alpha < 0 ? 0 : alpha > 255 ? 255 : alpha);
        for (int y = y0; y < y1; ++y) {
            const uint32_t* in = static_cast<const uint32_t*>(bits_) + (size_t)y * w_;
            uint32_t* o = out.data() + (size_t)y * w_;
            for (int x = x0; x < x1; ++x) {
                const uint32_t p = in[x];
                o[x] = a >= 255 ? (p | 0xFF000000u)
                                : (a << 24) | ((((p >> 16) & 0xFFu) * a / 255u) << 16) | ((((p >> 8) & 0xFFu) * a / 255u) << 8) | ((p & 0xFFu) * a / 255u);
            }
        }
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

// ------------------------------------------------------------------------------------------------ the painter

namespace {
// sRGB <-> linear for the shrink filter (8-bit in; a 4096-step table back)
const float* ToLin() {
    static const float* t = [] {
        static float a[256];
        for (int i = 0; i < 256; ++i) { const double c = i / 255.0; a[i] = (float)(c <= 0.04045 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4)); }
        return a;
    }();
    return t;
}
const uint8_t* ToSrgb() {
    static const uint8_t* t = [] {
        static uint8_t a[4096];
        for (int i = 0; i < 4096; ++i) {
            const double l = i / 4095.0, c = l <= 0.0031308 ? l * 12.92 : 1.055 * std::pow(l, 1 / 2.4) - 0.055;
            a[i] = (uint8_t)(c * 255.0 + 0.5);
        }
        return a;
    }();
    return t;
}
// The area filter: each target pixel = the average of the source pixels it covers (colour in linear light). One axis' weights.
struct Axis { std::vector<int> first, count, off; std::vector<float> w; int src = 0, dst = 0; };
void MakeAxis(int src, int dst, Axis& a) {
    a.src = src; a.dst = dst;
    a.first.assign((size_t)dst, 0); a.count.assign((size_t)dst, 0); a.off.assign((size_t)dst, 0); a.w.clear();
    const double k = (double)src / dst;               // source pixels per target pixel (>= 1)
    for (int j = 0; j < dst; ++j) {
        const double s0 = j * k, s1 = (j + 1) * k;
        int i0 = (int)s0, i1 = (int)std::ceil(s1) - 1;
        if (i1 >= src) i1 = src - 1;
        if (i1 < i0) i1 = i0;
        a.first[(size_t)j] = i0;
        a.count[(size_t)j] = i1 - i0 + 1;
        a.off[(size_t)j] = (int)a.w.size();
        for (int i = i0; i <= i1; ++i) {
            const double lo = i > s0 ? i : s0, hi = i + 1 < s1 ? i + 1 : s1;
            a.w.push_back((float)((hi - lo) / k));
        }
    }
}
// target pixels [x0, x1) x [y0, y1) of `dst` (dw x dh) from `src` (sw x sh); ax / ay: the weights (made when their sizes differ)
void Shrink(const uint32_t* src, int sw, int sh, uint32_t* dst, int dw, int dh, int x0, int y0, int x1, int y1, Axis& ax, Axis& ay) {
    if (ax.src != sw || ax.dst != dw) MakeAxis(sw, dw, ax);
    if (ay.src != sh || ay.dst != dh) MakeAxis(sh, dh, ay);
    const float* lin = ToLin();
    const uint8_t* srgb = ToSrgb();
    std::vector<float> acc((size_t)(x1 - x0) * 4);
    for (int y = y0; y < y1; ++y) {
        std::fill(acc.begin(), acc.end(), 0.0f);
        for (int r = 0; r < ay.count[(size_t)y]; ++r) {
            const float wy = ay.w[(size_t)(ay.off[(size_t)y] + r)];
            const uint32_t* sr = src + (size_t)(ay.first[(size_t)y] + r) * sw;
            for (int x = x0; x < x1; ++x) {
                float c0 = 0, c1 = 0, c2 = 0, ca = 0;
                const int f = ax.first[(size_t)x], n = ax.count[(size_t)x];
                const float* wx = &ax.w[(size_t)ax.off[(size_t)x]];
                for (int i = 0; i < n; ++i) {
                    const uint32_t q = sr[f + i];
                    c0 += wx[i] * lin[q & 0xFF]; c1 += wx[i] * lin[(q >> 8) & 0xFF]; c2 += wx[i] * lin[(q >> 16) & 0xFF];
                    ca += wx[i] * (float)(q >> 24);
                }
                float* o = &acc[(size_t)(x - x0) * 4];
                o[0] += wy * c0; o[1] += wy * c1; o[2] += wy * c2; o[3] += wy * ca;
            }
        }
        uint32_t* dr = dst + (size_t)y * dw;
        for (int x = x0; x < x1; ++x) {
            const float* o = &acc[(size_t)(x - x0) * 4];
            auto enc = [&](float v) { const int i = (int)(v * 4095.0f + 0.5f); return (uint32_t)srgb[i < 0 ? 0 : i > 4095 ? 4095 : i]; };
            const int a = (int)(o[3] + 0.5f);
            dr[x] = ((uint32_t)(a < 0 ? 0 : a > 255 ? 255 : a) << 24) | (enc(o[2]) << 16) | (enc(o[1]) << 8) | enc(o[0]);
        }
    }
}
void Unite(int d[4], int x0, int y0, int x1, int y1) {
    if (d[2] <= d[0] || d[3] <= d[1]) { d[0] = x0; d[1] = y0; d[2] = x1; d[3] = y1; return; }
    d[0] = d[0] < x0 ? d[0] : x0; d[1] = d[1] < y0 ? d[1] : y0; d[2] = d[2] > x1 ? d[2] : x1; d[3] = d[3] > y1 ? d[3] : y1;
}
}  // namespace

// One background thread at below-normal priority: paint jobs (a whole panel, or only some elements) and config.ini writes, in
// order; the output size / channel order of each panel (Output) -- a change re-shrinks the whole picture. Its result per
// panel: `src` (full size, BGRA, the tests' picture) and the published Picture (pub, under pubM).
class Painter {
public:
    struct Job {
        int p = 0;
        bool full = false, frame = false, save = false;
        int w = 0, h = 0, theme = 0, alpha = 255;
        std::vector<VrMenu::Elem> el;
        std::vector<uint8_t> st;
        std::wstring path;                                     // save: config.ini
        std::vector<std::pair<std::string, std::string>> kv;
    };
    Painter() { th_ = std::thread([this] { Run(); }); }
    ~Painter() {
        { std::lock_guard<std::mutex> lk(m_); stop_ = true; }
        cv_.notify_all();
        if (th_.joinable()) th_.join();
    }
    void Post(Job&& j) {
        { std::lock_guard<std::mutex> lk(m_); q_.push_back(std::move(j)); }
        cv_.notify_one();
    }
    void SetOut(int p, int w, int h, bool rgba) {
        {
            std::lock_guard<std::mutex> lk(m_);
            if (out_[p].w == w && out_[p].h == h && out_[p].rgba == rgba) return;
            out_[p] = { w, h, rgba };
            outChanged_[p] = true;
        }
        cv_.notify_one();
    }
    bool OutReady(int p) {                                    // the published picture has the size asked for
        std::lock_guard<std::mutex> lk(pubM);
        return pub[p].w == pubWant_[p][0] && pub[p].h == pubWant_[p][1] && pub[p].w > 0;
    }
    void Flush() {
        std::unique_lock<std::mutex> lk(m_);
        idle_.wait(lk, [this] { return q_.empty() && !busy_ && !outChanged_[0] && !outChanged_[1]; });
    }
    bool TakeSaveFailed() { return saveFailed_.exchange(false); }
    const std::vector<uint32_t>& Src(int p) const { return src_[p]; }
    std::mutex pubM;
    VrMenu::Picture pub[VrMenu::kPanCount];

private:
    struct Out { int w = 0, h = 0; bool rgba = false; };
    void Run() {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);   // never ahead of the game's threads
        for (;;) {
            Job j;
            bool has = false, re[VrMenu::kPanCount] = {};
            Out out[VrMenu::kPanCount];
            {
                std::unique_lock<std::mutex> lk(m_);
                cv_.wait(lk, [this] { return stop_ || !q_.empty() || outChanged_[0] || outChanged_[1]; });
                if (stop_) return;
                if (!q_.empty()) { j = std::move(q_.front()); q_.pop_front(); has = true; }
                for (int p = 0; p < VrMenu::kPanCount; ++p) { re[p] = outChanged_[p]; outChanged_[p] = false; out[p] = out_[p]; }
                busy_ = true;
            }
            try {
                if (has && j.save) Save(j);
                else if (has) Paint(j, out[j.p]);
                for (int p = 0; p < VrMenu::kPanCount; ++p)
                    if (re[p] && srcW_[p] > 0) { int all[4] = { 0, 0, srcW_[p], srcH_[p] }; Publish(p, all, out[p], true); }
            } catch (...) {
                LOGE("VR menu: the painter failed (exception) -- skipped");
            }
            { std::lock_guard<std::mutex> lk(m_); busy_ = false; }
            idle_.notify_all();
        }
    }
    void Save(const Job& j) {
        if (SaveConfigValues(j.path, j.kv)) LOGI("VR menu: settings saved to %ls", j.path.c_str());
        else if (!j.path.empty()) {
            LOGW("VR menu: could not write %ls -- the change works until the game ends", j.path.c_str());
            saveFailed_ = true;
        }
    }
    void Paint(const Job& j, const Out& out) {
        std::unique_ptr<MenuCanvas>& cvp = canvas_[j.p];
        bool full = j.full;
        if (!cvp || cvp->W() != j.w || cvp->H() != j.h) { cvp = std::make_unique<MenuCanvas>(j.w, j.h); full = true; }
        MenuCanvas& c = *cvp;
        if (!c.Ok()) return;
        if (full && !j.full) return;                          // a partial job for a canvas never drawn whole: wait for a full one
        const Theme& T = kThemes[j.theme];
        int d[4] = { 0, 0, 0, 0 };
        if (full) {
            c.Round(0, 0, j.w, j.h, 0, j.frame ? T.accent : T.bg);   // (picking a key: an accent frame)
            if (j.frame) c.Round(5, 5, j.w - 11, j.h - 11, 0, T.bg);
            for (size_t i = 0; i < j.el.size(); ++i) VrMenu::DrawElem(c, j.theme, j.el[i], j.st[i]);
            d[2] = j.w; d[3] = j.h;
        } else {
            for (size_t i = 0; i < j.el.size(); ++i) {
                const VrMenu::Elem& e = j.el[i];
                c.Round(e.x, e.y, e.w, e.h, 0, j.p == VrMenu::kPanMenu && e.y < 72 ? T.bar : T.bg);
                VrMenu::DrawElem(c, j.theme, e, j.st[i]);
                Unite(d, e.x - 1, e.y - 1, e.x + e.w + 2, e.y + e.h + 2);
            }
        }
        if (d[2] <= d[0]) return;
        c.CopyTo(src_[j.p], j.alpha, d[0], d[1], d[2], d[3]);
        if (srcW_[j.p] != j.w || srcH_[j.p] != j.h) { srcW_[j.p] = j.w; srcH_[j.p] = j.h; d[0] = d[1] = 0; d[2] = j.w; d[3] = j.h; }
        Publish(j.p, d, out, false);
    }
    // the changed area d of src -> the output picture (shrunk or copied, channel order), then published
    void Publish(int p, const int d[4], const Out& out, bool all) {
        const int sw = srcW_[p], sh = srcH_[p];
        const int ow = out.w > 0 ? out.w : sw, oh = out.h > 0 ? out.h : sh;
        std::vector<uint32_t>& ob = outBuf_[p];
        int r[4] = { d[0], d[1], d[2], d[3] };
        if (ob.size() != (size_t)ow * oh || all || outW_[p] != ow || outH_[p] != oh || outRgba_[p] != out.rgba) {
            ob.assign((size_t)ow * oh, 0);
            outW_[p] = ow; outH_[p] = oh; outRgba_[p] = out.rgba;
            r[0] = r[1] = 0; r[2] = ow; r[3] = oh;
        } else if (ow != sw || oh != sh) {                 // the changed area in the output (+ 1 px: the filter's reach)
            const double kx = (double)ow / sw, ky = (double)oh / sh;
            r[0] = (int)(d[0] * kx) - 1; r[1] = (int)(d[1] * ky) - 1; r[2] = (int)std::ceil(d[2] * kx) + 1; r[3] = (int)std::ceil(d[3] * ky) + 1;
            r[0] = r[0] < 0 ? 0 : r[0]; r[1] = r[1] < 0 ? 0 : r[1]; r[2] = r[2] > ow ? ow : r[2]; r[3] = r[3] > oh ? oh : r[3];
        }
        if (r[2] <= r[0] || r[3] <= r[1]) return;
        if (ow != sw || oh != sh) Shrink(src_[p].data(), sw, sh, ob.data(), ow, oh, r[0], r[1], r[2], r[3], ax_[p], ay_[p]);
        else for (int y = r[1]; y < r[3]; ++y) memcpy(ob.data() + (size_t)y * ow + r[0], src_[p].data() + (size_t)y * sw + r[0], (size_t)(r[2] - r[0]) * 4);
        if (out.rgba)                                       // ours are BGRA
            for (int y = r[1]; y < r[3]; ++y)
                for (uint32_t* q = ob.data() + (size_t)y * ow + r[0], *e = ob.data() + (size_t)y * ow + r[2]; q < e; ++q)
                    *q = (*q & 0xFF00FF00u) | ((*q >> 16) & 0xFFu) | ((*q & 0xFFu) << 16);
        std::lock_guard<std::mutex> lk(pubM);
        VrMenu::Picture& pb = pub[p];
        if (pb.w != ow || pb.h != oh || pb.px.size() != ob.size()) {
            pb.px = ob;
            pb.w = ow; pb.h = oh;
            pb.dirty[0] = pb.dirty[1] = 0; pb.dirty[2] = ow; pb.dirty[3] = oh;
        } else {
            for (int y = r[1]; y < r[3]; ++y) memcpy(pb.px.data() + (size_t)y * ow + r[0], ob.data() + (size_t)y * ow + r[0], (size_t)(r[2] - r[0]) * 4);
            Unite(pb.dirty, r[0], r[1], r[2], r[3]);
        }
        ++pb.version;
    }
public:
    void WantOut(int p, int w, int h) { std::lock_guard<std::mutex> lk(pubM); pubWant_[p][0] = w; pubWant_[p][1] = h; }
private:
    std::thread th_;
    std::mutex m_;
    std::condition_variable cv_, idle_;
    std::deque<Job> q_;
    bool stop_ = false, busy_ = false;
    Out out_[VrMenu::kPanCount];
    bool outChanged_[VrMenu::kPanCount] = {};
    std::atomic<bool> saveFailed_{ false };
    std::unique_ptr<MenuCanvas> canvas_[VrMenu::kPanCount];
    std::vector<uint32_t> src_[VrMenu::kPanCount], outBuf_[VrMenu::kPanCount];
    int srcW_[VrMenu::kPanCount] = {}, srcH_[VrMenu::kPanCount] = {}, outW_[VrMenu::kPanCount] = {}, outH_[VrMenu::kPanCount] = {};
    bool outRgba_[VrMenu::kPanCount] = {};
    int pubWant_[VrMenu::kPanCount][2] = {};
    Axis ax_[VrMenu::kPanCount], ay_[VrMenu::kPanCount];
};


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

VrMenu::VrMenu(const Config& cfg, const std::wstring& path) : pad_(cfg.pad), path_(path), painter_(std::make_unique<Painter>()) {
    gate_.btns = cfg.menuCombo;
    gate_.hold = cfg.menuHold;
    place_[kPanMenu] = cfg.menuPlace;
    place_[kPanKb] = cfg.kbPlace;
    pan_[kPanMenu].pw = kMenuW; pan_[kPanMenu].ph = kMenuH; pan_[kPanMenu].width = kMenuWidth;
    pan_[kPanKb].pw = kKbW; pan_[kPanKb].ph = kKbH; pan_[kPanKb].width = kKbWidth;
    look_ = cfg.look;
    if (look_.theme < 0 || look_.theme >= kThemeCount) look_.theme = 0;
    if (look_.kbCompact) { pan_[kPanKb].pw = kKbCompactW; pan_[kPanKb].width *= (double)kKbCompactW / kKbW; }   // (same key size)
    for (int p = 0; p < kPanCount; ++p)
        if (place_[p].set) { pan_[p].width = place_[p].width; Place(p, false, Pose(), false); }
    if (look_.kbShow) {                                 // the keyboard stays from the last game (at its place; else in front of
        if (place_[kPanKb].set) pan_[kPanKb].show = true;   //   the head at the first frame that has the head)
        else kbPending_ = true;
    }
    // both panels drawn now (on the painter's thread, while the game loads): opening the menu then only shows them
    RedrawAll();
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
        pl.yaw = yaw / kDeg; pl.pitch = pitch / kDeg; pl.roll = 0;
        pl.width = pn.width;
    }
    pn.width = pl.width;
    pn.pose.p = V3{ pl.x, pl.y, pl.z };
    const double rr = 0.5 * pl.roll * kDeg;            // the twist about the panel's own axis (+z)
    pn.pose.q = QNorm(QMul(QYawPitch(pl.yaw * kDeg, pl.pitch * kDeg), Q4{ 0, 0, std::sin(rr), std::cos(rr) }));
}

void VrMenu::Open(bool headOk, const Pose& head) {
    Place(kPanMenu, headOk, head, false);
    pan_[kPanMenu].show = true;
    status_.clear();                                    // (redrawn in Update only if what it shows changed meanwhile)
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

// steering (pad_map.h SteerStickOf) takes that stick's left / right directions
static bool DrivingUses(const VrPad::Config& p, int btn) {
    return btn >= VrPad::kLUp && (btn - VrPad::kLUp) % 4 >= 2 && VrPad::SteerStickOf(p) == (btn >= VrPad::kRUp ? 2 : 1);
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
    const std::string what = picking ? (pickMod_ && pad_.modBtn >= 0 ? std::string(BtnLabel(pad_.modBtn)) + " + " : std::string()) + BtnLabel(pickBtn_)
                                     : std::string();
    add(eBar, 0, 0, 0, kMenuW, 72, picking ? "Set " + what + ": pick a key" : "ETS2 ATS VR Controls", picking);
    add(eSmaller, 0, 600, 11, 64, 50, "\xE2\x88\x92");
    add(eBigger, 0, 672, 11, 64, 50, "+");
    add(eKbBtn, 0, 744, 11, 170, 50, "Keyboard", pan_[kPanKb].show);
    add(eClose, 0, 924, 11, 64, 50, "\xC3\x97");
    static const char* const kTabs[5] = { "Buttons", "Mouse", "Panels", "Look", "Driving" };
    for (int t = 0; t < 5; ++t) add(eTab, t, 24 + t * 193, 84, 181, 56, kTabs[t], tab_ == t);
    char b[96];
    if (tab_ == 0) {
        static const int kLeft[10] = { VrPad::kLTrig, VrPad::kLGrip, VrPad::kX, VrPad::kY, VrPad::kLStick, VrPad::kMenu,
                                       VrPad::kLUp, VrPad::kLDown, VrPad::kLLeft, VrPad::kLRight };
        static const int kRight[9] = { VrPad::kRTrig, VrPad::kRGrip, VrPad::kA, VrPad::kB, VrPad::kRStick, VrPad::kRUp,
                                       VrPad::kRDown, VrPad::kRLeft, VrPad::kRRight };
        // Normal / With modifier: which mapping the rows show and set; the modifier button itself (a click cycles it)
        add(eSet, 120, 24, 150, 230, 44, "Normal", layer_ == 0);
        add(eSet, 121, 264, 150, 230, 44, "With modifier", layer_ == 1);
        add(eSet, 122, 504, 150, 472, 44, pad_.modBtn >= 0 ? std::string("Modifier: ") + BtnLabel(pad_.modBtn) : "Modifier: off");
        for (int col = 0; col < 2; ++col)
            for (int i = 0; i < (col ? 9 : 10); ++i) {
                const int btn = col ? kRight[i] : kLeft[i];
                std::string val = Pretty(layer_ ? pad_.modMap[btn] : pad_.map[btn]);
                bool dim = false;
                if (layer_ && btn == pad_.modBtn) { val = "the modifier"; dim = true; }
                else if (btn == pad_.airBtn) { val = "air mouse"; dim = true; }
                else if (btn >= VrPad::kLUp && pad_.mouseStick == (btn >= VrPad::kRUp ? 2 : 1)) { val = "moves the mouse"; dim = true; }
                else if (DrivingUses(pad_, btn)) { val = "steers"; dim = true; }
                else if (layer_ && (pad_.modBtn < 0 || pad_.modMap[btn].kind == VrPad::kNone)) { val = "as normal"; dim = true; }
                bool combo = false;
                for (int cb : gate_.btns) combo = combo || cb == btn;
                add(eRow, btn, col ? 508 : 24, 202 + i * 49, 468, 45, BtnLabel(btn), pickBtn_ == btn, dim,
                    combo && !layer_ ? val + "  (+menu)" : val);
            }
        if (picking) {
            static const int kOuts[8] = { VrPad::kMouseL, VrPad::kMouseR, VrPad::kMouseM, VrPad::kWheelUp, VrPad::kWheelDown,
                                          VrPad::kGas, VrPad::kBrake, VrPad::kNone };
            static const char* const kOutNames[8] = { "Mouse L", "Mouse R", "Mouse M", "Wheel up", "Wheel dn", "Gas", "Brake", "None" };
            for (int i = 0; i < 8; ++i) add(ePickOut, kOuts[i], 24 + i * 103, 716, 97, 64, kOutNames[i]);
            add(ePickCancel, 0, 852, 716, 124, 64, "Cancel");
        } else {
            add(eInfo, 0, 24, 716, 742, 64, !status_.empty() ? status_
                : layer_ ? "While the modifier is held, these buttons send this instead." : "Point at a button, pull the trigger, pick its key.");
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
        add(eInfo, 0, 24, 716, 742, 64, status_.empty() ? "Changes work at once and are saved." : status_);
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
        add(eInfo, 0, 24, 716, 742, 64, status_.empty() ? "The keyboard stays when the menu closes (its x hides it)." : status_);
    } else if (tab_ == 3) {
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
        add(eInfo, 0, 24, 716, 742, 64, status_.empty() ? "- = more see-through, + = more solid." : status_);
    } else {                                            // Driving (scs_input.h): steering here, gas / brake on any button
        auto row = [&](int r, const char* label) { add(eInfo, 100 + r, 24, 156 + r * 68, 440, 60, label); };
        auto opts = [&](int r, int base, std::initializer_list<const char*> names, int cur) {
            const int n = (int)names.size(), w = (506 - (n - 1) * 10) / n;
            int i = 0;
            for (const char* nm : names) { add(eSet, base + i, 470 + i * (w + 10), 156 + r * 68, w, 60, nm, i == cur); ++i; }
        };
        auto plusMinus = [&](int r, int base, const std::string& value) {
            add(eSet, base, 470, 156 + r * 68, 90, 60, "\xE2\x88\x92");
            add(eInfo, 200 + r, 570, 156 + r * 68, 306, 60, value);
            add(eSet, base + 1, 886, 156 + r * 68, 90, 60, "+");
        };
        const bool rate = pad_.steerMode == 1;
        row(0, "Driving");
        opts(0, 147, { "On", "Off" }, pad_.drive ? 0 : 1);
        row(1, "Steering style");
        opts(1, 141, { "Turn speed", "Wheel" }, rate ? 0 : 1);
        row(2, "Stick that steers");
        opts(2, 130, { "Left", "Right", "Off" }, pad_.steerStick == 1 ? 0 : pad_.steerStick == 2 ? 1 : 2);
        row(3, "Steering dead zone");
        snprintf(b, sizeof(b), "%.2f", (double)pad_.steerDeadzone);
        plusMinus(3, 133, b);
        row(4, "Steering curve");
        snprintf(b, sizeof(b), pad_.steerCurve < 1.01f ? "%.2f (straight)" : "%.2f", (double)pad_.steerCurve);
        plusMinus(4, 135, b);
        row(5, rate ? "Wheel turns at most" : "Full stick turns the wheel");
        snprintf(b, sizeof(b), "%d %%", (int)(pad_.steerMax * 100.0f + 0.5f));
        plusMinus(5, 137, b);
        if (rate) {
            row(6, "Full stick: full turn in");
            snprintf(b, sizeof(b), "%.3g s", (double)pad_.steerTurnTime);
            plusMinus(6, 143, b);
            row(7, "Let go: straight again in");
            snprintf(b, sizeof(b), "%.3g s", (double)pad_.steerReturnTime);
            plusMinus(7, 145, b);
        } else {
            row(6, "Steering speed");
            if (pad_.steerSpeed <= 0.0f) snprintf(b, sizeof(b), "at once");
            else snprintf(b, sizeof(b), "%.2g s to full", 1.0 / (double)pad_.steerSpeed);
            plusMinus(6, 139, b);
        }
        add(eInfo, 0, 24, 716, 742, 64, !status_.empty() ? status_ : !pad_.drive ? "Driving off: no steering, gas or brake."
                                        : rate ? "Push a little = turn slowly, all the way = fast." : "The stick's place = the wheel's place.");
    }
    if (!(tab_ == 0 && picking))                        // every tab: back to the defaults (a second press does it)
        add(eSet, kResetArg, 776, 716, 200, 64, resetArm_ == tab_ ? "Sure? Reset" : "Reset tab", resetArm_ == tab_);
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

void VrMenu::Draw(int p) {                              // the whole panel: a job for the painter
    Panel& pn = pan_[p];
    Painter::Job j;
    j.p = p; j.full = true; j.w = pn.pw; j.h = pn.ph; j.theme = look_.theme;
    j.alpha = (int)((p == kPanMenu ? look_.menuOpacity : look_.kbOpacity) * 255.0f + 0.5f);
    j.frame = p == kPanKb && pickBtn_ >= 0;
    j.el = pn.el;
    elState_[p].assign(pn.el.size(), 0);
    for (size_t i = 0; i < pn.el.size(); ++i) elState_[p][i] = (uint8_t)ElemState(p, pn.el[i]);
    j.st = elState_[p];
    painter_->Post(std::move(j));
}

bool VrMenu::Output(int p, double wantPx, bool rgba) {
    // the size it is shown at: shrunk to its width in the headset in 10 % steps (each step a new swapchain), none at 80 % or
    // more (no visible gain), changed only when the wanted size leaves -20 % .. +15 % of the current one
    const Panel& pn = pan_[p];
    double scale = 1.0;
    if (wantPx > 0) {
        double want = wantPx / pn.pw;
        want = want < 0.2 ? 0.2 : want > 1.0 ? 1.0 : want;
        scale = scaleOf_[p] == pn.pw ? scale_[p] : 0.0;
        if (scale <= 0 || want > scale * 1.15 || want < scale / 1.25) {
            scale = 1.0;
            if (want < 0.8) while (scale / 1.1 >= want) scale /= 1.1;   // the smallest step still at least as big as wanted
        }
    } else if (scaleOf_[p] == pn.pw && scale_[p] > 0) {
        scale = scale_[p];
    }
    int ow = (int)(pn.pw * scale + 0.5), oh = (int)(pn.ph * scale + 0.5);
    ow = ow < 16 ? 16 : ow; oh = oh < 16 ? 16 : oh;
    if (scale != scale_[p] || scaleOf_[p] != pn.pw) {
        if (scale < 1.0) LOGI("VR menu: %s shown at %d x %d (%.0f %% of its %d x %d picture: its size in the headset)",
                              p == kPanMenu ? "menu" : "keyboard", ow, oh, scale * 100.0, pn.pw, pn.ph);
        scale_[p] = scale;
        scaleOf_[p] = pn.pw;
    }
    painter_->WantOut(p, ow, oh);
    painter_->SetOut(p, ow, oh, rgba);
    return painter_->OutReady(p);
}

std::unique_lock<std::mutex> VrMenu::LockPictures() { return std::unique_lock<std::mutex>(painter_->pubM); }
VrMenu::Picture& VrMenu::PictureLocked(int p) { return painter_->pub[p]; }
void VrMenu::Flush() { painter_->Flush(); }
const std::vector<uint32_t>& VrMenu::FullPicture(int p) { painter_->Flush(); return painter_->Src(p); }

int VrMenu::ElemState(int p, const Elem& e) const {
    int st = 0;
    if (e.kind == eBar) return 0;                       // the bar never changes its look (a shaky pointer on it made it blink)
    for (int h = 0; h < 2; ++h) {
        if (hov_[h].pan == p && hov_[h].kind == e.kind && hov_[h].arg == e.arg) st |= 1;
        if (press_[h].pan == p && press_[h].kind == e.kind && press_[h].arg == e.arg) st |= 2;
    }
    return st;
}

// A pointer moving over the keys changes two elements: only those go to the painter (a whole panel is ~2 ms of GDI).
bool VrMenu::DrawChanged(int p) {
    Panel& pn = pan_[p];
    if (elState_[p].size() != pn.el.size()) return false;
    Painter::Job j;
    for (size_t i = 0; i < pn.el.size(); ++i) {
        const Elem& e = pn.el[i];
        const int st = ElemState(p, e);
        if (st == elState_[p][i]) continue;
        if (e.kind == eBar) return false;            // the bar lies under its buttons: all again
        elState_[p][i] = (uint8_t)st;
        j.el.push_back(e);
        j.st.push_back((uint8_t)st);
    }
    if (j.el.empty()) return true;
    j.p = p; j.w = pn.pw; j.h = pn.ph; j.theme = look_.theme;
    j.alpha = (int)((p == kPanMenu ? look_.menuOpacity : look_.kbOpacity) * 255.0f + 0.5f);
    painter_->Post(std::move(j));
    return true;
}

void VrMenu::Redraw(int p) {
    Layout(p);
    Draw(p);
    content_[p] = ContentKey(p);
}

void VrMenu::DrawElem(MenuCanvas& c, int theme, const Elem& e, int st) {
    const Theme& T = kThemes[theme];
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
        c.Text(e.x + 210, e.y, e.w - 226, e.h, e.sub, 26, true, e.dim ? T.dim : T.text, DT_RIGHT, true);
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
               true, T.text, DT_CENTER, true);
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

void VrMenu::Save() { saveDue_ = true; }   // written by the painter at the end of this Update (see Update)

void VrMenu::SavePlaces() {
    for (int p = 0; p < kPanCount; ++p) {
        if (!place_[p].set) continue;
        const Panel& pn = pan_[p];
        double yaw, pitch;
        YawPitchOf(QRot(pn.pose.q, V3{ 0, 0, -1 }), &yaw, &pitch);
        const Q4 r = QMul(QConj(QYawPitch(yaw, pitch)), pn.pose.q);   // what is left = the twist about the panel's own axis
        place_[p].x = pn.pose.p.x; place_[p].y = pn.pose.p.y; place_[p].z = pn.pose.p.z;
        place_[p].yaw = yaw / kDeg; place_[p].pitch = pitch / kDeg;
        place_[p].roll = 2.0 * std::atan2(r.z, r.w) / kDeg;
        if (place_[p].roll > 180.0) place_[p].roll -= 360.0;
        if (place_[p].roll < -180.0) place_[p].roll += 360.0;
        place_[p].width = pn.width;
    }
    Save();
}

void VrMenu::Pick(const VrPad::Out& o) {
    if (pickBtn_ < 0) return;
    if (pickMod_) {                                     // the entry for while the modifier is held (None = as normal)
        pad_.modMap[pickBtn_] = o;
        status_ = std::string("Set: ") + (pad_.modBtn >= 0 ? BtnLabel(pad_.modBtn) : "Modifier") + " + " + BtnLabel(pickBtn_) + " = " +
                  (o.kind == VrPad::kNone ? std::string("as normal") : Pretty(o));
        LOGI("VR menu: modifier + %s = %s", VrPad::BtnNames()[pickBtn_], Pretty(o).c_str());
    } else {
        pad_.map[pickBtn_] = o;
        status_ = std::string("Set: ") + BtnLabel(pickBtn_) + " = " + Pretty(o);
        LOGI("VR menu: %s = %s", VrPad::BtnNames()[pickBtn_], Pretty(o).c_str());
    }
    pickBtn_ = -1;
    for (bool& l : latch_) l = false;
    Save();
}

void VrMenu::SetValue(int arg) {
    auto mul = [](float& v, bool up, float lo, float hi) { v = up ? v * 1.15f : v / 1.15f; v = v < lo ? lo : v > hi ? hi : v; };
    if (arg == kResetArg) {                             // the first press arms, the second resets this tab
        if (resetArm_ != tab_) { resetArm_ = tab_; status_ = "Press again: this tab goes back to the defaults."; return; }
        resetArm_ = -1;
        ResetTab();
        status_ = "Back to the defaults. Saved.";
        return;
    }
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
    case 130: case 131: case 132: pad_.steerStick = arg == 130 ? 1 : arg == 131 ? 2 : 0; break;
    case 133: case 134: {
        const float d = pad_.steerDeadzone + (arg == 134 ? 0.05f : -0.05f);
        pad_.steerDeadzone = d < 0.0f ? 0.0f : d > 0.5f ? 0.5f : (float)(int)(d * 20.0f + 0.5f) / 20.0f;
        break;
    }
    case 135: case 136: {
        const float k = pad_.steerCurve + (arg == 136 ? 0.25f : -0.25f);
        pad_.steerCurve = k < 1.0f ? 1.0f : k > 3.0f ? 3.0f : k;
        break;
    }
    case 137: case 138: {
        const float m = pad_.steerMax + (arg == 138 ? 0.1f : -0.1f);
        pad_.steerMax = m < 0.2f ? 0.2f : m > 1.0f ? 1.0f : (float)(int)(m * 10.0f + 0.5f) / 10.0f;
        break;
    }
    case 141: case 142: pad_.steerMode = arg == 141 ? 1 : 0; break;
    case 147: case 148: pad_.drive = arg == 147; break;
    case 143: case 144: case 145: case 146: {          // rate: full-turn time / back-to-the-middle time, in steps (seconds)
        static const float kTimes[] = { 0.5f, 1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f, 7.0f, 10.0f, 15.0f, 20.0f };
        constexpr int kLast = (int)(sizeof(kTimes) / sizeof(kTimes[0])) - 1;
        float& v = arg < 145 ? pad_.steerTurnTime : pad_.steerReturnTime;
        int i = 0;
        while (i < kLast && kTimes[i] < v) ++i;
        i += (arg & 1) ? -1 : 1;                        // (143 / 145 = -: quicker, 144 / 146 = +: slower)
        v = kTimes[i < 0 ? 0 : i > kLast ? kLast : i];
        break;
    }
    case 139: case 140: {                              // speed: seconds centre -> full lock in steps (0 = at once at the top)
        static const float kSpeeds[] = { 0.25f, 0.35f, 0.5f, 0.75f, 1.0f, 1.5f, 2.0f, 3.0f, 5.0f, 0.0f };
        constexpr int kLast = (int)(sizeof(kSpeeds) / sizeof(kSpeeds[0])) - 1;
        int i = 0;
        while (i < kLast && kSpeeds[i] < pad_.steerSpeed) ++i;   // (the first step at or above the current speed)
        if (pad_.steerSpeed <= 0.0f) i = kLast;
        i += arg == 140 ? 1 : -1;
        pad_.steerSpeed = kSpeeds[i < 0 ? 0 : i > kLast ? kLast : i];
        break;
    }
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
    case 120: case 121: layer_ = arg - 120; pickBtn_ = -1; status_.clear(); return;
    case 122: {                                         // the modifier button: the next of these (off at the end)
        static const int kChoices[] = { VrPad::kRGrip, VrPad::kLGrip, VrPad::kA, VrPad::kB, VrPad::kX, VrPad::kY, VrPad::kRStick,
                                        VrPad::kLStick, VrPad::kMenu, -1 };
        constexpr int n = (int)(sizeof(kChoices) / sizeof(kChoices[0]));
        int i = 0;
        while (i < n && kChoices[i] != pad_.modBtn) ++i;
        pad_.modBtn = kChoices[(i + 1) % n];
        pickBtn_ = -1;
        break;
    }
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

// the Reset button of the current tab: its settings as a fresh install has them (config.ini as shipped = these defaults)
void VrMenu::ResetTab() {
    const VrPad::Config d = VrPad::DefaultConfig();
    switch (tab_) {
    case 0:                                             // every button, both layers, the modifier button
        for (int b = 0; b < VrPad::kBtnCount; ++b) { pad_.map[b] = d.map[b]; pad_.modMap[b] = d.modMap[b]; }
        pad_.modBtn = d.modBtn;
        pickBtn_ = -1;
        break;
    case 1:
        pad_.mouseStick = d.mouseStick; pad_.mouseSpeed = d.mouseSpeed; pad_.deadzone = d.deadzone;
        pad_.airBtn = d.airBtn; pad_.airHand = d.airHand; pad_.airToggle = d.airToggle; pad_.airGain = d.airGain;
        break;
    case 2: {                                           // default sizes, the full keyboard, both in front of the head
        SetKbCompact(false);
        pan_[kPanMenu].width = kMenuWidth;
        pan_[kPanKb].width = kKbWidth;
        for (int h = 0; h < 2; ++h) grab_[h] = Grab();
        Place(kPanMenu, lastHeadOk_, lastHead_, true);
        if (pan_[kPanKb].show) Place(kPanKb, lastHeadOk_, lastHead_, true);
        SavePlaces();
        break;
    }
    case 3: {
        const MenuLook l;
        look_.theme = l.theme; look_.menuOpacity = l.menuOpacity; look_.kbOpacity = l.kbOpacity; look_.smoothing = l.smoothing;
        needRedraw_ = true;
        break;
    }
    default:
        pad_.drive = d.drive;
        pad_.steerStick = d.steerStick; pad_.steerDeadzone = d.steerDeadzone; pad_.steerCurve = d.steerCurve;
        pad_.steerMax = d.steerMax; pad_.steerSpeed = d.steerSpeed;
        pad_.steerMode = d.steerMode; pad_.steerTurnTime = d.steerTurnTime; pad_.steerReturnTime = d.steerReturnTime;
        break;
    }
    LOGI("VR menu: tab %d reset to the defaults", tab_);
    Save();
}

void VrMenu::Activate(int h, const Ref& r, const VrPad::Frame&, VrPad::Sink& sink) {
    if (!(r.kind == eSet && r.arg == kResetArg)) resetArm_ = -1;   // (anything else pressed: Reset is not armed any more)
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
        if (layer_ && pad_.modBtn < 0) { status_ = "Choose a modifier button first (the button above)."; break; }
        if (layer_ && r.arg == pad_.modBtn) { status_ = std::string(BtnLabel(r.arg)) + " is the modifier."; break; }
        if (r.arg == pad_.airBtn) { status_ = std::string(BtnLabel(r.arg)) + " is the air mouse button (Mouse tab)."; break; }
        if (r.arg >= VrPad::kLUp && pad_.mouseStick == (r.arg >= VrPad::kRUp ? 2 : 1)) {
            status_ = "That stick moves the mouse (Mouse tab).";
            break;
        }
        if (DrivingUses(pad_, r.arg)) { status_ = "That stick steers (Driving tab)."; break; }
        pickBtn_ = pickBtn_ == r.arg && pickMod_ == (layer_ == 1) ? -1 : r.arg;
        pickMod_ = layer_ == 1;
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
        own_[0] = own_[1] = false;
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
            if (hold && aimOk_[h] && pan_[heldPan].show)       // the panel hangs on the controller: it follows every turn,
                pan_[heldPan].pose = PMul(aim_[h], *hold);        //   tilt and twist of the hand (kept level, a twist did nothing)
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
            if (down && !trigDown_[h] && r.pan >= 0) { own_[h] = true; Activate(h, r, f, sink); }
            if (!down) own_[h] = false;
            if (!down && press_[h].pan >= 0) Release(h, sink);
            if (press_[h].pan >= 0 && !pan_[press_[h].pan].show) Release(h, sink);   // (its panel was closed by that press)
            trigDown_[h] = down;
            if (hov_[h].pan >= 0 || press_[h].pan >= 0 || own_[h]) f.trig[h] = 0.0f;   // this trigger belongs to the menu now
        }
    }
    for (int p = 0; p < kPanCount; ++p)                 // (closed: nothing drawn)
        if (pan_[p].show && (needRedraw_ || ContentKey(p) != content_[p] || !DrawChanged(p))) Redraw(p);
    needRedraw_ = false;
    if (saveDue_) {                                     // config.ini: written on the painter's thread (no file I/O in a frame)
        saveDue_ = false;
        Painter::Job j;
        j.save = true;
        j.path = path_;
        j.kv = MenuValues(pad_, place_[kPanMenu], place_[kPanKb], look_);
        painter_->Post(std::move(j));
    }
    if (painter_->TakeSaveFailed()) status_ = "Could not save config.ini (the change works until the game ends).";
}

std::string VrMenu::ContentKey(int p) const {
    char b[200];
    if (p == kPanKb) {                                  // the keyboard: its colours, layout, latched modifiers, the pick frame
        snprintf(b, sizeof(b), "%d|%.2f|%d|%d|%d%d%d", look_.theme, look_.kbOpacity, look_.kbCompact, pickBtn_ >= 0, latch_[0],
                 latch_[1], latch_[2]);
        return b;
    }
    // the menu: everything its tabs show (not whether it is shown itself)
    snprintf(b, sizeof(b), "%d|%d|%d|%d%d|%.3f,%.3f|%d|%.2f|%.2f|%d|%d|", pan_[1].show, tab_, pickBtn_, pickMod_, layer_, pan_[0].width,
             pan_[1].width, look_.theme, look_.menuOpacity, look_.kbOpacity, look_.smoothing, look_.kbCompact);
    return std::string(b) + std::to_string(resetArm_) + "|" + status_ + "|" + DescribePad(pad_);
}

}  // namespace ck
