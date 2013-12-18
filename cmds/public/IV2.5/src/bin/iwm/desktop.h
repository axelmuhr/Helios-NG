#ifndef desktop_h
#define desktop_h

#include <InterViews/worldview.h>
#include <InterViews/frame.h>
#include "lock.h"
#include "logo.h"

typedef enum {
    FOCUS = 0, LOWER = 1, RAISE = 2, REDRAW = 3, REPAIR = 4, ICONIFY = 5,
    MOVE = 6, RESIZE = 7, SCREENLOCK = 8, EXIT = 9, NULL_OP = 10
} OperationCode;

static const int Operations = 11;

class Icon : public Interactor {
    char* s;
    int padding;
    Coord xpos, ypos;
public:
    Icon(const char*, int pad = 4);
    ~Icon();

    boolean IsOld(const char*);
    void Resize();
private:
    void Reconfig();
    void Redraw(Coord, Coord, Coord, Coord);
};

class IconList {
public:
    RemoteInteractor ri;
    Icon* icon;	
    IconList* next;
};

class DesktopDispatcher;

class Desktop : public WorldView {
public:
    Desktop(World*);
    void Run();

    void DoFocus(Event&);
    void DoLower(Event&);
    void DoRaise(Event&);
    void DoRedraw(Event&);
    void DoRepair(Event&);
    void DoIconify(Event&);
    void DoMove(Event&);
    void DoResize(Event&);
    void DoLock(Event&);
    void DoExit(Event&);
    void InsertRemote(void*);

    Logo* GetLogo() { return logo; }
private:
    Sensor* tracking;
    DesktopDispatcher* dispatch;
    IconList* icons;
    boolean running, lock_server, ignorecaps;
    boolean snap_resize, constrain_resize;
    Bitmap* logo_bitmap;
    ScreenLock* lock;
    Logo* logo;
    Coord logosize, logo_x, logo_y;

    void AddIcon(RemoteInteractor, Icon*);
    void Config();
    boolean IconIsOld(RemoteInteractor, const char*);
    Icon* FindIcon(RemoteInteractor);
    boolean GrabEm();
    void RemoveIcon(RemoteInteractor);
    void SyncInput();
    void UnGrabEm();
};

#endif
