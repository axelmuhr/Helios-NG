
/* C compiler file cchdr.h :  Copyright (C) A.Mycroft and A.C.Norman */
/* version 0.01v */
/* $Id: cchdr.h,v 1.5 1994/08/03 09:47:10 al Exp $ */

#ifndef _CCHDR_LOADED

#define _CCHDR_LOADED 1

/* For non-ansi compilers, define some macros to get the code through... */
#ifdef COMPILING_ON_ST
#define void int
#define const
#define volatile
#endif
#ifdef COMPILING_ON_SUN
#define void int
#define const
#define volatile
#endif
#ifdef COMPILING_ON_SUN4
#define void int
#define const
#define volatile
#endif

#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include "system.h"     /* definition of what machine type etc */

#ifdef tolower
#undef tolower
extern int tolower();
#endif

#ifdef TARGET_IS_ARM
#  include "armspec.h"
#endif
#ifdef TARGET_IS_370
#  include "s370spec.h"
#endif
#ifdef TARGET_IS_CLIPPER
#  include "clipspec.h"
#endif
#ifdef TARGET_IS_XPUTER
#  include "xpuspec.h"
#endif

#include "modes.h"

/* the following disgraceful bit of code is to cheer up the type checker
   until we think how to typecheck the allocators more satisfactorily... */

#define global_list6(a,b,c,d,e,f) \
  xglobal_list6((int)(a),(void *)(b),(void *)(c),(void *)(d),(void *)(e), \
               (void *)(f))
#define global_list5(a,b,c,d,e) \
  xglobal_list5((int)(a),(void *)(b),(void *)(c),(void *)(d),(void *)(e))
#define global_list4(a,b,c,d) \
  xglobal_list4((int)(a),(void *)(b),(void *)(c),(void *)(d))
#define global_list3(a,b,c) \
  xglobal_list3((int)(a),(void *)(b),(void *)(c))
#define global_list2(a,b) xglobal_list2((int)(a),(void *)(b))
#define global_list1(a) xglobal_list1((int)(a))

#define list6(a,b,c,d,e,f) \
  xlist6((int)(a),(void *)(b),(void *)(c),(void *)(d),(void *)(e), \
               (void *)(f))
#define list5(a,b,c,d,e) \
  xlist5((int)(a),(void *)(b),(void *)(c),(void *)(d),(void *)(e))
#define list4(a,b,c,d) \
  xlist4((int)(a),(void *)(b),(void *)(c),(void *)(d))
#define list3(a,b,c) \
  xlist3((int)(a),(void *)(b),(void *)(c))
#define list2(a,b) xlist2((int)(a),(void *)(b))
#define list1(a) xlist1((int)(a))

extern void *xglobal_list1();
extern void *xglobal_list2();
extern void *xglobal_list3();
extern void *xglobal_list4();
extern void *xglobal_list5();
extern void *xglobal_list6();

#define memclr(a,n) memset(a,0,n)

extern int maxprocsize;
extern char *maxprocname;
extern int stuse_total, stuse_data, stuse_xref, stuse_xsym, stuse_sym,
    stuse_bind, stuse_type, stuse_const, stuse_pp;
extern int _debugging, suppress, feature, xrecovercount, xwarncount;
extern int warncount, recovercount, errorcount;
extern void *GlobAlloc();
extern void *BindAlloc();
extern void *SynAlloc();
#ifdef __DOS386
extern void syserr(char *s, ...);
extern void cc_fatalerr(char *s, ...);
extern void cc_warn(char *s, ...);
extern void cc_err(char *s, ...);
extern void cc_rerr(char *s, ...);
#else
extern void syserr();
extern void cc_fatalerr();
extern void cc_warn();
extern void cc_err();
extern void cc_rerr();
#endif

#define sem_warn cc_warn
#define sem_rerr cc_rerr
#define sem_err cc_err
#define KRC 0    /* set to 1 for K&R style coercions (i.e. no 'float' ops) */

extern int alignoftype();
extern Binder *global_mk_binder();
extern TagBinder *global_mk_tagbinder();
extern void suspend_local_store();
extern void drop_local_store();
extern List *discard2();
extern void *xlist1();
extern void *xlist2();
extern void *xlist3();
extern void *binder_list3();
extern void *syn_list2();
extern void *syn_list3();
extern void *syn_list5();
extern void *xlist4();
extern void *xlist5();
extern void *xlist6();
extern Binder *mk_binder();
extern TagBinder *mk_tagbinder();
extern Expr *mk_expr1();
extern Expr *mk_expr2();
extern Expr *mk_expr3();
#ifndef NO_VALOF_BLOCKS
extern Expr *mk_expr_valof();
#endif
extern Expr *mk_exprwdot();
extern Expr *mk_exprbdot();
extern Cmd *mk_cmd_0();
extern Cmd *mk_cmd_e();
extern Cmd *mk_cmd_default();
extern Cmd *mk_cmd_block();
extern Cmd *mk_cmd_do();
extern Cmd *mk_cmd_if();
extern Cmd *mk_cmd_switch();
extern Cmd *mk_cmd_for();
extern Cmd *mk_cmd_lab();
extern Cmd *mk_cmd_case();
extern TypeExpr *mk_typeexpr1();
extern void alloc_init();
extern void alloc_reinit();
extern void alloc_noteAEstoreuse();
extern void show_store_use();

extern void branch_round_literals();
extern void setlabel();
extern LabBind *label_define();
extern LabBind *label_reference();
extern LabelNumber *nextlabel();
extern block_head *start_basic_block_at_level();
extern bool is_exit_label();
extern LabelNumber *litlab;
extern block_head *top_block, *bottom_block;
extern void reopen_block();
extern void finishblock();
/* cg.c */
extern void cg_init();
extern void cg_topdecl();
extern void cg_tidy();
extern LoopList *all_loops;
extern BlockList *this_loop;
extern bool in_loop;

extern int pp_pragmavec[];
extern void pp_init();
extern int pp_nextchar();
extern void pp_predefine();
extern char *pp_cisname;
extern int pp_linect;
extern int pp_inhashif;
extern void pp_tidyup();

#define symbol_name_(s) ((sym_name_table[s]))

extern void fltrep_stod();
extern int fltrep_narrow();
extern int fltrep_narrow_round();
extern void fltrep_widen();
extern int flt_add();
extern int flt_subtract();
extern int flt_multiply();
extern int flt_divide();
extern int flt_compare();
extern int flt_move();
extern int flt_negate();
extern int flt_dtoi();
extern int flt_dtou();
extern FloatCon *real_of_string();
extern FloatCon *int_to_real();
extern FloatCon *real_to_real();

/* bind.c */
#define gentempbinder(typ) \
  mk_binder(0, gensymval(0), bitofstg_(s_auto), typ)
#define genglobinder(typ) \
  global_mk_binder(0, gensymval(1), bitofstg_(s_static), typ)
extern int bind_level;
extern void bind_init();
extern void bind_cleanup();

extern Symstr *sym_insert();
extern AEop nextsym();
extern SymInfo curlex;
extern Symstr *gensymval();
extern AEop nextsym_for_hashif();
extern void lex_beware_reinit();
extern void lex_reinit();
extern void lex_init();

#ifdef __DOS386
extern void eprintf(char *s, ...);
#else
extern void eprintf();
#endif
extern void pr_id();
extern void pr_expr();
extern void pr_typeexpr();
extern void pr_bind0();
extern void pr_stringsegs();
extern void pr_bindlist();
extern void pr_topdecl();
extern int obj_symref();
extern void obj_init();
extern void obj_header();
extern void obj_trailer();

extern TypeExpr *te_int;
extern TypeExpr *te_uint, *te_lint, *te_ulint;
extern TypeExpr *te_double;
extern TypeExpr *te_float;
extern TypeExpr *te_ldble;
extern TypeExpr *te_void;

extern Binder *datasegment, *codesegment;
extern Symstr *mainsym, *setjmpsym;
extern Symstr *libentrypoint, *stackoverflow, *stackoverflow1, *countroutine;
#ifndef NO_VALOF_BLOCKS
extern bool inside_valof_block;
#endif

typedef struct op_simulation {
   Expr *mulfn, *divfn, *udivfn, *divtestfn, *remfn, *uremfn, *memcpyfn;
   Expr *readcheck1, *readcheck2, *readcheck4,
        *writecheck1, *writecheck2, *writecheck4;
   Expr *xprintf, *xfprintf, *xsprintf;
   Symstr *yprintf, *yfprintf, *ysprintf;
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

extern int evaluate();

#undef max
#undef min
extern int max();
extern int min();
extern int bitcount();
extern int length();
extern RegList *rldiscard();
extern bool member();
extern RegList *ndelete();
extern List *dreverse();

extern Expr *globalize_int();
extern TypeExpr *globalize_typeexpr();

#define DUPL_OK            0x008
#define TOPLEVEL           0x010   /* see c.syn */

extern Expr *errornode;

extern Expr *mkcast();
extern Expr *mkfnap();
extern Expr *mkintconst();
extern void implicit_decl();
#define isvolatile_expr(e) (isvolatile_type(typeofexpr(e)))
#define ptrtotype_(t) mk_typeexpr1(t_content, t, 0)
#define primtype_(m)  (TypeExpr *)list3(s_typespec, (void *)(m), 0)
#define isprimtype_(x,s) (h0_(x) == s_typespec && \
                          (typespecmap_(x) & bitoftype_(s)))
extern bool isvolatile_type();
extern Expr *mkunary();
extern Expr *mkassign();
extern Expr *mkbinary();
extern Expr *mkfieldselector();
extern int sizeoftype();
extern TypeExpr *typeofexpr();
extern Expr *mkcond();
extern Expr *optimise0();
extern void moan_nonconst();
extern Expr *mktest();
extern bool implicit_return_ok;
extern TopDecl *rd_topdecl();
extern Expr *syn_rdinit();
extern int syn_canrdinit();
extern int syn_begin_agg();
extern void syn_end_agg();
extern int syn_undohack;
extern long int syn_hashif();
extern void syn_init();

typedef struct Unbinder {
    struct Unbinder *unbindcdr;
    Symstr *unbindsym;
    Binder *unbindold;
} Unbinder;

typedef struct Untagbinder {
    /* the field names are deliberately the same as for Unbinder */
    struct Untagbinder *unbindcdr;
    Symstr *unbindsym;
    TagBinder *unbindold;
} Untagbinder;

extern Untagbinder *push_tagenv();
extern void pop_tagenv();
extern Unbinder *push_varenv();
extern void pop_varenv();
extern void label_resolve();
extern int equivtype();
extern TagBinder *findtagbinding();
extern TagBinder *gentagbinding();
extern void settagmems();
extern TypeExpr *modify_formaltype();
extern TypeExpr *prunetype();
extern Binder *instate_declaration();
extern Expr *genstaticparts();
extern char *currentfunction;
extern TypeExpr *widen_formaltype();

#ifdef EXPERIMENTAL_DEBUGGER
extern FILE *dbgstream;
#else
#define dbgstream 0
#endif

extern BindList *active_binders;
#ifndef TARGET_IS_XPUTER
#define start_new_basic_block(l) \
    start_basic_block_at_level(l, active_binders)
#endif

extern AvailList *adconlist;
extern Binder *juststored;
extern VRegnum justregister;
extern int cautious_mcrepofexpr();
extern int cautious_mcrepoftype();
extern int mcrepofexpr();
extern FloatCon  *fc_two_31;
extern FloatCon  *fc_zero;
extern BindListList *current_env;
extern void flowgraph_reinit();
extern void linearize_code();
extern void codeseg_stringsegs();
extern void codeseg_flush();
extern void codeseg_function_name();
extern void show_entry();
extern void show_code();
extern int icode_cur, block_cur;
extern void codebuf_init();
extern void codebuf_reinit();
extern void codebuf_reinit2();
extern Binder *gentempvar();

typedef union vreg_type { VRegister *vreg; VRegnum type; } vreg_type;
extern vreg_type ((*(vregheap[REGHEAPSEGMAX]))[REGHEAPSEGSIZE]);

#define mkRegList list2

extern int immed_cmp();
extern const char *phasename;

/* regalloc.c */
extern void regalloc_init();
extern VRegnum vregister();
extern void allocate_registers();
extern int regmask;

/* loopopt.c */
extern void loopopt_reinit();
extern BindList *loop_invariants();
extern Readonly_copy *slave_list;

extern void mcdep_init();
#ifdef TARGET_IS_370
  extern int to370sex(int w, int flag);
#endif
extern void obj_codewrite();
extern void asm_header();
extern void asm_trailer();
extern void display_assembly_code();
extern void outcodeword();
extern int codeloc();
extern int lit_findadcon();
extern void dumplits2();
extern int lit_findword();
extern int codebase;
extern int litpoolp;
extern int mustlitby;
extern int (*(codeandflagvec[CODEVECSEGMAX]))[CODEVECSEGSIZE*2], codep;
#define code_inst_(q) (*codeandflagvec[(q)>>CODEVECSEGBITS+2]) \
                                      [((q)>>2)&CODEVECSEGSIZE-1]
#define code_flag_(q) (*codeandflagvec[(q)>>CODEVECSEGBITS+2]) \
                                [(((q)>>2)&CODEVECSEGSIZE-1)+CODEVECSEGSIZE]
#ifdef TARGET_HAS_HALFWORD_INSTRUCTIONS
#  define HWORD int
#  define code_hword_(q) \
     ((unsigned short *)(*codeandflagvec[(q)>>CODEVECSEGBITS+2])) \
                                        [((q)>>1)&2*CODEVECSEGSIZE-1]
#endif

/* xxxgen.c */
extern RealRegister local_base();
extern int local_address();

extern int dataloc;
extern bool has_main;

/* headers.c */
typedef struct header_files {char *name, *content;} header_files;
#ifndef NO_INSTORE_FILES
  extern header_files builtin_headers[];
#endif

extern int syserr_behaviour;
extern FILE *pp_inclopen();
extern void builtin_init();
extern void mcdep_init();
extern void initstaticvar();
extern void summarise();
extern void alloc_dispose();
extern FILE *asmstream, *objstream;
#ifdef TARGET_IS_ARM
  extern int normal_sp_sl;
#endif
extern int lsbitfirst;
extern void sem_init();

#ifdef PASCAL
extern Symstr *pasclab();
extern void syn_tidy();
#endif

#endif

/* end of cchdr.h */
