/* 
 * Module for handling chunks - portions of {text,data,bss} segments
 * defined in a module.
 * Segments are made up of chunks 
 */

#include "chunk.h"
#include "errhandler.h"
#include "symbols.h"
#include "program.h"
#include "loc.h"
#include "system.h"

static int maxChunk = 0;

/* 
 * Allocator for segments.
 */
int AllocSegment (int actual) {
    if (actual == 0) {
	return 0;
    } else if (!currProg->addSlop) {
	maxChunk = max(maxChunk, actual);
	return actual;
    } else {
        int powerOf2 = 128;
        while (actual > powerOf2 - sizeof(int)) {
            powerOf2 += powerOf2;
        }
        powerOf2 -= sizeof(int);
	maxChunk = max(maxChunk, powerOf2);
	return powerOf2;
    }
}

Chunk::Chunk (int origStart, int size, SegmentType st, const char*) : (CHUNK) {
    if (size < 0) {
	Panic("Chunk(size %d)", size);
    }
    this->origStart = origStart;
    this->start = -1;			/* on initial assign, set dirty */
    this->size = size;
    this->alloc = AllocSegment(size);
    this->relocList = nil;
    type = st;
    buff = nil;
    NeedToReloc(false);
    /*
     * Create dummy symbol for the chunk.
     * Local relocation info bound to text or data is attached here.
     */
    InsertInf(new Symbol(nil, SegToNType(st)));
}

Chunk::~Chunk () {
    if (buff) {
        delete buff;
    }
}

int Chunk::SetAddrs (int offset) {
    if (size > alloc) {
	alloc = AllocSegment(size);
    }
    if (start != offset) {
	start = offset;
	NeedToReloc(true);
    }
    return alloc;
}

/* 
 * need to relocate and need to write this chunk 
 */
void Chunk::NeedToReloc (boolean dirt) {
    if (dirt) {
	dirty = NEEDTORELOC | NEEDTOWRITE;
    } else {
	dirty = 0;
    }
}

void Chunk::FillBuff (FILE* f) {
    int zeroOut;
    
    if (alloc > 0) {
	if (buff == nil) {
	    buff = new char[alloc];
	}
	ReadBuff(f, buff, size);
	zeroOut = alloc - size;
	if (zeroOut != 0) {
	    bzero(&buff[size], zeroOut);
	}
	NeedToReloc();
    }
}

void Chunk::WriteBuff (FILE* f) {
    if (dirty /* & NEEDTOWRITE */) {
	if (alloc > 0) {
	    long offset = currProg->SegPos(type) + start;
            if (offset != ftell(f)) {
                fseek(f, offset, 0);
            }
	    ::WriteBuff(f, buff, alloc);		
            if (Debug.StartMessage()) {
		BaseName* bn = (BaseName*)owner;
		Debug.Add("wrote ");
		Debug.Add(SegTypeToStr(type));
		Debug.Add(" chunk ");
		Debug.Add(bn->GetName());
		Debug.Add(" at ");
		Debug.Add((int)offset);
		Debug.EndMessage();
 	    }
	}
	NeedToReloc(false);
    }
}

void Chunk::RelocChunk () {
    register Location* l;
    register Location* end;
    
    if (dirty & NEEDTORELOC) {
        if (Debug.StartMessage()) {
            Debug.Add("Relocating ");
            BaseName* bn = (BaseName*)owner;
            Debug.Add(" ");
            Debug.Add(bn->GetName());
            Debug.Add(" ");
            Debug.Add(SegTypeToStr(type));
            Debug.EndMessage();
        }
	l = (Location*) relocList;	/* relocate refs FROM this chunk */
	end = l;
	if (l != nil) {
	    do {
		l->RelocLocalRef();
		l = (Location*) l->cnext;
	    } while (l != end);
	}
    }
}

void Chunk::RelocINRefs () {
    register Symbol* s;

    s = (Symbol*) inf;
    if (s != nil) {			/* relocate refs TO this chunk */
	do {
	    s->RelocSym(origStart);
	    s = (Symbol*) s->nn;
	} while (s != inf);
    }
}

int Chunk::SetAnonListAddr (int anonSegStart) {
    Symbol* s;

    s = (Symbol*) inf;
    do {
	s->SetAnonAddr(alloc, anonSegStart, !currProg->fullLink);
	s = (Symbol*) s->nn;
    } while (s != inf);
    return alloc;
}

boolean Chunk::SetInfSymAddrs () {
    Symbol* s;
    s = (Symbol*) inf;
    if ((dirty & NEEDTORELOC) && s != nil) {
	do {
	    s->WhereAmI();
	    s = (Symbol*) s->nn;
	} while (s != inf);
    }
    return dirty;
}

void Chunk::InvalidateChunk () {
    Location* l;

    l = relocList;
    while (l != nil) {
	l->UnlinkLoc(this);
	l->DisposeLoc();
	l = relocList;
    }
    if (inf == nil) {
	Panic("InvalidateChunk : relocList == nil, not supposed to happen");
    }
    while (inf->nn != inf) {
	((Symbol*) inf->nn)->UndefineSym();
    }
    NeedToReloc();
}

void Chunk::UpdateChunk (int newSize, int &segAlloc, int origOffset) {
    if (newSize > alloc) {
	delete [alloc] buff;
	alloc = AllocSegment(newSize);
	buff = new char[alloc];
	start = segAlloc;
	segAlloc += alloc;
	if (Debug.StartMessage()) {
	    Debug.Add(SegTypeToStr(type));
	    Debug.Add(" chunk ");
	    Debug.Add(((BaseName*)owner)->GetName());
	    Debug.Add(" size change ");
	    Debug.Add("old=");
	    Debug.Add(size);
	    Debug.Add(" vs ");
	    Debug.Add("new=");
	    Debug.Add(newSize);
	    Debug.EndMessage();
	}
    }
    size = newSize;
    origStart = origOffset;
}

void Chunk::NullAnonChunk () {
    alloc = 0;
    size = 0;
    origStart = 0;
    start = 0;
}

void Chunk::IncRelocChunk () {
    Location* l;
    
    if (dirty & NEEDTORELOC) {
        if (Debug.StartMessage()) {
            Debug.Add("Relocating refs from ");
            BaseName* bn = (BaseName*)owner;
            Debug.Add(" ");
            Debug.Add(bn->GetName());
            Debug.Add(" ");
            Debug.Add(SegTypeToStr(type));
            Debug.EndMessage();
        }
	l = (Location*) relocList;	/* relocate refs FROM this chunk */
	if (l != nil) {
	    do {
		l->RelocLocalRef();
		l = (Location*) l->cnext;
	    } while (l != relocList);
	}
	RelocINRefs();
    }    
}
