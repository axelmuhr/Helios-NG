
/* error.c: ANSI draft (X3J11 May 86) code (various error routines) */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.01b */

#include "hostsys.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <signal.h>
#include <assert.h>  /* for _assert_fail() */

#define ESIGNUM 3  /* Also present in <signal.c> */


void _sysdie(const char *s)
{   char v[200];
    sprintf(v, "*** fatal error in run time system: %.80s", s);
    _sys_msg(v);
    exit(1);
}

/* from <assert.h> */
void _assert_fail(char *expr, char *file, int line)
{   fprintf(stderr, "*** assertion failed: %s, file %s, line %d\n",
            expr, file, line);
    abort();
}

/* from <string.h> */
char *strerror(int n)
{   static char v[80];
    switch (n)
    {   case 0:      return "No error (errno = 0)";
        case EDOM:   return "EDOM - function argument out of range";
        case ERANGE: return "ERANGE - function result not representable";
        case ESIGNUM:
            return "ESIGNUM - illegal signal number to signal() or raise()";
        default:
            sprintf(v, "Error code (errno) %d has no associated message", n);
            return v;
    }
}

/* from <stdio.h> */
void perror(const char *s)
{   if (s != 0 && *s != 0) fprintf(stderr, "%s: ", s);
    fprintf(stderr, "%s\n", strerror(errno));
}

/* end of error.c */
