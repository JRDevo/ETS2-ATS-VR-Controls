// scs_input -- one-controller driving through the game's own input plugin door (the SCS SDK input API, ETS2 / ATS 1.4x+).
// The same DLL is also an SCS plugin: the game loads it from the path in HKLM\SOFTWARE\SCS Software\<game>\Plugins (written by
// scripts\install.ps1) and calls scs_input_init. The OpenXR loader loads the same file later (same path = one module), so both
// sides share these values directly -- no shared memory, no driver, no game file, no DirectInput.
//   * Device: "semantical" (SCS_INPUT_DEVICE_TYPE_semantical): its inputs are named after the game's controls (steering,
//     aforward = throttle, abackward = brake) and work with no binding in the game's options (controls.sii has
//     "+ semantical.aforward?0" etc. in its mixes). Steering: -1 full left .. +1 full right; pedals 0 .. 1.
//   * The controllers' side (pad_map.h Mapper, the game's VR thread) only stores the values (DriveSet: three atomic floats,
//     on change). The game's main thread asks once per frame (event callback, first_in_frame) and gets ALL THREE every frame,
//     one event per call, then "not found" (a value not told in a frame may count as released: steering snapped back).
//   * Also a telemetry plugin (scs_telemetry_init: truck.speed, truck.input / effective.steering): the speed for
//     speed-sensitive steering (DriveSpeedKmh -> Mapper::SetSpeed); with log = 1 the log shows what the game made of our
//     steering ("drive: sent ... | game input ..., used ... | km/h"), 10 lines a second at most, and a "cab:" line with the
//     cabin / head offsets, the truck's orientation and our VR head pose (data for pinning panels to the cab).
//   * Only for a game in config.ini 'apps' with enabled = 1 (anything else: unsupported, nothing registered). The device is
//     registered whatever the settings say, so the VR menu can change steering / gas / brake at once (unused = 0).
#pragma once

namespace ck {

struct Config;
const Config* GameConfig();   // layer.cpp: config.ini of this program (read once); nullptr = not our game / enabled = 0

void DriveSet(float steer, float throttle, float brake);   // any thread
void DriveGet(float out[3]);                               // the values the game gets next (tests)
float DriveSpeedKmh();                                     // the truck's speed from the game's telemetry; -1 = not known
void VrHeadSet(const double p[3], const double q[4]);      // log = 1: the VR head pose (LOCAL) for the log's cab line

}  // namespace ck
