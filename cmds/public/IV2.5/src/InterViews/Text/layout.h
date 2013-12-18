/*
 * Layout - a chain of TextBlocks
 */

#ifndef layout_h
#define layout_h

#include <InterViews/Text/text.h>

class PaintMap;
class Context;
class TextBlock;
class TextPainter;
class Region;

class Sensor;
class Event;

class Layout {
    TextBlock* top, * tail;
    TextBlock* block;

    PaintMap* painters;
    Context** contexts;

    Coord x, y;
    Coord left, right;
    Coord width;

    Region* damage, * extradamage, * target;

    TextData hit;
    TextData lastcontext;

    boolean intarget;

    inline boolean PreTarget(Coord x, Coord y);
    inline boolean PostTarget(Coord x, Coord y);
    inline boolean InTarget();

    void Flush();
    void GoTo(Coord x, Coord y);
    void Overfull();
    void LastLine();
protected:
    Text* contents;
    TextPainter* output;
    Sensor* input;
public:
    Layout(Text*, Sensor*, TextPainter*);
    ~Layout();

    void Chain(TextBlock*);
    void Unchain(TextBlock*);
    void Rechain();

    Coord X() { return x; }
    Coord Y() { return y; }
    Coord Remaining() { return width-right-left-x; }
    boolean Drawing() { return target != nil; }

    void Margins(int left, int right);
    boolean SkipTo(Coord x, Coord y);

    void Damage(Coord x1, Coord y1, Coord x2, Coord y2);
    void ExtraDamage(Coord x, Coord y);
    void EndDamage(Coord x, Coord y);

    void String(const char* s, int length);
    void NewLine();
    void Space(int count);
    void Backspace(int count);
    void Caret();

    void Enter(TextData context);
    void Leave(TextData context);

    void Listen(Sensor*);
    void Paint(TextData context, TextPainter* paint);
    void Unpaint(TextData context);

    void Repair();
    void Touch(Text*);
    void Show(Text*);

    boolean Hit(Event&, TextData& context, TextData& after);
};

#endif
