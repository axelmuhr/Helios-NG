#ifdef USE_NORCROFT_PRAGMAS
#pragma force_top_level
#pragma include_only_once
#endif
/*
 * mip/builtin.h:
 * Copyright (C) Acorn Computers Ltd., 1988.
 * Copyright (C) Codemist Ltd., 1987.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1994/02/27 12:47:23 $
 * Revising $Author: nickc $
 */

#ifndef _builtin_h
#define _builtin_h

#ifndef _defs_LOADED
#  include "defs.h"
#endif

extern FloatCon  *fc_two_31;    /* floating point constant 2^31 */

typedef struct {
    FloatCon *s;        /* short version */
    FloatCon *d;        /* double version */
} FPConst;

extern FPConst fc_zero;         /* floating point constants 0.0  */
#ifdef PASCAL /*ECN*/
extern FPConst fc_half;         /*                          0.5  */
extern FPConst fc_big;          /*               FLT or DBL MAX  */
#endif
extern FPConst fc_one;          /*                          1.0  */
extern FPConst fc_two;          /*                          2.0  */
extern FPConst fc_minusone;     /*                          -1.0  */

extern TypeExpr *te_int;    /* = (global)primtype_(bitoftype_(s_int)) */
extern TypeExpr *te_uint, *te_lint, *te_ulint;  /* and friends */
extern TypeExpr *te_double; /* = (global)primtype_(bitoftype_(s_double)) */
extern TypeExpr *te_float;  /* its short friend */
extern TypeExpr *te_ldble;  /* and its long one */
extern TypeExpr *te_void;   /* = (global)primtype_(bitoftype_(s_void)) */

extern Binder *datasegment, *codesegment;
#ifdef TARGET_HAS_BSS
extern Binder *bsssegment;
#endif
extern Symstr *mainsym, *setjmpsym, *assertsym, *first_arg_sym, *last_arg_sym;
extern Symstr *libentrypoint, *stackoverflow, *stack1overflow,
              *countroutine, *count1routine;
#ifdef TARGET_IS_C40
extern Symstr * SaveCPUStatesym;
#endif

#ifdef TARGET_IS_ACW
extern Symstr *c_handler, *stackcheck, *heapend;
#endif
#ifdef TARGET_IS_KCM
extern Symstr *FPArg1, *FPArg2, *cnvtdw_routine, *cnvtwd_routine,
  *cnvtsd_routine, *cnvtds_routine,
  *addd_routine, *subd_routine, *muld_routine, *divd_routine,
  *cmpd_routine, *divu_routine, *remu_routine;
#endif
#ifdef TARGET_IS_SPARC
extern Symstr *fparg1;
#endif

#if defined TARGET_HAS_DEBUGGER && defined TARGET_IS_HELIOS
extern Symstr *	_notify_entry;
extern Symstr *	_notify_return;
extern Symstr *	_notify_command;
#endif

typedef struct op_simulation
  {
    Expr *	mulfn;
    Expr *	divfn;
    Expr *	udivfn;
    Expr *	divtestfn;
    Expr *	remfn;
    Expr *	uremfn;
#ifdef TARGET_LACKS_FP_DIVIDE
    Expr *	fdivfn;
    Expr *	ddivfn;
#endif
    Expr *	memcpyfn;
    Expr *	memsetfn;
    Expr *	inserted_word;
    Expr *	readcheck1;
    Expr *	readcheck2;
    Expr *	readcheck4;
    Expr *	writecheck1;
    Expr *	writecheck2;
    Expr *	writecheck4;
    Expr *	xprintf;
    Expr *	xfprintf;
    Expr *	xsprintf;
    Symstr *	yprintf;
    Symstr *	yfprintf;
    Symstr *	ysprintf;
#ifdef RANGECHECK_SUPPORTED
    Symstr *	abcfault;
    Symstr *	valfault;
#endif
#ifdef SOFTWARE_FLOATING_POINT
    Expr *dadd, *dsubtract, *dmultiply, *ddivide, *dnegate,
     *dgreater, *dgeq, *dless, *dleq, *dequal, *dneq, *dfloat, *dfloatu,
     *dfix, *dfixu;
    Expr *fadd, *fsubtract, *fmultiply, *fdivide, *fnegate,
     *fgreater, *fgeq, *fless, *fleq, *fequal, *fneq, *ffloat, *ffloatu,
     *ffix, *ffixu;
    Expr *fnarrow, *dwiden;
#endif
  }
op_simulation;

extern op_simulation sim;

extern void builtin_init(void);

#endif

/* end of mip/builtin.h */
