#ifndef symbols_h
#define symbols_h

#include "base.h"
#include "aout.h"

/* 
 * the final location of a (text) symbol is 
 * 	prog.a_text + mod.TextAddr() + sym.offset;
 * similarly for data and bss symbols.
 * This slows down full linking marginally but speeds up 
 * incremental linking.
 * A symbol can be on one of several lists :
 * 	a chunk list if it is defined.
 * 	the anon list if it appears to be a global bss element
 * 	the undefined list if we don't know jack about it. 
 */

class Symbol : public BaseName {
    friend Symbol* GetSym (const char*, Module*);

public:
    int	offset;				/* offset from start of chunk */
    int currAddr;			/* final address */
    int size;				/* for data and bss only */
    short type;				/* same as ld format */
    char global;
    char addrChanged;
    
    Symbol () : (SYMBOL) {
	type = -1; size = -1; offset = 0; global = false; currAddr = -1; 
    }
    Symbol(NList *, Module*, Chunk*);
    Symbol (const char* name, int t, boolean glbl =false) : (SYMBOL, name) {
	type = t; size = 0; offset = 0; global = glbl; currAddr = -1;
    }

    void NewSymbol(NList *nl, Module* mod, Chunk* ch);
    boolean Defined () { return type != N_UNDF; }
    boolean IsUndef () { return (type == N_UNDF && global); }
    void Dump(DumpLevel);
    int WhereAmI();
    void SetAddr(int finalAddr, boolean setAddrChanged =true);
    int GetAddr();
    Symbol* Resolve(NList *, Module*, Chunk*);
    Symbol* UpdateAnon(NList *, Module*);
    void UpdateUndef(NList *, Module*, Chunk*);
    void SetAnonAddr(
	int &offIntoChunk, int anonStart, boolean incremental =false
    );
    void RelocSym(int destOrig);
    void RelocAnon();
    void UndefineSym();
    void Dispose();
};

extern Symbol** symArray;
extern Symbol* DoSym (NList *, Module*);
extern int NumSyms ();

#endif
