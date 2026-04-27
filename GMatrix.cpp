/*
 *  Copyright 2025 <me>
 */

 #include "include/GMatrix.h"
 #include <cmath>
 
 GMatrix::GMatrix() {
     // Identity matrix: [1 0 0]
     //                   [0 1 0]
     fMat[0] = 1.0f;  // a
     fMat[1] = 0.0f;  // b
     fMat[2] = 0.0f;  // c
     fMat[3] = 1.0f;  // d
     fMat[4] = 0.0f;  // e
     fMat[5] = 0.0f;  // f
 }
 
 GMatrix GMatrix::Translate(float tx, float ty) {
     // Translation matrix: [1 0 tx]
     //                      [0 1 ty]
     return GMatrix(1.0f, 0.0f, tx,
                    0.0f, 1.0f, ty);
 }
 
 GMatrix GMatrix::Scale(float sx, float sy) {
     // Scaling matrix: [sx 0  0]
     //                 [0  sy 0]
     return GMatrix(sx, 0.0f, 0.0f,
                    0.0f, sy, 0.0f);
 }
 
 GMatrix GMatrix::Rotate(float radians) {
     // Rotation matrix: [cos  -sin  0]
     //                  [sin   cos  0]
     float c = cosf(radians);
     float s = sinf(radians);
     return GMatrix(c, -s, 0.0f,
                    s,  c, 0.0f);
 }
 
 GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
     // Matrix multiplication: result = a * b
     // [a0 a2 a4]   [b0 b2 b4]   [a0*b0+a2*b1  a0*b2+a2*b3  a0*b4+a2*b5+a4]
     // [a1 a3 a5] * [b1 b3 b5] = [a1*b0+a3*b1  a1*b2+a3*b3  a1*b4+a3*b5+a5]
     // [0  0  1 ]   [0  0  1 ]   [0            0            1             ]
     
     return GMatrix(
         a.fMat[0] * b.fMat[0] + a.fMat[2] * b.fMat[1],  // a
         a.fMat[0] * b.fMat[2] + a.fMat[2] * b.fMat[3],  // c
         a.fMat[0] * b.fMat[4] + a.fMat[2] * b.fMat[5] + a.fMat[4],  // e
         a.fMat[1] * b.fMat[0] + a.fMat[3] * b.fMat[1],  // b
         a.fMat[1] * b.fMat[2] + a.fMat[3] * b.fMat[3],  // d
         a.fMat[1] * b.fMat[4] + a.fMat[3] * b.fMat[5] + a.fMat[5]   // f
     );
 }
 
 std::optional<GMatrix> GMatrix::invert() const {
     // For 2x2 matrix [a c] the determinant is ad - bc
     //                [b d]
     float det = fMat[0] * fMat[3] - fMat[1] * fMat[2];
     
     // If determinant is zero, matrix is not invertible
     if (fabsf(det) < 1e-10f) {
         return {};
     }
     
     float invDet = 1.0f / det;
     
     // Inverse of [a c e] is [d  -c  ce-df] / det
     //            [b d f]     [-b  a  bf-ae]
     //            [0 0 1]     [0   0  1    ]
     
     float a = fMat[0], b = fMat[1], c = fMat[2], d = fMat[3], e = fMat[4], f = fMat[5];
     
     return GMatrix(
         d * invDet,           // a'
         -c * invDet,          // c'
         (c * f - d * e) * invDet,  // e'
         -b * invDet,          // b'
         a * invDet,           // d'
         (b * e - a * f) * invDet   // f'
     );
 }
 
 void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
     // Transform: [x']   [a c e] [x]
     //            [y'] = [b d f] [y]
     //            [1 ]   [0 0 1] [1]
     //
     // x' = a*x + c*y + e
     // y' = b*x + d*y + f
     
     for (int i = 0; i < count; ++i) {
         float x = src[i].x;
         float y = src[i].y;
         dst[i].x = fMat[0] * x + fMat[2] * y + fMat[4];
         dst[i].y = fMat[1] * x + fMat[3] * y + fMat[5];
     }
 }