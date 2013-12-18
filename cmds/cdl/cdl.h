/**
*
* Title:  CDL Compiler - Header File
*
* Author: Andy England
*
* Date:   June 1988
*
*         (C) Copyright 1988, Perihelion Software Ltd.
*
*         All Rights Reserverd.
*
* $Header: /hsrc/cmds/cdl/RCS/cdl.h,v 1.7 1993/08/02 12:33:01 nickc Exp $
*
**/

#ifdef __SUN4
#include </hsrc/include/memory.h>
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#endif

#ifdef RS6000
#include </hsrc/include/memory.h>
#endif

#include <syslib.h>
#include <stdlib.h>
#include <nonansi.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define NODE Node
#define LIST List

#define new(t) (t *)newmem(sizeof(t))

#define O_External 0x80000000
#define Mask_Fifo  0xFFFFFF00
#define AND &&
#define OR  ||
#define strequ(s,t) (strcmp(s,t) == 0)
#define strnequ(s,t,n) (strncmp(s,t,n) == 0)
#define isabspath(p) ((p)[0] == '/')
#define READ  0
#define WRITE 1
#define STDCHAN ((CHANNEL *)-1)

#define DEBUG(m) if (debugging) IOdebug("DEBUG: %s", m);

#define WORD_MAX 1024
#ifdef PATH_MAX
#undef PATH_MAX
#endif
#define PATH_MAX 1024

#define ARGV_MAX 20
#define NUMSTR_MAX 10
#define CHAN_DEFAULT 16
#define BIND_MAX 10

typedef word BOOL;
typedef char **ARGV;

typedef enum
{
  T_NULL,
  T_EOF,
  T_WORD,
  T_SEMICOLON,
  T_AMPERSAND,
  T_AND,
  T_OR,
  T_LBRACE,
  T_RBRACE,
  T_PAR,
  T_PIPE,
  T_REVPIPE,
  T_INTERLEAVE,
  T_SUBORDINATE,
  T_REPLICATOR,
  T_COMPONENT,
  T_PTYPE,
  T_PUID,
  T_ATTRIB,
  T_MEMORY,
  T_LIFE,
  T_TIME,
  T_PRIORITY,
  T_STREAMS,
  T_CODE,
  T_ARGV,
  T_COMMA,
  T_LPAREN,
  T_RPAREN,
  T_LBRACKET,
  T_RBRACKET,
  T_READ,
  T_WRITE,
  T_APPEND,
  T_READFIFO,
  T_WRITEFIFO,
  T_SIMPLE,
  T_LIST
} TOKEN;

#define T_READWRITE T_SUBORDINATE

typedef enum
{
  MORTAL,
  IMMORTAL
} LIFE;

typedef enum
{
  ANY_PROCESSOR,
  T212,
  T414,
  T800
} PTYPE;

typedef struct attrib
{
  struct attrib *next;
  struct attrib *prev;
  char *name;
  int count;
} ATTRIB;

typedef struct fifo
{
  struct fifo *next;
  struct fifo *prev;
  char *name;
  int flags;
  int index;
  int usage[2];
} FIFO;

typedef struct channel
{
  struct channel *next;
  struct channel *prev;
  char *name;
  FIFO *fifo;
  int mode;
  int fd;
  ARGV subv;
} CHANNEL;

typedef struct CHANLIST {
	int	size;
	CHANNEL	**channels;
} CHANLIST, *CHANV;

/*typedef CHANNEL **CHANV;*/

typedef struct
{
  char *name;
  int value;
} BINDING;

typedef BINDING *BINDV;

typedef struct cmd
{
  TOKEN op;
  struct cmd *this;
  struct cmd *next;
} CMD;

typedef struct
{
  TOKEN op;
  TOKEN repop;
  CMD *cmd;
  int dim;
  BINDING *bindv;
} REPLICATOR;

typedef struct component
{
  struct component *next;
  struct component *prev;
  char *name;
  char *path;
  ARGV argv;
  PTYPE ptype;
  char *puid;
  int attribcount;
  LIST attriblist;
  unsigned long memory;
  LIFE life;
  int time;
  int priority;
  CHANV chanv;
  ARGV subv;
} COMPONENT;

typedef struct
{
  TOKEN op;
  COMPONENT *component;
  CMD *aux;
  ARGV argv;
  ARGV subv;
  CHANV chanv;
} SIMPLE;

typedef struct
{
  char *name;
  TOKEN token;
} KEYWORD;

int 		cdl(void);
void		initialise(int, char *[]);
int		exectaskforce(void);
CMD *		parse(void);
void		readdeclaration(void);
void		readdefinition(void);
void		readattribute(COMPONENT *);
PTYPE		readptype(void);
LIFE		readlife(void);
void		readattriblist(COMPONENT *);
void		readattrib(COMPONENT *);
void		readstreamlist(COMPONENT *);
int		readmode(void);
void		readpath(COMPONENT *);
void		readargv(COMPONENT *);
CMD *		readcmd(void);
CMD *		readcmdlist(int);
CMD *		readtaskforce(void);
CMD *		readinterleave(void);
CMD *		readpipeline(void);
CMD *		readsubordinate(void);
CMD *		readauxlist(void);
CMD *		readaux(void);
CMD *		readsimplecmd(void);
REPLICATOR *	readreplicator(void);
void		listop(TOKEN, FILE *);
BOOL		isconstructor(TOKEN);
void		allocchannels(CMD *);
int		allocauxlist(CMD *, CMD *, int);
int		allocaux(CMD *, CMD *, int);
void		addstream(CMD *, int, CHANNEL *);
char *		locatecmd(char *);
void		putlisting(CMD *, FILE *);
void		listcmd(CMD *, FILE *);
void		listaux(CMD *, FILE *);
int		putcode(FILE *);
void		putheader(FILE *);
void		putstringv(FILE *);
word		putfifo(FIFO *, FILE *);
void		tidyup(void);
void		fatal(char *, ...);
void		error(char *, ...);
void		synerr(char *, ...);
void		warning(char *, ...);
void		bug(char *);
void		signon(void);
void		signoff(void);
char *		readname(void);
int		readnumber(void);
BOOL		checkfor(TOKEN);
BOOL		punctuation(TOKEN);
void		ignore(TOKEN);
void		nexttoken(void);
void		nextch(void);
void		newmacro(char *);
void		readmacroch(void);
int		putstring(char *);
BINDING *	newbindv(void);
REPLICATOR *	newreplicator(int, BINDING *);
CMD *		newcmd(TOKEN, CMD *, CMD *);
void		freecmd(CMD *);
SIMPLE *	newsimple(ARGV, ARGV, CHANV);
void		freesimple(SIMPLE *);
char *		inventname(void);
FIFO *		findfifo(char *);
FIFO *		newfifo(char *);
FIFO *		usefifo(char *, int);
word		freefifo(FIFO *);
word		freeattrib(ATTRIB *);
void		addattrib(COMPONENT *, char *, int);
CHANNEL *	newchannel(char *, ARGV, int);
void		freechannel(CHANNEL *);
void		addchannel(CHANV, int, CHANNEL *channel);
void		addpuid(COMPONENT *, char *);
void		addpath(COMPONENT *, char *);
void		addargv(COMPONENT *, ARGV);
void		addchanv(COMPONENT *, CHANV);
KEYWORD *	findkeyword(char *);
void		initdata(void);
word		listcomponent(COMPONENT *, FILE *);
word		putcomponent(COMPONENT *, FILE *);
word		putobject(COMPONENT *, FILE *);
word		putattribs(COMPONENT *, FILE *);
word		putattrib(ATTRIB *, FILE *);
word		listattrib(ATTRIB *, FILE *);
void		listptype(PTYPE, FILE *);
void		listlife(LIFE, FILE *);
word		putchannels(COMPONENT *, FILE *);
void		putchannel(FILE *, int, CHANNEL *);
void		listargv(ARGV, FILE *);
void		listchannel(FILE *, int, CHANNEL *);
COMPONENT *	newcomponent(char *);
word		freecomponent(COMPONENT *);
COMPONENT *	usecomponent(ARGV, int *, CHANV);
COMPONENT *	newtemplate(char *);
COMPONENT *	findtemplate(char *);
ARGV		nullargv(void);
ARGV	  	addword(ARGV, char *);
ARGV		dupargv(ARGV);
int		lenargv(ARGV);
void		freeargv(ARGV);
char *		strdup(char *);
void *		newmem(int);
void		freemem(void *);
CMD *		buildtaskforce(CMD *);
word		countobjects(COMPONENT *);
word		checkfifo(FIFO *);
int		bindnames(ARGV subnames, int *subvals);
void		bindname(char *name, int value);
void		unbindnames(int n);
void		unbind(void);
ARGV		expandargv(ARGV);
CHANV		expandchanv(CHANV);
CHANV		newchanv(void);
void		addsubnames(COMPONENT *component, ARGV subv);
int		getstd(int mode);
CMD *		expandcmd(CMD *cmd);
PUBLIC int	evalexpr(char *expr);


extern BOOL	compiling;
extern BOOL	debugging;
extern BOOL	expanding;
extern BOOL	listing;
extern BOOL	locating;
extern BOOL	memorycheck;
extern BOOL	parseonly;
extern int	ch;
extern TOKEN	token;
extern int	linenumber;
extern char *	filename;
extern FILE *	inputfile;
/*extern FILE *	outputfile;*/
extern FILE *	listingfile;
extern LIST	componentlist;
extern LIST	fifolist;
extern LIST	attriblist;
extern ARGV	stringv;
extern int	componentcount;
extern int	attribcount;
extern int	fifocount;
extern int	channelcount;
extern int	stringindex;
extern char **	arguments;
extern int	argcount;

extern void	report(char *format, ...);

#ifdef SHOWMEM
#define Memory(a) report("memory allocation, file %s, line %d, address %p", __FILE__, __LINE__, a)
#else
#define Memory(a)
#endif

