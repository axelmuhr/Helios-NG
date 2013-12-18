#ifndef dispatch_h
#define dispatch_h

#include <InterViews/sensor.h>
#include <InterViews/painter.h>
#include <InterViews/menu.h>
#include <InterViews/cursor.h>
#include <stdio.h>

typedef void (*DesktopFunction)(void*, Event&);		/* Desktop::* */

typedef enum {
    CTRL_MASK = 010, SHIFT_MASK = 020, META_MASK = 040, SHIFT_LOCK_MASK = 0100,
    LOGO_MASK = 0200
} ActivatorMask;

typedef enum {
    NO_ACTIVATOR = 00, LEFT = 01, MIDDLE = 02, RIGHT = 04
} Activator;

static const int Activators = 
    CTRL_MASK|SHIFT_MASK|META_MASK|SHIFT_LOCK_MASK|LOGO_MASK|RIGHT;

extern class World* world;

#ifdef X10

#define LockMask ShiftLockMask
#define Mod1Mask MetaMask

#else

#define LeftMask 0400
#define MiddleMask 0200
#define RightMask 0100

#endif

typedef struct ButtonList {
    unsigned mask;
    Cursor* cursor;
    ButtonList* next;
};

class DesktopMenu;

typedef struct Operation {
    DesktopFunction f;			/* Desktop function to perform */
    char* uop;				/* user (shell) command to perform */
    DesktopMenu* menu;			/* menu to pop up */
};

class Desktop;

class DesktopDispatcher {
    Desktop* desk;			/* controlling Desktop */
    FILE* configfile;			/* the configuration file */
    Operation* ops[Activators];		/* left, middle, right operations */
    DesktopMenu* menus;
    ButtonList* buttons;
    boolean forked;

    void Perform(Operation*, Event&);
    void InitMasks(int&, unsigned&, Activator&, boolean&);
    void Config();
    void AddButton(unsigned mask, Cursor* cursor) {
	ButtonList* tmp;
	if (mask) {
	    tmp = new ButtonList;
	    tmp->next = buttons; buttons = tmp;
	    buttons->mask = mask;
	    buttons->cursor = cursor;
	}
    }
public:
    DesktopDispatcher(Desktop*);
    ~DesktopDispatcher();

    void Perform(Event&);
    Operation* GetOperation(Event&);
    boolean NoFunction(Event&);

    ButtonList* Buttons() { return buttons; }		/* ick! */
};

#endif
