/*
 * Tool to translate between ANSI and pcc-style function headers.
 * Copyright (C) Acorn Computers, 1988, 1989.
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>

#define  SELF         "toansi"
#define  VSN          "3.00/013"
#define  BRIEF        "convert a C program from PCC to ANSI dialect"
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
#define  ROUND            32
#define  MAX_ARGS        128
#define  MAX_LOOKASIDE  2048
#define  HEADER_BRKLEN    72
#define  DEPTH_STACKSZ    50

static FILE *in_file, *out_file;

static int next_ch, copied_line_len, line_no, brace_depth, depth_sp;
static int depth_stack[DEPTH_STACKSZ];

static char id[ID_MAX], cur_fn[ID_MAX];

static char char_class[256];

static char *arg_id[MAX_ARGS];
static char *ansi_decl[MAX_ARGS];
static int narg_ids;

static char *lookaside, *la_id_start, *cur_name;
static char lookaside_buf[MAX_LOOKASIDE];


static void set_char_class(s, class)
char *s;
int class;
{
  /* Make every char of 's' (other than \0) a member of 'class' */
  while (*s) char_class[*s++] |= class;
}

static void init_char_class(void)
{
  int j;

  for (j = 0;  j < sizeof(char_class);  ++j) char_class[j] = 0;
  set_char_class("ABCDEFGHIJKLMNOPQRSTUVWXYZ",  ID_START_CL);
  set_char_class("abcdefghijklmnopqrstuvwxyz_", ID_START_CL);
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

static int get_token()
{
  int j, class;
  int ch = next_ch;
/*
 * Here, we're looking for identifiers, brackets, etc.
 * Strings are skipped as being uninteresting.
 */
  while (ch != EOF) {
    ch = get_tokch(ch);
    if (ch == '\'' || ch == '"') {
      ch = skip_string(ch);
    } else if ((class = char_class[ch]) & ID_CL) {
      la_id_start = lookaside;
      j = 0;
      do {id[j++] = ch;  ch = getch(ch);} while (char_class[ch] & ID_CL);
      id[j] = 0;
      next_ch = ch;
      return (class == ID_START_CL ? ID_TOKEN : NUM_TOKEN);
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

static char *savestr(str)
char *str;
{
  int l = strlen(str);
  char *s;
  l = (l + (ROUND-1)) & ~(ROUND-1);
  s = malloc(l);
  if (s == NULL) {
    errf("can't allocate space for %s\n", id);
    exit(1);
  }
  return strcpy(s, str);
}

static int formal_argument_list(void)
{
  int tok, is_ansi;
  /* check for id, id, ... */
  narg_ids = 0;
  is_ansi = 0;
  tok = get_token();
  while (tok != ')') {
    if (tok == ID_TOKEN) {
      ansi_decl[narg_ids] = NULL;
      arg_id[narg_ids++] = savestr(id);
    }
    tok = get_token();
    if (tok == ',') {
      tok = get_token();
    } else if (tok != ')') {
      is_ansi = 1;
    }
  }
  if (is_ansi) tok = get_token();            /* often ';' or '{' */
  if ((tok != ')') ||
      (narg_ids == 1) && (strcmp(id, "void") == 0)) {
    /* cleanup */
    while (narg_ids > 0) {free(arg_id[--narg_ids]);  arg_id[narg_ids] = NULL;};
  }
  return tok;
}

static void process_decl(prefix, str, id)
char * prefix;
char *str;
char *id;
{
  int j;
  char *decl, *s;
  char buf[ID_MAX+ID_MAX];

/*
 * Trim trailing blanks from id
 */
  s = id + strlen(id);
  while (*--s == ' ');
  s[1] = 0;
/*
 * Trim leading and trailing blanks from prefix and str.
 */
  while (*prefix == ' ') ++prefix;
  s = prefix + strlen(prefix) - 1;
  while (*s == ' ') --s;
  s[1] = 0;
  while (*str == ' ') ++str;
  s = str + strlen(str) - 1;
  while (*s == ' ') --s;
  s[1] = 0;
  for (j = 0;  j < narg_ids;  ++j) {
    if (strcmp(arg_id[j], id) == 0) {
      decl = ansi_decl[j];
      if (decl != NULL) {
        errf("duplicate definition of argument '%s' to '%s' ignored\n",
             id, cur_fn);
      } else {
        while ((char_class[*prefix] & ID_START_CL) == 0) ++prefix;
        sprintf(buf, "%s %s", prefix, str);
        ansi_decl[j] = savestr(buf);
      }
      return;
    }
  }
  errf("'%s' is not an argument to '%s'\n", id, cur_fn);
}

static int match_argument_decl(void)
{
  int tok, l;
  char *p, *d, *s;
  char prefix[ID_MAX], decl[ID_MAX], decl_id[ID_MAX];
/*
 * An approximation which often works is:-
 *   <decl> ::= <id> (<id>)* <thing> ( "," <thing> )*
 *   <thing>::= ("*")* <id> ("[" [<id>|<num>] "]")*
 * Assert: enter with id found;
 */
  p = NULL;
  d = la_id_start;
  for (;;) {
    s = lookaside;
    tok = get_token();
    if (tok != ID_TOKEN) break;
    if (p == NULL) p = d;
    d = la_id_start;
  }
  if (tok != '*') {
    if (p) {
      l = d - p;
      strncpy(prefix, p, l);  prefix[l] = 0;
    } else {
      strcpy(prefix, "int ");
    }
    l = lookaside - d - 1;
    strncpy(decl_id, d, l);  decl_id[l] = 0;
    strcpy(decl, decl_id);
  } else {
    if (p) d = p;
    s = lookaside -1;
    l = s - d;
    strncpy(prefix, d, l);
    prefix[l] = 0;
    d = s;
  }
again:
  while (tok == '*') tok = get_token();
  if (tok == ID_TOKEN) {
    strcpy(decl_id, id);
    tok = get_token();
  }
  while (tok == '[') {
    tok = get_token();
    while (tok == ID_TOKEN || tok == NUM_TOKEN) {
      tok = get_token();
      if (tok == ',') tok = get_token();
    }
    if (tok != ']') {
      errf("missing ']' in declaration of %s in %s\n", decl_id, cur_fn);
      return 0;
    } else {
      tok = get_token();
    }
  }
  if (tok == ','  || tok == ';') {
    l = lookaside - d - 1;
    strncpy(decl, d, l);  decl[l] = 0;
    process_decl(prefix, decl, decl_id);
    if (tok == ',') {
      d = lookaside;
      tok = get_token();
      goto again;
    }
  }
  if (tok != ';')
    errf("failed matching arg list for %s (missing ';')\n", cur_fn);
  return tok;
}

static int match_argument_decls()
{ int tok;
/*
 * Match:-
 *   ( <decl> ";")*
 *   <decl> ::= <id> (<id>)* ...
 */
  tok = get_token();
  if (tok == ';') return ';';     /* a declaration! */
  while (tok == ID_TOKEN) {
    tok = match_argument_decl();
    if (tok != ';') {
      return 0;
    } else {
      tok = get_token();
    }
  }
  if (tok != '{') {
    errf("failed matching arg list for %s (missing '{')\n", cur_fn);
  }
  return tok;
}

static void put_spaces(n)
int n;
{
  while (n-- > 0) putc(' ', out_file);
}

static void rewrite_fn_defn(void)
{
  int j, width;
  char *decl, *s;
  char buf[ID_MAX];
  
  if (narg_ids == 0) fprintf(out_file, "void)\n");
  width = copied_line_len;
  for (j = 0;  j < narg_ids; ++j) {
    decl = ansi_decl[j];
    if (decl == NULL) {
      if (strcmp(arg_id[j], "va_alist") == 0) {
        decl = "...";
      } else {
        decl = buf;
        sprintf(decl, "int %s", arg_id[j]);
      }
    }
    if ((width + strlen(decl) + 2) > HEADER_BRKLEN) {
      width = copied_line_len;
      putc('\n', out_file);
      put_spaces(width);
    }
    width += fprintf(out_file, "%s", decl);
    free(ansi_decl[j]);  free(arg_id[j]);
    ansi_decl[j] = arg_id[j] = NULL;
    width += fprintf(out_file, "%s", (j < (narg_ids-1) ? ", " : ")\n"));
  }
/*
 * Now see if any comments lie between the args and the '{'.
 */
  s = lookaside;
  *s = 0;
  s -= 2;
  for (;;) {
    while (s > lookaside_buf && *s != ';' && *s != ')' && *s != '/') --s;
    /* Assert: *s == ';', ')' or '/' */
    if (*s != '/') break;
    if (*s == '/' && s[-1] == '*') {
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

static int rewrite_fn_decl(char *endp)
{
/*
 * Strip out enclosing comment brackets if there are any...
 * This ussumes that that which is so enclosed is a valid ANSI argument list.
 */
  char *s, *e;
  s = lookaside_buf;
  while (isspace(*s)) ++s;
  /* Assert: *s != <space> */
  if (*s != '/' || s[1] != '*') return 0;
  s += 2;
  while (isspace(*s)) ++s;
  e = endp;
  while (e != s && *e-- != '/');
  if (e == s || *e != '*') return 0;
  while (isspace(e[-1])) --e;
  *e = 0;
  fputs(s, out_file);
  *lookaside = 0;
  fputs(endp, out_file);
  return 1;
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
        char *endp = lookaside-1;
        tok = match_argument_decls();
        if (tok == '{') {
          rewrite_fn_defn();
        } else if (tok == ';' && narg_ids == 0 && rewrite_fn_decl(endp)) {
          /* do nothing */
        } else {
          *lookaside = 0;
          fputs(lookaside_buf, out_file);
        }
      } else {
        *lookaside = 0;
        fputs(lookaside_buf, out_file);
      }
      lookaside = NULL;
      if (tok == '{') {skip_fn_body();  tok = '}';}
    }
    last_tok = tok;
  }
}

static void handle_escape(int signo)
{
  signal(signo, handle_escape);
  exit(EXIT_FAILURE);
}

static void describe_function(void)
{
  fprintf(stderr, "\n%s \
translates a (suitable) K&R-style or PCC-style C source program to a\n\
form acceptable to an ANSI-C compiler. Top-level function declarations with\n\
comments imbedded within their '()'s have the '/*'s and '*/'s removed on the\n\
assumption that they contain ANSI-style formal argument lists. Function\n\
definition prototypes are re-written the ANSI way (e.g. void foo(x) int x;\n\
{...} is re-written as void foo(int x) {...}).\n\n\
Other differences between the dialects must be addressed in the source\n\
before or after translation.\n\n",
  SELF);
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
  nerr = 0;
  for (j = 1;  j < argc;  ++j) {
    arg = argv[j];
    if (arg[0] == '-') {
      if (arg[1] == 'd' || arg[1] == 'D') {
        describe_function();
        exit(OK);
      } else {
        fprintf(stderr, "%s: unknown flag '%s'\n", SELF, arg);
        ++nerr;
      }
    } else {
      if (in_name == NULL) in_name = arg;
      else if (out_name == NULL) out_name = arg;
      else {
        fprintf(stderr, "%s: too many filenames ('%s' ignored)\n", SELF, arg);
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

  translate();

  return 0;
}
