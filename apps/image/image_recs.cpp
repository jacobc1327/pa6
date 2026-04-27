/**
 *  Copyright 2015 Mike Reed
 */

#include "image.h"

#include "image_pa1.inc"
#include "image_pa2.inc"
#include "image_pa3.inc"
#include "image_pa4.inc"
#include "image_pa5.inc"
#include "image_pa6.inc"

const GDrawRec gDrawRecs[] = {
    { draw_wheel,           256, 256,  "wheel",          1 },
    { draw_wheels_clipped,  256, 256,  "wheels_clipped", 1 },
    { draw_solid_ramp,      256, 7*28, "solid_ramp",     1 },
    { draw_graphs,          256, 256,  "rect_graphs",    1 },
    { draw_blend_black,     200, 200,  "blend_black",    1 },

    { draw_poly,        512, 512,   "poly",         2 },
    { draw_poly_center, 256, 256,   "poly_center",  2 },
    { rect_blendmodes,  450, 340,   "rect_blendmodes",   2 },
    { line_blendmodes,  450, 340,   "line_blendmodes",   2 },
    { fat_lines,        256, 256,   "fat_lines",   2 },

    { draw_checker,     300, 300,   "checkers",     3 },
    { draw_poly_rotate, 300, 300,   "color_clock",  3 },
    { draw_bitmaps_hole,300, 300,   "bitmap_hole",  3 },
    { draw_clock_bm,    480, 480,   "spock_clock",  3 },
    { draw_bm_blendmodes,  450, 340,"blendmodes2",   3 },
    { draw_hair_spock,  391, 353,   "hair_spock",   3 },

    { stars,            512, 512,   "stars",        4 },
    { draw_lion,        512, 512,   "lion",         4 },
    { draw_lion_head,   512, 512,   "lion_head",    4 },
    { draw_grad,        250, 200,   "grad",         4 },
    { draw_gradient_blendmodes, 450, 340, "gradient_blendmodes", 4 },
    { draw_graphs2,     256, 256,   "path_graphs",  4  },

    { draw_rings,       512, 512,   "rings", 5 },
    { draw_bm_tiling,   512, 512,   "bitmap_tiling", 5 },
    { draw_cartman,     512, 512,   "cartman", 5 },
    { draw_mirror_ramp, 512, 512,   "mirror_ramp", 5 },
    { draw_alpha_mod,   512, 512,   "alpha_mod", 5 },
    { image_curves,     256, 256,   "curves", 5 },
    { image_linecaps,   512, 512,   "linecaps", 5 },

    { draw_tri,         512, 512,   "tri_color",   6 },
    { draw_tri2,        512, 512,   "tri_texture", 6 },
    { mesh_1,           512, 512,   "sweep_mesh",  6 },
    { mesh_3,           512, 512,   "both_mesh",   6 },
    { spock_quad,       512, 512,   "spock_quad",  6 },
    { color_quad,       512, 512,   "color_quad",  6 },
    { draw_blendmodes_via_shader, 512, 512, "shader_blend_modes", 6},

    { nullptr, 0, 0, nullptr },
};
