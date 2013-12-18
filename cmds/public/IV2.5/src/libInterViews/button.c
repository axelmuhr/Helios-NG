/*
 * Button management.
 */

#include <InterViews/bitmap.h>
#include <InterViews/button.h>
#include <InterViews/font.h>
#include <InterViews/painter.h>
#include <InterViews/pattern.h>
#include <InterViews/sensor.h>
#include <InterViews/shape.h>
#include <InterViews/subject.h>

static const int sep = 3;
static const int pad = 3;

inline int HalfRadioButtonSize (int h) { return round(.4*h); }
inline int HalfCheckBoxSize (int h) { return round(.4*h); }

/*
 * A button state is a value that is settable from one or more buttons.
 * When the state is modified, it updates all its buttons.
 */

ButtonState::ButtonState () {
    value = nil;
}

ButtonState::ButtonState (int v) {
    value = (void*)v;
}

ButtonState::ButtonState (void* v) {
    value = v;
}

void ButtonState::SetValue (int v) {
    Modify((void*)v);
}

void ButtonState::SetValue (void* v) {
    Modify(v);
}

void ButtonState::operator= (ButtonState& s) {
    Modify(s.value);
}

void ButtonState::Modify (void* v) {
    if (value != v) {
	value = v;
	Notify();
    }
}

/*
 * Simple list of buttons.
 */

class ButtonList {
public:
    Button* cur;
    ButtonList* next;

    ButtonList (Button* b) { cur = b; next = nil; }
};

static void Remove (ButtonList*& blist, Button* b) {
    register ButtonList* bl;
    register ButtonList* prev;

    prev = nil;
    for (bl = blist; bl != nil; bl = bl->next) {
	if (bl->cur == b) {
	    if (prev == nil) {
		blist = bl->next;
	    } else {
		prev->next = bl->next;
	    }
	    delete bl;
	    break;
	}
	prev = bl;
    }
}

static void DeleteList (ButtonList* blist) {
    register ButtonList* bl;
    register ButtonList* next;

    for (bl = blist; bl != nil; bl = next) {
	next = bl->next;
	delete bl;
    }
}

/*
 * A button has a ButtonState subject that it modifies when pressed.
 * Also, a button may have associated buttons that are enabled/disabled
 * when it is chosen/unchosen.
 */

Button::Button (ButtonState* s, void* v) {
    Init(s, v);
}

Button::Button (const char* name, ButtonState* s, void* v) {
    SetInstance(name);
    Init(s, v);
}

Button::Button (Painter* out, ButtonState* s, void* v) : (nil, out) {
    Init(s, v);
}

void Button::Init (ButtonState* s, void* v) {
    SetClassName("Button");
    value = v;
    subject = s;
    associates = nil;
    enabled = true;
    hit = false;
    subject->Attach(this);
    Update();
    input = new Sensor(updownEvents);
    input->Catch(OnEvent);
    input->Catch(OffEvent);
}

Button::~Button () {
    if (subject != nil) {
	subject->Detach(this);
    }
    DeleteList(associates);
}

void Button::Attach (Button* b) {
    ButtonList* head;

    head = new ButtonList(b);
    head->next = associates;
    associates = head;
    if (chosen) {
	b->Enable();
    } else {
	b->Disable();
    }
}

void Button::Detach (Button* b) {
    Remove(associates, b);
}

void Button::Enable () {
    if (!enabled) {
	enabled = true;
	if (canvas != nil) {
	    Draw();
	}
    }
}

void Button::Disable () {
    if (enabled) {
	enabled = false;
	if (canvas != nil) {
	    Draw();
	}
    }
}

void Button::Choose () {
    register ButtonList* bl;

    if (!chosen) {
	chosen = true;
	if (enabled) {
	    if (canvas != nil) {
		Refresh();
	    }
	    for (bl = associates; bl != nil; bl = bl->next) {
		bl->cur->Enable();
	    }
	}
    }
}

void Button::UnChoose () {
    register ButtonList* bl;

    if (chosen) {
	chosen = false;
	if (enabled) {
	    if (canvas != nil) {
		Refresh();
	    }
	    for (bl = associates; bl != nil; bl = bl->next) {
		bl->cur->Disable();
	    }
	}
    }
}

void Button::SetDimensions (int width, int height) {
    shape->width = width;
    shape->height = height;
    shape->Rigid();
}

void Button::Refresh () {
    /* default shouldn't happen */
}

void Button::Handle (register Event& e) {
    if (e.eventType == DownEvent && e.target == this) {
	boolean inside = true;
	do {
	    if (enabled && e.target == this) {
		if (e.eventType == OnEvent) {
		    inside = true;
		} else if (e.eventType == OffEvent) {
		    inside = false;
		}
		if (inside) {
		    if (!hit) {
			hit = true;
			Refresh();
		    }
		} else {
		    if (hit) {
			hit = false;
			Refresh();
		    }
		}
	    }
	    Read(e);
	} while (e.eventType != UpEvent);
	if (hit) {
	    hit = false;
	    Refresh();
	}
	if (enabled && inside) {
	    Press();
	}
    }
}

void Button::Press () {
    if (subject != nil) {
	subject->SetValue(value);
    } else {
	Refresh();
    }
}

void Button::Update () {
    void* v;

    subject->GetValue(v);
    if (!chosen && value == v) {
	Choose();
    } else if (chosen && value != v) {
	UnChoose();
    }
}

TextButton::TextButton (const char* str, ButtonState* s, void* v) : (s, v) {
    Init(str);
}

TextButton::TextButton (
    const char* name, const char* str, ButtonState* s, void* v
) : (name, s, v) {
    Init(str);
}

TextButton::TextButton (
    const char* str, ButtonState* s, void* v, Painter* out
) : (out, s, v) {
    Init(str);
}

void TextButton::Init (const char* str) {
    SetClassName("TextButton");
    text = str;
    background = nil;
}

void TextButton::MakeBackground () {
    delete background;
    background = new Painter(output);
    background->SetColors(output->GetBgColor(), output->GetFgColor());
    if (grayout == nil) {
	grayout = new Painter(background);
	grayout->SetPattern(gray);
	grayout->FillBg(false);
    }
}

void TextButton::MakeShape () {
    if (text != nil) {
	Font* f = output->GetFont();
	shape->width += f->Width(text);
	shape->height += f->Height();
    }
    shape->Rigid();
}

TextButton::~TextButton () {
    delete background;
}

PushButton::PushButton (
    const char* str, ButtonState* s, int v
) : (str, s, (void*)v) {
    Init();
}

PushButton::PushButton (
    const char* str, ButtonState* s, void* v
) : (str, s, v) {
    Init();
}

PushButton::PushButton (
    const char* name, const char* str, ButtonState* s, int v
) : (name, str, s, (void*)v) {
    Init();
}
    
PushButton::PushButton (
    const char* name, const char* str, ButtonState* s, void* v
) : (name, str, s, v) {
    Init();
}

PushButton::PushButton (
    const char* str, ButtonState* s, int v, Painter* out
) : (str, s, (void*)v, out) {
    Init();
}

PushButton::PushButton (
    const char* str, ButtonState* s, void* v, Painter* out
) : (str, s, v, out) {
    Init();
}

void PushButton::Init () {
    SetClassName("PushButton");
}

void PushButton::Reconfig () {
    MakeBackground();
    if (shape->Undefined()) {
	MakeShape();
	shape->width += output->GetFont()->Width("    ");
	shape->height += 2*pad;
    }
}

void PushButton::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->ClearRect(canvas, x1, y1, x2, y2);
    Refresh();
}

void PushButton::Refresh () {
    register int r;
    Coord x[16], y[16];
    Coord tx, ty;

    r = min(10*pixels, min(xmax+1, ymax+1)/6);
    x[0] = 0; y[0] = r;
    x[1] = 0; y[1] = r + r;
    x[2] = 0; y[2] = ymax - r - r;
    x[3] = 0; y[3] = ymax - r;
    x[4] = r; y[4] = ymax;
    x[5] = r + r; y[5] = ymax;
    x[6] = xmax - r - r; y[6] = ymax;
    x[7] = xmax - r; y[7] = ymax;
    x[8] = xmax; y[8] = ymax - r;
    x[9] = xmax; y[9] = ymax - r - r;
    x[10] = xmax; y[10] = r + r;
    x[11] = xmax; y[11] = r;
    x[12] = xmax - r; y[12] = 0;
    x[13] = xmax - r - r; y[13] = 0;
    x[14] = r + r; y[14] = 0;
    x[15] = r; y[15] = 0;
    tx = (xmax - output->GetFont()->Width(text))/2;
    ty = pad;
    if (chosen || hit) {
	output->FillBSpline(canvas, x, y, 16);
	background->Text(canvas, text, tx, ty);
    } else {
	background->FillRect(canvas, 0, 0, xmax, ymax);
	output->ClosedBSpline(canvas, x, y, 16);
	output->Text(canvas, text, tx, ty);
    }
    if (!enabled) {
	grayout->FillRect(canvas, 0, 0, xmax, ymax);
    }
}

static const int radio_width = 11;
static const int radio_height = 11;
static unsigned char radio_mask[] = {
   0x70, 0x00, 0xfc, 0x01, 0xfe, 0x03, 0xfe, 0x03, 0xff, 0x07, 0xff, 0x07,
   0xff, 0x07, 0xfe, 0x03, 0xfe, 0x03, 0xfc, 0x01, 0x70, 0x00};
static unsigned char radio_plain[] = {
   0x70, 0x00, 0x8c, 0x01, 0x02, 0x02, 0x02, 0x02, 0x01, 0x04, 0x01, 0x04,
   0x01, 0x04, 0x02, 0x02, 0x02, 0x02, 0x8c, 0x01, 0x70, 0x00};
static unsigned char radio_hit[] = {
   0x70, 0x00, 0xfc, 0x01, 0x8e, 0x03, 0x06, 0x03, 0x03, 0x06, 0x03, 0x06,
   0x03, 0x06, 0x06, 0x03, 0x8e, 0x03, 0xfc, 0x01, 0x70, 0x00};
static unsigned char radio_chosen[] = {
   0x70, 0x00, 0x8c, 0x01, 0x02, 0x02, 0x72, 0x02, 0xf9, 0x04, 0xf9, 0x04,
   0xf9, 0x04, 0x72, 0x02, 0x02, 0x02, 0x8c, 0x01, 0x70, 0x00};
static unsigned char radio_both[] = {
   0x70, 0x00, 0xfc, 0x01, 0x8e, 0x03, 0x76, 0x03, 0xfb, 0x06, 0xfb, 0x06,
   0xfb, 0x06, 0x76, 0x03, 0x8e, 0x03, 0xfc, 0x01, 0x70, 0x00};

static Bitmap* radioMask;
static Bitmap* radioPlain;
static Bitmap* radioHit;
static Bitmap* radioChosen;
static Bitmap* radioBoth;

RadioButton::RadioButton (
    const char* str, ButtonState* s, int v
) : (str, s, (void*)v) {
    Init();
}

RadioButton::RadioButton (
    const char* str, ButtonState* s, void* v
) : (str, s, v) {
    Init();
}

RadioButton::RadioButton (
    const char* name, const char* str, ButtonState* s, int v
) : (name, str, s, (void*)v) {
    Init();
}

RadioButton::RadioButton (
    const char* name, const char* str, ButtonState* s, void* v
) : (name, str, s, v) {
    Init();
}

RadioButton::RadioButton (
    const char* str, ButtonState* s, int v, Painter* out
) : (str, s, (void*)v, out) {
    Init();
}

RadioButton::RadioButton (
    const char* str, ButtonState* s, void* v, Painter* out
) : (str, s, v, out) {
    Init();
}

void RadioButton::Init () {
    SetClassName("RadioButton");
}

void RadioButton::Reconfig () {
    MakeBackground();
    if (shape->Undefined()) {
	MakeShape();
	shape->width += shape->height + sep;
    }
    if (radioMask == nil) {
        radioMask = new Bitmap(radio_mask, radio_width, radio_height);
        radioPlain = new Bitmap(radio_plain, radio_width, radio_height);
        radioHit = new Bitmap(radio_hit, radio_width, radio_height);
        radioChosen = new Bitmap(radio_chosen, radio_width, radio_height);
        radioBoth = new Bitmap(radio_both, radio_width, radio_height);
    }
}

void RadioButton::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    int h = output->GetFont()->Height();
    int r = radio_width;
    output->ClearRect(canvas, x1, y1, x2, y2);
    Coord tx = r + sep;
    Coord ty = (ymax + 1 - h) / 2;
    output->Text(canvas, text, tx, ty);
    Refresh();
}

void RadioButton::Refresh () {
    Coord x = 0;
    Coord y = (ymax+1 - radio_height)/2;
    if (!hit && !chosen) {
        output->Stencil(canvas, x, y, radioPlain, radioMask);
    } else if (hit && !chosen) {
        output->Stencil(canvas, x, y, radioHit, radioMask);
    } else if (!hit && chosen) {
        output->Stencil(canvas, x, y, radioChosen, radioMask);
    } else if (hit && chosen) {
        output->Stencil(canvas, x, y, radioBoth, radioMask);
    }
    if (!enabled) {
	grayout->FillRect(canvas, 0, 0, xmax, ymax);
    }
}

CheckBox::CheckBox (
    const char* str, ButtonState* s, int on, int off
) : (str, s, (void*)on) {
    Init((void*)off);
}

CheckBox::CheckBox (
    const char* str, ButtonState* s, void* on, void* off
) : (str, s, on) {
    Init(off);
}

CheckBox::CheckBox (
    const char* name, const char* str, ButtonState* s, int on, int off
) : (name, str, s, (void*)on) {
    Init((void*)off);
}

CheckBox::CheckBox (
    const char* name, const char* str, ButtonState* s, void* on, void* off
) : (name, str, s, on) {
    Init(off);
}

CheckBox::CheckBox (
    const char* str, ButtonState* s, int on, int off, Painter* out
) : (str, s, (void*)on, out) {
    Init((void*)off);
}

CheckBox::CheckBox (
    const char* str, ButtonState* s, void* on, void* off, Painter* out
) : (str, s, on, out) {
    Init(off);
}

void CheckBox::Init (void* v) {
    SetClassName("CheckBox");
    offvalue = v;
}

void CheckBox::Reconfig () {
    MakeBackground();
    if (shape->Undefined()) {
	MakeShape();
	shape->width += shape->height + sep;
    }
    Update();
}

void CheckBox::Press () {
    if (chosen) {
	subject->GetValue(value);
	subject->SetValue(offvalue);
    } else {
	subject->SetValue(value);
    }
}

void CheckBox::Update () {
    void* v;

    subject->GetValue(v);
    if (v != offvalue) {
	Choose();
	value = v;
    } else {
	UnChoose();
    }
}

void CheckBox::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    int h = output->GetFont()->Height();
    int t = HalfCheckBoxSize(h);
    output->ClearRect(canvas, x1, y1, x2, y2);
    Coord tx = 2*t + sep;
    Coord ty = (ymax + 1 - h) / 2;
    output->Text(canvas, text, tx, ty);
    Refresh();
}

void CheckBox::Refresh () {
    int h = output->GetFont()->Height();
    int t = HalfCheckBoxSize(h);
    Coord cx = t;
    Coord cy = (ymax + 1)/2;
    Coord left = cx - t;
    Coord right = cx + t;
    Coord bottom = cy - t;
    Coord top = cy + t;
    output->Rect(canvas, left, bottom, right, top);
    background->FillRect(canvas, left+1, bottom+1, right-1, top-1);
    if (hit) {
	output->Rect(canvas, left+1, bottom+1, right-1, top-1);
    }
    if (chosen) {
	output->Line(canvas, left, bottom, right, top);
	output->Line(canvas, left, top, right, bottom);
    }
    if (!enabled) {
	grayout->FillRect(canvas, 0, 0, xmax, ymax);
    }
}
