
/* C compiler file coffobj.c :  Copyright (C) Codemist Ltd, 1988.       */
/* 'COFF' (system V unix) output routines */
/* version 8 */

#ifndef NO_VERSION_STRINGS
extern char coffobj_version[];
char coffobj_version[] = "\ncoffobj.c $Revision: 1.1 $ 8\n";
#endif

/* AM: Feb 90: add AUX items for sections.  Fix bug in common refs      */
/* (find #if (R_DIR32 == 17) below for detailed explanation).           */
/* Memo: stuff CC_BANNER in the object file in the .comment section. */
/* Put the COFF style debug info in the file too.      */
/* This will cause review of the ordering of symbols   */

/* target.h shall specify: TARGET_HAS_COFF, target_coff_magic = <number>, */
/* and (optionally) target_coff_prefix.                                   */
/* maybe (one day) also the target_coff_<relocations> below too.          */

#ifndef __STDC__
#  include <strings.h>
#  define  SEEK_SET 0
#else
#  include <string.h>
#endif
#include <time.h>              /* see time() below */

#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"
#include "store.h"
#include "codebuf.h"
#include "builtin.h"
#include "xrefs.h"
#include "errors.h"

#ifndef __STDC__
#  include <sys/types.h>
#  include <a.out.h>            /* sysV coff definitions */
/* On SysV this just does:
     #include <nlist.h>
     #include <filehdr.h>
     #include <aouthdr.h>
     #include <scnhdr.h>
     #include <reloc.h>
     #include <linenum.h>
     #include <syms.h>
*/
#else
/*
 * "target.h" (via globals.h) must be included before "coff.h", since our
 * private version of a header defining the COFF format is parameterised
 * wrt the type of target machine involved.  If we are not cross compiling
 * and a host specific <a.out.h> is available it might be better to use
 * that rather than "aout.h".
 */
#  include "coff.h"            /* Codemist private version */
#endif

#ifdef TARGET_IS_88000         /* to target.h soon.        */
#define R_DIR32  133           /* beware -- use name target_coff_abs_reloc? */
#define R_PCRLONG 129
#define R_HVRT16 131
#define R_LVRT16 132
#endif

#ifndef target_coff_prefix
#  define target_coff_prefix "_"
#endif

/* We now follow 'as' and always generate a BSS section which is        */
/* usually empty.  There MAY be common ext. ref. problems otherwise.    */
#define NSCNS  3
#define N_TEXT 1
#define N_DATA 2
#define N_BSS  3

/* The following #defines give the logical section origins.             */
#define ORG_TEXT 0
#define ORG_DATA 0
#define ORG_BSS  0

#define SYM_FULLY_RELOCATED ((Symstr *)0)       /* internal PCreloc ref */

static int32 obj_fwrite_cnt;

static void obj_fwrite(void *buff, int32 n, int32 m, FILE *f)
{   if (debugging(DEBUG_OBJ))
    {   int32 i;
        fprintf(f, "%.6lx:", (long)obj_fwrite_cnt);
        obj_fwrite_cnt += n*m;
        for (i=0; i<n*m; i++)
          fprintf(f, " %.2x", (int)((unsigned8 *)buff)[i]);
        fprintf(f, "\n");
    }
    else fwrite(buff,(size_t)n,(size_t)m,f);
}

FILE *objstream;

/* imports: codebase, dataloc */
static unsigned32 ncoderelocs, ndatarelocs, obj_symcount;
ExtRef *obj_symlist;
CodeXref *codexrefs;
DataXref *dataxrefs;

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

static void buffer_code(int32 *src, int32 nwords)
{
  int32 *p;
  for (p = src; nwords > 0; --nwords) {
    int32 hi = codesize >> (CODEVECSEGBITS+2);
    int32 lo = (codesize >> 2) & (CODEVECSEGSIZE-1);
    if (lo == 0) { /* need another segment */
      if (hi >= MAXCODESEGS) cc_fatalerr(coff_fatalerr_toobig);
      xcodevec[hi] = (int32(*)[CODEVECSEGSIZE]) GlobAlloc(
                                        SU_Other, sizeof(*xcodevec[0]));
    }
    (*xcodevec[hi])[lo] = *p++; codesize += 4;
  }
}


static int32 obj_checksym(Symstr *s);

static void relocate_code_refs_to_locals(void)
{   /* This proc. should soon callback to a routine in gen.c:          */
  CodeXref *cxr;
  for (cxr = codexrefs;  cxr != NULL;  cxr = cxr->codexrcdr)
  { Symstr *s = cxr->codexrsym;
    ExtRef *x = ((void)obj_checksym(s), symext_(s));
    int32 codeoff = cxr->codexroff & 0xffffff, w;
    w = get_code(codeoff);
    switch (cxr->codexroff & 0xff000000)
    {
case X_PCreloc:
        /* pcrelative code ref (presumed to) to code.               */
        /* @@@ cast of array to code pointer causes the following   */
        /* syserr().  Needs fixing properly.                        */
        if (!(x->extflags & xr_code))
            syserr(syserr_coff_reloc);
        if (x->extflags & (xr_defloc | xr_defext))
        {   /* defined in this compilation unit so relocate... */
            cxr->codexrsym = SYM_FULLY_RELOCATED;
/* @@@ AM: before 'rationalising' this code, it is IMPORTANT to build   */
/* in the 29000 or 88000 32 bits in 64 bits relocation modes.           */
/* AM: note that in all the following cases any offset in the code is   */
/* ignored and simply overwritten.  Change this one day?                */
#ifdef TARGET_IS_ARM
/* On the ARM relocate a B or BL; offset in WORDs; prefetch 8 bytes.    */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0xff000000) | (((n)-8) >> 2) & 0x00ffffff)
#endif
#ifdef TARGET_IS_I860
/* For the i860 use no prefetch but word offset.                        */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0xfc000000) | ((n) >> 2) & 0x03ffffff);
#endif
#ifdef TARGET_IS_88000
/* For the 88000 use no prefetch but word offset.                       */
#define obj_codeupdate(n) \
            set_code(codeoff, (w & 0xfc000000) | ((n) >> 2) & 0x03ffffff);
#endif
#ifdef TARGET_IS_MIPS
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
#           define obj_codeupdate(x) syserr(syserr_coff_pcrel);
#endif
#ifdef TARGET_IS_AMD
#           define obj_codeupdate(x) syserr(syserr_coff_pcrel);
#endif
#ifdef TARGET_IS_KCM
#           define obj_codeupdate(x) syserr(syserr_coff_pcrel);
#endif

#ifndef obj_codeupdate
            #error Missing COFF self-relocation code (unknown target machine).
            #define obj_codeupdate(x) syserr(syserr_coff_pcrel);
#endif
            obj_codeupdate(x->extoffset-codeoff);
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
/* Code here may need changing if you set ORG_TEXT &c to non-zero.      */
#if (R_DIR32 == 17)
/* When using R_DIR32 (code 17), COFF (as seen on 68000/i386) requires  */
/* common refs to have their size (->extoffset) added in when           */
/* generated, so the linker can later subtract the size and add in the  */
/* address.  Wonderful unix.  Hence the following code (which is a noop */
/* for non-common extern refs).                                         */
        set_code(codeoff, w + x->extoffset);
#else
/* More obviously sensible code including 88000 (R_DIR32 = 133)         */
        if (x->extflags & (xr_defloc | xr_defext))
        {   /*
             * code ref to local code or data via address literal...
             * (or clipper/vax absolute 32 bit branch).
             * Unix linker cannot cope with filling in the address literal
             * with the value of a locally defined symbol: we have to convert
             * it into a reference relative to v$codeseg or v$dataseg.
             * This process is completed by obj_checksym().
             * AM: note that we do take notice of the old value here.
             */
            set_code(codeoff, w + x->extoffset);
        }
#endif
        break;
case X_DataAddr:        /* 88000 (+i860 etc) */
case X_DataAddr1:
        {   int32 d = cxr->codexrlitoff;
            if (x->extflags & (xr_defloc | xr_defext))
                d += x->extoffset; /* see comment for absreloc */
/* 88000 ocs requires ls 16 bits even in high loading half.             */
            set_code(codeoff, (w & 0xffff0000) | (d & 0xffff));
        }
        break;
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

    while ((codesize>>2) - CODEVECSEGSIZE*i > CODEVECSEGSIZE)
      obj_fwrite(xcodevec[i++], 4, CODEVECSEGSIZE, objstream);
    obj_fwrite(xcodevec[i], 4,(codesize>>2)-CODEVECSEGSIZE*i, objstream);
    if (ferror(objstream)) cc_fatalerr(driver_fatalerr_io_object);
}

#endif /* COMPILING_ON_SMALL_MEMORY */


void obj_codewrite(Symstr *name)
{   /* Called after each routine is compiled -- code_instvec_ (doubly    */
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

/* the remaining fns are intended to be internal only */

/* In COFF, the data segment label is required to be .data etc.         */
#define coffname_(s) ((s) == bindsym_(codesegment) ? ".text" : \
                      (s) == bindsym_(datasegment) ? ".data" : \
                      symname_(s))

/* TEMPHACK: */
union auxent {
    struct syment pad;
    struct { long int x_scnlen;
             unsigned short x_nreloc;
             unsigned short x_nlinno;
           } x_scn;
};

/* TEMPHACK: */
static void segaux(int32 seglen, int32 nrelocs)
{   union auxent v;
    memclr(&v, sizeof(v));
    v.x_scn.x_scnlen = seglen;
    v.x_scn.x_nreloc = (unsigned int)nrelocs;
    v.x_scn.x_nlinno = 0;
    obj_fwrite(&v, SYMESZ, 1, objstream);
}

/* TEMPHACK: */
static void outbsslabel()
{
        struct syment v;
        memclr(&v, sizeof(v));
        strncpy(v.n_name, ".bss", SYMNMLEN);
        v.n_value = ORG_BSS;
        v.n_scnum = N_BSS;
        v.n_type = T_NULL;
        v.n_sclass = C_STAT;
        v.n_numaux = 1;
        obj_fwrite(&v, SYMESZ, 1, objstream);
        segaux(0,0);
}
/* TEMPHACK end */

static void obj_outsymtab()
{   ExtRef *x;
    unsigned32 obj_stringpos = sizeof(obj_stringpos);
    obj_symlist = (ExtRef *)dreverse((List *)obj_symlist);
                  /* oldest = smallest numbered first */
    for (x = obj_symlist; x != 0; x = x->extcdr)
    {   Symstr *s = x->extsym;
        char *name = coffname_(s);
        size_t n = strlen(name),
               k = name[0]=='.' ? 0 : sizeof(target_coff_prefix)-1;
        int32 flags = x->extflags;
        struct syment v;
        memclr(&v, sizeof(v));          /* clear 88k etc pad fields */
        if (debugging(DEBUG_OBJ)) cc_msg("sym $r%lx ", s, (long)flags);
        if (n+k > SYMNMLEN)        /* NB: '>' (not '>=') is OK here */
            /* do the long form, name in the string table */
            v.n_zeroes = 0,
            v.n_offset = obj_stringpos,
            obj_stringpos += (size_t)(n+k+1);
        else
        {   if (k > 0) strncpy(v.n_name, target_coff_prefix, k);
            strncpy(v.n_name+k, name, SYMNMLEN-k);
        }
/* The next line stores the value (= offset in segment) for code or     */
/* data definitions, and stores zero for external refs, except for      */
/* pcc-style common refs, in which it stores the length.                */
/* BSS mods needed on next three lines when cfe supports bss better.    */
        v.n_scnum = !(flags & xr_defloc+xr_defext) ? N_UNDEF :
                    flags & xr_code ? N_TEXT : N_DATA;
        v.n_value = x->extoffset + (v.n_scnum == N_DATA ? ORG_DATA : ORG_TEXT);
        v.n_type = T_NULL;         /* not set yet */
/* below, note that C_EXTDEF is documented as "internal to C, use C_EXT". */
        v.n_sclass = flags & xr_defloc ? C_STAT : C_EXT;
        v.n_numaux = 0;
/* TEMPHACK: */ if (s == bindsym_(codesegment)) v.n_numaux = 1;
/* TEMPHACK: */ if (s == bindsym_(datasegment)) v.n_numaux = 1;
        /* NB note that sizeof(struct syment) > SYMESZ on many machines */
        obj_fwrite(&v, SYMESZ, 1, objstream);
/* TEMPHACK: */ if (s == bindsym_(codesegment)) segaux(codesize,ncoderelocs);
/* TEMPHACK: */ if (s == bindsym_(datasegment)) segaux(dataloc,ndatarelocs);
/* TEMPHACK: */ if (s == bindsym_(datasegment)) outbsslabel();
    }
    /* now write the string table, preceded by its length */
    obj_fwrite(&obj_stringpos, sizeof(obj_stringpos), 1, objstream);
    for (x = obj_symlist; x != 0; x = x->extcdr)
    {   Symstr *s = x->extsym;
        char *name = coffname_(s);
        size_t n = strlen(name),
               k = name[0]=='.' ? 0 : sizeof(target_coff_prefix)-1;
        if (n+k > SYMNMLEN)
        {    if (k > 0) obj_fwrite(target_coff_prefix, 1, k, objstream);
             obj_fwrite(name, 1, (size_t)(n+1), objstream);
        }
    }
}

static int32 obj_checksym(Symstr *s)
{   ExtRef *x = symext_(s);
    if (x != 0)
    {   if (!(x->extflags & xr_defloc+xr_defext) ||
                s==bindsym_(codesegment) || s==bindsym_(datasegment))
            /* honest external or segment defining symbol */
            return x->extindex;
        else
            return obj_checksym(x->extflags & xr_code ?
                                bindsym_(codesegment) : bindsym_(datasegment));
    }
    syserr(syserr_coff_checksym, s);
    return 0;
}

static void obj_wr_reloc(struct reloc *p, int r)
{   p->r_type = r;
    /* NB note that sizeof(struct reloc) > RELSZ on many machines */
    obj_fwrite(p, RELSZ, 1, objstream);
    ncoderelocs++;
}

static void obj_coderelocation()
{   CodeXref *x;
#ifdef TARGET_IS_88000
    ExtRef *x1; int32 d;   /* move into relevant case soon */
#endif
    struct reloc v;
    for (x = codexrefs; x!=NULL; x = x->codexrcdr)
    {   Symstr *s = x->codexrsym;
        if (s == SYM_FULLY_RELOCATED) continue;
        v.r_vaddr = (x->codexroff & 0xffffff) + ORG_TEXT;
        v.r_symndx = obj_checksym(s);
#ifdef TARGET_IS_88000
        v.r_offset = 0;
#endif
        switch (x->codexroff & 0xff000000)
        {   case X_PCreloc:  /* PC rel ref to external */
                if (debugging(DEBUG_OBJ)) cc_msg("pcreloc$r ", s);
                obj_wr_reloc(&v, R_PCRLONG);    /* target_coff_pcrel? */
                break;
#ifdef TARGET_IS_88000          /* i860, 29000 too no doubt             */
/* For the 88000 see p50 of 88open OCS (draft 10 Mar 89).               */
/* See also fixups at case X_DataAddr in relocate_code_refs_to_locals.  */
            case X_DataAddr:
                if (debugging(DEBUG_OBJ)) cc_msg("DataAddr$r ", s);
                v.r_vaddr += 2;
                x1 = symext_(s);
                d = x->codexrlitoff;
                if (x1->extflags & (xr_defloc | xr_defext)) d += x1->extoffset;
                v.r_offset = d >> 16;
                obj_wr_reloc(&v, R_HVRT16);
                break;
            case X_DataAddr1:
                if (debugging(DEBUG_OBJ)) cc_msg("DataAddr$r ", s);
                v.r_vaddr += 2;
                x1 = symext_(s);
                d = x->codexrlitoff;
                if (x1->extflags & (xr_defloc | xr_defext)) d += x1->extoffset;
                v.r_offset = d >> 16;
                obj_wr_reloc(&v, R_LVRT16);
                break;
#endif
            case X_absreloc:    /* abs ref to external */
            case X_backaddrlit: /* ditto, but literal  */
                if (debugging(DEBUG_OBJ)) cc_msg("addreloc$r ", s);
                obj_wr_reloc(&v, R_DIR32);      /* target_coff_abs? */
                break;
            default:
                syserr(syserr_coff_reloc1, (long)x->codexroff);
                break;
        }
    }
}

static void obj_datarelocation()
{   DataXref *x;
    struct reloc v;
    for (x = dataxrefs; x!=NULL; x = x->dataxrcdr)
    {   Symstr *s = x->dataxrsym;
        /* all data relocs are implicitly X_absreloc so far */
        if (debugging(DEBUG_OBJ)) cc_msg("data reloc $r ", s);
        v.r_type = R_DIR32;                    /* target_coff_abs? */
        v.r_vaddr = x->dataxroff+ORG_DATA;     /* & 0xffffff ? */
        v.r_symndx = obj_checksym(s);
#ifdef TARGET_IS_88000
        v.r_offset = 0;         /* zero for R_DIR32 or R_VRT32  */
#endif
        /* NB note that sizeof(struct reloc) > RELSZ on many machines */
        obj_fwrite(&v, RELSZ, 1, objstream);
        ndatarelocs++;
    }
}

static void obj_writedata()  /* follows gendc exactly! */
{ DataInit *p;
  for (p = datainitp; p != 0; p = p->datacdr)
  { int32 rpt = p->rpt, sort = p->sort, len = p->len, val = p->val;
    switch (sort)
    {   case LIT_LABEL:   /* name only present for c.xxxasm */
            break;
        default:  syserr(syserr_coff_gendata, (long)sort);
        case LIT_BBBB:    /* the next 4 are the same as LIT_NUMBER except   */
        case LIT_HH:      /* for (as yet unsupported) cross compilation.    */
        case LIT_BBH:
        case LIT_HBB:
        case LIT_NUMBER:
            if (len != 4) syserr(syserr_coff_datalen, (long)len);
            /* beware: sex dependent... */
            while (rpt-- != 0) obj_fwrite(&val, 4, 1, objstream);
            break;
        case LIT_ADCON:              /* (possibly external) name + offset */
            {   Symstr *sv = (Symstr *)len;  /* this reloc also in dataxrefs */
                ExtRef *xr= symext_(sv);
                (void)obj_checksym(sv);
#if (R_DIR32 == 17)
/* The following line corrects offsets of forward refs within a module, */
/* but also adds in size in a COMMON ref. (COFF non-intuitively needs). */
                val += xr->extoffset;
#else
                if (xr->extflags & (xr_defloc|xr_defext)) val += xr->extoffset;
#endif
                /* beware: sex dependent... */
                while (rpt-- != 0) obj_fwrite(&val, 4, 1, objstream);
            }
            break;
        case LIT_FPNUM:
            {   FloatCon *fc = (FloatCon *)val;
                /* do we need 'len' when the length is in fc->floatlen?? */
                if (len == 4 || len == 8);
                else syserr(syserr_coff_data,
                            (long)rpt, (long)len, fc->floatstr);
                while (rpt-- != 0)
                  obj_fwrite(&(fc->floatbin), len, 1, objstream);
            }
            break;
    }
  }
}

/* exported functions... */

int32 obj_symref(Symstr *s, int flags, int32 loc)
{   ExtRef *x;
    if ((x = symext_(s)) == 0)    /* saves a quadratic loop */
    {   if (obj_symcount > 0x7fffffff)
            cc_fatalerr(coff_fatalerr_toomany);
        x = (ExtRef *)GlobAlloc(SU_Xref, sizeof(ExtRef));
        x->extcdr = obj_symlist,
          x->extsym = s,
          x->extindex = obj_symcount++,
          x->extflags = 0,
          x->extoffset = 0;
        obj_symlist = symext_(s) = x;
/* TEMPHACK: */ if (s == bindsym_(codesegment)) obj_symcount++;
/* TEMPHACK: */ if (s == bindsym_(datasegment)) obj_symcount+=3;
    }
/* The next two lines cope with further ramifications of the abolition of */
/* xr_refcode/refdata in favour of xr_code/data without xr_defloc/defext  */
/* qualification.  This reduces the number of bits, but needs more        */
/* checking in that a symbol defined as data, and then called via         */
/* casting to a code pointer may acquire defloc+data and then get         */
/* xr_code or'ed in.  Suffice it to say this causes confusion.            */
/* AM wonders if gen.c ought to be more careful instead.                  */
    if (flags & (xr_defloc+xr_defext)) x->extflags &= ~(xr_code+xr_data);
    if (x->extflags & (xr_defloc+xr_defext)) flags &= ~(xr_code+xr_data);
/* end of fix, but perhaps we should be more careful about mult. defs.?   */
    x->extflags |= flags;
    if (flags & xr_defloc+xr_defext)
    {            /* private or public data or code */
        x->extoffset = loc;
    }
    else if ((loc > 0) && !(flags & xr_code) &&
               !(x->extflags & xr_defloc+xr_defext))
    {
                            /* common data, not already defined */
                            /* -- put length in x->extoffset    */
        if (loc > x->extoffset) x->extoffset = loc;
    }
    /* The next line returns the offset of a function in the codesegment */
    /* if it has been previously defined -- this saves store on the arm  */
    /* and allows short branches on other machines.  Otherwise it        */
    /* returns -1 for undefined objects or data objects.                 */
    return ((x->extflags & (xr_defloc+xr_defext)) && (x->extflags & xr_code) ?
            x->extoffset : -1);
}

/* For fortran... */
void obj_common_start(Symstr *name)
{   /* There is no real support in COFF for common definitions (BLOCK   */
    /* DATA).  What needs to be done is to turn the block name into     */
    /* an exported symbol in the normal data area (.data).              */
    labeldata(name);
    obj_symref(name, xr_data+xr_defext, dataloc);
}

void obj_common_end(void) {}

void obj_init()
{   ncoderelocs = 0, ndatarelocs = 0, obj_symcount = 0;
    obj_symlist = 0;
    dataxrefs = 0;
    codexrefs = 0;
    codesize = 0;     /* remove */
}

void obj_header()
{   struct filehdr h;
    struct scnhdr s;
    if ((ncoderelocs | ndatarelocs) & ~(unsigned32)(unsigned short)-1)
        cc_fatalerr(coff_fatalerr_toomany);
    obj_fwrite_cnt = 0;
    h.f_magic = target_coff_magic;
    h.f_nscns = NSCNS;
    h.f_timdat = time(NULL);    /* hope unix format -- norcroft use too. */
    h.f_symptr = sizeof(struct filehdr) + NSCNS*sizeof(struct scnhdr) +
                 codesize + dataloc +
                 /* @@@ round to multiple of 4? (RELSZ = 10) */
                 ncoderelocs*RELSZ + ndatarelocs*RELSZ;
    h.f_nsyms = obj_symcount;
    h.f_opthdr = 0;             /* no optional header */
    h.f_flags = 0;              /* default F_xxxx flags */
    obj_fwrite(&h, sizeof(h), 1, objstream);
    /* code section header */
    strncpy(s.s_name, ".text", sizeof(s.s_name));
    s.s_paddr = s.s_vaddr = ORG_TEXT;
    s.s_size = codesize;
    s.s_scnptr = sizeof(struct filehdr) + NSCNS*sizeof(struct scnhdr);
    s.s_relptr = sizeof(struct filehdr) + NSCNS*sizeof(struct scnhdr) +
                 codesize + dataloc;
    s.s_lnnoptr = 0;            /* no line number info */
    s.s_nreloc = (unsigned int)ncoderelocs;
    s.s_nlnno = 0;              /* no line number info */
    s.s_flags = STYP_TEXT;
    obj_fwrite(&s, sizeof(s), 1, objstream);
    /* data section header */
    strncpy(s.s_name, ".data", sizeof(s.s_name));
    s.s_paddr = s.s_vaddr = ORG_DATA;
    s.s_size = dataloc;
    s.s_scnptr = sizeof(struct filehdr) + NSCNS*sizeof(struct scnhdr) +
                 codesize;
    s.s_relptr = sizeof(struct filehdr) + NSCNS*sizeof(struct scnhdr) +
                 codesize + dataloc + ncoderelocs*RELSZ;
    s.s_lnnoptr = 0;            /* no line number info */
    s.s_nreloc = (unsigned int)ndatarelocs;
    s.s_nlnno = 0;              /* no line number info */
    s.s_flags = STYP_DATA;
    obj_fwrite(&s, sizeof(s), 1, objstream);
    /* bss section header */
    strncpy(s.s_name, ".bss", sizeof(s.s_name));
    s.s_paddr = s.s_vaddr = ORG_BSS;
    s.s_size = 0;
/* scnptr and relptr are irrelevant since size and nreloc are zero,     */
/* but set them up in the natural manner (after the end of data scn).   */
    s.s_scnptr = sizeof(struct filehdr) + NSCNS*sizeof(struct scnhdr) +
                 codesize + dataloc;
    s.s_relptr = sizeof(struct filehdr) + NSCNS*sizeof(struct scnhdr) +
                 codesize + dataloc + ncoderelocs*RELSZ + ndatarelocs*RELSZ;
    s.s_lnnoptr = 0;            /* no line number info */
    s.s_nreloc = 0;             /* no relocations      */
    s.s_nlnno = 0;              /* no line number info */
    s.s_flags = STYP_BSS;
    obj_fwrite(&s, sizeof(s), 1, objstream);
}

void obj_trailer()
{   codexrefs = (CodeXref *)dreverse((List *)codexrefs);
    dataxrefs = (DataXref *)dreverse((List *)dataxrefs);
    if (debugging(DEBUG_OBJ)) cc_msg("writecode\n");
    obj_writecode();
    if (debugging(DEBUG_OBJ)) cc_msg("writedata\n");
    obj_writedata();
    if (debugging(DEBUG_OBJ)) cc_msg("coderelocation\n");
    obj_coderelocation();
    if (debugging(DEBUG_OBJ)) cc_msg("datarelocation\n");
    obj_datarelocation();
    if (debugging(DEBUG_OBJ)) cc_msg("symtab\n");
    obj_outsymtab();
    if (debugging(DEBUG_OBJ)) cc_msg("rewind\n");
    rewind(objstream);   /* works for hex format too */
    if (debugging(DEBUG_OBJ)) cc_msg("rewriting header\n");
    obj_header();        /* re-write header at top of file */
    /* file now opened and closed in main(). */
}

/* end of coffobj.c */
