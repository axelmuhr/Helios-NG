/*
 * An object stub interprets byte stream messages.
 * Stubs can be referenced from several other spaces;
 * therefore they must be reference-counted.
 */

#ifndef objectstub_h
#define objectstub_h

#include <InterViews/defs.h>
#include <InterViews/tag.h>

class ObjectStub {
public:
    ObjectStub();
    ~ObjectStub();

    void Reference () { ++refcount; }
    boolean LastRef () { return refcount == 1; }

    virtual void Message(
	class Connection* sender, ObjectTag, int op, void* msg, int len
    );
    virtual void ChannelReady(int);
    virtual ObjectStub* Clone();
private:
    unsigned refcount;
};

#endif
