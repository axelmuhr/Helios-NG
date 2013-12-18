/*
 * Associative map between objects.
 */

#include "InterViews/table.h"

class TableEntry {
    friend class Table;

    void* key;
    void* value;
    TableEntry* chain;
};

Table::Table (int n) {
    register TableEntry** e;

    size = 32;
    while (size < n) {
	size <<= 1;
    }
    first = new TableEntry*[size];
    --size;
    last = &first[size];
    for (e = first; e <= last; e++) {
	*e = nil;
    }
}

Table::~Table () {
    delete first;
}

inline TableEntry* Table::Probe (void* i) {
    return first[(unsigned int)i & size];
}

inline TableEntry** Table::ProbeAddr (void* i) {
    return &first[(unsigned int)i & size];
}

void Table::Insert (void* k, void* v) {
    register TableEntry* e;
    register TableEntry** a;

    e = new TableEntry;
    e->key = k;
    e->value = v;
    a = ProbeAddr(k);
    e->chain = *a;
    *a = e;
}

boolean Table::Find (void*& v, void* k) {
    register TableEntry* e;

    for (e = Probe(k); e != nil; e = e->chain) {
	if (e->key == k) {
	    v = e->value;
	    return true;
	}
    }
    return false;
}

void Table::Remove (void* k) {
    register TableEntry* e, * prev;
    TableEntry** a;

    a = ProbeAddr(k);
    e = *a;
    if (e != nil) {
	if (e->key == k) {
	    *a = e->chain;
	    delete e;
	} else {
	    do {
		prev = e;
		e = e->chain;
	    } while (e != nil && e->key != k);
	    if (e != nil) {
		prev->chain = e->chain;
		delete e;
	    }
	}
    }
}
