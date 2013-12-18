/*
 * Tray implementation.
 */

#include <InterViews/canvas.h>
#include <InterViews/painter.h>
#include <InterViews/shape.h>
#include <InterViews/tray.h>

inline float fmax (float f1, float f2) { return (f1 > f2) ? f1 : f2; }
inline float fmin (float f1, float f2) { return (f1 < f2) ? f1 : f2; }
inline float abs (float f) { return (f < 0) ? -f : f; }

/*************************************************************************/

class TElement {
public:
    TElement(Interactor*);
    TElement(TGlue*);
    TElement();
    ~TElement();

    float pos, sigma;			/* ultimate values */
    float nat, stretch, shrink;
    boolean combinable;		    /* false for tray elements when calcing */
				    /* shape of tray having no background */
    boolean leftBotHalf;	    /* true if represents left/bottom of a */
				    /* pair of elems modelling an interactor */
    Interactor* owner;
    TGlue* tglue;

    void HSetShape();
    void VSetShape();
    TElement* Series(TElement*);	/* series combination */
    TElement* Parallel(TElement*);	/* parallel combination */
    void Reverse();
    void Limit();
};

TElement::TElement (Interactor* i) {
    owner = i;
    tglue = nil;
    pos = sigma = 0;
}

TElement::TElement (TGlue* tg) {
    owner = nil;
    tglue = tg;
    pos = sigma = 0;
}

TElement::TElement () {
    owner = nil;
    tglue = nil;
    pos = sigma = 0;
}

TElement::~TElement () {
    delete tglue;
}

void TElement::HSetShape () {
    Shape* s;

    if (tglue == nil) {
	s = owner->GetShape();
	nat = float(s->width)/2;
	stretch = float(s->hstretch)/2;
	shrink = float(s->hshrink)/2;
    } else {
	s = tglue->GetShape();
	nat = s->width;
	stretch = s->hstretch;
	shrink = s->hshrink;
    }
}

void TElement::VSetShape () {
    Shape* s;

    if (tglue == nil) {
	s = owner->GetShape();
	nat = float(s->height)/2;
	stretch = float(s->vstretch)/2;
	shrink = float(s->vshrink)/2;
    } else {
	s = tglue->GetShape();
	nat = s->height;
	stretch = s->vstretch;
	shrink = s->vshrink;
    }
}

TElement* TElement::Series (TElement* e) {
    TElement* combo = new TElement;
    register TElement* c;
    
    combo->combinable = combinable || e->combinable;
    
    if (combinable && e->combinable) {
	combo->nat = nat + e->nat;
	combo->stretch = stretch + e->stretch;
	combo->shrink = shrink + e->shrink;

    } else if (combo->combinable) {
	c = combinable ? this : e;
	combo->nat = c->nat;
	combo->stretch = c->stretch;
	combo->shrink = c->shrink;
    }
    
    return combo;
}

TElement* TElement::Parallel (TElement* e) {
    TElement* combo = new TElement;
    register TElement* c;
    float emin, emax;
    
    emin = fmax(nat - shrink, e->nat - e->shrink);
    emax = fmin(nat + stretch, e->nat + e->stretch);
    combo->combinable = combinable || e->combinable;

    if (combinable && e->combinable) {
	combo->nat = fmax(nat, e->nat);
	combo->stretch = fmax(0, emax - combo->nat);
	combo->shrink = fmax(0, combo->nat - emin);

    } else if (combo->combinable) {
	c = combinable ? this : e;
	combo->nat = c->nat;
	combo->stretch = c->stretch;
	combo->shrink = c->shrink;
    }
	
    return combo;
}

void TElement::Reverse () {
    float tmp;

    nat = -nat;
    tmp = stretch;
    stretch = shrink;
    shrink = tmp;
    sigma = -sigma;
    pos -= nat + sigma + 1;
}

void TElement::Limit () {
    sigma = fmin(fmax(-shrink, sigma), stretch);
}

/*************************************************************************/

class TList {
public:
    ~TList();
protected:
    TList(void* = nil);

    void Append(TList*);
    void Prepend(TList*);
    void Remove(TList*);
    TList* First();
    TList* Last();
    TList* End();
    TList* Next();
    TList* Prev();
    boolean Empty();

    void SetContents(void*);
    void* GetContents();
    void Delete(void*);
    TList* Find(void*);
private:
    void* object;
    TList* next;
    TList* prev;
};

inline TList::TList (void* o) { next = this; prev = this; object = o; }
inline TList* TList::First () { return next; }
inline TList* TList::Last () { return prev; }
inline TList* TList::End () { return this; }
inline TList* TList::Next () { return next; }
inline TList* TList::Prev () { return prev; }
inline boolean TList::Empty () { return next == this; }
inline void TList::SetContents (void *o) { object = o; }
inline void* TList::GetContents () { return object; }

inline void TList::Append (TList* e) {
    prev->next = e;
    e->prev = prev;
    e->next = this;
    prev = e;
}

inline void TList::Prepend (TList* e) {
    next->prev = e;
    e->prev = this;
    e->next = next;
    next = e;
}

inline void TList::Remove (TList* e) {
    e->prev->next = e->next;
    e->next->prev = e->prev;
    e->next = e->prev = e;
}

TList::~TList () {
    TList* e;
    
    if (!Empty()) {
	e = First();
	Remove(this);
	delete e;
    }
}

void TList::Delete (void* o) {
    register TList* e;

    e = Find(o);
    if (e != nil) {
	Remove(e);
	delete e;
    }
}

TList* TList::Find (void* o) {
    register TList* e;

    for (e = next; e != this; e = e->next) {
	if (e->GetContents() == o) {
	    return e;
	}
    }
    return nil;
}

/*************************************************************************/

class TElementList : public TList {
public:
    TElementList(TElement* = nil);
    TElementList* Copy();

    boolean Includes(TElement*);
    boolean Includes(Interactor*, TElement*&);

    void Append(TElementList*);
    void Remove(TElementList*);
    TElementList* First();
    TElementList* End();
    TElementList* Next();
    void Delete(TElement*);
    boolean Empty();
    boolean OnlyOne();
    boolean OnlyTwo();
    TElement* GetElem();
};

inline TElement* TElementList::GetElem () { return (TElement*) GetContents();}
inline void TElementList::Append (TElementList* el) { TList::Append(el); }
inline void TElementList::Remove (TElementList* el) { TList::Remove(el); }
inline void TElementList::Delete (TElement* o) { TList::Delete(o); }
inline boolean TElementList::Includes (TElement* e) { return Find(e) != nil; }
inline TElementList* TElementList::First () 
    { return (TElementList*) TList::First(); }
inline TElementList* TElementList::End ()
    { return (TElementList*) TList::End(); }
inline TElementList* TElementList::Next () 
    { return (TElementList*) TList::Next(); }
inline boolean TElementList::Empty () { return TList::Empty(); }
inline boolean TElementList::OnlyOne () { return !Empty() && First()==Last(); }
inline boolean TElementList::OnlyTwo ()
    { return !Empty() && !OnlyOne() && First()->Next() == Last(); }

TElementList::TElementList (TElement* o) : (o) { }

TElementList* TElementList::Copy () {
    register TElementList* t;
    TElementList* newlist = new TElementList;
    
    for (t = First(); t != End(); t = t->Next()) {
	newlist->Append(new TElementList(t->GetElem()));
    }
    return newlist;
}

boolean TElementList::Includes (Interactor* i, TElement*& e) {
    register TElementList* t;

    for (t = First(); t != End(); t = t->Next()) {
	e = t->GetElem();
	if (e->owner == i) {
	    return true;
	}
    }
    return false;
}

/*************************************************************************/

class TTermination {
public:
    Alignment alignment, toAttached;
    TElement* dangling, *attached;

    TTermination(Alignment, TElement*, Alignment, TElement*);
};

TTermination::TTermination (
    Alignment a, TElement* d, Alignment to, TElement* e
) {
    alignment = a;
    dangling = d;
    toAttached = to;
    attached = e;
}

/*************************************************************************/

class TLoop {
public:
    Alignment toAttached;
    TElement* looped, *attached;
    
    TLoop(TElement*, Alignment, TElement*);
};

TLoop::TLoop (TElement* l, Alignment to, TElement* e) {
    looped = l;
    toAttached = to;
    attached = e;
}    

/*************************************************************************/

class TNode {
    friend class TNodeList;
public:
    TNode(TElementList* = nil, TElementList* = nil);
    TNode(Alignment, TElement*, Alignment = BottomLeft, TElement* = nil);
    ~TNode();
    TNode* Copy();
    void DeleteElements();    

    void Merge(TNode*);
    void Exclude(TElement*);
    boolean Includes(TElement*);
    boolean Includes(Alignment&, TElement*);
    boolean Overlaps(TNode*);
    boolean Empty();
    boolean Degenerate(TElement*&);
    boolean Degenerate(Alignment&, TElement*&);
    boolean Series(TElement*&, TElement*&);
    boolean Stub(TElement*&);
    boolean Loop(TElement*&);

    void SetPosition(float);
    float GetPosition();
private:
    TElementList* lbElems, *rtElems;
    float position;

    TElementList* LeftBottomElements();
    TElementList* RightTopElements();
    void DeleteElements(TElementList*);
};

inline boolean TNode::Includes (TElement* e) 
    { Alignment dummy; return Includes(dummy, e); }
inline boolean TNode::Empty () { return lbElems->Empty() && rtElems->Empty(); }
inline void TNode::SetPosition (float p) { position = p; }
inline float TNode::GetPosition () { return position; }
inline TElementList* TNode::LeftBottomElements () { return lbElems; }
inline TElementList* TNode::RightTopElements () { return rtElems; }

TNode::TNode (TElementList* l1, TElementList* l2) {
    lbElems = l1;
    rtElems = l2;
    position = 0;
}
    
TNode::TNode (Alignment a1, TElement* e1, Alignment a2, TElement* e2) {
    lbElems = new TElementList;
    rtElems = new TElementList;

    if (a1 == BottomLeft) {
	rtElems->Append(new TElementList(e1));
    } else {
	lbElems->Append(new TElementList(e1));
    }
    if (e2 != nil) {
	if (a2 == BottomLeft) {
	    rtElems->Append(new TElementList(e2));
	} else {
	    lbElems->Append(new TElementList(e2));
	}
	position = 0;
    }
}

TNode::~TNode () { delete lbElems; delete rtElems; }

void TNode::DeleteElements () {
    register TElementList* t;
    for (t = lbElems->First(); t != lbElems->End(); t = t->Next()) {
        rtElems->Delete(t->GetElem());
    }
    DeleteElements(lbElems);
    DeleteElements(rtElems);
}

void TNode::DeleteElements (TElementList* elems) {
    register TElementList* t;
    for (t = elems->First(); t != elems->End(); t = t->Next()) {
        delete t->GetElem();
    }
}

TNode* TNode::Copy () { 
    TNode* node = new TNode(lbElems->Copy(), rtElems->Copy());
    node->SetPosition(GetPosition());
    return node;
}

void TNode::Merge (TNode* n) {
    register TElementList* nelems, *next;
    TElementList* cur;
    
    nelems = n->lbElems;
    for (cur = nelems->First(); cur != nelems->End(); cur = next) {
	next = cur->Next();
	nelems->Remove(cur);
	if (lbElems->Includes(cur->GetElem())) {
            delete cur;
        } else {
	    lbElems->Append(cur);
	}
    }
    nelems = n->rtElems;
    for (cur = nelems->First(); cur != nelems->End(); cur = next) {
	next = cur->Next();
	nelems->Remove(cur);
	if (rtElems->Includes(cur->GetElem())) {
            delete cur;
        } else {
	    rtElems->Append(cur);
	}
    }
}

void TNode::Exclude (TElement* e) {
    lbElems->Delete(e);
    rtElems->Delete(e);
}

boolean TNode::Includes (Alignment& a, TElement* e) {
    if (lbElems->Includes(e)) {
	a = TopRight;
	return true;
    } else if (rtElems->Includes(e)) {
	a = BottomLeft;
	return true;
    };
    return false;
}

boolean TNode::Overlaps (TNode* n) {
    register TElementList* nelems;
    register TElementList* cur;
    
    nelems = n->lbElems;
    for (cur = nelems->First(); cur != nelems->End(); cur = cur->Next()) {
	if (lbElems->Includes(cur->GetElem())) {
	    return true;
	}
    }
    nelems = n->rtElems;
    for (cur = nelems->First(); cur != nelems->End(); cur = cur->Next()) {
	if (rtElems->Includes(cur->GetElem())) {
	    return true;
	}
    }
    return false;
}

boolean TNode::Degenerate (TElement*& e) {
    Alignment dummy; 

    return Degenerate(dummy, e);
}

boolean TNode::Degenerate (Alignment& a, TElement*& e) {
    if (!lbElems->Empty() && rtElems->Empty()) {
	if (lbElems->OnlyOne()) {
	    e = lbElems->First()->GetElem();
	    a = TopRight;
	    return true;
	}
    } else if (lbElems->Empty() && !rtElems->Empty()) {
	if (rtElems->OnlyOne()) {
	    e = rtElems->First()->GetElem();
	    a = BottomLeft;
	    return true;
	}
    }
    return false;
}

boolean TNode::Series (TElement*& e1, TElement*& e2) {
    if (
	!lbElems->Empty() && !rtElems->Empty() &&
	lbElems->OnlyOne() && rtElems->OnlyOne()
    ) {
	e1 = lbElems->First()->GetElem();
	e2 = rtElems->First()->GetElem();
	if (e1 != e2) {
	    return true;
	}
    }
    return false;
}

boolean TNode::Stub (TElement*& e) {
    if (lbElems->OnlyTwo() && rtElems->Empty()) {
	e = lbElems->First()->GetElem();
	return true;

    } else if (lbElems->Empty() && rtElems->OnlyTwo()) {
	e = rtElems->First()->GetElem();
	return true;
    }
    return false;
}

boolean TNode::Loop (TElement*& e) {
    register TElementList* cur;
    
    for (cur = lbElems->First(); cur != lbElems->End(); cur = cur->Next()) {
	e = cur->GetElem();
	if (rtElems->Includes(e)) {
	    return true;
	}
    }
    return false;
}

/*************************************************************************/

class TNodeList : public TList {
public:
    TNodeList(TNode* = nil);
    TNodeList* Copy();

    void Include(Alignment, TElement*, Alignment = BottomLeft, TElement* =nil);
    void Exclude(TElement*);
    boolean Includes(TNode*);
    void Nodes(TElement*, TNode*&, TNode*&);
    TNode* Node(Alignment, TElement*);
    TNode* OtherNode(TElement*, TNode*);
    void AddMissingNodes(TElement*);

    void Append(TNodeList*);
    void Remove(TNodeList*);
    TNodeList* First();
    TNodeList* End();
    TNodeList* Next();
    TNodeList* Last();
    void Delete(TNode*);
    boolean Empty();
    boolean OnlyOne();
    TNode* GetNode();
    
    boolean Degenerate(TElement*&);
    boolean FoundTermination(TTermination*&, TNode*, TNode*);
    boolean FoundSeries(TElement*&, TElement*&, TNode*, TNode*);
    boolean FoundStub(TElement*&);
    boolean FoundParallel(TElement*&, TElement*&);
    boolean FoundCrossover(TElement*&);
    boolean FoundLoop(TLoop*&);
    void Reverse(TElement*);
    
    void RemoveTermination(TTermination*);
    void RemoveSeries(TElement*, TElement*, TElement*);
    void RemoveParallel(TElement*, TElement*, TElement*);
    void RemoveLoop(TLoop*);

    void ReplaceTermination(TTermination*);
    void ReplaceSeries(TElement*, TElement*, TElement*);
    void ReplaceParallel(TElement*, TElement*, TElement*);
    void ReplaceLoop(TLoop*);

    void ApplyToTermination(TTermination*);
    void ApplyToSeries(TElement*, TElement*, TElement*);
    void ApplyToParallel(TElement*, TElement*, TElement*);
    void ApplyToLoop(TLoop*);

    void FindElements(Interactor*, TElement*&, TElement*&);
    void FindElement(TGlue*, TElement*&);
private:
    boolean FoundParallel(TNode*, TElement*&, TElement*&);
    boolean FoundParallel(TElementList*, TNode*, TElement*&, TElement*&);
    boolean FoundCrossover(TNode*, TElement*&);
    void FindElements(TElementList*, Interactor*, TElement*&, TElement*&);
    void FindElement(TElementList*, TGlue*, TElement*&);
    Alignment Inverse(Alignment);
    void GetElemOtherThan(TElement*, TNode*, Alignment&, TElement*&);
};

inline boolean TNodeList::Includes (TNode* t) { return TList::Find(t) != nil; }
inline TNode* TNodeList::GetNode () { return (TNode*) GetContents(); }
inline boolean TNodeList::FoundParallel (
    TNode* n, TElement*& e1, TElement*& e2
) {
    return    
	FoundParallel(n->LeftBottomElements(), n, e1, e2) ||
	FoundParallel(n->RightTopElements(), n, e1, e2);
}
inline void TNodeList::Append (TNodeList* t) { TList::Append(t); }
inline void TNodeList::Remove (TNodeList* t) { TList::Remove(t); }
inline void TNodeList::Delete (TNode* o) { TList::Delete(o); }
inline TNodeList* TNodeList::First () { return (TNodeList*) TList::First(); }
inline TNodeList* TNodeList::End () { return (TNodeList*) TList::End(); }
inline TNodeList* TNodeList::Next () { return (TNodeList*) TList::Next(); }
inline TNodeList* TNodeList::Last () { return (TNodeList*) TList::Last(); }
inline boolean TNodeList::Empty () { return TList::Empty(); }
inline boolean TNodeList::OnlyOne () { return !Empty() && First() == Last(); }
inline Alignment TNodeList::Inverse (Alignment a)
    { return (a == BottomLeft) ? TopRight : BottomLeft; }

TNodeList::TNodeList (TNode* o) : (o) { }

TNodeList* TNodeList::Copy () {
    register TNodeList* t;
    TNodeList* newlist = new TNodeList;
    TNode* node;
    
    for (t = First(); t != End(); t = t->Next()) {
	node = t->GetNode();
	newlist->Append(new TNodeList(node->Copy()));
    }
    return newlist;
}

void TNodeList::Include (
    Alignment a1, TElement* e1, Alignment a2, TElement* e2
) {
    register TNodeList* t;
    TNode pass1(a1, e1, a2, e2);
    TNode* node, *pass2;
    
    for (t = First(); t != End(); t = t->Next()) {
	node = t->GetNode();
	if (node->Overlaps(&pass1)) {
	    node->Merge(&pass1);
	    pass2 = node;
	    break;
	}
    }
    if (t == End()) {
	Append(new TNodeList(pass1.Copy()));

    } else {
	for (t = First(); t != End(); t = t->Next()) {
	    node = t->GetNode();
	    if (node != pass2 && node->Overlaps(pass2)) {
		node->Merge(pass2);
		Delete(pass2);
		delete pass2;
		break;
	    }
	}
    }
}

void TNodeList::Exclude (TElement* e) {
    register TNodeList* t, *next;
    TNode* node;
    int n = 0;
    
    for (t = First(); t != End() && n <= 1; t = next) {
	next = t->Next();
	node = t->GetNode();
	if (node->Includes(e)) {
	    ++n;
	    node->Exclude(e);
	    if (node->Empty()) {
		Remove(t);
		delete t;
	    }
	}
    }
}

boolean TNodeList::Degenerate (TElement*& e) {
    TElement* alt;
    TNode* nfirst = First()->GetNode();
    TNode* nlast = Last()->GetNode();

    return
	First()->Next() == Last() &&
	nfirst->Degenerate(e) && nlast->Degenerate(alt) &&
	e == alt;
}

boolean TNodeList::FoundTermination (
    TTermination*& term, TNode* lbMagic, TNode* rtMagic
) {
    register TNodeList* t;
    TElement* dangling, *attached;
    Alignment a, toAttached;
    TNode* degenTest, *attachment;
    
    for (t = First(); t != End(); t = t->Next()) {
	degenTest = t->GetNode();

	if (
	    degenTest != lbMagic && degenTest != rtMagic &&
	    degenTest->Degenerate(a, dangling)
	) {
	    attachment = OtherNode(dangling, degenTest);
            GetElemOtherThan(dangling, attachment, toAttached, attached);
            if (attached != nil) {
                term = new TTermination(a, dangling, toAttached, attached);
                return true;
            }
	}
    }
    return false;
}

boolean TNodeList::FoundSeries (
    TElement*& e1, TElement*& e2, TNode* lbMagic, TNode* rtMagic
) {
    register TNodeList* t;
    TNode* node;
    
    for (t = First(); t != End(); t = t->Next()) {
	node = t->GetNode();
	if (node != lbMagic && node != rtMagic && node->Series(e1, e2)) {
	    return true;
	}
    }
    return false;
}

boolean TNodeList::FoundStub (TElement*& e) {
    register TNodeList* t;
    TNode* node;
    
    for (t = First(); t != End(); t = t->Next()) {
	node = t->GetNode();
	if (node->Stub(e)) {
	    return true;
	}
    }
    return false;
}

boolean TNodeList::FoundParallel (TElement*& e1, TElement*& e2) {
    register TNodeList* t;
    TNode* node;
    
    for (t = First(); t != End(); t = t->Next()) {
	node = t->GetNode();
	if (FoundParallel(node, e1, e2)) {
	    return true;
	}
    }
    return false;
}

boolean TNodeList::FoundParallel (
    TElementList* elems, TNode* n, TElement*& e1, TElement*& e2
) {
    register TElementList* cur, *test;
    TNode* ncur, *ntest;
    
    for (cur = elems->First(); cur != elems->End(); cur = cur->Next()) {
	e1 = cur->GetElem();
	ncur = OtherNode(e1, n);

	if (ncur != nil) {
	    for (
		test = cur->Next();
		test != elems->End();
		test = test->Next()
	    ) {
		e2 = test->GetElem();
		ntest = OtherNode(e2, n);
		if (ntest == ncur) {
		    return true;
		}
	    }
	}
    }
    return false;
}

boolean TNodeList::FoundCrossover (TElement*& e) {
    register TNodeList* t;
    TNode* node;
    
    for (t = First(); t != End(); t = t->Next()) {
	node = t->GetNode();
	if (FoundCrossover(node, e)) {
	    return true;
	}
    }
    return false;
}

boolean TNodeList::FoundLoop (TLoop*& loop) {
    register TNodeList* t;
    TElement* looped, *attached;
    Alignment toAttached;
    TNode* loopTest;
    
    for (t = First(); t != End(); t = t->Next()) {
	loopTest = t->GetNode();
	if (loopTest->Loop(looped)) {
	    GetElemOtherThan(looped, loopTest, toAttached, attached);
	    loop = new TLoop(looped, toAttached, attached);
	    return true;
	}
    }
    return false;
}

boolean TNodeList::FoundCrossover (TNode* n, TElement*& e1) {
    TElementList* lbElems, *rtElems, *cur, *test;
    TElement* e2;
    TNode* ncur, *ntest;
    
    lbElems = n->LeftBottomElements();
    rtElems = n->RightTopElements();
    
    for (cur = lbElems->First(); cur != lbElems->End(); cur = cur->Next()) {
	e1 = cur->GetElem();
	ncur = OtherNode(e1, n);
	
	if (ncur != nil) {
	    for (
		test = rtElems->First(); 
		test != rtElems->End(); 
		test = test->Next()
	    ) {
		e2 = test->GetElem();
		ntest = OtherNode(e2, n);
		if (ntest == ncur) {
		    return true;
		}
	    }
	}
    }
    return false;
}

TNode* TNodeList::OtherNode (TElement* e, TNode* n) {
    register TNodeList* t;
    TNode* ntest;
    
    for (t = First(); t != End(); t = t->Next()) {
	ntest = t->GetNode();
	if (ntest != n && ntest->Includes(e)) {
	    return ntest;
	}
    }
    return nil;
}

void TNodeList::RemoveTermination (TTermination* t) {
    TNode* degen, *attachment = Node(t->toAttached, t->attached);

    attachment->Exclude(t->dangling);
    degen = OtherNode(t->dangling, attachment);
    Delete(degen);
    delete degen;
}

void TNodeList::RemoveSeries (TElement* equiv, TElement* e1, TElement* e2) {
    TNode* n1lb, *n2lb, *n2rt;
    TNode eqlb(BottomLeft, equiv);
    TNode eqrt(TopRight, equiv);
    
    Nodes(e2, n2lb, n2rt);
    n1lb = OtherNode(e1, n2lb);
    if (n1lb == nil) {			// check for ends tied together
	n1lb = n2rt;
    } else if (n2rt == nil) {
	n2rt = n1lb;
    }
    n1lb->Merge(&eqlb);
    n2rt->Merge(&eqrt);
    n1lb->Exclude(e1);
    n2rt->Exclude(e2);
    Delete(n2lb);
    delete n2lb;
}

void TNodeList::Reverse (TElement* e) {
    TNode* nlb, *nrt;
    TNode elb(BottomLeft, e);
    TNode ert(TopRight, e);
    
    Nodes(e, nlb, nrt);
    nlb->Exclude(e);
    nrt->Exclude(e);
    nlb->Merge(&ert);
    nrt->Merge(&elb);
    e->Reverse();
}

void TNodeList::RemoveParallel (TElement* equiv, TElement* e1, TElement* e2) {
    TNode* n1lb, *n1rt;
    TNode eqlb(BottomLeft, equiv);
    TNode eqrt(TopRight, equiv);
    
    Nodes(e1, n1lb, n1rt);
    n1lb->Merge(&eqlb);
    n1rt->Merge(&eqrt);
    n1lb->Exclude(e1);
    n1lb->Exclude(e2);
    n1rt->Exclude(e1);
    n1rt->Exclude(e2);
}

void TNodeList::RemoveLoop (TLoop* l) {
    TNode* attachment;
    
    if (l->attached == nil) {		    // isolated loop
	attachment = OtherNode(l->looped, nil);
	Delete(attachment);
	delete attachment;
    } else {
	attachment = Node(l->toAttached, l->attached);
	attachment->Exclude(l->looped);
    }
}

void TNodeList::ReplaceTermination (TTermination* t) {
    TNode* degen, *attachment = Node(t->toAttached, t->attached);
    float apos;

    TNode temp(Inverse(t->alignment), t->dangling);
    attachment->Merge(&temp);
    apos = attachment->GetPosition();
    degen = new TNode(t->alignment, t->dangling);

    if (t->alignment == BottomLeft) {
	degen->SetPosition(apos - t->dangling->nat - t->dangling->sigma);
    } else {
	degen->SetPosition(apos + t->dangling->nat + t->dangling->sigma);
    }
    Append(new TNodeList(degen));
}

void TNodeList::ReplaceSeries (TElement* equiv, TElement* e1, TElement* e2) {
    TNode* eqlb, *eqrt, *nctr;
    
    Nodes(equiv, eqlb, eqrt);
    if (eqlb == nil) {			// check for ends tied together
	eqlb = eqrt;
    } else if (eqrt == nil) {
	eqrt = eqlb;
    }
    TNode nlb(BottomLeft, e1);
    TNode nrt(TopRight, e2);
    eqlb->Merge(&nlb);
    eqrt->Merge(&nrt);
    eqlb->Exclude(equiv);
    eqrt->Exclude(equiv);
    nctr = new TNode(TopRight, e1, BottomLeft, e2);
    nctr->SetPosition(e2->pos);
    Append(new TNodeList(nctr));
}

void TNodeList::ReplaceParallel (
    TElement* equiv, TElement* e1, TElement* e2
) {
    TNode* eqlb, *eqrt;
    TNode nlb(BottomLeft, e1, BottomLeft, e2);
    TNode nrt(TopRight, e1, TopRight, e2);
    
    Nodes(equiv, eqlb, eqrt);
    eqlb->Merge(&nlb);
    eqrt->Merge(&nrt);
    eqlb->Exclude(equiv);
    eqrt->Exclude(equiv);
}

void TNodeList::ReplaceLoop (TLoop* l) {
    TNode* attachment;
    
    if (l->attached == nil) {
	attachment = new TNode(BottomLeft, l->looped, TopRight, l->looped);
	Append(new TNodeList(attachment));

    } else {
	attachment = Node(l->toAttached, l->attached);
	TNode node(BottomLeft, l->looped, TopRight, l->looped);
	attachment->Merge(&node);
    }
}

void TNodeList::FindElements (
    Interactor* i, TElement*& lbElem, TElement*& rtElem
) {
    register TNodeList* nl;
    TNode* node;
    TElementList* el;

    lbElem = rtElem = nil;
    
    for (
	nl = First(); 
	nl != End() && (lbElem == nil || rtElem == nil);
	nl = nl->Next()
    ) {
	node = nl->GetNode();
	el = node->LeftBottomElements();
	FindElements(el, i, lbElem, rtElem);

	if (lbElem == nil || rtElem == nil) {
	    el = node->RightTopElements();
	    FindElements(el, i, lbElem, rtElem);
	}
    }
}	    

void TNodeList::FindElement (TGlue* tg, TElement*& elem) {
    register TNodeList* nl;
    TNode* node;
    TElementList* el;

    elem = nil;
    
    for (nl = First(); nl != End() && elem == nil; nl = nl->Next()) {
	node = nl->GetNode();
	el = node->LeftBottomElements();
	FindElement(el, tg, elem);

	if (elem == nil) {
	    el = node->RightTopElements();
	    FindElement(el, tg, elem);
	}
    }
}	    

void TNodeList::FindElements (
    TElementList* el, Interactor* i, TElement*& lbElem, TElement*& rtElem
) {
    register TElementList* cur;
    TElement* test;

    for (
	cur = el->First(); 
	cur != el->End() && (lbElem == nil || rtElem == nil);
	cur = cur->Next()
    ) {
	test = cur->GetElem();
	if (test->owner == i) {
	    if (test->leftBotHalf) {
		lbElem = test;
	    } else {
		rtElem = test;
	    }
	}
    }
}

void TNodeList::FindElement (TElementList* el, TGlue* tg, TElement*& elem) {
    register TElementList* cur;
    TElement* test;

    for (cur = el->First(); cur != el->End(); cur = cur->Next()) {
	test = cur->GetElem();
	if (test->tglue == tg) {
	    elem = test;
	    return;
	}
    }
}

void TNodeList::Nodes (TElement* e, TNode*& nlb, TNode*& nrt) {
    register TNodeList* t;
    TNode* node;
    Alignment a;
    
    nlb = nrt = nil;
    for (t = First(); t != End() && (nlb==nil || nrt==nil); t = t->Next()) {
	node = t->GetNode();
	if (node->Includes(a, e)) {
	    if (a == BottomLeft) {	/* node is below/left of element */
		nlb = node;
	    } else {
		nrt = node;
	    }
	}
    }
}

TNode* TNodeList::Node (Alignment a, TElement* e) {
    register TNodeList* t;
    TNode* node;
    Alignment test;
    
    for (t = First(); t != End(); t = t->Next()) {
	node = t->GetNode();
	if (node->Includes(test, e) && test == a) {
	    return node;
	}
    }
}

void TNodeList::AddMissingNodes (TElement* e) {
    TNode* nlb, *nrt;
    TNodeList* lb, *rt;
    
    Nodes(e, nlb, nrt);
    if (nlb == nil) {
	nlb = new TNode(BottomLeft, e);
	nlb->SetPosition(e->pos);
	lb = new TNodeList(nlb);
	Append(lb);

    } 
    if (nrt == nil) {
	nrt = new TNode(TopRight, e);
	nrt->SetPosition(e->pos + e->nat + e->sigma);
	rt = new TNodeList(nrt);
	Append(rt);
    }
}

void TNodeList::GetElemOtherThan (
    TElement* avoid, TNode* n, Alignment& a, TElement*& e
) {
    register TElementList* cur;
    TElementList* lbElems, *rtElems;
    
    lbElems = n->LeftBottomElements();
    rtElems = n->RightTopElements();
    
    for (cur = lbElems->First(); cur != lbElems->End(); cur = cur->Next()) {
	e = cur->GetElem();
	if (e != avoid) {
	    a = TopRight;
	    return;
	}
    }
    for (cur = rtElems->First(); cur != rtElems->End(); cur = cur->Next()) {
	e = cur->GetElem();
	if (e != avoid) {
	    a = BottomLeft;
	    return;
	}
    }
    e = nil;
}

void TNodeList::ApplyToTermination (TTermination* t) {
    TNode* n = Node(t->toAttached, t->attached);

    if (t->alignment == BottomLeft) {
	t->dangling->pos = n->GetPosition() - t->dangling->nat;

    } else {
	t->dangling->pos = n->GetPosition();

    }
    t->dangling->sigma = 0;
}

void TNodeList::ApplyToSeries (TElement* equiv, TElement* e1, TElement* e2) {
    float d = equiv->nat + equiv->sigma - e1->nat - e2->nat;
    float s1, s2;
    
    if (d < 0) {
	s1 = e1->shrink;
	s2 = e2->shrink;
    } else {
	s1 = e1->stretch;
	s2 = e2->stretch;
    }
    if (s1 == 0 && s2 == 0) {
	e1->sigma = e2->sigma = 0;
    } else {
	e1->sigma = equiv->sigma * s1 / (s1 + s2);
    }

    e1->Limit();
    e2->sigma = equiv->sigma - e1->sigma;
    e2->Limit();
    e1->pos = equiv->pos;
    e2->pos = e1->pos + e1->nat + e1->sigma;
}

void TNodeList::ApplyToParallel (TElement* equiv, TElement* e1, TElement* e2) {
    e1->pos = e2->pos = equiv->pos;
    e1->sigma = equiv->nat + equiv->sigma - e1->nat;
    e2->sigma = equiv->nat + equiv->sigma - e2->nat;
    e1->Limit();
    e2->Limit();
}

void TNodeList::ApplyToLoop (TLoop* l) {
    TNode* n = Node(l->toAttached, l->attached);

    l->looped->pos = n->GetPosition();
    l->looped->sigma = -l->looped->shrink;
}

/*************************************************************************/

class TSolver {
public:
    TSolver(Tray*, Interactor*);
    ~TSolver();

    void AddAlignment(Alignment, Interactor*, TGlue* = nil);
    void AddAlignment(
	Alignment, Interactor*, Alignment, Interactor*, TGlue* = nil
    );
    void DeleteAlignmentsTo(Interactor*);
    void SetShape(Interactor*);
    void Solve(int, int);
    void CalcShape(Shape*);
    void GetPlacement(Interactor*, Coord&, Coord&, Coord&, Coord&);
private:
    TNodeList* hnodes, *vnodes;
    TNode* lmagic, *rmagic, *bmagic, *tmagic;
    Tray* tray;
    Interactor* background;

    Interactor* BgFilter(Interactor*);
    void Solve(
	TNodeList*, TNode*, TNode*, int size, int& nat, int& shr, int& str
    );
    void GetPlacement(TNodeList*, Interactor*, int, Coord&, Coord&);

    void HOrder(Alignment, Interactor*&, Interactor*&);
    void VOrder(Alignment, Interactor*&, Interactor*&);
    void HConvert(Interactor*, TElement*&, TElement*&);
    void HConvert(TGlue*, TElement*&);
    void VConvert(Interactor*, TElement*&, TElement*&);
    void VConvert(TGlue*, TElement*&);
    void HAddAlignment(
	Alignment, TElement*, TElement*, Alignment, TElement*, TElement*,
	TElement*
    );
    void VAddAlignment(
	Alignment, TElement*, TElement*, Alignment, TElement*, TElement*,
	TElement*
    );
    void Include(
	TNodeList*, Alignment, TElement*, Alignment, TElement*, TElement*
    );
    void TrayNodes(TNodeList*, TNode*&, TNode*&);
    void UpdateMagicNodes();
    void DeleteDanglingGlue(TNodeList*, TNode*);
    void DeleteNodesAndElements(TNodeList*);
};

TSolver::TSolver (Tray* t, Interactor* bg) {
    hnodes = new TNodeList;
    vnodes = new TNodeList;
    tray = t;
    background = bg;
    lmagic = rmagic = bmagic = tmagic = nil;
}

TSolver::~TSolver () {
    DeleteNodesAndElements(hnodes);
    DeleteNodesAndElements(vnodes);
    delete hnodes;
    delete vnodes;
}

void TSolver::DeleteNodesAndElements (TNodeList* nodes) {
    TNode* merged;
    register TNodeList* t = nodes->First();
    if (t == nodes->End()) {
        return;
    }
    merged = t->GetNode();
    for (t = t->Next(); t != nodes->End(); t = t->Next()) {
        TNode* doomed = t->GetNode();
        merged->Merge(doomed);
        delete doomed;
    }
    merged->DeleteElements();
    delete merged;
}
inline boolean HAlignment (Alignment a) {
    return a != Bottom && a != VertCenter && a != Top;
}

inline boolean VAlignment (Alignment a) {
    return a != Left && a != HorizCenter && a != Right;
}

void TSolver::AddAlignment (
    Alignment a1, Interactor* i1, Alignment a2, Interactor* i2, TGlue* tg
) {
    TElement* e1l, *e1r, *e2l, *e2r, *e3;
    
    i1 = BgFilter(i1);
    i2 = BgFilter(i2);
    
    if (HAlignment(a1) && HAlignment(a2)) {
	HConvert(i1, e1l, e1r);
	HConvert(i2, e2l, e2r);
	HConvert(tg, e3);
	HAddAlignment(a1, e1l, e1r, a2, e2l, e2r, e3);
    }

    if (VAlignment(a1) && VAlignment(a2)) {
	VConvert(i1, e1l, e1r);
	VConvert(i2, e2l, e2r);
	VConvert(tg, e3);
	VAddAlignment(a1, e1l, e1r, a2, e2l, e2r, e3);
    }
    UpdateMagicNodes();
}
 
void TSolver::AddAlignment (Alignment a, Interactor* i, TGlue* tg) {
    TElement* e1l, *e1r, *e2l, *e2r, *e3;
    Interactor* i1 = i;
    Interactor* i2 = tray;
    
    if (i == background || i == tray) {
	return;
    }

    if (HAlignment(a)) {
	HOrder(a, i1, i2);
	HConvert(i1, e1l, e1r);
	HConvert(i2, e2l, e2r);
	HConvert(tg, e3);
	HAddAlignment(a, e1l, e1r, a, e2l, e2r, e3);
    }

    if (VAlignment(a)) {
	VOrder(a, i1, i2);
	VConvert(i1, e1l, e1r);
	VConvert(i2, e2l, e2r);
	VConvert(tg, e3);
	VAddAlignment(a, e1l, e1r, a, e2l, e2r, e3);
    }
    UpdateMagicNodes();
}

void TSolver::HOrder (Alignment a, Interactor*& i1, Interactor*& i2) {
    Interactor* i = (i1 == tray) ? i2 : i1;
    
    if (a == BottomRight || a == CenterRight || a == TopRight || a == Right) {
	i1 = i;
	i2 = tray;
    } else {
	i1 = tray;
	i2 = i;
    }
}

void TSolver::VOrder (Alignment a, Interactor*& i1, Interactor*& i2) {
    Interactor* i = (i1 == tray) ? i2 : i1;
    
    if (a == TopLeft || a == TopCenter || a == TopRight || a == Top) {
	i1 = i;
	i2 = tray;
    } else {
	i1 = tray;
	i2 = i;
    }
}

void TSolver::DeleteAlignmentsTo (Interactor* i) {
    TElement* e1, *e2;
    TNode* lb, *rt;
    
    i = BgFilter(i);
    hnodes->FindElements(i, e1, e2);
    lb = hnodes->Node(BottomLeft, e1);
    rt = hnodes->Node(TopRight, e2);
    hnodes->Exclude(e1);
    hnodes->Exclude(e2);
    delete e1;
    delete e2;
    DeleteDanglingGlue(hnodes, lb);
    DeleteDanglingGlue(hnodes, rt);

    vnodes->FindElements(i, e1, e2);
    lb = vnodes->Node(BottomLeft, e1);
    rt = vnodes->Node(TopRight, e2);
    vnodes->Exclude(e1);
    vnodes->Exclude(e2);
    delete e1;
    delete e2;
    DeleteDanglingGlue(vnodes, lb);
    DeleteDanglingGlue(vnodes, rt);
}

void TSolver::SetShape (Interactor* i) {
    TElement* lbElem, *rtElem;
    
    i = BgFilter(i);
    
    hnodes->FindElements(i, lbElem, rtElem);
    if (lbElem == nil) {
	return;
    }
    lbElem->HSetShape();
    rtElem->HSetShape();

    vnodes->FindElements(i, lbElem, rtElem);
    if (lbElem == nil) {
	return;
    }
    lbElem->VSetShape();
    rtElem->VSetShape();
}

void TSolver::Solve (int w, int h) {
    int dummy;

    if (lmagic != nil) {
        lmagic->SetPosition(0);
        rmagic->SetPosition(w);
        Solve(hnodes, lmagic, rmagic, w, dummy, dummy, dummy);
    }
    if (bmagic != nil) {
        bmagic->SetPosition(0);
        tmagic->SetPosition(h);
        Solve(vnodes, bmagic, tmagic, h, dummy, dummy, dummy);
    }
}

void TSolver::CalcShape (Shape* s) {
    TElement* ltray, *rtray, *btray, *ttray;
    
    hnodes->FindElements(tray, ltray, rtray);
    vnodes->FindElements(tray, btray, ttray);

    if (ltray != nil && lmagic != nil) {
        ltray->combinable = rtray->combinable = false;
        Solve(hnodes, lmagic, rmagic, 0, s->width, s->hshrink, s->hstretch);
        ltray->combinable = rtray->combinable = true;
    }

    if (btray != nil && bmagic != nil) {
        btray->combinable = ttray->combinable = false;
        Solve(vnodes, bmagic, tmagic, 0, s->height, s->vshrink, s->vstretch);
        btray->combinable = ttray->combinable = true;
    }
}

void TSolver::GetPlacement (
    Interactor* i, Coord& l, Coord& b, Coord& r, Coord& t
) {
    Shape* s = i->GetShape();

    GetPlacement(hnodes, i, s->width, l, r);
    GetPlacement(vnodes, i, s->height, b, t);
}

void TSolver::GetPlacement (
    TNodeList* nodes, Interactor* i, int dfault, Coord& lb, Coord& rt
) {
    TElement* lbElem, *rtElem;

    nodes->FindElements(i, lbElem, rtElem);
    if (lbElem == nil) {
	lb = 0;
	rt = dfault - 1;
    } else {
	lb = round(lbElem->pos);
	rt = round(
	    lbElem->pos + lbElem->nat + lbElem->sigma +
	    rtElem->nat + rtElem->sigma - 1
	);
    }
}

void TSolver::Solve (
    TNodeList* nodes, TNode* lbMagic, TNode* rtMagic, int size,
    int& nat, int& shr, int& str
) {
    TElement* e1, *e2, *e3;
    TTermination* t;
    TLoop* l;
    TNode* n;
    
    if (nodes->Empty()) {
	/* no alignments; do nothing */
	
    } else if (nodes->Degenerate(e1)) {
	nat = round(e1->nat);
	shr = round(e1->shrink);
	str = round(e1->stretch);
	e1->pos = (lbMagic == nil) ? 0 : lbMagic->GetPosition();
	e1->sigma = (rtMagic == nil) ? e1->nat : size - round(e1->nat);
	e1->Limit();

    } else if (nodes->FoundSeries(e1, e2, lbMagic, rtMagic)) {
	e3 = e1->Series(e2);
	nodes->RemoveSeries(e3, e1, e2);
	Solve(nodes, lbMagic, rtMagic, size, nat, shr, str);
	nodes->ApplyToSeries(e3, e1, e2);
	nodes->ReplaceSeries(e3, e1, e2);
	delete e3;

    } else if (nodes->FoundParallel(e1, e2)) {
	e3 = e1->Parallel(e2);
	nodes->RemoveParallel(e3, e1, e2);
	Solve(nodes, lbMagic, rtMagic, size, nat, shr, str);
	nodes->ApplyToParallel(e3, e1, e2);
	nodes->ReplaceParallel(e3, e1, e2);
	delete e3;

    } else if (nodes->FoundTermination(t, lbMagic, rtMagic)) {
	nodes->RemoveTermination(t);
	Solve(nodes, lbMagic, rtMagic, size, nat, shr, str);
	nodes->ApplyToTermination(t);
	nodes->ReplaceTermination(t);
	delete t;
	
    } else if (nodes->FoundStub(e1) || nodes->FoundCrossover(e1)) {
	nodes->Reverse(e1);
	Solve(nodes, lbMagic, rtMagic, size, nat, shr, str);
	nodes->Reverse(e1);

    } else if (nodes->FoundLoop(l)) {
	nodes->RemoveLoop(l);
	Solve(nodes, lbMagic, rtMagic, size, nat, shr, str);
	nodes->ApplyToLoop(l);
	nodes->ReplaceLoop(l);
	delete l;

    } else if (nodes->OnlyOne()) {
	n = nodes->First()->GetNode();
	if (n != lbMagic && n != rtMagic) {
	    n->SetPosition(0);
	}
	nat = str = shr = 0;
    }
}

void TSolver::HConvert (Interactor* i, TElement*& el, TElement*& er) {
    if (i == nil) {
	el = er = nil;

    } else {
	hnodes->FindElements(i, el, er);
	if (el == nil) {
	    el = new TElement(i);
	    el->combinable = true;
	    el->leftBotHalf = true;
	    el->HSetShape();
	
	    er = new TElement(i);
	    er->combinable = true;
	    er->leftBotHalf = false;
	    er->HSetShape();
	    hnodes->Include(TopRight, el, BottomLeft, er);
	}
    }
}
 
void TSolver::HConvert (TGlue* tg, TElement*& e) {
    if (tg == nil) {
	e = nil;

    } else {
	hnodes->FindElement(tg, e);
	if (e == nil) {
	    e = new TElement(tg);
	    e->combinable = true;
	    e->leftBotHalf = true;
	    e->HSetShape();
	}
    }
}

void TSolver::VConvert (Interactor* i, TElement*& eb, TElement*& et) {
    if (i == nil) {
	eb = et = nil;

    } else {
	vnodes->FindElements(i, eb, et);
	if (eb == nil) {
	    eb = new TElement(i);
	    eb->combinable = true;
	    eb->leftBotHalf = true;
	    eb->VSetShape();
	
	    et = new TElement(i);
	    et->combinable = true;
	    et->leftBotHalf = false;
	    et->VSetShape();
	    vnodes->Include(TopRight, eb, BottomLeft, et);
	}
    }
}

void TSolver::VConvert (TGlue* tg, TElement*& e) {
    if (tg == nil) {
	e = nil;

    } else {
	vnodes->FindElement(tg, e);
	if (e == nil) {
	    e = new TElement(tg);
	    e->combinable = true;
	    e->leftBotHalf = true;
	    e->VSetShape();
	}
    }
}

void TSolver::HAddAlignment (
    Alignment a1, TElement* e1l, TElement* e1r,
    Alignment a2, TElement* e2l, TElement* e2r,
    TElement* tg
) {
    TElement* e1, *e2;
    Alignment na1, na2;
    
    switch (a1) {
	case TopLeft:
	case CenterLeft:
	case BottomLeft:
	case Left:
	    e1 = e1l;
	    na1 = BottomLeft;
	    break;
	case TopCenter:
	case Center:
	case BottomCenter:
	case HorizCenter:
	    e1 = e1l;
	    na1 = TopRight;
	    break;
	case TopRight:
	case CenterRight:
	case BottomRight:
	case Right:
	    e1 = e1r;
	    na1 = TopRight;
	    break;
    }
    switch (a2) {
	case TopLeft:
	case CenterLeft:
	case BottomLeft:
	case Left:
	    e2 = e2l;
	    na2 = BottomLeft;
	    break;
	case TopCenter:
	case Center:
	case BottomCenter:
	case HorizCenter:
	    e2 = e2l;
	    na2 = TopRight;
	    break;
	case TopRight:
	case CenterRight:
	case BottomRight:
	case Right:
	    e2 = e2r;
	    na2 = TopRight;
	    break;
    }
    hnodes->AddMissingNodes(e1l);
    hnodes->AddMissingNodes(e1r);
    hnodes->AddMissingNodes(e2l);
    hnodes->AddMissingNodes(e2r);
    Include(hnodes, na1, e1, na2, e2, tg);
}

void TSolver::VAddAlignment (
    Alignment a1, TElement* e1b, TElement* e1t,
    Alignment a2, TElement* e2b, TElement* e2t,
    TElement* tg
) {
    TElement* e1, *e2;
    Alignment na1, na2;
    
    switch (a1) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	case Bottom:
	    e1 = e1b;
	    na1 = BottomLeft;
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	case VertCenter:
	    e1 = e1b;
	    na1 = TopRight;
	    break;
	case TopLeft:
	case TopCenter:
	case TopRight:
	case Top:
	    e1 = e1t;
	    na1 = TopRight;
	    break;
    }
    switch (a2) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	case Bottom:
	    e2 = e2b;
	    na2 = BottomLeft;
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	case VertCenter:
	    e2 = e2b;
	    na2 = TopRight;
	    break;
	case TopLeft:
	case TopCenter:
	case TopRight:
	case Top:
	    e2 = e2t;
	    na2 = TopRight;
	    break;
    }
    vnodes->AddMissingNodes(e1b);
    vnodes->AddMissingNodes(e1t);
    vnodes->AddMissingNodes(e2b);
    vnodes->AddMissingNodes(e2t);
    Include(vnodes, na1, e1, na2, e2, tg);
}

void TSolver::Include (
    TNodeList* nodes, 
    Alignment na1, TElement* e1, Alignment na2, TElement* e2, TElement* tg
) {
    if (e1->owner == e2->owner && na1 == na2) {
	/* aligned a node to itself; do nothing */
    
    } else if (tg == nil) {
	nodes->Include(na1, e1, na2, e2);

    } else if (na1 == BottomLeft && na2 == TopRight) {
	nodes->Include(BottomLeft, e1, TopRight, tg);
	nodes->Include(TopRight, e2, BottomLeft, tg);

    } else {
	nodes->Include(na1, e1, BottomLeft, tg);
	nodes->Include(na2, e2, TopRight, tg);
    }
}

void TSolver::TrayNodes (TNodeList* nodes, TNode*& nlb, TNode*& nrt) {
    TNode *ctr;
    TElement* elb, *ert;

    nodes->FindElements(tray, elb, ert);
    if (elb == nil) {
	nlb = nrt = nil;
    } else {
	nodes->Nodes(elb, nlb, ctr);
	nrt = nodes->OtherNode(ert, ctr);
    }
}

void TSolver::UpdateMagicNodes () {
    TrayNodes(hnodes, lmagic, rmagic);
    TrayNodes(vnodes, bmagic, tmagic);
}

void TSolver::DeleteDanglingGlue (TNodeList* nodes, TNode* n) {
    TElement* e;

    if (n->Degenerate(e) && e->tglue != nil) {
        nodes->Exclude(e);
        delete e;
    }
}

Interactor* TSolver::BgFilter (Interactor* i) {
    return (i == background) ? tray : i;
}

/*************************************************************************/

class TrayElement {
public:
    Interactor* child;
    boolean visible;
    TrayElement* next;
};

/*************************************************************************/

TGlue::TGlue (int w, int h, int hstr, int vstr) {
    shape = new Shape;
    shape->width = w;
    shape->height = h;
    shape->hshrink = 0;
    shape->hstretch = hstr;
    shape->vshrink = 0;
    shape->vstretch = vstr;
}

TGlue::TGlue (int w, int h, int hshr, int hstr, int vshr, int vstr) {
    shape = new Shape;
    shape->width = w;
    shape->height = h;
    shape->hshrink = hshr;
    shape->hstretch = hstr;
    shape->vshrink = vshr;
    shape->vstretch = vstr;
}

TGlue::~TGlue () {
    delete shape;
}

/*************************************************************************/

inline boolean Tray::TrayOrBg (Interactor* i) { return i == this || i == bg; }

Tray::Tray (Interactor* b) {
    Init(b);
}

Tray::Tray (const char* name, Interactor* b) {
    SetInstance(name);
    Init(b);
}

Tray::Tray (Sensor* in, Painter* out, Interactor* b) : (in, out) {
    Init(b);
}

void Tray::Init (Interactor* b) {
    SetClassName("Tray");
    nelements = 0;
    head = nil;
    tail = nil;
    bg = b;
    tsolver = new TSolver(this, bg);
}

Tray::~Tray () {
    register TrayElement* e, *next;
    delete tsolver;

    for (e = head; e != nil; e = next) {
        next = e->next;
        delete e->child;
	delete e;
    }
    if (bg != nil) {
	delete bg;
    }
}

void Tray::ComponentBounds (int& w, int& h) {
    register TrayElement* e, *next;
    Shape* s;
    
    w = h = 0;
    for (e = head; e != nil; e = next) {
        next = e->next;
        s = e->child->GetShape();
	w = max(w, s->width);
	h = max(h, s->height);
    }
}    

void Tray::CalcShape () {
    int w, h;

    if (bg == nil) {
	ComponentBounds(w, h);
	tsolver->CalcShape(shape);
	shape->width = max(shape->width, w);
	shape->height = max(shape->height, h);
    } else {
	*shape = *bg->GetShape();
    }
    tsolver->SetShape(this);
}

void Tray::Reconfig () {
    register TrayElement* e;

    for (e = head; e != nil; e = e->next) {
	tsolver->SetShape(e->child);
    }
    CalcShape();
}

void Tray::DoInsert (Interactor* i, boolean, Coord&, Coord&) {
    ++nelements;
    register TrayElement* e = new TrayElement;
    e->child = i;
    e->next = nil;
    if (head == nil) {
        head = e;
        tail = e;
    } else {
        tail->next = e;
        tail = e;
    }
}

void Tray::DoChange (Interactor* i) {
    tsolver->SetShape(i);
    CalcShape();
}

void Tray::DoRemove (Interactor* i) {
    register TrayElement* e, * prev;

    if (i == bg) {
	bg = nil;
	tsolver->DeleteAlignmentsTo(i);
    } else {
	--nelements;
	prev = nil;
	for (e = head; e != nil; e = e->next) {
	    if (e->child == i) {
		if (prev == nil) {
		    head = e->next;
		} else {
		    prev->next = e->next;
		}
		if (e == tail) {
		    tail = prev;
		}
		delete e;
		tsolver->DeleteAlignmentsTo(i);
		break;
	    }
	    prev = e;
	}
    }
}

void Tray::Resize () {
    register TrayElement* e;

    canvas->SetBackground(output->GetBgColor());
    if (bg != nil) {
	Place(bg, 0, 0, xmax, ymax);
    }

    for (e = head; e != nil; e = e->next) {
	tsolver->SetShape(e->child);
    }

    tsolver->Solve(xmax+1, ymax+1);

    for (e = head; e != nil; e = e->next) {
	PlaceElement(e);
    }
}

boolean Tray::AlreadyInserted (Interactor* i) {
    register TrayElement* e;

    if (i == this || i == bg) {
	return true;
    }
    
    for (e = head; e != nil; e = e->next) {
	if (e->child == i) {
	    return true;
	}
    }
    return false;
}

void Tray::PlaceElement (TrayElement* e) {
    Coord l, b, r, t, tmp;
    tsolver->GetPlacement(e->child, l, b, r, t);

    if (
	r > 0 && l < xmax && t > 0 && b < ymax && 
	l - r != -1 && t - b != -1
    ) {
	e->visible = true;
	tmp = min(l, r);
	r = max(l, r);
	l = tmp;
	tmp = min(b, t);
	t = max(b, t);
	b = tmp;
	Place(e->child, l, b, r, t);
    } else {
	e->visible = false;
    }
}

void Tray::Draw () {
    register TrayElement* e;

    if (bg != nil) {
	bg->Draw();
    }
    for (e = head; e != nil; e = e->next) {
        if (e->visible) {
            e->child->Draw();
        }
    }
}

void Tray::Reshape (Shape& s) {
    *shape = s;
    Scene* p = Parent();
    if (p != nil) {
	p->Change(this);
    }
}

void Tray::GetComponents (Interactor** c, int nc, Interactor**& a, int& n) {
    register TrayElement* e;
    register Interactor** ap;

    n = nelements;
    if (bg != nil) {
	++n;
    }
    a = (n <= nc) ? c : new Interactor*[n];
    ap = a;
    for (e = head; e != nil; e = e->next) {
        *ap++ = e->child;
    }
    if (bg != nil) {
	*ap = bg;
    }
}

void Tray::Align (
    Alignment a1, Interactor* i1, Alignment a2, Interactor* i2, TGlue* tg
) {
    if (!AlreadyInserted(i1)) {
	Insert(i1);
    }
    if (!AlreadyInserted(i2)) {
	Insert(i2);
    }
    tsolver->AddAlignment(a1, i1, a2, i2, tg);
}

void Tray::Align (Alignment a, Interactor* i, TGlue* tg) {
    if (!AlreadyInserted(i)) {
	Insert(i);
    }
    tsolver->AddAlignment(a, i, tg);
}

static void LoadInteractorArray (
    Interactor* i[], Interactor* i0, Interactor* i1,
    Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5, Interactor* i6
) {
    i[0] = i0;
    i[1] = i1;
    i[2] = i2;
    i[3] = i3;
    i[4] = i4;
    i[5] = i5;
    i[6] = i6;
}

void Tray::Align (
    Alignment a, Interactor* i0, Interactor* i1,
    Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5, Interactor* i6
) {
    const int n = 7;
    Interactor* i[n];
    int k;
    
    LoadInteractorArray(i, i0, i1, i2, i3, i4, i5, i6);
    
    for (k = 0; k < n && i[k] != nil; ++k) {
	if (!AlreadyInserted(i[k])) {
	    Insert(i[k]);
	}
    }
    for (k = 1; k < n && i[k] != nil; ++k) {
	tsolver->AddAlignment(a, i[k-1], a, i[k]);
    }    
}

void Tray::HBox (
    Interactor* i0, Interactor* i1,
    Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5, Interactor* i6
) {
    const int n = 7;
    Interactor* i[n];
    int k, last = n - 1;
    
    LoadInteractorArray(i, i0, i1, i2, i3, i4, i5, i6);
    
    for (k = 0; k < n && i[k] != nil; ++k) {
	if (!AlreadyInserted(i[k])) {
	    Insert(i[k]);
	}
    }
    for (k = 1; k < n && i[k] != nil; ++k) {
	if (TrayOrBg(i[0]) && k == 1) {
	    tsolver->AddAlignment(Left, this, Left, i[1]);

	} else if (TrayOrBg(i[k]) && (k == last || i[k+1] == nil)) {
	    tsolver->AddAlignment(Right, i[k-1], Right, this);

	} else {
	    tsolver->AddAlignment(Right, i[k-1], Left, i[k]);
	}
    }    
}

void Tray::VBox (
    Interactor* i0, Interactor* i1,
    Interactor* i2, Interactor* i3, Interactor* i4,
    Interactor* i5, Interactor* i6
) {
    const int n = 7;
    Interactor* i[n];
    int k, last = n - 1;
    
    LoadInteractorArray(i, i0, i1, i2, i3, i4, i5, i6);
    
    for (k = 0; k < n && i[k] != nil; ++k) {
	if (!AlreadyInserted(i[k])) {
	    Insert(i[k]);
	}
    }
    for (k = 1; k < n && i[k] != nil; ++k) {
	if (TrayOrBg(i[0]) && k == 1) {
	    tsolver->AddAlignment(Top, this, Top, i[1]);

	} else if (TrayOrBg(i[k]) && (k == last || i[k+1] == nil)) {
	    tsolver->AddAlignment(Bottom, i[k-1], Bottom, this);

	} else {
	    tsolver->AddAlignment(Bottom, i[k-1], Top, i[k]);
	}
    }    
}
