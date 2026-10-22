// Unit test: src/profiles.cpp -- every action path is well formed and on the right hand, Touch has no right menu, the
// controllers without A / B put B on the right menu, the simple_controller translation (select -> trigger, menu -> menu / b on
// Index / none on the right Touch, poses and haptics unchanged, junk refused). Exit code 0 = every check passed.
#include <cstdio>
#include <string>
#include "profiles.h"

static int g_checks = 0, g_fails = 0;
#define CHECK(c, ...) do { ++g_checks; if (!(c)) { ++g_fails; if (g_fails <= 40) { printf("   FAIL: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)

int main() {
    using namespace ck;
    const std::string touch = "/interaction_profiles/oculus/touch_controller", index = "/interaction_profiles/valve/index_controller";
    const std::string vive = "/interaction_profiles/htc/vive_controller", simple = "/interaction_profiles/khr/simple_controller";
    std::string p;
    int paths = 0;
    for (const char* prof : KnownProfiles()) {
        CHECK(IsKnownProfile(prof), "%s known", prof);
        for (int a = 0; a < kActCount; ++a)
            for (int h = 0; h < 2; ++h) {
                if (!ActPath(prof, a, h, p)) continue;
                ++paths;
                const std::string top = h ? "/user/hand/right/input/" : "/user/hand/left/input/";
                CHECK(p.compare(0, top.size(), top) == 0, "%s %s hand %d: %s", prof, ActName(a), h, p.c_str());
                const bool pose = p.size() > 5 && p.compare(p.size() - 5, 5, "/pose") == 0;
                CHECK(pose == (a == kActAim), "%s %s: pose only on aim (%s)", prof, ActName(a), p.c_str());
                const bool vec = a == kActStick;
                const bool hasComp = p.find("/click") != std::string::npos || p.find("/value") != std::string::npos || pose;
                CHECK(vec ? !hasComp : hasComp, "%s %s: component (%s)", prof, ActName(a), p.c_str());
            }
        CHECK(ActPath(prof, kActAim, 0, p) && ActPath(prof, kActAim, 1, p), "%s has both aims", prof);
        CHECK(ActPath(prof, kActTrig, 0, p) && ActPath(prof, kActTrig, 1, p), "%s has both triggers", prof);
    }
    CHECK(paths > 60, "paths %d", paths);
    CHECK(!IsKnownProfile("/interaction_profiles/foo/bar") && !ActPath("/interaction_profiles/foo/bar", kActTrig, 0, p), "unknown");
    CHECK(ActPath(touch, kActPrim, 1, p) && p == "/user/hand/right/input/a/click", "touch A: %s", p.c_str());
    CHECK(ActPath(touch, kActSec, 0, p) && p == "/user/hand/left/input/y/click", "touch Y: %s", p.c_str());
    CHECK(!ActPath(touch, kActMenu, 1, p), "touch: no right menu");
    CHECK(ActPath(touch, kActMenu, 0, p) && p == "/user/hand/left/input/menu/click", "touch left menu");
    CHECK(ActPath(vive, kActSec, 1, p) && p == "/user/hand/right/input/menu/click", "vive right menu = B");
    CHECK(ActPath(vive, kActStick, 0, p) && p == "/user/hand/left/input/trackpad", "vive stick = trackpad");
    CHECK(!ActPath(vive, kActPrim, 0, p), "vive: no X");
    CHECK(ActPath(simple, kActTrig, 1, p) && p == "/user/hand/right/input/select/click", "simple trigger = select");
    CHECK(!ActPath(simple, kActStick, 0, p) && !ActPath(simple, kActGrip, 1, p), "simple: no stick / grip");
    CHECK(!ActPath(index, kActMenu, 0, p), "index: no menu");
    CHECK(!ActPath(touch, kActCount, 0, p) && !ActPath(touch, kActTrig, 2, p), "bad args");

    // simple_controller translation
    std::string o;
    CHECK(TranslateSimpleBinding(touch, "/user/hand/left/input/select/click", o) && o == "/user/hand/left/input/trigger/value", "select");
    CHECK(TranslateSimpleBinding(index, "/user/hand/right/input/select/click", o) && o == "/user/hand/right/input/trigger/value", "index select");
    CHECK(TranslateSimpleBinding(touch, "/user/hand/left/input/menu/click", o) && o == "/user/hand/left/input/menu/click", "menu left");
    CHECK(!TranslateSimpleBinding(touch, "/user/hand/right/input/menu/click", o), "touch right menu dropped");
    CHECK(TranslateSimpleBinding(index, "/user/hand/right/input/menu/click", o) && o == "/user/hand/right/input/b/click", "index menu = b");
    CHECK(TranslateSimpleBinding(vive, "/user/hand/right/input/menu/click", o) && o == "/user/hand/right/input/menu/click", "vive right menu");
    CHECK(TranslateSimpleBinding(vive, "/user/hand/right/input/aim/pose", o) && o == "/user/hand/right/input/aim/pose", "pose unchanged");
    CHECK(TranslateSimpleBinding(vive, "/user/hand/left/output/haptic", o) && o == "/user/hand/left/output/haptic", "haptic unchanged");
    CHECK(!TranslateSimpleBinding(vive, "/user/head/input/volume_up/click", o), "head refused");
    CHECK(!TranslateSimpleBinding(vive, "/user/hand/left/input/foo/click", o), "junk refused");
    for (const char* t : SimpleTranslationTargets()) CHECK(IsKnownProfile(t), "target %s known", t);
    CHECK(ShortProfile(touch) == "oculus/touch_controller" && ShortProfile("x") == "x", "short");
    printf("%d checks, %d failed\n", g_checks, g_fails);
    return g_fails ? 1 : 0;
}
