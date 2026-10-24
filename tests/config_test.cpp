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
    CHECK(VrPad::ParseOut("pause", &o) && o.code[0] == (0x13 | VrPad::kVk) && o.code[0] != 0x45,
          "pause = the virtual key VK_PAUSE (never the scan code 0x45: that is NumLock)");
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
          d.pad.map[VrPad::kA].kind == VrPad::kMouseL && d.pad.map[VrPad::kRTrig].kind == VrPad::kGas && d.pad.map[VrPad::kLTrig].kind == VrPad::kBrake &&
          d.pad.modMap[VrPad::kRTrig].kind == VrPad::kMouseL && d.pad.mouseStick == 2,
          "defaults: A = left click, right trigger = gas, left trigger = brake, modifier + A / right trigger = left click, + B = right click");
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
    // two modifiers (four layers): defaults, parsing, the menu's lines read back; clashes refused
    CHECK(d.pad.modBtn2 == VrPad::kLGrip && mo.pad.modBtn2 == -1 && mo.notes.empty(),
          "modifier 2 = left grip by default; an old file with modifier = lgrip: modifier 2 quietly off");
    const Config m2 = ParseConfig("modifier = rgrip\nmodifier2 = x\nmodifier2_keys = a:f2 | b:mouse_middle\nmodifier_both_keys = a:f3 | y:keyboard\n");
    CHECK(m2.pad.modBtn2 == VrPad::kX && m2.pad.modMap2[VrPad::kA].kind == VrPad::kKeys && m2.pad.modMap2[VrPad::kB].kind == VrPad::kMouseM &&
          m2.pad.modMapBoth[VrPad::kA].kind == VrPad::kKeys && m2.pad.modMapBoth[VrPad::kY].kind == VrPad::kKeyboard &&
          m2.pad.modMapBoth[VrPad::kB].kind == VrPad::kNone && m2.notes.empty(), "modifier2 + its keys + the both keys (%zu notes)", m2.notes.size());
    std::string w2;
    for (const auto& kv : MenuValues(m2.pad, PanelPlace(), PanelPlace(), MenuLook())) w2 += kv.first + " = " + kv.second + "\n";
    const Config m2r = ParseConfig(w2);
    CHECK(m2r.pad.modBtn2 == VrPad::kX && SameOut(m2r.pad.modMap2[VrPad::kA], m2.pad.modMap2[VrPad::kA]) &&
          SameOut(m2r.pad.modMap2[VrPad::kB], m2.pad.modMap2[VrPad::kB]) && SameOut(m2r.pad.modMapBoth[VrPad::kY], m2.pad.modMapBoth[VrPad::kY]) &&
          m2r.pad.modMapBoth[VrPad::kB].kind == VrPad::kNone && m2r.notes.empty(), "the menu's layer lines read back the same");
    const Config ms = ParseConfig("modifier = rgrip\nmodifier2 = rgrip\n");
    const Config mk = ParseConfig("modifier = lstick\nmodifier2 = rstick\n");
    CHECK(ms.pad.modBtn2 == -1 && ms.notes.size() == 1 && mk.pad.modBtn2 == -1 && mk.notes.size() == 1,
          "modifier2 = modifier, or both stick clicks (VR EyeCapture): modifier 2 off, noted");
    // steering with the hands: stick by default; wheel / point and their settings; the menu's lines read back
    const Config sw = ParseConfig("steer_with = point\nsteer_point_angle = 40\nsteer_wheel_angle = 120\nsteer_aim_deadzone = 3\n"
                                  "steer_aim_smooth = 0.1\nsteer_point_hand = left\nsteer_point_button = x\n");
    CHECK(d.pad.steerWith == 0 && sw.pad.steerWith == 2 && sw.pad.steerPointAngle == 40.0f && sw.pad.steerWheelAngle == 120.0f &&
          sw.pad.steerAimDeadzone == 3.0f && sw.pad.steerAimSmooth == 0.1f && sw.pad.steerPointHand == 0 &&
          sw.pad.steerPointBtn == VrPad::kX && sw.notes.empty(), "steer_with = point and its settings (%zu notes)", sw.notes.size());
    std::string w3;
    for (const auto& kv : MenuValues(sw.pad, PanelPlace(), PanelPlace(), MenuLook())) w3 += kv.first + " = " + kv.second + "\n";
    const Config swr = ParseConfig(w3);
    CHECK(d.pad.steerStickToo && !ParseConfig("steer_stick_too = 0\n").pad.steerStickToo, "steer_stick_too: on by default, 0 = off");
    const Config wg = ParseConfig("steer_wheel_grip = either\nsteer_point_hand = left\n");
    std::string w4;
    for (const auto& kv : MenuValues(wg.pad, PanelPlace(), PanelPlace(), MenuLook())) w4 += kv.first + " = " + kv.second + "\n";
    const Config wgr = ParseConfig(w4);
    CHECK(d.pad.steerWheelGrip == 0 && wg.pad.steerWheelGrip == 3 && wg.pad.steerPointHand == 0 && wgr.pad.steerWheelGrip == 3 &&
          wgr.pad.steerPointHand == 0 && ParseConfig("steer_wheel_grip = feet\n").notes.size() == 1,
          "steer_wheel_grip (both by default) and steer_point_hand: parsed, written, read back; a bad word noted");
    CHECK(swr.pad.steerWith == 2 && swr.pad.steerPointAngle == 40.0f && swr.pad.steerWheelAngle == 120.0f && swr.notes.empty(),
          "the menu's steering lines read back the same");
    const Config sb = ParseConfig("steer_with = mouse\nsteer_point_angle = 5\nsteer_point_hand = up\n");
    CHECK(sb.pad.steerWith == 0 && sb.pad.steerPointAngle == 10.0f && sb.pad.steerPointHand == 1 && sb.notes.size() == 2,
          "bad steering words noted (defaults kept), a number out of range clamped (%zu notes)", sb.notes.size());
    // driving: shipped = defaults; parsing; the menu's lines read back
    CHECK(c.pad.steerStick == d.pad.steerStick && d.pad.steerStick == 1 && c.pad.steerDeadzone == d.pad.steerDeadzone &&
          c.pad.steerFine == d.pad.steerFine && c.pad.steerFineZone == d.pad.steerFineZone && c.pad.steerMax == d.pad.steerMax && c.pad.steerRate == d.pad.steerRate &&
          c.pad.drive == d.pad.drive && d.pad.drive && c.pad.steerAtSpeed == d.pad.steerAtSpeed &&
          c.pad.steerSpeedSens == d.pad.steerSpeedSens && d.pad.steerSpeedSens && c.pad.steerFollowTime == d.pad.steerFollowTime && c.pad.steerCenterTime == d.pad.steerCenterTime &&
          c.pad.map[VrPad::kRTrig].kind == VrPad::kGas, "shipped driving = the defaults (left stick steers, right trigger = gas)");
    const Config dr = ParseConfig("steer = right\nsteer_deadzone = 0.2\nsteer_fine = 0.3\nsteer_fine_zone = 0.7\nsteer_max = 0.6\nsteer_max_speed = 0.75\n"
                                  "steer_follow_time = 0.35\nsteer_center_time = 1.5\ndrive = 0\nsteer_at_speed = 0.45\nsteer_speed_sens = 0\n"
                                  "lgrip = brake\nmodifier_keys = rtrigger:brake | a:gas\n");
    CHECK(dr.pad.steerStick == 2 && dr.pad.steerDeadzone == 0.2f && dr.pad.steerFine == 0.3f && dr.pad.steerFineZone == 0.7f && dr.pad.steerMax == 0.6f &&
          dr.pad.steerRate == 0.75f && dr.pad.map[VrPad::kLGrip].kind == VrPad::kBrake && dr.pad.modMap[VrPad::kRTrig].kind == VrPad::kBrake &&
          dr.pad.modMap[VrPad::kA].kind == VrPad::kGas && dr.pad.steerFollowTime == 0.35f &&
          dr.pad.steerCenterTime == 1.5f && !dr.pad.drive && dr.pad.steerAtSpeed == 0.45f && !dr.pad.steerSpeedSens && dr.notes.empty(), "driving keys, gas / brake on buttons (%zu notes)", dr.notes.size());
    const Config old = ParseConfig("steer_mode = rate\nsteer_turn_time = 7\nsteer_return_time = 1\nsteer_speed = 0.2\n");
    CHECK(old.notes.size() == 4 && old.pad.steerFollowTime == d.pad.steerFollowTime && old.pad.steerCenterTime == d.pad.steerCenterTime &&
          old.pad.steerRate == d.pad.steerRate && old.notes[0].find("no longer used") != std::string::npos,
          "the old steering keys: a note each, the defaults used (%zu notes)", old.notes.size());
    const Config dc = ParseConfig("steer_max = 5\nsteer_curve = x\nsteer = up\n");
    CHECK(dc.pad.steerMax == 1.0f && dc.pad.steerFine == d.pad.steerFine && dc.pad.steerStick == 1 && dc.notes.size() == 2,
          "steer_max clamped, the old steer_curve and a bad stick noted (%zu notes)", dc.notes.size());
    std::string wd;
    for (const auto& kv : MenuValues(dr.pad, PanelPlace(), PanelPlace(), MenuLook())) wd += kv.first + " = " + kv.second + "\n";
    const Config drr = ParseConfig(wd);
    CHECK(!drr.pad.drive && drr.pad.steerAtSpeed == 0.45f && !drr.pad.steerSpeedSens && drr.pad.steerFollowTime == 0.35f && drr.pad.steerCenterTime == 1.5f &&
          drr.pad.steerStick == 2 && drr.pad.steerFine == 0.3f && drr.pad.steerFineZone == 0.7f && drr.pad.steerMax == 0.6f && drr.pad.steerRate == 0.75f &&
          drr.pad.map[VrPad::kLGrip].kind == VrPad::kBrake && drr.pad.modMap[VrPad::kRTrig].kind == VrPad::kBrake && drr.notes.empty(),
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
