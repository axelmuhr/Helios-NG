/*
 * fname.c - Support for translation among Acorn, Unix, and MS-DOS file names
 * Copyright (C) Acorn Computers Ltd., 1988.
 */

/*
 * RCS $Revision: 1.1 $
 * Checkin $Date: 1993/11/23 16:41:52 $
 * Revising $Author: nickc $
 */

#ifndef NO_VERSION_STRINGS
extern char fname_version[];
char fname_version[] = "\nfname.c $Revision: 1.1 $ 18\n";
#endif

#include <string.h>
#include "globals.h"            /* for COMPILING_ON flags + sixchar.h   */
#include "fname.h"

#ifndef  TRUE
#  define  FALSE         0
#  define  TRUE          1
#endif
#ifndef  NULL
#  define  NULL          0
#endif
#define  dir_sep_(type)  ("*.\\/"[type])
#define  root_(type)     ("*$\\/"[type])
#define  here_(type)     ("*@.."[type])
#define  EXTN_SEP        '.'
#ifdef COMPILING_ON_MVS
#  define HACK_SEP ':'
#else
#  define HACK_SEP dir_sep_(FNAME_ACORN)
#endif
#define  ACORN_DEV_SEP   ':'
#define  NOSWAP          32
#define  PATH_IS_DEV     64
#define  TYPE_FLAGS      (NOSWAP | PATH_IS_DEV | FNAME_ROOTED)
#define  UNDEFINED       128

#ifndef NO_TRANSLATE_FILENAMES

static char *up_[4] = {
  NULL, "^",  "..",  ".."        /* assumed to be 1 or 2 chars only */
};

static long int try[4] = {
/* DFLT   */  (long)0,
/* ACORN  */  (long)FNAME_ACORN  + ((long)FNAME_UNIX << 8) + ((long)FNAME_MSDOS << 16),
/* MS-DOS */  (long)FNAME_MSDOS  + ((long)FNAME_UNIX << 8) + ((long)FNAME_ACORN << 16),
/* UNIX   */  (long)FNAME_UNIX,
  };

static int is_dir_sep(int ch, int *p_type)
{   int j;
    long int t = (long)*p_type;

    if (!(t & UNDEFINED))
        return (ch == dir_sep_(t) ? TRUE : FALSE);
    else
    {   t = try[t & ~UNDEFINED];
        do
        {   j = (int)t & 255;
            if ((ch == dir_sep_(j)) ||
                (j == FNAME_ACORN) && (ch == ACORN_DEV_SEP))
            {   *p_type = j;
                return TRUE;
            }
            t = t >> 8;
        }
        while (t != 0);
    }
    return FALSE;
}

static int is_suffix(const char *extn, int elen, const char *suffixes)
{   int l, sch;
    sch = *suffixes++;
    while (sch)
    {   while (sch == ' ') sch = *suffixes++;
        l = 0;
        while (l < elen && extn[l] == sch) {++l;  sch = *suffixes++;}
        /* Assert: l >= elen || *extn[l] != *suffixes */
        if (l >= elen && (sch == ' ' || sch == 0)) return TRUE;
        while (sch != ' ' && sch != 0) sch = *suffixes++;
    }
    return FALSE;
}

static int is_rooted(const char *path, int plen, int type)
{   int ch = 0;
    while (plen > 0)
    {   --plen;
        ch = path[plen];
        if (ch == ':' ||
            type == FNAME_ACORN && (ch == '$' || ch == '&'))
            return 1;
    }
    if (ch == dir_sep_(type)) return 1;
    return 0;
}

#endif /* NO_TRANSLATE_FILENAMES */

int fname_parse(const char s[], int type, const char *suffixes,
                UnparsedName *un)
{
  memset(un, 0, sizeof(UnparsedName));
  
#ifndef NO_TRANSLATE_FILENAMES
  { const char *root, *extn;
    int l, t, rlen;
    extn = NULL;
    root = s;
    rlen = strlen(s);
    
    if ((s[0] == '.') && (s[1] == '\0' || s[1] == '.' && s[2] == '\0'))
      {
        /* "." or ".." */
        t = type = FNAME_UNIX;
      }
    else
      {
	t = (type & 3) + UNDEFINED;

	/* seek the extension, root name, and file-name type */
	
        for (l = rlen;  --l >= 0;)
        {
	  if (extn == NULL && s[l] == EXTN_SEP)
            {
	      extn = s + l + 1;

	      un->elen = rlen - l - 1;

	      continue;
            }

	  if (is_dir_sep(s[l], &t))
            {
	      root = s + l;

	      break;
            }
        }
	
        type = t & ~UNDEFINED;
    }

    /* Assert: root != NULL */
    
    if (root != s)
    {   if (type == FNAME_ACORN && *root == ACORN_DEV_SEP)
            type |= (PATH_IS_DEV | FNAME_ROOTED);
        un->path = s;
        un->plen = l = root - s;
        rlen -= (l + 1);
        ++root;
        if (is_rooted(s, l, type)) type |= FNAME_ROOTED;
    }

    if (extn != NULL)
    {   rlen -= (un->elen + 1);
        if ((type & FNAME_TYPEMASK) == FNAME_ACORN &&
             is_suffix(root, rlen, suffixes))
        {   /* swap root and extn */
            l = un->elen;  un->elen = rlen;  rlen = l;
            un->extn = root;
            root = extn;
        }
        else
        {   un->extn = extn;
            if (!is_suffix(extn, un->elen, suffixes))
                /* (type != ACORN) || !is_suffix(root,...) */
                type |= NOSWAP;
        }
    }

    un->root = root;
    un->rlen = rlen;
    un->type = type;
    type &= FNAME_TYPEMASK;
    
    un->pathsep = dir_sep_(type);
  }

  return type;

#else /* NO_TRANSLATE_FILENAMES */

  un->path = s;
  un->plen = strlen(s);
  un->pathsep = NO_TRANSLATE_FILENAMES;

  return FNAME_DFLT;
#endif
}

int fname_unparse(UnparsedName *un, int type, char *buffer, int maxlen)
{
#ifndef NO_TRANSLATE_FILENAMES
    char *name;
    const char *s;
    int un_t, l, ch, sep;

    name = buffer;
    s = un->path;
    sep = un->pathsep;
    un->type &= ~FNAME_ROOTED;
    if (s)
    {   const char *up, *up_trans;
        int prev_is_sep;
        un_t = un->type & ~TYPE_FLAGS;
        l = un->plen;
        if (is_rooted(s, l, un_t))
        {   /* initial root denotation... */
            ch = *s++;  --l;
            if (un_t == FNAME_ACORN)
            {   if (type != FNAME_ACORN && (ch == '$' || ch == '&'))
                {   ch = root_(type);
                    ++s;  --l;
                }   
            }
            else if (ch == sep)
            {   ch = root_(type);
                if (type == FNAME_ACORN && maxlen > 1 /* && l > 0 */)
                {  *name++ = ch;  --maxlen;
                   ch = dir_sep_(FNAME_ACORN);
                }
            }
            if (maxlen > 1) {*name++ = ch;  --maxlen;}
            un->type |= FNAME_ROOTED;
        }
        if (l <= 0 && (un_t != FNAME_ACORN || type != FNAME_ACORN))
            goto skip_path_separator;
        /* now handle a sequence of path segments... */
        up = up_[un_t];
        up_trans = up_[type];
        prev_is_sep = 1;
        while (l > 0)
        {   ch = *s++;  --l;
            if (prev_is_sep)
            {   prev_is_sep = 0;
                if (ch == up[0] &&
                    (up[1] == 0  && (l == 0 || *s == sep) ||
                     up[1] == *s && l > 0 && (l == 1 || s[1] == sep)))
                {   if (up[1] != 0) {++s;  --l;}
                    if (up_trans[1] == 0)
                        ch = up_trans[0];
                    else
                    {   if (maxlen > 1) {*name++ = up_trans[0];  --maxlen;}
                        ch = up_trans[1];
                    }
                }
                else if (ch == here_(un_t) && (l == 0 || *s == sep))
                {   /* optimise out './', @., etc. */
                    if (l == 0) goto skip_path_separator;
                    ++s;  --l;
                    prev_is_sep = 1;
                    continue;
                }
            }
            else if (ch == sep)
            {   /* Assert: ch == sep && (!prev_is_sep || l == 0) */
                ch = dir_sep_(type);
                prev_is_sep = 1;
            }
            else
                prev_is_sep = 0;
            if (maxlen > 1) {*name++ = ch;  --maxlen;}
        }
        if (un->type & PATH_IS_DEV)
            ch = ACORN_DEV_SEP;
        else
            ch = dir_sep_(type);
        if (maxlen > 1) {*name++ = ch;  --maxlen;}
skip_path_separator:;
    }

    un->un_pathlen = name - buffer;

    if ((type == FNAME_ACORN) && ((un->type & NOSWAP) == 0))
    {   s = un->extn;
        if (s)
        {   l = un->elen;
            if (l > maxlen) l = maxlen;
            strncpy(name, s, l);
            name += l;  maxlen -= l;
            if (maxlen > 1) {*name++ = HACK_SEP;  --maxlen;}
        }
    }

    un_t = un->type & ~TYPE_FLAGS;
    l = un->rlen;
    if (l > maxlen) l = maxlen;
    strncpy(name, un->root, l);
    name += l;  maxlen -= l;

    if ((type != FNAME_ACORN) || (un->type & NOSWAP))
    {   s = un->extn;
        if (s)
        {   l = un->elen;
            if (maxlen > 1) {*name++ = EXTN_SEP;  --maxlen;}
            if (l > maxlen) l = maxlen;
            strncpy(name, s, l);
            name += l;
            maxlen -= l;
        }
    }

    if (maxlen > 0)
    {   *name = '\0';
        return (name - buffer);
    }
    else
        return (-1);

#else /* NO_TRANSLATE_FILENAMES */
    int l = un->plen, rc = l;
    type = type;
    if (l >= maxlen) {l = maxlen;  rc = -1;}
    strncpy(buffer, un->path, l);
    buffer[l] = 0;
    un->un_pathlen = l;
    return rc;
#endif
}

#ifdef FNAME_SET_TRY_ORDER

int fname_set_try_order(int what, int try1, int try2, int try3)
{
  if ((what < FNAME_ACORN) || (what > FNAME_UNIX)) return -1;
  try[what] = (long)try1 + ((long)try2 << 8) + ((long)try3 << 16);
  return 0;
}

#endif /* FNAME_SET_TRY_ORDER */
