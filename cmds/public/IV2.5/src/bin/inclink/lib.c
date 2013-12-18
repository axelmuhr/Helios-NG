/*
 * Routines to deal with library files in the ar(1) format.
 * We require that the library has been run through ranlib(1). 
 */

#include "errhandler.h"
#include "lib.h"
#include "symbols.h"
#include "module.h"
#include "program.h"
#include "types.h"
#include "system.h"
#include <string.h>

static const char* ranlibTitle = "__.SYMDEF";
static const int ranlibTitleLen = 9;		/* strlen(ranlibTitle) */
static char* currModST;

void ArHeader::ReadMe (FILE* f) {
    ReadBuff(f, this, sizeof(ArHeader), "ArHeader.ReadMe : failed", ftell(f));
}

const char* Ranlib::GetName (char* stringTable)  {
    if (strOff == -1) {
	fprintf(stderr, "strOff = -1\n");
	fflush(stderr);
    }
    return &stringTable[strOff];
}

LibFile::LibFile (const char* name)
: (LIBRARY, name, file = fopen(name, "r"))
{
    if (file == nil) {
	Panic("can't open library %s", name);
    }
    removeAll = false;
}

void LibFile::Init () {
    if (IsRanLib()) {
	ReadTableOfContents();
	isProcessed = new boolean[numRanLibs];
        bzero(isProcessed, sizeof(isProcessed[0]) * numRanLibs);
	ReadModules();
    }
    CloseFile();
}

LibFile::~LibFile() {
    RemoveModules();
    delete ranlibs;
    delete ranLibStrings;
    delete isProcessed;
}

/* Return true if the file is in ranlib format.
 * Currently, the program bombs if the file is in the wrong format.
 */
boolean LibFile::IsRanLib () {
    ArHeader ah;
    char arMag[arMagicLen];

    ReadBuff(file, &arMag[0], sizeof(arMag));
    if (strncmp(arMag, arMagicString, sizeof(arMag)) != 0) {
	Panic("library %s is not in ar(1) format.", GetName());
    }
    ReadBuff(file, &ah, sizeof(ah));
    if (strncmp(ah.ar_name, ranlibTitle, ranlibTitleLen) != 0) {
	Panic("Library has not been run through ranlib(1).  Do so");
    }
    return true;
}

static const int ABSOLUTE = 0;
static const int CURROFFSET = 1;

ArHeader ar;

/* Already found that ar_hdr.ar_name == "_.SYMDEF"
 * 
 * Random library format:
 *	!<arch>
 *	archive0
 *	archive1
 *	    ...
 *	archiveN
 *
 * where archive0 has the following format:
 *	ar_hdr.ar_name == "_.SYMDEF"
 *	    ...		remaining fields for this ar_hdr
 *	int		total size of table of contents
 *	RanLib.stroff	pointer into string table for table of contents
 *	RanLib.objoff	pointer to library file
 *	    ...		other RanLib pairs
 *	int		size of string table
 *	char[]		string table for table of contents
 *
 * archive 1 -- N has the following format:
 *	ar_hdr
 *	a.out
 *
 * Read table of contents, which corresponds to archive0
 */
void LibFile::ReadTableOfContents () {
    int sizeofStringTable;
    int sizeofTableOfContents;

    ReadBuff(file, &sizeofTableOfContents, sizeof(sizeofTableOfContents));

    numRanLibs = sizeofTableOfContents / sizeof(Ranlib);
    ranlibs = new Ranlib[numRanLibs];
    ReadBuff(file, ranlibs, sizeof(Ranlib) * numRanLibs );

    ReadBuff(file, &sizeofStringTable, sizeof(sizeofStringTable));
    ranLibStrings = new char[sizeofStringTable];
    ReadBuff(file, ranLibStrings, sizeofStringTable);
}

void LibFile::ReadModules () {
    boolean changed;

    changed = true;
    while (changed && currProg->undefList != nil) {
	changed = RunThruLib();
    }
}

static const int nlSize = 4096;
NList nlArr[nlSize];			/* for holding NLists from lib */
int numSyms;

boolean LibFile::RunThruLib () {
    char moduleName[32];
    int i, j, off, justLoaded, preLoaded;
    Exec e;
    boolean didSomething;
    Ranlib *r;
    Symbol* sym;

    didSomething = false;
    justLoaded = -1;
    preLoaded = -1;
    for (i=0; i<numRanLibs; i++) {
	r = &ranlibs[i];
	if (isProcessed[i] == false) {
	    if (r->objOff == justLoaded) {
		isProcessed[i] = true;	/* just loaded, so all defined */
	    } else {
		if (symTab->Get(r->GetName(ranLibStrings), sym) == true) {
		    if (sym->IsUndef()) {
			off = r->objOff + sizeof(ar);
			if (preLoaded != r->objOff) {
			    fseek(file, r->objOff, ABSOLUTE);
			    ReadBuff(file, &ar, sizeof(ar));
			    for (j=0; j<16 && (ar.ar_name[j] != ' '); j++) {
				moduleName[j] = ar.ar_name[j];
			    }
			    moduleName[j] = EOL;
			    Debug("loading %s", moduleName);
			    Debug("(for %s)", r->GetName(ranLibStrings));
			    PreLoadLib(off, e);
			    preLoaded = r->objOff;
                        }
                        if (ShouldLoad(sym)) {
			    currProg->LoadLib(
				file, moduleName, e, off, nlArr, this
			    );
			    justLoaded = r->objOff;
			}
			isProcessed[i] = true;
			didSomething = true;
		    }
		}
	    }
	}
    }
    return didSomething;
}

/* Load symbol table for this archive
 */
void LibFile::PreLoadLib (int off, Exec& e) {
    ReadBuff(file, &e, sizeof(e), "Couldn't read header in .o file", E_HEADER);
    fseek(file, e.StrOffset() + off, 0);
    int ssize;
    ReadBuff(
        file, &ssize, sizeof(ssize), "PreLoadLib - str tbl too big", E_STRING
    );
    ssize -= sizeof(int);
    if (currModST) {
        delete currModST;
    }
    currModST = new char[ssize];
    ReadBuff(file, currModST, ssize, "PreLoadLib - reading st", E_STRING);
    numSyms = e.a_syms / sizeof(NList);
    if (numSyms > nlSize) {
	Panic("nlArr not big enough");
    }
    fseek(file, e.SymOffset() + off, 0);
    ReadBuff(
	file, &nlArr[0], sizeof(NList) * numSyms,
	"LibFile.PreLoadLib : Error problem reading symbol table", E_SYMTAB
    );
}

/* 
 * return true if should load this library module
 */
boolean LibFile::ShouldLoad (Symbol* sym) {
    boolean shouldLoad, found;
    register NList *nl;
    register int i;
    const char* name;

    found = false;
    name = sym->GetName();
    for (i=0; i<numSyms && !found; i++) {
        nl = &nlArr[i];
        if (strcmp(name, &currModST[nl->Index() - sizeof(int)]) == 0) {
            found = true;
            if (nl->Type() == N_UNDF && nl->Global() && nl->n_value != 0) {
                sym->UpdateUndef(nl, nil, nil);
                shouldLoad = false;
            } else {
                shouldLoad = true;
            }
        }
    }
    if (!found) {
        Panic("Symbol %s not found in library", name);
    }
    return shouldLoad;
}

void LibFile::WriteText (FILE* outfile) {
    register Module* m = (Module*)inf;
    if (m != nil) {
	do {
	    m->WriteText(outfile);
	    m = (Module*) m->nn;
	} while (m != inf);
    }
}

void LibFile::WriteData (FILE* outfile) {
    register Module* m = (Module*)inf;
    if (m != nil) {
	do {
	    m->WriteData(outfile);
	    m = (Module*) m->nn;
	} while (m != inf);
    }
}

void LibFile::WriteSyms (FILE* outfile) {
    register Module* m = (Module*)inf;
    if (m != nil) {
	do {
            m->WriteSyms(outfile);
            m = (Module*) m->nn;
	} while (m != inf);
    }
}

void LibFile::WriteStrTab (FILE* outfile) {
    register Module* m = (Module*)inf;
    if (m != nil) {
	do {
            m->WriteStrTab(outfile);
            m = (Module*) m->nn;
	} while (m != inf);
    }
}

/* Library file has changed.  Purge all old contents.  New library modules
 * will be read in through Relink(), called when undefined references are
 * resolved at the end of the relink phase.
 */
void LibFile::Reread () {
    RemoveModules();
    delete ranlibs;
    delete ranLibStrings;
    delete isProcessed;

    OpenFile();
    if (IsRanLib()) {
        ReadTableOfContents();
        isProcessed = new boolean[numRanLibs];
        bzero(isProcessed, sizeof(isProcessed[0]) * numRanLibs);
    }
    CloseFile();
}

void LibFile::Relink () {
    OpenFile();
    ReadModules();
    CloseFile();
}

void LibFile::RemoveModules () {
    removeAll = true;
    while (inf != nil) {
        Module* doomed = (Module*) inf;
        doomed->Unlink(inf);
        delete doomed;
    }
    removeAll = false;
}

void LibFile::RelocLib (boolean incremental) {
    register Module* m = (Module*)inf;
    if (m != nil) {
	do {
	    if (incremental) {
		m->IncRelocMod();
	    } else {
		m->RelocMod();
	    }
	    m = (Module*) m->nn;
	} while (m != inf);
    }
}

/* Returns true if "sym" is defined in this archive
 */
boolean LibFile::DefinesSym (Symbol *sym) {
    const char *symName;
    int i;
    boolean found;
    Ranlib *r;

    found = false;
    symName = sym->GetName();
    for (i = 0; i < numRanLibs; i++) {
	r = &ranlibs[i];
	if (strcmp(r->GetName(ranLibStrings), symName) == 0) {
	    found = true;
	    break;
	}
    }
    return found;
}

void LibFile::RemoveModule (Module* doomed) {
    Debug("removing module %s", doomed->GetName());
    boolean found = false;
    if (removeAll == false) {
        for (register int i = 0; i < numRanLibs; i++) {
            if (isProcessed[i]) {
                Ranlib* r = &ranlibs[i];
                if (doomed->Defines(r->GetName(ranLibStrings))) {
                    Debug("    contains %s", r->GetName(ranLibStrings));
                    found = true;
                    isProcessed[i] = false;
                }
            }
        }
    }
    if (found || removeAll) {
        DeleteInf(doomed);
        delete doomed;
    } else {
        Panic("LibFile::RemoveModule: module %s not found", doomed->GetName());
    }
}
