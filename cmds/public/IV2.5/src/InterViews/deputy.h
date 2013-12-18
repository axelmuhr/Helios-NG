/*
 * Deputy objects perform operations on remote objects.
 */

#ifndef deputy_h
#define deputy_h

#include <InterViews/tag.h>

class ChiefDeputy;
class Connection;

class Deputy {
public:
    Deputy(ChiefDeputy*);
    ~Deputy();

    void Sync();
    ObjectTag Tag () { return (unsigned)this; }
    Connection* GetServer();
protected:
    ChiefDeputy* chief;

    Deputy();
};

#endif
