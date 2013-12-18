/*
 * peep.h - Header file for the C40 peepholer
 *
 *   Copyright (c) 1992 Perihelion Software Ltd.
 *   All rights reserved.
 *
 * Author :	N Clifton
 * Version :	$Revision: 1.15 $
 * Date :	$Date: 1994/06/07 12:28:28 $
 * Id :		$Id: peep.h,v 1.15 1994/06/07 12:28:28 nickc Exp $
 */

#ifndef _peep_h
#define _peep_h

#include "target.h"	/* for register aliases and target name */
#include "host.h"	/* for int32 */
#include "defs.h"	/* for Symstr */
#include "cgdefs.h"	/* for RealRegister */

#ifdef DEBUG
#define DBG( x )	x
#else
#define DBG( x )
#endif

#define	indirect_addr_reg( op )		(((op) >> 8) & 0x07)
#define real_addr_reg( reg )		((reg) + hardware_register( RR_AR0 ))

#define examines0()			0
#define examines1( r1 )			 regbit( r1 )
#define examines2( r1, r2 )		(regbit( r1 ) | regbit( r2 ))
#define examines3( r1, r2, r3 )		(regbit( r1 ) | regbit( r2 ) | regbit( r3 ))
#define examines4( r1, r2, r3, r4 )	(regbit( r1 ) | regbit( r2 ) | regbit( r3 ) | regbit( r4 ))
#define alters0()			examines0()
#define alters1( r1 )			examines1( r1 )
#define alters2( r1, r2 )		examines2( r1, r2 )
#define alters3( r1, r2, r3 )		examines3( r1, r2, r3 )
#define alters4( r1, r2, r3, r4 )	examines4( r1, r2, r3, r4 )

#define is_float( reg ) 	is_C40_float_register( reg )

#define is_op(      instruction, op_code )	 (((instruction) >> 23)        == (op_code))
#define is_mode(    instruction, mode )		((((instruction) >> 21) & 0x3) == (mode))
#define is_diadic(  instruction )		 (((instruction) >> 29)        == 0)
#define is_triadic( instruction )		 (((instruction) >> 29)        == 1)
#define is_normal(  instruction )		 (((instruction) >> 30)        == 0)
#define dest_of(    instruction )		 (((instruction) >> 16) & 0x1f)
#define source_of(  instruction )		  ((instruction) & 0xffff)
#define has_indirect_side_effects( instruction )((((instruction) >> 11) & 0x6) != 0)

#define is_load(    instruction )		(is_op( instruction, OP_LDI ) || \
						 is_op( instruction, OP_LDA ) || \
						 is_op( instruction, OP_LDF ) )

#define is_monadic( instruction )		(is_op( instruction, OP_ABSF  ) || \
						 is_op( instruction, OP_ABSI  ) || \
  						 is_op( instruction, OP_FIX   ) || \
  						 is_op( instruction, OP_FLOAT ) || \
  						 is_op( instruction, OP_NEGF  ) || \
  						 is_op( instruction, OP_NEGI  ) || \
  						 is_op( instruction, OP_NOT   ) )
typedef enum
  {
    OUT_NULL,		/* a padding op-code that can potentially be eliminated 		*/
    OUT_INSTR,		/* normal op-code 							*/
    OUT_DELAYED,	/* a delayed instruction 						*/
    OUT_SYMREF,		/* op-code that references a symbol 					*/
    OUT_DELSYMREF,	/* (delayed) op-code that references a symbol 				*/
    OUT_XREF,		/* op-code that cross references a symbol 				*/
    OUT_SYMXREF,	/* op-code that references and cross references a symbol 		*/
    OUT_DELSYMXREF,	/* (delayed) op-code that references and cross references a symbol 	*/
    OUT_LABREF,		/* op-code that references a label 					*/
    OUT_DELLABREF	/* (delayed) op-code that references a label				*/
  }
peep_type;

typedef enum
  {
    PUSH_INT,
    PUSH_FLOAT,
    PUSH_DOUBLE
  }
push_type;


/* functions */

#ifdef __HELIOS
#pragma -v1
#endif

/* in gen.c */
extern void	IOdebug(			const char * format, ... );

/* in peep.c */
void		peepf( 				const char *, ... );

#ifdef __HELIOS
#pragma -v0
#endif

/* address register peepholing */

void		peep_change_addr_offset( 	RealRegister, int32 );
void		peep_forget_about( 		RealRegister );
void		peep_corrupt_addr_reg(		RealRegister );
void		peep_corrupt_all_addr_regs(	void );
void		peep_note_addr_reg_loaded(	RealRegister, RealRegister, int32, bool );
RealRegister	peep_find_loaded_addr_reg(	RealRegister, int32 *, bool * );
RealRegister	peep_get_free_addr_reg(		RealRegister );
void		peep_init_addr_regs(		int32 );

/* push peepholing */

void		add_pending_push(		RealRegister, push_type );
int		pop_pending_push(		RealRegister, bool );
void		flush_pending_pushes(		void );
void		maybe_flush_pending_push(	RealRegister );

/* instruction peepholing */

void		peep_xref( 			int32, Symstr * );
void		peep_symref(			Symstr * );
void		peep_fref(			LabelNumber *, int32 );
int		peep_shift_back(		int );
void		append_peep(			peep_type, int32, int32, int32, Symstr *, LabelNumber *,int32 );
bool		peep_reg_transfer_to(		RealRegister  );
bool		peep_sets_status_reg(		RealRegister  );
bool		peep_refs_label(		LabelNumber * );
RealRegister	peep_eliminate_reg_transfer(	RealRegister  );
void		flush_peepholer(
#ifdef DEBUG
				const char *
#else
				void
#endif
				);

/* initialisation */

void		peep_init( 			void );
void		peep_tidy(			void );

/* variables */

/* in peep.c */
extern int32	peep_protect_pc;		/* number of instructions to protect  */
extern int32	peep_eliminated;		/* number of instructions eliminated  */
extern int32	peep_transformed;		/* number of instructions transformed */
extern int32	peep_swapped;			/* number of instructions swapped     */
extern int32	death;				/* registers dead at end of execution of current J_opcode */

/* in gen.c */
extern int32	stack_move;
extern int32	stack_offset;

#endif /* _peep_h */

/* do not put anything beyond this #endif */

/* @@ emacs customization */

/* Local Variables: */
/* mode: c */
/* End: */
