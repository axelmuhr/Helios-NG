/*
 * A catalog maps names to objects.
 */

#include <InterViews/catalog.h>
#include <bstring.h>
#include <string.h>

Catalog::Catalog (int n) {
    typedef CatalogEntry* EntryPtr;

    nelements = 128;
    while (nelements < n) {
	nelements <<= 1;
    }
    first = new EntryPtr[nelements];
    bzero(first, nelements * sizeof(EntryPtr));
    --nelements;
    last = &first[nelements];
}

Catalog::~Catalog () {
    register CatalogEntry** i;
    register CatalogEntry* e;
    register CatalogEntry* next;

    for (i = first; i <= last; i++) {
	for (e = *i; e != nil; e = next) {
	    next = e->chain;
	    delete e->name;
	    delete e->obj;
	    delete e;
	}
    }
    delete first;
}

/*
 * Enter a new object into a catalog.
 */

void Catalog::Register (const char* name, ObjectStub* s) {
    register unsigned h;
    int len;
    register CatalogEntry* e;

    h = Hash(name, len);
    e = new CatalogEntry;
    e->name = new char[len+1];
    e->len = len;
    strcpy(e->name, name);
    e->obj = s;
    e->chain = first[h];
    first[h] = e;
}

void Catalog::UnRegister (const char* name) {
    register unsigned h;
    int len;
    register CatalogEntry* e;
    register CatalogEntry* prev;

    h = Hash(name, len);
    e = first[h];
    prev = nil;
    while (e != nil && (e->len != len || strcmp(e->name, name) != 0)) {
	prev = e;
	e = e->chain;
    }
    if (e != nil) {
	if (prev == nil) {
	    first[h] = e->chain;
	} else {
	    prev->chain = e->chain;
	}
	delete e->name;
	delete e;
    }
}

boolean Catalog::Find (ObjectStub*& s, const char* name) {
    int len;
    register CatalogEntry* e;

    for (e = first[Hash(name, len)]; e != nil; e = e->chain) {
	if (e->len == len && strcmp(e->name, name) == 0) {
	    s = e->obj;
	    return true;
	}
    }
    return false;
}

/*
 * Return a hash value for a string name and set the length of the name
 * as a side effect.
 */

unsigned Catalog::Hash (const char *name, int& len) {
    register const char *c;
    register unsigned h;

    h = 0;
    c = name;
    while (*c != '\0') {
	h = (h << 1) ^ ((int) (*c));
	++c;
    }
    len = (c - name);
    return h & nelements;
}
