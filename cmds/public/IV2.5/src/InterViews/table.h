/*
 * Object association table.
 */

#ifndef Table_h
#define Table_h

#include <InterViews/defs.h>

class TableEntry;

class Table {
public:
    Table(int);
    ~Table();
    void Insert(void*, void*);
    boolean Find(void*&, void*);
    void Remove(void*);
private:
    int size;
    TableEntry** first;
    TableEntry** last;

    TableEntry* Probe(void*);
    TableEntry** ProbeAddr(void*);
};

#endif
