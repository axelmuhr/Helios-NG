/*
 * Author:      Alex Schuilenburg
 * Date:        28 July 1994
 *
 * Copyright 1994 Perihelion Distributed Software Limited
 *
 * $Id: getargs.h,v 1.1 1994/08/03 10:47:31 al Exp $
 */

/* The arguments stack */
typedef struct _ArgStack ArgStack;
struct _ArgStack
  {
    char **	argv;
    int         argc;
    ArgStack *	next;
  };


/* Prototypes for the getargs and freeargs */
#ifdef __STDC__
char ** getargs( char * name, int * rargc );
void    freeargs( void );
void    popargs(  ArgStack ** argstack, int * argc, char *** argv );
void    pushargs( ArgStack ** argstack, int   argc, char  ** argv );
#else
char ** getargs();
void    freeargs();
void    popargs();
void    pushargs()
#endif


