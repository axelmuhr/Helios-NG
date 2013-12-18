/*
 * Tool to translate from ANSI to pcc-style function headers.
 * Copyright (C) Acorn Computers Ltd., 1988, 1989.
 */

#define  DEBUGGING 1

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <stdarg.h>

#ifdef __STDC__
#  include <stdlib.h>
#  include <string.h>
#else
char *malloc();
int free();
#  include <strings.h>
#endif

#define  SELF         "topcc"
#define  VSN          "3.00/013"
#define  BRIEF        "convert a C program from ANSI to PCC dialect"
#ifdef   __STDC__
#define  DATE         __DATE__
#else
#define  DATE         "Jun 16 1989"
#endif
#define  OK           0
#define  BAD          1

/*
 * BEWARE: id, lookaside_buf, depth_stack and other arrays are not
 *         checked for potential overflow. If you over-run the limits
 *         listed below then the program will fail. This is, of course,
 *         extremely sloppy programming practice.
 */

#define  ID_MAX          256    /* At most this no of chars in an identifier */

#define  ID_START_CL       1    /* Class of chars starting an identifier */
#define  NUM_CL            2    /* Decimal digit class */
#define  ID_CL            (ID_START_CL + NUM_CL)

#define  ID_TOKEN          1
#define  NUM_TOKEN         2
#define  ID_OR_NUM        (ID_TOKEN + NUM_TOKEN)
#define  MAX_ARGS        128
#define  MAX_LOOKASIDE  2048
#define  HEADER_BRKLEN    72
#define  DEPTH_STACKSZ    50


static FILE *in_file,            /* used only by main and getch */
            *out_file;           /* used by every fn that outputs stuff */

static int next_ch, copied_line_len, line_no, brace_depth, depth_sp;
static int depth_stack[DEPTH_STACKSZ];

static char id[ID_MAX], cur_fn[ID_MAX];

static char char_class[256];

static char *arg_id[MAX_ARGS];
static int narg_ids;

static char *lookaside, *la_id_start, *cur_name;
static char lookaside_buf[MAX_LOOKASIDE];

static int warn_const, warn_signed, warn_volatile, warn_enum, debug;


static void set_char_class(s, class)
char *s;
int class;
{
  /* Make every char of 's' (other than \0) a member of 'class' */
  int j, ch;
  for (j = 0, ch = s[0];  ch != 0;  ++j, ch = s[j]) char_class[ch] |= class;
}

static void init_char_class(void)
{
  int j;

  for (j = 0;  j < sizeof(char_class);  ++j) char_class[j] = 0;
  set_char_class("ABCDEFGHIJKLMNOPQRSTUVWXYZ",  ID_START_CL);
  set_char_class("abcdefghijklmnopqrstuvwxyz_", ID_START_CL);
  /* It is a deliberate lie that digits are in ID_START_CL */
  /* get_token() carefully corrects for this...            */
  set_char_class("0123456789", ID_START_CL + NUM_CL);
}

static void errf(char *fmt, ...)
{
  va_list ap;
  fprintf(stderr, "%s, %4d: ", cur_name, line_no);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

static int getch(int last_ch)
{
/*
 * Get the next character, possibly copying the previous character
 * to stdout or to a lookaside buffer.
 */
  if (line_no != 0) {
    if (lookaside)
      *lookaside++ = last_ch; 
    else {
      if (last_ch == '\n')  {
        copied_line_len = 0;
      } else
        ++copied_line_len;
      putc(last_ch, out_file);
    }
  }
  if (last_ch == '\n') ++line_no;
  return getc(in_file);
}

static int skip_blanks(int ch)
{
  while (ch == ' ' || ch == '\t') ch = getch(ch);
  return ch;
}

static int skip1_blanks(int ch)
{
  do {ch = getch(ch);} while (ch == ' ' || ch == '\t');
  return ch;
}

static void pp_directive(char *pp)
{ int temp;

  if (strcmp(pp, "if") == 0 ||
      strcmp(pp, "ifdef") == 0 ||
      strcmp(pp, "ifndef") == 0)
  {
    depth_stack[depth_sp++] = brace_depth;
  }
  else if (strcmp(pp, "else") == 0)
  {
    temp = brace_depth;
    if (depth_sp > 0)
      brace_depth = depth_stack[depth_sp-1];
    else
      brace_depth = depth_sp = 1;
    depth_stack[depth_sp-1] = temp;
  }
  else if (strcmp(pp, "endif") == 0)
  {
    if (depth_sp == 0)
      temp = 1;
    else
      temp = depth_stack[--depth_sp];
    if (brace_depth != temp)
      errf("confused by mismatched, conditionally included braces...\n");
  }
}

static int get_tokch(int ch)
{
/*
 * Get the next token-starting character.
 * next_ch is used to buffer a character of look ahead.
 */
  next_ch = 0;
  for (;;) {
    /*
     * skip pre-processor lines.
     */
    if (ch == '\n') {
      ch = skip1_blanks(ch);
      if (ch == '#') {
        /* pre-processor directive */
        int j = 0, last_ch;
        char pp[ID_MAX];
        ch = skip1_blanks(ch);
        do {pp[j++] = ch;  ch = getch(ch);} while (char_class[ch] & ID_CL);
        pp[j] = 0;
        pp_directive(pp);
        last_ch = 0;
        while ((ch != '\n' || last_ch == '\\') && ch != EOF)
          last_ch = ch, ch = getch(ch);
        continue;
      }
    }
    else ch = skip_blanks(ch);
    /* 
     * skip comments
     */
    if (ch == '/') {
      ch = getch(ch);
      if (ch == '*') {
        /* found '/' '*' ... */
        ch = getch(ch);
        do {
          while (ch != '*') ch = getch(ch);
          ch = getch(ch);
        } while (ch != '/');
        /* ... then get character following '*' '/' */
        ch = getch(ch);
      } else {
        /* save char following '/' and return '/' */
        next_ch = ch;
        return '/';
      }
    } else if (ch != '\n') {
      /* got a non-preprocessor-line, non-comment, non-space character */
      return ch;
    }
  }
}

static int skip_string(int quote)
{ int ch = quote;
/*
 * Skip a quoted string - beware '\'quote within the string.
 */
  for (;;) {
    ch = getch(ch);
    if (ch == quote) break;
    if (ch == '\\') ch = getch(ch);
  }
  /* ... and get the character beyond the end of the string */
  return getch(ch);
}

static void warn_deleted(keyword)
char *keyword;
{
  /* Assert: keyword is signed, const, or volatile */
  switch (keyword[0]) {
case 's':
    if (warn_signed++) return;
    break;
case 'c':
    if (warn_const++)  return;
    break;
case 'v':
    if (warn_volatile++) return;
    break;
  }
  errf("ANSI keyword '%s' removed from source\n", keyword);
}

static void put_id_or_number(id)
char *id;
{ char *s = id;
  int ch, l, u;

  while (isdigit(*s)) ++s;
  l = u = 0;
  if (s != id)
  { ch = *s++;
    if (ch == 'l' || ch == 'L')
    { l = 1;  ch = *s++;
      if (ch == 'u' || ch == 'U') u = 1;
    }
    else if (ch == 'u' || ch == 'U')
    { u = 1;  ch = *s++;
      if (ch == 'l' || ch == 'L') l = 1;
    }
  }
  if (u)
  {
    fputs("(unsigned", out_file);
    if (l)
      fputs(" long)", out_file);
    else
      putc(')', out_file);
    s[-2] = 0;
    fputs(id, out_file);
    if (l) putc('L', out_file);
  }
  else fputs(id, out_file);
}

static int checkfor_voidstar(int ch, int j)
{ int j0 = j;
  if (strcmp(id, "void") == 0) {
    while (ch == ' ' || ch == '\t') {id[j++] = ch;  ch = getch(ch);}
    id[j] = 0;
    if (ch == '*') {
      do {++j;  ch = getch(ch);} while (ch == ' ' || ch == '\t');
    }
    else j0 = j;
  }
  next_ch = ch;
  return j - j0;
}

static int id_or_number(int ch, int class)
{
  int void_star = 0, j;
  la_id_start = lookaside;
  if (lookaside == NULL) lookaside = lookaside_buf;
  j = 0;
  do {id[j++] = ch;  ch = getch(ch);} while (char_class[ch] & ID_CL);
  id[j] = 0;
  j += (void_star = checkfor_voidstar(ch, j));
  ch = next_ch;
  if (!void_star &&
      (strcmp(id, "signed")   == 0 ||
       strcmp(id, "const")    == 0 ||
       strcmp(id, "volatile") == 0))
  {
    /* want to delete it */
    while (ch == ' ' || ch == '\t') {++j;  ch = getch(ch);}
    if (la_id_start == NULL) lookaside = NULL; else lookaside -= j;
    warn_deleted(id);
  } else {
    /* don't delete it... */
    if (warn_enum == 0 && out_file != stdout && strcmp(id, "enum") == 0) {
      errf("source contains 'enum's\n");
      ++warn_enum;
    }
    if (la_id_start == NULL) {
      if (void_star)
        fputs("VoidStar ", out_file);
      else
        put_id_or_number(id);
      lookaside = NULL;
    } else if (void_star) {
      lookaside -= j;
      strcpy(lookaside, "VoidStar ");
      lookaside += 9;
    } else {
      /* trim id... */
      while (isspace(id[--j])) id[j] = 0;
    }
  }
  next_ch = ch;
  return (class == ID_START_CL ? ID_TOKEN : NUM_TOKEN);
}

static int get_token(void)
{
  int class, ch = next_ch;
/*
 * Here, we're looking for identifiers, brackets, etc.
 * Strings are skipped as being uninteresting.
 */
  while (ch != EOF) {
    ch = get_tokch(ch);
    if (ch == '\'' || ch == '"') {
      ch = skip_string(ch);
    } else if ((class = char_class[ch]) & ID_CL) {
      return id_or_number(ch, class);
    } else {
      /* non-identifier, non-string token... ch is just beyond it */
      if (!next_ch && ch != EOF) next_ch = getch(ch);
      return ch;
    }
  }
  return EOF;
}

static void skip_fn_body(void)
{
  int tok, depth = depth_sp;
  brace_depth = 1;
  do {
    tok = get_token();
    if (tok == '{') {
      ++brace_depth;
    } else if (tok == '}') {
      --brace_depth;
    }
  } while ((brace_depth > 0 || depth_sp > depth) && (tok != EOF));
  /* Assert: brace_depth == 0 && depth_sp == depth || tok == EOF */
  if (tok == EOF && (brace_depth || depth_sp))
    errf("missing '}' or conditional #include confusion at EOF\n");
}

static char *savestr(str, l)
char *str;
int l;
{
  char *s;
  if (l == 0) l = strlen(str);
  s = malloc(l + 1);
  if (s == NULL) {
     errf("can't allocate space for %s\n", id);
     exit(1);
  }
  strncpy(s, str, l);
  s[l] = 0;
  return s;
}

static int formal_argument_list(void)
{
  int tok, paren_ct;
  char *arg;
  /* check for id, id, ... */
  narg_ids = 0;
  paren_ct = 0;
  arg = NULL;
  for (;;) {
    tok = get_token();
    if (arg == NULL) {
      if (tok == ID_TOKEN || tok == NUM_TOKEN)
        arg = la_id_start;
      else if (tok == '.')
      { arg = lookaside-1;
        while (tok == '.') tok = get_token();
      }
      else
        arg = lookaside;
    }
    if (tok == '(') ++paren_ct;
    else if ((tok == ',' || tok == ')') && paren_ct == 0) {
      if (arg != lookaside)
      { if (strncmp(arg, "...", 3) == 0)
          arg_id[narg_ids++] = savestr("int va_alist", 0);
        else
          arg_id[narg_ids++] = savestr(arg, lookaside - arg - 1);
        arg = NULL;
      }
      if (tok == ')') break;
    } else if (tok == ')') --paren_ct;
  }
  if (narg_ids == 1)
  { arg = arg_id[0];
    if (strcmp(id, "void") == 0)
    { narg_ids = 0;
      free(arg);
    }
  }
  return tok;
}

static void print_last_id_in(arg)
char *arg;
{
  /* Extract the last identifier from arg and print it */
  char *s, *end;
  s = arg + strlen(arg);     /* just past the end of s (*s == 0) */
  while (s != arg && (char_class[*s] & ID_CL) == 0) --s;
  /* Assert: s == arg || (char_class[*s] & ID_CL) != 0 */
  end = s;
  while (s != arg && (char_class[*s] & ID_CL) != 0) --s;
  /* Assert: s == arg || (char_class[*s] & ID_CL) == 0 */
  if ((char_class[*s] & ID_CL) == 0) ++s;
  /* want to print [s,end] */
  for (;;) {
    putc(*s, out_file);
    if (s == end) break;
    ++s;
  }
}

static void rewrite_fn_defn(void)
{
  int j;
  char *s;

  if (narg_ids == 0)
    fputs(")\n", out_file);
  else
  {
    for (j = 0;  j < narg_ids;  ++j) {
      char *arg = arg_id[j];
      print_last_id_in(arg);
      if (j < (narg_ids-1))
        fputs(", ", out_file);
      else
        fputs(")\n", out_file);
    }
    for (j = 0;  j < narg_ids;  ++j) {
      fprintf(out_file, "%s;\n", arg_id[j]);
    }
  }
/*
 * Now see if any comments lie between the args and the '{'.
 */
  s = lookaside;
  *s = 0;
  s -= 2;
  for (;;) {
    while (s > lookaside_buf && *s != ')' && *s != '/') --s;
    /* Assert: *s == ')' or '/' */
    if (*s != '/') break;
    if (s[-1] == '*') {
      s -= 2;
      while (*s != '*' || s[-1] != '/') --s;
      s -= 2;
    } else {
      --s;
    }
  }
  ++s;
  while (isspace(*s) && *s != '\n') ++s;
  if (*s == '\n') ++s;
  fputs(s, out_file);
}

static void rewrite_fn_decl(void)
{
  char *s, *e;
  s = lookaside_buf;
  while (isspace(*s)) ++s;
  /* Assert: *s != <space> */
  e = lookaside;
  while (*(--e) != ')');
  /* Assert: *e == ')' */
  if (s != e)
  { *e = 0;
    fputs("/* ", out_file);
    while (s != e)
    {
      if (s[0] == '*' && s[1] == '/' || s[0] == '/' && s[1] == '*') ++s;
      else putc(*s, out_file);
      ++s;
    }
    fputs(" */", out_file);
    *e = ')';
  }
  *lookaside = 0;
  fputs(e, out_file);
}

static int is_basic_type(char *s)
{
  static char *keywds[] = { /* MUST be in alphabetic order */
    "char",
    "double",
    "enum",
    "extern",
    "float",
    "int",
    "long",
    "short",
    "static",
  };
  int j, s0 = s[0], k0;
  char *k;
  for (j = 0;  j < sizeof(keywds)/sizeof(char *);  ++j)
  { k = keywds[j];
    k0 = k[0];
    if (k0 > s0) return 0;
    if (k0 < s0) continue;
    if (strcmp(s, k) == 0) return 1;
  }
  return 0;
}

static void translate(void)
{
  int tok, last_tok;

  init_char_class();
  lookaside = NULL;
  copied_line_len = 0;
  brace_depth = 0;
  depth_sp = 0;
  line_no = 0;
  next_ch = '\n';       /* fake being just past a new-line */

  last_tok = get_token();
  while (last_tok != EOF) {
    tok = get_token();
    if (last_tok == ID_TOKEN && tok == '(' && !is_basic_type(id)) {
      lookaside = lookaside_buf;
      strcpy(cur_fn, id);
      tok = formal_argument_list();
      if (tok == ')') {
        tok = get_token();
        if (tok == '{') {
          rewrite_fn_defn();
        } else if (tok == ';') {
          rewrite_fn_decl();
        } else {
          /* presume it's a pcc-style header already... */
          int argct = narg_ids;
          *lookaside = 0;
          fputs(lookaside_buf, out_file);
          lookaside = NULL;
          while (tok != '{' && tok != EOF && argct > 0)
          { if (tok == ';') --argct;
            tok = get_token();
          }
        }
      } else {
        /* probably we've botched the recognition and its not a */
        /* fn so just copy it unchanged to the output file.     */
        *lookaside = 0;
        fputs(lookaside_buf, out_file);
      }
      lookaside = NULL;
      if (tok == '{') {skip_fn_body();  tok = '}';}
    }
    last_tok = tok;
  }
}

static void describe_function(void)
{
  fprintf(stderr, "\n%s \
translates a (suitable) ANSI C source to a form acceptable to a pcc-\n\
compatible C compiler. Top-level function declarations get their arguments\n\
enclosed in /* and */ (original /* and */ tokens are removed); function\n\
definition prototypes are re-written the pcc way (e.g. void foo(int x) {...}\n\
-> void foo(x) int x; {...}); ANSI keywords const, signed, and volatile are\n\
elided (with a warning); enums are warned of (stricter usage under pcc);\n\
type void * is converted to VoidStar, which should be typedef'd to char * to\n\
be compatible with pcc; ANSI unsigned [long] int constants are re-written\n\
as (unsigned [long])<int>[L] (e.g. 300ul -> (unsigned long)300L)\n\n\
Other differences must be addressed in the source before or after translation.\
\n", SELF);
}

static void handle_escape(int signo)
{
  signal(signo, handle_escape);
  exit(EXIT_FAILURE);
}

static void give_help(void)
{
  fprintf(stderr, "\n%s vsn %s [%s] - %s\n", SELF, VSN, DATE, BRIEF);
  fprintf(stderr, "\n%s [options] [infile [outfile]]\n", SELF);
  fprintf(stderr, "    outfile defaults to stdout\n");
  fprintf(stderr, "    infile  defaults to stdin\n");
  fprintf(stderr, "\nOptions:-\n");
  fprintf(stderr, "-d  describe what the program does\n");
  fprintf(stderr, "\nExample:-\n");
  fprintf(stderr, "      %s c.pccprog c.ansiprog\n", SELF);
  exit(EXIT_SUCCESS);
}

int main(argc, argv)
int argc;
char *argv[];
{
  char *arg, *in_name, *out_name;
  int j, nerr;

  signal(SIGINT, handle_escape);

  /* parse help or identify args */
  for (j = 1;  j < argc;  ++j) {
    arg = argv[j];
    if (strcmp("-help", arg) == 0 || strcmp("-h", arg) == 0 ||
        strcmp("-HELP", arg) == 0 || strcmp("-H", arg) == 0)
      give_help();
  }

  in_name = out_name = NULL;
  nerr = debug = 0;
  for (j = 1;  j < argc;  ++j) {
    arg = argv[j];
    if (arg[0] == '-') {
      if (arg[1] == 'z' || arg[1] == 'Z')
        ++debug;
      else if (arg[1] == 'd' || arg[1] == 'D') {
        describe_function();
        exit(OK);
      } else {
        fprintf(stderr, "topcc: unknown flag '%s'\n", arg);
        ++nerr;
      }
    } else {
      if (in_name == NULL) in_name = arg;
      else if (out_name == NULL) out_name = arg;
      else {
        fprintf(stderr, "too many filenames: '%s' ignored\n", arg);
        ++nerr;
      }
    }
  }

  if (nerr) exit(BAD);

  if (in_name)
  { in_file = fopen(in_name, "r");
    if (in_file == NULL)
    { fprintf(stderr, "%s: can't open input file '%s' - assuming <stdin>\n",
                      SELF, in_name);
      fprintf(stderr, "%s: (press CTRL+D to end input)\n", SELF);
    }
  }
  else in_file = NULL;
  if (in_file == NULL)
  { in_file = stdin;
    cur_name = "<stdin>";
  }
  else cur_name = in_name;

  if (out_name) out_file = fopen(out_name, "w");
  else out_file = NULL;
  if (out_file == NULL) out_file = stdout;

  warn_const = warn_signed = warn_volatile = warn_enum = 0;

  translate();

  return OK;
}
