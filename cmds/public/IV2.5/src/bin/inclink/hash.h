#ifndef strhash_h
#define strhash_h

#include "defs.h"
#include "strtable.h"

static const int DEFAULTTABLESIZE = 1 << 9;	/* 512 */

class HashTable;
typedef Address Data;
typedef enum { FIND, INS, DEL, GET, ELE, COLL, HARDCOLL } Hashops;
typedef unsigned int HashVal;

struct Cell {
    unsigned int val;
    unsigned int stidx;
    Data thang;
    Cell *next;
};

typedef Cell* CellPointer;
typedef CellPointer* CellPointerArray;
typedef enum { NOMATCH, NAMEONLY, EXACT } MatchType;
typedef void (*voidFn)(Data);
typedef unsigned int (*HashFn)(const char*);
typedef boolean (*FindFn)(const char*, unsigned int&, CellPointer&);

class HashTable {
    unsigned int tsize;
    unsigned int sizeMask;
    unsigned int logTSize;
    int extend;
    CellPointer *table;
    CellPointer getNext;
    StrTable *sTable;
    int stat[HARDCOLL-FIND+1];
    boolean duplicateKeysOK;
    boolean keepCopyOfKey;
    HashFn hashFn;

    boolean Find(const char*, unsigned int&, CellPointer&);
    unsigned int KeyToBucket (unsigned int v) { return v & sizeMask; }
    void ValidateTableSize();
    MatchType PartialMatch(const char*, Data, unsigned int&, CellPointer&);
    int DeleteInternal(unsigned int val, CellPointer cp);
    int Modify(const char*, const void*);
    void ExtendTable();
    
    friend class HashIter;
public:
    HashTable ();				
    HashTable (const int, StrTable * =0, boolean dupKeysOK =true);
    ~HashTable ();
    void SetHashFn (HashFn f);

    CellPointer Insert(const char*, const void*);
    int Delete(const char*);
    boolean Get(const char*, Data&);
    boolean GetNext(Data&);

    STindex GetKeyIdx (CellPointer p) { return p->stidx; }
    const char* GetKey (CellPointer p) { return sTable->IdxToStr(p->stidx); }
    
    STindex StoreString (const char* s) { return sTable->AddString(s); }
    const char* GetString (STindex si) { return sTable->SafeIdxToStr(si); }

    void Apply(voidFn fn);		/* apply fn to every object */
    void DumpStats();			/* dump stats on the hash table */

    CellPointer Ins2(STindex, const void*);
    CellPointer InsertCopy(HashTable *src, CellPointer cp);
    int Delete(CellPointer);
    void ModifyCell (CellPointer c, int i) { c->thang = (Data)i; }
    void ModifyInt(const char*, int);
};

class HashIter {
    int index;
    CellPointer cursor;
    HashTable* ht;
public:
    HashIter(const HashTable*);
    boolean MoreEntries(Data&);
};

extern unsigned int HashString (const char*);
extern unsigned int FastSimpleHash (const char*);

#endif
