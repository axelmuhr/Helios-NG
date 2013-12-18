/*
 * Implementation of subject base class.
 */

#include <InterViews/interactor.h>
#include <InterViews/subject.h>

class ViewList {
    friend class Subject;

    Interactor* element;
    ViewList* next;

    ViewList (Interactor* i) { element = i; }
};

Subject::Subject () {
    views = nil;
}

Subject::~Subject () {
    register ViewList* v, * next;

    for (v = views; v != nil; v = next) {
	next = v->next;
	delete v;
    }
}

void Subject::Attach (Interactor* i) {
    ViewList* v = new ViewList(i);
    v->next = views;
    views = v;
}

void Subject::Detach (Interactor* i) {
    register ViewList* v, * prev;

    prev = nil;
    for (v = views; v != nil; v = v->next) {
	if (v->element == i) {
	    if (prev == nil) {
		views = v->next;
	    } else {
		prev->next = v->next;
	    }
	    delete v;
	    break;
	}
	prev = v;
    }
}

void Subject::Notify () {
    register ViewList* v;

    for (v = views; v != nil; v = v->next) {
	v->element->Update();
    }
}

void Subject::DeleteViews () {
    register ViewList* v;

    for (v = views; v != nil; v = v->next) {
	delete v->element;
    }
}
