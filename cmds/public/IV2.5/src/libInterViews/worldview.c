/*
 * Support for window managers.
 */

#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>
#include <InterViews/worldview.h>

WorldView* _worldview;

WorldView::WorldView (World* w, Sensor* s, Painter* p) : (s, p) {
    world = w;
    curfocus = nil;
    Init(w);
    if (input == nil) {
	input = new Sensor;
	input->Catch(DownEvent);
	input->Catch(UpEvent);
	input->CatchRemote();
    }
    w->Listen(input);
    _worldview = this;
}

WorldView::~WorldView () {
    /* nothing to do currently */
}

void WorldView::InsertRemote (RemoteInteractor i) {
    Map(i);
}

void WorldView::ChangeRemote (
    RemoteInteractor i, Coord left, Coord top, int w, int h
) {
    Change(i, left, top, w, h);
}

RemoteInteractor WorldView::Choose (Cursor* c, boolean waitforup) {
    Event e;

    GrabMouse(c);
    do {
	Read(e);
    } while (e.eventType != DownEvent);
    if (waitforup) {
	do {
	    Read(e);
	} while (e.eventType != UpEvent);
    }
    UngrabMouse();
    return Find(e.x, e.y);
}

void WorldView::FreeList (RemoteInteractor* i) {
    delete i;
}

class Blanket : public Interactor {
public:
    Blanket(int x, int y);
private:
    int _right;
    int _top;

    void Reconfig();
};

Blanket::Blanket (int x, int y) {
    _right = x;
    _top = y;
}

void Blanket::Reconfig () {
    shape->Rect(_right + 1, _top + 1);
}

void WorldView::RedrawAll () {
    Blanket* b = new Blanket(xmax, ymax);
    world->Insert(b, 0, 0);
    world->Remove(b);
    delete b;
    Sync();
}
