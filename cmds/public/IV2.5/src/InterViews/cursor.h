/*
 * An input cursor is defined by two 16x16 bitmaps, one that
 * specifies which pixels are to be drawn and one that specifies
 * which pixels are in foreground color and which in background color.
 * If a device does not support a mask the background pixels are not drawn.
 */

#ifndef cursor_h
#define cursor_h

static const int cursorHeight = 16;
static const int cursorWidth = 16;

typedef int CursorPattern[cursorHeight];

class Color;
class Bitmap;
class Font;

class Cursor {
public:
    Cursor(
	short xoff, short yoff,			/* hot spot */
	CursorPattern pat, CursorPattern mask,	/* pattern, mask */
	Color* fg, Color* bg
    );
    Cursor(Bitmap* pat, Bitmap* mask, Color* fg, Color* bg);
    Cursor(Font*, int pat, int mask, Color* fg, Color* bg);
    Cursor(int, Color* fg, Color* bg);
    ~Cursor();

    void* Id();
private:
    void* id;
    short x, y;
    int* pat, * mask;
    Color* foreground, * background;
};

/*
 * Predefined cursors.
 */

extern Cursor* defaultCursor;
extern Cursor* arrow;
extern Cursor* crosshairs;
extern Cursor* ltextCursor;
extern Cursor* rtextCursor;
extern Cursor* hourglass;
extern Cursor* upperleft;
extern Cursor* upperright;
extern Cursor* lowerleft;
extern Cursor* lowerright;
extern Cursor* noCursor;

#endif
