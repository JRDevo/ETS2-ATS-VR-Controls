// log -- see log.h. A line is formatted on the caller's thread and only appended to a buffer in memory; a background thread
// (below-normal priority, detached: never joined at unload) writes the buffer to the file 4 times a second, an error line at
// once -- no disk write inside a game frame (Fable audit: a per-line fflush cost milliseconds in xrBeginFrame).
#include "log.h"
#include <windows.h>
#include <shlobj.h>
#include <condition_variable>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <string>
#include <thread>

namespace ck {
namespace {
struct LogState {                 // on the heap, never freed: the writer thread may outlive the DLL's static destructors
    std::mutex   m;
    std::condition_variable cv;
    FILE*        f = nullptr;
    std::wstring path;
    std::string  exe;
    std::string  pending;         // lines not written yet
    bool         now = false;     // an error line: write at once
    std::mutex   wm;              // the file writes (the writer thread / LogFlush)
};
LogState& St() { static LogState* s = new LogState(); return *s; }

bool OpenIn(const std::wstring& dir) {
    CreateDirectoryW(dir.c_str(), nullptr);
    const std::wstring p = dir + L"\\ets2_ats_vr_controls.log";
    const std::wstring old = dir + L"\\ets2_ats_vr_controls.old.log";
    MoveFileExW(p.c_str(), old.c_str(), MOVEFILE_REPLACE_EXISTING);
    FILE* f = nullptr;
    if (_wfopen_s(&f, p.c_str(), L"wb") != 0 || !f) return false;
    St().f = f;
    St().path = p;
    return true;
}

void Writer() {
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    LogState& s = St();
    std::string out;
    for (;;) {
        {
            std::unique_lock<std::mutex> lk(s.m);
            s.cv.wait_for(lk, std::chrono::milliseconds(250), [&] { return s.now; });
            s.now = false;
        }
        std::lock_guard<std::mutex> wl(s.wm);          // (taken before the lines: LogFlush cannot write later lines first)
        { std::lock_guard<std::mutex> lk(s.m); out.swap(s.pending); }
        if (!out.empty() && s.f) {
            fwrite(out.data(), 1, out.size(), s.f);
            fflush(s.f);
            out.clear();
        }
    }
}
}  // namespace

void LogInit(const std::wstring& dllDir, const std::wstring& exeName) {
    LogState& s = St();
    std::lock_guard<std::mutex> lk(s.m);
    if (s.f) return;
    s.exe.clear();
    for (wchar_t c : exeName) s.exe += (c < 128) ? (char)c : '?';
    wchar_t env[MAX_PATH];
    bool ok = GetEnvironmentVariableW(L"VRC_LOG_DIR", env, MAX_PATH) > 0 && OpenIn(env);   // tests
    if (!ok) ok = OpenIn(dllDir + L"\\logs");
    if (!ok) {
        PWSTR la = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &la))) {
            const std::wstring d = std::wstring(la) + L"\\ETS2 ATS VR Controls";
            CoTaskMemFree(la);
            CreateDirectoryW(d.c_str(), nullptr);
            ok = OpenIn(d + L"\\logs");
        }
    }
    if (ok) std::thread(Writer).detach();
}

void LogWrite(char level, const char* fmt, ...) {
    LogState& s = St();
    if (!s.f) return;                                  // (no log: not our game, or a unit test)
    SYSTEMTIME t;
    GetLocalTime(&t);
    char head[96];
    const int hn = snprintf(head, sizeof(head), "[%02d:%02d:%02d.%03d] [%s] %c ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds,
                            s.exe.c_str(), level);
    char body[1024];
    va_list a;
    va_start(a, fmt);
    int bn = vsnprintf(body, sizeof(body), fmt, a);
    va_end(a);
    if (bn < 0) bn = 0;
    if (bn >= (int)sizeof(body)) bn = (int)sizeof(body) - 1;
    {
        std::lock_guard<std::mutex> lk(s.m);
        s.pending.append(head, hn > 0 ? (size_t)hn : 0);
        s.pending.append(body, (size_t)bn);
        s.pending.append("\r\n");
        if (level == 'E') s.now = true;
    }
    if (level == 'E') s.cv.notify_one();
}

void LogFlush() {
    LogState& s = St();
    if (!s.f) return;
    std::lock_guard<std::mutex> wl(s.wm);
    std::string out;
    { std::lock_guard<std::mutex> lk(s.m); out.swap(s.pending); }
    if (!out.empty()) { fwrite(out.data(), 1, out.size(), s.f); fflush(s.f); }
}

std::wstring LogPath() {
    std::lock_guard<std::mutex> lk(St().m);
    return St().path;
}
}  // namespace ck
