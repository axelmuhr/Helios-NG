
/* Helios Linker   26/9/88      */

#ifdef __STDC__
#define NORCROFT 1
#endif

#ifdef NORCROFT
#define ANSI
#include <helios.h>
#endif

/* Target processors... */

#ifdef ARM
#define BYTESEX_EVEN
#endif

#ifdef PGC1
#define BYTESEX_EVEN
#endif

#ifdef M68K
#define BYTESEX_ODD
#endif

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include "vm.h"
extern VMRef VMNew(int size);

#ifdef ANSI
#define ellipsis ...
#else
#define ellipsis
#endif


#ifdef MWC
#define locase(c) (isupper(c)?_tolower(c):c)
#else
#define locase(c) (isupper(c)?tolower(c):c)
#endif

#define eqs(a,b) (strncmp(a,b,31)==0)

#ifndef __STDC__
extern word time(ellipsis);
#endif

#define HASHSIZE 31

typedef union {
   VMRef   v;
   word   w;
   byte   b[4];
} Value ;

/* Modules */

typedef struct STEntry {
   VMRef   head;
   int   entries;
}STEntry;

typedef struct Module {
        VMRef   next;              /* link in modlist              */
   VMRef   start;         /* code start and end...   */
   VMRef   end;
        WORD   id;         /* module table slot            */
        WORD   linked;         /* true if added to link   */
        STEntry   symtab[HASHSIZE];      /* module symbol table      */
} Module;

typedef struct Symbol {
   VMRef      next;
   VMRef      prev;
   word      type;
   word      global;
   VMRef      module;
   Value      value;
   char      name[Variable];
} Symbol;

typedef struct {
   byte      type;      /* code object type      */
   byte      size;      /* size in bytes      */
   byte      vtype;      /* value type         */
   word      loc;      /* offset in module      */
   Value      value;      /* value         */
} Code;

typedef struct {
   byte      type;      /* patch sub op         */
   word      word;      /* word value to patch      */
   Value      value;      /* value         */
} Patch;

#define t_code      0x01
#define t_bss      0x02
#define t_init      0x03

#define t_byte      0x09   /* ls 3 bits = size      */
#define t_short      0x0a
#define   t_word      0x0c

#define t_labelref  0x0f
#define t_dataref   0x10
#define t_datamod   0x11
#define t_modnum   0x12
#define t_patch      0x13
#define t_maxpatch   0x1f

#define   t_module   0x20
#define t_bytesex   0x21
 
#define t_global   0x22
#define   t_label      0x23
#define   t_data      0x24
#define   t_common   0x25

#define t_newseg   0x30
#define t_end      0x31
#define t_literal   0x32   /* t_code of <= 4 bytes */

#define s_unbound   0x40
#define s_codesymb   0x41
#define s_datasymb   0x42
#define s_commsymb   0x43
#define s_datadone   0x44
#define s_commdone   0x45

#define i_helios   'h'

/* traceing */

extern word traceflags;

#define db_gencode   0x008
#define db_genimage  0x010
#define db_files     0x400
#define db_mem        0x800
#define db_sym        0x1000
#define db_modules   0x2000
#define db_scancode  0x4000

/* externals */

extern void error(ellipsis);
extern void report(ellipsis);
extern void warn(ellipsis);
extern void initmem(ellipsis);
extern void initcode(ellipsis);
extern void initmodules(ellipsis);
extern void initsym(ellipsis);
extern void readfile(ellipsis);
extern void genend(ellipsis);
extern void genimage(ellipsis);
extern void objed(ellipsis);
extern word vmpagesize;
extern VMRef codeptr(ellipsis);
extern void _trace(ellipsis);
extern word codesize;
extern word heapsize;
extern word symsize;
extern VMRef curmod;
extern VMRef module0;
extern void cps(ellipsis);
extern void *alloc(ellipsis);
extern VMRef lookup(ellipsis);
extern VMRef insert(ellipsis);
extern FILE *infd;
extern FILE *outfd;
extern word hexopt;
extern void movesym(ellipsis);
extern VMRef firstmodule;
extern VMRef tailmodule;
extern word inlib;
extern void refsymbol(ellipsis);
extern VMRef newcode(ellipsis);
extern void scancode(ellipsis);
extern word curloc;
extern word totalcodesize;
extern void genimage(ellipsis);
