/*
 * Stub for per-client objects.
 */

#ifndef client_stub_h
#define client_stub_h

#include <InterViews/stub.h>

class Client;
class Connection;

class ClientStub : public ObjectStub {
public:
    ClientStub(Client*, Connection*);
    ~ClientStub();

    virtual void Message(Connection*, ObjectTag, int op, void*, int);
    void Reply(class Port*);
private:
    Client* client;
    Connection* connection;
};

#endif
