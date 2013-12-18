/*
 * File:	lex.c
 * Subsystem:	Generic Assembler
 * Author:	P.A.Beskeen
 * Date:	Aug '91
 *
 * Description: lexical analyser for generic Helios assembler
 *
 *		This is the lower level lexical analyser that is called 
 *		from the higher level parser. Its job is to remove whitespace
 *		and comments from the input stream and return a series of
 *		tokens that describe the basic structure of the input.
 *
 *		Different processor targets cause different mneumonic
 *		keywords to be identified. Different processor targets may
 *		also recognise additional comment characters.
 *
 *
 * RcsId: $Id: lex.c,v 1.18 1993/07/12 16:17:56 nickc Exp $
 *
 * (C) Copyright 1991 Perihelion Software Ltd.
 * 
 * $RcsLog$
 *
 */


#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "gasm.h"
#include "y.tab.h"	/* token definitions automatically created by YACC */


/* Exported Functions: */

/* InitLex(void)	Initialise the lexical analyser			*/
/* int yylex(void)	return the next parse token from the input	*/
/*			called from yyparse()				*/
/* void	ClearInput(void) Clear input upto next \n			*/


/* Exported Variables: */

int CurLine = 1;	/* current lexical analyser input line number */


/* Internal Functions: */

#ifdef __STDC__
static int LexNames(int c);
static int LexNumericConst(int c);
static int LexCharConst(void);
static int LexStringConst(void);

static int ConvertCharESC(int c);

static int inputchar(void);
static void putback(int c);

static void InitPseudoOpKeywords(void);

static int numeric_const (char *buff, int c, int tok) ;
#endif


/* Internal Variables: */

/* used to flag character is at start of line */
static	int startofline = TRUE;
static	int EOFfound = FALSE;


/* Imported Functions: */

void InitMnemonicKeywords(void);


/************************************************************************/
/* yylex								*/
/*									*/
/* Lexical analyser for yyparse() higher level parser.			*/
/*									*/
/* This is the lower level lexical analyser that is called from the	*/
/* higher level parser. Its job is to remove whitespace and comments	*/
/* from the input stream and return a series of tokens that describe	*/
/* the basic structure of the input.					*/
/*									*/
/* It returns a token identifier and an associated value for that token	*/
/* in the global union yylval.						*/
/*									*/
/************************************************************************/

int yylex(void)
{
	int c;

	/* remove white space */

	while ((c=inputchar()) != EOF && isspace(c)) {
		if (c == '\n') {
			/* used for reporting line number in errors */
			CurLine++;

			/* keep track of line starts */
			/* used by some comment schemes */
			startofline = TRUE;
		}
		else
			startofline = FALSE;
	}

	/* quit lexing on EOF */
	if (c == EOF) {
#ifdef DEBUG
		fprintf(stderr,"EOF encountered\n");
#endif
		return 0;
	}

	/* remove standard line and block comments */
	if (c == '/') {
		if ((c = inputchar()) == '/') {
			/* remove line comment */
			while ((c=inputchar()) != EOF && c != '\n') ; /*null stat*/

			CurLine++;	/* keep line count correct */

			return yylex();
		}
		if (c == '*') {
			/* remove block comment */
			int foundstar = FALSE;

			while ((c=inputchar()) != EOF) {

				if (c == '\n')
					/* keep line count correct */
					CurLine++;

				if (c == '*')
					foundstar = TRUE;
				else {
					if (foundstar == TRUE) {
						if (c == '/')
							return yylex();
						else
							foundstar = FALSE;
					}
				}
			}
			Error("Encountered end of file inside a block comment");
			return 0;
		}
		putback(c);
		c = '/';
	}

#if defined(__C40TARGET) || defined(__ARMTARGET)
	/* remove Texas Instruments style assembler comments */
	if (startofline && c == '*') {
		while ((c=inputchar()) != EOF && c != '\n') ; /*null stat*/

		CurLine++;		/* keep line count correct */
		return yylex();
	}
	if (c == ';') {
		while ((c=inputchar()) != EOF && c != '\n') ; /*null stat*/

		CurLine++;		/* keep line count correct */
		return yylex();
	}
#endif
	/* check for known keywords, labels and symbol names */
	/* these names must start with either an alpha or '_', or ".' */
	
	if (isalpha(c) || c == '.' || c == '_')
		return LexNames(c);

	/* test for number constants */
	if (isdigit(c))
		return LexNumericConst(c);

	/* test for character constants */
	if (c == '\'')
		return LexCharConst();

	/* test for string constants */
	if (c == '\"')
		return LexStringConst();

	/* check for left and right shift operators */
	if (c == '<') {
		if ((c=inputchar()) != EOF && c == '<')
			return LSHIFT;
		putback(c);
		return '<';
	}
	if (c == '>') {
		if ((c=inputchar()) != EOF && c == '>')
			return RSHIFT;
		putback(c);
		return '>';
	}

	/* ++ and -- used in indirect addressing of many processors */
	/* check for pre/post increment operator ++ */
	if (c == '+') {
		if ((c=inputchar()) != EOF && c == '+')
			return PLUSPLUS;
		putback(c);
		return '+';
	}
	/* check for pre/post decrement operator -- */
	if (c == '-') {
		if ((c=inputchar()) != EOF && c == '-')
			return MINUSMINUS;
		putback(c);
		return '-';
	}

#ifdef __C40TARGET
	/*  parallel instruction specifier '||' */
	if (c == '|') {
		if ((c=inputchar()) != EOF && c == '|')
			return BARBAR;
		putback(c);
		return '|';
	}
#endif

	/* character unknown to lexical analyser - pass on to parser */
	return c;
}


/************************************************************************/
/* LexNames								*/
/*									*/
/* Checks for known keywords, labels and symbol names			*/
/*									*/
/* After the main lex fn has found a alpha character, this function is	*/
/* called to read in the remaining name and return the correct token	*/
/* and its associated value for processing by the parser.		*/
/*									*/
/* Returns parse token (keyword token | LABEL | NAME )			*/
/*									*/
/************************************************************************/

static int LexNames(int c)
{
	int	length = 16;
	char *	buf    = (char *)malloc(length);
	int	i = 0;
	Symbol	*s = NULL;

	do {
		if (i >= length - 1) {
			length += length;
			if ((buf = (char *)realloc(buf, length)) == NULL)
				Fatal("Out of memory in symbol space allocation");
		}
		buf[i++] = c;
	} while ((c = inputchar()) != EOF && (isalnum(c) || c == '.' || c == '_'));

	putback(c);
	buf[i] = '\0';

	/* check for pseudo op keywords and target CPU mnemonics */
	if ((s = CaseInsensitiveFindSymb(buf)) != NULL) {
		if (s->what == HT_PSEUDO)
			/* If we have found a pseudo opcode, */
			/* just return token type */
			return s->type.token;

		if (s->what == HT_TOKENVAL) {
			/* If we have found a register/patch/extra, */
			/* return its token type and value */
			yylval.num = s->type.misc->value;

			return s->type.misc->token;
		}

		if (s->what == HT_MNEMONIC) {
			/* If we have found a machine opcode */
			/* pass a cpu specific structure to the parser */

			yylval.mnem = s->type.mnemonic;

			/* return machine opcode mnemonic's token type */
			return (s->type.mnemonic->token);
		}

		/* pass previously defined labels back as names */
		if (s->what != HT_LABEL && s->what != HT_IMPORT ) {
			char Err[80];

			sprintf(Err, "(internal) unknown internal symbol type returned %d", s->what);
			Fatal(Err);
		}
	}

	/* Is symbol a label definition? - check for trailing ':' */
	/* remove any white space */
	while ((c = inputchar()) != EOF && isspace(c)) {
		if (c == '\n') {
			CurLine++;
			startofline = TRUE;
		}
		else
			startofline = FALSE;
	}

	if (c == ':') {
		yylval.str = buf;
		return LABEL;
	} else
		putback(c);

	/* if unknown keyword return symbol (NAME) token */
	yylval.str = buf;		/* pass on value of name found */

	return NAME;
}


/************************************************************************/
/* LexNumericConst							*/
/*									*/
/* Reads in numeric constants and converts them to binary.		*/
/*									*/
/* After the main lex fn has found a numeric character, this function	*/
/* is called to read in the remaining number, convert it to binary and	*/
/* return the 'NUMBER' parse token and the value we have just converted.*/
/*									*/
/* The fn understands decimal, hexadecimal and binary formatted numbers.*/
/*									*/
/* Returns parse token (NUMBER).					*/
/*									*/
/************************************************************************/

static int LexNumericConst(int c)
{
	if (c == '0') {
		/* possible hex or bin number */
		c = inputchar();
		if (c == 'x' || c == 'X') {
			/* convert textual hexadecimal number */
			yylval.num = 0;
			while (isxdigit(c = inputchar())) {
				yylval.num *= 16;
				if (isdigit(c))
					yylval.num += c - '0';
				else
					yylval.num += 10 + (toupper(c) - 'A');
			}

			putback(c);
			return NUMBER;
		}
		else if (c == 'b' || c == 'B') {
			/* convert textual binary number */
			yylval.num = 0;
			while ((c = inputchar()) == '1' || c == '0') {
				yylval.num *= 2;
				yylval.num += c - '0';
			} 

			putback(c);
			return NUMBER;
		}
#if 0 /* testing only */
		else if (c == 'f' || c == 'F') {
			/* convert bogus FP number */
			yylval.flt = 0;

			while (isxdigit(c = inputchar())) {
				yylval.flt *= 16;
				if (isdigit(c))
					yylval.flt += c - '0';
				else
					yylval.flt += 10 + (toupper(c) - 'A');
			}

			putback(c);
			return FLOATNUM;
		}
#endif /* testing only */
		/* drop back to decimal processing */
		putback(c);
		c = '0';
	}

#if 0 /* (No FP support) */
	/* convert textual decimal number */
	yylval.num = 0;
	while (isdigit(c)) {
		yylval.num *= 10;
		yylval.num += c - '0';
		c = inputchar();
	}

	putback(c);
	return NUMBER;
#else /* (FP support) */
	/* convert textual decimal / floating point number */

#define GET_NUM(c, p)	\
	while (isdigit (c)) { *p++ = c ; c = inputchar () ; }

	{
		char buff [256] ;
		char *p = buff ;
		memset (p, 0, sizeof (buff)) ;
#ifdef HOST_SUPPORTS_IEEE
		yylval.flt = 0.0 ;
#else
		fltrep_stod ("0.0", &yylval.flt) ;
#endif
		yylval.num = 0 ;
		GET_NUM(c, p)
/*
-- crf: Note - I am assuming that floating point constants *must* contain a
-- decimal point. If I don't find one here, I return NUMBER.
*/
		if (c == '.')
		{
			*p++ = c ;
			c = inputchar () ;
			if (isdigit (c))
				GET_NUM(c, p)
			/* get exponent */
			if (c == 'e' || c == 'E')
			{
				*p++ = c ;
				c = inputchar () ;
				if (c == '+' || c == '-')
				{
					*p++ = c ;
					c = inputchar () ;
				}
				if (isdigit (c))
					GET_NUM(c, p)
				else
				{
/*
-- @@@
-- crf: Invalid floating point format (e.g. 1.0e or 1.0e+). Not sure how to 
-- handle this ... for now, I will generate a warning and attempt to recover
-- (i.e. ignore the exponent). This is obviously tricky if a sign character 
-- has been read in - putback(c) will be called twice.
*/
					Warn ("Invalid floating point format (ignoring exponent)") ;
					c = *(--p) ;
					if (c == 'e' || c == 'E')
						*p = '\0' ;
					else
					{
						putback (c) ;
						c = *(--p) ;
						*p = '\0' ;
					}
				}
			} /* if (c == 'e' || c == 'E') */
			return (numeric_const (buff, c, FLOATNUM)) ;
		} /* if (c == '.') */
		else
			return (numeric_const (buff, c, NUMBER)) ;
	}
#endif /* (FP support) */
}


/************************************************************************/
/* LexCharConst								*/
/*									*/
/* Reads in character constants						*/
/*									*/
/* After the main lex fn has found a character constant starter		*/
/* character, this function is called to read in the constant, convert	*/
/* it to binary and returning the 'CHARCONST' parse token this value.	*/
/*									*/
/* The fn will allow up to 4 consecutive chars in character constants.	*/
/* This allows characters to be packed into words.			*/
/*									*/
/* Returns parse token (CHARCONST).					*/
/*									*/
/************************************************************************/

static int LexCharConst(void)
{
	int	i = 0;
	int	c;

	yylval.num = 0;

	while ((c = inputchar()) != EOF && c != '\'') {

		if (iscntrl(c) && c != '\t') {
			Warn("Control character encountered in character constant (ignoring character)");
			continue;
		}

		/* check for character escapes */
		if ( c == '\\') {
			if ((c = ConvertCharESC(inputchar())) == 0) 
				continue;
		}

		/* insert character into word sized container */
		yylval.num |= (yylval.num << 8) | c;
		i++;
	}

	if (i > 4)
		Error("More than 4 characters in a character constant");
	else if (i == 0) {
		Error("Empty character constant");
	}

	if (c == EOF) {
		Error("Unexpected end of file encountered while reading character constant");
		return 0;
	}

	return CHARCONST;
}


/************************************************************************/
/* LexStringConst							*/
/*									*/
/* Reads in string constants						*/
/*									*/
/* After the main lex fn has found a string constant starter		*/
/* character, this function is called to read in the string and closing	*/
/* string character, returning the 'STRINGCONST' parse token and the	*/
/* string.								*/
/*									*/
/* Returns parse token (CHARCONST).					*/
/*									*/
/************************************************************************/

static int LexStringConst(void)
{
	int	length = 40;
	char *	buf    = (char *)malloc(length);
	int	i = 0;
	int	c;


	while ((c = inputchar()) != EOF && c != '\"') {
			if (iscntrl(c) && c != '\t') {
				Warn("Control character encountered in string constant (ignoring character) ");
				continue;
			}

			/* check for character escapes */
			if ( c == '\\') {
				if ((c = ConvertCharESC(inputchar())) == 0)
					continue;
			}

			if (i >= length - 1) {
				length += length;
				if ((buf = (char *)realloc(buf, length)) == NULL)
					Fatal("Out of memory in string constant allocation");
			}
			buf[i++] = c;
		}

	if (c == EOF) {
		Error("Unexpected end of file encountered while reading string constant");
		return 0;
	}

	if (i == 0) {
		Error("Empty string constant (string constants are NOT zero terminated)");
		buf[i++] = '?';		/* continue processing text */
	}

	buf[i] = '\0';
	yylval.str = buf;		/* return value of string constant */
	return STRINGCONST;
}


/************************************************************************/
/* ConvertCharEsc							*/
/*									*/
/* Convert standard ANSI character escape sequences into their		*/
/* associated control character.					*/
/*									*/
/* ConvertEscChar() is called after an escape prefix '\' character has	*/
/* been recognised, it takes the next character from the input stream,	*/
/* converting it if it recognises it, or rolling back and ignoring it	*/
/* if it doesn't.							*/
/*									*/
/* Returns control character associated with character escape		*/
/*									*/
/************************************************************************/

static int ConvertCharESC(int c)
{
	switch (c) {
	  case 'n':
		return '\n';	/* newline (linefeed) */
	  case 'r':
		return '\r';	/* carrige return */
	  case 'a':
		return '\a';	/* bell */
	  case 'b':
		return '\b';	/* backspace */
	  case 'f':
		return '\f';	/* formfeed */
	  case 't':
		return '\t';	/* tab */
	  case 'v':
		return '\v';	/* vertical tab */
	  case '0':
		return '\0';	/* nul */
	  case '\\':
		return '\\';	/* backslash */
	  case '\'':
		return '\'';	/* single quote */
	  case '\"':
		return '\"';	/* double quote */

	  default:
		Error("Found unknown character escape sequence");
		return '?'; /* continue processing */
	}

	return 0;	/* happy compiler */
}


/************************************************************************/ 
/* inputchar and putback						*/
/*									*/
/* Simple low level character input and input rollback functions	*/
/*									*/
/************************************************************************/

static int inputchar()
{
	if (EOFfound == TRUE)
		return EOF;

	return fgetc(InputFile);
}

static void putback(int c)
{
	if (c == EOF)
		EOFfound = TRUE;
	else
		ungetc(c, InputFile);

	return;
}


/************************************************************************/
/* InitPseudoOpKeywords							*/
/*									*/
/* Add all the pseudo opcode keywords to the hash table.		*/
/* This is to increase the recognition speed.				*/
/*									*/
/************************************************************************/

static void InitPseudoOpKeywords(void)
{
	NewSymb("byte",		HT_PSEUDO,	BYTE);
	NewSymb("char",		HT_PSEUDO,	BYTE);		/* synonym */
	NewSymb("short",	HT_PSEUDO,	SHORT);
	NewSymb("word",		HT_PSEUDO,	WORD);
	NewSymb("int",		HT_PSEUDO,	WORD);		/* synonym */
	NewSymb("long",		HT_PSEUDO,	WORD);		/* synonym */
	NewSymb("blockchar",	HT_PSEUDO,	BLOCKBYTE);
	NewSymb("blockbyte",	HT_PSEUDO,	BLOCKBYTE);	/* synonym */
	NewSymb("blkb",		HT_PSEUDO,	BLOCKBYTE);	/* synonym */
	NewSymb("blockshort",	HT_PSEUDO,	BLOCKSHORT);
	NewSymb("blks",		HT_PSEUDO,	BLOCKSHORT);	/* synonym */
	NewSymb("blockint",	HT_PSEUDO,	BLOCKWORD);
	NewSymb("blocklong",	HT_PSEUDO,	BLOCKWORD);	/* synonym */
	NewSymb("blockword",	HT_PSEUDO,	BLOCKWORD);	/* synonym */
	NewSymb("blkw",		HT_PSEUDO,	BLOCKWORD);	/* synonym */
	NewSymb("ieee32",	HT_PSEUDO,	FLOATY);
	NewSymb("ieee64",	HT_PSEUDO,	DOUBLE);
#ifdef __C40TARGET
	NewSymb("c40float",	HT_PSEUDO,	C40FLOAT);
#endif
	NewSymb("space",	HT_PSEUDO,	SPACE);
	NewSymb("bss",		HT_PSEUDO,	SPACE);		/* synonym */
	NewSymb("align",	HT_PSEUDO,	ALIGN);
	NewSymb("data",		HT_PSEUDO,	DATA);
	NewSymb("common",	HT_PSEUDO,	COMMON);
	NewSymb("codetable",	HT_PSEUDO,	CODETABLE);
	NewSymb("codestub",	HT_PSEUDO,	CODESTUB);
#ifdef __C40TARGET
	NewSymb("addrstub",	HT_PSEUDO,	ADDRSTUB);
#endif
	NewSymb("import",	HT_PSEUDO,	IMPORT);
	NewSymb("extern",	HT_PSEUDO,	IMPORT);	/* synonym */
	NewSymb("export",	HT_PSEUDO,	EXPORT);
	NewSymb("global",	HT_PSEUDO,	EXPORT);	/* synonym */
	NewSymb("modsize",	HT_PSEUDO,	MODSIZE);
	NewSymb("modnum",	HT_PSEUDO,	MODNUM);
	NewSymb("datasymb",	HT_PSEUDO,	DATASYMB);
	NewSymb("codesymb",	HT_PSEUDO,	CODESYMB);
	NewSymb("datamodule",	HT_PSEUDO,	DATAMODULE);
	NewSymb("labelref",	HT_PSEUDO,	LABELREF);
	NewSymb("byteswap",	HT_PSEUDO,	BYTESWAP);
	NewSymb("shift",	HT_PSEUDO,	SHIFT);
	NewSymb("p_add",	HT_PSEUDO,	P_ADD);
	NewSymb("p_or",		HT_PSEUDO,	P_OR);
	NewSymb("module",	HT_PSEUDO,	MODULE);
	NewSymb("init",		HT_PSEUDO,	INIT);
	NewSymb("ref",		HT_PSEUDO,	REF);
	NewSymb("patchinstr",	HT_PSEUDO,	PATCHINSTR);

	NewSymb("error",	HT_PSEUDO,	USERERROR);
	NewSymb("warning",	HT_PSEUDO,	USERWARNING);
	NewSymb("note",		HT_PSEUDO,	USERNOTE);

	NewSymb("end",		HT_PSEUDO,	0);
}

/************************************************************************/
/* numeric_const							*/
/*									*/
/* Support routine for LexNumericConst()				*/
/* Converts character string to integer / double / IEEE 64 bit integer	*/
/* representation and returns token type.				*/
/*									*/
/************************************************************************/

/*
-- @@@
-- crf: Should we be testing the result of the conversion (e.g. for over/
-- underflow) ? At some point, this will probably be desirable (hence the 
-- use of strtod() and strtol() as opposed to other conversion routines).
*/

static int numeric_const (char *buff, int c, int tok)
{
	char *endptr ;
	switch (tok)
	{
		case FLOATNUM:
#ifdef HOST_SUPPORTS_IEEE
			yylval.flt = strtod (buff, &endptr) ;
#else
			fltrep_stod (buff, &yylval.flt) ;
#endif
			break ;
		case NUMBER:
			yylval.num = (int) strtol (buff, &endptr, 10) ;
			break ;
		default:
			{
				char Err[80];
				sprintf(Err, "(internal): numeric_const() - unknown token type: %d\n", tok) ;
				Fatal(Err);
			}
	}
	putback (c) ;
	return tok ;
}

/************************************************************************/
/* ClearInput								*/
/*									*/
/* Recover from parse errors by reading the remaining characters on the	*/
/* current line.							*/
/*									*/
/************************************************************************/

void	ClearInput(void)
{
	int	c;

	while((c=inputchar()) != EOF && c != '\n')	; /*null stat*/

	CurLine++;		/* keep line count accurate */
}


/************************************************************************/
/* InitLex								*/
/*									*/
/* Initialise the lexical analyser.					*/
/* Add the keywords we wish to recognise to the hash table.		*/
/* This allows us to recognise them efficently.				*/
/*									*/
/************************************************************************/

void InitLex(void)
{
	InitMnemonicKeywords();
	InitPseudoOpKeywords();
}



/* end of lex.c */
