/*
 * TextViewer - basic text buffer
 */

#ifndef textviewer_h
#define textviewer_h

#include <InterViews/interactor.h>
#include <InterViews/paint.h>
#include <InterViews/painter.h>

enum CaretStyle { None, Bar, Block, Outline, Underline };

struct StyleSet {
    char flags;

    StyleSet() { Plain(); }
    StyleSet(char c) { flags = c; }
    operator char() { return flags; }

    void Plain() { flags = 0; }
    void Add(TextStyle s) { flags |= (1<<s); }
    void Remove(TextStyle s) { flags &= ~(1<<s); }
    boolean Includes(TextStyle s) { return flags& (1<<s); }
};

class TPainter : public Painter {
public:
    TPainter() { }
    TPainter(Painter* p) : (p) { }
    void StyledText(Canvas*, const char*, int, StyleSet);
    void Reverse() { SetColors(GetBgColor(), GetFgColor()); }
};

struct TextLine {
    TextLine* above;			// links
    TextLine* below;

    char* text;
    char* attr;
    int size;				// space allocated
    int length;				// characters in line
    boolean touched;

    TextLine();
    ~TextLine();

    void Size(int);			// grow if needed

    void Replace(int index, const char*, int len, char attr);
    void Insert(int index, const char*, int len, char attr);
    void Delete(int index, int len);
    void Blank(int index, int len);
    void EndLine(int);

    TextLine* Split(int index);
    void Merge(TextLine*);
};

class TextViewer : public Interactor {
public:
    TextViewer(Painter* out = stdpaint, int cols = 80, int rows = 24);
    ~TextViewer();

    virtual void Draw();
    virtual void Redraw(Coord, Coord, Coord, Coord);
    virtual void Handle(Event&);
    virtual void Resize();
    virtual void Adjust(Perspective&);
    virtual void Reshape(Shape&);

    CaretStyle caretstyle;		/* type of caret */
    StyleSet style;			/* current text style */

    boolean overwrite;			/* replace or move old text? */
    boolean buffer;			/* don't update immediately? */
    boolean viewcaret;			/* keep caret in view? */

    void NoCaret();
    void Caret();			/* at dot */
    void Caret(Coord row, Coord col);
    void View();			/* dot */
    void View(Coord row, Coord col);
    void GoTo(Coord row, Coord col);
    void GetPos(Coord& row, Coord& col);/* of dot */

    void Margin(int);
    void Indent(int count);

    void Insert(int rows, int cols);	/* insert or delete before dot */

    void String(const char*, int);
    void String(const char*);
    void NewLine();
    void Rubout(int count);
    void Tab(int spacing);
    void Spaces(int count);

    void EndLine();				// after dot
    void EndText();				// after dot

    void Flush();
protected:
    TPainter* painter;
    TPainter* highlight;

    TextLine* top;
    TextLine* bottom;

    TextLine* prev;
    Coord prow;

    TextLine* dot;
    Coord row, col;

    TextLine* caret;
    Coord crow, ccol;

    int margin;

    Coord YPix(Coord y);
    Coord YChar(Coord y);
    Coord XPix(TextLine*, Coord);
    Coord XChar(TextLine*, Coord);
    void ToPix(Coord& x, Coord& y);
    void ToChar(Coord& x, Coord& y);

    TextLine* FindLine(Coord y);
    void Position(TextLine*&, Coord& row, Coord& col);

    void AddLine(TextLine* before, TextLine* newLine);
    void AddLines(TextLine* before, int count);
    void RemoveLine(TextLine* before);
    void RemoveLines(TextLine* before, int count);

    void DrawLine(TextLine*, Coord baseline, Coord first, Coord last);
    void FlushLine(TextLine*, Coord baseline);

    void HideCaret();
    void ShowCaret();
    void BringToView(Coord row, Coord col);
};

#endif
