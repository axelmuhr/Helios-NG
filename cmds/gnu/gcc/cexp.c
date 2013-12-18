
# line 27 "cexp.y"
#include "config.h"
#include <setjmp.h>
/* #define YYDEBUG 1 */

  int yylex ();
  void yyerror ();
  int expression_value;

  static jmp_buf parse_return_error;

  /* some external tables of character types */
  extern unsigned char is_idstart[], is_idchar[];


# line 42 "cexp.y"
typedef union  {
  long lval;
  int voidval;
  char *sval;
} YYSTYPE;
# define INT 257
# define CHAR 258
# define NAME 259
# define ERROR 260
# define OR 261
# define AND 262
# define EQUAL 263
# define NOTEQUAL 264
# define LEQ 265
# define GEQ 266
# define LSH 267
# define RSH 268
# define UNARY 269
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yylval, yyval;
typedef int yytabelem;
# define YYERRCODE 256

# line 148 "cexp.y"


/* During parsing of a C expression, the pointer to the next character
   is in this variable.  */

static char *lexptr;

/* Take care of parsing a number (anything that starts with a digit).
   Set yylval and return the token type; update lexptr.
   LEN is the number of characters in it.  */

/* maybe needs to actually deal with floating point numbers */

int
parse_number (olen)
     int olen;
{
  register char *p = lexptr;
  register long n = 0;
  register int c;
  register int base = 10;
  register len = olen;

  extern double atof ();

  for (c = 0; c < len; c++)
    if (p[c] == '.') {
      /* It's a float since it contains a point.  */
      yyerror ("floating point numbers not allowed in #if expressions");
      return ERROR;
      
/* ****************
	 yylval.dval = atof (p);
	 lexptr += len;
	 return FLOAT;
		 ****************  */
    }
  
  if (len >= 3 && (!strncmp (p, "0x", 2) || !strncmp (p, "0X", 2))) {
    p += 2;
    base = 16;
    len -= 2;
  }
  else if (*p == '0')
    base = 8;
  
  while (len-- > 0) {
    c = *p++;
    n *= base;
    if (c >= '0' && c <= '9')
      n += c - '0';
    else {
      if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
      if (base == 16 && c >= 'a' && c <= 'f')
	n += c - 'a' + 10;
      else if (len == 0 && c == 'l')
	;
      else {
	yyerror ("Invalid number in #if expression");
	return ERROR;
      }
    }
  }

  lexptr = p;
  yylval.lval = n;
  return INT;
}

struct token {
  char *operator;
  int token;
};

#define NULL 0

static struct token tokentab2[] = {
  {"&&", AND},
  {"||", OR},
  {"<<", LSH},
  {">>", RSH},
  {"==", EQUAL},
  {"!=", NOTEQUAL},
  {"<=", LEQ},
  {">=", GEQ},
  {NULL, ERROR}
};

/* Read one token, getting characters through lexptr.  */

int
yylex ()
{
  register int c;
  register int namelen;
  register char *tokstart;
  register struct token *toktab;

 retry:

  tokstart = lexptr;
  c = *tokstart;
  /* See if it is a special token of length 2.  */
  for (toktab = tokentab2; toktab->operator != NULL; toktab++)
    if (c == *toktab->operator && tokstart[1] == toktab->operator[1]) {
      lexptr += 2;
      return toktab->token;
    }

  switch (c) {
  case 0:
    return 0;
    
  case ' ':
  case '\t':
  case '\n':
    lexptr++;
    goto retry;
    
  case '\'':
    lexptr++;
    c = *lexptr++;
    if (c == '\\')
      c = parse_escape (&lexptr);
    yylval.lval = c;
    c = *lexptr++;
    if (c != '\'') {
      yyerror ("Invalid character constant in #if");
      return ERROR;
    }
    
    return CHAR;

    /* some of these chars are invalid in constant expressions;
       maybe do something about them later */
  case '/':
  case '+':
  case '-':
  case '*':
  case '%':
  case '|':
  case '&':
  case '^':
  case '~':
  case '!':
  case '@':
  case '<':
  case '>':
  case '(':
  case ')':
  case '[':
  case ']':
  case '.':
  case '?':
  case ':':
  case '=':
  case '{':
  case '}':
  case ',':
    lexptr++;
    return c;
    
  case '"':
    yyerror ("double quoted strings not allowed in #if expressions");
    return ERROR;
  }
  if (c >= '0' && c <= '9') {
    /* It's a number */
    for (namelen = 0;
	 c = tokstart[namelen], is_idchar[c] || c == '.'; 
	 namelen++)
      ;
    return parse_number (namelen);
  }
  
  if (!is_idstart[c]) {
    yyerror ("Invalid token in expression");
    return ERROR;
  }
  
  /* It is a name.  See how long it is.  */
  
  for (namelen = 0; is_idchar[tokstart[namelen]]; namelen++)
    ;
  
  lexptr += namelen;
  return NAME;
}


/* Parse a C escape sequence.  STRING_PTR points to a variable
   containing a pointer to the string to parse.  That pointer
   is updated past the characters we use.  The value of the
   escape sequence is returned.

   A negative value means the sequence \ newline was seen,
   which is supposed to be equivalent to nothing at all.

   If \ is followed by a null character, we return a negative
   value and leave the string pointer pointing at the null character.

   If \ is followed by 000, we return 0 and leave the string pointer
   after the zeros.  A value of 0 does not mean end of string.  */

int
parse_escape (string_ptr)
     char **string_ptr;
{
  register int c = *(*string_ptr)++;
  switch (c)
    {
    case 'a':
      return '\a';
    case 'b':
      return '\b';
    case 'e':
      return 033;
    case 'f':
      return '\f';
    case 'n':
      return '\n';
    case 'r':
      return '\r';
    case 't':
      return '\t';
    case 'v':
      return '\v';
    case '\n':
      return -2;
    case 0:
      (*string_ptr)--;
      return 0;
    case '^':
      c = *(*string_ptr)++;
      if (c == '\\')
	c = parse_escape (string_ptr);
      if (c == '?')
	return 0177;
      return (c & 0200) | (c & 037);
      
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
      {
	register int i = c - '0';
	register int count = 0;
	while (++count < 3)
	  {
	    if ((c = *(*string_ptr)++) >= '0' && c <= '7')
	      {
		i *= 8;
		i += c - '0';
	      }
	    else
	      {
		(*string_ptr)--;
		break;
	      }
	  }
	return i;
      }
    default:
      return c;
    }
}

void
yyerror (s)
     char *s;
{
  error (s);
  longjmp (parse_return_error, 1);
}

/* This page contains the entry point to this file.  */

/* Parse STRING as an expression, and complain if this fails
   to use up all of the contents of STRING.  */
/* We do not support C comments.  They should be removed before
   this function is called.  */

int
parse_c_expression (string)
     char *string;
{
  lexptr = string;
  
  if (lexptr == 0 || *lexptr == 0) {
    error ("empty #if expression");
    return 0;			/* don't include the #if group */
  }

  /* if there is some sort of scanning error, just return 0 and assume
     the parsing routine has printed an error message somewhere.
     there is surely a better thing to do than this.     */
  if (setjmp(parse_return_error))
    return 0;

  if (yyparse ())
    return 0;			/* actually this is never reached
				   the way things stand. */
  if (*lexptr)
    error ("Junk after end of expression.");

  return expression_value;	/* set by yyparse() */
}

#ifdef TEST_EXP_READER
/* main program, for testing purposes. */
main()
{
  int n;
  char buf[1024];
  extern int yydebug;
/*
  yydebug = 1;
*/
  initialize_random_junk ();

  for (;;) {
    printf("enter expression: ");
    n = 0;
    while ((buf[n] = getchar()) != '\n')
      n++;
    buf[n] = '\0';
    printf("parser returned %d\n", parse_c_expression(buf));
  }
}

/* table to tell if char can be part of a C identifier. */
char is_idchar[256];
/* table to tell if char can be first char of a c identifier. */
char is_idstart[256];
/* table to tell if c is horizontal space.  isspace() thinks that
   newline is space; this is not a good idea for this program. */
char is_hor_space[256];

/*
 * initialize random junk in the hash table and maybe other places
 */
initialize_random_junk()
{
  register int i;

  /*
   * Set up is_idchar and is_idstart tables.  These should be
   * faster than saying (is_alpha(c) || c == '_'), etc.
   * Must do set up these things before calling any routines tthat
   * refer to them.
   */
  for (i = 'a'; i <= 'z'; i++) {
    ++is_idchar[i - 'a' + 'A'];
    ++is_idchar[i];
    ++is_idstart[i - 'a' + 'A'];
    ++is_idstart[i];
  }
  for (i = '0'; i <= '9'; i++)
    ++is_idchar[i];
  ++is_idchar['_'];
  ++is_idstart['_'];
#ifdef DOLLARS_IN_IDENTIFIERS
  ++is_idchar['$'];
  ++is_idstart['$'];
#endif

  /* horizontal space table */
  ++is_hor_space[' '];
  ++is_hor_space['\t'];
}

error (msg)
{
  printf("error: %s\n", msg);
}
#endif
yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 30
# define YYLAST 340
yytabelem yyact[]={

    14,    25,    11,     2,    14,    12,    15,     1,    16,    12,
    13,    34,    14,    25,    13,     0,     0,    12,    15,     0,
    16,    56,    13,    23,    14,    24,    30,     0,     0,    12,
    15,     0,    16,    55,    13,    23,    11,    24,    30,    14,
    25,     0,     0,     0,    12,    15,     0,    16,     0,    13,
    14,    25,     0,     0,     0,    12,    15,    26,    16,     0,
    13,     0,    23,     0,    24,     0,    14,    25,     0,    26,
     0,    12,    15,    23,    16,    24,    13,     0,    14,    25,
     0,     0,     0,    12,    15,     0,    16,    27,    13,    23,
     0,    24,    14,     0,     0,     0,    26,    12,    15,    27,
    16,    23,    13,    24,    14,     0,     0,    26,     0,    12,
    15,     0,    16,     5,    13,    23,     0,    24,     0,     0,
     7,     0,     0,    26,     0,     4,    27,    23,     3,    24,
     0,     0,     0,    31,    32,    33,     0,    27,     0,     0,
    35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
    45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
    14,     0,     0,     0,     0,    12,    15,     0,    16,     0,
    13,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    57,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     6,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    29,    28,    19,    20,    21,    22,
    17,    18,     0,     0,     0,     0,    29,    28,    19,    20,
    21,    22,    17,    18,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    17,    18,     0,     0,     0,     0,
     0,     0,     0,     0,    28,    19,    20,    21,    22,    17,
    18,     0,     0,     0,     0,     0,    19,    20,    21,    22,
    17,    18,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,    19,    20,    21,    22,    17,    18,     0,     0,
     0,     0,     0,     0,    19,    20,    21,    22,    17,    18,
     0,     0,     0,     0,     0,     0,     0,     0,    19,    20,
    21,    22,    17,    18,     0,     0,     0,     0,     0,     0,
     0,     0,    21,    22,    17,    18,     0,     8,     9,    10 };
yytabelem yypact[]={

    80, -1000,   -42,   -25,    80,    80,    80,    80, -1000, -1000,
 -1000,    80,    80,    80,    80,    80,    80,    80,    80,    80,
    80,    80,    80,    80,    80,    80,    80,    80,    80,    80,
    80, -1000, -1000, -1000,    -8,   -25, -1000, -1000, -1000,   -33,
   -33,   123,   123,    67,    67,   -13,   -13,   -13,   -13,    55,
    41,    29,    13,     2,   -37, -1000,    80,   -25 };
yytabelem yypgo[]={

     0,   128,     3,     7 };
yytabelem yyr1[]={

     0,     3,     2,     2,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1 };
yytabelem yyr2[]={

     0,     3,     2,     7,     5,     5,     5,     7,     7,     7,
     7,     7,     7,     7,     7,     7,     7,     7,     7,     7,
     7,     7,     7,     7,     7,     7,    11,     3,     3,     3 };
yytabelem yychk[]={

 -1000,    -3,    -2,    -1,    45,    33,   126,    40,   257,   258,
   259,    44,    42,    47,    37,    43,    45,   267,   268,   263,
   264,   265,   266,    60,    62,    38,    94,   124,   262,   261,
    63,    -1,    -1,    -1,    -2,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    -1,    -1,    -1,    -1,    -1,    41,    58,    -1 };
yytabelem yydef[]={

     0,    -2,     1,     2,     0,     0,     0,     0,    27,    28,
    29,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     4,     5,     6,     0,     3,     8,     9,    10,    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,     0,     7,     0,    26 };
typedef struct { char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"INT",	257,
	"CHAR",	258,
	"NAME",	259,
	"ERROR",	260,
	"?",	63,
	":",	58,
	",",	44,
	"OR",	261,
	"AND",	262,
	"|",	124,
	"^",	94,
	"&",	38,
	"EQUAL",	263,
	"NOTEQUAL",	264,
	"<",	60,
	">",	62,
	"LEQ",	265,
	"GEQ",	266,
	"LSH",	267,
	"RSH",	268,
	"+",	43,
	"-",	45,
	"*",	42,
	"/",	47,
	"%",	37,
	"UNARY",	269,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"start : exp1",
	"exp1 : exp",
	"exp1 : exp1 ',' exp",
	"exp : '-' exp",
	"exp : '!' exp",
	"exp : '~' exp",
	"exp : '(' exp1 ')'",
	"exp : exp '*' exp",
	"exp : exp '/' exp",
	"exp : exp '%' exp",
	"exp : exp '+' exp",
	"exp : exp '-' exp",
	"exp : exp LSH exp",
	"exp : exp RSH exp",
	"exp : exp EQUAL exp",
	"exp : exp NOTEQUAL exp",
	"exp : exp LEQ exp",
	"exp : exp GEQ exp",
	"exp : exp '<' exp",
	"exp : exp '>' exp",
	"exp : exp '&' exp",
	"exp : exp '^' exp",
	"exp : exp '|' exp",
	"exp : exp AND exp",
	"exp : exp OR exp",
	"exp : exp '?' exp ':' exp",
	"exp : INT",
	"exp : CHAR",
	"exp : NAME",
};
#endif /* YYDEBUG */

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-1000)

/*
** global variables used by the parser
*/
YYSTYPE yyv[ YYMAXDEPTH ];	/* value stack */
int yys[ YYMAXDEPTH ];		/* state stack */

YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
int
yyparse()
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

	goto yystack;
	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ YYMAXDEPTH ] )	/* room on stack? */
		{
			yyerror( "yacc stack overflow" );
			YYABORT;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = yylex() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
				yynerrs++;
			skip_init:
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 72 "cexp.y"
{ expression_value = yypvt[-0].lval; } break;
case 3:
# line 78 "cexp.y"
{ yyval.lval = yypvt[-0].lval; } break;
case 4:
# line 83 "cexp.y"
{ yyval.lval = - yypvt[-0].lval; } break;
case 5:
# line 85 "cexp.y"
{ yyval.lval = ! yypvt[-0].lval; } break;
case 6:
# line 87 "cexp.y"
{ yyval.lval = ~ yypvt[-0].lval; } break;
case 7:
# line 89 "cexp.y"
{ yyval.lval = yypvt[-1].lval; } break;
case 8:
# line 94 "cexp.y"
{ yyval.lval = yypvt[-2].lval * yypvt[-0].lval; } break;
case 9:
# line 96 "cexp.y"
{ if (yypvt[-0].lval == 0)
			    {
			      error ("division by zero in #if");
			      yypvt[-0].lval = 1;
			    }
			  yyval.lval = yypvt[-2].lval / yypvt[-0].lval; } break;
case 10:
# line 103 "cexp.y"
{ if (yypvt[-0].lval == 0)
			    {
			      error ("division by zero in #if");
			      yypvt[-0].lval = 1;
			    }
			  yyval.lval = yypvt[-2].lval % yypvt[-0].lval; } break;
case 11:
# line 110 "cexp.y"
{ yyval.lval = yypvt[-2].lval + yypvt[-0].lval; } break;
case 12:
# line 112 "cexp.y"
{ yyval.lval = yypvt[-2].lval - yypvt[-0].lval; } break;
case 13:
# line 114 "cexp.y"
{ yyval.lval = yypvt[-2].lval << yypvt[-0].lval; } break;
case 14:
# line 116 "cexp.y"
{ yyval.lval = yypvt[-2].lval >> yypvt[-0].lval; } break;
case 15:
# line 118 "cexp.y"
{ yyval.lval = (yypvt[-2].lval == yypvt[-0].lval); } break;
case 16:
# line 120 "cexp.y"
{ yyval.lval = (yypvt[-2].lval != yypvt[-0].lval); } break;
case 17:
# line 122 "cexp.y"
{ yyval.lval = (yypvt[-2].lval <= yypvt[-0].lval); } break;
case 18:
# line 124 "cexp.y"
{ yyval.lval = (yypvt[-2].lval >= yypvt[-0].lval); } break;
case 19:
# line 126 "cexp.y"
{ yyval.lval = (yypvt[-2].lval < yypvt[-0].lval); } break;
case 20:
# line 128 "cexp.y"
{ yyval.lval = (yypvt[-2].lval > yypvt[-0].lval); } break;
case 21:
# line 130 "cexp.y"
{ yyval.lval = (yypvt[-2].lval & yypvt[-0].lval); } break;
case 22:
# line 132 "cexp.y"
{ yyval.lval = (yypvt[-2].lval ^ yypvt[-0].lval); } break;
case 23:
# line 134 "cexp.y"
{ yyval.lval = (yypvt[-2].lval | yypvt[-0].lval); } break;
case 24:
# line 136 "cexp.y"
{ yyval.lval = (yypvt[-2].lval && yypvt[-0].lval); } break;
case 25:
# line 138 "cexp.y"
{ yyval.lval = (yypvt[-2].lval || yypvt[-0].lval); } break;
case 26:
# line 140 "cexp.y"
{ yyval.lval = yypvt[-4].lval ? yypvt[-2].lval : yypvt[-0].lval; } break;
case 27:
# line 142 "cexp.y"
{ yyval.lval = yylval.lval; } break;
case 28:
# line 144 "cexp.y"
{ yyval.lval = yylval.lval; } break;
case 29:
# line 146 "cexp.y"
{ yyval.lval = 0; } break;
	}
	goto yystack;		/* reset registers in driver code */
}
