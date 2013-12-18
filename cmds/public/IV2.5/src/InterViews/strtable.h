/*
 * String table for mapping to unique pointers.
 */

#ifndef strtable_h
#define strtable_h

#include <InterViews/defs.h>

class StringId {
public:
    const char* Str();
private:
    friend class StringTable;

    const char* str;
    int len;
    StringId* chain;
};

inline const char* StringId::Str () { return str; }

class StringTable {
public:
    StringTable(int);
    ~StringTable();

    StringId* Id(const char*);
    StringId* Id(const char*, int);
    void Remove(const char*);
    void Remove(const char*, int);
private:
    int size;
    StringId** first;
    StringId** last;

    StringId** Probe(const char*, int&);
    unsigned int Hash(const char*, int&);
};

#endif
