/*
 * Object space definition.
 */

#ifndef objectspace_h
#define objectspace_h

#include <InterViews/stub.h>

class Catalog;
class Messenger;
class ObjectTable;
class SpaceManager;

class ObjectSpace : public ObjectStub {
public:
    ObjectSpace(const char*);
    ~ObjectSpace();

    void StartServer(Connection* local, Connection* remote);

    void UsePath(const char*);
    Connection* Find(const char*, boolean wait = false);

    void Dispatch();

    void Message(Connection*, ObjectTag, int op, void*, int len);
    void Deliver(Connection*, ObjectTag, int op, void*, int);
    ObjectStub* Map(Connection*, ObjectTag);

    void AddChannel(int, ObjectStub*);
    void RemoveChannel(int);

    void Attach(int);
    void Detach(int);
protected:
    const char* name;
    Catalog* dictionary;
    ObjectTable* table;

    ObjectSpace();
private:
    struct MQueue {
	Messenger* head, * tail;
    };
    struct Stream {
	int channel, mask;
	ObjectStub* object;
	Stream* next;
    };

    Connection* local;
    Connection* remote;
    SpaceManager* manager;
    int channels;
    int maxchannel;
    MQueue active;
    MQueue inactive;
    Stream* streams;

    void Init();
    void Listen(Connection*);
    boolean Ready(int, Connection*);
    void CheckServer(int, Connection*);
    void CheckClients();
    void Add(Messenger*, MQueue&);
    void Remove(Messenger*, MQueue&);
    void CloseDown(Messenger*);
    virtual void AddClient(Connection*);
};

/*
 * Protocol definitions.
 */

static const int objectspace_Find = 1;
static const int objectspace_Clone = 2;
static const int objectspace_Destroy = 3;

struct objectspace_Msg {
    ObjectTag tag;
    char name[1];
    /* rest of name */
};

#endif
