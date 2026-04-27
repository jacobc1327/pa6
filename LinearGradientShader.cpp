/*
 *  PA4: Linear gradient GShader
 */

 #include "include/GShader.h"
 #include "include/GColor.h"
 #include "include/GPixel.h"
 #include "include/GMatrix.h"
 #include "include/GMath.h"

 #include "mesh_sample.h"

 #include <algorithm>
 #include <cmath>
 #include <vector>
 
 static inline float clamp01(float v) {
     return std::max(0.0f, std::min(1.0f, v));
 }
 
 static inline unsigned float_to_u8(float v) {
     v = clamp01(v);
     return (unsigned)std::lround(v * 255.0f);
 }
 
 static inline GPixel color_to_pixel(const GColor& c) {
     const float a = clamp01(c.a);
     const unsigned a8 = float_to_u8(a);
     const unsigned r8 = float_to_u8(clamp01(c.r) * a);
     const unsigned g8 = float_to_u8(clamp01(c.g) * a);
     const unsigned b8 = float_to_u8(clamp01(c.b) * a);
     return GPixel_PackARGB(a8, r8, g8, b8);
 }

static float apply_gradient_tile(GTileMode tm, float t) {
    switch (tm) {
        case GTileMode::kClamp:
            return clamp01(t);
        case GTileMode::kRepeat:
            t = t - std::floor(t);
            return t;
        case GTileMode::kMirror: {
            float u = std::fmod(t, 2.0f);
            if (u < 0) {
                u += 2.0f;
            }
            return (u <= 1.0f) ? u : (2.0f - u);
        }
    }
    return clamp01(t);
}

static GPixel sample_gradient_at(const std::vector<GColor>& colors, GPoint p0, GPoint p1, GTileMode tm,
                                 GPoint loc) {
    const int n = (int)colors.size();
    if (n == 0) {
        return 0;
    }
    const float fVx = p1.x - p0.x;
    const float fVy = p1.y - p0.y;
    const float fLen2 = fVx * fVx + fVy * fVy;
    float t = 0.0f;
    if (fLen2 > 1e-10f) {
        const float dx = loc.x - p0.x;
        const float dy = loc.y - p0.y;
        t = (dx * fVx + dy * fVy) / fLen2;
    }
    t = apply_gradient_tile(tm, t);

    float ti = t * (n - 1);
    int idx = (int)std::floor(ti);
    float frac = ti - idx;
    if (idx >= n - 1) {
        idx = n - 1;
        frac = 0.0f;
    }

    const GColor& c0 = colors[idx];
    const GColor& c1 = colors[std::min(idx + 1, n - 1)];
    const GColor c = {
        c0.r + (c1.r - c0.r) * frac,
        c0.g + (c1.g - c0.g) * frac,
        c0.b + (c1.b - c0.b) * frac,
        c0.a + (c1.a - c0.a) * frac,
    };
    return color_to_pixel(c);
}
 
 class LinearGradientShader : public GShader {
 public:
     LinearGradientShader(GPoint p0, GPoint p1, const GColor colors[], int count, GTileMode tm)
         : fP0(p0)
         , fP1(p1)
         , fColors(colors, colors + count)
         , fTileMode(tm)
     {}
 
     bool isOpaque() override {
         for (const auto& c : fColors) {
             if (c.a < 1.0f) return false;
         }
         return true;
     }

     friend bool mesh_sample_linear_gradient(const GShader* s, const GMatrix& ctm, GPoint local,
                                             GPixel* out);
 
     class LinearGradientContext : public Context {
     public:
         LinearGradientContext(GPoint p0, GPoint p1,
                              const std::vector<GColor>& colors,
                              const GMatrix& inverse,
                              GTileMode tm)
             : fP0(p0)
             , fP1(p1)
             , fColors(colors)
             , fInverse(inverse)
             , fTileMode(tm)
         {}

         void shadeRow(int x, int y, int count, GPixel row[]) override {
             if (fColors.empty()) {
                 return;
             }

             GPoint loc = fInverse * GPoint{x + 0.5f, y + 0.5f};
             GPoint step = fInverse.e0();

             for (int i = 0; i < count; ++i) {
                 row[i] = sample_gradient_at(fColors, fP0, fP1, fTileMode, loc);
                 loc.x += step.x;
                 loc.y += step.y;
             }
         }
 
     private:
         GPoint fP0, fP1;
         const std::vector<GColor>& fColors;
         GMatrix fInverse;
         const GTileMode fTileMode;
     };
 
     std::unique_ptr<Context> makeContext(const GMatrix& ctm) override {
         auto inv = ctm.invert();
         if (!inv.has_value()) return nullptr;
         return std::make_unique<LinearGradientContext>(fP0, fP1, fColors, *inv, fTileMode);
     }
 
 private:
     GPoint fP0, fP1;
     std::vector<GColor> fColors;
     GTileMode fTileMode;
 };
 
 std::shared_ptr<GShader> GShader::LinearGradient(GPoint p0, GPoint p1,
                                                    const GColor colors[], int count,
                                                    GTileMode tm) {
     if (count < 1) return nullptr;
     return std::make_shared<LinearGradientShader>(p0, p1, colors, count, tm);
 }

bool mesh_sample_linear_gradient(const GShader* s, const GMatrix& /*ctm*/, GPoint local, GPixel* out) {
    auto* lg = dynamic_cast<const LinearGradientShader*>(s);
    if (!lg || !out) {
        return false;
    }
    *out = sample_gradient_at(lg->fColors, lg->fP0, lg->fP1, lg->fTileMode, local);
    return true;
}