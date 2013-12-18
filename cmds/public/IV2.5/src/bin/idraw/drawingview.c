// $Header: drawingview.c,v 1.10 89/04/17 00:30:21 linton Exp $
// implements class DrawingView.

#include "drawingview.h"
#include "grid.h"
#include "ipaint.h"
#include "istring.h"
#include "listselectn.h"
#include "selection.h"
#include "state.h"
#include <InterViews/Graphic/damage.h>
#include <InterViews/Graphic/util.h>
#include <InterViews/event.h>
#include <InterViews/painter.h>
#include <InterViews/perspective.h>
#include <InterViews/rubband.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>

// DrawingView caches its canvas' contents if possible to speed up
// redrawing after expose events and sets its class name to establish
// a path for the resource "small".

static const int PAD = 0;	// we don't want any padding around graphic

DrawingView::DrawingView (Graphic* graphic)
: (updownEvents, graphic, PAD, Center, Binary) {
    SetCanvasType(CanvasSaveContents);
    SetClassName("DrawingView");

    damage = nil;
    drawhandler = nil;
    eventhandler = nil;
    grid = nil;
    rasterxor = nil;
    sl = nil;
    state = nil;
}

// Free storage allocated to store members.

DrawingView::~DrawingView () {
    delete damage;
    delete rasterxor;
}

// Define access functions to set members' values.  Only Idraw sets
// their values.

void DrawingView::SetDrawHandler (Interactor* i) {
    drawhandler = i;
}

void DrawingView::SetEventHandler (Interactor* i) {
    eventhandler = i;
}

void DrawingView::SetGrid (Grid* g) {
    grid = g;
}

void DrawingView::SetPageBoundary (Graphic* p) {
    pageboundary = p;
}

void DrawingView::SetSelectionList (SelectionList* slist) {
    sl = slist;
}

void DrawingView::SetState (State* s) {
    state = s;
}

// Draw draws the entire view.  Draw calls Check for its side effect
// of flushing any redraws caused by a dialog box's removal before
// drawing the view.  The drawhandler resets the Selections' handles
// in case an adjuster or slider moved the view.

void DrawingView::Draw () {
    if (graphic != nil) {
	Graphic* backup = graphic;
	graphic = nil;
	Check();
	graphic = backup;

	GraphicBlock::Draw();
	damage->Reset();
	drawhandler->Update();
	RedrawHandles();
    }
}

// Handle delegates input events to the eventhandler.

void DrawingView::Handle (Event& e) {
    eventhandler->Handle(e);
}

// Update swaps pointers to make UpdatePerspective calculate the
// page's size instead of the drawing's size so the panner will always
// reflect the page's size.  The grid covers the entire page so it has
// the same size as the page.

void DrawingView::Update () {
    Graphic* backup = graphic;
    graphic = grid;
    UpdatePerspective();
    graphic = backup;
    Draw();
}

// Manipulate lets the user manipulate the Rubberband with the mouse
// until a specified event occurs.

void DrawingView::Manipulate (Event& e, Rubberband* rubberband, int et,
boolean constrain, boolean erase) {
    rubberband->SetPainter(rasterxor);
    rubberband->SetCanvas(canvas);
    Listen(allEvents);
    while (e.eventType != et) {
	if (e.eventType == MotionEvent) {
	    rubberband->Track(e.x, e.y);
	}
	Read(e);
	if (constrain) {
	    grid->Constrain(e.x, e.y);
	}
    }
    Listen(input);
    if (erase) {
	rubberband->Erase();
    }
}

// DrawHandles tells all the Selections to draw their handles unless
// they've already been drawn.

void DrawingView::DrawHandles () {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	sl->GetCur()->GetSelection()->DrawHandles(rasterxor, canvas);
    }
}

// RedrawHandles tells all the Selections to redraw their handles
// whether or not they've already been drawn.

void DrawingView::RedrawHandles () {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	sl->GetCur()->GetSelection()->RedrawHandles(rasterxor, canvas);
    }
}

// EraseHandles tells all the Selections to erase their handles unless
// they've already been erased.

void DrawingView::EraseHandles () {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	sl->GetCur()->GetSelection()->EraseHandles(rasterxor, canvas);
    }
}

// EraseExcessHandles erases the excess Selections' handles if it
// doesn't find the Selections in the current SelectionList.

void DrawingView::EraseExcessHandles (SelectionList* newsl) {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	Selection* oldselection = sl->GetCur()->GetSelection();
	if (!newsl->Find(oldselection)) {
	    oldselection->EraseHandles(rasterxor, canvas);
	}
    }
}

// ErasePickedHandles erases the picked Selection's handles if it
// finds the picked Selection in the SelectionList.

void DrawingView::ErasePickedHandles (Selection* pick) {
    if (sl->Find(pick)) {
	sl->GetCur()->GetSelection()->EraseHandles(rasterxor, canvas);
    }
}

// ErasePickedHandles erases the picked Selections' handles if it
// finds the picked Selections in the SelectionList.

void DrawingView::ErasePickedHandles (SelectionList* pl) {
    for (pl->First(); !pl->AtEnd(); pl->Next()) {
	Selection* pick = pl->GetCur()->GetSelection();
	if (sl->Find(pick)) {
	    sl->GetCur()->GetSelection()->EraseHandles(rasterxor, canvas);
	}
    }
}

// EraseUngraspedHandles erases all of the handles only if the
// SelectionList does not already include the picked Selection.

void DrawingView::EraseUngraspedHandles (Selection* pick) {
    if (!sl->Find(pick)) {
	EraseHandles();
    }
}

// Added adds the Selections to the list of Selections in the drawing
// to be drawn for the first time.

void DrawingView::Added () {
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
        damage->Added(sl->GetCur()->GetSelection());
    }
}

// Damaged adds the areas covered by the selected Selections
// (including their handles, too) to the list of damaged areas in the
// drawing to be repaired.

void DrawingView::Damaged () {
    BoxObj box;
    for (sl->First(); !sl->AtEnd(); sl->Next()) {
	sl->GetCur()->GetSelection()->GetPaddedBox(box);
        damage->Incur(box);
    }
}

// Repair repairs the drawing's damaged areas and then redraws the
// Selections' handles.  The damaged areas must have included all the
// handles for RedrawHandles to work correctly.

void DrawingView::Repair () {
    if (damage->Incurred()) {
	damage->Repair();
	RedrawHandles();
    }
}

// Magnify magnifies the given area to fill the view.

void DrawingView::Magnify (Coord l, Coord b, Coord r, Coord t) {
    Perspective np;
    np = *GetPerspective();
    np.curx += min(l, r);
    np.cury += min(b, t);
    np.curwidth = max(abs(r - l), 1);
    np.curheight = max(abs(t - b), 1);
    Adjust(np);
}

// Reduce reduces the drawing's magnification by a factor of two.

void DrawingView::Reduce () {
    SetMagnification(GetMagnification() / 2);
}

// Enlarge enlarges the drawing's magnification by a factor of two.

void DrawingView::Enlarge () {
    SetMagnification(2 * GetMagnification());
}

// NormalSize resets the drawing's magnification.

void DrawingView::NormalSize () {
    SetMagnification(1.);
}

// ReduceToFit reduces the drawing's magnification enough to fit all
// of the drawing in the window.

void DrawingView::ReduceToFit () {
    Perspective np;
    np = *GetPerspective();
    np.curx = np.x0;
    np.cury = np.y0;
    np.curwidth = np.width;
    np.curheight = np.height;
    Adjust(np);
}

// CenterPage scrolls the drawing so its center coincidences with the
// window's center.

void DrawingView::CenterPage () {
    Perspective np;
    np = *GetPerspective();
    np.curx = (np.width - np.curwidth) / 2;
    np.cury = (np.height - np.curheight) / 2;
    Adjust(np);
}

// Reconfig gives DrawingView the smallest possible canvas if the user
// wants a small window and creates a painter for drawing rubberbands.
// DrawingView will still get a canvas because it must fill some space
// to make the window rectangular.

void DrawingView::Reconfig () {
    GraphicBlock::Reconfig();
    if (strcmp(GetAttribute("small"), "true") == 0) {
	shape->width = 0;
	shape->height = 0;
    }
    if (rasterxor == nil) {
	rasterxor = new Painter(output);
    }
    Reconfigure(grid);
    Reconfigure(pageboundary);
}

// Reconfigure ensures the graphic has the same colors as the output
// painter.

void DrawingView::Reconfigure (Graphic* gr) {
    PColor* fg = gr->GetFgColor();
    PColor* bg = gr->GetBgColor();
    if (*fg != output->GetFgColor() || *bg != output->GetBgColor()) {
	fg = new IColor(output->GetFgColor(), "");
	bg = new IColor(output->GetBgColor(), "");
	gr->SetColors(fg, bg);
    }
}

// Redraw draws a rectangular subpart of the view.

void DrawingView::Redraw (Coord l, Coord b, Coord r, Coord t) {
    if (graphic != nil) {
	GraphicBlock::Redraw(l, b, r, t);
	rasterxor->Clip(canvas, l, b, r, t);
	for (sl->First(); !sl->AtEnd(); sl->Next()) {
	    Selection* selection = sl->GetCur()->GetSelection();
	    selection->RedrawUnclippedHandles(rasterxor, canvas);
	}
	rasterxor->NoClip();
    }
}

// Resize recreates damage in case canvas changed its value and calls
// drawhandler to reset the Selections' handles.

void DrawingView::Resize () {
    GraphicBlock::Resize();
    delete damage;
    damage = new Damage(canvas, output, graphic);
    drawhandler->Update();
}

// LimitMagnification limits the factor by which DrawingView may scale
// the view of the drawing.  Displaying polygons at very large
// magnifications may (figuratively) blow up the display (known to
// occur in X10 qdss server).  Very small factors may cause arithmetic
// overflows.  In addition, LimitMagnification updates State's stored
// magnification.  Alternatively, State could have attached itself to
// DrawingView's perspective if it was an Interactor like MagnifView.

float DrawingView::LimitMagnification (float desired) {
    const float MIN = 1./8.;
    const float MAX = 16.;

    if (desired < MIN) {
	desired = MIN;
    } else if (desired > MAX) {
	desired = MAX;
    }

    state->SetMagnif(desired);
    return desired;
}
