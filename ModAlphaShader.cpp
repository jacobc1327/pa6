/*
 *  PA5: GShader::ModAlpha
 */

#include "include/GShader.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"

#include "mesh_sample.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

static inline unsigned mul_div_255(unsigned a, unsigned b) {
    return (a * b + 127) / 255;
}

static inline GPixel scale_pixel(GPixel p, unsigned scale) {
    const unsigned a = mul_div_255(GPixel_GetA(p), scale);
    const unsigned r = mul_div_255(GPixel_GetR(p), scale);
    const unsigned g = mul_div_255(GPixel_GetG(p), scale);
    const unsigned b = mul_div_255(GPixel_GetB(p), scale);
    return GPixel_PackARGB(a, r, g, b);
}

class ModAlphaShader : public GShader {
    friend bool mesh_sample_mod_alpha(const GShader* s, const GMatrix& ctm, GPoint local, GPixel* out);

public:
    ModAlphaShader(std::shared_ptr<GShader> base, float alphaScale)
        : fBase(std::move(base)) {
        alphaScale = std::max(0.0f, std::min(1.0f, alphaScale));
        fScale = (unsigned)std::lround(alphaScale * 255.0f);
    }

    bool isOpaque() override {
        return fBase && fScale == 255 && fBase->isOpaque();
    }

    class ModAlphaContext : public Context {
    public:
        ModAlphaContext(std::unique_ptr<GShader::Context> baseCtx, unsigned scale)
            : fBaseCtx(std::move(baseCtx)), fScale(scale) {}

        void shadeRow(int x, int y, int count, GPixel row[]) override {
            fTmp.resize(count);
            fBaseCtx->shadeRow(x, y, count, fTmp.data());
            for (int i = 0; i < count; ++i) {
                row[i] = scale_pixel(fTmp[i], fScale);
            }
        }

    private:
        std::unique_ptr<GShader::Context> fBaseCtx;
        unsigned fScale;
        std::vector<GPixel> fTmp;
    };

    std::unique_ptr<Context> makeContext(const GMatrix& ctm) override {
        if (!fBase) return nullptr;
        auto baseCtx = fBase->makeContext(ctm);
        if (!baseCtx) return nullptr;
        return std::make_unique<ModAlphaContext>(std::move(baseCtx), fScale);
    }

private:
    std::shared_ptr<GShader> fBase;
    unsigned fScale = 255;
};

std::shared_ptr<GShader> GShader::ModAlpha(std::shared_ptr<GShader> base, float alphaScale) {
    if (!base) return nullptr;
    return std::make_shared<ModAlphaShader>(std::move(base), alphaScale);
}

bool mesh_sample_mod_alpha(const GShader* s, const GMatrix& ctm, GPoint local, GPixel* out) {
    auto* m = dynamic_cast<const ModAlphaShader*>(s);
    if (!m || !m->fBase || !out) {
        return false;
    }
    GPixel basePix;
    if (!GSampleShaderAtLocal(m->fBase.get(), ctm, local, &basePix)) {
        return false;
    }
    *out = scale_pixel(basePix, m->fScale);
    return true;
}

