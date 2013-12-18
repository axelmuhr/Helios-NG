/*
 * C compiler file aeops.h, Version 45
 * Copyright (C) Codemist Ltd., 1988-1993.
 * Copyright Advanced RISC Machines Limited, 1990-1993.
 */

/*
 * RCS $Revision: 1.2 $
 * Checkin $Date: 1995/05/19 13:32:55 $
 * Revising $Author: nickc $
 */

#ifndef _aeops_LOADED
#define _aeops_LOADED 1

/* ACN 31-10-88: add some things needed for BCPL    */
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

enum AE_op_enum {
    s_nothing,

/* identifier, constants: */
    s_identifier,
    s_integer,
    s_floatcon,
    s_character,
    s_string,
    s_wcharacter,
    s_wstring,
/* s_binder heads binding records - see type Binder */
    s_binder,
    s_error,
    s_tagbind,  /* AM may want this soon LDS wants NOW */
    s_member,   /*  LDS wants this too... for C++       */

#define hastypefield_(op) ((s_invisible<=(op) && (op)<=s_cast))
#define hasfileline_(op) ((op)==s_integer || (hastypefield_(op)))

    s_invisible,
#define isdiad_(op)       (s_andand<=(op) && (op)<=s_assign)
/* expression operators, mainly non-letter symbols */
    s_andand,
    s_comma,
    s_oror,
    s_arrowstar,                /* C++ only */
    s_dotstar,                  /* C++ only */

/* relations: reorder to make NOT or SWAP easier? */
#define isrelational_(op) (s_equalequal<=(op) && (op)<=s_lessequal)
    s_equalequal,
    s_notequal,
#define isinequality_(op) (s_greater<=(op) && (op)<=s_lessequal)
    s_greater,
    s_greaterequal,
    s_less,
    s_lessequal,

/* NB: the next 3 blocks of opcodes must correspond. */

#define floatableop_(op) \
    (isrelational_(op)||(s_times<=(op) && (op)<=s_div)||(op) == s_cond)

    s_and,
    s_times,
    s_plus,
#if 0
    s_sum,		/*
			 * Unsigned addition for Transputer.  Added here so that
			 * it satisfies the macros defined above, specifically
			 * hastypefield_.  Tony 23/1/95.
			 */
#endif
    s_minus,
    s_power,            /* Needed for Fortran */
    s_div,
    s_leftshift,
    s_or,
    s_idiv,
    s_rem,
    s_rightshift,
    s_xor,

#define can_have_becomes(x) (((x)>= s_and) && ((x) <= s_xor))
#define and_becomes(x)  ((AE_op)((x)+(s_andequal-s_and)))
#define assignop_(op)   ((AE_op)((op)+(s_andequal-s_and)))
#define unassignop_(op) ((AE_op)((op)-(s_andequal-s_and)))
#define isassignop_(x)  (((x)>= s_andequal) && ((x) <= s_xorequal))

    s_andequal,
    s_timesequal,
    s_plusequal,
    s_minusequal,
    s_powerequal,       /* here for consistency - do not remove */
    s_divequal,
    s_leftshiftequal,
    s_orequal,
    s_idivequal,
    s_remequal,
    s_rightshiftequal,
    s_xorequal,

#define diadneedslvalue_(op)    (s_andequal<=(op) && (op)<=s_assign)
    s_displace,         /* no repn in C,  ++ is spec case */
    s_assign,

/* unary ops.  note that the first 4 correspond (via unaryop_()) to diads */
#define ismonad_(op)    (s_addrof<=(op) && (op)<=s_postdec)
#define unaryop_(x)     ((AE_op)((x) + (s_addrof-s_and)))
    s_addrof,
    s_content,
    s_monplus,
    s_neg,
    s_bitnot,
    s_boolnot,
/* move the next block of 4 to just before s_addrof? */
#define monadneedslvalue_(op)   (isincdec_(op) || (op)==s_addrof)
#define isincdec_(op)           (s_plusplus<=(op) && (op)<=s_postdec)
#define incdecisinc_(op)        ((op) <= s_postinc)
#define incdecispre_(op)        ((((op)-s_plusplus) & 1) == 0)
#define postop_(preop)          ((AE_op)((preop)+(s_postinc-s_plusplus)))
    s_plusplus,
    s_postinc,
    s_minusminus,
    s_postdec,
/* end of unary ops */
    s_dot,
    s_arrow,
    s_cond,

/* pseudo expression operators */
    s_fnap,
    s_fnapstruct,
    s_fnapstructvoid,
    s_subscript,
    s_let,
#ifdef EXTENSION_VALOF
    s_valof,            /* BCPL-like valof block support */
#endif
#ifdef RANGECHECK_SUPPORTED
    s_rangecheck,
    s_checknot,
#endif
    s_cast,
/* see hastypefield_() above */
    s_sizeoftype,
    s_sizeofexpr,
    s_typeoftype,
    s_typeofexpr,
    s_ptrdiff,

/* command nodes (mainly keywords): */
    s_array,
    s_begin,
    s_break,
    s_case,
    s_colon,
    s_continue,
    s_default,
    s_do,
    s_downto,
    s_else,
    s_end,
    s_endcase,          /* C switch break = bcpl endcase */
    s_file,
    s_for,
    s_function,
    s_goto,
    s_if,
    s_in,
    s_label,
    s_nil,
    s_of,
    s_packed,
    s_procedure,
    s_program,
    s_record,
    s_repeat,
    s_return,
    s_semicolon,
    s_set,
    s_switch,
    s_then,
    s_to,
    s_type,
    s_until,
    s_var,
    s_while,
    s_with,
    s_block,

#ifdef EXTENSION_VALOF
    s_resultis,         /* used with valof blocks */
#endif

/* declaration nodes: */
    s_decl,
    s_fndef,
    s_typespec,

/* note the next two blocks must be adjacent for the next 3 tests. */
#define istypestarter_(x)   (s_char<=(x) && (x)<=s_volatile)
#define isstorageclass_(x)  (s_auto<=(x) && (x)<=s_globalfreg)
#define isdeclstarter_(x)   (s_char<=(x) && (x)<=s_typestartsym)
#define shiftoftype_(x)     ((x)-s_char)
#define bitoftype_(x)       (1L<<((x)-s_char))
    s_char,
    s_double,
    s_enum,
    s_float,
    s_int,
    s_struct,
    s_class,            /* here, whether or not C++     */
    s_union,
#  define CLASSBITS         (bitoftype_(s_union+1)-bitoftype_(s_struct))
#  define ENUMORCLASSBITS   (CLASSBITS|bitoftype_(s_enum))
    s_void,
    s_longlong,         /* C extension */
    s_typedefname,
#define NONQUALTYPEBITS (bitoftype_(s_typedefname+1)-bitoftype_(s_char))
/* modifiers last (high bits for m&-m trick) */
    s_long,
    s_short,
    s_signed,
    s_unsigned,
/* rework the next two lines?                                           */
#define TYPEDEFINHIBITORS   (bitoftype_(s_unsigned+1)-bitoftype_(s_char))
#define CVBITS              (bitoftype_(s_volatile)|bitoftype_(s_const))
    s_const,
    s_volatile,
#define NUM_OF_TYPES        (s_volatile-s_char+1)
    /* currently 17 */
#define TYPEBITS            (bitoftype_(s_volatile+1)-bitoftype_(s_char))
/* storage classes and qualifiers */
#define bitofstg_(x)        bitoftype_(x)
    s_auto,
    s_extern,
    s_static,
    s_typedef,
    s_globalreg,
    s_register,
    s_friend,
    s_inline,
    s_virtual,
    s_weak,

    s_globalfreg,
/* N.B. s_register is equivalent to 0x400000.  See h.defs for other bits */
#define PRINCSTGBITS        (bitoftype_(s_register)-bitoftype_(s_auto))
#define STGBITS             (bitoftype_(s_weak+1)-bitoftype_(s_auto))
#define NUM_OF_STGBITS      (s_weak-s_auto+1)
    /* currently 9 */
#define bitoffnaux_(x)      (1L<<((x)-s_pure))
#define isfnaux_(x)         (s_pure<=(x) && (x)<s_typestartsym)
    s_pure,
    s_structreg,
    s_swi,
    s_swi_i,
    s_irq,

    s_typestartsym,     /* used to help error reporting */

/* impedementia (not appearing in parse trees) */
    s_toplevel,
    s_lbrace,
    s_lbracket,
    s_lpar,
    s_rbrace,
    s_rbracket,
    s_rpar,
    s_typeof,           /* 2 special cases above */
    s_sizeof,           /* 2 special cases above */
    s_ellipsis,
    s_eol,
    s_eof,

#ifdef EXTENSION_UNSIGNED_STRINGS
    s_ustring,
    s_sstring,
#endif
/* Here are some symbols that BCPL seems to want...                 */
/* inserted so that an experiment with a BCPL parser can happen.    */

    s_global,
    s_manifest,
    s_abs,
    s_get,
    s_eqv,
    s_query,
    s_vecap,
    s_andlet,
    s_be,
    s_by,
    s_false,
    s_finish,
    s_into,
    s_repeatwhile,
    s_repeatuntil,
    s_test,
    s_true,
    s_table,
    s_unless,
    s_vec,
    s_valdef,

    s_boolean,          /* needed for Fortran */
    s_complex,          /* needed for Fortran */


/* C++ keywords not in ANSI C.                                          */
/* AM: memo, arrange non-aetree ops to be treated by langfe\*.[ch]      */
/* via s_frontend to avoid mip getting lots of ops per language.        */
    s_asm,
    s_catch,
    s_delete,
    s_new,
    s_operator,

#  define isaccessspec_(op) (s_private <= (op) && (op) <= s_public )
/* bitofaccess_() bits are contiguous with CLASSBITS... */
#  define bitofaccess_(x)  (1L<<((x)-s_private+shiftoftype_(s_union+1)))
#  define ACCESSBITS  (bitofaccess_(s_public+1)-bitofaccess_(s_private))
    s_private,
    s_protected,
    s_public,

    s_template,
    s_this,
    s_throw,
    s_try,
/* non-keyword C++ operators... */
    s_coloncolon,
    s_convfn,           /* C++ front end only (temp?)   */

    s_NUMSYMS
};
typedef enum AE_op_enum		AE_op;

extern char *sym_name_table[s_NUMSYMS];

/* synonyms (used in types for clarity) -- soon a separate ADT          */
/* (along with s_typespec!, but see use of t_coloncolon etc in syn.c).  */

#define t_fnap      s_fnap
#define t_subscript s_subscript
#define t_content   s_content
#define t_ref       s_addrof
#define t_coloncolon  s_coloncolon
#define t_ovld      s_packed            /* a random name */

#ifdef EXTENSION_UNSIGNED_STRINGS
#  define isstring_(op) ((op)==s_string||(op)==s_wstring||(op)==s_ustring||(op)==s_sstring)
#  define case_s_any_string  case s_string: case s_wstring: case s_ustring: case s_sstring:
#else
#  define isstring_(op) ((op)==s_string ||(op)==s_wstring)
#  define case_s_any_string  case s_string: case s_wstring:
#endif

#endif

/* end of file aeops.h */
