/*
 * A banner is a one line title bar.
 */

#ifndef banner_h
#define banner_h

#include <InterViews/interactor.h>

class Banner : public Interactor {
public:
    char* left;
    char* middle;
    char* right;
    boolean highlight;

    Banner(char* lt, char* mid, char* rt);
    Banner(const char*, char* lt, char* mid, char* rt);
    Banner(Painter* out, char* lt, char* mid = nil, char* rt = nil);
    ~Banner();

    void Update();
protected:
    int lw, mw, rw;
    Coord lx, mx, rx;
    Painter* inverse;

    void Init(char*, char*, char*);
    void Reconfig();
    void Redraw(Coord, Coord, Coord, Coord);
    void Resize();
};

#endif
