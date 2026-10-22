# VR Controller Keys -- working rules for Claude sessions

An implicit OpenXR API layer (`controller_keys.dll`, `XR_APILAYER_JRDEVO_controller_keys`) that turns VR controller buttons,
sticks and aim into keys / mouse (SendInput) for ETS2 / ATS. Split out of the ETS2 DLAA injector on 2026-10-23 (scope: that mod
is DLAA / DLSS only). The OpenXR input approach is the one of VR EyeCapture's controller hotkey (`E:\VR-EyeCapture\src\vr_input.cpp`).
Read `README.md` and the header comments of `src/pad_input.h` and `src/layer.cpp` first.

## Build / test
- CMake is NOT on PATH:
  `"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release`
- First configure without internet: `-DFETCHCONTENT_SOURCE_DIR_OPENXR_SDK=E:/VR-EyeCapture/build32/_deps/openxr_sdk-src`.
- 0 warnings (/W4) is the bar. Tests: `tests\run_unit.ps1` (CPU), `tests\run_e2e.ps1` (fake game + loader + layer + mock runtime,
  CPU, no real input sent: CK_TEST_EVENTS). The e2e script turns every installed implicit layer off through its
  disable_environment (Virtual Desktop, VectorXR, EyeCapture break the mock runtime).

## Install rules
- The game loads the ROOT copies `E:\VR-Controller-Keys\controller_keys.dll` + json (scripts\install.ps1 copies them from
  build\Release and registers HKLM). HKLM only -- ETS2 / ATS's built-in loader reads only HKLM; never also HKCU (the official
  loader then loads the layer twice).
- Never run `scripts\install.ps1` / `uninstall.ps1` without asking (UAC + HKLM). Refresh the root DLL only when no game runs.
- VR tests: install only; the user launches the game from the headset.

## Git
- Branch `master`. No remote yet (ask before creating the GitHub repo -- it is public). Commit with
  `-c user.name=JRDevo -c user.email=42043842+JRDevo@users.noreply.github.com`.

## Facts
- ETS2 / ATS: action set "input", suggest ONLY khr/simple_controller (select + menu). The layer translates that onto the real
  profiles (Touch, Index, Vive, WMR, HP) + ours, so the runtime picks the real controller.
- User: Quest 3 via Virtual Desktop (VDXR). Also installed: the DLAA mod (dinput8.dll, hooks OpenXR at the runtime too),
  VR EyeCapture (implicit layer), VectorXR.
- Idea queued: cab "hot boxes" -- boxes placed in the room space with the controller, drawn as OpenXR quad layers, a trigger
  pull on one sends its key (the cab does not move relative to a seated player; a recenter moves them).
