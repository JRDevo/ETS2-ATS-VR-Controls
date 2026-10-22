// pad_input -- the layer's own OpenXR action set next to the game's input, read every frame and turned into keys / mouse
// (pad_map.h Mapper -> output.h InputSink). One PadInput per XrInstance.
//
// How it stays out of the game's way (OpenXR 1.0 / 1.1 rules; the same approach as VR EyeCapture's controller hotkey):
//   * xrSuggestInteractionProfileBindings REPLACES the previous suggestion for the same profile, and the runtime picks ONE
//     interaction profile per hand from all suggested ones. So the layer forwards the game's suggestions untouched and remembers
//     them; only at the game's xrAttachSessionActionSets (the last moment suggestions are allowed) it re-suggests each profile
//     the game suggested as the game's bindings + ours. It never adds a profile the game did not suggest -- except for a game
//     that suggests ONLY khr/simple_controller (ETS2 / ATS): then the real controller profiles get the game's bindings
//     translated (select -> trigger, menu -> menu) + ours, so the runtime picks the real controller and the game keeps its
//     buttons. A merged suggestion the runtime refuses is followed by the game's own suggestion again (restored). A game that
//     suggests nothing at all gets every profile of ours.
//   * Our action set's priority = the highest priority of the game's attached sets: OpenXR hides an input from every set of
//     LOWER priority than the highest one using it, so equal priority = the game and we both see every button.
//   * Attach: our set is appended to the game's list. Sync: our set is appended to the game's active sets. Any error our
//     addition causes: the call is repeated exactly as the game made it and the controllers are off for that session (logged);
//     the game always gets the result of its own call.
//   * A game that never creates an action set gets ours attached by the layer after 20 s of frames, synced by the layer.
//   * Every frame (the game's xrBeginFrame): the states (from the game's last xrSyncActions, which carried our set), the aim poses
//     (in our own LOCAL space at the game's predicted display time), then the Mapper -> keys / mouse. The layer syncs on its own
//     ONLY for a game that never syncs (no input of its own, or no game sync in the first 2 s of FOCUSED): a sync with only our
//     set would make the game's own sets inactive until its next sync. A game sync older than 100 ms = the frame is skipped
//     (nothing changes, held keys stay held); older than 0.5 s = everything held is released. Input is sent only while the
//     session is FOCUSED and the game window is in front; otherwise (and when the session or instance ends) everything held is
//     released.
//   * On Index / Vive / WMR the game's translated menu lands on a button we also use (B / the right menu): ours is left out there,
//     so one press never does two things. Shared triggers and poses are kept (both see them).
// The layer calls the next layer's functions directly; our hooks never see our own calls.
#pragma once
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <openxr/openxr.h>
#include "config.h"
#include "pad_map.h"
#include "profiles.h"

namespace ck {

struct PadDispatch {
    PFN_xrStringToPath                      StringToPath = nullptr;
    PFN_xrPathToString                      PathToString = nullptr;
    PFN_xrCreateActionSet                   CreateActionSet = nullptr;
    PFN_xrCreateAction                      CreateAction = nullptr;
    PFN_xrSuggestInteractionProfileBindings SuggestInteractionProfileBindings = nullptr;
    PFN_xrAttachSessionActionSets           AttachSessionActionSets = nullptr;
    PFN_xrSyncActions                       SyncActions = nullptr;
    PFN_xrGetActionStateBoolean             GetActionStateBoolean = nullptr;
    PFN_xrGetActionStateFloat               GetActionStateFloat = nullptr;
    PFN_xrGetActionStateVector2f            GetActionStateVector2f = nullptr;
    PFN_xrGetCurrentInteractionProfile      GetCurrentInteractionProfile = nullptr;
    PFN_xrCreateActionSpace                 CreateActionSpace = nullptr;
    PFN_xrCreateReferenceSpace              CreateReferenceSpace = nullptr;
    PFN_xrLocateSpace                       LocateSpace = nullptr;
    PFN_xrDestroySpace                      DestroySpace = nullptr;
    bool Complete() const;
};

class PadInput {
public:
    PadInput(XrInstance instance, const PadDispatch& d, const Config& cfg);
    ~PadInput();                          // releases everything still held (no OpenXR calls)
    bool Enabled() const { return enabled_; }
    void ReleaseAll();                    // every held key / button up, in every session (no OpenXR calls)

    void SessionCreated(XrSession s);
    void SessionDestroyed(XrSession s);   // before the game's call is forwarded (our spaces go first)
    void SessionState(XrSession s, XrSessionState st);

    // after the game's call succeeded (bookkeeping only)
    void ActionSetCreated(XrActionSet set, const XrActionSetCreateInfo* ci);
    void ActionCreated(XrAction action, XrActionSet set);
    void Suggested(const XrInteractionProfileSuggestedBinding* sb);
    // these forward the game's call themselves (with our set added) and return what the game must see
    XrResult Attach(XrSession s, const XrSessionActionSetsAttachInfo* ai);
    XrResult Sync(XrSession s, const XrActionsSyncInfo* si);
    // game thread: after the game's xrWaitFrame (its predicted display time) / before its xrBeginFrame is forwarded (the tick)
    void FrameWaited(XrSession s, XrTime predictedDisplayTime);
    void BeginFrame(XrSession s);

private:
    struct SessionData {
        bool     appAttach = false;   // the game called xrAttachSessionActionSets
        bool     ours = false;        // our set is attached to this session
        bool     self = false;        // ... by us (the game has no input of its own): we sync it ourselves
        bool     off = false;         // our addition caused an error: controllers off for this session
        bool     focused = false;
        double   focusedSince = -1;   // the last change to FOCUSED (seconds)
        XrTime   displayTime = 0;
        double   firstFrame = -1;
        double   lastGameSync = -1;   // a game sync that carried our set (seconds)
        double   lastTick = -1;
        XrSpace  local = XR_NULL_HANDLE;
        XrSpace  aim[2] = {XR_NULL_HANDLE, XR_NULL_HANDLE};
        bool     spacesFailed = false;
        VrPad::Mapper mapper;
        uint64_t ticks = 0, readFails = 0, ownSyncs = 0;
        int      hands = -1;          // the hand mask last logged
        int      handLogs = 0;
        bool     loggedFirstSync = false, loggedFirstOwn = false;
        double   lastProfileCheck = -1;
        std::string profile[2];
    };
    bool PrepareLocked(const std::vector<XrActionSet>& appSets);
    void OffLocked(SessionData& sd, const char* what, XrResult r);
    void ReadLocked(XrSession s, SessionData& sd, VrPad::Frame& f);
    void ProfilesLocked(XrSession s, SessionData& sd, const VrPad::Frame& f, double now);
    std::string PathString(XrPath p);
    XrPath Path(const std::string& s);

    XrInstance  inst_;
    PadDispatch d_;
    Config      cfg_;
    bool        enabled_ = true;
    std::mutex  m_;
    // the game's input (this instance)
    std::map<XrActionSet, uint32_t> appSets_;                                // priority
    std::map<XrAction, XrActionSet> appActions_;
    std::map<XrPath, std::vector<XrActionSuggestedBinding>> appSuggest_;      // the game's latest suggestion per profile
    std::map<XrPath, std::string> pathNames_;
    bool frozen_ = false;             // some session of the instance has attached: no more suggestions
    bool prepared_ = false, prepareFailed_ = false;
    bool loggedLateSet_ = false;
    // ours
    XrActionSet set_ = XR_NULL_HANDLE;
    XrAction    acts_[kActCount] = {};
    XrPath      hand_[2] = {XR_NULL_PATH, XR_NULL_PATH};
    std::string ownSummary_;
    std::map<XrSession, SessionData> sessions_;
    double selfAttachS_ = 20.0;
};

double NowS();   // monotonic seconds (QPC)

}  // namespace ck
