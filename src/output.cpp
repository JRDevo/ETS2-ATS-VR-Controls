// output -- see output.h.
#include "output.h"
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>
#include "log.h"

namespace ck {
namespace {
std::mutex g_m;   // the test file + the order of events across threads

// The sender: one thread that hands the queued events to Windows (SendInput can block for milliseconds -- the user's game log
// showed 4-12 ms frames that were all SendInput). On the heap, never freed; the thread is detached (never joined at unload).
struct Sender {
    std::mutex m;
    std::condition_variable cv, idle;
    std::vector<INPUT> q;
    bool busy = false;
    std::atomic<long long> maxTicks{0};     // the longest SendInput call (measure on), for the log
};
Sender& Snd() {
    static Sender* s = [] {
        Sender* n = new Sender();
        std::thread([n] {
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);   // input should not wait behind others
            std::vector<INPUT> batch;
            for (;;) {
                {
                    std::unique_lock<std::mutex> lk(n->m);
                    n->cv.wait(lk, [n] { return !n->q.empty(); });
                    batch.swap(n->q);
                    n->busy = true;
                }
                LARGE_INTEGER a{}, b{};
                QueryPerformanceCounter(&a);
                SendInput((UINT)batch.size(), batch.data(), sizeof(INPUT));    // in order, in one call
                QueryPerformanceCounter(&b);
                const long long t = b.QuadPart - a.QuadPart;
                long long m = n->maxTicks.load();
                while (t > m && !n->maxTicks.compare_exchange_weak(m, t)) {}
                batch.clear();
                {
                    std::lock_guard<std::mutex> lk(n->m);
                    n->busy = false;
                }
                n->idle.notify_all();
            }
        }).detach();
        return n;
    }();
    return *s;
}
double TicksToS(long long t) {
    static const double f = [] { LARGE_INTEGER x; QueryPerformanceFrequency(&x); return (double)x.QuadPart; }();
    return (double)t / f;
}
}  // namespace

InputSink::InputSink() {
    wchar_t p[MAX_PATH];
    if (GetEnvironmentVariableW(L"VRC_TEST_EVENTS", p, MAX_PATH) > 0) {
        FILE* f = nullptr;
        if (_wfopen_s(&f, p, L"ab") == 0 && f) test_ = f;
    }
}

// the game's thread: the event joins the sender's queue (a relative move right after another move is added to it)
void InputSink::Queue(const INPUT& in) {
    LARGE_INTEGER a{}, b{};
    if (measure_) QueryPerformanceCounter(&a);
    Sender& s = Snd();
    {
        std::lock_guard<std::mutex> lk(s.m);
        INPUT* last = s.q.empty() ? nullptr : &s.q.back();
        if (last && in.type == INPUT_MOUSE && in.mi.dwFlags == MOUSEEVENTF_MOVE && last->type == INPUT_MOUSE &&
            last->mi.dwFlags == MOUSEEVENTF_MOVE) {
            last->mi.dx += in.mi.dx;
            last->mi.dy += in.mi.dy;
        } else {
            s.q.push_back(in);
        }
    }
    s.cv.notify_one();
    if (measure_) { QueryPerformanceCounter(&b); sendTicks_ += b.QuadPart - a.QuadPart; }
}

void InputSink::Flush() {
    if (test_) return;
    Sender& s = Snd();
    std::unique_lock<std::mutex> lk(s.m);
    s.idle.wait_for(lk, std::chrono::milliseconds(200), [&s] { return s.q.empty() && !s.busy; });
}

double InputSink::TakeSendTime() { return TicksToS(sendTicks_.exchange(0)); }
double InputSink::TakeSenderMax() { return TicksToS(Snd().maxTicks.exchange(0)); }

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
    Queue(in);
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
    Queue(in);
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
    Queue(in);
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
    Queue(in);
}

bool GameInFront() {
    if (Sink().TestMode()) return true;
    // asked at most every 0.1 s (it runs every frame; two window calls each time were wasted)
    static std::atomic<long long> last{ 0 };
    static std::atomic<bool> front{ false };
    LARGE_INTEGER now, fq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&fq);
    if (now.QuadPart - last.load() < fq.QuadPart / 10) return front.load();
    HWND w = GetForegroundWindow();
    DWORD pid = 0;
    if (w) GetWindowThreadProcessId(w, &pid);
    front = w && pid == GetCurrentProcessId();
    last = now.QuadPart;
    return front.load();
}

InputSink& Sink() {
    static InputSink* s = new InputSink();   // never destroyed: the game may call into the layer during process exit
    return *s;
}

}  // namespace ck
