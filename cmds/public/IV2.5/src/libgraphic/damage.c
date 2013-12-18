/*
 * Damage class implementation.
 */

#include <InterViews/canvas.h>
#include <InterViews/Graphic/base.h>
#include <InterViews/Graphic/damage.h>
#include <InterViews/Graphic/geomobjs.h>

class BoxList {
    int n;
    BoxList* next;
    BoxList* prev;
public:
    BoxObj* box;

    BoxList(BoxObj* =nil);
    ~BoxList();
    
    void Append(BoxObj*);
    void Prepend(BoxObj*);
    void Delete(BoxList*);
    BoxList* Next() { return next; }
    BoxList* Prev() { return prev; }
    BoxList* End() { return this; }
    boolean IsEmpty() { return next == this; }
    int Number() { return n; }
};

BoxList::BoxList (BoxObj* b) {
    box = b;
    next = this;
    prev = this;
    n = 0;
}

BoxList::~BoxList () {
    BoxList* doomed;

    while (next != this) {
	doomed = next;
	next = doomed->next;
	doomed->next = doomed;
	delete doomed;
    }
}

void BoxList::Append (BoxObj* b) {
    BoxList* cur = new BoxList(b);

    cur->next = this;
    cur->prev = prev;
    prev->next = cur;
    prev = cur;
    ++n;
}

void BoxList::Prepend (BoxObj* b) {
    BoxList* cur = new BoxList(b);
    
    cur->next = next;
    cur->prev = this;
    next->prev = cur;
    next = cur;
    ++n;
}

void BoxList::Delete (BoxList* elem) {
    elem->prev->next = elem->next;
    elem->next->prev = elem->prev;
    elem->next = elem;
    delete elem;
    --n;
}

Damage::Damage (Canvas* c, Painter* p, Graphic* g) {
    areas = new BoxList;
    additions = new RefList;
    canvas = c;
    output = p;
    if (output != nil) {
        output->Reference();
    }
    graphic = g;
}

Damage::~Damage () {
    delete output;
    delete areas;
    delete additions;
}

int Damage::Area (BoxObj& b) {
    return (b.right - b.left) * (b.top - b.bottom);
}

void Damage::DrawAreas () {
    register BoxList* a;
    BoxObj visible(0, 0, canvas->Width() - 1, canvas->Height() - 1);
    BoxObj b;

    for (a = areas->Next(); a != areas->End(); a = a->Next()) {
        b = *a->box - visible;
	output->ClearRect(canvas, b.left, b.bottom, b.right, b.top);
	graphic->DrawClipped(canvas, b.left, b.bottom, b.right, b.top);
    }
}    

void Damage::DrawAdditions () {
    RefList* cur;
    Graphic* gr;
    Coord xmax = canvas->Width() - 1;
    Coord ymax = canvas->Height() - 1;
    
    for (cur = additions->First(); cur != additions->End(); cur = cur->Next()){
	gr = (Graphic*) (*cur)();
	gr->Draw(canvas, 0, 0, xmax, ymax);
    }
}

void Damage::Merge (BoxObj& newb) {
    BoxList* elem1, *elem2;
    BoxObj* a1, *a2;
    int newArea, area1, area2, diff1, diff2, diff3, maximum;

    elem1 = areas->Next();
    elem2 = elem1->Next();
    a1 = elem1->box;
    a2 = elem2->box;
    BoxObj merge1(*a1 + newb);
    BoxObj merge2(*a2 + newb);
    BoxObj merge3(*a1 + *a2);

    newArea = Area(newb);
    area1 = Area(*a1);
    area2 = Area(*a2);
    diff1 = area1 + newArea - Area(merge1);
    diff2 = area2 + newArea - Area(merge2);
    diff3 = area1 + area2 - Area(merge3);
    maximum = max(max(diff1, diff2), diff3);
    if (maximum == diff1) {
	if (a2->Intersects(merge1)) {
	    *a1 = merge1 + *a2;
	    areas->Delete(elem2);
	} else {
	    *a1 = merge1;
	}
    } else if (maximum == diff2) {
	if (a1->Intersects(merge2)) {
	    *a2 = merge2 + *a1;
	    areas->Delete(elem1);
	} else {
	    *a2 = merge2;
	}
    } else {
	if (newb.Intersects(merge3)) {
	    *a1 = merge3 + newb;
	    areas->Delete(elem2);
	} else {
	    *a1 = merge3;
	    *a2 = newb;
	}
    }
}

void Damage::Added (Graphic* g) {
    additions->Append(new RefList(g));
}

boolean Damage::Incurred () {
    return !areas->IsEmpty() || !additions->IsEmpty();
}

void Damage::Incur (Graphic* g) {
    BoxObj box;

    g->GetBox(box);
    Incur(box);
}

void Damage::Incur (Coord l, Coord b, Coord r, Coord t) {
    Incur(BoxObj(l, b, r, t));
}

void Damage::Incur (BoxObj& newb) {
    BoxObj* b;

    if (areas->Number() == 0) {
	areas->Prepend(new BoxObj(&newb));

    } else if (areas->Number() == 1) {
	b = areas->Next()->box;
	if (newb.Intersects(*b)) {
	    if (!newb.Within(*b)) {
		*b = *b + newb;
	    }
	} else {
	    areas->Prepend(new BoxObj(&newb));
	}
    } else {
	Merge(newb);
    }
}

void Damage::Repair () {
    DrawAreas();
    DrawAdditions();
    Reset();
}

void Damage::Reset () {
    delete areas;
    areas = new BoxList;
    delete additions;
    additions = new RefList;
}

void Damage::SetCanvas (Canvas* c) {
    canvas = c;
}

void Damage::SetPainter (Painter* p) {
    delete output;
    output = p;
    output->Reference();
}

void Damage::SetGraphic (Graphic* gr) {
    graphic = gr;
}

Canvas* Damage::GetCanvas () {
    return canvas;
}

Painter* Damage::GetPainter () {
    return output;
}

Graphic* Damage::GetGraphic () {
    return graphic;
}
