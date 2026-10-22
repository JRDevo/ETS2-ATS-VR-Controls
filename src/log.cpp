// log -- see log.h.
#include "log.h"
#include <windows.h>
#include <shlobj.h>
#include <cstdarg>
#include <cstdio>
#include <mutex>

namespace ck {
namespace {
std::mutex   g_m;
FILE*        g_f = nullptr;
std::wstring g_path;
std::string  g_exe;

bool OpenIn(const std::wstring& dir) {
    CreateDirectoryW(dir.c_str(), nullptr);
    const std::wstring p = dir + L"\\controller_keys.log";
    const std::wstring old = dir + L"\\controller_keys.old.log";
    MoveFileExW(p.c_str(), old.c_str(), MOVEFILE_REPLACE_EXISTING);
    FILE* f = nullptr;
    if (_wfopen_s(&f, p.c_str(), L"wb") != 0 || !f) return false;
    g_f = f;
    g_path = p;
    return true;
}
}  // namespace

void LogInit(const std::wstring& dllDir, const std::wstring& exeName) {
    std::lock_guard<std::mutex> lk(g_m);
    if (g_f) return;
    g_exe.clear();
    for (wchar_t c : exeName) g_exe += (c < 128) ? (char)c : '?';
    wchar_t env[MAX_PATH];
    if (GetEnvironmentVariableW(L"CK_LOG_DIR", env, MAX_PATH) > 0 && OpenIn(env)) return;   // tests
    if (OpenIn(dllDir + L"\\logs")) return;
    PWSTR la = nullptr;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &la))) {
        const std::wstring d = std::wstring(la) + L"\\VR Controller Keys";
        CoTaskMemFree(la);
        CreateDirectoryW(d.c_str(), nullptr);
        OpenIn(d + L"\\logs");
    }
}

void LogWrite(char level, const char* fmt, ...) {
    std::lock_guard<std::mutex> lk(g_m);
    if (!g_f) return;
    SYSTEMTIME t;
    GetLocalTime(&t);
    fprintf(g_f, "[%02d:%02d:%02d.%03d] [%s] %c ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, g_exe.c_str(), level);
    va_list a;
    va_start(a, fmt);
    vfprintf(g_f, fmt, a);
    va_end(a);
    fputs("\r\n", g_f);
    fflush(g_f);
}

std::wstring LogPath() {
    std::lock_guard<std::mutex> lk(g_m);
    return g_path;
}
}  // namespace ck
