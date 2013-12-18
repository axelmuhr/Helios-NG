/*
 * Dialog class implementation.
 */

#include <InterViews/button.h>
#include <InterViews/dialog.h>
#include <InterViews/event.h>
#include <InterViews/world.h>

Dialog::Dialog (ButtonState* b, Interactor* i, Alignment a) {
    Init(b, i, a);
}

Dialog::Dialog (const char* name, ButtonState* b, Interactor* i, Alignment a) {
    SetInstance(name);
    Init(b, i, a);
}

void Dialog::Init (ButtonState* b, Interactor* i, Alignment a) {
    SetClassName("Dialog");
    state = b;
    align = a;
    Insert(i);
}

boolean Dialog::Popup (Event& e, boolean placed) {
    World* w;
    Coord wx, wy;
    boolean accept;

    e.GetAbsolute(w, wx, wy);
    if (placed) {
	w->InsertTransient(this, e.target, wx, wy, align);
    } else {
	w->InsertTransient(this, e.target);
    }
    accept = Accept();
    w->Remove(this);
    return accept;
}

boolean Dialog::Accept () {
    Event e;
    int v;

    state->SetValue(0);
    v = 0;
    do {
	Read(e);
	e.target->Handle(e);
	state->GetValue(v);
    } while (v == 0 && e.target != nil);
    return v == 1 || e.target == nil;
}

int Dialog::Status () {
    int v;

    state->GetValue(v);
    return v;
}
