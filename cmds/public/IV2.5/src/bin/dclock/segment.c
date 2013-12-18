/*
 * Segment class for digital clock
 */

#include "dclock.h"

#include "segment.h"

// pattern initialization
const int myPatSeed[17] = {
    0x0000, 0x8000, 0x8020, 0xA020,
    0xA0A0, 0xA4A0, 0xA4A1, 0xA5A1,
    0xA5A5, 0xA5B5, 0xE5B5, 0xF5B5,
    0xF5F5, 0xF5F7, 0xFDF7, 0xFFF7,
    0xFFFF
};

Pattern* Segment::MakePattern (int seed) {
    Pattern* pat;
    int dat[patternHeight];
    unsigned int Row[4];

    for (int i = 0; i <= 3; i++) {
	Row[i] = seed & 0xF;
	Row[i] |= Row[i]<<4;
	Row[i] |= Row[i]<<8;
	Row[i] |= Row[i]<<16;
	seed >>= 4;
    }
    for (i = 0; i <= patternHeight-1; i++) {
	dat[i] = Row[i%4];
    }
    pat = new Pattern(dat);
    return pat;
}

Segment::Segment (Seg s, float Xoff, float Yoff) {
    whichSeg = s;
    Xorg = Xoff;
    Yorg = Yoff;
    p.count = SegData[whichSeg].count;
    fade = 0;		// initially off
    fullFade = 16;

}

void Segment::Reconfig (Painter* output) {
    register int i;

    for (i = 0; i <= fullFade; i++) {
	Painter* p = new Painter(output);
	p->SetPattern(MakePattern(myPatSeed[i]));
	fadePainter[i] = p;
    }
}

Segment::~Segment () {}

void Segment::Resize (Canvas* c, int height) {
    canvas = c;
    int w = canvas->Width();
    int h = height;

    for (int i = 0; i < p.count; i++) {
	p.x[i] = Coord((SegData[whichSeg].x[i]+Xorg) * w);
	p.y[i] = Coord((SegData[whichSeg].y[i]+Yorg) * h);
    }
}
