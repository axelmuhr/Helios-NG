/*
 * Per-client information for space manager.
 */

#ifndef client_h
#define client_h

#include <InterViews/stub.h>

class Catalog;
class Manager;
class ObjectTable;

class Port : public ObjectStub {
private:
    friend class Client;
    friend class ClientStub;

    char* name;
    int pid;
    Client* owner;
    class WaitingList* waiting;

    Port(int);
    void Sleep(ClientStub*);
    void Wakeup();
    void Delete();
};

class Client {
public:
    Client(Manager*, Connection*, Catalog*, ObjectTable*);
    ~Client();

    void UsePath(const char*);
    void Register(const char*, int);
    void UnRegister(const char*);
    Port* Find(const char*);
    Port* Wait(ClientStub*, const char*);
    void Remove(const char*);
private:
    Manager* mgr;
    Connection* owner;
    Catalog* dictionary;
    ObjectTable* table;
    char path[1024];
    int pathlen;

    const char* FullName(const char*);
};

#endif
