// pad_input -- see pad_input.h.
#include "pad_input.h"
#include "scs_input.h"
#include <windows.h>
#include <cstdio>
#include <cstring>
#include "log.h"
#include "output.h"

namespace ck {
namespace {
// pixels per degree a panel's picture gets (overlay.h): a little above today's headsets (Quest 3 via Virtual Desktop ~20-25),
// so the compositor still squeezes it a bit (sharp), never 2-3x (shimmer)
constexpr double kHeadsetPxPerDeg = 30.0;
const char* const kSetName = "ets2_ats_vr_controls";
const char* const kSetLocalized = "ETS2 ATS VR Controls";
const char* const kSimple = "/interaction_profiles/khr/simple_controller";
bool IsMenuInput(const std::string& p) {
    const size_t at = p.find("/input/menu");
    return at != std::string::npos && (p.size() == at + 11 || p.compare(at + 11, std::string::npos, "/click") == 0);
}
}  // namespace

double NowS() {
    static const LARGE_INTEGER f = [] { LARGE_INTEGER x; QueryPerformanceFrequency(&x); return x; }();
    LARGE_INTEGER c;
    QueryPerformanceCounter(&c);
    return (double)c.QuadPart / (double)f.QuadPart;
}

bool PadDispatch::Complete() const {
    return StringToPath && PathToString && CreateActionSet && CreateAction && SuggestInteractionProfileBindings &&
           AttachSessionActionSets && SyncActions && GetActionStateBoolean && GetActionStateFloat && GetActionStateVector2f &&
           GetCurrentInteractionProfile && CreateActionSpace && CreateReferenceSpace && LocateSpace && DestroySpace;
}

PadInput::PadInput(XrInstance instance, const PadDispatch& d, const Config& cfg, const std::wstring& configPath, const OverlayDispatch& od)
    : inst_(instance), d_(d), cfg_(cfg), od_(od) {
    if (!d_.Complete()) {
        LOGW("the next layer / runtime did not return every action function -- controllers off, the game's input untouched");
        enabled_ = false;
    }
    wchar_t v[32];
    if (GetEnvironmentVariableW(L"VRC_TEST_SELF_ATTACH_S", v, 32) > 0) {
        selfAttachS_ = _wtof(v);
        LOGI("VRC_TEST_SELF_ATTACH_S=%.1f (test): a game without input gets our action set after %.1f s", selfAttachS_, selfAttachS_);
    }
    Sink().SetLogEvents(cfg_.logInputs);
    Sink().SetMeasure(cfg_.log);
    if (enabled_ && cfg_.menu) {
        menu_ = std::make_unique<VrMenu>(cfg_, configPath);
        std::string combo;
        for (int b : cfg_.menuCombo) combo += (combo.empty() ? "" : " + ") + std::string(VrPad::BtnNames()[b]);
        LOGI("VR menu: hold %s for %.1f s to open it%s", combo.c_str(), (double)cfg_.menuHold, od_.Complete() ? "" : " (no swapchain functions from the runtime: it cannot be shown)");
    }
}

PadInput::~PadInput() {
    ReleaseAll();
    std::lock_guard<std::mutex> lk(m_);
    for (auto& kv : sessions_) if (kv.second.ov) kv.second.ov->Abandon();   // (the instance is gone: no OpenXR calls)
}

void PadInput::ReleaseAll() {
    std::lock_guard<std::mutex> lk(m_);
    if (menu_) menu_->ReleaseAll(Sink());
    for (auto& kv : sessions_) kv.second.mapper.Step(VrPad::Frame(), false, 0.0, cfg_.pad, Sink());
}

std::string PadInput::PathString(XrPath p) {
    if (p == XR_NULL_PATH) return std::string();
    auto it = pathNames_.find(p);
    if (it != pathNames_.end()) return it->second;
    char buf[XR_MAX_PATH_LENGTH] = "";
    uint32_t n = 0;
    std::string s;
    if (XR_SUCCEEDED(d_.PathToString(inst_, p, (uint32_t)sizeof(buf), &n, buf))) s = buf;
    pathNames_[p] = s;
    return s;
}

XrPath PadInput::Path(const std::string& s) {
    XrPath p = XR_NULL_PATH;
    if (XR_FAILED(d_.StringToPath(inst_, s.c_str(), &p))) return XR_NULL_PATH;
    pathNames_[p] = s;
    return p;
}

void PadInput::SessionCreated(XrSession s, ID3D11Device* dev) {
    std::lock_guard<std::mutex> lk(m_);
    SessionData& sd = sessions_[s] = SessionData();
    if (!menu_) return;
    if (!dev) LOGW("the game's session does not use D3D11 -- the VR menu cannot be shown in this session (the controls work)");
    else if (od_.Complete()) sd.ov = std::make_unique<Overlay>(s, dev, od_);
}

void PadInput::SessionDestroyed(XrSession s) {
    std::unique_ptr<Overlay> ov;
    {
        std::lock_guard<std::mutex> lk(m_);
        auto it = sessions_.find(s);
        if (it == sessions_.end()) return;
        ov = std::move(it->second.ov);
    }
    {
        std::lock_guard<std::mutex> lk(ovM_);                       // (not while xrEndFrame uses it)
        ov.reset();                                                  // our swapchains go before the session
    }
    std::lock_guard<std::mutex> lk(m_);
    auto it = sessions_.find(s);
    if (it == sessions_.end()) return;
    SessionData& sd = it->second;
    sd.mapper.Step(VrPad::Frame(), false, 0.0, cfg_.pad, Sink());   // nothing stays held
    if (menu_) menu_->ReleaseAll(Sink());
    menuShown_ = false; handOwn_[0] = handOwn_[1] = false;
    for (XrSpace* sp : { &sd.local, &sd.view, &sd.aim[0], &sd.aim[1] })
        if (*sp != XR_NULL_HANDLE) { d_.DestroySpace(*sp); *sp = XR_NULL_HANDLE; }
    LOGI("session %p ends: %llu frames read, %llu own syncs, %llu read failures", (void*)s, (unsigned long long)sd.ticks,
         (unsigned long long)sd.ownSyncs, (unsigned long long)sd.readFails);
    sessions_.erase(it);
}

void PadInput::SessionState(XrSession s, XrSessionState st) {
    std::lock_guard<std::mutex> lk(m_);
    auto it = sessions_.find(s);
    if (it == sessions_.end()) return;
    const bool f = st == XR_SESSION_STATE_FOCUSED;
    SessionData& sd = it->second;
    if (f != sd.focused) {
        LOGI("session %p %s", (void*)s, f ? "FOCUSED: controller input is sent" : "not focused: no input sent");
        sd.focusedSince = NowS();
    }
    sd.focused = f;
    if (!f) {                                                          // now, not at the next frame (there may be none)
        sd.mapper.Step(VrPad::Frame(), false, 0.0, cfg_.pad, Sink());
        if (menu_) menu_->ReleaseAll(Sink());
        handOwn_[0] = handOwn_[1] = false;
    }
}

void PadInput::ActionSetCreated(XrActionSet set, const XrActionSetCreateInfo* ci) {
    std::lock_guard<std::mutex> lk(m_);
    appSets_[set] = ci ? ci->priority : 0;
    LOGI("the game creates action set '%s' (priority %u)", ci ? ci->actionSetName : "?", ci ? ci->priority : 0);
    bool self = false;
    for (auto& kv : sessions_) self = self || kv.second.self;
    if (self && !loggedLateSet_) {
        loggedLateSet_ = true;
        LOGE("the game creates its own action set AFTER the layer attached its own to a game without input -- the game's own "
             "controller input will fail to attach; remove this game from 'apps' in config.ini");
    }
}

void PadInput::ActionCreated(XrAction action, XrActionSet set) {
    std::lock_guard<std::mutex> lk(m_);
    appActions_[action] = set;
}

void PadInput::Suggested(const XrInteractionProfileSuggestedBinding* sb) {
    if (!sb) return;
    std::lock_guard<std::mutex> lk(m_);
    std::vector<XrActionSuggestedBinding>& v = appSuggest_[sb->interactionProfile];
    v.assign(sb->suggestedBindings, sb->suggestedBindings + (sb->suggestedBindings ? sb->countSuggestedBindings : 0));
    std::string list;
    for (const XrActionSuggestedBinding& b : v) list += (list.empty() ? "" : ", ") + PathString(b.binding);
    LOGI("the game suggests %s: %s", ShortProfile(PathString(sb->interactionProfile)).c_str(), list.empty() ? "(nothing)" : list.c_str());
    if (prepared_ && !frozen_) prepared_ = false;   // a re-suggest after our merge (its attach failed, it retries): merge again
}

bool PadInput::PrepareLocked(const std::vector<XrActionSet>& appSets) {
    if (prepared_) return true;
    if (prepareFailed_) return false;
    // ---- our action set + actions (once per instance)
    if (set_ == XR_NULL_HANDLE) {
        prepareFailed_ = true;   // cleared below
        uint32_t prio = 0;
        for (XrActionSet s : appSets) { auto it = appSets_.find(s); if (it != appSets_.end() && it->second > prio) prio = it->second; }
        hand_[0] = Path("/user/hand/left");
        hand_[1] = Path("/user/hand/right");
        if (hand_[0] == XR_NULL_PATH || hand_[1] == XR_NULL_PATH) { LOGW("xrStringToPath(/user/hand/...) failed -- controllers off"); return false; }
        XrActionSetCreateInfo ci{XR_TYPE_ACTION_SET_CREATE_INFO};
        strcpy_s(ci.actionSetName, kSetName);
        strcpy_s(ci.localizedActionSetName, kSetLocalized);
        ci.priority = prio;
        XrResult r = d_.CreateActionSet(inst_, &ci, &set_);
        if (XR_FAILED(r)) { set_ = XR_NULL_HANDLE; LOGW("xrCreateActionSet failed (%d) -- controllers off", (int)r); return false; }
        static const XrActionType kTypes[kActCount] = { XR_ACTION_TYPE_FLOAT_INPUT, XR_ACTION_TYPE_FLOAT_INPUT,
            XR_ACTION_TYPE_BOOLEAN_INPUT, XR_ACTION_TYPE_BOOLEAN_INPUT, XR_ACTION_TYPE_BOOLEAN_INPUT, XR_ACTION_TYPE_BOOLEAN_INPUT,
            XR_ACTION_TYPE_VECTOR2F_INPUT, XR_ACTION_TYPE_POSE_INPUT };
        for (int a = 0; a < kActCount; ++a) {
            XrActionCreateInfo ai{XR_TYPE_ACTION_CREATE_INFO};
            ai.actionType = kTypes[a];
            strcpy_s(ai.actionName, ActName(a));
            strcpy_s(ai.localizedActionName, ActLocalized(a));
            ai.countSubactionPaths = 2;
            ai.subactionPaths = hand_;
            r = d_.CreateAction(set_, &ai, &acts_[a]);
            if (XR_FAILED(r)) { LOGW("xrCreateAction(%s) failed (%d) -- controllers off", ActName(a), (int)r); return false; }
        }
        LOGI("action set '%s' priority %u (= the highest of the game's attached sets: the game and the layer both see every "
             "button), %d actions x 2 hands", kSetName, prio, (int)kActCount);
        prepareFailed_ = false;
    }
    // ---- suggestions: every profile the game suggested (merged), or all of ours when it suggested none
    std::vector<std::string> profiles;
    std::map<std::string, std::vector<XrActionSuggestedBinding>> base;   // the game's bindings per profile: its own or translated
    std::map<std::string, bool> translated;
    const bool appHas = !appSuggest_.empty();
    if (appHas) {
        const std::vector<XrActionSuggestedBinding>* simple = nullptr;
        bool onlySimple = true;
        for (const auto& kv : appSuggest_) {
            const std::string p = PathString(kv.first);
            if (p == kSimple) simple = &kv.second; else onlySimple = false;
            if (IsKnownProfile(p)) { profiles.push_back(p); base[p] = kv.second; }
            else LOGI("profile %s (suggested by the game) is not one the layer knows -- left as the game made it", p.c_str());
        }
        if (onlySimple && simple) {
            std::string names;
            size_t dropped = 0;
            for (const char* t : SimpleTranslationTargets()) {
                std::vector<XrActionSuggestedBinding> tr;
                for (const XrActionSuggestedBinding& sb : *simple) {
                    std::string op;
                    if (TranslateSimpleBinding(t, PathString(sb.binding), op)) tr.push_back({sb.action, Path(op)});
                    else ++dropped;
                }
                profiles.push_back(t);
                base[t] = tr;
                translated[t] = true;
                names += (names.empty() ? "" : ", ") + ShortProfile(t);
            }
            LOGI("the game suggests only khr/simple_controller (%zu bindings: no sticks, no A / B) -- also suggesting %s with the "
                 "game's bindings translated (select -> trigger, menu -> menu; %zu without an equivalent left out), so the runtime "
                 "can use the real controller and the game keeps its buttons", simple->size(), names.c_str(), dropped);
        }
    } else {
        for (const char* k : KnownProfiles()) profiles.push_back(k);
    }
    // the menu actions: any action suggested on a menu input, on any profile (by the game or by another layer passing it on)
    std::set<XrAction> menuActs;
    for (const auto& kv : appSuggest_)
        for (const XrActionSuggestedBinding& g : kv.second) if (IsMenuInput(PathString(g.binding))) menuActs.insert(g.action);
    int ok = 0;
    std::string summary;
    for (const std::string& prof : profiles) {
        std::vector<XrActionSuggestedBinding> all;
        auto bit = base.find(prof);
        if (bit != base.end()) all = bit->second;
        for (const XrActionSuggestedBinding& g : all) {                // the game's trigger actions (hidden while the menu owns it)
            const std::string gp = PathString(g.binding);
            if (gp.find("/input/trigger") != std::string::npos || gp.find("/input/select") != std::string::npos) gameTrig_.insert(g.action);
        }
        // the game's menu button on this profile is left to it (Index: b/click; Vive / WMR: the right menu) -- one press must not
        // do two things. Every other button is shared (both see it), whoever bound it.
        std::vector<XrActionSuggestedBinding> ours;
        std::string left, lost;                      // lost: config.ini buttons that do something and are dropped here
        for (int a = 0; a < kActCount; ++a)
            for (int h = 0; h < 2; ++h) {
                std::string p;
                if (!ActPath(prof, a, h, p)) continue;
                bool taken = false;
                if (a != kActTrig && a != kActAim)
                    for (const XrActionSuggestedBinding& g : all)
                        taken = taken || (menuActs.count(g.action) && SameInput(PathString(g.binding), p));
                if (!taken) { ours.push_back({acts_[a], Path(p)}); continue; }
                left += (left.empty() ? "" : ", ") + p;
                std::vector<int> btns;               // the config.ini buttons this action + hand stands for
                switch (a) {
                case kActGrip:  btns = { h ? VrPad::kRGrip : VrPad::kLGrip }; break;
                case kActPrim:  btns = { h ? VrPad::kA : VrPad::kX }; break;
                case kActSec:   btns = { h ? VrPad::kB : VrPad::kY }; break;
                case kActClick: btns = { h ? VrPad::kRStick : VrPad::kLStick }; break;
                case kActMenu:  btns = { VrPad::kMenu }; break;
                case kActStick: btns = h ? std::vector<int>{ VrPad::kRUp, VrPad::kRDown, VrPad::kRLeft, VrPad::kRRight }
                                         : std::vector<int>{ VrPad::kLUp, VrPad::kLDown, VrPad::kLLeft, VrPad::kLRight };
                                if (cfg_.pad.mouseStick == h + 1) lost += (lost.empty() ? "" : ", ") + std::string(h ? "right" : "left") + " stick mouse";
                                break;
                default: break;
                }
                for (int b : btns)
                    if (b == cfg_.pad.airBtn || cfg_.pad.map[b].kind != VrPad::kNone)
                        lost += (lost.empty() ? "" : ", ") + std::string(VrPad::BtnNames()[b]) + (b == cfg_.pad.airBtn ? " (air mouse)" : "");
            }
        if (!left.empty()) LOGI("%s: %s is the game's menu button -- left to the game (no mod key there)", ShortProfile(prof).c_str(), left.c_str());
        if (!lost.empty())
            LOGW("%s: config.ini %s will do nothing on this controller -- it is the game's menu button", ShortProfile(prof).c_str(), lost.c_str());
        const XrPath pp = Path(prof);
        auto it = appSuggest_.find(pp);
        const size_t nApp = all.size();
        const bool isTr = translated.count(prof) != 0;
        all.insert(all.end(), ours.begin(), ours.end());
        XrInteractionProfileSuggestedBinding sb{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
        sb.interactionProfile = pp;
        sb.countSuggestedBindings = (uint32_t)all.size();
        sb.suggestedBindings = all.data();
        const XrResult r = d_.SuggestInteractionProfileBindings(inst_, &sb);
        if (XR_SUCCEEDED(r)) {
            ++ok;
            LOGI("%zu bindings on %s (%s)", ours.size(), ShortProfile(prof).c_str(),
                 it != appSuggest_.end() ? ("merged into the game's " + std::to_string(nApp) + " bindings of this profile").c_str()
                 : isTr ? ("with the game's " + std::to_string(nApp) + " simple_controller bindings translated").c_str()
                        : "the game suggests no profile: ours only");
            summary += (summary.empty() ? "" : ", ") + ShortProfile(prof);
            continue;
        }
        if (it != appSuggest_.end()) {
            XrInteractionProfileSuggestedBinding back{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
            back.interactionProfile = pp;
            back.countSuggestedBindings = (uint32_t)it->second.size();
            back.suggestedBindings = it->second.data();
            const XrResult rb = d_.SuggestInteractionProfileBindings(inst_, &back);
            LOGW("the runtime refused our bindings on %s (%d) -- the game's own %zu bindings re-suggested (%d)",
                 ShortProfile(prof).c_str(), (int)r, it->second.size(), (int)rb);
        } else {
            LOGI("profile %s not accepted by the runtime (%d) -- skipped", ShortProfile(prof).c_str(), (int)r);
        }
    }
    if (!ok) {
        LOGW("no controller profile took our bindings -- controllers off for this game");
        prepareFailed_ = true;
        return false;
    }
    ownSummary_ = summary;
    prepared_ = true;
    return true;
}

void PadInput::OffLocked(SessionData& sd, const char* what, XrResult r) {
    if (sd.off) return;
    sd.off = true;
    LOGW("%s failed (%d) -- controllers off for this session (the game's own input untouched)", what, (int)r);
}

XrResult PadInput::Attach(XrSession s, const XrSessionActionSetsAttachInfo* ai) {
    std::unique_lock<std::mutex> lk(m_);
    auto it = sessions_.find(s);
    if (it == sessions_.end() || !ai) { lk.unlock(); return d_.AttachSessionActionSets(s, ai); }
    SessionData& sd = it->second;
    sd.appAttach = true;
    if (!enabled_ || sd.off) { lk.unlock(); return d_.AttachSessionActionSets(s, ai); }
    if (sd.ours) {
        LOGE("the game attaches its action sets after the layer attached its own (it had none for %.1f s) -- the runtime will "
             "refuse the game's call; remove this game from 'apps' in config.ini", selfAttachS_);
        lk.unlock();
        return d_.AttachSessionActionSets(s, ai);
    }
    std::vector<XrActionSet> list(ai->actionSets, ai->actionSets + (ai->actionSets ? ai->countActionSets : 0));
    const bool ready = frozen_ ? prepared_ : PrepareLocked(list);
    if (!ready) {
        const XrResult r = d_.AttachSessionActionSets(s, ai);
        if (XR_SUCCEEDED(r)) frozen_ = true;
        return r;
    }
    list.push_back(set_);
    XrSessionActionSetsAttachInfo a2 = *ai;
    a2.countActionSets = (uint32_t)list.size();
    a2.actionSets = list.data();
    const XrResult r = d_.AttachSessionActionSets(s, &a2);
    if (XR_SUCCEEDED(r)) {
        frozen_ = true;
        trigReady_ = true;                           // (gameTrig_ no longer changes: the filters may read it without m_)
        sd.ours = true;
        sd.gameActive.clear();                       // until the game's first sync: all of its attached sets count as active
        for (uint32_t k = 0; k < ai->countActionSets; ++k) sd.gameActive.push_back({ai->actionSets[k], XR_NULL_PATH});
        LOGI("attached with the game's %u action set(s) (profiles: %s) -- %s", ai->countActionSets, ownSummary_.c_str(),
             DescribePad(cfg_.pad).c_str());
        return r;
    }
    const XrResult r2 = d_.AttachSessionActionSets(s, ai);
    if (XR_SUCCEEDED(r2)) {
        frozen_ = true;
        sd.off = true;
        LOGW("xrAttachSessionActionSets with our action set failed (%d); the game's own sets attached alone -- controllers off for "
             "this session", (int)r);
    }
    return r2;
}

XrResult PadInput::Sync(XrSession s, const XrActionsSyncInfo* si) {
    std::unique_lock<std::mutex> lk(m_);
    auto it = sessions_.find(s);
    if (it == sessions_.end() || !si || !it->second.ours || it->second.off || it->second.self) {
        lk.unlock();
        return d_.SyncActions(s, si);
    }
    std::vector<XrActiveActionSet> act(si->activeActionSets, si->activeActionSets + (si->activeActionSets ? si->countActiveActionSets : 0));
    act.push_back({set_, XR_NULL_PATH});
    lk.unlock();
    XrActionsSyncInfo s2 = *si;
    s2.countActiveActionSets = (uint32_t)act.size();
    s2.activeActionSets = act.data();
    const XrResult r = d_.SyncActions(s, &s2);
    lk.lock();
    auto j = sessions_.find(s);
    if (XR_FAILED(r)) {
        lk.unlock();
        const XrResult r2 = d_.SyncActions(s, si);
        if (XR_SUCCEEDED(r2)) {
            lk.lock();
            auto k = sessions_.find(s);
            if (k != sessions_.end()) OffLocked(k->second, "xrSyncActions with our action set (the game's own call succeeds)", r);
        }
        return r2;
    }
    if (j != sessions_.end()) {
        j->second.lastGameSync = NowS();
        j->second.gameActive.assign(act.begin(), act.end() - 1);   // the game's own list (our own syncs repeat it)
        if (!j->second.loggedFirstSync) {
            j->second.loggedFirstSync = true;
            LOGI("first game xrSyncActions with our action set appended (%d)", (int)r);
        }
    }
    return r;
}

void PadInput::FrameWaited(XrSession s, XrTime t) {
    std::lock_guard<std::mutex> lk(m_);
    auto it = sessions_.find(s);
    if (it != sessions_.end()) it->second.displayTime = t;
}

void PadInput::ReadLocked(XrSession s, SessionData& sd, VrPad::Frame& f) {
    if (!sd.spacesFailed && sd.local == XR_NULL_HANDLE) {   // our LOCAL space + the aim spaces, once per session
        XrReferenceSpaceCreateInfo rc{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        rc.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
        rc.poseInReferenceSpace.orientation.w = 1.0f;
        XrResult r = d_.CreateReferenceSpace(s, &rc, &sd.local);
        if (XR_SUCCEEDED(r) && menu_) {
            rc.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
            if (XR_FAILED(d_.CreateReferenceSpace(s, &rc, &sd.view))) { sd.view = XR_NULL_HANDLE; LOGW("no VIEW space: the VR menu opens at the room's centre"); }
        }
        for (int h = 0; XR_SUCCEEDED(r) && h < 2; ++h) {
            XrActionSpaceCreateInfo ac{XR_TYPE_ACTION_SPACE_CREATE_INFO};
            ac.action = acts_[kActAim];
            ac.subactionPath = hand_[h];
            ac.poseInActionSpace.orientation.w = 1.0f;
            r = d_.CreateActionSpace(s, &ac, &sd.aim[h]);
        }
        if (XR_FAILED(r)) {
            sd.spacesFailed = true;
            LOGW("creating the aim spaces failed (%d) -- no air mouse in this session (buttons and sticks still work)", (int)r);
        }
    }
    int fails = 0;
    XrResult firstFail = XR_SUCCESS;
    // the aim poses: for the menu / keyboard pointer, the air mouse (its button may be on either hand: located for both hands
    // whenever the air mouse could run), steering with the hands (wheel / point: every frame) -- with the menu closed, no air
    // mouse and stick steering, 2 runtime calls fewer per frame
    const bool needAim = (menu_ && (menu_->AnyShown() || menu_->Pending())) || (cfg_.pad.airBtn >= 0 &&
                         (cfg_.pad.airToggle || sd.mapper.AirOn() || sd.airHeld)) ||
                         (cfg_.pad.drive && (cfg_.pad.steerWith == 1 || cfg_.pad.steerWith == 2));
    auto note = [&](XrResult r) { if (XR_FAILED(r) && !fails++) firstFail = r; };
    for (int h = 0; h < 2; ++h) {
        XrActionStateGetInfo gi{XR_TYPE_ACTION_STATE_GET_INFO};
        gi.subactionPath = hand_[h];
        bool act = false;
        float* const fl[2] = { &f.trig[h], &f.grip[h] };
        for (int k = 0; k < 2; ++k) {
            XrActionStateFloat st{XR_TYPE_ACTION_STATE_FLOAT};
            gi.action = acts_[kActTrig + k];
            note(d_.GetActionStateFloat(s, &gi, &st));
            *fl[k] = st.isActive ? st.currentState : 0.0f;
            act = act || st.isActive;
        }
        bool* const bo[4] = { &f.prim[h], &f.sec[h], &f.click[h], &f.menu[h] };
        for (int k = 0; k < 4; ++k) {
            XrActionStateBoolean st{XR_TYPE_ACTION_STATE_BOOLEAN};
            gi.action = acts_[kActPrim + k];
            note(d_.GetActionStateBoolean(s, &gi, &st));
            *bo[k] = st.isActive && st.currentState;
            act = act || st.isActive;
        }
        XrActionStateVector2f sv{XR_TYPE_ACTION_STATE_VECTOR2F};
        gi.action = acts_[kActStick];
        note(d_.GetActionStateVector2f(s, &gi, &sv));
        if (sv.isActive) { f.sx[h] = sv.currentState.x; f.sy[h] = sv.currentState.y; act = true; }
        f.hand[h] = act;
        if (sd.aim[h] != XR_NULL_HANDLE && sd.displayTime > 0 && needAim) {
            XrSpaceLocation loc{XR_TYPE_SPACE_LOCATION};
            const XrResult r = d_.LocateSpace(sd.aim[h], sd.local, sd.displayTime, &loc);
            note(r);
            f.aim[h] = XR_SUCCEEDED(r) && (loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) &&
                       (loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT);
            f.q[h][0] = loc.pose.orientation.x; f.q[h][1] = loc.pose.orientation.y;
            f.q[h][2] = loc.pose.orientation.z; f.q[h][3] = loc.pose.orientation.w;
            f.pos[h] = XR_SUCCEEDED(r) && (loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT);
            f.p[h][0] = loc.pose.position.x; f.p[h][1] = loc.pose.position.y; f.p[h][2] = loc.pose.position.z;
        }
    }
    sd.headOk = false;
    if (sd.view != XR_NULL_HANDLE && sd.displayTime > 0) {
        XrSpaceLocation loc{XR_TYPE_SPACE_LOCATION};
        if (XR_SUCCEEDED(d_.LocateSpace(sd.view, sd.local, sd.displayTime, &loc)) &&
            (loc.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT) && (loc.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)) {
            sd.headOk = true;
            sd.head.q = Q4{ loc.pose.orientation.x, loc.pose.orientation.y, loc.pose.orientation.z, loc.pose.orientation.w };
            sd.head.p = V3{ loc.pose.position.x, loc.pose.position.y, loc.pose.position.z };
            const double hp[3] = { sd.head.p.x, sd.head.p.y, sd.head.p.z }, hq[4] = { sd.head.q.x, sd.head.q.y, sd.head.q.z, sd.head.q.w };
            VrHeadSet(hp, hq);                         // (log = 1 only: the log's cab line)
        }
    }
    if (fails) {
        if (sd.readFails++ < 8) LOGW("%d controller state read(s) failed this frame, first %d (#%llu)", fails, (int)firstFail,
                                     (unsigned long long)sd.readFails);
        if (firstFail == XR_ERROR_ACTIONSET_NOT_ATTACHED || firstFail == XR_ERROR_SESSION_LOST) OffLocked(sd, "reading the controller states", firstFail);
    }
}

void PadInput::ProfilesLocked(XrSession s, SessionData& sd, const VrPad::Frame& f, double now) {
    const int hands = (f.hand[0] ? 1 : 0) | (f.hand[1] ? 2 : 0);
    if (sd.handLogs >= 16 || (hands == sd.hands && sd.lastProfileCheck >= 0 && now - sd.lastProfileCheck < 2.0)) return;
    sd.lastProfileCheck = now;
    std::string cur[2];
    for (int h = 0; h < 2; ++h) {
        XrInteractionProfileState ip{XR_TYPE_INTERACTION_PROFILE_STATE};
        if (XR_SUCCEEDED(d_.GetCurrentInteractionProfile(s, hand_[h], &ip))) cur[h] = PathString(ip.interactionProfile);
    }
    if (hands == sd.hands && cur[0] == sd.profile[0] && cur[1] == sd.profile[1]) return;
    sd.hands = hands;
    sd.profile[0] = cur[0];
    sd.profile[1] = cur[1];
    ++sd.handLogs;
    LOGI("left controller %s (%s), right %s (%s)", f.hand[0] ? "ACTIVE" : "inactive", cur[0].empty() ? "no profile" : ShortProfile(cur[0]).c_str(),
         f.hand[1] ? "ACTIVE" : "inactive", cur[1].empty() ? "no profile" : ShortProfile(cur[1]).c_str());
}

void PadInput::BeginFrame(XrSession s) {
    if (!enabled_) return;
    const bool meas = cfg_.log;                                      // (log = 0: nothing measured)
    const double t0 = meas ? NowS() : 0.0;
    if (meas) { Sink().TakeSendTime(); for (double& x : split_) x = 0; }
    std::lock_guard<std::mutex> lk(m_);
    auto it = sessions_.find(s);
    if (it == sessions_.end()) return;
    SessionData& sd = it->second;
    const double now = NowS();
    if (meas) split_[3] = now - t0;
    struct Part {                                                    // adds the time of a scope to split_[i]
        double* to; double t;
        Part(bool on, double* d) : to(on ? d : nullptr), t(on ? NowS() : 0.0) {}
        ~Part() { if (to) *to += NowS() - t; }
    };
    if (sd.firstFrame < 0) sd.firstFrame = now;
    // a game without any input of its own: ours alone after selfAttachS_ of frames
    if (!sd.ours && !sd.appAttach && !sd.off && !frozen_ && appSets_.empty() && appSuggest_.empty() && now - sd.firstFrame >= selfAttachS_) {
        if (!PrepareLocked({})) {
            sd.off = true;
        } else {
            XrSessionActionSetsAttachInfo a{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
            a.countActionSets = 1;
            a.actionSets = &set_;
            const XrResult r = d_.AttachSessionActionSets(s, &a);
            if (XR_FAILED(r)) {
                OffLocked(sd, "attaching our own action set (the game has none)", r);
            } else {
                frozen_ = true;
                sd.ours = sd.self = true;
                LOGI("the game has no controller input of its own (no action set after %.1f s of frames) -- the layer attached its own "
                     "and syncs it every frame (profiles: %s) -- %s", selfAttachS_, ownSummary_.c_str(), DescribePad(cfg_.pad).c_str());
            }
        }
    }
    VrPad::Frame f;
    bool live = false;
    if (sd.ours && !sd.off && sd.focused) {
        const double age = sd.lastGameSync < 0 ? -1.0 : now - sd.lastGameSync;
        // fresh = the game's own sync (with ours appended) within 0.25 s: just read. Our own sync otherwise, but never with only
        // our set while the game has input: the game's active sets go into it too, so they stay active (see pad_input.h).
        // The first 2 s of FOCUSED without any game sync: wait for it (nothing is held yet).
        const bool fresh = !sd.self && age >= 0 && age <= 0.25;
        if (!fresh && !sd.self && age < 0 && (sd.focusedSince < 0 || now - sd.focusedSince < 2.0)) return;
        bool synced = fresh;
        Part px(meas, &split_[0]);
        if (!fresh) {
            std::vector<XrActiveActionSet> act;
            if (!sd.self) act = sd.gameActive;
            act.push_back({set_, XR_NULL_PATH});
            XrActionsSyncInfo si{XR_TYPE_ACTIONS_SYNC_INFO};
            si.countActiveActionSets = (uint32_t)act.size();
            si.activeActionSets = act.data();
            const XrResult r = d_.SyncActions(s, &si);
            if (XR_FAILED(r)) {
                OffLocked(sd, "xrSyncActions of our action set", r);
            } else {
                synced = true;
                ++sd.ownSyncs;
                if (!sd.loggedFirstOwn) {
                    sd.loggedFirstOwn = true;
                    LOGI("the layer syncs itself (%s; %zu of the game's sets kept active) (%d)",
                         sd.self ? "the game has no input" : age < 0 ? "the game attached its input but has not synced in 2 s of FOCUSED"
                                                                     : "the game's last sync is older than 0.25 s",
                         act.size() - 1, (int)r);
                }
            }
        }
        if (synced && !sd.off) {
            ReadLocked(s, sd, f);
            sd.airHeld = cfg_.pad.airBtn >= 0 && BtnDown(f, cfg_.pad.airBtn);   // (its aim pose is located from the next frame)
            ++sd.ticks;
            ProfilesLocked(s, sd, f, now);
            live = !sd.off && cfg_.enabled && GameInFront();
        }
    }
    const double dt = sd.lastTick < 0 ? 0.0 : now - sd.lastTick;   // since the last Step (frames that waited are not lost)
    sd.lastTick = now;
    if (menu_) {                                                     // first: it takes what it uses out of the frame
        Part pm(meas, &split_[1]);
        menu_->Update(f, live, sd.headOk, sd.head, now, Sink());
        cfg_.pad = menu_->Pad();
        menuShown_ = menu_->AnyShown();
        handOwn_[0] = menu_->HandOnPanel(0);
        handOwn_[1] = menu_->HandOnPanel(1);
    }
    {
        Part pm(meas, &split_[1]);
        sd.mapper.SetSpeed(DriveSpeedKmh());             // (speed-sensitive steering; -1 = no telemetry)
        sd.mapper.SetHands(menu_ && menu_->HandOnPanel(0), menu_ && menu_->HandOnPanel(1),   // (point / wheel steering pause)
                           menu_ && menu_->HandHolding(0), menu_ && menu_->HandHolding(1));
        sd.mapper.Step(f, live, dt, cfg_.pad, Sink());   // (not live = everything held is released)
        DriveStickSet(sd.mapper.StickX(), sd.mapper.StickY());               // (log = 1: the drive line shows the thumb too)
        const int kb = sd.mapper.TakeKeyboardToggles();  // a button set to "keyboard": show / hide the VR keyboard
        if (menu_ && (kb & 1)) menu_->ToggleKeyboard();
    }
    if (meas) {                                          // (SendInput ran inside the menu / the mapper: counted apart)
        split_[2] = Sink().TakeSendTime();
        split_[1] = split_[1] > split_[2] ? split_[1] - split_[2] : 0.0;
    }
}

bool PadInput::EndFrame(XrSession s, const XrFrameEndInfo* in, XrFrameEndInfo* out, std::vector<const XrCompositionLayerBaseHeader*>& layers) {
    if (!menu_ || !in) return false;
    // 1. under m_ (no runtime / D3D call): what to show, at what size; each hidden panel's swapchain (and the pointer's) is
    //    made and filled ahead of time ~3 s into the session, one per frame, so opening the menu costs neither in a VR frame
    Overlay* ov = nullptr;
    XrSpace space = XR_NULL_HANDLE;
    QuadIn panel[VrMenu::kPanCount];
    bool want[VrMenu::kPanCount] = {}, show[VrMenu::kPanCount] = {}, ready[VrMenu::kPanCount] = {};
    std::vector<QuadIn> quads, warmQ;
    int warmSlot = -1;
    {
        std::lock_guard<std::mutex> lk(m_);
        auto it = sessions_.find(s);
        if (it == sessions_.end()) return false;
        SessionData& sd = it->second;
        if (!sd.ov || sd.ov->Dead() || sd.local == XR_NULL_HANDLE) return false;
        ov = sd.ov.get();
        space = sd.local;
        const bool warmTime = ++sd.endFrames > 270;
        for (int w = 0; w < 3 && warmTime && warmSlot < 0; ++w)
            if (!sd.warm[w] && (w == 2 || !menu_->Pan(w).show)) warmSlot = w;
            else if (!sd.warm[w]) sd.warm[w] = true;                     // (shown already: made when it was shown)
        if (!menu_->AnyShown() && warmSlot < 0) return false;
        const bool rgba = ov->Rgba();                                    // (the format is asked once)
        for (int p = 0; p < VrMenu::kPanCount; ++p) {
            const VrMenu::Panel& pn = menu_->Pan(p);
            show[p] = pn.show;
            want[p] = pn.show || warmSlot == p;
            if (!want[p]) continue;
            double wantPx = 0;
            if (sd.headOk) {                   // its width in the headset: angle x pixels per degree (overlay.h)
                const double dist = Len(pn.pose.p - sd.head.p);
                if (dist > 0.05) wantPx = 2.0 * std::atan(0.5 * pn.width / dist) * 57.29577951308232 * kHeadsetPxPerDeg;
            }
            ready[p] = menu_->Output(p, wantPx, rgba);
            QuadIn& q = panel[p];
            q.slot = p; q.pose = pn.pose; q.w = pn.width; q.h = pn.Height();
        }
        for (int h = 0; h < 2; ++h) {
            QuadIn q;
            if (!menu_->Cursor(h, &q.pose)) continue;
            q.slot = 2; q.w = q.h = VrMenu::kCursorM; q.pw = q.ph = VrMenu::kCursorPx; q.px = menu_->CursorPixels().data(); q.version = 1;
            quads.push_back(q);
        }
        if (warmSlot == 2) {
            QuadIn q;
            q.slot = 2; q.w = q.h = VrMenu::kCursorM; q.pw = q.ph = VrMenu::kCursorPx; q.px = menu_->CursorPixels().data(); q.version = 1;
            warmQ.push_back(q);
            sd.warm[2] = true;
        }
        if (warmSlot >= 0 && warmSlot < 2 && ready[warmSlot]) sd.warm[warmSlot] = true;   // (not ready: the painter is on it, next frame)
    }
    // 2. outside m_: the overlay (ovM_ keeps it alive), the pictures under the painter's lock (copied as they are, no work)
    std::lock_guard<std::mutex> olk(ovM_);
    {
        std::lock_guard<std::mutex> lk(m_);                              // (still this session's overlay?)
        auto it = sessions_.find(s);
        if (it == sessions_.end() || it->second.ov.get() != ov) return false;
    }
    std::vector<const XrCompositionLayerBaseHeader*> ours;
    {
        std::unique_lock<std::mutex> pl = menu_->LockPictures();
        for (int p = 0; p < VrMenu::kPanCount; ++p) {
            if (!want[p] || !ready[p]) continue;
            VrMenu::Picture& pic = menu_->PictureLocked(p);
            if (pic.px.empty()) continue;
            QuadIn q = panel[p];
            q.pw = pic.w; q.ph = pic.h; q.px = pic.px.data(); q.version = pic.version;
            q.cw = menu_->Pan(p).pw; q.ch = menu_->Pan(p).ph;           // (the swapchain: the full size)
            for (int i = 0; i < 4; ++i) { q.dirty[i] = pic.dirty[i]; pic.dirty[i] = 0; }
            if (show[p]) quads.insert(quads.begin(), q);
            else warmQ.push_back(q);
        }
        if (!warmQ.empty()) {
            ov->Frame(warmQ, space, ours, false);
            LOGI("VR menu: %s swapchain made and filled ahead of time", warmQ[0].slot == 0 ? "menu" : warmQ[0].slot == 1 ? "keyboard" : "pointer");
        }
        if (!show[0] && !show[1]) return false;
        ov->Frame(quads, space, ours);
    }
    if (ours.empty() || in->layerCount + ours.size() > 16) return false;   // (16 = the least any runtime allows)
    layers.assign(in->layers, in->layers + (in->layers ? in->layerCount : 0));
    layers.insert(layers.end(), ours.begin(), ours.end());
    *out = *in;
    out->layerCount = (uint32_t)layers.size();
    out->layers = layers.data();
    return true;
}

void PadInput::EndFrameRefused(XrSession s, XrResult r) {
    std::lock_guard<std::mutex> olk(ovM_);
    std::lock_guard<std::mutex> lk(m_);
    auto it = sessions_.find(s);
    if (it == sessions_.end() || !it->second.ov) return;
    char b[96];
    snprintf(b, sizeof(b), "the runtime refused xrEndFrame with the menu's layers (%d); the game's frame was sent alone", (int)r);
    it->second.ov->Kill(b);
}

bool PadInput::MenuOwns(XrAction a, XrPath sub) {
    if (!menuShown_ && !handOwn_[0] && !handOwn_[1]) return false;
    if (!trigReady_ || !gameTrig_.count(a)) return false;
    if (sub == hand_[0]) return handOwn_[0];
    if (sub == hand_[1]) return handOwn_[1];
    return handOwn_[0] || handOwn_[1];
}

void PadInput::FilterBool(XrSession, const XrActionStateGetInfo* gi, XrActionStateBoolean* st) {
    if (!menu_ || !gi || !st) return;
    if (MenuOwns(gi->action, gi->subactionPath)) { st->currentState = XR_FALSE; st->changedSinceLastSync = XR_FALSE; }
}

void PadInput::FilterFloat(XrSession, const XrActionStateGetInfo* gi, XrActionStateFloat* st) {
    if (!menu_ || !gi || !st) return;
    if (MenuOwns(gi->action, gi->subactionPath)) { st->currentState = 0.0f; st->changedSinceLastSync = XR_FALSE; }
}

}  // namespace ck
