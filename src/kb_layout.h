// kb_layout -- the VR keyboard's key labels in each ETS2 / ATS language: what that country's keyboards have printed on each key,
// from the standard Windows layout of the language (src/kb_layouts.inc, made by tools/kbd_dump.cpp from Windows' own layout
// DLLs) + the second scripts their keyboards also show (Korean Hangul, Japanese kana, Traditional Chinese Zhuyin) + the key
// SHAPE (geometry): ANSI (US: one-row Enter), ISO (Europe: tall Enter, an extra key next to the left Shift), ABNT2 (Brazil:
// ISO + a key next to the right Shift), JIS (Japan: ¥ and ろ keys, the IME keys around a shorter Space).
// Only the labels and the shape change: every key still sends its SCAN code (the physical key; output.h), so a game binding
// works whatever the labels say. config.ini keyboard_language = auto (the menu's language) or a code.
#pragma once
#include <cstdint>
#include <string>

namespace ck {
namespace KbLayout {

enum Geo { kAnsi, kIso, kAbnt, kJis };

int         Count();                              // the layouts (en_us, en_gb, then every language)
const char* CodeAt(int i);                        // "en_us", "en_gb", "bg_bg" ...
bool        Known(const std::string& code);
// the layout a menu language uses: `code` itself when there is one, else en_us
std::string For(const std::string& code);
int         Geometry(const std::string& code);
bool        AltGr(const std::string& code);       // the right Alt is AltGr there (it types the third characters)
// the label of the key with scan code `scan` (set 1; + 0x100 = E0) in layout `code`: main = the big one; small = the second
// legend top left ("" = none). False = no special label (the caller's own name of the key: Esc, F1, Enter ...).
bool        Label(const std::string& code, uint16_t scan, std::string* main, std::string* small);
// every character the labels of layout `code` use (the font check)
std::wstring Chars(const std::string& code);

}  // namespace KbLayout
}  // namespace ck
