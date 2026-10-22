// log -- one plain text file, <dll folder>\logs\ets2_ats_vr_controls.log (else %LOCALAPPDATA%\ETS2 ATS VR Controls\logs\...). New file
// per game start (the old one is kept as .old.log). Thread safe. Lines: "[hh:mm:ss.mmm] [exe] I message".
#pragma once
#include <string>

namespace ck {
void LogInit(const std::wstring& dllDir, const std::wstring& exeName);
void LogWrite(char level, const char* fmt, ...);
std::wstring LogPath();
}  // namespace ck

#define LOGI(...) ::ck::LogWrite('I', __VA_ARGS__)
#define LOGW(...) ::ck::LogWrite('W', __VA_ARGS__)
#define LOGE(...) ::ck::LogWrite('E', __VA_ARGS__)
