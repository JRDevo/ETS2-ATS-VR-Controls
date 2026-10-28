# Review request: Eesti (et_ee) translation of a VR game mod menu

You are a native Eesti speaker and a PC gaming / VR expert. Please review this translation.

**Context.** This is the in-headset settings menu and on-screen keyboard of a VR controller mod for the truck simulators
Euro Truck Simulator 2 and American Truck Simulator: it maps the VR controllers' buttons, sticks and pointing to keyboard
keys, the mouse and steering. The menu floats in front of the player; its boxes are small. All Eesti texts below were
written by an AI and may be wrong, unnatural or too long.

**Check every line for:**
1. Wrong meaning compared with the English.
2. Unnatural wording: use what Eesti PC gamers, VR players and game options menus normally say.
3. Consistency: the same English term should get the same Eesti term everywhere (grip, trigger, stick, modifier, air mouse,
   panel, see-through, dead zone, full lock).
4. Length: the "Where" column gives the room (Latin characters; a CJK character counts as 2). Too long = it gets smaller or
   cut off with "...". Key names on the keyboard: what keyboards in your country have printed (e.g. German: Strg, Entf).

**Rules your corrections must keep (or the mod falls back to English for that line):**
- Keep every placeholder exactly and in the same order: %s %d %.0f %.1f %.2f %.3g %.2g ...
- `%%` is a literal percent sign: keep it as `%%`. Never add a single `%`.
- Do not translate: VR, ETS2, ATS, config.ini, the button letters X / Y / A / B, units px, km/h.

**Answer format.** Only list the lines you would change, as a table:

| # | Current Eesti text | Corrected Eesti text | Reason (short, in English) |
|---|---|---|---|

Then one line: how many lines you checked and an overall quality rating (1-10). Do not repeat lines that are fine.

| # | Where | English | Eesti |
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
| 15 | Key on the VR keyboard (very short: about 6 Latin characters) | Shift | Shift |
| 16 | Key on the VR keyboard (very short: about 6 Latin characters) | Ctrl | Ctrl |
| 17 | Key on the VR keyboard (very short: about 6 Latin characters) | Alt | Alt |
| 18 | Key on the VR keyboard (very short: about 6 Latin characters) | Space | Tühik |
| 19 | Controller button name (about 16 Latin characters) | Left trigger | V. päästik |
| 20 | Controller button name (about 16 Latin characters) | Right trigger | P. päästik |
| 21 | Controller button name (about 16 Latin characters) | Left grip | V. grip |
| 22 | Controller button name (about 16 Latin characters) | Right grip | P. grip |
| 23 | Controller button name (about 16 Latin characters) | Left stick click | V. kangi klõps |
| 24 | Controller button name (about 16 Latin characters) | Right stick click | P. kangi klõps |
| 25 | Controller button name (about 16 Latin characters) | Left menu | V. menüü |
| 26 | Controller button name (about 16 Latin characters) | Left stick up | V. kang üles |
| 27 | Controller button name (about 16 Latin characters) | Left stick down | V. kang alla |
| 28 | Controller button name (about 16 Latin characters) | Left stick left | V. kang vasak |
| 29 | Controller button name (about 16 Latin characters) | Left stick right | V. kang parem |
| 30 | Controller button name (about 16 Latin characters) | Right stick up | P. kang üles |
| 31 | Controller button name (about 16 Latin characters) | Right stick down | P. kang alla |
| 32 | Controller button name (about 16 Latin characters) | Right stick left | P. kang vasak |
| 33 | Controller button name (about 16 Latin characters) | Right stick right | P. kang parem |
| 34 | Value or choice (about 18 Latin characters) | Mouse left | Hiir vasak |
| 35 | Value or choice (about 18 Latin characters) | Mouse right | Hiir parem |
| 36 | Value or choice (about 18 Latin characters) | Mouse middle | Hiir keskmine |
| 37 | Button (short: about 10 Latin characters) | Wheel up | Ratas üles |
| 38 | Value or choice (about 18 Latin characters) | Wheel down | Ratas alla |
| 39 | Button (short: about 10 Latin characters) | Gas | Gaas |
| 40 | Button (short: about 10 Latin characters) | Brake | Pidur |
| 41 | Button (short: about 10 Latin characters) | Keyboard | Klaviat. |
| 42 | Value or choice (about 18 Latin characters) | Num %s | Num %s |
| 43 | Title bar (one line, about 40 Latin characters) | Set %s: pick a key | Määra %s: vali klahv |
| 44 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu smaller. | Tee see menüü väiksemaks. |
| 45 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu bigger. | Tee see menüü suuremaks. |
| 46 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show or hide the VR keyboard. | Näita või peida VR-klaviatuur. |
| 47 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Close this menu. To open it again: hold %s. | Sulge see menüü. Ava uuesti: hoia %s. |
| 48 | Tab name at the top of the menu (about 11 Latin characters) | Buttons | Nupud |
| 49 | Tab name at the top of the menu (about 11 Latin characters) | Mouse | Hiir |
| 50 | Tab name at the top of the menu (about 11 Latin characters) | Panels | Paneelid |
| 51 | Tab name at the top of the menu (about 11 Latin characters) | Look | Välimus |
| 52 | Tab name at the top of the menu (about 11 Latin characters) | Driving | Sõit |
| 53 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Choose what each controller button does. | Vali, mida iga puldi nupp teeb. |
| 54 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The stick mouse and the air mouse (point with the controller). | Kangihiir ja õhuhiir (osuta puldiga). |
| 55 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Size and place of this menu and the keyboard. | Selle menüü ja klaviatuuri suurus ja asukoht. |
| 56 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Colours, how see-through the panels are, pointer smoothing and the language. | Värvid, paneelide läbipaistvus, kursori silumine ja keel. |
| 57 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Steering, gas and brake: how the truck's wheel follows your stick. | Roolimine, gaas ja pidur: kuidas veoki rool su kangi järgib. |
| 58 | Button (short: about 10 Latin characters) | Normal | Tavaline |
| 59 | Button (short: about 10 Latin characters) | Mod 1 | Mod 1 |
| 60 | Button (short: about 10 Latin characters) | Mod 2 | Mod 2 |
| 61 | Button (short: about 10 Latin characters) | Both | Mõlemad |
| 62 | Value or choice (about 18 Latin characters) | off | väljas |
| 63 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does normally. | Näita ja määra, mida iga nupp tavaliselt teeb. |
| 64 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 1 (%s). | Näita ja määra, mida iga nupp teeb, kui hoiad modifikaatorit 1 (%s). |
| 65 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 2 (%s). | Näita ja määra, mida iga nupp teeb, kui hoiad modifikaatorit 2 (%s). |
| 66 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold BOTH modifiers. | Näita ja määra, mida iga nupp teeb, kui hoiad MÕLEMAID modifikaatoreid. |
| 67 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 1: hold it, and other buttons do their Mod 1 job. Press here to choose another button. | Modifikaator 1: hoia seda, siis teevad teised nupud oma Mod 1 töö. Vajuta siia, et valida teine nupp. |
| 68 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 2: hold it, and other buttons do their Mod 2 job. Hold both modifiers for the Both jobs. Press here to choose another button. | Modifikaator 2: hoia seda, siis teevad teised oma Mod 2 töö. Hoia mõlemaid Mõlemad-tööde jaoks. Vajuta siia, et valida teine nupp. |
| 69 | Value or choice (about 18 Latin characters) | the wheel | rool |
| 70 | Value or choice (about 18 Latin characters) | (grips = the wheel) | (gripid = rool) |
| 71 | Value or choice (about 18 Latin characters) | modifier 1 | modifikaator 1 |
| 72 | Value or choice (about 18 Latin characters) | modifier 2 | modifikaator 2 |
| 73 | Value or choice (about 18 Latin characters) | air mouse | õhuhiir |
| 74 | Value or choice (about 18 Latin characters) | moves the mouse | liigutab hiirt |
| 75 | Value or choice (about 18 Latin characters) | steers | roolib |
| 76 | Value or choice (about 18 Latin characters) | opens the menu | avab menüü |
| 77 | Value or choice (about 18 Latin characters) | as normal | nagu tavaliselt |
| 78 | Value or choice (about 18 Latin characters) | (+menu) | (+menüü) |
| 79 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: %s (set elsewhere). | %s + %s: %s (määratud mujal). |
| 80 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: %s (set elsewhere). | %s: %s (määratud mujal). |
| 81 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s + %s: nüüd %s. Osuta siia, tõmba päästikut, siis vali klahv või valik allpool. |
| 82 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s: nüüd %s. Osuta siia, tõmba päästikut, siis vali klahv või valik allpool. |
| 83 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Also part of the menu's open buttons. | On ka osa menüü avamisnuppudest. |
| 84 | Button (short: about 10 Latin characters) | Mouse L | Hiir V |
| 85 | Button (short: about 10 Latin characters) | Mouse R | Hiir P |
| 86 | Button (short: about 10 Latin characters) | Mouse M | Hiir M |
| 87 | Button (short: about 10 Latin characters) | Wheel dn | Ratas alla |
| 88 | Button (short: about 10 Latin characters) | pick\|Keyboard | Klaviat. |
| 89 | Button (short: about 10 Latin characters) | None | Puudub |
| 90 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Left mouse click. | Hiire vasakklõps. |
| 91 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Right mouse click. | Hiire paremklõps. |
| 92 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Middle mouse click. | Hiire keskklõps. |
| 93 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel up (one step per press). | Hiireratas üles (üks samm vajutuse kohta). |
| 94 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel down (one step per press). | Hiireratas alla (üks samm vajutuse kohta). |
| 95 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Gas pedal: press harder = more gas. | Gaasipedaal: vajuta tugevamini = rohkem gaasi. |
| 96 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Brake: press harder = more brake. | Pidur: vajuta tugevamini = rohkem pidurit. |
| 97 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Shows / hides the VR keyboard. | Näitab / peidab VR-klaviatuuri. |
| 98 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Nothing. | Mitte midagi. |
| 99 | Button (short: about 10 Latin characters) | Cancel | Tühista |
| 100 | Status / hint line at the bottom (one line, about 55 Latin characters) | Hold %s first, then the button. | Hoia esmalt %s, siis nuppu. |
| 101 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point at a button, pull the trigger, pick its key. | Osuta nupule, tõmba päästikut, vali selle klahv. |
| 102 | Name of a setting row (about 30 Latin characters) | Stick that moves the mouse | Hiirt liigutav kang |
| 103 | Value or choice (about 18 Latin characters) | Left | Vasak |
| 104 | Value or choice (about 18 Latin characters) | Right | Parem |
| 105 | Button (short: about 10 Latin characters) | Off | Väljas |
| 106 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick moves the mouse pointer (for the game's menus and map). Off = no stick mouse. | Milline kang liigutab kursorit (menüüd ja kaart). Väljas = kangihiirt pole. |
| 107 | Name of a setting row (about 30 Latin characters) | Stick mouse speed | Kangihiire kiirus |
| 108 | Value or choice (about 18 Latin characters) | %.0f px/s | %.0f px/s |
| 109 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How fast the pointer moves with the stick pushed all the way, in pixels per second. | Kui kiiresti kursor liigub, kui kang on lõpuni lükatud, pikslit sekundis. |
| 110 | Name of a setting row (about 30 Latin characters) | Stick dead zone | Kangi surnud ala |
| 111 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the pointer moves. More = no creeping pointer. | Kui kaugele pead kangi lükkama, enne kui kursor liigub. Rohkem = kursor ei triivi. |
| 112 | Name of a setting row (about 30 Latin characters) | Air mouse button | Õhuhiire nupp |
| 113 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Hold this button and turn the controller: the pointer follows, like a laser pointer. Off = no air mouse. | Hoia seda nuppu ja pööra pulti: kursor järgib nagu laserosuti. Väljas = õhuhiirt pole. |
| 114 | Name of a setting row (about 30 Latin characters) | Air mouse works while | Õhuhiir töötab |
| 115 | Value or choice (about 18 Latin characters) | Button held | Nupp all |
| 116 | Value or choice (about 18 Latin characters) | Toggled on | Lülitatud sisse |
| 117 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Button held: only while you hold the button. Toggled on: one press switches it on, the next press off. | Nupp all: ainult kui hoiad nuppu. Lülitatud sisse: üks vajutus sisse, järgmine välja. |
| 118 | Name of a setting row (about 30 Latin characters) | Air mouse speed | Õhuhiire kiirus |
| 119 | Value or choice (about 18 Latin characters) | %.0f px/deg | %.0f px/kraad |
| 120 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the pointer moves when you turn the controller, in pixels per degree. | Kui kaugele kursor liigub, kui pöörad pulti, pikslit kraadi kohta. |
| 121 | Status / hint line at the bottom (one line, about 55 Latin characters) | Changes work at once and are saved. | Muudatused mõjuvad kohe ja salvestatakse. |
| 122 | Name of a setting row (about 30 Latin characters) | Menu size | Menüü suurus |
| 123 | Value or choice (about 18 Latin characters) | %.2f m wide | %.2f m lai |
| 124 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big this menu is (its width in metres). | Kui suur see menüü on (laius meetrites). |
| 125 | Name of a setting row (about 30 Latin characters) | Menu distance | Menüü kaugus |
| 126 | Value or choice (about 18 Latin characters) | %.2f m | %.2f m |
| 127 | Button (short: about 10 Latin characters) | Closer | Lähemale |
| 128 | Button (short: about 10 Latin characters) | Farther | Kaugemale |
| 129 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far this menu is from your head. | Kui kaugel see menüü su peast on. |
| 130 | Name of a setting row (about 30 Latin characters) | Keyboard size | Klaviatuuri suurus |
| 131 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big the VR keyboard is (its width in metres). | Kui suur VR-klaviatuur on (laius meetrites). |
| 132 | Name of a setting row (about 30 Latin characters) | Keyboard distance | Klaviatuuri kaugus |
| 133 | Value or choice (about 18 Latin characters) | (hidden) | (peidetud) |
| 134 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the VR keyboard is from your head. | Kui kaugel VR-klaviatuur su peast on. |
| 135 | Button (short: about 10 Latin characters) | Show | Näita |
| 136 | Button (short: about 10 Latin characters) | Hide | Peida |
| 137 | Button (short: about 10 Latin characters) | Full | Täis |
| 138 | Button (short: about 10 Latin characters) | Compact | Kompaktne |
| 139 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show / hide the VR keyboard. Full = every key; Compact = only the main keys (narrower). | Näita / peida VR-klaviatuur. Täis = kõik klahvid; Kompaktne = ainult põhiklahvid (kitsam). |
| 140 | Name of a setting row (about 30 Latin characters) | Bring the menu and the keyboard in front of me | Too menüü & klaviatuur minu ette |
| 141 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Moves the menu and the keyboard to where you look now. | Viib menüü ja klaviatuuri sinna, kuhu praegu vaatad. |
| 142 | Status / hint line at the bottom (one line, about 55 Latin characters) | Menu open / close: hold %s for %.1f s | Menüü ava / sulge: hoia %s %.1f s |
| 143 | Status / hint line at the bottom (one line, about 55 Latin characters) | Move: point at a panel, hold the grip, move. Its stick: farther / closer. | Liiguta: osuta paneelile, hoia gripi, liiguta. Kang: kaugemale / lähemale. |
| 144 | Status / hint line at the bottom (one line, about 55 Latin characters) | The keyboard stays when the menu closes (its x hides it). | Klaviatuur jääb, kui menüü sulgub (selle x peidab selle). |
| 145 | Button (short: about 10 Latin characters) | Dark | Tume |
| 146 | Button (short: about 10 Latin characters) | Blue | Sinine |
| 147 | Button (short: about 10 Latin characters) | Grey | Hall |
| 148 | Button (short: about 10 Latin characters) | Light | Hele |
| 149 | Button (short: about 10 Latin characters) | Black | Must |
| 150 | Button (short: about 10 Latin characters) | Green | Roheline |
| 151 | Name of a setting row (about 30 Latin characters) | Colours | Värvid |
| 152 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The colours of the menu and the keyboard. | Menüü ja klaviatuuri värvid. |
| 153 | Name of a setting row (about 30 Latin characters) | Menu see-through | Menüü läbipaistvus |
| 154 | Value or choice (about 18 Latin characters) | %d %% solid | %d %% kattev |
| 155 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid this menu is. - = more see-through, + = more solid. | Kui kattev see menüü on. - = läbipaistvam, + = kattevam. |
| 156 | Name of a setting row (about 30 Latin characters) | Keyboard see-through | Klaviatuuri läbipaistvus |
| 157 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid the VR keyboard is. - = more see-through, + = more solid. | Kui kattev VR-klaviatuur on. - = läbipaistvam, + = kattevam. |
| 158 | Name of a setting row (about 30 Latin characters) | Pointer smoothing | Kursori silumine |
| 159 | Button (short: about 10 Latin characters) | Low | Madal |
| 160 | Button (short: about 10 Latin characters) | Medium | Keskmine |
| 161 | Button (short: about 10 Latin characters) | High | Kõrge |
| 162 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Calms a shaky hand: the menu pointer and the air mouse. Higher = steadier, but it follows a bit later. | Rahustab värisevat kätt: menüü kursor ja õhuhiir. Kõrgem = stabiilsem, kuid järgib veidi hiljem. |
| 163 | Value or choice (about 18 Latin characters) | %s (no font) | %s (fonti pole) |
| 164 | Name of a setting row (about 30 Latin characters) | Language | Keel |
| 165 | Value or choice (about 18 Latin characters) | Auto: %s | Auto: %s |
| 166 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The menu's language. Auto = the language the game is set to. | Menüü keel. Auto = keel, millele mäng on seatud. |
| 167 | Name of a setting row (about 30 Latin characters) | Keyboard letters | Klaviatuuri paigutus |
| 168 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The letters on the VR keyboard, as printed in that country. Auto = as the menu. A key always presses the same key in the game. | VR-klaviatuuri tähed, nagu selles riigis trükitud. Auto = nagu menüü. Klahv vajutab mängus alati sama klahvi. |
| 169 | Status / hint line at the bottom (one line, about 55 Latin characters) | - = more see-through, + = more solid. | - = läbipaistvam, + = kattevam. |
| 170 | Name of a setting row (about 30 Latin characters) | Steer with | Roolimisviis |
| 171 | Button (short: about 10 Latin characters) | Stick | Kang |
| 172 | Button (short: about 10 Latin characters) | Wheel | Rool |
| 173 | Button (short: about 10 Latin characters) | Point | Osuta |
| 174 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick: the stick steers. Wheel: hold the grip(s) and turn your hands like a steering wheel. Point: point a controller left / right (on a panel the wheel holds). Off: no driving. | Kang: kang roolib. Rool: hoia gripp(e) ja pööra käsi nagu rooli. Osuta: osuta puldiga vasak / parem (paneelil, mida rool hoiab). Väljas: sõitu pole. |
| 175 | Name of a setting row (about 30 Latin characters) | Hands turn for full lock | Käte pööre rooli lõpuni |
| 176 | Name of a setting row (about 30 Latin characters) | Point for full lock | Osutus rooli lõpuni |
| 177 | Value or choice (about 18 Latin characters) | %.0f deg | %.0f kraadi |
| 178 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you turn your hands (like a wheel) for full lock. Less = quicker, more = calmer. | Kui palju pöörad käsi (nagu rooli) rooli lõpuni. Vähem = kiirem, rohkem = rahulikum. |
| 179 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you point left / right for full lock. Less = quicker, more = calmer. | Kui kaugele osutad vasak / parem rooli lõpuni. Vähem = kiirem, rohkem = rahulikum. |
| 180 | Button (short: about 10 Latin characters) | Stick on | Kang sees |
| 181 | Button (short: about 10 Latin characters) | Stick off | Kang väljas |
| 182 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick on: the stick steers too (it wins while you push it). Stick off: only your hands steer, and the stick's left / right are normal buttons again. | Kang sees: ka kang roolib (ta võidab, kui seda lükkad). Kang väljas: ainult su käed roolivad, ja kangi vasak / parem on taas tavalised nupud. |
| 183 | Name of a setting row (about 30 Latin characters) | Hold the wheel with | Hoia rooli |
| 184 | Value or choice (about 18 Latin characters) | Either | Kumb tahes |
| 185 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Both: both grips together (one grip alone keeps its own job). Left / Right / Either: one grip is enough -- that grip is then only the wheel (no modifier, no air mouse, no key on it). | Mõlemad: mõlemad gripid koos (üks gripp üksi säilitab oma töö). Vasak / Parem / Kumb tahes: üks gripp piisab -- see gripp on siis ainult rool (modifikaatorit pole, õhuhiirt pole, klahvi pole). |
| 186 | Name of a setting row (about 30 Latin characters) | Hand that points | Osutav käsi |
| 187 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The controller you point with. The other hand stays free for the keyboard and the menu. | Pult, millega osutad. Teine käsi jääb vabaks klaviatuurile ja menüüle. |
| 188 | Name of a setting row (about 30 Latin characters) | Stick that steers | Rooliv kang |
| 189 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick steers the truck (left / right). Its up / down stay free for other buttons. | Milline kang roolib veokit (vasak / parem). Üles / alla jäävad vabaks teistele nuppudele. |
| 190 | Name of a setting row (about 30 Latin characters) | Steering dead zone | Roolimise surnud ala |
| 191 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the wheel turns. More = a calm middle, less = it reacts sooner. | Kui kaugele pead kangi lükkama, enne kui rool pöörab. Rohkem = rahulik kese, vähem = reageerib varem. |
| 192 | Name of a setting row (about 30 Latin characters) | Small turns (most of the stick) | Väikesed pöörded (kangist) |
| 193 | Value or choice (about 18 Latin characters) | even (off) | ühtlane (väljas) |
| 194 | Value or choice (about 18 Latin characters) | up to %d %% | kuni %d %% |
| 195 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Most of the stick (the first %d %% of a push) makes small, careful turns, up to this much of the wheel. Only the last bit, at the edge, turns hard. | Suurem osa kangist (lükke esimesed %d %%) teeb väikeseid, ettevaatlikke pöördeid, kuni selle osani roolist. Ainult viimane tükk, servas, pöörab järsult. |
| 196 | Name of a setting row (about 30 Latin characters) | Full stick turns the wheel | Täis kang pöörab rooli |
| 197 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the wheel turns with the stick pushed all the way. 100 % = full lock. | Kui palju rool pöörab, kui kang on lõpuni lükatud. 100 % = rool lõpuni. |
| 198 | Name of a setting row (about 30 Latin characters) | Wheel follows the stick | Rool järgib kangi |
| 199 | Value or choice (about 18 Latin characters) | at once | kohe |
| 200 | Value or choice (about 18 Latin characters) | %.3g s | %.3g s |
| 201 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Smoothing while you hold the stick: how quickly the wheel goes where the stick points. More = smoother, calmer. | Silumine, kui hoiad kangi: kui kiiresti rool kangi järgib. Rohkem = sujuvam, rahulikum. |
| 202 | Name of a setting row (about 30 Latin characters) | Let go: back to the middle | Lase lahti: tagasi keskele |
| 203 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Let go of the stick: how the wheel comes back to straight. At once = at the wheel speed below. More = a slow glide. | Lase kang lahti: kuidas rool naaseb otse. Kohe = alloleva rooli kiirusega. Rohkem = aeglane libisemine. |
| 204 | Name of a setting row (about 30 Latin characters) | Wheel speed at most | Rooli max kiirus |
| 205 | Value or choice (about 18 Latin characters) | no limit | pole piiri |
| 206 | Value or choice (about 18 Latin characters) | %.2g s to full | %.2g s lõpuni |
| 207 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The fastest the wheel turns (full push; a small push turns it slower): seconds from straight to full lock. | Kiireim rooli pööre (täislüke; väike lüke on aeglasem): sekundit otsest rooli lõpuni. |
| 208 | Name of a setting row (about 30 Latin characters) | At 90 km/h: steering | 90 km/h: roolimine |
| 209 | Button (short: about 10 Latin characters) | On | Sees |
| 210 | Value or choice (about 18 Latin characters) | (off) | (väljas) |
| 211 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | On: at speed the wheel turns less, for calm highway driving. The % = how much at 90 km/h. | Sees: kiirusel pöörab rool vähem, rahulikuks maanteesõiduks. % = kui palju 90 km/h juures. |
| 212 | Status / hint line at the bottom (one line, about 55 Latin characters) | Driving off: no steering, gas or brake. | Sõit väljas: roolimist, gaasi ega pidurit pole. |
| 213 | Status / hint line at the bottom (one line, about 55 Latin characters) | Wheel: hold the grip(s) and turn your hands. Let go = straight. | Rool: hoia gripp(e) ja pööra käsi. Lase lahti = otse. |
| 214 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point: point left / right. On a panel the wheel holds. | Osuta: osuta vasak / parem. Paneelil, mida rool hoiab. |
| 215 | Status / hint line at the bottom (one line, about 55 Latin characters) | Most of the stick = small turns. All the way = hard turns. | Suurem osa kangist = väikesed pöörded. Lõpuni = järsud pöörded. |
| 216 | Button (short: about 10 Latin characters) | Sure? Reset | Lähtesta? |
| 217 | Button (short: about 10 Latin characters) | Reset tab | Lähtesta |
| 218 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Puts this tab back to the default settings. Press twice to be sure. | Taastab selle vahelehe vaikeseaded. Vajuta kaks korda kindluse mõttes. |
| 219 | Value or choice (about 18 Latin characters) | Set: %s + %s = %s | Määratud: %s + %s = %s |
| 220 | Value or choice (about 18 Latin characters) | Set: %s = %s | Määratud: %s = %s |
| 221 | Status / hint line at the bottom (one line, about 55 Latin characters) | Press again: this tab goes back to the defaults. | Vajuta uuesti: see vaheleht läheb tagasi vaikeseadetele. |
| 222 | Status / hint line at the bottom (one line, about 55 Latin characters) | Back to the defaults. Saved. | Tagasi vaikeseadetele. Salvestatud. |
| 223 | Status / hint line at the bottom (one line, about 55 Latin characters) | Saved. | Salvestatud. |
| 224 | Status / hint line at the bottom (one line, about 55 Latin characters) | Choose that modifier button first (the buttons above). | Vali esmalt see modifikaatorinupp (nupud üleval). |
| 225 | Status / hint line at the bottom (one line, about 55 Latin characters) | Steering with the wheel: those grips hold it (Driving tab). | Roolimine rooliga: need gripid hoiavad seda (Sõit-vaheleht). |
| 226 | Value or choice (about 18 Latin characters) | %s holds the wheel (Driving tab). | %s hoiab rooli (Sõit-vaheleht). |
| 227 | Value or choice (about 18 Latin characters) | %s is modifier 1. | %s on modifikaator 1. |
| 228 | Value or choice (about 18 Latin characters) | %s is modifier 2. | %s on modifikaator 2. |
| 229 | Value or choice (about 18 Latin characters) | %s is the air mouse button (Mouse tab). | %s on õhuhiire nupp (Hiir-vaheleht). |
| 230 | Value or choice (about 18 Latin characters) | With these modifiers, %s opens the menu. | Nende modifikaatoritega avab %s menüü. |
| 231 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick moves the mouse (Mouse tab). | See kang liigutab hiirt (Hiir-vaheleht). |
| 232 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick steers (Driving tab). | See kang roolib (Sõit-vaheleht). |
| 233 | Status / hint line at the bottom (one line, about 55 Latin characters) | Could not save config.ini (the change works until the game ends). | config.ini salvestamine ebaõnnestus (muudatus kehtib mängu lõpuni). |
