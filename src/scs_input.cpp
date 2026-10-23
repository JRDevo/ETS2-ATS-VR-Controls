// scs_input -- see scs_input.h. The SCS SDK 1.14 input API, declared here with only what is used (layouts as in
// scssdk_input.h / scssdk_input_device.h / scssdk_input_event.h, 64-bit: the game is x64 only).
#include <windows.h>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "config.h"
#include "log.h"
#include "scs_input.h"

namespace ck {
namespace {

typedef int32_t scs_result_t;
constexpr scs_result_t kOk = 0, kUnsupported = -1, kNotFound = -4, kGenericError = -7;
constexpr uint32_t kInputVersion100 = (1u << 16) | 0u;     // SCS_INPUT_VERSION_1_00
constexpr uint32_t kTypeSemantical = 2;                     // SCS_INPUT_DEVICE_TYPE_semantical
constexpr uint32_t kValueFloat = 5;                         // SCS_VALUE_TYPE_float
constexpr uint32_t kFirstInFrame = 1, kFirstAfterActivation = 2;
constexpr int32_t  kLogError = 2;                           // SCS_LOG_TYPE_error

struct ScsEvent {                                           // scs_input_event_t (28 bytes)
    uint32_t input_index;
    union { uint8_t value_bool; float value_float; float sizing[6]; };
};
typedef scs_result_t (__stdcall* ScsEventCb)(ScsEvent* ev, uint32_t flags, void* context);
typedef void (__stdcall* ScsActiveCb)(uint8_t active, void* context);
struct ScsDeviceInput { const char* name; const char* display_name; uint32_t value_type; uint32_t padding; };
struct ScsDevice {
    const char* name; const char* display_name; uint32_t type; uint32_t input_count; const ScsDeviceInput* inputs;
    void* callback_context; ScsActiveCb input_active_callback; ScsEventCb input_event_callback;
};
typedef scs_result_t (__stdcall* ScsRegister)(const ScsDevice* device);
typedef void (__stdcall* ScsLog)(int32_t type, const char* message);
struct ScsInitV100 {                                        // scs_input_init_params_v100_t (40 bytes)
    const char* game_name; const char* game_id; uint32_t game_version; uint32_t padding; ScsLog log;
    ScsRegister register_device;
};
struct ScsValue { uint32_t type; uint32_t padding; union { float value_float; double sizing[7]; }; };   // scs_value_t
typedef void (__stdcall* ScsChannelCb)(const char* name, uint32_t index, const ScsValue* value, void* context);
typedef scs_result_t (__stdcall* ScsRegisterChannel)(const char* name, uint32_t index, uint32_t type, uint32_t flags,
                                                     ScsChannelCb callback, void* context);
struct ScsTelemetryInitV100 {                               // scs_telemetry_init_params_v100_t (64 bytes)
    const char* game_name; const char* game_id; uint32_t game_version; uint32_t padding; ScsLog log;
    void* register_for_event; void* unregister_from_event; ScsRegisterChannel register_for_channel; void* unregister_from_channel;
};
static_assert(sizeof(ScsTelemetryInitV100) == 64, "SCS telemetry layout (x64)");
static_assert(sizeof(ScsEvent) == 28 && sizeof(ScsDeviceInput) == 24 && sizeof(ScsDevice) == 56 && sizeof(ScsInitV100) == 40,
              "SCS SDK layouts (x64)");

std::atomic<float> g_val[3];                                // steer, gas, brake
const ScsDeviceInput kInputs[3] = {
    { "steering", "Steering", kValueFloat, 0 },
    { "aforward", "Throttle", kValueFloat, 0 },
    { "abackward", "Brake", kValueFloat, 0 },
};
// main thread only (the game's)
float g_now[3] = {};
int   g_next = 3;
int   g_activeLogs = 0;
// telemetry (log = 1: what the game made of our steering; the game's main thread too)
std::atomic<float> g_tm[3];                                 // truck.input.steering, truck.effective.steering, truck.speed
bool   g_tmOn = false, g_logDrive = false;
double g_lastDriveLog = 0;
float  g_lastLogged[3] = { NAN, NAN, NAN };
int    g_driveLogs = 0;

double NowSec() {
    static LARGE_INTEGER f = [] { LARGE_INTEGER x; QueryPerformanceFrequency(&x); return x; }();
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart / (double)f.QuadPart;
}

// log = 1: ten lines a second at most while anything moves, the first 3000 (about 5 minutes of steering): what we sent, what
// the game took as input and what its simulation used (truck.input / effective.steering are counterclockwise: + = left)
void LogDrive() {
    if (!g_logDrive || g_driveLogs >= 3000) return;
    const double now = NowSec();
    if (now - g_lastDriveLog < 0.1) return;
    const float in = g_tm[0].load(std::memory_order_relaxed), eff = g_tm[1].load(std::memory_order_relaxed);
    if (g_now[0] == g_lastLogged[0] && in == g_lastLogged[1] && eff == g_lastLogged[2]) return;
    g_lastDriveLog = now;
    g_lastLogged[0] = g_now[0]; g_lastLogged[1] = in; g_lastLogged[2] = eff;
    ++g_driveLogs;
    if (g_tmOn)
        LOGI("drive: sent steer %+.3f gas %.2f brake %.2f | game input steer %+.3f, used %+.3f | %.0f km/h%s", g_now[0], g_now[1],
             g_now[2], -in, -eff, g_tm[2].load(std::memory_order_relaxed) * 3.6f, g_driveLogs == 3000 ? " (no more of these lines)" : "");
    else
        LOGI("drive: sent steer %+.3f gas %.2f brake %.2f%s", g_now[0], g_now[1], g_now[2], g_driveLogs == 3000 ? " (no more of these lines)" : "");
}

// All three values every frame: a semantical input the game is not told in a frame may count as released (steering that
// snapped back the moment the thumb held still).
scs_result_t __stdcall OnEvent(ScsEvent* ev, uint32_t flags, void*) {
    if (flags & kFirstInFrame) {
        for (int i = 0; i < 3; ++i) g_now[i] = g_val[i].load(std::memory_order_relaxed);
        g_next = 0;
        LogDrive();
    }
    if (g_next >= 3) return kNotFound;
    ev->input_index = (uint32_t)g_next;
    ev->value_float = g_now[g_next];
    ++g_next;
    return kOk;
}
void __stdcall OnActive(uint8_t active, void*) {
    if (g_activeLogs < 10) LOGI("driving: the game %s our input device%s", active ? "uses" : "stopped using",
                                ++g_activeLogs == 10 ? " (no more of these lines)" : "");
}

}  // namespace

void __stdcall OnChannel(const char*, uint32_t, const ScsValue* v, void* context) {
    if (v && v->type == kValueFloat) g_tm[(int)(intptr_t)context].store(v->value_float, std::memory_order_relaxed);
}

void DriveSet(float steer, float throttle, float brake) {
    g_val[0].store(steer, std::memory_order_relaxed);
    g_val[1].store(throttle, std::memory_order_relaxed);
    g_val[2].store(brake, std::memory_order_relaxed);
}
void DriveGet(float out[3]) { for (int i = 0; i < 3; ++i) out[i] = g_val[i].load(std::memory_order_relaxed); }

}  // namespace ck

extern "C" __declspec(dllexport) ck::scs_result_t __stdcall scs_input_init(const uint32_t version, const void* params) {
    using namespace ck;
    if (version != kInputVersion100 || !params) return kUnsupported;   // (the game then tries an older version: none)
    const Config* c = GameConfig();
    if (!c) return kUnsupported;
    HMODULE self = nullptr;                                 // never unloaded under the OpenXR side (or its input thread)
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                       reinterpret_cast<LPCWSTR>(&DriveSet), &self);
    const ScsInitV100* p = static_cast<const ScsInitV100*>(params);
    ScsDevice d = {};
    d.name = "ets2_ats_vr_controls";
    d.display_name = "ETS2 ATS VR Controls";
    d.type = kTypeSemantical;
    d.input_count = 3;
    d.inputs = kInputs;
    d.input_active_callback = OnActive;
    d.input_event_callback = OnEvent;
    const scs_result_t r = p->register_device ? p->register_device(&d) : kGenericError;
    if (r != kOk) {
        if (p->log) p->log(kLogError, "ETS2 ATS VR Controls: the driving device was not registered");
        LOGE("driving: the game refused our input device (%d) -- no steering / throttle from the controller", (int)r);
        return kGenericError;
    }
    LOGI("driving: input device registered in %s (input API 1.0, game input version %u.%u)", p->game_name ? p->game_name : "?",
         p->game_version >> 16, p->game_version & 0xFFFF);
    g_logDrive = c->log;
    return kOk;
}

// The telemetry API: only with log = 1, to write what the game made of our steering (LogDrive). Nothing else uses it yet.
extern "C" __declspec(dllexport) ck::scs_result_t __stdcall scs_telemetry_init(const uint32_t version, const void* params) {
    using namespace ck;
    if ((version >> 16) != 1 || !params) return kUnsupported;
    const Config* c = GameConfig();
    if (!c || !c->log) return kUnsupported;
    const ScsTelemetryInitV100* p = static_cast<const ScsTelemetryInitV100*>(params);
    if (!p->register_for_channel) return kGenericError;
    static const char* const kCh[3] = { "truck.input.steering", "truck.effective.steering", "truck.speed" };
    int ok = 0;
    for (int i = 0; i < 3; ++i) ok += p->register_for_channel(kCh[i], 0xFFFFFFFFu, kValueFloat, 0, OnChannel, (void*)(intptr_t)i) == kOk;
    g_tmOn = ok == 3;
    LOGI("driving: telemetry for the log (%d of 3 channels)", ok);
    return kOk;
}

extern "C" __declspec(dllexport) void __stdcall scs_telemetry_shutdown(void) { ck::g_tmOn = false; }

extern "C" __declspec(dllexport) void __stdcall scs_input_shutdown(void) {}
