/* error.c: ANSI draft (X3J11 May 86) code (various error routines) */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01 */
/* $Id: error.c,v 1.5 1992/12/02 16:45:01 nickc Exp $ */

#include <helios.h>
#include "norcrosys.h"
#include "sysdep.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <signal.h>

#ifndef ESIGNUM
#define ESIGNUM 3  /* Also present in <signal.c> */
#endif

/* from <assert.h> */
void _assert_fail(char *expr, char *file, int line)
{   fprintf(stderr, "*** assertion failed: %s, file %s, line %d\n",
            expr, file, line);
    abort();
}

/* following static moved out from fn to get past Helios/ARM compiler bug */
static char v[80];

/* from <string.h> */
char *
strerror( int n )
{
  switch (n)
    {
    case 0:      return "No error (errno = 0)";
    case EDOM:   return "EDOM - function argument out of range";
    case ERANGE: return "ERANGE - function result not representable";
    case ESIGNUM:return "ESIGNUM - illegal signal number to signal() or raise()";
    default:
      if (n & 0x80000000)
	sprintf(v, "Helios error %x", n);
      else
	sprintf(v, "Posix error %d", n);
      return v;
    }
}

/* from <stdio.h> */
void
perror( const char * s )
{
  if (s != NULL && *s != '\0')
    fprintf( stderr, "%s: ", s );

  fprintf( stderr, "%s\n", strerror( errno ) );
}


void _sysdie(const char *s)
{   char v[200];
    sprintf(v, "*** fatal error in run time system: %s", s); 
/* BLV - used to be %.80s, but this does not work at the moment */
    _sys_msg(v);
    _exit(1);
}


/* end of error.c */
