
/*
 * C compiler file errors.h
 * Copyright (C) Codemist Ltd, 1993
 */

#ifndef _errors_LOADED
#define _errors_LOADED 1

#pragma -v3
typedef char *syserr_message_type;
extern void syserr(syserr_message_type errcode, ...);

#pragma -v0

/*
 * C compiler error prototype file (miperrs.h)
 * Copyright (C) Codemist Ltd, 1988.
 */

/*
 * RCS $Revision: 1.3 $ Codemist 150
 * Checkin $Date: 1993/07/27 16:09:21 $
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


      /* Map strings to offsets in compressed string table */

#define warn_usage_rw \
        "\027\246\020behavi\206r\030$b wr\034t\023 \033\005\007a\005\271\
h\206\002\001\021v\023\024\220qu\023c\004\215t"    /* "undefined behaviour: $b written and read without intervening sequence point" */
#define warn_usage_ww \
        "\027\246\020behavi\206r\030$b wr\034t\023 tw\213\004\271h\206\002\
\001\021v\023\024\220qu\023c\004\215t"    /* "undefined behaviour: $b written twice without intervening sequence point" */

#define bind_warn_extern_clash \
        "\204\021\267\244\037\221$r\357$r \244\037\221\050ANSI 6 \322\022 \
m\006o\313e\051"    /* "extern clash $r, $r clash (ANSI 6 char monocase)" */
#define bind_warn_unused_static_decl "\027\237\020e\022li\240\100\013\336\
\343\022\013\176\140$r"    /* "unused earlier static declaration of $r" */
#define bind_warn_not_in_hdr "\204\021\267$r \200\343a\007\005\250he\211\
\003"    /* "extern $r not declared in header" */
#define bind_warn_main_not_int "\204\021\267\'ma\001\201ne\205\016\035b\
\004\'\276\201\256\025"    /* "extern 'main' needs to be 'int' function" */
#define bind_warn_label_not_used "l\265\235 $r w\253\246\020\333\002\200\
\237\205"    /* "label $r was defined but not used" */
/*
 * Note that when part of an error string MUST be stored as a regular
 * non-compressed string I have to inform the GenHdrs utility with %Z and %O
 * This arises when a string contains literal strings as extra sub-args.
 */
#define bind_warn_not_used(is_typedef,is_fn,binder) \
        "%s $b \343a\007\005\333\002\200\237\205"    /* "%s $b declared but not used" */, \
      ((is_typedef) ? "typedef" : ((is_fn) ? "function" : "variable")),\
      binder
#define bind_warn_static_not_used "\100\013\336$b \343a\007\005\333\002\
\200\237\205"    /* "static $b declared but not used" */
#define cg_warn_implicit_return "i\233l\213\257\007t\374n\252\261\375\361\
%s\050\051"    /* "implicit return in non-void %s()" */
#define flowgraf_warn_implicit_return "i\233l\213\257\007t\374n\252\261\
\375\361\256\025"    /* "implicit return in non-void function" */
#define pp_warn_triglyph \
        "ANSI \'%c%c%c\201\224\214rap\221\247\'%c\201f\347\005\216w\253\
\262\266\276\023\036d\077"    /* "ANSI '%c%c%c' trigraph for '%c' found - was this intended?" */
#define pp_warn_nested_comment "\322\022\226\210\220qu\023c\004%s7s\304\
\004\223mm\307"    /* "character sequence %s inside comment" */
#define pp_warn_many_arglines \
        "\050\207ssib\260\003r\010\051\030\076\075 %lu l\001e\016\140m\226\
r\015\317\227\307s"    /* "(possible error): >= %lu lines of macro arguments" */
#define pp_warn_redefinition "\007\174\330\246\034\176\140\043\246\004m\
\226r\015%s"    /* "repeated definition of #define macro %s" */
#define pp_rerr_redefinition "\334\272\003\024\007\246\034\176\140\043\246\
\004m\226r\015%s"    /* "differing redefinition of #define macro %s" */
#define pp_rerr_nonunique_formal "du\315\213\341m\226r\015\264\203\362\243\
e\021\030\'%s\'"    /* "duplicate macro formal parameter: '%s'" */
#define pp_rerr_define_hash_arg "\245\003\033\005\140\043 \200m\226r\015\
\264\203\362\243e\021"    /* "operand of # not macro formal parameter" */
#define pp_rerr_define_hashhash "\043\043 \323rs\002\231l\037\002tok\023\
\252\043\246\004body"    /* "## first or last token in #define body" */
#define pp_warn_ifvaldef "\043if\036\032%s may7d\213\341\224\206b\177\251\
."    /* "#ifdef %s may indicate trouble..." */ /* MACH_EXTNS */
#define pp_warn_nonansi_header "N\006-ANSI \043\001\244u\371\074%s\076"    /* "Non-ANSI #include <%s>" */
#define pp_warn_bad_pragma "Un\007\223gn\031\020\043pragm\212\050n\015\'\
-\201\231\363\267w\010d\051"    /* "Unrecognised #pragma (no '-' or unknown word)" */
#define pp_warn_bad_pragma1 "Un\007\223gn\031\020\043pragm\212-%c"    /* "Unrecognised #pragma -%c" */
#define pp_warn_unused_macro "\043\246\004m\226r\015\'%s\201\246\020\333\
\002\200\237\205"    /* "#define macro '%s' defined but not used" */
#define regalloc_warn_use_before_set "$b m\367b\004\237\020beW\004be\024\
\220t"    /* "$b may be used before being set" */
#define regalloc_warn_never_used "$b \266\305\333\002nev\240\237\205"    /* "$b is set but never used" */
#define sem_warn_unsigned "ANSI s\374pr\031e\030\'l\006g\201$s \'\027s\214\
n\205\201yi\235d\016\'l\006g\'"    /* "ANSI surprise: 'long' $s 'unsigned' yields 'long'" */
#define sem_warn_format_type "aQu\203\306$t m\031m\013\322e\016\264\345\'\
%.\052s\'"    /* "actual type $t mismatches format '%.*s'" */
#define sem_warn_bad_format "Il\222\203\264\345\217\325s\176\'%%%c\'"    /* "Illegal format conversion '%%%c'" */
#define sem_warn_incomplete_format "In\223\233\177t\004\264\345\321\017"    /* "Incomplete format string" */
#define sem_warn_format_nargs "F\010m\345\007qui\007\016%ld \362\243e\021\
%s\357\333\002%ld giv\023"    /* "Format requires %ld parameter%s, but %ld given" */
#define sem_warn_addr_array "\'\046\201\027necess\022\202\247\327\231\022\
r\367$e"    /* "'&' unnecessary for function or array $e" */
#define sem_warn_bad_shift(_m,_n) "shif\354$m b\202%ld \027\246\020\250\
ANSI C"    /* "shift of $m by %ld undefined in ANSI C" */,_m,_n
#define sem_warn_divrem_0 "\334v\031\176b\202z\003o\030$s"    /* "division by zero: $s" */
#define sem_warn_ucomp_0 "od\005\027s\214n\020\223\233\022\031\006 \271\
\2210\030$s"    /* "odd unsigned comparison with 0: $s" */
#define sem_warn_fp_overflow(op) "\337\311\002\217\100\033\002o\325\255\
w\030$s"    /* "floating point constant overflow: $s" */,op
#define sem_rerr_udiad_overflow(op,_a,_b,_c) "\027s\214n\020\217\100\033\
\002o\325\255w\030$s"    /* "unsigned constant overflow: $s" */,op
#define sem_rerr_diad_overflow(op,_a,_b,_c) "s\214n\020\217\100\033\002\
o\325\255w\030$s"    /* "signed constant overflow: $s" */,op
#define sem_rerr_umonad_overflow(op,_a,_b) "\027s\214n\020\217\100\033\002\
o\325\255w\030$s"    /* "unsigned constant overflow: $s" */,op
#define sem_rerr_monad_overflow(op,_a,_b) "s\214n\020\217\100\033\002o\325\
\255w\030$s"    /* "signed constant overflow: $s" */,op
#define sem_rerr_implicit_cast_overflow(_t,_a,_b) \
                                "i\233l\213\257\313\002\050\035$t\332o\325\
\255w"    /* "implicit cast (to $t) overflow" */,_t
#define sem_warn_fix_fail "\337\310\276egr\203\217\325s\176fa5\205"    /* "floating to integral conversion failed" */
#define sem_warn_index_ovfl "\206t-of-b\347\005o\272\305%ld\252\275s"    /* "out-of-bound offset %ld in address" */
#define sem_warn_low_precision "\136w\240p\007c\031\025\252w\304\240\217\
t\204t\030$s"    /* "lower precision in wider context: $s" */
#define sem_warn_odd_condition "\237\004\140$s\252\217d\034\176\217t\204\
t"    /* "use of $s in condition context" */
#define sem_warn_void_context "n\015s\304\004e\272ec\002\250\375\361\217\
t\204t\030$s"    /* "no side effect in void context: $s" */
#define sem_warn_olde_mismatch "\317\227\356\033\005\236d-\100y\260\362\
\243e\210m\031m\013\322\030$e"    /* "argument and old-style parameter mismatch: $e" */
#define sem_warn_uncheckable_format \
        "\'\264\013\201\317. \035pr\276f\057sc\033\032etc. \266v\022i\265\
\177\357s\015c\033\200b\004\322eck\205"    /* "'format' arg. to printf/scanf etc. is variable, so cannot be checked" */
#define sem_warn_narrow_voidstar "i\233l\213\257\313\002f\350m \050\375\
\361\052\051\357C\053\053 Wb\304s"    /* "implicit cast from (void *), C++ forbids" */
#define sem_warn_narrowing "i\233l\213\257n\022\350w\024\313t\030$s"    /* "implicit narrowing cast: $s" */
#define sem_warn_fn_cast \
        "$s\030\313\002betwe\023 \327\215\210\033\005\261\327\373eQ"    /* "$s: cast between function pointer and non-function object" */
#define sem_warn_pointer_int "\204\315\213\257\313\354\215\210\035\'\276\'"    /* "explicit cast of pointer to 'int'" */
#define bind_err_extern_clash "\204\021\267\244\037\221$r\357$r \050l\001\
k\240%ld \322\022%s\051"    /* "extern clash $r, $r (linker %ld char%s)" */
#define bind_err_duplicate_tag "du\315\213\341\246\034\176\140$s tag $b"    /* "duplicate definition of $s tag $b" */
#define bind_err_reuse_tag "\007-\237\024$s tag $b \253$s tag"    /* "re-using $s tag $b as $s tag" */
#define bind_err_incomplete_tentative \
        "\001\223\233\177t\004t\307\013iv\004\343\022\013\176\140$r"    /* "incomplete tentative declaration of $r" */
#define bind_err_type_disagreement "\306d\031ag\007\301\356\247$r"    /* "type disagreement for $r" */
#define bind_err_duplicate_definition "du\315\213\341\246\034\176\140$r"    /* "duplicate definition of $r" */
#define bind_err_duplicate_label "du\315\213\341\246\034\176\140l\265\235 \
$r \216\326"    /* "duplicate definition of label $r - ignored" */
#define bind_err_unset_label "l\265\235 $r h\253\200be\023 \220t"    /* "label $r has not been set" */
#define bind_err_undefined_static \
        "\100\013\336\327$b \200\246\020\216t\007\330\253\204\021n"    /* "static function $b not defined - treated as extern" */
#define bind_err_conflicting_globalreg \
        "\217fliQ\024g\136b\203\340\210\343\022\013\025\016\247$b"    /* "conflicting global register declarations for $b" */
#define fp_err_very_big "O\325l\317\004\337\311\002\351\004f\347d"    /* "Overlarge floating point value found" */
#define fp_err_big_single \
        "O\325l\317\004\050s\017\260p\007c\031\025\332\337\311\002\351\004\
f\347d"    /* "Overlarge (single precision) floating point value found" */
#define pp_err_eof_comment "EOF\252\223mm\307"    /* "EOF in comment" */
#define pp_err_eof_string "EOF\252\321\017"    /* "EOF in string" */
#define pp_err_eol_string "quot\004\050%c\0517s\003t\020beW\004newl\001\
e"    /* "quote (%c) inserted before newline" */
#define pp_err_eof_escape "EOF\252\321\024esca\174"    /* "EOF in string escape" */
#define pp_err_missing_quote "M\031\316\'%c\'\252p\007-p\350cess\231\223\
mm\033\005l\001e"    /* "Missing '%c' in pre-processor command line" */
#define pp_err_if_defined "N\015\304\307i\323\240\366\210\043i\032\246\205"    /* "No identifier after #if defined" */
#define pp_err_if_defined1 "N\015\'\051\201\366\210\043i\032\246\205\050\
\251."    /* "No ')' after #if defined(..." */
#define pp_err_rpar_eof "M\031\316\'\051\201\366\210%s\050\251. \006 l\001\
\004%ld"    /* "Missing ')' after %s(... on line %ld" */
#define pp_err_many_args "T\372m\033\202\317\227\307\016\035m\226r\015%\
s\050\251. \006 l\001\004%ld"    /* "Too many arguments to macro %s(... on line %ld" */
#define pp_err_few_args "T\372few \317\227\307\016\035m\226r\015%s\050\251\
. \006 l\001\004%ld"    /* "Too few arguments to macro %s(... on line %ld" */
#define pp_err_missing_identifier "M\031\316\304\307i\323\240\366\210\043\
\246e"    /* "Missing identifier after #define" */
#define pp_err_missing_parameter "M\031\316\362\243e\210n\243\004\250\043\
\246\004%s\050\251."    /* "Missing parameter name in #define %s(..." */
#define pp_err_missing_comma "M\031\316\',\201\231\'\051\201\366\210\043\
\246\004%s\050\251."    /* "Missing ',' or ')' after #define %s(..." */
#define pp_err_undef "M\031\316\304\307i\323\240\366\210\043\027\230"    /* "Missing identifier after #undef" */
#define pp_err_ifdef "M\031\316\304\307i\323\240\366\210\043if\230"    /* "Missing identifier after #ifdef" */
#define pp_err_include_quote "M\031\316\'\074\201\231\'\"\201\366\210\043\
\001\244u\036"    /* "Missing '<' or '\"' after #include" */
#define pp_err_include_junk "J\320 \366\210\043\001\244u\371%c%s%c"    /* "Junk after #include %c%s%c" */
#define pp_err_include_file "\043\001\244u\371f5\004%c%s%c w\206ldn\'\002\
\245\023"    /* "#include file %c%s%c wouldn't open" */
#define pp_err_unknown_directive "Unk\331\267\334\007Qive\030\043%s"    /* "Unknown directive: #%s" */
#define pp_err_endif_eof "M\031\316\043\023\334\032\345EOF"    /* "Missing #endif at EOF" */
#define sem_err_typeclash "Il\222\203\225\174\016\247\245\003\033ds\030\
$s"    /* "Illegal types for operands: $s" */
#define sem_err_sizeof_struct "siz\004\140$c ne\205\020\333\002\200ye\002\
\246\205"    /* "size of $c needed but not yet defined" */
#define sem_err_lvalue "Il\222\014\252l\351e\030\327\231\022r\367$e"    /* "Illegal in lvalue: function or array $e" */
#define sem_err_bitfield_address "b\257\323\235d\016d\015\200hav\004\275\
\220s"    /* "bit fields do not have addresses" */
#define sem_err_lvalue1 "Il\222\014\252l-\351e\030\'\023\227\201\217\100\
\033\002$b"    /* "Illegal in l-value: 'enum' constant $b" */
#define sem_err_lvalue2 "Il\222\014\252\262\004\217t\204\354\033 l-\351\
e\030$s"    /* "Illegal in the context of an l-value: $s" */
#define sem_err_nonconst "\234\014\252%s\030\074\363n\076"    /* "illegal in %s: <unknown>" */
#define sem_err_nonconst1 "\234\014\252%s\030\242 \217\100\033\002$b"    /* "illegal in %s: non constant $b" */
#define sem_err_nonconst2 "\234\014\252%s\030$s"    /* "illegal in %s: $s" */
#define sem_err_nonfunction "\013te\233\002\035ap\315\202\212\261\256\025"    /* "attempt to apply a non-function" */
#define sem_err_void_argument "\'\375\304\201\351e\016m\367\200b\004\317\
\227\307s"    /* "'void' values may not be arguments" */
#define sem_err_bad_cast "$s\030\274\313\354$t \035\376"    /* "$s: illegal cast of $t to pointer" */
#define sem_err_bad_cast1 "$s\030\274\313\002\035$t"    /* "$s: illegal cast to $t" */
#define sem_err_bad_cast2 "$s\030\313\002\035\261equ\203$t \234\014"    /* "$s: cast to non-equal $t illegal" */
#define sem_err_undef_struct \
        "$c \200ye\002\246\020\216c\033\200b\004\220\177\344f\350m"    /* "$c not yet defined - cannot be selected from" */
#define sem_err_unknown_field "$c h\253n\015$r \323\235d"    /* "$c has no $r field" */
#define errs_membobj(_m)\
  (_m ? "member":"object")


#define bind_rerr_undefined_tag "$s tag $b \200\246\205"    /* "$s tag $b not defined" */
#define bind_rerr_linkage_disagreement \
        "l\001kag\004d\031ag\007\301\356\247$r \216t\007\330\253$m"    /* "linkage disagreement for $r - treated as $m" */
#define bind_rerr_duplicate_typedef "du\315\213\341\225\174\036\032$r"    /* "duplicate typedef $r" */
#define bind_rerr_local_extern "\204\021\267$r m\031m\013\322e\016t\245\
-\177v\235 \343\022\013\025"    /* "extern $r mismatches top-level declaration" */
#define fp_rerr_very_small "sm\014l \337\311\002\351\004\217\325t\020\035\
0.0"    /* "small floating point value converted to 0.0" */
#define fp_rerr_small_single \
        "sm\014l \050s\017\260p\007c\031\025\332\337\024\351\004\217\325\
t\020\0350.0"    /* "small (single precision) floating value converted to 0.0" */
#define pp_rerr_newline_eof "m\031\316newl\001\004beW\004EOF -7s\003t\205"    /* "missing newline before EOF - inserted" */
#define pp_rerr_nonprint_char "\027pr\276\265\260\322\022 %\043.2x f\347\
\005\216\326"    /* "unprintable char %#.2x found - ignored" */
#define pp_rerr_illegal_option "\274\245t\176-D%s%s"    /* "illegal option -D%s%s" */
#define pp_rerr_spurious_else "sp\374i\206\016\043\235s\004\326"    /* "spurious #else ignored" */
#define pp_rerr_spurious_elif "sp\374i\206\016\043\235i\032\326"    /* "spurious #elif ignored" */
#define pp_rerr_spurious_endif "sp\374i\206\016\043\023\334\032\326"    /* "spurious #endif ignored" */
#define pp_rerr_hash_line "n\227b\240m\031\316\250\043l\001e"    /* "number missing in #line" */
#define pp_rerr_hash_error "\043\003r\231\023\223\027\021\020\"%s\""    /* "#error encountered \"%s\"" */
#define pp_rerr_hash_ident "\043\304\356\266\200\250ANSI C"    /* "#ident is not in ANSI C" */
#define pp_rerr_junk_eol "j\320 \345\023\005\140\043%s l\001\004\216\326"    /* "junk at end of #%s line - ignored" */
#define sem_rerr_sizeof_bitfield \
        "size\140\074b\257\323\235d\300\274\216sizeof\050\276\332\277\227\
\205"    /* "sizeof <bit field> illegal - sizeof(int) assumed" */
#define sem_rerr_sizeof_void "siz\004\140\'\375\304\201\007qui\007\005\216\
t\007\330\2531"    /* "size of 'void' required - treated as 1" */
#define sem_rerr_sizeof_array "siz\004\140\212\133\135 \022r\367\007qui\
\302\357t\007\330\253\1331\135"    /* "size of a [] array required, treated as [1]" */
#define sem_rerr_sizeof_function \
        "siz\004\140\327\007qui\007\005\216t\007\330\253siz\004\140\376"    /* "size of function required - treated as size of pointer" */
#define sem_rerr_pointer_arith \
        "\074\276\300$s \074\376\300t\007\330\253\074\276\300$s \050\276\
\051\074\376\076"    /* "<int> $s <pointer> treated as <int> $s (int)<pointer>" */
#define sem_rerr_pointer_arith1 \
        "\074\376\300$s \074\276\300t\007\330\253\050\276\051\074\376\300\
$s \074\276\076"    /* "<pointer> $s <int> treated as (int)<pointer> $s <int>" */
#define sem_rerr_assign_const "\277\214nm\356\035\'\217\100\201\373ec\002\
$e"    /* "assignment to 'const' object $e" */
#define sem_rerr_addr_regvar \
        "\'\340\021\201\013\224i\333t\004\247$b \214\026\007\005wh\023 \
\275\016tak\023"    /* "'register' attribute for $b ignored when address taken" */
#define sem_rerr_lcast "\373eQ\016\262\345hav\004be\023 \313\002\022\004\
\200l-\351es"    /* "objects that have been cast are not l-values" */
#define sem_rerr_pointer_compare \
"\223\233\022\031\006 $s \140\215\210\033\005\276:\n\364li\021\2030 \050\
\247\075\075 \033\005\041\075\332\266\006l\202\222\203\313e"    /* "comparison $s of pointer and int:\n\
  literal 0 (for == and !=) is only legal case" */
#define sem_rerr_different_pointers "\334\272\003\311\210\225\174s\030$\
s"    /* "differing pointer types: $s" */
#define sem_rerr_wrong_no_args "wr\006g n\227b\240\140\362\243e\021\016\
\035$e"    /* "wrong number of parameters to $e" */
#define sem_rerr_casttoenum "$s\030\313\354$m \035\334\272\003\024\023\227"    /* "$s: cast of $m to differing enum" */ /* warn in C */
#define sem_rerr_valcasttoref "$s\030\261l\351\004\313\002\035\261\217s\
\002\007f\003\023ce"    /* "$s: non-lvalue cast to non-const reference" */
#define sem_rerr_implicit_cast1 \
        "$s\030i\233l\213\257\313\354\215\210\035\261equ\203\376"    /* "$s: implicit cast of pointer to non-equal pointer" */
#define sem_rerr_implicit_cast2 "$s\030i\233l\213\257\313\354\26107\002\
\035\376"    /* "$s: implicit cast of non-0 int to pointer" */
#define sem_rerr_implicit_cast3 "$s\030i\233l\213\257\313\354\215\210\035\'\
\276\'"    /* "$s: implicit cast of pointer to 'int'" */
#define sem_rerr_implicit_cast4 "$s\030i\233l\213\257\313\354$t \035\'\276\'"    /* "$s: implicit cast of $t to 'int'" */
#define sem_rerr_nonpublic "$r \266\261publ\336m\301b\240\140$c"    /* "$r is non-public member of $c" */
#define sem_rerr_cant_balance "\334\272\003\311\210\225\174s\030$s"    /* "differing pointer types: $s" */

#define sem_rerr_void_indirection "\234\0147\334\007Q\176\006 \050\375\361\
\052\051\030\'\052\'"    /* "illegal indirection on (void *): '*'" */
#define obj_fatalerr_io_object "I\057O \003r\231\006 \373ec\002\100\007\
\243"    /* "I/O error on object stream" */
#define compiler_rerr_no_extern_decl\
    "n\015\204\021n\203\343\022\013\025\252\224\033sl\013\176\027\034"    /* "no external declaration in translation unit" */
#define compiler_fatalerr_io_error "I\057O \003r\231wr\034\024\'%s\'"    /* "I/O error writing '%s'" */
#define driver_fatalerr_io_object "I\057O \003r\231\006 \373ec\002\100\007\
\243"    /* "I/O error on object stream" */
#define driver_fatalerr_io_asm "I\057O \003r\231\006 \037\220mbl\240\206\
tpu\002\100\007\243"    /* "I/O error on assembler output stream" */
#define driver_fatalerr_io_listing "I\057O \003r\231\006 l\031t\024\100\
\007\243"    /* "I/O error on listing stream" */
#ifdef TARGET_HAS_AOUT
#define aout_fatalerr_toomany 	"T\372m\033\202symb\236\016\247\'a.\206t\
\201\206tput"    /* "Too many symbols for 'a.out' output" */
#define aout_fatalerr_toobig	"Modu\260t\372b\214 \247a.\206\002\264\013\
\021"    /* "Module too big for a.out formatter" */
#endif
#ifdef TARGET_HAS_COFF
#define coff_fatalerr_toomany 	"T\372m\033\202\007\365\013\025\016\247C\
OFF \264\345\250.\015f5e"    /* "Too many relocations for COFF format in .o file" */
#define coff_fatalerr_toobig	"Modu\260t\372b\214 \247COFF \264\013\021"    /* "Module too big for COFF formatter" */
#endif
#ifdef TARGET_IS_HELIOS
#define heliobj_warn_12bits       "O\272\305%ld \30012 b\034s"    /* "Offset %ld > 12 bits" */
#define heliobj_warn_16bits       "O\272\305%ld \30016 b\034s"    /* "Offset %ld > 16 bits" */
#define heliobj_warn_24bits       "O\272\305%ld \30024 b\034s"    /* "Offset %ld > 24 bits" */
#define heliobj_misplaced_offsets "h\235i\373\030symb\236 %s h\253o\272\
\305%ld \033\005f\236\136w\016symb\236 %s \271\221o\272\305%ld"    /* "heliobj: symbol %s has offset %ld and follows symbol %s with offset %ld" */
#define heliobj_repeated_symbol   "h\235i\373\030c\033\200\050yet\332\007\
\174\013\205l\202\204p\010\002\262\004s\243\004symb\236"    /* "heliobj: cannot (yet) repeatedly export the same symbol" */
#define heliobj_bad_fp_number     "h\235i\373\030ba\005siz\004\140\337\311\
\002n\227b\240%ld"    /* "heliobj: bad size of floating point number %ld" */
#define heliobj_unknown_data_type "h\235i\373\030\363\267\360\212\306%l\
x"    /* "heliobj: unknown data type %lx" */
#define heliobj_bad_packed_length "h\235i\373\030ba\005l\023gt\221\247p\
\226k\020\360\212\050%ld\051"    /* "heliobj: bad length for packed data (%ld)" */
#define syserr_heliobj_bad_xref	  "In\273\361\204\021n\203\007f\003\023\
c\004$r %\043lx"    /* "Invalid external reference $r %#lx" */
#define syserr_heliobj_dataseggen "D\013\212\220g g\023\003\013\176\217\
f\237\205"    /* "Data seg generation confused" */
#define syserr_heliobj_gendata	  "\373\137g\023\360a\050%ld\051"    /* "obj_gendata(%ld)" */
#define syserr_heliobj_datalen	  "\373\137\360\212l\023\075%ld"    /* "obj_data len=%ld" */
#define syserr_heliobj_data	  "\373\137\360\212%ldEL%ld\'%s\'"    /* "obj_data %ldEL%ld'%s'" */
#define syserr_heliobj_align	  "H\235io\016\373 \014\214n"    /* "Helios obj align" */
#define syserr_heliobj_2def	  "d\206b\260\246\034\025\252\373\137sym\007\
\032$r"    /* "double definition in obj_symref $r" */
#define syserr_heliobj_codedata	  "\223\036\057\360\212\217f\237\176\247\
$r"    /* "code/data confusion for $r" */
#endif

#define misc_fatalerr_space1 "\206\354\100\010\004\050\247\003r\231\333\
\272\003\051"    /* "out of store (for error buffer)" */
#define misc_fatalerr_toomanyerrs "T\372m\033\202\003r\010s"    /* "Too many errors" */
#define misc_fatalerr_space2 "\206\354\100\010\004\050\250cc\137\014\365\
\051\n\050Co\2335\013\176\140\262\004\036\333gg\024t\265\177\016\007que\
\100\020\271\221\262\004-g \245t\025\n \007qui\007\016\212g\007\345\036\
\203\140m\301\010y. Re\223\2335\024\271h\206\002-g\357\271h\n \262\004m\
\010\004\007\321i\344-g\032\245t\025\357\231\271\221\262\004p\350gr\243 \
b\350k\0237to\n sm\014l\240pieces\357m\367h\235p.\051"    /* "out of store (in cc_alloc)\n\
(Compilation of the debugging tables requested with the -g option\n\
 requires a great deal of memory. Recompiling without -g, with\n\
 the more restricted -gf option, or with the program broken into\n\
 smaller pieces, may help.)" */
#define misc_fatalerr_space3 "\206\354\100\010\004\050\250cc\137\014\365\
\051"    /* "out of store (in cc_alloc)" */
#define pp_fatalerr_hash_error "\043\003r\231\023\223\027\021\020\"%s\""    /* "#error encountered \"%s\"" */

#define driver_message_nolisting \
        "Un\265\260\035\245\023 %s \247l\031t\017\030-l \245t\176\326\n"    /* "Unable to open %s for listing: -l option ignored\n" */
#ifdef NO_ASSEMBLER_OUTPUT
#define driver_message_noasm \
        "Th\266\325s\176\140\262\004\223\2335\240doe\016\200supp\010\002\
-s\n"    /* "This version of the compiler does not support -s\n" */
#endif
#define driver_message_writefail "C\206ldn\'\002wr\034\004f5\004\'%s\'\n"    /* "Couldn't write file '%s'\n" */
#define driver_message_oddoption "Un\007\223gniz\020\245t\176\'%c\'\030\
\326\n"    /* "Unrecognized option '%c': ignored\n" */
#define driver_message_readfail "C\206ldn\'\002\007a\005f5\004\'%s\'\n"    /* "Couldn't read file '%s'\n" */
/* NB the next error can not arise with the current ARM driver */
#define driver_message_toomanyfiles "T\372m\033\202f5\004\317s"    /* "Too many file args" */
#define driver_message_asmstdout "As\220mbl\202\223\371w5l g\015\035\100\
d\206t\n"    /* "Assembly code will go to stdout\n" */
#define driver_message_no_listing \
        "-m \245t\176u\220\177s\016\271h\206\002s\206rc\004l\031t\017. \
Ig\312\n"    /* "-m option useless without source listing. Ignored\n" */
#define driver_message_nomap "-m f5\004\200ava5\265\260\231c\010rup\002\
\216\326\n"    /* "-m file not available or corrupt - ignored\n" */
#define driver_message_notest \
        "Th\266\325s\176\140\262\004\223\2335\240doe\016\200supp\010\002\
\262\004-tes\002\245t\025\n"    /* "This version of the compiler does not support the -test option\n" */
#define driver_message_needfile "A\002\177\037\002\006\004f5\004\317\227\
\356w\033t\205\n"    /* "At least one file argument wanted\n" */
#ifndef COMPILING_ON_ARM_OS
#define driver_message_spool "\206tpu\002\035c\136g1.\136g \046 c\136g2\
.\136g\n"    /* "output to clog1.log & clog2.log\n" */
#endif
#define driver_message_testfile "N\015f5e\016\014\136w\020\271\221-te\100\n"    /* "No files allowed with -test\n" */
/* messages generated by misc.c */

#ifndef TARGET_IS_UNIX
#ifndef COMPILING_ON_MPW
#define misc_message_lineno(_f,_l,_s) "\"%s\"\357l\001\004%ld\030%s\030"    /* "\"%s\", line %ld: %s: " */,_f,_l,_s
#else
#define misc_message_lineno(_f,_l,_s) "F5\004\"%s\"\073 L\001\004%ld \043 \
%s\030"    /* "File \"%s\"; Line %ld # %s: " */,_f,_l,_s
#endif
#else
#define misc_message_lineno(_f,_l,_s) "%s\030%ld\030%s\030"    /* "%s: %ld: %s: " */,_f,_l,_s
#endif
#ifndef COMPILING_ON_MPW
#define misc_message_sum1(_f,nx,neq1) "%s\030%ld w\022n\017%s"    /* "%s: %ld warning%s" */, _f, nx, \
 neq1 ? "":"s"

#else
#define misc_message_sum1(_f,nx,neq1) "\043\043\043 \"%s\"\030%ld w\022\
n\017%s"    /* "### \"%s\": %ld warning%s" */, _f, nx, \
 neq1 ? "":"s"

#endif
#define misc_message_sum2 " \050\053 %ld supp\241s\205\051"    /* " (+ %ld suppressed)" */
#define misc_message_sum3(nx,neq1) "\357%ld \003r\010%s"    /* ", %ld error%s" */, nx, \
 neq1 ? "":"s"

#define misc_message_sum5(nx,neq1) "\357%ld s\003i\206\016\003r\010%s\n"    /* ", %ld serious error%s\n" */, nx, \
 neq1 ? "":"s"


#ifdef TARGET_STACK_MOVES_ONCE
/* Cannot be issued if NARGREGS==0 */
#define warn_untrustable "\027\224u\100\265\260\223\371g\023\003\330\247\
$r"    /* "untrustable code generated for $r" */
#endif

 /* The next batch of things just get mapped onto syserr codes */

#define syserr_removepostincs "unexpected op in RemovePostIncs"
#define syserr_mkqualifiedtype "mkqualifiedtype(..., %ld)"
#define syserr_unbitfield "unbitfield_type $t"
#define syserr_bf_promote "bf_promoted_type $t"
#define syserr_typeof "typeof(%ld)"
#define syserr_alignoftype "alignoftype(%ld,%#lx)"
#define syserr_sizeoftype "sizeoftype(%ld,%#lx)"
#define syserr_codeoftype "codeoftype"
#define syserr_equivtype "equivtype(%ld)"
#define syserr_compositetype "compositetype(%ld)"
#define syserr_trydiadicreduce "trydiadreduce(unsigned op %ld)"
#define syserr_trydiadicreduce1 "trydiadreduce(signed op %ld)"
#define syserr_trydiadicreduce2 "trydiadreduce(float op %ld)"
#define syserr_fp_op "FP op %ld unknown"
#define syserr_trymonadicreduce "trymonadreduce(int op %ld)"
#define syserr_trymonadicreduce1 "trymonadreduce(float op %ld)"
#define syserr_bf_container "bf_container"
#define syserr_coerceunary1 "coerceunary(%ld,%#lx)"
#define syserr_bitfieldassign "bitfieldassign"
#define syserr_mkindex "sem(mkindex)"
#define syserr_ptrdiff "sem(mkbinary/ptrdiff)"
#define syserr_va_arg_fn "sem(odd va_arg fn)"
#define syserr_mkcast "mkcast(%ld,%#lx)"
#define syserr_mkcast1 "mkcast(%ld)"
#define syserr_te_plain "te_plain(%ld)"
#define syserr_clone_node "clone_node(%ld)"
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
#define syserr_insertblockbetween "insertblockbetween(%ld, %ld)"
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
#define syserr_modifycode_2 "compare ref L%ld not reachable from def L%ld"
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
#define syserr_cg_narrowformal "unwidened formal"

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


      /* The following remain as ordinary (uncompressed) strings */

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

#ifdef TARGET_HAS_ELF
#define syserr_elf_reloc "relocate_code_to_data(PCreloc)"
#define syserr_elf_pcrel "elf(unexpected X_PCreloc)"
#define syserr_elf_m88000 "elfobj(X_DataAddr needs extending)"
#define syserr_elf_toobig "elfobj(Module over 64K -- fix)"
#define syserr_elf_checksym "obj_checksym($r)"
#define syserr_elf_reloc1 "obj_coderelocation(%.8lx)"
#define syserr_elf_gendata "obj_gendata(%ld)"
#define syserr_elf_datalen "obj_data len=%ld"
#define syserr_elf_data "obj_data %ldEL%ld'%s'"
#endif

/* end of miperrs.h */
/*
 * cfe/feerrs.h - prototype for front-end error messages file
 * version 3a.
 */

  /* Ordinary error messages - mapped onto numeric codes */

#define lex_warn_force_unsigned "%s t\007\330\253%sul\25232-b\257i\233\177\
m\307\013\025"    /* "%s treated as %sul in 32-bit implementation" */
#define lex_warn_multi_char "\261p\010t\265\260\216\2001 \322\022\252\'\
\251.\'"    /* "non-portable - not 1 char in '...'" */
#define lex_warn_cplusplusid "C\053\053 keyw\010\005\237\020\253\304\307\
i\323\355$r"    /* "C++ keyword used as identifier: $r" */

#define syn_warn_hashif_undef "Un\246\020m\226r\015\'%s\'\252\043i\032\216\
t\007\330\2530"    /* "Undefined macro '%s' in #if - treated as 0" */
#define syn_warn_invent_extern "\001v\307\024\'\204\021n7\002%s\050\051\
\073\'"    /* "inventing 'extern int %s();'" */
#define syn_warn_unary_plus "Un\022\202\'\053\201\266\212fe\013\374\004\
\140ANSI C"    /* "Unary '+' is a feature of ANSI C" */
#define syn_warn_spurious_braces "sp\374i\206\016\173\175 \022\347\005s\
c\014\0227\353\003"    /* "spurious {} around scalar initialiser" */
#define syn_warn_dangling_else "D\033gl\024\'\235\220\'7d\213\013e\016\207\
ssib\260\003r\010"    /* "Dangling 'else' indicates possible error" */
#define syn_warn_void_return "\261\351\004\007t\374n\252\261\375\361\256\
\025"    /* "non-value return in non-void function" */
#define syn_warn_use_of_short \
        "\'sh\010t\201s\136w\240\262\033 \'\276\201\006 \262\266m\226h\001\
\004\050\220\004m\033u\014\051"    /* "'short' slower than 'int' on this machine (see manual)" */
#define syn_warn_undeclared_parm \
        "\264\203\362\243e\210$r \200\343a\007\005\216\'\276\201\277\227\
\205"    /* "formal parameter $r not declared - 'int' assumed" */
#define syn_warn_old_style "Old-\100y\260\327$r"    /* "Old-style function $r" */
#define syn_warn_give_args "D\314\007c\330\343\022\013\176%s\050\332\216\
giv\004\317 \225\174s"    /* "Deprecated declaration %s() - give arg types" */
#define syn_warn_ANSI_decl "ANSI \100y\260\327\343\022\013\176\237\205\357\'\
%s\050\251.\051\'"    /* "ANSI style function declaration used, '%s(...)'" */
#define syn_warn_archaic_init "Anci\356\264 \140\001\353\013\025\357\237\
\004\'\075\'"    /* "Ancient form of initialisation, use '='" */
#define syn_warn_untyped_fn "\'\001\002%s\050\051\201\277\227\020\216\'\
\375\304\'7t\023\036d\077"    /* "'int %s()' assumed - 'void' intended?" */
#define syn_warn_no_named_member "$c h\253n\015n\243\020m\301b\003"    /* "$c has no named member" */
#define syn_warn_extra_comma "Sup\003flu\206\016\',\'\252\'\023\227\201\
\343\022\013\025"    /* "Superfluous ',' in 'enum' declaration" */
#define syn_warn_struct_padded "p\263\024\001s\003t\020\250\321uc\002$b"    /* "padding inserted in struct $b" */

#define vargen_warn_nonull "om\034t\024\224a5\024\'\\0\201\247%s \133%l\
d\135"    /* "omitting trailing '\\0' for %s [%ld]" */
#define vargen_warn_unnamed_bitfield \
        "Unn\243\020b\257\323\235\005\001\353\020\0350"    /* "Unnamed bit field initialised to 0" */
#define vargen_warn_init_non_aggregate \
        "Atte\233\002\035\001\353\004\261agg\303\013e"    /* "Attempt to initialise non-aggregate" */

#define lex_err_ioverflow "N\227b\240%s t\372l\317\004\24732-b\257i\233\
\177m\307\013\025"    /* "Number %s too large for 32-bit implementation" */
#define lex_err_overlong_fp "G\350ssl\202o\325-l\006g \337\311\002n\227\
b\003"    /* "Grossly over-long floating point number" */
#define lex_err_fp_syntax1 "D\214\257\007qui\007\005\366\210\204p\006\356\
m\022k\003"    /* "Digit required after exponent marker" */
#define lex_err_overlong_hex "G\350ssl\202o\325-l\006g h\204a\036cim\203\
\217\100\033t"    /* "Grossly over-long hexadecimal constant" */
#define lex_err_overlong_int "G\350ssl\202o\325-l\006g n\227b\003"    /* "Grossly over-long number" */
#define lex_err_need_hex_dig "H\204 d\214\257ne\205\020\366\2100x \2310\
X"    /* "Hex digit needed after 0x or 0X" */
#define lex_err_need_hex_dig1 "M\031\316h\204 d\214\034\050s\332\366\210\\\
x"    /* "Missing hex digit(s) after \\x" */
#define lex_err_backslash_blank \
        "\\\074sp\226e\300\033\005\\\074t\265\300\022\004\001\273\361\321\
\024esca\174s"    /* "\\<space> and \\<tab> are invalid string escapes" */
#define lex_err_unterminated_string "Newl\001\004\231\023\005\140f5\004\
\271h\250\321\017"    /* "Newline or end of file within string" */
#define lex_err_bad_hash "m\031\315\226\020p\007p\350cess\231\322\022\226\
\210\'%c\'"    /* "misplaced preprocessor character '%c'" */
#define lex_err_bad_char "\274\322\022\226\210\0500x%lx \075 \'%c\'\051\
\252s\206rce"    /* "illegal character (0x%lx = \'%c\') in source" */
#define lex_err_bad_noprint_char "\274\322\022\226\210\050h\204 \223\371\
0x%x\051\252s\206rce"    /* "illegal character (hex code 0x%x) in source" */
#define lex_err_ellipsis "\050\251.\332m\237\002hav\004\204aQl\2023 dot\
s"    /* "(...) must have exactly 3 dots" */
#define lex_err_illegal_whitespace "$s m\367\200hav\004wh\034esp\226\004\
\250\034"    /* "$s may not have whitespace in it" */

#define syn_err_bitsize "b\257siz\004%ld \274\2161 \277\227\205"    /* "bit size %ld illegal - 1 assumed" */
#define syn_err_zerobitsize "z\003\015w\304t\221n\243\020b\257\323\235\005\
\2161 \277\227\205"    /* "zero width named bit field - 1 assumed" */
#define syn_err_arraysize "Arr\367siz\004%ld \274\2161 \277\227\205"    /* "Array size %ld illegal - 1 assumed" */
#define syn_err_expected "\204\174\344$s -7s\003t\020beW\004$l"    /* "expected $s - inserted before $l" */
#define syn_err_expected1 "\204\174\344$s%s -7s\003t\020beW\004$l"    /* "expected $s%s - inserted before $l" */
#define syn_err_expected2 "\204\174\344$s \231$s -7s\003t\020$s beW\004\
$l"    /* "expected $s or $s - inserted $s before $l" */
#define syn_err_expecteda "\204\174\344$s"    /* "expected $s" */
#define syn_err_expected1a "\204\174\344$s%s"    /* "expected $s%s" */
#define syn_err_expected2a "\204\174\344$s \231$s"    /* "expected $s or $s" */
#define syn_err_mix_strings "\322\022 \033\005w\304\004\050L\"\251.\"\332\
\321\017\016d\015\200\217c\013\023\013e"    /* "char and wide (L\"...\") strings do not concatenate" */
#define syn_err_expected_expr "\074\204p\241s\025\300\204\174\344\333\002\
f\347\005$l"    /* "<expression> expected but found $l" */
#ifdef EXTENSION_VALOF
#define syn_err_valof_block \
        "\173 f\236\136w\024\212\313\002w5l b\004t\007\330\253VALOF b\365\
k"    /* "{ following a cast will be treated as VALOF block" */
#endif
#define syn_err_typedef "\225\174\036\032n\243\004$r \237\020\250\204p\241\
s\176\217t\204t"    /* "typedef name $r used in expression context" */
#define syn_err_assertion "\137\137\137\277\003t\0500\357$e\051"    /* "___assert(0, $e)" */
#define syn_err_expected_member "Ex\174\344\074m\301b\003\300\333\002f\347\
\005$l"    /* "Expected <member> but found $l" */
#define syn_err_hashif_eof "EOF \200newl\001\004\366\210\043i\032\251."    /* "EOF not newline after #if ..." */
#define syn_err_hashif_junk "J\320 \366\210\043i\032\074\204p\241s\025\076"    /* "Junk after #if <expression>" */
#define syn_err_initialisers "t\372m\033y7\353\003\016\250\173\175 \247\
agg\303\013e"    /* "too many initialisers in {} for aggregate" */
#define syn_err_initialisers1 "\173\175 m\237\002hav\0041 e\177m\356\035\
\001\353\004sc\014\022"    /* "{} must have 1 element to initialise scalar" */
#define syn_err_default "\'\230ault\201\200\250s\271c\221\216\326"    /* "'default' not in switch - ignored" */
#define syn_err_default1 "du\315\213\341\'\230ault\201\313\004\326"    /* "duplicate 'default' case ignored" */
#define syn_err_case "\'\313e\201\200\250s\271c\221\216\326"    /* "'case' not in switch - ignored" */
#define syn_err_case1 "du\315\213\330\313\004\217\100\033t\030%ld"    /* "duplicated case constant: %ld" */
#define syn_err_expected_cmd "\074\223mm\033d\300\204\174\344\333\002f\347\
\005$l"    /* "<command> expected but found $l" */
#define syn_err_expected_while "\'wh5e\201\204\174\344\366\210\'do\201\333\
\002f\347\005$l"    /* "'while' expected after 'do' but found $l" */
#define syn_err_else "M\031\315\226\020\'\235\220\201\326"    /* "Misplaced 'else' ignored" */
#define syn_err_continue "\'\217t\001ue\201\200\250\136\245 \216\326"    /* "'continue' not in loop - ignored" */
#define syn_err_break "\'b\007ak\201\200\250\136\245 \231s\271c\221\216\
\326"    /* "'break' not in loop or switch - ignored" */
#define syn_err_no_label "\'goto\201\200f\236\136w\020b\202l\265\235 \216\
\326"    /* "'goto' not followed by label - ignored" */
#define syn_err_no_brace "\'\173\201\140\327bod\202\204\174\344\216f\347\
\005$l"    /* "'{' of function body expected - found $l" */
#define syn_err_stgclass \
        "\100\010ag\004\244\037\016$s \200p\003m\034t\020\250\217t\204\002\
%s \216\326"    /* "storage class $s not permitted in context %s - ignored" */
#define syn_err_stgclass1 "\100\010ag\004\244\037\016$s7\223\233\013ib\260\
\271\221$m \216\326"    /* "storage class $s incompatible with $m - ignored" */
#define syn_err_typeclash "\306$s7\217s\031t\356\271\221$m"    /* "type $s inconsistent with $m" */
#define syn_err_tag_brace \
        "\'\173\201\231\074\304\307i\323\003\300\204\174\344\366\210$s \
\333\002f\347\005$l"    /* "'{' or <identifier> expected after $s but found $l" */
#define syn_err_expected3 "Ex\174Q\024\074\343\022\013\010\300\231\074\225\
\174\300\333\002f\347\005$l"    /* "Expecting <declarator> or <type> but found $l" */
#define syn_err_unneeded_id \
        "sup\003flu\206\016$l\252\074\265\321\226\002\343\022\013\010\300\
\216\326"    /* "superfluous $l in <abstract declarator> - ignored" */
#define syn_err_undef_struct(_m,_b,_s) \
        "\027\246\020$c %s\030$r"    /* "undefined $c %s: $r" */, _b, errs_membobj(_m), _s
#define syn_err_selfdef_struct(_m,_b,_s) \
        "\013te\233\002\035\001\244u\371$c %s\030$r \271h\250\034\220lf"    /* "attempt to include $c %s: $r within itself" */, \
        _b, errs_membobj(_m), _s
#define syn_err_void_object(_m,_s) "\274\'\375\304\201%s\030$r"    /* "illegal 'void' %s: $r" */, errs_membobj(_m), _s
#define syn_err_duplicate_type \
        "du\315\213\341\306s\174cif\213\013\176\140\264\203\362\243e\210\
$r"    /* "duplicate type specification of formal parameter $r" */
#define syn_err_not_a_formal "N\006-\264\203$r\252\362\243e\021-\225\174\
-s\174ci\323\003"    /* "Non-formal $r in parameter-type-specifier" */
#define syn_err_cant_init "$m v\022i\265\177\016m\367\200b\004\001\353\205"    /* "$m variables may not be initialised" */
#define syn_err_enumdef \
        "\074\304\307i\323\003\300\204\174\344\333\002f\347\005$l\252\'\
\023\227\201\246\034\025"    /* "<identifier> expected but found $l in 'enum' definition" */
#define syn_err_undef_enum "Un\246\020\023\227 $r"    /* "Undefined enum $r" */
#define syn_err_misplaced_brace "M\031\315\226\020\'\173\201\345t\245 \177\
v\235 \216\214n\010\024b\365k"    /* "Misplaced '{' at top level - ignoring block" */

#define vargen_err_long_string "\321\024\001\353\240l\006g\240\262\033 \
%s \133%ld\135"    /* "string initialiser longer than %s [%ld]" */
#define vargen_err_nonstatic_addr \
        "\261\100\013\336\275\016$b\252\3767\353\003"    /* "non-static address $b in pointer initialiser" */
#define vargen_err_bad_ptr "$s\030\274\237\004\250\3767\353\003"    /* "$s: illegal use in pointer initialiser" */
#define vargen_err_init_void "\373eQ\016\140\306\'\375\304\201c\033 \200\
b\004\001\353\205"    /* "objects of type 'void' can not be initialised" */
#define vargen_err_undefined_struct \
        "$c m\237\002b\004\246\020\247\050\100\013\213\332v\022i\265\260\
\343\022\013\025"    /* "$c must be defined for (static) variable declaration" */
#define vargen_err_open_array "Un\001\353\020\100\013\336\133\135 \022r\
ay\016\234\014"    /* "Uninitialised static [] arrays illegal" */
#define vargen_err_overlarge_reg "g\136b\203\340\210n\227b\240t\372l\317\
e"    /* "global register number too large" */
#define vargen_err_not_int "\001\273\361\306\247g\136b\0147\002\340\021"    /* "invalid type for global int register" */
#define vargen_err_not_float "\001\273\361\306\247g\136b\203\255\345\340\
\021"    /* "invalid type for global float register" */
#ifdef TARGET_CALL_USES_DESCRIPTOR
#define vargen_err_badinit "\234\0147\353\013\176\035$r%\053ld"    /* "illegal initialisation to $r%+ld" */
#endif
#ifdef TARGET_IS_HELIOS
#define vg_err_dynamicinit "In\353\020dyn\243\336\022r\367\271\221-ZR \231\
-ZL"    /* "Initialised dynamic array with -ZR or -ZL" */
#endif
#define vargen_rerr_nonaligned \
        "N\006-\014\214n\020ADCON \345\360a\0530x%lx \050\351\004$r\053\
0x%lx\332\305\035NULL"    /* "Non-aligned ADCON at data+0x%lx (value $r+0x%lx) set to NULL" */
#define vargen_rerr_datadata_reloc \
       "RISC OS \050\231o\262\003\332\007\023\224\033\002modu\260h\253\100\013\
\2137\034. \035\360\212$r"    /* "RISC OS (or other) reentrant module has static init. to data $r" */

#define lex_rerr_8_or_9 "d\214\2578 \2319 f\347\005\250oQ\203n\227b\003"    /* "digit 8 or 9 found in octal number" */
#define lex_rerr_pp_number "n\227b\240\234\014l\202f\236\136w\020b\202\177\
t\021"    /* "number illegally followed by letter" */
#define lex_rerr_hex_exponent "h\204 n\227b\240c\033\200hav\004\204p\006\
\307"    /* "hex number cannot have exponent" */
#define lex_rerr_esc16_truncated \
        "o\325l\317\004esca\270\'\\x%s%lx\201t\007\330\253\'\\x%lx\'"    /* "overlarge escape '\\x%s%lx' treated as '\\x%lx'" */
#define lex_rerr_esc8_truncated "o\325l\317\004esca\270\'\\%o\201t\007\330\
\253\'\\%o\'"    /* "overlarge escape '\\%o' treated as '\\%o'" */
#define lex_rerr_illegal_esc "\274\321\024esca\270\'\\%c\201\216t\007\330\
\253%c"    /* "illegal string escape '\\%c' - treated as %c" */
#define lex_rerr_not1wchar "L\'\251.\201ne\205\016\204aQl\2021 w\304\004\
\322\022\226\021"    /* "L'...' needs exactly 1 wide character" */
#define lex_rerr_empty_char "n\015\322\022\016\250\322\022\226\210\217\100\
\033\002\'\'"    /* "no chars in character constant ''" */
#define lex_rerr_overlong_char "m\010\004\262\033 4 \322\022\016\250\322\
\022\226\210\217\100\033t"    /* "more than 4 chars in character constant" */

#define syn_rerr_array_0 "\022r\367\1330\135 f\347d"    /* "array [0] found" */
#ifdef EXTENSION_VALOF
#define syn_rerr_void_valof "\375\361\273\140b\365k\016\022\004\200p\003\
m\034t\205"    /* "void valof blocks are not permitted" */
#endif
#define syn_rerr_undeclared "\027\343a\007\005n\243e,7v\307\024\'\204\021\
n7\002%s\'"    /* "undeclared name, inventing 'extern int %s'" */
#define syn_rerr_insert_parens \
        "pa\007n\262e\220\016\050\251\0517s\003t\020\022\347\005\204p\241\
s\176f\236\136w\024$s"    /* "parentheses (..) inserted around expression following $s" */
#define syn_rerr_return "\007t\374\267\074\204pr\300\274\247\375\361\256\
\025"    /* "return <expr> illegal for void function" */
#define syn_rerr_qualified_typedef(_b,_m) \
        "$m \225\174\036\032$b h\253$m \007-s\174ci\323\205"    /* "$m typedef $b has $m re-specified" */, _m, _b, _m
#define syn_rerr_missing_type "m\031\316\306s\174cif\213\013\176\216\'\276\
\201\277\227\205"    /* "missing type specification - 'int' assumed" */
#define syn_rerr_long_float "ANSI C doe\016\200supp\010\002\'l\006g \337\'"    /* "ANSI C does not support 'long float'" */
#define syn_rerr_missing_type1 \
        "om\034t\020\074\225\174\300beW\004\264\203\343\022\013\231\216\'\
\276\201\277\227\205"    /* "omitted <type> before formal declarator - 'int' assumed" */
#define syn_rerr_missing_type2 \
        "\327p\350to\306\264\203$r ne\205\016\306\231\244\037\016\216\'\
\276\201\277\227\205"    /* "function prototype formal $r needs type or class - 'int' assumed" */
#define syn_rerr_ellipsis_first "\235lips\266\050\251.\332c\033\200b\004\
\006l\202\362\243e\021"    /* "ellipsis (...) cannot be only parameter" */
#define syn_rerr_mixed_formals "p\350to\306\033\005\236d-\100y\260\362\243\
e\021\016mix\205"    /* "prototype and old-style parameters mixed" */
#define syn_rerr_open_member "\274\133\135 m\301b\355$r"    /* "illegal [] member: $r" */
#define syn_rerr_ref_void "\274\306\050\375\361\046\332t\007\330\253\050\
\001\002\046\051"    /* "illegal type (void &) treated as (int &)" */
#define syn_rerr_ill_ref "$t \140\007f\003\023c\004\274-\216\'\046\201\326"    /* "$t of reference illegal -- '&' ignored" */
#define syn_rerr_fn_returntype "\327\007t\374n\024$t \274-\216\277\227\311\
\021"    /* "function returning $t illegal -- assuming pointer" */
#define syn_rerr_array_elttype "\022r\367\140$t \274-\216\277\227\311\021"    /* "array of $t illegal -- assuming pointer" */
#define syn_rerr_fn_ptr(_m,_s) \
   "%s $r m\367\200b\004\327-\216\277\227\311\021"    /* "%s $r may not be function -- assuming pointer" */, errs_membobj(_m), _s
#define syn_rerr_fn_ptr1 \
        "\327$r m\367\200b\004\001\353\020\216\277\227\024\327\376"    /* "function $r may not be initialised - assuming function pointer" */
#define syn_rerr_archaic_init "Anci\356\264 \140\001\353\013\025\357\237\
\004\'\075\'"    /* "Ancient form of initialisation, use '='" */
#define syn_rerr_bitfield "\274b\257\323\235\005\306$t \216\'\276\201\277\
\227\205"    /* "illegal bit field type $t - 'int' assumed" */
#define syn_rerr_ANSIbitfield "ANSI C Wb\304\016b\257\323\235\005\306$t"    /* "ANSI C forbids bit field type $t" */
#define syn_rerr_missing_formal "\264\203n\243\004m\031\316\250\327\246\
\034\025"    /* "formal name missing in function definition" */
#define syn_rerr_ineffective "\343\022\013\176\271\221n\015e\272eQ"    /* "declaration with no effect" */
#define syn_rerr_duplicate_member(sv,_b) "du\315\213\341m\301b\240$r \140\
$c"    /* "duplicate member $r of $c" */, sv, _b
#define syn_rerr_missing_type3 \
        "\306\231\244\037\016ne\205\020\050\204c\314\002\250\327\246\034\
\025\332\216\'\276\201\277\227\205"    /* "type or class needed (except in function definition) - 'int' assumed" */
#define syn_rerr_semicolon_in_arglist \
        "\',\201\050\200\'\073\'\332\220\362\013e\016\264\203\362\243e\021\
s"    /* "',' (not ';') separates formal parameters" */
#define syn_rerr_no_members "$c h\253n\015m\301b\003s"    /* "$c has no members" */

      /* The following remain as ordinary (uncompressed) strings */

#define syn_moan_hashif "#if <expression>"
#define syn_moan_case "case expression (ignored)"

 /* The next batch of things just get mapped onto syserr codes */

#define syserr_genpointer "genpointer&(%ld)"
#define syserr_initsubstatic "initsubstatic(bit)"
#define syserr_initstatic "initstatic(%ld,%#lx)"
#define syserr_initstatic1 "initstatic(%ld)"
#define syserr_rd_decl_init "rd_decl/init(%#lx)"
#define syserr_rd_typename "rd_typename()=0"
#define syserr_rdinit "syn_rdinit"
#define syserr_rd_declarator "rd_declarator(%ld)"
#define syserr_defaultstgclass "defaultstorageclass(%#x)"
#define syserr_rd_declrhslist "rd_declrhslist confused"
#define syserr_rd_decl2 "rd_decl2(%p,%ld)"
#define syserr_rd_strdecl "rd_strdecl"
#define syserr_lex_string "lex_string"

/* end of cfe/feerrs.h */
/*
 * c40/mcerrs.h - prototype for machine-specific error messages file
 */

  /* Ordinary error messages - mapped onto numeric codes */

/*
 * The following message is always machine specific since it may include
 * information about the source of support for the product.
 */
#define misc_disaster_banner   "\n\377\377\377\377\232\052\n\052 Th\004\
\223\2335\240h\253\036te\344\0337\021n\0147\217s\031t\023cy.\364Th\266c\
\033 occ\374\364\052\n\052 beca\237\004\257h\253r\027 \206\354\212v\034\
\203\241\206rc\004suc\221\253m\301\010\202\231d\031k\364\364 \052\n\052 \
sp\226\004\231beca\237\004\262\003\004\266\212faul\002\250\034.\364I\032\
y\206 c\033\200e\0375\202\014\210 \052\n\052 y\206r p\350gr\243 \035a\375\
\361ca\237\024\262\266r\022\004fa5u\007\357p\177\037\004\217t\226\002y\206\
r\364\052\n\052 \036\014\003.\364Th\004\036\014\240m\367b\004\265\260\035\
h\235p y\206 imm\205i\013\235\202\033\005w5l b\004\364\052\n\052 \265\260\
\035\007p\010\002\212s\237\174\344\223\2335\240faul\002\035\262\004supp\
\010\002c\307\007.\364\364\364\052\n\377\377\377\377\232\052\n\n"    /* "\n\
*************************************************************************\n\
* The compiler has detected an internal inconsistency.  This can occur  *\n\
* because it has run out of a vital resource such as memory or disk     *\n\
* space or because there is a fault in it.  If you cannot easily alter  *\n\
* your program to avoid causing this rare failure, please contact your  *\n\
* dealer.  The dealer may be able to help you immediately and will be   *\n\
* able to report a suspected compiler fault to the support centre.      *\n\
*************************************************************************\n\
\n" */

#define syserr_unknown_indirect_mode    "\363n7\334\007c\002\275\316mo\371\
%lx"    /* "unknown indirect addressing mode %lx" */
#define syserr_destination_not_FP	"\036\100\001\013\176\140FP \245 \050\
%s\332\266\200\033 FP \340\210\050%s\051"    /* "destination of FP op (%s) is not an FP register (%s)" */
#define syserr_bad_source_value		"ba\005\351\004\250s\206rc\004\323\235\
\005\140\340\210\275s\020\245 %s \050%lx\051"    /* "bad value in source field of register addressed op %s (%lx)" */
#define syserr_source_not_FP		"\242 FP s\206rc\004\340\210%s \247\245 %\
s \050%lx\051"    /* "non FP source register %s for op %s (%lx)" */
#define syserr_no_triadic_FP_imm	"\224i\211\336\337\311\002imm\205i\341\
\245\003\013\025\016\022\004im\207ssib\177"    /* "triadic floating point immediate operations are impossible" */
#define syserr_mislaid_a_symbol		"m\031la\361\212symb\236\041"    /* "mislaid a symbol!" */
#define syserr_local_address 		"\365\014\137\275\016%lx"    /* "local_address %lx" */
#define syserr_decode_branch_address	"\036\223\036\137br\033\322\137\275\
\016%.8lx"    /* "decode_branch_address %.8lx" */
#define syserr_unknown_diadic_op	"\363\267\334\211\336\245 \223\371%02l\
x"    /* "unknown diadic op code %02lx" */
#define syserr_unknown_triadic_op	"\363\267\224i\211\336\245 \223\371%0\
2lx"    /* "unknown triadic op code %02lx" */
#define syserr_unknown_triadic_address	"\363\267\264 \140\224i\211\336\275\
s\017,7\321uQ\176\075 %lx"    /* "unknown form of triadic addressing, instruction = %lx" */
#define syserr_unknown_parallel_op	"\363\267\362\014\177l \245 \223\371\
%lx"    /* "unknown parallel op code %lx" */
#define syserr_bad_parallel_addressing	"m\014\264\020\362\014\177l \275\
\316d\031\223\325\205,7\321uQ\176\075 %lx"    /* "malformed parallel addressing discovered, instruction = %lx" */
#define syserr_unknown_op_code		"\363\267\245 \223\371%lx"    /* "unknown op code %lx" */
#define syserr_asmlab 			"od\005\037ml\265\050%lx\051"    /* "odd asmlab(%lx)" */
#define syserr_display_asm 		"d\031\315ay\137\037m\050%lx\051"    /* "display_asm(%lx)" */
#define syserr_asm_trailer 		"\037m\137\224a5\003\050%ld\051"    /* "asm_trailer(%ld)" */
#define syserr_datalen 			"\037m\137\360\212l\023\075%ld"    /* "asm_data len=%ld" */
#define syserr_asm_trailer1 		"\037m\137\224a5\003\050%ldF%ld\051"    /* "asm_trailer(%ldF%ld)" */
#define syserr_asm_trailer2 		"\037m\137\224a5\003\050LIT\137ADCON rpt\051"    /* "asm_trailer(LIT_ADCON rpt)" */
#define syserr_asm_confused 		"As\220mbl\240\206tpu\002\217f\237\020\216\
f\001\005\'\077\'"    /* "Assembler output confused - find '?'" */
#define syserr_unknown_addressing_mode	"Unk\331\267\275\316mo\371\216%x"    /* "Unknown addressing mode - %x" */
#define syserr_not_address_register	"\007l\013iv\004\007f\003\023c\004\262\
r\206g\221\261\275\016\340\210%d"    /* "relative reference through non-address register %d" */
#define syserr_unsupported_branch_type	"\027s\245p\010t\020br\033c\221\306\
%ld"    /* "unsopported branch type %ld" */
#define syserr_bad_block_length		"neg\013iv\004b\365k l\023gt\221%ld"    /* "negative block length %ld" */
#define syserr_large_shift		"\013te\233t\310shif\002b\202\027\007\037\006\
\265l\202l\317\004\243\347\002%d"    /* "attempting to shift by unreasonably large amount %d" */
#define syserr_bad_addressing_mode	"\037k\020\035\3335\005\0337\273\361\
\275\316mo\036"    /* "asked to build an invalid addressing mode" */
#define syserr_bad_arg 			"Ba\005\317 %lx"    /* "Bad arg %lx" */
#define syserr_local_addr 		"\365\014\137\263r"    /* "local_addr" */
#define syserr_firstbit			"w\022n\017\030\013te\233t\310f\001\005\323rs\
\002b\257\305\1400 \041"    /* "warning: attempting to find first bit set of 0 !" */
#define syserr_displacement 		"d\031\315\226\301\356\206\354r\033g\004%\
ld"    /* "displacement out of range %ld" */
#define syserr_labref 			"\363\267l\265\235 \007f\003\023c\004\306%.8lx"    /* "unknown label reference type %.8lx" */
#define syserr_enter 			"\301\034\050J\137ENTER %ld\051"    /* "emit(J_ENTER %ld)" */
#define syserr_data_sym			"Un\265\260\035f\001\005a\026\262\240\360\212\
symb\236 \345%ld"    /* "Unable to find another data symbol at %ld" */
#define syserr_show_inst 		"show\137\001\321uQ\176\050%\043lx\051"    /* "show_instruction (%#lx)" */
#define syserr_movr 			"movr r,r"    /* "movr r,r" */
#define syserr_offset_out_of_range	"o\272\305\050%d\332\266t\372b\214"    /* "offset (%d) is too big" */
#define syserr_cannot_do_block_move	"c\033\200d\015b\365k move"    /* "cannot do block move" */
#define syserr_bad_length		"ba\005l\023gt\221\247b\365k \245\003\013\176\
\075\300%ld"    /* "bad length for block operation => %ld" */
#define syserr_local_base 		"\365\014\137b\037\004%lx"    /* "local_base %lx" */
#define syserr_setsp_confused 		"SETSP \217f\237\020%ld\041\075%ld %ld"    /* "SETSP confused %ld!=%ld %ld" */
#define syserr_illegal_register         "h\022dwa\007\137\340\021\030\224\
y\310e\351\341\274\340\2100x%lx"    /* "hardware_register: trying to evaluate illegal register 0x%lx" */
#define syserr_illegal_register2	"\007\014\137\340\021\030\224y\310e\351\
\341\274\340\2100x%lx"    /* "real_register: trying to evaluate illegal register 0x%lx" */
#define syserr_illegal_displacement	"\274d\031\315\226\301\356\0017\334\
\007Q\057\001\334\007c\002\224i\211\336\275s\017"    /* "illegal displacement in indirect/indirect triadic addressing" */
#define syserr_illegal_displacement2	"\274d\031\315\226\301\356\050\261\
0\05177\334\007c\002\224i\211\336\275s\017"    /* "illegal displacement (non-0) in indirect triadic addressing" */
#define syserr_illegal_displacement3	"\274d\031\315\226\301\356\050\200\
1\05177\334\007c\002\224i\211\336\275s\017"    /* "illegal displacement (not 1) in indirect triadic addressing" */
#define syserr_signed_val_too_large	"imm\205i\341\351\004t\372l\317\004\
\247s\214n\020\224i\211\336imm\205i\341\275s\017"    /* "immediate value too large for signed triadic immediate addressing" */
#define syserr_unsigned_val_too_large   "imm\205i\341\351\004t\372l\317\
\004\247\027s\214n\020\224i\211\336imm\205i\341\275s\017"    /* "immediate value too large for unsigned triadic immediate addressing" */
#define syserr_unknown_triadic		"\363\267k\001\005\140\224i\211\336\275\
\316%d"    /* "unknown kind of triadic addressing %d" */
#define syserr_displacement_out_of_range "d\031\315\226\301\356\206\354\
r\033g\004W7\334\007Q\057\001\334\007c\002\224i\211\213"    /* "displacement out of range for indirect/indirect triadic" */
#define syserr_illegal_indirect		 "\234\0147\334\007c\002\224i\211\336\275\
\316mo\036"    /* "illegal indirect triadic addressing mode" */
#define syserr_triadic_disp_oor		 "\224i\211\336d\031\315\226\301\356\206\
\354r\033ge"    /* "triadic displacement out of range" */
#define syserr_unknown_op		 "\206t\137\224i\211\213\137\245\030wh\345\266\
\262\266mess\077 %d"    /* "out_triadic_op: what is this mess? %d" */
#define syserr_non_word_offset		 "\224y\310\211\005\212\261w\010\005o\272\
\305\035\212w\010\005\376"    /* "trying to add a non-word offset to a word pointer" */
#define syserr_FP_value_not_fit		 "\037k\020\035p\003\264 \033 \245 \271\
\221\212\337\311\002\351\004\262\345doe\016\200f\034"    /* "asked to perform an op with a floating point value that does not fit" */
#define syserr_offset_reg_conflict	 "o\272\305\340\210\217fliQ"    /* "offset register conflict" */
#define syserr_bad_stack_move		 "\100\226k mov\004b\202\212\261multip\260\
\140%d"    /* "stack move by a non-multiple of %d" */
#define syserr_too_many_to_skip		 "t\372m\033y7\321uQ\025\016\035skip\041 \
\050%ld\051"    /* "too many instructions to skip! (%ld)" */
#define syserr_cannot_cond_branch	 "c\033\200\217d\034\025\014l\202br\033\
c\221\035o\272\305%lx"    /* "cannot conditionally branch to offset %lx" */
#define syserr_not_identifier		 "\031\137\256\025\030\200p\277\020\033 \
\304\307i\323\003"    /* "is_function: not passed an identifier" */
#define syserr_fixed_SP			 "\136\211\024\100\226k \215\210\271\221\212\217\
\100\033\002\275\016\266\200ye\002supp\010t\205"    /* "loading stack pointer with a constant address is not yet supported" */
#define syserr_offset_from_fn		 "\257\266\274\035tak\004\033 o\272\305f\
\350m \212\327\376"    /* "it is illegal to take an offset from a function pointer" */
#define syserr_offset_too_big		 "\204\021n\203\360a\030o\272\305t\372b\214 \
\050o\272\305\075 %ld\051"    /* "external data: offset too big (offset = %ld)" */
#define syserr_need_patch		 "\037\220mbl\202\223\371\206tpu\002\266m\031\
\316\212p\013c\221\334\007Qive"    /* "assembly code output is missing a patch directive" */
#define syserr_no_data_to_init		 "b\365k\137\360a\137\001\034\030n\015\360\
\212\035\001\257\050%ld\051"    /* "block_data_init: no data to init (%ld)" */
#define syserr_store_zero		 "\037k\020\035\100\010\00407\035\014\007\211\
\202z\003o\'\020m\301\010y"    /* "asked to store 0 into already zero'ed memory" */
#define syserr_export_non_data		 "\204p\010t\137\360a\137symb\236\030\037\
k\020\035\204p\010\002\261\360a\030%s"    /* "export_data_symbol: asked to export non-data: %s" */
#define syserr_offset_too_large		 "c\033\200\001\353\004l\317\004\360\212\
o\272\220t"    /* "cannot initialise large data offset" */
#define syserr_cannot_call_offset	 "c\033\200c\014l \035o\272\305%lx"    /* "cannot call to offset %lx" */
#define syserr_already_set_label	 "\224y\310\305\033 \014\007\211\202\305\
l\265\235\041"    /* "trying to set an already set label!" */
#define syserr_copy_less_than_four	 "\223py\024\177s\016\262\033 4 byte\
\016\140m\301\010\202\050%ld\051"    /* "copying less than 4 bytes of memory (%ld)" */
#define syserr_non_word_multiple	 "\223p\202m\301\010\202\037k\020\035\223\
p\202\212\242 w\010\005multip\260%ld"    /* "copy memory asked to copy a non word multiple %ld" */
#define syserr_urg			 "\374g \216\262\266sh\206l\005\200happ\023"    /* "urg - this should not happen" */
#define syserr_init_failed		 "fa5\020\035\001\353\004sav\205\137\303s"    /* "failed to initialise saved_regs" */
#define syserr_stack_mis_aligned	 "\100\226k m\031-\014\214n\205\n"    /* "stack mis-aligned\n" */
#define syserr_not_match_pending	 "br\033c\221%lx doe\016\200m\013c\221\
p\023d\024\217d\034\176%lx\n"    /* "branch %lx does not match pending condition %lx\n" */
#define syserr_adjust_non_word		 "\224y\310\237\004\212\261w\010\005o\272\
\305\271\221\212w\010\005\376"    /* "trying to use a non-word offset with a word pointer" */
#define syserr_no_byte_short_sign_extend "s\214\267\204t\023\005byt\004\
\035sh\010\002\200ye\002supp\010t\205"    /* "sign extend byte to short not yet supported" */
#define syserr_unknown_sign_extend_mode	 "\363\267s\214\267\204t\023\005\
mo\371%ld"    /* "unknown sign extend mode %ld" */
  
#ifdef TARGET_HAS_COND_EXEC
#define syserr_no_fast_compare		 "c\033\200d\015f\037\002\217d\034\025\203\
\223\233a\241"    /* "cannot do fast conditional compares" */
#define syserr_cond_exec		 "\013te\233t\020\035\217d\034\025\014l\202\204\
ecut\004J\137\245\223\371%ld\n"    /* "attempted to conditionally execute J_opcode %ld\n" */
#endif
  
#ifdef TARGET_LACKS_UNSIGNED_FIX
#define syserr_no_unsigned_fix		 "\027s\214n\020FIX \200supp\010t\205"    /* "unsigned FIX not supported" */
#endif
  
#define syserr_non_float_dest		 "\224y\310p\003\264 \212\337\311\002\245\
\003\013\176\271\221\212\242 \337\311\002\036\100\001\013\025"    /* "trying to perform a floating point operation with a non floating point destination" */

#ifdef DEBUG
#define syserr_push_non_float		 "\224y\310\255\345p\237\221\212\261\337\
\311\002\340\210%x"    /* "trying to float push a non-floating point register %x" */
#define syserr_pop_non_float		 "\224y\310\255\345\207p \212\261\337\311\
\002\340\210%x"    /* "trying to float pop a non-floating point register %x" */
#define syserr_push_non_double		 "\224y\310d\206b\260p\237\221\212\261\337\
\311\002\340\210%x"    /* "trying to double push a non-floating point register %x" */
#define syserr_pop_non_double		 "\224y\310d\206b\260\207p \212\261\337\311\
\002\340\210%x"    /* "trying to double pop a non-floating point register %x" */
#define syserr_non_float_source		 "\224y\310p\003\264 \212\337\311\002\245\
\003\013\176\271\221\212\242 \337\311\002s\206rce"    /* "trying to perform a floating point operation with a non floating point source" */
#define syserr_void_compare		 "\037k\020\035p\003\264 \375\361\223\233\022\
\031\025"    /* "asked to perform void comparision" */  
#define syserr_null_pointer		 "g\023.c\030\031\137\256\025\030p\277\020\
\212NULL \376"    /* "gen.c: is_function: passed a NULL pointer" */  
#define syserr_non_function		 "\136\211\137\275s\137\217\100\033t\030\224\
y\310tak\004\275\016\140\261\256\025\041"    /* "load_address_constant: trying to take address of non-function!" */
#define syserr_non_aligned_fn		 "\136\211\137\275s\137\217\100\033t\030\
\327\215\210\200w\010\005\014\214n\205\041"    /* "load_address_constant: function pointer not word aligned!" */
#define syserr_fn_call_in_stub		 "c\014l\030\327c\014l \035%s wh5s\002g\
\023\003\013\024\212\100ub"    /* "call: function call to %s whilst generating a stub" */
#define debug_null_type_expr		 "db\137\225\174\030p\277\020\212NULL Ty\174\
Expr \376"    /* "db_type: passed a NULL TypeExpr pointer" */
#define debug_null_parameter		 "up\360e\137v\022\030NULL \362\243e\021"    /* "update_var: NULL parameter" */
#endif /* DEBUG */

#define debug_cannot_open_output	 "fa5\020\035\245\023 \036\333g \206tp\
u\002f5\004%s"    /* "failed to open debug output file %s" */
#define debug_db_type			 "db\137\225\174\050 %ld,0x%lx \051"    /* "db_type( %ld,0x%lx )" */
#define debug_no_block			 "n\015c\374\007n\002b\365k \231v\022i\265\177"    /* "no current block or variable" */
#define debug_already_associated	 "\224y\310\211\005\033 \014\007\211\202\
\277oci\330\365\203v\022i\265\177"    /* "trying to add an already associated local variable" */
#define debug_unknown_storage_type	 "\363\267\100\010ag\004\225\174"    /* "unknown storage type" */
#define debug_unknown_auto_storage	 "\363\267\306\140au\035\100\010age"    /* "unknown type of auto storage" */
#define debug_bad_scope			 "ba\005s\223\174"    /* "bad scope" */
#define debug_unresolved_var_class	 "\027\241\236v\020v\022i\265\260\244\
\037\016\247%s"    /* "unresolved variable class for %s" */
#define debug_no_blocks			 "\327\271\221n\015b\365ks"    /* "function with no blocks" */

#define peep_null_format		"NULL \264\345\321\017"    /* "NULL format string" */
#define peep_null_list			"p\277\020NULL l\031t"    /* "passed NULL list" */
#define peep_null_parameter		"p\277\020NULL \362\243e\021"    /* "passed NULL parameter" */
#define peep_urg			"\352\355\374g \374g"    /* "peepholer: urg urg" */
#define peep_peepholer_empty		"\013te\233t\310\301\257f\350m EMPTY \352\
\003\041"    /* "attempting to emit from EMPTY peepholer!" */
#define peep_unknown_push		"\352\355\363\267p\237\221\225\174"    /* "peepholer: unknown push type" */
#define peep_out_of_memory		"\352\355\206\354m\301\010y"    /* "peepholer: out of memory" */
#define peep_no_offset			"\352\355n\015o\272\305\250br\033\322\041"    /* "peepholer: no offset in branch!" */
#define peep_unknown_delay_type		"\352\355\363\267\036l\367\225\174"    /* "peepholer: unknown delay type" */
#define peep_unknown_type		"\352\355\037k\020\035\301\257\033 \363\267\306\
%d"    /* "peepholer: asked to emit an unknown type %d" */
#define peep_unexpected_back_ref	"\352\355\027\204\174\344b\226kw\022d\016\
\007f\003\023ce"    /* "peepholer: unexpected backwards reference" */
#define peep_fwd_and_cross_ref		"\352\355p\023d\024Ww\022\005\033\005c\350\
s\016\007f\003\023ce\016\247s\243\004\001\321uQ\025\041"    /* "peepholer: pending forward and cross references for same instruction!" */
#define peep_special_pending_cross	"\352\355s\174c\3247\321uQ\176\306ha\
\005\212p\023d\024c\350s\016\007f\003\023ce\041"    /* "peepholer: special instruction type had a pending cross reference!" */
#define peep_special_pending_fwd	"\352\355s\174c\3247\321uQ\176\306ha\005\
\212p\023d\024Ww\022\005\007f\003\023ce\041"    /* "peepholer: special instruction type had a pending forward reference!" */
#define peep_elim_clash			"\352\355\013te\233t\310\235im\001\341\001\321\
uQ\176wh5s\002\212\007f\003\023c\004\266\1005l p\023d\017"    /* "peepholer: attempting to eliminate instruction whilst a reference is still pending" */
#define peep_non_existant_xfer		"\352\355\224y\310\235im\001\341\261\204\
\031t\033\002\340\210\224\033sf\003"    /* "peepholer: trying to eliminate non-existant register transfer" */
#define peep_cross_ref_pending		"\352\355\013te\233t\310fl\237\221\352\240\
wh5s\002\212c\350s\016\007\032\266\1005l p\023d\017"    /* "peepholer: attempting to flush peepholer whilst a cross ref is still pending" */
#define peep_fwd_ref_pending		"\352\355\013te\233t\310fl\237\221\352\240\
wh5s\002\212Ww\022\005\007\032\266\1005l p\023d\017"    /* "peepholer: attempting to flush peepholer whilst a forward ref is still pending" */
#define peep_failed_reset_count		"\352\355fl\237\221h\253fa5\020\035\007\
\305\335 \223\027t"    /* "peepholer: flush has failed to reset peep count" */

  
/* end of mbe/mcerrs.h */
#ifndef NO_INSTORE_FILES
#  define NO_INSTORE_FILES 1
#endif

#define COMPRESSED_ERROR_MESSAGES 1

#ifdef DEFINE_ERROR_COMPRESSION_TABLE

static unsigned short int ecompression_info[256] = {
    0x0000, 0x696e, 0x7420, 0x6572, 0x6520, 0x6420, 0x6f6e, 0x7265, 
    0x6f72, 0x2a2a, 0x000a, 0x6174, 0x616c, 0x6f20, 0x7320, 0x0167, 
    0x6505, 0x7403, 0x6172, 0x656e, 0x0f20, 0x6906, 0x6e6f, 0x756e, 
    0x3a20, 0x6973, 0x6620, 0x616e, 0x6974, 0x740d, 0x6465, 0x6173, 
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x696c, 0x0036, 0x2001, 
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
    0x7374, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
    0x0048, 0x0049, 0x004a, 0x0909, 0x004c, 0x004d, 0x004e, 0x004f, 
    0x0050, 0x6374, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x6608, 
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x6c6f, 0x005f, 
    0x6f1a, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
    0x0078, 0x0079, 0x007a, 0x007b, 0x7065, 0x007d, 0x1520, 0x6c65, 
    0x1602, 0x2720, 0x7920, 0x0c20, 0x6578, 0x6564, 0x6f75, 0x706f, 
    0x1120, 0x6164, 0x6120, 0x6963, 0x6967, 0x8701, 0x2d20, 0x6306, 
    0x7365, 0x6820, 0x7f67, 0x636f, 0x7472, 0x7479, 0x6163, 0x756d, 
    0x1e66, 0x0820, 0x4b4b, 0x6d70, 0x3592, 0x656c, 0x6f6c, 0x7573, 
    0x0320, 0x0773, 0x6e06, 0x616d, 0x636c, 0x6f70, 0x9801, 0x5720, 
    0x0120, 0x2e2e, 0x3720, 0x610e, 0x1751, 0x665e, 0x66ac, 0x6902, 
    0x6c04, 0xa22d, 0x7468, 0x8964, 0x576d, 0x6162, 0x690e, 0x6e20, 
    0x7004, 0x771c, 0x6666, 0x760c, 0x9c83, 0xb3a1, 0x0174, 0x1f73, 
    0x3e20, 0x656d, 0x0764, 0x0767, 0x6964, 0x9002, 0x95b8, 0x1374, 
    0x141d, 0x148d, 0x16c2, 0x631f, 0x6570, 0x706c, 0x7314, 0x1267, 
    0x176b, 0x4072, 0x6368, 0x6669, 0x690c, 0x7603, 0x8cca, 0xae7e, 
    0x0b10, 0x1677, 0x2920, 0x6275, 0x6469, 0x7ccc, 0x8b20, 0xad0b, 
    0xc319, 0x0b04, 0x1cd4, 0x1ea4, 0x5110, 0x6102, 0x689e, 0x6f17, 
    0x726f, 0xbb75, 0xdde6, 0xe219, 0x0260, 0x0318, 0x1302, 0x2c20, 
    0x640b, 0x6905, 0x7012, 0xd0d9, 0x2020, 0x5e63, 0x6166, 0x6182, 
    0x626a, 0x6404, 0x6f0d, 0x6ff8, 0x7572, 0x766f, 0x8d11, 0x9a9a};


#endif

#define MAXSTACKDEPTH 4

#endif /* already loaded */

/* end of errors.h */
