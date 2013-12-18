#ifndef loc_h
#define loc_h

#include "base.h"

static const int INVALIDVAL = -87654321;

class Location : public Base {
    Chunk* home;
    Location* cnext;
    Location* cprev;
    int address;
    int oldValue;		/* INVALIDVAL == not relocated */
    char extref;
    char pcrel;
    char indirect;		/* for indirect calls, future work */
    char length;		/* in bytes */

    void IntraReloc(int destFinal, int destDelta);
    void ExtReloc (int destFinal) {
	if (IsNewLoc()) {
	    NewExtReloc(destFinal);
	} else {
	    OldExtReloc(destFinal);
	}
    }
    void OldIntraReloc(int destFinal, int destDelta);
    void OldExtReloc(int destFinal);
    void NewIntraReloc(int destFinal, int destDelta);
    void NewExtReloc(int destFinal);

    friend void DoLoc(RelocInfo *, Module*, Chunk*, Symbol* [], int);
    friend Location* AllocLoc(RelocInfo *);
    friend class Symbol;
    friend class Chunk;
    friend class Ctdt;

public:
    Location (const RelocInfo* ri);

    void LocInit (const RelocInfo* ri) {
	extref = ri->r_extern;
	pcrel = ri->r_pcrel;
	length = BYTESIZE << (ri->r_length);
	address = ri->r_address;
	indirect = false;
	oldValue = INVALIDVAL;
    }
    void LinkLoc(Chunk*);
    void UnlinkLoc(Chunk*);
    void Reloc (int destFinal, int delta) {
	if (extref) {
	    if (IsNewLoc()) {
		NewExtReloc(destFinal);
	    } else {
		OldExtReloc(destFinal);
	    }
	} else {
	    if (IsNewLoc()) {
		NewIntraReloc(destFinal, delta);
	    } else {
		OldIntraReloc(destFinal, delta);
	    }
	}
    }
    void Dump(DumpLevel);
    void DisposeLoc();
    boolean IsNewLoc () { return (oldValue == INVALIDVAL); }
    void RelocLocalRef();
};

extern void DoLoc(RelocInfo *, Module*, Chunk*, Symbol* [], int =1);
extern Location* AllocLoc(RelocInfo *);
extern int NumLocs();

#endif
