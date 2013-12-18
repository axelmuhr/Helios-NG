#ifndef types_h
#define types_h

#include "aout.h"
#include "defs.h"
#include "errhandler.h"
#include "hash.h"
#include "strtable.h"
#include <stdio.h>
#include <sys/types.h>

static const int BUFFLEN = 4096;
static const int WORDSIZE = sizeof(int);
static const int BYTESIZE = sizeof(char);

static const int modSTSize = 1 << 16;

typedef enum {
    OTHER_FILE=0, OBJECT_FILE=1, LIBRARY_FILE=2
} FileType;

typedef enum {
    E_OPEN=7, E_HEADER, E_STRING, E_SYMTAB,
    E_RELOC, E_MULTIPLE, E_UNDEF
} ConvError;

typedef enum {
    UNDEF_SEG=0x0, ABS_SEG=0x2, TEXT_SEG=0x4,
    DATA_SEG=0x6, BSS_SEG=0x8, ANON_SEG=0x10,
    MISC_SEG=0x14, SYM_SEG=0x16, STR_SEG=0x17,
    ERROR_SEG=0x18
} SegmentType;

typedef enum { SKIM, DETAILED } DumpLevel;

extern StrTable *strTab;
extern HashTable* symTab;
extern HashTable* units;
extern FILE* dumpFile;

#endif
