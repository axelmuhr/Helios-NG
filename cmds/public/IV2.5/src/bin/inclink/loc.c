#include "lib.h"
#include "loc.h"
#include "module.h"
#include "program.h"
#include "symbols.h"
#include "types.h"

Location* bssList = nil;

Location::Location (RelocInfo *ri) : (LOC) {
    Warning("calling Location constructor");
    LocInit(ri);
}

static int numLocInUse = 0;
static Location* locFreeList = nil;
static int locsInBuff = 0;
static Location* locBuff = nil;
static int locChunk = 93;  // new size == 4092

int NumLocs () { return numLocInUse; }

inline static void RefillLocBuff () {
    locBuff = (Location*) new char[ sizeof(Location)*locChunk ];
    locsInBuff = locChunk;
}

Location* AllocLoc (RelocInfo *ri) {
    register Location* l;
    
    if (locFreeList != nil) {
	l = locFreeList;
	locFreeList = l->cnext;
    } else {
	if (locsInBuff == 0) {
	    RefillLocBuff();
	}
	l = locBuff;
	locsInBuff -= 1;
	locBuff += 1;			/* pointer arithmetic */
    }
    l->LocInit(ri);
    numLocInUse += 1;			/* monotonically increasing */
    return l;
}

void Location::DisposeLoc () {
    BaseInit();
    extref = false;
    pcrel = false;
    length = -1;
    address = -1;
    oldValue = INVALIDVAL;
    indirect = false;
    cnext = locFreeList;
    locFreeList = this;
}

void Location::LinkLoc (Chunk* c) {
    if (c->relocList == nil) {
	c->relocList = this;
	cnext = this;
	cprev = this;
    } else {
	cprev = c->relocList->cprev;
	cnext = c->relocList;
	cprev->cnext = this;
	cnext->cprev = this;
    }
    home = c;
}

void DoLoc (RelocInfo r[], Module* m, Chunk* src, Symbol* symArray[], int n) {
    register int i;
    register RelocInfo *ri;
    register Location* l;
    Symbol* sym;
    Chunk* c;
    
    for (i = 0; i < n; i++) {
	ri = &r[i];
	l = AllocLoc(ri);
	if (ri->r_extern) {
	    sym = symArray[ri->r_symbolnum];
	} else {
	    c = m->DetermineChunk(ri->r_symbolnum & ~N_EXT);
	    sym = (Symbol*) c->inf;
	}
	sym->InsertInf(l);
	l->LinkLoc(src);
    }
}

void Location::RelocLocalRef () {
    register int final, delta;
    register Symbol* s;
    
    s = (Symbol*) owner;
    final = s->GetAddr();
    if (extref) {
	ExtReloc(final);
    } else {
	delta = ((Chunk*)(s->owner))->origStart;
	IntraReloc(final, delta);
    }
}

void Location::UnlinkLoc (Chunk*) {
    Base* symbolOwner = owner->owner;
    Base::Unlink(owner->inf);
    if (cnext == this) {		/* only loc on relocList */
	home->relocList = nil;
    } else {
	if (home->relocList == this) {	
	    home->relocList = cnext;	/* first loc on relocList */
	}
	cprev->cnext = cnext;
	cnext->cprev = cprev;
    }
    cnext = nil;
    cprev = nil;

    if (symbolOwner != currProg /* undefList */ &&
        symbolOwner != currProg->anonList &&
        symbolOwner != currProg->absList
    ) {
        Module* owningMod = (Module*) symbolOwner->owner;
        if (owningMod->PartOfLib() &&
            owningMod->NoRefs() &&
            owningMod != home->owner /* not recursive in same module */
        ) {
            ((LibFile*)owningMod->owner)->RemoveModule(owningMod);
        }
    }
}

void Location::IntraReloc (int destFinal, int destOrig) {
    if (IsNewLoc()) {
	NewIntraReloc(destFinal, destOrig);
    } else {
	OldIntraReloc(destFinal, destOrig);
    }
}

void Location::NewIntraReloc (int destFinal, int destOrig) {
    register int *lp;
    int hf, ho;	/* this should be long, but.. */
    register Symbol* sym = (Symbol*) owner;

    if (pcrel) {			/* know home is text. pcrel right? */
	if (sym->type == N_TEXT && sym->owner == home) {
	    return;
	} else {
	    lp = (int*) home->Addr(address);
	    oldValue = *lp;
	    hf = home->FinalAddr();
	    ho = home->origStart;
	    *lp += (destFinal - destOrig) - (hf - ho);
	    home->NeedToWrite();
	}
    } else {				/* not pcrel */
	lp = (int*) home->Addr(address);
	oldValue = *lp;
	*lp += (destFinal - destOrig);	/* destFinal = start of chunk
					 * since symbol is anonymous */
	home->NeedToWrite();
    }
}

void Location::OldIntraReloc (int destFinal, int destOrig) {
    register int* lp;
    register int hf, ho;	/* this should be long, but.. */
    Symbol* sym = (Symbol*) owner;

    if (pcrel) {			/* know home is text. pcrel right? */
	if (sym->type == N_TEXT && sym->owner == home) {
	    return;
	} else {
	    lp = (int *) home->Addr(address);
	    hf = home->FinalAddr();
	    ho = home->origStart;
	    *lp = oldValue + (destFinal - destOrig) - (hf - ho);
	    home->NeedToWrite();
	}
    } else {
	lp = (int *) home->Addr(address);
	*lp = oldValue + (destFinal - destOrig);
					/* destFinal = start of chunk
					 * since symbol is anonymous */
	home->NeedToWrite();					 
    }
}

void Location::NewExtReloc (int destFinal) {
    register int* lp;
    register int m;

    if (pcrel) {
	lp = (int*) home->Addr(address);
	oldValue = *lp;
	m = home->FinalAddr();
	*lp += destFinal - m;

	/* - (home->origStart) but this = 0,
	 * since pcrel implies text */
	if (home->origStart != 0) {
	    Panic("home->origStart != 0 in NewExtReloc");
	}
    } else {
	lp = (int*) home->Addr(address);
	oldValue = *lp;
	*lp += destFinal;
    }
    home->NeedToWrite();
}

void Location::OldExtReloc (int destFinal) {
    register int* lp;
    register int m;

    lp = (int*) home->Addr(address);
    if (pcrel) {
	m = home->FinalAddr();
	*lp = oldValue + destFinal - m; 

	/* - (home->origStart) but this = 0,
	 * since pcrel implies text */
	if (home->origStart != 0) {
	    Panic("home->origStart != 0 in ExtReloc");
	}
    } else {
	*lp = oldValue + destFinal;
    }
    home->NeedToWrite();
}

void Location::Dump (DumpLevel) {
    fprintf(dumpFile,"Addr=%05d(0x%04x) ",address,address);
    if (extref) {
	fprintf(dumpFile,"Ext ref ");
    }
    if (pcrel) {
	fprintf(dumpFile,"PC Rel ");
    }
    if (indirect) {
	fprintf(dumpFile,"Indirect ");
    }
    if (length == 0) {
	fprintf(dumpFile,"Len=%d bits",length<<3);
    }
    if (extref == false) {
	fprintf(dumpFile,"---");
    } else {
	fprintf(dumpFile,"Sym %s",((Symbol*)owner)->GetName());
    }
    fprintf(dumpFile,"\n");
}
