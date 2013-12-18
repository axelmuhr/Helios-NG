/*
 * C compiler file mip/aoutobj.c
 * Copyright (C) Acorn Computers, 1987.
 * Copyright (C) Codemist Ltd., 1988.
 * version 12a
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:07:18 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char aoutobj_version[];
char aoutobj_version[] = "\naoutobj.c $Revision: 1.1 $ 12a\n";
#endif

/* Memo: AM is converting this file into a generic (target machine/os      */
/*   parameterised) a.out formatter.                                       */
/*   Note that #ifdef TARGET_IS are soon to become procedural interfaces.  */
/*   However, do the 88000 or 29000 32 from 64 bit references first.       */
/* Memo: currently this formatter dies with calls of (data cast to code).  */

#include "globals.h"            /* for "target.h" via "host.h" */
/*
 * "target.h" (via globals.h) must be included before "aout.h", since our
 * private version of a header defining the a.out format is parameterised
 * wrt the type of target machine involved.  If we are not cross compiling
 * and a host specific <a.out.h> is available it might be better to use
 * that rather than "aout.h".
 */

#ifndef __STDC__
#include <a.out.h>
#include <strings.h>
#include <sys/types.h>
#define  SEEK_SET 0
#else
#include "aout.h"
#include <string.h>
#endif
#define MAXNUMSYMS ((int32)1 << 24)   /* limited by bit field in a.out.h */

#include "mcdep.h"
#include "mcdpriv.h"
#include "store.h"
#include "codebuf.h"
#include "xrefs.h"
#include "errors.h"

/* All the code from all functions is buffered until the end of           */
/* compilation so that local references can be resolved to avoid the      */
/* linker bombing.  AM vaguely remembers weasel words about a.out which   */
/* might fix this.                                                        */

static int32 obj_fwrite_cnt;

static void obj_fwrite(VoidStar buff, int32 n, int32 m, FILE *f)
{   if (debugging(DEBUG_OBJ) && 0)
    {   int i;
        fprintf(f, "%.6lx:", (long)obj_fwrite_cnt);
        obj_fwrite_cnt += n*m;
        for (i=0; i<n*m; i++)
          fprintf(f, " %.2x", ((unsigned char *)buff)[i]);
        fprintf(f, "\n");
    }
    else fwrite(buff, (size_t)n, (size_t)m, f);
}

void obj_common_start(Symstr *name)
{
 /* No support in a.out for common definitions - just the block name as an
    exported label in the normal data area.
  */
    labeldata(name);
    obj_symref(name, xr_data+xr_defext, dataloc);
}

void obj_common_end(void)
{
}


/*
 * The compiler works quite happily on most machines, generating code
 * in the host byte sex.  But this seems to be not quite what ald
 * expects on the SUN.  Hence the frig below to explicitly swap the
 * byte sex on various bits of the output.
 */
#ifdef sun
static void obj_specialfwrite(VoidStar buff, int32 n, int32 m, FILE *f)
{
    m *= n;
    for (n = 0; n < m; n += 4) {
       char *p = ((char *)buff) + n;
       fputc(p[3], f);
       fputc(p[2], f);
       fputc(p[1], f);
       fputc(p[0], f);
    }
}
#define if_sun_swap(a, b) { char tmp = (a); (a) = (b); (b) = tmp; }
#else
#define obj_specialfwrite obj_fwrite
#define if_sun_swap(a, b)
#endif

FILE *objstream;

/* offset of first string in the string table section */
#define FIRST_STR        4
#define MAXEXTNAMELEN  256

/*
 * Imports: codebase, dataloc
 */

typedef struct StabList {
    struct StabList *next;
    struct nlist nlist;
} StabList;

static StabList *obj_stablist;

static int32 ncoderelocs, ndatarelocs, obj_symcount, obj_stabcount;

ExtRef *obj_symlist;
CodeXref *codexrefs;
DataXref *dataxrefs;
DataXref *dbgxrefs;

#ifdef COMPILING_ON_SMALL_MEMORY
/* Buffer code in a temporary file */

FILE *obj_tmpfile;
____notyetfinished;

#else /* !COMPILING_ON_SMALL_MEMORY */
/* Buffer code in memory */

#define MAXCODESEGS 256

/* AM: @@@ codesize seems simply to duplicate 'codebase' (q.v.).        */
static int32 (*(xcodevec[MAXCODESEGS]))[CODEVECSEGSIZE], codesize;

#define xcode_inst_(q) (*xcodevec[(q)>>(CODEVECSEGBITS+2)]) \
                                  [((q)>>2)&(CODEVECSEGSIZE-1)]

#ifdef TARGET_HAS_HALFWORD_INSTRUCTIONS
/* A fix up for machines (like Clipper) which may have unaligned relocated */
/* words in opcodes.  NB: needs improving for 386 (use memcpy to align?).  */
/* (Or make xcode_inst_ optionally a halfword/byte vector as code_inst_).  */
/* Also worry about cross compilation and byte sex here one day.           */
#  ifdef TARGET_IS_LITTLE_ENDIAN
static void set_code(int32 q, int32 n)
{   if ((q&3)==0) xcode_inst_(q) = n;
    else
    {   xcode_inst_(q)   = ((xcode_inst_(q))&0xffff)+(n<<16);
        xcode_inst_(q+2) = ((xcode_inst_(q+2))&0xffff0000)+((unsigned32)n>>16);
    }
}
static int32 get_code(int32 q)
{   return ((q&3)==0 ? xcode_inst_(q) :
             ((xcode_inst_(q+2))<<16) + (((unsigned32)xcode_inst_(q))>>16));
}
#  endif
#  ifdef TARGET_IS_BIG_ENDIAN
static void set_code(int32 q, int32 n)
{   if ((q&3)==0) xcode_inst_(q) = n;
    else
    {   xcode_inst_(q)   = ((xcode_inst_(q))&0xffff0000)+((unsigned32)n>>16);
        xcode_inst_(q+2) = ((xcode_inst_(q+2))&0xffff)+(n<<16);
    }
}
static int32 get_code(int32 q)
{   return ((q&3)==0 ? xcode_inst_(q) :
             ((xcode_inst_(q))<<16) + (((unsigned32)xcode_inst_(q+2))>>16));
}
#  endif
#else
#  define get_code(q)   xcode_inst_(q)
#  define set_code(q,n) xcode_inst_(q)=n
#endif

#define FileOffsetOfSym(x) \
   (x->extoffset + ((x->extflags & xr_code) ? 0 :        \
                    (x->extflags & xr_data) ? codesize : codesize+dataloc))


static void buffer_code(int32 *src, int32 nwords)
{
  int32 *p;
  for (p = src; nwords > 0; --nwords) {
    int32 hi = codesize >> (CODEVECSEGBITS+2);
    int32 lo = (codesize >> 2) & (CODEVECSEGSIZE-1);
    if (lo == 0) { /* need another segment */
      if (hi >= MAXCODESEGS) cc_fatalerr(aout_fatalerr_toobig);
      xcodevec[hi] = (int32(*)[CODEVECSEGSIZE]) GlobAlloc(
                                        SU_Other, sizeof(*xcodevec[0]));
    }
    (*xcodevec[hi])[lo] = *p++; codesize += 4;
  }
}

static void relocate_code_refs_to_locals(void)
{   /* This proc. should soon callback to a routine in gen.c:          */
  CodeXref *cxr;
  for (cxr = codexrefs;  cxr != NULL;  cxr = cxr->codexrcdr)
  { Symstr *s = cxr->codexrsym;
    ExtRef *x = symext_(s);
    int32 codeoff = cxr->codexroff & 0xffffff;
    int32 w = get_code(codeoff);
    switch (cxr->codexroff & 0xff000000)
    {
#ifdef TARGET_IS_SPARC
    case X_absreloc:
        if (x->extflags & (xr_defloc | xr_defext))
        {   /*
             * code ref to local code or data via address literal...
             * (or clipper/vax absolute 32 bit branch).
             * Unix linker cannot cope with filling in the address literal
             * with the value of a locally defined symbol: we have to convert
             * it into a reference relative to v$codeseg or v$dataseg.
             * AM: note that we do take notice of the old value here.
             */
            set_code(codeoff, w + FileOffsetOfSym(x));
        }
        break;
    case X_PCw30reloc:
        if (x->extflags & (xr_defloc | xr_defext))
        {   /* defined in this compilation unit so relocate... */
            set_code(codeoff, (w & 0xc0000000) |
                              (((x->extoffset-codeoff) >> 2) & 0x3fffffff));
        }
        if (debugging(DEBUG_OBJ))
            cc_msg("Fixup %.8lx extoff=%.8lx, codeoff=%.8lx, make %.8lx\n",
                (long)w, (long)x->extoffset, (long)codeoff,
                (long)get_code(codeoff));
        break;
    case X_DataAddr:
        set_code(codeoff, w & 0xffc00000); /* Remove the immediate field */
        break;
    case X_DataAddr1:
        set_code(codeoff, w & 0xffffe000); /* Remove the immediate field */
        break;
#else
case X_PCreloc:
        /* pcrelative code ref (presumed to) to code.               */
        /* @@@ cast of array to code pointer causes the following   */
        /* syserr().  Needs fixing properly.                        */
        if (!(x->extflags & xr_code))
            syserr(syserr_aout_reloc);
        if (x->extflags & (xr_defloc | xr_defext))
        {   /* defined in this compilation unit so relocate... */
/* @@@ AM: before 'rationalising' this code, it is IMPORTANT to build   */
/* in the 29000 or 88000 32 bits in 64 bits relocation modes.           */
/* AM: note that in all the following cases any offset in the code is   */
/* ignored and simply overwritten.  Change this one day?                */
#ifdef TARGET_IS_ARM
/* On the ARM relocate a B or BL; offset in WORDs; prefetch 8 bytes.    */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0xff000000) | (((n)-8) >> 2) & 0x00ffffff)
#endif
#ifdef TARGET_IS_88000
/* For the 88000 use no prefetch but word offset.                       */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0xfc000000) | ((n) >> 2) & 0x03ffffff);
#endif
#ifdef TARGET_IS_I860
/* /* (check) For the I860 use no prefetch but word offset.             */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0xfc000000) | ((n) >> 2) & 0x03ffffff);
#endif
#ifdef TARGET_IS_AMD
/* For the AMD 29000 I use something which is WRONG!!!                  */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0xfc000000) | ((n) >> 2) & 0x03ffffff);
#endif
#ifdef TARGET_IS_CLIPPER
/* On clipper relocate the whole 32 bit word with a byte offset.        */
/* Note that the clipper version does not yet (Dec 88) use X_PCreloc.   */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0) | (n) & 0xffffffff)
#endif
#ifdef TARGET_IS_GOULD
/* On Gould relocate the whole 32 bit word with a byte offset.        */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0) | (n) & 0xffffffff)
#endif
#ifdef TARGET_IS_68000
/* On M68K relocate the whole 32 bit word with a byte offset.        */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0) | (n) & 0xffffffff)
#endif
#ifdef TARGET_IS_KCM
/* This is a fake */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0) | (n) & 0xffffffff)
#endif

#ifdef obj_codeupdate
            obj_codeupdate(x->extoffset-codeoff);
#else
            #error Missing a.out self-relocation code (unknown target machine).
#endif
        }
        else
        {   /* Branch to external symbol.  Most Unices expect to be     */
            /* tight branch to self.  (On the ARM, the Unix linker      */
            /* expects unrelocated branch to be -2 words,               */
            obj_codeupdate(0);
        }
        if (debugging(DEBUG_OBJ))
            cc_msg("Fixup %.8lx extoff=%.8lx, codeoff=%.8lx, make %.8lx\n",
                (long)w, (long)x->extoffset, (long)codeoff,
                (long)get_code(codeoff));
        break;
case X_absreloc:
case X_backaddrlit: /* abs ref to external */
        if (x->extflags & (xr_defloc | xr_defext))
        {   /*
             * code ref to local code or data via address literal...
             * (or clipper/vax absolute 32 bit branch).
             * Unix linker cannot cope with filling in the address literal
             * with the value of a locally defined symbol: we have to convert
             * it into a reference relative to v$codeseg or v$dataseg.
             * AM: note that we do take notice of the old value here.
             */
            set_code(codeoff, w + FileOffsetOfSym(x));
        }
        break;
#endif
    }
  }
}

static void obj_writecode(void)
{
    int i = 0;
#if (alignof_double > 4)      /* TARGET_ALIGNS_DOUBLES */
    if (codesize & 7)
    {   static int32 pad[] = {0};
        buffer_code(pad, 1); /* Double word aligned */
    }
#endif
    relocate_code_refs_to_locals();

#ifdef TARGET_IS_SPARC
                                /* We need to remove bits 12, 11 and 10
                                   from lo10 words; hence the junk in
                                   relocate_code_refs_to_locals
                                 */
#endif

   while ((codesize>>2) - CODEVECSEGSIZE*i > CODEVECSEGSIZE)
      obj_specialfwrite(xcodevec[i++], 4, CODEVECSEGSIZE, objstream);
    obj_specialfwrite(xcodevec[i], 4,(codesize>>2)-CODEVECSEGSIZE*i, objstream);
    if (ferror(objstream)) cc_fatalerr(obj_fatalerr_io_object);
}

#endif /* COMPILING_ON_SMALL_MEMORY */


static void obj_outsymtab(void)
{   ExtRef   *x;
    StabList *p;
    int obj_stringpos = FIRST_STR;
    struct nlist n;
    obj_symlist = (ExtRef *)dreverse((List *)obj_symlist);
                  /* oldest = smallest numbered first */

    memset(&n, 0, sizeof(n));
/*
 *  Output the symbol table.
 */
    for (x = obj_symlist; x != 0; x = x->extcdr)
    {
        Symstr *s = x->extsym;
        int flags = x->extflags;
        int type;
        if (debugging(DEBUG_OBJ))
           cc_msg("sym'%s'%lx/%x ", symname_(s), (long)x->extindex, flags);
        if (x->extindex >= 0) {
          /* not internal symbol ... */
          n.n_un.n_strx = obj_stringpos;
          n.n_value = x->extoffset;
          if (flags & (xr_defloc | xr_defext)) {
              if (flags & xr_code) {
                  type = N_TEXT;
              } else if (flags & xr_data) {
                  type = N_DATA;
                  n.n_value += codesize;
              } else {
#ifdef TARGET_IS_SPARC
                  type = N_UNDF | N_EXT;
                  {
                    ExtRef *y = obj_symlist;
                    int32 base = bss_size;
                                /* This is an appalling loop searching
                                   all symbols for the next BSS location
                                 */
                    while(y!=NULL) {
                      if (y != x && y->extflags & xr_bss &&
                          y->extoffset > x->extoffset &&
                          y->extoffset < bss_size)
                        base = y->extoffset;
                      y=y->extcdr;
                    }
                    n.n_value = base - x->extoffset;
                  }
#else
                  type = N_BSS;
                  n.n_value += codesize+dataloc;
#endif
              }
          } else {
              type = N_UNDF;
              /* n.n_value is 0 except for pcc-style common things */
          }
          if (!(flags & xr_defloc)) type |= N_EXT;
          n.n_type = type;
          obj_fwrite(&n, sizeof(struct nlist), 1, objstream);
          obj_stringpos += strlen(symname_(s)) + 2;
        }
    }
    obj_stablist = (StabList *)dreverse((List *)obj_stablist);
    for (p = obj_stablist; p != 0; p = p->next)
    {
        char *name = p->nlist.n_un.n_name;
        if (name != 0) {
            p->nlist.n_un.n_strx = obj_stringpos;
            obj_stringpos += strlen(name) + 1;
        }
        if ((p->nlist.n_type & N_TYPE) == N_DATA) {
            p->nlist.n_value += codesize;
        } else if ((p->nlist.n_type & N_TYPE) == N_BSS)
            p->nlist.n_value += codesize+dataloc;
        obj_fwrite(&p->nlist, sizeof(p->nlist), 1, objstream);
        p->nlist.n_un.n_name = name;     /* @@@ totally spurious */
    }
/*
 *  Output the string table.
 */
    /* first output the length... */
    obj_fwrite(&obj_stringpos, sizeof(obj_stringpos), 1, objstream);
    /* then the contents of the string table */
    for (x = obj_symlist; x != 0; x = x->extcdr)
    {   Symstr *s = x->extsym;
        int l;
        char *name, extname[MAXEXTNAMELEN];
        /* ...construct '_'name then write it out... */
        extname[0] = '_';
        if (x->extindex >= 0) {
          l = strlen(name = symname_(s));
          if (l > (MAXEXTNAMELEN-2)) l = MAXEXTNAMELEN-2;
#ifdef TARGET_IS_UNIX
          if (strncmp( name,"x$",2 ) != 0)
            {
#endif
              strncpy(extname+1, name, l);
              extname[l+1] = 0;
#ifdef TARGET_IS_UNIX
            }
          else
            {
              strncpy(extname, & name[2], l-2);
              extname[l-2] = 0;
            }
#endif
          obj_fwrite(extname, 1, (int32)l+2, objstream);
        }
    }
    for (p = obj_stablist; p != 0; p = p->next) {
        char *name = p->nlist.n_un.n_name;
        if (name != 0) {
            obj_fwrite(name, 1, (int32)strlen(name)+1, objstream);
        }
    }
}

static int32 obj_checksym(Symstr *s)
{   ExtRef *x = symext_(s);
    if (x != 0)
    {   int32 idx = x->extindex;
        int flags = x->extflags;
        if (flags & (xr_defloc | xr_defext)) {
            return (flags & xr_code) ? -N_TEXT :
                   (flags & xr_data) ? -N_DATA :
#ifdef TARGET_IS_SPARC
                     (int)idx;  /* SPARC must avoid BSS */
#else
                     -N_BSS;
#endif
        }
        return (int)x->extindex;
    }
    syserr(syserr_aout_checksym, symname_(s));
    return 0;              /* just to keep dataflow happy.              */
}

static void obj_coderelocation(void)
{   CodeXref *x;
    struct relocation_info r;
    ncoderelocs = 0;

#ifdef TARGET_IS_SPARC
    codexrefs = (CodeXref*)dreverse((List*)codexrefs);
#endif
    memset(&r, 0, sizeof(r));
    for (x = codexrefs; x != NULL; x = x->codexrcdr)
    {   Symstr *s    = x->codexrsym;
        int    sno   = obj_checksym(s);

        r.r_address  = (int)x->codexroff & 0xffffff;
        if (sno >= 0) {
            r.r_extern = 1;
            r.r_symbolnum = sno;
        } else {
            r.r_extern = 0;
            r.r_symbolnum = -sno;
        }
        switch (x->codexroff & 0xff000000)
#ifdef TARGET_IS_SPARC
        {
            case X_absreloc:
                r.r_type = RELOC_32;
                r.r_addend = (sno == -N_DATA ? codesize :
                              sno == -N_TEXT ?  FileOffsetOfSym(symext_(s)) :
                              0) + x->codexrlitoff;
                obj_fwrite(&r, sizeof(r), 1, objstream);
                if (debugging(DEBUG_OBJ)) cc_msg("addreloc '%s' ",symname_(s));
                if (debugging(DEBUG_OBJ))
                    cc_msg("\
r_address=%x, r_symbolnum=%x, r_extern=%d, r_type=%d, r_addend=%x, sno=%d\n",
                        r.r_address, r.r_symbolnum, r.r_extern,
                        r.r_type, r.r_addend, sno);
                ncoderelocs++;
                break;
            case X_DataAddr:
                r.r_type = RELOC_HI22;
                r.r_addend = (sno == -N_DATA ?  FileOffsetOfSym(symext_(s)) :
                              sno == -N_TEXT ?  FileOffsetOfSym(symext_(s)) :
                              0) + x->codexrlitoff;
                                /* If not dataseg then need to add offset */
                obj_fwrite(&r, sizeof(r), 1, objstream);
                if (debugging(DEBUG_OBJ))
                  cc_msg("addreloc-hi '%s'%lx\n",symname_(s), x->codexrlitoff);
                if (debugging(DEBUG_OBJ))
                    cc_msg("\
r_address=%x, r_symbolnum=%x, r_extern=%d, r_type=%d, r_addend=%x, sno=%d\n",
                        r.r_address, r.r_symbolnum, r.r_extern,
                        r.r_type, r.r_addend, sno);
                ncoderelocs++;
                break;
            case X_DataAddr1:
                r.r_type = RELOC_LO10;
                r.r_addend = (sno == -N_DATA ?  FileOffsetOfSym(symext_(s)) :
                              sno == -N_TEXT ?  FileOffsetOfSym(symext_(s)) :
                              0) + x->codexrlitoff;
                                /* If not dataseg then need to add offset */
                obj_fwrite(&r, sizeof(r), 1, objstream);
                if (debugging(DEBUG_OBJ))
                  cc_msg("addreloc-lo '%s'%lx\n",symname_(s), x->codexrlitoff);
                if (debugging(DEBUG_OBJ))
                    cc_msg("\
r_address=%x, r_symbolnum=%x, r_extern=%d, r_type=%d, r_addend=%x, sno=%d\n",
                        r.r_address, r.r_symbolnum, r.r_extern,
                        r.r_type, r.r_addend, sno);
                ncoderelocs++;
                break;
            case X_PCw30reloc:
                r.r_type = RELOC_WDISP30;
                r.r_addend = - r.r_address;
                if (debugging(DEBUG_OBJ)) cc_msg("W30reloc '%s'\n",symname_(s));
                if (debugging(DEBUG_OBJ))
                    cc_msg("\
r_address=%8x, r_symbolnum=%6x, r_extern=%d, r_type=%d, r_addend=%x, sno=%d\n",
                        r.r_address, r.r_symbolnum, r.r_extern,
                        r.r_type, r.r_addend, sno);
                if (sno >= 0) { /* really external */
                    obj_fwrite(&r, sizeof(r), 1, objstream);
                    ncoderelocs++;
                  }
                break;
            default:
                syserr(syserr_aout_reloc1, (long)x->codexroff);
                break;
        }
#else
        {   case X_PCreloc:  /* PC rel ref to external */
                r.r_pcrel = 0; /* not yet relocated PC-relative */
#ifdef TARGET_IS_ARM
                r.r_length   = 3;  /* ARM Branch */
#else
                r.r_length   = 2;  /* long */
#endif
                if (debugging(DEBUG_OBJ)) cc_msg("pcreloc '%s'",symname_(s));
                if (debugging(DEBUG_OBJ))
                    cc_msg("\
r_address=%8x, r_symbolnum=%6x, r_pcrel=%d, \
r_length=%d, r_extern=%d, r_neg=%d sno=%d\n",
                        r.r_address, r.r_symbolnum, r.r_pcrel,
                        r.r_length, r.r_extern, r.r_neg, sno);
                if (sno >= 0) { /* really external */
                    obj_fwrite(&r, sizeof(r), 1, objstream);
                    ncoderelocs++;
                }
                break;
            case X_absreloc:
            case X_backaddrlit: /* abs ref to external */
                r.r_pcrel = 0;
                r.r_length   = 2;  /* long */
                obj_fwrite(&r, sizeof(r), 1, objstream);
                if (debugging(DEBUG_OBJ)) cc_msg("addreloc '%s' ",symname_(s));
                if (debugging(DEBUG_OBJ))
                    cc_msg("\
r_address=%8x, r_symbolnum=%6x, r_pcrel=%d, \
r_length=%d, r_extern=%d, r_neg=%d sno=%d\n",
                        r.r_address, r.r_symbolnum, r.r_pcrel,
                        r.r_length, r.r_extern, r.r_neg, sno);
                ncoderelocs++;
                break;
            default:
                syserr(syserr_aout_reloc1, (long)x->codexroff);
                break;
        }
#endif
    }
}

static void obj_datarelocation(void)
{   DataXref *x;
    struct relocation_info r;
    ndatarelocs = 0;

    memset(&r, 0, sizeof(r));
#ifndef TARGET_IS_SPARC
    r.r_length = 2;  /* long */
#else
    r.r_type = RELOC_32;
    dataxrefs = (DataXref*)dreverse((List*)dataxrefs);
#endif
    for (x = dataxrefs; x != NULL; x = x->dataxrcdr)
    {   Symstr *s  = x->dataxrsym;
        int    sno = obj_checksym(s);
        /* all data relocs are X_backaddrlit (abs ref) so far ? */
        r.r_address = (int)x->dataxroff;
        if (sno >= 0) {
            r.r_extern = 1;
            r.r_symbolnum = sno;
        } else {
            r.r_extern = 0;
            r.r_symbolnum = -sno;
        }
#ifdef TARGET_IS_SPARC

        {
          DataInit *p;
          int32 i;
/*        cc_msg("Looking for %x\n", r.r_address); */
          for (i = 0, p = datainitp;
               i!=r.r_address;
               i += (p->rpt) * (p->sort == LIT_ADCON ? 4 : p->len),
                  p = p->datacdr);
/*        cc_msg("Found: rpt, sort, len, val = %ld, %ld, %ld, %lx\n",
                    p->rpt, p->sort, p->len, p->val);  */
          if (p->sort != LIT_ADCON)
            cc_msg("Not ADCON in Data Reloc %ld\n", p->sort);
          r.r_addend = p->val + (sno>=0 ? 0 :
                                 (sno== -N_TEXT? FileOffsetOfSym(symext_(s))
                                  :codesize));
        }
#endif
        /*
         * Unix linker can't handle relocations relative to things
         * defined locally... checksym converts to text/data relative.
         */
        obj_fwrite(&r, sizeof(r), 1, objstream);
        if (debugging(DEBUG_OBJ)) cc_msg("data reloc '%s'", symname_(s));
        ndatarelocs++;
    }
}

static void obj_writedata(void)
/* follows gendc exactly! */
{ DataInit *p;
  for (p = datainitp; p != 0; p = p->datacdr)
  { int32 rpt, sort, len, val;
    FloatCon *fc;
    rpt = p->rpt, sort = p->sort, len = p->len, val = p->val;
    switch (sort)
    {   case LIT_LABEL:   /* name only present for c.xxxasm */
            break;
        default:  syserr(syserr_aout_gendata, (long)sort);
        /*
         * Beware of cross-compilation between machines of different
         * bytesex in the following stuff.  Currently this is protected
         * by #ifdef sun (see top of file), but it's all very dubious.
         */
        case LIT_BBBB:
            while (rpt-- != 0) obj_fwrite(&val, len, 1, objstream);
            break;
        case LIT_HH:
            {   char *p = (char *)&val;
                if_sun_swap(p[0], p[1]);
                if_sun_swap(p[2], p[3]);
                while (rpt-- != 0) obj_fwrite(p, len, 1, objstream);
            }
            break;
        case LIT_BBH:
            {   char *p = (char *)&val;
                if_sun_swap(p[2], p[3]);
                while (rpt-- != 0) obj_fwrite(p, len, 1, objstream);
            }
            break;
        case LIT_HBB:
            {   char *p = (char *)&val;
                if_sun_swap(p[0], p[1]);
                while (rpt-- != 0) obj_fwrite(p, len, 1, objstream);
            }
            break;
        case LIT_NUMBER:
            if (len != 4) syserr(syserr_aout_datalen, (long)len);
            /* beware: sex dependent... */
            while (rpt-- != 0) obj_specialfwrite(&val, len, 1, objstream);
            break;
        case LIT_FPNUM:
            fc = (FloatCon *)val;
            /* do we need 'len' when the length is in fc->floatlen?? */
            if (len == 4 || len == 8);
            else syserr(syserr_aout_data, (long)rpt, (long)len, fc->floatstr);
            while (rpt-- != 0)
                obj_specialfwrite(&(fc->floatbin), len, 1, objstream);
            break;
        case LIT_ADCON:              /* (possibly external) name + offset */
            {   Symstr *sv = (Symstr *)len;  /* this reloc also in dataxrefs */
                ExtRef *xr = symext_(sv);
                int sno = obj_checksym(sv);
                if (sno == -N_DATA)
                    val += codesize;
                else if (sno == -N_BSS)
                    val += codesize+dataloc;
                if (xr->extflags & (xr_defloc|xr_defext)) val += xr->extoffset;
                /* beware: sex dependent... */
                while (rpt-- != 0) obj_specialfwrite(&val, 4, 1, objstream);
                break;
            }
    }
  }
}

/* exported functions... */

int32 obj_symref(Symstr *s, int flags, int32 loc)
{   ExtRef *x;
    if ((x = symext_(s)) == 0)    /* saves a quadratic loop */
    {   if (obj_symcount >= MAXNUMSYMS)
            cc_fatalerr(aout_fatalerr_toomany);
        x = (ExtRef *) GlobAlloc(SU_Xsym, sizeof(ExtRef));
        x->extcdr = obj_symlist,
          x->extsym = s,
          x->extindex = obj_symcount++,
          x->extflags = 0,
          x->extoffset = 0;
        obj_symlist = symext_(s) = x;
    }
/* The next few lines cope with further ramifications of the abolition of */
/* xr_refcode/refdata in favour of xr_code/data without xr_defloc/defext  */
/* qualification.  This reduces the number of bits, but needs more        */
/* checking in that a symbol defined as data, and then called via         */
/* casting to a code pointer may acquire defloc+data and then get         */
/* xr_code or'ed in.  Suffice it to say this causes confusion.            */
/* AM wonders if gen.c ought to be more careful instead.                  */
    if (flags & (xr_defloc+xr_defext)) {
        if (x->extflags & (xr_defloc+xr_defext)) {
        /* can only legitimately happen for a tentatively defined object  */
        /* in which case both flags and x->extflags should have type      */
        /* xr_data.  Perhaps I should check ?                             */
        } else
            x->extflags &= ~(xr_code+xr_data);
    } else if (x->extflags & (xr_defloc+xr_defext))
        flags &= ~(xr_code+xr_data);
/* end of fix.                                                            */
    x->extflags |= flags;
    if (flags & xr_defloc+xr_defext)
    {            /* private or public data */
        x->extoffset = loc;
    }
    else if ((loc > 0) && !(flags & xr_code) &&
               !(x->extflags & xr_defloc+xr_defext))
    {
                            /* common data, not already defined */
        if (loc > x->extoffset) x->extoffset = loc;
    }
#ifdef TARGET_IS_KCM
    return -1;
#else
    /* The next line returns the offset of a function in the codesegment */
    /* if it has been previously defined -- this saves store on the arm  */
    /* and allows short branches on other machines.  Otherwise it        */
    /* returns -1 for undefined objects or data objects.                 */
    return ((x->extflags & (xr_defloc+xr_defext)) && (x->extflags & xr_code) ?
            x->extoffset : -1);
#endif
}

void obj_init(void)
{
    ncoderelocs   = 0;
    ndatarelocs   = 0;
    obj_symcount  = 0;
    obj_symlist   = 0;
    obj_stabcount = 0;
    obj_stablist  = 0;
    dataxrefs     = 0;
    codexrefs     = 0;
    dbgxrefs      = 0;
    codesize      = 0;
}

/* Rearrangement of function between aoutobj and dbx here - dbx data are now
   buffered (in global store) in dbx, and only handed to aoutobj at compilation
   unit end (as distinct from handed over at the end of each unit, and buffered
   in aoutobj).
 */

void obj_stabentry(struct nlist *p)
{
    StabList *stab;
    stab = (StabList *)SynAlloc(sizeof(StabList));
    stab->next  = obj_stablist; obj_stablist = stab;
    stab->nlist = *p;
    ++obj_stabcount;
}

void obj_codewrite(Symstr *name)
{   /* Called after each routine is compiled -- code_instvec (doubly     */
    /* indexed) has codep (multiple of 4) bytes of code.                 */
    /* In BSD a.out, this has to be buffered to the end of compilation   */
    /* so that the BSD linker can be cow-towed to.                       */
    /* #define COMPILING_ON_SMALL_MEMORY can be used to buffer on disc.  */
    int32 i, nwords;
    IGNORE(name);
    for (i = 0, nwords = codep>>2; nwords > 0; ++i)
    { int32 seg = nwords > CODEVECSEGSIZE ? CODEVECSEGSIZE : nwords;
/* @@@ When cross compiling we really ought to swap byte sex here        */
/* (by consulting code_flagvec_) because we are just about to throw      */
/* away the information of how to swap bytes.                            */
      buffer_code(code_instvec_(i), seg); nwords -= seg;
    }
}

void obj_header(void)
{
}

static void obj_writeheader()
{
    struct exec h;
    memclr(&h, sizeof(h));   /* clear any extra fields (e.g. Clipper bsd)  */

    h.a_magic  = OMAGIC;
    h.a_text   = codesize;   /* actually the next virtual code location... */
    h.a_data   = dataloc;    /* actually the next virtual data location... */
#ifdef TARGET_IS_SPARC
    h.a_bss    = 0;             /* SPARC just does not have a working BSS */
#else
    h.a_bss    = bss_size;
#endif
    h.a_syms   = (obj_symcount+obj_stabcount) * sizeof(struct nlist);
#ifdef TARGET_IS_SPARC
#define M_SPARC 3               /* /* This needs to be fixed */
    h.a_machtype = M_SPARC;
    h.a_toolversion = 1;        /* /* I do not know why */
#endif
    h.a_entry  = 0;
    h.a_trsize = ncoderelocs * sizeof(struct relocation_info);
    h.a_drsize = ndatarelocs * sizeof(struct relocation_info);

    obj_fwrite_cnt = 0;
    obj_fwrite(&h, sizeof(h), 1, objstream);
}

void obj_trailer()
{
    obj_writeheader();
    if (debugging(DEBUG_OBJ)) cc_msg("writecode at %lx\n", ftell(objstream));
    obj_writecode();
    if (debugging(DEBUG_OBJ)) cc_msg("writedata at %lx\n", ftell(objstream));
    obj_writedata();
    if (debugging(DEBUG_OBJ)) cc_msg("codereloc at %lx\n", ftell(objstream));
    obj_coderelocation();
    if (debugging(DEBUG_OBJ)) cc_msg("datareloc at %lx\n", ftell(objstream));
    obj_datarelocation();
    if (debugging(DEBUG_OBJ)) cc_msg("symtab    at %lx\n", ftell(objstream));
#ifdef TARGET_HAS_DEBUGGER
    /*
     * c.armdbx calls back to obj_writedebug to add things to the symbol table.
     */
    dbg_writedebug();
#endif
    obj_outsymtab();
    if (debugging(DEBUG_OBJ)) cc_msg("rewind\n");
/*
 * It is (unexpectedly) vital to use fseek here rather than rewind, since
 * rewind clears error flags. Note further that if there is a partially
 * full buffer just before the rewind & flushing that data causes an error
 * (e.g. because there is no room on disc for it) this error seems to get
 * lost. This looks to me like a shambles in the October 1986 ANSI draft!
 */
    fseek(objstream, 0L, SEEK_SET);   /* works for hex format too */
/*
 * The next line represents a balance between neurotic overchecking and
 * the fact that it would be nice to detect I/O errors before the end of
 * compilation.
 */
    if (ferror(objstream)) cc_fatalerr(obj_fatalerr_io_object);
    if (debugging(DEBUG_OBJ)) cc_msg("rewriting header\n");
    obj_writeheader();        /* re-write header at top of file */
/*
 * file now closed in main() where opened
 */
}

/* end of mip/aoutobj.c */
