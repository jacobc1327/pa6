#include "mesh_sample.h"

#include "include/GShader.h"

bool GSampleShaderAtLocal(const GShader* shader, const GMatrix& ctm, GPoint local, GPixel* out) {
    if (!shader || !out) {
        return false;
    }
    if (mesh_sample_mod_alpha(shader, ctm, local, out)) {
        return true;
    }
    if (mesh_sample_blend(shader, ctm, local, out)) {
        return true;
    }
    if (mesh_sample_bitmap(shader, ctm, local, out)) {
        return true;
    }
    if (mesh_sample_linear_gradient(shader, ctm, local, out)) {
        return true;
    }
    return false;
}
