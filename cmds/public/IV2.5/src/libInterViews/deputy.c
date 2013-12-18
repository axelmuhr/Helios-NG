/*
 * Implementation of base deputy class.
 */

#include <InterViews/deputy.h>
#include <InterViews/chief.h>

Deputy::Deputy () {
    chief = nil;
}

Deputy::Deputy (ChiefDeputy* c) {
    chief = c;
}

Deputy::~Deputy () {
    /* nothing to do */
}

void Deputy::Sync () {
    chief->Sync();
}

Connection* Deputy::GetServer () {
    return chief->GetServer();
}
