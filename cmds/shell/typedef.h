/**
*
* Title:  Helios Shell - Typedef header file
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) CopyRight 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/typedef.h,v 1.3 1993/08/12 15:56:14 nickc Exp $
*
**/

#ifndef __typedef_h
#define __typedef_h

typedef char **ARGV;

typedef enum
{
  NEUTRAL,
  INWORD,
  INSQUOTE,
  INBQUOTE,
  INDQUOTE
} SUBSTATE;

typedef enum
{
  NOTFOUND,
  COMPLETE,
  INCOMPLETE
} COMPSTATE;

typedef enum
{
  NOPATTERN,
  NOMATCH,
  MATCH
} GLOBSTATE;

typedef enum
{
  T_EOF,
  T_RPAREN,
  T_AMPERSAND,
  T_OR,
  T_AND,
  T_SEMICOLON,
  T_PIPE,
  T_BITOR,
  T_BITXOR,
  T_BITAND,
  T_EQ,
  T_NE,
  T_MATCH,
  T_NOMATCH,
  T_LE,
  T_GE,
  T_LT,
  T_GT,
  T_LSHIFT,
  T_RSHIFT,
  T_PLUS,
  T_MINUS,
  T_TIMES,
  T_DIVIDE,
  T_REM,
  T_NOT,
  T_ONECOMP,
  T_LPAREN,
  T_LIST,
  T_READ,
  T_SHELLREAD,
  T_WRITE,
  T_APPEND,
  T_WRITEDIAG,				/* CFL	additions for		*/
  T_APPENDDIAG,				/*	stderr redirection	*/
  T_NEWLINE,
  T_WORD,
  T_SIMPLE
#ifdef CDL
  ,T_REVPIPE,
  T_SUBORDINATE,
  T_FARM,
  T_COMMA,
  T_PAR,
  T_READFIFO,
  T_WRITEFIFO,
  T_LBRACKET,
  T_RBRACKET
#endif
} TOKEN, MODE;
#ifdef CDL
typedef TOKEN CONSTRUCTOR;
#endif

typedef struct expr
{
  TOKEN op;
  struct expr *left;
  struct expr *right;
} EXPR;

typedef struct openinfo
{
  TOKEN op;
  char *name;
  long flags;
} OPENINFO;

typedef struct ioinfo
{
  OPENINFO *input;
  OPENINFO *output;
  OPENINFO *diag;
} IOINFO;

typedef struct cmd
{
  TOKEN op;
  struct cmd *This;
  struct cmd *next;
  IOINFO *ioinfo;
} CMD;

typedef struct subnode
{
  struct subnode *next;
  struct subnode *prev;
  ARGV argv;
  char *name;
} SUBNODE;

typedef struct builtin
{
  char *name;
  int (*func)();
} BUILTIN;

typedef struct dirnode
{
  struct dirnode *next;
  struct dirnode *prev;
  char *name;
} DIRNODE;

typedef struct fileinfo
{
  struct fileinfo *next;
  struct fileinfo *prev;
  FILE *inputfile;
  BOOL interactive;
  CMD *cmd;
  ARGV argv;
} FILEINFO;

typedef struct loopinfo
{
  struct loopinfo *next;
  struct loopinfo *prev;
  CMD *cmd;
} LOOPINFO;

typedef struct resource
{
  char *name;
  int value;
} RESOURCE;

typedef struct process
{
  struct process *next;
  struct process *prev;
  int pid;
  int status;
  ARGV argv;
} PROCESS;

typedef struct job
{
  struct job *next;
  struct job *prev;
  int pid;
  int status;
  BOOL notify;
  BOOL pending;
  CMD *cmd;
} JOB;

typedef struct entry
{
  struct entry *next;
  struct entry *prev;
  char *name;
} ENTRY;

typedef struct directory
{
  struct directory *next;
  struct directory *prev;
  char *name;
  LIST hashtable[HASH_MAX];
} DIRECTORY;

typedef struct expansion
{
  struct expansion *next;
  struct expansion *prev;
  char *name;
  ARGV argv;
  char **wordlist;
  BOOL lastword;
} EXPANSION;

#endif /* __typedef_h */
