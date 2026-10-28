# Review request: Svenska (sv_se) translation of a VR game mod menu

You are a native Svenska speaker and a PC gaming / VR expert. Please review this translation.

**Context.** This is the in-headset settings menu and on-screen keyboard of a VR controller mod for the truck simulators
Euro Truck Simulator 2 and American Truck Simulator: it maps the VR controllers' buttons, sticks and pointing to keyboard
keys, the mouse and steering. The menu floats in front of the player; its boxes are small. All Svenska texts below were
written by an AI and may be wrong, unnatural or too long.

**Check every line for:**
1. Wrong meaning compared with the English.
2. Unnatural wording: use what Svenska PC gamers, VR players and game options menus normally say.
3. Consistency: the same English term should get the same Svenska term everywhere (grip, trigger, stick, modifier, air mouse,
   panel, see-through, dead zone, full lock).
4. Length: the "Where" column gives the room (Latin characters; a CJK character counts as 2). Too long = it gets smaller or
   cut off with "...". Key names on the keyboard: what keyboards in your country have printed (e.g. German: Strg, Entf).

**Rules your corrections must keep (or the mod falls back to English for that line):**
- Keep every placeholder exactly and in the same order: %s %d %.0f %.1f %.2f %.3g %.2g ...
- `%%` is a literal percent sign: keep it as `%%`. Never add a single `%`.
- Do not translate: VR, ETS2, ATS, config.ini, the button letters X / Y / A / B, units px, km/h.

**Answer format.** Only list the lines you would change, as a table:

| # | Current Svenska text | Corrected Svenska text | Reason (short, in English) |
|---|---|---|---|

Then one line: how many lines you checked and an overall quality rating (1-10). Do not repeat lines that are fine.

| # | Where | English | Svenska |
|---|---|---|---|
| 1 | Key on the VR keyboard (very short: about 6 Latin characters) | Esc | Esc |
| 2 | Key on the VR keyboard (very short: about 6 Latin characters) | PrtSc | PrtSc |
| 3 | Key on the VR keyboard (very short: about 6 Latin characters) | ScrLk | ScrLk |
| 4 | Key on the VR keyboard (very short: about 6 Latin characters) | Pause | Pause |
| 5 | Key on the VR keyboard (very short: about 6 Latin characters) | Backspace | ← |
| 6 | Key on the VR keyboard (very short: about 6 Latin characters) | Ins | Ins |
| 7 | Key on the VR keyboard (very short: about 6 Latin characters) | Home | Home |
| 8 | Key on the VR keyboard (very short: about 6 Latin characters) | PgUp | PgUp |
| 9 | Key on the VR keyboard (very short: about 6 Latin characters) | Tab | Tab |
| 10 | Key on the VR keyboard (very short: about 6 Latin characters) | Enter | Enter |
| 11 | Key on the VR keyboard (very short: about 6 Latin characters) | Del | Del |
| 12 | Key on the VR keyboard (very short: about 6 Latin characters) | End | End |
| 13 | Key on the VR keyboard (very short: about 6 Latin characters) | PgDn | PgDn |
| 14 | Key on the VR keyboard (very short: about 6 Latin characters) | Caps | Caps |
| 15 | Key on the VR keyboard (very short: about 6 Latin characters) | Shift | Skift |
| 16 | Key on the VR keyboard (very short: about 6 Latin characters) | Ctrl | Ctrl |
| 17 | Key on the VR keyboard (very short: about 6 Latin characters) | Alt | Alt |
| 18 | Key on the VR keyboard (very short: about 6 Latin characters) | Space | Blank |
| 19 | Controller button name (about 16 Latin characters) | Left trigger | V. trigger |
| 20 | Controller button name (about 16 Latin characters) | Right trigger | H. trigger |
| 21 | Controller button name (about 16 Latin characters) | Left grip | V. grepp |
| 22 | Controller button name (about 16 Latin characters) | Right grip | H. grepp |
| 23 | Controller button name (about 16 Latin characters) | Left stick click | V. spakklick |
| 24 | Controller button name (about 16 Latin characters) | Right stick click | H. spakklick |
| 25 | Controller button name (about 16 Latin characters) | Left menu | V. meny |
| 26 | Controller button name (about 16 Latin characters) | Left stick up | V. spak upp |
| 27 | Controller button name (about 16 Latin characters) | Left stick down | V. spak ner |
| 28 | Controller button name (about 16 Latin characters) | Left stick left | V. spak vänster |
| 29 | Controller button name (about 16 Latin characters) | Left stick right | V. spak höger |
| 30 | Controller button name (about 16 Latin characters) | Right stick up | H. spak upp |
| 31 | Controller button name (about 16 Latin characters) | Right stick down | H. spak ner |
| 32 | Controller button name (about 16 Latin characters) | Right stick left | H. spak vänster |
| 33 | Controller button name (about 16 Latin characters) | Right stick right | H. spak höger |
| 34 | Value or choice (about 18 Latin characters) | Mouse left | Mus vänster |
| 35 | Value or choice (about 18 Latin characters) | Mouse right | Mus höger |
| 36 | Value or choice (about 18 Latin characters) | Mouse middle | Mus mitten |
| 37 | Button (short: about 10 Latin characters) | Wheel up | Hjul upp |
| 38 | Value or choice (about 18 Latin characters) | Wheel down | Hjul ner |
| 39 | Button (short: about 10 Latin characters) | Gas | Gas |
| 40 | Button (short: about 10 Latin characters) | Brake | Broms |
| 41 | Button (short: about 10 Latin characters) | Keyboard | Tangentb. |
| 42 | Value or choice (about 18 Latin characters) | Num %s | Num %s |
| 43 | Title bar (one line, about 40 Latin characters) | Set %s: pick a key | Ställ in %s: välj en tangent |
| 44 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu smaller. | Gör den här menyn mindre. |
| 45 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu bigger. | Gör den här menyn större. |
| 46 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show or hide the VR keyboard. | Visa eller dölj VR-tangentbordet. |
| 47 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Close this menu. To open it again: hold %s. | Stäng den här menyn. Öppna igen: håll %s. |
| 48 | Tab name at the top of the menu (about 11 Latin characters) | Buttons | Knappar |
| 49 | Tab name at the top of the menu (about 11 Latin characters) | Mouse | Mus |
| 50 | Tab name at the top of the menu (about 11 Latin characters) | Panels | Paneler |
| 51 | Tab name at the top of the menu (about 11 Latin characters) | Look | Utseende |
| 52 | Tab name at the top of the menu (about 11 Latin characters) | Driving | Körning |
| 53 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Choose what each controller button does. | Välj vad varje kontrollknapp gör. |
| 54 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The stick mouse and the air mouse (point with the controller). | Spakmusen och luftmusen (peka med kontrollen). |
| 55 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Size and place of this menu and the keyboard. | Storlek och plats för den här menyn och tangentbordet. |
| 56 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Colours, how see-through the panels are, pointer smoothing and the language. | Färger, panelernas genomskinlighet, pekarutjämning och språket. |
| 57 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Steering, gas and brake: how the truck's wheel follows your stick. | Styrning, gas och broms: hur lastbilens ratt följer din spak. |
| 58 | Button (short: about 10 Latin characters) | Normal | Normal |
| 59 | Button (short: about 10 Latin characters) | Mod 1 | Mod 1 |
| 60 | Button (short: about 10 Latin characters) | Mod 2 | Mod 2 |
| 61 | Button (short: about 10 Latin characters) | Both | Båda |
| 62 | Value or choice (about 18 Latin characters) | off | av |
| 63 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does normally. | Visa och ställ in vad varje knapp gör normalt. |
| 64 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 1 (%s). | Visa och ställ in vad varje knapp gör medan du håller mod 1 (%s). |
| 65 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 2 (%s). | Visa och ställ in vad varje knapp gör medan du håller mod 2 (%s). |
| 66 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold BOTH modifiers. | Visa och ställ in vad varje knapp gör medan du håller BÅDA modifierarna. |
| 67 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 1: hold it, and other buttons do their Mod 1 job. Press here to choose another button. | Mod 1: håll den, så gör andra knappar sitt Mod 1-jobb. Tryck här för att välja en annan knapp. |
| 68 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 2: hold it, and other buttons do their Mod 2 job. Hold both modifiers for the Both jobs. Press here to choose another button. | Mod 2: håll den, så gör andra knappar sitt Mod 2-jobb. Håll båda för Båda-jobben. Tryck här för att välja en annan knapp. |
| 69 | Value or choice (about 18 Latin characters) | the wheel | ratten |
| 70 | Value or choice (about 18 Latin characters) | (grips = the wheel) | (grepp = ratten) |
| 71 | Value or choice (about 18 Latin characters) | modifier 1 | modifierare 1 |
| 72 | Value or choice (about 18 Latin characters) | modifier 2 | modifierare 2 |
| 73 | Value or choice (about 18 Latin characters) | air mouse | luftmus |
| 74 | Value or choice (about 18 Latin characters) | moves the mouse | flyttar musen |
| 75 | Value or choice (about 18 Latin characters) | steers | styr |
| 76 | Value or choice (about 18 Latin characters) | opens the menu | öppnar menyn |
| 77 | Value or choice (about 18 Latin characters) | as normal | som normalt |
| 78 | Value or choice (about 18 Latin characters) | (+menu) | (+meny) |
| 79 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: %s (set elsewhere). | %s med %s: %s (inställt på annat håll). |
| 80 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: %s (set elsewhere). | %s: %s (inställt på annat håll). |
| 81 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s med %s: nu %s. Peka hit, dra i triggern, välj sedan en tangent eller ett val nedan. |
| 82 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s: nu %s. Peka hit, dra i triggern, välj sedan en tangent eller ett val nedan. |
| 83 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Also part of the menu's open buttons. | Är också en del av menyns öppna-knappar. |
| 84 | Button (short: about 10 Latin characters) | Mouse L | Mus V |
| 85 | Button (short: about 10 Latin characters) | Mouse R | Mus H |
| 86 | Button (short: about 10 Latin characters) | Mouse M | Mus M |
| 87 | Button (short: about 10 Latin characters) | Wheel dn | Hjul ner |
| 88 | Button (short: about 10 Latin characters) | pick\|Keyboard | Tangentb. |
| 89 | Button (short: about 10 Latin characters) | None | Ingen |
| 90 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Left mouse click. | Vänster musklick. |
| 91 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Right mouse click. | Höger musklick. |
| 92 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Middle mouse click. | Mittenmusklick. |
| 93 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel up (one step per press). | Mushjul upp (ett steg per tryck). |
| 94 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel down (one step per press). | Mushjul ner (ett steg per tryck). |
| 95 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Gas pedal: press harder = more gas. | Gaspedal: tryck hårdare = mer gas. |
| 96 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Brake: press harder = more brake. | Broms: tryck hårdare = mer broms. |
| 97 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Shows / hides the VR keyboard. | Visar / döljer VR-tangentbordet. |
| 98 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Nothing. | Inget. |
| 99 | Button (short: about 10 Latin characters) | Cancel | Avbryt |
| 100 | Status / hint line at the bottom (one line, about 55 Latin characters) | Hold %s first, then the button. | Håll %s först, sedan knappen. |
| 101 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point at a button, pull the trigger, pick its key. | Peka på en knapp, dra i triggern, välj dess tangent. |
| 102 | Name of a setting row (about 30 Latin characters) | Stick that moves the mouse | Spak som flyttar musen |
| 103 | Value or choice (about 18 Latin characters) | Left | Vänster |
| 104 | Value or choice (about 18 Latin characters) | Right | Höger |
| 105 | Button (short: about 10 Latin characters) | Off | Av |
| 106 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick moves the mouse pointer (for the game's menus and map). Off = no stick mouse. | Vilken spak flyttar muspekaren (för menyer och karta). Av = ingen spakmus. |
| 107 | Name of a setting row (about 30 Latin characters) | Stick mouse speed | Spakmusens fart |
| 108 | Value or choice (about 18 Latin characters) | %.0f px/s | %.0f px/s |
| 109 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How fast the pointer moves with the stick pushed all the way, in pixels per second. | Hur snabbt pekaren rör sig med spaken i fullt utslag, i pixlar per sekund. |
| 110 | Name of a setting row (about 30 Latin characters) | Stick dead zone | Spakens dödzon |
| 111 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the pointer moves. More = no creeping pointer. | Hur långt du måste trycka spaken innan pekaren rör sig. Mer = ingen krypning. |
| 112 | Name of a setting row (about 30 Latin characters) | Air mouse button | Luftmusknapp |
| 113 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Hold this button and turn the controller: the pointer follows, like a laser pointer. Off = no air mouse. | Håll den här knappen och vrid kontrollen: pekaren följer, som en laserpekare. Av = ingen luftmus. |
| 114 | Name of a setting row (about 30 Latin characters) | Air mouse works while | Luftmus aktiv |
| 115 | Value or choice (about 18 Latin characters) | Button held | Knapp hålls |
| 116 | Value or choice (about 18 Latin characters) | Toggled on | Växla på/av |
| 117 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Button held: only while you hold the button. Toggled on: one press switches it on, the next press off. | Knapp hålls: bara medan du håller knappen. Växla på/av: ett tryck på, nästa av. |
| 118 | Name of a setting row (about 30 Latin characters) | Air mouse speed | Luftmusens fart |
| 119 | Value or choice (about 18 Latin characters) | %.0f px/deg | %.0f px/grad |
| 120 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the pointer moves when you turn the controller, in pixels per degree. | Hur långt pekaren rör sig när du vrider kontrollen, i pixlar per grad. |
| 121 | Status / hint line at the bottom (one line, about 55 Latin characters) | Changes work at once and are saved. | Ändringar gäller direkt och sparas. |
| 122 | Name of a setting row (about 30 Latin characters) | Menu size | Menystorlek |
| 123 | Value or choice (about 18 Latin characters) | %.2f m wide | %.2f m bred |
| 124 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big this menu is (its width in metres). | Hur stor den här menyn är (bredd i meter). |
| 125 | Name of a setting row (about 30 Latin characters) | Menu distance | Menyavstånd |
| 126 | Value or choice (about 18 Latin characters) | %.2f m | %.2f m |
| 127 | Button (short: about 10 Latin characters) | Closer | Närmare |
| 128 | Button (short: about 10 Latin characters) | Farther | Längre bort |
| 129 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far this menu is from your head. | Hur långt den här menyn är från ditt huvud. |
| 130 | Name of a setting row (about 30 Latin characters) | Keyboard size | Tangentbordsstorlek |
| 131 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big the VR keyboard is (its width in metres). | Hur stort VR-tangentbordet är (bredd i meter). |
| 132 | Name of a setting row (about 30 Latin characters) | Keyboard distance | Tangentbordsavstånd |
| 133 | Value or choice (about 18 Latin characters) | (hidden) | (dolt) |
| 134 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the VR keyboard is from your head. | Hur långt VR-tangentbordet är från ditt huvud. |
| 135 | Button (short: about 10 Latin characters) | Show | Visa |
| 136 | Button (short: about 10 Latin characters) | Hide | Dölj |
| 137 | Button (short: about 10 Latin characters) | Full | Fullt |
| 138 | Button (short: about 10 Latin characters) | Compact | Kompakt |
| 139 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show / hide the VR keyboard. Full = every key; Compact = only the main keys (narrower). | Visa / dölj VR-tangentbordet. Fullt = alla tangenter; Kompakt = bara huvudtangenterna (smalare). |
| 140 | Name of a setting row (about 30 Latin characters) | Bring the menu and the keyboard in front of me | Hämta meny & tangentbord hit |
| 141 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Moves the menu and the keyboard to where you look now. | Flyttar menyn och tangentbordet dit du tittar nu. |
| 142 | Status / hint line at the bottom (one line, about 55 Latin characters) | Menu open / close: hold %s for %.1f s | Meny öppna / stäng: håll %s i %.1f s |
| 143 | Status / hint line at the bottom (one line, about 55 Latin characters) | Move: point at a panel, hold the grip, move. Its stick: farther / closer. | Flytta: peka på en panel, håll greppet, flytta. Dess spak: längre / närmare. |
| 144 | Status / hint line at the bottom (one line, about 55 Latin characters) | The keyboard stays when the menu closes (its x hides it). | Tangentbordet står kvar när menyn stängs (dess x döljer det). |
| 145 | Button (short: about 10 Latin characters) | Dark | Mörk |
| 146 | Button (short: about 10 Latin characters) | Blue | Blå |
| 147 | Button (short: about 10 Latin characters) | Grey | Grå |
| 148 | Button (short: about 10 Latin characters) | Light | Ljus |
| 149 | Button (short: about 10 Latin characters) | Black | Svart |
| 150 | Button (short: about 10 Latin characters) | Green | Grön |
| 151 | Name of a setting row (about 30 Latin characters) | Colours | Färger |
| 152 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The colours of the menu and the keyboard. | Färgerna på menyn och tangentbordet. |
| 153 | Name of a setting row (about 30 Latin characters) | Menu see-through | Menyns genomskinlighet |
| 154 | Value or choice (about 18 Latin characters) | %d %% solid | %d %% täckande |
| 155 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid this menu is. - = more see-through, + = more solid. | Hur täckande den här menyn är. - = mer genomskinlig, + = mer täckande. |
| 156 | Name of a setting row (about 30 Latin characters) | Keyboard see-through | Tangentbordets genomskinlighet |
| 157 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid the VR keyboard is. - = more see-through, + = more solid. | Hur täckande VR-tangentbordet är. - = mer genomskinlig, + = mer täckande. |
| 158 | Name of a setting row (about 30 Latin characters) | Pointer smoothing | Pekarutjämning |
| 159 | Button (short: about 10 Latin characters) | Low | Låg |
| 160 | Button (short: about 10 Latin characters) | Medium | Mellan |
| 161 | Button (short: about 10 Latin characters) | High | Hög |
| 162 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Calms a shaky hand: the menu pointer and the air mouse. Higher = steadier, but it follows a bit later. | Lugnar en skakig hand: menypekaren och luftmusen. Högre = stadigare, men följer lite senare. |
| 163 | Value or choice (about 18 Latin characters) | %s (no font) | %s (inget typsnitt) |
| 164 | Name of a setting row (about 30 Latin characters) | Language | Språk |
| 165 | Value or choice (about 18 Latin characters) | Auto: %s | Auto: %s |
| 166 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The menu's language. Auto = the language the game is set to. | Menyns språk. Auto = det språk spelet är inställt på. |
| 167 | Name of a setting row (about 30 Latin characters) | Keyboard letters | Tangentbordslayout |
| 168 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The letters on the VR keyboard, as printed in that country. Auto = as the menu. A key always presses the same key in the game. | Bokstäverna på VR-tangentbordet, som tryckt i det landet. Auto = som menyn. En tangent trycker alltid samma tangent i spelet. |
| 169 | Status / hint line at the bottom (one line, about 55 Latin characters) | - = more see-through, + = more solid. | - = mer genomskinlig, + = mer täckande. |
| 170 | Name of a setting row (about 30 Latin characters) | Steer with | Styr med |
| 171 | Button (short: about 10 Latin characters) | Stick | Spak |
| 172 | Button (short: about 10 Latin characters) | Wheel | Ratt |
| 173 | Button (short: about 10 Latin characters) | Point | Peka |
| 174 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick: the stick steers. Wheel: hold the grip(s) and turn your hands like a steering wheel. Point: point a controller left / right (on a panel the wheel holds). Off: no driving. | Spak: spaken styr. Ratt: håll greppet/greppen och vrid händerna som en ratt. Peka: peka en kontroll vänster / höger (på en panel som ratten håller). Av: ingen körning. |
| 175 | Name of a setting row (about 30 Latin characters) | Hands turn for full lock | Handvridning för fullt utslag |
| 176 | Name of a setting row (about 30 Latin characters) | Point for full lock | Peka för fullt utslag |
| 177 | Value or choice (about 18 Latin characters) | %.0f deg | %.0f grader |
| 178 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you turn your hands (like a wheel) for full lock. Less = quicker, more = calmer. | Hur långt du vrider händerna (som en ratt) för fullt utslag. Mindre = snabbare, mer = lugnare. |
| 179 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you point left / right for full lock. Less = quicker, more = calmer. | Hur långt du pekar vänster / höger för fullt utslag. Mindre = snabbare, mer = lugnare. |
| 180 | Button (short: about 10 Latin characters) | Stick on | Spak på |
| 181 | Button (short: about 10 Latin characters) | Stick off | Spak av |
| 182 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick on: the stick steers too (it wins while you push it). Stick off: only your hands steer, and the stick's left / right are normal buttons again. | Spak på: spaken styr också (den vinner medan du trycker den). Spak av: bara dina händer styr, och spakens vänster / höger är vanliga knappar igen. |
| 183 | Name of a setting row (about 30 Latin characters) | Hold the wheel with | Håll ratten med |
| 184 | Value or choice (about 18 Latin characters) | Either | Valfri |
| 185 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Both: both grips together (one grip alone keeps its own job). Left / Right / Either: one grip is enough -- that grip is then only the wheel (no modifier, no air mouse, no key on it). | Båda: båda greppen ihop (ett grepp ensamt behåller sitt eget jobb). Vänster / Höger / Valfri: ett grepp räcker -- det greppet är då bara ratten (ingen modifierare, ingen luftmus, ingen tangent). |
| 186 | Name of a setting row (about 30 Latin characters) | Hand that points | Hand som pekar |
| 187 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The controller you point with. The other hand stays free for the keyboard and the menu. | Kontrollen du pekar med. Den andra handen är fri för tangentbordet och menyn. |
| 188 | Name of a setting row (about 30 Latin characters) | Stick that steers | Spak som styr |
| 189 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick steers the truck (left / right). Its up / down stay free for other buttons. | Vilken spak styr lastbilen (vänster / höger). Upp / ner är fria för andra knappar. |
| 190 | Name of a setting row (about 30 Latin characters) | Steering dead zone | Styrningens dödzon |
| 191 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the wheel turns. More = a calm middle, less = it reacts sooner. | Hur långt du måste trycka spaken innan ratten vrids. Mer = lugn mitt, mindre = reagerar tidigare. |
| 192 | Name of a setting row (about 30 Latin characters) | Small turns (most of the stick) | Små svängar (mesta av spaken) |
| 193 | Value or choice (about 18 Latin characters) | even (off) | jämn (av) |
| 194 | Value or choice (about 18 Latin characters) | up to %d %% | upp till %d %% |
| 195 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Most of the stick (the first %d %% of a push) makes small, careful turns, up to this much of the wheel. Only the last bit, at the edge, turns hard. | Mesta av spaken (de första %d %% av ett tryck) ger små, försiktiga svängar, upp till denna del av ratten. Bara sista biten, vid kanten, svänger hårt. |
| 196 | Name of a setting row (about 30 Latin characters) | Full stick turns the wheel | Full spak vrider ratten |
| 197 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the wheel turns with the stick pushed all the way. 100 % = full lock. | Hur långt ratten vrids med spaken i fullt utslag. 100 % = fullt utslag. |
| 198 | Name of a setting row (about 30 Latin characters) | Wheel follows the stick | Ratten följer spaken |
| 199 | Value or choice (about 18 Latin characters) | at once | direkt |
| 200 | Value or choice (about 18 Latin characters) | %.3g s | %.3g s |
| 201 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Smoothing while you hold the stick: how quickly the wheel goes where the stick points. More = smoother, calmer. | Utjämning medan du håller spaken: hur snabbt ratten följer spaken. Mer = mjukare, lugnare. |
| 202 | Name of a setting row (about 30 Latin characters) | Let go: back to the middle | Släpp: tillbaka till mitten |
| 203 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Let go of the stick: how the wheel comes back to straight. At once = at the wheel speed below. More = a slow glide. | Släpp spaken: hur ratten kommer tillbaka till rakt. Direkt = med rattfarten nedan. Mer = en långsam glidning. |
| 204 | Name of a setting row (about 30 Latin characters) | Wheel speed at most | Rattfart högst |
| 205 | Value or choice (about 18 Latin characters) | no limit | ingen gräns |
| 206 | Value or choice (about 18 Latin characters) | %.2g s to full | %.2g s till fullt |
| 207 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The fastest the wheel turns (full push; a small push turns it slower): seconds from straight to full lock. | Rattens högsta fart (fullt tryck; ett litet tryck är långsammare): sekunder från rakt till fullt utslag. |
| 208 | Name of a setting row (about 30 Latin characters) | At 90 km/h: steering | Vid 90 km/h: styrning |
| 209 | Button (short: about 10 Latin characters) | On | På |
| 210 | Value or choice (about 18 Latin characters) | (off) | (av) |
| 211 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | On: at speed the wheel turns less, for calm highway driving. The % = how much at 90 km/h. | På: vid fart vrids ratten mindre, för lugn motorvägskörning. % = hur mycket vid 90 km/h. |
| 212 | Status / hint line at the bottom (one line, about 55 Latin characters) | Driving off: no steering, gas or brake. | Körning av: ingen styrning, gas eller broms. |
| 213 | Status / hint line at the bottom (one line, about 55 Latin characters) | Wheel: hold the grip(s) and turn your hands. Let go = straight. | Ratt: håll greppet/greppen och vrid händerna. Släpp = rakt. |
| 214 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point: point left / right. On a panel the wheel holds. | Peka: peka vänster / höger. På en panel som ratten håller. |
| 215 | Status / hint line at the bottom (one line, about 55 Latin characters) | Most of the stick = small turns. All the way = hard turns. | Mesta av spaken = små svängar. Helt ut = hårda svängar. |
| 216 | Button (short: about 10 Latin characters) | Sure? Reset | Säker? Nollst. |
| 217 | Button (short: about 10 Latin characters) | Reset tab | Nollst. flik |
| 218 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Puts this tab back to the default settings. Press twice to be sure. | Ställer den här fliken åter till standardinställningarna. Tryck två gånger för säkerhets skull. |
| 219 | Value or choice (about 18 Latin characters) | Set: %s + %s = %s | Inställt: %s + %s = %s |
| 220 | Value or choice (about 18 Latin characters) | Set: %s = %s | Inställt: %s = %s |
| 221 | Status / hint line at the bottom (one line, about 55 Latin characters) | Press again: this tab goes back to the defaults. | Tryck igen: den här fliken återgår till standard. |
| 222 | Status / hint line at the bottom (one line, about 55 Latin characters) | Back to the defaults. Saved. | Åter till standard. Sparat. |
| 223 | Status / hint line at the bottom (one line, about 55 Latin characters) | Saved. | Sparat. |
| 224 | Status / hint line at the bottom (one line, about 55 Latin characters) | Choose that modifier button first (the buttons above). | Välj den modifierarknappen först (knapparna ovan). |
| 225 | Status / hint line at the bottom (one line, about 55 Latin characters) | Steering with the wheel: those grips hold it (Driving tab). | Styrning med ratten: de greppen håller den (fliken Körning). |
| 226 | Value or choice (about 18 Latin characters) | %s holds the wheel (Driving tab). | %s håller ratten (fliken Körning). |
| 227 | Value or choice (about 18 Latin characters) | %s is modifier 1. | %s är modifierare 1. |
| 228 | Value or choice (about 18 Latin characters) | %s is modifier 2. | %s är modifierare 2. |
| 229 | Value or choice (about 18 Latin characters) | %s is the air mouse button (Mouse tab). | %s är luftmusknappen (fliken Mus). |
| 230 | Value or choice (about 18 Latin characters) | With these modifiers, %s opens the menu. | Med dessa modifierare öppnar %s menyn. |
| 231 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick moves the mouse (Mouse tab). | Den spaken flyttar musen (fliken Mus). |
| 232 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick steers (Driving tab). | Den spaken styr (fliken Körning). |
| 233 | Status / hint line at the bottom (one line, about 55 Latin characters) | Could not save config.ini (the change works until the game ends). | Kunde inte spara config.ini (ändringen gäller tills spelet avslutas). |
