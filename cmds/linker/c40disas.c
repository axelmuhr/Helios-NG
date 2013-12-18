/*
 * c40disas - a stand alone disassembler for the TMS320C40
 *
 * Copyright (c) 1992 - 1993 Perihelion Software Ltd.
 *  All Rights Reserved.
 *
 * Author: 	N Clifton
 * Version:	$Revision: 1.30 $
 * Date:	$Date: 1994/03/02 15:16:30 $
 * Header:	$Header: /hsrc/cmds/linker/RCS/c40disas.c,v 1.30 1994/03/02 15:16:30 nickc Exp $
 */

/*{{{  Headers   */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "c40ops.h"
#include "c40target.h"
#include <helios.h>
#include <module.h>

#ifdef SUN4
#include <unistd.h>	/* for SEEK_SET */
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS	0
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE	-1
#endif

#define strtoul		strtol
#endif

/*}}}*/
/*{{{  Types     */

typedef enum
  {
    TYPE_SIGNED,
    TYPE_UNSIGNED,
    TYPE_FLOATING
  }
immed_type;

/*}}}*/
/*{{{  Constants */

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#define bool	int
#endif

/*
 * the disassembly layout is as follows:
 *
 * AAAAAAAA WWWWWWWW BBBB OOOOO PPPPPPPPPPPP DDDDDD TTTTTTT
 *
 * where
 *
 *   A - word address
 *   W - value at address
 *   B - byte equivalents of word
 *   O - corresponding op code
 *   P - operands of op code
 *   D - destination of branches
 *   T - trace information
 */

#define COL_ADDR	1
#define COL_WORD	9
#define COL_BYTES	21
#define COL_OP_CODE	35
#define COL_OPERAND	41
#define COL_DESTINATION	56
#define COL_TRACE	73

#ifndef RS6000
#define abs( a )	((a) < 0 ? -(a) : (a))
#endif

/*}}}*/
/*{{{  Variables */

static unsigned char * 		ProgName     = (unsigned char *)"disas"; /* the name of the program */
static unsigned long int	word_address = 0;			 /* address of current instruction */
static unsigned long int	address_base = 0;			 /* offset to word address for displaying addresses */

static unsigned char *		output_file  = (unsigned char *)"-";	 /* name of output destination */
static FILE *			output	     = NULL;			 /* file handle for output */

int	pcsregs = FALSE;	/* default to std machine register names */

/*}}}*/
/*{{{  Functions */

/*{{{  Print Routines */

/*{{{  inform */

/*
 * display an error or information message
 */

#ifdef __CC_NORCROFT
#pragma -v3
#endif

static void
inform(
       char *	message,
       ...		)
{
  va_list	args;


  va_start( args, message );

#if 0
  fflush( stderr );

  fseek( stderr, 0L, SEEK_END );
#endif
  
  if (ProgName)
    fprintf( stderr, "%s: ", ProgName );

  vfprintf( stderr, message, args );

  fprintf( stderr, "\n" );

#if 0
  fflush( stderr );
#endif
  
  va_end( args );

  return;
  
} /* inform */

/*}}}*/
/*{{{  myprint */

static int 	current_column = 0;		/* number of characters printed out on current line */


/*
 * display a message, keeping a track of where we are
 */

static void
myprint( const char * format, ... )
{
  static char	buffer[ 1024 ];			/* XXX */
  va_list	args;
  char *	start;
  char *	ptr;


  if (output == NULL)
    return;
  
  va_start( args, format );

#if 0
  fflush( output );

  fseek( output, 0L, SEEK_END );
#endif
  
  vsprintf( buffer, format, args );

  current_column += strlen( buffer );

  start = buffer;
  
  while ((ptr = (char *)strchr( start, '%' )) != NULL)
    {
      *ptr = '\0';

      fprintf( output, start );

      fprintf( output, "%%" );

      start = ptr + 1;
    }

  fprintf( output, start );

#if 0
  fflush( output );
#endif

  va_end( args );

  return;
    
} /* myprint */

#ifdef __CC_NORCROFT
#pragma -v0
#endif

/*}}}*/
/*{{{  pad_to */

/*
 * output spaces until the given column is reached
 */

static void
pad_to( int goal_column )
{
  if (current_column < goal_column)
    {
      while (current_column < goal_column)
	{
	  fputc( ' ', output );

	  ++current_column;
	}
    }
  else if (current_column > goal_column)
    {
      fputc( ' ', output );
    }

  fflush( output );

  return;

} /* pad_to */

/*}}}*/
/*{{{  new_line */

/*
 * send a newline character, adjusting column count aswell
 */

static void
new_line( void )
{
  fputc( '\n', output );

  current_column = 0;

  return;
  
} /* new_line */

/*}}}*/

/*}}}*/
/*{{{  List Routines */

/*
 * list manipulation routines - stolen from Helios
 */

/*{{{  Types */

typedef struct Node
  {
    struct Node *	next;
    struct Node *	prev;
  }
Node;

typedef struct
  {
    Node *	head;	/* points to first real item on list */
    Node *	earth;	/* always NULL */
    Node *	tail;	/* points to last real item on list */
  }
List;

/*}}}*/
/*{{{  Functions */

/*{{{  seearch_list */

/*
 * search a list using a supplied boolean function
 * returns the first node that "matches" or NULL
 */

static Node *
search_list(
	    register List *	plist,		/* the list to scan */
	    register bool (*	pfunc)(),	/* the function to apply */
	    register long int 	arg )		/* the argument to the applied function */
{
  register Node *	pnode;


  for (pnode = plist->head; pnode->next != NULL; pnode = pnode->next)
    {
      if ((*pfunc)( pnode, arg ))
	return pnode;
    }
  
  return NULL;

} /* search_list */

/*}}}*/
/*{{{  init_list */

/*
 * initialise a list
 */

static void
init_list( register List * plist )
{
  plist->head  = (Node *)&plist->earth;
  plist->earth = NULL;
  plist->tail  = (Node *)&plist->head;
  
  return;
  
} /* init_list */

/*}}}*/
/*{{{  pre_insert */

/*
 * insert a node before another node in a list
 */

static void
pre_insert(
	   register Node *	pnext,
	   register Node *	pnode )
{
  pnode->prev       = pnext->prev;
  pnode->next       = pnext;
  pnext->prev       = pnode;
  pnode->prev->next = pnode;
	
  return;
  
} /* pre_insert */

/*}}}*/
/*{{{  add_tail */

/*
 * add a node to the end of the list
 */

static void
add_tail(
	register List *	plist,
	register Node *	pnode )
{
  pnode->prev       = plist->tail;
  pnode->next       = (Node *)&plist->earth;
  plist->tail->next = pnode;
  plist->tail       = pnode; 
  
  return;

} /* add_tail */

/*}}}*/
/*{{{  remove_head */

/*
 * remove the first node on a list
 * returns the node or NULL
 */

static Node *
remove_head( register List * plist )
{
  register Node *	pnode;
  

  pnode = plist->head;
  
  if (pnode->next == NULL)
    return NULL;
  
  pnode->next->prev = (Node *)plist;
  plist->head       = pnode->next;
  pnode->next       = NULL;
  pnode->prev       = NULL;
	
  return pnode;

} /* remove_head */

/*}}}*/

/*}}}*/

/*}}}*/
/*{{{  Disassembler Routines */

/*
 * Disassembler routines
 */

/*{{{  Macros */

static char *	regstr =
  "R0\0\0R1\0\0R2\0\0R3\0\0R4\0\0R5\0\0R6\0\0R7\0\0"
  "AR0\0AR1\0AR2\0AR3\0AR4\0AR5\0AR6\0AR7\0"
  "DP\0\0IR0\0IR1\0BK\0\0SP\0\0ST\0\0DIE\0IIE\0"
  "IIF\0RS\0\0RE\0\0RC\0\0R8\0\0R9\0\0R10\0R11\0";

#define _regname( r ) ((((r) & 0x1f) * 4) + regstr)
		       
static char *	condstr =
  "u\0\0\0\0lo\0\0\0ls\0\0\0hi\0\0\0hs\0\0\0eq\0\0\0ne\0\0\0"
  "lt\0\0\0le\0\0\0gt\0\0\0ge\0\0\0<xx>\0nv\0\0\0v\0\0\0\0"
  "nuf\0\0uf\0\0\0nlv\0\0lv\0\0\0nluf\0luf\0\0zuf\0\0"
  "<xx>\0<xx>\0<xx>\0<xx>\0<xx>\0<xx>\0<xx>\0"
  "<xx>\0<xx>\0<xx>\0<xx>\0";
  
#define _condname( r ) ((((r) & 0x1f) * 5) + condstr)

/*}}}*/
/*{{{  Functions */

/*{{{  decode_indirect_address */

/*
 * decodes the indirect addressing field of a 'C40 op code
 */

static void
decode_indirect_address(
			long int	mode,	/* the indirect addressing mode (5 bits wide) */
			long int	addr,	/* the address register involved */
			long int	disp )	/* the displacement involved */
{
  pad_to( COL_OPERAND );
  
  switch (mode)
    {
    case B_00000:	myprint( "*+AR%ld(%ld)",         addr, disp ); break;
    case B_00001:	myprint( "*-AR%ld(%ld)",         addr, disp ); break;
    case B_00010:	myprint( "*++AR%ld(%ld)",        addr, disp ); break;
    case B_00011:	myprint( "*--AR%ld(%ld)",        addr, disp ); break;
    case B_00100:	myprint( "*AR%ld++(%ld)",        addr, disp ); break;
    case B_00101:	myprint( "*AR%ld--(%ld)",        addr, disp ); break;
    case B_00110:	myprint( "*AR%ld++(%ld)%%",      addr, disp ); break;
    case B_00111:	myprint( "*AR%ld--(%ld)%%",      addr, disp ); break;
    case B_01000:	myprint( "*+AR%ld(IR0)",         addr       ); break;
    case B_01001:	myprint( "*-AR%ld(IR0)",         addr       ); break;
    case B_01010:	myprint( "*++AR%ld(IR0)",        addr       ); break;
    case B_01011:	myprint( "*--AR%ld(IR0)",        addr       ); break;
    case B_01100:	myprint( "*AR%ld++(IR0)",        addr       ); break;
    case B_01101:	myprint( "*AR%ld--(IR0)",        addr       ); break;
    case B_01110:	myprint( "*AR%ld++(IR0)%%",      addr       ); break;
    case B_01111:	myprint( "*AR%ld--(IR0)%%",      addr       ); break;
    case B_10000:	myprint( "*+AR%ld(IR1)",         addr       ); break;
    case B_10001:	myprint( "*-AR%ld(IR1)",         addr       ); break;
    case B_10010:	myprint( "*++AR%ld(IR1)",        addr       ); break;
    case B_10011:	myprint( "*--AR%ld(IR1)",        addr       ); break;
    case B_10100:	myprint( "*AR%ld++(IR1)",        addr       ); break;
    case B_10101:	myprint( "*AR%ld--(IR1)",        addr       ); break;
    case B_10110:	myprint( "*AR%ld++(IR1)%%",      addr       ); break;
    case B_10111:	myprint( "*AR%ld--(IR1)%%",      addr       ); break;
    case B_11000:	myprint( "*AR%ld",               addr       ); break;
    case B_11001:	myprint( "*AR%ld++(IR0)B",       addr       ); break;
    default:
      return;      
    }
  
  return;
  
} /* decode_indirect_addressing */

/*}}}*/
/*{{{  mask_and_sign_extend_word */

/*
 * extracts the bits specified by 'mask' from the word 'value'
 * if necessary the resulting word is sign extended.
 * 'mask' must be a contigous set of bits starting from
 * the least significant bit
 */

signed long int
mask_and_sign_extend_word(
			  unsigned long	int	value,
			  unsigned long	int	mask )
{
  value &= mask;

  if (value & ((mask + 1) >> 1))
    {
      value |= ~mask;
    }

  return (signed long int)value;
  
} /* mask_and_sign_extend_word */

/*}}}*/
/*{{{  decode_short_float */

/*
 * turns a 16 bit integer into a string
 * representing a short format C40 style
 * floating point number
 */

static const char *
decode_short_float( unsigned long number )
{
  static char	buffer[ 20 ];
  unsigned long	e;
  unsigned long	s;
  unsigned long	f;
  union
    {
      float		f;	/* XXX beware of assumption that 	*/
      unsigned long	l;	/* sizeof (float) == sizeof (long)	*/
    }
  converter;
  
  
  if (number & 0xffff0000U)
    {
      return "bad format short float";
    }

  /*
   * format is ...
   *
   *
   *  15      12   11   10       0
   *  ____________________________
   * |          |      |          |
   * | exponent | sign | mantissa |
   * |          |      |          |
   *  ----------------------------
   *
   * interpretation is ...
   *
   * e = -8, s = 0, m = 0 => 0.0
   * s = 0                =>   01.m  x 2^e
   * s = -1               => -(10.m) x 2^e
   *
   */

  /* extract components */
  
  e = (number & 0xf000U) >> 12;
  s =  number & 0x0800U;
  f =  number & 0x07ffU;

  e = mask_and_sign_extend_word( e, 0xf );
  
  /* handle the special case */
  
  if (s == 0 && e == -8 && f == 0)
    {
      return "0.0";
    }

  /* convert the binary value into host specific floating point value */
  
#ifdef __CROSSCOMP

  /* convert to IEEE single precision */
  
  e += 0x7f;
  
  if (s == 0)
    {
      converter.l = (e << 23) | (f << (22 - 10));
    }
  else
    {
      if (f == 0)
	{
	  converter.l = ((e + 1) << 23) | (1 << 31);
	}
      else
	{
	  converter.l = (e << 23) | (((((~f) + 1) << (22 - 10))) & ((1 << 23) - 1)) | (1 << 31);
	}
    }
#else

  /* convert to C40 single precision */
  
  converter.l = (e << 24) | (s << (23 - 11)) | (f << (22 - 10));
  
#endif /* __C40 */

  /* convert to a string */
  
  sprintf( buffer, "%f", converter.f );

  /* return the string */
  
  return buffer;

} /* decode_short_float */

/*}}}*/
/*{{{  decode_diadic_address */

/*
 * decodes the addressing modes (the 'G' field)
 * of a 'C40 diadic op code
 */

static void
decode_diadic_address(
		      unsigned long	op_code,	/* instruction 			  */
		      char *		instruction,	/* string identifying instruction */
		      immed_type	type,		/* type of immediate value	  */
		      bool		store )		/* non zero if a store operation  */
{
  char *	pdest;
  long int	dest;

  
  /*
   * we have a diaidic op code
   *
   * bits 16 - 20 encode destination
   * bits 21 - 22 encode addressing type
   * bits 23 - 28 encode op code
   * bits 29 - 31 are 000
   */
  
  dest = (op_code >> 16) & 0x1f;

  myprint( instruction );

  pad_to( COL_OPERAND );

  if ((op_code >> 23) == OP_RPTS || (op_code >> 23) == OP_IACK)
    {
      /* this op code has no destination */
      
      pdest = NULL;
    }
  else
    {
      dest = (op_code >> 16) & 0x1f;

      if (type == TYPE_FLOATING)
	{
	  /* the destination (or source) register of a floating point op must be R0 - R11 */

	  if (!is_extended_precision_register( dest ))
	    {
	      myprint( "R<illegal>," );
	    }
	}

      pdest = _regname( dest );
    }
  
  switch ((op_code >> 21) & 0x3)
    {
    case 0: /* register to register */
      /*
       * we have a diaidic op code, using register addressing
       *
       * bits  0 - 4  encode source register
       * bits  5 - 15 are 00000000000
       * bits 16 - 20 encode destination register
       * bits 21 - 22 encode addressing type
       * bits 23 - 28 encode op code
       * bits 29 - 31 are 000
       */

      op_code &= 0xffff;
      
      if ((op_code > 0x1f) ||
	  (type == TYPE_FLOATING &&
	  !is_extended_precision_register( (signed long) op_code )))
	{
	  myprint( "R<illegal>" );
	  
	  return;
	}
      
      if (pdest)
	myprint( "%s, %s", _regname( op_code ), pdest );
      else
	myprint( "%s", _regname( op_code ) );

      break;
	      
    case 1: /* direct */
      /*
       * we have a diaidic op code, using direct addressing
       *
       * bits  0 - 15 are an unsigned offset to the data page register
       * bits 16 - 20 encode destination register
       * bits 21 - 22 encode addressing type
       * bits 23 - 28 encode op code
       * bits 29 - 31 are 000
       */

      op_code &= 0xffff;
      
      if (store)	/* pdest is actually source */
	myprint( "%s, @%#04.4lx", pdest, op_code );
      else if (pdest)
	myprint( "@%#04.4lx, %s", op_code, pdest );
      else
	myprint( "@%#04.4lx", op_code );
      
      break;

    case 2: /* indirect */
      /*
       * we have a diaidic op code, using indirect addressing
       *
       * bits  0 -  7 encode the displacment
       * bits  8 - 10 encode the address register
       * bits 11 - 15 encode the mode
       * bits 16 - 20 encode destination register
       * bits 21 - 22 encode addressing type
       * bits 23 - 28 encode op code
       * bits 29 - 31 are 000
       */

	{
	  long int	disp = op_code & 0xff;
	  long int	addr = (op_code >> 8) & 0x7;


	  if (store)
	    {
	      myprint( "%s, ", pdest );

	      decode_indirect_address( op_code >> 11 & 0x1f, addr, disp );
	    }
	  else
	    {
	      decode_indirect_address( (op_code >> 11) & 0x1f, addr, disp );

	      if (pdest)
		myprint( ", %s", pdest );
	    }
	}
      break;
      
    case 3: /* immediate */
      /*
       * we have a diaidic op code, using immeadiate addressing
       *
       * bits  0 - 15 are a signed immeadiate value
       * bits 16 - 20 encode destination register
       * bits 21 - 22 encode addressing type
       * bits 23 - 28 encode op code
       * bits 29 - 31 are 000
       */

      if (type == TYPE_SIGNED)
	dest = mask_and_sign_extend_word( op_code, 0x0000ffffU );
      else
	dest = op_code & 0xffffU;
      
      if (type == TYPE_FLOATING)
	{
	  myprint( "%s, %s", decode_short_float( dest ), pdest );
	}
      else if (dest < 500 && dest > -500 && type == TYPE_SIGNED)
	{
	  myprint( "%ld, %s", dest, pdest );
	}
      else if (pdest)
	{
	  myprint( "%#lx, %s", dest, pdest );
	}
      else
	{
	  myprint( "%#lx", dest );
	}
	
      break;      
    }

  return;
  
} /* decode_diadic_address */

/*}}}*/
/*{{{  decode_integer_store */

/*
 * handle the special case of the STI op code
 */

static void
decode_integer_store( unsigned long op_code )
{
  long int		dest;

  
  /*
   * we have an integer store
   *
   * bits  0 - 15 encode destination
   * bits 16 - 20 encode source
   * bits 21 - 22 encode addressing type
   * bits 23 - 31 are 000101010
   */

  dest = (op_code >> 16) & 0x1f;

  switch ((op_code >> 21) & 0x3)
    {
    case 0: /* indirect immediate */
      /*
       * bits  0 - 15 are an unsigned offset from the data page register
       * bits 16 - 20 encode value
       * bits 21 - 31 are 00010101000
       */

      myprint( "STIK" );

      pad_to( COL_OPERAND );

      myprint( "%ld, @%#04.4lx", mask_and_sign_extend_word( dest, 0x1f ), (op_code & 0xffff) );

      break;
	      
    case 1: /* direct register */
      /*
       * bits  0 - 15 are an unsigned offset to the data page register
       * bits 16 - 20 encode source register
       * bits 21 - 31 are 00010101001
       */

      myprint( "STI" );

      pad_to( COL_OPERAND );

      myprint( "%s, @%#04.4lx", _regname( dest ), (op_code & 0xffff) );

      break;

    case 2: /* indirect register */
      /*
       * bits  0 -  7 encode the displacment
       * bits  8 - 10 encode the address register
       * bits 11 - 15 encode the mode
       * bits 16 - 20 encode source register
       * bits 21 - 31 are 00010101010
       */

      myprint( "STI" );

      pad_to( COL_OPERAND );

      myprint( "%s, ", _regname( dest ) );

      decode_indirect_address( (op_code >> 11) & 0x1f, (op_code >> 8) & 0x7, op_code & 0xff );

      break;
      
    case 3: /* indirect immediate */
      /*
       * bits  0 -  7 encode the displacment
       * bits  8 - 10 encode the address register
       * bits 11 - 15 encode the mode
       * bits 16 - 20 encode source value
       * bits 21 - 31 are 00010101011
       */

      myprint( "STIK" );

      pad_to( COL_OPERAND );

      myprint( "%ld, ", mask_and_sign_extend_word( dest, 0x1f ) );
	
      decode_indirect_address( (op_code >> 11) & 0x1f, (op_code >> 8) & 0x7, op_code & 0xff );

      break;
      
    }

  return;
  
} /* decode_integer_store */
  

/*}}}*/
/*{{{  decode_flow_control */

/*
 * decodes a 'C40 opcode concerned with handling program execution
 */

static void
decode_flow_control( unsigned long op_code )	/* the instruction to decode */
{
  /*
   * we have a flow control op code
   *
   * bits 30 - 31 are 01
   *
   * other bits are semi-random
   */

  if (op_code & (1 << 29))
    {
      /* bits 29 - 31 are 011 */

      if (op_code & (1 << 28))
	{
	  /* bits 28 - 31 are 0111 */
	  
	  if (op_code & (1 << 27))
	    {
	      /* bits 27 - 31 are 01111 */

	      if (op_code & (1 << 24))
		{
		  /*
		   * we have a repeat block with register addressing
		   *
		   * bits  0 -  4 encode the source
		   * bits  5 - 22 are 000000000000000000
		   * bit  23      encodes the delay
		   * bits 24 - 31 are 01111001
		   */

		  if (op_code & (1 << 23))
		    {
		      myprint( "RPTBD" );
		      
		      pad_to( COL_OPERAND );
		      
		      myprint( "%s", _regname( op_code ) );
		    }
		  else
		    {
		      myprint( "RPTB" );

		      pad_to( COL_OPERAND );

		      myprint( "%s", _regname( op_code ) );
		    }
		}
	      else
		{
		  /* bits 24 - 31 are 01111xx0 */

		  if (op_code & (1 << 23))
		    {
		      /*
		       * we have a conditional return from subroutine
		       *
		       * bits  0 - 15 are 0000000000000000
		       * bits 16 - 20 encode the condition 
		       * bits 21 - 31 are 01111000100
		       */

		      myprint( "RETS%s", _condname( op_code >> 16 ) );
		    }
		  else
		    {
		      /*
		       * we have a conditional return from interrupt
		       *
		       * bits  0 - 15 are 0000000000000000
		       * bits 16 - 20 encode the condition
		       * bit  21      encodes the delay type
		       * bits 22 - 31 are 0111100000
		       */

		      if (op_code & (1 << 21))
			{
			  myprint( "RETI%sD", _condname( op_code >> 16 ) );
			}
		      else
			{
			  myprint( "RETI%s", _condname( op_code >> 16 ) );
			}
		    }
		}
	    }
	  else
	    {
	      /* bits 27 - 31 are 01110 */
	      
	      if (op_code & (1 << 26))
		{
		  /* bits 26 - 31 are 011101 */

		  if (op_code & (1 << 25))
		    {
		      /*
		       * we have a transfer between register file and expansion register file
		       *
		       * bits  0 -  5 encode the source register
		       * bits  6 - 15 are 0000000000
		       * bits 16 - 20 encode the destination register
		       * bits 21 - 22 are 00
		       * bit  23      encodes the transfer type
		       * bits 24 - 31 are 01110110
		       */

		      if (op_code & (1 << 23))
			{
			  myprint( "LDPE" );

			  pad_to( COL_OPERAND );
			  
			  myprint( "%s, %s",
				  _regname( op_code ),
				  (((op_code >> 16) & 0xf) == 0) ? "IVTP" : "TVTP" );
			}
		      else
			{
			  myprint( "LDEP" );

			  pad_to( COL_OPERAND );
			  
			  myprint( "%s, %s",
				  ((op_code & 0xf) == 0) ? "IVTP" : "TVTP",
				  _regname( op_code >> 16 ) );
			}
		    }
		  else
		    {
		      char	op[ 10 ];

		  
		      /*
		       * we have a conditional trap
		       *
		       * bits  0 -  8 encode the trap number
		       * bits  9 - 15 are 0000000
		       * bits 16 - 20 encode the condition
		       * bits 21 - 22 are 00
		       * bit  23      encodes the trap type
		       * bits 24 - 31 are 01110100
		       */

		      if (op_code & (1 << 23))
			{
			  strcpy( op, "LAT" );
			}
		      else
			{
			  strcpy( op, "TRAP" );
			}
		      
		      strcat( op, _condname( (op_code >> 16) & 0x1f ) );

		      myprint( op );

		      pad_to( COL_OPERAND );
		      
		      myprint( "%lu", op_code & 0x1ff );
		    }
		}
	      else
		{
		  char	op[ 10 ];
		  
		  
		  /*
		   * we have a conditional jump to subroutine
		   *
		   * bits  0 - 15 encode the source register or displacement
		   * bits 16 - 20 encode the condition
		   * bits 21      encodes the jump type
		   * bits 22 - 24 are 000
		   * bit  25      encodes the addressing mode
		   * bits 26 - 31 are 011100
		   */

		  if (op_code & (1 << 21))
		    {
		      strcpy( op, "LAJ" );
		    }
		  else
		    {
		      strcpy( op, "CALL" );
		    }

		  strcat( op, _condname( (op_code >> 16) & 0x1f ) );

		  myprint( op );

		  pad_to( COL_OPERAND );
		  
		  if (op_code & (1 << 25))
		    {
		      op_code = mask_and_sign_extend_word( op_code, 0xffffU );

		      myprint( "%+ld", op_code );

		      pad_to( COL_DESTINATION );

		      myprint( "(branch to %lx)", op_code + 3 + word_address + address_base );
		    }
		  else
		    {
		      myprint( "%s", _regname( op_code & 0x1f ) );
		    }
		}
	    }
	}
      else
	{
	  /* bits 28 - 31 are 0110 */

	  if (op_code & (1 << 27))
	    {
	      /* bits 27 - 31 are 01101 */

	      if (op_code & (1 << 26))
		{
		  char		op[ 10 ];
		  long int	addr;
		  long int	offset = 0;
		  

		  /*
		   * we have a conditional decrement and branch
		   *
		   * bits  0 - 15 encode the source register or displacement
		   * bits 16 - 20 encode the condition
		   * bits 21      encodes delay
		   * bits 22 - 24 encodes the address register to be decremented
		   * bit  25      encodes the addressing mode
		   * bits 26 - 31 are 011011
		   */

		  strcpy( op, "DB" );

		  strcat( op, _condname( (op_code >> 16) & 0x1f ) );
		  
		  if (op_code & (1 << 21))
		    {
		      strcat( op, "D" );

		      offset = 4;
		    }
		  else
		    {
		      offset = 1;
		    }

		  addr = (op_code >> 22) & 0x7;

		  myprint( op );

		  pad_to( COL_OPERAND );
		  
		  if (op_code & (1 << 25))
		    {
		      op_code = mask_and_sign_extend_word( op_code, 0xffffU );
		      
		      myprint( "AR%1ld, %+ld", addr, op_code );
		    }
		  else
		    {
		      myprint( "AR%1ld, %s", addr, _regname( op_code & 0x1f ) );
		    }
		}
	      else
		{
		  char		op[ 10 ];
		  long int	offset = 0;
		  

		  /*
		   * we have a conditional branch
		   *
		   * bits  0 - 15 encode the source register or displacement
		   * bits 16 - 20 encode the condition
		   * bits 21 - 23 encode the type of branch
		   * bits 24      is 0
		   * bit  25      encodes the addressing mode
		   * bits 26 - 31 are 011010
		   */

		  strcpy( op, "B" );

		  strcat( op, _condname( (op_code >> 16) & 0x1f ) );
		  
		  switch ((op_code >> 21) & 0x7)
		    {
		    case B_000:	offset = 1; break;
		    case B_001:	strcat( op, "D"  ); offset = 3; break;
		    case B_010: return;
		    case B_011:	strcat( op, "AT" ); offset = 3; break;
		    case B_100:	return;
		    case B_101:	strcat( op, "AF" ); offset = 3; break;
		    case B_110:	return;
		    case B_111:	return;
		    }

		  myprint( op );

		  pad_to( COL_OPERAND );
		  
		  if (op_code & (1 << 25))
		    {
		      signed long int	dest;
		      signed long int	displacement;
		      
		      
		      displacement = mask_and_sign_extend_word( op_code, 0xffffU );

		      dest = displacement + offset + word_address;

		      myprint( "%+ld", displacement );

		      pad_to( COL_DESTINATION );

		      myprint( "(branch to %lx)", dest + address_base );
		    }
		  else
		    {
		      myprint( "%s", _regname( op_code & 0x1f ) );
		    }
		}
	    }
	  else
	    {
	      long int	offset = 0;
	      
	      
	      /* bits 27 - 31 are 01100 */

	      switch ((op_code >> 24) & 0x7)
		{
		case B_000:	myprint( "BR"    ); offset = 1; break;
		case B_001:	myprint( "BRD"   ); offset = 3; break;
		case B_010:	myprint( "CALL"  ); offset = 1; break;
		case B_011:	myprint( "LAJ"   ); offset = 3; break;
		case B_100:	myprint( "RPTB"  ); offset = 1; break;
		case B_101:	myprint( "RPTBD" ); offset = 3; break;
		case B_110:	myprint( "SWI"   ); return;
		default:
		case B_111:	return;
		}

	      op_code = mask_and_sign_extend_word( op_code, 0xffffffU );

	      pad_to( COL_OPERAND );

	      myprint( "%+ld", op_code );

	      pad_to( COL_DESTINATION );

	      myprint( "(branch to %lx)", op_code + offset + word_address + address_base );
	    }
	}
    }
  else
    {
      char 		op[ 10 ];
      immed_type	type;
      
      
      /*
       * we have a conditional load op
       *
       * bits  0 - 15 encode the source
       * bits 16 - 20 encode the destination
       * bits 21 - 22 encode the addressing mode
       * bits 23 - 27 encode the condition
       * bit  28      encodes op
       * bits 29 - 31 are 010
       */

      if (((op_code >> 28) & 0x1) == 0)
	{
	  strcpy( op, "LDF" );

	  type = TYPE_FLOATING;
	}
      else
	{
	  strcpy( op, "LDI" );

	  type = TYPE_SIGNED;
	}
      
      strcat( op, _condname( (op_code >> 23) & 0x1f ) );

      decode_diadic_address( op_code, op, type, FALSE );
      
      return;
    }

  return;

} /* decode_flow_control */

/*}}}*/
/*{{{  decode_non_word_load */

/*
 * decodes a 'C40 op code concerened with loading non-word quantities
 */

static void
decode_non_word_load( unsigned long	op_code )	/* instruction */
{
  char	op[ 5 ];
  char	B;
  
  
  /*
   * we have a non-word data transfer
   *
   * bits  0 - 16 encode source
   * bits 17 - 20 encode destination
   * bits 21 - 22 encode addressing type
   * bits 23 - 24 MAY encode a byte selector
   * bits 24 - 27 encode op
   * bits 28 - 31 are 1011
   */

  B = (char)((op_code >> 23) & 0x03);
  
  switch ((op_code >> 24) & 0x0f)
    {
    case OP_LB:
    case B_0001:
      strcpy( op, "LB0" );
      
      op[ 2 ] = '0' + B;
      
      decode_diadic_address( op_code, op, TYPE_SIGNED, FALSE );
      
      break;

    case OP_LBU:
    case B_0011:
      strcpy( op, "LBU0" );
      
      op[ 3 ] = '0' + B;
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_LWL:
    case B_0101:
      strcpy( op, "LWL0" );

      op[ 3 ] = '0' + B;
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_LWR:
    case B_0111:
      strcpy( op, "LWR0" );

      op[ 3 ] = '0' + B;
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_MB:
    case B_1001:
      strcpy( op, "MB0" );

      op[ 2 ] = '0' + B;
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_LH:
      strcpy( op, "LH0" );

      op[ 2 ] = '0' + (B & 1);
      
      decode_diadic_address( op_code, op, TYPE_SIGNED, FALSE );
      
      break;
      
    case OP_LHU:
      strcpy( op, "LHU0" );

      op[ 3 ] = '0' + (B & 1);
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_MH:
      strcpy( op, "MH0" );

      op[ 2 ] = '0' + (B & 1);
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    default:
      break;
    }

  return;

} /* decode_non_word_load */

/*}}}*/
/*{{{  decode_special_triadic */

/*
 * decodes a multiple parallel triadic op code
 */

static void
decode_special_triadic( unsigned long 	op_code ) 	/* instruction to decode */
{
  char *	op1;
  char *	op2;
  long int	src1;
  long int	src2;
  long int	src3;
  long int	src4;
  long int	dst1;
  long int	dst2;
  
  
  /*
   * we have a special form of triadic op code
   *
   * bits  0 -  7 encode src4
   * bits  8 - 15 encode src3
   * bits 16 - 18 encode src2
   * bits 19 - 21 encode src1
   * bit  22      encodes dst2
   * bit  23      encodes dst1
   * bits 24 - 25 encode addressing mode
   * bits 26 - 27 encode operation
   * bits 28 - 31 are 1000
   */

  src4 =   op_code        & 0xff;
  src3 =  (op_code >>  8) & 0xff;
  src2 =  (op_code >> 16) & 0x07;
  src1 =  (op_code >> 19) & 0x07;
  dst2 = ((op_code >> 22) & 0x01) + 2;
  dst1 =  (op_code >> 23) & 0x01;
  
  switch ((op_code >> 26) & 0x3)
    {
    default:
    case OP_MPYF3_ADDF3:	op1 = "MPYF3"; op2 = "ADDF3"; break;
    case OP_MPYF3_SUBF3:	op1 = "MPYF3"; op2 = "SUBF3"; break;
    case OP_MPYI3_ADDI3:	op1 = "MPYI3"; op2 = "ADDI3"; break;
    case OP_MPYI3_SUBI3:	op1 = "MPYI3"; op2 = "SUBI3"; break;
    }

  switch ((op_code >> 24) & 0x3)
    {
    case B_00:
      myprint( "%s", op1 );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      myprint( ", " );
      
      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      myprint( ", %s", _regname( dst1 ) );

      new_line();

      pad_to( COL_OP_CODE - 3 );

      myprint( "|| %s", op2 );

      pad_to( COL_OPERAND );
      
      myprint( "%s, %s, %s", _regname( src1 ), _regname( src2 ), _regname( dst2 + 2 ) );

      break;
      
    case B_01:
      myprint( "%s", op1 );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      myprint( ", %s, %s", _regname( src1 ), _regname( dst1 ) );

      new_line();

      pad_to( COL_OP_CODE - 3 );
      
      myprint( "|| %s", op2 );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      myprint( ", %s, %s", _regname( src2 ), _regname( dst2 + 2 ) );

      break;
      
    case B_10:
      myprint( "%s", op1 );

      pad_to( COL_OPERAND );

      myprint( "%s, %s, %s", _regname( src1 ), _regname( src2 ), _regname( dst1 ) );

      new_line();

      pad_to( COL_OP_CODE - 3 );
      
      myprint( "|| %s ", op2 );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      myprint( ", " );

      decode_indirect_address( src4 >> 4, src4 & 7, 1 );

      myprint( ", %s", _regname( dst2 + 2 ) );

      break;
      
    case B_11:
      myprint( "%s", op1 );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      myprint( ", %s, %s", _regname( src1 ), _regname( dst1 ) );

      new_line();

      pad_to( COL_OP_CODE - 3 );
      
      myprint( "|| %s", op2 );

      pad_to( COL_OPERAND );
      
      myprint( "%s, ", _regname( src2 ) );

      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      myprint( ", %s", _regname( dst2  + 2 ) );

      break;
    }
  
  return;
  
} /* decode_special_triadic */

/*}}}*/
/*{{{  decode_non_word_load_or_triadic */

static void
decode_non_word_load_or_triadic( unsigned long	op_code )	/* instruction to decode */
{
  /*
   * we have either a special form of triadic op code or a non-word data transfer
   *
   * bits 28 - 29 encode form
   * bits 30 - 31 are 10
   */

  switch ((op_code >> 28) & 0x3)
    {
    case B_00:	decode_special_triadic( op_code ); break;
    case B_11:	decode_non_word_load( op_code ); break;
    default:	break;
    }

  return;

} /* decode_non_word_load_or_triadic */

/*}}}*/
/*{{{  decode_triadic_parallel_addressing */

static void
decode_triadic_parallel_addressing(
				   unsigned long	op_code,	/* the instruction to decode */
				   char *		first_op,	/* the name of the first op code */
				   char *		second_op )	/* the name of the second op code */
{
  unsigned long	src1;
  unsigned long	src2;
  unsigned long	src3;
  unsigned long	dst1;
  unsigned long	dst2;

  
  /*
   * we have a triaidic parallel op code of the form:
   *
   *    first_op  src2, src1, dst1
   * || second_op src3, dst2
   *
   *
   * bits  0 -  7 encode src2
   * bits  8 - 15 encode dst2
   * bits 16 - 18 encode src3
   * bits 19 - 21 encode src1
   * bits 22 - 24 encode dst1
   * bits 25 - 29 encode the operation
   * bits 30 - 31 are 11
   */

  src2 = (op_code      ) & 0xff;
  dst2 = (op_code >>  8) & 0xff;
  src3 = (op_code >> 16) & 0x07;
  src1 = (op_code >> 19) & 0x07;
  dst1 = (op_code >> 22) & 0x07;
  
  myprint( "%s ", first_op );

  pad_to( COL_OPERAND );
  
  decode_indirect_address( src2 >> 3, src2 & 7, 1 );
  
  myprint( ", %s, %s", _regname( src1 ), _regname( dst1 ) );

  new_line();
  
  pad_to( COL_OP_CODE - 3 );
  
  myprint( "|| %s", second_op );

  pad_to( COL_OPERAND );
  
  myprint( "%s, ", _regname( src3 ) );
      
  decode_indirect_address( dst2 >> 3, dst2 & 7, 1 );
      
  return;
      
} /* decode_triadic_parallel_addressing */

  

/*}}}*/
/*{{{  decode_diadic_parallel_addressing */

static void
decode_diadic_parallel_addressing(
				  unsigned long	op_code,	/* the instruction to decode */
				  char *	first_op,	/* string identifying the first op code */
				  char *	second_op )	/* string identifying the second op code */
{
  unsigned long	src2;
  unsigned long	src3;
  unsigned long	dst1;
  unsigned long	dst2;

  
  /*
   * we have a diaidic parallel op code of the form:
   *
   *    first_op  src2, dst1
   * || second_op src3, dst2
   *
   *
   * bits  0 -  7 encode src2
   * bits  8 - 15 encode dst2
   * bits 16 - 18 encode src3
   * bits 19 - 21 are 000
   * bits 22 - 24 encode dst1
   * bits 25 - 29 encode the operation
   * bits 30 - 31 are 11
   */

  src2 = (op_code      ) & 0xff;
  dst2 = (op_code >>  8) & 0xff;
  src3 = (op_code >> 16) & 0x07;
  dst1 = (op_code >> 22) & 0x07;
  
  /* validity test */
  
  if (((op_code >> 19) & 0x07) != 0)
    {
      return;
    }

  pad_to( COL_OP_CODE );
  
  myprint( "%s", first_op );

  pad_to( COL_OPERAND );
  
  decode_indirect_address( src2 >> 3, src2 & 7, 1 );

  myprint( ", %s", _regname( dst1 ) );

  new_line();
  
  pad_to( COL_OP_CODE - 3 );
  
  myprint( "|| %s", second_op );

  pad_to( COL_OPERAND );
  
  myprint( "%s, ", _regname( src3 ) );
      
  decode_indirect_address( dst2 >> 3, dst2 & 7, 1 );
  
  return;
      
} /* decode_diadic_parallel_addressing */

  

/*}}}*/
/*{{{  decode_parallel_op */

static void
decode_parallel_op( unsigned long 	op_code ) 	/* instruction to decode */
{
  unsigned long	field1;
  unsigned long	field2;
  unsigned long	field3;
  unsigned long	field4;
  unsigned long	field5;

  
  /*
   * we have a parallel op code
   *
   * bits  0 -  7 encode field1
   * bits  8 - 15 encode field2
   * bits 16 - 18 encode field3
   * bits 19 - 21 encode field4
   * bits 22 - 24 encode field5
   * bits 25 - 29 encode the operation
   * bits 30 - 31 are 11
   */

  field1 = (op_code      ) & 0xff;
  field2 = (op_code >>  8) & 0xff;
  field3 = (op_code >> 16) & 0x07;
  field4 = (op_code >> 19) & 0x07;
  field5 = (op_code >> 22) & 0x07;
  
  switch ((op_code >> 25) & 0x1f)
    {
    case OP_STF_STF:
      /* validity tests */

      if (field4 != 0)
	{
	  return;
	}
      
      myprint( "STF" );
      
      pad_to( COL_OPERAND );

      myprint( "%s, ", _regname( field5 ) );
      
      decode_indirect_address( field1 >> 3, field1 & 7, 1 );

      new_line();

      pad_to( COL_OP_CODE - 3 );
      
      myprint( "|| STF" );

      pad_to( COL_OPERAND );
      
      myprint( "%s, ", _regname( field3 ) );
      
      decode_indirect_address( field2 >> 3, field2 & 7, 1 );
      
      break;
      
    case OP_STI_STI:
      /* validity tests */

      if (field4 != 0)
	{
	  return;
	}
      
      myprint( "STI" );

      pad_to( COL_OPERAND );

      myprint( "%s, ", _regname( field5 ) );
      
      decode_indirect_address( field1 >> 3, field1 & 7, 1 );

      new_line();
      
      pad_to( COL_OP_CODE - 3 );
      
      myprint( "|| STI" );

      pad_to( COL_OPERAND );
      
      myprint( "%s, ", _regname( field3 ) );
      
      decode_indirect_address( field2 >> 3, field2 & 7, 1 );
      
      break;
      
    case OP_LDF_LDF:
      /* validity tests */

      if (field3 != 0)
	{
	  return;
	}
      
      myprint( "LDF" );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( field1 >> 3, field1 & 7, 1 );
      
      myprint( ", %s", _regname( field5 ) );

      new_line();

      pad_to( COL_OP_CODE - 3 );
      
      myprint( "|| LDF" );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( field2 >> 3, field2 & 7, 1 );

      myprint( ", %s", _regname( field4 ) );
      
      break;
      
    case OP_LDI_LDI:
      /* validity tests */

      if (field3 != 0)
	{
	  return;
	}
      
      myprint( "LDI" );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( field1 >> 3, field1 & 7, 1 );
      
      myprint( ", %s", _regname( field5 ) );

      new_line();
      
      pad_to( COL_OP_CODE - 3 );
      
      myprint( "|| LDI" );
      
      pad_to( COL_OPERAND );
      
      decode_indirect_address( field2 >> 3, field2 & 7, 1 );

      myprint( ", %s", _regname( field4 ) );
      
      break;
      
    case OP_ABSF_STF:	decode_diadic_parallel_addressing(  op_code, "ABSF",   "STF" ); break;
    case OP_ABSI_STI:	decode_diadic_parallel_addressing(  op_code, "ABSI",   "STI" ); break;
    case OP_ADDF3_STF:	decode_triadic_parallel_addressing( op_code, "ADDF3",  "STF" ); break;
    case OP_ADDI3_STI:	decode_triadic_parallel_addressing( op_code, "ADDI3",  "STI" ); break;
    case OP_AND3_STI:	decode_triadic_parallel_addressing( op_code, "AND3",   "STI" ); break;
    case OP_ASH3_STI:	decode_triadic_parallel_addressing( op_code, "ASH3",   "STI" ); break;
    case OP_FIX_STI:	decode_diadic_parallel_addressing(  op_code, "FIX",    "STI" ); break;
    case OP_FLOAT_STF:	decode_diadic_parallel_addressing(  op_code, "FLOAT",  "STF" ); break;
    case OP_LDF_STF:	decode_diadic_parallel_addressing(  op_code, "LDF",    "STF" ); break;
    case OP_LDI_STI:	decode_diadic_parallel_addressing(  op_code, "LDI",    "STI" ); break;
    case OP_LSH3_STI:	decode_triadic_parallel_addressing( op_code, "LSH3",   "STI" ); break;
    case OP_MPYF3_STF:	decode_triadic_parallel_addressing( op_code, "MPYF3",  "STF" ); break;
    case OP_MPYI3_STI:	decode_triadic_parallel_addressing( op_code, "MPYI3",  "STI" ); break;
    case OP_NEGF_STF:	decode_diadic_parallel_addressing(  op_code, "NEGF",   "STF" ); break;
    case OP_NEGI_STI:	decode_diadic_parallel_addressing(  op_code, "NEGI",   "STI" ); break;
    case OP_NOT_STI:	decode_diadic_parallel_addressing(  op_code, "NOT",    "STI" ); break;
    case OP_OR3_STI:	decode_triadic_parallel_addressing( op_code, "OR3",    "STI" ); break;
    case OP_SUBF3_STF:	decode_triadic_parallel_addressing( op_code, "SUBF3",  "STF" ); break;
    case OP_SUBI3_STI:	decode_triadic_parallel_addressing( op_code, "SUBI3",  "STI" ); break;
    case OP_XOR3_STI:	decode_triadic_parallel_addressing( op_code, "XOR3",   "STI" ); break;
    case OP_TOIEEE_STF:	decode_diadic_parallel_addressing(  op_code, "TOIEEE", "STF" ); break;
    case OP_FRIEEE_STF:	decode_diadic_parallel_addressing(  op_code, "FRIEEE", "STF" ); break;
    default:
      break;
    }

  return;

} /* decode_parallel_op */

/*}}}*/
/*{{{  decode_triadic_address */

/*
 * deocdes a 'C40 triadic op code
 */

static void
decode_triadic_address(
		       unsigned long	op_code,	/* instruction to decode 	  */
		       char *		instruction,	/* string identifying the op code */
		       immed_type	type )		/* type of immediate values       */
{
  char *	pdest;
  long int	dest;
  unsigned long	addr;
  unsigned long	src1;
  unsigned long	src2;
  

  /*
   * we have a triaidic op code
   *
   * bits  0 -  7 encode source2
   * bits  8 - 15 encode source1
   * bits 16 - 20 encode destination register
   * bits 21 - 22 encode addessing mode
   * bits 23 - 27 encode op code
   * bit  28      encodes addressing type
   * bits 29 - 31 are 001
   */

  myprint( instruction );

  pad_to( COL_OPERAND );
  
  dest = (op_code >> 16) & 0x1f;

  if (type == TYPE_FLOATING)
    {
      /* the destination (or source) register of a floating point op must be R0 - R11 */

      if (!is_extended_precision_register( dest ))
	{
	  myprint( "R<illegal>," );
	  return;
	}
    }

  pdest = _regname( dest );
  
  /* build combined addressing type */

  addr = (op_code >> 28) & 0x1;

  addr <<= 2;

  addr |= ((op_code >> 21) & 0x3);

  /* extract source fields */
  
  src2 = op_code & 0xff;
  src1 = (op_code >> 8) & 0xff;

  /* and print out result */
  
  switch (addr)
    {
    case B_000:	/* src1 = register, src2 = register */
      
      myprint( "%s, %s, %s",
	      _regname( src2 & 0x3f ),
	      _regname( src1 & 0x3f ),
	      pdest );
      break;

    case B_001: /* src1 = indirect, src2 = register */

      myprint( "%s, ", _regname( src2 & 0x3f ) );
	      
      decode_indirect_address( src1 >> 3, src1 & 7, 1 );

      myprint( ", %s", pdest );
      
      break;
      
    case B_010:	/* src1 = register, src2 = indirect */
      
      decode_indirect_address( src2 >> 3, src2 & 7, 1 );

      myprint( ", %s", _regname( src1 & 0x3f ) );
      
      myprint( ", %s", pdest );
      
      break;

    case B_011:	/* src1 = indirect, src2 = indirect */
      
      decode_indirect_address( src2 >> 3, src2 & 7, 1 );

      myprint( ", " );
      
      decode_indirect_address( src1 >> 3, src1 & 7, 1 );

      myprint( ", %s", pdest );
      
      break;

    case B_100: /* src1 = register, src2 = immediate */
      
      if (type == TYPE_FLOATING)
	{
	  myprint( "triadic floating point immediate operations are impossible" );
	}
      else if (type == TYPE_SIGNED)
	{
	  myprint( "%ld, %s, %s",
		  mask_and_sign_extend_word( op_code, 0xff ),
		  _regname( src1 & 0x3f ),
		  pdest );
	}
      else
	{
	  myprint( "%lx, %s, %s",
		  op_code & 0xff,
		  _regname( src1 & 0x3f ),
		  pdest );
	}
      
      break;
      
    case B_101:	/* src1 = register, src2 = indirect */
      
      myprint( "*+AR%.1ld(%.2lu), %s, %s",
	      src2 & 0x7, (src2 >> 3),
	      _regname( src1 & 0x3f ),
	      pdest );
      break;
      
    case B_110:	/* src1 = indirect, src2 = immediate */
      
      if (type == TYPE_FLOATING)
	{
	  myprint( "triadic floating point immediate operations are impossible" );
	}
      else if (type == TYPE_SIGNED)
	{
	  myprint( "%ld, *+AR%.1ld(%.2lu), %s",
		  mask_and_sign_extend_word( op_code, 0xff ),
		  src1 & 0x7, (src1 >> 3),
		  pdest );
	}
      else
	{
	  myprint( "%#lx, *+AR%.1ld(%.2lu), %s",
		  op_code & 0xff,
		  src1 & 0x7, (src1 >> 3),
		  pdest );
	}

      break;
      
    case B_111:	/* src1 = indirect, src2 = indirect */
      
      myprint( "*+AR%.1ld(%.2lu), *+AR%.1ld(%.2lu), %s",
	      src2 & 0x7, (src2 >> 3),
	      src1 & 0x7,(src1 >> 3),
	      pdest );
      break;
      
    default:
      break;
    }

  return;
  
} /* decode_triadic_address */

/*}}}*/
/*{{{  decode_monadic_op */

static void
decode_monadic_op(
		  unsigned long	op_code,
		  char *	instruction )
{
  myprint( instruction );
  
  pad_to( COL_OPERAND );

  myprint( "%s", _regname( (op_code >> 16) & 0x1f ) );

  return;
  
} /* decode_monadic_op */

/*}}}*/
/*{{{  decode_sequential_op */

static void
decode_sequential_op( unsigned long	op_code )	/* instruction to decode */
{
  /*
   * bit  29      encodes diadic or triadic style op codes
   * bits 30 - 31 are 00
   */
  
  if (op_code & 0x20000000L)
    {
      /*
       * triaidic op code
       *
       * bits 23 - 27 encode op code
       * bit  28      encodes addressing type
       * bits 29 - 31 are 001
       */

      switch ((op_code >> 23) & 0x1f)
	{
	case OP_ADDC3:	decode_triadic_address( op_code, "ADDC3"   , TYPE_SIGNED   ); return;
	case OP_ADDF3:	decode_triadic_address( op_code, "ADDF3"   , TYPE_FLOATING ); return;
	case OP_ADDI3:	decode_triadic_address( op_code, "ADDI3"   , TYPE_SIGNED   ); return;
	case OP_AND3:	decode_triadic_address( op_code, "AND3"    , TYPE_UNSIGNED ); return;
	case OP_ANDN3:	decode_triadic_address( op_code, "ANDN3"   , TYPE_UNSIGNED ); return;
	case OP_ASH3:	decode_triadic_address( op_code, "ASH3"    , TYPE_SIGNED   ); return;
	case OP_CMPF3:	decode_triadic_address( op_code, "CMPF3"   , TYPE_FLOATING ); return;
	case OP_CMPI3:	decode_triadic_address( op_code, "CMPI3"   , TYPE_SIGNED   ); return;
	case OP_LSH3:	decode_triadic_address( op_code, "LSH3"    , TYPE_SIGNED   ); return;
	case OP_MPYF3:	decode_triadic_address( op_code, "MPYF3"   , TYPE_FLOATING ); return;
	case OP_MPYI3:	decode_triadic_address( op_code, "MPYI3"   , TYPE_SIGNED   ); return;
	case OP_OR3:	decode_triadic_address( op_code, "OR3"     , TYPE_UNSIGNED ); return;
	case OP_SUBB3:	decode_triadic_address( op_code, "SUBB3"   , TYPE_SIGNED   ); return;
	case OP_SUBF3:	decode_triadic_address( op_code, "SUBF3"   , TYPE_FLOATING ); return;
	case OP_SUBI3:	decode_triadic_address( op_code, "SUBI3"   , TYPE_SIGNED   ); return;
	case OP_TSTB3:	decode_triadic_address( op_code, "TSTB3"   , TYPE_UNSIGNED ); return;
	case OP_XOR3:	decode_triadic_address( op_code, "XOR3"    , TYPE_UNSIGNED ); return;
	case OP_MPYSHI3:decode_triadic_address( op_code, "MPYSHI3" , TYPE_SIGNED   ); return;
	case OP_MPYUHI3:decode_triadic_address( op_code, "MPYUHI3" , TYPE_UNSIGNED ); return;
	default:
	  return;
	}
    }
  else
    {
      /*
       * diaidic op code
       *
       * bits 23 - 28 encode op code
       * bits 29 - 31 are 000
       */

      switch (op_code >> 23)
	{
	case OP_ABSF:	decode_diadic_address( op_code, "ABSF"   , TYPE_FLOATING, FALSE ); return;
	case OP_ABSI:	decode_diadic_address( op_code, "ABSI"   , TYPE_SIGNED,   FALSE ); return;
	case OP_ADDC:	decode_diadic_address( op_code, "ADDC"   , TYPE_SIGNED,   FALSE ); return;
	case OP_ADDF:	decode_diadic_address( op_code, "ADDF"   , TYPE_FLOATING, FALSE ); return;
	case OP_ADDI:	decode_diadic_address( op_code, "ADDI"   , TYPE_SIGNED,   FALSE ); return;
	case OP_AND:	decode_diadic_address( op_code, "AND"    , TYPE_UNSIGNED, FALSE ); return;
	case OP_ANDN:	decode_diadic_address( op_code, "ANDN"   , TYPE_UNSIGNED, FALSE ); return;
	case OP_ASH:	decode_diadic_address( op_code, "ASH"    , TYPE_SIGNED,   FALSE ); return;
	case OP_CMPF:	decode_diadic_address( op_code, "CMPF"   , TYPE_FLOATING, FALSE ); return;
	case OP_CMPI:	decode_diadic_address( op_code, "CMPI"   , TYPE_SIGNED,   FALSE ); return;
	case OP_FIX:	decode_diadic_address( op_code, "FIX"    , TYPE_FLOATING, FALSE ); return;
	case OP_FLOAT:	decode_diadic_address( op_code, "FLOAT"  , TYPE_SIGNED,   FALSE ); return;
	case OP_IDLE:	myprint( "IDLE" ); return;
	case OP_LDE:	decode_diadic_address( op_code, "LDE"    , TYPE_FLOATING, FALSE ); return;
	case OP_LDF:	decode_diadic_address( op_code, "LDF"    , TYPE_FLOATING, FALSE ); return;
	case OP_LDFI:	decode_diadic_address( op_code, "LDFI"   , TYPE_FLOATING, FALSE ); return;
	case OP_LDI:	decode_diadic_address( op_code, "LDI"    , TYPE_SIGNED,   FALSE ); return;
	case OP_LDII:	decode_diadic_address( op_code, "LDII"   , TYPE_SIGNED,   FALSE ); return;
	case OP_LDM:	decode_diadic_address( op_code, "LDM"    , TYPE_FLOATING, FALSE ); return;
	case OP_LSH:	decode_diadic_address( op_code, "LSH"    , TYPE_SIGNED,   FALSE ); return;
	case OP_MPYF:	decode_diadic_address( op_code, "MPYF"   , TYPE_FLOATING, FALSE ); return;
	case OP_MPYI:	decode_diadic_address( op_code, "MPYI"   , TYPE_SIGNED,   FALSE ); return;
	case OP_NEGB:	decode_diadic_address( op_code, "NEGB"   , TYPE_SIGNED,   FALSE ); return;
	case OP_NEGF:	decode_diadic_address( op_code, "NEGF"   , TYPE_FLOATING, FALSE ); return;
	case OP_NEGI:	decode_diadic_address( op_code, "NEGI"   , TYPE_SIGNED,   FALSE ); return;
	case OP_NOP:	myprint( "NOP" ); return;	/* XXX - what about indirect addressing modes */
	case OP_NORM:	decode_diadic_address( op_code, "NORM"   , TYPE_FLOATING, FALSE ); return;
	case OP_NOT:	decode_diadic_address( op_code, "NOT"    , TYPE_UNSIGNED, FALSE ); return;
	case OP_POP:	decode_monadic_op(     op_code, "POP"   ); return;
	case OP_POPF:	decode_monadic_op(     op_code, "POPF"  ); return;
	case OP_PUSH:	decode_monadic_op(     op_code, "PUSH"  ); return;
	case OP_PUSHF:	decode_monadic_op(     op_code, "PUSHF" ); return;
	case OP_OR:	decode_diadic_address( op_code, "OR"     , TYPE_UNSIGNED, FALSE ); return;
	case OP_RND:	decode_diadic_address( op_code, "RND"    , TYPE_FLOATING, FALSE ); return;
	case OP_ROL:	decode_monadic_op(     op_code, "ROL"   ); return;
	case OP_ROLC:	decode_monadic_op(     op_code, "ROLC"  ); return;
	case OP_ROR:	decode_monadic_op(     op_code, "ROR"   ); return;
	case OP_RORC:	decode_monadic_op(     op_code, "RORC"  ); return;
	case OP_RPTS:	decode_diadic_address( op_code, "RPTS"   , TYPE_UNSIGNED, FALSE ); return;
	case OP_STF:	decode_diadic_address( op_code, "STF"    , TYPE_FLOATING, TRUE  ); return;
	case OP_STFI:	decode_diadic_address( op_code, "STFI"   , TYPE_FLOATING, TRUE  ); return;
	case OP_STI:	decode_integer_store(  op_code ); return;
	case OP_SIGI:	decode_diadic_address( op_code, "SIGI"   , TYPE_SIGNED,   FALSE ); return;
	case OP_STII:	decode_diadic_address( op_code, "STII"   , TYPE_SIGNED,   TRUE  ); return;
	case OP_SUBB:	decode_diadic_address( op_code, "SUBB"   , TYPE_SIGNED,   FALSE ); return;
	case OP_SUBC:	decode_diadic_address( op_code, "SUBC"   , TYPE_SIGNED,   FALSE ); return;
	case OP_SUBF:	decode_diadic_address( op_code, "SUBF"   , TYPE_FLOATING, FALSE ); return;
	case OP_SUBI:	decode_diadic_address( op_code, "SUBI"   , TYPE_SIGNED,   FALSE ); return;
	case OP_SUBRB:	decode_diadic_address( op_code, "SUBRB"  , TYPE_SIGNED,   FALSE ); return;
	case OP_SUBRF:	decode_diadic_address( op_code, "SUBRF"  , TYPE_FLOATING, FALSE ); return;
	case OP_SUBRI:	decode_diadic_address( op_code, "SUBRI"  , TYPE_SIGNED,   FALSE ); return;
	case OP_TSTB:	decode_diadic_address( op_code, "TSTB"   , TYPE_UNSIGNED, FALSE ); return;
	case OP_XOR:	decode_diadic_address( op_code, "XOR"    , TYPE_UNSIGNED, FALSE ); return;
	case OP_IACK:	decode_diadic_address( op_code, "IACK"   , TYPE_UNSIGNED, FALSE ); return;
	case OP_TOIEEE:	decode_diadic_address( op_code, "TOIEEE" , TYPE_FLOATING, FALSE ); return;
	case OP_FRIEEE:	decode_diadic_address( op_code, "FRIEEE" , TYPE_FLOATING, FALSE ); return;
	case OP_RSQRF:	decode_diadic_address( op_code, "RSQRF"  , TYPE_FLOATING, FALSE ); return;
	case OP_RCPF:	decode_diadic_address( op_code, "RCPF"   , TYPE_FLOATING, FALSE ); return;
	case OP_MPYSHI:	decode_diadic_address( op_code, "MPYSHI" , TYPE_SIGNED,   FALSE ); return;
	case OP_MPYUHI:	decode_diadic_address( op_code, "MPYUHI" , TYPE_UNSIGNED, FALSE ); return;
	case OP_LDA:	decode_diadic_address( op_code, "LDA"    , TYPE_SIGNED,   FALSE ); return;
	case OP_LDPK:	decode_diadic_address( op_code, "LDPK"   , TYPE_SIGNED,   FALSE ); return;
	case OP_LDHI:	decode_diadic_address( op_code, "LDHI"   , TYPE_UNSIGNED, FALSE ); return;
	default:
	  return;
	}
    }
  
  return;
  
} /* decode_sequential_op */

/*}}}*/
/*{{{  decode */

static void
decode( unsigned long op_code )	/* instruction to decode */
{
  /*
   * bits 30 and 31 encode instruction type
   */

  switch ((op_code >> 30) & 0x3)
    {
    default:
    case 0: decode_sequential_op(            op_code ); return;
    case 1: decode_flow_control(             op_code ); return;
    case 2: decode_non_word_load_or_triadic( op_code ); return;
    case 3: decode_parallel_op(              op_code ); return;
    }

} /* decode */

/*}}}*/

/*}}}*/

/*}}}*/
/*{{{  Control Functions */

/*{{{  Variables */

static List branch_trace;		/* linked list of branch_ref structures */
static List func_trace;			/* linked list of function headers      */

/*}}}*/
/*{{{  Types */

typedef struct branch_ref
  {
    Node		node;			/* link into branch trace list       */
    unsigned long int	address_refered_to;	/* address to which a branch will go */
    unsigned long int	address_of_refer;	/* address of branch instruction     */
  }
branch_ref;

typedef struct func_ref
  {
    Node		node;			/* link into function header list         */
    unsigned long int	address_of_name;	/* word address of start of function name */
    unsigned long int	length_of_name;		/* byte length of function name           */
  }
func_ref;

/*}}}*/
/*{{{  Functions */

/*{{{  print_byte */

static void
print_byte( long int value )
{
  if (value < 0)
    value = -value;

  value &= 0xff;

  if (value == '\n')
    {
      myprint( "\\n " );
    }
  else if (value == '\t')
    {
      myprint( "\\t " );
    }
  else if (value == '\v')
    {
      myprint( "\\v " );
    }
  else if (value == '\f')
    {
      myprint( "\\f " );
    }
  else if (value >= ' ' && value <= '~' )
    {
      myprint( " %c ", (char)value );
    }
  else
    {
      myprint( "..." );
    }

  return;
  
} /* print_byte */
  

/*}}}*/
/*{{{  is_branch */

/*
 * returns non-zero if the instruction is a branch instruction, 0 otherwise
 */

static signed long int
is_branch( unsigned long instruction )
{
  if (((instruction >> 24) & 0xffL) == B_01101010)
    {
      if (((instruction >> 21) & 0x7L) == B_000)
	{
	  return 1L + mask_and_sign_extend_word( instruction, 0xffffL );
	}
      else
	{
	  return 3L + mask_and_sign_extend_word( instruction, 0xffffL );
	}
    }

  if (((instruction >> 26) & 0x3fL) == B_011000)
    {
      if (instruction & (0x1 << 24))
	{
	  return 3L + mask_and_sign_extend_word( instruction, 0xffffffL );
	}
      else
	{
	  return 1L + mask_and_sign_extend_word( instruction, 0xffffffL );
	}
    }

  return 0L;
  
} /* is_branch */

/*}}}*/
/*{{{  check_node */

/*
 * returs TRUE if the address refered to in
 * the node is earlier in memory than arg
 */

static bool
check_node(
	   Node *	pnode,
	   int		arg )
{
  return (((branch_ref *)pnode)->address_refered_to > arg);

} /* check_node */
  

/*}}}*/
/*{{{  store_func_ref */

/*
 * add an entry to the function header list
 */

static void
store_func_ref(
	       unsigned long int	address,
	       unsigned long int	length )
{
  func_ref *	pref;


  pref = (func_ref *)malloc( sizeof( *pref ) );

  if (pref == NULL)
    {
      inform( "out of memory allocating function reference" );

      return;
    }

  pref->address_of_name = address;
  pref->length_of_name  = length;

  add_tail( &func_trace, &pref->node );
  
  return;
  
} /* store_func_ref */

/*}}}*/
/*{{{  store_branch */

  
/*
 * add an entry to the branch reference list
 */

static void
store_branch(
	     unsigned long int	address,
	     signed long int	offset )
{
  Node *	pprev;
  branch_ref *	pref;


  pref = (branch_ref *)malloc( sizeof( *pref ) );

  if (pref == NULL)
    {
      inform( "out of memory allocating branch list" );

      return;
    }

  pref->address_of_refer   = word_address;
  pref->address_refered_to = word_address + offset;

  pprev = search_list( &branch_trace, check_node, word_address + offset );

  if (pprev)
    {
      pre_insert( pprev, &pref->node );
    }
  else
    {
      add_tail( &branch_trace, &pref->node );
    }
  
  return;
  
} /* store branch */

/*}}}*/
/*{{{  read_word */

  
static bool
read_word(
	  unsigned int long *	destination,
	  FILE *		input )
{
  unsigned char	val;
  long int	tmp;
  

  if (fread( &val, 1, 1, input ) != 1)
    {
      return FALSE;
    }

  tmp = val;

  if (fread( &val, 1, 1, input ) != 1)
    {
      return FALSE;
    }

  tmp |= (((unsigned long int)val) << 8);
  
  if (fread( &val, 1, 1, input ) != 1)
    {
      return FALSE;
    }

  tmp |= (((unsigned long int)val) << 16);
  
  if (fread( &val, 1, 1, input ) != 1)
    {
      return FALSE;
    }

  *destination = tmp | (((unsigned long int)val) << 24);

  return TRUE;
  
} /* read_word */
  

/*}}}*/
/*{{{  load_trace_data */

  
/*
 * get the information needed by the branch tracing code
 * returns TRUE upon success, FALSE otherwise
 */

static bool
load_trace_data( FILE * input )
{
  unsigned long int	value;

  
  if (fseek( input, 0L, SEEK_SET ) != 0)
    {
      extern int errno;

      
      inform( "failed to rewind input, errno = %d", errno );

      return FALSE;
    }
  
  word_address = 0;

  init_list( &branch_trace );
  init_list( &func_trace   );
  
  while (read_word( &value, input ))
    {
      signed long int	offset;


      if ((value & 0xFFFFFF00U) == 0xFF000000U)
	{
	  /* we have found a start of function marker */
	  
	  offset = value & 0xFF;
	  offset /= sizeof (int);

	  /* function name starts at 'word_address - offset' */

	  store_func_ref( word_address - offset, offset );
	}
      
      if ((offset = is_branch( value )) != 0)
	{
	  store_branch( word_address, offset );
	}

      word_address++;
    }

  return TRUE;
  
} /* load_trace_data */

/*}}}*/
/*{{{  ti_disassemble */

static void
ti_disassemble( FILE * input )	/* file containing binary image */
{
  unsigned long int	value;
  int			i;
  
  
  /* nothing fancy - just turn into .word directives! */

  if (input == NULL)
    return;

  if ( output_file      == NULL  ||
      *output_file      == '\0'  ||
      (output_file[ 0 ] == '-'  &&
       output_file[ 1 ] == '\0' ) )
    {
      output = stdout;
    }
  else
    {
      output = fopen( (char *)output_file, "w" );

      if (output == NULL)
	{
	  inform( "could not open output file '%s' - using stdout instead", output_file );

	  output = stdout;	  
	}      
    }
  
  /* skip image header - we do not need it */
  
  for (i = 0; i < sizeof (ImageHdr) / sizeof (unsigned long); i++)
    read_word( &value, input );

  /* decode rest of file */
  
  while (read_word( &value, input ))
    {
      myprint( " .word 0%lxH", value );
      
      new_line();
    }

  myprint( ".end:" );

  new_line();
  
  return;
  

} /* ti_disassemble */

/*}}}*/
/*{{{  disassemble */

static void
disassemble(
	    FILE *	input,		/* file containing binary op-codes */
	    bool	trace,		/* TRUE if branches are to be traced to their destinations */
	    bool	helios_format )	/* TRUE if the input is in Helios Linker format */
{
  unsigned long int	value;
  branch_ref *		pnext_branch;
  func_ref *		pnext_func;
  bool			show_code;
  long int		name_len      = 0;
  char *		func_name     = NULL;
  

  if (input == NULL)
    return;

  if ( output_file      == NULL  ||
      *output_file      == '\0'  ||
      (output_file[ 0 ] == '-'  &&
       output_file[ 1 ] == '\0' ) )
    {
      output = stdout;
    }
  else
    {
      output = fopen( (char *)output_file, "w" );

      if (output == NULL)
	{
	  inform( "could not open output file '%s' - using stdout instead", output_file );

	  output = stdout;	  
	}      
    }
  
  if (input == stdin)
    trace = FALSE;

  if (helios_format)
    show_code = FALSE;
  else
    show_code = TRUE;
  
  if (trace)
    {
      trace = load_trace_data( input );
    }

  (void) fseek( input, 0L, SEEK_SET );
  
  word_address = 0;

  if (trace)
    {
      pnext_branch = (branch_ref *)remove_head( &branch_trace );
      pnext_func   =   (func_ref *)remove_head( &func_trace   );

      if (pnext_func)
	{
	  name_len = pnext_func->length_of_name;

	  func_name = (char *)malloc( (int)(name_len * sizeof (long) + 5) );

	  if (func_name == NULL)
	    {
	      inform( "out of memory allocating buffer for function name" );

	      name_len = 0;
	    }
	}
    }
  else
    {
      pnext_branch = NULL;
      pnext_func   = NULL;
    }
  
  while (read_word( &value, input ))
    {
      if (value == T_Module)
	{
	  int	i;

	  
	  /* we have the start of a module header */

	  for (i = sizeof (Module) / sizeof (unsigned long); i--; )
	    {
	      myprint( "%8lx", word_address + address_base );
	      
	      pad_to( COL_WORD );
	      
	      myprint( "0x%08.8lx ", value );
	      
	      pad_to( COL_BYTES );
	      
	      print_byte( value );
	      print_byte( value >>  8 );	
	      print_byte( value >> 16 );
	      print_byte( value >> 24 );
	      
	      new_line();
	      
	      word_address++;
	      
	      if (!read_word( &value, input ))
		break;
	    }
	}
      
      myprint( "%8lx", word_address + address_base );

      pad_to( COL_WORD );
      
      myprint( "0x%08.8lx ", value );

      if (helios_format && ((value & 0xFFFFFF00U) == 0xFF000000U))
	{
	  if (func_name)
	    {
	      pad_to( COL_OP_CODE );

	      myprint( "%s():", func_name );

	      free( func_name );

	      func_name = NULL;
	      name_len  = 0;
	    }

	  show_code = TRUE;

	  value = 0;
	}

      if (helios_format && pnext_func && word_address == pnext_func->address_of_name)
	{
	  show_code = FALSE;
	  name_len  = pnext_func->length_of_name;
	  
	  free( pnext_func );

	  pnext_func = (func_ref *)remove_head( &func_trace );
	  
	  func_name  = (char *)malloc( (int)(name_len * sizeof (long) + 5 ) );

	  if (func_name == NULL)
	    {
	      inform( "out of memory allocating buffer for function name" );
	      
	      name_len = 0;
	    }
	  else
	    {
	      func_name[ 0 ] = '\0';
	    }
	}

      if (!helios_format || value > 10)
	{
	  pad_to( COL_BYTES );
      
	  print_byte( value );
	  print_byte( value >>  8 );	
	  print_byte( value >> 16 );
	  print_byte( value >> 24 );

	  pad_to( COL_OP_CODE );
	  
	  if (show_code)
	    {
	      decode( value );
	    }
	}

      if (name_len > 0 && func_name)
	{
	  int 	i = strlen( func_name );

	  
	  func_name[ i++ ] = (char)( value        & 0xff);
	  func_name[ i++ ] = (char)((value >>  8) & 0xff);
	  func_name[ i++ ] = (char)((value >> 16) & 0xff);
	  func_name[ i++ ] = (char)((value >> 24) & 0xff);
	  func_name[ i++ ] = '\0';

	  --name_len;
	}

      if (trace && pnext_branch && word_address == pnext_branch->address_refered_to)
	{
	  pad_to( COL_TRACE );

	  myprint( "branched from: " );

	  while (pnext_branch->address_refered_to == word_address)
	    {
	      myprint( "%lx ", pnext_branch->address_of_refer + address_base );

	      free( pnext_branch );

	      pnext_branch = (branch_ref *)remove_head( &branch_trace );
	    }
	}

      new_line();
      
      fflush( output );

      word_address++;
    }

  return;
  
} /* disassemble */

/*}}}*/
/*{{{  usage */

/*
 * describe the command line options
 */

static void
usage( void )
{
  inform( "valid command line options:" );
  inform( "-b        enable  branch tracing" );
  inform( "-B        disable branch tracing (the default)" );
  inform( "-t        produce output compatible with the TI assembler" );
  inform( "-T        produce verbose output (the default)" );
  inform( "-h        assume input is in Generic Helios Executable Format (the default)" );
  inform( "-H        do not make such an assumption" );
  inform( "-p        display PCS register aliases" );
  inform( "-o <file> send output to the named file" );  
  inform( "-help     this message" );
  inform( "-?        this message" );
  inform( "+<offset> addition to current offset for instruction addresses" );
  inform( "-         disassemble from stdin (default)" );
  inform( "<file>    disassemble <file>" );
  inform( "--        treat all futher command line options as file names to disassemble" );
  
  exit( 0 );
  
} /* usage */

/*}}}*/
/*{{{  main */

/*
 * disassembler entry point
 */
  
int
main(
     int		argc,
     unsigned char **	argv )
{
  bool		helios_format = TRUE;
  bool		trace         = FALSE;
  bool		ti_format     = FALSE;
  FILE *	input	      = NULL;


  /* find name of program */
  
  if ((ProgName = (unsigned char *)strrchr( (const char *)argv[ 0 ], '/' )) != NULL)
    {
      ProgName++;
    }
  else
    {
      ProgName = argv[ 0 ];
    }

  /*
   * parse command line arguments
   *
   * what we want to achieve is this:
   *
   * disas fred			- disassembles file 'fred' in Helios format
   * disas -t fred		- disassembles file 'fred' in TI format
   * disas fred -t		- disassembles file 'fred' in TI format
   * disas fred -t jim		- disassembles file 'fred' in Helios format and file 'jim' in TI format
   * disas fred jim -t		- disassembles files 'fred' and 'jim' in TI format
   * disas -t fred jim		- disassembles files 'fred' and 'jim' in TI format
   * disas -t fred -T jim	- disaeembles file 'fred' in TI format, file 'jim' in Helios format
   * disas -t fred -T jim -t	- illegal
   *
   * yeah!
   *
   */
   
  if (argc > 1)
    {
      int	arg;
      int	found_switch = -1;


      /*
       * pre-scan arguments for the special case where we have
       * one or more files followed by switches but then no more
       * files ...
       */
      

      for (arg = 1; arg < argc; arg++)
	{
	  if (argv[ arg ][ 0 ] == '-'  &&
	      argv[ arg ][ 1 ] != '\0' &&
	      found_switch     == -1    )
	    {
	      found_switch = arg;
	    }
	  else if (found_switch != -1)
	    {
	      if (arg > 1                     &&
		  argv[ arg - 1 ][ 0 ] == '-' &&
		  argv[ arg - 1 ][ 1 ] == 'o' &&
		  argv[ arg - 1 ][ 2 ] == '\0' )
		{
		  /* catch the very special cases of -o <name> */
		  
		  continue;		  
		}
	      
	      if (arg > 1                     &&
		  argv[ arg - 1 ][ 0 ] == '+' &&
		  argv[ arg - 1 ][ 1 ] == '\0' )
		{
		  /* catch the very special cases of + <offset> */
		  
		  continue;		  
		}
	      
	      /* we have a switch followed by a file name - special case does not apply */

	      found_switch = -1;
	      
	      break;
	    }
	}

      if (found_switch != -1)
	{
	  /* yup special case applies */

	  for (arg = found_switch; arg < argc; arg++ )
	    {
	      if (argv[ arg ][ 0 ] == '+')
		{
		  long int	off = 0;
		      

		  if (argv[ arg ][ 1 ] == '\0')
		    {
		      if (++arg >= argc)
			{
			  inform( "warning - no value following '+' option; option has been ignored" );
			}
		      else
			{
			  if (argv[ arg ][ 1 ] == 'x' || argv[ arg ][ 1 ] == 'X')
			    off = strtoul( (char *)argv[ arg ], (char **)(&argv[ arg ]), 16 );
			  else
			    off = atoi( (char *)argv[ arg ] );

			  if (off == 0)
			    {
			      inform( "warning - no value following '+' option; option has been ignored" );
			  
			      --arg;
			    }
			} 
		    }
		  else
		    {
		      if (argv[ arg ][ 2 ] == 'x' || argv[ arg ][ 2 ] == 'X')
			off = strtoul( (char *)argv[ arg ], (char **)(&argv[ arg ]), 16 );
		      else
			off = atoi( (char *)argv[ arg ] );
		      
		      if (off == 0)
			inform( "warning - string following '+' option is not a number; option has been ignored" );		  
		    }
	      
		  address_base += off;		      
		}
	      else
		{
		  switch (argv[ arg ][ 1 ])
		    {
		    case 'o':
		      if (argv[ arg ][ 2 ] == '\0')
			{
			  if (++arg >= argc)
			    {
			      inform( "no name following '-o' option" );
			      
			      return EXIT_FAILURE;
			    }
			  
			  output_file = argv[ arg ];
			}
		      else
			{
			  output_file = argv[ arg ] + 2;		      
			}
		      
		      break;
		      
		    case 'H':
		      helios_format = FALSE;
		      break;
		      
		    case 'h':
		      if (argv[ arg ][ 2 ] == '\0')
			{
			  helios_format = TRUE;
			  break;
			}
		      
		      /* drop through */
		      
		    case '?':
		      break;
		      
		    case 'T':
		      ti_format = FALSE;
		      break;
		      
		    case 't':
		      ti_format = TRUE;
		      break;
		      
		    case 'b':
		      trace = TRUE;
		      break;
		      
		    case 'B':
		      trace = FALSE;
		      break;
		    }
		}	      
	    }
	}
      
      for (arg = 1; arg < argc; arg++)
	{
	  if (argv[ arg ][ 0 ] == '-')
	    {
	      long int	next_arg = TRUE;

	      
	      switch (argv[ arg ][ 1 ])
		{
		case 'o':
		  if (argv[ arg ][ 2 ] == '\0')
		    {
		      if (++arg >= argc)
			{
			  inform( "no name following '-o' option" );

			  return EXIT_FAILURE;
			}		      
		      
		      output_file = argv[ arg ];
		    }
		  else
		    {
		      output_file = argv[ arg ] + 2;		      
		    }
		  break;
		  
		case 'H':
		  helios_format = FALSE;
		  break;
		  
		case 'h':
		  if (argv[ arg ][ 2 ] == '\0')
		    {
		      helios_format = TRUE;
		      break;
		    }
		  
		  /* drop through */
		    
		case '?':
		  usage();
		  break;
		  
		case '\0':
		  input    = stdin;
		  next_arg = FALSE;
		  break;

		case 'T':
		  ti_format = FALSE;
		  break;

		case 't':
		  ti_format = TRUE;
		  break;

		case 'b':
		  trace = TRUE;
		  break;

		case 'B':
		  trace = FALSE;
		  break;

		case 'p':
		  pcsregs = TRUE;
		  break;

		case '-':
		  while (++arg < argc)
		    {
		      input = fopen( (char *)argv[ arg ], "rb" );

		      if (input == NULL)
			{
			  inform( "could not open input file '%s' - skipping", argv[ arg ] );
			}
		      else if (ti_format)
			{
			  ti_disassemble( input );
			}
		      else
			{
			  disassemble( input, trace, helios_format );
			}

		      if (input)
			fclose( input );
		    }
		  
		  break;

		default:
		  inform( "unknown command line option '%c'", argv[ arg ][ 1 ] );
		  usage();
		  break;
		}

	      if (next_arg)
		continue;
	    }
	  else if (argv[ arg ][ 0 ] == '+')
	    {
	      unsigned long int	off = 0;
		      

	      if (argv[ arg ][ 1 ] == '\0')
		{
		  if (++arg >= argc)
		    {
		      inform( "warning - no value following '+' option; option has been ignored" );
		    }
		  else
		    {
		      if (argv[ arg ][ 1 ] == 'x' || argv[ arg ][ 1 ] == 'X')
			off = strtoul( (char *)argv[ arg ], (char **)(&argv[ arg ]), 16 );
		      else
			off = (unsigned long int)atoi( (char *)argv[ arg ] );
		      
		      if (off == 0)
			{
			  inform( "warning - no value following '+' option; option has been ignored" );
			  
			  --arg;
			}
		    } 
		}
	      else
		{
		  if (argv[ arg ][ 2 ] == 'x' || argv[ arg ][ 2 ] == 'X')
		    off = strtoul( (char *)argv[ arg ], (char **)(&argv[ arg ]), 16 );
		  else
		    off = (unsigned long int)atoi( (char *)argv[ arg ] );
		      
		  if (off == 0)
		    inform( "warning - string following '+' option is not a number; option has been ignored" );		  
		}
	      
	      address_base += off;		      
	    }	  
	  else
	    {
	      input = fopen( (char *)argv[ arg ], "rb" );

	      if (input == NULL)
		{
		  inform( "could not open input file %s - using stdin instead", argv[ arg ] );

		  /* possible bug - stdin may have been opened in text mode */
		  
		  input = stdin;
		}
	    }

	  if (ti_format)
	    ti_disassemble( input );
	  else	  
	    disassemble( input, trace, helios_format );

	  if (input != stdin)
	    {
	      fclose( input );

	      input = NULL;
	    }
	}
    }
  else
    { 
      /* possible bug - stdin may have been opened in text mode */

      if (ti_format)
	ti_disassemble( stdin );
      else
	disassemble( stdin, trace, helios_format );
    }

  return EXIT_SUCCESS;
  
} /* main */

/*}}}*/

/*}}}*/

/*}}}*/
/*{{{  SUN4 Routines */

#ifdef SUN4
/*
 * The gcc compiler appears to generate references to the following functions
 * without providing them in a standard library.  Since the code for the
 * linker does not use either of them, they are provided here as stubs
 */

int ___assert( void ) { return 0; }
int ___typeof( void ) { return 0; }  

#endif /* SUN4 */

/*}}}*/

/*}}}*/
