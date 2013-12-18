/*
 * A terminal is a character array window.
 *
 * Terminals support a set of operations suitable for an emulator or editor.
 */

#ifndef terminal_h
#define terminal_h

#include <InterViews/scene.h>

enum TerminalEvent {
    TLEFTMOUSE, TMIDDLEMOUSE, TRIGHTMOUSE, TMOTION, TCHAR
};

struct TerminalInput {
    TerminalEvent eventType;
    union {
	struct {
	    short row;
	    short col;
	    boolean down;
	} mouse;
	struct {
	    short row;
	    short col;
	} motion;
	char tchar;
    };
};

class Frame;
class Banner;
class OldTextBuffer;

class Terminal : public MonoScene {
public:
    Terminal(boolean autohide = false);
    Terminal(int rows, int cols, boolean autohide = false);
    void BatchedScrolling(boolean);
    void GetSize(int& rows, int& cols);

    void ClearLines(int where, int count);
    void ClearScreen();
    void EraseBOL();
    void EraseEOL();
    void EraseLine();
    void EraseEOS();

    void CursorOn();
    void CursorOff();
    void CursorDown(int);
    void CursorLeft(int);
    void CursorRight(int);
    void CursorUp(int);
    void Goto(int row, int col);

    void InsertCharacters(int);
    void DeleteCharacters(int);
    void InsertLines(int);
    void DeleteLines(int);

    void ScrollDown(int where, int count);
    void ScrollUp(int where, int count);

    void RingBell(int vol);

    void AddChar(char);
    void AddString(const char *str);
    void AddString(const char *str, int len);

    void CarriageReturn();
    void LineFeed();
    void Tab();
    void BackSpace();

    void Underline(boolean);
    void Inverse(boolean);
    void Bold(boolean);
    void Blink(boolean);

    void QMotion();
    void UnQMotion();
    void QMouseButton(int);
    void UnQMouseButton(int);

    void Read(TerminalInput&);
    boolean QTest();
    boolean Interesting(Event&, TerminalInput&);
    void SetBannerString(const char* string);
    void Raise();
    void SetCursor(Cursor*);
protected:
    Banner* heading;
    OldTextBuffer* text;
    boolean placed;
    boolean changed;
    boolean showcursor;
    boolean autohide;	    /* hide cursor on key event, show on motion */
    boolean autohidden;	    /* cursor is currently hidden */
    int currow, curcol;

    void Init();
    void BeginOp();
    void EndOp();
};

#endif
