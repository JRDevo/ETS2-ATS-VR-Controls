---
name: Bug report
about: Something doesn't work (a button sends nothing, the VR menu does not open, the pointer / keyboard misbehaves, a crash)
title: "[Bug] "
labels: bug
---

<!-- BEFORE you write: in config.ini (next to ets2_ats_vr_controls.dll) change  log_inputs = 0  to  log_inputs = 1 . -->
<!-- Start the game in VR, do what goes wrong (1-2 minutes), QUIT the game, then attach the files below. -->

## What happens

<!-- What you did (which button, which menu page), what you expected, and what happened instead. -->

## Setup

- Game (ETS2 / ATS) and game version:
- Headset and how it connects (e.g. Quest 3 via Virtual Desktop, Index via SteamVR):
- Controllers (Touch, Index, Vive wands, Windows MR, ...):
- Mod version (top line of the log):
- Other VR add-ons (OpenXR Toolkit, VR EyeCapture, the DLAA mod, OpenKneeboard, ...):

## Files (drag them into this box)

Both are in the mod's folder (where you ran `install.ps1`).

- [ ] `logs\ets2_ats_vr_controls.log` (**required**, made with `log_inputs = 1`; no file = the mod did not load, say so)
- [ ] `config.ini` (**required**)
- [ ] A screenshot or short video (optional)
