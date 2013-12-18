// Implementation of class Ctdt
//
// Strategy:
//
//   Ctdt keeps a list of static initializers (ctors) and a list of static
//   destructors (dtors).  Before reading an object file, Ctdt purges the
//   ctors/dtors lists.  During symbol input, global symbols that are of
//   the format "__ST[I|D]" are added to the ctors/dtors list.  Before
//   relocation, the munch phase creates a relocation list (relocList) for
//   the Ctdt data chunk from the ctors/dtors list.

#include "ctdt.h"
#include "loc.h"
#include "symbols.h"
#include "symtab.h"
#include <string.h>
#include <osfcn.h>

Ctdt::Ctdt () : ("__ctdt.o") {
    ctor = new Symbol("__ctors", N_DATA, true);
    dtor = new Symbol("__dtors", N_DATA, true);
    ctors = nil;
    dtors = nil;
    nctors = 0;
    ndtors = 0;

    // Insert place-holders for _ctor[], and _dtor[], blocking these
    // symbols from being loaded from libC.a
    ctor->Enter();
    dtor->Enter();
    data->InsertInf(ctor);
    data->InsertInf(dtor);
}

// Purge relocation list for data chunk before a relink.
void Ctdt::RemoveRelocs() {
    Location* l = data->relocList;

    if (l != nil) {
        while (l != nil) {
            l->UnlinkLoc(data);
            l->DisposeLoc();
            l = data->relocList;
        }
        data->NeedToReloc();
    }
}

// Select static constructors/destructors of the form _ST* or __ST*.
void Ctdt::Select (const char* sname) {
    if (sname[0] == '_') {
	int indexOfS = 1;
	if (sname[indexOfS] == '_') {
	    indexOfS++;
	}
	if (sname[indexOfS] == 'S' && sname[indexOfS + 1] == 'T') {
	    int iOrD = indexOfS + 2;
            switch (sname[iOrD]) {
            case 'I':
                if (NotFound(sname, ctors)) {
                    Add(sname, ctors);
                }
                break;
            case 'D':
                if (NotFound(sname, dtors)) {
                    Add(sname, dtors);
                }
                break;
            }
        }
    }
}

boolean Ctdt::NotFound (const char* sname, BaseName*& head) {
    if (head != nil) {
        BaseName* ctdt = head;
        do {
            if (strcmp(sname, ctdt->GetName()) == 0) {
                return false;
            }
            ctdt = (BaseName*) ctdt->nn;
        } while (ctdt != head);
    }
    return true;
}

void Ctdt::Add (const char* sname, BaseName*& head) {
    BaseName* newctdt = new BaseName(BASE, sname);
    newctdt->Link(head);
}

void Ctdt::Munch () {
    nctors = AddRelocList(ctors, 0, 0);
    int dtorsOffset = (nctors + 1) * sizeof(void*);

    ndtors = AddRelocList(dtors, nctors, dtorsOffset);
    int dataSize = (nctors + 1 + ndtors + 1) * sizeof(void*);

    data->UpdateChunk(dataSize, 0, 0);
    bzero(data->buff, data->alloc);
    dtor->offset = dtorsOffset;

    UpdateSymtab();
}

int Ctdt::AddRelocList (BaseName*& ctdt, int symNum, int offset) {
    int count = 0;
    if (ctdt) {
        BaseName* b = ctdt;
        do {
            count++;
            b = (BaseName*) b->nn;
        } while (b != ctdt);

        int deleted = 0;
        for (int i = 0; i < count; i++) {
            BaseName* curName = b;
            b = (BaseName*) b->nn;
            Symbol* sym;
            if (symTab->Get(curName->GetName(), sym)) {
                GenReloc(sym, symNum, offset);
                symNum++;
                offset += sizeof(void*);
            } else {
                curName->Unlink(ctdt);
                delete curName;
                deleted++;
            }
        }
        count -= deleted;
    }
    return count;
}

void Ctdt::GenReloc (Symbol* sym, int symNum, int offset) {
    RelocInfo ri;
        ri.r_address = offset;
        ri.r_symbolnum = symNum;
        ri.r_pcrel = false;
        ri.r_length = 2;     // long
        ri.r_extern = true;
    Location* l = AllocLoc(&ri);
    sym->InsertInf(l);
    l->LinkLoc(data);
    Chunk* ch = (Chunk*)sym->owner;
    ch->NeedToReloc(true);
    data->NeedToReloc();
}

void Ctdt::UpdateSymtab () {
    symtab->Reinit();
    delete outStrtab;
    outStrtab = new StrTable();

    if (nctors + ndtors > 0) {
        symtab->AddModName();
        outStrtab->AddString(GetName());
    }
}

void Ctdt::WriteSource () {
    if (currProg->k_flag) {
        FILE* srcfile = fopen("__ctdt.c", "w");
        fprintf(srcfile, "typedef int (*PFV)();\n");
        DoWrite(srcfile, ctors, "ctors");
        DoWrite(srcfile, dtors, "dtors");
        fclose(srcfile);
        Compile();
    }
}

void Ctdt::DoWrite (FILE* srcfile, BaseName* ctdt, const char* arrayName) {
    if (ctdt) {
        BaseName* cp = ctdt;
        do {
            fprintf(srcfile, "int %s();\n", cp->GetName());
            cp = (BaseName*)cp->nn;
        } while (cp != ctors);
        fprintf(srcfile, "PFV _%s[] = {\n", arrayName);
        cp = ctdt;
        do {
            fprintf(srcfile, "\t%s,\n", cp->GetName());
            cp = (BaseName*)cp->nn;
        } while (cp != ctdt);
        fprintf(srcfile, "\t0\n};\n");
    } else {
        fprintf(srcfile, "PFV _%s[] = { 0 };\n", arrayName);
    }
}

void Ctdt::Compile () {
    int childid = fork();
    switch (childid) {
    case 0:	// child
	execl("/bin/cc", "cc", "__ctdt.c", "-c", 0);
        Panic("Ctdt::Compile: failed in execl");
    default:   // parent
        int status;
        int pid;
	do {
	    pid = wait(&status);
	} while (pid != childid && pid != -1);
	if (status != 0) {
	    if ((status & 0xff) == 0) {
		status = status >> 8;
		if (status != 0) {
		    Panic("Program::CompileCtdt, status = %d", status);
		}
	    }
	}
	break;
    case -1: /* error */
	Panic("Program::CompileCtdt, can't fork");
	break;
    }
}
