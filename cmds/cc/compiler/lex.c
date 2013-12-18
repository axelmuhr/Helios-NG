
/* C compiler file lex.c: Copyright (C) A.C.Norman and A.Mycroft */
/* $Id: lex.c,v 1.3 1991/03/08 10:59:20 nick Exp $ */

/* version 0.39
 *   AM 29-may-87: redo string allocation.
 *   ACN 10-12-86: start at support of Pascal option.
 *   AM 23-10-86: Fix bug in string wrap round segments, add cc_rerr() calls.
 *   AM 28-7-86: Recognise += in lexer without whitespace.
 *               Note that #if directives in the middle of a K&R style
 *               + = token pair could have done very nasty things.
 *               Hence removed (non ANSI) and simple hack inserted.
 *   should 'curlex.a2.len' replace namecount soon?
 */

#include <stddef.h>
#include <string.h>
#include <limits.h>
#include "cchdr.h"
#include "util.h"
#include "AEops.h"

/* exports: nextsym(), curlex, gensymval(), nextsym_for_hashif(),
            lex_init(),            and some more! */

/* utilities */
static int intofxdigit(c)
int c;
{   return(isdigit(c) ? (c) - '0' :
           islower(c) ? (c) - 'a' + 10 :
           isupper(c) ? (c) - 'A' + 10 :
           -1);
}

#define lex_err cc_err
#define lex_rerr cc_rerr
#define lex_warn cc_warn
#define NOTACHAR        0

#define LEX_STRBUFSIZE  256  /* (multiple of 4) initial size lex_strbuf */
static char *lex_strptr;     /* next free char in lex_strbuf            */
static char *lex_strend;     /* one beyond end of lex_strbuf            */

static int     curchar;        /* The look-ahead character     */
#ifdef PASCAL
static int pending_dot;        /* Used to deal with 1..2 vs. 1.2 */
#endif
/* The next 3 symbols are notionally a single tuple value.  Export to misc */
SymInfo curlex;         /* Current token and aux. info.        */
static int     linenum;        /* Number of current input line */
static int     colnum;         /* Input column                 */
static int     gensymnum;      /* For generating unique syms   */
#ifndef NO_VALOF_BLOCKS
bool inside_valof_block;
static Symstr *resultisword;
#endif
FloatCon  *fc_two_31;   /* floating point constant 2^31 */
FloatCon  *fc_zero;     /* floating point constant 0.0  */

/* the +4 in the next line is slop so we do not check on dec. pt. etc  */
static char namebuf[NAMEMAX+4];     /* Buffer for reading identifiers      */
static int  namecount;              /* Number of chars in above            */
static int  namehash;               /* Current hash value of namebuf       */

static Symstr  *(*hashvec)[LEX_HASHSIZE];  /* Symbol table buckets         */
char *sym_name_table[s_NUMSYMS];    /* translation back to strings         */

static int lexclass[UCHAR_MAX+1];   /* Character table, yielding:
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

#define l_illegal   0
#define l_noglue    1
#define l_selfglue  2
#define l_idstart   3
#define l_digit0    4
#define l_digit1    5
#define l_quote1    6
#define l_quote2    7
#define l_dot       8
#define l_shifter   9
#define l_eqglue    10
#define l_minus     11
#define l_lpar      12
#define l_less      13
#define LEX_CHTYPE  15  /* mask */
#define l_idcont    16  /* orable */
#define mklc(l,t1,t2,t3) ((l) | ((t1)<<8) | ((t2) << 16) | ((t3) << 24))

static void setuplexclass1(s,l)
char *s;
int l;
{   unsigned char ch;
    while ((ch = *s++) != 0) lexclass[ch] = l | l_idcont;
}

/* lex 'semantic' routines... */

static Symstr *lookup(name,lvptr,glo)
char *name;
Symstr **lvptr;
int glo;
{   int wsize;
    Symstr *next;
    while ((next = *lvptr) != NULL)
    {   if (strcmp(_symname(next), name) == 0) return(next);
        lvptr = &_symchain(next);
    }
    wsize = offsetof(Symstr, symname) + padstrlen(strlen(name));
    next = glo ? (stuse_sym+=wsize,GlobAlloc(wsize)) : BindAlloc(wsize);
    memclr(next, wsize);
    *lvptr = next;
    _symtype(next) = s_identifier;
    strcpy(_symname(next), name);
    return(next);
}


static int lookupname(s)
char *s;
{
    Symstr *p = lookup(s, &(*hashvec)[namehash % LEX_HASHSIZE], 1);
    curlex.a1.sv = p;
    return(_symtype(p));
}


static void update_hash(ch)
char ch;
{   int hashtemp;
    /* This is my favourite hash function at */
    /* present - it cycles a 31-bit shift    */
    /* register with maximum period.         */
    hashtemp = (namehash << 7);
    namehash = ((namehash >> 25) ^
                (hashtemp >> 1) ^
                (hashtemp >> 4) ^
                ch) & 0x7fffffff;
}

Symstr *sym_insert(name,type)
char *name;
AEop type;
{
    char ch, *ptr = name;
    Symstr *p;
    namehash = 1;           /* start of hash total   */
    while((ch = *ptr++) != 0) update_hash(ch);
    p = lookup(name, &(*hashvec)[namehash % LEX_HASHSIZE], 1);
    _symtype(p) = type;
    return(p);
}

/* 'real' in the following represents generic float/double/long double */

/* real_of_string() uses GLOBAL store for its value.  Is this reasonable? */
/* special case flag==0 for double_to_double only */
FloatCon *real_of_string(s,flag)
char *s;
int flag;
{   int wsize = offsetof(FloatCon,floatstr) + padstrlen(strlen(s));
    FloatCon *x = (stuse_const+=wsize, GlobAlloc(wsize));
    /* real_of_string is now also used to initialise fc_two_32. */
    x->h0 = s_floatcon;
    strcpy(x->floatstr, s);
    if (flag)
    {   x->floatlen = flag;              /* the bitoftype_ / curlex.a2.flag pun!!! */
        fltrep_stod(s, &x->floatbin.db); /* double = long double for us...  */
        if (flag & NUM_SHORT)            /* so we only need to narrow once  */
            fltrep_narrow_round(&x->floatbin.db, &x->floatbin.fb);
    }
    return x;
}

/* the next two routines do not really belong here, but they do manipulate
   the same structure as make_floating */

FloatCon *real_to_real(fc,m)
FloatCon *fc;
SET_BITMAP m;
{   FloatCon *x = real_of_string(fc->floatstr, 0);
    /* N.B. the last line just copies the floating point number text
       and so this text does not reflect narrowing and re-widening.
       The string is only provided for the ASM output not AOF, and so
       the inaccuracy in it is not critical: the associated binary FP
       value will be kept exactly as it should be and that is what
       matters.  */
    x->floatlen = m;
    memcpy(&x->floatbin, &fc->floatbin, sizeof(x->floatbin));
    /* long double = double in this implementation so widen/narrow is easy: */
    if ((m & NUM_SHORT) && !(fc->floatlen & NUM_SHORT))
        fltrep_narrow(&fc->floatbin.db, &x->floatbin.fb);
    else if (!(m & NUM_SHORT) && (fc->floatlen & NUM_SHORT))
        fltrep_widen(&fc->floatbin.fb, &x->floatbin.db);
    return x;
}

FloatCon *int_to_real(n,u,m)
int n;
int u;
SET_BITMAP m;
{   char s[20];
    /* there must be better ways - e.g. fltrep_itod in fltrep.c ??       */
    /* The use of sprintf/real_of_string ensures that the floatcon that  */
    /* is generated has a sensible string attached to it so leave this   */
    /* code alone - it works quite well enough.                          */
    if (u) sprintf(s, "%u.0", n);
    else sprintf(s, "%d.0", n);
    return real_of_string(s,m);    
}

/* make_floating uses GLOBAL store for its value.  Is this reasonable? */
static AEop make_floating(s,flag)
char *s;
int flag;
{   FloatCon *x = real_of_string(s,flag);
    curlex.a1.fc = x;
    return x->h0;
}

/* make_integer does not use GLOBAL or LOCAL store.  It could use
   the former (but this could be wasteful), or LOCAL, but beware placement
   of alloc_reinit() in driver.c and lookahead tokens.
   Currently it just assumes that syn.c will look at curlex.a2.flag.
*/
static AEop make_integer(radix,flag)
int radix;
int flag;
/* Numbers that overflow lead to a warning message. Note that octal    */
/* & hex numbers only overflow if an unsigned value could not be used  */
/* to store the number, whereas decimal values can only span the range */
/* available to signed numbers.                                        */
/* I have allowed the digits '8' and '9' to build up even in           */
/* octal numbers - this needs checking for here.                       */
/* On an inplementation where (int) == (long) there is no need to      */
/* inspect NUM_LONG - all integers are read as 32-bit values.          */
/* BEWARE: possible two's complement dependency.                       */
{
    int overflow=0, badoct=0;
    unsigned int value=0;
    char c, *cp = namebuf;
    switch(radix)
    {
case 8: while((c= (*cp++))!=0)
        {   if ((value & 0xe0000000)!=0) overflow=1;
            if (c>='8') badoct=1;
            value=(value<<3) | intofdigit(c);
        }
        if (badoct) lex_rerr("Digit 8 or 9 found in octal number");
        break;
case 10:while((c=(*cp++))!=0)
        {   int bigish = 0;
#ifdef COMPILING_ON_ST
	    /* Lattice C cannot calculate the division (signed arithmetic
	     * being used where it shouldn't), so we replace it by the
             * correct constant (I think).
	     */
            if (value >= (unsigned)0x19999999) bigish = 1;
            if (value > (unsigned)0x19999999) overflow=1;
#else
            if (value >= ((unsigned)0xffffffff)/10) bigish = 1;
            if (value > ((unsigned)0xffffffff)/10) overflow=1;
#endif
            value = value*10 + intofdigit(c);
            if (bigish && value < 10) overflow=1;
        }
        break;
case 16:while((c=(*cp++))!=0)
        {   if ((value & 0xf0000000)!=0) overflow=1;
            value = (value<<4) | intofxdigit(c);
        }
        break;
    }
    if (overflow)
        lex_err("Number %s too large for 32-bit implementation", namebuf);
    if ((int)value < 0  && !(flag & NUM_UNSIGN))
    {   flag |= NUM_UNSIGN;    /* add 'U' - see ANSI spec */
        if (radix == 10)       /* but for base 10 add 'L' too */
        {   flag |= NUM_LONG;
            if (!overflow)     /* only 1 error message */
                lex_warn("%s treated as %sul in 32-bit implementation",
                         namebuf, namebuf);
        }
    }        
    curlex.a1.i = value;
    curlex.a2.flag = flag;
    return s_integer;
}


/* reading routines */

static int nextchar()
/* I may also want to buffer some recently read characters so that I   */
/* can display them when I notice an error.                            */
/* the linenum/colnum code is probably inappropriate now               */
{
    if (curchar==EOF) return(curchar);
#ifdef PASCAL
    if (pending_dot != NOTACHAR) curchar = pending_dot, pending_dot = NOTACHAR;
    else
#endif
    curchar = pp_nextchar();
    if (curchar=='\n')
    {   linenum += 1;
        colnum = 0;
    }
    else colnum += 1;
    return(curchar);
}

#define is_e(c)                         /* e or E (exponent marker) */ \
    (((c) == 'e') || ((c) == 'E'))

#define is_x(c)                         /* x or X (hex num marker)  */ \
    (((c) == 'x') || ((c) == 'X'))


static int read_floating()
{   int flag = NUM_FLOAT;
    while(isdigit(curchar))
    {   if (namecount<NAMEMAX)
        {   namebuf[namecount++] = curchar;
            nextchar();
        }
        else
        {   lex_err("Grossly over-long floating point number");
            while(isdigit(curchar)) nextchar();
        }
    }
    if (is_e(curchar))
    {
        namebuf[namecount++] = 'e';  /* normalize case of the exponent mark */
        nextchar();
        if ((curchar == '+') | (curchar=='-'))
        {   namebuf[namecount++] = curchar;
            nextchar();
        }
        if (!isdigit(curchar))
        {   lex_err("Digit required after exponent marker");
            /* Assume exponent of zero */
            namebuf[namecount++] = '0';
        }
        else
        {   while(isdigit(curchar))
            {   if (namecount<NAMEMAX)
                {   namebuf[namecount++] = curchar;
                    nextchar();
                }
                else
                {   lex_err("Grossly over-long floating point number");
                    while(isdigit(curchar)) nextchar();
                }
            }
        }
    }
#ifndef PASCAL
    switch (curchar)
    {   case 'l': case 'L': flag |= NUM_LONG; nextchar(); break;
        case 'f': case 'F': flag |= NUM_SHORT; nextchar(); break;
    }
#endif
    return flag;
}

static AEop read_number(radix)
int radix;
{   int flag = NUM_INT;
    namecount = 0;          /* namebuf useful to collect chars */
    if (radix == 16)
    {   while (isxdigit(curchar))
        {   if (namecount<NAMEMAX)
            {   namebuf[namecount++] = curchar;
                nextchar();
            }
            else
            {   lex_err("Grossly over-long hexadecimal constant");
                while (isxdigit(curchar)) nextchar();
            }
        }
    }
    else
    {   while (isdigit(curchar))
        {   if (namecount<NAMEMAX)
            {   namebuf[namecount++] = curchar;
                nextchar();
            }
            else
            {   lex_err("Grossly over-long number");
                while (isdigit(curchar)) nextchar();
            }
        }
        if (curchar=='.')
        {   nextchar();
#ifdef PASCAL
/* Pascal seems to need an extra character of look-ahead here, for       */
/* consider the sequence 1.2 which is a floating point constant and then */
/* 1..2 which is three tokens. For that matter the lexical analyser must */
/* view 1.e as three tokens (integer, dot, symbol with name 'E').        */
            if (!isdigit(curchar)) pending_dot = curchar, curchar = '.';
            else
            {   namebuf[namecount++] = '.';
                flag = read_floating();
            }
#else
            if (namecount==0) namebuf[namecount++] = '0'; /* consider '0.' */
            namebuf[namecount++] = '.';
            flag = read_floating();
#endif
        }
        else if (is_e(curchar))
        {   if (namecount==0) namebuf[namecount++] = '0'; /* consider '0e5' */
            flag = read_floating();
        }
    }
    if (namecount==0)
    {   if (radix == 16) lex_err("Hex digit needed after 0x or 0X");
        namebuf[namecount++] = '0';         /* treat as 0x0 */
    }
    namebuf[namecount] = 0;
    if (flag & NUM_FLOAT) return make_floating(namebuf,flag);
    for (;;)
    {
#ifndef PASCAL
        if ((curchar == 'l' || curchar == 'L') && !(flag & NUM_LONG))
        {   flag |= NUM_LONG; nextchar(); continue;
        }
        if ((curchar == 'u' || curchar == 'U') && !(flag & NUM_UNSIGN))
        {   flag |= NUM_UNSIGN; nextchar(); continue;
        }
#endif
        return make_integer(radix, flag);
    }
}


/* string and char reading (unified reading & semantic routines) ... */

static int character_literal()
/* read a possibly escaped (with backslash (\)) character for a           */
/* character or string literal.  Result is in range 0-0xfff (e.g. 0xfff). */
/* Special case -1 => null escape read.                                   */
{
    int ch = curchar;
#ifndef PASCAL
    if (ch == '\\') switch(ch = nextchar())
    {
#ifdef __STDC__
/* '\a' id only an ANSI escape, for non-ansi compilers we need to use	  */
/* 0x07 which is ascii BEL character.					  */
case 'a':       ch = '\a';      break;  /* attn (otherwise known as bell) */
#else
case 'a':       ch = 0x07;      break;  /* attn (otherwise known as bell) */
#endif
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

case 'x':       if (!isxdigit(ch = nextchar()))
                {   lex_err("Missing hex digit(s) after \\x");
                    return 'x';         /* any better ideas?              */
                }
                ch = intofxdigit(ch);
                if (isxdigit(nextchar()))       /* second & third digits  */
                {   ch = (ch<<4) + intofxdigit(curchar);
                    if (isxdigit(nextchar()))
                        ch = (ch<<4) + intofxdigit(curchar), nextchar();
                }
                return(ch);

case '0':       case '1':
case '2':       case '3':               /* octal escapes                  */
case '4':       case '5':
case '6':       case '7':
                ch = intofdigit(ch);
                if (isodigit(nextchar()))       /* second & third digits  */
                {   ch = (ch<<3) + intofdigit(curchar);
                    if (isodigit(nextchar()))
                        ch = (ch<<3) + intofdigit(curchar), nextchar();
                }
                return(ch);

/* note that the sequence \<nl> (but not \<space><nl>) is now removed at
   all possible occasions by the preprocessor.   However, code for handling
   such situations is left here so that \<space><nl> is removed with just a
   warning message.
   Moreover we have to take care that (e.g.) \\<nl><nl> gives an error.
*/
case ' ':
case '\t':      lex_err("\\<space> and \\<tab> are invalid string escapes");

/* If I see \<space> I generate a warning message. I then skip over any
 * following whitespace, up to one newline.  Thus the effect is that
 * \(space|tab)*(nl|) are ignored just as \nl.
 */
           do nextchar(); while ((curchar==' ') || (curchar=='\t'));
           if (curchar=='\n') nextchar();
case '\n': /* drop through.  note no nextchar() here so read_string()
              will give an error message. */
           return -1;

default:  lex_rerr("illegal string escape '\\%c' - treated as %c", ch, ch);
          break;
          /* use "return '\\';" to treat '\Q' as '\\Q'  */
    }
#endif
    nextchar();
    return(ch);
}

static void read_string(quote)
int quote;
{   int escch;
    char *symstrp = lex_strptr, *val = lex_strptr;
    nextchar();
#ifdef PASCAL
    while (curchar!=quote || (nextchar(), curchar==quote))
#else
    while (curchar!=quote)
#endif
    {   if ((curchar=='\n') || (curchar==EOF))
        {   lex_err("Newline or end of file within string");
            break;   /* slightly untidy about nextchar() below, but OK */
        }
/* If I run off the end of this segment of lex_strbuf I allocate another  */
/* and copy the part-string into it, doubling the size if necessary.      */
        if (symstrp >= lex_strend)
        {   int n = symstrp - val, allocsize = LEX_STRBUFSIZE;
            while (n*2 > allocsize) allocsize *= 2;
            {   char *oval = val;
                val = BindAlloc(allocsize);
                memcpy(val, oval, symstrp-oval);
                lex_strptr = symstrp = val + (symstrp-oval);
                lex_strend = val + allocsize;
            }
        }
        if ((escch = character_literal()) != -1) *symstrp++ = escch;
    }
#ifdef PASCAL
    if (1)        /* chars and strings both parsed as strings for Pascal */
#else
    nextchar();
    if (quote == '"')
#endif
    {   curlex.a2.len = symstrp - val;
        lex_strptr = symstrp;         /* commit usage */
        curlex.a1.s = val;
        curlex.sym = s_string;
    }
    else
    {   int k = 0, n = symstrp - val;
/* note that for char constants we do not commit symstrp to lex_strptr */
        if (n == 1) k = *(unsigned char *)val;  /* works on all machines */
        else
        {   if (n == 0)
                lex_rerr("no chars in character constant ''");
            if (n > INTWIDTH)
                lex_rerr("more than 4 chars in '...'"), n = INTWIDTH;
            else
                lex_warn("non-portable - not 1 char in '...'");
            /* remember 'val' may not be word aligned */
            memcpy(&k, val, n);   /* may be unexpected on 68000 type m/c's */
        }
        curlex.a1.i = k;
        curlex.a2.flag = NUM_INT;     /* perhaps NUM_INT|NUM_LONG if n=4?         */
        curlex.sym = s_integer;    /* chars give produce int consts.           */
    }
}

static int next_basic_sym()
{   int charinfo, savechar;

    if (curchar == NOTACHAR) nextchar();
    for (;;)
    {   if (curchar == EOF)  return (curlex.sym = s_eof);
        if (!isspace(curchar)) break;
        if (curchar == '\n' && pp_inhashif) return (curlex.sym = s_eol);
        nextchar();
    }
    switch ((charinfo = lexclass[(unsigned char)curchar]) & LEX_CHTYPE)
    {
default:    lex_err((curchar == '#' || curchar == '\\') ?
                            "misplaced preprocessor character '%c'" :
                            "illegal character (0x%x = \'%c\') in source",
                      curchar, curchar);
            nextchar();
            next_basic_sym();
            break;
case l_idstart:
            namecount = 0;          /* number of characters read */
            namehash = 1;           /* start of hash total       */
            do
            {   if (namecount<NAMEMAX)
                {
#ifdef PASCAL
                    curchar = tolower(curchar);
#endif
                    namebuf[namecount++] = curchar;
                    update_hash(curchar);
                }
                nextchar();
            } while (lexclass[(unsigned char)curchar] & l_idcont);
            namebuf[namecount] = 0;
            curlex.sym = lookupname(namebuf);
#ifndef NO_VALOF_BLOCKS
/* The following represents a rather nasty piece of context-sensitive      */
/* hackery - inside a valof block the word 'resultis' is recognized as a   */
/* keyword, but otherwise (as required in regular ANSI C) it is just       */
/* another ordinary symbol. I wonder if there is a better solution to this */
/* issue........... ??                                                     */
            if (inside_valof_block && curlex.sym == s_identifier &&
                curlex.a1.sv == resultisword) curlex.sym = s_resultis;
#endif
            break;
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
#ifdef PASCAL
/* Note that in Pascal numbers are not allowed to start with dots        */
                if (curchar == ')')
                    curlex.sym = s_rbracket,
                    curchar = NOTACHAR;
                else if (curchar == '.')
                    curlex.sym = s_ellipsis,
                    curchar = NOTACHAR;
                else curlex.sym = s_dot;
                break;
#else
                if (isdigit(curchar))
                {   int flag;
                    namecount = 0;
                    namebuf[namecount++] = '.';
                    namebuf[namecount++] = curchar;
                    nextchar();
                    flag = read_floating();  /* change to use read_number()? */
                    namebuf[namecount] = 0;
                    curlex.sym = make_floating(namebuf, flag);
                }
                else
                {   int n = 1;
                    while (curchar=='.') n++, nextchar();
                    switch (n)
                    {   case 1: curlex.sym = s_dot; break;
                        default: lex_err("'...' must have exactly 3 dots");
                                /* drop through */
                        case 3: curlex.sym = s_ellipsis; break;
                    }
                }
                break;
#endif
#ifdef PASCAL
case l_lpar:
                savechar = curchar;
                nextchar();
                if (curchar == '.')
                    curlex.sym = s_lbracket,
                    curchar = NOTACHAR;
                else curlex.sym = s_lpar;
                break;
case l_less:
                savechar = curchar;
                nextchar();
                if (curchar == '>')
                    curlex.sym = s_notequal,
                    curchar = NOTACHAR;
                else if (curchar == '=')
                    curlex.sym = s_lessequal,
                    curchar = NOTACHAR;
                else curlex.sym = s_less;
                break;
#endif
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
                else if (curchar == '=')
                    curlex.sym = charinfo >> 24,
                    curchar = NOTACHAR;
                else curlex.sym = charinfo >> 8 & 255;
                break;
case l_shifter: savechar = curchar;            /* very much like l_selfglue */
                nextchar();                    /* (32 bits is too few)      */
                if (curchar == '=')
                    curlex.sym = charinfo >> 24,
                    curchar = NOTACHAR;
#ifndef PASCAL
                else if (curchar == savechar)
                {   curlex.sym = charinfo >> 16 & 255;
                    nextchar();
                    if (curchar == '=')
                        curlex.sym = assignop_(curlex.sym),        /* >> to >>= etc */
                        curchar = NOTACHAR;
                }
#endif
                else curlex.sym = charinfo >> 8 & 255;
                break;
#ifndef PASCAL
case l_minus:   nextchar();            /* very like l_selfglue but for "->" */
                if (curchar=='-')
                    curlex.sym = s_minusminus,
                    curchar = NOTACHAR;
                else if (curchar=='=')
                    curlex.sym = s_minusequal,
                    curchar = NOTACHAR;
                else if (curchar=='>')
                    curlex.sym = s_arrow,
                    curchar = NOTACHAR;
                else curlex.sym = s_minus;
                break;
#endif
case l_quote1:
case l_quote2:  read_string(curchar);
                break;
#ifdef lex_ppsyms
case l_hash:    if (colnum==1)
                {   preprocess_request();
                    next_basic_sym();
                }
                else
                {   curlex.sym = s_sharp;
                    curchar = NOTACHAR;
                }
                break;
#endif
    }
#ifndef PASCAL
    /* recognise whitespace (but NOT newlines) in += etc    */
    /* as a sop to olde-style (K&R) C.                      */
    if (can_have_becomes(curlex.sym))
    {   if (curchar == NOTACHAR) nextchar();
        while (curchar == ' ' || curchar == '\t') nextchar();
        if (curchar == '=')
        {   curlex.sym = and_becomes(curlex.sym);
            curchar = NOTACHAR;
            lex_err("$s may not have whitespace in it", curlex.sym);
        }
    }
#endif
    return(curlex.sym);
}


/* exported routines... */

AEop nextsym()
{
    next_basic_sym();
    if ((debugging(DEBUG_LEX))!=0)
        {   fprintf(stderr, "<nextsym: sym %d=%s %x>",
                            curlex.sym, symbol_name_(curlex.sym), curlex.a1.i);
            if (curlex.sym==s_identifier)
                fprintf(stderr, "%s", _symname(curlex.a1.sv));
            putc('\n', stderr);
        }
    return(curlex.sym);
}

AEop nextsym_for_hashif()
{   curchar = NOTACHAR;   /* could only have been '\n' or NOTACHAR */
    return nextsym();
}

Symstr *gensymval(glo)
int glo;
{
    /* Generates a new symbol with unique name                       */
    char name[30];
    Symstr *temp = NULL;                 /* an empty chain to put it on */
    sprintf(name, "<Intsym_%d>", ++gensymnum);
    return(lookup(name, &temp, glo));
}

static void init_sym_name_table()
{   /* add entries for error messages for non-reserved words, e.g. block */
    /* (currently) non-table driven... */
    sym_name_table[s_error]             = "<previous error>";
    sym_name_table[s_invisible]         = "<invisible>";
    sym_name_table[s_let]               = "<let>";
    sym_name_table[s_character]         = "<character constant>";
    sym_name_table[s_integer]           = "<integer constant>";
    sym_name_table[s_floatcon]          = "<floating constant>";
    sym_name_table[s_string]            = "<string constant>";
    sym_name_table[s_identifier]        = "<identifier>";
    sym_name_table[s_binder]            = "<variable>";
    sym_name_table[s_tagbind]           = "<struct/union tag>";
    sym_name_table[s_displace]          = "++ or --";
    sym_name_table[s_postinc]           = "++";
    sym_name_table[s_postdec]           = "--";
    sym_name_table[s_arrow]             = "->";
    sym_name_table[s_leftshiftequal]    = "<<=";
    sym_name_table[s_rightshiftequal]   = ">>=";
    sym_name_table[s_addrof]            = "unary &";
    sym_name_table[s_content]           = "unary *";
    sym_name_table[s_monplus]           = "unary +";
    sym_name_table[s_neg]               = "unary -";
    sym_name_table[s_fnap]              = "<function argument>";
    sym_name_table[s_subscript]         = "<subscript>";
    sym_name_table[s_cast]              = "<cast>";
    sym_name_table[s_sizeoftype]        = "sizeof";
    sym_name_table[s_sizeofexpr]        = "sizeof";
    sym_name_table[s_endcase]           = "break";
    sym_name_table[s_block]             = "<block>";
    sym_name_table[s_decl]              = "decl";
    sym_name_table[s_fndef]             = "fndef";
    sym_name_table[s_typespec]          = "typespec";
    sym_name_table[s_typedefname]       = "typedefname";
    sym_name_table[s_valof]             = "valof";
    sym_name_table[s_ellipsis]          = "...";
    sym_name_table[s_eol]               = "\\n";
    sym_name_table[s_eof]               = "<eof>";
    sym_name_table[s_sharp]             = "#";
#ifdef TARGET_IS_XPUTER
    sym_name_table[s_diff ]             = "diff";
#endif
   
}

/* exported initialiser lex_init() is language dependent ... */

#ifdef PASCAL

Symstr *pasclab(n)
int n;
/* This routine forges a Symstr for a pascal numerical label */
{   char v[20];
    sprintf(v, "%u", n);
    return sym_insert(v, s_identifier);
}

void lex_init()
{
    int i;
    static struct { int lc; char name[2]; char name1[3]; char name2[3]; }
      sp[20] = {
        { mklc(l_noglue, s_rpar, 0, 0),      ")" },
        { mklc(l_noglue, s_lbracket, 0, 0),  "[" },
        { mklc(l_noglue, s_rbracket, 0, 0),  "]" },
        { mklc(l_noglue, s_lbrace, 0, 0),    "{" },
        { mklc(l_noglue, s_rbrace, 0, 0),    "}" },
        { mklc(l_noglue, s_semicolon, 0, 0), ";" },
        { mklc(l_noglue, s_comma, 0, 0),     "," },
        { mklc(l_noglue, s_content, 0, 0),   "@" },
        { mklc(l_noglue, s_content, 0, 0),   "^" },
        { mklc(l_noglue, s_plus, 0, 0),      "+" },
        { mklc(l_noglue, s_minus, 0, 0),     "-" },
        { mklc(l_noglue, s_times, 0, 0),     "*" },
        { mklc(l_noglue, s_div, 0, 0),       "/" },
        { mklc(l_noglue, s_equalequal, 0, 0),"=" },
        { mklc(l_shifter, s_colon, s_assign, s_assign),
                                             ":", ":=", ":=" },
        { mklc(l_shifter, s_greater, s_greaterequal, s_greaterequal),
                                             ">", ">=", ">=" },
        /* magic chars - 1 per class */
        { mklc(l_lpar,   s_lpar, s_lbracket, 0),           "(", "[" },
        { mklc(l_less,   s_less, s_boolnot, s_lessequal),  "<", "<>", "<=" },
        { mklc(l_quote2, 0, 0, 0),                         "\'" },
        { mklc(l_dot,    s_dot, s_rbracket, 0),            ".", "]" }
    };
    static struct { char *name; AEop sym; } ns[35] = {
        { "and",      s_and },
        { "array",    s_array },
        { "begin",    s_begin },
        { "case",     s_case },
        { "const",    s_const },
        { "div",      s_idiv },
        { "do",       s_do },
        { "downto",   s_downto },
        { "else",     s_else },
        { "end",      s_end },
        { "file",     s_file },
        { "for",      s_for },
        { "function", s_function },
        { "goto",     s_goto },
        { "if",       s_if },
        { "in",       s_in },
        { "label",    s_label },
        { "mod",      s_rem },
        { "nil",      s_nil },
        { "not",      s_boolnot },
        { "of",       s_of },
        { "or",       s_or },
        { "packed",   s_packed },
        { "procedure",s_procedure },
        { "program",  s_program },
        { "record",   s_record },
        { "repeat",   s_repeat },
        { "set",      s_set },
        { "then",     s_then },
        { "to",       s_to },
        { "type",     s_type },
        { "until",    s_until },
        { "var",      s_var },
        { "while",    s_while },
        { "with",     s_with },
    };
    for (i=0; i<s_NUMSYMS; i++) sym_name_table[i] = "<?>";
    /* although lexclass[] is notionally constant, C's initialisation
       facilities do not conveniently enable us to initialise it... */
    for (i=0; i<=UCHAR_MAX; i++) lexclass[i] = l_illegal;
    setuplexclass1("ABCDEFGHIJKLMNOPQRSTUVWXYZ_", l_idstart);
    setuplexclass1("abcdefghijklmnopqrstuvwxyz", l_idstart);
    setuplexclass1("0123456789", l_digit1);
    pending_dot = NOTACHAR;
    /* common code continues below ... */

#else  /* not PASCAL */

void lex_init()
{
    int i;
    static struct { int lc; char name[2]; char name1[3]; char name2[3]; }
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
        { "const",    s_const },
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
#ifndef NO_VALOF_BLOCKS
        { "resultis", s_resultis },
#endif
        { "return",   s_return },
        { "short" ,   s_short },
        { "signed",   s_signed },
        { "sizeof",   s_sizeof },
        { "static",   s_static },
        { "struct",   s_struct },
        { "switch",   s_switch },
        { "typedef",  s_typedef },
        { "union",    s_union },
        { "unsigned", s_unsigned },
        { "void",     s_void },
        { "volatile", s_volatile },
        { "while",    s_while },
#ifdef lex_ppsyms  /* dead */
        { "#define",  s_define },
        { "defined",  s_defined },
        { "#elif",    s_elif },
        { "#else",    s_else },
        { "#endif",   s_endif },
        { "#if",      s_if },
        { "#ifdef",   s_ifdef },
        { "#ifndef",  s_ifndef },
        { "#include", s_include },
        { "#line",    s_line },
        { "#pragma",  s_pragma },
        { "#undef",   s_undef },
#endif  /* lex_ppsyms */
    };
    for (i=0; i<s_NUMSYMS; i++) sym_name_table[i] = "<?>";
    /* although lexclass[] is notionally constant, C's initialisation
       facilities do not conveniently enable us to initialise it... */
    for (i=0; i<=UCHAR_MAX; i++) lexclass[i] = l_illegal;
    setuplexclass1("ABCDEFGHIJKLMNOPQRSTUVWXYZ_", l_idstart);
    setuplexclass1("abcdefghijklmnopqrstuvwxyz", l_idstart);
    setuplexclass1("123456789", l_digit1);
    setuplexclass1("0", l_digit0);

#endif   /* PASCAL */

    /* common code for both languages */
    for (i = 0; i < sizeof(sp)/sizeof(sp[0]); i++)
    {   char *name = sp[i].name; int lc = sp[i].lc;
        AEop s;
        lexclass[(unsigned char)*name] = lc;
        if ((s = (lc >> 8) & 255) != 0) sym_name_table[s] = name;
        if ((s = (lc >> 16) & 255) != 0) sym_name_table[s] = sp[i].name1;
        if ((s = (lc >> 24) & 255) != 0) sym_name_table[s] = sp[i].name2;
    }
    hashvec = GlobAlloc(sizeof(*hashvec));
    for (i = 0; i<LEX_HASHSIZE; i++) (*hashvec)[i] = NULL;
    for (i = 0; i < sizeof(ns)/sizeof(ns[0]); i++)
    {   char *name = ns[i].name; int sym = ns[i].sym;
        sym_insert(name, sym);
        sym_name_table[sym] = name;
    }
    init_sym_name_table();
#ifndef NO_VALOF_BLOCKS
/* 'resultis' is a funny (experimental) syntax extension */
    resultisword = sym_insert("resultis", s_identifier);
#endif
    curchar = NOTACHAR; /* Kill lookahead  */
    curlex.sym = s_nothing;
    linenum =               /* At start of file                    */
    colnum =
    gensymnum = 0;
    lex_strend = lex_strptr = DUFF_ADDR;
    
    fc_zero   = real_of_string("0.0", NUM_FLOAT);
    fc_two_31 = real_of_string("2147483648.0", NUM_FLOAT);
}

void lex_beware_reinit()
{   /* this routine patches the fact that an (illegal) C program of the */
    /* form 'int a = 1 "abc"' or 'f(){} "abc"' needs to be able to      */
    /* print out "abc" in an error message even though Local store will */
    /* have been clobbered by alloc_reinit().  Move it to Global store! */
    if (curlex.sym == s_string)
    {   char *oval = curlex.a1.s;
        curlex.a1.s = GlobAlloc(pad_to_word(curlex.a2.len));
        memcpy(curlex.a1.s, oval, curlex.a2.len);
    }
}

void lex_reinit()
{
    lex_strend = lex_strptr = DUFF_ADDR;
}

/* End of lex.c */
