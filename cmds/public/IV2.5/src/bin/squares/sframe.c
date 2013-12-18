/*
 * Implementation of the frame around a squares view.
 */

#include "sframe.h"
#include "squares.h"
#include "metaview.h"
#include "view.h"
#include <InterViews/banner.h>
#include <InterViews/border.h>
#include <InterViews/box.h>
#include <InterViews/button.h>
#include <InterViews/event.h>
#include <InterViews/frame.h>
#include <InterViews/glue.h>
#include <InterViews/menu.h>
#include <InterViews/panner.h>
#include <InterViews/scroller.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/tray.h>
#include <InterViews/viewport.h>
#include <InterViews/world.h>
#include <string.h>

static int nviews;

SquaresFrame::SquaresFrame (SquaresFrame* f) {
    view = new SquaresView(f->view->subject);
    Init();
    style = new SquaresMetaView(f->style);
    MakeFrame();
}

SquaresFrame::SquaresFrame (SquaresView* v) {
    const char* a;

    view = v;
    Init();
    style = new SquaresMetaView;
    a = GetAttribute("panner");
    style->type = (a == nil) ? AdjustByScrollers : AdjustByPanner;
    a = GetAttribute("adjustersize");
    if (*a == 'm') {
	style->size = Medium;
    } else if (*a == 'l') {
	style->size = Large;
    } else {
	style->size = Small;
    }
    style->right = (GetAttribute("left") == nil);
    style->below = (GetAttribute("above") == nil);
    style->hscroll = (GetAttribute("!hscroll") == nil);
    style->vscroll = (GetAttribute("!vscroll") == nil);
    MakeFrame();
}

void SquaresFrame::Init () {
    SetClassName("SquaresFrame");
    menu = new Menu;
    menu->Insert(new TextItem("add square", 'a'));
    menu->Insert(new TextItem("new view", 'v'));
    menu->Insert(new TextItem("view setup", 's'));
    menu->Insert(new TextItem("close", 'C'));
    menu->Insert(new TextItem("quit", 'q'));
    menu->Compose();
    adjust = new Menu;
    adjust->Insert(new TextItem("zoom in", 'z'));
    adjust->Insert(new TextItem("zoom out", 'Z'));
    adjust->Insert(new TextItem("normal size", 'n'));
    adjust->Insert(new TextItem("center view", 'c'));
    adjust->Compose();
    quit = new Menu;
    quit->Insert(new TextItem("yes, quit", 'q'));
    quit->Insert(new TextItem("no, don't quit", 'n'));
    quit->Compose();
    delete input;
    input = updownEvents;
    input->Reference();
    viewport = new Viewport(new Frame(view));
    Propagate(false);
    ++nviews;
}

SquaresFrame::~SquaresFrame () {
    delete menu;
    delete quit;
    delete style;
}

void SquaresFrame::MakeFrame () {
    Scene* p = viewport->Parent();
    Interactor* interior;

    if (p != nil) {
	p->Remove(viewport);
    }
    if (style->type == AdjustByPanner) {
	interior = PannerFrameInterior();
    } else if (style->type == AdjustByScrollers) {
	interior = ScrollerFrameInterior();
    }
    Insert(interior);
}

Interactor* SquaresFrame::ScrollerFrameInterior () {
    Interactor* v = viewport;
    Tray* t = new Tray;
    Interactor* hs, *vs;
    Border* hb, *vb;
    int size;

    if (style->size == Small) {
	size = round(0.15*inches);
    } else if (style->size == Medium) {
	size = round(0.20*inches);
    } else if (style->size == Large) {
	size = round(0.25*inches);
    }

    if (style->vscroll) {
	vs = new VScroller(viewport, size);
	vb = new VBorder;

	if (style->right) {
	    t->HBox(t, v, vb, vs, t);
	} else {
	    t->HBox(t, vs, vb, v, t);
	}
	t->VBox(t, vb, t);
    } else {
	t->HBox(t, v, t);
    }

    if (style->hscroll) {
	hs = new HScroller(viewport, size);
	hb = new HBorder;

	if (style->below) {
	    t->VBox(t, v, hb, hs, t);
	} else {
	    t->VBox(t, hs, hb, v, t);
	}
	t->HBox(t, hb, t);
    } else {
	t->VBox(t, v, t);
    }
    
    if (style->vscroll && style->hscroll) {
	if (style->below) {
	    t->VBox(t, vs, hb);
	} else {
	    t->VBox(hb, vs, t);
	}
	if (style->right) {
	    t->HBox(t, hs, vb);
	} else {
	    t->HBox(vb, hs, t);
	}

    } else if (style->vscroll) {
	t->VBox(t, vs, t);

    } else if (style->hscroll) {
	t->HBox(t, hs, t);
    }
    return t;
}

Interactor* SquaresFrame::PannerFrameInterior () {
    Interactor* v = viewport;
    Tray* t = new Tray(v);
    int size;

    if (style->size == Small) {
	size = 0;
    } else if (style->size == Medium) {
	size = round(0.75*inches);
    } else if (style->size == Large) {
	size = round(1.0*inches);
    }
    t->Align(style->align, new Frame(new Panner(v, size)));
    t->Propagate(false);
    return t;
}

void SquaresFrame::Handle (Event& e) {
    if (e.eventType == DownEvent) {
	Interactor* item;

	if (e.button == RIGHTMOUSE) {
	    adjust->Popup(e, item);
	} else {
	    menu->Popup(e, item);
	}
	if (item != nil) {
	    TextItem* i = (TextItem*)item;
	    switch (i->tag) {
		case 'a':
		    view->subject->Add();
		    break;
		case 'z':
		    viewport->ZoomBy(2.0, 2.0);
		    break;
		case 'Z':
		    viewport->ZoomBy(0.5, 0.5);
		    break;
                case 'n':
                    viewport->ZoomTo(1.0, 1.0);
                    break;
                case 'c':
                    viewport->ScrollTo(0.5, 0.5);
                    break;
		case 'v':
		    World* w;
		    Coord wx, wy;
		    e.GetAbsolute(w, wx, wy);
		    w->InsertToplevel(new SquaresFrame(this), this);
		    break;
		case 's':
		    if (style->Popup(e)) {
			MakeFrame();
			Change();
		    }
		    break;
                case 'C':
		    if (nviews == 1) {
			quit->Popup(e, item);
			if (item != nil) {
			    i = (TextItem*)item;
			    if (i->tag == 'q') {
				e.target = nil;
				delete this;
			    }
			}
		    } else {
			--nviews;
			delete this;
		    }
                    break;
		case 'q':
		    quit->Popup(e, item);
		    if (item != nil) {
			i = (TextItem*)item;
			if (i->tag == 'q') {
			    e.target = nil;
			}
		    }
		    break;
	    }
	}
    }
}
