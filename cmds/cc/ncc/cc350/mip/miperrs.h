/*
 * C compiler error prototype file (miperrs.h)
 * Copyright (C) Codemist Ltd, 1988.
 */

/*
 * RCS $Revision: 1.2 $ Codemist 131
 * Checkin $Date: 1993/01/12 10:47:40 $
 * Revising $Author: nickc $
 */

/*
 * This file is input to the genhdrs utility, which can compress error
 * strings (but leaving escape sequences alone so that format checking can
 * occur) and optionally mapping syserr messages onto numeric codes
 * (in case somebody wants to save the about 4Kbytes of memory involved).
 */

/* AM: (after discussion with LDS) It would seem that error texts below */
/* which (seriously) take 2 or more arguments should be of the form     */
/*    #define ermsg(a,b,c) "ho hum %s had a %s %s", a, b, c             */
/* etc. to allow different sentence order in other (natural) languages. */

/* One nice thing would be to have a variant form of $r (etc) which did */
/* not quote its arg to avoid many uses of symname_() in the code.      */

#ifdef __CC_NORCROFT
  /*
   * The next procedure takes a string as a format... check args.
   */
  #pragma -v3
#endif

/* cc_msg has been left in globals.h since it takes an uncompressed string */
extern void cc_rerr(char *errcode, ...);
extern void cc_ansi_rerr(char *errcode, ...);
extern void cc_warn(char *errcode, ...);
extern void cc_ansi_warn(char *errcode, ...);
extern void cc_pccwarn(char *errcode, ...);
extern void cc_err(char *errcode, ...);
extern void cc_fatalerr(char *errcode, ...);

#ifdef __CC_NORCROFT
  /*
   * End of procedures that take error strings or codes.
   */
  #pragma -v0
#endif


%O      /* Map strings to offsets in compressed string table */

#define bind_warn_extern_clash			"extern clash $r, $r clash (ANSI 6 char monocase)"
#define bind_warn_unused_static_decl		"unused earlier static declaration of $r"
#define bind_warn_not_in_hdr			"extern $r not declared in header"
#define bind_warn_main_not_int			"extern 'main' needs to be 'int' function"
#define bind_warn_label_not_used		"label $r was defined but not used"
/*
 * Note that when part of an error string MUST be stored as a regular
 * non-compressed string I have to inform the GenHdrs utility with %Z and %O
 * This arises when a string contains literal strings as extra sub-args.
 */
#define bind_warn_not_used(is_typedef,is_fn,binder) \
        "%s $b declared but not used", \
%Z      ((is_typedef) ? "typedef" : ((is_fn) ? "function" : "variable")),\
%O      binder
#define bind_warn_static_not_used 		"static $b declared but not used"
#define cg_warn_implicit_return			"implicit return in non-void %s()"
#define flowgraf_warn_implicit_return		"implicit return in non-void function"
#define pp_warn_triglyph			"ANSI '%c%c%c' trigraph for '%c' found - was this intended?"
#define pp_warn_nested_comment			"character sequence %s inside comment"
#define pp_warn_many_arglines			"(possible error): >= %lu lines of macro arguments"
#define pp_warn_redefinition			"repeated definition of #define macro %s"
#define pp_rerr_redefinition			"differing redefinition of #define macro %s"
#define pp_rerr_nonunique_formal		"duplicate macro formal parameter: '%s'"
#define pp_rerr_define_hash_arg			"operand of # not macro formal parameter"
#define pp_rerr_define_hashhash			"## first or last token in #define body"
#define pp_warn_ifvaldef			"#ifdef %s may indicate trouble..." /* MACH_EXTNS */
#define pp_warn_nonansi_header			"Non-ANSI #include <%s>"
#define pp_warn_bad_pragma			"Unrecognised #pragma (no '-' or unknown word)"
#define pp_warn_bad_pragma1			"Unrecognised #pragma -%c"
#define pp_warn_unused_macro			"#define macro '%s' defined but not used"
#define regalloc_warn_use_before_set		"$b may be used before being set"
#define regalloc_warn_never_used		"$b is set but never used"
#define sem_warn_unsigned			"ANSI surprise: 'long' $s 'unsigned' yields 'long'"
#define sem_warn_format_type			"actual type $t mismatches format '%.*s'"
#define sem_warn_bad_format			"Illegal format conversion '%%%c'"
#define sem_warn_incomplete_format		"Incomplete format string"
#define sem_warn_format_nargs			"Format requires %ld parameter%s, but %ld given"
#define sem_warn_addr_array			"'&' unnecessary for function or array $e"
#define sem_warn_bad_shift( m, n )		"shift of $m by %ld undefined in ANSI C",m,n
#define sem_warn_divrem_0			"division by zero: $s"
#define sem_warn_ucomp_0			"odd unsigned comparison with 0: $s"
#define sem_warn_fp_overflow( op )		"floating point constant overflow: $s",op
#define sem_rerr_udiad_overflow(op,a,b,c)	"unsigned constant overflow: $s",op
#define sem_rerr_diad_overflow(op,a,b,c)	"signed constant overflow: $s",op
#define sem_rerr_umonad_overflow(op,a,b)	"unsigned constant overflow: $s",op
#define sem_rerr_monad_overflow(op,a,b)		"signed constant overflow: $s",op
#define sem_rerr_implicit_cast_overflow(t,a,b)	"implicit cast (to $t) overflow",t
#define sem_warn_fix_fail			"floating to integral conversion failed"
#define sem_warn_index_ovfl			"out-of-bound offset %ld in address"
#define sem_warn_low_precision			"lower precision in wider context: $s"
#define sem_warn_odd_condition			"use of $s in condition context"
#define sem_warn_void_context			"no side effect in void context: $s"
#define sem_warn_olde_mismatch			"argument and old-style parameter mismatch: $e"
#define sem_warn_uncheckable_format \
        "'format' arg. to printf/scanf etc. is variable, so cannot be checked"
#define sem_warn_narrowing			"implicit narrowing cast: $s"
#define sem_warn_fn_cast			"$s: cast between function pointer and non-function object"
#define sem_warn_pointer_int			"explicit cast of pointer to 'int'"
#define bind_err_extern_clash			"extern clash $r, $r (linker %ld char%s)"
#define bind_err_duplicate_tag			"duplicate definition of $s tag $b"
#define bind_err_reuse_tag			"re-using $s tag $b as $s tag"
#define bind_err_incomplete_tentative		"incomplete tentative declaration of $r"
#define bind_err_type_disagreement		"type disagreement for $r"
#define bind_err_duplicate_definition		"duplicate definition of $r"
#define bind_err_duplicate_label		"duplicate definition of label $r - ignored"
#define bind_err_unset_label			"label $r has not been set"
#define bind_err_undefined_static		"static function $b not defined - treated as extern"
#define fp_err_very_big				"Overlarge floating point value found"
#define fp_err_big_single			"Overlarge (single precision) floating point value found"
#define pp_err_eof_comment			"EOF in comment"
#define pp_err_eof_string			"EOF in string"
#define pp_err_eol_string			"quote (%c) inserted before newline"
#define pp_err_eof_escape			"EOF in string escape"
#define pp_err_missing_quote			"Missing '%c' in pre-processor command line"
#define pp_err_if_defined			"No identifier after #if defined"
#define pp_err_if_defined1			"No ')' after #if defined(..."
#define pp_err_rpar_eof				"Missing ')' after %s(... on line %ld"
#define pp_err_many_args			"Too many arguments to macro %s(... on line %ld"
#define pp_err_few_args				"Too few arguments to macro %s(... on line %ld"
#define pp_err_missing_identifier		"Missing identifier after #define"
#define pp_err_missing_parameter		"Missing parameter name in #define %s(..."
#define pp_err_missing_comma			"Missing ',' or ')' after #define %s(..."
#define pp_err_undef				"Missing identifier after #undef"
#define pp_err_ifdef				"Missing identifier after #ifdef"
#define pp_err_include_quote			"Missing '<' or '\"' after #include"
#define pp_err_include_junk			"Junk after #include %c%s%c"
#define pp_err_include_file			"#include file %c%s%c wouldn't open"
#define pp_err_unknown_directive		"Unknown directive: #%s"
#define pp_err_endif_eof			"Missing #endif at EOF"
#define sem_err_typeclash			"Illegal types for operands: $s"
#define sem_err_sizeof_struct			"size of $c needed but not yet defined"
#define sem_err_lvalue				"Illegal in lvalue: function or array $e"
#define sem_err_bitfield_address		"bit fields do not have addresses"
#define sem_err_lvalue1				"Illegal in l-value: 'enum' constant $b"
#define sem_err_lvalue2				"Illegal in the context of an l-value: $s"
#define sem_err_nonconst			"illegal in %s: <unknown>"
#define sem_err_nonconst1			"illegal in %s: non constant $b"
#define sem_err_nonconst2			"illegal in %s: $s"
#define sem_err_nonfunction			"attempt to apply a non-function"
#define sem_err_void_argument			"'void' values may not be arguments"
#define sem_err_bad_cast			"$s: illegal cast of $t to pointer"
#define sem_err_bad_cast1			"$s: illegal cast to $t"
#define sem_err_bad_cast2			"$s: cast to non-equal $t illegal"
#define sem_err_undef_struct			"$c not yet defined - cannot be selected from"
#define sem_err_unknown_field			"$c has no $r field"
#define errs_membobj(m)\
%Z  (m ? "member":"object")
%O

#define bind_rerr_undefined_tag 		"$s tag $b not defined"
#define bind_rerr_linkage_disagreement		"linkage disagreement for $r - treated as $m"
#define bind_rerr_local_extern			"extern $r mismatches top-level declaration"
#define fp_rerr_very_small			"small floating point value converted to 0.0"
#define fp_rerr_small_single			"small (single precision) floating value converted to 0.0"
#define pp_rerr_newline_eof			"missing newline before EOF - inserted"
#define pp_rerr_nonprint_char			"unprintable char %#.2x found - ignored"
#define pp_rerr_illegal_option			"illegal option -D%s%s"
#define pp_rerr_spurious_else			"spurious #else ignored"
#define pp_rerr_spurious_elif			"spurious #elif ignored"
#define pp_rerr_spurious_endif			"spurious #endif ignored"
#define pp_rerr_hash_line			"number missing in #line"
#define pp_rerr_hash_error			"#error encountered \"%s\""
#define pp_rerr_hash_ident			"#ident is not in ANSI C"
#define pp_rerr_junk_eol			"junk at end of #%s line - ignored"
#define sem_rerr_sizeof_bitfield		"sizeof <bit field> illegal - sizeof(int) assumed"
#define sem_rerr_sizeof_void			"size of 'void' required - treated as 1"
#define sem_rerr_sizeof_array			"size of a [] array required, treated as [1]"
#define sem_rerr_sizeof_function		"size of function required - treated as size of pointer"
#define sem_rerr_pointer_arith			"<int> $s <pointer> treated as <int> $s (int)<pointer>"
#define sem_rerr_pointer_arith1			"<pointer> $s <int> treated as (int)<pointer> $s <int>"
#define sem_rerr_assign_const			"assignment to 'const' object $e"
#define sem_rerr_addr_regvar			"'register' attribute for $b ignored when address taken"
#define sem_rerr_lcast				"objects that have been cast are not l-values"
#define sem_rerr_pointer_compare		"comparison $s of pointer and int:\n\
  literal 0 (for == and !=) is only legal case"
#define sem_rerr_different_pointers	  	"differing pointer types: $s"
#define sem_rerr_wrong_no_args			"wrong number of parameters to $e"
#define sem_rerr_implicit_cast1			"$s: implicit cast of pointer to non-equal pointer"
#define sem_rerr_implicit_cast2			"$s: implicit cast of non-0 int to pointer"
#define sem_rerr_implicit_cast3			"$s: implicit cast of pointer to 'int'"
#define sem_rerr_implicit_cast4			"$s: implicit cast of $t to 'int'"
#define sem_rerr_cant_balance			"differing pointer types: $s"

#define sem_rerr_void_indirection		"illegal indirection on (void *): '*'"
#define obj_fatalerr_io_object			"I/O error on object stream"
#define compiler_rerr_no_extern_decl		"no external declaration in translation unit"
#define compiler_fatalerr_io_error		"I/O error writing '%s'"
#define driver_fatalerr_io_object		"I/O error on object stream"
#define driver_fatalerr_io_asm			"I/O error on assembler output stream"
#define driver_fatalerr_io_listing		"I/O error on listing stream"
#ifdef TARGET_HAS_AOUT
#define aout_fatalerr_toomany 			"Too many symbols for 'a.out' output"
#define aout_fatalerr_toobig			"Module too big for a.out formatter"
#endif
#ifdef TARGET_HAS_COFF
#define coff_fatalerr_toomany 			"Too many relocations for COFF format in .o file"
#define coff_fatalerr_toobig			"Module too big for COFF formatter"
#endif
#ifdef TARGET_IS_HELIOS
#define heliobj_warn_12bits 			"Offset %ld > 12 bits"
#define heliobj_warn_16bits			"Offset %ld > 16 bits"
#define heliobj_warn_24bits			"Offset %ld > 24 bits"
#endif
#define misc_fatalerr_space1 			"out of store (for error buffer)"
#define misc_fatalerr_toomanyerrs		"Too many errors"
#define misc_fatalerr_space2			"out of store (in cc_alloc)\n\
(Compilation of the debugging tables requested with the -g option\n\
 requires a great deal of memory. Recompiling without -g, with\n\
 the more restricted -gf option, or with the program broken into\n\
 smaller pieces, may help.)"
#define misc_fatalerr_space3 			"out of store (in cc_alloc)"
#define pp_fatalerr_hash_error			"#error encountered \"%s\""

#define driver_message_nolisting		"Unable to open %s for listing: -l option ignored\n"
#ifdef NO_ASSEMBLER_OUTPUT
#define driver_message_noasm 			"This version of the compiler does not support -s\n"
#endif
#define driver_message_writefail 		"Couldn't write file '%s'\n"
#define driver_message_oddoption		"Unrecognized option '%c': ignored\n"
#define driver_message_readfail			"Couldn't read file '%s'\n"
/* NB the next error can not arise with the current ARM driver */
#define driver_message_toomanyfiles		"Too many file args"
#define driver_message_asmstdout		"Assembly code will go to stdout\n"
#define driver_message_no_listing		"-m option useless without source listing. Ignored\n"
#define driver_message_nomap			"-m file not available or corrupt - ignored\n"
#define driver_message_notest			"This version of the compiler does not support the -test option\n"
#define driver_message_needfile			"At least one file argument wanted\n"
#ifndef COMPILING_ON_ARM_OS
#define driver_message_spool 			"output to clog1.log & clog2.log\n"
#endif
#define driver_message_testfile 		"No files allowed with -test\n"
/* messages generated by misc.c */

#ifndef TARGET_IS_UNIX
#define misc_message_lineno(f,l,s) 		"\"%s\", line %ld: %s: ",f,l,s
#else
#define misc_message_lineno(f,l,s) 		"%s: %ld: %s: ",f,l,s
#endif
#define misc_message_sum1(f,nx,neq1) 		"%s: %ld warning%s", f, nx, \
%Z neq1 ? "":"s"
%O
#define misc_message_sum2 			" (+ %ld suppressed)"
#define misc_message_sum3(nx,neq1)		", %ld error%s", nx, \
%Z neq1 ? "":"s"
%O
#define misc_message_sum5(nx,neq1) 		", %ld serious error%s\n", nx, \
%Z neq1 ? "":"s"
%O

#ifdef TARGET_STACK_MOVES_ONCE
/* Cannot be issued if NARGREGS==0 */
#define warn_untrustable	   		"untrustable code generated for %s"
#endif

%S /* The next batch of things just get mapped onto syserr codes */

#define syserr_mkqualifiedtype			"mkqualifiedtype(..., %ld)"
#define syserr_typeof				"typeof(%ld)"
#define syserr_alignoftype			"alignoftype(%ld,%#lx)"
#define syserr_sizeoftype			"sizeoftype(%ld,%#lx)"
#define syserr_codeoftype			"codeoftype"
#define syserr_equivtype			"equivtype(%ld)"
#define syserr_compositetype			"compositetype(%ld)"
#define syserr_trydiadicreduce			"trydiadreduce(unsigned op %ld)"
#define syserr_trydiadicreduce1			"trydiadreduce(signed op %ld)"
#define syserr_trydiadicreduce2			"trydiadreduce(float op %ld)"
#define syserr_fp_op				"FP op %ld unknown"
#define syserr_trymonadicreduce			"trymonadreduce(int op %ld)"
#define syserr_trymonadicreduce1		"trymonadreduce(float op %ld)"
#define syserr_coerceunary			"coerceunary(bitfieldvalue)"
#define syserr_coerceunary1			"coerceunary(%ld,%#lx)"
#define syserr_bitfieldassign			"bitfieldassign"
#define syserr_mkindex				"sem(mkindex)"
#define syserr_ptrdiff				"sem(mkbinary/ptrdiff)"
#define syserr_va_arg_fn			"sem(odd va_arg fn)"
#define syserr_mkcast				"mkcast(%ld,%#lx)"
#define syserr_mkcast1				"mkcast(%ld)"
#define syserr_te_plain				"te_plain(%ld)"
#define syserr_clone_node			"clone_node(%ld)"
#define syserr_optimise "optimise &(%ld)"
#define syserr_optimise1 "optimise(%ld)"
#define syserr_mcrepofexpr "mcrepofexpr(%ld,%#lx)"
#define syserr_mcreparray "mcrep(array %ld)"
#define syserr_newdbuf "pp_newdbuf(%ld,%ld)"
#define syserr_pp_recursion "pp recursive sleep: '%s'"
#define syserr_pp_special "pp_special(%ld)"
#define syserr_overlarge_store1 "Overlarge storage request (binder %ld)"
#define syserr_overlarge_store2 "Overlarge storage request (local %ld)"
#define syserr_discard2 "discard2 %p"
#define syserr_discard3 "discard3 %p"
#define syserr_alloc_unmark "alloc_unmark - called too often"
#define syserr_alloc_unmark1 "alloc_unmark(no drop_local_store())"
#define syserr_alloc_reinit "alloc_reinit(no drop_local_store())"
#define syserr_addclash "add_clash (0x%lx, 0x%lx)"
#define syserr_forget_slave "forget_slave(%ld, %ld) %ld"
#define syserr_GAP "GAP in reference_register"
#define syserr_corrupt_register "corrupt_register %ld %p"
#define syserr_regalloc "regalloc(corrupt/alloc)"
#define syserr_regalloc_typefnaux "regalloc(typefnaux)"
#define syserr_regalloc_POP "regalloc(POP)"
#define syserr_call2 "CALL2 %ld"
#define syserr_dataflow "dataflow &-var"
#define syserr_choose_real_reg "choose_real_reg %lx"
#define syserr_fail_to_spill "Failed to spill register for %ld"
#define syserr_regalloc_reinit2 "regalloc_reinit2"
#define syserr_regheap "Register heap overflow"
#define syserr_bad_fmt_dir "bad fmt directive"
#define syserr_syserr "syserr simulated"
#define syserr_r1r "r1r %ld"
#define syserr_r2r "r2r %ld"
#define syserr_mr "mr %ld"
#define syserr_expand_jop "expand_jop(2address)"
#define syserr_nonauto_active "Non auto 'active_binders' element"
#define syserr_size_of_binder "size_of_binder"
#define syserr_insertblockbetween "insertblockbetween"
#define syserr_reopen_block "reopen_block called"
#define syserr_scaled_address "emit5(scaled address)"
#define syserr_expand_pushr "expand_jop_macro(PUSHR)"
#define syserr_remove_noop_failed "remove_noop failed"
#define syserr_remove_noop_failed2 "remove_noop failed2"
#define syserr_bad_bindaddr "Bad bindaddr_() with LDRVx1"
#define syserr_ldrfk "duff LDRF/DK %lx"
#define syserr_ldrk "duff LDR/B/WK %lx"
#define syserr_branch_backptr "Bad back-pointer code in branch_chain"
#define syserr_no_main_exit "use_cond_field(no main exit)"
#define syserr_two_returns "Two return exits from a block"
#define syserr_unrefblock "unrefblock"
#define syserr_zip_blocks "zip_blocks(SETSP confused %ld!=%ld)"
#define syserr_live_empty_block "ALIVE empty block L%ld"
#define syserr_loctype "loctype"
#define syserr_adconbase "cse_adconbase"
#define syserr_find_exprn "CSE: find_exprn %ld"
#define syserr_removecomparison "CSE: removecomparison %lx"
#define syserr_evalconst "CSE: evalconst %lx"
#define syserr_scanblock "cse_scanblock %08lx"
#define syserr_prune "csescan(prune)"
#define syserr_globalize "globalize_declaree1(%p,%ld)"
#define syserr_globalize1 "globalize_typeexpr(%p,%ld)"
#define syserr_copy_typeexpr "copy_typeexpr(%p,%ld)"
#define syserr_tentative "is_tentative(tmpdataq == NULL)"
#define syserr_tentative1 "is_tentative(ADCON)"
#define syserr_tentative2 "tentative definition confusion"
#define syserr_instate_decl "instate_decl %ld"
#define syserr_totarget "totargetsex(%d)"
#define syserr_vg_wpos "vg_wpos(%ld)"
#define syserr_vg_wflush "vg_wflush(type=0x%x)"
#define syserr_gendcI "gendcI(%ld,%ld)"
#define syserr_vg_wtype "vg_wtype=0x%x"
#define syserr_codevec "code vector overflow"
#define syserr_nonstring_lit "non-string literal: %.8lx"
#define syserr_addr_lit "Address-literals should not arise in HELIOS mode"
#define syserr_dumplits "dumplits(codep&3)"
#define syserr_dumplits1 "codebuf(dumplits1)"
#define syserr_dumplits2 "codebuf(dumplits2)"
#define syserr_outlitword "outlitword confused"
#define syserr_dumplits3 "codebuf(dumplits3)"
#define syserr_addlocalcse "addlocalcse %ld"
#define syserr_cse_lost_def "cse: def missing"
#define syserr_cse_lost_use "cse: use missing"
#define syserr_cse_linkrefstodefs "CSE: linkrefstodefs"
#define syserr_linkrefstodefs "cse_linkrefstodefs"
#define syserr_safetolift "cse_safetolift"
#define syserr_storecse "storecse %ld %ld\n"
#define syserr_baseop "CSE: baseop %lx"
#define syserr_cse_wordn "CSE_WORDn"
#define syserr_addcsedefs "addcsedefs"
#define syserr_cse_preheader "CSE: loop preheader %d != %ld"
#define syserr_modifycode "modifycode %ld %ld!=%ld"
#define syserr_regtype "ensure_regtype(%lx)"
#define syserr_struct_val "Value of structure requested improperly"
#define syserr_missing_expr "missing expr"
#define syserr_checknot "s_checknot"
#define syserr_structassign_val "value of structure assignment needed"
#define syserr_structdot "Struct returning function (with '.') reaches cg"
#define syserr_floating "Float %%"
#define syserr_cg_expr  "cg_expr(%ld = $s)"
#define syserr_bad_reg "bad reg %lx in use"
#define syserr_bad_fp_reg "fp reg in use"
#define syserr_cg_fnarg "cg_fnarg(odd rep %lx)"
#define syserr_fnarg_struct "cg(struct arg confused)"
#define syserr_cg_fnarg1 "cg_fnargs confused"
#define syserr_cg_argcount "arg count confused"
#define syserr_cg_fnarg2 "cg_fnargs tidy"
#define syserr_padbinder "odd padbinder$b in cg_fnargs()"
#define syserr_cg_fnap "cg_fnap"
#define syserr_cg_cmd "cg_cmd(%ld = $s)"
#define syserr_cg_endcase "cg_cmd(endcase)"
#define syserr_cg_break "cg_cmd(break)"
#define syserr_cg_cont "cg_cmd(cont)"
#define syserr_cg_switch "switch expression must have integer type"
#define syserr_cg_caselist "cg_caselist"
#define syserr_cg_case "cg_cmd(case)"
#define syserr_unset_case "Unset case_lab"
#define syserr_cg_default "cg_cmd(default)"
#define syserr_cg_badrep "rep bad in comparison %.8lx"
#define syserr_cg_plain "(plain) qualifies non-<narrow-int-binder>"
#define syserr_cg_cast "Illegal cast involving a structure or union"
#define syserr_cg_fpsize "fp sizes are wrong %ld %ld"
#define syserr_cg_cast1 "bad mode %ld in cast expression"
#define syserr_cg_cast2 "cast %ld %ld %ld %ld"
#define syserr_cg_indexword "Indexed address mode with word-store"
#define syserr_cg_bad_width "bad width %ld in cg_stind"
#define syserr_cg_bad_mode "bad mcmode %ld in cg_stind"
#define syserr_chroma "chroma_check(target.h setup wrong or multi-temp op confused)"
#define syserr_Q_swap "Q_swap(%lx)"
#define syserr_cg_stgclass "Funny storage class %#lx"
#define syserr_cg_storein "cg_storein(%ld)"
#define syserr_cg_addr "p nasty in '&(p=q)'"
#define syserr_cg_addr1 "cg_addr(%ld)"
#define syserr_cg_shift0 "n=0 in shift_op1"
#define syserr_not_shift "not a shift in shift_operand()"
#define syserr_not_shift1 "not a shift in shift_amount()"
#define syserr_integer_expected "integer expression expected"
#define syserr_nonauto_arg "Non-auto arg!"
#define syserr_struct_result "Unexpected struct result"
#define syserr_cg_topdecl "cg_topdecl(not fn type)"
#define syserr_cg_unknown "unknown top level %ld"

#ifdef TARGET_HAS_AOUT
#define syserr_aout_reloc "relocate_code_to_data(PCreloc)"
#define syserr_aout_checksym "obj_checksym(%s)"
#define syserr_aout_reloc1 "obj_coderelocation %.8lx"
#define syserr_aout_gendata "obj_gendata(%ld)"
#define syserr_aout_datalen "obj_data len=%ld"
#define syserr_aout_data "obj_data %ldEL%ld'%s'"
#define syserr_aout_debug "writedebug: aoutobj linked with xxxdbg not dbx"
#  ifdef TARGET_HAS_DEBUGGER  /* dbx support */
#define syserr_too_many_types "too many types in dbx"
#define syserr_addcodep "bad pointer in dbg_addcodep"
#define syserr_tagbindsort "bad tagbindsort 0x%08lx"
#define syserr_sprinttype "sprinttype(%p,0x%lx)"
#define syserr_dbx_locvar "debugger table confusion(local variable $r %lx %lx)"
#define syserr_dbx_scope "dbg_scope"
#define syserr_dbx_proc "dbg_proc"
#define syserr_dbx_proc1 "dbg_proc confused"
#define syserr_dbx_write "dbg_write(%lx)"
#  endif
#endif

#ifdef TARGET_HAS_COFF
#define syserr_coff_reloc "relocate_code_to_data(PCreloc)"
#define syserr_coff_pcrel "coff(unexpected X_PCreloc)"
#define syserr_coff_m88000 "coffobj(X_DataAddr needs extending)"
#define syserr_coff_toobig "coffobj(Module over 64K -- fix)"
#define syserr_coff_checksym "obj_checksym($r)"
#define syserr_coff_reloc1 "obj_coderelocation(%.8lx)"
#define syserr_coff_gendata "obj_gendata(%ld)"
#define syserr_coff_datalen "obj_data len=%ld"
#define syserr_coff_data "obj_data %ldEL%ld'%s'"
#endif

#ifdef TARGET_IS_HELIOS
#define syserr_heliobj_bad_xref "Invalid external reference $r %#lx"
#define syserr_heliobj_dataseggen "Data seg generation confused"
#define syserr_heliobj_gendata "obj_gendata(%ld)"
#define syserr_heliobj_datalen "obj_data len=%ld"
#define syserr_heliobj_data "obj_data %ldEL%ld'%s'"
#define syserr_heliobj_align "Helios obj align"
#define syserr_heliobj_2def "double definition in obj_symref $r"
#define syserr_heliobj_codedata "code/data confusion for $r"
#endif

%Z      /* The following remain as ordinary (uncompressed) strings */

#define misc_message_announce "+++ %s: "
#define misc_message_announce1 "+++ %s: %ld: %s: "

/*
 * @@@ Wording here is subject to change...
 */
#define misc_message_warning   "Warning"
#define misc_message_error     "Error"
#define misc_message_serious   "Serious error"
#define misc_message_fatal     "Fatal error"
#define misc_message_fatal_internal "Fatal internal error"
#define misc_message_abandoned "\nCompilation abandoned.\n"

#define bind_msg_const "constant expression"

#define moan_floating_type "floating type initialiser"
#define moan_static_int_type "static integral type initialiser"

/*
 * The following are used in init_sym_name_table() and/or ctxtofdeclflag()
 * and eventually find their ways into various error messages.
 */

#define errname_aftercommand       " after command"
#define errname_unset              "<?>"
#define errname_pointertypes       "<after * in declarator>"
#define errname_toplevel           "<top level>"
#define errname_structelement      "<structure component>"
#define errname_formalarg          "<formal parameter>"
#define errname_formaltype         "<formal parameter type declaration>"
#define errname_blockhead          "<head of block>"
#define errname_typename           "<type-name>"
#define errname_unknown            "<unknown context>"
#define errname_error              "<previous error>"
#define errname_invisible          "<invisible>"
#define errname_let                "<let>"
#define errname_character          "<character constant>"
#define errname_wcharacter         "<wide character constant>"
#define errname_integer            "<integer constant>"
#define errname_floatcon           "<floating constant>"
#define errname_string             "<string constant>"
#define errname_wstring            "<wide string constant>"
#define errname_identifier         "<identifier>"
#define errname_binder             "<variable>"
#define errname_tagbind            "<struct/union tag>"
#define errname_cond               "_?_:_"
#define errname_displace           "++ or --"
#define errname_postinc            "++"
#define errname_postdec            "--"
#define errname_arrow              "->"
#define errname_addrof             "unary &"
#define errname_content            "unary *"
#define errname_monplus            "unary +"
#define errname_neg                "unary -"
#define errname_fnap               "<function argument>"
#define errname_subscript          "<subscript>"
#define errname_cast               "<cast>"
#define errname_sizeoftype         "sizeof"
#define errname_sizeofexpr         "sizeof"
#define errname_ptrdiff            "-"   /* for (a-b)=c msg */
#define errname_endcase            "break"
#define errname_block              "<block>"
#define errname_decl               "decl"
#define errname_fndef              "fndef"
#define errname_typespec           "typespec"
#define errname_typedefname        "typedefname"
#define errname_valof              "valof"
#define errname_ellipsis           "..."
#define errname_eol                "\\n"
#define errname_eof                "<eof>"

#ifdef RANGECHECK_SUPPORTED
#  define errname_rangecheck       "<rangecheck>"
#  define errname_checknot         "<checknot>"
#endif

/* end of miperrs.h */
