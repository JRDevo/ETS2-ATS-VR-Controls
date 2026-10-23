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
