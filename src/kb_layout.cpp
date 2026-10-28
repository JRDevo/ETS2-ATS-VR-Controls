// kb_layout -- see kb_layout.h.
#include "kb_layout.h"
#include <windows.h>
#include <cstring>

namespace ck {
namespace KbLayout {
namespace {

struct KeyChars { uint16_t scan; const char* base; const char* shift; const char* altgr; int dead; };
struct LayoutDef { const char* code; const char* klid; const char* name; bool altgr; KeyChars keys[52]; };
const LayoutDef kLayouts[] = {
#include "kb_layouts.inc"
};
constexpr int kLayoutCount = (int)(sizeof(kLayouts) / sizeof(kLayouts[0]));

// second scripts printed on that country's keys (not in the Windows layout: an input method types them)
struct Extra { uint16_t scan; const char* text; };
const Extra kHangul[] = {                         // Korean 2-set (Dubeolsik): the Hangul letter is the big label
    { 0x10, "ㅂ" }, { 0x11, "ㅈ" }, { 0x12, "ㄷ" }, { 0x13, "ㄱ" }, { 0x14, "ㅅ" }, { 0x15, "ㅛ" }, { 0x16, "ㅕ" },
    { 0x17, "ㅑ" }, { 0x18, "ㅐ" }, { 0x19, "ㅔ" }, { 0x1E, "ㅁ" }, { 0x1F, "ㄴ" }, { 0x20, "ㅇ" }, { 0x21, "ㄹ" },
    { 0x22, "ㅎ" }, { 0x23, "ㅗ" }, { 0x24, "ㅓ" }, { 0x25, "ㅏ" }, { 0x26, "ㅣ" }, { 0x2C, "ㅋ" }, { 0x2D, "ㅌ" },
    { 0x2E, "ㅊ" }, { 0x2F, "ㅍ" }, { 0x30, "ㅠ" }, { 0x31, "ㅜ" }, { 0x32, "ㅡ" }, { 0, nullptr } };
const Extra kKana[] = {                           // Japanese JIS kana: the small label
    { 0x02, "ぬ" }, { 0x03, "ふ" }, { 0x04, "あ" }, { 0x05, "う" }, { 0x06, "え" }, { 0x07, "お" }, { 0x08, "や" },
    { 0x09, "ゆ" }, { 0x0A, "よ" }, { 0x0B, "わ" }, { 0x0C, "ほ" }, { 0x0D, "へ" }, { 0x7D, "ー" }, { 0x10, "た" },
    { 0x11, "て" }, { 0x12, "い" }, { 0x13, "す" }, { 0x14, "か" }, { 0x15, "ん" }, { 0x16, "な" }, { 0x17, "に" },
    { 0x18, "ら" }, { 0x19, "せ" }, { 0x1A, "゛" }, { 0x1B, "゜" }, { 0x1E, "ち" }, { 0x1F, "と" }, { 0x20, "し" },
    { 0x21, "は" }, { 0x22, "き" }, { 0x23, "く" }, { 0x24, "ま" }, { 0x25, "の" }, { 0x26, "り" }, { 0x27, "れ" },
    { 0x28, "け" }, { 0x2B, "む" }, { 0x2C, "つ" }, { 0x2D, "さ" }, { 0x2E, "そ" }, { 0x2F, "ひ" }, { 0x30, "こ" },
    { 0x31, "み" }, { 0x32, "も" }, { 0x33, "ね" }, { 0x34, "る" }, { 0x35, "め" }, { 0x73, "ろ" }, { 0, nullptr } };
const Extra kZhuyin[] = {                         // Traditional Chinese (Taiwan) Zhuyin: the small label
    { 0x02, "ㄅ" }, { 0x03, "ㄉ" }, { 0x04, "ˇ" }, { 0x05, "ˋ" }, { 0x06, "ㄓ" }, { 0x07, "ˊ" }, { 0x08, "˙" },
    { 0x09, "ㄚ" }, { 0x0A, "ㄞ" }, { 0x0B, "ㄢ" }, { 0x0C, "ㄦ" }, { 0x10, "ㄆ" }, { 0x11, "ㄊ" }, { 0x12, "ㄍ" },
    { 0x13, "ㄐ" }, { 0x14, "ㄔ" }, { 0x15, "ㄗ" }, { 0x16, "ㄧ" }, { 0x17, "ㄛ" }, { 0x18, "ㄟ" }, { 0x19, "ㄣ" },
    { 0x1E, "ㄇ" }, { 0x1F, "ㄋ" }, { 0x20, "ㄎ" }, { 0x21, "ㄑ" }, { 0x22, "ㄕ" }, { 0x23, "ㄘ" }, { 0x24, "ㄨ" },
    { 0x25, "ㄜ" }, { 0x26, "ㄠ" }, { 0x27, "ㄤ" }, { 0x2C, "ㄈ" }, { 0x2D, "ㄌ" }, { 0x2E, "ㄏ" }, { 0x2F, "ㄒ" },
    { 0x30, "ㄖ" }, { 0x31, "ㄙ" }, { 0x32, "ㄩ" }, { 0x33, "ㄝ" }, { 0x34, "ㄡ" }, { 0x35, "ㄥ" }, { 0, nullptr } };
// whole-key labels of that country's keyboards (not characters: IME / mode keys)
const Extra kJisKeys[] = { { 0x29, "半角/全角" }, { 0x3A, "英数" }, { 0x7B, "無変換" }, { 0x79, "変換" }, { 0x70, "かな" },
                           { 0x7D, "¥" }, { 0, nullptr } };
const Extra kKoKeys[] = { { 0x138, "한/영" }, { 0x11D, "한자" }, { 0, nullptr } };

const LayoutDef* Find(const std::string& code) {
    for (const LayoutDef& l : kLayouts) if (code == l.code) return &l;
    return nullptr;
}
const KeyChars* KeyOf(const LayoutDef* l, uint16_t scan) {
    if (!l) return nullptr;
    for (const KeyChars& k : l->keys) {
        if (!k.base) break;
        if (k.scan == scan) return &k;
    }
    return nullptr;
}
const char* ExtraOf(const Extra* t, uint16_t scan) {
    for (; t && t->text; ++t) if (t->scan == scan) return t->text;
    return nullptr;
}

std::wstring W(const char* s) {
    if (!s || !*s) return std::wstring();
    const int n = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
    std::wstring w((size_t)(n > 0 ? n - 1 : 0), L'\0');
    if (n > 1) MultiByteToWideChar(CP_UTF8, 0, s, -1, &w[0], n);
    return w;
}
std::string U(const std::wstring& w) {
    if (w.empty()) return std::string();
    const int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s((size_t)n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), (int)w.size(), &s[0], n, nullptr, nullptr);
    return s;
}
std::wstring Upper(std::wstring w) { if (!w.empty()) CharUpperBuffW(&w[0], (DWORD)w.size()); return w; }
bool OneLetter(const std::wstring& w) { return w.size() == 1 && IsCharAlphaW(w[0]); }
// a lone combining mark (Vietnamese tone keys) gets a dotted circle to sit on
std::wstring Visible(const std::wstring& w) {
    if (w.size() == 1 && w[0] >= 0x0300 && w[0] <= 0x036F) return std::wstring(1, L'\x25CC') + w;
    return w;
}

}  // namespace

int Count() { return kLayoutCount; }
const char* CodeAt(int i) { return i >= 0 && i < kLayoutCount ? kLayouts[i].code : "en_us"; }
bool Known(const std::string& code) { return Find(code) != nullptr; }
std::string For(const std::string& code) { return Find(code) ? code : std::string("en_us"); }
bool AltGr(const std::string& code) { const LayoutDef* l = Find(code); return l && l->altgr; }

int Geometry(const std::string& code) {
    const LayoutDef* l = Find(code);
    if (!l) return kAnsi;
    if (code == "ja_jp") return kJis;
    if (KeyOf(l, 0x73)) return kAbnt;
    const KeyChars* a = KeyOf(l, 0x56);
    const KeyChars* b = KeyOf(l, 0x2B);
    if (a && (!b || strcmp(a->base, b->base) || strcmp(a->shift, b->shift))) return kIso;
    return kAnsi;
}

bool Label(const std::string& code, uint16_t scan, std::string* main, std::string* small) {
    main->clear();
    small->clear();
    const LayoutDef* l = Find(code);
    if (!l) return false;
    const bool ja = code == "ja_jp", ko = code == "ko_kr";
    if (const char* k = ExtraOf(ja ? kJisKeys : ko ? kKoKeys : nullptr, scan)) {
        *main = k;
        if (scan == 0x7D) if (const char* kana = ExtraOf(kKana, scan)) *small = kana;
        return true;
    }
    if (scan == 0x138 && l->altgr) { *main = "AltGr"; return true; }
    const KeyChars* k = KeyOf(l, scan);
    if (!k) return false;
    const std::wstring b = W(k->base), s = W(k->shift);
    std::wstring m, sm;
    if (OneLetter(b) && (s == b || (OneLetter(s) && IsCharUpperW(s[0])))) {
        m = s == b ? b : s;                             // a letter: its capital (Turkish i: İ), as printed
    } else if (s.size() == 1 && s[0] >= L'0' && s[0] <= L'9' && !b.empty()) {
        m = s; sm = b;                                  // AZERTY / Czech number row: the digit, its own character small
    } else {
        m = b.empty() ? s : b;
        if (s != b && s != Upper(b)) sm = s;
    }
    const KeyChars* us = KeyOf(Find("en_us"), scan);
    bool latin = true;                                  // the A .. L row has Latin letters (Greek / Bulgarian: their Q key is
    for (uint16_t sc = 0x1E; sc <= 0x26; ++sc)          //   a symbol, so the letters decide)
        if (const KeyChars* r = KeyOf(l, sc)) latin = latin && (unsigned char)r->base[0] < 0x80;
    if (!latin && us && W(us->base) != b) sm = Upper(W(us->base));   // Cyrillic / Greek / Georgian: the Latin letter small
    if (ko) if (const char* h = ExtraOf(kHangul, scan)) { sm = m; m = W(h); }
    if (ja) if (const char* kana = ExtraOf(kKana, scan)) sm = W(kana);
    if (code == "zh_tw") if (const char* z = ExtraOf(kZhuyin, scan)) sm = W(z);
    *main = U(Visible(m));
    *small = U(Visible(sm));
    return !main->empty();
}

std::wstring Chars(const std::string& code) {
    std::wstring out;
    const LayoutDef* l = Find(code);
    if (!l) return out;
    std::string m, s;
    for (const KeyChars& k : l->keys) {
        if (!k.base) break;
        if (Label(code, k.scan, &m, &s)) out += W(m.c_str()) + W(s.c_str());
    }
    for (const Extra* t : { kJisKeys, kKoKeys })
        for (const Extra* e = t; e->text; ++e)
            if (Label(code, e->scan, &m, &s)) out += W(m.c_str()) + W(s.c_str());
    return out;
}

}  // namespace KbLayout
}  // namespace ck
