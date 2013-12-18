// Interface to class Inclink

#ifndef _inclink_h
#define _inclink_h

#include <InterViews/space.h>

class Connection;

class Inclink : public ObjectSpace {
public:
    Inclink(char*);
    void Message(Connection*, ObjectTag, int, void*, int);
    void SendMsg(int, char*, int);

private:
    void SendAck();

    Connection* client;
};

extern Inclink* server;

#endif
