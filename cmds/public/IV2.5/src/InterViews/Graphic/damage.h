/*
 * Damage - maintains an array of non-overlapping rectangles representing
 * damaged areas of of a canvas, used for smart redraw.
 */

#ifndef damage_h
#define damage_h

#include <InterViews/defs.h>

class BoxList;
class BoxObj;
class Canvas;
class Graphic;
class Painter;
class RefList;

class Damage {
public:
    Damage(Canvas* = nil, Painter* = nil, Graphic* = nil);
    ~Damage();
    
    boolean Incurred();
    void Added(Graphic*);
    void Incur(Graphic*);
    void Incur(BoxObj&);
    void Incur(Coord, Coord, Coord, Coord);
    void Repair();
    void Reset();

    void SetCanvas(Canvas*);
    void SetPainter(Painter*);
    void SetGraphic(Graphic*);

    Canvas* GetCanvas();
    Painter* GetPainter();
    Graphic* GetGraphic();
protected:    
    int Area(BoxObj&);
    void DrawAreas();
    void DrawAdditions();
    void Merge(BoxObj&);
protected:
    BoxList* areas;
    RefList* additions;
    Canvas* canvas;
    Painter* output;
    Graphic* graphic;
};

#endif
