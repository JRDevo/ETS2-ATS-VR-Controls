# ETS2 ATS VR Controls -- working rules for Claude sessions

An implicit OpenXR API layer (`ets2_ats_vr_controls.dll`, `XR_APILAYER_JRDEVO_ets2_ats_vr_controls`) that turns VR controller buttons,
sticks and aim into keys / mouse (SendInput) for ETS2 / ATS. Split out of the ETS2 DLAA injector on 2026-10-23 (scope: that mod
is DLAA / DLSS only). The OpenXR input approach is the one of VR EyeCapture's controller hotkey (`E:\VR-EyeCapture\src\vr_input.cpp`).
Read `README.md` and the header comments of `src/pad_input.h`, `src/layer.cpp`, `src/vr_menu.h` and `src/overlay.h` first.

## Build / test
- CMake is NOT on PATH:
  `"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build build --config Release`
- First configure without internet: `-DFETCHCONTENT_SOURCE_DIR_OPENXR_SDK=E:/VR-EyeCapture/build32/_deps/openxr_sdk-src`.
- 0 warnings (/W4) is the bar. Tests: `tests\run_unit.ps1` (CPU; vr_menu_test writes the panel pictures to build\menu_test\*.bmp
  -- look at them after a layout change), `tests\run_e2e.ps1` (fake game + loader + layer + mock runtime, CPU, no real input sent:
  VRC_TEST_EVENTS; case `menu` = a D3D11 WARP session, the mock dumps the quad swapchains to build\e2e\menu\quad_*.bmp). The e2e script turns every installed implicit layer off through its
  disable_environment (Virtual Desktop, VectorXR, EyeCapture break the mock runtime).

## Install rules
- The game loads the ROOT copies `E:\ETS2-ATS-VR-Controls\ets2_ats_vr_controls.dll` + json (scripts\install.ps1 copies them from
  build\Release and registers HKLM). HKLM only -- ETS2 / ATS's built-in loader reads only HKLM; never also HKCU (the official
  loader then loads the layer twice).
- Never run `scripts\install.ps1` / `uninstall.ps1` without asking (UAC + HKLM). Refresh the root DLL only when no game runs.
- VR tests: install only; the user launches the game from the headset.

## Git
- **Nothing personal on the internet (the user's rule):** before every push, tag, release or upload follow the user-level
  `privacy-check` skill. The only identity: `JRDevo <42043842+JRDevo@users.noreply.github.com>` (also this repo's local git
  config); `.git/hooks/pre-push` runs the check (a fresh clone: put the hook back first); never `--no-verify`.
- Branch `master`. Remote: https://github.com/JRDevo/ETS2-ATS-VR-Controls (PUBLIC since v0.1.0, 2026-10-23). Issues: bug /
  feature templates only (no blank issues); needs-logs / needs-info bots as in the DLAA repo; CI = build.yml (build + all tests). Commit with
  `-c user.name=JRDevo -c user.email=42043842+JRDevo@users.noreply.github.com`.

## Facts
- ETS2 / ATS: action set "input", suggest ONLY khr/simple_controller (select + menu). The layer translates that onto the real
  profiles (Touch, Index, Vive, WMR, HP) + ours, so the runtime picks the real controller.
- Only the game's MENU button is left to the game; every other button is shared. Other API layers above ours (VR
  EyeCapture ...) reach our hooks like the game, so never key a rule on "the game binds it" (that dropped the stick clicks once).
- Driving (2026-10-23; steer = stick, gas / brake = button outputs): the same DLL is also an SCS SDK input plugin (scs_input_init: a SEMANTICAL device,
  floats steering / aforward / abackward; no binding in the game needed). The game loads it from
  HKLM\SOFTWARE\SCS Software\<game>\Plugins (install.ps1); the OpenXR loader then gets the same module (same path), so the
  values are shared directly. e2e case `drive`: fake_game --scs plays the game's plugin loader.
- v0.2.0 (2026-10-24, tested in the game by the user): steer_with = stick / wheel (steer_wheel_grip both / left / right /
  either: one hand moves round a rim) / point (steer_point_hand); steer_stick_too; a grip that holds the wheel is ONLY the
  wheel (Mapper::WheelHands). Four button layers: modifier (rgrip) + modifier2 (lgrip). README screenshots come from
  vr_menu_test (build\menu_test\shots\*.bmp -> docs\images\*.png; keyboard-pointer gets the dot from shot_pointer.txt).
- v0.3.0 languages (2026-10-28; the DLAA mod's phase-74 system, E:\ETS2-DLAA-Injector\src\lang.*, here UTF-8): src/lang.h
  (TR / TRF / N_, lang/*.po packed into the DLL by scripts/lang_tool.py; auto = the game's game.log.txt / config.cfg), and
  src/kb_layout.h (the keyboard's labels + shape ANSI / ISO / ABNT2 / JIS per language from src/kb_layouts.inc, made by
  tools/kbd_dump.cpp from Windows' own KBD*.DLL tables; never LoadKeyboardLayout). New menu text: TR("...") then
  `lang_tool.py extract` + `update`, translate all 41 .po; vr_menu_test fails when any text does not fit its box.
  Review files: `lang_tool.py review translation_review`.
- Short README per language (2026-10-29, for searches in each language): docs/readme/README.<code>.md = translations of
  docs/readme/TEMPLATE.md (no versions in it; a template change = the same change in all 41), linked from the line under
  README.md's banner. The full guide stays English.
- User: Quest 3 via Virtual Desktop (VDXR). Also installed: the DLAA mod (dinput8.dll, hooks OpenXR at the runtime too),
  VR EyeCapture (implicit layer), VectorXR.

## Roadmap (set by the user 2026-10-23)
1. **Phase 1 (built; released in v0.1.0):** controller buttons -> keys, stick mouse + air mouse + trigger click, for the game's menus.
2. **Phase 2 (built and tested in the game; released in v0.1.0 with driving):** the VR menu (both grips + left stick click 1 s) + a full ON-SCREEN keyboard in
   the headset; point + trigger; grab with the grip (stick = farther / closer), - / + size, distance, themes + opacity (Look tab);
   the keyboard is its own panel and stays (keyboard_show); the menu sets every button (keys picked on the keyboard), the
   mouse, air hold / toggle, and writes config.ini at once. OpenXR quad layers on D3D11 swapchains (no game graphics hooks).
3. **Phase 3:** single buttons ("hot boxes") the user places anywhere (e.g. over a cab switch); pointing + trigger on one presses
   its key. Room space: a seated player's cab does not move; a recenter moves them.
- Own settings must never use keyboard keys (no clash with the DLAA mod's Delete / arrows / Ctrl-Shift-Alt+F10..F12): a
  controller combo (menu_combo, default lgrip+rgrip+lstick) opens this mod's own VR panel. NEVER both stick clicks: that is
  VR EyeCapture's record combo (it fires whatever else is held).
- The menu WRITES the root config.ini (the installed one, tracked in git): a dirty config.ini there can be the user's own
  settings -- never revert it without asking. run_unit.ps1 tests the staged / committed copy.
