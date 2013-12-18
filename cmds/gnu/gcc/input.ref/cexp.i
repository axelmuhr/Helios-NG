
extern int target_flags;

enum reg_class { NO_REGS, LO_FPA_REGS, FPA_REGS, FP_REGS,
  FP_OR_FPA_REGS, DATA_REGS, DATA_OR_FPA_REGS, DATA_OR_FP_REGS,
  DATA_OR_FP_OR_FPA_REGS, ADDR_REGS, GENERAL_REGS,
  GENERAL_OR_FPA_REGS, GENERAL_OR_FP_REGS, ALL_REGS,
  LIM_REG_CLASSES };

extern enum reg_class regno_reg_class[];

extern int sigsetjmp(), setjmp(), _setjmp();
extern void siglongjmp(), longjmp(), _longjmp();

typedef int jmp_buf[58 ];

typedef	int sigjmp_buf[58 +1];

  int yylex ();
  void yyerror ();
  int expression_value;

  static jmp_buf parse_return_error;

  extern unsigned char is_idstart[], is_idchar[];

typedef union {
  long lval;
  int voidval;
  char *sval;
} YYSTYPE;

typedef
  struct yyltype
 {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
 yyltype;

extern	struct	_iobuf {
	int	_cnt;
	unsigned char *_ptr;
	unsigned char *_base;
	int	_bufsiz;
	short	_flag;
	char	_file;		 
} _iob[];

extern struct _iobuf 	*fopen();
extern struct _iobuf 	*fdopen();
extern struct _iobuf 	*freopen();
extern struct _iobuf 	*popen();
extern struct _iobuf 	*tmpfile();
extern long	ftell();
extern char	*fgets();
extern char	*gets();

extern char	*ctermid();
extern char	*cuserid();
extern char	*tempnam();
extern char	*tmpnam();

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    29,     2,     2,     2,    27,    14,     2,    31,
    32,    25,    23,     9,    24,     2,    26,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     8,     2,    17,
     2,    18,     7,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    13,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    12,     2,    30,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,    10,    11,    15,    16,    19,    20,    21,    22,    28
};

static const short yyrline[] = {     0,
    71,    76,    77,    82,    84,    86,    88,    93,    95,   102,
   109,   111,   113,   115,   117,   119,   121,   123,   125,   127,
   129,   131,   133,   135,   137,   139,   141,   143,   145
};

static const char * const yytname[] = {     0,
"error","$illegal.","INT","CHAR","NAME","ERROR","'?'","':'","','","OR",
"AND","'|'","'^'","'&'","EQUAL","NOTEQUAL","'<'","'>'","LEQ","GEQ",
"LSH","RSH","'+'","'-'","'*'","'/'","'%'","UNARY","'!'","'~'",
"'('","')'","start"
};

static const short yyr1[] = {     0,
    33,    34,    34,    35,    35,    35,    35,    35,    35,    35,
    35,    35,    35,    35,    35,    35,    35,    35,    35,    35,
    35,    35,    35,    35,    35,    35,    35,    35,    35
};

static const short yyr2[] = {     0,
     1,     1,     3,     2,     2,     2,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     5,     1,     1,     1
};

static const short yydefact[] = {     0,
    27,    28,    29,     0,     0,     0,     0,     1,     2,     4,
     5,     6,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     7,     3,     0,    25,    24,    23,    22,
    21,    15,    16,    19,    20,    17,    18,    13,    14,    11,
    12,     8,     9,    10,     0,    26,     0,     0,     0
};

static const short yydefgoto[] = {    57,
     8,     9
};

static const short yypact[] = {    28,
-32768,-32768,-32768,    28,    28,    28,    28,    -3,    74,-32768,
-32768,-32768,    -2,    28,    28,    28,    28,    28,    28,    28,
    28,    28,    28,    28,    28,    28,    28,    28,    28,    28,
    28,    28,    28,-32768,    74,    53,    23,    90,   105,   119,
   132,   143,   143,   150,   150,   150,   150,   155,   155,   -22,
   -22,-32768,-32768,-32768,    28,    74,     8,     9,-32768
};

static const short yypgoto[] = {-32768,
    46,    -4
};

static const short yytable[] = {    10,
    11,    12,    31,    32,    33,    14,    14,    58,    59,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
    46,    47,    48,    49,    50,    51,    52,    53,    54,    34,
     1,     2,     3,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    56,     4,    13,     0,     0,     0,     5,     6,     7,    15,
    55,     0,    16,    17,    18,    19,    20,    21,    22,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    15,     0,     0,    16,    17,    18,    19,    20,    21,    22,
    23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
    33,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    28,    29,    30,    31,
    32,    33,    20,    21,    22,    23,    24,    25,    26,    27,
    28,    29,    30,    31,    32,    33,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    23,
    24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
    27,    28,    29,    30,    31,    32,    33,    29,    30,    31,
    32,    33
};

static const short yycheck[] = {     4,
     5,     6,    25,    26,    27,     9,     9,     0,     0,    14,
    15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
    25,    26,    27,    28,    29,    30,    31,    32,    33,    32,
     3,     4,     5,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    55,    24,     7,    -1,    -1,    -1,    29,    30,    31,     7,
     8,    -1,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
     7,    -1,    -1,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,    22,    23,    24,    25,    26,    27,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    14,    15,    16,    17,    18,    19,    20,    21,
    22,    23,    24,    25,    26,    27,    15,    16,    17,    18,
    19,    20,    21,    22,    23,    24,    25,    26,    27,    17,
    18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
    21,    22,    23,    24,    25,    26,    27,    23,    24,    25,
    26,    27
};

int	yychar;			 
YYSTYPE	yylval;			 

yyltype  yylloc;			 

int yynerrs;			 

int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  yyltype  *yylsp;
  int yyerrstatus;	 
  int yychar1;		 

  short	yyssa[200 ];	 
  YYSTYPE yyvsa[200 ];	 
  yyltype  yylsa[200 ];	 

  short *yyss = yyssa;		 
  YYSTYPE *yyvs = yyvsa;	 
  yyltype  *yyls = yylsa;

  int yymaxdepth = 200 ;

  YYSTYPE yyval;		 

  int yylen;

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = 	-2 ;		 

  yyssp = yyss - 1;
  yyvsp = yyvs;
  yylsp = yyls;

yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yymaxdepth - 1)
    {
      YYSTYPE *yyvs1 = yyvs;
      yyltype  *yyls1 = yyls;
      short *yyss1 = yyss;

      int size = yyssp - yyss + 1;

      if (yymaxdepth >= 10000 )
	yyerror("parser stack overflow");
      yymaxdepth *= 2;
      if (yymaxdepth > 10000 )
	yymaxdepth = 10000 ;
      yyss = (short *) __builtin_alloca  (yymaxdepth * sizeof (*yyssp));
      memcpy ( (char *)yyss,(char *)yyss1, size * sizeof (*yyssp)) ;
      yyvs = (YYSTYPE *) __builtin_alloca  (yymaxdepth * sizeof (*yyvsp));
      memcpy ( (char *)yyvs,(char *)yyvs1, size * sizeof (*yyvsp)) ;

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;

      if (yyssp >= yyss + yymaxdepth - 1)
		return(1) ;
    }

yyresume:

  yyn = yypact[yystate];
  if (yyn == 	-32768 )
    goto yydefault;

  if (yychar == 	-2 )
    {

      yychar = 	yylex() ;
    }

  if (yychar <= 0)		 
    {
      yychar1 = 0;
      yychar = 	0 ;		 

    }
  else
    {
      yychar1 = ((unsigned)(yychar) <= 270 ? yytranslate[yychar] : 36) ;

    }

  yyn += yychar1;
  if (yyn < 0 || yyn > 	182  || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  if (yyn < 0)
    {
      if (yyn == 	-32768 )
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == 	59 )
    return(0) ;

  if (yychar != 	0 )
    yychar = 	-2 ;

  *++yyvsp = yylval;

  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen];  

  switch (yyn) {

case 1:

{ expression_value = yyvsp[0].lval; ;
    break;}
case 3:

{ yyval.lval = yyvsp[0].lval; ;
    break;}
case 4:

{ yyval.lval = - yyvsp[0].lval; ;
    break;}
case 5:

{ yyval.lval = ! yyvsp[0].lval; ;
    break;}
case 6:

{ yyval.lval = ~ yyvsp[0].lval; ;
    break;}
case 7:

{ yyval.lval = yyvsp[-1].lval; ;
    break;}
case 8:

{ yyval.lval = yyvsp[-2].lval * yyvsp[0].lval; ;
    break;}
case 9:

{ if (yyvsp[0].lval == 0)
			    {
			      error ("division by zero in #if");
			      yyvsp[0].lval = 1;
			    }
			  yyval.lval = yyvsp[-2].lval / yyvsp[0].lval; ;
    break;}
case 10:

{ if (yyvsp[0].lval == 0)
			    {
			      error ("division by zero in #if");
			      yyvsp[0].lval = 1;
			    }
			  yyval.lval = yyvsp[-2].lval % yyvsp[0].lval; ;
    break;}
case 11:

{ yyval.lval = yyvsp[-2].lval + yyvsp[0].lval; ;
    break;}
case 12:

{ yyval.lval = yyvsp[-2].lval - yyvsp[0].lval; ;
    break;}
case 13:

{ yyval.lval = yyvsp[-2].lval << yyvsp[0].lval; ;
    break;}
case 14:

{ yyval.lval = yyvsp[-2].lval >> yyvsp[0].lval; ;
    break;}
case 15:

{ yyval.lval = (yyvsp[-2].lval == yyvsp[0].lval); ;
    break;}
case 16:

{ yyval.lval = (yyvsp[-2].lval != yyvsp[0].lval); ;
    break;}
case 17:

{ yyval.lval = (yyvsp[-2].lval <= yyvsp[0].lval); ;
    break;}
case 18:

{ yyval.lval = (yyvsp[-2].lval >= yyvsp[0].lval); ;
    break;}
case 19:

{ yyval.lval = (yyvsp[-2].lval < yyvsp[0].lval); ;
    break;}
case 20:

{ yyval.lval = (yyvsp[-2].lval > yyvsp[0].lval); ;
    break;}
case 21:

{ yyval.lval = (yyvsp[-2].lval & yyvsp[0].lval); ;
    break;}
case 22:

{ yyval.lval = (yyvsp[-2].lval ^ yyvsp[0].lval); ;
    break;}
case 23:

{ yyval.lval = (yyvsp[-2].lval | yyvsp[0].lval); ;
    break;}
case 24:

{ yyval.lval = (yyvsp[-2].lval && yyvsp[0].lval); ;
    break;}
case 25:

{ yyval.lval = (yyvsp[-2].lval || yyvsp[0].lval); ;
    break;}
case 26:

{ yyval.lval = yyvsp[-4].lval ? yyvsp[-2].lval : yyvsp[0].lval; ;
    break;}
case 27:

{ yyval.lval = yylval.lval; ;
    break;}
case 28:

{ yyval.lval = yylval.lval; ;
    break;}
case 29:

{ yyval.lval = 0; ;
    break;}
}

  yyvsp -= yylen;
  yyssp -= yylen;

  *++yyvsp = yyval;

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - 33 ] + *yyssp;
  if (yystate >= 0 && yystate <= 	182  && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - 33 ];

  goto yynewstate;

yyerrlab:    

  if (! yyerrstatus)
    {
      ++yynerrs;
      yyerror("parse error");
    }

  if (yyerrstatus == 3)
    {

      if (yychar == 	0 )
		return(1) ;

      yychar = 	-2 ;
    }

  yyerrstatus = 3;		 

  goto yyerrhandle;

yyerrdefault:   

yyerrpop:    

  if (yyssp == yyss) 	return(1) ;
  yyvsp--;
  yystate = *--yyssp;

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == 	-32768 )
    goto yyerrdefault;

  yyn += 1 ;
  if (yyn < 0 || yyn > 	182  || yycheck[yyn] != 1 )
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == 	-32768 )
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == 	59 )
    return(0) ;

  *++yyvsp = yylval;

  yystate = yyn;
  goto yynewstate;
}

static char *lexptr;

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
      yyerror ("floating point numbers not allowed in #if expressions");
      return 261 ;

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
	return 261 ;
      }
    }
  }

  lexptr = p;
  yylval.lval = n;
  return 258 ;
}

struct token {
  char *operator;
  int token;
};

static struct token tokentab2[] = {
  {"&&", 263 },
  {"||", 262 },
  {"<<", 268 },
  {">>", 269 },
  {"==", 264 },
  {"!=", 265 },
  {"<=", 266 },
  {">=", 267 },
  {0 , 261 }
};

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
  for (toktab = tokentab2; toktab->operator != 0 ; toktab++)
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
      return 261 ;
    }
    return 259 ;

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
    return 261 ;
  }
  if (c >= '0' && c <= '9') {
    for (namelen = 0;
	 c = tokstart[namelen], is_idchar[c] || c == '.'; 
	 namelen++)
      ;
    return parse_number (namelen);
  }
  if (!is_idstart[c]) {
    yyerror ("Invalid token in expression");
    return 261 ;
  }
  for (namelen = 0; is_idchar[tokstart[namelen]]; namelen++)
    ;
  lexptr += namelen;
  return 260 ;
}

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

int
parse_c_expression (string)
     char *string;
{
  lexptr = string;
  if (lexptr == 0 || *lexptr == 0) {
    error ("empty #if expression");
    return 0;			 
  }

  if (setjmp(parse_return_error))
    return 0;

  if (yyparse ())
    return 0;			 

  if (*lexptr)
    error ("Junk after end of expression.");

  return expression_value;	 
}


