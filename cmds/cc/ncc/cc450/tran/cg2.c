/*{{{  Includes */
/* $Id: cg2.c,v 1.2 1995/08/04 11:19:48 nickc Exp $ */
#ifdef __old
#include "cchdr.h"
#include "AEops.h"
#include "util.h"
#include "xpuops.h"
#include "cg.h"
#else
#include "globals.h"
#include "builtin.h"
#include "store.h"
#include "util.h"
#include "aeops.h"
#include "xpuops.h"
#include "cg.h"
#endif
/* This file contains code for generating assignment statements and accessing
   variables (in particular indexed or indirected variables).
   
   Also the translation between AEops and transputer instructions
*/
/*}}}*/
/*{{{  Externs and fwd defs */

extern int maxssp;
extern int ssp;
extern int maxcall;
extern int depth();

extern bool is_same();
extern void emitFloatCon( FloatCon * x , RegSort rsort );
extern void emitFpConst ( FPConst fc, RegSort	rsort );

/* forward references */
void cg_storein(Expr *, AEop, int);
void cg_var(Binder *, AEop, int, int, int);
void cg_indirect(Expr *, int, int, int, int);
void cg_doubleassign (Expr *, Expr *, int, int);
void codeFpCall (AEop, int);

static int loadOp, storeOp, loadAddressOp;
static int vardepthneeded;

struct AssignInfo
{
	Expr	*Target;
	int	TargetUsed;
	Expr	*Source;
	int	SourceEval;
} Assign = { 0, 0, 0, 0 };


#define islocalvar_(b)\
	(h0_(b) == s_binder && (bindstg_((Binder *)b) & bitofstg_(s_auto))) 

#define isindirection_(e) (h0_(e) == s_dot || h0_(e) == s_content)

/* #define dbtrace if(debugging(DEBUG_CG)) trace */
extern void trace (char *, ...);
#define dbtrace trace

/*}}}*/
/*{{{  set_binder_ops */

Binder *set_binder_ops( Binder *b )
{
	switch( bindstg_(b) & PRINCSTGBITS )
	{
	case bitofstg_(s_auto):
	        loadOp = p_ldvl;
	        storeOp= p_stvl;
	        loadAddressOp = p_ldvlp;
	        vardepthneeded = 0;
	        b = (Binder *)bindxx_(b);
	        break;

	case bitofstg_(s_extern):
	        loadOp = p_ldx;
	        storeOp= p_stx;
	        loadAddressOp = p_ldxp;
	        vardepthneeded = 1;	        
		break;

	case bitofstg_(s_static):
	        loadOp = p_lds;
	        storeOp= p_sts;
	        loadAddressOp = p_ldsp;
	        vardepthneeded = 1;	        
		break;
	
	default:
		syserr("Funny storage class %#x\n",bindstg_(b));

	}
	return b;
}
/*}}}*/
/*{{{  substvar */

/* Substitute the variable binder b for the expression s in the	*/
/* expression x. At present this is only used to substitute in	*/
/* the increment expression for ++ or --, hence the restricted	*/
/* set of things tested.					*/

Expr *substvar(x,b,s)
Expr *x;
Binder *b;
Expr *s;
{
	if( is_same(x,s) ) return (Expr *)b;
	
	switch( h0_(x) )
	{
	case s_cast:
		arg1_(x) = substvar(arg1_(x),b, NULL);
		return x;

	case s_plus:
	case s_minus:
		arg1_(x) = substvar(arg1_(x),b, NULL);
		arg2_(x) = substvar(arg2_(x),b, NULL);
		return x;
		
	default:
		return x;
	}
}
/*}}}*/
/*{{{  isstructurepointer */
bool isstructurepointer(x)
Expr *x;
{
    	TypeExpr *t = prunetype(typeofexpr(x));

    	dbtrace("h0_(x) %s",symbol_name_(h0_(x)));
    	dbtrace("typeexpr %s",symbol_name_(h0_(t)));

    	if( h0_(t) != t_content ) return FALSE;
    	
    	t = typearg_(t);

    	if( h0_(t) != s_typespec ) return FALSE;

    	dbtrace("typeexpr %s",symbol_name_(h0_(t)));
	dbtrace("typemap %x size %d",typespecmap_(t),sizeoftype(t));    	
    	
    	if( typespecmap_(t) == bitoftype_(s_struct)	||
    	    typespecmap_(t) == bitoftype_(s_class)	||
	    typespecmap_(t) == bitoftype_(s_union)
        ) return TRUE;

	return FALSE;    	    
}
/*}}}*/
/*{{{  aligned_short_struct_access */
bool aligned_short_struct_access(x)
Expr *x;
{
	dbtrace("assa: h0 %s",symbol_name_(h0_(x)));

	if( h0_(x) == s_content )
	{

		x = arg1_(x);

		dbtrace("assa: arg1_h0 %s",symbol_name_(h0_(x)));

		if( h0_(x) != s_plus ) return FALSE;

		dbtrace("assa: arg1_arg1_h0 %s",symbol_name_(h0_(arg1_(x))));

		if( !isstructurepointer(arg1_(x)) ) return FALSE;

		dbtrace("assa: arg1_arg2_h0 %s",symbol_name_(h0_(arg2_(x))));

		if( !integer_constant(arg2_(x)) ) return FALSE;

		dbtrace("assa: r2 %d",result2);
	
		return (result2&3)==0;
	}
	else if( h0_(x) == s_dot )
	{
		Expr *x1 = arg1_(x);

		dbtrace("assa: h0_(x1) %s",symbol_name_(h0_(x1)));

		if( h0_(x1) == s_content ) x1 = arg1_(x1);

		dbtrace("assa: h0_(x1) %s",symbol_name_(h0_(x1)));

		if( !isstructurepointer(x1) ) return FALSE;

		dbtrace("assa: dotoff %d",exprdotoff_(x));
		
		return (exprdotoff_(x)&3)==0;
	}

	return FALSE;
}
/*}}}*/
/*{{{  cg_scalarAssign */

/* We have found a scalar assignment statement,
   this is either s_assign    e.g.  a = ...;
   or             s_displace  e.g.  a++;
   
   in addition valneeded may or may not be set depending on whether we
   need to leave the value in the register as well
   e.g. a = b = 3;
   vs.  a = 3;

   This is part of an attempt to improve the code for simple assignments
   etc. More work is intended to be done here. In particular evaluating
   the depth of the two sides, to evaluate them in the best order, and
   dealing with valneeded by using dup when we know it is safe...
   For expediency at the moment, this just calls the old routines.
   This should also check for (e.g.) float x, y; x = y
   and generate ldl y stl x rather than ldlp y fpldnlsn  ldlp x fpstnlsn
*/
void cg_scalarAssign(x,valneeded)
Expr *x;
bool valneeded;
{
    int mclength  = mcrepofexpr(x);
    int mcmode    = (mclength>>24) & 0xff;
    Expr *target  = arg1_(x), *source = arg2_(x);

    AEop targetOp = h0_(target);
    RegSort rsort = (mclength &= 0x00ffffff,
	(mcmode!=2) ? INTREG : (mclength==4 ? FLTREG : DBLREG ));

    int targetmode = mcrepofexpr(target);
    int sourcemode = mcrepofexpr(source);

    struct AssignInfo aisave;
    int atu;

    int evalsource = TRUE;
	
    push_trace("cg_scalarAssign");
    
    if(debugging(DEBUG_CG))
    {
       trace("Cg_scalarAssign: %s valneeded %d",symbol_name_(h0_(x)),valneeded);
       trace("mcmode %x targetmode %x sourcemode %x",(mcmode<<24)|mclength,
       		targetmode,sourcemode);
#if 1
       eprintf("Target "); pr_expr(target);
       eprintf("\nSource "); pr_expr(source);eprintf("\n");
#endif
    }

    /* We sometimes get an unnecessary cast in the source...		*/
    if( (h0_(source) == s_cast) && mcrepofexpr(arg1_(source)) == sourcemode )
	source = arg1_(source);

    if( h0_(source) == s_cast && h0_(arg1_(source)) == s_cast )
    {
    	Expr *s = arg1_(source);

	source = mk_expr1(s_cast, typeofexpr(source), arg1_(s) );
    }
	
    /* The following seems wrong, I would have expected source to	*/
    /* contain an appropriate cast if it differed from target. It does	*/
    /* not always appear to do so. Is this a bug in cfe?		*/
    if( sourcemode != targetmode )
    {
    	dbtrace("add cast to %x",targetmode);
    	source = mk_expr1( s_cast, typeofexpr(target), source );
    	sourcemode = targetmode;
    }

    	
    /* A displace in a void context can be replaced with a simple */
    /* assignment.						  */
    if( h0_(x) == s_displace && !valneeded ) h0_(x) = s_assign;

    if( !floatingHardware && rsort == DBLREG ) 
    {
	cg_doubleassign( target, source, h0_(x), valneeded );
	pop_trace();
	return;
    }

	/* The following sections attempt to optimise various common	*/
	/* constructs. These are particularly important for short and	*/
	/* byte sized values on non-T9000 processors.			*/
    
	
	    if( !valneeded && targetOp == s_binder && h0_(source) == s_cond )
	    {
		/* convert a = b ? c : d into b ? (a = c) : (a = d)	*/
	
		TypeExpr *t = typeofexpr(target);
		Expr *e = mk_expr3(s_cond,
				   t,
				   arg1_(source),
				   mk_expr2(s_assign, t, target, arg2_(source)),
				   mk_expr2(s_assign, t, target, arg3_(source)));
		pp_expr(e,0);
		cg_exprvoid(e);	   	    
		pop_trace(); 
		return;
	    }
	    if
	    (
		h0_(x) == s_assign 					&&
		(
			h0_(source) == s_assign 				||
			(h0_(source) == s_cast && h0_(arg1_(source)) == s_assign)
		)
	    )
	    {
	    	/* convert a = b = c; into b = c; a = c; if c is a	*/
	    	/* local or a constant					*/
	
	    	TypeExpr *t = typeofexpr(x);
		Expr *b = source;
	    	Expr *c = NULL;
	    	Expr *bc = NULL;
	    	Expr *cc = NULL;
		Expr *e, *a1, *a2, *a3;
		int cmode, bmode;
		int clength, blength;
		
	    	if( h0_(b) == s_cast ) bc = b, b = arg1_(b);
	
		c = arg2_(b);
		b = arg1_(b);
		
	    	if( h0_(c) == s_cast ) cc = c, c = arg1_(c);	
	
	    	bmode = mcrepofexpr(b);
	    	blength = bmode&7; bmode >>= 24;
	    	cmode = mcrepofexpr(c);
	    	clength = cmode&7; cmode >>= 24;
	    	    	
	    	dbtrace("target %x mode %x",target,mcrepofexpr(target));
	    	dbtrace("b %x mode %x",b,mcrepofexpr(b));
	    	dbtrace("c %x mode %x",c,mcrepofexpr(c));
	    	if( bc ) dbtrace("bc %x mode %x",bc,mcrepofexpr(bc));
	    	if( cc ) dbtrace("cc %x mode %x",cc,mcrepofexpr(cc));
	    	
	
		/* The extra cases in the following test attempt to filter out	*/
		/* the cases where assignment through b has an effect on the	*/
		/* value which arrives in c as a result of size changing and/or	*/
		/* sign extension.						*/
	    	if
		(
			(
				islocalvar_(c) 				&&
				!(bmode != mcmode && mclength>blength)	&&
				!(cmode != bmode  && clength>blength)	&&
				!(cmode != mcmode && mclength>clength)
			)						||
			h0_(c) == s_integer
		)
	    	{
	    		/* generate b = (cast)c; a = (cast)c; [c]		*/
	
	    		dbtrace("(a = b = c) c local");
	    		
	    		a1 = c;
	    		if( cc != NULL ) a1 = cc;
	    		a1 = mk_expr2(s_assign, typeofexpr(b), b, a1 );
	
	    		a2 = c;
	    		if( bc != NULL ) a2 = mk_expr1(s_cast, typeofexpr(bc), a2);
	    		a2 = mk_expr2( s_assign, t, target, a2);
	    		
	    		e = mk_expr2( s_comma, t, a1, a2 );
	
			pp_expr(e,0);
			cg_exprvoid(e);
			if( valneeded ) cg_expr(c);
			pop_trace(); 
			return;
	    	}
	    	else if( islocalvar_(b) )
	    	{
	    		/* generate b = (cast)c; a = (cast)b; [b]		*/
	
	    		dbtrace("(a = b = c) b local");
	    		
	    		a1 = c;
	    		if( cc != NULL ) a1 = cc;
	    		a1 = mk_expr2(s_assign, typeofexpr(b), b, a1 );
	
	    		a2 = b;
	    		if( bc != NULL ) a2 = mk_expr1(s_cast, typeofexpr(bc), a2);
	    		a2 = mk_expr2( s_assign, t, target, a2);
	    		
	    		e = mk_expr2( s_comma, t, a1, a2 );
	
			pp_expr(e,0);
			cg_exprvoid(e);
			if( valneeded ) cg_expr(b);
			pop_trace();
			return;
	    	}
	    	else if( islocalvar_(target) )
	    	{
	    		/* generate a = (cast)c; b = (cast)a; [a]		*/
	
	    		dbtrace("(a = b = c) a local");
	    		
	    		a1 = c;
	    		if( cc != NULL ) a1 = cc;
	    		a1 = mk_expr2(s_assign, typeofexpr(target), target, a1 );
	
	    		a2 = target;
	    		if( bc != NULL ) a2 = mk_expr1(s_cast, typeofexpr(bc), a2);
	    		a2 = mk_expr2( s_assign, typeofexpr(b), b, a2);
	    		
	    		e = mk_expr2( s_comma, t, a1, a2 );
	
			pp_expr(e,0);
			cg_exprvoid(e);
			if( valneeded ) cg_expr(b);
			pop_trace();
			return;
	    	}
	    	else
	    	{
	    		/* generate t = c; b = t, a = t; [t]			*/
	
	    		Binder *tmp;
	    		TypeExpr *tmptype = typeofexpr(b);
	
	    		tmp = gentempbinder(tmptype);
	    		
	    		dbtrace("(a = b = c) none local");
	
	    		a1 = c;
			if( cc != NULL ) a2 = mk_expr1(s_cast, typeofexpr(cc), a1);
			a1 = mk_expr2(s_assign, tmptype, (Expr *)tmp, a1);
	
	    		a2 = mk_expr2(s_assign, typeofexpr(b), b, (Expr *)tmp);
	
			a3 = (Expr *)tmp;
			if( bc != NULL ) a3 = mk_expr1(s_cast, typeofexpr(bc), a3);
			a3 = mk_expr2(s_assign, typeofexpr(target), target, a3);
	
			e = mk_expr2( s_let,
					t,
					(Expr *)mkBindList( 0, tmp),
					mk_expr2(s_comma, tmptype,
						a1,
					mk_expr2(s_comma, tmptype,    		
	   					a2,
	   					a3)));
	
			pp_expr(e,valneeded);
			cg_expr1(e, valneeded);
			pop_trace();
			return;
	   					
	    	}	
	
	#if 0
	    	if
		( 	islocalvar_(c) 					||
			((h0_(c) == s_cast) && (islocalvar_(arg1_(c))))	|| 
			h0_(c) == s_integer
		)
	    	{
	    		TypeExpr *t = typeofexpr(source);
	    		Expr *e = mk_expr2(s_assign, t, b, c);
			dbtrace("(a = b = c)");
			pp_expr(e,0);
			cg_exprvoid(e);
			if( mcrepofexpr(b) == targetmode )
				e = mk_expr2(s_assign, typeofexpr(b), target, b);
	    		else 	e = mk_expr2(s_assign, typeofexpr(target), target, c);
			pp_expr(e,0);
			cg_exprvoid(e);
			if( valneeded ) cg_expr(c);
			pop_trace(); 
			return;
	    	}
	#endif
	    }
	
	    if
	    (
	        (mcmode < 2)						&&
		(mclength <= 4)						&&
		(h0_(target) == s_binder)				&&
		(h0_(source) == s_binder)
	    )
	    {
	    	/* Handle direct assignment of equal type int variables as if	*/
	    	/* they were full width ints. This is safe because variables	*/
	    	/* are always kept zero padded in memory and are sign extended	*/
	    	/* on load.							*/
		if( targetmode == sourcemode )
		{
			dbtrace("(a = b) same type");
	    		cg_var( (Binder *)source, s_content, mcmode, 4, TRUE );
	    		cg_var( (Binder *)target, h0_(x), mcmode, 4, valneeded );
	    		pop_trace();
			return;
	    	}
	    }
	
	    /* Direct assignment between float/double variables	can be	*/
	    /* done directly, no need to use FP stack.			*/
#if 0	    
	    if
	    (
	    	!valneeded					&&
	    	(mcmode == 2)					&&
	    	(h0_(target) == s_binder)			&&
	    	(h0_(source) == s_binder)
	    )
	    {
	
		/* @@@ what if there is insufficient stack depth? */
		
		Binder *b = set_binder_ops((Binder *)source);
	
		dbtrace("(float a = b)");
	
		emit( loadOp, b );
		if( mclength == 8 )
		{
			emit( loadAddressOp, b );
			emit( f_ldnl, 1 );
		}
	
		b = set_binder_ops( (Binder *)target );
	
		if( mclength == 8 )
		{
			emit( loadAddressOp, b );
			emit( f_stnl, 1 );
		}
		emit( storeOp, b );
	
		/* @@@ valneeded? */
	
		pop_trace();
		return;
	    }
#endif
	    /* Try to optimise short assignment of a variable to an		*/
	    /* indirection. So long as the source variable is >= 2 bytes wide	*/
	    /* we can just move the bottom 2 bytes out into the dest.		*/
	    if
	    (
		(!lxxAllowed) 						&& 
		(mclength == 2) 					&& 
		(h0_(x) == s_assign)					&&
		(h0_(target) == s_dot || h0_(target) == s_content)	&&
		(
		    (h0_(source) == s_binder) 				||
		    ((h0_(source) == s_cast) && (h0_(arg1_(source)) == s_binder))
		)
	    )
	    {
	    	Expr *s = source;
		int sm;
	    	if( h0_(source) == s_cast ) s = arg1_(s);
	
		sm = mcrepofexpr(s);
	
		dbtrace("short(*a = b) lxx %d mcl %d sm %x tm %x h0s %d h0t %d",lxxAllowed,mclength,
						sm,targetmode,h0_(s),h0_(target));
	
	    	if( (sm&0x00FFFFFF) >= 2 )
	    	{
		    	VLocal *v = NULL;
	    		if( ida != 3 ) v = pushtemp( INTREG );
	    		cg_binary( s_nothing,
				take_address(s),
				take_address(target),
				0, INTREG );
	    		emit( f_ldc, 2 );
	    		emit( f_opr, op_move );
	    		setInt(FullDepth);
	    		if( v != NULL ) poptemp(v);
	    		if( valneeded ) cg_expr(s);
	    		pop_trace(); 
			return;
		}
	    }
	
	    /* Try to optimise a = *b for shorts where a is a local variable.	*/
	    /* In this case we can use a as a temporary variable to move 	*/
	    /* the value out of b. If necessary we must then sign extend a.	*/
	    if
	    (
		(!lxxAllowed) 						&& 
		(h0_(x) == s_assign)					&&
		islocalvar_(target)					&&
		(
		    isindirection_(source)				||
		    ((h0_(source) == s_cast) && isindirection_(arg1_(source)))
		)
	    )
	    {
	    	Expr *s = source;
		int sm;
	
	    	if( h0_(source) == s_cast ) s = arg1_(s);
	
		sm = mcrepofexpr(s);
	
	#if 1
		if
		(
			((sm&7) == 1)			&&
			((targetmode&7) >= 1)
		)
		{
			dbtrace("byte (a = *b) sm %x",sm);
	
			if( structure_function_value(s) ) cg_expr(s);
			else
			{
				/* arrange to load the byte without any	*/
				/* sign extension			*/
				s = cg_content_for_dot(s);
				pp_expr(s, TRUE);
				if( h0_(s) == s_content ) s = arg1_(s);
				cg_indirect( s, s_content, 1, 1, TRUE );
				if( (sm>>24) == 0 /*&& mcmode == 0*/ && mclength != 1)
				{
					/* we must do some sign extending */
					emitxword( 1 );
					if( (targetmode&7) < 4 ) emitmask( targetmode&7 );
				}
			}
	
			emit( p_stvl, bindxx_((Binder *)target) );
	    		if( valneeded ) 
			{
				emit( p_ldvl, bindxx_((Binder *)target) );
				if( mcmode == 0 && mclength < 4 ) emitxword( mclength );
			}
	    		else popInt();
	    		pop_trace(); 
			return;
		}
#if 0
		else
#endif
	#endif /* 1 above */
#if 0
		/*
		 * The following "optimisation" causes
		 *
		 *	short array[4] = {10, 20, 30, 40};
		 *	long code = 3;
		 *	code = array[code];
		 *
		 * to be incorrectly compiled.  The assembler for accessing the
		 * array becomes something like
		 *
		 *	ldc	3
		 *	stl	0	-- set code to 3
		 *
		 *	ldl	0      
		 *	stl	0	-- set code to 0 (WRONG!)
		 *
		 *	ldl	0
		 *	etc, etc, etc
		 *
		 * I've just ignored this entire piece of code for now.  I'll
		 * have a look at it later, time permitting.  Tony 13/1/95.
		 */
		if
		(
			((sm&7) == 2)			&&
			((targetmode&7) >= 2)		&&
			!aligned_short_struct_access(s)
		)
		{
			VLocal *v = NULL;
		
			dbtrace("short (a = *b) lxx %d mcl %d sm %x tm %x h0s %d h0t %d",lxxAllowed,mclength,
						sm,targetmode,h0_(s),h0_(target));
	
	    		if( ida != 3 ) v = pushtemp( INTREG );
			emit( f_ldc, 0 );
			emit( p_stvl, bindxx_((Binder *)target) );
	    		cg_binary( s_nothing,
				take_address(s),
				take_address(target),
				0, INTREG );
	    		emit( f_ldc, 2 );
	    		emit( f_opr, op_move );
	    		setInt(FullDepth);
	    		if( (sm>>24) == 0 && (targetmode&7) > (sm&7) )
	    		{
				/* sign extend target if necessary	*/
	    			emit( p_ldvl, bindxx_((Binder *)target) );
	    			emitxword( 2 );
	    			emit( p_stvl, bindxx_((Binder *)target) );
	    		}
	    			
	    		if( v != NULL ) poptemp(v);
	    		if( valneeded )
			{
				emit( p_ldvl, bindxx_((Binder *)target) );
				pushInt();
			}
	    		pop_trace(); 
			return;
	    	}
#endif
	
	    }
	
	
	    /* Try to optimise the assignment of an indirection to an	*/
	    /* indirections for shorts					*/
	    if
	    (
		!lxxAllowed 						&&
		mclength == 2 						&&
		(h0_(source) == s_dot || h0_(source) == s_content) 	&&
		(h0_(target) == s_dot || h0_(target) == s_content)
	    )
	    {
	    	VLocal *v = NULL;
		dbtrace("short (*a = *b)");
	    	if( ida != 3 ) v = pushtemp( INTREG );
	    	cg_binary( s_nothing,
			take_address(source),
			take_address(target),
			0, INTREG );
	    	emit( f_ldc, 2 );
	    	emit( f_opr, op_move );
	    	setInt(FullDepth);
	    	if( v != NULL ) poptemp(v);
	    	if( valneeded ) cg_expr(source);
	    	pop_trace(); 
		return;
	    }
	
	    /* Try to optimise an assignment of the form x = y++ or x = y--.	*/
	    /* We do this by rewriting it as x=y; y = y(+/-)1; The exact form	*/
	    /* depends on whether x, y or neither is a local variable.          */
	    /* Note that things are somewhat complicated if x and y are of 	*/
	    /* differing sizes.							*/
	    if( h0_(x) == s_assign &&
		(
		   h0_(source) == s_displace ||
		  (h0_(source) == s_cast && h0_(arg1_(source)) == s_displace)
	        )
	      )
	    {
		Expr *x = target;
	    	Expr *y = arg1_(source);
	    	Expr *c = NULL;
	    	Expr *s = NULL;
		int xmode, ymode;
		TypeExpr *t = typeofexpr(x);
		Expr *e, *e1;
	
		/* jump over cast, but remember it */
		if( h0_(source) == s_cast )
		{
			c = source;
			s = arg2_(y);
			y = arg1_(y);
		}
		else
		{
			c = NULL;
			s = arg2_(source);
		}
	
		xmode = mcrepofexpr(x);
		ymode = mcrepofexpr(y);
	
		dbtrace("(a = b++)");
		dbtrace("x %d mode %x",x,xmode);
		dbtrace("y %d mode %x",y,ymode);
		dbtrace("s %d mode %x",s,mcrepofexpr(s));
		if( c ) dbtrace("c %d mode %x",c,mcrepofexpr(c));
	
		if( is_same(x,y) )
		{
			/* an expression of the form x = x++; has no	*/
			/* effect. Treat it as null, except where a	*/
			/* value is required.				*/
	
			dbtrace("a = a++");
			if( valneeded ) cg_expr1(e,valneeded);
		}
		else if
		( 
			islocalvar_(y) 					&&
			!( islocalvar_(x) && (mcrepofexpr(x)&7) == 4)
		)
		{
			/* If y is local				*/
		    	/* convert x = y++; into x = (cast)y, y = y+1;	*/
	
			dbtrace("local b");
			/* include cast in assignment if needed		*/
			if( c != NULL )
			{
				e1 = mk_expr2( s_assign, t, x,
					mk_expr1( s_cast, typeofexpr(c), y ) );
			}
			else e1 = mk_expr2( s_assign, t, target, y );
			
			e = mk_expr2( s_comma,
				    t,
				    e1,
				    mk_expr2( s_assign, typeofexpr(y), y, s)
				  );
			pp_expr(e,0);
			cg_exprvoid(e);
			if( valneeded ) cg_expr(y);
		}
		else if
		(
			islocalvar_(x)					&&
			((xmode&7) >= (ymode&7))
		)
		{
			/* If x is local					*/
		    	/* convert x = y++; into x = (cast)y, y = (cast)x+1;	*/
	
		    	Expr *e2 = s;
		    	Expr *s2 = s;
	
			dbtrace("local a");
	
			/* include cast in assignement if needed	*/
			if( c != NULL )
			{
				e1 = mk_expr2( s_assign, t, x,
					mk_expr1( s_cast, typeofexpr(c), y ) );
			}
			else e1 = mk_expr2( s_assign, t, target, y );
			
			if( h0_(e2) == s_cast )	s2 = e2 = arg1_(e2);
	dbtrace("e2 %s arg1 %s",symbol_name_(h0_(e2)),symbol_name_(h0_(arg1_(e2))));
	
			e2 = substvar(e2, (Binder *)x, y);
	
			if( s2 != s ) e2 = mk_expr1(s_cast, typeofexpr(s), e2);
	
			e = mk_expr2( s_comma,
				    t,
				    e1,
				    mk_expr2( s_assign, typeofexpr(y), y, e2)
				  );
			pp_expr(e,0);
			cg_exprvoid(e);
			if( valneeded ) cg_expr(x);
		}
		else /* Neither are local variables */
		{
			/* generate: (let t; t = y, x = (cast)t, y = t+1 [, t]) */
			TypeExpr *tmptype;
			Binder *tmp;
			Expr *s2 = s;
			Expr *e2 = s;
	
			dbtrace("no locals");
	
			if( h0_(e2) == s_cast )	s2 = e2 = arg1_(e2);
	
			tmptype = typeofexpr(y);
			tmp = gentempbinder(tmptype);
	
			/* include cast in assignment if needed	*/
			if( c != NULL )
			{
				e1 = mk_expr2( s_assign, t, x,
					mk_expr1( s_cast, typeofexpr(c), (Expr *)tmp ) );
			}
			else e1 = mk_expr2( s_assign, tmptype, x, (Expr *)tmp );
	
			e2 = substvar(e2, tmp, y);
	#if 0
			/* replace y with tmp in source expression	*/
		    	if( (h0_(e2) == s_plus || h0_(e2) == s_minus) &&
		    		is_same(arg1_(e2),y)
			  )
		    	{
		    		e2 = mk_expr2(h0_(e2), tmptype,
		    			(Expr *)tmp, arg2_(e2) );
		    	}
	#endif
			if( s2 != s ) e2 = mk_expr1(s_cast, typeofexpr(s), e2);
	
			e2 = mk_expr2( s_assign, tmptype, y, e2);
	
			if( valneeded ) e2 = mk_expr2(s_comma, tmptype, e2, (Expr *)tmp );
			e = mk_expr2( s_let,
					t,
					(Expr *)mkBindList( 0, tmp),
					mk_expr2(s_comma, tmptype,
						mk_expr2(s_assign, tmptype, (Expr *)tmp, y),
					mk_expr2(s_comma, tmptype,
						e1,
						e2
						))
				    );
			pp_expr(e, 0);
			cg_expr1(e, valneeded);
		}
	    	pop_trace(); 
		return;	
	    }
	
#if 0
   /*
    * This optimisation causes an internal error in cg_cast () [int32 -> double
    * with no destination] when compiling source code ...
    *
    *		longvar = (longvar > (longvar * doubleconstant)) ? 
    *				(longvar * doubleconstant) :
    *				longvar;
    *
    * It may be possible to fix it, but for now I'm simply ignoring it. 
    * Tony 10/3/95
    */
	    /* Try to optimise an assignment of the form a = (cast)b	*/
	    /* where both are variables and a is a narrower (integer)	*/
	    /* type than b. In this case we can simply truncate b.	*/
	    /* This also handles assignment of same size signed and	*/
	    /* unsigned variables.					*/
	
	    if
	    (
		(h0_(x) == s_assign)					&&
		(mcmode < 2)			/*			&&
		(h0_(target) == s_binder)	*/			
	    )
	    {
	    	Expr *s = source;
		int sm;
	    	if( h0_(source) == s_cast ) s = arg1_(s);
	
		sm = mcrepofexpr(s);
	
	    	if( (sm&7) >= (targetmode&7) )
	    	{
			dbtrace("(a = (cast)b) lxx %d mcl %d sm %x tm %x h0s %d h0t %d",lxxAllowed,mclength,
							sm,targetmode,h0_(s),h0_(target));
	
	    		if( h0_(s) == s_binder )
	    			cg_var( (Binder *)s, s_content, 0, 4, TRUE );
			else if( (sm&7)==2 && aligned_short_struct_access(s) )
			{
				s = cg_content_for_dot(s);
				if( h0_(s) == s_content ) s = arg1_(s);
				cg_indirect( s, s_content, 1, 2, TRUE );
			}
	#if 0
	 		else if( isindirection_(s) )
			{
				s = cg_content_for_dot(s);
				if( h0_(s) == s_content ) s = arg1_(s);
				cg_indirect( s, s_content, 1, sm&7, TRUE );
			}
	#endif
			else cg_expr(s);
	
			/* truncate if this is a narrowing cast, or casting	*/
			/* short/char signed to unsigned.			*/
	    		if
			(
				(sm&7) != (targetmode&7)		||
				((sm>>24) == 0 && (targetmode>>24) == 1 && (targetmode&7) != 4)
			) emitmask( targetmode&7 );
	
			
	
	    		if( h0_(target) == s_binder )
			{
				cg_var( (Binder *)target, s_assign, 0, 4, FALSE );
		    		if( valneeded ) cg_var( (Binder *)target, s_content, mcmode, mclength, valneeded );
		    	}
		    	else
		    	{
		    		cg_storein(target, s_assign, valneeded);
		    	}
	    		pop_trace(); 
			return;
	    	}
	    	else
	    	{
	    		/* Widening assignment knock out the cast since the load */
	    		/* will sign extend if necessary and the store will mask */
	    		/* down if necessary.					 */
			dbtrace("(a = (cast)b) eliminate widening cast");
	    		source = s;
	    	}
	    	
	    }
#endif /* 0 */
	/*{{{  optimise *a = b for bytes */
	    /* Try to optimise byte assignment to an				*/
	    /* indirection. So long as the source is >= 1 byte wide		*/
	    /* we can just store the bottom byte into the dest.			*/
	    if
	    (
		(!lxxAllowed) 						&& 
		(mcmode < 2)						&&
		(mclength == 1) 					&& 
		(h0_(x) == s_assign)					&&
		isindirection_(target)
	    )
	    {
	    	Expr *s = source;
		int sm;
	    	if( h0_(source) == s_cast ) s = arg1_(s);
	
		sm = mcrepofexpr(s);
		dbtrace("byte(*a = b) lxx %d mcl %d sm %x tm %x h0s %d h0t %d",lxxAllowed,mclength,
						sm,targetmode,h0_(s),h0_(target));
	
		if( h0_(s) == s_binder ) cg_var( (Binder *)s, s_content, 0, 4, TRUE );
		else cg_expr( s );
	
		cg_storein( target, h0_(x), valneeded );
		
		pop_trace(); return;
	
	    }
	
	/*}}}*/
	
    aisave		= Assign;
    Assign.Target 	= target;
    Assign.TargetUsed	= FALSE;
    Assign.Source	= source;
    Assign.SourceEval	= evalsource;
    
    if( evalsource ) cg_expr(source);

    atu			= Assign.TargetUsed;

    if( !atu )
    {
	if (targetOp != s_binder)
		cg_storein( target, h0_(x), valneeded);
	else 
		cg_var( (Binder *)target, h0_(x), mcmode, mclength, valneeded);
    }

    Assign		= aisave;
       
    pop_trace();
}
/*}}}*/
/*{{{  cg_indirect */

/* The code generated here is not really very clever. It should be
 * improved later, so that it extracts constants from the index expression
 * into the base offset. It should also make use of wsubdb. These improvements
 * are probably better done in the pre parser (or at least supported there).
 * More attention should be paid to the context in which this procedure is
 * called.
 * At present something like short a[]; a[i] = a[j]; generates a move from a[j]
 * to a temp and then a move from this to a[i] where a direct move a[j] to a[i]
 * is obviously what is needed.
 * In general this, cg_scalarAssign, cg_var, cg_storein and cg_addr all 
 * need a total re-think and re-write.
 */
void cg_indirect(x,flag,mcmode,mclength,valneeded)
Expr *x;
int flag, mcmode, mclength,valneeded;
{
	VLocal *v1 = NULL;
	VLocal *v2 = NULL;
	Expr *x1, *x2 = NULL;
	int offset = 0, postinc = 0;
	/*const*/ RegSort rsort = mcmode!=2 ? INTREG : 
				(mclength==4 ? FLTREG : DBLREG);
	int d = depth(x);
	int scaled = FALSE;

	struct AssignInfo aisave;
	
	/* if this is a T4 and we are trying to get a single precision	*/
	/* float, lie about it and load an integer instead.		*/
	if( !floatingHardware && rsort==FLTREG )
	{
		mcmode = 0;
		mclength = 4;
		rsort=INTREG;
	}

	push_trace("cg_indirect");
	
	if(debugging(DEBUG_CG))
	{ 
	    trace("CG_indirect: %x %s flag %d mcmode %d mclength %d",
		x,symbol_name_(h0_(x)),flag,mcmode,mclength);
	    trace("valneeded %d depth %d op: %x",valneeded,d, h0_(x));
	}
	
		switch( h0_(x))
		{
		case s_plus:
		/* test for (x + n), common case for structure and array access */
			x1 = arg1_(x); x2 = arg2_(x);
			if(integer_constant(x1) ) offset=result2, x1=x2, x2=NULL, d=depth(x1);
			else if(integer_constant(x2) ) offset=result2, x2 = NULL, d=depth(x1);
			else if( h0_(x2) == s_times && mclength == 4 )
			{/* check for (x + (y * 4)), common case for array access */
				if( integer_constant(arg1_(x2)) && result2 == 4 )
					scaled = TRUE, x2 = arg2_(x2);
				else if( integer_constant(arg2_(x2)) && result2 == 4 )
					scaled = TRUE, x2 = arg1_(x2);
			}
			break;
	
		case s_minus:
			x1 = arg1_(x); x2 = arg2_(x);
			if( integer_constant(x2) ) offset= -result2, x2 = NULL,d=depth(x1);
			else x1 = x, x2 = NULL;
			break;
	
		case s_displace:
		{
			Expr *v = arg1_(x), *x3 = arg2_(x);
			if( h0_(x3)==s_plus &&
				is_same(arg1_(x3),v) && integer_constant(arg2_(x3)))
			{ /* i.e. something like *p++ */
				postinc = result2;
				x = v;
				d = depth(x);
			}
		}
			/* drop through */
		
		default:
			x1 = x; x2 = NULL;
			break;
	
		} /* end of switch */

	/* Here x1 is the expression to generate the address,	*/
	/* x2 is the offset, unless it is NULL in which case	*/
	/* there is a constant integer offset in offset. If the */
	/* expression contains a displace, there will be a post */
	/* increment in postinc.				*/

/*	if(debugging(DEBUG_CG)) */
		trace("x1 %s x2 %s offset %d postinc %d",
			x1?symbol_name_(h0_(x1)):"<NULL>",
			x2?symbol_name_(h0_(x2)):"<NULL>",
			offset,postinc);

		if( !lxxAllowed && mcmode != 2 && mclength == 2 )
		{
			/* access to shorts on T4/8 always uses all registers */
			switch( flag )
			{
			case s_content:
				break;
			case s_assign:
				/* if( Assign.SourceEval ) */
					v2 = pushtemp( INTREG );
				break;
			case s_displace:
				v2 = pushtemp( INTREG );
				break;		
			}
			if( ida < 3 ) v1 = pushtemp( INTREG );
		}
		else if( mcmode != 2 || !floatingHardware )
		{
			/* integers and others */
			switch( flag )
			{
			case s_content:
				break;
			case s_assign:
				if( valneeded || idepth(d) > 2 ) v2 = pushtemp(INTREG);
				if( v2 && idepth(d) < 3 )
				{
					emit(p_ldvl, v2);
					pushInt();
				}
				break;
			case s_displace:
				if( idepth(d) > 2) v2 = pushtemp(INTREG);
				if( v2 && ida < 3 ) v1 = pushtemp(INTREG);
				break;
			}
		}
		else if ( mcmode == 2 )
		{
			/* floats and doubles, in T800 only */
			switch( flag )
			{
			case s_content:
				break;
			case s_assign:
				if( valneeded ) duplicate( FLTREG );
				break;
			case s_displace:
				break;
			}
		}

	/* Dispose of AssignInfo while evaluating address	*/
	/* index expressions, in case they contain assignements	*/
	/* of their own.					*/

	aisave			= Assign;	
	Assign.Target	 	= NULL;
	Assign.TargetUsed	= FALSE;
	Assign.Source		= NULL;
	Assign.SourceEval	= FALSE;

	if( x2 != NULL ) 
	{
		int oldae = addrexpr;
		addrexpr = TRUE;
		cg_binary(s_nothing,x2,x1,0,INTREG);
		addrexpr = oldae;
	}
	else cg_addrexpr(x1);

	Assign			= aisave;
	
	switch( mcmode )
	{
	case 0:
	case 1:
	case 4:
		/* integer-like things	*/
	    switch( mclength )
	    {
	    case 1:		/* byte sized pieces 		*/
			if( x2 ) emit(f_opr, op_bsub ), popInt(); 
			else if( offset ) emit( f_adc, offset ); 
			if( v2 != NULL && d > 2 )
			{
			   emit(p_ldvl, v2);
			   emit(f_opr, op_rev );
			   pushInt();
			}
			switch( flag )
			{
			case s_content:
				if( lxxAllowed )
				{
					if( mcmode == 0 ) emit(f_opr, op_lbx);
					else emit(f_opr, op_lb );
				}
				else
				{
					emit(f_opr, op_lb );
					if( mcmode == 0 ) emitxword( mclength );
				}
				setTOS(mcmode,mclength);
				break;
		
			case s_displace:
			{
			/* The following code is NASTY. We enter with A=address	*/
			/* and B = new value. We want to store the new value and*/
			/* return the original value. While there are enough 	*/
			/* registers for this, at some point we would want to   */
			/* swap B and C without touching A.			*/
				VLocal *v = pushtemp(INTREG);
  
				emit( p_ldvl, v ); pushInt();
				if( lxxAllowed ) emit(f_opr, mcmode==0?op_lbx:op_lb );
				else emit(f_opr, op_lb );
				emit(f_opr, op_rev );
				poptemp(v, INTREG);
				emit(f_opr, op_sb ); popInt(); popInt();
				if( !lxxAllowed && mcmode == 0 ) 
					emitxword( mclength );
				setTOS(mcmode,mclength);
				break;
			}
				
			case s_assign:
				emit(f_opr, op_sb );popInt(); popInt();
				break;
			}
		break;
	
	    case 2: 	/* short (16 bit) integers	*/
		
			if( x2 != NULL ){ emit(f_opr, op_bsub ); popInt(); }
		
			if( lxxAllowed )
			{
				{
					int opls = op_ls;
					if( mcmode == 0 ) opls = op_lsx;
				
					if( offset ) emit( f_adc, offset ); 
					if( v2 != NULL && d > 2 )
					{
					   emit(p_ldvl, v2);
					   emit(f_opr, op_rev );
					   pushInt();
					}
					switch( flag )
					{
					case s_content:
						emit(f_opr, opls );
						setTOS(mcmode,mclength);		
						break;
				
					case s_displace:
					{
					/* The following code is NASTY. We enter with A=address	*/
					/* and B = new value. We want to store the new value and*/
					/* return the original value. While there are enough 	*/
					/* registers for this, at some point we would want to   */
					/* swap B and C without touching A.			*/
						VLocal *v = pushtemp(INTREG);
				
						emit( p_ldvl, v ); pushInt();
						emit(f_opr, opls );
						emit(f_opr, op_rev );
						poptemp(v, INTREG);
						emit(f_opr, op_ss ); popInt(); popInt();
						setTOS(mcmode,mclength);
						break;
					}
						
					case s_assign:
						emit(f_opr, op_ss );popInt(); popInt();
						break;
					}
				}
			}
			else switch( flag ) 
			{
			/* Shorts are done using move. This is VERY expensive	*/		
			case s_content:
			    {
		#if 1
				if( isstructurepointer(x1) && (offset&3) == 0 )
				{
					/* Loading a short off a 4 byte offset in a	*/
					/* structure. Since structures are word aligned */
					/* we know that we can load a word, mask it and	*/
					/* then sign extend.				*/
		
					if( offset != 0 ) emit( f_ldnl, offset>>2 );
		
					if( mcmode == 0 ) emitmaskxword( 2 );
					else emitmask( 2 );
					
				}
				else
		#endif
		
				{
		#if 1
					Binder *b;
					int direct_assign;
		
					if( Assign.Target != NULL &&
					    h0_(Assign.Target) == s_binder &&
					    (mcrepofexpr(Assign.Target)&0x00FFFFFF) == mclength
					  )
					{
						direct_assign = 1;
						Assign.TargetUsed = 1;
						b = set_binder_ops((Binder *)Assign.Target);
					}
					else
					{
						direct_assign = 0;				
						loadOp = p_ldvl;
						storeOp = p_stvl;
						loadAddressOp = p_ldvlp;
						b = (Binder *)allocatetemp(INTREG);
					}
					if( offset ) emit( f_adc, offset );
					if( loadOp == p_ldvl )
					{ /* locals and temps need zeroing	*/
						emit( f_ldc, 0 );
						emit( storeOp, b );
					}
					emit( loadAddressOp, b);
					emit( f_ldc, 2 );
					emit( f_opr, op_move );
					if( !direct_assign )
					{
						emit( loadOp, b );
		
						if( mcmode == 0 ) emitxword( mclength );
						freetemp(b);
						setInt(FullDepth-1);
					}
					else setInt(FullDepth);
					
		#else
					VLocal *v = allocatetemp(INTREG);
					if( offset ) emit( f_adc, offset );
					emit( f_ldc, 0 );
					emit( p_stvl, v );
					emit( p_ldvlp, v);
					emit( f_ldc, 2 );
					emit( f_opr, op_move );
					emit( p_ldvl, v );
					setInt(FullDepth-1);
		
					if( mcmode == 0 ) emitxword( mclength );
		
					freetemp(v);
		#endif
				}
				setTOS(mcmode,mclength);		
				break;
			    }
		
			case s_displace:
			    {
				VLocal *p;
				VLocal *v1;
		
				if( offset ) emit( f_adc, offset );
				p = pushtemp(INTREG);
				v1 = allocatetemp(INTREG);
					
				emit( f_ldc, 0 );
				emit( p_stvl, v1 );
				emit( p_ldvl, p );
				emit( p_ldvlp, v1);
				emit( f_ldc, 2 );
				emit( f_opr, op_move );
				emit( p_ldvlp, v2);
				emit( p_ldvl, p );
				emit( f_ldc, 2 );
				emit( f_opr, op_move );
		
				emit( p_ldvl, v1 );
				setInt(FullDepth-1);
		
				if( mcmode == 0 ) emitxword( mclength );
		
				freetemp(v1);
				freetemp(p);
				setTOS(mcmode,mclength);
				break;	
			    }
				
			case s_assign:
			    {
		#if 0
				Binder *b;
				int direct_assign;
				if( !Assign.SourceEval )
				{
					if
					(   Assign.Source != NULL &&
					    h0_(Assign.Source) == s_binder &&
					    (mcrepofexpr(Assign.Source)&0x00FFFFFF) >= mclength
					)
					{
						direct_assign = 1;
						b = set_binder_ops((Binder *)Assign.Source);
					}
					else
					{	/* We should never actually get here... */
						
						syserr("Short assigment optimisation error");
					}
				}
				else
				{
					direct_assign = 0;				
					loadOp = p_ldvl;
					storeOp = p_stvl;
					loadAddressOp = p_ldvlp;
					b = (Binder *)v2;
				}
		
				if( offset ) emit( f_adc, offset );
				emit( loadAddressOp, b );
				emit( f_opr, op_rev );
				emit( f_ldc, 2 );
				emit( f_opr, op_move );
				setInt(FullDepth);
		#else
				if( offset ) emit( f_adc, offset );
				emit( p_ldvlp, v2 );
				emit( f_opr, op_rev );
				emit( f_ldc, 2 );
				emit( f_opr, op_move );
				setInt(FullDepth);
		#endif
				break;
			    }
			}
		break;
	

	    case 4:		/* normal ints			*/
		
			if( x2 ) 
			{
				if( scaled ) emit(f_opr, op_wsub );
				else emit(f_opr, op_sum );
				popInt();
			}
			if( v2 != NULL && d > 2 )
			{
			   emit(p_ldvl, v2);
			   emit(f_opr, op_rev );
			   pushInt();
			}
			switch( flag )
			{
			case s_content:
				emit( f_ldnl, offset / 4 );
				setTOS(mcmode,mclength);
				break;
				
			case s_displace:
			    {
			/* More yukky code, see comment above.			*/
				VLocal *v = pushtemp(INTREG);
					
				emit( p_ldvl, v ); pushInt();
				emit( f_ldnl, offset / 4 );
				emit(f_opr, op_rev );
				poptemp(v, INTREG);
				emit( f_stnl, offset / 4 ); popInt();popInt();
				setTOS(mcmode,mclength);
				break;
			    }
		
			case s_assign:
				emit( f_stnl, offset / 4 ); popInt();popInt();
				break;
			}
	    } /* mclength */
	    break;


	case 2:
		/* real numbers		*/
			if( x2 ) 
			{
				if( scaled ) emit(f_opr, op_wsub );
				else emit(f_opr, op_sum );
				popInt();
			}
			if( v2 != NULL && d > 2 )
			{
			   emit(p_ldvl, v2);
			   emit(f_opr, op_rev );
			   pushInt();
			}
		
			switch( flag )
			{
			case s_content:
			    if( floatingHardware )
			    {
				emit( f_ldnlp, offset / 4 );
				emit(f_opr, rsort == FLTREG ? 
						op_fpldnlsn: op_fpldnldb);
				popInt(); pushFloat();
			    }
			    else emit( rsort==FLTREG?f_ldnl:f_ldnlp, offset / 4 );
			    break;
				
			case s_displace:
			/* More yukky code, see comment above.			*/
			   if( floatingHardware )
			   {
				emit( f_ldnlp, offset / 4 );
				emit(f_opr, op_dup ); pushInt();
		           	emit(f_opr, rsort == FLTREG ? op_fpldnlsn: op_fpldnldb);
		           	popInt(); pushFloat();
		           	emit(f_opr, op_fprev );
		           	emit(f_opr, rsort == FLTREG ? op_fpstnlsn: op_fpstnldb);
		           	popInt(); popFloat();
			   }
			   else {
				syserr("cg_indirect: attempt to use double in T4");
			   }
			   break;
				
			case s_assign:
			    if( floatingHardware )
			    {
				emit( f_ldnlp, offset / 4 );
				emit(f_opr, rsort == FLTREG ? op_fpstnlsn:op_fpstnldb);
				popInt(); popFloat();
			    }
			    else {
				syserr("cg_indirect: attempt to use double in T4");
			    }
			    break;
			}
		
		break;
	} /* mcmode */

	/* if there was a displace within the indirection, do the	*/
	/* update part here. 						*/

	if( postinc ) 
	{
		cg_addrexpr( x1 );		/* @@@ side effects ??? */
		emit( f_adc, postinc );
		cg_storein( x1 , s_assign, 0 );
	}

dbtrace("v1 %x v2 %x flag %d valneeded %d",v1,v2,flag,valneeded);
	if( v1 != NULL ) poptemp( v1, INTREG );
	if( v2 != NULL && flag==s_assign && valneeded ) poptemp( v2, INTREG );
	
	pop_trace();
}
/*}}}*/
/*{{{  cg_var */

/* I don't like the way this generates things if the value is required,
 * I would prefer to do a dup on the value before saving it away.
 * This saves a store cycle !!! FIX THIS LATER ! Jim.
 */
void cg_var(b,flag,mcmode,mclength,valneeded)
Binder *b;
AEop flag;
int mcmode,mclength,valneeded;
{

	push_trace("cg_var");
	         
	if(debugging(DEBUG_CG))
	{
	    trace("Cg_var: %s flag %d mcmode %d mclength %d valneeded %d ida %d",
			_symname(bindsym_(b)),flag,
			mcmode,mclength,valneeded,ida);
	    trace("binder mode %x",mcrepofexpr((Expr *)b));
	}

	b = set_binder_ops(b);				
	
	if( mcmode < 2 || mcmode == 4 || (!floatingHardware && mcmode==2 && mclength==4) )
	{ /* all integer-like objects are stored in 1 word */
	  /* also all 4 byte floats in T414 */

		switch(flag)
		{
		case s_displace: 
			emit( loadOp,  b);pushInt();
			if( mcmode == 0 && mclength < 4) emitxword( mclength );
		        emit(f_opr, op_rev );
			emit(  storeOp, b);popInt();
			setTOS(mcmode,mclength);
			break;

		case s_assign:
		{
			int dupped = 0;
			if( valneeded && dupAllowed && ida > vardepthneeded )
			{
				emit( f_opr, op_dup ); pushInt();
				dupped = 1;
			}
			if( mclength < 4 ) emitmask( mclength );
			emit( storeOp, b );popInt();
		        if (!valneeded || dupped ) break;
			
			/* drop through to re-load */
		}
				
		case s_content:
		        if (valneeded)
		        {  
		            /* Check that higher up sets valneeded if volatile */
	        	    emit( loadOp, b); pushInt();
			    if( mcmode == 0 && mclength < 4) emitxword( mclength );
			    setTOS(mcmode,mclength);
		        }
		        break;
		    
		default:
			syserr("Unknown flag %x8 in cg_var",flag);
		}
	}
	else 
	{ 
	     if( !floatingHardware )
	     {
		/* Only doubles get through to here			*/
		/* doubles are always represented by a pointer to the	*/
		/* actual value in store.				*/
		switch( flag )
		{
		case s_assign:
		{
			/* copy both words 1 by 1 */
			VLocal *v = pushtemp(INTREG);
			emit( p_ldvl, v ); pushInt();
			emit( f_ldnl, 0 );
			emit( loadAddressOp, b ); pushInt();
			emit( f_stnl, 0 ); popInt(); popInt();

			emit( p_ldvl, v ); pushInt();
			emit( f_ldnl, 1 ); 
			emit( loadAddressOp, b ); pushInt();
			emit( f_stnl, 1 ); popInt(); popInt();
			if( valneeded )
			{
				emit( loadAddressOp, b );
				pushInt();
			}
			freetemp( v );
			break;
		}
		case s_displace:
		{
			/* copy both words 1 by 1 */
			VLocal *v = pushtemp(INTREG);
			emit( p_ldvl, v ); pushInt();
			emit( f_ldnl, 0 );
			emit( loadAddressOp, b ); pushInt();
			emit( f_stnl, 0 ); popInt(); popInt();

			emit( p_ldvl, v ); pushInt();
			emit( f_ldnl, 1 ); 
			emit( loadAddressOp, b ); pushInt();
			emit( f_stnl, 1 ); popInt(); popInt();

			/* Return original pointer */
			emit( p_ldvl, v ); pushInt();

			freetemp( v );
			break;
		}
		case s_content:
			/* simply return address of variable */
			emit( loadAddressOp, b); pushInt();
		 	break;
		} /* end of switch */
		{ pop_trace(); return; }
	     }
	     /* floating objects, T8 only at present */
	     emit( loadAddressOp, b); pushInt();
	     switch (flag)
             {
	 	  case s_displace:
	  	       emit(f_opr,op_dup); pushInt();
		       emit(f_opr,mclength == 4 ? op_fpldnlsn:op_fpldnldb);
		       popInt(); pushFloat();
		       emit(f_opr,op_fprev);
		       emit(f_opr,mclength == 4 ? op_fpstnlsn:op_fpstnldb);
		       popInt(); popFloat();
		       break;
		  case s_assign:
		       if (valneeded)
	  	       {
		          emit(f_opr, op_fpdup );
	   	          pushFloat();
		       }
		       emit(f_opr,mclength == 4 ? op_fpstnlsn:op_fpstnldb);
		       popInt(); popFloat();
	 	       break;
		  case s_content:
		      emit(f_opr,mclength == 4 ? op_fpldnlsn:op_fpldnldb);
		      popInt(); pushFloat();
		      break;
		  default:
		      syserr("Unexpected flag in cg_var #%x8",flag);
		}
	}
	pop_trace();
}
/*}}}*/
/*{{{  cg_cast1 */

#define cg_expr4(x,mode) cg_cast1(x,4,mode)

void cg_cast1(x1,mclength,mcmode)
Expr *x1; const int mclength; const int mcmode;
{
    int arglength = mcrepofexpr(x1);
    int argmode = (arglength>>24) & 0xff;
    RegSort rsort = (mcmode!=2) ? INTREG : mclength==4 ? FLTREG : DBLREG;
    arglength &= 0x00ffffff;

    push_trace("cg_cast");

    if(debugging(DEBUG_CG)) 
	trace("Cg_cast: %x op %s mc mode %d l %d  arg mode %d l %d",
		x1,symbol_name_(h0_(x1)),mcmode,mclength,argmode,arglength);

    if (mclength==0) 
    {
       cg_exprvoid(x1);  /* cast to void */
       pop_trace(); return;
    }

#if 0
    if (mcmode == 4)
    {   /* @@@ LDS 13Aug89 (non-cast) to 'plain' type - i.e. load narrow */
        /* integer in most efficient manner, irrespective of real type.  */
        /* Used to suppress s/u bits on J_LDR[B|W]Xx jopcodes.           */
        /* NOTE: guaranteed that *x1 IS a Binder (by simplify.optimise0) */
        /* so the call to cg_var IS appropriate.                         */

	cg_var((Binder *)x1, s_content, mcmode, arglength, TRUE);
	/* cg_var((Binder *)x1, s_content, argmode, arglength, TRUE); */
	pop_trace(); return;
    }
#endif

    if (mcmode==3 || argmode==3)
    {   if (mcmode==argmode && mclength==arglength) { pop_trace(); return; }
        else syserr("Illegal cast involving a structure or union");
    }

    if (mcmode==argmode) switch(mcmode)
    {
case 4:
case 0:
	        cg_expr(x1);
	/*trace("mclen %d arglen %d ida %d",mclength,arglength,ida);*/
	
		argmode = TOSmode();
		arglength = TOSlength();
		
		/* values are kept in the evaluation stack as full-width values */
		/* hence widening casts can be ignored.				*/
		if( mclength >= arglength ) { pop_trace(); return; }
		
		/* a narrowing cast requires us to mask and sign extend from	*/
		/* the new width.						*/
	
		if( !lxxAllowed && ida < 2 )
		{
			/* to sign extend we need 2 stack regs, make space and do it */
			VLocal *v;
			
			emit(f_opr, op_rev );
			v = pushtemp( INTREG );
			emitmaskxword( mclength );
			poptemp( v, INTREG );
			emit(f_opr, op_rev );
		}
		else emitmaskxword( mclength );
	
	        pop_trace(); return;
case 1:
	        cg_expr(x1);
	
		argmode = TOSmode();
		arglength = TOSlength();
	
		/* values are kept in the evaluation stack as full-width values */
		/* hence widening casts can be ignored.				*/
		if( mclength >= arglength ) { pop_trace(); return; }
		
		/* a narrowing cast requires us to mask down to the new width.	*/
	
		if( ida < 2 )		/* narrow to mclength by masking	*/
		{
			/* to mask we need 2 stack regs, make space and do it	*/
			VLocal *v;
	
			emit(f_opr, op_rev );
			v = pushtemp( INTREG );
			emitmask( mclength );
			poptemp( v, INTREG );
			emit(f_opr, op_rev );
		}
		else emitmask( mclength );
		
	        pop_trace(); return;
case 2:
		{
			if( mclength==arglength )
			{
			        cg_expr(x1);
				{ pop_trace(); return; }
			}
			if( floatingHardware )
			{
			        cg_expr(x1);
				if( mclength < arglength ) emit(f_opr, op_fpur64tor32 );
				else emit(f_opr, op_fpur32tor64 );
				pop_trace(); return;
			}
			else
			{
				if( mclength < arglength )
				{
					VLocal *proc = allocatetemp(INTREG);
					VLocal *olddd = doubledest;
					
					doubledest = allocatetemp(DBLREG);
					
					emit( p_ldx, fplib.real64toreal32 );
					emit( p_stvl, proc );
						
				        cg_expr(x1);
					emit( f_ldc, ROUND_NEAREST );
					emit( p_fpcall, proc );
		
					freetemp( doubledest );
					freetemp( proc );
					doubledest = olddd;
				}
				else {
					VLocal *proc = allocatetemp(INTREG);
					
					if( doubledest == NULL )
						syserr("real32 -> real64 with no dest");
	
					emit( p_ldx, fplib.real32toreal64 );
					emit( p_stvl, proc );
				
				        cg_expr(x1);
					doubleaddr(doubledest);
					emit( p_fpcall, proc );
	
					freetemp( proc );
					doubleaddr(doubledest);
	
				}
				setInt(FullDepth-1);
				{ pop_trace(); return; }
			}
		}
default:
        if (mclength!=arglength) 
            syserr("bad mode %d in cast expression", mcmode);
        cg_expr(x1); 
        { pop_trace(); return; }
    }
    else if (mcmode==2)
	/*{{{  floating something */
	    {   /* floating something */
	
	/* Earlier parts of the compiler ensure that it is only necessary to     */
	/* cope with full 32-bit integral types here. Such things as (float) on  */
	/* a character are dealt with as (float)(int)<char> with the inner cast  */
	/* explicit in the parse tree.                                           */
	/****** FLOATING HARDWARE CHECK ******/
	#if 0
	        if (arglength!=4)
	            syserr("cg_cast(float %d)", arglength);
	#endif
	        if (argmode == 1)    /* unsigned -> float - simulate with signed */
	        {
	            /* This is done by an ACN trick ...                          */
	            /* xor source, minint                                        */
	            /* -> double                                                 */
	            /* add in the 2**31 again                                    */
	            /* and if necessary round to single                          */
	            /* How unpleasant !                                          */
		    if( floatingHardware )
		    {
			VLocal *v;
	            	cg_expr4(x1,argmode);             /* Load the argument */
	#if 0
	            	emit(f_opr, op_mint );pushInt();
	            	emit(f_opr, op_xor  );popInt();
	            	v = pushtemp( INTREG ); /* Ready to load and float */
	            	emit( p_ldvlp, v); pushInt();
	            	emit(f_opr, op_fpi32tor64 ); popInt(); pushFloat(); /* Loaded as a double */
	            	freetemp(v);
	            	emitFloatCon( fc_two_31, DBLREG);
	            	emit(f_opr, op_fpadd ); popFloat();
	#else
	            	v = pushtemp( INTREG );
	            	emit( p_ldvlp, v); pushInt();
			emit( f_opr, op_fpb32tor64 ); popInt(); pushFloat();
	            	freetemp(v);
	#endif
	            	if (mclength == 4) 
	               		emit(f_opr, op_fpur64tor32 );
		    }
		    else {
			VLocal *v = NULL;
			VLocal *proc = allocatetemp(INTREG);
	
			if( doubledest == NULL )
			{
				if( mclength == 8 ) syserr("unsigned -> double without dest");
				v = doubledest = allocatetemp(DBLREG);	
			}
		
			emit( p_ldx, fplib.int32toreal64 );
			emit( p_stvl, proc );
	
	            	cg_expr4(x1,argmode);		/* Load the argument	*/
	            	emit(f_opr, op_mint );		/* subtract 2**31	*/
	            	emit(f_opr, op_xor  );
			doubleaddr(doubledest);
			emit( p_fpcall, proc );		/* convert to double	*/
	
	            	emitFloatCon( fc_two_31, DBLREG);
			doubleaddr(doubledest);
			codeFpCall( s_plus, 0x02000008 ); /* add in 2**31	*/
	
	            	if (mclength == 4) 		/* convert to single if necc */
			{
				emit( p_ldx, fplib.real64toreal32 );
				emit( p_stvl, proc );
					
				emit( f_ldc, ROUND_NEAREST );
				emit( p_fpcall, proc );
			}
			/* in double case result is in doubledest */
			
			if( v ) freetemp(v), doubledest = NULL;
	
			freetemp(proc);
	
			setInt(FullDepth-1);
		    }
	        }
		else
	        {   /* Check to see whether the value we're converting */
	            /* is already in store, in which case load its address */
	            /* Else store it to a temp first */
		    if( floatingHardware )
		    {
	            	if (instore( x1 )) 
	               		cg_addr(x1, TRUE);
	            	else
	            	{
	               		VLocal *temp;
	               		cg_expr4(x1,argmode);
	               		temp = pushtemp(INTREG);
	               		emit( p_ldvlp, temp); pushInt();
	               		freetemp(temp); /* Actually after the next instruction ...*/
	            	}
	            	emit(f_opr,rsort==FLTREG ? op_fpi32tor32:op_fpi32tor64);pushFloat(); popInt();
		    }
		    else
		    {
			if( rsort == FLTREG )
			{	/* signed int to single */
				VLocal *proc = allocatetemp(INTREG);
				emit( p_ldx, fplib.int32toreal32 );
				emit( p_stvl, proc );
				cg_expr4(x1,argmode);
				emit( f_ldc, ROUND_NEAREST );
				emit( p_fpcall, proc );
				freetemp(proc);
			}
			else {	/* signed int to double */
				VLocal *dest = doubledest;
				VLocal *proc = allocatetemp(INTREG);
	
				if( dest == NULL )
					syserr("int32 -> double with no dest");
	
				emit( p_ldx, fplib.int32toreal64 );
				emit( p_stvl, proc );
				cg_expr4(x1,argmode);
				doubleaddr(dest);
				emit( p_fpcall, proc );
				freetemp(proc);
				doubleaddr(dest);
			}
			setInt(FullDepth-1);
		    }
		}
	        { pop_trace(); return; }
	    }
	/*}}}*/

    else if (argmode==2)
	/*{{{  fixing something */
	    {   /* fixing something THIS CODE IS NOT VERY CLEVER at generating
	           int i = fix( xxx ) ; because it doesn't know that the target
	           is really a store location, hence it will introduce a temp,
	           and then load and store it... FIX LATER ...
	        */
	        VLocal *v = allocatetemp(INTREG);
	        
	/* N.B. the mclength==4 test in the next line is to produce shorter code */
	/* for (unsigned short)(double)x.  It implies that this is calculated as */
	/* (unsigned short)(int)(double)x.                                       */
	        if (mcmode != 0 && mclength == 4)
	        {
	/* Fixing to an unsigned result is HORRIBLE, and is done using lots of   */
	/* instructions here. The idea is to subtract 2**31 and fix, and then    */
	/* to add back 2**31 to the resulting integer. We can cheat slightly here*/
	/* by using round negative so that what we generate is                   */
	/*     fixu(x) rounded to zero = fixs(x-2^31) round neg + 2^31           */     
	/* The trick only works if I use double precision, (cos 2^31 is not      */
	/* precisely representable in SP) so for float I start off by widening the*/
	/* input.                                                                */
		    if( floatingHardware )
	            {   
	                cg_expr(x1);
	                if (arglength==4)
	                   emit(f_opr,op_fpur32tor64);
	                emitFloatCon(fc_two_31, DBLREG);
	                emit(f_opr,op_fpadd); popFloat();
	                emit( p_ldvlp, v); pushInt();
	                emit(f_opr,op_fpurm);
	                emit(f_opr,op_fprtoi32); 
			emit(f_opr, op_fpstnli32);
			popInt(); popFloat();
	                emit(p_ldvl, v); pushInt();
	                emit(f_opr, op_mint ); pushInt();
	                emit(f_opr, op_xor ) ; popInt();
	             }
		     else {
		     	VLocal *dbl = NULL;
		     	extern FloatCon *fc_unsfix;
		     	
		     	if( doubledest == NULL ) 
		     		dbl = doubledest = allocatetemp(DBLREG);
		     	
			if( arglength == 4 )
			{
				emit( p_ldx, fplib.real32toreal64 );
				emit( p_stvl, v );
				
			        cg_expr(x1);
				doubleaddr(doubledest);
				emit( p_fpcall, v );
				doubleaddr(doubledest);
			}
			else cg_expr(x1);
			/* doubedest now contains the value to convert */
	
	                emitFloatCon(fc_unsfix, DBLREG);
	                codeFpCall(s_minus,0x02000008);
	                /* doubledest = doubledest - (2**31+0.5)	*/
	                
	                emit( p_ldx, fplib.real64toint32 );
	                emit( p_stvl, v );
	                doubleaddr(doubledest);
	                emit(f_ldc,ROUND_NEAREST);
	                emit(p_fpcall, v);
			/* we now have (int)doubledest on the stack add back 2**31 */
	
			emit(f_opr, op_mint);
			emit(f_opr, op_xor );
			
			if( dbl != NULL ) freetemp(dbl), doubledest = NULL;
			
			setInt(FullDepth-1);             
		     }
	        }
	        else 
	        { /* Fix to a signed result is rather easier. */
	          /* This is the case where we would like to know the target */
		     if( floatingHardware )
		     {
		             cg_expr(x1);
			     emit(p_ldvlp, v);pushInt();
			     emit( f_opr, op_fpurz );	/* round to zero */
		             emit(f_opr, op_fprtoi32 ); 
			     emit(f_opr, op_fpstnli32);
			     popFloat(); popInt();
	        	     emit( p_ldvl, v); pushInt();
		     }
		     else {
			if( arglength == 4 )
			{
				emit( p_ldx, fplib.real32toint32 );
				emit( p_stvl, v );
				cg_expr(x1);
				emit( f_ldc, ROUND_ZERO );
				emit( p_fpcall, v );
			}
			else {
				VLocal *olddd = doubledest;
				doubledest = allocatetemp(DBLREG);
				emit( p_ldx, fplib.real64toint32 );
				emit( p_stvl, v );
				cg_expr(x1);
				emit( f_ldc, ROUND_ZERO );
				emit( p_fpcall, v );
				freetemp( doubledest );
				doubledest = olddd;
			}
			setInt(FullDepth-1);
		     }
	        }
	        
	        freetemp(v);
	        
	/* If I do something like (short)<some floating expression> I need to    */
	/* squash the result down to 16 bits.                                    */
	        if (mclength < 4)
	        {
	            if (mcmode == 0 || mcmode == 4) emitmaskxword( mclength );
	            else emitmask( mclength );
	        }
	    } /* End of the FIX cases */
	/*}}}*/

    else if( mclength == arglength && argmode == 4 )
	    {
	       cg_expr(x1);
	       { pop_trace(); return; }
	    }

    else if (arglength==4 && mclength==4)
	    {   
	       cg_expr(x1);
	       { pop_trace(); return; }
	    }

    else if (mcmode==1)
	    {   
	    	int masklength = mclength;
		void (*maskfn)() = emitmask;
	        cg_expr(x1);
	
		/* For widening casts we mask the sign bits out down to the */
		/* arglength. When narrowing we mask down to the mclength.  */
		if( mclength >= arglength )
		{
			if( arglength == 4 ) { pop_trace(); return; }
			masklength = arglength;
			maskfn = emitmaskxword;
		}
		
		if( ida < 2 )
		{
			/* to mask we need 2 stack regs, make space and do it */
			VLocal *v;
	
			emit(f_opr, op_rev );
			v = pushtemp( INTREG );
			maskfn( masklength );
			poptemp( v, INTREG );
			emit(f_opr, op_rev );
		}
		else maskfn( masklength );
	
	        { pop_trace(); return; }
	    }

    else if (mcmode==0 || mcmode == 4)
	    {
	        cg_expr(x1);
	
		if( mclength >= arglength ) { pop_trace(); return; }
	
		if( !lxxAllowed && ida < 2 )
		{
			/* to sign extend we need 2 stack regs, make space and do it */
			VLocal *v;
			int savessp = ssp;
			emit(f_opr, op_rev );
			v = pushtemp( INTREG );
			emitmaskxword( mclength );
			poptemp( v, INTREG );
			emit(f_opr, op_rev );
		}
		else emitmaskxword( mclength );
	
	        { pop_trace(); return; }
	    }

    else
    {   syserr("cast %d %d %d %d", mcmode, mclength, argmode, arglength);
        { pop_trace(); return; }
    }
}
/*}}}*/
/*{{{  cg_storein */
void cg_storein(e,flag,valneeded)
Expr *e;
AEop flag; 
int valneeded;
{
	int mclength = mcrepofexpr(e);
	int mcmode = (mclength>>24) & 0xff ;
	mclength &= 0x00ffffff;

	push_trace("cg_storein");
	
	if(debugging(DEBUG_CG))	trace("cg_storein: %s %s mode %d len %d valneeded %d",symbol_name_(h0_(e)),symbol_name_(flag),mcmode,mclength,valneeded);
		
	switch( h0_(e) )
	{
	case s_binder:
		cg_var( (Binder *)e, flag, mcmode, mclength, valneeded );
		break;
	
	case s_dot:
		e = cg_content_for_dot(e);
		pp_expr(e,valneeded);
		cg_storein( e, flag, valneeded );
                break;
                
	case s_content:
		e = arg1_(e);
		cg_indirect(e , flag, mcmode, mclength, valneeded );
                break;

	case s_cast:
		cg_storein(arg1_(e), flag, valneeded);		
		break;

	default:
		syserr("cg_storein(%d)",h0_(e));
	}
	pop_trace();
}
/*}}}*/
/*{{{  codeOperation */
void codeOperation( op, floating, mode )
AEop op;
bool floating;
int mode;
{
    switch (op)
    {
       case  s_times: 
       		    if( addrexpr && !floating ) emit( f_opr, op_prod );
/*       		    else emit(f_opr, floating ? op_fpmul: op_mul); */
       		    else emit(f_opr, floating ? op_fpmul: op_prod);
                    break;
       case  s_plus:
		    emit(f_opr, floating ? op_fpadd: op_add);
                    break;
#if 0
       case s_sum:  /*
		     * Symbol defined only in the back end/middle for unsigned
		     * additions. Tony 19/1/95
		     * Actually, added into aeops.h.  Tony 23/1/95
		     */
		    emit (f_opr, op_sum);
		    break;
#endif
       case  s_minus:
		    emit(f_opr, floating ? op_fpsub: op_sub);
                    break;
       case  s_div: emit(f_opr, floating ? op_fpdiv: op_div);
                    break;
       case  s_rem: if (floating)
                        syserr("%% on floating value");
                    emit(f_opr,op_rem);
                    break;       
       case  s_and: if (floating)
                        syserr("& on floating value");
                    emit(f_opr, op_and );
                    break;
       case  s_or:  if (floating)
                        syserr("| on floating value");
                    emit(f_opr, op_or );
                    break;
       case  s_xor: if (floating)
                        syserr("^ on floating value");
                    emit(f_opr, op_xor );
                    break;
       case  s_leftshift: 
                    if (floating)
                        syserr("<< on floating value");
                    emit(f_opr, op_shl );
                    break;
       case  s_rightshift:
                    if (floating)
                        syserr(">> on floating value");
                    emit(f_opr, op_shr );
                    break;
       case  s_equalequal:
                    if (floating)
                        emit(f_opr, op_fpeq);
                    else
                    {
                        emit(f_opr, op_diff );
                        emit(  f_eqc, 0 );
                    }
                    break;
       case  s_diff:if (floating)
       		    {
                        syserr("diff on floating value");
		    }
                    else
		    {
                        emit(f_opr, op_diff );
		    }
                    break;
       case  s_greater: 
       		    if( (mode>>24) == 1 )
       		    {
       		    	if( gtuAllowed )
       		    	{
       		    		emit( f_opr, op_gtu );
       		    	}
       		    	else
       		    	{
				emit( f_opr, op_mint );
				emit( f_opr, op_xor );
				emit( f_opr,op_rev );
				emit( f_opr, op_mint );
				emit( f_opr, op_xor );
				emit( f_opr,op_rev );
				emit( f_opr, op_gt );
			}
       		    }
                    else emit(f_opr, floating ? op_fpgt: op_gt);
                    break;
       
       default:     syserr("Unexpected aeop %d (%s) in codeOperation",
                             op, symbol_name_(op));
    }
}
/*}}}*/
/*{{{  T4 FP ops */

/* The following routines are used to implement floating point emulation on*/
/* the T4. The single precsion float stuff is quite simple, but the double */
/* stuff is seriously complicated by the fact that doubles must always live*/
/* in store, and are passed around as pointers. 			   */

void codeFpCall( op, mode1 )
AEop op; int mode1;
{
	VLocal *v;
	int code = 3;

	int dbl = (mode1==0x02000008);

	push_trace("codeFpCall");
	
	if(debugging(DEBUG_CG))
		trace("codeFpCall(%s,%x) dbl = %d doubledest = %x",
			symbol_name_(op),mode1,dbl,doubledest);

	if( !dbl )	/* single precision floating point */
	{
		switch( op )
		{
		case s_plus:  code--;
		case s_minus: code--;
		case s_times: code--;
		case s_div:
		{
			if( real32op == NULL )
				syserr("codeFpCall: real32op not cached");

			/* this is a temporary kludge to get operands	*/
			/* in the right order for - & /. The proper fix	*/
			/* is to generate them correctly in the first	*/
			/* place.					*/
			if( op == s_minus || op == s_div ) emit( f_opr, op_rev );
			
			emit( f_ldc, code );
			emit( f_opr, op_rev );
			emit( p_fpcall, real32op );
			setInt(FullDepth-1);
			break;
		}

#if 0
		/* actually the front end does not allow this to happen */
		case s_rem:
		{
			VLocal *proc = allocatetemp(INTREG);
			emit( p_ldx, fplib.real32rem );
			emit( p_stvl, proc );
			emit( p_fpcall, proc );
			freetemp(proc);
			break;
		}
#endif		
		case s_greater:
		{
			VLocal *proc = allocatetemp(INTREG);
			/* this is a temporary kludge to get operands	*/
			/* in the right order. The proper fix		*/
			/* is to generate them correctly in the first	*/
			/* place.					*/
			emit( f_opr, op_rev );

			emit( p_ldx, fplib.real32gt );
			emit( p_stvl, proc );
			emit( p_fpcall, proc );
			freetemp(proc);
			setInt(FullDepth-1);
			break;
		}

		case s_equalequal:
		{
			VLocal *proc = allocatetemp(INTREG);
			emit( p_ldx, fplib.real32eq );
			emit( p_stvl, proc );
			emit( p_fpcall, proc );
			freetemp(proc);
			setInt(FullDepth-1);
			break;
		}


		default:
			syserr("Unexpected AeOp in codeFpCall: %d %s",
				op,symbol_name_(op));
		}
	}
	else
	{
		switch( op )
		{
		case s_plus:  code--;
		case s_minus: code--;
		case s_times: code--;
		case s_div:
		{
			if( real64op == NULL )
				syserr("codeFpCall: real64op not cached");

			if( doubledest == NULL ) syserr("double operation without dest");

			/* ensure there is enough room */
			if( ssp == maxssp ) maxssp++;

			emit( f_stl, 0 );

			emit( f_ldc, code );
			emit( f_opr, op_rev );
			doubleaddr(doubledest);
			emit( p_fpcall, real64op );
			doubleaddr(doubledest);
			setInt(FullDepth-1);
			break;
		}

#if 0
		/* actually the front end does not allow this to happen */
		case s_rem:
		{
			VLocal *proc = allocatetemp(INTREG);

			if( doubledest == NULL ) syserr("double operation without dest");

			emit( p_ldx, fplib.real64rem );
			emit( p_stvl, proc );
			doubleaddr(doubledest);
			emit( p_fpcall, proc );
			freetemp(proc);
			doubleaddr(doubledest);
			break;
		}
#endif
		default:
			syserr("Unexpected AeOp in codeFpCall: %d %s",
				op,symbol_name_(op));
		}
	}
	
	pop_trace();
}

#define islocal(x) ( (h0_(x) == s_binder) && ( ( bindstg_((Binder *)(x)) & bitofstg_(s_auto) ) != 0 ) )

void cg_doubleassign( target, source, flag, valneeded )
Expr *target, *source;
int flag, valneeded;
{
	int code = 3;
	VLocal *prevdest = doubledest;
	VLocal *src = NULL;
	int savessp = ssp;
	int op;

	push_trace("cg_doubleassign");
	
	if(debugging(DEBUG_CG))
		trace("cg_doubleassign(%x,%x,%d,%d) doubledest = %x",
			target,source,flag,valneeded,doubledest);

	if( islocal(target) )
	{
		doubledest = (VLocal *)bindxx_((Binder *)target);
	}
	else {
		if( (h0_(target) == s_content) && islocal(arg1_(target)) )
		{
			doubledest = (VLocal *)bindxx_((Binder *)arg1_(target));
		}
		else {
			cg_addrexpr( target );
			doubledest = pushtemp( INTREG );
		}
	}

	/* If the operation is a displace, copy the old value out first	  */
	/* Note that a displace cannot happen in a void context, so there */
	/* MUST be a previous doubledest set up. If not, winge.		  */
	if( flag == s_displace )
	{
		if( prevdest == NULL )
			syserr("Double displace in void context");

		/*
		 * Replaced the following define with memory access checking switch.
		 * Tony 9/1/95.
		 */
/* #ifdef LONGCOPY */
		if (memory_access_checks)
		{
		    doubleaddr(prevdest);
		    emit( f_ldnl, 0 );
		    doubleaddr(doubledest);
		    emit( f_stnl, 0 );

		    doubleaddr(prevdest);
		    emit( f_ldnl, 1 );
		    doubleaddr(doubledest);
		    emit( f_stnl, 1 );
		}
		
/* #else */
		else
		{
		    doubleaddr( prevdest );
		    doubleaddr( doubledest );
		    emit( f_ldc, 8 );
		    emit( f_opr, op_move );
		}
/* #endif */
	}

again:
	if(debugging(DEBUG_CG))	
		trace("source op %s",symbol_name_(h0_(source)));
	switch( op=h0_(source) )
	{
	case s_plus:  code--;
	case s_minus: code--;
	case s_times: code--;
	case s_div:
	case s_rem:
	{
		Expr* a1 = arg1_(source);
		Expr* a2 = arg2_(source);
		int d1 = idepth(depth(a1));
		int d2 = idepth(depth(a2));
		VLocal *olddd = doubledest;
		VLocal *temp = NULL;
		VLocal *v1 = NULL;

		if( real64op == NULL )
			syserr("codeFpCall: real32op not cached");

		if(debugging(DEBUG_CG))
			trace("(%s depth %d) %s (%s depth %d)",
				symbol_name_(h0_(a1)),d1,
				symbol_name_(h0_(source)),
				symbol_name_(h0_(a2)),d2);
				
		/* While in theory it should be possible to use doubledest */
		/* as the destination of one of the following expressions  */
		/* there is always the possibility that one of these	   */
		/* expressions uses the value currently in it. 		   */
		/* For safety, therefore, evaluate them onto the stack.	   */
		/* Note that we only need to allocate temps for expressions*/
		/* which have maximum depth, only constants and binders have*/
		/* less depth, and these do not need a destination.	   */
		/* Note also that indirections may require more than 2 regs*/
		/* we detect this case and don't generate a dest.	   */

		if( d1 >= FullDepth )
		{
			if( h0_(a1) == s_content || h0_(a1) == s_dot )
			{
				cg_addrexpr( a1 );
				v1 = pushtemp( INTREG );
			}
			else {
				v1 = doubledest = allocatetemp(DBLREG);
				cg_expr( a1 );
			}
		}
		/* else we can load a1 later */

		doubledest = olddd;

		if( d2 >= FullDepth )
		{
			if( h0_(a2) != s_content && h0_(a2) != s_dot )
				doubledest = allocatetemp(DBLREG);

			cg_expr( a2 );

			if( v1 == NULL ) cg_expr( a1 );
			else doubleaddr( v1 );

			/* if op is not commutable, get args in right order */
			if( (op == s_div) || (op == s_minus) ) 
				emit( f_opr, op_rev);
		}
		else {
			if( v1 == NULL ) cg_expr( a1 );
			else doubleaddr( v1 );

			cg_expr( a2 );
		}

		/* pointers to the two operands should now be on the stack */
		/* with a2 on top.					   */

		/* ensure there is enough room */
		if( ssp == maxssp ) maxssp++;

		emit( f_stl, 0 );

		emit( f_ldc, code ); 
		emit( f_opr, op_rev );

		doubleaddr(olddd);

		emit( p_fpcall, real64op );

		doubledest = olddd;

		break;
	}

	case s_cond:
	/* Convert    a = (b ? c : d) to                          	*/
	/*    (LET double *g,                                           */
	/*       g = &a,                                                */
	/*       b ? (*g = c) : (*g = d))                               */
	/*                                                              */
        {   TypeExpr *t = typeofexpr(target);
            Binder *gen = gentempbinder(ptrtotype_(t));
	    Expr *e = mk_expr2(s_let,
                         t,
                         (Expr *)mkBindList(0, gen),
                         mk_expr2(s_comma,
                                  t,
                                  mk_expr2(s_assign,
                                           ptrtotype_(t),
                                           (Expr *)gen,
                                           take_address(target)),
                                  mk_expr3(s_cond,
                                           t,
                                           arg1_(source),
                                           mk_expr2(s_assign,
                                                    t,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen),
                                                    arg2_(source)),
                                           mk_expr2(s_assign,
                                                    t,
                                                    mk_expr1(s_content,
                                                             t,
                                                             (Expr *)gen),
                                                    arg3_(source)))));
		pp_expr(e,0);
		cg_exprvoid(e);
		break;		
        }

	case s_cast:
	{
		int mode = cautious_mcrepofexpr( source );
		int argmode = mcrepofexpr(arg1_(source) );
		
		/* this filters out unnecessary casts which can happen	*/
		/* if we have a typedef for double.			*/
		
		if( mode == argmode ) 
		{ source = arg1_(source); goto again; }
		
		cg_cast1( arg1_(source), mode & 0x00ffffff, mode>>24 );

		if( mcrepofexpr(arg1_(source)) != 0x02000008 ) break;
		
		/* here we are casting to or from a typedef, where a 	*/
		/* conversion routine will not be called.		*/
		/*
		 * Replaced the following define with memory access checking switch.
		 * Tony 9/1/95.
		 */
/* #ifdef LONGCOPY */
		if (memory_access_checks)
		{
		    src = pushtemp(INTREG);
		
		    goto copydouble;
		}
/* #else */
		else
		{
		    doubleaddr( doubledest );
		    emit( f_ldc, 8 );
		    emit( f_opr, op_move );
		    break;
		}
/* #endif */
	}

	/* nested assignment, evaluate it and then copy result into our	*/
	/* dest.							*/
	case s_displace:
	case s_assign:
	case s_floatcon:
		cg_expr( source );
		src = pushtemp( INTREG );
		goto copydouble;

	/* for s_dot we convert and treat as content		*/
	case s_dot:
		source = cg_content_for_dot(source);

	/* for s_content we get the address and then copy out	*/
	case s_content:
		if( islocal(arg1_(source)) )
		{
			src = (VLocal *)bindxx_((Binder *)arg1_(source));
		}
		else {
			cg_addrexpr( arg1_(source) );
			src = pushtemp( INTREG );
		}
		goto copydouble;

	case s_binder:
	{
		Binder *b = (Binder *)source;
		if( ( bindstg_(b) & bitofstg_(s_auto) ) != 0 )
		{ 
			/* local -> var 	copy directly */
			src = (VLocal *)bindxx_(b);
		}
		else
		{	
			/* static -> var	go via a local pointer */
			cg_expr( source );
			src = pushtemp(INTREG);
		}
	}

	copydouble:
		/*
		 * Replaced the following define with memory access checking switch.
		 * Tony 9/1/95.
		 */
/* #ifdef LONGCOPY */
	    if (memory_access_checks)
	    {
		doubleaddr( src );
		emit( f_ldnl, 0 );
		doubleaddr(doubledest);
		emit( f_stnl, 0 );

		doubleaddr( src );
		emit( f_ldnl, 1 );
		doubleaddr(doubledest);
		emit( f_stnl, 1 );
	    }
/* #else */
	    else
	    {
		doubleaddr( src );
		doubleaddr( doubledest );
		emit( f_ldc, 8 );
		emit( f_opr, op_move );
	    }
/* #endif */
		break;

		/* the default is to evaluate the expression with doubledest */
		/* set up as necessary.					     */
	default:
		cg_expr( source );
		break;
#if 0
		syserr("Unexpected AeOp in cg_doubleassign: %d %s",
			op,symbol_name_(op));
#endif
	}

	if( valneeded )
	{
		doubleaddr(doubledest);
		setInt(FullDepth-1);
	}
	else setInt(FullDepth);

	/* dispose of all temps */
	ssp = savessp;
	doubledest = prevdest;
	
	pop_trace();
}

/* When compiling comparisons we may need to supply a destination for the */
/* two sides. We only do this if the depth of the expressions is >= 3 	  */
/* since variables and constant dont need it, and have depth=1.		  */
/* NOTE: a1 & a2 swapped over here to fix a small bug.			  */

void cg_doublecmp( op, a2, a1 )
AEop op;
Expr *a1, *a2;
{
	int d1 = idepth(depth(a1));
	int d2 = idepth(depth(a2));
	int savessp = ssp;
	VLocal *olddd = doubledest;
	Binder *func = op==s_greater ? fplib.real64gt : fplib.real64eq ;
	VLocal *proc = allocatetemp( INTREG );

	push_trace("cg_doublecmp");
	
	if( debugging(DEBUG_CG))
		trace("cg_doublecmp(%s,%x,%x) d1 %x d2 %x",
			symbol_name_(op),a1,a2,d1,d2);

	emit( p_ldx, func );
	emit( p_stvl, proc );

	if( d1 >= FullDepth ) doubledest = allocatetemp( DBLREG );

	cg_expr( a1 );

	doubledest = NULL;

	if( d2 >= FullDepth )
	{
		VLocal *tmp = allocatetemp( INTREG );

		doubledest = allocatetemp( DBLREG );

		emit( p_stvl, tmp ); popInt();

		cg_expr( a2 );

		emit( p_ldvl, tmp );  pushInt();
		if( op == s_greater ) emit( f_opr, op_rev );
	}
	else cg_expr( a2 );

	emit( p_fpcall, proc );

	doubledest = olddd;
	ssp = savessp;

	setInt(FullDepth-1);
	
	pop_trace();
}
/*}}}*/
