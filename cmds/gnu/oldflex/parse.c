
/*  A Bison parser, made from parse.y  */

#define	CHAR	258
#define	NUMBER	259
#define	SECTEND	260
#define	SCDECL	261
#define	XSCDECL	262
#define	WHITESPACE	263
#define	NAME	264
#define	PREVCCL	265

#line 17 "parse.y"

#include "flexdef.h"

int pat, scnum, eps, headcnt, trailcnt, anyccl, lastchar, i, actvp, rulelen;
int trlcontxt, xcluflg, cclsorted, varlength;
char clower();

static int madeany = false;  /* whether we've made the '.' character class */


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

#define	YYACCEPT	return(0)
#define	YYABORT	return(1)
#define	YYERROR	return(1)
#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>



#define	YYFINAL		81
#define	YYFLAG		-32768
#define	YYNTBASE	31

#define YYTRANSLATE(x) (yytranslate[x])

static char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    11,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,    25,     2,    16,     2,     2,     2,    26,
    27,    19,    20,    15,    30,    24,    18,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    13,
     2,    14,    21,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    28,     2,    29,    12,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    22,    17,    23,     2,     2,     2,     2,     2,
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
     6,     7,     8,     9,    10
};

static short yyrline[] = {     0,
    29,    32,    41,    42,    43,    47,    50,    60,    64,    67,
    70,    74,    75,    78,    86,    95,   104,   118,   128,   132,
   135,   144,   152,   156,   178,   194,   201,   204,   209,   232,
   240,   244,   251,   258,   265,   279,   293,   311,   333,   350,
   357,   360,   363,   377,   380,   394,   422,   434,   442,   453
};

static char * yytname[] = {     0,
"error","$illegal.","CHAR","NUMBER","SECTEND","SCDECL","XSCDECL","WHITESPACE","NAME","PREVCCL",
"'\\n'","'^'","'<'","'>'","','","'$'","'|'","'/'","'*'","'+'",
"'?'","'{'","'}'","'.'","'\"'","'('","')'","'['","']'","'-'",
"goal"
};

static short yyr1[] = {     0,
    31,    32,    33,    33,    33,    34,    35,    35,    36,    36,
    36,    37,    37,    38,    39,    39,    39,    39,    39,    40,
    41,    41,    41,    42,    42,    43,    43,    43,    44,    45,
    45,    46,    46,    46,    46,    46,    46,    46,    46,    46,
    46,    46,    46,    47,    47,    48,    48,    48,    49,    49
};

static short yyr2[] = {     0,
     4,     0,     5,     0,     2,     1,     1,     1,     3,     1,
     1,     4,     0,     0,     4,     3,     3,     2,     1,     3,
     3,     1,     1,     1,     0,     3,     2,     1,     2,     2,
     1,     2,     2,     2,     6,     5,     4,     1,     1,     1,
     3,     3,     1,     3,     4,     4,     2,     0,     2,     0
};

static short yydefact[] = {     2,
     0,     0,     0,     5,     6,     7,     8,    13,     0,    14,
     0,     0,    11,    10,     0,    19,    43,    40,     0,     0,
    38,    50,     0,    48,     0,     0,    25,     0,    28,    31,
    39,     0,     3,    25,    23,    22,     0,     0,     0,    48,
     0,    12,     0,    25,    24,     0,    29,    18,    27,    30,
    32,    33,    34,     0,     9,    17,    20,     0,    49,    41,
    42,     0,    47,    44,    25,    16,    26,     0,    21,    45,
     0,    15,     0,    37,    46,     0,    36,    35,     0,     0,
     0
};

static short yydefgoto[] = {    79,
     1,     3,     8,     9,    15,    10,    12,    25,    26,    37,
    48,    27,    28,    29,    30,    31,    41,    38
};

static short yypact[] = {-32768,
    61,    -5,    43,-32768,-32768,-32768,-32768,-32768,     8,     9,
     1,     2,-32768,-32768,    28,-32768,-32768,-32768,    32,    45,
-32768,-32768,    32,    13,    42,    19,    57,    32,    32,    50,
-32768,    55,-32768,    57,-32768,-32768,    62,    15,    34,-32768,
     4,-32768,    32,    57,-32768,    32,-32768,-32768,    32,    50,
-32768,-32768,-32768,    74,-32768,-32768,-32768,    56,-32768,-32768,
-32768,     5,   -17,-32768,    57,-32768,    32,    40,-32768,-32768,
    76,-32768,     0,-32768,-32768,    36,-32768,-32768,    80,    81,
-32768
};

static short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
   -33,    -2,-32768,    -9,   -29,-32768,    44,-32768
};


#define	YYLAST		84


static short yytable[] = {    50,
    56,    13,    16,    76,    17,     4,    63,    63,    -1,    14,
    66,    18,    71,    19,    20,    11,    34,    59,    49,    50,
    39,    17,    77,    44,    40,    21,    22,    23,    18,    24,
    43,    72,    64,    70,    17,    32,    67,    50,    33,    60,
    65,    18,    21,    22,    23,    35,    24,     5,     6,     7,
    46,    47,    42,    36,    73,    21,    22,    23,    78,    24,
    61,     2,    74,    55,    69,    -4,    -4,    -4,    51,    52,
    53,    54,    45,    46,    47,    57,    58,    68,    75,    80,
    81,     0,     0,    62
};

static short yycheck[] = {    29,
    34,     1,     1,     4,     3,    11,     3,     3,     0,     9,
    44,    10,    30,    12,    13,     8,    19,     3,    28,    49,
    23,     3,    23,    26,    12,    24,    25,    26,    10,    28,
    12,    65,    29,    29,     3,     8,    46,    67,    11,    25,
    43,    10,    24,    25,    26,     1,    28,     5,     6,     7,
    17,    18,    11,     9,    15,    24,    25,    26,    23,    28,
    27,     1,    23,     9,     9,     5,     6,     7,    19,    20,
    21,    22,    16,    17,    18,    14,    15,     4,     3,     0,
     0,    -1,    -1,    40
};
#define YYPURE 1

#line 2 "bison.simple"

/* Skeleton output parser for bison,
   copyright (C) 1984 Bob Corbett and Richard Stallman

   Permission is granted to anyone to make or distribute verbatim copies of this program
   provided that the copyright notice and this permission notice are preserved;
   and provided that the recipient is not asked to waive or limit his right to
   redistribute copies as permitted by this permission notice;
   and provided that anyone possessing an executable copy
   is granted access to copy the source code, in machine-readable form,
   in some reasonable manner.

   Permission is granted to distribute derived works or enhanced versions of
   this program under the above conditions with the additional condition
   that the entire derivative or enhanced work
   must be covered by a permission notice identical to this one.

   Anything distributed as part of a package containing portions derived
   from this program, which cannot in current practice perform its function usefully
   in the absense of what was derived directly from this program,
   is to be considered as forming, together with the latter,
   a single work derived from this program,
   which must be entirely covered by a permission notice identical to this one
   in order for distribution of the package to be permitted.

 In other words, you are welcome to use, share and improve this program.
 You are forbidden to forbid anyone else to use, share and improve
 what you give them.   Help stamp out software-hoarding!  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#ifdef __STDC__
#define bcopy(s2,s1,n) memcpy(s1,s2,n)
#endif

#define yyerrok      (yyerrstatus = 0)
#define yyclearin   (yychar = YYEMPTY)
#define YYEMPTY      -2
#define YYEOF      0
#define YYFAIL      goto yyerrlab;

#define YYTERROR   1

#ifndef YYIMPURE
#define YYLEX      yylex()
#endif

#ifndef YYPURE
#define YYLEX      yylex(&yylval, &yylloc)
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYIMPURE

int   yychar;         /*  the lookahead symbol      */
YYSTYPE   yylval;         /*  the semantic value of the      */
            /*  lookahead symbol         */

YYLTYPE yylloc;         /*  location data for the lookahead   */
            /*  symbol            */

int yydebug = 0;      /*  nonzero means print parse trace   */

#endif  /* YYIMPURE */


/*  YYMAXDEPTH indicates the initial size of the parser's stacks   */

#ifndef   YYMAXDEPTH
#define YYMAXDEPTH 200
#endif

/*  YYMAXLIMIT is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#ifndef YYMAXLIMIT
#define YYMAXLIMIT 10000
#endif


#line 87 "bison.simple"
int
yyparse()
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  YYLTYPE *yylsp;
  int yyerrstatus;   /*  number of tokens to shift before error messages enabled */
  int yychar1;      /*  lookahead token as an internal (translated) token number */

  short   yyssa[YYMAXDEPTH];   /*  the state stack         */
  YYSTYPE yyvsa[YYMAXDEPTH];   /*  the semantic value stack      */
  YYLTYPE yylsa[YYMAXDEPTH];   /*  the location stack         */

  short *yyss = yyssa;      /*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;   /*  to allow yyoverflow to reallocate them elsewhere */
  YYLTYPE *yyls = yylsa;

  int yymaxdepth = YYMAXDEPTH;

#ifndef YYPURE

  int yychar;
  YYSTYPE yylval;
  YYLTYPE yylloc;

  extern int yydebug;

#endif


  YYSTYPE yyval;      /*  the variable used to return      */
            /*  semantic values from the action   */
            /*  routines            */

  int yylen;

  if (yydebug)
    fprintf(stderr, "Starting parse\n");

  yystate = 0;
  yyerrstatus = 0;
  yychar = YYEMPTY;      /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
  yylsp = yyls;

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yymaxdepth - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      YYLTYPE *yyls1 = yyls;
      short *yyss1 = yyss;

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
    the data in use in that stack, in bytes.  */
      yyoverflow("parser stack overflow",
       &yyss1, size * sizeof (*yyssp),
       &yyvs1, size * sizeof (*yyvsp),
       &yyls1, size * sizeof (*yylsp),
       &yymaxdepth);

      yyss = yyss1; yyvs = yyvs1; yyls = yyls1;
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yymaxdepth >= YYMAXLIMIT)
   yyerror("parser stack overflow");
      yymaxdepth *= 2;
      if (yymaxdepth > YYMAXLIMIT)
   yymaxdepth = YYMAXLIMIT;
      yyss = (short *) alloca (yymaxdepth * sizeof (*yyssp));
      bcopy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyls = (YYLTYPE *) alloca (yymaxdepth * sizeof (*yylsp));
      bcopy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
      yyvs = (YYSTYPE *) alloca (yymaxdepth * sizeof (*yyvsp));
      bcopy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yylsp = yyls + size - 1;
      yyvsp = yyvs + size - 1;

      if (yydebug)
   fprintf(stderr, "Stack size increased to %d\n", yymaxdepth);

      if (yyssp >= yyss + yymaxdepth - 1)
   YYERROR;
    }

  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
yyresume:

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)      /* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;      /* Don't call YYLEX any more */

      if (yydebug)
   fprintf(stderr, "Now at end of input.\n");
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

      if (yydebug)
      { char *s = yytname[yychar1];
        if( s == NULL ) s = "YYEOF";
   fprintf(stderr, "Parsing next token; it is %d (%s)\n", yychar, s);
      }
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

  if (yydebug)
  { char *s = yytname[yychar1];
    if( s==NULL ) s = "YYEOF";
    fprintf(stderr, "Shifting token %d (%s), ", yychar, s);
  }
  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
  *++yylsp = yylloc;

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

  if (yydebug)
    {
      if (yylen == 1)
   fprintf (stderr, "Reducing 1 value via line %d, ",
       yyrline[yyn]);
      else
   fprintf (stderr, "Reducing %d values via line %d, ",
       yylen, yyrline[yyn]);
    }


  switch (yyn) {

case 2:
#line 33 "parse.y"
{
			/* initialize for processing rules */

			/* create default DFA start condition */
			scinstal( "INITIAL", false );
			;
    break;}
case 5:
#line 44 "parse.y"
{ synerr( "unknown error processing section 1" ); ;
    break;}
case 7:
#line 51 "parse.y"
{
			/* these productions are separate from the s1object
			 * rule because the semantics must be done before
			 * we parse the remainder of an s1object
			 */

			xcluflg = false;
			;
    break;}
case 8:
#line 61 "parse.y"
{ xcluflg = true; ;
    break;}
case 9:
#line 65 "parse.y"
{ scinstal( nmstr, xcluflg ); ;
    break;}
case 10:
#line 68 "parse.y"
{ scinstal( nmstr, xcluflg ); ;
    break;}
case 11:
#line 71 "parse.y"
{ synerr( "bad start condition list" ); ;
    break;}
case 14:
#line 79 "parse.y"
{
			/* initialize for a parse of one rule */
			trlcontxt = varlength = false;
			trailcnt = headcnt = rulelen = 0;
			;
    break;}
case 15:
#line 87 "parse.y"
{
			pat = link_machines( yyvsp[-1], yyvsp[0] );
			add_accept( pat, headcnt, trailcnt );

			for ( i = 1; i <= actvp; ++i )
			    scbol[actvsc[i]] = mkbranch( scbol[actvsc[i]], pat );
			;
    break;}
case 16:
#line 96 "parse.y"
{
			pat = link_machines( yyvsp[-1], yyvsp[0] );
			add_accept( pat, headcnt, trailcnt );

			for ( i = 1; i <= actvp; ++i )
			    scset[actvsc[i]] = mkbranch( scset[actvsc[i]], pat );
			;
    break;}
case 17:
#line 105 "parse.y"
{
			pat = link_machines( yyvsp[-1], yyvsp[0] );
			add_accept( pat, headcnt, trailcnt );

			/* add to all non-exclusive start conditions,
			 * including the default (0) start condition
			 */

			for ( i = 1; i <= lastsc; ++i )
			    if ( ! scxclu[i] )
				scbol[i] = mkbranch( scbol[i], pat );
			;
    break;}
case 18:
#line 119 "parse.y"
{
			pat = link_machines( yyvsp[-1], yyvsp[0] );
			add_accept( pat, headcnt, trailcnt );

			for ( i = 1; i <= lastsc; ++i )
			    if ( ! scxclu[i] )
				scset[i] = mkbranch( scset[i], pat );
			;
    break;}
case 19:
#line 129 "parse.y"
{ synerr( "unrecognized rule" ); ;
    break;}
case 21:
#line 136 "parse.y"
{
			if ( (scnum = sclookup( nmstr )) == 0 )
			    synerr( "undeclared start condition" );

			else
			    actvsc[++actvp] = scnum;
			;
    break;}
case 22:
#line 145 "parse.y"
{
			if ( (scnum = sclookup( nmstr )) == 0 )
			    synerr( "undeclared start condition" );
			else
			    actvsc[actvp = 1] = scnum;
			;
    break;}
case 23:
#line 153 "parse.y"
{ synerr( "bad start condition list" ); ;
    break;}
case 24:
#line 157 "parse.y"
{
			if ( trlcontxt )
			    {
			    synerr( "trailing context used twice" );
			    yyval = mkstate( SYM_EPSILON );
			    }
			else
			    {
			    trlcontxt = true;

			    if ( ! varlength )
				headcnt = rulelen;

			    ++rulelen;
			    trailcnt = 1;

			    eps = mkstate( SYM_EPSILON );
			    yyval = link_machines( eps, mkstate( '\n' ) );
			    }
			;
    break;}
case 25:
#line 179 "parse.y"
{
		        yyval = mkstate( SYM_EPSILON );

			if ( trlcontxt )
			    {
			    if ( varlength && headcnt == 0 )
				/* both head and trail are variable-length */
				synerr( "illegal trailing context" );

			    else
				trailcnt = rulelen;
			    }
		        ;
    break;}
case 26:
#line 195 "parse.y"
{
			varlength = true;

			yyval = mkor( yyvsp[-2], yyvsp[0] );
			;
    break;}
case 27:
#line 202 "parse.y"
{ yyval = link_machines( yyvsp[-1], yyvsp[0] ); ;
    break;}
case 28:
#line 205 "parse.y"
{ yyval = yyvsp[0]; ;
    break;}
case 29:
#line 210 "parse.y"
{
			/* this rule is separate from the others for "re" so
			 * that the reduction will occur before the trailing
			 * series is parsed
			 */

			if ( trlcontxt )
			    synerr( "trailing context used twice" );
			else
			    trlcontxt = true;

			if ( varlength )
			    /* the trailing context had better be fixed-length */
			    varlength = false;
			else
			    headcnt = rulelen;

			rulelen = 0;
			yyval = yyvsp[-1];
			;
    break;}
case 30:
#line 233 "parse.y"
{
			/* this is where concatenation of adjacent patterns
			 * gets done
			 */
			yyval = link_machines( yyvsp[-1], yyvsp[0] );
			;
    break;}
case 31:
#line 241 "parse.y"
{ yyval = yyvsp[0]; ;
    break;}
case 32:
#line 245 "parse.y"
{
			varlength = true;

			yyval = mkclos( yyvsp[-1] );
			;
    break;}
case 33:
#line 252 "parse.y"
{
			varlength = true;

			yyval = mkposcl( yyvsp[-1] );
			;
    break;}
case 34:
#line 259 "parse.y"
{
			varlength = true;

			yyval = mkopt( yyvsp[-1] );
			;
    break;}
case 35:
#line 266 "parse.y"
{
			varlength = true;

			if ( yyvsp[-3] > yyvsp[-1] || yyvsp[-3] <= 0 )
			    {
 fprintf(stderr,"iter 1 = %d, iter 2 = %d\n",yyvsp[-3],yyvsp[-1]);
			    synerr( "bad iteration values" );
			    yyval = yyvsp[-5];
			    }
			else
			    yyval = mkrep( yyvsp[-5], yyvsp[-3], yyvsp[-1] );
			;
    break;}
case 36:
#line 280 "parse.y"
{
			varlength = true;

			if ( yyvsp[-2] <= 0 )
			    {
			    synerr( "iteration value must be positive" );
			    yyval = yyvsp[-4];
			    }

			else
			    yyval = mkrep( yyvsp[-4], yyvsp[-2], INFINITY );
			;
    break;}
case 37:
#line 294 "parse.y"
{
			/* the singleton could be something like "(foo)",
			 * in which case we have no idea what its length
			 * is, so we punt here.
			 */
			varlength = true;

			if ( yyvsp[-1] <= 0 )
			    {
			    synerr( "iteration value must be positive" );
			    yyval = yyvsp[-3];
			    }

			else
			    yyval = link_machines( yyvsp[-3], copysingl( yyvsp[-3], yyvsp[-1] - 1 ) );
			;
    break;}
case 38:
#line 312 "parse.y"
{
			if ( ! madeany )
			    {
			    /* create the '.' character class */
			    anyccl = cclinit();
			    ccladd( anyccl, '\n' );
			    cclnegate( anyccl );

			    if ( useecs )
				mkeccl( ccltbl + cclmap[anyccl],
					ccllen[anyccl], nextecm,
					ecgroup, CSIZE );
			    
			    madeany = true;
			    }

			++rulelen;

			yyval = mkstate( -anyccl );
			;
    break;}
case 39:
#line 334 "parse.y"
{
			if ( ! cclsorted )
			    /* sort characters for fast searching.  We use a
			     * shell sort since this list could be large.
			     */
			    cshell( ccltbl + cclmap[yyvsp[0]], ccllen[yyvsp[0]] );

			if ( useecs )
			    mkeccl( ccltbl + cclmap[yyvsp[0]], ccllen[yyvsp[0]],
				    nextecm, ecgroup, CSIZE );
				     
			++rulelen;

			yyval = mkstate( -yyvsp[0] );
			;
    break;}
case 40:
#line 351 "parse.y"
{
			++rulelen;

			yyval = mkstate( -yyvsp[0] );
			;
    break;}
case 41:
#line 358 "parse.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 42:
#line 361 "parse.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 43:
#line 364 "parse.y"
{
			++rulelen;

			if ( yyvsp[0] == '\0' )
			    synerr( "null in rule" );

			if ( caseins && yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
			    yyvsp[0] = clower( yyvsp[0] );

			yyval = mkstate( yyvsp[0] );
			;
    break;}
case 44:
#line 378 "parse.y"
{ yyval = yyvsp[-1]; ;
    break;}
case 45:
#line 381 "parse.y"
{
			/* *Sigh* - to be compatible Unix lex, negated ccls
			 * match newlines
			 */
#ifdef NOTDEF
			ccladd( yyvsp[-1], '\n' ); /* negated ccls don't match '\n' */
			cclsorted = false; /* because we added the newline */
#endif
			cclnegate( yyvsp[-1] );
			yyval = yyvsp[-1];
			;
    break;}
case 46:
#line 395 "parse.y"
{
			if ( yyvsp[-2] > yyvsp[0] )
			    synerr( "negative range in character class" );

			else
			    {
			    if ( caseins )
				{
				if ( yyvsp[-2] >= 'A' && yyvsp[-2] <= 'Z' )
				    yyvsp[-2] = clower( yyvsp[-2] );
				if ( yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
				    yyvsp[0] = clower( yyvsp[0] );
				}

			    for ( i = yyvsp[-2]; i <= yyvsp[0]; ++i )
			        ccladd( yyvsp[-3], i );

			    /* keep track if this ccl is staying in alphabetical
			     * order
			     */
			    cclsorted = cclsorted && (yyvsp[-2] > lastchar);
			    lastchar = yyvsp[0];
			    }
			
			yyval = yyvsp[-3];
			;
    break;}
case 47:
#line 423 "parse.y"
{
			if ( caseins )
			    if ( yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
				yyvsp[0] = clower( yyvsp[0] );

			ccladd( yyvsp[-1], yyvsp[0] );
			cclsorted = cclsorted && (yyvsp[0] > lastchar);
			lastchar = yyvsp[0];
			yyval = yyvsp[-1];
			;
    break;}
case 48:
#line 435 "parse.y"
{
			cclsorted = true;
			lastchar = 0;
			yyval = cclinit();
			;
    break;}
case 49:
#line 443 "parse.y"
{
			if ( caseins )
			    if ( yyvsp[0] >= 'A' && yyvsp[0] <= 'Z' )
				yyvsp[0] = clower( yyvsp[0] );

			++rulelen;

			yyval = link_machines( yyvsp[-1], mkstate( yyvsp[0] ) );
			;
    break;}
case 50:
#line 454 "parse.y"
{ yyval = mkstate( SYM_EPSILON ); ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 303 "bison.simple"


  yyvsp -= yylen;
  yylsp -= yylen;
  yyssp -= yylen;

  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now", yyssp-yyss);
      while (ssp1 != yyssp)
   fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }

  *++yyvsp = yyval;

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
      yyerror("parse error");
    }

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
   YYERROR;

      if (yydebug)
   fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;      /* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYERROR;
  yyvsp--;
  yylsp--;
  yystate = *--yyssp;

  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now", yyssp-yyss);
      while (ssp1 != yyssp)
   fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }

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

  if (yydebug)
    fprintf(stderr, "Shifting error token, ");

  *++yyvsp = yylval;
  *++yylsp = yylloc;

  yystate = yyn;
  goto yynewstate;
}
#line 457 "parse.y"


/* synerr - report a syntax error
 *
 * synopsis
 *    char str[];
 *    synerr( str );
 */

synerr( str )
char str[];

    {
    syntaxerror = true;
    fprintf( stderr, "Syntax error at line %d:  %s\n", linenum, str );
    }


/* yyerror - eat up an error message from the parser
 *
 * synopsis
 *    char msg[];
 *    yyerror( msg );
 */

yyerror( msg )
char msg[];

    {
    }
