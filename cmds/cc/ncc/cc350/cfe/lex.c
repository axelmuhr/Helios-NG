/*
 * C compiler file lex.c, version 55
 * Copyright (C) Codemist Ltd., 1988
 * Copyright (C) Acorn Computers Ltd., 1988
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1992/03/23 15:10:26 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char lex_version[];
char lex_version[] = "\ncfe/lex.c $Revision: 1.1 $ 55\n";
#endif

/*
 *   AM Nov 89: treat ANSI pp-number conversion errors properly.
 *   AM Jun 89: add 16 bit int support
 *   13.x.88   Pascal split off
 *   AM 23-10-86: Fix bug in string wrap round segments, add cc_rerr() calls.
 *   AM 28-7-86: Recognise += in lexer without whitespace, glue (nextsym)
 *               '+ =' with space/tab white space.  Problematic here
 *               to recognise '+\n='.  A #if would clobber things.
 */

#ifdef __STDC__
#  include <stddef.h>
#  include <string.h>
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

static unsigned32 lexclass[1+(unsigned char)(-1)];
                                    /* Character table, yielding:
                                       glue-ishness (low 8 bits),
                                       corresponding symbol (next 8 bits),
                                       doubled symbol (next 8 bits)
                                       relop equal symbol (top 8 bits)     */

/* 'curlex.a2.flag' values - only exported for make_integer (q.v.) */
/* the values are chosen for easy punning into sem.c types  */

#define NUM_FLOAT  bitoftype_(s_double)
#define NUM_INT    bitoftype_(s_int)
#define NUM_LONG   bitoftype_(s_long)
#define NUM_SHORT  bitoftype_(s_short)
#define NUM_UNSIGN bitoftype_(s_unsigned)

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
        {   if ((value & (unsigned long)0xe0000000)!=0) overflow=1;
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
        {   if ((value & (unsigned long)0xf0000000)!=0) overflow=1;
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
#if sizeof_int == 2	/* TARGET_HAS_16_BIT_INT        */
    {    if (value & (unsigned long)0xffff0000 || flag & NUM_LONG)
        {   flag |= NUM_LONG;
            if (value & (unsigned long)0x80000000 && !(flag & NUM_UNSIGN))
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
#else                                /* TARGET_HAS_32_BIT_INT        */
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
#endif /* sizeof_int == 2 */
    curlex.a1.i = value;
    curlex.a2.flag = flag;
    return s_integer;
}

/* The following routine does checks for 'constraint violation' when    */
/* coverting a pp_number to a token, caller has already checked for     */
/* illegal uses of 'e+' in fp or hex numbers.                           */
static void lex_check_pp_number()
{   if (curchar == '.' ||
        curchar != EOF && lexclass[(unsigned char)curchar] & l_idcont)
            cc_ansi_rerr(lex_rerr_pp_number);
}

/* reading routines */

static int nextchar(void)
/* I may also want to buffer some recently read characters so that I   */
/* can display them when I notice an error.                            */
{
    if (curchar==EOF) return curchar;
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
    {   case 4: return *(unsigned32 *)(int)where = (unsigned32)what;
        case 2: return *(unsigned16 *)(int)where = (unsigned16)what;
        case 1: return *(unsigned8 *)      where = (unsigned8)what;
        default: syserr(syserr_lex_string); return what;
    }
}

static bool lex_string_char(char *where, int size)
/* read a possibly escaped (with backslash (\)) character for a           */
/* character or string literal (possibly wide).  If read succeeds         */
/* result is placed in where (if char) and (<size> *)where (if wide char).*/
/* Caller aligns if wide.  Result is 1 if succeeds, 0 if null escape read */
{
    int ch = curchar;
    if (ch == '\\') switch (ch = nextchar())
    {
case 'a':       ch = BELL;      break;  /* attn (otherwise known as bell) */
case 'b':       ch = '\b';      break;  /* backspace                      */
case 'f':       ch = CHAR_FF;   break;  /* form feed                      */
case 'n':       ch = '\n';      break;  /* newline                        */
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

static void read_string(int quote, bool wide)
{   char *symstrp = lex_strptr, *val = lex_strptr;
    nextchar();
    while (curchar!=quote)
    {   if ((curchar=='\n') || (curchar==EOF))
        {   cc_err(lex_err_unterminated_string);
            break;   /* slightly untidy about nextchar() below, but OK */
        }
/* If I run off the end of this segment of lex_strbuf I allocate another  */
/* and copy the part-string into it, doubling the size if necessary.      */
        if (symstrp >= lex_strend)
        {   int32 n = symstrp - val, allocsize = LEX_STRBUFSIZE;
            while (n*2 > allocsize) allocsize *= 2;
            {   char *oval = val;
                val = BindAlloc(allocsize);
                memcpy(val, oval, symstrp-oval);
                lex_strptr = symstrp = val + (symstrp-oval);
                lex_strend = val + allocsize;
            }
        }
        if (lex_string_char(symstrp, wide ? sizeof_wchar : 1))
            symstrp += wide ? sizeof_wchar : 1;
    }
    nextchar();
    if (quote == '"')
    {   curlex.a2.len = symstrp - val;
        lex_strptr = &val[pad_to_word(curlex.a2.len)];   /* commit usage */
        curlex.a1.s = val;
        curlex.sym = wide ? s_wstring : s_string;
    }
    else
    {   int32 k = 0, n = symstrp - val;
/* note that for char constants we do not commit symstrp to lex_strptr */
/* The following line deals host-independently with single char        */
/* constants, at least if host char is 8 bits.                         */
        if (wide)
        {   if (n != sizeof_wchar) cc_rerr(lex_rerr_not1wchar);
            if (n != 0) k = sizeof_wchar == 2 ? *(int16 *)(int)val:
                                                *(int32 *)(int)val;
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
#if sizeof_int == 2
	    k = (int16)k;  /* normalise, eg for cmp.   */
#endif
        }
        curlex.a1.i = k;
        curlex.a2.flag = wide ? NUM_WCHAR : NUM_INT;
                                      /* perhaps NUM_INT|NUM_LONG if n=4?   */
        curlex.sym = s_integer;       /* chars give produce int consts.     */
                     /* maybe one day s_character/s_wcharacter.             */
    }
}

static AEop next_basic_sym(void)
/* all of nextsym() except debug info */
{   unsigned32 charinfo; int32 savechar;

    if (curchar == NOTACHAR) nextchar();
    for (;;)
    {   if (curchar == EOF)  return (curlex.sym = s_eof);
        if (!isspace(curchar)) break;
        if (curchar == '\n' && pp_inhashif) return (curlex.sym = s_eol);
        nextchar();
    }
    switch ((charinfo = lexclass[(unsigned char)curchar]) & LEX_CHTYPE)
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
            } while (lexclass[(unsigned char)curchar] & l_idcont);
            namebuf[k] = 0;
/* Check if ANSI wide string/char -- illegal syntax in olde-C.             */
            if (k == 1 && namebuf[0] == 'L' &&
                 (curchar == '"' || curchar == '\''))
            {
                read_string(curchar,1);
            }
            else
            {   curlex.a1.sv = sym_lookup(namebuf, SYM_GLOBAL);
/* There are no keywords during pp (including #if), following ANSI/Reiser. */
/* Consider adding a new s_ppidentifier which could aid lex.c.             */
                curlex.sym = pp_inhashif ? s_identifier :
                                           symtype_(curlex.a1.sv);
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
                else
                  {
                    if (curchar == '=')
                        curlex.sym = charinfo >> 24,
                        curchar = NOTACHAR;
                    else curlex.sym = charinfo >> 8 & 255;
                  }
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
case l_minus:   nextchar();            /* very like l_selfglue but for "->" */
                if (curchar=='-')
                    curlex.sym = s_minusminus,
                    curchar = NOTACHAR;
                else if (curchar=='>')
                    curlex.sym = s_arrow,
                    curchar = NOTACHAR;
                else
                  {
                    if (curchar=='=')
                        curlex.sym = s_minusequal,
                        curchar = NOTACHAR;
                    else curlex.sym = s_minus;
                  }
                break;
case l_quote1:
case l_quote2:  read_string(curchar, 0);
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
    return(curlex.sym);
}


/* exported routines... */

AEop nextsym(void)
/* sets curlex.sym to next symbol */
{
    errs_on_this_sym = 0;
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
    while ((ch = *s++) != 0) lexclass[ch] = l | l_idcont;
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
        { mklc(l_noglue, s_colon, 0, 0),     ":" },
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
#ifdef EXTENSION_VALOF
        { "resultis", s_resultis },
#endif
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
        { "___toplevel",s_toplevel },
        { "___typeof",s_typeof },
/* The following are used only in ANSI C - disabled by -cc -fussy */
#define N_ANSI_ONLY_KWDS  4
        { "const",    s_const },
        { "signed",   s_signed },
        { "volatile", s_volatile },
        { "___type",  s_typestartsym },
    };
    /* although lexclass[] is notionally constant, C's initialisation
       facilities do not conveniently enable us to initialise it... */
    for (i = 0; i <= (unsigned char)(-1); i++) lexclass[i] = l_illegal;
    setuplexclass1("ABCDEFGHIJKLMNOPQRSTUVWXYZ_", l_idstart);
    setuplexclass1("abcdefghijklmnopqrstuvwxyz",  l_idstart);
    setuplexclass1("123456789", l_digit1);
    setuplexclass1("0",         l_digit0);
    /*
     * The following line is for the sake of SUN NeWS !
     *   This is because SUN NeWS allows the use of '$'
     *   in variable names (yuk !)
     */
    if (feature & FEATURE_PCC) setuplexclass1("$", l_idstart);

    /* common code for both languages */
    {   unsigned int u, n = sizeof(ns)/sizeof(ns[0]);
        if ((feature & (FEATURE_PCC|FEATURE_FUSSY)) ==
                       (FEATURE_PCC|FEATURE_FUSSY))  n -= N_ANSI_ONLY_KWDS;
#undef N_ANSI_ONLY_KWDS
        for (u = 0; u < n; ++u)
        {   char *name = ns[u].name; int32 sym = ns[u].sym;
            sym_insert(name, sym);
            sym_name_table[sym] = name;
        }
    }
    {   unsigned int u;
        for (u = 0; u < sizeof(sp)/sizeof(sp[0]); u++)
        {   char *name = sp[u].name; unsigned32 lc = sp[u].lc;
            AEop s;
            lexclass[(unsigned char)*name] = lc;
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
    lex_strend = lex_strptr = DUFF_ADDR;  /* OK with Feb90+ errprintf.  */
}

void lex_beware_reinit()
{   /* this routine patches the fact that an (illegal) C program of the */
    /* form 'int a = 1 "abc"' or 'f(){} "abc"' needs to be able to      */
    /* print out "abc" in an error message even though Local store will */
    /* have been clobbered by alloc_reinit().  Move it to Global store! */
    if (curlex.sym == s_string || curlex.sym == s_wstring)
    {   char *oval = curlex.a1.s;
        curlex.a1.s = GlobAlloc(SU_Other, pad_to_word(curlex.a2.len));
        memcpy(curlex.a1.s, oval, (size_t)curlex.a2.len);
    }
}

void lex_reinit()
{
    lex_strend = lex_strptr = DUFF_ADDR;
}

/* End of lex.c */
