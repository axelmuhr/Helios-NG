/*{{{  Comments */

/*
 * objdisas - disassembler for the 'C40 object dumper
 *
 * Author: 	N Clifton
 * Version:	$Revision: 1.22 $
 * Date:	$Date: 1994/01/18 12:27:24 $
 */

/* @@@ Still to be done:
 *
 * IACK		- no second reg
 * RPTS		- no second reg
 * ROTS		- no first operand
 * SUB / ASH3 / LSH3 parallel instructions
 *		- swap operand order
 *
 */

/*}}}*/
/*{{{  Includes */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef SUN4
#include <unistd.h>	/* for SEEK_SET */
#endif

#include "c40ops.h"
#include "c40target.h"

/*}}}*/
/*{{{  Constants */

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#define COL_OP_CODE	39
#define COL_OPERAND	10
#define COL_DESTINATION	30
#define COL_OPERAND2	39 + 10

/*}}}*/
/*{{{  Macros */

#define is_op(      instruction, op_code )	 (((instruction) >> 23)        == (op_code))
#define is_monadic( instruction )		(is_op( instruction, OP_ABSF  ) || \
						 is_op( instruction, OP_ABSI  ) || \
  						 is_op( instruction, OP_FIX   ) || \
  						 is_op( instruction, OP_FLOAT ) || \
  						 is_op( instruction, OP_NEGF  ) || \
  						 is_op( instruction, OP_NEGI  ) || \
  						 is_op( instruction, OP_NOT   ) )

/*}}}*/
/*{{{  Types */

typedef enum
  {
    TYPE_SIGNED,
    TYPE_UNSIGNED,
    TYPE_FLOATING
    }
immed_type;

/*}}}*/
/*{{{  Variables */

static int 	current_column = 0;		/* number of characters printed out on current line */
extern FILE *	outfd;				/* file to which to send output */
extern int	pcsregs;			/* display true of PCS reg aliases */

/*}}}*/
/*{{{  Code */

/*{{{  myprint() */

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


  va_start( args, format );

  vsprintf( buffer, format, args );

  current_column += strlen( buffer );

  start = buffer;
  
  while ((ptr = (char *)strchr( start, '%' )) != NULL)
    {
      *ptr = '\0';

      fprintf( outfd, start );

      fprintf( outfd, "%%" );

      start = ptr + 1;
    }

  fprintf( outfd, start );

  va_end( args );

  return;
    
} /* myprint */

/*}}}*/
/*{{{  pad_to() */

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
	  fputc( ' ', outfd );

	  ++current_column;
	}
    }
  else if (current_column > goal_column)
    {
      fputc( ' ', outfd );
    }

  return;

} /* pad_to */

/*}}}*/
/*{{{  new_line() */

/*
 * send a newline character, adjusting column count aswell
 */

static void
new_line( void )
{
  fputc( '\n', outfd );

  current_column = 0;

  return;
  
} /* new_line */

/*}}}*/
/*{{{  _regname() */

/*
 * Disassembler routines
 */

char *
_regname( long int r )
{
  static char *	regstr =
    "R0 \0R1 \0R2 \0R3 \0R4 \0R5 \0R6 \0R7 \0"
      "AR0\0AR1\0AR2\0AR3\0AR4\0AR5\0AR6\0AR7\0"
	"DP \0IR0\0IR1\0BK \0SP \0ST \0DIE\0IIE\0"
	  "IIF\0RS \0RE \0RC \0R8 \0R9 \0R10\0R11\0";
  
  r &= 0x1f;
  
  if (pcsregs) {
      switch (r)	{
	  /* r0-7 */
	default:
	  
	case 0x00 : return("R_A1");
	case 0x01 : return("R_A2");
	case 0x02 : return("R_A3");
	case 0x03 : return("R_A4");
	case 0x04 : return("R_FV1");
	case 0x05 : return("R_FV2");
	case 0x06 : return("R_FT1");
	case 0x07 : return("R_FT2");
	  
	  /* ar0 - ar7 */
	case 0x08 : return("R_ADDR1");
	case 0x09 : return("R_ADDR2");
	case 0x0a : return("R_ADDR3");
	case 0x0b : return("R_ADDR4");
	case 0x0c : return("R_MT");
	case 0x0d : return("R_ATMP");
	case 0x0e : return("R_USP");
	case 0x0f : return("R_FP");
	  
	  /* odds and sods */
	case 0x10 : return("R_V1");
	case 0x11 : return("R_BASE");
	case 0x12 : return("R_USE");
	case 0x13 : return("R_V2");
	case 0x14 : return("R_SSP");
	case 0x15 : return("R_ST");
	case 0x16 : return("DIE");
	case 0x17 : return("IIE");
	case 0x18 : return("IIF");
	  
	case 0x19 : return("R_TMP1");
	case 0x1a : return("R_TMP2");
	case 0x1b : return("R_TMP3");
	  
	  /* ar8 - ar11 */
	case 0x1c : return("R_V3");
	case 0x1d : return("R_V4");
	case 0x1e : return("R_T1");
	case 0x1f : return("R_LR");
	}
    }
  else
    {
      return ((r * 4) + regstr);
    }  
} /* _regname */

/*}}}*/
/*{{{  _condname() */

static char *	condstr =
  "u\0\0\0\0lo\0\0\0ls\0\0\0hi\0\0\0hs\0\0\0eq\0\0\0ne\0\0\0"
  "lt\0\0\0le\0\0\0gt\0\0\0ge\0\0\0<xx>\0nv\0\0\0v\0\0\0\0"
  "nuf\0\0uf\0\0\0nlv\0\0lv\0\0\0nluf\0luf\0\0zuf\0\0"
  "<xx>\0<xx>\0<xx>\0<xx>\0<xx>\0<xx>\0<xx>\0"
  "<xx>\0<xx>\0<xx>\0<xx>\0";
  
#define _condname( r ) ((((r) & 0x1f) * 5) + condstr)

/*}}}*/
/*{{{  decode_indirect_address() */

/*
 * decodes the indirect addressing field of a 'C40 op code
 */

static void
decode_indirect_address(
			long int	mode,
			long int	addr,
			long int	disp )
{
  switch (mode)
    {
    case B_00000:	myprint( "*+AR%ld(%d)",          addr, disp ); break;
    case B_00001:	myprint( "*-AR%ld(%d)",          addr, disp ); break;
    case B_00010:	myprint( "*++AR%ld(%d)",         addr, disp ); break;
    case B_00011:	myprint( "*--AR%ld(%d)",         addr, disp ); break;
    case B_00100:	myprint( "*AR%ld++(%d)",         addr, disp ); break;
    case B_00101:	myprint( "*AR%ld--(%d)",         addr, disp ); break;
    case B_00110:	myprint( "*AR%ld++(%d)%%",       addr, disp ); break;
    case B_00111:	myprint( "*AR%ld--(%d)%%",       addr, disp ); break;
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
    }
  
  return;
  
} /* decode_indirect_addressing */

/*}}}*/
/*{{{  mask_and_sign_extend_word() */

/*
 * extracts the bits specified by 'mask' from the word 'value'
 * if necessary the resulting word is sign extended.
 * 'mask' must be a contigous set of bits starting from
 * the least significant bit
 */

signed long
mask_and_sign_extend_word(
			  unsigned long	value,
			  unsigned long	mask )
{
  value &= mask;

  if (value & ((mask + 1) >> 1))
    {
      value |= ~mask;
    }

  return (signed long)value;
  
} /* mask_and_sign_extend_word */

/*}}}*/
/*{{{  decode_short_float() */

/*
 * turns a 16 bit integer into a string
 * representing a short format C40 style
 * floating point number
 */

static const char *
decode_short_float( long int number )
{
  static char	buffer[ 20 ];
  long int	e;
  long int	s;
  long int	f;
  union
    {
      float		f;	/* XXX beware of assumption that 	*/
      unsigned long	l;	/* sizeof (float) == sizeof (long)	*/
    }
  converter;
  
  
  if (number & 0xffff0000UL)
    {
      return "";
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
  
#if defined HP || defined __SUN4 || defined RS6000
  
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
  
#endif /* __HELIOSC40 */
  
  /* convert to a string */
  
  sprintf( buffer, "%f", converter.f );

  /* return the string */
  
  return buffer;
  
} /* decode_short_float */

/*}}}*/
/*{{{  decode_diadic_address() */

/*
 * decodes the addressing modes (the 'G' field)
 * of a 'C40 diadic op code
 */

static void
decode_diadic_address(
		      long int		op_code,	/* instruction to be decode       */
		      char *		instruction,	/* string identifying instruction */
		      immed_type	type,		/* type of immediate values       */
		      int		store )		/* non zero if a store operation  */
{
  char *	pdest;
  long int	dest;
  long int	src;
  
  
  /*
   * we have a diaidic op code
   *
   * bits 16 - 20 encode destination
   * bits 21 - 22 encode addressing type
   * bits 23 - 28 encode op code
   * bits 29 - 31 are 000
   */

  dest = (op_code >> 16) & 0x1f;
      
  if ((op_code >> 23) == OP_RPTS || (op_code >> 23) == OP_IACK)
    {
      /* this op code has no destination */
      
      pdest = NULL;
    }
  else
    {
      if (type == TYPE_FLOATING && (op_code >> 23) != OP_FIX)
	{
	  /* the destination (or source) register of a floating point op must be R0 - R11 */
	  
	  if (!is_extended_precision_register( dest ))
	    {
	      return;
	    }
	}
      
      pdest = _regname( dest );
    }
  
  myprint( instruction );

  pad_to( COL_OPERAND );

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

      if (op_code & 0xffe0)
	{
	  return;	  
	}
      
      src = op_code & 0x1f;
      
      if (type == TYPE_FLOATING)
	{
	  /* the source register of a floating point op must be R0 - R11 */
	  
	  if (!is_extended_precision_register( src ))
	    {
	      return;
	    }
	}
      
      myprint( "%s", _regname( src ) );
      
      if (pdest && (dest != src || !is_monadic( op_code )))
	myprint( ", %s", pdest );
      
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

      if (store)	/* pdest is actually source */
	myprint( "%s, @%#04.4lx", pdest, (op_code & 0xffff) );
      else if (pdest)
	myprint( "@%#04.4lx, %s", (op_code & 0xffff), pdest );
      else
	myprint( "@%#04.4lx", (op_code & 0xffff) );

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
	  long int	disp =  op_code       & 0xff;
	  long int	addr = (op_code >> 8) & 0x07;


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
	  myprint( "%-3ld, %s", dest, pdest );
	}
      else if (pdest)
	{
	  myprint( "%#-3lx, %s", dest, pdest );
	}
      else
	{
	  myprint( "%#-3lx", dest );
	}
      
      break;      
    }

  return;
  
} /* decode_diadic_address */
  

/*}}}*/
/*{{{  decode_integer_store() */

/*
 * handle the special case of the STI op code
 */

static void
decode_integer_store( long int op_code )
{
  long int	dest;

  
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
      
      myprint( "%-3ld, @%#04.4lx", mask_and_sign_extend_word( dest, 0x1f ), (op_code & 0xffff) );

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
      
      myprint( "%-3ld, ", mask_and_sign_extend_word( dest, 0x1f ) );
      
      decode_indirect_address( (op_code >> 11) & 0x1f, (op_code >> 8) & 0x7, op_code & 0xff );

      break;      
    }

  return;
  
} /* decode_integer_store */

/*}}}*/
/*{{{  decode_flow_control() */

/*
 * decodes a 'C40 opcode concerned with handling program execution
 */

static void
decode_flow_control(
		    long int	op_code,
		    long int	pc )
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
		      op_code = mask_and_sign_extend_word( (unsigned long)op_code, 0xffffL );
		      
		      myprint( "%+d", op_code );

		      pad_to( COL_DESTINATION );

		      myprint( "// PC = %#x", pc / 4 + (op_code + 1) );
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
		  long int	offset;
		  

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
		      op_code = mask_and_sign_extend_word( (unsigned long)op_code, 0xffffL );
		      
		      myprint( "AR%1ld, %+d", addr, op_code );

		      pad_to( COL_DESTINATION );

		      myprint( "// PC = %#x", pc / 4 + (op_code + offset) );
		    }
		  else
		    {
		      myprint( "AR%1ld, %s", addr, _regname( op_code & 0x1f ) );
		    }
		}
	      else
		{
		  char	op[ 10 ];
		  int	offset = 0;
		  

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
		    case B_000:	                    offset = 1; break;
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
		      op_code = mask_and_sign_extend_word( (unsigned long)op_code, 0xffffL );
		      
		      myprint( "%+d", op_code );

		      pad_to( COL_DESTINATION );
		      
		      myprint( "// PC = %#x", pc / 4 + (op_code + offset) );
		    }
		  else
		    {
		      myprint( "%s", _regname( op_code & 0x1f ) );
		    }
		}
	    }
	  else
	    {
	      int	offset = 0;

	      
	      /* bits 27 - 31 are 01100 */

	      switch ((op_code >> 24) & 0x7)
		{
		case B_000: myprint( "BR"    ); offset = 1;  break;
		case B_001: myprint( "BRD"   ); offset = 3;  break;
		case B_010: myprint( "CALL"  ); offset = 1;  break;
		case B_011: myprint( "LAJ"   ); offset = 3;  break;
		case B_100: myprint( "RPTB"  ); offset = 1;  break;
		case B_101: myprint( "RPTBD" ); offset = 3;  break;
		case B_110: myprint( "SWI"   ); return;
		default:
		case B_111: return;
		}

	      op_code = mask_and_sign_extend_word( (unsigned long)op_code, 0xffffffL );

	      pad_to( COL_OPERAND );

	      myprint( "%+d", op_code );

	      pad_to( COL_DESTINATION );
	      
	      myprint( "// PC = %#x", pc / 4 + (op_code + offset) );
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
    }

  return;

} /* decode_flow_control */

/*}}}*/
/*{{{  decode_non_word_load() */

/*
 * decodes a 'C40 op code concerened with loading non-word quantities
 */

static void
decode_non_word_load( long int op_code ) /* instruction */
{
  char		op[ 5 ];
  long int	B;
  
  
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

  B = ((op_code >> 23) & 0x03);
  
  switch ((op_code >> 24) & 0x0f)
    {
    case OP_LB:
    case B_0001:
      strcpy( op, "LB0" );
      
      op[ 2 ] = (char)('0' + B);
      
      decode_diadic_address( op_code, op, TYPE_SIGNED, FALSE );
      
      break;

    case OP_LBU:
    case B_0011:
      strcpy( op, "LBU0" );
      
      op[ 3 ] = (char)('0' + B);
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_LWL:
    case B_0101:
      strcpy( op, "LWL0" );

      op[ 3 ] = (char)('0' + B);
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_LWR:
    case B_0111:
      strcpy( op, "LWR0" );

      op[ 3 ] = (char)('0' + B);
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_MB:
    case B_1001:
      strcpy( op, "MB0" );

      op[ 2 ] = (char)('0' + B);
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_LH:
      strcpy( op, "LH0" );

      op[ 2 ] = (char)('0' + (B & 1));
      
      decode_diadic_address( op_code, op, TYPE_SIGNED, FALSE );
      
      break;
      
    case OP_LHU:
      strcpy( op, "LHU0" );

      op[ 3 ] = (char)('0' + (B & 1));
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    case OP_MH:
      strcpy( op, "MH0" );

      op[ 2 ] = (char)('0' + (B & 1));
      
      decode_diadic_address( op_code, op, TYPE_UNSIGNED, FALSE );
      
      break;

    default:
      return;
    }

  return;

} /* decode_non_word_load */

/*}}}*/
/*{{{  decode_special_triadic() */

/*
 * decodes a multiple parallel triadic op code
 */

static void
decode_special_triadic( long int op_code ) /* instruction */
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

  src4 =  op_code        & 0xff;
  src3 = (op_code >>  8) & 0xff;
  src2 = (op_code >> 16) & 0x7;
  src1 = (op_code >> 19) & 0x7;
  dst2 = ((op_code >> 22) & 0x1) + 2;
  dst1 = (op_code >> 23) & 0x1;
  
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
    case B_00:	/* src3 op1 src4, src1 op2 src2 */
      
      myprint( "%s", op1 );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      myprint( ", " );
      
      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      myprint( ", %s", _regname( dst1 ) );

      new_line();
      
      pad_to( COL_OP_CODE );

      myprint( "|| %s", op2 );

      pad_to( COL_OPERAND2 );
      
      myprint( "%s, %s, %s", _regname( src1 ), _regname( src2 ), _regname( dst2 ) );

      break;
      
    case B_01:	/* src3 op1 src1, src4 op2 src2 */
      
      myprint( "%s", op1 );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      myprint( ", %s, %s", _regname( src1 ), _regname( dst1 ), op2 );

      new_line();

      pad_to( COL_OP_CODE );
      
      myprint( "|| %s", op2 );

      pad_to( COL_OPERAND2 );
      
      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      myprint( ", %s, %s", _regname( src2 ), _regname( dst2 ) );

      break;
      
    case B_10:	/* src1 op1 src2, src3 op2 src4 */
      
      myprint( "%s", op1 );

      pad_to( COL_OPERAND );

      myprint( "%s, %s, %s", _regname( src1 ), _regname( src2 ), _regname( dst1 ) );

      new_line();

      pad_to( COL_OP_CODE );
      
      myprint( "|| %s ", op2 );

      pad_to( COL_OPERAND2 );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      myprint( ", " );

      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      myprint( ", %s", _regname( dst2 ) );

      break;
      
    case B_11:	/* src3 op1 src1, src2 op2 src4 */
      
      myprint( "%s", op1 );

      pad_to( COL_OPERAND );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      myprint( ", %s, %s", _regname( src1 ), _regname( dst1 ) );

      new_line();

      pad_to( COL_OP_CODE );
      
      myprint( "|| %s", op2 );

      pad_to( COL_OPERAND2 );
      
      myprint( "%s, ", _regname( src2 ) );

      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      myprint( ", %s", _regname( dst2 ) );

      break;
    }
  
  return;
  
} /* decode_special_triadic */

/*}}}*/
/*{{{  decode_non_word_load_or_triadic() */

static void
decode_non_word_load_or_triadic( long int op_code ) /* instruction */
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
    case B_01:	return;
    case B_10:	return;
    case B_11:	decode_non_word_load(   op_code ); break;
    }

  return;

} /* decode_non_word_load_or_triadic */

/*}}}*/
/*{{{  decode_traidic_parallel_addressing() */

static void
decode_triadic_parallel_addressing(
				   long int	op_code,
				   char *	first_op,
				   char *	second_op )
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
  
  pad_to( COL_OP_CODE );
  
  myprint( "|| %s", second_op );

  pad_to( COL_OPERAND2 );
  
  myprint( "%s, ", _regname( src3 ) );
      
  decode_indirect_address( dst2 >> 3, dst2 & 7, 1 );
      
  return;
      
} /* decode_triadic_parallel_addressing */

/*}}}*/
/*{{{  decode_diadic_parallel_addressing() */

static void
decode_diadic_parallel_addressing(
				  long int	op_code,
				  char *	first_op,
				  char *	second_op )
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

  myprint( "%s", first_op );

  pad_to( COL_OPERAND );
  
  decode_indirect_address( src2 >> 3, src2 & 7, 1 );

  myprint( ", %s", _regname( dst1 ) );

  new_line();
  
  pad_to( COL_OP_CODE );
  
  myprint( "|| %s", second_op );

  pad_to( COL_OPERAND2 );
  
  myprint( "%s, ", _regname( src3 ) );
      
  decode_indirect_address( dst2 >> 3, dst2 & 7, 1 );
  
  return;
      
} /* decode_triadic_parallel_addressing */

/*}}}*/
/*{{{  decode_parallel_op() */

  
static void
decode_parallel_op( long int op_code ) /* instruction */
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

      pad_to( COL_OP_CODE );
      
      myprint( "|| STF" );

      pad_to( COL_OPERAND2 );

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
      
      pad_to( COL_OP_CODE );
      
      myprint( "|| STI" );

      pad_to( COL_OPERAND2 );

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

      pad_to( COL_OP_CODE );
      
      myprint( "|| LDF" );

      pad_to( COL_OPERAND2 );
      
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
      
      pad_to( COL_OP_CODE );

      myprint( "|| LDI" );

      pad_to( COL_OPERAND2 );
      
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
      return;
    }

  return;

} /* decode_parallel_op */

/*}}}*/
/*{{{  decode_triadic_address() */

/*
 * deocdes a 'C40 triadic op code
 */

static void
decode_triadic_address(
		       long int		op_code,	/* instruction                    */
		       char *		instruction,	/* string identifying the op code */
		       immed_type	type )		/* type of operation              */
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
	  return;	  
	}
    }

  pdest = _regname( dest );
  
  /* build combined addressing type */

  addr = (op_code >> 28) & 0x1;

  addr <<= 2;

  addr |= ((op_code >> 21) & 0x3);

  /* extract source fields */
  
  src2 =  op_code       & 0xff;
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
	return;
      else if (type == TYPE_SIGNED)
	{
	  op_code = mask_and_sign_extend_word( op_code, 0xff );
	  
	  myprint( "%-3ld, %s, %s",
		  op_code,
		  _regname( src1 & 0x3f ),
		  pdest );	  
	}      
      else
	{
	  myprint( "%#-3lx, %s, %s",
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
	  return;
	}
      else if (type == TYPE_SIGNED)
	{
	  myprint( "%-3ld, *+AR%.1ld(%.2lu), %s",
		  mask_and_sign_extend_word( op_code, 0xff ),
		  src1 & 0x7, (src1 >> 3),
		  pdest );      
	}
      else
	{
	  myprint( "%#-3lx, *+AR%.1ld(%.2lu), %s",
		  op_code & 0xff,
		  src1 & 0x7, (src1 >> 3),
		  pdest );
	}
     
      break;
      
    case B_111:	/* src1 = indirect, src2 = indirect */
      
      myprint( "*+AR%.1ld(%.2lu), *+AR%.1ld(%.2lu), %s",
	      src2 & 0x7, (src2 >> 3),
	      src1 & 0x7, (src1 >> 3),
	      pdest );
      break;
      
    default:
      break;
    }

  return;
  
} /* decode_triadic_address */

/*}}}*/
/*{{{  decode_monadic_op() */

static void
decode_monadic_op(
		  long int	op_code,
		  char *	instruction )
{
  myprint( "%s", instruction );

  pad_to( COL_OPERAND );
  
  myprint( "%s", _regname( (op_code >> 16) & 0x1f ) );

  return;
  
} /* decode_mondadic_op */

/*}}}*/
/*{{{  decode_sequential_op() */

  
static void
decode_sequential_op( long int op_code ) /* instruction */
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
	case OP_ASH3:	decode_triadic_address( op_code, "ASH3"    , TYPE_SIGNED   ); return;
	case OP_CMPF3:	decode_triadic_address( op_code, "CMPF3"   , TYPE_FLOATING ); return;
	case OP_CMPI3:	decode_triadic_address( op_code, "CMPI3"   , TYPE_SIGNED   ); return;
	case OP_LSH3:	decode_triadic_address( op_code, "LSH3"    , TYPE_SIGNED   ); return;
	case OP_MPYF3:	decode_triadic_address( op_code, "MPYF3"   , TYPE_FLOATING ); return;
	case OP_MPYI3:	decode_triadic_address( op_code, "MPYI3"   , TYPE_SIGNED   ); return;
	case OP_SUBB3:	decode_triadic_address( op_code, "SUBB3"   , TYPE_SIGNED   ); return;
	case OP_SUBF3:	decode_triadic_address( op_code, "SUBF3"   , TYPE_FLOATING ); return;
	case OP_SUBI3:	decode_triadic_address( op_code, "SUBI3"   , TYPE_SIGNED   ); return;
	case OP_MPYSHI3:decode_triadic_address( op_code, "MPYSHI3" , TYPE_SIGNED   ); return;
	case OP_AND3:	decode_triadic_address( op_code, "AND3"    , TYPE_SIGNED   ); return;
	case OP_ANDN3:	decode_triadic_address( op_code, "ANDN3"   , TYPE_SIGNED   ); return;
	case OP_OR3:	decode_triadic_address( op_code, "OR3"     , TYPE_SIGNED   ); return;
	case OP_TSTB3:	decode_triadic_address( op_code, "TSTB3"   , TYPE_SIGNED   ); return;
	case OP_XOR3:	decode_triadic_address( op_code, "XOR3"    , TYPE_SIGNED   ); return;
	case OP_MPYUHI3:decode_triadic_address( op_code, "MPYUHI3" , TYPE_SIGNED   ); return;
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
	case OP_IDLE:	myprint( "IDLE"                          ); return;
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
	case OP_NOP:	myprint( "NOP"                           ); return;
	case OP_NORM:	decode_diadic_address( op_code, "NORM"   , TYPE_FLOATING, FALSE ); return;
	case OP_NOT:	decode_diadic_address( op_code, "NOT"    , TYPE_UNSIGNED, FALSE ); return;
	case OP_POP:	decode_monadic_op(     op_code, "POP"    ); return;
	case OP_POPF:	decode_monadic_op(     op_code, "POPF"   ); return;
	case OP_PUSH:	decode_monadic_op(     op_code, "PUSH"   ); return;
	case OP_PUSHF:	decode_monadic_op(     op_code, "PUSHF"  ); return;
	case OP_OR:	decode_diadic_address( op_code, "OR"     , TYPE_UNSIGNED, FALSE ); return;
	case OP_RND:	decode_diadic_address( op_code, "RND"    , TYPE_FLOATING, FALSE ); return;
	case OP_ROL:	decode_monadic_op(     op_code, "ROL"    ); return;
	case OP_ROLC:	decode_monadic_op(     op_code, "ROLC"   ); return;
	case OP_ROR:	decode_monadic_op(     op_code, "ROR"    ); return;
	case OP_RORC:	decode_monadic_op(     op_code, "RORC"   ); return;
	case OP_RPTS:	decode_diadic_address( op_code, "RPTS"   , TYPE_UNSIGNED, FALSE ); return;
	case OP_STF:	decode_diadic_address( op_code, "STF"    , TYPE_FLOATING, TRUE  ); return;
	case OP_STFI:	decode_diadic_address( op_code, "STFI"   , TYPE_FLOATING, TRUE  ); return;
	case OP_STI:	decode_integer_store(  op_code           ); return;
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
/*{{{  disassemble() */

void
disassemble(
	    unsigned long	instruction,
	    unsigned long	pc,
	    int			flag )
{
  /*
   * bits 30 and 31 encode instruction type
   */

  if (flag)
    current_column = 80;
  else
    current_column = 0;
  
  switch ((instruction >> 30) & 0x3)
    {
    default:
    case 0: decode_sequential_op(            instruction     ); return;
    case 1: decode_flow_control(             instruction, pc ); return;
    case 2: decode_non_word_load_or_triadic( instruction     ); return;
    case 3: decode_parallel_op(              instruction     ); return;
    }

} /* disassemble */

/*}}}*/

/*}}}*/

/* end of c40objdis.c */
