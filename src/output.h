// output -- the Mapper's events as real Windows input (SendInput): keys as SCAN codes (DirectInput games such as ETS2 / ATS read
// the physical key, whatever the keyboard layout), mouse buttons, wheel and relative moves. Only while the GAME's window is in
// front (GameInFront): the layer never types into another program. Key / button RELEASES are always sent, so nothing stays
// held when the game loses focus.
// The game's thread only queues an event (microseconds); one sender thread hands them to Windows in order (SendInput can
// block for milliseconds -- the user's log showed every 4-12 ms frame was SendInput). Flush (the session / instance ends)
// waits until all are sent, so no key stays held when the game closes.
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
    void Drive(float steer, float throttle, float brake) override;   // -> scs_input.h (no SendInput; test mode: a line too)
    bool TestMode() const { return test_ != nullptr; }
    void SetLogEvents(bool on) { logEvents_ = on; }
    void SetMeasure(bool on) { measure_ = on; }
    double TakeSendTime();            // seconds the game's thread spent queueing input since the last call (measure on)
    double TakeSenderMax();           // the longest SendInput of the sender thread since the last call (seconds)
    void   Flush();                   // wait (up to 0.2 s) until every queued event is sent
private:
    void Emit(const char* line);
    void Queue(const INPUT& in);
    void* test_ = nullptr;   // FILE*
    bool  logEvents_ = false;
    bool  measure_ = false;
    std::atomic<long long> sendTicks_{0};
};

bool GameInFront();   // the foreground window belongs to this process (always true in test mode)
InputSink& Sink();    // the one sink of the process

}  // namespace ck
