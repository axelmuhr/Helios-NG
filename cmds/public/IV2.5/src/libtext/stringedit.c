/*
 * StringEdit - basic interactive editor for character strings
 */

#include <InterViews/Text/stringedit.h>
#include <InterViews/sensor.h>
#include <InterViews/paint.h>
#include <InterViews/cursor.h>
#include <InterViews/shape.h>
#include <InterViews/painter.h>
#include <InterViews/scene.h>
#include <string.h>
#include <ctype.h>

static const char SEAccept = '\015';		// ^M, CR
static const char SECancel = '\007';		// ^G
static const char SEDeleteBefore = '\177';	// ^?
static const char SEDeleteAfter = '\004';	// ^D
static const char SEErase = '\010';		// ^H
static const char SERestart = '\022';		// ^R
static const char SELeft = '\002';		// ^B
static const char SERight = '\006';		// ^F
static const char SEBegin = '\001';		// ^A
static const char SEEnd = '\005';		// ^E
static const char SESelectAll = '\025';		// ^U
static const char SESelectWord = '\027';	// ^W
static const char SECut = '\030';		// ^X
static const char SEPaste = '\026';		// ^V
static const char SECopy = '\003';		// ^C

static const int SEDoubleClickTime = 200;	// ms
static const int SEDoubleClickOffset = 3;	// pixels

inline int abs (int x) { return (x>=0) ? x : -x; }

StringEdit::StringEdit (const char* sample, int b) {
    Init(sample, b);
}

StringEdit::StringEdit (const char* name, const char* sample, int b) {
    SetInstance(name);
    Init(sample, b);
}

StringEdit::StringEdit (const char* sample, Painter* p, int b) : (nil, p) {
    Init(sample, b);
}

void StringEdit::Init (const char* s, int b) {
    SetClassName("StringEdit");
    selecting = false;
    caret = false;
    border = b;
    sample = s;
    old = sample;
    buffer = new char[SEBufferSize];
    SetCursor(ltextCursor);
    input = new Sensor;
    input->Catch(DownEvent);
    edit = new Sensor;
    edit->Catch(KeyEvent);
    edit->Catch(DownEvent);
    edit->Catch(MotionEvent);
    select = new Sensor;
    select->Catch(MotionEvent);
    select->Catch(UpEvent);
    highlight = nil;
    lasttime = 0;
    lastx = lasty = 0;
    Restart();
    Flash(5);
    Extra(0);
}

void StringEdit::Reconfig () {
    delete highlight;
    highlight = new Painter(output);
    highlight->SetColors(output->GetBgColor(), output->GetFgColor());
    shape->hunits = output->GetFont()->Width("n");
    shape->vunits = output->GetFont()->Height();
    int height = shape->vunits + 2 * border;
    int width = output->GetFont()->Width(sample) + 2 * border;
    shape->Rect(width, height);
    shape->Rigid(0, hfil, 0, 0);
}

StringEdit::~StringEdit () {
    delete edit;
    delete select;
    delete highlight;
    delete buffer;
}

boolean StringEdit::DoubleClick (Event& e) {
    return (
	e.eventType == DownEvent
	&& (e.timestamp - lasttime) < SEDoubleClickTime
	&& (abs(e.x-lastx) + abs(e.y-lasty)) < SEDoubleClickOffset
    );
}

void StringEdit::HideSelection () {
    if (EmptySelection()) {
	HideCaret();
    } else {
	Refresh(prev+1, next-1, output);
    }
}

void StringEdit::ShowSelection () {
    if (EmptySelection()) {
	ShowCaret();
    } else {
	Refresh(prev+1, next-1, highlight);
    }
}

void StringEdit::HideCaret () {
    if (caret && EmptySelection()) {
	Refresh(prev, next, output);
    }
    caret = false;
}

void StringEdit::ShowCaret () {
    if (!caret && EmptySelection()) {
	DrawCaret(next);
    }
    caret = true;
}

void StringEdit::Restart () {
    length = min(strlen(old), SEBufferSize-1);
    strncpy(buffer, old, length);
    Unselect();
}

void StringEdit::DoChar (char c) {
    switch (c) {
    case SERestart :
	Restart(); Draw(); break;
    case SELeft :
	HideSelection();
	prev = max(-1, prev - 1);
	next = prev + 1;
	ShowSelection();
	break;
    case SERight :
	HideSelection();
	next = min(length, next + 1);
	prev = next - 1;
	ShowSelection();
	break;
    case SEBegin:
	HideSelection();
	prev = -1;
	next = prev + 1;
	ShowSelection();
	break;
    case SEEnd:
	HideSelection();
	next = length;
	prev = next - 1;
	ShowSelection();
	break;
    case SEErase :
    case SEDeleteBefore:
	if (! EmptySelection()) {
	    Clear();
	} else if (! AtBegin()) {
	    DeleteChar(prev);
	}
	break;
    case SEDeleteAfter:
	if (! EmptySelection()) {
	    Clear();
	} else if (! AtEnd()) {
	    DeleteChar(next);
	}
	break;
    case SESelectAll :
	Select(); ShowSelection(); break;
    case SESelectWord :
	SelectWord(); ShowSelection(); break;
    case SECut :
	Cut(); break;
    case SEPaste:
	Clear(); Paste(); break;
    case SECopy:
	Copy(); break;
    default :
	if (!iscntrl(c)) {
	    Clear();
	    AddChar(c);
	}
	break;
    }
}

int StringEdit::Hit (Coord x) {
    return output->GetFont()->Index(buffer, length, x, true);
}

void StringEdit::DeleteChar (int i) {
    if (i < 0 || i >= length) {
	return;
    }
    --length;
    if (prev >= i) {
	--prev;
    }
    if (next > i) {
	--next;
    }
    for (int j = i; j<length; ++j) {
	buffer[j] = buffer[j+1];
    }
    buffer[length] = ' ';
    Refresh(i-1, length, output);
}

void StringEdit::AddChar (char c) {
    if (length >= SEBufferSize-1) {
	return;
    }
    for (int i = length; i>next; --i) {
	buffer[i] = buffer[i-1];
    }
    buffer[next] = c;
    length += 1;
    next += 1;
    prev = next - 1;
    Refresh(prev-1, length, output);
}

void StringEdit::Clear () {
    while (next > prev+1) {
	DeleteChar(prev+1);
    }
}

void StringEdit::Cut () {
    cliplength = 0;
    while (prev < next-1) {
	clip[ cliplength ] = buffer[ prev+1 ];
	DeleteChar(prev+1);
	cliplength += 1;
    }
    clip[ cliplength ] = '\0';
}

void StringEdit::Copy () {
    cliplength = 0;
    for (int i = prev+1; i<next; ++i) {
	clip[ cliplength ] = buffer[ i ];
	cliplength += 1;
    }
    clip[ cliplength ] = '\0';
}

void StringEdit::Paste () {
    for (int i = 0; i<cliplength; ++i) {
	AddChar(clip[i]);
    }
}

void StringEdit::StartSelection (Coord x) {
    next = hitnext = Hit(x);
    prev = hitprev = hitnext-1;
    caret = true;
}

void StringEdit::ExtendSelection (Coord x) {
    int oldprev = prev;
    int oldnext = next;
    int h = Hit(x);
    prev = min(h-1, hitprev);
    next = max(h, hitnext);
    if (next > oldnext) {
	Refresh(oldnext, next-1, highlight);
    } else if (next < oldnext) {
	Refresh(next, oldnext-1, output);
    }
    if (prev < oldprev) {
	Refresh(prev+1, oldprev, highlight);
    } else if (prev > oldprev) {
	Refresh(oldprev+1, prev, output);
    }
}

void StringEdit::SetString (const char* string, boolean select) {
    if (string != nil) {
	old = string;
	Restart();
    }
    if (select) {
	Select();
    }
    Draw();
}

char* StringEdit::GetString () {
    char* result = new char[length+1];
    strncpy(result, buffer, length);
    result[length] = '\0';
    return result;
}

void StringEdit::Handle (Event& e) {
    e.target = this;
    if (e.eventType != DownEvent) {
	e.eventType = OnEvent;
    }
    Listen(edit);
    boolean done = false;
    while (!done) {
	switch (e.eventType) {
	case DownEvent:
            SetCursor(ltextCursor);
	    if (e.target != this) {
		UnRead(e);
		done = true;
		break;
	    }
	    if (selecting) {
		break;
	    }
	    Listen(select);
	    selecting = true;
	    if (!e.shift) {
		HideSelection();
	    }
	    if (DoubleClick(e)) {
		if (e.shift) {
		    Select();
		} else {
		    SelectWord();
		}
	    } else {
		if (e.shift) {
		    ExtendSelection(Coord(e.x-border));
		} else {
		    StartSelection(Coord(e.x-border));
		}
	    }
	    ShowSelection();
	    break;
	case MotionEvent:
            SetCursor(ltextCursor);
	    if (!selecting) {
		break;
	    }
	    HideCaret();
	    if (e.target != this) {
		e.target->GetRelative(e.x, e.y, this);
	    }
	    ExtendSelection(Coord(e.x-border));
	    break;
	case UpEvent:
	    if (!selecting) {
		break;
	    }
	    caret = false;
	    ShowCaret();
	    selecting = false;
	    Listen(edit);
	    break;
	case KeyEvent:
	    SetCursor(noCursor);
	    if (e.len == 0) {
		break;
	    }
	    if (e.keystring[0] == SEAccept || e.keystring[0] == SECancel) {
		UnRead(e);
		done = true;
		break;
	    }
	    HideSelection();
	    DoChar(e.keystring[0]);
	    FixSize();
	    ShowSelection();
	    break;
	case TimerEvent:
	    if (caret) {
		HideCaret();
	    } else {
		ShowCaret();
	    }
	    break;
	default:
	    break;
	}
	if (e.eventType == UpEvent) {
	    lasttime = e.timestamp;
	    lastx = e.x;
	    lasty = e.y;
	}
	if (!done) {
	    Read(e);
	}
    }
    SetCursor(ltextCursor);
    Listen(input);
    HideSelection();
    Unselect();
}

void StringEdit::Refresh (int from, int to, Painter* p) {
    Font* font = output->GetFont();
    int last = font->Index(buffer, length, xmax - 2*border, false);
    int final = min(last, length-1);
    if (from > to || from > last) {
	return;
    } else {
	from = max(from, 0);
	to = min(to, final);
    }
    Coord left = font->Width(buffer, from) + border;
    Coord base = (ymax+1 - shape->vunits)/2;
    if (from == 0) {
	output->ClearRect(canvas, 0, 0, left-1, ymax);
    }
    p->MoveTo(left, base);
    p->Text(canvas, buffer+from, to+1-from);
    if (to == final) {
	Coord x, y;
	p->GetPosition(x, y);
	output->ClearRect(canvas, x, 0, xmax, ymax);
	if (length-1 > last) {
	    output->FillRect(canvas, x+1, y-3, x+2, y+shape->hunits-4);
	    output->FillRect(canvas, x-shape->hunits+3, y-3, x, y-2);
	} else if (length-1 == last) {
	    output->ClearRect(canvas, x-shape->hunits+3, y-3, x, y-2);
	}
    }
}

void StringEdit::DrawCaret (int pos) {
    Font* font = output->GetFont();
    Coord base = (ymax+1 - shape->vunits)/2;
    Coord caretx = font->Width(buffer, pos) + border;
    output->Line(canvas, caretx, base, caretx, base+shape->vunits-1);
}

void StringEdit::FixSize () {
    if (extra == 0) {
	return;
    }
    Font* font = output->GetFont();
    int w = font->Width(buffer, length) + 2 * border;
    if (w > xmax+1) {
	shape->width = int(w + extra * shape->hunits);
	Parent()->Change(this);
    }
}

void StringEdit::Unselect () {
    prev = hitprev = length-1;
    next = hitprev = length;
}

void StringEdit::Select () {
    prev = hitprev = -1;
    next = hitnext = length;
}

void StringEdit::SelectWord () {
    while (next < length && isalnum(buffer[next])) {
	++next;
    }
    while (prev >= 0 && !isalnum(buffer[prev])) {
	--prev;
    }
    while (prev >= 0 && isalnum(buffer[prev])) {
	--prev;
    }
    hitprev = prev;
    hitnext = next;
}

void StringEdit::Flash (int flash) {
    if (flash > 0) {
	edit->CatchTimer(flash/10, (flash%10) * 100000);
    } else {
	edit->Ignore(TimerEvent);
    }
}

void StringEdit::Extra (int ex) {
    extra = ex;
}

void StringEdit::Redraw (Coord, Coord, Coord, Coord) {
    if (canvas == nil) {
	return;
    }
    output->ClearRect(canvas, 0, 0, xmax, ymax);
    if (EmptySelection()) {
	Refresh(0, length-1, output);
	if (caret) {
	    DrawCaret(next);
	}
    } else {
	Refresh(0, prev, output);
	Refresh(prev+1, next-1, highlight);
	Refresh(next, length-1, output);
    }
}

void StringEdit::Reshape (Shape& s) {
    *shape = s;
    if (Parent() != nil) {
	Parent()->Change(this);
    }
}
