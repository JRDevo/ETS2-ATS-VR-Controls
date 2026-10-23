# ETS2 ATS VR Controls

Use your VR controllers as a keyboard and mouse in **Euro Truck Simulator 2** and **American Truck Simulator** (OpenXR VR).
Press a controller button, and the game gets a key press. Move a stick or point the controller, and the mouse pointer moves.

> Status: v0.2.0. New in v0.2.0: **a VR menu and a full keyboard in the headset**, and **driving with the controllers**
> (smooth steering, gas and brake). Planned: the keyboard pinned to the cab, and single buttons you place anywhere (for
> example over a cab switch) that press a key.

> **OpenXR only.** The mod works when the game runs in its **OpenXR** VR mode (`-openxr`). It does not work in the game's
> `-openvr` (SteamVR native) or `-oculus` (Oculus native) modes. Every headset can use OpenXR, see
> [Which VR mode?](#which-vr-mode).

## Which VR mode?

ETS2 / ATS have three VR modes, chosen by a launch option (Steam: right-click the game > Properties > Launch options).
This mod works **only** in the OpenXR one:

| Launch option | VR system | This mod |
|---|---|---|
| `-openxr` | OpenXR | **works** |
| `-openvr` | SteamVR's own system (OpenVR) | does not load |
| `-oculus` | Meta's old Oculus system | does not load |

SCS also recommends OpenXR: it is the only VR mode in the normal game builds (1.55+; there, also add `-experimental_vr`),
while `-openvr` and `-oculus` exist only in the `oculus` beta branch on Steam.

How to use OpenXR with your headset:

- **Quest via Virtual Desktop:** `-openxr`, with Virtual Desktop's VDXR as the OpenXR runtime (Virtual Desktop sets it).
- **Quest via Link / Air Link:** `-openxr`, with the Meta Quest Link app as the OpenXR runtime (Meta app > Settings >
  General > OpenXR Runtime).
- **Valve Index, HTC Vive, Pico and other SteamVR headsets:** `-openxr`, with SteamVR as the OpenXR runtime (SteamVR >
  Settings > OpenXR > Set SteamVR as OpenXR runtime). The game then still runs through SteamVR.
- **Windows Mixed Reality:** `-openxr`, with the Windows Mixed Reality OpenXR runtime.

Only one OpenXR runtime is active at a time; the last one you set wins.

## What it does

- Each controller button sends a key or a mouse button that you choose in `config.ini`.
- One stick moves the mouse pointer (the right stick by default).
- **Air mouse**: hold the right grip and turn the controller. The pointer follows, like a laser pointer. (Or set it to
  toggle: one press on, the next press off.)
- **Modifier**: while you hold the modifier button (right grip), buttons can send something else. Default: **right grip +
  A or right trigger = left click, right grip + B = right click** -- so one controller does everything: hold the grip (the
  pointer follows), press A or the trigger to click, hold B to right-click (drag the map). Set any button's modified key in the VR menu.
- **Driving**: the left stick steers, the right trigger is the gas pedal, the left trigger the brake. Smooth, like a
  gamepad, not on / off keys. Gas and brake can go on any button. See [Driving](#driving-with-one-controller).
- **VR menu + keyboard**: hold both grips + the left stick click for 1 second. A menu opens in front of you. See [The VR menu](#the-vr-menu).
- The game's own VR controls (the trigger and the left menu button) still work.
- It only works in the games named in `config.ini`. In every other VR program it does nothing.
- It only sends input while the game window is in front. It never types into another program.

It is an OpenXR API layer: a small add-on that the VR system loads into the game. It does not change any game file, and it
works next to the ETS2 / ATS DLAA mod, VR EyeCapture, Virtual Desktop, OpenXR Toolkit and VectorXR.

## Default buttons

Made so that **one controller (the right one) does the whole mouse**: point with the right stick or the air mouse, A =
left click, right grip + right trigger = left click too, right grip + B = right click. **Driving**: left stick =
steering, right trigger = gas, left trigger = brake.

| Controller | Sends | In ETS2 / ATS (default keys) |
|---|---|---|
| Right stick | moves the mouse pointer | |
| Right grip (hold) | air mouse (the pointer follows the controller), and the **modifier** | |
| A | left mouse click | click |
| Right grip + A | left mouse click | click while the air mouse aims |
| Right grip + B | right mouse click | drag the map: hold the grip, aim, hold B |
| Right grip + right trigger | left mouse click | click while the air mouse aims |
| Right trigger | **gas pedal** (press harder = more gas, let go = roll) | |
| B | Esc | menu / back |
| Left stick left / right | **steering** (stick half left = wheel half left; let go = straight again) | see [Driving](#driving-with-one-controller) |
| Left trigger | **brake** (press harder = more brake) | |
| X | `[` | left indicator |
| Y | `]` | right indicator |
| Right stick click | H | horn |
| Left stick click | F | hazard lights |
| Both grips + left stick click, 1 second | opens / closes the VR menu | |

(The right stick's arrow keys -- up / down / left / right -- work when you set the mouse to the left stick: Mouse tab.)

Change them in `config.ini`. If you changed the game's key bindings, change `config.ini` to match.

The game keeps its own menu button: the left menu button on every controller, B and Y on Valve Index, B on Vive and Windows
Mixed Reality. A key set for that button does nothing (the log says so).

Every other button is shared with other VR mods that watch it (for example VR EyeCapture's record combo, both stick
clicks): both react. If that is not what you want, set that button to `none` here or in the VR menu.

**Cab buttons:** the game has no clickable buttons in the cab, so pointing at a switch does nothing. Map the key of that
function to a controller button instead (for example `y = l` for the lights).

## Driving with one controller

By default:

| Controller | Does |
|---|---|
| Left stick left / right | steering. The wheel glides to where the stick is: a little push = a little turn, all the way = full turn. Ease the stick = less turn. Let go: the wheel glides back to straight. |
| Right trigger | gas pedal. Press a little = a little gas. Let go = the truck rolls (no gas). |
| Left trigger | brake. Press a little = a little brake. |
| Right grip + right trigger | left mouse click (not gas): hold the grip to aim with the air mouse, pull the trigger to click |

**You choose what drives.** Gas and brake are normal button choices, like a key:

- In the VR menu: Buttons tab, point at a button, then pick **Gas** or **Brake** in the bar at the bottom. Also with the
  modifier ("With modifier": for example right grip + right trigger = brake).
- In `config.ini`: `rtrigger = brake`, `lgrip = brake`, `lup = gas` (stick forward), `modifier_keys = rtrigger:brake` ...
- Triggers and grips are analog (press harder = more). A stick direction too. Any other button (A, B, X, Y, stick click)
  is full gas / full brake.
- The steering stick: left, right or off (Driving tab, or `steer = left / right / off`). That stick's left / right then
  steer; its up / down are still free buttons (for example gas and brake).

**How the wheel moves:** the stick's place is where the wheel goes (stick half left = wheel half left). The wheel does not
jump there: it glides. A big push moves it fast, a small correction gently. Ease the stick a bit (the next bend needs less):
the wheel comes back to the new place and stays there. Let go: the wheel glides back to straight -- calmer at speed, like a
real truck.

**Steering feel** (Driving tab, or `config.ini`):

| Setting | What it does | Default |
|---|---|---|
| Steering dead zone (`steer_deadzone`) | how far the stick must move before the wheel turns | 0.10 |
| Steering curve (`steer_curve`) | 1 = straight. More = small pushes much gentler | 1.00 |
| Full stick turns the wheel (`steer_max`) | how far the wheel turns with the stick all the way | 100 % |
| Wheel follows the stick (`steer_follow_time`) | how quickly the wheel goes to the stick's place, in seconds. More = calmer. 0 = at once | 0.25 s |
| Let go: back to the middle (`steer_center_time`) | how quickly the wheel comes back when you ease or let go, in seconds (when standing; calmer at speed by itself). 0 = at once | 0.8 s |
| Wheel speed at most (`steer_max_speed`) | the wheel never turns faster than this: seconds from the middle to full lock (in config.ini: full turns per second, 0.6 = 1.7 s). Slower at speed too | 1.7 s |
| At 90 km/h (`steer_speed_sens`, `steer_at_speed`) | **speed-sensitive steering**, On / Off: up to 10 km/h the stick steers fully; at 90 km/h and faster full stick turns the wheel only this much (30 %), so small moves are fine. A bend you hold while you speed up stays | On, 30 % |

Too twitchy at speed? Lower "At 90 km/h". Too twitchy everywhere? Make the curve bigger, or "Wheel follows the stick"
slower. The wheel comes back too fast or too slowly? Change "Let go: back to the middle".

**Switch driving off:** Driving tab > Driving: **Off** (or `drive = 0`). Then nothing steers, the gas / brake buttons do
nothing, and the steering stick's left / right send their keys again. **On** brings it all back.

It is smooth, like a gamepad: not on / off keys. How: ETS2 and ATS have their own door for input add-ons (the SCS input
SDK). This mod is also such an add-on, so the game gets the stick and the triggers directly. **No driver**, no fake gamepad,
no file in the game folder, and **nothing to set in the game's options** (it works next to your wheel, keyboard or gamepad
settings). `install.ps1` tells the game where the mod is (see [What install.ps1 does](#what-installps1-does-and-why-there-is-one)).

- **The game shows a note at every start**: "Request to use advanced SDK features detected". That is this mod. Point at
  **OK** and click (A or the right trigger). SCS shows it for every add-on that uses their SDK; as far as we know, a mod
  cannot hide it.

  ![The game's note at start: Request to use advanced SDK features detected](docs/images/sdk-popup.png)
- `steer = off` and no gas / brake buttons: nothing drives, but the game still loads the add-on (so the VR menu can change
  it at once). To remove it completely, run `uninstall.ps1`.
- It works in every OpenXR mode and headset (Virtual Desktop, Link, SteamVR, Windows MR): the driving part does not depend
  on the headset at all.

## The VR menu

Hold **both grips**, then press and hold the **left stick click**, for 1 second. The menu opens in front of you. Do it again
to close it. (Grips first: the left stick click alone is still the hazard lights.) It is not both stick clicks, because
VR EyeCapture starts a recording on that.

### Two different pointers (read this first)

There are **two** pointers, and only one of them works on the menu and the keyboard:

| | The game's mouse pointer | The VR pointer (white dot) |
|---|---|---|
| What it is | Windows' mouse arrow, inside the game's own screens | A white dot that this mod draws on its own panels |
| How it moves | The right stick, or the **air mouse**: hold the **right grip** and turn the controller | Just **point** a controller at the menu or the keyboard. No button needed. |
| Where it works | Only inside the game's picture (game menus, map, ...). It can **not** reach the VR menu or keyboard. | Only on the VR menu and the keyboard |
| How it clicks | A = left click, right grip + B = right click | **Trigger** while the dot is on a key / button |

So for the menu and the keyboard: **do not use the air mouse** (do not hold the grip). Aim the controller at the panel like
a laser pointer. A **white dot** appears where you point, and the key under it lights up. Then pull the trigger.

![The white pointer dot on the G key of the keyboard](docs/images/keyboard-pointer.png)

(While a controller points at a panel, its grip does not start the air mouse, and the game does not get its trigger.)

- **Point** with either controller. A white dot shows where. **Pull the trigger** to press.
  While you point at the menu, the game does not get that trigger.
- **Buttons** tab: every controller button and what it sends. Point at one, pull the trigger, then press a key on the
  keyboard (or pick Mouse L / Mouse R / Wheel / None at the bottom). For Shift + F: press Shift, then F.
  **Normal / With modifier** at the top: "With modifier" shows and sets what each button sends while the modifier is held
  ("as normal" = no entry). **Modifier: ...** picks the modifier button (each press: the next button, or off).
- **Mouse** tab: which stick moves the mouse, its speed and dead zone, the air mouse button, hold or toggle, its speed.
- **Panels** tab: menu size, keyboard size, show / hide the keyboard, bring both in front of you.
- **Panels** tab also: the distance of each panel (Closer / Farther).
- **Driving** tab: driving on / off, the stick that steers (left / right / off), the dead zone, curve, how far the wheel
  turns, how quickly it follows the stick and comes back, its top speed, and speed-sensitive steering. Gas and brake: any button, Buttons tab. See [Driving](#driving-with-one-controller).
- **Look** tab: colours (Dark, Blue, Grey, Light, Black, Green), how see-through the menu and the keyboard are, and
  **pointer smoothing** (Off / Low / Medium / High): calms a shaky hand, for the menu pointer and the air mouse.
- **Reset tab** (bottom right of every tab): puts that tab's settings back to the defaults. Press it twice (the first
  press asks "Sure?"). Only that tab changes. On the Panels tab it also brings the menu (and the keyboard, if it is shown) in front of you.
- **Keyboard** (button at the top of the menu): a full keyboard with F-keys, arrows and number pad. A key goes down when
  you pull the trigger and up when you let go. Shift, Ctrl and Alt stay on for the next key.
  **A button for the keyboard:** Buttons tab, point at a button, pick **Keyboard** in the bottom bar. Then that button
  shows / hides the keyboard (`y = keyboard` in `config.ini`; it needs the VR menu on: `vr_menu = 1`). The keyboard **stays** when the menu closes, also at the next game start (put it at the side of your cab). Its x hides it.
  **Full or Compact** (Panels tab): Compact has only the main keys (Esc / F-keys down to Enter, Shift, Ctrl), with its
  only an x (above Backspace) to hide it: about 30 % narrower. Its size: Panels tab > Keyboard size.
- **Move** a panel: point at it, **hold the grip** and move the controller. While you hold it, push that controller's stick
  up / down to move it farther / closer. **Size**: the - and + at the top.

Every change works at once and is saved in `config.ini`. The menu uses no keyboard key, so it never clashes with the
ETS2 / ATS DLAA mod's menu (`Delete`). The menu and keyboard float in the room: after a recenter they move with it.

### Screenshots

| Buttons tab: picking a key | Buttons tab: with the modifier | Mouse tab |
|---|---|---|
| ![The Buttons tab while a key is picked](docs/images/menu-pick.png) | ![The Buttons tab, with modifier](docs/images/menu-modifier.png) | ![The Mouse tab](docs/images/menu-mouse.png) |

| Panels tab | Look tab | The keyboard |
|---|---|---|
| ![The Panels tab](docs/images/menu-panels.png) | ![The Look tab](docs/images/menu-look.png) | ![The on-screen keyboard](docs/images/keyboard.png) |

| Driving tab | The compact keyboard (Panels tab > Compact) |
|---|---|
| ![The Driving tab](docs/images/menu-driving.png) | ![The compact keyboard](docs/images/keyboard-compact.png) |

## Install

1. Close the game.
2. Unzip the folder to a place where it can stay (for example `C:\ETS2 ATS VR Controls`).
3. Right-click `install.ps1` > **Run with PowerShell**. Click **Yes** in the Windows admin prompt.
   (Admin is needed: ETS2 / ATS only load VR add-ons that are registered for all users.)
4. Start the game in VR.

Uninstall: run `uninstall.ps1` the same way. Then you can delete the folder.

### What `install.ps1` does, and why there is one

**Why a script?** This mod is not a game mod (`.scs` file) and does not go into the game folder. It is an *OpenXR API
layer*: a small add-on that the VR system (OpenXR) loads into the game when the game starts VR. OpenXR finds add-ons through
**one entry in the Windows registry**. For driving, the same DLL is also the game's own kind of input add-on, and the game
finds those through **one registry entry per game**. The script writes these entries for you, so you do not have to edit
the registry by hand.

**What it installs:** nothing is copied into Windows or into the game. It writes three registry values:

| Where | Name | Value |
|---|---|---|
| `HKEY_LOCAL_MACHINE\SOFTWARE\Khronos\OpenXR\1\ApiLayers\Implicit` | the full path of `XR_APILAYER_JRDEVO_ets2_ats_vr_controls.json` in your mod folder | `0` (= switched on) |
| `HKEY_LOCAL_MACHINE\SOFTWARE\SCS Software\Euro Truck Simulator 2\Plugins` | `ets2_ats_vr_controls` | the full path of `ets2_ats_vr_controls.dll` (driving) |
| `HKEY_LOCAL_MACHINE\SOFTWARE\SCS Software\American Truck Simulator\Plugins` | `ets2_ats_vr_controls` | the same (driving) |

The `.json` file tells OpenXR the add-on's name and where `ets2_ats_vr_controls.dll` is (next to it). That is why the folder
must stay where you unzipped it: the registry points to it.

**How it works, step by step:**

1. It stops if ETS2 or ATS is running (the DLL would be in use).
2. It removes an old per-user entry of this mod, if there is one (that would load the mod twice).
3. It asks for admin rights: you see one Windows prompt (UAC). This is needed because the entry is under
   `HKEY_LOCAL_MACHINE` (for all users): the OpenXR loader built into ETS2 / ATS only looks there.
4. It removes an older entry of this mod from another folder (if you moved the folder), then writes the new one. The same
   for the two driving entries (the keys are made if they are missing, so a game you install later finds it too).
5. Done. The next time the game starts VR, OpenXR loads the mod. In every other VR game the mod does nothing (see `apps`
   in `config.ini`).

`uninstall.ps1` removes these registry entries again (also with one admin prompt). Your files and `config.ini` stay; delete
the folder yourself if you like. You can check the entries yourself with `regedit` at the paths above.

**"Run with PowerShell" is not there, or Windows blocks the script?** Open PowerShell in the mod folder and run:

```
powershell -ExecutionPolicy Bypass -File .\install.ps1
```

## What runs when / privacy

- **Nothing runs in the background.** No service, no tray icon, no startup entry. With no VR game running, Task Manager
  shows nothing from this mod.
- **It only starts with a VR game.** The game's own VR system (OpenXR) loads it when the game starts VR, the same way it
  loads OpenXR Toolkit or other VR add-ons. When the game closes, the mod is gone too.
- **Other VR games:** it checks the game's name once at start (less than a millisecond), then switches itself off for the
  rest of that game: no work, no log, no buttons. (The small DLL stays in that game's memory until it closes.)
- **No internet.** It never connects anywhere.
- **Only its own files:** it reads `config.ini` and writes its own log and the settings you change in its menu.
- **Keys only to the game:** it sends keys / mouse only while the game window is in front. It does not look at other
  programs or at what you type outside the game.
- **One install, both games:** the registry entry is for the whole PC, so it works in ETS2 and ATS (the `apps` line in
  `config.ini` lists both). All of the source code is in this repository.

## Settings

Use the VR menu, or open `config.ini` in Notepad. Every setting is explained in the file. Restart the game after a change
in Notepad (the VR menu's changes work at once).

Button names: `ltrigger rtrigger lgrip rgrip x y a b lstick rstick menu lup ldown lleft lright rup rdown rleft rright`.
Values: a key (`h`, `f1`, `space`, `[`), up to 3 keys with `+` (`shift+f`), `mouse_left`, `mouse_right`, `mouse_middle`,
`wheel_up`, `wheel_down`, `gas`, `brake` (driving), `keyboard` (shows / hides the VR keyboard) or `none`.

## Problems

The log is `logs\ets2_ats_vr_controls.log` next to the DLL. It says which controller the VR system uses, which buttons are mapped,
and every problem. The log is **off by default** (no file, nothing measured: the lightest). Set `log = 1` in `config.ini`
to write it, and also `log_inputs = 1` to see every key the layer sends.

<a name="reporting-a-problem-what-to-upload"></a>
### Reporting a problem (what to upload)

1. In `config.ini` (next to `ets2_ats_vr_controls.dll`) set `log = 1` (and `log_inputs = 1` if a button does the wrong thing).
2. Start the game in VR and do what goes wrong for 1 to 2 minutes.
3. Quit the game.
4. Open a [bug report](../../issues/new/choose) and drag `logs\ets2_ats_vr_controls.log` and `config.ini` into it.

## Build from source

Visual Studio 2022 (C++), CMake 3.20+. The OpenXR SDK is downloaded on the first configure.

```
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
powershell -ExecutionPolicy Bypass -File tests\run_unit.ps1
powershell -ExecutionPolicy Bypass -File tests\run_e2e.ps1
```

`tests\run_e2e.ps1` runs a fake game through the real OpenXR loader, this layer and a fake VR runtime with scripted button
presses. No headset is needed, and no real key presses are sent.

## License

PolyForm Noncommercial 1.0.0, see `LICENSE.md`.
