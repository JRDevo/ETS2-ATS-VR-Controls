// output -- the Mapper's events as real Windows input (SendInput): keys as SCAN codes (DirectInput games such as ETS2 / ATS read
// the physical key, whatever the keyboard layout), mouse buttons, wheel and relative moves. Only while the GAME's window is in
// front (GameInFront): the layer never types into another program. Key / button RELEASES are always sent, so nothing stays
// held when the game loses focus.
// Test mode (environment VRC_TEST_EVENTS = a file path): every event is written to that file as one line instead
// ("key 023 down", "mouse 0 up", "wheel +1", "move 12 -3"), and GameInFront is always true.
#pragma once
#include <windows.h>
#include <atomic>
#include <cstdint>
#include "pad_map.h"

namespace ck {

class InputSink : public VrPad::Sink {
public:
    InputSink();
    ~InputSink() override;
    void Key(uint16_t code, bool down) override;
    void MouseButton(int which, bool down) override;
    void Wheel(int notches) override;
    void Move(int dx, int dy) override;
    bool TestMode() const { return test_ != nullptr; }
    void SetLogEvents(bool on) { logEvents_ = on; }
    void SetMeasure(bool on) { measure_ = on; }
    double TakeSendTime();            // seconds spent in SendInput (or the test file) since the last call (measure on)
private:
    void Emit(const char* line);
    void Timed(const INPUT& in);
    void* test_ = nullptr;   // FILE*
    bool  logEvents_ = false;
    bool  measure_ = false;
    std::atomic<long long> sendTicks_{0};
};

bool GameInFront();   // the foreground window belongs to this process (always true in test mode)
InputSink& Sink();    // the one sink of the process

}  // namespace ck
