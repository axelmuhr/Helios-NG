
/* stdlib.c: ANSI draft (X3J11 Oct 86) library, section 4.10 */
/* Copyright (C) A.C. Norman and A. Mycroft */
/* version 0.02a */

#include "hostsys.h"      /* for _terminateio(), and _exit()   */
#include <stdlib.h>
#include <signal.h>

/* atof, atoi, atol, strtod, strtol, strtoul are implemented in scanf.c  */

static unsigned long int next = 1;

int rand()
{
/* I do not like this random number generator very much, but it is given */
/* in the ANSI document and for portability and conformance this is what */
/* must be provided.                                                     */
    next = next * 1103515245 + 12345;
    return (unsigned int) ((next >> 16) & 0x7fff);
}

void srand(unsigned int seed)
{
    next = seed;
}

/* free, malloc, realloc etc are in the file alloc.c                     */

#define EXIT_LIMIT 33

static void (*_exitvector[EXIT_LIMIT])(void);
static int number_of_exit_functions;

void _exit_init()
{
    number_of_exit_functions = 0;
}

int atexit(void (*func)(void))
{
    if (number_of_exit_functions >= EXIT_LIMIT) return 1;    /* failure */
    _exitvector[number_of_exit_functions++] = func;
    return 0;                                                /* success */
}

void exit(int n)
{
    while (number_of_exit_functions!=0)
        (*_exitvector[--number_of_exit_functions])();
    _terminateio();
    _exit(n);
}

void abort()
{   raise(SIGABRT);
    exit(1);
}

int abs(int x)
{
    if (x<0) return (-x);
    else return x;
}

long int labs(long int x)
{
    if (x<0) return (-x);
    else return x;
}

div_t div(int numer, int denom)
{
/* This is a candidate for re-implementation in machine code so that the */
/* quotient and remainder can be computed all at once. However I am not  */
/* really convinced about the importance of the function so will not do  */
/* that yet!                                                             */
    div_t res;
    res.quot = numer / denom;
    res.rem  = numer % denom;
    return res;
}

ldiv_t ldiv(long int numer, long int denom)
{
    ldiv_t res;
    res.quot = numer / denom;
    res.rem  = numer % denom;
    return res;
}


/* end of stdlib.c */
