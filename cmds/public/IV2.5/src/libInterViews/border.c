/*
 * Border implementation.
 */

#include <InterViews/border.h>
#include <InterViews/painter.h>
#include <InterViews/shape.h>

Border::Border (int t) {
    thickness = t;
}

Border::Border (const char* name, int t) {
    SetInstance(name);
    thickness = t;
}

Border::Border (Painter* out, int t) : (nil, out) {
    thickness = t;
}

void Border::Redraw (Coord x1, Coord y1, Coord x2, Coord y2) {
    output->FillRect(canvas, x1, y1, x2, y2);
}

HBorder::HBorder (int t) : (t) {
    Init();
}

HBorder::HBorder (const char* name, int t) : (name, t) {
    Init();
}

HBorder::HBorder (Painter* out, int t) : (out, t) {
    Init();
    Reconfig();
}

void HBorder::Init () {
    SetClassName("HBorder");
}

void HBorder::Reconfig () {
    shape->height = thickness;
    shape->Rigid(hfil, hfil, 0, 0);
}

VBorder::VBorder (int t) : (t) {
    Init();
}

VBorder::VBorder (const char* name, int t) : (name, t) {
    Init();
}

VBorder::VBorder (Painter* out, int t) : (out, t) {
    Init();
    Reconfig();
}

void VBorder::Init () {
    SetClassName("VBorder");
}

void VBorder::Reconfig () {
    shape->width = thickness;
    shape->Rigid(0, 0, vfil, vfil);
}
