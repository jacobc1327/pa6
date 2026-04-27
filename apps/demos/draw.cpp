/**
 *  Copyright 2015 Mike Reed
 */

#include "draw.h"
#include "draw_tools.h"

extern float DrawStr(GCanvas*, GPoint origin, const std::string&, float size, const GColor&);

template <typename T> int find_index(const std::vector<std::unique_ptr<T>>& list, T* target) {
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].get() == target) {
            return i;
        }
    }
    return -1;
}

static GRandom gRand;

static GColor rand_color() {
    return {gRand.nextF(), gRand.nextF(), gRand.nextF(), 0.75f};
}

static GColor constrain_color(GColor c) {
    return {
        std::clamp<float>(c.r, 0, 1),
        std::clamp<float>(c.g, 0, 1),
        std::clamp<float>(c.b, 0, 1),
        std::clamp<float>(c.a, 0, 1),
    };
}

static std::optional<GBitmap> cons_up_bm(unsigned index) {
    index -= 1;
    const char* gBitmapNames[] = {
            "apps/spock.png", "apps/wheel.png",
    };
    if (index < std::size(gBitmapNames)) {
        return GBitmap::ReadFromFile(gBitmapNames[index]);
    }
    return {};
}

bool Shape::handleSym(unsigned sym) {
    if (sym == 't') {
        this->toggleTiling();
        return true;
    }
    unsigned index = sym - '1';
    if (index == 0) {
        this->clearShader();
        auto alpha = this->color().a;
        this->setColor(rand_color().withAlpha(alpha));
        return true;
    } else if (auto bm = cons_up_bm(index)) {
        this->setBitmap(*bm);
        return true;
    } else if (index == 3) {
        const size_t n = 2 + gRand.nextU() % 4;
        std::vector<GColor> colors(n);
        for (auto& c : colors) {
            c = rand_color();
        }
        this->setGradient(colors.data(), colors.size());
        return true;
    }
    return false;
}

REGISTER_TOOLBAR(1.0, [](Toolbar* bar) {
    bar->addTool([](GCanvas* canvas, GRect r, const GPaint& paint) {
        canvas->drawRect(r.inset(3, 3), paint);
    }, [](GPoint p) {
        return std::make_unique<RectShape>(p);
    });
});

class LineShape : public Shape {
    using INHERITED = Shape;
    bool fShowExtras = false;
public:
    LineShape(GPoint p) {
        fPaint.setLineWidth(kDefLineWidth);
        fPts[0] = fPts[1] = p;
    }

    void setGradient(const GColor colors[], size_t n) override {
        fShader = Shader::LinearGradient(fPts[0], fPts[1], colors, n);
    }

    GRect bounds() const override { return make_from_pts(fPts[0], fPts[1]); }

    void onDraw(GCanvas* canvas) override {
        canvas->drawLine(fPts[0], fPts[1], fPaint);
    }

    void onDrawHilite(GCanvas* canvas) override {
        GPaint paint;
        paint.setHairline();
        canvas->drawLine(fPts[0], fPts[1], paint);
        paint.setLineWidth(6);
        for (auto p : fPts) {
            draw_point(canvas, p, paint);
        }

        if (fShowExtras) {
            const auto v = rotate_cw(normalize(fPts[1] - fPts[0])) * fPaint.lineWidth() * 0.5f;
            constexpr int n = 10;
            for (int i = 0; i <= 10; ++i) {
                const float t = i * 1.0f / n;
                auto p = lerp(fPts[0], fPts[1], t);
                canvas->hairLine(p + v, p - v, gLineNormalsColor);
            }
        }
    }

    bool hitTest(GPoint p) override {
        const float rad = std::max(5.0f, fPaint.lineWidth()/2);
        if (fPaint.capType() == GCapType::kRound) {
            // check round caps first
            auto inside = [rad](GPoint center, GPoint p) {
                return (p - center).length() <= rad;
            };
            if (inside(fPts[0], p) || inside(fPts[1], p)) {
                return true;
            }
        }
        auto base = fPts[1] - fPts[0];
        auto hypt = p - fPts[0];
        auto len = base.length();
        auto dist = std::abs(cross(p - fPts[0], base)/len);
        auto proj = dot(base, hypt) / len;
        if (fPaint.capType() == GCapType::kSquare) {
            len += 2 * rad;
            proj += rad;
        }
        return dist <= rad && proj >= 0 && proj <= len;
    }

    void onOffset(GVector v) override {
        for (auto& p : fPts) {
            p += v;
        }
    }

    std::unique_ptr<GClick> onFindClick(GPoint loc, unsigned fastkeys) override {
        // important to search in reverse order, for initial creation-dragging
        for (size_t i = fPts.size(); i --> 0;) {
            if (hit_test(fPts[i], loc)) {
                return std::make_unique<GClick>(loc, [ptr = &fPts[i]](const GClick* click) {
                    *ptr = click->curr();
                });
            }
        }
        return nullptr;
    }

    bool handleSym(unsigned sym) override {
        float w = fPaint.lineWidth();
        switch (sym) {
            case 'c': {
                auto ct = static_cast<unsigned>(fPaint.capType());
                fPaint.setCapType((GCapType) ((ct + 1) % 3));
                return true;
            }
            case 'e': fShowExtras = !fShowExtras; return true;
            case SDLK_LEFT:
                fPaint.setLineWidth(std::max(1.0f, w - 2));
                return true;
            case SDLK_RIGHT:
                fPaint.setLineWidth(w + 2);
                return true;
        }
        return this->INHERITED::handleSym(sym);
    }

private:
    std::array<GPoint, 2> fPts;
};
REGISTER_TOOLBAR(1.1, [](Toolbar* bar) {
    bar->addTool([](GCanvas* canvas, GRect r, const GPaint& paint) {
        auto p = paint;
        p.setLineWidth(10);
        r = r.inset(6, 6);
        canvas->drawLine(r.TL(), r.BR(), p);
    }, [](GPoint p) {
        return std::make_unique<LineShape>(p);
    });
});

std::string gLabel;

void draw_set_label(const char label[], size_t n) {
    gLabel.replace(0, gLabel.size(), label, n);
}

static void draw_label(GCanvas* canvas, const GRect& bounds) {
    if (gLabel.size()) {
        const float size = 18;
        const float widthScale = 4.0f / 6;
        const float width = gLabel.size() * widthScale * size;
        GPoint origin = {bounds.center().x - width / 2, bounds.bottom - 10};
        DrawStr(canvas, origin, gLabel, size, GColor_black);
    }
}

template ToolbarRegistrant* ToolbarRegistrant::gHead;

class TestWindow : public GWindow {
    Toolbar                             fToolBar;
    std::vector<std::unique_ptr<Shape>> fList;
    Shape* fShape;      // borrowed pointer
    GColor fBGColor;
    int fScaleLevel = 0;

public:
    TestWindow(GISize sz) : GWindow(sz) {
        fBGColor = {1, 1, 1, 1};
        fShape = NULL;

        fToolBar.m_topLeft = {10, 10};

        // gather up factories and sort them by their order (PA#)
        std::vector<ToolbarFactory*> facts;
        auto reg = ToolbarRegistrant::Head();
        while (reg) {
            facts.push_back(&reg->value());
            reg = reg->next();
        }
        std::sort(facts.begin(), facts.end(), [](const auto* a, const auto* b) {
            return a->m_order < b->m_order;
        });
        for (const auto* f : facts) {
            f->m_factory(&fToolBar);
        }
    }

    virtual ~TestWindow() {}

    void setWindowZoom() {
        float scale = std::powf(1.25f, fScaleLevel);
        this->setCtm(GMatrix::Scale(scale, scale));
    }

    void setSelectedShape(Shape* newShape) {
        if (fShape != newShape) {
            if (fShape) {
                fShape->setSelected(false);
            }
            fShape = newShape;
            if (fShape) {
                fShape->setSelected(true);
            }
            this->requestDraw();
            this->updateTitle();
        }
    }
protected:
    void onDraw(GCanvas* canvas) override {
        canvas->fillRect(GRect::XYWH(0, 0, 10000, 10000), fBGColor);

        draw_label(canvas, this->bounds());

        if (fShape) {
            fShape->drawHiliteBefore(canvas);
        }
        for (auto& shape : fList) {
            shape->draw(canvas);
        }
        if (fShape) {
            fShape->drawHilite(canvas);
        }

        fToolBar.draw(canvas);
    }

    bool onKeyPress(uint32_t sym) override {
        if (fShape) {
            if (fShape->handleSym(sym)) {
                this->updateTitle();
                this->requestDraw();
                return true;
            }
            switch (sym) {
                case SDLK_UP: [[fallthrough]];
                case SDLK_DOWN:
                    if (this->pollFastKeys() & GFastKeys::opt_key) {
                        int index = find_index(fList, fShape);
                        int oindex = (sym == SDLK_UP) ? index + 1 : index - 1;
                        oindex = std::clamp<int>(oindex, 0, fList.size() - 1);
                        if (index != oindex) {
                            std::swap(fList[index], fList[oindex]);
                            this->requestDraw();
                            return true;
                        }
                    } else {
                        fScaleLevel += (sym == SDLK_UP) ? +1 : -1;
                        this->setWindowZoom();
                        return true;
                    }
                    return false;
                case SDLK_DELETE:
                case SDLK_BACKSPACE:
                    if (fShape) {
                        this->removeShape(fShape);
                        fShape = nullptr;
                        this->updateTitle();
                        this->requestDraw();
                    }
                    this->setSelectedShape(nullptr);
                    return true;
                case SDLK_ESCAPE:
                    if (fShape) {
                        this->setSelectedShape(nullptr);
                    }
                    break;
                default:
                    break;
            }
        }

        GColor c = fShape ? fShape->color() : fBGColor;
        const float delta = 0.1f;
        switch (sym) {
            case 'a': c.a -= delta; break;
            case 'A': c.a += delta; break;
            case 'r': c.r -= delta; break;
            case 'R': c.r += delta; break;
            case 'g': c.g -= delta; break;
            case 'G': c.g += delta; break;
            case 'b': c.b -= delta; break;
            case 'B': c.b += delta; break;
            default:
                return false;
        }
        c = constrain_color(c);
        if (fShape) {
            fShape->setColor(c);
        } else {
            c.a = 1;   // need the bg to stay opaque
            fBGColor = c;
        }
        this->updateTitle();
        this->requestDraw();
        return true;
    }

    std::unique_ptr<GClick> onFindClickHandler(GPoint loc) override {
        const uint32_t fastkeys = this->pollFastKeys();

        if (auto c = fToolBar.findClick(loc)) {
            this->requestDraw();
            return c;
        }

        if (fShape) {
            if (auto c = fShape->findClick(loc, fastkeys)) {
                return c;
            }
        }

        for (int i = fList.size() - 1; i >= 0; --i) {
            if (fList[i]->hitTest(loc)) {
                this->setSelectedShape(fList[i].get());
                return std::make_unique<GClick>(loc, [this](const GClick* click) {
                    fShape->offset(click->curr() - click->prev());
                    this->updateTitle();
                    this->requestDraw();
                });
            }
        }
        
        // else create a new shape
        fList.push_back(fToolBar.makeShape(loc));
        this->setSelectedShape(fList.back().get());
        fShape->setColor(rand_color());
        return fShape->findClick(loc, fastkeys);
    }

private:
    void removeShape(Shape* target) {
        assert(target);
        target->setSelected(false);
        int index = find_index(fList, target);
        if (index >= 0) {
            fList.erase(fList.begin() + index);
        } else {
            assert(!"shape not found?");
        }
    }

    void updateTitle() {
        char buffer[100];
        buffer[0] = ' ';
        buffer[1] = 0;

        GColor c = fBGColor;
        if (fShape) {
            c = fShape->color();
        }

        snprintf(buffer, sizeof(buffer), "R:%02X  G:%02X  B:%02X  A:%02X",
                int(c.r * 255), int(c.g * 255), int(c.b * 255), int(c.a * 255));
        this->setTitle(buffer);
    }

    typedef GWindow INHERITED;
};

int main(int argc, char const* const* argv) {
    GWindow* wind = new TestWindow({640, 480});

    return wind->run();
}
