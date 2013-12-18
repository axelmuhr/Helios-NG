/*
 * cfe/feerrs.h - prototype for front-end error messages file
 * version 2.
 */

%O  /* Ordinary error messages - mapped onto numeric codes */

#define lex_warn_force_unsigned "%s treated as %sul in 32-bit implementation"
#define lex_warn_multi_char "non-portable - not 1 char in '...'"

#define syn_warn_hashif_undef "Undefined macro '%s' in #if - treated as 0"
#define syn_warn_invent_extern "inventing 'extern int %s();'"
#define syn_warn_unary_plus "Unary '+' is a feature of ANSI C"
#define syn_warn_spurious_braces "spurious {} around scalar initialiser"
#define syn_warn_dangling_else "Dangling 'else' indicates possible error"
#define syn_warn_void_return "non-value return in non-void function"
#define syn_warn_use_of_short \
        "'short' slower than 'int' on this machine (see manual)"
#define syn_warn_enum_unchecked "No type checking of 'enum' in this compiler"
#define syn_warn_undeclared_parm \
        "formal parameter $r not declared - 'int' assumed"
#define syn_warn_old_style "Old-style function $r"
#define syn_warn_give_args "Deprecated declaration %s() - give arg types"
#define syn_warn_ANSI_decl "ANSI style function declaration used, '%s(...)'"
#define syn_warn_archaic_init "Ancient form of initialisation, use '='"
#define syn_warn_untyped_fn "'int %s()' assumed - 'void' intended?"
#define syn_warn_no_named_member "$c has no named member"
#define syn_warn_extra_comma "Superfluous ',' in 'enum' declaration"

#define vargen_warn_nonull "omitting trailing '\\0' for %s [%ld]"
#define vargen_warn_unnamed_bitfield \
        "Unnamed bit field initialised to 0"

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
#define lex_err_bad_noprint_char "illegal character (hex code 0x%x) in source"
#define lex_err_ellipsis "(...) must have exactly 3 dots"
#define lex_err_illegal_whitespace "$s may not have whitespace in it"

#define syn_err_bitsize "bit size %ld illegal - 1 assumed"
#define syn_err_zerobitsize "zero width named bit field - 1 assumed"
#define syn_err_arraysize "Array size %ld illegal - 1 assumed"
#define syn_err_expected "expected $s - inserted before $l"
#define syn_err_expected1 "expected $s%s - inserted before $l"
#define syn_err_expected2 "expected $s or $s - inserted $s before $l"
#define syn_err_expecteda "expected $s"
#define syn_err_expected1a "expected $s%s"
#define syn_err_expected2a "expected $s or $s"
#define syn_err_mix_strings "char and wide (L\"...\") strings do not concatenate"
#define syn_err_expected_expr "<expression> expected but found $s"
#define syn_err_type_needed "type name expected"
#ifdef EXTENSION_VALOF
#define syn_err_valof_block \
        "{ following a cast will be treated as VALOF block"
#endif
#define syn_err_typedef "typedef name $r used in expression context"
#define syn_err_assertion "___assert(0, $e)"
#define syn_err_expected_id "Expected <identifier> after $s but found $l"
#define syn_err_hashif_eof "EOF not newline after #if ..."
#define syn_err_hashif_junk "Junk after #if <expression>"
#define syn_err_initialisers "too many initialisers in {} for aggregate"
#define syn_err_initialisers1 "{} must have 1 element to initialise scalar"
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
#define syn_err_undef_struct(m,b,s) \
        "undefined $c %s: $r", b, errs_membobj(m), s
#define syn_err_selfdef_struct(m,b,s) \
        "attempt to include $c %s: $r within itself", \
        b, errs_membobj(m), s
#define syn_err_void_object(m,s) "illegal 'void' %s: $r", errs_membobj(m), s
#define syn_err_duplicate_type \
        "duplicate type specification of formal parameter $r"
#define syn_err_not_a_formal "Non-formal $r in parameter-type-specifier"
#define syn_err_cant_init "$m variables may not be initialised"
#define syn_err_enumdef \
        "<identifier> expected but found $l in 'enum' definition"
#define syn_err_misplaced_brace "Misplaced '{' at top level - ignoring block"

#define vargen_err_long_string "string initialiser longer than %s [%ld]"
#define vargen_err_nonstatic_addr \
        "non-static address $b in pointer initialiser"
#define vargen_err_bad_ptr "$s: illegal use in pointer initialiser"
#define vargen_err_init_void "objects of type 'void' can not be initialised"
#define vargen_err_undefined_struct \
        "$c must be defined for (static) variable declaration"
#define vargen_err_open_array "Uninitialised static [] arrays illegal"
#define vargen_err_overlarge_reg "global register number too large"
#define vargen_err_not_int "invalid type for global int register"
#define vargen_err_not_float "invalid type for global float register"
#ifdef TARGET_CALL_USES_DESCRIPTOR
#define vargen_err_badinit "illegal initialisation to $r%+ld"
#endif
#ifdef TARGET_IS_HELIOS
#define vg_err_dynamicinit "Initialised dynamic array with -ZR or -ZL"
#endif
#define vargen_rerr_nonaligned \
        "Non-aligned ADCON at data+0x%lx (value $r+0x%lx) set to NULL"
#define vargen_rerr_datadata_reloc \
       "RISC OS (or other) reentrant module has static init. to data $r"

#define lex_rerr_8_or_9 "digit 8 or 9 found in octal number"
#define lex_rerr_pp_number "number illegally followed by letter"
#define lex_rerr_hex_exponent "hex number cannot have exponent"
#define lex_rerr_esc16_truncated \
        "overlarge escape '\\x%s%lx' treated as '\\x%lx'"
#define lex_rerr_esc8_truncated "overlarge escape '\\%o' treated as '\\%o'"
#define lex_rerr_illegal_esc "illegal string escape '\\%c' - treated as %c"
#define lex_rerr_not1wchar "L'...' needs exactly 1 wide character"
#define lex_rerr_empty_char "no chars in character constant ''"
#define lex_rerr_overlong_char "more than 4 chars in character constant"

#define syn_rerr_array_0 "array [0] found"
#ifdef EXTENSION_VALOF
#define syn_rerr_void_valof "void valof blocks are not permitted"
#endif
#define syn_rerr_undeclared "undeclared name, inventing 'extern int %s'"
#define syn_rerr_insert_parens \
        "parentheses (..) inserted around expression following $s"
#define syn_rerr_return "return <expr> illegal for void function"
#define syn_rerr_qualified_typedef(b,m) \
        "$m typedef $b has $m re-specified", m, b, m
#define syn_rerr_missing_type "missing type specification - 'int' assumed"
#define syn_rerr_long_float "ANSI C does not support 'long float'"
#define syn_rerr_missing_type1 \
        "omitted <type> before formal declarator - 'int' assumed"
#define syn_rerr_missing_type2 \
        "function prototype formal $r needs type or class - 'int' assumed"
#define syn_rerr_ellipsis_first "ellipsis (...) cannot be only parameter"
#define syn_rerr_mixed_formals "prototype and old-style parameters mixed"
#define syn_rerr_open_member "illegal [] member: $r"
#define syn_rerr_fn_returntype "function returning $t illegal -- assuming pointer"
#define syn_rerr_array_elttype "array of $t illegal -- assuming pointer"
#define syn_rerr_fn_ptr(m,s) \
   "%s $r may not be function -- assuming pointer", errs_membobj(m), s
#define syn_rerr_fn_ptr1 \
        "function $r may not be initialised - assuming function pointer"
#define syn_rerr_archaic_init "Ancient form of initialisation, use '='"
#define syn_rerr_bitfield "illegal bit field type $t - 'int' assumed"
#define syn_rerr_missing_formal "formal name missing in function definition"
#define syn_rerr_ineffective "declaration with no effect"
#define syn_rerr_duplicate_member(sv,b) "duplicate member $r of $c", sv, b
#define syn_rerr_missing_type3 \
        "type or class needed (except in function definition) - 'int' assumed"
#define syn_rerr_semicolon_in_arglist \
        "',' (not ';') separates formal parameters"
#define syn_rerr_no_members "$c has no members"

%Z      /* The following remain as ordinary (uncompressed) strings */

#define syn_moan_hashif "#if <expression>"
#define syn_moan_case "case expression (ignored)"

%S /* The next batch of things just get mapped onto syserr codes */

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
