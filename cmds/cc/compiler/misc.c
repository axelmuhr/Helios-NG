
/* c.misc: Misc features for the Norman & Mycroft C compiler */
/* version 44 */
/* $Id: misc.c,v 1.7 1994/09/23 08:44:45 nickc Exp $ */

/* AM 25-may-87: local (subpool) store allocation reworked.       */

#include "cchdr.h"
#include <stddef.h>
#include <stdlib.h>
#ifdef __STDC__
#include <stdarg.h>
#define start_args( a, b )	va_start( a, b )
#define va_dcl
#else
#include <varargs.h>
#define start_args( a, b )	va_start( a )
#endif
#include <string.h>
#include "AEops.h"

#ifdef COMPILING_ON_XPUTER
#undef va_alist
#define va_alist ...
#define va_dcl
#endif

#ifdef COMPILING_ON_DOS
#define va_alist ...
#define va_dcl
#endif
#ifdef __DOS386
#undef max
#undef min
#endif

int _debugging, suppress, feature;

int length(l)
List *l;
{
    int i = 0;
    for (; l != NULL; l = cdr_(l)) i++;
    return i;
}

List *dreverse(x)
List *x;
{
    List *y = 0, *t;
    while (x != 0)
    {   t = cdr_(x);
        cdr_(x) = y;
        y = x;
        x = t;
    }
    return y;
}

List *nconc(x,y)
List *x;
List *y;
/* destructively appends y to x */
{
    if (x == 0) return y;
    else
    {   List *t, *a = x;
        while ((t = cdr_(x)) != 0) x = t;
        cdr_(x) = y;
        return a;
    }
}

int max(a,b)
int a;
int b;
{
    return (a>b ? a : b);
}

int bitcount(n)
int n;
{
/* return the number of bits present in the integer n.                   */
    int r = 0;
    while (n!=0) n ^= n & (-n), r++;
    return(r);
}

/* the following functions are essentially only used by regalloc         */
/* (and a bit by cg for the same purpose).  Accordingly they have been   */
/* 'properly typed'.  The really are part of the ADT of RegList          */

RegList *rldiscard(x)
RegList *x;
{   return (RegList *)discard2((List *)x);
}

bool member(a,l)
VRegnum a;
RegList *l;
{
    for (; l!=NULL; l = l->rlcdr) if (a == l->rlcar) return 1;
    return 0;
}

RegList *ndelete(a,ll)
VRegnum a;
RegList *ll;
{
/* delete a from the list l (destructive operation) - recode with types! */
    List *l = (List *)ll, *r = l, *s;
    if (l==NULL) return((RegList *)l);
    else if (car_(l)==(int)a) return (RegList *)(discard2(l));
    do
    {   s = l;
        l = cdr_(l);
        if (l==NULL) return (RegList *)r;
    } while (car_(l)!=(int)a);
    cdr_(s) = discard2(l);
    return (RegList *)r;
}

/* error message routines... */
/* NB. note that no more notice is taken of what characters an
   error message contains.  The routine called determines...
   1) cc_warn:  perfectly legal, but curious, C program (e.g. = for ==).
   2)           Offences againgst ANSI draft (Return normally).
   2a) cc_rerr:   Recoverable error without loss of code (e.g. int *x=1;)
                  Code 'works' Compiled as on UNIX.
   2b) cc_err:    Code probably lost, e.g. syntax error, or
                       { struct {int a;}*x; f(x->b); }
                  Sets 'compilation failed' flag.
   3) cc_extension:  ditto, but allows controlled extension use.
   3) cc_fatalerr: A cause for giving up compilation (e.g. out of store/
                   too many errors).  NEVER returns.
   4) syserr:   Internal consistency error.  Exits with legs in air,
                unless debugging (syserr_behaviour) flag set.
                N.B. May return for system debuggers only.
   To facilitate previous use, main.c will #define warn and err to syserr.
*/

int warncount=0, recovercount=0, errorcount=0;
int xwarncount=0, xrecovercount=0;
int stuse_total = 0, stuse_data=0, stuse_xref=0, stuse_xsym = 0,
    stuse_sym = 0, stuse_bind = 0, stuse_type = 0, stuse_const = 0,
    stuse_pp = 0;
static int stuse_waste;
static int maxAEstore;
int syserr_behaviour = 0;

void summarise()
{
    fprintf(stderr, "%s: %d warning%s", pp_cisname,
                    warncount,      warncount==1 ? "" : "s");
    if (xwarncount) fprintf(stderr, " (+ %d suppressed)", xwarncount);
    fprintf(stderr, ", %d error%s",
                    recovercount,   recovercount==1 ? "" : "s");
    if (xrecovercount) fprintf(stderr, " (+ %d suppressed)", xrecovercount);
    fprintf(stderr, ", %d serious error%s.\n",
                    errorcount,     errorcount==1 ? "" : "s");
}

static void announce(reason)
char *reason;
{
    fprintf(stderr, "%s, line %d, %s: ", pp_cisname? pp_cisname : "<startup>", pp_linect, reason);
}

/* note that stg_name also gives the names of types since the same bit
   map is used.  Tidy later, see how they are read in rd_declspec().
*/
static char *xstg_name(stg)
SET_BITMAP stg;
{  AEop s;
   for (s = s_char; isdeclstarter_(s); s++)
       if (stg & bitofstg_(s)) return sym_name_table[s];
   return "???";
}

static char *xtype_name(e)
TypeExpr *e;
{   switch (h0_(e))
    {   case s_typespec: return xstg_name(typespecmap_(e));
        case t_fnap: return "<function>";
        case t_content: return "<pointer>";
        case t_subscript: return "<array>";
        default: return "???";
    }
}

static void qprints(s)
char *s;
{   /* used to print symbols (quoted) or syntactic categories */
    if (*s == '<') fprintf(stderr, "%s", s);
    else fprintf(stderr, "'%s'", s);
}

static void superrprintf(s,a)
const char *s;
va_list a;
{
    /* This routine behaves like printf but also recognises escape chars of
       the form $<char> and acts upon them. */
    char v[201]; int n = 0;
    for (;;)
    {   char ch = *s++;
        if (ch == 0 || ch == '$')
        {   v[n] = 0;
            (void)_vfprintf(stderr,v,a);  /* vfprintf segment without '$' */
            if (ch == 0) return;
            switch (ch = *s++)
            {
        case 0:
                return;
        case 's':
                qprints(sym_name_table[va_arg(a,AEop)]);
                break;
        case 'l':   /* current lexeme */
                switch (curlex.sym)
                {   case s_integer:
                        fprintf(stderr, "'%d'", curlex.a1.i); break;
                    case s_floatcon:
                        fprintf(stderr, "'%s'", curlex.a1.fc->floatstr); break;
                    case s_identifier:
                        fprintf(stderr, "'%s'", _symname(curlex.a1.sv)); break;
                    case s_string:  /* Do better with control chars? */
                        fprintf(stderr, "'\"%.*s\"'", curlex.a2.len, curlex.a1.s); break;
                    default:
                        qprints(sym_name_table[curlex.sym]); break;
                }
                break;
        case 'r':
                {   Symstr *r = va_arg(a, Symstr *);
                    if (r==0 || h0_(r) != s_identifier)
                    {   if (r == 0) fprintf(stderr, "<missing>");
                        else fprintf(stderr, "<oddity>");
                    }
                    else fprintf(stderr, "'%s'", _symname(r));
                }
                break;
        case 'b':
                fprintf(stderr, "'%s'", _symname(va_arg(a,Binder *)->bindsym));
                break;
        case 'e':
                {   Expr *e = va_arg(a,Expr *);
                    while (h0_(e) == s_invisible) e = arg1_(e);
                    if (h0_(e) == s_binder) fprintf(stderr, "'%s'",
                          _symname(((Binder *)e)->bindsym));
/* improve the next line -- I wonder what the best heuristic is. */
                    else fprintf(stderr, "<expr>");
                }
                break;
        case 't':
                qprints(xtype_name(va_arg(a,TypeExpr *)));
                break;
        case 'm':
                fprintf(stderr, "'%s'", xstg_name(va_arg(a,SET_BITMAP)));
                break;
        default:
                fprintf(stderr, "$%c", ch);   /* really an error */
                break;
            }
            n = 0;
        }
        else if (n<200) v[n++] = ch;
    }
}

void syserr(s, va_alist)
char *s;
va_dcl
{
    va_list a;
    start_args( a, s);
    announce("Fatal internal error");
    superrprintf(s,a);
    va_end(a);
    fprintf(stderr, "\n");
    switch (syserr_behaviour)
    {
        case 1: return;
        case 2: _postmortem(); break;
        default:  break;		/* NHG - forget about banner msg */
fprintf(stderr, "\n\
*************************************************************************\n\
*                                                                       *\n\
*                      This error should not occur.                     *\n");
fprintf(stderr,"\
*        Please contact your maintenance authority (e.g. dealer)        *\n\
*                        if you can duplicate it.                       *\n");
fprintf(stderr,"\
*                                                                       *\n\
*************************************************************************\n\
\n");
    }
    exit(1);
}

void cc_fatalerr(s, va_alist)
char *s;
va_dcl
{
    va_list a;
    start_args(a, s);
    announce("Fatal error");
    superrprintf(s,a);
    va_end(a);
    fprintf(stderr, "\nCompilation abandoned.\n");
    if (syserr_behaviour)
    {   show_store_use();
        syserr("syserr simulated");
    }
    exit(1);
}

void cc_warn(s, va_alist)
char *s;
va_dcl
{
    va_list a;
    start_args(a, s);
    ++warncount;
    announce("Warning");
    superrprintf(s,a);
    va_end(a);
    fprintf(stderr, "\n");
}

void cc_rerr(s, va_alist)
char *s;
va_dcl
{
    va_list a;
    start_args(a, s);
    ++recovercount;
    announce("Error");
    superrprintf(s,a);
    va_end(a);
    fprintf(stderr, "\n");
}

void cc_err(s, va_alist)
char *s;
va_dcl
{
    va_list a;
    start_args(a, s);
    if (++errorcount > 100) cc_fatalerr("Too many errors");
    announce("Serious");
    superrprintf(s,a);
    va_end(a);
    fprintf(stderr, "\n");
}

/* store allocator - was c.store */

static char *alloc_chain;    /* see alloc_init(), alloc_dispose() */

static void *cc_alloc(n)
int n;
{   char *p;
    stuse_total += n;
    if ((p = malloc(n+4)) != 0)
    { *(char **)p = alloc_chain;
      alloc_chain = p;
      return p+4;
    }
    cc_fatalerr("out of store (in cc_alloc)");
    return 0;   /* stop compiler wingeing re implicit junk return */
}

void alloc_dispose()
{
    unsigned int count = 0;
    if (debugging(DEBUG_STORE)) fprintf(stderr, "Freeing block(s) at:");
    while (alloc_chain != NULL)
    {   char *next = *(char **)alloc_chain;
        if (debugging(DEBUG_STORE))
          fprintf(stderr, "%s %p", count++ % 8 == 0 ? "\n":"", alloc_chain);
        free(alloc_chain);
        alloc_chain = next;
    }
    if (debugging(DEBUG_STORE)) fprintf(stderr, "\n");
}

static int     globallcnt;
static char    *globallp;        /* pointers into symbol table          */
static char    *globalltop;      /* end of symbol table                 */
char    *currentfunction;

static bool phase2alloc;

#define SYN_STOREMAX	200
#define BIND_STOREMAX	200

static char    *synsegbase[SYN_STOREMAX];   /* list of blocks of 'per routine' store */
static char    *synsegptr[SYN_STOREMAX];    /* list of corresponding free addresses  */
static int     synsegcnt;          /* number thereof */
static char *synallp, *synalltop;  /* allocators therein */
static int synallhwm, synallmax;   /* high water         */
static List *synallchain;          /* and a dispose list */

static char    *bindsegbase[BIND_STOREMAX];   /* list of blocks of 'per routine' store */
static char    *bindsegptr[BIND_STOREMAX];    /* list of corresponding free addresses  */
static int     bindsegcnt;          /* number thereof */
static int     bindsegcur;          /* next block to use */
static char *bindallp, *bindalltop; /* allocators therein */
static int bindallhwm, bindallmax;  /* high water         */
static List *bindallchain;          /* and a dispose list */

const char *phasename;

static void *new_global_segment()
{
    char *w;
/* I will recycle a segment that had been used for local space if there  */
/* are any such available.                                               */
    if (bindsegcur < bindsegcnt)
    {   w = bindsegbase[--bindsegcnt];
        if (debugging(DEBUG_STORE))
            fprintf(stderr, "Global store %d from binder size %d at %p\n",
                    globallcnt, SEGSIZE, w);
    }
    else
    {   w = cc_alloc(SEGSIZE);
        if (debugging(DEBUG_STORE))
            fprintf(stderr, "Global store alloc %d size %d at %p (in %s)\n",
                    globallcnt, SEGSIZE, w, currentfunction);
    }
    globallcnt++;
    globallp = w, globalltop = w + SEGSIZE;
    return w;
}

void *GlobAlloc(n)
int n;
{
    char *p = globallp;
    if (n > SEGSIZE || n & 3)  /* 2 extra temp tests */
      syserr("Overlarge storage request (glob %d)", n);
    if (p+n > globalltop)
        stuse_waste += globalltop-p,
        p = new_global_segment();
    globallp = p + n;
    return p;
}

void *xglobal_list1(a)
int a;
{
    void **p = GlobAlloc(INTWIDTH*1);
    p[0]=(void *)a;
    return (void *)p;
}

void *xglobal_list2(a,b)
int a;
void *b;
{
    void **p = GlobAlloc(INTWIDTH*2);
    p[0]=(void *)a;
    p[1]=b;
    return (void *)p;
}

void *xglobal_list3(a,b,c)
int a;
void *b;
void *c;
{
    void **p = GlobAlloc(INTWIDTH*3);
    p[0]=(void *)a;
    p[1]=b;
    p[2]=c;
    return (void *)p;
}

void *xglobal_list4(a,b,c,d)
int a;
void *b;
void *c;
void *d;
{
    void **p = GlobAlloc(INTWIDTH*4);
    p[0]=(void *)a;
    p[1]=b;
    p[2]=c;
    p[3]=d;
    return (void *)p;
}

void *xglobal_list5(a,b,c,d,e)
int a;
void *b;
void *c;
void *d;
void *e;
{
    void **p = GlobAlloc(INTWIDTH*5);
    p[0]=(void *)a;
    p[1]=b;
    p[2]=c;
    p[3]=d;
    p[4]=e;
    return (void *)p;
}

void *xglobal_list6(a,b,c,d,e,f)
int a;
void *b;
void *c;
void *d;
void *e;
void *f;
{
    void **p = GlobAlloc(INTWIDTH*6);
    p[0]=(void *)a;
    p[1]=b;
    p[2]=c;
    p[3]=d;
    p[4]=e;
    p[5]=f;
    return (void *)p;
}

Binder *global_mk_binder(b,c,d,e)
Binder *b;
Symstr *c;
SET_BITMAP d;
TypeExpr *e;
{
    Binder *p = GlobAlloc(SIZEOF_NONAUTO_BINDER);
    stuse_bind += SIZEOF_NONAUTO_BINDER;
    if (d & bitofstg_(s_auto)) syserr("Odd global binder(%x)", d);
    p->h0 = s_binder;
    p->bindcdr=b;
    p->bindsym=c;
    p->bindstg=d;
    p->bindtype=e;
    p->bindaddr.i = 0;  /* soon BINDADDR_UNSET - remember 'datasegment' */
    return p;
}

TagBinder *global_mk_tagbinder(b,c,d)
TagBinder *b;
Symstr *c;
AEop d;
{
    TagBinder *p = GlobAlloc(sizeof(TagBinder));
                   stuse_bind += sizeof(TagBinder);
    p->h0 = s_tagbind;
    p->bindcdr=b;
    p->bindsym=c;
    p->tagbindsort=d;
    p->tagbindmems=0;
    return p;
}


/* volatile storage allocation for use within treatment of 1 function */

static void *new_bindalloc_segment()
{
    if (bindsegcur >= bindsegcnt)
    {   char *w = cc_alloc(SEGSIZE);
        if (bindsegcnt >= BIND_STOREMAX)
            syserr("More than %d blocks of binder store",BIND_STOREMAX);
        if (debugging(DEBUG_STORE))
            fprintf(stderr, "Binder store alloc %d size %d at %p (%s in %s)\n",
                    bindsegcnt, SEGSIZE, w, phasename, currentfunction);
        bindsegbase[bindsegcnt++] = w;
    }
    return bindsegbase[bindsegcur++];
}

static void *new_synalloc_segment()
{
    char *w;
    if (synsegcnt >= SYN_STOREMAX)
        syserr("More than %d blocks of syn store",SYN_STOREMAX);
    if (bindsegcur < bindsegcnt)
    {   w = bindsegbase[--bindsegcnt];
        if (debugging(DEBUG_2STORE))
            fprintf(stderr, "Syntax store %d from binder size %d at %p\n",
                    synsegcnt, SEGSIZE, w);
    }
    else
    {   w = cc_alloc(SEGSIZE);
        if (debugging(DEBUG_STORE))
            fprintf(stderr, "Syntax store alloc %d size %d at %p (%s in %s)\n",
                    synsegcnt, SEGSIZE, w, phasename, currentfunction);
    }
    return synsegbase[synsegcnt++] = w;
}

void suspend_local_store()
{
/* Calling this procedure gives warning that all local store allocated
   thus far is under threat of disposal.   Note that SynAlloc may still
   be used to allocate store until drop_local_store().  This facility
   is used by cg.c for getting a vector of sorted cases (q.v.).
   Now extended to allow (e.g.) loopopt.c to re-allocate Syn-store.
*/
    phase2alloc = 1;
}

void drop_local_store()
{
/* Here the threat issues by suspend_local_store() materialises, and a
   lot of local store is trampled upon. */
/* N.B. drop_local_store *MUST* be called before reinit_alloc()          */
   while (synsegcnt > 0)
    {    char *p = synsegbase[--synsegcnt];
         if (debugging(DEBUG_2STORE))
             fprintf(stderr, "Re-using syntax store %p as binder %d\n",
                     p, bindsegcnt);
/* we do not need to mess with limits here as set to SEGSIZE when used */
         bindsegbase[bindsegcnt++] = p;
    }
    synallp = synalltop = DUFF_ADDR;
    synallhwm = 0;
    synallchain = NULL;
}

void *BindAlloc(n)
int n;
{
    char *p = bindallp;
    if (n > SEGSIZE) syserr("Overlarge storage request (binder %d)", n);
    if (p + n > bindalltop)
    {   int i;
        if (bindsegcur > 0)
            bindsegptr[bindsegcur-1] = p;      /* stash highest used */
        for (i = 0;; i++)                      /* search for scraps  */
        {   if (i == bindsegcur)               /* nowhere big enough */
            {   p = new_bindalloc_segment();
                bindalltop = p + SEGSIZE;
                break;
            }
            p = bindsegptr[i];                 /* hope springs eternal */
            bindalltop = bindsegbase[i] + SEGSIZE;
            if (p+n <= bindalltop)             /* fingers crossed      */
            {   /* we have scavenged something useful - swap to current */
                char *t = bindsegbase[i];
                bindsegbase[i] = bindsegbase[bindsegcur-1];
                bindsegbase[bindsegcur-1] = t;
                bindsegptr[i] = bindsegptr[bindsegcur-1];
                if (debugging(DEBUG_2STORE))
                {   fprintf(stderr, "Scavage binder %d (%p), %d left\n",
                            i, t, bindalltop-(p+n));
                }
                break;
            }
        }
        bindsegptr[bindsegcur-1] = DUFF_ADDR;
    }
    bindallp = p + n;
    if ((bindallhwm += n) > bindallmax) bindallmax = bindallhwm;
    return p;
}

void *SynAlloc(n)
int n;
{   char *p = synallp;
    if (n > SEGSIZE) syserr("Overlarge storage request (local %d)", n);
    if (p + n > synalltop)
    {   int i;
        if (synsegcnt > 0)
            synsegptr[synsegcnt-1] = p;        /* stash highest used */
        for (i = 0;; i++)                      /* search for scraps  */
        {   if (i == synsegcnt)                /* nowhere big enough */
            {   p = new_synalloc_segment();
                synalltop = p + SEGSIZE;
                break;
            }
            p = synsegptr[i];                  /* hope springs eternal */
            synalltop = synsegbase[i] + SEGSIZE;
            if (p+n <= synalltop)              /* fingers crossed      */
            {   /* we have scavenged something useful - swap to current */
                char *t = synsegbase[i];
                synsegbase[i] = synsegbase[synsegcnt-1];
                synsegbase[synsegcnt-1] = t;
                synsegptr[i] = synsegptr[synsegcnt-1];
                if (debugging(DEBUG_2STORE))
                {   fprintf(stderr, "Scavage syntax %d (%p), %d left\n",
                            i, t, synalltop-(p+n));
                }
                break;
            }
        }
        synsegptr[synsegcnt-1] = DUFF_ADDR;
    }
    synallp = p + n;
    if ((synallhwm += n) > synallmax) synallmax = synallhwm;
    return p;
}

List *discard2(p)
List *p;
{
/* As cdr_(p) but returns the cell p to freestorage pool.      */
/* The freechain has a funny number xored in to help debugging */
    List *q = cdr_(p);
    int i;
    if ((((int) q) & 0xff000000)!=0)
        syserr("corrupt list in discard2 %x->%x", p, q);
    car_(p) ^= 0x99990000;   /* to help with debugging */
    for (i = 0; i < synsegcnt; i++)
       if (synsegbase[i] <= (char *)p && (char *)p < synsegbase[i]+SEGSIZE)
       {   cdr_(p) = synallchain;
           synallchain = (List *)(((int) p) ^ 0x6a6a6a6a);
           return q;
       }
    for (i = 0; i < bindsegcur; i++)
       if (bindsegbase[i] <= (char *)p && (char *)p < bindsegbase[i]+SEGSIZE)
       {   cdr_(p) = bindallchain;
           bindallchain = (List *)(((int) p) ^ 0x5a5a5a5a);
           return q;
       }
    syserr("discard2 %p", (void *)p);
    return q;
}

void *binder_list3(a,b,c)
void *a;
void *b;
int c;
{
    void **p = BindAlloc(INTWIDTH*3);
    p[0]=a;
    p[1]=b;
    p[2]=(void *)c;
    return (void *)p;
}

void *syn_list2(a,b)
void *a;
void *b;
{   void **p;
    if (synallchain==NULL) p = SynAlloc(INTWIDTH*2);
    else
    {   p = (void **)((int) synallchain ^ 0x6a6a6a6a);
        synallchain = p[0];
    }
    p[0]=a;
    p[1]=b;
    return (void *)p;
}

void *syn_list5(a,b,c,d,e)
void *a;
void *b;
int c;
int d;
int e;
{
    void **p = SynAlloc(INTWIDTH*5);
    p[0]=a;
    p[1]=b;
    p[2]=(void *)c;
    p[3]=(void *)d;
    p[4]=(void *)e;
    return (void *)p;
}

void *syn_list3(a,b,c)
int a;
int b;
int c;
{   int *p = SynAlloc(INTWIDTH*3);
    p[0]=a, p[1]=b, p[2]=c;
    return (void *)p;
}

static void *syn_list4(a,b,c,d)
int a;
int b;
int c;
int d;
{   int *p = SynAlloc(INTWIDTH*4);
    p[0]=a, p[1]=b, p[2]=c, p[3]=d;
    return (void *)p;
}

static void *syn_list6(a,b,c,d,e,f)
int a;
int b;
int c;
int d;
int e;
int f;
{   int *p = SynAlloc(INTWIDTH*6);
    p[0]=a, p[1]=b, p[2]=c, p[3]=d, p[4]=e, p[5]=f;
    return (void *)p;
}

/* deprecated things to get space according to 'phase2alloc' ...   */

static void *LocAlloc(n)
int n;
{   if (phase2alloc) return BindAlloc(n);
    return SynAlloc(n);
}

void *xlist1(a)
int a;
{
    void **p = LocAlloc(INTWIDTH*1);
    p[0]=(void *)a;
    return (void *)p;
}

void *xlist2(a,b)
int a;
void *b;
{
    if (!phase2alloc) return syn_list2((void *)a, b);
    if (bindallchain==NULL)
    {   void **p = BindAlloc(INTWIDTH*2);
        p[0]=(void *)a;
        p[1]=b;
        return (void *)p;
    }
    else
    {   void **p = (void **)((int) bindallchain ^ 0x5a5a5a5a);
        bindallchain = p[0];
        p[0]=(void *)a;
        p[1]=b;
        return (void *)p;
    }
}

void *xlist3(a,b,c)
int a;
void *b;
void *c;
{
    void **p = LocAlloc(INTWIDTH*3);
    p[0]=(void *)a;
    p[1]=b;
    p[2]=c;
    return (void *)p;
}

void *xlist4(a,b,c,d)
int a;
void *b;
void *c;
void *d;
{
    void **p = LocAlloc(INTWIDTH*4);
    p[0]=(void *)a;
    p[1]=b;
    p[2]=c;
    p[3]=d;
    return (void *)p;
}

void *xlist5(a,b,c,d,e)
int a;
void *b;
void *c;
void *d;
void *e;
{
    void **p = LocAlloc(INTWIDTH*5);
    p[0]=(void *)a;
    p[1]=b;
    p[2]=c;
    p[3]=d;
    p[4]=e;
    return (void *)p;
}

void *xlist6(a,b,c,d,e,f)
int a;
void *b;
void *c;
void *d;
void *e;
void *f;
{
    void **p = LocAlloc(INTWIDTH*6);
    p[0]=(void *)a;
    p[1]=b;
    p[2]=c;
    p[3]=d;
    p[4]=e;
    p[5]=f;
    return (void *)p;
}

Binder *mk_binder(b,c,d,e)
Binder *b;
Symstr *c;
SET_BITMAP d;
TypeExpr *e;
{
    Binder *p = BindAlloc((d & bitofstg_(s_auto)) ? sizeof(Binder) :
                                                    SIZEOF_NONAUTO_BINDER);
    p->h0 = s_binder;
    p->bindcdr=b;
    p->bindsym=c;
    p->bindstg = d | b_localstg;
    p->bindtype=e;
    p->bindaddr.i = BINDADDR_UNSET;
    if (d & bitofstg_(s_auto))
    {   p->bindxx=GAP;
        p->bindrefcount=0;
        p->bindmcrep = NOMCREPCACHE;
    }
    return p;
}

TagBinder *mk_tagbinder(b,c,d)
TagBinder *b;
Symstr *c;
AEop d;
{
    TagBinder *p = SynAlloc(sizeof(TagBinder));
    p->h0 = s_tagbind;
    p->bindcdr=b;
    p->bindsym=c;
    p->tagbindsort=d;
    p->tagbindmems=0;
    return p;
}

/* Expr parse tree constructors */
Expr *mk_expr1(op,t,a1)
AEop op;
TypeExpr *t;
Expr *a1;
{
    return syn_list3(op, (int)t, (int)a1);
}

Expr *mk_expr2(op,t,a1,a2)
AEop op;
TypeExpr *t;
Expr *a1;
Expr *a2;
{
    return syn_list4(op, (int)t, (int)a1, (int)a2);
}

Expr *mk_expr3(op,t,a1,a2,a3)
AEop op;
TypeExpr *t;
Expr *a1;
Expr *a2;
Expr *a3;
{
    return syn_list5((void *)op, (void *)t, (int)a1, (int)a2, (int)a3);
}

#ifndef NO_VALOF_BLOCKS
Expr *mk_expr_valof(op,t,c)
AEop op;
TypeExpr *t;
Cmd *c;
{
    return syn_list3(op, (int)t, (int)c);
}
#endif

Expr *mk_exprwdot(op,t,a1,a2)
AEop op;
TypeExpr *t;
Expr *a1;
int a2;
{
    return syn_list4(op, (int)t, (int)a1, a2);
}

Expr *mk_exprbdot(op,t,a1,a2,a3,a4)
AEop op;
TypeExpr *t;
Expr *a1;
int a2;
int a3;
int a4;
{
    return syn_list6(op, (int)t, (int)a1, a2, a3, a4);
}


/* command nodes... */
Cmd *mk_cmd_0(op,x)
AEop op;
FileLine x;
{
    Cmd *p = SynAlloc(offsetof(Cmd,cmd1));
    p->fileline = x;
    h0_(p) = op;
    return p;
}

Cmd *mk_cmd_e(op,x,e)
AEop op;
FileLine x;
Expr *e;
{
    Cmd *p = SynAlloc(offsetof(Cmd,cmd2));
    p->fileline = x;
    h0_(p) = op, cmd1e_(p) = e;
    return p;
}

Cmd *mk_cmd_default(x,c)
FileLine x;
Cmd *c;
{
    Cmd *p = SynAlloc(offsetof(Cmd,cmd2));
    p->fileline = x;
    h0_(p) = s_default, cmd1c_(p) = c;
    return p;
}

Cmd *mk_cmd_lab(op,x,b,c)
AEop op;
FileLine x;
LabBind *b;
Cmd *c;
{   /* op = s_colon,s_goto */
    Cmd *p = SynAlloc(offsetof(Cmd,cmd3));
    p->fileline = x;
    h0_(p) = op, cmd1c_(p) = (void *)b, cmd2c_(p) = c;
    return p;
}

Cmd *mk_cmd_block(x,bl,cl)
FileLine x;
BindList *bl;
CmdList *cl;
{
    Cmd *p = SynAlloc(offsetof(Cmd,cmd3));
    p->fileline = x;
    h0_(p) = s_block, cmd1c_(p) = (void *)bl, cmd2c_(p) = (void *)cl;
    return p;
}

Cmd *mk_cmd_do(x,c,e)
FileLine x;
Cmd *c;
Expr *e;
{
    Cmd *p = SynAlloc(offsetof(Cmd,cmd3));
    p->fileline = x;
    h0_(p) = s_do, cmd1c_(p) = c, cmd2e_(p) = e;
    return p;
}

Cmd *mk_cmd_if(x,e,c1,c2)
FileLine x;
Expr *e;
Cmd *c1;
Cmd *c2;
{
    Cmd *p = SynAlloc(offsetof(Cmd,cmd4));
    p->fileline = x;
    h0_(p) = s_if, cmd1e_(p) = e, cmd2c_(p) = c1, cmd3c_(p) = c2;
    return p;
}

Cmd *mk_cmd_switch(x,e,c1,c2,c3)
FileLine x;
Expr *e;
Cmd *c1;
Cmd *c2;
Cmd *c3;
{
    Cmd *p = SynAlloc(sizeof(Cmd));
    p->fileline = x;
    h0_(p) = s_switch, cmd1e_(p) = e, cmd2c_(p) = c1,
                       cmd3c_(p) = c2, cmd4c_(p) = c3;
    return p;
}

Cmd *mk_cmd_for(x,e1,e2,e3,c)
FileLine x;
Expr *e1;
Expr *e2;
Expr *e3;
Cmd *c;
{
    Cmd *p = SynAlloc(sizeof(Cmd));
    p->fileline = x;
    h0_(p) = s_for, cmd1e_(p) = e1, cmd2e_(p) = e2,
                    cmd3e_(p) = e3, cmd4c_(p) = c;
    return p;
}

/* for 'case' labels of a switch */
Cmd *mk_cmd_case(x,e,c1,c2)
FileLine x;
Expr *e;
Cmd *c1;
Cmd *c2;
{
    Cmd *p = SynAlloc(sizeof(Cmd));
    p->fileline = x;
    h0_(p) = s_case, cmd1e_(p) = e, cmd2c_(p) = c1,
                     cmd3c_(p) = c2, cmd4c_(p) = 0; /* cmd4c_ = LabelNumber */
    return p;
}


/* Hmm.  The last arg to this varies wildly in type */
TypeExpr *mk_typeexpr1(op,t,a1)
AEop op;
TypeExpr *t;
Expr *a1;
{
    return syn_list3(op, (int)t, (int)a1);
}

void alloc_init()
{
    /* reset the following vars for each one of a many file compilation */
    warncount=0, recovercount=0, errorcount=0;
    warncount=0, recovercount=0;
    stuse_total = 0, stuse_data=0, stuse_xref=0, stuse_xsym = 0, stuse_sym = 0,
    stuse_bind = 0, stuse_type = 0, stuse_const = 0, stuse_pp = 0;
    stuse_waste = 0;
    phase2alloc = 0;
    alloc_chain = NULL;
    synsegcnt = 0;
    synallp = synalltop = DUFF_ADDR;
    synallchain = NULL;
    synallhwm = 0, synallmax = 0;
    bindsegcur = 0, bindsegcnt = 0;
    bindallp = bindalltop = DUFF_ADDR;
    bindallhwm = 0, bindallmax = 0;
    bindallchain = NULL;
    globallcnt = 0;
    globallp = globalltop = DUFF_ADDR;
    maxAEstore = 0;
    phasename = "init";
    currentfunction = "<none>";
}

void alloc_reinit()
{   phase2alloc = 0;
    if (synsegcnt > 0) syserr("alloc_reinit(no drop_local_store())");
    synallp = synalltop = DUFF_ADDR;
    synallchain = NULL;
    bindallhwm = 0;
    bindsegcur = 0,
    bindallp = bindalltop = DUFF_ADDR;
    bindallchain = NULL;
}

void alloc_noteAEstoreuse()
/* note that this does not work between suspend_local_store() and        */
/*      drop_local_store().  (synallp/synalltop are reset).              */
/* Calculate as blocks allocated minus space unused in (only) LAST BLOCK */
{   int n = (synsegcnt*SEGSIZE - (synalltop - synallp)) +
            (bindsegcur*SEGSIZE - (bindalltop - bindallp));
    if (n > maxAEstore) maxAEstore = n;
}

void show_store_use()
{   fprintf(stderr,
            "Total store use (excluding stdio buffers/stack) %d bytes\n",
            stuse_total);
    fprintf(stderr, "Global store use %d/%d bytes\n",
        globallcnt*SEGSIZE- (globalltop - globallp),
        globallcnt*SEGSIZE);
    fprintf(stderr,
"  thereof %d+%d bytes pended relocation, %d bytes pended data\n\
  %d bytes symbols, %d bytes top-level vars, %d bytes types\n\
  %d bytes constants, %d bytes pre-processor, %d bytes wasted\n",
        stuse_xref, stuse_xsym, stuse_data, stuse_sym, stuse_bind,
        stuse_type, stuse_const, stuse_pp, stuse_waste);
    fprintf(stderr, "Local store use %d+%d/%d bytes - front end max %d\n",
        synallmax, bindallmax, (synsegcnt+bindsegcnt)*SEGSIZE, maxAEstore);
    fprintf(stderr, "Code/data generated (%d,%d) bytes\n",
                    codeloc(), dataloc);
    fprintf(stderr, "Max procedure (%s) size %d bytes\n",
                    maxprocname, maxprocsize);
}

/* end of c.misc */
