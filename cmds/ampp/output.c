/****************************************************************/
/* File: output.c                                               */
/*                                                              */
/* Routines to manage the output of characters either to the    */
/* standard output or to a macro definition buffer.             */
/*                                                              */
/* Author: NHG 19-Feb-87                                        */
/****************************************************************/
#ifdef __TRAN
static char RcsId[] = "$Id: output.c,v 1.5 1993/08/12 16:57:00 nickc Exp $ Copyright (C) Perihelion Software Ltd.";
#endif
  
#include "ampp.h"

PUBLIC struct List *outbuf = NULL;
PUBLIC FILE *outfd = NULL;

#define LINEBUF_SIZE 128
PRIVATE BYTE linebuf[ LINEBUF_SIZE + 2 ];
PRIVATE INT linepos = 0;
PRIVATE INT lineempty = TRUE;

/********************************************************/
/* wrch                                                 */
/* wrstr                                                */
/*                                                      */
/* Write out a character and a string.                  */
/*                                                      */
/********************************************************/

PUBLIC void wrch(UBYTE ch )
{
  if ( outbuf == NULL )
    {
      /* This is a bit kludgy, but under certain circumstances */
      /* brackets will find themselves in the output           */

      if ( ch == c_lbra || ch == c_rbra ) return;
      
      linebuf[ linepos++ ] = ch & 0x7f;

      if ( ch == c_nl )
	{
	  if ( !lineempty )
	    {
	      linebuf[ linepos ] = '\0';
	      fputs( linebuf, outfd );
	      lineempty = TRUE;
	    }
	  linepos = 0;
	}
      else if ( ch != c_space && ch != c_tab ) lineempty = FALSE;

      if (linepos == LINEBUF_SIZE)
	{
	  linebuf[ linepos ] = '\0';
	  
	  fputs( linebuf, outfd );
	  
	  lineempty = TRUE;
	  linepos   = 0;	  
	}      
    }
#if 0
  /* '\' escapes characters only on first pass */
  else addch(outbuf,ch & 0x7f);
#else
  /* Permanently '\' escape characters. Macro definitions will output */
  /* expected text - PAB Jan '93. */
  else addch(outbuf, ch);
#endif
  
  /*
   * top bit stripping added by NC 4/11/91
   * top bit is used by lex to indicate a non-parseable character
   */
}

PUBLIC void wrstr(BYTE *s)
{
  while( *s )
    {
      wrch(*s++);
    }
}

/********************************************************/
/* wrnum                                                */
/*                                                      */
/* Write out a decimal number.                          */
/*                                                      */
/********************************************************/

PRIVATE void wrnum1(INT n )
{
        if( n == 0 ) return;
        wrnum1(n/10);
        wrch('0'+((UBYTE)n%10));
}

PUBLIC void wrnum(INT n)
{
        if( n < 0 ) wrch(c_minus);
        if( n == 0 ) wrch('0');
        else wrnum1(abs((int)n));
       
}

/********************************************************/
/* wrhex                                                */
/*                                                      */
/* Write out a hexadecimal number.                      */
/*                                                      */
/********************************************************/

PRIVATE void wrhex1(INT n )
{
        static char *hextab = "0123456789ABCDEF";
        if( n == 0 ) return;
        wrhex1((n>>4) & 0x0FFFFFFFL);
        wrch(hextab[n & 0xF]);
}

PUBLIC void wrhex(INT n)
{
        if( n == 0 ) wrch('0');
        else wrhex1(n);
}

