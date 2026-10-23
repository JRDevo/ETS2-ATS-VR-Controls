// Unit test: src/config.cpp -- the shipped config.ini parses with no notes and equals the built-in defaults (so a game without a
// config.ini behaves the same), the apps list, bad values keep the default and are noted, the air mouse button.
// Usage: config_test <path of config.ini>. Exit code 0 = every check passed.
#include <cstdio>
#include <string>
#include "config.h"

static int g_checks = 0, g_fails = 0;
#define CHECK(c, ...) do { ++g_checks; if (!(c)) { ++g_fails; if (g_fails <= 40) { printf("   FAIL: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)

static bool SameOut(const VrPad::Out& a, const VrPad::Out& b) {
    if (a.kind != b.kind || a.n != b.n) return false;
    for (int i = 0; i < a.n; ++i) if (a.code[i] != b.code[i]) return false;
    return true;
}

int main(int argc, char** argv) {
    using namespace ck;
    if (argc < 2) { printf("usage: config_test <config.ini>\n"); return 2; }
    std::string text;
    FILE* f = nullptr;
    if (fopen_s(&f, argv[1], "rb") || !f) { printf("cannot read %s\n", argv[1]); return 2; }
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) text.append(buf, n);
    fclose(f);
    const Config c = ParseConfig(text);
    for (const std::string& note : c.notes) printf("   note: %s\n", note.c_str());
    CHECK(c.notes.empty(), "shipped config.ini has %zu note(s)", c.notes.size());
    const Config d = ParseConfig(std::string());
    for (int b = 0; b < VrPad::kBtnCount; ++b) CHECK(SameOut(c.pad.map[b], d.pad.map[b]), "button %s differs from the default", VrPad::BtnNames()[b]);
    CHECK(c.pad.mouseStick == d.pad.mouseStick && c.pad.mouseSpeed == d.pad.mouseSpeed && c.pad.deadzone == d.pad.deadzone &&
          c.pad.airBtn == d.pad.airBtn && c.pad.airGain == d.pad.airGain, "mouse settings differ from the defaults");
    CHECK(c.enabled && !c.logInputs && !c.log, "enabled, no log file, no input log");
    CHECK(AppAllowed(c, L"EuroTrucks2.exe") && AppAllowed(c, L"amtrucks.exe") && !AppAllowed(c, L"hl2.exe"), "apps");

    const Config e = ParseConfig("apps = *\nmouse_stick = sideways\nmouse_speed = abc\nb = nokey\nair_mouse = lgrip\nfoo = 1\nnoequals\n"
                                 "deadzone = 5\nlog_inputs = 1\nenabled = 0\n");
    CHECK(AppAllowed(e, L"anything.exe"), "apps = *");
    CHECK(e.pad.mouseStick == d.pad.mouseStick && e.pad.mouseSpeed == d.pad.mouseSpeed, "bad values keep the default");
    CHECK(SameOut(e.pad.map[VrPad::kB], d.pad.map[VrPad::kB]), "bad key keeps the default");
    CHECK(e.pad.airBtn == VrPad::kLGrip && e.pad.airHand == 0, "air_mouse = lgrip -> left hand");
    CHECK(e.pad.deadzone > 0.89f && e.pad.deadzone < 0.91f, "deadzone clamped to 0.9");
    CHECK(e.logInputs && !e.enabled, "log_inputs / enabled");
    CHECK(e.notes.size() == 5, "5 notes (mouse_stick, mouse_speed, b, foo, noequals) -- got %zu", e.notes.size());
    const Config g = ParseConfig("air_mouse = lup\n");
    CHECK(g.pad.airBtn == d.pad.airBtn && g.notes.size() == 1, "a stick direction is not an air mouse button");
    const Config h = ParseConfig("apps = \n");
    CHECK(!AppAllowed(h, L"eurotrucks2.exe") && h.notes.size() == 1, "empty apps = nowhere, noted");
    // comments after a value
    const Config k = ParseConfig("a = h   # horn\nb = shift+f ; hazards\nx = ;\ny = ' # quote key\nmouse_speed = 500 # px\n");
    VrPad::Out o;
    VrPad::ParseOut("h", &o);
    CHECK(SameOut(k.pad.map[VrPad::kA], o), "a = h # comment");
    VrPad::ParseOut("shift+f", &o);
    CHECK(SameOut(k.pad.map[VrPad::kB], o), "b = shift+f ; comment");
    CHECK(k.pad.map[VrPad::kX].kind == VrPad::kKeys && k.pad.map[VrPad::kX].code[0] == 0x27, "x = ; is the ; key");
    CHECK(k.pad.map[VrPad::kY].kind == VrPad::kKeys && k.pad.map[VrPad::kY].code[0] == 0x28, "y = ' with a comment");
    CHECK(k.pad.mouseSpeed == 500.0f && k.notes.empty(), "mouse_speed = 500 # px (%zu notes)", k.notes.size());
    CHECK(!VrPad::ParseOut("pause", &o), "no pause key (it would be NumLock)");
    // the VR menu keys
    CHECK(c.menu && c.menuCombo == d.menuCombo && c.menuCombo.size() == 3 && c.menuCombo[2] == VrPad::kLStick && c.menuHold == 1.0f &&
          !c.pad.airToggle, "shipped: menu on, lgrip+rgrip+lstick, 1 s, air hold");
    CHECK(!c.menuPlace.set && !c.kbPlace.set, "shipped: no panel places");
    const Config mc = ParseConfig("menu_combo = a + b\nmenu_hold = 2\nair_mode = toggle\nmenu_place = 0.1, -0.2, -0.7, 15, -8, 0.6\n"
                                  "keyboard_place = 1, 2\nvr_menu = 0\n");
    CHECK(mc.menuCombo == std::vector<int>({ VrPad::kA, VrPad::kB }) && mc.menuHold == 2.0f && mc.pad.airToggle && !mc.menu, "menu keys");
    CHECK(mc.menuPlace.set && mc.menuPlace.x > 0.09 && mc.menuPlace.yaw == 15.0 && mc.menuPlace.width == 0.6, "menu_place");
    CHECK(!mc.kbPlace.set && mc.notes.size() == 1, "a short keyboard_place is noted (%zu notes)", mc.notes.size());
    const Config md = ParseConfig("menu_combo = lstick+lstick\nmenu_combo = lup+a\nair_mode = sometimes\n");
    CHECK(md.menuCombo == d.menuCombo && md.notes.size() == 3, "bad combos / air_mode noted");
    const Config me = ParseConfig("menu_combo = lgrip+rgrip+x+y\nmenu_combo = a+b+x+y+lstick\n");
    CHECK(me.menuCombo.size() == 4 && me.notes.size() == 1, "4 buttons ok, 5 noted");
    // the modifier: shipped = the defaults (right grip; a:mouse_left | b:mouse_right), parsing, the menu's write read back
    CHECK(c.pad.modBtn == d.pad.modBtn && d.pad.modBtn == VrPad::kRGrip, "shipped modifier = rgrip");
    for (int b = 0; b < VrPad::kBtnCount; ++b) CHECK(SameOut(c.pad.modMap[b], d.pad.modMap[b]), "modifier key %s differs", VrPad::BtnNames()[b]);
    CHECK(d.pad.modMap[VrPad::kB].kind == VrPad::kMouseR && d.pad.modMap[VrPad::kA].kind == VrPad::kMouseL &&
          d.pad.map[VrPad::kA].kind == VrPad::kMouseL && d.pad.map[VrPad::kRTrig].kind == VrPad::kNone && d.pad.mouseStick == 2,
          "defaults: A = left click, right trigger = none, modifier + A = left click, + B = right click, right stick = mouse");
    const Config mo = ParseConfig("modifier = lgrip\nmodifier_keys = a:shift+f | ltrigger:mouse_right | x:,\n");
    CHECK(mo.pad.modBtn == VrPad::kLGrip && mo.pad.modMap[VrPad::kA].n == 2 && mo.pad.modMap[VrPad::kLTrig].kind == VrPad::kMouseR &&
          mo.pad.modMap[VrPad::kX].kind == VrPad::kKeys && mo.pad.modMap[VrPad::kRTrig].kind == VrPad::kNone && mo.notes.empty(),
          "modifier = lgrip, 3 pairs (the , key too), the default pair replaced (%zu notes)", mo.notes.size());
    const Config mb = ParseConfig("modifier = lup\nmodifier_keys = a:nokey | b\nmodifier = off\n");
    CHECK(mb.pad.modBtn == -1 && mb.notes.size() == 3, "bad modifier / pairs noted, off works (%zu notes)", mb.notes.size());
    std::string w;
    for (const auto& kv : MenuValues(mo.pad, PanelPlace(), PanelPlace(), MenuLook())) w += kv.first + " = " + kv.second + "\n";
    const Config mr = ParseConfig(w);
    CHECK(mr.pad.modBtn == VrPad::kLGrip && SameOut(mr.pad.modMap[VrPad::kA], mo.pad.modMap[VrPad::kA]) &&
          SameOut(mr.pad.modMap[VrPad::kX], mo.pad.modMap[VrPad::kX]) && mr.notes.empty(), "the menu's modifier lines read back the same");
    // driving: shipped = defaults; parsing; the menu's lines read back
    CHECK(c.pad.drive == d.pad.drive && d.pad.drive && c.pad.driveBrake == d.pad.driveBrake && c.pad.steerDeadzone == d.pad.steerDeadzone &&
          c.pad.steerCurve == d.pad.steerCurve && c.pad.steerMax == d.pad.steerMax, "shipped driving = the defaults (on)");
    const Config dr = ParseConfig("drive = 0\ndrive_brake = 1\nsteer_deadzone = 0.2\nsteer_curve = 2\nsteer_max = 0.6\n");
    CHECK(!dr.pad.drive && dr.pad.driveBrake && dr.pad.steerDeadzone == 0.2f && dr.pad.steerCurve == 2.0f && dr.pad.steerMax == 0.6f &&
          dr.notes.empty(), "driving keys (%zu notes)", dr.notes.size());
    const Config dc = ParseConfig("steer_max = 5\nsteer_curve = x\n");
    CHECK(dc.pad.steerMax == 1.0f && dc.pad.steerCurve == 1.5f && dc.notes.size() == 1, "steer_max clamped, a bad curve noted (%zu notes)", dc.notes.size());
    std::string wd;
    for (const auto& kv : MenuValues(dr.pad, PanelPlace(), PanelPlace(), MenuLook())) wd += kv.first + " = " + kv.second + "\n";
    const Config drr = ParseConfig(wd);
    CHECK(!drr.pad.drive && drr.pad.driveBrake && drr.pad.steerCurve == 2.0f && drr.pad.steerMax == 0.6f && drr.notes.empty(),
          "the menu's driving lines read back the same");
    // OutToString is the inverse of ParseOut (the menu writes what it read)
    for (const char* t : { "shift+f", "mouse_left", "wheel_down", "none", "num7", "f12", "[", "\\", "ctrl+alt+delete", "rctrl" }) {
        VrPad::Out a1, a2;
        char ob[64];
        VrPad::ParseOut(t, &a1);
        VrPad::OutToString(a1, ob, sizeof(ob));
        CHECK(VrPad::ParseOut(ob, &a2) && SameOut(a1, a2), "OutToString round trip %s -> %s", t, ob);
    }
    printf("%d checks, %d failed\n", g_checks, g_fails);
    return g_fails ? 1 : 0;
}
