/*
 * Persistent class implementation.  Persistent is the general persistent
 * object from which Graphic is derived.
 */

#include <InterViews/Graphic/objman.h>
#include <InterViews/Graphic/persistent.h>
#include <InterViews/Graphic/pfile.h>
#include <stdlib.h>

void Persistent::Warning (const char* msg) {
    fflush(stdout);
    fprintf(stderr, "warning: %s\n", msg);
}

void Persistent::Panic (const char* msg) {
    fflush(stdout);
    fprintf(stderr, "internal error: %s\n", msg);
    exit(2);
}

void Persistent::Fatal (const char* msg) {
    fflush(stdout);
    fprintf(stderr, "%s\n", msg);
    exit(2);
}

UID Persistent::getUID () {
    return TheManager->GetUID( this );
}

boolean Persistent::write (PFile*) {
    if (TheManager->Update(this)) {
	Clean();
	return true;
    } else {
	return false;
    }
}

boolean Persistent::read (PFile*) {
    return true;
}

boolean Persistent::readObjects (PFile*) {
    return true;
}

boolean Persistent::initialize () {
    return true;
}

ClassId Persistent::GetClassId () { 
    return PERSISTENT;
}

boolean Persistent::IsA (ClassId id) { 
    return PERSISTENT == id;
}

Persistent::Persistent () { } 

Persistent::~Persistent () {
    if (
	TheManager != nil && this != TheManager &&
	!TheManager->Invalidate(this)
    ) {
	Panic("couldn't invalidate object during destruction");
    }
}

Persistent* Persistent::GetCluster () { 
    return this;
}

boolean Persistent::Save () {
    return IsDirty() ? TheManager->Store(this) : true;
}

boolean Persistent::writeObjects (PFile* f) {
    return write(f);
}

boolean Persistent::IsDirty () {
    if (TheManager == nil) {
	return true;
    } else {
	return TheManager->ObjectIsDirty(this);
    }
}

void Persistent::Touch () {
    if (TheManager != nil) {
	TheManager->ObjectTouch(this);
    }
}

void Persistent::Clean () {
    if (TheManager != nil) {
	TheManager->ObjectClean(this);
    }
}
