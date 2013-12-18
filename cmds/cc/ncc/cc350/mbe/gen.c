/*{{{  comments */

/* C compiler file gen.c :  Copyright (c) Perihelion Software Ltd, 1991, 1992. 1993. 1994. */
/* version 4 */
/* $Header: /users/nickc/RTNucleus/cmds/cc/ncc/cc350/mbe/RCS/gen.c,v 1.10 1994/01/04 14:04:02 nickc Exp $ */

/*
 * XXX - NC - 30/07/91
 *
 * This is my first attempt at converting a code generator to work
 * for the Texas Instruments TMS320C4x chips.  I am basing this
 * port on the SPARC code generator, with bits stolen from the MIPS
 * and 68K code generators.
 *
 *	Caveat Emptor
 */
/*
 * XXX - NC
 *
 * For Your Information
 *
 * If you are looking at this file, and you are not familiar with the 'C40
 * instruction set then a few points might prove helpful:
 *
 *	+ all op codes are 32 bits wide
 *
 *	+ all op codes except CALL (jump to subroutine) and B (branch) take 1 instruction cycle
 *
 *	+ the chip only supports word addressing and has a 16 gigabyte range (where 1 byte = 8 bits)
 *
 *	+ the machine shares floating point and integer registers, and has seperate address registers
 *
 *	+ the instruction set has delayed branches (delayed for 3 instructions) and link-and-jump instructions
 *
 *	+ we are using byte addressing where addresses are a byte offset from register IR0
 *        (in this case a byte is an 8 bit quantity)
 *
 *	+ the instruction set is a fairly standard RISC set except that:
 *		- it has diadic and triadic versions of most operations
 *		- triadic ops can sometimes be done in parallel with a load or store
 *		- floating point is not IEEE standard, but instructions exist to convert between formats
 *		- there is a "repeat single instruction" and "repeat block of instructions" mode
 */


/*
 * due to an incredible bug in the C40 silicon design, logical triadic
 * operations take a signed immediate value.  If TI ever fix this
 * stupidity then enable this define
 *
 *  #define TRIADIC_BINARY_OPS_ARE_UNSIGNED		1
 */

/* similarly define these if the hardware bugs are ever fixed */

/* #define STI_STI_NOW_WORKS_ON_HARDWARE 1 	*/
/* #define STIK_NOW_WORKS_ON_HARDWARE 1 	*/

/* AM July 88: add TAILCALLK/TAILCALLR -- ACN to check anomalies. */
/* bug: try the following and see uneven noop insertion:
   f(int *x,int *y) { x[5] = y[5];  x[4] = y[4];  x[3] = y[3];
   x[2] = y[2];  x[1] = y[1];  x[0] = y[0]; }
   g(int *x) { h(x[1],x[2],x[3]); }
   */

/*}}}*/
/*{{{  #includes */

#ifdef __STDC__
#  include <string.h>
#else
#  include <strings.h>
#endif

#include "globals.h"
#include "builtin.h"
#include "mcdep.h"
#include "ops.h"
#include "mcdpriv.h"
#include "xrefs.h"
#include "jopcode.h"
#include "store.h"
#include "codebuf.h"
#include "regalloc.h"
#include "cg.h"        /* for procflags, greatest_stackdepth */
#include "errors.h"
#include "util.h"
#include "aeops.h"
#include "peep.h"
#include "bind.h"	/* for sym_insert() */

/*}}}*/
/*{{{  #defines */

#if defined __HELIOS
#define UNUSED( var )	var = var;
#else
#define UNUSED( var )
#endif

#ifndef streq
#define streq( a, b )	(strcmp( (a), (b) ) == 0)
#endif

#define CONTIGUOUS_ARGS			(PROC_ARGPUSH | PROC_ARGADDR)
#define NONLEAF 			(CONTIGUOUS_ARGS | PROC_BIGSTACK | BLKCALL)

/*
 * macros for manipulating the stack pointer
 */

#define adjust_stack( by ) 	(stack_move += (by))

/*}}}*/
/*{{{  local variables */

/* vvvvvvvvvvvvvvvvvvvvvv    PEEPHOLER    vvvvvvvvvvvvvvvvvvvv */

int32			death              = 0;		/* mask of dead registers at end of current J_opcode */

/* ^^^^^^^^^^^^^^^^^^^^^^    PEEPHOLER    ^^^^^^^^^^^^^^^^^^^^ */

static void		routine_exit( bool tailcall );

static bool		saved_frame        = FALSE;	/* true if frame pointer saved on stack		*/
static int32		saved_ivars        = 0;		/* mask of variable registers saved on stack	*/
static int32		saved_fvars        = 0;		/* mask of variable registers saved on stack	*/
static int32		saved_args         = 0;		/* mask of argument registers saved on stack 	*/
static int32 		casebranch_pending = 0;
static int32		casebranch_r1r     = GAP;
static RealRegister	saved_link_reg	   = GAP;	/* register containing return address, if not R_LR */
static Symstr *		saved_regs         = NULL;	/* register saving structure used by debugger */

static LabelNumber *	returnlab;

static int32		codep_of_call = 0;	/* used during data initialisation */

int32			stack_move    = 0;	/* number of bytes potentially subtracted from stack */
int32			stack_offset  = 0;	/* number of bytes from stack pointer to first local arg */

/*}}}*/
/*{{{  functions */

#ifndef __HELIOS
/*{{{  IOdebug() */

void
IOdebug( const char * format, ... )
{
  return;
}  

/*}}}*/
#endif

/*{{{  Instruction Output */

/*{{{  outinstr() */

/*
 * output an instruction
 */

static void
outinstr(
	 int32	op_code,	/* the 32 bit op code */
	 int32	reads,		/* mask of registers read by the op code */
	 int32	writes )	/* mask of registers altered by the op code */
{
  append_peep( OUT_INSTR, op_code, reads, writes, NULL, NULL, LABREF_NONE );
    
  return;
    
} /* outinstr */

/*}}}*/
/*{{{  nop() */

/*
 * send a NO-OP instruction
 */

static void
nop( bool is_delay )
{
  append_peep( OUT_NULL, OP_NOP << 23,
	      is_delay ? examines1( RR_PC ) : examines0(),
	      alters0(), NULL, NULL, LABREF_NONE );
    
  return;
    
} /* nop */

/*}}}*/
/*{{{  outdelayed() */

/*
 * output a delayed instruction
 */

static void
outdelayed(
	   int32	op_code,	/* the 32 bit op code */
	   int32	reads,		/* mask of registers read by the op code */
	   int32	writes )	/* mask of registers altered by the op code */
{
  append_peep( OUT_DELAYED, op_code, reads, writes, NULL, NULL, LABREF_NONE );
    
  nop( TRUE );
  nop( TRUE );
  nop( TRUE );

  /* do not allow peepholing to occur across a delayed instruction */
  
  flush_peepholer( DBG( "delayed instruction" ) );
  
  return;
    
} /* outdelayed */

/*}}}*/
/*{{{  outdellabref() */

/*
 * output a delayed instruction that references a label
 */

static void
outdellabref(
	     int32		op_code,	/* the 32 bit op code */
	     LabelNumber *	label,		/* the label referenced */
	     int32		reftype,	/* the size of the offset field */
	     int32		reads,		/* mask of registers read by the op code */
	     int32		writes )	/* mask of registers altered by the op code */
{
  append_peep( OUT_DELLABREF, op_code, reads, writes, NULL, label, reftype );
    
  nop( TRUE );
  nop( TRUE );
  nop( TRUE );

  /* do not allow peepholing across delayed instructions */
  
  flush_peepholer( DBG( "delayed instruction" ) );
  
  return;
    
} /* outdellabref */

/*}}}*/
#ifdef NOT_USED
/*{{{  outinstrref() */

/*
 * output an instruction which references symbol 'name'
 */

static void
outinstrref(
	    int32	op_code,	/* 32 bit op code */
	    Symstr *	name,		/* symbol used by op code */
	    int32	reads,		/* mask of registers examined by op code */
	    int32	writes )	/* mask of registers altered by op_code */
{
  append_peep( OUT_SYMREF, op_code, reads, writes, name, NULL, LABREF_NONE );
    
  return;
    
} /* outinstrref */

/*}}}*/
#endif /* NOT_USED */
/*{{{  outdelsymref() */

/*
 * output a delayed instruction which references symbol 'name'
 */

static void
outdelsymref(
	    int32	op_code,	/* 32 bit op code */
	    Symstr *	name,		/* symbol used by op code */
	    int32	reads,		/* mask of registers examined by op code */
	    int32	writes )	/* mask of registers altered by op_code */
{
  append_peep( OUT_DELSYMREF, op_code, reads, writes, name, NULL, LABREF_NONE );

  nop( TRUE );
  nop( TRUE );
  nop( TRUE );
  
  /* do not allow peepholing across delayed instructions */
  
  flush_peepholer( DBG( "delayed instruction" ) );
  
  return;
    
} /* outdelsymref */

/*}}}*/
#ifdef NOT_USED
/*{{{  outxref() */

/*
 * output an instruction which cross references symbol 'name'
 */

static void
outxref(
	int32		op_code,		/* 32 bit operand 				*/
	int32		addressing_mode,	/* form of addressing 				*/
	RealRegister	destination,		/* register written to 				*/
	int32		source,			/* register read from 				*/
	Symstr *	name,			/* symbol used by op code 			*/
	int32		ref_type,		/* type of cross reference 			*/
	int32		reads,			/* mask of registers examined by op code	*/
	int32		writes )		/* mask of registers altered by op_code 	*/
{
  append_peep( OUT_XREF, build_op( op_code, addressing_mode, destination, source ),
	      reads, writes, name, NULL, ref_type );
    
  return;
    
} /* outxref */

/*}}}*/
#endif /* NOT_USED */

/*}}}*/
/*{{{  Register Naming Conventions */

/*{{{  hardware_register() */

#ifdef NOT_USED

/*
 * op code construction functions
 */

static char *	regstr =
  "R0\0\0R1\0\0R2\0\0R3\0\0R4\0\0R5\0\0R6\0\0R7\0\0"
  "AR0\0AR1\0AR2\0AR3\0AR4\0AR5\0AR6\0AR7\0"
  "DP\0\0IR0\0IR1\0BK\0\0SP\0\0ST\0\0DIE\0IIE\0"
  "IIF\0RS\0\0RE\0\0RC\0\0R8\0\0R9\0\0R10\0R11\0";

/* NB/ regname_ has already been taken */

#define reg_( r ) ((((r) & 0x1f) * 4) + regstr)

#endif /* NOT_USED */

/*
 * maps between a RealRegister and a hardware register
 * NB/ this must agree with the register naming scheme
 * documented in c40/target.h
 *
 * This also affects is_C40_float_register() below
 */

int32
hardware_register( RealRegister r )
{
  static int32 tab[ MAXREGNUMBER + 1 ] =
    {
      0x00,	/*  0 => R0  */
      0x01,	/*  1 => R1  */
      0x02,	/*  2 => R2  */
      0x03,	/*  3 => R3  */
      0x04,	/*  4 => R4  */
      0x05,	/*  5 => R5  */
      0x06,	/*  6 => R6  */
      0x07,	/*  7 => R7  */
      0x10,	/*  8 => DP  */
      0x13,	/*  9 => BK  */
      0x1c,	/* 10 => R8  */
      0x1d,	/* 11 => R9  */
      0x1e,	/* 12 => R10 */
      0x08, 	/* 13 => AR0 */
      0x09,	/* 14 => AR1 */
      0x0a,	/* 15 => AR2 */
      0x0b,	/* 16 => AR3 */
      0x1f,	/* 17 => R11 */
      0x0c,	/* 18 => AR4 */
      0x0d,	/* 19 => AR5 */
      0x0e,	/* 20 => AR6 */
      0x0f,  	/* 21 => AR7 */
      0x11,	/* 22 => IR0 */
      0x12,	/* 23 => IR1 */
      0x14,	/* 24 =>  SP */
      0x15,	/* 25 =>  ST */
      0x19,	/* 26 =>  RS */
      0x1a,	/* 27 =>  RE */
      0x1b	/* 28 =>  RC */
    };

  if (r > MAXREGNUMBER)
    syserr( gen_illegal_register, r );
  
  return tab[ r ];
  
} /* hardware_register */

/*}}}*/
/*{{{  is_C40_float_register() */

bool
is_C40_float_register( RealRegister r )
{
  int32	reg = hardware_register( r );


  if (reg <= 0x07 || (reg >= 0x1c && reg <= 0x1f))
    return TRUE;

  return FALSE;

} /* is_C40_float_register */
  

/*}}}*/
/*{{{  real_register() */

/*
 * maps between a harwdare register and a RealRegister 
 */

RealRegister
real_register( int32 r )
{
  static int32 tab[ 0x1f + 1 ] =
    {
      RR_R0,	/*  0 */
      RR_R1,	/*  1 */
      RR_R2,	/*  2 */
      RR_R3,	/*  3 */
      RR_R4,	/*  4 */
      RR_R5,	/*  5 */
      RR_R6,	/*  6 */
      RR_R7,	/*  7 */
      RR_AR0,	/*  8 */
      RR_AR1,	/*  9 */
      RR_AR2,	/*  a */
      RR_AR3,	/*  b */
      RR_AR4,	/*  c */
      RR_AR5, 	/*  d */
      RR_AR6,	/*  e */
      RR_AR7,	/*  f */
      RR_DP,	/* 10 */
      RR_IR0,	/* 11 */
      RR_IR1,	/* 12 */
      RR_BK,	/* 13 */
      RR_SP,	/* 14 */
      RR_ST,  	/* 15 */
      -1,	/* 16 */ /* DIE */
      -1,	/* 17 */ /* IIE */
      -1,	/* 18 */ /* IIF */
      RR_RS,	/* 19 */
      RR_RE,	/* 1a */
      RR_RC,	/* 1b */
      RR_R8,	/* 1c */
      RR_R9,	/* 1d */
      RR_R10,	/* 1e */
      RR_R11	/* 1f */
    };

  if (r > 0x1f)
    syserr( gen_illegal_register, r );
  
  return tab[ r ];
  
} /* real_register */

/*}}}*/

/*}}}*/
/*{{{  Instruction Building */

/*{{{  build_op() */

/*
 * builds a diadic op-code
 */

int32
build_op(
	 int32		op_code,		/* 32 bit operand */
	 int32		addressing_mode,	/* form of addressing */
	 RealRegister	destination,		/* registers written to */
	 int32		source			/* NB/ doubles up as a RealRegister */
	 )
{
  int32	op;
  int32	dst;


  if (op_code == OP_STIK && addressing_mode == ADDR_MODE_IMMEDIATE)
    {
      dst = destination;	/* ie do NOT translate, the 'destintaion' is NOT a register */
    }
  else
    {
      dst = hardware_register( destination );
    }

  /* ensure bit fields are the right length */
  
  op_code &= 0x3fU;
  source  &= 0xffffU;
  dst     &= 0x1fU;

  /* construct op code */

  op = (op_code << 23) | (dst << 16);

  /* set addressing bits, and munge source if necessary */
  
  switch (addressing_mode)
    {
    case ADDR_MODE_REGISTER:
      source = hardware_register( (RealRegister)source );
      break;

    case ADDR_MODE_DIRECT:
      op |= 0x1U << 21;
      break;
      
    case ADDR_MODE_INDIRECT:
      op |= 0x2U << 21;
      break;

    case ADDR_MODE_IMMEDIATE:
      op |= 0x3U << 21;
      break;

    default:
      syserr( gen_bad_addr_mode, addressing_mode );
      break;
    }

  return op | source;

} /* build_op */

/*}}}*/
/*{{{  out_diadic_op() */

/*
 * builds and sends a diadic op-code
 */

static void
out_diadic_op(
	      int32		op_code,		/* 32 bit operand 				*/
	      int32		addressing_mode,	/* form of addressing 				*/
	      RealRegister	destination,		/* registers written to 			*/
	      int32		source,			/* NB/ doubles up as a RealRegister 		*/
	      unsigned long	reads,			/* mask of registers examined by op_code	*/
	      unsigned long	writes )		/* mask of registers altered by op_code		*/
{
  /* send completed op-code */
  
  outinstr( build_op( op_code, addressing_mode, destination, source ), reads, writes );
  
  return;
  
} /* out_diadic_op */

/*}}}*/
/*{{{  out_triadic_op() */

/*
 * build and send a simple triadic operation
 *
 * NB/ when using an ADDR_MODE_INDIRECT form of addressing
 * construct the source field with the build_indirect
 * macro NOT the build_parallel_indirect macro
 */
  
static void
out_triadic_op(
	       int32		op_code,
	       RealRegister	destination,
	       int32		source1,		/* can double up as RealRegister */
	       int32		source2,		/* can double up as RealRegister */
	       int32		source1_type,
	       int32		source2_type,
	       int32		reads,			/* mask of registers examined by operation */
	       int32		writes )		/* mask of registers altered by examination */
{
  int32		op;
  int32		dst;


  dst = hardware_register( destination );
  
  /*
   * when the source type is ADDR_MODE_INDIRECT then
   * the corresponding source field is encoded as:
   *
   * bits  0 - 7  encode 8-bit displacement
   * bits  8 - 10 encode addressing register
   * bits 11 - 15 encode indirect addressing mode
   *
   * and the instruction can be encoded as:
   *
   * bits  0 - 2  encode addressing register      \__ implies INDIRECT_PRE_ADD addressing mode
   * bits  3 - 7  encode 5-bit displacement       /
   *
   * OR
   *
   * bits  0 - 2  encode addressing register      \__ implies a displacement of 1
   * bits  3 - 7  encode indirect addressing mode /
   *
   */
	  
  /* ensure bit fields are the right length */
  
  op_code &= 0x1f;
  dst     &= 0x1f;

  /* partially construct op code */

  op = (0x1U << 29) | (op_code << 23) | (dst << 16);

  /* set addressing bits */
  
  switch (source1_type)
    {
    case ADDR_MODE_REGISTER:

      source1 = hardware_register( (RealRegister)source1 );
      
      switch (source2_type)
	{
	case ADDR_MODE_REGISTER:

	  source2 = hardware_register( (RealRegister)source2 );
	  
	  op |= (0x0U << 21);	/* set the T    field */
	  op |= (0x0U << 28);	/* set the type field */
	  
	  break;
	  
	case ADDR_MODE_INDIRECT:
	  switch ((source2 >> 11) & 0x1f)	/* get indirect addressing type */
	    {
	    case INDIRECT_PRE_ADD:
	      if (source2 & 0xe0 != 0)
		{
		  syserr( gen_bad_displacement );
		}

	      /* extract register and 5 bit displacement - addressing mode ignored */
	      
	      source2 = ((source2 & 0x1f) << 3) | indirect_addr_reg( source2 );

	      /* set up type fields */
	      
	      op |= (0x1U << 21);	/* set the T field */
	      op |= (0x1U << 28);	/* set the type field */
	      
	      break;

	    default:	/* in all other cases we loose the displacment field */

	      if (source2 & (B_11000 << 1))
		{
		  /* index register relative */
		  
		  if ((source2 & 0xff) != 0)
		    {
		      syserr( gen_bad_displacement );
		    }
		}
	      else if ((source2 & 0xff) != 1)
		{
		  syserr( gen_bad_displacement );
		}

	      /* extract addressing mode and register - displacement ignored */
	      
	      source2 = (((source2 >> 11) & 0x1f) << 3) | indirect_addr_reg( source2 );
	      
	      /* set up type fields */
	      
	      op |= (0x2 << 21);	/* set the T field */
	      op |= (0x0 << 28);	/* set the type field */
	      
	      break;
	    }
	  break;
	  
	case ADDR_MODE_IMMEDIATE:
	  switch (op_code)
	    {
#ifdef TRIADIC_BINARY_OPS_ARE_UNSIGNED
	    case OP_AND3:
	    case OP_OR3:
	    case OP_XOR3:
	      if (!fits_in_8_bits_unsigned( source2 ))
		{
		  syserr( gen_value_too_big );
		}
	      break;
#endif	      
	    default:
	      if (!fits_in_8_bits_signed( source2 ))
		{
		  syserr( gen_value_too_big );
		}
	      break;
	    }
	  
	  /* set up type fields */
	  
	  op |= (0x0U << 21);	/* set the T field */
	  op |= (0x1U << 28);	/* set the type field */
	  
	  break;
	  
	default:
	  syserr( gen_unknown_addressing, source2_type );
	  break;
	}
      break;

    case ADDR_MODE_INDIRECT:
      switch (source2_type)
	{
	case ADDR_MODE_REGISTER:
	  if (source1 & (B_11000 << 11))
	    {
	      /* index register relative */
	      
	      if ((source1 & 0xff) != 0)
		{
		  syserr( gen_bad_displacement );
		}
	    }
	  else if ((source1 & 0xff) != 1)
	    {
	      syserr( gen_bad_displacement );
	    }

	  /* extract addressing mode and register fields - displacement is ignored */
	      
	  source1 = ((source1 >> 11) & 0x1f) << 3 | indirect_addr_reg( source1 );

	  source2 = hardware_register( (RealRegister)source2 );
	      
	  /* set up type fields */
	      
	  op |= (0x1U << 21);	/* set the T field */
	  op |= (0x0U << 28);	/* set the type field */
	      
	  break;
	  
	case ADDR_MODE_INDIRECT:
	  if (((source1 >> 11) & 0x1f) == INDIRECT_PRE_ADD &&
	      ((source2 >> 11) & 0x1f) == INDIRECT_PRE_ADD )
	    {
	      /* ensure that displacments are 5 bits */
	      
	      if ((source1 & 0xe0) != 0 ||
		  (source2 & 0xe0) != 0  )
		{
		  syserr( gen_bad_displacement );
		}

	      /* extract register and 5 bit displacement - addressing mode ignored */
	      
	      source1 = ((source1 & 0x1f) << 3) | indirect_addr_reg( source1 );
	      source2 = ((source2 & 0x1f) << 3) | indirect_addr_reg( source2 );
	      
	      /* set up type fields */
	      
	      op |= (0x3U << 21);	/* set the T field */
	      op |= (0x1U << 28);	/* set the type field */
	    }
	  else
	    {
	      /* ensure that displacement is 1 */

	      if (source1 & (B_11000 << 11))
		{
		  /* index register relative */
	      
		  if ((source1 & 0xff) != 0)
		    {
		      syserr( gen_bad_displacement );
		    }
		}
	      else if ((source1 & 0xff) != 1)
		{
		  syserr( gen_bad_displacement );
		}

	      if (source2 & (B_11000 << 11))
		{
		  /* index register relative */
	      
		  if ((source2 & 0xff) != 0)
		    {
		      syserr( gen_bad_displacement );
		    }
		}
	      else if ((source2 & 0xff) != 1)
		{
		  syserr( gen_bad_displacement );
		}

	      /* extract addressing mode and register fields - displacement is ignored */
	      
	      source1 = (((source1 >> 11) & 0x1f) << 3) | indirect_addr_reg( source1 );
	      source2 = (((source2 >> 11) & 0x1f) << 3) | indirect_addr_reg( source2 );
	      
	      /* set up type fields */
	      
	      op |= (0x3U << 21);	/* set the T field */
	      op |= (0x0U << 28);	/* set the type field */
	    }
	  
	  break;
	    
	case ADDR_MODE_IMMEDIATE:
	  /* ensure that indirect address mode is PRE_ADD */
	  
	  if (((source1 >> 11) & 0x1f) != INDIRECT_PRE_ADD)
	    {
	      syserr( gen_unknown_addressing, source1 );
	    }

	  /* ensure displacement is 5 bits */
	  
	  if ((source1 & 0xe0) != 0)
	    {
	      syserr( gen_bad_displacement );
	    }

	  /* extract register and 5 bit displacement - addressing mode ignored */
	      
	  source1 = ((source1 & 0x1f) << 3) | indirect_addr_reg( source1 );

	  /* ensure that immediate value fits in 8 bits */
	  
	  if (!fits_in_8_bits_signed( source2 ))
	    {
	      syserr( syserr_bad_addressing_mode );
	    }

	  /* set up type fields */
	      
	  op |= (0x2U << 21);	/* set the T field */
	  op |= (0x1U << 28);	/* set the type field */

	  break;
	  
	default:
	  syserr( gen_unknown_addressing, source2_type );
	  break;
	}
      break;

    default:
      syserr( gen_unknown_addressing, source1_type );
      break;
    }

  source1 &= 0xff;
  source2 &= 0xff;

  op |=  (source1 << 8) | source2;
  
  /* send completed op-code */

  outinstr( op, reads, writes );
  
  return; 
  
} /* out_triadic_op */

/*}}}*/
/*{{{  C_FROMQ() */

static unsigned long
C_FROMQ( int32 q )
{
  switch (q & Q_MASK)
    {
    case Q_EQ:	return C_EQ;	/* equal to */
    case Q_NE:	return C_NE;	/* not equal to */
    case Q_HS:	return C_HS;	/* higher than or the same (unsigned) */
    case Q_LO:	return C_LO;	/* lower than (unsigned) */
    case Q_HI:	return C_HI;	/* higher than (unsigned) */
    case Q_LS:	return C_LS;	/* less than or the same (unsigned) */
    case Q_GE:	return C_GE;	/* greater than or equal to (signed) */
    case Q_LT:	return C_LT;	/* less than (signed) */
    case Q_GT:	return C_GT;	/* greater than (signed) */
    case Q_LE:	return C_LE;	/* less than or equal to (signed) */
    case Q_AL:	return C_U;	/* always */
    case Q_UEQ:	return C_EQ;	/* equal to (unsigned) */
    case Q_UNE:	return C_NE;	/* not equal to (unsigned) */
    case Q_NOT:	syserr( syserr_unsupported_branch_type, q );	/* never */      
    default:	syserr( syserr_unsupported_branch_type, q );
    }

  return C_U;
  
} /* C_FROMQ */

/*}}}*/
/*{{{  conditional_load() */

/*
 * build and send a conditional load op code
 */
  
static void
conditional_load(
		 int32		condition,		/* condition in Norcroft format     */
		 int32		addressing_mode,	/* addressing mode for the load     */
		 RealRegister	destination,		/* destination register             */
		 int32		source,			/* NB/ doubles up as a RealRegister */
		 int32		reads,			/* bit mask of registers examined   */
		 int32		writes )		/* bit mask of registers altered    */
{
  int32		op;

  
  addressing_mode &= 0x3;
  destination     &= 0x1f;
  source          &= 0xffff;

  op = (OP_LDIc << 28) | (C_FROMQ( condition ) << 23) | (hardware_register( destination ) << 16);

  if (addressing_mode == ADDR_MODE_REGISTER)
    {
      outinstr( op | hardware_register( source ), reads, writes );
    }
  else
    {
      outinstr( op | (addressing_mode << 21) | source, reads, writes );
    }

  peep_forget_about( destination );
  
  return;
  
} /* conditional_load */

/*}}}*/
/*{{{  conditional_load_float() */

/*
 * build and send a conditional FP load op code
 */
  
static void
conditional_load_float(
		       int32		condition,
		       int32		addressing_mode,
		       RealRegister	destination,
		       int32		source,		/* NB/ doubles up as a RealRegister */
		       int32		reads,
		       int32		writes )
{
  int32		op;

  
  addressing_mode &= 0x3;
  destination     &= 0x1f;
  source          &= 0xffff;

  op = (OP_LDFc << 28) | (C_FROMQ( condition ) << 23) | (hardware_register( destination ) << 16);

  if (addressing_mode == ADDR_MODE_REGISTER)
    {
      outinstr( op | hardware_register( source ), reads, writes );
    }
  else
    {
      outinstr( op | (addressing_mode << 21) | source, reads, writes );
    }

  peep_forget_about( destination );
  
  return;
  
} /* conditional_load_float */

/*}}}*/

/*}}}*/
/*{{{  Miscellaneous */

/*{{{  firstbit() */

/*
 * returns the index of the first bit set in a word.
 * The least significant bit is index 0 (not 1).
 * cf ffs().
 */
  
static int
firstbit( int32 w )
{
  int 		i;
  int32		mask = 1;
  

  for (i = 0; i < 32; i++)
    if (w & mask)
      return i;
    else
      mask <<= 1;
  
  syserr( syserr_firstbit );
  
  return 0;

} /* firstbit */

/*}}}*/
#ifdef NOT_USED
/*{{{  lastbit() */

/*
 * returns the index of the last bit set in a word.
 * The least significant bit is index 0 (not 1).
 * cf ffs().
 */
  
static int
lastbit( int32 w )
{
  int 		i;
  int32		mask = 0x80000000U;
  

  for (i = 32; i--;)
    if (w & mask)
      return i;
    else
      mask >>= 1;
  
  syserr( syserr_firstbit );
  
  return 0;

} /* lastbit */

/*}}}*/
#endif /* NOT_USED */

/*}}}*/
/*{{{  Register Movement */

/*{{{  move_register() */

/*
 * move register 'source' into register 'dest'
 * MUST only use one instruction
 */

void
move_register(
	      RealRegister	source,
	      RealRegister	dest,
	      bool		floating )
{
  /* make sure that the destination is not cached in a register push */

  maybe_flush_pending_push( dest );
  
  if (source != dest)
    {
      if (floating && is_float( source ) && is_float( dest ))
	{
	  out_diadic_op( OP_LDF, ADDR_MODE_REGISTER, dest, source,
			examines1( source ),
			alters2( dest, RR_ST ) );
	}
      else if (is_special_register( hardware_register( dest ) ))
	{
	  out_diadic_op( OP_LDA, ADDR_MODE_REGISTER, dest, source,
			examines1( source ),
			alters1( dest ) );
	}
      else
	{
	  out_diadic_op( OP_LDI, ADDR_MODE_REGISTER, dest, source,
			examines1( source ),
			alters2( dest, RR_ST ) );
	}
    }

  /* if dest is also in an address register, forget about it */
  
  peep_forget_about( dest );
  
  return;
  
} /* move_register */

/*}}}*/
/*{{{  ipush() */

/* push a integer register onto the stack */

void
ipush( RealRegister reg )
{
  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, reg, build_indirect( INDIRECT_PRE_DECR, R_SP, 1 ),
		examines2( reg, R_SP ),
		alters1( R_SP ) );

  peep_change_addr_offset( R_SP, -1 );
  
  return;
  
} /* ipush */

/*}}}*/
/*{{{  ipop() */

  
/* pop an integer register from the stack */

void
ipop( RealRegister reg )
{
  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, reg, build_indirect( INDIRECT_POST_INCR, R_SP, 1 ),
		examines1( R_SP ),
		alters3( R_SP, reg, RR_ST ) );

  peep_change_addr_offset( R_SP, +1 );
  
  return;
  
} /* ipop */

/*}}}*/
/*{{{  fpush() */

/* push a floating point register onto the stack */

void
fpush( RealRegister reg )
{
#ifdef DEBUG
  if (!is_float( reg ))
    syserr( gen_bad_float_push );
#endif
  
  out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, reg, build_indirect( INDIRECT_PRE_DECR, R_SP, 1 ),
		examines2( reg, R_SP ),
		alters1( R_SP ) );

  peep_change_addr_offset( R_SP, -1 );
  
  return;
  
} /* fpush */

/*}}}*/
/*{{{  fpop() */

  
/* pop a floating point register from the stack */

static void
fpop( RealRegister reg )
{
#ifdef DEBUG
  if (!is_float( reg ))
    syserr( gen_bad_float_pop );
#endif
  
  out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, reg, build_indirect( INDIRECT_POST_INCR, R_SP, 1 ),
		examines1( R_SP ),
		alters3( R_SP, reg, RR_ST ) );

  peep_change_addr_offset( R_SP, +1 );
  
  return;
  
} /* fpop */

/*}}}*/
/*{{{  dpush() */

  
/* push a double precision register onto the stack */

void
dpush( RealRegister reg )
{
#ifdef DEBUG
  if (!is_float( reg ))
    syserr( gen_bad_float_push );
#endif
  
  /* NB/ opposite order to dpop() */
  
  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, reg, build_indirect( INDIRECT_PRE_DECR, R_SP, 1 ),
		examines2( reg, R_SP ),
		alters1( R_SP ) );

  out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, reg, build_indirect( INDIRECT_PRE_DECR, R_SP, 1 ),
		examines2( reg, R_SP ),
		alters1( R_SP ) );

  peep_change_addr_offset( R_SP, -2 );
  
  return;
  
} /* dpush */

/*}}}*/
/*{{{  dpop() */

  
/* pop a double precision register from the stack */

static void
dpop( RealRegister reg )
{
#ifdef DEBUG
  if (!is_float( reg ))
    syserr( gen_bad_float_pop );
#endif
  
  /* NB/ LDF before LDI */
  
  out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, reg, build_indirect( INDIRECT_POST_INCR, R_SP, 1 ),
		examines1( R_SP ),
		alters2( reg, RR_ST ) );
  
  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, reg, build_indirect( INDIRECT_POST_INCR, R_SP, 1 ),
		examines1( R_SP ),
		alters3( R_SP, reg, RR_ST ) );

  peep_change_addr_offset( R_SP, +2 );
  
  return;
  
} /* dpop */

/*}}}*/

/*}}}*/
/*{{{  Register Load/Store */

/*{{{  load_integer() */

  
/*
 * Set register 'r' to the integer 'n'
 * if 'must_save_st' is true then the status register must be preserved
 */

static void
load_integer(
	     RealRegister	r,
	     int32 		n,
	     bool		must_save_st )
{
#if 0	/* This can corrupt the value in DP in the instruction before this one !!!! */
  if (fits_in_16_bits_unsigned( n )  &&
      hardware_register( r ) == 0x10  ) /* DP */
    {
      out_diadic_op( OP_LDPK, ADDR_MODE_IMMEDIATE, r, n,
		    examines0(),
		    alters1( r ) );

      return;
    }
#endif
  
  if (fits_in_16_bits_signed( n ))
    {
      /* 16 bit signed immediate, does not set ST  */

      if (is_special_register( hardware_register( r ) ))
	{
	  out_diadic_op( OP_LDA, ADDR_MODE_IMMEDIATE, r, n,
			examines0(),
			alters1( r ) );
	}
      else
	{
	  conditional_load( Q_AL, ADDR_MODE_IMMEDIATE, r, n,
			   examines0(),
			   alters1( r ) );
	}

      return;
    }
  else if ((n & 0xffff) == 0)
    {
      /* only 16 msb's set, use special half word load instruction */
      
      out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, r, n >> 16,
		    examines0(),
		    alters1( r ) );

      return;
    }

  if (must_save_st)
    {
      /*
       * save status register
       *
       * (NB/ since this function can be called from anywhere in
       * compiler we cannot use one of the temporary registers).
       */

      ipush( RR_ST );
    }
  
  if ((n & 0xffff0000U) == 0xffff0000U)
    {
      /* all 16 msb's set, use inversion */
      
        out_diadic_op( OP_NOT, ADDR_MODE_IMMEDIATE, r, ~n,
		    examines0(),
		    alters2( r, RR_ST ) );
    }
  else
    {
      /*
       * Other op codes we could try ....
       *
       * LDF, NORM, RND, FLOAT, FIX
       */
      
      /* load high part -- bottom 16 bits are set to 0 */

      out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, r, n >> 16,
		    examines0(),
		    alters1( r ) );

      /* load low part */
      
      out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, r, n,
		    examines1( r ),
		    alters2( r, RR_ST ) );
    }
      
  if (must_save_st)
    {
      /* restore ST */

      ipop( RR_ST );
    }
  
  return;

} /* load_integer */

/*}}}*/
/*{{{  integer_immediate_op() */

/*
 * output an instruction using immediate addressing with integer values
 * handles the case when triadic addressing may be necessary
 */

static void
integer_immediate_op(
		     int32		diadic_op,	/* diadic version of the operation */
		     int32		triadic_op,	/* triadic version of the operation */
		     RealRegister	dest,		/* detination register for operation */
		     RealRegister	source,		/* source register for operation */
		     int32		value,		/* the immediate value */
		     bool		signed_op )	/* TRUE if value op takes signed immediate values */
{
  if (is_word_addressed_( source ) || is_word_addressed_( dest ))
    {
      flush_pending_pushes();

      if (value & (sizeof_int - 1))
	syserr( gen_non_word_offset );
      
      /* calculations use word addressing */
      
      value /= sizeof_int;
    }

  if (value == 0 && !no_peepholing)
    {
      if (diadic_op == OP_OR   ||
	  diadic_op == OP_XOR  ||
	  diadic_op == OP_ADDI ||
	  diadic_op == OP_SUBI )
	{
	  if (source != dest)
	    {
	      move_register( source, dest, FALSE );
	      
	      peepf( "transformed addition into move" );

	      ++peep_transformed;
	    }
	  else
	    {
	      peepf( "eliminating op %lx with value 0", diadic_op );

	      ++peep_eliminated;
	    }

	  return;
	}
      else
	{
	  cc_warn( "possibly generating unneccessary op code %lx!", diadic_op );
	}
    }
  
  if (dest == source && (signed_op ? fits_in_16_bits_signed( value ) : fits_in_16_bits_unsigned( value )))
    {
      out_diadic_op( diadic_op, ADDR_MODE_IMMEDIATE, dest, value,
		    examines1( dest ),
		    alters2( dest, RR_ST ) );
    }
#ifdef TRIADIC_BINARY_OPS_ARE_UNSIGNED
  else if (signed_op ? fits_in_8_bits_signed( value ) : fits_in_8_bits_unsigned( value ))
#else
  else if (fits_in_8_bits_signed( value ))
#endif
    {
      out_triadic_op( triadic_op, dest, source, value, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
		     examines1( source ),
		     alters2( dest, RR_ST ) );
    }
  else if (diadic_op == OP_AND)
    {
      if (source == dest && fits_in_16_bits_unsigned( ~value ))
	{
	  out_diadic_op( OP_ANDN, ADDR_MODE_IMMEDIATE, dest, ~value,
			examines1( dest ),
			alters2( dest, RR_ST ) );
	}
#ifdef TRIADIC_BINARY_OPS_ARE_UNSIGNED
      else if (fits_in_8_bits_unsigned( ~value ))
#else
      else if (fits_in_8_bits_signed( ~value ))
#endif
	{
	  out_triadic_op( OP_ANDN3, dest, source, ~value, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			 examines1( source ),
			 alters2( dest, RR_ST ) );
	}
      else
	{
	  RealRegister	tmp;

	  
	  tmp = R_TMP1;

	  if (dest == R_TMP1)
	    {
	      if (source == R_TMP2)
		{
		  tmp = R_TMP3;
		}
	      else
		{
		  tmp = R_TMP2;
		}
	    }
	  else if (source == R_TMP1)
	    {
	      if (dest == R_TMP2)
		{
		  tmp = R_TMP3;
		}
	      else
		{
		  tmp = R_TMP2;
		}
	    }
	  else
	    {
	      tmp = R_TMP1;
	    }
	  
	  load_integer( tmp, value, FALSE );
      
	  out_triadic_op( OP_AND3, dest, source, tmp, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( tmp, source ),
		     alters2( dest, RR_ST ) );
	}
    }
  else
    {
      RealRegister	tmp;

	  
      tmp = R_TMP1;
      
      if (dest == R_TMP1)
	{
	  if (source == R_TMP2)
	    {
	      tmp = R_TMP3;
	    }
	  else
	    {
	      tmp = R_TMP2;
	    }
	}
      else if (source == R_TMP1)
	{
	  if (dest == R_TMP2)
	    {
	      tmp = R_TMP3;
	    }
	  else
	    {
	      tmp = R_TMP2;
	    }
	}
      else
	{
	  tmp = R_TMP1;
	}
      
      load_integer( tmp, value, FALSE );
      
      out_triadic_op( triadic_op, dest, source, tmp, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( tmp, source ),
		     alters2( dest, RR_ST ) );
    }

  /* we have altered dest so do not remember it - maybe optimise one day ? */
  
  peep_forget_about( dest );
  
  return;
  
} /* integer_immediate_op */

/*}}}*/
#ifdef TARGET_HAS_FP_LITERALS
/*{{{  convert_ro_C40_format() */

/*
 * Converts an IEEE number produced by the compiler into
 * a C40 short format floating point format number.  Returns
 * 0xFFFFFFFFU if the conversion fails
 */


static int32
convert_to_C40_format(
		      FloatCon *	fc,		/* the immediate value                  */
		      bool		is_double )	/* TRUE iff 'fc' is a 64 bit IEEE value */
{
  if (is_double)
    {
      int32	high;
      int32	low;

      
      high = IEEE_to_extended_float( fc->floatbin.db.msd, fc->floatbin.db.lsd, &low );

      /* catch 0.0 */
      
      if (low == 0 && high == 0x80000000U)
	return 0x8000;
      
      if ((low & 0xfffff) == 0 &&		   /* no bits will be lost by the truncation */
	  (((high & 0xf8000000U) == 0) ||	   /* exponent is 0 to 7 */
	   ((high & 0xff000000U) >  0xf8000000U))) /* exponent in range -1 to -7 */
	{
	  return (high >> 12);
	}
    }
  else
    {
      int32		val;
  

      val = IEEE_to_single_float( fc->floatbin.fb.val );


      /* catch 0.0 */
      
      if (val == 0x80000000U)
	return 0x8000;

      if ( ((val & 0xfff) == 0) &&			/* no bits will be lost by truncation */
	  (((val & 0xf8000000U) == 0) ||		/* exponent is in range 0 to 7 */
	   ((val & 0xff000000U) >  0xf8000000U)))	/* exponent is in range -1 to -7 */
	return (val >> 12);
    }

  /* indicate that the conversion failed */
  
  return 0xFFFFFFFFU;
  
} /* convert_to_C40_format */

/*}}}*/
/*{{{  fpliteral() */

/*
 * returns TRUE if the floating point value is suitable for use by the op code indicated
 */

bool
fpliteral(
	  FloatCon *	fc,
	  J_OPCODE	op )
{
  if (convert_to_C40_format( fc, (bool) J_fltisdouble( op ) ) != 0xFFFFFFFFU)
    return TRUE;

  return FALSE;
  
} /* fpliteral */

/*}}}*/
/*{{{  float_immediate_op() */

/*
 * output an instruction using immediate addressing with floating point values
 * handles the case when triadic addressing may be necessary
 */

static void
float_immediate_op(
		   int32	op,		/* the instruction to be performed      */
		   RealRegister	dest,		/* detination register for operation    */
		   RealRegister	source,		/* source register for operation        */
		   FloatCon *	fc,		/* the immediate value                  */
		   bool		is_double )	/* TRUE iff 'fc' is a 64 bit IEEE value */
{
  int32		val;
  

#ifdef DEBUG
  if (!is_float( source ))
    syserr( gen_non_FP_source );
  
  if (source != dest && !is_float( dest ))
    syserr( gen_non_FP_dest );
#endif

  val = convert_to_C40_format( fc, is_double );

  if (val == 0xFFFFFFFFU)
    {
      syserr( gen_FP_value_not_fit );
  
      return;
    }
  
  /* check for operations using immediate value 0.0 */
  
  if (val == 0x8000)
    {
      if (!no_peepholing)
	{
	  if (op == OP_ADDF || op == OP_SUBF)
	    {
	      if (source != dest)
		{
		  out_diadic_op( OP_LDF, ADDR_MODE_REGISTER, dest, source,
				examines1( source ),
				alters2( dest, RR_ST ) );
		  
		  peepf( "transformed FP addition of 0.0 into move" );

		  ++peep_transformed;
		}
	      else
		{
		  peepf( "eliminating op %lx with value 0", op );

		  ++peep_eliminated;
		}
	      
	      return;
	    }
	  else if (op == OP_MPYF)
	    {
	      out_diadic_op( OP_LDF, ADDR_MODE_IMMEDIATE, dest,
			    0x8000,
			    examines0(),
			    alters2( dest, RR_ST ) );
	      
	      peepf( "transformed FP multiplication of 0.0 into load" );

	      ++peep_transformed;
	      
	      return;
	    }
	  
	  cc_warn( "possibly generating unneccessary op code %lx!", op );
	}
      
      if (dest == source)
	{
	  out_diadic_op( op, ADDR_MODE_IMMEDIATE, source,
			0x8000,
			examines1( source ),
			alters2( source, RR_ST ) );
	}
      else
	{
	  out_diadic_op( OP_LDF, ADDR_MODE_REGISTER, dest, source,
			examines1( source ),
			alters2( dest, RR_ST ) );
	  
	  out_diadic_op( op, ADDR_MODE_IMMEDIATE, dest,
			0x8000,
			examines1( dest ),
			alters2( dest, RR_ST ) );
	}
    }

  if (dest != source)
    {
      out_diadic_op( OP_LDF, ADDR_MODE_REGISTER, dest, source,
		    examines1( source ),
		    alters2( dest, RR_ST ) );
    }

  out_diadic_op( op, ADDR_MODE_IMMEDIATE, dest, val,
		examines1( dest ),
		alters2( dest, RR_ST ) );
  
  return;
  
} /* float_immediate_op */

/*}}}*/
/*{{{  compare_float() */

/*
 * Compare register 'r' with the floating point constant 'fc' using comparison 'test'
 */

static void
compare_float(
	      RealRegister	r,
	      FloatCon *	fc,
	      int32 		test,
	      bool		is_double )
{
  int32		val;
  
  
  if (test == Q_AL)
    {
      syserr( gen_void_compare );
    }
  
  correct_stack( TRUE );

  val = convert_to_C40_format( fc, is_double );

  if (val == 0xFFFFFFFFU)
    {
      syserr( gen_FP_value_not_fit );

      return;
    }

  if (!is_double && fast_FP)
    {
      out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r, r,
		    examines1( r ),
		    alters2( r, RR_ST ) );
    }
  
  out_diadic_op( OP_CMPF, ADDR_MODE_IMMEDIATE, r,
		val,
		examines1( r ),
		alters1( RR_ST ) );

  return;
  
} /* compare_float */

/*}}}*/
#endif /* TARGET_HAS_FP_LITERALS */
/*{{{  load_float() */

/*
 * Set register 'r' to the floating point value 'n'
 */

static void
load_float(
	   RealRegister	r,		/* register into which constant is to be loaded	*/
	   FloatCon * 	fc,		/* constant to be loaded 			*/
	   bool		is_double )	/* true if the constant is double precision	*/
{
  int32	val;

#ifdef DEBUG
  if (!is_float( r ))
    syserr( gen_non_FP_dest );
#endif
  
  val = convert_to_C40_format( fc, is_double );
      
  if (val != 0xFFFFFFFFU)
    {
      out_diadic_op( OP_LDF, ADDR_MODE_IMMEDIATE, r, val,
		    examines0(),
		    alters2( r, RR_ST ) );
    }
  else if (is_double)
    {
      unsigned32	high;
      int32		low;

      
      high = (unsigned32) IEEE_to_extended_float( fc->floatbin.db.msd, fc->floatbin.db.lsd, &low );
      
      /*
       * Other op codes we could try ....
       *
       * FLOAT, LDE, LDM, RND, FLOAT
       */
      
      /* load high part of FP number */
      
      load_integer( R_TMP1, high, FALSE );
      
      /* save as integer */
      
      ipush( R_TMP1 );
      
      /* pop as FP */
      
      fpop( r );
      
      if (low & 0xFF)
	{
	  /* OR in low part of FP number */
	  
	  out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, r, low & 0xFF,
			examines1( r ),
			alters2( r, RR_ST ) );
	}
    }
  else
    {
      int32	val;


      /* convert */
      
      val = IEEE_to_single_float( fc->floatbin.fb.val );
      
      /* load high part of FP number */
	  
      load_integer( R_TMP1, val, FALSE );

      /* save as integer */
      
      ipush( R_TMP1 );
      
      /* pop as FP */
      
      fpop( r );
    }

  return;
  
} /* load_float */

/*}}}*/
/*{{{  convert_to_word_alignment() */

/*
 * converts the byte offset in 'src' into a word offset in 'dest'
 * returns TRUE if the result is already biased by R_BASE
 */

static bool
convert_to_word_alignment(
			  RealRegister	src,		/* register contain byte offset 	*/
			  RealRegister	dest,		/* register to contain word offset 	*/
			  int32		offset,		/* offset in bytes to be added in 	*/
			  int32 *	bias_offset )	/* return value for bias offset or NULL	*/
{
  RealRegister	ar;
  int32		off;
  bool		biased;
  
  
  offset /= sizeof_int;

  ar = peep_find_loaded_addr_reg( src, &off, &biased );
  
  if (!no_peepholing      &&
      bias_offset != NULL &&
      ar          != GAP   )
    {
      if (offset == off)
	{
	  peepf( "eliminated load and add to temporary address register" );

	  if (ar == dest)
	    {
	      peep_eliminated += 2;
	    }
	  else
	    {
	      move_register( ar, dest, FALSE );

	      peep_note_addr_reg_loaded( dest, src, off, biased );
	      
	      peep_eliminated += 1;
	    }

	  if (biased)
	    {
	      *bias_offset = 0;

	      return TRUE;
	    }
	  
	  return FALSE;
	}
      else if (biased)
	{
	  if (fits_in_8_bits_unsigned( abs( offset - off )))
	    {
	      if (ar != dest)
		{
		  move_register( ar, dest, FALSE );

		  peep_corrupt_addr_reg( ar );

		  peep_note_addr_reg_loaded( dest, src, offset, TRUE );
		}

	      *bias_offset = offset - off;

	      return TRUE;
	    }
	}
      else
	{
	  if (ar == dest && fits_in_16_bits_signed( offset - off ))
	    {
	      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, dest, offset - off,
			    examines1( dest ),
			    alters2( dest, RR_ST ) );
		  
	      peepf( "eliminated load to temporary address register" );

	      ++peep_eliminated;
	      
	      peep_note_addr_reg_loaded( dest, src, offset, FALSE );
		  
	      return FALSE;
	    }
	}
    }
  
  /* divide address by sizeof_int (here assumed to be 4) and place in dest */

  if (dest == src)
    {
      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, dest, -2,
		    examines1( dest ),
		    alters2( dest, RR_ST ) );
    }
  else
    {
      out_triadic_op( OP_LSH3, dest, src, -2, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
		     examines1( src ),
		     alters2( dest, RR_ST ) );
    }

  if (offset == 0)
    {
      ;	/* do nothing */
    }
  else if (fits_in_16_bits_signed( offset ))
    {
      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, dest, offset,
		    examines1( dest ),
		    alters2( dest, RR_ST ) );
    }
  else
    {
      if (dest == R_TMP2)
	syserr( gen_offset_reg_conflict );

      load_integer( R_TMP2, offset, FALSE );
      
      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, dest, R_TMP2,
		    examines2( dest, R_TMP2 ),
		    alters2( dest, RR_ST ) );
    }
      
  peep_note_addr_reg_loaded( dest, src, offset, FALSE );
  
  return FALSE;

} /* convert_to_word_alignment */

/*}}}*/
/*{{{  out_immediate_offset() */

/*
 * perform 'dst' = 'op' 'src','offset'
 *
 * ie using indirect addressing off 'src'
 */

static void
out_immediate_offset(
		     int32		op,			/* op code of operation to be performed	*/
		     RealRegister	dst,			/* register to be loaded with result	*/
		     RealRegister	src,			/* register pointing to source value	*/
		     int32		offset,			/* offset from location pointed to	*/
		     int32		can_corrupt_src )	/* non-zero if 'src' may be courrupted	*/
{
  bool		is_store = (op == OP_STI || op == OP_STF);
  RealRegister	ar       = R_ATMP;


  if (is_word_addressed_( src ))
    {
      /*
       * the stack pointer and frame pointers are always
       * word pointers so no special adjustments should
       * be necessary
       */
      
      /* check to see if we about to access the stack and there are pushes pending */
      
      if (src == R_SP)
	{
	  flush_pending_pushes();
	}
	  
      offset /= sizeof_int;

      if (offset)
	{
	  if (!fits_in_8_bits_unsigned( abs( offset ) ))
	    {
	      int32		off;
	      bool		biased;


	      ar = peep_find_loaded_addr_reg( src, &off, &biased );
	      
	      if (!no_peepholing  &&
		  ar     != GAP   &&
		  biased == FALSE &&
		  fits_in_16_bits_signed( offset - off ) )
		{
		  peepf( "eliminated load to temporary address register" );

		  if ((offset - off) == 0)
		    {
		      peep_eliminated += 2;
		    }
		  else
		    {
		      ++peep_eliminated;
		      
		      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, ar, offset - off,
				    examines1( ar ),
				    alters2( ar, RR_ST ) );
		    }
		}
	      else
		{
		  if (ar == GAP)
		    {
		      ar = peep_get_free_addr_reg( GAP );
		    }
		  
		  load_integer( ar, offset, FALSE );

		  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, src,
				examines2( src, ar ),
				alters2( ar, RR_ST ) );
		}
			    
	      out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
			    is_store ? examines2( dst, ar ) : examines1( ar ),
			    is_store ? alters0()            : alters2( dst, RR_ST ) );

	      peep_note_addr_reg_loaded( ar, src, offset, FALSE );
	    }
	  else if (offset > 0)
	    {
	      out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_ADD, src, offset ),
			    is_store ? examines2( dst, src ) : examines1( src ),
			    is_store ? alters0()             : alters2( dst, RR_ST ) );
	    }
	  else
	    {
	      out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_SUB, src, -offset ),
			    is_store ? examines2( dst, src ) : examines1( src ),
			    is_store ? alters0()             : alters2( dst, RR_ST ) );
	    }
	}
      else
	{
	  out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, src, 0 ),
			    is_store ? examines2( dst, src ) : examines1( src ),
			    is_store ? alters0()             : alters2( dst, RR_ST ) );
	}
    }
  else if (can_corrupt_src && is_address_register( hardware_register( src )))
    {
      /* divide by sizeof_int (here assumed to be 4) */
      
      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, src, -2,
		     examines1( src ),
		     alters2( src, RR_ST ) );

      offset /= sizeof_int;

      if (offset == 0)
	{
	  ;	/* do nothing */
	}
      else if (fits_in_16_bits_signed( offset ))
	{
	  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, src, offset,
			examines1( src ),
			alters2( src, RR_ST ) );
	}
      else
	{
	  if (src == R_TMP2)
	    syserr( gen_offset_reg_conflict );

	  load_integer( R_TMP2, offset, FALSE );
	  
	  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, src, R_TMP2,
			examines2( src, R_TMP2 ),
			alters2( src, RR_ST ) );
	}

      /* send the operation */
      
      out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR_IR0, src, 0 ),
		    is_store ? examines3( dst, R_BASE, src ) : examines2( R_BASE, src ),
		    is_store ? alters1( src )                : alters3( src, dst, RR_ST ) );
      
      peep_corrupt_addr_reg( src );
    }
  else
    {
      int32	off;


      
      if (src == R_ATMP)
	{
	  /* XXX - this can happen with the code generated during the static data initialisation phase */
	  
	  ar = RR_AR2;
	}
      else
	{
	  ar = peep_find_loaded_addr_reg( src, NULL, NULL );
	  
	  if (ar == GAP)
	    ar = peep_get_free_addr_reg( src );

	  if (ar == GAP)
	    {
	      peep_corrupt_addr_reg( R_ATMP );
	      
	      ar = R_ATMP;
	    }
	}

      if (convert_to_word_alignment( src, ar, offset, &off ))
	{
	  if (off > 0)
	    out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR, ar, off ),
			  is_store ? examines2( dst, ar ) : examines1( ar ),
			  is_store ? alters1( ar )        : alters3( ar, dst, RR_ST ) );
	  else if (off == 0)
	    out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
			  is_store ? examines2( dst, ar ) : examines1( ar ),
			  is_store ? alters0()            : alters2( dst, RR_ST ) );
	  else
	    out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_DECR, ar, -off ),
			  is_store ? examines2( dst, ar ) : examines1( ar ),
			  is_store ? alters1( ar )        : alters3( ar, dst, RR_ST ) );
	}
      else
	{
	  /* send the operation */
      
	  out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR_IR0, ar, 0 ),
			is_store ? examines3( dst, R_BASE, ar ) : examines2( R_BASE, ar ),
			is_store ? alters1( ar )                : alters3( ar, dst, RR_ST ) );
	}
      
      peep_note_addr_reg_loaded( ar, src, offset / sizeof_int, TRUE );
    }
  
  if (op == OP_LDI || op == OP_LDF || op == OP_LDA)
    {
      /* if dest is also in an address register, forget about it */
      
      peep_forget_about( dst );      
    }  

  return;

} /* out_immediate_offset */

/*}}}*/
/*{{{  load_double_relative() */

/*
 * load a double precision value from the address in 'dst'
 * offset by 'offset'
 */

static void
load_double_relative(
		     RealRegister	dst,			/* register to be loaded 		*/
		     RealRegister	src,			/* register containing address 		*/
		     int32		offset,			/* byte offset from the address 	*/
		     int32		can_corrupt_src )	/* non-zero if 'src' may be courrupted	*/
{
  RealRegister	ar;


#ifdef DEBUG
  if (!is_float( dst ))
    syserr( gen_non_FP_dest );
#endif
  
  /* NB/ LDF before LDI */

  if (is_word_addressed_( src ))
    {
      if (src == R_SP)
	{
	  flush_pending_pushes();
	}
	  
      offset /= sizeof_int;

      if (fits_in_8_bits_unsigned( abs( offset + 1 ) ))
	{
	  if (offset == 0)
	    {
	      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst,
			    build_indirect( INDIRECT_REL, src, 0 ),
			    examines1( src ),
			    alters2( dst, RR_ST ) );
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst,
			    build_indirect( INDIRECT_PRE_ADD, src, 1 ),
			    examines1( src ),
			    alters2( dst, RR_ST ) );
	    }
	  else if (offset > 0)
	    {
	      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst,
			    build_indirect( INDIRECT_PRE_ADD, src, offset ),
			    examines1( src ),
			    alters2( dst, RR_ST ) );
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst,
			    build_indirect( INDIRECT_PRE_ADD, src, offset + 1 ),
			    examines1( src ),
			    alters2( dst, RR_ST ) );
	    }
	  else
	    {
	      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst,
			    build_indirect( INDIRECT_PRE_SUB, src, -offset ),
			    examines1( src ),
			    alters2( dst, RR_ST ) );
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst,
			    build_indirect( INDIRECT_PRE_SUB, src, -offset - 1 ),
			    examines1( src ),
			    alters2( dst, RR_ST ) );
	    }
	}
      else
	{
	  int32	off;
	  bool	biased;
	  
	    
	  ar = peep_find_loaded_addr_reg( src, &off, &biased );

	  if (!no_peepholing &&
	      ar     != GAP  &&
	      biased == FALSE )
	    {
	      off = offset - off;

	      
	      if (off == 0)
		{
		  out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
				examines1( ar ),
				alters2( dst, RR_ST ) );
		}
	      else if (fits_in_8_bits_unsigned( off ))
		{
		  out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR, ar, off ),
				examines1( ar ),
				alters3( ar, dst, RR_ST ) );	      
		}
	      else if (fits_in_8_bits_unsigned( -off ))
		{
		  out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_DECR, ar, -off ),
				examines1( ar ),
				alters3( ar, dst, RR_ST ) );	      
		}
	      else if (fits_in_16_bits_signed( off ))
		{
		  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, ar, off,
				examines1( ar ),
				alters2( ar, RR_ST ) );

		  out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
				examines1( ar ),
				alters2( dst, RR_ST ) );
		}
	      else
		{
		  /* sigh, we have to do this by hand */
		  
		  load_integer( ar, offset, FALSE );

		  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, src,
				examines2( src, ar ),
				alters2( ar, RR_ST ) );
		  
		  out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
				examines1( ar ),
				alters2( dst, RR_ST ) );
		}
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR, ar, 1 ),
			    examines1( ar ),
			    alters3( ar, dst, RR_ST ) );		  

	      peep_note_addr_reg_loaded( ar, src, offset + 1, FALSE );
	    }
	  else
	    {
	      if (ar == GAP)
		{
		  ar = peep_get_free_addr_reg( GAP );
		}

	      load_integer( ar, offset, FALSE );

	      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, src,
			    examines2( src, ar ),
			    alters2( ar, RR_ST ) );
		  
	      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
			    examines1( ar ),
			    alters2( dst, RR_ST ) );
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR, ar, 1 ),
			    examines1( ar ),
			    alters3( dst, ar, RR_ST ) );		  

	      peep_note_addr_reg_loaded( ar, src, offset + 1, FALSE );
	      
	    }
	}
    }
  else if (can_corrupt_src &&
	   is_address_register( hardware_register( src )) &&
	   fits_in_16_bits_signed( offset / sizeof_int ))
    {
      /* divide by sizeof_int (here assumed to be 4) */
      
      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, src, -2,
		     examines1( src ),
		     alters2( src, RR_ST ) );

      offset /= sizeof_int;

      if (offset != 0)
	{
	  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, src, offset,
			examines1( src ),
			alters2( src, RR_ST ) );
	}

      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst,
		    build_indirect( INDIRECT_PRE_INCR_IR0, src, 0 ),
		    examines2( src, R_BASE ),
		    alters3( dst, src, RR_ST ) );
	      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst,
		    build_indirect( INDIRECT_PRE_INCR, src, 1 ),
		    examines1( src ),
		    alters3( dst, src, RR_ST ) );		  

      peep_corrupt_addr_reg( src );
    }
  else
    {
      int32	off;

      
      ar = peep_find_loaded_addr_reg( src, NULL, NULL );
	  
      if (ar == GAP)
	{
	  ar = peep_get_free_addr_reg( src );

	  if (ar == GAP)
	    {
	      peep_corrupt_addr_reg( R_ATMP );
	  
	      ar = R_ATMP;
	    }
	}

      if (convert_to_word_alignment( src, ar, offset, &off ))
	{
	  if (off > 0)
	    {
	      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR, ar, off ),
			    examines1( ar ),
			    alters3( ar, dst, RR_ST ) );
	    }
	  else if (off == 0)
	    {
	      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
			    examines1( ar ),
			    alters2( dst, RR_ST ) );
	    }
	  else
	    {
	      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_DECR, ar, -off ),
			    examines1( ar ),
			    alters3( ar, dst, RR_ST ) );
	    }
	}
      else
	{
	  out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR_IR0, ar, 0 ),
			examines2( R_BASE, ar ),
			alters3( ar, dst, RR_ST ) );
	}
      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR, ar, 1 ),
		    examines1( ar ),
		    alters3( ar, dst, RR_ST ) );
      
      peep_note_addr_reg_loaded( ar, src, offset / sizeof_int + 1, TRUE );
    }
  
  /* if dest is also in an address register, forget about it */
      
  peep_forget_about( dst );
  
  return;
  
} /* load_double_relative */

/*}}}*/
/*{{{  store_double_relative() */

/*
 * store a double precision value into the address in 'dst'
 * offset by 'offset'
 */

static void
store_double_relative(
		      RealRegister	val,			/* register containing the value	*/
		      RealRegister	addr,			/* register containing the address 	*/
		      int32		offset,			/* offset from the address in bytes	*/
		      int32		can_corrupt_addr )	/* non-zero if 'addr' may be courrupted	*/

{
  RealRegister	ar;


#ifdef DEBUG
  if (!is_float( addr ))
    syserr( gen_non_FP_source );
#endif
  
  /* NB/ STI must be AFTER the STF in memory */

  if (is_word_addressed_( addr ))
    {
      /*
       * the stack pointer and frame pointers are always
       * word pointers so no special adjustments should
       * be necessary
       */
      
      /* check to see if we about to access the stack and there are pushes pending */
      
      if (addr == R_SP)
	{
	  flush_pending_pushes();
	}
	  
      offset /= sizeof_int;

      if (fits_in_8_bits_unsigned( abs( offset + 1 ) ))
	{
	  if (offset == 0)
	    {
	      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_REL, addr, 0 ),
			    examines2( val, addr ),
			    alters0() );
	      
	      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_ADD, addr, 1 ),
			    examines2( val, addr ),
			    alters0() );
	    }
	  else if (offset > 0)
	    {
	      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_ADD, addr, offset ),
			    examines2( val, addr ),
			    alters0() );
	      
	      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_ADD, addr, ++offset ),
			    examines2( val, addr ),
			    alters0() );
	    }
	  else
	    {
	      offset = -offset;
	      
	      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_SUB, addr, offset ),
			    examines2( val, addr ),
			    alters0() );
	      
	      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_SUB, addr, --offset ),
			    examines2( val, addr ),
			    alters0() );
	    }
	}
      else
	{
	  int32	off;
	  bool	biased;
	  
	    
	  ar = peep_find_loaded_addr_reg( addr, &off, &biased );

	  if (!no_peepholing &&
	      ar     != GAP  &&
	      biased == FALSE )
	    {
	      off = offset - off;

	      
	      if (off == 0)
		{
		  out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_REL, ar, 0 ),
				examines2( val, ar ),
				alters0() );
		}
	      else if (fits_in_8_bits_unsigned( off ))
		{
		  out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR, ar, off ),
				examines2( val, ar ),
				alters1( ar ) );	      
		}
	      else if (fits_in_8_bits_unsigned( -off ))
		{
		  out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_DECR, ar, -off ),
				examines2( val, ar ),
				alters1( ar ) );	      
		}
	      else if (fits_in_16_bits_signed( off ))
		{
		  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, ar, off,
				examines1( ar ),
				alters2( ar, RR_ST ) );

		  out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_REL, ar, 0 ),
				examines2( val, ar ),
				alters0() );
		}
	      else
		{
		  /* sigh, we have to do this by hand */
		  
		  load_integer( ar, offset, FALSE );

		  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, addr,
				examines2( addr, ar ),
				alters2( ar, RR_ST ) );
		  
		  out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_REL, ar, 0 ),
				examines2( val, ar ),
				alters0() );
		}
	      
	      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR, ar, 1 ),
			    examines2( val, ar ),
			    alters1( ar ) );		  

	      peep_note_addr_reg_loaded( ar, addr, offset + 1, FALSE );
	    }
	  else
	    {
	      if (ar == GAP)
		{
		  ar = peep_get_free_addr_reg( GAP );
		}

	      load_integer( ar, offset, FALSE );

	      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, addr,
			    examines2( addr, ar ),
			    alters2( ar, RR_ST ) );
		  
	      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_REL, ar, 0 ),
			    examines2( val, ar ),
			    alters0() );
	      
	      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR, ar, 1 ),
			    examines2( val, ar ),
			    alters1( ar ) );		  

	      peep_note_addr_reg_loaded( ar, addr, offset + 1, FALSE );
	      
	    }
	}
    }
  else if (can_corrupt_addr &&
	   is_address_register( hardware_register( addr )) &&
	   fits_in_16_bits_signed( offset / sizeof_int ))
    {
      /* divide by sizeof_int (here assumed to be 4) */
      
      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, addr, -2,
		     examines1( addr ),
		     alters2( addr, RR_ST ) );

      offset /= sizeof_int;

      if (offset != 0)
	{
	  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, addr, offset,
			examines1( addr ),
			alters2( addr, RR_ST ) );
	}

      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR_IR0, addr, 0 ),
		    examines3( val, addr, R_BASE ),
		    alters1( addr ) );
	      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR, addr, 1 ),
		    examines2( val, addr ),
		    alters1( addr ) );		  

      peep_corrupt_addr_reg( addr );
    }
  else
    {
      int32	off;

      
      ar = peep_find_loaded_addr_reg( addr, NULL, NULL );
	  
      if (ar == GAP)
	{
	  ar = peep_get_free_addr_reg( addr );

	  if (ar == GAP)
	    {
	      peep_corrupt_addr_reg( R_ATMP );
	  
	      ar = R_ATMP;
	    }
	}

      if (convert_to_word_alignment( addr, ar, offset, &off ))
	{
	  if (off > 0)
	    {
	      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR, ar, off ),
			    examines2( val, ar ),
			    alters1( ar ) );
	    }
	  else if (off == 0)
	    {
	      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_REL, ar, 0 ),
			    examines2( val, ar ),
			    alters0() );
	    }
	  else
	    {
	      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_DECR, ar, -off ),
			    examines2( val, ar ),
			    alters1( ar ) );
	    }
	}
      else
	{
	  out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR_IR0, ar, 0 ),
			examines3( val, R_BASE, ar ),
			alters1( ar ) );
	}
      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR, ar, 1 ),
		    examines2( val, ar ),
		    alters1( ar ) );
      
      peep_note_addr_reg_loaded( ar, addr, offset / sizeof_int + 1, TRUE );
    }
  
  return;

} /* store_double_relative */

/*}}}*/
/*{{{  out_register_offset() */

/*
 * perform 'dst' = 'op' 'src','offset'
 *
 * ie using indirect addressing off 'src'
 */

static void
out_register_offset(
		    int32		op,		/* operation to perform 		*/
		    RealRegister	dst,		/* destintaion of operation 		*/
		    RealRegister	src,		/* address of value for operation 	*/
		    RealRegister	offset )	/* offset from source address 		*/
{
  bool		is_store = (op == OP_STI || op == OP_STF);
  RealRegister	ar;

  
  if (is_word_addressed_( src ))
    {
      int32	off;
      bool	biased;

      
      /* check to see if we about to access the stack and there are pushes pending */
      
      if (src == R_SP)
	{
	  flush_pending_pushes();
	}
	  
      /*
       * XXX
       *
       * we could save an instruction here if we could use IR1,
       * ie:
       *
       *	LSH3  offset, -2, IR1
       *	op    *+src(IR1), dst
       *
       * instead of:
       *	LSH3  offset, -2, R_ATMP
       *	ADDI  src,     R_ATMP
       *        op    *R_ATMP, dst
       */


      ar = peep_find_loaded_addr_reg( offset, &off, &biased );

      if (ar     != GAP  &&
	  off    == 0    &&
	  biased == FALSE )
	{
	  peepf( "eliminated register conversion" );

	  ++peep_eliminated;
	}
      else
	{
	  /* get an address register */
	  
	  ar = peep_get_free_addr_reg( GAP );
	  
	  /* copy offset to address register adjusting for word offsets */

	  (void) convert_to_word_alignment( offset, ar, 0, NULL );
	}
      
      /* add in the source register */
      
      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, src,
		    examines2( src, ar ),
		    alters2( ar, RR_ST ) );

      /* do not remember contents of 'ar' as we have just corrupted it */

      peep_corrupt_addr_reg( ar );
      
      /* and perform op */

      out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
		    is_store ? examines2( dst, ar ) : examines1( ar ),
		    is_store ? alters0()            : alters2( dst, RR_ST ) );
    }
  else
    {
      ar = peep_get_free_addr_reg( GAP );
      
      /* add offset to address and place in temporary register */

      out_triadic_op( OP_ADDI3, ar, offset, src, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( offset, src ),
		     alters2( ar, RR_ST ) );

      /* forget about contents of address reg */

      peep_corrupt_addr_reg( ar );
      
      /* loose bottom two bits (byte selector) */

      (void) convert_to_word_alignment( ar, ar, 0, NULL );

      /* complete operation */
      
      out_diadic_op( op, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_ADD_IR0, ar, 0 ),
		    is_store ? examines3( dst, ar, R_BASE ) : examines2( R_BASE, ar ),
		    is_store ? alters0()                    : alters2( dst, RR_ST ) );
    }
  
  if (op == OP_LDI || op == OP_LDF || op == OP_LDA)
    {
      peep_forget_about( dst );      
    }
  
  return;

} /* out_register_offset */

/*}}}*/
/*{{{  load_double_indirect() */

static void
load_double_indirect(
		     RealRegister	dst,		/* destintaion of operation */
		     RealRegister	src,		/* address of value for operation */
		     RealRegister	offset )	/* offset from source address */
{
  RealRegister	ar;


#ifdef DEBUG
  if (!is_float( dst ))
    syserr( gen_non_FP_dest );
#endif
  
  /* NB/ LDF before LDI */
  
  if (is_word_addressed_( src ))
    {
      int32	off;
      bool	biased;

      
      /* check to see if we about to access the stack and there are pushes pending */
      
      if (src == R_SP)
	{
	  flush_pending_pushes();
	}

      ar = peep_find_loaded_addr_reg( offset, &off, &biased );

      if (ar     != GAP  &&
	  off    == 0    &&
	  biased == FALSE )
	{
	  peepf( "eliminated register transfer" );

	  ++peep_eliminated;
	}
      else
	{
	  ar = peep_get_free_addr_reg( GAP );
	  
	  /* copy offset to temporary register adjusting for word offsets */

	  (void) convert_to_word_alignment( ar, offset, 0, NULL );
	}
      
      /* add in the source register */
      
      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, src,
		    examines2( src, ar ),
		    alters2( ar, RR_ST ) );

      /* forget about contents of 'ar' */

      peep_corrupt_addr_reg( ar );
      
      /* and perform load */

      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_REL, ar, 0 ),
		    examines1( ar ),
		    alters2( dst, RR_ST ) );
      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_ADD, ar, 1 ),
		    examines1( ar ),
		    alters2( dst, RR_ST ) );
    }
  else
    {
      ar = peep_get_free_addr_reg( GAP );
      
      /* add offset to address and place in temporary register */

      out_triadic_op( OP_ADDI3, ar, src, offset, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( offset, src ),
		     alters2( ar, RR_ST ) );

      /* forget about contents of 'ar' */

      peep_corrupt_addr_reg( ar );
      
      /* loose bottom two bits (byte selector) */

      (void) convert_to_word_alignment( ar, ar, 0, NULL );
      
      /* complete operation (NB/ LDF before LDI) */
  
      out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_INCR_IR0, ar, 0 ),
		    examines2( R_BASE, ar ),
		    alters3( dst, ar, RR_ST ) );
      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dst, build_indirect( INDIRECT_PRE_ADD, ar, 1 ),
		    examines1( ar ),
		    alters2( dst, RR_ST ) );
    }

  peep_forget_about( dst );
  
  return;

} /* load_double_indirect */

/*}}}*/
/*{{{  store_double_indirect() */

static void
store_double_indirect(
		     RealRegister	val,		/* source of value to store	*/
		     RealRegister	addr,		/* address into which to store	*/
		     RealRegister	offset )	/* offset from address		*/
{
  RealRegister	ar;


#ifdef DEBUG
  if (!is_float( val ))
    syserr( gen_non_FP_source );
#endif
  
  /* NB/ STI must be AFTER the STF in memory */
  
  if (is_word_addressed_( addr ))
    {
      int32	off;
      bool	biased;
      
      
      /* check to see if we about to access the stack and there are pushes pending */
      
      if (addr == R_SP)
	{
	  flush_pending_pushes();
	}

      ar = peep_find_loaded_addr_reg( offset, &off, &biased );

      if (ar     != GAP  &&
	  off    == 0    &&
	  biased == FALSE )
	{
	  peepf( "eliminated register load" );

	  ++peep_eliminated;
	}
      else
	{
	  ar = peep_get_free_addr_reg( GAP );
	  
	  /* copy offset to temporary register adjusting for word offsets */

	  (void) convert_to_word_alignment( ar, offset, 0, NULL );
	}
      
      /* add in the source register */
      
      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, addr,
		    examines2( addr, ar ),
		    alters2( ar, RR_ST ) );

      /* forget about contents of ar */

      peep_corrupt_addr_reg( ar );
      
      /* and perform store */
      
      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_REL, ar, 0 ),
		    examines2( val, ar ),
		    alters0() );

      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_ADD, ar, 1 ),
		    examines2( val, ar ),
		    alters0() );
    }
  else
    {
      ar = peep_get_free_addr_reg( GAP );
      
      /* add offset to address and place in temporary register */

      out_triadic_op( OP_ADDI3, ar, addr, offset, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( addr, offset ),
		     alters2( ar, RR_ST ) );

      /* forget about contents of 'ar' */

      peep_corrupt_addr_reg( ar );
      
      /* loose bottom two bits (byte selector) */

      (void) convert_to_word_alignment( ar, ar, 0, NULL );

      /* complete store */
  
      out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_INCR_IR0, ar, 0 ),
		    examines3( val, R_BASE, ar ),
		    alters1( ar ) );
      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, val, build_indirect( INDIRECT_PRE_ADD, ar, 1 ),
		    examines2( val, ar ),
		    alters0() );
    }
  
  return;

} /* store_double_indirect */

/*}}}*/

/*}}}*/
#ifdef TARGET_HAS_PROFILE
/*{{{  union count_position */

typedef union count_position
  {
    int32 i;

    struct s
      {
	unsigned int posn : 12,
                     line : 16,
                     file : 4;
      }
    s;
  }
count_position;

/*}}}*/
#endif /* TARGET_HAS_PROFILE */
/*{{{  correct_stack() */

/*
 * ensures that the stack is increased or decreased as necessary
 */

void
correct_stack( bool flush_pushes )
{
  int32	move;

  
  if (stack_move == 0)
    {
      return;
    }

  if (flush_pushes)
    {
      flush_pending_pushes();
    }

  if (stack_move & (sizeof_long - 1))
    syserr( gen_non_word_offset );

  move = stack_move / sizeof_long;

  /* NB/ do not use integer_immediate_op() as this might flush pushes */
  
  if (!fits_in_16_bits_signed( move ))
    {
      load_integer( R_TMP3, move, FALSE );
      
      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, R_SP, R_TMP3,
		    examines2( R_SP, R_TMP3 ),
		    alters2( R_SP, RR_ST ) );
      
      peep_change_addr_offset( R_SP, move );
    }  
  else if (move > 0)
    {
      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_SP, move,
		    examines1( R_SP ),
		    alters2( R_SP, RR_ST ) );
      
      peep_change_addr_offset( R_SP, move );
    }
  else
    {
      out_diadic_op( OP_SUBI, ADDR_MODE_IMMEDIATE, R_SP, -move,
		    examines1( R_SP ),
		    alters2( R_SP, RR_ST ) );
      
      peep_change_addr_offset( R_SP, move );
    }

  adjust_stack( -stack_move );
  
  return;
  
} /* correct_stack */

/*}}}*/
/*{{{  compare_integer() */

/*
 * Compare register 'r' with the integer 'n' using comparison 'test'
 */

static void
compare_integer(
		RealRegister	r,
		int32		n,
		int32 		test )
{
  if (test == Q_AL)
    {
      syserr( gen_void_compare );
    }
  
  correct_stack( TRUE );

  if (fits_in_16_bits_signed( n ))
    {
      if (n == 0 && !no_peepholing)
	{
	  /* check to see if the test is unnecessary */

	  if (peep_sets_status_reg( r ))
	    {
	      peepf( "eliminated compare!" );

	      ++peep_eliminated;
	      
	      return;
	    }
	}
  
      /* 16 bit signed immediate - can do */

      out_diadic_op( OP_CMPI, ADDR_MODE_IMMEDIATE, r, n,
		    examines1( r ),
		    alters1( RR_ST ) );
    }
  else
    {
      /* use the temporary register */
      
      load_integer( R_TMP1, n, FALSE );
      
      out_diadic_op( OP_CMPI, ADDR_MODE_REGISTER, r, R_TMP1,
		    examines2( r, R_TMP1 ),
		    alters1( RR_ST ) );
    }

  return;
  
} /* compare_integer */

/*}}}*/
/*{{{  shift_register() */

/*
 * perform a shift by 'abs( amount )' on 'source'
 * placing the result in 'dest'.  If 'amount' < 0
 * the shift is to the right otherwise it is to the
 * left.  If 'arithmetic' is true the a right shift
 * should preserve the sign bit
 */
 
static void
shift_register(
	       bool		arithmetic,	/* true for arithmetic shift, false otherwise */
	       RealRegister	dest,
	       RealRegister	source,
	       int32		amount )
{
  /*
   * if amount < 0 then shift right otherwise shift left
   */

  if (amount == 0)
    {
      move_register( source, dest, FALSE );

      return;
    }
  else if (amount >= 32)
    {
      /* nothing left in dest but 0's */

      cc_rerr( syserr_large_shift, amount );

      /* still do the shift as we may want the last bit left in the carry flag */
      
      amount = 32;
    }
  else if (amount <= -32)
    {
      cc_rerr( syserr_large_shift, amount );

      amount = -32;
    }

  /*
   * at this point we know that |amount| <= 32 and so fits in 8 bits
   */
  
  if (source == dest)
    {
      out_diadic_op( arithmetic ? OP_ASH : OP_LSH,
		     ADDR_MODE_IMMEDIATE, dest, amount,
		     examines1( dest ),
		     alters2( dest, RR_ST ) );
    }
  else
    {
      out_triadic_op( arithmetic ? OP_ASH3 : OP_LSH3,
		     dest, source, amount, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
		     examines1( source ),
		     alters2( dest, RR_ST ) );
    }
  
  peep_forget_about( dest );
  
  return;
  
} /* shift_register */

/*}}}*/
/*{{{  multiply_integer() */

/*
 * register 'dest' = register 'source' * integer 'val'
 */

static void
multiply_integer(
		 RealRegister	dest,
		 RealRegister	source,
		 int32		val )
{
  /*
   * the following special cases are detected in cg.c (with the exception of
   * bitcount(-m) = 1, m != -1), but are included for completeness and because
   * there is a possibility they may be introduced by attempting to factorise
   * and invoke mulk recursively.
   */

  if (val == 0)
    {
      load_integer( dest, 0, FALSE );
    }
  else if (val == 1)
    {
      move_register( source, dest, FALSE );
    }      
  else if (val == -1)
    {
      out_diadic_op( OP_NEGI, ADDR_MODE_REGISTER, dest, source,
		    examines1( source ),
		    alters2( dest, RR_ST ) );
    }
  else if (bitcount( val ) == 1)
    {
      shift_register( FALSE, dest, source, firstbit( val ) );
    }
  else if (fits_in_16_bits_signed( val ))
    {
      if (fits_in_8_bits_signed( val ))
	{
	  out_triadic_op( OP_MPYI3,
			 dest,
			 source,
			 val,
			 ADDR_MODE_REGISTER,
			 ADDR_MODE_IMMEDIATE,
			 examines1( source ),
			 alters2( dest, RR_ST ) );
	}
      else if (dest == source)
	{
	  out_diadic_op( OP_MPYI, ADDR_MODE_IMMEDIATE, dest, val,
			examines1( dest ),
			alters2( dest, RR_ST ) );
	}
      else
	{
	  load_integer( dest, val, FALSE );
	  
	  out_triadic_op( OP_MPYI3, dest, source, dest, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			examines2( source, dest ),
			alters2( dest, RR_ST ) );
	}
    }
  else if (bitcount( -val ) == 1)
    {
      out_diadic_op( OP_NEGI, ADDR_MODE_REGISTER, dest, source,
		    examines1( source ),
		    alters2( dest, RR_ST ) );
      
      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, dest, firstbit( -val ),
		    examines1( dest ),
		    alters2( dest, RR_ST ) );
    }
  else
    {
      load_integer( R_TMP1, val, FALSE );

      out_triadic_op( OP_MPYI3, 
		     dest,
		     source,
		     R_TMP1,
		     ADDR_MODE_REGISTER,
		     ADDR_MODE_REGISTER,
		     examines1( source ),
		     alters2( dest, RR_ST ) );
    }
  
  peep_forget_about( dest );
  
  return;

} /* mutiply_integer */

/*}}}*/
/*{{{  conditional_skip_instructions() */

/*
 * generates a branch by a given number
 * of instructions if a given condition is true
 */

static void
conditional_skip_instructions(
			      int32	condition,	/* condition - TI version */
			      int32	num_to_skip )	/* number of instructions to miss out */
{
  if (!fits_in_16_bits_signed( num_to_skip ))
    {
      syserr( gen_too_many_to_skip, num_to_skip );

      return;
    }
  
  /* output a conditional branch (delayed), PC relative */
  
  outdelayed( (OP_BRcD << 24) | (0x1U << 21) | (condition << 16) | (num_to_skip & 0xffff),
	     condition == C_U ? examines0() : examines1( RR_ST ),
	     alters1( RR_PC ) );
  
  return;
  
} /* conditional_skip_instructions */

/*}}}*/
/*{{{  conditional_branch_to() */

/*
 * This function handles the case of branching
 * to a label, possibly conditionally
 */

static void
conditional_branch_to(
		      int32		condition,
		      LabelNumber *	destination )
{
  /* don't keep pushes pending */
  
  flush_pending_pushes();

  if (destination == RETLAB)
    {
      /* AM: treat the special value RETLAB of destination as return address */

      destination = returnlab;

      if (condition == Q_AL)
	{
	  /* if get an unconditional return expand it inline      */
	  /* and save its address if it was the first             */
	  /* - but ACN suggests we may get better code by not     */
	  /* doing so, waiting instead until we are at ENDPROC    */
	  
	  /* if (!lab_isset_(destination)) setlabel(destination); */
	    
	  routine_exit( FALSE );
	  
	  return;
	}
      else if (!no_peepholing    &&	/* if we are allowed to peephole */
	       stack_move   == 0 &&	/* and the stack is OK */
	       stack_offset == 0 &&	/* totally OK */
	       saved_fvars  == 0 &&	/* and we have no register to pop */
	       saved_ivars  == 0 &&	/* ditto */
	       saved_frame  == 0 &&	/* ditto */
	       saved_args   == 0 &&	/* ditto */
	       !usrdbg( DBG_PROC ) )	/* and we do not want an exit routine called */
	{
	  peepf( "eliminated branch to return jump" );
	  
	  /* this is a conditional, delayed branch, register relative */
      
	  outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( condition ) << 16 | hardware_register( R_LR ),
		     examines2( R_LR, RR_ST ),
		     alters1( RR_PC ) );

	  return;
	}
    }

  /*
   * If the label we are branching to has
   * already been defined then we can
   * calculate the offset straight away
   */

  if (lab_isset_( destination ))
    {
      int32	w;
      int32	off;


      flush_peepholer( DBG( "backward conditional branch" ) );
  
      w   = (destination->u.defn) & 0x7fffffffU;	/* get position of label in our code */
      off = (w - codep) / sizeof_int - 3;		/* calculate offset (in words) */
      							/* (the -3 is because of the delay in the branch)  */
      
      if (condition == Q_AL && fits_in_24_bits_signed( off ))
	{
	  /* branch (delayed), PC relative */

	  outdellabref( OP_BRD << 24 | (off & 0xffffff),
		       destination,
		       LABREF_OFF24,
		       examines0(),
		       alters1( RR_PC ) );
	}
      else if (fits_in_16_bits_signed( off ))		/* short branch */
        {
	  conditional_skip_instructions( C_FROMQ( condition ), off );
        }
      else					/* word branch */
        {
	  syserr( gen_cannot_branch, off );
        }
    }
  else    /* Forward reference */
    {
      /*
       * put in a branch instruction with a fake offset as a placeholder
       * this will be patched with the correct offset when setlabel() is called
       * to actually position our destination label
       */

      if (condition == Q_AL)
	{
	  outdellabref( OP_BRD << 24 | (-3 & 0xffffff), destination, LABREF_OFF24,
		     examines0(), alters1( RR_PC ) );
	}
      else
        {
	  /* output a conditional branch (delayed), PC relative */
  
	  outdellabref( OP_BRcD << 24 | 0x1U << 21 | C_FROMQ( condition ) << 16 | (-3 & 0xffff),
		    destination,
		    LABREF_OFF16,
		    condition == Q_AL ? examines0() : examines1( RR_ST ),
		    alters1( RR_PC ) );
        }
    }

  return;
  
} /* conditional_branch_to */

/*}}}*/
/*{{{  convert_to_byte_offset() */

/*
 * convert the absolute word address in 'reg'
 * into a byte offset from IR0
 */

static void
convert_to_byte_offset( RealRegister reg )
{
  out_diadic_op( OP_SUBI, ADDR_MODE_REGISTER, reg, R_BASE,
		examines2( reg, R_BASE ),
		alters2( reg, RR_ST ) );

  out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, reg, 2,
		 examines1( reg ),
		 alters2( reg, RR_ST ) );

 return;
  
} /* convert_to_byte_offset */

/*}}}*/
/*{{{  convert_to_word_address() */

  
/*
 * convert the byte offset in 'src'
 * into an absolute word address in 'dst'
 */

static void
convert_to_word_address(
			RealRegister src,	/* register to convert to word alignment	*/
			RealRegister dst )	/* register to hold result of convertion	*/
{
  int32	off;

  
  if (convert_to_word_alignment( src, dst, 0, &off ))
    {
      if (off != 0)
	integer_immediate_op( OP_ADDI, OP_ADDI3, dst, dst, off, TRUE );
    }
  else
    {
      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, dst, R_BASE,
		    examines2( dst, R_BASE ),
		    alters2( dst, RR_ST ) );

      peep_note_addr_reg_loaded( dst, src, 0, TRUE );
    }

 return;
  
} /* convert_to_word_address */

/*}}}*/
/*{{{  find_nearest_symbol() */

/*
 * find a symbol that is as close as possible
 * to the given symbol, including the offset
 * provided.
 */

static Symstr *
find_nearest_symbol(
		    Symstr * 	symbol,
		    unsigned32	offset )
{
  ExtRef *	r;
  ExtRef *	least;
  unsigned long	least_diff;
  
  
  offset += symext_( symbol )->extoffset;
  
  r = obj_symlist; 

  least      = NULL;
  least_diff = -1;

  /* debug( "searching for nearest match to %s, offset %d", symname_( symbol ), offset ); */
  
  for (; r != NULL; r = r->extcdr)
    {
      if (is_data( r ) && is_local( r ))
	{
	  unsigned long	diff = abs( r->extoffset - offset );


	  /* debug( "checking symbol %s, offset %d, diff %d", symname_( r->extsym ), r->extoffset, diff ); */
	  
	  if (diff <= least_diff)
	    {
	      least_diff = diff;
	      least      = r;
	    }	  
	}
    }

  /* debug( "result is %s", least == NULL ? "nothing" : symname_( least->extsym ) ); */
  
  return least == NULL ? NULL : least->extsym ;

} /* find_nearest_symbol */

/*}}}*/
/*{{{  static_definition() */

/*
 * returns non-0 if the symbol is static
 * returns 0 otherwise
 */

static int32
static_definition( Symstr * name )
{
  if (symdata_( name ) != 0)
    return (bindstg_( symdata_( name ) ) & bitofstg_( s_static ) );
  else
    return 0;

} /* static_definition */

/*}}}*/
/*{{{  load_static_data_pointer() */

/*
 * XXX
 *
 * Module Layout
 *
 * version: 	7	(keep in step with stack scheme and register scheme)
 * date:	29/2/92
 *
 *
 *                                MT
 *                                |
 *                        (word pointer to)
 *                                |
 *                                v
 *                             ------------------------------ - - -
 *         The (split)      ->|       .      |      .      |
 *         Module Table    |  |   |   .    | |  |   .   |  |
 *                         |   ---|--------|----|-------|---- - - -
 *                         |______|     ^  |    |       |
 *                                      |  |    |       |
 *                                       --     |       |
 *                                              |       |
 *                                 _____________|       |____________________
 *                                |                                          |
 *                                |                                          |
 *                        (byte offset from IR0 of ...) (*)          (word pointer to)
 *                                |                                          |
 *                                v                                          |
 *      Per Module             ------------------------------ - - -          |
 *      Data Slots            |       |      |      |      |                 |
 * (one copy per process)     |       |      |      |      |                 |
 *                             ------------------------------ - - -          |
 *                                                                           |
 *                                                                           |
 *                                 __________________________________________|
 *                                |
 *                                |
 *                                v                                 
 *        Per Module           ------------------------------ - - - 
 *     Function Pointers      |       |      |      |      |        
 * (shared between processes) |   |   |      |      |      |        
 *                             ---|-------------------------- - - - 
 *                                |
 *                        (word address of first
 *                         function in module)
 *                                |
 *                                v
 *
 * (*) The data slot pointer is held as a byte offset, rather than a word pointer, because
 * all data pointers must be held as byte offsets, and data is fetched by taking the data
 * slot pointer from the module table and just adding a known byte offset to obtain a pointer
 * to the required piece of data.
 * 
 */


/*
 * Places a pointer to the static data area or the
 * function pointer table into the indicated register.
 *
 * If 'iscode' is true then the pointer returned is
 *   an absolute, word address,
 * whereas if 'iscode' is false then the pointer returned is
 *   a byte offset from IR0.
 */

void
load_static_data_ptr(
		     RealRegister	r,
		     bool		iscode,
		     Symstr *		symbol )
{
  RealRegister	tmp;
  int32		op;

  
  if (is_address_register( hardware_register( r ) ))
    {
      tmp = r;
    }
  else
    {
      tmp = peep_get_free_addr_reg( GAP );
    }

  if (is_special_register( hardware_register( r ) ))
    {
      op = OP_LDA;
    }
  else
    {
      op = OP_LDI;
    }
      
  if (!few_modules)
    {
      /*
       * XXX - we are assuming that the module number fits into 16 bits (signed)
       * (ie that there will never be more that 32766 modules) ...
       */

      /*
       * load the module table address into temporary address register
       */

      move_register( R_MT, tmp, FALSE );
      
      /*
       * add in the offset of our module's entry in the module table
       */
  
      /*
       * indicate that the following instruction loads the module
       * number of the current module as the least 16 significant bits
       * of the instruction.  This number will actually have to be
       * inserted at link time (rather than now), so we use the cross
       * reference facility to ensure that the linker will patch
       * this instruction.  This instruction loads the WORD offset
       * from the module table pointer.
       */
  
      peep_xref( X_Modnum, symbol );
  
      if (iscode && split_module_table)
	{
	  /*
	   * if we are accessing the function table of a split
	   * module table then we actually want the next word on
	   */
      
	  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, tmp, 1,
			examines0(),
			alters2( tmp, RR_ST ) );
	}
      else
	{
	  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, tmp, 0,
			examines0(),
			alters2( tmp, RR_ST ) );
	}

      /*
       * get the appropriate pointer out of the module table
       */
  
      out_diadic_op( op, ADDR_MODE_INDIRECT, r, build_indirect( INDIRECT_REL, tmp, 0 ),
		    examines1( tmp ),
		    alters2( r, RR_ST ) );
    }
  else
    {
      /*
       * XXX - we are assuming that the module number fits into 8 bits (unsigned)
       * (ie that there will never be more that 255 modules) ...
       */

      peep_xref( X_Modnum, symbol );
  
      if (iscode && split_module_table)
	{
	  /*
	   * if we are accessing the function table of a split
	   * module table then we actually want the next word on
	   */
      
	  out_diadic_op( op, ADDR_MODE_INDIRECT, r,
			build_indirect( INDIRECT_PRE_ADD, R_MT, 1 ),
			examines1( R_MT ),
			alters2( r, RR_ST ) );
	}
      else
	{
	  out_diadic_op( op, ADDR_MODE_INDIRECT, r,
			build_indirect( INDIRECT_PRE_ADD, R_MT, 0 ),
			examines1( R_MT ),
			alters2( r, RR_ST ) );
	}
    }
  
  /* 'r' now contains the address or offset of the appropriate table */
  
  return;

} /* load_static_data_ptr */

/*}}}*/
/*{{{  is_function() */

bool
is_function( Symstr * symbol )
{
  struct Binder *	data;


  /* be paranoid */

#ifdef DEBUG
  if (symbol == NULL)
    syserr( gen_NULL_param );
#endif
  
  /* catch a rare special case */
  
  if (symtype_( symbol ) != s_identifier)
    syserr( gen_not_identifier );
  
   /* __dataseg has no data associated with it */
  
  if (symbol == bindsym_( datasegment ))
    return FALSE;
  
  /*
   * We default to FALSE because even though functions like
   * '__stackoverflow' & '_printf' have no data this function should
   * never be called for them, and local external data will have
   * no data.  ie
   *
   * int func ( void )
   * {
   *    extern int fred;
   *    return fred;
   * }
   *
   * Will call this function with a symbol 'fred' with no data.
   *
   */

  if ((data = symdata_( symbol )) == NULL)
    {
      struct ExtRef *	ref;

      
      if ((ref = symext_( symbol )) != NULL)
	{
	  /* cope with external symbols */
	  
	  if (is_data( ref ))
	    {
	      return FALSE;
	    }
	  else
	    {
	      return TRUE;
	    }
	}
      else
	{
	  return FALSE;
	}
    }

  return ((bindstg_( data ) & b_fnconst) != 0 );

} /* is_function */

/*}}}*/
/*{{{  load_address_constant() */

/*
 * place the address of 'symbol',
 * offset by 'offset'
 * into register 'dest'
 */

static void
load_address_constant(
		      RealRegister	dest,
		      Symstr *		symbol,
		      int32		offset )
{
  ExtRef *		x = symext_( symbol );
  int32 		d = obj_symref( symbol, xr_code, 0 );


  if (dest == R_SP)
    {
      syserr( gen_not_supported );
    }
  /*
   * find out what kind of symbol we are dealing with
   */
  
  if (is_data( x ) && is_defined( x ))
    {
      /*
       * A previously defined data symbol
       *
       * address = module table[ our module number ].data_slots + offset_of_symbol_in_data_slots
       */

      if (suppress_module || !fits_in_16_bits_signed( offset + x->extoffset ))
	{
	  Symstr *	newsymbol;

	  
	  /*
	   * try to eliminate references to __dataseg or offsets that do not fit in 16 bits
	   */
	      
	  if ((newsymbol = find_nearest_symbol( symbol, offset )) != NULL)
	    {
	      offset += symext_( symbol )->extoffset - symext_( newsymbol )->extoffset;		  
	      symbol = newsymbol;
	      x      = symext_( symbol );
	    }
        }

      if (suppress_module || !fits_in_16_bits_signed( offset + x->extoffset ))
	{
	  /*
	   * get the address of the data slots of the current module into 'tmp'
	   */
      
	  load_static_data_ptr( dest, FALSE, symbol );

	  if (much_data)
	    {
	      /*
	       * load R_TMP1 with the high part offset of 'symbol' into the data table
	       */
	      
	      peep_xref( X_DataSymbHi, symbol );
	      
	      out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, R_TMP1, 0,
			    examines0(),
			    alters1( R_TMP1 ) );
	      
	      /*
	       * load R_TMP1 with the low part offset of 'symbol' into the data table
	       *
	       * Note that we cannot include 'offset' in this part of the addition because
	       * the sum of 'offset' and the low part of 'symbol's offset might exceeed 16
	       * bits
	       */
	      
	      peep_xref( X_DataSymbLo, symbol );
	      
	      out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, R_TMP1, 0,
			    examines1( R_TMP1 ),
			    alters1( R_TMP1 ) );	/* XXX - do not mention setting ST flag as ADDI does that */
	      
	      /*
	       * and add R_TMP1 into dest
	       */
	      
	      if (offset)
		{
		  integer_immediate_op( OP_ADDI, OP_ADDI3, R_TMP1, R_TMP1, offset, TRUE );
		  
		  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, dest, R_TMP1,
				examines2( dest, R_TMP1 ),
				alters2( dest, RR_ST ) );
		}
	      else
		{
		  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, dest, R_TMP1,
				examines2( dest, R_TMP1 ),
				alters2( dest, RR_ST ) );
		}
	    }
	  else
	    {
	      peep_xref( X_DataSymbLo, symbol );

	      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, dest, 0,
			    examines1( dest ),
			    alters2( dest, RR_ST ) );
	  
	      if (offset)
		{
		  integer_immediate_op( OP_ADDI, OP_ADDI3, dest, dest, offset, TRUE );
		}
	    }
	}
      else
	{
	  load_static_data_ptr( dest, FALSE, symbol );

	  /*
	   * do not bother to ask the linker for the offset of symbol,
	   * as we already know it
	   */
		  
	  offset += x->extoffset;
	  
	  if (offset)
	    {
	      integer_immediate_op( OP_ADDI, OP_ADDI3, dest, dest, offset, TRUE );
	    }
	}      
    }
  else if (d != -1 && !in_stubs)
    {
      /*
       * An already defined internal code symbol
       *
       * This is called when we want to take the address of an already
       * defined function, eg:
       *
       *   int munge( int (*f)() ) { return (*f)(); }
       *
       *   int func1( void ) { return 3; }
       *
       *   int main( void ) { return munge( func1 ); }
       *
       */

      /* be paraniod */
      
      if (!is_function( symbol ))
	syserr( gen_address_of_non_function );

      if ((dest != R_LR)                        &&	 /* register being corrupted is not linker register */
	  (((saved_ivars & regbit( R_LR )) == 0) ||	 /* link register has not been saved on the stack */
	   usedmaskvec.map[ 0 ] & regbit( R_LR )) )	 /* link register being used as temporary */
	   
	{
	  /* save link register */

	  if (dest == R_TMP1)
	    {
	      move_register( R_LR, R_TMP2, FALSE );
	    }
	  else
	    {
	      move_register( R_LR, R_TMP1, FALSE );
	    }
	}

      flush_peepholer( DBG( "about to emit a LAJ" ) );
      
      /* put PC of next instruction into R_LR */

      outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

      flush_peepholer( DBG( "getting address of LAJ (2)" ) );

      d = codep + codebase - d + (3 * sizeof_int);

      /* protect the next three instructions from future BRs and LAJs */
      
      peep_protect_pc = 3;
      
      /*
       * XXX - the following NOP is because of a bug in the C40
       * silicon, whereby R_LR is occaisionally not set by the
       * LAJ until the second instruction after the LAJ
       */

      nop( FALSE );

      /*
       * XXX - beware special knowledge of the position of this NOP is
       * is used in asm.c: decode_diadic_address()
       */

      /* prevent NOP from being peepholed */
      
      flush_peepholer( DBG( "LAJ bug" ) );
      
      /* retreive PC from R_LR */

      move_register( R_LR, dest, FALSE );
      
      /* compute compound offset */

      if (d & 3)
	syserr( gen_ptr_not_aligned );

      /* adjust symbol offset to be relative to next instruction */

      d += offset;

      if (d & 3)
	syserr( gen_ptr_not_aligned );

      /* subtract offset (in words) */

      d /= sizeof_int;
      
      if (d)
	{
	  /* indicate that the next instruction will reference 'symbol' */
  
	  peep_symref( symbol );
	  
	  /*
	   * XXX - beware special knowledge of the position of this instruction is
	   * is used in asm.c: decode_diadic_address()
	   */

	  integer_immediate_op( OP_SUBI, OP_SUBI3, dest, dest, d, TRUE );
	}
      else
	{
	  nop( FALSE );
	}

      if ((dest != R_LR) &&
	  (((saved_ivars & regbit( R_LR )) == 0) ||
	   usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */	   
	{
	  /* restore saved link register */
      
	  if (dest == R_TMP1)
	    {
	      move_register( R_TMP2, R_LR, FALSE );
	    }
	  else
	    {
	      move_register( R_TMP1, R_LR, FALSE );
	    }
	}

      /* finished */
    }
  else if (static_definition( symbol ))
    {
      /*
       * A not yet defined internal code symbol
       *
       * word address = PC of current position in code +
       *               offset of symbol from current position in code
       *
       * This is caused by code such as...
       * 
       *  static int func( void );
       *
       *  int
       *  main( void )
       *  {
       *    int (* fn )();
       *
       *
       *    fn = func;
       *
       *    return (*fn)();
       *  }
       *
       *  static int func( void ) { return 1; }
       *
       */

      if (offset != 0)
	syserr( gen_offset_from_ptr );

      if ((dest != R_LR) &&
	  (((saved_ivars & regbit( R_LR )) == 0) ||
	   usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */
	{
	  /* save link register */

	  move_register( R_LR, R_TMP1, FALSE );
	}
      
      /* put PC of next instruction into R_LR */

      outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

      peep_protect_pc = 3;
      
      /*
       * The value loaded here should be
       * 'top 16 bits of offset of symbol from (this instruction + 3)'
       * The +3 is taken of by the patch generated in heliobj.c
       */
      
      peep_xref( X_DataAddr1, symbol ); 

      out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, R_TMP2, 0,
		    examines0(),
		    alters2( R_TMP2, RR_ST ) );

      peep_xref( X_DataAddr, symbol ); 
      
      out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, R_TMP2, -4 + 2,
		    examines1( R_TMP2 ),
		    alters1( R_TMP2 ) );	/* XXX - note lie about not affecting RR_ST */

      flush_peepholer( DBG( "symbol address" ) );
      
      /*
       * now add the two offsets place in destination register
       *
       * (This instruction does not need to be patched)
       */
      
      out_triadic_op( OP_ADDI3, dest, R_LR, R_TMP2, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( R_LR, R_TMP2 ),
		     alters2( dest, RR_ST ) );
      
      if ((dest != R_LR) &&
	  (((saved_ivars & regbit( R_LR )) == 0) ||
	   usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */
	{
	  /* restore saved link register */
      
	  move_register( R_TMP1, R_LR, FALSE );
	}
    }
  else if (/* is_function( symbol ) && */ !new_stubs && in_stubs)
    {
      RealRegister	real_dest = dest;
      bool		must_copy = FALSE;
      
      
      /*
       * Calling stub
       *
       *    address = contents of
       *              module table[ symbol's module number ].function table +
       *              offset of symbol in function table
       *
       * Note that we have special case code here, (instead of using the more
       * general function address loading code below), because we want to reduce
       * the function call overhead as much as possible, and that means having
       * efficient calling stubs.
       */

      if (offset != 0)
	{
	  syserr( gen_offset_from_ptr );
	}
      
      if (dest != RR_AR5)
	{
	  real_dest = dest;
	  dest      = RR_AR5;
	  must_copy = TRUE;
	}
      else
	{
	  must_copy = FALSE;
	}

      /*
       * XXX
       *
       * In an ideal world we could be sure of having less than 256
       * modules in a program, and less than 256 exported functions
       * per module.  If this were true then out calling stubs
       * could be :-
       *
       *   LDI   *+R_MT( modnum ), R_ATMP	- get the function table pointer into temporary address register
       *   LDI   *+R_MT( offset ), R_ATMP	- get the function pointer into temp reg
       *   Bu    R_ATMP				- jump to function
       *
       * Unhappily, we cannot guarantee these things, and so our
       * calling stubs may have to be :-
       *
       *   LDI   R_MT,    R_ATMP		- get module table pointer into temporary address register
       *   ADDI  modnum,  R_ATMP		- add in the module number
       *   LDI   *R_ATMP, R_ATMP		- get the function table pointer into temp reg
       *   ADDI  offset,  R_ATMP		- add in the word offset of the function into the table
       *   LDI   *R_ATMP, R_ATMP		- get the function pointer
       *   Bu    R_ATMP				- and jump
       *
       * NB/ IF we know that the module table is at a fixed offset from IR0 then we could do :-
       *
       *   LDI   modnum + offset_of_MT, R_ATMP	- get offset of module table plus offset of module table pointer
       *   LDI   *+R_ATMP(IR0),         R_ATMP	- get the function table pointer into temp reg
       *   ADDI  offset,                R_ATMP	- add in the word offset of the function into the table
       *   LDI   *R_ATMP,               R_ATMP	- get the function pointer
       *   Bu    R_ATMP				- and jump
       *
       * The bad news is that we can only make the decision about
       * the size of the offsets we are going to have to add in
       * at link time, not a compile time.  Hence we are going to
       * assume the pessimistic case, but we will allow the linker
       * to change the instructions around, (replacing redundant
       * instructions with NOPs ?)
       */

      /*
       * get the module table pointer into 'dest'
       *
       * this instruction could be patched if the module
       * number of 'symbol' is less than 256
       */

      peep_xref( X_DataModule1, symbol );

      move_register( R_MT, dest, FALSE );

      /*
       * add in the offset of the module containing 'symbol'
       *
       * XXX - we asssume that there will never be more than 32767 modules
       */

      peep_xref( X_DataModule2, symbol );

      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, dest, split_module_table ? 1 : 0,
		    examines1( dest ),
		    alters2( dest, RR_ST ) );
      
      /*
       * get the address of the module's function table
       */
 
      peep_xref( X_DataModule3, symbol );

      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dest, build_indirect( INDIRECT_REL, dest, 0 ),
		    examines1( dest ),
		    alters2( dest, RR_ST ) );

      /*
       * add in the offset of 'symbol' into the function table
       *
       * XXX - we are assuming that there will never be more than 65535 exported functions per module
       */

      peep_xref( X_DataModule4, symbol );

      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, dest, 0,
		    examines1( dest ),
		    alters2( dest, RR_ST ) );

      /*
       * get the address of the function
       */

      peep_xref( X_DataModule5, symbol );
      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dest, build_indirect( INDIRECT_REL, dest, 0 ),
		    examines1( dest ),
		    alters2( dest, RR_ST ) );

      if (must_copy)
	{
	  move_register( dest, real_dest, FALSE );
	}
    }
  else
    {
      bool		is_func = is_function( symbol );      
      RealRegister	tmp;


      /*
       * All we have left are external data and function symbols
       *
       *    address = module table[ symbol's module number ].data slots +
       *              offset of symbol in data slots
       * or
       *    address = module table[ symbol's module number ].function table +
       *              offset of symbol in function table       
       */

      /*
       * In an ideal world we could do :-
       *
       *   LDI  *+R_MT( modnum ), dest
       *   ADDI byte_offset,      dest
       *
       * but in the worst case we have :-
       *
       *   LDI  R_MT,   dest
       *   ADDI modnum, dest
       *   LDI  *dest,  dest
       *   LDHI byte_offset >> 16, R_TMP1
       *   OR   byte_offset,       R_TMP1
       *   ADDI R_TMP1, dest
       *
       * NB/ If we know that the module table is at a fixed offset from IR0 then we could do :-
       *
       *   LDI  modnum + offset_of_MT, dest
       *   LDI  *+dest(IR0),           dest
       *   LDHI byte_offset >> 16,     R_TMP1
       *   OR   byte_offset,           R_TMP1
       *   ADDI R_TMP1,                dest
       *
       * Unfortunately since this code can occur in the middle of a function body,
       * we gain nothing by optimising down to the best case.
       */

      if (suppress_module && is_func)
	{
	  /*
	   * XXX - urg!
	   *
	   * If we are compiling a resident library and we have a reference
	   * the address of a function, then we must treat this as a reference
	   * to a static function, EVEN IF the function's prototype has
	   * suggested that it is automatic or extern.  This is because all
	   * functions in a resident library are by default static, (rather than
	   * auto as for normal C programs), but the front end does not know
	   * this.
	   *
	   * As a consequence of this, if a function really is external to the
	   * library then this code will actually load the address of the function
	   * stub, rather than the address in the functions module table entry,
	   * and hence comparisions between function pointers will not work.
	   *
	   * Anyway, hence the following code.
	   */

	  if (feature & FEATURE_FUSSY)
	    cc_warn( "Comparing the addresses of an external functions may not work with -Zl or -Zr" );

	  if ((dest != R_LR) &&
	      (((saved_ivars & regbit( R_LR )) == 0) ||
	       usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */	      
	    {
	      /* save link register */

	      move_register( R_LR, R_TMP1, FALSE );
	    }
      
	  /* put PC of next instruction into R_LR */

	  outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

	  peep_protect_pc = 3;
	  
	  /*
	   * The value loaded here should be
	   * 'top 16 bits of offset of symbol from (this instruction + 3)'
	   * The +3 is taken of by the patch generated in heliobj.c
	   */
      
	  peep_xref( X_DataAddr1, symbol );
	  
	  out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, R_TMP2, 0,
			examines0(),
			alters2( RR_ST, R_TMP2 ) );

	  peep_xref( X_DataAddr, symbol );
	  
	  out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, R_TMP2, -4 + 2,
			examines1( R_TMP2 ),
			alters1( R_TMP2 ) );	/* XXX - note lie about not affecting RR_ST */

	  flush_peepholer( DBG( "symbol address 2" ) );
	  
	  /*
	   * now add the two offsets place in destination register
	   *
	   * (This instruction does not need to be patched)
	   */
      
	  out_triadic_op( OP_ADDI3, dest, R_LR, R_TMP2, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			 examines2( R_LR, R_TMP2 ),
			 alters2( dest, RR_ST ) );
      
	  if ((dest != R_LR) &&
	      (((saved_ivars & regbit( R_LR )) == 0) ||
	       usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */
	    {
	      /* restore saved link register */
	  
	      move_register( R_TMP1, R_LR, FALSE );
	    }
	}
      else
	{
	  /* we need an address register to load the pointer */
      
	  if (is_address_register( hardware_register( dest ) ))
	    {
	      tmp = dest;
	    }
	  else
	    {
	      tmp = peep_get_free_addr_reg( GAP );
	    }

	  if (is_func && new_stubs)
	    {
	      request_new_stub( symbol );

	      /* do nothing here, do something in next if statement ... */
	      ;	      
	    }	  
	  else if (!few_modules)
	    {
	      /*
	       * get address of module table into 'tmp'
	       */

	      move_register( R_MT, tmp, FALSE );
	  
	      /*
	       * add in the offset of the module containing 'name'
	       *
	       * XXX - we asssume that there will never be more than 32767 modules
	       */
	  
	      peep_xref( X_DataModule, symbol );
	      
	      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, tmp, 0,
			    examines1( tmp ),
			    alters2( tmp, RR_ST ) );
	  
	      /*
	       * get the address of the module's function table or data slots
	       */
	  
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, is_func ? tmp : dest,
			    build_indirect( INDIRECT_REL, tmp, 0 ),
			    examines1( tmp ),
			    alters2( is_func ? tmp : dest, RR_ST ) );
	    }
	  else
	    {
	      peep_xref( X_DataModule, symbol );
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, is_func ? tmp : dest,
			    build_indirect( INDIRECT_PRE_ADD, R_MT, 0 ),
			    examines1( R_MT ),
			    alters2( is_func ? tmp : dest, RR_ST ) );
	    }
	  
	  if (is_func)
	    {
	      if (offset != 0)
		syserr( gen_offset_from_ptr );
	      
	      /*
	       * add in the offset of the function
	       *
	       * XXX - we are assuming that there will never be more than 32767
	       * functions in one module
	       *
	       * This code is used by :-
	       *
	       * int use( int (* f )() ) { return (*f)(); }
	       * int func( void );
	       * int main( void ) { return use( func ); }
	       * int func( void ) { return 1; }
	       */

	      if (new_stubs)
		{
		  if ((dest != R_LR) &&
		      (((saved_ivars & regbit( R_LR )) == 0) ||
		       usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */
		    {
		      /* save link register */

		      /* NB/ do NOT use R_TMP1 as this is used by the address generating stub */
		      
		      move_register( R_LR, R_TMP2, FALSE );
		    }

		  /* call function address stub ! */

		  request_addr_stub( symbol );
		  
		  peep_xref( X_FuncAddr, symbol );
		  
		  peep_corrupt_addr_reg( R_ATMP );
		  
		  outdelsymref( OP_LAJ << 24 | (-3 & 0xffffff), symbol, examines0(), alters2( R_LR, RR_PC ) );

		  peep_protect_pc = 3;

		  /* result is placed in AR5 (alias R_ATMP) */
		  
		  move_register( R_ATMP, dest, FALSE );
		  
		  if ((dest != R_LR) &&
		      (((saved_ivars & regbit( R_LR )) == 0) ||	
		       usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */
		    {
		      /* restore saved link register */
		  
		      move_register( R_TMP2, R_LR, FALSE );
		    }
		}
	      else /* old stubs */
		{
		  peep_xref( X_DataSymb, symbol );
	      
		  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, tmp, 0,
				examines1( tmp ),
				alters2( tmp, RR_ST ) );
	      
		  /* and fetch the address out of the function table */
	      
		  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, dest, build_indirect( INDIRECT_REL, tmp, 0 ),
				examines1( tmp ),
				alters2( dest, RR_ST ) );
		}
	    }
	  else /* is data */
	    {
	      if (!fits_in_16_bits_signed( offset ))
		{
		  syserr( gen_offset_too_big, offset );
		}
	      
	      /*
	       * load R_TMP1 with the high part offset of 'name' into the data table
	       */

	      if (much_data)
		{
		  /*
		   * Note that we assume here, (and elsewhere) that there will never be more than
		   * 32767 functions in one module.
		   */

		  peep_xref( X_DataSymbHi, symbol );
	      
		  out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, R_TMP1, 0,
				examines0(),
				alters1( R_TMP1 ) );
	      
		  /*
		   * load R_TMP1 with the low part offset of 'name' into the data slots or function table
		   */
	      
		  peep_xref( X_DataSymbLo, symbol );
	      
		  out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, R_TMP1, 0,
				examines1( R_TMP1 ),
				alters1( R_TMP1 ) );	/* XXX - hide alteration of ST */

		  /*
		   * and add R_TMP1 into dest
		   */
	      
		  if (offset)
		    {
		      integer_immediate_op( OP_ADDI, OP_ADDI3, R_TMP1, R_TMP1, offset, TRUE );
		      
		      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, dest, R_TMP1,
				    examines2( dest, R_TMP1 ),
				    alters2( dest, RR_ST ) );
		    }
		  else
		    {		  
		      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, dest, R_TMP1,
				    examines2( dest, R_TMP1 ),
				    alters2( dest, RR_ST ) );
		    }		  
		}
	      else /* not much_data */
		{
		  peep_xref( X_DataSymbLo, symbol );

		  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, dest, 0,
				examines1( dest ),
				alters2( dest, RR_ST ) );
				
		  if (offset != 0)
		    {
		      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, dest, offset,
				    examines1( dest ),
				    alters2( dest, RR_ST ) );
		    }
		}
	    }
	}
    }

  peep_forget_about( dest );
  
  return;
  
} /* load_address_constant */

/*}}}*/
/*{{{  load_string_constant() */

static void
load_string_constant( RealRegister dest )
{
  /*
   * address = start of literal pool + current offset into literal pool
   *         =     `litlab'          +         `litpoolp'
   */


  if ((dest != R_LR) &&
      (((saved_ivars & regbit( R_LR )) == 0) ||
       usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */
    {
      /* save link register */

      move_register( R_LR, R_TMP1, FALSE );
    }
      
  /* put PC of next instruction into R_LR */

  outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

  peep_protect_pc = 3;
  
  /*
   * add in offset of start of literal pool
   *
   * NB/ beware of assumption that literal pool is within 16
   * (signed) bits of this instruction - this may not be true
   */
  
  peep_fref( litlab, LABREF_LIT16 ); 

  out_diadic_op( OP_LDI, ADDR_MODE_IMMEDIATE, dest, 0,
		examines0(),
		alters2( dest, RR_ST ) );

  /* add in offset into literal pool */
  
  /*
   * XXX - this is done as a seperate instruction because we cannot
   * reliably use R_LR immediately after a LAJ (due to a silicon bug)
   * and so I have reordered the ops
   */

  integer_immediate_op( OP_ADDI, OP_ADDI3, dest, dest, litpoolp -4 + 1, TRUE );

  /* add in value in link register */
  
  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, dest, R_LR,
		examines1( R_LR ),
		alters2( dest, RR_ST ) );

  if ((dest != R_LR) &&
      (((saved_ivars & regbit( R_LR )) == 0) ||
       usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */
    {
      /* restore saved link register */
      
      move_register( R_TMP1, R_LR, FALSE );
    }

  /* convert offset to byte offset */

  convert_to_byte_offset( dest );

  peep_forget_about( dest );
  
  return;
      
} /* load_string_constant */

/*}}}*/
/*{{{  register_op() */

  

/*
 * output an instruction using register addressing
 */

static void
register_op(
	    int32		diadic_op,
	    int32		triadic_op,
	    RealRegister	dest,
	    RealRegister	source1,
	    RealRegister	source2,
	    bool		can_commute )
{
  if (source1 == dest)
    {
      out_diadic_op( diadic_op, ADDR_MODE_REGISTER, dest, source2,
		     examines2( dest, source2 ),
		     alters2( dest, RR_ST ) );
    }
  else if (can_commute && source2 == dest)
    {
      out_diadic_op( diadic_op, ADDR_MODE_REGISTER, dest, source1,
		     examines2( dest, source1 ),
		     alters2( dest, RR_ST ) );
    }
  else
    {
      out_triadic_op( triadic_op, dest, source1, source2, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( source1, source2 ),
		     alters2( dest, RR_ST ) );
    }

  peep_forget_about( dest );
  
  return;
  
} /* register_op */

/*}}}*/
/*{{{  Initialisation Code */

/*{{{  prepare_for_initialisation() */

/*
 * this code is called just before we start to initialise
 * the data and function tables of the current module
 */

void
prepare_for_initialisation(
			   int			data_init,
			   LabelNumber *	data_label )
{
  death = 0;
  
  if (var_cc_private_flags > 2)
    {
      LabelNumber *	l;

      
      /* insert a JTAGHalt() to allow for debugging init code */
      
      flush_peepholer( DBG( "why" ) );
      
      l = nextlabel();
      
      load_integer( R_TMP1, 0x2f, FALSE );
      
      flush_peepholer( DBG( "hmm" ) );
      
      setlabel( l );
      
      integer_immediate_op( OP_SUBI, OP_SUBI3, R_TMP1, R_TMP1, 1, TRUE );
      
      conditional_branch_to( Q_NE, l );
      
      outinstr( 0x66FFFFFF, 0, 0 );
      
      flush_peepholer( DBG( "why not" ) );
    }
  
  if (suppress_module)
    {
      if (!data_init)
	{
	  outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( Q_AL ) << 16 | hardware_register( R_LR ),
		     examines1( R_LR ),
		     alters1( RR_PC ) );
	}
      else
	{
	  /* if we are only initialising data then skip all stages except stage 0 */
      
	  out_diadic_op( OP_CMPI, ADDR_MODE_IMMEDIATE, R_A1, 0,
			examines1( R_A1 ),
			alters1( RR_ST ) );

	  outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( Q_NE ) << 16 | hardware_register( R_LR ),
		     examines2( R_LR, RR_ST ),
		     alters1( RR_PC ) );
	}
    }
  else if (!data_init)
    {
      if (new_stubs && !usrdbg( DBG_ANY ))
	{
	  /* no data or functions to init */
	  
	  outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( Q_AL ) << 16 | hardware_register( R_LR ),
		     examines1( R_LR ),
		     alters1( RR_PC ) );
	}
      else
	{
	  /* if we are only initialising functions then skip all stages except stage 2 */
      
	  out_diadic_op( OP_CMPI, ADDR_MODE_IMMEDIATE, R_A1, 2,
			examines1( R_A1 ),
			alters1( RR_ST ) );

	  outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( Q_NE ) << 16 | hardware_register( R_LR ),
		     examines2( R_LR, RR_ST ),
		     alters1( RR_PC ) );
	}      
    }
  else
    {
      if (new_stubs && !usrdbg( DBG_ANY ))
	{
	  /* if we are only initialising data then skip all stages except stage 0 */
      
	  out_diadic_op( OP_CMPI, ADDR_MODE_IMMEDIATE, R_A1, 0,
			examines1( R_A1 ),
			alters1( RR_ST ) );

	  outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( Q_NE ) << 16 | hardware_register( R_LR ),
		     examines2( R_LR, RR_ST ),
		     alters1( RR_PC ) );
	}
      else
	{
	  /* we are initialisaing data and functions */
      
	  out_diadic_op( OP_CMPI, ADDR_MODE_IMMEDIATE, R_A1, 1,
			examines1( R_A1 ),
			alters1( RR_ST ) );

	  /* stage 0 initialises data */
      
	  conditional_branch_to( Q_LT, data_label );

	  /* and stage 1 is ignored */
      
	  outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( Q_EQ ) << 16 | hardware_register( R_LR ),
		     examines2( R_LR, RR_ST ),
		     alters1( RR_PC ) );
	}      
    }

  return;
  
} /* prepare_for_initialisation */

/*}}}*/
/*{{{  prepare_for_function_exporting() */

/*
 * this code is called just before we start to initialise the
 * function table of the module containing the
 * code we have just compiled.  This function is used to get
 * hold to the PC once, rather than every time a symbol is
 * referenced
 */

void
prepare_for_function_exporting( void )
{
  /*
   * this should not be necessary, but lets be paranoid ...
   */
  
  flush_peepholer( DBG( "start of function exports" ) );

  /* save the return address */

  move_register( R_LR, R_DS, FALSE );
  
  /*
   * make sure that the previous instruction is emitted
   */
  
  flush_peepholer( DBG( "call used in preparing for exports" ) );
   
  /*
   * remember the address of the next instruction
   */
  
  codep_of_call = codep;
  
  /* put PC of next instruction into R_LR */

  outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

  peep_protect_pc = 3;
  
  /*
   * XXX - the following NOP is because of a bug in the C40
   * silicon, whereby R_LR is occaisionally not set by the
   * LAJ until the second instruction after the LAJ
   */

  nop( FALSE );

  /* prevent this NOP from being peepholed */
  
  flush_peepholer( DBG( "LAJ bug 1" ) );
  
  /*
   * Use R_ADDR1 as a temporary address register.
   * We can do this safely as we know that we are
   * being called from outside of any of the normal
   * code generation sequence.
   */
  
  /* retreive PC from R_LR */

  move_register( R_LR, R_ADDR1, FALSE );

  /*
   * our temporary register now contains the word
   * address of the LAJ instruction plus 4.
   */

  /* restore the return address */

  move_register( R_DS, R_LR, FALSE );
  
  /*
   * There is no need to pad the LAJ instruction
   * as the load_static_data_pointer() function
   * is about to be called.  (see dumpdata() in heliobj.c)
   */
  
  return;
  
} /* prepare_for_function_exporting */

/*}}}*/
/*{{{  export_function() */

/*
 * load word address of symbol 'name' and
 * store it in the word pointed to by 'dest', and
 * increment 'dest' to point to the next word.
 *
 * This function is called from export_routines() in
 * c40/heliobj.c, where it used to be implemented by
 * emitting J op codes, but this proved to be too
 * inefficient.
 *
 * Note dest is a word pointer
 *
 * Also note that 'dest' is very probably 'R_ATMP'.
 */
 
void
export_function(
		Symstr *	name,
		RealRegister	dest )
{
  int32		offset;

  
  /*
   * calculate the word offset of the symbol
   * we have been given from the 'CALL'
   * in prepare_for_exporting()
   */

  /*
   * The offsets look like this:
   *
   *                                                     <-codep_of_call-->
   *              <---obj_symref()-->
   *              <---------------codebase--------------->
   *  <---------------------------R_ADDR1 -------------------------------->
   *
   *   ----------------------------------------------------------------------- - -
   *  |
   *  |
   *   ----------------------------------------------------------------------- - - 
   *  ^           ^                 ^                    ^               ^
   *  |           |                 |                    |               |
   *  |           |                 |                    |               |
   *  0    start of code    start of 'name'     start of init code   LAJ +0
   *
   *
   */

  /*
   * the + 4 is because the LAJ instruction puts PC + 4 into R_ADRR1
   */
  
  offset = ((codep_of_call + codebase) - obj_symref( name, xr_code, 0 )) / sizeof_int - 1 + 4;

  /*
   * the linker is going to insert an extra word between
   * the end of the code and the start of the initialisation code
   * as a chain of init sequences, so ...
   */

  offset += 1;
  
  /*
   * indicate that the next instruction will reference 'name'
   */
  
  peep_symref( name );
      
  /*
   * subtract the offset we have just calculated to the offset
   * we have in R_ADDR1 (see prepare_for_exporting())
   * and place the result in R_TMP1
   *
   * Note that if we are producing assembly output, we do not
   * really know what the offset is, and so we cannot use the
   * optimised version.
   */

  if (fits_in_8_bits_signed( offset ) && asmstream == NULL)
    {      
      out_triadic_op( OP_SUBI3, R_TMP1, R_ADDR1, offset, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
		     examines1( R_ADDR1 ),
		     alters2( R_TMP1, RR_ST ) );
    }
  else
    {
      if (asmstream != NULL && !fits_in_16_bits_signed( offset ))
	{
	  /* catch the case where two instructions are needed */
	  
	  syserr( gen_need_patch );
	}
      
      load_integer( R_TMP1, offset, FALSE );

      out_triadic_op( OP_SUBI3, R_TMP1, R_ADDR1, R_TMP1, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( R_TMP1, R_ADDR1 ),
		     alters2( R_TMP1, RR_ST ) );
    }

  /*
   * and store the result into 'dest'
   * (incrementing 'dest' afterwards)
   */
  
  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_POST_INCR, dest, 1 ),
		examines2( dest, R_TMP1 ),
		alters1( dest ) );
  return;
  
} /* export_function */

/*}}}*/
/*{{{  prepare_for_data_exporting() */

/*
 * this code is called just before we start to initialise the
 * data slots of the module containing the
 * code we have just compiled. 
 */

void
prepare_for_data_exporting( RealRegister reg )
{
  if (suppress_module == 2)
    {
      RealRegister	tmp;

      
      /*
       * Resident libraries only have one static data area, shared amoungst
       * many source files.  To cope with this each file is allocated part
       * of the data area for its own use, (determined by the amount of
       * data that file requires).  Hence when we are loading the static
       * data pointer we must add in the file's offset into the data area.
       */

      if (reg == R_TMP1)
	tmp = R_TMP2;
      else
	tmp = R_TMP1;

      if (much_data)
	{
	  peep_xref( X_DataSymbHi, bindsym_( datasegment ) );
	  
	  out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, tmp, 0,
			examines1( tmp ),
			alters2( tmp, RR_ST ) );
      
	  peep_xref( X_DataSymbLo, bindsym_( datasegment ) );
	  
	  out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, tmp, 0,
			examines1( tmp ),
			alters1( RR_ST ) );	/* XXX - note lie about not affecting RR_ST */
	}
      else
	{
	  peep_xref( X_DataSymbLo, bindsym_( datasegment ) );
	  
	  conditional_load( Q_AL, ADDR_MODE_IMMEDIATE, tmp, 0,
			   examines0(),
			   alters1( tmp ) );
	}      

      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, reg, tmp,
		    examines2( reg, tmp ),
		    alters2( reg, RR_ST ) );
    }
  
  /*
   * XXX
   *
   * 'reg' currently contains the byte offset from IR0
   * of the start of the data table.  Convert this to
   * a word pointer to speed up stores of data values
   */

  convert_to_word_address( reg, reg );

  /* save the return address so that we can use LAJ's with impunity */
  
  move_register( R_LR, R_FT1, FALSE );

  saved_link_reg = R_FT1;

  /* note that we have not done a LAJ yet */
  
  codep_of_call = 0;
  
  return;
  
} /* prepare_for_data_exporting */

/*}}}*/
/*{{{  prepare_for_block_copying() */

/*
 * initialise the pointer to the data that is to be copied into the program's data area
 */

#define	SRC_REG		R_ADDR1
#define DST_REG		R_ATMP
#define VAL_REG		RR_R1

void
prepare_for_block_copying( LabelNumber * start_of_data )
{
  flush_peepholer( DBG( "do not swap this LAJ backwards" ) );
  
  /* put PC into R_LR */

  outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

  /* ensure that codep is up to date */
	  
  flush_peepholer( DBG( "getting hold of PC" ) );

  codep_of_call = codep;

  peep_protect_pc = 3;

  peep_fref( start_of_data, LABREF_LIT16 );

  /* load the offset from the return address of the LAJ to the start of the data */
  
  out_diadic_op( OP_LDI, ADDR_MODE_IMMEDIATE, SRC_REG, -3,
		examines0(),
		alters2( SRC_REG, RR_ST ) );

  /* add in the program counter */
  
  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, SRC_REG, R_LR,
		examines2( SRC_REG, R_LR ),
		alters2( SRC_REG, RR_ST ) );

  /* ensure that three instructions follow the LAJ */
  
  nop( TRUE );
  
  return;  
  
} /* prepare_for_block_copying */

/*}}}*/
/*{{{  block_copy_data() */

void
block_copy_data( int32 num_words_to_copy )
{
  if (num_words_to_copy < 1)
    {
      syserr( gen_no_data_to_init, num_words_to_copy );
    }
  else if (num_words_to_copy < 5)
    {
      /* copy by hand - it is quicker than a loop */
      
      /* load first word */
      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, VAL_REG, build_indirect( INDIRECT_POST_INCR, SRC_REG, 1 ),
		    examines1( SRC_REG ),
		    alters3( VAL_REG, RR_ST, SRC_REG ) );

      while (--num_words_to_copy)
	{
	  /*
	   * instruction repeated is:
	   *
	   *  LDI    *R_ADDR1++(1), R1
	   *  || STI  R1,          *R_ATMP++(1)
	   *
	   */
      
	  outinstr( B_1101 << 28 | B_1010 << 24 |
		   hardware_register( VAL_REG ) << 22 | hardware_register( VAL_REG ) << 16 |
		   build_parallel_indirect( INDIRECT_POST_INCR, DST_REG ) << 8 |
		   build_parallel_indirect( INDIRECT_POST_INCR, SRC_REG ),
		   examines3( SRC_REG, VAL_REG, DST_REG ),
		   alters3(   SRC_REG, VAL_REG, DST_REG ) );
	}
      
      /* save last word */
      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, VAL_REG, build_indirect( INDIRECT_POST_INCR, DST_REG, 1 ),
		    examines2( VAL_REG, DST_REG ),
		    alters1( DST_REG ) );
    }
  else
    {
      LabelNumber *	l;  
  
     
      /* load repeat count */
      
      load_integer( RR_RC, num_words_to_copy - 2, FALSE );
      
      /* try to move the instruction to a more advantagous location */
      
      peep_shift_back( 2 );
      
      /* load first word */
      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, VAL_REG, build_indirect( INDIRECT_POST_INCR, SRC_REG, 1 ),
		    examines1( SRC_REG ),
		    alters3( VAL_REG, RR_ST, SRC_REG ) );
      
      /* start repeat block */
      
      l = nextlabel();
      
      outdellabref( (OP_RPTBD << 24) | (-3 & 0x00FFFFFFU),
		   l,
		   LABREF_OFF24,
		   examines1( RR_PC ),
		   alters4( RR_RC, RR_RS, RR_RE, RR_PC ) );
      
      setlabel( l );
      
      /*
       * instruction repeated is:
       *
       *  LDI    *R_ADDR1++(1), R1
       *  || STI  R1,          *R_ATMP++(1)
       *
       */
      
      outinstr( B_1101 << 28 | B_1010 << 24 |
	       hardware_register( VAL_REG ) << 22 | hardware_register( VAL_REG ) << 16 |
	       build_parallel_indirect( INDIRECT_POST_INCR, DST_REG ) << 8 |
	       build_parallel_indirect( INDIRECT_POST_INCR, SRC_REG ),
	       examines3( SRC_REG, VAL_REG, DST_REG ),
	       alters3(   SRC_REG, VAL_REG, DST_REG ) );
      
      /* prevent the repeated instruction from being peepholed */
      
      flush_peepholer( DBG( "block data init" ) );
      
      /* save last word */
      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, VAL_REG, build_indirect( INDIRECT_POST_INCR, DST_REG, 1 ),
		    examines2( VAL_REG, DST_REG ),
		    alters1( DST_REG ) );
    }
  
  return;
  
} /* block_copy_data */

/*}}}*/
/*{{{  store_data_value() */

void
store_data_value( unsigned32 value )
{
  if (value == 0)
    syserr( gen_already_zero );

#ifdef STIK_NOW_WORKS_ON_HARDWARE
  if (fits_in_5_bits_signed( value ))
    {
      /* addressing mode is actually ADDR_MODE_INDIRECT */
      
      out_diadic_op( OP_STIK, ADDR_MODE_IMMEDIATE, value, build_indirect( INDIRECT_POST_INCR, DST_REG, 1 ),
		    examines1( DST_REG ),
		    alters1( DST_REG ) );

      return;
    }
#endif

  load_integer( VAL_REG, value, FALSE );

  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, VAL_REG, build_indirect( INDIRECT_POST_INCR, DST_REG, 1 ),
		examines2( VAL_REG, DST_REG ),
		alters1( DST_REG ) );

  return;
  
} /* store_data_value */

/*}}}*/
/*{{{  export_data_symbol() */

  

/*
 * load byte offset from IR0 of symbol 'name' and
 * store it in the word pointed to by 'R_ATMP', and
 * increment 'R_ATMP' by one word.
 *
 * This function is called from dump_data() in
 * c40/heliobj.c, where it used to be implemented by
 * emitting J op codes, but this proved to be too
 * inefficient.
 *
 * Note R_ATMP is a word pointer.
 */
 
void
export_data_symbol(
		   Symstr *	name,		/* symbol whoes address is to be stored */
		   int32	offset,		/* offset (in bytes) from symbol to store */
		   unsigned32	current_offset )/* the offset (in words) of R_ATMP from start of data table */
{
  if (streq( symname_( name ), "__dataseg" ))
    {
      /*
       * Ah ha, we are trying to place __dataseg + offset
       * into static_data_ptr + current_offset,
       * BUT
       * we know that DST_REG holds IR0 + [static_data_ptr / sizeof_int] + current_offset (in words)
       * and that static_data_ptr == __dataseg !
       */

      if (offset & (sizeof_int - 1))
	{
	  /* offset is not an integer multiple, *sigh* */

	  offset -= current_offset * sizeof_int;
	  
	  /* get DST_REG - IR0 into temporary register */

	  out_triadic_op( OP_SUBI3, VAL_REG, DST_REG, R_BASE, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			 examines2( DST_REG, R_BASE ),
			 alters2( VAL_REG, RR_ST ) );

	  /* convert to a byte offset */

	  out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, VAL_REG, 2,
			examines1( VAL_REG ),
			alters2( VAL_REG, RR_ST ) );

	  /* add in offset */

	  integer_immediate_op( OP_ADDI, OP_ADDI3, VAL_REG, VAL_REG, offset, TRUE );

	  /* result is now in VAL_REG, ready to store */
	}
      else
	{
	  offset /= sizeof_int;

	  /*
	   * adjust offset
	   */
      
	  offset -= current_offset;

	  /* get DST_REG - IR0 into temporary register */

	  out_triadic_op( OP_SUBI3, VAL_REG, DST_REG, R_BASE, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			 examines2( DST_REG, R_BASE ),
			 alters2( VAL_REG, RR_ST ) );

	  /* add in offset */

	  integer_immediate_op( OP_ADDI, OP_ADDI3, VAL_REG, VAL_REG, offset, TRUE );

	  /* convert to a byte offset */

	  out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, VAL_REG, 2,
			examines1( VAL_REG ),
			alters2( VAL_REG, RR_ST ) );

	  /* result is now in VAL_REG, ready to store */
	}
    }
  else if (is_function( name ))
    {
      if (offset != 0)
	syserr( gen_offset_from_ptr );

      offset = obj_symref( name, xr_code, 0 );

      if (offset != -1)
	{
	  /*
	   * This is can happen with the following code:
	   *
	   *   int func( void ) { return 3; }
	   *
	   *   int (* func_ptr)() = func;
	   *
	   */

	  if (codep_of_call == 0)
	    {
	      flush_peepholer( DBG( "save out LAJs" ) );
	      
	      /* put PC into R_LR */

	      outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

	      /* adjust symbol offset to be relative to next instruction */

	      flush_peepholer( DBG( "getting address of LAJ" ) );

	      peep_protect_pc = 3;

	      codep_of_call = codep;
	    }
	  
	  /* calculate required offset */
      
	  offset = codep_of_call + codebase - offset + (3 * sizeof_int);

	  /* compute compound offset */
	  
	  if (offset & 3)
	    syserr( gen_ptr_not_aligned );
	  
	  offset /= sizeof_int;
	  
	  /* indicate that the next instruction will reference 'symbol' */
	      
	  peep_symref( name );
	  
	  load_integer( VAL_REG, offset, FALSE );
	  
	  /* subtract offset from PC (in R_LR) */

	  out_diadic_op( OP_SUBRI, ADDR_MODE_REGISTER, VAL_REG, R_LR,
			examines2( VAL_REG, R_LR ),
			alters2( VAL_REG, RR_ST ) );	  
	}
      else
	{
	  /*
	   * function is defined in another module, eg:
	   *
	   *
	   *   extern int func( void );
	   *
	   *   int (* func_ptr)() = func;
	   *
	   */

	  if (new_stubs)
	    {
	      request_new_stub( name );
	      
	      /* dealt with later on */
	    }
	  else if (!few_modules)
	    {
	      /* copy module table pointer */

	      move_register( R_MT, R_DS, FALSE );
	  
	      /*
	       * add in the offset of the module containing 'name'
	       *
	       * XXX - we are assuming that there will never be more than 32767 modules
	       */

	      peep_xref( X_DataModule, name );
	  
	      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_DS, 0,
			    examines1( R_DS ),
			    alters2( R_DS, RR_ST ) );
      
	      /* get the address of that module's function table */
 
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_DS, build_indirect( INDIRECT_REL, R_DS, 0 ),
			    examines1( R_DS ),
			    alters2( R_DS, RR_ST ) );
	    }
	  else
	    {
	      peep_xref( X_DataModule, name );
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_DS,
			    build_indirect( INDIRECT_PRE_ADD, R_DS, 0 ),
			    examines1( R_DS ),
			    alters2( R_DS, RR_ST ) );
	    }

	  if (new_stubs)
	    {
	      /* call function address stub ! */
	      
	      request_addr_stub( name );
	      
	      /*
	       * unfortunately the address returning stub places its result
	       * in R_ATMP, the register we are using to index the current
	       * slot into the module table!
	       */
	      
	      move_register( R_ATMP, R_DS, FALSE );
	      
	      peep_corrupt_addr_reg( R_ATMP );

	      peep_xref( X_FuncAddr, name );

	      outdelsymref( OP_LAJ << 24 | (-3 & 0xffffff), name, examines0(), alters2( R_LR, RR_PC ) );

	      move_register( R_ATMP, VAL_REG, FALSE );

	      move_register( R_DS, R_ATMP, FALSE );
	      
	      codep_of_call = 0;	      
	    }
	  else
	    {
	      /* add in the offset of 'name' into the function table */

	      peep_xref( X_DataSymbLo, name );

	      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_DS, 0,
			    examines1( R_DS ),
			    alters2( R_DS, RR_ST ) );
	  
	      /* get the address of the function */

	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, VAL_REG, build_indirect( INDIRECT_REL, R_DS, 0 ),
			    examines1( R_DS ),
			    alters2( VAL_REG, RR_ST ) );
	    }	  
	}
      
      /* result is now in VAL_REG, ready to store */
    }
  else
    {
      ExtRef *		x = symext_( name );


      if (!is_data( x ))
	syserr( gen_not_data, symname_( name ) );

      if (is_defined( x ))
	{
	  /*
	   * This is generated by code such as :
	   *
	   *     int	fred;
	   *     int *	jim = &fred + 5;
	   *
	   * XXX - maybe this could be optimised one day
	   * since we might know the offset of fred from
	   * the start of the data table without using
	   * the DataModule directives as below.
	   */

	  /* get pointer to this module's data slots */

	  load_static_data_ptr( R_DS, FALSE, name );
	}
      else
	{
	  /*
	   * This is generated by code such as :
	   *
	   *     extern int	fred;
	   *     int *		jim = &fred;
	   *
	   */

	  if (!few_modules)
	    {
	      move_register( R_MT, R_DS, FALSE );
	  
	      peep_xref( X_DataModule, name );
	  
	      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_DS, 0,
			    examines1( R_DS ),
			    alters2( R_DS, RR_ST) );

	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_DS, build_indirect( INDIRECT_REL, R_DS, 0 ),
			    examines1( R_DS ),
			    alters2( R_DS, RR_ST ) );
	    }
	  else
	    {
	      peep_xref( X_DataModule, name );
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_DS,
			    build_indirect( INDIRECT_PRE_ADD, R_MT, 0 ),
			    examines2( R_DS, R_MT ),
			    alters2( R_DS, RR_ST ) );
	    }
	}
      
      /*
       * load VAL_REG with the high part offset of 'symbol' into the data table
       */

      if (much_data)
	{
	  peep_xref( X_DataSymbHi, name );
	  
	  out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, VAL_REG, 0,
			examines0(),
			alters1( VAL_REG ) );
	  
	  /*
	   * load VAL_REG with the low part offset of 'symbol' into the data table
	   */
	  
	  peep_xref( X_DataSymbLo, name );
	  
	  out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, VAL_REG, 0,
			examines1( VAL_REG ),
			alters1( VAL_REG ) );	/* XXX - note lie about not affecting RR_ST */
	}
      else
	{
	  peep_xref( X_DataSymbLo, name );
	  
	  conditional_load( Q_AL, ADDR_MODE_IMMEDIATE, VAL_REG, 0,
			   examines0(),
			   alters1( VAL_REG ) );
	}
      
      /*
       * and add R_DS to VAL_REG
       */

      if (offset)
	{
	  if (!fits_in_16_bits_signed( offset ))
	    {
	      syserr( gen_offset_too_big, offset );
	    }
	  else
	    {	  	  
	      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, VAL_REG, offset,
			    examines1( VAL_REG ),
			    alters2( VAL_REG, RR_ST ) );
	      
	      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, VAL_REG, R_DS,
			    examines2( R_DS, VAL_REG ),
			    alters2( VAL_REG, RR_ST ) );
	    }
	}
      else
	{
	  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, VAL_REG, R_DS,
			examines2( R_DS, VAL_REG ),
			alters2( VAL_REG, RR_ST ) );
	}
    }

  /* store result into data area, incrementing pointer as we go */
  
  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, VAL_REG, build_indirect( INDIRECT_POST_INCR, DST_REG, 1 ),
		examines2( DST_REG, VAL_REG ),
		alters1( DST_REG ) );
  return;
  
} /* export_data_symbol */

/*}}}*/
/*{{{  finished_exporting() */

/*
 * we have finished exporting symbols
 */

void
finished_exporting( void )
{
  VRegInt vr1, vr2, vm;


  vr1.r = vr2.r = GAP;
  vm.i  = 0;

  show_instruction( J_ENDPROC, vr1, vr2, vm );

  return;  
}

/*}}}*/

/*}}}*/
/*{{{  call() */

static void
call(
     Symstr *	name,		/* the function to call */
     bool	tailcall )	/* true if this call is the last action of the current function */
{
  int32		dest;


  /* The initial part of this code is independent of operating   */  
  /* system since if we know how far the routine is there is no  */
  /* problem. Things get a bit trickier from there on in.        */

  if (suppress_module == 1 && streq( symname_( name ), "GetExecRoot" ))
    {
      /*
       * This is a HORRIBLE HACK.
       *
       * The idea is to allow the kernel to get hold of the
       * exec root structure, (whoes address is in TVTP),
       * without using a function call.  I tried using _word(),
       * but the compiler does not really understand about
       * this function returning a result in R0.
       */
      
      outinstr( 0x76190001, examines0(), alters2( R_TMP1, RR_ST ) );	/* LDEP TVTP, R_TMP1 */

      out_diadic_op( OP_SUBI, ADDR_MODE_REGISTER, R_TMP1, R_BASE,
		    examines2( R_BASE, R_TMP1 ),
		    alters2( R_TMP1, RR_ST ) );
      
      out_triadic_op( OP_LSH3, R_A1, R_TMP1, 2,
		     ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
		     examines1( R_TMP1 ),
		     alters2( R_A1, RR_ST ) );

      peep_forget_about( RR_R0 );
      
      return;
    }
  
  /* forget about contents of address registers after a function call */

  peep_corrupt_all_addr_regs();

  /* make sure stack is up to date before calling */
  
  flush_pending_pushes();

  if (!new_stubs && in_stubs)
    {
      /* load temporary address register with address of symbol */
      
      load_address_constant( R_ATMP, name, 0 );

      /* and jump to it */
      
      if (tailcall)
	{
	  /*
	   * (the following instruction is a conditional CALL, register relative)
	   */

	  outinstr( OP_BRcr << 24 | C_FROMQ( Q_AL ) << 16 | hardware_register( R_ATMP ),
		   examines1( R_ATMP ),
		   alters1( RR_PC ) );
	}
      else
	{
	  syserr( gen_fn_call_in_stub, symname_( name ) );      
	}

      return;
    }
  
  /* XXX - by removing this flush I can slide the LAJ back several instructions... */

  /*  flush_peepholer( DBG( "calling another function" ) ); */
  
  dest = obj_symref( name, xr_code, 0 );

  /*
   * determine if we already know the location
   * of the function to be called
   */
  
  if (dest != -1)
    {
      int32 	off;

      
      /* previously defined routine - save the symbol name for disassembly purposes */

      /* calculate offset of dest from current position (in words) */
  
      off  = (dest - codebase) / sizeof_int;

      /* codep ins subtracted in the peepholer */
      
      off -= 3;	/* allow for offset built into LAJ instruction */

      if (fits_in_24_bits_signed( off ))
        {
	  /* the following is an unconditional link and jump, PC relative */

	  if (tailcall)
	    {
	      outdelsymref( OP_BRD << 24 | (off & 0xffffff), name,
			  examines0(), alters1( RR_PC ) );
	    }
	  else
	    {
	      outdelsymref( OP_LAJ << 24 | (off & 0xffffff), name,
			  examines0(),
			  alters2( R_LR, RR_PC ) );
	    }

	  return;
        }
      else
        {
	  syserr( gen_cannot_call_offset, off );
        }
    }

  /* Static forward or external calls are generated in the same   */
  /*  way. The addition to the stublist will be removed if the    */
  /*  function is subsequently defined.                           */

  peep_xref( X_PCreloc /* X_FUNCstub */, name );

  if (tailcall)
    {
      outdelsymref( (OP_BRD << 24) | (-3 & 0x00ffffffU), name, examines0(), alters1( RR_PC ) );
    }
  else
    {
      outdelsymref( (OP_LAJ << 24) | (-3 & 0x00ffffffU), name, examines0(), alters2( R_LR, RR_PC ) );
    }

  request_stub( name );
  
  return;
  
} /* call */

/*}}}*/
/*{{{  Routine Entry/Exit */

/*{{{  save_regs() */

/*
 * pushes every register whoes bit is set in 'mask'
 */

static void
save_regs(
	  int32		mask,		/* bit mask of registers to save 			*/
	  bool		is_float )	/* TRUE if floating point registers are being saved 	*/
{
  int32	i;	/* XXX beware if number of registers > 32 */


  /*
   * NB/ save in reverse order so that argument registers are
   * placed on the stack in descending order
   */

  if (is_float)
    {
      for (i = 31; i >= 0; i--)
	{
	  if (mask & regbit( i ))
	    {
	      dpush( i );
	    }
	}
    }
  else
    {
      for (i = 31; i >= 0; i--)
	{
	  if (mask & regbit( i ))
	    {
	      ipush( i );
	    }
	}
    }

  return;
    
} /* save_regs */

/*}}}*/
/*{{{  restore_regs() */

/*
 * pops every integer register whoes bit is set in 'mask'
 */

static void
restore_regs(
	     int32	mask,		/* bit mask of registers to remove from stack	*/
	     bool	is_float )	/* TRUE if registers are floating point		*/
{
  /* NB/ restore must be done in opposite order to save */


  if (is_float)
    {
      while (mask)
	{
	  RealRegister r1  = firstbit( mask );

      
	  dpop( r1 );

	  mask ^= regbit( r1 );
	}
    }
  else
    {
      while (mask)
	{
	  RealRegister r1  = firstbit( mask );

      
	  ipop( r1 );

	  mask ^= regbit( r1 );
	}
    }

  return;
  
} /* restore_regs */

/*}}}*/
#ifdef TARGET_HAS_DEBUGGER
/*{{{  do_notify_entry() */

/*
 * set up and perform a call to _notify_entry()
 */

static void
do_notify_entry( void )
{
  extern Symstr *	current_proc;
  extern LabelNumber *	proc_label;
  int32			save_mask;
  

  if (debugging( DEBUG_Q ))
    cc_msg( "debugger: generating _notify_entry for: %s\n", symname_( current_proc ) );
  
  flush_peepholer( DBG( "notify_entry" ) );
  
  /*
   * we know that no temporaries have yet been assigned
   * so just save the argument registers,
   * but remember that we want to pass the stack
   * pointer as it was before pushing these registers
   * as our third argument to _notify_entry()
   */

  save_mask  = regbit( R_A1 + NARGREGS ) - regbit( R_A1 );

  save_regs( save_mask, FALSE );
  
  /* third argument: the stack pointer */

  out_triadic_op( OP_ADDI3, R_A1 + 2, R_SP, bitcount( save_mask ),
		 ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
		 examines1( R_SP ),
		 alters1( R_A1 + 2 ) );
		
  /* second argument: the frame pointer */
  
  move_register( R_FP, R_A1 + 1, FALSE );

  /* first argument: the address of the ProcInfo structure as a word pointer */
  
  proc_label = nextlabel();

  /*
   * put PC of next instruction into R_LR
   *
   * (There is no need to save R_LR, as it will
   *  have been pushed onto the stack)
   */

  outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

  /*
   * add in offset of the proc structure
   *
   * NB/ beware of assumption that proc structure is within 16
   * (signed) bits of this instruction - this may not be true
   */
  
  peep_fref( proc_label, LABREF_LIT16 ); 

  out_diadic_op( OP_LDI, ADDR_MODE_IMMEDIATE, R_A1, -3,
		examines0(),
		alters2( R_A1, RR_ST ) );

  /* add in value in link register */
  
  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, R_A1, R_LR,
		examines1( R_LR ),
		alters2( R_A1, RR_ST ) );

  nop( FALSE );

  flush_peepholer( DBG( "protect LAJ" ) );
  
  /* finally call the _notify_entry function */
  
  call( _notify_entry, FALSE );

  /* now restore saved argument registers */
  
  restore_regs( save_mask, FALSE );
  
  return;
  
} /* do_notify_entry */

/*}}}*/
#endif /* TARGET_HAS_DEBUGGER */
/*{{{  demo stack extension code */

/*
 * XXX - CALLING SCHEME
 *
 * version:	7			(keep in step with module table and register schemes)
 * date:	23rd October 1991
 * designed:	NC
 *
 * changes from version 3 to version 7:
 *      Register allocation scheme has changed.  There are
 *	now 6 variable registers and 7 temporaries.
 *	(There were no versions 4, 5 or 6)
 *
 * changes from version 2 to version 3:
 *	frame pointer now pushed before link register
 *	frame pointer always retrieved relative to frame pointer, not stack pointer
 *	stack chunks now have a 6 word header
 *	fake arguments and (optionally) frame pointer pushed onto new stack chunk
 *	 by stack extension code
 *	stack extension code now implemented
 *	stack end pointer no longer points to end of stack
 *
 * changes from version 1 to version 2:
 *	stack is now falling
 *
 * This scheme uses a frame pointer, a stack pointer, a stack end pointer
 * and a module table pointer.  Arguments are accesssed relative to the
 * frame pointer.  Local variables are accessed relative to the stack
 * pointer.  External functions and data are accessed relative to the
 * module table.  If the stack fills up, a new chunk is allocated for it
 * by the compiler, so that the program can carry on execution normally.
 *
 * This scheme does not use a display, but if we need one then it would
 * be placed on the stack and accessed via the stack pointer.  The display
 * is a dynamic structure that cannot be a fixed because languages like
 * PASCAL require function tables in the display on a per function basis.
 *
 * We use an falling, full, stack pointer
 *
 *
 * before entry to a (non-leaf) function has completed :-
 * (NB/ arguments 1 to 4 held in registers, return address held in link register)
 *
 *
 *     This is the execution stack, memory DECREASES this way -->
 *     -----------------------------  -  -  -  -  - ----
 *               |     |   |     |                      |
 *      previous | arg |...| arg |                      |
 *      function |  n  |   |  5  |                      |
 *               |     |   |     |                      |
 *     -----------------------------  -  -  -  -  - ----
 *         ^                  ^              ^         ^
 *         |                  |              |         |
 *         FP                 SP             SE   (end of stack)
 *
 *    FP  - current frame pointer
 *    SP  - current stack pointer
 *    SE  - stack end pointer (plus STACK_GUARD words)
 *
 *
 * after entry to a (non-leaf) function has completed
 * (NB/ arguments 1 to 4 now on stack, all arguments accessed via FP)
 *
 *
 *         This is the execution stack, memory DECREASES this way -->
 *     ------------------------------------------------------------- - - - ---
 *               |     |   |     |     |     |       |   |       |            |
 *      previous | arg |...| arg | FP' | LNK | saved |...| saved |            |
 *      function |  n  |   |  1  |     |     |  reg  |   |  reg  |            |
 *               |     |   |     | |   |     |       |   |       |            |
 *     ----------------------------|-------------------------------- - - - ---
 *        ^                        | ^                       ^           ^
 *        |________________________| |                       |           |
 *                                   |                       |           |
 *                                   FP                      SP          SE
 *
 *	FP' - frame pointer  of previous function
 *	LNK - return address to previous function
 *
 * during execution of a (non-leaf) function :-
 * (note locals on the stack)
 *
 *
 *         This is the execution stack, memory DECREASES this way -->
 *     -------------------------------------------------------------------------------- -
 *              |     |   |     |     |     |       |   |       |       |   |       |
 *     previous | arg |...| arg | FP' | LNK | saved |...| saved | local |...| local |
 *     function |  n  |   |  1  |     |     |  reg  |   |  reg  |   n   |   |   1   |
 *              |     |   |     | |   |     |       |   |       |       |   |       |
 *     ---------------------------|---------------------------------------------------- -
 *       ^                        | ^                                           ^
 *       |________________________| |                                           |
 *                                  |                                           |
 *                                  FP                                          SP
 *
 *
 * before entry to a (non-leaf) function has completed :-
 * (with no more than 4 arguments passed to the function)
 *
 *         This is the execution stack, memory DECREASES this way -->
 *     -------------  -  -  -  -  - ---
 *               |                     |
 *      previous |                     |
 *      function |                     |
 *               |                     |
 *     -------------  -  -  -  -  - ---
 *       ^     ^                 ^
 *       |     |                 |
 *       FP    SP                SE
 *
 * after entry to such a function has completed :-
 * (NB/ no new stack frame allocated)
 * (NB/ this example only applies if none of the arguments have their address taken)
 *
 *         This is the execution stack, memory DECREASES this way -->
 *     --------------------------------------- - - - - ---
 *               |     |       |   |       |              |
 *      previous | LNK | saved |...| saved |              |
 *      function |     |  reg  |   |  reg  |              |
 *               |     |       |   |       |              |
 *     --------------------------------------- - - - - ---
 *        ^                             ^            ^
 *        |                             |            |
 *        |                             |            |
 *        FP                            SP           SE
 *
 *
 * after entry to a such function has completed :-
 * (when the function is a LEAF FUNCTION)
 * (leaf functions call no other functions,
 *                 have all their arguments passed in registers,
 *                 do not take the address of any argument,
 *                 do not use varargs
 *                 and do not need a large amount of stack space)
 *
 * (NB/ link register not saved)
 *
 *
 *         This is the execution stack, memory DECREASES this way -->
 *     ---------------------------------- - - - - ---
 *               |       |   |       |               |
 *      previous | saved |...| saved |               |
 *      function |  reg  |   |  reg  |               |
 *               |       |   |       |               |
 *     ---------------------------------- - - - - ---
 *        ^                       ^            ^
 *        |                       |            |
 *        |                       |            |
 *        FP                      SP           SE
 *
 *
 * after entry to a (non-leaf) function has completed (when there is insufficient stack space) :-
 * (NB/ insufficient space is defined to be less than STACK_GUARD words, (usually 64 words),
 *      the code will fail if there is insufficient room to store 5 words on stack).
 *
 *         This is the execution stack, memory DECREASES this way -->
 *     ------------------------------------------------------
 *               |     |   |     |     |   |     |      |    |
 *      previous | arg |...| arg | arg |...| arg |  FP' |    |
 *      function |  n  |   |  5  |  4  |   |  1  |      |    |
 *               |     |   |     |     |   |     |  |   |    |
 *     ---------------------------------------------|--------
 *        ^                   ^               ^     | ^  
 *        |___________________|_______________|_____| |  
 *                            |               |       |  
 *                            |               |       FP
 *                            |___       _____|
 *                                |     |
 *                                |     |
 *                <this is a new chunk of memory> memory DECREASES this way -->
 *      --------------------------|-----|------------------------------------------------------------ - - ---
 *     |       |       |       |  |  |  |  |     |dummy|   |dummy|dummy|      |       |   |       |          |
 *     | Chunk | Next  | Prev  | SP' | SE' | LNK | arg |...| arg | FP' | LNK' | saved |...| saved |          |
 *     | Size  | Chunk | Chunk |     |     |     |  4  |   |  1  |     |      |  reg  |   |  reg  |          |
 *     |       |       |       |     |     |     |     |   |     |     |      |       |   |       |          |
 *      --------------------------------------------------------------------------------------------- - - ---
 *                                                                                            ^         ^
 *                                                                                            |         |
 *                                                                                            SP        SE
 *
 *      LNK' - link address of a special assembler routine that frees up the newly allocated stack chunk
 *             (or more probably frees up the chunk after this one)
 *
 *	SP' - stack pointer before this function
 *
 *	SE' - stack end pointer for previous chunk
 *     
 *
 *
 * after entry to a (leaf) function has completed (when there was insufficient stack space) :-
 *
 *         This is the execution stack, memory DECREASES this way -->
 *     ----------------------
 *                 |         |
 *      previous   |         |
 *      function   |         |
 *                 |         |
 *     ----------------------
 *        ^       ^      ^
 *        |       |      |
 *        |       |      |______________
 *        FP      |                     |
 *                |_______________      |
 *                                |     |
 *                                |     |
 *      <this is a new chunk of memory> memory DECREASES this way -->
 *      --------------------------|-----|------------------------------- - - - ---
 *     |       |       |       |  |  |  |  |     |       |   |       |            |
 *     | Chunk | Next  | Prev  | SP' | SE' | LNK | saved |...| saved |            |
 *     | Size  | Chunk | Chunk |     |     |     |  reg  |   |  reg  |            |
 *     |       |       |       |     |     |     |       |   |       |            |
 *      ---------------------------------------------------------------- - - - ---
 *                                                               ^           ^
 *                                                               |           |
 *                                                               SP          SE
 *
 *	NB/ Link register now points to the special free_extension_stack routine
 *          and the original value of the link register has been saved on the new stack
 *          
 *
 * STILL TO DO
 *  alloca		(emulate J_SETSP + add stack checking - is alloca allowed to fail ? )
 */

/*
 * pseudo-code for stack extension routines
 *
 * 
 * typedef struct stack_chunk_header
 *   {
 *     unsigned int			size_of_chunk_in_words;	// excludes the size of the header      
 *     struct stack_chunk_header *	ptr_to_next_chunk;	// NULL if no following chunk           
 *     struct stack_chunk_header *	ptr_to_prev_chunk;	// NULL if no previous  chunk           
 *     unsigned int *			old_stack_pointer;	// last stack pointer in previous chunk 
 *     unsigned int *			old_stack_end_pointer;	// stack end pointer of previous chunk  
 *     void (*				old_return_address)();	// old value of link register           
 *   }
 * stack_chunk_header;
 * 
 * #define CHUNK_HEADER_SIZE	(sizeof (stack_chunk_header) / sizeof (int))	// in words 
 * 
 * static stack_chunk_header *	next_free_chunk = NULL;
 * static stack_chunk_header *	current_chunk   = NULL;
 * static int			min_chunk_size  = 2000;
 * 
 * static void
 * free_stack_chunk( void )
 * {
 *   // indicate that this chunk is now free 
 *   
 *   next_free_chunk = current_chunk;
 * 
 *   // get the previous chunk 
 *   
 *   current_chunk = next_free_chunk->ptr_to_prev_chunk;
 * 
 *   // check to see if we have unwound the stack all the way! 
 *   
 *   if (current_chunk == NULL)
 *     {
 *       abort();
 *     }
 * 
 *   // recover registers from the newly freed chunk 
 * 
 *   return_address    = next_free_chunk->old_return_address;
 *   stack_end_pointer = next_free_chunk->old_stack_end_pointer;
 *   stack_pointer     = next_free_chunk->old_stack_pointer;
 * 
 *   // XXX we are currently NOT freeing any chunks 
 *   
 *   return;
 *   
 * } // free_stack_chunk 
 * 
 * 
 * static void
 * get_new_stack_chunk(
 * 		    unsigned int	num_words_required,
 * 		    unsigned int	num_words_on_stack )
 * {
 *   stack_chunk_header *	new_chunk;
 *   int			size_of_chunk;
 * 
 * 
 *   // see if there are any free stack chunks 
 *   
 *   if (next_free_chunk == NULL)
 *     {
 *       // none available - try allocating a new one 
 *       
 *       size_of_chunk = min_chunk_size;
 * 
 *       // ensure that the new stack chunk will of suffiecient size 
 *       
 *       if (size_of_chunk < num_words_required)
 *       {
 * 	   size_of_chunk = num_words_required;
 *       }
 * 
 *       // allocate a new chunk 
 *       
 *       new_chunk = (stack_chunk_header *) malloc( (size_of_chunk + CHUNK_HEADER_SIZE) * sizeof (int) );
 * 
 *       // make sure that we got a chunk 
 *       
 *       if (new_chunk == NULL)
 *       {
 * 	   // oh dear - no more memory available 
 * 	  
 * 	   abort();
 * 	 }
 * 
 *       // fill in the first two fields of the chunk header 
 *       
 *       new_chunk->size_of_chunk_in_words = size_of_chunk;
 *       new_chunk->ptr_to_next_chunk      = NULL;
 *       new_chunk->ptr_to_prev_chunk      = current_chunk;
 *     }
 *   else 
 *     {
 *       // get the size of the next free chunk 
 *       
 *       size_of_chunk = next_free_chunk->size_of_chunk_in_words;
 * 
 *       // ensure that the chunk will be of sufficient size 
 *       
 *       if (size_of_chunk < num_words_required)
 * 	   {
 * 	     // if not then create a new chunk
 * 	     //
 * 	     // XXX
 * 	     // we really ought to search the
 * 	     // free chunk list for another chunk
 * 	     // of sufficient size before allocating
 * 	     // a new one.
 * 	  
 * 	     size_of_chunk = num_words_required;
 * 	  
 * 	     new_chunk = (stack_chunk_header *) malloc( (size_of_chunk + CHUNK_HEADER_SIZE) * sizeof (int) );
 * 
 * 	     // make sure that the malloc succeded 
 * 	  
 * 	     if (new_chunk == NULL)
 * 	       {
 * 	         // oh dear - out of memory 
 * 	      
 * 	         abort();
 * 	       }
 * 
 * 	     // store the size of this chunk 
 * 	  
 * 	     new_chunk->size_of_chunk_in_words  = size_of_chunk;
 * 
 * 	     // insert the new chunk into the chunk list 
 * 	  
 * 	           new_chunk->ptr_to_next_chunk = next_free_chunk;
 * 	     next_free_chunk->ptr_to_prev_chunk = new_chunk;
 * 	       current_chunk->ptr_to_next_chunk = new_chunk;
 * 	           new_chunk->ptr_to_prev_chunk = current_chunk;
 * 	   }
 *       else
 * 	   {
 * 	     // we can use the next available chunk 
 * 	  
 * 	     new_chunk = next_free_chunk;
 * 
 * 	     // advance free pointer to next chunk on list 
 * 	  
 * 	     next_free_chunk = next_free_chunk->ptr_to_next_chunk;
 * 	   }
 *      }
 * 
 *   // advance current chunk pointer 
 *   
 *   current_chunk = new_chunk;
 *   
 *   // fill in the rest of the chunk header 
 *   
 *   new_chunk->old_stack_pointer     = stack_pointer - num_words_on_stack;
 *   new_chunk->old_stack_end_pointer = stack_end_pointer;
 *   new_chunk->old_return_address    = return_address;
 * 
 *   // set up stack pointer 
 * 
 *   stack_pointer = ((unsigned int *)new_chunk) - CHUNK_HEADER_SIZE;
 * 
 *   // set up the stack end pointer 
 *   
 *   stack_end_pointer = stack_pointer - size_of_chunk + STACK_GUARD;
 * 
 *   // advance stack pointer by the number of words already
 *   // pushed onto the stack by the entry code for the current
 *   // function
 *      
 *   stack_pointer += num_words_on_stack;
 * 
 *   // and replace return address by the address of our chunk free routine 
 *   
 *   return_address = free_stack_chunk;
 * 
 *   return;
 *   
 * } // get_new_stack_chunk 
 */

/*}}}*/
/*{{{  routine_entry() */

/*
 * emit code to handle entry to a routine
 * this routine is being passed 'num_args' arguments
 */

static void
routine_entry( int32 num_args )
{
  int32	n        = 0;
  int32	mask     = 0;
  int32	fmask    = 0;
  int32	maskarg  = 0;


  /* initialise available address registers */
  
  peep_init_addr_regs( usedmaskvec.map[ 0 ] );

  if (!new_stubs && in_stubs)
    {      
      /* stub routines have no entry code */
  
      saved_ivars = 0;
      saved_fvars = 0;
      saved_args  = 0;
      saved_frame = 0;
      stack_move  = 0;
      stack_offset = 0;
      
      return;
    }
  
  /* check the number of arguments passed to this function */
  
  if (num_args < 0)
    {
      syserr( syserr_enter, num_args );

      num_args = 0;
    }

  /*
   * n 		- is the number of arguments we have in registers
   * mask 	- is a bit mask of registers used by this function which MUST be saved before use
   * maskarg	- is a bit mask of the argument registers that must be saved (since their address will be used)
   */

  n = (num_args <= NARGREGS) ? num_args : NARGREGS;

  /*
   * note that the compiler is paranoid - even if only
   * one argument has its address taken then it will ask
   * us to save all of the arguments on the stack - such is life
   */
  
  mask = usedmaskvec.map[ 0 ] & M_VARREGS;

  if (procflags & BLKCALL
#ifdef TARGET_HAS_DEBUGGER
      || usrdbg( DBG_ANY )
#endif
      || usedmaskvec.map[ 0 ] & regbit( R_LR )
    )
    {
      /*
       * if we are going to call another procedure then we must
       * save the link register
       *
       * XXX - should I check against BLK2CALL aswell ?
       */
      
      mask |= regbit( R_LR );
    }

  /* remember FP variable registers */
  
  fmask = usedmaskvec.map[ 0 ] & M_FVARREGS;

#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
  mask &= ~fmask;		/* do not save integer vars if they are going to be saved as FP vars */  
#endif
  
  if (procflags & PROC_ARGPUSH)
    {
#if NFLTARGREGS > 0
      int32	fn;
#endif
      
      /*
       * XXX
       *
       * since we do not know if an argument is a
       * floating point or integer argument, we have
       * to assume the worst case for both types.
       *
       * Note, however that since we share integer and
       * floating point argument registers, we only need
       * to save the intersection of masks for the two
       * types.
       */

      maskarg  = regbit( R_A1 + n ) - regbit( R_A1 );

#if NFLTARGREGS > 0
      fn = (((n <= NFLTARGREGS) ? n : NFLTARGREGS) + sizeof_float / sizeof_int ) / (sizeof_double / sizeof_int);
      
      maskarg |= regbit( R_FA1 + fn ) - regbit( R_FA1 );
#endif
      
      /*
       * XXX - ignore registers in the regmaskvec as these are the
       * registers USED or CORRUPTED by the function, and they may not include
       * all of the registers used to PASS arguments to the function
       */ 
    }

#if defined TARGET_HAS_DEBUGGER
  if (usrdbg( DBG_ANY ))
    {
      /*
       * save all registers since debugger wants to
       * be able to grab initial values for arguments
       * even after the function has started executing
       */
      
      maskarg |= regbit( R_A1 + NARGREGS ) - regbit( R_A1 );
      mask    |= M_VARREGS;
      fmask   |= M_FVARREGS;
    }
#endif /* TARGET_HAS_DEBUGGER */
  
  /* peepf( "num_args = %d, register masks: used = %x, var = %x, FP var = %x, args = %x",
       num_args, usedmaskvec.map[ 0 ], mask, fmask, maskarg ); */
  
  /*
   * The return label is created here, but it is not set until its first use.
   * When it is used then routine_exit code is generated and from then on all
   * return statements will branch to that label.  If the function does not
   * return then the return code will never be generated!
   */
		    
  returnlab = nextlabel();
  
  /*
   * at the start of a function that stack is set correctly
   * and there are no local variables
   */
  
  stack_move   = 0;
  stack_offset = 0;

  /*
   * this should not be necessary, but let's be paranoid ....
   */
  
  flush_peepholer( DBG( "start of function" ) );
  
  /*
   * If necessary, push arguments onto stack.
   * We do not need to keep the stack double word
   * aligned, so do not bother.
   */

  if (maskarg)
    {
      save_regs( maskarg, FALSE );
    }

  /*
   * next save the frame pointer (if necessary)
   */

  if (num_args > NARGREGS ||		/* if we have arguments passed on the stack 	 */
      maskarg             ||		/* or we have had to save arguments on the stack */
      backtrace_enabled    )		/* or we are supporting back traces		 */
    {
      /**/                              /* then we must save old frame pointer */
      
      ipush( R_FP );

      /* set new frame pointer to be current stack position */

      move_register( R_SP, R_FP, FALSE );

      /* make a note that we have saved FP */

      saved_frame = TRUE;
    }
  else
    {
      saved_frame = FALSE;
    }

  /*
   * now check to see if we have sufficient stack remaining
   * or if we need to allocate a new stack hunk
   */
  
  if (!no_stack_checks)
    {
      LabelNumber *	stack_ok = NULL;
      int32		require;
      int32		already_placed;
      

      /* calculate number of words already placed upon stack */
      
      already_placed = bitcount( maskarg ) + (saved_frame ? 1 : 0);

      /* calculate the maximum number of words of stack we will need in the function we are entering */
      
      /*
       * XXX for some reason the front end does always get the greatest depth correct
       * it seems to forget about saving registers (especially the link register) onto the stack
       */
      
      require = greatest_stackdepth / sizeof_int + bitcount( mask ) + bitcount( fmask ) * 2;

      /* fprintf( asmstream, "; words required = %ld, already placed = %ld, greatest depth = %ld\n",
	      require, already_placed, greatest_stackdepth ); */
      
      /*
       * note that we have optimised leaf procedures by assuming that we
       * will always have 25 words of stack available to us
       */
      
      if (require > 25 || ((procflags & BLKCALL) && require > 0))
	{
	  RealRegister	cmp_reg = R_TMP1;

	  
	  /*
	   * place stack pointer less number of words of stack we will require,
	   * into temporary register
	   */

	  if (!fits_in_8_bits_signed( require ))
	    {
	      load_integer( R_TMP2, require, FALSE );

	      out_triadic_op( OP_SUBI3, cmp_reg, R_SP, R_TMP2, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			     examines2( cmp_reg, R_SP ),
			     alters2( R_TMP1, RR_ST ) );
	    }
	  else if (require > 25)
	    {
	      out_triadic_op( OP_SUBI3, cmp_reg, R_SP, require,
			     ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			     examines1( R_SP ),
			     alters2( cmp_reg, RR_ST ) );
	    }
	  else
	    {
	      /*
	       * XXX - special optimisation - with less than 26 words of stack
	       * being used just check the stack pointer against the stack end
	       * pointer.  This saves an entire instruction.  We have a excess
	       * of 64 words on the stack end pointer 25 of which are used here,
	       * and 25 are used in leaf functions.
	       */
	      
	      cmp_reg = R_SP;	      
	    }
	  
	  /* if this is still above the stack end pointer ... */
	  
	  out_diadic_op( OP_CMPI, ADDR_MODE_REGISTER, R_SE, cmp_reg,
			examines2( cmp_reg, R_SE ),
			alters1( RR_ST ) );
	  
	  if (!few_modules)
	    {
	      /* ... then skip the next few instructions */

	      /*
	       * with the few_modules model we assume that we can use a
	       * conditional LAJ which only hsa a 16 bit displacement
	       */
	      
	      stack_ok = nextlabel();

	      /*
	       * NB must use an UNSIGNED compare as stack pointer could be
	       * on global bus (>= 0x80000000) and stack end could be on
	       * global bus (<= 0x7fffffff) ...
	       */
	      
	      conditional_branch_to( Q_LO, stack_ok );
	    }	  
	  
	  conditional_load( Q_HS, ADDR_MODE_REGISTER, R_TMP3, R_LR,
			   examines1( R_LR ),
			   alters1( R_TMP3 ) );
	  
	  /* place number of words we require in R_TMP2 (if not already placed there) */
	  
	  if (fits_in_8_bits_signed( require ))
	    {
	      conditional_load( Q_HS, ADDR_MODE_IMMEDIATE, R_TMP2, require,
			       examines0(), alters1( R_TMP2 ) );
	    }

	  /* and place number of words we have added to the stack into R_TMP1 */

	  conditional_load( Q_HS, ADDR_MODE_IMMEDIATE, R_TMP1, already_placed,
			   examines0(), alters1( R_TMP1 ) );

	  /* and call get_new_stack_chunk function */

	  if (few_modules)
	    {
	      peep_xref( X_PCreloc2, stackoverflow );
	      
	      outdelsymref( (OP_LAJc << 24) | 0x1U << 21 | C_FROMQ( Q_HS ) << 16 | (-3 & 0x0000ffffU),
			   stackoverflow,
			   examines0(),
			   alters2( R_LR, RR_PC ) );
      
	      flush_peepholer( DBG( "stack size checking code" ) );
	    }
	  else
	    {
	      call( stackoverflow, FALSE );

	      flush_peepholer( DBG( "stack size checking code" ) );
	  
	      /* on return from this function R_SP, R_LR and R_SE will have been set up */

	      setlabel( stack_ok );
	    }	  
	}
      else if (already_placed > 0 && (procflags & BLKCALL))
	{
	  /*
	   * if we have just put some words on the stack and we are going
	   * to call another function, and we do not require any more stack space
	   * ourselves, we still have to check to see if we have used up the
	   * current stack chunk ....
	   */

	  /* check stack pointer in case we have run out of space */
	  
	  out_diadic_op( OP_CMPI, ADDR_MODE_REGISTER, R_SE, R_SP,
			examines2( R_SE, R_SP ),
			alters1( RR_ST ) );

	  if (!few_modules)
	    {
	      stack_ok = nextlabel();

	      /* if we are OK skip the next bit */
	  
	      conditional_branch_to( Q_LO, stack_ok );
	    }

	  /* save the return address */
	  
	  conditional_load( Q_HS, ADDR_MODE_REGISTER, R_TMP3, R_LR,
			   examines1( R_LR ),
			   alters1( R_TMP3 ) );
	  
	  /* we do not require any more words */
	  
	  conditional_load( Q_HS, ADDR_MODE_IMMEDIATE, R_TMP2, 0,
			   examines0(),
			   alters1( R_TMP2 ) );

	  /* but we have already used some ... */
	  
	  conditional_load( Q_HS, ADDR_MODE_IMMEDIATE, R_TMP1, already_placed,
			   examines0(),
			   alters1( R_TMP1 ) );

	  /* call get_new_stack_chunk function */

	  if (few_modules)
	    {
	      peep_xref( X_PCreloc2, stackoverflow );
	      
	      outdelsymref( (OP_LAJc << 24) | 0x1U << 21 | C_FROMQ( Q_HS ) << 16 | (-3 & 0x0000ffffU),
			   stackoverflow,
			   examines0(),
			   alters2( R_LR, RR_PC ) );
      
	      flush_peepholer( DBG( "stack size checking code" ) );
	    }
	  else
	    {
	      call( stackoverflow, FALSE );

	      flush_peepholer( DBG( "stack size checking code" ) );
	  
	      /* on return from this function R_SP, R_LR and R_SE will have been set up */

	      setlabel( stack_ok );
	    }	  
	}
    }
  
  /*
   * now save any register we are going to corrupt in this function
   * this may include the link register
   */

  if (mask)
    {
      save_regs( mask, FALSE );
    }
  
  if (fmask)
    {
      save_regs( fmask, TRUE );
    }
  
  /*
   * OK we are done - set up the remaining statics used by routine_exit()
   */

  saved_ivars = mask;
  saved_fvars = fmask;
  saved_args  = maskarg;

#if defined TARGET_HAS_DEBUGGER
  if (usrdbg( DBG_PROC ) && !in_stubs)
    {
      do_notify_entry();
    }
#endif
  
  return;
  
} /* routine_entry */

/*}}}*/
#ifdef TARGET_HAS_DEBUGGER
/*{{{  do_notify_leave() */

static void
do_notify_leave( void )
{
  extern Symstr *	current_proc;
  extern LabelNumber *	proc_label;


  if (debugging( DEBUG_Q ))
    cc_msg( "debugger: generating _notify_return for: %s\n", symname_( current_proc ) );
  
  flush_peepholer( DBG( "notify_leave" ) );

  /*
   * At this point the rotuine is about to exit.
   * The return code, (if any) is in R_A1, (or the
   * area of memory pointed to by R_A1).  The return
   * will either be a BuD R_LR or a BRD <offset> if a
   * tailcall is taking place.
   *
   * We can call our _notif_return function
   * directly as R_LR will have been cached upon the stack,
   * and no variable registers have significant values
   * left in them.
   */

  /* second argument: the return value */
  
  move_register( R_A1, R_A1 + 1, FALSE ); /* XXX - potential bug, if return value is a double */

  /* first argument: the address of the ProcInfo structure as a word pointer */
  
  /*
   * put PC of next instruction into R_LR
   *
   * (There is no need to save R_LR, as it will
   *  have been pushed onto the stack)
   */

  outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

  /*
   * add in offset of the proc structure
   *
   * NB/ beware of assumption that proc structure is within 16
   * (signed) bits of this instruction - this may not be true
   */
  
  peep_fref( proc_label, LABREF_LIT16 ); 

  out_diadic_op( OP_LDI, ADDR_MODE_IMMEDIATE, R_A1, -3,
		examines0(),
		alters2( R_A1, RR_ST ) );

  /* add in value in link register */
  
  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, R_A1, R_LR,
		examines1( R_LR ),
		alters2( R_A1, RR_ST ) );

  nop( FALSE );
  
  flush_peepholer( DBG( "protect LAJ again" ) );
  
  /* finally call the _notify_return function */
  
  call( _notify_return, FALSE );

  /*
   * upon return from this function R_A1 will have been restored
   * to its proper value
   */
  
  return;
  
} /* do_notifty_leave */

/*}}}*/
#endif /* TARGET_HAS_DEBUGGER */
/*{{{  routine_exit() */

static void
routine_exit( bool tailcall )
{
  RealRegister	link_reg = R_LR;


  if (saved_link_reg != GAP)
    {
      /* this is a special optimisation for data initialisation */
      
      link_reg       = saved_link_reg;
      saved_link_reg = GAP;
    }
  
  /*
   * ensure that the stack pointer is back at start of local variables
   * this should remove local variables and effectively resets any
   * stack manipulations
   */

  /* The correct_stack() has been put here as a bugfix. cf genmodule() in linker/readfile.c */
  
  correct_stack( TRUE );
  
  if (stack_offset >= sizeof_int)
    {
      flush_pending_pushes();
      
      stack_offset /= sizeof_int;

      if (fits_in_16_bits_signed( stack_offset ))
	{
	  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_SP, stack_offset,
			examines1( R_SP ),
			alters2( R_SP, RR_ST ) );
	}
      else
	{
	  load_integer( R_TMP1, stack_offset, FALSE );
	  
	  out_triadic_op( OP_ADDI3, R_SP, R_SP, R_TMP1, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			examines2( R_SP, R_TMP1 ),
			alters2( R_SP, RR_ST ) );
	}

      stack_offset = 0;
    }

  stack_move = 0;
  
  /*
   * if the return label has not yet been set
   * and we are not doing a tail call then
   * set the label
   */
  
  if (!tailcall && !lab_isset_( returnlab ))
    {
      /* note to NC from NC - this flush is necessary! */
#if 1
      /* experimental - eg for int func( int arg ){ return arg * arg; } */
      
      if (returnlab->u.frefs != NULL || peep_refs_label( returnlab ))
#endif
	{
	  flush_peepholer( DBG( "routine_exit" ) );

	  setlabel( returnlab );
	}
    }
  
#if defined TARGET_HAS_DEBUGGER
  if (usrdbg( DBG_PROC ) && !in_stubs)
    {
      do_notify_leave();
    }
#endif
  
  /*
   * if registers were saved onto the stack then pop them back
   */

  if (saved_fvars)
    {
      restore_regs( saved_fvars, TRUE );
    }

  if (saved_ivars)
    {
      if (!no_peepholing && !tailcall && (saved_ivars & regbit( R_LR )))
	{
	  int32		pos;
	  int32		count;

	  
	  /*
	   * special optimisation
	   *
	   * We are about to do a "BuD R_LR", so try
	   * popping this off the stack first, in the hopes that
	   * the BuD can be pushed back over the remaining pops
	   */

	  pos   = 1;
	  count = 0;

	  /* count number of registers on stack "above" the link register */
	  
	  while (pos < R_LR)
	    {
	      if (saved_ivars & regbit( pos ))
		++count;
	      
	      ++pos;
	    }

	  if (count > 0)
	    {
	      int32	TMP;
	      
	      
	      /* pull the link register off the stack */

	      if (count == 1)
		{
		  /*
		   * another piece of optimisation here
		   * If we pull the link register into one of R0 - R7
		   * (and the increment is 1)
		   * then we may be able to combine it with the previous
		   * instruction, making a parallel load !
		   * (But only a temporary register that we have already dirtied)
		   */
		  
		  if (usedmaskvec.map[ 0 ] & regbit( R_A1 + 1 ))
		    link_reg = R_A1 + 1;
		  else if (usedmaskvec.map[ 0 ] & regbit( R_A1 + 2 ))
		    link_reg = R_A1 + 2;
		  else if (usedmaskvec.map[ 0 ] & regbit( R_A1 + 3 ))
		    link_reg = R_A1 + 3;
		  else if (usedmaskvec.map[ 0 ] & regbit( R_FT1 ))
		    link_reg = R_FT1;					/* currently R6 */
		  else if (usedmaskvec.map[ 0 ] & regbit( R_FT1 + 1 ))
		    link_reg = R_FT1 + 1;	
		}
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, link_reg,
			    build_indirect( INDIRECT_PRE_ADD, R_SP, count ),
			    examines1( R_SP ),
			    alters2( link_reg, RR_ST ) );

	      /* preserve a copy of saved_ivars before altering it */
	      
	      TMP = saved_ivars;
	      
	      /* remove the link register (and all registers above it) from the mask of registers to pop */
	  
	      saved_ivars &= (regbit( R_LR ) - 1);
	      
	      /* pop all the other variable registers off the stack */
	  
	      restore_regs( saved_ivars, FALSE );

	      /* restore saved_ivars - in case it is used again */
	  
	      saved_ivars = TMP;

	      /* add one to stack pointer to skip past link register (which is still on the stack) */

	      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_SP, 1,
			    examines1( R_SP ),
			    alters2( R_SP, RR_ST ) );
	      
	      /* get any registers above R_LR */
	      
	      TMP &= ~(regbit( R_LR + 1 ) - 1);

	      /* if there are any then restore those as well */
	      
	      if (TMP)
		{
		  restore_regs( TMP, FALSE );
		}
	    }
	  else
	    {
	      restore_regs( saved_ivars, FALSE );
	    }
	}
      else
	{
	  restore_regs( saved_ivars, FALSE );
	}
    }

  /*
   * at this point SP == FP unless we ahave split the stack
   * across two stack chunks, in which case we MUST restore
   * the frame pointer relative to FP
   */

  /*
   * if the previous frame pointer
   * was saved on the stack then pop it
   */

  if (saved_frame)
    {
      /*
       * special optimisation -
       *
       * we must reduce the stack by an extra word to account
       * for the frame pointer, BUT, if we did not push any
       * arguments onto the stack then we will not be able to
       * include this word in simulated pop below.  Instead
       * we include the reduction here, and the peepholer
       * will try to merge the addition into the restoring
       * of the saved temporary registers above
       *
       * XXX - this is now disallowed because it means that
       * the stack pointer is incremented before the FP
       * is read off the stack, and so an interrupt routine
       * could, conceivably, come in and corrupt the FP on
       * the stack.  Hence we do the load before we do the add
       */
      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_FP, build_indirect( INDIRECT_REL, R_FP, 0 ),
		    examines1( R_FP ),
		    alters2( R_FP, RR_ST ) );
      
      if (saved_args == 0)
	{
	  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_SP, 1,
			examines1( R_SP ),
			alters2( R_SP, RR_ST ) );
	}
    }

  /*
   * if arguments were pushed onto the stack then adjust
   * the stack pointer, (do not bother to pop)
   */

  if (saved_args)
    {
      out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_SP, bitcount( saved_args ) + (saved_frame ? 1 : 0),
		     examines1( R_SP ),
		     alters2( R_SP, RR_ST ) );
    }
  
  /*
   * if we are doing a tailcall then the next instruction output
   * will be a branch to the next function, otherwise we have to
   * return through the link (by doing a conditional branch)
   */

  if (!tailcall)
    {
      /* this is a conditional, delayed branch, register relative */
      
      outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( Q_AL ) << 16 | hardware_register( link_reg ),
		 examines1( link_reg ),
		 alters1( RR_PC ) );
      /*
       * make sure that all of the op codes have been emitted
       */
  
      flush_peepholer( DBG( "end of function" ) );
    }

  /*
   * finished
   */

  return;
    
} /* routine_exit */

/*}}}*/

/*}}}*/
/*{{{  local_base() */

/*
 * this function returns the register, relative to which
 * the given symbol may be accessed
 */

RealRegister
local_base( Binder * b )
{
  int32	addr = bindaddr_( b );

  
  /*
   * arguments are accessed relative to frame pointer
   * locals    are accessed relative to stack pointer
   */
  
  switch (addr & BINDADDR_MASK)
    {
    default:
      syserr( syserr_local_base, (long)addr );
      
    case BINDADDR_ARG:
      return R_FP;
      
    case BINDADDR_LOC:
      return R_SP;
    }
  
} /* local_base */

/*}}}*/
/*{{{  local_address() */

/*
 * This function gives the offset of the
 * symbol passed in from the
 * register given by the 'local_base()'
 */
  
int32
local_address( Binder * b )
{
  int32 	addr = bindaddr_( b );
  int32 	off  = addr & ~BINDADDR_MASK;

  
  switch (addr & BINDADDR_MASK)
    {
    default:
      syserr( syserr_local_address, (long)addr );

      /* drop through */
      
    case BINDADDR_LOC:	/* relative to stack pointer */

      /*
       * The values look like this:
       *
       *           decreasing memory -->
       * ___________________________________________________ _ _
       *  |           |       |                           |
       *  | arguments | saved | locals    (local)         |
       *  |           | regs  |           (sought)        |
       *  |           |       |               |           |
       * --------------------------------------------------- - - 
       *            ^                                    ^
       *            |                                    |
       *            FP                                   SP
       *                       <------stack offset-------> <---stack move--->
       *                       <----off------>
       *
       */
       
      correct_stack( TRUE );

      return (stack_offset - off);

    case BINDADDR_ARG:	/* relative to frame pointer */

      /*
       * The stack can look like this ... ((saved_args != 0))
       *
       *           decreasing memory -->
       * __________________________________________ _ _
       *  |     |   |     |     |   |     |     |
       *  | arg |...| arg | arg |...| arg | old |
       *  |  n  |   |  5  |  4  |   |  1  | FP  |
       *  |     |   |     |     |   |     |     |
       * ------------------------------------------ - - 
       *                                     ^                            
       *                                     |                            
       *                                     FP
       *
       * or like this ... ((saved_args == 0))
       *
       *           decreasing memory -->
       * __________________________________________ _ _
       *  |     |   |     |     |
       *  | arg |...| arg | old |
       *  |  n  |   |  5  | FP  |
       *  |     |   |     |     |
       * ------------------------------------------ - - 
       *                     ^                            
       *                     |                            
       *                     FP                           
       *
       * offset is the (number of the argument minus 1) multiplied by sizeof_int
       *
       * saved_frame is always True if this function is called
       */

      if (saved_args == 0)
	{
	  /* arguments 1 - 4 held in registers */
	    	
	  return (off + (1 - NARGREGS) * sizeof_int );
	}
      else
	{
	  return (off + sizeof_int);
	}
    }
  
} /* local_address */

/*}}}*/
/*{{{  setlabel() */

/*
 * Although the idea of setlabel is machine independent, it stays here
 * because it back-patches code.  In the long term setlabel should be
 * in codebuf.c and call a machine dependent backpatch routine.
 */

void
setlabel( LabelNumber * l )
{
  List *	p;
  

  if (lab_isset_( l ))
    {
      syserr( gen_already_set_label );
    }
  
  /* do not flush the peepholer - this is done elsewhere */
  
  if (asmstream)
    {
      asm_lablist = mkLabList( asm_lablist, l );
    }
  
  /* resolve all the forward references to this label */

  p = l->u.frefs;	/* XXX should this be before the mkLabList() call ??? */

  while (p != NULL)
    {
      int32		v = car_( p );
      int32 		q = (v & 0x00ffffffU);   /* BYTE address */
      int32 		w = code_inst_( q );
      unsigned32	d;


      /*
       * v	- the type of forward reference
       * q	- the address (in code space) of the instruction to be patched
       * w	- the instruction to be patched
       * d	- work variable
       */

      switch (v & 0xff000000U)
        {
	case LABREF_OFF24:     /* e.g. forw. BR ref. */
	  
	  /* note assumption that sizeof_int == 4 */

	  d = (codep - q  >> 2) + mask_and_sign_extend_word( w, 0x00ffffffU );

	  if (!fits_in_24_bits_signed( d ))
	    syserr( syserr_displacement, (long)d );

	  if (d == 0 && !no_peepholing)
	    {
	      /* we have a branch to the next instruction ! */

	      w = OP_NOP << 23;
	      
	      peepf( "converted branch to next instruction into NOP" );

	      ++peep_transformed;
	    }
	  else
	    {
	      w = (w & 0xFF000000U) | (d & 0x00FFFFFFU);
	    }

	  break;

	case LABREF_OFF16:     /* e.g. forw. Bc ref. */

	  /* note assumption that sizeof_byte * 4 == sizeof_int */
	  
	  d = (codep - q  >> 2) + mask_and_sign_extend_word( w, 0xffff );

	  if (!fits_in_16_bits_signed( d ))
	    syserr( syserr_displacement, (long)d );
	  
	  w = (w & 0xffff0000U) | (d & 0xffff);

	  break;
	  
	case LABREF_LIT16:     /* e.g. forw. LDI ref. */

	  /* note assumption that sizeof_byte * 4 == sizeof_int */
	  
	  d = (codep - q  >> 2) + mask_and_sign_extend_word( w, 0xffff );

	  if (!fits_in_16_bits_signed( d ))
	    syserr( syserr_displacement, (long)d );
	  
	  w = (w & 0xffff0000U) | (d & 0xffff);
	  
	  break;
	  
	case LABREF_LIT8:     /* e.g. forw. ADDI3 ref. */

	  /* note assumption that sizeof_byte * 4 == sizeof_int */
	  
	  d = ((codep - q  >> 2) >> 16) + mask_and_sign_extend_word( w, 0xff );

	  if (!fits_in_8_bits_signed( d ))
	    syserr( syserr_displacement, (long)d );
	  
	  w = (w & 0xffffff00U) | (d & 0xff);
	  
	  break;
	  
	case LABREF_ABS32:     /* e.g. BXX */
	  {
	    CodeXref *	z = (CodeXref *) (((List3 *)p)->csr);

	    
	    /* note assumption that sizeof_byte * 4 == sizeof_int */
	  
	    z->codexrlitoff += (codebase + codep) >> 2;
	    
	    p = discard3( p );

	    code_inst_( q ) = z->codexrlitoff;
	    
	    continue;
	  }
	  
	default:
	  syserr( syserr_labref, (long)v );
        }

      code_inst_( q ) = w;
      
      p = discard2( p );
    }

  /* and set the label */

  lab_setloc_( l, codep | 0x80000000U ); /* cheapo union checker for ->frefs */

  return;
  
} /* setlabel */

/*}}}*/
/*{{{  get_free_register() */

/*
 * Locates a free register from the list of hardware
 * registers supplied.
 * The list is terminated by a register of value GAP.
 * If no free register can be found GAP is returned.
 */

static RealRegister
get_free_register(
		  RealRegister	reg,
		  ... 		)
{
  int32		map  = usedmaskvec.map[ 0 ];	/* XXX - assumes less than 32 registers */
  va_list	args;

  
  /*
   * XXX - there is a potential bug here
   *
   * If this function selects an argument register as being free
   * because the function does not use that argument register,
   * but the Norcroft front end, with its global knowledge of the
   * registers corrupted by a function, thinks that the argument
   * register is not corrupted, then it might be tempted to use
   * the argument register as a variable register !
   *
   * NB/ This will not work for variable registers!
   *
   * The augmentation below is an attempt to fix this.
   */

  va_start( args, reg );

  while (reg != GAP)
    {
      /* see if the register is used by the function */
      
      if ((map & regbit( reg )) == 0)
	{
	  /* let the front end know that we are going to corrupt this register */

	  augment_RealRegSet( &regmaskvec, reg );

	  /* paranoia alert - make sure that this register has not been push cached */

	  maybe_flush_pending_push( reg );
	  
	  break;	  
	}

      /* get the next register in the list */
      
      reg = va_arg( args, RealRegister );
    }
  
  /* tidy up */
  
  va_end( args );

  /* return located register */
  
  return reg;
  	    
} /* get_free_register */

/*}}}*/
/*{{{  clear_memory() */

  
/*
 * set to zero the 'length' "bytes" of memory pointed to by 'start'
 */

static void
clear_memory(
	     RealRegister	start,			/* pointer to memory to be set to 0 */
	     int32		length,			/* number of bytes to clear, (a word multiple) */
	     int32		can_corrupt_start )	/* TRUE if the start register can be corrupted */
{
  RealRegister	dst;
  
  
  if (length < 1 || length & (sizeof_int - 1))
    {
      syserr( syserr_bad_block_length, length );
      
      return;
    }

  /*
   * XXX
   *
   * NB/ we are assuming that the address given to us
   * will be word aligned - THIS HAD BETTER BE TRUE!
   *
   */

  if (!can_corrupt_start || !is_address_register( hardware_register( start ) ))
    {
      /* use temporary address register */

      dst = peep_get_free_addr_reg( GAP );
    }
  else
    {
      dst = start;
    }

  /*
   * round length down to nearest word
   */
  
  length /= sizeof_int;
  
  /*
   * store temporary into block
   */

  if (length == 1)		/* XXX - can this ever happen ? */
    {
      (void) convert_to_word_alignment( start, dst, 0, NULL );
      
#ifdef STIK_NOW_WORKS_ON_HARDWARE     
      /* XXX - this addressing mode is actually ADDR_MODE_INDIRECT ! */
      
      out_diadic_op( OP_STIK, ADDR_MODE_IMMEDIATE, 0, build_indirect( INDIRECT_PRE_ADD_IR0, dst, 0 ),
		    examines2( dst, R_BASE ),
		    alters0() );
#else
      load_integer( R_TMP1, 0, FALSE );
      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_ADD_IR0, dst, 0 ),
		    examines3( dst, R_BASE, R_TMP1 ),
		    alters0() );
      
#endif /* STIK_NOW_WORKS_ON_HARDWARE */
    }
  else if (length == 2)		/* XXX - can this ever happen ? */
    {
      (void) convert_to_word_alignment( start, dst, 0, NULL );
      
#ifdef STIK_NOW_WORKS_ON_HARDWARE     
      /* XXX - this addressing mode is actually ADDR_MODE_INDIRECT ! */
      
      out_diadic_op( OP_STIK, ADDR_MODE_IMMEDIATE, 0, build_indirect( INDIRECT_PRE_INCR_IR0, dst, 0 ),
		    examines2( dst, R_BASE ),
		    alters1( dst ) );

      /* XXX - this addressing mode is actually ADDR_MODE_INDIRECT ! */
      
      out_diadic_op( OP_STIK, ADDR_MODE_IMMEDIATE, 0, build_indirect( INDIRECT_PRE_ADD, dst, 1 ),
		    examines1( dst ),
		    alters0() );
#else
      load_integer( R_TMP1, 0, FALSE );
      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_INCR_IR0, dst, 0 ),
		    examines3( dst, R_BASE, R_TMP1 ),
		    alters1( dst ) );

      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_ADD, dst, 1 ),
		    examines2( dst, R_TMP1 ),
		    alters0() );
      
#endif /* STIK_NOW_WORKS_ON_HARDWARE */
      
      peep_note_addr_reg_loaded( dst, start, 0, TRUE );
    }
  else
    {
      LabelNumber *	l = nextlabel();
#ifndef STIK_NOW_WORKS_ON_HARDWARE
      RealRegister	zero;
#endif
      

      /* load repeat count */

      load_integer( RR_RC, length - 1, FALSE );

      flush_peepholer( DBG( "force RC flush" ) );

#ifndef STIK_NOW_WORKS_ON_HARDWARE
#ifdef TARGET_R0_ALWAYS_ZERO
      zero = RR_R0;
#else
      zero = get_free_register( RR_R0,  RR_R1, RR_R2, RR_R3, RR_R6, RR_R7, RR_R10, GAP );

      if (zero == GAP)
	{
	  zero = R_LR;

	  if ((saved_ivars & regbit( R_LR )) == 0 ||
	      usedmaskvec.map[ 0 ] & regbit( R_LR ))	/* link register being used as temporary */
	    ipush( R_LR );
	}
#endif
#endif /* !STIK_NOW_WROKS_ON_HARDWARE */
      
      /* convert destintation pointer to word offset */

      convert_to_word_address( start, dst );

      /* NB/ do NOT use RPTS as this locks out interrupts */

      outdellabref( (OP_RPTBD << 24) | (-3 & 0x00FFFFFFU),
		   l,
		   LABREF_OFF24,
		   examines1( RR_PC ),
		   alters4( RR_RC, RR_RS, RR_RE, RR_PC ) );
      
      setlabel( l );

#ifdef STIK_NOW_WORKS_ON_HARDWARE
      /* XXX - this addressing mode is actually ADDR_MODE_INDIRECT ! */
      
      out_diadic_op( OP_STIK, ADDR_MODE_IMMEDIATE, 0, build_indirect( INDIRECT_POST_INCR, dst, 1 ),
		    examines1( dst ),
		    alters1( dst ) );
#else
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, zero, build_indirect( INDIRECT_POST_INCR, dst, 1 ),
		    examines2( dst, zero ),
		    alters1( dst ) );
      
#endif /* STIK_NOW_WORKS_ON_HARDWARE */
      
      flush_peepholer( DBG( "end of repeat block" ) );

#ifndef STIK_NOW_WORKS_ON_HARDWARE
      if (zero == R_LR &&
	  (((saved_ivars & regbit( R_LR )) == 0) ||
	   usedmaskvec.map[ 0 ] & regbit( R_LR )) )	/* link register being used as temporary */
	{
	  ipop( R_LR );
	}      
#endif
      peep_note_addr_reg_loaded( dst, start, length, TRUE );
    }
  
  return;
      
} /* clear_memory */

/*}}}*/
/*{{{  copy_memory() */

/*
 * copy the 'length' "bytes" of memory pointed to by 'src' to the area pointed to by 'dst'
 * Both pointers are word aligned, and 'length' is a whole number of words
 */

static void
copy_memory(
	    RealRegister	src,		/* register containing byte piinter to source */
	    RealRegister	dst,		/* register containing byte pointer to dest */
	    int32		length,			/* number of bytes to copy */
	    int32		can_corrupt_source,	/* non-zero if 'src' can be courrupted */
	    int32		can_corrupt_dest )	/* non-zero if 'dst' can be corrupted */
{
  RealRegister	source;
  RealRegister	dest;
  int		restore_ar    = FALSE;
  

  if (length < 4)
    syserr( gen_copy_too_small, length );

  if (length & (sizeof_int - 1))
    syserr( gen_copy_non_word_multiple, length );
      
  /*
   * get registers into address registers
   */
  
  if (!can_corrupt_source || !is_address_register( hardware_register( src ) ))
    {
      source = peep_get_free_addr_reg( GAP );
    }
  else
    {
      source = src;
    }
      
  if (!can_corrupt_dest || !is_address_register( hardware_register( dst ) ))
    {
      dest = peep_get_free_addr_reg( source );

      if (dest == GAP)
	{
	  restore_ar = TRUE;

	  dest = RR_AR3;

	  peep_corrupt_addr_reg( RR_AR3 );
	      
	  if (length > 2 * sizeof_int)
	    {
	      ipush( dest );
	    }
	  else
	    {
	      move_register( dest, R_TMP2, FALSE );
	    }
	}
    }
  else
    {
      dest = dst;
    }
      
  /*
   * copy whole number of words
   */

  if (length == sizeof_int)
    {
      (void) convert_to_word_alignment( src, source, 0, NULL );
      (void) convert_to_word_alignment( dst, dest,   0, NULL );
      
      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_ADD_IR0, source, 0 ),
		    examines2( source, R_BASE ),
		    alters2( R_TMP1, RR_ST ) );
      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_ADD_IR0, dest, 0 ),
		    examines3( dest, R_TMP1, R_BASE ),
		    alters0() );
    }
  else if (length == sizeof_int * 2)
    {
      RealRegister	TMP;
      

      (void) convert_to_word_alignment( src, source, 0, NULL );
      (void) convert_to_word_alignment( dst, dest,   0, NULL );
      
      /*
       * find a (temporary) register that can be used in a parallel op
       */
      
      TMP = get_free_register( RR_R0, RR_R1, RR_R2, RR_R3, RR_R6, RR_R7, GAP );

      if (TMP == GAP)
	{
	  /* get first word into TMP */
      
	  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_INCR_IR0, source, 0 ),
			examines2( source, R_BASE ),
			alters3( R_TMP1, RR_ST, source ) );

	  /* store first word */
	  
	  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_INCR_IR0, dest, 0 ),
			examines3( dest, R_TMP1, R_BASE ),
			alters1( dest ) );

	  /* do second word */
	  
	  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_ADD, source, 1 ),
			examines1( source ),
			alters2( R_TMP1, RR_ST ) );

	  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_PRE_ADD, dest, 1 ),
			examines2( dest, R_TMP1 ),
			alters0() );
	}
      else
	{
	  /* get first word into TMP */
      
	  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, TMP, build_indirect( INDIRECT_PRE_INCR_IR0, source, 0 ),
			examines2( source, R_BASE ),
			alters3( TMP, RR_ST, source ) );
	  
	  /*
	   * instruction is:
	   *
	   *    LDI  *<source>++(1), TMP
	   * || STI  TMP, *<dest>++(1)
	   *
	   */
	  
	  outinstr( B_1101 << 28 | B_1010 << 24 |
		   hardware_register( TMP ) << 22 | hardware_register( TMP ) << 16 |
		   build_parallel_indirect( INDIRECT_PRE_INCR_IR0, dest ) << 8 |
		   build_parallel_indirect( INDIRECT_PRE_ADD, source ),
		   examines4( source, TMP, dest, R_BASE ),
		   alters3(   source, TMP, dest ) );
	  
	  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, TMP, build_indirect( INDIRECT_PRE_ADD, dest, 1 ),
			examines2( dest, R_TMP1 ),
			alters0() );
	}

      peep_note_addr_reg_loaded( source, src, 0, TRUE );
      peep_note_addr_reg_loaded( dest,   dst, 0, TRUE );
    }
  else 
    {
      RealRegister	TMP;
      bool		must_save = FALSE;
      LabelNumber *	l;
      int32		off;
      

      /* get a label for the end of the repeat block */
      
      l = nextlabel();

      /*
       * load repeat count
       *
       * the -2 is because we will have already loaded one word, and the
       * RPTB instruction takes 'number of itterations - 1' as its count
       */
      
      load_integer( RR_RC, length / sizeof_int - 2, FALSE );
      
      /*
       * find a (temporary) register than can be used in a parallel op
       */
      
      TMP = get_free_register( RR_R0, RR_R1, RR_R2, RR_R3, RR_R6, RR_R7, GAP );
      
#ifdef TARGET_R0_ALWAYS_ZERO
      if (TMP == GAP)
	{
	  TMP = R_ZERO;
	}
#else
      if (TMP == GAP)
	{
	  must_save = TRUE;
	  TMP       = RR_R0;
	  
	  /*
	   * preserve TMP register
	   * This is not as expensive as it might seems,
	   * since the push can be combined with the load
	   * below to form a parallel instruction !
	   *
	   * Also we cannot just copy the register to a temporary register
	   * as all three are used by the RPTB instruction below
	   */

	  ipush( TMP );

	  /*
	   * NB/ we do not need to dpush() this as we are only doing integer
	   * loads and stores below, so we will not upset the exponent.
	   */
	}
#endif /* TARGET_R0_ALWAYS_ZERO */  

      /* prevent the RC loading from being moved over address calculation below */
      
      flush_peepholer( DBG( "copy_memory" ) );
      
      /* convert offsets to word pointers */

      convert_to_word_address( dst, dest );
      
      if (convert_to_word_alignment( src, source, 0, &off ))
	{
	  if (off != 0)
	    {
	      if (fits_in_8_bits_unsigned( off ))
		{
		  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, TMP,
				build_indirect( INDIRECT_PRE_INCR, source, off ),
				examines1( source ),
				alters3( TMP, RR_ST, source ) );
		}
	      else if (fits_in_8_bits_unsigned( -off ))
		{
		  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, TMP,
				build_indirect( INDIRECT_PRE_DECR, source, -off ),
				examines1( source ),
				alters3( TMP, RR_ST, source ) );
		}
	      else
		{
		  integer_immediate_op( OP_ADDI, OP_ADDI3, source, source, off, TRUE );

		  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, TMP, build_indirect( INDIRECT_REL, source, 0 ),
				examines1( source ),
				alters2( TMP, RR_ST ) );
		}
	    }
	  else
	    {
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, TMP, build_indirect( INDIRECT_REL, source, 0 ),
			    examines1( source ),
			    alters2( TMP, RR_ST ) );
	    }	  
	}
      else
	{
	  /* load first word */

	  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, TMP, build_indirect( INDIRECT_PRE_INCR_IR0, source, 0 ),
			examines2( source, R_BASE ),
			alters3( TMP, RR_ST, source ) );
	}

      /* repeat load and save ops */

      /* NB/ do NOT use RPTS as this locks out interrupts */

      outdellabref( (OP_RPTBD << 24) | (-3 & 0x00FFFFFFU),
		   l,
		   LABREF_OFF24,
		   examines1( RR_PC ),
		   alters4( RR_RC, RR_RS, RR_RE, RR_PC ) );
      
      setlabel( l );
      
      /*
       * instruction repeated is:
       *
       *    LDI  *++<source>(1), TMP
       * || STI  TMP, *<dest>++(1)
       *
       */
      
      outinstr( B_1101 << 28 | B_1010 << 24 |
	       hardware_register( TMP ) << 22 | hardware_register( TMP ) << 16 |
	       build_parallel_indirect( INDIRECT_POST_INCR, dest ) << 8 |
	       build_parallel_indirect( INDIRECT_PRE_INCR, source ),
	       examines3( source, TMP, dest ),
	       alters3(   source, TMP, dest ) );

      /* prevent the repeated instruction from being peepholed */
      
      flush_peepholer( DBG( "copy memory" ) );
      
      /* save last word */
      
      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, TMP, build_indirect( INDIRECT_POST_INCR, dest, 1 ),
		    examines2( TMP, dest ),
		    alters1( dest ) );

#ifdef TARGET_R0_ALWAYS_ZERO
      if (TMP == R_ZERO)
	out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, TMP, 0,
		      examines0(), alters1( TMP ) );
#else
      if (must_save)
	{
	  /* restore TMP register */

	  ipop( TMP );
	}
#endif

      peep_note_addr_reg_loaded( source, src, length / sizeof_int - 1, TRUE );
      peep_note_addr_reg_loaded( dest,   dst, length / sizeof_int, TRUE );
    }  

  if (restore_ar)
    {
      if (length > 2 * sizeof_int)
	{
	  ipop( dest );
	}
      else
	{
	  move_register( R_TMP2, dest, FALSE );
	}
    }

  return;

} /* copy_memory */

/*}}}*/
#ifdef ADDRESS_REG_STUFF
/*{{{  target_isaddrreg_() */

/*
 * Returns true iff the register is an address register
 * that can be used by the compiler
 */

bool
target_isaddrreg_( RealRegister r )
{
  /* any register will do since we have to translate all addresses anyway */
  
  return TRUE;
  
} /* target_isaddrreg_ */

/*}}}*/
#endif /* ADDRESS_REG_STUFF */
/*{{{  immed_cmp() */

/*
 * Returns true iff the value 'n' is suitable for an immediate comparison
 */

bool
immed_cmp( int32 n )
{
  return fits_in_16_bits_signed( n );

} /* immed_cmp */

/*}}}*/
#ifdef NOT_USED
/*{{{  out_merge_byte() */

/*
 * copy the 'which_byte'th byte out of 'source' and
 * merge into 'destination'
 * NB/ This function must produce exactly one instruction
 */

static void
out_merge_byte(
	       int32		which_byte,
	       RealRegister	destination,
	       RealRegister	source )
{
  which_byte &= 0x3;
  
  outinstr( FUNC_LOAD << 28 | OP_MB << 24 | which_byte << 23 | ADDR_MODE_REGISTER << 21 |
	       hardware_register( destination ) | hardware_register( source ),
	   examines2( destination, source ),
	   alters2( destination, RR_ST ) );

  return;
  
} /* out_merge byte */

/*}}}*/
#endif /* NOT_USED */
/*{{{  non_word_op() */

/*
 * general function to load or store a byte or half-word
 */

static void
non_word_op(
	    RealRegister	operand,		/* register "containing" operand */
	    RealRegister	address,		/* register "pointing to" operand */
	    int32		offset,			/* offset from address (doubles as a RealRegister) */
	    int32		offset_in_register,	/* non-zero if offset is a RealRegister */
	    int32		is_signed,		/* non-zreo if loading a signed quantity */
	    int32		half_word_op,		/* non-zero if operand is a half-word quantity */
	    int32		is_load )		/* non-zero if the operation is a load op */
{
  RealRegister	addr      = GAP;
  bool		absolute  = FALSE;
  bool		addr_not_loaded = FALSE;

  
  if (is_word_addressed_( address ))
    {
      if (address == R_SP)
	flush_pending_pushes();
      
      if (offset_in_register)
	{
	  addr = peep_get_free_addr_reg( address );
	  
	  /*
	   * copy stack pointer to 'addr' adjusting for byte offsets
	   *
	   * XXX - beware this might overflow if either of the top two
	   *       bits of the stack pointer are set !!!
	   */

	  out_triadic_op( OP_LSH3, addr, address, 2, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			 examines1( address ),
			 alters2( addr, RR_ST ) );

	  /* add in the offset */
      
	  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, addr, offset,
			examines2( addr, offset ),
			alters2( addr, RR_ST ) );

	  /* indicate that we have an absolute address */

	  absolute   = TRUE;
	}
      else
	{
	  int32		selector;
	  int32		indirect_field;
	  RealRegister	ar;
	  int32		off;
	  bool		biased;
	  int32		op;
	  

	  selector = half_word_op ?
	    (offset & 0x2) >> 1 :
	     offset & 0x3;
	  
	  offset /= sizeof_int;

	  if (offset)
	    {
	      if (!fits_in_8_bits_unsigned( abs( offset ) ))
		{
		  ar = peep_find_loaded_addr_reg( address, &off, &biased );
		  
		  if (!no_peepholing  &&
		      ar     != GAP   &&
		      biased == FALSE &&
		      fits_in_16_bits_signed( offset - off ))
		    {
		      peepf( "eliminated load to temporary address register" );

		      if ((offset - off) == 0)
			{
			  peep_eliminated += 2;
			}
		      else
			{
			  ++peep_eliminated;
		      
			  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, ar, offset - off,
					examines1( ar ),
					alters2( ar, RR_ST ) );
			}
		    }
		  else
		    {
		      if (ar == GAP)
			{
			  ar = peep_get_free_addr_reg( GAP );
			}
		      
		      load_integer( ar, offset, FALSE );

		      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, address,
				examines2( address, ar ),
				alters2( ar, RR_ST ) );
		    }

		  peep_note_addr_reg_loaded( ar, address, offset, FALSE );
		  
		  indirect_field = build_indirect( INDIRECT_REL, ar, 0 );

		  address = ar;
		}
	      else if (offset > 0)
		{
		  indirect_field = build_indirect( INDIRECT_PRE_ADD, address, offset );
		}
	      else
		{
		  indirect_field = build_indirect( INDIRECT_PRE_SUB, address, -offset );
		}
	    }
	  else
	    {
	      indirect_field = build_indirect( INDIRECT_REL, address, 0 );
	    }

	  if (is_load)
	    {	      
	      if (half_word_op)
		{
		  op = is_signed ? OP_LH : OP_LHU;
		}
	      else
		{
		  op = is_signed ? OP_LB : OP_LBU;
		}
	  
	      outinstr( FUNC_LOAD << 28 | op << 24 | selector << 23 | ADDR_MODE_INDIRECT << 21 |
		       hardware_register( operand ) << 16 | indirect_field,
		       examines1( address ),
		       alters2( operand, RR_ST ) );
	    }
	  else
	    {
	      /* get word on stack */
	      
	      out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_TMP1, indirect_field,
			    examines1( address ),
			    alters2( R_TMP1, RR_ST ) );
	      
	      /* merge in byte or half-word */

	      op = half_word_op ? OP_MH : OP_MB;
	      
	      outinstr( FUNC_LOAD                  << 28 |
		       op                          << 24 |
		       selector                    << 23 |
		       ADDR_MODE_REGISTER          << 21 |
		       hardware_register( R_TMP1 ) << 16 |
		       hardware_register( operand ),
		       examines2( operand, R_TMP1 ),
		       alters2( R_TMP1, RR_ST ) );
	      
	      /* and store back onto the stack */
	      
	      out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, indirect_field,
			    examines2( R_TMP1, address ),
			    alters0() );
	    }
	  
	  return;
	}
    }
  else
    {
      addr = peep_get_free_addr_reg( address );
	  
      if (addr == GAP)
	syserr( gen_URG );
  
      /* get byte offset into 'addr' */
      
      if (offset_in_register)
	{
	  /* add offset to address and place in temporary register */
	  
	  out_triadic_op( OP_ADDI3, addr, address, offset, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			 examines2( address, offset ),
			 alters2( addr, RR_ST ) );
	}
      else
	{
	  if (offset == 0)
	    {
	      addr_not_loaded = TRUE;
	    }
	  else if (fits_in_8_bits_signed( offset ))
	    {
	      out_triadic_op( OP_ADDI3, addr, address, offset, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			     examines1( address ),
			     alters2( addr, RR_ST ) );
	    }
	  else
	    {
	      load_integer( R_TMP1, offset, FALSE );
	      
	      out_triadic_op( OP_ADDI3, addr, address, R_TMP1, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			     examines2( address, R_TMP1 ),
			     alters2( addr, RR_ST ) );
	    }
	}
    }

  /*
   *  addr    - byte offset if 'addr_not_loaded' is FALSE
   *  address - byte offset if 'addr_not_loaded' is TRUE
   *  operand - free or value to store
   *  R_TMP1  - free
   *  R_TMP2  - free
   *  R_TMP3  - free
   */
  
  if (is_load)
    {
      if (half_word_op)
	{
	  /*
	   *    addr    - byte offset
	   *    address - byte offset if 'addr_not_loaded' is TRUE
	   *    operand - free 
	   *    R_TMP1  - free
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   *
	   * loose bottom two bits (byte selector) of byte offset
	   */

	  if (addr_not_loaded)
	    {
	      out_triadic_op( OP_LSH3, addr, address, -2, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			    examines1( address ),
			    alters2( addr, RR_ST ) );
	    }
	  else
	    {
	      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, addr, -2,
			    examines1( addr ),
			    alters2( addr, RR_ST ) );
	    }

	  /*
	   *    addr    - word offset
	   *    operand - free 
	   *    R_TMP1  - free
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   *
	   * load half word selector into R_TMP2
	   *
	   * (the carry flag contains the half-word selector, so ...)
	   */
	  
	  conditional_load( Q_LO, ADDR_MODE_IMMEDIATE, R_TMP2,
			   is_signed ? 0 : -16,
			   examines1( RR_ST ),
			   alters1( R_TMP2 ) );
	  
	  conditional_load( Q_HS, ADDR_MODE_IMMEDIATE, R_TMP2,
			   is_signed ? 16 : 0,
			   examines1( RR_ST ),
			   alters1( R_TMP2 ) );
	  /*
	   *    addr    - word offset
	   *    operand - free 
	   *    R_TMP1  - free
	   *    R_TMP2  - half word selector (0, 16 or -16)
	   *    R_TMP3  - free
	   *
	   * shift source half-word up or down and place in destination
	   */

	  out_triadic_op( OP_LSH3,
			 operand,
			 absolute ?
			 build_indirect( INDIRECT_REL, addr, 0 ) :
			 build_indirect( INDIRECT_PRE_INCR_IR0, addr, 0 ),
			 R_TMP2,
			 ADDR_MODE_INDIRECT,
			 ADDR_MODE_REGISTER,
			 absolute ?
			 examines2( R_TMP2, addr ) :
			 examines3( R_TMP2, addr, R_BASE ),
			 alters2( operand, RR_ST ) );

	  /*
	   *    addr    - free
	   *    operand - shifted word at destination
	   *    R_TMP1  - free
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   */
	  
	  if (is_signed)
	    {
	      /*
	       * shift source half-word down and place in destination
	       */
	      
	      out_diadic_op( OP_ASH, ADDR_MODE_IMMEDIATE, operand, -16,
			     examines1( operand ),
			     alters2( operand, RR_ST ) );
	    }
	  else
	    {
	      /*
	       * ensure rest of word is zero
	       *
	       * NB/ this is needed because code like
	       *
	       * int func( char * p ) { if (*p < 10) return 1; return 0 }
	       *
	       * does not mask result of load
	       */
	      
	      out_diadic_op( OP_AND, ADDR_MODE_IMMEDIATE, operand, 0xFFFF,
			    examines1( operand ),
			    alters2( operand, RR_ST ) );
	    }
	}
      else
	{
	  /*
	   *    addr    - byte offset if 'addr_not_loaded' is FALSE
	   *    address - byte offset if 'addr_not_loaded' is TRUE
	   *    operand - free 
	   *    R_TMP1  - free
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   *
	   * loose bottom two bits (byte selector) of offset AND
	   * save non-word selector of address into R_TMP2
	   */

	  if (addr_not_loaded)
	    {
	      out_triadic_op( OP_LSH3, addr, address, -2, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			    examines1( address ),
			    alters2( addr, RR_ST ) );
	      
	      out_triadic_op( OP_AND3, R_TMP2, address,
			     0x3, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			     examines1( address ),
			     alters2( R_TMP2, RR_ST ) );	  
	    }
	  else
	    {
	      out_triadic_op( OP_AND3, R_TMP2, addr,
			     0x3, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			     examines1( addr ),
			     alters2( R_TMP2, RR_ST ) );
	  
	      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, addr, -2,
			    examines1( addr ),
			    alters2( addr, RR_ST ) );
	    }

	  /*
	   *    addr    - word offset
	   *    operand - free
	   *    R_TMP1  - free
	   *    R_TMP2  - byte selector (0, 1, 2 or 3)
	   *    R_TMP3  - free
	   *
	   * convert byte selector to negated bit_position
	   * NB/ done before load to help avoid pipeline conflicts
	   */
	      
	  out_diadic_op( OP_MPYI, ADDR_MODE_IMMEDIATE, R_TMP2, -8,
			examines1( R_TMP2 ),
			alters2( R_TMP2, RR_ST ) );
	      
	  /*
	   *    addr    - word offset
	   *    operand - free
	   *    R_TMP1  - free
	   *    R_TMP2  - byte selector (0, -8, -16 or -24)
	   *    R_TMP3  - free
	   *
	   * shift source word to correct source alignment and place in destination or R_TMP3
	   */
	      
	  out_triadic_op( OP_LSH3,
			 is_signed ? R_TMP3 : operand,
			 absolute ?
			 build_indirect( INDIRECT_REL, addr, 0 ) :
			 build_indirect( INDIRECT_PRE_INCR_IR0, addr, 0 ),
			 R_TMP2,
			 ADDR_MODE_INDIRECT,
			 ADDR_MODE_REGISTER,
			 absolute ?
			 examines2( addr, R_TMP2 ) :
			 examines3( addr, R_TMP2, R_BASE ),
			 absolute ?
			 alters2( is_signed ? R_TMP3 : operand, RR_ST ) :
			 alters3( is_signed ? R_TMP3 : operand, RR_ST, addr ) );

	  if (is_signed)
	    {
	      /*
	       *    addr    - free
	       *    operand - free
	       *    R_TMP1  - free
	       *    R_TMP2  - free
	       *    R_TMP3  - part of word at destination
	       *
	       * load bottom byte from R_TMP3 into destination, sign extending
	       */
	      
	      outinstr( (FUNC_LOAD << 28) | (OP_LB << 24) | (0 << 23) | (ADDR_MODE_REGISTER << 21) |
		       hardware_register( operand ) << 16 | hardware_register( R_TMP3 ),
		       examines1( R_TMP3 ),
		       alters2( operand, RR_ST ) );
	    }
	  else
	    {
	      /*
	       *    addr    - free
	       *    operand - part of word at destination
	       *    R_TMP1  - free
	       *    R_TMP2  - free
	       *    R_TMP3  - free
	       *
	       * ensure rest of word is zero
	       *
	       * NB/ this is needed because code like
	       *
	       * int func( char * p ) { if (*p < 10) return 1; return 0 }
	       *
	       * does not mask result of load
	       */
	      
	      out_diadic_op( OP_AND, ADDR_MODE_IMMEDIATE, operand, 0xFF,
			    examines1( operand ),
			    alters2( operand, RR_ST ) );
	    }
	}
    }
  else /* non-word store */
    {
      if (half_word_op)
	{
	  /*
	   *    addr    - byte offset if 'addr_not_loaded' is FALSE
	   *    address - byte offset if 'addr_not_loaded' is TRUE
	   *    operand - value to store
	   *    R_TMP1  - free
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   *
	   * loose bottom two bits of byte offset (ie convert to word offset)
	   * NB/ This is done 6 instructions before the load to avoid pipeline conflicts
	   */

	  if (addr_not_loaded)
	    {
	      out_triadic_op( OP_LSH3, addr, address, -2, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			    examines1( address ),
			    alters2( addr, RR_ST ) );
	    }
	  else
	    {
	      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, addr, -2,
			    examines1( addr ),
			    alters2( addr, RR_ST ) );
	    }

	  /*
	   *    addr    - word offset
	   *    operand - value to store
	   *    R_TMP1  - free
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   *
	   * load word at destination into R_TMP1, without corrupting carry flag
	   */

	  if (absolute)
	    {
	      conditional_load( Q_AL, ADDR_MODE_INDIRECT, R_TMP1,
			       build_indirect( INDIRECT_REL, addr, 0 ),
			       examines1( addr ),
			       alters1( R_TMP1 ) );
	    }
	  else
	    {
	      conditional_load( Q_AL, ADDR_MODE_INDIRECT, R_TMP1,
			       build_indirect( INDIRECT_PRE_INCR_IR0, addr, 0 ),
			       examines2( addr, R_BASE ),
			       alters2( R_TMP1, addr ) );
	    }
	  
	  /*
	   *    addr    - word address
	   *    operand - value to store
	   *    R_TMP1  - word at destination
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   *
	   * copy destination word into R_TMP2, without corrupting carry flag
	   */

	  conditional_load( Q_AL, ADDR_MODE_REGISTER, R_TMP2, R_TMP1,
			   examines1( R_TMP1 ),
			   alters1( R_TMP2 ) );
	  
	  /*
	   *    addr    - word address
	   *    operand - value to store
	   *    R_TMP1  - word at destination
	   *    R_TMP2  - word at destination
	   *    R_TMP3  - free
	   *
	   * merge source value into bottom half of destination word (does not affect carry flag)
	   */

	  outinstr( (FUNC_LOAD << 28) | (OP_MH << 24) | (0 << 23) | (ADDR_MODE_REGISTER << 21) |
		   hardware_register( R_TMP1 ) << 16 | hardware_register( operand ),
		   examines2( R_TMP1, operand ),
		   alters2( R_TMP1, RR_ST ) );
	  
	  /*
	   *    addr    - word address
	   *    operand - value to store
	   *    R_TMP1  - merged source value & top half of word at destination
	   *    R_TMP2  - word at destination
	   *    R_TMP3  - free
	   *
	   * merge source value into top half of destination word (does not affect carry flag)
	   */

	  outinstr( (FUNC_LOAD << 28) | (OP_MH << 24) | (1 << 23) | (ADDR_MODE_REGISTER << 21) |
		   hardware_register( R_TMP2 ) << 16 | hardware_register( operand ),
		   examines2( R_TMP1, operand ),
		   alters2( R_TMP2, RR_ST ) );
	  
	  /*
	   *    addr    - word address
	   *    operand - free (not corruptible)
	   *    R_TMP1  - merged source value & top    half of word at destination
	   *    R_TMP2  - merged source value & bottom half of word at destination
	   *    R_TMP3  - free
	   *
	   * select the correct value to store
	   *
	   * (The LSH above left the half-word selector in the carry flag, and none
	   *  of the other instructions have affected this flag).
	   */

	  conditional_load( Q_LO, ADDR_MODE_REGISTER, R_TMP1, R_TMP2,
			   examines2( RR_ST, R_TMP2 ),
			   alters1( R_TMP1 ) );
	  
	  /*
	   *    addr    - word address
	   *    operand - free (not corruptible)
	   *    R_TMP1  - merged value to store
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   *
	   * store the value back into memory
	   */

	  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, R_TMP1, build_indirect( INDIRECT_REL, addr, 0 ),
			examines2( R_TMP1, addr ),
			alters0() );
	}
      else
	{
	  RealRegister	dest;


	  if (addr_not_loaded)
	    {
	      /*
	       *    address - byte offset
	       *    addr    - free
	       *    operand - value to store
	       *    R_TMP1  - free
	       *    R_TMP2  - free
	       *    R_TMP3  - free
	       *
	       * convert byte offset to word offset
	       */

	      out_triadic_op( OP_LSH3, addr, address, -2, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			     examines1( address ),
			     alters2( addr, RR_ST ) );
	      
	      /*
	       *    address - byte offset
	       *    addr    - word offset
	       *    operand - value to store
	       *    R_TMP1  - byte selector (0, 1, 2, or 3)
	       *    R_TMP2  - free
	       *    R_TMP3  - free
	       *
	       * save byte selector of address into R_TMP1
	       */
	  
	      out_triadic_op( OP_AND3, R_TMP1, address, 0x3, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			     examines1( address ),
			     alters2( R_TMP1, RR_ST ) );	  
	    }
	  else
	    {
	      /*
	       *    addr    - byte offset
	       *    operand - value to store
	       *    R_TMP1  - free
	       *    R_TMP2  - free
	       *    R_TMP3  - free
	       *
	       * save byte selector of address into R_TMP1
	       */
	  
	      out_triadic_op( OP_AND3, R_TMP1, addr, 0x3, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			     examines1( addr ),
			     alters2( R_TMP1, RR_ST ) );
	  
	      /*
	       *    addr    - byte offset
	       *    operand - value to store
	       *    R_TMP1  - byte selector (0, 1, 2, or 3)
	       *    R_TMP2  - free
	       *    R_TMP3  - free
	       *
	       * convert byte offset to word offset
	       */
	  
	      out_diadic_op( OP_LSH, ADDR_MODE_IMMEDIATE, addr, -2,
			    examines1( addr ),
			    alters2( addr, RR_ST ) );
	    }

	  /*
	   *    addr    - word offset
	   *    operand - value to store
	   *    R_TMP1  - byte selector (0, 1, 2, or 3)
	   *    R_TMP2  - free
	   *    R_TMP3  - free
	   *
	   * convert byte selector to bit-position selector in R_TMP2
	   */
	  
	  out_triadic_op( OP_MPYI3, R_TMP2, R_TMP1, 8, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			 examines1( R_TMP1 ),
			 alters2( R_TMP2, RR_ST ) );
	  
	  /*
	   *    addr    - word offset
	   *    operand - value to store
	   *    R_TMP1  - free
	   *    R_TMP2  - bit-position selector (0, 8, 16 or 24)
	   *    R_TMP1  - free
	   *
	   * shift source value to correct alignment and place in R_TMP3
	   */
	  
	  out_triadic_op( OP_LSH3, R_TMP3, operand, R_TMP2, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			 examines2( operand, R_TMP2 ),
			 alters2( R_TMP3, RR_ST ) );
	  
	  /*
	   *    addr    - word offset
	   *    operand - free (might not be corruptible)
	   *    R_TMP1  - free
	   *    R_TMP2  - bit-position selector (0, 8, 16 or 24)
	   *    R_TMP3  - shifted source value
	   *
	   * load mask into R_TMP1
	   */
	  
	  out_diadic_op( OP_LDI, ADDR_MODE_IMMEDIATE, R_TMP1, 0xFF,
			examines1( R_TMP1 ),
			alters2( R_TMP1, RR_ST ) );
	  
	  /*
	   *    addr    - word offset
	   *    operand - free (might not be corruptible)
	   *    R_TMP1  - 0x000000FF
	   *    R_TMP2  - bit-position selector (0, 8, 16 or 24)
	   *    R_TMP3  - shifted source value
	   *
	   * shift mask up if necessary
	   */
	  
	  out_diadic_op( OP_LSH, ADDR_MODE_REGISTER, R_TMP1, R_TMP2,
			examines2( R_TMP2, R_TMP1 ),
			alters2( R_TMP1, RR_ST ) );
	  
	  /*
	   *    addr    - word offset
	   *    operand - free (might not be corruptible)
	   *    R_TMP1  - byte mask (0x000000FF, 0x0000FF00, 0x00FF0000 or 0xFF000000)       
	   *    R_TMP2  - free
	   *    R_TMP3  - shifted source value
	   *
	   * get word at destination into R_TMP2 with unwanted byte masked out
	   */

	  if (death & regbit( operand ))
	    {
	      dest = operand;
	    }
	  else
	    {
	      dest = R_TMP2;
	    }

	  if (absolute)
	    {
	      out_triadic_op( OP_ANDN3, dest, build_indirect( INDIRECT_REL, addr, 0 ),
			     R_TMP1, ADDR_MODE_INDIRECT, ADDR_MODE_REGISTER,
			     examines3( dest, R_TMP1, addr ),
			     alters2( dest, RR_ST ) );
	    }
	  else
	    {
	      out_triadic_op( OP_ANDN3, dest, build_indirect( INDIRECT_PRE_INCR_IR0, addr, 0 ),
			     R_TMP1, ADDR_MODE_INDIRECT, ADDR_MODE_REGISTER,
			     examines4( dest, R_TMP1, addr, R_BASE ),
			     alters3( dest, RR_ST, addr ) );
	    }
	  
	  /*
	   *    addr    - word address
	   *    R_TMP1  - byte mask (0x000000FF, 0x0000FF00, 0x00FF0000 or 0xFF000000)       
	   *    dest    - other part of word at destination
	   *    R_TMP3  - shifted source value
	   *
	   * mask out all but the byte to be stored
	   */
	  
	  out_diadic_op( OP_AND, ADDR_MODE_REGISTER, R_TMP3, R_TMP1,
			examines2( R_TMP1, R_TMP3 ),
			alters2( R_TMP3, RR_ST ) );
	  
	  /*
	   *    addr    - word address
	   *    R_TMP1  - free
	   *    dest    - other part of word at destination
	   *    R_TMP3  - shifted and masked source value
	   *
	   * OR in the byte
	   */
	  
	  out_diadic_op( OP_OR, ADDR_MODE_REGISTER, dest, R_TMP3,
			examines2( dest, R_TMP3 ),
			alters2( dest, RR_ST ) );
	  
	  /*
	   *    addr    - word address
	   *    R_TMP1  - free
	   *    dest    - source part word plus destination part word
	   *    R_TMP3  - free
	   *
	   * and store word back in destination
	   */
	  
	  out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, dest, build_indirect( INDIRECT_REL, addr, 0 ),
			examines2( dest, addr ),
			alters0() );
	}
    }

  if (!offset_in_register)
    {
      peep_note_addr_reg_loaded( addr, address, offset / sizeof_int, TRUE );
    }
  
  if (is_load)
    {
      peep_forget_about( operand );
    }
      
  return;
  
} /* non_word_op */

/*}}}*/
/*{{{  fix() */

/*
 * convert an FP value into an integer value
 */

static void
fix(
    RealRegister	src,		/* register containing FP value  */
    RealRegister	dst,		/* register to contain INT value */
    bool		is_signed )	/* non-0 if 'dst' is signed      */
{
#ifdef DEBUG
  if (!is_float( src ))
    syserr( gen_non_FP_source );
#endif

  if (is_signed)
    {
      bool		must_save = FALSE;
      RealRegister	tmp;


      /* convert the number to integer format */
      
      out_diadic_op( OP_FIX, ADDR_MODE_REGISTER, dst, src,
		    examines1( src ),
		    alters2( dst, RR_ST ) );
	      
      /*
       * ANSI spec says that J_FIX must truncate floating point
       * numbers, but the OP_FIX rounds towards negative infinity
       * so ....
       */

      /* get hold of a free floating point temporary register */
      
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
      tmp = get_free_register( RR_R1, RR_R2, RR_R3, RR_R8, RR_R9, RR_R10, GAP );
      
      /* if we must then save a register on the stack */
      
      if (tmp == GAP)
	{
	  tmp = R_LR;
	  
	  if ((saved_ivars & regbit( R_LR )) == 0 ||
	      usedmaskvec.map[ 0 ] & regbit( R_LR ))	/* link register being used as temporary */
	    {
	      must_save = TRUE;

	      dpush( tmp );
	    }
	}
#else
      tmp = get_free_register( RR_R1, RR_R2, RR_R3, RR_R10, GAP );
      
      /*
       * if we must then save a register in temporary.
       * Note/ we have chosen an FP reg that is only used to hold INTs
       */
      
      if (tmp == GAP)
	{
	  tmp = R_LR;
	  
	  if ((saved_ivars & regbit( R_LR )) == 0 ||
	      usedmaskvec.map[ 0 ] & regbit( R_LR ))	/* link register being used as temporary */
	    {	
	      must_save = TRUE;
	  
	      move_register( tmp, R_TMP1, FALSE );
	    }
	}
#endif
      /* negate original number */
      
      out_diadic_op( OP_NEGF, ADDR_MODE_REGISTER, tmp, src,
		    examines1( src ),
		    alters2( tmp, RR_ST ) );
      
      /* fix this number */
      
      out_diadic_op( OP_FIX, ADDR_MODE_REGISTER, tmp, tmp,
		    examines1( tmp ),
		    alters2( tmp, RR_ST ) );
      
      /* negate the result */
      
      out_diadic_op( OP_NEGI, ADDR_MODE_REGISTER, tmp, tmp,
		    examines1( tmp ),
		    alters2( tmp, RR_ST ) );
      
      /* if the result is negative, then so was the original */
      
      conditional_load( Q_LE, ADDR_MODE_REGISTER, dst, tmp,
		       examines2( tmp, RR_ST ),
		       alters1( dst ) );
      
      /* restore register if necessary */
      
      if (must_save)
	{
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
	  dpop( tmp );
#else
	  move_register( R_TMP1, tmp, FALSE );
#endif
	}
    }
  else
    {
#ifdef TARGET_LACKS_UNSIGNED_FIX
      syserr( gen_no_unsigned_fix );
#else
      bool		must_save = FALSE;
      RealRegister	tmp;
      LabelNumber *	l;
      
      
      /*
       * ANSI spec does not say what to do if the FP value is < 0.0
       * All the compilers I have tested so far just return the bit
       * pattern for the negative number, ie
       *    ((unsigned int) -2.0) = 0xfffffffe
       * which I consider to be wrong.  I would expect rounding to 0.0, ie
       *    ((unsigned int) -2.0) = 0x00000000
       *
       * The real problem, however is when the FP number does not fit
       * in a signed int, but it would fit in an unsigned int.  The
       * compiler has code to cope with this, but I am hoping that I
       * will be able to produce better code here.
       */
      
      /* convert the number */
      
      out_diadic_op( OP_FIX, ADDR_MODE_REGISTER, dst, src,
		    examines1( src ),
		    alters2( dst, RR_ST ) );
      
#ifdef UNSIGNED_FIX_GIVES_0_FOR_NEGATIVE
      conditional_load( Q_LT, ADDR_MODE_IMMEDIATE, dst, 0,
		       examines1( RR_ST ),
		       alters1( dst ) );
#endif
      if (src == dst)
	{
	  /* get hold of a free floating point register */
	  
	  tmp = get_free_register( RR_R1, RR_R2, RR_R3, RR_R6, RR_R7, RR_R10, GAP );
	}
      else
	{
	  tmp = dst;
	}
      
      l = nextlabel();
      
      /* if integer overflow did not occur, then skip the next bit */
      
      conditional_branch_to( C_NV, l );
      
      /* if we must then save a register on the stack */
      
      if (tmp == GAP)
	{
	  tmp = R_LR;
	  
	  if ((saved_ivars & regbit( R_LR )) == 0 ||
	      usedmaskvec.map[ 0 ] & regbit( R_LR ))	/* link register being used as temporary */	      
	    {	
	      must_save = TRUE;

#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS	      
	      dpush( tmp );
#else
	      move_register( tmp, R_TMP2, FALSE );
#endif
	    }
	}
      
      /* load 2e31 */
      
      load_integer( R_TMP1, 0x1f000000U, FALSE );
      
      /* save on stack as an integer */
      
      ipush( R_TMP1 );
      
      /* pop the value off the stack as a float */
      
      fpop( tmp );
      
      /* subtract 2^31 from the original number */
      
      out_triadic_op( OP_SUBF3, dst, src, tmp, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( src, tmp ),
		     alters2( dst, RR_ST ) );
      
      /* convert the number again */
      
      out_diadic_op( OP_FIX, ADDR_MODE_REGISTER, dst, dst,
		    examines1( dst ),
		    alters2( dst, RR_ST ) );
      
      /* then add back in 2^31 */
      
      load_integer( R_TMP1, 0x80000000U, FALSE );
      
      out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, dst, R_TMP1,
		    examines2( dst, R_TMP1 ),
		    alters2( dst, RR_ST ) );
      
      if (must_save)
	{
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS
	  dpop( tmp );
#else
	  move_register( R_TMP2, tmp, FALSE );
#endif
	}
      
      /* prevent these instructions from being peepholed */
      
      flush_peepholer( DBG( "unsigned fix" ) );
      
      /* target the end of the branch */
      
      setlabel( l );
#endif /* ! TARGET_LACKS_UNSIGNED_FIX */
    }

  peep_forget_about( dst );
  
  return;
  
} /* fix */

/*}}}*/
#ifdef TARGET_HAS_DEBUGGER
/*{{{  macros for register saving */

/*
 * macros for storing and retrieving a register from a known location in memory
 */

int32 saved_regs_offsets[ MAXREGNUMBER + 1 ] =
  {
    0,	/* R0  -  0 - integer */
    1,  /* R1  -  1 - integer */
    2,  /* R2  -  2 - integer */
    3,  /* R3  -  3 - integer */
    4,  /* R4  -  4 - FP      */
    6,  /* R5  -  5 - FP      */
    8,  /* R6  -  6 - FP      */
   10,  /* R7  -  7 - FP      */
   12,  /* DP  -  8 - integer */
   13,  /* BK  -  9 - integer */
   14,  /* R8  - 10 - integer */
   15,  /* R9  - 11 - integer */
   16,  /* R10 - 12 - integer */
   17,  /* AR0 - 13 - address */
   18,  /* AR1 - 14 - address */
   19,  /* AR2 - 15 - address */
   20,  /* AR3 - 16 - address */
   21,  /* R11 - 17 - integer */
   22,  /* AR4 - 18 - address */
   23,  /* AR5 - 19 - address */
   24,  /* AR6 - 20 - address */
   25,  /* AR7 - 21 - address */
   26,  /* IR0 - 22 - index   */
   27,  /* IR1 - 23 - index   */
   28,  /* SP  - 24 - stack   */
   29,  /* ST  - 25 - status  */
   30,  /* RS  - 26 - integer */
   31,  /* RE  - 27 - integer */
   32   /* RC  - 28 - integer */
  };

#define save_reg( r )	\
  if (usedmaskvec.map[ 0 ] & (int32) regbit( r )) \
    out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, r, \
		  build_indirect( INDIRECT_PRE_ADD, R_ATMP, saved_regs_offsets[ r ] ), \
		  examines2( r, R_ATMP ), \
		  alters0() )

#define restore_reg( r )	\
  if (usedmaskvec.map[ 0 ] & (int32) regbit( r )) \
    out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, r, \
		  build_indirect( INDIRECT_PRE_ADD, R_ATMP, saved_regs_offsets[ r ] ), \
		  examines1( R_ATMP ), \
		  alters1( r ) )

#define save_freg( r )	\
  if (usedmaskvec.map[ 0 ] & (int32) regbit( r )) \
  { \
    out_diadic_op( OP_STF, ADDR_MODE_INDIRECT, r, \
		  build_indirect( INDIRECT_PRE_ADD, R_ATMP, saved_regs_offsets[ r ] ), \
		  examines2( r, R_ATMP ), \
		  alters0() ); \
    \
    out_diadic_op( OP_STI, ADDR_MODE_INDIRECT, r, \
		  build_indirect( INDIRECT_PRE_ADD, R_ATMP, saved_regs_offsets[ r ] + 1 ), \
		  examines2( r, R_ATMP ), \
		  alters0() ); \
  }


#define restore_freg( r ) \
  if (usedmaskvec.map[ 0 ] & (int32) regbit( r )) \
  { \
    out_diadic_op( OP_LDF, ADDR_MODE_INDIRECT, r, \
		  build_indirect( INDIRECT_PRE_ADD, R_ATMP, saved_regs_offsets[ r ] ), \
		  examines1( R_ATMP ), \
		  alters1( r ) ); \
    \
    out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, r, \
		  build_indirect( INDIRECT_PRE_ADD, R_ATMP, saved_regs_offsets[ r ] + 1 ), \
		  examines1( R_ATMP ), \
		  alters1( r ) ); \
    }

/*}}}*/
/*{{{  do_notify_command() */

static void
do_notify_command(
		  int32		linenumber,
		  char *	filename )
{
  LabelNumber *		label;
  
  
  if (debugging( DEBUG_Q ))
    cc_msg( "debugger: generating _notify_command of: %s at line %ld\n", filename, linenumber );

  if (saved_regs == NULL)
    {
      syserr( gen_failed_to_init );
    }  
  
  flush_peepholer( DBG( "notify_command" ) );

  /*
   * save registers into saved_regs structure of debugger library
   */

  peep_corrupt_addr_reg( R_ATMP );
  
  /* get address of module table into temporary */

  move_register( R_MT, R_ATMP, FALSE );

  /* add in the offset of the module containing 'saved_regs' */
	  
  peep_xref( X_DataModule, saved_regs );
  
  out_diadic_op( OP_ADDI, ADDR_MODE_IMMEDIATE, R_ATMP, 0,
		examines1( R_ATMP ),
		alters2( R_ATMP, RR_ST ) );
  
  /* get the address of the module's function table or data slots */
  
  out_diadic_op( OP_LDI, ADDR_MODE_INDIRECT, R_ATMP,
		build_indirect( INDIRECT_REL, R_ATMP, 0 ),
		examines1( R_ATMP ),
		alters2( R_ATMP, RR_ST ) );

  /* get high part of data offset */
  
  peep_xref( X_DataSymbHi, saved_regs );
  
  out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, R_TMP1, 0,
		examines0(),
		alters1( R_TMP1 ) );
  
  /* load R_TMP1 with the low part offset of 'name' into the data slots or function table */
	      
  peep_xref( X_DataSymbLo, saved_regs );
  
  out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, R_TMP1, 0,
		examines1( R_TMP1 ),
		alters1( R_TMP1 ) );	/* XXX - hide alteration of ST */

  /* and add R_TMP1 into temporary */
	      
  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, R_ATMP, R_TMP1,
		examines2( R_ATMP, R_TMP1 ),
		alters2( R_ATMP, RR_ST ) );

  /* convert */
  
  convert_to_word_address( R_ATMP, R_ATMP );
  
  /* save registers into register array in KNOWN order */

  save_reg( RR_R0  );
  save_reg( RR_R1  );
  save_reg( RR_R2  );
  save_reg( RR_R3  );
  save_freg( RR_R4  );
  save_freg( RR_R5  );
  save_freg( RR_R6  );
  save_freg( RR_R7  );
  save_reg( RR_R8  );
  save_reg( RR_R9  );
  save_reg( RR_R10 );
  save_reg( RR_R11 );
  save_reg( RR_BK  );
  save_reg( RR_DP  );
  save_reg( RR_AR0 );
  save_reg( RR_AR1 );
  save_reg( RR_AR2 );
  save_reg( RR_AR3 );

  /* save address register */

  ipush( R_V1 );
  move_register( R_ATMP, R_V1, FALSE );
  
  /* second argument: the address of the SourceInfo structure (as word pointer) */
  
  label = debugger_filenamelabel( filename );

  /*
   * put PC of next instruction into R_LR
   *
   * (There is no need to save R_LR, as it will
   *  have been pushed onto the stack at the start
   *  of the function.)
   */
  
  outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

  /* protect the following three instructions against LAJs abd BRs */
  
  peep_protect_pc = 3;
  
  /*
   * add in offset of the SourceInfo structure
   */
      
  if (lab_isset_( label ))
    {
      int32	off = (label->u.defn) & 0x7fffffffU;


      flush_peepholer( DBG( "calculating position" ) );
      
      off = (off - codep - codebase) / sizeof_int - 3;

      load_integer( R_A1 + 1, off, FALSE );
    }
  else
    {
      /*
       * NB/ beware of assumption that the structure is within 16
       * (signed) bits of this instruction - this may not be true
       */
      
      peep_fref( label, LABREF_LIT16 ); 
      
      out_diadic_op( OP_LDI, ADDR_MODE_IMMEDIATE, R_A1 + 1, -3,
		    examines0(),
		    alters2( R_A1 + 1, RR_ST ) );
    }      

  /* add in value in link register */
      
  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, R_A1 + 1, R_LR,
		examines1( R_LR ),
		alters2( R_A1 + 1, RR_ST ) );
      
  /* first argument: the line number */
  
  load_integer( R_A1, linenumber, FALSE );

  /* finally call the _notify_command function */
  
  call( _notify_command, FALSE );

  /* restore saved registers */

  move_register( R_V1, R_ATMP, FALSE );
  ipop( R_V1 );
  
  restore_reg( RR_R0  );
  restore_reg( RR_R1  );
  restore_reg( RR_R2  );
  restore_reg( RR_R3  );
  restore_freg( RR_R4  );
  restore_freg( RR_R5  );
  restore_freg( RR_R6  );
  restore_freg( RR_R7  );
  restore_reg( RR_R8  );
  restore_reg( RR_R9  );
  restore_reg( RR_R10 );
  restore_reg( RR_R11 );
  restore_reg( RR_AR0 );
  restore_reg( RR_AR0 );
  restore_reg( RR_AR1 );
  restore_reg( RR_AR2 );
  restore_reg( RR_AR3 );
  restore_reg( RR_BK  );
  restore_reg( RR_DP  );
  
  /* finished */
  
  return;
  
} /* do_notifty_command */

/*}}}*/
#endif /* TARGET_HAS_DEBUGGER */
#ifndef TARGET_LACKS_FP_DIVIDE
/*{{{  fp_reg_divide() */

static void
fp_reg_divide(
	      RealRegister	dest,
	      RealRegister	top,
	      RealRegister	bottom,
	      bool		is_double )
{
  RealRegister	tmp1;
  RealRegister	tmp2;
  bool		must_restore_tmp1 = FALSE;
  bool		must_restore_tmp2 = FALSE;
  
  
  /* dest = top / bottom */
  
  if ((saved_ivars & regbit( R_LR )) &&
      (usedmaskvec.map[ 0 ] & regbit( R_LR )) == 0)	/* link register being used as temporary */
    {
      /* use R_LR as an FP temporary ! */
      
      tmp1 = R_LR;
    }
  else
    {
      /* find a free (integer) register */
      
      tmp1 = get_free_register( RR_R0, RR_R1, RR_R2, RR_R3, RR_R10, GAP );
      
      if (tmp1 == GAP)
	{
	  tmp1 = RR_R10;
	  
	  must_restore_tmp1 = TRUE;
	  
	  move_register( tmp1, R_TMP1, FALSE );
	}
    }
  
  if (dest == top || dest == bottom)
    {
      int32	oldmask = usedmaskvec.map[ 0 ];
      
      
      /* prevent 'tmp1' register from being selected */
      
      usedmaskvec.map[ 0 ] |= regbit( tmp1 );
      
      tmp2 = get_free_register( RR_R0, RR_R1, RR_R2, RR_R3, RR_R10, GAP );
      
      usedmaskvec.map[ 0 ] = oldmask;
      
      if (tmp2 == GAP)
	{
	  tmp2 = RR_R9;
	  
	  must_restore_tmp2 = TRUE;
	  
	  move_register( tmp2, R_TMP2, FALSE );
	}
    }
  else
    {
      /* use destination as a temporary register */
      
      tmp2 = dest;
    }
  
  out_diadic_op( OP_RCPF, ADDR_MODE_REGISTER, tmp1, bottom,
		examines1( bottom ),
		alters2( tmp1, RR_ST ) );
  
  out_triadic_op( OP_MPYF3, tmp2, tmp1, bottom, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		 examines2( tmp1, bottom ),
		 alters2( tmp2, RR_ST ) );
  
  out_diadic_op( OP_SUBRF, ADDR_MODE_IMMEDIATE, tmp2, 0x1000,	/* 0x1000 = 2.0 */
		examines1( tmp2 ),
		alters2( tmp2, RR_ST ) );
  
  out_diadic_op( OP_MPYF, ADDR_MODE_REGISTER, tmp1, tmp2,
		examines2( tmp1, tmp2 ),
		alters2( tmp1, RR_ST ) );
  
  /* repeat last three instruction */
  
  out_triadic_op( OP_MPYF3, tmp2, tmp1, bottom, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		 examines2( tmp1, bottom ),
		 alters2( tmp2, RR_ST ) );
  
  out_diadic_op( OP_SUBRF, ADDR_MODE_IMMEDIATE, tmp2, 0x1000,	/* 0x1000 = 2.0 */
		examines1( tmp2 ),
		alters2( tmp2, RR_ST ) );
  
  out_diadic_op( OP_MPYF, ADDR_MODE_REGISTER, tmp1, tmp2,
		examines2( tmp1, tmp2 ),
		alters2( tmp1, RR_ST ) );

  if (!fast_FP)
    {
      if (is_double)
	{
	  /* for double precision we require a final loop to get the last bit right */
	  
	  out_triadic_op( OP_MPYF3, tmp2, tmp1, bottom, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
			 examines2( tmp1, bottom ),
			 alters2( tmp2, RR_ST ) );
  
	  out_diadic_op( OP_SUBRF, ADDR_MODE_IMMEDIATE, tmp2, 0x0000,	/* 0x0000 = 1.0 */
			examines1( tmp2 ),
			alters2( tmp2, RR_ST ) );
  
	  out_diadic_op( OP_MPYF, ADDR_MODE_REGISTER, tmp2, tmp1,
			examines2( tmp1, tmp2 ),
			alters2( tmp2, RR_ST ) );
      
	  out_diadic_op( OP_ADDF, ADDR_MODE_REGISTER, tmp1, tmp2,
			examines2( tmp1, tmp2 ),
			alters2( tmp2, RR_ST ) );
	}
      else
	{
	  out_diadic_op( OP_RND, ADDR_MODE_REGISTER, tmp1, tmp1,
			examines1( tmp1 ),
			alters2( tmp1, RR_ST ) );	  
	}
    }  
  
  /* finally multiply and place in destination */
  
  out_triadic_op( OP_MPYF3, dest, tmp1, top, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		 examines2( tmp1, top ),
		 alters2( dest, RR_ST ) );

  if (!is_double && !fast_FP)
    {
      out_diadic_op( OP_RND, ADDR_MODE_REGISTER, dest, dest,
		    examines1( dest ),
		    alters2( dest, RR_ST ) );	  
    }
  
  if (must_restore_tmp2)
    {
      move_register( R_TMP2, tmp2, FALSE );
    }
  
  if (must_restore_tmp1)
    {
      move_register( R_TMP1, tmp1, FALSE );
    }
  
  return;
	
} /* fp_reg_divide */

/*}}}*/
/*{{{  fp_imm_divide() */

static void
fp_imm_divide(
	      RealRegister	dest,
	      RealRegister	top,
	      FloatCon *	fc,
	      bool		is_double )
{
  bool		must_restore_bottom = FALSE;
  RealRegister	bottom;
  int32		map;
  int32		i;
  
  
  /* dest = top / fc */
  
  /* we must load the constant into an FP register */

  map = usedmaskvec.map[ 0 ];

  for (i = NFLTTEMPREGS; i--;)
    {
      if ((map & regbit( R_FT1 + i )) == 0)
	break;
    }

  if (i < 0)
    {
      for (i = NFLTVARREGS; i--;)
	{
	  if ((map & regbit( R_FV1 + i )) == 0)
	    break;
	}      

      if (i < 0)
	{
	  if (dest != R_FT1)
	    {
	      if (top != R_FT1)
		{
		  bottom = R_FT1;
		}
	      else if (dest != R_FV1)
		{
		  bottom = R_FV1;
		}
	      else
		{
		  bottom = R_FV1 + 1;	/* XXX beware of assumption that NFLTVARREGS > 1 */
		}
	    }
	  else if (top != R_FV1)
	    {
	      bottom = R_FV1;
	    }
	  else
	    {
	      bottom = R_FV1 + 1;	/* XXX beware of assumption that NFLTVARREGS > 1 */
	    }
	  
	  dpush( bottom );
	  
	  must_restore_bottom = TRUE;
	}
      else
	{
	  bottom = R_FV1 + i;

	  if ((saved_fvars & regbit( bottom )) == 0)
	    {
	      dpush( bottom );
	      
	      must_restore_bottom = TRUE;
	    }
	}
    }
  else
    {
      bottom = R_FT1 + i;

      augment_RealRegSet( &regmaskvec, bottom );
    }

  load_float( bottom, fc, is_double );
  
  fp_reg_divide( dest, top, bottom, is_double );

  if (must_restore_bottom)
    {
      dpop( bottom );
    }
  
  return;
  
} /* fp_imm_divide */

/*}}}*/
#endif /* ! TARGET_LACKS_FP_DIVIDE */
/*{{{  show_instruction() */

/*
 * XXX - comment by NC
 *
 * This is the biggy.
 * This function is called by the code generator when it wants
 * to emit an instruction.  It is passed in the generic op code
 * 'op', two virtual source registers 'vr2' and 'vm', and a
 * virtual destination register 'vr1'
 *
 * performs 'vr1' = 'vr2' 'op' 'm'
 */

void
show_instruction(
		 J_OPCODE	op,
		 VRegInt	vr1,
		 VRegInt	vr2,
		 VRegInt	vm )
{
#ifdef TARGET_HAS_COND_EXEC
  static int32		pending_condition = Q_AL;
#endif
  int32			m   = vm.i;
  RealRegister		r1r = vr1.r;
  RealRegister		r2r = vr2.r;
  RealRegister		mr  = m;
  int32			opm;
  int32			dead;
  

  /* The types of the arguments here are rather unsatisfactory - in        */
  /* particular the last one (m) is really a big union.                    */
  
  /*
   * Is this the way to do it?  Concern over code quality - the compiler
   * clearly needs to know how to put union values of size 4 in registers.
   *  union { Symstr *sym; int32 umint; } um;
   *  um.umint = m;
   */

  if (uses_r1( op ))
    {
      if (r1r >= MAXREGNUMBER)
	{
	  syserr( syserr_r1r, (long)r1r );
	}
    }

  if (uses_r2( op ))
    {
      if (r2r >= MAXREGNUMBER)
	{
	  syserr( syserr_r2r, (long)r2r );
	}
    }

  if (uses_r3( op ))
    {
        if (mr >= MAXREGNUMBER)
	  {
	    syserr( syserr_mr, (long)mr );
	  }
      }
  
  if (debugging( DEBUG_CG ))
    {
      int r1dead = (op & J_DEAD_R1 ? '#' : ' ');
      int r2dead = (op & J_DEAD_R2 ? '#' : ' ');
      int r3dead = (op & J_DEAD_R3 ? '#' : ' ');

      
      cc_msg( "GEN: " );

      jopprint_opname( op );

      if (r1r == GAP)
	cc_msg( "GAP  " );
      else
	cc_msg( "%3ld%c ", (long)r1r, r1dead );
      
      if (r2r == GAP)
	cc_msg( "GAP  " );
      else
	cc_msg( "%3ld%c ", (long)r2r, r2dead );
      
      if (mr == GAP)
	cc_msg( "GAP\n" );
      else
	cc_msg( "%3ld%c\n", (long)mr, r3dead );
    }
  
  death = 0;
      
  if (op & J_DEAD_R1)
    death |= regbit( r1r );
      
  if (op & J_DEAD_R2)
    death |= regbit( r2r );
      
  if (op & J_DEAD_R3)
    death |= regbit( mr );
      
  /*
   * to enable future JOPCODE peephole optimisation expand_jop_macros()
   *  tries quite hard not to call show_inst() with instructions with
   *  no effect.
   */

  dead = op & J_DEADBITS;		/* Keep just in case */
  op  &= ~J_DEADBITS;                   /* ignore as yet */
  opm  = op & ~Q_MASK;

  /* Stack optimisation code */
  
  if ((r1r == R_SP) || (r2r == R_SP))
    {
      if ((opm == J_ADDK) && (r1r == r2r))
	{
	  adjust_stack( m );
	  
	  if (stack_move < 0)
	    correct_stack( TRUE );
	  
	  return;
	}
      else if ((opm == J_SUBK) && (r1r == r2r))
	{
	  adjust_stack( -m );

	  if (stack_move < 0)
	    correct_stack( TRUE );
	  
	  return;
	}
      else if ((uses_r1( op ) && r1r == R_SP) ||
	       (uses_r2( op ) && r2r == R_SP) ||
	       (uses_r3( op ) && mr  == R_SP))
	{
	  correct_stack( TRUE );
	}
    }
  else if ((stack_move < 0) &&
	   ((uses_r1( op )  && r1r == R_SP) ||	/* XXX - used to R_FP */
	    (uses_r2( op )  && r2r == R_SP) ||	/* XXX - used to R_FP */
	    (uses_r3( op )  && mr  == R_SP) ))	/* XXX - used to R_FP */
    {
      correct_stack( TRUE );
    }

  if (loads_r1( op ))
    {
      maybe_flush_pending_push( r1r );
    }

  /*
   * XXX - NC
   *
   * In the switch below, case entries should terminate with a
   * 'return' if they do not care about pending conditional execution
   * (or if they have delt with it), and they should 'break' if it
   * is an error for the J_opcode to be produced conditionally
   */
  
  switch (opm)
    {
    case J_CLRC:
      /*
       * clear memory the 'm' "bytes" of memory pointed
       * to by register 'r1r'
       * (register 'r2r' not used)
       */
      
      clear_memory( r1r, m, dead & J_DEAD_R1 );
      break;

    case J_MOVC:
      /*
       * copy the 'm' "bytes" of memory pointed
       * to by register 'r2r' to the area
       * pointed to by register 'r1r'
       */

      copy_memory( r2r, r1r, m, dead & J_DEAD_R2, dead & J_DEAD_R1 );
      break;
      
    case J_CALLK:
      /*
       * call subroutine at symbol 'm'
       * (registers 'r1r' and 'r2r' not used)
       */

      correct_stack( TRUE );

      if (stack_move)
	syserr( gen_CALL_misaligned );
      
      call( (Symstr *)m, FALSE );

      break;

    case J_TAILCALLK:
      /*
       * call subroutine at symbol 'm'
       * this call is the last instruction in the current subroutine
       * (registers 'r1r' and 'r2r' not used)
       */
      if (!new_stubs || !in_stubs)
	{
	  routine_exit( TRUE );   /* effectively includes correct_stack() */

	  if (stack_move)
	    syserr( gen_CALL_misaligned );
	}
      
      call( (Symstr *)m, TRUE );

      flush_peepholer( DBG( "tailcall" ) );
      
      break;

    case J_CALLR:	/* call subroutine at register 'mr' ????? */
      
      correct_stack( TRUE );

      flush_pending_pushes();
      
      if (stack_move)
	syserr( gen_CALL_misaligned );

      outdelayed( (OP_LAJcr << 24) | 0x1U << 21 | C_FROMQ( Q_AL ) << 16 | hardware_register( mr ),
		 examines1( mr ),
		 alters2( R_LR, RR_PC ) );
      
      /* forget about contents of address registers after a call */

      peep_corrupt_all_addr_regs();
      
      break;

    case J_TAILCALLR:	/* tail call subroutine at register 'mr' ????? */

      if ((regbit( mr ) & saved_fvars) ||
	  (regbit( mr ) & saved_ivars) )
	{
	  /* we are about to pop 'mr' off the stack! */

	  move_register( mr, R_TMP1, FALSE );

	  mr = R_TMP1;	  
	}
      
      routine_exit( TRUE );   /* effectively includes correct_stack() */

      if (stack_move)
	syserr( gen_CALL_misaligned );

      outdelayed( OP_BRcrD << 24 | 0x1U << 21 | C_FROMQ( Q_AL ) << 16 | hardware_register( mr ),
		 examines1( mr ),
		 alters1( RR_PC ) );

      flush_peepholer( DBG( "tailcall R" ) );
      
      break;

#ifdef TARGET_HAS_PROFILE
    case J_COUNT:	/* something to do with profiler ???? */
      
      correct_stack( TRUE );
      
      /* (int)r1 is ? (I would like the character on the line) ????              */
      /* (char *)r2 is the name of the file, and (int)m is the line number       */
      /* within that file. I will assume here a sensible limit on the length     */
      /* of files and hence pack these two pieces of information into a single   */
      /* 32-bit word. The structure used is count_position, and up to 16 files   */
      /* can be referenced. If there is any danger of running out of same I will */
      /* flush out the table used to decode files names and start again.         */
      
        {
	  count_position	k;


	  /* beware that the next line may flush literals etc. */

	  k.s.file = lit_of_count_name( (char *)r2 );
	  k.s.line = (unsigned int)m;
	  k.s.posn = 0;       /* Not available here */

	  flush_peepholer( DBG( "emitting count" ) );
	  
	  /* 	  outcall( countroutine ); */
	  
	  codexrefs = global_list4( SU_Xref, codexrefs, X_absreloc | (codebase + codep),
				   bindsym_( datasegment ), dataloc );

	  outcodewordaux( dataloc, LIT_RELADDR, 0 );

	  gendcI( 4, 0 );        /* Generate the slot */

	  outcodeword( k.i, LIT_NUMBER );
        }

      return;
#endif /* TARGET_HAS_PROFILE */
      
    case J_ADCON:
      /*
       * load register 'r1r' with the address of
       * symbol 'm' offset by the integer 'r2'
       */

      load_address_constant( r1r, (Symstr *)m, (int32)r2r );
      break;
      
    case J_STRING:
      /*
       * place string 'm' in literal pool
       * and place pointer to the string in register 'r1r'
       * (register 'r2r' not used)
       */

      load_string_constant( r1r );

      /*
       * strings for -Zr are placed in code segment
       */
      
      if (suppress_module == 1)
	codeseg_stringsegs( (StringSegList *)m, 1 );
      
      return;

    case J_LABEL:	/* place label in code */
	{
	  LabelNumber *	l = (LabelNumber *)m;

	  
	  if (l->block == DUFF_ADDR && l != returnlab)
	    {
	      cc_msg( "Unused label L%ld\n", lab_name_( l ) & 0x7fffffffU );
	    }
	  else
	    {
	      correct_stack( TRUE );
	      
	      flush_peepholer( DBG( "j_label" ) );
	      
	      setlabel( l );
	    }

	  /* cannot remember register contents across loops and branches */

	  peep_corrupt_all_addr_regs();
	}
      
      return;

/* XXX - flow control */
      
    case J_B:		/* conditional branch */
      correct_stack( TRUE );

#ifdef TARGET_HAS_COND_EXEC
      if (pending_condition != Q_AL)
	{
	  if ((op & Q_MASK) == Q_AL)
	    conditional_branch_to( pending_condition, (LabelNumber *)m );
	  else
	    syserr( gen_mismatched_pending, pending_condition, op & Q_MASK );
	}
      else
#endif
	{
	  conditional_branch_to( op & Q_MASK, (LabelNumber *)m );
	}

      return;
      
    case J_CASEBRANCH:	/* start of case branch */
      
      /* defer any action until the first BXX, when the default label becomes known */
      
      casebranch_pending = m;
      casebranch_r1r     = r1r;
      
      break;
      
    case J_BXX:             /* Used with case tables */
      if (casebranch_pending)
	{
	  LabelNumber *		tablelab = nextlabel();
	  int32			offset;
	  RealRegister		ar;
	  bool			saved_lr = FALSE;
	  int			num_instructions;
	  
	  
	  /* this is the first BXX following a J_CASEBRANCH */
	  
	  r1r = casebranch_r1r;

	  /* compare the value with the index of the first entry in the branch table */
	  
	  compare_integer( r1r, casebranch_pending - 1, Q_HS );   /* 1 for default */

	  /* and if we are out of range then branch to label 'm' */
	  
	  (void) conditional_branch_to( Q_HS, (LabelNumber *)m );

	  /*
	   * Note - we do not have to check for underflow, as the compiler
	   * will ensure that the table index starts from 0
	   */

	  if ((r1r == R_LR) ||				/* switch register is link register      */
	      ((saved_ivars & regbit( R_LR )) == 0) ||	/* return address was not saved on stack */
	      usedmaskvec.map[ 0 ] & regbit( R_LR )	/* link register being used as temporary */
	      )
	    {
	      /* save the return address */

	      move_register( R_LR, R_TMP2, FALSE );

	      if (r1r == R_LR)
		r1r = R_TMP2;	/* choose a different temporary */

	      saved_lr = TRUE;
	    }

	  flush_peepholer( DBG( "paranoid, moi?" ) );
	  
	  /* put PC into R_LR */

	  outinstr( OP_LAJ << 24 | (1 & 0xffffff), examines0(), alters2( R_LR, RR_PC ) );

	  /* ensure that codep is up to date */
	  
	  flush_peepholer( DBG( "getting hold of PC" ) );

	  codep_of_call = codep;

	  peep_protect_pc = 3;
	  
	  /*
	   * get address of jump table (set later)
	   *
	   * XXX - we assume this offset will fit in 8 bits
	   */

	  peep_fref( tablelab, LABREF_LIT8 );

	  /* add the offset of start of table and switch index into temp register */

	  ar = peep_get_free_addr_reg( GAP );
	  
	  /*
	   * Note that the 'num_instructions' constant in the following op code
	   * comes from that fact that there are 'n' instructions between the
	   * instruction now pointed at by the link register and the start of
	   * the jump table
	   */

	  offset = ((codep_of_call + codebase) >> 2) + 3;

	  num_instructions = saved_lr ? 3 : 2;

	  if (!fits_in_16_bits_signed( offset ))
	    num_instructions += 2;
	  
	  out_triadic_op( OP_ADDI3, ar, r1r,
			 num_instructions,
			 ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
			 examines1( r1r ),
			 alters2( ar, RR_ST ) );  /* relative to the LAJ instruction above */

	  /* add PC into address */

	  out_diadic_op( OP_ADDI, ADDR_MODE_REGISTER, ar, R_LR,
			examines2( ar, R_TMP1 ),
			alters2( ar, RR_ST ) );
	  
	  /* add PC to address in jump table and place result back in temporary */

	  out_triadic_op( OP_ADDI3, ar, R_LR, build_indirect( INDIRECT_REL, ar, 1 ),
			 ADDR_MODE_REGISTER, ADDR_MODE_INDIRECT,
			 examines2( R_LR, ar ),
			 alters2( ar, RR_ST ) );

	  /* R_LR points to following instruction */
	     
	  /* adjust offset to be relative to the LAJ instruction */
	  
	  if (fits_in_16_bits_signed( offset ))
	    {
	      out_diadic_op( OP_SUBI, ADDR_MODE_IMMEDIATE, ar, offset,
			    examines1( ar ),
			    alters2( ar, RR_ST ) );
	    }
	  else
	    {
	      RealRegister	tmp = R_TMP3;


	      /* load high part of offset */
	      
	      out_diadic_op( OP_LDHI, ADDR_MODE_IMMEDIATE, tmp, offset >> 16,
			    examines0(),
			    alters1( tmp ) );

	      /* load low part of offset */
      
	      out_diadic_op( OP_OR, ADDR_MODE_IMMEDIATE, tmp, offset,
			    examines1( tmp ),
			    alters2( tmp, RR_ST ) );

	      /* subtract offset from value in jump table */
	      
	      out_diadic_op( OP_SUBI, ADDR_MODE_REGISTER, ar, tmp,
			    examines2( ar, tmp ),
			    alters2( ar, RR_ST ) );
	    }
	  
	  flush_peepholer( DBG( "switch table 2" ) );
	  
	  if (saved_lr)
	    {
	      /* restore return address */
	      
	      move_register( R_TMP2, R_LR, FALSE );
	    }
	  
	  /* and jump */
	  
	  outinstr( OP_BRcr << 24 | C_FROMQ( Q_AL ) << 16 | hardware_register( ar ),
		   examines1( ar ),
		   alters1( RR_PC ) );
	  
	  /* start the branch table */
	  
	  flush_peepholer( DBG( "switch table 3" ) );
	  
	  setlabel( tablelab );
	  
	  /* remember to reset the casebranch_pending flag */
	  
	  casebranch_pending = 0;
	  
	  /* and codep flag */
	  
	  codep_of_call = 0;
	}
      else
	{
	  LabelNumber *	l = (LabelNumber *)m;
	  
	  
	  /* not first BXX */
	  
	  if (l == RETLAB)
	    l = returnlab;	/* yuk!! ?? */
	  
	  flush_peepholer( DBG( "generating switch table" ) );
	  
	  if lab_isset_( l )
	    {
	      /*
	       * This can occur because of code like ...
	       *
	       * label:
	       *  <code>
	       *
	       * switch (arg)
	       * {
	       * case 1: goto label;
	       * }
	       *
	       */
	      
	      codexrefs = global_list4( SU_Xref, codexrefs, X_absreloc | (codebase + codep),
				       bindsym_( codesegment ),
				       (codebase + l->u.defn & 0x00ffffffUL) / sizeof_int );
	      
	      outcodewordaux( (codebase + l->u.defn & 0x00ffffffUL) / sizeof_int,
			     LIT_ADCON, bindsym_( codesegment ) );
	    }
	  else
	    {
	      codexrefs = global_list4( SU_Xref, codexrefs, X_absreloc | (codebase + codep),
				       bindsym_( codesegment ), 0 );
	      
	      addlongfref_( l, codep | LABREF_ABS32, codexrefs );
	      
	      outcodewordaux( 0, LIT_ADCON, bindsym_( codesegment ) );
	    }
	  
	  flush_peepholer( DBG( "generating switch table 2" ) );
	}
      
      break;
      
      /* XXX - stack manipulation */
      
    case J_STACK:
      /*
       * the compiler now believes that we have 'm' bytes of
       * stack used by this function (presumably for local
       * variables). This is an informative op code and
       * should not really generate any output.
       */
      
      stack_offset = m;
      stack_move   = 0;
      
      return;
      
    case J_SETSP:
      /*
       * set the stack pointer
       * the compiler currently thinks that we have 'r2r' bytes
       * of stack allocated to local variables, and it wants us
       * to have 'm' bytes
       */
        {
	  int32	diff;
	  int32	oldstack = (int32)r2r;
	  int32	newstack = m;
	  
	  
	  if (stack_offset != oldstack)
	    syserr( syserr_setsp_confused, (long)stack_offset, (long)oldstack, (long)newstack );
	  
	  diff         = newstack - oldstack;
	  stack_offset = newstack;
	  
	  adjust_stack( - diff );
	  
	  flush_pending_pushes();
        }
      
      break;
      
#ifdef TARGET_HAS_COND_EXEC
    case J_CONDEXEC:
      pending_condition = m;
      return;
#endif
      
    case J_CMPFR:
      if (fast_FP)
	{
	  out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r2r, r2r,
			examines1( r2r ),
			alters2( r2r, RR_ST ) );
      
	  out_diadic_op( OP_RND, ADDR_MODE_REGISTER, mr, mr,
			examines1( mr ),
			alters2( mr, RR_ST ) );
	}
      
    case J_CMPDR:
      /*
       * compare the contents of register 'r2r'
       * with the contents of register 'mr'
       * (register 'r1r' not used)
       */
      
      correct_stack( TRUE );
      
      out_diadic_op( OP_CMPF, ADDR_MODE_REGISTER, r2r, mr,
		    examines2( r2r, mr ),
		    alters1( RR_ST ) );
      
      break;
      
    case J_CMPR:
      correct_stack( TRUE );
      
      out_diadic_op( OP_CMPI, ADDR_MODE_REGISTER, r2r, mr,
		    examines2( r2r, mr ),
		    alters1( RR_ST ) );
      
      break;
      
    case J_CMPK:
      /*
       * compare the contents of register 'r2r'
       * against the integer 'm'
       * (register 'r1r' not used)
       */
      
      compare_integer( r2r, m, op & Q_MASK );
      break;
      
#ifdef TARGET_HAS_FP_LITERALS      
    case J_CMPFK:
      /*
       * compare the contents of register 'r2r'
       * against the floating point constant 'm'
       * (register 'r1r' not used)
       */

      compare_float( r2r, (FloatCon *)m, op & Q_MASK, FALSE );
      break;
      
    case J_CMPDK:
      /*
       * compare the contents of register 'r2r'
       * against the floating point constant 'm'
       * (register 'r1r' not used)
       */
      
      compare_float( r2r, (FloatCon *)m, op & Q_MASK, TRUE );
      break;
#endif /* TARGET_HAS_FP_LITERALS */
      
    case J_MOVDFR:
      /*
       * move the contents of double precision register 'mr'
       * into the floating point register 'r1r'
       * (register 'r2r' not used)
       */
      
      out_diadic_op( OP_RND, ADDR_MODE_REGISTER, mr, mr,
		    examines1( mr ),
		    alters2( mr, RR_ST ) );
      
      /* drop through */
      
    case J_MOVFDR:
      /*
       * move the contents of floating point register 'mr'
       * into the double precision register 'r1r'
       * (register 'r2r' not used)
       */
      
      /* drop through */
      
    case J_MOVDR:
      /*
       * move the contents of double precision register 'mr'
       * into the double precision register register 'r1r'
       * (register 'r2r' not used)
       */
      
      /* drop through */
      
    case J_MOVFR:
      /*
       * move the contents of floating point register 'mr'
       * into the floating point register register 'r1r'
       * (register 'r2r' not used)
       */
      
#ifdef DEBUG
      if (!is_float( mr ))
	syserr( gen_non_FP_source );
      
      if (!is_float( r1r ))
	syserr( gen_non_FP_dest );
#endif
      
      /* drop through */
      
#ifdef TARGET_HAS_COND_EXEC
      if (pending_condition)
	{
	  conditional_load_float( pending_condition, ADDR_MODE_REGISTER, r1r, mr,
				 examines2( mr, RR_ST ),
				 alters1( r1r ) );
	}
      else
#endif /* TARGET_HAS_COND_EXEC */
	{
	  move_register( mr, r1r, TRUE );
	}
      
      return;
      
    case J_MOVR:
      if (r1r == mr)
	syserr( syserr_movr );
      
#ifdef TARGET_HAS_COND_EXEC
      if (pending_condition)
	{
	  conditional_load( pending_condition, ADDR_MODE_REGISTER, r1r, mr,
			   examines2( mr, RR_ST ),
			   alters1( r1r ) );
	}
      else
#endif
	{
	  move_register( mr, r1r, FALSE );
	}
      
      return;
      
    case J_MOVIDR:
      
      /*
       * move the contents of integer registers 'mr' (lsd) and 'r2r' (msd)
       * into the double precision register register 'r1r'
       */

#ifdef DEBUG
      if (!is_float( r1r ))
	syserr( gen_non_FP_dest );
#endif

      ipush( r2r );	/* big endian doubles */
      fpop(  r1r );
      
      move_register( mr, r1r, FALSE );
      
      break;
      
    case J_MOVIFR:
      /*
       * move the contents of integer register 'mr' 
       * into the floating point register register 'r1r'
       * (register 'r2r' not used)
       */

#ifdef DEBUG
      if (!is_float( r1r ))
	syserr( gen_non_FP_dest );
#endif      
      
      ipush( mr );
      fpop( r1r );
      
      peep_forget_about( r1r );
      
      break;
      
    case J_FLTDR + J_SIGNED:
#ifndef TARGET_LACKS_UNSIGNED_FIX
    case J_FLTDR /* + J_UNSIGNED */:
#endif
      /*
       * place the nearest double precision representation of
       * the contents of integer register 'mr'
       * into the double precision register 'r1r'
       * (register 'r2r' not used)
       */
      
      /* drop through */
      
    case J_FLTFR + J_SIGNED:
#ifndef TARGET_LACKS_UNSIGNED_FIX
    case J_FLTFR /* + J_UNSIGNED */:
#endif
      /*
       * place the nearest floating point representation of
       * the contents of integer register 'mr'
       * into the floating point register 'r1r'
       * (register 'r2r' not used)
       */

#ifdef DEBUG
      if (!is_float( r1r ))
	{
	  syserr( gen_non_FP_dest );
	}      
#endif
      out_diadic_op( OP_FLOAT, ADDR_MODE_REGISTER, r1r, mr,
		    examines1( mr ),
		    alters2( r1r, RR_ST ) );
      
#ifndef TARGET_LACKS_UNSIGNED_FIX
      if (!(opm & J_SIGNED))
	{
	  bool		bMustRestoreLR = FALSE;
	  RealRegister	dst;
	  
	  
	  /* get hold of a free floating point register */
	  
	  dst = get_free_register( RR_R1, RR_R2, RR_R3, RR_R6, RR_R7, RR_R10, GAP );
	  
	  /* if we must then save a register on the stack */
	  
	  if (dst == GAP)
	    {
	      dst = R_LR;
		  
	      /* can we use R_LR ? */
	      
	      if ((saved_ivars & regbit( R_LR )) == 0 ||
		  usedmaskvec.map[ 0 ] & regbit( R_LR ))	/* link register being used as temporary */
		{
		  bMustRestoreLR = TRUE;
		  
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS	      
		  dpush( R_LR );
#else
		  /* NB/ do not use move_register() as this corrupts status register */
		  maybe_flush_pending_push( R_TMP2 );
		  conditional_load( Q_AL, ADDR_MODE_REGISTER, R_TMP2, R_LR,
				   examines1( R_LR ), alters1( R_TMP2 ) );
		  peep_forget_about( R_TMP2 );
#endif
		}
	    }
	  
	  /* load 2e32 */
	  
	  load_integer( R_TMP1, 0x20000000U, TRUE );
	  
	  /* save on stack as an integer */
	  
	  ipush( R_TMP1 );
	  
	  /* pop back as a float */
	  
	  conditional_load_float( Q_AL, ADDR_MODE_INDIRECT, dst,
				 build_indirect( INDIRECT_POST_INCR, R_SP, 1 ),	/* NB/ must be same as fpop() */
				 examines1( R_SP ),
				 alters2( R_SP, dst ) );
	  
	  peep_change_addr_offset( R_SP, +1 );
	  
	  /* overwrite with 0 if the conversion went OK */
	  
	  conditional_load_float( Q_GE, ADDR_MODE_IMMEDIATE, dst, 0x8000,
				 examines1( RR_ST ),
				 alters1( dst ) );
	  
	  /* add the value as required */
	  
	  out_diadic_op( OP_ADDF, ADDR_MODE_REGISTER, r1r, dst,
			examines2( r1r, dst ),
			alters2( r1r, RR_ST ) );
	  
	  /* restore destination if necessary */
	  
	  if (bMustRestoreLR)
	    {
#ifdef TARGET_SHARES_INTEGER_AND_FP_REGISTERS	      
	      dpop( R_LR );
#else
	      move_register( R_TMP2, R_LR, FALSE );
#endif
	    }
	  
	  peep_forget_about( r1r );
	}
#endif /* ! TARGET_LACKS_UNSIGNED_FIX */
      
      if ((opm == J_FLTFR + J_SIGNED) ||
	  (opm == J_FLTFR + J_UNSIGNED) )
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ),
		      alters2( r1r, RR_ST ) );
      
      break;
      
    case J_FIXDR + J_SIGNED:
#ifndef TARGET_LACKS_UNSIGNED_FIX
    case J_FIXDR /* + J_UNSIGNED */:
#endif      
      /*
       * place the nearest integer representation of
       * the contents of double precision register 'mr'
       * into the integer register 'r1r'
       * (register 'r2r' not used)
       */

      /* drop through */
      
    case J_FIXFR + J_SIGNED:
#ifndef TARGET_LACKS_UNSIGNED_FIX
    case J_FIXFR /* + J_UNSIGNED */:
#endif      
      /*
       * place the nearest integer representation of
       * the contents of floating point register 'mr'
       * into the integer register 'r1r'
       * (register 'r2r' not used)
       */

      fix( mr, r1r, (bool) (opm & J_SIGNED) );
      break;
      
    case J_NEGDR:
    case J_NEGFR:
      /*
       * place the negated contents of floating point register 'mr'
       * into floating point register 'r1r'
       * (register 'r2r' not used)
       */

#ifdef DEBUG
      if (!is_float( mr ) || !is_float( r1r ))
	{
	  syserr( gen_non_FP_source );
	}      
#endif
      
      out_diadic_op( OP_NEGF, ADDR_MODE_REGISTER, r1r, mr,
		    examines1( mr ),
		    alters2( r1r, RR_ST ) );

      peep_forget_about( r1r );

      if (!fast_FP && opm == J_NEGFR)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ),
		      alters2( r1r, RR_ST ) );
      
      break;

    case J_NEGR:
      /*
       * place the negated contents of register 'mr'
       * into register 'r1r'
       * (register 'r2r' not used)
       */

      out_diadic_op( OP_NEGI, ADDR_MODE_REGISTER, r1r, mr,
		    examines1( mr ),
		    alters2( r1r, RR_ST ) );
      
      peep_forget_about( r1r );
      break;
      
    case J_NOTR:
      /*
       * place the inverted contents of register 'mr'
       * into register 'r1r'
       * (register 'r2r' not used)
       */

      out_diadic_op( OP_NOT, ADDR_MODE_REGISTER, r1r, mr,
		    examines1( mr ),
		    alters2( r1r, RR_ST ) );
      
      peep_forget_about( r1r );
      break;
      
    case J_MOVDK:
      /*
       * load the double precision value 'm' into
       * register double precision register 'r1r'
       * (register 'r2r not used)
       */

#ifdef TARGET_HAS_COND_EXEC
      if (pending_condition != Q_AL)
	{
	  int32	val = convert_to_C40_format( (FloatCon *)m, TRUE );


	  if (val == 0xFFFFFFFFU)
	    syserr( gen_FP_value_not_fit );

	  conditional_load_float( pending_condition, ADDR_MODE_IMMEDIATE, r1r, val,
				 examines1( RR_ST ),
				 alters1( r1r ) );
	}
      else
#endif /* TARGET_HAS_COND_EXEC */
	{	  
	  load_float( r1r, (FloatCon *)m, TRUE );
	}

      return;
      
    case J_MOVFK:
      /*
       * load the floating point value 'm' into
       * register floating point register 'r1r'
       * (register 'r2r not used)
       */

#ifdef TARGET_HAS_COND_EXEC
      if (pending_condition != Q_AL)
	{
	  int32	val = convert_to_C40_format( (FloatCon *)m, FALSE );


	  if (val == 0xFFFFFFFFU)
	    syserr( gen_FP_value_not_fit );

	  conditional_load_float( pending_condition, ADDR_MODE_IMMEDIATE, r1r, val,
				 examines1( RR_ST ),
				 alters1( r1r ) );
	}
      else
#endif /* TARGET_HAS_COND_EXEC */
	{	  
	  load_float( r1r, (FloatCon *)m, FALSE );
	}
      
      return;
      
    case J_MOVK:
      /*
       * load the integer 'm' into
       * register 'r1r'
       * (register 'r2r not used)
       */

#ifdef TARGET_HAS_COND_EXEC
      if (pending_condition != Q_AL)
	{
	  if (fits_in_16_bits_signed( m ))
	    {
	      conditional_load( pending_condition, ADDR_MODE_IMMEDIATE, r1r, m,
			       examines1( RR_ST ),
			       alters1( r1r ) );
	    }
	  else
	    {
	      load_integer( R_TMP1, m, TRUE );
	      
	      conditional_load( pending_condition, ADDR_MODE_REGISTER, r1r, R_TMP1,
			       examines2( R_TMP1, RR_ST ),
			       alters1( r1r ) );
	    }
	}
      else
#endif /* TARGET_HAS_COND_EXEC */
	{
	  load_integer( r1r, m, FALSE );
	}
      
      return;

/* XXX - binary ops */
      
    case J_SHRK + J_SIGNED:	
    case J_SHRK: /* + J_UNSIGNED: */
      /*
       * shift the contents of register 'r2r'
       * right by 'm' and place the result
       * in register 'r1r'
       */
      
      m = -m;

      /* drop through */

    case J_SHLK + J_SIGNED:
    case J_SHLK: /* + J_UNSIGNED: */
      
      /*
       * shift the contents of register 'r2r'
       * left by 'm' and place the result
       * in register 'r1r'
       */
      
      shift_register( (op & J_SIGNED) ? TRUE : FALSE, r1r, r2r, m ); 
      break;
		     
#ifdef TARGET_HAS_FP_LITERALS
    case J_SUBFK:
      /*
       * subtract floating point value 'm' from
       * the contents of register 'r2r' and place
       * result in register 'r1r'
       */

      float_immediate_op( OP_SUBF, r1r, r2r, (FloatCon *)m, FALSE );

      if (!fast_FP)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ),
		      alters2( r1r, RR_ST ) );      
      break;
      
    case J_SUBDK:
      /*
       * subtract double precision value 'm' from
       * the contents of register 'r2r' and place
       * result in register 'r1r'
       */

      float_immediate_op( OP_SUBF, r1r, r2r, (FloatCon *)m, TRUE );
      break;
#endif /* TARGET_HAS_FP_LITERALS */
      
    case J_SUBK:
      /*
       * subtract 'm' from the contents of
       * register 'r2r' and place result
       * in register 'r1r'
       */

      if (r2r == R_SP)
	flush_pending_pushes();

      if (is_word_addressed_( r2r ))
	{
	  RealRegister	ar;
	  int32		off;
	  bool		biased;

	  
	  if (m & (sizeof_int - 1))
	    syserr( gen_non_word_offset );

	  ar = peep_find_loaded_addr_reg( r2r, &off, &biased );
	      
	  m /= sizeof_int;
	  
	  if (ar     != GAP   &&
	      biased == FALSE &&
	      !no_peepholing   )
	    {
	      m -= off;
	  
	      if (m == 0)
		{
		  peepf( "eliminated unnecessary integer load" );

		  ++peep_eliminated;
		  
		  move_register( ar, r1r, FALSE );
		}
	      else if (fits_in_8_bits_signed( m ))
		{
		  peepf( "eliminated unnecessary integer load" );

		  ++peep_eliminated;
	      
		  out_triadic_op( OP_SUBI3, r1r, ar, m, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
				 examines1( ar ),
				 alters2( r1r, RR_ST ) );
		}
	      else
		{
		  integer_immediate_op( OP_SUBI, OP_SUBI3, r1r, r2r, (m + off) * sizeof_int, TRUE );
		}
	    }
	  else
	    {
	      integer_immediate_op( OP_SUBI, OP_SUBI3, r1r, r2r, m * sizeof_int, TRUE );
	    }

	  convert_to_byte_offset( r1r );
	}
      else if (m == 0)
	{
	  move_register( r2r, r1r, FALSE );
	}
      else if (m < 0)
	{
	  integer_immediate_op( OP_ADDI, OP_ADDI3, r1r, r2r, -m, TRUE );
	}
      else
	{
	  integer_immediate_op( OP_SUBI, OP_SUBI3, r1r, r2r, m, TRUE );
	}
      
      break;

#ifdef TARGET_HAS_FP_LITERALS
    case J_ADDFK:
      /*
       * add floating point value 'm' to the
       * contents of register 'r2r' and place
       * the result in register 'r1r'
       */

      float_immediate_op( OP_ADDF, r1r, r2r, (FloatCon *)m, FALSE );

      if (!fast_FP)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ),
		      alters2( r1r, RR_ST ) );
      
      break;
      
    case J_ADDDK:
      /*
       * add double precision value 'm' to the
       * contents of register 'r2r' and place
       * the result in register 'r1r'
       */

      float_immediate_op( OP_ADDF, r1r, r2r, (FloatCon *)m, TRUE );
      break;
      
#endif /* TARGET_HAS_FP_LITERALS */
      
    case J_ADDK:
      /*
       * add 'm' to the contents of
       * register 'r2r' and place the
       * result in register 'r1r'
       */

      if (r2r == R_SP)
	flush_pending_pushes();
      
      if (is_word_addressed_( r2r ))
	{
	  RealRegister	ar;
	  int32		off;
	  bool		biased;

	  
	  if (m & (sizeof_int - 1))
	    syserr( gen_non_word_offset );
	  
	  m /= sizeof_int;

	  ar = peep_find_loaded_addr_reg( r2r, &off, &biased );

	  if (ar     != GAP   &&
	      biased == FALSE &&
	      !no_peepholing   )
	    {
	      m -= off;
	  
	      if (m == 0)
		{
		  peepf( "eliminated unnecessary integer load" );

		  ++peep_eliminated;
	      
		  move_register( ar, r1r, FALSE );
		}
	      else if (fits_in_8_bits_signed( m ))
		{
		  peepf( "eliminated unnecessary integer load" );

		  ++peep_eliminated;
	      
		  out_triadic_op( OP_ADDI3, r1r, ar, m, ADDR_MODE_REGISTER, ADDR_MODE_IMMEDIATE,
				 examines1( ar ),
				 alters2( r1r, RR_ST ) );
		}
	      else
		{
		  integer_immediate_op( OP_ADDI, OP_ADDI3, r1r, r2r, (m + off) * sizeof_int, TRUE );
		}
	    }
	  else
	    {
	      integer_immediate_op( OP_ADDI, OP_ADDI3, r1r, r2r, m * sizeof_int, TRUE );
	    }

	  convert_to_byte_offset( r1r );
	}
      else if (m == 0)
	{
	  move_register( r2r, r1r, FALSE );
	}
      else if (m < 0)
	{
	  integer_immediate_op( OP_SUBI, OP_SUBI3, r1r, r2r, -m, TRUE );
	}
      else
	{
	  integer_immediate_op( OP_ADDI, OP_ADDI3, r1r, r2r, m, TRUE );
	}

      break;

#ifdef TARGET_HAS_FP_LITERALS
    case J_MULFK:
      /*
       * multiply the floating point value 'm' with the
       * contents of register 'r2r' and place
       * the result in register 'r1r'
       */

      float_immediate_op( OP_MPYF, r1r, r2r, (FloatCon *)m, FALSE );

      if (!fast_FP)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ),
		      alters2( r1r, RR_ST ) );      
      break;
      
    case J_MULDK:
      /*
       * multiply the double precision value 'm' with the
       * contents of register 'r2r' and place
       * the result in register 'r1r'
       */

      float_immediate_op( OP_MPYF, r1r, r2r, (FloatCon *)m, TRUE );
      break;

#ifndef TARGET_LACKS_FP_DIVIDE
    case J_DIVDK:
      /* r1r = r2r / m */      
      fp_imm_divide( r1r, r2r, (FloatCon *)m, TRUE );
      break;
      
    case J_DIVFK:
      fp_imm_divide( r1r, r2r, (FloatCon *)m, FALSE );
      break;
#endif /* !TARGET_LACKS_FP_DIVIDE */
#endif /* TARGET_HAS_FP_LITERALS */
      
      /*
       * the front-end maps MULK to MULR if TARGET_LACKS_MULTIPLY_LITERALS,
       * and DIVK/REMK to DIVR/REMR if TARGET_LACKS_DIVIDE_LITERALS.
       * TARGET_LACKS_MULDIV_LITERALS is equivalent to setting both of these.
       * MULR/DIVR/REMR get mapped to function calls unless TARGET_HAS_MULTIPLY,
       * TARGET_HAS_DIVIDE etc. Beware also CONFIG_HAS_MULTIPLY in mcdep.c
       */

    case J_MULK:
      /*
       * multiply contents of register 'r2r'
       * by 'm' and place the results
       * in register 'r1r'
       */

      multiply_integer( r1r, r2r, m );
      break;
      
    case J_ANDK:
      /*
       * perform a bitwise AND between 'm' and
       * the contents of register 'r2r' and
       * place the result in register 'r1r'
       */

      /*
       * XXX - NB we cannot translate J_AND into a OP_TSTB
       * because we do not know when the destination register
       * is really dead.
       *
       * cf below where in both cases R2 is marked as dead and R1 is alive
       *
       * int func( int arg ) {
       * arg &= 0xff;	     
       * return (arg & 0x80) ? 1 : 2; }
       */
        
      integer_immediate_op( OP_AND, OP_AND3, r1r, r2r, m, FALSE );

      break;
  
    case J_ORRK:
      /*
       * preform a bitwise OR between 'm' and
       * the contents of register 'r2r' and
       * place the result in register 'r1r'
       */
      
      integer_immediate_op( OP_OR, OP_OR3, r1r, r2r, m, FALSE );
      break;
      
    case J_EORK:
      /*
       * perform a bitwise XOR between 'm' and
       * the contents of register 'r2r' and
       * place the result in register 'r1r'
       */
      
      integer_immediate_op( OP_XOR, OP_XOR3, r1r, r2r, m, FALSE );
      break;

    case J_ADDDR:
    case J_ADDFR:
      /*
       * add contents of register 'r2r' to
       * the contents of register 'mr' and
       * place the result in register 'r1r'
       */
#ifdef DEBUG
      if (!is_float( r1r ) || !is_float( mr ) || !is_float( r2r ))
	{
	  syserr( gen_non_FP_source );
	}
#endif
      register_op( OP_ADDF, OP_ADDF3, r1r, r2r, mr, TRUE );

      if (!fast_FP && opm == J_ADDFR)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ),
		      alters2( r1r, RR_ST ) );
      
      /* no need to check for word addressing */
      
      break;
      
    case J_ADDR:
      if (r2r == R_SP)
	flush_pending_pushes();
      
      register_op( OP_ADDI, OP_ADDI3, r1r, r2r, mr, TRUE );
      
      if (is_word_addressed_( r2r ))
	{
	  convert_to_byte_offset( r1r );
	}
      
      break;
      
    case J_SUBFR:
    case J_SUBDR:
      /*
       * subtract the contents of register 'mr' from
       * the contents of register 'r2r' and place
       * the result in register 'r1r'
       */
      
#ifdef DEBUG
      if (!is_float( r1r ) || !is_float( mr ) || !is_float( r2r ))
	{
	  syserr( gen_non_FP_source );
	}
#endif
      register_op( OP_SUBF, OP_SUBF3, r1r, r2r, mr, FALSE );

      /* no need to check for word addressing */

      if (!fast_FP && opm == J_SUBFR)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ),
		      alters2( r1r, RR_ST ) );
      break;
      
    case J_SUBR:
      if (r2r == R_SP)
	flush_pending_pushes();

      register_op( OP_SUBI, OP_SUBI3, r1r, r2r, mr, FALSE );
      
      if (is_word_addressed_( r2r ))
	{
	  convert_to_byte_offset( r1r );
	}
      
      break;
      
    case J_MULDR:
    case J_MULFR:
      /*
       * multiply contents of register 'r2r'
       * by contents of register 'mr' and
       * place the results in register 'r1r'
       */      
#ifdef DEBUG
      if (!is_float( r1r ) || !is_float( mr ) || !is_float( r2r ))
	{
	  syserr( gen_non_FP_source );
	}
#endif
      register_op( OP_MPYF, OP_MPYF3, r1r, r2r, mr, TRUE );

      if (!fast_FP && opm == J_MULFR)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ),
		      alters2( r1r, RR_ST ) );
      break;

    case J_MULR:
      register_op( OP_MPYI, OP_MPYI3, r1r, r2r, mr, TRUE );
      break;

#ifndef TARGET_LACKS_FP_DIVIDE
    case J_DIVDR:
    case J_DIVFR:
      /* r1r = r2r / mr */
      
      fp_reg_divide( r1r, r2r, mr, opm == J_DIVDR );
      break;
#endif /* ! TARGET_LACKS_FP_DIVIDE */
      
    case J_ANDR:
      /*
       * perform a bitwise AND between the contents of register 'mr'
       * and the contents of register 'r2r' and place the result in
       * register 'r1r'
       */

      register_op( OP_AND, OP_AND3, r1r, r2r, mr, TRUE );
      break;
      
    case J_ORRR:
      /*
       * perform a bitwise OR between the contents of register 'mr'
       * and the contents of register 'r2r' and place the result in
       * register 'r1r'
       */

      register_op( OP_OR, OP_OR3, r1r, r2r, mr, TRUE );
      break;
      
    case J_EORR:
      /*
       * perform a bitwise XOR between the contents of register 'mr'
       * and the contents of register 'r2r' and place the result in
       * register 'r1r'
       */

      register_op( OP_XOR, OP_XOR3, r1r, r2r, mr, TRUE );
      break;
      
    case J_SHLR + J_SIGNED:
      /*
       * arithmetically shift contents of register 'r2r'
       * left by the amount in register 'mr' and
       * place the result in register 'r1r'
       */
      
      register_op( OP_ASH, OP_ASH3, r1r, r2r, mr, FALSE );
      break;
      
    case J_SHLR: /* + J_UNSIGNED: */
      /*
       * logically shift contents of register 'r2r'
       * left by the amount in register 'mr' and
       * place the result in register 'r1r'
       */

      register_op( OP_LSH, OP_LSH3, r1r, r2r, mr, FALSE );
      break;
      
    case J_SHRR + J_SIGNED:
      /*
       * arithmetically shift contents of register 'r2r'
       * right by the amount in register 'mr' and
       * place the result in register 'r1r'
       */

      if (r2r == r1r)
	{
	  /* negate 'mr' and move into R_TMP1 */
      
	  out_diadic_op( OP_NEGI, ADDR_MODE_REGISTER, R_TMP1, mr,
			examines1( mr ),
			alters2( R_TMP1, RR_ST ) );

	  mr = R_TMP1;	  
	}
      else
	{
	  /* negate 'mr' and move into 'r1r' */
      
	  out_diadic_op( OP_NEGI, ADDR_MODE_REGISTER, r1r, mr,
			examines1( mr ),
			alters2( r1r, RR_ST ) );

	  mr = r1r;
	}
      
      /* 'r1r' = 'r2r' >> 'mr' */
      
      out_triadic_op( OP_ASH3, r1r, r2r, mr, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( mr, r2r ),
		     alters2( r1r, RR_ST ) );
      
      peep_forget_about( r1r );
      
      break;
      
    case J_SHRR: /* + J_UNSIGNED: */
      /*
       * logically shift contents of register 'r2r'
       * right by the amount in register 'mr' and
       * place the result in register 'r1r'
       */

      if (r2r == r1r)
	{	  
	  /* negate 'mr' and move into R_TMP1 */
      
	  out_diadic_op( OP_NEGI, ADDR_MODE_REGISTER, R_TMP1, mr,
			examines1( mr ),
			alters2( R_TMP1, RR_ST ) );

	  mr = R_TMP1;	  
	}
      else
	{
	  /* negate 'mr' and move into 'r1r' */
      
	  out_diadic_op( OP_NEGI, ADDR_MODE_REGISTER, r1r, mr,
			examines1( mr ),
			alters2( r1r, RR_ST ) );

	  mr = r1r;
	}
      
      /* 'r1r' = 'r2r' >> 'mr' */
      
      out_triadic_op( OP_LSH3, r1r, r2r, mr, ADDR_MODE_REGISTER, ADDR_MODE_REGISTER,
		     examines2( mr, r2r ),
		     alters2( r1r, RR_ST ) );
      
      peep_forget_about( r1r );
      break;

/* XXX loads and stores to memory */

    case J_LDRDK:
      /*
       * load register 'r1r' with the "double precision value"
       * pointed to by register 'r2r'
       * offset by 'm'
       */

      load_double_relative( r1r, r2r, m, dead & J_DEAD_R2 );
      break;

    case J_LDRFK:
      /*
       * load register 'r1r' with the "floating point value"
       * pointed to by register 'r2r'
       * offset by 'm'
       */

      out_immediate_offset( OP_LDF, r1r, r2r, m, dead & J_DEAD_R2 );
      break;

    case J_LDRK:
      /*
       * load register 'r1r' with the "integer"
       * pointed to by register 'r2r'
       * offset by 'm'
       */

      /* catch a special case that occurs because of our word addressing */
      
      if (r1r == r2r && !no_peepholing && peep_reg_transfer_to( r2r ))
	{
	  r2r = peep_eliminate_reg_transfer( r2r );

	  peepf( "eliminated register transfer" );

	  ++peep_eliminated;
	}
      
      out_immediate_offset( OP_LDI, r1r, r2r, m, dead & J_DEAD_R2 );
      break;

    case J_LDRWK: /* + J_UNSIGNED:*/
    case J_LDRWK + J_SIGNED:
      /*
       * load register 'r1r' with the "half-word"
       * pointed to by register 'r2r'
       * offset by 'm'
       */
      
      non_word_op( r1r, r2r, m, FALSE, opm == J_LDRWK + J_SIGNED, TRUE, TRUE );
      break;

    case J_LDRBK: /* + J_UNSIGNED:*/
    case J_LDRBK + J_SIGNED:
      /*
       * load register 'r1r' with the "byte"
       * pointed to by register 'r2r'
       * offset by 'm'
       */

      non_word_op( r1r, r2r, m, FALSE, opm == (J_LDRBK + J_SIGNED), FALSE, TRUE );
      break;
     
    case J_STRDK:
      /*
       * store the contents of register 'r1r'
       * into the "double precision value" pointed at by register 'r2r'
       * offset by 'm'
       */

      store_double_relative( r1r, r2r, m, dead & J_DEAD_R2 );
      break;
      
    case J_STRFK:
      /*
       * store the contents of register 'r1r'
       * into the "floating point value" pointed at by register 'r2r'
       * offset by 'm'
       */

      if (fast_FP)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ), alters2( r1r, RR_ST ) );
      
      out_immediate_offset( OP_STF, r1r, r2r, m, dead & J_DEAD_R2 );
      break;
      
    case J_STRK:
      /*
       * store the contents of register 'r1r'
       * into the "word" pointed at by register 'r2r'
       * offset by 'm'
       */

      if (!no_peepholing && peep_reg_transfer_to( r2r ))
	{
	  RealRegister	src = peep_eliminate_reg_transfer( r2r );

	  
	  out_immediate_offset( OP_STI, r1r, src, m, dead & J_DEAD_R2 );

	  move_register( src, r2r, FALSE );

	  peepf( "swapped order of register transfer and store" );

	  ++peep_swapped;
	}
      else

	{
	  out_immediate_offset( OP_STI, r1r, r2r, m, dead & J_DEAD_R2 );
	}
      break;
      
    case J_STRWK:
      /*
       * store the contents of register 'r1r'
       * into the "half-word" pointed at by register 'r2r'
       * offset by 'm'
       */

      non_word_op( r1r, r2r, m, FALSE, FALSE, TRUE, FALSE );
      break;
      
    case J_STRBK:
      /*
       * store the contents of register 'r1r'
       * into the "byte" pointed at by register 'r2r'
       * offset by 'm'
       */

      non_word_op( r1r, r2r, m, FALSE, FALSE, FALSE, FALSE );
      break;

    case J_LDRDR:
      /*
       * load register 'r1r' with the "double precision value"
       * pointed to by register 'r2r'
       * offset by the contents of register 'mr'
       */

      load_double_indirect( r1r, r2r, m );
      break;
      
    case J_LDRFR:
      /*
       * load register 'r1r' with the "floating point value"
       * pointed to by register 'r2r'
       * offset by the contents of register 'mr'
       */

      out_register_offset( OP_LDF, r1r, r2r, m );
      break;
      
    case J_LDRR:
      /*
       * load register 'r1r' with the "word"
       * pointed to by register 'r2r'
       * offset by the contents of register 'mr'
       */

      out_register_offset( OP_LDI, r1r, r2r, m );
      break;
      
    case J_LDRWR: /* + J_UNSIGNED:*/
    case J_LDRWR + J_SIGNED:
      /*
       * load register 'r1r' with the "half-word"
       * pointed to by register 'r2r'
       * offset by the contents of register 'mr'
       */

      non_word_op( r1r, r2r, mr, TRUE, opm == J_LDRWR + J_SIGNED, TRUE, TRUE );
      break;

    case J_LDRBR: /* + J_UNSIGNED:*/
    case J_LDRBR + J_SIGNED:
      /*
       * load register 'r1r' with the "byte"
       * pointed to by register 'r2r'
       * offset by the contents of register 'mr'
       */

      non_word_op( r1r, r2r, mr, TRUE, opm == J_LDRBR + J_SIGNED, FALSE, TRUE );
      break;
      
    case J_STRDR:
      /*
       * store the contents register 'r1r'
       * into the "double precision value" pointed at by register 'r2r'
       * offset by the contents of register 'mr'
       */

      store_double_indirect( r1r, r2r, m );
      break;
      
    case J_STRFR:
      /*
       * store the contents register 'r1r'
       * into the "floating point value" pointed at by register 'r2r'
       * offset by the contents of register 'mr'
       */

      if (fast_FP)
	out_diadic_op( OP_RND, ADDR_MODE_REGISTER, r1r, r1r,
		      examines1( r1r ), alters2( r1r, RR_ST ) );
      
      out_register_offset( OP_STF, r1r, r2r, m );
      break;
      
    case J_STRR:
      /*
       * store the contents register 'r1r'
       * into the "word" pointed at by register 'r2r'
       * offset by the contents of register 'mr'
       */

      out_register_offset( OP_STI, r1r, r2r, m );
      break;
      
    case J_STRWR:
      /*
       * store the contents register 'r1r'
       * into the "half-word" pointed at by register 'r2r'
       * offset by the contents of register 'mr'
       */

      non_word_op( r1r, r2r, mr, TRUE, FALSE, TRUE, FALSE );
      break;
      
    case J_STRBR:
      /*
       * store the contents register 'r1r'
       * into the "byte" pointed at by register 'r2r'
       * offset by the contents of register 'mr'
       */

      non_word_op( r1r, r2r, mr, TRUE, FALSE, FALSE, FALSE );
      break;
      
    case J_ENDPROC:
      /*
       * reached end of function
       */

      if (!lab_isset_( returnlab ) && returnlab->u.frefs != NULL)
	{
	  flush_peepholer( DBG( "J_ENDPROC" ) );
	  
	  conditional_branch_to( Q_AL, RETLAB );
	}

      /* NB/ flush peepholer before dumping literals */
      
      flush_peepholer( DBG( "end of procedure" ) );

#ifdef TARGET_HAS_DEBUGGER
      if (usrdbg( DBG_ANY ) /* && !new_stubs */ && !in_stubs)
	debugger_end_of_function();
#endif
      dumplits2( 0 );
      
      asm_lablist = (LabList *)dreverse( (List *)asm_lablist );
      
      dump_count_names();

      peepf( "instructions: eliminated %ld, transformed %ld, swapped %ld",
	    peep_eliminated, peep_transformed, peep_swapped );

      peep_eliminated = peep_transformed = peep_swapped = 0;

      break;
      
    case J_ENTER:
      /*
       * reached start of function
       */
      
      asm_lablist = NULL;
      
      routine_entry( m );
      
      break;
      
    case J_PUSHM:
      /*
       * save the registers indicate by the bits set in 'm' onto the stack
       */
      
      if (m != 0)
	{
	  int32	i;

	  /*
	   * NB/ push in REVERSE order to the pops.
	   * If we have peepholing disabled, each push will be emitted
	   * straight away, and so they will be in the correct order.
	   * If peepholing is enabled then each push will be prepended
	   * to the list, and the list will be emptied from the tail
	   * backwards, so emitting the pushes in reverse order.
	   * If pushes are later added to the list (by futher J_PUSH[M|F|D]
	   * op codes), then they will be prepended as well, and hence they
	   * will be in the right order as well.
	   *
	   * NB/ beware if number of registers > 32
	   */

	  for (i = 32; i--;)
	    {
	      if (m & regbit( i ))
		add_pending_push( i, PUSH_INT );
	    }	  
	}
      
      break;
            
    case J_PUSHF:
#ifdef DEBUG
      if (!is_float( r1r ))
	{
	  syserr( gen_bad_float_push );
	}
#endif
      add_pending_push( r1r, PUSH_FLOAT );
      break;

    case J_PUSHD:
#ifdef DEBUG
      if (!is_float( r1r ))
	{
	  syserr( gen_bad_float_push );
	}
#endif      
      add_pending_push( r1r, PUSH_DOUBLE );
      break;

    case J_POPM:
      /*
       * restore the registers indicated by the bits set in 'm' from the stack
       */

      while (m)
	{
	  RealRegister	r = firstbit( m );

	  
	  m ^= regbit( r );

	  if (pop_pending_push( r, (bool) (m & regbit( r + 1 )) ) == 2)
	    {
	      m ^= regbit( r + 1 );
	    }
	}
      
      break;
      
#ifdef TARGET_HAS_SIGN_EXTEND
    case J_EXTEND:
      /*
       * sign extend the contents of r2r into r1r
       * the value in m is
       * 0 for byte  to short extensions
       * 1 for byte  to int   extensions
       * 2 for short to int   extensions
       */

      switch (m)
	{
	case 0:
	  syserr( gen_no_byte_to_short );
	  break;

	case 1:
	  /* this instruction is an LB0 r2r, r1r */
	  
	  outinstr( FUNC_LOAD << 28 | OP_LB << 24 | 0x0U << 23 | ADDR_MODE_REGISTER << 21 |
		   hardware_register( r1r ) << 16 | hardware_register( r2r ),
		   examines1( r2r ),
		   alters2( r1r, RR_ST ) );
	  break;

	case 2:
	  /* this instruction is an LH0 r2r, r1r */
	  
	  outinstr( FUNC_LOAD << 28 | OP_LH << 24 | 0x0U << 23 | ADDR_MODE_REGISTER << 21 |
		   hardware_register( r1r ) << 16 | hardware_register( r2r ),
		   examines1( r2r ),
		   alters2( r1r, RR_ST ) );
	  break;
	  
	default:
	  syserr( gen_unknown_extend, m );
	  break;
	}
      
      break;
#endif /* TARGET_HAS_SIGN_EXTEND */

#ifdef TARGET_HAS_DEBUGGER
    case J_INFOLINE:
      flush_peepholer( DBG( "J_INFOLINE" ) );

      debugger_add_line( m );

      do_notify_command( m, vr2.str );

      break;

    case J_INFOBODY:
      break;

    case J_INFOSCOPE:
      break;
      
#endif /* TARGET_HAS_DEBUGGER */

    case J_WORD:
      flush_peepholer( DBG( "inline op code" ) );
      
      outinstr( m, examines0(), alters2( RR_R0, RR_ST ) );

      flush_peepholer( DBG( "inline op code" ) );

      break;
      
    default:
      syserr( syserr_show_inst, (long)op );
      
      nop( FALSE );		/* placeholder */

      break;
    }

#ifdef TARGET_HAS_COND_EXEC
  if (pending_condition != Q_AL)
    {
      syserr( gen_pending_cond_exec, opm );
    }
#endif /* TARGET_HAS_COND_EXEC */
  
  return;
  
} /* show_instruction */

/*}}}*/
/*{{{  branch_round_literals() */

/* the next routine is required for the machine independent codebuf.c */

void
branch_round_literals( LabelNumber * m )
{
  conditional_branch_to( Q_AL, m );
    
  return;
    
} /* branch_round_literals */

/*}}}*/
/*{{{  mcdep_init() */

void
mcdep_init( void )
{
  peep_init();

  if (usrdbg( DBG_ANY ))
    saved_regs = sym_insert( "saved_regs", s_identifier );
  
  return;
    
} /* mcdep_init */

/*}}}*/
/*{{{  localcg_tidy() */

void
localcg_tidy( void )
{
  peep_tidy();

  if (usrdbg( DBG_ANY ))
    saved_regs = NULL;
  
  return;
    
} /* localcg_tidy */

/*}}}*/
/*{{{  localcg_reinit() */

void
localcg_reinit( void )
{
  return;
    
} /* localcg_reinit */

/*}}}*/

/*}}}*/

/* End of gen.c */
