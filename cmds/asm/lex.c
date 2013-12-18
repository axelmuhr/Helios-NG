/****************************************************************/
/* File: lex.c                                                  */
/*                                                              */
/* Lexical analyser                                             */
/*                                                              */
/* Author: NHG 19-Feb-87                                        */
/****************************************************************/
/* $Id: lex.c,v 1.5 1994/08/09 16:43:25 al Exp $ */

 
#include "asm.h"
#include <ctype.h>
#include "hash.c"

#define trace if(traceflags&db_lex)_trace
#ifndef isascii
#define isascii(ch) ( ((ch < 0x80) && (ch >= 0)) ? TRUE : FALSE )
#endif

PUBLIC INT symb;
PUBLIC BYTE token[maxtok+16]; 	/* +16 for safety */
PUBLIC Symbol *toksym;
PUBLIC VMRef toksymref;
PUBLIC struct keyentry *tokkey;
PUBLIC INT toksize;
PUBLIC Value tokval;
PUBLIC WORD tempval = 0;

#ifdef __DOS386

PRIVATE struct keyentry *keylookup(char *mode);
WORD readnumber(WORD ch);
static void tokerror(void);
int rdcharnum(int radix, int ch);

#else /* !__DOS386 */

PRIVATE struct keyentry *keylookup(ellipsis);
WORD readnumber(ellipsis);
static void tokerror(ellipsis);
int rdcharnum(ellipsis);

#endif
/********************************************************/
/* nextsym                                              */
/*                                                      */
/* The next token in the input stream is analysed and   */
/* its type returned in symb. The text of the token is  */
/* in token.                                            */
/*                                                      */
/********************************************************/

PUBLIC void
nextsym()
{
  INT ch;
  

  ch = ch;
  
  /* first read & ignore leading white space */
  
  do
    {
      ch = asm_rdch();
    }
  while ( ch == c_space || ch == c_tab );
  
  trace( "1ch = %02lx",ch );
  
  toksize = 0;
  
  if ( isascii( ch ) && (isalpha( ch ) || ch == '_' || ch == c_dot) )
    {
      /* possible first char of a token */
      
      do
	{
	  token[ toksize++ ] = ch;
	  
	  if ( toksize >= maxtok )
	    tokerror();
	  
	  ch = asm_rdch();
	}
      while( isascii( ch ) && (isalnum( ch ) || ch == '_' || ch == c_dot) );
      
      unrdch( ch );
      
      token[ toksize ] = '\0';

      /* see if it is a keyword */
      
      if (token[ 0 ] != c_dot)	/* XXX - NC */
	{
	  if (token[ 0 ] != '_')
	    {
	      tokkey = keylookup( token );
	      
	      if ( tokkey != NULL )
		{
		  symb     = tokkey->type;
		  tokval.w = tokkey->value;
		  
		  return;
		}
	    }
	}
      else
	{
	  /* is it a temporary label, if so, add current  */
	  /* tempval. 			 		  */
	  
	  if ( token[ 1 ] == c_dot )
	    {
	      int t = tempval;
	      
	      token[ toksize++ ] = c_dot;
	      
	      do
		{
		  UBYTE	d = t & 0xf;
		  
		  
		  token[ toksize++ ] = 'A' + d;
		  
		  t = t >> 4;
		}
	      while( t != 0 );
	      
	      token[ toksize ] = '\0';
	    }
	}
      
      /* now see if it is in a symbol table */
      
      toksymref = lookup( token );
      
      if ( !NullRef( toksymref ) )
	{
	  toksym = VMAddr( Symbol, toksymref );
	  
	  symb   = toksym->type;
	  tokval = toksym->def;
	  
	  return;
	}
      else
	{
	  toksym = NULL;
	}
      
      do
	{
	  ch = asm_rdch();
	}
      while( ch == c_space || ch == c_tab );
      
      if ( ch == c_colon )
	{
	  symb = s_labdef;
	}
      else
	{
	  unrdch( ch );
	  
	  symb = s_token;
	}
      
      return;
    }
  
  trace( "2ch = %02lx", ch );
  
  if ( ch == EOF )
    {
      symb = s_eof;
      
      return;
    }
  
  if ( ch & 0x80 )
    {
      parsepreasm( ch );
      
      return;
    }
  
  switch ( (int)ch )
    {
      /* this could either be an operator or a comment */
    case c_minus :
      {
	INT 	c = asm_rdch();         /* look at next char */

	
	unrdch( c );              /* put the char right back */
	
	if ( c == c_minus )
	  {
	    while ( (ch = asm_rdch()) != c_nl )
	      ;
	    
	    symb = s_nl;
	  }
	else
	  {
	    symb = s_minus;
	  }
	
	break;
      }
      
    case '0': case '1': case '2':
    case '3': case '4': case '5':
    case '6': case '7': case '8':
    case '9': case '#':
      trace( "Calling readnumber" );
      
      readnumber( ch );
      return;
      
      /* a string */
      
    case c_quote :
      {
	ch = asm_rdch();
	
	while ( ch != c_quote )
	  {
	    if ( ch == c_backslash )
	      {
		ch = asm_rdch();
		
		switch ( (int)locase( ch ) )
		  {
		  case 'a': ch = c_bell; break;
		  case 'b': ch = c_bs; break;
		  case 'f': ch = c_ff; break;
		  case 'n': ch = c_nl; break;
		  case 'r': ch = '\r'; break;
		  case 't': ch = c_tab; break;
		  case 'v': ch = c_vt; break;
		  case '\\':		    /* \" \' \? and \\ represent themselves */
		  case '\'':
		  case '\?':
		  case '\"': break;
		  case 'x': ch = rdcharnum( 16, asm_rdch() ); break;
		  default:
		    if ( '0' <= ch && ch <= '7' ) 
		      ch = rdcharnum( 8, ch );
		  }
	      }
	    
	    token[ toksize++ ] = ch;
	    
	    if ( toksize > maxtok )
	      tokerror();
	    
	    ch = asm_rdch();
	  }
	
	token[ toksize ] = '\0';
	
	symb = s_string;
	
	return;
      }
      
    case c_plus  : symb = s_plus;   break;
    case c_mul   : symb = s_mul;    break;
    case c_div   : symb = s_div;    break;
    case c_rem   : symb = s_rem;    break;
    case c_shl   : symb = s_shl;    break;
    case c_shr   : symb = s_shr;    break;
    case c_and   : symb = s_and;    break;
    case c_or    : symb = s_or;     break;
    case c_xor   : symb = s_xor;    break;
    case c_not   : symb = s_not;    break;
      
    case c_lbra  : symb = s_lbra;   break;
    case c_rbra  : symb = s_rbra;   break;
    case c_comma : symb = s_comma;  break;
    case c_nl    : symb = s_nl;     break;
    case c_cr    : symb = s_cr;     break;
    case c_colon : symb = s_colon;  break;
    case c_semic : symb = s_semic;  break;
    case c_at    : symb = s_at;     break;
      default      : symb = s_other;  break;
      
    }
  
  trace( "3ch = %02lx", ch );
  
  token[ toksize++ ] = ch;
  token[ toksize   ] = '\0';

  return;
}

/********************************************************/
/* readnumber                                           */
/*                                                      */
/* Read a number from the input. Stops on the first non */
/* digit.                                               */
/*                                                      */
/********************************************************/

PUBLIC INT readnumber(ch)
WORD ch;
{
        INT radix = 10;
        INT val = 0;
        BYTE sign = c_plus;
        if( ch == c_minus )
        {
                sign = ch;
                token[toksize] = ch;
		toksize++;
                ch = asm_rdch();
        }

        if( ch == c_hash )
        {
                radix = 16;
                ch = asm_rdch();
        }

        for(;;)
        {
                INT digit;
                UBYTE uch = locase(ch);
		trace("Readnumber %x %x",ch,(WORD)uch);	
                if ( '0' <= uch && uch <= '9' ) digit = uch - '0';
                else if (radix == 16 && 'a' <= uch && uch <= 'f') 
                                   digit = uch - 'a' + 10;
                else break;
                val = val * radix + digit;
                token[toksize] = ch;
		toksize++;
                ch = asm_rdch();
        }

        unrdch(ch);
        token[toksize] = '\0';

        symb = s_number;
        tokval.w = (sign==c_minus?-val:val);

	trace("readnumber: %d",tokval.w);
}

int rdcharnum(radix,ch)
int radix;
int ch;
{
	INT val = 0;
	int i;
        for( i = 0 ; i < 3 ; i++ )
        {
                INT digit;
                UBYTE uch = locase(ch);

                switch( radix )
                {
                case 8:
	                if ( '0' <= uch && uch <= '7' ) digit = uch - '0';
        	        else goto done;
        	        break;
        	        
        	case 16:
	                if ( '0' <= uch && uch <= '9' ) digit = uch - '0';
        	        else if( 'a' <= uch && uch <= 'f' )
        	        			digit = uch - 'a' + 10;
        	        else goto done;
			break;
		}
                val = (val * radix) + digit;
                ch = asm_rdch();
        }
done:
	unrdch(ch);
	return val;
}

/********************************************************/
/* keylookup                                            */
/*                                                      */
/* search key table for a keyword.                      */
/*                                                      */
/********************************************************/

PRIVATE struct keyentry *keylookup(name)
char *name;
{
  UWORD	h = (hash(name) % GLOBAL_HASHSIZE);
  INT 	offset = dicttab[h];


  while ( offset != 0 )
    {
      struct keyentry *k = (struct keyentry *)(&(keytab[ offset ]));

      trace("keylookup %s == %s", name, k->name );

      if ( eqs( name, k->name ) )
      {
	return k;
      }
      
      offset = k->next;
    }

  return NULL;
}

static void tokerror()
{
	token[toksize] = '\0';
	error("Token too large:\n%s",token);
}
