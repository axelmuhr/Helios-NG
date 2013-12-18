
/* C compiler file AEops.h  Copyright (C) A.C.Norman, A.Mycroft */
/* Version 0.33 */

#ifndef _AEOPS_LOADED

#define _AEOPS_LOADED 1

/* ACN 11-12-86: add some things needed for Pascal. */

/* Notes (AM):
   1) some lexical operators, like ++ and & correspond to more
      than one AE tree operation (pre/post incr. and bitand/addrof).
      The lexical routines are all assumed to return the operator
      corresponding to the binary or prefix form.
      Macros unaryop_() and postop_() are here provided for the syntax
      analyser to use for getting the other form.
   2) Assignment operators are treated similarly.  This time by
      the lexical routines which use assignop_() to get the
      assignment form.
   3) It goes without saying that the caller must only apply
      these macros to valid arguments.
   4) s_and provides a good view of this.
   5) some AE operators do not have tokens (e.g. s_fnap, s_block).
      I have tried to enclose these in parens in the following.
*/

/* Here is a list of all tokens types used in C                        */

#define  s_nothing              0L

/* identifier, constants: */
#define  s_character            1L
#define  s_integer              2L
#define  s_floatcon             3L
#define  s_string               4L
#define  s_identifier           5L
/* s_binder heads binding records - see type Binder */
#define  s_binder               6L
#define  s_error                7L
#define  s_tagbind              8L /* AM may want this soon */

#define hastypefield_(op)       (s_invisible<=(op) && (op)<=s_cast)
#define  s_invisible            9L
#define isdiad_(op)             (s_andand<=(op) && (op)<=s_assign)
/* expression operators, mainly non-letter symbols */
#define  s_andand               10L
#define  s_comma                11L
#define  s_oror                 12L

/* relations: reorder to make NOT or SWAP easier? */
#define isrelational_(op) (s_equalequal<=(op) && (op)<=s_lessequal)
#define  s_equalequal           13L
#define  s_notequal             14L
#define  s_greater              15L
#define  s_greaterequal         16L
#define  s_less                 17L
#define  s_lessequal            18L

/* NB: the next 3 blocks of opcodes must correspond. */

#define floatableop_(op) \
    (isrelational_(op)||(s_times<=(op) && (op)<=s_div)||(op) == s_cond)

#define  s_and                  19L
#define  s_times                20L
#define  s_plus                 21L
#define  s_minus                22L
#define  s_div                  23L
#define  s_leftshift            24L
#define  s_or                   25L
#define  s_idiv                 26L
#define  s_rem                  27L
#define  s_rightshift           28L
#define  s_xor                  29L

#define can_have_becomes(x) (((x)>= s_and) && ((x) <= s_xor))
#define and_becomes(x)  ((x)+(s_andequal-s_and))
#define assignop_(op)   ((op)+(s_andequal-s_and))
#define unassignop_(op) ((op)-(s_andequal-s_and))
#define isassignop_(x) (((x)>= s_andequal) && ((x) <= s_xorequal))

#define  s_andequal             30L
#define  s_timesequal           31L
#define  s_plusequal            32L
#define  s_minusequal           33L
#define  s_divequal             34L
#define  s_leftshiftequal       35L
#define  s_orequal              36L
#define  s_idivequal            37L
#define  s_remequal             38L
#define  s_rightshiftequal      39L
#define  s_xorequal             40L

#define diadneedslvalue_(op)    (s_andequal<=(op) && (op)<=s_assign)
#define  s_displace             41L     /* no repn in C,  ++ is spec case */
#define  s_assign               42L

/* unary ops.  note that the first 4 correspond (via unaryop_()) to diads */
#define ismonad_(op)   (s_addrof<=(op) && (op)<=s_postdec)
#define unaryop_(x)    ((x) + (s_addrof-s_and))
#define  s_addrof               43L
#define  s_content              44L
#define  s_monplus              45L
#define  s_neg                  46L
#define  s_bitnot               47L
#define  s_boolnot              48L
/* move the next block of 4 to just before s_addrof? */
#define monadneedslvalue_(op)   (isincdec_(op) || (op)==s_addrof)
#define isincdec_(op)           (s_plusplus<=(op) && (op)<=s_postdec)
#define incdecisinc_(op)        ((op) <= s_postinc)
#define incdecispre_(op)        ((op) & 1L)
#define  postop_(preop)         ((preop)+(s_postinc-s_plusplus))
#define  s_plusplus             49L
#define  s_postinc              50L
#define  s_minusminus           51L
#define  s_postdec              52L
/* end of unary ops */

#define  s_dot                  53L
#define  s_arrow                54L
#define  s_cond                 55L

/* pseudo expression operators */
#define  s_fnap                 56L
#define  s_subscript            57L
#define  s_let                  58L
#ifndef NO_VALOF_BLOCKS
#define  s_valof                59L     /* BCPL-like valof block support */
#endif
#define  s_cast                 60L
/* see hastypefield_() above */
#define  s_sizeoftype           61L
#define  s_sizeofexpr           62L
#define  s_ptrdiff              63L

/* command nodes (mainly keywords): */
#define  s_array                64L
#define  s_begin                65L
#define  s_break                66L
#define  s_case                 67L
#define  s_colon                68L
#define  s_continue             69L
#define  s_default              70L
#define  s_do                   71L
#define  s_downto               72L
#define  s_else                 73L
#define  s_end                  74L
#define  s_endcase              75L  /* C switch break = bcpl endcase */
#define  s_file                 76L
#define  s_for                  77L
#define  s_function             78L
#define  s_goto                 79L
#define  s_if                   80L
#define  s_in                   81L
#define  s_label                82L
#define  s_nil                  83L
#define  s_of                   84L
#define  s_packed               85L
#define  s_procedure            86L
#define  s_program              87L
#define  s_record               88L
#define  s_repeat               89L
#define  s_return               90L
#define  s_semicolon            91L
#define  s_set                  92L
#define  s_switch               93L
#define  s_then                 94L
#define  s_to                   95L
#define  s_type                 96L
#define  s_until                97L
#define  s_var                  98L
#define  s_while                99L
#define  s_with                100L

#define  s_block              (101L)

#ifndef NO_VALOF_BLOCKS
#define  s_resultis            102L     /* used with valof blocks */
#endif

/* declaration nodes: */
#define  s_decl                103L
#define  s_fndef               104L
#define  s_typespec            105L

/* note the next two blocks must be adjacent for the next 3 tests. */
#define istypestarter_(x)  (s_char<=(x) && (x)<=s_volatile)
#define isstorageclass_(x) (s_auto<=(x) && (x)<=s_register)
#define isdeclstarter_(x)  (s_char<=(x) && (x)<=s_register)
#define shiftoftype_(x)    ((x)-s_char)
#define bitoftype_(x)      (1L<<((x)-s_char))
#define  s_char                106L
#define  s_double              107L
#define  s_enum                108L
#define  s_float               109L
#define  s_int                 110L
#define  s_struct              111L
#define  s_union               112L
#define  s_void                113L
#define  s_typedefname        (114L)
#define NONQUALTYPEBITS      0x1FFL
/* modifiers last (high bits for m&-m trick) */
#define  s_long                115L
#define  s_short               116L
#define  s_signed              117L
#define  s_unsigned            118L
#define TYPEDEFINHIBITORS    0x1FFFL  /* ie everything above here */
#define  s_const               119L
#define  s_volatile            120L
#define NUM_OF_TYPES       (s_volatile-s_char+1)
/* storage classes and qualifiers */
#define bitofstg_(x)        bitoftype_(x)
#define PRINCSTGBITS       (bitoftype_(s_register)-bitoftype_(s_auto))
#define STGBITS            (bitoftype_(s_register+1)-bitoftype_(s_auto))
#define  s_auto                121L
#define  s_extern              122L
#define  s_static              123L
#define  s_typedef             124L
#define  s_register            125L
/* N.B. s_register is equivalent to 0x80000.  See h.modes for other bits */

/* impedementia (not appearing in parse trees) */
#define  s_lbrace              126L
#define  s_lbracket            127L
#define  s_lpar                128L
#define  s_rbrace              129L
#define  s_rbracket            130L
#define  s_rpar                131L
#define  s_sizeof              132L  /* 2 special cases above */
#define  s_ellipsis            133L
#define  s_eol                 134L
#define  s_eof                 135L

/* preprocessor-only symbols (NB: these are NOT reserved C ids) */
#define  s_sharp               136L
#define  s_define              137L
#define  s_defined             138L
#define  s_elif                139L
#define  s_endif               140L
#define  s_ifdef               141L
#define  s_ifndef              142L
#define  s_include             143L
#define  s_line                144L
#define  s_pragma              145L
#define  s_undef               146L

/* remember NUMSYMS is 1 greater than the last number used.
   apologies for gaps (about 10 worth)
*/
#define  s_NUMSYMS             147L

extern char *sym_name_table[s_NUMSYMS];

/* synonyms (used in types for clarity) */

#define t_fnap s_fnap
#define t_subscript s_subscript
#define t_content s_content

#endif

/* End of AEops.h header file */
