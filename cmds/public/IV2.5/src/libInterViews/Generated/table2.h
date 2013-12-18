/*
 * Object association table.
 */

#ifndef Table2_h
#define Table2_h

#include <InterViews/defs.h>

class Table2Entry;

class Table2 {
public:
    Table2(int);
    ~Table2();
    void Insert(void*, void*, void*);
    boolean Find(void*&, void*, void*);
    void Remove(void*, void*);
private:
    int size;
    Table2Entry** first;
    Table2Entry** last;

    Table2Entry* Probe(void*, void*);
    Table2Entry** ProbeAddr(void*, void*);
};

#endif
