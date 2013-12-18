/*
 * Implementation of the base class of interaction.
 */

#include "itable.h"
#include <InterViews/canvas.h>
#include <InterViews/interactor.h>
#include <InterViews/paint.h>
#include <InterViews/painter.h>
#include <InterViews/propsheet.h>
#include <InterViews/scene.h>
#include <InterViews/shape.h>
#include <InterViews/sensor.h>
#include <InterViews/strtable.h>
#include <os/ipc.h>
#include <stdio.h>
#include <string.h>

#if defined(hpux)
#   include <time.h>
#else
#   include <sys/time.h>
#endif

StringTable* nameTable;

class DefaultProps {
public:
    PropertyDef font;
    PropertyDef foreground;
    PropertyDef background;
    PropertyDef geometry;
    PropertyDef icongeometry;
    PropertyDef iconic;
    PropertyDef reverseVideo;
    PropertyDef title;
    boolean rootReversed;
};

static DefaultProps* props;

class UnReadEvent {
    friend class Interactor;

    void* w;
    Event e;
    UnReadEvent* next;
};

static UnReadEvent* unread;

class TopLevel {
    friend class Interactor;

    const char* name;
    const char* geometry;
    InteractorType interactortype;
    CanvasType canvastype;
    Cursor* cursor;
    Interactor* groupleader;
    Interactor* transientfor;
    boolean starticonic;

    const char* icon_name;
    Interactor* icon_interactor;
    const char* icon_geometry;
    Bitmap* icon_bitmap;
    Bitmap* icon_mask;

    TopLevel();
};

Interactor::Interactor () {
    Init();
}

Interactor::Interactor (const char* name) {
    Init();
    SetInstance(name);
}

Interactor::Interactor (Sensor* in, Painter* out) {
    Init();
    if (in != nil) {
	input = in;
	in->Reference();
    }
    if (out != nil) {
	output = out;
	out->Reference();
    }
}

void Interactor::Init () {
    shape = new Shape;
    canvas = nil;
    perspective = nil;
    xmax = 0;
    ymax = 0;
    input = nil;
    output = nil;
    toplevel = nil;
    classname = nil;
    instance = nil;
    parent = nil;
    left = 0;
    bottom = 0;
    cursensor = nil;
}

Interactor::~Interactor () {
    delete canvas;
    delete input;
    delete output;
    delete shape;
    delete toplevel;
}

/*
 * Read an event.  Check for previously read events that were put back
 * with UnRead; check for non-workstation events (timer or channel); then
 * check for an input event.  We have to loop because some out-of-band
 * events may come along the way (e.g., exposures).
 */

void Interactor::Read (Event& e) {
    for (;;) {
	if (unread != nil && PrevEvent(e)) {
	    break;
	}
	if (CheckQueue() == 0 && cursensor != nil &&
	    (cursensor->timer || cursensor->channels != 0) &&
	    Select(e)
	) {
	    break;
	}
	if (GetEvent(e, true)) {
	    break;
	}
    }
}

/*
 * Do a select on input and desired channels, also catching timeouts
 * if desired.  Return true if the event is NOT an input event.
 */

boolean Interactor::Select (Event& e) {
#if defined(no_select)
    /* systems that do not have select */
    return false;
#else
    register Sensor* s;
    unsigned d, dmask, m, rd;
    struct timeval tv, * t;
    int r;

    Flush();
    s = cursensor;
    d = Fileno();
    dmask = 1 << d;
    m = max(d+1, s->maxchannel);
    rd = s->channels | dmask;
    if (s->timer) {
	tv.tv_sec = s->sec;
	tv.tv_usec = s->usec;
	t = &tv;
    } else {
	t = nil;
    }
    for (;;) {
	e.channel = rd;
	r = select(m, &e.channel, 0, 0, t);
	if (r > 0) {
	    e.target = this;
	    e.eventType = ChannelEvent;
	    return (e.channel & dmask) == 0;
	} else if (r == 0) {
	    e.target = this;
	    e.eventType = TimerEvent;
	    return true;
	}
	/* otherwise probably interrupted system call, try again */
    }
#endif
}

void Interactor::UnRead (Event& e) {
    if (e.target != nil && e.target->canvas != nil) {
	register UnReadEvent* u;

	u = new UnReadEvent;
	u->w = e.target->canvas->id;
	u->e = e;
	u->next = unread;
	unread = u;
    }
}

boolean Interactor::PrevEvent (Event& e) {
    extern InteractorTable* assocTable;
    register UnReadEvent* u;
    Interactor* i;

    for (u = unread; u != nil; u = unread) {
	unread = u->next;
	if (assocTable->Find(i, u->w) && i == u->e.target) {
	    e = u->e;
	    delete u;
	    return true;
	}
	delete u;
    }
    return false;
}

/*
 * Set (x,y) for a given alignment of a shape within a canvas.
 */

void Interactor::Align (
    Alignment a, int width, int height, Coord& l, Coord& b
) {
    switch (a) {
	case Left:
	case TopLeft:
	case CenterLeft:
	case BottomLeft:
	    l = 0;
	    break;
	case TopCenter:
	case Center:
	case BottomCenter:
	case HorizCenter:
	    l = (xmax + 1 - width) / 2;
	    break;
	case Right:
	case TopRight:
	case CenterRight:
	case BottomRight:
	    l = xmax + 1 - width;
	    break;
    }
    switch (a) {
	case Bottom:
	case BottomLeft:
	case BottomCenter:
	case BottomRight:
	    b = 0;
	    break;
	case CenterLeft:
	case Center:
	case CenterRight:
	case VertCenter:
	    b = (ymax + 1 - height) / 2;
	    break;
	case Top:
	case TopLeft:
	case TopCenter:
	case TopRight:
	    b = ymax + 1 - height;
	    break;
    }
}

void Interactor::Adjust (Perspective&) {
    /* default is to ignore */
}

void Interactor::Update () {
    /* default is to ignore */
}

/*
 * Start a configuration traversal of an interactor hierarchy,
 * assigning parents, output painters, and calling Reconfig
 * for each interactor.
 *
 * This routine only works correctly when "s" is a world.
 * In any other case, it correctly assigns parents but does not
 * set up the property path for attribute lookup.  It should go up
 * from "s" to the world, then come down pushing property directories
 * in the same manner as DoConfig.  Instead of passing rootReversed
 * to DoConfig, it should compute "reverseVideo" for "s" and
 * pass whether it is on or not.
 */

void Interactor::Config (Scene* s) {
    if (parent != s) {
	if (parent != nil) {
	    parent->Remove(this);
	}
	parent = s;
	DoConfig(props->rootReversed);
    }
}

/*
 * Add a <class, instance> pair to the property search path.
 */

static void EnterDirs (
    PropertyName c, PropertyName i, PropDir*& classdir, PropDir*& instdir
) {
    classdir = (c == nil) ? nil : properties->Find(c);
    instdir = (i == nil) ? nil : properties->Find(i);
    if (classdir != nil) {
	properties->Push(classdir, false);
    }
    if (instdir != nil) {
	properties->Push(instdir, classdir != nil);
    }
}

/*
 * Check to see if any standard interactor properties
 * are defined local to the given directory.
 * The "long circuit" or ("|") is necessary because we must look for
 * all the properties and return true if one or more are specified.
 */

static boolean DefinesProperties (PropDir* dir) {
    return (
	properties->GetLocal(dir, props->font) |
	properties->GetLocal(dir, props->foreground) |
	properties->GetLocal(dir, props->background) |
	properties->GetLocal(dir, props->reverseVideo)
    );
}

/*
 * See if any painter attributes are defined for the current
 * class or instance.  We check both because a partial set
 * of values might be specified in each.  The instance check is
 * second so that its values take override class values.
 */

static boolean CheckDirs (PropDir* classdir, PropDir* instdir) {
    boolean foundany = false;
    if (classdir != nil && DefinesProperties(classdir)) {
	foundany = true;
    }
    if (instdir != nil && DefinesProperties(instdir)) {
	foundany = true;
    }
    return foundany;
}

static Color* ValidColor (const char* name) {
    register Color* c;

    if (name == nil) {
	c = nil;
    } else {
	c = new Color(name);
	if (!c->Valid()) {
	    fprintf(stderr, "Color '%s' is not valid\n", name);
	    c = nil;
	}
    }
    return c;
}

/*
 * Create a new painter based on the one or more properties that
 * were set as a side effect of the DefinesProperties call.
 * All other attributes are inherited from the given painter.
 */

static Painter* MakeNewPainter (Painter* old, boolean& reversed) {
    Painter* output = new Painter(old);
    register DefaultProps* p = props;

    /* font */
    if (p->font.value != nil) {
	Font* f = new Font(p->font.value);
	if (!f->Valid()) {
	    fprintf(stderr, "Font '%s' is not valid\n", p->font.value);
	} else {
	    output->SetFont(f);
	}
    }

    /* reverse video */
    enum { rvOn, rvOff, rvInherit } rv = rvInherit;
    if (p->reverseVideo.value != nil) {
	if (strcmp(p->reverseVideo.value, "on") == 0) {
	    rv = rvOn;
	} else if (strcmp(p->reverseVideo.value, "off") == 0) {
	    rv = rvOff;
	} else {
	    /* error message? */
	}
    }
    if (rv == rvOff && reversed) {
	output->SetColors(output->GetBgColor(), output->GetFgColor());
	reversed = false;
    }

    /* foreground, background colors */
    Color* fg = ValidColor(p->foreground.value);
    Color* bg = ValidColor(p->background.value);
    if (fg != nil || bg != nil) {
	if (rv == rvOn) {
	    output->SetColors(bg, fg);
	} else {
	    output->SetColors(fg, bg);
	}
    } else if (rv == rvOn && !reversed) {
	output->SetColors(output->GetBgColor(), output->GetFgColor());
	reversed = true;
    }
    return output;
}

/*
 * Set any window-related properties that are defined locally to this
 * top-level interactor.
 */

static void SetToplevelProperties (
    Interactor* i, PropDir* classdir, PropDir* instdir
) {
    props->geometry.value = nil;
    props->icongeometry.value = nil;
    props->iconic.value = nil;
    props->title.value = nil;
    if (classdir != nil) {
	properties->GetLocal(classdir, props->geometry);
	properties->GetLocal(classdir, props->icongeometry);
	properties->GetLocal(classdir, props->iconic);
	properties->GetLocal(classdir, props->title);
    }
    if (instdir != nil) {
	properties->GetLocal(instdir, props->geometry);
	properties->GetLocal(instdir, props->icongeometry);
	properties->GetLocal(instdir, props->iconic);
	properties->GetLocal(instdir, props->title);
    }
    if (props->geometry.value != nil) {
	i->SetGeometry(props->geometry.value);
    }
    if (props->icongeometry.value != nil) {
	i->SetIconGeometry(props->icongeometry.value);
    }
    if (props->iconic.value != nil) {
	if (strcmp(props->iconic.value, "on") == 0) {
	    i->SetStartIconic(true);
	} else if (strcmp(props->iconic.value, "off") == 0) {
	    i->SetStartIconic(false);
	}
    }
    if (props->title.value != nil) {
	i->SetName(props->title.value);
    }
}

static void LeaveDirs (PropDir* classdir, PropDir* instdir) {
    if (instdir != nil) {
	properties->Pop();
    }
    if (classdir != nil) {
	properties->Pop();
    }
}

/*
 * Initialize the configuration information for the root interactor (World).
 */

void Interactor::RootConfig () {
    register DefaultProps* p = props;
    PropDir* classdir, * instdir;

    if (p == nil) {
	p = new DefaultProps;
	p->font.name = nameTable->Id("font");
	p->foreground.name = nameTable->Id("foreground");
	p->background.name = nameTable->Id("background");
	p->geometry.name = nameTable->Id("geometry");
	p->icongeometry.name = nameTable->Id("iconGeometry");
	p->iconic.name = nameTable->Id("iconic");
	p->reverseVideo.name = nameTable->Id("reverseVideo");
	p->title.name = nameTable->Id("title");
	props = p;
    }
    p->rootReversed = false;
    if (DefinesProperties(properties->Root())) {
	stdpaint = MakeNewPainter(stdpaint, p->rootReversed);
    }
    EnterDirs(classname, instance, classdir, instdir);
    output = stdpaint;
    if (CheckDirs(classdir, instdir)) {
	DefaultConfig(p->rootReversed);
    } else {
	output->Reference();
    }
}

/*
 * Configure an interactor.  This implies first configuring
 * all of its children, then calling the Reconfig virtual.
 * We automatically setup the interactor's painter and check
 * for any local properties before calling Reconfig.
 *
 * DoConfig is passed a flag indicating whether this interactor's
 * parent painter has had its foreground and background colors reversed.
 * We copy this into a local that DefaultConfig will modify
 * if a reverseVideo attribute is specified that swaps the colors.
 * A swap occurs if either reversed is false and reverseVideo:on is found
 * or reversed is true and reverseVideo:off is found.
 */

void Interactor::DoConfig (boolean parentReversed) {
    Interactor* children[100];
    Interactor** a;
    int n;
    PropDir* classdir, * instdir;
    boolean reversed;

    EnterDirs(classname, instance, classdir, instdir);
    reversed = parentReversed;
    if (output == nil) {
	output = parent->output;
	if (CheckDirs(classdir, instdir)) {
	    DefaultConfig(reversed);
	} else {
	    output->Reference();
	}
    }
    if (parent->parent == nil) { /* top-level interactor? */
	SetToplevelProperties(this, classdir, instdir);
    }
    GetComponents(children, sizeof(children) / sizeof(Interactor*), a, n);
    if (n > 0) {
	register int i;

	for (i = 0; i < n; i++) {
	    a[i]->parent = (Scene*)this;
	    a[i]->DoConfig(reversed);
	}
	if (a != children) {
	    delete a;
	}
    }
    Reconfig();
    LeaveDirs(classdir, instdir);
}

/*
 * Configure an interactor from the standard properties,
 * at least one of which has been set specially for this class or instance.
 */

void Interactor::DefaultConfig (boolean& reversed) {
    output = MakeNewPainter(output, reversed);
}

/*
 * We assume GetAttribute is called from an interactor's Reconfig method
 * so that the property path is correct.
 */

const char* Interactor::GetAttribute (const char* name) {
    PropertyDef p;

    p.name = nameTable->Id(name);
    properties->Get(p);
    return p.value;
}

boolean Interactor::IsMapped () {
    return canvas != nil && canvas->status == CanvasMapped;
}

void Interactor::GetComponents (Interactor**, int, Interactor**&, int& n) {
    n = 0;
}

void Interactor::Handle (class Event& e) {
    /* default is to ignore */
}

void Interactor::Draw () {
    if (canvas != nil && canvas->status != CanvasUnmapped) {
	Redraw(0, 0, xmax, ymax);
    }
}

void Interactor::Reconfig () {
    /* default is to do nothing */
}

void Interactor::Redraw (Coord left, Coord bottom, Coord right, Coord top) {
    /* default is to ignore */
}

/*
 * Default is to redraw each area separately.
 */

void Interactor::RedrawList (
    int n, Coord left[], Coord bottom[], Coord right[], Coord top[]
) {
    register int i;

    for (i = 0; i < n; i++) {
	Redraw(left[i], bottom[i], right[i], top[i]);
    }
}

/*
 * Default is to accept a new shape completely and
 * notify the parent (if set).
 */

void Interactor::Reshape (Shape& ns) {
    *shape = ns;
    if (parent != nil) {
	parent->Change(this);
    }
}

void Interactor::Resize () {
    /* default is to ignore */
}

void Interactor::Activate () {
    /* default is to ignore */
}

void Interactor::Deactivate () {
    /* default is to ignore */
}

Scene* Interactor::Root () {
    register Scene* s = parent;
    if (s != nil) {
	register Scene* t;

	for (t = s->parent; t != nil; t = t->parent) {
	    s = t;
	}
    } else if (IsMapped()) {
	s = (Scene*)this;	/* I'm the world */
    }
    return s;
}

World* Interactor::GetWorld () {
    register Scene* r = nil;
    register Scene* t = nil;

    for (t = (Scene*)this; t != nil; t = t->parent) {
	r = t;
    }

    return r->IsMapped() ? (World*)r : nil;
}

void Interactor::Run () {
    Event e;

    do {
	Read(e);
	e.target->Handle(e);
    } while (e.target != nil);
}

void Interactor::QuitRunning (Event& e) {
    e.target = nil;
}

void Interactor::SetClassName (const char* name) {
    if (name == nil) {
	classname = nil;
    } else {
	classname = nameTable->Id(name);
    }
}

const char* Interactor::GetClassName () {
    return classname == nil ? nil : classname->Str();
}

void Interactor::SetInstance (const char* name) {
    if (name == nil) {
	instance = nil;
    } else {
	instance = nameTable->Id(name);
    }
}

const char* Interactor::GetInstance () {
    return instance == nil ? nil : instance->Str();
}

TopLevel::TopLevel () {
    name = nil;
    geometry = nil;
    interactortype = InteriorInteractor;
    canvastype = CanvasInputOutput;
    cursor = nil;
    groupleader = nil;
    transientfor = nil;
    starticonic = false;
    icon_name = nil;
    icon_bitmap = nil;
    icon_geometry = nil;
    icon_interactor = nil;
    icon_mask = nil;
}

TopLevel* Interactor::GetTopLevel () {
    if (toplevel == nil) {
	toplevel = new TopLevel;
    }
    return toplevel;
}

void Interactor::SetCursor (Cursor* c) {
    TopLevel* t = GetTopLevel();
    if (t->cursor != c) {
	t->cursor = c;
	if (canvas != nil) {
	    DoSetCursor(c);
	}
    }
}

Cursor* Interactor::GetCursor () {
    return (toplevel == nil) ? nil : toplevel->cursor;
}

void Interactor::SetName (const char* s) {
    GetTopLevel()->name = s;
    if (canvas != nil) {
	DoSetName(s);
    }
}

const char* Interactor::GetName () {
    return (toplevel == nil) ? nil : toplevel->name;
}

void Interactor::SetGeometry (const char* g) {
    GetTopLevel()->geometry = g;
    /*
     * I don't want to write code to configure mapped windows unless
     * a lot of people want this functionality
     */
}

const char* Interactor::GetGeometry () {
    return (toplevel == nil) ? nil : toplevel->geometry;
}

void Interactor::SetCanvasType (CanvasType t) {
    GetTopLevel()->canvastype = t;
    /* too much work to change mapped interactors' canvas types */
}

CanvasType Interactor::GetCanvasType () {
    return (toplevel == nil) ? CanvasInputOutput : toplevel->canvastype;
}

void Interactor::SetInteractorType (InteractorType w) {
    GetTopLevel()->interactortype = w;
    /* too much work to change mapped interactors' types */
}

InteractorType Interactor::GetInteractorType () {
    return (toplevel == nil) ? InteriorInteractor : toplevel->interactortype;
}

void Interactor::SetGroupLeader (Interactor* g) {
    GetTopLevel()->groupleader = g;
    if (canvas != nil) {
	DoSetGroupLeader(g);
    }
}

Interactor* Interactor::GetGroupLeader () {
    return (toplevel == nil) ? nil : toplevel->groupleader;
}

void Interactor::SetTransientFor (Interactor* i) {
    GetTopLevel()->transientfor = i;
    if (canvas != nil) {
	DoSetTransientFor(i);
    }
}

Interactor* Interactor::GetTransientFor () {
    return (toplevel == nil) ? nil : toplevel->transientfor;
}

void Interactor::SetIconName (const char* name) {
    GetTopLevel()->icon_name = name;
    if (canvas != nil) {
	DoSetIconName(name);
    }
}

const char* Interactor::GetIconName () {
    return (toplevel == nil) ? nil : toplevel->icon_name;
}

void Interactor::SetIconBitmap (Bitmap* b) {
    GetTopLevel()->icon_bitmap = b;
    if (canvas != nil) {
	DoSetIconBitmap(b);
    }
}

Bitmap* Interactor::GetIconBitmap () {
    return (toplevel == nil) ? nil : toplevel->icon_bitmap;
}

void Interactor::SetIconMask (Bitmap* m) {
    GetTopLevel()->icon_mask = m;
    if (canvas != nil) {
	DoSetIconMask(m);
    }
}

Bitmap* Interactor::GetIconMask () {
    return (toplevel == nil) ? nil : toplevel->icon_mask;
}

void Interactor::SetIconInteractor (Interactor* i) {
    GetTopLevel()->icon_interactor = i;
    if (canvas != nil) {
	DoSetIconInteractor(i);
    }
}

Interactor* Interactor::GetIconInteractor () {
    return (toplevel == nil) ? nil : toplevel->icon_interactor;
}

void Interactor::SetIconGeometry (const char* g) {
    GetTopLevel()->icon_geometry = g;
    if (canvas != nil) {
	DoSetIconGeometry(g);
    }
}

const char* Interactor::GetIconGeometry () {
    return (toplevel == nil) ? nil : toplevel->icon_geometry;
}

void Interactor::SetStartIconic (boolean s) {
    GetTopLevel()->starticonic = s;
}

boolean Interactor::GetStartIconic () {
    return toplevel != nil && toplevel->starticonic;
}
