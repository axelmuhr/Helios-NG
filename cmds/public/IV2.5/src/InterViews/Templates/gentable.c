/*
 * Associative map between objects.
 */

#include "\name.h"

class \TableEntry {
    friend class \Table;

    \TableKey key;
    \TableValue value;
    \TableEntry* chain;
};

\Table::\Table (int n) {
    register \TableEntry** e;

    size = 32;
    while (size < n) {
	size <<= 1;
    }
    first = new \TableEntry*[size];
    --size;
    last = &first[size];
    for (e = first; e <= last; e++) {
	*e = nil;
    }
}

\Table::~\Table () {
    delete first;
}

inline \TableEntry* \Table::Probe (\TableKey i) {
    return first[(unsigned int)i & size];
}

inline \TableEntry** \Table::ProbeAddr (\TableKey i) {
    return &first[(unsigned int)i & size];
}

void \Table::Insert (\TableKey k, \TableValue v) {
    register \TableEntry* e;
    register \TableEntry** a;

    e = new \TableEntry;
    e->key = k;
    e->value = v;
    a = ProbeAddr(k);
    e->chain = *a;
    *a = e;
}

boolean \Table::Find (\TableValue& v, \TableKey k) {
    register \TableEntry* e;

    for (e = Probe(k); e != nil; e = e->chain) {
	if (e->key == k) {
	    v = e->value;
	    return true;
	}
    }
    return false;
}

void \Table::Remove (\TableKey k) {
    register \TableEntry* e, * prev;
    \TableEntry** a;

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
