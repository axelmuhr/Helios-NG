/*
 * Message class implementation.
 */

#include <InterViews/font.h>
#include <InterViews/message.h>
#include <InterViews/painter.h>
#include <InterViews/shape.h>

Message::Message (const char* t, Alignment a) {
    Init(t, a);
}

Message::Message (const char* name, const char* t, Alignment a) {
    SetInstance(name);
    Init(t, a);
}

Message::Message (Painter* p, const char* t, Alignment a) : (nil, p) {
    Init(t, a);
}

void Message::Init (const char* t, Alignment a) {
    SetClassName("Message");
    text = t;
    alignment = a;
}

void Message::Reconfig () {
    Font* f = output->GetFont();
    shape->width = f->Width(text);
    shape->height = f->Height();
    shape->Rigid();
}

void Message::Realign (Alignment a) {
    alignment = a;
    Redraw(0, 0, xmax, ymax);
}

void Message::Redraw (Coord l, Coord b, Coord r, Coord t) {
    Coord x = 0, y = 0;
    output->Clip(canvas, l, b, r, t);
    output->ClearRect(canvas, l, b, r, t);
    Align(alignment, shape->width, shape->height, x, y);
    output->Text(canvas, text, x, y);
    output->NoClip();
}
