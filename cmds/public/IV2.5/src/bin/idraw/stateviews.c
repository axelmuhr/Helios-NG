// $Header: stateviews.c,v 1.9 89/04/17 00:31:54 linton Exp $
// implements classes StateView and StateView's subclasses.

#include "ipaint.h"
#include "istring.h"
#include "sllines.h"
#include "state.h"
#include "stateviews.h"
#include <InterViews/painter.h>
#include <InterViews/perspective.h>
#include <InterViews/shape.h>
#include <stdio.h>

// StateView attaches itself to the State's list of views to update
// and stores the State and text label.

StateView::StateView (State* s, const char* l) {
    s->Attach(this);
    state = s;
    label = strdup(l ? l : "");
}

// Free storage allocated for the text label.

StateView::~StateView () {
    delete label;
}

// Reconfig pads the view's shape to accomodate its text label.
// Basing padding on the font in use ensures the padding will change
// proportionally with changes in the font's size.

static const float WIDTHPAD = 1.0; // fraction of font->Width(EM)
static const float HTPAD = 0.2;	   // fraction of font->Height() 
static const char* EM = "m";	   // widest alphabetic character in any font

void StateView::Reconfig () {
    Interactor::Reconfig();
    Font* font = output->GetFont();
    int xpad = round(WIDTHPAD * font->Width(EM));
    int ypad = round(HTPAD * font->Height());
    shape->width = font->Width(label) + (2 * xpad);
    shape->height = font->Height() + (2 * ypad);
    shape->Rigid(shape->width - xpad, 0, 2 * ypad, 0);
}

// Redraw displays the text label.

void StateView::Redraw (Coord l, Coord b, Coord r, Coord t) {
    output->ClearRect(canvas, l, b, r, t);
    output->Text(canvas, label, label_x, label_y);
}

// Resize centers the text label's position unless the label won't fit
// on the canvas, in which case Resize left justifies the position.

void StateView::Resize () {
    Font* font = output->GetFont();
    label_x = max(0, (xmax - font->Width(label) + 1) / 2);
    label_y = (ymax - font->Height() + 1) / 2;
}

// BrushView creates a graphic to demonstrate the brush's effect on a
// line.

static const int PICXMAX = 28;	// chosen to minimize scaling for canvas
static const int PICYMAX = 18;

BrushView::BrushView (State* s) : (s, " N ") {
    brushindic = new LineSelection(0, 0, PICXMAX, 0, *state);
}

// Free storage allocated for the graphic.

BrushView::~BrushView () {
    delete brushindic;
}

// Skew comments/code ratio to work around cpp bug
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

// Update updates the view if any of the brush, colors, or pattern
// changes.

void BrushView::Update () {
    IBrush* brush = state->GetBrush();
    IColor* fgcolor = state->GetFgColor();
    IColor* bgcolor = state->GetBgColor();
    IPattern* pattern = state->GetPattern();
    if (brushindic->GetBrush() != brush ||
	brushindic->GetFgColor() != fgcolor ||
	brushindic->GetBgColor() != bgcolor ||
	brushindic->GetPattern() != pattern)
    {
	brushindic->SetBrush(brush);
	brushindic->SetColors(fgcolor, bgcolor);
	brushindic->SetPattern(pattern);
	Draw();
    }
}

// Reconfig computes BrushView's shape and makes room for the VBorder
// between itself and the PatternView.

void BrushView::Reconfig () {
    StateView::Reconfig();
    shape->width -= 1;
    shape->Rigid();
}

// Redraw displays the text label if the brush's the none brush, else
// it displays the graphic to demonstrate the brush's effect.

void BrushView::Redraw (Coord l, Coord b, Coord r, Coord t) {
    IBrush* brush = (IBrush*) brushindic->GetBrush();
    if (brush->None()) {
	StateView::Redraw(l, b, r, t);
    } else {
	output->ClearRect(canvas, l, b, r, t);
	brushindic->Draw(canvas);
    }
}

// Resize scales the graphic to fit the canvas' size.

void BrushView::Resize () {
    StateView::Resize();
    float xmag = float(xmax) / PICXMAX;
    float hy = float(ymax) / 2;
    brushindic->SetTransformer(nil);
    brushindic->Scale(xmag, 1.);
    brushindic->Translate(0., hy);
}

// DrawingNameView just passes the drawing's name to StateView.

DrawingNameView::DrawingNameView (State* s) : (s, GetDrawingName(s)) {
}

// Update updates the view of the drawing's name.

void DrawingNameView::Update () {
    const char* drawingname = GetDrawingName(state);
    if (strcmp(drawingname, label) != 0) {
	delete label;
	label = strdup(drawingname);
	Resize();
	Draw();
    }
}

// Reconfig gives DrawingNameView's shape some stretchability to get
// more space if the HBox has room for it to stretch.

void DrawingNameView::Reconfig () {
    StateView::Reconfig();
    shape->hstretch = hfil;
}

// Resize left justifies the text label's position.

void DrawingNameView::Resize () {
    Font* font = output->GetFont();
    label_x = 0;
    label_y = (ymax - font->Height() + 1) / 2;
}

// GetDrawingName returns the drawing's name or a default label if it
// has no name.

const char* DrawingNameView::GetDrawingName (State* state) {
    const char* drawingname = state->GetDrawingName();
    return drawingname ? drawingname : "[unnamed drawing]";
}

// FontView passes the font's print name and size to StateView for its
// label.

FontView::FontView (State* s) : (s, GetPrintFontAndSize(s)) {
    background = nil;
}

// Free storage allocated for the background painter.

FontView::~FontView () {
    delete background;
}

// Update updates the view if the label or the current color changes.

void FontView::Update () {
    const char* name = GetPrintFontAndSize(state);
    IColor* fgcolor = state->GetFgColor();
    if (strcmp(name, label) != 0) {
	delete label;
	label = strdup(name);
	output->SetColors(*fgcolor, output->GetBgColor());
	Resize();
	Draw();
    } else if (output->GetFgColor() != *fgcolor) {
	output->SetColors(*fgcolor, output->GetBgColor());
	Draw();
    }
}

// Reconfig gives FontView's shape some stretchability to get more
// space if the HBox has room for it to stretch, creates a new painter
// to draw a gray background behind the label, and replaces output
// with a new painter to use a different color.

void FontView::Reconfig () {
    StateView::Reconfig();
    shape->hstretch = hfil;

    if (background == nil) {
	background = new Painter(output);
	background->SetPattern(gray);
    }

    Painter* fontindic = new Painter(output);
    IColor* fgcolor = state->GetFgColor();
    fontindic->SetColors(*fgcolor, fontindic->GetBgColor());
    fontindic->FillBg(false);
    delete output;
    output = fontindic;
}

// Redraw displays the graphic label over a gray background to make
// the label visible even if it uses the white color.

void FontView::Redraw (Coord l, Coord b, Coord r, Coord t) {
    background->FillRect(canvas, l, b, r, t);
    output->Text(canvas, label, label_x, label_y);
}

// GetPrintFontAndSize returns the font's print name and size.

const char* FontView::GetPrintFontAndSize (State* state) {
    IFont* f = state->GetFont();
    return f->GetPrintFontAndSize();
}

// GriddingView passes the grid's gridding on/off status to StateView.

GriddingView::GriddingView (State* s) : (s, GetGridding(s)) {
}

// Update updates the view of the grid's gridding on/off status.

void GriddingView::Update () {
    const char* gridding = GetGridding(state);
    if (strcmp(gridding, label) != 0) {
	delete label;
	label = strdup(gridding);
	Draw();
    }
}

// GetGridding returns the status of the grid's gridding as a text
// string.

const char* GriddingView::GetGridding (State* state) {
    const char* gridding = nil;
    if (state->GetGridding()) {
	gridding = "gridding on";
    } else {
	gridding = "          ";
    }
    return gridding;
}

// MagnifView attaches itself to the DrawingView's perspective to get
// itself updated whenever the magnification changes.

MagnifView::MagnifView (State* s, Interactor* dwgview) : (s, GetMagnif(s)) {
    dwgview->GetPerspective()->Attach(this);
}

// Update updates the view of the current magnification.

void MagnifView::Update () {
    const char* magnif = GetMagnif(state);
    if (strcmp(magnif, label) != 0) {
	delete label;
	label = strdup(magnif);
	Resize();
	Draw();
    }
}

// Resize right justifies the text label's position.

void MagnifView::Resize () {
    Font* font = output->GetFont();
    label_x = xmax - font->Width(label) + 1;
    label_y = (ymax - font->Height() + 1) / 2;
}

// GetMagnif returns the drawing's magnification as a text string.

const char* MagnifView::GetMagnif (State* state) {
    static char mag[20];

    float magnif = state->GetMagnif();
    sprintf(mag, "  mag %.10gx ", magnif);
    return mag;
}

// ModifStatusView just passes the modification status to StateView.

ModifStatusView::ModifStatusView (State* s) : (s, GetModifStatus(s)) {
}

// Update updates the view of the current modification status.

void ModifStatusView::Update () {
    const char* modifstatus = GetModifStatus(state);
    if (strcmp(modifstatus, label) != 0) {
	delete label;
	label = strdup(modifstatus);
	Draw();
    }
}

// GetModifStatus returns the drawing's modification status.

const char* ModifStatusView::GetModifStatus (State* state) {
    switch (state->GetModifStatus()) {
    case ReadOnly:
	return "%";
    case Unmodified:
	return " ";
    default:
	return "*";
    }
}

// PatternView passes its none pattern label to StateView.

PatternView::PatternView (State* s) : (s, " N ") {
    patindic = nil;
}

// Free storage allocated for the fillrect painter.

PatternView::~PatternView () {
    delete patindic;
}

// Update updates the view if any of the colors or pattern changes.

void PatternView::Update () {
    IColor* fgcolor = state->GetFgColor();
    IColor* bgcolor = state->GetBgColor();
    IPattern* pattern = state->GetPattern();
    if (patindic->GetFgColor() != *fgcolor ||
	patindic->GetBgColor() != *bgcolor ||
	patindic->GetPattern() != *pattern)
    {
	patindic->SetColors(*fgcolor, *bgcolor);
	patindic->SetPattern(*pattern);
	Draw();
    }
}

// Reconfig creates the pattern indicator's painter.

void PatternView::Reconfig () {
    StateView::Reconfig();
    shape->Rigid();
    if (patindic == nil) {
	IColor* fgcolor = state->GetFgColor();
	IColor* bgcolor = state->GetBgColor();
	IPattern* pattern = state->GetPattern();
	patindic = new Painter(output);
	patindic->SetColors(*fgcolor, *bgcolor);
	patindic->SetPattern(*pattern);
    }
}

// Redraw displays the text label if the pattern's the none pattern,
// else it displays the fillrect to demonstrate the pattern's effect.

void PatternView::Redraw (Coord l, Coord b, Coord r, Coord t) {
    if (state->GetPattern()->None()) {
	StateView::Redraw(l, b, r, t);
    } else {
	patindic->FillRect(canvas, l, b, r, t);
    }
}
