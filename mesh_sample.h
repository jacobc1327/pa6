#pragma once

class GShader;

#include "include/GMatrix.h"
#include "include/GPoint.h"
#include "include/GPixel.h"

bool mesh_sample_mod_alpha(const GShader* shader, const GMatrix& ctm, GPoint local, GPixel* out);
bool mesh_sample_blend(const GShader* shader, const GMatrix& ctm, GPoint local, GPixel* out);
bool mesh_sample_bitmap(const GShader* shader, const GMatrix& ctm, GPoint local, GPixel* out);
bool mesh_sample_linear_gradient(const GShader* shader, const GMatrix& ctm, GPoint local, GPixel* out);

bool GSampleShaderAtLocal(const GShader* shader, const GMatrix& ctm, GPoint local, GPixel* out);
