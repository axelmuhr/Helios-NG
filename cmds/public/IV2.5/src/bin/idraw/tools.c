// $Header: tools.c,v 1.6 88/09/24 15:09:17 interran Exp $
// implements class Tools.

#include "editor.h"
#include "keystrokes.h"
#include "mapkey.h"
#include "tools.h"
#include <InterViews/box.h>
#include <InterViews/event.h>
#include <InterViews/painter.h>
#include <InterViews/shape.h>

// An IdrawTool enters itself into the MapKey so Idraw can send
// a KeyEvent to the right IdrawTool.

class IdrawTool : public PanelItem {
public:
    IdrawTool(Panel*, const char*, char, Editor*, MapKey*);
protected:
    Editor* editor;		// handles drawing and editing operations
};

// IdrawTool stores the editor pointer and enters itself and its
// associated character into the MapKey.

IdrawTool::IdrawTool (Panel* p, const char* n, char c, Editor* e,
MapKey* mapkey) : (p, n, mapkey->ToStr(c), c) {
    editor = e;
    mapkey->Enter(this, c);
}

// A SelectTool selects a set of Selections.

class SelectTool : public IdrawTool {
public:
    SelectTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "Select", SELECTCHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleSelect(e);
    }
};

// A MoveTool moves a set of Selections.

class MoveTool : public IdrawTool {
public:
    MoveTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "Move", MOVECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleMove(e);
    }
};

// A ScaleTool scales a set of Selections.

class ScaleTool : public IdrawTool {
public:
    ScaleTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "Scale", SCALECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleScale(e);
    }
};

// A StretchTool stretches a set of Selections.

class StretchTool : public IdrawTool {
public:
    StretchTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "Stretch", STRETCHCHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleStretch(e);
    }
};

// A RotateTool rotates a set of Selections.

class RotateTool : public IdrawTool {
public:
    RotateTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "Rotate", ROTATECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleRotate(e);
    }
};

// A ReshapeTool reshapes a Selection.

class ReshapeTool : public IdrawTool {
public:
    ReshapeTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "Reshape", RESHAPECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleReshape(e);
    }
};

// A MagnifyTool magnifies a part of the drawing.

class MagnifyTool : public IdrawTool {
public:
    MagnifyTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "Magnify", MAGNIFYCHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleMagnify(e);
    }
};

// A TextTool draws some text.

class TextTool : public IdrawTool {
public:
    TextTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "Text", TEXTCHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleText(e);
    }
};

// A LineTool draws a line.

class LineTool : public IdrawTool {
public:
    LineTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "", LINECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleLine(e);
    }
protected:
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	IdrawTool::Redraw(l, b, r, t);
	Coord x0 = offx + side * 1/5;
	Coord y0 = offy + side * 4/5;
	Coord x1 = offx + side * 4/5;
	Coord y1 = offy + side * 1/5;
	output->Line(canvas, x0, y0, x1, y1);
    }
};

// A MultiLineTool draws a set of connected lines.

class MultiLineTool : public IdrawTool {
public:
    MultiLineTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "", MULTILINECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleMultiLine(e);
    }
protected:
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	IdrawTool::Redraw(l, b, r, t);
	const int N = 4;
	Coord x[N];
	Coord y[N];
	x[0] = offx + side * 1/5;
	y[0] = offy + side * 4/5;
	x[1] = offx + side * 1/2;
	y[1] = offy + side * 4/5 - side * 1/10;
	x[2] = offx + side * 1/2;
	y[2] = offy + side * 1/5 + side * 1/10;
	x[3] = offx + side * 4/5;
	y[3] = offy + side * 1/5;
	output->MultiLine(canvas, x, y, N);
    }
};

// A BSplineTool draws an open B-spline.

class BSplineTool : public IdrawTool {
public:
    BSplineTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "", BSPLINECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleBSpline(e);
    }
protected:
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	IdrawTool::Redraw(l, b, r, t);
	const int N = 4;
	Coord x[N];
	Coord y[N];
	x[0] = offx + side * 1/5;
	y[0] = offy + side * 4/5;
	x[1] = offx + side * 1/2;
	y[1] = offy + side * 4/5;
	x[2] = offx + side * 1/2;
	y[2] = offy + side * 1/5;
	x[3] = offx + side * 4/5;
	y[3] = offy + side * 1/5;
	output->BSpline(canvas, x, y, N);
    }
};

// An EllipseTool draws an ellipse.

class EllipseTool : public IdrawTool {
public:
    EllipseTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "", ELLIPSECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleEllipse(e);
    }
protected:
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	IdrawTool::Redraw(l, b, r, t);
	Coord x0 = offx + side * 1/2;
	Coord y0 = offy + side * 1/2;
	Coord xradius = side * 1/3 + side * 1/16;
	Coord yradius = side * 1/3 - side * 1/16;
	output->Ellipse(canvas, x0, y0, xradius, yradius);
    }
};

// A RectTool draws a rectangle.

class RectTool : public IdrawTool {
public:
    RectTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "", RECTCHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleRect(e);
    }
protected:
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	IdrawTool::Redraw(l, b, r, t);
	Coord x0 = offx + side * 1/5;
	Coord y0 = offy + side * 1/5;
	Coord x1 = offx + side * 4/5;
	Coord y1 = offy + side * 4/5;
	output->Rect(canvas, x0, y0, x1, y1);
    }
};

// A PolygonTool draws a polygon.

class PolygonTool : public IdrawTool {
public:
    PolygonTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "", POLYGONCHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandlePolygon(e);
    }
protected:
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	IdrawTool::Redraw(l, b, r, t);
	const int N = 5;
	Coord x[N];
	Coord y[N];
	x[0] = offx + side * 1/5 + side * 1/8;
	y[0] = offy + side * 1/5;
	x[1] = offx + side * 1/5;
	y[1] = offy + side * 1/2;
	x[2] = offx + side * 1/2;
	y[2] = offy + side * 4/5;
	x[3] = offx + side * 4/5;
	y[3] = offy + side * 1/2 + side * 1/8;
	x[4] = offx + side * 4/5 - side * 1/32;
	y[4] = offy + side * 1/5 + side * 1/8;
	output->Polygon(canvas, x, y, N);
    }
};

// A ClosedBSplineTool draws a closed B-spline.

class ClosedBSplineTool : public IdrawTool {
public:
    ClosedBSplineTool (Panel* p, Editor* e, MapKey* mk)
    : (p,  "", CLOSEDBSPLINECHAR, e, mk) {}
    void Perform (Event& e) {
	editor->HandleClosedBSpline(e);
    }
protected:
    void Redraw (Coord l, Coord b, Coord r, Coord t) {
	IdrawTool::Redraw(l, b, r, t);
	const int N = 6;
	Coord x[N];
	Coord y[N];
	x[0] = offx + side * 1/10;
	y[0] = offy + side * 1/2;
	x[1] = offx + side * 3/5;
	y[1] = offy + side * 1/5;
	x[2] = offx + side * 4/5;
	y[2] = offy + side * 2/5;
	x[3] = offx + side * 1/2;
	y[3] = offy + side * 1/2;
	x[4] = offx + side * 4/5;
	y[4] = offy + side * 3/5;
	x[5] = offx + side * 3/5;
	y[5] = offy + side * 4/5;
	output->ClosedBSpline(canvas, x, y, N);
    }
};

// Tools creates its tools.

Tools::Tools (Editor* e, MapKey* mk) {
    Init(e, mk);
}

// Handle tells one of the tools to perform its function if a
// DownEvent occurs.

void Tools::Handle (Event& e) {
    switch (e.eventType) {
    case DownEvent:
	switch (e.button) {
	case LEFTMOUSE:
	    PerformCurrentFunction(e);
	    break;
	case MIDDLEMOUSE:
	    PerformTemporaryFunction(e, MOVECHAR);
	    break;
	case RIGHTMOUSE:
	    PerformTemporaryFunction(e, SELECTCHAR);
	    break;
	default:
	    break;
	}
    default:
	break;
    }
}

// Init creates the tools, lays them together, and inserts them.

void Tools::Init (Editor* e, MapKey* mk) {
    PanelItem* first = new SelectTool(this, e, mk);

    VBox* tools = new VBox;
    tools->Insert(first);
    tools->Insert(new MoveTool(this, e, mk));
    tools->Insert(new ScaleTool(this, e, mk));
    tools->Insert(new StretchTool(this, e, mk));
    tools->Insert(new RotateTool(this, e, mk));
    tools->Insert(new ReshapeTool(this, e, mk));
    tools->Insert(new MagnifyTool(this, e, mk));
    tools->Insert(new TextTool(this, e, mk));
    tools->Insert(new LineTool(this, e, mk));
    tools->Insert(new MultiLineTool(this, e, mk));
    tools->Insert(new BSplineTool(this, e, mk));
    tools->Insert(new EllipseTool(this, e, mk));
    tools->Insert(new RectTool(this, e, mk));
    tools->Insert(new PolygonTool(this, e, mk));
    tools->Insert(new ClosedBSplineTool(this, e, mk));

    Insert(tools);
    Highlight(first);
}

// Reconfig makes Tools's shape unstretchable but shrinkable.

void Tools::Reconfig () {
    Panel::Reconfig();
    shape->Rigid(0, 0, vfil, 0);
}
