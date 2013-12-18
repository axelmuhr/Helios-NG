/* 
 * A chief deputy multiplexes communcation with a remote object space
 * across a single connection.
 */

#ifndef chiefdeputy_h
#define chiefdeputy_h

#include <InterViews/defs.h>
#include <InterViews/tag.h>

class Connection;

class ChiefDeputy {
public:
    ChiefDeputy(Connection*);
    ~ChiefDeputy();

    Connection* GetServer() { return server; }
    ObjectTag Tag () { return 0; }
    void Sync();

    void Alloc(void*& dst, ObjectTag, int op, int len);
    int PackString(const char*, void*);
    int PackString(const char*, int, void*);
    void Msg(ObjectTag, int op);
    void StringMsg(ObjectTag, int op, const char*);
    void IntegerMsg(ObjectTag, int op, int val);

    int ReadReply(void* reply, int len);
    void GetReply(void* reply, int len);
    void GetString(const char*& str, int len);
private:
    Connection* server;
    char buffer[1024];
    int cur;
};

#endif
