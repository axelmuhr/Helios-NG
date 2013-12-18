/*
 * Pages - demo for Text and Layout
 */

#include "pages.h"

#include <InterViews/Text/layout.h>
#include <InterViews/Text/textblock.h>
#include <InterViews/Text/textpainter.h>

#include <InterViews/deck.h>
#include <InterViews/sensor.h>
#include <InterViews/paint.h>
#include <InterViews/scroller.h>
#include <InterViews/box.h>
#include <InterViews/border.h>
#include <InterViews/glue.h>
#include <InterViews/scene.h>

#include <ctype.h>

extern Text* Build(FILE*, Layout*);

static const int WIDTH = 400;
static const int HEIGHT = 300;

Pages::Pages (FILE* f, int p, int c) : (nil, stdpaint) {
    hit = nil;
    after = nil;
    edit = nil;

    sensor = new Sensor;
    sensor->Catch(DownEvent);
    sensor->Catch(KeyEvent);

    normal = new TextPainter(output);
    highlight = new TextPainter(output);
    highlight->SetColors(output->GetBgColor(), output->GetFgColor());
    Composition* text = new Composition(nil);
    layout = new Layout(text, sensor, normal);
    text->Build(Build(f, layout));
    text->Reshape();

    deck = new Deck;
    pages = p;
    columns = c;

    for (int i = 1; i <= pages; ++i) {
	HBox* inner = new HBox;
	inner->Insert(new HGlue(5, 0, 0));
        TextBlock* block;
        for (int j = 1; j <= columns; ++j) {
            block = new TextBlock(WIDTH/columns, HEIGHT, this);
            layout->Chain(block);
            inner->Insert(block);
            if (j != columns) {
                inner->Insert(new HGlue(5, 0, 0));
                inner->Insert(new VBorder);
                inner->Insert(new HGlue(5, 0, 0));
            }
        }
	inner->Insert(new HGlue(5, 0, 0));
        if (i == pages) {
            inner->Insert(new VBorder);
            inner->Insert(new VScroller(block, 7));
        }
	VBox* page = new VBox(
	    new VGlue(10, 0, 0),
	    inner,
	    new VGlue(10, 0, 0)
	);
	deck->Insert(page);
    }

    Box* contents = new VBox(deck);
    if (pages > 1) {
        contents->Insert(new HBorder);
        contents->Insert(new HScroller(deck));
    }
    Insert(contents);
}

void Pages::Delete () {
    delete layout;
    delete normal;
    delete highlight;
    delete sensor;
}

void Pages::Handle (Event& e) {
    TextData hit_context, after_context;
    Text* newhit;
    Text* newafter;
    Word* w;
    if (e.eventType == KeyEvent) {
	if (e.len > 0 && hit != nil) {
	    switch (e.keystring[0]) {
	    case '\002':
		if (edit != nil) {
		    edit->Backward();
		}
		break;
	    case '\006':
		if (edit != nil) {
		    edit->Forward();
		}
		break;
	    case '\010':
	    case '\177':
	    case '\004':
	    case '\027':
		if (edit != nil) {
		    edit->Delete();
		    edit->Parent()->Reshape();
		} else if (hit != nil) {
		    Composition* parent = hit->Parent();
		    if (parent != nil) {
			layout->Unpaint(hit);
			parent->Remove(hit);
			delete hit;
			hit = nil;
			layout->Show(parent);
		    }
		}
		break;
	    case ' ':
	    case '\n':
	    case '\r':
	    case '\t':
		w = new Word("",0);
		if (edit == nil) {
		    edit = new EditWord(editbuffer,255);
		    layout->Unpaint(hit);
		} else {
		    edit->Done();
		}
		hit->Parent()->InsertAfter(hit,w);
		hit = w;
		edit->Edit(w);
		break;
	    default:
		if (isgraph(e.keystring[0]) && edit != nil) {
		    edit->Insert(e.keystring, e.len);
		    edit->Parent()->Reshape();
		}
		break;
	    }
	    if (edit != nil) {
		layout->Show(edit->Parent());
	    }
	}
    } else if (e.eventType==DownEvent &&
	layout->Hit(e, hit_context, after_context)
    ) {
	newhit = (Text*)hit_context;
	newafter = (Text*)after_context;
	if (edit != nil) {
	    edit->Done();
	    if (newhit == edit) {
		newhit = hit;
		layout->Paint(hit, highlight);
	    }
	    delete edit;
	    edit = nil;
	    layout->Show(hit);
	}
	if (newhit != hit) {
	    if (hit != nil) {
		layout->Unpaint(hit);
		layout->Show(hit);
	    }
	    if (newhit != nil) {
		layout->Paint(newhit, highlight);
		layout->Show(newhit);
	    }
	    hit = newhit;
	    after = newafter;
	}
    } else {
	e.target->Handle(e);
    }
}

