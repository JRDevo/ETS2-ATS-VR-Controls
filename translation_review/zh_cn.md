# Review request: 简体中文 (zh_cn) translation of a VR game mod menu

You are a native 简体中文 speaker and a PC gaming / VR expert. Please review this translation.

**Context.** This is the in-headset settings menu and on-screen keyboard of a VR controller mod for the truck simulators
Euro Truck Simulator 2 and American Truck Simulator: it maps the VR controllers' buttons, sticks and pointing to keyboard
keys, the mouse and steering. The menu floats in front of the player; its boxes are small. All 简体中文 texts below were
written by an AI and may be wrong, unnatural or too long.

**Check every line for:**
1. Wrong meaning compared with the English.
2. Unnatural wording: use what 简体中文 PC gamers, VR players and game options menus normally say.
3. Consistency: the same English term should get the same 简体中文 term everywhere (grip, trigger, stick, modifier, air mouse,
   panel, see-through, dead zone, full lock).
4. Length: the "Where" column gives the room (Latin characters; a CJK character counts as 2). Too long = it gets smaller or
   cut off with "...". Key names on the keyboard: what keyboards in your country have printed (e.g. German: Strg, Entf).

**Rules your corrections must keep (or the mod falls back to English for that line):**
- Keep every placeholder exactly and in the same order: %s %d %.0f %.1f %.2f %.3g %.2g ...
- `%%` is a literal percent sign: keep it as `%%`. Never add a single `%`.
- Do not translate: VR, ETS2, ATS, config.ini, the button letters X / Y / A / B, units px, km/h.

**Answer format.** Only list the lines you would change, as a table:

| # | Current 简体中文 text | Corrected 简体中文 text | Reason (short, in English) |
|---|---|---|---|

Then one line: how many lines you checked and an overall quality rating (1-10). Do not repeat lines that are fine.

| # | Where | English | 简体中文 |
|---|---|---|---|
| 1 | Key on the VR keyboard (very short: about 6 Latin characters) | Esc | Esc |
| 2 | Key on the VR keyboard (very short: about 6 Latin characters) | PrtSc | PrtSc |
| 3 | Key on the VR keyboard (very short: about 6 Latin characters) | ScrLk | ScrLk |
| 4 | Key on the VR keyboard (very short: about 6 Latin characters) | Pause | Pause |
| 5 | Key on the VR keyboard (very short: about 6 Latin characters) | Backspace | Backspace |
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
| 18 | Key on the VR keyboard (very short: about 6 Latin characters) | Space | Space |
| 19 | Controller button name (about 16 Latin characters) | Left trigger | 左扳机 |
| 20 | Controller button name (about 16 Latin characters) | Right trigger | 右扳机 |
| 21 | Controller button name (about 16 Latin characters) | Left grip | 左握把 |
| 22 | Controller button name (about 16 Latin characters) | Right grip | 右握把 |
| 23 | Controller button name (about 16 Latin characters) | Left stick click | 左摇杆按下 |
| 24 | Controller button name (about 16 Latin characters) | Right stick click | 右摇杆按下 |
| 25 | Controller button name (about 16 Latin characters) | Left menu | 左菜单 |
| 26 | Controller button name (about 16 Latin characters) | Left stick up | 左摇杆上 |
| 27 | Controller button name (about 16 Latin characters) | Left stick down | 左摇杆下 |
| 28 | Controller button name (about 16 Latin characters) | Left stick left | 左摇杆左 |
| 29 | Controller button name (about 16 Latin characters) | Left stick right | 左摇杆右 |
| 30 | Controller button name (about 16 Latin characters) | Right stick up | 右摇杆上 |
| 31 | Controller button name (about 16 Latin characters) | Right stick down | 右摇杆下 |
| 32 | Controller button name (about 16 Latin characters) | Right stick left | 右摇杆左 |
| 33 | Controller button name (about 16 Latin characters) | Right stick right | 右摇杆右 |
| 34 | Value or choice (about 18 Latin characters) | Mouse left | 鼠标左键 |
| 35 | Value or choice (about 18 Latin characters) | Mouse right | 鼠标右键 |
| 36 | Value or choice (about 18 Latin characters) | Mouse middle | 鼠标中键 |
| 37 | Button (short: about 10 Latin characters) | Wheel up | 滚轮上 |
| 38 | Value or choice (about 18 Latin characters) | Wheel down | 滚轮下 |
| 39 | Button (short: about 10 Latin characters) | Gas | 油门 |
| 40 | Button (short: about 10 Latin characters) | Brake | 刹车 |
| 41 | Button (short: about 10 Latin characters) | Keyboard | 键盘 |
| 42 | Value or choice (about 18 Latin characters) | Num %s | 小键盘%s |
| 43 | Title bar (one line, about 40 Latin characters) | Set %s: pick a key | 设置%s：选一个键 |
| 44 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu smaller. | 把此菜单变小。 |
| 45 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Make this menu bigger. | 把此菜单变大。 |
| 46 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show or hide the VR keyboard. | 显示或隐藏VR键盘。 |
| 47 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Close this menu. To open it again: hold %s. | 关闭此菜单。再次打开：长按 %s。 |
| 48 | Tab name at the top of the menu (about 11 Latin characters) | Buttons | 按键 |
| 49 | Tab name at the top of the menu (about 11 Latin characters) | Mouse | 鼠标 |
| 50 | Tab name at the top of the menu (about 11 Latin characters) | Panels | 面板 |
| 51 | Tab name at the top of the menu (about 11 Latin characters) | Look | 外观 |
| 52 | Tab name at the top of the menu (about 11 Latin characters) | Driving | 驾驶 |
| 53 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Choose what each controller button does. | 设置每个手柄按键的功能。 |
| 54 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The stick mouse and the air mouse (point with the controller). | 摇杆鼠标与空中鼠标（用手柄指向）。 |
| 55 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Size and place of this menu and the keyboard. | 此菜单与键盘的大小和位置。 |
| 56 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Colours, how see-through the panels are, pointer smoothing and the language. | 颜色、面板透明度、指针平滑与语言。 |
| 57 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Steering, gas and brake: how the truck's wheel follows your stick. | 转向、油门与刹车：方向盘如何跟随摇杆。 |
| 58 | Button (short: about 10 Latin characters) | Normal | 普通 |
| 59 | Button (short: about 10 Latin characters) | Mod 1 | 修饰1 |
| 60 | Button (short: about 10 Latin characters) | Mod 2 | 修饰2 |
| 61 | Button (short: about 10 Latin characters) | Both | 两者 |
| 62 | Value or choice (about 18 Latin characters) | off | 关 |
| 63 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does normally. | 显示并设置每个按键的普通功能。 |
| 64 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 1 (%s). | 显示并设置按住修饰键1（%s）时每个按键的功能。 |
| 65 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold modifier 2 (%s). | 显示并设置按住修饰键2（%s）时每个按键的功能。 |
| 66 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show and set what each button does while you hold BOTH modifiers. | 显示并设置同时按住两个修饰键时每个按键的功能。 |
| 67 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 1: hold it, and other buttons do their Mod 1 job. Press here to choose another button. | 修饰键1：按住时其他按键执行其修饰1功能。按此可另选按键。 |
| 68 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Modifier 2: hold it, and other buttons do their Mod 2 job. Hold both modifiers for the Both jobs. Press here to choose another button. | 修饰键2：按住时其他按键执行其修饰2功能。同时按住两者则执行“两者”功能。按此可另选按键。 |
| 69 | Value or choice (about 18 Latin characters) | the wheel | 方向盘 |
| 70 | Value or choice (about 18 Latin characters) | (grips = the wheel) | （握把=方向盘） |
| 71 | Value or choice (about 18 Latin characters) | modifier 1 | 修饰键1 |
| 72 | Value or choice (about 18 Latin characters) | modifier 2 | 修饰键2 |
| 73 | Value or choice (about 18 Latin characters) | air mouse | 空中鼠标 |
| 74 | Value or choice (about 18 Latin characters) | moves the mouse | 移动鼠标 |
| 75 | Value or choice (about 18 Latin characters) | steers | 转向 |
| 76 | Value or choice (about 18 Latin characters) | opens the menu | 打开菜单 |
| 77 | Value or choice (about 18 Latin characters) | as normal | 同普通 |
| 78 | Value or choice (about 18 Latin characters) | (+menu) | （+菜单） |
| 79 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: %s (set elsewhere). | %s + %s：%s（在别处设置）。 |
| 80 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: %s (set elsewhere). | %s：%s（在别处设置）。 |
| 81 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s with %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s + %s：当前 %s。指向此处扣动扳机，再在键盘上选键或选下方选项。 |
| 82 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | %s: now %s. Point here, pull the trigger, then pick a key on the keyboard or a choice below. | %s：当前 %s。指向此处扣动扳机，再在键盘上选键或选下方选项。 |
| 83 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Also part of the menu's open buttons. | 也是打开菜单的按键之一。 |
| 84 | Button (short: about 10 Latin characters) | Mouse L | 鼠标左 |
| 85 | Button (short: about 10 Latin characters) | Mouse R | 鼠标右 |
| 86 | Button (short: about 10 Latin characters) | Mouse M | 鼠标中 |
| 87 | Button (short: about 10 Latin characters) | Wheel dn | 滚轮下 |
| 88 | Button (short: about 10 Latin characters) | pick\|Keyboard | 键盘 |
| 89 | Button (short: about 10 Latin characters) | None | 无 |
| 90 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Left mouse click. | 鼠标左键单击。 |
| 91 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Right mouse click. | 鼠标右键单击。 |
| 92 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Middle mouse click. | 鼠标中键单击。 |
| 93 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel up (one step per press). | 鼠标滚轮上（每按一次一格）。 |
| 94 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Mouse wheel down (one step per press). | 鼠标滚轮下（每按一次一格）。 |
| 95 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Gas pedal: press harder = more gas. | 油门：按得越重油门越大。 |
| 96 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Brake: press harder = more brake. | 刹车：按得越重刹车越大。 |
| 97 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Shows / hides the VR keyboard. | 显示/隐藏VR键盘。 |
| 98 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Nothing. | 无。 |
| 99 | Button (short: about 10 Latin characters) | Cancel | 取消 |
| 100 | Status / hint line at the bottom (one line, about 55 Latin characters) | Hold %s first, then the button. | 先按住 %s，再按该键。 |
| 101 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point at a button, pull the trigger, pick its key. | 指向某个按键，扣动扳机，选择它对应的键。 |
| 102 | Name of a setting row (about 30 Latin characters) | Stick that moves the mouse | 移动鼠标的摇杆 |
| 103 | Value or choice (about 18 Latin characters) | Left | 左 |
| 104 | Value or choice (about 18 Latin characters) | Right | 右 |
| 105 | Button (short: about 10 Latin characters) | Off | 关 |
| 106 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick moves the mouse pointer (for the game's menus and map). Off = no stick mouse. | 用哪个摇杆移动鼠标指针（用于游戏菜单和地图）。关=无摇杆鼠标。 |
| 107 | Name of a setting row (about 30 Latin characters) | Stick mouse speed | 摇杆鼠标速度 |
| 108 | Value or choice (about 18 Latin characters) | %.0f px/s | %.0f px/s |
| 109 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How fast the pointer moves with the stick pushed all the way, in pixels per second. | 摇杆推到底时指针移动速度（像素/秒）。 |
| 110 | Name of a setting row (about 30 Latin characters) | Stick dead zone | 摇杆死区 |
| 111 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the pointer moves. More = no creeping pointer. | 指针开始移动前需推动摇杆的幅度。越大越不会漂移。 |
| 112 | Name of a setting row (about 30 Latin characters) | Air mouse button | 空中鼠标按键 |
| 113 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Hold this button and turn the controller: the pointer follows, like a laser pointer. Off = no air mouse. | 按住此键并转动手柄，指针会像激光笔一样跟随。关=无空中鼠标。 |
| 114 | Name of a setting row (about 30 Latin characters) | Air mouse works while | 空中鼠标工作方式 |
| 115 | Value or choice (about 18 Latin characters) | Button held | 按住时 |
| 116 | Value or choice (about 18 Latin characters) | Toggled on | 切换开启 |
| 117 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Button held: only while you hold the button. Toggled on: one press switches it on, the next press off. | 按住时：仅在按住按键时。切换：按一次开，再按一次关。 |
| 118 | Name of a setting row (about 30 Latin characters) | Air mouse speed | 空中鼠标速度 |
| 119 | Value or choice (about 18 Latin characters) | %.0f px/deg | %.0f px/度 |
| 120 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the pointer moves when you turn the controller, in pixels per degree. | 转动手柄时指针的移动量（像素/度）。 |
| 121 | Status / hint line at the bottom (one line, about 55 Latin characters) | Changes work at once and are saved. | 更改立即生效并保存。 |
| 122 | Name of a setting row (about 30 Latin characters) | Menu size | 菜单大小 |
| 123 | Value or choice (about 18 Latin characters) | %.2f m wide | 宽 %.2f m |
| 124 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big this menu is (its width in metres). | 此菜单的大小（宽度，米）。 |
| 125 | Name of a setting row (about 30 Latin characters) | Menu distance | 菜单距离 |
| 126 | Value or choice (about 18 Latin characters) | %.2f m | %.2f m |
| 127 | Button (short: about 10 Latin characters) | Closer | 更近 |
| 128 | Button (short: about 10 Latin characters) | Farther | 更远 |
| 129 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far this menu is from your head. | 此菜单离头部的距离。 |
| 130 | Name of a setting row (about 30 Latin characters) | Keyboard size | 键盘大小 |
| 131 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How big the VR keyboard is (its width in metres). | VR键盘的大小（宽度，米）。 |
| 132 | Name of a setting row (about 30 Latin characters) | Keyboard distance | 键盘距离 |
| 133 | Value or choice (about 18 Latin characters) | (hidden) | （已隐藏） |
| 134 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the VR keyboard is from your head. | VR键盘离头部的距离。 |
| 135 | Button (short: about 10 Latin characters) | Show | 显示 |
| 136 | Button (short: about 10 Latin characters) | Hide | 隐藏 |
| 137 | Button (short: about 10 Latin characters) | Full | 完整 |
| 138 | Button (short: about 10 Latin characters) | Compact | 精简 |
| 139 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Show / hide the VR keyboard. Full = every key; Compact = only the main keys (narrower). | 显示/隐藏VR键盘。完整=全部按键；精简=仅主要按键（更窄）。 |
| 140 | Name of a setting row (about 30 Latin characters) | Bring the menu and the keyboard in front of me | 将菜单和键盘移到我面前 |
| 141 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Moves the menu and the keyboard to where you look now. | 把菜单和键盘移到你当前看的方向。 |
| 142 | Status / hint line at the bottom (one line, about 55 Latin characters) | Menu open / close: hold %s for %.1f s | 打开/关闭菜单：长按 %s %.1f 秒 |
| 143 | Status / hint line at the bottom (one line, about 55 Latin characters) | Move: point at a panel, hold the grip, move. Its stick: farther / closer. | 移动：指向面板，按住握把并移动。其摇杆=更远/更近。 |
| 144 | Status / hint line at the bottom (one line, about 55 Latin characters) | The keyboard stays when the menu closes (its x hides it). | 菜单关闭后键盘仍保留（用×隐藏）。 |
| 145 | Button (short: about 10 Latin characters) | Dark | 深色 |
| 146 | Button (short: about 10 Latin characters) | Blue | 蓝色 |
| 147 | Button (short: about 10 Latin characters) | Grey | 灰色 |
| 148 | Button (short: about 10 Latin characters) | Light | 浅色 |
| 149 | Button (short: about 10 Latin characters) | Black | 黑色 |
| 150 | Button (short: about 10 Latin characters) | Green | 绿色 |
| 151 | Name of a setting row (about 30 Latin characters) | Colours | 颜色 |
| 152 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The colours of the menu and the keyboard. | 菜单和键盘的颜色。 |
| 153 | Name of a setting row (about 30 Latin characters) | Menu see-through | 菜单透明度 |
| 154 | Value or choice (about 18 Latin characters) | %d %% solid | %d%% 不透明 |
| 155 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid this menu is. - = more see-through, + = more solid. | 此菜单的不透明度。-=更透明，+=更不透明。 |
| 156 | Name of a setting row (about 30 Latin characters) | Keyboard see-through | 键盘透明度 |
| 157 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How solid the VR keyboard is. - = more see-through, + = more solid. | VR键盘的不透明度。-=更透明，+=更不透明。 |
| 158 | Name of a setting row (about 30 Latin characters) | Pointer smoothing | 指针平滑 |
| 159 | Button (short: about 10 Latin characters) | Low | 低 |
| 160 | Button (short: about 10 Latin characters) | Medium | 中 |
| 161 | Button (short: about 10 Latin characters) | High | 高 |
| 162 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Calms a shaky hand: the menu pointer and the air mouse. Higher = steadier, but it follows a bit later. | 减轻手抖：菜单指针和空中鼠标。越高越稳，但跟随会稍慢。 |
| 163 | Value or choice (about 18 Latin characters) | %s (no font) | %s（无字体） |
| 164 | Name of a setting row (about 30 Latin characters) | Language | 语言 |
| 165 | Value or choice (about 18 Latin characters) | Auto: %s | 自动：%s |
| 166 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The menu's language. Auto = the language the game is set to. | 菜单语言。自动=游戏所设语言。 |
| 167 | Name of a setting row (about 30 Latin characters) | Keyboard letters | 键盘字母 |
| 168 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The letters on the VR keyboard, as printed in that country. Auto = as the menu. A key always presses the same key in the game. | VR键盘上的字母（按该国布局）。自动=同菜单。任何键在游戏中始终按下相同的键。 |
| 169 | Status / hint line at the bottom (one line, about 55 Latin characters) | - = more see-through, + = more solid. | -=更透明，+=更不透明。 |
| 170 | Name of a setting row (about 30 Latin characters) | Steer with | 转向方式 |
| 171 | Button (short: about 10 Latin characters) | Stick | 摇杆 |
| 172 | Button (short: about 10 Latin characters) | Wheel | 方向盘 |
| 173 | Button (short: about 10 Latin characters) | Point | 指向 |
| 174 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick: the stick steers. Wheel: hold the grip(s) and turn your hands like a steering wheel. Point: point a controller left / right (on a panel the wheel holds). Off: no driving. | 摇杆：用摇杆转向。方向盘：握住握把，双手像方向盘一样转动。指向：手柄向左/右指（在方向盘所持面板上）。关：不驾驶。 |
| 175 | Name of a setting row (about 30 Latin characters) | Hands turn for full lock | 打满所需双手转角 |
| 176 | Name of a setting row (about 30 Latin characters) | Point for full lock | 打满所需指向幅度 |
| 177 | Value or choice (about 18 Latin characters) | %.0f deg | %.0f 度 |
| 178 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you turn your hands (like a wheel) for full lock. Less = quicker, more = calmer. | 打满时双手转动的角度（像方向盘）。越小越快，越大越平稳。 |
| 179 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you point left / right for full lock. Less = quicker, more = calmer. | 打满时左右指向的角度。越小越快，越大越平稳。 |
| 180 | Button (short: about 10 Latin characters) | Stick on | 摇杆开 |
| 181 | Button (short: about 10 Latin characters) | Stick off | 摇杆关 |
| 182 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Stick on: the stick steers too (it wins while you push it). Stick off: only your hands steer, and the stick's left / right are normal buttons again. | 摇杆开：摇杆也可转向（推动时优先）。摇杆关：仅用双手转向，摇杆左/右恢复为普通按键。 |
| 183 | Name of a setting row (about 30 Latin characters) | Hold the wheel with | 握方向盘的手 |
| 184 | Value or choice (about 18 Latin characters) | Either | 任一 |
| 185 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Both: both grips together (one grip alone keeps its own job). Left / Right / Either: one grip is enough -- that grip is then only the wheel (no modifier, no air mouse, no key on it). | 两者：两握把同时（单独一个保持本职）。左/右/任一：一个握把即可——该握把仅作方向盘（无修饰、无空中鼠标、无按键）。 |
| 186 | Name of a setting row (about 30 Latin characters) | Hand that points | 指向的手 |
| 187 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The controller you point with. The other hand stays free for the keyboard and the menu. | 用于指向的手柄。另一只手留给键盘和菜单。 |
| 188 | Name of a setting row (about 30 Latin characters) | Stick that steers | 转向的摇杆 |
| 189 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Which stick steers the truck (left / right). Its up / down stay free for other buttons. | 用哪个摇杆为卡车转向（左/右）。其上/下留给其他按键。 |
| 190 | Name of a setting row (about 30 Latin characters) | Steering dead zone | 转向死区 |
| 191 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far you must push the stick before the wheel turns. More = a calm middle, less = it reacts sooner. | 方向盘开始转动前需推动摇杆的幅度。越大中间越平稳，越小反应越快。 |
| 192 | Name of a setting row (about 30 Latin characters) | Small turns (most of the stick) | 小幅转向（摇杆大部分） |
| 193 | Value or choice (about 18 Latin characters) | even (off) | 均匀（关） |
| 194 | Value or choice (about 18 Latin characters) | up to %d %% | 最多 %d%% |
| 195 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Most of the stick (the first %d %% of a push) makes small, careful turns, up to this much of the wheel. Only the last bit, at the edge, turns hard. | 摇杆大部分（推动的前 %d%%）做小而精细的转向，最多到方向盘的此比例。只有边缘最后一段才大幅转。 |
| 196 | Name of a setting row (about 30 Latin characters) | Full stick turns the wheel | 满杆时方向盘转角 |
| 197 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | How far the wheel turns with the stick pushed all the way. 100 % = full lock. | 摇杆推到底时方向盘转动的幅度。100%=打满。 |
| 198 | Name of a setting row (about 30 Latin characters) | Wheel follows the stick | 方向盘跟随摇杆 |
| 199 | Value or choice (about 18 Latin characters) | at once | 立即 |
| 200 | Value or choice (about 18 Latin characters) | %.3g s | %.3g 秒 |
| 201 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Smoothing while you hold the stick: how quickly the wheel goes where the stick points. More = smoother, calmer. | 推住摇杆时的平滑：方向盘转到摇杆所指方向的快慢。越大越平滑、越平稳。 |
| 202 | Name of a setting row (about 30 Latin characters) | Let go: back to the middle | 松开：回正中 |
| 203 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Let go of the stick: how the wheel comes back to straight. At once = at the wheel speed below. More = a slow glide. | 松开摇杆时方向盘回正的方式。立即=按下方方向盘速度。越大回得越慢。 |
| 204 | Name of a setting row (about 30 Latin characters) | Wheel speed at most | 方向盘最高速度 |
| 205 | Value or choice (about 18 Latin characters) | no limit | 无限制 |
| 206 | Value or choice (about 18 Latin characters) | %.2g s to full | %.2g 秒打满 |
| 207 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | The fastest the wheel turns (full push; a small push turns it slower): seconds from straight to full lock. | 方向盘转动的最高速度（满推时；轻推更慢）：从回正到打满的秒数。 |
| 208 | Name of a setting row (about 30 Latin characters) | At 90 km/h: steering | 90 km/h 时转向 |
| 209 | Button (short: about 10 Latin characters) | On | 开 |
| 210 | Value or choice (about 18 Latin characters) | (off) | （关） |
| 211 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | On: at speed the wheel turns less, for calm highway driving. The % = how much at 90 km/h. | 开：车速越高方向盘转得越少，高速更平稳。%=在90 km/h时的比例。 |
| 212 | Status / hint line at the bottom (one line, about 55 Latin characters) | Driving off: no steering, gas or brake. | 驾驶关：无转向、油门或刹车。 |
| 213 | Status / hint line at the bottom (one line, about 55 Latin characters) | Wheel: hold the grip(s) and turn your hands. Let go = straight. | 方向盘：握住握把并转动双手。松开=回正。 |
| 214 | Status / hint line at the bottom (one line, about 55 Latin characters) | Point: point left / right. On a panel the wheel holds. | 指向：向左/右指。在方向盘所持的面板上。 |
| 215 | Status / hint line at the bottom (one line, about 55 Latin characters) | Most of the stick = small turns. All the way = hard turns. | 摇杆大部分=小幅转向。推到底=大幅转向。 |
| 216 | Button (short: about 10 Latin characters) | Sure? Reset | 确定？重置 |
| 217 | Button (short: about 10 Latin characters) | Reset tab | 重置本页 |
| 218 | Help shown at the bottom while the pointer is on something (up to 2 lines, about 110 Latin characters) | Puts this tab back to the default settings. Press twice to be sure. | 将此页恢复为默认设置。按两次以确认。 |
| 219 | Value or choice (about 18 Latin characters) | Set: %s + %s = %s | 设置：%s + %s = %s |
| 220 | Value or choice (about 18 Latin characters) | Set: %s = %s | 设置：%s = %s |
| 221 | Status / hint line at the bottom (one line, about 55 Latin characters) | Press again: this tab goes back to the defaults. | 再按一次：此页恢复为默认。 |
| 222 | Status / hint line at the bottom (one line, about 55 Latin characters) | Back to the defaults. Saved. | 已恢复默认。已保存。 |
| 223 | Status / hint line at the bottom (one line, about 55 Latin characters) | Saved. | 已保存。 |
| 224 | Status / hint line at the bottom (one line, about 55 Latin characters) | Choose that modifier button first (the buttons above). | 请先选择该修饰按键（上方按键）。 |
| 225 | Status / hint line at the bottom (one line, about 55 Latin characters) | Steering with the wheel: those grips hold it (Driving tab). | 正用方向盘转向：那些握把握住它（驾驶页）。 |
| 226 | Value or choice (about 18 Latin characters) | %s holds the wheel (Driving tab). | %s 握住方向盘（驾驶页）。 |
| 227 | Value or choice (about 18 Latin characters) | %s is modifier 1. | %s 是修饰键1。 |
| 228 | Value or choice (about 18 Latin characters) | %s is modifier 2. | %s 是修饰键2。 |
| 229 | Value or choice (about 18 Latin characters) | %s is the air mouse button (Mouse tab). | %s 是空中鼠标按键（鼠标页）。 |
| 230 | Value or choice (about 18 Latin characters) | With these modifiers, %s opens the menu. | 在这些修饰键下，%s 打开菜单。 |
| 231 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick moves the mouse (Mouse tab). | 该摇杆移动鼠标（鼠标页）。 |
| 232 | Status / hint line at the bottom (one line, about 55 Latin characters) | That stick steers (Driving tab). | 该摇杆转向（驾驶页）。 |
| 233 | Status / hint line at the bottom (one line, about 55 Latin characters) | Could not save config.ini (the change works until the game ends). | 无法保存 config.ini（更改在游戏结束前有效）。 |
