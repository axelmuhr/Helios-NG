/*
 * Interface to Persistent and related base classes.
 */

#ifndef persistent_h
#define persistent_h

#include <InterViews/defs.h>
#include <InterViews/Graphic/classes.h>

#define INVALIDUID	1
			  /* INVALIDUID & OBJMANUID must be consecutive uids */
#define OBJMANUID	3   

typedef unsigned UID;

class ListObject;
class ObjectMan;
class PFile;
class Persistent;

extern ObjectMan* TheManager;	    /* the well-known global object manager */

class Persistent {		    /* basic (persistent) Persistent */
    friend class Ref;
    friend class ObjectMan;
public:
    Persistent();
    virtual ~Persistent();

    virtual Persistent* GetCluster();
	/*
	 * returns head of cluster.  If GetCluster is redefined to return
	 * nil, then the object is assumed to be within the "current"
	 * cluster, i.e., it is a part of whatever cluster is currently
	 * being written out or read in.  If the object is Saved explicitly,
	 * a warning is issued and the object is NOT saved.
	 */
    virtual boolean Save();
    virtual boolean IsDirty();
    virtual void Touch();
    virtual void Clean();

    virtual ClassId GetClassId();
    virtual boolean IsA(ClassId);
protected:
    virtual UID getUID();

    void Warning(const char*);
    void Panic(const char*);
    void Fatal(const char*);

    virtual boolean write(PFile*);
    virtual boolean read(PFile*);
    virtual boolean writeObjects(PFile*);
    virtual boolean readObjects(PFile*);
    virtual boolean initialize();
};

#endif
