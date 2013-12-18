/*
 * The space manager is itself an object space.
 */

#ifndef manager_h
#define manager_h

#include <InterViews/space.h>

class Manager : public ObjectSpace {
public:
    Manager(int argc, char* argv[]);
private:
    friend class Client;
    friend class Port;

    char local[100];
    const char* dir;

    void Usage();
    virtual void Message(Connection*, ObjectTag, int op, void*, int);
};

#endif
