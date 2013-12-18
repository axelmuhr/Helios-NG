/*
 * Digit class for digital clock
 */

#include "dclock.h"
#include "digit.h"
#include "segment.h"

Digit::Digit (float Xoff, float Yoff) {
    Xorg = Xoff;
    Yorg = Yoff;
    for (Seg s = SegA; s <= SegG; s++) {
	Segs[s] = new Segment(s, Xoff, Yoff);
    }
}

Digit::~Digit () {
    for (Seg s = SegA; s <= SegG; s++) {
	delete Segs[s];
    }
}

boolean Digit::Set (int value) {
    if (value > 9 || value < 0) {
	// out of range, use blank
	value = 10;
    }
    boolean done = true;
    for (Seg s = SegA; s <= SegG; s++) {
	done &= SegCode[value][s] ? Segs[s]->On() : Segs[s]->Off();
    };
    return done;
}

void Digit::Reconfig (Painter* output) {
    // configure any segment to initialize all segment patterns
    Segs[SegA]->Reconfig(output);
}

void Digit::Resize (Canvas* canvas, int height) {
    for (Seg s = SegA; s <= SegG; s++) {
	Segs[s]->Resize(canvas, height);
    }
}

void Digit::Redraw () {
    for (Seg s = SegA; s <= SegG; s++) {
	Segs[s]->Redraw();
    }
}
