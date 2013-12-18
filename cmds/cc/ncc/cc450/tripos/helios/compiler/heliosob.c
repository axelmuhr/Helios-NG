#include <stddef.h>
#include <string.h>
#include "cchdr.h"
#include "xrefs.h"
#include "m68ops.h"
#include "jopcode.h"
#include "module.h"

#define OBJCODE       0x01  
#define OBJBSS        0x02
#define OBJINIT       0x03
#define OBJBYTE       0x09
#define OBJSHORT      0x0a
#define OBJWORD       0x0c
#define OBJLABELOFF   0x0f
#define OBJDATASYMB   0x10
#define OBJDATAMODULE 0x11
#define OBJMODNUM     0x12
#define OBJPATCH1     0x13   /* PATCHES are 0x13-0x1f */
#define OBJPATCHMAX   0x1f
#define OBJMODULE     0x20
#define OBJGLOBAL     0x22
#define OBJLABEL      0x23
#define OBJDATA       0x24
#define OBJCOMMON     0x25

#define OBJPATCHADD OBJPATCH1+0
#define OBJPATCHSHF OBJPATCH1+1

typedef struct slist {
   struct slist *s;
}  slist;

typedef struct GSymlist {
   struct GSymlist *cdr;
   Symstr          *sym;
} GSymlist;

FILE *objstream;
char *objfilename;
char *sourcefile;    /* Must be compiled with GLOBAL_SOURCE_NAME */

int ncoderelocs, ndatarelocs, obj_stringpos, obj_symcount;
ExtRef   *obj_symlist;
ExtRef   **obj_symlistend;
CodeXref *codexrefs;
DataXref *dataxrefs;
CodeXref *reloclist;
Stub     *stublist;
GSymlist *global_symbols;

static int32     codesize;
static int32     codesizepos;
static int32     datasize;
static int32     datasizepos;

int32 suppress_module;

#define objbyte(b) putc(b,objstream)

static void objshort(short x)
{
   fwrite(&x,1,2,objstream);
}

static void objword(int32 x)
{
   fwrite(&x,1,4,objstream);
}

static slist *reverselist(slist *l1)
{
   slist *l2 = NULL;
   slist *t;

   while( l1 != 0 )
   {
      t = l1;
      l1 = l1->s;
      t->s = l2;
      l2 = t;
   }
   return l2;
}

#define bitmask(n) ((1<<(n))-1)

static void objnum(int32 n)
{   int32 i;
    int32 nflag = (n<0)? n = -n, 1 : 0;
    int32 mask7 = bitmask(7);

#define NFLAG 0x40
#define MORE  0x80
/* The prefix notation expressed in this function is described */
/* in Nick Garnett's description of the Helios Link Format for */
/* Non-Transputer Processors                                   */

    for( i = 28; i != 0; i -= 7 )
      if ((n & (mask7 << i)) != 0) break;

    if( (n >> i) & NFLAG ) i += 7;

    objbyte( (n >> i) | (nflag? NFLAG:0) | (i>0? MORE: 0) );

    for( i -= 7; i >= 0; i -= 7)
       objbyte( ((n >> i) & mask7) | (i>0? MORE: 0) );
}

static void objsymbol(char prefix,char *s)
{
   objbyte(prefix);
   fputs(s,objstream);
   objbyte('\0');
}

static void globalise(Symstr *s)
{
   GSymlist *g;
   for( g = global_symbols; g != NULL; g = g->cdr )
      if( g->sym == s ) return;
   global_symbols = (GSymlist *)global_cons2(global_symbols, s);
}

static void dumpglobals(void)
{
   GSymlist *g;
   for( g = global_symbols; g != NULL; g = g->cdr)
   {
      objbyte(OBJGLOBAL);
      objsymbol('_',_symname(g->sym));
   }
}

static int objdirective( int32 xrtype, Symstr *sym, int32 code)
{
    switch( xrtype )
    {
    case X_Modnum:
      objbyte(OBJSHORT);
      objbyte(OBJPATCHSHF); objnum(2);
      objbyte(OBJMODNUM);
      return 2;

    case X_PCreloc:
    case X_DataSymb:
      {
         objbyte(OBJSHORT);
         if( code != 0 )
         {  objbyte(OBJPATCHADD);
            objnum(code);
         }
         if( xrtype == X_PCreloc )
         {
            objbyte(OBJLABELOFF);
            objsymbol('.',_symname(sym));
         }
         else
         {
            objbyte(OBJDATASYMB);
            objsymbol('_',_symname(sym));
         }
         return 2;
      }

    case X_DataModule:
      globalise(sym);
      objbyte(OBJSHORT); objbyte(OBJDATAMODULE);
      objsymbol('_',_symname(sym));
      return 2;

    case X_Init:
      objbyte(OBJINIT); return 4;
      break;

    default:
      syserr("Invalid external reference\n");
    }
}

static void startcode(int32 n)
{  if( n == 0 ) return;
   objbyte(OBJCODE);
   objnum(n);
}

static void startdata(int32 n, char *name)
{
   objbyte(OBJDATA);
   objnum(n);
   objsymbol('_',name);
   datasize += n;
}

static void objlabel(char prefix,char *name)
{
    objbyte(OBJLABEL);
    objsymbol(prefix,name);
}

/* Since an object directive might take us over the end of     */
/* the buffer we are actually outputting this routine returns  */
/* the number of bytes it has encroached into the next buffer. */

static int32 obj_outcode( char *buff, int32 *flag, int32 nbytes,
                                                   int32 segbase)
{   int32 i;
    int32 b=0,n;

    for( b = 0; b < nbytes; b += n)
    {   bool xrflag = 0;
        int32 xrtype = 0;
        if( codexrefs )
        {   int32 x = codexrefs->codexroff & 0x00ffffff;
            if( x < segbase+nbytes )
            {
               xrflag = 1;
               xrtype  = codexrefs->codexroff & 0xff000000;
               n = x - segbase - b;
            }
            else
               n = nbytes-b;
        }
        else
            n = nbytes-b;

        startcode(n);

        for( i = b; i < (b+n); i++)
            objbyte(buff[i]);

        if( xrflag )
        {   CodeXref  *c = codexrefs;

            codexrefs = c->codexrcdr;

            n += objdirective( xrtype, c->codexrsym, c->codexrcode );
        }
    }   
    return (b - nbytes);
}

void obj_codewrite()
{
    int32 i = 0;
    int32 overrun = 0;
    int32 segbase = codebase;

    if( codep == 0) return;

    codexrefs = (CodeXref *)reverselist((slist *)codexrefs);

    while(  (codep>>2) - CODEVECSEGSIZE*i > CODEVECSEGSIZE)
    {   overrun = obj_outcode( (char *)&(*codeandflagvec[i])[0] + overrun,
                        &(*codeandflagvec[i])[CODEVECSEGSIZE],
                        CODEVECSEGSIZE*4 - overrun,segbase);
        segbase += CODEVECSEGSIZE*4+overrun;
        i++;
    }
    obj_outcode( (char *)&(*codeandflagvec[i])[0] + overrun,
                    &(*codeandflagvec[i])[CODEVECSEGSIZE],
                    (codep - CODEVECSEGSIZE*i*4) - overrun,segbase );
    codesize += codep;
}
#ifdef never
static void backpatch(Symstr *sym, int32 addr)
{  CodeXref **r1;
   Stub     **stp;
   int32 x = ftell(objstream);
/* First find all PCrelocation requests which refer to this symbol */
/* and patch the offset                                            */
   for( r1 = &reloclist; *r1 != NULL;)
   {  CodeXref *r = *r1;
      if( r->codexrsym == sym )
      {
         fseek( objstream, r->codexrpos, SEEK_SET );
         objshort( (short)(addr + r->codexrcode -
                           (r->codexroff & 0xffffff)) );
         *r1 = r->codexrcdr;
      }
      else
         r1 = &(r->codexrcdr);
   }

/* Now remove this symbol from the stublist */
   for( stp = &stublist; *stp != NULL; stp = &((*stp)->stubcdr) )
   {
      if( (*stp)->stubsym == sym )
         *stp = (*stp)->stubcdr;
   }

   fseek(objstream,x,SEEK_SET);
}
#endif

void request_stub(Symstr *name)
{  Stub *s;
   for( s = stublist; s != NULL; s = s->stubcdr)
      if( s->stubsym == name ) return;
   stublist = global_cons2(stublist,name);
}

static void show_stubs(void)
{
   in_stubs = 1;
   show_instruction(J_ENTER,GAP,GAP,0);
   while (stublist)
   {  Symstr *sym = stublist->stubsym;
      show_entry(sym, xr_code+xr_defloc); /* really a call to obj_symref */
      show_instruction(J_TAILCALLK,GAP,GAP,(int32)sym);
      show_code(0);
   }
   in_stubs = 0;
}

static void export_routines(void)
{  ExtRef *x;

   for(x = obj_symlist; x != NULL; x = x->extcdr )
   {
      if( (x->extflags & (xr_code+xr_defext)) == (xr_code + xr_defext) )
      {
         globalise(x->extsym);
         startdata(4, _symname(x->extsym) );
         show_instruction(J_ADCON,R_ADDR1,0,(int32)(x->extsym));
         show_instruction(J_STRK, R_ADDR1, R_AS, 0);
         show_instruction(J_ADDK, R_AS, R_AS, 4);
         show_code(0);
      }
   }
}

static void output_symbol(Symstr *sym, int32 size)
{
   if( sym != NULL )
   {  if( ((ExtRef *)_symext(sym))->extflags & xr_defext )
         globalise(sym);
   }

   startdata(size, sym? _symname(sym): NULL );
}

static LabelNumber *loopstart(int32 rpt)
{  LabelNumber *l;
   show_instruction(J_MOVK, R_A1, GAP, rpt-1 );
   l = nextlabel();
   show_instruction(J_LABEL, GAP, GAP, (int32)l);
   return l;
}

static void loopend(LabelNumber *l)
{
   show_instruction(J_SUBK, R_A1, R_A1, 1);
   show_instruction(J_B+Q_GE, GAP, GAP, (int32)l);
}

static void show_init_entry(ExtRef *d)
{
   outINIT();
   regmask = regbit(R_ADDR1);
   show_instruction(J_ENTER, GAP, GAP, 0);
   show_instruction(J_MOVR, R_A2, GAP, R_DP);
   load_static_data_ptr(R_AS);
}

static void show_init_return(void)
{
  show_instruction(J_MOVR, R_DP, GAP, R_A2);
  show_instruction(J_B|Q_AL,GAP,GAP,(int32)RETLAB);
  show_instruction(J_ENDPROC,GAP,GAP,GAP);
}

/* We also have to generate stubs for the variables which are      */
/* initialised to point to functions not defined in this module.   */
static void add_data_stubs(void)
{  DataInit *p;

   for( p = datainitp; p != 0; p = p->datacdr)
   {
      if( p->sort == LIT_ADCON )
      {  Symstr *sym = (Symstr *)p->len;
         ExtRef *x = (ExtRef *)_symext(sym);
         if( is_function(sym) &&
            !(x->extflags & (xr_defext+xr_defloc) ) )
               request_stub(sym);
      }
   }
}

static void dumpdata(void)
{
   ExtRef *datasymbols=NULL;
   ExtRef **dsend=&datasymbols;
/* With any luck the symbols should be in obj_symlist in increasing */
/* order so all I have to do is fetch the data symbols which are    */ 
/* in obj_symlist and append them onto datasymbols to get them in   */
/* the right order   (ascending order of extoffset)                 */
   {
      ExtRef **x1 = &obj_symlist;
      while( *x1 != NULL )
      {  ExtRef *x = *x1;
         if( (x->extflags & xr_data) && (x->extflags & (xr_defext+xr_defloc)) )
         {
            *x1 = x->extcdr;
            x->extcdr = NULL;
            *dsend = x;
            dsend = &x->extcdr;
         }
         else
           x1= &(x->extcdr);
      }
   }

/* At this point we generate the code to initialise the static data */
/* section. */

   {  DataInit *p;
      ExtRef *dsymb = datasymbols;
      int32 dataslot = 0;
      int32 curdsize = 0;
      ExtRef *curdsymb = NULL;

      show_init_entry(datasymbols);
      export_routines();

      for( p = datainitp; p != 0; p = p->datacdr)
      { int32 rpt, sort, len, val;
        Symstr *sv; FloatCon *fc;
        LabelNumber *l;

        rpt = p->rpt, sort = p->sort, len = p->len, val = p->val;

        if( sort == LIT_LABEL ) continue;
        if( sort == LIT_ADCON ) sv = (Symstr *)len, len = 4;

/* Generate the data space offset and label */
        if( dsymb != NULL )
        {  int32 dataoff = dsymb->extoffset;

           if( dataslot == dataoff )
           {
              if(curdsymb != NULL)
                 output_symbol(curdsymb->extsym,curdsize);
              for(; dsymb->extcdr && dsymb->extcdr->extoffset == dataoff;
                      dsymb = dsymb->extcdr )
                 output_symbol(dsymb->extsym, 0);
              curdsize = 0;
              curdsymb = dsymb;

              dsymb = dsymb->extcdr;
           }
           else
           {  if( dataoff < dataslot )
                 syserr("Data seg generation confused");
           }
        }
        curdsize += len*rpt;

/* Now generate the code to initialise the data */
        switch (sort)
        {
            default:  syserr("internal error in obj_gendata(%ld)", sort);
            case LIT_HH:      /* This one may have a len of 2 */
            case LIT_BBBB:    /* the next 4 are the same as LIT_NUMBER except   */
            case LIT_BBH:     /* for (as yet unsupported) cross compilation.    */
            case LIT_HBB:
            case LIT_NUMBER:
                if (! ( (len == 4) || (sort == LIT_HH && len == 2 ) ) )
                     syserr("obj_data len=%ld", len);

                show_instruction(J_MOVK, R_DS, GAP, val);

                if( rpt > 1) l = loopstart(rpt);

                show_instruction(len==4? J_STRK: len==2? J_STRWK: J_STRBK,
                                  R_DS, R_AS, 0);
                show_instruction(J_ADDK, R_AS, R_AS, len);

                if( rpt > 1 ) loopend(l);

                dataslot += rpt*len;
                show_code(0);
                break;

            case LIT_FPNUM:
#ifdef never
                fc = (FloatCon *)val;
                databytes+=len*rpt;

                if (len == 4 || len == 8);
                else syserr("internal error %dEL%d'%s'\n", rpt, len, fc->floatstr);
                while (rpt-- != 0)
                  fwrite(&(fc->floatbin), len, 1, objstream);
#endif
                break;

            case LIT_ADCON:              /* (possibly external) name + offset */

                show_instruction(J_ADCON,R_ADDR1,val,(int32)sv);

                if( rpt > 1) l = loopstart(rpt);

                show_instruction(J_STRK, R_ADDR1, R_AS, 0);
                show_instruction(J_ADDK, R_AS, R_AS, len);

                if( rpt > 1) loopend(l);

                dataslot += rpt*len;
                show_code(0);
                break;
        }
      }
      if( ((curdsize+datasize) & 3) != 0 )
         curdsize += 2;       /* Ensure long alignment of data section */
      if(curdsymb)
         output_symbol(curdsymb->extsym, curdsize);
      show_init_return();
   }
}

static void align(void)
{  int x = 4-(codesize & 3);  /* No. of bytes to get to next mult. of 4 */

   if( x != 4 )
   {  int i;
      objbyte(OBJBSS); objnum(x);
      codesize += x;
   }
}

#ifdef __STDC__
int32 obj_symref(Symstr *s, int flags, int32 loc)
#else
int32 obj_symref(s, flags, loc)
Symstr *s;
int32 flags;
int32 loc;
#endif
{
    ExtRef *x;

    if( (x = _symext(s)) == 0 )     /* if not already defined */
    {
        x = GlobAlloc(sizeof(ExtRef));
        x->extcdr       = NULL;
        x->extsym       = s,
        x->extindex     = obj_symcount++,
        x->extflags     = 0,
        x->extoffset    = 0;
       *obj_symlistend  = _symext(s) = x;
        obj_symlistend  = &x->extcdr;
    }

    x->extflags |= flags;

    if( flags & xr_defloc+xr_defext )
    {
       x->extoffset = loc;
       if( flags & xr_code )
       { Stub **stp;
/* Now remove this symbol from the stublist */
         for( stp = &stublist; *stp != NULL; stp = &((*stp)->stubcdr) )
         {
            if( (*stp)->stubsym == s )
            {  *stp = (*stp)->stubcdr;
               break;
            }
         }
         objlabel('.',_symname(s));
       }
    }
    return( x->extflags & xr_code )? x->extoffset: -1;
}

void obj_init()
{
    obj_symcount = 0;
    obj_symlist  = NULL;
    obj_symlistend = &obj_symlist;
    dataxrefs    = NULL;
    codexrefs    = NULL;
    reloclist    = NULL;
    stublist     = NULL;
    global_symbols = NULL;
    in_stubs     = 0;
}

void obj_header()
{
   if( !suppress_module )
   {  int i,l = strlen(sourcefile);

      objbyte(OBJMODULE); objnum(-1);
   
      startcode(offsetof(Module,Name)+l);
      objword(T_Module);
   
      codesizepos = ftell(objstream);
      objword(0);
   
      fputs(sourcefile, objstream);
   
      objbyte(OBJBSS); objnum(32-l);
   
      objbyte(OBJWORD); objbyte(OBJMODNUM);
   
      startcode(offsetof(Module,Init)-offsetof(Module,Version));
      objword(1);
      datasizepos = ftell(objstream);
      objword(0);

      codesize = sizeof(Module)-sizeof(int);
      outINIT();
   }
}

void obj_trailer()
{  align();
   if( !suppress_module )
   {
      procflags = 0;
      regmask   = 0;
   
      add_data_stubs();
      show_stubs();

      dumpdata();
      show_code(0);
      dumpglobals();
      align();

      fseek(objstream, codesizepos, SEEK_SET);
      objword(codesize);
      fseek(objstream, datasizepos, SEEK_SET);
      objword(datasize);
   }
}
