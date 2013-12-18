/*
 *  InterViews Desktop WorldView (for window managers)
 *
 *  This contains the built-in operations the desktop can perform.
 *  To add a new operation, five things must be updated:
 *
 *	o OperationCode			(in desktop.h)
 *	o Operations			(in desktop.h)
 * 	o DesktopFunctionNames[]	(in desktop.c)
 *	o DesktopFunctions()		(in desktop.c)
 *	o Desktop::Do<function>(Event&)	(in desktop.c)
 *
 *  This file also contains the Desktop constructor and destructor, Run(),
 *  and Config(), none of which need to be modified for new operations.
 */

#include "rubber.h"
#include "desktop.h"
#include "dispatch.h"
#include <InterViews/bitmap.h>
#include <InterViews/cursor.h>
#include <InterViews/canvas.h>
#include <InterViews/paint.h>
#include <InterViews/reqerr.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef X10
#include <InterViews/X10/Xinput.h>
#endif
#ifdef X11
#include <InterViews/X11/Xlib.h>
#endif

extern char* program_name;

class IconFrame : public Frame {
public:
    IconFrame (Icon* i) : (i) {}
    RemoteInteractor RemoteId () { return canvas->Id(); }
};

Icon::Icon (const char* name, int n) {
    SetClassName("Icon");
    s = new char[strlen(name)+1];
    strcpy(s, name);
    padding = n;
}

void Icon::Reconfig () {
    Font* f = output->GetFont();
    shape->width = f->Width(s) + 2*padding;
    shape->height = f->Height() + padding;
}

Icon::~Icon () {
    delete s;
}

boolean Icon::IsOld (const char* name) {
    if (s != nil && name != nil) {
	return strcmp(s, name) != 0;
    }
    return true;
}

void Icon::Resize () {
    Font* f = output->GetFont();
    xpos = (xmax - f->Width(s)) >> 1;
    ypos = (ymax - f->Height()) >> 1;
}

void Icon::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    output->Text(canvas, s, xpos, ypos);
}

static char* ButtonNames (unsigned mask) {
    char* s = new char[28];
    s[0] = '\0';

    if (mask&LockMask) {
	strcat(s, "lock ");
    }
    if (mask&ShiftMask) {
	strcat(s, "shift ");
    }
    if (mask&ControlMask) {
	strcat(s, "ctrl ");
    }
    if (mask&Mod1Mask) {
	strcat(s, "meta ");
    }
    if (mask&LeftMask) {
	strcat(s, "left");
    } else if (mask&MiddleMask) {
	strcat(s, "middle");
    } else if (mask&RightMask) {
	strcat(s, "right");
    }
    return s;
}

class ErrHandler : public ReqErr {
public:
    ErrHandler () { Install(); }
    void Error();
};

void ErrHandler::Error () {
    fprintf(stderr, "iwm: X Error ");
    fprintf(stderr, "%s on request %d for %d\n", message, request, id);
}

Desktop::Desktop (World* w) : (w, nil, stdpaint) {
    tracking = new Sensor(input);
    tracking->Catch(MotionEvent);

    Config();
    if (logosize > 0) {
	if (logo_bitmap != nil) {
	    logo = new BitmapLogo(logo_bitmap, logosize);
	} else {
	    logo = new PolygonLogo(logosize);
	}
    }
    lock = new ScreenLock(output);
    dispatch = new DesktopDispatcher(this);	/* reads .iwmrc */
    icons = nil;
    if (!GrabEm()) {
	UnGrabEm();
	exit(1);
    }
    UnGrabEm();
    new ErrHandler;
}

#ifdef X10

static void ConvertButtonMask (unsigned bmask, unsigned& b, unsigned& m) {
    b = 0;
    m = bmask;
}

#else

static void ConvertButtonMask (unsigned bmask, unsigned& b, unsigned& m) {
    b = 0;
    m = 0;
    if ((bmask&LeftMask) != 0) {
	b |= Button1;
    }
    if ((bmask&MiddleMask) != 0) {
	b |= Button2;
    }
    if ((bmask&RightMask) != 0) {
	b |= Button3;
    }
    if ((bmask&ShiftMask) != 0) {
	m |= ShiftMask;
    }
    if ((bmask&ControlMask) != 0) {
	m |= ControlMask;
    }
    if ((bmask&LockMask) != 0) {
	m |= LockMask;
    }
    if ((bmask&Mod1Mask) != 0) {
	m |= Mod1Mask;
    }
}

#endif

boolean Desktop::GrabEm () {
    boolean ok = true;
    register ButtonList* b = dispatch->Buttons();
    unsigned button, mask;

    while (b != nil) {
	ConvertButtonMask(b->mask, button, mask);
	if (!GrabButton(button, mask, b->cursor)) {
	    fprintf(stderr, "%s: couldn't grab `%s button'\n",
		    program_name, ButtonNames(b->mask));
	    ok = false;
	}
	if (ignorecaps) {
	    GrabButton(button, mask|LockMask, defaultCursor);
	}
	b = b->next;
    }

    return(ok);
}

void Desktop::UnGrabEm () {
    register ButtonList* b = dispatch->Buttons();
    unsigned button, mask;

    while (b != nil) {
	ConvertButtonMask(b->mask, button, mask);
	UngrabButton(button, mask);
	b = b->next;
    }
}

void Desktop::Run () {
    Event e;
    RemoteInteractor i;

    GrabEm();
    if (logo != nil) {
	world->Insert(logo, logo_x, logo_y);
    }
    running = true;
    while (running) {
	Read(e);
	if (e.eventType == DownEvent) {
	    if (dispatch->NoFunction(e) && FindIcon(Find(e.x, e.y)) != nil) {
		DoIconify(e);
	    } else {
		dispatch->Perform(e);
	    }
	}
    }
    if (logo != nil) {
	world->Remove(logo);
    }
    UnGrabEm();
    while (icons != nil) {
	i = GetIcon(icons->ri);
	if (i != nil) {
	    Map(i);
	}
	Unmap(icons->ri);
	if (i != nil) {
	    UnassignIcon(i);
	}
	delete icons->icon->Parent();
	icons = icons->next;
    }
    delete dispatch;
    delete logo;
    delete tracking;
    Sync();
}

char* DesktopFunctionNames[Operations] = {
    "Focus", "Lower", "Raise", "Redraw", "Repair", "Iconify",
    "Move", "Resize", "Lock", "Exit", "Null"
};

DesktopFunction DesktopFunctions (OperationCode op) {
    switch(op) {
	case FOCUS:
	    return(DesktopFunction(&Desktop::DoFocus));
	case LOWER:
	    return(DesktopFunction(&Desktop::DoLower));
	case RAISE:
	    return(DesktopFunction(&Desktop::DoRaise));
	case REDRAW:
	    return(DesktopFunction(&Desktop::DoRedraw));
	case REPAIR:
	    return(DesktopFunction(&Desktop::DoRepair));
	case ICONIFY:
	    return(DesktopFunction(&Desktop::DoIconify));
	case MOVE:
	    return(DesktopFunction(&Desktop::DoMove));
	case RESIZE:
	    return(DesktopFunction(&Desktop::DoResize));
	case SCREENLOCK:
	    return(DesktopFunction(&Desktop::DoLock));
	case EXIT:
	    return(DesktopFunction(&Desktop::DoExit));
	case NULL_OP:
	    return(nil);
    }
}

/*
 * Ignore all pending input--we want to synchronize with the user
 * starting now.
 */

void Desktop::SyncInput () {
    Event e;

    while (Check()) {
	Read(e);
    }
}

void Desktop::InsertRemote (void* i) {
    Coord x, y, x1, y1, x2, y2, tmp;
    int w, h;
    Shape s;
    RubberRect* r;
    Event e;

    if (TransientOwner(i) != nil) {
	MapRaised(i);
	return;
    }
    GetInfo(i, x1, y1, x2, y2);
    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    if (GetHints(i, x, y, s)) {
	if (x != x1 || y != y2 || s.width != w || s.height != h) {
	    Change(i, x, y, s.width, s.height);
	}
	MapRaised(i);
	return;
    }
    SyncInput();
    GrabMouse(upperleft);
    do {
	Read(e);
    } while (e.eventType != DownEvent);
    e.GetAbsolute(x1, y1);
    if (e.button == LEFTMOUSE && w > 1) {
	x2 = x1 + w - 1;
	y2 = y1 + h - 1;
	r = new SlidingRect(output, canvas, x1, y1, x2, y2, x1, y2);
	r->Track(x1, y1);
    } else {
	do {
	    Read(e);
	    e.GetAbsolute(x2, y2);
	    x2 = e.x; y2 = e.y;
	} while (x2 == x1 && y2 == y1 && e.eventType != UpEvent);
	r = new RubberRect(output, canvas, x1, y1, x2, y1);
	r->Track(x2, y1);
	GrabMouse(lowerright);
    }
    Listen(tracking);
    Lock();
    while (e.eventType != UpEvent) {
	Read(e);
	e.GetAbsolute(x1, y1);
	x1 = e.x; y1 = e.y;
	r->Track(x1, y1);
    }
    Unlock();
    UngrabMouse();
    Listen(input);
    r->GetCurrent(x1, y1, x2, y2);
    delete r;
    if (x1 > x2) {
	tmp = x1; x1 = x2; x2 = tmp;
    }
    if (y1 > y2) {
	tmp = y1; y1 = y2; y2 = tmp;
    }
    s.width = x2 - x1 + 1;
    s.height = y2 - y1 + 1;
    SetHints(i, x1, y2, s);
    Change(i, x1, y2, s.width, s.height);
    MapRaised(i);
}

void Desktop::DoFocus (Event& e) {
    RemoteInteractor i;

    if (e.eventType == UpEvent) {
	i = Choose(crosshairs, false);
    } else {
	i = Find(e.x, e.y);
    }
    Focus(i);
}

void Desktop::DoLower (Event& e) {
    RemoteInteractor i;

    if (e.eventType == UpEvent) {
	i = Choose(crosshairs, false);
    } else {
	i = Find(e.x, e.y);
    }
    if (i != nil) {
	Lower(i);
    }
}

void Desktop::DoRaise (Event& e) {
    RemoteInteractor i;

    if (e.eventType == UpEvent) {
	i = Choose(crosshairs, false);
    } else {
	i = Find(e.x, e.y);
    }
    if (i != nil) {
	Raise(i);
    }
}

void Desktop::DoRedraw (Event&) {
    RedrawAll();
}

class Cover : public Interactor {
public:
    Cover(int w, int h);
};

Cover::Cover (int w, int h) {
    shape->Rect(w, h);
}

void Desktop::DoRepair (Event& e) {
    Coord x1, y1, x2, y2;
    RubberRect* r;

    GrabMouse(upperleft);
    do {
	Poll(e);
    } while (!e.leftmouse && !e.middlemouse && !e.rightmouse);
    GrabMouse(lowerright);

    Lock();
    e.GetAbsolute(x1, y1);
    r = new RubberRect(output, canvas, x1, y1, x1, y1);
    do {
	Poll(e);
	e.GetAbsolute(x2, y2);
	x2 = e.x; y2 = e.y;
	r->Track(x2, y2);
    } while (e.leftmouse || e.middlemouse || e.rightmouse);
    UngrabMouse();
    r->GetCurrent(x1, y1, x2, y2);
    delete r;
    Unlock();

    Cover* dummy = new Cover(abs(x1-x2)+1, abs(y1-y2)+1);
    world->Insert(dummy, min(x1, x2), min(y1, y2));
    world->Remove(dummy);
    delete dummy;
    Flush();
}

void Desktop::AddIcon (RemoteInteractor r, Icon* icon) {
    register IconList* i = new IconList;
    i->ri = r;
    i->icon = icon;
    i->next = icons;
    icons = i;
}

void Desktop::RemoveIcon (RemoteInteractor ri) {
    register IconList* i, * prev;

    prev = nil;
    for (i = icons; i != nil; i = i->next) {
	if (i->ri == ri) {
	    if (prev != nil) {
		prev->next = i->next;
	    } else {
		icons = i->next;
	    }
	    i->ri = GetIcon(i->ri);
	    if (i->ri != nil) {
		UnassignIcon(i->ri);
	    }
	    delete i->icon->Parent();
	    delete i;
	    break;
	}
	prev = i;
    }
}

boolean Desktop::IconIsOld (RemoteInteractor ri, const char* s) {
    Icon* i = FindIcon(ri);
    if (i != nil && i->IsOld(s)) {
	RemoveIcon(ri);
	return true;
    }
    return false;
}

Icon* Desktop::FindIcon (RemoteInteractor ri) {
    register IconList* i;

    for (i = icons; i != nil; i = i->next) {
	if (i->ri == ri) {
	    return i->icon;
	}
    }
    return nil;
}

void Desktop::DoIconify (Event& e) {
    RemoteInteractor i, icon;
    Coord x, y, x1, y1, x2, y2;
    int width, height;
    IconFrame* f = nil;
    Shape* sh;
    Icon* new_icon;
    char* s;

    if (e.eventType == UpEvent) {
	i = Choose(crosshairs, false);
    } else {
	i = Find(e.x, e.y);
    }
    if (i == nil) {
	return;
    }
    icon = GetIcon(i);
    if (FindIcon(i) == nil) {
	/*
	 * Not on our list of icons.  If the remote interactor has an
	 * icon, use it, otherwise create one for it based on its name.
	 */
	s = GetName(i);
	if (icon == nil || IconIsOld(icon, s)) {
	    if (s == nil) {
		s = "<window>";
	    }
	    new_icon = new Icon(s);
	    f = new IconFrame(new_icon);
	    f->Config(world);
	    sh = f->GetShape();
	    width = sh->width;
	    height = sh->height;
	} else {
	    GetInfo(icon, x1, y1, x2, y2);
	    width = x2 - x1;
	    height = y2 - y1;
	}
	Poll(e);
	x = e.x - (width >> 1) + 1;
	if (x < 0) {
	    x = 0;
	}
	if (x + width + 2 > world->Width()) {
	    x = world->Width() - width - 2;
	}
	y = e.y - (height >> 1) + 1;
	if (y < 0) {
	    y = 0;
	}
	if (y + height + 2 > world->Height()) {
	    y = world->Height() - height - 2;
	}
	if (f != nil) {
	    world->Insert(f, x, y);
	    AssignIcon(i, f->RemoteId());
	    AddIcon(f->RemoteId(), new_icon);
	    icon = nil;
	}
    }
    if (icon != nil) {
	MapRaised(icon);
    }
    Unmap(i);
}

void Desktop::DoExit (Event&) {
    running = false;
}

void Desktop::DoMove (Event& e) {
    Coord startleft, startbottom;
    Coord left, bottom, right, top;
    Coord x, y;
    RemoteInteractor i;

    SyncInput();
    GrabMouse(crosshairs);
    while (e.eventType != DownEvent) {
	Read(e);
    }
    e.GetAbsolute(x, y);
    i = Find(x, y);
    if (i == nil) {
	UngrabMouse();
	return;
    }
    GetInfo(i, left, bottom, right, top);
    startleft = left;
    startbottom = bottom;
    Poll(e);
    e.GetAbsolute(x, y);
    SlidingRect r(output, canvas, left, bottom, right, top, x, y);
    Lock();
    r.Draw();
    boolean aborted = false;
    boolean tracking = true;
    while (tracking && !aborted) {
	int buttoncount = 0;
	if ( e.leftmouse ) {
	    ++buttoncount;
	}
	if ( e.middlemouse ) {
	    ++buttoncount;
	}
	if ( e.rightmouse ) {
	    ++buttoncount;
	}
	if ( buttoncount > 1 ) {
	    aborted = true;
	} else if ( buttoncount == 0 ) {
	    tracking = false;
	} else {
	    r.Track(e.x, e.y);
	    Poll(e);
	    e.GetAbsolute(x, y);
	}
    }
    r.Erase();
    Unlock();
    UngrabMouse();
    SyncInput();
    if (!aborted) {
	r.GetCurrent(left, bottom, right, top);
	Move(i, left, top);
    }
}

void Desktop::DoResize (Event& e) {
    Coord left, bottom, right, top;
    Coord xfixed, yfixed, xmoved, ymoved;
    Coord x, y;
    int width, height;
    RemoteInteractor i;

    SyncInput();
    GrabMouse(crosshairs);
    while (e.eventType != DownEvent) {
	Read(e);
    }
    e.GetAbsolute(x, y);
    i = Find(x, y);
    if (i == nil) {
	UngrabMouse();
	return;
    }
    GetInfo(i, left, bottom, right, top);
    width = right - left + 1;
    height = top - bottom + 1;
    Poll(e);
    e.GetAbsolute(x, y);
    SizingRect r(
	output, canvas,
	left, bottom, right, top, x, y,
	snap_resize, constrain_resize
    );
    r.Draw();
    boolean aborted = false;
    boolean tracking = true;
    while (tracking && !aborted) {
	int buttoncount = 0;
	if (e.leftmouse) {
	    ++buttoncount;
	}
	if (e.middlemouse) {
	    ++buttoncount;
	}
	if (e.rightmouse) {
	    ++buttoncount;
	}
	if (buttoncount > 1) {
	    aborted = true;
	} else if (buttoncount == 0) {
	    tracking = false;
	} else {
	    r.Track(x, y);
	    Poll(e);
	    e.GetAbsolute(x, y);
	}
    }
    r.Erase();
    Unlock();
    UngrabMouse();
    ClearInput();
    if (!aborted) {
	r.GetCurrent(xfixed, yfixed, xmoved, ymoved);
	if (xfixed > xmoved) {
	    width = xfixed - xmoved + 1;
	    left = xmoved;
	} else {
	    width = xmoved - xfixed + 1;
	    left = xfixed;
	}
	if (yfixed > ymoved) {
	    height = yfixed - ymoved + 1;
	    bottom = ymoved;
	} else {
	    height = ymoved - yfixed + 1;
	    bottom = yfixed;
	}
	Change(i, left, bottom + height - 1, width, height);
    }
}

void Desktop::DoLock (Event&) {
    lock->InsertInto(world);
    Focus(Find(world->Width()/2, world->Height()/2));
    GrabMouse(lock->LockCursor());
    if (lock_server) {
	Lock();
    }
    lock->Activate();
    if (lock_server) {
	Unlock();
    }
    UngrabMouse();
    Focus(nil);
    ClearInput();
    lock->RemoveFrom(world);
}

static void BadDefault (const char* msg) {
    fprintf(stderr, "%s: .Xdefaults: %s\n", program_name, msg);
    exit(1);
}

void Desktop::Config () {
    const char* v;
    int w, h;
    unsigned mask;
    int height = world->Height();
    int width = world->Width();

    v = GetAttribute("logo");
    if (v != nil) {
	mask = world->ParseGeometry((char*)v, logo_x, logo_y, w, h);
    } else {
	mask = 0;
    }
    if (mask & GeomHeightValue) {
	if (w != h) {
	    BadDefault("do not specify a height for logo (it is square)");
	    exit(1);
	}
    } else if (!(mask & GeomWidthValue)) {
	/* width should have form 13n + 1 */
	w = 53;
    }
    logosize = w;

    if (mask & GeomYValue) {
	logo_y = (mask & GeomYNegative) ? -logo_y : height - logosize - logo_y;
    } else {
	logo_y = height - logosize - 4;
    }
    if (mask & GeomXValue) {
	logo_x = (mask & GeomXNegative) ? width - logosize + logo_x : logo_x;
    } else {
	logo_x = width - logosize - 4;
    }
    ignorecaps = false;
    v = GetAttribute("ignorecaps");
    if (v != nil) {
	if (strcmp(v, "true") == 0) {
	    ignorecaps = true;
	} else if (strcmp(v, "false") != 0) {
	    BadDefault("iwm.ignorecaps must be 'true' or 'false'");
	}
    }

    lock_server = false;
    v = GetAttribute("lock");
    if (v != nil) {
	if (strcmp(v, "true") == 0) {
	    lock_server = true;
	} else if (strcmp(v, "false") != 0) {
	    BadDefault("iwm.lock must be 'true' or 'false'");
	    exit(1);
	}
    }
    constrain_resize = true;
    v = GetAttribute("constrainresize");
    if (v != nil) {
	if (strcmp(v, "false") == 0) {
	    constrain_resize = false;
	} else if (strcmp(v, "true") != 0) {
	    BadDefault("constrainresize must be 'true' or 'false'");
	}
    }
    snap_resize = false;
    v = GetAttribute("snapresize");
    if (v != nil) {
	if (strcmp(v, "true") == 0) {
	    snap_resize = true;
	} else if (strcmp(v, "false") != 0) {
	    BadDefault("snapresize must be 'true' or 'false'");
	}
    }
    v = GetAttribute("bitmap");
    if (v != nil) {
	logo_bitmap = new Bitmap(v);
	if (logo_bitmap->Width() <= 0) {
	    BadDefault("cannot open bitmap file `%s'");
	}
    }
    v = GetAttribute("Xcursor");
    if (v == nil) {
	SetCursor(arrow);
    }
}
