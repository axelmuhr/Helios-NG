/*
 * Glue is useful for variable spacing between interactors.
 */

#ifndef glue_h
#define glue_h

#include <InterViews/interactor.h>
#include <InterViews/shape.h>

class Glue : public Interactor {
protected:
    Glue();
    Glue(const char*);
    Glue(Painter* bg);

    void Redraw(Coord, Coord, Coord, Coord);
private:
    void Init();
};

class HGlue : public Glue {
public:
    HGlue(int natural = 0, int stretch = hfil);
    HGlue(const char*, int natural = 0, int stretch = hfil);
    HGlue(int natural, int shrink, int stretch);
    HGlue(const char*, int natural, int shrink, int stretch);
    HGlue(Painter* bg, int natural = 0, int stretch = hfil);
    HGlue(Painter* bg, int natural, int shrink, int stretch);
private:
    void Init(int nat, int shrink, int stretch);
};

class VGlue : public Glue {
public:
    VGlue(int natural = 0, int stretch = vfil);
    VGlue(const char*, int natural = 0, int stretch = vfil);
    VGlue(int natural, int shrink, int stretch);
    VGlue(const char*, int natural, int shrink, int stretch);
    VGlue(Painter* bg, int natural = 0, int stretch = vfil);
    VGlue(Painter* bg, int natural, int shrink, int stretch);
private:
    void Init(int nat, int shrink, int stretch);
};

#endif
