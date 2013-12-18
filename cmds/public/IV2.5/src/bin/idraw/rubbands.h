// $Header: rubbands.h,v 1.7 89/03/19 12:18:14 interran Exp $
// defines classes IStretchingRect, RubberMultiLine, and RubberPolygon.

#ifndef rubbands_h
#define rubbands_h

#include <InterViews/rubcurve.h>
#include <InterViews/rubline.h>
#include <InterViews/rubrect.h>

// An IStretchingRect uses its first few Track calls to decide which
// side it will let the user drag.

class IStretchingRect : public StretchingRect {
public:

    IStretchingRect(Painter*, Canvas*, Coord, Coord, Coord, Coord,
		    Coord = 0, Coord = 0);

    void Track(Coord, Coord);
    void DefineSide(Coord, Coord);
    Alignment CurrentSide();

protected:

    boolean firsttime;		// stores true until after first call of Track
    boolean undefinedside;	// stores true until side has been determined
    Coord cx, cy;		// stores original center of rectangle
    Coord origx, origy;		// stores point passed by first call of Track

};

// A RubberMultiLine lets the user drag one of its vertices.

class RubberMultiLine : public RubberVertex {
public:
    RubberMultiLine(
        Painter*, Canvas*, Coord px[], Coord py[], int n, int pt,
	Coord offx = 0, Coord offy = 0
    );
    void Draw();
};

// A RubberPolygon lets the user drag one of its vertices.

class RubberPolygon : public RubberVertex {
public:
    RubberPolygon(
        Painter*, Canvas*, Coord px[], Coord py[], int n, int pt,
	Coord offx = 0, Coord offy = 0
    );
    void Draw();
};

#endif
