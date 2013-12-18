/*
 * A brush specifies how lines should be drawn.
 */

#ifndef brush_h
#define brush_h

#include <InterViews/resource.h>

class Brush : public Resource {
public:
    Brush(int pattern, int width = 1);
    Brush(int* pattern, int count, int width = 1);
    ~Brush();

    int Width();
private:
    friend class Painter;

    class BrushRep* rep;
    int width;
};

class BrushRep {
    friend class Brush;
    friend class Painter;

    BrushRep(int* pattern, int count, int width);
    ~BrushRep();
    void* info;
    int count;
};

inline int Brush::Width () { return width; }

extern Brush* single;

#endif
