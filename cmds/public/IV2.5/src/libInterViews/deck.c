/*
 * Deck - a Scene for stacking Interactors
 */

#include <InterViews/deck.h>
#include <InterViews/perspective.h>
#include <InterViews/glue.h>

class Card {
public:
    Interactor* i;
    Card* next;
    Card* prev;
    Card (Interactor* ii = nil) { i = ii; next = prev = this; }
    ~Card () { next->prev = prev; prev->next = next; }
};

Deck::Deck () {
    Init();
}

Deck::Deck (const char* name) {
    SetInstance(name);
    Init();
}

Deck::Deck (Painter* out) : (nil, out) {
    Init();
}

void Deck::Init () {
    SetClassName("Deck");
    cards = new Card;
    register Perspective* p = new Perspective;
    perspective = p;
    p->sx = p->lx = 1;
    p->sy = p->ly = 1;
    p->curwidth = p->curheight = 1;
    p->width = p->height = 0;
    p->x0 = 1;
    p->y0 = 0;
}

Deck::~Deck () {
    register Card* c, * next;

    for (c = cards->next; c != cards; c = next) {
	next = c->next;
	delete c->i;
	delete c;
    }
    delete cards;
    delete perspective;
}
    
void Deck::Reconfig () {
    int hnat = 0, hmin = 0, hmax = hfil;
    int vnat = 0, vmin = 0, vmax = vfil;
    Card* card = cards->next;
    while (card != cards) {
	Shape* s = card->i->GetShape();
	hnat = max(hnat, s->width);
	hmin = max(hmin, s->width - s->hshrink);
	hmax = min(hmax, s->width + s->hstretch);
	vnat = max(vnat, s->height);
	vmin = max(vmin, s->height - s->vshrink);
	vmax = min(vmax, s->height + s->vstretch);
	card = card->next;
    }
    shape->width = hnat;
    shape->hshrink = max(0, shape->width - hmin);
    shape->hstretch = max(0, hmax - shape->width);
    shape->height = vnat;
    shape->vshrink = max(0, shape->height - vmin);
    shape->vstretch = max(0, vmax - shape->height);
    FixPerspective();
}

void Deck::FixPerspective () {
    register Perspective* p = perspective;
    p->curx = max(p->x0, min(p->width, p->curx));
    p->cury = p->y0 + p->height - (p->curx - p->x0) - p->curheight;
    p->Update();
}

void Deck::NewTop () {
    Card* card = cards;
    for (int i = perspective->curx; i > 0; --i) {
	card = card->next;
    }
    if (top != nil && card->i != top) {
	Map(card->i);
	Unmap(top);
	top = card->i;
    }
}

void Deck::DoInsert (Interactor* i, boolean, Coord&, Coord&) {
    if (i != nil) {
	Card* c = new Card(i);
	c->prev = cards->prev;
	c->next = cards;
	cards->prev->next = c;
	cards->prev = c;
        ++perspective->width;
        ++perspective->height;
        FixPerspective();
    }
}

void Deck::DoRemove (Interactor* i) {
    Card* card = cards->next;
    while (card != cards) {
	if (card->i == i) {
            card->prev->next = card->next;
            card->next->prev = card->prev;
	    delete card;
            --perspective->width;
            --perspective->height;
            FixPerspective();
	    break;
	} else {
	    card = card->next;
	}
    }
}

void Deck::Resize () {
    int pos = 1;
    Card* card = cards->next;
    while (card != cards) {
	Interactor* i = card->i;
	Shape* s = i->GetShape();
	int l, r, b, t;
	int width = xmax+1;
	width = max(width, s->width - s->hshrink);
	width = min(width, s->width + s->hstretch);
	int height = ymax+1;
	height = max(height, s->height - s->vshrink);
	height = min(height, s->height + s->vstretch);
	l = (xmax+1-width)/2; r = xmax - l;
	b = (ymax+1-height)/2; t = ymax - b;
	if (pos == perspective->curx) {
	    top = i;
	    Place(i, l, b, r, t, true);
	} else {
	    Place(i, l, b, r, t, false);
	}
	card = card->next;
	pos += 1;
    }
}

void Deck::Draw () {
    if (top != nil) {
	top->Draw();
    }
}

void Deck::GetComponents (Interactor** c, int nc, Interactor**& a, int& n) {
    register Card* card;
    register Interactor** ap;

    n = perspective->width;
    if (n > 0) {
	a = (n <= nc) ? c : new Interactor*[n];
	ap = a;
	for (card = cards->next; card != cards; card = card->next) {
	    *ap++ = card->i;
	}
    }
}

void Deck::Adjust (Perspective& np) {
    Perspective* p = perspective;
    int nx = round(float(np.curx-np.x0) / float(np.width) * float(p->width));
    int ny = round(
        float(np.y0 + np.height - np.cury - np.curheight)
        / float(np.height)
        * float(p->height)
    );
    if (nx != p->curx - p->x0) {
        p->curx = p->x0 + nx;
    } else if (ny != p->curx - p->x0) {
        p->curx = p->x0 + ny;
    }
    FixPerspective();
    np = *p;
    NewTop();
}

void Deck::Flip (int count) {
    perspective->curx += count;
    FixPerspective();
    NewTop();
}

void Deck::FlipTo (int position) {
    if (position > 0) {
	perspective->curx = position;
    } else if (position < 0) {
	perspective->curx = perspective->width + 1 + position;
    }
    FixPerspective();
    NewTop();
}
