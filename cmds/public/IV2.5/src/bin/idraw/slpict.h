// $Header: slpict.h,v 1.8 89/05/28 21:18:43 interran Exp $
// declares class PictSelection.

#ifndef slpict_h
#define slpict_h

#include "selection.h"

// Declare imported types.

class IFont;
class IFontList;

// A PictSelection contains other Selections.

class PictSelection : public Selection {
public:

    PictSelection(Graphic* = nil);
    PictSelection(FILE*, State*);

    Graphic* Copy();
    boolean HasChildren();
    void Propagate();

    boolean Valid();
    boolean WritePicture(FILE*, boolean);

    Selection* GetCurrent();
    Selection* First();
    Selection* Last();
    Selection* Next();
    Selection* Prev();

    Selection* FirstSelectionContaining(PointObj&);
    Selection* LastSelectionContaining(PointObj&);
    int SelectionsContaining(PointObj&, Selection**&);

    Selection* FirstSelectionIntersecting(BoxObj&);
    Selection* LastSelectionIntersecting(BoxObj&);
    int SelectionsIntersecting(BoxObj&, Selection**&);

    Selection* FirstSelectionWithin(BoxObj&);
    Selection* LastSelectionWithin(BoxObj&);
    int SelectionsWithin(BoxObj&, Selection**&);

protected:

    PictSelection(istream&, State*);
    void ReadChildren(istream&, State*);

    void WritePicture(ostream&, boolean);
    void WriteComments(ostream&);
    void WritePrologue(ostream&);
    void WriteDrawing(ostream&);
    void WriteData(ostream&);
    void WriteTrailer(ostream&);
    void ScaleToPostscriptCoords();
    void ScaleToScreenCoords();
    void CollectFonts(IFontList*);
    void Merge(IFont*, IFontList*);

    boolean valid;		// true if creation of PictSelection succeeded

};

// Define inline access functions to get members' values.

inline boolean PictSelection::Valid () {
    return valid;
}

// Cast these functions to return Selections instead of Graphics.

inline Selection* PictSelection::GetCurrent() {
    return (Selection*) Picture::GetCurrent();
}

inline Selection* PictSelection::First() {
    return (Selection*) Picture::First();
}

inline Selection* PictSelection::Last() {
    return (Selection*) Picture::Last();
}

inline Selection* PictSelection::Next() {
    return (Selection*) Picture::Next();
}

inline Selection* PictSelection::Prev() {
    return (Selection*) Picture::Prev();
}

inline Selection* PictSelection::FirstSelectionContaining(PointObj& p) {
    return (Selection*) Picture::FirstGraphicContaining(p);
}

inline Selection* PictSelection::LastSelectionContaining(PointObj& p) {
    return (Selection*) Picture::LastGraphicContaining(p);
}

inline int PictSelection::SelectionsContaining(PointObj& p, Selection**& ss) {
    Graphic** gg = nil;
    int num = Picture::GraphicsContaining(p, gg);
    ss = (Selection**) gg;
    return num;
}

inline Selection* PictSelection::FirstSelectionIntersecting(BoxObj& b) {
    return (Selection*) Picture::FirstGraphicIntersecting(b);
}

inline Selection* PictSelection::LastSelectionIntersecting(BoxObj& b) {
    return (Selection*) Picture::LastGraphicIntersecting(b);
}

inline int PictSelection::SelectionsIntersecting(BoxObj& b, Selection**& ss) {
    Graphic** gg = nil;
    int num = Picture::GraphicsIntersecting(b, gg);
    ss = (Selection**) gg;
    return num;
}

inline Selection* PictSelection::FirstSelectionWithin(BoxObj& b) {
    return (Selection*) Picture::FirstGraphicWithin(b);
}

inline Selection* PictSelection::LastSelectionWithin(BoxObj& b) {
    return (Selection*) Picture::LastGraphicWithin(b);
}

inline int PictSelection::SelectionsWithin(BoxObj& b, Selection**& ss) {
    Graphic** gg = nil;
    int num = Picture::GraphicsWithin(b, gg);
    ss = (Selection**) gg;
    return num;
}

#endif
