/*
 * Implementation of Adjuster and derived classes.
 */

#include <InterViews/adjuster.h>
#include <InterViews/bitmap.h>
#include <InterViews/painter.h>
#include <InterViews/perspective.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>

Adjuster::Adjuster (Interactor* i, int d) {
    Init(i, d);
}

Adjuster::Adjuster (const char* name, Interactor* i, int d) {
    SetInstance(name);
    Init(i, d);
}

Adjuster::Adjuster (Interactor* i, int d, Painter* p) : (nil, p) {
    Init(i, d);
}

void Adjuster::Init (Interactor* i, int d) {
    SetClassName("Adjuster");
    view = i;
    highlighted = false;
    delay = d;
    shown = new Perspective;
    plain = nil;
    hit = nil;
    mask = nil;
    input = new Sensor(onoffEvents);
    input->Catch(UpEvent);
    input->Catch(DownEvent);
}    

void Adjuster::Reconfig () {
    Painter* tmp = new Painter(output);
    delete output;
    output = tmp;
    shape->width = mask->Width();
    shape->height = mask->Height();
}

void Adjuster::AutoRepeat () {
    Event e;
    
    Poll(e);	// initialize event
    do {
	if (Check()) {
	    Read(e);
	    if (e.target == this) {
		switch (e.eventType) {
		    case OnEvent:
			Highlight();
			break;
		    case OffEvent:
			UnHighlight();
			break;
		    default:
			break;
		}
	    }		    
	} else if (highlighted) {
	    Flash();
	    AdjustView(e);
	    Sync();
	}
    } while (e.eventType != UpEvent);
}

void Adjuster::HandlePress () {
    Event e;
    
    do {
	Read(e);
	if (e.target == this) {
	    switch (e.eventType) {
		case OnEvent:
		    TimerOn();
		    Highlight();
		    break;
		case OffEvent:
		    TimerOff();
		    UnHighlight();
		    break;
		case UpEvent:
		    if (highlighted) {
			AdjustView(e);
		    }
		    break;
		case TimerEvent:
		    AutoRepeat();
		    return;
		default:
		    break;
	    }
	}
    } while (e.eventType != UpEvent);
}

void Adjuster::Flash () {
    UnHighlight();
    Highlight();
}

static const int USEC_PER_DELAY_UNIT = 100000;	    // delay unit = 1/10 secs

void Adjuster::TimerOn () {
    if (delay >= 0) {
	input->CatchTimer(0, delay * USEC_PER_DELAY_UNIT);
    }
}

void Adjuster::TimerOff () {
    input->Ignore(TimerEvent);
}

void Adjuster::~Adjuster () {
    delete shown;
}

void Adjuster::Handle (Event& e) {
    if (e.eventType == DownEvent) {
	Highlight();
	TimerOn();
	if (delay == 0) {
	    AutoRepeat();
	} else {
	    HandlePress();
	}
	UnHighlight();
	TimerOff();
    }
}

void Adjuster::Redraw (Coord, Coord, Coord, Coord) {
    Coord x = (xmax+1 - mask->Width())/2;
    Coord y = (ymax+1 - mask->Height())/2;
    if (highlighted) {
        output->Stencil(canvas, x, y, hit, mask);
    } else {
        output->Stencil(canvas, x, y, plain, mask);
    }
}

void Adjuster::Reshape (Shape& s) {
    shape->Rigid(s.hshrink, s.hstretch, s.vshrink, s.vstretch);
}

void Adjuster::Highlight () {
    if (!highlighted) {
	highlighted = true;
        Draw();
    }
}

void Adjuster::UnHighlight () {
    if (highlighted) {
	highlighted = false;
	Draw();
    }
}

void Adjuster::AdjustView (Event&) {
    // nop default
}

Zoomer::Zoomer (Interactor* i, float f) : (i, NO_AUTOREPEAT) {
    Init(f);
}

Zoomer::Zoomer (
    const char* name, Interactor* i, float f
) : (name, i, NO_AUTOREPEAT) {
    Init(f);
}

Zoomer::Zoomer (
    Interactor* i, float f, Painter* p
) : (i, NO_AUTOREPEAT, p) {
    Init(f);
}

void Zoomer::Init (float f) {
    SetClassName("Zoomer");
    factor = f;
}

void Zoomer::AdjustView (Event&) {
    register Perspective* s = shown;
    Coord cx, cy;

    *s = *view->GetPerspective();
    cx = s->curx + s->curwidth/2;
    cy = s->cury + s->curheight/2;
    s->curwidth = round(float(s->curwidth) / factor);
    s->curheight = round(float(s->curheight) / factor);
    s->curx = cx - s->curwidth/2;
    s->cury = cy - s->curheight/2;
    view->Adjust(*s);    
}

static const int enl_width = 25;
static const int enl_height = 15;
static unsigned char enl_mask[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   0x00, 0xfe, 0x00, 0x00, 0xc0, 0xff, 0x07, 0x00, 0xf0, 0xff, 0x1f, 0x00,
   0xf0, 0xff, 0x1f, 0x00, 0xf8, 0xff, 0x3f, 0x00, 0xf8, 0xff, 0x3f, 0x00,
   0xf8, 0xff, 0x3f, 0x00, 0xf8, 0xff, 0x3f, 0x00, 0xc0, 0xff, 0x07, 0x00,
   0xc0, 0xff, 0x07, 0x00, 0xc0, 0xff, 0x07, 0x00, 0xc0, 0xff, 0x07, 0x00};
static unsigned char enl_plain[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x38, 0x00, 0x00, 0x00, 0xc6, 0x00, 0x00, 0x80, 0x01, 0x03, 0x00,
   0x60, 0x00, 0x0c, 0x00, 0x18, 0x00, 0x30, 0x00, 0xf8, 0x01, 0x3f, 0x00,
   0x88, 0x00, 0x22, 0x00, 0x78, 0x00, 0x3c, 0x00, 0xc0, 0xff, 0x07, 0x00,
   0x40, 0x00, 0x04, 0x00, 0x40, 0x00, 0x04, 0x00, 0xc0, 0xff, 0x07, 0x00};
static unsigned char enl_hit[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   0x00, 0xee, 0x00, 0x00, 0xc0, 0x01, 0x07, 0x00, 0x30, 0x00, 0x18, 0x00,
   0xf0, 0x83, 0x1f, 0x00, 0x10, 0x01, 0x11, 0x00, 0xf0, 0xff, 0x1f, 0x00,
   0x80, 0x00, 0x02, 0x00, 0x80, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static Bitmap* enlMask;
static Bitmap* enlPlain;
static Bitmap* enlHit;

Enlarger::Enlarger (Interactor* i) : (i, 2.0) {
    Init();
}

Enlarger::Enlarger (const char* name, Interactor* i) : (name, i, 2.0) {
    Init();
}

Enlarger::Enlarger (Interactor* i, Painter* p) : (i, 2.0, p) {
    Init();
}

void Enlarger::Init () {
    SetClassName("Enlarger");
    if (enlMask == nil) {
        enlMask = new Bitmap(enl_mask, enl_width, enl_height);
        enlPlain = new Bitmap(enl_plain, enl_width, enl_height);
        enlHit = new Bitmap(enl_hit, enl_width, enl_height);
    }
    mask = enlMask;
    plain = enlPlain;
    hit = enlHit;
    shape->Rigid(shape->width/2, hfil, shape->height/2);
}

static const int red_width = 25;
static const int red_height = 15;
static unsigned char red_mask[] = {
   0x00, 0x7c, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0xff, 0x01, 0x00,
   0xf8, 0xff, 0x3f, 0x00, 0xf8, 0xff, 0x3f, 0x00, 0xff, 0xff, 0xff, 0x01,
   0xff, 0xff, 0xff, 0x01, 0xff, 0xff, 0xff, 0x01, 0xff, 0xff, 0xff, 0x01,
   0xfe, 0xff, 0xff, 0x00, 0xf8, 0xff, 0x3f, 0x00, 0xe0, 0xff, 0x0f, 0x00,
   0x80, 0xff, 0x03, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00};
static unsigned char red_plain[] = {
   0x00, 0x7c, 0x00, 0x00, 0x00, 0x82, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
   0xf8, 0x00, 0x3e, 0x00, 0x38, 0x00, 0x38, 0x00, 0xc8, 0x00, 0x26, 0x00,
   0x08, 0x83, 0x21, 0x00, 0x30, 0x6c, 0x18, 0x00, 0xc0, 0x10, 0x06, 0x00,
   0x00, 0x93, 0x01, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static unsigned char red_hit[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
   0x80, 0x00, 0x02, 0x00, 0x40, 0x00, 0x04, 0x00, 0x3f, 0x00, 0xf8, 0x01,
   0x07, 0x00, 0xc0, 0x01, 0x19, 0x00, 0x30, 0x01, 0x61, 0x00, 0x0c, 0x01,
   0x86, 0x01, 0xc3, 0x00, 0x18, 0xc6, 0x30, 0x00, 0x60, 0x38, 0x0c, 0x00,
   0x80, 0x11, 0x03, 0x00, 0x00, 0xd6, 0x00, 0x00, 0x00, 0x38, 0x00, 0x00};

static Bitmap* redMask;
static Bitmap* redPlain;
static Bitmap* redHit;

Reducer::Reducer (Interactor* i) : (i, 0.5) {
    Init();
}

Reducer::Reducer (const char* name, Interactor* i) : (name, i, 0.5) {
    Init();
}

Reducer::Reducer (Interactor* i, Painter* p) : (i, 0.5, p) {
    Init();
}

void Reducer::Init () {
    SetClassName("Reducer");
    if (redMask == nil) {
        redMask = new Bitmap(red_mask, red_width, red_height);
        redPlain = new Bitmap(red_plain, red_width, red_height);
        redHit = new Bitmap(red_hit, red_width, red_height);
    }
    mask = redMask;
    plain = redPlain;
    hit = redHit;
    shape->Rigid(shape->width/2, hfil, shape->height/2);
}

typedef enum MoveType { 
    MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN, MOVE_UNDEF
};

Mover::Mover (Interactor* i, int delay, int mt) : (i, delay) {
    Init(mt);
}

Mover::Mover (
    const char* name, Interactor* i, int delay, int mt
) : (name, i, delay) {
    Init(mt);
}

Mover::Mover (
    Interactor* i, int delay, int mt, Painter* p
) : (i, delay, p) {
    Init(mt);
}

void Mover::Init (int mt) {
    SetClassName("Mover");
    moveType = mt;
}

void Mover::AdjustView (Event& e) {
    register Perspective* s = shown;
    int amtx, amty;

    *s = *view->GetPerspective();
    amtx = e.shift ? s->lx : s->sx;
    amty = e.shift ? s->ly : s->sy;

    switch (moveType) {
	case MOVE_LEFT:	    s->curx -= amtx; break;
	case MOVE_RIGHT:    s->curx += amtx; break;
	case MOVE_UP:	    s->cury += amty; break;
	case MOVE_DOWN:	    s->cury -= amty; break;
	default:	    break;
    }
    view->Adjust(*s);    
}

static const int lmover_width = 11;
static const int lmover_height = 11;
static unsigned char lmover_mask[] = {
   0x00, 0x00, 0x60, 0x00, 0x70, 0x00, 0xf8, 0x03, 0xfc, 0x07, 0xfe, 0x07,
   0xfc, 0x07, 0xf8, 0x07, 0xf0, 0x07, 0xe0, 0x00, 0xc0, 0x00};
static unsigned char lmover_plain[] = {
   0x00, 0x00, 0x60, 0x00, 0x50, 0x00, 0xc8, 0x03, 0x04, 0x06, 0x02, 0x06,
   0x04, 0x06, 0xc8, 0x07, 0xd0, 0x07, 0xe0, 0x00, 0xc0, 0x00};
static unsigned char lmover_hit[] = {
   0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0xa0, 0x00, 0x90, 0x07, 0x08, 0x04,
   0x04, 0x04, 0x08, 0x04, 0x90, 0x07, 0xa0, 0x00, 0xc0, 0x00};

static Bitmap* lmoverMask;
static Bitmap* lmoverPlain;
static Bitmap* lmoverHit;

LeftMover::LeftMover (Interactor* i, int delay) : (i, delay, MOVE_LEFT) {
    Init();
}

LeftMover::LeftMover (
    const char* name, Interactor* i, int delay
) : (name, i, delay, MOVE_LEFT) {
    Init();
}

LeftMover::LeftMover (
    Interactor* i, int delay, Painter* p
) : (i, delay, MOVE_LEFT, p) {
    Init();
};

void LeftMover::Init () {
    SetClassName("LeftMover");
    if (lmoverMask == nil) {
        lmoverMask = new Bitmap(lmover_mask, lmover_width, lmover_height);
        lmoverPlain = new Bitmap(lmover_plain, lmover_width, lmover_height);
        lmoverHit = new Bitmap(lmover_hit, lmover_width, lmover_height);
    }
    mask = lmoverMask;
    plain = lmoverPlain;
    hit = lmoverHit;
    shape->Rigid(shape->width/2, 0, shape->height/2, vfil);
}

static const int rmover_width = 11;
static const int rmover_height = 11;
static unsigned char rmover_mask[] = {
   0x00, 0x00, 0x30, 0x00, 0x70, 0x00, 0xfe, 0x00, 0xfe, 0x01, 0xfe, 0x03,
   0xfe, 0x07, 0xfe, 0x03, 0xfc, 0x01, 0xf0, 0x00, 0x60, 0x00};
static unsigned char rmover_plain[] = {
   0x00, 0x00, 0x30, 0x00, 0x50, 0x00, 0x9e, 0x00, 0x02, 0x01, 0x02, 0x02,
   0x02, 0x07, 0x9e, 0x03, 0xdc, 0x01, 0xf0, 0x00, 0x60, 0x00};
static unsigned char rmover_hit[] = {
   0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0xa0, 0x00, 0x3c, 0x01, 0x04, 0x02,
   0x04, 0x04, 0x04, 0x02, 0x3c, 0x01, 0xa0, 0x00, 0x60, 0x00};

static Bitmap* rmoverMask;
static Bitmap* rmoverPlain;
static Bitmap* rmoverHit;

RightMover::RightMover (Interactor* i, int delay) : (i, delay, MOVE_RIGHT) {
    Init();
}

RightMover::RightMover (
    const char* name, Interactor* i, int delay
) : (name, i, delay, MOVE_RIGHT) {
    Init();
}

RightMover::RightMover (
    Interactor* i, int delay, Painter* p
) : (i, delay, MOVE_RIGHT, p) {
    Init();
};

void RightMover::Init () {
    SetClassName("RightMover");
    if (rmoverMask == nil) {
        rmoverMask = new Bitmap(rmover_mask, rmover_width, rmover_height);
        rmoverPlain = new Bitmap(rmover_plain, rmover_width, rmover_height);
        rmoverHit = new Bitmap(rmover_hit, rmover_width, rmover_height);
    }
    mask = rmoverMask;
    plain = rmoverPlain;
    hit = rmoverHit;
    shape->Rigid(shape->width/2, 0, shape->height/2, vfil);
}

static const int umover_width = 11;
static const int umover_height = 11;
static unsigned char umover_mask[] = {
   0x00, 0x00, 0x20, 0x00, 0x70, 0x00, 0xf8, 0x00, 0xfc, 0x01, 0xfe, 0x03,
   0xfe, 0x07, 0xf8, 0x07, 0xf8, 0x01, 0xf8, 0x01, 0xf0, 0x01};
static unsigned char umover_plain[] = {
   0x00, 0x00, 0x20, 0x00, 0x50, 0x00, 0x88, 0x00, 0x04, 0x01, 0x02, 0x02,
   0x8e, 0x07, 0x88, 0x07, 0x88, 0x01, 0xf8, 0x01, 0xf0, 0x01};
static unsigned char umover_hit[] = {
   0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0xa0, 0x00, 0x10, 0x01, 0x08, 0x02,
   0x04, 0x04, 0x1c, 0x07, 0x10, 0x01, 0x10, 0x01, 0xf0, 0x01};

static Bitmap* umoverMask;
static Bitmap* umoverPlain;
static Bitmap* umoverHit;

UpMover::UpMover (Interactor* i, int delay) : (i, delay, MOVE_UP) {
    Init();
}

UpMover::UpMover (
    const char* name, Interactor* i, int delay
) : (name, i, delay, MOVE_UP) {
    Init();
}

UpMover::UpMover (
    Interactor* i, int delay, Painter* p
) : (i, delay, MOVE_UP, p) {
    Init();
};

void UpMover::Init () {
    SetClassName("UpMover");
    if (umoverMask == nil) {
        umoverMask = new Bitmap(umover_mask, umover_width, umover_height);
        umoverPlain = new Bitmap(umover_plain, umover_width, umover_height);
        umoverHit = new Bitmap(umover_hit, umover_width, umover_height);
    }
    mask = umoverMask;
    plain = umoverPlain;
    hit = umoverHit;
    shape->Rigid(shape->width/2, hfil, shape->height/2);
}

static const int dmover_width = 11;
static const int dmover_height = 11;
static unsigned char dmover_mask[] = {
   0x00, 0x00, 0xf8, 0x00, 0xf8, 0x01, 0xf8, 0x01, 0xfe, 0x03, 0xfe, 0x07,
   0xfc, 0x07, 0xf8, 0x03, 0xf0, 0x01, 0xe0, 0x00, 0x40, 0x00};
static unsigned char dmover_plain[] = {
   0x00, 0x00, 0xf8, 0x00, 0x88, 0x01, 0x88, 0x01, 0x8e, 0x03, 0x02, 0x06,
   0x04, 0x07, 0x88, 0x03, 0xd0, 0x01, 0xe0, 0x00, 0x40, 0x00};
static unsigned char dmover_hit[] = {
   0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x10, 0x01, 0x10, 0x01, 0x1c, 0x07,
   0x04, 0x04, 0x08, 0x02, 0x10, 0x01, 0xa0, 0x00, 0x40, 0x00};

static Bitmap* dmoverMask;
static Bitmap* dmoverPlain;
static Bitmap* dmoverHit;

DownMover::DownMover (Interactor* i, int delay) : (i, delay, MOVE_DOWN) {
    Init();
}

DownMover::DownMover (
    const char* name, Interactor* i, int delay
) : (name, i, delay, MOVE_DOWN) {
    Init();
}

DownMover::DownMover (
    Interactor* i, int delay, Painter* p
) : (i, delay, MOVE_DOWN, p) {
    Init();
};

void DownMover::Init () {
    SetClassName("DownMover");
    if (dmoverMask == nil) {
        dmoverMask = new Bitmap(dmover_mask, dmover_width, dmover_height);
        dmoverPlain = new Bitmap(dmover_plain, dmover_width, dmover_height);
        dmoverHit = new Bitmap(dmover_hit, dmover_width, dmover_height);
    }
    mask = dmoverMask;
    plain = dmoverPlain;
    hit = dmoverHit;
    shape->Rigid(shape->width/2, hfil, shape->height/2);
}
