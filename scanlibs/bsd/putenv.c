/* $Id: putenv.c,v 1.4 1991/09/23 12:30:48 nickc Exp $ */

#include <stdio.h>	/* for NULL */
#include <string.h>	/* for strchr() and strlen() */
#include <stdlib.h>	/* for malloc(), realloc(), and free() */

/*
 * sets the value of var to be arg in the environment vector
 * bindings are of the form "var=value".
 * assumes that the enviroment vector was 'malloc'ed.
 * assumes that it does not need to 'free' replaced entries in the vector. (ie it will leak memory)
 */

void
putenv( register char * variable )
{
  extern char **	environ;
  char **		env;
  register int 		index = 0;
  register char *	value;
  register int 		len;


  /*
   * check to see if we have a string in the correct format
   */
  
  value = strchr( variable, '=' );

  if (value == NULL)
    {
      char * 	var;

      /*
       * left hand side of binding must end in '='
       */
      
      len   = strlen( variable ) + 1;
      var   = (char *)malloc( len + 1 );

      if (var == NULL)
	return;
      
      var[ 0 ] = '\0';
      
      strcpy( var, variable );
      strcat( var, "=" );

      variable = var;
    }
  else
    {
      len = value - variable + 1;

      /*
       * make value point at right hand side of the binding
       */
      
      ++value;
    }

  /*
   * search for a matching entry in environment vector
   */
  
  while (environ[ index ] != NULL)
    {
      if (strncmp( environ[ index ], variable, len ) == 0)
	{
	  /*
	   * found a match - allocate space for a replacement entry
	   *
	   * XXX -
	   * we really ought to try to free the old entry, but
	   * the memory may not be ours
	   */
	  
	  environ[ index ] = (char *) malloc( (unsigned)len + strlen( value ) + 1 );
	  
	  if (environ[ index ] != NULL)
	    {
	      /* insert new value */
	      
	      /* XXX - bug fix applied by NC 23/9/91 */
	      
	      (void) strncpy( environ[ index ], variable, len );

	      if (value != NULL)
		strcpy( environ[ index ] + len, value );
	      else
	        environ[ index ][ len ] = '\0';
	    }
	  else if (environ[ index + 1 ] != NULL)
	    {
	      /*
	       * we cannot leave a NULL entry as this will block off the
	       * rest of the vector
	       */

	      environ[ index ] = "";	/* This is a memory bug */
	    }

	  if (value == NULL)
	    {
	      /*
	       * if we allocated space for a new variable
	       * then we must free it
	       */
	      
	      free( variable );
	    }

	  return;
	}
      
      index ++;
    }

  /* extend environ vector */

  env = (char **)realloc( environ, (index + 2) * sizeof( char * ) );

  if (env != NULL)
    {
      environ = env;

      /* add new entry */
      
      environ[ index ] = (char *) malloc( (unsigned)len + strlen( value ) + 1 );

      if (environ[ index ] != NULL)
	{
	  (void) strcpy( environ[ index ], variable );

	  if (value != NULL)
	    strcat( environ[ index ], value );

	  /* terminate environ vector */
	  
	  environ[ index + 1 ] = NULL;
	}
    }
  
  if (value == NULL)
    free( variable );

  /* finished */
  
  return;
}
