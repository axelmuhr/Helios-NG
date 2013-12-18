/*
 * prgname.c - Extract program name from argv[0], system-independently.
 * Copyright (C) Advanced RISC Machines Ltd., 1991.
 */

#include <stdlib.h>
#include <string.h>
#include "prgname.h"

char *program_name(char *arg, char *result, int reslen)
{
    int len, idx, ch;
    char *endptr;

    if (result == NULL || reslen < 1) return arg;

    len = strlen(arg);
    endptr = arg + len;
    if (len > 4)
    {   if (strcmp(endptr-4, ".exe") == 0 || strcmp(endptr-4, ".EXE") == 0)
            endptr -= 4, len -=4;
    }

    for (idx = len - 2;  idx >= 0;  --idx)
    {    ch = arg[idx];
         /* The following works for DOS, Unix and Acorn commands... */
         if (ch == '\\' || ch == '/' || ch == ':' || ch == '.')
         {   arg += idx + 1;
             break;
         }
    }

    len = endptr - arg;
    /* Assert: relen >= 1 */
    if (len > reslen-1) len = reslen-1;
    if (len > 0) (void) strncpy(result, arg, len);
    result[len] = 0;
    return result;
}
