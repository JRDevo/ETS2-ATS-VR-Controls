// output -- see output.h.
#include "output.h"
#include <windows.h>
#include <cstdio>
#include <mutex>
#include "log.h"

namespace ck {
namespace {
std::mutex g_m;   // the test file + SendInput order across threads
}

InputSink::InputSink() {
    wchar_t p[MAX_PATH];
    if (GetEnvironmentVariableW(L"CK_TEST_EVENTS", p, MAX_PATH) > 0) {
        FILE* f = nullptr;
        if (_wfopen_s(&f, p, L"ab") == 0 && f) test_ = f;
    }
}

InputSink::~InputSink() {
    if (test_) fclose(static_cast<FILE*>(test_));
}

void InputSink::Emit(const char* line) {
    if (logEvents_) LOGI("input: %s", line);
    if (!test_) return;
    FILE* f = static_cast<FILE*>(test_);
    fprintf(f, "%s\n", line);
    fflush(f);
}

void InputSink::Key(uint16_t code, bool down) {
    std::lock_guard<std::mutex> lk(g_m);
    char l[48];
    snprintf(l, sizeof(l), "key %03x %s", code, down ? "down" : "up");
    Emit(l);
    if (test_) return;
    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wScan = (WORD)(code & 0xFF);
    in.ki.dwFlags = KEYEVENTF_SCANCODE | ((code & VrPad::kExt) ? KEYEVENTF_EXTENDEDKEY : 0) | (down ? 0 : KEYEVENTF_KEYUP);
    SendInput(1, &in, sizeof(INPUT));
}

void InputSink::MouseButton(int which, bool down) {
    std::lock_guard<std::mutex> lk(g_m);
    char l[48];
    snprintf(l, sizeof(l), "mouse %d %s", which, down ? "down" : "up");
    Emit(l);
    if (test_) return;
    static const DWORD kDown[3] = { MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_MIDDLEDOWN };
    static const DWORD kUp[3] = { MOUSEEVENTF_LEFTUP, MOUSEEVENTF_RIGHTUP, MOUSEEVENTF_MIDDLEUP };
    if (which < 0 || which > 2) return;
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = down ? kDown[which] : kUp[which];
    SendInput(1, &in, sizeof(INPUT));
}

void InputSink::Wheel(int notches) {
    std::lock_guard<std::mutex> lk(g_m);
    char l[48];
    snprintf(l, sizeof(l), "wheel %+d", notches);
    Emit(l);
    if (test_) return;
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = MOUSEEVENTF_WHEEL;
    in.mi.mouseData = (DWORD)(notches * WHEEL_DELTA);
    SendInput(1, &in, sizeof(INPUT));
}

void InputSink::Move(int dx, int dy) {
    std::lock_guard<std::mutex> lk(g_m);
    if (test_) {
        char l[48];
        snprintf(l, sizeof(l), "move %d %d", dx, dy);
        FILE* f = static_cast<FILE*>(test_);
        fprintf(f, "%s\n", l);
        fflush(f);
        return;                                    // (moves are never logged: one line per frame)
    }
    INPUT in{};
    in.type = INPUT_MOUSE;
    in.mi.dwFlags = MOUSEEVENTF_MOVE;
    in.mi.dx = dx;
    in.mi.dy = dy;
    SendInput(1, &in, sizeof(INPUT));
}

bool GameInFront() {
    if (Sink().TestMode()) return true;
    HWND w = GetForegroundWindow();
    if (!w) return false;
    DWORD pid = 0;
    GetWindowThreadProcessId(w, &pid);
    return pid == GetCurrentProcessId();
}

InputSink& Sink() {
    static InputSink* s = new InputSink();   // never destroyed: the game may call into the layer during process exit
    return *s;
}

}  // namespace ck
