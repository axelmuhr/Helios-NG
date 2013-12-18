/*
 * Object association table.
 */

#ifndef \Table_h
#define \Table_h

#include "table2.h"

class \TableEntry;

class \Table : public Table2 {
public:
    \Table(int);
    void Insert(\TableKey1, \TableKey2, \TableValue);
    boolean Find(\TableValue&, \TableKey1, \TableKey2);
    void Remove(\TableKey1, \TableKey2);
};

inline \Table::\Table (int n) : (n) {}

inline void \Table::Insert (\TableKey1 k1, \TableKey2 k2, \TableValue v) {
    Table2::Insert((void*)k1, (void*)k2, (void*)v);
}

inline boolean \Table::Find (\TableValue& v, \TableKey1 k1, \TableKey2 k2) {
    void* vv;

    boolean b = Table2::Find(vv, (void*)k1, (void*)k2);
    if (b) {
	v = (\TableValue)vv;
    }
    return b;
}

inline void \Table::Remove (\TableKey1 k1, \TableKey2 k2) {
    Table2::Remove((void*)k1, (void*)k2);
}

#endif
