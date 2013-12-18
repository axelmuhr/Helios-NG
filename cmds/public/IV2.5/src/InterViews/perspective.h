/*
 * A perspective describes what portion of the total view
 * an interactor shows.
 */

#ifndef perspective_h
#define perspective_h

#include <InterViews/resource.h>

class Perspective : public Resource {
public:
    Coord x0, y0;		/* origin of view */
    int width, height;		/* total size of view */
    Coord curx, cury;		/* current position */
    int curwidth, curheight;	/* current size */
    int sx, sy, lx, ly;		/* small and large scrolling increments */

    Perspective();
    Perspective(Perspective&);
    ~Perspective();

    void Init(Coord ix0, Coord iy0, int iwidth, int iheight);
    void Attach(class Interactor*);
    void Detach(Interactor*);
    void Update();

    boolean operator ==(Perspective&);
    boolean operator !=(Perspective&);
    Perspective& operator =(Perspective&);
private:
    class ViewList* views;	/* interactors that access the perspective */
};

#endif
