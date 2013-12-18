/* 
 * The program object is the top-level symbol.
 */

#include "chunk.h"
#include "ctdt.h"
#include "errhandler.h"
#include "lib.h"
#include "loc.h"
#include "module.h"
#include "program.h"
#include "symbols.h" 
#include "symtab.h"
#include "system.h"
#include "types.h"
#include <errno.h>
#include <osfcn.h>
#include <sys/file.h>

#ifdef vax
/* should be defined in <sys/exec.h> */
static const int PAGSIZ = 1024;
#endif

static Symbol *etext, *edata;

Program::Program (const char* progName)  : (PROGRAM, progName)
{
    for (int i = 0; i < ERROR_SEG; i++) {
	segStart[i] = -1;
	segPos[i] = -1;
    }
    header.ZeroMe();
    textStart = dataStart = bssStart = anonStart = -1;
    undefList = nil;
    anonList = new Chunk(0, 0, ANON_SEG, progName);
    absList = new Chunk(0, 0, ABS_SEG, progName);
    absList->start = 0;
    fullLink = true;

    etext = AddAbsList("_etext", N_TEXT);
    edata = AddAbsList("_edata", N_DATA);
    AddAbsList("_end", N_BSS);

    globSyms = new SymTab(this);
    s_flag = false;
    X_flag = false;
    x_flag = false;
    k_flag = false;
    ctdt = new Ctdt();
    oldModList = nil;
}

Symbol* Program::AddAbsList (const char* name, int type) {
    Symbol* s = new Symbol(name, type, true);
    s->Enter();
    absList->InsertInf(s);
    return s;
}

/* 
 * Add/delete symbol to undefined list
 */

void Program::AddUndef (Symbol* s) {
    s->Link(undefList);
    s->owner = this;
}

void Program::DeleteUndef (Symbol* s) {
    s->Unlink(undefList);
}

/*
 * Reread a file contained in inclink (invoked by tinclink)
 */
void Program::Reread (const char* filename) {
    InputFile* file;
    if (units->Get(filename, file)) {
        file->Reread();
    }
}

/*
 * Before relinking, PrepareLink is called.  Then LinkFile is is used
 * to form the new list of files.
 */
void Program::PrepareLink (boolean full) {
    Error.Reset();
    fullLink = full;
    if (inf) {
        ctdt->Unlink(inf);
    }
    oldModList = (InputFile*)inf;
    inf = nil;
}

void Program::LinkFile (const char* filename) {
    InputFile* file;
    if (units->Get(filename, file)) {
        file->Unlink(oldModList);
    } else {
        file = AddFile(filename);
    }
    InsertInf(file);
}

void Program::DoLink () {
    while (oldModList != nil) {
        InputFile* doomed = (InputFile*) oldModList;
        oldModList = (InputFile*)oldModList->nn;
        doomed->Unlink(oldModList);
        delete doomed;
    }
    Base* insertPoint = inf->nn;
    ctdt->Link(insertPoint);
    if (fullLink) {
        FullLink();
    } else {
        Relink();
    }
}

InputFile* Program::AddFile (const char* filename) {
    FILE* f = fopen(filename, "r");
    InputFile* newFile;
    if (f == nil) {
         Warning("can't open %s", filename);
         newFile = nil;
    } else {
        FileType filetype = GetFileType(f);
        switch (filetype) {
        case OBJECT_FILE:
            newFile = currProg->AddModule(filename, f);
            fclose(f);
            break;
        case LIBRARY_FILE:
            fclose(f);
            newFile = currProg->AddLibrary(filename, filename);
            break;
        default:
            Warning("file '%s', not an object file or library", filename);
            fclose(f);
            newFile = nil;
        }
    }
    return newFile;
}

InputFile* Program::AddModule (const char* name, FILE* f) {
    Exec e;
    Module *mod;

    Debug("reading module %s", name);
    ReadBuff(f, &e, sizeof(e), "Couldn't read header in .o file", E_HEADER);
    mod = new Module(name, &e, f, 0);
    mod->Read3Parts(e);
    mod->CloseFile();
    units->Insert(name, mod);
    return mod;
}

InputFile* Program::AddLibrary (const char* unitName, const char* libName) {
    LibFile* lib;

    Debug("reading library %s", libName);
    lib = new LibFile(libName);
    units->Insert(unitName, lib);
    lib->Init();
    return lib;
}

void Program::LoadLib (
    FILE* f, const char* modName, Exec &e, int off, NList nla[], LibFile* lib
) {
    int n, i;
    Module* mod;

    mod = new Module(modName, &e, f, off, lib);
    lib->InsertInf(mod);
    mod->ReadStrTable(e);
    n = e.a_syms / sizeof(NList);
    ResizeSymArray(n);
    for (i=0; i<n; i++) {
	symArray[i] = DoSym(&nla[i], mod);
        mod->AddLocal(&nla[i]);
    }
    mod->ReadRelocs(e);
}

inline boolean IsModule (Base* b) {
    return (b->WhatAmI() == MODULE);
}

void Program::FinalizeAddrs () {
    Debug("finalizing addresses");
    textAlloc = 0;
    dataAlloc = 0;
    bssAlloc = 0;
    SetChunkAddrs();			/* set chunk addrs, so set seg size */
    FinalizeSegments();			/* round up seg size to page size */
    SetAnonAddrs();			/* uses dataStart+dataAlloc */
    SetSymAddrs();
    SetESymAddrs();			/* uses anonAlloc from SetAnonAddrs */
}

void Program::SetChunkAddrs () {
    register Module* m;
    register Base* b = inf;

    do {
	if (IsModule(b)) {
	    m = (Module*) b;
	    textAlloc += m->text->SetAddrs(textAlloc);
	    dataAlloc += m->data->SetAddrs(dataAlloc);
	    bssAlloc += m->bss->SetAddrs(bssAlloc);
	} else {
	    m = (Module*) b->inf;	/* b = pointer to library */
	    if (m != nil) {
		do {
		    textAlloc += m->text->SetAddrs(textAlloc);
		    dataAlloc += m->data->SetAddrs(dataAlloc);
		    bssAlloc += m->bss->SetAddrs(bssAlloc);
		    m = (Module*) m->nn;
		} while (m != b->inf);
	    }
	}
	b = b->nn;
    } while (b != inf);
}

void Program::SetESymAddrs () {
    Symbol* sym;
    					/* this stuff is hardwired, ugh. */
    if (symTab->Get("_etext",sym)) {
	sym->SetAddr(dataStart, !fullLink);
    }
    if (symTab->Get("_edata",sym)) {
	sym->SetAddr(anonStart, !fullLink);
    }
    if (symTab->Get("_end",sym)) {
	sym->SetAddr(anonStart+anonAlloc, !fullLink);
    }
}

void Program::SetAnonAddrs () {
    if (anonList->inf != nil) {
	anonAlloc = ((Chunk*)anonList)->SetAnonListAddr(anonStart);
    }
    header.a_bss = header.a_bss + anonAlloc;
    if (int(header.a_bss) < 0) {
	header.a_bss = 0;		/* spare space in data segment */
    }
}

void Program::SetSymAddrs () {
    Module* m;
    Base* b = inf;
    
    do {
	if (IsModule(b)) {
	    m = (Module*) b;
	    m->SetSymAddrs();
	} else {
	    m = (Module*) b->inf;	/* b = pointer to library */
	    if (m != nil) {
		do {
		    m->SetSymAddrs();
		    m = (Module*) m->nn;
		} while (m != b->inf);
	    }
	}
	b = b->nn;
    } while (b != inf);
}

void Program::FinalizeSegments () {
    header.a_magic = ZMAGIC;
    header.a_text = RoundUp(textAlloc, PAGSIZ);
    header.a_data = RoundUp(dataAlloc, PAGSIZ);
    header.a_bss = bssAlloc - (header.a_data - dataAlloc);
#ifdef sun
    header.a_machtype = M_68020;
    header.a_entry = PAGSIZ + sizeof(header);
    UpdateSegment(textStart, header.a_entry, N_TEXT);
    UpdateSegment(dataStart, RoundUp(textAlloc, SEGSIZ), N_DATA);
    segPos[N_TEXT] = sizeof(header);
    segPos[N_DATA] = header.a_text;
#endif
#ifdef vax
    UpdateSegment(textStart, 0, N_TEXT);
    UpdateSegment(dataStart, header.a_text, N_DATA);
    segPos[N_TEXT] = PAGSIZ;
    segPos[N_DATA] = segPos[N_TEXT] + header.a_text;
#endif
    UpdateSegment(bssStart, dataStart + dataAlloc, N_BSS);
    segPos[N_BSS] = segPos[N_DATA] + dataAlloc;
    segPos[ANON_SEG] = segPos[N_BSS] + bssAlloc;
    int newAnonStart = AlignTo(WORDSIZE/BYTESIZE, bssStart + bssAlloc);
    if (anonStart != newAnonStart) {
	anonStart = newAnonStart;
	segStart[ANON_SEG] = newAnonStart;
	anonList->NeedToReloc();
    }
}

void Program::UpdateSegment (int& sStart, int newStart, int segType) {
    if (sStart != newStart) {
	if (sStart != -1) {
	    MarkSegment(segType, true);
	}
	sStart = newStart;
	segStart[segType] = newStart;
    }
}
    	
void Program::MarkSegment (int segType, boolean status) {
    Module* m;
    Chunk* c;
    Base* b = inf;
    
    b = inf;
    do {
	if (IsModule(b)) {
	    m = (Module*) b;
	    c = m->DetermineChunk(segType);
	    c->NeedToReloc(status);
	} else {
	    m = (Module*) b->inf;	/* b = pointer to library */
	    if (m != nil) {
		do {
		    c = m->DetermineChunk(segType);
		    c->NeedToReloc(status);
		    m = (Module*) m->nn;
		} while (m != b->inf);
	    }
	}
	b = b->nn;
    } while (b != inf);
}

boolean Program::CheckForUndefs (boolean final) {
    boolean unresolved = false;
    Symbol* sym;

    if (undefList != nil) {
	sym = (Symbol*) undefList;
	do {
	    if (sym->inf != nil) {
                if (final) {
                    Error("error: undefined: %s", sym->GetName());
		}
		unresolved = true;
                sym = (Symbol*) sym->nn;
	    } else {
		Debug("removing %s (no references)", sym->GetName());
                Symbol* doomed = sym;
                sym = (Symbol*) sym->nn;
                DeleteUndef(doomed);
                doomed->Dispose();
	    }
	} while (undefList != nil && sym != undefList);
    }
    return unresolved;
}

boolean Program::Relocate (boolean incremental) {
    register Module* mod;
    register Base* base;
    register Base* end;
    LibFile* lib;
    boolean unresolved;
    
    unresolved = CheckForUndefs(true);
    if (unresolved) {
	return false;
    }

    base = inf;
    end = base;
    do {
	if (incremental) {
	    if (IsModule(base)) {
		mod = (Module*) base;
		mod->IncRelocMod();
	    } else {
		lib = (LibFile*) base;
		lib->IncRelocLib();
	    }
	} else {
	    if (IsModule(base)) {
		mod = (Module*) base;
		mod->RelocMod();
	    } else {
		lib = (LibFile*) base;
		lib->RelocLib();
	    }
	}
	base = base->nn;
    } while (base != end);
    if (incremental) {
	anonList->RelocINRefs();
	absList->RelocINRefs();
    } else {
	MarkSegment(N_BSS, false);	/* do not reloc bss chunks 1st time */
    }
    return true;
}

boolean Program::WriteObjFile () {
    const char* filename = GetName();
    boolean needRewrite;
    FILE* f = OpenObjFile(filename, needRewrite);
    if (Error.NumErrors() > 0) {
        Error("Errors encountered.  Object file not written.");
        if (f > 0) {
            fclose(f);
        }
        return false;
    }

    Debug("Writing object file");
    if (needRewrite) {
       ForceRewrite();
    }
    WriteBuff(f, &header, sizeof(header));
    WriteTextSeg(f);
    WriteDataSeg(f);
    if (!s_flag) {
	WriteSymbols(f);
        WriteStrTab(f);
    }
    ftruncate(fileno(f), header.StrOffset() + strAlloc);
    int mode = 0777 & ~umask(0);
    chmod(filename, mode);
    fclose(f);

    return true;
}

FILE* Program::OpenObjFile (const char* filename, boolean &needRewrite) {
    // Mainly check for "Text file busy" conditions
    int fd = open(filename, O_RDWR);
    if (fd == -1) {
        if (errno == 2) {
            // No such file or directory
        } else {
            Error(sys_errlist[errno]);
            return nil;
        }
    } else {
        close(fd);
    }

    needRewrite = false;
    FILE* f = fopen(filename, "r+");
    if (f == NULL) {
        f = fopen(filename, "w");
        if (fullLink == false) {
            needRewrite = true;
        }
    }
    return f;
}

void Program::ForceRewrite () {
    register Module* m;
    register Base* b = inf;

    Warning("Can't find '%s', rewriting the entire executable.", GetName());
    do {
        if (IsModule(b)) {
            m = (Module*) b;
            m->text->NeedToReloc(true);
            m->data->NeedToReloc(true);
            m->bss->NeedToReloc(true);
            m->symtab->SetDirty();
            m->outStrtab->SetDirty();
        } else {
            m = (Module*) b->inf;       /* b = pointer to library */
            if (m != nil) {
                do {
                    m->text->NeedToReloc(true);
                    m->data->NeedToReloc(true);
                    m->bss->NeedToReloc(true);
                    m->symtab->SetDirty();
                    m->outStrtab->SetDirty();
                    m = (Module*) m->nn;
                } while (m != b->inf);
            }
        }
        b = b->nn;
    } while (b != inf);
    globSyms->SetDirty();
    strTab->SetDirty();
}

void Program::WriteTextSeg (FILE* f) {
    Debug("writing text");
    register InputFile* b = (InputFile*)inf;
    do {
	b->WriteText(f);
	b = (InputFile*)b->nn;
    } while (b != inf);
    Pad(f, segPos[N_TEXT] + textAlloc);
}

void Program::WriteDataSeg (FILE* f) {
    Debug("writing data");
    InputFile* b = (InputFile*)inf;
    do {
	b->WriteData(f);
	b = (InputFile*)b->nn;
    } while (b != inf);
    Pad(f, segPos[N_DATA] + dataAlloc);
}

void Program::Pad (FILE* f, int pos) {
    if (ftell(f) != pos) {
        fseek(f, pos, 0);
    }
    int fill = (PAGSIZ - (pos % PAGSIZ)) % PAGSIZ;
    if (fill > 0) {
	WriteBuff(f, zero, fill);
    }
}

void Program::WriteSymbols (FILE* f) {
    Debug("Writing symbols");
    if (!x_flag) {
        WriteLocals(f);
    }
    WriteGlobals(f);
}

void Program::WriteLocals (FILE* f) {
    register InputFile* b = (InputFile*)inf;
    do {
        b->WriteSyms(f);
        b = (InputFile*) b->nn;
    } while (b != inf);
}

void Program::WriteGlobals (FILE* f) {
    globSyms->WriteBuff(f);
}

void Program::WriteStrTab (FILE* f) {
    Debug("Writing string table");
    register InputFile* b = (InputFile*) inf;
    if (ftell(f) != header.StrOffset()) {
        fseek(f, header.StrOffset(), 0);
    }
    WriteBuff(f, &strAlloc, sizeof(strAlloc));
    if (!s_flag) {
        if (!x_flag) {
            do {
                b->WriteStrTab(f);
                b = (InputFile*) b->nn;
            } while (b != inf);
        }
        strTab->WriteBuff(f);
    }
}

void Program::SelectCtdt (const char* sname) {
    ctdt->Select(sname);
}

void Program::FullLink () {
    if (inf != nil) {
	ctdt->Munch();
	FinalizeAddrs();
	Relocate(false);
        FinalizeStab();
        if (WriteObjFile()) {
            fullLink = false;
        }
    }
    /* in preparation for a relink */
    anonAlloc = 0;
    anonStart = 0;
    anonList->NullAnonChunk();
}

void Program::Relink () {
    if (inf != nil) {
        UpdateFiles();
        boolean undef = CheckForUndefs(false);
        if (undef) {
            RereadLibs();
        }
        ctdt->Munch();
        FinalizeAddrs();
        IncReloc();
        FinalizeStab();
        IncWriteObjFile();
        anonAlloc = 0;
        anonStart = 0;
        anonList->NullAnonChunk();
    }
}

void Program::UpdateFiles () {
    InputFile* f = (InputFile*) inf;
    do {
        if (tinclink == false && f != ctdt) {
            f->Update();
        } else if (IsModule(f) == false) {
            /* always stat libraries */
            f->Update();
        }
        f = (InputFile*)f->nn;
    } while (f != inf);
}

void Program::RereadLibs () {
    if (inf != nil) {
        Base* b = inf;
        do {
            if (b->WhatAmI() == LIBRARY) {
                ((LibFile*)b)->Relink();
            }
            b = b->nn;
        } while (b != inf);
    }
}

void Program::FinalizeStab () {
    SetSymTabAddr();
    int newSymPos = segPos[N_DATA] + header.a_data;
    if (newSymPos != segPos[SYM_SEG]) {
        segPos[SYM_SEG] = newSymPos;
        ShiftSymTab();
    }
    header.a_syms = symAlloc;

    SetStrTabAddr();
    int newStrPos = segPos[SYM_SEG] + header.a_syms;
    if (newStrPos != segPos[STR_SEG]) {
        segPos[STR_SEG] = newStrPos;
        ShiftStrTab();
    }
}

void Program::SetSymTabAddr () {
    symAlloc = 0;
    if (!x_flag && !s_flag) {
        register Base* b = inf;
        do {
            if (IsModule(b)) {
                symAlloc += ((Module*)b)->symtab->SetOffset(symAlloc);
            } else {
                register Module* m = (Module*) b->inf;
                if (m != nil) {
                    do {
                        symAlloc += ((Module*)m)->symtab->SetOffset(symAlloc);
                        m = (Module*) m->nn;
                    } while (m != b->inf);
                }
            }
            b = b->nn;
        } while (b != inf);
    }
    if (!s_flag) {
        GenGlobals();
        symAlloc += globSyms->SetOffset(symAlloc);
    }
}

void Program::SetStrTabAddr () {
    strAlloc = sizeof(int);
    if (!x_flag && !s_flag) {
        register Base* b = inf;
        register Module* m;
        do {
            if (IsModule(b)) {
                m = (Module*) b;
                strAlloc += m->outStrtab->SetOffset(strAlloc, m->symtab);
            } else {
                register Module* m = (Module*) b->inf;
                if (m != nil) {
                    do {
                        strAlloc += 
                            m->outStrtab->SetOffset(strAlloc, m->symtab);
                        m = (Module*) m->nn;
                    } while (m != b->inf);
                }
            }
            b = b->nn;
        } while (b != inf);
    }
    if (!s_flag) {
        strAlloc += strTab->SetOffset(strAlloc, globSyms);
    }
}

void Program::ShiftSymTab () {
    if (!x_flag && !s_flag) {
        register Base* b = inf;
        register Module* m;
        do {
            if (IsModule(b)) {
                m = (Module*) b;
                m->symtab->SetDirty();
            } else {
                register Module* m = (Module*) b->inf;
                if (m != nil) {
                    do {
                        m->symtab->SetDirty();
                        m = (Module*) m->nn;
                    } while (m != b->inf);
                }
            }
            b = b->nn;
        } while (b != inf);
    }
    if (!s_flag) {
        strTab->SetDirty();
    }
}

void Program::ShiftStrTab () {
    if (!x_flag && !s_flag) {
        register Base* b = inf;
        register Module* m;
        do {
            if (IsModule(b)) {
                m = (Module*) b;
                m->outStrtab->SetDirty();
            } else {
                register Module* m = (Module*) b->inf;
                if (m != nil) {
                    do {
                        m->outStrtab->SetDirty();
                        m = (Module*) m->nn;
                    } while (m != b->inf);
                }
            }
            b = b->nn;
        } while (b != inf);
    }
    if (!s_flag) {
        strTab->SetDirty();
    }
}

void Program::GenGlobals() {
    globSyms->Reinit();
    HashIter i(symTab);
    Symbol* sym;
    nlist n;
    n.n_other = 0;
    n.n_desc = 0;
    register nlist* np = &n;
    while (i.MoreEntries(sym)) {
        register Symbol* s = sym;
        if (s == etext || s == edata) {
	    continue;
	}
	np->n_un.n_strx = s->name;
	if (s->type == N_ANON) {
	    np->n_type = N_BSS;
	} else {
	    np->n_type = s->type;
	}
	if (s->global) {
	    np->n_type |= N_EXT;
	}
	np->n_value = s->currAddr;
        globSyms->Append(np);
    }
}

void Program::SetsFlag (boolean b) {
    if (inf == nil) {
        s_flag = b;
    } else if (s_flag != b) {
        Warning("Ignoring change in -s flag during relink.");
    }
}

void Program::SetxFlag (boolean b) {
    if (inf == nil) {
        x_flag = b;
    } else if (x_flag != b) {
        Warning("Ignoring change in -x flag during relink.");
    }
}

void Program::SetXFlag (boolean b) {
    if (inf == nil) {
        X_flag = b;
    } else if (X_flag != b) {
        Warning("Ignoring change in -X flag during relink.");
    }
}
