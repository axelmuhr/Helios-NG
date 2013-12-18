/*
 * C compiler file lex.c
 * Copyright (C) Codemist Ltd., 1988-1992
 * Copyright (C) Acorn Computers Ltd., 1988-1990
 * Copyright (C) Advanced RISC Machines Ltd., 1990-1992
 */

/*
 * RCS $Revision: 1.2 $ Codemist 61
 * Checkin $Date: 1995/05/19 11:34:01 $
 * Revising $Author: nickc $
 */

/*
 *   AM Feb 92: add some C++-isms when CPLUSPLUS is #defined.
 *              When not defined only effect is a warning for C++ keywords.
 *   AM Nov 89: treat ANSI pp-number conversion errors properly.
 *   AM Jun 89: add 16 bit int support
 *   13.x.88   Pascal split off
 *   AM 23-10-86: Fix bug in string wrap round segments, add cc_rerr() calls.
 *   AM 28-7-86: Recognise += in lexer without whitespace, glue (nextsym)
 *               '+ =' with space/tab white space.  Problematic here
 *               to recognise '+\n='.  A #if would clobber things.
 */

#ifdef __STDC__
#  ifndef __GNUC__
#    include <stddef.h>
#  endif
#  include <string.h>
#  ifdef __GNUC__
#    include <stddef.h>
#  endif
#else
#  include "stddef.h"
#  include <strings.h>
#endif
#include <ctype.h>

#include "globals.h"
#include "lex.h"
#include "bind.h"
#include "pp.h"
#include "store.h"
#include "util.h"
#include "aeops.h"
#include "errors.h"

static FileLine endofsym_fl;

/* exports: nextsym(), curlex, nextsym_for_hashif(),
            lex_init(),            and some more! */

/* utilities */
static int intofxdigit(int c)
/* convert hex digit to integer  */
{   return(isdigit(c) ? (c) - '0' :
           islower(c) ? (c) - 'a' + 10 :
           isupper(c) ? (c) - 'A' + 10 :
           -1);
}

#define NOTACHAR        0

#define LEX_STRBUFSIZE  256  /* (multiple of 4) initial size lex_strbuf */
static char *lex_strptr;     /* next free char in lex_strbuf            */
static char *lex_strend;     /* one beyond end of lex_strbuf            */

static SymInfo prevlex, nextlex = {s_nothing};

static int curchar;          /* The look-ahead character */
/* The next 3 variables are notionally a single tuple. Export to misc. */
SymInfo curlex;              /* Current token and aux. info.            */
#ifdef EXTENSION_VALOF
bool inside_valof_block;
static Symstr *resultisword;
#endif

int errs_on_this_sym;    /* To help error recovery */


/* the +4 in the next line is slop so we do not check on dec. pt. etc.  */
static char namebuf[NAMEMAX+4];     /* Buffer for reading identifiers   */

static unsigned32 lexclass[1+255];
                                    /* Character table, yielding:
                                       glue-ishness (low 8 bits),
                                       corresponding symbol (next 8 bits),
                                       doubled symbol (next 8 bits)
                                       relop equal symbol (top 8 bits)     */
#define lexclass_(ch) lexclass[ch & 0xff]
/* or [(unsigned char)ch], but there are compilers which have difficulty
   with that
 */

/* 'curlex.a2.flag' values - only exported for make_integer (q.v.) */
/* the values are chosen for easy punning into sem.c types  */

#define NUM_FLOAT  bitoftype_(s_double)
#define NUM_INT    bitoftype_(s_int)
#define NUM_LONG   bitoftype_(s_long)
#define NUM_SHORT  bitoftype_(s_short)
#define NUM_UNSIGN bitoftype_(s_unsigned)
#ifdef CPLUSPLUS
#  define NUM_CHAR bitoftype_(s_char)   /* Type of 'a' is char in C++.  */
#else
#  define NUM_CHAR NUM_INT              /* Type of 'a' is int in C.     */
#endif
/* Note that NUM_WCHAR is defined in defaults.h.                        */


/* lexical classes - local to this file */

#define l_illegal   0L
#define l_noglue    1L
#define l_selfglue  2L
#define l_idstart   3L
#define l_digit0    4L
#define l_digit1    5L
#define l_quote1    6L
#define l_quote2    7L
#define l_dot       8L
#define l_shifter   9L
#define l_eqglue    10L
#define l_minus     11L
#define l_lpar      12L
#define l_less      13L
#define LEX_CHTYPE  15L  /* mask */
#define l_idcont    16L  /* orable */
#define mklc(l,t1,t2,t3) (((unsigned32)(l)) | ((unsigned32)(t1) << 8) | \
                          ((unsigned32)(t2) << 16) | ((unsigned32)(t3) << 24))

/* lex 'semantic' routines... */

/* make_floating uses GLOBAL store for its value.  Is this reasonable? */
static AEop make_floating(char *s, int32 flag)
{   FloatCon *x = real_of_string(s,flag);
    curlex.a1.fc = x;
    return x->h0;
}

/* make_integer does not use GLOBAL or LOCAL store.  It could use
   the former (but this could be wasteful), or LOCAL, but beware placement
   of alloc_reinit() in driver.c and lookahead tokens.
   Currently it just assumes that syn.c will look at curlex.a2.flag.
*/
static AEop make_integer(int32 radix, int32 flag)
/* Numbers that overflow 2**32-1 lead to a diagnostic.                 */
/* Otherwise their type suffix (in flag) is modified to yield a type.  */
/* Note that octal and hex numbers sometimes have different types.     */
/* I have allowed the digits '8' and '9' to build up even in           */
/* octal numbers - this is checked for here.                           */
/* BEWARE: possible two's complement dependency.                       */
{
    bool overflow = 0, badoct = 0;
    unsigned32 value = 0;
    char c, *cp = namebuf;
    switch (radix)
    {
case 8: while ((c = *cp++) != 0)
        {   if ((value & 0xe0000000)!=0) overflow=1;
            if (c>='8') badoct=1;
            value=(value<<3) | (int)intofdigit(c);
        }
        if (badoct) cc_rerr(lex_rerr_8_or_9);
        break;
case 10:while ((c = *cp++) != 0)
        {   bool bigish = 0;
            if (value >= ((unsigned32)0xffffffff)/10) bigish = 1;
            if (value > ((unsigned32)0xffffffff)/10) overflow=1;
            value = value*10 + (int)intofdigit(c);
            if (bigish && value < 10) overflow=1;
        }
        break;
case 16:while ((c = *cp++) != 0)
        {   if ((value & 0xf0000000)!=0) overflow=1;
            value = (value<<4) | intofxdigit(c);
        }
        break;
    }
    if (overflow)
        cc_err(lex_err_ioverflow, namebuf);

/* Now modify 'flag' (if necessary) to get a type for the constant.     */
    /* ANSI rules (bracketted cases cannot happen if int==long):
     *             (10) int, [long], unsigned long.
     *             (8,16) int, unsigned, [long], [unsigned long].
     *             (u)  unsigned, [unsigned long].
     *             (l)  long, unsigned long.
     *             (ul) unsigned long.
     */
    if (sizeof_int == 2)                /* TARGET_HAS_16_BIT_INT        */
    {    if (value & 0xffff0000 || flag & NUM_LONG)
        {   flag |= NUM_LONG;
            if (value & 0x80000000 && !(flag & NUM_UNSIGN))
            {   if (radix == 10)
                    if (!overflow)     /* only 1 error message */
                        cc_ansi_warn(lex_warn_force_unsigned, namebuf, namebuf);
                flag |= NUM_UNSIGN;
            }
        }
        else
        {   if (value & 0x8000 && !(flag & NUM_UNSIGN))
            {   /* warnings here?                                       */
                flag |= (radix == 10) ? NUM_LONG : NUM_UNSIGN;
            }
        }
    }
    else                                /* TARGET_HAS_32_BIT_INT        */
    {   /* (As above) the rules simplify if sizeof_int == sizeof_long == 4. */
        /* Since if value is +ve or unsigned then 'flag' is OK, else        */
        /* we need to convert to 'ul' (decimal) else 'flag+u' otherwise.    */
        if ((int32)value < 0  && !(flag & NUM_UNSIGN))
        {   flag |= NUM_UNSIGN;    /* add 'U' - see ANSI spec */
            if (radix == 10)       /* but for base 10 add 'L' too */
            {   flag |= NUM_LONG;
                if (!overflow)     /* only 1 error message */
                    cc_ansi_warn(lex_warn_force_unsigned, namebuf, namebuf);
            }
        }
    }
    curlex.a1.i = value;
    curlex.a2.flag = flag;
    return s_integer;
}

/* The following routine does checks for 'constraint violation' when    */
/* coverting a pp_number to a token, caller has already checked for     */
/* illegal uses of 'e+' in fp or hex numbers.                           */
static void lex_check_pp_number()
{   if (curchar == '.' ||
        curchar != PP_EOF && (lexclass_(curchar) & l_idcont))
            cc_ansi_rerr(lex_rerr_pp_number);
}

/* reading routines */

static int nextchar(void)
/* I may also want to buffer some recently read characters so that I   */
/* can display them when I notice an error.                            */
{
    if (curchar==PP_EOF) return curchar;
    curchar = pp_nextchar();
    return curchar;
}

#define is_e(c)                         /* e or E (exponent marker) */ \
    (((c) == 'e') || ((c) == 'E'))

#define is_x(c)                         /* x or X (hex num marker)  */ \
    (((c) == 'x') || ((c) == 'X'))


static int32 read_floating(int32 k)
{   int32 flag = NUM_FLOAT;
    while (isdigit(curchar))
    {   if (k < NAMEMAX)
        {   namebuf[k++] = curchar;
            nextchar();
        }
        else
        {   cc_err(lex_err_overlong_fp);
            while (isdigit(curchar)) nextchar();
        }
    }
    if (is_e(curchar))
    {
        namebuf[k++] = 'e';  /* normalize case of the exponent mark */
        nextchar();
        if ((curchar == '+') | (curchar=='-'))
        {   namebuf[k++] = curchar;
            nextchar();
        }
        if (!isdigit(curchar))
        {   cc_err(lex_err_fp_syntax1);
            /* Assume exponent of zero */
            namebuf[k++] = '0';
        }
        else
        {   while (isdigit(curchar))
            {   if (k < NAMEMAX)
                {   namebuf[k++] = curchar;
                    nextchar();
                }
                else
                {   cc_err(lex_err_overlong_fp);
                    while (isdigit(curchar)) nextchar();
                }
            }
        }
    }
    /* note that calls ensure that k > 0 here. */
    namebuf[k] = '\0';
    switch (curchar)
    {   case 'l': case 'L': flag |= NUM_LONG; nextchar(); break;
        case 'f': case 'F': flag |= NUM_SHORT; nextchar(); break;
    }
/* Note that a fp number ending in 'E' above has already been diagnosed */
    lex_check_pp_number();
    return flag;
}

static AEop read_number(int radix)
{   int32 flag = NUM_INT, k = 0;        /* namebuf useful to collect chars */
    if (radix == 16)
    {   while (isxdigit(curchar))
        {   if (k < NAMEMAX)
            {   namebuf[k++] = curchar;
                nextchar();
            }
            else
            {   cc_err(lex_err_overlong_hex);
                while (isxdigit(curchar)) nextchar();
            }
        }
        if (k==0)
        {   cc_err(lex_err_need_hex_dig);
            namebuf[k++] = '0';         /* treat as 0x0 */
        }
        namebuf[k] = '\0';
/* ANSI (Dec 88 draft) require a diagnostic for 0xee+1 or 0xee+spqr.    */
        if (is_e(namebuf[k-1]) && (curchar == '+' || curchar == '-'))
            cc_ansi_rerr(lex_rerr_hex_exponent);
    }
    else
    {   while (isdigit(curchar))
        {   if (k < NAMEMAX)
            {   namebuf[k++] = curchar;
                nextchar();
            }
            else
            {   cc_err(lex_err_overlong_int);
                while (isdigit(curchar)) nextchar();
            }
        }
        if (curchar=='.')
        {   nextchar();
            if (k == 0) namebuf[k++] = '0'; /* consider '0.' */
            namebuf[k++] = '.';
            flag = read_floating(k);
        }
        else if (is_e(curchar))
        {   if (k == 0) namebuf[k++] = '0'; /* consider '0e5' */
            flag = read_floating(k);
        }
        else
        {   if (k == 0) namebuf[k++] = '0'; /* consider '0'   */
            namebuf[k] = '\0';
        }
    }
    if (flag & NUM_FLOAT) return make_floating(namebuf,flag);
    for (;;)
    {
        if ((curchar == 'l' || curchar == 'L') && !(flag & NUM_LONG))
        {   flag |= NUM_LONG; nextchar(); continue;
        }
        if ((curchar == 'u' || curchar == 'U') && !(flag & NUM_UNSIGN))
        {   flag |= NUM_UNSIGN; nextchar(); continue;
        }
        lex_check_pp_number();
        return make_integer(radix, flag);
    }
}


/* string and char reading (unified reading & semantic routines) ... */

/* The wide character support below supports wchar_t == target int.     */
/* You can change this via mip/defaults.h.                              */
static unsigned32 lex_string_insert(char *where, int size, unsigned32 what)
{   switch (size)
    {   case 4: return *(unsigned32 *)where = (unsigned32)what;
        case 2: return *(unsigned16 *)where = (unsigned16)what;
        case 1: return *(unsigned8 *)where = (unsigned8)what;
        default: syserr(syserr_lex_string); return what;
    }
}

static bool lex_string_char(char *where, int size, bool escaped)
/* read a possibly escaped (with backslash (\)) character for a           */
/* character or string literal (possibly wide).  If read succeeds         */
/* result is placed in where (if char) and (<size> *)where (if wide char).*/
/* Caller aligns if wide.  Result is 1 if succeeds, 0 if null escape read */
{
    int ch = curchar;
    if (escaped) switch (ch)     /* (next character has already been read) */
    {
case 'a':       ch = BELL;      break;  /* attn (otherwise known as bell) */
case 'b':       ch = '\b';      break;  /* backspace                      */
case 'f':       ch = CHAR_FF;   break;  /* form feed                      */
case 'n':       ch = CHAR_NL;   break;  /* newline                        */
case 'r':       ch = CHAR_CR;   break;  /* carriage return                */
case 't':       ch = '\t';      break;  /* (horizontal) tab               */
case 'v':       ch = CHAR_VT;   break;  /* vertical tab                   */

case '\\':      ch = '\\';      break;  /* backslash                      */
case '\'':      ch = '\'';      break;  /* single quote mark              */
case '\"':      ch = '\"';      break;  /* double quote mark              */
case '?':       ch = '?';       break;  /* '?' in case \?\?\? is needed   */

case 'x':
        {   bool ovfl = 0;
            unsigned32 escch, resultch;
            if (!isxdigit(ch = nextchar()))
            {   cc_err(lex_err_need_hex_dig1);
                ch = 'x';
                goto return_ch;       /* "\xg" -> "xg": any better ideas? */
            }
            escch = intofxdigit(ch);
            /* ANSI (3rd review draft) say any any number of digits.      */
            while (isxdigit(nextchar()))
            {   if (((escch << 4) >> 4) != escch) ovfl = 1;
                escch = (escch<<4) + intofxdigit(curchar);
            }
            /* Never translate \x escapes.                                */
            resultch = lex_string_insert(where, size, escch);
            if (ovfl || escch != resultch)    /* constraint violation     */
                cc_rerr(lex_rerr_esc16_truncated, ovfl ? "...": "",
                        (long)escch, (long)resultch);
            return 1;
        }

case '0':       case '1':
case '2':       case '3':               /* octal escapes                  */
case '4':       case '5':
case '6':       case '7':
/* This code is simpler than that for \x since 3 digits make overflow hard */
        {   unsigned32 resultch;
                ch = intofdigit(ch);
                if (isodigit(nextchar()))       /* second & third digits  */
                {   ch = (ch<<3) + intofdigit(curchar);
                    if (isodigit(nextchar()))
                        ch = (ch<<3) + intofdigit(curchar), nextchar();
                }
            /* Never translate \ooo escapes.                              */
            resultch = lex_string_insert(where, size, ch);
            if (ch != resultch)                   /* constraint violation */
                cc_rerr(lex_rerr_esc8_truncated, ch, (int)resultch);
            return 1;
        }

/* note that the sequence \<nl> (but not \<space><nl>) is now removed at
   all possible occasions by the preprocessor.   However, code for handling
   such situations is left here so that \<space><nl> is removed with just a
   warning message.
   Moreover we have to take care that (e.g.) \\<nl><nl> gives an error.
*/
case ' ':  /* In pcc mode, allow '\ ' to appease XOpen test suite */
           if (feature & FEATURE_PCC) break;
case '\t':
           cc_err(lex_err_backslash_blank);

/* If I see \<space> I generate a warning message. I then skip over any
 * following whitespace, up to one newline.  Thus the effect is that
 * \(space|tab)*(nl|) are ignored just as \nl.
 */
           do nextchar(); while ((curchar==' ') || (curchar=='\t'));
           if (curchar=='\n') nextchar();
case '\n': /* drop through.  note no nextchar() here so read_string()
              will give an error message. */
           return 0;

default:  /* pp.c removes control chars if !FEATURE_PCC */
          cc_ansi_rerr(lex_rerr_illegal_esc, (int)ch, (int)ch);
          break;      /* i.e. treat unknown escape "\Q" as "Q"            */
    }
    nextchar();
    /* note the next line translates all chars except \ooo and \xooo */
return_ch:
    (void)lex_string_insert(where, size, char_translation(ch));
    return 1;
}

static void read_string(int quote, AEop type, bool lengthwanted)
{   char *symstrp = lex_strptr, *val = lex_strptr;
#ifdef EXTENSION_COUNTED_STRINGS
    bool isMPWPascalString = NO;
    if (lengthwanted) {
        curchar = 0;
        isMPWPascalString = YES;
    } else
#endif
    nextchar();
    while (curchar!=quote)
    {   bool escaped = curchar == '\\';
        if ((curchar=='\n') || (curchar==PP_EOF))
        {   cc_err(lex_err_unterminated_string);
            break;   /* slightly untidy about nextchar() below, but OK */
        }
/* If I run off the end of this segment of lex_strbuf I allocate another  */
/* and copy the part-string into it, doubling the size if necessary.      */
        if (symstrp >= lex_strend)
        {   size_t n = symstrp - val, allocsize = LEX_STRBUFSIZE;
            while (n*2 > allocsize) allocsize *= 2;
            {   char *oval = val;
                val = (char *)BindAlloc(allocsize);
                if (n != 0) memcpy(val, oval, n);
                lex_strptr = symstrp = val + n;
                lex_strend = val + allocsize;
            }
        }
        if (escaped) nextchar();
#ifdef EXTENSION_COUNTED_STRINGS
        if (val == symstrp && escaped && curchar == 'p' &&
            (feature & FEATURE_ALLOWCOUNTEDSTRINGS))
        { /* "\p" at start of string */
            isMPWPascalString = YES;
            escaped = NO;  /* discard the '\' : 'p' will be planted in the */
                           /* string and overwritten at the end            */
        }
#endif
        if (lex_string_char(symstrp, (type == s_wstring ? sizeof_wchar : 1),
                            escaped))
            symstrp += (type == s_wstring ? sizeof_wchar : 1);
    }
    nextchar();
    if (quote == '"')
    {   curlex.a2.len = symstrp - val;
#ifdef EXTENSION_COUNTED_STRINGS
        if (isMPWPascalString)  /* treat wide strings rationally        */
            lex_string_insert(val, (type == s_wstring ? sizeof_wchar : 1),
                              curlex.a2.len-1);
#endif
        lex_strptr = &val[pad_to_word(curlex.a2.len)];   /* commit usage */
        curlex.a1.s = val;
        curlex.sym = type;
    }
    else
    {   int32 k = 0, n = symstrp - val;
/* note that for char constants we do not commit symstrp to lex_strptr */
/* The following line deals host-independently with single char        */
/* constants, at least if host char is 8 bits.                         */
        if (type == s_wstring)
        {   if (n != sizeof_wchar) cc_rerr(lex_rerr_not1wchar);
            if (n != 0) k = sizeof_wchar == 2 ? *(int16 *)val:
                                                *(int32 *)val;
        }
        else if (n == 1)
            k = (feature & FEATURE_SIGNED_CHAR) ? *(int8 *)val
                                                : *(unsigned8 *)val;
        else
        {   /* The effect of n>1 is implementation-defined */
            int32 i;
            if (n == 0)
                cc_rerr(lex_rerr_empty_char);
            else if (n > sizeof_int)
                cc_rerr(lex_rerr_overlong_char), n = sizeof_int;
            else
                cc_warn(lex_warn_multi_char);
            /* The following code follows pcc, and is host independent  */
            /* but assembles bytes 'backwards' on 68000-sex machines.   */
            /*   (It agrees with previous code on intel-sex hosts.)     */
            /* It is arguable that we should sign-extend the last char  */
            /* (only!) if FEATURE_SIGNED_CHAR (merges in n==1 case).    */
            for (i=0; i<n; i++)
                k = (unsigned32)k << 8 | ((unsigned8 *)val)[i];
            if (sizeof_int == 2) k = (int16)k;  /* normalise, eg for cmp.   */
        }
        curlex.a1.i = k;
        curlex.a2.flag = (type == s_wstring) ? NUM_WCHAR : NUM_CHAR;
                                      /* perhaps NUM_INT|NUM_LONG if n=4?   */
        curlex.sym = s_integer;       /* chars give produce int consts.     */
/* returning s_character/s_wcharacter would improve messages for int'a';    */
    }
}

static AEop next_basic_sym(void)
/* all of nextsym() except debug info */
{   unsigned32 charinfo;
    int32 savechar;
    FileLine startofsym_fl;

    if (endofsym_fl.filepos != -1 && !pp_inhashif)
        curlex.fl = endofsym_fl;

    if (curchar == NOTACHAR) nextchar();
    startofsym_fl = curlex.fl;
    for (;;)
    {   if (curchar == PP_EOF)  return (curlex.sym = s_eof);
        if (!isspace(curchar)) break;
        if (curchar == '\n' && pp_inhashif) return (curlex.sym = s_eol);
        nextchar();
    }
    /* Not just startofsym_fl = curlex.fl below, because we want the column
       to be that of the first whitespace character preceding the symbol on
       the same line, not that of the first character of the symbol (so that
       a debugger may see the whitespace preceding a statement as part of it).
     */
    if (startofsym_fl.l != curlex.fl.l)
    {   startofsym_fl = curlex.fl;
        startofsym_fl.column = 1;
    }
    switch ((charinfo = lexclass_(curchar)) & LEX_CHTYPE)
    {
default:    if (curchar == '#' || curchar == '\\')
                cc_err(lex_err_bad_hash, (int)curchar);
            else if (isprint(curchar))  /* can only happen if FEATURE_PCC */
                 cc_err(lex_err_bad_char, (long)curchar, (int)curchar);
            else cc_err(lex_err_bad_noprint_char, (int)curchar);
            nextchar();
            next_basic_sym();
            break;
case l_idstart:
        {   int k = 0;          /* number of characters read */
            do
            {   if (k < NAMEMAX)
                {
                    namebuf[k++] = curchar;
                }
                nextchar();
            } while (lexclass_(curchar) & l_idcont);
            namebuf[k] = 0;
/* Check if ANSI wide string/char -- illegal syntax in olde-C.             */
            if (k == 1 && namebuf[0] == 'L'
                       && (curchar == '"' || curchar == '\''))
                read_string(curchar, s_wstring, NO);
#ifdef EXTENSION_UNSIGNED_STRINGS
            else if (k == 1 && namebuf[0] == 'U' && curchar == '"')
                read_string(curchar, s_ustring, NO);
            else if (k == 1 && namebuf[0] == 'B' && curchar == '"')
                read_string(curchar, s_ustring, YES);
#endif
            else
            {   curlex.a1.sv = sym_lookup(namebuf, SYM_GLOBAL);
#ifdef CPLUSPLUS
#  define CPP_word 0            /* treat as per C keyword.      */
#else
#  define CPP_word 256          /* treat as warning for of id.  */
/* To prepare for C++, give a warning ONCE per file in ANSI mode when a */
/* C++ keyword is used as a C identifier.                               */
                if (symtype_(curlex.a1.sv) & CPP_word)
		{
		    symtype_(curlex.a1.sv) = s_identifier;

#ifdef TARGET_IS_TRAN
		    /*
		     * Addition to remove C++ warnings (for fussy IGM people).
		     * Tony 8/2/95.
		     */
		    if (!(feature & FEATURE_NO_CPLUSPLUS_WARNINGS))
#endif
			cc_ansi_warn(lex_warn_cplusplusid, curlex.a1.sv);
		}
#endif
/* There are no keywords during pp (including #if), following ANSI/Reiser. */
/* Consider adding a new s_ppidentifier which could aid lex.c.             */
#ifdef ALLOW_KEYWORDS_IN_HASHIF
                curlex.sym = symtype_(curlex.a1.sv);
#else
                curlex.sym = pp_inhashif ? s_identifier :
                                           symtype_(curlex.a1.sv);
#endif
#ifdef EXTENSION_VALOF
/* The following represents a rather nasty piece of context-sensitive      */
/* hackery - inside a valof block the word 'resultis' is recognized as a   */
/* keyword, but otherwise (as required in regular ANSI C) it is just       */
/* another ordinary symbol. I wonder if there is a better solution to this */
/* issue........... ??                                                     */
                if (inside_valof_block && curlex.sym == s_identifier &&
                    curlex.a1.sv == resultisword) curlex.sym = s_resultis;
#endif
            }
            break;
        }
case l_digit0:                  /* octal or hex or floating     */
            nextchar();         /* N.B. initial 0 not buffered  */
            if (is_x(curchar))
            {   nextchar();
                curlex.sym = read_number(16);   /* hex */
            }
            else
                curlex.sym = read_number(8);    /* octal or float */
            break;
case l_digit1:                  /* decimal int or floating */
            curlex.sym = read_number(10);
            break;
case l_dot:     nextchar();
#ifdef CPLUSPLUS
                if (curchar == '*')
                        curlex.sym = s_dotstar,
                        curchar = NOTACHAR;
                else
#endif
                if (isdigit(curchar))
                {   int32 flag, k = 0;
                    namebuf[k++] = '.';
                    namebuf[k++] = curchar;
                    nextchar();
                    flag = read_floating(k); /* change to use read_number()? */
                    curlex.sym = make_floating(namebuf, flag);
                }
                else
                {   int32 n = 1;
                    while (curchar=='.') n++, nextchar();
                    switch (n)
                    {   case 1: curlex.sym = s_dot; break;
                        default: cc_err(lex_err_ellipsis);
                                /* drop through */
                        case 3: curlex.sym = s_ellipsis; break;
                    }
                }
                break;
case l_noglue:
                curlex.sym = charinfo >> 8 & 255;
                curchar = NOTACHAR;
                break;
case l_eqglue:  nextchar();
                if (curchar == '=')
                    curlex.sym = charinfo >> 16,
                    curchar = NOTACHAR;
                else curlex.sym = charinfo >> 8 & 255;
                break;
case l_selfglue:
                savechar = curchar;
                nextchar();
                if (curchar == savechar)
                    curlex.sym = charinfo >> 16 & 255,
                    curchar = NOTACHAR;
                /* all selfgluer's '+' have a '+=' form except C++ ':=' */
                else if (curchar == '=' && (charinfo >> 24) != 0)
                    curlex.sym = charinfo >> 24,
                    curchar = NOTACHAR;
                else curlex.sym = charinfo >> 8 & 255;
                break;
case l_shifter: savechar = curchar;            /* very much like l_selfglue */
                nextchar();                    /* (32 bits is too few)      */
                if (curchar == '=')
                    curlex.sym = charinfo >> 24,
                    curchar = NOTACHAR;
                else if (curchar == savechar)
                {   curlex.sym = charinfo >> 16 & 255;
                    nextchar();
                    if (curchar == '=')
                        curlex.sym = assignop_(curlex.sym), /* >> to >>= etc */
                        curchar = NOTACHAR;
                }
                else curlex.sym = charinfo >> 8 & 255;
                break;
case l_minus:   nextchar();   /* l_selfglue but for "->", (C++)"->*". */
                if (curchar=='-')
                    curlex.sym = s_minusminus,
                    curchar = NOTACHAR;
                else if (curchar=='>')
                  { nextchar();
#ifdef CPLUSPLUS
                    if (curchar=='*')
                        curlex.sym = s_arrowstar,
                        curchar = NOTACHAR;
                    else
#endif
                        curlex.sym = s_arrow;
                  }
                else
                  {
                    if (curchar=='=')
                        curlex.sym = s_minusequal,
                        curchar = NOTACHAR;
                    else curlex.sym = s_minus;
                  }
                break;
case l_quote1:
case l_quote2:  read_string(curchar, s_string, NO);
                break;
    }
    /* recognise whitespace (but NOT newlines) in += etc    */
    /* as a sop to olde-style (K&R) C.                      */
    if (can_have_becomes(curlex.sym))
    {   if (curchar == NOTACHAR) nextchar();
        while (curchar == ' ' || curchar == '\t') nextchar();
/* Beware the effect of pp.c if you ever want to extend this over a '\n'. */
        if (curchar == '=')
        {   curlex.sym = and_becomes(curlex.sym);
            curchar = NOTACHAR;
            cc_ansi_rerr(lex_err_illegal_whitespace, curlex.sym);
        }
    }
    endofsym_fl = curlex.fl;     /* save for next call */
    curlex.fl = startofsym_fl;   /* reset to start of symbol */
    return(curlex.sym);
}


/* exported routines... */

#ifdef CPLUSPLUS
/* C++ requires function definitions within class definitions to be     */
/* lexed, but not parsed (typedefs) nor typechecked [ES, p178].         */
/* We save them as as token streams until we are ready -- the end       */
/* is observable by counting {}'s.                                      */
/* Note there is a complication about reading such a saved token stream */
/* since such a fn body may contain embedded fns within yet other       */
/* (local) class definitions.                                           */
/* Globalise everything to be buffered for now (very little to do).     */
typedef struct SymBuf {
  struct SymBuf *prev; SymInfo prevsym;         /* for save/restore     */
  SymInfo *buf;
  int size, pos;
} SymBuf;
static int lexbuf_max;
static SymBuf *lexbuf_vec, *nextsym_lookaside;
static const SymBuf lexbuf_empty =
   { 0, { s_eof }, (SymInfo *)DUFF_ADDR, 0, /*pos*/ -1 };
#define INIT_NLEXBUFS 8         /* initially max 8 member fn defs.      */
#define INIT_LEXBUFSIZE 32      /* initially max 32 tokens per def.     */

static int lex_findsavebuf()
{   int i;
    for (i = 0; i<lexbuf_max; i++)
        if (lexbuf_vec[i].pos < 0) return i;
    {   int nmax = (lexbuf_max == 0 ? INIT_NLEXBUFS : 2*lexbuf_max);
        SymBuf *nvec = (SymBuf *)GlobAlloc(SU_Other,
                                     (int32)nmax * sizeof(SymBuf));
        memcpy(nvec, lexbuf_vec, lexbuf_max * sizeof(SymBuf));
        for (i = lexbuf_max; i < nmax; i++) nvec[i] = lexbuf_empty;
        lexbuf_max = nmax, lexbuf_vec = nvec;
        return lex_findsavebuf();       /* retry */
    }
}

int lex_savebody()
{   int h = lex_findsavebuf();
    SymBuf *p = &lexbuf_vec[h];
    int k = 0, braces = 0;
    p->pos = 0;                 /* inuse + ready for reader.        */
/* save all the tokens in p->buf, followed by a s_eof token.        */
    for (;;)
    {   if (k >= p->size)
        {   int nsize = (p->size == 0 ? INIT_LEXBUFSIZE : p->size*2);
            SymInfo *nbuf = (SymInfo *)GlobAlloc(SU_Other,
                                           (int32)nsize * sizeof(SymInfo));
            memcpy(nbuf, p->buf, p->size * sizeof(SymInfo));
            p->size = nsize, p->buf = nbuf;
        }
        lex_beware_reinit();                    /* globalise curlex     */
        p->buf[k++] = curlex;
        switch (curlex.sym)
        {   case s_lbrace: braces++;
            default:       nextsym();
                           break;
            case s_rbrace: if (--braces == 0) curlex.sym = s_eof;
                           else nextsym();
                           break;
            case s_eof:    return h;
        }
    }
}

void lex_openbody(int h)
{   lexbuf_vec[h].prev = nextsym_lookaside;
    lex_beware_reinit();                        /* globalise curlex     */
    lexbuf_vec[h].prevsym = curlex;
    nextsym_lookaside = &lexbuf_vec[h];
    nextsym();                  /* currently always '{'!                */
}

void lex_closebody()
{   nextsym_lookaside -> pos = -1;
    curlex = nextsym_lookaside -> prevsym;
    nextsym_lookaside = nextsym_lookaside -> prev;
}

static void lex_getbodysym()
{   curlex = nextsym_lookaside->buf[nextsym_lookaside->pos];
    if (curlex.sym != s_eof) nextsym_lookaside->pos++;
}

/* check nextsym_for_hashif uses consistent!.                           */
#endif

void ungetsym(void)
{   nextlex = curlex;    /* Surprisingly, this copying is as efficient */
    curlex = prevlex;    /* as pointer juggling - perhaps more so.     */
}

AEop nextsym(void)
/* sets curlex.sym to next symbol */
{
    errs_on_this_sym = 0;
    prevlex = curlex;
    if (nextlex.sym != s_nothing)
    {   curlex = nextlex;
        nextlex.sym = s_nothing;
    }
    else
#ifdef CPLUSPLUS
    if (nextsym_lookaside && !pp_inhashif)
        lex_getbodysym();
    else
#endif
        next_basic_sym();
    if ((debugging(DEBUG_LEX))!=0)
        {   cc_msg("<nextsym: sym %ld=%s %lx",
                            (long)curlex.sym,
                            sym_name_table[curlex.sym],
                            (long)curlex.a1.i);
            if (curlex.sym==s_identifier)
                cc_msg(" $r", curlex.a1.sv);
            cc_msg(">\n");
        }
    return(curlex.sym);
}

AEop nextsym_for_hashif(void)
{   curchar = NOTACHAR;   /* could only have been '\n' or NOTACHAR */
    return nextsym();
}

/* exported initialiser lex_init() is language dependent ... */

static void setuplexclass1(char *s, unsigned32 l)
{   unsigned char ch;
    while ((ch = *s++) != 0) lexclass_(ch) = l | l_idcont;
}

void lex_init()         /* C version  */
{
    int32 i;
    static struct { unsigned32 lc; char name[2], name1[3], name2[3]; }
      sp[26] = {
        { mklc(l_noglue, s_lpar, 0, 0),      "(" },
        { mklc(l_noglue, s_rpar, 0, 0),      ")" },
        { mklc(l_noglue, s_lbracket, 0, 0),  "[" },
        { mklc(l_noglue, s_rbracket, 0, 0),  "]" },
        { mklc(l_noglue, s_lbrace, 0, 0),    "{" },
        { mklc(l_noglue, s_rbrace, 0, 0),    "}" },
        { mklc(l_noglue, s_semicolon, 0, 0), ";" },
        { mklc(l_noglue, s_comma, 0, 0),     "," },
        { mklc(l_noglue, s_bitnot, 0, 0),    "~" },
        { mklc(l_noglue, s_cond, 0, 0),      "?" },
        /* now the assignable nongluers */
        { mklc(l_eqglue, s_times, s_timesequal, 0), "*", "*=" },
        { mklc(l_eqglue, s_div, s_divequal, 0),     "/", "/=" },
        { mklc(l_eqglue, s_rem, s_remequal, 0),     "%", "%=" },
        { mklc(l_eqglue, s_xor, s_xorequal, 0),     "^", "^=" },
        { mklc(l_eqglue, s_boolnot, s_notequal, 0), "!", "!=" },
        /* now the self-gluers (only the single form is assignable) */
#ifdef CPLUSPLUS
        { mklc(l_selfglue, s_colon, s_coloncolon, 0),  ":", "::", "" },
#else
        { mklc(l_noglue, s_colon, 0, 0),     ":" },
#endif
        { mklc(l_selfglue, s_assign, s_equalequal, s_equalequal),
                                                    "=", "==", "==" },
        { mklc(l_selfglue, s_and,  s_andand, s_andequal),
                                                    "&", "&&", "&=" },
        { mklc(l_selfglue, s_or,   s_oror, s_orequal),
                                                    "|", "||", "|=" },
        { mklc(l_selfglue, s_plus, s_plusplus, s_plusequal),
                                                    "+", "++", "+=" },
        /* shifts/relops - both the single and double form is assignable */
        { mklc(l_shifter, s_less, s_leftshift, s_lessequal),
                                                         "<", "<<", "<=" },
        { mklc(l_shifter, s_greater, s_rightshift, s_greaterequal),
                                                         ">", ">>", ">=" },
        /* magic chars - 1 per class */
        { mklc(l_quote1, 0, 0, 0),       "'" },
        { mklc(l_quote2, 0, 0, 0),       "\"" },
        { mklc(l_minus,  s_minus, s_minusminus, s_minusequal),
                                                    "-", "--", "-=" },
        { mklc(l_dot,    s_dot, 0, 0),   "." }
    };
    static struct { char *name; AEop sym; } ns[] = {
/* ANSI keywords which also were in PCC.                                */
        { "auto",     s_auto },
        { "break",    s_break },
        { "case",     s_case },
        { "char",     s_char },
        { "continue", s_continue },
        { "default",  s_default },
        { "do",       s_do },
        { "double",   s_double },
        { "else",     s_else },
        { "enum",     s_enum },
        { "extern",   s_extern },
        { "float",    s_float },
        { "for",      s_for },
        { "goto",     s_goto },
        { "if",       s_if },
        { "int",      s_int },
        { "long",     s_long },
        { "register", s_register },
        { "return",   s_return },
        { "short" ,   s_short },
        { "sizeof",   s_sizeof },
        { "static",   s_static },
        { "struct",   s_struct },
        { "switch",   s_switch },
        { "typedef",  s_typedef },
        { "union",    s_union },
        { "unsigned", s_unsigned },
        { "void",     s_void },
        { "while",    s_while },
/* C extension:                                                         */
        { "__int64", s_longlong},
/* specials to help the compiler/library.                               */
        { "___toplevel",s_toplevel },
        { "___type",  s_typestartsym },
        { "___typeof",s_typeof },
        { "___weak",  s_weak },
        { "__pure",    s_pure },
        { "__value_in_regs", s_structreg },
        { "__swi",    s_swi },
        { "__swi_indirect", s_swi_i },
        { "__irq", s_irq },
        { "__global_reg", s_globalreg },
        { "__global_freg", s_globalfreg },

#define N_PCC_FUSSY_KEYWORDS 41 /* everything above!  Bad style!!!      */
/* The following are ANSI C keywords, but old PCC (unix) sources        */
/* (compile with -pcc -fussy) may treat them as identifiers!            */
        { "const",    s_const },
        { "signed",   s_signed },
        { "volatile", s_volatile },
/* C++ only keywords (redeclared as s_identifier on use in C mode)      */
        { "asm",      CPP_word|s_asm },
        { "catch",    CPP_word|s_catch },
        { "class",    CPP_word|s_class },
        { "delete",   CPP_word|s_delete },
        { "friend",   CPP_word|s_friend },
        { "inline",   CPP_word|s_inline },
        { "new",      CPP_word|s_new },
        { "operator", CPP_word|s_operator },
        { "private",  CPP_word|s_private },
        { "protected",CPP_word|s_protected },
        { "public",   CPP_word|s_public },
        { "template", CPP_word|s_template },
        { "this",     CPP_word|s_this },
        { "throw",    CPP_word|s_throw },
        { "try",      CPP_word|s_try },
        { "virtual",  CPP_word|s_virtual },
#ifdef EXTENSION_VALOF
        { "resultis", s_resultis },
#endif
    };
    /* although lexclass[] is notionally constant, C's initialisation
       facilities do not conveniently enable us to initialise it... */
    for (i = 0; i <= 255; i++) lexclass[i] = l_illegal;
    setuplexclass1("ABCDEFGHIJKLMNOPQRSTUVWXYZ_", l_idstart);
    setuplexclass1("abcdefghijklmnopqrstuvwxyz",  l_idstart);
    setuplexclass1("123456789", l_digit1);
    setuplexclass1("0",         l_digit0);
    /*
     * The following line is for the sake of SUN NeWS !
     *   This is because SUN NeWS allows the use of '$'
     *   in variable names (yuk !)
     */
    if (feature & (FEATURE_PCC|FEATURE_LIMITED_PCC))
        setuplexclass1("$", l_idstart);

    {   unsigned int u, n = sizeof(ns)/sizeof(ns[0]);
        if ((feature & (FEATURE_PCC|FEATURE_FUSSY)) ==
                       (FEATURE_PCC|FEATURE_FUSSY))  n = N_PCC_FUSSY_KEYWORDS;
#undef N_PCC_FUSSY_KEYWORDS
        for (u = 0; u < n; ++u)
        {   char *name = ns[u].name; int32 sym = ns[u].sym;
            sym_insert(name, sym);
            sym_name_table[sym & ~CPP_word] = name;
        }
    }
    {   unsigned int u;
        for (u = 0; u < sizeof(sp)/sizeof(sp[0]); u++)
        {   char *name = sp[u].name; unsigned32 lc = sp[u].lc;
            AEop s;
            lexclass_(*name) = lc;
            if ((s = (lc >>  8) & 255) != 0) sym_name_table[s] = name;
            if ((s = (lc >> 16) & 255) != 0) sym_name_table[s] = sp[u].name1;
            if ((s = (lc >> 24) & 255) != 0) sym_name_table[s] = sp[u].name2;
        }
    }
#ifdef EXTENSION_VALOF
/* 'resultis' is a funny (experimental) syntax extension */
    resultisword = sym_insert_id("resultis");
#endif
    curchar = NOTACHAR; /* Kill lookahead  */
    curlex.sym = s_nothing;
    errs_on_this_sym = 0;
    lex_strend = lex_strptr = (char *)DUFF_ADDR;
    endofsym_fl.filepos = -1;  /* mark as invalid */
#ifdef CPLUSPLUS
    lexbuf_max = 0; lexbuf_vec = (SymBuf *)DUFF_ADDR; nextsym_lookaside = 0;
#endif
}

void lex_beware_reinit()
{   /* this routine patches the fact that an (illegal) C program of the */
    /* form 'int a = 1 "abc"' or 'f(){} "abc"' needs to be able to      */
    /* print out "abc" in an error message even though Local store will */
    /* have been clobbered by alloc_reinit().  Move it to Global store! */
    if (isstring_(curlex.sym))
    {   char *oval = curlex.a1.s;
        curlex.a1.s = (char *)GlobAlloc(SU_Other, pad_to_word(curlex.a2.len));
        memcpy(curlex.a1.s, oval, (size_t)curlex.a2.len);
    }
}

void lex_reinit()
{
    lex_strend = lex_strptr = (char *)DUFF_ADDR;   /* better to use ""? */
    nextlex.sym = s_nothing;
}

/* End of lex.c */
