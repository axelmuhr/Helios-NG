/*
 * RefList class implementation.  A RefList is a list of Refs, which are
 * references to Persistent objects.
 */

#include <InterViews/Graphic/reflist.h>

RefList* RefList::Find (Ref p) {
    register RefList* e;

    for (e = next; e != this; e = e->next) {
	if (*e == p) {
	    return e;
	}
    }
    return nil;
}

boolean RefList::Write (PFile* f) {
    RefList* i;
    int count = 0;

    for (i = next; i != this; i = i->next) {
        ++count;
    }
    boolean ok = f->Write(count);
    for (i = next; i != this && ok; i = i->next) {
	ok = ((Ref*) i)->Write(f);
    }
    return ok;
}

boolean RefList::Read (PFile* f) {
    int count;
    RefList* i;

    boolean ok = f->Read(count);
    for (--count; count >= 0 && ok; --count) {
	i = new RefList;
	ok = ((Ref*) i)->Read(f);
	Append(i);
    }
    return ok;
}

boolean RefList::WriteObjects (PFile* f) {
    RefList* i;
    boolean ok = true;

    for (i = next; i != this && ok; i = i->next) {
	ok = ((Ref*) i)->WriteObjects(f);
    }
    return ok;
}

boolean RefList::ReadObjects (PFile* f) {
    RefList* i;
    boolean ok = true;

    for (i = next; i != this && ok; i = i->next) {
	ok = ((Ref*) i)->ReadObjects(f);
    }
    return ok;
}

RefList* RefList::operator[] (int count) {
    RefList* pos = First();
    int i;

    for (i = 1; i < count && pos != End(); ++i) {
	pos = pos->Next();
    }
    if (i == count) {
	return pos;
    }
    return nil;
}	
