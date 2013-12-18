/*
 * Object association table.
 */

#ifndef InteractorTable_h
#define InteractorTable_h

#include <InterViews/table.h>

class InteractorTableEntry;

class InteractorTable : public Table {
public:
    InteractorTable(int);
    void Insert(void*, class Interactor*);
    boolean Find(class Interactor*&, void*);
    void Remove(void*);
};

inline InteractorTable::InteractorTable (int n) : (n) {}

inline void InteractorTable::Insert (void* k, class Interactor* v) {
    Table::Insert((void*)k, (void*)v);
}

inline boolean InteractorTable::Find (class Interactor*& v, void* k) {
    void* vv;

    boolean b = Table::Find(vv, (void*)k);
    if (b) {
	v = (class Interactor*)vv;
    }
    return b;
}

inline void InteractorTable::Remove (void* k) {
    Table::Remove((void*)k);
}

#endif
