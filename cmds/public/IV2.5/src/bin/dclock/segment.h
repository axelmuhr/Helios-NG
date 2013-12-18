/*
 * Segment class for digital clock
 */

#ifndef segment_h
#define segment_h

enum Seg { SegA, SegB, SegC, SegD, SegE, SegF, SegG };

class Segment {
public:
    Segment(Seg, float, float);
    ~Segment();
    void Reconfig(Painter*);
    void Resize(Canvas*, int);
    void Redraw();
    boolean On();
    boolean Off();
private:
    SegPoints p;			// polygon points for this segment
    Seg whichSeg;			// for indexing into segment data array
    float Xorg;				// fractional amount from canvas origin
    float Yorg;
    Canvas *canvas;			// canvas of DFace
    int fullFade;
    int fade;				// 0 = off; fullFade = on
    static Painter* fadePainter[17];	// painters for fading

    Pattern* MakePattern(int seed);

    void FadeUp () {
	if (fade < fullFade) {
	    fade += FadeStep;
	    Draw();
	}
    }
    void FadeDown() {
	if (fade > 0) {
	    fade -= FadeStep;
	    Draw();
	}
    }
    boolean IsOn () {
	return fade == fullFade;
    }
    boolean IsOff() {
	return fade == 0;
    }
    void Draw () {
	fadePainter[fade]->FillPolygon(canvas, p.x, p.y, p.count);
    }
};

inline void Segment::Redraw () {
    if (!IsOff()) {
	Draw();
    }
}

inline boolean Segment::On () {
    if (!IsOn()) {
	FadeUp();
    }
    return IsOn();
}

inline boolean Segment::Off () {
    if (!IsOff()) {
	FadeDown();
    }
    return IsOff();
}

#endif
