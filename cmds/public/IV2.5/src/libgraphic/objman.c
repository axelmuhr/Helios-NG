/*
 * ObjectMan (object manager) class implementation.  ObjectMan is used by
 * Persistent.
 */

#include <InterViews/Graphic/cache.h>
#include <InterViews/Graphic/classes.h>
#include <InterViews/Graphic/objman.h>
#include <InterViews/Graphic/pfile.h>
#include <InterViews/Graphic/ref.h>
#include <InterViews/Graphic/reflist.h>
#include <string.h>

char* OBJMAP_POSTFIX = ".map";
char* OBJSTORE_POSTFIX = ".sto";

static const int BUCKETS = 1000;	// # of buckets in cache hash table

ObjectMan* TheManager = nil;		// global object manager object
					// (user initialized!)

static Persistent* ObjectConstruct (ClassId id) {
    switch (id) {
	case PERSISTENT:    return new Persistent;
	case OBJECTMAN:	    return new ObjectMan;
	default:	    return nil;
    }
}

ClassId ObjectMan::GetClassId () { 
    return OBJECTMAN;
}

boolean ObjectMan::IsA (ClassId id) { 
    return OBJECTMAN == id || Persistent::IsA(id);
}

ObjectMan::ObjectMan () { }

boolean ObjectMan::seek (UID uid) {	// into Persistent store
    int objOffset;
    boolean ok = objMap->SeekTo(getOffset(uid)) && objMap->Read(objOffset);

    if (ok && objOffset == INVALIDOFFSET) {
	Fatal("nonexistent persistent object referenced");
    } else if (ok && objStore->SeekTo(objOffset)) {
	return true;
    } else {
	Warning("seek into object store failed");
    }
    return false;
}

boolean ObjectMan::read (PFile* f) {
    int last;
    char tmpfilename[NAMESIZE];
    boolean ok = Persistent::read(f) &&
	f->Read(tmpfilename) && root->Read(f) && f->Read(last);

    if (!ok) {
	Fatal("couldn't read object manager");
    } else if (strcmp(tmpfilename, filename) != 0) {
	delete objMap;
	delete objStore;
	char objMapName[NAMESIZE], objStoreName[NAMESIZE];
    
	strcpy(filename, tmpfilename);
	strcpy(objMapName, filename);
	strcat(objMapName, OBJMAP_POSTFIX);
	strcpy(objStoreName, filename);
	strcat(objStoreName, OBJSTORE_POSTFIX);

	objMap = new PFile (objMapName);
	objStore = new PFile (objStoreName);
	lastuid = last;
    }
    return ok;
}

boolean ObjectMan::write (PFile* f) {
    boolean ok = Persistent::write(f) && f->Write(filename) && root->Write(f);
    lastuidOffset = f->CurOffset();
    ok = ok && f->Write((int)lastuid);
    if (!ok) {
	Fatal("couldn't write object manager");
    }
    return ok;
}


ObjectMan::ObjectMan (
    char* filename,
    void (*userInitializer) (RefList*),
    Persistent* (*userCreator) (ClassId)
) {
    this->userInitializer = userInitializer;
    this->userCreator = userCreator;

    objCache = new Cache (BUCKETS);
    char objMapName[NAMESIZE], objStoreName[NAMESIZE];

    strcpy(this->filename, filename);
    strcpy(objMapName, filename);
    strcat(objMapName, OBJMAP_POSTFIX);
    strcpy(objStoreName, filename);
    strcat(objStoreName, OBJSTORE_POSTFIX);

    objMap = new PFile (objMapName);
    objStore = new PFile (objStoreName);

    Ref uid (lastuid = OBJMANUID);
    root = new RefList;
    int id;
    if (objStore->IsEmpty()) {
	if (userInitializer != nil ) {
	    (*userInitializer)(root);	// initialize root object(s)
	}
    } else if (
	!(seek(OBJMANUID) && objStore->Read(id) && read(objStore))
    ) {
	Panic( "can't read root object" );
    }
    Ref refto (this);
    objCache->Set( uid, refto );
    objCache->Set( refto, uid );
}

ObjectMan::~ObjectMan () {
    Touch();				// so we'll get written out
    if (!objCache->Flush()) {
	Panic("couldn't flush cache during object manager destruction");
    }
    if (! (objStore->SeekTo(lastuidOffset) && objStore->Write((int)lastuid))) {
	Panic("couldn't write out lastuid");
    }
    delete objCache;
    delete objMap;
    delete objStore;
}

Persistent* ObjectMan::Create (ClassId id) {
    Persistent* newObj = nil;
    char string[80];

    if (userCreator != nil) {
	newObj = (*userCreator)(id);
    }
    if (newObj == nil && (newObj = ObjectConstruct(id)) == nil) {
	sprintf(
	    string, "object creation function(s) returned nil for id %d", id
	);
	Warning(string);
    }
    return newObj;
}

UID ObjectMan::GetUID (Persistent* obj) {
    Ref search( obj );
    Ref result = objCache->Get( search );
    if ( !result.Valid() ) {			// not found
	result = Ref(nextUID());
	objCache->touch(result);		// cached initially dirty
	objCache->Set( search, result );
	objCache->Set( result, search );	// both mappings cached
    }
    return result.getUID();
}

boolean ObjectMan::Invalidate (Persistent* obj) {
    Ref search (obj);
    Ref result = objCache->Get( search );
    boolean ok = true;
    if ( result.Valid() ) {			// found
	ok = objMap->SeekTo(getOffset(result.getUID())) &&
	    objMap->Write(INVALIDOFFSET);	// invalidate objMap offset
	objCache->Unset(search);		// invalidate both mappings
	objCache->Unset(result);
    }
    return ok;
}

boolean ObjectMan::IsCached (Ref* ref) {
    UID uid = ref->getUID();
    Ref refto = objCache->Get( Ref(uid) );	// must be uid

    if ( refto.Valid() ) {			// was in cache
	*ref = refto;
	return true;
    } else {
	return false;
    }
}

boolean ObjectMan::Find (Ref* ref) {
    return
	IsCached(ref) || ( seek( ref->getUID() ) && Retrieve(ref) );
}

boolean ObjectMan::Retrieve (Ref* ref) {
    int id;
    Persistent* obj;

    UID uid = ref->getUID();
    if (IsCached(ref)) {
	Warning (
	    "attempt to read (clustered) object that is already in memory"
	);
    }
    if (
	objStore->Read(id)
	&& (ref->refto = Create((ClassId)id)) != nil
	&& ref->refto->read( objStore )
    ) {
	objCache->Set(uid, *ref);
	objCache->Set(*ref, uid);
	obj = (*ref)();
	return obj->readObjects(objStore) && obj->initialize();
    } else {
	return false;
    }
}

boolean ObjectMan::Store (Persistent* obj) {
    if ( obj == nil ) {
	Warning("attempt to Store through nil pointer");
	return false;
    }
    boolean ok = objStore->SeekToEnd();
    currentOffset = objStore->CurOffset();
    Persistent* head = obj->GetCluster();
    if (head == nil) {
	Warning("an object within a cluster was explicitly saved");
    } else {
	ok = ok && head->writeObjects(objStore);
    }
    return ok;
}

boolean ObjectMan::Update (Persistent* obj) {
    if ( obj == nil ) {
	Warning("attempt to Update through nil pointer");
	return false;
    }
    UID uid = obj->getUID();
    return
	objMap->SeekTo( getOffset( uid ) )
	&& objMap->Write( int(currentOffset) )
	&& objStore->Write( (int)obj->GetClassId() );
}


boolean ObjectMan::ObjectIsDirty (Persistent* obj) {
    return objCache->IsDirty(Ref(obj));
}

void ObjectMan::ObjectTouch (Persistent* obj) {
    objCache->Touch(Ref(obj));
}

void ObjectMan::ObjectClean (Persistent* obj) {
    objCache->Clean(Ref(obj));
}
