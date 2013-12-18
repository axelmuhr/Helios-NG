/*
 * Symbols class.  All objects defined in a module.
 */

#include "chunk.h"
#include "errhandler.h"
#include "lib.h"
#include "loc.h"
#include "module.h"
#include "program.h"
#include "symbols.h"
#include "types.h"

static Symbol* AllocSym(NList*, Module*, Chunk*);

/* 
 * Ignore useless symbols like stabs.
 * Determine the chunk a symbol belongs to. 
 */
Symbol* DoSym (NList* nl, Module* mod) {
    char symName[256];
    Symbol* s, * t;
    int type;
    Chunk* ch;

    s = nil;
    type = nl->Type();
    if (type == N_FN) {
	return s;
    }
    if (nl->Stab() != 0) {
	return s;			/* ignore dbx stabs */
    }
    if (!nl->Global() && (type == N_UNDF || type == N_ABS)) {
	return s;	    		/* remove local assembler syms */
    }
    if (nl->Index() != 0) {
	if (nl->Global()) {
	    mod->ReadSymName(nl->n_un.n_strx, symName);
	    t = GetSym(symName ,mod);	/* look for all syms with this name */
	    ch = mod->DetermineChunk(type);
	    if (t != nil) {
		s = t->Resolve(nl, mod, ch); /* new sym or ref to old one? */
	    } else {
		nl->n_un.n_strx = strTab->AddString(symName);
		s = AllocSym(nl, mod, ch);
	    }
            if (type == N_TEXT) {
                currProg->SelectCtdt(symName);
            }
	}
    } else {
	if (nl->Stab() == 0) {
	    fprintf(
		dumpFile,
		"Local non stab: no name ndesc=%d, ntype=%d nvalue=%d\n",
		nl->n_desc, nl->n_type, nl->n_value
	    );
	}
    }
    return s;
}

Symbol::Symbol (
    NList *nl, Module* mod, Chunk* ch
) : (SYMBOL, STindex(nl->Index())) {
    owner = nil;
    currAddr = -1;
    addrChanged = true;
    UpdateUndef(nl, mod, ch);
}

void Symbol::NewSymbol (NList *nl, Module* mod, Chunk* ch) {
    CellPointer cp;

    name = nl->Index();
    cp = symTab->Ins2(name, this);
    UpdateUndef(nl, mod, ch);
}

static int numSymInUse = 0;
static Symbol* symFreeList = nil;
static int symsInBuff = 0;
static Symbol* symBuff = nil;
const int SYMCHUNK = 102;

int NumSyms () { return numSymInUse; }

/* This should be replaced with constructors as defined in section 5.5.6
 * of the C++ book.
 */
static void RefillSymBuff () {
    symBuff = (Symbol*) new char[ sizeof(Symbol)*SYMCHUNK ];
    symsInBuff = SYMCHUNK;
}

static Symbol* AllocSym (NList *nl, Module* mod, Chunk* ch) {
    Symbol* s;

    if (symFreeList == nil) {
	if (symsInBuff == 0) {
	    RefillSymBuff();
	}
	s = symBuff;
	symsInBuff -= 1;
	symBuff += 1;			/* pointer arithmetic */
    } else {
	s = symFreeList;
	symFreeList = (Symbol*) s->pp;
    }
    s->inf = nil;
    s->NewSymbol(nl, mod, ch);

    numSymInUse += 1;			/* +1 for every call */
    return s;
}

/* 
 * given name, mod: find local symbol defined in mod, or global sym 
 */
Symbol* GetSym (const char* name, Module* mod) {
    boolean found;
    Symbol* localGuy, * globalGuy, * tmp;
    
    localGuy = nil;
    globalGuy = nil;
    
    found = symTab->Get(name, tmp);
    if (!found) {
	return nil;
    }
    do {
	if (tmp->global) {
	    if (globalGuy == nil) {
		globalGuy = tmp;
	    } else {
		Panic("GetSym : two definitions for (%s)", name);
	    }
	} else {
	    if (tmp->owner->owner == mod) {
		if (localGuy == nil) {
		    localGuy = tmp;
		}
	    }
	}
    } while (localGuy == nil && symTab->GetNext(tmp) );
    if (localGuy == nil) {
	return globalGuy;
    } else {
	return localGuy;
    }
}

void Symbol::Dump (DumpLevel) {
}

/* 
 * symbol exists with same name as nl: create new sym or merge info 
 * this==existing symbol, nl==current symbol information. 
 */
Symbol* Symbol::Resolve (NList *nl, Module* m, Chunk* ch) {
    Symbol* s;
    int status;
    Module* mm;
    char msg[128];
    
    status = 0;
					/* if two of the same sym
					 * in the same module
					 * the compiler would complain 
					 */
			/* one of the symbols is local, so no conflict */
    if (global != true || nl->Global() == false) {
	s = AllocSym(nl, m, ch);
	return s;
    }
    if (Defined()) {			/* both symbols are global */
	if (nl->Undefined()) {
	    s = this;
	} else {			/* nl has a definite type */
	    if (type == N_ANON && nl->IsVar()) {
		s = UpdateAnon(nl, m);	/* this=anonymous, nl=data */
	    } else if (type == N_DATA && nl->MaybeBss()) {
		s = this;		/* this=data, nl=anonymous */
	    } else {
					/* check if on relink, we redefine a
					 * symbol defined in a library 
					 */
		mm = (Module*) owner->owner;
		if (mm->PartOfLib()) {
		    sprintf(msg,
                        "Sym (%s) redefined in %s. Old def in %s (lib)",
                        GetName(), m->GetName(), mm->GetName()
		    );
		    Warning(msg);
                    DeleteInf(owner);
		    UpdateUndef(nl, m, ch);
		    s = this;
		} else if (m->PartOfLib()) {
		    s = this;		/* ignore current sym def */
		} else {
		    sprintf(msg,
			"error: symbol (%s) redefined in %s.  Old def in %s",
			GetName(), m->GetName(), mm->GetName()
		    );
		    Error(msg);
                    status = E_MULTIPLE;
                    s = nil;
		}
	    }
	}
    } else {
	if (! nl->Undefined()) {
	    UpdateUndef(nl, m, ch);
	}
	s = this;
    }
    return s;
}

/* 
 * Add more information about the symbol
 * the symbol is guaranteed not to exist or to be undefined.
 */
void Symbol::UpdateUndef (NList *nl, Module* mod, Chunk* ch) {
    register int val;
    
    global = nl->n_type & N_EXT;
    type = nl->Type();
    val = nl->n_value;
    switch (type) {
	case N_UNDF:
	    if (val != 0) {
		type = N_ANON;
		offset = -1;
		size = val;
		if (mod != nil) {
		    ch = mod->bss;
		} else {
		    ch = currProg->anonList; /* this was the default action */
		}
		ch = currProg->anonList;
	    }
	    break;
	case N_ABS:
	    if (global) {
		offset = val;
	    }
	    fprintf(dumpFile, "ABS %s\n",GetName());
	    break;
	case N_TEXT:
	    offset = val - ch->origStart;
	    size = 0;
	    break;
	case N_DATA:
	    offset = val- ch->origStart;
	    size = -1;			/* unknown size */
	    break;
	case N_BSS:
	    offset = val - ch->origStart;;
	    break;
	case N_COMM:
	    fprintf(dumpFile, "N_COMM %s\n",GetName());
	    break;
	case N_FN:
	    fprintf(dumpFile, "This is a module - FILENAME %s\n",GetName());
	    break;
	default:
	    fprintf(dumpFile, "??? UNKNOWN type %s",GetName());
	    break;
    }

    if (type != N_UNDF) {
	if (owner == currProg) {
	    currProg->DeleteUndef(this);
/* 	    Unlink(currProg->undefList); */
	}
	if (ch == nil) {
	    Warning("UpdateUndef : ?? Chunk is nil");
	} else {
	    ch->InsertInf(this);
	}
    } else {
	if (global && owner == nil) {
	    currProg->AddUndef(this);
	}
    }
}

/* 
 * Update *this with data from nl.  nl->Type() = N_BSS
 * this is a global var by default alloc'd on the heap.
 */
Symbol* Symbol::UpdateAnon (NList *nl, Module* m) {
    if (type != N_ANON) {
	Warning("UpdateBss : Symbol not of type N_ANON");
	return nil;
    }
    if (nl->Type() == N_DATA) {
	Unlink(currProg->anonList);
	m->data->InsertInf(this);
	offset = nl->n_value + m->data->Move();
	size = -1;
    } else if (nl->Type() == N_BSS) {
	Warning("Symbol.UpdateAnon - global Bss??");
    } else {				/* nl->Type() == N_UNDF */
	if (nl->n_value == 0) {
	    Warning("Symbol.UpdateAnon - huh??");
	} else {
	    if (size != nl->n_value) {
		fprintf(dumpFile,
		    "Warning (%s,%s) redefined w/ different sizes\n",
		    GetName(), m->GetName()
		);
		size = max(size,nl->n_value); /* what unix ld does */
	    }
	}
    }
    return this;
}

/* 
 * Relocate all uses of this symbol 
 */
void Symbol::RelocSym (int destOrig) {
    Location* l = (Location*) inf;
    int refs = 0;
    if (addrChanged != 0 && l != nil) {
	do {
	    l->Reloc(currAddr, destOrig);
	    l = (Location*) l->nn;
	    refs += 1;
	} while (l != inf);
    }
    addrChanged = false;
}

void Symbol::SetAnonAddr (
    int& offIntoChunk, int anonStart, boolean incremental
) {
    int final, alignment;
    
    if (size < 0 || type == -1) {
	Panic("SetAnonAddr(name %s, size %d)", GetName(), size);
    }
    if (size <= sizeof(short)) {
	alignment = sizeof(short);
    } else {
	alignment = sizeof(int);
    }
    offset = AlignTo(alignment, offIntoChunk); /* align data object on word */
    final = offset + anonStart;
    SetAddr(final, incremental);
    offIntoChunk = offset + size;
}

void Symbol::RelocAnon () {
    Location* l = (Location*) inf;
    if (l != nil) {
	do {
	    l->Reloc(currAddr,-1);
	    l = (Location*) l->nn;
	} while (l != inf);
    }
}

/* 
 * routines to set and get the address of a symbol.
 */
int Symbol::WhereAmI () {
    Chunk* ch = (Chunk*) owner;
    int final = currProg->SegStart(ch->type) + ch->Offset(offset);
    SetAddr(final, !currProg->fullLink);
    return final;
}

void Symbol::SetAddr (int finalAddress, boolean rememberAddrChanged) {
    if (currAddr != finalAddress) {
	addrChanged = rememberAddrChanged;
	currAddr = finalAddress;
    } else {
/*	
	if (addrChanged == true) {
	    Warning("Symbols.SetAddr : (%s) huh?", GetName());
	}
*/
	addrChanged = false;
    }
}

int Symbol::GetAddr () {
    if (currAddr == -1) {
	return WhereAmI();
    }
    return currAddr;
}

void Symbol::UndefineSym () {
    if (global && inf) { /* still someone referencing */
        type = N_UNDF;
        offset = -1;
        size = 0;
        Unlink(owner->inf);
	currProg->AddUndef(this);	/* make look like global undefined */
    } else {
        Unlink(owner->inf);
        Dispose();
    }
}

void Symbol::Dispose () {
    type = N_UNDF;
    offset = -1;
    size = 0;
    owner = nil;
    currAddr = -1;

    const char* symName = GetName();
    Data dummy;
    if (symTab->Get(symName, dummy)) {
        symTab->Delete(symName);
    }
    pp = symFreeList;
    symFreeList = this;
    numSymInUse -= 1;
}
