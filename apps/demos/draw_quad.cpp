/**
 *  Copyright 2026 Mike Reed
 */

#include "draw.h"
#include "draw_tools.h"
#include "../../include/GPathBuilder.h"

static void draw_quad_lines(GCanvas* canvas, const GPoint pts[4], int level) {
    const int n = level + 1;
    for (int y = 0; y <= n; ++y) {
        const float u = y * 1.0f / n;
        GPoint p0 = lerp(pts[0], pts[1], u),
                p1 = lerp(pts[3], pts[2], u);
        canvas->hairLine(p0, p1, GColor_black);
    }
    for (int x = 0; x <= n; ++x) {
        const float v = x * 1.0f / n;
        GPoint p0 = lerp(pts[0], pts[3], v),
                p1 = lerp(pts[1], pts[2], v);
        canvas->hairLine(p0, p1, GColor_black);
    }
}

static void draw_quad_diags(GCanvas* canvas, const GPoint pts[4], int level, bool majorDiag) {
    const int n = level + 1;
    auto lrp = [=](int x, int y) -> GPoint {
        const float u = x * 1.0f / n,
                    v = y * 1.0f / n;
        return lerp(lerp(pts[0], pts[1], u),
                    lerp(pts[3], pts[2], u), v);
    };
    for (int y = 0; y < n; ++y) {
        if (majorDiag) {
            for (int x = 0; x < n; ++x) {
                canvas->hairLine(lrp(x, y), lrp(x+1, y+1), GColor_black);
            }
        } else {
            for (int x = 0; x < n; ++x) {
                canvas->hairLine(lrp(x + 1, y), lrp(x, y + 1), GColor_black);
            }
        }
    }
}

class QuadShape : public Shape {
    using INHERITED = Shape;
    std::array<GPoint, 4> fPts;
    int                   fLevel = 0;
    bool fInitialRectMode;
    bool fShowExtra = false,
         fShowColors = true,
         fShowTexs = false;
    int  fShowDiag = 0;
    std::array<GColor, 4> fColors;
    std::array<GPoint, 4> fTexs;
    std::shared_ptr<GShader> fBMShader;

    std::pair<const GColor*, const GPoint*> payloads() {
        // We should only do this when the paint's color's alpha changes, but we don't have a notification
        // for that (yet), so we do it every time.
        float alpha = fPaint.alpha();
        for (auto& c : fColors) {
            c.a = alpha;
        }
        if (fShowColors && fShowTexs) {
            alpha = 1;  // otherwise we'd modulate the alpha of colors AND bitmap, and when they combine
                        // we'd double modulate the result.
        }
        if (fBMShader) {
            // this is safely ignored in the draw if fShowTexs is not set
            fPaint.setShader(GShader::ModAlpha(fBMShader, alpha));
        }

        return {
            fShowColors ? fColors.data() : nullptr,
            fShowTexs ? fTexs.data() : nullptr,
        };
    }

public:
    QuadShape(GPoint p) {
        fInitialRectMode = true;
        std::fill(fPts.begin(), fPts.end(), p);

        fColors = {GColor_red, GColor_green, GColor_blue, GColor_magenta};
        fTexs = {GPoint{0, 0}, GPoint{1, 0}, GPoint{1, 1}, GPoint{0, 1}};

        auto bm = GBitmap::ReadFromFile("apps/spock.png");
        if (bm) {
            fBMShader = GShader::Bitmap(*bm, GMatrix::Scale(1.0f / bm->width(), 1.0f / bm->height()));
        }
    }

    void onDraw(GCanvas* canvas) override {
        auto [colors, texs] = this->payloads();
        if (colors || texs) {
            canvas->drawQuad(fPts.data(), colors, texs, fLevel, fPaint);
        } else {
            draw_quad_lines(canvas, fPts.data(), fLevel);
        }
    }

    void onDrawHilite(GCanvas* canvas) override {
        if (fShowExtra) {
            auto [colors, texs] = this->payloads();
            if (colors || texs) {
                draw_quad_lines(canvas, fPts.data(), fLevel);
            }
            if (fShowDiag != 0) {
                draw_quad_diags(canvas, fPts.data(), fLevel, fShowDiag == 1);
            }
        }

        GPaint paint;
        paint.setLineWidth(6);
        paint.setColor(GColor_black.withAlpha(0.75));
        for (auto p : fPts) {
            draw_point(canvas, p, paint);
        }
    }

    bool hitTest(GPoint p) override {
        GPathBuilder bu;
        bu.addPolygon(fPts.data(), fPts.size());
        return hit_test(*bu.detach(), p);
    }

    bool handleSym(unsigned sym) override {
        switch (sym) {
            case 'e': fShowExtra = !fShowExtra; return true;
            case 'c': fShowColors = !fShowColors; return true;
            case 't': fShowTexs = !fShowTexs; return true;
            case 'd': fShowDiag = (fShowDiag + 1) % 3; return true;
            case SDLK_LEFT:  fLevel = std::max(0, fLevel - 1); return true;
            case SDLK_RIGHT: fLevel = std::min(32, fLevel + 1); return true;
            default: break;
        }
        return this->INHERITED::handleSym(sym);
    }

    GRect bounds() const override {
        GPoint min = fPts[0], max = fPts[0];

        std::for_each(fPts.begin(), fPts.end(), [&](GPoint p) {
            min.x = std::min(min.x, p.x);
            min.x = std::min(min.x, p.y);
            max.x = std::max(max.y, p.x);
            max.x = std::max(max.y, p.y);
        });
        return {min.x, min.y, max.x, max.y};
    }

    void onOffset(GVector v) override {
        std::transform(fPts.begin(), fPts.end(), fPts.begin(), [v](GPoint p) {
            return p + v;
        });
    }

    std::unique_ptr<GClick> onFindClick(GPoint clickp, unsigned fastkeys)  override {
        if (fInitialRectMode) {
            return std::make_unique<GClick>(clickp, [this](const GClick* c) {
                if (c->state() == GClick::State::kUp_State) {
                    fInitialRectMode = false;
                } else {
                    fPts[2] = c->curr();
                    fPts[1] = {fPts[2].x, fPts[0].y};
                    fPts[3] = {fPts[0].x, fPts[2].y};
                }
            });
        }

        for (auto& p : fPts) {
            if (hit_test(clickp, p)) {
                return std::make_unique<GClick>(clickp, [ptr = &p](const GClick* c) {
                    *ptr = c->curr();
                });
            }
        }
        return nullptr;
    }
};

REGISTER_TOOLBAR(6.0, [](Toolbar* bar) {
    bar->addTool([](GCanvas* canvas, GRect r, const GPaint& paint) {
        r = r.inset(2, 2);
        GPoint pts[] = {
                lerp(r.TL(), r.TR(), 0.25),
                lerp(r.TL(), r.TR(), 0.75),
                lerp(r.TR(), r.BR(), 0.75),
                lerp(r.BL(), r.BR(), 0.25),
        };
        canvas->drawConvexPolygon(pts, std::size(pts), paint);
    }, [](GPoint p) {
        return std::make_unique<QuadShape>(p);
    });
});
