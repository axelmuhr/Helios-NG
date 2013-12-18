
/*  A Bison parser, made from iasm.y  */

#define	MACROLINE	258
#define	MACROARG	259
#define	LABEL	260
#define	SYMBOL	261
#define	MACRO	262
#define	IREGISTER	263
#define	FREGISTER	264
#define	CREGISTER	265
#define	AT	266
#define	INSTRUCTION	267
#define	NUMBER	268
#define	STRING	269
#define	COMMA	270
#define	RPAREN	271
#define	LPAREN	272
#define	AUTOINC	273
#define	HASH	274
#define	NEWLINE	275
#define	MODNUM	276
#define	GET	277
#define	MACROEND	278
#define	MACROSTART	279
#define	IMAGESIZE	280
#define	PLUS	281
#define	MINUS	282
#define	TIMES	283
#define	DIVIDE	284
#define	LSHIFT	285
#define	RSHIFT	286
#define	AND	287
#define	XOR	288
#define	OR	289
#define	EQ	290
#define	NE	291
#define	GT	292
#define	LT	293
#define	GE	294
#define	LE	295
#define	NOT	296
#define	DOT	297
#define	JUNK	298
#define	MOD	299


#include "iasm.h"
Line nextline;         /* The result ! */
Expression *binexpression(ETYPE t, Expression *e1, Expression *e2);
Expression *NullExpression(void);
TextSeg *NullTextSeg(void);
void yyerror(char *s);

typedef union {
      Expression  *instr;
      Symbol      *symbol;
      Expression  *expression;
      int32        ival;
      char        *text;
      TextSeg     *textseg;
} YYSTYPE;

#ifndef YYLTYPE
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

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __STDC__
#define const
#endif



#define	YYFINAL		95
#define	YYFLAG		-32768
#define	YYNTBASE	45

#define YYTRANSLATE(x) ((unsigned)(x) <= 299 ? yytranslate[x] : 63)

static const char yytranslate[] = {     0,
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
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
    26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
    36,    37,    38,    39,    40,    41,    42,    43,    44
};

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    49,    51,    54,    57,    59,    62,    64,    64,    71,    77,
    82,    89,    94,    97,   108,   110,   113,   115,   118,   121,
   124,   135,   138,   142,   150,   157,   163,   168,   171,   173,
   176,   179,   184,   192,   200,   203,   206,   210,   214,   218,
   222,   226,   230,   234,   238,   242,   246,   249,   252,   255,
   258,   261,   265,   269,   273,   278,   282,   287,   291,   296
};

static const char * const yytname[] = {     "EOF",
"error","$illegal.","MACROLINE","MACROARG","LABEL","SYMBOL","MACRO","IREGISTER","FREGISTER","CREGISTER",
"AT","INSTRUCTION","NUMBER","STRING","COMMA","RPAREN","LPAREN","AUTOINC","HASH","NEWLINE",
"MODNUM","GET","MACROEND","MACROSTART","IMAGESIZE","PLUS","MINUS","TIMES","DIVIDE","LSHIFT",
"RSHIFT","AND","XOR","OR","EQ","NE","GT","LT","GE","LE",
"NOT","DOT","JUNK","MOD","lines"
};
#endif

static const short yyr1[] = {     0,
    45,    45,    46,    47,    47,    48,    49,    48,    48,    50,
    50,    50,    51,    51,    52,    52,    53,    53,    54,    55,
    55,    56,    56,    56,    56,    56,    56,    56,    57,    57,
    58,    58,    59,    60,    61,    61,    62,    62,    62,    62,
    62,    62,    62,    62,    62,    62,    62,    62,    62,    62,
    62,    62,    62,    62,    62,    62,    62,    62,    62,    62
};

static const short yyr2[] = {     0,
     1,     2,     2,     1,     0,     2,     0,     6,     3,     2,
     3,     0,     1,     3,     1,     0,     2,     0,     1,     1,
     3,     1,     1,     1,     5,     1,     1,     0,     1,     0,
     1,     1,     2,     2,     2,     0,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     2,     2,     1,     1,     1,     1,     1,     3
};

static const short yydefact[] = {     5,
     4,     5,     1,    18,     2,    16,    28,     0,     3,     0,
    15,     0,    13,    55,    36,    36,    24,    57,    59,    26,
     0,    56,    27,     0,     0,    58,    17,    19,    20,    22,
    23,    31,    32,     7,     6,    16,     9,     0,    34,    33,
     0,    53,    54,    28,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    12,    14,    35,    60,    21,     0,    37,    38,    39,
    40,    42,    43,    44,    45,    46,    47,    48,    49,    50,
    52,    51,    41,     0,     0,    30,    10,     0,     0,    29,
    25,    11,     8,     0,     0
};

static const short yydefgoto[] = {     2,
     3,     4,     9,    62,    85,    12,    13,    10,    27,    28,
    29,    91,    30,    31,    32,    39,    33
};

static const short yypact[] = {    -3,
-32768,    13,-32768,    46,-32768,     1,    35,    -8,-32768,    -5,
-32768,    -1,-32768,-32768,   -10,   -10,-32768,-32768,-32768,-32768,
    44,-32768,-32768,    44,    44,-32768,-32768,     5,-32768,     6,
-32768,-32768,    81,-32768,-32768,     1,-32768,    44,-32768,-32768,
    62,-32768,-32768,    35,    14,    44,    44,    44,    44,    44,
    44,    44,    44,    44,    44,    44,    44,    44,    44,    44,
    44,    39,-32768,    81,-32768,-32768,    31,   161,   161,-32768,
-32768,    40,    40,   138,   119,   100,   157,   157,   -20,   -20,
   -20,   -20,-32768,    34,    -2,    33,-32768,    43,    52,-32768,
-32768,-32768,-32768,    59,-32768
};

static const short yypgoto[] = {-32768,
    71,-32768,-32768,-32768,-32768,-32768,    28,-32768,-32768,-32768,
    30,-32768,-32768,-32768,    36,    63,   -21
};


#define	YYLAST		205


static const short yytable[] = {    41,
    88,     1,    42,    43,    11,    46,    47,    48,    49,    50,
    51,    34,    94,    36,    35,    38,    64,     1,    37,    44,
    89,    15,    45,    61,    68,    69,    70,    71,    72,    73,
    74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
    14,    84,    15,    16,    17,    18,    86,    19,    20,    14,
    90,    21,     6,    87,    18,    22,    19,     7,    95,    23,
    21,    24,    92,    63,    22,    46,    47,    48,    49,     8,
    24,    93,     5,    66,     0,    25,    26,    65,    40,     0,
    67,     0,     0,    61,    25,    26,     0,    46,    47,    48,
    49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
    59,    60,     0,     0,     0,    61,    46,    47,    48,    49,
    50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
    60,     0,     0,     0,    61,    46,    47,    48,    49,    50,
    51,    52,    53,     0,    55,    56,    57,    58,    59,    60,
     0,     0,     0,    61,    46,    47,    48,    49,    50,    51,
    52,     0,     0,    55,    56,    57,    58,    59,    60,     0,
     0,     0,    61,    46,    47,    48,    49,    50,    51,     0,
     0,     0,    55,    56,    57,    58,    59,    60,     0,     0,
     0,    61,    46,    47,    48,    49,    50,    51,    48,    49,
     0,     0,     0,    57,    58,    59,    60,     0,     0,     0,
    61,     0,     0,     0,    61
};

static const short yycheck[] = {    21,
     3,     5,    24,    25,     4,    26,    27,    28,    29,    30,
    31,    20,     0,    15,    20,    26,    38,     5,    20,    15,
    23,     8,    17,    44,    46,    47,    48,    49,    50,    51,
    52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
     6,     3,     8,     9,    10,    11,    16,    13,    14,     6,
    18,    17,     7,    20,    11,    21,    13,    12,     0,    25,
    17,    27,    20,    36,    21,    26,    27,    28,    29,    24,
    27,    20,     2,    44,    -1,    41,    42,    16,    16,    -1,
    45,    -1,    -1,    44,    41,    42,    -1,    26,    27,    28,
    29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
    39,    40,    -1,    -1,    -1,    44,    26,    27,    28,    29,
    30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
    40,    -1,    -1,    -1,    44,    26,    27,    28,    29,    30,
    31,    32,    33,    -1,    35,    36,    37,    38,    39,    40,
    -1,    -1,    -1,    44,    26,    27,    28,    29,    30,    31,
    32,    -1,    -1,    35,    36,    37,    38,    39,    40,    -1,
    -1,    -1,    44,    26,    27,    28,    29,    30,    31,    -1,
    -1,    -1,    35,    36,    37,    38,    39,    40,    -1,    -1,
    -1,    44,    26,    27,    28,    29,    30,    31,    28,    29,
    -1,    -1,    -1,    37,    38,    39,    40,    -1,    -1,    -1,
    44,    -1,    -1,    -1,    44
};
#define YYPURE 1

/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */


/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Bob Corbett and Richard Stallman

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* Not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__)
#include <alloca.h>
#endif /* Sparc.  */
#endif /* Not GNU C.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYIMPURE
#define YYLEX		yylex()
#endif

#ifndef YYPURE
#define YYLEX		yylex(&yylval, &yylloc)
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYIMPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* YYIMPURE */

#ifndef YYDEBUG
#define YYDEBUG 0
#endif

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_bcopy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}


int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/

#define YYPOPSTACK   (yyvsp--, yysp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yysp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifndef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
#ifdef YYLSP_NEEDED
		 &yyls1, size * sizeof (*yylsp),
#endif
		 &yystacksize);

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Next token is %d (%s)\n", yychar, yytname[yychar1]);
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      if (yylen == 1)
	fprintf (stderr, "Reducing 1 value via rule %d (line %d), ",
		 yyn, yyrline[yyn]);
      else
	fprintf (stderr, "Reducing %d values via rule %d (line %d), ",
		 yylen, yyn, yyrline[yyn]);
    }
#endif


  switch (yyn) {

case 3:
{  dot = pcloc; ;
    break;}
case 4:
{ nextline.label = yyvsp[0].symbol; ;
    break;}
case 5:
{ nextline.label = NULL; ;
    break;}
case 6:
{ assemble(&nextline); ;
    break;}
case 7:
{ outlistline(); ;
    break;}
case 8:
{  if( nextline.label == NULL )
            error("Macro definition needs macro name");
         else
            if (pass==1) resolvesym(nextline.label,(int32)yyvsp[-2].textseg, S_MACRO);
         outlistline();
      ;
    break;}
case 9:
{  if( nextline.label ) setlabel(nextline.label);
         startmacro(yyvsp[-2].symbol,yyvsp[-1].textseg);
         outlistline();
      ;
    break;}
case 10:
{  yyval.textseg = yyvsp[-1].textseg;
               yyval.textseg->cdr = NULL;
               outlistline();
            ;
    break;}
case 11:
{  TextSeg **ml = &(yyvsp[-2].textseg->cdr);
               while( *ml != NULL ) ml = &((*ml)->cdr);
               *ml = yyvsp[-1].textseg;
               yyvsp[-1].textseg->cdr = NULL;
               outlistline();
            ;
    break;}
case 12:
{
               yyval.textseg = NULL;
            ;
    break;}
case 13:
{  yyval.textseg = yyvsp[0].textseg;
            ;
    break;}
case 14:
{  TextSeg **ma;
               if( yyvsp[-2].textseg == NULL ) yyvsp[-2].textseg = NullTextSeg();
               if( yyvsp[0].textseg == NULL ) yyvsp[0].textseg = NullTextSeg();
               ma = &(yyvsp[-2].textseg->cdr);
               while( *ma != NULL ) ma = &((*ma)->cdr);
               *ma = yyvsp[0].textseg;
               yyvsp[0].textseg->cdr = NULL;
               yyval.textseg = yyvsp[-2].textseg;
            ;
    break;}
case 15:
{ yyval.textseg = yyvsp[0].textseg; ;
    break;}
case 16:
{ yyval.textseg = NULL; ;
    break;}
case 17:
{ nextline.instr = yyvsp[-1].instr; ;
    break;}
case 18:
{ nextline.instr = NULL; ;
    break;}
case 19:
{ nextline.args = yyvsp[0].expression; ;
    break;}
case 20:
{  yyval.expression = yyvsp[0].expression; ;
    break;}
case 21:
{  Expression *e;

         if( yyvsp[-2].expression == NULL ) yyvsp[-2].expression = NullExpression();
         if( yyvsp[0].expression == NULL ) yyvsp[0].expression = NullExpression();
         e = yyvsp[-2].expression;
         while( e->cdr != NULL ) e = e->cdr;
         e->cdr = yyvsp[0].expression;
         yyval.expression = yyvsp[-2].expression;
      ;
    break;}
case 22:
{ yyval.expression = yyvsp[0].expression; ;
    break;}
case 23:
{  yyval.expression = yyvsp[0].expression;
      ;
    break;}
case 24:
{
         yyval.expression = newexpression();
         yyval.expression->exprtype = E_CREGISTER;
         yyval.expression->e1.symbol = yyvsp[0].symbol;
         yyval.expression->cdr = NULL;
      ;
    break;}
case 25:
{  Expression *e1;
         e1 = yyval.expression = newexpression();
         e1->exprtype   = (yyvsp[0].ival)? E_REGOFFSETINC: E_REGOFFSET;
         e1->e1.expr    = yyvsp[-4].expression;
         e1->cdr        = yyvsp[-2].expression;
      ;
    break;}
case 26:
{  yyval.expression = newexpression();
         yyval.expression->exprtype = E_STRING;
         yyval.expression->e1.text  = yyvsp[0].text;
         yyval.expression->cdr      = NULL;
      ;
    break;}
case 27:
{  yyval.expression = newexpression();
         yyval.expression->exprtype = E_IMAGESIZE;
         yyval.expression->cdr      = NULL;
      ;
    break;}
case 28:
{  yyval.expression = NULL; ;
    break;}
case 29:
{ yyval.ival = 1; ;
    break;}
case 30:
{ yyval.ival = 0; ;
    break;}
case 31:
{  yyval.expression = yyvsp[0].expression;
      ;
    break;}
case 32:
{ yyval.expression = yyvsp[0].expression;
        yyval.expression->cdr = NULL;
      ;
    break;}
case 33:
{  yyval.expression = newexpression();
         yyval.expression->exprtype = E_FREGISTER;
         yyval.expression->e1.symbol = yyvsp[-1].symbol;
         yyval.expression->e2.expr   = yyvsp[0].expression;
         yyval.expression->cdr = NULL;
      ;
    break;}
case 34:
{  yyval.expression = newexpression();
         yyval.expression->exprtype = E_IREGISTER;
         yyval.expression->e1.symbol = yyvsp[-1].symbol;
         yyval.expression->e2.expr   = yyvsp[0].expression;
         yyval.expression->cdr = NULL;
      ;
    break;}
case 35:
{  yyval.expression = yyvsp[0].expression;
      ;
    break;}
case 36:
{  yyval.expression = NULL; ;
    break;}
case 37:
{  yyval.expression = binexpression(E_PLUS,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 38:
{  yyval.expression = binexpression(E_MINUS,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 39:
{  yyval.expression = binexpression(E_TIMES,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 40:
{  yyval.expression = binexpression(E_DIVIDE,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 41:
{  yyval.expression = binexpression(E_MOD, yyvsp[-2].expression, yyvsp[0].expression);
             ;
    break;}
case 42:
{  yyval.expression = binexpression(E_LSHIFT, yyvsp[-2].expression, yyvsp[0].expression);
             ;
    break;}
case 43:
{  yyval.expression = binexpression(E_RSHIFT,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 44:
{  yyval.expression = binexpression(E_AND,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 45:
{  yyval.expression = binexpression(E_XOR,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 46:
{  yyval.expression = binexpression(E_OR,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 47:
{  yyval.expression = binexpression(E_EQ,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 48:
{  yyval.expression = binexpression(E_NE,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 49:
{  yyval.expression = binexpression(E_GT,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 50:
{  yyval.expression = binexpression(E_LT,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 51:
{  yyval.expression = binexpression(E_LE,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 52:
{  yyval.expression = binexpression(E_GE,yyvsp[-2].expression,yyvsp[0].expression);
             ;
    break;}
case 53:
{  yyval.expression = binexpression(E_UMINUS,yyvsp[0].expression,NULL);
             ;
    break;}
case 54:
{  yyval.expression = binexpression(E_NOT,yyvsp[0].expression,NULL);
             ;
    break;}
case 55:
{  yyval.expression = newexpression();
                yyval.expression->exprtype = E_SYMBOL;
                yyval.expression->e1.symbol    = yyvsp[0].symbol;
             ;
    break;}
case 56:
{  yyval.expression = newexpression();
                yyval.expression->exprtype = E_MODNUM;
             ;
    break;}
case 57:
{  yyval.expression = newexpression();
                yyval.expression->exprtype = E_AT;
                yyval.expression->e1.symbol    = yyvsp[0].symbol;
             ;
    break;}
case 58:
{  yyval.expression = newexpression();
                yyval.expression->exprtype = E_DOT;
             ;
    break;}
case 59:
{  yyval.expression = newexpression();
                yyval.expression->exprtype = E_NUMBER;
                yyval.expression->e1.value    = yyvsp[0].ival;
             ;
    break;}
case 60:
{  yyval.expression = yyvsp[-1].expression; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */


  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) xmalloc(size + 15);
	  strcpy(msg, "parse error");

	  if (count < 5)
	    {
	      count = 0;
	      for (x = 0; x < (sizeof(yytname) / sizeof(char *)); x++)
		if (yycheck[x + yyn] == x)
		  {
		    strcat(msg, count == 0 ? ", expecting `" : " or `");
		    strcat(msg, yytname[x]);
		    strcat(msg, "'");
		    count++;
		  }
	    }
	  yyerror(msg);
	  free(msg);
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}

void yyerror(char *s)
{
   warn("Parse error at line %d: %s",currentfile->lineno,s);
}

Expression *binexpression(ETYPE t, Expression *e1, Expression *e2)
{
   Expression *r = newexpression();
   r->exprtype   = t;
   r->e1.expr    = e1;
   r->e2.expr    = e2;
   return r;
}

int performget(char *s, bool rootfile)
{
   FILE *f;
   char fullname[MAXNAME];
   char *ip = includepaths;

   if( ip && !rootfile)
   {
      while(1)
      {  char *fp = fullname;
         char ch;
         while( (ch = *ip++) != '\0')
         {
            if( ch == ',' ) break;
            *fp++ = ch;
         }
         strcpy(fp,s);
         if( (f = fopen(fullname,"r")) != NULL ) break;
         if( ch == '\0' ) break;
      }
   }
   else
      f = fopen(s, "r");

   if( f != NULL )
   {  AFILE *nf = nextcurrentfile = currentfile+1;
      filenum++;
      nf->io.file.stream = f;
      nf->io.file.buf    = aalloc(LINE_BUF_SIZE);
      setvbuf( f, nf->io.file.buf, _IOLBF, LINE_BUF_SIZE );
      nf->lineno = 1;
      nf->flags  = 0;
      nf->name   = s;
      return 1;
   }
   else
   {
      error("File \"%s\" not found\n",s);
      return 0;
   }
}

void startmacro(Symbol *s, TextSeg *args)
{
   AFILE *nf = nextcurrentfile = currentfile+1;
   Macro *m;
   filenum++;
   nf->io.macro = m = aalloc(sizeof(Macro));
   m->line      = s->symv.macrotext;
   m->tptr      = m->line->text;
   m->argtext   = NULL;
   m->args      = args;

   nf->lineno = 1;
   nf->flags  = ff_macro;
   nf->name   = s->name;
}

TextSeg *NullTextSeg(void)
{  TextSeg *r = (TextSeg *)aalloc(sizeof(TextSeg));
   r->cdr = NULL;
   r->text[0] = '\0';
   return r;
}

Expression *NullExpression(void)
{  Expression *r = newexpression();
   r->exprtype = E_NULL;
   r->cdr = NULL;
   return r;
}
