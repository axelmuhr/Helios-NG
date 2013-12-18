// $Header: drawingview.h,v 1.8 89/04/17 00:30:25 linton Exp $
// declares class DrawingView.

#ifndef drawingview_h
#define drawingview_h

#include <InterViews/Graphic/grblock.h>

// Declare imported types.

class Damage;
class Grid;
class Rubberband;
class Selection;
class SelectionList;
class State;

// A DrawingView displays the user's drawing.

class DrawingView : public GraphicBlock {
public:

    DrawingView(Graphic*);
    ~DrawingView();

    void SetDrawHandler(Interactor*);
    void SetEventHandler(Interactor*);
    void SetGrid(Grid*);
    void SetPageBoundary(Graphic*);
    void SetSelectionList(SelectionList*);
    void SetState(State*);

    void Draw();
    void Handle(Event&);
    void Update();

    void Manipulate(
	Event&, Rubberband*, int, boolean constrain=true, boolean erase=true
    );

    void DrawHandles();
    void RedrawHandles();
    void EraseHandles();
    void EraseExcessHandles(SelectionList*);
    void ErasePickedHandles(Selection*);
    void ErasePickedHandles(SelectionList*);
    void EraseUngraspedHandles(Selection*);

    void Added();
    void Damaged();
    void Repair();

    void Magnify(Coord, Coord, Coord, Coord);
    void Reduce();
    void Enlarge();
    void NormalSize();
    void ReduceToFit();
    void CenterPage();

protected:

    void Reconfig();
    void Reconfigure(Graphic*);
    void Redraw(Coord, Coord, Coord, Coord);
    void Resize();
    float LimitMagnification(float);

    Damage* damage;		// keeps track of damaged areas of drawing
    Interactor* drawhandler;	// updates Selections' handles for us
    Interactor* eventhandler;	// handles events for us
    Grid* grid;			// constrains points to lie on a grid it draws
    Graphic* pageboundary;	// draws a boundary around the page
    Painter* rasterxor;		// stores painter with which to draw handles
    SelectionList* sl;		// lists current Selections
    State* state;		// stores Graphic and nonGraphic attributes

};

#endif
