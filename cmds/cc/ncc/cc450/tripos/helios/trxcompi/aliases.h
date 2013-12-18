/* misc.c */
#define mk_cmd_0       L0
#define mk_cmd_e       L1
#define mk_cmd_default L2
#define mk_cmd_lab     L3
#define mk_cmd_block   L4
#define mk_cmd_do      L5
#define mk_cmd_if      L6
#define mk_cmd_switch  L7
#define mk_cmd_for     L8
#define mk_cmd_case    L9
#define mk_expr1       L10
#define mk_expr2       L11
#define mk_exprlet     L12
#define mk_expr3       L13
#define mk_expr_valof  L14
#define mk_exprwdot    L15
#define mk_exprbdot    L16
#define syn_list2        L17
#define syn_list3        L18
#define binder_list3     L19
#define binder_cons2     L20
#define xglobal_list3    L21
#define xglobal_list4    L22
#define global_cons2     L23
#define global_list6     L24
#define stuse_xref       L25
#define stuse_xsym       L26
#define stuse_total      L27
#define stuse_type       L28

/* pp.c */
#define listing_file        L29
#define listing_diagnostics L30

/* ieeeflt.c */
#define flt_dtoi            L31
#define flt_dtou            L32
#define fltrep_stod         L33
#define fltrep_narrow       L34
#define fltrep_narrow_round L35
#define fltrep_widen        L36

/* lex.c */
#define nextsym            L37
#define nextsym_for_hashif L38

/* bind.c */
#define label_resolve       L39
#define label_reference     L40
#define globalize_int       L41
#define globalize_typeexpr  L42
#define global_mk_binder    L43
#define global_mk_tagbinder L44

/* simplify.c */
#define cautious_mcrepofexpr L45
#define cautious_mcrepoftype L46

/* syn.c */
#define implicit_return_ok L47
#define implicit_decl      L48

/* codebuf.c */
#define codebuf_reinit        L49
#define codebuf_reinit2       L50
#define codebuf_init          L51
#define codeseg_stringsegs    L52
#define codeseg_flush         L53
#define codeseg_function_name L54
#define lit_findadcon         L55
#define lit_findword          L56
#define maxprocsize           L57
#define maxprocname           L58

/* flowgraf.c */
#define current_env  L59
#define current_env2 L60

/* regalloc.c */
#define regalloc_reinit L61
#define regalloc_init   L62
#define regalloc_tidy   L63

/* m68gen.c */
#define compare_integer       L64
#define compare_register      L65
#define routine_entry         L66
#define routine_exit          L67
#define condition_mask        L68
#define conditional_branch_to L69

/* triposobj.c */
#define dumpdataxrefs      L70
#define dumpdatahunkrelocs L71
#define dumpdata           L72
#define obj_symcount       L73
#define obj_symlist        L74
#define obj_symref         L75

/* decins.c */
#define register_list  L76
#define write_op       L77
#define write_opi      L78
#define condition_code L79

/* builtin.c */
#define builtin_init   L80

/* m68gen.c */
#define load_string L81
#define load_static_data_ptr L82
