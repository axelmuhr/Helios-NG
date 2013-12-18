/*
 * Picture class implementation.  A Picture is a Graphic that contains other 
 * Graphics.
 */

#include <InterViews/canvas.h>
#include <InterViews/transformer.h>
#include <InterViews/Graphic/grclasses.h>
#include <InterViews/Graphic/picture.h>

Picture::~Picture () {
    while (!refList->IsEmpty()) {
	cur = refList->First();
	refList->Remove(cur);
	delete (*cur)();
	delete cur;
    }
    delete refList;
    uncacheExtent();
}

boolean Picture::read (PFile* f) {
    return FullGraphic::read(f) && refList->Read(f);
}

boolean Picture::readObjects (PFile* f) {
    return FullGraphic::readObjects(f) && refList->ReadObjects(f);
}

boolean Picture::write (PFile* f) {
    return FullGraphic::write(f) && refList->Write(f);
}

boolean Picture::writeObjects (PFile* f) {
    return FullGraphic::writeObjects(f) && refList->WriteObjects(f);
}

boolean Picture::extentCached () { return caching && extent != nil; }

void Picture::uncacheExtent () {
    delete extent; 
    extent = nil;
}

ClassId Picture::GetClassId  () { return PICTURE; }

boolean Picture::IsA (ClassId id) {
    return PICTURE == id || FullGraphic::IsA(id);
}

Picture::Picture (Graphic* gr) : (gr) {
    extent = nil;
    refList = cur = new RefList();
}

void Picture::Append (Graphic* p0, Graphic* p1, Graphic* p2, Graphic* p3) {
    invalidateCachesGraphic(p0);
    refList->Append(new RefList(p0));
    setParent(p0, this);
    if (p1 != nil) {
	invalidateCachesGraphic(p1);
	refList->Append(new RefList(p1));
	setParent(p1, this);
    }
    if (p2 != nil) {
	invalidateCachesGraphic(p2);
	refList->Append(new RefList(p2));
	setParent(p2, this);
    }
    if (p3 != nil) {
	invalidateCachesGraphic(p3);
	refList->Append(new RefList(p3));
	setParent(p3, this);
    }
    uncacheExtent();
    uncacheParents();
}

void Picture::Prepend (Graphic* p0, Graphic* p1, Graphic* p2, Graphic* p3) {
    if (p3 != nil) {
	invalidateCachesGraphic(p3);
	refList->Prepend(new RefList(p3));
	setParent(p3, this);
    }
    if (p2 != nil) {
	invalidateCachesGraphic(p2);
	refList->Prepend(new RefList(p2));
	setParent(p2, this);
    }
    if (p1 != nil) {
	invalidateCachesGraphic(p1);
	refList->Prepend(new RefList(p1));
	setParent(p1, this);
    }
    invalidateCachesGraphic(p0);
    refList->Prepend(new RefList(p0));
    setParent(p0, this);
    uncacheExtent();
    uncacheParents();
}

void Picture::InsertAfterCur (
    Graphic* p0, Graphic* p1, Graphic* p2,Graphic* p3
) {
    invalidateCachesGraphic(p0);
    cur->Prepend(new RefList(p0));
    setParent(p0, this);
    if (p1 != nil) {
	invalidateCachesGraphic(p1);
	cur->Prepend(new RefList(p1));
	setParent(p1, this);
    }
    if (p2 != nil) {
	invalidateCachesGraphic(p2);
	cur->Prepend(new RefList(p2));
	setParent(p2, this);
    }
    if (p3 != nil) {
	invalidateCachesGraphic(p3);
	cur->Prepend(new RefList(p3));
	setParent(p3, this);
    }
    uncacheExtent();
    uncacheParents();
}

void Picture::InsertBeforeCur (
    Graphic* p0, Graphic* p1, Graphic* p2, Graphic* p3
) {
    if (p3 != nil) {
	invalidateCachesGraphic(p3);
	cur->Append(new RefList(p3));
	setParent(p3, this);
    }
    if (p2 != nil) {
	invalidateCachesGraphic(p2);
	cur->Append(new RefList(p2));
	setParent(p2, this);
    }
    if (p1 != nil) {
	invalidateCachesGraphic(p1);
	cur->Append(new RefList(p1));
	setParent(p1, this);
    }
    invalidateCachesGraphic(p0);
    cur->Append(new RefList(p0));
    setParent(p0, this);
    uncacheExtent();
    uncacheParents();
}

Graphic* Picture::First () {
    cur = refList->First(); 
    return getGraphic(cur);
}

Graphic* Picture::Last () {
    cur = refList->Last(); 
    return getGraphic(cur);
}

Graphic* Picture::Next () {
    cur = cur->Next(); 
    return getGraphic(cur);
}

Graphic* Picture::Prev () {
    cur = cur->Prev(); 
    return getGraphic(cur);
}

void Picture::RemoveCur () {
    RefList* prev;

    if (cur != refList) {
	unsetParent((Graphic*) (*cur)());
	prev = cur;
        cur = cur->Next();
	refList->Remove(prev);
	delete prev;
	uncacheExtent();
	uncacheParents();
    }
}	
    
void Picture::Remove (Graphic* p) {
    RefList* temp;

    if (getGraphic(cur) == p) {
        RemoveCur ();
    } else if ((temp = refList->Find(Ref(p))) != nil) {
	unsetParent((Graphic*) (*temp)());
	refList->Remove(temp);
	delete temp;
	uncacheExtent();
	uncacheParents();
    }
}

void Picture::SetCurrent (Graphic* p) {
    RefList* temp;

    if ((temp = refList->Find(Ref(p))) != nil) {
        cur = temp;
    }
}

Graphic* Picture::FirstGraphicContaining (PointObj& pt) {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	if (subgr->Contains(pt)) {
	    return subgr;
	}
    }
    return nil;
}

Graphic* Picture::LastGraphicContaining (PointObj& pt) {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->Last(); i != refList->End(); i = i->Prev()) {
	subgr = getGraphic(i);
	if (subgr->Contains(pt)) {
	    return subgr;
	}
    }
    return nil;
}

int Picture::GraphicsContaining (PointObj& pt, Graphic**& garray) {
    register RefList* i;
    RefList glist;
    Graphic* subgr;
    int size = 0, n = 0;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	if (subgr->Contains(pt)) {
	    glist.Append(new RefList(subgr));
	    ++size;
	}
    }
    if (size != 0) {
	garray = new Graphic*[size];
	for (i = glist.First(); i != glist.End(); i = glist.First(), ++n) {
	    garray[n] = (Graphic*) (*i)();
	    glist.Remove(i);
	    delete i;
	}
    }
    return size;
}

Graphic* Picture::FirstGraphicIntersecting (BoxObj& b) {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	if (subgr->Intersects(b)) {
	    return subgr;
	}
    }
    return nil;
}

Graphic* Picture::LastGraphicIntersecting (BoxObj& b) {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->Last(); i != refList->End(); i = i->Prev()) {
	subgr = getGraphic(i);
	if (subgr->Intersects(b)) {
	    return subgr;
	}
    }
    return nil;
}

int Picture::GraphicsIntersecting (BoxObj& b, Graphic**& garray) {
    register RefList* i;
    RefList glist;
    Graphic* subgr;
    int size = 0, n = 0;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	if (subgr->Intersects(b)) {
	    glist.Append(new RefList(subgr));
	    ++size;
	}
    }
    if (size != 0) {
	garray = new Graphic*[size];
	for (i = glist.First(); i != glist.End(); i = glist.First(), ++n) {
	    garray[n] = (Graphic*) (*i)();
	    glist.Remove(i);
	    delete i;
	}
    }
    return size;
}

Graphic* Picture::FirstGraphicWithin (BoxObj& userb) {
    register RefList* i;
    Graphic* subgr;
    BoxObj b;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	subgr->GetBox(b);
	if (b.Within(userb)) {
	    return subgr;
	}
    }
    return nil;
}

Graphic* Picture::LastGraphicWithin (BoxObj& userb) {
    register RefList* i;
    Graphic* subgr;
    BoxObj b;

    for (i = refList->Last(); i != refList->End(); i = i->Prev()) {
	subgr = getGraphic(i);
	subgr->GetBox(b);
	if (b.Within(userb)) {
	    return subgr;
	}
    }
    return nil;
}

int Picture::GraphicsWithin (BoxObj& userb, Graphic**& garray) {
    register RefList* i;
    RefList glist;
    Graphic* subgr;
    int size = 0, n = 0;
    BoxObj b;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	subgr->GetBox(b);
	if (b.Within(userb)) {
	    glist.Append(new RefList(subgr));
	    ++size;
	}
    }
    if (size != 0) {
	garray = new Graphic*[size];
	for (i = glist.First(); i != glist.End(); i = glist.First(), ++n) {
	    garray[n] = (Graphic*) (*i)();
	    glist.Remove(i);
	    delete i;
	}
    }
    return size;
}

Graphic* Picture::Copy () {
    register RefList* i;
    Picture* newPicture = new Picture(this);
    
    for (i = refList->First(); i != refList->End(); i = i->Next()) {
        newPicture->Append(getGraphic(i)->Copy());
    }
    return newPicture;
}

void Picture::draw (Canvas* c, Graphic* gs) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;

    gstemp.SetTransformer(&ttemp);
    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	gr = getGraphic(i);
	concatGraphic(gr, gr, gs, &gstemp);
	drawGraphic(gr, c, &gstemp);
    }
    gstemp.SetTransformer(nil);	/* to avoid deleting ttemp explicitly */
}

void Picture::drawClipped (
    Canvas* c, Coord l, Coord b, Coord r, Coord t, Graphic* gs
) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    BoxObj box, clipBox(l, b, r, t);
    
    getBox(box, gs);
    if (clipBox.Intersects(box)) {
	gstemp.SetTransformer(&ttemp);
	for (i = refList->First(); i != refList->End(); i = i->Next()) {
	    gr = getGraphic(i);
	    concatGraphic(gr, gr, gs, &gstemp);
	    drawClippedGraphic(gr, c, l, b, r, t, &gstemp);
	}
	gstemp.SetTransformer(nil); /* to avoid deleting ttemp explicitly */
    }
}

void Picture::cacheExtent (float l, float b, float cx, float cy, float tol) {
    if (caching) {
	uncacheExtent();
	extent = new Extent(l, b, cx, cy, tol);
    }
}

void Picture::uncacheChildren () {
    register RefList* i;
    Graphic* subgr;

    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	subgr = getGraphic(i);
	uncacheExtentGraphic(subgr);
	uncacheChildrenGraphic(subgr);
    }
}

void Picture::getCachedExtent (
    float& l, float& b, float& cx, float& cy, float& tol
) {
    l = extent->left;
    b = extent->bottom;
    cx = extent->cx;
    cy = extent->cy;
    tol = extent->tol;
}

void Picture::getExtent (
    float& l, float& b, float& cx, float& cy, float& tol, Graphic* gs
) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    float right, top, dummy;
    Extent e, te;
    
    if (extentCached()) {
	getCachedExtent(e.left, e.bottom, e.cx, e.cy, e.tol);

    } else {
	if (IsEmpty()) {
	    l = b = cx = cy = tol = 0.0;
	    return;
	} else {
            gstemp.SetTransformer(&ttemp);
	    i = refList->First();
	    gr = getGraphic(i);
	    concatGSGraphic(gr, gr, gs, &gstemp);
            concatTransformerGraphic(gr, nil, gr->GetTransformer(), &ttemp);
	    getExtentGraphic(gr, e.left, e.bottom, e.cx, e.cy, e.tol, &gstemp);
	    for (i = i->Next(); i != refList->End(); i = i->Next()) {
		gr = getGraphic(i);
		concatGSGraphic(gr, gr, gs, &gstemp);
                concatTransformerGraphic(gr,nil, gr->GetTransformer(), &ttemp);
		getExtentGraphic(
                    gr, te.left, te.bottom, te.cx, te.cy, te.tol, &gstemp
                );
		e.Merge(te);
	    }
	    cacheExtent(e.left, e.bottom, e.cx, e.cy, e.tol);
            gstemp.SetTransformer(nil); // to avoid deleting ttemp explicitly
	}
    }
    right = 2*e.cx - e.left;
    top = 2*e.cy - e.bottom;
    transformRect(e.left, e.bottom, right, top, l, b, dummy, dummy, gs);
    transform(e.cx, e.cy, cx, cy, gs);
    tol = e.tol;
}

boolean Picture::contains (PointObj& po, Graphic* gs) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    BoxObj b;
    
    if (!IsEmpty()) {
	getBox(b, gs);
	if (b.Contains(po)) {
	    gstemp.SetTransformer(&ttemp);
	    for (i = refList->First(); i != refList->End(); i = i->Next()) {
		gr = getGraphic(i);
		concatGraphic(gr, gr, gs, &gstemp);
		if (containsGraphic(gr, po, &gstemp)) {
		    gstemp.SetTransformer(nil);
		    return true;
		}
	    }
	    gstemp.SetTransformer(nil); /* to avoid deleting ttemp explicitly*/
	}
    }
    return false;
}

boolean Picture::intersects (BoxObj& userb, Graphic* gs) {
    register RefList* i;
    Graphic* gr;
    FullGraphic gstemp;
    Transformer ttemp;
    BoxObj b;
    
    if (!IsEmpty()) {
	getBox(b, gs);
	if (b.Intersects(userb)) {
	    gstemp.SetTransformer(&ttemp);
	    for (i = refList->First(); i != refList->End(); i = i->Next()){
		gr = getGraphic(i);
		concatGraphic(gr, gr, gs, &gstemp);
		if (intersectsGraphic(gr, userb, &gstemp)) {
		    gstemp.SetTransformer(nil);
		    return true;
		}
	    }
	    gstemp.SetTransformer(nil); /* to avoid deleting ttemp explicitly*/
	}
    }
    return false;
}

boolean Picture::HasChildren () { return !IsEmpty(); }

void Picture::Propagate () {
    register RefList* i;
    Graphic* gr;
    
    for (i = refList->First(); i != refList->End(); i = i->Next()) {
	gr = getGraphic(i);
	concatGraphic(gr, gr, this, gr);
    }
    Graphic null;
    *((Graphic*) this) = null;
}
