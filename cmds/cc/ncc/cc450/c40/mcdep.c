/*
  Title:        mcdep.c - miscellaneous target-dependent things.
  Copyright:    (C) 1988, Codemist Ltd
*/

#include <ctype.h>

#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"


bool 	in_stubs               = NO;
#ifdef __SMT
int 	split_module_table     = YES;
#else
int 	split_module_table     = NO;
#endif /* SMT */

int32   config;


bool
mcdep_config_option(
		    char	name,
		    char	tail[] )
{
  switch (name)
    {
    case 'r':
    case 'R': suppress_module = 1;        return YES;
    case 'l': 
    case 'L': suppress_module = 2;        return YES;
    case 's':
    case 'S': split_module_table = 0;     return YES;
    default:
      return NO;
    }

  use( tail );  

} /* mcdep_config_option */


/*************************************************************/
/*                                                           */
/*       Code to configure compiler for host system          */
/*                                                           */
/*************************************************************/


void
config_init( void )
{
  config          = CONFIG_HAS_MULTIPLY
#ifdef TARGET_FP_ARGS_IN_FP_REGS
                    | CONFIG_FPREGARGS
#endif
                    ;
  suppress_module = 0;

  return;
    
} /* config_init */

KW_Status
mcdep_keyword(
	      const char *	key,
	      int *		argp,
	      char **		argv)
{
  return KW_NONE;
}

