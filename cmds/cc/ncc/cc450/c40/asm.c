/*{{{  Comments */

/* c40/asm.c: Copyright (C) Codemist Ltd., 1991.                       */
/* c40/asm.c: Copyright (C) Perihelion Software Ltd., 1991, 1992, 1993, 1994. */

/* version 1 */

/* Assembler output is routed to asmstream, annotated if FEATURE_ANNOTATE.  */
/* See aoutobj.c for more details on datastructures.                        */

/* exports: asmstream, 
   display_assembly_code, asm_header, asm_trailer */

/*}}}*/
/*{{{  Includes */

#include <stdarg.h>
#include <errno.h>

#ifdef __STDC__
#  include <string.h>
#else
#  include <strings.h>
#endif

#include <ctype.h>

#include "helios.h"
#ifndef __C40
#define __C40
#endif
#include "module.h"
#include "globals.h"
#include "mcdep.h"
#include "xrefs.h"
#include "store.h"
#include "codebuf.h"
#include "ops.h"
#include "mcdpriv.h"
#include "builtin.h"
#include "version.h"
#include "errors.h"
#include "peep.h"

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
/*{{{  Macros */

#define toevensex( x, y )	(x)    /* for cross compilation one day */

/*}}}*/
/*{{{  Constants */

#define	ASM_COMMENT_CHAR	';'

/*}}}*/
/*{{{  Variables */

FILE *			asmstream   = NULL;
Symstr *		sym_reladdr = NULL;
static bool		patch_pend  = FALSE;

static int32	fncount;	/* maybe should be more global */
static bool	asm_error;
static int32	current_pos;	/* current byte offset into code */

/*}}}*/
/*{{{  Code */

/*{{{  asmf() */

/*
 * print a string to the assembler output stream
 */

void
asmf( const char * format, ... )
{
  va_list	args;
	

  if (asmstream == NULL)
    return;
  
  va_start( args, format );

/*  fflush( asmstream ); */
	
/*  fseek( asmstream, 0L, SEEK_END ); */

  (void) vfprintf( asmstream, (char *)format, args );

/* fflush( asmstream ); */
	
  va_end( args );

  return;
  
} /* asmf */

/*}}}*/
/*{{{  asm_blank() */

static void
asm_blank( int32 n )
{
  while (n-- > 0)
    asmf( "%c\n", ASM_COMMENT_CHAR );
    
  return;
    
} /* asm_blank */

/*}}}*/

#ifndef TARGET_IS_C40
/*{{{  asm_padcol8() */

static int32
asm_padcol8( int32 n )
{
  if (!annotations)
    n = 7;      /* compact the asm file */
    
  while (n < 8)
    fputc( ' ', asmstream ), n++;
    
  return n;
    
} /* asm_padcol8 */

/*}}}*/
#endif
/*{{{  pr_chars() */

static void
pr_chars( int32 w )   /* works on both sex machines */
{
  int	i, c;
    

  fputc( '\'', asmstream );
    
  for (i = 0; i < sizeof( int32 ); i++)
    {
      switch (c = ((unsigned char *)&w)[ i ])
	{
	case '\\':
	case '\'':
	case '\"':
	  break;
	  
	case '\a':
	  c = 'a';
	  break;
	  
	case '\b':
	  c = 'b';
	  break;
	  
	case CHAR_FF:
	  c = 'f';
	  break;
	  
	case '\n':
	  c = 'n';
	  break;
	  
	case CHAR_CR:
	  c = 'r';
	  break;
	  
	case '\t':
	  c = 't';
	  break;
	  
	case CHAR_VT:
	  c = 'v';
	  break;
	  
	default:
	  if (c < ' ' || c >= 127)
	    asmf( "\\%o", (int)c );
	  else
	    putc( c, asmstream );
	  continue;
	}
      
      putc( '\\', asmstream );
      putc( c,    asmstream );
    }
  
  fputc( '\'', asmstream );
  
  return;
  
} /* pr_chars */

/*}}}*/
/*{{{  pr_asmname() */

/*
 * prints the name of symbol 'sym' to the file 'stream'
 * sets 'asm_error' if the symbol does not exist
 */

static void
pr_asmname( Symstr * sym )
{
  char *	s = sym == 0 ? (asm_error = 1, "?") : symname_( sym );
    

  asmf( s );

  return;  
    
} /* pr_asmname */

/*}}}*/
/*{{{  decode_external() */

static unsigned32 xroffset = 0;

/* decode_external checks if (and by what) a location is to be relocated.  */

static Symstr *
decode_external( int32 p ) /* p is now a byte address */
{
  CodeXref *	x;
  
  
  for (x = codexrefs; x != NULL; x = x->codexrcdr)    /* SLOW loop !!! */
    {
      if (p == (x->codexroff & 0x00ffffffL))
	{
	  switch (x->codexroff & 0xff000000UL)
	    {
	    default:
	      xroffset = 0;
	      break;
	      
	    case X_DataAddr:
	    case X_DataAddr1:
	    case X_FuncAddr:
	    case X_absreloc:
	      xroffset = x->codexrlitoff;
	    }
	  
	  return x->codexrsym;
	}
    }
  
  /* syserr( syserr_decode_external );     *//* may exploit zero return one day */
  
  return 0;	/* not an external reference */
  
} /* decode_external */

/*}}}*/
/*{{{  printlabelname() */

static bool
printlabelname(
	       int32	offset,
	       bool	flag_errs )
{
  LabList *	p;
  
  
  /*
   * For data references, ignore the function name: it will be wrong for backward
   * references to the literal pool of another function.  (Labels which are the
   * subject of data references have already been removed from asm_lablist).
   * For code references, generate a label of the form L<label>F<proc>.
   * Such label names seem a bit long and ugly.
   */
  
  for (p = asm_lablist ; p != NULL ; p = p->labcdr)
    {
      LabelNumber *	lab = p->labcar;
      
      
      if ((lab->u.defn & 0x00ffffffL) == offset)
	{
	  asmf( "L%ldF%ld", (long)(lab_name_( lab ) & 0x7fffffffL), (long)fncount );
	  
	  return 1;
	}
    }
  
  if (flag_errs)
#if 0
    asmf( "<unknown destination> offset + base = %ld, current pos = %ld, offset = %ld",
	 (offset + codebase) / sizeof_int, current_pos / sizeof_int, offset / sizeof_int);
#else
  asmf( "0" );
#endif
  
  return 0;
  
} /* printlabelname */

/*}}}*/
/*{{{  decode_addr() */

/*
 * Decodes the address addr (referenced at code offset off in current function)
 * Works for external addresses
 */

static void
decode_addr( int32 offset )
{   
  Symstr *	name = decode_external( codebase + current_pos );
  
  
  /* First, is it an external name ? */

  if (name == bindsym_( codesegment ))
    {
      LabList *	p;
      

      /*
       * although we have found it in codexrefs, we may still be able to
       * produce a local label name for it
       */
      
      for (p = asm_lablist ; p != NULL ; p = p->labcdr)
	{
	  LabelNumber *	lab = p->labcar;
	  
	  
	  if ((codebase + (lab->u.defn & 0x00ffffff)) / sizeof_int == xroffset)
	    {
	      asmf( "L%ldF%ld", (long)(lab_name_( lab ) & 0x7fffffffL), (long)fncount );
	      
	      return;
	    }
	}
    }

  if (name != 0)
    {
      fputc( '_', asmstream );
      
      pr_asmname( name );
      
      if (xroffset != 0)
	asmf( " + 0x%lx", xroffset );

      return;
    }

  /* Otherwise, assume it is a local label */
  
  (void) printlabelname( codebase + current_pos + offset * sizeof_int, TRUE );	/* not right ??? */
  
  return;
  
} /* decode_addr */

/*}}}*/
/*{{{  _regname() */

/* Disassembler routines                                                 */

static char *	regstr =
  "R0 \0R1 \0R2 \0R3 \0R4 \0R5 \0R6 \0R7 \0"
  "AR0\0AR1\0AR2\0AR3\0AR4\0AR5\0AR6\0AR7\0"
  "DP \0IR0\0IR1\0BK \0SP \0ST \0DIE\0IIE\0"
  "IIF\0RS \0RE \0RC \0R8 \0R9 \0R10\0R11\0";

#define _regname( r ) ((((r) & 0x1f) * 4) + regstr)

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
			int32	mode,
			int32	addr,
			int32	disp )
{
  switch (mode)
    {
    case B_00000:	asmf( "*+AR%ld(%ld)",         addr, disp ); break;
    case B_00001:	asmf( "*-AR%ld(%ld)",         addr, disp ); break;
    case B_00010:	asmf( "*++AR%ld(%ld)",        addr, disp ); break;
    case B_00011:	asmf( "*--AR%ld(%ld)",        addr, disp ); break;
    case B_00100:	asmf( "*AR%ld++(%ld)",        addr, disp ); break;
    case B_00101:	asmf( "*AR%ld--(%ld)",        addr, disp ); break;
    case B_00110:	asmf( "*AR%ld++(%ld)%%",      addr, disp ); break;
    case B_00111:	asmf( "*AR%ld--(%ld)%%",      addr, disp ); break;
    case B_01000:	asmf( "*+AR%ld(IR0)",         addr       ); break;
    case B_01001:	asmf( "*-AR%ld(IR0)",         addr       ); break;
    case B_01010:	asmf( "*++AR%ld(IR0)",        addr       ); break;
    case B_01011:	asmf( "*--AR%ld(IR0)",        addr       ); break;
    case B_01100:	asmf( "*AR%ld++(IR0)",        addr       ); break;
    case B_01101:	asmf( "*AR%ld--(IR0)",        addr       ); break;
    case B_01110:	asmf( "*AR%ld++(IR0)%%",      addr       ); break;
    case B_01111:	asmf( "*AR%ld--(IR0)%%",      addr       ); break;
    case B_10000:	asmf( "*+AR%ld(IR1)",         addr       ); break;
    case B_10001:	asmf( "*-AR%ld(IR1)",         addr       ); break;
    case B_10010:	asmf( "*++AR%ld(IR1)",        addr       ); break;
    case B_10011:	asmf( "*--AR%ld(IR1)",        addr       ); break;
    case B_10100:	asmf( "*AR%ld++(IR1)",        addr       ); break;
    case B_10101:	asmf( "*AR%ld--(IR1)",        addr       ); break;
    case B_10110:	asmf( "*AR%ld++(IR1)%%",      addr       ); break;
    case B_10111:	asmf( "*AR%ld--(IR1)%%",      addr       ); break;
    case B_11000:	asmf( "*AR%ld",               addr       ); break;
    case B_11001:	asmf( "*AR%ld++(IR0)B",       addr       ); break;
    default:
      syserr( syserr_unknown_indirect_mode, mode );
    }
  
  return;
  
} /* decode_indirect_addressing */

/*}}}*/
/*{{{  mask_and_sign_extend_word() */

/*
 * Extracts the bits specified by 'mask' from the word 'value'
 * If necessary the resulting word is sign extended.
 * 'mask' must be a contigous set of bits starting from
 * the least significant bit.
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
decode_short_float( int32 number )
{
  static char	buffer[ 20 ];
  int32		e;
  int32		s;
  int32		f;
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
  
#ifdef __HELIOSC40
  
  /* convert to C40 single precision */
  
  converter.l = (e << 24) | (s << (23 - 11)) | (f << (22 - 10));
  
#else
  
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
#endif /* __HELIOSC40 */
  
  /* convert to a string */
  
  sprintf( buffer, "%f", converter.f );
  
  /* return the string */
  
  return buffer;
  
} /* decode_short_float */

/*}}}*/
/*{{{   * decodes the addressing modes () */

/*
 * decodes the addressing modes (the 'G' field)
 * of a 'C40 diadic op code
 */

static void
decode_diadic_address(
		      int32		op_code,	/* instruction */
		      char *		instruction,	/* string identifying instruction */
		      immed_type	type,		/* type of immediate values */
		      int32		store )		/* non zero if a store operation */
{
  char *	pdest;
  int32		dest;
  int32		src;
  
  
  /*
   * we have a diaidic op code
   *
   * bits 16 - 20 encode destination
   * bits 21 - 22 encode addressing type
   * bits 23 - 28 encode op code
   * bits 29 - 31 are 000
   */
  
  if ((op_code >> 23) == OP_RPTS || (op_code >> 23) == OP_IACK)
    {
      /* this op code has no destination */
      
      pdest = NULL;
      dest  = 0;      
    }
  else
    {
      dest = (op_code >> 16) & 0x1f;
      
      if (type == TYPE_FLOATING && (op_code >> 23) != OP_FIX)
	{
	  /* the destination (or source) register of a floating point op must be R0 - R11 */
	  
	  if (!is_extended_precision_register( dest ))
	    {
	      syserr( syserr_destination_not_FP, instruction, _regname( dest ) );
	    }
	}
      
      pdest = _regname( dest );
    }
  
  asmf( "%s\t", instruction );
  
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
	  syserr( syserr_bad_source_value, instruction, op_code );
	}
      
      src = op_code & 0x1f;
      
      if (type == TYPE_FLOATING)
	{
	  /* the source register of a floating point op must be R0 - R11 */
	  
	  if (!is_extended_precision_register( src ))
	    {
	      syserr( syserr_source_not_FP, _regname( src ), instruction, op_code );
	    }
	}
      
      asmf( "%s", _regname( src ) );
      
      if (pdest && (dest != src || !is_monadic( op_code )))
	asmf( ", %s", pdest );
      
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
	asmf( "%s, @%#04.4lx", pdest, (op_code & 0xffff) );
      else if (pdest)
	asmf( "@%#04.4lx, %s", (op_code & 0xffff), pdest );
      else
	asmf( "@%#04.4lx", (op_code & 0xffff) );
      
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
	  int32		disp = op_code & 0xff;
	  int32		addr = (op_code >> 8) & 0x7;
	  
	  
	  if (store)
	    {
	      asmf( "%s, ", pdest );
	      
	      decode_indirect_address( op_code >> 11 & 0x1f, addr, disp );
	    }
	  else
	    {
	      decode_indirect_address( (op_code >> 11) & 0x1f, addr, disp );
	      
	      if (pdest)
		asmf( ", %s", pdest );
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
      
      if (sym_reladdr != NULL)
	{
	  /* we know how this constant was calculated ... */

	  if (exporting_routines)
	    asmf( "L%ldF%ld - .%s, %s", (long)(lab_name_( exporting_routines ) & 0x7fffffffL),
		 (long)fncount, symname_( sym_reladdr ), pdest );	  
	  else
	    /*
	     * XXX
	     * 
	     * The + 1 below comes from the fact that the SUBI is the third instruction after
	     * the LAJ 4 generated by the code loading the address of a function 
	     * [gen.c: load_address_constant()].  This is only true whilst a NOP is being
	     * inserted after the LAJ because of a silicon bug.  Hence if the bug is fixed
	     * and the NOP is removed, the +1 will have to be changed (probably to +2).
	     */

	    asmf( ".%s + 1, %s", symname_( sym_reladdr ), pdest );
	}
      else if (type == TYPE_FLOATING)
	{
	  asmf( "%s, %s", decode_short_float( dest ), pdest );
	}
      else if (dest < 500 && dest > -500 && type == TYPE_SIGNED)
	{
	  asmf( "%-3ld, %s", dest, pdest );
	}
      else if (pdest)
	{
	  asmf( "%#-3lx, %s", dest, pdest );
	}
      else
	{
	  asmf( "%#-3lx", dest );
	}
      
      if (sym_reladdr != NULL)
	{
	  /*
	   * be ready to print out the symbol that is
	   * responsible for the value just display.
	   *
	   * NB/ we must not replace the value in the
	   * instruction with the symbol's name, as it
	   * is almost certain that some calculations
	   * have been performed on the value.
	   */
	  
	  asmf( "\t%c", ASM_COMMENT_CHAR );
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
decode_integer_store( int32 op_code )
{
  int32		dest;
  
  
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
      
      asmf( "STIK\t%-3ld, @%#04.4lx",
	      mask_and_sign_extend_word( dest, 0x1f ), (op_code & 0xffff) );
      
      break;
      
    case 1: /* direct register */
      /*
       * bits  0 - 15 are an unsigned offset to the data page register
       * bits 16 - 20 encode source register
       * bits 21 - 31 are 00010101001
       */
      
      asmf( "STI\t%s, @%#04.4lx", _regname( dest ), (op_code & 0xffff) );
      
      break;
      
    case 2: /* indirect register */
      /*
       * bits  0 -  7 encode the displacment
       * bits  8 - 10 encode the address register
       * bits 11 - 15 encode the mode
       * bits 16 - 20 encode source register
       * bits 21 - 31 are 00010101010
       */
      
      asmf( "STI\t%s, ", _regname( dest ) );
      
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
      
      asmf( "STIK\t%-3ld, ", mask_and_sign_extend_word( dest, 0x1f ) );
      
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
		    int32	op_code, 	/* instruction				*/
		    bool	flag_errs )	/* TRUE if errors should be reported	*/
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
		      asmf( "RPTBD\t%s", _regname( op_code ) );
		    }
		  else
		    {
		      asmf( "RPTB\t%s", _regname( op_code ) );
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
		      
		      asmf( "RETS%s", _condname( op_code >> 16 ) );
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
			  asmf( "RETI%sD", _condname( op_code >> 16 ) );
			}
		      else
			{
			  asmf( "RETI%s", _condname( op_code >> 16 ) );
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
			  asmf( "LDPE\t%s, %s",
			       _regname( op_code ),
			       (((op_code >> 16) & 0xf) == 0) ? "IVTP" : "TVTP" );
			}
		      else
			{
			  asmf( "LDEP\t%s, %s",
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
		      
		      asmf( "%s\t%lu", op, op_code & 0x1ff );
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
		  
		  if (op_code & (1 << 25))
		    {
		      op_code = mask_and_sign_extend_word( (unsigned long)op_code, 0xffffL );
		      
		      asmf( "%s\t%+ld", op, op_code );
		    }
		  else
		    {
		      asmf( "%s\t%s", op, _regname( op_code & 0x1f ) );
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
		  char	op[ 10 ];
		  int32	addr;
		  int	offset = 0;
		  
		  
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
		  
		  if (op_code & (1 << 25))
		    {
		      op_code = mask_and_sign_extend_word( (unsigned long)op_code, 0xffffL );
		      
		      asmf( "%s\tAR%1ld, ", op, addr );
		      
		      if (!printlabelname( current_pos + (op_code + offset) * sizeof_int, flag_errs ))
			{
			  asmf( "\t%c offset = %+ld", ASM_COMMENT_CHAR, (op_code + offset) );
			}
		    }
		  else
		    {
		      asmf( "%s\tAR%1ld, %s", op, addr, _regname( op_code & 0x1f ) );
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
		    case B_010:	syserr( syserr_unknown_op_code, op_code ); return;
		    case B_011:	strcat( op, "AT" ); offset = 3; break;
		    case B_100:	syserr( syserr_unknown_op_code, op_code ); return;
		    case B_101:	strcat( op, "AF" ); offset = 3; break;
		    case B_110:	syserr( syserr_unknown_op_code, op_code ); return;
		    case B_111:	syserr( syserr_unknown_op_code, op_code ); return;
		    }
		  
		  if (op_code & (1 << 25))
		    {
		      op_code = mask_and_sign_extend_word( (unsigned long)op_code, 0xffffL );
		      
		      asmf( "%s\t", op );
		      
		      if (!printlabelname( current_pos + (op_code + offset) * sizeof_int, flag_errs ))
			{
			  asmf( "\t%c offset = %+ld", ASM_COMMENT_CHAR, op_code + offset );
			}
		    }
		  else
		    {
		      asmf( "%s\t%s", op, _regname( op_code & 0x1f ) );
		    }
		}
	    }
	  else
	    {
	      int	offset = 0;
	      
	      
	      /* bits 27 - 31 are 01100 */
	      
	      switch ((op_code >> 24) & 0x7)
		{
		case B_000:	asmf( "BR\t"    ); offset = -2; break;
		case B_001:	asmf( "BRD\t"   ); offset = -4; break;
		case B_010:	asmf( "CALL\t"  ); offset = -2; break;
		case B_011:	asmf( "LAJ\t"   ); offset = -4; break;
		case B_100:	asmf( "RPTB\t"  ); offset = 0;  break;
		case B_101:	asmf( "RPTBD\t" ); offset = 3;  break;
		case B_110:	asmf( "SWI\t"   ); return;
		default:
		case B_111:	syserr( syserr_unknown_op_code, op_code ); return;					
		}

	      op_code = mask_and_sign_extend_word( (unsigned long)op_code, 0xffffffL );
	      
	      if (op_code == 1 && offset == -4)
		{
		  /*
		   * A 'LAJ' with an offset of 1 is a special case.
		   * What it is doing is putting the PC into R11 so
		   * that the next few instructions can be PC relative
		   * (This is the only way to access the value of the PC
		   * on the 'C40).  See the code for load_address_constant()
		   * in c40/gen.c
		   */
		  
		  asmf( "4\t%c <fetching PC>", ASM_COMMENT_CHAR );
		}
	      else if (offset < 0)
		{
		  if (sym_reladdr != NULL)
		    {
		      /*
		       * this instruction will have been issued as a
		       * LIT_RELADDR and so the function name will be
		       * decoded later
		       */
		    }
		  else
		    {
#if 1
		      if (!printlabelname( current_pos + (op_code - offset - 1) * sizeof_int, FALSE ))
			{
			  asmf( "%ld %c%c offset = %+ld", op_code - offset - 1,
			       patch_pend ? ')' : ' ',
			       ASM_COMMENT_CHAR, op_code - offset - 1 );
			}
#else
		      asmf( "%ld", op_code - offset - 1 );
#endif
		    }
		}
	      else
		{
		  if (!printlabelname( current_pos + (op_code + offset) * sizeof_int, flag_errs ))
		    {
		      asmf( "%c offset = %+ld", ASM_COMMENT_CHAR, op_code + offset );
		    }
		}
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
/*{{{  decode_non_word_load() */

/*
 * decodes a 'C40 op code concerened with loading non-word quantities
 */

static void
decode_non_word_load( int32 op_code ) /* instruction */
{
  char	op[ 5 ];
  int	B;
  
  
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

  B = (int)((op_code >> 23) & 0x03);
  
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
      syserr( syserr_unknown_op_code, op_code );
      break;
    }

  return;

} /* decode_non_word_load */

/*}}}*/
/*{{{  decode_special_triadic() */

/*
 * decodes a multiple parallel triadic op code
 */

static void
decode_special_triadic( int32 op_code ) /* instruction */
{
  char *	op1;
  char *	op2;
  int32		src1;
  int32		src2;
  int32		src3;
  int32		src4;
  int32		dst1;
  int32		dst2;
  
  
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

  src4 =  op_code         & 0xff;
  src3 = (op_code >>   8) & 0xff;
  src2 = (op_code >>  16) & 0x7;
  src1 = (op_code >>  19) & 0x7;
  dst2 = ((op_code >> 22) & 0x1) + 2;
  dst1 = (op_code >>  23) & 0x1;
  
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
      
      asmf( "%s\t", op1 );

      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      asmf( ", " );
      
      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      asmf( ", %s  || %s\t%s, %s, %s",
	      _regname( dst1 ), op2, _regname( src1 ), _regname( src2 ), _regname( dst2 ) );

      break;
      
    case B_01:	/* src3 op1 src1, src4 op2 src2 */
      
      asmf( "%s\t", op1 );
      
      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      asmf( ", %s, %s  || %s\t",
	      _regname( src1 ), _regname( dst1 ), op2 );

      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      asmf( ", %s, %s", _regname( src2 ), _regname( dst2 ) );

      break;
      
    case B_10:	/* src1 op1 src2, src3 op2 src4 */
      
      asmf( "%s\t%s, %s, %s  || %s\t",
	      op1, _regname( src1 ), _regname( src2 ), _regname( dst1 ), op2 );

      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      asmf( ", " );

      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      asmf( ", %s", _regname( dst2 ) );

      break;
      
    case B_11:	/* src3 op1 src1, src2 op2 src4 */
      
      asmf( "%s\t", op1 );

      decode_indirect_address( src3 >> 3, src3 & 7, 1 );

      asmf( ", %s, %s  || %s\t, %s, ",
	      _regname( src1 ), _regname( dst1 ), op2, _regname( src2 ) );

      decode_indirect_address( src4 >> 3, src4 & 7, 1 );

      asmf( ", %s", _regname( dst2 ) );

      break;
    }
  
  return;
  
} /* decode_special_triadic */

/*}}}*/
/*{{{  decode_non_word_load_or_triadic() */

static void
decode_non_word_load_or_triadic( int32 op_code ) /* instruction */
{
  /*
   * we have either a special form of triadic op code or a non-word data transfer
   *
   * bits 28 - 29 encode form
   * bits 30 - 31 are 10
   */

  switch ((op_code >> 28) & 0x3)
    {
    case B_00:	decode_special_triadic(         op_code ); break;
    case B_01:	syserr( syserr_unknown_op_code, op_code ); break;
    case B_10:	syserr( syserr_unknown_op_code, op_code ); break;
    case B_11:	decode_non_word_load(           op_code ); break;
    }

  return;

} /* decode_non_word_load_or_triadic */

/*}}}*/
/*{{{  decode_triadic_parallel_addressing() */

static void
decode_triadic_parallel_addressing(
				   int32	op_code,
				   char *	first_op,
				   char *	second_op )
{
  unsigned long	src1;
  unsigned long	src2;
  unsigned long	src3;
  unsigned long	dst1;
  unsigned long	dst2;

  
  /*
   * we have a triadic parallel op code of the form:
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
  
  asmf( "%s\t", first_op );
  
  decode_indirect_address( src2 >> 3, src2 & 7, 1 );
  
  asmf( ", %s, %s  || %s\t%s, ",
	  _regname( src1 ), _regname( dst1 ), second_op, _regname( src3 ) );
      
  decode_indirect_address( dst2 >> 3, dst2 & 7, 1 );
      
  return;
      
} /* decode_triadic_parallel_addressing */

/*}}}*/
/*{{{  decode_diadic_parallel_addressing() */

  
static void
decode_diadic_parallel_addressing(
				  int32		op_code,
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
      syserr( syserr_bad_parallel_addressing, op_code );
    }
      
  asmf( "%s\t", first_op );
      
  decode_indirect_address( src2 >> 3, src2 & 7, 1 );
      
  asmf( ", %s  || %s\t%s, ", _regname( dst1 ), second_op, _regname( src3 ) );
      
  decode_indirect_address( dst2 >> 3, dst2 & 7, 1 );
  
  return;
      
} /* decode_triadic_parallel_addressing */

/*}}}*/
/*{{{  decode_parallel_op() */

  
static void
decode_parallel_op( int32 op_code ) /* instruction */
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
	  syserr( syserr_bad_parallel_addressing, op_code );
	}
      
      asmf( "STF\t%s, ", _regname( field5 ) );
      
      decode_indirect_address( field1 >> 3, field1 & 7, 1 );
      
      asmf( "  || STF\t%s, ", _regname( field3 ) );
      
      decode_indirect_address( field2 >> 3, field2 & 7, 1 );
      
      break;
      
    case OP_STI_STI:
      /* validity tests */

      if (field4 != 0)
	{
	  syserr( syserr_bad_parallel_addressing, op_code );
	}
      
      asmf( "STI\t%s, ", _regname( field5 ) );
      
      decode_indirect_address( field1 >> 3, field1 & 7, 1 );
      
      asmf( "  || STI\t%s, ", _regname( field3 ) );
      
      decode_indirect_address( field2 >> 3, field2 & 7, 1 );
      
      break;
      
    case OP_LDF_LDF:
      /* validity tests */

      if (field3 != 0)
	{
	  syserr( syserr_bad_parallel_addressing, op_code );
	}
      
      asmf( "LDF\t" );
      
      decode_indirect_address( field1 >> 3, field1 & 7, 1 );
      
      asmf( ", %s  || LDF\t", _regname( field5 ) );
      
      decode_indirect_address( field2 >> 3, field2 & 7, 1 );

      asmf( ", %s", _regname( field4 ) );
      
      break;
      
    case OP_LDI_LDI:
      /* validity tests */

      if (field3 != 0)
	{
	  syserr( syserr_bad_parallel_addressing, op_code );
	}
      
      asmf( "LDI\t" );
      
      decode_indirect_address( field1 >> 3, field1 & 7, 1 );
      
      asmf( ", %s  || LDI\t", _regname( field5 ) );
      
      decode_indirect_address( field2 >> 3, field2 & 7, 1 );

      asmf( ", %s", _regname( field4 ) );
      
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
      syserr( syserr_unknown_parallel_op, op_code );
      break;
    }

  return;

} /* decode_parallel_op */

/*}}}*/
/*{{{  decode_triadic_address() */

/*
 * decodes a 'C40 triadic op code
 */

static void
decode_triadic_address(
		       int32		op_code,	/* the triadic instruction        */
		       char *		instruction,	/* string identifying the op code */
		       immed_type	type )		/* type of immediate values       */
{
  char *	pdest;
  int32		dest;
  unsigned long	addr;
  unsigned long	src1;
  unsigned long	src2;
  

  /*
   * we have a triadic op code
   *
   * bits  0 -  7 encode source2
   * bits  8 - 15 encode source1
   * bits 16 - 20 encode destination register
   * bits 21 - 22 encode addessing mode
   * bits 23 - 27 encode op code
   * bit  28      encodes addressing type
   * bits 29 - 31 are 001
   */

  dest = (op_code >> 16) & 0x1f;

  if (type == TYPE_FLOATING)
    {
      /* the destination (or source) register of a floating point op must be R0 - R11 */

      if (!is_extended_precision_register( dest ))
	{
	  syserr( syserr_non_float_dest );
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

  asmf( "%s\t", instruction );
  
  switch (addr)
    {
    case B_000:	/* src1 = register, src2 = register */

      asmf( "%s, %s, %s",
	      _regname( src2 & 0x3f ),
	      _regname( src1 & 0x3f ),
	      pdest );
	      
      break;

    case B_001: /* src1 = indirect, src2 = register */

      asmf( "%s, ", _regname( src2 & 0x3f ) );
      
      decode_indirect_address( src1 >> 3, src1 & 7, 1 );
      
      asmf( ", %s", pdest );
     
      break;
      
    case B_010:	/* src1 = register, src2 = indirect */

      decode_indirect_address( src2 >> 3, src2 & 7, 1 );
      
      asmf( ", %s", _regname( src1 & 0x3f ) );

      asmf( ", %s", pdest );
      
      break;

    case B_011:	/* src1 = indirect, src2 = indirect */

      decode_indirect_address( src2 >> 3, src2 & 7, 1 );

      asmf( ", " );

      decode_indirect_address( src1 >> 3, src1 & 7, 1 );

      asmf( ", %s", pdest );
      
      break;

    case B_100: /* src1 = register, src2 = immediate */

      if (type == TYPE_FLOATING)
	syserr( syserr_no_triadic_FP_imm );
      else if (type == TYPE_SIGNED)
	op_code = mask_and_sign_extend_word( op_code, 0xff );
      else
	op_code &= 0xff;
     
      if (sym_reladdr != NULL)
	{
	  asmf( ".%s, %s, %s",
		  symname_( sym_reladdr ),
		  _regname( src1 & 0x3f ),
		  pdest );

	  sym_reladdr = NULL;
	}
      else if (type == TYPE_UNSIGNED)
	{
	  asmf( "%#-3lx, %s, %s",
		  op_code,
		  _regname( src1 & 0x3f ),
		  pdest );
	}
      else
	{
	  asmf( "%-3ld, %s, %s",
		  op_code,
		  _regname( src1 & 0x3f ),
		  pdest );
	}
      
      break;
      
    case B_101:	/* src1 = register, src2 = indirect */

      asmf( "*+AR%.1ld(%.2lu), %s, %s",
	      src2 & 0x7, (src2 >> 3),
	      _regname( src1 & 0x3f ),
	      pdest );
      
      break;
      
    case B_110:	/* src1 = indirect, src2 = immediate */

      if (type == TYPE_FLOATING)
	{
	  syserr( syserr_no_triadic_FP_imm );
	}
      else if (type == TYPE_SIGNED)
	{
	  asmf( "%-3ld, *+AR%.1ld(%.2lu), %s",
		  mask_and_sign_extend_word( op_code, 0xff ),
		  src1 & 0x7, (src1 >> 3),
		  pdest );      
	}
      else
	{
	  asmf( "%#-3lx, *+AR%.1ld(%.2lu), %s",
		  op_code & 0xff,
		  src1 & 0x7, (src1 >> 3),
		  pdest );
	}
     
      break;
      
    case B_111:	/* src1 = indirect, src2 = indirect */
      
      asmf( "*+AR%.1ld(%.2lu), *+AR%.1ld(%.2lu), %s",
	      src2 & 0x7, (src2 >> 3),
	      src1 & 0x7, (src1 >> 3),
	      pdest );
      break;
      
    default:
      syserr( syserr_unknown_triadic_address, op_code );
      break;
    }

  return;
  
} /* decode_triadic_address */

/*}}}*/
/*{{{  decode_monadic_op() */

static void
decode_monadic_op(
		  int32		op_code,
		  char *	instruction )
{
  asmf( "%s\t%s", instruction, _regname( (op_code >> 16) & 0x1f ) );

  return;
  
} /* decode_mondadic_op */

/*}}}*/
/*{{{  decode_sequential_op() */

  
static void
decode_sequential_op( int32 op_code ) /* instruction */
{
  /*
   * bit  29      encodes diadic or triadic style op codes
   * bits 30 - 31 are 00
   */
  
  if (op_code & 0x20000000L)
    {
      /*
       * triadic op code
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
#ifdef TRIADIC_BINARY_OPS_ARE_UNSIGNED
	case OP_AND3:	decode_triadic_address( op_code, "AND3"    , TYPE_UNSIGNED ); return;
	case OP_ANDN3:	decode_triadic_address( op_code, "ANDN3"   , TYPE_UNSIGNED ); return;
	case OP_OR3:	decode_triadic_address( op_code, "OR3"     , TYPE_UNSIGNED ); return;
	case OP_TSTB3:	decode_triadic_address( op_code, "TSTB3"   , TYPE_UNSIGNED ); return;
	case OP_XOR3:	decode_triadic_address( op_code, "XOR3"    , TYPE_UNSIGNED ); return;
	case OP_MPYUHI3:decode_triadic_address( op_code, "MPYUHI3" , TYPE_UNSIGNED ); return;
#else
	case OP_AND3:	decode_triadic_address( op_code, "AND3"    , TYPE_SIGNED   ); return;
	case OP_ANDN3:	decode_triadic_address( op_code, "ANDN3"   , TYPE_SIGNED   ); return;
	case OP_OR3:	decode_triadic_address( op_code, "OR3"     , TYPE_SIGNED   ); return;
	case OP_TSTB3:	decode_triadic_address( op_code, "TSTB3"   , TYPE_SIGNED   ); return;
	case OP_XOR3:	decode_triadic_address( op_code, "XOR3"    , TYPE_SIGNED   ); return;
	case OP_MPYUHI3:decode_triadic_address( op_code, "MPYUHI3" , TYPE_SIGNED   ); return;
#endif
	default:
	  syserr( syserr_unknown_triadic_op, (op_code >> 23) & 0x1f );
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
	case OP_IDLE:	asmf( "IDLE" ); return;
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
	case OP_NOP:	asmf( "NOP" ); return;	/* XXX - not quite true can use indirect addressing to adjust address registers */
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
	  syserr( syserr_unknown_diadic_op, op_code >> 23 );
	}
    }
  
  return;
  
} /* decode_sequential_op */

/*}}}*/
/*{{{  decode_instruction() */

void
decode_instruction(
		   int32	op_code,	/* instruction 				*/
		   bool		flag_errs )	/* TRUE if errors should be reported	*/
{
  /*
   * bits 30 and 31 encode instruction type
   */
  
  switch ((op_code >> 30) & 0x3)
    {
    default:
    case 0: decode_sequential_op( op_code );            return;
    case 1: decode_flow_control(  op_code, flag_errs ); return;
    case 2: decode_non_word_load_or_triadic( op_code ); return;
    case 3: decode_parallel_op(   op_code );            return;
    }

} /* decode_instruction */

/*}}}*/
/*{{{  decode_DC() */

static void
decode_DC( int32 op_code )
{
  asmf( "word\t0x%.8lx", (long)op_code );

  return;

} /* decode_DC */

/*}}}*/
/*{{{  decode_DCAx() */

static void
decode_DCAx( int32 offset )
{
  asmf( "word \t" );
  
  decode_addr( offset );

  return;
  
} /* decode_DCAx */

/*}}}*/

#ifdef NOT_USED
/*{{{  decode_DCA() */

static void
decode_DCA(
	   Symstr *	s,
	   int32 	op_code )
{
  asmf( "word\t_ " );
  
  /* decode_addr( op_code, 0 ); */
  
  pr_asmname( s );
  
  if (op_code != 0)
    asmf( " %+ld", (long)op_code );

  return;
  
} /* decode_DCA */

/*}}}*/
/*{{{  maybe_export() */

static void
maybe_export( Symstr * sym )
{
  char *	p = symname_( sym );
  char  	c;
  ExtRef *	x;


  /* Unless external there is nothing to do here. */

  if ((x = symext_( sym )) != 0 && !is_global( x ))
    return;

  /*@@@ AM does not see how the following can ever now happen as _dataseg etc. */
  /*@@@ are very local statics.  Is this if error recovery inserted gensyms?   */
  
  while ((c = *p++) != 0)
    {
      /* look for odd characters in _dataseg etc */
      
      if (!(isalnum( c ) || (c == '_')))
	return;
    }

#if 0
  asmf( "\t.export\t_" );
     
  pr_asmname( sym );
     
  fputs( "\n", asmstream );
#endif

  return;
  
} /* maybe_export */

/*}}}*/
#endif /* NOT_USED */
/*{{{  display_assembly_code() */

/* exported functions ...*/

void
display_assembly_code( Symstr *	name )
{
  LabList *	asm_lablist2 = NULL;


  asm_blank( 2 );

  asm_lablist2 = asm_lablist;

  fncount++;

  if (name != NULL)   /* may be NULL for string literals from static inits   */
    {
      asmf( "%c        align 128\n", ASM_COMMENT_CHAR );
  
      if (annotations)
	asmf( "%.6lx  %20s", (long)codebase, "" );

      fputc( '.', asmstream );
      
      pr_asmname( name );
      
      asmf( ":\n" );
    }

  for (current_pos = 0; current_pos < codep; current_pos += sizeof_int)    /* q is now a BYTE offset */
    {
      const int32	op_code    = code_inst_( current_pos );
      const int32	f          = code_flag_( current_pos );
      VoidStar 		aux 	   = code_aux_(  current_pos );
#ifdef TARGET_IS_C40
      int32		pending_fpnum;
#endif
      
      
	{
	  int32 	labq;
	  LabelNumber *	t;

	  
	  while (asm_lablist2 &&
		 (t    = asm_lablist2->labcar,
		  labq = t->u.defn & 0x00ffffff) <= current_pos)
	    {
	      if (annotations)
		asmf( "%28s", "" );

	      asmf( "L%ldF%ld:\n", (long)(lab_name_( t ) & 0x7fffffff), (long)fncount );

	      /* asmf( "\t%c offset = %+ld\n", ASM_COMMENT_CHAR, labq ); */

	      if (labq != current_pos)
		syserr( syserr_asmlab, (long)labq );
      
	      asm_lablist2 = asm_lablist2->labcdr;
	    }
	}

      patch_pend = FALSE;
      
      if (codexrefs)
	{
	  CodeXref *	x;
	  int32		pos = 0;
	  

	  for (x = codexrefs;
	       x && (pos = (x->codexroff & 0x00ffffffU), pos > codebase + current_pos);
	       x = x->codexrcdr )
	    ;

	  if (pos == codebase + current_pos)
	    {
	      int32	xrtype = x->codexroff & 0xff000000U;


	      patch_pend = TRUE;
	      
	      switch (xrtype)
		{
		case X_PCreloc:
		  asmf( "patchinstr( PATCHC40MASK24ADD, shift( -2, %s ( .%s ) ),\n",
		       new_stubs ? "CODESTUB" : "LABELREF",
		       symname_( (Symstr *)(int)aux ) );
		  break;
		  
		case X_PCreloc2:
		  asmf( "patchinstr( PATCHC40MASK16ADD, shift( -2, %s ( .%s ) ),\n",
		       new_stubs ? "CODESTUB" : "LABELREF",
		       symname_( (Symstr *)(int)aux ) );
		  break;
		  
		case X_DataModule1:
		  asmf( "patchinstr( PATCHC40DATAMODULE1, shift( -2, DATAMODULE ( _%s ) ),\n",
			  symname_( (Symstr *)(int)aux ) );
		  break;
		  
		case X_DataModule2:
		  asmf( "patchinstr( PATCHC40DATAMODULE2, shift ( -2, CODESYMB ( _%s ) ),\n",
			  symname_( (Symstr *)(int)aux ) );
		  break;
		  
		case X_DataModule3:
		  asmf( "patchinstr( PATCHC40DATAMODULE3, MODNUM,\n" );
		  break;
		  
		case X_DataModule4:
		  asmf( "patchinstr( PATCHC40DATAMODULE4, MODNUM,\n" );
		  break;
		  
		case X_DataModule5:
		  asmf( "patchinstr( PATCHC40DATAMODULE5, MODNUM,\n" );
		  break;
		  
		case X_DataSymbHi:
		  asmf( "patchinstr( PATCHC40MASK16ADD, " );
		  
		  if (split_module_table && is_function( (Symstr *)(int)aux ))
		    {
		      asmf( "shift ( -18, CODESYMB ( _%s ) ),\n",
			      symname_( (Symstr *)(int)aux ) );
		    }
		  else
		    {
		      asmf( "shift ( -16, DATASYMB ( _%s ) ),\n",
			      symname_( (Symstr *)(int)aux ) );
		    }
		  break;
		  
		case X_DataSymbLo:
		  asmf( "patchinstr( PATCHC40MASK16ADD, " );
		  
		  if (split_module_table && is_function( (Symstr *)(int)aux ))
		    {		      
		      asmf( "shift ( -16, shift ( 14, CODESYMB ( _%s ) ) ),\n",
			      symname_( (Symstr *)(int)aux ) );
		    }
		  else
		    {
		      asmf( "shift ( -16, shift ( 16, DATASYMB ( _%s ) ) ),\n",
			      symname_( (Symstr *)(int)aux ) );
		    }
		  break;
		  
		case X_DataAddr1:
		  asmf( "patchinstr( PATCHC40MASK16ADD, shift ( -18, p_add ( -12, " );

		  if (is_function( (Symstr *)(int)aux ))
		    {
		      if (new_stubs)
			{
			  asmf( "CODESTUB ( .%s ) ) ),\n", symname_( (Symstr *)(int)aux ) );
			}
		      else
			{
			  asmf( "LABELREF ( .%s ) ) ),\n", symname_( (Symstr *)(int)aux ) );
			}		      
		    }
		  else
		    {
		      asmf( "LABELREF ( _%s ) ) ),\n", symname_( (Symstr *)(int)aux ) );
		    }
		  
		  break;
		  
		case X_DataAddr:
		  asmf( "patchinstr( PATCHC40MASK16ADD, shift ( -16, shift ( 14, " );

		  if (is_function( (Symstr *)(int)aux ))
		    {
		      if (new_stubs)
			{
			  asmf( "CODESTUB ( .%s ) ) ),\n", symname_( (Symstr *)(int)aux ) );
			}
		      else
			{
			  asmf( "LABELREF ( .%s ) ) ),\n", symname_( (Symstr *)(int)aux ) );
			}		      
		    }
		  else
		    {
		      asmf( "LABELREF ( _%s ) ) ),\n", symname_( (Symstr *)(int)aux ) );
		    }
		  
		  break;
		    
		case X_FuncAddr:
		  asmf( "patchinstr( PATCHC40MASK24ADD, shift( -2, " );

		  asmf( "ADDRSTUB ( .addr.%s ) ),\n", symname_( (Symstr *)(int)aux ) );
		  
		  break;
		  
		case X_DataModule:
		  asmf( "patchinstr( %s, shift ( -2, DATAMODULE ( _%s ) ),\n",
			  few_modules ? "PATCHC40MASK8ADD" : "PATCHC40MASK16ADD" ,
			  symname_( (Symstr *)(int)aux ) );

		  break;
		    
		case X_Modnum:
		  asmf( "patchinstr( %s, ",
		       few_modules ? "PATCHC40MASK8ADD" : "PATCHC40MASK16ADD" );
		  
		  if (split_module_table)
		    asmf( "shift ( 1, MODNUM ),\n" );
		  else
		    asmf( "MODNUM,\n" );
		    
		  break;

		case X_DataSymb:
		  asmf( "patchinstr( PATCHC40MASK16ADD, shift ( -2, %s ( _%s ) ),\n",
			  split_module_table ? "CODESYMB" : "DATASYMB",
			  symname_( (Symstr *)(int)aux ) );
		  break;

		case X_Debug_Modnum:
		  asmf( "patchinstr( MODNUM\n" );
		  break;
		  
		case X_Debug_Offset:
		  asmf( "patchinstr( %s ( _%s ) \n",
		       split_module_table ? "CODESYMB" : "DATASYMB",
		       symname_( (Symstr *)(int)aux ) );
		  break;
		  
		case X_Debug_Ref:
		  asmf( "patchinstr( LABELREF ( _%s ) \n",
			  symname_( (Symstr *)(int)aux ) );
		  break;
		  
		default:
		  asmf( "%c unknown cross reference type %lx\n", ASM_COMMENT_CHAR, xrtype );

		  /* drop through */
		  
		case X_absreloc:
		  patch_pend = FALSE;
		  break;	      
		}
	    }
	}

      if (annotations)
        {
	  int32	i;

	  
	  asmf( "%.6lx  ", (long)(current_pos + codebase) );
	  
	  switch (f)
            {
	    case LIT_OPCODE:
	      for (i = 0; i < (sizeof_int * 2); i += sizeof_int)
		if (i < sizeof_int)
		  asmf( "%.8lx ", (long)code_inst_( current_pos + i ) );
		else
		  asmf( "     " );
	      break;

	    case LIT_STRING:
	      asmf( "      %.8lx      ", (long)totargetsex( op_code, LIT_BBBB ) );
	      break;

	    default:
	      asmf( "      %.8lx      ", (long)op_code );
	      break;
            }
        }

      putc( ' ', asmstream );

      switch (f)
	{
	case LIT_OPCODE:
	  /* all instructions are four bytes on 'C40  */
	  /* so a lot of grotty code has been removed */
	  /* concerned with making branch table       */
	  /* entries always four bytes                */

	  sym_reladdr = NULL;
	  
	  decode_instruction( op_code, TRUE );
	  
	  break;

	case LIT_RELADDR:
	  if (patch_pend)
	    sym_reladdr = NULL;
	  else
	    sym_reladdr = (Symstr *)(int)aux;
	  
	  decode_instruction( op_code, TRUE );

	  if (!patch_pend && sym_reladdr && !exporting_routines)
	    {
	      asmf( ".%s", symname_( sym_reladdr ) );
	    }
	  
	  break;
	  
	case LIT_STRING:
	  decode_DC( totargetsex( op_code, LIT_BBBB ) );

	  if (annotations)
	    {
	      asmf( "         %c ", ASM_COMMENT_CHAR );
	      
	      pr_chars( op_code );
	    }

	  break;

	case LIT_NUMBER:
	  decode_DC( op_code );
	  break;

	case LIT_ADCON:
	  decode_DCAx( op_code );	/* ??? */
	  
	  break;

	case LIT_FPNUM:
#ifdef TARGET_IS_C40
	  decode_DC( IEEE_to_single_float( op_code ) );
#else  
	  decode_DC( op_code );
#endif
	  if (annotations)
	    asmf( " %c E'%s'", ASM_COMMENT_CHAR, aux );
	  
	  break;

	case LIT_FPNUM1:
#ifdef TARGET_IS_C40
	  pending_fpnum = op_code;
#else  
	  decode_DC( op_code );

	  if (annotations)
	    asmf( " %c D'%s'", ASM_COMMENT_CHAR, aux );
#endif
	  break;

	case LIT_FPNUM2:    /* all printed by the FPNUM1 */
#ifdef TARGET_IS_C40
	  decode_DC( IEEE_to_extended_float( pending_fpnum, op_code, NULL  ) );
	  
	  asmf( "\n " );
	  
	  IEEE_to_extended_float( pending_fpnum, op_code, &pending_fpnum );
	  
	  decode_DC( pending_fpnum );

	  if (annotations)
	    asmf( " %c D'%s'", ASM_COMMENT_CHAR, aux );
#else  
	  decode_DC( op_code );
#endif
	  break;

	default:
	  syserr( syserr_display_asm, (long)f );
	  
	  asmf( "?" );
        }

      if (patch_pend)
	asmf( " )\n" );
      else
	asmf( "\n" );
    }

  if (asm_lablist2)
    {
      syserr( syserr_asmlab, 0L );
    }  

/*  putc( ' ', asmstream ); */

  asm_lablist = NULL;    /* stop confusion when called from vargen.c  */

  return;

} /* display_assembly_code */

/*}}}*/
/*{{{  asm_header() */

void
asm_header( void )
{
#ifdef TARGET_HAS_DEBUGGER
  if (usrdbg( DBG_ANY ))
    db_init( sourcefile );
#endif
  
  if (asmstream == NULL)
    {
      fprintf( stderr, "WARNING: no assembler output stream!\n" );

      asmstream = stderr;
    }

  asm_error = 0;
  fncount   = 0;

  asmf( "%c generated by %s\n", ASM_COMMENT_CHAR, CC_BANNER );

#if 0	/* assembler automatically inserts this code unless -d option is used */  
  if (!suppress_module)
    {
      asmf( "\talign\n" );
      asmf( "\tmodule  -1\n" );
      asmf( ".ModStart:\n" );      
      asmf( "\tword    %#lx\n", T_Module );
      asmf( "\tword    (.ModEnd - .ModStart) * 4\n" );
      asmf( "\tblkb    32,\"%.31s\"\n", sourcefile );
      asmf( "\tword    modnum\n" );
      asmf( "\tword    1\n" );
      asmf( "\tword    datasymb ( .MaxData )\n" );
      asmf( "\tinit\n" );
      asmf( "\tword    codesymb ( .MaxCodeP )\n" );      
    }
#endif
  
  return;
  
} /* asm_header */

/*}}}*/
/*{{{  asm_outextern() */

/* (not exported) */

static void
asm_outextern( void )
{
  ExtRef *	x;

  if (new_stubs)
    {
      for (x = obj_symlist; x != NULL; x = x->extcdr)
	{
	  if (!is_local( x ))
	    {
	      asmf( "export    ." );

	      pr_asmname( x->extsym );
	  
	      asmf( "\nref       _" );

	      pr_asmname( x->extsym );
	  
	      asmf( "\n" );
	    }
	}
    }
  else
    {
      bool		done_first = FALSE;
  

      for (x = obj_symlist; x != NULL; x = x->extcdr)
	{
	  if (!is_local( x ) && is_global( x ))
	    {
	      if (done_first)
		{
		  asmf( "\n" );
		}
	      else
		{
		  done_first = TRUE;
		}

	      asmf( "codetable _" );
	  
	      pr_asmname( x->extsym );
	  
	      asmf( "\nexport    _" );
	  
	      pr_asmname( x->extsym );
	  
	      asmf( "\n" );
	    }
	}
    }  
  
  asm_blank( 1 );

  return;
  
} /* asm_outextern */

/*}}}*/
/*{{{  asm_output_symbol() */

static void
asm_output_symbol(
		  Symstr *	symbol,
		  unsigned32	size )
{
  if (symbol != NULL)
    {
      asmf( "data   _%s, %ld\n", symname_( symbol ), size );
      
      if (is_global( symext_( symbol ) ))
	{
	  asmf( "export %s\n", symname_( symbol ) );
	}
    }

  return;
  
} /* asm_output_symbol */
  

/*}}}*/
/*{{{  ExtRefList Type*/

/* XXX - NC -> *sigh* do these LISP hackers get everywhere ? */

typedef struct ExtRefList
  {
    struct ExtRefList *	cdr;
    ExtRef *		car;
  }
ExtRefList;

/*}}}*/
/*{{{  asm_trailer() */

void
asm_trailer( void )
{
  DataInit *		p;
  unsigned32		dlen;
  ExtRef *		dsymb;
  ExtRef *		curdsymb;
  unsigned32		old_dlen;
  

#ifdef TARGET_HAS_DEBUGGER
  if (usrdbg( DBG_ANY ))
    db_tidy();
#endif
  
  asm_blank( 1 );

#if 0
  asm_padcol8( 0 );
#endif

  dsymb    = datasymbols;
  curdsymb = NULL;
  dlen     = 0;
  old_dlen = 0;
  
  for (p = datainitp; p != NULL; p = p->datacdr)
    {
      int32	sort = p->sort;
      int32	len  = p->len;
      int32	val  = p->val;
      
      
      if (dsymb != NULL)
	{
	  int32	dataoff = dsymb->extoffset;
		  

	  /* have we reached the location of the next symbol to be exported  ? */
	  
	  if (dlen == dataoff)
	    {
	      /* do we have a previous symbol to export ? */
	      
	      if (curdsymb != NULL)
		{
		  asm_output_symbol( curdsymb->extsym, dlen - old_dlen );
		}

	      /* export all but the last symbol with this offset */
	      
	      for (; dsymb->extcdr && dsymb->extcdr->extoffset == dataoff; dsymb = dsymb->extcdr )
		{
		  asm_output_symbol( dsymb->extsym, 0 );
		}

	      /* remember the last symbol at this offset, and the size of the data segment at this point */
	      
	      old_dlen = dlen;
	      curdsymb = dsymb;

	      /* point to the next symbol in the list */
	      
	      dsymb = dsymb->extcdr;
	    }
	  else if (dataoff < dlen)
	    {
	      /* oh dear - we have mislaid a symbol */
	      
	      syserr( syserr_mislaid_a_symbol );
	    }
	}
#if 0
      if (sort != LIT_LABEL)
	asm_padcol8( 0 );
#endif
      
      switch (sort)
	{
	case LIT_LABEL:
	  break;
	  
	default:
	  syserr( syserr_asm_trailer, (long)sort );
	  
	case LIT_BBBB:
	case LIT_HH:
	case LIT_BBH:
	case LIT_HBB:
	case LIT_NUMBER:
	  if (len != 4)
	    syserr( syserr_datalen, (long)len );
	  
	  dlen += len * (p->rpt);
	  
	  break;
	  
	case LIT_FPNUM:
	    {
	      FloatCon *	fc = (FloatCon *)val;
	      
	      
#ifndef TARGET_IS_C40
	      decode_DC( fc->floatbin.irep[ 0 ] );
#endif
	      if (annotations) 
		asmf( " %c %s", ASM_COMMENT_CHAR, fc->floatstr );
	      
#ifndef TARGET_IS_C40
	      if (len == 8)
		{
		  asmf( "\n" );
		  
		  asm_padcol8( 0 );
		  
		  decode_DC( fc->floatbin.db.lsd );
		}
#endif	      
	      dlen += len;
	      
	      break;
	    }
	  
	case LIT_ADCON:              /* (possibly external) name + offset */
	  if (p->rpt != 1)
	    syserr( syserr_asm_trailer2 );
#if 0
	  decode_DCA( (Symstr *)len, val );
#endif
	  dlen += sizeof_ptr;
	  
	  break;
	}
#if 0
      asmf( "\n" );
#endif
    }

  if (curdsymb)
    {
      asm_output_symbol( curdsymb->extsym, dlen - old_dlen );
    }
  
#ifdef TARGET_HAS_BSS
  if (bss_size != 0)
    {
      int32 		n      = 0;
      ExtRef *		x      = obj_symlist;
      ExtRefList *	zisyms = NULL;

      
      asmf( "%c%c%c Start of BSS\n", ASM_COMMENT_CHAR, ASM_COMMENT_CHAR, ASM_COMMENT_CHAR );
      
      for (; x != NULL; x = x->extcdr)
	{
	  if (is_bss( x ))
	    {
	      ExtRefList **	prev = &zisyms;
	      ExtRefList *	p;

	    
	      for (; (p = *prev) != 0; prev = &cdr_( p ))
		if (x->extoffset < car_( p )->extoffset)
		  break;
	    
	      *prev = syn_cons2( *prev, x );
	    }
	}

      for (; zisyms != NULL; zisyms = cdr_( zisyms ))
	{
	  x = car_( zisyms );
	  
	  if (x->extoffset != n)
	    asmf( ",%ld,%cbss%c\n", x->extoffset-n, '"', '"' );
	  
	  n = x->extoffset;
	  
	  maybe_export( x->extsym );
	  
	  asmf( "\t.common\t_" );
	  
	  pr_asmname( x->extsym );
	}
      
      if (n != bss_size)
	asmf( ",%ld, %cbss%c\n", bss_size - n, '"', '"' );
    }
#endif /* TARGET_HAS_BSS */

  asm_blank( 1 );
  
  asm_outextern();

  asm_blank( 1 );

#if 0	     /* generated automatically by assembler */
  if (!suppress_module)
    {
      asmf( "\tdata      .MaxData, 0\n" );
      asmf( "\tcodetable .MaxCodeP\n%c\n", ASM_COMMENT_CHAR );
      asmf( ".ModEnd:\n" );
    }
#endif
  
  asmf( "%c END\n", ASM_COMMENT_CHAR );

  if (asm_error)
    syserr( syserr_asm_confused );

  return;
  
} /* asm_trailer */

/*}}}*/

/*}}}*/
/* end of asm.c */
