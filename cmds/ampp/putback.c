/****************************************************************/
/* File: putback.c                                              */
/*                                                              */
/* Routines to manage the unreading and rereading of unlimited  */
/* sections of text.                                            */
/*                                                              */
/* Author: NHG 19-Feb-87                                        */
/****************************************************************/
#ifdef __TRAN
static char *RcsId = "$Id: putback.c,v 1.6 1993/08/12 16:45:32 nickc Exp $ Copyright (C) Perihelion Software Ltd.";
#endif
  
#include "ampp.h"
#include <string.h>
  
#define DBPB 0

#define trace if(traceflags&db_putback)_trace

PUBLIC BYTE *pbstack;
PUBLIC BYTE *pbspos;
PUBLIC BYTE *pbsbase;
PRIVATE INT pbssize = 50000;

PUBLIC FILE *infd;

PUBLIC void initpb()
{
#ifdef MWC
   pbstack = (BYTE *)lmalloc(pbssize);
#else
#ifdef IBMPC
   pbstack = (BYTE *)malloc((int)pbssize);
#endif
   pbstack = (BYTE *)malloc((int)pbssize);
#endif

   if( pbstack == NULL ) error("Cannot allocate put back stack");
   pbspos = pbsbase = pbstack+pbssize;
/*_trace("pbstack: %x %x %x",pbstack,pbsbase,pbspos);*/
}

/********************************************************/
/* pbchar                                               */
/* pbstr                                                */
/*                                                      */
/* Unread a single character, and a string              */
/*                                                      */
/********************************************************/

PUBLIC void pbchar(UBYTE ch)
{
#if DBPB
	trace("put back %8x %8x %2x '%c'",pbsbase,pbspos,
		(WORD)ch,(UBYTE)(' '<=ch&&ch<='~'?ch:'.'));
#endif
        if( pbspos == pbstack ) 
        {	
	        _trace("1 pbstack: %x %x %x",pbstack,pbsbase,pbspos);
        	error("Put back stack full");
	}
        *(--pbspos) = ch;
}

PUBLIC void pbstr(BYTE *s)
{
        int size = strlen(s);
	pbspos -= size;        
        if( pbspos <= pbstack ) 
        {
        	_trace("2 pbstack: %x %x %x",pbstack,pbsbase,pbspos);
        	error("Put back stack full");
        }
#if DBPB
	trace("put back string '%s'",s);
#endif
	memcpy(pbspos,s,size);
}

/********************************************************/
/* savepb                                               */
/* setpb                                                */
/*                                                      */
/* Routines to save and restore the putback buffer.     */
/* Used to save state during includes.                  */
/********************************************************/

PUBLIC BYTE *savepb()
{
        BYTE *res = pbsbase;
        pbsbase = pbspos;
#if DBPB
        trace("Savepb %8x %8x",res,pbsbase);
#endif
        return res;
}

PUBLIC void setpb(BYTE *newpbsbase)
{
#if DBPB
        trace("Setpb %8x",newpbsbase);
#endif
        pbsbase = newpbsbase;
}


PUBLIC INT
get_char( FILE * fd )
{
  static int	saw_nl = 0;
  INT		ch;

  
  ch = getc( fd );

  if (saw_nl)    
    {
      saw_nl = 0;
      
      ++in_line;
    }
  
  if (ch == '\n')
    saw_nl = 1;
  
  return ch;
  
} /* get_char */


/********************************************************/
/* pbrdch                                               */
/*                                                      */
/* Read a character from the put-back buffer, or from   */
/* the standard input if it is empty                    */
/*                                                      */
/********************************************************/

PUBLIC INT pbrdch()
{
        INT ch;
        if( pbspos >= pbsbase )
	  ch = get_char(infd);
        else
	  ch = *(pbspos++);

#if DBPB
        trace("rdch %8x %8x %2x '%c'",pbsbase,pbspos,
		(WORD)ch,(UBYTE)(' '<=ch&&ch<='~'?ch:'.'));
#endif
        return ch;
}

/********************************************************/
/* pbdef                                                */
/*                                                      */
/* given a charbuf containing some text, this routine   */
/* simply pushes it back, backwards, onto the putback   */
/* stack.                                               */
/*                                                      */
/********************************************************/

PUBLIC void pbdef(struct List *def)
{
        struct Charbuf *buf = (struct Charbuf *)(def->Head);

        while( buf->node.Next != NULL )
        {
                BYTE *s = buf->text;
		WORD size = buf->size;
		
                if( buf->size < 0 || buf->size > 1000000 )
                {
                	_trace("def = %x buf = %x buf->size = %x",def,buf,buf->size);
                	error("Invalid def buffer");
                }
                
                pbspos -= size;
                
                if( pbspos == pbstack ) 
                {
                	_trace("def = %x buf = %x buf->size = %x",def,buf,buf->size);
                	_trace("3 pbstack: %x %x %x",pbstack,pbsbase,pbspos);
                	error("Put back stack full");
                }

		memcpy(pbspos,s,(int)size);
		
                buf = (struct Charbuf *)(buf->node.Next);
        }
}

/********************************************************/
/* pbnum                                                */
/*                                                      */
/* push back a numerical value                          */
/*                                                      */
/********************************************************/

PUBLIC void pbnum(INT n)
{
        INT m = n<0?-n:n;
        do {
                pbchar((UBYTE)('0' + (m%10)));
                m = m/10;
        } while ( m != 0 );
        if( n < 0 ) pbchar(c_minus);
}

/********************************************************/
/* pbhex                                                */
/*                                                      */
/* push back a hexadecimal value preceeded by a #       */
/*                                                      */
/********************************************************/

PUBLIC void pbhex(INT n)
{
        do {
                INT d = n & 0xF;
                n = (n >> 4) & 0x0FFFFFFFL;
                pbchar((UBYTE)(d<10 ? d+'0' : d-10+'A'));
        }
        while( n != 0 );
        pbchar(c_hash);
}
