#ifndef base_h
#define base_h

/* 
 * Hierarchry of classes: (all derived from Base)
 * Program, Module, Chunk, Symbol, Location.
 * program owns modules owns chunks owns symbols owns locations.
 * defines the superior/inferior relation. 
 */

#include "types.h"
#include <limits.h>

class StrTable;
extern StrTable *strTab;

class Base;
class BaseName;
class BaseNameMod;
class Location;
class Symbol;
class Chunk;
class Module;
class Program;

typedef enum { BASE, LOC, SYMBOL, CHUNK, MODULE, LIBRARY, PROGRAM } What;

class Base {
    What  id;			/* type of class */
public:
    Base* owner;		/* same as superior */
    Base* inf;
    Base* pp;			/* pointer to same class as this */
    Base* nn;			/* pointer to same class as this */
    
    Base(What);
    virtual ~Base();
    void BaseInit () { owner = inf = nn = pp = nil; }
    What WhatAmI () { return id; }
    /*
     * called by the object wanting to be inserted.  param = head of list
     * typical use : minor->Link(major.listHead);
     */
    void Link (Base*& head) {
	if (head == nil) {
	    head = this;
	    nn = this;
	    pp = this;
	} else {
	    pp = head->pp;
	    head->pp->nn = this;
	    nn = head;
	    head->pp = this;
	}
    }
    void Unlink (Base*& head);

    void InsertInf (Base* sap) { sap->Link(this->inf); sap->owner = this; }
    void DeleteInf (Base*);
    boolean AmIHead () { return (owner != nil && owner->inf == this); }
    void ZeroMe () { owner = nil; inf = nil; pp = nil; nn = nil; }
};

/* 
 * same as the base only there is a name associated with this.
 * used for Program, and Symbols.
 */
class BaseName : public Base {
public:
    STindex name;

    BaseName(What, const char*);
    BaseName(What, STindex);
    BaseName (What id) : (id) { name = -1; }
    ~BaseName();

    const char* GetName () {
        return (name == -1) ? nil : strTab->IdxToStr(name);
    }
    void Enter();
};

/* Base class for modules and libraries that share operations on
 * input files.
 */
class InputFile : public BaseName {
public:
    InputFile(What id, const char* name, FILE* file);
    ~InputFile();

    FILE* GetFile () { return file; }
    virtual void OpenFile();
    virtual void CloseFile();

    virtual void WriteText(FILE*);
    virtual void WriteData(FILE*);
    virtual void WriteSyms(FILE*);
    virtual void WriteStrTab(FILE*);

    void Update();
    virtual void Reread();

protected:
    FILE* file;		    /* file descriptor when open else nil */
    time_t updateTime;      /* last updatetime */
    Exec ex;		    /* a.out header */
};

#endif
