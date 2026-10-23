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
constexpr uint32_t kValueFPlacement = 10, kValueDPlacement = 11;
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
struct ScsFPlacement { float x, y, z, heading, pitch, roll; };                 // scs_value_fplacement_t
struct ScsDPlacement { double x, y, z; float heading, pitch, roll; uint32_t padding; };   // scs_value_dplacement_t
struct ScsValue {                                                                 // scs_value_t
    uint32_t type; uint32_t padding;
    union { float value_float; ScsFPlacement value_fplacement; ScsDPlacement value_dplacement; double sizing[7]; };
};
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
std::atomic<bool> g_tmOn{false};                            // (main thread writes, the VR thread reads)
// log = 1 only (data for pinning panels to the cab): the cabin's and the head's offsets (fplacement: x y z metres, heading
// pitch roll in turns), the truck's heading / pitch / roll, and our VR head pose (LOCAL) at the same moment
std::atomic<float> g_cab[6], g_headOff[6], g_truck[3], g_vrHead[7];   // vrHead: x y z, then the orientation's x y z w
bool   g_logDrive = false;
double g_lastDriveLog = 0;
float  g_lastLogged[3] = { NAN, NAN, NAN }, g_lastCab = NAN, g_lastStick = NAN;
std::atomic<float> g_stick, g_stickY;                        // the steering stick's x / y (DriveStickSet; log = 1)
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
    const float cabP = g_cab[4].load(std::memory_order_relaxed);
    const float stick = g_stick.load(std::memory_order_relaxed), stickY = g_stickY.load(std::memory_order_relaxed);
    if (g_now[0] == g_lastLogged[0] && in == g_lastLogged[1] && eff == g_lastLogged[2] && cabP == g_lastCab && stick == g_lastStick) return;
    g_lastCab = cabP;
    g_lastStick = stick;
    g_lastDriveLog = now;
    g_lastLogged[0] = g_now[0]; g_lastLogged[1] = in; g_lastLogged[2] = eff;
    ++g_driveLogs;
    if (g_tmOn)
        LOGI("drive: stick %+.2f (y %+.2f) sent steer %+.3f gas %.2f brake %.2f | game input steer %+.3f, used %+.3f | %.0f km/h%s",
             stick, stickY, g_now[0], g_now[1], g_now[2], -in, -eff, g_tm[2].load(std::memory_order_relaxed) * 3.6f,
             g_driveLogs == 3000 ? " (no more of these lines)" : "");
    else
        LOGI("drive: stick %+.2f (y %+.2f) sent steer %+.3f gas %.2f brake %.2f%s", stick, stickY, g_now[0], g_now[1], g_now[2],
             g_driveLogs == 3000 ? " (no more of these lines)" : "");
    if (g_tmOn) {                                       // the cab line (same moment)
        float c[6], h[6], t[3], v[7];
        for (int i = 0; i < 6; ++i) { c[i] = g_cab[i].load(std::memory_order_relaxed); h[i] = g_headOff[i].load(std::memory_order_relaxed); }
        for (int i = 0; i < 3; ++i) t[i] = g_truck[i].load(std::memory_order_relaxed);
        for (int i = 0; i < 7; ++i) v[i] = g_vrHead[i].load(std::memory_order_relaxed);
        LOGI("cab: offset %+.3f %+.3f %+.3f m, h/p/r %+.4f %+.4f %+.4f | head offset %+.3f %+.3f %+.3f, %+.4f %+.4f %+.4f | truck "
             "h/p/r %.4f %+.4f %+.4f | vr head %+.3f %+.3f %+.3f q %+.3f %+.3f %+.3f %+.3f", c[0], c[1], c[2], c[3], c[4], c[5], h[0],
             h[1], h[2], h[3], h[4], h[5], t[0], t[1], t[2], v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
    }
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
void __stdcall OnPlacement(const char*, uint32_t, const ScsValue* v, void* context) {   // context 0 cabin, 1 head, 2 truck
    if (!v) return;
    const int w = (int)(intptr_t)context;
    if (w < 2 && v->type == kValueFPlacement) {
        const ScsFPlacement& p = v->value_fplacement;
        std::atomic<float>* d = w ? g_headOff : g_cab;
        const float a[6] = { p.x, p.y, p.z, p.heading, p.pitch, p.roll };
        for (int i = 0; i < 6; ++i) d[i].store(a[i], std::memory_order_relaxed);
    } else if (w == 2 && v->type == kValueDPlacement) {
        g_truck[0].store(v->value_dplacement.heading, std::memory_order_relaxed);
        g_truck[1].store(v->value_dplacement.pitch, std::memory_order_relaxed);
        g_truck[2].store(v->value_dplacement.roll, std::memory_order_relaxed);
    }
}

void DriveStickSet(float x, float y) {
    if (!g_logDrive) return;
    g_stick.store(x, std::memory_order_relaxed);
    g_stickY.store(y, std::memory_order_relaxed);
}
void DriveSet(float steer, float throttle, float brake) {
    g_val[0].store(steer, std::memory_order_relaxed);
    g_val[1].store(throttle, std::memory_order_relaxed);
    g_val[2].store(brake, std::memory_order_relaxed);
}
void VrHeadSet(const double p[3], const double q[4]) {
    if (!g_logDrive) return;                            // (log = 1 only: the cab line)
    for (int i = 0; i < 3; ++i) g_vrHead[i].store((float)p[i], std::memory_order_relaxed);
    for (int i = 0; i < 4; ++i) g_vrHead[3 + i].store((float)q[i], std::memory_order_relaxed);
}
float DriveSpeedKmh() {
    return g_tmOn ? std::fabs(g_tm[2].load(std::memory_order_relaxed)) * 3.6f : -1.0f;
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

// The telemetry API: the truck's speed (speed-sensitive steering: DriveSpeedKmh) and, with log = 1, what the game made of our
// steering (LogDrive).
extern "C" __declspec(dllexport) ck::scs_result_t __stdcall scs_telemetry_init(const uint32_t version, const void* params) {
    using namespace ck;
    if ((version >> 16) != 1 || !params) return kUnsupported;
    const Config* c = GameConfig();
    if (!c) return kUnsupported;
    const ScsTelemetryInitV100* p = static_cast<const ScsTelemetryInitV100*>(params);
    if (!p->register_for_channel) return kGenericError;
    static const char* const kCh[3] = { "truck.input.steering", "truck.effective.steering", "truck.speed" };
    int ok = 0;
    for (int i = 0; i < 3; ++i) ok += p->register_for_channel(kCh[i], 0xFFFFFFFFu, kValueFloat, 0, OnChannel, (void*)(intptr_t)i) == kOk;
    g_tm[2].store(0.0f, std::memory_order_relaxed);
    g_tmOn = ok == 3;
    if (c->log) {                                       // the cab data for the log (pinning panels to the cab, coming)
        int okc = 0;
        okc += p->register_for_channel("truck.cabin.offset", 0xFFFFFFFFu, kValueFPlacement, 0, OnPlacement, (void*)(intptr_t)0) == kOk;
        okc += p->register_for_channel("truck.head.offset", 0xFFFFFFFFu, kValueFPlacement, 0, OnPlacement, (void*)(intptr_t)1) == kOk;
        okc += p->register_for_channel("truck.world.placement", 0xFFFFFFFFu, kValueDPlacement, 0, OnPlacement, (void*)(intptr_t)2) == kOk;
        LOGI("driving: telemetry for the log: cab / head / truck placement %d of 3", okc);
    }
    if (ok == 3) LOGI("driving: telemetry (truck speed, steering) %d of 3 channels", ok);
    else LOGW("driving: telemetry: only %d of 3 channels -- no speed-sensitive steering", ok);
    return kOk;
}

extern "C" __declspec(dllexport) void __stdcall scs_telemetry_shutdown(void) {
    ck::g_tmOn = false;
    for (std::atomic<float>& v : ck::g_tm) v.store(0.0f, std::memory_order_relaxed);   // (no stale speed after a re-init)
}

extern "C" __declspec(dllexport) void __stdcall scs_input_shutdown(void) {}
