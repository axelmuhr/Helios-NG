/*
 * Object association table.
 */

#ifndef \Table_h
#define \Table_h

#include <InterViews/table.h>

class \TableEntry;

class \Table : public Table {
public:
    \Table(int);
    void Insert(\TableKey, \TableValue);
    boolean Find(\TableValue&, \TableKey);
    void Remove(\TableKey);
};

inline \Table::\Table (int n) : (n) {}

inline void \Table::Insert (\TableKey k, \TableValue v) {
    Table::Insert((void*)k, (void*)v);
}

inline boolean \Table::Find (\TableValue& v, \TableKey k) {
    void* vv;

    boolean b = Table::Find(vv, (void*)k);
    if (b) {
	v = (\TableValue)vv;
    }
    return b;
}

inline void \Table::Remove (\TableKey k) {
    Table::Remove((void*)k);
}

#endif
