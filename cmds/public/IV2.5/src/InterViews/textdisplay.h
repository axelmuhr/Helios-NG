/*
 * TextDisplay - basic text displaying
 */

#ifndef textdisplay_h
#define textdisplay_h

#include <InterViews/defs.h>

enum CaretStyleOptions { NoCaret, DefaultCaret, BarCaret };

class Rubberband;
class Painter;
class Canvas;

class TextDisplay {
public:
    TextDisplay();
    ~TextDisplay();

    void Scroll(int line, Coord x, Coord y);

    void Resize(Coord xmin, Coord ymin, Coord xmax, Coord ymax, int h, int t);
    void Redraw(Painter*, Canvas*, Coord l, Coord b, Coord r, Coord t);

    void InsertLinesAfter(int line, int count = 1);
    void InsertLinesBefore(int line, int count = 1);
    void DeleteLinesAfter(int line, int count = 1);
    void DeleteLinesBefore(int line, int count = 1);

    void InsertText(int line, int index, const char*, int count);
    void DeleteText(int line, int index, int count);
    void ReplaceText(int line, const char*, int count);

    void Style(int line1, int index1, int line2, int index2, int style);
    void AddStyle(int line1, int index1, int line2, int index2, int style);
    void RemoveStyle(int line1, int index1, int line2, int index2, int style);

    void Caret(int line, int index);
    void CaretStyle(int);

    int LineNumber(Coord y);
    int LineIndex(int line, Coord x);

    Coord Base(int line);
    Coord Top(int line);
    Coord Left(int line, int index);
    Coord Right(int line, int index);
private:
friend class TextLine;

    void Size(int, int);
    TextLine* Line(int, boolean);
    int Index(int);
    void HideCaret();
    void ShowCaret();

    Painter* painter;
    Canvas* canvas;
    Coord xmin, xmax;
    Coord ymin, ymax;
    Coord x0, y0;
    int lineheight;
    int tabwidth;
    TextLine** lines;
    int maxlines;
    int firstline;
    int lastline;
    int topline;
    int bottomline;
    Rubberband* caret;
    int caretline;
    int caretindex;
    int caretstyle;
};

#endif
