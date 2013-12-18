/* m68k/asm.c: copyright (C) Codemist Ltd., 1988. */

#ifndef NO_VERSION_STRINGS
extern char asm_version[];
char asm_version[] = "\nm68k/asm.c $Revision: 1.3 $ 19\n";
#endif

/* version 19 */
/* Assembler output is routed to asmstream, annotated if FEATURE_ANNOTATE.  */
/* See m68obj.c for more details on datastructures.                         */

/* exports: asmstream,
            display_assembly_code, asm_header, asm_trailer */


#ifdef __STDC__
#  include <string.h>
#else
#  include <strings.h>
#endif
#include <ctype.h>

#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"
#include "xrefs.h"
#include "store.h"
#include "codebuf.h"
#include "ops.h"
#include "version.h"
#include "errors.h"

FILE *	 		asmstream;
#ifdef TARGET_IS_HELIOS
#ifndef False
#define False 0
#define True (~False)
#endif
extern bool		split_module_table;
static bool		patch_pend  = False;
#endif
static int32		fncount;         /* maybe should be more global */
static bool		asm_error;
static int32		tom68sex(int32, int32);

#ifndef NO_ASSEMBLER_OUTPUT

static int  maybe_export(Symstr *);
static void maybe_import(Symstr *, bool);

#ifdef TARGET_IS_UNIX
static void asm_blank(int32 n)
{   while (n-- > 0) fprintf(asmstream, "#\n");
}
#else
static void asm_blank(int32 n)
{   while (n-- > 0) fprintf(asmstream, "*\n");
}
#endif

int32 asm_padcol8(int32 n)
{   if (!annotations) n = 7;      /* compact the asm file */
    while (n<8) fputc(' ',asmstream), n++;
    return n;
}

static void pr_chars(int32 w)   /* works on both sex machines */
{
  int i, c;
  fputc('\'', asmstream);
  for (i=0; i<sizeof(int32); i++)
  { switch(c = ((unsigned char *)&w)[i])
    {
case '\\':
case '\'':
case '\"':
        break;
case CHAR_BEL:
        c = 'a';
        break;
case '\b':
        c = 'b';
        break;
case CHAR_FF:
        c = 'f';
        break;
case '\n':
        c = 'n';
        break;
case CHAR_CR:
        c = 'r';
        break;
case '\t':
        c = 't';
        break;
case CHAR_VT:
        c = 'v';
        break;
default:
        if (c < ' ') fprintf(asmstream, "\\%o", (int)c);
        else putc(c, asmstream);
        continue;
    }
    putc('\\', asmstream);
    putc(c, asmstream);
  }
  fputc('\'', asmstream);
}

static void pr_asmname(Symstr *sym)
{   char *s;
    if (sym==0) syserr(syserr_pr_asmname);
    s = symname_(sym);
    fputs(s,asmstream);
}

static int spr_asmname(char *where, Symstr *sym)
{   char *s = sym == 0 ? (asm_error = 1, "?") : symname_(sym);
    return sprintf(where, "%.32s", s);
}

void spr_pcrel(char *where, int32 fnoff, int32 pcx)
{   /* Notionally this routine could just do (modulo relocation):  */
    /*     sprintf(where, "%ld-.(pc)", (long)(codebase+fnoff));    */
    LabList *p; LabelNumber *lq = 0;

    if (fnoff <= 0)
    {   /* backward branch or call to a previous routine */
        /* code here depends too much on aoutobj.c etc.  */
        ExtRef *x;
        for (x = obj_symlist; x != 0; x = x->extcdr)  /* SLOW loop !!! */
        {   if (x->extflags & xr_code && (pcx+fnoff) == x->extoffset)
            {   spr_asmname(where, x->extsym);
                return;
            }
        }
    }
    for (p = asm_lablist; p != 0; p = p->labcdr)
    {   LabelNumber *lp = p->labcar;
        if ((lp->u.defn & 0x00ffffff) > fnoff+pcx-codebase) break;
        lq = lp;
    }
    if (lq)
    {   int32 laboff = fnoff + pcx - codebase - (lq->u.defn & 0x00ffffff);
        int32 labnum = lab_name_(lq) & 0x7fffffff;
        if (laboff == 0)
            sprintf(where, "F%ldL%ld", (long)fncount, (long)labnum);
        /* This next case ought only to arise for literal pool refs */
        /* insert extra flags to check this oneday?                 */
        else sprintf(where, "F%ldL%ld%+ld", (long)fncount, (long)labnum,
                                            (long)laboff);
    }
    else asm_error = 1,  /* beware lack of codeseg symbol in next line */
         sprintf(where, "?__codeseg+%ld", (long)(fnoff+pcx));
}

/* decode_external checks if (and by what) a location is to be relocated.  */
Symstr *decode_external(int32 p)        /* hack: exported to m68k/decins.c */
{
    CodeXref *x;
    for (x = codexrefs; x!=NULL; x = x->codexrcdr)    /* SLOW loop !!! */
        if (p == (x->codexroff & 0x00ffffff))
            return x->codexrsym;
    return 0;        /* not an external reference */
}

/* Disassembler routines                                                 */

static void decode_DC(int32 w)
{   
#ifdef TARGET_IS_UNIX
    int32 col = fprintf(asmstream, " .long");
#else
    int32 col = fprintf(asmstream, "\tDC.L");
#endif
    asm_padcol8(col);
    fprintf(asmstream, "$%.8lX", (long)w);
}

#ifdef NOT_NEEDED
static void decode_DW(int32 w)
{   
#ifdef TARGET_IS_UNIX
    int32 col = fprintf(asmstream, " .word");
#else
    int32 col = fprintf(asmstream, "\tDC.W");
#endif
    asm_padcol8(col);
    fprintf(asmstream, "$%.8lX", (long)w);
}
#endif /* NOT_NEEDED */

static void decode_DCA(Symstr *s, int32 w)
{   
#ifdef TARGET_IS_UNIX
    int32 col = fprintf(asmstream, " .long");
#else
    int32 col = fprintf(asmstream, "\tDC.L");
#endif
    col = asm_padcol8(col);
    pr_asmname(s);
    if (w!=0) fprintf(asmstream, "+$%lX", (long)w);
}

static Symstr *find_extsym(int32 p)
{   CodeXref  *x;
    for (x = codexrefs; x != NULL; x = x->codexrcdr) {
        if (p == (x->codexroff & 0x00ffffffL)) return(x->codexrsym);
    }
    syserr("syserr_find_extsym %lx", p);
    return(NULL);
}

typedef struct Import {
    struct Import *next;
    Symstr  *sym;
    int32   patch;
} Import;

static Import *asm_imported;

static void maybe_import(Symstr *sym, bool atcol8)
{   ExtRef  *x;
    Import  *p;

/* Unless external there is nothing to do here. */
    if ((x = symext_(sym)) != 0 &&
        (x->extflags & xr_defloc) != 0) return;

/* Else put out an IMPORT the first time and possibly later patch.  Yuk.   */
    for (p = asm_imported; p != NULL; p = p->next) if (p->sym == sym) return;

/* @@@HCM what is this loop about - isn't symext_(sym) guaranteed to be what
         this is looking for ? */

    for (x = obj_symlist; x != NULL; x = x->extcdr) {
        if (x->extsym == sym) {
            if ((x->extflags & (xr_data|xr_defloc)) == xr_data ||
		(x->extflags & (xr_defloc|xr_defext)) == 0 ||
                (x->extflags & xr_cblock)) {
                /*
                 * Horribly, we remember where we are in the output stream
                 * so that we can later change "IM" to "EX" if we discover
                 * that the thing is declared locally.
                 */
                asm_imported = p = (Import *)global_list3(SU_Other,
                                                          asm_imported, sym,0);
                p->patch = ftell(asmstream);
                if (!(x->extflags & xr_code) && x->extoffset != 0)
                  /*
                   * Common reference
                   */
                    fprintf(asmstream, "\tXDEF\t");
                else
                    fprintf(asmstream, "\tXREF\t");
                pr_asmname(x->extsym);
                putc('\n', asmstream);
            }
            break;
        }
    }
}

static int maybe_export(Symstr *sym)
{   char  *s = symname_(sym);
    char  c;
    FILE   *as = asmstream;
    Import *p;
    ExtRef *x;

/* Unless external there is nothing to do here. */
    if ((x = symext_(sym)) != 0 &&
        (x->extflags & xr_defext) == 0) return 0;

/*@@@ AM does not see how the following can ever now happen as _dataseg etc. */
/*@@@ are very local statics.  Is this if error recovery inserted gensyms?   */
    while ((c = *s++) != 0) { /* look for odd characters in _dataseg etc */
      if (!(isalnum(c) || (c == '_'))) return 0;
    }
    for (p = asm_imported; p != NULL; p = p->next)
      if ((p->sym == sym) && (p->patch != 0)) { /* have IMported it! */
	int32 curpos = ftell(as);
	/* Pray tell, how does this work if output is to terminal? Answer: badly.  */
	/*	    fprintf(stderr,"Seeking to %lx (%d)\n", p->patch, 
		    ((x = symext_(sym)) != 0 && (x->extflags & xr_defloc))); */
	fseek(as, p->patch, 0);
	if ((x = symext_(sym)) != 0 && (x->extflags & xr_defloc))
	  fprintf(as, "\tXREF");
	else
	  fprintf(as, "\tXDEF");
	fseek(as, curpos,   0);
	return 1;
      }
    /* If static, don't export */
    if ((x = symext_(sym)) != 0 && (x->extflags & xr_defloc)) return 0;
#ifdef TARGET_IS_UNIX
    fprintf(as, "\t.globl\t"); pr_asmname(sym); fputs("\n", as);
#else
    fprintf(as, "\tXDEF\t"); pr_asmname(sym); fputs("\n", as);
#endif
    return 1;
}

/* exported functions ...*/

void
display_assembly_code(Symstr *name)
{
  int32 q, ilen;
  char outstring[80];
  LabList *asm_lablist2 = 0;
  
  asm_blank(2);
  
  if (name != 0)   /* may be 0 for string literals from static inits   */
    {
      if (annotations)
	fprintf(asmstream, "%.6lx  %15s", (long)codebase, "");
      asm_lablist2 = asm_lablist;
      fncount++;
      maybe_export(name);
      pr_asmname(name);
      fprintf(asmstream, ":\n");
    }
  
  for (q=0; q < codep; q+=ilen)    /* q is now a BYTE offset */
    {
      const int32 w = code_inst_(q), /* works even if (q & ~3) */
      f = code_flag_(q);
      VoidStar aux = code_aux_(q);
      ilen = 4;

	{
	  int32 labq;
	  LabelNumber *t;

	  while (asm_lablist2 &&
		 (t = asm_lablist2->labcar,
		  labq = t->u.defn & 0x00ffffff) <= q)
	    {   
	      if (f != LIT_OPCODE)
		{   /* literal pool -- ensure aligned */
		  if (annotations)
		    fprintf(asmstream, "%28s", "");
		  putc(' ', asmstream);
		  asm_padcol8(fprintf(asmstream, ".align"));
		  fprintf(asmstream, "4\n");
		}

	      if (annotations)
		fprintf(asmstream, "%18s", "");

	      fprintf(asmstream, "F%ldL%ld:\n", (long)fncount,
		      (long)(lab_name_(t) & 0x7fffffff));
	      if (labq != q)
		syserr(syserr_asmlab, (long)labq);
	      asm_lablist2 = asm_lablist2->labcdr;
	    }
	}
	
#ifdef TARGET_IS_HELIOS	
      patch_pend = False;
	
      if (codexrefs)
	{	
	  CodeXref *	x;
	  int32		pos = 0;
	    
	    
	  for (x = codexrefs;
	       x && (pos = (x->codexroff & 0x00ffffffU), pos > codebase + q);
	       x = x->codexrcdr )
	    ;
	    
	  if (pos == codebase + q)
	    {
	      int32	xrtype = x->codexroff & 0xff000000U;
	      
	      
	      patch_pend = True;
	      
	      switch (xrtype)
		{
		case X_Modnum:
		  fprintf( asmstream, "patchinstr( PATCHADD, shift( %d, MODNUM ),\n",
			  split_module_table ? 3 : 2 );
		  break;
		  
		case X_PCreloc:
		  fprintf( asmstream, "patchinstr( PATCHADD, LABELREF ( .%s ),\n",
			  symname_( (Symstr *)(int)aux ) );
		  break;
		  
		case X_DataModule:
		  fprintf( asmstream, "patchinstr( PATCHADD, DATAMODULE ( _%s ),\n",
			  symname_( (Symstr *)(int)aux ) );
		  break;
		  
		case X_DataSymb:
		  fprintf( asmstream, "patchinstr( PATCHADD, DATASYMB ( _%s ),\n",
			  symname_( (Symstr *)(int)aux ) );
		  break;
		  
		case X_Init:
		  fprintf( asmstream, "INIT\n" );
		  break;
		  
		default:
		  fprintf( asmstream, "* unknown cross reference type %lx\n", xrtype );
		  
		  /* drop through */
		  
		case X_absreloc:
		  patch_pend = False;
		  break;	      
		}
	    }
	}
#endif /* TARGET_IS_HELIOS */
	  
      if (annotations)
	{
	  fprintf(asmstream, "%.6lx  ", (long)(q + codebase));
	  switch (f)
	    {
	    case LIT_OPCODE:      /* handled later */
	      break;
	    case LIT_STRING:
	      fprintf(asmstream, "   %.8lx    ", (long)tom68sex(w,LIT_BBBB));
	      break;
	    default:
	      fprintf(asmstream, "   %.8lx    ", (long)w);
	      break;
	    }
	}
      
      putc(' ', asmstream);
      switch(f)
	{
	case LIT_OPCODE:
	    {   int32 i = 0;
		ilen = decode_instruction(q,outstring);
		if (annotations)
		  {
		    for (i = 0; i < 6; i += 2)
		      if (i < ilen)
			fprintf(asmstream, "%0.4lx ", (long)code_hword_(q+i));
		      else
			fprintf(asmstream, "     ");
		  }
		fputs(outstring,asmstream);
		if( annotations )
		  {   if( i < ilen )
			{
			  fprintf(asmstream, "\n%.6lx   ", (long)(q + codebase));
			  for(; i < ilen; i+= 2)
			    fprintf(asmstream, "%0.4lx ", (long)code_hword_(q+i));
			}
		    }
		break;
	      }
	  case LIT_STRING:
	    decode_DC(tom68sex(w, LIT_BBBB));
	    if (annotations) fprintf(asmstream, "         ; "), pr_chars(w);
	    break;
	  case LIT_NUMBER:
	    decode_DC(w);
	    break;
	  case LIT_ADCON:
	    maybe_import(find_extsym(codebase+q), YES);
	    decode_DCA(decode_external(codebase+q), w);
	    break;
	  case LIT_FPNUM:
	    decode_DC(w);
	    if (annotations)
	      fprintf(asmstream, " ; E'%s'",(char *)aux);
	    break;
	  case LIT_FPNUM1:
	    decode_DC(w);
	    if (annotations)
	      fprintf(asmstream, " ; D'%s'",(char *)aux);
	    break;
	  case LIT_FPNUM2:    /* all printed by the FPNUM1 */
	    decode_DC(w);
	    break;
	  case LIT_RELADDR:
	    maybe_import((Symstr *)aux, NO);
	  default:
	    syserr(syserr_display_asm, (long)f);
	    fprintf(asmstream, "?");
	  }
      
      fprintf( asmstream, patch_pend ? " )\n" : "\n" );
    }
  
  return;
  
} /* display_assembly_code */

void asm_header()
{/*   int32 i; */
    asm_error = 0;
    fncount = 0;
    if (annotations) return;   /* do not bore interactive user */
#ifdef TARGET_IS_UNIX
    fprintf(asmstream, "# generated by %s\n", CC_BANNER);
#else
    fprintf(asmstream, "* generated by %s\n", CC_BANNER);
#endif
/*
 *    for (i=0; i<16; i++)
 *    {   decode_regname(i);
 *        fprintf(asmstream, " EQU %ld\n", (long)i);
 *    }
*/
#ifdef TARGET_IS_UNIX
    fprintf(asmstream, " .text\n");
#else
    fprintf(asmstream, " SECTION\tCODE\n");
#endif
}

/* (not exported) */

static void asm_outextern()
{   ExtRef *x;
    for (x = obj_symlist; x != 0; x = x->extcdr)
    {   int32 flags = x->extflags;
        if (!(flags & xr_defloc) && (flags & xr_defext))
        {
#ifdef TARGET_IS_UNIX
	    fprintf(asmstream, " .globl ");
            pr_asmname(x->extsym);
            fprintf(asmstream, "\n");
#endif
        }
    }
    asm_blank(1);
    for (x = obj_symlist; x != 0; x = x->extcdr)
    {   int32 flags = x->extflags;
        if (!(flags & xr_defloc) && !(flags & xr_defext))
        {   if (x->extoffset != 0)
                fprintf(asmstream, " COMMON "),
                pr_asmname(x->extsym),
                fprintf(asmstream, ",%ld", (long)x->extoffset);
            else
#ifdef TARGET_IS_UNIX
                fprintf(asmstream, "# .extern ");
#else
                fprintf(asmstream, "\tXREF\t");
#endif
                pr_asmname(x->extsym);
            fprintf(asmstream, "\n");
        }
    }
}

typedef struct ExtRefList {
        struct ExtRefList *cdr;
        ExtRef *car;
} ExtRefList;

void asm_trailer()
{ DataInit *p;
  asm_blank(1);
#ifdef TARGET_IS_UNIX
  fprintf(asmstream, "\t.data\n");
#else
  fprintf(asmstream, "\tCSECT\tDATA\n");
#endif
  asm_blank(1);
  for (p = datainitp; p != 0; p = p->datacdr)
  { int32 rpt = p->rpt, sort = p->sort, len = p->len, val = p->val;
    switch (sort)
    {   case LIT_LABEL:
	    maybe_export((Symstr *)rpt);
            pr_asmname((Symstr *)rpt);
            fprintf(asmstream, ":");
            break;
        default:  syserr(syserr_asm_trailer, (long)sort);
        case LIT_BBBB:
        case LIT_HH:
        case LIT_BBH:
        case LIT_HBB:
            val = tom68sex(val, sort);
        case LIT_NUMBER:
            if (!(( len == 4) || (sort == LIT_HH && len == 2)))
                syserr(syserr_asm_datalen, (long)len);
            if (rpt == 1)
            {
                if( len == 2 )
                {
#ifdef TARGET_IS_UNIX
                   asm_padcol8(fprintf(asmstream, " .word") );
#else
                   asm_padcol8(fprintf(asmstream, "\tDC.W") );
#endif
                   fprintf(asmstream,"$%.4lX", (long)(val>>16));
                }
                else
                {
#ifdef TARGET_IS_UNIX
                   asm_padcol8(fprintf(asmstream, " .long") );
#else
                   asm_padcol8(fprintf(asmstream, "\tDC.L") );
#endif
                   fprintf(asmstream,"$%.8lX", (long)val);
                }
            }
            else if (val == 0)
                asm_padcol8(fprintf(asmstream, " DS.B")),
                fprintf(asmstream,"%ld", (long)(rpt*len));
            else syserr(syserr_asm_trailer1, (long)rpt, (long)val);
            break;
        case LIT_FPNUM:
        {   int32 *p = ((FloatCon *)val) -> floatbin.irep;
#ifdef TARGET_IS_UNIX
	    asm_padcol8(fprintf(asmstream, " .long") );
#else
	    asm_padcol8(fprintf(asmstream, "\tDC.L") );
#endif
            fprintf(asmstream,"$%.8lX", (long)p[0]);
            if (annotations) 
                fprintf(asmstream, " ; %s", ((FloatCon *)val) -> floatstr);
            if (len == 8)
#ifdef TARGET_IS_UNIX
                asm_padcol8(fprintf(asmstream, "\n .long")-1L),
#else
                asm_padcol8(fprintf(asmstream, "\n\tDC.L")-1L),
#endif
                fprintf(asmstream,"$%.8lX", (long)p[1]);
            break;
        }
        case LIT_ADCON:              /* (possibly external) name + offset */
        {   int32 i;
	    maybe_import((Symstr *)len, YES);
            for( i = 1; i<= rpt; i++)
            {   
#ifdef TARGET_IS_UNIX
	        asm_padcol8(fprintf(asmstream," .word"));
#else
	        asm_padcol8(fprintf(asmstream,"\tDC.L"));
#endif
                pr_asmname((Symstr *)len);
                if(val!=0) fprintf(asmstream, "+%ld", (long)val);
                if(i!=rpt)fprintf(asmstream,"\n");
            }
            break;
        }
    }
    fprintf(asmstream, "\n");
  }
  asm_blank(1);
#ifdef TARGET_HAS_BSS
 if (bss_size != 0)
    { int32 n = 0;
      ExtRef *x = obj_symlist;
      ExtRefList *zisyms = NULL;
      fprintf(asmstream, "\tCSECT\tBSS\n");
      for (; x != NULL; x = x->extcdr)
	if (x->extflags & xr_bss) {
	  ExtRefList **prev = &zisyms;
	  ExtRefList *p;
	  for (; (p = *prev) != 0; prev = &cdr_(p))
	    if (x->extoffset < car_(p)->extoffset) break;
	  *prev = syn_cons2(*prev, x);
	}
      for (; zisyms != NULL; zisyms = cdr_(zisyms))
	{ x = car_(zisyms);
	  if (x->extoffset != n)
	    fprintf(asmstream, "%ld\n", (x->extoffset-n)>>2);
	  else fprintf(asmstream, "\n");
	  n = x->extoffset;
	  maybe_export(x->extsym);
	  pr_asmname(x->extsym);
	  fprintf(asmstream, "\tDS.L\t");
	}
      if (n != bss_size)
	fprintf(asmstream, "%ld\n", (bss_size-n)>>2);
    }
#endif
  asm_outextern();
  asm_blank(1);
#ifdef TARGET_IS_UNIX
  fprintf(asmstream, ";; .end\n");
#else
  fprintf(asmstream, " END\n");
#endif
}

static int32 tom68sex(int32 w, int32 flag)
{   /* cast in next line ensures byte sex independence */
    unsigned char *pb = (unsigned char *)&w;
    unsigned short *ph = (unsigned short *)&w;
    switch (flag)
    {   default: syserr("syserr_tosexm68 %lx", (long)flag);
        case LIT_BBBB:
            return (int32)pb[0]<<24 | (int32)pb[1]<<16 |
                   (int32)pb[2]<<8 | (int32)pb[3];
        case LIT_OPCODE:
        case LIT_HH:
            return (int32)ph[0]<<16 | (int32)ph[1];
        case LIT_HBB:
            return (int32)ph[0]<<16 | (int32)pb[2]<<8 | (int32)pb[3];
        case LIT_BBH:
            return (int32)pb[0]<<24 | (int32)pb[1]<<16 | (int32)ph[1];
        case LIT_NUMBER:
        case LIT_ADCON:
        case LIT_FPNUM:
        case LIT_FPNUM1:
        case LIT_FPNUM2:
            return w;
    }
}

#endif

/* end of m68k/asm.c */
