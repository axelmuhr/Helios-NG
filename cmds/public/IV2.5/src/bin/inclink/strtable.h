/* Interface to class StrTable
 *
 * The final location of a string table on disk is
 *     prog.header.StrOffset() + header.a_syms + offset
 */

#ifndef strtable_h
#define strtable_h

#include "defs.h"
#include <stdio.h>

static const int DEFAULTSTRINGTABLESIZE = 32768 - sizeof(int);
typedef int STindex;

class BaseName;
class SymTab;

class StrTable {
public:
    StrTable (const int  =DEFAULTSTRINGTABLESIZE);
    ~StrTable ();

    STindex AddString(const char*);
    STindex StrToIdx(const char* =0);
    const char* IdxToStr(int);
    const char* SafeIdxToStr(int);

    int SetOffset(int, SymTab*);  /* set offset and return alloc */
    void WriteBuff(FILE*);        /* output to file */
    void SetDirty();              /* set dirty to true */

private:
    char* strTable;
    int tableSize; 
    STindex lastChar;		  /* points to next free space */

    int offset;                   /* offset from program string table */
    boolean dirty;                /* disk copy is dated */
    SymTab* symtab;               /* corresponding symbol table, if any */

    const char* GetAddr (const int =-1);
    char* PrepareToInsert (const int len);
    STindex AppendBlock (const char*, const int len);
    int GetSize();
};

inline const char* StrTable::IdxToStr (int idx) { return &strTable[idx]; }
inline const char* StrTable::SafeIdxToStr (int idx) {
    return ((idx < lastChar) ? &strTable[idx] : nil);
}
inline void StrTable::SetDirty () { dirty = true; }

#endif
