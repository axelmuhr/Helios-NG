/*
 * X11-dependent painter interface.
 */

#ifndef painterrep_h
#define painterrep_h

#include <InterViews/X11/Xlib.h>

class PainterRep {
public:
    PainterRep();
    ~PainterRep();

    void PrepareFill(void* info);
    void PrepareDash(int width, void* info, int count);

    GC fillgc;
    GC dashgc;
    boolean fillbg;
    boolean overwrite;
    boolean xor;
};

#endif
