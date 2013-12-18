// Interface to class Ctdt

#ifndef _ctdt_h
#define _ctdt_h

#include "module.h"

class Ctdt : public Module {
public:
    Ctdt();

    void RemoveRelocs();
    void Select(const char*);
    void Munch();
    void WriteSource();

private:
    Symbol* ctor;       // place-holder for _ctor[]
    Symbol* dtor;       // place-holder for _dtor[]
    BaseName* ctors;    // list of names of static initializers
    BaseName* dtors;    // list of names of static destructors
    int nctors;         // number of initializers
    int ndtors;         // number of destructors

    boolean NotFound(const char*, BaseName*&);
    void Add(const char*, BaseName*&);
    void DoMunch(BaseName*, int, int);
    int AddRelocList(BaseName*&, int, int);
    void GenReloc(Symbol*, int, int);
    void UpdateSymtab();

    void DoWrite(FILE*, BaseName*, const char*);
    void Compile();
};

#endif
