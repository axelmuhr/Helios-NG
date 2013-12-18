/*
 * misc.c: Misc features for the Norcroft C compiler
 * Copyright (C) Codemist Ltd, 1988.
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/11/23 16:41:52 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char misc_version[];
char misc_version[] = "\nmisc.c $Revision: 1.1 $ 58\n";
#endif

/* Find cc_err below for discussion on error messages, and errors.h   */

#include <stddef.h>
#ifdef __STDC__
#  include <stdlib.h>
#  include <stdarg.h>
#  include <string.h>
#else
#  include <varargs.h>
#  include <strings.h>
#endif
#include <ctype.h>

#include "globals.h"
#ifndef _defs_LOADED
#  include "defs.h"
#endif
#include "store.h"
#include "aeops.h"
#include "aetree.h"
#include "lex.h"               /* for curlex... */
#include "util.h"

#define DEFINE_ERROR_COMPRESSION_TABLE 1

#include "errors.h"

#undef DEFINE_ERROR_COMPRESSION_TABLE


#ifndef __STDC__
/* host.h does this if bsd is set, so maybe it is not needed here too */
#define sprintf ansi_sprintf
#endif
#ifndef _vsprintf
#define _vsprintf vsprintf  /* should be in host.h probably */
#endif

int32 pp_linect;
int32 pp_pragmavec['z'-'a'+1];
char *pp_cisname;
int32 sysdebugmask, suppress, feature;
FILE *listingstream;
#ifdef PASCAL /*ECN*/
int32 rtcheck;
#endif

int32 length(List *l)
{
    int32 i = 0;
    for (; l != NULL; l = cdr_(l)) i++;
    return i;
}

List *dreverse(List *x)
{
  List *y = 0, *t;


  while (x != NULL)
    {
      t = cdr_(x);
      cdr_(x) = y;
      y = x;
      x = t;      
    }
    
  return y;
}

List *nconc(List *x, List *y)
/* Destructively appends y to x, turning x back to front as it does so. */
{
    if (x == 0) return y;
    else
    {   List *t, *a = x;
        while ((t = cdr_(x)) != 0) x = t;
        cdr_(x) = y;
        return a;
    }
}

int32 max(int32 a, int32 b)
{
    return (a>b ? a : b);
}

int32 bitcount(int32 n)
{
/* return the number of bits present in the integer n.                   */
    int32 r = 0;
    while (n!=0) n ^= n & (-n), r++;
    return(r);
}

/* 'real' in the following represents generic float/double/long double */
/* real_of_string() uses GLOBAL store for its value.  Is this reasonable? */
/* special case flag==0 for double_to_double only */
FloatCon *real_of_string(const char *s, int32 flag)
{   int32 wsize = offsetof(FloatCon,floatstr[0]) + padstrlen(strlen(s));
    FloatCon *x = (FloatCon *) GlobAlloc(SU_Const, wsize);
    /* real_of_string is now also used to initialise fc_two_32. */
    x->h0 = s_floatcon;
    strcpy(x->floatstr, s);
    if (flag)
    {   x->floatlen = flag;       /* the bitoftype_ / curlex.a2.flag pun!!! */
        fltrep_stod(s, &x->floatbin.db); /* double = long double for us...  */
        if (flag & bitoftype_(s_short))  /* so we only need to narrow once  */
            fltrep_narrow_round(&x->floatbin.db, &x->floatbin.fb);
    }
    return x;
}

/* the next two routines do not really belong here, but they do manipulate
   the same structure as make_floating */

FloatCon *real_to_real(FloatCon *fc, SET_BITMAP m)
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
    if ((m & bitoftype_(s_short)) && !(fc->floatlen & bitoftype_(s_short)))
        fltrep_narrow(&fc->floatbin.db, &x->floatbin.fb);
    else if (!(m & bitoftype_(s_short)) &&
              (fc->floatlen & bitoftype_(s_short)))
        fltrep_widen(&fc->floatbin.fb, &x->floatbin.db);
    return x;
}

FloatCon *int_to_real(int32 n, int32 u, SET_BITMAP m)
{   char s[20];
    /* there must be better ways - e.g. fltrep_itod in fltrep.c ??       */
    /* The use of sprintf/real_of_string ensures that the floatcon that  */
    /* is generated has a sensible string attached to it so leave this   */
    /* code alone - it works quite well enough.                          */
    if (u) sprintf(s, "%lu.0", (long)n);
    else sprintf(s, "%ld.0", (long)n);
    return real_of_string(s,m);
}

/* error message routines...
   0) cc_msg:  Internal compiler messages, enabled by DEBUG_xxx flags.
   1) cc_warn:  perfectly legal, but curious, C program (e.g. = for ==),
                or 'undefined at run time' (e.g. printf("%f", 1);).
   2)           Offences againgst ANSI draft (Return normally).
   2') cc_pccwarn: Recoverable, only a warning if in pcc mode.
   2a) cc_rerr:   Recoverable error without loss of code (e.g. int *x=1;)
                  Code 'works' -- compiled as on UNIX.
   2b) cc_err:    Code probably lost, e.g. syntax error, or
                       { struct {int a;}*x; f(x->b); }
                  Sets 'compilation failed' flag.
   3) cc_extension:  ditto, but allows controlled extension use.
   3) cc_fatalerr: A cause for giving up compilation (e.g. out of store/
                   too many errors).  NEVER returns.
   4) syserr:   Internal consistency error.  Exits with legs in air,
                unless debugging (syserr_behaviour) flag set.
                N.B. May return for system debuggers only.
   Note all messages (except class 0 and 4) must now be in errors.h (q.v.).
*/

int32 warncount=0, recovercount=0, errorcount=0;
/* The following 2 vars count suppressed errors/warns.  They needn't    */
/* be printed on systems where this would be unusual.                   */
int32 xwarncount=0;
int32 syserr_behaviour = 0;
/* The following variables should only be ref'd if listingstream != NULL */
static char *errsaves;
static int32 errsaven;
static int32 errsavep;

static struct uncompression_record
{
    char *pointer;
    char compressed;
    unsigned char height;
    char stack[MAXSTACKDEPTH];
} errmsg;

#ifdef COMPRESSED_ERROR_MESSAGES

static void start_string_char(char *s) /* Compressed but still a string */
{
    errmsg.height = 0;
    errmsg.pointer = s;
    errmsg.compressed = 1;
}

static int fetch_string_char()
/*
 * This is the same code (pretty well) as pp_fetch_string_char() in
 * "pp.c", but having a separate version here keeps the module structure
 * of the compiler cleaner and only spends about 150 bytes.  It also
 * reserves the option of using different compression techniques for
 * error messages and built-in headers.
 */
{
    int c, k;
    if (errmsg.height == 0) c = *errmsg.pointer++;
    else c = errmsg.stack[--errmsg.height];
    for (;;)
    {   c &= 0xff;
        k = ecompression_info[c];
        if (k == c || errmsg.compressed == 0) return c;
/*
 * The genhdrs utility establishes the greatest possible depth needed in
 * this stack and arranges to define MAXSTACKDEPTH suitably - thus no
 * run-time check for stack overflow is needed.
 */
        errmsg.stack[errmsg.height++] = k;
        c = k >> 8;
    }
}

static void unfetch_string_char(int ch)
{
    errmsg.stack[errmsg.height++] = ch;
}

#else

static void start_string_char(char *s)
{
    errmsg.pointer = s;
}

static int fetch_string_char()
{
    return *errmsg.pointer++;
}

static void unfetch_string_char(int ch)
{
    errmsg.pointer--;
    IGNORE(ch);
}

#endif

static void sstart_string_char(char *s)
{
    errmsg.height = 0;
    errmsg.pointer = s;
    errmsg.compressed = 0;
}

static void nprintf(char *errcode, ...)
{
    va_list a;
    char s[80];
    char *p = s;
    start_string_char(errcode);
/*
 * Convert error code into a string so that it can be printed. I expect
 * all strings used to be less than 80 characters long.
 */
    while ((*p++ = fetch_string_char()) != 0);
    va_start(a, errcode);
    _vfprintf(stderr, s, a);
    va_end(a);
}

void summarise(void)
{
    if (warncount || recovercount || errorcount || (feature & FEATURE_VERBOSE))
    {
        nprintf(misc_message_sum1(pp_cisname, (long)warncount,warncount==1));
        if (xwarncount && !(feature & FEATURE_PCC))
            nprintf(misc_message_sum2, (long)xwarncount);
        nprintf(misc_message_sum3((long)recovercount,recovercount==1));
        nprintf(misc_message_sum5((long)errorcount,errorcount==1));
    }
}

static void check_error_buffer(void)
{
    if (errsaves == NULL || errsavep > errsaven - 200)
    {   errsaves = (char *)realloc(errsaves, (size_t)(errsaven += 1024));
        if (errsaves == NULL)
        {   fclose(listingstream);
            listingstream = NULL;
            cc_fatalerr(misc_fatalerr_space1);
        }
    }
}

static void announce(char *reason, int32 line)
{
    nprintf(misc_message_lineno(pp_cisname, (long)line, reason));
    if (listingstream != NULL)
    {   check_error_buffer();
        if (list_this_file)
             errsavep += (sprintf(&errsaves[errsavep],
                                 misc_message_announce, reason),
                          strlen(&errsaves[errsavep]));
        else errsavep += (sprintf(&errsaves[errsavep],
                                 misc_message_announce1,
                                 pp_cisname, (long)line, reason),
                          strlen(&errsaves[errsavep]));
    }
}

void listing_diagnostics(void)
{
    if (listingstream && errsavep != 0)
    {   fprintf(listingstream, "%s", errsaves);
        errsavep = 0;
    }
}

static void errprintf(char *s, ...)
{
    va_list a;
    va_start(a, s);
    _vfprintf(stderr, s, a);
    va_end(a);
    if (listingstream != NULL)
    {   check_error_buffer();
        va_start(a, s);
        errsavep += _vsprintf(&errsaves[errsavep], s, a);
        va_end(a);
    }
}

static void qprints(char *s)
{   /* used to print symbols (quoted) or syntactic categories */
    if (s[0] == '<' && isalpha(s[1])) errprintf("%s", s);
    else errprintf("'%s'", s);
}

/* Export for more general use soon (e.g. asm.c).                       */
static void esccharname(char *s, int ctarget)   /* s shall be char[>=5] */
{   int c;
    switch (c = char_untranslation(ctarget))
    {
default:
        if (isprint(c)) s[0] = c, s[1] = 0;
        else sprintf(s, "\\x%.2x", ctarget);
        return;
case '\\':
case '\'':
case '\"': break;
case '\a': c = 'a'; break;
case '\b': c = 'b'; break;
case '\f': c = 'f'; break;
case '\n': c = 'n'; break;
case '\r': c = 'r'; break;
case '\t': c = 't'; break;
case '\v': c = 'v'; break;
case 0:    c = '0'; break;
    }
    s[0] = '\\', s[1] = c, s[2] = 0;
}

static void escstring(char *d, int32 dl, char *s, int32 sl)
{   /* Used to print strings escaping chars as needed.                  */
    /* Note that s==DUFF_ADDR is OK if sl==0.                           */
    int32 si, di;
    for (si = di = 0; si<sl && di<dl-8; si++)
    {   esccharname(&d[di], s[si]);
        di += strlen(&d[di]);
    }
    if (si<sl) strcpy(&d[di], "...");
}

/* note that stg_name also gives the names of types since the same bit
   map is used.  Tidy later, see how they are read in rd_declspec().
*/
static char *xstg_name(SET_BITMAP stg)
{  AEop s;
   /* Hack: the next line helps printing 'long int', but not much else.   */
   if (stg & bitoftype_(s_long)) stg &= ~bitoftype_(s_int);
   for (s = s_char; isdeclstarter_(s); s++)
       if (stg & bitofstg_(s)) return sym_name_table[s];
   return "???";
}

static char *xtype_name(TypeExpr *e)
{   switch (h0_(e))
    {   case s_typespec: return xstg_name(typespecmap_(e));
        case t_fnap: return "<function>";
        case t_content: return "<pointer>";
        case t_subscript: return "<array>";
        default: return "???";
    }
}

static void ssuperrprintf(va_list a)
{
    /* This routine behaves like printf but also recognises escape chars of
     * the form $<char> and acts upon them.   It writes its output to
     * stderr, and if listing-file!= NULL also to a buffer associated with
     * the listing file.
     */

    for (;;)
    {
#define no_arg_type     0
#define int_arg_type    2
#define ptr_arg_type    4
#define no_qualifier    0
#define long_qualifier  8
#define star_qualifier 16   /* indirection for field width (yuk) */
        int arg_type = no_arg_type, w;
        void *pnt;
        long l_int;

        int ch;
        char v[80+12];      /* Buffer for segment of the format string */
        int n = 0;          /* 0..91 */

        /* Characters are moved to the buffer up as far as a $ or % escape */
        while ((ch = fetch_string_char()) != 0 &&
               ch != '$' &&
               n != 80 &&       /* also break at char 80... */
               ch != '%') v[n++] = ch;

        if (ch == '%')
        {
/* It is assumed here that there will never be more than 11 characters in */
/* an escape sequence (e.g. %-20.20l#x has 10 chars in it and seems a     */
/* bit excessive. The limit is because of overflow in the buffer v[]      */
            v[n++] = ch;
            for (;;)
            {   ch = fetch_string_char();
                if (n == 91) syserr(syserr_bad_fmt_dir);
                v[n++] = ch;
                switch (safe_tolower(ch))
                {
        case 'l':   arg_type |= long_qualifier;
                    continue;
        case '*':   arg_type |= star_qualifier;
                    continue;
        /* Note that 'h' indicates that an int should be treated as a */
        /* short value, but the va_arg() call still fetches an int.   */
        default:    continue;
        case '%':   break;
        case 'c': case 'd': case 'i': case 'o':
        case 'u': case 'x':
                    arg_type |= int_arg_type;
                    break;
        case 'p': case 's':
                    arg_type |= ptr_arg_type;
                    break;
        case 'e': case 'f': case 'g':   /* disallow floating point here */
        case 'n': case 0:
                    syserr(syserr_bad_fmt_dir);
                }
                break;
            }
            ch = fetch_string_char();
        }

        v[n] = 0;           /* terminate format string */

        if (listingstream != NULL) check_error_buffer();

        switch (arg_type)
        {
    default:
    case no_arg_type:
            if (listingstream != NULL)
                errsavep += (_sprintf(&errsaves[errsavep], v),
        strlen(&errsaves[errsavep]));
            _fprintf(stderr, v);
            break;

    case ptr_arg_type:
            pnt = va_arg(a, void *);
            if (listingstream != NULL)
                errsavep += (_sprintf(&errsaves[errsavep], v, pnt),
        strlen(&errsaves[errsavep]));
            _fprintf(stderr, v, pnt);
            break;

/* At present I am only supporting '*' qualifiers for use with string    */
/* printing (i.e. %.*s) - at the time of writing this code there is just */
/* one such format in errors.h - I might like to get rid of it by        */
/* achieving the same result some other way so that this extra mess here */
/* wrt format decoding could be discarded.                           ACN */
	    
	    /*
	     * XXX - NC - memo: above suggestion would not work as the
	     * %.*s format is used in some of the compiler debugging
	     * statements as well, (cf cfe/pp.c pp_expand()
	     */
	    
    case ptr_arg_type + star_qualifier:
            w   = va_arg( a, int );
            pnt = va_arg( a, void * );
	    
            if (listingstream != NULL)
                errsavep += (_sprintf(&errsaves[errsavep], v, w, pnt),
			     strlen(&errsaves[errsavep]));
	    
            _fprintf(stderr, v, w, pnt);
            break;

    case int_arg_type + long_qualifier:
            l_int = va_arg(a, long);
            if (listingstream != NULL)
                errsavep += (_sprintf(&errsaves[errsavep], v, l_int),
        strlen(&errsaves[errsavep]));
            _fprintf(stderr, v, l_int);
            break;

    case int_arg_type:
            w = va_arg(a, int);
            if (listingstream != NULL)
               errsavep += (_sprintf(&errsaves[errsavep], v, w),
       strlen(&errsaves[errsavep]));
            _fprintf(stderr, v, w);
            break;
        }

        if (ch == 0) return;        /* Message now complete */

        if (ch != '$') unfetch_string_char(ch);
        else switch (ch = fetch_string_char())
        {
    case 0:
            return;
    case 's':
            qprints(sym_name_table[va_arg(a,AEop)]);
            break;
    case 'l':   /* current lexeme */
            switch (curlex.sym)
            {   case s_integer:
                    errprintf("'%ld'", (long)curlex.a1.i); break;
                case s_floatcon:
                    errprintf("'%s'", curlex.a1.fc->floatstr); break;
                case s_identifier:
                    errprintf("'%s'", symname_(curlex.a1.sv)); break;
                case s_string:
                    {   char e[80];
                        escstring(e, 20, curlex.a1.s, curlex.a2.len);
                        errprintf("'\"%s\"'", e);
                    }
                    break;
                default:
                    qprints(sym_name_table[curlex.sym]);
                    break;
            }
            break;
    case 'r':
            {   Symstr *r = va_arg(a, Symstr *);
                if (r==0 || h0_(r) != s_identifier)
                {   if (r == 0) errprintf("<missing>");
                    else errprintf("<oddity>");
                }
                else errprintf("'%s'", symname_(r));
            }
            break;
    case 'b':   /* ordinary binder, but works for tag binder too.       */
            errprintf("'%s'", symname_(va_arg(a,Binder *)->bindsym));
            break;
    case 'c':   /* tag binder -- this will simplify many err msgs.      */
            {   TagBinder *b = va_arg(a,TagBinder *);
                errprintf("'%s %s'",
                          sym_name_table[b->tagbindsort],
                          symname_(b->bindsym));
            }
            break;
    case 'e':
            {   Expr *e = va_arg(a,Expr *);
                while (h0_(e) == s_invisible) e = arg1_(e);
                if (h0_(e) == s_binder) errprintf("'%s'",
                      symname_(((Binder *)e)->bindsym));
                else if (h0_(e) == s_string)
                    {   char s[80];
                        StringSegList *z = ((String *)e)->strseg;
                        escstring(s, 60, z->strsegbase, z->strseglen);
                        errprintf("'\"%s\"'", s);
                    }
    /* improve the next line -- I wonder what the best heuristic is. */
    /* Note that any routines MUST eventually call errprintf, if you */
    /* ever aspire to have listing output as well as stderr.         */
                else errprintf("<expr>");
            }
            break;
    case 't':
            qprints(xtype_name(va_arg(a,TypeExpr *)));
            break;
    case 'm':
            errprintf("'%s'", xstg_name(va_arg(a,SET_BITMAP)));
            break;
    default:
            errprintf("$%c", (int)ch);   /* really an error */
            break;
        }
    }
}

static void superrprintf(char *errorcode, va_list a)
{
    start_string_char(errorcode);
    ssuperrprintf(a);
}

void cc_msg(char *s, ...)
{
    va_list a;
    va_start(a, s);
    sstart_string_char(s); ssuperrprintf(a);
    va_end(a);
}

void syserr(syserr_message_type errorcode, ...)
{
    va_list a;
#ifdef NUMERIC_SYSERR_CODES
    char s[48];
/*
 * syserr codes are listed in the file errors.h, and are just
 * numeric to save space in the compiler.  Furthermore at present the
 * extra args are treated by printing two of them as hex values -
 * if less than two args were passed this will display junk!
 */
    sprintf(s, "maintainer-info(errors.h,%d,%%.8x,%%.8x)", errorcode);
#else
    char *s = errorcode;
#endif
    announce(misc_message_fatal_internal, pp_linect);
    va_start(a, errorcode);
    sstart_string_char(s); ssuperrprintf(a);
    va_end(a);
    errprintf("\n");
    
    switch (syserr_behaviour)
    {   case 1: return;         /* Try to go on with compilation */
        case 2: abort();        /* hard stop - would like diagnostics */
                break;
        default:                /* stop tolerably quietly and tidily */
                va_start(a, errorcode);
                superrprintf(misc_disaster_banner, a);  /* no escapes */
                va_end(a);
                compile_abort(0);
                break;
    }
    exit(EXIT_syserr);
}

void cc_fatalerr_l(int32 n, char *errorcode, va_list a)
{
    announce(misc_message_fatal, n);
    superrprintf(errorcode, a);
    errprintf(misc_message_abandoned);
    if (syserr_behaviour)
    {   show_store_use();
        syserr(syserr_syserr);  /* rethink this msg if syserrs lack strings */
    }
    compile_abort(-1);  /* (-1) is not a signal number used by signal() */
}

void cc_fatalerr(char *errorcode, ...)
{
    va_list a;
    va_start(a, errorcode);
    cc_fatalerr_l(pp_linect, errorcode, a);
    va_end(a);
}

void cc_warn_l(int32 n, char *errorcode, va_list a)
{
    if (!(feature & FEATURE_NOWARNINGS))
    {
        ++warncount;
        announce(misc_message_warning, n);
        superrprintf(errorcode, a);
        errprintf("\n");
    }
}

void cc_warn(char *errorcode, ...)
{
    va_list a;
    va_start(a, errorcode);
    cc_warn_l(pp_linect, errorcode, a);
    va_end(a);
}

void cc_ansi_warn(char *errorcode, ...)
/* called to issue a warning that's suppressed in pcc mode */
{
    va_list a;
    if (feature & FEATURE_PCC) return;
    va_start(a, errorcode);
    cc_warn_l(pp_linect, errorcode, a);
    va_end(a);
}

void cc_pccwarn(char *errorcode, ...)
/* This counts as a warning in pcc mode, but a recoverable error in */
/* ANSI mode.  Hence it should ONLY be called when the compiler     */
/* can claim to repair the construct. In PCC mode ONLY the warning  */
/* can be suppressed using the -w option on the command line.       */
{
    va_list a;
    if (feature & FEATURE_PCC)
    {   if (feature & FEATURE_NOWARNINGS) return;
        ++warncount;
        announce(misc_message_warning, (int32)pp_linect);
    }
    else
    {   ++recovercount;
        announce(misc_message_error, (int32)pp_linect);
    }
    va_start(a, errorcode);
    superrprintf(errorcode, a);
    va_end(a);
    errprintf("\n");
}

void cc_rerr_l(int32 n, char *errorcode, va_list a)
{
    ++recovercount;
    announce(misc_message_error, n);
    superrprintf(errorcode, a);
    errprintf("\n");
}

void cc_rerr(char *errorcode, ...)
{
    va_list a;
    va_start(a, errorcode);
    cc_rerr_l(pp_linect, errorcode, a);
    va_end(a);
}


void cc_ansi_rerr(char *errorcode, ...)
/* called to output an ANSI mode recoverable error mesage */
/*  which is always suppressed in -pcc mode.              */
{
    va_list a;
    if (feature & FEATURE_PCC) return;
    va_start(a, errorcode);
    cc_rerr_l(pp_linect, errorcode, a);
    va_end(a);
}


void cc_err_l(int32 n, char *errorcode, va_list a)
{
    if (++errorcount > 100) cc_fatalerr(misc_fatalerr_toomanyerrs);
    announce(misc_message_serious, n);
    superrprintf(errorcode, a);
    errprintf("\n");
}

void cc_err(char *errorcode, ...)
{
    va_list a;
    va_start(a, errorcode);
    cc_err_l(pp_linect, errorcode, a);
    va_end(a);
}

void err_init(void)
{
    /* reset the following vars for each one of a many file compilation */
    warncount=0, recovercount=0, errorcount=0;
    xwarncount=0;
    errsaves = NULL;
    errsaven = errsavep = 0;
}

/* end of mip/misc.c */
