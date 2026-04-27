/*
 *  Copyright 2015 Mike Reed
 */

#ifndef GCanvas_DEFINED
#define GCanvas_DEFINED

#include "GMatrix.h"
#include "GPaint.h"
#include <string>

class GBitmap;
class GPath;
class GPoint;
class GRect;

class GCanvas {
public:
    virtual ~GCanvas() {}

    /**
     *  Save off a copy of the canvas state (CTM), to be later used if the balancing call to
     *  restore() is made. Calls to save/restore can be nested:
     *  save();
     *      save();
     *          concat(...);    // this modifies the CTM
     *          .. draw         // these are drawn with the modified CTM
     *      restore();          // now the CTM is as it was when the 2nd save() call was made
     *      ..
     *  restore();              // now the CTM is as it was when the 1st save() call was made
     */
    virtual void save() = 0;

    /**
     *  Copy the canvas state (CTM) that was record in the correspnding call to save() back into
     *  the canvas. It is an error to call restore() if there has been no previous call to save().
     */
    virtual void restore() = 0;

    /**
     *  Modifies the CTM by preconcatenating the specified matrix with the CTM. The canvas
     *  is constructed with an identity CTM.
     *
     *  CTM' = CTM * matrix
     */
    virtual void concat(const GMatrix& matrix) = 0;

    /**
     *  Fill the entire canvas with the specified color, using kSrc porter-duff mode.
     */
    virtual void clear(const GColor&) = 0;

    /**
     *  Draw a hairline from one point to the other, using the specified color and blendmode.
     *  If the paint's lineWidth is < 0, draw a hairline (ala PA1), else construct a stroked line,
     *  and draw it as a polygon.
     */
    virtual void drawLine(GPoint, GPoint, const GPaint&) = 0;

    /**
     *  Fill the rectangle with the color, using the specified blendmode.
     *
     *  The affected pixels are those whose centers are "contained" inside the rectangle:
     *      e.g. contained == center > min_edge && center <= max_edge
     */
    virtual void drawRect(const GRect&, const GPaint&) = 0;

    /**
     *  Fill the convex polygon with the color and blendmode,
     *  following the same "containment" rule as rectangles.
     */
    virtual void drawConvexPolygon(const GPoint[], int count, const GPaint&) = 0;

    /**
     *  Fill the path with the paint, interpreting the path using winding-fill (non-zero winding).
    */
    virtual void drawPath(const GPath&, const GPaint&) = 0;

    // Helpers

    void translate(float x, float y) {
        this->concat(GMatrix::Translate(x, y));
    }

    void translate(GVector v) {
        this->translate(v.x, v.y);
    }

    void scale(float x, float y) {
        this->concat(GMatrix::Scale(x, y));
    }

    void rotate(float radians) {
        this->concat(GMatrix::Rotate(radians));
    }


    /**
     *  Draw a mesh of triangles, with optional colors and/or texture-coordinates at each vertex.
     *
     *  The triangles are specified by successive triples of indices.
     *      int n = 0;
     *      for (i = 0; i < count; ++i) {
     *          point0 = vertx[indices[n+0]]
     *          point1 = verts[indices[n+1]]
     *          point2 = verts[indices[n+2]]
     *          ...
     *          n += 3
     *      }
     *
     *  If colors is not null, then each vertex has an associated color, to be interpolated
     *  across the triangle. The colors are referenced in the same way as the verts.
     *          color0 = colors[indices[n+0]]
     *          color1 = colors[indices[n+1]]
     *          color2 = colors[indices[n+2]]
     *
     *  If texs is not null, then each vertex has an associated texture coordinate, to be used
     *  to specify a coordinate in the paint's shader's space. If there is no shader on the
     *  paint, then texs[] should be ignored. It is referenced in the same way as verts and colors.
     *          texs0 = texs[indices[n+0]]
     *          texs1 = texs[indices[n+1]]
     *          texs2 = texs[indices[n+2]]
     *
     *  If both colors and texs[] are specified, then at each pixel their values are multiplied
     *  together, component by component.
     */
    virtual void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[],
                          int count, const int indices[], const GPaint&) = 0;

    /**
     *  Draw the quad, with optional color and/or texture coordinate at each corner. Tesselate
     *  the quad based on "level":
     *      level == 0 --> 1 quad  -->  2 triangles
     *      level == 1 --> 4 quads -->  8 triangles
     *      level == 2 --> 9 quads --> 18 triangles
     *      ...
     *  The 4 corners of the quad are specified in this order:
     *      top-left --> top-right --> bottom-right --> bottom-left
     *  Each quad is triangulated on the diagonal top-right --> bottom-left
     *      0---1
     *      |  /|
     *      | / |
     *      |/  |
     *      3---2
     *
     *  colors and/or texs can be null. The resulting triangles should be passed to drawMesh(...).
     *
     *  The order to draw the triangles is as follows: (e.g. levels == 1)
     *   |0 /|2 /|
     *   | / | / |
     *   |/ 1|/ 3|
     *   ---------
     *   |4 /|6 /|
     *   | / | / |
     *   |/ 5|/ 7|
     */
    virtual void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4],
                          int level, const GPaint&) = 0;

    // Compatibility helper for PA1 callers (before we had GPaint)

    void fillRect(const GRect& rect, const GColor& color) {
        this->drawRect(rect, GPaint(color));
    }

    void hairLine(GPoint a, GPoint b, const GColor& color) {
        GPaint paint(color);
        paint.setHairline();
        this->drawLine(a, b, paint);
    }

    void drawPath(const std::shared_ptr<GPath>& path, const GPaint& paint) {
        if (path) {
            this->drawPath(*path, paint);
        }
    }

    void drawOval(const GRect& oval, const GPaint&);
    void drawCircle(GPoint center, float radius, const GPaint&);
};

/**
 *  Implemnt this, returning an instance of your subclass of GCanvas.
 */
std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& bitmap);

/**
 *  Implement this, drawing into the provided canvas, and returning the title of your artwork.
 */
std::string GDrawSomething(GCanvas* canvas, GISize dim);

#endif
