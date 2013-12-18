/*
 * Rubber - rubberbanding graphical objects
 */

#ifndef rubband_h
#define rubband_h

#include <InterViews/defs.h>
#include <InterViews/resource.h>

enum Side { LeftSide, RightSide, BottomSide, TopSide };

class Canvas;
class Painter;

class Rubberband : public Resource {
public:
    Rubberband(Painter*, Canvas*, Coord offx, Coord offy);
    virtual ~Rubberband();

    virtual void Draw();
    virtual void Redraw();
    virtual void Erase();
    virtual void Track(Coord x, Coord y);

    virtual void SetPainter(Painter*);
    virtual void SetCanvas(Canvas*);
    Painter* GetPainter();
    Canvas* GetCanvas();
protected:
    float Angle(Coord, Coord, Coord, Coord);	// angle line makes w/horiz
    float Distance(Coord, Coord, Coord, Coord); // distance between 2 points
protected:
    Painter* output;
    Canvas* canvas;
    boolean drawn;
    Coord trackx, offx;
    Coord tracky, offy;
};

inline Painter* Rubberband::GetPainter () { return output; }
inline Canvas* Rubberband::GetCanvas () { return canvas; }

#endif
