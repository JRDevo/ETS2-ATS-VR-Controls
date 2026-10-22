// profiles -- see profiles.h.
#include "profiles.h"
#include <cstring>

namespace ck {
namespace {

// per profile, per hand: the input component of each Act (nullptr = none). Every path must exist in its profile: one wrong
// path makes the runtime refuse the whole suggestion for that profile.
struct ProfileDef {
    const char* path;
    const char* in[2][kActCount];   // [hand][act]: trigger, grip, primary, secondary, stick_click, menu, stick, aim
};
#define TOUCH { { "trigger/value", "squeeze/value", "x/click", "y/click", "thumbstick/click", "menu/click", "thumbstick", "aim/pose" }, \
                { "trigger/value", "squeeze/value", "a/click", "b/click", "thumbstick/click", nullptr,      "thumbstick", "aim/pose" } }
const ProfileDef kProfiles[] = {
    // Oculus / Meta Touch: the right "menu" is the reserved system (Meta) button
    { "/interaction_profiles/oculus/touch_controller", TOUCH },
    { "/interaction_profiles/valve/index_controller",
      { { "trigger/value", "squeeze/value", "a/click", "b/click", "thumbstick/click", nullptr, "thumbstick", "aim/pose" },
        { "trigger/value", "squeeze/value", "a/click", "b/click", "thumbstick/click", nullptr, "thumbstick", "aim/pose" } } },
    // Vive wands: the trackpad is the stick, the right menu is B
    { "/interaction_profiles/htc/vive_controller",
      { { "trigger/value", "squeeze/click", nullptr, nullptr, "trackpad/click", "menu/click", "trackpad", "aim/pose" },
        { "trigger/value", "squeeze/click", nullptr, "menu/click", "trackpad/click", nullptr, "trackpad", "aim/pose" } } },
    // Windows Mixed Reality: trackpad click = A / X, the right menu is B
    { "/interaction_profiles/microsoft/motion_controller",
      { { "trigger/value", "squeeze/click", "trackpad/click", nullptr, "thumbstick/click", "menu/click", "thumbstick", "aim/pose" },
        { "trigger/value", "squeeze/click", "trackpad/click", "menu/click", "thumbstick/click", nullptr, "thumbstick", "aim/pose" } } },
    { "/interaction_profiles/hp/mixed_reality_controller",
      { { "trigger/value", "squeeze/value", "x/click", "y/click", "thumbstick/click", "menu/click", "thumbstick", "aim/pose" },
        { "trigger/value", "squeeze/value", "a/click", "b/click", "thumbstick/click", nullptr,      "thumbstick", "aim/pose" } } },
    { "/interaction_profiles/khr/simple_controller",
      { { "select/click", nullptr, nullptr, nullptr,      nullptr, "menu/click", nullptr, "aim/pose" },
        { "select/click", nullptr, nullptr, "menu/click", nullptr, nullptr,      nullptr, "aim/pose" } } },
};
#undef TOUCH

const char* const kActNames[kActCount] = { "trigger", "grip", "primary", "secondary", "stick_click", "menu", "stick", "aim" };
const char* const kActLocalized[kActCount] = { "Trigger (Controller Keys)", "Grip (Controller Keys)", "A / X (Controller Keys)",
                                               "B / Y (Controller Keys)", "Stick click (Controller Keys)", "Menu (Controller Keys)",
                                               "Stick (Controller Keys)", "Aim (Controller Keys)" };
}  // namespace

const char* ActName(int a) { return a >= 0 && a < kActCount ? kActNames[a] : "?"; }
const char* ActLocalized(int a) { return a >= 0 && a < kActCount ? kActLocalized[a] : "?"; }

const std::vector<const char*>& KnownProfiles() {
    static const std::vector<const char*> v = [] {
        std::vector<const char*> p;
        for (const ProfileDef& d : kProfiles) p.push_back(d.path);
        return p;
    }();
    return v;
}

bool IsKnownProfile(const std::string& profile) {
    for (const ProfileDef& d : kProfiles) if (profile == d.path) return true;
    return false;
}

bool ActPath(const std::string& profile, int a, int h, std::string& path) {
    if (a < 0 || a >= kActCount || h < 0 || h > 1) return false;
    for (const ProfileDef& d : kProfiles) {
        if (profile != d.path) continue;
        const char* c = d.in[h][a];
        if (!c) return false;
        path = std::string(h ? "/user/hand/right/input/" : "/user/hand/left/input/") + c;
        return true;
    }
    return false;
}

const std::vector<const char*>& SimpleTranslationTargets() {
    static const std::vector<const char*> t = {
        "/interaction_profiles/oculus/touch_controller", "/interaction_profiles/valve/index_controller",
        "/interaction_profiles/htc/vive_controller", "/interaction_profiles/microsoft/motion_controller",
        "/interaction_profiles/hp/mixed_reality_controller" };
    return t;
}

bool TranslateSimpleBinding(const std::string& target, const std::string& p, std::string& out) {
    std::string hand;
    if (p.compare(0, 16, "/user/hand/left/") == 0) hand = "left";
    else if (p.compare(0, 17, "/user/hand/right/") == 0) hand = "right";
    else return false;
    const std::string top = "/user/hand/" + hand + "/", rest = p.substr(top.size());
    if (rest == "input/grip/pose" || rest == "input/aim/pose" || rest == "output/haptic") { out = p; return true; }
    const bool touch = target == "/interaction_profiles/oculus/touch_controller";
    const bool index = target == "/interaction_profiles/valve/index_controller";
    if (rest == "input/select/click" || rest == "input/select") { out = top + "input/trigger/value"; return true; }
    if (rest == "input/menu/click" || rest == "input/menu") {
        if (index) { out = top + "input/b/click"; return true; }
        if (touch && hand == "right") return false;   // the right "menu" is the system button there
        out = top + "input/menu/click";
        return true;
    }
    return false;
}

std::string ShortProfile(const std::string& p) {
    const char* pre = "/interaction_profiles/";
    return p.compare(0, strlen(pre), pre) == 0 ? p.substr(strlen(pre)) : p;
}

}  // namespace ck
