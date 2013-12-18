/*
 * prgname.h - Extract program name from argv[0], system-independently.
 * Copyright (C) Advanced RISC Machines Ltd., 1991.
 */

#ifndef __prgname_h
#define __prgname_h

char *program_name(char *arg, char *result, int reslen);
/*
 * Extract the program name from the first argument and copy it
 * into the result buffer, returning a pointer to teh result.
 */

#endif

