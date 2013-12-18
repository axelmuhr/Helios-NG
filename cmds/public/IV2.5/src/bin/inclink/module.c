/*
 * Module class.
 */

#include <stab.h>

#include "aout.h"
#include "ctdt.h"
#include "lib.h"
#include "loc.h"
#include "module.h"
#include "program.h"
#include "types.h"
#include "strtable.h"
#include "symbols.h"
#include "symtab.h"
#include "system.h"
#include <string.h>

static int symArraySize = 0;
Symbol** symArray;

static char* strtab;                    /* string table used by all mods */
static const strInitSize = 12;

void ResizeSymArray (int numSyms) {
    if (numSyms > symArraySize) {
        int byteSize = 1;
        int needed = numSyms * sizeof(Symbol*) + sizeof(int);
        while (needed) {
            needed >>= 1;
            byteSize <<= 1;
        }
        symArraySize = (byteSize - sizeof(int)) / sizeof(Symbol*);
        delete symArray;
        symArray = new Symbol*[symArraySize];
    }
}

// This contructor used exclusively by Ctdt
Module::Module (const char* name) : (MODULE, name, nil) {
    text = new Chunk(0, 0, TEXT_SEG, name);
    data = new Chunk(0, 0, DATA_SEG, name);
    bss = new Chunk(0, 0, BSS_SEG, name);

    symtab = new SymTab(this);
    symtab->AddModName();
    outStrtab = new StrTable(strInitSize);
    outStrtab->AddString(StripPath(GetName()));

    libOwner = nil;
    foffset = 0;

    InsertInf(text);
    InsertInf(data);
    InsertInf(bss);
}

Module::Module (
    const char* name, Exec* e, FILE* f, int off, Base* lib
) : (MODULE, name, f) {
    ex = *e;
    text = new Chunk(0, e->a_text, TEXT_SEG, name);
    data = new Chunk(e->a_text, e->a_data, DATA_SEG, name);
    bss = new Chunk(e->a_text+e->a_data, e->a_bss, BSS_SEG, name);

    symtab = new SymTab(this);
    symtab->AddModName();
    outStrtab = new StrTable(strInitSize);
    outStrtab->AddString(StripPath(GetName()));

    libOwner = lib;
    foffset = off;

    InsertInf(text);
    InsertInf(data);
    InsertInf(bss);
    
    fseek(f, e->TextOffset() + foffset, 0);
    text->FillBuff(f);
    data->FillBuff(f);
}

Module::~Module () {
    currProg->ctdt->RemoveRelocs();
    while (inf != nil) {
        Chunk* doomed = (Chunk*) inf;
        doomed->InvalidateChunk();
        doomed->Unlink(inf);
        delete doomed;
    }
    delete symtab;
    delete outStrtab;
}

void Module::RelocMod (boolean incremental) {
    Chunk* ch = (Chunk*)inf;
    if (ch != nil) {
	do {
	    if (incremental) {
		ch->IncRelocChunk();
	    } else {
		ch->RelocChunk();
	    }
	    ch = (Chunk*) ch->nn;
	} while (ch != inf);
    }
}

Chunk* Module::DetermineChunk (int t) {
    if (t == N_TEXT) {			/* hardwire this for now - faster */
	return text;
    } else if (t == N_DATA) {
	return data;
    } else if (t == N_BSS) {
	return bss;
    } else if (t == N_ANON) {
	return currProg->anonList;
    } else if (t == N_ABS) {
	return currProg->absList;
    } else {
	return nil;
    }
}

const int HANDFUL = 1024;

int Module::ReadSymbols (Exec e) {
    NList nl[HANDFUL];
    int i, j, numSyms, numRead;
    Symbol* s;

    symtab->Reinit();
    symtab->AddModName();
    delete outStrtab;
    outStrtab = new StrTable(strInitSize);
    outStrtab->AddString(StripPath(GetName()));

    numSyms = e.a_syms / sizeof(nlist);
    ResizeSymArray(numSyms);
    fseek(file, e.SymOffset() + foffset, 0);
    j = 0;
    while (numSyms > 0) {
	numRead = min(numSyms, HANDFUL);
	ReadBuff(
	    file, &nl[0], numRead * sizeof(nl[0]), 
	    "Error reading symbol table entry in a.out file", E_SYMTAB
	);
	for (i=0; i<numRead; ++i) {
	    s = DoSym(&nl[i], this);
	    symArray[j] = s;		/* need to make this extensible */
            AddLocal(&nl[i]);
	    j++;
	}
	numSyms -= numRead;
    }
    return 0;
}

void Module::AddLocal (NList* nlp) {
    if (nlp->Stab() != 0 || !nlp->Global()) {
        register int strIndex = nlp->Index();
        if (!currProg->X_flag || strtab[strIndex - sizeof(int)] != 'L') {
            if (nlp->n_type == N_SLINE) {
                nlp->n_un.n_strx = 0;
                symtab->Append(nlp);
            } else {
                if (strIndex != 0) {
                    nlp->n_un.n_strx =
                        outStrtab->AddString(&strtab[strIndex - sizeof(int)]);
                }
                symtab->Append(nlp);
            }
        }
    }
}

void Module::InternalizeRelocs (Chunk* ch, int numRelocs) {
    const int HANDFUL = 1024;

    int numRead;
    RelocInfo ri[HANDFUL];
    
    while (numRelocs > 0) {
	numRead = min(HANDFUL, numRelocs);
	ReadBuff(file, &ri[0], numRead * sizeof(ri[0]), 
	    "Error reading text relocation info in a.out file", E_RELOC
	);
	DoLoc(&ri[0], this, ch, symArray, numRead);
	numRelocs -= numRead;
    }
}

int Module::ReadRelocs (Exec e) {
    fseek(file, e.RelocOffset() + foffset, 0);
    int tr = e.a_trsize / sizeof(RelocInfo);
    int dr = e.a_drsize / sizeof(RelocInfo);
    InternalizeRelocs(text, tr);
    InternalizeRelocs(data, dr);
    return 0;
}

static int inStrtabSize = 0;              /* size of string table */
void Module::ReadStrTable (Exec e) {
    if (inStrtabSize != 0) {
	delete [inStrtabSize] strtab;
    }
    fseek(file, e.StrOffset() + foffset, 0);
    ReadBuff(file, &inStrtabSize, sizeof(inStrtabSize),
        "Can't read string table size"
    );
    if (inStrtabSize > 0) {
	strtab = new char[inStrtabSize];
	ReadBuff(file, strtab, inStrtabSize - sizeof(int),
            "Can't read string table"
	);
    }
}

void Module::Read3Parts (Exec e) {
    ReadStrTable(e);
    ReadSymbols(e);
    ReadRelocs(e);
}

void Module::SetSymAddrs () {
    boolean t, d, b;
    t = text->SetInfSymAddrs();
    d = data->SetInfSymAddrs();
    b = bss->SetInfSymAddrs();
}

void Module::WriteText (FILE* f) {
    text->WriteBuff(f);
}

void Module::WriteData (FILE* f) {
    data->WriteBuff(f);
}

void Module::WriteSyms (FILE* f) {
    symtab->WriteBuff(f);
}

void Module::WriteStrTab (FILE* f) {
    outStrtab->WriteBuff(f);
}

void Module::OpenFile () {
    if (PartOfLib()) {
	file = ((LibFile *)owner)->GetFile();
	fseek(file, foffset, 0);
    } else {
        InputFile::OpenFile();
    }
}

void Module::Reread () {
    register Chunk* ch;
    Exec e;

    Debug("Rereading '%s'", GetName());
    OpenFile();
    ReadBuff(file, &e, sizeof(e), "Couldn't read header", E_HEADER);
    ex = e;
    currProg->ctdt->RemoveRelocs();
    ch = (Chunk*) inf;
    if (ch != nil) {
        do {
    	ch->InvalidateChunk();
    	ch = (Chunk*) ch->nn;
        } while (ch != inf);
    }
    UpdateChunkAlloc(&e);

    Read3Parts(e);
    CloseFile();
}

void Module::UpdateChunkAlloc (Exec* e) {
    text->UpdateChunk(e->a_text, currProg->textAlloc, 0);
    data->UpdateChunk(e->a_data, currProg->dataAlloc, e->a_text);
    bss->UpdateChunk(e->a_bss, currProg->bssAlloc, e->a_text+e->a_data);
    fseek(file, e->TextOffset() + foffset, 0);
    text->FillBuff(file);
    data->FillBuff(file);
}

boolean Module::Defines (const char* symName) {
    Chunk* c = (Chunk*) inf;
    do {
        Symbol* s = (Symbol*) c->inf->nn;
        while (s != c->inf) {
            if (strcmp(symName, s->GetName()) == 0 && s->global) {
                return true;
            }
            s = (Symbol*) s->nn;
        }
        c = (Chunk*) c->nn;
    } while (c != inf);
    return false;
}

/* Return true if all chunks of this module have no list of locations
 * attached all lists of symbols.
 */
boolean Module::NoRefs () {
    Chunk* c = (Chunk*) inf;
    do {
        Symbol* s = (Symbol*) c->inf->nn;   /* first symbol is a placeholder */
        while (s != c->inf) {
            if (s->inf != nil) {
                return false;
            }
            s = (Symbol*) s->nn;
        }
        c = (Chunk*) c->nn;
    } while (c != inf);
    return true;
}

void Module::IncRelocMod () {
    RelocMod(true);
}

void Module::ReadSymName (int strIndex, char* symName) {
    char* src;
    char* dst;

    src = strtab + strIndex - sizeof(int);
    dst = symName;
    while (*src && *src != EOF) {
	*dst++ = *src++;
    }
    *dst = 0;
}
