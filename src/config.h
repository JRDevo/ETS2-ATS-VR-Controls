// config -- config.ini next to ets2_ats_vr_controls.dll (or the file named by the VRC_CONFIG environment variable, for tests).
// key = value lines, '#' or ';' starts a comment line. Read once per game start. Unknown keys and bad values are logged and
// ignored (the default stays). See config.ini in the repo for every key.
#pragma once
#include <string>
#include <utility>
#include <vector>
#include "pad_map.h"

namespace ck {

// where a VR panel floats (config.ini menu_place / keyboard_place = x, y, z, yaw, pitch, width): metres in the room's LOCAL
// space, degrees (yaw + = turned left, pitch + = tilted up), width in metres. set = false: in front of the head when opened.
struct PanelPlace { bool set = false; double x = 0, y = 0, z = 0, yaw = 0, pitch = 0, width = 0; };

// the VR menu's look (config.ini theme / menu_opacity / keyboard_opacity) and whether the keyboard stays shown (keyboard_show)
inline const char* const* ThemeNames() {
    static const char* const k[6] = { "dark", "blue", "grey", "light", "black", "green" };
    return k;
}
constexpr int kThemeCount = 6;
struct MenuLook { int theme = 0; float menuOpacity = 0.95f, kbOpacity = 0.95f; bool kbShow = false; };

struct Config {
    bool                      enabled = true;      // enabled = 0: the layer passes everything through
    std::vector<std::wstring> apps;                // lower-case exe names; "*" = every OpenXR program
    VrPad::Config             pad = VrPad::DefaultConfig();
    bool                      logInputs = false;   // log_inputs = 1: every key / mouse event in the log
    bool                      menu = true;         // vr_menu = 0: no VR menu / keyboard (and no combo)
    std::vector<int>          menuCombo = { VrPad::kLGrip, VrPad::kRGrip, VrPad::kLStick };   // menu_combo: all held = open / close
    float                     menuHold = 1.0f;     // menu_hold: seconds
    PanelPlace                menuPlace, kbPlace;  // menu_place / keyboard_place
    MenuLook                  look;                // theme, menu_opacity, keyboard_opacity, keyboard_show
    std::wstring              loadedFrom;          // empty = no file (defaults)
    std::vector<std::string>  notes;               // problems found while reading (logged once the log is open)
};

// Parse `text` (the whole file) on top of the defaults.
Config ParseConfig(const std::string& text);
Config LoadConfig(const std::wstring& dllDir);
bool   AppAllowed(const Config& c, const std::wstring& exeName);   // exeName = file name only, any case
std::string DescribePad(const VrPad::Config& p);                     // one line per mapped button, for the log
std::string PlaceString(const PanelPlace& p);                        // "x, y, z, yaw, pitch, width" (config.ini)

// Write `kv` (key, value) into the config.ini at `path`: each key's first line "key = ..." (not a comment) gets the new value,
// a key without such a line is added at the end. Comments and every other line are kept. False = not written (logged by caller).
bool SaveConfigValues(const std::wstring& path, const std::vector<std::pair<std::string, std::string>>& kv);
// The settings the VR menu owns, as config.ini lines (every button, the mouse, the panel places, the look).
std::vector<std::pair<std::string, std::string>> MenuValues(const VrPad::Config& p, const PanelPlace& menu, const PanelPlace& kb,
                                                            const MenuLook& look);

}  // namespace ck
