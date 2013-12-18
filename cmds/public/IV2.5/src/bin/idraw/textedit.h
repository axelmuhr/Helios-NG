// $Header: textedit.h,v 1.6 88/09/24 15:09:13 interran Exp $
// declares class TextEdit.

#ifndef textedit_h
#define textedit_h

#include <InterViews/Text/textviewer.h>

// Declare imported types.

class Graphic;
class StringList;

// A TextEdit edits an array of lines of text.

class TextEdit : public TextViewer {
public:

    TextEdit(StringList*, Graphic*);
    void Handle(Event&);

    StringList* GetText();
    void SetText(StringList*);

protected:

    Painter* GraphicToPainter(Graphic*);
    int NumCols(StringList*);
    int NumLines(StringList*);
    void HandleChar(char);
    void begline();
    void delline();
    void delnextchar();
    void delprevchar();
    void delrest();
    void endline();
    void indent();
    void newline();
    void nextchar();
    void nextline();
    void openline();
    void prevchar();
    void prevline();
    void tab();
    void toggleow();
    void unindent();

    Coord line;			// stores current row position
    Coord column;		// stores current column position
    Coord tline;		// stores separate row position when needed
    Coord tcolumn;		// stores separate column position when needed

};

#endif
