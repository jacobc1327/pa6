/*
 *  Copyright 2025 <me>
 */

 #ifndef _g_starter_canvas_h_
 #define _g_starter_canvas_h_
 
 #include "include/GCanvas.h"
 #include "include/GPoint.h"
 #include "include/GColor.h"
 #include "include/GBitmap.h"
 #include "include/GRect.h"
 #include "include/GPaint.h"
 #include "include/GBlendMode.h"
#include "include/GMatrix.h"
#include "include/GPath.h"
#include <vector>

class MyCanvas : public GCanvas {
public:
    MyCanvas(const GBitmap& device) : fDevice(device), fCTM() {
        // Tests expect a new device to start cleared.
        if (auto pixels = fDevice.pixels()) {
            const int w = fDevice.width();
            const int h = fDevice.height();
            for (int y = 0; y < h; ++y) {
                GPixel* row = fDevice.getAddr(0, y);
                for (int x = 0; x < w; ++x) {
                    row[x] = 0;
                }                                           
            }
        }
    }

    void save() override;
    void restore() override;
    void concat(const GMatrix&) override;
    void clear(const GColor&) override;
    void drawLine(GPoint, GPoint, const GPaint&) override;
    void drawRect(const GRect&, const GPaint&) override;
    void drawConvexPolygon(const GPoint[], int count, const GPaint&) override;
    void drawPath(const GPath&, const GPaint&) override;
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count,
                  const int indices[], const GPaint&) override;
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level,
                  const GPaint&) override;

 private:
     // Note: we store a copy of the bitmap
     const GBitmap fDevice;
     
     // Current transformation matrix (CTM)
     GMatrix fCTM;
     
     // Stack for save/restore
     std::vector<GMatrix> fCTMStack;
 
    // Private helper methods (not overriding base class)
    void hairLine(GPoint, GPoint, const GColor&);
    void drawHairLineWithShader(GPoint, GPoint, const GPaint&);
    void fillRect(const GRect&, const GColor&);
    void drawDeviceSpacePolygon(const GPoint[], int count, const GPaint&);
 };

    // Make the public helper methods (for filling up the rectangle)
 
 #endif