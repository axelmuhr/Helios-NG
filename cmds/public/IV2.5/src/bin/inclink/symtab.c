/* Implementation of class SymTab and class StrTab
 */

#include "module.h"
#include "program.h"
#include "symtab.h"
#include "system.h"
#include "types.h"
#include <stab.h>

SymTab::SymTab (BaseName* b) {
    buff = nil;
    owner = b;
    Reinit();
}

SymTab::~SymTab () {
    delete buff;
}

void SymTab::Reinit () {
    delete buff;
    buff = nil;
    size = 0;
    alloc = 0;
    offset = -1;
    dirty = true;
}

void SymTab::Append (nlist* nl) {
    if (size == alloc) {
        AllocBuff();
    }
    nlist* bp = (nlist*)((char*)buff + size);
    *bp = *nl;
    size += sizeof(nlist);
    dirty = true;
}

int SymTab::SetOffset (int newOffset) {
    if (offset != newOffset) {
        offset = newOffset;
        dirty = true;
    }
    return GetSize();
}

void SymTab::Relocate (int strtabOffset) {
    if (size > 0) {
        int textAddr, dataAddr, bssAddr;
        boolean isModule = owner->WhatAmI() == MODULE;
        if (isModule) {
            Module* m = (Module*) owner;
            textAddr = m->text->FinalAddr();
            dataAddr = currProg->SegStart(m->data->type) + m->data->Move();
            bssAddr = currProg->SegStart(m->bss->type) + m->bss->Move();
            buff[0].n_value = textAddr;    // module name
            buff[0].n_un.n_strx += strtabOffset;
        } else {
            // program
            buff[0].n_un.n_strx += strtabOffset;
        }
        nlist* buffend = (nlist*)((char*)buff + size);
        for (register nlist* bp = buff + 1; bp < buffend; bp++) {
            if (isModule) {
                register int type = bp->n_type;
                if (type == N_FUN || type == N_SLINE ||
                    type == N_SO  || type == N_SOL ||
                    type == N_TEXT      // static functions
                ){
                    bp->n_value += textAddr; 
                } else if (type == N_DATA || type == N_STSYM) {
                    bp->n_value += dataAddr;
                } else if (type == N_BSS  || type == N_LCSYM) {
                    bp->n_value += bssAddr;
                }
            }
            if (bp->n_un.n_strx != 0) {
                bp->n_un.n_strx += strtabOffset;
            }
        }
        dirty = true;
    }
}

void SymTab::WriteBuff (FILE* f) {
    if (dirty) {
        long foffset = currProg->SegPos(SYM_SEG) + offset;
        if (ftell(f) != foffset) {
            fseek(f, foffset, 0);
        }
        ::WriteBuff(f, buff, GetSize());
        if (Debug.StartMessage()) {
            BaseName* bn = (BaseName*) owner;
            Debug.Add("wrote symtab ");
            Debug.Add(bn->GetName());
            Debug.Add(" at ");
            Debug.Add((int)foffset);
            Debug.EndMessage();
        }
        dirty = false;
    }
}

void SymTab::AddModName () {
    nlist modname;
    modname.n_type = N_TEXT;
    modname.n_desc = 0;
    modname.n_other = 0;
    modname.n_un.n_strx = 0;
    Append(&modname);
}

void SymTab::AllocBuff () {
    int needed = size + sizeof(nlist);
    int powerOf2 = 8;
    for (;;) {
        int malloced = powerOf2 - sizeof(int);
        alloc = malloced - malloced % sizeof(nlist);
        if (alloc > needed) {
            break;
        }
        powerOf2 += powerOf2;
    }
    nlist* newbuff = (nlist*) new char[alloc];
    if (buff != nil) {
        bcopy(buff, newbuff, size);
        bzero((char*)newbuff + size, alloc - size);
        delete buff;
    }
    buff = newbuff;
}

int SymTab::GetSize () {
    int returnValue;
    int withSlop = min(int(size * 1.1), alloc);
    withSlop -= withSlop % sizeof(nlist); // ensure on nlist boundary
    int baseType = owner->WhatAmI();
    switch (baseType) {
    case MODULE:
        if (((Module*)owner)->PartOfLib()) {
            returnValue = size;
        } else {
            returnValue =  currProg->addSlop ? withSlop : size;
        }
        break;
    case PROGRAM:
        returnValue = currProg->addSlop ? withSlop : size;
        break;
    default:
        Panic("SymTab::GetSize: bad baseType %d", baseType);
    }
    return returnValue;
}
