/*
 * Default stub routines.
 */

#include <InterViews/stub.h>

ObjectStub::ObjectStub () {
    refcount = 1;
}

ObjectStub::~ObjectStub () {
    --refcount;
    if (refcount != 0) {
	this = 0;
    }
}

void ObjectStub::Message (Connection*, ObjectTag, int, void*, int) {
    /* do nothing */
}

void ObjectStub::ChannelReady (int) {
    /* do nothing */
}

ObjectStub* ObjectStub::Clone () {
    return nil;
}
