/*
 * Object space manager.
 */

#ifndef spaceman_h
#define spaceman_h

#include <InterViews/defs.h>
#include <InterViews/deputy.h>

class Connection;

class SpaceManager : public Deputy {
public:
    SpaceManager();
    ~SpaceManager();

    void UsePath(const char*);
    void Register(const char*, Connection* local, Connection* remote);
    void UnRegister(const char*);
    Connection* Find(const char*, boolean wait = false);
private:
    char filename[128];
    char hostname[128];
};

extern const char* spaceman_dir;
extern const char* spaceman_mgr;
extern const char* spaceman_name;

static const int spaceman_UsePath = 1;
static const int spaceman_Register = 2;
static const int spaceman_UnRegister = 3;
static const int spaceman_Find = 4;
static const int spaceman_WaitFor = 5;

#endif
