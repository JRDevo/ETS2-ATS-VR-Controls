// Lang -- the VR menu in the game's languages (the same system as the ETS2 / ATS DLAA mod's menu, there src/lang.h; here
// UTF-8, the menu's own text type).
// gettext style: the English text in the code IS the key. TR("Mouse") returns the active language's text for "Mouse", or
// "Mouse" itself when there is none; TRF("%d %% solid") the same for a printf FORMAT, whose translation is used only when it
// has exactly the same conversions (%d, %.2f, %s ...) in the same order -- else the English one (a player's file can never
// make snprintf read the wrong argument). N_("...") / NF_("...") only MARK a text in a table for scripts/lang_tool.py (the
// extractor); the table's user translates it at the use with Lang::Tr() / Lang::TrF().
// Files: lang/<code>.po in the repo (one per game language, the game's own codes: de_de, ru_ru, ko_kr ...; lang/template.pot
// = every English text, made by `python scripts/lang_tool.py extract`). The build packs them into the DLL (RCDATA LANGPACK:
// a hash of each English text + its translation). A lang\<code>.po NEXT TO THE DLL wins over the packed one, text by text,
// and is read again when it changed (at every menu open): players edit it and see the result in the headset.
// Language: config.ini `language` = auto (the default: the game's language from its game.log.txt / config.cfg, NOT Windows')
// or a code. Font: the menu's GDI face is picked per language from fonts Windows ships (Segoe UI; Yu Gothic UI / Malgun
// Gothic / Microsoft YaHei UI / Microsoft JhengHei UI for ja / ko / zh) and every character of the language is probed
// (GetGlyphIndicesW); none fits (a PC without the fonts) = English + FontMissing() (the menu says so).
// The game's thread only (the menu's layout runs there); the returned texts stay valid until the next Refresh that changes the
// language or reloads a file (Gen() changes then), so a caller copies them (into the menu's std::string elements) at once.
#pragma once
#include <cstdint>
#include <string>

namespace Lang {

uint32_t        Gen();                              // bumped whenever the active texts change (the TR sites re-resolve)
const char*     Tr(const char* en);                 // the translation of `en`, or `en` (content lookup: any string)
const char*     TrF(const char* en);                // the same for a printf format: the conversions must match `en`'s
const char*     TrC(const char* ctxEn);             // "context|English": its own translation (the same English elsewhere may be
                                                    //   longer); untranslated = the English after the '|'

struct Site {                                       // one TR(...) in the code: resolved once per Gen()
    uint32_t gen = ~0u;
    const char* s = nullptr;
    const char* Get(const char* en, bool fmt) {
        if (gen != Gen()) { s = fmt ? TrF(en) : Tr(en); gen = Gen(); }
        return s;
    }
};

void            SetSetting(const std::string& v);   // config.ini language: "auto" or a code (unknown codes = auto)
const char*     Setting();                          // "auto" or the code
void            Refresh();                          // at every menu open: resolve the language, reload changed files, probe
                                                    //   the font (cheap when nothing changed). Logs what it picked.
const char*     Active();                           // the language in use now ("en_us" = English)
const char*     Wanted();                           // the language asked for, or the game's (en_gb stays en_gb: its keyboard)
bool            FontMissing();                      // Wanted() has no font on this PC: the menu shows English
const wchar_t*  Face();                             // the GDI face for the menu ("Segoe UI" for English)
// the first face for language `code` that has a glyph for every character of `chars` ("" = the language's first face);
// nullptr = none has them
const wchar_t*  FaceFor(const std::string& code, const std::wstring& chars);
int             Count();                            // the languages the menu row cycles: 0 = auto, then English, then the packed ones
const char*     CodeAt(int i);                      // "auto", "en_us", "de_de" ...
std::string     NativeName(const std::string& code);  // "Deutsch", "English" ...; the code itself when unknown
bool            LoadForTest(const wchar_t* poPath, const char* code);   // tests: that .po file active as `code` (no pack)
bool            UsePackForTest(const char* code);   // tests: the packed `code` active (as a setting would), no game files read

}  // namespace Lang

#define N_(s)  s
#define NF_(s) s
#define TR(s)  ([]() -> const char* { static Lang::Site site_; return site_.Get(s, false); }())
#define TRF(s) ([]() -> const char* { static Lang::Site site_; return site_.Get(s, true); }())
