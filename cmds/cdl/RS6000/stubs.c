/*
 * stubs.c : 	
 *
 *   Copyright (c) 1993 Perihelion Software Ltd.
 *     All rights reserved.
 *
 * Author :	N Clifton
 * Version :	$Revision: 1.1 $
 * Date :	$Date: 1993/04/14 17:19:46 $
 * Id :		$Id: stubs.c,v 1.1 1993/04/14 17:19:46 nickc Exp $
 */


#include <stdio.h>
#include <stdarg.h>


void
IOdebug( const char * format, ... )
{
  va_list args;


  va_start( args, format );

  vfprintf( stderr, format, args );

  va_end( args );

  return;
  
} /* IOdebug */

  
/* end of skeleton.c */

/* @@ emacs customization */

/* Local Variables: */
/* mode: c */
/* outline-regexp: "^[a-zA-Z_]*(" */
/* eval: (outline-minor-mode 1) */
/* End: */
