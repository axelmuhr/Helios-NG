/* $Id: pp.c,v 1.7 1994/09/08 10:17:13 nickc Exp $ */

/* C pre-processor, pp.c:  Copyright (C) A. Mycroft and A.C. Norman */

#define PP_HERALD "C preprocessor version 0.32a\n"
/* #defining PP_STANDALONE should enable a stand-alone pre-processor.   */

#include <stddef.h>
#include <string.h>
#include <time.h>
#ifndef PP_STANDALONE
#  include "cchdr.h"
#else
#include <ctype.h>
#include <stdio.h>
#endif 

/* N.B. This pre-processor conforms to the May 86 draft ANSI spec.       */
/* However, there it was defined by example and the Oct 86 draft gave    */
/* a more stringent definition by (more) examples!  I doubt that it is   */
/* up to the latter spec, but propose to do nothing about this until the */
/* next draft spec or standard as further changes are proposed.          */

/* Memo: chase out '@@@'s.  Also free pp_ch's. Also abuf for junk    */
/* This pre-preprocessor can be compiled stand-alone -- but see machine */
/* dependencies (search for string 'stand-alone').                    */

/* Note that is much nastiness (probably necessary in C) in the odd
 * interface of the pre-processor having a back-route into the parser
 * and HENCE back into itself!!!                                           
 */

#ifdef PP_STANDALONE
#  define bool int
#  define CHAR_FF '\f'
#  define CHAR_VT '\v'
#  define CHAR_CR '\r'
   static FILE *pp_inclopen();
   static int pp_debugging;
#  define debugging(flavour) pp_debugging
#  define memclr(a,n) memset(a,0,n)
#endif

#define PP_HASHSIZE 259
#define PP_DIRLEN 16
/* PP_DEFLEN is max number of significant chars in an identifier.
   It must be LESS than min(PP_ABUFSIZ,PP_DBUFSIZ)-UNRDCHMAX.
*/
#define PP_DEFLEN 250
#define PP_UNRDCHMAX 2  /* see ansi trigraphs */
/* PP_NOEXPAND is a pseudo character to inhibit re-expansion of the
   preceding name.  This is required in (say) ansi #define f(x) f(x+1)
   which is thus saved as "f(x) = f<PP_NOEXPAND>(x+1)".  See uses.
   Note that pp_rdch means that '\f' can never get into macro definitions. */
#define PP_NOEXPAND '\f'

#define pp_rdch() ((pp_ebufptr==pp_ebuftop) ? pp_rdch3() : *pp_ebufptr++)
/* pp_unrdch evals its arg twice.  Do not call more than UNRDCHMAX times.  */
#ifdef ACE
#define pp_unrdch(ch) ((ch) == EOF ? (void)0 : (pp_ebufptr--, (void)(*pp_ebufptr = (ch))))
#else
#define pp_unrdch(ch) ((ch) == EOF ? (void)0 : (void)(*--pp_ebufptr = (ch)))
#endif
#define pp_wrch(ch)     \
    (pp_abuf_ensure(1), \
     (void)(*pp_abufptr++ = (ch)))
#define pp_wrbuf(buf, len) \
    (pp_abuf_ensure(len),  \
       (void)(memcpy(pp_abufptr, buf, len), pp_abufptr += (len)))
#define pp_savbuf(buf,len) \
    (pp_ebuf_ensure(len),  \
       (void)(memcpy(pp_ebuftop, buf, len), pp_ebuftop += (len)))
#define pp_savch(ch)    \
    (pp_ebuf_ensure(1), \
       (void)(*pp_ebuftop++ = (ch)))

#define pp_skipb0(ch) { while ((ch = pp_rdch()) == ' ' || ch == '\t'); }
#define pp_skipb1(ch) { while (ch == ' ' || ch == '\t') ch = pp_rdch(); }
#define pp_skipl0(ch) { while (ch = pp_rdch(), !PP_EOLP(ch) && ch!=EOF); }
#define pp_skipl1(ch) { while (!PP_EOLP(ch) && ch!=EOF) ch = pp_rdch(); }
#define pp_cidchar(ch) (isalnum(ch) || (ch)=='_')
#define PP_EOLP(ch) ((ch) == '\n')

/* structures - local to this file ... */

typedef struct arglist {
  char *argname;
  struct arglist *argchain;
  int argactual;   /* now offset into pp_abufbase */
/* beware: it is only our expansion strategy that allows us to
   use shallow binding of actual/formal correspondence
*/
} PP_ARGENTRY;

#define pp_argname_(p)    ((p)->argname)
#define pp_argchain_(p)   ((p)->argchain)
#define pp_argactual_(p)  ((p)->argactual)

typedef struct hashentry {
  char *name;
  union { char *s; int i; } body;  /* #define text or magic (e.g PP__LINE) */
  struct hashentry *chain;
  unsigned int uses:29,   /* incremented on reference */
               ismagic:1, /* things like __TIME__     */
               noargs:1,  /* 1 if noargs              */
               alive:1;   /* 0 => undef'd, 1 => def'd */
  struct hashentry *defchain;  /* chain in definition order */
  struct arglist *arglist;     /* only if noargs==0   */
} PP_HASHENTRY;

#define pp_hashname_(p) ((p)->name) 
#define pp_hasharglist_(p) ((p)->arglist)
#define pp_hashbody_(p) ((p)->body.s)
#define pp_hashmagic_(p) ((p)->body.i)
#define pp_hashchain_(p) ((p)->chain)
#define pp_hashnoargs_(p) ((p)->noargs)
#define pp_hashalive_(p) ((p)->alive)
#define pp_hashismagic_(p) ((p)->ismagic)
#define pp_hashuses_(p)  ((p)->uses)
#define pp_hashdefchain_(p) ((p)->defchain)

#define PP__LINE (-1)
#define PP__FILE (-2)
#define PP__DATE (-3)
#define PP__TIME (-4)
#define PP__STDC (-5)
#define PP__ZERO (-6)
#define PP__ONE  (-7)

typedef struct ifstack {
  struct ifstack *chain;
  int seenelse;
  int oldskip;
  int skipelse;
} PP_IFSTACK;

#define pp_ifchain_(p)    ((p)->chain)
#define pp_ifseenelse_(p) ((p)->seenelse)
#define pp_ifoldskip_(p)  ((p)->oldskip)
#define pp_ifskipelse_(p) ((p)->skipelse)

typedef struct filestack {
  struct filestack *chain;
  int ch;
  FILE *stream;
  char *name;
  int line;
} PP_FILESTACK;

#define pp_filchain_(p)   ((p)->chain)
#define pp_filech_(p)     ((p)->ch)
#define pp_filstream_(p)  ((p)->stream)
#define pp_filename_(p)   ((p)->name)
#define pp_fileline_(p)   ((p)->line)

static PP_HASHENTRY *((*pp_hashtable)[PP_HASHSIZE]),
             *pp_hashfirst, *pp_hashlast, *pp_hashone, *pp_hashzero;
static PP_IFSTACK *pp_ifstack, *pp_freeifstack;
static PP_FILESTACK *pp_filestack, *pp_freefilestack;

static int pp_nsubsts;
static bool pp_skipping, pp_instring;
static int pp_ch, pp_xlinect;

int pp_linect;
bool pp_inhashif;
int pp_pragmavec['z'-'a'+1];
char *pp_cisname;

static FILE *pp_cis;
static char pp_datetime[26];

/* diagnostics and free store package... */

#ifndef PP_STANDALONE
#  define pp_warn    cc_warn
#  define pp_err     cc_err
#  define pp_rerr    cc_rerr
   static void *pp_alloc(n)
   int n;
   {   stuse_pp += n;
       return GlobAlloc(n);
   }
#else
#define syserr pp_die
void pp_die(s)
char *s;
{ fprintf(stderr, "Fatal error: %s\n", s);
  exit(1);
}

#define cc_fatalerr pp_warn
#define pp_err pp_warn
#define pp_rerr pp_warn
void pp_warn(s, a,b,c,d,e,f,g,h)
int a,b,c,d,e,f,g,h;
char *s;
{ fprintf(stderr, "Warning in line %d of %s: ", pp_linect, pp_cisname);
  fprintf(stderr,s,a,b,c,d,e,f,g,h);
  fprintf(stderr, "\n");
}

static void *pp_alloc(n)
int n;
{ char *p;
  if ((p = malloc(n)) == 0) pp_die("No store left");
  return p;
}
#endif

/* buffers:  pbuf (ptrs) dbuf (defns) ebuf (expansions) abuf (args)
   Maybe optimise later.
*/
#define PP_PBUFSIZ (1024 * sizeof(int *))
#define PP_DBUFSIZ 1024    /* default chunk for definitions */
#define PP_ABUFINITSIZ 256
#define PP_EBUFINITSIZ 256

#ifndef PP_STANDALONE
#define pp_new_(type) (pp_alloc(type))
#ifdef COMPILING_ON_ST
#define PP_NOARGHASHENTRY sizeof(PP_HASHENTRY)
#else
#define PP_NOARGHASHENTRY offsetof(PP_HASHENTRY,arglist)
#endif
#else
#define pp_new_(type) (pp_pvec(type))
#define PP_NOARGHASHENTRY sizeof(PP_HASHENTRY)

char *pbufptr, *pbufbase;

static void *pp_pvec(n)   /* pvec allocates records for #if's etc */
int n;                 /* n must always be a multiple of 4 */
{ if (pbufptr == 0 || (pbufptr = &pbufptr[-n]) < pbufbase)
  { pbufbase = pp_alloc(PP_PBUFSIZ);
    pbufptr = pbufbase + (PP_PBUFSIZ - n);
  }
  return pbufptr;
}
#endif

static char *pp_dbufend, *pp_dbufseg, *pp_dbufptr;
#define pp_stuffid_(ch) \
    (pp_dbufptr==pp_dbufend ? pp_newdbuf(ch) : (*pp_dbufptr++ = (ch)))

static char pp_newdbuf(x)
char x;
{ char *dbufbase;
  unsigned int size = pp_dbufptr-pp_dbufseg;   /* size used in current dbuf */
  unsigned int allocsize = PP_DBUFSIZ;         /* default size for new dbuf */
  /* allocate 1024 for small requests, but 2048 for 512-1023, etc. */
  while (size >= allocsize/2) allocsize *= 2;
  if (size > allocsize) syserr("pp_newdbuf(%d,%d)", size, allocsize);
  dbufbase = pp_alloc(allocsize);
  if (debugging(DEBUG_PP))
      fprintf(stderr, "new pp_dbuf(%d)\n", allocsize);
  memcpy(dbufbase, pp_dbufseg, size);
  pp_dbufseg = dbufbase;
  pp_dbufptr = dbufbase+size;
  pp_dbufend = dbufbase+allocsize;
  return (*pp_dbufptr++ = x);
}

static char *pp_closeid()
{ char *result;
  pp_stuffid_(0);
  result = pp_dbufseg;
  pp_dbufseg = pp_dbufptr;
  return result;
}

static char *pp_ebufbase, *pp_ebufptr, *pp_ebuftop, *pp_ebufend;
static char *pp_abufbase, *pp_abufoptr, *pp_abufptr, *pp_abufend;

static void pp_abuf_ensure(n)
int n;
{   while (pp_abufptr + n >= pp_abufend)
    {   int k = pp_abufend - pp_abufbase;
        char *d = pp_alloc(2*k);
        if (debugging(DEBUG_PP))
            fprintf(stderr, "up pp_abuf to %d\n", 2*k);
        memcpy(d, pp_abufbase, k);
        pp_abufend = d + 2*k;
        pp_abufptr = d + (pp_abufptr - pp_abufbase);
        pp_abufoptr = d + (pp_abufoptr - pp_abufbase);
        pp_abufbase = d;
    }
}

static void pp_ebuf_ensure(n)
int n;
{   while (pp_ebuftop + n >= pp_ebufend)
    {   int k = pp_ebufend - pp_ebufbase;
        char *d = pp_alloc(2*k);
        if (debugging(DEBUG_PP))
            fprintf(stderr, "up pp_ebuf to %d\n", 2*k);
        memcpy(d, pp_ebufbase, k);
        pp_ebufend = d + 2*k;
        pp_ebuftop = d + (pp_ebuftop - pp_ebufbase);
        pp_ebufptr = d + (pp_ebufptr - pp_ebufbase);
        pp_ebufbase = d;
    }
}

/* I/O routines, including ANSI trigraph routine. */
/* the routines pp_rdch1, pp_rdch2,... correspond to the phases of the
   1-Jul-85 ANSI draft section B.1.1.2.
*/

static int pp_rdch0last, pp_peep1cnt, pp_peep2cnt, pp_peep3cnt;

/* note that all the read-ahead buffers below are empty at '\n' (also
   pp_rdch0last == '\n') hence #includes do not need to worry about them.  */

static int pp_rdch0()
{   if (pp_rdch0last == '\n') pp_linect++;
    /* N.B. we can only count the newline when we read the NEXT character
       if we have any hope of getting line numbers of (e.g.) identifiers
       at EOL correct.
    */
    for (;;)
    {   int ch = getc(pp_cis);

        if (!iscntrl(ch)) return pp_rdch0last = ch;

        switch (ch)
        {   case EOF:  if (pp_rdch0last == '\n') return EOF;
                       pp_rerr("Missing newline before EOF - inserted");
                       /* drop through */
            case CHAR_VT:
            case CHAR_FF:
            case CHAR_CR:     /* convert all end-of-line chars here to '\n' */
            case '\n': return pp_rdch0last = '\n';
            case '\t': return pp_rdch0last = ch;
            default:   pp_rerr("Control character 0x%.2x found - ignored", ch);
                       break;
        }
    }
}

static int pp_rdch1()
/* Main effect is to read chars from pp_cis and compress trigraphs.    */
/* If we really wanted to compiler this code in an environment where  ...
... 7 bit ascii chars were not available, then we have have to change ...
... the following character constants to their trigraph form too.      */
/* I may also want to buffer some recently read characters so that I   */
/* can display them when I notice an error.                            */
{
   static int peep1vec[3];   /* int in case EOF */
   int ch1, ch2, ch3, ch4;
/* Note: there is a small difference in the following whether I do
   an ungetc() or a pp_unrdch(). (consider ? ? ? = ).
   Try as hard as possible to spot one here, but check with ANSI spec.
   Moreover, the inclusion of ? escapes in strings (e.g. to get '"')
   is not upwards compatible and thus we give a warning if
   one is encountered.
*/
#define pp_nc() (pp_peep1cnt ? peep1vec[--pp_peep1cnt] : pp_rdch0())
#ifdef PASCAL
/* No need to worry about the ANSI C trigraphs                           */
    return pp_nc();
#endif
    if ((ch1 = pp_nc()) != '?') return ch1;
    if ((ch2 = pp_nc()) != '?')
      { peep1vec[pp_peep1cnt++]=ch2; return ch1; }
    switch (ch3 = pp_rdch0())
#undef pp_nc
    {   default:
            peep1vec[pp_peep1cnt++] = ch3;
            peep1vec[pp_peep1cnt++] = ch2;
            return ch1;
        case '=':   ch4 = '#'; break;
        case '(':   ch4 = '['; break;
        case ')':   ch4 = ']'; break;
        case '<':   ch4 = '{'; break;
        case '>':   ch4 = '}'; break;
        case '/':   ch4 = '\\'; break;
        case '\'':  ch4 = '"'; break;
        case '-':   ch4 = '~'; break;
        case '!':   ch4 = '|'; break;
    }
    pp_warn("ANSI '%c%c%c' trigraph for '%c' found - was this intended?",
            ch1, ch2, ch3, ch4);
    return ch4;
}

/* pp_rdch2() turns "\\\n" to "".
   It really ought to remove "\\<whitespace>\n" too, but this is rather
   delicate inside strings.  Moreoever, note that ANSI draft (July 85)
   allows a comment to end over a newline by '*\\\n/' !!!  */
static int pp_rdch2()
{   static int peep2vec[1];   /* int in case EOF */
    int ch;
    ch = (pp_peep2cnt ? peep2vec[--pp_peep2cnt] : pp_rdch1());
    for (;;)
    {   if (ch != '\\') return ch;
#ifdef PASCAL
/* Use of backslash is not proper in Pascal, but I will leave this in for now */
#endif
        /* think more about space and tab here */ 
        if ((ch = pp_rdch1()) != '\n')
        {   peep2vec[pp_peep2cnt++] = ch;
            return '\\';
        }
        ch = pp_rdch1();
    }
}

#ifdef PASCAL
#  define COMMENT_START '('
#  define COMMENT_END   ')'
#else
#  define COMMENT_START '/'
#  define COMMENT_END   '/'
#endif

/* pp_rdch3() turns comments into a single space.  Was pp_skipcomment().  */
static int pp_rdch3()
{   static int peep3vec[1];   /* int in case EOF */
    int ch;
    if (((ch = pp_peep3cnt ?
          peep3vec[--pp_peep3cnt] :
          pp_rdch2()) != COMMENT_START
#ifdef PASCAL
        && ch != '{'
#endif
       ) || pp_instring) return ch;
#ifdef PASCAL
    if (ch != '{')
#endif
    {   if ((ch = pp_rdch2()) != '*')
        {   peep3vec[pp_peep3cnt++] = ch;
            return COMMENT_START;
        }
        ch = pp_rdch2();
    }
    for (;;) switch (ch)
    {   case EOF: pp_err("EOF in comment"); return EOF;
        case '\n': pp_xlinect++; pp_wrch('\n');
                  /* drop through */
        default:  ch = pp_rdch2(); break;
#ifdef PASCAL
        case '}':
                  return ' ';
#else
        case COMMENT_START:
                  if ((ch = pp_rdch2()) != '*') break;
                  pp_warn("character sequence /* inside comment - error?");
                  /* drop through */
#endif
        case '*': if ((ch = pp_rdch2()) == COMMENT_END) return ' ';
                  break;
    }
}

static void pp_copystring(quote)
char quote;
{ int pp_ch;
  pp_instring = 1;
  if (!pp_skipping) pp_wrch(quote);
  while ((pp_ch = pp_rdch()) != quote)
  { switch (pp_ch)
    { case EOF: pp_err("EOF in string"); goto out;
      case '\n': pp_xlinect++;
                 pp_err("quote (%c) inserted before newline", quote);
                 goto out;
      default: if (!pp_skipping) pp_wrch(pp_ch); break;
      case '\\':
        if (!pp_skipping) pp_wrch('\\');
        switch (pp_ch = pp_rdch())
        { case EOF: pp_err("EOF in string escape"); goto out;
          case '\n': pp_xlinect++; pp_wrch(pp_ch); break;
          default: if (!pp_skipping) pp_wrch(pp_ch); break;
        }
    }
  }
out:
  if (!pp_skipping) pp_wrch(quote);
  if (pp_ch == '\n') pp_wrch(pp_ch);
  pp_instring = 0;
}

static void pp_stuffstring(quote,dequote)
char quote; int dequote;
{ int pp_ch, err = 0;
  pp_instring = 1;
  if (!dequote) pp_stuffid_(quote);
  while (!err && (pp_ch = pp_rdch()) != quote)
  { switch (pp_ch)
    { case EOF: 
      case '\n': err = 1; break;
      default:   pp_stuffid_(pp_ch); break;
      case '\\':
        if (!dequote) pp_stuffid_('\\');  /* @@@ what about #include "\x66" */
        switch (pp_ch = pp_rdch())
        { case EOF:
          case '\n': err = 1;
          default: pp_stuffid_(pp_ch); break;
        }
    }
  }
  if (err) pp_err("Missing '%c' in pre-processor command line", quote);
  if (!dequote) pp_stuffid_(quote);
  pp_instring = 0;
}

static int pp_spanstring(p)
char *p;
/* p is a buffer holding a string in text form.  Return offset beyond it */
{   char ch, quote = *p;
    int i = 0;
    while ((ch = p[++i]) != quote)
    { if (ch == 0) return i;                         /* really malformed */
      if (ch == '\\') if (p[++i] == 0) return i;     /* really malformed */
    }
    return i+1;
}


static int pp_eqname(s, v, n)   /* like strcmp but 2nd arg is base/length format */
char *s, *v;
int n;
{ while (n-- > 0) if (*s++ != *v++) return 0;
  if (*s != 0) return 0;
  return 1;
}

static char *pp_special(n,s)
int n; char *s;
{   switch (n)
    {   default:       syserr("Internal error in pp_special(%d)", n);
                       return "";
        case PP__LINE: sprintf(s, "%d", pp_linect);
                       return s;
        case PP__FILE: sprintf(s, "\"%s\"", pp_cisname);
#ifdef __DOS386
      /* translate MS-DOG path seperators '\' into UN*X ones '/' so that
	 C does not interpret them as escape characters */
      for (n = 0; s[n] != '\0'; n++)
	if (s[n] == '\\') s[n] = '/';
#endif
                       return s;
/* see the spec of asctime for the following numbers */
        case PP__DATE: sprintf(s, "\"%.7s%.4s\"",
                               pp_datetime+4, pp_datetime+20);
                       return s;
        case PP__TIME: sprintf(s, "\"%.8s\"", pp_datetime+11);
                       return s;
        case PP__ZERO: return "0";
        case PP__ONE:  /* drop through */
        case PP__STDC: return "1";
    }
}

static void pp_expand(p, nlsinargs)
PP_HASHENTRY *p; int nlsinargs;
{ char *dp, dch, *ap, specialbuf[256];
  int aftercallchars = pp_ebuftop-pp_ebufptr, hashflag = 0;
  if (debugging(DEBUG_PP))
    fprintf(stderr, "aftercall chars %d'%.*s'\n", aftercallchars,
                    aftercallchars, pp_ebufptr);
  /* first copy stuff in ebuf after the macro call to after last actual */
  pp_wrbuf(pp_ebufptr, aftercallchars);
  pp_ebufptr = pp_ebuftop = pp_ebufbase + PP_UNRDCHMAX;
  dp = pp_hashismagic_(p) ? pp_special(pp_hashmagic_(p), specialbuf)
                          : pp_hashbody_(p);
  while ((dch = *dp) != 0) switch (dch)
  { default:
        if (pp_cidchar(dch))
        { int i = 0;
          PP_ARGENTRY *a = 0;
          do i++, dp++; while (pp_cidchar(*dp));
          if (!pp_hashnoargs_(p))
            for (a = pp_hasharglist_(p); a != 0; a = pp_argchain_(a))
              if (pp_eqname(pp_argname_(a), dp-i, i)) break;
          if (hashflag==1) pp_savch('"');
          if (a == 0) pp_savbuf(dp-i, i);             /* not an arg */
          else for (ap = pp_abufbase+pp_argactual_(a); *ap != 0; )
          { if (hashflag==1 && *ap == '"')
            { int i = pp_spanstring(ap);
              pp_savch('\\');           /* quote begin " with \ */
              pp_savbuf(ap,  i-1);      /* leave off trailing " */
              pp_savbuf("\\\"", 2);     /* quote final " with \ */
              ap += i;
            }
            else pp_savch(*ap++);
          }
          if (hashflag==1) pp_savch('"');
        }
        else pp_savch(*dp++);
        hashflag = 0;
        break;
    case '\'':
    case '"':
        { int i = pp_spanstring(dp);
          pp_savbuf(dp, i);
          dp += i;
        }
        hashflag = 0;
        break;
    case '#': hashflag++; dp++; break;
  }
  while (nlsinargs-- > 0)
  { pp_savch('\n');        /* save up the NL's in args... */
    pp_xlinect--;          /* and set to recount them again (after reread) */
  }
  pp_savbuf(pp_abufptr-aftercallchars, aftercallchars);
  pp_abufptr = pp_abufbase;   /* clear for the rescan */
  pp_nsubsts++;
}

static void pp_trimarg(abufarg)
char *abufarg;
{   while (pp_abufptr != abufarg && pp_abufptr[-1] == ' ')
    pp_abufptr--;   /* trim off trailing spaces */
}

static PP_HASHENTRY *pp_lookup(name,hash)
char *name; int hash;
{   PP_HASHENTRY *p;
    for (p = (*pp_hashtable)[hash % PP_HASHSIZE]; p != 0; p = pp_hashchain_(p))
        if (strcmp(pp_hashname_(p),name) == 0 && pp_hashalive_(p)) break;
    return p;
}

/* pp_checkid reads in a source id to see if it is a macro name, either
   outputting it unchanged or expanding it.  BEWARE - reuse of abuf
*/
static void pp_checkid(pp_ch)
int pp_ch;
{ PP_HASHENTRY *p;
  PP_ARGENTRY *a;
  int i = 0, hash = 0;
  int uselinect, parcnt = 0;
  int abufarg;   /* now offset into pp_abufbase */
  do { if (i<PP_DEFLEN)
       { hash = (((hash * 39) + pp_ch) & 0xffffff);
         pp_abufbase[i++] = pp_ch;
       }
       pp_ch = pp_rdch();
     } while pp_cidchar(pp_ch);
  pp_abufbase[i] = 0;
  if (pp_ch == PP_NOEXPAND) pp_ch = pp_rdch(), p = 0;  /* see PP_NOEXPAND */
  else
    p = pp_lookup(pp_abufbase, hash);
  if (p == 0)
  { if (strcmp("defined",pp_abufbase) != 0 || !pp_inhashif)
    { pp_abufptr = pp_abufbase + i;     /* leave chars in output buffer */
      pp_unrdch(pp_ch);
      return;
    }
    else                   /* poxy "defined id" or "defined(id)" in #if */
    { int parens = 0;
      pp_skipb1(pp_ch);
      if (pp_ch == '(') { parens = 1; pp_skipb0(pp_ch); }
      if (!pp_cidchar(pp_ch)) pp_err("No identifier after #if defined");
      else
      { i = 0, hash = 0;
        do { if (i<PP_DEFLEN)
                 hash = (((hash * 39) + pp_ch) & 0xffffff),
                 pp_abufbase[i++] = pp_ch;
             pp_ch = pp_rdch();
           } while pp_cidchar(pp_ch);
        pp_abufbase[i] = 0;
        p = pp_lookup(pp_abufbase, hash);
        if (parens)
        { pp_skipb1(pp_ch);
          if (pp_ch == ')'); /* nothing to do */
          else pp_err("No ')' after #if defined(...");
        }
        else pp_unrdch(pp_ch);
      }
      pp_abufptr = pp_abufbase;
      pp_expand(p != 0 ? pp_hashone : pp_hashzero, 0);
      return;
    }
  }
  pp_hashuses_(p)++;
  abufarg = 0, pp_abufptr = pp_abufbase;
  if (pp_hashnoargs_(p))           /* macro with no parameters */
  { pp_unrdch(pp_ch);
    pp_expand(p,0);
    return;
  }
  a = pp_hasharglist_(p);          /* macro with parameters */
  uselinect = pp_linect;
  pp_skipb1(pp_ch);
  if (pp_ch != '(')                /* ANSI says ignore if no '(' present */
  { pp_abufptr = pp_abufbase + i;     /* leave chars in output buffer */
    pp_wrch(' ');                     /* do not concatenate - @@@ not nec. */
    pp_unrdch(pp_ch);
    return;
  }
  for (pp_ch = pp_rdch();;)        /* read args */
  { switch (pp_ch)
    { case EOF: pp_err("Missing ')' after %s(... on line %d",
                        pp_hashname_(p), uselinect);
                return;
      case '\n': pp_xlinect++;  /* drop through - treat nl in arg as space  */
      case '\t': pp_ch = ' ';   /* drop through - treat tab in arg as space */
      case ' ':  if (pp_abufptr == pp_abufbase+abufarg) break;
                                /* ignore leading spaces                    */
                 /* drop through */
      default:  pp_wrch(pp_ch); break;
      case '\'':
      case '"': pp_copystring(pp_ch); break;
      case '(': parcnt++; pp_wrch(pp_ch); break;
      case ',': if (parcnt > 0) { pp_wrch(pp_ch); break; }
                pp_trimarg(pp_abufbase+abufarg);
                pp_wrch(0);
                if (a != 0) pp_argactual_(a) = abufarg,
                            abufarg = pp_abufptr - pp_abufbase,
                            a = pp_argchain_(a);
                if (a == 0)
                { pp_err("Too many arguments to macro %s(... on line %d",
                          pp_hashname_(p), uselinect);
                  return;
                }
                break;
      case ')': if (parcnt-- > 0) { pp_wrch(pp_ch); break; }
                pp_trimarg(pp_abufbase+abufarg);
                if (a == 0 && pp_abufptr != pp_abufbase+abufarg)
                {   /* no tokens allowed as arg in calls of #define f() ... */
                    pp_err("Too many arguments to macro %s(... on line %d",
                           pp_hashname_(p), uselinect);
                    return;
                }
                pp_wrch(0);
                if (a != 0) pp_argactual_(a) = abufarg,
                            abufarg = pp_abufptr - pp_abufbase,
                            a = pp_argchain_(a);
                if (a != 0)
                { pp_err("Too few arguments to macro %s(... on line %d",
                          pp_hashname_(p), uselinect);
                  return;  /* improve by defaulting them to null */
                }
                pp_expand(p, pp_linect-uselinect);
                return;
    }
    pp_ch = pp_rdch();
  }
}

static PP_HASHENTRY *pp_predefine2(s,n)
char *s; int n;
{ PP_HASHENTRY *p = pp_new_(PP_NOARGHASHENTRY);
  int i = 0, hash = 0, ch;

  pp_hashuses_(p) = 0;
  pp_hashalive_(p) = 1;
  pp_hashnoargs_(p) = 1;
  pp_hashismagic_(p) = n < 0;
  if (n < 0) pp_hashmagic_(p) = n; else pp_hashbody_(p) = "1";
  pp_hashdefchain_(p) = 0;
  while (ch = *s++, n<0 ? ch!=0 : pp_cidchar(ch))
     {
       if (i<PP_DEFLEN)
       { hash = (((hash * 39) + ch) & 0xffffff);
         pp_stuffid_(ch);
       }
     }
  pp_hashname_(p) = pp_closeid();
  switch (ch)
  {  default:  if (n >= 0)
                   pp_rerr("illegal option -D%s%s", pp_hashname_(p), s-1);
               /* drop through */
     case 0:   break;
     case '=': pp_hashbody_(p) = s; break;
  }
  pp_hashchain_(p) = (*pp_hashtable)[hash % PP_HASHSIZE];
  (*pp_hashtable)[hash % PP_HASHSIZE] = p;
  if (pp_hashfirst == 0) pp_hashfirst = pp_hashlast = p;
  else pp_hashdefchain_(pp_hashlast) = p, pp_hashlast = p;
  return p;
}

static PP_ARGENTRY *pp_addtoarglist(id)
char *id;
{ PP_ARGENTRY *p = pp_new_(sizeof(PP_ARGENTRY));
  pp_argchain_(p) = 0;
  pp_argname_(p) = id;
  return p;
}

static void pp_define() /* free pp_ch */
{ int i = 0, hash = 0;
  pp_skipb1(pp_ch);
  if (!pp_cidchar(pp_ch))
  { pp_err("Missing identifier after #define"); pp_skipl1(pp_ch); return; }
  if (pp_skipping) { pp_skipl1(pp_ch); return; }   /* @@@ untidy */
  do { if (i<PP_DEFLEN)
       { hash = (((hash * 39) + pp_ch) & 0xffffff);
         pp_stuffid_(pp_ch);
         i++;
       }
       pp_ch = pp_rdch();
     } while (pp_cidchar(pp_ch));
  { char *name = pp_closeid();
    PP_HASHENTRY *p;
    if (pp_ch != '(')
      p = pp_new_(PP_NOARGHASHENTRY), pp_hashnoargs_(p) = 1;
    else
    { int params = 0;
      PP_ARGENTRY *last;
      p = pp_new_(sizeof(PP_HASHENTRY));
      pp_hashnoargs_(p) = 0;
      pp_hasharglist_(p) = 0;
      do
       { pp_skipb0(pp_ch);
         if (!pp_cidchar(pp_ch))
         { if (pp_ch == ')' && params == 0) break;
           pp_err("Missing parameter name in #define %s(...", pp_hashname_(p));
           pp_skipl1(pp_ch);
           return;
         }
         do { pp_stuffid_(pp_ch); pp_ch = pp_rdch();
            } while (pp_cidchar(pp_ch));
         if (pp_hasharglist_(p) == 0)
             last = pp_hasharglist_(p) = pp_addtoarglist(pp_closeid());
         else last = pp_argchain_(last) = pp_addtoarglist(pp_closeid());
         params++;
         pp_skipb1(pp_ch);
       } while (pp_ch == ',');
      if (pp_ch == ')') pp_ch = pp_rdch();
      else { pp_err("Missing ',' or ')' after #define %s(...", pp_hashname_(p));
             pp_skipl1(pp_ch);
           }
    }
    pp_hashalive_(p) = 1;
    pp_hashismagic_(p) = 0;
    pp_hashuses_(p) = 0;
    pp_hashdefchain_(p) = 0;
    pp_hashname_(p) = name;
    pp_skipb1(pp_ch);
    for (;;)
    {   switch (pp_ch)
        {
    default:     if (pp_cidchar(pp_ch))
                 {   int n = 0;
                     do n++, pp_stuffid_(pp_ch), pp_ch = pp_rdch();
                       while (pp_cidchar(pp_ch));
                     /* check for seemingly recursive call (see PP_NOEXPAND) */
                     if (pp_eqname(pp_hashname_(p), pp_dbufptr-n, n))
                       pp_stuffid_(PP_NOEXPAND);  /* inhibit later expansion */
                     continue;
                 }
                 pp_stuffid_(pp_ch);
                 break;
    case '\'':
    case '"':    pp_stuffstring(pp_ch,0);
                 break;
    case ' ':
    case '\t':   /* turn multiple spaces to one and remove all before '##' */
                 /* swallow also before NL/EOF                             */
                 pp_skipb0(pp_ch);
                 if (pp_ch == '\n' || pp_ch == EOF) continue;
                 if (pp_ch != '#') { pp_stuffid_(' '); continue; }
                 pp_stuffid_('#');
                 pp_ch = pp_rdch();
                 if (pp_ch != '#') continue;
                 pp_stuffid_('#');
                 pp_skipb0(pp_ch);
                 continue;
    case '\n':
    case EOF:
            pp_hashbody_(p) = pp_closeid();
            { PP_HASHENTRY *q = pp_lookup(pp_hashname_(p), hash);
              if (q)
              { pp_hashalive_(q) = 0;   /* omit for a #define def. stack */
                pp_warn("Re-definition of #define macro %s",
                           pp_hashname_(p));
              }
            }
            pp_hashchain_(p) = (*pp_hashtable)[hash % PP_HASHSIZE];
            (*pp_hashtable)[hash % PP_HASHSIZE] = p;
            if (pp_hashfirst == 0) pp_hashfirst = pp_hashlast = p;
            else pp_hashdefchain_(pp_hashlast) = p, pp_hashlast = p;
            return;  /* gasp */
        }
        pp_ch = pp_rdch();
    }
  }
}

static void pp_undef() /* free pp_ch */
{ int i = 0, hash = 0;
  pp_skipb1(pp_ch);
  if (!pp_cidchar(pp_ch))
  { pp_err("Missing identifier after #undef"); pp_skipl1(pp_ch); return; }
  do { if (i<PP_DEFLEN)
       { hash = (((hash * 39) + pp_ch) & 0xffffff);
         pp_abufbase[i++] = pp_ch;
       }
       pp_ch = pp_rdch();
     } while (pp_cidchar(pp_ch));
  pp_abufbase[i++] = 0;
  if (!pp_skipping)
  { PP_HASHENTRY *p = pp_lookup(pp_abufbase, hash);
    if (p) pp_hashalive_(p) = 0;
  }
}

static void pp_addconditional(skipelsepart)
int skipelsepart;
{ PP_IFSTACK *q = pp_freeifstack;
  if (q) pp_freeifstack = pp_ifchain_(q);
  else q = pp_new_(sizeof(PP_IFSTACK));
  pp_ifchain_(q) = pp_ifstack;
  pp_ifseenelse_(q) = 0;
  pp_ifoldskip_(q) = pp_skipping;                      /* caller pp_skipping */
  pp_ifskipelse_(q) = pp_skipping || skipelsepart;     /* 'else' part */
  pp_ifstack = q;
  pp_skipping = pp_skipping || !skipelsepart;          /* 'then' part */
}

static void pp_h_ifdef(skipelsepart) /* free pp_ch */
int skipelsepart;
{ PP_HASHENTRY *p;
  int i = 0, hash = 0;
  pp_skipb1(pp_ch);
  if (!pp_cidchar(pp_ch))
  { pp_err("Missing identifier after #ifdef"); pp_skipl1(pp_ch); return; }
  do { if (i<PP_DEFLEN)
       { hash = (((hash * 39) + pp_ch) & 0xffffff);
         pp_abufbase[i++] = pp_ch;
       }
       pp_ch = pp_rdch();
     } while (pp_cidchar(pp_ch));
  pp_abufbase[i++] = 0;
  p = pp_lookup(pp_abufbase, hash);
  if (p != 0 && !pp_skipping) pp_hashuses_(p)++;
  pp_addconditional((p != 0) == skipelsepart);
}

static void pp_h_if() /* free pp_ch */
{   pp_unrdch(pp_ch);
#ifndef PP_STANDALONE
    {   int oskip = pp_skipping;
        long int b;
        pp_inhashif = (!oskip) ? 2 : 1; pp_skipping = 0;
        b = syn_hashif();            /* always read to check syntax */
        pp_inhashif = 0; pp_skipping = oskip;
        pp_addconditional(b);
        pp_ch = '\n';   /* @@@ rather nasty */
    }
#else
    pp_err("Unimplemented directive #if (use #ifdef), treated as #if 0");
    pp_addconditional(0);
    pp_skipl0(pp_ch);
#endif
}

static void pp_h_else()
{ if (pp_ifstack == 0 || pp_ifseenelse_(pp_ifstack))
      pp_rerr("Spurious #else ignored");
  else { pp_skipping = pp_ifskipelse_(pp_ifstack);
         pp_ifseenelse_(pp_ifstack) = 1; }
}

static void pp_h_elif()
{   pp_unrdch(pp_ch);
    if (pp_ifstack == 0 || pp_ifseenelse_(pp_ifstack))
    {   pp_rerr("Spurious #elif ignored");
        pp_skipl0(pp_ch);
        return;
    }
#ifndef PP_STANDALONE
    {   int oskip = pp_skipping;
        long int b;
        pp_inhashif = (oskip && !pp_ifskipelse_(pp_ifstack)) ? 2 : 1;
            pp_skipping = 0;
        b = syn_hashif();            /* always read to check syntax */
        pp_inhashif = 0; pp_skipping = oskip;
        if (b)
            pp_skipping = pp_ifskipelse_(pp_ifstack),
            pp_ifskipelse_(pp_ifstack) = 1;
        else
            pp_skipping = 1;
        pp_ch = '\n';   /* @@@ rather nasty */
    }
#else
    pp_err("Unimplemented directive #elif, treated as #elif 1");
    pp_skipping = pp_ifskipelse_(pp_ifstack);
    pp_skipl0(pp_ch);
#endif
}

static void pp_h_endif()
{ if (pp_ifstack == 0) pp_rerr("Spurious #endif ignored");
  else { pp_skipping = pp_ifoldskip_(pp_ifstack);
         { PP_IFSTACK *q = pp_ifchain_(pp_ifstack);
           /* discard old */
           pp_ifchain_(pp_ifstack) = pp_freeifstack,
           pp_freeifstack = pp_ifstack;
           pp_ifstack = q;
         }
       }
}

static void pp_h_line() /* free pp_ch */
{ int n = 0; char *file;
  pp_skipb1(pp_ch);
  if (isdigit(pp_ch))
  { while (isdigit(pp_ch)) n = n*10 + pp_ch-'0', pp_ch = pp_rdch();
    if (!pp_skipping)
      pp_linect = n-1;   /* we have yet to count the NL at the end */
    pp_skipb1(pp_ch);
  }
  if (pp_ch == '"')
  { pp_stuffstring('"',1);
    file = pp_closeid();    /* ensure done always */
    if (!pp_skipping) pp_cisname = file;
    pp_ch = pp_rdch();
  }
}

static void pp_include() /* free pp_ch */
{ char lquote, rquote, *fname;
  FILE *fp;
  pp_skipb1(pp_ch);
  switch (lquote = pp_ch)
  { case '"': rquote = '"'; break;
    case '<': rquote = '>'; break;
    default:  pp_err("Missing '<' or '\"' after #include");
              pp_skipl1(pp_ch);
              return;
  }
  /* this next line needs work to buffer input and expand any macros */
  pp_stuffstring(rquote,1);
  fname = pp_closeid();    /* ensure done always */
  pp_skipb0(pp_ch);
  if (!PP_EOLP(pp_ch) && pp_ch != EOF)
  { pp_err("Junk after #include %c%s%c", lquote, fname, rquote);
    pp_skipl1(pp_ch);
  }
  if (pp_skipping) return;
  if ((fp = pp_inclopen(fname,lquote=='<')) != NULL)
  /* the following block is notionally a recursive call to pp_process()
     but that would mean a co-routine structure if used with the cc. */
  { PP_FILESTACK *fs = pp_freefilestack;
    if (fs) pp_freefilestack = pp_filchain_(fs);
    else fs = pp_new_(sizeof(PP_FILESTACK));
    pp_filchain_(fs) = pp_filestack;
    pp_filech_(fs) = pp_ch;         /* \n or EOF */
    pp_filstream_(fs) = pp_cis;     pp_cis = fp;
    pp_filename_(fs) = pp_cisname;  pp_cisname = fname;
    pp_fileline_(fs) = pp_linect;   pp_linect = 0;
    pp_filestack = fs;
  }
  else
  { pp_err("#include file %c%s%c wouldn't open", lquote,fname,rquote);
  }
}

/* Pragmas: syntax allowed (we can argue more later) is:
   "#pragma -<letter><optional digit> ... -<letter><optional digit>".
   The effect of "#pragma -<letter>" is to set pp_pragmavec[letter-a] to -1
   and "#pragma -<letter><digit>" to set pp_pragmavec[letter-a] to <digit>-0.
*/

static void pp_pragma() /* free pp_ch */
{ int i;
  /* note that ANSI say it is NOT an error to fail to parse a #pragma */
  /* that does not stop us warning on syntax we fail to recognise     */
  if (pp_skipping) return;
  for (;;)
  { pp_skipb1(pp_ch);
    switch (pp_ch)
    { default:   pp_warn("Unrecognised #pragma (no '-')"); return;
      case '\n': return;
      case '-':  pp_ch = pp_rdch();
                 if (isupper(pp_ch)) i = pp_ch - 'A';
                 else if (islower(pp_ch)) i = pp_ch -'a';
                 else { pp_warn("Unrecognised #pragma -%c", pp_ch); return; }
                 pp_ch = pp_rdch();
                 if ('0' <= pp_ch && pp_ch <= '9')
                      pp_pragmavec[i] = pp_ch - '0', pp_ch = pp_rdch();
                 else pp_pragmavec[i] = -1;
                 break;
    }
  }  
}

static void pp_h_error() /* free pp_ch */
{   int n = 0; char msg[256];
    pp_skipb1(pp_ch);
    while (pp_ch != '\n' && pp_ch != EOF)
    {   if (n < 255) msg[n++] = pp_ch == '\t' ? ' ' : pp_ch;
        pp_ch = pp_rdch();
    }
    /* fill in terminator, and continue if last char was space */
    do msg[n] = 0; while (--n >= 0 && msg[n] == ' ');
    if (!pp_skipping)
    {   /* Groan: the ANSI rationale recommends that #error terminates */
        /* compilation, but provides no way of fixup and continue.     */
        /* The following (hopefully) temporary fix provides #pragma -e */
        /* to continue.                                                */
        if (pp_pragmavec['e'-'a'] > 0)
            pp_rerr("#error encountered \"%s\"", msg);
        else
            cc_fatalerr("#error encountered \"%s\"", msg);
    }
}

static void pp_directive() /* free pp_ch */
{ char v[PP_DIRLEN+1];
  int i, unixallowsjunk=0;
  pp_skipb0(pp_ch);
  for (i=0; pp_cidchar(pp_ch); )
  { if (i<PP_DIRLEN) v[i++] = pp_ch;
    pp_ch = pp_rdch();
  }
  v[i] = 0;
  if (strcmp(v, "include") == 0)     pp_include();
  else if (strcmp(v, "define") == 0) pp_define();
  else if (strcmp(v, "undef") == 0)  pp_undef();
  else if (strcmp(v, "if") == 0)     pp_h_if();
  else if (strcmp(v, "ifdef") == 0)  pp_h_ifdef(1);
  else if (strcmp(v, "ifndef") == 0) pp_h_ifdef(0);
  else if (strcmp(v, "else") == 0)   pp_h_else(), unixallowsjunk=1;
  else if (strcmp(v, "elif") == 0)   pp_h_elif();
  else if (strcmp(v, "endif") == 0)  pp_h_endif(), unixallowsjunk=1;
  else if (strcmp(v, "line") == 0)   pp_h_line();
  else if (strcmp(v, "pragma") == 0) { pp_pragma(); pp_skipl1(pp_ch); }
  else if (strcmp(v, "error") == 0)  pp_h_error();
  else if (strcmp(v,"") == 0);
  else { pp_err("Unknown directive: #%s", v); pp_skipl1(pp_ch); }
  pp_skipb1(pp_ch);
  if (!PP_EOLP(pp_ch) && pp_ch != EOF)
  {
#ifndef PP_STANDALONE
      if ((suppress & D_PPALLOWJUNK) && unixallowsjunk) xrecovercount++;
      else
#endif
      pp_rerr("junk at end of #%s line - ignored", v);
      pp_skipl1(pp_ch);
  }
  if (pp_ch != EOF) { pp_xlinect++; pp_wrch('\n'); }
}

/* pp_process() behaves like _filbuf in stdio.
   Analogously it could collect larger bits of text before returning.
   But beware the interaction via recursive calls via #if.
*/

/* pp_lastch really keeps a record of a preceding newline (or file start) */
/* for #<directive>.  Note that it is not updated on <space> or <tab>.    */
static int pp_lastch;   /* perhaps could be done via unrdch(). */

static int pp_process()
{   int pp_ch;
    pp_abufptr = pp_abufbase;
    while (pp_abufptr == pp_abufbase) /* do {} while really. */
    {   (pp_ch = pp_rdch());
        if (debugging(DEBUG_PP)) fprintf(stderr, "process %d'%c'\n", pp_ch, pp_ch);
        switch (pp_ch)
        {
    case EOF: fclose(pp_cis);    /* see pp_include() */
/* @@@ ANSI do not specify whether #if's must match in a #include file.   */
/* hence only check all #if's closed on real EOF.                         */
              if (pp_filestack == 0)
              { if (pp_ifstack != 0) pp_err("Missing #endif at EOF");
                return EOF;
              }
              pp_ch = pp_filech_(pp_filestack);
              pp_cis = pp_filstream_(pp_filestack);
              pp_cisname = pp_filename_(pp_filestack);
              pp_linect = pp_fileline_(pp_filestack)+1; /* @@@ +1 by NHG 1990 */
							/* AND MJT 1992 !!!!  */

              { PP_FILESTACK *q = pp_filchain_(pp_filestack);
                /* discard old */
                pp_filchain_(pp_filestack) = pp_freefilestack,
                pp_freefilestack = pp_filestack;
                pp_filestack = q;
              }
              pp_unrdch(pp_ch); pp_ch = EOF;  /* so pp_xlinect is right */
              break;
    case '#': if (PP_EOLP(pp_lastch))
              { pp_directive(); pp_ch = '\n'; }
              else if (!pp_skipping) pp_wrch(pp_ch);
              break;
    case '\'':
    case '"': pp_copystring(pp_ch);
              break;
    case ' ':
    case '\t': if (!pp_skipping) pp_wrch(pp_ch);
               pp_ch = pp_lastch;    /* spaces may precede '#' */
               break;
    case '\n': pp_xlinect++; pp_wrch(pp_ch); break;
              /* output nl even if skipping */
    default:  if (!pp_skipping)
              { if (pp_cidchar(pp_ch)) pp_checkid(pp_ch);
                else pp_wrch(pp_ch);
              }
        }
        pp_lastch = pp_ch;
    }
    pp_abufoptr = pp_abufbase;
    return *pp_abufoptr++;
}

/* exported things ...                                                    */

int pp_nextchar()
{
    return ((pp_abufoptr < pp_abufptr) ? *pp_abufoptr++ : pp_process());
}

void pp_predefine(s)
char *s;
{
    (void)pp_predefine2(s,1);
}

void pp_tidyup()
{ int i, argcnt;
  PP_HASHENTRY *p;
  PP_ARGENTRY *a;
#ifndef PP_STANDALONE
  if (feature & FEATURE_PPNOUSE)
      for (p = pp_hashfirst; p != 0; p = pp_hashdefchain_(p))
      { if (pp_hashuses_(p) == 0)
          pp_warn("#define macro '%s' defined but not used", pp_hashname_(p));
      }
#endif
  if (!debugging(DEBUG_PP)) return;
  fprintf(stderr, "%d substitutions\n", pp_nsubsts);
  fprintf(stderr, "Hash table:\n");
  for (i=0; i<PP_HASHSIZE; i++)
    for (p = (*pp_hashtable)[i]; p != 0; p = pp_hashchain_(p))
      { fprintf(stderr, "%d: %s", i, pp_hashname_(p));
        if (!pp_hashnoargs_(p))
        { fprintf(stderr, "(");
          for (a = pp_hasharglist_(p), argcnt = 0; a != 0; a = pp_argchain_(a))
            fprintf(stderr, "%s%s", (argcnt++ == 0 ? "" : ","), pp_argname_(a));
          fprintf(stderr, ")");
        }
        fprintf(stderr, " '%s'%s uses %d\n",
                        (pp_hashismagic_(p) ? "<magic>" : pp_hashbody_(p)),
                        (pp_hashalive_(p) ? "" : " (undef'd)"),
                        pp_hashuses_(p));
      }
}

void pp_init()
{ int i;
  time_t t0 = time(0);
  strncpy(pp_datetime, ctime(&t0), 26-1);   /* be cautious */
  pp_hashtable = pp_alloc(sizeof(*pp_hashtable));
  memclr(pp_hashtable, sizeof(*pp_hashtable));
  pp_hashfirst = pp_hashlast = 0;
  pp_dbufend = pp_dbufseg = pp_dbufptr = 0;
#ifdef PP_STANDALONE
  pbufbase = pbufptr = 0;
#endif
  pp_ebufbase = pp_alloc(PP_EBUFINITSIZ);
  pp_ebufptr = pp_ebuftop = pp_ebufbase + PP_UNRDCHMAX;
  pp_ebufend = pp_ebufbase + PP_EBUFINITSIZ;
  pp_abufptr = pp_abufoptr = pp_abufbase = pp_alloc(PP_ABUFINITSIZ);
  pp_abufend = pp_abufbase + PP_ABUFINITSIZ;
  pp_peep1cnt = pp_peep2cnt = pp_peep3cnt = 0;
  pp_rdch0last = '\n';
  pp_lastch = '\n';
  pp_linect = 0;
  pp_xlinect = 1;
  pp_nsubsts = 0;
  pp_instring = 0;
  pp_inhashif = 0;
  pp_ifstack = 0; pp_freeifstack = 0; pp_skipping = 0;
  pp_filestack = 0; pp_freefilestack = 0;
/* args, options... */
  for (i=0; i <= 'z'-'a'; i++) pp_pragmavec[i] = -1;
  pp_cis = stdin; pp_cisname = "<stdin>";
  (void)pp_predefine2("__LINE__", PP__LINE);
  (void)pp_predefine2("__FILE__", PP__FILE);
  (void)pp_predefine2("__DATE__", PP__DATE);
  (void)pp_predefine2("__TIME__", PP__TIME);
  (void)pp_predefine2("__STDC__", PP__STDC);
  pp_hashone = pp_predefine2("!!ONE!!",  PP__ONE);  /* for #if defined(...) */
  pp_hashzero = pp_predefine2("!!ZERO!!", PP__ZERO);
  (void)pp_predefine2("__CC_NORCROFT", PP__ONE);
}


/* The rest of this file is only compiled in a stand-alone pre-processor */

#ifdef PP_STANDALONE

/* machine dependencies... */

#ifdef ARM
#  define PP_SYSAREA ":MEM.,$.ARM.CLIB."
                               /* default directories for system includes */
#  define PP_USERAREA "@."     /* default directories for user includes   */
#  define PP_ROOTEDFILE(file) (file[0] == '$' || file[0] == ':')
#  define PP_DIRCHAR '.'
#endif
#ifdef ibm370
#  define PP_SYSAREA ":,sys"    /* default pds's for system includes is sysh */
#  define PP_USERAREA ","       /* default pds's for user includes   is h    */
#  define PP_DIRCHAR ':'
#  define PP_ROOTEDFILE(file) 0
#endif

#define FILE_EXTENSION 'c'

static char *pp_sysarea, *pp_userarea;

static int swapround(s, c)
char *s; int c;
{   /* changes abc.def.h to h.abc.def for brazil-like things */
    int i = strlen(s);
    if (i >= 3 && s[i-2] == '.' && tolower(s[i-1]) == c)
    {   int j;
        for (j = i-1; j >= 2; j--) s[j] = s[j-2];
        s[0] = c;
        s[1] = PP_DIRCHAR;
        return 1;
    }
    return 0;
}

static FILE *pp_inclopen(file, systemheader)
char *file; int systemheader;
{   char *area = systemheader ? pp_sysarea : pp_userarea;
    if (PP_ROOTEDFILE(file)) area = ",";  /* go once round loop below */
    while (*area)
    {   char dir[256], ch;
        int n = 0;
        while ((ch = *area) != 0 && (area++, ch != ','))
          if (n<255) dir[n++] = ch;
        if (n+strlen(file) > 255) continue;
        strcpy(dir+n, file);
        if (debugging(DEBUG_PP)) fprintf(stderr, "File %s(%s)", dir, dir+n);
        {   FILE *fp;
    /* try to swap round a possible .c and .h suffix (or .p for Pascal)  */
            if (!swapround(dir+n, 'h')) swapround(dir+n, FILE_EXTENSION);
            if (debugging(DEBUG_PP)) fprintf(stderr, "=> %s", dir);
            if ((fp = fopen(dir,"r")) != 0)
            { if (debugging(DEBUG_PP)) fprintf(stderr, " found\n");
              return fp;
            }
        }
        if (debugging(DEBUG_PP)) fprintf(stderr, " failed\n");
    }
    return 0;    /* tough ched */
}

/* end of machine dependencies */

int main(argc,argv)
int argc; char *argv[];
{ int i, nfiles = 0;
  char *ap;
  fprintf(stderr, PP_HERALD);
  pp_sysarea = PP_SYSAREA; pp_userarea = PP_USERAREA;
  pp_debugging = 0;
  pp_init();
  for (i=1; i<argc; i++)
  { ap = argv[i];
    if (*ap++ == '-') switch (*ap++)
    { case 'q': case 'Q': pp_debugging = 1; break;
      case 'd': case 'D': pp_predefine(ap); break;
      case 'i': case 'I': pp_userarea = ap; break;
      case 'j': case 'J': pp_sysarea = ap; break;
    }
    else switch (++nfiles)
    { case 1:
        if ((pp_cis = fopen(pp_cisname=argv[i],"r")) == 0) pp_die("No input file");
        break;
      case 2:
        if (freopen(argv[i],"w",stdout) == 0) pp_die("Can't write output file");
        break;
      default: pp_die("Too many file args");
    }
  }
  while ((i = pp_nextchar()) != EOF) putchar(i);
  pp_tidyup();
  return 0;
}
#endif

/* end of pp.c */
