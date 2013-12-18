/* Interface to class Module
 */

#ifndef module_h
#define module_h

#include "base.h"
#include "chunk.h"
#include "system.h"

class Exec;
class Nlist;

class Module : public InputFile {
    friend class Location;
    friend class Program;
    friend class Symbol;
    friend class SymTab;

public:
    Module(const char*);  // used exclusively by Ctdt::Ctdt
    Module(
        const char*, Exec *, FILE* f,
        int fileOffset =0, Base* lib =nil
    );
    ~Module();

    void RelocMod(boolean incremental =false);
    void SetSymAddrs();
    Chunk* DetermineChunk(int t);
    void InternalizeRelocs(Chunk* ch, int numRelocs);
    void ReadSymName(int, char*);
    void Read3Parts(Exec);

    void WriteText(FILE*);
    void WriteData(FILE*);
    void WriteSyms(FILE*);
    void WriteStrTab(FILE*);
   
    void Reread();
    boolean PartOfLib () { return foffset != 0 && libOwner != nil; }
    boolean Defines(const char*);
    boolean NoRefs();
    void IncRelocMod();

protected:
    Chunk* text;
    Chunk* data;
    Chunk* bss;
    SymTab* symtab;
    StrTable* outStrtab;

private:
    Base* libOwner;     /* nil if not a library module */
    int foffset;	/* offset; 0 ==> mod; else ==> lib */

    int ReadSymbols(Exec);
    void AddLocal(NList*);
    int ReadRelocs(Exec);
    void ReadStrTable(Exec);
    void UpdateChunkAlloc(Exec*);
    void OpenFile();
};

extern void ResizeSymArray(int);

#endif
