# Review request: Galego (gl_es) translation of a VR game mod menu

You are a native Galego speaker and a PC gaming / VR expert. Please review this translation.

**Context.** This is the in-headset settings menu and on-screen keyboard of a VR controller mod for the truck simulators
Euro Truck Simulator 2 and American Truck Simulator: it maps the VR controllers' buttons, sticks and pointing to keyboard
keys, the mouse and steering. The menu floats in front of the player; its boxes are small. All Galego texts below were
written by an AI and may be wrong, unnatural or too long.

**Check every line for:**
1. Wrong meaning compared with the English.
2. Unnatural wording: use what Galego PC gamers, VR players and game options menus normally say.
3. Consistency: the same English term should get the same Galego term everywhere (grip, trigger, stick, modifier, air mouse,
   panel, see-through, dead zone, full lock).
4. Length: the "Where" column gives the room (Latin characters; a CJK character counts as 2). Too long = it gets smaller or
   cut off with "...". Key names on the keyboard: what keyboards in your country have printed (e.g. German: Strg, Entf).

**Rules your corrections must keep (or the mod falls back to English for that line):**
- Keep every placeholder exactly and in the same order: %s %d %.0f %.1f %.2f %.3g %.2g ...
- `%%` is a literal percent sign: keep it as `%%`. Never add a single `%`.
- Do not translate: VR, ETS2, ATS, config.ini, the button letters X / Y / A / B, units px, km/h.

**Answer format.** Only list the lines you would change, as a table:

| # | Current Galego text | Corrected Galego text | Reason (short, in English) |
|---|---|---|---|

Then one line: how many lines you checked and an overall quality rating (1-10). Do not repeat lines that are fine.

| # | Where | English | Galego |
|---|---|---|---|
| 1 | Key on the VR keyboard (very short: about 6 Latin characters) | Esc | Esc |
| 2 | Key on the VR keyboard (very short: about 6 Latin characters) | PrtSc | Impr |
| 3 | Key on the VR keyboard (very short: about 6 Latin characters) | ScrLk | Despl |
| 4 | Key on the VR keyboard (very short: about 6 Latin characters) | Pause | Pausa |
| 5 | Key on the VR keyboard (very short: about 6 Latin characters) | Backspace | ← |
| 6 | Key on the VR keyboard (very short: about 6 Latin characters) | Ins | Insert |
| 7 | Key on the VR keyboard (very short: about 6 Latin characters) | Home | Inicio |
| 8 | Key on the VR keyboard (very short: about 6 Latin characters) | PgUp | Re Páx |
| 9 | Key on the VR keyboard (very short: about 6 Latin characters) | Tab | Tab |
| 10 | Key on the VR keyboard (very short: about 6 Latin characters) | Enter | Intro |
| 11 | Key on the VR keyboard (very short: about 6 Latin characters) | Del | Supr |
| 12 | Key on the VR keyboard (very short: about 6 Latin characters) | End | Fin |
| 13 | Key on the VR keyboard (very short: about 6 Latin characters) | PgDn | Av Páx |
| 14 | Key on the VR keyboard (very short: about 6 Latin characters) | Caps | Bloq M |
| 15 | Key on the VR keyboard (very short: about 6 Latin characters) | Shift | Maiús |
| 16 | Key on the VR keyboard (very short: about 6 Latin characters) | Ctrl | Ctrl |
| 17 | Key on the VR keyboard (very short: about 6 Latin characters) | Alt | Alt |
| 18 | Key on the VR keyboard (very short: about 6 Latin characters) | Space | Espazo |
| 19 | Controller button name (about 16 Latin characters) | Left trigger | Gatillo esq. |
| 20 | Controller button name (about 16 Latin characters) | Right trigger | Gatillo der. |
| 21 | Controller button name (about 16 Latin characters) | Left grip | Grip esq. |
| 22 | Controller button name (about 16 Latin characters) | Right grip | Grip der. |
| 23 | Controller button name (about 16 Latin characters) | Left stick click | Clic stick esq. |
| 24 | Controller button name (about 16 Latin characters) | Right stick click | Clic stick der. |
| 25 | Controller button name (about 16 Latin characters) | Left menu | Menú esq. |
| 26 | Controller button name (about 16 Latin characters) | Left stick up | Stick esq. ↑ |
| 27 | Controller button name (about 16 Latin characters) | Left stick down | Stick esq. ↓ |
| 28 | Controller button name (about 16 Latin characters) | Left stick left | Stick esq. ← |
| 29 | Controller button name (about 16 Latin characters) | Left stick right | Stick esq. → |
| 30 | Controller button name (about 16 Latin characters) | Right stick up | Stick der. ↑ |
| 31 | Controller button name (about 16 Latin characters) | Right stick down | Stick der. ↓ |
| 32 | Controller button name (about 16 Latin characters) | Right stick left | Stick der. ← |
| 33 | Controller button name (about 16 Latin characters) | Right stick right | Stick der. → |
| 34 | Value or choice (about 18 Latin characters) | Mouse left | Rato esq. |
| 35 | Value or choice (about 18 Latin characters) | Mouse right | Rato der. |
| 36 | Value or choice (about 18 Latin characters) | Mouse middle | Rato central |
| 37 | Button (short: about 10 Latin characters) | Wheel up | Roda ↑ |
| 38 | Value or choice (about 18 Latin characters) | Wheel down | Roda ↓ |
| 39 | Button (short: about 10 Latin characters) | Gas | Acelerar |
| 40 | Button (short: about 10 Latin characters) | Brake | Frear |
| 41 | Button (short: about 10 Latin characters) | Keyboard | Teclado |
| 42 | Value or choice (about 18 Latin characters) | Num %s | Num %s |
| 43 | Title bar (one line, about 40 Latin characters) | Set %s: pick a key | Asignar %s: elixe unha tecla |
| 44 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu smaller. | Fai este menú máis pequeno. |
| 45 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu bigger. | Fai este menú máis grande. |
| 46 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show or hide the VR keyboard. | Mostra ou agocha o teclado VR. |
| 47 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Close this menu. To open it again: hold %s. | Pecha este menú. Para abrilo de novo: mantén %s. |
| 48 | Tab name at the top of the menu (about 11 Latin characters) | Buttons | Botóns |
| 49 | Tab name at the top of the menu (about 11 Latin characters) | Mouse | Rato |
| 50 | Tab name at the top of the menu (about 11 Latin characters) | Panels | Paneis |
| 51 | Tab name at the top of the menu (about 11 Latin characters) | Look | Aspecto |
| 52 | Tab name at the top of the menu (about 11 Latin characters) | Driving | Condución |
| 53 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Choose what each controller button does. | Elixe que fai cada botón do mando. |
| 54 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The stick mouse and the air mouse (point with the controller). | O rato co stick e o air mouse (apunta co mando). |
| 55 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Size and place of this menu and the keyboard. | Tamaño e posición deste menú e do teclado. |
| 56 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Colours, how see-through the panels are, pointer smoothing and the language. | Cores, transparencia dos paneis, suavizado do punteiro e idioma. |
| 57 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Steering, gas and brake: how the truck's wheel follows your stick. | Dirección, acelerador e freo: como o volante do camión segue o teu stick. |
| 58 | Button (short: about 10 Latin characters) | Normal | Normal |
| 59 | Button (short: about 10 Latin characters) | Mod 1 | Mod 1 |
| 60 | Button (short: about 10 Latin characters) | Mod 2 | Mod 2 |
| 61 | Button (short: about 10 Latin characters) | Both | Ambos |
| 62 | Value or choice (about 18 Latin characters) | off | desactivado |
| 63 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does normally. | Mostra e axusta que fai cada botón normalmente. |
| 64 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 1 (%s). | Mostra e axusta que fai cada botón mentres mantés o modificador 1 (%s). |
| 65 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 2 (%s). | Mostra e axusta que fai cada botón mentres mantés o modificador 2 (%s). |
| 66 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold BOTH modifiers. | Mostra e axusta que fai cada botón mentres mantés OS DOUS modificadores. |
| 67 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 1: hold it, and other buttons do their Mod 1 job. Press here to choose another button. | Modificador 1: mantéino e os demais botóns fan a función Mod 1. Preme aquí para elixir outro botón. |
| 68 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 2: hold it, and other buttons do their Mod 2 job. Hold both modifiers for the Both jobs. Press here to choose another button. | Modificador 2: mantéino, os demais botóns fan a función Mod 2. Mantén os dous para as funcións «Ambos». Preme aquí para elixir outro botón. |
| 69 | Value or choice (about 18 Latin characters) | the wheel | o volante |
| 70 | Value or choice (about 18 Latin characters) | (grips = the wheel) | (grips = volante) |
| 71 | Value or choice (about 18 Latin characters) | modifier 1 | modificador 1 |
| 72 | Value or choice (about 18 Latin characters) | modifier 2 | modificador 2 |
| 73 | Value or choice (about 18 Latin characters) | air mouse | air mouse |
| 74 | Value or choice (about 18 Latin characters) | moves the mouse | move o rato |
| 75 | Value or choice (about 18 Latin characters) | steers | dirixe |
| 76 | Value or choice (about 18 Latin characters) | opens the menu | abre o menú |
| 77 | Value or choice (about 18 Latin characters) | as normal | como normal |
| 78 | Value or choice (about 18 Latin characters) | (+menu) | (+menú) |
| 79 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: %s (set elsewhere). | %s con %s: %s (configúrase noutro sitio). |
| 80 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: %s (set elsewhere). | %s: %s (configúrase noutro sitio). |
| 81 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s con %s: agora %s. Apunta aquí, preme o gatillo e elixe unha tecla do teclado ou unha opción de abaixo. |
| 82 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s: agora %s. Apunta aquí, preme o gatillo e elixe unha tecla do teclado ou unha opción de abaixo. |
| 83 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Also part of the menu's open buttons. | Tamén forma parte dos botóns que abren o menú. |
| 84 | Button (short: about 10 Latin characters) | Mouse L | Rato esq |
| 85 | Button (short: about 10 Latin characters) | Mouse R | Rato der |
| 86 | Button (short: about 10 Latin characters) | Mouse M | Rato c. |
| 87 | Button (short: about 10 Latin characters) | Wheel dn | Roda ↓ |
| 88 | Button (short: about 10 Latin characters) | pick\|Keyboard | Teclado |
| 89 | Button (short: about 10 Latin characters) | None | Ningún |
| 90 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Left mouse click. | Clic esquerdo do rato. |
| 91 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Right mouse click. | Clic dereito do rato. |
| 92 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Middle mouse click. | Clic central do rato. |
| 93 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel up (one step per press). | Roda arriba (un paso por pulsación). |
| 94 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel down (one step per press). | Roda abaixo (un paso por pulsación). |
| 95 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Gas pedal: press harder = more gas. | Pedal do acelerador: canto máis forte, máis gas. |
| 96 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Brake: press harder = more brake. | Freo: canto máis forte, máis freo. |
| 97 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Shows / hides the VR keyboard. | Mostra / agocha o teclado VR. |
| 98 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Nothing. | Nada. |
| 99 | Button (short: about 10 Latin characters) | Cancel | Cancelar |
| 100 | Status / hint line at the bottom (one line, about 55 Latin characters) | Hold %s first, then the button. | Mantén %s primeiro e logo o botón. |
| 101 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point at a button, pull the trigger, pick its key. | Apunta a un botón, preme o gatillo e elixe a súa tecla. |
| 102 | Name of a setting row (about 30 Latin characters) | Stick that moves the mouse | Stick que move o rato |
| 103 | Value or choice (about 18 Latin characters) | Left | Esquerdo |
| 104 | Value or choice (about 18 Latin characters) | Right | Dereito |
| 105 | Button (short: about 10 Latin characters) | Off | Desactivado |
| 106 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick moves the mouse pointer (for the game's menus and map). Off = no stick mouse. | Que stick move o punteiro (menús e mapa do xogo). Desactivado = sen rato co stick. |
| 107 | Name of a setting row (about 30 Latin characters) | Stick mouse speed | Velocidade do rato con stick |
| 108 | Value or choice (about 18 Latin characters) | %.0f px/s | %.0f px/s |
| 109 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How fast the pointer moves with the stick pushed all the way, in pixels per second. | A que velocidade se move o punteiro co stick a tope, en píxeles por segundo. |
| 110 | Name of a setting row (about 30 Latin characters) | Stick dead zone | Zona morta do stick |
| 111 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the pointer moves. More = no creeping pointer. | Canto hai que empurrar o stick antes de que o punteiro se mova. Máis = o punteiro non se move só. |
| 112 | Name of a setting row (about 30 Latin characters) | Air mouse button | Botón do air mouse |
| 113 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Hold this button and turn the controller: the pointer follows, like a laser pointer. Off = no air mouse. | Mantén este botón e xira o mando: o punteiro segue, como un punteiro láser. Desactivado = sen air mouse. |
| 114 | Name of a setting row (about 30 Latin characters) | Air mouse works while | O air mouse funciona cando |
| 115 | Value or choice (about 18 Latin characters) | Button held | Botón mantido |
| 116 | Value or choice (about 18 Latin characters) | Toggled on | Alternado |
| 117 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Button held: only while you hold the button. Toggled on: one press switches it on, the next press off. | Botón mantido: só mentres mantés o botón. Alternado: unha pulsación actívao, a seguinte desactívao. |
| 118 | Name of a setting row (about 30 Latin characters) | Air mouse speed | Velocidade do air mouse |
| 119 | Value or choice (about 18 Latin characters) | %.0f px/deg | %.0f px/deg |
| 120 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the pointer moves when you turn the controller, in pixels per degree. | Canto se move o punteiro ao xirar o mando, en píxeles por grao. |
| 121 | Status / hint line at the bottom (one line, about 55 Latin characters) | Changes work at once and are saved. | Os cambios aplícanse ao instante e gárdanse. |
| 122 | Name of a setting row (about 30 Latin characters) | Menu size | Tamaño do menú |
| 123 | Value or choice (about 18 Latin characters) | %.2f m wide | %.2f m de largo |
| 124 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big this menu is (its width in metres). | O tamaño deste menú (a súa largura en metros). |
| 125 | Name of a setting row (about 30 Latin characters) | Menu distance | Distancia do menú |
| 126 | Value or choice (about 18 Latin characters) | %.2f m | %.2f m |
| 127 | Button (short: about 10 Latin characters) | Closer | Máis preto |
| 128 | Button (short: about 10 Latin characters) | Farther | Máis lonxe |
| 129 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far this menu is from your head. | A distancia deste menú á túa cabeza. |
| 130 | Name of a setting row (about 30 Latin characters) | Keyboard size | Tamaño do teclado |
| 131 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big the VR keyboard is (its width in metres). | O tamaño do teclado VR (a súa largura en metros). |
| 132 | Name of a setting row (about 30 Latin characters) | Keyboard distance | Distancia do teclado |
| 133 | Value or choice (about 18 Latin characters) | (hidden) | (agochado) |
| 134 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the VR keyboard is from your head. | A distancia do teclado VR á túa cabeza. |
| 135 | Button (short: about 10 Latin characters) | Show | Mostrar |
| 136 | Button (short: about 10 Latin characters) | Hide | Agochar |
| 137 | Button (short: about 10 Latin characters) | Full | Completo |
| 138 | Button (short: about 10 Latin characters) | Compact | Compacto |
| 139 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show / hide the VR keyboard. Full = every key; Compact = only the main keys (narrower). | Mostra / agocha o teclado VR. Completo = todas as teclas; Compacto = só as principais (máis estreito). |
| 140 | Name of a setting row (about 30 Latin characters) | Bring the menu and the keyboard in front of me | Menú e teclado diante de min |
| 141 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Moves the menu and the keyboard to where you look now. | Move o menú e o teclado a onde miras agora. |
| 142 | Status / hint line at the bottom (one line, about 55 Latin characters) | Menu open / close: hold %s for %.1f s | Abrir / pechar menú: mantén %s %.1f s |
| 143 | Status / hint line at the bottom (one line, about 55 Latin characters) | Move: point at a panel, hold the grip, move. Its stick: farther / closer. | Mover: apunta a un panel, mantén o grip e move. Stick: lonxe / preto. |
| 144 | Status / hint line at the bottom (one line, about 55 Latin characters) | The keyboard stays when the menu closes (its x hides it). | O teclado queda cando o menú se pecha (o seu x agóchao). |
| 145 | Button (short: about 10 Latin characters) | Dark | Escuro |
| 146 | Button (short: about 10 Latin characters) | Blue | Azul |
| 147 | Button (short: about 10 Latin characters) | Grey | Gris |
| 148 | Button (short: about 10 Latin characters) | Light | Claro |
| 149 | Button (short: about 10 Latin characters) | Black | Negro |
| 150 | Button (short: about 10 Latin characters) | Green | Verde |
| 151 | Name of a setting row (about 30 Latin characters) | Colours | Cores |
| 152 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The colours of the menu and the keyboard. | As cores do menú e do teclado. |
| 153 | Name of a setting row (about 30 Latin characters) | Menu see-through | Transparencia do menú |
| 154 | Value or choice (about 18 Latin characters) | %d %% solid | %d %% opaco |
| 155 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid this menu is. - = more see-through, + = more solid. | Como de opaco é este menú. - = máis transparente, + = máis opaco. |
| 156 | Name of a setting row (about 30 Latin characters) | Keyboard see-through | Transparencia do teclado |
| 157 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid the VR keyboard is. - = more see-through, + = more solid. | Como de opaco é o teclado VR. - = máis transparente, + = máis opaco. |
| 158 | Name of a setting row (about 30 Latin characters) | Pointer smoothing | Suavizado do punteiro |
| 159 | Button (short: about 10 Latin characters) | Low | Baixo |
| 160 | Button (short: about 10 Latin characters) | Medium | Medio |
| 161 | Button (short: about 10 Latin characters) | High | Alto |
| 162 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Calms a shaky hand: the menu pointer and the air mouse. Higher = steadier, but it follows a bit later. | Calma unha man que treme: o punteiro do menú e o air mouse. Máis alto = máis estable, pero segue un pouco despois. |
| 163 | Value or choice (about 18 Latin characters) | %s (no font) | %s (sen fonte) |
| 164 | Name of a setting row (about 30 Latin characters) | Language | Idioma |
| 165 | Value or choice (about 18 Latin characters) | Auto: %s | Auto: %s |
| 166 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The menu's language. Auto = the language the game is set to. | O idioma do menú. Auto = o idioma configurado no xogo. |
| 167 | Name of a setting row (about 30 Latin characters) | Keyboard letters | Letras do teclado |
| 168 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The letters on the VR keyboard, as printed in that country. Auto = as the menu. A key always presses the same key in the game. | As letras do teclado VR, como nese país. Auto = como o menú. Unha tecla sempre preme a mesma tecla no xogo. |
| 169 | Status / hint line at the bottom (one line, about 55 Latin characters) | - = more see-through, + = more solid. | - = máis transparente, + = máis opaco. |
| 170 | Name of a setting row (about 30 Latin characters) | Steer with | Dirixir con |
| 171 | Button (short: about 10 Latin characters) | Stick | Stick |
| 172 | Button (short: about 10 Latin characters) | Wheel | Volante |
| 173 | Button (short: about 10 Latin characters) | Point | Apuntar |
| 174 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick: the stick steers. Wheel: hold the grip(s) and turn your hands like a steering wheel. Point: point a controller left / right (on a panel the wheel holds). Off: no driving. | Stick: o stick dirixe. Volante: mantén o(s) grip(s) e xira as mans como un volante. Apuntar: apunta un mando á esquerda / dereita (nun panel que o volante sostén). Desactivado: sen condución. |
| 175 | Name of a setting row (about 30 Latin characters) | Hands turn for full lock | Xiro de mans a tope |
| 176 | Name of a setting row (about 30 Latin characters) | Point for full lock | Apuntar a tope |
| 177 | Value or choice (about 18 Latin characters) | %.0f deg | %.0f° |
| 178 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you turn your hands (like a wheel) for full lock. Less = quicker, more = calmer. | Canto xiras as mans (como un volante) para o xiro máximo. Menos = máis rápido, máis = máis suave. |
| 179 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you point left / right for full lock. Less = quicker, more = calmer. | Canto apuntas á esquerda / dereita para o xiro máximo. Menos = máis rápido, máis = máis suave. |
| 180 | Button (short: about 10 Latin characters) | Stick on | Co stick |
| 181 | Button (short: about 10 Latin characters) | Stick off | Sen stick |
| 182 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick on: the stick steers too (it wins while you push it). Stick off: only your hands steer, and the stick's left / right are normal buttons again. | Co stick: o stick tamén dirixe (gaña mentres o empurras). Sen stick: só as túas mans dirixen, e esquerda / dereita do stick volven ser botóns normais. |
| 183 | Name of a setting row (about 30 Latin characters) | Hold the wheel with | Suxeitar o volante con |
| 184 | Value or choice (about 18 Latin characters) | Either | Calquera |
| 185 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Both: both grips together (one grip alone keeps its own job). Left / Right / Either: one grip is enough -- that grip is then only the wheel (no modifier, no air mouse, no key on it). | Ambos: os dous grips xuntos (un só grip mantén a súa función). Esquerdo / Dereito / Calquera: abonda cun grip -- ese grip é entón só o volante (sen modificador, sen air mouse, sen tecla). |
| 186 | Name of a setting row (about 30 Latin characters) | Hand that points | Man que apunta |
| 187 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The controller you point with. The other hand stays free for the keyboard and the menu. | O mando co que apuntas. A outra man queda libre para o teclado e o menú. |
| 188 | Name of a setting row (about 30 Latin characters) | Stick that steers | Stick que dirixe |
| 189 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick steers the truck (left / right). Its up / down stay free for other buttons. | Que stick dirixe o camión (esquerda / dereita). O seu arriba / abaixo queda libre para outros botóns. |
| 190 | Name of a setting row (about 30 Latin characters) | Steering dead zone | Zona morta de dirección |
| 191 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the wheel turns. More = a calm middle, less = it reacts sooner. | Canto hai que empurrar o stick antes de que o volante xire. Máis = un centro tranquilo, menos = reacciona antes. |
| 192 | Name of a setting row (about 30 Latin characters) | Small turns (most of the stick) | Xiros suaves (case todo) |
| 193 | Value or choice (about 18 Latin characters) | even (off) | lineal (desact.) |
| 194 | Value or choice (about 18 Latin characters) | up to %d %% | ata %d %% |
| 195 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Most of the stick (the first %d %% of a push) makes small, careful turns, up to this much of the wheel. Only the last bit, at the edge, turns hard. | A maior parte do stick (o primeiro %d %% dun empurrón) fai xiros suaves e precisos, ata esta parte do volante. Só o final, no bordo, xira forte. |
| 196 | Name of a setting row (about 30 Latin characters) | Full stick turns the wheel | Stick a tope xira o volante |
| 197 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the wheel turns with the stick pushed all the way. 100 % = full lock. | Canto xira o volante co stick a tope. 100 % = xiro máximo. |
| 198 | Name of a setting row (about 30 Latin characters) | Wheel follows the stick | O volante segue o stick |
| 199 | Value or choice (about 18 Latin characters) | at once | ao instante |
| 200 | Value or choice (about 18 Latin characters) | %.3g s | %.3g s |
| 201 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Smoothing while you hold the stick: how quickly the wheel goes where the stick points. More = smoother, calmer. | Suavizado mentres mantés o stick: con que rapidez o volante vai a onde apunta o stick. Máis = máis suave e tranquilo. |
| 202 | Name of a setting row (about 30 Latin characters) | Let go: back to the middle | Ao soltar: volve ao centro |
| 203 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Let go of the stick: how the wheel comes back to straight. At once = at the wheel speed below. More = a slow glide. | Solta o stick: como o volante volve recto. Ao instante = á velocidade do volante de abaixo. Máis = un retorno lento. |
| 204 | Name of a setting row (about 30 Latin characters) | Wheel speed at most | Velocidade máx. do volante |
| 205 | Value or choice (about 18 Latin characters) | no limit | sen límite |
| 206 | Value or choice (about 18 Latin characters) | %.2g s to full | %.2g s ao máx. |
| 207 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The fastest the wheel turns (full push; a small push turns it slower): seconds from straight to full lock. | A máxima velocidade do volante (empurrón cheo; un empurrón pequeno xírao máis lento): segundos de recto a xiro máximo. |
| 208 | Name of a setting row (about 30 Latin characters) | At 90 km/h: steering | A 90 km/h: dirección |
| 209 | Button (short: about 10 Latin characters) | On | Activado |
| 210 | Value or choice (about 18 Latin characters) | (off) | (desactivado) |
| 211 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | On: at speed the wheel turns less, for calm highway driving. The % = how much at 90 km/h. | Activado: a velocidade o volante xira menos, para autoestrada tranquila. O % = canto a 90 km/h. |
| 212 | Status / hint line at the bottom (one line, about 55 Latin characters) | Driving off: no steering, gas or brake. | Condución desactivada: sen dirección, gas nin freo. |
| 213 | Status / hint line at the bottom (one line, about 55 Latin characters) | Wheel: hold the grip(s) and turn your hands. Let go = straight. | Volante: mantén o(s) grip(s) e xira as mans. Solta = recto. |
| 214 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point: point left / right. On a panel the wheel holds. | Apuntar: apunta á esquerda / dereita. Nun panel que o volante sostén. |
| 215 | Status / hint line at the bottom (one line, about 55 Latin characters) | Most of the stick = small turns. All the way = hard turns. | Case todo o stick = xiros suaves. A tope = xiros fortes. |
| 216 | Button (short: about 10 Latin characters) | Sure? Reset | Seguro? |
| 217 | Button (short: about 10 Latin characters) | Reset tab | Restablecer |
| 218 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Puts this tab back to the default settings. Press twice to be sure. | Devolve esta lapela aos axustes por defecto. Preme dúas veces para confirmar. |
| 219 | Value or choice (about 18 Latin characters) | Set: %s + %s = %s | Asignado: %s + %s = %s |
| 220 | Value or choice (about 18 Latin characters) | Set: %s = %s | Asignado: %s = %s |
| 221 | Status / hint line at the bottom (one line, about 55 Latin characters) | Press again: this tab goes back to the defaults. | Preme de novo: esta lapela volve aos valores por defecto. |
| 222 | Status / hint line at the bottom (one line, about 55 Latin characters) | Back to the defaults. Saved. | Volta aos valores por defecto. Gardado. |
| 223 | Status / hint line at the bottom (one line, about 55 Latin characters) | Saved. | Gardado. |
| 224 | Status / hint line at the bottom (one line, about 55 Latin characters) | Choose that modifier button first (the buttons above). | Elixe primeiro ese botón modificador (os botóns de arriba). |
| 225 | Status / hint line at the bottom (one line, about 55 Latin characters) | Steering with the wheel: those grips hold it (Driving tab). | Dirección co volante: eses grips sostéñeno (lapela Condución). |
| 226 | Value or choice (about 18 Latin characters) | %s holds the wheel (Driving tab). | %s sostén o volante (lapela Condución). |
| 227 | Value or choice (about 18 Latin characters) | %s is modifier 1. | %s é o modificador 1. |
| 228 | Value or choice (about 18 Latin characters) | %s is modifier 2. | %s é o modificador 2. |
| 229 | Value or choice (about 18 Latin characters) | %s is the air mouse button (Mouse tab). | %s é o botón do air mouse (lapela Rato). |
| 230 | Value or choice (about 18 Latin characters) | With these modifiers, %s opens the menu. | Con estes modificadores, %s abre o menú. |
| 231 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick moves the mouse (Mouse tab). | Ese stick move o rato (lapela Rato). |
| 232 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick steers (Driving tab). | Ese stick dirixe (lapela Condución). |
| 233 | Status / hint line at the bottom (one line, about 55 Latin characters) | Could not save config.ini (the change works until the game ends). | Non se puido gardar config.ini (o cambio vale ata que peches o xogo). |
