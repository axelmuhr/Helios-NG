/* $Id: emit.c,v 1.5 1994/08/03 09:55:34 al Exp $ */
#include "cchdr.h"
#include "util.h"
#include "xpuops.h"
#include "cg.h"
#include "AEops.h"

#define EMIT(a,b)	emit((a), (b))
#define STORECODE(a,b)	storeCode((a), (b))

#ifdef COMPILING_ON_ST
#define UNIONFIX 1
#endif
#ifdef COMPILING_ON_SUN4
#define UNIONFIX 1
#endif
#ifdef COMPILING_ON_DOS
#define UNIONFIX 1
#undef EMIT
#undef STORECODE
#define EMIT(a,b)	emit((a), (long)(b))
#define STORECODE(a,b)	storeCode((a), (long)(b))
#endif

extern int ssp;
static int next_label_name;
int deadcode;

int maxprocsize;
char *maxprocname;

Block *topblock,*curblock;
Tcode *blockcode, *curcode;
Literal *litpoolhead, *litpoolend;
int litpoolsize;

FloatCon *fc_unsfix;

int addFpConst();

extern char* opName();

void codebuf_init() 
{
	next_label_name = 0;
	
	fc_unsfix = real_of_string("2147483648.5", bitoftype_(s_double));
}

void codebuf_reinit() {}

void codeseg_flush() {}

void codebuf_reinit2()
{
	curblock = topblock = NULL;
	curcode = blockcode = NULL;
	litpoolhead = litpoolend = NULL;
	litpoolsize = 0;
	deadcode = TRUE;
}

LabelNumber *nextlabel()
{
    LabelNumber *w = GlobAlloc(sizeof(LabelNumber));
    w->block = DUFF_ADDR;          /* unset label - illegal pointer */
    w->refs = 0;
    w->defn = -1;
    w->name = next_label_name++;
    return w;
}

Block *newblock(l)
LabelNumber *l;
{
	Block *b = SynAlloc(sizeof(Block)); block_cur += sizeof(Block);
	b->code = NULL;
	b->next = NULL;
	b->jump = p_noop;
	b->flags = in_loop?BlkFlg_align:0;
	b->succ1 = NULL;
	b->succ2 = NULL;
	b->prev = curblock;
	if( curblock != NULL ) curblock->next = b;
	b->lab = l;
	curblock = b;
	if( topblock == NULL ) 
	{
	    topblock = b;
	    l-> refs++;
	}
	blockcode = curcode = NULL;
	return b;
}

void align_block()
{
	curblock->flags |= BlkFlg_align;
}

Block *start_new_basic_block(l)
LabelNumber *l;
{
	Block *b;
	EMIT( f_j, l);
	l->block = b = newblock(l);
	
	deadcode = FALSE;
/* trace("Start block %x %x",b,l); */
	return b;
}

emitcasebranch(table,size,l)
LabelNumber **table;
int size;
LabelNumber *l;
{
	Block *b = start_new_basic_block(l);

	b->jump = p_case;
	b->operand1.table = table;
	b->operand2.tabsize = size;
	l->refs++;	
	deadcode = TRUE;
}

emitprofile()
{
	trace("EmitProfile");
}

         
duplicate(rsort)
RegSort rsort;
{
	/* @@@ T414 variant needed here		*/
	if( floatingHardware )
	{
		EMIT( f_opr, rsort==INTREG?op_dup:op_fpdup );
		if( rsort == INTREG ) pushInt(); else pushFloat();
	}
	else {
		VLocal *v = pushtemp( INTREG );
		EMIT( p_ldvl, v ); pushInt();
		poptemp( v, INTREG );
	}
}

void linearize_code() {}

/* literal pool handling */

emitconstant( i )
int i;
{
	int offset = addlitint( i );
	if( litpool == NULL ) syserr("Uninitialised literal pool");
	EMIT( p_ldvl, litpool );
	EMIT( f_ldnl, offset );
}

emitstring(s)
StringSegList *s;
{
	int offset = addlitstring( s );
	if( litpool == NULL ) syserr("Uninitialised literal pool");
	EMIT( p_ldvl, litpool );
	EMIT( f_ldnlp, offset );
}

emitFpConst( x , rsort)
FloatCon *x;
RegSort rsort;
{
   if (floatingHardware )
   {
      if (isFpZero(x, rsort))
         EMIT( f_opr, rsort == FLTREG ? op_fpldzerosn : op_fpldzerodb );
      else
      {
         int offset = addFpConst(x, rsort);
         if( litpool == NULL ) syserr("Uninitialised literal pool");
 	 EMIT( p_ldvl, litpool ); pushInt();
 	 EMIT( f_ldnlp, offset );
	 EMIT( f_opr,rsort == FLTREG ? op_fpldnlsn : op_fpldnldb );
	 popInt();
      }
      pushFloat() ;
   }
   else
   {
	/* T414 variant */
        int offset = addFpConst(x, rsort);
	if( litpool == NULL ) syserr("Uninitialised literal pool");
	EMIT( p_ldvl, litpool ); pushInt();
	if( rsort == FLTREG ) EMIT( f_ldnl, offset ); 
	else EMIT( f_ldnlp, offset ); /* doubles are manipulated via pointers */
   }
}

int addlitint( i )
int i;
{
	Literal *l = litpoolhead;

	/* first see if we already have this constant in the pool. */
	for(; l != NULL; l = l->next )
	{
		if( l->type == lit_integer && l->v.i == i )
			return l->offset;
	}
	
	l = BindAlloc(sizeof(Literal));
	l->next = NULL;
	l->type = lit_integer;
	l->v.i = i;
	l->offset = litpoolsize;
	litpoolsize += 1;
	if( litpoolend != NULL ) litpoolend->next = l;
	litpoolend = l;
	if( litpoolhead == NULL ) litpoolhead = l;
	return l->offset;
}

/* @@@ addlitstring - possibly share strings ? */
int addlitstring( s )
StringSegList *s;
{
	int stringlen = 0;
	Literal *l = BindAlloc(sizeof(Literal));
	l->next = NULL;
	l->type = lit_string;
	l->v.s = s;
	l->offset = litpoolsize;

	while( s != NULL )
	{
		stringlen += s->strseglen;
		s = cdr_((List *)s);
	}
	litpoolsize += padstrlen(stringlen)/4;	/* size in words */
	
	if( litpoolend != NULL ) litpoolend->next = l;
	litpoolend = l;
	if( litpoolhead == NULL ) litpoolhead = l;
	return l->offset;
}

/* Look down the literal list for an identical constant,
   if none is present then add a new constant */
int addFpConst( x, rsort )
FloatCon *x;
RegSort rsort;
{
   Literal *lit = litpoolhead;
   bool found   = FALSE;
   
   if (debugging (DEBUG_CG)) 
       trace("Addfpconst %8x %8x", x->floatbin.db.msd, x->floatbin.db.lsd);
   
   for (; lit != NULL; lit = lit->next)
   {
     switch (lit -> type)
     {
        case lit_floatDb: 
           if(rsort != DBLREG) break;
           if ( (lit->v.db.lsd) == (x->floatbin.db.lsd) &&
		(lit->v.db.msd) == (x->floatbin.db.msd) ) found = TRUE;
              break;
             
        case lit_floatSn:
           if( rsort != FLTREG ) break;
           if ( (lit->v.fb.val) == (x->floatbin.fb.val) ) found = TRUE;
        case lit_string: break;
      }
      if (found) break;
   }
   
   if (found)
   {
      if (debugging (DEBUG_CG)) 
          trace("Addfpconst found value at offset #%x", lit->offset);
      return lit->offset;
   }
   else
   {
	Literal *l = BindAlloc(sizeof(Literal));
	l->next = NULL;
	l->type = rsort == DBLREG ? lit_floatDb : lit_floatSn ;
	l->v.db = x->floatbin.db;
	l->offset = litpoolsize;
	litpoolsize += rsort==DBLREG ? 2 : 1 ;
	if( litpoolend != NULL ) litpoolend->next = l;
	litpoolend = l;
	if( litpoolhead == NULL ) litpoolhead = l;
        if (debugging (DEBUG_CG)) 
            trace("Addfpconst added value at offset %d", l->offset);
	return l->offset;
   }
}

/* This version is ONLY for IEEE format arithmetic */
bool isFpZero( x, rsort)
FloatCon *x;
RegSort rsort;
{
  if (rsort == DBLREG)
     if (x->floatbin.db.msd != 0)
        return FALSE;
  return (x->floatbin.fb.val == 0);
}

emitcjfix()
{
	EMIT( f_opr, op_diff );
}

#ifndef UNIONFIX
emit(op,opd)
Xop op;
Operand opd;
{
#else
emit(op,opdarg)
Xop op;
long opdarg;
{
	/* Lattice C gets union arguments wrong, the following is a fix */
	Operand opd;
	opd.value = opdarg;
#endif

	/* some small optimisations */
	switch( op )
	{
	case f_ldc:
		if( smallint(opd.value) ) break;
		else if( opd.value < (int)(MinInt+maxsmallint) )
		{
			STORECODE( f_opr, op_mint );
			opd.value &= ~MinInt; /* Careful with overflow ! */
			op  = f_adc;
			/* fall through to adc case */
		}
		else if( opd.value == ~MinInt )
		{
			STORECODE( f_opr,op_mint );
			STORECODE( f_opr,op_not );
			return;
		}
		else
		{
			emitconstant( opd.value );
			return;
		}
	case f_adc:
		/* if we are in an address expression, convert adc to	*/
		/* ldnlp if possible.					*/
		if( (opd.value % 4) == 0 && addrexpr )
		{
		    op = f_ldnlp;
		    opd.value = opd.value>>2;
		}
                /* Fall through for ldnlp */

	case f_ldnlp:	
		if( opd.value == 0 ) return;
	        break;
	 
	/* Label reference instructions */
	case p_ldpi:       
#ifdef JIMS
	case f_j:
	case f_cj:
	case p_j:
#endif
	        if (!deadcode) opd.label->refs++;

                break;
                
        /* VLocal reference instructions */
        case p_ldvl:
        case p_ldvlp:
        case p_stvl:
                if (!deadcode)
                   opd.local -> refs += 1; /* (1 << loopdepth ?) */ 
             
	default: break;
	}

#ifdef UNIONFIX
	STORECODE(op, opd.value);
#else
	STORECODE(op, opd);
#endif
}

/* The real routine to store a code tem into the code structure */
#ifndef UNIONFIX
storeCode(op,opd)
Xop op;
Operand opd;
{
	Tcode *t;
#else
storeCode(op,opdarg)
Xop op;
long opdarg;
{
	/* Lattice C gets union arguments wrong, the following is a fix */
	Operand opd;
	Tcode *t;
	opd.value = opdarg;
#endif


	if(debugging(DEBUG_CG))
		trace("%s #%x #%x %s",op!=f_opr?opName(op):opName(opd.value),
			op,opd.value,deadcode ? "dead":"");

	if( deadcode ) return;
	/* note that f_j and f_cj never get into the code but control the
	 * generation of the flow graph.
	 */
	if( op == f_j || op == p_j )
	{
		deadcode = TRUE;
		if( curblock -> jump == p_noop ) curblock->jump = op;
		if (curblock -> succ1 == NULL)
		    curblock->succ1  = opd.label;
	        else
	            curblock->succ2 = opd.label;
		(opd.label -> refs) ++;
		return;
	}
	if( op == f_cj )
	{
		/* conditional branches terminate the current block and
		 * set the label as this blocks alternate successor.
		 * If the instruction above this is an eqc 0, then
		 * this is removed, (reversing the branch sense), and
		 * the label is set as the block successor ...
		 */
#ifdef JIMS
		if ((curcode -> op == f_eqc) && (curcode -> opd.value == 0))
		{ /* Need to remove this item. We simply make it a noop */
		   curcode -> op    = p_noop;
		   curblock -> succ1 = opd.label;
		}
		else
		{
		    curblock->succ2 = opd.label;
		} 
#else
		curblock->succ2 = opd.label;
#endif

		(opd.label -> refs) ++;
		curblock->jump = f_cj;
		start_new_basic_block(nextlabel());
		return;
	}

	t = SynAlloc(sizeof(Tcode));
	t->next = NULL;
	t->op   = op;
	t->opd.value  = opd.value;

	if( curcode != NULL ) curcode->next = t;
	curcode = t;
	if( blockcode == NULL )
	{ blockcode = t; curblock->code = t; }
	curblock -> current = curcode;
	if (op == p_ret)
	   deadcode = TRUE;
}

