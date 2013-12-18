/* $Id: vargen.c,v 1.1 1990/09/13 17:10:43 nick Exp $ */

/* vargen.c: static initialised variable generator module of C compiler */
/* Copyright (C) A.Mycroft and A.C.Norman       */
/* version 0.09 */

/* 30-Nov-86: vargen now assembles bytes/halfword data initialisations      */
/* into bytes and adds LIT_XXX flags for xxxasm.c, xxxobj.c                 */

/* Discussion:  Observe that generating code in full generality for
   arbitrary open arrays requires two passes - consider (e.g.)
   "char [][] = { "a", "bc",... }".  Moreover ANSI forbid such
   possibly useful initialisations.  So do we, but for reasons of
   producing (type and size) error messages as soon as possible rather
   than after a reading phase.  Accordingly we adopt a coroutine-like
   linkage to syn.c.
*/

#include "AEops.h"
#include "cchdr.h"
#include "xrefs.h"

#ifdef TARGET_IS_XPUTER
#include "cg.h"
#endif

int dataloc;

DataInit *datainitp;     /* exported to xxxobj.c, xxxasm.c */
static DataInit *datainitq;
static int vg_wpos, vg_wtype, vg_wbuff[1];

void sem_init()
{   dataloc = 0; datainitp = 0;
    vg_wbuff[0] = 0, vg_wpos = 0, vg_wtype = 0;
    errornode = global_list1(s_error);
}

void adddata(x)
DataInit *x;
{   if (vg_wpos != 0) syserr("vg_wpos(%d)", vg_wpos);  /* consistency */
    if (datainitp == 0) datainitp = datainitq = x;
    else datainitq->datacdr = x, datainitq = x;
}

static void gendcE(len,val)
int len;
FloatCon *val;
{   adddata(global_list5(0, 1, LIT_FPNUM, len, (int)val));   stuse_data += 20;
    if (debugging(DEBUG_DATA))
    { fprintf(stderr, "%.6x:  ", dataloc);
      { int *p = val -> floatbin.irep;
        fprintf(stderr, " %.8x", p[0]);
        if (len == 8) fprintf(stderr, " %.8x", p[1]);
        fprintf(stderr, " DC %dEL%d'%s'\n", 1, len, val -> floatstr);
      }
    }
    dataloc += len;
}

static void gendcA(sv,offset)
Symstr *sv;
int offset;
{   /* (possibly external) name + offset */
    obj_symref(sv, xr_reference, 0);
    dataxrefs = global_list3(dataxrefs, dataloc, sv);        stuse_xref += 12;
    adddata(global_list5(0, 1, LIT_ADCON, (int)sv, offset)); stuse_data += 20;
    if (debugging(DEBUG_DATA))
        fprintf(stderr, "%.6x:  ", dataloc),
        fprintf(stderr, " DC A(%s+%d)\n", _symname(sv), offset);
    dataloc += 4;
}

static void gendcI(len,val)
int len;
int val;
{   if (debugging(DEBUG_DATA))
        fprintf(stderr, "%.6x:  ", dataloc),
        fprintf(stderr, " DC FL%d'%d'\n", len, val);
    if ((len != 1 && len != 2 && len != 4) || len+vg_wpos > 4
                                           || (vg_wpos & len-1))
        /* check consistent - includes integral alignment                  */
        syserr("gendcI(%d,%d)", len, vg_wpos);
    /* N.B. the next line is written carefully to be sex independent       */
    switch (len)
    {   case 1: ((unsigned char *)vg_wbuff)[vg_wpos] = val; break;
        case 2: ((unsigned short *)vg_wbuff)[vg_wpos>>1] = val; break;
        case 4: vg_wbuff[0] = val; break;
    }
    vg_wtype = (vg_wtype << len) | 1;            /* flag 'byte' boundaries */  
    vg_wpos += len;
    if (vg_wpos == 4)
    {   int lit_flag;
/* the following values could be coded into the LIT_xxx values             */
        switch (vg_wtype)
        {   default:  syserr("vg_wtype=%.8x", vg_wtype);
            case 1:  lit_flag = LIT_NUMBER; break;
            case 5:  lit_flag = LIT_HH;     break;
            case 15: lit_flag = LIT_BBBB;   break;
            case 7:  lit_flag = LIT_HBB;    break;
            case 13: lit_flag = LIT_BBH;    break;
        }
        vg_wpos = vg_wtype = 0;
        adddata(global_list5(0, 1, lit_flag, 4, vg_wbuff[0])),
        stuse_data += 20;
    }
    dataloc += len;
}

static void gendc0(nbytes)
int nbytes;
{   if (debugging(DEBUG_DATA))
        fprintf(stderr, "%.6x:  ", dataloc),
        fprintf(stderr, " DC %dX'00'\n", nbytes);
    while (nbytes != 0 && vg_wpos != 0) gendcI(1,0), nbytes--;
    if ((nbytes>>2) != 0)
    {   adddata(global_list5(0, nbytes>>2, LIT_NUMBER, 4, 0));
        stuse_data += 20;
    }
    while (nbytes & 3) gendcI(1,0), nbytes--;
    dataloc += nbytes;
}

static void padstatic(align)
int align;
{   if (dataloc & (align-1)) gendc0((-dataloc) & (align-1));
}

static int vg_genstring(p,size)
StringSegList *p;
int size;
{   /* the efficiency of this is abysmal. */
    int planted = 0;
    for (; p != 0; p = p->strsegcdr)
    {   unsigned char *s = (unsigned char *)p->strsegbase;
        int n = p->strseglen;
        while (n-- > 0)
        {   if (planted < size) gendcI(1,*s++), planted++;
            else { cc_err("string initialiser longer than char [%d]", size);
                   return size;
                 }
        }
    }
    if (size == 0xffffff) /* yuk */
        gendcI(1,0), planted++;   /* add trailer */
    else if (planted < size)
        gendc0(size-planted), planted = size;
    else
        cc_warn("omitting trailing '\\0' for char [%d]", size);
    return planted;
}

static int int_of_init(init)
Expr *init;
{   int ival = 0;
    if (init != 0)
    {   if (h0_(init) == s_integer) ival = intval_(init);
        else moan_nonconst(init,"static integral type initialiser");
    }
    return ival;
}

static FloatCon *float_of_init(init)
Expr *init;
{   FloatCon *fval = fc_zero;
    if (init != 0)
    {   if (h0_(init) == s_floatcon) fval = (FloatCon *)init;
        else moan_nonconst(init, "floating type initialiser");
    }
    return fval;
}

static void genpointer(x)
Expr *x;
{   int offset = 0;
    AEop op;
    for (;;) switch (op = h0_(x))
    {   case s_addrof:
        {   Expr *y = arg1_(x); Binder *b;
            if (h0_(y) != s_binder)
                syserr("genpointer&(%d)", h0_(y));
            b = (Binder *)y;
            if (b->bindstg & bitofstg_(s_static))
            {   /* now statics may be local (and hence use datasegment
                 * offsets -- see initstaticvar()) or functions (possibly
                 * not yet defined -- "static f(), (*g)() = f;".
                 */
#ifdef TARGET_IS_XPUTER
                if (b->bindstg & b_fnconst)
		{
			gendcA(b->bindsym, offset);
			genstub( 1, b->bindsym, 1 );
		}
#else
                if (b->bindstg & b_fnconst) gendcA(b->bindsym, offset);
#endif
                else gendcA(datasegment->bindsym, bindaddr_(b)+offset);
            }
            else if (b->bindstg & bitofstg_(s_extern))
	    {
#ifdef TARGET_IS_XPUTER
		/* detect here whether the symbol if for a function	*/
		/* and install a pointer to a stub.			*/
		if( b->bindstg & b_fnconst )
		{
			gendcA(b->bindsym, offset);
			genstub( 1, b->bindsym, 1 );
		}
                else gendcA(b->bindsym, offset);
#else
                gendcA(b->bindsym, offset);
#endif
	    }
            else
            {   sem_err("non-static address $b in pointer initialiser", b);
                gendcI(4,0);
            }
            return;
        }
        case s_monplus:
        case s_cast:
            x = arg1_(x); break;
        case s_plus:
            if (h0_(arg1_(x)) == s_integer)
              { offset += evaluate(arg1_(x)); x = arg2_(x); break; }
            /* drop through */
        case s_minus:
            if (h0_(arg2_(x)) == s_integer)
              { offset += (op == s_plus ? evaluate(arg2_(x)) :
                                          -evaluate(arg2_(x)));
                x = arg1_(x); break; }
            /* drop through */
        default:
            /* I wonder if the type system allows the next error to occur! */
            /* ',' operator probably does */
            sem_err("$s: illegal use in pointer initialiser", op);
            gendcI(4,0);
            return;
    }
}

/* NB. this MUST be kept in step with sizeoftype and findfield (q.v.) */
static void initsubstatic(t)
TypeExpr *t;
{   SET_BITMAP m;
    switch (h0_(t))
    {   case s_typespec:
            m = typespecmap_(t);
            switch (m & -m)    /* LSB - unsigned/long etc. are higher */
            {   case bitoftype_(s_char):
                case bitoftype_(s_enum):    /* maybe do something more later */
                case bitoftype_(s_int):
                    if (m & BITFIELD)
                    {   /* these are all supposedly done as part of the
                           enclosing struct or union. */
                        syserr("initsubstatic(bit)");
                        gendcI(4, 0);
                        break;
                    }
                    gendcI(sizeoftype(t), int_of_init(syn_rdinit(t,0)));
                    break;
                case bitoftype_(s_double):
                    gendcE(sizeoftype(t), float_of_init(syn_rdinit(t,0)));
                    break;                    
                case bitoftype_(s_struct):
                {   int note = syn_begin_agg();   /* skips and notes if '{' */
                    int bitoff, bitword;
                    TagBinder *b = typespectagbind_(t); TagMemList *l;
                    if ((l = tagbindmems_(b)) == 0)
                      sem_err("struct $b must be defined for (static) variable declaration", b);
                    padstatic(4);
                    for (bitoff=bitword=0; l != 0; l = l->memcdr)
                    {   if (l->membits)
                        {   int k = evaluate(l->membits), ival = 0; Expr *e=0;
                            padstatic(alignoftype(te_int));
                            if (k == 0) k = 32-bitoff;  /* rest of int */
                            if (k+bitoff > 32)
                                gendcI(4, bitword),
                                bitoff = bitword = 0;   /* overflow */
                            if (l->memsv == 0 && syn_canrdinit())
/* for the next line consider what the effect should be of:
     "struct { int a:8, :16, b:8; } = { 1,2};"
*/
                              sem_warn("*** unclear ANSI spec: unnamed bit field initialised to 0");
                            else e = syn_rdinit(te_int,0);
                            ival = int_of_init(e);
                            bitword |= (ival & (1<<k)-1) << 
                                (lsbitfirst ? bitoff : 32-k-bitoff);
                            bitoff += k;
                        }
                        else
                        {   if (bitoff != 0)
                                gendcI(4, bitword),
                                bitoff = bitword = 0;
                            padstatic(alignoftype(l->memtype));
                            initsubstatic(l->memtype);
                        }
                    }
                    if (bitoff != 0)
                        gendcI(4, bitword),
                        bitoff = bitword = 0;
                    syn_end_agg(note);
                    padstatic(4);
                    break;
                }
                case bitoftype_(s_union):
/* @@@ Oh dear, another ambiguity in the May 86 ANSI draft.  Consider
   union { int a[2][2] a, ... } x = {{1,2}}.  Is this the same as
   int a[2][2] = {1,2}   ( = {{1,0},{2,0}}) or
   int a[2][2] = {{1,2}} ( = {{1,2},{0,0}})?
   Here it amounts as to whether we do a syn_begin_agg around the following -
   we don't and so arbitrarily favour the second.
*/
                {   TagBinder *b = typespectagbind_(t); TagMemList *l;
                    padstatic(4);
                    if ((l = tagbindmems_(b)) == 0)
                        sem_err("union $b must be defined for (static) variable declaration", b);
                    else
                    {   initsubstatic(l->memtype);
                        gendc0(sizeoftype(t) - sizeoftype(l->memtype));
                    }
                    padstatic(4);
                    break;
                }
                case bitoftype_(s_typedefname):
                    initsubstatic(bindtype_(typespecbind_(t)));
                    break;
#ifdef TARGET_IS_XPUTER
		case bitoftype_(s_void):
		    sem_err("illegal use of void");
		    break;
#endif
                default:
                    syserr("initstatic(%d,0x%x)", h0_(t), m);
                    break;
            }
            break;
        case t_fnap:  /* spotted earlier */
        default:
            syserr("initstatic(%d)", h0_(t));
        case t_subscript:
/* remember struct { char x; int y[0]; char y; } ... */
        {   int note = syn_begin_agg();           /* skips and notes if '{' */
            int i, m = typesubsize_(t) ? evaluate(typesubsize_(t)) : 0xffffff;
            TypeExpr *t2;
            padstatic(alignoftype(typearg_(t)));
/* @@@ N.B. the code here updates the size of an initialised [] array   */
/* with the size of its initialiser.  Currently this side effects the   */
/* typedef in the following: typedef int a[]; a b = { 1,2};             */
/* ANSI may choose to disallow such things.                             */
            if (!syn_canrdinit())
            {   if (typesubsize_(t) == 0)
                    sem_err("Uninitialised static [] arrays illegal");
            }
            else if (t2 = prunetype(typearg_(t)), isprimtype_(t2,s_char))
            {   Expr *e = syn_rdinit(0,1);
                if (e != 0 && h0_(e) == s_string)
                {   int k = vg_genstring(((String *)e)->strseg,m);
                    if (typesubsize_(t) == 0)
                        typesubsize_(t) = globalize_int(k);
                    break;
                }
                syn_undohack = 1;
                /* we know that m > 0 so the syn_rdinit() is executed */
            }
            for (i = m; i > 0; i--)
            {   if (syn_canrdinit())
                    initsubstatic(typearg_(t));
                else
                {   if (typesubsize_(t)) 
                        gendc0(i*sizeoftype(typearg_(t)));
                    else
                        typesubsize_(t) = globalize_int(m-i);
                    break;  /* optimise boring initialisation */
                }
            }
            syn_end_agg(note);
            break;
        }
        case t_content:
        {   Expr *init = syn_rdinit(t,0);
            while (init && h0_(init) == s_cast) init = arg1_(init);
            if (init == 0)
                gendcI(4,0);
            else if (h0_(init) == s_string)
            {   /* put the strings in code space, nonmodifiable -
                   rely on the fact that CG thinks it is between routines
                   (even for local static inits).                      */
                /* codeloc() is a routine temporarily for forward ref */
                gendcA(bindsym_(codesegment),codeloc());
                codeseg_stringsegs(((String *)init)->strseg, 0);
            }
            else if (h0_(init) == s_integer)
                gendcI(4,intval_(init));   /* casted int to pointer */
            else genpointer(init);
            break;
        }
    }
}

void initstaticvar(b,topflag)
Binder *b;
int topflag;
{   padstatic(alignoftype(b->bindtype));
    if (debugging(DEBUG_DATA))
        fprintf(stderr, "%.6x: %s%s\n", dataloc, topflag ? "":"; ",
               _symname(b->bindsym));
    bindaddr_(b) = dataloc;
    if (topflag) /* note: names of local statics may clash but cannot be
                    forward refs (except for fns which don't come here) */
    {   if (asmstream)  /* @@@ nasty space saving hack */
#ifdef TARGET_IS_XPUTER
            adddata(global_list4(0, bindsym_(b), LIT_LABEL,
		bindstg_(b) & bitofstg_(s_extern))), stuse_data += 16;
#else
            adddata(global_list3(0, bindsym_(b), LIT_LABEL)), stuse_data += 12;
#endif
        obj_symref(bindsym_(b),
                   (bindstg_(b) & bitofstg_(s_extern) ? xr_data+xr_defext :
                                                        xr_data+xr_defloc),
                   dataloc);
    }
    if (b == datasegment) return;      /* not really tidy */
    initsubstatic(bindtype_(b));
    padstatic(4);    /* for the next/last one */
    codeseg_flush();
}

/* The following routine removes generates statics, which MUST have been
   instated with instate_declaration().  Dynamic initialistions are turned
   into assignments for rd_block(), by return'ing.  0 means no init.
   Ensure type errors are noticed here (for line numbers etc.) */
Expr *genstaticparts(d,topflag)
DeclRhsList *d;
int topflag;
{   SET_BITMAP stg = d->declstg;
    Expr *dyninit = 0;        /* no dynamic init by default */
#ifdef TARGET_IS_XPUTER
#ifdef DBX
    if( dump_info && topflag ) db_static(d);
#endif    
#endif
    if (!(stg & b_fnconst)) switch (stg & -stg)
    {   case bitofstg_(s_typedef):
            break;
        case bitofstg_(s_auto):
            if (syn_canrdinit() && h0_(prunetype(d->decltype)) == t_subscript)
                cc_err("auto array $r may not be initialised", d->declname),
                syn_rdinit(0,4);
            else
            {   Expr *e = syn_rdinit(d->decltype,0);
                if (e)
                    dyninit = optimise0(
                        mkassign(s_assign, (Expr *)(d->declbind), e));
            }
            break;
        default:
            syserr("rd_decl/init(0x%x)", stg);
            /* assume static */
        case bitofstg_(s_static):
        case bitofstg_(s_extern):
            if (!(d->declstg & b_undef))
                initstaticvar(d->declbind, topflag);
            break;
    }
    return dyninit;
}

/* end of vargen.c */
