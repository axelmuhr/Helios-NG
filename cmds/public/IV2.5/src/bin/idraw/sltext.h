// $Header: sltext.h,v 1.7 89/04/17 00:31:48 linton Exp $
// declares class TextSelection.

#ifndef sltext_h
#define sltext_h

#include "selection.h"

// Declare imported types.

class StringList;

// A TextSelection draws one to several lines of text.

static const ClassId TEXTSELECTION = 2100;

class TextSelection : public Selection {
public:

    TextSelection(StringList*, Graphic* = nil);
    TextSelection(istream&, State*);
    ~TextSelection();

    Graphic* Copy();
    boolean IsA(ClassId);

    StringList* GetOriginal();
    boolean ShapedBy(Coord, Coord, float);

protected:

    void draw(Canvas*, Graphic*);
    void drawClipped(Canvas*, Coord, Coord, Coord, Coord, Graphic*);
    void ReadjustSpacing(PFont*);

    const char* ReadString(istream&);
    void WriteData(ostream&);
    const char* Filter(const char*);

    int height;			// remembers previous interline spacing
    StringList* stringlist;	// contains the TextSelection's text

};

#endif
