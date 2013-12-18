/*
 * Input events.
 */

#ifndef event_h
#define event_h

#include <InterViews/defs.h>

enum EventType {
    MotionEvent,	/* mouse moved */
    DownEvent,		/* button pressed */
    UpEvent,		/* button released */
    KeyEvent,		/* key pressed, intepreted as ascii */
    EnterEvent,		/* mouse enters canvas */
    LeaveEvent,		/* mouse leaves canvas */
    ChannelEvent,	/* input pending on channel */
    TimerEvent,		/* time out on read */
    FocusInEvent,	/* focus for keyboard events */
    FocusOutEvent 	/* lose keyboard focus */
};

/* obsolete */
static const int OnEvent = EnterEvent;
static const int OffEvent = LeaveEvent;

/* mouse buttons */
static const int LEFTMOUSE = 0;
static const int MIDDLEMOUSE = 1;
static const int RIGHTMOUSE = 2;

/*
 * EventFlag should be boolean, but C++ doesn't allow enum bitfields.
 */
typedef unsigned int EventFlag;

class World;

class Event {
public:
    class Interactor* target;
    int timestamp;
    EventType eventType;
    Coord x, y;			/* mouse position relative to target */
    EventFlag control : 1;	/* true if down */
    EventFlag meta : 1;
    EventFlag shift : 1;
    EventFlag shiftlock : 1;
    EventFlag leftmouse : 1;
    EventFlag middlemouse : 1;
    EventFlag rightmouse : 1;
    unsigned char button;	/* button pressed or released, if any */
    short len;			/* length of ASCII string */
    char* keystring;		/* ASCII interpretation of event, if any */
    int channel;		/* set of channels ready */

    Event& operator=(Event&);
    void GetAbsolute(Coord&, Coord&);
    void GetAbsolute(World*&, Coord&, Coord&);
private:
    friend class Sensor;
    friend class Interactor;

    World* w;			/* world in which event occurred */
    Coord wx, wy;		/* mouse position relative to world */
    char keydata[sizeof(int)];	/* keystring points here for simple mappings */

    void GetButtonInfo(void*);
};

#endif
