// xr_math -- the little 3-D math the VR menu needs (header only): vectors, quaternions (x, y, z, w; OpenXR: -z forward, +x right,
// +y up), rigid poses, a ray against a quad. Doubles inside, floats only at the OpenXR boundary.
#pragma once
#include <cmath>

namespace ck {

struct V3 { double x = 0, y = 0, z = 0; };
struct Q4 { double x = 0, y = 0, z = 0, w = 1; };
struct Pose { Q4 q; V3 p; };

inline V3 operator+(V3 a, V3 b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
inline V3 operator-(V3 a, V3 b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
inline V3 operator*(V3 a, double k) { return { a.x * k, a.y * k, a.z * k }; }
inline double Dot(V3 a, V3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline double Len(V3 a) { return std::sqrt(Dot(a, a)); }

inline Q4 QMul(Q4 a, Q4 b) {
    return { a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y, a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
             a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w, a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z };
}
inline Q4 QConj(Q4 a) { return { -a.x, -a.y, -a.z, a.w }; }
inline Q4 QNorm(Q4 a) {
    const double n = std::sqrt(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
    if (n < 1e-12) return Q4();
    return { a.x / n, a.y / n, a.z / n, a.w / n };
}
inline V3 QRot(Q4 q, V3 v) {
    const Q4 r = QMul(QMul(q, Q4{ v.x, v.y, v.z, 0 }), QConj(q));
    return { r.x, r.y, r.z };
}
// yaw (rad, + = turned LEFT, about +y) then pitch (rad, + = tilted UP, about +x): no roll
inline Q4 QYawPitch(double yaw, double pitch) {
    const Q4 qy{ 0, std::sin(0.5 * yaw), 0, std::cos(0.5 * yaw) }, qx{ std::sin(0.5 * pitch), 0, 0, std::cos(0.5 * pitch) };
    return QMul(qy, qx);
}
// the yaw / pitch (QYawPitch angles) whose -z axis points along unit `fwd`
inline void YawPitchOf(V3 fwd, double* yaw, double* pitch) {
    *yaw = std::atan2(-fwd.x, -fwd.z);
    *pitch = std::asin(fwd.y < -1 ? -1 : (fwd.y > 1 ? 1 : fwd.y));
}

inline Pose PMul(const Pose& a, const Pose& b) { return { QNorm(QMul(a.q, b.q)), a.p + QRot(a.q, b.p) }; }
inline Pose PInv(const Pose& a) { const Q4 c = QConj(a.q); return { c, QRot(c, a.p * -1.0) }; }

// Hand-shake filter for a pose (a "one euro" filter): the cut-off frequency rises with the speed, so a hand held still is
// smoothed hard (the shake goes) and a fast move follows with little lag. minCut = the cut-off at rest (Hz; lower = smoother),
// beta = how much it rises per degree / second of turn. Reset: the next pose is taken as it is.
struct PoseSmoother {
    bool init = false;
    Pose s;
    void Reset() { init = false; }
    Pose Step(const Pose& in, double dt, double minCut, double beta) {
        if (!init || dt <= 0.0) { s = in; init = true; return s; }
        Q4 q = in.q;
        if (q.x * s.q.x + q.y * s.q.y + q.z * s.q.z + q.w * s.q.w < 0) q = Q4{ -q.x, -q.y, -q.z, -q.w };   // the short way
        const double dot = std::fabs(q.x * s.q.x + q.y * s.q.y + q.z * s.q.z + q.w * s.q.w);
        const double ang = 2.0 * std::acos(dot > 1.0 ? 1.0 : dot) * 57.29577951308232;   // degrees turned since the last output
        const double cut = minCut + beta * ang / dt;
        const double tau = 1.0 / (6.283185307179586 * cut), a = 1.0 / (1.0 + tau / dt);
        s.q = QNorm(Q4{ s.q.x + (q.x - s.q.x) * a, s.q.y + (q.y - s.q.y) * a, s.q.z + (q.z - s.q.z) * a, s.q.w + (q.w - s.q.w) * a });
        s.p = s.p + (in.p - s.p) * a;
        return s;
    }
};

// A ray (origin o, unit direction d) against a quad at `quad` (seen from its +z side, centre at the pose, w x h metres).
// True = hit from the front within the quad (or within `margin` metres around it): t = distance, u / v = 0..1 from the
// top-left corner (may be a little outside with a margin).
inline bool RayQuad(V3 o, V3 d, const Pose& quad, double w, double h, double margin, double* t, double* u, double* v) {
    const Q4 c = QConj(quad.q);
    const V3 lo = QRot(c, o - quad.p), ld = QRot(c, d);
    if (ld.z > -1e-6 || lo.z <= 0) return false;       // pointing away from it, or behind it
    const double k = -lo.z / ld.z;
    const double x = lo.x + ld.x * k, y = lo.y + ld.y * k;
    if (std::fabs(x) > 0.5 * w + margin || std::fabs(y) > 0.5 * h + margin) return false;
    *t = k;
    *u = x / w + 0.5;
    *v = 0.5 - y / h;
    return true;
}

}  // namespace ck
