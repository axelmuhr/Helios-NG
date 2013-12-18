/*
 * mcdep.c - miscellaneous target-dependent things.
 * Copyright (C) Codemist Ltd., 1988.
 */

#ifndef NO_VERSION_STRINGS
extern char mcdep_version[];
char mcdep_version[] = "\nm68k/mcdep.c $Revision: 1.2 $\n";
#endif

#include <ctype.h>

#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"

int32 config;
#ifdef __SMT
int 	split_module_table     = YES;
#else
int 	split_module_table     = NO;
#endif /* SMT */

bool mcdep_config_option(char name, char tail[])
{
  switch (name)
    {
#ifdef TARGET_IS_HELIOS
    case 'r':
    case 'R': suppress_module = 1;        return YES;
    case 'l': 
    case 'L': suppress_module = 2;        return YES;
    case 's':
    case 'S': split_module_table = 0;     return YES;
#endif
    default:
      return NO;
    }
}

void config_init(void)
{
#ifdef TARGET_IS_68020
    config = CONFIG_HAS_MULTIPLY;
#else
    config = 0;
#endif
#ifdef TARGET_IS_HELIOS
    suppress_module = 0;
#endif
}

KW_Status mcdep_keyword(const char *key, int *argp, char **argv) {
    return KW_NONE;
}

/* end of m68k/mcdep.c */
