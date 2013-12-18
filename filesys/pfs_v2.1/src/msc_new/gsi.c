/*
 * $Header: /Chris/00/helios/msc/RCS/gsi.c,v 2.0 91/08/21 18:07:43 chris Exp
 * Locker: chris $
 */

/*************************************************************************
**									**
**		    M S C   D I S C   D E V I C E			**
**		    -----------------------------			**
**									**
**		  Copyright (C) 1990, Parsytec GmbH			**
**			 All Rights Reserved.				**
**									**
**									**
** gsi.c								**
**									**
**	- Generate SCSI Info						**
**									**
**************************************************************************
** HISTORY   :								**
** -----------								**
** Author    :	xx/xx/90 : C. Fleischer					**
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <ctype.h>
#include <syslib.h>
#include <device.h>
#include <string.h>
#include <setjmp.h>
#include <module.h>

#include "scsiinfo.h"

/* Symbol types from the scanner					 */

#define S_LBracket	1		/* left curly bracket '{'	 */
#define S_RBracket	2		/* right curly bracket '}'	 */
#define S_Keyword	3		/* keyword from table		 */
#define S_Number	4		/* number			 */
#define S_String	5		/* string			 */
#define S_EOF		6		/* end of input file		 */
#define S_Error		-1		/* bad symbol			 */

/* Keyword Symbol definition structure					 */

typedef struct KeySymbol {
    char           *Name;		/* Keyword			 */
    int             Type;		/* Token Type			 */
    int             ArgType;		/* Argument Type		 */
    int             Offset;		/* Offset in its structure	 */
}               KeySymbol;

/* Token Type values							 */

#define T_ScsiDev	Info_ScsiDev
#define T_Param		-1
#define T_Type		-2

/* OI 27 Jan 92
#define T_Sync		-4
*/

#define T_Ident		-5

#define T_Command	-6
#define T_Name		-7
#define T_Direction	-8
#define T_Blockmove	-9
#define T_CDBSize	-10
#define T_CDB		-11
#define T_DataSize	-12
#define T_Data		-13

#define T_Error		-14
#define T_ErrCode	-15
#define T_Condition	-16

#define T_Request	-17
#define T_FnCode	-18
#define T_Item		-19

/* Argument Type values							 */

#define AT_None		0
#define AT_Number	(S_Number)
#define AT_String	(S_String)
#define AT_List		(S_LBracket)
#define AT_Keyword	(S_Keyword)

/* Token tables								 */

KeySymbol       TopTab[2] =
{
    {"device", T_ScsiDev, AT_String, offsetof (InfoNode, Name)},
    {0, 0, 0, 0}
};

KeySymbol       ScsiDevTab[6] =
{
    {"type", T_Type, AT_None, offsetof (ScsiDevInfo, Type)},

/* OI  27 Jan 92

    {"synchronous", T_Sync, AT_None, offsetof (ScsiDevInfo, Synchronous)},
*/

    {"ident", T_Ident, AT_String, offsetof (ScsiDevInfo, Ident)},
    {"command", T_Command, AT_None, offsetof (ScsiDevInfo, Commands)},
    {"error", T_Error, AT_None, offsetof (ScsiDevInfo, Errors)},
    {"request", T_Request, AT_None, offsetof (ScsiDevInfo, Requests)},
    {0, 0, 0, 0}
};

KeySymbol       RawStrTab[5] =
{
    {"random", 0},
    {"sequential", 1},
    {"raw", 1},
    {"structured", 0},
    {0, 0}
};

KeySymbol       YesNoTab[3] =
{
    {"yes", 1},
    {"no", 0},
    {0, 0}
};

KeySymbol       CommandTab[8] =
{
    {"name", T_Name, AT_String, offsetof (CmdInfo, Name)},
    {"read", T_Direction, AT_None, offsetof (CmdInfo, Read)},
    {"blockmove", T_Blockmove, AT_None, offsetof (CmdInfo, Blockmove)},
    {"cdbsize", T_CDBSize, AT_Number, offsetof (CmdInfo, CDBSize)},
    {"cdb", T_CDB, AT_List, offsetof (CmdInfo, CDB)},
    {"datasize", T_DataSize, AT_Number, offsetof (CmdInfo, DataSize)},
    {"data", T_Data, AT_List, offsetof (CmdInfo, Data)},
    {0, 0, 0, 0}
};

KeySymbol       ErrorTab[3] =
{
    {"code", T_ErrCode, AT_Number, offsetof (ErrorInfo, Code)},
    {"condition", T_Condition, AT_None, offsetof (ErrorInfo, Conditions)},
    {0, 0, 0, 0}
};

KeySymbol       ConditionTab[4] =
{
    {"offset", T_Param, AT_Number, offsetof (CondInfo, Offset)},
    {"mask", T_Param, AT_Number, offsetof (CondInfo, Mask)},
    {"value", T_Param, AT_Number, offsetof (CondInfo, Value)},
    {0, 0, 0, 0}
};

KeySymbol       RequestTab[3] =
{
    {"fncode", T_FnCode, AT_Number, offsetof (ReqInfo, FnCode)},
    {"item", T_Item, AT_None, offsetof (ReqInfo, Items)},
    {0, 0, 0, 0}
};

/* Default entries for all Types					 */
struct {
    ScsiDevInfo     ScsiDevInfo;
    CmdInfo         CmdInfo;
    ErrorInfo       ErrorInfo;
    CondInfo        CondInfo;
    ReqInfo         ReqInfo;
    ItemInfo        ItemInfo;
}               Default =

{
    {
	vvt_structured, 0, -1, -1, -1, -1
    },
    {
	-1, -1, -1, 0, 0, -1, -1, -1, -1
    },
    {
	-1, 0, -1
    },
    {
	-1, 0, 0, 1
    },
    {
	-1, 0, -1
    },
    {
	-1, -1
    }
};

/* List of error messages						 */

char           *Error_Msg[28] =
{
    "Output buffer full",		/* 0	 */
    "Digit '%c' too large for base %d",	/* 1	 */
    "Cannot allocate memory for command",	/* 2	 */
    "Missing command definition for '%s'",	/* 3	 */
    "Wrong direction for '%s'",		/* 4	 */
    "Quote (\") expected before newline",	/* 5	 */
    "Unexpected character '%c'",	/* 6	 */
    "'%s' expected",			/* 7	 */
    "%s expected",			/* 8	 */
    "Internal argument type error (%d)",/* 9	 */
    "Keyword or '}' expected",		/* 10	 */
    "Value 0x%x out of byte range",	/* 11	 */
    "Too many arguments for this list",	/* 12	 */
    "Duplicate definition of '%s'",	/* 13	 */
    "Offset greater than 'Request_Sense' reply",	/* 14	 */
    "Missing definition for '%s'",	/* 15	 */
    "%s is either '%s' or '%s'",	/* 16	 */
    "Unknown command '%s'",		/* 17	 */
    "Cannot open input file '%s'",	/* 18	 */
    "Cannot allocate memory for input buffer",	/* 19	 */
    "Cannot read input file '%s'",	/* 20	 */
    "Cannot open output file '%s'",	/* 21  */
    "Numeric argument or '}' expected",	/* 22	 */
    "CDB size out of range",		/* 23	 */
    "Wrong CDB size for command group %d",	/* 24	 */
    "Missing CDB size for unknown command group",	/* 25	 */
    "Duplicate command definition for '%s'",	/* 26	 */
    NULL
};

typedef struct CmdDef {
    char           *Name;
    word            Dir;
    word            Mask;
    word            Code;
}

                CmdDef;

#define CM_Random	0x01		/* Mandatory for Random		 */
#define CM_Sequential	0x02		/* Mandatory for Sequential	 */

#define CD_Write	0
#define CD_Read		1
#define CD_Unknown	2

CmdDef          StdCmds[16] =
{
    {"Test_Unit_Ready", CD_Unknown, 0x03, CC_Test_Unit_Ready},
    {"Rewind", CD_Unknown, 0x02, CC_Rewind},
    {"Request_Sense", CD_Read, 0x03, CC_Request_Sense},
    {"Read", CD_Read, 0x03, CC_Read},
    {"Write", CD_Write, 0x03, CC_Write},
    {"Write Filemarks", CD_Unknown, 0x02, CC_Write_Filemarks},
    {"Space", CD_Unknown, 0x02, CC_Space},
    {"Inquiry", CD_Read, 0x03, CC_Inquiry},
    {"Mode_Sense", CD_Read, 0x03, CC_Mode_Sense},
    {"Format", CD_Write, 0x01, CC_Format},
    {"Prevent_Media_Removal", CD_Unknown, 0x00, CC_Prevent_Media_Removal},
    {"Allow_Media_Removal", CD_Unknown, 0x00, CC_Allow_Media_Removal},
    {"Reassign_Blocks", CD_Write, 0x01, CC_Reassign_Blocks},
    {"Read_Capacity", CD_Read, 0x01, CC_Read_Capacity},
    {"Verify", CD_Write, 0x01, CC_Verify},
    {NULL, CD_Unknown, 0x00, CC_Unknown}
};

typedef struct CmdNode {
    Node            Node;		/* standard list node		 */
    word            Offset;		/* CmdInfo struct offset	 */
    char            Name[1];		/* Command Name			 */
}

                CmdNode;

/* Global variables							 */

int             Symbol;			/* Symbol type			 */
KeySymbol      *TokSym;			/* Token table ptr for keyword	 */
int             TokIndex;		/* Token index in Token table	 */
union {
    int             i;
    char           *s;
}               TokVal;			/* symbol value for params	 */
char            Token[128];		/* Token buffer			 */

int             LineNum = 1;
int             LStart = 0;
int             TStart = 0;
bool            EStart = TRUE;

char           *Input = NULL;		/* Input buffer			 */
int             Isize = 0;		/* Size of input buffer		 */
int             Ipos = 0;		/* Current read position	 */
int             DevCount = 0;		/* Device number for devinfo	 */

/* CF:
#define OutMax	2000
*/
/* OI  08 Jan 1992 */
#define OutMax	10000

word            Output[OutMax];		/* Output buffer		 */
word            Osize = 0;		/* Output position (WORDS !)	 */

List            CmdList;		/* List of defined Commands	 */

jmp_buf         Error_Jmp;


/************************************************************************
 * PRINT AN ERROR MESSAGE
 *
 * - Print the error message using vprintf ().
 *
 * Parameter :	msg	= Error message text
 *		...	= Message arguments
 *
 ***********************************************************************/

static void
ErrMsg (int i,...)
{
    va_list         args;
    char           *cp;

    if (EStart) {
	cp = strpbrk (&Input[LStart], "\r\n\0");
	if (cp)
	    *cp = '\0';
	puts (&Input[LStart]);
	while (TStart > LStart)
	    putchar (Input[LStart++] == '\t' ? '\t' : ' ');
	puts ("^");
	EStart = FALSE;
    }
    printf ("Error on line %d : ", LineNum);
    va_start (args, i);
    vprintf (Error_Msg[i], args);
    putchar ('\n');
    va_end (args);
}

/************************************************************************
 * EXIT AFTER AN ERROR MESSAGE
 *
 * - Jump to the end of main ().
 *
 ***********************************************************************/

static void
ErrJmp (void)
{
    longjmp (Error_Jmp, 1);
}

/************************************************************************
 * PRINT AN ERROR MESSAGE AND EXIT
 *
 * - Print the error message using vprintf (), then jump...
 *
 * Parameter :	msg	= Error message text
 *		...	= Message arguments
 *
 ***********************************************************************/

static void
Error (int i,...)
{
    va_list         args;
    char           *cp;

    if (EStart) {
	cp = strpbrk (&Input[LStart], "\r\n\0");
	if (cp)
	    *cp = '\0';
	puts (&Input[LStart]);
	while (TStart > LStart)
	    putchar (Input[LStart++] == '\t' ? '\t' : ' ');
	puts ("^");
	EStart = FALSE;
    }
    printf ("Error on line %d : ", LineNum);
    va_start (args, i);
    vprintf (Error_Msg[i], args);
    putchar ('\n');
    va_end (args);
    longjmp (Error_Jmp, 1);
}

/************************************************************************
 * EXTEND THE OUTPUT BUFFER BY A NUMBER OF BYTES
 *
 * Parameter :	bsize	= number of bytes necessary
 * Result    :	index	of extension area (WORD)
 *
 ***********************************************************************/

int
Extend (int bsize)
{
    int             Oldsize = Osize;	/* save old buffer end		 */

/* advance to next word offset	 */
    Osize += ((bsize + 3) & ~3) / sizeof (word);
    if (Osize >= OutMax)
	Error (0);
    return Oldsize;			/* return old buffer end	 */
}

/************************************************************************
 * READ THE NEXT CHARACTER FROM THE INPUT BUFFER
 *
 * Result    :	next character or EOF if buffer is exhausted
 *
 ***********************************************************************/

int
ReadChar (void)
{
    if (Ipos >= Isize)
	return EOF;
    else
	return Input[Ipos++];
}

/************************************************************************
 * UNREAD A CHARACTER, PLACE IT INTO THE INPUT BUFFER AGAIN
 *
 * Parameter :	c	= character to unread
 *
 ***********************************************************************/

void
UnreadChar (int c)
{
    Ipos--;
    Input[Ipos] = c;
}

/************************************************************************
 * ADD A STRING TO THE OUTPUT
 *
 * - extend the buffer by the string length, copy the string to
 *   the old buffer end position
 *
 * Parameter :	s	= String to be added
 * Result    :	index	of string start
 *
 ***********************************************************************/

int
NewString (char *s)
{
/* extend output buffer		 */
    int             offset = Extend (strlen (s) + 1);

/* get target ptr (old bufend)	 */
    char           *t = (char *) &Output[offset];

    strcpy (t, s);			/* copy string			 */
    return offset;			/* return string start position	 */
}

/************************************************************************
 * READ AN INTEGER FROM THE INPUT BUFFER
 *
 * - Check for an optional sign, evaluate the base and read the value
 *
 * Parameter :	c	= first number character
 * Result    :	value	of the number
 *
 ***********************************************************************/

int
ctoi (int c)
{
    int             val = 0;
    int             d;
    int             base = 10;
    int             sign = 1;

    if (c == '-') {			/* check for lasding '-' sign	 */
	sign = -1;
	c = ReadChar ();
    }
    if (c == '0') {			/* check for base		 */
	if ((c = ReadChar ()) == 'x') {
	    c = ReadChar ();
	    base = 16;
	} else {
	    UnreadChar (c);
	    base = 8;
	}
    }
    forever				/* read subsequent digits	 */
    {
	if ('0' <= c && c <= '9')
	    d = c - '0';
	elif ('A' <= c && c <= 'F')
	  d = c - '7';
	elif ('a' <= c && c <= 'f')
	  d = c - 'W';
	else
	break;

	if (d >= base)			/* check for base overflow	 */
	    Error (1, c, base);

	val = val * base + d;
	c = ReadChar ();
    }
    UnreadChar (c);			/* unread mon-=number character	 */
    return val * sign;
}

/************************************************************************
 * FREE A HELIOS LIST
 *
 * - Free all nodes contained in the list.
 *
 * Parameter :	List	= List to be emptied
 *
 ***********************************************************************/

static void
FreeList (List * List)
{
    Node           *Node1 = List->Head;
    Node           *Node2;

    while ((Node2 = Node1->Next) != NULL) {
	Free (Remove (Node1));
	Node1 = Node2;
    }
}

/************************************************************************
 * LOWERCASE STRING COMPARE
 *
 * Parameter :	a	= string with unknown case
 *		b	= lowercase string
 * Result    :	> 0	if a is literally greater than b
 *		0	if strings are equal
 *		< 0	if a is literally less than b
 *
 ***********************************************************************/

static int
lstrcmp (char *a, char *b)
{
    char            aa, bb;

    forever
    {
	if ((aa = tolower (*a++)) != (bb = tolower (*b++)))
	    return aa - bb;
	if (aa == 0)
	    return 0;
    }
}

/************************************************************************
 * COMMAND COMPARE
 *
 * - Commands are compared case-insensitive,
 *   and spaces and underscores are regarded as equal
 *
 * Parameter :	a	= user defined Command name
 *		b	= Command name from Command table
 * Result    :	!= 0	if commands do not match
 *		0	if commands match
 *
 ***********************************************************************/

static int
cmdcmp (char *a, char *b)
{
    char            aa, bb;

    forever
    {
	aa = tolower (*a++);
	bb = tolower (*b++);
	if (aa == '_')
	    aa = ' ';
	if (bb == '_')
	    bb = ' ';
	if (aa != bb)
	    return 1;
	if (aa == 0)
	    return 0;
    }
}

/************************************************************************
 * COMPARE A COMMAND NODE WITH A COMMAND
 *
 * - This function is used as an argument to SearchList ().
 *
 * Parameter :	Node	= CmdNode
 *		arg	= Command
 * Return    :	0	if the names do not match
 *		1	if the names match
 *
 ***********************************************************************/

static          word
CmpCommand (Node * Node, word * arg)
{
    return !cmdcmp (((CmdNode *) Node)->Name, (char *) arg);
}

static CmdNode *
FindCommand (char *Cmd)
{
    return (CmdNode *) SearchList (&CmdList, CmpCommand, (word) Cmd);
}

/************************************************************************
 * ADD A COMMAND TO THE COMMAND LIST
 *
 * - Allocate a new node, initialise it and append it to the command list.
 *
 * Parameter :	Cmd	= Command name as defined by the user
 *		Offset	= Offset of Command struct
 *
 ***********************************************************************/

static void
AddCommand (char *Cmd, word Offset)
{
    CmdNode        *NewCmd;

    if (FindCommand (Cmd) != NULL)
	Error (26, Cmd);

    NewCmd = Malloc (sizeof (CmdNode) + strlen (Cmd));
    if (NewCmd == NULL) {
	FreeList (&CmdList);
	Error (2);
    }
    NewCmd->Offset = Offset;
    strcpy (NewCmd->Name, Cmd);
    AddTail (&CmdList, (Node *) NewCmd);
}

/************************************************************************
 * COMPARE THE COMMAND LIST WITH A TABLE OF NECESSARY COMMANDS
 *
 * - Check whether all commands from the table are contained in the
 *   Command list. Print a message for each missing command.
 *
 * Parameter :	Cmds	= Command table
  * Return    :	0	if the names do not match
 *		1	if the names match
 *
 ***********************************************************************/

static void
CheckCommands (word Type)
{
    CmdNode        *CNode;
    CmdInfo        *CInfo;
    CmdDef         *Cmd = StdCmds;
    word            mask = 1 << Type;
    bool            error = FALSE;

    while (Cmd != NULL && Cmd->Name != NULL) {
	CNode = (CmdNode *) SearchList (&CmdList, CmpCommand, (word) Cmd->Name);
	if (CNode == NULL) {
	    if (Cmd->Mask & mask) {
		error = TRUE;
		ErrMsg (3, Cmd->Name);
	    }
	} else {
	    CInfo = (CmdInfo *) & Output[CNode->Offset];
	    CInfo->Code = Cmd->Code;
	    if (Cmd->Dir < CD_Unknown && Cmd->Dir != CInfo->Read) {
		ErrMsg (4, Cmd->Name);
		error = TRUE;
	    }
	}
	Cmd++;
    }
    if (error)				/* some commands were missing..	 */
	ErrJmp ();
}

/************************************************************************
 * READ THE NEXT SYMBOL FROM THE INPUT BUFFER
 *
 * - Skip over leading whitespace and comments
 * - read token: Word tokens may be keywords and are searched in the
 *   table of valid keywords, other tokens are evaluated
 *
 * Parameter :	keytab	= table of valid keywords
 * Result    :	the next token is placed in the global token variables
 *
 ***********************************************************************/

static void
NextSym (KeySymbol * keytab)
{
    char           *t;
    int             c;
    int             toksize;

again:
    t = Token;

    do {
	c = ReadChar ();		/* skip whitespace		 */
	if (c == '\n') {
	    LineNum++;			/* count lines			 */
	    LStart = Ipos;
	}
    }
    while (isspace (c));

    toksize = 0;
    TStart = Ipos - 1;

    if (isalpha (c)) {			/* start of a Word token	 */
	KeySymbol      *k;
	int             i;

	do {
	    *t++ = c;			/* read it into Token buffer	 */
	    c = ReadChar ();
	}
	while (isalpha (c) || c == '_');

	UnreadChar (c);			/* unread non_word character	 */

	*t = 0;
    /* scan for keyword		 */
	for (k = keytab, i = 0; k != NULL && k->Name != 0; k++, i++) {
	    if (lstrcmp (Token, k->Name) == 0) {	/* keyword found: set
							 * S_Keyword	 */
		Symbol = S_Keyword;
		TokVal.i = k->Type;
		TokSym = k;
		TokIndex = i;

#if	DEBUG
		IOdebug ("Keyword %s", Token);
#endif

		return;
	    }
	}
    /* not a keyword: set S_String	 */
	Symbol = S_String;
	TokVal.s = Token;
	TokSym = NULL;
	TokIndex = -1;

#if	DEBUG
	IOdebug ("Word %s", Token);
#endif

	return;
    }
    if (isdigit (c) || c == '-') {	/* a number:			 */
	TokVal.i = ctoi (c);		/* set S_Number and value	 */
	Symbol = S_Number;

#if	DEBUG
	IOdebug ("number %d", TokVal.i);
#endif

	return;
    }
    switch (c) {			/* other character:		 */
    case EOF:				/* check for symbol characters	 */
	Symbol = S_EOF;
	break;
    case '{':
	Symbol = S_LBracket;
	break;
    case '}':
	Symbol = S_RBracket;
	break;
    case '"':				/* a string: read it		 */
	c = ReadChar ();
	while (c != '"' && c != '\n') {
	    *t++ = c;
	    c = ReadChar ();
	}
	if (c != '"')
	    Error (5);
	Symbol = S_String;		/* set S_String			 */
	*t = 0;
	TokVal.s = Token;
	TokSym = NULL;
	TokIndex = -1;

#if	DEBUG
	IOdebug ("String %s", Token);
#endif

	return;
    case '#':				/* comment: rea to end of line	 */
	while (c != '\n')
	    c = ReadChar ();
    case '\n':				/* end of line: count it	 */
	LineNum++;
	LStart = Ipos;
    case '\r':				/* CR: try again		 */
	goto again;
    default:				/* other character: Error !	 */
	Error (6, c);
    }

#if	DEBUG
    IOdebug ("Symbol %c", c);
#endif

    return;
}

/************************************************************************
 * READ THE NEXT SYMBOL AND CHECK WHETHER IT IS THE EXPECTED ONE
 *
 * - It is an error if the specified symbol does not appear.
 *
 * Parameter :	sym	= expected symbol
 *
 ***********************************************************************/

void
CheckSym (int sym)
{
    NextSym (NULL);
    if (Symbol != sym) {
	switch (sym) {
	case S_LBracket:
	    Error (7, "{");
	case S_RBracket:
	    Error (7, "}");
	case S_Keyword:
	    Error (8, "Keyword");
	case S_Number:
	    Error (8, "Numeric value");
	case S_String:
	    Error (8, "String");
	}
    }
}

/************************************************************************
 * PARSE A KEYWORD'S ARGUMENT (for STRING, WORD and NUMBER only)
 *
 * - Read the next symbol, which has to match the keyword's arg type
 * - Place the argument value into the output buffer:
 *   - Strings are added to the end of the buffer with an RPTR in the
 *     structure
 *   - Numeric values are inserted into the structure
 *
 * Parameter :	StrOff	= Structure start offset in the output buffer
 *
 ***********************************************************************/

void
ParseArg (int StrOff)
{
    int             ArgType = TokSym->ArgType;
    word           *OutPtr = &Output[StrOff + TokSym->Offset / 4];

    CheckSym (ArgType);			/* get argument with type check	 */
/* check for expected type	 */
    switch (Symbol) {
    case S_Number:			/* numeric argument:		 */
	*OutPtr = TokVal.i;		/* place argument value		 */
	break;

    case S_String:			/* string argument:		 */
    /* add string to bufend		 */
	*OutPtr = (word) & Output[NewString (TokVal.s)];
	*OutPtr = ATOR (*OutPtr);	/* place RPTR to string		 */
	break;

    default:				/* other type: internal error	 */
	Error (9, 1);
    }
}

/************************************************************************
 * PARSE A LIST ARGUMENT
 *
 * - Read the next symbol, which has to be a left bracket
 * - Read subsequent symbols, which have to be byte-valued numbers,
 *   and add them to the end of the output buffer.
 * - Fill missing bytes with zeroes.
 * - Put a RPTR into the structure.
 *
 * Parameter :	StrOff	= Structure start offset in the output buffer
 *		Size	= number of bytes expected
 *
 ***********************************************************************/

void
ParseList (int StrOff, word Size)
{
    byte           *ValPtr = (byte *) & Output[Extend (Size)];
    word           *OutPtr = &Output[StrOff + TokSym->Offset / 4];
    word            val;
    word            count = 0;

    CheckSym (S_LBracket);

    *OutPtr = (word) ValPtr;		/* place Value ptr into struct	 */
    *OutPtr = ATOR (*OutPtr);		/* place RPTR to values		 */
    forever
    {
	NextSym (NULL);			/* get next symbol		 */

	if (Symbol == S_RBracket) {	/* right bracket ends list	 */
	    while (count++ < Size)	/* fill up with zeroes		 */
		*ValPtr++ = 0;
	    return;			/* everything fine.		 */
	}
	if (Symbol != S_Number)		/* check for numeric argument	 */
	    Error (22);

    /* check argument range		 */
	if (((val = TokVal.i) & 0xFFFFFF00) != 0)
	    Error (11, val);

	if (count++ >= Size)		/* check number of arguments	 */
	    Error (12);

	*ValPtr++ = (byte) val;		/* store byte value		 */
    }
}

/************************************************************************
 * PARSE A SUBSTRUCTURE
 *
 * - Obtain the index of the substructure in the parent structure,
 *   parse the substructure and update the substructure's Next entry.
 *
 * Parameter :	StrOff	= Struct start offset in the output buffer
 *		ParseFn	= Function to parse SubStruct
 *
 ***********************************************************************/

void
ParseStruct (int StrOff, word (*ParseFn) (void))
{
    int             last = StrOff + TokSym->Offset / 4;
    int             x;

    x = ParseFn ();

    while (Output[last] != -1)		/* skip over full substructs	 */
	last = last + Output[last] / 4;

    Output[last] = (x - last) * 4;	/* save Next RPTR		 */
}

/************************************************************************
 * PARSE A CONDITION STRUCTURE
 *
 * Result    :	offset of the new InfoNode in the output buffer
 *
 ***********************************************************************/

word
ParseCondition (void)
{
    word            StrOff = Extend (sizeof (CondInfo));
    CmdNode        *CNode = FindCommand ("Request_Sense");
    CmdInfo        *CInfo;
    word            index;
    word            mask;
    word            defined = 0;

/* search for Request Sense	 */
    if (CNode == NULL)
	Error (3, "Request_Sense");

    CInfo = (CmdInfo *) & Output[CNode->Offset];

    CheckSym (S_LBracket);		/* first Symbol is left bracket	 */

/* set up default values	 */
    *(CondInfo *) (Output + StrOff) = Default.CondInfo;

    forever				/* loop for keywords		 */
    {
	NextSym (ConditionTab);		/* get next symbol		 */

	if (Symbol == S_RBracket)	/* right bracket ends struct	 */
	    break;

	if (Symbol == S_Keyword) {	/* keyword found :		 */
	    mask = 1 << (index = TokIndex);
	    if (defined & mask)		/* check duplicate specs	 */
		Error (13, ConditionTab[index].Name);

	    defined |= mask;		/* mark specification		 */

	    if (TokVal.i == T_Param)
		ParseArg (StrOff);
	    else			/* we should never get here	 */
		Error (9, 2);

	    if (index == 0 && TokVal.i >= CInfo->DataSize)
		Error (14);

	} else				/* not a keyword found: Error !	 */
	    Error (10);
    }

    if (defined != 7) {			/* check completeness		 */
	unless (defined & 1)
	  ErrMsg (15, "offset");
	unless (defined & 2)
	  ErrMsg (15, "mask");
	unless (defined & 4)
	  ErrMsg (15, "value");
	ErrJmp ();
    }
    return StrOff;			/* return Offset of struct	 */
}

/************************************************************************
 * PARSE AN ERROR STRUCTURE
 *
 * Result    :	offset of the new InfoNode in the output buffer
 *
 ***********************************************************************/

word
ParseError (void)
{
    word            StrOff = Extend (sizeof (ErrorInfo));
    word            defined = 0;

    CheckSym (S_LBracket);		/* first Symbol is left bracket	 */

/* set up default values	 */
    *(ErrorInfo *) (Output + StrOff) = Default.ErrorInfo;

    forever				/* loop for keywords		 */
    {
	NextSym (ErrorTab);		/* get next symbol		 */

	if (Symbol == S_RBracket)	/* right bracket ends struct	 */
	    break;

	if (Symbol == S_Keyword) {	/* keyword found :		 */
	    switch (TokVal.i) {
	    case T_ErrCode:		/* parse code Parameter		 */
		if (defined & 1)	/* check duplicate code		 */
		    Error (13, "code");
		defined |= 1;
		ParseArg (StrOff);
		break;

	    case T_Condition:		/* parse Condition substruct	 */
		defined |= 2;
		ParseStruct (StrOff, ParseCondition);
		break;

	    default:			/* we should never get here	 */
		Error (9, 3);
	    }
	} else				/* not a keyword found: Error !	 */
	    Error (10);
    }

    if (defined != 3) {			/* check completeness		 */
	unless (defined & 1)
	  ErrMsg (15, "code");
	unless (defined & 2)
	  ErrMsg (15, "condition");
	ErrJmp ();
    }
    return StrOff;			/* return Offset of struct	 */
}

/************************************************************************
 * PARSE A COMMAND STRUCTURE
 *
 * Result    :	offset of the new InfoNode in the output buffer
 *
 ***********************************************************************/

word
ParseCommand (void)
{
    word            StrOff = Extend (sizeof (CmdInfo));
    word            defined = 0;
    word            group;
    word            index;
    word            mask;
    word            dir;
    word            csize = -1;
    word            dsize = 0;
    word            TempOff = 0;

    CheckSym (S_LBracket);		/* first Symbol is left bracket	 */

/* set up default values	 */
    *(CmdInfo *) (Output + StrOff) = Default.CmdInfo;

    forever				/* loop for keywords		 */
    {
	NextSym (CommandTab);		/* get next symbol		 */

	if (Symbol == S_RBracket)	/* right bracket ends struct	 */
	    break;

	if (Symbol == S_Keyword) {	/* keyword found :		 */
	    mask = 1 << (index = TokIndex);
	    if (defined & mask)		/* check duplicate specs	 */
		Error (13, ConditionTab[index].Name);

	    defined |= mask;		/* mark specification		 */

	    switch (TokVal.i) {
	    case T_Name:		/* Command name			 */
		ParseArg (StrOff);
		AddCommand (TokVal.s, StrOff);	/* add to list	 */
		break;

	    case T_Direction:
	    /* save keyword offset		 */
		TempOff = StrOff + TokSym->Offset / 4;

		NextSym (YesNoTab);	/* get the value	 */
		if (Symbol != S_Keyword)/* no keyword: Error	 */
		    Error (16, "Read", "yes", "no");
	    /* store value		 */
		Output[TempOff] = dir = TokVal.i;
		break;

	    case T_Blockmove:
	    /* save keyword offset		 */
		TempOff = StrOff + TokSym->Offset / 4;

		NextSym (YesNoTab);	/* get the value	 */
		if (Symbol != S_Keyword)/* no keyword: Error	 */
		    Error (16, "Blockmove", "yes", "no");
	    /* store value		 */
		Output[TempOff] = TokVal.i;
		break;

	    case T_CDBSize:		/* CDB size for vendor cmds	 */
		ParseArg (StrOff);
		if ((csize = TokVal.i) < 0 || csize > 12)
		    Error (23);		/* range check failed		 */
		break;

	    case T_CDB:		/* CDB contents			 */
		TempOff = StrOff + TokSym->Offset / 4;
		ParseList (StrOff, 12);
	    /* check for cmd group		 */
		group = ((*(byte *) RTOA (Output[TempOff])) >> 5) & 7;
		TempOff = StrOff + offsetof (CmdInfo, CDBSize) / 4;

		switch (group) {	/* check groups and cdb sizes	 */
		case 0:
		    if (csize >= 0 && csize != 6)
			Error (24, group);
		    Output[TempOff] = 6;
		    break;
		case 1:
		case 2:
		    if (csize >= 0 && csize != 10)
			Error (24, group);
		    Output[TempOff] = 10;
		    break;
		case 6:
		    if (csize >= 0 && csize != 12)
			Error (24, group);
		    Output[TempOff] = 12;
		    break;
		default:
		    if (csize < 0)
			Error (25);
		    break;
		}
		break;

	    case T_DataSize:		/* darasize for default sizes	 */
		ParseArg (StrOff);
		dsize = TokVal.i;
		break;

	    case T_Data:		/* default data			 */
	    /* check for default w/o size	 */
		if (!(defined & 0x20))
		    Error (15, "datasize");

		ParseList (StrOff, dsize);
		break;

	    default:			/* we should never get here	 */
		Error (9, 4);
	    }
	} else				/* not a keyword found: Error !	 */
	    Error (10);
    }
    if ((defined & 0x13) != 0x13) {	/* check for missing keywords	 */
	unless (defined & 0x01)
	  ErrMsg (15, "name");
	unless (defined & 0x02)
	  ErrMsg (15, "direction");
	unless (defined & 0x10)
	  ErrMsg (15, "cdb");
	ErrJmp ();
    }
/* check for missing defdata	 */
    if (!dir && (defined & 0x60) == 0x20 && dsize > 0)
	Error (15, "default");

    return StrOff;			/* return Offset of struct	 */
}

/************************************************************************
 * PARSE A REQUEST STRUCTURE
 *
 * Result    :	offset of the new InfoNode in the output buffer
 *
 ***********************************************************************/

word
ParseRequest (void)
{
    word            StrOff = Extend (sizeof (ReqInfo));
    word            defined = 0;

    CheckSym (S_LBracket);		/* first Symbol is left bracket	 */

/* set up default values	 */
    *(ReqInfo *) (Output + StrOff) = Default.ReqInfo;

    forever				/* loop for keywords		 */
    {
	NextSym (RequestTab);		/* get next symbol		 */

	if (Symbol == S_RBracket)	/* right bracket ends struct	 */
	    break;

	if (Symbol != S_Keyword)	/* not a keyword found: Error !	 */
	    Error (10);

	switch (TokVal.i) {
	case T_FnCode:
	    if (defined & 1)		/* check for duplicate spec	 */
		Error (13, "fncode");
	    defined |= 1;

	    ParseArg (StrOff);		/* parse fncode			 */
	    break;

	case T_Item:
	    {
		word            TempOff = Extend (sizeof (ItemInfo));
		int             LastOff = StrOff + TokSym->Offset / 4;
		ItemInfo       *Info = (ItemInfo *) & Output[TempOff];
		CmdNode        *CNode;

		unless (defined)	/* check missing fncode		 */
		  Error (15, "fncode");

		CheckSym (S_String);	/* get Command name		 */

	    /* search the command		 */
		if ((CNode = FindCommand (TokVal.s)) == NULL)
		    Error (17, TokVal.s);

		Info->Next = -1;	/* initialise as last node	 */
	    /* insert RPTR to related cmd	 */
		Info->Item = (CNode->Offset - TempOff) * 4
		  - offsetof (ItemInfo, Item);

	    /* complete Item chain		 */
		while (Output[LastOff] != -1)
		    LastOff = LastOff + Output[LastOff] / 4;

		Output[LastOff] = (TempOff - LastOff) * 4;
		break;
	    }
	default:			/* we should never get here	 */
	    Error (9, 5);
	}
    }

    unless (defined)			/* check missing fncode		 */
      Error (15, "fncode");

    return StrOff;			/* return Offset of struct	 */
}

/************************************************************************
 * PARSE A SCSI DEVICE STRUCTURE
 *
 * Result    :	offset of the new InfoNode in the output buffer
 *
 ***********************************************************************/

word
ParseScsiDev (void)
{
    word            StrOff = Extend (sizeof (ScsiDevInfo));
    word            DevType;
    word            defined = 0;
    word            TempOff = 0;

    CheckSym (S_LBracket);		/* first Symbol is left bracket	 */

/* set up default values	 */
    *(ScsiDevInfo *) (Output + StrOff) = Default.ScsiDevInfo;

    forever				/* loop for keywords		 */
    {
	NextSym (ScsiDevTab);		/* get next symbol		 */

	if (Symbol == S_RBracket)	/* right bracket ends struct	 */
	    break;

	if (Symbol == S_Keyword) {	/* keyword found :		 */
	    switch (TokVal.i) {
	    case T_Type:
		if (defined & 1)	/* check kw sequence	 */
		    Error (13, "type");
		defined |= 1;
	    /* save keyword offset		 */
		TempOff = StrOff + TokSym->Offset / 4;

		NextSym (RawStrTab);	/* get the value	 */
		if (Symbol != S_Keyword)/* no keyword: Error	 */
		    Error (16, "Type", "raw / sequential", "structured / random");
	    /* store value		 */
		Output[TempOff] = DevType = TokVal.i;
		break;

#if 0       /* OI 27 Jan 92 */

	    case T_Sync:
		if (defined & 4)	/* check kw sequence	 */
		    Error (13, "synchronous");
		defined |= 4;
	    /* save keyword offset		 */
		TempOff = StrOff + TokSym->Offset / 4;

		NextSym (YesNoTab);	/* get the value	 */
		if (Symbol != S_Keyword)/* no keyword: Error	 */
		    Error (16, "synchronous", "yes", "no");
	    /* store value		 */
		Output[TempOff] = TokVal.i;
		break;
#endif

	    case T_Ident:
		unless (defined & 1)	/* check kw sequence	 */
		  Error (15, "type");
		if (defined & 8)
		    Error (13, "ident");
		defined |= 8;

		ParseArg (StrOff);
		break;

	    case T_Command:
		unless (defined & 1)	/* check kw sequence	 */
		  Error (15, "type");
	    /* parse substructure	 */
		ParseStruct (StrOff, ParseCommand);
		break;

	    case T_Error:
		unless (defined & 1)	/* check kw sequence	 */
		  Error (15, "type");

	    /* parse substructure	 */
		ParseStruct (StrOff, ParseError);
		break;

	    case T_Request:
		unless (defined & 1)	/* check kw sequence	 */
		  Error (15, "type");

	    /* parse substructure	 */
		ParseStruct (StrOff, ParseRequest);
		break;

	    default:			/* we should never get here	 */
		Error (9, 6);
	    }
	} else				/* not a keyword found: Error !	 */
	    Error (10);
    }
    CheckCommands (DevType);

    FreeList (&CmdList);		/* Free the command list.	 */

    return StrOff;			/* return Offset of struct	 */
}

/************************************************************************
 * PARSE A FILE ENTRY
 *
 * - Get the next symbol, which has to be a TopTab keyword.
 *   Parse the keyword's substructure and fill up the InfoNode for
 *   this entry.
 *
 * Result    :	offset of the new InfoNode in the output buffer
 *
 ***********************************************************************/

word
ParseInfo (void)
{
    InfoNode       *i;
    int             StrOff;
    word            info;
    word            type;

    NextSym (TopTab);			/* read next token		 */

    if (Symbol == S_EOF)		/* EOF : done			 */
	return EOF;

    if (Symbol != S_Keyword)		/* not a keyword: Error !	 */
	Error (8, "Keyword");

/* obtain space for an InfoNode	 */
    StrOff = Extend (sizeof (InfoNode));
    type = TokVal.i;			/* save InfoNode type		 */

    ParseArg (StrOff);			/* parse entry name		 */

    if (type == T_ScsiDev) {		/* right type: parse entry	 */
	printf ("parsing device %d : '%s'\n", DevCount++, TokVal.s);
	info = ParseScsiDev ();
    } else
	Error (9, 7);

/* complete InfoNode		 */
    i = ((InfoNode *) & Output[StrOff]);

    i->Type = type;			/* with Type and Info		 */
    i->Info = (word) & Output[info];
    i->Info = ATOR (i->Info);

    return StrOff;
}

/************************************************************************
 * PARSE AN INPUT FILE INTO MEMORY
 *
 * - Open and read the input file, then parse the entries contained in it.
 *
 * Parameter :	file	= input file name
 *
 ***********************************************************************/

void
ParseScsiInfo (char *file)
{
    Stream         *s = NULL;
    word            info = NULL;
    word            last = -1;

/* Open the input file		 */
    if ((s = Open (CurrentDir, file, O_ReadOnly)) == NULL)
	Error (18, file);

    Isize = GetFileSize (s);		/* allocate a buffer		 */

    if ((Input = Malloc (Isize + 1)) == NULL)
	Error (19);
/* and read the whole file.	 */
    if (Read (s, Input, Isize, -1) != Isize)
	Error (20, file);

/* Parse single entries		 */
    while ((info = ParseInfo ()) != EOF) {
	if (last != -1) {		/* and chain the InfoNodes.	 */
	    Output[last] = (word) & Output[info];
	    Output[last] = ATOR (Output[last]);
	}
	last = info;
    }
}

/************************************************************************
 * WRITE THE PARSED DATA TO A FILE
 *
 * - Open the output file, generate an image header
 *   and a module struct for the ScsiInfo structure and
 *   write everything out to file.
 *
 * Parameter :	file	= output file name
 *
 ***********************************************************************/

void
WriteScsiInfo (char *file)
{
    Stream         *s = NULL;
    ImageHdr        hdr;
    Module          module;
    int             size = Osize * sizeof (word);
    int             zero = 0;

    if ((s = Open (CurrentDir, file, O_WriteOnly | O_Create)) == NULL)
	Error (21, file);

    hdr.Magic = Image_Magic;		/* Prepare the Image Header	 */
    hdr.Flags = 0;
    hdr.Size = size + sizeof (Module) + sizeof (zero);

/* and write it to the file.	 */
    Write (s, (byte *) & hdr, sizeof (hdr), -1);

    module.Type = T_DevInfo;		/* Prepare the Module Structure	 */
    module.Size = sizeof (Module) + size;
    strcpy (module.Name, "ScsiInfo");
    module.Id = -1;
    module.Version = 1000;
    module.MaxData = 0;
    module.Init = 0;
/* and write it to the file.	 */
    Write (s, (byte *) & module, sizeof (Module), -1);
/* write parsed data		 */
    Write (s, (byte *) Output, size, -1);
/* and a terminating zero.	 */
    Write (s, (byte *) & zero, sizeof (zero), -1);

    Close (s);				/* That's all, folks !		 */
}

/************************************************************************
 * MAIN PROGRAM
 *
 * - Check the number of arguments, then initialise the Error_Jmp
 *   and parse the input file. Write the output to the output file.
 *
 * Parameter :	argc	= Number of arguments
 *		argv	= argument vector
 * Result    :	0	if everything was OK
 *
 ***********************************************************************/

int
main (int argc, char **argv)
{
    word            e;

    InitList (&CmdList);

    if (argc != 3) {
	printf ("usage: %s source dest\n", argv[0]);
	exit (1);
    }
    if ((e = setjmp (Error_Jmp)) == 0) {
	ParseScsiInfo (argv[1]);
	WriteScsiInfo (argv[2]);
    }
    if (Input != NULL)
	Free (Input);
}
