/*
 * References to resource objects are counted for convenient sharing.
 */

#ifndef resource_h
#define resource_h

#include <InterViews/defs.h>

class Resource {
public:
    Resource () { refcount = 1; }
    ~Resource () { if (--refcount > 0) { this = 0; } }

    void Reference () { ++refcount; }
    boolean LastRef () { return refcount == 1; }
private:
    unsigned refcount;
};

#endif
