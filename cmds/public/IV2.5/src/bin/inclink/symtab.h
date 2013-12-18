/*  Interface to class SymTab
 */

#ifndef _symtab_h_
#define _symtab_h_

#include "aout.h"
#include "base.h"
#include "defs.h"
#include <stdio.h>


/*
 * The final location of a symbol table is:
 *     prog.header.SymOffset() + offset;
 *
 * For each nlist structure in the symbol table, the index into the output
 * string table is:
 *     prog.header.StrOffset() + sizeof(int) + strtabOffset + origIndex
 *
 *         where origIndex is the index into the string table of the input
 *         object file
 *
 * This index is finalized when the offset of the corresponding StrTab
 * is set and Relocate() is called;
 */

class SymTab {
    friend class StrTable;
public:
    SymTab(BaseName*);
    ~SymTab();

    void Reinit();            /* reinitialize the symbol table */
    void AddModName();        /* add module name */
    void Append(nlist*);      /* append a nlist to the buffer */
    int SetOffset(int);       /* set offset and return alloc */
    void Relocate(int);       /* relocs text values & string table indices */
    void WriteBuff(FILE*);    /* output to file */
    void SetDirty();          /* set to dirty */

private:
    void AllocBuff();         /* resize buff */
    int GetSize();            /* return size or alloc */

    nlist* buff;              /* symbol table */
    int size;                 /* filled size buff */
    int alloc;                /* potential capicity */
    int offset;               /* offset from start of program symbol table */
    boolean dirty;            /* disk copy is dated */
    BaseName* owner;          /* owner of this symbol table */
};

inline void SymTab::SetDirty () { dirty = true; }

#endif
