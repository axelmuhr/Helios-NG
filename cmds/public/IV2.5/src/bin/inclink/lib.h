/* 
 * Archive library information.
 */

#ifndef lib_h
#define lib_h

#include "types.h"
#include "base.h"
#include <ar.h>

struct ArHeader : public ar_hdr {
    void ReadMe(FILE* f);
};

struct Ranlib {
    int strOff;
    int objOff;
    
    const char* GetName (char* stringTable);
    void ReadIn(FILE* f);
};

class LibFile : public InputFile {
public:
    LibFile(const char*);
    ~LibFile();

    void Init();
    void Reread ();
    void Relink();
    void RemoveModule(Module*);
    void RelocLib (boolean incremental =false);

    void WriteText(FILE*);
    void WriteData(FILE*);
    void WriteSyms(FILE*);
    void WriteStrTab(FILE*);

    void IncRelocLib () { RelocLib(true); }
    boolean DefinesSym(Symbol*);

private:
    void RemoveModules();

    boolean IsRanLib();
    void ReadTableOfContents();
    void ReadModules();
    boolean RunThruLib();
    boolean ShouldLoad(Symbol*);

    void PreLoadLib(int off, Exec& e);

    int numRanLibs;	    /* number of modules in the library */
    Ranlib* ranlibs;	    /* table of contents */
    char* ranLibStrings;    /* string table used used by "ranlibs" */
    boolean* isProcessed;   /* true if corresponding sym has been processed */
    boolean removeAll;      /* true when ReadModules is on the stack */
};

#endif
