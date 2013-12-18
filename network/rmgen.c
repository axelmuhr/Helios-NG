/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- rmgen.c								--
--                                                                      --
--	The resource map parser						--
--									--
-- Author :								--
--		************		********			--
--		************ 		**********			--
--		     **			**	***			--
--		     **			**	 **			--
--		     ** AMES  		**	***  OWDELLS		--
--		*******			**********			--
--		*******			********			--
--									--
-- with some help from BLV						--
-- (well, quite a bit of help actually)					--
-- (in fact a near complete rewrite by BLV, as usual)			--
--                                                                      --
------------------------------------------------------------------------*/
/* $Header: /hsrc/network/RCS/rmgen.c,v 1.15 1994/03/10 17:13:33 nickc Exp $ */

static char *version_number = "2.07";
/**
*** Revision history:
***    2.01, first numbered version.
***    2.02, added C40, waitfor, console
***    2.03, fixed handling of reserved words
***    2.04, added idrom support for C40
***    2.05, bug fixex for idrom handling
***    2.06, changed default IDROM values for gbcr and lbcr
***    2.07, now compiles on Unix systems
**/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <helios.h>
#include <stdarg.h>
#include <config.h>
#include <c40.h>
#include "private.h"
#include "rmlib.h"
#include "netutils.h"

#ifdef Malloc
#undef Malloc
#endif

/**
*** As the parser goes up and down the subnet tree, it is necessary to
*** keep track of the current hierarchy. For example, a reset driver is
*** defined at the top level. Then a subnet is read in with its own
*** reset driver. When the subnet has been read the higher level reset
*** driver must be restored.
**/
typedef	struct	StackNode	{
	Node			Node;
	RmNetwork		Network;
	RmHardwareFacility	*ResetDriver;
	RmHardwareFacility	*ConfigureDriver;
	bool			NewResetDriver;
	bool			NewConfigureDriver;
	char			*ResetMnode;
	char			*ConfigureMnode;
	int			Level;
} StackNode;

/**
*** These structures are used to keep track of the processor links
**/
typedef struct ProcessorLinks {
	List		List;
	int		Count;
} ProcessorLinks;

	/* avoid clash with servlib.h */
#define LinkNode	my_LinkNode
typedef struct LinkNode {
	Node		Node;
	int		Linkno;
	RmProcessor	Target;
	int		Destlink;
	char		*Name;
} LinkNode;

/**
*** This structure is used to keep track of the reset commands
**/
typedef	struct	ResetCommand {
	Node			Node;
	RmHardwareFacility	*hardware;
	List			Processors;	
} ResetCommand;

/**
*** static variables
**/
static	List		Stack;
static	StackNode	*Current;
static	bool		LexicalAnalysis;
static	char		InputFile[IOCDataMax];
static	char		OutputFile[IOCDataMax];
static	int		ErrorCount = 0;
static	RmNetwork	Network;
static	RmTaskforce	Taskforce;

/**
*** forward declarations
**/
static	void		OpenInputFile(void);
static	void		usage(void);
static	void		MemoryError(int);
static	void		Error(char *message, ...);
static	void		InitialiseParser(void);
static	void		ReadNetwork(void);
static	void		ValidateName(char *);
static	void		ResolveProblems(RmNetwork);
static	char		*CopyString(char *);
static	char		*BuildName(char *);
static	int		ReadNumber(void);
static	void		Recover(int x, ...);
static	void		ReadIDRom(RmProcessor);
static	void		ReadBootfile(RmProcessor);

/**
*** Main().
*** 1) Check the arguments. rmgen takes two forms of arguments
***    a) rmgen default		compiles default.rm to default.map
***    b) rmgen -o x.map y.rm	compiles the specified file
*** 2) Open the resource map file.
*** 3) Initialise the stack used to maintain the details as the parser
***    moves up and down the parse tree.
*** 4) Initialise and call the  parser
*** 5) If succesful, produce the output file.
**/

static	void usage(void);

int main(int argc, char **argv)
{  
  printf("Resource Map Generator version %s\n", version_number);
  
  LexicalAnalysis = FALSE;	/* see Error() routine */
  
  if ((argc <= 1) || (argc > 5)) usage();
  if (argc eq 2)
   {
     char *	ptr;
     
     if ((ptr = strrchr( argv[1], '.' )) != NULL)
       {
	 if (ptr[1] == 'm')
	   {
	     strcpy(OutputFile, argv[1]);
	     *ptr = '\0';
	     strcpy(InputFile, argv[1]);
	     strcat(InputFile, ".rm");
	   }
	 else
	   {
	     strcpy(InputFile, argv[1]);
	     *ptr = '\0';
	     strcpy(OutputFile, argv[1]);
	     strcat(OutputFile, ".map");
	   }
       }
     else
       {
	 strcpy(InputFile, argv[1]);
	 strcat(InputFile, ".rm");
	 strcpy(OutputFile, argv[1]);
	 strcat(OutputFile, ".map");
       }
   }
  else
   { char *arg = argv[1];
     if ((arg[0] eq '-') && 
         ((arg[1] eq 'o') || (arg[1] eq 'O')))
      { if (arg[2] eq '\0')
         { 
           if (argc ne 4) usage();
           strcpy(OutputFile, argv[2]);
           strcpy(InputFile,  argv[3]);
         }
        else
         { 
           if (argc ne 3) usage();
           strcpy(OutputFile,  &(arg[2]));
           strcpy(InputFile, argv[2]);
         }
      }
     else
      { 
        strcpy(InputFile, argv[1]);
        arg = argv[2];
        unless ((arg[0] eq '-') && ((arg[1] eq 'o') || (arg[1] eq 'O')))
         usage();
        if (arg[2] eq '\0')
         { if (argc ne 4) usage();
           strcpy(OutputFile, argv[3]);
         }
        else
         { if (argc ne 3) usage();
           strcpy(OutputFile, &(arg[2]));
         }
      }
   }

  OpenInputFile();

  InitList(&Stack);
  if ((Current = New(StackNode)) eq Null(StackNode)) MemoryError(1);
  if ((Network = RmNewNetwork()) eq (RmNetwork)NULL) MemoryError(2);
  { List	*list = New(List);
    if (list eq Null(List)) MemoryError(35);
    RmSetNetworkPrivate(Network, (int) list);
    InitList(list);
  }
  if ((Taskforce = RmNewTaskforce()) eq (RmTaskforce) NULL) MemoryError(3);
  Current->Network		= Network;
  Current->ResetDriver		= Null(RmHardwareFacility);
  Current->ConfigureDriver	= Null(RmHardwareFacility);
  Current->NewResetDriver	= FALSE;
  Current->NewConfigureDriver	= FALSE;
  Current->ResetMnode		= Null(char);
  Current->ConfigureMnode	= Null(char);
  Current->Level		= 0;
#ifdef SYSDEB
  Current->Node.Next = Current->Node.Prev = &Current->Node;
#endif
  AddTail(&Stack, &(Current->Node));

  LexicalAnalysis = TRUE;
  InitialiseParser();
  ReadNetwork();
  LexicalAnalysis = FALSE;

  if (ErrorCount eq 0)
   ResolveProblems(Network);

  if (ErrorCount eq 0)
   { 
#if 0
     PrintNetwork(Network);
#endif
#if 0
     PrintTaskforce(Taskforce);
#endif

     if (RmWrite(OutputFile, Network, Taskforce) ne RmE_Success)
      { fputs("rmgen: failed to write output file.\n", stderr);
        return(EXIT_FAILURE);
      }
     else
      return(EXIT_SUCCESS);
   }
  else
   { fputs("rmgen: compilation failed.\n", stderr);
     return(EXIT_FAILURE);
   }
}

static	void	usage(void)
{ fputs("rmgen: usage,        rmgen <basename>\n", stderr);
  fputs("       (for example) rmgen default\n", stderr);
  fputs("       or :          rmgen -o<output> <input>\n", stderr);
  fputs("       (for example) rmgen -o default.map default.rm\n", stderr);
  exit(EXIT_FAILURE);
}

static	void MemoryError(int x)
{ fprintf(stderr, "rmgen: fatal error, out of memory(%d).\n", x);
  exit(EXIT_FAILURE);
}

/**
*** Attempt to open the input file. This parser works in terms of C file
*** I/O, so a FILE * structure is required.
**/
static	FILE *SourceFile = Null(FILE);

static void	OpenInputFile(void)
{ 
   SourceFile = fopen(InputFile, "r");
   if (SourceFile eq Null(FILE))
    { fprintf(stderr, "rmgen: failed to open %s\n", InputFile);
      exit(EXIT_FAILURE);
    }
}

/**----------------------------------------------------------------------------
*** The bulk of the parser. This is mostly a copy of the parser in the
*** CDL compiler.
***
*** nexttoken() takes a stream of bytes, obtained by nextch(), and turns
*** it into a stream of tokens.
*** 
*** nextch() extracts data from the file into a buffer
***
*** findkeyword() attempts to match some input string with one of the known
*** tokens.
**/

typedef enum	/* These are the things the parser must know about	*/
{
  T_NULL,		T_EOF,		T_WORD,
  T_SEMICOLON,		T_LBRACE,	T_RBRACE,
  T_NETWORK,		T_RESET,  	T_CONFIGURE,
  T_CONTROL,  		T_MNODE,	T_RSTANL,
  T_PROCESSOR,    	T_RUN,		T_DRIVER,
  T_HELIOS,		T_SYSTEM,	T_NATIVE,
  T_IO,			T_PTYPE,	T_MEMORY,
  T_NUCLEUS,		T_ATTRIB,	T_ARGV,
  T_COMMA,		T_HEX,		T_COMMENT,
  T_EXTERNAL,		T_800,		T_414,
  T_425,		T_400,		T_212,
  T_9000,		T_i860,		T_ARM,
  T_680x0,		T_USER,		T_ROUTER,
  T_WAITFOR,		T_CONSOLE,	T_C40,
  T_IDROM,		T_BOOTFILE
} TOKEN;

typedef struct {
	char	*name;
	TOKEN	token;
} KEYWORD;

static	KEYWORD keywords[] =
{
  { "network", 	T_NETWORK 	},	/* this table maps strings onto	*/
  { "subnet", 	T_NETWORK 	},	/* tokens. N.B. the strings must*/
  { "reset",	T_RESET 	},	/* be lowercase and the table is*/
  { "configure",T_CONFIGURE 	},	/* terminated with an empty	*/
  { "processor",T_PROCESSOR 	},	/* string.			*/
  { "terminal", T_PROCESSOR 	},
  { "run",	T_RUN	    	},
  { "driver",	T_DRIVER	},
  { "helios",	T_HELIOS	},
  { "system",	T_SYSTEM	},
  { "native",	T_NATIVE	},
  { "io",	T_IO		},
  { "ptype",	T_PTYPE		},
  { "memory",	T_MEMORY	},
  { "nucleus",	T_NUCLEUS	},
  { "attrib",	T_ATTRIB	},
  { "argv",	T_ARGV		},
  { "control",	T_CONTROL	},
  { "mnode",	T_MNODE		},
  { "rst_anl",	T_RSTANL	},
  { "--",	T_COMMENT	},
  { "ext",	T_EXTERNAL	},
  { "t800",	T_800		},
  { "t801",	T_800		},
  { "t805",	T_800		},
  { "t414",	T_414		},
  { "t425",	T_425		},
  { "t400",	T_400		},
  { "t222",	T_212		},
  { "t212",	T_212		},
  { "h1",	T_9000		},
  { "t9000",	T_9000		},
  { "i860",	T_i860		},
  { "arm",	T_ARM		},
  { "68000",	T_680x0		},
  { "68020",	T_680x0		},
  { "68030",	T_680x0		},
  { "680x0",	T_680x0		},
  { "user",	T_USER		},
  { "router",	T_ROUTER	},
  { "waitfor",	T_WAITFOR	},
  { "console",	T_CONSOLE	},
  { "c40",	T_C40		},
  { "idrom",	T_IDROM		},
  { "bootfile", T_BOOTFILE	},
  { "",		T_EOF		}
 };

static	int	ch = ' '; /* the current character being processed*/
static	TOKEN	token;	  /* and the current token.			*/
static	int	lineindex = -1;		/* index in current buffer	*/
static	char	linebuffer[256];	/* and the buffer.		*/
static	char	tokenbuffer[256];	/* and another buffer.		*/
static	int	tokenindex = -1;
static	int	linenumber = 1;
static	int	previousindex = -1;
static	void	nextch(void);
static	bool	ismetach(int);
static	KEYWORD	*findkeyword(char *);

static	void	nexttoken(void)
{ int index = 0;
  KEYWORD *keyword;

  previousindex = lineindex;
  forever
  {
    tokenindex = lineindex;
    switch (ch)
    {
      case EOF	: if (token eq T_EOF)	/* repeated end-of-file */
                   { fputs("Unexpected end of file, aborting.\n", stderr);
                     exit(EXIT_FAILURE);
                   }
                  token = T_EOF; 
                  return;

      case '\n'	: linenumber++;
      case '\r' :
      case ' '	:
      case '\t'	: nextch(); continue;

      case '#'	: if (lineindex eq 0)	/* comment at start of line */
         	   { for ( ; (ch ne '\n') && (ch ne EOF); nextch() );
		     continue;
		   }
		  else
		   { token = T_HEX;
		     nextch();
		     return;
		   }
		   
      case ';'	: nextch();
		  token = T_SEMICOLON;
		  return;

      case ','	: nextch();
		  token = T_COMMA;
		  return;

      case '['	:
      case '{'	: nextch();
		  token = T_LBRACE;
		  return;

      case ']'	:
      case '}'	: nextch();
		  token = T_RBRACE;
		  return;

      default	: until (isspace(ch) || ismetach(ch) || (ch == EOF))
      		   { if (ch == '\\') nextch();
		     tokenbuffer[index++] = ch;
		     nextch();
		   }

		  tokenbuffer[index] = '\0';
		  keyword = findkeyword(tokenbuffer);
		  if (keyword eq NULL) 
		   token = T_WORD;
		  elif (keyword->token eq T_COMMENT)
		   { 
		     for ( ; (ch ne '\n') && (ch ne EOF); nextch());
		     index = 0;
		     continue;
	 	   }
	 	  else
		   token = keyword->token;
		  return;
    }
  }
}

/**
*** Check whether the current token can be treated as a string.
**/
static bool	IsString(TOKEN token)
{ int	i;
  if (token eq T_WORD) return(TRUE);
  for (i = 0; keywords[i].name[0] ne '\0'; i++)
   if (keywords[i].token eq token)
    return(TRUE);
  return(FALSE);
}
/**
*** Get the next character from the buffer, reading in another line
*** from the input file if necessary.
**/

static void nextch(void)
{ 
  if (ch eq EOF) return;

  if (lineindex == -1 || (ch = linebuffer[++lineindex]) == '\0')
   { 
     if (fgets(linebuffer, 255, SourceFile) == NULL)
      { lineindex = -1;
        ch = EOF;
      }
     else
      { previousindex = lineindex = 0;
        ch = linebuffer[0];
      }
   }
  if (ch eq '\t') linebuffer[lineindex] = ' ';	/* for Error() to work */
}

static bool ismetach(int c)
{
  return c == ';' || c == '[' || c == ']' || c == '{' || c == '}' ||
         c == ',' || c == '\n';
}

static KEYWORD *findkeyword(char *name)
{ int i;
  static char buffer[256];

  for (i = 0; name[i] ne '\0'; i++)
   if (isupper(name[i]))
    buffer[i] = tolower(name[i]);
   else
    buffer[i] = name[i];
  buffer[i] = '\0';
  
  for (i = 0; (keywords[i].name)[0] ne '\0'; i++)
   { if (!strcmp(buffer, keywords[i].name))
      return(&(keywords[i]));
   }

  return(Null(KEYWORD));
}

static void Error(char *message, ...)
{ int i;
  va_list	args;
  
  va_start(args, message);

  if (LexicalAnalysis)  
   fprintf(stderr, "%s, line %d: ", InputFile, linenumber);
  else
   fprintf(stderr, "%s: ", InputFile);
  vfprintf(stderr, message, args);
  va_end(args);
  fputc('\n', stderr);

  if (LexicalAnalysis && (previousindex ne -1))
   { fputs(linebuffer, stderr);
     for (i = 0; i < previousindex; i++) fputc(' ', stderr);
     fputc('^', stderr); 
     for (i = previousindex; i < lineindex; i++) fputc(' ', stderr);
     fputc('^', stderr);
     fputc('\n', stderr);
   }
   
  if (++ErrorCount eq 10) 
   { fputs("rmgen: too many errors, aborting compilation.\n", stderr);
     exit(EXIT_FAILURE);
   }
}

/**
*** Initialise the parser. This means getting the first character, and
*** trying to turn it into a token.
**/
static	void	InitialiseParser(void)
{ nextch();
  nexttoken();
}

/**
*** Attempt to recover from an error. Continue reading tokens, but ignore
*** them until one of the specified ones is encountered or end-of-file is
*** reached.
**/
static	void	Recover(int count, ...)
{ TOKEN		table[8];
  int		i;
  va_list	args;

  va_start(args, count);
  for (i = 0; i < count; i++)
   table[i] = va_arg(args, TOKEN);
  va_end(args);
  
  forever
   { 
     if (token eq T_EOF) return;
     for (i = 0; i < count; i++)
      if (token eq table[i])
       return;
     nexttoken();
   }
}
  
/**
*** Validate a name. A name, either for a processor or a network, can be
*** up to 31 characters long. It must not contain a / or other funny
*** characters, although underscores are permitted.
**/
static	void	ValidateName(char *name)
{ 
  if (*name eq '\0')
   { Error("a name cannot be an empty string");
     return;
   }

  if (strlen(name) > 31)
   { Error("names are limited to 31 characters, %s is too long", name);
     return;
   }

  for ( ; *name ne '\0'; name++)
   { if ((*name ne '_') && (!isalnum(*name)))
      Error("illegal character %c in name", *name);
   }
}

/**
*** Make a copy of a string, by allocating a new bit of memory. This is
*** useful when reading in lists, e.g. as part of a network command or
*** in a processor list of links.
**/
static	char	*CopyString(char *str)
{ int	amount	= strlen(str) + 1;
  char	*buf	= (char *) Malloc(amount);
  if (buf eq Null(char)) MemoryError(4);
  strcpy(buf, str);
  return(buf);
}

/**
*** Given a processor name, build the full network name by walking back
*** up the hierarchy. The name is built into a static buffer, and then
*** copied into a new bit of memory.
**/
static	char	NetworkName[256];

static	char	*BuildNameAux(RmNetwork Network)
{ char		*ptr;
  const char	*id = RmGetNetworkId(Network);
  
  if (Network eq RmRootNetwork((RmProcessor) Network))
   { NetworkName[0] = '/';
     for (ptr = &(NetworkName[1]); *id ne '\0'; )
      *ptr++ = *id++;
     return(ptr);
   }
  else
   { ptr = BuildNameAux(RmParentNetwork((RmProcessor) Network));
     *ptr++ = '/';
     while (*id ne '\0')
      *ptr++ = *id++;
     return(ptr);
   }
}

static	char	*BuildName(char *name)
{ char *str = BuildNameAux(Current->Network);
  *str++ = '/';
  strcpy(str, name);
  return(CopyString(NetworkName));
}

/**
*** Read a number, which may be either hex, octal, or decimal
**/
static int	ReadNumber(void)
{ int	base = 10;
  char	*ptr	= tokenbuffer;
  int	result = 0;

  if (token eq T_HEX)	/* memory #100000 */
   { base = 16; nexttoken(); }
  
  unless(IsString(token))
   { Error("number expected"); return(-1); }

  if ((base eq 10) && (*ptr eq '0'))
   { ptr++;
     if ((*ptr eq 'x') || (*ptr eq 'X'))
      { base = 16; ptr++; }
     else
      base = 8;
   }

  while (*ptr ne '\0')
   { switch(base)
      { case 10 : if (('0' <= *ptr) && (*ptr <= '9'))
                   { result = (10 * result) + (*ptr - '0'); break; }
                  else
                   goto error;
        case 8	: if (('0' <= *ptr) && (*ptr <= '7'))
                   { result = (8 * result) + (*ptr - '0'); break; }
                  else
                   goto error;
        case 16 : if (('0' <= *ptr) && (*ptr <= '9'))
                   { result = (16 * result) + (*ptr - '0'); break; }
                  elif (('a' <= *ptr) && (*ptr <= 'f'))
                   { result = (16 * result) + (*ptr - 'a' + 10); break; }
                  elif (('A' <= *ptr) && (*ptr <= 'F'))
                   { result = (16 * result) + (*ptr - 'A' + 10); break; }
		  else
		   goto error;
      }
     ptr++;
   }

  nexttoken();
  return(result);

error:     
  Error("unexpected character %c in number", *ptr);
  Recover(2, T_SEMICOLON, T_RBRACE);
  return(-1);	                                     
}
  
/**
*** Read a network:
***  Network /xxx {
***  Subnet /xxx  {
**/
static	void	ReadNetworkContents(void);

static	void	ReadNetwork(void)
{ char	*network_name;

  if (token ne T_NETWORK)
   { Error("syntax error, expecting Network or Subnet");
     Recover(1, T_NETWORK);
   }

  nexttoken();
  unless (IsString(token))
   { Error("name expected after Network/Subnet");
     Recover(1, T_WORD);
   }

  if (tokenbuffer[0] eq '/')
   network_name = &(tokenbuffer[1]);
  else
   network_name = tokenbuffer;
  ValidateName(network_name);
  
  RmSetNetworkId(Current->Network, network_name);
   
  nexttoken();
  if (token ne T_LBRACE)
   { Error("{ expected after network name");
     Recover(1, T_LBRACE); 
   }
  nexttoken();
  
  ReadNetworkContents();

	/* If the network included a new reset driver or a new		*/
	/* configure driver, it must now be added to the network.	*/
  if (Current->NewResetDriver)
   if (RmAddHardwareFacility(Current->Network, Current->ResetDriver)
    	ne RmE_Success)
    MemoryError(5);

  if (Current->NewConfigureDriver)
   if (RmAddHardwareFacility(Current->Network, Current->ConfigureDriver)
      	ne RmE_Success)
    MemoryError(6);
             
  if (Current->Level eq 0)	/* top-level, next should be EOF */
   if (token ne T_EOF)
    Error("end of file expected after the last }");
}

/**
*** Network contents :
*** 1) the current network/subnet is terminated by a closing brace }
*** 2) named subnets are introduced by Network or Subnet
*** 3) unnamed subnets, which may have their own reset and configuration
***    specifications, are introduced with a new brace {
*** 4) processors are introduced with Processor or TERMINAL
*** 5) new-style hardware drivers are fairly obvious
*** 6) old-style hardware is specified by CONTROL
***
*** The main difference between named subnets and unnamed subnets is that the
*** former adds to the network hierarchy, i.e. a new RmNetwork structure has
*** to be allocated and included in the current subnet. Unnamed subnets may
*** have their own 
**/

static	void	ReadProcessor(void);
static	void	ReadReset(void);
static	void	ReadConfigure(void);
static	void	ReadControl(void);
static	void	AddToHardware(RmProcessor);

static	void	ReadNetworkContents(void)
{
  forever
   { 
     switch(token)
      { case	T_RBRACE	: 
      				  nexttoken();	/* } terminates this subnet */
      				  return;
      				  
	case	T_NETWORK	:
	 { StackNode	*newnode	= New(StackNode);
	   StackNode	*temp		= Current;
			/* Push a new stack node */
	   if (newnode eq Null(StackNode)) MemoryError(7);
	   newnode->Network		= RmNewNetwork();
	   if (newnode->Network eq (RmNetwork) NULL) MemoryError(8);
	   { List	*list = New(List);
	     if (list eq Null(List)) MemoryError(36);
	     InitList(list);
	     RmSetNetworkPrivate(newnode->Network, (int) list);
	   }
	   RmAddtailProcessor(Current->Network, (RmProcessor) newnode->Network);
	   newnode->ResetDriver 	= Current->ResetDriver;
	   newnode->ConfigureDriver	= Current->ConfigureDriver;
	   newnode->NewResetDriver	= FALSE;
	   newnode->NewConfigureDriver	= FALSE;
	   newnode->ResetMnode		= Null(char);	/* must be reset */
	   newnode->ConfigureMnode	= Null(char);
	   newnode->Level		= Current->Level + 1;
#ifdef SYSDEB
	   newnode->Node.Next = newnode->Node.Prev = &newnode->Node;
#endif
	   AddTail(&(Stack), &(newnode->Node));
	   Current			= newnode;

	   ReadNetwork();		/* extract the subnet	*/

	   Remove(&(newnode->Node));	/* and pop the stack	*/
	   Free(newnode);
	   Current = temp;
	   break;
	 }

	case	T_LBRACE	:
	 { StackNode	*newnode	= New(StackNode);
	   StackNode	*temp		= Current;
			/* Push a new stack node */
	   if (newnode eq Null(StackNode)) MemoryError(9);
	   newnode->Network		= Current->Network;
	   newnode->ResetDriver 	= Current->ResetDriver;
	   newnode->ConfigureDriver	= Current->ConfigureDriver;
	   newnode->NewResetDriver	= FALSE;
	   newnode->NewConfigureDriver	= FALSE;
	   newnode->ResetMnode		= Null(char);
	   newnode->ConfigureMnode	= Null(char);
	   newnode->Level		= Current->Level + 1;
#ifdef SYSDEB
	   newnode->Node.Next = newnode->Node.Prev = &newnode->Node;
#endif
	   AddTail(&(Stack), &(newnode->Node));
	   Current			= newnode;
	   
	   nexttoken();			/* skip the brace { */
	   ReadNetworkContents();	/* extract the subnet	*/

	   Current = temp;		/* pop the stack again */

		/* If the group of processors has its own reset or	*/
		/* configuration drivers, these must be added to the	*/
		/* current network.					*/
	   if (newnode->NewResetDriver)
	    if (RmAddHardwareFacility(Current->Network, newnode->ResetDriver) ne RmE_Success)
	     MemoryError(10);

	   if (newnode->NewConfigureDriver)
	    if (RmAddHardwareFacility(Current->Network, newnode->ConfigureDriver) ne RmE_Success)
	     MemoryError(11);
	     
	   Remove(&(newnode->Node));
	   Free(newnode);
	   break;
	 }
	
	case	T_PROCESSOR	: nexttoken();
				  ReadProcessor();
				  break;

	case	T_RESET		: nexttoken();
				  ReadReset();
				  break;
	
	case	T_CONFIGURE	: nexttoken();
				  ReadConfigure();
				  break;
	
	case	T_CONTROL	: nexttoken();
				  ReadControl();
				  break;
	
	case	T_SEMICOLON	:	/* for compatibility */
	case	T_HELIOS	:	/* old maps could give types	*/
	case	T_SYSTEM	:	/* to networks.			*/
	case	T_NATIVE	:
	case	T_IO		: nexttoken();
				  break;

	default	: Error("syntax error, expecting network contents");
		  Recover( 6, T_RBRACE, T_PROCESSOR, T_NETWORK, T_RESET,
		  		T_CONFIGURE, T_CONTROL);
		  break;
      }
   }
}

/**
*** ReadProcessor() : a processor consists of 
*** PROCESSOR
*** TERMINAL   name { links ; description }
**/
static	void	ReadLinksList(RmProcessor);
static	void	ReadProcessorDescription(RmProcessor);

static	void	ReadProcessor(void)
{ RmProcessor	processor = RmNewProcessor();
  char		*procname;
  
  if (processor eq (RmProcessor) NULL) MemoryError(12);

  unless(IsString(token))
   { Error("processor name expected");
     Recover(1, T_RBRACE);	/* skip this processor */
     nexttoken();
     return;
   }
     
  if (tokenbuffer[0] eq '/')
   procname = &(tokenbuffer[1]);
  else
   procname = tokenbuffer;

  ValidateName(procname);
  RmSetProcessorId(processor, procname);
  RmAddtailProcessor(Current->Network, processor);
  nexttoken();

  if (token ne T_LBRACE)
   { Error("{ expected after processor name");
     Recover(1, T_RBRACE);	/* skip this processor */
     nexttoken();
     return;
   }
  nexttoken();

  ReadLinksList(processor);
  ReadProcessorDescription(processor);
}

/**
*** ReadLinksList(). The first part of the processor description is
*** a list of link connections, separated by commas, and terminated by
*** a semicolon or a brace.
**/

static	void ReadLinksList(RmProcessor processor)
{ ProcessorLinks	*links = New(ProcessorLinks);
  LinkNode		*node;
  int			linkno = 0;

  if (links eq Null(ProcessorLinks)) MemoryError(13);
  InitList(&(links->List));
  links->Count = 0;
  RmSetProcessorPrivate(processor, (int) links);
  
  until ((token eq T_SEMICOLON) || (token eq T_RBRACE))
   { 
     if (token eq T_COMMA)
      { nexttoken();
        linkno++;
        continue;
      }
     else
      { node		= New(LinkNode);
	if (node eq Null(LinkNode)) MemoryError(14);
	node->Linkno	= linkno;
	node->Target	= RmM_NoProcessor;
	node->Destlink	= RmM_AnyLink;
#ifdef SYSDEB
	node->Node.Next = node->Node.Prev = &node->Node;
#endif
again:
        if (token eq T_EXTERNAL)
         node->Target	= RmM_ExternalProcessor;
        elif (token eq T_WORD)
         { if (tokenbuffer[0] eq '~')
            node->Name = BuildName(&(tokenbuffer[1]));
           else
            node->Name = CopyString(tokenbuffer);
         }
        else
         { Error("link connection expected");
           nexttoken();
	   goto again;
	 }
        AddTail(&(links->List), &(node->Node));
        links->Count++;
        
		/* Cope with ext[0], or ~00[0] */
        nexttoken();
        if (token eq T_LBRACE)
         { nexttoken();
           node->Destlink = ReadNumber();
           if (token ne T_RBRACE)
            { Error("] expected after link number");
              Recover(2, T_RBRACE, T_SEMICOLON);
            }
           nexttoken();
         }
      }
   }
}

/**
*** A processor description can contain lots of different thingies.
**/
static	void	ReadTask(RmProcessor      processor);
static	void	ReadMnode(RmProcessor     processor);
static	void	ReadNucleus(RmProcessor   processor);
static	void	ReadSpecial(RmProcessor   processor);

static	void	ReadProcessorDescription(RmProcessor processor)
{ int	purpose		= 0;
  bool	ptype_defined	= FALSE;
  bool	memory_defined	= FALSE;
  bool	isIO		= FALSE;
  
  forever
   { 
     switch(token)
      { case T_SEMICOLON : nexttoken();
 			   continue;
      				  
      	case T_RBRACE	 : nexttoken();
      			   unless(isIO) AddToHardware(processor);
      			   return;
      				  
      	case T_IO	 : purpose = (purpose & ~RmP_Mask) | RmP_IO;
  			   RmSetProcessorPurpose(processor, purpose);
      			   isIO = TRUE;
      			   nexttoken();
      			   continue;
      			   
      	case T_HELIOS	: purpose = (purpose & ~RmP_Mask) | RmP_Helios;
   			  RmSetProcessorPurpose(processor, purpose);
      			  nexttoken();
      			  continue;

	case T_NATIVE	: purpose = (purpose & ~RmP_Mask) | RmP_Native;
			  RmSetProcessorPurpose(processor, purpose);
      			  nexttoken();
      			  continue;

	case T_ROUTER	: purpose = (purpose & ~RmP_Mask) | RmP_Router;
			  RmSetProcessorPurpose(processor, purpose);
      			  nexttoken();
      			  continue;

	case T_SYSTEM	: if (purpose eq 0) purpose = RmP_Helios;
			  purpose |= RmP_System;
			  RmSetProcessorPurpose(processor, purpose);
      			  nexttoken();
      			  continue;

	case T_USER	: purpose &= ~RmP_System;
			  RmSetProcessorPurpose(processor, purpose);
      			  nexttoken();
      			  continue;

	case T_MEMORY	: { int size;
			    nexttoken();
			    size = ReadNumber();
			    if (size eq -1)
			     { Recover(2, T_SEMICOLON, T_RBRACE);
			       continue;
			     }
			    if (memory_defined)
			     Error("processor memory has been defined already");
			    else
			     RmSetProcessorMemory(processor, size);
			    continue;
			  }

	case T_PROCESSOR :
	case T_PTYPE	 :
		 { int ptype = RmT_Unknown;
		   nexttoken();
		   switch(token)
		    { case T_800	: ptype = RmT_T800;  break;
		      case T_414	: ptype = RmT_T414;  break;
		      case T_425	: ptype = RmT_T425;  break;
		      case T_400	: ptype = RmT_T400;  break;
		      case T_212	: ptype = RmT_T212;  break;
		      case T_9000	: ptype = RmT_T9000; break;
		      case T_i860	: ptype = RmT_i860;  break;
		      case T_ARM	: ptype = RmT_Arm;   break;
		      case T_680x0	: ptype = RmT_680x0; break;
		      case T_C40	: ptype = RmT_C40;   break;
		      default : Error("unknown processor type");
				Recover(2, T_SEMICOLON, T_RBRACE);
		      	        break;
		    }
		   if (ptype ne RmT_Unknown)
		    { if (ptype_defined)
		       Error("a processor type has been defined already");
		      else
		       RmSetProcessorType(processor, ptype);
		    }
		   nexttoken();
	           continue;
		 }

	case T_NUCLEUS	 : nexttoken();
			   ReadNucleus(processor);
			   continue;
	
	case T_ATTRIB	 : nexttoken();
	 		   unless(IsString(token))
	 		    { Error("attribute string expected");
			      Recover(2, T_SEMICOLON, T_RBRACE);
	 		      continue;
	 		    }
	 		   if (RmAddProcessorAttribute(processor, tokenbuffer)
	 		   	ne RmE_Success)
	 		    MemoryError(15);
	 		   nexttoken();
	 		   continue;
	
	case T_RUN	 : nexttoken();
			   ReadTask(processor);
			   continue;

	case T_WAITFOR	:	/* Initrc options supported by rmgen */
	case T_CONSOLE	:
			  ReadSpecial(processor);
			  continue;

	case T_MNODE	: nexttoken();
			  ReadMnode(processor);
			  continue;

	case T_IDROM	: nexttoken();
			  ReadIDRom(processor);
			  continue;

	case T_BOOTFILE	: nexttoken();
			  ReadBootfile(processor);
			  continue;

	default : Error("syntax error, processor description expected");
		  Recover(2, T_SEMICOLON, T_RBRACE);
		  break;
      }
   }
}

static void ReadNucleus(RmProcessor processor)
{ char		name[IOCDataMax];
  Object	*o;

  unless(IsString(token))
   { Error("nucleus file name expected");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }

  if (tokenbuffer[0] eq '/')
   strcpy(name, tokenbuffer);
  else
   { strcpy(name, "/helios/lib/");
     strcat(name, tokenbuffer);
   }

#ifdef __HELIOS
  o = Locate(Null(Object), name);
  if (o eq Null(Object))
   fprintf(stderr, "Warning: failed to locate nucleus %s\n", name);
  else
   Close(o);
#endif

  if (RmSetProcessorNucleus(processor, name) ne RmE_Success) MemoryError(17);

  nexttoken();
}

/**
*** ReadTask(). Tasks take two forms
*** 1) run -e /helios/bin/ls ls -l
*** 2) run /helios/bin/ls
**/

static	void	ReadTask(RmProcessor processor)
{ bool		env = FALSE;
  Object	*o;
  RmTask	task;
  int		argno = 0;
      
  unless(IsString(token))
   { Error("-e or command name expected after run");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }

  if ((tokenbuffer[0] eq '-') &&
      ((tokenbuffer[1] eq 'e') ||(tokenbuffer[1] eq 'E')) &&
      (tokenbuffer[2] eq '\0'))
   { env = TRUE; 
     nexttoken();
     unless(IsString(token))
      { Error("command expected after -e");
        Recover(2, T_SEMICOLON, T_RBRACE);
        return;
      }
   }

#ifdef __HELIOS
  o = Locate(Null(Object), tokenbuffer);
  if (o eq Null(Object))
   fprintf(stderr, "Warning: cannot find program %s\n", tokenbuffer);
  else
   Close(o);
#endif

  task = RmNewTask();
  if (task eq (RmTask) NULL) MemoryError(18);
  if (RmSetTaskCode(task, tokenbuffer) ne RmE_Success) MemoryError(19);

  RmAddtailTask(Taskforce, task);
  if (RmMapTask(processor, task) ne RmE_Success)
   Error("failed to map task onto processor.");
   
  nexttoken();
  
  if (!env)
   { RmSetTaskId(task, "0");	/* BLV - hack for now */
     if ((token ne T_SEMICOLON) && (token ne T_RBRACE))
      { Error("-e not used, environment should not be specified");
        Recover(2, T_SEMICOLON, T_RBRACE);
       }
     return;
   }

  RmSetTaskId(task, "1");	/* BLV - hack for now */
  until ((token eq T_SEMICOLON) || (token eq T_RBRACE))
   { unless(IsString(token))
      { Error("program argument expected");
        Recover(2, T_SEMICOLON, T_RBRACE);
        return;
      }
     if (argno eq 0)	/* BLV - skip for now */
      argno++;
     else
      if (RmAddTaskArgument(task, argno++, tokenbuffer) ne RmE_Success)
       MemoryError(20);
     nexttoken();
   }
}

/**
*** ReadSpecial(). This supports the initrc commands waitfor and console.
*** Correct interpretation of these is left to
*** the Network Server. rmgen simply puts all the information into an
*** RmTask structure.
**/
static void	ReadSpecial(RmProcessor processor)
{ RmTask	task;
  TOKEN		old_token = token;
  int		argno	  = 1;

  nexttoken();	/* NOT done in ReadProcessorContents() */

  task = RmNewTask();
  if (task eq (RmTask) NULL) MemoryError(41);
  RmSetTaskId(task, (old_token eq T_WAITFOR) ? "waitfor" : "console");

	/* Everything after the initial command is held as arguments */
  until ((token eq T_SEMICOLON) || (token eq T_RBRACE))
   { unless(IsString(token))
      { Error("argument expected after %s", RmGetTaskId(task));
        Recover(2, T_SEMICOLON, T_RBRACE);
        RmFreeTask(task);
        return;
      }

     if (RmAddTaskArgument(task, argno++, tokenbuffer) ne RmE_Success)
      MemoryError(20);

     nexttoken();
   }

	/* The different specials take different numbers of arguments */
	/* Note that there is no need to recover since we are already */
	/* at a semicolon or closing brace.			      */
  if (old_token == T_WAITFOR)
   { 			/* waitfor /xyz */
     if (argno != 2)
      { Error("single server name expected after waitfor");
        RmFreeTask(task);
        return;
      }
   }
  else
   {			/* console /window console */
			/* console /logger	   */
     if (argno > 3)
      { Error("too many arguments after console");
        RmFreeTask(task);
        return;
      }
   }

  RmAddtailTask(Taskforce, task);
  if (RmMapTask(processor, task) ne RmE_Success)
   Error("failed to map special onto processor.");
}

/**
*** Hardware nasties.
***
*** First, ReadConfigure(). This one is easy. The first few bits are fixed.
*** Configure { driver; option; name }
***
*** Note that the hardware facility cannot be added to the network until
*** all the controlled processors are known. This is because
*** RmAddHardwareFacility() makes a copy of the data structure, which cannot
*** change in size as the network is parsed. Hence adding hardware facilities
*** to the network is delayed until a whole network/subnet has been parsed.
**/

static	void ReadConfigure(void)
{ RmHardwareFacility	*hardware;
  Object		*lib;
  Object		*o;

  if (token ne T_LBRACE)
   { Error("{ expected after configure"); goto error; }

  nexttoken();
  if (token ne T_DRIVER)
   { Error("\"driver\" expected, configuration commands are not supported");
     goto error;
   }
  nexttoken();
  if (token ne T_SEMICOLON)
   { Error("; expected after driver"); goto error; }
  nexttoken();

  hardware = New(RmHardwareFacility);  
  if (hardware eq Null(RmHardwareFacility)) MemoryError(21);
  hardware->Type	     = RmH_ConfigureDriver;
  hardware->NumberProcessors = 0;
  hardware->Spare[0]	     = 32;
  hardware->Processors	     = (RmProcessor *) Malloc(32 * sizeof(RmProcessor));
  if (hardware->Processors eq Null(RmProcessor)) MemoryError(22);

  if (IsString(token))
   { char *temp;
     if (tokenbuffer[0] eq '~')
      temp = BuildName(&(tokenbuffer[1]));
     else
      temp = tokenbuffer;
     if (strlen(temp) > 63)
      { Error("driver option too long"); goto error; }
     strcpy(hardware->Option, temp);
     if (tokenbuffer[0] eq '~') Free(temp);
     nexttoken();
   }
  else
   hardware->Option[0] = '\0';
   
  if (token ne T_SEMICOLON)
   { Error("; expected after driver option"); goto error; }
  nexttoken();
  
  unless(IsString(token))
   { Error("a driver name must be provided"); goto error; }
  
  if (strlen(tokenbuffer) > 63)
   { Error("driver name too long"); goto error; }

  strcpy(hardware->Name, tokenbuffer);
  Current->ConfigureDriver = hardware;
  Current->NewConfigureDriver = TRUE;

#ifdef __HELIOS  
  if (tokenbuffer[0] eq '/')
   o = Locate(Null(Object), tokenbuffer);
  else
   { lib = Locate(Null(Object), "/helios/lib");
     if (lib eq Null(Object))
      o = Null(Object);
     else
      { o = Locate(lib, tokenbuffer);
        Close(lib);
      }
   }
  if (o eq Null(Object))
   fprintf(stderr, "Warning: failed to find configuration driver %s\n",
   		 tokenbuffer);
  else
   Close(o);
#endif
   
  nexttoken();
  
  if (token ne T_RBRACE)
   { Error("} expected after driver name"); goto error; }
   
  nexttoken();
  return;
  
error:
  Recover(1, T_RBRACE);
  nexttoken();	/* the } must not terminate the current network */
  return;   
}

/**
*** The reset case is slightly more difficult. There are two versions,
*** one for reset commands, and one for reset drivers.
**/
static	void	ReadResetCommand(void);

static	void ReadReset(void)
{ RmHardwareFacility	*hardware;
  Object		*lib;
  Object		*o;

  if (token ne T_LBRACE)
   { Error("{ expected after reset"); goto error; }

  nexttoken();
  if ((token ne T_DRIVER) && IsString(token))
   { ReadResetCommand(); return; }
   
  if (token ne T_DRIVER)
   { Error("\"driver\" expected");
     goto error;
   }
  nexttoken();
  if (token ne T_SEMICOLON)
   { Error("; expected after driver"); goto error; }
  nexttoken();

  hardware = New(RmHardwareFacility);  
  if (hardware eq Null(RmHardwareFacility)) MemoryError(23);
  hardware->Type	     = RmH_ResetDriver;
  hardware->NumberProcessors = 0;
  hardware->Spare[0]	     = 32;
  hardware->Processors	     = (RmProcessor *) Malloc(32 * sizeof(RmProcessor));
  if (hardware->Processors eq Null(RmProcessor)) MemoryError(24);

  if (IsString(token))
   { char *temp;
     if (tokenbuffer[0] eq '~')
      temp = BuildName(&(tokenbuffer[1]));
     else
      temp = tokenbuffer;
     if (strlen(temp) > 63)
      { Error("driver option too long"); goto error; }
     strcpy(hardware->Option, temp);
     if (tokenbuffer[0] eq '~') Free(temp);
     nexttoken();
   }
  else
   hardware->Option[0] = '\0';
   
  if (token ne T_SEMICOLON)
   { Error("; expected after driver option"); goto error; }
  nexttoken();
  
  unless(IsString(token))
   { Error("a driver name must be provided"); goto error; }
  
  if (strlen(tokenbuffer) > 63)
   { Error("driver name too long"); goto error; }

  strcpy(hardware->Name, tokenbuffer);
  Current->ResetDriver = hardware;
  Current->NewResetDriver = TRUE;

#ifdef __HELIOS  
  if (tokenbuffer[0] eq '/')
   o = Locate(Null(Object), tokenbuffer);
  else
   { lib = Locate(Null(Object), "/helios/lib");
     if (lib eq Null(Object))
      o = Null(Object);
     else
      { o = Locate(lib, tokenbuffer);
        Close(lib);
      }
   }
  if (o eq Null(Object))
   fprintf(stderr, "Warning: failed to find reset driver %s\n",
   		 tokenbuffer);
  else
   Close(o);
#endif
   
  nexttoken();
  
  if (token ne T_RBRACE)
   { Error("} expected after driver name"); goto error; }
   
  nexttoken();
  return;
  
error:
  Recover(1, T_RBRACE);
  nexttoken();	/* the } must not terminate the current network */
  return;   
}

/**
*** Reset commands. These take the following form
***    Reset { <processor list> ; processor; command }
*** The processor list looks just like the links list, and a similar
*** same structures can be used. For now only the processor names are
*** considered, and the semantic analysis stage will turn these into
*** RmProcessor pointers. This means that the reset command cannot be
*** added to the hardware facilities until then.
***
*** One problem with the syntax is the command. The current
*** data structures simply copy the command and its arguments into a
*** buffer, rather than parsing it and turning it into a task structure.
**/

static	void	ReadResetCommand(void)
{ RmHardwareFacility	*hardware = New(RmHardwareFacility);
  ResetCommand		*command  = New(ResetCommand);
  List			*list;
  LinkNode		*node;
  char			*control;
  char			*cmd;
        
  if (hardware eq Null(RmHardwareFacility)) MemoryError(25);
  if (command eq Null(ResetCommand)) MemoryError(26);
  list = (List *) RmGetNetworkPrivate(Current->Network);
  
  hardware->Type		= RmH_ResetCommand;
  hardware->NumberProcessors	= 0;
  hardware->Device		= Current->Network;
  command->hardware		= hardware;
  InitList(&(command->Processors));

  until (token eq T_SEMICOLON)
   { 
     if (token eq T_COMMA)
      { nexttoken(); continue; }
     unless(IsString(token))
      { Error("processor name expected"); goto error; }   	

     node = New(LinkNode);
     if (node eq Null(LinkNode)) MemoryError(27);
     if (tokenbuffer[0] eq '~')
      node->Name = BuildName(&(tokenbuffer[1]));
     else
      node->Name = CopyString(tokenbuffer);
#ifdef SYSDEB
     node->Node.Next = node->Node.Prev = &node->Node;
#endif
     AddTail(&(command->Processors), &(node->Node));
     hardware->NumberProcessors++;
     nexttoken();
   }  

  nexttoken();	/* get past the semicolon */
  unless(IsString(token))
   { Error("processor name expected"); goto error; }
  if (tokenbuffer[0] eq '~')
   control = BuildName(&(tokenbuffer[1]));
  else
   control = CopyString(tokenbuffer);
   
  if (strlen(control) > 63)
   { Error("processor name %s too long", control); goto error; }
  strcpy(hardware->Option, control);
  Free(control);
  
  nexttoken();
  if (token ne T_SEMICOLON)
   { Error("; expected after processor"); goto error; }

  nexttoken();      
  if (token ne T_RUN)
   { Error("\"run\" command expected"); goto error; }

	/* ch is currently blank space or a meta character after run */
  while (isspace(ch)) nextch();
  cmd = &(hardware->Name[0]);
  until( (ch eq '}') || (ch eq EOF))
   { if (isspace(ch))
      { *cmd++ = ch; 
        while (isspace(ch)) nextch();
        continue;
      }
     else
      { *cmd++ = ch;
        nextch();
      }
   } 
  if (ch ne '}') Error("} expected after reset command");

  nextch();	/* skip the { */
  nexttoken();	/* and get the next token */

#ifdef SYSDEB
  command->Node.Next = command->Node.Prev = &command->Node;
#endif
  AddTail(list, &(command->Node));
   
  return;
  
error:
  Recover(1, T_RBRACE);
  nexttoken();
  return;
}

/**
*** AddToHardware(). Whenever a processor is added to the network it must
*** be added to the current reset and configuration drivers, if defined.
**/
static	void	AddToHardware(RmProcessor processor)
{ RmHardwareFacility	*hardware;

  if (Current->ConfigureDriver ne Null(RmHardwareFacility))
   { hardware = Current->ConfigureDriver;
     if (hardware->NumberProcessors  eq hardware->Spare[0])
      { if ((hardware->Processors = (RmProcessor *)
          realloc(hardware->Processors, 2 * hardware->Spare[0] * sizeof(RmProcessor)))
      	     eq NULL)
      	 MemoryError(29);
      	hardware->Spare[0] *= 2;
      }
     hardware->Processors[hardware->NumberProcessors] = processor;
     hardware->NumberProcessors++;
   }

  if (Current->ResetDriver ne Null(RmHardwareFacility))
   { hardware = Current->ResetDriver;
     if (hardware->NumberProcessors  eq hardware->Spare[0])
      { if ((hardware->Processors = (RmProcessor *)
          realloc(hardware->Processors, 2 * hardware->Spare[0] * sizeof(RmProcessor)))
      	     eq NULL)
      	 MemoryError(30);
      	hardware->Spare[0] *= 2;
      }
     hardware->Processors[hardware->NumberProcessors] = processor;
     hardware->NumberProcessors++;
   }
}

/**
*** Support for the old style reset and configuration specifications.
*** YUK YUK YUK.
***
*** When a control statement is encountered, a hardware facility is
*** allocated and put into Current, so it will percolate down as
*** before. In addition the Mnode field of Current is filled in, but
*** this does not percolate down. When the Mnode is reached the stack is
*** searched for the right string, and the corresponding hardware facility
*** is updated.
**/

static void ReadControl(void)
{ bool			ConfigureControl;
  RmHardwareFacility	*hardware;
  char			*mnode;

  if (token eq T_RSTANL)
   ConfigureControl = FALSE;
  elif (token eq T_CONFIGURE)
   ConfigureControl = TRUE;
  else
   { Error("Rst_Anl or CONFIGURE expected after CONTROL");
     Recover(1, T_RBRACE);
     return;
   }
  nexttoken();
  
  if (token ne T_LBRACE)
   { Error("[ expected before Mnode specifier");
     Recover(1, T_RBRACE);
     return;
   }
  nexttoken();

  unless(IsString(token))
   { Error("Mnode processor expected");
     Recover(1, T_RBRACE);
     return;
   }

  if (tokenbuffer[0] ne '~')
   mnode = CopyString(tokenbuffer);
  else
   mnode = BuildName(&(tokenbuffer[1]));
    
  hardware = New(RmHardwareFacility);
  if (hardware eq Null(RmHardwareFacility)) MemoryError(31);
  hardware->Type = (ConfigureControl) ? RmH_ConfigureDriver : RmH_ResetDriver;
  hardware->NumberProcessors	= 0;
  hardware->Spare[0]		= 32;
  hardware->Processors		= (RmProcessor *) Malloc(32 * sizeof(RmProcessor));
  if (hardware->Processors eq Null(RmProcessor)) MemoryError(32);
  hardware->Option[0]		= '\0';
  hardware->Name[0]		= '\0';
    
  if (ConfigureControl)
   { Current->ConfigureDriver		= hardware;
     Current->ConfigureMnode		= mnode;
     Current->NewConfigureDriver	= TRUE;
   }
  else
   { Current->ResetDriver		= hardware;
     Current->ResetMnode		= mnode;
     Current->NewResetDriver		= TRUE;
   }

  nexttoken();		/* get past the mnode */
  if (token ne T_RBRACE)
   { Error("] expected after Mnode processor");
     Recover(1, T_RBRACE);
     return;
   }
  nexttoken();  
}

static	void	ReadMnode(RmProcessor processor)
{ bool			ConfigureMnode;
  char			*procname;
  StackNode		*node;
  RmHardwareFacility	*hardware;

  if (token eq T_RSTANL)
   ConfigureMnode = FALSE;
  elif (token eq T_CONFIGURE)
   ConfigureMnode = TRUE;
  else
   { Error("Rst_Anl or CONFIGURE expected after CONTROL");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }
   
  nexttoken();
  if (token ne T_LBRACE)
   { Error("[ expected before driver name");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }
   
  nexttoken();
  unless(IsString(token))
   { Error("device driver name expected after [");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }
   
  procname = BuildName((char *) RmGetProcessorId(processor));

  for (node = Head_(StackNode, Stack); 
       !EndOfList_(node); 
       node = Next_(StackNode, node))
   if (  ((ConfigureMnode) && 
          (node->ConfigureMnode ne Null(char)) && 
          (!strcmp(node->ConfigureMnode, procname)) )
       ||          
         ((!ConfigureMnode) &&       
          (node->ResetMnode ne Null(char)) &&
          (!strcmp(node->ResetMnode, procname)) ) )
    break;
    
  if (EndOfList_(node))
   { Error("no matching CONTROL statement");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }

  if (ConfigureMnode)
   hardware = node->ConfigureDriver;
  else
   hardware = node->ResetDriver;

  if (strlen(tokenbuffer) > 63)
   { Error("device driver name too long");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }
   
  strcpy(hardware->Name, tokenbuffer);

#ifdef __HELIOS
  { Object	*lib, *o;
    
    if (tokenbuffer[0] eq '/')
     o = Locate(Null(Object), tokenbuffer);
    else
     { lib = Locate(Null(Object), "/helios/lib");
       if (lib eq Null(Object))
        o = Null(Object);
       else
        { o = Locate(lib, tokenbuffer);
          Close(lib);
        }
     }
    if (o eq Null(Object))
     fprintf(stderr, "Warning: failed to locate device driver %s\n", tokenbuffer);
    else
     Close(o);
  }
#endif

  nexttoken();		/* skip past the device driver name */
  if (token ne T_RBRACE)
   { Error("] expected after device driver name");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }
  nexttoken();
}

/**
*** IdRom support. Some C40 networks contain processors without IdRoms,
*** and Helios is still expected to run on them. To implement this it
*** is possible to specify IdRom files in the resource map which provide
*** or replace the necessary information. This would work roughly
*** as follows:
***
*** Network /Net {
***	Processor root { ... }
***     Processor 01   { ... ; idrom /helios/etc/id1.rom }
*** }
***
*** The file id1.rom contains the same information as the host.con file,
*** and works in effectively the same way:
***
*** # IdRom for DSP1 board with 1Mb SRAM and 4Mb DRAM on local,
*** # plus 1Mb SRAM on global.
***
*** c40_idrom_cpu_clk	=	59
*** c40_idrom_lbase0	=	0x00300000
*** c40_idrom_lbase1	=	0x00400000
*** c40_idrom_lsize0	=	0x00040000	# 1Mb SRAM
*** c40_idrom_lsize1	=	0x00100000	# 4Mb DRAM
*** c40_idrom_lbcr	=	0x154d4010
***
*** Note that the undocumented features provided by the I/O Server are
*** also supported. This information is held in a private attribute,
*** idrom=000000001111111122222222...
***
*** In addition to the IdRom information it is necessary to specify
*** a hardware config word. This is used to disable the cache etc.
**/
/*{{{  encode data */
/**
*** This little routine is used to encode binary data into text,
*** so that it can be embedded in an attribute string.
**/
static void encode_data(char *buffer, int length, BYTE *data)
{
  while (length-- > 0)
   { int x = (*data >> 4) & 0x0F;
     if (x <= 9)
      *buffer++ = x + '0';
     else
      *buffer++ = x + 'a' - 10;
     x = (*data++) & 0x0F;
     if (x < 10)
      *buffer++ = x + '0';
     else
      *buffer++ = x + 'a' - 10;
   }
  *buffer = '\0';
}
/*}}}*/

static void	ReadIDRom(RmProcessor processor)
{ FILE		*idrom_file	= NULL;
  static IDROM	 default_rom	= {
     /*{{{  default values */
         	sizeof(IDROM) / sizeof(word),	/* self inclusive size of this block		*/
         	0,				/* TIM-40 module manufacturer's ID		*/
         	0,				/* CPU type (00 = C40				*/
         	49,				/* CPU cycle time (49 = 50ns = 40MHz)		*/
         	0,				/* manufacturer's module type			*/
         	0,				/* module revision level			*/
         	0,				/* reserved byte				*/
         	0x80000000,			/* address base of global bus strobe 0		*/
         	0xffffffff,			/* address base of global bus strobe 1		*/
         	0x00300000,			/* address base of local bus strobe 0		*/
         	0xffffffff,			/* address base of local bus strobe 1		*/
         	0x00100000, /*(4Mb, 1Mw)*/	/* size of memory on global bus strobe 0	*/
         	0x00000000,			/* size of memory on global bus strobe 1	*/
         	0x00100000,			/* size of memory on local bus strobe 0		*/
         	0x00000000,			/* size of memory on local bus strobe 1		*/
         	0x00000800,			/* size of fast ram pool (inc. on-chip RAM	*/
         	0x22,				/* wait states within page on global bus	*/
         	0x22,				/* wait states within page on local bus		*/
         	0x55,				/* wait states outside page on global bus	*/
         	0x55,				/* wait states outside page on local bus	*/
         	0x2710,				/* period time for 1ms interval on timer 0	*/
         	0x80,				/* period for DRAM refresh timer		*/
         	0x2c2,				/* contents set TCLK0 to access RAM not IDROM	*/
         	0,				/* sets up timer to refresh DRAM		*/
         	0x3e39fff0,			/* global bus control register			*/
         	0x3e39fff0,			/* local bus control register			*/
         	0				/* total size of auto-initialisation data	*/
         };
     /*}}}*/
  IDROM		 idrom;
  char		 buf[IOCDataMax];
  word		 hwconfig	= 0;
  char		 hwconfig_buf[18];
  char		 idrom_buf[7 + (2 * sizeof(IDROM))];

  idrom = default_rom;

  unless(IsString(token))
   { Error("IdRom file name expected");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }

  if (tokenbuffer[0] eq '/')
   strcpy(buf, tokenbuffer);
  else
   { strcpy(buf, "/helios/etc/");
     strcat(buf, tokenbuffer);
   }

  idrom_file	= fopen(tokenbuffer, "r");
  if (idrom_file eq NULL)
   { Error("cannot open idrom file %s", tokenbuffer);
     goto done;
   }

  while (fgets(buf, IOCDataMax - 1, idrom_file) ne NULL)
   /*{{{  process idrom info */
    { char	*tmp;
      char 	*parameter;
      char	*value_pos;
      int	 config_value = 0;
      char	 ch;
   
      enum
	{ 
	  ERROR_NONE,
	  ERROR_EXTRANEOUS_VALUE,
	  ERROR_NO_VALUE
	} 
      error;
      
     	/* Step 1, search the string for a # and strip out the comment	*/
      for (tmp = buf; *tmp ne '\0'; tmp++)
       if (*tmp eq '#')
        { *tmp = '\0'; break; }
     
   	/* Step 2, ignore leading space and identify the parameter	*/
      for (parameter = buf; isspace(*parameter) && (*parameter ne '\0'); parameter++);
      if (*parameter eq '\0')	/* blank line ? */
       continue;
   
    	/* Step 3, terminate the parameter string			*/
      for (value_pos = parameter; !(isspace(*value_pos)) && (*value_pos ne '=') && (*value_pos ne '\0'); value_pos++);
      ch = *value_pos;
      *value_pos = '\0';
   
     	/* Step 4, skip white space between parameter and value		*/
      if (isspace(ch))
       { for (value_pos++; isspace(*value_pos) && (*value_pos ne '\0'); value_pos++);
         ch = *value_pos;
       }
   
    	/* Step 5, if ch is =, identify the start of the value field	*/
      if (ch ne '=')
       value_pos = NULL;
      else
       { for (value_pos++; isspace(*value_pos) && (*value_pos ne '\0'); value_pos++);
         if (*value_pos eq '\0')
          { Error("IDRom file %s, no value for %s", tokenbuffer, parameter);
   	    continue;
    	  }
   	 /* Step 6, determine the end of the value field		*/
   	 for (tmp = value_pos; !(isspace(*tmp)) && (*tmp ne '\0'); tmp++);
         *tmp = '\0';
       }
   
   	/* Step 7, turn value into a number				*/
      if (value_pos ne NULL)
       { if ((value_pos[0] eq '0') && ((value_pos[1] eq 'x') || (value_pos[1] eq 'X')))
          { value_pos += 2;
   	    if (*value_pos eq '\0')
             { Error("IDRom file %s, invalid value for %s", tokenbuffer, parameter);
               continue;
             }
   	    for (config_value = 0; *value_pos ne '\0'; value_pos++)
   	     if ((*value_pos >= '0') && (*value_pos <= '9'))
              config_value = (16 * config_value) + *value_pos - '0';
   	     elif ((*value_pos >= 'a') && (*value_pos <= 'f'))
   	      config_value = (16 * config_value) + *value_pos + 10 - 'a';
   	     elif ((*value_pos >= 'A') && (*value_pos <= 'F'))
   	      config_value = (16 * config_value) + *value_pos + 10 - 'A';
   	     else
   	      { Error("IDRom file %s, invalid value for %s", tokenbuffer, parameter);
   		goto loop;
   	      }
          }
         else
          for (config_value = 0; *value_pos ne '\0'; value_pos++)
   	   if ((*value_pos >= '0') && (*value_pos <= '9'))
            config_value = (10 * config_value) + *value_pos - '0';
   	   else
   	    { Error("IDRom file %s, invalid value for %s", tokenbuffer, parameter);
   	      goto loop;
   	    }
       }
   
   	/* At this point "parameter" is the name of the configuration	*/
   	/* option and either value_pos is NULL or config_value holds	*/
   	/* the numerical value associated with this option.		*/
   	/* It is now possible to match the option with the known	*/
   	/* strings and update the IDRom or the hardware configuration	*/
   	/* word.							*/
   
      error = ERROR_NONE;
      
      if   (!mystrcmp(parameter, "c40_disable_cache"))
       { hwconfig |= HW_CacheOff;
	 if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
       }
      elif (!mystrcmp(parameter, "c40_load_nucleus_local_s0"))
       {	/* LocalS0 == 0, set another bit to sort this out */
   	 hwconfig |= (HW_NucleusLocalS0 + 0x00008000);
   	 if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
       }
      elif (!mystrcmp(parameter, "c40_load_nucleus_local_s1"))
       { hwconfig |= HW_NucleusLocalS1;
	 if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
       }
      elif (!mystrcmp(parameter, "c40_load_nucleus_global_s0"))
       { hwconfig |= HW_NucleusGlobalS0;
	 if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
       }
      elif (!mystrcmp(parameter, "c40_load_nucleus_global_s1"))
       { hwconfig |= HW_NucleusGlobalS1;
	 if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
       }
      elif (!mystrcmp(parameter, "c40_replace_idrom"))
       { hwconfig |= HW_ReplaceIDROM;
	 if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
       }
      elif (!mystrcmp(parameter, "c40_use_pseudo_idrom"))
       { hwconfig |= HW_PseudoIDROM;
	 if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
       }
      elif (!mystrcmp(parameter, "c40_idrom_man_id"))
       { 
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.MAN_ID = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_cpu_id"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.CPU_ID = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_cpu_clk"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.CPU_CLK = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_model_no"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.MODEL_NO = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_rev_lvl"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.REV_LVL = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_reserved"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.RESERVED = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_gbase0"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.GBASE0 = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_gbase1"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.GBASE1 = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_lbase0"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.LBASE0 = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_lbase1"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.LBASE1 = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_gsize0"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.GSIZE0 = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_gsize1"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.GSIZE1 = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_lsize0"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.LSIZE0 = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_lsize1"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.LSIZE1 = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_fsize"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.FSIZE = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_wait_g0"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.WAIT_G = (idrom.WAIT_G & 0xf0) | config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_wait_g1"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.WAIT_G = (idrom.WAIT_G & 0x0f) | (config_value << 4);
       }
      elif (!mystrcmp(parameter, "c40_idrom_wait_l0"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.WAIT_L = (idrom.WAIT_L & 0xf0) | config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_wait_l1"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.WAIT_L = (idrom.WAIT_L & 0x0f) | (config_value << 4);
       }
      elif (!mystrcmp(parameter, "c40_idrom_pwait_g0"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.PWAIT_G = (idrom.PWAIT_G & 0xf0) | config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_pwait_g1"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.PWAIT_G = (idrom.PWAIT_G & 0x0f) | (config_value << 4);
       }
      elif (!mystrcmp(parameter, "c40_idrom_pwait_l0"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.PWAIT_L = (idrom.PWAIT_L & 0xf0) | config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_pwait_l1"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.PWAIT_L = (idrom.PWAIT_L & 0x0f) | (config_value << 4);
       }
      elif (!mystrcmp(parameter, "c40_idrom_timer0_period"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.TIMER0_PERIOD = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_timer1_period"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.TIMER1_PERIOD = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_timer0_ctrl"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.TIMER0_CTRL = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_timer1_ctrl"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.TIMER1_CTRL = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_gbcr"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.GBCR = config_value;
       }
      elif (!mystrcmp(parameter, "c40_idrom_lbcr"))
       {
	 if (value_pos eq NULL)
	   error = ERROR_NO_VALUE;
	 else
	   idrom.LBCR = config_value;
       }
      else
       Error("IDRom file %s, unknown parameter %s", tokenbuffer, parameter);
   
      if (error == ERROR_NO_VALUE)
	{ 
	  Error("IDRom file %s, missing value for %s", tokenbuffer, parameter);
	} 
      elif (error == ERROR_EXTRANEOUS_VALUE)
	{ 
	  Error("IDRom file %s, extraneous value for parameter %s", tokenbuffer, parameter);
	} 
      
    loop:
      ch = ch;
    }
   /*}}}*/

  if (memcmp(&idrom, &default_rom, sizeof(IDROM)) ne 0)
   if ((hwconfig & (HW_ReplaceIDROM | HW_PseudoIDROM)) == 0)
    { Error("IDRom information has been supplied without setting either ReplaceIDRom or PseudoIDRom");
      goto done;
    }

  if ((hwconfig == 0) && (memcmp(&idrom, &default_rom, sizeof(IDROM)) eq 0))
   { fprintf(stderr, "Warning, IDRom file %s has no effect.\n", tokenbuffer);
     goto done;
   }
  hwconfig &= ~0x00008000;
  
	/* At this point there is guaranteed to be something in 	*/
	/* hwconfig, but the IDRom info may be unchanged. It is still	*/
	/* necessary to store the IDRom info in an attribute if the	*/
	/* ReplaceIDRom or PseudoIDRom flag has been set, to overwrite	*/
	/* an existing IDRom with the default one.			*/
  strcpy(hwconfig_buf, "hwconfig=");
  encode_data(&(hwconfig_buf[9]), sizeof(word), (BYTE *) &hwconfig);
  if (RmAddObjectAttribute((RmObject) processor, hwconfig_buf, TRUE) ne RmE_Success)
   MemoryError(42);

  if (hwconfig & (HW_ReplaceIDROM | HW_PseudoIDROM))
   { strcpy(idrom_buf, "idrom=");
     encode_data(&(idrom_buf[6]), sizeof(IDROM), (BYTE *) &idrom);
     if (RmAddObjectAttribute((RmObject) processor, idrom_buf, TRUE) ne RmE_Success)
      MemoryError(43);
#if 0
     /*{{{  output idrom info */
     printf("Processor %s: idrom info\n", RmGetProcessorId(processor));
     printf("Hardware config word : %x\n", hwconfig);
     printf("SIZE                 : %d\n", idrom.SIZE);
     printf("MAN_ID               : %d\n", idrom.MAN_ID);
     printf("CPU_ID               : %d\n", idrom.CPU_ID);
     printf("CPU_CLK              : %d\n", idrom.CPU_CLK);
     printf("MODEL_NO             : %d\n", idrom.MODEL_NO);
     printf("REV_LVL              : %d\n", idrom.REV_LVL);
     printf("RESERVED             : %d\n", idrom.RESERVED);
     printf("GBASE0               : 0x%08x\n", idrom.GBASE0);
     printf("GBASE1               : 0x%08x\n", idrom.GBASE1);
     printf("LBASE0               : 0x%08x\n", idrom.LBASE0);
     printf("LBASE1               : 0x%08x\n", idrom.LBASE1);
     printf("GSIZE0               : 0x%08x\n", idrom.GSIZE0);
     printf("GSIZE1               : 0x%08x\n", idrom.GSIZE1);
     printf("LSIZE0               : 0x%08x\n", idrom.LSIZE0);
     printf("LSIZE1               : 0x%08x\n", idrom.LSIZE1);
     printf("FSIZE                : 0x%08x\n", idrom.FSIZE);
     printf("WAIT_G               : 0x%02x\n", idrom.WAIT_G);
     printf("WAIT_L               : 0x%02x\n", idrom.WAIT_L);
     printf("PWAIT_G              : 0x%02x\n", idrom.PWAIT_G);
     printf("PWAIT_L              : 0x%02x\n", idrom.PWAIT_L);
     printf("TIMER0_PERIOD        : 0x%08x\n", idrom.TIMER0_PERIOD);
     printf("TIMER0_CTRL          : 0x%08x\n", idrom.TIMER0_CTRL);
     printf("TIMER1_PERIOD        : 0x%08x\n", idrom.TIMER1_PERIOD);
     printf("TIMER1_CTRL          : 0x%08x\n", idrom.TIMER1_CTRL);
     printf("GBCR                 : 0x%08x\n", idrom.GBCR);
     printf("LBCR                 : 0x%08x\n", idrom.LBCR);
     /*}}}*/
#endif

   }
done:
  if (idrom_file ne NULL)
   fclose(idrom_file);
  nexttoken();	/* skip past the filename	*/
}

/**
*** Also, support for special bootfiles.
**/
static void ReadBootfile(RmProcessor processor)
{ char		 buf[IOCDataMax];
  Object	*o;
  char		*name;

  strcpy(buf, "bootfile=");
  name = buf + strlen(buf);

  unless(IsString(token))
   { Error("bootstrap file name expected");
     Recover(2, T_SEMICOLON, T_RBRACE);
     return;
   }

  if (tokenbuffer[0] eq '/')
   strcpy(name, tokenbuffer);
  else
   { strcpy(name, "/helios/lib/");
     strcat(name, tokenbuffer);
   }

#ifdef __HELIOS
  o = Locate(Null(Object), name);
  if (o eq Null(Object))
   fprintf(stderr, "Warning: failed to locate nucleus %s\n", name);
  else
   Close(o);
#endif

  if (RmAddObjectAttribute((RmObject) processor, buf, TRUE) ne RmE_Success)
   MemoryError(44);

  nexttoken();
}

/**----------------------------------------------------------------------------
*** Right, that is the syntax analysis taken care off. It is time for the
*** semantic analysis.
***
*** The following jobs are required:
*** 1) check for name clashes, e.g. two processors in the same subnet with
***    the same name
*** 2) check all hardware facilities. In the case of reset and configure
***    drivers check that the name has been filled in. If not there is
***    a CONTROL statement without a missing Mnode. In the case of reset
***    commands figure out which processors are affected and finish off
***    the hardware facility structure.
*** 3) for every link
***     a) if external, fill it in
***     b) if connected, find the matching processor
*** 4) for every connected link, try to resolve the topology
**/

static	int	CheckHardware(RmHardwareFacility *hardware, ...);
static	int	ValidateNetwork(RmProcessor, ...);
static	int	ResolveLinks(RmProcessor, ...);
static	void	CheckResetCommand(ResetCommand *);

static	void ResolveProblems(RmNetwork network)
{ List	*list;

  if (RmCountProcessors(network) eq 0)
   Error("the network is empty");

  (void) RmApplyHardwareFacilities(network, &CheckHardware);
  list = (List *) RmGetNetworkPrivate(network);
  (void) WalkList(list, (WordFnPtr) &CheckResetCommand);
  
  (void) RmApplyNetwork(network, &ValidateNetwork);
  (void) RmApplyProcessors(network, &ResolveLinks);
}

/**
*** This routine copes with reset and configuration drivers only.
**/
static int CheckHardware(RmHardwareFacility *hardware, ...)
{  
  if (hardware->Type eq RmH_ConfigureDriver)
   { if (hardware->Name[0] eq '\0')
      Error("missing Mnode statement for CONTROL CONFIGURE");
     elif (hardware->NumberProcessors eq 0)
      Error("configure driver %s does not control any processors", hardware->Name);
     return(0);
   }

  if (hardware->Type eq RmH_ResetDriver)
   { if (hardware->Name[0] eq '\0')
      Error("missing Mnode statement for CONTROL Rst_Anl");
     elif (hardware->NumberProcessors eq 0)
      Error("reset driver %s does not control any processors", hardware->Name);
     return(0);
   }

  return(0);
}

/**
*** This routine copes with the reset commands. When the reset command
*** was encountered in the file two structures were allocated. An
*** RmHardwareFacility was allocated, which will eventually hold all the
*** details. Also, a ResetCommand was allocated which was used to build
*** a list of the processor(s) affected. The facility could not be
*** added to the network immediately, since the processors involved are
*** not yet known. Hence the ResetCommand was added to a list held in
*** the private field of the network.
**/
static void CheckResetCommand(ResetCommand *command)
{ RmHardwareFacility	*hardware = command->hardware;
  RmNetwork		network	  = hardware->Device;
  LinkNode		*node;
  int			i;
      
  hardware->Processors = (RmProcessor *)
		Malloc((word)hardware->NumberProcessors * sizeof(RmProcessor));
  if (hardware->Processors eq Null(RmProcessor)) MemoryError(33);

  for (i = 0; i < hardware->NumberProcessors; i++)
   { node = (LinkNode *) RemHead(&(command->Processors));
     hardware->Processors[i] = RmLookupProcessor(Network, node->Name);
     if (hardware->Processors[i] eq (RmProcessor) NULL)
      Error("reset command (%s), processor %s is not in the network",
      		hardware->Name, node->Name);
     Free(node->Name);
     Free(node);
   }

  if (RmAddHardwareFacility(network, hardware) ne RmE_Success) MemoryError(37);
}

/**
*** ValidateNetwork(). This routine is applied to every processor and
*** subnet in the network, and performs various useful tests.
***  1) check for name clashes, taking care not to give the same error
***     twice by corrupting the name
***  2) for subnets, check all the hardware facilities
***  3) for processors, perform the initial stage of checking the links.
***     This means filling in external links, and looking up processors.
***     Making the actual connections is left until later
**/
static int count_matching_names(RmProcessor processor, ...);

static int ValidateNetwork(RmProcessor processor, ...)
{ int			name_count;
  char			*name;
  ProcessorLinks	*links;
  LinkNode		*node, *next;

  if (RmIsNetwork(processor))
   name = (char *) RmGetNetworkId((RmNetwork) processor);
  else
   name = (char *) RmGetProcessorId(processor);
   
  name_count = RmApplyNetwork(RmParentNetwork(processor), 
   		&count_matching_names, name);
   		
  if (name_count > 1)
   { Error("%d objects with name %s in the same network/subnet /%s",
   	name_count, name, RmGetNetworkId(RmParentNetwork(processor)));
     if (RmIsNetwork(processor))
      RmSetNetworkId((RmNetwork) processor, "AVerySillyName");
     else
      RmSetProcessorId(processor, "AnotherSillyName");
     return(0);
   }
   
  if (RmIsNetwork(processor))
   { List	*list = (List *) RmGetNetworkPrivate((RmNetwork) processor);
     RmApplyHardwareFacilities((RmNetwork) processor, &CheckHardware);
     WalkList(list, (WordFnPtr) &CheckResetCommand);
     return(RmApplyNetwork((RmNetwork) processor, &ValidateNetwork));
   }
   
  links = (ProcessorLinks *) RmGetProcessorPrivate(processor);
  if (links->Count eq 0)
   { Error("processor %s is not connected to the rest of the network", name);
     return(0);
   }

  for (node = Head_(LinkNode, links->List);
       !EndOfList_(node);
       node = next)
   { next = Next_(LinkNode, node);

	/* External links can be handled immediately	*/
     if (node->Target eq RmM_ExternalProcessor)
      { if (RmMakeLink(processor, node->Linkno, RmM_ExternalProcessor, 
      			node->Destlink)	ne RmE_Success)
      	 MemoryError(34);
      	Remove(&(node->Node));
      	Free(node);
      }
     else
      { node->Target = RmLookupProcessor(Network, node->Name);
        if (node->Target eq (RmProcessor) NULL)
         { Error("processor %s, link %d, cannot find connected processor %s",
         		RmGetProcessorId(processor), node->Linkno, node->Name);
           Remove(&(node->Node));
           Free(node->Name);
           Free(node);
         }
        else
         { Free(node->Name);
           node->Name = Null(char);
         }
      }
   }
   
  return(0);
}
  
static int count_matching_names(RmProcessor processor, ...)
{ char		*its_name;
  char		*my_name;
  va_list	args;
  
  va_start(args, processor);
  its_name = va_arg(args, char *);
  va_end(args);
  
  if (RmIsNetwork(processor))
   my_name = (char *) RmGetNetworkId((RmNetwork) processor);
  else
   my_name = (char *) RmGetProcessorId(processor);
   
  if (!strcmp(my_name, its_name))
   return(1);
  else
   return(0);  
}

/**
*** And finally, resolving the links between the processors.
**/
static int ResolveLinks(RmProcessor processor, ...)
{ LinkNode		*node;
  RmProcessor		neighbour;
  int			destlink;
  ProcessorLinks	*links;
  const			char *id;
  int			linkno	= 0;
  
  id = RmGetProcessorId(processor);
     
  links = (ProcessorLinks *) RmGetProcessorPrivate(processor);
  for (node = Head_(LinkNode, links->List);
       !EndOfList_(node);
       node = Next_(LinkNode, node))
   { 
	/* For every link, check whether or not it has been made	*/
	/* already. If a connection has been made already then this	*/
	/* may or may not refer to the right processor.			*/
     linkno = node->Linkno;
     neighbour = RmFollowLink(processor, linkno, &destlink);

     if (neighbour ne RmM_NoProcessor)         
      { if (neighbour eq processor)
         fprintf(stderr, "Warning, processor %s is connected to itself via link %d.\n", id, linkno);
	if (neighbour ne node->Target) 
         Error("processor %s, link %d, is used more than once", id, linkno);
        else
         { if ((node->Destlink ne RmM_AnyLink) && (node->Destlink ne destlink))
            Error("processor %s, link %d, is over-used", id, linkno);
         }
        continue;
      }

     neighbour = node->Target;	/* this is the right neighbour */
     if (neighbour eq processor)
      fprintf(stderr, "Warning, processor %s is connected to itself via link %d.\n", id, linkno);

	/* This link has not yet been allocated. If the resource map	*/
	/* indicated a specific link, use it.				*/
     if (node->Destlink ne RmM_AnyLink)
      { RmProcessor	temp;
        int		templink;
        temp = RmFollowLink(neighbour, node->Destlink, &templink);
        if (temp ne RmM_NoProcessor)
         { Error("processor %s, link %d, connecting processor %s link %d is in use",
         	id, linkno, RmGetProcessorId(neighbour), node->Destlink);
           continue;
         }
        if (RmMakeLink(processor, linkno, neighbour, node->Destlink) ne
        	RmE_Success)
         MemoryError(38);
        continue;
      }

	/* See how many links there are on the neighbour which go	*/
	/* to this processor. If 0 then there is an error. If one then	*/
	/* it is easy. If more than one than it gets complicated.	*/
     { ProcessorLinks 	*itslinks;
       LinkNode	     	*itsnode;
       int		linkcount = 0;
         
       itslinks = (ProcessorLinks *) RmGetProcessorPrivate(neighbour);
       for (itsnode = Head_(LinkNode, itslinks->List);
            !EndOfList_(itsnode);
            itsnode = Next_(LinkNode, itsnode))
        if ((itsnode->Target eq processor) &&
            ( (itsnode->Destlink eq RmM_AnyLink) || (itsnode->Destlink eq linkno)))
         { linkcount++;
           if (linkcount eq 1)
            node->Destlink = itsnode->Linkno;
         } 

        if (linkcount eq 0)
         Error("processor %s, link %d, there is no matching link from %s",
        	id, linkno, RmGetProcessorId(neighbour));
        elif (linkcount eq 1)
         { RmProcessor	again;
           int		anotherlink;
           
           again = RmFollowLink(neighbour, node->Destlink, &anotherlink);
           if (again ne RmM_NoProcessor)
            { Error("processor %s, link %d, no suitable link on %s",
            		id, linkno, RmGetProcessorId(neighbour));
              continue;
            }
           if (RmMakeLink(processor, linkno, neighbour, node->Destlink)
        	ne RmE_Success)
	            MemoryError(39);
         }
        else
         if (RmMakeLink(processor, linkno, neighbour, RmM_AnyLink) ne
        	RmE_Success)
          MemoryError(40);
     }
   }

  if (RmGetProcessorPurpose(processor) ne RmP_IO)
   { int minlink;
     switch(RmGetProcessorType(processor))
      { case	RmT_Default	:
        case	RmT_T800	:
        case	RmT_T414	:
        case	RmT_T425	:
        case	RmT_T212	:
        case	RmT_T9000	: minlink = 3; break;
        case	RmT_T400	: minlink = 1; break;
	case	RmT_C40		: minlink = 5; break;
        default			: minlink = 0; break;
      }    
     if (linkno < minlink)
      (void) RmMakeLink(processor, minlink, RmM_NoProcessor, RmM_AnyLink);
   }
  
  return(0);
}


