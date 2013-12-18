#include "fname.h"

#define  NULL       0

#define  COLON     ':'
#define  AMPERSAND '&'
#define  DOLLAR    '$'
#define  DOT       '.'
#define  SLASH     '/'
#define  BACKSLASH '\\'
#define  HAT       '^'
#define  AT        '@'

#define  FNAME_ACORN     1
#define  FNAME_MSDOS     2
#define  FNAME_UNIX      3
#define  FNAME_MACINTOSH 4
#define  FNAME_TYPEMASK  7

#define  ROOTED     FNAME_ROOTED
#define  HASSLASH   16
#define  NOSLASH    32
#define  PSEUDOUNIX 64

#ifndef FNAME_TESTING
#  ifdef __riscos
#    define HOST_IS_ACORN 1
#  endif
#  ifdef __mvs
#    define HOST_IS_MVS   1
#    define HOST_IS_ACORN 1     /* a bit of a (dying) hack for Codemist */
#  endif
#  ifdef __ZTC__
#    define HOST_IS_DOS   1
#  endif
#  ifdef macintosh
#    define HOST_IS_MAC   1
#  endif
#  ifdef __unix
#    define HOST_IS_UNIX  1
#  endif
#  ifdef unix
#    define HOST_IS_UNIX  1
#  endif
#  ifdef _AIX
#    define HOST_IS_UNIX  1
#  endif
#  ifdef __HELIOS
#    define HOST_IS_UNIX  1
#  endif
#endif

#ifdef HOST_IS_ACORN
#  define HOST_FTYPE FNAME_ACORN
#  define DIR_SEP    DOT
#endif
#ifdef HOST_IS_MAC
#  define HOST_FTYPE FNAME_MACINTOSH
#  define DIR_SEP    COLON
#endif
#ifdef HOST_IS_DOS
#  define HOST_FTYPE FNAME_MSDOS
#  define DIR_SEP    BACKSLASH
#endif
#ifdef HOST_IS_UNIX
#  define HOST_FTYPE FNAME_UNIX
#  define DIR_SEP    SLASH
#endif


static void parse_as_unix_or_dos_name(const char *s, UnparsedName *un, int sl)
{   const char *p;

    {   /* Zero/NULLify *un... */
        int l = sizeof(UnparsedName);
        char *p = (char *)un;
        while (l-- > 0) *p++ = 0;
    }

    for (p = s;  *p != 0 && *p != sl;  ++p)
        if (*p == COLON && p != s && un->vol == NULL)
        {   un->vol = s;
            ++p;
            un->vlen = p - s;
            /* strictly, PSEUDOUNIX only if sl == '/'... */
            un->flags |= (*p == sl) ? HASSLASH+PSEUDOUNIX : NOSLASH;
            un->flags |= ROOTED;
            s = p;
            if (*p == 0) break;
        }

    if (*s == sl)
    {   un->flags |= ROOTED+HASSLASH;
        ++s;
    }

    for (un->path = s;  *p != 0;  ++p)
        if (*p == sl) {s = p+1;  un->flags |= HASSLASH;}

    un->root = s;
    if (s[0] == DOT && (s[1] == 0 || s[1] == DOT && s[2] == 0))
        /* Really means 'is a Unix file name'... */
    {   un->flags |= HASSLASH;
        s = p;
        un->plen = s - un->path;
    }
    else if (un->path == s)
        un->path = NULL;
    else
        un->plen = s - un->path - 1;

    un->rlen = p - s;
    while (p != s)
    {   --p;
        if (*p == DOT)
        {   un->extn = p + 1;
            un->elen = un->rlen - (p - s) - 1;
            un->rlen = p - s;
            break;
        }
    }
    un->pathsep = sl;
}

#ifdef HOST_IS_ACORN

static int is_suffix(const char *extn, int elen, const char *suffixes)
{   int l, sch;
    sch = *suffixes++;
    while (sch)
    {   while (sch == ' ') sch = *suffixes++;
        l = 0;
        while (l < elen && extn[l] == sch) {++l;  sch = *suffixes++;}
        /* Assert: l >= elen || *extn[l] != *suffixes */
        if (l >= elen && (sch == ' ' || sch == 0)) return 1;
        while (sch != ' ' && sch != 0) sch = *suffixes++;
    }
    return 0;
}

static void reparse_as_risc_os_name(const char *s, UnparsedName *un,
    const char *suffixes)
{   const char *p = s, *e;

    if (un->flags & NOSLASH)
    {   /* starts volume:... but not volume:/... */
        p += un->vlen;
    }

    if (*p != COLON && *p != AMPERSAND && *p != DOLLAR)
    {   if (*p != HAT && *p != AT && un->flags & HASSLASH)
            /* interpretation as a Unix file name was just fine... */
            return;
        else
            un->flags = 0;
    }
    else
    {   if (*p == COLON)
        {   if (un->vol == NULL) un->vol = s;
            do ++p; while (*p != DOT && *p != 0);
            if (*p == DOT) ++p;
        }
        if (un->vol != NULL) un->vlen = p - un->vol - 1;
        if (*p == AMPERSAND || *p == DOLLAR) p += 2;  /* skip &. or $. */
        un->flags = ROOTED;
    }

    s = e = p;
    for (un->path = p;  *p != 0;  ++p)
        if (*p == DOT) {s = e;  e = p+1;}

    if (s != e && is_suffix(s, e-s-1, suffixes))
    {   un->extn = s;
        un->elen = e - s - 1;
        un->root = e;
        un->rlen = p - e;
    }
    else if (s != e && is_suffix(e, p-e, suffixes))
    {   un->extn = e;
        un->elen = p - e;
        un->root = s;
        un->rlen = e - s - 1;
    }
    else
    {   un->extn = NULL;
        un->elen = 0;
        s = e;
        un->root = e;
        un->rlen = p - e;
    }

    if (un->path == s)
    {   un->path = NULL;
        un->plen = 0;
    }
    else
        un->plen = s - un->path - 1;
    un->pathsep = DOT;
}

#endif /* HOST_IS_ACORN */

#ifdef HOST_IS_MAC

static void reparse_as_mac_name(const char *s, UnparsedName *un)
{   const char *p;

    if (un->flags & HASSLASH)
        /* interpretation as a Unix file name was just fine... */
        return;

    un->flags = 0;
    un->vol = un->path = NULL;
    un->plen = un->vlen = 0;

    for (p = s;  *p != 0;  ++p)
        if (*p == COLON)
        {   if (un->path == NULL)
            {   if (p != s && un->vol == NULL)
                {   un->vol = s;
                    un->vlen = p - s;
                    un->flags |= ROOTED;
                }
                else
                    un->path = s+1;
            }
            s = p;
        }

    if (un->path == (s+1))
        un->path = NULL;
    else if (un->path != NULL)
        un->plen = s - un->path;
    if (*s == COLON) ++s;
    un->root = s;
    un->rlen = p - s;
    if (un->extn != NULL) un->rlen -= (un->elen + 1);
    un->pathsep = COLON;
}

#endif /* HOST_IS_MAC */


void fname_parse(const char s[], const char *suffixes, UnparsedName *un)
{
    parse_as_unix_or_dos_name(s, un, SLASH);

#ifndef HOST_IS_UNIX
    if (un->flags & PSEUDOUNIX
        ||
        un->vol == NULL && (un->flags & HASSLASH)
#  ifdef HOST_IS_ACORN
        && !(s[0] == COLON || s[0] == DOLLAR ||
             s[0] == AMPERSAND || s[0] == HAT || s[0] == AT)
#  endif
        )
#endif
    {   un->type = FNAME_UNIX | (un->flags & ROOTED);
        return;
    }

#ifdef HOST_IS_ACORN
    reparse_as_risc_os_name(s, un, suffixes);
#endif

#ifdef HOST_IS_MAC
    reparse_as_mac_name(s, un);
#endif

#ifdef HOST_IS_DOS
    parse_as_unix_or_dos_name(s, un, BACKSLASH);
#endif

    un->type = HOST_FTYPE | (un->flags & ROOTED);
}

static char *copyto(char *s, const char *fm, int l, char *limit)
{   while (s != limit && l > 0) {*s++ = *fm++;  --l;}
    return s;
}

/* dir_sep_() takes FNAME_ACORN, _MSDOS, _UNIX or _MACINTOSH as argument.  */
#define  dir_sep_(type)  ("\0.\\/:"[type])
#define  vol_sep_(type)  ("\0:\\/:"[type])
#define  here_(type)  "\0@..\0"[type]

int fname_unparse(UnparsedName *un, int as_path, char *buffer, int maxlen)
{   int intype = un->type & FNAME_TYPEMASK;
    char *limit = buffer + maxlen;
    char *bufp = buffer;
    as_path &= FNAME_AS_PATH;
#define stuffbuf(c) { if (bufp != limit) *bufp++ = c; }

    if (un->vol != NULL)
    {   bufp = copyto(bufp, un->vol, un->vlen, limit);
        if (un->flags & PSEUDOUNIX)
        {
#ifndef HOST_IS_MAC
            stuffbuf(vol_sep_(HOST_FTYPE));
#endif
        }
        else if (!(un->flags & NOSLASH))
            stuffbuf(DIR_SEP);
        un->type |= ROOTED;
    }
    else if (un->type & ROOTED)
    {
#ifdef HOST_IS_ACORN
        stuffbuf(DOLLAR);
#endif
#ifndef HOST_IS_MAC
        stuffbuf(dir_sep_(HOST_FTYPE));
#endif
    }
    else
    {
#ifdef HOST_IS_MAC
        stuffbuf(COLON);
#endif
    }

    if (un->path != NULL)
    {   const char *s = un->path, *p, *e = s + un->plen;
        int isep = dir_sep_(intype), l;

        for (p = s;  p <= e;  ++p)
        {   if (p < e && *p != isep) continue;
            l = p - s;
            if (
#ifdef HOST_IS_MAC
                l == 0 && intype == FNAME_MACINTOSH ||
#endif
#ifdef HOST_IS_ACORN
                l == 1 && intype == FNAME_ACORN && *s == '^' ||
#endif
                l == 2 && (intype == FNAME_MSDOS || intype == FNAME_UNIX) &&
                    s[0] == '.' && s[1] == '.')
            {
#ifndef HOST_IS_MAC
#ifdef HOST_IS_ACORN
                stuffbuf(HAT);
#else
                stuffbuf(DOT);
                stuffbuf(DOT);
#endif
#endif
            }
            else if (l == 1 && *s == here_(intype))
                goto skip_here;
            else
                bufp = copyto(bufp, s, l, limit);
#ifndef HOST_IS_MAC
            if (p < e || un->rlen > 0)
#endif
                stuffbuf(DIR_SEP);
skip_here:  s = p + 1;
        }
    }
    un->un_pathlen = (bufp - buffer);
#ifdef HOST_IS_MAC
    if (bufp != buffer && bufp[-1] == COLON) --(un->un_pathlen);
#endif

#ifdef HOST_IS_ACORN
    if (un->extn != NULL && !(un->flags & PSEUDOUNIX))
    {   bufp = copyto(bufp, un->extn, un->elen, limit);
        stuffbuf(DOT);
    }
#endif
    bufp = copyto(bufp, un->root, un->rlen, limit);
    if (un->extn != NULL
#ifdef HOST_IS_ACORN
        && (un->flags & PSEUDOUNIX)
#endif
       )
    {
#ifdef HOST_IS_ACORN
        stuffbuf(SLASH);
#else
        stuffbuf(DOT);
#endif
        bufp = copyto(bufp, un->extn, un->elen, limit);
    }
#ifndef HOST_IS_MVS     /* hackery for Codemist */
    if (as_path)
    {
#ifdef HOST_IS_MAC
        if (bufp > buffer && bufp[-1] == COLON) --bufp;
#else
        if (bufp != buffer) stuffbuf(DIR_SEP);
#endif
        un->un_pathlen = (bufp - buffer);
    }
#endif
    if (bufp == limit)
        return -1;
    else
    {   *bufp = 0;
        return  (bufp - buffer);
    }
}
