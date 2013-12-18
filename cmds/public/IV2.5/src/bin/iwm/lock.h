#ifndef lock_h
#define lock_h

#include <InterViews/frame.h>
#include <InterViews/world.h>
#include <pwd.h>

extern World* world;

class ScreenLock : public Interactor {
public:
    ScreenLock(Painter*);

    Cursor* LockCursor();
    void InsertInto(Scene* s) {
	s->Insert(lockdialog, world->Width()/2, world->Height()/2, Center);
    }
    void RemoveFrom(Scene* s) { s->Remove(lockdialog); }
    void Activate();
private:
    Frame* lockdialog;
    char username[ 128 ];
    char userpasswd[ 128 ];
    char rootpasswd[ 128 ];
    struct passwd pwentry;
};

#endif
