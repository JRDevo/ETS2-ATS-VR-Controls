# ETS2 ATS VR Controls -- working rules for Claude sessions

An implicit OpenXR API layer (`ets2_ats_vr_controls.dll`, `XR_APILAYER_JRDEVO_ets2_ats_vr_controls`) that turns VR controller buttons,
sticks and aim into keys / mouse (SendInput) for ETS2 / ATS. Split out of the ETS2 DLAA injector on 2026-10-23 (scope: that mod
is DLAA / DLSS only). The OpenXR input approach is the one of VR EyeCapture's controller hotkey (`E:\VR-EyeCapture\src\vr_input.cpp`).
Read `README.md` and the header comments of `src/pad_input.h` and `src/layer.cpp` first.

## Build / test
- CMake is NOT on PATH:
  `"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release`
- First configure without internet: `-DFETCHCONTENT_SOURCE_DIR_OPENXR_SDK=E:/VR-EyeCapture/build32/_deps/openxr_sdk-src`.
- 0 warnings (/W4) is the bar. Tests: `tests\run_unit.ps1` (CPU), `tests\run_e2e.ps1` (fake game + loader + layer + mock runtime,
  CPU, no real input sent: VRC_TEST_EVENTS). The e2e script turns every installed implicit layer off through its
  disable_environment (Virtual Desktop, VectorXR, EyeCapture break the mock runtime).

## Install rules
- The game loads the ROOT copies `E:\ETS2-ATS-VR-Controls\ets2_ats_vr_controls.dll` + json (scripts\install.ps1 copies them from
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

## Roadmap (set by the user 2026-10-23)
1. **Phase 1 (v0.1, built):** controller buttons -> keys, stick mouse + air mouse + trigger click, for the game's menus.
2. **Phase 2:** an ON-SCREEN keyboard in the headset: full size, the user moves / resizes / hides it; point + trigger presses a
   key (F1, 1..9 cameras, ...). Drawn by the layer as an OpenXR quad layer (no game graphics hooks).
3. **Phase 3:** single buttons ("hot boxes") the user places anywhere (e.g. over a cab switch); pointing + trigger on one presses
   its key. Room space: a seated player's cab does not move; a recenter moves them.
- Own settings must never use keyboard keys (no clash with the DLAA mod's Delete / arrows / Ctrl-Shift-Alt+F10..F12): a
  controller combo opens this mod's own VR panel (planned with phase 2).
