#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double __strtod(const char *nptr, char **endptr)
{ return strtod(nptr,endptr); }

double __atof(const char *nptr) { return atof(nptr); }

extern double __difftime(time_t time1, time_t time0)
{ return difftime(time1,time0); }

