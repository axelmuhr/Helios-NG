
/* 
 * C compiler support file genhdrs.c
 * Copyright (C) Codemist Ltd, 1987, 1989.
 * Copyright (C) Acorn Computers Ltd., 1988
 * modified A C Norman, February 1990
 */

/*
 * This file is a program that is run in order to create a file called 
 * headers.c which is part of the source of the compiler.  It also creates
 * errors.h.  To do its work it needs prototype files errproto.h, feerrs.h &
 * mcerrs.h, and access to directories containing the standard headers,
 * such as stdio.h etc.  The object of having this utility is twofold -
 * first it arranges for standard headers to be built into an in-store
 * mini file system so that #including them can be fast, then it organises
 * some data compression for both builtin headers and the text of error
 * messages.
 *
 * Call this program with
 *
 * Genhdrs -o headers.c -e errors.h \
 *         -q hdrproto.h -q mcerrs.h -i <dir> <files>
 *
 * where the <files> at the end will be a list of names of files from the
 * given directory that are to be included as built-in headers.
 * If the list of files is too long to fit on the line it is possible to go
 *         -v <via-file>
 * where the via-file contains a list.  If -v is specified separate files
 * may not be given as well.
 *
 * Flag -n causes syserrs to get mapped to numbers
 * Flag -s causes other error messages to be compressed
 * Flag -b for NO_INSTORE_FILES
 *
 * On an Acorn ARM a certain version of the compiler was 323304 bytes
 * in size with no compression of error messages.  The savings by selecting
 * various of the above options were (where -t was a further option to
 * compress error strings in with the built-in headers, which option has
 * now been removed):
 *
 *      -n          4476
 *      -s          4496
 *      -n -s       8972    (about 2.7%)
 *      -t          2276
 *      -n -t       6752
 *
 * It seems that -n -s will be useful for production code, -s reasonably
 * painless for most system developers (not using -n because system
 * developers may expect to see syserrs from time to time), and no
 * compression being the mode of choice when doing cross-development
 * for a new machine, especially one with a different character set.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <time.h>

#define  MAXPATHS        40
#define  MAXFILENAMESZ  256
#define  MAXSTRINGS     1000       /* How many strings can be stored here */
#define  MAXCHARS       100000     /* Total length of strings to be processed */
#define  HEADERS         "headers.c"
#define  ERRORS          "errors.h"
static char *headers   = "c.headers";
static char *errhdr    = "h.errors";
static char *errproto  = "h.errproto";
static char *viafile   = NULL;

static int number_syserrs   = 0;
static int squeeze_errors   = 0;
static int no_instore_files = 0;

static int lastchar = 0;
static int npaths = 0;
static int nerrors = 0;
static char *paths[MAXPATHS];
static int nepaths = 0;
static char *epaths[MAXPATHS];
static char hdrname[40][MAXPATHS];
static char file_name[MAXFILENAMESZ];
static unsigned char *strings[MAXSTRINGS];
static int stringlength[MAXSTRINGS];
static unsigned short compression[256], compression_depth[256];
static unsigned short ecompression[256];
static unsigned char *chardata, *chardata1;
static int stringcount = 0;
static int syserrcount = 0;
static int charpoint = 0;
static int errfilecol = 0;

/* The following are states in a machine that detects & deletes comments */
#define NORMAL   0x0
#define STRING   0x1
#define CHAR     0x2
#define STRBACK  0x3
#define CHARBACK 0x4
#define SLASH    0x5
#define COMMENT  0x6
#define STAR     0x7
#define STREND   0x8
#define STRSTART 0x9

#ifndef __STDC__
#  ifndef HOST_HAS_MEMMOVE
char *memmove(a, b, n)
char *a, *b;
size_t n;
/* copy memory taking care of overlap */
/* Relies on sizeof(int)=sizeof(void *) and byte addressing.
   Also that memory does not wrap round for direction test. */
{   char *r = a;
    if (a < b)
        for (; n-- > 0;) *a++ = *b++;
    else
        for (a +=n, b += n; n-- > 0;) *--a = *--b;
    return r;
}

#  endif
#endif

static FILE *path_open(name, mode)
char *name, *mode;
{
  FILE *f;
  int j;

  for (j = 0;  j < npaths;  ++j)
  {
    strcpy(file_name, paths[j]);
    strcat(file_name, name);
    f = fopen(file_name, mode);
    if (f != NULL) return f;
  }
  return NULL;
}

#define outch(ch) { if (charpoint >= MAXCHARS) { charpoint = 0; \
    fprintf(stderr, "genhdrs: Too many chars (%u permitted)\n", MAXCHARS); \
    ++nerrors; } else chardata[charpoint++] = (ch); }

static void copy_header(name)
char *name;
{
    FILE *fh1;
    int ch, state = NORMAL;
    char ansi_name[32];

    fh1 = path_open(name, "r");
    if (fh1==NULL)
    {
      fprintf(stderr, "Unable to read input file %s\n", name);
      ++nerrors;
      return;
    }
    if ((name[0] == 'h' || name[0] == 'H') && name[1] == '.')
    {
      strcpy(ansi_name, name+2);
      strcat(ansi_name, ".h");
    }
    else strcpy(ansi_name, name);

    fprintf(stderr, "genhdrs: Copying %s as %s\n", file_name, ansi_name);
    strcpy(hdrname[stringcount], ansi_name);
    if (stringcount >= MAXSTRINGS)
    {   fprintf(stderr, "genhdrs: Too many strings (%u allowed)\n", MAXSTRINGS);
        stringcount = 0;
        ++nerrors;
        return;
    }
    strings[stringcount++] = &chardata[charpoint];

    lastchar = '\n';

    ch = getc(fh1);
    while (ch!=EOF)
    {
        switch (state)
        {
    case NORMAL:
            switch (ch)
            {
        case '\t':
        case ' ':   ch = getc(fh1);
                    if (ch != ' ') outch(' ');
                    continue;
        case '\'':  outch(ch);  state = CHAR;   break;
        case '\"':  outch(ch);  state = STRING; break;
        case '/':               state = SLASH;  break;
        default:    outch(ch);                  break;
            }
            break;
    case STRING:
            switch (ch)
            {
        case '\"':  outch(ch);  state = NORMAL; break;
        case '\\':  outch(ch);  state = STRBACK;break;
        default:    outch(ch);                  break;
            }
            break;
    case CHAR:
            switch (ch)
            {
        case '\'':  outch(ch);  state = NORMAL; break;
        case '\\':  outch(ch);  state = CHARBACK;break;
        default:    outch(ch);                  break;
            }
            break;
    case STRBACK:
            outch(ch);
            state = STRING;
            break;
    case CHARBACK:
            outch(ch);
            state = CHAR;
            break;
    case SLASH:
            switch (ch)
            {
        case '*':               state = COMMENT;break;
        case '\'':  outch('/');
                    outch(ch);  state = CHAR;   break;
        case '\"':  outch('/');
                    outch(ch);  state = STRING; break;
        default:    outch('/');
                    outch(ch);  state = NORMAL; break;
            }
            break;
    case COMMENT:
            if (ch == '*') state = STAR;
            break;
    case STAR:
            switch (ch)
            {
        case '/':               state = NORMAL; break;
        case '*':                               break;
        default:                state = COMMENT;break;
            }
            break;
    default:
            fprintf(stderr, "\nBad state %d\n", state);
            ++nerrors;
            return;
        }
        ch = getc(fh1);
    }
    if (state!=NORMAL || lastchar!='\n')
    {
        fprintf(stderr, "\nUnexpected end of file in %s?\n", file_name);
        ++nerrors;
    }
    outch(0);
    while (charpoint & 3) outch(0);
    fclose(fh1);
}

static void decompress_char(c)
int c;
{
    int z = compression[c];
    if (c == z)
    {   if (c == '\n') c = 'n', putc('\\', stderr);
        else if (c == 0) c = '0', putc('\\', stderr);
        putc(c, stderr);
    }
    else
    {   decompress_char((z >> 8) & 0xff);
        decompress_char(z & 0xff);
    }
}

/*
 * Macintosh has 0xD and 0xA swapped problem:
 * Comparison of headers.c generated on all hosts is simplified by modifying
 * the literal codes in the compression_info array.  If the compressed headers
 * are identical, then the headers.c files will only differ at this location.
 * TO BE IMPLEMENTED
 */
#ifdef mapping
#  define MAPCHAR(c) ( ( (c) == 0xD || (c) == 0xA ) ? ( (c) ^ 0x7) : (c) )
#else
#  define MAPCHAR(c) (c)
#endif

static int compress_strings(emsg, digraph_counts)
int emsg;
int *digraph_counts;
/*
 * value returned is greatest stack depth needed during decompression.
 * A nonzero arg => treat strings as error messages and try to preserve
 * %? and $? escape sequences in them.
 */
{
    int i, nsquashed = 0, stackdepth_needed = 0, worst_char = 0;
    int total_bytes = 0;

/*
 * Identify the characters that are in use in the given set of strings.
 */
    for (i=0; i<stringcount; i++)
    {   unsigned char *s = strings[i];
        int l = stringlength[i] = 1 + strlen((char *)s);
        int k;
        total_bytes += l;
        for (k=0; k<l; k++) compression[MAPCHAR(s[k] & 0xff)] = s[k];
    }
    fprintf(stderr, "%d bytes of strings found\n", total_bytes);

    for (i=1; i<256 && stackdepth_needed < 10; i++)
    {   int z = compression[i];
        int j, best, bestj;
        if ( (z>>8) == 0 && z != 0 ) continue;   /* Char code already in use */
        for (j=0; j<256*256; j++) digraph_counts[j] = 0;
        for (j=0; j<stringcount; j++)
        {   unsigned char *s = strings[j];
            int l = stringlength[j];
            int k, inesc = 0;
            for (k=1; k<l; k++)
            {   int a = MAPCHAR(s[k-1]) & 0xff, b = MAPCHAR(s[k] & 0xff);
                if (!emsg) digraph_counts[256*a+b]++;
                else switch (inesc)
                {
        case 0:     switch (a)
                    {
            case '%':   inesc = 1;
                        continue;
            case '$':   inesc = 2;
                        continue;
            default:    if (b != '%' && b != '$' && b != 0)
                            digraph_counts[256*a+b]++;
                        continue;
                    }
        case 1:     switch (tolower(a))
                    {
            case '%': case 'c': case 'd': case 'e': case 'f': case 'g':
            case 'i': case 'n': case 'o': case 'p': case 's': case 'u':
            case 'x':   inesc = 0;  /* Drop through */
            default:    continue;
                    }
        default:    inesc = 0;
                    continue;
                }
            }
        }
        best = bestj = -1;
        for (j=0; j<256*256; j++)
            if (digraph_counts[j] > best) best = digraph_counts[j], bestj = j;
        if (best <= 1) break;   /* Compression finished! */
#ifdef VERBOSE_MODE
        fprintf(stderr, "%.4x -> %.2x  ", bestj, i);
#else
        putc('.', stderr);
        if (++nsquashed == 64) putc('\n', stderr), nsquashed = 0;
#endif
        compression[i] = bestj;
        {   int l1 = compression_depth[bestj & 0xff];
            int l2 = compression_depth[bestj >> 8];
            int l = 1 + l2;
            if (l1 > l) l = l1;
            compression_depth[i] = l;
            if (l > stackdepth_needed)
            {   stackdepth_needed = l;
                worst_char = i;
            }
        }
        for (j=0; j<=stringcount; j++)
        {   unsigned char *s = strings[j];
            int l = stringlength[j];
            int ba = bestj>>8, bb = bestj & 0xff;
            int k, inesc = 0;
            for (k=1; k<l; k++)
            {   int a = MAPCHAR(s[k-1] & 0xff), b = MAPCHAR(s[k] & 0xff);
/*
 * I only combine characters if so doing would not disrupt an escape
 * sequence.
 */
                if (!emsg) goto combine;
                else switch (inesc)
                {
        case 0:     switch (a)
                    {
            case '%':   inesc = 1;
                        continue;
            case '$':   inesc = 2;
                        continue;
            default:    if (b != '%' && b != '$' && b != 0) goto combine;
                    }
        case 1:     switch (tolower(a))
                    {
            case '%': case 'c': case 'd': case 'e': case 'f': case 'g':
            case 'i': case 'n': case 'o': case 'p': case 's': case 'u':
            case 'x':   inesc = 0;  /* Drop through */
            default:    continue;
                    }
        default:    inesc = 0;
                    continue;
                }
        combine:
                if (a == ba && b == bb)
                {   s[k-1] = i;
                    memmove(&s[k], &s[k+1], l-k-1);
                    stringlength[j] = l = l-1;
                    total_bytes--;
                }
            }
        }
    }
    fprintf(stderr,
            "\nNeed %d bytes of stack in decompression for char %.2x (",
            stackdepth_needed, worst_char);
    decompress_char(worst_char);
    fprintf(stderr, ")\nsize = %d now\n", total_bytes);
    return stackdepth_needed;
}

static int squash_all_messages(digraph_counts)
int *digraph_counts;
{
    int sk = compress_strings(1, digraph_counts);
/* Save the decompression table for printing at a later stage */
    memcpy(ecompression, compression, sizeof(ecompression));
    return sk;
}

static void print_squashed_error(s, l, fe)
unsigned char *s;
int l;
FILE *fe;
{
    int c, i;
    putc('\"', fe);
    errfilecol++;
    for(i=0; i<l; i++)
    {   switch (c = s[i])
        {
    case '\n':
            c = 'n';        /* Drop through */
    case '\'': case '\"': case '\\':
            break;
    case 0: if (i == l-1) continue;
    default:
            if (errfilecol > 70 && !isspace(c))
            {   putc('\\', fe);
                putc('\n', fe);
                errfilecol = 0;
            }
            if (isalnum(c) || c == ' ' ||
                c == '%' || c == '$' || c == ':' || c == '.' ||
                c == ',' || c == '-') putc(c, fe), errfilecol++;
/*
 * If I need to print an (octal) escape I always show 3 digits to avoid
 * confusion if the next character happens to be numeric.
 */
            else fprintf(fe, "\\%.3o", c & 0xff), errfilecol+=4;
            continue;
        }
        putc('\\', fe);
        putc(c, fe), errfilecol+=2;
    }
    putc('\"', fe), errfilecol++;
}

#define MAP         0
#define SYSERR      1
#define LEAVE       2

static void scan_error_file(fq)
FILE *fq;
{
/*
 * This is only called if I am squeezing error strings.
 * In MAP mode I will collect strings so that the
 * compression scheme has some data to work on.
 */
    int prevch = -1, ch, ch1, state = NORMAL;
    int action = MAP;

    while ((ch1 = ch = getc(fq)) != EOF)
    {
        if (state == NORMAL && prevch == '\n' && ch == '%')
        {   ch1 = ch = getc(fq);
            switch (tolower(ch))
            {
    case 'o':
                action = MAP; break;
    case 's':
    case 'z':   action = LEAVE;  break;
    default:    fprintf(stderr, "\n%%%c unrecognised\n", ch);
                break;
            }
            continue;
        }
        if (state == NORMAL && prevch == '/' && ch == '*') state = COMMENT;
        else if (state == COMMENT && prevch == '*' && ch == '/') state = NORMAL;
        else if (state == NORMAL && ch == '\"')
        {   state = STRSTART;
            switch (action)
            {
        case MAP:
                if (stringcount >= MAXSTRINGS)
                {   fprintf(stderr,
                      "genhdrs: Too many strings (%u allowed)\n", MAXSTRINGS);
                    stringcount = 0;
                    ++nerrors;
                    return;
                }
                strings[stringcount++] = &chardata[charpoint];
                break;
        default:
                break;
            }
        }
        else if (state == STRING)
        {   for (;;)
            {   if (ch == '\\')
                {   switch (ch1 = ch = getc(fq))
                    {
            case '\n':
                        ch1 = ch = getc(fq);
                        continue;
            case 'n': case 'N':
                        ch1 = '\n';
                        break;
            case 'a': case 'A':
            case 'b': case 'B':
            case 'f': case 'F':
            case 'r': case 'R':
            case 't': case 'T':
            case 'v': case 'V':
            case 'x': case 'X':
            case '0':   fprintf(stderr,
                                "Bad escape found in header prototype\n");
            
            default:
                        break;
                    }
                }
                else if (ch == '\"') state = STREND;
                break;
            }
        }
        if (action == MAP && state == STRING) outch(ch1);
        if (state == STREND)
        {   state = NORMAL;
            switch (action)
            {
        case MAP:
                outch(0);
                while (charpoint & 3) outch(0);
                break;
        default:
                break;
            }
        }
        if (state == STRSTART) state = STRING;
        prevch = ch;
    }
}

static void copy_error_file(fe, fq)
FILE *fe, *fq;
{
    int prevch = -1, ch, ch1, state = NORMAL;
    int action = squeeze_errors ? MAP : LEAVE;

    while ((ch1 = ch = getc(fq)) != EOF)
    {
        if (state == NORMAL && prevch == '\n' && ch == '%')
        {   ch1 = ch = getc(fq);
            switch (tolower(ch))
            {
    case 'o':   action = squeeze_errors ? MAP : LEAVE; break;
    case 's':   action = number_syserrs ? SYSERR : LEAVE; break;
    case 'z':   action = LEAVE;  break;
    default:    fprintf(stderr, "\n%%%c unrecognised\n", ch);
                break;
            }
            continue;
        }
        if (state == NORMAL && prevch == '/' && ch == '*') state = COMMENT;
        else if (state == COMMENT && prevch == '*' && ch == '/') state = NORMAL;
        else if (state == NORMAL && ch == '\"')
        {   state = STRSTART;
            switch (action)
            {
        case MAP:
                if (stringcount >= MAXSTRINGS)
                {   fprintf(stderr,
                      "genhdrs: Too many strings (%u allowed)\n", MAXSTRINGS);
                    stringcount = 0;
                    ++nerrors;
                    return;
                }
                print_squashed_error(strings[stringcount],
                                     stringlength[stringcount], fe);
                stringcount++;
/*
 * I wanted to put a newline here to improve the format of the converted
 * error file - but that caused confusion in places where I had line
 * continuation in the header prototype files.  Hence I will end up
 * with longer lines here than I like.
 */
                errfilecol+=fprintf(fe, "    /* ");
                break;
        case SYSERR:
                errfilecol+=fprintf(fe, "%d /* ", syserrcount++);
                break;
        default:
                break;
            }
        }
        else if (state == STRING)
        {   for (;;)
            {   if (ch == '\\')
                {   putc(ch, fe), errfilecol++;
                    switch (ch1 = ch = getc(fq))
                    {
            case '\n':
                        putc(ch, fe), errfilecol = 0;
                        ch1 = ch = getc(fq);
                        continue;
            case 'n': case 'N':
                        ch1 = '\n';
                        break;
            case 'a': case 'A':
            case 'b': case 'B':
            case 'f': case 'F':
            case 'r': case 'R':
            case 't': case 'T':
            case 'v': case 'V':
            case 'x': case 'X':
            case '0':   fprintf(stderr,
                                "Bad escape found in header prototype\n");
            
            default:
                        break;
                    }
                }
                else if (ch == '\"') state = STREND;
                break;
            }
        }
        putc(ch, fe);
        if (ch == '\n') errfilecol = 0;
        else errfilecol++;
        if (state == STREND)
        {   state = NORMAL;
            if (action==MAP || action==SYSERR)
                errfilecol+=fprintf(fe, " */");
        }
        if (state == STRSTART) state = STRING;
        prevch = ch;
    }
}

int main(argc, argv)
int argc;
char *argv[];
{
    int j, where, esk = 0;
    char *arg;
    FILE *fh, *fe;
    int *digraph_counts;

    chardata = (unsigned char *)malloc(MAXCHARS);
    chardata1 = (unsigned char *)malloc(MAXCHARS);
    digraph_counts = (int *)malloc(256*256*sizeof(int));

    if (chardata == NULL || chardata1 == NULL || digraph_counts == NULL)
    {   fprintf(stderr, "genhdrs: Not enough free memory\n");
        exit(1);
    }

    charpoint = stringcount = syserrcount = nerrors = npaths = 0;
    viafile = NULL;

    for (j = 1;  j < argc;  ++j)
    {
        arg = argv[j];
        if (arg[0] == '-')
        {
            switch (arg[1])
            {
/*
 * -n           get syserr codes mapped onto numeric values
 */
    case 'n':
    case 'N':   number_syserrs = 1;
                break;
/*
 * -s           squeeze error messages (as individual strings)
 */
    case 's':
    case 'S':   squeeze_errors = 1;
                break;
/*
 * -b           do not create file-system for <xxx.h> files
 */
    case 'b':
    case 'B':   no_instore_files = 1;
                break;
/*
 * -h           introduces list of header files to be copied across
 *              to form the in-store file system built into headers.c
 *              (actually -h is ignored!)
 */
    case 'h':
    case 'H':   break;
/*
 * -i           specify a search path for finding ordinary files to be
 *              copied to create in-store headers.  E.g. -i /usr/include/
 *              might make sense on a Unix system.
 */
    case 'i':
    case 'I':   if (npaths >= MAXPATHS)
                {
                    fprintf(stderr,
                        "genhdrs: Too many paths - only %u allowed\n",
                        MAXPATHS);
                    exit(1);
                }
                else
                {
                    arg += 2;
                    if (*arg == 0) arg = argv[++j];
                    paths[npaths++] = arg;
                }
                break;
/*
 * -o           File to put bulk of compressed text into. This file
 *              will be compiled (as "headers.c") to form part of the
 *              compiler.
 */
    case 'o':
    case 'O':   arg += 2;
                if (*arg == 0) arg = argv[++j];
                headers = arg;
                break;
/*
 * -e           Error header output file ("errors.h") gets created by
 *              scanning error prototype file and assigning numeric error
 *              codes to messages, inserting compressed text for the
 *              messages into headers.c
 */
    case 'e':
    case 'E':   arg += 2;
                if (*arg == 0) arg = argv[++j];
                errhdr = arg;
                break;
/*
 * -q           prototype for "errors.h" contains raw form of error messages
 *              with various marker strings (%Z, %O and %S) to show what
 *              form of transformation is needed.
 *              %O   define a numeric error code & arrange decoding tables,
 *              %S   define numeric code but do not provide for decoding,
 *              %Z   leave error text as a literal string (not numeric).
 *
 *              There will normally be (at least) two error prototype files,
 *              one the generic one (mip/errproto.h) and one that is target
 *              specific (xxx/mcerrs.h).  It may also prove necessary to
 *              have language (i.e. front-end) specific error files and
 *              even host-system specific ones - hence this utility allows
 *              for MAXPATHS inclusions here.
 */
    case 'q':
    case 'Q':   if (npaths >= MAXPATHS)
                { 
                    fprintf(stderr,
                        "genhdrs: Too many paths - only %u allowed\n",
                        MAXPATHS);
                    exit(1);
                }
                else
                {
                    arg += 2;
                    if (*arg == 0) arg = argv[++j];
                    epaths[nepaths++] = arg;
                }
                break;
/*
 * -v           Privide the list of files for scanning through an indirection.
 */
    case 'v':
    case 'V':   arg += 2;
                if (*arg == 0) arg = argv[++j];
                viafile = arg;
                break;
default:        break;
            }
        }
    }

    if (nepaths == 0) epaths[nepaths++] = errproto;

    fh = fopen(headers, "w");
    if (fh == NULL)
    {   fprintf(stderr, "Unable to create output file %s\n", headers);
        exit(1);
    }

    fe = fopen(errhdr, "w");
    if (fe == NULL)
    {   fprintf(stderr, "Unable to create output file %s\n", errhdr);
        exit(1);
    }

    fprintf(stderr, "genhdrs: Creating %s and %s...\n", headers, errhdr);

    for (j=0; j<256; j++) compression[j] = compression_depth[j] = 0;

    if (squeeze_errors)
    {   /* Here a pass over the error files is needed to collect */
        /* information for the compression process               */
        for (j=0; j<nepaths; j++)
        {
            FILE *fq = fopen(epaths[j], "r");
            if (fq == NULL)
            {
                fprintf(stderr, "Unable to open error prototype file %s\n",
                        epaths[j]);
                ++nerrors;
                continue;
            }
            fprintf(stderr, "Scan error file %s\n", epaths[j]);
            scan_error_file(fq);
            fclose(fq);
        }
        esk = squash_all_messages(digraph_counts);
        charpoint = stringcount = 0;
    }

    {   time_t t0 = time(NULL);
/* /* AM wants these to be textual, not to require hacking every change */
        fprintf(fe, "\n/*\n * C compiler file %s\n", ERRORS);
        fprintf(fe, " * Copyright (C) Codemist Ltd, %.4s\n */\n\n",
                    20+ctime(&t0));
/*         fprintf(fe, "#pragma force_top_level\n");                    */
/*         fprintf(fe, "#pragma include_only_once\n");                  */
        fprintf(fe, "#ifndef _errors_LOADED\n#define _errors_LOADED 1\n\n");
    }

    if (number_syserrs) fprintf(fe, "typedef int syserr_message_type;\n\n");
    else
    {
/* We really need a magic name like __check_ncc_internal_formats for -v3. */
        fprintf(fe, "#pragma -v3\n");
        fprintf(fe, "typedef char *syserr_message_type;\n");
    }
    fprintf(fe, "extern void syserr(syserr_message_type errcode, ...);\n\n");
    if (number_syserrs);        /* nothing yet */
    else fprintf(fe, "#pragma -v0\n\n");

    for (j=0; j<nepaths; j++)
    {
        FILE *fq = fopen(epaths[j], "r");
        if (fq == NULL)
        {
            fprintf(stderr, "Unable to open error prototype file %s\n",
                    epaths[j]);
            ++nerrors;
            continue;
        }
        fprintf(stderr, "Copy error file %s\n", epaths[j]);
        errfilecol = 0;
        copy_error_file(fe, fq);
        fclose(fq);
    }

    charpoint = stringcount = 0;

    if (nerrors != 0)
        fprintf(fe, "\n#error genhdrs failed to create %s\n", ERRORS);

    if (number_syserrs)
        fprintf(fe, "\n#define NUMERIC_SYSERR_CODES 1\n");

    if (no_instore_files)
    {   fprintf(fe, "#ifndef NO_INSTORE_FILES\n");
        fprintf(fe, "#  define NO_INSTORE_FILES 1\n");
        fprintf(fe, "#endif\n");
    }

    if (squeeze_errors)
    {   fprintf(fe, "\n#define COMPRESSED_ERROR_MESSAGES 1\n");
        fprintf(fe, "\n#ifdef DEFINE_ERROR_COMPRESSION_TABLE\n");
        fprintf(fe, "\nstatic unsigned short int ecompression_info[256] = {");
        for (j=0; j<256; j++)
        {   if ((j & 0x7) == 0) fprintf(fe, "\n    ");
            fprintf(fe, "0x%.4x", ecompression[j]);
            if (j == 255) fprintf(fe, "};\n\n");
            else fprintf(fe, ", ");
        }
        fprintf(fe, "\n#endif\n");
    }

    {   time_t t0 = time(NULL);
        fprintf(fh, "\n/* %s, created %.24s by the genhdrs utility */\n\n",
                    HEADERS, ctime(&t0));
    }
    fprintf(fh,
        "/* Copyright (C) Acorn Computers Ltd and Codemist Ltd, 1990 */\n\n");

    if (nerrors == 0)
    {

        if (!no_instore_files)
        {   if (viafile)
            {   FILE *vv = fopen(viafile, "r");
                if (vv == NULL)
                    fprintf(stderr, "File %s could not be opened\n", viafile);
                else
                {
                    char fname[64];
                    int rc;
/* fscanf is magic here - it strips away whitespace just the way I want */
                    while ((rc = fscanf(vv, "%s", fname)) == 1)
                        copy_header(fname);
                    fclose(vv);
                }
            }
            else
            {   for (j = 1;  j < argc;  ++j)
                {   int cc;
                    arg = argv[j];
                    cc = tolower(arg[1]);
/*
 * Now I process all the args that were not introduced by explicit keys
 * (treating them as if there had been an implicit -h in front of them).
 * Maybe it would have been better to collect these in a list during
 * the earlier pass that picked out keyed args?
 */
                    if (arg[0] != '-') copy_header(arg);
                    else if (cc != 0 &&                     /* - on its own */
                             cc != 'h' &&                   /* -h -H */
                             cc != 'n' && cc != 's' &&      /* other flags */
                             cc != 'b' &&
                             arg[2] == 0) j++;              /* arg followed on */
                }
            }
        }
    }

    if (nerrors == 0)
    {   int sk = 0;
        if (!no_instore_files)
        {   sk = compress_strings(0, digraph_counts);
            if (esk > sk) sk = esk;
            fprintf(fh,
                "\nstatic unsigned short int compression_info[256] = {");
            for (j=0; j<256; j++)
            {   if ((j & 0x7) == 0) fprintf(fh, "\n    ");
                fprintf(fh, "0x%.4x", compression[j]);
                if (j == 255) fprintf(fh, "};\n\n");
                else fprintf(fh, ", ");
            }
        }

        if (!no_instore_files)
        {
            fprintf(fh, "\nstatic header_files builtin_headers[] = {\n");

            where = 0;
            for (j=0;j<stringcount;j++)
            {   unsigned char *s = strings[j];
                int l = stringlength[j];
                int w;
                fprintf(fh, "   {\"%s\", %d},\n", hdrname[j], where);
/*
 * Strings can NEVER grow when compress_strings() is called, so if my data
 * fitted in the array chardata[] it will also fit in chardata1[] without
 * overflow.  Therefore I do not test for overflow here.
 */
                for (w=0; w<l; w++) chardata1[where++] = *s++;
            }
            fprintf(fh,"   {0, 0}};\n\n");
            fprintf(fh, "static char string_data[] = {");
            for (j=0;;)
            {   if ((j & 0xf) == 0) fprintf(fh, "\n    ");
                fprintf(fh, "%3d", chardata1[j]);
                j++;
                if (j < where)
                {   putc(',', fh);
                    continue;
                }
                else
                {   fprintf(fh, "};\n\n");
                    break;
                }
            }
        }

        fprintf(fh, "/* end of %s */\n\n", HEADERS);

        sk = (sk + 4) & ~3; /* Round up to multiple of 4, adding 1 for safety */
        fprintf(fe, "\n#define MAXSTACKDEPTH %d\n\n", sk);
        fprintf(fe, "#endif /* already loaded */\n\n/* end of %s */\n", ERRORS);
        fclose(fe);


        fprintf(stderr, "genhdrs: Finished\n");
    }
    else
    {
        fclose(fh);
        fh = fopen(headers, "w");  /* to truncate it to zero length */
        fprintf(fh, "#error \"%s is empty because of genhdrs errors\"\n",
                    HEADERS);
        fprintf(stderr, "\ngenhdrs: %s is junk because of errors\n\n", headers);
        fprintf(fh,"/* end of %s */\n", HEADERS);
        exit(1);
    }

    fclose(fh);
    free(chardata);
    free(chardata1);
    free(digraph_counts);
    return 0;
}

/* End of genhdrs.c */
