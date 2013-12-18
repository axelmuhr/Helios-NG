// $Header: selection.h,v 1.10 89/05/12 15:03:18 calder Exp $
// declares classes Selection and NPtSelection.

#ifndef selection_h
#define selection_h

#include <InterViews/Graphic/picture.h>

// Declare imported types.

class Rubberband;
class RubberVertex;
class State;
class istream;
class ostream;

// A Selection can draw handles around itself and create a reshaped
// copy of itself.

static const int ARROWHEIGHT = 8;      // how long arrows are in points
static const int ARROWWIDTH = 4;       // how wide arrows are in points
static const int BUFSIZE = 256;	       // size of buffer for reading data
static const int HDSIZE = 5;	       // how wide handles are in points
extern const char* startdata;	       // signals place to read valid data

class Selection : public Picture {
public:

    Selection(Graphic* = nil);
    ~Selection();

    Graphic* Copy();
    boolean HasChildren();

    double GetGridSpacing();
    void SetGridSpacing(double);
    void GetPaddedBox(BoxObj&);
    void DrawHandles(Painter*, Canvas*);
    void EraseHandles(Painter*, Canvas*);
    void RedrawHandles(Painter*, Canvas*);
    void RedrawUnclippedHandles(Painter*, Canvas*);
    void ResetHandles();

    virtual boolean ShapedBy(Coord, Coord, float);
    virtual Rubberband* CreateShape(Coord, Coord);
    virtual Selection* GetReshapedCopy();

    virtual void WriteData(ostream&);

protected:

    void Skip(istream&);
    void ReadVersion(istream&);
    void ReadGridSpacing(istream&);
    void ReadGS(istream&, State*);
    void ReadPictGS(istream&, State*);
    void ReadTextGS(istream&, State*);
    void ReadBrush(istream&, State*);
    void ReadFgColor(istream&, State*);
    void ReadBgColor(istream&, State*);
    void ReadFont(istream&, State*);
    void ReadPattern(istream&, State*);
    void ReadTransformer(istream&);
    float CalcGrayLevel(int);

    void WriteVersion(ostream&);
    void WriteGridSpacing(ostream&);
    void WriteGS(ostream&);
    void WritePictGS(ostream&);
    void WriteTextGS(ostream&);
    void WriteBrush(ostream&);
    void WriteFgColor(ostream&);
    void WriteBgColor(ostream&);
    void WriteFont(ostream&);
    void WritePattern(ostream&);
    void WriteTransformer(ostream&);

    virtual void CreateHandles();
    void DeleteHandles();

    Rubberband* handles;	// contains handles outlining Selection

    static char buf[BUFSIZE];	// contains storage for reading data
    static int versionnumber;	// stores version of drawing read from file
    static double gridspacing;	// stores grid spacing drawing is aligned to

};

// Define inline access functions to get and set members' values.

inline double Selection::GetGridSpacing () {
    return gridspacing;
}

inline void Selection::SetGridSpacing (double g) {
    gridspacing = g;
}

// An NPtSelection knows how to create handles, read or write its
// points, and draw arrowheads so many subclasses can reuse the same
// code.

class NPtSelection : public Selection {
public:

    NPtSelection(Graphic*);

    virtual void GetOriginal(Coord*&, Coord*&, int&);
    boolean ShapedBy(Coord, Coord, float);
    Rubberband* CreateShape(Coord, Coord);
    Selection* GetReshapedCopy();

protected:

    void ReadPoints(istream&, const Coord*&, const Coord*&, int&);
    void WriteData(ostream&);

    virtual RubberVertex* CreateRubberVertex(Coord*, Coord*, int, int);
    virtual Selection* CreateReshapedCopy(Coord*, Coord*, int);
    void CreateHandles();
    void TotalTransform(Coord*, Coord*, int);
    void InvTotalTransform(Coord*, Coord*, int);
    int ClosestPoint(Coord*, Coord*, int, Coord, Coord);

    boolean LeftAcont(Coord, Coord, Coord, Coord, PointObj&, Graphic*);
    boolean RightAcont(Coord, Coord, Coord, Coord, PointObj&, Graphic*);
    boolean LeftAints(Coord, Coord, Coord, Coord, BoxObj&, Graphic*);
    boolean RightAints(Coord, Coord, Coord, Coord, BoxObj&, Graphic*);
    void drawLeftA(Coord, Coord, Coord, Coord, Canvas*, Graphic*);
    void drawRightA(Coord, Coord, Coord, Coord, Canvas*, Graphic*);
    float MergeArrowHeadTol(float, Graphic*);
    boolean ArrowHeadcont(Coord, Coord, Coord, Coord, PointObj&, Graphic*);
    boolean ArrowHeadints(Coord, Coord, Coord, Coord, BoxObj&, Graphic*);
    void drawArrowHead(Coord, Coord, Coord, Coord, Canvas*, Graphic*);
    void SetCTM(Coord, Coord, Coord, Coord, Graphic*, boolean);
    void RestoreCTM(Graphic*);
    float Slope(float, float);

    const char* myname;		// tells which NPtSelection subclass this is
    RubberVertex* rubbervertex;	// tells us how the user wants to reshape us

};

#endif
