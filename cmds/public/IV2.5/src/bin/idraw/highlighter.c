// $Header: highlighter.c,v 1.8 89/04/17 00:30:33 linton Exp $
// implements classes HighlighterParent and Highlighter.

#include "highlighter.h"
#include <InterViews/painter.h>

// HighlighterParent starts with no highlight painter.

HighlighterParent::HighlighterParent () {
    highlight = nil;
}

// Free storage allocated for the highlight painter.

HighlighterParent::~HighlighterParent () {
    delete highlight;
}

// SameOutputAs compares the given painter to our output painter so
// our interior Highlighters can decide if they can share our
// highlight painter.

boolean HighlighterParent::SameOutputAs (Painter* out) {
    return out == output;
}

// GetHighlightPainter creates our highlight painter if we don't have
// one yet and returns it so all of our interior Highlighters can
// share it as well as our output painter which they inherit
// automatically.  We can't just create highlight in Reconfig because
// our interior Highlighters execute their Reconfig before we do, so
// we create it here (once) when they call us from their Reconfig.

Painter* HighlighterParent::GetHighlightPainter () {
    if (highlight == nil) {
	highlight = new Painter(output);
	highlight->SetColors(output->GetBgColor(), output->GetFgColor());
    }
    return highlight;
}

// Highlighter starts off unhighlighted with no HighlighterParent yet.

Highlighter::Highlighter () {
    hparent = nil;
    highlighted = false;
    highlight = nil;
    normal = nil;
}

// Free storage allocated for the highlight painter.

Highlighter::~Highlighter () {
    output = normal;
    delete highlight;
}

// SetHighlighterParent gives us a HighlighterParent.

void Highlighter::SetHighlighterParent (HighlighterParent* hp) {
    hparent = hp;
}

// Highlight switches to our highlight painter and draws us unless we
// don't have a canvas so a panel can highlight us before the panel's
// inserted.

void Highlighter::Highlight () {
    highlighted = true;
    output = highlight;
    if (canvas != nil) {
	Redraw(0, 0, xmax, ymax);
    }
}

// Unhighlight switches to our normal painter and draws us unless we
// don't have a canvas so if we're a menu entry we can unhighlight
// ourself after the menu's removed.

void Highlighter::Unhighlight () {
    highlighted = false;
    output = normal;
    if (canvas != nil) {
	Redraw(0, 0, xmax, ymax);
    }
}

// Reconfig initializes our highlight painter if necessary by getting
// it from our HighlighterParent if possible or else creating a new
// painter.  Then Reconfig switches to the appropriate painter.

void Highlighter::Reconfig () {
    Interactor::Reconfig();
    if (output != highlight && output != normal) {
	delete highlight;
	if (hparent != nil && hparent->SameOutputAs(output)) {
	    highlight = hparent->GetHighlightPainter();
	    highlight->Reference();
	} else {		// bite the bullet
	    highlight = new Painter(output);
	    highlight->SetColors(output->GetBgColor(), output->GetFgColor());
	}
	normal = output;
    }
    output = highlighted ? highlight : normal;
}
