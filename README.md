# ETS2 ATS VR Controls

Use your VR controllers as a keyboard and mouse in **Euro Truck Simulator 2** and **American Truck Simulator** (OpenXR VR).
Press a controller button, and the game gets a key press. Move a stick or point the controller, and the mouse pointer moves.

> Status: v0.1.0, first version (phase 1: buttons, pointer and click). Tested with a fake game and a fake VR runtime.
> **Not tested in the real game yet.** Planned: phase 2, an on-screen keyboard in VR that you can move, resize and hide;
> phase 3, single buttons you place anywhere (for example over a cab switch) that press a key.

## What it does

- Each controller button sends a key or a mouse button that you choose in `config.ini`.
- One stick moves the mouse pointer (the left stick by default).
- **Air mouse**: hold the right grip and turn the controller. The pointer follows, like a laser pointer.
- The game's own VR controls (the trigger and the left menu button) still work.
- It only works in the games named in `config.ini`. In every other VR program it does nothing.
- It only sends input while the game window is in front. It never types into another program.

It is an OpenXR API layer: a small add-on that the VR system loads into the game. It does not change any game file, and it
works next to the ETS2 / ATS DLAA mod, VR EyeCapture, Virtual Desktop, OpenXR Toolkit and VectorXR.

## Default buttons

| Controller | Sends | In ETS2 / ATS (default keys) |
|---|---|---|
| Right trigger | left mouse click | click |
| Left trigger | right mouse click | |
| Right grip (hold) | air mouse | |
| A | Enter | |
| B | Esc | menu / back |
| X | `[` | left indicator |
| Y | `]` | right indicator |
| Right stick click | H | horn |
| Left stick click | F | hazard lights |
| Right stick up / down / left / right | arrow keys | |
| Left stick | moves the mouse pointer | |

Change them in `config.ini`. If you changed the game's key bindings, change `config.ini` to match.

**Cab buttons:** the game has no clickable buttons in the cab, so pointing at a switch does nothing. Map the key of that
function to a controller button instead (for example `y = l` for the lights).

## Install

1. Close the game.
2. Unzip the folder to a place where it can stay (for example `C:\ETS2 ATS VR Controls`).
3. Right-click `install.ps1` > **Run with PowerShell**. Click **Yes** in the Windows admin prompt.
   (Admin is needed: ETS2 / ATS only load VR add-ons that are registered for all users.)
4. Start the game in VR.

Uninstall: run `uninstall.ps1` the same way. Then you can delete the folder.

## Settings

Open `config.ini` in Notepad. Every setting is explained in the file. Restart the game after a change.

Button names: `ltrigger rtrigger lgrip rgrip x y a b lstick rstick menu lup ldown lleft lright rup rdown rleft rright`.
Values: a key (`h`, `f1`, `space`, `[`), up to 3 keys with `+` (`shift+f`), `mouse_left`, `mouse_right`, `mouse_middle`,
`wheel_up`, `wheel_down` or `none`.

## Problems

The log is `logs\ets2_ats_vr_controls.log` next to the DLL. It says which controller the VR system uses, which buttons are mapped,
and every problem. Set `log_inputs = 1` in `config.ini` to see every key the layer sends.

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
