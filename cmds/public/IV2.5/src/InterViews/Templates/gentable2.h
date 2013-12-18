/*
 * Object association table.
 */

#ifndef \Table_h
#define \Table_h

#include <InterViews/defs.h>

class \TableEntry;

class \Table {
public:
    \Table(int);
    ~\Table();
    void Insert(\TableKey1, \TableKey2, \TableValue);
    boolean Find(\TableValue&, \TableKey1, \TableKey2);
    void Remove(\TableKey1, \TableKey2);
private:
    int size;
    \TableEntry** first;
    \TableEntry** last;

    \TableEntry* Probe(\TableKey1, \TableKey2);
    \TableEntry** ProbeAddr(\TableKey1, \TableKey2);
};

#endif
