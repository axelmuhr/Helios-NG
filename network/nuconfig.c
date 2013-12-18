/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : get_config						--
--									--
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId:$Header: /hsrc/network/RCS/nuconfig.c,v 1.5 1993/08/12 11:32:06 nickc Exp $*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "private.h"
#include "netutils.h"

/**
*** String comparison routine which is not case sensitive. It returns the
*** same result as strcmp, i.e. 0 for identical strings
**/
int mystrcmp(char *ms1, char *ms2)
{ char *s1 = ms1;
  char *s2 = ms2; 
#define ToUpper(x) (islower(x) ? toupper(x) : x)
  
  for (;;)
   { if (*s1 eq '\0')
       return((*s2 eq '\0') ? 0 : -1);
     elif (*s2 eq '\0')
       return(1);
     elif(ToUpper(*s1) < ToUpper(*s2))
       return(-1);
     elif(ToUpper(*s1) > ToUpper(*s2))
       return(1);
     else
       { s1++; s2++; }
   }
}

/**
*** A routine to extract options from the nsrc file. Usually the
*** startns program reads in the file, stores the strings in a table,
*** and sends this table to another program such as the Network Server
*** as the environment strings. All nsrc options are prefixed by the ~
*** character.
**/
char *get_config(char *pattern, char **environ)
{ int pattern_len = strlen(pattern);

  for ( ; *environ ne Null(char); environ++)
   if (**environ eq '~')
    { char *target = &((*environ)[1]);
      char temp; int match;

      if (strlen(target) < pattern_len) continue;  /* definitely wrong one */

      temp = target[pattern_len];	/* try to match */
      target[pattern_len] = '\0';
      match = !mystrcmp(target, pattern);
      target[pattern_len] = temp;	/* restore the string */

      unless(match) continue;		/* different strings */

      target = &(target[pattern_len]);	/* partial match achieved */
      while (isspace(*target)) target++;

      if (*target eq '\0')    /* just the string */
       return(target);
      if (*target ne '=')    /* matching failure */
       continue;	     /* e.g. pattern "xx", target "xxy" or "xx y" */
      target++;
      while (isspace(*target)) target++;
      return(target);
    }

  return(Null(char));      
}

int get_int_config(char *string, char **environ)
{ char *result = get_config(string, environ);
  char *end;
  word  value;
  
  if (result eq Null(char)) return(Invalid_config);
  if (*result eq '\0') return(Invalid_config);
  value = strtol(result, &end, 0);
  while(isspace(*end)) end++;	/* must be at end of line to be valid */
  if (*end ne '\0') return(Invalid_config);
  return((int)value);
}
