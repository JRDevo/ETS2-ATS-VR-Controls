# Translations of the VR menu and keyboard

The VR menu and the VR keyboard are available in **all 43 languages of ETS2 and ATS**.

> [!WARNING]
> **Best effort — help wanted.** The translations were created by **Claude** (AI) and reviewed by other AI models.
> An AI can never be as accurate as a native speaker, so some texts may still be wrong, clumsy or too long. If you
> have a change or a suggestion, please open a new translation issue: it helps every player of your language.
> **[Report a translation problem](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml)**

## The languages

| Code | Language | Keyboard (as printed in that country) | Status |
|---|---|---|---|
| `en_us` | English (US) | US (QWERTY) | original |
| `en_gb` | English (UK) | UK (QWERTY, ISO) | original |
| `bg_bg` | Български | Bulgarian + Latin | AI draft |
| `ca_es` | Català | Spanish (QWERTY, Ñ Ç) | AI draft |
| `cs_cz` | Česky | Czech (QWERTZ) | AI draft |
| `da_dk` | Dansk | Danish (Æ Ø Å) | AI draft |
| `de_de` | Deutsch | German (QWERTZ, Ü Ö Ä) | AI draft |
| `el_gr` | Ελληνικά | Greek + Latin | AI draft |
| `es_es` | Español | Spanish (QWERTY, Ñ Ç) | AI draft |
| `es_la` | Español (Am. Latina) | Latin American (Ñ) | AI draft |
| `et_ee` | Eesti | Estonian (Õ Ü Ö Ä) | AI draft |
| `eu_es` | Euskara | Spanish (QWERTY, Ñ Ç) | AI draft |
| `fi_fi` | Suomi | Finnish (Å Ö Ä) | AI draft |
| `fr_ca` | Français (Québec) | Canadian French | AI draft |
| `fr_fr` | Français (France) | French (AZERTY) | AI draft |
| `gl_es` | Galego | Spanish (QWERTY, Ñ Ç) | AI draft |
| `hr_hr` | Hrvatski | Croatian (QWERTZ, Š Đ Ž Č Ć) | AI draft |
| `hu_hu` | Magyar | Hungarian (QWERTZ) | AI draft |
| `it_it` | Italiano | Italian | AI draft |
| `ja_jp` | 日本語 | Japanese JIS (kana, ¥, 変換 keys) | AI draft |
| `ka_ge` | ქართული | Georgian + Latin | AI draft |
| `ko_kr` | 한국어 | Korean (Hangul + Latin, 한/영) | AI draft |
| `lt_lt` | Lietuviškai | Lithuanian | AI draft |
| `lv_lv` | Latviešu | Latvian | AI draft |
| `mk_mk` | Македонски | Macedonian + Latin | AI draft |
| `nl_nl` | Nederlands | US International | AI draft |
| `no_no` | Norsk (bokmål) | Norwegian (Ø Æ Å) | AI draft |
| `pl_pl` | Polski | Polish (programmers: US letters) | AI draft |
| `pl_si` | Ślōnskŏ gŏdka | Polish (programmers: US letters) | AI draft |
| `pt_br` | Português (Brasil) | Brazilian ABNT2 (Ç, extra keys) | AI draft |
| `pt_pt` | Português (Portugal) | Portuguese (Ç) | AI draft |
| `ro_ro` | Română | Romanian (Ă Î Â Ș Ț) | AI draft |
| `ru_ru` | Русский | Russian + Latin | AI draft |
| `sk_sk` | Slovenčina | Slovak (QWERTZ) | AI draft |
| `sl_sl` | Slovenščina | Slovenian (QWERTZ) | AI draft |
| `sr_sp` | Srpski (latinica) | Serbian Latin (QWERTZ) | AI draft |
| `sr_sr` | Српски (ћирилица) | Serbian Cyrillic + Latin | AI draft |
| `sv_se` | Svenska | Swedish (Å Ö Ä) | AI draft |
| `tr_tr` | Türkçe | Turkish Q (Ğ Ü Ş İ Ö Ç) | AI draft |
| `uk_uk` | Українська | Ukrainian + Latin | AI draft |
| `vi_vn` | Tiếng Việt | Vietnamese | AI draft |
| `zh_cn` | 简体中文 | US (QWERTY) | AI draft |
| `zh_tw` | 繁體中文 | US + Zhuyin (ㄅㄆㄇㄈ) | AI draft |

A language moves from "AI draft" to "checked" once a native speaker has gone through it.

## How the language is chosen

- **Automatic (the default).** The menu uses **the game's own language**: set ETS2 / ATS to Polish and the menu
  is Polish. The mod reads it from the game, not from Windows. It looks again each time you open the menu.
- **The `Language` row** on the menu's **Look** tab: `‹` / `›` pick another language. The first choice, `Auto`,
  goes back to the game's language. **Reset tab** puts it back to `Auto` too.
- **`config.ini`:** `language = auto`, or a code from the table, for example `language = de_de`.

## The VR keyboard

- Its keys show what **keyboards in that country** have printed: German QWERTZ with Ü Ö Ä, French AZERTY, the Russian
  letters next to the Latin ones, Japanese kana and the JIS keys (半角/全角, 変換, ¥), Korean Hangul, and so on. The labels
  come from the standard Windows keyboard layout of each language.
- The **key shape** follows too: the tall Enter and the extra key next to the left Shift on European keyboards, the extra
  keys of Brazilian and Japanese keyboards.
- **Every key still presses the same physical key in the game.** Only the labels change, so your game key bindings keep
  working.
- **`Keyboard letters` row** (Look tab) or `keyboard_language` in `config.ini`: `auto` = the same language as the menu,
  or a code. Example: an English menu with a German keyboard: `language = en_us`, `keyboard_language = de_de`.

## Fonts

Windows has the fonts for every language: Segoe UI for Latin, Greek, Cyrillic and Georgian; Yu Gothic UI (Japanese),
Malgun Gothic (Korean), Microsoft YaHei UI (Simplified Chinese) and Microsoft JhengHei UI (Traditional Chinese).

If a PC has no font with a language's letters, the menu stays English and the `Language` row says `(no font)`. On
Windows, add the language's fonts: **Settings > Time & language > Language & region > Add a language** (only the
fonts are needed, not the display language).

This mod is made for **Windows**. ETS2 / ATS on Linux (Proton) is not tested with it. If you try it there and the
menu stays English for Chinese, Japanese or Korean, install the fonts once per game (227300 = ETS2, 270880 = ATS):

```
protontricks 227300 cjkfonts
protontricks 270880 cjkfonts
```

## What to change

Each language is one plain text file in this folder: `<code>.po` (for example [`de_de.po`](de_de.po)). Any
text editor opens it; [Poedit](https://poedit.net) makes it easier. One entry looks like this:

```
#. Name of a setting row (about 30 Latin characters)
#: src/vr_menu.cpp
msgid "Air mouse button"
msgstr "Taste für die Luftmaus"
```

- `msgid` = the English text. **Do not change it.**
- `msgstr` = the text in your language. **Change only this line.**
- The `#.` line says where the text is shown and how much room it has. The boxes in the headset are small: keep
  names, buttons and values short. Too long = the text gets smaller, then cut off with "...".
- **Placeholders:** keep every `%s`, `%d`, `%.0f`, `%.2f`, `%.3g` ... in the **same order**, and keep `%%` (that is a
  percent sign). If a line breaks this rule, the mod shows the English text for that line instead.
- **Keyboard keys** (`Esc`, `Enter`, `Shift` ...): what keyboards in your country have printed (German: `Strg`, `Entf`).
  The letter and symbol keys come from the Windows layout, not from this file.
- Keep these as they are: VR, ETS2, ATS, `config.ini`, the controller letters X / Y / A / B.
- An empty `msgstr ""` means "not translated yet": the menu shows the English text there.

## Test your changes in the headset

1. Make a folder `lang` next to `ets2_ats_vr_controls.dll` (the folder where you ran `install.ps1`).
2. Put your edited file there, for example `lang\de_de.po`.
3. Open the VR menu. Your file wins over the built-in texts, line by line. Each time you open the menu, the mod reads
   the file again: edit, close the menu, open it again, look.

## Send your fixes

- **Please write in English.** Explain the problem in English, so that everybody can follow it (a translation
  tool is fine). Only the menu text itself (what it says now, what it should say) is in your language.
- **The easy way: [Report a translation problem](https://github.com/JRDevo/ETS2-ATS-VR-Controls/issues/new?template=translation.yml).**
  Pick the language, say what is wrong and what it should say. You can also attach your edited `.po` file there.
  No git needed.
- **With git:** a pull request that changes your `<code>.po` file.

`python scripts/lang_tool.py check` checks a file before you send it (placeholders, file format).

## For developers

- New menu text in the code goes into `TR("...")`, or `TRF("...")` when it is a printf format; a text in a table:
  `N_("...")` there and `Lang::Tr()` where it is shown (`src/lang.h`).
- `python scripts/lang_tool.py extract` updates [`template.pot`](template.pot) (every English text);
  `python scripts/lang_tool.py update` adds the new texts to every `.po` file (empty = English until translated).
- The build packs all `.po` files into the DLL (`lang_tool.py pack`). `tests\run_unit.ps1` draws every tab in every
  language and fails when a text does not fit.
- `python scripts/lang_tool.py review translation_review` writes one review file per language (paste one into another
  AI model or give it to a native speaker) and a CSV of all languages.
- The keyboard labels: `src/kb_layouts.inc`, made by `tools/kbd_dump.cpp` from Windows' own layout files
  (`build\Release\kbd_dump.exe src\kb_layouts.inc`); the second scripts (Hangul, kana, Zhuyin) are in `src/kb_layout.cpp`.
