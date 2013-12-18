/**
*
* Title:  Helios Shell - Define header file
*
* Author: Andy England
*
* Date:   May 1988
*
*         (c) CopyRight 1988, Perihelion Software Ltd.
*
*         All Rights Reserved.
*
* $Header: /hsrc/cmds/shell/RCS/define.h,v 1.7 1993/08/12 15:19:52 nickc Exp $
*
**/

#define ignore (void)
#define AND &&
#define OR  ||

#define new(type) (type *)newmemory(sizeof(type))
#define OK 0

#define WORD_MAX    1024
#define LINE_MAX    255
#define VARNAME_MAX 20
#define NUMSTR_MAX  10

#define READ       0
#define WRITE      1
#define DIAGNOSTIC 2

#define lowbyte(w) ((w) & 0377)
#define highbyte(w) lowbyte((w) >> 8)

#define PROMPT "% "

#define SCREEN_WIDTH 80

#define ERR_TOOFEWARGS   0
#define ERR_TOOMANYARGS  1
#define ERR_VARIABLE     2
#define ERR_BADNUMBER    3
#define ERR_NOTFOUND     4
#define ERR_VARSYNTAX    5
#define ERR_SUBSCRIPT    6
#define ERR_EVENT        7
#define ERR_NOTINLOOP    8
#define ERR_THEN         9
#define ERR_EMPTYIF     10
#define ERR_EXPSYNTAX   11
#define ERR_WORDLIST    12
#define ERR_LPAREN      13
#define ERR_RPAREN      14
#define ERR_INVALIDVAR  15
#define ERR_SYNTAX      16
#define ERR_DANGEROUS   17
#define ERR_NOENDIF     18
#define ERR_NOELSE      19
#define ERR_WORDTOOLONG 20
#define ERR_BADPARENS   21
#define ERR_REDIRECT    22
#define ERR_INPUT       23
#define ERR_OUTPUT      24
#define ERR_LIMIT       25
#define ERR_SCALEFACTOR 26
#define ERR_NULLCMD     27
#define ERR_NOMOREWORDS 28
#define ERR_MASK        29
#define ERR_ALIASLOOP   30
#define ERR_SQUOTE      31
#define ERR_BQUOTE      32
#define ERR_DQUOTE      33
#define ERR_USELOGOUT   34
#define ERR_NOFILENAME  35
#define ERR_NOMATCH     36
#define ERR_NOENDSW     37
#define ERR_NOLABEL     38
#define ERR_AUTOLOGOUT  39
#define ERR_NOTLOGIN    40
#define ERR_NOHOME      41
#define ERR_NOEND       42
#define ERR_USEEXIT     43
#define ERR_STACKEMPTY  44
#define ERR_BADDIR      45
#define ERR_NOTTHATDEEP 46
#define ERR_NOOTHERDIR  47
#define ERR_UNKNOWNUSER 48
#define ERR_NOTINCLUDED 49
#define ERR_TERMINATOR  50
#define ERR_NOMEMORY    51
#define ERR_AMBIGUOUS   52
#define ERR_MODIFIER    53
#define ERR_BADSUB      54
#define ERR_NOPREVLHS   55
#define ERR_BADAUX      56
#define ERR_NOCURJOB    57
#define ERR_NOPREVJOB   58
#define ERR_NOSUCHJOB   59
#define ERR_TERMINAL    60
#define ERR_DIAGNOSTIC	61
#define ERR_BADOPTION	62
#define ERR_BADSIGNAL	63

#define CTRL_D    0x04
#define BELL      0x07
#define BACKSPACE 0x08
#define SPACE     0x20
#define DELETE    0x7f
#define CSI       0x9b

#define ARGV_MAX 20
#define MAX_ARGV 230

#define MODE_EXECUTE 0x01
#define MODE_HISTORY 0x02
#define MODE_END     0x04
#define MODE_BREAK   0x08

#define FLAG_STDERR  0x01
#define FLAG_CLOBBER 0x02

#define setmode(f) (mode |= (f))
#define unsetmode(f) (mode &= ~(f))

#define alias(n, w)    setsubnode(&aliaslist, n, w)
#define findalias(n)   findsubnode(&aliaslist, n)
#define findvar(n)     findsubnode(&varlist, n)
#define getvar(n)      getsubnode(&varlist, n)
#define unalias(p)     patremsubnode(&aliaslist, p)
#define putalias()     fputsublist(stdout, &aliaslist, -1, (BOOL)0, (BOOL)0, (BOOL)0)
#define putvars()      fputsublist(stdout, &varlist,   -1, (BOOL)0, (BOOL)0, (BOOL)0)
#define putsublist(s, l, n, p, r) fputsublist(stdout, s, l, n, p, r)
#define putargv(a, n)  fputargv(stdout, a, n); fflush (stdout)

#define readoctal(a, p)   readnumber(a, p, 8)
#define readdecimal(a, p) readnumber(a, p, 10)
#define readhex(a, p)     readnumber(a, p, 16)

#define RESOURCE_MAX 6
#define JOBS_MAX 20
#define HASH_MAX 17

#define CHECKBUILTIN 1
#define NOBUILTIN    0

#ifdef HELIOS
#define MAX_BUILT 61
#else
#define MAX_BUILT 58
#endif
