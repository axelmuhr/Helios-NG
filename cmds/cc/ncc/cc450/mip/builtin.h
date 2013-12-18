#pragma force_top_level
#pragma include_only_once
/*
 * mip/builtin.h:
 * Copyright (C) Acorn Computers Ltd., 1988-1990.
 * Copyright (C) Codemist Ltd., 1987-1992.
 * Copyright (C) Advanced RISC Machines Limited, 1991-1992.
 */

/*
 * RCS $Revision: 1.3 $
 * Checkin $Date: 1994/02/27 12:57:41 $
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

extern Binder *datasegment, *codesegment, *constdatasegment;
#ifdef TARGET_HAS_BSS
extern Binder *bsssegment;
#endif
extern Symstr *mainsym, *setjmpsym, *assertsym, *first_arg_sym, *last_arg_sym;
extern Symstr *thissym, *ctorsym, *dtorsym, *vtabsym;
extern Symstr *libentrypoint, *stackoverflow, *stack1overflow,
              *countroutine, *count1routine;
#ifdef TARGET_IS_C40
extern Symstr * SaveCPUStatesym;
#endif

#ifdef TARGET_IS_ACW
extern Symstr *c_handler, *stackcheck, *heapend;
#endif
#ifdef TARGET_IS_KCM
/* @@@ These should go in struct op_simulation to avoid name clutter.   */
extern Symstr *FPArg1, *FPArg2, *FPArg1x, *FPArg2x, *cnvtdw_routine,
  *cnvtwd_routine, *cnvtsd_routine, *cnvtds_routine,
  *addd_routine, *subd_routine, *negd_routine, *muld_routine, *divd_routine,
  *cmpd_routine, *divu_routine, *remu_routine;
#endif
#ifdef TARGET_IS_SPARC
extern Symstr *fparg1;
extern Symstr *multiply;
extern Symstr *divide;
extern Symstr *udivide;
#endif

#if defined TARGET_HAS_DEBUGGER && defined TARGET_IS_HELIOS
extern Symstr *	_notify_entry;
extern Symstr *	_notify_return;
extern Symstr *	_notify_command;
#endif

typedef struct op_simulation {
   Expr *mulfn, *divfn, *udivfn, *divtestfn, *remfn, *uremfn;
#ifdef TARGET_HAS_DIV_10_FUNCTION
   Expr *div10fn, *udiv10fn, *rem10fn, *urem10fn;
#endif
   Expr *fdivfn, *ddivfn;
   Expr *memcpyfn, *memsetfn;
   Expr *inserted_word;
   Expr *readcheck1, *readcheck2, *readcheck4,
        *writecheck1, *writecheck2, *writecheck4;
   Expr *xprintf, *xfprintf, *xsprintf;
   Expr *realmemcpyfn;
   Symstr *strcpysym;
   Symstr *yprintf, *yfprintf, *ysprintf;
#ifdef RANGECHECK_SUPPORTED
   Symstr *abcfault, *valfault;
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
} op_simulation;

extern op_simulation sim;

extern bool returnsheapptr(Symstr *fn);

extern void builtin_init(void);

#endif

/* end of mip/builtin.h */
