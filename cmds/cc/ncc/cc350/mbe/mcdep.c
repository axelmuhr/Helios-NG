/*
  Title:        mcdep.c - miscellaneous target-dependent things.
  Copyright:    (C) 1988, Codemist Ltd
*/

#include <ctype.h>

#ifdef __HELIOS
#include <helios.h>
#include <sem.h>
#include <module.h>
#include <task.h>
#include <syslib.h>
#include <nonansi.h>
#endif

#include "globals.h"
#include "mcdep.h"
#include "mcdpriv.h"

#ifdef __HELIOS
char *
file_name( FILE * f )
{
  return (Heliosno( f )->Name); 
}
#endif

#ifdef TARGET_IS_HELIOS
bool 	in_stubs               = NO;

#ifdef __SMT
int 	split_module_table     = YES;
#else
int 	split_module_table     = NO;
#endif /* SMT */
#endif /* TARGET_IS_HELIOS */

int32 config;


bool
mcdep_config_option(
		    char	name,
		    char	tail[] )
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
  config = CONFIG_HAS_MULTIPLY;

#ifdef TARGET_IS_HELIOS
  suppress_module        = 0;
#endif

  return;
    
} /* config_init */

