/*
 * A canvas is an area of graphics output.
 */

#ifndef canvas_h
#define canvas_h

#include <InterViews/defs.h>

class Color;

enum CanvasStatus {
    CanvasMapped, CanvasUnmapped, CanvasOffscreen
};

class Canvas {
public:
    Canvas(int w, int h);
    Canvas(void*);
    ~Canvas();

    void* Id();
    int Width();
    int Height();
    CanvasStatus Status();

    void SetBackground(Color*);

    void Clip(Coord, Coord, Coord, Coord);
    void NoClip();
    void ClipOn();
    void ClipOff();
    boolean IsClipped();
    void Map(Coord&, Coord&);
private:
    friend class Interactor;
    friend class Scene;
    friend class World;
    friend class WorldView;
    friend class Painter;

    void* id;
    CanvasStatus status;
    int width, height;
    class CanvasRep* rep;

    void WaitForCopy();
};

inline void* Canvas::Id () { return id; }
inline int Canvas::Width () { return width; }
inline int Canvas::Height () { return height; }
inline CanvasStatus Canvas::Status () { return status; }

#endif
