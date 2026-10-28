# Review request: Suomi (fi_fi) translation of a VR game mod menu

You are a native Suomi speaker and a PC gaming / VR expert. Please review this translation.

**Context.** This is the in-headset settings menu and on-screen keyboard of a VR controller mod for the truck simulators
Euro Truck Simulator 2 and American Truck Simulator: it maps the VR controllers' buttons, sticks and pointing to keyboard
keys, the mouse and steering. The menu floats in front of the player; its boxes are small. All Suomi texts below were
written by an AI and may be wrong, unnatural or too long.

**Check every line for:**
1. Wrong meaning compared with the English.
2. Unnatural wording: use what Suomi PC gamers, VR players and game options menus normally say.
3. Consistency: the same English term should get the same Suomi term everywhere (grip, trigger, stick, modifier, air mouse,
   panel, see-through, dead zone, full lock).
4. Length: the "Where" column gives the room (Latin characters; a CJK character counts as 2). Too long = it gets smaller or
   cut off with "...". Key names on the keyboard: what keyboards in your country have printed (e.g. German: Strg, Entf).

**Rules your corrections must keep (or the mod falls back to English for that line):**
- Keep every placeholder exactly and in the same order: %s %d %.0f %.1f %.2f %.3g %.2g ...
- `%%` is a literal percent sign: keep it as `%%`. Never add a single `%`.
- Do not translate: VR, ETS2, ATS, config.ini, the button letters X / Y / A / B, units px, km/h.

**Answer format.** Only list the lines you would change, as a table:

| # | Current Suomi text | Corrected Suomi text | Reason (short, in English) |
|---|---|---|---|

Then one line: how many lines you checked and an overall quality rating (1-10). Do not repeat lines that are fine.

| # | Where | English | Suomi |
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
| 18 | Key on the VR keyboard (very short: about 6 Latin characters) | Space | Väli |
| 19 | Controller button name (about 16 Latin characters) | Left trigger | V. liipaisin |
| 20 | Controller button name (about 16 Latin characters) | Right trigger | O. liipaisin |
| 21 | Controller button name (about 16 Latin characters) | Left grip | V. grip |
| 22 | Controller button name (about 16 Latin characters) | Right grip | O. grip |
| 23 | Controller button name (about 16 Latin characters) | Left stick click | V. tatti (paina) |
| 24 | Controller button name (about 16 Latin characters) | Right stick click | O. tatti (paina) |
| 25 | Controller button name (about 16 Latin characters) | Left menu | V. valikko |
| 26 | Controller button name (about 16 Latin characters) | Left stick up | V. tatti ylös |
| 27 | Controller button name (about 16 Latin characters) | Left stick down | V. tatti alas |
| 28 | Controller button name (about 16 Latin characters) | Left stick left | V. tatti vasen |
| 29 | Controller button name (about 16 Latin characters) | Left stick right | V. tatti oikea |
| 30 | Controller button name (about 16 Latin characters) | Right stick up | O. tatti ylös |
| 31 | Controller button name (about 16 Latin characters) | Right stick down | O. tatti alas |
| 32 | Controller button name (about 16 Latin characters) | Right stick left | O. tatti vasen |
| 33 | Controller button name (about 16 Latin characters) | Right stick right | O. tatti oikea |
| 34 | Value or choice (about 18 Latin characters) | Mouse left | Hiiri vasen |
| 35 | Value or choice (about 18 Latin characters) | Mouse right | Hiiri oikea |
| 36 | Value or choice (about 18 Latin characters) | Mouse middle | Hiiri keski |
| 37 | Button (short: about 10 Latin characters) | Wheel up | Rulla ylös |
| 38 | Value or choice (about 18 Latin characters) | Wheel down | Rulla alas |
| 39 | Button (short: about 10 Latin characters) | Gas | Kaasu |
| 40 | Button (short: about 10 Latin characters) | Brake | Jarru |
| 41 | Button (short: about 10 Latin characters) | Keyboard | Näppäim. |
| 42 | Value or choice (about 18 Latin characters) | Num %s | Num %s |
| 43 | Title bar (one line, about 40 Latin characters) | Set %s: pick a key | Aseta %s: valitse näppäin |
| 44 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu smaller. | Tee tästä valikosta pienempi. |
| 45 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu bigger. | Tee tästä valikosta suurempi. |
| 46 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show or hide the VR keyboard. | Näytä tai piilota VR-näppäimistö. |
| 47 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Close this menu. To open it again: hold %s. | Sulje tämä valikko. Avaa uudelleen: pidä %s. |
| 48 | Tab name at the top of the menu (about 11 Latin characters) | Buttons | Painikkeet |
| 49 | Tab name at the top of the menu (about 11 Latin characters) | Mouse | Hiiri |
| 50 | Tab name at the top of the menu (about 11 Latin characters) | Panels | Paneelit |
| 51 | Tab name at the top of the menu (about 11 Latin characters) | Look | Ulkoasu |
| 52 | Tab name at the top of the menu (about 11 Latin characters) | Driving | Ajo |
| 53 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Choose what each controller button does. | Valitse, mitä kukin ohjaimen painike tekee. |
| 54 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The stick mouse and the air mouse (point with the controller). | Tattihiiri ja ilmahiiri (osoita ohjaimella). |
| 55 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Size and place of this menu and the keyboard. | Tämän valikon ja näppäimistön koko ja paikka. |
| 56 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Colours, how see-through the panels are, pointer smoothing and the language. | Värit, paneelien läpinäkyvyys, osoittimen pehmennys ja kieli. |
| 57 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Steering, gas and brake: how the truck's wheel follows your stick. | Ohjaus, kaasu ja jarru: miten rekan ratti seuraa tattia. |
| 58 | Button (short: about 10 Latin characters) | Normal | Normaali |
| 59 | Button (short: about 10 Latin characters) | Mod 1 | Mod 1 |
| 60 | Button (short: about 10 Latin characters) | Mod 2 | Mod 2 |
| 61 | Button (short: about 10 Latin characters) | Both | Molemmat |
| 62 | Value or choice (about 18 Latin characters) | off | pois |
| 63 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does normally. | Näytä ja aseta, mitä kukin painike tekee normaalisti. |
| 64 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 1 (%s). | Näytä ja aseta, mitä kukin painike tekee, kun pidät muokkainta 1 (%s). |
| 65 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 2 (%s). | Näytä ja aseta, mitä kukin painike tekee, kun pidät muokkainta 2 (%s). |
| 66 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold BOTH modifiers. | Näytä ja aseta, mitä kukin painike tekee, kun pidät MOLEMPIA muokkaimia. |
| 67 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 1: hold it, and other buttons do their Mod 1 job. Press here to choose another button. | Muokkain 1: pidä pohjassa, niin muut painikkeet tekevät Mod 1 -työnsä. Paina tästä valitaksesi toisen painikkeen. |
| 68 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 2: hold it, and other buttons do their Mod 2 job. Hold both modifiers for the Both jobs. Press here to choose another button. | Muokkain 2: pidä pohjassa, niin muut tekevät Mod 2 -työnsä. Pidä molempia Molemmat-töitä varten. Paina tästä valitaksesi toisen painikkeen. |
| 69 | Value or choice (about 18 Latin characters) | the wheel | ratti |
| 70 | Value or choice (about 18 Latin characters) | (grips = the wheel) | (gripit = ratti) |
| 71 | Value or choice (about 18 Latin characters) | modifier 1 | muokkain 1 |
| 72 | Value or choice (about 18 Latin characters) | modifier 2 | muokkain 2 |
| 73 | Value or choice (about 18 Latin characters) | air mouse | ilmahiiri |
| 74 | Value or choice (about 18 Latin characters) | moves the mouse | liikuttaa hiirtä |
| 75 | Value or choice (about 18 Latin characters) | steers | ohjaa |
| 76 | Value or choice (about 18 Latin characters) | opens the menu | avaa valikon |
| 77 | Value or choice (about 18 Latin characters) | as normal | kuten normaalisti |
| 78 | Value or choice (about 18 Latin characters) | (+menu) | (+valikko) |
| 79 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: %s (set elsewhere). | %s + %s: %s (asetettu muualla). |
| 80 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: %s (set elsewhere). | %s: %s (asetettu muualla). |
| 81 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s + %s: nyt %s. Osoita tähän, vedä liipaisimesta, valitse sitten näppäin tai alla oleva vaihtoehto. |
| 82 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s: nyt %s. Osoita tähän, vedä liipaisimesta, valitse sitten näppäin tai alla oleva vaihtoehto. |
| 83 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Also part of the menu's open buttons. | On myös osa valikon avauspainikkeita. |
| 84 | Button (short: about 10 Latin characters) | Mouse L | Hiiri V |
| 85 | Button (short: about 10 Latin characters) | Mouse R | Hiiri O |
| 86 | Button (short: about 10 Latin characters) | Mouse M | Hiiri M |
| 87 | Button (short: about 10 Latin characters) | Wheel dn | Rulla alas |
| 88 | Button (short: about 10 Latin characters) | pick\|Keyboard | Näppäim. |
| 89 | Button (short: about 10 Latin characters) | None | Ei mitään |
| 90 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Left mouse click. | Hiiren vasen painallus. |
| 91 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Right mouse click. | Hiiren oikea painallus. |
| 92 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Middle mouse click. | Hiiren keskipainallus. |
| 93 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel up (one step per press). | Hiiren rulla ylös (yksi askel per painallus). |
| 94 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel down (one step per press). | Hiiren rulla alas (yksi askel per painallus). |
| 95 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Gas pedal: press harder = more gas. | Kaasupoljin: paina kovempaa = enemmän kaasua. |
| 96 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Brake: press harder = more brake. | Jarru: paina kovempaa = enemmän jarrua. |
| 97 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Shows / hides the VR keyboard. | Näyttää / piilottaa VR-näppäimistön. |
| 98 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Nothing. | Ei mitään. |
| 99 | Button (short: about 10 Latin characters) | Cancel | Peruuta |
| 100 | Status / hint line at the bottom (one line, about 55 Latin characters) | Hold %s first, then the button. | Pidä %s ensin, sitten painike. |
| 101 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point at a button, pull the trigger, pick its key. | Osoita painiketta, vedä liipaisimesta, valitse sen näppäin. |
| 102 | Name of a setting row (about 30 Latin characters) | Stick that moves the mouse | Hiirtä liikuttava tatti |
| 103 | Value or choice (about 18 Latin characters) | Left | Vasen |
| 104 | Value or choice (about 18 Latin characters) | Right | Oikea |
| 105 | Button (short: about 10 Latin characters) | Off | Pois |
| 106 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick moves the mouse pointer (for the game's menus and map). Off = no stick mouse. | Kumpi tatti liikuttaa osoitinta (valikot ja kartta). Pois = ei tattihiirtä. |
| 107 | Name of a setting row (about 30 Latin characters) | Stick mouse speed | Tattihiiren nopeus |
| 108 | Value or choice (about 18 Latin characters) | %.0f px/s | %.0f px/s |
| 109 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How fast the pointer moves with the stick pushed all the way, in pixels per second. | Kuinka nopeasti osoitin liikkuu tatti ääriasennossa, pikseliä sekunnissa. |
| 110 | Name of a setting row (about 30 Latin characters) | Stick dead zone | Tatin kuollut alue |
| 111 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the pointer moves. More = no creeping pointer. | Kuinka pitkälle tattia pitää työntää ennen kuin osoitin liikkuu. Enemmän = ei ryömintää. |
| 112 | Name of a setting row (about 30 Latin characters) | Air mouse button | Ilmahiiren painike |
| 113 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Hold this button and turn the controller: the pointer follows, like a laser pointer. Off = no air mouse. | Pidä tätä painiketta ja käännä ohjainta: osoitin seuraa kuin laserosoitin. Pois = ei ilmahiirtä. |
| 114 | Name of a setting row (about 30 Latin characters) | Air mouse works while | Ilmahiiri toimii |
| 115 | Value or choice (about 18 Latin characters) | Button held | Painike pohjassa |
| 116 | Value or choice (about 18 Latin characters) | Toggled on | Päälle kytketty |
| 117 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Button held: only while you hold the button. Toggled on: one press switches it on, the next press off. | Painike pohjassa: vain kun pidät painiketta. Päälle kytketty: yksi painallus päälle, seuraava pois. |
| 118 | Name of a setting row (about 30 Latin characters) | Air mouse speed | Ilmahiiren nopeus |
| 119 | Value or choice (about 18 Latin characters) | %.0f px/deg | %.0f px/aste |
| 120 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the pointer moves when you turn the controller, in pixels per degree. | Kuinka pitkälle osoitin liikkuu, kun käännät ohjainta, pikseliä astetta kohti. |
| 121 | Status / hint line at the bottom (one line, about 55 Latin characters) | Changes work at once and are saved. | Muutokset toimivat heti ja tallennetaan. |
| 122 | Name of a setting row (about 30 Latin characters) | Menu size | Valikon koko |
| 123 | Value or choice (about 18 Latin characters) | %.2f m wide | %.2f m leveä |
| 124 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big this menu is (its width in metres). | Kuinka suuri tämä valikko on (leveys metreinä). |
| 125 | Name of a setting row (about 30 Latin characters) | Menu distance | Valikon etäisyys |
| 126 | Value or choice (about 18 Latin characters) | %.2f m | %.2f m |
| 127 | Button (short: about 10 Latin characters) | Closer | Lähemmäs |
| 128 | Button (short: about 10 Latin characters) | Farther | Kauemmas |
| 129 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far this menu is from your head. | Kuinka kaukana tämä valikko on päästäsi. |
| 130 | Name of a setting row (about 30 Latin characters) | Keyboard size | Näppäimistön koko |
| 131 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big the VR keyboard is (its width in metres). | Kuinka suuri VR-näppäimistö on (leveys metreinä). |
| 132 | Name of a setting row (about 30 Latin characters) | Keyboard distance | Näppäimistön etäisyys |
| 133 | Value or choice (about 18 Latin characters) | (hidden) | (piilossa) |
| 134 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the VR keyboard is from your head. | Kuinka kaukana VR-näppäimistö on päästäsi. |
| 135 | Button (short: about 10 Latin characters) | Show | Näytä |
| 136 | Button (short: about 10 Latin characters) | Hide | Piilota |
| 137 | Button (short: about 10 Latin characters) | Full | Täysi |
| 138 | Button (short: about 10 Latin characters) | Compact | Tiivis |
| 139 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show / hide the VR keyboard. Full = every key; Compact = only the main keys (narrower). | Näytä / piilota VR-näppäimistö. Täysi = kaikki näppäimet; Tiivis = vain päänäppäimet (kapeampi). |
| 140 | Name of a setting row (about 30 Latin characters) | Bring the menu and the keyboard in front of me | Tuo valikko & näppäimistö eteeni |
| 141 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Moves the menu and the keyboard to where you look now. | Siirtää valikon ja näppäimistön sinne, minne katsot nyt. |
| 142 | Status / hint line at the bottom (one line, about 55 Latin characters) | Menu open / close: hold %s for %.1f s | Valikko auki / kiinni: pidä %s %.1f s |
| 143 | Status / hint line at the bottom (one line, about 55 Latin characters) | Move: point at a panel, hold the grip, move. Its stick: farther / closer. | Siirto: osoita paneelia, pidä grip, siirrä. Tatti: kauemmas / lähemmäs. |
| 144 | Status / hint line at the bottom (one line, about 55 Latin characters) | The keyboard stays when the menu closes (its x hides it). | Näppäimistö jää, kun valikko sulkeutuu (sen x piilottaa sen). |
| 145 | Button (short: about 10 Latin characters) | Dark | Tumma |
| 146 | Button (short: about 10 Latin characters) | Blue | Sininen |
| 147 | Button (short: about 10 Latin characters) | Grey | Harmaa |
| 148 | Button (short: about 10 Latin characters) | Light | Vaalea |
| 149 | Button (short: about 10 Latin characters) | Black | Musta |
| 150 | Button (short: about 10 Latin characters) | Green | Vihreä |
| 151 | Name of a setting row (about 30 Latin characters) | Colours | Värit |
| 152 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The colours of the menu and the keyboard. | Valikon ja näppäimistön värit. |
| 153 | Name of a setting row (about 30 Latin characters) | Menu see-through | Valikon läpinäkyvyys |
| 154 | Value or choice (about 18 Latin characters) | %d %% solid | %d %% peittävä |
| 155 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid this menu is. - = more see-through, + = more solid. | Kuinka peittävä tämä valikko on. - = läpinäkyvämpi, + = peittävämpi. |
| 156 | Name of a setting row (about 30 Latin characters) | Keyboard see-through | Näppäimistön läpinäkyvyys |
| 157 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid the VR keyboard is. - = more see-through, + = more solid. | Kuinka peittävä VR-näppäimistö on. - = läpinäkyvämpi, + = peittävämpi. |
| 158 | Name of a setting row (about 30 Latin characters) | Pointer smoothing | Osoittimen pehmennys |
| 159 | Button (short: about 10 Latin characters) | Low | Matala |
| 160 | Button (short: about 10 Latin characters) | Medium | Keskitaso |
| 161 | Button (short: about 10 Latin characters) | High | Korkea |
| 162 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Calms a shaky hand: the menu pointer and the air mouse. Higher = steadier, but it follows a bit later. | Rauhoittaa tärisevää kättä: valikon osoitin ja ilmahiiri. Korkeampi = vakaampi, mutta seuraa hieman myöhemmin. |
| 163 | Value or choice (about 18 Latin characters) | %s (no font) | %s (ei fonttia) |
| 164 | Name of a setting row (about 30 Latin characters) | Language | Kieli |
| 165 | Value or choice (about 18 Latin characters) | Auto: %s | Auto: %s |
| 166 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The menu's language. Auto = the language the game is set to. | Valikon kieli. Auto = kieli, johon peli on asetettu. |
| 167 | Name of a setting row (about 30 Latin characters) | Keyboard letters | Näppäinasettelu |
| 168 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The letters on the VR keyboard, as printed in that country. Auto = as the menu. A key always presses the same key in the game. | VR-näppäimistön kirjaimet, kuten maassa painettuna. Auto = kuten valikko. Näppäin painaa pelissä aina samaa näppäintä. |
| 169 | Status / hint line at the bottom (one line, about 55 Latin characters) | - = more see-through, + = more solid. | - = läpinäkyvämpi, + = peittävämpi. |
| 170 | Name of a setting row (about 30 Latin characters) | Steer with | Ohjaustapa |
| 171 | Button (short: about 10 Latin characters) | Stick | Tatti |
| 172 | Button (short: about 10 Latin characters) | Wheel | Ratti |
| 173 | Button (short: about 10 Latin characters) | Point | Osoita |
| 174 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick: the stick steers. Wheel: hold the grip(s) and turn your hands like a steering wheel. Point: point a controller left / right (on a panel the wheel holds). Off: no driving. | Tatti: tatti ohjaa. Ratti: pidä gripi(t) ja käännä käsiä kuin rattia. Osoita: osoita ohjaimella vasen / oikea (paneelilla, jota ratti pitää). Pois: ei ajoa. |
| 175 | Name of a setting row (about 30 Latin characters) | Hands turn for full lock | Käsien käännös ääriasentoon |
| 176 | Name of a setting row (about 30 Latin characters) | Point for full lock | Osoitus ääriasentoon |
| 177 | Value or choice (about 18 Latin characters) | %.0f deg | %.0f astetta |
| 178 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you turn your hands (like a wheel) for full lock. Less = quicker, more = calmer. | Kuinka paljon käännät käsiä (kuin rattia) ääriasentoon. Vähemmän = nopeampi, enemmän = rauhallisempi. |
| 179 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you point left / right for full lock. Less = quicker, more = calmer. | Kuinka pitkälle osoitat vasen / oikea ääriasentoon. Vähemmän = nopeampi, enemmän = rauhallisempi. |
| 180 | Button (short: about 10 Latin characters) | Stick on | Tatti päällä |
| 181 | Button (short: about 10 Latin characters) | Stick off | Tatti pois |
| 182 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick on: the stick steers too (it wins while you push it). Stick off: only your hands steer, and the stick's left / right are normal buttons again. | Tatti päällä: tatti ohjaa myös (se voittaa, kun työnnät sitä). Tatti pois: vain kädet ohjaavat, ja tatin vasen / oikea ovat taas normaaleja painikkeita. |
| 183 | Name of a setting row (about 30 Latin characters) | Hold the wheel with | Pidä rattia |
| 184 | Value or choice (about 18 Latin characters) | Either | Kumpi vain |
| 185 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Both: both grips together (one grip alone keeps its own job). Left / Right / Either: one grip is enough -- that grip is then only the wheel (no modifier, no air mouse, no key on it). | Molemmat: molemmat gripit yhdessä (yksi grip yksin säilyttää oman työnsä). Vasen / Oikea / Kumpi vain: yksi grip riittää -- se grip on silloin vain ratti (ei muokkainta, ei ilmahiirtä, ei näppäintä). |
| 186 | Name of a setting row (about 30 Latin characters) | Hand that points | Osoittava käsi |
| 187 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The controller you point with. The other hand stays free for the keyboard and the menu. | Ohjain, jolla osoitat. Toinen käsi jää vapaaksi näppäimistölle ja valikolle. |
| 188 | Name of a setting row (about 30 Latin characters) | Stick that steers | Ohjaava tatti |
| 189 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick steers the truck (left / right). Its up / down stay free for other buttons. | Kumpi tatti ohjaa rekkaa (vasen / oikea). Ylös / alas jäävät vapaaksi muille painikkeille. |
| 190 | Name of a setting row (about 30 Latin characters) | Steering dead zone | Ohjauksen kuollut alue |
| 191 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the wheel turns. More = a calm middle, less = it reacts sooner. | Kuinka pitkälle tattia pitää työntää ennen kuin ratti kääntyy. Enemmän = rauhallinen keskikohta, vähemmän = reagoi aiemmin. |
| 192 | Name of a setting row (about 30 Latin characters) | Small turns (most of the stick) | Pienet käännökset (tatista) |
| 193 | Value or choice (about 18 Latin characters) | even (off) | tasainen (pois) |
| 194 | Value or choice (about 18 Latin characters) | up to %d %% | enint. %d %% |
| 195 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Most of the stick (the first %d %% of a push) makes small, careful turns, up to this much of the wheel. Only the last bit, at the edge, turns hard. | Suurin osa tatista (työnnön ensimmäiset %d %%) tekee pieniä, varovaisia käännöksiä, tähän osaan rattia asti. Vain viimeinen pätkä, reunalla, kääntää kovaa. |
| 196 | Name of a setting row (about 30 Latin characters) | Full stick turns the wheel | Täysi tatti kääntää rattia |
| 197 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the wheel turns with the stick pushed all the way. 100 % = full lock. | Kuinka paljon ratti kääntyy tatti ääriasennossa. 100 % = ääriasento. |
| 198 | Name of a setting row (about 30 Latin characters) | Wheel follows the stick | Ratti seuraa tattia |
| 199 | Value or choice (about 18 Latin characters) | at once | heti |
| 200 | Value or choice (about 18 Latin characters) | %.3g s | %.3g s |
| 201 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Smoothing while you hold the stick: how quickly the wheel goes where the stick points. More = smoother, calmer. | Pehmennys, kun pidät tattia: kuinka nopeasti ratti seuraa tattia. Enemmän = pehmeämpi, rauhallisempi. |
| 202 | Name of a setting row (about 30 Latin characters) | Let go: back to the middle | Irrota: takaisin keskelle |
| 203 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Let go of the stick: how the wheel comes back to straight. At once = at the wheel speed below. More = a slow glide. | Irrota tatti: miten ratti palaa suoraan. Heti = alla olevalla rattinopeudella. Enemmän = hidas liuku. |
| 204 | Name of a setting row (about 30 Latin characters) | Wheel speed at most | Ratin enimmäisnopeus |
| 205 | Value or choice (about 18 Latin characters) | no limit | ei rajaa |
| 206 | Value or choice (about 18 Latin characters) | %.2g s to full | %.2g s täyteen |
| 207 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The fastest the wheel turns (full push; a small push turns it slower): seconds from straight to full lock. | Ratin nopein kääntö (täysi työntö; pieni työntö on hitaampi): sekuntia suorasta ääriasentoon. |
| 208 | Name of a setting row (about 30 Latin characters) | At 90 km/h: steering | 90 km/h: ohjaus |
| 209 | Button (short: about 10 Latin characters) | On | Päällä |
| 210 | Value or choice (about 18 Latin characters) | (off) | (pois) |
| 211 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | On: at speed the wheel turns less, for calm highway driving. The % = how much at 90 km/h. | Päällä: vauhdissa ratti kääntyy vähemmän, rauhalliseen moottoritieajoon. % = kuinka paljon 90 km/h. |
| 212 | Status / hint line at the bottom (one line, about 55 Latin characters) | Driving off: no steering, gas or brake. | Ajo pois: ei ohjausta, kaasua tai jarrua. |
| 213 | Status / hint line at the bottom (one line, about 55 Latin characters) | Wheel: hold the grip(s) and turn your hands. Let go = straight. | Ratti: pidä gripi(t) ja käännä käsiä. Irrota = suora. |
| 214 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point: point left / right. On a panel the wheel holds. | Osoita: osoita vasen / oikea. Paneelilla, jota ratti pitää. |
| 215 | Status / hint line at the bottom (one line, about 55 Latin characters) | Most of the stick = small turns. All the way = hard turns. | Suurin osa tatista = pienet käännökset. Ääriasento = kovat käännökset. |
| 216 | Button (short: about 10 Latin characters) | Sure? Reset | Varma? Nollaa |
| 217 | Button (short: about 10 Latin characters) | Reset tab | Nollaa väl. |
| 218 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Puts this tab back to the default settings. Press twice to be sure. | Palauttaa tämän välilehden oletusasetuksiin. Paina kahdesti varmistaaksesi. |
| 219 | Value or choice (about 18 Latin characters) | Set: %s + %s = %s | Asetettu: %s + %s = %s |
| 220 | Value or choice (about 18 Latin characters) | Set: %s = %s | Asetettu: %s = %s |
| 221 | Status / hint line at the bottom (one line, about 55 Latin characters) | Press again: this tab goes back to the defaults. | Paina uudelleen: tämä välilehti palaa oletuksiin. |
| 222 | Status / hint line at the bottom (one line, about 55 Latin characters) | Back to the defaults. Saved. | Takaisin oletuksiin. Tallennettu. |
| 223 | Status / hint line at the bottom (one line, about 55 Latin characters) | Saved. | Tallennettu. |
| 224 | Status / hint line at the bottom (one line, about 55 Latin characters) | Choose that modifier button first (the buttons above). | Valitse ensin se muokkainpainike (painikkeet yllä). |
| 225 | Status / hint line at the bottom (one line, about 55 Latin characters) | Steering with the wheel: those grips hold it (Driving tab). | Ohjaus ratilla: ne gripit pitävät sitä (Ajo-välilehti). |
| 226 | Value or choice (about 18 Latin characters) | %s holds the wheel (Driving tab). | %s pitää rattia (Ajo-välilehti). |
| 227 | Value or choice (about 18 Latin characters) | %s is modifier 1. | %s on muokkain 1. |
| 228 | Value or choice (about 18 Latin characters) | %s is modifier 2. | %s on muokkain 2. |
| 229 | Value or choice (about 18 Latin characters) | %s is the air mouse button (Mouse tab). | %s on ilmahiiren painike (Hiiri-välilehti). |
| 230 | Value or choice (about 18 Latin characters) | With these modifiers, %s opens the menu. | Näillä muokkaimilla %s avaa valikon. |
| 231 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick moves the mouse (Mouse tab). | Se tatti liikuttaa hiirtä (Hiiri-välilehti). |
| 232 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick steers (Driving tab). | Se tatti ohjaa (Ajo-välilehti). |
| 233 | Status / hint line at the bottom (one line, about 55 Latin characters) | Could not save config.ini (the change works until the game ends). | config.ini-tallennus epäonnistui (muutos toimii pelin loppuun asti). |
