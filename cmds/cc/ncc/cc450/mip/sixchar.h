#pragma force_top_level
#pragma include_only_once
/*
 * C compiler file sixchar.h, version 3
 * Copyright (C) Codemist Ltd., 1989
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/07/14 14:07:18 $
 * Revising $Author: nickc $
 */

/* The definitions that follow are to map long and meaningful (?) names  */
/* onto ones that are distinct in the first 6 characters (monocase) so   */
/* that machines with truly archaic linkers can still cope...            */
/* It is not necessary that the names just have six chars.               */

#ifndef _sixchar_h
#define _sixchar_h 1

/*
 * To rebuild this list and avoid unnecessary entries:
 * Build a compiler with the following extra stuff in "options.h",
 * or with "target.h" adjusted if a linker limit is already in force:
 *
 * #define'ing POLICE_THE_SIX_CHAR_NAME_LIMIT will cause mip/defaults.h to
 * arrange this for you.
 *
 * #define TARGET_HAS_LINKER_NAME_LIMIT 1
 * #  define LINKER_NAME_MAX 6
 * #  define LINKER_NAME_MONOCASE 1
 * #include "sixchar.h"
 *
 * Alter "sixchar.h" to remove any definitions not known to be needed,
 * indeed comment it ALL out as a start.
 * Then compile the entire compiler, collecting all the error messages
 * that come out moaning about linker name clashes.  Consolidate and sort
 * same to produce definitions as seen below, and make into the new
 * "sixchar.h".
 * The names documented here relate to the ARM and the s370 targetted
 * versions of the compiler - extra things may need including to cope
 * with long names in "asm.c" and "gen.c".
 */

#define alloc_dispose              G001_alloc_dispose
#define alloc_init                 G002_alloc_init
#define alloc_noteaestoreuse       G003_alloc_noteaestoreuse
#define alloc_reinit               G004_alloc_reinit
#define alloc_unmark               G005_alloc_unmark
#define builtin_init               G006_builtin_init
#define cautious_mcrepofexpr       G007_cautious_mcrepofexpr
#define cautious_mcrepoftype       G008_cautious_mcrepoftype
#define cc_err                     G009_cc_err
#define cc_err_l                   G010_cc_err_l
#define cc_fatalerr                G011_cc_fatalerr
#define cc_fatalerr_l              G012_cc_fatalerr_l
#define cc_rerr                    G013_cc_rerr
#define cc_rerr_l                  G014_cc_rerr_l
#define cc_warn                    G015_cc_warn
#define cc_warn_l                  G016_cc_warn_l
#define codebuf_init               G017_codebuf_init
#define codebuf_reinit             G018_codebuf_reinit
#define codebuf_reinit2            G019_codebuf_reinit2
#define codeseg_flush              G020_codeseg_flush
#define codeseg_function_name      G021_codeseg_function_name
#define codeseg_stringsegs         G022_codeseg_stringsegs
#define codesegment                G023_codesegment
#define compile_abort              G024_compile_abort
#define config                     G025_config
#define config_init                G026_config_init
#define current_env                G027_current_env
#define currentfunction            G028_currentfunction
#define discard2                   G031_discard2
#define discard3                   G032_discard3
#define driver_abort               G033_driver_abort
#define emitfl                     G034_emitfl
#define emitfloat                  G035_emitfloat
#define emitsetsp                  G036_emitsetsp
#define emitsetspandjump           G037_emitsetspandjump
#define fc_two                     G038_fc_two
#define fc_two_31                  G039_fc_two_31
#define flowgraph_reinit           G040_flowgraph_reinit
#define flt_dtoi                   G041_flt_dtoi
#define flt_dtou                   G042_flt_dtou
#define fltrep_narrow              G043_fltrep_narrow
#define fltrep_narrow_round        G044_fltrep_narrow_round
#define fltrep_stod                G045_fltrep_stod
#define fltrep_widen               G046_fltrep_widen
#define fname_parse                G047_fname_parse
#define fname_set_try_order        G048_fname_set_try_order
#define fname_unparse              G049_fname_unparse
#define global_mk_binder           G050_global_mk_binder
#define global_mk_tagbinder        G051_global_mk_tagbinder
#define globalize_int              G052_globalize_int
#define globalize_typeexpr         G053_globalize_typeexpr
#define globalloc                  G054_globalloc
#define implicit_decl              G055_implicit_decl
#define implicit_return_ok         G056_implicit_return_ok
#define jopprint_opname            G057_jopprint_opname
#define label_define               G058_label_define
#define label_reference            G059_label_reference
#define label_resolve              G060_label_resolve
#define lit_findadcon              G063_lit_findadcon
#define lit_findstringincurpool    G064_lit_findstringincurpool
#define lit_findstringinprevpools  G065_lit_findstringinprevpools
#define lit_findword               G066_lit_findword
#define lit_findwordsincurpool     G067_lit_findwordsincurpool
#define lit_findwordsinprevpools   G068_lit_findwordsinprevpools
#define local_address              G069_local_address
#define local_base                 G070_local_base
#define loopopt_reinit             G071_loopopt_reinit
#define mcdep_config_option        G072_mcdep_config_option
#define mcdep_init                 G073_mcdep_init
#define mk_cmd_0                   G074_mk_cmd_0
#define mk_cmd_block               G075_mk_cmd_block
#define mk_cmd_case                G076_mk_cmd_case
#define mk_cmd_default             G077_mk_cmd_default
#define mk_cmd_do                  G078_mk_cmd_do
#define mk_cmd_e                   G079_mk_cmd_e
#define mk_cmd_for                 G080_mk_cmd_for
#define mk_cmd_if                  G081_mk_cmd_if
#define mk_cmd_lab                 G082_mk_cmd_lab
#define mk_cmd_switch              G083_mk_cmd_switch
#define mk_expr1                   G084_mk_expr1
#define mk_expr2                   G085_mk_expr2
#define mk_expr3                   G086_mk_expr3
#define mk_exprbdot                G087_mk_exprbdot
#define mk_exprlet                 G088_mk_exprlet
#define mk_exprwdot                G089_mk_exprwdot
#define nextsym                    G090_nextsym
#define nextsym_for_hashif         G091_nextsym_for_hashif
#define obj_codewrite              G092_obj_codewrite
#define obj_common_end             G093_obj_common_end
#define obj_common_start           G094_obj_common_start
#define obj_symlist                G095_obj_symlist
#define obj_symref                 G096_obj_symref
#define pp_cis                     G097_pp_cis
#define pp_cisname                 G098_pp_cisname
#define pp_inclclose               G099_pp_inclclose
#define pp_inclopen                G100_pp_inclopen
#define pp_predefine               G101_pp_predefine
#define pp_preundefine             G102_pp_preundefine
#define print_jopcode              G103_print_jopcode
#define print_xjopcode             G104_print_xjopcode
#define regalloc_init              G105_regalloc_init
#define regalloc_reinit            G106_regalloc_reinit
#define regalloc_tidy              G107_regalloc_tidy
#define relation_add               G108_relation_add
#define relation_delete            G109_relation_delete
#define relation_init              G110_relation_init
#define relation_map               G111_relation_map
#define relation_mapanddelete      G112_relation_mapanddelete
#define relation_member            G113_relation_member
#define show_code                  G114_show_code
#define show_code_summary          G115_show_code_summary
#define sym_insert                 G116_sym_insert
#define sym_insert_id              G117_sym_insert_id
#define syserr                     G118_syserr
#define syserr_behaviour           G119_syserr_behaviour
#define vregset_compare            G120_vregset_compare
#define vregset_copy               G121_vregset_copy
#define vregset_delete             G122_vregset_delete
#define vregset_difference         G123_vregset_difference
#define vregset_discard            G124_vregset_discard
#define vregset_init               G125_vregset_init
#define vregset_insert             G126_vregset_insert
#define vregset_intersection       G127_vregset_intersection
#define vregset_map                G128_vregset_map
#define vregset_member             G129_vregset_member
#define vregset_union              G130_vregset_union
#define xbinder_list2              G131_xbinder_list2
#define xbinder_list3              G132_xbinder_list3
#define xglobal_cons2              G133_xglobal_cons2
#define xglobal_list3              G134_xglobal_list3
#define xglobal_list4              G135_xglobal_list4
#define xglobal_list6              G136_xglobal_list6
#define xsyn_list2                 G137_xsyn_list2
#define xsyn_list3                 G138_xsyn_list3
#define xsyn_list4                 G139_xsyn_list4
#define xsyn_list5                 G140_xsyn_list5
#define xsyn_list6                 G141_xsyn_list6
#define xsyn_list7                 G142_xsyn_list7

#define outcodewordaux             G144_outcodewordaux
#define cc_ansi_warn               G145_cc_ansi_warn
#define cse_print_node             G146_cse_print_node
#define cse_print_loc              G147_cse_print_loc
#define cse_printexits             G148_cse_printexits

#define localcg_tidy               G149_localcg_tidy
#define localcg_reinit             G150_localcg_reinit
#define instate_declaration        G151_instate_declaration
#define instate_alias              G152_instate_alias
#define issimplelvalue             G153_issimplelvalue
#define issimplevalue              G154_issimplevalue
#define emitcallreg                G155_emitcallreg
#define emitsetspgoto              G156_emitsetspgoto
#define codebuf_tidy               G157_codebuf_tidy

#endif

/* end of sixchar.h */
