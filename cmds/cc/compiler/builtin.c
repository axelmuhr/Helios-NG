
/* c.builtin: constants/global symbols for C compiler */
/* Copyright (C) A.Mycroft and A.C.Norman       */
/* version 0.07 */
/* $Id: builtin.c,v 1.1 1990/09/13 17:09:37 nick Exp $ */

#include <time.h>
#include "AEops.h"
#include "cchdr.h"

TypeExpr *te_int;    /* = (global)primtype_(bitoftype_(s_int)) */
TypeExpr *te_uint, *te_lint, *te_ulint;  /* and friends */
TypeExpr *te_double; /* = (global)primtype_(bitoftype_(s_double)) */
TypeExpr *te_float;  /* its short friend */
TypeExpr *te_ldble;  /* and its long one */
TypeExpr *te_void;   /* = (global)primtype_(bitoftype_(s_void)) */

/* since no-one looks inside datasegment and code segment perhaps they
   should be Symstr's */
Binder *datasegment, *codesegment;
Symstr *mainsym, *setjmpsym;
Symstr *libentrypoint, *stackoverflow, *stackoverflow1, *countroutine;

op_simulation sim;

static Expr *library_function(name,minf,maxf)
char *name;
int minf;
int maxf;
{
    Symstr *w = sym_insert(name, s_identifier);
    Binder *b;
    b = global_mk_binder(0,
                         w,
                         bitofstg_(s_extern),
                         global_list6(t_fnap, te_int, 0, minf, maxf, 0));
    return global_list3(s_addrof,
                        te_int,  /* oh what a lie */
                        b);
}


#ifdef SOFTWARE_FLOATING_POINT

static Expr *floating_function(int nargs, TypeExpr *result,
                                TypeExpr *a1, TypeExpr *a2, char *name)
{
    Symstr *w = sym_insert(name, s_identifier),
           *a_name = sym_insert("a", s_identifier),
           *b_name = sym_insert("b", s_identifier);
    Binder *b;
    FormTypeList *a = global_list3(0, a_name, a1);
    if (nargs != 1) a->ftcdr = global_list3(0, b_name, a2);
    b = global_mk_binder(0,
                         w,
                         bitofstg_(s_extern),
                         global_list6(t_fnap, result, a, nargs, nargs, 0));
    return (Expr *)b;
}
#endif

#ifdef UNIQUE_DATASEG_NAMES
/* The following routine hacks round a bug in Acorn's linker (June 87) */
/* w.r.t. local symbols in different files being confused.             */
/* Something like it is probably needed for 370 CSECT names.           */
/* Acorn linker bug corrected July 87, so this code disabled.          */
static char *probably_unique_name(ch)
int ch;
{
    static char name[32];
    sprintf(name, "v$%c%x", ch, 20*time(NULL)+main_compilation_count);
    return name;
}
#endif

void builtin_init()
{
#define initprimtype_(t) global_list3(s_typespec, (t), 0);
    te_int = initprimtype_(bitoftype_(s_int));
    te_uint = initprimtype_(bitoftype_(s_int)|bitoftype_(s_unsigned));
    te_lint = initprimtype_(bitoftype_(s_int)|bitoftype_(s_long));
    te_ulint = initprimtype_(bitoftype_(s_int)|bitoftype_(s_long)|bitoftype_(s_unsigned));
    te_double = initprimtype_(bitoftype_(s_double));
    te_float = initprimtype_(bitoftype_(s_double)|bitoftype_(s_short));
    te_ldble = initprimtype_(bitoftype_(s_double)|bitoftype_(s_long));
    te_void = initprimtype_(bitoftype_(s_void));

    sim.mulfn = library_function("x$multiply", 2, 2);
    sim.divfn = library_function("x$divide", 2, 2);
    sim.udivfn = library_function("x$udivide", 2, 2);
    sim.divtestfn = library_function("x$divtest", 1, 1);
    sim.remfn = library_function("x$remainder", 2, 2);
    sim.uremfn = library_function("x$uremainder", 2, 2);
    sim.xprintf = library_function("_printf", 1, 1999);
    sim.xfprintf = library_function("_fprintf", 2, 1999);
    sim.xsprintf = library_function("_sprintf", 2, 1999);
    sim.yprintf = sym_insert("printf", s_identifier);
    sim.yfprintf = sym_insert("fprintf", s_identifier);
    sim.ysprintf = sym_insert("sprintf", s_identifier);
#ifdef SOFTWARE_FLOATING_POINT
    sim.dadd = floating_function(2,te_double,te_double,te_double,"_dadd");
    sim.dsubtract = floating_function(2,te_double,te_double,te_double,"_dsub");
    sim.dmultiply = floating_function(2,te_double,te_double,te_double,"_dmul");
    sim.ddivide = floating_function(2,te_double,te_double,te_double,"_ddiv");
    sim.dnegate = floating_function(1,te_double,te_double,NULL,"_dneg");
    sim.dgreater = floating_function(2,te_int,te_double,te_double,"_dgr");
    sim.dgeq = floating_function(2,te_int,te_double,te_double,"_dgeq");
    sim.dless = floating_function(2,te_int,te_double,te_double,"_dls");
    sim.dleq = floating_function(2,te_int,te_double,te_double,"_dleq");
    sim.dequal = floating_function(2,te_int,te_double,te_double,"_deq");
    sim.dneq = floating_function(2,te_int,te_double,te_double,"_dneq");
    sim.dfloat = floating_function(1,te_double,te_int,NULL,"_dflt");
    sim.dfloatu = floating_function(1,te_double,te_uint,NULL,"_dfltu");
    sim.dfix = floating_function(1,te_int,te_double,NULL,"_dfix");
    sim.dfixu = floating_function(1,te_uint,te_double,NULL,"_dfixu");

    sim.fadd = floating_function(2,te_float,te_int,te_int,"_fadd");
    sim.fsubtract = floating_function(2,te_float,te_int,te_int,"_fsub");
    sim.fmultiply = floating_function(2,te_float,te_int,te_int,"_fmul");
    sim.fdivide = floating_function(2,te_float,te_int,te_int,"_fdiv");
    sim.fnegate = floating_function(1,te_float,te_int,NULL,"_fneg");
    sim.fgreater = floating_function(2,te_int,te_int,te_int,"_fgr");
    sim.fgeq = floating_function(2,te_int,te_int,te_int,"_fgeq");
    sim.fless = floating_function(2,te_int,te_int,te_int,"_fls");
    sim.fleq = floating_function(2,te_int,te_int,te_int,"_fleq");
    sim.fequal = floating_function(2,te_int,te_int,te_int,"_feq");
    sim.fneq = floating_function(2,te_int,te_int,te_int,"_fneq");
    sim.ffloat = floating_function(1,te_float,te_int,NULL,"_fflt");
    sim.ffloatu = floating_function(1,te_float,te_uint,NULL,"_ffltu");
    sim.ffix = floating_function(1,te_int,te_int,NULL,"_ffix");
    sim.ffixu = floating_function(1,te_uint,te_int,NULL,"_ffixu");

    sim.fnarrow = floating_function(1,te_float,te_double,NULL,"_d2f");
    sim.dwiden = floating_function(1,te_double,te_float,NULL,"_f2d");
#endif
    sim.readcheck1 = library_function("x$readcheck1", 1, 1);
    sim.readcheck2 = library_function("x$readcheck2", 1, 1);
    sim.readcheck4 = library_function("x$readcheck4", 1, 1);
    sim.writecheck1 = library_function("x$writecheck1", 1, 1);
    sim.writecheck2 = library_function("x$writecheck2", 1, 1);
    sim.writecheck4 = library_function("x$writecheck4", 1, 1);
    sim.memcpyfn = library_function("_memcpy", 3, 3);

    stackoverflow = sym_insert("x$stack_overflow", s_identifier);
    stackoverflow1 = sym_insert("x$stack_overflow_1", s_identifier);
    datasegment = global_mk_binder(0,
#ifdef UNIQUE_DATASEG_NAMES
                sym_insert(probably_unique_name('d'), s_identifier),
#else
                sym_insert("v$dataseg", s_identifier),
#endif
                bitofstg_(s_static),
                te_int);
    codesegment = global_mk_binder(0,
#ifdef UNIQUE_DATASEG_NAMES
                sym_insert(probably_unique_name('c'), s_identifier),
#else
                sym_insert("v$codeseg", s_identifier),
#endif
                bitofstg_(s_static),
                te_int);
    mainsym = sym_insert("main", s_identifier);
    setjmpsym = sym_insert("setjmp", s_identifier);
    libentrypoint = sym_insert("__main", s_identifier);
    countroutine = sym_insert("_count", s_identifier);
}

/* end of builtin.c */
