/*
 * A string pool is an area for allocating string constants
 * without the overhead (hopefully) of malloc/new.
 */

#ifndef strpool_h
#define strpool_h

#include <InterViews/defs.h>

class StringPool {
public:
    StringPool(int poolsize = 800);
    ~StringPool();

    char* Append(const char*, int);
private:
    char* data;
    int size;
    int cur;
    StringPool* prev;
};

#endif
