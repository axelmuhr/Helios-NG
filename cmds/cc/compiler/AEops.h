/* C compiler file AEops.h  Copyright (C) A.C.Norman, A.Mycroft */
/* Version 0.32a */
/* $Id: AEops.h,v 1.1 1990/09/13 17:11:04 nick Exp $ */

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

#define  s_nothing              0

/* identifier, constants: */
#define  s_character            1
#define  s_integer              2
#define  s_floatcon             3
#define  s_string               4
#define  s_identifier           5
/* s_binder heads binding records - see type Binder */
#define  s_binder               6
#define  s_error                7
#define  s_tagbind              8 /* AM may want this soon */

#define hastypefield_(op)       (s_invisible<=(op) && (op)<=s_cast)
#define  s_invisible            9
#define isdiad_(op)             (s_andand<=(op) && (op)<=s_assign)
/* expression operators, mainly non-letter symbols */
#define  s_andand               10
#define  s_comma                11
#define  s_oror                 12

/* relations: reorder to make NOT or SWAP easier? */
#define isrelational_(op) (s_equalequal<=(op) && (op)<=s_lessequal)
#define  s_equalequal           13
#define  s_notequal             14
#define  s_greater              15
#define  s_greaterequal         16
#define  s_less                 17
#define  s_lessequal            18

/* NB: the next 3 blocks of opcodes must correspond. */

#define floatableop_(op) \
    (isrelational_(op)||(s_times<=(op) && (op)<=s_div)||(op) == s_colon)

#define  s_and                  19
#define  s_times                20
#define  s_plus                 21
#define  s_minus                22
#define  s_div                  23
#define  s_leftshift            24
#define  s_or                   25
#define  s_idiv                 26
#define  s_rem                  27
#define  s_rightshift           28
#define  s_xor                  29

#define can_have_becomes(x) (((x)>= s_and) && ((x) <= s_xor))
#define and_becomes(x)  ((x)+(s_andequal-s_and))
#define assignop_(op)   ((op)+(s_andequal-s_and))
#define unassignop_(op) ((op)-(s_andequal-s_and))
#define isassignop_(x) (((x)>= s_andequal) && ((x) <= s_xorequal))

#define  s_andequal             30
#define  s_timesequal           31
#define  s_plusequal            32
#define  s_minusequal           33
#define  s_divequal             34
#define  s_leftshiftequal       35
#define  s_orequal              36
#define  s_idivequal            37
#define  s_remequal             38
#define  s_rightshiftequal      39
#define  s_xorequal             40

#define diadneedslvalue_(op)    (s_andequal<=(op) && (op)<=s_assign)
#define  s_displace             41      /* no repn in C,  ++ is spec case */
#define  s_assign               42

/* unary ops.  note that the first 4 correspond (via unaryop_()) to diads */
#define ismonad_(op)   (s_addrof<=(op) && (op)<=s_postdec)
#define unaryop_(x)    ((x) + (s_addrof-s_and))
#define  s_addrof               43
#define  s_content              44
#define  s_monplus              45
#define  s_neg                  46
#define  s_bitnot               47
#define  s_boolnot              48
/* move the next block of 4 to just before s_addrof? */
#define monadneedslvalue_(op)   (isincdec_(op) || (op)==s_addrof)
#define isincdec_(op)           (s_plusplus<=(op) && (op)<=s_postdec)
#define incdecisinc_(op)        ((op) <= s_postinc)
#define incdecispre_(op)        ((op) & 1)
#define  postop_(preop)         ((preop)+(s_postinc-s_plusplus))
#define  s_plusplus             49
#define  s_postinc              50
#define  s_minusminus           51
#define  s_postdec              52
/* end of unary ops */

#define  s_dot                  53
#define  s_arrow                54
#define  s_cond                 55

/* pseudo expression operators */
#define  s_fnap                 56
#define  s_subscript            57
#define  s_let                  58
#ifndef NO_VALOF_BLOCKS
#define  s_valof                59      /* BCPL-like valof block support */
#endif
#define  s_cast                 60
/* see hastypefield_() above */
#define  s_sizeoftype           61
#define  s_sizeofexpr           62

/* command nodes (mainly keywords): */
#define  s_array                63
#define  s_begin                64
#define  s_break                65
#define  s_case                 66
#define  s_colon                67
/* ----- spare                  68    ------- */
#define  s_continue             69
#define  s_default              70
#define  s_do                   71
#define  s_downto               72
#define  s_else                 73
#define  s_end                  74
#define  s_endcase              75   /* C switch break = bcpl endcase */
#define  s_file                 76
#define  s_for                  77
#define  s_function             78
#define  s_goto                 79
#define  s_if                   80
#define  s_in                   81
#define  s_label                82
#define  s_nil                  83
#define  s_of                   84
#define  s_packed               85
#define  s_procedure            86
#define  s_program              87
#define  s_record               88
#define  s_repeat               89
#define  s_return               90
#define  s_semicolon            91
#define  s_set                  92
#define  s_switch               93
#define  s_then                 94
#define  s_to                   95
#define  s_type                 96
#define  s_until                97
#define  s_var                  98
#define  s_while                99
#define  s_with                100

#define  s_block              (101)

#ifndef NO_VALOF_BLOCKS
#define  s_resultis            102      /* used with valof blocks */
#endif

/* declaration nodes: */
#define  s_decl                103
#define  s_fndef               104
#define  s_typespec            105

/* note the next two blocks must be adjacent for the next 3 tests. */
#define istypestarter_(x)  (s_char<=(x) && (x)<=s_volatile)
#define isstorageclass_(x) (s_auto<=(x) && (x)<=s_register)
#define isdeclstarter_(x)  (s_char<=(x) && (x)<=s_register)
#define shiftoftype_(x)    ((x)-s_char)
#define bitoftype_(x)      (1<<((x)-s_char))
#define  s_char                106
#define  s_double              107
#define  s_enum                108
#define  s_float               109
#define  s_int                 110
#define  s_struct              111
#define  s_union               112
#define  s_void                113
#define  s_typedefname        (114)
#define NONQUALTYPEBITS      0x1FF
/* modifiers last (high bits for m&-m trick) */
#define  s_long                115
#define  s_short               116
#define  s_signed              117
#define  s_unsigned            118
#define TYPEDEFINHIBITORS    0x1FFF  /* ie everything above here */
#define  s_const               119
#define  s_volatile            120
#define NUM_OF_TYPES       (s_volatile-s_char+1)
/* storage classes and qualifiers */
#define bitofstg_          bitoftype_
#define PRINCSTGBITS       (bitoftype_(s_register)-bitoftype_(s_auto))
#define STGBITS            (bitoftype_(s_register+1)-bitoftype_(s_auto))
#define  s_auto                121
#define  s_extern              122
#define  s_static              123
#define  s_typedef             124
#define  s_register            125
/* N.B. s_register is equivalent to 0x80000.  See h.modes for other bits */

/* impedimentia (not appearing in parse trees) */
#define  s_lbrace              126
#define  s_lbracket            127
#define  s_lpar                128
#define  s_rbrace              129
#define  s_rbracket            130
#define  s_rpar                131
#define  s_sizeof              132  /* 2 special cases above */
#define  s_ellipsis            133
#define  s_eol                 134
#define  s_eof                 135

/* preprocessor-only symbols (NB: these are NOT reserved C ids) */
#define  s_sharp               136
#define  s_define              137
#define  s_defined             138
#define  s_elif                139
#define  s_endif               140
#define  s_ifdef               141
#define  s_ifndef              142
#define  s_include             143
#define  s_line                144
#define  s_pragma              145
#define  s_undef               146

/* One more for the transputer only ... */
#ifdef TARGET_IS_XPUTER
#define  s_diff                147
#endif
/* remember NUMSYMS is 1 greater than the last number used.
   apologies for gaps (about 10 worth)
*/
#define  s_NUMSYMS             148

extern char *sym_name_table[s_NUMSYMS];

/* synonyms (used in types for clarity) */

#define t_fnap s_fnap
#define t_subscript s_subscript
#define t_content s_content

#endif

/* End of AEops.h header file */
