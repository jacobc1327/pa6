/*
 *  PA4/PA5: path bounds + builder utilities (rect/polygon/oval)
 */

#include "include/GPath.h"
#include "include/GPathBuilder.h"
#include "include/GRect.h"
#include <algorithm>
#include <cmath>

static inline void accum_pt(GRect* r, GPoint p) {
    r->left   = std::min(r->left, p.x);
    r->top    = std::min(r->top, p.y);
    r->right  = std::max(r->right, p.x);
    r->bottom = std::max(r->bottom, p.y);
}

static inline GPoint eval_quad(GPoint p0, GPoint p1, GPoint p2, float t) {
    float u = 1 - t;
    return u*u*p0 + 2*u*t*p1 + t*t*p2;
}

static inline GPoint eval_cubic(GPoint p0, GPoint p1, GPoint p2, GPoint p3, float t) {
    float u = 1 - t;
    return (u*u*u)*p0 + (3*u*u*t)*p1 + (3*u*t*t)*p2 + (t*t*t)*p3;
}

static inline void accum_quad_extrema(GRect* r, GPoint p0, GPoint p1, GPoint p2) {
    // derivative: 2 * ( (p1 - p0) + t*(p2 - 2p1 + p0) )
    auto add_if = [r, p0, p1, p2](float t) {
        if (t > 0 && t < 1) {
            accum_pt(r, eval_quad(p0, p1, p2, t));
        }
    };
    const float dx = p0.x - 2*p1.x + p2.x;
    if (std::fabs(dx) > 1e-10f) {
        add_if((p0.x - p1.x) / dx);
    }
    const float dy = p0.y - 2*p1.y + p2.y;
    if (std::fabs(dy) > 1e-10f) {
        add_if((p0.y - p1.y) / dy);
    }
}

static inline void solve_quadratic(float a, float b, float c, float roots[2], int* count) {
    *count = 0;
    if (std::fabs(a) < 1e-10f) {
        if (std::fabs(b) < 1e-10f) return;
        roots[(*count)++] = -c / b;
        return;
    }
    float disc = b*b - 4*a*c;
    if (disc < 0) return;
    float s = std::sqrt(std::max(0.0f, disc));
    float q = (b < 0) ? (-b - s) * 0.5f : (-b + s) * 0.5f;
    roots[(*count)++] = q / a;
    if (std::fabs(q) > 1e-10f) {
        roots[(*count)++] = c / q;
    }
}

static inline void accum_cubic_extrema(GRect* r, GPoint p0, GPoint p1, GPoint p2, GPoint p3) {
    // p(t) = p0 + 3(p1-p0)t + 3(p2-2p1+p0)t^2 + (p3-3p2+3p1-p0)t^3
    // p'(t) = 3(p1-p0) + 6(p2-2p1+p0)t + 3(p3-3p2+3p1-p0)t^2
    auto add_roots = [r, p0, p1, p2, p3](float a, float b, float c) {
        float roots[2];
        int n;
        solve_quadratic(a, b, c, roots, &n);
        for (int i = 0; i < n; ++i) {
            float t = roots[i];
            if (t > 0 && t < 1) {
                accum_pt(r, eval_cubic(p0, p1, p2, p3, t));
            }
        }
    };

    {
        const float a = (p3.x - 3*p2.x + 3*p1.x - p0.x);
        const float b = (3*p2.x - 6*p1.x + 3*p0.x);
        const float c = (3*p1.x - 3*p0.x);
        add_roots(3*a, 2*b, c);
    }
    {
        const float a = (p3.y - 3*p2.y + 3*p1.y - p0.y);
        const float b = (3*p2.y - 6*p1.y + 3*p0.y);
        const float c = (3*p1.y - 3*p0.y);
        add_roots(3*a, 2*b, c);
    }
}

GRect GPath::bounds() const {
    if (fPts.empty()) {
        return GRect::WH(0, 0);
    }

    GRect r = GRect::LTRB(fPts[0].x, fPts[0].y, fPts[0].x, fPts[0].y);

    GPoint pts[GPath::kMaxNextPoints];
    Iter iter(*this);
    while (auto v = iter.next(pts)) {
        switch (v.value()) {
            case kMove:
                accum_pt(&r, pts[0]);
                break;
            case kLine:
                accum_pt(&r, pts[0]);
                accum_pt(&r, pts[1]);
                break;
            case kQuad:
                accum_pt(&r, pts[0]);
                accum_pt(&r, pts[2]);
                accum_quad_extrema(&r, pts[0], pts[1], pts[2]);
                break;
            case kCubic:
                accum_pt(&r, pts[0]);
                accum_pt(&r, pts[3]);
                accum_cubic_extrema(&r, pts[0], pts[1], pts[2], pts[3]);
                break;
        }
    }
    return r;
}

void GPathBuilder::addRect(const GRect& r, GPathDirection dir) {
    if (r.empty()) return;
    moveTo(r.left, r.top);
    if (dir == GPathDirection::kCW) {
        lineTo(r.right, r.top);
        lineTo(r.right, r.bottom);
        lineTo(r.left, r.bottom);
    } else {
        lineTo(r.left, r.bottom);
        lineTo(r.right, r.bottom);
        lineTo(r.right, r.top);
    }
}

void GPathBuilder::addPolygon(const GPoint pts[], int count) {
    if (count <= 0) return;
    moveTo(pts[0]);
    for (int i = 1; i < count; ++i) {
        lineTo(pts[i]);
    }
}

void GPathBuilder::addOval(const GRect& r, GPathDirection dir) {
    if (r.empty()) return;

    const float cx = (r.left + r.right) * 0.5f;
    const float cy = (r.top + r.bottom) * 0.5f;
    const float rx = (r.right - r.left) * 0.5f;
    const float ry = (r.bottom - r.top) * 0.5f;

    // 4-cubic approximation of a unit circle, scaled.
    constexpr float kKappa = 0.5522847498307936f;
    const float ox = rx * kKappa;
    const float oy = ry * kKappa;

    const GPoint top    = {cx,      cy - ry};
    const GPoint right  = {cx + rx, cy};
    const GPoint bottom = {cx,      cy + ry};
    const GPoint left   = {cx - rx, cy};

    if (dir == GPathDirection::kCW) {
        moveTo(top);
        cubicTo({cx + ox, cy - ry}, {cx + rx, cy - oy}, right);
        cubicTo({cx + rx, cy + oy}, {cx + ox, cy + ry}, bottom);
        cubicTo({cx - ox, cy + ry}, {cx - rx, cy + oy}, left);
        cubicTo({cx - rx, cy - oy}, {cx - ox, cy - ry}, top);
    } else {
        moveTo(top);
        cubicTo({cx - ox, cy - ry}, {cx - rx, cy - oy}, left);
        cubicTo({cx - rx, cy + oy}, {cx - ox, cy + ry}, bottom);
        cubicTo({cx + ox, cy + ry}, {cx + rx, cy + oy}, right);
        cubicTo({cx + rx, cy - oy}, {cx + ox, cy - ry}, top);
    }
}