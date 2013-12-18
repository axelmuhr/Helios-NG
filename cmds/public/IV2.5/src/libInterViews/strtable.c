/*
 * Map strings to unique pointers.
 */

#include <InterViews/strpool.h>
#include <InterViews/strtable.h>
#include <string.h>

static const int nolength = -1;

static StringPool* pool;

StringTable::StringTable (int n) {
    register StringId** s;

    size = 32;
    while (size < n) {
	size <<= 1;
    }
    first = new StringId*[size];
    --size;
    last = &first[size];
    for (s = first; s <= last; s++) {
	*s = nil;
    }
    if (pool == nil) {
	pool = new StringPool;
    }
}

StringTable::~StringTable () {
    delete first;
}

inline StringId** StringTable::Probe (const char* str, int& len) {
    return &first[Hash(str, len) & size];
}

StringId* StringTable::Id (const char* str) {
    return Id(str, nolength);
}

StringId* StringTable::Id (const char* str, int len) {
    register StringId* s;
    register StringId** a;
    register int n;
    int slen;
    char* newstr;

    slen = len;
    a = Probe(str, slen);
    n = slen;
    for (s = *a; s != nil; s = s->chain) {
	if (s->len == n && strncmp(s->str, str, n) == 0) {
	    return s;
	}
    }
    s = new StringId;
    newstr = pool->Append(str, n + 1);
    newstr[n] = '\0';
    s->str = newstr;
    s->len = n;
    s->chain = *a;
    *a = s;
    return s;
}

void StringTable::Remove (const char* str) {
    Remove(str, nolength);
}

void StringTable::Remove (const char* str, int len) {
    register StringId* s, * prev;
    StringId** a;

    a = Probe(str, len);
    s = *a;
    if (s != nil) {
	if (s->len == len && strcmp(s->str, str) == 0) {
	    *a = s->chain;
	    delete s;
	} else {
	    for (prev = s; ; s = s->chain) {
		if (s == nil) {
		    break;
		}
		if (s->len == len && strcmp(s->str, str) == 0) {
		    prev->chain = s->chain;
		    delete s;
		    break;
		}
	    }
	}
    }
}

unsigned int StringTable::Hash (const char* str, int& len) {
    register const char* p;
    register unsigned int v;
    register unsigned int t;

    v = 0;
    if (len == nolength) {
	for (p = str; *p != '\0'; p++) {
	    v += v;
	    v ^= *p;
	}
	len = p - str;
    } else {
	const char* q = &str[len];
	for (p = str; p < q; p++) {
	    v += v;
	    v ^= *p;
	}
    }
    t = v >> 10;
    t ^= t >> 10;
    v ^= t;
    return v;
}
