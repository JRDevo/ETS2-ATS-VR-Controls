// Unit test: the VR menu + keyboard (src/vr_menu.cpp) with made-up controller frames -- no OpenXR, no game.
//   (1) the combo gate (both grips + left stick click): each alone goes through at once, both stick clicks (VR EyeCapture's
//       combo) are not ours, grips first then the stick = all taken out from that frame, the combo after 1 s, once.
//   (2) the menu: opens in front of the head; pointing + trigger on a tab / a setting / a button row; picking a key on the
//       keyboard (Shift latched + F = shift+f); typing a key (down on the trigger, up on its release); the trigger is taken out
//       of the frame while it points at a panel; moving a panel by its top bar; - / + size; close.
//   (3) config.ini: the changes are written into a copy (comments kept, new keys added at the end) and read back the same.
// Usage: vr_menu_test <work folder>. Writes menu.bmp / keyboard.bmp (the panel pictures) and config.ini there.
// Exit code 0 = every check passed.
#include <windows.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "config.h"
#include "vr_menu.h"

static int g_checks = 0, g_fails = 0;
#define CHECK(c, ...) do { ++g_checks; if (!(c)) { ++g_fails; if (g_fails <= 40) { printf("   FAIL: "); printf(__VA_ARGS__); printf("\n"); } } } while (0)

struct Rec : VrPad::Sink {
    std::vector<std::string> ev;
    void Key(uint16_t c, bool d) override { char b[32]; snprintf(b, sizeof(b), "k%03x%s", c, d ? "v" : "^"); ev.push_back(b); }
    void MouseButton(int w, bool d) override { char b[32]; snprintf(b, sizeof(b), "m%d%s", w, d ? "v" : "^"); ev.push_back(b); }
    void Wheel(int n) override { char b[32]; snprintf(b, sizeof(b), "w%+d", n); ev.push_back(b); }
    void Move(int, int) override {}
    std::string Take() { std::string s; for (auto& e : ev) { if (!s.empty()) s += ' '; s += e; } ev.clear(); return s; }
};

static std::string ReadFile(const std::wstring& p) {
    std::string t;
    FILE* f = nullptr;
    if (_wfopen_s(&f, p.c_str(), L"rb") || !f) return t;
    char b[4096];
    size_t n;
    while ((n = fread(b, 1, sizeof(b), f)) > 0) t.append(b, n);
    fclose(f);
    return t;
}
static void WriteBmp(const std::wstring& path, const ck::VrMenu::Panel& p) {
    FILE* f = nullptr;
    if (_wfopen_s(&f, path.c_str(), L"wb") || !f) return;
    BITMAPFILEHEADER fh{};
    BITMAPINFOHEADER ih{};
    ih.biSize = sizeof(ih); ih.biWidth = p.pw; ih.biHeight = -p.ph; ih.biPlanes = 1; ih.biBitCount = 32; ih.biCompression = BI_RGB;
    fh.bfType = 0x4D42;
    fh.bfOffBits = sizeof(fh) + sizeof(ih);
    fh.bfSize = fh.bfOffBits + (DWORD)(p.px.size() * 4);
    fwrite(&fh, sizeof(fh), 1, f);
    fwrite(&ih, sizeof(ih), 1, f);
    fwrite(p.px.data(), 4, p.px.size(), f);
    fclose(f);
}

// a frame with hand h aiming from the head (origin) at world point `at`, trigger `trig`
static VrPad::Frame Aim(int h, ck::V3 at, float trig) {
    VrPad::Frame f;
    f.hand[0] = f.hand[1] = true;
    const double len = ck::Len(at);
    double yaw, pitch;
    ck::YawPitchOf(at * (1.0 / len), &yaw, &pitch);
    const ck::Q4 q = ck::QYawPitch(yaw, pitch);
    f.aim[h] = f.pos[h] = true;
    f.q[h][0] = q.x; f.q[h][1] = q.y; f.q[h][2] = q.z; f.q[h][3] = q.w;
    f.trig[h] = trig;
    return f;
}

int main(int argc, char** argv) {
    using namespace ck;
    if (argc < 2) { printf("usage: vr_menu_test <work folder>\n"); return 2; }
    const std::string dirA = argv[1];
    const std::wstring dir(dirA.begin(), dirA.end());
    CreateDirectoryW(dir.c_str(), nullptr);

    printf("(1) combo gate\n");
    {
        ComboGate g;                                     // default: lgrip + rgrip + lstick
        VrPad::Frame f;
        double t = 0;
        auto step = [&](bool lg, bool rg, bool ls, bool rs) {
            f = VrPad::Frame();
            f.grip[0] = lg ? 1.0f : 0.0f; f.grip[1] = rg ? 1.0f : 0.0f; f.click[0] = ls; f.click[1] = rs;
            const bool done = g.Step(f, t);
            t += 0.011;
            return done;
        };
        // each button alone goes through at once (hazards, air mouse grip): nothing held back
        step(false, false, true, false);
        CHECK(f.click[0], "the left stick click alone goes through at once");
        step(false, true, false, false);
        CHECK(f.grip[1] == 1.0f, "the right grip alone goes through at once");
        step(false, false, false, false);
        // both stick clicks (VR EyeCapture's combo) are not ours
        bool done = false;
        for (int i = 0; i < 120; ++i) done = step(false, false, true, true) || done;
        CHECK(!done && f.click[0] && f.click[1], "both stick clicks: not the menu, both go through");
        step(false, false, false, false);
        // grips first (they go through), then the stick: from that frame on all three are taken out; the combo after 1 s
        step(true, true, false, false);
        CHECK(f.grip[0] == 1.0f && f.grip[1] == 1.0f, "grips go through before the stick");
        bool leak = false;
        const double t0 = t;
        for (int i = 0; i < 150 && !done; ++i) { done = step(true, true, true, false); leak = leak || f.click[0] || f.grip[0] > 0 || f.grip[1] > 0; }
        CHECK(done && !leak, "grips + left stick held 1 s = the combo, nothing leaks (%d %d)", done, leak);
        CHECK(t - t0 > 0.95 && t - t0 < 1.1, "the combo after ~1 s (%.2f)", t - t0);
        done = false;
        for (int i = 0; i < 20; ++i) done = step(true, true, true, false) || done;
        CHECK(!done, "fires once per hold");
        step(true, false, false, false);
        CHECK(f.grip[0] == 0.0f, "still taken out while any of them is held");
        step(false, false, false, false);
        step(false, false, true, false);
        CHECK(f.click[0], "all let go: the stick click is its own again");
        step(false, false, false, false);
    }

    printf("(1b) pointer smoothing\n");
    {
        // a hand held still with a 0.6 deg shake: the medium filter takes most of it out; a fast 30 deg turn is followed
        auto shake = [](int level, double* jitterOut, double* lagAfter) {
            PoseSmoother sm;
            static const double kMinCut[4] = { 0, 2.5, 1.2, 0.6 }, kBeta[4] = { 0, 0.04, 0.03, 0.02 };
            double prevYaw = 0, sum = 0;
            int n = 0;
            for (int i = 0; i < 200; ++i) {
                const double yaw = ((i * 7919) % 13 - 6) / 6.0 * 0.6 * 3.14159265358979 / 180.0;   // +-0.6 deg, every frame
                Pose in;
                in.q = QYawPitch(yaw, 0);
                const Pose o = level ? sm.Step(in, 0.011, kMinCut[level], kBeta[level]) : in;
                double y, p;
                YawPitchOf(QRot(o.q, V3{ 0, 0, -1 }), &y, &p);
                if (i > 100) { sum += std::fabs(y - prevYaw); ++n; }
                prevYaw = y;
            }
            *jitterOut = sum / n * 180.0 / 3.14159265358979;
            // then a 30 deg turn held: how far behind after 0.15 s
            Pose in;
            in.q = QYawPitch(30.0 * 3.14159265358979 / 180.0, 0);
            Pose o = in;
            for (int i = 0; i < 14; ++i) o = level ? sm.Step(in, 0.011, kMinCut[level], kBeta[level]) : in;
            double y, p;
            YawPitchOf(QRot(o.q, V3{ 0, 0, -1 }), &y, &p);
            *lagAfter = 30.0 - y * 180.0 / 3.14159265358979;
        };
        double j0, l0, j2, l2, j3, l3;
        shake(0, &j0, &l0);
        shake(2, &j2, &l2);
        shake(3, &j3, &l3);
        printf("   shake per frame: off %.3f deg, medium %.3f, high %.3f; behind after a fast 30 deg turn + 0.15 s: medium %.2f deg, high %.2f\n",
               j0, j2, j3, l2, l3);
        CHECK(j2 < j0 * 0.35 && j3 < j2, "medium takes most of the shake out, high more");
        CHECK(l2 < 1.5 && l3 < 3.0, "a fast turn is followed (medium %.2f deg, high %.2f deg behind)", l2, l3);
    }

    printf("(2) menu\n");
    const std::wstring cfgPath = dir + L"\\config.ini";
    {
        FILE* f = nullptr;
        _wfopen_s(&f, cfgPath.c_str(), L"wb");
        if (f) { fputs("# test\r\na = enter   # was\r\nmouse_speed = 900\r\n", f); fclose(f); }
    }
    Config cfg = ParseConfig("a = enter\npointer_smoothing = off\n");   // (the clicks below jump the aim in one frame)
    VrMenu m(cfg, cfgPath);
    Rec sink;
    double now = 10.0;
    Pose head;                                         // at the origin, looking down -z
    auto upd = [&](VrPad::Frame& f) { m.Update(f, true, true, head, now, sink); now += 0.011; };
    VrPad::Frame idle;
    idle.hand[0] = idle.hand[1] = true;
    // open with the combo
    for (int i = 0; i < 120 && !m.AnyShown(); ++i) { VrPad::Frame f = idle; f.grip[0] = f.grip[1] = 1.0f; f.click[0] = true; upd(f); }
    for (int i = 0; i < 3; ++i) { VrPad::Frame f = idle; upd(f); }   // (let go)
    CHECK(m.AnyShown() && m.Pan(VrMenu::kPanMenu).show && !m.Pan(VrMenu::kPanKb).show, "the combo opens the menu (keyboard hidden)");
    for (int i = 0; i < 3; ++i) { VrPad::Frame f = idle; upd(f); }
    const VrMenu::Panel& mp = m.Pan(VrMenu::kPanMenu);
    CHECK(mp.pose.p.z < -0.6 && mp.pose.p.z > -0.9 && std::fabs(mp.pose.p.x) < 0.01, "in front of the head (%.2f %.2f %.2f)", mp.pose.p.x, mp.pose.p.y, mp.pose.p.z);
    CHECK(mp.px.size() == (size_t)mp.pw * mp.ph && mp.version > 0, "the menu picture is drawn");
    // press = aim at the element, trigger down one frame, up the next; returns the frame the menu saw (trigger taken out?)
    auto click = [&](int h, int pan, int kind, int arg, bool* taken = nullptr) {
        V3 at;
        if (!m.ElemCentre(pan, kind, arg, &at)) { printf("   (element %d/%d/%d not shown)\n", pan, kind, arg); return false; }
        VrPad::Frame f = Aim(h, at, 0.0f);
        upd(f);
        f = Aim(h, at, 1.0f);
        upd(f);
        if (taken) *taken = f.trig[h] == 0.0f;
        f = Aim(h, at, 0.0f);
        upd(f);
        return true;
    };
    bool taken = false;
    CHECK(click(1, VrMenu::kPanMenu, VrMenu::eTab, 1, &taken), "Mouse tab shown");
    CHECK(taken, "the trigger is taken out of the frame on a panel");
    CHECK(m.Describe().find("tab 1") != std::string::npos, "Mouse tab selected (%s)", m.Describe().c_str());
    CHECK(click(1, VrMenu::kPanMenu, VrMenu::eSet, 41), "air toggle option shown");
    CHECK(m.Pad().airToggle, "air mouse: toggle");
    CHECK(click(0, VrMenu::kPanMenu, VrMenu::eSet, 11), "stick speed + shown (left hand)");
    CHECK(m.Pad().mouseSpeed > 1000.0f, "stick speed up (%.0f)", (double)m.Pad().mouseSpeed);
    CHECK(click(1, VrMenu::kPanMenu, VrMenu::eTab, 0), "Buttons tab");
    CHECK(click(1, VrMenu::kPanMenu, VrMenu::eRow, VrPad::kA), "row A");
    CHECK(m.Pan(VrMenu::kPanKb).show, "picking opens the keyboard");
    for (int i = 0; i < 2; ++i) { VrPad::Frame f = idle; upd(f); }
    const VrMenu::Panel& kp = m.Pan(VrMenu::kPanKb);
    CHECK(kp.pose.p.y < mp.pose.p.y - 0.15, "the keyboard below the menu (%.2f vs %.2f)", kp.pose.p.y, mp.pose.p.y);
    WriteBmp(dir + L"\\keyboard_pick.bmp", kp);
    WriteBmp(dir + L"\\menu_pick.bmp", mp);
    CHECK(click(1, VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("shift")), "Shift key");
    CHECK(click(1, VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("f")), "F key");
    char out[64];
    VrPad::OutToString(m.Pad().map[VrPad::kA], out, sizeof(out));
    CHECK(std::string(out) == "shift+f", "A = shift+f (%s)", out);
    CHECK(sink.Take().empty(), "picking sends no key");
    // typing: F1 down on the trigger, up at its release
    {
        V3 at;
        CHECK(m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("f1"), &at), "F1 shown");
        VrPad::Frame f = Aim(0, at, 0.0f);
        upd(f);
        f = Aim(0, at, 1.0f);
        upd(f);
        CHECK(sink.Take() == "k03bv", "F1 down on the trigger");
        for (int i = 0; i < 5; ++i) { f = Aim(0, at, 1.0f); upd(f); }
        CHECK(sink.Take().empty(), "held: nothing more");
        f = Aim(0, at, 0.0f);
        upd(f);
        CHECK(sink.Take() == "k03b^", "F1 up at the release");
    }
    // ctrl latched + c: ctrl down, c down ... c up, ctrl up
    click(1, VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("ctrl"));
    click(1, VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("c"));
    CHECK(sink.Take() == "k01dv k02ev k02e^ k01d^", "Ctrl + C (latched Ctrl)");
    // not on a panel: the trigger stays in the frame
    {
        VrPad::Frame f = Aim(1, V3{ 0, 2, 0.5 }, 1.0f);   // up and behind: nothing there
        upd(f);
        CHECK(f.trig[1] == 1.0f, "off the panels the trigger stays the game's");
        f = Aim(1, V3{ 0, 2, 0.5 }, 0.0f);
        upd(f);
    }
    // move the menu by its bar: grab, turn 20 deg left, let go
    {
        V3 at;
        m.ElemCentre(VrMenu::kPanMenu, VrMenu::eBar, 0, &at);
        const V3 before = m.Pan(VrMenu::kPanMenu).pose.p;
        VrPad::Frame f = Aim(1, at, 0.0f);
        upd(f);
        f = Aim(1, at, 1.0f);
        upd(f);
        const V3 turned = QRot(QYawPitch(20.0 * 3.14159265358979 / 180.0, 0), at);
        for (int i = 0; i < 5; ++i) { f = Aim(1, turned, 1.0f); upd(f); }
        f = Aim(1, turned, 0.0f);
        upd(f);
        const V3 after = m.Pan(VrMenu::kPanMenu).pose.p;
        CHECK(after.x < before.x - 0.15, "dragged to the left (x %.2f -> %.2f)", before.x, after.x);
        CHECK(std::fabs(Len(after) - Len(before)) < 0.01, "same distance");
    }
    // size
    const double w0 = m.Pan(VrMenu::kPanKb).width;
    click(1, VrMenu::kPanKb, VrMenu::eBigger, 0);
    CHECK(m.Pan(VrMenu::kPanKb).width > w0 * 1.1, "keyboard bigger");
    for (int i = 0; i < 2; ++i) { VrPad::Frame f = idle; upd(f); }
    // grab the keyboard with the right GRIP: it follows the controller, its stick pushes it away; the grip and the stick are
    // taken out of the frame meanwhile (no air mouse, no stick mouse / arrows)
    {
        V3 at;
        m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("g"), &at);
        const V3 before = m.Pan(VrMenu::kPanKb).pose.p;
        const double d0 = Len(before);
        VrPad::Frame f = Aim(1, at, 0.0f);
        upd(f);
        f = Aim(1, at, 0.0f);
        f.grip[1] = 1.0f;
        upd(f);
        CHECK(f.grip[1] == 0.0f, "the grip on a panel is the menu's (no air mouse)");
        const V3 turned = QRot(QYawPitch(-15.0 * 3.14159265358979 / 180.0, 0), at);   // 15 deg right
        for (int i = 0; i < 5; ++i) { f = Aim(1, turned, 0.0f); f.grip[1] = 1.0f; upd(f); }
        CHECK(m.Pan(VrMenu::kPanKb).pose.p.x > before.x + 0.1, "the held keyboard follows the controller (x %.2f -> %.2f)",
              before.x, m.Pan(VrMenu::kPanKb).pose.p.x);
        for (int i = 0; i < 30; ++i) { f = Aim(1, turned, 0.0f); f.grip[1] = 1.0f; f.sy[1] = 1.0f; upd(f); }
        CHECK(f.sy[1] == 0.0f, "the stick of the holding hand is the menu's");
        const double d1 = Len(m.Pan(VrMenu::kPanKb).pose.p);
        CHECK(d1 > d0 + 0.15, "stick up while holding = farther (%.2f -> %.2f m)", d0, d1);
        f = Aim(1, turned, 0.0f);
        upd(f);
        CHECK(!m.HandOnPanel(1) || m.Pan(VrMenu::kPanKb).show, "let go");
    }
    // a twist of the wrist while holding = the held keyboard twists too (it used to stay level); the twist is saved
    {
        V3 at;
        m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("h"), &at);
        auto twisted = [&](double deg, float grip) {
            VrPad::Frame f = Aim(1, at, 0.0f);
            const Q4 q = QNorm(QMul(Q4{ f.q[1][0], f.q[1][1], f.q[1][2], f.q[1][3] },
                                    Q4{ 0, 0, std::sin(deg * 3.14159265358979 / 360.0), std::cos(deg * 3.14159265358979 / 360.0) }));
            f.q[1][0] = q.x; f.q[1][1] = q.y; f.q[1][2] = q.z; f.q[1][3] = q.w;
            f.grip[1] = grip;
            upd(f);
        };
        twisted(0, 0.0f);
        twisted(0, 1.0f);
        for (int i = 0; i < 4; ++i) twisted(30, 1.0f);      // the wrist twisted 30 deg to the left
        const V3 up = QRot(m.Pan(VrMenu::kPanKb).pose.q, V3{ 0, 1, 0 });
        twisted(30, 0.0f);                                   // let go: saved
        for (int i = 0; i < 2; ++i) { VrPad::Frame f = idle; upd(f); }
        CHECK(up.x < -0.3, "the held keyboard twists with the wrist (its up %.2f %.2f %.2f)", up.x, up.y, up.z);
        const Config c2 = ParseConfig(ReadFile(cfgPath));
        CHECK(std::fabs(c2.kbPlace.roll - 30.0) < 5.0, "the twist is saved (about the tilted panel's own axis) (keyboard_place roll %.1f deg)", c2.kbPlace.roll);
        VrMenu m4(c2, std::wstring());
        const V3 up4 = QRot(m4.Pan(VrMenu::kPanKb).pose.q, V3{ 0, 1, 0 });
        CHECK(Len(up4 - up) < 0.02, "a new game: the keyboard comes back twisted the same way");
        twisted(0, 0.0f);
        twisted(0, 1.0f);
        twisted(0, 1.0f);
        twisted(0, 0.0f);                                    // (straight again for the tests below)
    }
    // Panels tab: the menu 5 cm farther; Look tab: theme blue, the menu more see-through
    {
        click(1, VrMenu::kPanMenu, VrMenu::eTab, 2);
        const double d0 = Len(m.Pan(VrMenu::kPanMenu).pose.p);
        CHECK(click(1, VrMenu::kPanMenu, VrMenu::eSet, 63), "Farther shown");
        CHECK(std::fabs(Len(m.Pan(VrMenu::kPanMenu).pose.p) - d0 - 0.05) < 0.005, "menu 5 cm farther");
        CHECK(click(1, VrMenu::kPanMenu, VrMenu::eTab, 3), "Look tab");
        CHECK(click(1, VrMenu::kPanMenu, VrMenu::eSet, 101), "Blue shown");
        CHECK(m.Look().theme == 1, "theme blue");
        for (int i = 0; i < 3; ++i) click(1, VrMenu::kPanMenu, VrMenu::eSet, 90);
        CHECK(std::fabs(m.Look().menuOpacity - 0.7f) < 0.01f, "menu 70 %% solid (%.2f)", (double)m.Look().menuOpacity);
        for (int i = 0; i < 2; ++i) { VrPad::Frame f = idle; upd(f); }
        const uint32_t px0 = m.Pan(VrMenu::kPanMenu).px[(size_t)400 * 1000 + 10];
        CHECK((px0 >> 24) == 179, "the picture is 70 %% solid (alpha %u)", px0 >> 24);
    }
    WriteBmp(dir + L"\\menu.bmp", m.Pan(VrMenu::kPanMenu));
    WriteBmp(dir + L"\\keyboard.bmp", m.Pan(VrMenu::kPanKb));
    // the pointer on the top bar first, then onto a button on the bar: the button gets it (the bar used to keep it)
    {
        V3 bar, minus;
        m.ElemCentre(VrMenu::kPanMenu, VrMenu::eBar, 0, &bar);
        m.ElemCentre(VrMenu::kPanMenu, VrMenu::eSmaller, 0, &minus);
        const double wm = m.Pan(VrMenu::kPanMenu).width;
        for (int i = 0; i < 3; ++i) { VrPad::Frame f = Aim(1, bar, 0.0f); upd(f); }
        VrPad::Frame f = Aim(1, minus, 0.0f);
        upd(f);
        f = Aim(1, minus, 1.0f);
        upd(f);
        f = Aim(1, minus, 0.0f);
        upd(f);
        CHECK(m.Pan(VrMenu::kPanMenu).width < wm * 0.95, "from the bar onto its - button: pressed (%.3f -> %.3f m)", wm, m.Pan(VrMenu::kPanMenu).width);
    }
    // Panels tab: the keyboard compact (main keys only, - / + / x in a column next to Enter), same key size, then full again
    {
        click(1, VrMenu::kPanMenu, VrMenu::eTab, 2);
        const VrMenu::Panel& kb = m.Pan(VrMenu::kPanKb);
        const int pw0 = kb.pw;
        const double wk = kb.width;
        CHECK(click(1, VrMenu::kPanMenu, VrMenu::eSet, 83), "Compact shown");
        WriteBmp(dir + L"\\menu_panels.bmp", m.Pan(VrMenu::kPanMenu));
        for (int i = 0; i < 2; ++i) { VrPad::Frame f = idle; upd(f); }
        V3 at;
        CHECK(kb.pw < pw0 * 0.75 && std::fabs(kb.width / kb.pw - wk / pw0) < 1e-9, "compact: narrower, same key size (%d px, %.3f m)", kb.pw, kb.width);
        CHECK(!m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("home"), &at) &&
              !m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("num5"), &at), "compact: no navigation / number pad");
        V3 x, bs, f12, tmp;
        CHECK(m.ElemCentre(VrMenu::kPanKb, VrMenu::eClose, 0, &x) && m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("backspace"), &bs) &&
              m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("f12"), &f12), "compact: x, Backspace, F12 there");
        CHECK(!m.ElemCentre(VrMenu::kPanKb, VrMenu::eSmaller, 0, &tmp) && !m.ElemCentre(VrMenu::kPanKb, VrMenu::eBigger, 0, &tmp),
              "compact: no - / + (the Panels tab sets its size)");
        const Q4 iq = QConj(kb.pose.q);
        const V3 lx = QRot(iq, x - kb.pose.p), lb = QRot(iq, bs - kb.pose.p), lf = QRot(iq, f12 - kb.pose.p);
        const double keyM = kb.width / kb.pw * ((1600 - 16) / 22.3);   // one key (vr_menu.cpp kKbUnit) in metres
        CHECK(lx.y > lb.y && std::fabs(lx.y - lf.y) < 1e-6 && lx.x > lb.x && lx.x - lb.x < keyM && lx.x > lf.x + keyM,
              "compact: x in the top row, above the right half of Backspace, right of F12");
        CHECK(m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("f5"), &at), "compact: F-keys there");
        WriteBmp(dir + L"\\keyboard_compact.bmp", kb);
        CHECK(click(1, VrMenu::kPanMenu, VrMenu::eSet, 82), "Full shown");
        for (int i = 0; i < 2; ++i) { VrPad::Frame f = idle; upd(f); }
        CHECK(kb.pw == pw0 && std::fabs(kb.width - wk) < 1e-9, "full again: the old size");
    }
    // close: the menu's x; the keyboard STAYS (it is its own panel)
    click(1, VrMenu::kPanMenu, VrMenu::eClose, 0);
    CHECK(!m.Pan(VrMenu::kPanMenu).show && m.Pan(VrMenu::kPanKb).show, "menu closed, the keyboard stays");
    for (int i = 0; i < 3; ++i) { VrPad::Frame f = idle; upd(f); }
    {
        V3 at;
        CHECK(m.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("f5"), &at), "F5 still there");
        VrPad::Frame f = Aim(1, at, 0.0f);
        upd(f);
        f = Aim(1, at, 1.0f);
        upd(f);
        f = Aim(1, at, 0.0f);
        upd(f);
        CHECK(sink.Take() == "k03fv k03f^", "the keyboard works with the menu closed");
    }

    printf("(3) config.ini\n");
    const std::string text = ReadFile(cfgPath);
    CHECK(text.find("# test\r\n") == 0, "comments kept, CRLF kept");
    CHECK(text.find("a = shift+f\r\n") != std::string::npos, "a = shift+f written in place");
    CHECK(text.find("# was") == std::string::npos, "the old a line replaced");
    CHECK(text.find("air_mode = toggle") != std::string::npos, "air_mode added");
    CHECK(text.find("menu_place = ") != std::string::npos && text.find("keyboard_place = ") != std::string::npos, "places added");
    const Config back = ParseConfig(text);
    for (const std::string& n : back.notes) printf("   note: %s\n", n.c_str());
    CHECK(back.notes.empty(), "it reads back without notes");
    CHECK(back.pad.airToggle && back.pad.mouseSpeed > 1000.0f, "read back: toggle, speed");
    CHECK(back.menuPlace.set && std::fabs(back.menuPlace.x - m.Pan(VrMenu::kPanMenu).pose.p.x) < 0.002, "read back: the menu place");
    CHECK(std::fabs(back.kbPlace.width - m.Pan(VrMenu::kPanKb).width) < 0.002, "read back: the keyboard width");
    CHECK(back.look.kbShow && back.look.theme == 1 && std::fabs(back.look.menuOpacity - 0.7f) < 0.01f, "read back: keyboard shown, blue, 70 %%");
    VrMenu m2(back, std::wstring());
    CHECK(m2.Pan(VrMenu::kPanKb).show, "a new game: the keyboard is there at once (keyboard_show)");
    CHECK(std::fabs(m2.Pan(VrMenu::kPanKb).pose.p.x - m.Pan(VrMenu::kPanKb).pose.p.x) < 0.002, "... at its saved place");
    m2.Open(true, head);
    CHECK(std::fabs(m2.Pan(VrMenu::kPanMenu).pose.p.x - m.Pan(VrMenu::kPanMenu).pose.p.x) < 0.002, "a new menu opens at the saved place");
    // cost of opening the menu (a new VrMenu = a new game; then closed and opened again): printed, not checked
    {
        LARGE_INTEGER q0, q1, q2, q3, fq;
        QueryPerformanceFrequency(&fq);
        QueryPerformanceCounter(&q0);
        VrMenu mo(back, std::wstring());
        QueryPerformanceCounter(&q1);
        mo.Open(true, head);
        QueryPerformanceCounter(&q2);
        mo.Close();
        mo.Open(true, head);
        QueryPerformanceCounter(&q3);
        printf("   new menu (game start) %.2f ms, first open %.2f ms, open again %.2f ms\n", (q1.QuadPart - q0.QuadPart) * 1000.0 / fq.QuadPart,
               (q2.QuadPart - q1.QuadPart) * 1000.0 / fq.QuadPart, (q3.QuadPart - q2.QuadPart) * 1000.0 / fq.QuadPart);
    }
    // cost of one keyboard redraw (it happens when the pointer moves onto another key): printed, not checked
    {
        Config c3 = back;
        c3.look.kbShow = false;                         // (its menu button shows it below)
        VrMenu m3(c3, std::wstring());
        Rec s3;
        auto u3 = [&](VrPad::Frame f) {                  // (+ the changed area taken every frame, as the layer's xrEndFrame does)
            m3.Update(f, true, true, head, now, s3);
            now += 0.011;
            int dd[4];
            m3.TakeDirty(VrMenu::kPanMenu, dd);
            m3.TakeDirty(VrMenu::kPanKb, dd);
        };
        m3.Open(true, head);
        V3 a1, a2;
        m3.ElemCentre(VrMenu::kPanMenu, VrMenu::eKbBtn, 0, &a1);
        u3(Aim(1, a1, 0.0f));
        u3(Aim(1, a1, 1.0f));
        u3(Aim(1, a1, 0.0f));
        const bool ok = m3.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("g"), &a1) &&
                        m3.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("h"), &a2);
        CHECK(ok, "keyboard shown by its menu button");
        LARGE_INTEGER q0, q1, fq;
        QueryPerformanceFrequency(&fq);
        const uint64_t v0 = m3.Pan(VrMenu::kPanKb).version;
        QueryPerformanceCounter(&q0);
        for (int i = 0; ok && i < 20; ++i) u3(Aim(1, (i & 1) ? a1 : a2, 0.0f));
        QueryPerformanceCounter(&q1);
        const uint64_t n = m3.Pan(VrMenu::kPanKb).version - v0;
        CHECK(n >= 20, "a hover change redraws (%llu)", (unsigned long long)n);
        printf("   one hover change (only the changed keys drawn): %.2f ms\n", (q1.QuadPart - q0.QuadPart) * 1000.0 / fq.QuadPart / 20);
    }
    // the README screenshots (default look, solid): <work folder>\shots\*.bmp -- every tab, the keyboard full / compact, and the
    // keyboard with the pointer on G (shot_pointer.txt = G's centre in pixels, for the dot)
    {
        const std::wstring shots = dir + L"\\shots";
        CreateDirectoryW(shots.c_str(), nullptr);
        Config cs = ParseConfig("menu_opacity = 1\nkeyboard_opacity = 1\n");   // (pointer smoothing medium: the default)
        VrMenu ms(cs, std::wstring());
        Rec s5;
        auto u5 = [&](VrPad::Frame f) { ms.Update(f, true, true, head, now, s5); now += 0.011; };
        auto press = [&](int pan, int kind, int arg) {
            V3 at;
            if (!ms.ElemCentre(pan, kind, arg, &at)) return;
            for (int i = 0; i < 60; ++i) u5(Aim(1, at, 0.0f));   // (the smoothed pointer settles first)
            u5(Aim(1, at, 1.0f)); u5(Aim(1, at, 0.0f));
        };
        auto settle = [&](const V3& at) { for (int i = 0; i < 60; ++i) u5(Aim(1, at, 0.0f)); };
        const V3 away{ 0, 3, 0.5 };                     // (pointing away: nothing hovered in the pictures)
        ms.Open(true, head);
        u5(Aim(1, away, 0.0f));
        static const wchar_t* const kTab[4] = { L"menu-buttons", L"menu-mouse", L"menu-panels", L"menu-look" };
        for (int t = 0; t < 4; ++t) {
            press(VrMenu::kPanMenu, VrMenu::eTab, t);
            settle(away);
            WriteBmp(shots + L"\\" + kTab[t] + L".bmp", ms.Pan(VrMenu::kPanMenu));
        }
        press(VrMenu::kPanMenu, VrMenu::eTab, 0);
        press(VrMenu::kPanMenu, VrMenu::eRow, VrPad::kA);   // picking a key for A: the keyboard opens with its frame
        settle(away);
        WriteBmp(shots + L"\\menu-pick.bmp", ms.Pan(VrMenu::kPanMenu));
        WriteBmp(shots + L"\\keyboard-pick.bmp", ms.Pan(VrMenu::kPanKb));
        press(VrMenu::kPanMenu, VrMenu::ePickCancel, 0);
        settle(away);
        WriteBmp(shots + L"\\keyboard.bmp", ms.Pan(VrMenu::kPanKb));
        V3 g;
        if (ms.ElemCentre(VrMenu::kPanKb, VrMenu::eKey, VrMenu::KeyIndex("g"), &g)) {
            settle(g);
            WriteBmp(shots + L"\\keyboard-pointer.bmp", ms.Pan(VrMenu::kPanKb));
            const VrMenu::Panel& kb = ms.Pan(VrMenu::kPanKb);
            const V3 l = QRot(QConj(kb.pose.q), g - kb.pose.p);
            FILE* f = nullptr;
            if (!_wfopen_s(&f, (shots + L"\\shot_pointer.txt").c_str(), L"wb") && f) {
                fprintf(f, "%d %d\n", (int)((l.x / kb.width + 0.5) * kb.pw), (int)((0.5 - l.y / kb.Height()) * kb.ph));
                fclose(f);
            }
        }
        press(VrMenu::kPanMenu, VrMenu::eTab, 2);
        press(VrMenu::kPanMenu, VrMenu::eSet, 83);       // compact
        settle(away);
        WriteBmp(shots + L"\\keyboard-compact.bmp", ms.Pan(VrMenu::kPanKb));
    }
    printf("%d checks, %d failed\n", g_checks, g_fails);
    return g_fails ? 1 : 0;
}
