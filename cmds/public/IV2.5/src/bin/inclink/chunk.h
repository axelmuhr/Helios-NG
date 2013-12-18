#ifndef chunk_h
#define chunk_h

#include "base.h"
#include "program.h"			/* for currProg */

static const int NEEDTOWRITE = 1 ;
static const int NEEDTORELOC = 1 << 1;

class Chunk : public Base {
    char* buff;
    int origStart;			/* orig offset within file */
    int start;				/* offset within a segment */
    int size;				/* actual size of this chunk */
    int alloc;  			/* allocated space for this chunk */
    Location* relocList;		/* pointer to the head */
    
    friend class Module;
    friend class Location;
    friend class Program;
    friend class Symbol;
    friend class Ctdt;

public:
    short type;
    short dirty;

    Chunk (int orig, int size, SegmentType st, const char* modName);
    ~Chunk();

    char *Addr (int i =0) { return &buff[i]; }
    int Offset (int i =0) { return (i + start); }
    int Move () { return (start - origStart); }
    int FinalAddr () { return (currProg->SegStart(type) + start); }
    int SetAddrs (int offset);
    void FillBuff (FILE*);
    void WriteBuff (FILE*);
    void LinkLoc (Location*);
    void RelocINRefs ();
    void RelocChunk ();
    int SetAnonListAddr (int anonSegStart);
    boolean SetInfSymAddrs ();
    
    void InvalidateChunk();
    void UpdateChunk(int newSize, int& segAlloc, int origOffset);
    void NullAnonChunk();
    void NeedToReloc(boolean dirty =true);
    void NeedToWrite () { dirty |= NEEDTOWRITE; }
    int IsDirty () { return (int) dirty; }
    void IncRelocChunk();
};

#endif
