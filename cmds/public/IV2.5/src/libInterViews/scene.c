/*
 * Base composite interactor.
 */

#include <InterViews/scene.h>
#include <InterViews/shape.h>
#include <InterViews/canvas.h>
#include <InterViews/painter.h>
#include <InterViews/strtable.h>

Scene::Scene () {
    propagate = true;
}

Scene::Scene (Sensor* in, Painter* out) : (in, out) {
    propagate = true;
}

/*
 * Assign the actual location of an interactor and call its Resize operation
 * so that if it is a scene it can place its components.
 */

void Scene::Assign (register Interactor* i, Coord x, Coord y, int w, int h) {
    i->left = x;
    i->bottom = y;
    i->xmax = w - 1;
    i->ymax = h - 1;
    i->canvas->width = w;
    i->canvas->height = h;
    i->Resize();
}

void Scene::Insert (Interactor* component) {
    Interactor* i = Wrap(component);
    PrepareToInsert(i);
    DoInsert(i, false, i->left, i->bottom);
}

void Scene::Insert (Interactor* component, Coord x, Coord y, Alignment a) {
    Interactor* i = Wrap(component);
    PrepareToInsert(i);
    Coord ax = x, ay = y;
    DoAlign(i, a, ax, ay);
    DoInsert(i, true, ax, ay);
}

void Scene::PrepareToInsert (Interactor* i) {
    if (parent != nil || (canvas != nil && canvas->status == CanvasMapped)) {
	i->Config(this);
    }
}

void Scene::DoAlign (Interactor* i, Alignment a, Coord& x, Coord& y) {
    switch (a) {
	case TopLeft:
	case CenterLeft:
	case BottomLeft:
	    /* nothing to do */
	    break;
	case TopCenter:
	case Center:
	case BottomCenter:
	    x -= i->shape->width/2;
	    break;
	case TopRight:
	case CenterRight:
	case BottomRight:
	    x -= i->shape->width;
	    break;
    }
    switch (a) {
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	    /* nothing to do */
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	    y -= i->shape->height/2;
	    break;
	case TopLeft:
	case TopCenter:
	case TopRight:
	    y -= i->shape->height;
	    break;
    }
}

void Scene::Change (Interactor* i) {
    if (propagate) {
	DoChange(i);
	if (parent != nil) {
	    parent->Change(this);
	}
    } else if (canvas != nil) {
	Resize();
    }
}

void Scene::Remove (Interactor* i) {
    DoRemove(i);
    i->parent = nil;
    if (i->canvas != nil) {
	Unmap(i);
	Orphan(i);
    }
}

void Scene::Orphan (Interactor* i) {
    Interactor* children[100];
    Interactor** a;
    int n;

    i->GetComponents(children, sizeof(children) / sizeof(Interactor*), a, n);
    if (n > 0) {
	register int index;

	for (index = 0; index < n; index++) {
	    Orphan(a[index]);
	}
	if (a != children) {
	    delete a;
	}
    }
    delete i->canvas;
    i->canvas = nil;
}

void Scene::DoInsert (Interactor* i, boolean, Coord&, Coord&) {
    /* default is to ignore */
}

void Scene::DoChange (Interactor*) {
    /* default is to ignore */
}

void Scene::DoRemove (Interactor*) {
    /* default is to ignore */
}

void Scene::DoRaise (Interactor*) {
    /* default is to ignore */
}

void Scene::DoLower (Interactor*) {
    /* default is to ignore */
}

void Scene::DoMove (Interactor* i, Coord&, Coord&) {
    /* default is to ignore */
}

/*
 * Wrap is called to put any desired layer around an interactor
 * that is inserted into a scene.  The default is to simply
 * return the interactor as is.
 */

Interactor* Scene::Wrap (Interactor* i) {
    return i;
}

void Scene::Propagate (boolean b) {
    propagate = b;
}

/*
 * A common case is a scene with a single subcomponent.  This construct
 * occurs when one interactor is defined in terms of another, e.g.,
 * a menu is built out of a frame around a box.  The reason a MonoScene
 * is preferred over subclassing is that it simplies implementing the virtuals.
 * In the menu example, menus can handle events independently of frames.
 */

MonoScene::MonoScene () {
    component = nil;
}

MonoScene::MonoScene (Sensor* in, Painter* out) : (in, out) {
    component = nil;
}

MonoScene::~MonoScene () {
    delete component;
}

void MonoScene::DoInsert (Interactor* i, boolean, Coord&, Coord&) {
    if (component != nil) {
	delete component;
    }
    component = i;
}

void MonoScene::DoChange (Interactor*) {
    Reconfig();
}

void MonoScene::DoRemove (Interactor*) {
    component = nil;
}

void MonoScene::Reconfig () {
    if (component != nil) {
	*shape = *component->GetShape();
    }
}

void MonoScene::Resize () {
    if (output != nil) {
	canvas->SetBackground(output->GetBgColor());
    }
    Place(component, 0, 0, xmax, ymax);
}

void MonoScene::Draw () {
    Scene::Draw();
    component->Draw();
}

void MonoScene::GetComponents (
    Interactor** c, int nc, Interactor**& a, int& n
) {
    if (component == nil) {
	n = 0;
    } else if (nc > 0) {
	n = 1;
	a = c;
	a[0] = component;
    } else {
	n = 1;
	a = new Interactor*[1];
	a[0] = component;
    }
}
