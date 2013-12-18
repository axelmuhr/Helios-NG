
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


      /* Map strings to offsets in compressed string table */

#define bind_warn_extern_clash			"\206\025n\210l\032\226$r\337$r\210l\032\
\226\050ANSI 6\210h\023 m\006oc\032e\051"    /* "extern clash $r, $r clash (ANSI 6 char monocase)" */
#define bind_warn_unused_static_decl		"\027\251\021e\023li\246\100\014\233 \
\351\023\372\201$r"    /* "unused earlier static declaration of $r" */
#define bind_warn_not_in_hdr			"\206\025\322$r \202\351a\013\005\275he\245\
\002"    /* "extern $r not declared in header" */
#define bind_warn_main_not_int			"\206\025\322\'ma\001\204ne7\017Qb\004\'\
\277\204\257\024"    /* "extern 'main' needs to be 'int' function" */
#define bind_warn_label_not_used		"l\267\237 $r w\254\260\021\255\003\202\
\2517"    /* "label $r was defined but not used" */
/*
 * Note that when part of an error string MUST be stored as a regular
 * non-compressed string I have to inform the GenHdrs utility with %Z and %O
 * This arises when a string contains literal strings as extra sub-args.
 */
#define bind_warn_not_used(is_typedef,is_fn,binder) \
        "%s $b \351a\013\005\255\003\202\2517"    /* "%s $b declared but not used" */, \
      ((is_typedef) ? "typedef" : ((is_fn) ? "function" : "variable")),\
      binder
#define bind_warn_static_not_used 		"\100\014\233 $b \351a\013\005\255\003\
\202\2517"    /* "static $b declared but not used" */
#define cg_warn_implicit_return			"i\242\354\263\013t\365n\261\345voi\005\
%s\050\051"    /* "implicit return in non-void %s()" */
#define flowgraf_warn_implicit_return		"i\242\354\263\013t\365n\261\345\
voi\005\257\024"    /* "implicit return in non-void function" */
#define pp_warn_triglyph			"ANSI \'%c%c%c\204\273\220rap\226\265\'%c\204\
\375\005\215w\254\264\276\277\016\031d\077"    /* "ANSI '%c%c%c' trigraph for '%c' found - was this intended?" */
#define pp_warn_nested_comment			"ch\023\036\214\213qu\016c\004%s\140si\
\305\262mm\300"    /* "character sequence %s inside comment" */
#define pp_warn_many_arglines			"\050\212ssib\302\002r\007\051\011\076\075 \
%lu l\001e\017\201m\036r\020\360\231\300s"    /* "(possible error): >= %lu lines of macro arguments" */
#define pp_warn_redefinition			"\013\200\334\260\037\174\201\043\260\004\
m\036r\020%s"    /* "repeated definition of #define macro %s" */
#define pp_rerr_redefinition			"di\312\002\026\013\260\037\174\201\043\260\
\004m\036r\020%s"    /* "differing redefinition of #define macro %s" */
#define pp_rerr_nonunique_formal		"d\330\354\014\004m\036r\020\274\225\355\
\236e\025\011\'%s\'"    /* "duplicate macro formal parameter: '%s'" */
#define pp_rerr_define_hash_arg			"\234\002\034\005\201\043 \202m\036r\020\
\274\225\355\236e\025"    /* "operand of # not macro formal parameter" */
#define pp_rerr_define_hashhash			"\043\043 \320rs\003\247l\032\003tok\016\
\261\043\260\004body"    /* "## first or last token in #define body" */
#define pp_warn_ifvaldef			"\043if\031\035%s may\140d\233\014\004\273\216\
b\203\250."    /* "#ifdef %s may indicate trouble..." */ /* MACH_EXTNS */
#define pp_warn_nonansi_header			"N\006-ANSI \043\001\271u\305\074%s\076"    /* "Non-ANSI #include <%s>" */
#define pp_warn_bad_pragma			"Un\013\262gn\030\021\043pragm\227\050n\020\'\
-\204\247\335\322w\007d\051"    /* "Unrecognised #pragma (no '-' or unknown word)" */
#define pp_warn_bad_pragma1			"Un\013\262gn\030\021\043pragm\227-%c"    /* "Unrecognised #pragma -%c" */
#define pp_warn_unused_macro			"\043\260\004m\036r\020\'%s\204\260\021\255\
\003\202\2517"    /* "#define macro '%s' defined but not used" */
#define regalloc_warn_use_before_set		"$b m\363b\004\251\021be\176\004b\
e\026\213t"    /* "$b may be used before being set" */
#define regalloc_warn_never_used		"$b \276\314\255\003nev\246\2517"    /* "$b is set but never used" */
#define sem_warn_unsigned			"ANSI s\365pr\030e\011\'l\006g\204$s \'\027\
s\2357\204yi\237d\017\'l\006g\'"    /* "ANSI surprise: 'long' $s 'unsigned' yields 'long'" */
#define sem_warn_format_type			"\036tu\225\310$t m\030m\014che\017\274\352\'\
%.\052s\'"    /* "actual type $t mismatches format '%.*s'" */
#define sem_warn_bad_format			"Il\241\225\274\352\270\324s\174\'%%%c\'"    /* "Illegal format conversion '%%%c'" */
#define sem_warn_incomplete_format		"In\262\242\203t\004\274\352\317\371"    /* "Incomplete format string" */
#define sem_warn_format_nargs			"F\007m\352\013qui\013\017%ld \355\236e\
\025%s\337\255\003%ld giv\016"    /* "Format requires %ld parameter%s, but %ld given" */
#define sem_warn_addr_array			"\'\046\204\027necess\023\205\265\326\247\
\023r\363$e"    /* "'&' unnecessary for function or array $e" */
#define sem_warn_bad_shift( m, n )		"shif\003\201$m b\205%ld \027\260\021\
\275ANSI C"    /* "shift of $m by %ld undefined in ANSI C" */,m,n
#define sem_warn_divrem_0			"div\030\174b\205z\002o\011$s"    /* "division by zero: $s" */
#define sem_warn_ucomp_0			"od\005\027s\235\021\262\242\023\030\006 \307\
\2260\011$s"    /* "odd unsigned comparison with 0: $s" */
#define sem_warn_fp_overflow( op )		"\344\327\003\270\100\034\003o\324\256\
w\011$s"    /* "floating point constant overflow: $s" */,op
#define sem_rerr_udiad_overflow(op,a,b,c)	"\027s\235\021\270\100\034\003\
o\324\256w\011$s"    /* "unsigned constant overflow: $s" */,op
#define sem_rerr_diad_overflow(op,a,b,c)	"s\235\021\270\100\034\003o\324\
\256w\011$s"    /* "signed constant overflow: $s" */,op
#define sem_rerr_umonad_overflow(op,a,b)	"\027s\235\021\270\100\034\003\
o\324\256w\011$s"    /* "unsigned constant overflow: $s" */,op
#define sem_rerr_monad_overflow(op,a,b)		"s\235\021\270\100\034\003o\324\
\256w\011$s"    /* "signed constant overflow: $s" */,op
#define sem_rerr_implicit_cast_overflow(t,a,b)	"i\242\354\263c\032\003\050\
Q$t\336o\324\256w"    /* "implicit cast (to $t) overflow" */,t
#define sem_warn_fix_fail			"\344\350\277egr\015\210\006\324s\174faW7"    /* "floating to integral conversion failed" */
#define sem_warn_index_ovfl			"\216t-of-b\323\005o\312\314%ld\261\347s"    /* "out-of-bound offset %ld in address" */
#define sem_warn_low_precision			"\177w\246p\013c\030\024\261w\364\002\210\
\006t\206t\011$s"    /* "lower precision in wider context: $s" */
#define sem_warn_odd_condition			"\251\004\201$s\140\210\006d\037\174\270\
t\206t"    /* "use of $s in condition context" */
#define sem_warn_void_context			"n\020si\305e\312ec\003\275voi\005\270t\
\206t\011$s"    /* "no side effect in void context: $s" */
#define sem_warn_olde_mismatch			"\360\231\016\003\034\005\240d-\100y\302\
\355\236e\214m\030m\014ch\011$e"    /* "argument and old-style parameter mismatch: $e" */
#define sem_warn_uncheckable_format \
        "\'\274\014\204\360. Qpr\277f\057sc\034\035etc. \276v\023i\267\203\
\337s\020c\034\202b\004check7"    /* "'format' arg. to printf/scanf etc. is variable, so cannot be checked" */
#define sem_warn_narrowing			"i\242\354\263n\023\343w\026c\032t\011$s"    /* "implicit narrowing cast: $s" */
#define sem_warn_fn_cast			"$s\011c\032\003betwe\016 \326\221\214\034\005\
\345\326obje\136"    /* "$s: cast between function pointer and non-function object" */
#define sem_warn_pointer_int			"\206p\354\263c\032\003\201\221\214Q\'\277\'"    /* "explicit cast of pointer to 'int'" */
#define bind_err_extern_clash			"\206\025n\210l\032\226$r\337$r \050l\001\
k\246%ld\210h\023%s\051"    /* "extern clash $r, $r (linker %ld char%s)" */
#define bind_err_duplicate_tag			"d\330\354\014\004\260\037\174\201$s t\
a\022$b"    /* "duplicate definition of $s tag $b" */
#define bind_err_reuse_tag			"\013-\251\026$s ta\022$b \254$s tag"    /* "re-using $s tag $b as $s tag" */
#define bind_err_incomplete_tentative		"\001\262\242\203t\004t\300\014i\
v\004\351\023\372\201$r"    /* "incomplete tentative declaration of $r" */
#define bind_err_type_disagreement		"\310d\030ag\013em\016\003\265$r"    /* "type disagreement for $r" */
#define bind_err_duplicate_definition		"d\330\354\014\004\260\037\174\201\
$r"    /* "duplicate definition of $r" */
#define bind_err_duplicate_label		"d\330\354\014\004\260\037\174\201l\267\
\237 $r \215\333"    /* "duplicate definition of label $r - ignored" */
#define bind_err_unset_label			"l\267\237 $r h\254\202be\016 \213t"    /* "label $r has not been set" */
#define bind_err_undefined_static		"\100\014\233 \326$b \202\260\021\215\
t\013\334\254\206\025n"    /* "static function $b not defined - treated as extern" */
#define fp_err_very_big				"O\324l\360\004\344\327\003\325u\004\375d"    /* "Overlarge floating point value found" */
#define fp_err_big_single			"O\324l\360\004\050s\371\302p\013c\030\024\336\
\344\327\003\325u\004\375d"    /* "Overlarge (single precision) floating point value found" */
#define pp_err_eof_comment			"EOF\140\210omm\300"    /* "EOF in comment" */
#define pp_err_eof_string			"EOF\261\317\371"    /* "EOF in string" */
#define pp_err_eol_string			"quot\004\050%c\051\140s\002t\021be\176\004\
newl\001e"    /* "quote (%c) inserted before newline" */
#define pp_err_eof_escape			"EOF\261\317\026esca\200"    /* "EOF in string escape" */
#define pp_err_missing_quote			"M\373\'%c\'\261p\013-p\343cess\007\210o\
mm\034\005l\001e"    /* "Missing '%c' in pre-processor command line" */
#define pp_err_if_defined			"N\020\364\300i\320\246\356\043i\035\2607"    /* "No identifier after #if defined" */
#define pp_err_if_defined1			"N\020\'\051\204\356\043i\035\2607\050\250\
."    /* "No ')' after #if defined(..." */
#define pp_err_rpar_eof				"M\373\'\051\204\356%s\050\250. \006 l\001\004\
%ld"    /* "Missing ')' after %s(... on line %ld" */
#define pp_err_many_args			"T\377m\034\205\360\231\300\017Qm\036r\020%s\
\050\250. \006 l\001\004%ld"    /* "Too many arguments to macro %s(... on line %ld" */
#define pp_err_few_args				"T\377few \360\231\300\017Qm\036r\020%s\050\250\
. \006 l\001\004%ld"    /* "Too few arguments to macro %s(... on line %ld" */
#define pp_err_missing_identifier		"M\373\364\300i\320\246\356\043\260e"    /* "Missing identifier after #define" */
#define pp_err_missing_parameter		"M\373\355\236e\214n\236\004\275\043\260\
\004%s\050\250."    /* "Missing parameter name in #define %s(..." */
#define pp_err_missing_comma			"M\373\',\204\247\'\051\204\356\043\260\004\
%s\050\250."    /* "Missing ',' or ')' after #define %s(..." */
#define pp_err_undef				"M\373\364\300i\320\246\356\043\027\244"    /* "Missing identifier after #undef" */
#define pp_err_ifdef				"M\373\364\300i\320\246\356\043if\244"    /* "Missing identifier after #ifdef" */
#define pp_err_include_quote			"M\373\'\074\204\247\'\"\204\356\043\001\
\271u\031"    /* "Missing '<' or '\"' after #include" */
#define pp_err_include_junk			"J\027\211\356\043\001\271u\305%c%s%c"    /* "Junk after #include %c%s%c" */
#define pp_err_include_file			"\043\001\271u\305fW\004%c%s%c w\216ldn\'\
\003\234\016"    /* "#include file %c%s%c wouldn't open" */
#define pp_err_unknown_directive		"Un\321\322di\013\136ive\011\043%s"    /* "Unknown directive: #%s" */
#define pp_err_endif_eof			"M\373\043\207i\035\352EOF"    /* "Missing #endif at EOF" */
#define sem_err_typeclash			"Il\241\225\230\200\017\265\234\002\034ds\011\
$s"    /* "Illegal types for operands: $s" */
#define sem_err_sizeof_struct			"siz\004\201$c ne7\021\255\003\202ye\003\
\2607"    /* "size of $c needed but not yet defined" */
#define sem_err_lvalue				"Il\241\015\261l\325ue\011\326\247\023r\363$e"    /* "Illegal in lvalue: function or array $e" */
#define sem_err_bitfield_address		"b\263\320\237d\017d\020\202hav\004\347\
\213s"    /* "bit fields do not have addresses" */
#define sem_err_lvalue1				"Il\241\015\261l-\325ue\011\'\016\231\204\270\
\100\034\003$b"    /* "Illegal in l-value: 'enum' constant $b" */
#define sem_err_lvalue2				"Il\241\015\261\264\004\270t\206\003\201\361\
l-\325ue\011$s"    /* "Illegal in the context of an l-value: $s" */
#define sem_err_nonconst			"\253\015\261%s\011\074\335n\076"    /* "illegal in %s: <unknown>" */
#define sem_err_nonconst1			"\253\015\261%s\011\303\210\006\100\034\003\
$b"    /* "illegal in %s: non constant $b" */
#define sem_err_nonconst2			"\253\015\261%s\011$s"    /* "illegal in %s: $s" */
#define sem_err_nonfunction			"\014te\242\003Qapp\376\227\345\257\024"    /* "attempt to apply a non-function" */
#define sem_err_void_argument			"\'vo\364\204\325ue\017m\363\202b\004\360\
\231\300s"    /* "'void' values may not be arguments" */
#define sem_err_bad_cast			"$s\011\253\015\210\032\003\201$t Q\221\025"    /* "$s: illegal cast of $t to pointer" */
#define sem_err_bad_cast1			"$s\011\253\015\210\032\003Q$t"    /* "$s: illegal cast to $t" */
#define sem_err_bad_cast2			"$s\011c\032\003Q\345equ\225$t \253\015"    /* "$s: cast to non-equal $t illegal" */
#define sem_err_undef_struct			"$c \202ye\003\260\021-\210\034\202b\004\
\213\203\340f\343m"    /* "$c not yet defined - cannot be selected from" */
#define sem_err_unknown_field			"$c h\254n\020$r \320\237d"    /* "$c has no $r field" */
#define errs_membobj(m)\
  (m ? "member":"object")


#define bind_rerr_undefined_tag 		"$s ta\022$b \202\2607"    /* "$s tag $b not defined" */
#define bind_rerr_linkage_disagreement		"l\001kag\004d\030ag\013em\016\003\
\265$r \215t\013\334\254$m"    /* "linkage disagreement for $r - treated as $m" */
#define bind_rerr_local_extern			"\206\025\322$r m\030m\014che\017t\234\
-\203v\237 \351\023\014\024"    /* "extern $r mismatches top-level declaration" */
#define fp_rerr_very_small			"sm\015l \344\327\003\325u\004\270\324t\021\
Q0.0"    /* "small floating point value converted to 0.0" */
#define fp_rerr_small_single			"sm\015l \050s\371\302p\013c\030\024\336\
\344\026\325u\004\270\324t\021Q0.0"    /* "small (single precision) floating value converted to 0.0" */
#define pp_rerr_newline_eof			"m\373newl\001\004be\176\004EOF -\140s\002\
t7"    /* "missing newline before EOF - inserted" */
#define pp_rerr_nonprint_char			"\027pr\277\267\302ch\023 %\043.2x \375\
\005\215\333"    /* "unprintable char %#.2x found - ignored" */
#define pp_rerr_illegal_option			"\253\225\234t\174-D%s%s"    /* "illegal option -D%s%s" */
#define pp_rerr_spurious_else			"sp\365i\216\017\043\237s\004\333"    /* "spurious #else ignored" */
#define pp_rerr_spurious_elif			"sp\365i\216\017\043\237i\035\333"    /* "spurious #elif ignored" */
#define pp_rerr_spurious_endif			"sp\365i\216\017\043\207i\035\333"    /* "spurious #endif ignored" */
#define pp_rerr_hash_line			"n\231b\246m\373\275\043l\001e"    /* "number missing in #line" */
#define pp_rerr_hash_error			"\043\002r\247\016\262\027\025\021\"%s\""    /* "#error encountered \"%s\"" */
#define pp_rerr_hash_ident			"\043\364\016\003\276\202\275ANSI C"    /* "#ident is not in ANSI C" */
#define pp_rerr_junk_eol			"j\027\211\352\016\005\201\043%s l\001\004\215\
\333"    /* "junk at end of #%s line - ignored" */
#define sem_rerr_sizeof_bitfield		"size\201\074b\263\320\237d\304\253\225\
\215sizeof\050\277\336\301\2317"    /* "sizeof <bit field> illegal - sizeof(int) assumed" */
#define sem_rerr_sizeof_void			"siz\004\201\'vo\364\204\013qui\013\005\215\
t\013\334\2541"    /* "size of 'void' required - treated as 1" */
#define sem_rerr_sizeof_array			"siz\004\201\227\133\135 \023r\363\013q\
ui\013d\337t\013\334\254\1331\135"    /* "size of a [] array required, treated as [1]" */
#define sem_rerr_sizeof_function		"siz\004\201\326\013qui\013\005\215t\013\
\334\254siz\004\201\221\025"    /* "size of function required - treated as size of pointer" */
#define sem_rerr_pointer_arith			"\074\277\304$s \074\221\025\304t\013\334\
\254\074\277\304$s \050\277\051\074\221\025\076"    /* "<int> $s <pointer> treated as <int> $s (int)<pointer>" */
#define sem_rerr_pointer_arith1			"\074\221\025\304$s \074\277\304t\013\
\334\254\050\277\051\074\221\025\304$s \074\277\076"    /* "<pointer> $s <int> treated as (int)<pointer> $s <int>" */
#define sem_rerr_assign_const			"\301\235m\016\003Q\'\270\100\204objec\003\
$e"    /* "assignment to 'const' object $e" */
#define sem_rerr_addr_regvar			"\'\357\030\025\204\014\273i\255t\004\265\
$b \315\021wh\016 \347\017tak\016"    /* "'register' attribute for $b ignored when address taken" */
#define sem_rerr_lcast				"obje\136\017\264\352hav\004be\016\210\032\003\
\023\004\202l-\325ues"    /* "objects that have been cast are not l-values" */
#define sem_rerr_pointer_compare		"\262\242\023\030\006 $s \201\221\214\
\034\005\277:\n\374li\025\2250 \050\265\075\075 \034\005\041\075\336\276\
\006\376\241\015\210\032e"    /* "comparison $s of pointer and int:\n\
  literal 0 (for == and !=) is only legal case" */
#define sem_rerr_different_pointers	  	"di\312\002\327\214\230\200s\011\
$s"    /* "differing pointer types: $s" */
#define sem_rerr_wrong_no_args			"wr\006\022n\231b\246\201\355\236e\025\
\017Q$e"    /* "wrong number of parameters to $e" */
#define sem_rerr_implicit_cast1			"$s\011i\242\354\263c\032\003\201\221\
\214Q\345equ\225\221\025"    /* "$s: implicit cast of pointer to non-equal pointer" */
#define sem_rerr_implicit_cast2			"$s\011i\242\354\263c\032\003\201\345\
0\140\003Q\221\025"    /* "$s: implicit cast of non-0 int to pointer" */
#define sem_rerr_implicit_cast3			"$s\011i\242\354\263c\032\003\201\221\
\214Q\'\277\'"    /* "$s: implicit cast of pointer to 'int'" */
#define sem_rerr_implicit_cast4			"$s\011i\242\354\263c\032\003\201$t Q\'\
\277\'"    /* "$s: implicit cast of $t to 'int'" */
#define sem_rerr_cant_balance			"di\312\002\327\214\230\200s\011$s"    /* "differing pointer types: $s" */

#define sem_rerr_void_indirection		"\253\015\140di\013\136\174\006 \050\
voi\005\052\051\011\'\052\'"    /* "illegal indirection on (void *): '*'" */
#define obj_fatalerr_io_object			"I\057O \002r\247\006 objec\003\100\013\
\236"    /* "I/O error on object stream" */
#define compiler_rerr_no_extern_decl		"n\020\206\025n\225\351\023\014\024\
\261\273\034sl\372\027\037"    /* "no external declaration in translation unit" */
#define compiler_fatalerr_io_error		"I\057O \002r\247wr\037\026\'%s\'"    /* "I/O error writing '%s'" */
#define driver_fatalerr_io_object		"I\057O \002r\247\006 objec\003\100\013\
\236"    /* "I/O error on object stream" */
#define driver_fatalerr_io_asm			"I\057O \002r\247\006 \032\213\313l\246\
\216tpu\003\100\013\236"    /* "I/O error on assembler output stream" */
#define driver_fatalerr_io_listing		"I\057O \002r\247\006 l\030t\026\100\013\
\236"    /* "I/O error on listing stream" */
#ifdef TARGET_HAS_AOUT
#define aout_fatalerr_toomany 			"T\377m\034\205sy\313\240\017\265\'a.\216\
t\204\216tput"    /* "Too many symbols for 'a.out' output" */
#define aout_fatalerr_toobig			"Modu\302t\377bi\022\265a.\216\003\274\014\
\025"    /* "Module too big for a.out formatter" */
#endif
#ifdef TARGET_HAS_COFF
#define coff_fatalerr_toomany 			"T\377m\034\205\013\366\014\024\017\265\
COFF \274\352\275.\020fWe"    /* "Too many relocations for COFF format in .o file" */
#define coff_fatalerr_toobig			"Modu\302t\377bi\022\265COFF \274\014\025"    /* "Module too big for COFF formatter" */
#endif
#ifdef TARGET_IS_HELIOS
#define heliobj_warn_12bits 			"O\312\314%ld \30412 b\037s"    /* "Offset %ld > 12 bits" */
#define heliobj_warn_16bits			"O\312\314%ld \30416 b\037s"    /* "Offset %ld > 16 bits" */
#define heliobj_warn_24bits			"O\312\314%ld \30424 b\037s"    /* "Offset %ld > 24 bits" */
#endif
#define misc_fatalerr_space1 			"\216\003\201\100\007\004\050\265\002r\247\
\255\312\002\051"    /* "out of store (for error buffer)" */
#define misc_fatalerr_toomanyerrs		"T\377m\034\205\002r\007s"    /* "Too many errors" */
#define misc_fatalerr_space2			"\216\003\201\100\007\004\050\001\210c\137\015\
\366\051\n\050Co\242W\372\201\264\004\031\255gg\026t\267\203\017\013que\
\100\021\307\226\264\004-\022\234t\024\n \013qui\013\017\227g\013\352\031\
\225\201mem\007y. Re\262\242W\026\307h\216\003-g\337\307h\n \264\004m\007\
\004\013\317i\340-g\035\234t\024\337\247\307\226\264\004p\343gr\236 b\343\
k\016\140to\n sm\015l\246pieces\337m\363h\237p.\051"    /* "out of store (in cc_alloc)\n\
(Compilation of the debugging tables requested with the -g option\n\
 requires a great deal of memory. Recompiling without -g, with\n\
 the more restricted -gf option, or with the program broken into\n\
 smaller pieces, may help.)" */
#define misc_fatalerr_space3 			"\216\003\201\100\007\004\050\001\210c\137\015\
\366\051"    /* "out of store (in cc_alloc)" */
#define pp_fatalerr_hash_error			"\043\002r\247\016\262\027\025\021\"%s\""    /* "#error encountered \"%s\"" */

#define driver_message_nolisting		"Un\267\302Q\234\016 %s \265l\030t\371\011\
-l \234t\174\333\n"    /* "Unable to open %s for listing: -l option ignored\n" */
#ifdef NO_ASSEMBLER_OUTPUT
#define driver_message_noasm 			"Th\276\324s\174\201\264\004\262\242W\246\
doe\017\202s\330p\007\003-s\n"    /* "This version of the compiler does not support -s\n" */
#endif
#define driver_message_writefail 		"C\216ldn\'\003wr\037\004fW\004\'%s\'\n"    /* "Couldn't write file '%s'\n" */
#define driver_message_oddoption		"Un\013\262gniz\021\234t\174\'%c\'\011\
\333\n"    /* "Unrecognized option '%c': ignored\n" */
#define driver_message_readfail			"C\216ldn\'\003\013a\005fW\004\'%s\'\n"    /* "Couldn't read file '%s'\n" */
/* NB the next error can not arise with the current ARM driver */
#define driver_message_toomanyfiles		"T\377m\034\205fW\004\360s"    /* "Too many file args" */
#define driver_message_asmstdout		"As\213\313\376\262\305wWl g\020Q\100\
d\216t\n"    /* "Assembly code will go to stdout\n" */
#define driver_message_no_listing		"-m \234t\174u\213\203s\017\307h\216\
\003s\216rc\004l\030t\371. Ign\0077\n"    /* "-m option useless without source listing. Ignored\n" */
#define driver_message_nomap			"-m fW\004\202avaW\267\302\007\210\007r\330\
\003\215\333\n"    /* "-m file not available or corrupt - ignored\n" */
#define driver_message_notest			"Th\276\324s\174\201\264\004\262\242W\246\
doe\017\202s\330p\007\003\264\004-tes\003\234t\024\n"    /* "This version of the compiler does not support the -test option\n" */
#define driver_message_needfile			"A\003\203\032\003\006\004fW\004\360\231\
\016\003w\034t7\n"    /* "At least one file argument wanted\n" */
#ifndef COMPILING_ON_ARM_OS
#define driver_message_spool 			"\216tpu\003Qc\177g1.\177\022\046\210\177\
g2.\177g\n"    /* "output to clog1.log & clog2.log\n" */
#endif
#define driver_message_testfile 		"N\020fWe\017\015\177w\021\307\226-te\
\100\n"    /* "No files allowed with -test\n" */
/* messages generated by misc.c */

#ifndef TARGET_IS_UNIX
#define misc_message_lineno(f,l,s) 		"\"%s\"\337l\001\004%ld\011%s\011"    /* "\"%s\", line %ld: %s: " */,f,l,s
#else
#define misc_message_lineno(f,l,s) 		"%s\011%ld\011%s\011"    /* "%s: %ld: %s: " */,f,l,s
#endif
#define misc_message_sum1(f,nx,neq1) 		"%s\011%ld w\023n\371%s"    /* "%s: %ld warning%s" */, f, nx, \
 neq1 ? "":"s"

#define misc_message_sum2 			" \050\053 %ld s\330p\266s7\051"    /* " (+ %ld suppressed)" */
#define misc_message_sum3(nx,neq1)		"\337%ld \002r\007%s"    /* ", %ld error%s" */, nx, \
 neq1 ? "":"s"

#define misc_message_sum5(nx,neq1) 		"\337%ld s\002i\216\017\002r\007%s\n"    /* ", %ld serious error%s\n" */, nx, \
 neq1 ? "":"s"


#ifdef TARGET_STACK_MOVES_ONCE
/* Cannot be issued if NARGREGS==0 */
#define warn_untrustable	   		"\027\273u\100\267\302\262\305g\016\002\334\
\265%s"    /* "untrustable code generated for %s" */
#endif

 /* The next batch of things just get mapped onto syserr codes */

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

/* end of miperrs.h */
/*
 * cfe/feerrs.h - prototype for front-end error messages file
 * version 2.
 */

  /* Ordinary error messages - mapped onto numeric codes */

#define lex_warn_force_unsigned "%s t\013\334\254%sul\26132-b\263i\242\203\
m\300\014\024"    /* "%s treated as %sul in 32-bit implementation" */
#define lex_warn_multi_char "\345p\007t\267\302\215\2021\210h\023\261\'\
\250.\'"    /* "non-portable - not 1 char in '...'" */

#define syn_warn_hashif_undef "Un\260\021m\036r\020\'%s\'\261\043i\035\215\
t\013\334\2540"    /* "Undefined macro '%s' in #if - treated as 0" */
#define syn_warn_invent_extern "\001v\300\026\'\206\025n\140\003%s\050\051\
\073\'"    /* "inventing 'extern int %s();'" */
#define syn_warn_unary_plus "Un\023\205\'\053\204\276\227fe\014\365\004\
\201ANSI C"    /* "Unary '+' is a feature of ANSI C" */
#define syn_warn_spurious_braces "sp\365i\216\017\173\175 \023\323\005s\
c\015\023\140\370\002"    /* "spurious {} around scalar initialiser" */
#define syn_warn_dangling_else "D\034gl\026\'\237\213\'\140d\233\014e\017\
\212ssib\302\002r\007"    /* "Dangling 'else' indicates possible error" */
#define syn_warn_void_return "\345\325u\004\013t\365n\261\345voi\005\257\
\024"    /* "non-value return in non-void function" */
#define syn_warn_use_of_short \
        "\'sh\007t\204s\177w\246\264\361\'\277\204\006 \264\276m\036h\001\
\004\050\213\004m\034u\015\051"    /* "'short' slower than 'int' on this machine (see manual)" */
#define syn_warn_enum_unchecked "N\020\310check\026\201\'\016\231\'\261\
\264\276\262\242W\002"    /* "No type checking of 'enum' in this compiler" */
#define syn_warn_undeclared_parm \
        "\274\225\355\236e\214$r \202\351a\013\005\215\'\277\204\301\231\
7"    /* "formal parameter $r not declared - 'int' assumed" */
#define syn_warn_old_style "Old-\100y\302\326$r"    /* "Old-style function $r" */
#define syn_warn_give_args "D\311\013c\334\351\023\372%s\050\336\215giv\
\004\023\022\230\200s"    /* "Deprecated declaration %s() - give arg types" */
#define syn_warn_ANSI_decl "ANSI \100y\302\326\351\023\372\2517\337\'%s\
\050\250.\051\'"    /* "ANSI style function declaration used, '%s(...)'" */
#define syn_warn_archaic_init "Anci\016\003\274 \201\001\370\014\024\337\
\251\004\'\075\'"    /* "Ancient form of initialisation, use '='" */
#define syn_warn_untyped_fn "\'\001\003%s\050\051\204\301\231\021\215\'\
vo\364\'\140t\016\031d\077"    /* "'int %s()' assumed - 'void' intended?" */
#define syn_warn_no_named_member "$c h\254n\020n\236\021me\313\002"    /* "$c has no named member" */
#define syn_warn_extra_comma "S\330\002flu\216\017\',\'\261\'\016\231\204\
\351\023\014\024"    /* "Superfluous ',' in 'enum' declaration" */

#define vargen_warn_nonull "om\037t\026\273aW\026\'\\0\204\265%s \133%l\
d\135"    /* "omitting trailing '\\0' for %s [%ld]" */
#define vargen_warn_unnamed_bitfield \
        "Unn\236\021b\263\320\237\005\001\370\021Q0"    /* "Unnamed bit field initialised to 0" */

#define lex_err_ioverflow "N\231b\246%s t\377l\360\004\26532-b\263i\242\
\203m\300\014\024"    /* "Number %s too large for 32-bit implementation" */
#define lex_err_overlong_fp "G\343ss\376o\324-l\006\022\344\327\003n\231\
b\002"    /* "Grossly over-long floating point number" */
#define lex_err_fp_syntax1 "D\220\263\013qui\013\005\356\206p\006\016\003\
m\023k\002"    /* "Digit required after exponent marker" */
#define lex_err_overlong_hex "G\343ss\376o\324-l\006\022h\206a\031cim\015\
\210\006\100\034t"    /* "Grossly over-long hexadecimal constant" */
#define lex_err_overlong_int "G\343ss\376o\324-l\006\022n\231b\002"    /* "Grossly over-long number" */
#define lex_err_need_hex_dig "H\206 d\220\263ne7\021\3560x \2470X"    /* "Hex digit needed after 0x or 0X" */
#define lex_err_need_hex_dig1 "M\373h\206 d\220\037\050s\336\356\\x"    /* "Missing hex digit(s) after \\x" */
#define lex_err_backslash_blank \
        "\\\074sp\036e\304\034\005\\\074t\267\304\023\004\001\325i\005\317\
\026esca\200s"    /* "\\<space> and \\<tab> are invalid string escapes" */
#define lex_err_unterminated_string "Newl\001\004\247\016\005\201fW\004\
\307h\275\317\371"    /* "Newline or end of file within string" */
#define lex_err_bad_hash "m\030pl\036\021p\013p\343cess\007\210h\023\036\
\214\'%c\'"    /* "misplaced preprocessor character '%c'" */
#define lex_err_bad_char "\253\015\210h\023\036\214\0500x%lx \075 \'%c\'\
\051\261s\216rce"    /* "illegal character (0x%lx = \'%c\') in source" */
#define lex_err_bad_noprint_char "\253\015\210h\023\036\214\050h\206\210\
o\3050x%x\051\261s\216rce"    /* "illegal character (hex code 0x%x) in source" */
#define lex_err_ellipsis "\050\250.\336m\251\003hav\004\206\036t\3763 d\
ots"    /* "(...) must have exactly 3 dots" */
#define lex_err_illegal_whitespace "$s m\363\202hav\004wh\037esp\036\004\
\275\037"    /* "$s may not have whitespace in it" */

#define syn_err_bitsize "b\263siz\004%ld \253\225\2151 \301\2317"    /* "bit size %ld illegal - 1 assumed" */
#define syn_err_zerobitsize "z\002\020w\364t\226n\236\021b\263\320\237\005\
\2151 \301\2317"    /* "zero width named bit field - 1 assumed" */
#define syn_err_arraysize "Arr\363siz\004%ld \253\225\2151 \301\2317"    /* "Array size %ld illegal - 1 assumed" */
#define syn_err_expected "\206\200\340$s -\140s\002t\021be\176\004$l"    /* "expected $s - inserted before $l" */
#define syn_err_expected1 "\206\200\340$s%s -\140s\002t\021be\176\004$l"    /* "expected $s%s - inserted before $l" */
#define syn_err_expected2 "\206\200\340$s \247$s -\140s\002t\021$s be\176\
\004$l"    /* "expected $s or $s - inserted $s before $l" */
#define syn_err_expecteda "\206\200\340$s"    /* "expected $s" */
#define syn_err_expected1a "\206\200\340$s%s"    /* "expected $s%s" */
#define syn_err_expected2a "\206\200\340$s \247$s"    /* "expected $s or $s" */
#define syn_err_mix_strings "ch\023 \034\005wi\305\050L\"\250.\"\336\317\
\371\017d\020\202\270c\014\016\014e"    /* "char and wide (L\"...\") strings do not concatenate" */
#define syn_err_expected_expr "\074\206p\266s\024\304\206\200\340\255\003\
\375\005$s"    /* "<expression> expected but found $s" */
#define syn_err_type_needed "\310n\236\004\206\200\1367"    /* "type name expected" */
#ifdef EXTENSION_VALOF
#define syn_err_valof_block \
        "\173 f\240\177w\026a\210\032\003wWl b\004t\013\334\254VALOF b\366\
k"    /* "{ following a cast will be treated as VALOF block" */
#endif
#define syn_err_typedef "\230\200\031\035n\236\004$r \251\021\275\206p\266\
s\174\270t\206t"    /* "typedef name $r used in expression context" */
#define syn_err_assertion "\137\137\137\301\002t\0500\337$e\051"    /* "___assert(0, $e)" */
#define syn_err_expected_id "Ex\200\340\074\364\300i\320\002\304\356$s \
\255\003\375\005$l"    /* "Expected <identifier> after $s but found $l" */
#define syn_err_hashif_eof "EOF \202newl\001\004\356\043i\035\250."    /* "EOF not newline after #if ..." */
#define syn_err_hashif_junk "J\027\211\356\043i\035\074\206p\266s\024\076"    /* "Junk after #if <expression>" */
#define syn_err_initialisers "t\377m\034y\140\370\002\017\275\173\175 \265\
agg\357\014e"    /* "too many initialisers in {} for aggregate" */
#define syn_err_initialisers1 "\173\175 m\251\003hav\0041 e\203m\016\003\
Q\001\370\004sc\015\023"    /* "{} must have 1 element to initialise scalar" */
#define syn_err_default "\'\244ault\204\202\275s\307c\226\215\333"    /* "'default' not in switch - ignored" */
#define syn_err_default1 "d\330\354\014\004\'\244ault\204c\032\004\333"    /* "duplicate 'default' case ignored" */
#define syn_err_case "\'c\032e\204\202\275s\307c\226\215\333"    /* "'case' not in switch - ignored" */
#define syn_err_case1 "d\330\354\334c\032\004\270\100\034t\011%ld"    /* "duplicated case constant: %ld" */
#define syn_err_expected_cmd "\074\262mm\034d\304\206\200\340\255\003\375\
\005\227$s"    /* "<command> expected but found a $s" */
#define syn_err_expected_while "\'whWe\204\206\200\340\356\'do\204\215\375\
\005$l"    /* "'while' expected after 'do' - found $l" */
#define syn_err_else "M\030pl\036\021\'\237\213\204\333"    /* "Misplaced 'else' ignored" */
#define syn_err_continue "\'\270t\001ue\204\202\275\177\234 \215\333"    /* "'continue' not in loop - ignored" */
#define syn_err_break "\'b\013ak\204\202\275\177\234 \247s\307c\226\215\
\333"    /* "'break' not in loop or switch - ignored" */
#define syn_err_no_label "\'goto\204\202f\240\177w\021b\205l\267\237 \215\
\333"    /* "'goto' not followed by label - ignored" */
#define syn_err_no_brace "\'\173\204\201\326bod\205\206\200\340\215\375\
\005$l"    /* "'{' of function body expected - found $l" */
#define syn_err_stgclass \
        "\100\007ag\004\271\032\017$s \202p\002m\037t\021\001\210\006t\206\
\003%s \215\333"    /* "storage class $s not permitted in context %s - ignored" */
#define syn_err_stgclass1 "\100\007ag\004\271\032\017$s\140\262\242\014\
ib\302\307\226$m \215\333"    /* "storage class $s incompatible with $m - ignored" */
#define syn_err_typeclash "\310$s\140\270s\030t\016\003\307\226$m"    /* "type $s inconsistent with $m" */
#define syn_err_tag_brace \
        "\'\173\204\247\074\364\300i\320\002\304\206\200\340\356$s\337\255\
\003\375\005$l"    /* "'{' or <identifier> expected after $s, but found $l" */
#define syn_err_expected3 "Ex\200\136\026\074\351\023\014\007\304\247\074\
\230\200\076\337\255\003\375\005$l"    /* "Expecting <declarator> or <type>, but found $l" */
#define syn_err_unneeded_id \
        "Id\300i\320\246\050%s\336\375\005\275\074\267\317\036\003\351\023\014\
\007\304\215\333"    /* "Identifier (%s) found in <abstract declarator> - ignored" */
#define syn_err_undef_struct(m,b,s) \
        "\027\260\021$c %s\011$r"    /* "undefined $c %s: $r" */, b, errs_membobj(m), s
#define syn_err_selfdef_struct(m,b,s) \
        "\014te\242\003Q\001\271u\305$c %s\011$r \307h\275\037\213lf"    /* "attempt to include $c %s: $r within itself" */, \
        b, errs_membobj(m), s
#define syn_err_void_object(m,s) "\253\225\'vo\364\204%s\011$r"    /* "illegal 'void' %s: $r" */, errs_membobj(m), s
#define syn_err_duplicate_type \
        "d\330\354\014\004\310s\200cif\233\372\201\274\225\355\236e\214\
$r"    /* "duplicate type specification of formal parameter $r" */
#define syn_err_not_a_formal "N\006-\274\225$r\261\355\236e\025-\230\200\
-s\200ci\320\002"    /* "Non-formal $r in parameter-type-specifier" */
#define syn_err_cant_init "$m v\023i\267\203\017m\363\202b\004\001\3707"    /* "$m variables may not be initialised" */
#define syn_err_enumdef \
        "\074\364\300i\320\002\304\206\200\340\255\003\375\005$l\261\'\016\
\231\204\260\037\024"    /* "<identifier> expected but found $l in 'enum' definition" */
#define syn_err_misplaced_brace "M\030pl\036\021\'\173\204\352t\234 \203\
v\237 \215\315\026b\366k"    /* "Misplaced '{' at top level - ignoring block" */

#define vargen_err_long_string "\317\026\001\370\246l\006g\246\264\361%\
s \133%ld\135"    /* "string initialiser longer than %s [%ld]" */
#define vargen_err_nonstatic_addr \
        "\345\100\014\233 \347\017$b\261\221\025\140\370\002"    /* "non-static address $b in pointer initialiser" */
#define vargen_err_bad_ptr "$s\011\253\225\251\004\275\221\025\140\370\002"    /* "$s: illegal use in pointer initialiser" */
#define vargen_err_init_void "obje\136\017\201\310\'vo\364\204c\361\202\
b\004\001\3707"    /* "objects of type 'void' can not be initialised" */
#define vargen_err_undefined_struct \
        "$c m\251\003b\004\260\021\265\050\100\014\233\336v\023i\267\302\
\351\023\014\024"    /* "$c must be defined for (static) variable declaration" */
#define vargen_err_open_array "Un\001\370\021\100\014\233 \133\135 \023\
ray\017\253\015"    /* "Uninitialised static [] arrays illegal" */
#define vargen_err_overlarge_reg "g\177b\225\357\030\214n\231b\246t\377\
l\360e"    /* "global register number too large" */
#define vargen_err_not_int "\001\325i\005\310\265g\177b\015\140\003\357\
\030\025"    /* "invalid type for global int register" */
#define vargen_err_not_float "\001\325i\005\310\265g\177b\225\256\352\357\
\030\025"    /* "invalid type for global float register" */
#ifdef TARGET_CALL_USES_DESCRIPTOR
#define vargen_err_badinit "\253\015\140\370\372Q$r%\053ld"    /* "illegal initialisation to $r%+ld" */
#endif
#ifdef TARGET_IS_HELIOS
#define vg_err_dynamicinit "In\370\021dyn\236\233 \023r\363\307\226-ZR \
\247-ZL"    /* "Initialised dynamic array with -ZR or -ZL" */
#endif
#define vargen_rerr_nonaligned \
        "N\006-\015\235\021ADCON \352d\014a\0530x%lx \050\325u\004$r\053\
0x%lx\336\314QNULL"    /* "Non-aligned ADCON at data+0x%lx (value $r+0x%lx) set to NULL" */
#define vargen_rerr_datadata_reloc \
       "RISC OS \050\247o\264\002\336\013\016\273\034\003modu\302h\254\100\014\
\233\140\037. Qd\014\227$r"    /* "RISC OS (or other) reentrant module has static init. to data $r" */

#define lex_rerr_8_or_9 "d\220\2638 \2479 \375\005\275o\136\225n\231b\002"    /* "digit 8 or 9 found in octal number" */
#define lex_rerr_pp_number "n\231b\246\253\015\376f\240\177w\021b\205\203\
t\025"    /* "number illegally followed by letter" */
#define lex_rerr_hex_exponent "h\206 n\231b\002\210\034\202hav\004\206p\
\006\300"    /* "hex number cannot have exponent" */
#define lex_rerr_esc16_truncated \
        "o\324l\360\004esca\272\'\\x%s%lx\204t\013\334\254\'\\x%lx\'"    /* "overlarge escape '\\x%s%lx' treated as '\\x%lx'" */
#define lex_rerr_esc8_truncated "o\324l\360\004esca\272\'\\%o\204t\013\334\
\254\'\\%o\'"    /* "overlarge escape '\\%o' treated as '\\%o'" */
#define lex_rerr_illegal_esc "\253\225\317\026esca\272\'\\%c\204\215t\013\
\334\254%c"    /* "illegal string escape '\\%c' - treated as %c" */
#define lex_rerr_not1wchar "L\'\250.\204ne7\017\206\036t\3761 wi\305ch\023\
\036\025"    /* "L'...' needs exactly 1 wide character" */
#define lex_rerr_empty_char "n\020ch\023\017\001\210h\023\036\025\210\006\
\100\034\003\'\'"    /* "no chars in character constant ''" */
#define lex_rerr_overlong_char "m\007\004\264\3614\210h\023\017\001\210\
h\023\036\025\210\006\100\034t"    /* "more than 4 chars in character constant" */

#define syn_rerr_array_0 "\023r\363\1330\135 \375d"    /* "array [0] found" */
#ifdef EXTENSION_VALOF
#define syn_rerr_void_valof "voi\005\325\201b\366k\017\023\004\202p\002\
m\037t7"    /* "void valof blocks are not permitted" */
#endif
#define syn_rerr_undeclared "\027\351a\013\005n\236e,\140v\300\026\'\206\
\025n\140\003%s\'"    /* "undeclared name, inventing 'extern int %s'" */
#define syn_rerr_insert_parens \
        "pa\013n\264e\213\017\050\250\051\140s\002t\021\023\323\005\206\
p\266s\174f\240\177w\026$s"    /* "parentheses (..) inserted around expression following $s" */
#define syn_rerr_return "\013t\365\322\074\206pr\304\253\225\265voi\005\
\257\024"    /* "return <expr> illegal for void function" */
#define syn_rerr_qualified_typedef(b,m) \
        "$m \230\200\031\035$b h\254$m \013-s\200ci\3207"    /* "$m typedef $b has $m re-specified" */, m, b, m
#define syn_rerr_missing_type "m\373\310s\200cif\233\372\215\'\277\204\301\
\2317"    /* "missing type specification - 'int' assumed" */
#define syn_rerr_long_float "ANSI C doe\017\202s\330p\007\003\'l\006\022\
\344\'"    /* "ANSI C does not support 'long float'" */
#define syn_rerr_missing_type1 \
        "om\037t\021\074\230\200\304be\176\004\274\225\351\023\014\247\215\'\
\277\204\301\2317"    /* "omitted <type> before formal declarator - 'int' assumed" */
#define syn_rerr_missing_type2 \
        "\326p\343to\310\274\225$r ne7\017\310\007\210l\032\017\215\'\277\
\204\301\2317"    /* "function prototype formal $r needs type or class - 'int' assumed" */
#define syn_rerr_ellipsis_first "\237lips\276\050\250.\051\210\034\202b\
\004\006\376\355\236e\025"    /* "ellipsis (...) cannot be only parameter" */
#define syn_rerr_mixed_formals "p\343to\310\034\005\240d-\100y\302\355\236\
e\025\017mix7"    /* "prototype and old-style parameters mixed" */
#define syn_rerr_open_member "\253\225\133\135 me\313\243$r"    /* "illegal [] member: $r" */
#define syn_rerr_fn_returntype "\326\013t\365n\026$t \253\225-\215\301\231\
\327\025"    /* "function returning $t illegal -- assuming pointer" */
#define syn_rerr_array_elttype "\023r\363\201$t \253\225-\215\301\231\327\
\025"    /* "array of $t illegal -- assuming pointer" */
#define syn_rerr_fn_ptr(m,s) \
   "%s $r m\363\202b\004\326-\215\301\231\327\025"    /* "%s $r may not be function -- assuming pointer" */, errs_membobj(m), s
#define syn_rerr_fn_ptr1 \
        "\326$r m\363\202b\004\001\370\021\215\301\231\026\326\221\025"    /* "function $r may not be initialised - assuming function pointer" */
#define syn_rerr_archaic_init "Anci\016\003\274 \201\001\370\014\024\337\
\251\004\'\075\'"    /* "Ancient form of initialisation, use '='" */
#define syn_rerr_bitfield "\253\225b\263\320\237\005\310$t \215\'\277\204\
\301\2317"    /* "illegal bit field type $t - 'int' assumed" */
#define syn_rerr_missing_formal "\274\225n\236\004m\373\275\326\260\037\
\024"    /* "formal name missing in function definition" */
#define syn_rerr_ineffective "\351\023\372\307\226n\020e\312e\136"    /* "declaration with no effect" */
#define syn_rerr_duplicate_member(sv,b) "d\330\354\014\004me\313\246$r \
\201$c"    /* "duplicate member $r of $c" */, sv, b
#define syn_rerr_missing_type3 \
        "\310\007\210l\032\017ne7\021\050\206c\311\003\275\326\260\037\024\
\336\215\'\277\204\301\2317"    /* "type or class needed (except in function definition) - 'int' assumed" */
#define syn_rerr_semicolon_in_arglist \
        "\',\204\050\202\'\073\'\336\213\355\014e\017\274\225\355\236e\025\
s"    /* "',' (not ';') separates formal parameters" */
#define syn_rerr_no_members "$c h\254n\020me\313\002s"    /* "$c has no members" */

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
#define misc_disaster_banner   "\n\367\367\367\367\232\052\n\052 Th\004\
\262\242W\246h\254\031te\340\034\140\025n\015\140\270s\030t\016cy.\374T\
h\276c\361occ\365\374\052\n\052 beca\251\004\263h\254r\027 \216\003\201\
\227v\037\225\266\216rc\004suc\226\254mem\007\205\247d\030\211\374\374\052\n\
\052 sp\036\004\247beca\251\004\264\002\004\276\227faul\003\275\037.\374\
I\035y\216\210\034\202e\032W\205\015\214 \052\n\052 y\216r p\343gr\236 \
Qavoi\005ca\251\026\264\276r\023\004faWu\013\337p\203\032\004\270t\036\003\
y\216r\374\052\n\052 \031\015\002.\374Th\004\031\015\246m\363b\004\267\302\
Qh\237p y\216 imm7i\014\237\205\034\005wWl b\004\374\052\n\052 \267\302\
Q\013p\007\003\227s\251\200\340\262\242W\246faul\003Q\264\004s\330p\007\
\003c\300\013.\374\374\374\052\n\367\367\367\367\232\052\n\n"    /* "\n\
*************************************************************************\n\
* The compiler has detected an internal inconsistency.  This can occur  *\n\
* because it has run out of a vital resource such as memory or disk     *\n\
* space or because there is a fault in it.  If you cannot easily alter  *\n\
* your program to avoid causing this rare failure, please contact your  *\n\
* dealer.  The dealer may be able to help you immediately and will be   *\n\
* able to report a suspected compiler fault to the support centre.      *\n\
*************************************************************************\n\
\n" */


#define syserr_local_address 		"\366\015\137\347\017%lx"    /* "local_address %lx" */
#define syserr_decode_branch_address	"\031\262\031\137br\034ch\137\347\017\
%.8lx"    /* "decode_branch_address %.8lx" */
#define syserr_unknown_diadic_op	"\335\322di\245\233 \234\210o\305%02lx"    /* "unknown diadic op code %02lx" */
#define syserr_unknown_triadic_op	"\335\322\273i\245\233 \234\210o\305%\
02lx"    /* "unknown triadic op code %02lx" */
#define syserr_non_float_dest		"\031\100\001\372\201\344\327\003\234 \276\
\202\361\206t\207\021p\013c\030\174\357\030\025\041 \050%s\051"    /* "destination of floating point op is not an extended precision register! (%s)" */
#define syserr_unknown_indirect_mode	"\335n\140di\013c\003\347\306mo\305\
%lx"    /* "unknown indirect addressing mode %lx" */
#define syserr_unknown_triadic_address	"\335\322\274 \201\273i\245\233 \
\347s\371,\140\317u\136\174\075 %lx"    /* "unknown form of triadic addressing, instruction = %lx" */
#define syserr_unknown_parallel_op	"\335\322\355\015\203l \234\210o\305\
%lx"    /* "unknown parallel op code %lx" */
#define syserr_bad_parallel_addressing	"m\015\274\021\355\015\203l \347\
\306d\030\262\3247,\140\317u\136\174\075 %lx"    /* "malformed parallel addressing discovered, instruction = %lx" */
#define syserr_unknown_op_code		"\335\322\234\210o\305%lx"    /* "unknown op code %lx" */
#define syserr_asmlab 			"od\005\032ml\267\050%lx\051"    /* "odd asmlab(%lx)" */
#define syserr_display_asm 		"d\030play\137\032m\050%lx\051"    /* "display_asm(%lx)" */
#define syserr_asm_trailer 		"\032m\137\273aW\002\050%ld\051"    /* "asm_trailer(%ld)" */
#define syserr_datalen 			"\032m\137d\014\227l\016\075%ld"    /* "asm_data len=%ld" */
#define syserr_asm_trailer1 		"\032m\137\273aW\002\050%ldF%ld\051"    /* "asm_trailer(%ldF%ld)" */
#define syserr_asm_trailer2 		"\032m\137\273aW\002\050LIT\137ADCON rpt\051"    /* "asm_trailer(LIT_ADCON rpt)" */
#define syserr_asm_confused 		"As\213\313l\246\216tpu\003\270f\251\021\215\
f\001\005\'\077\'"    /* "Assembler output confused - find '?'" */
#define syserr_unknown_addressing_mode	"Un\321\322\347\306mo\305\215%lx"    /* "Unknown addressing mode - %lx" */
#define syserr_not_address_register	"\013l\014iv\004\013f\002\016c\004\264\
r\216g\226\345\347\017\357\030\214%ld"    /* "relative reference through non-address register %ld" */
#define syserr_unsupported_branch_type	"\027s\330p\007t\021br\034c\226\310\
%ld"    /* "unsupported branch type %ld" */
#define syserr_bad_block_length		"neg\014iv\004b\366\211l\016gt\226%ld"    /* "negative block length %ld" */
#define syserr_large_shift		"\014te\242t\350shif\003b\205\027\013\032\006\
\267\376l\360\004\236\323\003%ld"    /* "attempting to shift by unreasonably large amount %ld" */
#define syserr_bad_addressing_mode	"\032k\021Q\255W\005\034\140\325i\005\
\347\306mo\031"    /* "asked to build an invalid addressing mode" */
#define syserr_bad_arg 			"Ba\005\023\022%lx"    /* "Bad arg %lx" */
#define syserr_local_addr 		"\366\015\137\332r"    /* "local_addr" */
#define syserr_firstbit			"w\023n\371\011\014te\242t\350f\001\005\320rs\
\003b\263\314\2010 \041"    /* "warning: attempting to find first bit set of 0 !" */
#define syserr_displacement 		"d\030pl\036em\016\003\216\003\201r\034g\004\
%ld"    /* "displacement out of range %ld" */
#define syserr_labref 			"\335\322l\267\237 \013f\002\016c\004\310%.8lx"    /* "unknown label reference type %.8lx" */
#define syserr_enter 			"em\037\050J\137ENTER %ld\051"    /* "emit(J_ENTER %ld)" */
#define syserr_data_sym			"Un\267\302Qf\001\005a\033\264\246d\014\227sy\
\313\240 \352%ld"    /* "Unable to find another data symbol at %ld" */
#define syserr_show_inst 		"show\137\001\317u\136\174\050%\043lx\051"    /* "show_instruction (%#lx)" */
#define syserr_movr 			"movr r,r"    /* "movr r,r" */
#define syserr_offset_out_of_range	"o\312\314\050%d\336\276t\377b\220"    /* "offset (%d) is too big" */
#define syserr_cannot_do_block_move	"c\034\202d\020b\366\211move"    /* "cannot do block move" */
#define syserr_bad_length		"ba\005l\016gt\226\265b\366\211\234\002\372\075\
\304%ld"    /* "bad length for block operation => %ld" */
#define syserr_local_base 		"\366\015\137b\032\004%lx"    /* "local_base %lx" */
#define syserr_setsp_confused 		"SETSP\210\006f\251\021%ld\041\075%ld %\
ld"    /* "SETSP confused %ld!=%ld %ld" */

#ifdef DEBUG
#define peep_null_format		"NULL \274\352\317\371"    /* "NULL format string" */
#define peep_null_list			"p\301\021NULL l\030t"    /* "passed NULL list" */
#define peep_null_parameter		"p\301\021NULL \355\236e\025"    /* "passed NULL parameter" */
#define peep_urg			"\346\243\365\022\365g"    /* "peepholer: urg urg" */
#define peep_peepholer_empty		"\014te\242t\350em\263f\343m EMPTY \346\002\
\041"    /* "attempting to emit from EMPTY peepholer!" */
#define gen_bad_float_push		"\224\273y\350\256\352p\251\226\227\345\344\
\327\003\357\030\025"    /* "back end: trying to float push a non-floating point register" */
#define gen_bad_float_pop		"\224\273y\350\256\352\212p \227\345\344\327\
\003\357\030\025"    /* "back end: trying to float pop a non-floating point register" */
#define gen_non_FP_source		"\224\273y\350p\002\274 \227\344\327\003\234\
\002\372\307\226\227\303 \344\327\003s\216rce"    /* "back end: trying to perform a floating point operation with a non floating point source" */
#define gen_non_FP_dest			"\224\273y\350p\002\274 \227\344\327\003\234\002\
\372\307\226\227\303 \344\327\003\031\100\001\014\024"    /* "back end: trying to perform a floating point operation with a non floating point destination" */
#define gen_NULL_param			"g\016.c\011p\301\021\227NULL \355\236e\025"    /* "gen.c: passed a NULL parameter" */
#endif /* DEBUG */
  
#define peep_unknown_push		"\346\243\335\322p\251\226\230\200"    /* "peepholer: unknown push type" */
#define peep_out_of_memory		"\346\243\216\003\201mem\007y"    /* "peepholer: out of memory" */
#define peep_no_offset			"\346\243n\020o\312\314\275br\034ch\041"    /* "peepholer: no offset in branch!" */
#define peep_unknown_delay_type		"\346\243\335\322\031l\363\230\200"    /* "peepholer: unknown delay type" */
#define peep_unknown_type		"\346\243\032k\021Qem\263\361\335\322\310%d"    /* "peepholer: asked to emit an unknown type %d" */
#define peep_unexpected_back_ref	"\346\243\027\206\200\340b\036kw\023d\017\013\
f\002\016ce"    /* "peepholer: unexpected backwards reference" */
#define peep_fwd_and_cross_ref		"\346\243p\207\026\176w\023\005\034\005\
c\343s\017\013f\002\016ce\017\265s\236\004\001\317u\136\024\041"    /* "peepholer: pending forward and cross references for same instruction!" */
#define peep_special_pending_cross	"\346\243s\200c\342\140\317u\136\174\
\310ha\005\227p\207\026c\343s\017\013f\002\016ce\041"    /* "peepholer: special instruction type had a pending cross reference!" */
#define peep_special_pending_fwd	"\346\243s\200c\342\140\317u\136\174\310\
ha\005\227p\207\026\176w\023\005\013f\002\016ce\041"    /* "peepholer: special instruction type had a pending forward reference!" */
#define peep_elim_clash			"\346\243\014te\242t\350\237im\001\014\004\001\
\317u\136\174whWs\003\227\013f\002\016c\004\276\100Wl p\207\371"    /* "peepholer: attempting to eliminate instruction whilst a reference is still pending" */
#define peep_non_existant_xfer		"\346\243\273y\350\237im\001\014\004\345\
\206\030t\034\003\357\030\214\273\034sf\002"    /* "peepholer: trying to eliminate non-existant register transfer" */
#define peep_cross_ref_pending		"\346\243\014te\242t\350fl\251\226\346\246\
whWs\003a\210\343s\017\013\035\276\100Wl p\207\371"    /* "peepholer: attempting to flush peepholer whilst a cross ref is still pending" */
#define peep_fwd_ref_pending		"\346\243\014te\242t\350fl\251\226\346\246\
whWs\003\227\176w\023\005\013\035\276\100Wl p\207\371"    /* "peepholer: attempting to flush peepholer whilst a forward ref is still pending" */
#define peep_failed_reset_count		"\346\243fl\251\226h\254faW\021Q\013\314\
\331\210\323t"    /* "peepholer: flush has failed to reset peep count" */

#define asm_unknown_indirect_addr	"d\030\032\213\313l\243\335n\140di\013\
c\003\347\306mo\305%lx"    /* "disassembler: unknown indirect addressing mode %lx" */
#define asm_dest_not_FP_reg		"d\030\032\213\313l\243\031\100\001\372\201\
FP \234 \050%s\336\276\202\361FP \357\030\214\050%s\051"    /* "disassembler: destination of FP op (%s) is not an FP register (%s)" */
#define asm_bad_source_field		"d\030\032\213\313l\243ba\005\325u\004\275\
s\216rc\004\320\237\005\201\357\030\214\347s\021\234 %s \050%lx\051"    /* "disassembler: bad value in source field of register addressed op %s (%lx)" */
#define asm_source_not_FP_reg		"d\030\032\213\313l\243\303 FP s\216rc\004\
\357\030\214%s \265\234 %s"    /* "disassembler: non FP source register %s for op %s" */
#define asm_no_triadic_FP_immediates	"d\030\032\213\313l\243\273i\245\233 \
\344\327\003imm7i\014\004\234\002\014\024\017\023\004im\212ssib\203"    /* "disassembler: triadic floating point immediate operations are impossible" */
#define asm_lost_symbol			"d\030\032\213\313l\243m\030lai\005\227sy\313\
\240\041"    /* "disassembler: mislaid a symbol!" */

#define heliobj_conflicting_offsets	"h\237iobj\011sy\313\240 %s h\254o\312\
\314%ld \034\005f\240\177w\017sy\313\240 %s \307\226o\312\314%ld"    /* "heliobj: symbol %s has offset %ld and follows symbol %s with offset %ld" */
#define heliobj_repeated_export		"h\237iobj\011c\034\202\050yet\336\013\
\200\0147\376\206p\007\003\264\004s\236\004sy\313\240"    /* "heliobj: cannot (yet) repeatedly export the same symbol" */
#define heliobj_bad_sized_FP		"h\237iobj\011ba\005siz\004\201\344\327\003\
n\231b\246%ld"    /* "heliobj: bad size of floating point number %ld" */
#define heliobj_unknown_data_type	"h\237iobj\011\335\322d\014\227\310%l\
x"    /* "heliobj: unknown data type %lx" */
#define heliobj_bad_length		"h\237iobj\011ba\005l\016gt\226\265p\036k\021\
d\014\227\050%ld\051"    /* "heliobj: bad length for packed data (%ld)" */

#define gen_illegal_register		"\224\273y\350e\325u\014\004\253\225\357\030\
\2140x%lx"    /* "back end: trying to evaluate illegal register 0x%lx" */
#define gen_bad_addr_mode		"\224ba\005\347\306mo\305%ld"    /* "back end: bad addressing mode %ld" */
#define gen_bad_displacement		"\224\253\225d\030pl\036em\300"    /* "back end: illegal displacement" */
#define gen_value_too_big		"\224imm7i\014\004\325u\004t\377l\360e"    /* "back end: immediate value too large" */
#define gen_unknown_addressing		"\224\335\322k\001\005\201\273i\245\233 \
\347\306%ld"    /* "back end: unknown kind of triadic addressing %ld" */
#define gen_non_word_offset		"\224\273y\350\245\005\227\345w\007\005o\312\
\314Q\227w\007\005\221\025"    /* "back end: trying to add a non-word offset to a word pointer" */
#define gen_FP_value_not_fit		"\224\344\137imm7i\014e\137\234\011\032k\021\
Qp\002\274 \361\234 \307\226\361FP \325u\004\264\352doe\017\202f\037"    /* "back end: float_immediate_op: asked to perform an op with an FP value that does not fit" */
#define gen_void_compare		"\224\032k\021Qp\002\274 voi\005\262\242\023\030\
\024"    /* "back end: asked to perform void comparision" */
#define gen_offset_reg_conflict		"\224o\312\314\357\030\025\210\006fli\136"    /* "back end: offset register conflict" */
#define gen_too_many_to_skip		"\224\270d\037\024\015\137skip\137\001\317\
u\136\024s\011t\377m\034y\140\317u\136\024\017Qskip\041 \050%ld\051"    /* "back end: conditional_skip_instructions: too many instructions to skip! (%ld)" */
#define gen_cannot_branch		"\224c\034\202\270d\037\024\015\376br\034c\226\
Qo\312\314%lx"    /* "back end: cannot conditionally branch to offset %lx" */
#define gen_not_identifier		"\224\030\137\257\024\011\202p\301\021\361\364\
\300i\320\002"    /* "back end: is_function: not passed an identifier" */
#define gen_not_supported		"\224\177\245\026\100\217\221\214\307\226a\210\
\006\100\034\003\347\017\276\202ye\003s\330p\007t7"    /* "back end: loading stack pointer with a constant address is not yet supported" */
#define gen_address_of_non_function	"\224\273y\350tak\004\347\017\201\345\
\257\024\041"    /* "back end: trying to take address of non-function!" */
#define gen_ptr_not_aligned		"\224\326\221\214\202w\007\005\015\2357\041"    /* "back end: function pointer not word aligned!" */
#define gen_offset_from_ptr		"\224\263\276\253\225Qtak\004\361o\312\314\
f\343m \227\326\221\025"    /* "back end: it is illegal to take an offset from a function pointer" */
#define gen_offset_too_big		"\224o\312\314t\377bi\022\050o\312\314\075 \
%ld\051"    /* "back end: offset too big (offset = %ld)" */
#define gen_need_patch			"\224\032\213\313\376\262\305\216tpu\003\276m\373\
\227p\014c\226di\013\136ive"    /* "back end: assembly code output is missing a patch directive" */
#define gen_no_data_to_init		"\224b\366k\137d\014a\137\001\037\011n\020\
d\014\227Q\001\263\050%ld\051"    /* "back end: block_data_init: no data to init (%ld)" */
#define gen_already_zero		"\224\032k\021Q\100\007\0040\140Q\015\013\245\
\205z\002o\'\021mem\007y"    /* "back end: asked to store 0 into already zero'ed memory" */
#define gen_not_data			"\224\206p\007t\137d\014a\137sy\313\240\011\032k\
\021Q\206p\007\003\345d\014a\011%s"    /* "back end: export_data_symbol: asked to export non-data: %s" */
#define gen_fn_call_in_stub		"\224c\015l\011\326c\015l Q%s whWs\003g\016\
\002\014\026\227\100ub"    /* "back end: call: function call to %s whilst generating a stub" */
#define gen_cannot_call_offset		"\224c\034\202c\015l Qo\312\314%lx"    /* "back end: cannot call to offset %lx" */
#define gen_already_set_label		"\224\273y\350\314\361\015\013\245\205\314\
l\267\237\041"    /* "back end: trying to set an already set label!" */
#define gen_copy_too_small		"\224c\234y\026\203s\017\264\3614 byte\017\201\
mem\007\205\050%ld\051"    /* "back end: copying less than 4 bytes of memory (%ld)" */
#define gen_copy_non_word_multiple	"\224c\234\205mem\007\205\032k\021Qc\
\234\205\227\303 w\007\005multip\302%ld"    /* "back end: copy memory asked to copy a non word multiple %ld" */
#define gen_URG				"\224\365\022\215\264\276sh\216l\005\202happ\016"    /* "back end: urg - this should not happen" */
#ifdef TARGET_LACKS_UNSIGNED_FIX
#define gen_no_unsigned_fix		"\224\027s\235\021FIX \202s\330p\007t7"    /* "back end: unsigned FIX not supported" */
#endif
#define gen_failed_to_init		"\224faW\021Q\001\370\004sav7\137\357s"    /* "back end: failed to initialise saved_regs" */
#define gen_CALL_misaligned		"\224\173TAIL\175CALL\133KR\135 \100\217m\030\
-\015\2357"    /* "back end: {TAIL}CALL[KR] stack mis-aligned" */
#ifdef TARGET_HAS_COND_EXEC
#define gen_mismatched_pending		"\224br\034c\226%lx doe\017\202m\014c\226\
p\207\026\270d\037\174%lx"    /* "back end: branch %lx does not match pending condition %lx" */
#define gen_pending_cond_exec		"\224\014te\242t\021Q\270d\037\024\015\376\
\206ecut\004J\137\234\262\305%ld"    /* "back end: attempted to conditionally execute J_opcode %ld" */
#endif
#define gen_no_byte_to_short		"\224s\235 \206t\016\005byt\004Qsh\007\003\
\202ye\003s\330p\007t7"    /* "back end: sign extend byte to short not yet supported" */
#define gen_unknown_extend		"\224\335\322s\235 \206t\016\005mo\305%ld"    /* "back end: unknown sign extend mode %ld" */

#define debugger_cannot_open_output	"\031\255gg\243faW\021Q\234\016 \031\
\255\022\216tpu\003fW\004%s"    /* "debugger: failed to open debug output file %s" */
#define debugger_NULL_pointer		"\031\255gg\243p\301\021\227NULL \221\025"    /* "debugger: passed a NULL pointer" */
#define debugger_type			"\031\255gg\243db\137\230\200\050 %ld,0x%lx \051"    /* "debugger: db_type( %ld,0x%lx )" */
#define debugger_no_block		"\031\255gg\243n\020c\365\013n\003b\366\211\247\
v\023i\267\203"    /* "debugger: no current block or variable" */
#define debugger_already_associated	"\031\255gg\243\273y\350\245\005\361\015\013\
\245\205\301oci\334\366\225v\023i\267\203"    /* "debugger: trying to add an already associated local variable" */
#define debugger_unknown_storage	"\031\255gg\243\335\322\100\007ag\004\230\
\200"    /* "debugger: unknown storage type" */
#define debugger_bad_scope		"\031\255gg\243ba\005s\262\200"    /* "debugger: bad scope" */
#define debugger_unresolved_variable	"\031\255gg\243\027\266\240v\021v\023\
i\267\302\271\032\017\265%s"    /* "debugger: unresolved variable class for %s" */
#define debugger_fn_with_no_blocks	"\031\255gg\243\326\307\226n\020b\366\
ks"    /* "debugger: function with no blocks" */
#define heliobj_unknown_ref		"h\237iobj\011\335n\210\343s\017\013f\002n\
c\004\310%lx"    /* "heliobj: unknown cross refernce type %lx" */
#define heliobj_no_output_stream	"h\237iobj\011n\020\216tpu\003\100\013\
\236"    /* "heliobj: no output stream" */
  
/* end of mbe/mcerrs.h */
#ifndef NO_INSTORE_FILES
#  define NO_INSTORE_FILES 1
#endif

#define COMPRESSED_ERROR_MESSAGES 1

#ifdef DEFINE_ERROR_COMPRESSION_TABLE

static unsigned short int ecompression_info[256] = {
    0x0000, 0x696e, 0x6572, 0x7420, 0x6520, 0x6420, 0x6f6e, 0x6f72, 
    0x2a2a, 0x3a20, 0x000a, 0x7265, 0x6174, 0x616c, 0x656e, 0x7320, 
    0x6f20, 0x6505, 0x6720, 0x6172, 0x6906, 0x7402, 0x0112, 0x756e, 
    0x6973, 0x6465, 0x6173, 0x6e6f, 0x616e, 0x6620, 0x6163, 0x6974, 
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027, 
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f, 
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0808, 0x0036, 0x6564, 
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f, 
    0x7374, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f, 
    0x0050, 0x7410, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x696c, 
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x6374, 0x005f, 
    0x2001, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f, 
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 
    0x0078, 0x0079, 0x007a, 0x007b, 0x1420, 0x007d, 0x6607, 0x6c6f, 
    0x7065, 0x6f1d, 0x1b03, 0x6c65, 0x2720, 0x7920, 0x6578, 0x0e64, 
    0x2063, 0x6b20, 0x706f, 0x7365, 0x1520, 0x2d20, 0x6f75, 0x1e89, 
    0x6967, 0x8a01, 0x628f, 0x8709, 0x9293, 0x0d20, 0x6820, 0x6120, 
    0x7479, 0x756d, 0x3535, 0x6963, 0x6f70, 0x906e, 0x616d, 0x656c, 
    0x6f6c, 0x8367, 0x6d70, 0x0209, 0x1966, 0x6164, 0x0220, 0x0720, 
    0x2e2e, 0x7573, 0x175e, 0x57a1, 0x610f, 0x6275, 0x667f, 0x66aa, 
    0xa401, 0x6020, 0x636f, 0x6903, 0x7468, 0x7e20, 0x0b73, 0x6162, 
    0x6306, 0x636c, 0x7004, 0x7472, 0x7e6d, 0x0120, 0x690f, 0x0174, 
    0x0e74, 0x1a73, 0x6c04, 0x6e06, 0x3e20, 0x6404, 0x7316, 0x771f, 
    0x98ba, 0x6570, 0x6666, 0x6d62, 0x8b03, 0x9d07, 0x1b77, 0x4072, 
    0x6669, 0x6bce, 0x6e20, 0x6f17, 0x7602, 0x760d, 0xaf7c, 0x1691, 
    0x7570, 0x80c9, 0xa564, 0xcd37, 0x0c11, 0x17d1, 0x2920, 0x2c20, 
    0x5e11, 0x68a0, 0x690d, 0x726f, 0xae0c, 0xc32d, 0xd9e1, 0xdab6, 
    0x1651, 0x19b9, 0x6103, 0x6166, 0x6c9b, 0x7013, 0xeb8c, 0x0b67, 
    0x1367, 0x1c20, 0x1fe2, 0x6185, 0x6964, 0x7572, 0x7f63, 0x9a9a, 
    0xf218, 0x0167, 0x0c7c, 0x18c6, 0x2020, 0x66d3, 0x6c85, 0x6f10};


#endif

#define MAXSTACKDEPTH 4

#endif /* already loaded */

/* end of errors.h */
