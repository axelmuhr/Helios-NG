// $Header: panel.c,v 1.8 89/04/17 00:31:21 linton Exp $
// implements classes Panel and PanelItem.

#include "istring.h"
#include "panel.h"
#include <InterViews/event.h>
#include <InterViews/font.h>
#include <InterViews/painter.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <ctype.h>

// Define the maximum value of any character.

static const int MAXCHAR = 127;

// Panel starts with no currently highlighted or stored items.

Panel::Panel () {
    cur = nil;
    for (int i = 0; i <= MAXCHAR; i++) {
	items[i] = nil;
    }
}

// Enter stores an item using its associated character as its index
// and tells it it can get its highlight painter from us.

void Panel::Enter (PanelItem* i, char c) {
    if (c >= 0 && c <= MAXCHAR) {
	items[c] = i;
    }
    i->SetHighlighterParent(this);
}

// LookUp returns the item associated with the given character.

PanelItem* Panel::LookUp (char c) {
    PanelItem* i = nil;
    if (c >= 0 && c <= MAXCHAR) {
	i = items[c];
    }
    return i;
}

// Highlight sets the currently highlighted item.  The panel
// highlights only one item at any time.

void Panel::Highlight (PanelItem* item) {
    if (cur != nil) {
	cur->Unhighlight();
    }
    cur = item;
    if (cur != nil) {
	cur->Highlight();
    }
}

// GetCur returns the currently highlighted item.

PanelItem* Panel::GetCur () {
    return cur;
}

// PerformCurrentFunction tells the currently highlighted item to
// perform its function.

void Panel::PerformCurrentFunction (Event& e) {
    if (cur != nil) {
	cur->Perform(e);
    }
}

// PerformTemporaryFunction temporarily highlights the item associated
// with the given character and tells it to perform its function.

void Panel::PerformTemporaryFunction (Event& e, char c) {
    PanelItem* tmp = LookUp(c);
    if (tmp != nil) {
	PanelItem* prev = cur;
	Highlight(tmp);
	tmp->Perform(e);
	Highlight(prev);
    }
}

// PanelItem stores the panel it belongs to and its text labels.

PanelItem::PanelItem (Panel* p, const char* l, const char* k, char c) {
    p->Enter(this, c);
    panel = p;
    name = strdup(l ? l : "");
    key = strdup(k ? k : "");
    input = updownEvents;
    input->Reference();
}

// Free storage allocated to store the text labels.

PanelItem::~PanelItem () {
    delete name;
    delete key;
}

// Handle highlights the item if the user clicks on it or types its
// associated character (mapped by the program).

void PanelItem::Handle (Event& e) {
    switch (e.eventType) {
    case DownEvent:
    case KeyEvent:
	panel->Highlight(this);
	break;
    default:
	break;
    }
}

// Perform performs the item's function.

void PanelItem::Perform (Event&) {
    // define it in your subclass
}

// Reconfig pads the item's shape to accomodate its text labels.
// Basing padding on the font in use ensures the padding will change
// proportionally with changes in the font's size.

static const float WIDTHPAD = 1.5; // fraction of font->Width(EM)
static const float HTPAD = 0.77;   // fraction of font->Height()
static const float KEYPAD = 1./6.; // fraction of font->Width(EM)
static const char* EM = "m";	   // widest alphabetic character in any font

void PanelItem::Reconfig () {
    Highlighter::Reconfig();
    Font* font = output->GetFont();
    int xpad = round(WIDTHPAD * font->Width(EM));
    int ypad = round(HTPAD * font->Height());
    shape->width = font->Width(name) + (2 * xpad);
    shape->height = font->Height() + (2 * ypad);
    shape->Rigid(2 * xpad, hfil, 2 * ypad, 0);
}

// Redraw displays the text labels.

void PanelItem::Redraw (Coord l, Coord b, Coord r, Coord t) {
    output->ClearRect(canvas, l, b, r, t);
    output->FillBg(false);
    output->Text(canvas, name, name_x, name_y);
    output->Text(canvas, key, key_x, key_y);
    output->FillBg(true);
}

// Resize calculates the text labels' positions.  For the convenience
// of subclasses which want to draw graphic labels centered in the
// canvas and as large as possible while maintaining a 1:1 aspect
// ratio, Resize also calculates side, offx, and offy which define the
// largest possible square centered in the canvas.

void PanelItem::Resize () {
    Font* font = output->GetFont();
    name_x = max(0, (xmax - font->Width(name) + 1) / 2);
    name_y = (ymax - font->Height() + 1) / 2;
    int pad = round(KEYPAD * font->Width(EM));
    key_x = xmax - font->Width(key) - pad;
    key_y = pad;
    side = min(xmax, ymax);
    offx = (xmax - side) / 2;
    offy = (ymax - side) / 2;
}
