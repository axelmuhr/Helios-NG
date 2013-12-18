/*
 * A catalog is used to map names to objects.
 */

#ifndef catalog_h
#define catalog_h

#include <InterViews/stub.h>

class Catalog {
public:
    Catalog(int size);
    ~Catalog();

    void Register(const char*, ObjectStub*);
    void UnRegister(const char*);
    boolean Find(ObjectStub*&, const char*);
private:
    struct CatalogEntry {
	char* name;
	int len;
	ObjectStub* obj;
	CatalogEntry* chain;
    };

    int nelements;
    CatalogEntry** first;
    CatalogEntry** last;

    unsigned Hash(const char*, int&);
};

#endif
