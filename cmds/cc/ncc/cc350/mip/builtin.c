/*
 * builtin.c: constants/global symbols for C compiler, version 15
 * Copyright (C) Codemist Ltd., 1987.
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1994/02/27 12:47:01 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char builtin_version[];
char builtin_version[] = "\nbuiltin.c $Revision: 1.2 $ 15\n";
#endif

/* AM memo: more thought is required in this file to account for        */
/* natural (and unnatural) machine parameterisations.  In particular    */
/* getting the bsd vax/bsd sun/sysV names right is a pain.              */


#include <time.h>
#include "globals.h"
#include "defs.h"
#include "builtin.h"
#include "bind.h"
#include "store.h"
#include "aeops.h"
#include "aetree.h"
#include "xrefs.h"		/* For obj_symref */

#ifdef TARGET_IS_HELIOS
#include <string.h>
#endif

/* The following line indicates more thought is required re naming. */
#ifdef TARGET_LINKER_OMITS_DOLLAR
#  define SYSPFX "__"
#else
#  define SYSPFX "x$"
#endif

FloatCon  *fc_two_31;   /* floating point constant 2^31 */

FPConst fc_zero;         /* floating point constants 0.0  */
#ifdef PASCAL /*ECN*/
FPConst fc_half;         /*                          0.5  */
FPConst fc_big;          /*               FLT or DBL MAX  */
#endif
FPConst fc_one;          /*                          1.0  */
FPConst fc_two;          /*                          2.0  */
FPConst fc_minusone;     /*                          -1.0  */

TypeExpr *te_int;    /* = (global)primtype_(bitoftype_(s_int)) */
TypeExpr *te_uint, *te_lint, *te_ulint;  /* and friends */
TypeExpr *te_double; /* = (global)primtype_(bitoftype_(s_double)) */
TypeExpr *te_float;  /* its short friend */
TypeExpr *te_ldble;  /* and its long one */
TypeExpr *te_void;   /* = (global)primtype_(bitoftype_(s_void)) */

/* since no-one looks inside datasegment and code segment perhaps they
   should be Symstr's */
Binder *datasegment, *codesegment;
#ifdef TARGET_HAS_BSS
Binder *bsssegment;
#endif
Symstr *mainsym, *setjmpsym, *assertsym, *first_arg_sym, *last_arg_sym;
Symstr *libentrypoint, *stackoverflow, *stack1overflow,
       *countroutine, *count1routine;
#ifdef TARGET_IS_C40
Symstr * SaveCPUStatesym;
#endif

#ifdef TARGET_IS_ACW
Symstr *c_handler, *stackcheck, *heapend;
#endif
#ifdef TARGET_IS_KCM
Symstr *FPArg1, *FPArg2, *cnvtdw_routine, *cnvtwd_routine, *cnvtsd_routine,
  *cnvtds_routine, *addd_routine, *subd_routine, *muld_routine, *divd_routine,
  *cmpd_routine, *divu_routine, *remu_routine;
#endif
#ifdef TARGET_IS_SPARC
Symstr *fparg1;
#endif

#if defined TARGET_HAS_DEBUGGER && defined TARGET_IS_HELIOS
Symstr *	_notify_entry;		/* called after arguments & variables saved on stack */
Symstr *	_notify_return;		/* tail call after stack has been restored */
Symstr *	_notify_command;	/* called before executing code corresponding to a source line */
#endif

op_simulation sim;

static Expr *library_function(char *name, int minf, int maxf,
                              bool nosideeffects)
{
    Symstr *w = sym_insert_id(name);
    Binder *b;
    TypeExprFnAux s;
    TypeExpr *t = g_mkTypeExprfn(t_fnap, te_int, 0,
                      packTypeExprFnAux(s, minf, maxf, 0, nosideeffects, 0));
    b = global_mk_binder(0,
                         w,
                         bitofstg_(s_extern) | b_undef | b_fnconst,
                         t);
    return (Expr*) global_list3(SU_Other, s_addrof,
                        te_int,  /* oh what a lie */
                        b);
}


#if defined SOFTWARE_FLOATING_POINT

static Expr *floating_function(int nargs, TypeExpr *result,
                               TypeExpr *a1, TypeExpr *a2, char *name)
{
    Symstr *w = sym_insert_id(name),
           *a_name = sym_insert_id("a"),
           *b_name = sym_insert_id("b");
    Binder *b;
    FormTypeList *a = global_list3(SU_Other, 0, a_name, a1);
    TypeExprFnAux s;
    if (nargs != 1) a->ftcdr = global_list3(SU_Other, 0, b_name, a2);
    b = global_mk_binder(0,
                         w,
                         bitofstg_(s_extern) | b_undef | b_fnconst,
                         g_mkTypeExprfn(t_fnap, result, a,
                            packTypeExprFnAux(s, nargs, nargs, 0, 0, 0)));
    return (Expr *)b;
}
#endif

#ifdef UNIQUE_DATASEG_NAMES
/* The following routine hacks round a bug in Acorn's linker (June 87) */
/* w.r.t. local symbols in different files being confused.             */
/* Something like it is probably needed for 370 CSECT names.           */
/* Acorn linker bug corrected July 87, so this code disabled.          */
/* ... but the code is still useful for Helios!                        */

static int main_compilation_count = 0;

static char *probably_unique_name(int ch)
{
    static char name[32];
#ifdef TARGET_LINKER_OMITS_DOLLAR
    sprintf(name, "__%c%lx", ch, (long)(20L*time(NULL)+main_compilation_count));
#else
    sprintf(name, "x$%c%lx", ch, (long)(20L*time(NULL)+main_compilation_count));
#endif
    return name;
}
#endif

static void initfpconst(FPConst *fc, const char val[])
{
    fc->s = real_of_string(val, bitoftype_(s_double)|bitoftype_(s_short));
    fc->d = real_of_string(val, bitoftype_(s_double));
}

void builtin_init(void)
{
    initfpconst(&fc_zero, "0.0");
#ifdef PASCAL /*ECN*/
    initfpconst(&fc_half, "0.5");
    fc_big.s = real_of_string("3.40282347e+38", bitoftype_(s_double) |
                                                bitoftype_(s_short));
    fc_big.d = real_of_string("1.79769313486231571e+308",
                                                bitoftype_(s_double));
#endif
    initfpconst(&fc_one, "1.0");
    initfpconst(&fc_two, "2.0");
    initfpconst(&fc_minusone, "-1.0");

    fc_two_31 = real_of_string("2147483648.0", bitoftype_(s_double));
#define initprimtype_(t) (TypeExpr*)global_list4(SU_Other, s_typespec, (t),0,0);
    te_int = initprimtype_(bitoftype_(s_int));
    te_uint = initprimtype_(bitoftype_(s_int)|bitoftype_(s_unsigned));
    te_lint = initprimtype_(bitoftype_(s_int)|bitoftype_(s_long));
    te_ulint = initprimtype_(bitoftype_(s_int)|bitoftype_(s_long)|
                             bitoftype_(s_unsigned));
    te_double = initprimtype_(bitoftype_(s_double));
    te_float = initprimtype_(bitoftype_(s_double)|bitoftype_(s_short));
    te_ldble = initprimtype_(bitoftype_(s_double)|bitoftype_(s_long));
    te_void = initprimtype_(bitoftype_(s_void));

#if defined(TARGET_IS_UNIX) && !defined(TARGET_IS_SPARC)
    sim.mulfn = library_function("x$mul", 2, 2, YES);
    sim.divfn = library_function("x$div", 2, 2, YES);
    sim.udivfn = library_function("x$udiv", 2, 2, YES);
    sim.divtestfn = library_function("x$divtest", 1, 1, YES);
    sim.remfn = library_function("x$mod", 2, 2, YES);
    sim.uremfn = library_function("x$umod", 2, 2, YES);
    sim.fdivfn = library_function("x$fdiv", 2, 2, YES);
    sim.ddivfn = library_function("x$ddiv", 2, 2, YES);
#else
#ifdef TARGET_LINKER_OMITS_DOLLAR
    sim.mulfn = library_function("__multiply", 2, 2, YES);
    sim.divfn = library_function("__divide", 2, 2, YES);
    sim.udivfn = library_function("__udivide", 2, 2, YES);
    sim.divtestfn = library_function("__divtest", 1, 1, YES);
    sim.remfn = library_function("__remainder", 2, 2, YES);
    sim.uremfn = library_function("__uremainder", 2, 2, YES);
#ifdef TARGET_LACKS_FP_DIVIDE
    sim.fdivfn = library_function("__fdivide", 2, 2, YES);
    sim.ddivfn = library_function("__ddivide", 2, 2, YES);
#endif
#else
    sim.mulfn     = library_function("x$multiply", 2, 2, YES);
    sim.divfn     = library_function("x$divide", 2, 2, YES);
    sim.udivfn    = library_function("x$udivide", 2, 2, YES);
    sim.divtestfn = library_function("x$divtest", 1, 1, YES);
    sim.remfn     = library_function("x$remainder", 2, 2, YES);
    sim.uremfn    = library_function("x$uremainder", 2, 2, YES);
#ifdef TARGET_LACKS_FP_DIVIDE
    sim.fdivfn    = library_function("x$fdivide", 2, 2, YES);
    sim.ddivfn    = library_function("x$ddivide", 2, 2, YES);
#endif /* LACKS_FP_DIVIDE */
#endif /* LINKER_OMITS_DOLLAR */
#endif /* UNIX not SPARC */
    sim.xprintf   = library_function("_printf", 1, 1999, NO);
    sim.xfprintf  = library_function("_fprintf", 2, 1999, NO);
    sim.xsprintf  = library_function("_sprintf", 2, 1999, NO);
    sim.yprintf   = sym_insert_id("printf");
    sim.yfprintf  = sym_insert_id("fprintf");
    sim.ysprintf  = sym_insert_id("sprintf");
#ifdef SOFTWARE_FLOATING_POINT
    sim.dadd      = floating_function(2,te_double,te_double,te_double,"_dadd");
    sim.dsubtract = floating_function(2,te_double,te_double,te_double,"_dsub");
    sim.dmultiply = floating_function(2,te_double,te_double,te_double,"_dmul");
    sim.ddivide   = floating_function(2,te_double,te_double,te_double,"_ddiv");
    sim.dnegate   = floating_function(1,te_double,te_double,NULL,"_dneg");
    sim.dgreater  = floating_function(2,te_int,te_double,te_double,"_dgr");
    sim.dgeq      = floating_function(2,te_int,te_double,te_double,"_dgeq");
    sim.dless     = floating_function(2,te_int,te_double,te_double,"_dls");
    sim.dleq      = floating_function(2,te_int,te_double,te_double,"_dleq");
    sim.dequal    = floating_function(2,te_int,te_double,te_double,"_deq");
    sim.dneq      = floating_function(2,te_int,te_double,te_double,"_dneq");
    sim.dfloat    = floating_function(1,te_double,te_int,NULL,"_dflt");
    sim.dfloatu   = floating_function(1,te_double,te_uint,NULL,"_dfltu");
    sim.dfix      = floating_function(1,te_int,te_double,NULL,"_dfix");
    sim.dfixu     = floating_function(1,te_uint,te_double,NULL,"_dfixu");

    sim.fadd      = floating_function(2,te_float,te_int,te_int,"_fadd");
    sim.fsubtract = floating_function(2,te_float,te_int,te_int,"_fsub");
    sim.fmultiply = floating_function(2,te_float,te_int,te_int,"_fmul");
    sim.fdivide   = floating_function(2,te_float,te_int,te_int,"_fdiv");
    sim.fnegate   = floating_function(1,te_float,te_int,NULL,"_fneg");
    sim.fgreater  = floating_function(2,te_int,te_int,te_int,"_fgr");
    sim.fgeq      = floating_function(2,te_int,te_int,te_int,"_fgeq");
    sim.fless     = floating_function(2,te_int,te_int,te_int,"_fls");
    sim.fleq      = floating_function(2,te_int,te_int,te_int,"_fleq");
    sim.fequal    = floating_function(2,te_int,te_int,te_int,"_feq");
    sim.fneq      = floating_function(2,te_int,te_int,te_int,"_fneq");
    sim.ffloat    = floating_function(1,te_float,te_int,NULL,"_fflt");
    sim.ffloatu   = floating_function(1,te_float,te_uint,NULL,"_ffltu");
    sim.ffix      = floating_function(1,te_int,te_int,NULL,"_ffix");
    sim.ffixu     = floating_function(1,te_uint,te_int,NULL,"_ffixu");

    sim.fnarrow   = floating_function(1,te_float,te_double,NULL,"_d2f");
    sim.dwiden    = floating_function(1,te_double,te_float,NULL,"_f2d");
#endif
    sim.readcheck1  = library_function("_rd1chk", 1, 1, YES);
    sim.readcheck2  = library_function("_rd2chk", 1, 1, YES);
    sim.readcheck4  = library_function("_rd4chk", 1, 1, YES);
    sim.writecheck1 = library_function("_wr1chk", 1, 1, YES);
    sim.writecheck2 = library_function("_wr2chk", 1, 1, YES);
    sim.writecheck4 = library_function("_wr4chk", 1, 1, YES);
#if alignof_struct == 1
/* The following is a bit of a hack for MIPS, but the essence is that   */
/* if we hope to inline memcpy then we may wish to rely on alignment    */
/* of structs to do word copy.  If we used memcpy instead of _memcpy    */
/* when alignof_struct=4 then we would not be able to distinguish an    */
/* aligned call struct a=b; from a non-aligned call memcpy(a+1,b+1,4).  */
    sim.memcpyfn = library_function("memcpy", 3, 3, NO);
    sim.memsetfn = library_function("memset", 3, 3, NO);
#else
    sim.memcpyfn = library_function("_memcpy", 3, 3, NO);
    sim.memsetfn = library_function("_memset", 3, 3, NO);
#endif
/* _word(nnn) is a specially-treated 'function' to put nnn in-line in the */
/* generated code.  People may have views on a better name for it, esp.   */
/* in view of machines with byte and halfword instructions!               */
/* Introduced by ACN to help him with an 88000 library.                   */
    sim.inserted_word = library_function("_word", 1, 1, NO);

#ifdef TARGET_IS_ACW
    c_handler = sym_insert_id(SYSPFX"c_handler");
    stackcheck = sym_insert_id(SYSPFX"stackcheck");
    heapend = sym_insert_id("CurrentHeapEnd");
#endif
#ifdef TARGET_LINKER_OMITS_DOLLAR
    stackoverflow = sym_insert_id("__stack_overflow");
    stack1overflow = sym_insert_id("__stack_overflow_1");
#else
    stackoverflow = sym_insert_id("x$stack_overflow");
    stack1overflow = sym_insert_id("x$stack_overflow_1");
#endif

#ifdef TARGET_IS_HELIOS
      {
	extern int	suppress_module;
	char		name[ 51 ];
	 

	/*
	 * XXX - NC - 13/12/91
	 *
	 * This may be ugly, but it works.
	 *
	 * If we are building resident libraries then each file
	 * in the library must use a unique data segment name,
	 * or else they will clash when the files are linked.
	 * (The transputer linker had some sneaky code to cope
	 * with this, I do not have this luxury.  *sigh* )
	 */
	
	strcpy( name, "__dataseg" );
	
	if (suppress_module)
	  {
	    char *		ptr;
	    
	    
	    strcat( name, "_" );

	    /* skip directory componenets in source file's name */
	    
	    if ((ptr = strrchr( sourcefile, '/' )) != NULL)
	      ++ptr;
	    else
	      ptr = sourcefile;

	    /* copy source file name onto end of name */
	    
	    strncat( name, ptr, 40 );
	  }
	
	datasegment = global_mk_binder(0,
				       sym_insert_id(name),
				       bitofstg_(s_static),
				       te_int);
      }    
#else
    datasegment = global_mk_binder(0,
#ifdef UNIQUE_DATASEG_NAMES
                sym_insert_id(probably_unique_name('d')),
#else
#ifdef TARGET_LINKER_OMITS_DOLLAR
                sym_insert_id("__dataseg"),
#else
                sym_insert_id("x$dataseg"),
#endif
#endif
                bitofstg_(s_static),
                te_int);
#endif
    
    codesegment = global_mk_binder(0,
#ifdef UNIQUE_DATASEG_NAMES
                sym_insert_id(probably_unique_name('c')),
#else
#ifdef TARGET_LINKER_OMITS_DOLLAR
                sym_insert_id("__codeseg"),
#else
                sym_insert_id("x$codeseg"),
#endif
#endif
                bitofstg_(s_static),
                te_int);
#ifdef TARGET_HAS_BSS
    bsssegment = global_mk_binder(0,
#ifdef UNIQUE_DATASEG_NAMES
                sym_insert_id(probably_unique_name('z')),
#else
#ifdef TARGET_LINKER_OMITS_DOLLAR
                sym_insert_id("_bssseg"),
#else
                sym_insert_id("x$bssseg"),
#endif
#endif
                bitofstg_(s_static),
                te_int);
#endif
    mainsym = sym_insert_id("main");
    setjmpsym = sym_insert_id("setjmp");
#ifdef TARGET_IS_C40
    /* SaveCPUState() is functionally equivalent to setjmp() and must have similar protection */
    SaveCPUStatesym = sym_insert_id( "SaveCPUState" );
#endif
    assertsym = sym_insert_id("___assert");
/* AM: hmm, is the name '___assert right in that users might get to see */
/* it if (say) a semicolon is omitted (check macro which use) and       */
/* query the next line which would mean ___assert without () fn call    */
/* would not get reported, or be done confusingly.  Probably OK.        */
    implicit_decl(assertsym, 1);    /* forge an 'extern int ___assert()' */
    first_arg_sym = sym_insert_id("___first_arg");
    last_arg_sym = sym_insert_id("___last_arg");
    libentrypoint = sym_insert_id("__main");
#ifdef TARGET_LINKER_OMITS_DOLLAR
    countroutine = sym_insert_id("__mcount");/*for Unix, x$ goes*/
#else
    countroutine = sym_insert_id("x$mcount");/*for Unix, x$ goes*/
#endif
    count1routine = sym_insert_id("_count1");
#ifdef RANGECHECK_SUPPORTED
#ifdef PASCAL /*ECN*/
    sim.abcfault = sym_insert_id("_range");
    sim.valfault = sym_insert_id("_badvalue");
#else
    sim.abcfault = sym_insert_id("s_rnge"); /* BSD F77 library name */
    sim.valfault = sym_insert_id("x$badvalue");
#endif
#endif
#ifdef TARGET_IS_KCM
    FPArg1         = sym_insert_id("_ARG1");
    FPArg2         = sym_insert_id("_ARG2");
    cnvtdw_routine = sym_insert_id("_CNVTDW");
    cnvtwd_routine = sym_insert_id("_CNVTWD");
    cnvtds_routine = sym_insert_id("_CNVTDS");
    cnvtsd_routine = sym_insert_id("_CNVTSD");
    addd_routine   = sym_insert_id("_ADDD");
    subd_routine   = sym_insert_id("_SUBD");
    muld_routine   = sym_insert_id("_MULD");
    divd_routine   = sym_insert_id("_DIVD");
    cmpd_routine   = sym_insert_id("_CMPD");
    divu_routine   = sym_insert_id("_DIVU");
    remu_routine   = sym_insert_id("_REMU");
#endif
#ifdef TARGET_IS_SPARC
    fparg1         = sym_insert_id("_fparg1");
/*    implicit_decl (fparg1, 0); */
/*    obj_symref(fparg1, 0, 0);  */
#endif

#if defined TARGET_HAS_DEBUGGER && TARGET_IS_HELIOS
    _notify_entry   = sym_insert_id( "_notify_entry"   );
    _notify_return  = sym_insert_id( "_notify_return"  );
    _notify_command = sym_insert_id( "_notify_command" );
#endif

}

/* end of builtin.c */
