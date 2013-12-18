/*
 * C compiler file dbx.c, version 5
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Codemist Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:07:18 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char dbx_version[];
char dbx_version[] = "\ndbx.c $Revision: 1.1 $ 5\n";
#endif

/* This file contains routines to buffer information required for         */
/* the Unix symbolic debugger dbx.  This has been hacked from c.armdbg,   */
/* which generated debugging tables for the Arthur symbolic debugger ASD. */

/*
 * Notes:
 *   1. RCC. Parameters are tricky. Currently a parameter in a register
 *      generates 2 dbx things, one a param, with its offset from fp,
 *      and the other a register variable.  This enables dbx to pick
 *      up the original value of the parameter when running down the
 *      the stack, and the current value (if anyone assigns to it) while
 *      inside the procedure.  But it is a bit of a tacky thing to do.
 */

#ifdef unix
#  include <a.out.h>
#  ifdef arm                 /* a.out.h defines size_t via sys/types.h */
#    define __size_t         /* so inhibit later definition by stddef.h */
#  endif
#else
#  include "aout.h"
#endif

#ifdef __STDC__
#  include <string.h>
#else
#  include <strings.h>
#endif
#include <stddef.h>

#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"
#include "store.h"
#include "codebuf.h"
#include "aeops.h"
#include "xrefs.h"
#include "util.h"
#include "regalloc.h"  /* for register_number */
#include "bind.h"      /* evaluate */
#include "sem.h"       /* alignoftype, sizeoftype, structfield */
#include "errors.h"
#ifdef FORTRAN
#include "feext.h"     /* syn_equivcheck */
#endif

#ifndef __STDC__
#  define sprintf ansi_sprintf
#endif

#ifdef TARGET_HAS_DEBUGGER

#define ACORN_DBX_VSN   315L

char dbg_name[4] = "dbx";
int usrdbgmask;

/*
 * Definitions of type values for dbx information in symbol table.
 * Really should get these from h.stab, but what the hell.
 */

#define N_GSYM  0x20            /* global symbol: name,,0,type,0 */
#define N_FNAME 0x22            /* procedure name (f77 kludge): name,,0 */
#define N_FUN   0x24            /* procedure: name,,0,linenumber,address */
#define N_STSYM 0x26            /* static symbol: name,,0,type,address */
#define N_LCSYM 0x28            /* .lcomm symbol: name,,0,type,address */
#define N_RSYM  0x40            /* register sym: name,,0,type,register */
#define N_SLINE 0x44            /* src line: 0,,0,linenumber,address */
#define N_SSYM  0x60            /* structure elt: name,,0,type,struct_offset */
#define N_SO    0x64            /* source file name: name,,0,0,address */
#define N_LSYM  0x80            /* local sym: name,,0,type,offset */
#define N_SOL   0x84            /* #included file name: name,,0,0,address */
#define N_PSYM  0xa0            /* parameter: name,,0,type,offset */
#define N_ENTRY 0xa4            /* alternate entry: name,linenumber,address */
#define N_LBRAC 0xc0            /* left bracket: 0,,0,nesting level,address */
#define N_RBRAC 0xe0            /* right bracket: 0,,0,nesting level,address */
#define N_BCOMM 0xe2            /* begin common: name,, */
#define N_ECOMM 0xe4            /* end common: name,, */
#define N_ECOML 0xe8            /* end common (local name): ,,address */
#define N_LENG  0xfe            /* second stab entry with length information */
#define N_PC    0x30            /* global pascal symbol: name,,0,subtype,line */
/*
 * The following values are copied from debugging information generated
 * by VAX-Unix PCC.  These types must all be declared explicitly (though
 * self-referentially).
 */

#ifndef FORTRAN /* ie is C */

#define T_INT     1
#define T_CHAR    2
#define T_LONG    3
#define T_SHORT   4
#define T_UCHAR   5
#define T_USHORT  6
#define T_ULONG   7
#define T_UINT    8
#define T_FLOAT   9
#define T_DOUBLE 10
#define T_VOID   11

#define T_BYTE     0
#define T_UBYTE    0
#define T_COMPLEX  0
#define T_DCOMPLEX 0
#define T_EXTEND   0
#define T_LBYTE    0
#define T_LSHORT   0
#define T_LOGICAL  0
#define T_LLONG    0

#else /* ie is F77 */

#define T_INT     3
#define T_CHAR    9
#define T_LONG    3
#define T_SHORT   2
#define T_UCHAR   9
#define T_USHORT  2
#define T_ULONG   3
#define T_UINT    3
#define T_FLOAT   4
#define T_DOUBLE  5
#define T_VOID   10

#define T_BYTE     1
#define T_UBYTE    1
#define T_COMPLEX  6
#define T_DCOMPLEX 7
#define T_EXTEND  13
#define T_LBYTE   11
#define T_LSHORT  12
#define T_LOGICAL  8
#define T_LLONG    8

#endif

static int32 tableindex[DT_MAX+1] = {
    0,
    T_BYTE,
    T_SHORT,
    T_INT,
    T_LONG,
    T_UBYTE,
    T_USHORT,
    T_UINT,
    T_ULONG,
    T_FLOAT,
    T_DOUBLE,
    T_EXTEND,
    T_COMPLEX,
    T_DCOMPLEX,
    T_LBYTE,
    T_LSHORT,
    T_LOGICAL,
    T_LLONG,
    T_CHAR,
    T_UCHAR
};

#define DbgAlloc(n) GlobAlloc(SU_Dbg, n)

static char *copy(char *s)
{   int l = strlen(s) + 1;
    /* Makes a heap copy of something in a local array to pass to
     * aoutobj (no longer in global store).
     */
    char *p = (char *)SynAlloc((int32)l);
    memcpy(p, s, l);
    return(p);
}

static char *globalcopy(char *s)
{   int l = strlen(s) + 1;
    /* Makes a global heap copy of something in a local array (for typereps)
     */
    char *p = (char *)DbgAlloc((int32)l);
    memcpy(p, s, l);
    return(p);
}

#define TYPESEGSIZE 256  /* type table segment size */
#define TYPESEGBITS   8  /* log2(MAXTYPES) */
#define TYPESEGMAX  256  /* maximum number of type table segments */

#define MAXSTRING 4096 /* size of temporary string buffers */

typedef struct TypeInfo {
    TypeExpr *type;
    char     *name;
    char     *defn;
} TypeInfo;

typedef TypeInfo *TypeSeg[TYPESEGSIZE];

static TypeSeg *(*dbg_typetab)[TYPESEGMAX];

#define typeinfo_(n) (*(*dbg_typetab)[((n)-1)>>TYPESEGBITS]) \
                                     [((n)-1)&(TYPESEGSIZE-1)]

static int32 dbg_freetype;

/*
 * The following definitions of the standard types are stolen from
 * decoded output of VAX pcc.  Don't ask me why there is a thing
 * called "???" as type 12.
 */

#ifndef FORTRAN  /* ie is C */

static TypeInfo dbg_stdtypes[] = {
    { NULL, "int",            "r1;-2147483648;2147483647;" },
    { NULL, "char",           "r2;-128;127;" },
    { NULL, "long",           "r1;-2147483648;2147483647;" },
    { NULL, "short",          "r1;-32768;32767;" },
    { NULL, "unsigned char",  "r1;0;255;" },
    { NULL, "unsigned short", "r1;0;65535;" },
    { NULL, "unsigned long",  "r1;0;-1;" },
    { NULL, "unsigned int",   "r1;0;-1;" },
    { NULL, "float",          "r1;4;0;" },
    { NULL, "double",         "r1;8;0;" },
    { NULL, "void",           "11" },
    { NULL, "???",            "1" },
    { NULL, NULL, NULL },
};

#else /* ie is fortran */

static TypeInfo dbg_stdtypes[] = {
    { NULL, "byte",           "r1;-128;127;" },
    { NULL, "integer*2",      "r2;-32768;32767;" },
    { NULL, "integer",        "r3;-2147483648;2147483647;" },
    { NULL, "real",           "r4;4;0;" },
    { NULL, "double precision", "r5;8;0;" },
    { NULL, "complex",        "r6;8;0;" },
    { NULL, "double complex", "r7;16;0;" },
    { NULL, "logical",        "r8;4;0;" },
    { NULL, "char",           "r9;-128;127;" },
    { NULL, "void",           "r10;0;0;" },
    { NULL, "logical*1",      "r11;1;0;" },
    { NULL, "logical*2",      "r12;2;0;" },
    { NULL, "real*16",        "r13;16;0;" },
    { NULL, NULL, NULL },
};

#endif

static void dbg_stab(char *name, int32 type, int32 desc,
                     unsigned long value)
{
    struct nlist nlist;
    nlist.n_un.n_name = name;
    nlist.n_type      = (char)type;
    nlist.n_other     = (char)0;
    nlist.n_desc      = (short)desc;
    nlist.n_value     = value;
    obj_stabentry(&nlist);
}

static void dbg_stdheader(void)
{   TypeInfo *p;
    char buf[64];
    int idx, pos;
    for (p = dbg_stdtypes, idx = 1; p->name != NULL; ++p) {
       pos = sprintf(buf, "%s:t%d=%s", p->name, idx++, p->defn);
       buf[pos] = 0;
       dbg_stab(copy(buf), N_LSYM, 0, 0);
    }
}

#define newtypeseg_() (TypeSeg *)DbgAlloc(TYPESEGSIZE * sizeof(TypeInfo *))

static void dbg_inittypes(void)
{   TypeInfo *p;
    TypeSeg *q;
    int32 i;

    if (!usrdbg(DBG_ANY)) { dbg_typetab = (TypeSeg *(*)[])DUFF_ADDR; return; }

    dbg_typetab = (TypeSeg *(*)[])DbgAlloc(sizeof(*dbg_typetab));
    for (i = 0; i < TYPESEGMAX; i++) (*dbg_typetab)[i] = NULL;
    q = (*dbg_typetab)[0] = newtypeseg_();
    for (p = &dbg_stdtypes[0], i = 0; p->name != NULL; ++p) { (*q)[i++] = p; }
    (*q)[i] = NULL; dbg_freetype = i;
}

static int32 dbg_typeintab(char *name, TypeExpr *t)
{
    if (h0_(t) == s_typespec) { /* may be a built-in one */
        SET_BITMAP m = typespecmap_(t);
        int32 ubit = m & bitoftype_(s_unsigned);
        int32 langinfo = typedbginfo_(t);
        if (langinfo != 0) return langinfo;
        switch (m & -m) {
        case bitoftype_(s_char):
            return(ubit ? T_UCHAR : T_CHAR);
        case bitoftype_(s_int):
            if (m & bitoftype_(s_short)) {
                return(ubit ? T_USHORT : T_SHORT);
            }
            return(ubit ? T_UINT : T_INT);
        case bitoftype_(s_double):
            return((m & bitoftype_(s_short)) ? T_FLOAT : T_DOUBLE);
        case bitoftype_(s_void):
            return(T_VOID);
        default:
            break;
        }
    }
    /*
     * Not built-in: search table for it.
     */
    {   int32 i;
        for (i = 0; i < TYPESEGMAX; i++) {
            TypeSeg *q = (*dbg_typetab)[i];
            int32 j;
            if (q == NULL) return 0;
            for (j = 0; j < TYPESEGSIZE; j++) {
                TypeInfo *p = (*q)[j];
                if (p == NULL) return 0;
                if (p->type == t && (name[0] == '\0' || strcmp(p->name,name) == 0))
                    return(i * TYPESEGMAX + j + 1);
            }
        }
    }
    return(0);
}

static int32 dbg_newtype(TypeExpr *type, char *name, char *defn)
{   TypeInfo *p;
    if (++dbg_freetype > TYPESEGSIZE * TYPESEGMAX)
        syserr(syserr_too_many_types);
    p = (TypeInfo *)DbgAlloc(sizeof(TypeInfo));
    typeinfo_(dbg_freetype) = p;
    p->type = type;
    p->name = name;
    p->defn = defn;
    if ((dbg_freetype & (TYPESEGSIZE-1)) == 0)
        (*dbg_typetab)[dbg_freetype >> TYPESEGBITS] = newtypeseg_();
    typeinfo_(dbg_freetype+1) = NULL;
    return(dbg_freetype);
}

typedef int VarClass;

#define CLASS_AUTO     0
#define CLASS_EXT      1
#define CLASS_STATIC   2
#define CLASS_REG      3
#define CLASS_OWN      4
#define CLASS_PARAM    5
#define CLASS_VARPARAM 6
#define CLASS_COMMMEM  7
#define CLASS_BSS      8
#define CLASS_EXTBSS   9

static char charofclass[] = "@GSrVpvVSG";

static char ntypeof[] = {
    N_LSYM, N_GSYM, N_STSYM, N_RSYM, N_STSYM, N_PSYM, N_PSYM, N_GSYM,
    N_LCSYM,N_LCSYM,
    0
};

/* flag values for 'debsort' */
#define D_SRCPOS      1
#define D_PROC        2
#define D_VAR         3
#define D_TYPE        4
#define D_STRUCT      5
#define D_FILEINFO    6
#define D_SCOPE       7
#define D_COMMON      8

#define dbg_hdr_(debsort,len) ((int32)(len)<<16 | (debsort))
/* The next macro seems to indicate a lack to do this portably in ANSI-C */
/* Are discriminated unions second class objects?                        */
#define DbgListVarSize(variant) \
    ((size_t)(sizeof(p->car.variant)+offsetof(DbgList,car)))

/* The following is the *internal* data structure in which debug info. */
/* is buffered.                                                        */
typedef struct DbgList
{   struct DbgList *cdr;
    int32 debsort;
    union Deb_Things
    {  struct { char *fname;
                int32 sourcepos;
                int32 codeaddr;
              } DEB_SRCPOS;
       struct { char *type;
                int32 isextern;
                int32 sourcepos;
                int32 codeaddr;
                char *name;
              } DEB_PROC;
       struct { char *type;
                int32 sourcepos;
                VarClass dbxclass;
                int32 location;
                Symstr *sym;
              } DEB_VAR;
       struct { int32 typeidx;
              } DEB_TYPE;
       struct { int32 typeidx;
              } DEB_STRUCT;
       struct { int32 levelchange;
                int32 level;
                int32 codeaddr;
              } DEB_SCOPE;
       struct { int32 isstart;
                char *name;
              } DEB_COMMON;
    } car;
} DbgList;

static DbgList *dbglist, *dbglistproc;

static DbgList *dbgscope; /* used only briefly */

int32 dbg_tablesize()
{
  return 0;  /* harmless, but should raise an error ?? */
}

int32 dbg_tableindex(int32 dt_number)
{
    if (dt_number > DT_MAX) return 0;
    return tableindex[dt_number];
}

VoidStar dbg_notefileline(FileLine fl)
{
    DbgList *p;

    if (!usrdbg(DBG_LINE)) return(DUFF_ADDR);
    if (debugging(DEBUG_Q)) cc_msg("-- noteline(%ld)\n", fl.l & 0x3fffff);
    p = (DbgList *)DbgAlloc(DbgListVarSize(DEB_SRCPOS));
    p->cdr = dbglist; dbglist = p;
    p->debsort = D_SRCPOS;
    p->car.DEB_SRCPOS.fname     = fl.f;
    p->car.DEB_SRCPOS.sourcepos = fl.l;
    p->car.DEB_SRCPOS.codeaddr  = -1; /* to be patched later */
    return((VoidStar)p);
}

/*
 * Bits 0..21 of a sourcepos are the lineno, bits 22..31 are character
 * position on line: this is the wrong way round for comparing them,
 * but that's the way TopExpress defined it, so have to rotate before
 * comparing.
 */
#define ROTPOS(srcpos) (((srcpos)<<10) | ((srcpos)>>22))

/*
 * dbg_insertitem is needed to put things into the dbglist at a
 * place determined by source order or code order, rather than always
 * tacking them on the beginning of the list (which is reversed later).
 */

static void dbg_insertitem(DbgList *p, int32 srcpos, int32 codeaddr)
{   int32   rpos = ROTPOS(srcpos);
    DbgList *prev, *x;

    if (debugging(DEBUG_Q)) cc_msg("-- insert(line=%ld)\n", srcpos & 0x3fffff);
    for (prev = NULL, x = dbglist; x != NULL; x = x->cdr) {
        if (x->debsort != D_SRCPOS) continue;
        if (srcpos == 0) {
            int32 xaddr = x->car.DEB_SRCPOS.codeaddr;
            if ((xaddr >= 0) && (xaddr <= codeaddr)) break;
        } else {
            if (ROTPOS(x->car.DEB_SRCPOS.sourcepos) <= rpos) break;
        }
        prev = x;
    }
    if (srcpos == 0) { /* no point adding a D_SRCPOS record */
        x = p;
    } else {
        x = (DbgList *)DbgAlloc(DbgListVarSize(DEB_SRCPOS));
        x->debsort = D_SRCPOS;
        x->car.DEB_SRCPOS.fname     = NULL; /* dubious? */
        x->car.DEB_SRCPOS.sourcepos = srcpos;
        x->car.DEB_SRCPOS.codeaddr  = -1;
        p->cdr = x;
    }
    if (prev != NULL) {
        x->cdr = prev->cdr; prev->cdr = p;
    } else {
        x->cdr = dbglist; dbglist = p;
    }
}

void dbg_addcodep(VoidStar debaddr, int32 codeaddr)
{
    DbgList *p = (DbgList *)debaddr;

    if (!usrdbg(DBG_ANY)) return;

    if (p == NULL) { /* J_INFOSCOPE */
        /*
         * c.flowgraf outputs a J_INFOSCOPE immediately after calling
         * dbg_scope, to mark the relevant code address.
         */
        if (debugging(DEBUG_Q)) cc_msg("-- scope at 0x%lx\n", codeaddr);
        if ((p = dbgscope) != NULL) {
            dbgscope = NULL; /* mustn't leave it lying around... */
            p->car.DEB_SCOPE.codeaddr = codeaddr;
            dbg_insertitem(p, 0, codeaddr);
        }
    } else {         /* J_INFOLINE */
        if (p->debsort != D_SRCPOS) syserr(syserr_addcodep);
        if (p->car.DEB_SRCPOS.codeaddr == -1) {
            p->car.DEB_SRCPOS.codeaddr = codeaddr;
        }
    }
}

/* End of file/line co-ordinate code */

static int32 sprinttype(char *name,char *, TypeExpr *);

static int32 sprintstruct(char *pp, TypeExpr *type)
{
    char   *start = pp;
    TagBinder  *b = typespectagbind_(type);
    Symstr *sym;
    AEop sort;

    /* do two passes to cope with cycles */
    sym = bindsym_(b);
    if (b->tagbinddbg > 0) {
        pp += sprintf(pp, "%ld", b->tagbinddbg); *pp = 0;
        return(pp - start);    /* seen before */
    }
    if (!(tagbindstate_(b) & TB_DEFD)) { /* no definition yet */
        b->tagbinddbg = -dbg_newtype(type, symname_(sym), NULL);
        pp += sprintf(pp, "%ld", -b->tagbinddbg); *pp = 0;
        return(pp - start);
    }
    if (b->tagbinddbg < 0) { /* it has become defined */
        b->tagbinddbg = -b->tagbinddbg;
    } else {
        b->tagbinddbg = dbg_newtype(type, symname_(sym), NULL);
    }
    /*
     * Beware circular types and anonymous types.
     */
    if (!isgensym(sym)) { /* not anonymous */
        DbgList *p = (DbgList*)DbgAlloc(DbgListVarSize(DEB_STRUCT));
        pp += sprintf(pp, "T%ld=", b->tagbinddbg);
        p->debsort = D_STRUCT;
        p->car.DEB_STRUCT.typeidx = b->tagbinddbg;
        p->cdr = dbglist;
        dbglist = p;
    } else {
        pp += sprintf(pp, "%ld=", b->tagbinddbg);
    }
    {   char dbxclass = 0;
        switch (sort = tagbindsort_(b)) {
            case s_struct: dbxclass = 's'; break;
            case s_class:  dbxclass = 's'; break;
            case s_union:  dbxclass = 'u'; break;
            case s_enum:   dbxclass = 'e'; break;
            default:       syserr(syserr_tagbindsort,
                                  (long)tagbindsort_(b));
        }
        *pp++ = dbxclass;
        if (dbxclass != 'e') pp += sprintf(pp, "%ld", sizeoftype(type));
    }
    {   StructPos p;
        TagMemList *l;
        p.n = p.bitoff = 0;
        for (l = tagbindmems_(b); l != 0; l = l->memcdr) {
            if (sort != s_enum) structfield(l, sort, &p);
            if (l->memsv) {
                /* note that memsv is 0 for padding bit fields */
                pp += sprintf(pp, "%s:", symname_(l->memsv));
                if (sort == s_enum) {
                /* symdata_ here is ok, since dbg_typerep gets called for local
                   things when a declaration is first seen (by dbg_locvar)
                   at which time bindings are still in place.
                 */
                    pp += sprintf(pp, "%ld", bindaddr_(symdata_(l->memsv)));
                    *pp++ = ',';
                } else {
                    pp += sprinttype("", pp, l->memtype); *pp++ = ',';
                    pp += sprintf(pp, "%ld,%ld;",
                                      p.woffset*8 + p.boffset,
                                      p.bsize != 0 ? p.bsize :
                                                     p.typesize * 8);
                }
            }
        }
    }
    *pp++ = ';'; *pp = 0;
    if (!isgensym(sym)) { /* not anonymous */
        typeinfo_(b->tagbinddbg)->defn = globalcopy(start);
        pp = start; pp += sprintf(pp, "%ld", b->tagbinddbg); *pp = 0;
    }
    return(pp - start);
}

static int32 sprinttype(char *name, char *buf, TypeExpr *x)
{   char *p = buf;
    int32 typeidx = dbg_typeintab(name,x);
    int32 n;
    if (typeidx != 0) {
        TypeInfo *t = typeinfo_(typeidx);
        if (t->name[0] == '<' && t->defn != NULL) { /* anonymous structure */
            p += sprintf(p, "%s", t->defn);
        } else {
            p += sprintf(p, "%ld", typeidx);
        }
        *p = 0; return(p - buf);
    }
    switch (h0_(x)) {
    case t_content:
    case t_ref:                 /* @@@ ok?  Or f77 'v'? */
#ifndef FORTRAN /* dbx doesn't understand f77 pointers, & has special type 'v'*/
        *p++ = '*';
#endif
        p += sprinttype("", p, typearg_(x));
        break;
    case t_subscript:
        *p++ = 'a';
        n = typesubsize_(x) ? evaluate(typesubsize_(x)) : 1;
        p += sprintf(p, "r%d;0;%ld", T_INT, n-1); *p++ = ';';
        p += sprinttype("", p, typearg_(x));
        break;
    case t_fnap:
        *p++ = 'f'; p += sprinttype("", p, typearg_(x));
        break;
    case s_typespec:
        {   SET_BITMAP m = typespecmap_(x);
            switch (m & -m) {
            case bitoftype_(s_enum):
            case bitoftype_(s_struct):
            case bitoftype_(s_class):
            case bitoftype_(s_union):
                p += sprintstruct(p, x);
                break;
            case bitoftype_(s_typedefname):
                p += sprinttype("", p, bindtype_(typespecbind_(x)));
                break;
            default:
                syserr(syserr_sprinttype,
                       (VoidStar )x, (long)typespecmap_(x));
                break;
            }
        }
        break;
    default:
        syserr(syserr_sprinttype, (VoidStar)x, (long)typespecmap_(x));
        break;
    }
    *p = 0;
    return(p - buf);
}

static char *dbg_typerep(TypeExpr *x)
{
    char buf[MAXSTRING];
    sprinttype("", buf, x);
    return globalcopy(buf);
}

static void dbg_addvar_t(Symstr *name, char *typerep, int32 sourcepos,
                         VarClass dbxclass, int32 addr)
{   DbgList *p = (DbgList*)DbgAlloc(DbgListVarSize(DEB_VAR));
    p->debsort = D_VAR;
    p->car.DEB_VAR.type = typerep;
    p->car.DEB_VAR.sourcepos = sourcepos;
    p->car.DEB_VAR.dbxclass = dbxclass;
    p->car.DEB_VAR.location = addr;
    p->car.DEB_VAR.sym = name;
    dbg_insertitem(p, sourcepos, -1);
}

static void dbg_addvar(Symstr *name, TypeExpr *t, int32 sourcepos,
                       VarClass dbxclass, int32 addr)
{
    dbg_addvar_t(name, dbg_typerep(t), sourcepos, dbxclass, addr);
}

static void dbg_addcomm(int32 isstart, char *name, int32 sourcepos)
{   DbgList *p = (DbgList*)DbgAlloc(DbgListVarSize(DEB_COMMON));
    p->debsort = D_COMMON;
    p->car.DEB_COMMON.isstart = isstart;
    p->car.DEB_COMMON.name = name;
    dbg_insertitem(p, sourcepos, -1);
}

static char stgclasses[] = {CLASS_STATIC, CLASS_EXT, CLASS_BSS, CLASS_EXTBSS};

void dbg_topvar(Symstr *name, int32 addr, TypeExpr *t, int stgclass,
                FileLine fl)
/* For scoping reasons this only gets called on top-level variables (which */
/* are known to be held in global store).  (Does this matter?)             */
{
    if (usrdbg(DBG_PROC))
    {   DbgList *p, **prev = &dbglist;
        if (debugging(DEBUG_Q))
            cc_msg("top var $r @ %d:%.6lx\n", name, stgclass, (long)addr);
        for (; (p = *prev) != NULL ; prev = &cdr_(p)) {
            int oldclass = p->car.DEB_VAR.dbxclass;
            if (p->debsort==D_VAR &&
                ( oldclass == CLASS_EXT || oldclass == CLASS_STATIC ||
                  oldclass == CLASS_BSS || oldclass == CLASS_EXTBSS) &&
                p->car.DEB_VAR.location==0 &&
                p->car.DEB_VAR.sym == name) {
                /* Remove the superceded declaration */
                *prev = cdr_(p);
                break;
            }
        }
        dbg_addvar(name, t, fl.l, stgclasses[stgclass], addr);
    }
}

void dbg_type(Symstr *name, TypeExpr *t)
/*
 * This does NOT only get called on top-level types (which
 * are known to be held in global store).  (So: globalize_typeexpr()).
 * Now called also from dbg_locvar1() and rd_decl().
 */
{
    if (usrdbg(DBG_ANY))
    {   char buf[MAXSTRING];
        char *pp = buf;
        char *s = symname_(name);

        if (debugging(DEBUG_Q)) cc_msg("type $r\n", name);
        pp += sprinttype(s, pp, t); *pp = 0;
        if (!isgensym(name))
        { /* which means that anonymous enums will get no type entry, though
             the members may be used.  But this is just what PCC does */
            DbgList *p = (DbgList*)DbgAlloc(DbgListVarSize(DEB_TYPE));
            p->debsort = D_TYPE;
            p->car.DEB_TYPE.typeidx = dbg_newtype(t, s, globalcopy(buf));
            p->cdr = dbglist;              /* do this last (typerep above) */
            dbglist = p;
        }
    }
}

typedef struct Dbg_LocList
{   struct Dbg_LocList *cdr;
    Binder *name;
    char *typerep;
    int32 pos;
    bool processed;
} Dbg_LocList;

static Dbg_LocList *dbg_loclist;

/*
 * dbg_locvar() only registers the name and line of a declaration --
 * the location info cannot be added until after register allocation.
 * See also dbg_scope which completes.
 * Also remember that dead code elimination may remove some decls.
 */

void dbg_locvar(Binder *name, FileLine fl)
{
    if (usrdbg(DBG_VAR) && !isgensym(bindsym_(name))) {
        TypeExpr *t = bindtype_(name);
        if (debugging(DEBUG_Q)) cc_msg("-- locvar(%s)\n", symname_(bindsym_(name)));
        if (bindstg_(name) & bitofstg_(s_typedef))
            dbg_type(bindsym_(name), t);
        else {
        /* local to a proc */
            Dbg_LocList *p = (Dbg_LocList*)DbgAlloc(sizeof(Dbg_LocList));
            char *typerep = dbg_typerep(t);
            if (debugging(DEBUG_Q)) cc_msg("note loc var $b [%s]\n", name, typerep);
            p->cdr  = dbg_loclist;
            p->name = name;
            p->pos  = fl.l;
            p->typerep = typerep;
            p->processed = NO;
            dbg_loclist = p;
        }
    }
}

static Dbg_LocList *dbg_lookupline(Binder *b)
{   Dbg_LocList *p;
    for (p = dbg_loclist; p; p = p->cdr) if (p->name == b) return p;
    return NULL;
}

/* dbg_locvar1 is called when the location for a declaration is known. By this
   time, things allocated in local storage have evaporated (in particular,
   bindtype_(b). And the Symstrs for most gensyms.
 */

void dbg_locvar1(Binder *b)
{   Symstr *name = b->bindsym;
    Dbg_LocList *loc = dbg_lookupline(b);
    VarClass dbxclass;
    int32 addr = bindaddr_(b);
    if (debugging(DEBUG_Q)) cc_msg("-- locvar1(%s)", symname_(name));
    if (loc == NULL)
    {   if (debugging(DEBUG_Q)) cc_msg(" omitted\n");
        return;   /* invented variable name (e.g. s_let) */
    }
    loc->processed = YES;
    switch (bindstg_(b) & PRINCSTGBITS)
    {
default:
defolt:
        syserr(syserr_dbx_locvar,
               name, (long)bindstg_(b), (long)addr);
        return;
case bitofstg_(s_typedef):
        return;                   /* typedef already done in locvar */
case bitofstg_(s_extern):
        if (debugging(DEBUG_Q)) cc_msg(" <extern>\n");
        return;                   /* local externs do not allocate store */
case bitofstg_(s_static):
        dbxclass = CLASS_OWN;
        break;
case bitofstg_(s_auto):
#ifndef FORTRAN
        if (bindxx_(b) != GAP) {
            if ((addr & BINDADDR_MASK) == BINDADDR_ARG) {
                /*
                 * An arg which is mostly in a register: persuade dbx
                 * that there are 2 things with same name, first an arg
                 * and second a register variable.  This depends on the
                 * later definition taking precedence in dbx.
                 */
                dbg_addvar_t(name, loc->typerep, loc->pos,
                             CLASS_PARAM, local_fpaddress(addr));
                loc->pos += (1L << 22); /* one char later... */
            }
            dbxclass = CLASS_REG;
            addr = register_number(bindxx_(b));
        }
        else /* continues below... */
#endif
             switch (addr & BINDADDR_MASK) {
            case BINDADDR_ARG:
#ifdef FORTRAN /* dbx doesn't understand f77 pointers, so has special type */
                dbxclass = CLASS_VARPARAM;
#else
                dbxclass = CLASS_PARAM;
#endif
                addr = local_fpaddress(addr);
                break;
            case BINDADDR_LOC:
                dbxclass = CLASS_AUTO;
                addr = local_fpaddress(addr);
                break;
            default:
                goto defolt;
        }
        break;
    }
    if (debugging(DEBUG_Q))
        cc_msg(" %c %lx", charofclass[dbxclass], (long)addr);
    dbg_addvar_t(name, loc->typerep, loc->pos, dbxclass, addr);
}

/*
 * The code here is written is this manner, so that for *significant*
 * effort, we could save variable (code) extent information for
 * a debugger.  Consider f() { { int x; ... } { int y; ... } }.
 */

bool dbg_scope(BindListList *newbll, BindListList *oldbll)
{   int32 oldlevel = length((List *)oldbll);
    int32 newlevel = length((List *)newbll);
    int32 entering = newlevel - oldlevel;
    int   nvars;

    if (!usrdbg(DBG_VAR)) return NO;
    if (newbll == oldbll) return NO;
    if (debugging(DEBUG_Q)) cc_msg("-- dbg_scope(entering=%ld)\n", entering);
    if (entering < 0)
    {   BindListList *t = newbll;
        newbll = oldbll, oldbll = t;
    }
    /*
     * Patch all the local variable declarations back into the dbglist
     * at the appropriate position with respect to the D_SRCPOS items.
     */
    for (nvars = 0; newbll != oldbll; newbll = newbll->bllcdr)
    {   SynBindList *bl;
        if (newbll == 0) syserr(syserr_dbx_scope);
        for (bl = newbll->bllcar; bl; bl = bl->bindlistcdr)
        {   Binder *b = bl->bindlistcar;
            if (bindstg_(b) & b_dbgbit) continue;
            if (debugging(DEBUG_Q))
                cc_msg("  %s $b",
                        entering>=0 ? "binding" : "unbinding",
                        b);
            ++nvars;
            if (entering < 0) { dbg_locvar1(b); bindstg_(b) |= b_dbgbit; }
        }
    }
    {
        DbgList *p = (DbgList *)DbgAlloc(DbgListVarSize(DEB_SCOPE));
        dbgscope = p;  /* it is added to dbglist soon by addcodep(NULL, ...) */
        p->cdr = NULL; /* for safety ... */
        p->debsort = D_SCOPE;
        p->car.DEB_SCOPE.levelchange = entering;
        p->car.DEB_SCOPE.level       = oldlevel;
        p->car.DEB_SCOPE.codeaddr    = -1;
    }
    /*
     * Return YES here to ask the local code-generator
     * to notify us the code location of where these declarations start/end,
     * resulting in a call to dbg_addcodep(NULL, codeaddr).
     */
    return YES;
}

void dbg_proc(Symstr *name, TypeExpr *t, bool ext, FileLine fl)
{
    if (debugging(DEBUG_Q)) cc_msg("-- proc(%s)\n", symname_(name));
    if (usrdbg(DBG_ANY))
    {   DbgList *p = (DbgList*)DbgAlloc(DbgListVarSize(DEB_PROC));
        if (debugging(DEBUG_Q)) cc_msg("startproc $r\n", name);
        p->debsort = D_PROC;
        if (h0_(t) != t_fnap) syserr(syserr_dbx_proc);
        p->car.DEB_PROC.type = dbg_typerep(typearg_(t));
        p->car.DEB_PROC.isextern = ext;
        /*
         * Don't use DEB_PROC.args currently, so I am expunging it to
         * save space.
         *
           p->car.DEB_PROC.args = length((List *)typefnargs_(t));
         */
        p->car.DEB_PROC.sourcepos = fl.l;
        p->car.DEB_PROC.codeaddr = 0;        /* fill in at dbg_enterproc */
        p->car.DEB_PROC.name = symname_(name);
#ifdef FORTRAN
        dbglistproc = p;
        dbg_insertitem(p, fl.l, -1);
#else /* (not f77) - above may not do any harm, but to be careful: */
        p->cdr = dbglist;              /* do this last (typerep above) */
        dbglistproc = dbglist = p;       /* so can be filled in */
#endif
    }
    dbg_loclist = 0;
}

void dbg_enterproc(void)
{
    if (debugging(DEBUG_Q)) cc_msg("-- enterproc(%d)\n", 0);
    if (usrdbg(DBG_ANY))
    {   DbgList *p = dbglistproc;
        if (p == 0 || p->debsort != D_PROC || p->car.DEB_PROC.codeaddr != 0)
            syserr(syserr_dbx_proc1);
        if (debugging(DEBUG_Q))
            cc_msg("enter '%s' @ %.6lx\n",
                    p->car.DEB_PROC.name, (long)codebase);
        p->car.DEB_PROC.codeaddr = codebase;
    }
}

void dbg_bodyproc(void)
{
    /* ignored by dbx */
}

void dbg_return(int32 addr)
{
    /* ignored by dbx */
    addr = addr;
}

void dbg_xendproc(FileLine fl)
{
#ifdef FORTRAN
    Dbg_LocList *p = dbg_loclist;
    for (; p != NULL; p = p->cdr)
        if (!(p->processed)) syn_equivcheck(p->name);
#endif
    if (debugging(DEBUG_Q)) cc_msg("-- endproc()\n");
    dbg_notefileline(fl);
    dbglistproc = 0;
}

void dbg_commblock(Binder *b, SynBindList *members, FileLine fl)
{
    char buf[64], *name = symname_(bindsym_(b));
    if (members == NULL) return;
    sprintf(buf, "%s_", ++name); /* lose starting '/', add trailing '_' */
    name = copy(buf);
    dbg_addcomm(1, name, fl.l);
    for (; members != NULL; members = members->bindlistcdr)
    {
        Binder *m = members->bindlistcar;
        dbg_addvar(bindsym_(m), bindtype_(m), fl.l, CLASS_COMMMEM,bindaddr_(m));
    }
    dbg_addcomm(0, name, fl.l);
}

#ifdef COMPILING_ON_RISC_OS
/*
 * Dbx looks for the .c .f .h extension on filenames to decide what
 * language was used, so we have to fiddle with filenames a little.
 * The code below will not work very well, but cross-compiling on
 * Arthur to produce an a.out/dbx object is useful only in the
 * short term for testing. RCC.
 */
static char xbuf[256];

static char *frigfname(char *name)
{
    if (name[1] == '.') {
        char t = name[0];
        if ((t == 'c') || (t == 'h') || (t == 's')) {
            sprintf(xbuf, "%s.%c%c", name+2, t, 0);
            return(xbuf);
        }
    }
    return(name);
}
#else
#define frigfname(name) (name)
#endif

/*
 * The object formatter calls writedebug to generate the debugging tables.
 * It must call obj_stab to put the appropriate things in sym table.
 */

void dbg_writedebug()
{   DbgList *p;
    char  *curfile = sourcefile;
    int32 curline = 0;
    int32 curcode = -1;
    char buf[MAXSTRING];          /* to build names in */

    if (!usrdbg(DBG_ANY)) return;

#define sameline(a, b) ((a)<<10 == (b)<<10)
#define ADDRMASK 0x3fffffc
#define setpos(x,c) \
if ((c)>=0 && !sameline((x),curline)) \
    curline=(x), curcode=(c), dbg_stab(0, N_SLINE, curline, curcode&ADDRMASK)

    if (debugging(DEBUG_Q)) cc_msg("-- dbg_flush()\n");

    dbg_stab(frigfname(curfile), N_SO, ACORN_DBX_VSN, 0);
    dbg_stdheader();

    for (p = (DbgList *)dreverse((List *)dbglist); p != NULL; p = p->cdr)
    {   int32 sort = p->debsort;
        switch (sort)
        {
    default:
            syserr(syserr_dbx_write, (long)sort);
            break;
    case D_SRCPOS:
            {   int32 codeaddr = p->car.DEB_SRCPOS.codeaddr;
                char *fname    = p->car.DEB_SRCPOS.fname;
                if (fname == NULL) fname = curfile;
                if (fname != curfile) {
                    curfile = p->car.DEB_SRCPOS.fname;
                    curline = -1;
                    if (codeaddr < 0) codeaddr = curcode;
                    dbg_stab(frigfname(curfile), N_SOL, 0, codeaddr);
                }
                setpos(p->car.DEB_SRCPOS.sourcepos, codeaddr);
            }
            break;
    case D_PROC:
#ifndef FORTRAN /* ie is C */
            {   sprintf(buf, "%s:%c%s",
                    p->car.DEB_PROC.name,
                    p->car.DEB_PROC.isextern ? 'F' : 'f',
                    p->car.DEB_PROC.type);
                dbg_stab(copy(buf), N_FUN,
                    p->car.DEB_PROC.sourcepos,
                    p->car.DEB_PROC.codeaddr & ADDRMASK);
            }
#else /* ie is F77 */
            {   char *tail;
                strcpy(buf, p->car.DEB_PROC.name);
                tail = buf + strlen(buf) - 1;
                if (*tail != '_') tail++;
                sprintf(tail, ":%c%s",
                    p->car.DEB_PROC.isextern ? 'F' : 'f',
                    p->car.DEB_PROC.type);
                dbg_stab(copy(buf), N_FUN,
                    p->car.DEB_PROC.sourcepos,
                    p->car.DEB_PROC.codeaddr & ADDRMASK);
            }
#endif
            break;
    case D_SCOPE:
            {   int32 d     = p->car.DEB_SCOPE.levelchange;
                int32 level = p->car.DEB_SCOPE.level;
                int32 addr  = p->car.DEB_SCOPE.codeaddr;
                int32 final;

                final = level + d;
                if (d < 0) { --level; --final; d = -1; } else { d = 1; }
                for (; level != final; level += d) {
                    if (level > 1) {
                        dbg_stab(NULL, d > 0 ? N_LBRAC:N_RBRAC, level, addr);
                    }
                }
            }
            break;
    case D_VAR:
            {   char *pp = buf;
                int  ntype;
                Symstr *sym = p->car.DEB_VAR.sym;
                setpos(p->car.DEB_VAR.sourcepos, curcode);
                pp += sprintf(pp, "%s:", symname_(sym));
                if (p->car.DEB_VAR.dbxclass != CLASS_AUTO) {
                    *pp++ = charofclass[p->car.DEB_VAR.dbxclass];
                }
                pp += sprintf(pp, "%s", p->car.DEB_VAR.type);
                ntype = (int)ntypeof[p->car.DEB_VAR.dbxclass];
                *pp = 0;
                {   ExtRef *x = symext_(sym);
                    int32 loc = p->car.DEB_VAR.location;
                    if (x != NULL && (x->extflags & xr_bss) &&
                        (x->extflags & (xr_defloc | xr_defext)) &&
                        x->extoffset != 0)
                        loc = x->extoffset;
                    dbg_stab(copy(buf), ntype, 0, loc);
                }
            }
            break;
    case D_COMMON:
            dbg_stab(p->car.DEB_COMMON.name,
                    (p->car.DEB_COMMON.isstart == 0) ? N_ECOMM : N_BCOMM, 0, 0);
            break;
    case D_TYPE:
            {   TypeInfo *t = typeinfo_(p->car.DEB_TYPE.typeidx);
                if (debugging(DEBUG_Q))
                    cc_msg("-- D_TYPE %ld\n", p->car.DEB_TYPE.typeidx);
                sprintf(buf, "%s:t%ld=%s",
                    t->name, p->car.DEB_TYPE.typeidx, t->defn);
                dbg_stab(copy(buf), N_LSYM, 0, 0);
            }
            break;
    case D_STRUCT:
            {   TypeInfo *t = typeinfo_(p->car.DEB_STRUCT.typeidx);
                if (debugging(DEBUG_Q)) cc_msg("-- D_STRUCT %s\n", t->name);
                sprintf(buf, "%s:%s", t->name, t->defn);
                dbg_stab(copy(buf), N_LSYM, 0, 0);
            }
            break;
        }
    }
    dbglist = NULL;
}

void dbg_init()
{
    dbglist     = 0;
    dbgscope    = 0;
    dbglistproc = 0;
    dbg_loclist = 0;    /* for safety */
    dbg_inittypes();
}

#endif /* TARGET_HAS_DEBUGGER */

/* End of section dbx.c */
