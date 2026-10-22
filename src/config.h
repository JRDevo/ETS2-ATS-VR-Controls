// config -- config.ini next to ets2_ats_vr_controls.dll (or the file named by the VRC_CONFIG environment variable, for tests).
// key = value lines, '#' or ';' starts a comment line. Read once per game start. Unknown keys and bad values are logged and
// ignored (the default stays). See config.ini in the repo for every key.
#pragma once
#include <string>
#include <vector>
#include "pad_map.h"

namespace ck {

struct Config {
    bool                      enabled = true;      // enabled = 0: the layer passes everything through
    std::vector<std::wstring> apps;                // lower-case exe names; "*" = every OpenXR program
    VrPad::Config             pad = VrPad::DefaultConfig();
    bool                      logInputs = false;   // log_inputs = 1: every key / mouse event in the log
    std::wstring              loadedFrom;          // empty = no file (defaults)
    std::vector<std::string>  notes;               // problems found while reading (logged once the log is open)
};

// Parse `text` (the whole file) on top of the defaults.
Config ParseConfig(const std::string& text);
Config LoadConfig(const std::wstring& dllDir);
bool   AppAllowed(const Config& c, const std::wstring& exeName);   // exeName = file name only, any case
std::string DescribePad(const VrPad::Config& p);                     // one line per mapped button, for the log

}  // namespace ck
