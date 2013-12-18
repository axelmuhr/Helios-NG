/*
 *	Include header for make
 *
 * $Header: /hsrc/cmds/make/RCS/h.h,v 1.8 1994/03/08 16:10:00 nickc Exp $
 *
 */

#ifdef __HELIOS
#define unlink remove
#define index strchr
#define rindex strrchr

#include <stdlib.h>
#include <string.h>
#endif

#ifndef uchar
#ifdef os9
#define uchar		char
#define void		int
#define fputc		putc
#else
#ifdef orion
#define void		int
#endif
#define uchar		unsigned char
#endif
#endif

#define bool		uchar
#define time_t		long
#ifndef TRUE
#define TRUE		(1)
#define FALSE		(0)
#endif
#define max(a,b)	((a)>(b)?(a):(b))

#define DEFN1		"makefile"	/* Default names  */
#ifdef unix
#define DEFN2		"Makefile"
#endif
#ifdef eon
#define DEFN2		"Makefile"
#endif
/* os9 is case insensitive */
/* helios is case insensitive */

/* BLV - static buffers of size LZ are used to hold $? and $^ expansions.
   so this number must be quite large.
*/   
#define LZ		(4096)	/* Line size  */



/*
 *	A name.  This represents a file, either to be made, or existant
 */

struct name {
    struct name    *n_next;	/* Next in the list of names */
    char           *n_name;	/* Called */
    struct line    *n_line;	/* Dependencies */
    time_t          n_time;	/* Modify time of this name */
    uchar           n_flag;	/* Info about the name */
};

#define N_MARK		0x01	/* For cycle check */
#define N_DONE		0x02	/* Name looked at */
#define N_TARG		0x04	/* Name is a target */
#define N_PREC		0x08	/* Target is precious */
#define N_DOUBLE	0x10	/* Double colon target */
#define N_LT_DEFINED	0x20	/* $< has been defined	*/

/*
 *	Definition of a target line.
 */
struct line {
    struct line    *l_next;	/* Next line (for ::) */
    struct depend  *l_dep;	/* Dependents for this line */
    struct cmd     *l_cmd;	/* Commands for this line */
};


/*
 *	List of dependents for a line
 */
struct depend {
    struct depend  *d_next;	/* Next dependent */
    struct name    *d_name;	/* Name of dependent */
};


/*
 *	Commands for a line
 */
struct cmd {
    struct cmd     *c_next;	/* Next command line */
    char           *c_cmd;	/* Command line */
};


/*
 *	Macro storage
 */
struct macro {
    struct macro   *m_next;	/* Next variable */
    char           *m_name;	/* Called ... */
    char           *m_val;	/* Its value */
    uchar           m_flag;	/* Infinite loop check */
};

extern char    *myname;
extern struct name namehead;
extern struct macro *macrohead;
extern struct name *firstname;
extern bool     silent;
extern bool     ignore;
extern bool     rules;
extern bool     dotouch;
extern bool     quest;
extern bool     domake;
extern char     str1[LZ];
extern char     str2[LZ];
extern int      lineno;

char           *fgets();
char           *index();
char           *rindex();
#ifndef __HELIOS
char           *malloc();
extern int      errno;
#endif

#ifdef __STDC__
#include <stdarg.h>
#endif

char           *getmacro();
struct macro   *setmacro();
void            input();
#ifdef __STDC__
void            fatal( char * msg, ... );
void            error( char * msg, ... );
#else
void            error();
void            fatal();
#endif
int             make();
struct name    *newname();
struct depend  *newdep();
struct cmd     *newcmd();
void            newline();
char           *suffix();
void            touch();
void            makerules();
char           *gettok();
void            precious();
