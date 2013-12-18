
/* C compiler file cchdr.h :  Copyright (C) A.Mycroft and A.C.Norman */
/* version 4d */

#ifndef _CCHDR_LOADED

#define _CCHDR_LOADED 1

#include <stdio.h>
#include "system.h"     /* definition of what machine type etc */

#if defined TARGET_IS_ARM
#  include "armspec.h"
#elif defined TARGET_IS_370
#  include "s370spec.h"
#elif defined TARGET_IS_CLIPPER
#  include "clipspec.h"
#elif defined TARGET_IS_XPUTER
#  include "xpuspec.h"
#elif defined TARGET_IS_ACW
#  include "acwspec.h"
#elif defined TARGET_IS_AMD
#  include "amdspec.h"
#elif defined TARGET_IS_68000
#  include "m68spec.h"
#elif defined TARGET_IS_68020
#  include "m68spec.h"
#elif defined TARGET_IS_NULL
#  include "nullspec.h"
#else
#  error No target machine selected - defaulting to ARM
#  include "armspec.h"
#endif

#ifndef TARGET_HAS_REGISTER_WINDOW
#  define R_P1 R_A1
#endif

/* note that for object file purposes CC_BANNERlen MUST be a multiple of 4
   and include the final null in CC_BANNER (which should preferably be
   normalised by zero padding in the last word)... */

#ifdef TARGET_HAS_DEBUGGER
#define CC_BANNER     \
  "Norcroft " TARGET_MACHINE " " LANGUAGE " (with debugger support) " \
                  CC_VERSION " " __DATE__ "\0\0\0"
#else
#define CC_BANNER     \
  "Norcroft " TARGET_MACHINE " " LANGUAGE " " CC_VERSION " " __DATE__ "\0\0\0"
#endif
#define CC_BANNERlen  (sizeof(CC_BANNER) & ~3)

#include "modes.h"

/* the following disgraceful bit of code is to cheer up the type checker
   until we think how to typecheck the allocators more satisfactorily... */

#ifdef HELIOS
#define global_list5(a,b,c,d,e) \
  xglobal_list5((int32)(a),(void *)(b),(void *)(c),(void *)(d),(void *)(e))
#endif
#define global_list4(a,b,c,d) \
  xglobal_list4((int32)(a),(void *)(b),(void *)(c),(void *)(d))
#define global_list3(a,b,c) \
  xglobal_list3((int32)(a),(void *)(b),(void *)(c))

extern void *global_cons2(void *a,void *b);
extern void *xglobal_list3(int32 a,void *b,void *c);
extern void *xglobal_list4(int32 a,void *b,void *c,void *d);
#ifdef HELIOS
extern void *xglobal_list5(int32 a,void *b,void *c,void *d,void *e);
#endif
TypeExpr *global_list6(AEop a,TypeExpr *b,FormTypeList *c,
                       int32 d,int32 e,int32 f);

#define memclr(a,n) memset(a,0,n)

extern int32 maxprocsize;
extern char *maxprocname;
extern int32 stuse_total, stuse_data, stuse_xref, stuse_xsym, stuse_sym,
    stuse_bind, stuse_type, stuse_const, stuse_pp;
extern int32 sysdebugmask, supress, feature, xrecovercount, xwarncount;
extern int32 warncount, recovercount, errorcount;
extern void *GlobAlloc(int32 n);
extern void *BindAlloc(int32 n);
extern void *SynAlloc(int32 n);

#ifdef __CC_NORCROFT
#pragma -v3  /* This pramga enables internal compiler checks on ... */
#endif
extern void syserr(char *s, ...);
extern void cc_msg(char *s, ...);
extern void cc_rerr(char *s, ...);
extern void cc_warn(char *s, ...);
extern void cc_err(char *s, ...);
extern void cc_fatalerr(char *s, ...);
#ifdef __CC_NORCROFT
#pragma -v0  /* This pramga marks the end of cc_warn-like routines */
#endif
extern void listing_diagnostics(void);
extern bool list_this_file;

extern int32 alignoftype(TypeExpr *x);
extern void drop_local_store(void);
extern List *discard2(List *p);
extern void *binder_cons2(void *a,void *b);
#define binder_icons2(a,b) binder_cons2((a), (void *)(b))
extern void *syn_list3(int32 a,int32 b,int32 c);
extern void *binder_list3(void *a,void *b,int32 c);
extern void *syn_cons2(void *a,void *b);
extern void *syn_list2(int32 a,void *b);
extern TopDecl *mkTopDeclFnDef(AEop a,Binder *b,SynBindList *c,Cmd *d,bool e);
extern DeclRhsList *mkDeclRhsList(DeclRhsList *a, Symstr *b, TypeExpr *c,
                           Expr *d, SET_BITMAP e, Binder *f);
extern TypeExpr *mkTypeExprfn(AEop a,TypeExpr *b,DeclRhsList *c,
                       int32 d,int32 e,int32 f);
extern Expr *mk_expr1(AEop op, TypeExpr *t, Expr *a1);
extern Expr *mk_expr2(AEop op, TypeExpr *t, Expr *a1, Expr *a2);
extern Expr *mk_exprlet(AEop op, TypeExpr *t, SynBindList *a1, Expr *a2);
extern Expr *mk_expr3(AEop op, TypeExpr *t, Expr *a1, Expr *a2, Expr *a3);
#ifndef NO_VALOF_BLOCKS
extern Expr *mk_expr_valof(AEop op, TypeExpr *t, Cmd *c);
#endif
extern Expr *mk_exprwdot(AEop op, TypeExpr *t, Expr *a1, int32 a2);
extern Expr *mk_exprbdot(AEop op, TypeExpr *t, Expr *a1, int32 a2, int32 a3, int32 a4);
extern Cmd *mk_cmd_0(AEop op, FileLine x);
extern Cmd *mk_cmd_e(AEop op, FileLine x, Expr *e);
extern Cmd *mk_cmd_default(FileLine x, Cmd *c);
extern Cmd *mk_cmd_block(FileLine x, SynBindList *bl, CmdList *cl);
extern Cmd *mk_cmd_do(FileLine x, Cmd *c, Expr *e);
extern Cmd *mk_cmd_if(FileLine x, Expr *e, Cmd *c1, Cmd *c2);
extern Cmd *mk_cmd_switch(FileLine x, Expr *e, Cmd *c1, Cmd *c2, Cmd *c3);
extern Cmd *mk_cmd_for(FileLine x, Expr *e1, Expr *e2, Expr *e3, Cmd *c);
extern Cmd *mk_cmd_lab(AEop op, FileLine x, LabBind *b, Cmd *c);
extern Cmd *mk_cmd_case(FileLine x, Expr *e, Cmd *c1, Cmd *c2);
extern TypeExpr *mk_typeexpr1(AEop op, TypeExpr *t, Expr *a1);
extern void alloc_init(void);
extern void alloc_reinit(void);
extern void alloc_noteAEstoreuse(void);
extern void show_store_use(void);

extern void branch_round_literals(LabelNumber *m);
extern void setlabel(LabelNumber *);
extern LabBind *label_define(Symstr *id);
extern LabBind *label_reference(Symstr *id);
extern LabelNumber *nextlabel(void);
extern block_head *start_basic_block_at_level(LabelNumber *l,
                                       BindList *active_on_entry);
extern bool is_exit_label(LabelNumber *ll);
extern LabelNumber *litlab;
extern block_head *top_block, *bottom_block;
extern void reopen_block(block_head *p);
extern void finishblock(void);
/* cg.c */
extern void cg_init(void);
extern void cg_topdecl(TopDecl *x);
extern void cg_tidy(void);
extern BlockList *this_loop;
extern bool in_loop;

extern int32 pp_pragmavec[];
extern void pp_init(void);
extern int pp_nextchar(void);
extern void pp_predefine(char *);
extern char *pp_cisname;
extern int32 pp_linect;
extern bool pp_inhashif;
extern void pp_tidyup(void);
#ifndef NO_LISTING_OUTPUT
extern bool map_init(FILE *map_file);
#endif
extern void pp_notesource(char *filename);
extern FILE *listing_file;

extern void fltrep_stod(char *s, DbleBin *p);
extern bool fltrep_narrow(DbleBin *d, FloatBin *e);
extern bool fltrep_narrow_round(DbleBin *d, FloatBin *e);
extern void fltrep_widen(FloatBin *e, DbleBin *d);
extern bool flt_add(DbleBin *a, DbleBin *b, DbleBin *c);
extern bool flt_subtract(DbleBin *a, DbleBin *b, DbleBin *c);
extern bool flt_multiply(DbleBin *a, DbleBin *b, DbleBin *c);
extern bool flt_divide(DbleBin *a, DbleBin *b, DbleBin *c);
extern int flt_compare(DbleBin *b, DbleBin *c);
extern bool flt_move(DbleBin *a, DbleBin *b);
extern bool flt_negate(DbleBin *a, DbleBin *b);
extern bool flt_dtoi(int32 *n, DbleBin *a);
extern bool flt_dtou(unsigned32 *u, DbleBin *a);
extern FloatCon *real_of_string(char *s, int32 flag);
extern FloatCon *int_to_real(int32 n, int32 u, SET_BITMAP m);
extern FloatCon *real_to_real(FloatCon *fc, SET_BITMAP m);

/* bind.c */
extern Binder *global_mk_binder(Binder *b,Symstr *c,SET_BITMAP d,TypeExpr *e);
extern Binder *mk_binder(Symstr *c,SET_BITMAP d,TypeExpr *e);
#define gentempbinder(typ) \
  mk_binder(gensymval(0), bitofstg_(s_auto), typ)
#define genglobinder(typ) \
  global_mk_binder(0, gensymval(1), bitofstg_(s_static), typ)
extern int32 bind_level;
extern void bind_init(void);
extern void bind_cleanup(void);

extern Symstr *sym_insert(char *name, AEop type);
extern AEop nextsym(void);
extern SymInfo curlex;
extern Symstr *gensymval(bool glo);
extern AEop nextsym_for_hashif(void);
extern void lex_beware_reinit(void);
extern void lex_check_extern(Symstr *s);
extern void lex_reinit(void);
extern void lex_init(void);

#ifdef __CC_NORCROFT
#  pragma -v1    /* turn on printf checking on eprintf */
#endif
extern void eprintf(char *s, ...);
#ifdef __CC_NORCROFT
#  pragma -v0
#endif
extern void pr_expr(Expr *x);
extern void pr_typeexpr(TypeExpr *x, Symstr *s);
extern void pr_stringsegs(StringSegList *z);
extern void pr_topdecl(TopDecl *x);
extern void obj_init(void);
extern void obj_header(void);
extern void obj_trailer(void);

extern TypeExpr *te_int;
extern TypeExpr *te_uint, *te_lint, *te_ulint;
extern TypeExpr *te_double;
extern TypeExpr *te_float;
extern TypeExpr *te_ldble;
extern TypeExpr *te_void;

extern Binder *datasegment, *codesegment;
extern Symstr *mainsym, *setjmpsym;
extern Symstr *libentrypoint, *stackoverflow, *stack1overflow,
              *countroutine, *count1routine;
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

extern int32 evaluate(Expr *);

extern int32 max(int32 a, int32 b);
extern int32 bitcount(int32 n);
extern int32 length(List *l);
extern RegList *rldiscard(RegList *x);
extern bool member(VRegnum a, RegList *l);
extern RegList *ndelete(VRegnum a, RegList *ll);
extern List *dreverse(List *x), *nconc(List *x, List *y);

extern Expr *globalize_int(int32 n);
extern TypeExpr *globalize_typeexpr(TypeExpr *t);

/* The next two lines interface syn.c and bind.c: see syn.c for more such */
#define DUPL_OK            0x001
#define TOPLEVEL           0x002

extern Expr *errornode;

extern Expr *mkcast(AEop op, Expr *e, TypeExpr *tr);
extern Expr *mkfnap(Expr *e, ExprList *l);
extern Expr *mkintconst(TypeExpr *te, int32 n, Expr *e);
extern void implicit_decl(Symstr *a, int32 fn);
#define isvolatile_expr(e) (isvolatile_type(typeofexpr(e)))
#define ptrtotype_(t) mk_typeexpr1(t_content, t, 0)
#define primtype_(m)  mk_typeexpr1(s_typespec, (void *)(m), 0)
#define isprimtype_(x,s) (h0_(x) == s_typespec && \
                          (typespecmap_(x) & bitoftype_(s)))
extern bool isvolatile_type(TypeExpr *x);
extern Expr *mkunary(AEop op, Expr *a);
extern Expr *mkassign(AEop op, Expr *a, Expr *b);
extern Expr *mkbinary(AEop op, Expr *a, Expr *b);
extern Expr *mkfieldselector(AEop op, Expr *e, Symstr *sv);
extern int32 sizeoftype(TypeExpr *x);
extern TypeExpr *typeofexpr(Expr *x);
extern Expr *mkcond(Expr *a, Expr *b, Expr *c);
extern Expr *optimise0(Expr *e);
extern void moan_nonconst(Expr *e, char *s);
extern Expr *mktest(AEop opreason, Expr *a);
extern Expr *mkswitch(Expr *a);
extern bool implicit_return_ok;
extern TopDecl *rd_topdecl(void);
extern Expr *syn_rdinit(TypeExpr *t, int32 flag);
extern bool syn_canrdinit(void);
extern int32 syn_begin_agg(void);
extern void syn_end_agg(int32);
extern int32 syn_undohack;
extern bool syn_hashif(void);
extern void syn_init(void);

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

extern Untagbinder *push_tagenv(void);
extern void pop_tagenv(Untagbinder *old);
extern Unbinder *push_varenv(void);
extern void pop_varenv(Unbinder *old);
extern void label_resolve(void);
extern bool equivtype(TypeExpr *t1, TypeExpr *t2);
extern TagBinder *findtagbinding(Symstr *sv, AEop s, bool defining);
extern TagBinder *gentagbinding(AEop s);
extern void settagmems(TagBinder *b, TagMemList *l);
extern TypeExpr *modify_formaltype(TypeExpr *t);
extern TypeExpr *prunetype(TypeExpr *t);
extern Binder *instate_declaration(DeclRhsList *d, int declflag);
extern Expr *genstaticparts(DeclRhsList *d, bool topflag, FileLine fl);
extern void vargen_init(void);
extern char *currentfunction;
extern TypeExpr *widen_formaltype(TypeExpr *t);

#ifdef TARGET_HAS_DEBUGGER
  extern int usrdbgmask;
#  define usrdbg(DBG_WHAT) (usrdbgmask & (DBG_WHAT))
#  define DBG_LINE 1         /* line info -- reduces peepholing     */
#  define DBG_PROC 2         /* top level info -- no change to code */
#  define DBG_VAR  4         /* local var info -- no leaf procs     */
#  define DBG_ANY  (DBG_LINE|DBG_PROC|DBG_VAR)
  extern int32 dbgloc;
  extern void *dbg_notefileline(FileLine fl);
  extern void dbg_addcodep(void *debaddr, int32 codeaddr);
  extern bool dbg_scope(BindListList *, BindListList *);
  extern void dbg_topvar(Symstr *name, int32 addr, TypeExpr *t, bool ext,
                         FileLine fl);
  extern void dbg_type(Symstr *name, TypeExpr *t);
  extern void dbg_proc(Symstr *name, TypeExpr *t, FileLine fl);
  extern void dbg_locvar(Binder *name, FileLine fl);
  extern void dbg_enterproc(void);
  extern void dbg_bodyproc(void);
  extern void dbg_return(int32 addr);
  extern void dbg_endproc(FileLine fl);
  extern void dbg_writedebug(void);
  extern void dbg_init(void);
  extern void obj_writedebug(void *, size_t);
  extern int32 local_fpaddress(int32);
#else
#  define usrdbg(DBG_WHAT) 0
#  define dbgloc 0
#  define dbg_init() 0
#  define dbg_scope(a,b) 0
#  define dbg_addcodep(debaddr,codeaddr) 0
#  define dbg_notefileline(a) 0
#  define dbg_enterproc() 0
#  define dbg_bodyproc() 0
#  define dbg_return(a) 0
#  define dbg_endproc(a) 0
#  define dbg_type(a,b) 0
#  define dbg_proc(a,b,c) 0
#  define dbg_topvar(a,b,c,d,e) 0
#  define dbg_locvar(a,b) 0
#endif

extern BindList *active_binders;
#define start_new_basic_block(l) \
    start_basic_block_at_level(l, active_binders)

extern AvailList *adconlist;
extern Binder *juststored;
extern VRegnum justregister;
extern int32 cautious_mcrepofexpr(Expr *e);
extern int32 cautious_mcrepoftype(TypeExpr *t);
extern int32 mcrepofexpr(Expr *e);
extern FloatCon  *fc_two_31;
extern FloatCon  *fc_zero;
extern BindListList *current_env;
extern void flowgraph_reinit(void);
extern void linearize_code(void);
extern void codeseg_stringsegs(StringSegList *x, bool incode);
extern void codeseg_flush(void);
extern void codeseg_function_name(Symstr *name);
extern void show_entry(Symstr *name, int flags);
extern void show_code(Symstr *name);
extern int32 icode_cur, block_cur;
extern void codebuf_init(void);
extern void codebuf_reinit(void);
extern void codebuf_reinit2(void);
extern Binder *gentempvar(TypeExpr *t, VRegnum r);

typedef union vreg_type { VRegister *vreg; VRegnum type; } vreg_type;
extern vreg_type ((*(vregheap[REGHEAPSEGMAX]))[REGHEAPSEGSIZE]);

extern bool immed_cmp(int32 n);
extern const char *phasename;

/* regalloc.c */
extern void regalloc_init(void), regalloc_reinit(void), regalloc_tidy(void);
extern VRegnum vregister(int32 type);
extern void allocate_registers(BindList *spill_order);
extern int32 regmask;

/* loopopt.c */
extern void loopopt_reinit(void);
extern BindList *loop_invariants(void);
extern Readonly_copy *slave_list;
extern void note_loop(BlockList *b, block_head *c);

extern void mcdep_init(void);
#ifdef TARGET_IS_370
  extern int32 to370sex(int32 w, int32 flag);
#endif
extern void obj_codewrite(void);
#ifndef NO_ASSEMBLER_OUTPUT
extern void asm_header(void);
extern void asm_trailer(void);
extern void display_assembly_code(Symstr *);
#endif
extern void outcodeword(int32 w, int32 f);
extern int32 codeloc(void);
extern int32 lit_findadcon(Symstr *name, int32 offset, int32 wherefrom);
extern void dumplits2(int32 needsjump);
extern int lit_of_count_name(char *s);
extern void dump_count_names(void);
extern int32 lit_findword(int32 w, int32 flavour, Symstr *sym, int32 flag);
extern int32 codebase;
extern int32 litpoolp;
extern int32 mustlitby;
extern int32 (*(codeandflagvec[CODEVECSEGMAX]))[CODEVECSEGSIZE*2], codep;
#define code_inst_(q) (*codeandflagvec[(q)>>CODEVECSEGBITS+2]) \
                                      [((q)>>2)&CODEVECSEGSIZE-1]
#define code_flag_(q) (*codeandflagvec[(q)>>CODEVECSEGBITS+2]) \
                                [(((q)>>2)&CODEVECSEGSIZE-1)+CODEVECSEGSIZE]
#ifdef TARGET_HAS_HALFWORD_INSTRUCTIONS
#  define HWORD int32
#  define code_hword_(q) \
     ((unsigned16 *)(*codeandflagvec[(q)>>CODEVECSEGBITS+2])) \
                                        [((q)>>1)&2*CODEVECSEGSIZE-1]
#endif

/* xxxgen.c */
extern RealRegister local_base(Binder *b);
extern int32 local_address(Binder *b);

extern int32 dataloc;
extern bool has_main;

/* headers.c */
typedef struct header_files {char *name, *content;} header_files;
#ifndef NO_INSTORE_FILES
  extern header_files builtin_headers[];
#endif

extern int32 syserr_behaviour;
extern FILE *pp_inclopen(char *, bool);
extern void builtin_init(void);
extern void mcdep_init(void);
extern void initstaticvar(Binder *b, bool topflag);
extern void summarise(void);
extern void alloc_dispose(void);
extern FILE *asmstream, *objstream;
extern char *objfilename;
#ifdef GLOBAL_SOURCE_NAME
extern char *sourcefile;
#endif
#ifdef UNIQUE_DATASEG_NAMES
extern int main_compilation_count;
#endif
#ifdef TARGET_IS_ARM
  extern bool normal_sp_sl;
#endif
#ifdef TARGET_IS_370
  extern char *csectname;
#endif
extern bool lsbitfirst;
extern void sem_init(void);

#ifdef PASCAL
extern Symstr *pasclab(int32 n);
extern void syn_tidy(void);
#endif

#endif

/* end of cchdr.h */
