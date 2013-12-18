/*
 * View of a list of squares.
 */

#ifndef view_h
#define view_h

#include <InterViews/interactor.h>

class Squares;

class SquaresView : public Interactor {
public:
    SquaresView(Squares*);
    ~SquaresView();

    virtual void Update();
protected:
    virtual void Redraw(Coord, Coord, Coord, Coord);
private:
    friend class SquaresFrame;

    Squares* subject;
};

#endif
