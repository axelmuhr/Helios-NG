/*
 * TextPainter - styled text
 */

#ifndef textpainter_h
#define textpainter_h

#include <InterViews/painter.h>

class Canvas;

class TextPainter : public Painter {
    boolean bold;
    boolean underlined;
    boolean boxed;
    boolean transparent;
    boolean greyed;
    boolean inverted;
public:
    TextPainter( Painter * p );

    void Text( Canvas *, const char *, int );

    void Bold() { bold = true; }
    void NotBold() { bold = false; }
    void Underlined() { underlined = true; }
    void NotUnderlined() { underlined = false; }
    void Boxed() { boxed = true; }
    void NotBoxed() { boxed = false; }
    void Greyed() { greyed = true; }
    void NotGreyed() { greyed = false; }
    void Transparent();
    void NotTransparent();
    void Inverted();
    void NotInverted();
};

#endif
