/* Interface to class Program
 */

#ifndef program_h
#define program_h

#include "base.h"

class Ctdt;
class Exec;
class LibFile;

class Program : public BaseName {
    friend class Module;

public:
    Program(const char*);

    void Reread(const char*);
    void PrepareLink(boolean);
    void LinkFile(const char*);
    void DoLink();
    void FullLink();
    void Relink();

    void LoadLib(FILE*, const char*, Exec&, int, NList*, LibFile*);

    void AddUndef(Symbol*);
    void DeleteUndef(Symbol*);

    int SegStart (SegmentType st) { return segStart[st]; }
    int SegPos (SegmentType st) { return segPos[st]; }

    void SelectCtdt(const char*);
    Exec* Header () { return &header; }

    void SetsFlag(boolean);
    void SetxFlag(boolean);
    void SetXFlag(boolean);

    boolean fullLink;                   /* fulllink or relink */
    boolean addSlop;                    /* add slop space between chunks */
    boolean tinclink;                   /* use tinclink, don't stat modules */
    boolean s_flag;                     /* strip symbols */
    boolean x_flag;                     /* strip locals */
    boolean X_flag;                     /* strip labels */
    boolean k_flag;                     /* keep ctdt flag */

    Base* undefList;			/* list of undefined symbols */
    Chunk* anonList;			/* list of bss guys */
    Chunk* absList;			/* take care of abs refs */

private:
    Symbol* AddAbsList(const char*, int);

    InputFile* AddFile(const char* name);
    InputFile* AddModule(const char* name, FILE* f);
    InputFile* AddLibrary(const char* unitName, const char* libName);

    void UpdateFiles();
    void FinalizeAddrs();
    void FinalizeSegments();
    void UpdateSegment(int& segStart, int newStart, int segType);
    void MarkSegment(int segType, boolean dirty);
    void SetChunkAddrs();
    void SetAnonAddrs();
    void SetESymAddrs();
    void SetSymAddrs();

    boolean Relocate(boolean incremental =false);
    boolean IncReloc () { return Relocate(true); }
    boolean CheckForUndefs(boolean);
    void RereadLibs();

    void FinalizeStab();
    void SetSymTabAddr();
    void SetStrTabAddr();
    void ShiftSymTab();
    void ShiftStrTab();
    void GenGlobals();

    boolean WriteObjFile();
    FILE* OpenObjFile(const char*, boolean&);
    void ForceRewrite();
    void IncWriteObjFile() { WriteObjFile(); }
    void Pad(FILE*, int pos);
    void WriteTextSeg(FILE*);
    void WriteDataSeg(FILE*);
    void WriteSymbols(FILE*);
    void WriteLocals(FILE*);
    void WriteGlobals(FILE*);
    void WriteStrTab(FILE*);

    Exec header;
    int textStart, dataStart, bssStart, anonStart;
    int textAlloc, dataAlloc, bssAlloc, anonAlloc, symAlloc, strAlloc;
    int segStart[ERROR_SEG];		/* program addresses */
    int segPos[ERROR_SEG];		/* file addresses */

    InputFile* oldModList;
    Ctdt* ctdt;
    SymTab* globSyms;
};

extern Program* currProg;

#endif
