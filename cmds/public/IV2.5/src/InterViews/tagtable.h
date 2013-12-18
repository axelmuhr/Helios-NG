/*
 * An object table provides a mapping from tags to objects.
 */

#ifndef tagtable_h
#define tagtable_h

#include <InterViews/stub.h>

class Connection;

class ObjectTableEntry {
private:
    friend class ObjectTable;

    Connection* client;
    ObjectTag id;
    ObjectStub* ref;
    ObjectTableEntry* chain;

    boolean Match(Connection*, ObjectTag);
};

class ObjectTable {
public:
    ObjectTable(int);
    ~ObjectTable();

    void Add(Connection*, ObjectTag, ObjectStub*);
    ObjectStub* Find(Connection*, ObjectTag);
    void Remove(Connection*, ObjectTag);
    void RemoveAll(Connection*);
private:
    int nelements;
    ObjectTableEntry** first;
    ObjectTableEntry** last;

    unsigned Hash(Connection*, ObjectTag);
    ObjectTableEntry* Start(Connection*, ObjectTag);
    ObjectTableEntry** StartAddr(Connection*, ObjectTag);
};

#endif
