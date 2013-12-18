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
    void Insert(\TableKey, \TableValue);
    boolean Find(\TableValue&, \TableKey);
    void Remove(\TableKey);
private:
    int size;
    \TableEntry** first;
    \TableEntry** last;

    \TableEntry* Probe(\TableKey);
    \TableEntry** ProbeAddr(\TableKey);
};

#endif
