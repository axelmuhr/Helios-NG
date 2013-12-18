
/* C compiler file sparc/target.h :  Copyright (C) Codemist Ltd., 1991. */
/* Copyright (c) 1991 Perihelion Software Ltd.
   All Rights Reserved */
/* version 5 */
/* $Header: /hsrc/cmds/cc/ncc/cc450/c40/RCS/target.h,v 1.77 1993/07/29 13:35:12 nickc Exp $ */

#ifndef _c40target_LOADED
#define _c40target_LOADED 1

#define TARGET_MACHINE 			"C40"
#define TARGET_PREDEFINES 		{ "__C40", "__JMP_BUF_SIZE=22" }

   #define TARGET_IS_C40				1

/* #define TARGET_IS_BIG_ENDIAN    			0 */	/* but doubles are big endian ! */
   #define TARGET_IS_LITTLE_ENDIAN    			1  	/* because of C40 byte instructions */

#if !(defined TARGET_HAS_AOUT || defined TARGET_HAS_COFF)
   #define TARGET_HAS_AOUT 				1	/* XXX we do not actually care about this */
#endif

   #define target_coff_magic 				0577     /* inspected if TARGET_HAS_COFF */

   #define TARGET_HAS_DEBUGGER				1	/* It may not be DBX, but hey! */

   #define TARGET_HAS_BLOCKMOVE 			1	/* XXX - well, we are faking it */
   #define TARGET_HAS_COND_EXEC				1 	/* can do conditionally execution */
   #define TARGET_HAS_FP_LITERALS			1	/* FP constants can be included in OP codes */
   #define TARGET_HAS_IEEE 				1	/* XXX - not quite true ... */
   #define TARGET_HAS_MULTIPLY 				1   	/* but see also config_init in mcdep.c */
   #define TARGET_HAS_SEPARATE_CODE_DATA_SEGS		1	/* because of seperqte address buses */
   #define TARGET_HAS_SIGN_EXTEND			1	/* XXX we can fake it */
   #define TARGET_HAS_TAILCALL 				1	/* yup we can do tailcalls */
   #define TARGET_HAS_TAILCALLR 			1	/* XXX - does this work ? */
   #define TARGET_ONLY_HAS_CONDITIONAL_LOAD		1	/* but only for load instructions */

/* #define TARGET_HAS_2ADDRESS_CODE 			0 */	/* implies only supports diadic operations */
/* #define TARGET_HAS_370_FP				0 */	/* IBM370 specific */
/* #define TARGET_HAS_BSS 				0 */	/* XXX - do we ? */
/* #define TARGET_HAS_BYTE_INSTRUCTIONS			0 */	/* indicates instructions are byte sized */
/* #define TARGET_HAS_DIVIDE   				0 */
/* #define TARGET_HAS_DIVREM_FUNCTION			0 */
/* #define TARGET_HAS_EBCDIC				0 */
/* #define TARGET_HAS_HALFWORD_INSTRUCTIONS		0 */	/* indicates that instructions can be 16 bits */
/* #define TARGET_HAS_LINKER_NAME_LIMIT			0 */	/* not that I know of */
/* #define TARGET_HAS_NEGATIVE_INDEXING			0 */	/* XXX - what does this do ? */
/* #define TARGET_HAS_NON_FORTRAN_DIVIDE		0 */	/* XXX - we do not have any divide ops */
/* #define TARGET_HAS_OTHER_IEEE_ORDER			0 */	/* IEEE doubles are stored big-endian */
/* #define TARGET_HAS_PROFILE 				0 */	/* XXX - not for now */
/* #define TARGET_HAS_RISING_STACK			0 */	/* XXX - added by NC, see c40/flowgraf.c */
/* #define TARGET_HAS_ROTATE                            0 */	/* C40 rotates not sufficient for this */
/* #define TARGET_HAS_SCALED_ADDRESSING			0 */	/* indicates that load offsets can be in bytes, words, etc */
/* #define TARGET_HAS_SCALED_OPS			0 */	/* XXX - what does this do ? */
/* #define TARGET_HAS_SCCK				0 */	/* m68k specific */
/* #define TARGET_HAS_SWITCH_BRANCHTABLE 		0 */	/* indicates that target has special support for switching */

   #define TARGET_ADDRESSES_UNSIGNED			1
   #define TARGET_ALLOWS_COMPARE_CSES			1 
   #define TARGET_COUNT_IS_PROC 			1
   #define TARGET_FP_ARGS_IN_FP_REGS			1	/* oh yes ! */
   #define TARGET_LACKS_DIVIDE_LITERALS 		1
   #define TARGET_LINKER_OMITS_DOLLAR 			1	/* XXX - what does this do ? */
   #define TARGET_USES_NEW_STUBS			1	/* New style linker generated stubs used */

/* #define TARGET_CALL_USES_DESCRIPTOR			0 */
/* #define TARGET_CORRUPTS_SWITCH_REGISTER 		0 */	/* switch might corrupt index register */
/* #define TARGET_FLAGS_VA_CALLS			0 */	/* IBM370 specific */
/* #define TARGET_GEN_NEEDS_VOLATILE_INFO		0 */	/* do we ? */
/* #define TARGET_INLINES_MONADS			0 */	/* XXX - what does this do ? */
/* #define TARGET_LACKS_3WAY_COMPARE			0 */
/* #define TARGET_LACKS_FP_DIVIDE			0 */	/* we can simulate it efficiently */
/* #define TARGET_LACKS_HALFWORD_STORE			0 */	/* implies that we cannot store half words */
/* #define TARGET_LACKS_MULDIV_LITERALS			0 */	/* cannot multiply or divide by a constant */
/* #define TARGET_LACKS_MULTIPLY_LITERALS		0 */
/* #define TARGET_LACKS_REMAINDER			0 */
/* #define TARGET_LACKS_RIGHTSHIFT			0 */
/* #define TARGET_LACKS_ROL				0 */	/* no Rotate Left instruction */
/* #define TARGET_LACKS_RR_STORE			0 */	/* XXX - what does this do ? */
/* #define TARGET_LACKS_UNSIGNED_FIX 			0 */
/* #define TARGET_LDRK_MAX 				0 */	/* maximum permitted pointer offset */
/* #define TARGET_LDRK_MIN 				0 */	/* maximum permitted negative pointer offset */
/* #define TARGET_LDRK_QUANTUM				0 */	/* superceeded by LDRK_MIN and LDRK_MAX */
/* #define TARGET_NULL_BITPATTERN			0 */	/* defined elsewhere */
/* #define TARGET_R0_ALWAYS_ZERO 			0 */	
/* #define TARGET_SHARES_INTEGER_AND_FP_REGISTERS	0 */ 	/* removed on advice from AM */
/* #define TARGET_STACKS_LINK				0 */	/* link address is placed on stack */
/* #define TARGET_STACK_MOVES_ONCE			0 */	/* Hitachi specific */
/* #define TARGET_STRUCT_RESULT_REGISTER		0 */	/* names register used to return structs */
/* #define TARGET_SWITCH_isdense			0 */	/* advanced tuning for switch statements */

/* #define ADDRESS_REG_STUFF 				0 */	/* indicates seperate address and data regs - we dont support this */
/* #define RANGECHECK_SUPPORED				0 */	/* only for FORTRAN / PASCAL */

/* #define DO_NOT_OPTIMISE_CHAR_AND_SHORT_ARITHMETIC 	0 */	/* XXX - experimentally put in ?? */

   #define NO_INSTORE_FILES				1 
   #define UNIQUE_DATASEG_NAMES				1

#if defined __hp9000s700 || defined _AIX
#define REVERSE_OBJECT_BYTE_ORDER			1 	/* big endian host */
#endif

/* #define SOFTWARE_FLOATING_POINT			0 */	/* defined if no FP support in op codes */


/*
 * XXX - NC
 *
 * Register Ordering
 * Version:		7
 * Date:		29/2/92
 *
 * Naming Scheme:
 * -------------
 * Name:	| R0 |  R1 |  R2 |  R3 | | R4 |  R5 | | R6 |  R7 | | R8 |  R9 | R10 | | R11 |
 *               ----------------------   ----------   ----------   ----------------   -----
 * Number:	|  0 |   1 |   2 |   3 | |  4 |   5 | |  6 |   7 | | 10 |  11 |  12 | |  17 |
 *
 *
 * Name:	| AR0 | AR1 | AR2 | AR3 | | AR4 | AR5 | AR6 | AR7 |
 *		 -----------------------   -----------------------
 * Number:	|  13 |  14 |  15 |  16 | |  18 |  19 |  20 |  21 |
 *
 *
 * Name:	| DP | BK | | IR0 | IR1 | | SP | ST | | RS | RE | RC | | PC |
 *		 ---------   -----------   ---------   --------------   ----
 * Number:	|  8 |  9 | |  22 |  23 | | 24 | 25 | | 26 | 27 | 28 | | 30 |
 */

#define RR_R0	 0
#define RR_R1	 1
#define RR_R2	 2
#define RR_R3	 3
#define RR_R4	 4
#define RR_R5	 5
#define RR_R6	 6
#define RR_R7	 7
#define RR_R8	10
#define RR_R9	11
#define RR_R10	12
#define RR_R11	17
#define RR_AR0	13
#define RR_AR1	14
#define RR_AR2	15
#define RR_AR3	16
#define RR_AR4	18
#define RR_AR5	19
#define RR_AR6	20
#define RR_AR7	21
#define RR_DP	 8
#define RR_IR0	22
#define RR_IR1	23
#define RR_BK	 9
#define RR_SP	24
#define RR_ST	25
#define RR_RS	26
#define RR_RE	27
#define RR_RC	28
#define RR_PST	29	/* pseudo ST register, used by peepholer register checkers */
#define RR_PC	30	/* pseudo register used by peepholer register checkers */


/*
 * Usage:
 * -----
 *
 * Arguments:			R0,  R1, R2,  R3	(arguments passed in to function)
 * Variables:			DP,  BK, R8,  R9	(saved on function entry)
 * Temporaries:			R10			(saved before function call)
 * Floating Point Arg:		R4, R5			(must not intersect variable or temporary registers)
 * Floating Point Var:		R6, R7	 		(must not intersect argument or temporary registers)
 * Floating Point Tmp:  				(must not intersect argument or variable  registers)
 * Link:			R11			(honorary temporary and variable register)
 * Address:			AR0, AR1, AR2, AR3	(address registers or temporary registers)
 * Universal Temporaries:	RS, RE, RC		(NOT available to front end)
 * Module Table Pointer:	AR4			(NOT available to front end)
 * Temporary address:		AR5			(NOT available to front end)
 * User Stack Pointer:		AR6			
 * Frame Pointer:       	AR7
 * Addressing Base:		IR0		(NB/ this is hard coded in gen.c)
 * Stack End Pointer:		IR1		(see load_static_data_ptr() in c40/gen.c)
 * System Stack Pointer:	SP		(used by interrupt handlers)
 */

/*
 * XXX - following definitions must agree with those in:
 *
 * /helios/include/ampp/c40.m
 * c40/gen.c -> hardware_register()
 *           -> get_free_register()
 *
 */

#define R_A1  		RR_R0	/* first argument register */
#define R_A1result 	RR_R0	/* result of function left here (must be able to hold a double) */
#define R_TMP1		RR_RS	/* first  universal temporary register */
#define R_TMP2		RR_RE	/* second universal temporary register */
#define R_TMP3		RR_RC	/* third  universal temporary register */
#define R_T1  		RR_R10	/* first temporary register */
#define R_V1  		RR_DP	/* first variable register */
#define R_FA1		RR_R4	/* first floating point argument register */
#define R_F1		RR_R4	/* first floating point register */
#define R_FV1		RR_R6	/* first floating point variable register */
#define R_FT1		-1	/* first floating point temporary register */

#define R_IP		RR_AR3	/* XXX WHAT IS THIS REGISTER ? */ /* cf regalloc.c line 2131 */
#define R_LR  		RR_R11 	/* Link register */ /* NB/ must be visible to compiler */
#define R_ADDR1		RR_AR0	/* first address register (also used by heliobj.c when initialising data) */
#define R_MT		RR_AR4	/* module table pointer */	/* NB/ linker/genimage.c relies on this */
#define R_ATMP		RR_AR5	/* temporary address register *//* NB/ linker/genimage.c relies on this */
#define R_SP		RR_AR6	/* stack pointer */
#define R_FP		RR_AR7	/* frame pointer */
#define R_BASE		RR_IR0	/* addressing base */
#define R_SE		RR_IR1	/* stack end pointer */
#define R_PC		XXX	/* program counter - this register does not exist */
#define R_DS		RR_AR2	/* second temporary address register, need during data initialisation */

#define NARGREGS	 4L	/* number of argument registers, should really be called NINTARGREGS */
#define NVARREGS	 4L	/* number of variable registers, should really be called NINTVARREGS */
#define NTEMPREGS	 6L	/* number of temporary registers, ... */  /* XXX see comment below */
#define MAXGLOBINTREG	17L	/* maximum index of integer registers ??? */

#define NFLTARGREGS	 2L	/* number of floating point argument registers */
#define NFLTVARREGS	 2L	/* number of floating point variable registers */
#define NFLTTEMPREGS	 0L	/* number of floating point variable registers */
#define MAXGLOBFLTREG	 7L	/* maximum index of floating point registers  */
#define NFLTREGS	 4L	/* number of floating point registers */

#define MAXREGNUMBER	28L	/* maximum legal value of a register */

#define NINTREGS	18L	/* number of "integer" registers */
#define NMAGICREGS	18L	/* maximum valid bit number for a compiler used register (see mip/defs.h) */

#define mask_of_regs_( start, length ) (regbit( start + length ) - regbit( start ))

#define M_VARREGS	mask_of_regs_( R_V1,  NVARREGS )
#define M_FVARREGS	mask_of_regs_( R_FV1, NFLTVARREGS )
#define M_INT_ARG_REGS  mask_of_regs_( R_A1,  NARGREGS )
#define M_FLT_ARG_REGS  mask_of_regs_( R_FA1, NFLTARGREGS )
#define M_ALL_ARG_REGS  (M_INT_ARG_REGS | M_FLT_ARG_REGS)
  
/* This macro is used by c40/gen.c to see if a register has a word address or a byte offset */

#define is_word_addressed_( r )	((r) == R_SP || (r) == R_FP)

/*
 * XXX - NC - 2/10/91
 *
 * What, I hear you cry, is going on ?
 *
 * Well, the situation is this.  mip/regalloc,c uses NMAGICREGS
 * as the NUMBER of registers available to the compiler, and it
 * expects to find this many registers in the ALLOCATION_ORDER
 * array.  What is more it expects the registers in this array
 * to be numbered from 0 to (NMAGICREGS - 1) in some indeterminate
 * order with NO OVERLAPS.  What is more the compiler also expects
 * the special register R_LR (the link register) to be a valid
 * register (ie with a number less than NMAGICREGS), BUT it does
 * not expect to find R_LR in the ALLOCATION_ORDER array!
 *
 * How do I cope, I hear you ask, (and for that matter, how on
 * earth do any of the other targets cope) ?  Well, I do not
 * know how other targets cope, but I have decided to make the link
 * register be one of the TEMPORARY registers available to the
 * compiler.  This way if it is used, it will be saved before calling
 * any other functions.  I have placed R_LR last
 * in the allocation order in the hopes that it will not be used
 * very often.  This does mean that NINTTMPREGS above is set to 6
 * rather than 5 (as might be assumed from the register allocation
 * scheme), so that the compiler knows that R_LR is a variable
 * register.
 */

#define ALLOCATION_ORDER {\
			   RR_R0,  RR_R1,  RR_R2,  RR_R3,	/* params in */   	\
			   RR_R10,		   		/* tmp       */   	\
			   RR_DP,  RR_BK,  RR_R8,  RR_R9,	/* var       */   	\
			   RR_AR0, RR_AR1, RR_AR2, RR_AR3, 	/* address or temp */  	\
			   RR_R4,  RR_R5,			/* flt params */	\
			   RR_R6,  RR_R7,			/* flt var */		\
			   RR_R11,				/* link register */	\
			   255 }

/* theses tests apply to the register numbering as defined by the 'C40, not the scheme above */

#define	is_extended_precision_register( r )	(((r) >= 0x00 && (r) <= 0x07) || ((r) >=  0x1c && (r) <= 0x1f))
#define	is_address_register( r )		 ((r) >= 0x08 && (r) <= 0x0F)
#define	is_index_register( r )			 ((r) >= 0x11 && (r) <= 0x12)

/*
 * decides if register is one of:
 *	address, index, SP, BK, DP
 */

#define	is_special_register( r )		 ((r) >= 0x08 && (r) <= 0x14)

/*
 * XXX - NC
 *
 * Oh Yez! Oh Yez! Oh Yez!
 *
 * Now hear this!
 *
 * We are going to have all pointers be BYTE offsets from
 * a base register (IR0), (where a BYTE is an 8 bit quantity)
 */

#define sizeof_char		1	/* a bit of a tautology, but what the hell */
#define sizeof_short		2
#define sizeof_int      	4
#define sizeof_long		4
#define sizeof_ptr		4
#define sizeof_float		4
#define sizeof_double		8
#define sizeof_ldble		8

#define alignof_short		2
#define alignof_int     	4
#define alignof_long		4
#define alignof_ptr		4
#define algn_of_float		4
#define alignof_double  	4
#define alignof_ldble		4
#define alignof_max		4
#define alignof_struct  	4
#define alignof_toplevel	4

#define NUM_BITS_PER_BYTE	8

/*
 * Finallly a word about doubles.
 *
 * When doubles are passed as arguments, they are passed in register pairs.
 * The pair is arranged in BIG ENDIAN order, (ie the lowered number register
 * of the pair contains the more significant value), because ....
 *
 * When doubles are pushed onto the stack they must be pushed low part then
 * high part, (since the instruction to load the high part (LDF) will overwrite
 * the low part, but the instruction to load the low part (LDI) will not
 * overwrite the high part).  This allows the doubles to be popped as high part
 * followed by low part.  Hence accessing doubles saved on the stack must also
 * use this same method - load high followed by low.  Thus argument registers,
 * when they are saved onto the stack, must be sure to conform to the same
 * convention.  Argument registers are pushed onto the stack in reverse order
 * (higher numbered registers first).  So the higher numbered register of
 * a double containing register pair must contain the lower significant part of
 * the double, as it will be pushed onto the stack first.  Hence a big endian
 * arrangement.
 */
 
#endif /* _c40target_LOADED */

/* end of c40/target.h */
