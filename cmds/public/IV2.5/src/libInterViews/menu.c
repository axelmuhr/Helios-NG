/*
 * Menu implementation.
 */

#include <InterViews/box.h>
#include <InterViews/font.h>
#include <InterViews/frame.h>
#include <InterViews/menu.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>

Menu::Menu (boolean persist) {
    persistent = persist;
    Init();
}

Menu::Menu (const char* name, boolean persist) {
    SetInstance(name);
    persistent = persist;
    Init();
}

Menu::Menu (Sensor* in, Painter* out, boolean persist) : (in, out) {
    persistent = persist;
    Init();
}

void Menu::Init () {
    SetClassName("Menu");
    layout = new VBox;
    cur = nil;
    xoff = 0;
    yoff = 0;
    delete input;
    input = new Sensor(onoffEvents);
    input->Catch(DownEvent);
    input->Catch(UpEvent);
}

void Menu::Reconfig () {
    MonoScene::Reconfig();
    if (xoff == 0) {
	xoff = shape->width / 2;
	yoff = shape->height / 2;
    }
}

HMenu::HMenu (boolean persist) : (persist) {
    Init();
}

HMenu::HMenu (const char* name, boolean persist) : (name, persist) {
    Init();
}

HMenu::HMenu (Sensor* in, Painter* out, boolean persist) : (in, out, persist) {
    Init();
}

void HMenu::Init () {
    SetClassName("HMenu");
    delete layout;
    layout = new HBox;
}

VMenu::VMenu (boolean persist) : (persist) {
    Init();
}

VMenu::VMenu (const char* name, boolean persist) : (name, persist) {
    Init();
}

VMenu::VMenu (Sensor* in, Painter* out, boolean persist) : (in, out, persist) {
    Init();
}

void VMenu::Init () {
    SetClassName("VMenu");
}

void Menu::DoInsert (Interactor* i, boolean, Coord&, Coord&) {
    layout->Insert(i);
}

void Menu::Compose () {
    Interactor* i;
    Coord x, y;

    i = new ShadowFrame(layout);
    PrepareToInsert(i);
    MonoScene::DoInsert(i, false, x, y);
}

/*
 * Pop up a menu in the world at the coordinates specified
 * by the given event.  The event is what caused the popup request --
 * set it to what causes the menu to close.  Store the selected item
 * in the final parameter as well as the current selection.
 */

void Menu::Popup (register Event& e, Interactor*& item) {
    register Interactor* i;
    World* w;
    Coord wx, wy;

    e.GetAbsolute(w, wx, wy);
    /* force computation of (xoff, yoff) */
    Config(w);
    w->InsertPopup(this, wx - xoff, wy - yoff);
    i = nil;
    for (;;) {
	Read(e);
	if (e.eventType == UpEvent) {
	    break;
	}
	if (e.target->Parent() == layout) {
	    i = e.target;
	    i->Handle(e);
	    if (e.eventType == UpEvent) {
		break;
	    }
	    if (!persistent) {
		Poll(e);
		if (e.x < 0 || e.x > xmax || e.y < 0 || e.y > ymax) {
		    i = nil;
		    break;
		}
	    }
	} else if (e.target == this && e.eventType == OffEvent) {
	    i = nil;
	    if (!persistent) {
		break;
	    }
	}
    }
    if (i != nil) {
	xoff = e.x;
	yoff = e.y;
	e.target->GetRelative(xoff, yoff, this);
    }
    cur = i;
    item = i;
    e.GetAbsolute(wx, wy);
    e.target = w;
    e.x = wx;
    e.y = wy;
    w->Remove(this);
    Flush();
}

void Menu::Handle (Event& e) {
    for (;;) {
	if (e.eventType == UpEvent) {
	    break;
	} else if (e.target->Parent() == layout) {
	    cur = e.target;
	    cur->Handle(e);
	    if (e.eventType == UpEvent) {
		break;
	    }
	} else if (e.target != this) {
	    Parent()->Handle(e);
	} else if (e.eventType != DownEvent) {
	    cur = nil;
	}
	Read(e);
    }
}

MenuItem::MenuItem (int t) {
    Init(t);
}

MenuItem::MenuItem (const char* name, int t) {
    SetInstance(name);
    Init(t);
}

MenuItem::MenuItem (Painter* out, int t) : (nil, out) {
    Init(t);
}

void MenuItem::Init (int t) {
    SetClassName("MenuItem");
    tag = t;
    normal = nil;
    highlight = nil;
    input = onoffEvents;
    input->Reference();
}

void MenuItem::Reconfig () {
    if (output == highlight) {
	output = normal;
    }
    delete highlight;
    highlight = new Painter(output);
    highlight->SetColors(output->GetBgColor(), output->GetFgColor());
    normal = output;
}

MenuItem::~MenuItem () {
    output = normal;
    delete highlight;
}

void MenuItem::Handle (Event& e) {
    switch (e.eventType) {
	case OnEvent:
	    Highlight();
	    break;
	case OffEvent:
	    UnHighlight();
	    break;
	default:
	    /* shouldn't happen */;
    }
}

/*
 * Default highlighting is to redraw the item with the highlight painter
 * as output.
 */

void MenuItem::Highlight () {
    output = highlight;
    Draw();
}

void MenuItem::UnHighlight () {
    output = normal;
    Draw();
}

/*
 * Simple text menu item.
 */

TextItem::TextItem (const char* s, int t) : (t) {
    Init(s);
}

TextItem::TextItem (const char* name, const char* s, int t) : (name, t) {
    Init(s);
}

TextItem::TextItem (Painter* out, const char* s, int t) : (out, t) {
    Init(s);
}

void TextItem::Init (const char* s) {
    SetClassName("TextItem");
    text = s;
}

static const int pad = 2;

void TextItem::Reconfig () {
    MenuItem::Reconfig();
    Font* f = output->GetFont();
    shape->width = f->Width(text) + 2*pad;
    shape->height = f->Height() + 2*pad;
    shape->Rigid(0, hfil, 0, 0);
}

void TextItem::Redraw (Coord, Coord, Coord, Coord) {
    output->ClearRect(canvas, 0, 0, xmax, ymax);
    output->Text(canvas, text, (xmax - shape->width) / 2 + pad, pad);
}
