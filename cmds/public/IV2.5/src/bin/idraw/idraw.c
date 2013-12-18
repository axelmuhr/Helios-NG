// $Header: idraw.c,v 1.10 89/04/17 00:30:43 linton Exp $
// implements class Idraw.

#include "commands.h"
#include "drawing.h"
#include "drawingview.h"
#include "editor.h"
#include "errhandler.h"
#include "idraw.h"
#include "istring.h"
#include "mapkey.h"
#include "state.h"
#include "stateviews.h"
#include "tools.h"
#include <InterViews/Graphic/ppaint.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/cursor.h>
#include <InterViews/event.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/panner.h>
#include <InterViews/sensor.h>
#include <stdio.h>
#include <stdlib.h>

// Idraw parses its command line and initializes its members.

Idraw::Idraw (int argc, char** argv) {
    ParseArgs(argc, argv);
    InitPPaint();		// I keep forgetting to call this....
    Init();
}

// Free storage allocated for members not in Idraw's scene.

Idraw::~Idraw () {
    delete drawing;
    delete editor;
    delete errhandler;
    delete mapkey;
    delete state;
}

// Run opens the initial file if one was given before starting to run.

void Idraw::Run () {
    if (initialfile != nil) {
	SetCursor(hourglass);
	editor->Open(initialfile);
	SetCursor(defaultCursor);
    }

    Interactor::Run();
}

// Handle routes keystrokes to their associated interactors.

void Idraw::Handle (Event& e) {
    switch (e.eventType) {
    case KeyEvent:
	if (e.len > 0) {
	    Interactor* i = mapkey->LookUp(e.keystring[0]);
	    if (i != nil) {
		i->Handle(e);
	    }
	}
	break;
    default:
	break;
    }
}

// Update knows DrawingView called it and DrawingView just finished
// redrawing the entire view so Update tells Drawing to update all of
// its Selections' handles in case the view moved.

void Idraw::Update () {
    drawing->ResetAllHandles();
}

// ParseArgs stores the name of an initial file to open if any.

void Idraw::ParseArgs (int argc, char** argv) {
    initialfile	= nil;
    if (argc == 2) {
	initialfile = argv[1];
    } else if (argc > 2) {
	fprintf(stderr, "too many arguments, usage: idraw [file]\n");
	const int PARSINGERROR = 1;
	exit(PARSINGERROR);
    }
}

// Init creates a sensor to catch keystrokes, creates members and
// initializes links between them, and composes them into a view with
// boxes, borders, glue, and frames.

void Idraw::Init () {
    input = new Sensor;
    input->Catch(KeyEvent);

    drawing	= new Drawing(8.5*inches, 11*inches);
    drawingview	= new DrawingView(drawing->GetPage());
    editor	= new Editor(this);
    errhandler	= new ErrHandler;
    mapkey	= new MapKey;
    state	= new State(this);
    tools	= new Tools(editor, mapkey);

    drawingview->SetDrawHandler(this);
    drawingview->SetEventHandler(tools);
    drawingview->SetGrid(drawing->GetGrid());
    drawingview->SetPageBoundary(drawing->GetPageBoundary());
    drawingview->SetSelectionList(drawing->GetSelectionList());
    drawingview->SetState(state);
    editor->SetDrawing(drawing);
    editor->SetDrawingView(drawingview);
    editor->SetGrid(drawing->GetGrid());
    editor->SetState(state);
    errhandler->SetEditor(editor);
    errhandler->Install();

    HBox* us = new HBox;
    us->Insert(new ModifStatusView(state));
    us->Insert(new DrawingNameView(state));
    us->Insert(new GriddingView(state));
    us->Insert(new FontView(state));
    us->Insert(new MagnifView(state, drawingview));

    HBox* ls_c = new HBox;
    ls_c->Insert(new BrushView(state));
    ls_c->Insert(new VBorder);
    ls_c->Insert(new PatternView(state));
    ls_c->Insert(new VBorder);
    ls_c->Insert(new Commands(editor, mapkey, state));
    ls_c->Insert(new HGlue);

    VBox* us_ls_c = new VBox;
    us_ls_c->Insert(us);
    us_ls_c->Insert(new HBorder);
    us_ls_c->Insert(ls_c);

    VBox* t_p = new VBox;
    t_p->Insert(tools);
    t_p->Insert(new VGlue);
    t_p->Insert(new HBorder);
    t_p->Insert(new Panner(drawingview));
    t_p->Propagate(false);

    HBox* t_p_d = new HBox;
    t_p_d->Insert(t_p);
    t_p_d->Insert(new VBorder);
    t_p_d->Insert(drawingview);

    VBox* us_ls_c_t_p_d = new VBox;
    us_ls_c_t_p_d->Insert(us_ls_c);
    us_ls_c_t_p_d->Insert(new HBorder);
    us_ls_c_t_p_d->Insert(t_p_d);

    Insert(new Frame(us_ls_c_t_p_d, 1));
}
