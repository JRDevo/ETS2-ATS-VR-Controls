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
static_assert(sizeof(ScsEvent) == 28 && sizeof(ScsDeviceInput) == 24 && sizeof(ScsDevice) == 56 && sizeof(ScsInitV100) == 40,
              "SCS SDK layouts (x64)");

std::atomic<float> g_val[3];                                // steer, gas, brake
const ScsDeviceInput kInputs[3] = {
    { "steering", "Steering", kValueFloat, 0 },
    { "aforward", "Throttle", kValueFloat, 0 },
    { "abackward", "Brake", kValueFloat, 0 },
};
// main thread only (the game's)
float g_now[3] = {}, g_told[3] = { NAN, NAN, NAN };
int   g_next = 3;
int   g_activeLogs = 0;

scs_result_t __stdcall OnEvent(ScsEvent* ev, uint32_t flags, void*) {
    if (flags & kFirstAfterActivation) for (float& t : g_told) t = NAN;   // (NaN differs from everything: all told again)
    if (flags & kFirstInFrame) {
        for (int i = 0; i < 3; ++i) g_now[i] = g_val[i].load(std::memory_order_relaxed);
        g_next = 0;
    }
    for (; g_next < 3; ++g_next) {
        if (g_now[g_next] == g_told[g_next]) continue;
        ev->input_index = (uint32_t)g_next;
        ev->value_float = g_now[g_next];
        g_told[g_next] = g_now[g_next];
        ++g_next;
        return kOk;
    }
    return kNotFound;
}
void __stdcall OnActive(uint8_t active, void*) {
    if (g_activeLogs < 10) LOGI("driving: the game %s our input device%s", active ? "uses" : "stopped using",
                                ++g_activeLogs == 10 ? " (no more of these lines)" : "");
}

}  // namespace

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
    return kOk;
}

extern "C" __declspec(dllexport) void __stdcall scs_input_shutdown(void) {}
