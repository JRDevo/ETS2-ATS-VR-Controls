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
    CHECK(c.enabled && !c.logInputs, "enabled, no input log");
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
    printf("%d checks, %d failed\n", g_checks, g_fails);
    return g_fails ? 1 : 0;
}
