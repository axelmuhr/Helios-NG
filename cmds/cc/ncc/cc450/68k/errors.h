
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
        "\030\220\265ehavi\203r\033$b wr\027t\021 \356\020a7\314\203\002\
\001\017v\021\036\243\351\021c\003\224t"    /* "undefined behaviour: $b written and read without intervening sequence point" */
#define warn_usage_ww \
        "\030\220\265ehavi\203r\033$b wr\027t\021 tw\211\003\314\203\002\
\001\017v\021\036\243\351\021c\003\224t"    /* "undefined behaviour: $b written twice without intervening sequence point" */

#define bind_warn_extern_clash \
        "\174\367 \216Kh $r\270$r \216Kh \050ANSI 6 \345 m\006o\251e\051"    /* "extern clash $r, $r clash (ANSI 6 char monocase)" */
#define bind_warn_unused_static_decl "\030\235\013e\016li\225\136\014\211 \
\354\372$r"    /* "unused earlier static declaration of $r" */
#define bind_warn_not_in_hdr "\174\367 $r \200\272\013\037head\004"    /* "extern $r not declared in header" */
#define bind_warn_main_not_int "\174\367 \'\231\001\032\377\005\015\206\
\334\371\032\252\022"    /* "extern 'main' needs to be 'int' function" */
#define bind_warn_label_not_used "l\275\222 $r w\226\220\265\310\200\235\
\005"    /* "label $r was defined but not used" */
/*
 * Note that when part of an error string MUST be stored as a regular
 * non-compressed string I have to inform the GenHdrs utility with %Z and %O
 * This arises when a string contains literal strings as extra sub-args.
 */
#define bind_warn_not_used(is_typedef,is_fn,binder) \
        "%s $b \272\265\310\200\235\005"    /* "%s $b declared but not used" */, \
      ((is_typedef) ? "typedef" : ((is_fn) ? "function" : "variable")),\
      binder
#define bind_warn_static_not_used "\136\014\211 $b \272\265\310\200\235\
\005"    /* "static $b declared but not used" */
#define cg_warn_implicit_return "\375\256\242\020t\361n\316\321\323\336\
%s\050\051"    /* "implicit return in non-void %s()" */
#define flowgraf_warn_implicit_return "\375\256\242\020t\361n\316\321\323\
\336\252\022"    /* "implicit return in non-void function" */
#define pp_warn_triglyph \
        "ANSI \'%c%c%c\032tr\204\360ph \236\'%c\032\260\332\176w\226\255\
\335\233\021d\005\077"    /* "ANSI '%c%c%c' trigraph for '%c' found - was this intended?" */
#define pp_warn_nested_comment "\345\302\202\243\351\021c\003%s \001s\253\
\003\241mm\244"    /* "character sequence %s inside comment" */
#define pp_warn_many_arglines \
        "\050\214\341ib\254\350\010\051\033\076\075 %lu \337e\015W\231c\
r\023\330\205\244s"    /* "(possible error): >= %lu lines of macro arguments" */
#define pp_warn_redefinition "\020\217\266\220\027\372\043\220\003\231c\
r\023%s"    /* "repeated definition of #define macro %s" */
#define pp_rerr_redefinition "d\271f\004\036r\005ef\273\372\043\220\003\
\231cr\023%s"    /* "differing redefinition of #define macro %s" */
#define pp_rerr_nonunique_formal "d\276\256\014\003\231cr\023\3445\366\017\
\033\'%s\'"    /* "duplicate macro formal parameter: '%s'" */
#define pp_rerr_define_hash_arg "\322\004\356W\043 \200\231cr\023\3445\366\
\017"    /* "operand of # not macro formal parameter" */
#define pp_rerr_define_hashhash "\043\043 firs\002\035lK\002tok\021\316\
\043\220\003body"    /* "## first or last token in #define body" */
#define pp_warn_ifvaldef "\043\271\031\026%s \231\177\001d\211\014\003t\
r\203bY\343"    /* "#ifdef %s may indicate trouble..." */ /* MACH_EXTNS */
#define pp_warn_nonansi_header "N\264ANSI \043\001\216ud\003\074%s\076"    /* "Non-ANSI #include <%s>" */
#define pp_warn_bad_pragma "Un\020\241gn\034\013\043p\360g\231 \050n\023\'\
-\032\035\030k\140wn w\010d\051"    /* "Unrecognised #pragma (no '-' or unknown word)" */
#define pp_warn_bad_pragma1 "Un\020\241gn\034\013\043p\360g\231 -%c"    /* "Unrecognised #pragma -%c" */
#define pp_warn_unused_macro "\043\220\003\231cr\023\'%s\032\220\265\310\
\200\235\005"    /* "#define macro '%s' defined but not used" */
#define regalloc_warn_use_before_set "$b \231\177\334\235\265e\230\003b\
e\036\243t"    /* "$b may be used before being set" */
#define regalloc_warn_never_used "$b \335\243\002b\310\377v\225\235\005"    /* "$b is set but never used" */
#define sem_warn_unsigned "ANSI s\361pr\034e\033\'l\006g\032$s \'\030s\215\
\005\032yi\222d\015\'l\006g\'"    /* "ANSI surprise: 'long' $s 'unsigned' yields 'long'" */
#define sem_warn_format_type "aPu5\312$t m\034m\014\240e\015\230\231\002\'\
%.\052s\'"    /* "actual type $t mismatches format '%.*s'" */
#define sem_warn_bad_format "Il\223\230\231\002\213\262sQ\'%%%c\'"    /* "Illegal format conversion '%%%c'" */
#define sem_warn_incomplete_format "In\241\234Yt\003\230\231\002\373\001\
g"    /* "Incomplete format string" */
#define sem_warn_format_nargs "F\010\231\002\020\351i\020\015%ld \366\017\
%s\270b\310%ld giv\021"    /* "Format requires %ld parameter%s, but %ld given" */
#define sem_warn_addr_array "\'\046\032\030\377ce\341\016\177\236\313\035\
\016\360\177$e"    /* "'&' unnecessary for function or array $e" */
#define sem_warn_bad_shift(_m,_n) "sh\271\002W$m b\177%ld \030\220\013\037\
ANSI C"    /* "shift of $m by %ld undefined in ANSI C" */,_m,_n
#define sem_warn_divrem_0 "div\034Qb\177z\004o\033$s"    /* "division by zero: $s" */
#define sem_warn_ucomp_0 "od7\030s\215\013\241\234\016\034\355\314 0\033\
$s"    /* "odd unsigned comparison with 0: $s" */
#define sem_warn_fp_overflow(op) "\305\014\036\224\002\363B\002o\262\305\
w\033$s"    /* "floating point constant overflow: $s" */,op
#define sem_rerr_udiad_overflow(op,_a,_b,_c) "\030s\215\013\363B\002o\262\
\305w\033$s"    /* "unsigned constant overflow: $s" */,op
#define sem_rerr_diad_overflow(op,_a,_b,_c) "s\215\013\363B\002o\262\305\
w\033$s"    /* "signed constant overflow: $s" */,op
#define sem_rerr_umonad_overflow(op,_a,_b) "\030s\215\013\363B\002o\262\
\305w\033$s"    /* "unsigned constant overflow: $s" */,op
#define sem_rerr_monad_overflow(op,_a,_b) "s\215\013\363B\002o\262\305w\
\033$s"    /* "signed constant overflow: $s" */,op
#define sem_rerr_implicit_cast_overflow(_t,_a,_b) \
                                "\375\256\242\325\050\206$t\250o\262\305\
w"    /* "implicit cast (to $t) overflow" */,_t
#define sem_warn_fix_fail "\305\014\036\206\233egr5\213\262sQfa\100\005"    /* "floating to integral conversion failed" */
#define sem_warn_index_ovfl "\203t-of-bo\332off\243\002%ld\316add\020\341"    /* "out-of-bound offset %ld in address" */
#define sem_warn_low_precision "\201w\225p\020c\034Q\037w\253\225\213t\174\
t\033$s"    /* "lower precision in wider context: $s" */
#define sem_warn_odd_condition "\235\003W$s\316\213d\027Q\213t\174t"    /* "use of $s in condition context" */
#define sem_warn_void_context "n\023s\253\003effec\002\037\323\336\213t\
\174t\033$s"    /* "no side effect in void context: $s" */
#define sem_warn_olde_mismatch "\330\205\370\356old-\136y\254\366\202m\034\
m\014\240\033$e"    /* "argument and old-style parameter mismatch: $e" */
#define sem_warn_uncheckable_format \
        "\'\344\014\032\330. \206pr\233f\057scB\026etc. \335v\016i\275Y\
\270s\023cB\200\334\240eck\005"    /* "'format' arg. to printf/scanf etc. is variable, so cannot be checked" */
#define sem_warn_narrow_voidstar "\375\256\242\325f\352m \050\323\336\052\
\051\270C\053\053 \230b\253s"    /* "implicit cast from (void *), C++ forbids" */
#define sem_warn_narrowing "\375\256\242n\016\352w\036\251t\033$s"    /* "implicit narrowing cast: $s" */
#define sem_warn_fn_cast \
        "$s\033\325betwe\021 \313\224\202\356\321\313\307eP"    /* "$s: cast between function pointer and non-function object" */
#define sem_warn_pointer_int "\174p\256\242\325W\224\202\206\371\'"    /* "explicit cast of pointer to 'int'" */
#define bind_err_extern_clash "\174\367 \216Kh $r\270$r \050\337k\225%l\
d \345%s\051"    /* "extern clash $r, $r (linker %ld char%s)" */
#define bind_err_duplicate_tag "d\276\256\014\003\220\027\372$s ta\025$\
b"    /* "duplicate definition of $s tag $b" */
#define bind_err_reuse_tag "\020-\235\036$s ta\025$b \226$s tag"    /* "re-using $s tag $b as $s tag" */
#define bind_err_incomplete_tentative \
        "\001\241\234Yt\003t\244\014iv\003\354\372$r"    /* "incomplete tentative declaration of $r" */
#define bind_err_type_disagreement "\312d\034ag\020em\370\236$r"    /* "type disagreement for $r" */
#define bind_err_duplicate_definition "d\276\256\014\003\220\027\372$r"    /* "duplicate definition of $r" */
#define bind_err_duplicate_label "d\276\256\014\003\220\027\372l\275\222 \
$r\317\257"    /* "duplicate definition of label $r - ignored" */
#define bind_err_unset_label "l\275\222 $r h\226\200be\021 \243t"    /* "label $r has not been set" */
#define bind_err_undefined_static \
        "\136\014\211 \313$b \200\220\013\176\347\174\367"    /* "static function $b not defined - treated as extern" */
#define bind_err_conflicting_globalreg \
        "\213fliP\036g\201b5\020g\034\202\354\022\015\236$b"    /* "conflicting global register declarations for $b" */
#define fp_err_very_big "O\262l\330\003\305\014\036\224\002\365\003\260\
\030d"    /* "Overlarge floating point value found" */
#define fp_err_big_single \
        "O\262l\330\003\050s\001g\254p\020c\034\022\250\305\014\036\224\
\002\365\003\260\030d"    /* "Overlarge (single precision) floating point value found" */
#define pp_err_eof_comment "EOF\316\241mm\244"    /* "EOF in comment" */
#define pp_err_eof_string "EOF\316\373\001g"    /* "EOF in string" */
#define pp_err_eol_string "\351ot\003\050%c\250\001s\004t\265e\230\003\377\
w\337e"    /* "quote (%c) inserted before newline" */
#define pp_err_eof_escape "EOF\316\373\036esca\217"    /* "EOF in string escape" */
#define pp_err_missing_quote "M\346\'%c\032\037p\020-p\352ce\341\035\241\
mm\356\337e"    /* "Missing '%c' in pre-processor command line" */
#define pp_err_if_defined "N\023\253\244\271i\225\315\043i\026\220\005"    /* "No identifier after #if defined" */
#define pp_err_if_defined1 "N\023\'\051\032\315\043i\026\220\005\050\343"    /* "No ')' after #if defined(..." */
#define pp_err_rpar_eof "M\346\'\051\032\315%s\050\343 \355\337\003%ld"    /* "Missing ')' after %s(... on line %ld" */
#define pp_err_many_args "To\023mB\177\330\205\244\015\206\231cr\023%s\050\
\343 \355\337\003%ld"    /* "Too many arguments to macro %s(... on line %ld" */
#define pp_err_few_args "To\023few \330\205\244\015\206\231cr\023%s\050\
\343 \355\337\003%ld"    /* "Too few arguments to macro %s(... on line %ld" */
#define pp_err_missing_identifier "M\346\253\244\271i\225\315\043\220e"    /* "Missing identifier after #define" */
#define pp_err_missing_parameter "M\346\366\202n\227\003\037\043\220\003\
%s\050\343"    /* "Missing parameter name in #define %s(..." */
#define pp_err_missing_comma "M\346\',\032\035\'\051\032\315\043\220\003\
%s\050\343"    /* "Missing ',' or ')' after #define %s(..." */
#define pp_err_undef "M\346\253\244\271i\225\315\043\030\210"    /* "Missing identifier after #undef" */
#define pp_err_ifdef "M\346\253\244\271i\225\315\043\271\210"    /* "Missing identifier after #ifdef" */
#define pp_err_include_quote "M\346\'\074\032\035\'\"\032\315\043\001\216\
u\031"    /* "Missing '<' or '\"' after #include" */
#define pp_err_include_junk "J\030k \315\043\001\216ud\003%c%s%c"    /* "Junk after #include %c%s%c" */
#define pp_err_include_file "\043\001\216ud\003f\100\003%c%s%c w\203ldn\'\
\002\322\021"    /* "#include file %c%s%c wouldn't open" */
#define pp_err_unknown_directive "Unk\140wn di\020Pive\033\043%s"    /* "Unknown directive: #%s" */
#define pp_err_endif_eof "M\346\043\021di\026a\002EOF"    /* "Missing #endif at EOF" */
#define sem_err_typeclash "Il\223\232\217\015\236\322\004Bds\033$s"    /* "Illegal types for operands: $s" */
#define sem_err_sizeof_struct "s\374\003W$c \377\005\265\310\200ye\002\220\
\005"    /* "size of $c needed but not yet defined" */
#define sem_err_lvalue "Il\223\037l\365e\033\313\035\016\360\177$e"    /* "Illegal in lvalue: function or array $e" */
#define sem_err_bitfield_address "b\242fi\222d\015d\023\200hav\003add\020\
s\243s"    /* "bit fields do not have addresses" */
#define sem_err_lvalue1 "Il\223\037l-\365e\033\'\021\205\032\363B\002$b"    /* "Illegal in l-value: 'enum' constant $b" */
#define sem_err_lvalue2 "Il\223\037\255\003\213t\174\002WB l-\365e\033$\
s"    /* "Illegal in the context of an l-value: $s" */
#define sem_err_nonconst "\237\037%s\033\074\030k\140wn\076"    /* "illegal in %s: <unknown>" */
#define sem_err_nonconst1 "\237\037%s\033n\355\363B\002$b"    /* "illegal in %s: non constant $b" */
#define sem_err_nonconst2 "\237\037%s\033$s"    /* "illegal in %s: $s" */
#define sem_err_nonfunction "\014te\234\002\206app\376\320\321\252\022"    /* "attempt to apply a non-function" */
#define sem_err_void_argument "\'\323\253\032\365e\015\231\177\200\334\330\
\205\244s"    /* "'void' values may not be arguments" */
#define sem_err_bad_cast "$s\033\237\325W$t \206\364"    /* "$s: illegal cast of $t to pointer" */
#define sem_err_bad_cast1 "$s\033\237\325\206$t"    /* "$s: illegal cast to $t" */
#define sem_err_bad_cast2 "$s\033\325\206\321e\3515$t \100\212\011"    /* "$s: cast to non-equal $t illegal" */
#define sem_err_undef_struct \
        "$c \200ye\002\220\013\176cB\200\334\243YP\013f\352m"    /* "$c not yet defined - cannot be selected from" */
#define sem_err_unknown_field "$c h\226n\023$r fi\222d"    /* "$c has no $r field" */
#define errs_membobj(_m)\
  (_m ? "member":"object")


#define bind_rerr_undefined_tag "$s ta\025$b \200\220\005"    /* "$s tag $b not defined" */
#define bind_rerr_linkage_disagreement \
        "\337kag\003d\034ag\020em\370\236$r\317\347$m"    /* "linkage disagreement for $r - treated as $m" */
#define bind_rerr_duplicate_typedef "d\276\256\014\003\232p\005e\026$r"    /* "duplicate typedef $r" */
#define bind_rerr_local_extern "\174\367 $r m\034m\014\240e\015t\322-Yv\
\222 \354\022"    /* "extern $r mismatches top-level declaration" */
#define fp_rerr_very_small "sm\011l \305\014\036\224\002\365\003\213\262\
t\013\2060.0"    /* "small floating point value converted to 0.0" */
#define fp_rerr_small_single \
        "sm\011l \050s\001g\254p\020c\034\022\250\305\014\036\365\003\213\
\262t\013\2060.0"    /* "small (single precision) floating value converted to 0.0" */
#define pp_rerr_newline_eof "m\346\377w\337\003be\230\003EOF\317\001s\004\
t\005"    /* "missing newline before EOF - inserted" */
#define pp_rerr_nonprint_char "\030pr\233\275\254\345 %\043.2x \260\332\
\176\257"    /* "unprintable char %#.2x found - ignored" */
#define pp_rerr_illegal_option "\237\322tQ-D%s%s"    /* "illegal option -D%s%s" */
#define pp_rerr_spurious_else "sp\361i\203\015\043\222s\003\257"    /* "spurious #else ignored" */
#define pp_rerr_spurious_elif "sp\361i\203\015\043\222i\026\257"    /* "spurious #elif ignored" */
#define pp_rerr_spurious_endif "sp\361i\203\015\043\021di\026\257"    /* "spurious #endif ignored" */
#define pp_rerr_hash_line "n\205b\225m\346\037\043\337e"    /* "number missing in #line" */
#define pp_rerr_hash_error "\043\350\035\021\241\030\017\013\"%s\""    /* "#error encountered \"%s\"" */
#define pp_rerr_hash_ident "\043\253\370\335\200\037ANSI C"    /* "#ident is not in ANSI C" */
#define pp_rerr_junk_eol "j\030k a\002\0217W\043%s \337\003\176\257"    /* "junk at end of #%s line - ignored" */
#define sem_rerr_sizeof_bitfield \
        "s\374eW\074b\242fi\222d\245\237\176s\374eof\050\233\250\327\005"    /* "sizeof <bit field> illegal - sizeof(int) assumed" */
#define sem_rerr_sizeof_void "s\374\003W\'\323\253\032\020\351ir\013\176\
\3471"    /* "size of 'void' required - treated as 1" */
#define sem_rerr_sizeof_array "s\374\003W\320\133\135 \016\360\177\020\351\
ir\005\270\347\1331\135"    /* "size of a [] array required, treated as [1]" */
#define sem_rerr_sizeof_function \
        "s\374\003W\313\020\351ir\013\176\347s\374\003W\364"    /* "size of function required - treated as size of pointer" */
#define sem_rerr_pointer_arith \
        "\074\233\245$s \074\364\245\347\074\233\245$s \050\233\051\074\
\364\076"    /* "<int> $s <pointer> treated as <int> $s (int)<pointer>" */
#define sem_rerr_pointer_arith1 \
        "\074\364\245$s \074\233\245\347\050\233\051\074\364\245$s \074\
\233\076"    /* "<pointer> $s <int> treated as (int)<pointer> $s <int>" */
#define sem_rerr_assign_const "\274\215m\370\206\'\363\032\307ec\002$e"    /* "assignment to 'const' object $e" */
#define sem_rerr_addr_regvar \
        "\'\020g\034\017\032\014tribut\003\236$b \246\013wh\021 add\020\
s\015tak\021"    /* "'register' attribute for $b ignored when address taken" */
#define sem_rerr_lcast "\307eP\015\255a\002hav\003be\021 \325\016\003\200\
l-\365es"    /* "objects that have been cast are not l-values" */
#define sem_rerr_pointer_compare \
"\241\234\016\034\355$s W\224\202\356\233:\n\301li\01750 \050\236\075\075 \
\356\041\075\250\335\006\376\223\251e"    /* "comparison $s of pointer and int:\n\
  literal 0 (for == and !=) is only legal case" */
#define sem_rerr_different_pointers "d\271f\004\036\224\202\232\217s\033\
$s"    /* "differing pointer types: $s" */
#define sem_rerr_wrong_no_args "wr\006\025n\205b\225W\366\017\015\206$e"    /* "wrong number of parameters to $e" */
#define sem_rerr_casttoenum "$s\033\325W$m \206d\271f\004\036\021\205"    /* "$s: cast of $m to differing enum" */ /* warn in C */
#define sem_rerr_valcasttoref "$s\033\321l\365\003\325\206\321\213s\002\
\020f\004\021ce"    /* "$s: non-lvalue cast to non-const reference" */
#define sem_rerr_implicit_cast1 \
        "$s\033\375\256\242\325W\224\202\206\321e\3515\364"    /* "$s: implicit cast of pointer to non-equal pointer" */
#define sem_rerr_implicit_cast2 "$s\033\375\256\242\325W\3210 \001\002\206\
\364"    /* "$s: implicit cast of non-0 int to pointer" */
#define sem_rerr_implicit_cast3 "$s\033\375\256\242\325W\224\202\206\371\'"    /* "$s: implicit cast of pointer to 'int'" */
#define sem_rerr_implicit_cast4 "$s\033\375\256\242\325W$t \206\371\'"    /* "$s: implicit cast of $t to 'int'" */
#define sem_rerr_nonpublic "$r \335\321pub\256 me\357\225W$c"    /* "$r is non-public member of $c" */
#define sem_rerr_cant_balance "d\271f\004\036\224\202\232\217s\033$s"    /* "differing pointer types: $s" */

#define sem_rerr_void_indirection "\237\001di\020PQ\355\050\323\336\052\
\051\033\'\052\'"    /* "illegal indirection on (void *): '*'" */
#define obj_fatalerr_io_object "I\057O \350\035\355\307ec\002\136\020\227"    /* "I/O error on object stream" */
#define compiler_rerr_no_extern_decl\
    "n\023\174\3675\354Q\037trBsl\014Q\030\027"    /* "no external declaration in translation unit" */
#define compiler_fatalerr_io_error "I\057O \350\035wr\027\036\'%s\'"    /* "I/O error writing '%s'" */
#define driver_fatalerr_io_object "I\057O \350\035\355\307ec\002\136\020\
\227"    /* "I/O error on object stream" */
#define driver_fatalerr_io_asm "I\057O \350\035\355K\243\357l\225\203tp\
\310\136\020\227"    /* "I/O error on assembler output stream" */
#define driver_fatalerr_io_listing "I\057O \350\035\355l\034t\036\136\020\
\227"    /* "I/O error on listing stream" */
#ifdef TARGET_HAS_AOUT
#define aout_fatalerr_toomany 	"To\023mB\177sy\357ol\015\236\'a.\203t\032\
\203tput"    /* "Too many symbols for 'a.out' output" */
#define aout_fatalerr_toobig	"Modu\254to\023bi\025\236a.\203\002\344\014\
\017"    /* "Module too big for a.out formatter" */
#endif
#ifdef TARGET_HAS_COFF
#define coff_fatalerr_toomany 	"To\023mB\177\020\201c\014\022\015\236CO\
FF \230\231\002\037.\023f\100e"    /* "Too many relocations for COFF format in .o file" */
#define coff_fatalerr_toobig	"Modu\254to\023bi\025\236COFF \344\014\017"    /* "Module too big for COFF formatter" */
#endif
#ifdef TARGET_IS_HELIOS
#define heliobj_warn_12bits       "Off\243\002%ld \24512 b\027s"    /* "Offset %ld > 12 bits" */
#define heliobj_warn_16bits       "Off\243\002%ld \24516 b\027s"    /* "Offset %ld > 16 bits" */
#define heliobj_warn_24bits       "Off\243\002%ld \24524 b\027s"    /* "Offset %ld > 24 bits" */
#define heliobj_misplaced_offsets "h\222i\307\033sy\357ol %s h\226off\243\
\002%ld \356\260l\201w\015sy\357ol %s \314 off\243\002%ld"    /* "heliobj: symbol %s has offset %ld and follows symbol %s with offset %ld" */
#define heliobj_repeated_symbol   "h\222i\307\033cB\200\050yet\250\020\217\014\
\005\376\174p\010\002\255\003s\227\003sy\357ol"    /* "heliobj: cannot (yet) repeatedly export the same symbol" */
#define heliobj_bad_fp_number     "h\222i\307\033ba7s\374\003W\305\014\036\
\224\002n\205b\225%ld"    /* "heliobj: bad size of floating point number %ld" */
#define heliobj_unknown_data_type "h\222i\307\033\030k\140wn d\014\320\312\
%lx"    /* "heliobj: unknown data type %lx" */
#define heliobj_bad_packed_length "h\222i\307\033ba7l\021g\255 \236p\302\
k\013d\014\320\050%ld\051"    /* "heliobj: bad length for packed data (%ld)" */
#define syserr_heliobj_bad_xref	  "In\277\336\174\3675\020f\004\021c\003\
$r %\043lx"    /* "Invalid external reference $r %#lx" */
#define syserr_heliobj_dataseggen "D\014\320\243\025g\021\004\014Q\213f\
\235\005"    /* "Data seg generation confused" */
#define syserr_heliobj_gendata	  "\307\137g\021d\014a\050%ld\051"    /* "obj_gendata(%ld)" */
#define syserr_heliobj_datalen	  "\307\137d\014\320l\021\075%ld"    /* "obj_data len=%ld" */
#define syserr_heliobj_data	  "\307\137d\014\320%ldEL%ld\'%s\'"    /* "obj_data %ldEL%ld'%s'" */
#define syserr_heliobj_align	  "H\222io\015\307 \011\215"    /* "Helios obj align" */
#define syserr_heliobj_2def	  "d\203b\254\220\027Q\037\307\137sym\020\026\
$r"    /* "double definition in obj_symref $r" */
#define syserr_heliobj_codedata	  "\241\031\057d\014\320\213f\235Q\236$\
r"    /* "code/data confusion for $r" */
#endif

#define misc_fatalerr_space1 "\203\002W\136\010\003\050\236\350\035buff\
\004\051"    /* "out of store (for error buffer)" */
#define misc_fatalerr_toomanyerrs "To\023mB\177\350\010s"    /* "Too many errors" */
#define misc_fatalerr_space2 "\203\002W\136\010\003\050\037cc\137\011\201\
c\051\n\050Co\234\100\014\372\255\003\031bugg\036t\275Y\015\020\351e\136\013\
\314 \255\003-\025\322t\022\n \020\351i\020\015\320g\020a\002\0315Wmem\010\
y. Re\241\234\100\036\314\203\002-g\270\314\n \255\003m\010\003\020\373\
iP\013-g\026\322t\022\270\035\314 \255\003p\352gr\227 b\352k\021 \233o\n \
sm\011l\225pieces\270\231\177h\222p.\051"    /* "out of store (in cc_alloc)\n\
(Compilation of the debugging tables requested with the -g option\n\
 requires a great deal of memory. Recompiling without -g, with\n\
 the more restricted -gf option, or with the program broken into\n\
 smaller pieces, may help.)" */
#define misc_fatalerr_space3 "\203\002W\136\010\003\050\037cc\137\011\201\
c\051"    /* "out of store (in cc_alloc)" */
#define pp_fatalerr_hash_error "\043\350\035\021\241\030\017\013\"%s\""    /* "#error encountered \"%s\"" */

#define driver_message_nolisting \
        "Un\275\254\206\322\021 %s \236l\034t\001g\033-l \322tQ\257\n"    /* "Unable to open %s for listing: -l option ignored\n" */
#ifdef NO_ASSEMBLER_OUTPUT
#define driver_message_noasm \
        "Th\335\262s\372\255\003\241\234\100\225doe\015\200s\276p\010\002\
-s\n"    /* "This version of the compiler does not support -s\n" */
#endif
#define driver_message_writefail "C\203ldn\'\002wr\027\003f\100\003\'%s\'\n"    /* "Couldn't write file '%s'\n" */
#define driver_message_oddoption "Un\020\241gn\374\013\322tQ\'%c\'\033\257\n"    /* "Unrecognized option '%c': ignored\n" */
#define driver_message_readfail "C\203ldn\'\002\020a7f\100\003\'%s\'\n"    /* "Couldn't read file '%s'\n" */
/* NB the next error can not arise with the current ARM driver */
#define driver_message_toomanyfiles "To\023mB\177f\100\003\330s"    /* "Too many file args" */
#define driver_message_asmstdout "As\243\357\376\241d\003w\100l g\023\206\
\136d\203t\n"    /* "Assembly code will go to stdout\n" */
#define driver_message_no_listing \
        "-m \322tQ\235eYs\015\314\203\002s\203rc\003l\034t\001g. Ign\010\
\005\n"    /* "-m option useless without source listing. Ignored\n" */
#define driver_message_nomap "-m f\100\003\200ava\100\275\254\035c\010r\
\276\002\176\257\n"    /* "-m file not available or corrupt - ignored\n" */
#define driver_message_notest \
        "Th\335\262s\372\255\003\241\234\100\225doe\015\200s\276p\010\002\
\255\003-tes\002\322t\022\n"    /* "This version of the compiler does not support the -test option\n" */
#define driver_message_needfile "A\002YK\002\006\003f\100\003\330\205\370\
wBt\005\n"    /* "At least one file argument wanted\n" */
#ifndef COMPILING_ON_ARM_OS
#define driver_message_spool "\203tp\310\206c\201g1.\201\025\046 c\201g\
2.\201g\n"    /* "output to clog1.log & clog2.log\n" */
#endif
#define driver_message_testfile "N\023f\100e\015\011\201w\013\314 -te\136\n"    /* "No files allowed with -test\n" */
/* messages generated by misc.c */

#ifndef TARGET_IS_UNIX
#ifndef COMPILING_ON_MPW
#define misc_message_lineno(_f,_l,_s) "\"%s\"\270\337\003%ld\033%s\033"    /* "\"%s\", line %ld: %s: " */,_f,_l,_s
#else
#define misc_message_lineno(_f,_l,_s) "F\100\003\"%s\"\073 L\001\003%ld \
\043 %s\033"    /* "File \"%s\"; Line %ld # %s: " */,_f,_l,_s
#endif
#else
#define misc_message_lineno(_f,_l,_s) "%s\033%ld\033%s\033"    /* "%s: %ld: %s: " */,_f,_l,_s
#endif
#ifndef COMPILING_ON_MPW
#define misc_message_sum1(_f,nx,neq1) "%s\033%ld w\016n\001g%s"    /* "%s: %ld warning%s" */, _f, nx, \
 neq1 ? "":"s"

#else
#define misc_message_sum1(_f,nx,neq1) "\043\043\043 \"%s\"\033%ld w\016\
n\001g%s"    /* "### \"%s\": %ld warning%s" */, _f, nx, \
 neq1 ? "":"s"

#endif
#define misc_message_sum2 " \050\053 %ld s\276p\020\341\005\051"    /* " (+ %ld suppressed)" */
#define misc_message_sum3(nx,neq1) "\270%ld \350\010%s"    /* ", %ld error%s" */, nx, \
 neq1 ? "":"s"

#define misc_message_sum5(nx,neq1) "\270%ld s\004i\203\015\350\010%s\n"    /* ", %ld serious error%s\n" */, nx, \
 neq1 ? "":"s"


#ifdef TARGET_STACK_MOVES_ONCE
/* Cannot be issued if NARGREGS==0 */
#define warn_untrustable "\030tru\136\275\254\241d\003g\021\004\266\236\
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

#define lex_warn_force_unsigned "%s \347%sul\31632-b\242\375Ym\244\014\022"    /* "%s treated as %sul in 32-bit implementation" */
#define lex_warn_multi_char "\321p\010t\275\254\176\2001 \345\316\'\343\'"    /* "non-portable - not 1 char in '...'" */
#define lex_warn_cplusplusid "C\053\053 keyw\0107\235\013\226\253\244\271\
i\004\033$r"    /* "C++ keyword used as identifier: $r" */

#define syn_warn_hashif_undef "Un\220\013\231cr\023\'%s\032\037\043i\026\
\176\3470"    /* "Undefined macro '%s' in #if - treated as 0" */
#define syn_warn_invent_extern "\001v\244\036\'\174\367 \001\002%s\050\051\
\073\'"    /* "inventing 'extern int %s();'" */
#define syn_warn_unary_plus "Un\016\177\'\053\032\335\320fe\014\361\003\
WANSI C"    /* "Unary '+' is a feature of ANSI C" */
#define syn_warn_spurious_braces "sp\361i\203\015\173\175 \016o\332sc\011\
\016 \326\004"    /* "spurious {} around scalar initialiser" */
#define syn_warn_dangling_else "DBgl\036\'\222\243\032\001d\211\014e\015\
\214\341ib\254\350\010"    /* "Dangling 'else' indicates possible error" */
#define syn_warn_void_return "\321\365\003\020t\361n\316\321\323\336\252\
\022"    /* "non-value return in non-void function" */
#define syn_warn_use_of_short \
        "\'sh\010t\032s\201w\225\255B \371\032\355\255\335\231\240\001\003\
\050\243\003mBu\011\051"    /* "'short' slower than 'int' on this machine (see manual)" */
#define syn_warn_undeclared_parm \
        "\3445\366\202$r \200\272\013\176\371\032\327\005"    /* "formal parameter $r not declared - 'int' assumed" */
#define syn_warn_old_style "Old-\136y\254\313$r"    /* "Old-style function $r" */
#define syn_warn_give_args "Dep\020c\266\354Q%s\050\250\176giv\003\016\025\
\232\217s"    /* "Deprecated declaration %s() - give arg types" */
#define syn_warn_ANSI_decl "ANSI \136y\254\313\354Q\235\005\270\'%s\050\
\343\051\'"    /* "ANSI style function declaration used, '%s(...)'" */
#define syn_warn_archaic_init "Anci\370\344 W\326\014\022\270\235\003\'\
\075\'"    /* "Ancient form of initialisation, use '='" */
#define syn_warn_untyped_fn "\'\001\002%s\050\051\032\327\013\176\'\323\
\253\032\233\021d\005\077"    /* "'int %s()' assumed - 'void' intended?" */
#define syn_warn_no_named_member "$c h\226n\023n\227\013me\357\004"    /* "$c has no named member" */
#define syn_warn_extra_comma "S\276\004flu\203\015\',\032\037\'\021\205\
\032\354\022"    /* "Superfluous ',' in 'enum' declaration" */
#define syn_warn_struct_padded "padd\036\001s\004t\013\037\373uc\002$b"    /* "padding inserted in struct $b" */

#define vargen_warn_nonull "om\027t\036t\360\100\036\'\\0\032\236%s \133\
%ld\135"    /* "omitting trailing '\\0' for %s [%ld]" */
#define vargen_warn_unnamed_bitfield \
        "Unn\227\265\242fi\2227\326\013\2060"    /* "Unnamed bit field initialised to 0" */
#define vargen_warn_init_non_aggregate \
        "Atte\234\002\206\326\003\321agg\020g\014e"    /* "Attempt to initialise non-aggregate" */

#define lex_err_ioverflow "N\205b\225%s to\023l\330\003\23632-b\242\375\
Ym\244\014\022"    /* "Number %s too large for 32-bit implementation" */
#define lex_err_overlong_fp "G\352\341\376o\262-l\006\025\305\014\036\224\
\002n\205b\004"    /* "Grossly over-long floating point number" */
#define lex_err_fp_syntax1 "D\204\242\020\351ir\013\315\174p\006\370m\016\
k\004"    /* "Digit required after exponent marker" */
#define lex_err_overlong_hex "G\352\341\376o\262-l\006\025h\174a\031cim\
5\363Bt"    /* "Grossly over-long hexadecimal constant" */
#define lex_err_overlong_int "G\352\341\376o\262-l\006\025n\205b\004"    /* "Grossly over-long number" */
#define lex_err_need_hex_dig "H\174 d\204\242\377\005\013\3150x \0350X"    /* "Hex digit needed after 0x or 0X" */
#define lex_err_need_hex_dig1 "M\346h\174 d\204\027\050s\250\315\\x"    /* "Missing hex digit(s) after \\x" */
#define lex_err_backslash_blank \
        "\\\074sp\302e\245\356\\\074t\275\245\016\003\001\277\336\373\036\
esca\217s"    /* "\\<space> and \\<tab> are invalid string escapes" */
#define lex_err_unterminated_string "New\337\003\035\0217Wf\100\003\314\
\037\373\001g"    /* "Newline or end of file within string" */
#define lex_err_bad_hash "m\034pl\302\013p\020p\352ce\341\035\345\302\202\'\
%c\'"    /* "misplaced preprocessor character '%c'" */
#define lex_err_bad_char "\237\345\302\202\0500x%lx \075 \'%c\'\250\037\
s\203rce"    /* "illegal character (0x%lx = \'%c\') in source" */
#define lex_err_bad_noprint_char "\237\345\302\202\050h\174 \241d\0030x\
%x\250\037s\203rce"    /* "illegal character (hex code 0x%x) in source" */
#define lex_err_ellipsis "\050\343\250m\235\002hav\003\174aP\3763 dots"    /* "(...) must have exactly 3 dots" */
#define lex_err_illegal_whitespace "$s \231\177\200hav\003wh\027esp\302\
\003\037\027"    /* "$s may not have whitespace in it" */

#define syn_err_bitsize "b\242s\374\003%ld \237\1761 \327\005"    /* "bit size %ld illegal - 1 assumed" */
#define syn_err_zerobitsize "z\004\023w\253\255 n\227\265\242fi\2227\176\
1 \327\005"    /* "zero width named bit field - 1 assumed" */
#define syn_err_arraysize "Ar\360\177s\374\003%ld \237\1761 \327\005"    /* "Array size %ld illegal - 1 assumed" */
#define syn_err_expected "\362\013$s\317\001s\004t\265e\230\003$l"    /* "expected $s - inserted before $l" */
#define syn_err_expected1 "\362\013$s%s\317\001s\004t\265e\230\003$l"    /* "expected $s%s - inserted before $l" */
#define syn_err_expected2 "\362\013$s \035$s\317\001s\004t\013$s be\230\
\003$l"    /* "expected $s or $s - inserted $s before $l" */
#define syn_err_expecteda "\362\013$s"    /* "expected $s" */
#define syn_err_expected1a "\362\013$s%s"    /* "expected $s%s" */
#define syn_err_expected2a "\362\013$s \035$s"    /* "expected $s or $s" */
#define syn_err_mix_strings "\345 \356w\253\003\050L\"\343\"\250\373\001\
g\015d\023\200\213c\014\021\014e"    /* "char and wide (L\"...\") strings do not concatenate" */
#define syn_err_expected_expr "\074\174p\020\341\022\245\362\265\310\260\
\332$l"    /* "<expression> expected but found $l" */
#ifdef EXTENSION_VALOF
#define syn_err_valof_block \
        "\173 \260l\201w\036\320\325w\100l \334\347VALOF b\201ck"    /* "{ following a cast will be treated as VALOF block" */
#endif
#define syn_err_typedef "\232p\005e\026n\227\003$r \235\013\037\174p\020\
\341Q\213t\174t"    /* "typedef name $r used in expression context" */
#define syn_err_assertion "\137\137\137\274\004t\0500\270$e\051"    /* "___assert(0, $e)" */
#define syn_err_expected_member "Ex\324\013\074me\357\004\245b\310\260\332\
$l"    /* "Expected <member> but found $l" */
#define syn_err_hashif_eof "EOF \200\377w\337\003\315\043i\026\343"    /* "EOF not newline after #if ..." */
#define syn_err_hashif_junk "J\030k \315\043i\026\074\174p\020\341\022\076"    /* "Junk after #if <expression>" */
#define syn_err_initialisers "to\023mB\177\326\004\015\037\173\175 \236\
agg\020g\014e"    /* "too many initialisers in {} for aggregate" */
#define syn_err_initialisers1 "\173\175 m\235\002hav\0031 eYm\370\206\326\
\003sc\011\016"    /* "{} must have 1 element to initialise scalar" */
#define syn_err_default "\'\210ault\032\200\037s\263\240\317\257"    /* "'default' not in switch - ignored" */
#define syn_err_default1 "d\276\256\014\003\'\210ault\032\251\003\257"    /* "duplicate 'default' case ignored" */
#define syn_err_case "\'\251e\032\200\037s\263\240\317\257"    /* "'case' not in switch - ignored" */
#define syn_err_case1 "d\276\256\266\251\003\363Bt\033%ld"    /* "duplicated case constant: %ld" */
#define syn_err_expected_cmd "\074\241mmBd\245\362\265\310\260\332$l"    /* "<command> expected but found $l" */
#define syn_err_expected_while "\'wh\100e\032\362\013\315\'do\032b\310\260\
\332$l"    /* "'while' expected after 'do' but found $l" */
#define syn_err_else "M\034pl\302\013\'\222\243\032\257"    /* "Misplaced 'else' ignored" */
#define syn_err_continue "\'\213t\001ue\032\200\037\201\322\317\257"    /* "'continue' not in loop - ignored" */
#define syn_err_break "\'b\020ak\032\200\037\201\322 \035s\263\240\317\257"    /* "'break' not in loop or switch - ignored" */
#define syn_err_no_label "\'goto\032\200\260l\201w\265\177l\275\222\317\
\257"    /* "'goto' not followed by label - ignored" */
#define syn_err_no_brace "\'\173\032W\313bod\177\362\013\176\260\332$l"    /* "'{' of function body expected - found $l" */
#define syn_err_stgclass \
        "\136\010ag\003\216K\015$s \200p\004m\027t\013\037\213t\174\002\
%s\317\257"    /* "storage class $s not permitted in context %s - ignored" */
#define syn_err_stgclass1 "\136\010ag\003\216K\015$s \001\241\234\014ib\
\254\314 $m\317\257"    /* "storage class $s incompatible with $m - ignored" */
#define syn_err_typeclash "\312$s \001\213s\034t\370\314 $m"    /* "type $s inconsistent with $m" */
#define syn_err_tag_brace \
        "\'\173\032\035\074\253\244\271i\004\245\362\013\315$s b\310\260\
\332$l"    /* "'{' or <identifier> expected after $s but found $l" */
#define syn_err_expected3 "Ex\324\036\074\354\010\245\035\074\232\217\245\
b\310\260\332$l"    /* "Expecting <declarator> or <type> but found $l" */
#define syn_err_unneeded_id \
        "s\276\004flu\203\015$l\316\074\275\373\302\002\354\010\245\176\
\257"    /* "superfluous $l in <abstract declarator> - ignored" */
#define syn_err_undef_struct(_m,_b,_s) \
        "\030\220\013$c %s\033$r"    /* "undefined $c %s: $r" */, _b, errs_membobj(_m), _s
#define syn_err_selfdef_struct(_m,_b,_s) \
        "\014te\234\002\206\001\216ud\003$c %s\033$r \314\037\027s\222f"    /* "attempt to include $c %s: $r within itself" */, \
        _b, errs_membobj(_m), _s
#define syn_err_void_object(_m,_s) "\237\'\323\253\032%s\033$r"    /* "illegal 'void' %s: $r" */, errs_membobj(_m), _s
#define syn_err_duplicate_type \
        "d\276\256\014\003\312s\217c\271\211\014\372\3445\366\202$r"    /* "duplicate type specification of formal parameter $r" */
#define syn_err_not_a_formal "N\264\3445$r\316\366\017-\232\217-s\217c\271\
i\004"    /* "Non-formal $r in parameter-type-specifier" */
#define syn_err_cant_init "$m v\016i\275Y\015\231\177\200\334\326\005"    /* "$m variables may not be initialised" */
#define syn_err_enumdef \
        "\074\253\244\271i\004\245\362\265\310\260\332$l\316\'\021\205\032\
\220\027\022"    /* "<identifier> expected but found $l in 'enum' definition" */
#define syn_err_undef_enum "Un\220\013\021\205 $r"    /* "Undefined enum $r" */
#define syn_err_misplaced_brace "M\034pl\302\013\'\173\032a\002t\322 Yv\
\222\317\246\036b\201ck"    /* "Misplaced '{' at top level - ignoring block" */

#define vargen_err_long_string "\373\036\326\225l\006g\225\255B %s \133\
%ld\135"    /* "string initialiser longer than %s [%ld]" */
#define vargen_err_nonstatic_addr \
        "\321\136\014\211 add\020s\015$b\316\224\202\326\004"    /* "non-static address $b in pointer initialiser" */
#define vargen_err_bad_ptr "$s\033\237\235\003\037\224\202\326\004"    /* "$s: illegal use in pointer initialiser" */
#define vargen_err_init_void "\307eP\015W\312\'\323\253\032cB \200\334\326\
\005"    /* "objects of type 'void' can not be initialised" */
#define vargen_err_undefined_struct \
        "$c m\235\002\334\220\013\236\050\136\014\211\250v\016i\275\254\
\354\022"    /* "$c must be defined for (static) variable declaration" */
#define vargen_err_open_array "Un\326\013\136\014\211 \133\135 \016\360\
y\015\100\212\011"    /* "Uninitialised static [] arrays illegal" */
#define vargen_err_overlarge_reg "g\201b5\020g\034\202n\205b\225to\023l\
\330e"    /* "global register number too large" */
#define vargen_err_not_int "\001\277\336\312\236g\201b5\001\002\020g\034\
\017"    /* "invalid type for global int register" */
#define vargen_err_not_float "\001\277\336\312\236g\201b5\305a\002\020g\
\034\017"    /* "invalid type for global float register" */
#ifdef TARGET_CALL_USES_DESCRIPTOR
#define vargen_err_badinit "\237\326\014Q\206$r%\053ld"    /* "illegal initialisation to $r%+ld" */
#endif
#ifdef TARGET_IS_HELIOS
#define vg_err_dynamicinit "In\027\306\013dyn\227\211 \016\360\177\314 \
-ZR \035-ZL"    /* "Initialised dynamic array with -ZR or -ZL" */
#endif
#define vargen_rerr_nonaligned \
        "N\264\011\215\013ADCON a\002d\014a\0530x%lx \050\365\003$r\053\
0x%lx\250\243\002\206NULL"    /* "Non-aligned ADCON at data+0x%lx (value $r+0x%lx) set to NULL" */
#define vargen_rerr_datadata_reloc \
       "RISC OS \050\035o\255\004\250\020\244rB\002modu\254h\226\136\014\
\211 \273. \206d\014\320$r"    /* "RISC OS (or other) reentrant module has static init. to data $r" */

#define lex_rerr_8_or_9 "d\204\2428 \0359 \260\332\037oP5n\205b\004"    /* "digit 8 or 9 found in octal number" */
#define lex_rerr_pp_number "n\205b\225\100\212\011\376\260l\201w\265\177\
Yt\017"    /* "number illegally followed by letter" */
#define lex_rerr_hex_exponent "h\174 n\205b\225cB\200hav\003\174p\006\244"    /* "hex number cannot have exponent" */
#define lex_rerr_esc16_truncated \
        "o\262l\330\003esca\261\'\\x%s%lx\032\347\'\\x%lx\'"    /* "overlarge escape '\\x%s%lx' treated as '\\x%lx'" */
#define lex_rerr_esc8_truncated "o\262l\330\003esca\261\'\\%o\032\347\'\\\
%o\'"    /* "overlarge escape '\\%o' treated as '\\%o'" */
#define lex_rerr_illegal_esc "\237\373\036esca\261\'\\%c\032\176\347%c"    /* "illegal string escape '\\%c' - treated as %c" */
#define lex_rerr_not1wchar "L\'\343\032\377\005\015\174aP\3761 w\253\003\
\345\302\017"    /* "L'...' needs exactly 1 wide character" */
#define lex_rerr_empty_char "n\023\345\015\037\345\302\202\363B\002\'\'"    /* "no chars in character constant ''" */
#define lex_rerr_overlong_char "m\010\003\255B 4 \345\015\037\345\302\202\
\363Bt"    /* "more than 4 chars in character constant" */

#define syn_rerr_array_0 "\016\360\177\1330\135 \260\030d"    /* "array [0] found" */
#ifdef EXTENSION_VALOF
#define syn_rerr_void_valof "\323\336\277Wb\201ck\015\016\003\200p\004m\
\027t\005"    /* "void valof blocks are not permitted" */
#endif
#define syn_rerr_undeclared "\030\272\013n\353\270\001v\244\036\'\174\367 \
\001\002%s\'"    /* "undeclared name, inventing 'extern int %s'" */
#define syn_rerr_insert_parens \
        "\340\244he\243\015\050\221\250\001s\004t\013\016o\332\174p\020\
\341Q\260l\201w\036$s"    /* "parentheses (..) inserted around expression following $s" */
#define syn_rerr_return "\020t\361n \074\174pr\245\237\236\323\336\252\022"    /* "return <expr> illegal for void function" */
#define syn_rerr_qualified_typedef(_b,_m) \
        "$m \232p\005e\026$b h\226$m \020-s\217c\271i\005"    /* "$m typedef $b has $m re-specified" */, _m, _b, _m
#define syn_rerr_missing_type "m\346\312s\217c\271\211\014Q\176\371\032\
\327\005"    /* "missing type specification - 'int' assumed" */
#define syn_rerr_long_float "ANSI C doe\015\200s\276p\010\002\'l\006\025\
\305\014\'"    /* "ANSI C does not support 'long float'" */
#define syn_rerr_missing_type1 \
        "om\027t\013\074\232\217\245be\230\003\3445\354\035\176\371\032\
\327\005"    /* "omitted <type> before formal declarator - 'int' assumed" */
#define syn_rerr_missing_type2 \
        "\313p\352to\312\3445$r \377\005\015\312\035\216K\015\176\371\032\
\327\005"    /* "function prototype formal $r needs type or class - 'int' assumed" */
#define syn_rerr_ellipsis_first "\222lips\335\050\343\250cB\200\334\006\
\376\366\017"    /* "ellipsis (...) cannot be only parameter" */
#define syn_rerr_mixed_formals "p\352to\312\356old-\136y\254\366\017\015\
mix\005"    /* "prototype and old-style parameters mixed" */
#define syn_rerr_open_member "\237\133\135 me\357\004\033$r"    /* "illegal [] member: $r" */
#define syn_rerr_ref_void "\237\312\050\323\336\046\250\347\050\001\002\
\046\051"    /* "illegal type (void &) treated as (int &)" */
#define syn_rerr_ill_ref "$t W\020f\004\021c\003\237-\176\'\046\032\257"    /* "$t of reference illegal -- '&' ignored" */
#define syn_rerr_fn_returntype "\313\020t\361n\036$t \237-\176\327\036\364"    /* "function returning $t illegal -- assuming pointer" */
#define syn_rerr_array_elttype "\016\360\177W$t \237-\176\327\036\364"    /* "array of $t illegal -- assuming pointer" */
#define syn_rerr_fn_ptr(_m,_s) \
   "%s $r \231\177\200\334\313-\176\327\036\364"    /* "%s $r may not be function -- assuming pointer" */, errs_membobj(_m), _s
#define syn_rerr_fn_ptr1 \
        "\313$r \231\177\200\334\326\013\176\327\036\313\364"    /* "function $r may not be initialised - assuming function pointer" */
#define syn_rerr_archaic_init "Anci\370\344 W\326\014\022\270\235\003\'\
\075\'"    /* "Ancient form of initialisation, use '='" */
#define syn_rerr_bitfield "\237b\242fi\2227\312$t\317\371\032\327\005"    /* "illegal bit field type $t - 'int' assumed" */
#define syn_rerr_ANSIbitfield "ANSI C \230b\253\015b\242fi\2227\312$t"    /* "ANSI C forbids bit field type $t" */
#define syn_rerr_missing_formal "\3445n\227\003m\346\037\313\220\027\022"    /* "formal name missing in function definition" */
#define syn_rerr_ineffective "\354Q\314 n\023effeP"    /* "declaration with no effect" */
#define syn_rerr_duplicate_member(sv,_b) "d\276\256\014\003me\357\225$r \
W$c"    /* "duplicate member $r of $c" */, sv, _b
#define syn_rerr_missing_type3 \
        "\312\035\216K\015\377\005\013\050\174cep\002\037\313\220\027\022\
\250\176\371\032\327\005"    /* "type or class needed (except in function definition) - 'int' assumed" */
#define syn_rerr_semicolon_in_arglist \
        "\',\032\050\200\'\073\'\250\243\340\014e\015\3445\366\017s"    /* "',' (not ';') separates formal parameters" */
#define syn_rerr_no_members "$c h\226n\023me\357\004s"    /* "$c has no members" */

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
 * m68k/mcerrs.h - prototype for machine-specific error messages file
 */

  /* Ordinary error messages - mapped onto numeric codes */

/*
 * The following message is always machine specific since it may include
 * information about the source of support for the product.
 */
#define misc_disaster_banner   "\n\311\311\311\311\207\052\n\052 Th\003\
\241\234\100\225h\226\031teP\013B \001\3675\001\213s\034t\021cy.\301Th\335\
cB occ\361\301\052\n\052 beca\235\003\242h\226r\030 \203\002W\320v\0275\
\020s\203rc\003su\240 \226mem\010\177\035d\034k\301\301 \052\n\052 sp\302\
\003\035beca\235\003\255\004\003\335\320faul\002\037\027.\301I\026y\203 \
cB\200eK\100\177\011\202 \052\n\052 y\203r p\352gr\227 \206a\323\336ca\235\
\036\255\335r\016\003fa\100u\020\270pYK\003\213t\302\002y\203r\301\052\n\
\052 \031\011\004.\301Th\003\031\011\225\231\177\334\275\254\206h\222p \
y\203 imm\005i\014\222\177\356w\100l \334\301\052\n\052 \275\254\206\020\
p\010\002\320s\235\324\013\241\234\100\225faul\002\206\255\003s\276p\010\
\002c\244\020.\301\301\301\052\n\311\311\311\311\207\052\n\n"    /* "\n\
*************************************************************************\n\
* The compiler has detected an internal inconsistency.  This can occur  *\n\
* because it has run out of a vital resource such as memory or disk     *\n\
* space or because there is a fault in it.  If you cannot easily alter  *\n\
* your program to avoid causing this rare failure, please contact your  *\n\
* dealer.  The dealer may be able to help you immediately and will be   *\n\
* able to report a suspected compiler fault to the support centre.      *\n\
*************************************************************************\n\
\n" */

  /* System failure messages - text not preserved */

#define syserr_branch_round_literals "branch round literals"
#define syserr_big_branch "branch offset too large %lx"
#define syserr_big_displacement "displacement out of range %ld"
#define syserr_labref_type "Unknown label reference type %.8lx"
#define syserr_local_base "local_base %lx"
#define syserr_local_address "local_address %lx"
#define syserr_local_addr "local_addr"
#define syserr_regnum "Invalid register number %ld"
#define syserr_outHW "outHW(%lx)"
#define syserr_ill_instr "Illegal bits set in instruction field (%lx,%lx)"
#define syserr_addr_disp "Address register displacement out of range"
#define syserr_invalid_index_mode "Invalid index mode extension field"
#define syserr_pc_disp "PC displacement out of range"
#define syserr_eff_addr "Bad effective address mode for extension field"
#define syserr_data_sym "Unable to find another data symbol at %ld"
#define syserr_enter "emit(J_ENTER %ld)"
#define syserr_ldrrk "m68kgen(unsigned LDRR/K"
#define syserr_remove_noops "remove_noops(MOVR r,r) failed"
#define syserr_silly_shift "Silly shift value %ld"
#define syserr_setsp_confused "SETSP confused %ld!=%ld %ld"
#define syserr_illegal_jopmode "Illegal JOP mode(%lx)"
#define syserr_remove_fpnoops "remove_noops(MOVF/DR r,r) failed"
#define syserr_unimp_jopmode "Non-implemented JOP mode(%lx)"
#define syserr_fp_reg "Attempt to use non fp reg for fp value"
#define syserr_pr_asmname "pr_asmname"
#define syserr_asmlab "odd asmlab(%lx)"
#define syserr_display_asm "display_asm(%lx)"
#define syserr_asm_trailer "asm_trailer(%ld)"
#define syserr_asm_datalen "asm_data len=%ld"
#define syserr_asm_trailer1 "asm_trailer(%ldF%ld)"

/* end of m68k/mcerrs.h */
#ifndef NO_INSTORE_FILES
#  define NO_INSTORE_FILES 1
#endif

#define COMPRESSED_ERROR_MESSAGES 1

#ifdef DEFINE_ERROR_COMPRESSION_TABLE

static unsigned short int ecompression_info[256] = {
    0x0000, 0x696e, 0x7420, 0x6520, 0x6572, 0x6564, 0x6f6e, 0x2a2a, 
    0x6f72, 0x616c, 0x000a, 0x0520, 0x6174, 0x7320, 0x6172, 0x7404, 
    0x7265, 0x656e, 0x6906, 0x6f20, 0x0707, 0x6720, 0x6620, 0x6974, 
    0x756e, 0x6465, 0x2720, 0x3a20, 0x6973, 0x0820, 0x0115, 0x0120, 
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0920, 0x0036, 0x6420, 
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
    0x696c, 0x0041, 0x616e, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
    0x0048, 0x0049, 0x004a, 0x6173, 0x004c, 0x004d, 0x004e, 0x004f, 
    0x6374, 0x1220, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x6f16, 
    0x0058, 0x6c65, 0x005a, 0x005b, 0x005c, 0x005d, 0x7374, 0x005f, 
    0x6e6f, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
    0x0078, 0x0079, 0x007a, 0x007b, 0x6578, 0x007d, 0x2d20, 0x7920, 
    0x6002, 0x6c6f, 0x0f20, 0x6f75, 0x6967, 0x756d, 0x7413, 0x1414, 
    0x1966, 0x6963, 0x5967, 0x6306, 0x706f, 0x846e, 0x636c, 0x7065, 
    0x8801, 0x2e2e, 0x656c, 0x8a35, 0x8c01, 0x0420, 0x610d, 0x616d, 
    0x6608, 0x6d61, 0x7479, 0x0174, 0x6d70, 0x7573, 0x661d, 0x4093, 
    0x6368, 0x636f, 0x6902, 0x7365, 0x1174, 0x3e20, 0x8d08, 0x1850, 
    0x2920, 0x634b, 0x66a7, 0x6964, 0x6c03, 0x7468, 0x6c89, 0xa605, 
    0x666f, 0x7003, 0x7604, 0x7717, 0x062d, 0x0b62, 0x0c0b, 0x198e, 
    0x2c20, 0x6966, 0xb70e, 0x0117, 0x4b73, 0x6162, 0x7570, 0x7609, 
    0x091c, 0x2020, 0x6163, 0x6166, 0x626a, 0x6681, 0x69c0, 0x6fc4, 
    0x7502, 0x8787, 0x9ab1, 0xaa51, 0xb368, 0xc382, 0x201f, 0x207e, 
    0x6120, 0x6eb4, 0x6f70, 0x766f, 0x8f50, 0xa902, 0xbbc6, 0xbc85, 
    0x0e67, 0x10b6, 0x1837, 0x1c73, 0x6203, 0x690d, 0x6937, 0x6c01, 
    0x700e, 0x7373, 0x74d9, 0x912e, 0x986d, 0xa00e, 0xdb1e, 0xe296, 
    0x0472, 0x7175, 0x726f, 0x9765, 0xba0c, 0x0620, 0x4237, 0x6d62, 
    0x7261, 0x7572, 0x7cd4, 0x8b5e, 0x940f, 0xbf75, 0xe0eb, 0x0f6e, 
    0x1102, 0x279b, 0x5157, 0x5e72, 0x697a, 0x699c, 0x6c7f, 0x6e65};


#endif

#define MAXSTACKDEPTH 4

#endif /* already loaded */

/* end of errors.h */
