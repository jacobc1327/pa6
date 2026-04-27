/**
 *  Copyright 2018 Mike Reed
 */

static void draw_tri(GCanvas* canvas) {
    const GPoint pts[] = {
        { 10, 10 }, { 400, 100 }, { 250, 400 },
    };
    const GColor clr[] = {
        { 1, 0, 0, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 },
    };
    const int indices[] = {
        0, 1, 2,
    };

    canvas->translate(250, 250);
    canvas->rotate(gFloatPI/2);
    canvas->translate(-250, -250);
    canvas->drawMesh(pts, clr, nullptr, 1, indices, GPaint());
}

static void draw_tri2(GCanvas* canvas) {
    auto bm = GBitmap::ReadFromFile("apps/spock.png");
    if (!bm) {
        return;
    }
    float w = bm->width();
    float h = bm->height();

    const GPoint pts[] = {
        { 10, 10 }, { 400, 100 }, { 250, 400 },
    };
    const GPoint tex[] = {
        { 0, 0 }, { w, 0 }, { 0, h },
    };
    const int indices[] = {
        0, 1, 2,
    };

    GPaint paint(GShader::Bitmap(*bm, GMatrix()));

    canvas->translate(250, 250);
    canvas->rotate(gFloatPI/2);
    canvas->translate(-250, -250);
    canvas->drawMesh(pts, nullptr, tex, 1, indices, paint);
}

const float twopi = (float)(2*gFloatPI);

static void circular_mesh(GCanvas* canvas, bool showColors, bool showTex) {
    auto shader = GShader::LinearGradient({0, 0}, {1,1}, {0.5,0.25f,0.5,1}, {1,1,1,1});

    const int TRIS = 31;
    GPoint pts[TRIS + 1];
    GColor colors[TRIS + 1];
    GPoint tex[TRIS + 1];
    int indices[TRIS * 3];

    const float radius = 250;
    const float center = 256;
    pts[0] = { center, center };
    colors[0] = { 0.5f, 0, 0.75f, 1 };
    colors[0] = {1,1,1,1};
    tex[0] = { 0, 0 };

    auto wave = [](float radians) {
        return (std::sin(radians) + 1) * 0.5f;
    };
    float angle = 0;
    float da = twopi / (TRIS - 1);
    int* idx = indices;
    for (int i = 1; i <= TRIS; ++i) {
        float x = cos(angle);
        float y = sin(angle);
        const float rad = radius - float(i&1) * 50;
        pts[i] = { x * rad + center, y * rad + center };
        colors[i] = { wave(angle*2), wave(angle*2.5f), wave(angle*3), 1 };
        tex[i] = { angle / twopi, 1 };
        idx[0] = 0; idx[1] = i; idx[2] = i < TRIS ? i + 1 : 1;
        idx += 3;
        angle += da;
    }

    const GColor* colorPtr = showColors ? colors : nullptr;
    const GPoint* texPtr = showTex ? tex : nullptr;
    canvas->drawMesh(pts, colorPtr, texPtr, TRIS, indices, GPaint(shader));
}

static void mesh_1(GCanvas* canvas) { circular_mesh(canvas, true, false); }
static void mesh_3(GCanvas* canvas) { circular_mesh(canvas, true, true); }

static void draw_quad(GCanvas* canvas, const GColor colors[], const GPoint texs[],
                      std::shared_ptr<GShader> shader) {
    GPoint verts[4] = {
        {50, 120}, {470.140625f, 86.2226562f},
        {137.21875f, 332.925781f}, {458.402344f, 450.890625},
    };
    GPaint paint(std::move(shader));
    canvas->drawQuad(verts, colors, texs, 12, paint);
}

static void spock_quad(GCanvas* canvas) {
    auto bitmap = GBitmap::ReadFromFile("apps/spock.png");
    if (!bitmap) {
        return;
    }
    const float w = bitmap->width();
    const float h = bitmap->height();
    auto shader = GShader::Bitmap(*bitmap, GMatrix(), GTileMode::kMirror);
    const GPoint texs[4] = {
            {w, 0}, {2*w, 0}, {2*w, h}, {w, h},
    };
    draw_quad(canvas, nullptr, texs, std::move(shader));
}

static void color_quad(GCanvas* canvas) {
    const GColor colors[] = {
            {1, 0, 0, 1},
            {0, 1, 0, 1},
            {0, 0, 1, 1},
            {1, 1, 0, 1},
    };
    draw_quad(canvas, colors, nullptr, nullptr);
}

static void draw_roundy(GCanvas* canvas, const GRect& r, const GPaint& p) {
    const GPoint c = r.center();
    GPathBuilder bu;
    bu.moveTo(c.x, r.top);
    bu.cubicTo(r.TR(), r.TR(), {r.right, c.y});
    bu.cubicTo(r.BR(), r.BR(), {c.x, r.bottom});
    bu.cubicTo(r.BL(), r.BL(), {r.left, c.y});
    bu.cubicTo(r.TL(), r.TL(), {c.x, r.top});
    canvas->drawPath(bu.detach(), p);
}

static void draw_blendmodes_via_shader(GCanvas* canvas) {
    const GRect bounds = {0, 0, 512, 512};
    const float gray = 36.0f/255;
    canvas->fillRect(bounds, {gray, gray, gray, 1});

    auto bm = GBitmap::ReadFromFile("apps/spock.png");
    if (!bm) {
        return;
    }
    auto bmsh = GShader::Bitmap(*bm, GMatrix::Scale(bounds.width() / bm->width(),
                                                    bounds.height() / bm->height()));
    bmsh = GShader::ModAlpha(bmsh, 0.75);

    GColor colors[] = {
        GColor_red, GColor_yellow, GColor_green, GColor_cyan, GColor_blue, GColor_magenta, GColor_red,
    };
    for (auto& c : colors) {
        c.a = 0.5;
    }

    const float w = bounds.width() / 4,
                h = bounds.height() / 4;
    int modeIndex = 0;
    for (int r = 0; r < 4; ++r) {
        float y = r * h;
        for (int c = 0; c < 4; ++c) {
            float x = c * w;
            const GRect rect = {x, y, x + w, y + h};
            auto gradsh = GShader::LinearGradient(rect.TL(), rect.BR(), colors, std::size(colors));
            GPaint paint(GShader::Blend(gradsh, bmsh, static_cast<GBlendMode>(modeIndex)));
            draw_roundy(canvas, rect, paint);
            modeIndex += 1;
        }
    }
}