// profiles -- which OpenXR input each of our actions reads on each controller profile, and the khr/simple_controller
// translation. Pure logic (no OpenXR calls); unit-tested by tests/profiles_test.cpp.
//
// Our 8 actions (each with the subaction paths /user/hand/left and /user/hand/right):
//   trigger (float), grip (float), primary (bool: X left / A right), secondary (bool: Y left / B right),
//   stick_click (bool), menu (bool, left), stick (vector2), aim (pose)
// Controllers without A / B (Vive wands, Windows MR, simple): the right menu button is "secondary" (B) there.
#pragma once
#include <string>
#include <vector>

namespace ck {

enum Act { kActTrig, kActGrip, kActPrim, kActSec, kActClick, kActMenu, kActStick, kActAim, kActCount };
const char* ActName(int a);           // "trigger" ... (the OpenXR action names)
const char* ActLocalized(int a);      // "Trigger" ... (the localized names, with the mod's name)

// The interaction profiles the layer knows, in order of preference for a game that suggests none.
const std::vector<const char*>& KnownProfiles();
bool IsKnownProfile(const std::string& profile);
// The full input path of action `a` for hand `h` (0 left, 1 right) on `profile`, e.g. "/user/hand/left/input/trigger/value";
// false = that controller has no such input (or the profile is unknown).
bool ActPath(const std::string& profile, int a, int h, std::string& path);

// ETS2 / ATS suggest ONLY khr/simple_controller: no profile with sticks or A / B. The layer then also suggests these real
// controller profiles, each with the game's own bindings translated (so the runtime picks the real controller and the game keeps
// its buttons) plus ours.
const std::vector<const char*>& SimpleTranslationTargets();
// The game's simple_controller binding `simplePath` on `targetProfile`: select -> trigger, menu -> menu (b on Index; the right
// menu does not exist where it is the system button), grip / aim pose and haptics unchanged. false = no equivalent.
bool TranslateSimpleBinding(const std::string& targetProfile, const std::string& simplePath, std::string& out);

std::string ShortProfile(const std::string& p);   // "/interaction_profiles/oculus/touch_controller" -> "oculus/touch_controller"
// Whether two binding paths read the same physical input: the same "<top>/input/<identifier>", whatever the component
// ("/user/hand/right/input/b/click" and ".../b/touch" or ".../b" = the same; "a/click" and "b/click" = not).
bool SameInput(const std::string& a, const std::string& b);

}  // namespace ck
