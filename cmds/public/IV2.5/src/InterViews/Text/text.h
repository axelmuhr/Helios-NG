/*
 * Text - structured text primitives and compositions
 */

#ifndef text_h
#define text_h

#include <InterViews/defs.h>

class Layout;
class Composition;

typedef const void* TextData;

static const int BREAK = 1000000;
static TextData SELF = (void*)-1;

class Text {
    friend class Composition;
    friend class Layout;
public:
    Text(TextData context = SELF);
    ~Text();

    virtual Text* Copy () { return nil; }
    virtual void Draw (Layout*) { }
    virtual boolean Overflows(Layout*);
    virtual void Reshape () { }

    TextData GetContext();
    void SetContext(TextData);
    Composition* Parent();
    Text* Next();
    int GetSize();
protected:
    TextData context;
    Composition* parent;
    Text* next;
    int size;

    virtual void Locate(Coord &x1, Coord &y1, Coord &x2, Coord &y2);
    virtual void Undraw();
};

inline TextData Text::GetContext () { return context; }
inline void Text::SetContext (TextData c) { context = c; }
inline Composition* Text::Parent () { return parent; }
inline Text* Text::Next () { return next; }
inline int Text::GetSize () { return size; }

class Whitespace : public Text {
protected:
    virtual void Draw(Layout*);
public:
    Whitespace(int size, TextData context = SELF);
    virtual Text* Copy();
};

class LineBreak : public Whitespace {
protected:
public:
    LineBreak(TextData context = SELF);
};

class Caret : public Text {
    virtual boolean Overflows(Layout*);
    virtual void Draw(Layout*);
public:
    Caret();
};

class Word : public Text {
friend class EditWord;
protected:
    const char* word;
    virtual void Draw(Layout*);
public:
    Word(const char* word, int size, TextData context = SELF);
    virtual Text* Copy();
};

class EditWord : public Text {
protected:
    Word* word;
    boolean drawn;
    Coord firstx, firsty;
    int pos;
    char* buffer;
    int bufferlength;

    virtual void Draw(Layout*);
    virtual void Undraw();
    virtual void Locate(Coord&, Coord&, Coord&, Coord&);
public:
    EditWord(char* buffer, int length);
    ~EditWord();

    void Edit(Word*);
    void Done();
    void Insert(const char*, int length);
    void Delete(int length = 1);
    void GoTo(int pos);
    void Forward(int count = 1);
    void Backward(int count = 1);
};

class Composition : public Text {
friend class Text;
protected:
    Text* first, * last;
    int count;

    boolean drawn;
    Coord firstx, firsty;
    Coord nextx, nexty;

    void Duplicate(Composition*);
    void Size();
    virtual void DrawContents(Layout*);
    virtual void Draw(Layout*);
    virtual void Locate(Coord &x1, Coord &y1, Coord &x2, Coord &y2);
    virtual void Undraw();
public:
    Composition(TextData context = SELF);
    ~Composition();
    virtual Text* Copy();
    virtual void Reshape();

    Text* First() { return first; }
    Text* Succ(Text*);
    Text* Pred(Text*);
    boolean Follows(Text*, Text*);

    virtual void Append(Text*);
    virtual void Prepend(Text*);
    virtual void Build(Text* t) { Append(t); }
    virtual void InsertAfter(Text* old, Text*);
    virtual void InsertBefore(Text* old, Text*);
    virtual void Remove(Text*);
    virtual void Replace(Text* old, Text*);
};

class Paragraph : public Composition {
protected:
    Text* prefix;
    int left;
    int right;

    virtual boolean Overflows(Layout*);
    virtual void DrawContents(Layout*);
    virtual void Locate(Coord &x1, Coord &y1, Coord &x2, Coord &y2);
public:
    Paragraph(Text* prefix, int left, int right, TextData context = SELF);
    virtual Text* Copy();
    virtual void Reshape();
};

class Sentence : public Composition {
protected:
    Text* separator;

    virtual boolean Overflows(Layout*);
    virtual void DrawContents(Layout*);
public:
    Sentence(Text* sep, TextData context = SELF);
    virtual void Reshape();
    virtual Text* Copy();
};

class Display : public Composition {
protected:
    int indent;
    boolean broken;

    virtual boolean Overflows(Layout*);
    virtual void DrawContents(Layout*);
    virtual void Drawer(Layout*, Text*, Text*);
    virtual void Locate(Coord &x1, Coord &y1, Coord &x2, Coord &y2);
public:
    Display(int indent, TextData context = SELF);
    virtual Text* Copy();
    virtual void Reshape();
};

class TextList : public Composition {
protected:
    Text* prefix, * postfix;
    Text* separator;
    Text* keeper;

    virtual void DrawContents(Layout*);
public:
    TextList(
	Text* sep, Text* keeper,
	Text* prefix, Text* postfix,
	TextData context = SELF   
    );
    virtual Text* Copy();
    virtual void Reshape();
};

#endif
