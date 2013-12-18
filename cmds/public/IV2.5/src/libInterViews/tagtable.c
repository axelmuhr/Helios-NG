/*
 * Object tables are implemented by hashing resolved by chaining.
 */

#include <InterViews/tagtable.h>
#include <bstring.h>

/*
 * Create a new table of (at least) a given size.
 *
 * To make hashing faster, the size is rounded up to the next power of 2 and
 * then decremented so it can be used as a mask.
 */

ObjectTable::ObjectTable (int n) {
    typedef ObjectTableEntry* EntryPtr;

    nelements = 128;
    while (nelements < n) {
	nelements <<= 1;
    }
    first = new EntryPtr[nelements];
    bzero(first, nelements * sizeof(EntryPtr));
    --nelements;
    last = &first[nelements];
}

ObjectTable::~ObjectTable () {
    delete first;
}

inline unsigned ObjectTable::Hash (Connection* c, ObjectTag t) {
    return ((((unsigned) c) >> 2) ^ (t >> 2)) & nelements;
}

inline ObjectTableEntry* ObjectTable::Start (Connection* c, ObjectTag t) {
    return first[Hash(c, t)];
}

inline ObjectTableEntry** ObjectTable::StartAddr (Connection* c, ObjectTag t) {
    return &first[Hash(c, t)];
}

inline boolean ObjectTableEntry::Match (Connection* c, ObjectTag t) {
    return client == c && id == t;
}

/*
 * Add an object into the table.
 */

void ObjectTable::Add (Connection* c, ObjectTag t, ObjectStub* s) {
    register ObjectTableEntry* e;
    register ObjectTableEntry** a;

    e = new ObjectTableEntry;
    e->client = c;
    e->id = t;
    e->ref = s;
    a = StartAddr(c, t);
    e->chain = *a;
    *a = e;
}

/*
 * Find an object in the table.
 * If found return a pointer to the object, otherwise return nil.
 */

ObjectStub* ObjectTable::Find (Connection* c, ObjectTag t) {
    register ObjectTableEntry* e;

    for (e = Start(c, t); e != nil; e = e->chain) {
	if (e->Match(c, t)) {
	    return e->ref;
	}
    }
    return nil;
}

/*
 * Remove an object from the table.
 */

void ObjectTable::Remove (Connection* c, ObjectTag t) {
    register ObjectTableEntry* e;
    register ObjectTableEntry* prev;
    register ObjectTableEntry** a;

    a = StartAddr(c, t);
    e = *a;
    if (e->Match(c, t)) {
	*a = e->chain;
    } else {
	do {
	    prev = e;
	    e = e->chain;
	} while (e != nil && !e->Match(c, t));
	if (e != nil) {
	    prev->chain = e->chain;
	    delete e;
	}
    }
}

/*
 * Remove all the objects associated with a given connection.
 */

void ObjectTable::RemoveAll (Connection* c) {
    register ObjectTableEntry** i;
    register ObjectTableEntry* e;
    register ObjectTableEntry* next;
    register ObjectTableEntry* prev;

    for (i = first; i <= last; i++) {
	prev = nil;
	for (e = *i; e != nil; e = next) {
	    next = e->chain;
	    if (e->client == c) {
		if (prev == nil) {
		    *i = e->chain;
		} else {
		    prev->chain = e->chain;
		}
		delete e->ref;
		delete e;
	    }
	}
    }
}
