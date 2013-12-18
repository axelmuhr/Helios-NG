/*
 * Object association table.
 */

#ifndef FontTable_h
#define FontTable_h

#include <InterViews/table.h>

class FontTableEntry;

class FontTable : public Table {
public:
    FontTable(int);
    void Insert(class StringId*, class FontRep*);
    boolean Find(class FontRep*&, class StringId*);
    void Remove(class StringId*);
};

inline FontTable::FontTable (int n) : (n) {}

inline void FontTable::Insert (class StringId* k, class FontRep* v) {
    Table::Insert((void*)k, (void*)v);
}

inline boolean FontTable::Find (class FontRep*& v, class StringId* k) {
    void* vv;

    boolean b = Table::Find(vv, (void*)k);
    if (b) {
	v = (class FontRep*)vv;
    }
    return b;
}

inline void FontTable::Remove (class StringId* k) {
    Table::Remove((void*)k);
}

#endif
