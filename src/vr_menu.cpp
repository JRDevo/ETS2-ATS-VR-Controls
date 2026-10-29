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
#include "kb_layout.h"
#include "lang.h"
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

// ---- the keyboard (units of one key; x 0..15 main block, 15.5..18.5 navigation, 19..23 number pad). geo: the key shapes
// (kb_layout.h Geo) that have this key here -- a key that sits elsewhere in another shape has one entry per place. part: the
// lower half of the tall ISO / JIS Enter (no label; pressed and lit together with the upper half, the entry before it).
// label: the English name; a character key's label comes from the layout (KeyLabel), a named key's is translated (N_).
enum : uint8_t { gA = 1, gI = 2, gB = 4, gJ = 8, gAll = 15, gAIB = 7, gAJ = 9, gIB = 6, gIBJ = 14, gBJ = 12 };
struct KeyDef { const char* label; const char* name; float x, y, w, h; uint8_t geo = gAll; bool part = false; };
const KeyDef kKeys[] = {
    { N_("Esc"), "esc", 0, 0, 1, 1 },
    { "F1", "f1", 2, 0, 1, 1 }, { "F2", "f2", 3, 0, 1, 1 }, { "F3", "f3", 4, 0, 1, 1 }, { "F4", "f4", 5, 0, 1, 1 },
    { "F5", "f5", 6.5f, 0, 1, 1 }, { "F6", "f6", 7.5f, 0, 1, 1 }, { "F7", "f7", 8.5f, 0, 1, 1 }, { "F8", "f8", 9.5f, 0, 1, 1 },
    { "F9", "f9", 11, 0, 1, 1 }, { "F10", "f10", 12, 0, 1, 1 }, { "F11", "f11", 13, 0, 1, 1 }, { "F12", "f12", 14, 0, 1, 1 },
    { N_("PrtSc"), "printscreen", 15.5f, 0, 1, 1 }, { N_("ScrLk"), "scrolllock", 16.5f, 0, 1, 1 },
    { N_("Pause"), "pause", 17.5f, 0, 1, 1 },
    { "`", "`", 0, 1.25f, 1, 1 }, { "1", "1", 1, 1.25f, 1, 1 }, { "2", "2", 2, 1.25f, 1, 1 }, { "3", "3", 3, 1.25f, 1, 1 },
    { "4", "4", 4, 1.25f, 1, 1 }, { "5", "5", 5, 1.25f, 1, 1 }, { "6", "6", 6, 1.25f, 1, 1 }, { "7", "7", 7, 1.25f, 1, 1 },
    { "8", "8", 8, 1.25f, 1, 1 }, { "9", "9", 9, 1.25f, 1, 1 }, { "0", "0", 10, 1.25f, 1, 1 }, { "-", "-", 11, 1.25f, 1, 1 },
    { "=", "=", 12, 1.25f, 1, 1 }, { N_("Backspace"), "backspace", 13, 1.25f, 2, 1, gAIB },
    { "\xC2\xA5", "intl3", 13, 1.25f, 1, 1, gJ }, { N_("Backspace"), "backspace", 14, 1.25f, 1, 1, gJ },
    { N_("Ins"), "insert", 15.5f, 1.25f, 1, 1 }, { N_("Home"), "home", 16.5f, 1.25f, 1, 1 },
    { N_("PgUp"), "pageup", 17.5f, 1.25f, 1, 1 },
    { "/", "numdiv", 20, 1.25f, 1, 1 }, { "*", "nummul", 21, 1.25f, 1, 1 }, { "-", "numminus", 22, 1.25f, 1, 1 },
    { N_("Tab"), "tab", 0, 2.25f, 1.5f, 1 }, { "Q", "q", 1.5f, 2.25f, 1, 1 }, { "W", "w", 2.5f, 2.25f, 1, 1 },
    { "E", "e", 3.5f, 2.25f, 1, 1 }, { "R", "r", 4.5f, 2.25f, 1, 1 }, { "T", "t", 5.5f, 2.25f, 1, 1 },
    { "Y", "y", 6.5f, 2.25f, 1, 1 }, { "U", "u", 7.5f, 2.25f, 1, 1 }, { "I", "i", 8.5f, 2.25f, 1, 1 },
    { "O", "o", 9.5f, 2.25f, 1, 1 }, { "P", "p", 10.5f, 2.25f, 1, 1 }, { "[", "[", 11.5f, 2.25f, 1, 1 },
    { "]", "]", 12.5f, 2.25f, 1, 1 }, { "\\", "\\", 13.5f, 2.25f, 1.5f, 1, gA },
    { N_("Enter"), "enter", 13.5f, 2.25f, 1.5f, 1, gIBJ }, { "", "enter", 13.75f, 3.0f, 1.25f, 1.25f, gIBJ, true },
    { N_("Del"), "delete", 15.5f, 2.25f, 1, 1 }, { N_("End"), "end", 16.5f, 2.25f, 1, 1 },
    { N_("PgDn"), "pagedown", 17.5f, 2.25f, 1, 1 },
    { "7", "num7", 19, 2.25f, 1, 1 }, { "8", "num8", 20, 2.25f, 1, 1 }, { "9", "num9", 21, 2.25f, 1, 1 },
    { "+", "numplus", 22, 2.25f, 1, 2 },
    { N_("Caps"), "capslock", 0, 3.25f, 1.75f, 1 }, { "A", "a", 1.75f, 3.25f, 1, 1 }, { "S", "s", 2.75f, 3.25f, 1, 1 },
    { "D", "d", 3.75f, 3.25f, 1, 1 }, { "F", "f", 4.75f, 3.25f, 1, 1 }, { "G", "g", 5.75f, 3.25f, 1, 1 },
    { "H", "h", 6.75f, 3.25f, 1, 1 }, { "J", "j", 7.75f, 3.25f, 1, 1 }, { "K", "k", 8.75f, 3.25f, 1, 1 },
    { "L", "l", 9.75f, 3.25f, 1, 1 }, { ";", ";", 10.75f, 3.25f, 1, 1 }, { "'", "'", 11.75f, 3.25f, 1, 1 },
    { N_("Enter"), "enter", 12.75f, 3.25f, 2.25f, 1, gA }, { "\\", "\\", 12.75f, 3.25f, 1, 1, gIBJ },
    { "4", "num4", 19, 3.25f, 1, 1 }, { "5", "num5", 20, 3.25f, 1, 1 }, { "6", "num6", 21, 3.25f, 1, 1 },
    { N_("Shift"), "shift", 0, 4.25f, 2.25f, 1, gAJ }, { N_("Shift"), "shift", 0, 4.25f, 1.25f, 1, gIB },
    { "\\", "oem102", 1.25f, 4.25f, 1, 1, gIB }, { "Z", "z", 2.25f, 4.25f, 1, 1 }, { "X", "x", 3.25f, 4.25f, 1, 1 },
    { "C", "c", 4.25f, 4.25f, 1, 1 }, { "V", "v", 5.25f, 4.25f, 1, 1 }, { "B", "b", 6.25f, 4.25f, 1, 1 },
    { "N", "n", 7.25f, 4.25f, 1, 1 }, { "M", "m", 8.25f, 4.25f, 1, 1 }, { ",", ",", 9.25f, 4.25f, 1, 1 },
    { ".", ".", 10.25f, 4.25f, 1, 1 }, { "/", "/", 11.25f, 4.25f, 1, 1 },
    { N_("Shift"), "rshift", 12.25f, 4.25f, 2.75f, 1, gA | gI }, { "\\", "intl1", 12.25f, 4.25f, 1, 1, gBJ },
    { N_("Shift"), "rshift", 13.25f, 4.25f, 1.75f, 1, gBJ },
    { "\xE2\x86\x91", "up", 16.5f, 4.25f, 1, 1 },
    { "1", "num1", 19, 4.25f, 1, 1 }, { "2", "num2", 20, 4.25f, 1, 1 }, { "3", "num3", 21, 4.25f, 1, 1 },
    { N_("Enter"), "numenter", 22, 4.25f, 1, 2 },
    { N_("Ctrl"), "ctrl", 0, 5.25f, 1.5f, 1 }, { N_("Alt"), "alt", 1.5f, 5.25f, 1.5f, 1 },
    { N_("Space"), "space", 3, 5.25f, 9, 1, gAIB },
    { "\xE7\x84\xA1\xE5\xA4\x89\xE6\x8F\x9B", "muhenkan", 3, 5.25f, 1.25f, 1, gJ }, { N_("Space"), "space", 4.25f, 5.25f, 4.5f, 1, gJ },
    { "\xE5\xA4\x89\xE6\x8F\x9B", "henkan", 8.75f, 5.25f, 1.5f, 1, gJ }, { "\xE3\x81\x8B\xE3\x81\xAA", "kana", 10.25f, 5.25f, 1.75f, 1, gJ },
    { N_("Alt"), "ralt", 12, 5.25f, 1.5f, 1 }, { N_("Ctrl"), "rctrl", 13.5f, 5.25f, 1.5f, 1 },
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

// a controller button's name in the menu's language (X / Y / A / B: as printed on the controller)
const char* BtnLabel(int b) {
    static const char* const k[VrPad::kBtnCount] = { N_("Left trigger"), N_("Right trigger"), N_("Left grip"), N_("Right grip"),
        "X", "Y", "A", "B", N_("Left stick click"), N_("Right stick click"), N_("Left menu"), N_("Left stick up"),
        N_("Left stick down"), N_("Left stick left"), N_("Left stick right"), N_("Right stick up"), N_("Right stick down"),
        N_("Right stick left"), N_("Right stick right") };
    return b >= 0 && b < VrPad::kBtnCount ? Lang::Tr(k[b]) : "?";
}
// a keyboard key's label in layout `kb`: main (big) + small (the second legend, "" = none). Character keys: as that country's
// keyboards have them (kb_layout.h); named keys (Esc, Enter ...): translated.
void KeyLabel(const KeyDef& d, const std::string& kb, std::string* main, std::string* small) {
    if (!d.part && KbLayout::Label(kb, VrPad::ScanOfName(d.name), main, small)) return;
    *main = Lang::Tr(d.label);
    small->clear();
}
// "shift+f" -> "Shift + F", "mouse_left" -> "Mouse left", "none" -> "-" (in the menu's language and keyboard layout `kb`)
std::string Pretty(const VrPad::Out& o, const std::string& kb) {
    static const char* const kKinds[] = { "-", "", N_("Mouse left"), N_("Mouse right"), N_("Mouse middle"), N_("Wheel up"),
                                          N_("Wheel down"), N_("Gas"), N_("Brake"), N_("Keyboard") };
    if (o.kind != VrPad::kKeys) return o.kind < sizeof(kKinds) / sizeof(kKinds[0]) ? Lang::Tr(kKinds[o.kind]) : "?";
    char raw[64];
    VrPad::OutToString(o, raw, sizeof(raw));
    std::string out, part, small;
    for (const char* p = raw;; ++p) {
        if ((*p == '+' && !part.empty()) || !*p) {
            for (const KeyDef& k : kKeys)
                if (part == k.name && !k.part) {
                    std::string m;
                    KeyLabel(k, kb, &m, &small);
                    char b[64];
                    if (k.name[0] == 'n' && k.name[1] == 'u') snprintf(b, sizeof(b), TRF("Num %s"), m.c_str());
                    part = k.name[0] == 'n' && k.name[1] == 'u' ? std::string(b) : m;
                    break;
                }
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
// a label's characters (UTF-8 code points), for "one big character" keys
int CodePoints(const std::string& s) { int n = 0; for (unsigned char c : s) n += (c & 0xC0) != 0x80; return n; }

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
        if (dc_) SelectObject(dc_, GetStockObject(SYSTEM_FONT));   // (a font still selected would not be deleted)
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
    // the GDI face of the texts from now on (the menu's language / the keyboard layout: lang.h); a new face drops the fonts made
    void SetFace(const std::wstring& face) {
        if (face.empty() || face == face_) return;
        face_ = face;
        // (a font still selected would not be deleted; a stock font, never old_: that is the bitmap, and the picture would freeze)
        SelectObject(dc_, GetStockObject(SYSTEM_FONT));
        for (int i = 0; i < 2; ++i) { for (auto& f : fonts_[i]) if (f.second) DeleteObject(f.second); fonts_[i].clear(); }
    }
    // a smaller font (down to 60 %) when the text is wider than the box minus 12 px, before "..." is used (a translation may be
    // longer than the English: every text shrinks to fit first). Still too wide: "..." + the text goes to Clipped() (tests).
    void Text(int x, int y, int w, int h, const std::string& s, int px, bool bold, COLORREF c, UINT align) {
        if (s.empty()) return;
        const std::wstring wt = Wide(s);
        SIZE sz{};
        for (int min = px * 6 / 10;; px -= 2) {
            SelectObject(dc_, Font(px, bold));
            if (!GetTextExtentPoint32W(dc_, wt.c_str(), (int)wt.size(), &sz) || sz.cx <= w - 12 || px <= min) break;
        }
        if (sz.cx > w) NoteClipped(s);
        SetTextColor(dc_, c);
        RECT r{ x, y, x + w, y + h };
        DrawTextW(dc_, wt.c_str(), (int)wt.size(), &r, align | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    }
    // the text fits one line of the box at the smallest size Text() shrinks it to
    bool FitsLine(int w, const std::string& s, int px, bool bold) {
        const std::wstring wt = Wide(s);
        SIZE sz{};
        SelectObject(dc_, Font(px * 6 / 10, bold));
        return GetTextExtentPoint32W(dc_, wt.c_str(), (int)wt.size(), &sz) && sz.cx <= w - 12;
    }
    static std::vector<std::string> TakeClipped() {
        std::lock_guard<std::mutex> lk(clipM_);
        std::vector<std::string> v;
        v.swap(clipped_);
        return v;
    }
    // up to two lines, word-wrapped and centred in the box (vertically; center: also across); a smaller font (down to 70 %)
    // when it does not fit
    void TextWrap(int x, int y, int w, int h, const std::string& s, int px, COLORREF c, bool bold = false, bool center = false) {
        if (s.empty()) return;
        const std::wstring ws = Wide(s);
        RECT m{};
        for (int min = px * 7 / 10;; px -= 1) {
            SelectObject(dc_, Font(px, bold));
            m = RECT{ 0, 0, w, 0 };
            DrawTextW(dc_, ws.c_str(), (int)ws.size(), &m, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
            if (m.bottom <= h || px <= min) break;
        }
        if (m.bottom > h || m.right > w) NoteClipped(s);   // (more lines, or one word wider than the box)
        SetTextColor(dc_, c);
        const int top = y + (h - (m.bottom < h ? m.bottom : h)) / 2;
        RECT r{ x, top, x + w, y + h };
        DrawTextW(dc_, ws.c_str(), (int)ws.size(), &r, DT_WORDBREAK | DT_NOPREFIX | DT_END_ELLIPSIS | (center ? DT_CENTER : 0));
    }
    // a padlock in colour c, centred in the box, smooth edges (4 x 4 samples a pixel; GDI has no anti-aliased shapes); open =
    // the shackle lifted with its right leg out of the body. Drawn on a 24-unit grid (a common icon's proportions).
    void Padlock(int x, int y, int w, int h, bool closed, COLORREF c) {
        GdiFlush();
        const double ux = 0.6 * w / 16.0, uy = 0.62 * h / 23.0, u = ux < uy ? ux : uy;   // body 16 units wide, all 23 tall
        const double ox = x + w * 0.5 - 12.0 * u, oy = y + h * 0.5 - 10.5 * u;            // grid x 4..20, y -1..22
        const double lift = closed ? 0.0 : 3.0, cy = 7.0 - lift;                          // the shackle's arc centre
        auto inside = [&](double gx, double gy) {
            if (gx >= 4 && gx <= 20 && gy >= 10 && gy <= 22) {                            // the body (corners r 2) ...
                const double dx = gx < 6 ? 6 - gx : gx > 18 ? gx - 18 : 0, dy = gy < 12 ? 12 - gy : gy > 20 ? gy - 20 : 0;
                if (dx * dx + dy * dy > 4) return false;
                const double kx = gx - 12, ky = gy - 15;                                  // ... less its keyhole
                return !(kx * kx + ky * ky < 3.2 || (std::fabs(kx) < 0.8 && ky > 0 && ky < 3.2));
            }
            const double ax = gx - 12, ay = gy - cy, r2 = ax * ax + ay * ay;
            if (ay <= 0) return r2 >= 9 && r2 <= 25;                                      // the arc (r 3 .. 5)
            if (gx >= 7 && gx <= 9) return gy <= 10.5;                                    // left leg
            if (gx >= 15 && gx <= 17) return gy <= (closed ? 10.5 : cy + 1.5);            // right leg (open: short)
            return false;
        };
        const int px0 = (int)(ox + 3 * u), px1 = (int)(ox + 21 * u) + 1, py0 = (int)(oy - 2 * u), py1 = (int)(oy + 23 * u) + 1;
        for (int py = py0 < 0 ? 0 : py0; py < py1 && py < h_; ++py)
            for (int px = px0 < 0 ? 0 : px0; px < px1 && px < w_; ++px) {
                int n = 0;
                for (int sy = 0; sy < 4; ++sy)
                    for (int sx = 0; sx < 4; ++sx) n += inside((px + (sx + 0.5) / 4 - ox) / u, (py + (sy + 0.5) / 4 - oy) / u);
                if (!n) continue;
                uint32_t& d = static_cast<uint32_t*>(bits_)[(size_t)py * w_ + px];         // (BGRA; COLORREF is RGB)
                const uint32_t src[3] = { GetBValue(c), GetGValue(c), GetRValue(c) };
                uint32_t o = 0;
                for (int k = 0; k < 3; ++k) {
                    const uint32_t b = (d >> (8 * k)) & 0xFFu;
                    o |= ((b * (16 - n) + src[k] * n + 8) / 16) << (8 * k);
                }
                d = o;
            }
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
                              CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, face_.c_str());
        fonts_[bold ? 1 : 0].push_back({ px, f });
        return f;
    }
    int w_, h_;
    HDC dc_ = nullptr;
    HBITMAP bmp_ = nullptr;
    HGDIOBJ old_ = nullptr;
    void* bits_ = nullptr;
    std::vector<std::pair<int, HFONT>> fonts_[2];
    std::wstring face_ = L"Segoe UI";
    static void NoteClipped(const std::string& s) { std::lock_guard<std::mutex> lk(clipM_); if (clipped_.size() < 200) clipped_.push_back(s); }
    static std::mutex clipM_;
    static std::vector<std::string> clipped_;
};
std::mutex MenuCanvas::clipM_;
std::vector<std::string> MenuCanvas::clipped_;

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
        std::wstring face;                                     // the GDI face of its texts (the language's: lang.h)
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
        c.SetFace(j.face);
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
    RefreshLang();                                      // the game's language (its files; not in a frame: the session starts)
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

int VrMenu::KeyIndex(const char* name) {               // (the US keyboard's entry when a key sits elsewhere in other shapes)
    int any = -1;
    for (int k = 0; k < kKeyCount; ++k)
        if (!strcmp(kKeys[k].name, name) && !kKeys[k].part) {
            if (kKeys[k].geo & gA) return k;
            if (any < 0) any = k;
        }
    return any;
}

std::vector<std::string> VrMenu::TakeClipped() { return MenuCanvas::TakeClipped(); }
bool VrMenu::FitsForTest(const std::string& s, int w, int h, int px, bool wrap) {
    MenuCanvas c(w + 32, h + 32);
    c.SetFace(Lang::Face());
    MenuCanvas::TakeClipped();
    if (wrap && px < 24) {                              // a pick button: one line when it fits, else two (as DrawElem)
        if (c.FitsLine(w, s, 24, true)) c.Text(0, 0, w, h, s, 24, true, 0, DT_CENTER);
        else c.TextWrap(3, 0, w - 6, h, s, px, 0, true, true);
    } else if (wrap) c.TextWrap(0, 0, w, h, s, px, 0);
    else c.Text(0, 0, w, h, s, px, true, 0, DT_CENTER);
    return MenuCanvas::TakeClipped().empty();
}

void VrMenu::RefreshLang() {
    Lang::SetSetting(look_.lang);
    Lang::Refresh();
    std::string kb = KbLayout::For(look_.kbLang == "auto" ? std::string(Lang::Wanted()) : look_.kbLang);
    static std::string s_faceOf;                        // (the font probe: only when the layout changes)
    static std::wstring s_face;
    if (kb != s_faceOf) {
        const wchar_t* f = Lang::FaceFor(kb, KbLayout::Chars(kb));
        s_faceOf = kb;
        s_face = f ? f : L"";
        if (!f) LOGW("VR menu: no font on this PC has the letters of the %s keyboard -- the US keyboard is shown", kb.c_str());
        else LOGI("VR menu: keyboard layout %s, font %ls", kb.c_str(), f);
    }
    std::wstring face = s_face;
    if (face.empty()) { kb = "en_us"; face = L"Segoe UI"; }
    if (kb != kbCode_ || face != kbFace_ || Lang::Gen() != langGen_) {
        kbCode_ = kb;
        kbFace_ = face;
        langGen_ = Lang::Gen();
        needRedraw_ = true;
    }
}

void VrMenu::CycleLang(bool kb, int step) {
    std::string& cur = kb ? look_.kbLang : look_.lang;
    const int n = kb ? 1 + KbLayout::Count() : Lang::Count();
    auto at = [&](int i) { return kb ? (i == 0 ? std::string("auto") : std::string(KbLayout::CodeAt(i - 1))) : std::string(Lang::CodeAt(i)); };
    int i = 0;
    while (i < n && at(i) != cur) ++i;
    i = i >= n ? 0 : (i + step + n) % n;
    cur = at(i);
    RefreshLang();
    LOGI("VR menu: %s = %s", kb ? "keyboard_language" : "language", cur.c_str());
}

void VrMenu::SetLanguage(const std::string& lang, const std::string& kbLang) {
    look_.lang = lang;
    look_.kbLang = kbLang;
    RefreshLang();
    needRedraw_ = true;
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
    RefreshLang();                                      // (the player may have changed the game's language meanwhile)
    Place(kPanMenu, headOk, head, false);
    pan_[kPanMenu].show = true;
    status_.clear();                                    // (redrawn in Update only if what it shows changed meanwhile)
    LOGI("VR menu open (%s)", Describe().c_str());
}

void VrMenu::ShowKb(bool on) {
    if (on && !pan_[kPanKb].show) { RefreshLang(); Place(kPanKb, lastHeadOk_, lastHead_, false); }
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
    resetArm_ = -1;                                     // ("Sure? Reset" does not wait for the next open)
    for (bool& l : latch_) l = false;
    LOGI("VR menu closed");
}

// steering (pad_map.h SteerStickOf) takes that stick's left / right directions
// the modifier buttons a layer holds, as a label: "Right grip", "Left grip", "Right grip + Left grip" ("" for 0)
static std::string LayerKeys(const VrPad::Config& p, int layer) {
    std::string s;
    if ((layer & 1) && p.modBtn >= 0) s += BtnLabel(p.modBtn);
    if ((layer & 2) && p.modBtn2 >= 0) s += (s.empty() ? "" : " + ") + std::string(BtnLabel(p.modBtn2));
    return s;
}
// in this layer, button btn + the layer's modifiers = all the menu's open buttons: the menu takes them first (ComboGate runs
// before the Mapper), so a key set there could never be sent
static bool MenuOnly(const VrPad::Config& p, const std::vector<int>& combo, int layer, int btn) {
    bool in = false, all = layer != 0;
    for (int cb : combo) {
        in = in || cb == btn;
        all = all && (cb == btn || ((layer & 1) && cb == p.modBtn) || ((layer & 2) && cb == p.modBtn2));
    }
    return in && all;
}
// the layer needs a grip that holds the wheel (steer with the wheel): those grips do nothing else, so the layer is out
static bool LayerOnWheel(const VrPad::Config& p, int layer) {
    auto g = [](int m) { return m == VrPad::kLGrip ? 1 : m == VrPad::kRGrip ? 2 : 0; };
    const int need = ((layer & 1) ? g(p.modBtn) : 0) | ((layer & 2) ? g(p.modBtn2) : 0);
    if (need & VrPad::WheelGripMask(p)) return true;
    return p.drive && p.steerWith == 1 && p.steerWheelGrip == 0 && need == 3;   // (both grips together = the wheel)
}
// the layer's modifiers are all set (layer 3: both)
static bool LayerReady(const VrPad::Config& p, int layer) {
    return layer == 0 || (((layer & 1) == 0 || p.modBtn >= 0) && ((layer & 2) == 0 || p.modBtn2 >= 0));
}

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
    // the help of everything added since `from` (a row: its label, buttons and value)
    auto tip = [&](size_t from, const std::string& t) { for (size_t i = from; i < el.size(); ++i) el[i].help = t; };
    char b[96], hb[400];
    // a translated printf format filled in (a text of the menu's language with values in it)
    auto fmt = [&](const char* f, auto... a) { snprintf(hb, sizeof(hb), f, a...); return std::string(hb); };
    size_t m0 = 0;
    std::string openCombo;                              // the menu's open / close buttons ("Left grip + Right grip + ...")
    for (int cb : gate_.btns) openCombo += (openCombo.empty() ? "" : " + ") + std::string(BtnLabel(cb));
    const bool picking = pickBtn_ >= 0;
    const std::string what = picking ? (pickLayer_ ? LayerKeys(pad_, pickLayer_) + " + " : std::string()) + BtnLabel(pickBtn_)
                                     : std::string();
    add(eBar, 0, 0, 0, kMenuW, 72, picking ? fmt(TRF("Set %s: pick a key"), what.c_str()) : "ETS2 ATS VR Controls", picking);
    m0 = el.size(); add(eSmaller, 0, 600, 11, 64, 50, "\xE2\x88\x92"); tip(m0, TR("Make this menu smaller."));
    m0 = el.size(); add(eBigger, 0, 672, 11, 64, 50, "+"); tip(m0, TR("Make this menu bigger."));
    m0 = el.size(); add(eKbBtn, 0, 744, 11, 170, 50, TR("Keyboard"), pan_[kPanKb].show); tip(m0, TR("Show or hide the VR keyboard."));
    m0 = el.size(); add(eClose, 0, 924, 11, 64, 50, "\xC3\x97");
    tip(m0, fmt(TRF("Close this menu. To open it again: hold %s."), openCombo.c_str()));
    static const char* const kTabs[5] = { N_("Buttons"), N_("Mouse"), N_("Panels"), N_("Look"), N_("Driving") };
    static const char* const kTabHelp[5] = { N_("Choose what each controller button does."),
                                             N_("The stick mouse and the air mouse (point with the controller)."),
                                             N_("Size and place of this menu and the keyboard."),
                                             N_("Colours, how see-through the panels are, pointer smoothing and the language."),
                                             N_("Steering, gas and brake: how the truck's wheel follows your stick.") };
    for (int t = 0; t < 5; ++t) {
        m0 = el.size();
        add(eTab, t, 24 + t * 193, 84, 181, 56, Lang::Tr(kTabs[t]), tab_ == t);
        tip(m0, Lang::Tr(kTabHelp[t]));
    }
    // the language's code (EN, DE, KO ...) in the top bar = the Look tab: a way back to the Language row from a menu in a
    // language one cannot read (after the real tabs: a search for the Look tab finds that one first)
    const bool english = !strncmp(Lang::Active(), "en", 2);
    if (!picking) {
        std::string code(Lang::Active(), 2);
        for (char& ch : code) ch = (char)toupper((unsigned char)ch);
        m0 = el.size(); add(eTab, 3, 516, 11, 74, 50, code, false);
        tip(m0, english ? std::string(TR("Language")) : std::string(TR("Language")) + " (Language)");
    }
    if (tab_ == 0) {
        static const int kLeft[10] = { VrPad::kLTrig, VrPad::kLGrip, VrPad::kX, VrPad::kY, VrPad::kLStick, VrPad::kMenu,
                                       VrPad::kLUp, VrPad::kLDown, VrPad::kLLeft, VrPad::kLRight };
        static const int kRight[9] = { VrPad::kRTrig, VrPad::kRGrip, VrPad::kA, VrPad::kB, VrPad::kRStick, VrPad::kRUp,
                                       VrPad::kRDown, VrPad::kRLeft, VrPad::kRRight };
        // the layer the rows show and set (Normal / Mod 1 / Mod 2 / Both = no modifier, modifier 1, modifier 2, both held), and
        // the two modifier buttons (a click cycles each). Chip args: 120, 121, 123, 124 = layer 0 .. 3; 122 / 125 = the choosers.
        static const int kChipArg[4] = { 120, 121, 123, 124 };
        static const char* const kChip[4] = { N_("Normal"), N_("Mod 1"), N_("Mod 2"), N_("Both") };
        const std::string m1 = pad_.modBtn >= 0 ? BtnLabel(pad_.modBtn) : TR("off"),
                          m2 = pad_.modBtn2 >= 0 ? BtnLabel(pad_.modBtn2) : TR("off");
        const std::string kChipHelp[4] = { TR("Show and set what each button does normally."),
                                           fmt(TRF("Show and set what each button does while you hold modifier 1 (%s)."), m1.c_str()),
                                           fmt(TRF("Show and set what each button does while you hold modifier 2 (%s)."), m2.c_str()),
                                           TR("Show and set what each button does while you hold BOTH modifiers.") };
        for (int l = 0; l < 4; ++l) {
            m0 = el.size();
            add(eSet, kChipArg[l], 24 + l * 146, 150, 138, 44, Lang::Tr(kChip[l]), layer_ == l,
                !LayerReady(pad_, l) || LayerOnWheel(pad_, l));
            tip(m0, kChipHelp[l]);
        }
        m0 = el.size(); add(eSet, 122, 612, 150, 178, 44, "1: " + m1);
        tip(m0, TR("Modifier 1: hold it, and other buttons do their Mod 1 job. Press here to choose another button."));
        m0 = el.size(); add(eSet, 125, 798, 150, 178, 44, "2: " + m2);
        tip(m0, TR("Modifier 2: hold it, and other buttons do their Mod 2 job. Hold both modifiers for the Both jobs. Press here "
                   "to choose another button."));
        const VrPad::Out* lmap = VrPad::LayerMap(pad_, layer_);
        const std::string lkeys = layer_ ? LayerKeys(pad_, layer_) : std::string();
        for (int col = 0; col < 2; ++col)
            for (int i = 0; i < (col ? 9 : 10); ++i) {
                const int btn = col ? kRight[i] : kLeft[i];
                std::string val = Pretty(lmap ? lmap[btn] : pad_.map[btn], kbCode_);
                bool dim = false;
                bool combo = false;                  // (one of the menu's open / close buttons)
                for (int cb : gate_.btns) combo = combo || cb == btn;
                const bool menuOnly = MenuOnly(pad_, gate_.btns, layer_, btn);
                const int wg = VrPad::WheelGripMask(pad_);
                if ((btn == VrPad::kLGrip && (wg & 1)) || (btn == VrPad::kRGrip && (wg & 2))) { val = TR("the wheel"); dim = true; }
                else if (LayerOnWheel(pad_, layer_)) { val = TR("(grips = the wheel)"); dim = true; }
                else if (layer_ && VrPad::IsModifier(pad_, btn)) { val = btn == pad_.modBtn ? TR("modifier 1") : TR("modifier 2"); dim = true; }
                else if (btn == pad_.airBtn) { val = TR("air mouse"); dim = true; }
                else if (btn >= VrPad::kLUp && pad_.mouseStick == (btn >= VrPad::kRUp ? 2 : 1)) { val = TR("moves the mouse"); dim = true; }
                else if (DrivingUses(pad_, btn)) { val = TR("steers"); dim = true; }
                else if (menuOnly) { val = TR("opens the menu"); dim = true; }
                else if (layer_ && (!LayerReady(pad_, layer_) || lmap[btn].kind == VrPad::kNone)) { val = TR("as normal"); dim = true; }
                add(eRow, btn, col ? 508 : 24, 202 + i * 49, 468, 45, BtnLabel(btn), pickBtn_ == btn, dim,
                    combo && !layer_ ? val + "  " + TR("(+menu)") : val);
                std::string h;
                if (dim) h = layer_ ? fmt(TRF("%s with %s: %s (set elsewhere)."), BtnLabel(btn), lkeys.c_str(), val.c_str())
                                    : fmt(TRF("%s: %s (set elsewhere)."), BtnLabel(btn), val.c_str());
                else h = layer_ ? fmt(TRF("%s with %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a "
                                          "choice below."), BtnLabel(btn), lkeys.c_str(), val.c_str())
                                : fmt(TRF("%s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice "
                                          "below."), BtnLabel(btn), val.c_str());
                if (combo && !layer_) h += std::string(" ") + TR("Also part of the menu's open buttons.");
                el.back().help = h;
            }
        if (picking) {
            static const int kOuts[9] = { VrPad::kMouseL, VrPad::kMouseR, VrPad::kMouseM, VrPad::kWheelUp, VrPad::kWheelDown,
                                          VrPad::kGas, VrPad::kBrake, VrPad::kKeyboard, VrPad::kNone };
            static const char* const kOutNames[9] = { N_("Mouse L"), N_("Mouse R"), N_("Mouse M"), N_("Wheel up"), N_("Wheel dn"),
                                                      N_("Gas"), N_("Brake"), N_("pick|Keyboard"), N_("None") };
            static const char* const kOutHelp[9] = { N_("Left mouse click."), N_("Right mouse click."), N_("Middle mouse click."),
                                                     N_("Mouse wheel up (one step per press)."),
                                                     N_("Mouse wheel down (one step per press)."),
                                                     N_("Gas pedal: press harder = more gas."), N_("Brake: press harder = more brake."),
                                                     N_("Shows / hides the VR keyboard."), N_("Nothing.") };
            for (int i = 0; i < 9; ++i) {
                add(ePickOut, kOuts[i], 24 + i * 92, 704, 86, 54, Lang::TrC(kOutNames[i]));   // ("pick|": a narrow button)
                el.back().help = Lang::Tr(kOutHelp[i]);
            }
            add(ePickCancel, 0, 852, 704, 124, 54, TR("Cancel"));
        } else {
            add(eInfo, 0, 24, 704, 742, 54, !status_.empty() ? status_
                : layer_ ? fmt(TRF("Hold %s first, then the button."), LayerKeys(pad_, layer_).c_str())
                         : std::string(TR("Point at a button, pull the trigger, pick its key.")));
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
        m0 = el.size(); row(0, TR("Stick that moves the mouse"));
        opts(0, 0, { TR("Left"), TR("Right"), TR("Off") }, pad_.mouseStick == 1 ? 0 : pad_.mouseStick == 2 ? 1 : 2);
        tip(m0, TR("Which stick moves the mouse pointer (for the game's menus and map). Off = no stick mouse."));
        m0 = el.size(); row(1, TR("Stick mouse speed"));
        snprintf(b, sizeof(b), TRF("%.0f px/s"), (double)pad_.mouseSpeed);
        plusMinus(1, 10, b);
        tip(m0, TR("How fast the pointer moves with the stick pushed all the way, in pixels per second."));
        m0 = el.size(); row(2, TR("Stick dead zone"));
        snprintf(b, sizeof(b), "%.2f", (double)pad_.deadzone);
        plusMinus(2, 20, b);
        tip(m0, TR("How far you must push the stick before the pointer moves. More = no creeping pointer."));
        m0 = el.size(); row(3, TR("Air mouse button"));
        opts(3, 30, { BtnLabel(VrPad::kRGrip), BtnLabel(VrPad::kLGrip), TR("Off") },
             pad_.airBtn == VrPad::kRGrip ? 0 : pad_.airBtn == VrPad::kLGrip ? 1 : pad_.airBtn < 0 ? 2 : -1);
        tip(m0, TR("Hold this button and turn the controller: the pointer follows, like a laser pointer. Off = no air mouse."));
        m0 = el.size(); row(4, TR("Air mouse works while"));
        opts(4, 40, { TR("Button held"), TR("Toggled on") }, pad_.airToggle ? 1 : 0);
        tip(m0, TR("Button held: only while you hold the button. Toggled on: one press switches it on, the next press off."));
        m0 = el.size(); row(5, TR("Air mouse speed"));
        snprintf(b, sizeof(b), TRF("%.0f px/deg"), (double)pad_.airGain);
        plusMinus(5, 50, b);
        tip(m0, TR("How far the pointer moves when you turn the controller, in pixels per degree."));
        add(eInfo, 0, 24, 704, 742, 54, status_.empty() ? TR("Changes work at once and are saved.") : status_);
    } else if (tab_ == 2) {
        auto row = [&](int r, const char* label) { add(eInfo, 100 + r, 24, 156 + r * 76, 440, 64, label); };
        auto plusMinus = [&](int r, int base, const std::string& value, const char* minus, const char* plus) {
            const bool glyph = !strcmp(minus, "\xE2\x88\x92");
            add(eSet, base, 470, 156 + r * 76, glyph ? 90 : 130, 64, minus);
            add(eInfo, 200 + r, glyph ? 570 : 610, 156 + r * 76, glyph ? 306 : 226, 64, value);
            add(eSet, base + 1, glyph ? 886 : 846, 156 + r * 76, glyph ? 90 : 130, 64, plus);
        };
        const V3 from = lastHeadOk_ ? lastHead_.p : V3{ 0, 0, 0 };
        m0 = el.size(); row(0, TR("Menu size"));
        snprintf(b, sizeof(b), TRF("%.2f m wide"), pan_[kPanMenu].width);
        plusMinus(0, 60, b, "\xE2\x88\x92", "+");
        tip(m0, TR("How big this menu is (its width in metres)."));
        m0 = el.size(); row(1, TR("Menu distance"));
        snprintf(b, sizeof(b), TRF("%.2f m"), Len(pan_[kPanMenu].pose.p - from));
        plusMinus(1, 62, b, TR("Closer"), TR("Farther"));
        tip(m0, TR("How far this menu is from your head."));
        m0 = el.size(); row(2, TR("Keyboard size"));
        snprintf(b, sizeof(b), TRF("%.2f m wide"), pan_[kPanKb].width);
        plusMinus(2, 70, b, "\xE2\x88\x92", "+");
        tip(m0, TR("How big the VR keyboard is (its width in metres)."));
        m0 = el.size(); row(3, TR("Keyboard distance"));
        if (pan_[kPanKb].show) snprintf(b, sizeof(b), TRF("%.2f m"), Len(pan_[kPanKb].pose.p - from));
        else snprintf(b, sizeof(b), "%s", TR("(hidden)"));
        plusMinus(3, 72, b, TR("Closer"), TR("Farther"));
        tip(m0, TR("How far the VR keyboard is from your head."));
        m0 = el.size(); row(4, TR("Keyboard"));
        add(eSet, 80, 470, 460, 119, 64, TR("Show"), pan_[kPanKb].show);
        add(eSet, 81, 599, 460, 119, 64, TR("Hide"), !pan_[kPanKb].show);
        add(eSet, 82, 738, 460, 114, 64, TR("Full"), !look_.kbCompact);
        add(eSet, 83, 862, 460, 114, 64, TR("Compact"), look_.kbCompact);
        tip(m0, TR("Show / hide the VR keyboard. Full = every key; Compact = only the main keys (narrower)."));
        m0 = el.size(); add(eHere, 0, 24, 540, 952, 64, TR("Bring the menu and the keyboard in front of me"));
        tip(m0, TR("Moves the menu and the keyboard to where you look now."));
        add(eInfo, 300, 24, 612, 952, 46, fmt(TRF("Menu open / close: hold %s for %.1f s"), openCombo.c_str(), gate_.hold));
        add(eInfo, 301, 24, 656, 952, 46, TR("Move: point at a panel, hold the grip, move. Its stick: farther / closer."));
        add(eInfo, 0, 24, 704, 742, 54, status_.empty() ? TR("The keyboard stays when the menu closes (its x hides it).") : status_);
    } else if (tab_ == 3) {
        auto row = [&](int y, int arg, const char* label) { add(eInfo, arg, 24, y, 440, 64, label); };
        auto arrows = [&](int y, int arg, int base, const std::string& value) {   // [<] value [>]
            add(eSet, base, 470, y, 90, 64, "\xE2\x80\xB9");
            add(eInfo, arg, 570, y, 306, 64, value);
            add(eSet, base + 1, 886, y, 90, 64, "\xE2\x80\xBA");
        };
        static const char* const kThemeLabel[kThemeCount] = { N_("Dark"), N_("Blue"), N_("Grey"), N_("Light"), N_("Black"), N_("Green") };
        m0 = el.size(); row(156, 100, TR("Colours"));
        for (int i = 0; i < kThemeCount; ++i)
            add(eSet, 100 + i, 470 + (i % 3) * 172, 156 + (i / 3) * 70, 162, 64, Lang::Tr(kThemeLabel[i]), look_.theme == i);
        tip(m0, TR("The colours of the menu and the keyboard."));
        m0 = el.size(); row(302, 102, TR("Menu see-through"));
        add(eSet, 90, 470, 302, 90, 64, "\xE2\x88\x92");
        snprintf(b, sizeof(b), TRF("%d %% solid"), (int)(look_.menuOpacity * 100.0f + 0.5f));
        add(eInfo, 202, 570, 302, 306, 64, b);
        add(eSet, 91, 886, 302, 90, 64, "+");
        tip(m0, TR("How solid this menu is. - = more see-through, + = more solid."));
        m0 = el.size(); row(378, 103, TR("Keyboard see-through"));
        add(eSet, 92, 470, 378, 90, 64, "\xE2\x88\x92");
        snprintf(b, sizeof(b), TRF("%d %% solid"), (int)(look_.kbOpacity * 100.0f + 0.5f));
        add(eInfo, 203, 570, 378, 306, 64, b);
        add(eSet, 93, 886, 378, 90, 64, "+");
        tip(m0, TR("How solid the VR keyboard is. - = more see-through, + = more solid."));
        m0 = el.size(); row(454, 104, TR("Pointer smoothing"));
        static const char* const kSm[4] = { N_("Off"), N_("Low"), N_("Medium"), N_("High") };
        for (int i = 0; i < 4; ++i) add(eSet, 110 + i, 470 + i * 129, 454, 119, 64, Lang::Tr(kSm[i]), look_.smoothing == i);
        tip(m0, TR("Calms a shaky hand: the menu pointer and the air mouse. Higher = steadier, but it follows a bit later."));
        // the language: auto (the game's) or one of them; the keyboard's letters: auto (as the menu) or one of them
        std::string name = Lang::FontMissing() ? fmt(TRF("%s (no font)"), Lang::Wanted()) : Lang::NativeName(Lang::Wanted());
        // (+ the English word in another language: findable by anyone who cannot read the menu's language)
        const std::string langLabel = english ? std::string(TR("Language")) : std::string(TR("Language")) + " (Language)";
        m0 = el.size(); row(530, 105, langLabel.c_str());
        arrows(530, 205, 170, look_.lang == "auto" ? fmt(TRF("Auto: %s"), name.c_str()) : name);
        tip(m0, TR("The menu's language. Auto = the language the game is set to."));
        name = Lang::NativeName(kbCode_);
        m0 = el.size(); row(606, 106, TR("Keyboard letters"));
        arrows(606, 206, 172, look_.kbLang == "auto" ? fmt(TRF("Auto: %s"), name.c_str()) : name);
        tip(m0, TR("The letters on the VR keyboard, as printed in that country. Auto = as the menu. A key always presses the same "
                   "key in the game."));
        add(eInfo, 0, 24, 704, 742, 54, status_.empty() ? TR("- = more see-through, + = more solid.") : status_);
    } else {                                            // Driving (scs_input.h): steering here, gas / brake on any button
        // wheel / point add a row (which grips / which hand): 10 rows, a little closer together
        const bool hands = pad_.drive && (pad_.steerWith == 1 || pad_.steerWith == 2);
        const int o = hands ? 1 : 0, pitch = hands ? 55 : 61, rh = hands ? 48 : 54;
        auto Y = [&](int r) { return 150 + r * pitch; };
        auto row = [&](int r, const char* label) { add(eInfo, 100 + r, 24, Y(r), 440, rh, label); };
        auto opts = [&](int r, int base, std::initializer_list<const char*> names, int cur) {
            const int n = (int)names.size(), w = (506 - (n - 1) * 10) / n;
            int i = 0;
            for (const char* nm : names) { add(eSet, base + i, 470 + i * (w + 10), Y(r), w, rh, nm, i == cur); ++i; }
        };
        auto plusMinus = [&](int r, int base, const std::string& value) {
            add(eSet, base, 470, Y(r), 90, rh, "\xE2\x88\x92");
            add(eInfo, 200 + r, 570, Y(r), 306, rh, value);
            add(eSet, base + 1, 886, Y(r), 90, rh, "+");
        };
        {                                               // steer with: Stick 155 / Wheel 156 / Point 157 / Off 148 (147 = on)
            m0 = el.size();
            row(0, TR("Steer with"));
            const int cur = !pad_.drive ? 3 : pad_.steerWith == 1 ? 1 : pad_.steerWith == 2 ? 2 : 0;
            static const int kArg[4] = { 155, 156, 157, 148 };
            static const char* const kName[4] = { N_("Stick"), N_("Wheel"), N_("Point"), N_("Off") };
            for (int i = 0; i < 4; ++i) add(eSet, kArg[i], 470 + i * 129, Y(0), 119, rh, Lang::Tr(kName[i]), i == cur);
            tip(m0, TR("Stick: the stick steers. Wheel: hold the grip(s) and turn your hands like a steering wheel. Point: point "
                       "a controller left / right (on a panel the wheel holds). Off: no driving."));
        }
        if (hands) {                                    // the hands: [Stick on / off] [-] angle [+], then grips / hand
            const bool wheel = pad_.steerWith == 1;
            m0 = el.size(); row(1, wheel ? TR("Hands turn for full lock") : TR("Point for full lock"));
            snprintf(b, sizeof(b), TRF("%.0f deg"), (double)(wheel ? pad_.steerWheelAngle : pad_.steerPointAngle));
            add(eSet, 158, 590, Y(1), 80, rh, "\xE2\x88\x92");
            add(eInfo, 201, 680, Y(1), 196, rh, b);
            add(eSet, 159, 886, Y(1), 90, rh, "+");
            tip(m0, wheel ? TR("How far you turn your hands (like a wheel) for full lock. Less = quicker, more = calmer.")
                          : TR("How far you point left / right for full lock. Less = quicker, more = calmer."));
            m0 = el.size();
            add(eSet, 160, 470, Y(1), 110, rh, pad_.steerStickToo ? TR("Stick on") : TR("Stick off"), pad_.steerStickToo);
            tip(m0, TR("Stick on: the stick steers too (it wins while you push it). Stick off: only your hands steer, and the "
                       "stick's left / right are normal buttons again."));
            if (wheel) {
                m0 = el.size(); row(2, TR("Hold the wheel with"));
                opts(2, 161, { TR("Both"), TR("Left"), TR("Right"), TR("Either") }, pad_.steerWheelGrip & 3);
                tip(m0, TR("Both: both grips together (one grip alone keeps its own job). Left / Right / Either: one grip is "
                           "enough -- that grip is then only the wheel (no modifier, no air mouse, no key on it)."));
            } else {
                m0 = el.size(); row(2, TR("Hand that points"));
                opts(2, 165, { TR("Left"), TR("Right") }, pad_.steerPointHand ? 1 : 0);
                tip(m0, TR("The controller you point with. The other hand stays free for the keyboard and the menu."));
            }
        } else {
            m0 = el.size(); row(1, TR("Stick that steers"));
            opts(1, 130, { TR("Left"), TR("Right"), TR("Off") }, pad_.steerStick == 1 ? 0 : pad_.steerStick == 2 ? 1 : 2);
            tip(m0, TR("Which stick steers the truck (left / right). Its up / down stay free for other buttons."));
        }
        m0 = el.size(); row(2 + o, TR("Steering dead zone"));
        snprintf(b, sizeof(b), "%.2f", (double)pad_.steerDeadzone);
        plusMinus(2 + o, 133, b);
        tip(m0, TR("How far you must push the stick before the wheel turns. More = a calm middle, less = it reacts sooner."));
        m0 = el.size(); row(3 + o, TR("Small turns (most of the stick)"));
        if (pad_.steerFine >= pad_.steerFineZone - 0.001f) snprintf(b, sizeof(b), "%s", TR("even (off)"));
        else snprintf(b, sizeof(b), TRF("up to %d %%"), (int)(pad_.steerFine * 100.0f + 0.5f));
        plusMinus(3 + o, 135, b);
        tip(m0, fmt(TRF("Most of the stick (the first %d %% of a push) makes small, careful turns, up to this much of the wheel. "
                        "Only the last bit, at the edge, turns hard."), (int)(pad_.steerFineZone * 100.0f + 0.5f)));
        m0 = el.size(); row(4 + o, TR("Full stick turns the wheel"));
        snprintf(b, sizeof(b), "%d %%", (int)(pad_.steerMax * 100.0f + 0.5f));
        plusMinus(4 + o, 137, b);
        tip(m0, TR("How far the wheel turns with the stick pushed all the way. 100 % = full lock."));
        m0 = el.size(); row(5 + o, TR("Wheel follows the stick"));
        if (pad_.steerFollowTime <= 0.0f) snprintf(b, sizeof(b), "%s", TR("at once"));
        else snprintf(b, sizeof(b), TRF("%.3g s"), (double)pad_.steerFollowTime);
        plusMinus(5 + o, 143, b);
        tip(m0, TR("Smoothing while you hold the stick: how quickly the wheel goes where the stick points. More = smoother, calmer."));
        m0 = el.size(); row(6 + o, TR("Let go: back to the middle"));
        if (pad_.steerCenterTime <= 0.0f) snprintf(b, sizeof(b), "%s", TR("at once"));
        else snprintf(b, sizeof(b), TRF("%.3g s"), (double)pad_.steerCenterTime);
        plusMinus(6 + o, 145, b);
        tip(m0, TR("Let go of the stick: how the wheel comes back to straight. At once = at the wheel speed below. More = a slow glide."));
        m0 = el.size(); row(7 + o, TR("Wheel speed at most"));
        if (pad_.steerRate <= 0.0f) snprintf(b, sizeof(b), "%s", TR("no limit"));
        else snprintf(b, sizeof(b), TRF("%.2g s to full"), 1.0 / (double)pad_.steerRate);
        plusMinus(7 + o, 139, b);
        tip(m0, TR("The fastest the wheel turns (full push; a small push turns it slower): seconds from straight to full lock."));
        {                                               // speed-sensitive: [On / Off] [-] value [+]
            const int r = 8 + o, y = Y(r);
            m0 = el.size();
            row(r, TR("At 90 km/h: steering"));
            add(eSet, 154, 470, y, 110, rh, pad_.steerSpeedSens ? TR("On") : TR("Off"), pad_.steerSpeedSens);
            snprintf(b, sizeof(b), "%d %%", (int)(pad_.steerAtSpeed * 100.0f + 0.5f));
            add(eSet, 152, 590, y, 80, rh, "\xE2\x88\x92");
            add(eInfo, 200 + r, 680, y, 196, rh, pad_.steerSpeedSens ? std::string(b) : std::string(TR("(off)")));
            add(eSet, 153, 886, y, 90, rh, "+");
            tip(m0, TR("On: at speed the wheel turns less, for calm highway driving. The % = how much at 90 km/h."));
        }
        add(eInfo, 0, 24, 704, 742, 54, !status_.empty() ? status_ : !pad_.drive ? TR("Driving off: no steering, gas or brake.")
                                        : pad_.steerWith == 1 ? TR("Wheel: hold the grip(s) and turn your hands. Let go = straight.")
                                        : pad_.steerWith == 2 ? TR("Point: point left / right. On a panel the wheel holds.")
                                        : TR("Most of the stick = small turns. All the way = hard turns."));
    }
    if (!(tab_ == 0 && picking))                        // every tab: back to the defaults (a second press does it)
        add(eSet, kResetArg, 776, 704, 200, 54, resetArm_ == tab_ ? TR("Sure? Reset") : TR("Reset tab"), resetArm_ == tab_);
    if (!el.empty() && el.back().kind == eSet && el.back().arg == kResetArg)
        el.back().help = TR("Puts this tab back to the default settings. Press twice to be sure.");
    // the project, the copyright and the licence (as the DLAA mod and VR EyeCapture show them; text only: a link would open a
    // browser on the desktop and take the focus from the game)
    add(eInfo, kFootArg, 24, 764, 952, 30,
        "github.com/JRDevo/ETS2-ATS-VR-Controls  \xC2\xB7  (c) 2026 JRDevo  \xC2\xB7  License: PolyForm Noncommercial");
    // a pointer on something with a help: the hint line shows it (the pick bar of the Buttons tab has no hint line)
    const std::string help = HoverHelp();
    if (!help.empty())
        for (Elem& e : el)
            if (e.kind == eInfo && e.arg == 0) { e.text = help; e.arg = kHelpArg; break; }
}

void VrMenu::LayoutKb() {
    std::vector<Elem>& el = pan_[kPanKb].el;
    el.clear();
    auto add = [&](int kind, int arg, int x, int y, int w, int h, const std::string& t, bool on = false) {
        Elem e;
        e.kind = kind; e.arg = arg; e.x = x; e.y = y; e.w = w; e.h = h; e.text = t; e.on = on;
        el.push_back(e);
    };
    // one flat block, no title bar: - / + / padlock / x sit in the empty space above the number pad (compact: padlock + x, above Backspace;
    // its size is set in the Panels tab); move it with the grip anywhere. Picking a key for a button: the whole keyboard gets an accent
    // frame (the menu's bar says which button). kKeys is a real keyboard's layout (gaps of 1 / 0.5 key between the groups);
    // here every gap between groups is only kGap of a key: still visible, less wasted room. 2 px of gap around each key.
    // The keys of the layout's shape (kb_layout.h: ANSI / ISO / ABNT2 / JIS) with its labels.
    const bool compact = look_.kbCompact;
    const int geo = 1 << KbLayout::Geometry(kbCode_);
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
        const double b = 0.75 * u;                       //   Panels tab) -- with the padlock left of it: 3/4 of a key each
        add(eLock, 0, (int)(x0 + 15 * u - 2 * b + 2), (int)(y0 + 2), (int)(b - 4), (int)(u - 4), "", look_.kbLock);
        add(eClose, 0, (int)(x0 + 15 * u - b + 2), (int)(y0 + 2), (int)(b - 4), (int)(u - 4), "\xC3\x97");
    } else {
        at(eSmaller, 0, 19, 0, 1, 1, "\xE2\x88\x92");
        at(eBigger, 0, 20, 0, 1, 1, "+");
        at(eLock, 0, 21, 0, 1, 1, "", look_.kbLock);
        at(eClose, 0, 22, 0, 1, 1, "\xC3\x97");
    }
    std::string main, small;
    for (int k = 0; k < kKeyCount; ++k) {
        const KeyDef& d = kKeys[k];
        if (!(d.geo & geo)) continue;
        if (compact && d.x >= 15) continue;              // compact: no navigation / arrows / number pad
        const int m = ModOf(d.name);
        KeyLabel(d, kbCode_, &main, &small);
        at(eKey, d.part ? k - 1 : k, d.x, d.y, d.w, d.h, main, m >= 0 && latch_[m]);   // (the Enter's lower half: the upper's arg)
        el.back().sub = small;
    }
}

void VrMenu::Draw(int p) {                              // the whole panel: a job for the painter
    Panel& pn = pan_[p];
    Painter::Job j;
    j.p = p; j.full = true; j.w = pn.pw; j.h = pn.ph; j.theme = look_.theme;
    j.alpha = (int)((p == kPanMenu ? look_.menuOpacity : look_.kbOpacity) * 255.0f + 0.5f);
    j.frame = p == kPanKb && pickBtn_ >= 0;
    j.face = p == kPanMenu ? Lang::Face() : kbFace_;
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
    j.face = p == kPanMenu ? Lang::Face() : kbFace_;
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
        if (e.arg == kHelpArg) { c.TextWrap(e.x + 4, e.y, e.w - 4, e.h, e.text, 24, T.text); break; }   // a pointed-at help
        const bool label = e.arg >= 100 && e.arg < 200, value = e.arg >= 200 && e.arg < 300, foot = e.arg == kFootArg;
        c.Text(e.x + (value || foot ? 0 : 4), e.y, e.w, e.h, e.text, foot ? 19 : label ? 28 : value ? 30 : 26, value,
               label || value ? T.text : T.dim, value || foot ? DT_CENTER : DT_LEFT);
        break;
    }
    case eRow: {
        c.Round(e.x, e.y, e.w, e.h, 12, prs ? T.press : e.on ? T.on : hov ? T.hover : T.elem);
        c.Text(e.x + 16, e.y, 200, e.h, e.text, 24, false, e.dim ? T.dim : T.text, DT_LEFT);
        c.Text(e.x + 210, e.y, e.w - 226, e.h, e.sub, 26, true, e.dim ? T.dim : T.text, DT_RIGHT);
        break;
    }
    case eKey: {                                        // the label; a second legend (e.sub) small at the top left, as printed
        c.Round(e.x, e.y, e.w, e.h, 8, prs ? T.press : e.on ? T.on : hov ? T.hover : T.elem);
        const int n = CodePoints(e.text);              // a letter / digit / arrow / 2 CJK signs: bigger (not F1, Ins)
        const bool one = n == 1 || ((unsigned char)e.text[0] >= 0x80 && n <= 2);
        if (e.sub.empty()) {
            c.Text(e.x, e.y, e.w, e.h, e.text, one ? 30 : 21, true, T.text, DT_CENTER);
        } else {
            c.Text(e.x + 4, e.y + 2, e.w - 4, e.h * 4 / 10, e.sub, 17, false, T.dim, DT_LEFT);
            c.Text(e.x + e.w / 6, e.y + e.h * 3 / 10, e.w - e.w / 6, e.h * 7 / 10, e.text, one ? 27 : 20, true, T.text, DT_CENTER);
        }
        break;
    }
    case eLock:                                         // the theme's button; a closed padlock = locked, open = not
        c.Round(e.x, e.y, e.w, e.h, 12, prs ? T.press : hov ? T.hover : T.elem);
        c.Padlock(e.x, e.y, e.w, e.h, e.on, T.text);
        break;
    default: {
        c.Round(e.x, e.y, e.w, e.h, 12, prs ? T.press : e.on ? T.on : hov ? T.hover : T.elem);
        const bool glyph = e.text == "+" || e.text == "\xE2\x88\x92" || e.text == "\xC3\x97" || e.text == "\xE2\x80\xB9" ||
                           e.text == "\xE2\x80\xBA";   // + - x < >: bigger
        const int px = glyph ? 40 : e.kind == ePickOut ? 24 : 28;
        if (e.kind == ePickOut && !c.FitsLine(e.w, e.text, px, true))   // the narrow pick buttons: 2 lines when one is too long
            c.TextWrap(e.x + 3, e.y, e.w - 6, e.h, e.text, 22, T.text, true, true);
        else c.Text(e.x, e.y, e.w, e.h, e.text, px, true, T.text, DT_CENTER);
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
            if (e.kind == eInfo && e.help.empty()) break;   // text: nothing to press (a row label: its help only)
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
    if (VrPad::Out* t = VrPad::LayerMap(pad_, pickLayer_)) {   // a layer's entry (None = as normal)
        t[pickBtn_] = o;
        char b[300], raw[64];
        snprintf(b, sizeof(b), TRF("Set: %s + %s = %s"), LayerKeys(pad_, pickLayer_).c_str(), BtnLabel(pickBtn_),
                 o.kind == VrPad::kNone ? TR("as normal") : Pretty(o, kbCode_).c_str());
        status_ = b;
        VrPad::OutToString(o, raw, sizeof(raw));
        LOGI("VR menu: layer %d + %s = %s", pickLayer_, VrPad::BtnNames()[pickBtn_], raw);
    } else {
        pad_.map[pickBtn_] = o;
        char b[300], raw[64];
        snprintf(b, sizeof(b), TRF("Set: %s = %s"), BtnLabel(pickBtn_), Pretty(o, kbCode_).c_str());
        status_ = b;
        VrPad::OutToString(o, raw, sizeof(raw));
        LOGI("VR menu: %s = %s", VrPad::BtnNames()[pickBtn_], raw);
    }
    pickBtn_ = -1;
    for (bool& l : latch_) l = false;
    Save();
}

void VrMenu::SetValue(int arg) {
    auto mul = [](float& v, bool up, float lo, float hi) { v = up ? v * 1.15f : v / 1.15f; v = v < lo ? lo : v > hi ? hi : v; };
    if (arg == kResetArg) {                             // the first press arms, the second resets this tab
        if (resetArm_ != tab_) { resetArm_ = tab_; status_ = TR("Press again: this tab goes back to the defaults."); return; }
        resetArm_ = -1;
        ResetTab();
        status_ = TR("Back to the defaults. Saved.");
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
        const float z = pad_.steerFineZone, k = pad_.steerFine + (arg == 136 ? 0.05f : -0.05f);   // small turns: 5 % steps
        pad_.steerFine = k < 0.05f ? 0.05f : k > z ? z : (float)(int)(k * 20.0f + 0.5f) / 20.0f;
        break;
    }
    case 137: case 138: {
        const float m = pad_.steerMax + (arg == 138 ? 0.1f : -0.1f);
        pad_.steerMax = m < 0.2f ? 0.2f : m > 1.0f ? 1.0f : (float)(int)(m * 10.0f + 0.5f) / 10.0f;
        break;
    }
    case 147: case 148: pad_.drive = arg == 147; break;
    case 155: case 156: case 157: pad_.drive = true; pad_.steerWith = arg - 155; break;
    case 160: pad_.steerStickToo = !pad_.steerStickToo; break;
    case 161: case 162: case 163: case 164: pad_.steerWheelGrip = arg - 161; break;
    case 165: case 166: pad_.steerPointHand = arg - 165; break;
    case 158: case 159: {                              // the hands' angle to full lock: 5 degree steps
        const bool wheel = pad_.steerWith == 1;
        float& v = wheel ? pad_.steerWheelAngle : pad_.steerPointAngle;
        const float lo = 10.0f, hi = wheel ? 180.0f : 90.0f;
        v = (float)(int)((v + (arg == 159 ? 5.0f : -5.0f)) / 5.0f + 0.5f) * 5.0f;
        v = v < lo ? lo : v > hi ? hi : v;
        break;
    }
    case 154: pad_.steerSpeedSens = !pad_.steerSpeedSens; break;
    case 152: case 153: {                              // at 90 km/h: 10 .. 100 % in 5 % steps
        const float v = pad_.steerAtSpeed + (arg == 153 ? 0.05f : -0.05f);
        pad_.steerAtSpeed = v < 0.1f ? 0.1f : v > 1.0f ? 1.0f : (float)(int)(v * 20.0f + 0.5f) / 20.0f;
        break;
    }
    case 143: case 144: case 145: case 146: {          // follow / back-to-the-middle time, in steps (seconds; 0 = at once)
        static const float kFollow[] = { 0.0f, 0.1f, 0.15f, 0.25f, 0.35f, 0.4f, 0.5f, 0.75f, 1.0f, 1.5f };
        static const float kCenter[] = { 0.0f, 0.3f, 0.5f, 0.8f, 1.2f, 1.6f, 2.0f, 3.0f, 4.0f, 5.0f };
        constexpr int kLast = (int)(sizeof(kFollow) / sizeof(kFollow[0])) - 1;
        static_assert(sizeof(kFollow) == sizeof(kCenter), "same number of steps");
        const float* steps = arg < 145 ? kFollow : kCenter;
        float& v = arg < 145 ? pad_.steerFollowTime : pad_.steerCenterTime;
        int i = 0;
        while (i < kLast && steps[i] < v - 1e-4f) ++i;
        i += (arg & 1) ? -1 : 1;                        // (143 / 145 = -: quicker, 144 / 146 = +: calmer)
        v = steps[i < 0 ? 0 : i > kLast ? kLast : i];
        break;
    }
    case 139: case 140: {                              // wheel speed limit: full turns / s in steps (0 = no limit at the top)
        static const float kSpeeds[] = { 0.25f, 0.35f, 0.5f, 0.6f, 0.75f, 1.0f, 1.5f, 2.0f, 3.0f, 5.0f, 0.0f };
        constexpr int kLast = (int)(sizeof(kSpeeds) / sizeof(kSpeeds[0])) - 1;
        int i = 0;
        while (i < kLast && kSpeeds[i] < pad_.steerRate - 1e-4f) ++i;   // (the first step at or above the current limit)
        if (pad_.steerRate <= 0.0f) i = kLast;
        i += arg == 140 ? 1 : -1;
        pad_.steerRate = kSpeeds[i < 0 ? 0 : i > kLast ? kLast : i];
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
    case 120: case 121: case 123: case 124:            // the layer chips
        layer_ = arg == 120 ? 0 : arg == 121 ? 1 : arg == 123 ? 2 : 3;
        pickBtn_ = -1;
        status_.clear();
        return;
    case 122: case 125: {                               // a modifier button: the next of these (off at the end), never the other one
        static const int kChoices[] = { VrPad::kRGrip, VrPad::kLGrip, VrPad::kA, VrPad::kB, VrPad::kX, VrPad::kY, VrPad::kRStick,
                                        VrPad::kLStick, VrPad::kMenu, -1 };
        constexpr int n = (int)(sizeof(kChoices) / sizeof(kChoices[0]));
        int& m = arg == 122 ? pad_.modBtn : pad_.modBtn2;
        const int other = arg == 122 ? pad_.modBtn2 : pad_.modBtn;
        int i = 0;
        while (i < n && kChoices[i] != m) ++i;
        do {                                            // (not both stick clicks: VR EyeCapture records on that pair)
            i = (i + 1) % n;
        } while (kChoices[i] >= 0 && (kChoices[i] == other || (kChoices[i] == VrPad::kLStick && other == VrPad::kRStick) ||
                                      (kChoices[i] == VrPad::kRStick && other == VrPad::kLStick)));
        m = kChoices[i];
        pickBtn_ = -1;
        break;
    }
    case 110: case 111: case 112: case 113: look_.smoothing = arg - 110; break;
    case 170: case 171: CycleLang(false, arg == 171 ? 1 : -1); break;   // the language < / >
    case 172: case 173: CycleLang(true, arg == 173 ? 1 : -1); break;    // the keyboard's letters < / >
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
    status_ = TR("Saved.");
    Save();
}

// the Reset button of the current tab: its settings as a fresh install has them (config.ini as shipped = these defaults)
void VrMenu::ResetTab() {
    const VrPad::Config d = VrPad::DefaultConfig();
    switch (tab_) {
    case 0:                                             // every button, both layers, the modifier button
        for (int b = 0; b < VrPad::kBtnCount; ++b) {
            pad_.map[b] = d.map[b]; pad_.modMap[b] = d.modMap[b]; pad_.modMap2[b] = d.modMap2[b]; pad_.modMapBoth[b] = d.modMapBoth[b];
        }
        pad_.modBtn = d.modBtn;
        pad_.modBtn2 = d.modBtn2;
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
        look_.lang = l.lang; look_.kbLang = l.kbLang;
        RefreshLang();
        needRedraw_ = true;
        break;
    }
    default:
        pad_.drive = d.drive;
        pad_.steerStick = d.steerStick; pad_.steerDeadzone = d.steerDeadzone; pad_.steerFine = d.steerFine;
        pad_.steerFineZone = d.steerFineZone; pad_.steerFullAt = d.steerFullAt;
        pad_.steerMax = d.steerMax; pad_.steerFollowTime = d.steerFollowTime; pad_.steerCenterTime = d.steerCenterTime;
        pad_.steerRate = d.steerRate;
        pad_.steerAtSpeed = d.steerAtSpeed; pad_.steerSpeedSens = d.steerSpeedSens;
        pad_.steerWith = d.steerWith; pad_.steerWheelAngle = d.steerWheelAngle; pad_.steerPointAngle = d.steerPointAngle;
        pad_.steerStickToo = d.steerStickToo; pad_.steerWheelGrip = d.steerWheelGrip; pad_.steerPointHand = d.steerPointHand;
        break;
    }
    LOGI("VR menu: tab %d reset to the defaults", tab_);
    Save();
}

void VrMenu::Activate(int h, const Ref& r, const VrPad::Frame&, VrPad::Sink& sink) {
    if (r.kind == eInfo) return;                         // a row label: only its help (nothing to press)
    if (!(r.kind == eSet && r.arg == kResetArg)) resetArm_ = -1;   // (anything else pressed: Reset is not armed any more)
    Press& ps = press_[h];
    ps = Press();
    ps.pan = r.pan; ps.kind = r.kind; ps.arg = r.arg;
    Panel& pn = pan_[r.pan];
    static const char* const kKindName[] = { "-", "top bar", "close", "smaller", "bigger", "keyboard button", "tab", "button row",
                                             "pick", "pick cancel", "setting", "key", "bring here", "text", "lock" };
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
    case eLock:                                          // the keyboard stays where it is: no grip moves it
        look_.kbLock = !look_.kbLock;
        for (int i = 0; i < 2; ++i) if (grab_[i].pan == kPanKb) { grab_[i] = Grab(); SavePlaces(); }
        LOGI("VR menu: keyboard %s", look_.kbLock ? "locked" : "unlocked");
        Save();
        break;
    case eTab: tab_ = r.arg; status_.clear(); break;
    case eRow:
        {
            char b[300];
            auto say = [&](const char* f) { snprintf(b, sizeof(b), f, BtnLabel(r.arg)); status_ = b; };
            if (!LayerReady(pad_, layer_)) { status_ = TR("Choose that modifier button first (the buttons above)."); break; }
            if (LayerOnWheel(pad_, layer_)) { status_ = TR("Steering with the wheel: those grips hold it (Driving tab)."); break; }
            if ((r.arg == VrPad::kLGrip && (VrPad::WheelGripMask(pad_) & 1)) || (r.arg == VrPad::kRGrip && (VrPad::WheelGripMask(pad_) & 2))) {
                say(TRF("%s holds the wheel (Driving tab)."));
                break;
            }
            if (layer_ && VrPad::IsModifier(pad_, r.arg)) { say(r.arg == pad_.modBtn ? TRF("%s is modifier 1.") : TRF("%s is modifier 2.")); break; }
            if (r.arg == pad_.airBtn) { say(TRF("%s is the air mouse button (Mouse tab).")); break; }
            if (MenuOnly(pad_, gate_.btns, layer_, r.arg)) { say(TRF("With these modifiers, %s opens the menu.")); break; }
            if (r.arg >= VrPad::kLUp && pad_.mouseStick == (r.arg >= VrPad::kRUp ? 2 : 1)) {
                status_ = TR("That stick moves the mouse (Mouse tab).");
                break;
            }
            if (DrivingUses(pad_, r.arg)) { status_ = TR("That stick steers (Driving tab)."); break; }
        }
        pickBtn_ = pickBtn_ == r.arg && pickLayer_ == layer_ ? -1 : r.arg;
        pickLayer_ = layer_;
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
            if (grip && !gripDown_[h] && r.pan >= 0 && g.pan < 0 && !ps.drag && !(r.pan == kPanKb && look_.kbLock)) {
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
    if (painter_->TakeSaveFailed()) status_ = TR("Could not save config.ini (the change works until the game ends).");
}

std::string VrMenu::ContentKey(int p) const {
    char b[200];
    if (p == kPanKb) {                                  // the keyboard: its colours, layout, latched modifiers, the pick frame
        snprintf(b, sizeof(b), "%d|%.2f|%d|%d|%d%d%d|%d|%u|", look_.theme, look_.kbOpacity, look_.kbCompact, pickBtn_ >= 0, latch_[0],
                 latch_[1], latch_[2], look_.kbLock, Lang::Gen());
        return b + kbCode_;
    }
    // the menu: everything its tabs show (not whether it is shown itself)
    snprintf(b, sizeof(b), "%d|%d|%d|%d%d|%.3f,%.3f|%d|%.2f|%.2f|%d|%d|", pan_[1].show, tab_, pickBtn_, pickLayer_, layer_, pan_[0].width,
             pan_[1].width, look_.theme, look_.menuOpacity, look_.kbOpacity, look_.smoothing, look_.kbCompact);
    char dr[160];                                       // the Driving tab's values (DescribePad leaves them out while off)
    snprintf(dr, sizeof(dr), "|%d%d|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|%.3f|", pad_.drive ? 1 : 0, pad_.steerStick,
             pad_.steerDeadzone, pad_.steerFine + 10.0f * pad_.steerFineZone, pad_.steerMax, pad_.steerFollowTime, pad_.steerCenterTime, pad_.steerRate,
             pad_.steerSpeedSens ? pad_.steerAtSpeed : -pad_.steerAtSpeed);
    return std::string(b) + std::to_string(resetArm_) + "|" + status_ + dr + DescribePad(pad_) + "|" + HoverHelp() + "|" +
           std::to_string(Lang::Gen()) + kbCode_ + look_.lang + look_.kbLang;
}

const std::string& VrMenu::HoverHelp() const {
    static const std::string kNone;
    for (int h = 1; h >= 0; --h) {                      // (the right hand first: it points most)
        if (hov_[h].pan != kPanMenu || hov_[h].kind == eNone) continue;
        const Elem* e = Find(kPanMenu, hov_[h].kind, hov_[h].arg);
        if (e && !e->help.empty()) return e->help;
    }
    return kNone;
}

}  // namespace ck
