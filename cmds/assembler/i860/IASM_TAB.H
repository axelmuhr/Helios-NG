typedef union {
      Expression  *instr;
      Symbol      *symbol;
      Expression  *expression;
      int32        ival;
      char        *text;
      TextSeg     *textseg;
} YYSTYPE;
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


extern YYSTYPE yylval;
