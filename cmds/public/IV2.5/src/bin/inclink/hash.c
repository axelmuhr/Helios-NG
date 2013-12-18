/* 
 * Hash table manager 
 */

#include "errhandler.h"
#include "hash.h"
#include <stdio.h>
#include <string.h>

const int INITIALCHUNKSIZE = 1024 - 1;

Cell initialBlock[INITIALCHUNKSIZE];	/* initial Cell pool */
/* HashIter theIter(nil); */
/* HashIter *iterFreeList = &theIter; */

static StrTable *hashStrTable = nil;

/*
 * class CellManager 
 */
class CellManager {
    unsigned int chunksize;
    unsigned int nextFree;
    CellPointer freeBlock;
    CellPointer freeList;
public:
    CellManager ();
    ~CellManager ();
    CellPointer NewCell (const int, const int, const void*);
    void DisposeCell (CellPointer);
    CellPointer GetAChunk (const int);
};

static CellManager *cellPool = nil;

CellManager::CellManager () {
    chunksize = INITIALCHUNKSIZE;
    nextFree = INITIALCHUNKSIZE-1;
    freeBlock = &initialBlock[0];
    freeList = nil;
}

CellManager::~CellManager () { }

CellPointer CellManager::NewCell (
    const int v, const int sti, const void* stuff
) {
    CellPointer t;
       
    if (freeList != nil) {
	t = freeList;
	freeList = freeList->next;
    } else {
	if (nextFree == -1) {
	    freeBlock = GetAChunk(chunksize);
	}
	t = &freeBlock[nextFree];
	nextFree--;
    }
    t->val = v;
    t->stidx = sti;
    t->thang = stuff;
    return t;
}

void CellManager::DisposeCell (CellPointer p) {
    if (p->next != nil) {
	Warning("DisposeCell : Cell may still contain data");
    } else {
	p->next = freeList;
	freeList = p;
    }
}

CellPointer CellManager::GetAChunk (const int cells) {
    CellPointer t;
    
    t = new Cell[cells];
    nextFree = cells-1;
    return t;
}

void HashTable::SetHashFn (HashFn f) {
    if (f != nil) {
	hashFn = f;
    }
}

StrTable *GetStrTable () {
    if (hashStrTable == nil) {
	hashStrTable = new StrTable();
    }
    return hashStrTable;
}

void GetCellManager () {
    if (cellPool == nil) {
	cellPool = new CellManager();
    }
}

HashTable::HashTable () {
    unsigned int i;
    
    GetCellManager();
    tsize = DEFAULTTABLESIZE;
    extend = (tsize << 1) - 1;
    table = new CellPointer[DEFAULTTABLESIZE];	/* the hash table */

    for (i=0; i<tsize; i++) {
	table[i] = nil;
    }
    sTable = GetStrTable();
    for (i=0; i<=HARDCOLL; i++) {
	 stat[i] = 0;
    }
    duplicateKeysOK = true;
    hashFn = &HashString;
}

HashTable::HashTable (const int size, StrTable *t, boolean dupKeysOK) {
    unsigned int i;
    
    GetCellManager();
    tsize = size;
    ValidateTableSize();
    table = new CellPointer[tsize];	/* array of pointers to cells */
    for (i=0; i<tsize; i++) {
	table[i] = nil;
    }
    extend = (tsize * 3) >> 1;		/* extend table at 3*tsize/2 */
    if (t == nil) {
	sTable = GetStrTable();
    } else {
	sTable = t;
    }
    for (i=0; i<=HARDCOLL; i++) {
	 stat[i] = 0;
    }
    duplicateKeysOK = dupKeysOK;
    hashFn = &FastSimpleHash;
}

HashTable::~HashTable () {
    unsigned int i;
    Cell *p, *next;
    					/* add cells to freeList */
    for (i=0; i<tsize; i++) {
	p = table[i];
	while (p != nil) {
	    next = p->next;
	    p->next = nil;		/* DisposeCell expects this */
	    cellPool->DisposeCell(p);
	    p = next;
	}
    }
}

/* 
 * keep table size a power of 2 
 */
void HashTable::ValidateTableSize () {
    int i, log;

    i = 1 << 5;			/* min table size - 32 */
    log = 5;
    while (i < tsize) {
	i += i;
	log++;
    }
    tsize = i;
    sizeMask = tsize-1;
    logTSize = log;
}

/* 
 * find first occurence of object with string s 
 */
boolean HashTable::Find (const char* s, unsigned int &v, CellPointer &p) {
    register boolean found;
    unsigned int tableIdx;
    
    stat[FIND]++;
    v = (*hashFn)(s);
    tableIdx = KeyToBucket(v);
    p = table[tableIdx];
    found = false;
    while (p != nil && !found) {
	if (p->val == v) {
	     if ( strcmp(GetKey(p), s)==0 ) {
		found = true;
	     } else {			/* should test if inserting */
		stat[HARDCOLL]++;
		p = p->next;
	     }
	} else {
	    stat[COLL]++;
	    p = p->next;
	}
    }
    return found;
}

/* 
 * Find object with string s, and data stuff 
 * return 0 = no match
 *        1 = name matches, but not data
 * 	  2 = exact match.
 */
MatchType HashTable::PartialMatch (
    const char* s, Data dd, unsigned int &v, CellPointer &p
) {
    unsigned int degreeMatch, vt;
    CellPointer sameName;
    
    degreeMatch = NOMATCH;
    v = (*hashFn)(s);
    vt = KeyToBucket(v);
    p = table[vt];
    while (p != nil && degreeMatch != EXACT) {
	if (p->val == v && strcmp(GetKey(p),s) == 0) {
	    if (p->thang == dd) {
		degreeMatch = EXACT;
	    } else {
		degreeMatch = NAMEONLY;
		sameName = p;
		stat[HARDCOLL]++;
		p = p->next;
	    }		
	} else {
	    stat[COLL]++;
	    p = p->next;
	}
    }
    if (degreeMatch == NAMEONLY) {
	p = sameName;
    }
    return degreeMatch;
}

/* 
 * Extend the hash table - double it's size, sort of.
 */
void HashTable::ExtendTable () {
    CellPointer cp, cpnext;
    CellPointer *newtable;
    unsigned int oldsize, v, i;
    
    oldsize = tsize;
    tsize += tsize;
    extend += extend;
    newtable = new CellPointer[tsize];

    for (i=0; i<tsize; i++ ) {
	newtable[i] = nil;
    }
    for (i=0; i<oldsize; i++) {
	cp = table[i];
	while (cp != nil) {
	    cpnext = cp->next;
	    v = KeyToBucket(cp->val);
	    cp->next = newtable[v];
	    newtable[v] = cp;
	    cp = cpnext;
	}
    }
    delete [oldsize] table;
    table = newtable;
}

/* 
 * Only check if exact match.  Don't insert if so.
 * Insert duplicate entry only if duplicate keys are allowed.
 */
CellPointer HashTable::Insert (const char* s, const void* stuff) {
    unsigned int v, vt, ii, match;
    CellPointer cp;
    
    stat[INS]++;
    match = PartialMatch(s,stuff,v,cp);
    if (match == NAMEONLY && ! duplicateKeysOK) {
	Warning("HashTable::Insert- item has identical key to existing item");
	return nil;
    }
    if (match != EXACT) {
	stat[ELE]++;
	if (stat[ELE] > extend) {
	    ExtendTable();
	}
	vt = KeyToBucket(v);
	if (match == NAMEONLY) {
	    ii = cp->stidx;
	} else {
	    ii = sTable->AddString(s);
	}
	cp = cellPool->NewCell(v,ii,stuff);
	cp->next = table[vt];
	table[vt] = cp;
    }
    return cp;
}

/* 
 * Same as insert but takes a string table index instead of string
 * For objects with the name already in the string table 
 */
CellPointer HashTable::Ins2 (STindex i, const void* stuff) {
    unsigned int v, vt;
    CellPointer cp;
    register const char* s;
    MatchType match;

    stat[INS]++;
    s = sTable->IdxToStr(i);
    match = PartialMatch(s,stuff,v,cp);
    if (match == NAMEONLY && ! duplicateKeysOK) {
	Warning("HashTable::Ins2: two items with the same key not allowed");
	return nil;
    }
    if (match != EXACT) {
	stat[ELE]++;
	if (stat[ELE] > extend) {
	    ExtendTable();
	}
	v = (*hashFn)(s);
	vt = KeyToBucket(v);
	cp = cellPool->NewCell(v,i,stuff);
	cp->next = table[vt];
	table[vt] = cp;
    }
    return cp;
}

/* 
 * Insert an object from another hash table 
 * The two must share the same hash table and hashing algorithm.
 */
CellPointer HashTable::InsertCopy (HashTable *src, CellPointer cp) {
    CellPointer dup;
    unsigned int vt;

    if (cp == nil) {
	return nil;
    }
    if (src == this) {
	Warning("HashTable::InsertCopy : source == dest hash table");
	return cp;
    }
    if (src->sTable != this->sTable) {
	Warning("HashTable::InsertCopy : string tables don't match");
	return nil;
    }
    stat[INS]++;
    stat[ELE]++;
    dup = cellPool->NewCell(cp->val, cp->stidx, cp->thang);
    vt = KeyToBucket(cp->val);
    dup->next = table[vt];
    table[vt] = dup;
    return dup;
}

/* 
 * Modify entry with name str 
 * if multiple entries will surely now work.
 */
int HashTable::Modify (const char* str, const void* stuff) {
    unsigned int v;
    int status;
    CellPointer cp;
    
    if (Find(str,v,cp)) {
	cp->thang = stuff;
	status = 0;
    } else {
	Warning("Modify : string doesn't exist, use Insert");
	status = -1;
    }
    return status;
}

void HashTable::ModifyInt (const char* s, int i) {
    int val;
    CellPointer cp;

    if (Find(s, val, cp)) {
	cp->thang = (Data) i;
    } else {
	Warning("ModifyInt : string %s not found", s);
    }
}
    

/* 
 * Delete given CellPointer (internal use only).
 */
int HashTable::DeleteInternal (unsigned int v, CellPointer cp) {
    unsigned int vt;
    CellPointer dopey;

    --stat[ELE];
    vt = KeyToBucket(v);
    dopey = table[vt];
    if (dopey == cp) {
	table[vt] = cp->next;
    } else {
	while (dopey != nil && dopey->next != cp) {
	    dopey = dopey->next;
	}
	if (dopey != nil) {
	    dopey->next = cp->next;
	} else {
	    return -1;
	}
    }
    cp->next = nil;
    cellPool->DisposeCell(cp);
    return 0;
}

/* 
 * Delete entry with the given name, 
 * If multiple entries deletes first entry.
 */
int HashTable::Delete (const char* str) {
    unsigned int v;
    int status;
    CellPointer cp;
    boolean found;
    
    stat[DEL]++;
    found = Find(str,v,cp);
    if (! found) {
	Warning("Delete : object not in hashtable");
	status = -1;
    } else {
	DeleteInternal(v,cp);
	status = 0;
    }
    return status;
}

/* 
 * Delete entry CellPointer 
 */
int HashTable::Delete (CellPointer cp) {
    stat[DEL]++;
    return DeleteInternal(cp->val,cp);
}

CellPointer lastGet = nil;		/* non nil if last get succeeded */
const char* lastKey = nil;

boolean HashTable::Get (const char* str, Data& stuff) {
    unsigned int v;
    CellPointer cp;
    boolean found;
    
    stat[GET]++;
    found = Find(str,v,cp);
    if (found) {
	stuff = cp->thang;
	lastGet = cp;
	lastKey = GetKey(cp);
    } else {
	lastGet = nil;
	lastKey = nil;
    }
    return found;
}

boolean HashTable::GetNext (Data& stuff) {
    register CellPointer cp;
    register unsigned int hashval;
    register boolean found;

    found = false;
    if (lastGet == nil) {
	Warning("GetNext : previous Get over or not successful");
    } else {
	hashval = lastGet->val;
	cp = lastGet->next;
	while (cp != nil && !found) {
	    if (cp->val == hashval) {
		if (cp->stidx == lastGet->stidx || 
		 strcmp(GetKey(cp), GetKey(lastGet)) == 0
		) {
		    lastGet = cp;
		    lastKey = GetKey(cp);
		    stuff = cp->thang;
		    found = true;
		} else {
		    cp = cp->next;
		}
	    } else {
		cp = cp->next;
	    }
	}
    }
    if (!found) {
	lastKey = nil;
	lastGet = nil;
    }
    return found;
}

void HashTable::Apply (voidFn fn) {
    int i;
    CellPointer cp;
    Data d;

    for (i=0; i<tsize; i++) {
	cp = table[i];
	while (cp != nil) {
	    d = cp->thang;		/* protect cp from passing by ref */
	    lastKey = GetKey(cp);
	    fn(d);
	    cp = cp->next;
	}
    }
    lastKey = nil;
}

void HashTable::DumpStats () {
    int i, j, maxChain, totalLinks;
    int chainLen[100];
    CellPointer cp;
    
    printf("  Table size = %d, strtable = %d, elements = %d\n", 
	tsize, sTable->StrToIdx(nil), stat[ELE]);
    printf(
      "  Insertions = %d, Deletions = %d, Gets = %d, (internal) finds = %d\n",
	stat[INS], stat[DEL], stat[GET], stat[FIND]
    );
    printf(
      "  Chain links followed = %d, hard collisions (same hash value) = %d\n",
	stat[COLL], stat[HARDCOLL]
    );
    for (i=0; i<100; i++) {
	chainLen[i] = 0;
    }
    maxChain = 0;
    totalLinks = 0;
    for (i=0; i<tsize; i++) {
	j = 0;
	cp = table[i];
	while (cp != nil) {
	    j++;
	    cp = cp->next;
	}
	if (j > 16) {
	    fprintf(stderr, "  Hash table bucket %d has a chain %d long\n",
		i, j);
	}
	if (j > 100) {
	    j = 100;
	}
	totalLinks += j*j;
	chainLen[j]++;
	if (j > maxChain) {
	    maxChain = j;
	}
    }
    printf("  Chain Length distribution\n  ");
    for (i=0; i<10; i++) {
	printf("%6d",i);
    }
    for (i=0; i<= maxChain; i++) {
	if ((i % 10) == 0) {
	    printf("\n  ");
	}
	printf("%6d",chainLen[i]);
    }
    printf("\n  Sum of links squared : %d\n",totalLinks);
    printf("\n");
}

HashIter::HashIter (const HashTable* h) {
    index = -1;
    cursor = nil;
    ht = h;
}

boolean HashIter::MoreEntries (Data &p) {
    if (cursor != nil) {
	cursor = cursor->next;
    }
    while (cursor == nil) {
	++index;
	if (index == ht->tsize) {
	    return false;
	}
	cursor = ht->table[index];
    }
    p = cursor->thang;
    return true;
}

/* 
 * Algorithm based on Aho, Sethi, Ullman Dragon Book. (pg 436).
 * Guarantees all strings <= shorter than 4 chars hash uniquely.
 */
unsigned int HashString (const char* str) {
    register unsigned int v;
    register unsigned int upper7;
    register const char* p;
    
    p = str;
    v = 0;
    while (*p != EOL) {
	v <<= 7;
	v += *p;			/* v = (v<<7) + *p */
	upper7 = v & 0xfe000000;	/* take upper 7 bits of v */
	if (upper7 != 0) {
	    v ^= (upper7 >> 20);	/* XOR into ALMOST lowest bits. */
	}
	p++;
    }
    return v;
}

unsigned int FastSimpleHash (const char* str) {
    register const char* p;
    register unsigned int v;
    register unsigned int t;
    
    p = str;
    v = 0;
    while (*p != EOL) {
	v += v;
	v ^= *p;
	p++;
    }
    t = v >> 10;
    t ^= t >> 10;
    v ^= t;
    return v;
}
