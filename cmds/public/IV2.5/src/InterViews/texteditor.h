/*
 * TextEditor - basic interactive editor for mulit-line text
 */

#ifndef texteditor_h
#define texteditor_h

#include <InterViews/interactor.h>

class TextDisplay;
class TextBuffer;

class TextEditor : public Interactor {
public:
    TextEditor(int rows, int cols, int tabsize, int highlight);
    TextEditor(const char* name, int r, int c, int t, int h);
    virtual ~TextEditor();

    void Edit(TextBuffer*);

    int Dot();
    int Mark();

    void InsertText(const char*, int);
    void DeleteText(int);
    void DeleteSelection();

    void BackwardCharacter(int = 1),    ForwardCharacter(int = 1);
    void BackwardLine(int = 1),         ForwardLine(int = 1);
    void BackwardWord(int = 1),         ForwardWord(int = 1);
    void BackwardPage(int = 1),         ForwardPage(int = 1);

    void BeginningOfLine(),             EndOfLine();
    void BeginningOfWord(),             EndOfWord();
    void BeginningOfSelection(),        EndOfSelection();
    void BeginningOfText(),             EndOfText();

    void ScrollToSelection();
    void ScrollToView(Coord x, Coord y);
    void ScrollBy(Coord dx, Coord dy);

    void Select(int dot);
    void SelectMore(int mark);
    void SelectAll();
    void Select(int dot, int mark);

    int Locate(Coord x, Coord y);
protected:
    virtual void Adjust(Perspective&);
    virtual void Reconfig();
    virtual void Redraw(Coord, Coord, Coord, Coord);
    virtual void Resize();
private:
    void ScrollTo(int x, int y);
    void Init(int r, int c, int t, int h);

    int dot, mark;
    TextBuffer* text;
    TextDisplay* display;
    int tabsize;
    int lineheight;
    int highlight;
    int shaperows;
    int shapecolumns;
};

inline int TextEditor::Dot () {
    return dot;
}
inline int TextEditor::Mark () {
    return mark;
}

#endif
