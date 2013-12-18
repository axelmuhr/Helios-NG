/*
 * A string pool is an area for allocating string constants
 * without the overhead (hopefully) of malloc/new.
 */

#include <InterViews/strpool.h>
#include <string.h>

StringPool::StringPool (int poolsize) {
    data = new char[poolsize];
    size = poolsize;
    cur = 0;
    prev = nil;
};

/*
 * Tail-recursive deletion to walk the list back to the head
 * of the pool.
 */

StringPool::~StringPool () {
    delete data;
    delete prev;
}

/*
 * Add a string of a given length to the pool.  If it won't fit,
 * create a copy of the current pool and allocate space for a new one.
 *
 * No null-padding is implied, so if you want that you must include
 * the null in the length.
 */

char* StringPool::Append (const char* str, int len) {
    register int index = cur;	/* cse: this->cur */
    register int newcur = index + len;
    if (newcur > size) {
	StringPool* s = new StringPool;
	*s = *this;
	data = new char[size];
	prev = s;
	index = 0;
	newcur = len;
    }
    char* r = &data[index];
    strncpy(r, str, len);
    cur = newcur;
    return r;
}
