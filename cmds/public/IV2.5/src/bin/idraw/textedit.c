// $Header: textedit.c,v 1.9 89/05/18 15:58:30 vlis Exp $
// implements class TextEdit.

#include "ipaint.h"
#include "istring.h"
#include "liststring.h"
#include "textedit.h"
#include <InterViews/event.h>
#include <InterViews/graphic.h>
#include <InterViews/painter.h>
#include <InterViews/shape.h>
#include <InterViews/world.h>
#include <stdio.h>

// Define constants.

static const Coord ENDCOL	= 10000; // gets us to actual end of most lines
static const int SPACING	= 4;	 // gives amount by which to tab

// Create bindings between keystrokes and editing commands.

static const char BEGLINE	= '\001'; // ^A
static const char DELLINE	= '\013'; // ^K
static const char DELNEXTCHAR	= '\004'; // ^D
static const char DELPREVCHAR1	= '\010'; // ^H
static const char DELPREVCHAR2	= '\177'; // DEL
static const char DELREST	= '\032'; // ^Z
static const char ENDLINE	= '\005'; // ^E
static const char INDENT	= '\027'; // ^W
static const char NEWLINE	= '\015'; // RET
static const char NEXTCHAR	= '\006'; // ^F
static const char NEXTLINE	= '\016'; // ^N
static const char OPENLINE	= '\017'; // ^O
static const char PREVCHAR	= '\002'; // ^B
static const char PREVLINE	= '\020'; // ^P
static const char TAB		= '\011'; // TAB
static const char TOGGLEOW	= '\024'; // ^T
static const char UNINDENT	= '\021'; // ^Q

// TextEdit sets itself up to begin editing a buffer initialized with
// some text if there's any (stringlist == nil is okay).

TextEdit::TextEdit (StringList* stringlist, Graphic* out)
: (GraphicToPainter(out), NumCols(stringlist), NumLines(stringlist)) {
    caretstyle = Bar;
    overwrite = false;
    buffer = false;

    SetText(stringlist);
}

// Handle inserts the TextEdit into the world, reads and processes
// subsequent events until a DownEvent occurs, puts the DownEvent back
// on the queue, and removes the TextEdit from the world.  Thus the
// user can terminate typing text by clicking the mouse in the
// TextEdit or selecting another tool or pulling down a menu.

void TextEdit::Handle (Event& e) {
    Interactor* underlying = e.target;
    Coord l, b, r, t;
    l = e.x;
    t = e.y;
    underlying->Align(BottomRight, 0, 0, r, b);
    underlying->GetRelative(l, b);
    underlying->GetRelative(r, t);
    shape->width = min(shape->width, max(r - l, shape->hunits));
    shape->height = min(shape->height, max(t - b, shape->vunits));
    shape->height = (shape->height / shape->vunits) * shape->vunits;

    World* world;
    e.GetAbsolute(world, e.x, e.y);
    world->InsertPopup(this, l, t, TopLeft);
    Read(e);
    while (e.eventType != DownEvent) {
	if (e.eventType == KeyEvent && e.len > 0) {
	    HandleChar(e.keystring[0]);
	}
	Read(e);
    }
    if (e.target == this) {
	GetRelative(e.x, e.y, underlying);
	e.target = underlying;
    }
    UnRead(e);
    world->Remove(this);
}

// GetText returns the buffer contents in a newly allocated list.

StringList* TextEdit::GetText () {
    StringList* stringlist = new StringList;
    for (TextLine* l = top; l != nil && l->text != nil; l = l->below) {
	stringlist->Append(new StringNode(l->text, l->length));
    }
    return stringlist;
}

// SetText sets the initial contents of the text buffer.

void TextEdit::SetText (StringList* stringlist) {
    buffer = true;

    GoTo(1, 1);
    EndText();
    if (stringlist != nil) {
	if (stringlist->Size() > 0) {
	    String(stringlist->First()->GetString());
	}
	for (stringlist->Next(); !stringlist->AtEnd(); stringlist->Next()) {
	    const char* string = stringlist->GetCur()->GetString();
	    NewLine();
	    String(string);
	}
    }
    GoTo(1, 1);
    GetPos(line, column);
    Caret();

    buffer = false;
    Flush();
}

// GraphicToPainter returns a Painter which will draw text like the
// Graphic would have.

Painter* TextEdit::GraphicToPainter (Graphic* gs) {
    Painter* p = new Painter(stdpaint);

    PColor* fg = gs->GetFgColor();
    if (fg != nil) {
	p->SetColors(*fg, p->GetBgColor());
    }
    PColor* bg = gs->GetBgColor();
    if (bg != nil) {
	p->SetColors(p->GetFgColor(), *bg);
    }

    PFont* f = gs->GetFont();
    if (f != nil) {
	p->SetFont(*f);
    }

    return p;
}

// NumCols returns the number of columns the text spans plus a default
// additional number of columns.

int TextEdit::NumCols (StringList* stringlist) {
    int cols = 0;
    if (stringlist != nil) {
	for (stringlist->First(); !stringlist->AtEnd(); stringlist->Next()) {
	    const char* string = stringlist->GetCur()->GetString();
	    cols = max(cols, strlen(string));
	}
    }
    return cols + 80;
}

// NumLines returns the number of lines in the text plus a default
// additional number of rows.

int TextEdit::NumLines (StringList* stringlist) {
    int rows = 0;
    if (stringlist != nil) {
	rows = stringlist->Size();
    }
    return rows + 5;
}

// HandleChar carries out the editing command bound to the keystroke
// if any.  Otherwise, it echoes the keystroke if it's a printing
// character.

void TextEdit::HandleChar (char c) {
    switch (c) {
    case BEGLINE:
	begline();
	break;
    case DELLINE:
	delline();
	break;
    case DELNEXTCHAR:
	delnextchar();
	break;
    case DELPREVCHAR1:
    case DELPREVCHAR2:
	delprevchar();
	break;
    case DELREST:
	delrest();
	break;
    case ENDLINE:
	endline();
	break;
    case INDENT:
	indent();
	break;
    case NEWLINE:
	newline();
	break;
    case NEXTCHAR:
	nextchar();
	break;
    case NEXTLINE:
	nextline();
	break;
    case OPENLINE:
	openline();
	break;
    case PREVCHAR:
	prevchar();
	break;
    case PREVLINE:
	prevline();
	break;
    case TAB:
	tab();
	break;
    case TOGGLEOW:
	toggleow();
	break;
    case UNINDENT:
	unindent();
	break;
    default:
	if (c >= ' ' && c <= '~') {
	    String(&c, 1);
	}
	break;
    }
    GetPos(line, column);
    Caret();
}

// begline moves dot to the beginning of the current line.

void TextEdit::begline () {
    GoTo(line, 1);
}

// delline deletes any text following dot on the current line.  If dot
// was at the beginning or end of the line, delline joins the next
// line to the current line.

void TextEdit::delline () {
    GoTo(line, ENDCOL);
    GetPos(tline, tcolumn);
    GoTo(line, column);
    if (tcolumn > column) {
	Rubout(tcolumn - column);
    }
    if (column == 1 || column == tcolumn) {
	GoTo(line + 1, 1);
	GetPos(tline, tcolumn);
	if (tline > line) {
	    Insert(-1, column - 1);
	}
	GoTo(line, column);
    }
}

// delnextchar deletes the character after dot.  If dot was at the end
// of the line, delnextchar joins the next line to the end of the
// current line.

void TextEdit::delnextchar () {
    GoTo(line, ENDCOL);
    GetPos(tline, tcolumn);
    if (column == tcolumn) {
	GoTo(line + 1, 1);
	GetPos(tline, tcolumn);
	if (tline > line) {
	    Insert(-1, column - 1);
	}
	GoTo(line, column);
    } else {
	GoTo(line, column);
	Rubout(1);
    }
}

// delprevchar deletes the character before dot.  If dot was at the
// beginning of the line, delprevchar joins the current line to the
// previous line.

void TextEdit::delprevchar () {
    if (column > 1) {
	GoTo(line, column - 1);
	Rubout(1);
    } else {
	prevchar();
	GetPos(line, column);
	Caret();
	delnextchar();
    }
}

// delrest deletes any text following dot in the rest of the buffer.

void TextEdit::delrest () {
    EndText();
}

// endline moves dot to the end of the current line.

void TextEdit::endline () {
    GoTo(line, ENDCOL);
}

// indent adds count SPACING blanks to the current margin.

void TextEdit::indent () {
    Indent(SPACING);
}

// newline moves dot to the beginning of the next line, opening a new
// blank line and copying any text following dot if overwrite is false
// or truncating the current line to dot if overwrite is true.

void TextEdit::newline () {
    NewLine();
}

// nextchar moves dot after the next character.  If dot is at the end
// of the current line, nextchar moves dot to the beginning of the
// next line.

void TextEdit::nextchar () {
    GoTo(line, column + 1);
    GetPos(tline, tcolumn);
    if (tcolumn == column) {
	GoTo(line + 1, 1);
    }
}

// nextline moves dot to the same column in the next line or the end
// of the next line, whichever is smaller.

void TextEdit::nextline () {
    GoTo(line + 1, column);
}

// openline inserts a new blank line following the current line.

void TextEdit::openline () {
    Insert(1, -column);
    GoTo(line, column);
}

// prevchar moves dot before the previous character.  If dot is at the
// beginning of the current line, prevchar moves dot to the end of the
// previous line.

void TextEdit::prevchar () {
    if (column > 1) {
	GoTo(line, column - 1); 
    } else {
	GoTo(line - 1, ENDCOL);
    }
}

// prevline moves dot to the same column in the previous line or the
// end of the previous line, whichever is smaller.

void TextEdit::prevline () {
    GoTo(line - 1, column);
}

// tab inserts space characters up to the next multiple of SPACING.

void TextEdit::tab () {
    Tab(SPACING);
}

// toggleow toggles overwrite which determines whether new characters
// will replace existing text or existing text will move to the right
// to make room.

void TextEdit::toggleow () {
    overwrite = !overwrite;
}

// unindent removes count SPACING blanks from the current margin.

void TextEdit::unindent () {
    Indent(-SPACING);
}
