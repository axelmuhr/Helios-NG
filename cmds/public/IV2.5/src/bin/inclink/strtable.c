/* 
 * String Table class 
 */

#include "defs.h"
#include "errhandler.h"
#include "module.h"
#include "program.h"
#include "strtable.h"
#include "symtab.h"
#include "system.h"
#include "types.h"
#include <string.h>

StrTable::StrTable (const int stsize) {
    strTable = new char[stsize];
    bzero(strTable, stsize);
    tableSize = stsize;
    lastChar = 0;
    offset = 0;
    dirty = false;
    symtab = nil;
}

StrTable::~StrTable () {
    delete strTable;
}

/* 
 * Called before a block insert of many strings, i.e. merging string tables.
 * len = size of block to insert. 
 */
char* StrTable::PrepareToInsert (const int len) {
    if (len <= 0) {
	Warning("StrTable::PrepareToInsert(%d)", len);
	return nil;
    }
    boolean expand = len + lastChar > tableSize;
    if (expand) {
        int needed = len + lastChar + sizeof(int);
        tableSize = 1;          // make tableSize + sizeof(int) a power of 2
        while (needed) {
            needed >>= 1;
            tableSize <<= 1;
        }
        tableSize -= sizeof(int);
        char* oldTable = strTable;
	strTable = new char[tableSize];
	bcopy(oldTable, strTable, lastChar);
        bzero(strTable + lastChar, tableSize - lastChar);
	delete oldTable;		/* old table is from heap */
    }
    int t = lastChar;
    lastChar += len;
    return &strTable[t];
}

STindex StrTable::AppendBlock (const char *str, const int len) {
    char *end;
    end = PrepareToInsert(len);
    strncpy(end, str, len);
    return lastChar;
}

/* return index into string table */
STindex StrTable::AddString (const char *str) {
    int len, t;
    char* here;
    
    t = lastChar;
    len = strlen(str) + 1;
    here = PrepareToInsert(len);
    strcpy(here,str);
    dirty = true;
    return t;
}

/* Get address of strTable[idx] */
const char* StrTable::GetAddr (const int idx) {
    if (idx == -1) {
	return &strTable[lastChar];
    } else if (idx <= lastChar && idx >= 0) {
	return &strTable[idx];
    } else {
	return nil;
    }
}

STindex StrTable::StrToIdx (const char *ptr) {
    STindex i;
    if (ptr == nil) {
	return lastChar;
    } else {
	i = (char*)ptr - &strTable[0];
	if (i >= lastChar || i < 0) {
	    return -1;
	} else {
	    return i;
	}
    }
}

int StrTable::SetOffset (int newOffset, SymTab* stab) {
    symtab = stab;
    if (symtab->owner->WhatAmI() == MODULE) {
        if (offset != newOffset) {
            dirty = true;
            int diff = newOffset - offset;
            offset = newOffset;
            symtab->Relocate(diff);
        }
    } else {
        if (offset != newOffset) {
            dirty = true;
            offset = newOffset;
        }
        if (dirty || symtab->dirty) {
            symtab->Relocate(offset);
        }
    }
    return GetSize();
}

void StrTable::WriteBuff (FILE* f) {
    if (dirty) {
        long foffset = currProg->SegPos(STR_SEG) + offset;
        if (ftell(f) != foffset) {
            fseek(f, foffset, 0);
        }
        ::WriteBuff(f, strTable, GetSize());
        if (Debug.StartMessage()) {
            Debug.Add("wrote strtab ");
            Debug.Add(IdxToStr(0));
            Debug.Add(" at ");
            Debug.Add((int)foffset);
            Debug.EndMessage();
        }
        dirty = false;
    }
}

int StrTable::GetSize () {
    int returnValue;
    int withSlop = min(int(lastChar * 1.1), tableSize);
    int baseType = symtab->owner->WhatAmI();
    switch (baseType) {
    case MODULE:
        if (((Module*)symtab->owner)->PartOfLib()) {
            returnValue = lastChar;
        } else {
            returnValue =  currProg->addSlop ? withSlop : lastChar;
        }
        break;
    case PROGRAM:
        returnValue = currProg->addSlop ? withSlop : lastChar;
        break;
    default:
        Panic("StrTable::GetSize: bad baseType %d", baseType);
    }
    return returnValue;
}
