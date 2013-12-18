
/* C compiler file errors.h :  Copyright (C) A.Mycroft and A.C.Norman */
/* version 1a */

/* AM: (after discussion with LDS) It would seem that error texts below */
/* which (seriously) take 2 or more arguments should be of the form     */
/*    #define ermsg(a,b,c) "ho hum %s had a %s %s", a, b, c             */
/* etc. to allow different sentence order in other (natural) languages. */

/* Beware the macro for pp_warn_nested_comment and its hack.            */
/* N.B. AM suspects the same problem obtains with the Reiser (PCC) pp.  */

/* One nice thing would be to have a variant form of $r (etc) which did */
/* not quote its arg to avoid many uses of _symname() in the code.      */

#ifndef _ERRORS_LOADED

#define _ERRORS_LOADED 1

#define bind_warn_unused_static_decl "unused earlier static declaration of $r"
#define bind_warn_not_in_hdr "extern $r not declared in header"
#define bind_warn_main_not_int "extern 'main' needs to be 'int' function"
#define bind_warn_not_used "variable $r declared but not used"
#define bind_warn_label_not_used "label $r was defined but not used"
#define bind_warn_static_not_used "%s $b declared but not used"
#ifdef never
#define cg_warn_implicit_return "implicit return in non-void %s()"
#endif
#define flowgraf_warn_implicit_return "implicit return in non-void function"
#define lex_warn_extern_clash "extern clash $r, $r clash (ANSI 6 char monocase)"
#define lex_warn_force_unsigned "%s treated as %sul in 32-bit implementation"
#define lex_warn_multi_char "non-portable - not 1 char in '...'"
#define pp_warn_triglyph \
        "ANSI '%c%c%c' trigraph for '%c' found - was this intended?"
/* The next line's concatenation is a hack round pre 1.59 NorCroft bug.    */
#define pp_warn_nested_comment "character sequence /""* inside comment - error?"
#define pp_warn_redefinition "Re-definition of #define macro %s"
#define pp_warn_bad_pragma "Unrecognised #pragma (no '-')"
#define pp_warn_bad_pragma1 "Unrecognised #pragma -%c"
#define pp_warn_unused_macro "#define macro '%s' defined but not used"
#define regalloc_warn_use_before_set "$b may be used before being set"
#define regalloc_warn_never_used "$b is set but never used"
#define sem_warn_unsigned "ANSI surprise: 'long' $s 'unsigned' yields 'long'"
#define sem_warn_format_type "actual type $t mismatches format '%.*s'"
#define sem_warn_bad_format "Illegal format conversion '%%%c'"
#define sem_warn_incomplete_format "Incomplete format string"
#define sem_warn_format_nargs "Format requires %ld parameter%s, but %ld given"
#define sem_warn_addr_array "'&' unnecessary for function or array $e"
#define sem_warn_bad_shift "shift by %ld illegal in ANSI C"
#define sem_warn_bad_shift1 "shift by %ld illegal - here treated as %ld"
#define sem_warn_fp_overflow "floating point overflow when folding"
#define sem_warn_fix_fail "floating to integral conversion failed"
#define sem_warn_low_precision "lower precision in wider context: $s"
#define sem_warn_odd_condition "use of $s in condition context"
#define sem_warn_void_context "no side effect in void context: $s"
#define sem_warn_narrowing "implicit narrowing cast: $s"
#define sem_warn_pointer_int "explicit cast of pointer to 'int'"
#define syn_warn_hashif_undef "Undefined macro $r in #if - treated as 0"
#define syn_warn_invent_extern "inventing 'extern int %s();'"
#define syn_warn_spurious_braces "spurious {} around scalar initialiser"
#define syn_warn_dangling_else "Dangling 'else' indicates possible error"
#define syn_warn_void_return "non-value return in non-void function"
#define syn_warn_use_of_short \
        "'short' slower than 'int' on this machine (see manual)"
#define syn_warn_undeclared_parm \
        "formal parameter $r not declared - 'int' assumed"
#define syn_warn_old_style "Olde-style function $r"
#define syn_warn_give_args "Deprecated declaration %s() - give arg types"
#define syn_warn_untyped_fn "'int %s()' assumed - 'void' intended?"
#define vargen_warn_nonull "omitting trailing '\\0' for char [%ld]"
#define vargen_warn_unnamed_bitfield \
        "*** unclear ANSI spec: unnamed bit field initialised to 0"

#define bind_err_duplicate_tag "duplicate definition of $s tag $b"
#define bind_err_reuse_tag "re-using $s tag $b as $s tag"
#define bind_err_type_disagreement "type disagreement for $r"
#define bind_err_duplicate_definition "duplicate definition of $r"
#define bind_err_duplicate_label "duplicate definition of label $r - ignored"
#define bind_err_unset_label "label $r has not been set"
#define bind_err_undefined_static \
        "static function $b not defined - treated as extern"
#define lex_err_extern_clash "extern clash $r, $r (linker %ld char%s)"
#define lex_err_ioverflow "Number %s too large for 32-bit implementation"
#define lex_err_overlong_fp "Grossly over-long floating point number"
#define lex_err_fp_syntax1 "Digit required after exponent marker"
#define lex_err_overlong_hex "Grossly over-long hexadecimal constant"
#define lex_err_overlong_int "Grossly over-long number"
#define lex_err_need_hex_dig "Hex digit needed after 0x or 0X"
#define lex_err_need_hex_dig1 "Missing hex digit(s) after \\x"
#define lex_err_backslash_blank \
        "\\<space> and \\<tab> are invalid string escapes"
#define lex_err_unterminated_string "Newline or end of file within string"
#define lex_err_bad_hash "misplaced preprocessor character '%c'"
#define lex_err_bad_char "illegal character (0x%lx = \'%c\') in source"
#define lex_err_ellipsis "'...' must have exactly 3 dots"
#define lex_err_illegal_whitespace "$s may not have whitespace in it"
#define fp_err_very_big "Overlarge floating point value found"
#define fp_err_big_single \
        "Overlarge (single precision) floating point value found"
#define pp_err_eof_comment "EOF in comment"
#define pp_err_eof_string "EOF in string"
#define pp_err_eol_string "quote (%c) inserted before newline"
#define pp_err_eof_escape "EOF in string escape"
#define pp_err_missing_quote "Missing '%c' in pre-processor command line"
#define pp_err_if_defined "No identifier after #if defined"
#define pp_err_if_defined1 "No ')' after #if defined(..."
#define pp_err_rpar_eof "Missing ')' after %s(... on line %ld"
#define pp_err_many_args "Too many arguments to macro %s(... on line %ld"
#define pp_err_few_args "Too few arguments to macro %s(... on line %ld"
#define pp_err_missing_identifier "Missing identifier after #define"
#define pp_err_missing_parameter "Missing parameter name in #define %s(..."
#define pp_err_missing_comma "Missing ',' or ')' after #define %s(..."
#define pp_err_undef "Missing identifier after #undef"
#define pp_err_ifdef "Missing identifier after #ifdef"
#define pp_err_include_quote "Missing '<' or '\"' after #include"
#define pp_err_include_junk "Junk after #include %c%s%c"
#define pp_err_include_file "#include file %c%s%c wouldn't open"
#define pp_err_unknown_directive "Unknown directive: #%s"
#define pp_err_endif_eof "Missing #endif at EOF"
#define sem_err_typeclash "Illegal types for operands: $s"
#define sem_err_sizeof_struct "size of struct $b needed but not yet defined"
#define sem_err_sizeof_union "size of union $b needed but not yet defined"
#define sem_err_lvalue "Illegal in lvalue: function or array $e"
#define sem_err_bitfield_address "bit fields do not have addresses"
#define sem_err_lvalue1 "Illegal in l-value: 'enum' constant $b"
#define sem_err_lvalue2 "Illegal in l-value: $s"
#define sem_err_nonconst "illegal in %s: <unknown>"
#define sem_err_nonconst1 "illegal in %s: non constant $b"
#define sem_err_nonconst2 "illegal in %s: $s"
#define sem_err_nonfunction "attempt to apply a non-function"
#define sem_err_void_argument "'void' values may not be arguments"
#define sem_err_wrong_no_args "wrong number of parameters to $e"
#define sem_err_bad_cast "$s: illegal cast of $t to pointer"
#define sem_err_bad_cast1 "$s: illegal cast to $t"
#define sem_err_bad_cast2 "$s: cast to non-equal $t illegal"
#define sem_err_undef_struct \
        "struct/union $b not yet defined - cannot be selected from"
#define sem_err_unknown_field "struct/union $b has no $r field"
#define syn_err_bitsize "Bit size %ld illegal - 1 assumed"
#define syn_err_arraysize "Array size %ld illegal - 1 assumed"
#define syn_err_expected "expected $s - inserted before $l"
#define syn_err_expected1 "expected $s%s - inserted before $l"
#define syn_err_expected2 "expected $s or $s - inserted $s before $l"
#define syn_err_expected_expr "<expression> expected but found $s"
#ifndef NO_VALOF_BLOCKS
#define syn_err_valof_block \
        "{ following a cast will be treated as VALOF block"
#endif
#define syn_err_typedef "typedef name $r used in expression context"
#define syn_err_expected_id "Expected <identifier> after $s but found $l"
#define syn_err_hashif_eof "EOF not newline after #if ..."
#define syn_err_hashif_junk "Junk after #if <expression>"
#define syn_err_initialisers "too many initialisers in {} for aggregate"
#define syn_err_initialisers1 \
        "{} must have 1 element to initialise scalar or auto"
#define syn_err_default "'default' not in switch - ignored"
#define syn_err_default1 "duplicate 'default' case ignored"
#define syn_err_case "'case' not in switch - ignored"
#define syn_err_case1 "duplicated case constant: %ld"
#define syn_err_expected_cmd "<command> expected but found a $s"
#define syn_err_expected_while "'while' expected after 'do' - found $l"
#define syn_err_else "Misplaced 'else' ignored"
#define syn_err_continue "'continue' not in loop - ignored"
#define syn_err_break "'break' not in loop or switch - ignored"
#define syn_err_no_label "'goto' not followed by label - ignored"
#define syn_err_no_brace "'{' of function body expected - found $l"
#define syn_err_stgclass \
        "storage class $s not permitted in context %s - ignored"
#define syn_err_stgclass1 "storage class $s incompatible with $m - ignored"
#define syn_err_typeclash "type $s inconsistent with $m"
#define syn_err_tag_brace \
        "'{' or <identifier> expected after $s, but found $l"
#define syn_err_expected3 "Expecting <declarator> or <type>, but found $l"
#define syn_err_unneeded_id \
        "Identifier (%s) found in <abstract declarator> - ignored"
#define syn_err_duplicate_type \
        "duplicate type specification of formal parameter $r"
#define syn_err_not_a_formal "Non-formal $r in parameter-type-specifier"
#define syn_err_cant_init "$m variables may not be initialised"
#define syn_err_bitfield "illegal bit field type $t - 'int' assumed"
#define syn_err_undef_struct \
        "undefined struct/union $b cannot be member"
#define syn_err_enumdef \
        "<identifier> expected but found $l in 'enum' definition"
#define syn_err_misplaced_brace "Misplaced '{' at top level - ignoring block"
#ifdef TARGET_CALL_USES_DESCRIPTOR
#define vargen_err_badinit "illegal initialisation to $r%+d"
#endif
#define vargen_err_long_string "string initialiser longer than char [%ld]"
#define vargen_err_nonstatic_addr \
        "non-static address $b in pointer initialiser"
#define vargen_err_bad_ptr "$s: illegal use in pointer initialiser"
#define vargen_err_undefined_struct \
        "struct $b must be defined for (static) variable declaration"
#define vargen_err_undefined_union \
        "union $b must be defined for (static) variable declaration"
#define vargen_err_open_array "Uninitialised static [] arrays illegal"
#define vargen_err_auto_array "auto array $r may not be initialised"

#define syn_moan_hashif "#if <expression>"
#define syn_moan_case "case expression (ignored)"

#define bind_rerr_undefined_tag "$s tag $b not defined"
#define bind_rerr_linkage_disagreement \
        "linkage disagreement for $r - treated as $m"
#define lex_rerr_8_or_9 "Digit 8 or 9 found in octal number"
#define lex_rerr_illegal_esc "illegal string escape '\\%c' - treated as %c"
#define lex_rerr_empty_char "no chars in character constant ''"
#define lex_rerr_overlong_char "more than 4 chars in '...'"
#define fp_rerr_very_small "small floating point value converted to 0.0"
#define fp_rerr_small_single \
        "small (single precision) floating value converted to 0.0"
#define pp_rerr_newline_eof "missing newline before EOF - inserted"
   /* should the next line say "unprintable char"? */
#define pp_rerr_nonprint_char "control character %#.2x found - ignored"
#define pp_rerr_illegal_option "illegal option -D%s%s"
#define pp_rerr_spurious_else "spurious #else ignored"
#define pp_rerr_spurious_elif "spurious #elif ignored"
#define pp_rerr_spurious_endif "spurious #endif ignored"
#define pp_rerr_hash_error "#error encountered \"%s\""
#define pp_rerr_junk_eol "junk at end of #%s line - ignored"
#define sem_rerr_sizeof_bitfield \
        "sizeof <bit field> illegal - sizeof(int) assumed"
#define sem_rerr_sizeof_void "size of 'void' required - treated as 1"
#define sem_rerr_sizeof_array "size of a [] array required, treated as [1]"
#define sem_rerr_sizeof_function \
        "size of function required - treated as size of pointer"
#define sem_rerr_pointer_arith \
        "<int> $s <pointer> treated as <int> $s (int)<pointer>"
#define sem_rerr_pointer_arith1 \
        "<pointer> $s <int> treated as (int)<pointer> $s <int>"
#define sem_rerr_assign_const "assignment to 'const' object $e"
#define sem_rerr_addr_regvar \
        "'register' attribute for $b ignored when address taken"
#define sem_rerr_pointer_compare                      \
        "comparison $s of pointer and int:\n" \
        "  literal 0 (for == and !=) is only legal case"
#define sem_rerr_implicit_case "$s: implicit cast of $t to 'int'"
#define sem_rerr_different_pointers "differing pointer types: $s"
#define sem_rerr_fn_cast "$s: cast between function and object pointer"
#define sem_rerr_implicit_cast1 \
        "$s: implicit cast of pointer to non-equal pointer"
#define sem_rerr_implicit_cast2 "$s: implicit cast of non-0 int to pointer"
#define sem_rerr_implicit_cast3 "$s: implicit cast of pointer to 'int'"
#define sem_rerr_implicit_cast4 "$s: implicit cast of $t to 'int'"
#define sem_rerr_cant_balance "differing pointer types: $s"
#define syn_rerr_array_0 "Array [0] found"
#ifndef NO_VALOF_BLOCKS
#define syn_rerr_void_valof "void valof blocks are not permitted"
#endif
#define syn_rerr_undeclared "Undeclared name, inventing 'extern int %s'"
#define syn_rerr_insert_parens \
        "parentheses (..) inserted around expression following $s"
#define syn_rerr_return "return <expr> illegal for void function"
#define syn_rerr_missing_type "Missing type specification - 'int' assumed"
#define syn_rerr_missing_type1 \
        "Omitted <type> before formal declarator - 'int' assumed"
#define syn_rerr_missing_type2 \
        "function prototype formal $r needs type or class - 'int' assumed"
#define syn_rerr_fn_ptr "%s $r may not be function - assuming function pointer"
#define syn_rerr_variable "variable"
#define syn_rerr_struct "struct component"
#define syn_rerr_fn_ptr1 \
        "function $r may not be initialised - assuming function pointer"
#define syn_rerr_missing_formal "formal name missing in function definition"
#define syn_rerr_ineffective "declaration with no effect"
#define syn_rerr_missing_type3 \
        "type or class needed (except in function definition) - 'int' assumed"
#define syn_rerr_semicolon_in_arglist \
        "',' (not ';') separates formal parameters"

#define obj_fatalerr_io_object "I/O error on object stream"
#define driver_fatalerr_io_object "I/O error on object stream"
#define driver_fatalerr_io_asm "I/O error on assembler output stream"
#define driver_fatalerr_io_listing "I/O error on listing stream\n"
#define armobj_fatalerr_toomany "Too many symbols for ACORN linker"
#define s370obj_fatalerr_toomany "Too many symbols for ACORN linker"
#define xpuobj_fatalerr_toomany "Too many symbols for ACORN linker"
#define aout_fatalerr_toomany "Too many symbols for 'a.out' output"
#define armdbg_fatalerr_toobig \
        "Debug table size exceeds space in Acorn AOF format"
#define misc_fatalerr_space1 "out of store (for error buffer)"
#define misc_fatalerr_toomanyerrs "Too many errors"
#define misc_fatalerr_space2 "out of store (in cc_alloc)\n"         \
 "  Sorry, but: you requested debugger tables with the -g option\n" \
 "  and the Acorn debugger tables are store profligate.\n"          \
 "  Recompilation without -g or in pieces may help."
#define misc_fatalerr_space3 "out of store (in cc_alloc)"
#define pp_fatalerr_error "#error encountered \"%s\""

#define misc_message_fatal_internal "Fatal internal error"
#define misc_disaster_banner                                                 \
                                                                         "\n"\
"*************************************************************************\n"\
"*                                                                       *\n"\
"*                      This error should not occur.                     *\n"\
"*        Please contact your maintenance authority (e.g. dealer)        *\n"\
"*                        if you can duplicate it.                       *\n"\
"*                                                                       *\n"\
"*************************************************************************\n"\
                                                                         "\n"

#define misc_message_fatal "Fatal error"
#define misc_message_abandoned "\nCompilation abandoned.\n"
#define misc_message_warning "Warning"
#define misc_message_error "Error"

#define driver_message_nolisting \
        "Unable to open %s for listing: -l option ignored\n"
#ifdef NO_ASSEMBLER_OUTPUT
#define driver_message_noasm \
        "This version of the compiler does not support -s\n"
#endif
#define driver_message_writefail "Couldn't write file '%s'\n"
#define driver_message_oddoption "Unrecognized option '%c': ignored\n"
#define driver_message_readfail "Couldn't read file '%s'\n"
/* NB the next error can not arise with the current ARM driver */
#define driver_message_toomanyfiles "Too many file args"
#define driver_message_asmstdout "Assembly code will go to stdout\n"
#define driver_message_no_listing \
        "-m option useless without source listing. Ignored\n"
#define driver_message_nomap "-m file not available or corrupt - ignored\n"
#define driver_message_notest \
        "This version of the compiler does not support the -test option\n"
#define driver_message_needfile "At least one file argument wanted\n"
#ifndef COMPILING_ON_ARM
#define driver_message_spool "output to clog1.log & clog2.log\n"
#endif
#define driver_message_testfile "No files allowed with -test\n"
/* messages generated by misc.c */

#define misc_message_lineno "%s, line %ld, %s: "
#define misc_message_announce "+++ %s: "
#define misc_message_announce1 "+++ %s, line %ld, %s: "
#define misc_message_sum1(f,n,neq1) "%s: %ld warning%s", f, n, neq1 ? "":"s"
#define misc_message_sum2 " (+ %ld supressed)"
#define misc_message_sum3(n,neq1) ", %ld error%s", n, neq1 ? "":"s"
#define misc_message_sum4 " (+ %ld supressed)"
#define misc_message_sum5(n,neq1) ", %ld serious error%s\n", n, neq1 ? "":"s"

/* The following are used in init_sym_name_table() and eventually find     */
/* their ways into various error messages.                                 */

#define errname_error              "<previous error>"
#define errname_invisible          "<invisible>"
#define errname_let                "<let>"
#define errname_character          "<character constant>"
#define errname_integer            "<integer constant>"
#define errname_floatcon           "<floating constant>"
#define errname_string             "<string constant>"
#define errname_identifier         "<identifier>"
#define errname_binder             "<variable>"
#define errname_tagbind            "<struct/union tag>"
#define errname_cond               "_?_:_"
#define errname_displace           "++ or --"
#define errname_postinc            "++"
#define errname_postdec            "--"
#define errname_arrow              "->"
#define errname_leftshiftequal     "<<"
#define errname_rightshiftequal    ">>"
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
#define errname_sharp              "#"

#endif

/* end of errors.h */
