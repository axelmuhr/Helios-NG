// $Header: state.c,v 1.8 89/03/19 12:18:30 interran Exp $
// implements class State.

#include "ipaint.h"
#include "istring.h"
#include "listintrctr.h"
#include "mapipaint.h"
#include "state.h"
#include <InterViews/graphic.h>
#include <InterViews/interactor.h>

// State maintains a Graphic state as well as some other attributes.

State::State (Interactor* i) {
    drawingname = nil;
    graphicstate = new FullGraphic;
    gridding = false;
    magnif = 1.0;
    mapibrush = new MapIBrush(i, "brush");
    mapifgcolor = new MapIColor(i, "fgcolor");
    mapibgcolor = new MapIColor(i, "bgcolor");
    mapifont = new MapIFont(i, "font");
    mapipattern = new MapIPattern(i, "pattern");
    modifstatus = Unmodified;
    viewlist = new InteractorList;

    SetBrush(mapibrush->GetInitial());
    SetFgColor(mapifgcolor->GetInitial());
    SetBgColor(mapibgcolor->GetInitial());
    SetFillBg(true);
    SetFont(mapifont->GetInitial());
    SetPattern(mapipattern->GetInitial());
}

// ~State frees storage allocated to store members.

State::~State () {
    delete drawingname;
    delete graphicstate;
    delete mapibrush;
    delete mapifgcolor;
    delete mapibgcolor;
    delete mapifont;
    delete mapipattern;
    delete viewlist;
}

// The following operations return Graphic and nonGraphic attributes
// of the State.

IBrush* State::GetBrush () {
    return (IBrush*) graphicstate->GetBrush();
}

IColor* State::GetFgColor () {
    return (IColor*) graphicstate->GetFgColor();
}

IColor* State::GetBgColor () {
    return (IColor*) graphicstate->GetBgColor();
}

const char* State::GetDrawingName () {
    return drawingname;
}

boolean State::GetFillBg () {
    return graphicstate->BgFilled();
}

IFont* State::GetFont () {
    return (IFont*) graphicstate->GetFont();
}

boolean State::GetGridding () {
    return gridding;
}

float State::GetMagnif () {
    return magnif;
}

MapIBrush* State::GetMapIBrush () {
    return mapibrush;
}

MapIColor* State::GetMapIFgColor () {
    return mapifgcolor;
}

MapIColor* State::GetMapIBgColor () {
    return mapibgcolor;
}

MapIFont* State::GetMapIFont () {
    return mapifont;
}

MapIPattern* State::GetMapIPattern () {
    return mapipattern;
}

ModifStatus State::GetModifStatus () {
    return modifstatus;
}

IPattern* State::GetPattern () {
    return (IPattern*) graphicstate->GetPattern();
}

// The following operations set Graphic and nonGraphic attributes of
// the State.

void State::SetBrush (IBrush* b) {
    graphicstate->SetBrush(b);
}

void State::SetFgColor (IColor* fg) {
    IColor* bg = (IColor*) graphicstate->GetBgColor();
    graphicstate->SetColors(fg, bg);
}

void State::SetBgColor (IColor* bg) {
    IColor* fg = (IColor*) graphicstate->GetFgColor();
    graphicstate->SetColors(fg, bg);
}

void State::SetDrawingName (const char* name) {
    delete drawingname;
    drawingname = name ? strdup(name) : nil;
}

void State::SetFillBg (boolean fill) {
    graphicstate->FillBg(fill);
}

void State::SetFont (IFont* f) {
    graphicstate->SetFont(f);
}

void State::SetGridding (boolean g) {
    gridding = g;
}

void State::SetMagnif (float m) {
    magnif = m;
}

void State::SetModifStatus (ModifStatus m) {
    modifstatus = m;
}

void State::SetPattern (IPattern* p) {
    graphicstate->SetPattern(p);
}

// Attach informs us a view has attached itself to us.

void State::Attach (Interactor* i) {
    viewlist->Append(new InteractorNode(i));
}

// Detach informs us a view has detached itself from us.

void State::Detach (Interactor* i) {
    if (viewlist->Find(i)) {
	viewlist->DeleteCur();
    }
}

// UpdateViews informs all attached views we have changed our state.

void State::UpdateViews () {
    for (viewlist->First(); !viewlist->AtEnd(); viewlist->Next()) {
	Interactor* view = viewlist->GetCur()->GetInteractor();
	view->Update();
    }
}
