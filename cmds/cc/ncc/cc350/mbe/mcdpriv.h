
/*
 * C compiler file mcdpriv.h :  Copyright (C) Codemist Ltd
 * (Private interfaces within machine-dependent back end).
 * version 1
 *
 * Modified by Nicholas Clifton
 *
 * Copyright (c) 1992 Perihelion Software Ltd.
 *  All Rights Reserved.
 *
 * RCS Id: $Id: mcdpriv.h,v 1.3 1994/06/02 16:30:27 nickc Exp $
 */

#ifndef __mcdpriv_h
#define __mcdpriv_h 1

#include "xrefs.h"	/* for ExtRef */
#include "jopcode.h"	/* for J_OPCODE */

/* exported functions */

/* from gen.c: */
extern int32		hardware_register( 		RealRegister r );
extern bool		is_C40_float_register(		RealRegister );
extern RealRegister	real_register(			int32 );
extern int32		build_op(			int32, int32, RealRegister, int32 );
extern void		move_register(			RealRegister, RealRegister, bool );
extern void		ipush(				RealRegister );
extern void		ipop(				RealRegister );
extern void		fpush(				RealRegister );
extern void		dpush(				RealRegister );
extern bool		fpliteral(			FloatCon * val, J_OPCODE op );
extern void		correct_stack(			bool );
extern void		load_static_data_ptr(		RealRegister r, bool iscode, Symstr * symbol );
extern bool		is_function(			Symstr * );
extern void		prepare_for_initialisation(	int data_init, LabelNumber * data_label );
extern void		prepare_for_function_exporting( void );
extern void		export_function(		Symstr * name, RealRegister dest );
extern void		prepare_for_data_exporting(	RealRegister );
extern void		prepare_for_block_copying(	LabelNumber * );
extern void		block_copy_data(		int32 num_words_to_cpoy );
extern void		store_data_value(		unsigned32 value );
extern void		export_data_symbol(		Symstr *, int32, unsigned32 );
extern void		finished_exporting(		void );
extern RealRegister	local_base(			Binder * );
extern int32		local_address(			Binder * );
extern void		setlabel(			LabelNumber * );
extern bool		target_isaddrreg(		RealRegister );
extern bool		immed_cmp(			int32 );
extern void         	show_instruction(		J_OPCODE op, VRegInt r1, VRegInt r2, VRegInt m );
extern void		branch_round_literals(		LabelNumber * );
extern void		mcdep_init(			void );
extern void		localcg_tidy(			void );
extern void		localcg_reinit(			void );

/* from asm.c: */
#ifdef __HELIOS
#pragma -v1
extern void		asmf( 				const char * format, ... );
#pragma -v0
#else
extern void		asmf( 				const char * format, ... ); 
#endif
extern signed long	mask_and_sign_extend_word(	unsigned long, unsigned long );
extern void		decode_instruction(		int32, bool );
extern void		display_assembly_code(		Symstr * );
extern void		asm_header(			void );
extern void		asm_trailer(			void );

/* from heliobj.c */
extern void	  	request_new_stub(		Symstr * );
extern void	  	request_addr_stub(		Symstr * );
extern void		obj_codewrite(			Symstr * );
extern void		request_stub(			Symstr * );
extern int32		obj_symref(			Symstr *, int, int32 );
extern void		obj_init(			void );
extern void		obj_header(			void );
extern void		obj_makestubs(			void );
extern void		obj_trailer(			void );
extern int32		IEEE_to_single_float( 		int32 );
extern int32		IEEE_to_extended_float(		int32, unsigned32, int32 * );

/* from mcdep.c */
extern void		config_init(			void );
extern bool		mcdep_config_option(		char, char tail[] );
extern char *		file_name(			FILE * );


#ifdef __HELIOS
#pragma -v1
#endif

/* from peep.c */
extern void		debug(				const char *, ... );

#ifdef __HELIOS
#pragma -v0
#endif

#ifdef TARGET_HAS_DEBUGGER

/* from debug.c: */
extern void		db_init( 			char * );
extern void		db_tidy(			void );
extern void 		debugger_add_line(		int32 );
extern void		debugger_end_of_function(	void );      
extern LabelNumber *	debugger_filenamelabel(		char * );
extern void		debugger_end_of_function(	void );
extern VoidStar		dbg_notefileline(		FileLine );
extern void		dbg_locvar(			Binder *, FileLine, bool );
extern void		dbg_proc(			Symstr *, TypeExpr *, bool, FileLine );
extern void		dbg_type(			Symstr *, TypeExpr * );
extern void		dbg_topvar(			Symstr *, int32, TypeExpr *, bool, FileLine );
extern void		dbg_init(			void );
extern void		debugger_add_line(		int32 );
extern void		dbg_enterproc(			void );
extern void		dbg_locvar1(			Binder * );
extern bool		dbg_scope(			BindListList *, BindListList * );
extern void		dbg_xendproc(			FileLine );

#endif /* TARGET_HAS_DEBUGGER */

#ifndef __SPARC
/* other: */
extern unsigned32	abs(				int32 );
#endif

/* variables */

/* in heliobj.c */
extern FILE *		objstream;
extern ExtRef *		obj_symlist;
extern CodeXref *	codexrefs;
extern DataXref *	dataxrefs;
extern int 		suppress_module;
extern ExtRef *		datasymbols;
extern LabelNumber *	exporting_routines;

/* in asm.c */
extern FILE *		asmstream;
extern Symstr *		sym_reladdr;

/* in gen.c */

#ifdef TARGET_HAS_DEBUGGER
extern int32 		saved_regs_offsets[ MAXREGNUMBER + 1 ];
#endif

/* in mcdep.c */
extern bool    		in_stubs;
extern int		split_module_table;
extern int32		config;

/* in debug.c */
extern int    		usrdbgmask;
extern char		dbg_name[ 4 ];
extern Symstr *		current_proc;
extern LabelNumber *	proc_label;

/* macros */

#define annotations 			  (feature & FEATURE_ANNOTATE)
#define new_stubs			(!(feature & FEATURE_OLD_STUBS))

#define xr_definition			(xr_defloc | xr_defext)
#define xr_external_code		(xr_code   | xr_defext)
#define is_defined( x )			 ((x)->extflags & xr_definition)
#define is_defined_( flags )		 ((flags)       & xr_definition)
#define is_code( x )			 ((x)->extflags & xr_code)
#define is_code_( flags )		 ((flags)       & xr_code)
#define is_data( x )			 ((x)->extflags & xr_data)
#define is_external_code( x )		(((x)->extflags & xr_external_code) == xr_external_code)
#define is_local( x )			 ((x)->extflags & xr_defloc)
#define is_global( x )			 ((x)->extflags & xr_defext)
#define is_bss( x )			 ((x)->extflags & xr_bss)

#define fits_in_5_bits_signed( val )	(((val) & 0xfffffff0U) == 0 || ((val) & 0xfffffff0U) == 0xfffffff0U)
#define fits_in_8_bits_signed( val )	(((val) & 0xffffff80U) == 0 || ((val) & 0xffffff80U) == 0xffffff80U)
#define fits_in_8_bits_unsigned( val )	(((val) & 0xffffff00U) == 0 )
#define fits_in_16_bits_signed( val )	(((val) & 0xffff8000U) == 0 || ((val) & 0xffff8000U) == 0xffff8000U)
#define fits_in_16_bits_unsigned( val )	(((val) & 0xffff0000U) == 0 )
#define fits_in_24_bits_signed( val )	(((val) & 0xff800000U) == 0 || ((val) & 0xff800000U) == 0xff800000U)

#ifdef __HELIOS
#define use( x )	x = x
#else
#define use( x )
#endif

/* constants */

#ifndef TRUE
#define TRUE 	1U
#define FALSE 	0U
#endif

/* addressing modes for forw. refs. */

#define LABREF_OFF16    0x00000000
#define LABREF_OFF24    0x01000000
#define LABREF_LIT16    0x02000000
#define LABREF_LIT8     0x03000000
#define LABREF_ABS32    0x04000000
#define LABREF_NONE	0x05000000	/* used by peep hole type changer */


#define build_indirect( mode, reg, disp )	\
	((((mode) & 0x1fU) << 11) | ((hardware_register( reg ) & 0x7) << 8) | ((disp) & 0xffU))

/*
 * NB/ for triadic addressing use the build_indirect macro
 */

#define build_parallel_indirect( mode_or_disp, reg )	\
	((((mode_or_disp) & 0x1f) << 3) | (hardware_register( reg ) & 0x7))

#define INDIRECT_PRE_ADD	    B_00000 /*  *+AR(x)                 addr = ARn + x                        */
#define INDIRECT_PRE_SUB	    B_00001 /*  *-AR(x)	                addr = ARn - x                        */
#define INDIRECT_PRE_INCR	    B_00010 /*  *++AR(x)    ARn += x,   addr = ARn                            */
#define INDIRECT_PRE_DECR	    B_00011 /*  *--AR(x)    ARn -= x,   addr = ARn                            */
#define INDIRECT_POST_INCR	    B_00100 /*  *AR++(x)                addr = ARn,      ARn += x             */
#define INDIRECT_POST_DECR	    B_00101 /*  *AR--(x)                addr = ARn,      ARn -= x             */
#define INDIRECT_POST_INCR_CIRC	    B_00110 /*  *AR++(x)%               addr = ARn,      ARn += x, ARn %= BK  */
#define INDIRECT_POST_DECR_CIRC	    B_00111 /*  *AR--(x)%               addr = ARn,      ARn -= x, ARn %= BK  */
#define INDIRECT_PRE_ADD_IR0	    B_01000 /*  *+AR(IR0)               addr = ARn + IR0                      */
#define INDIRECT_PRE_SUB_IR0	    B_01001 /*  *-AR(IR0)               addr = ARn - IR0                      */
#define INDIRECT_PRE_INCR_IR0	    B_01010 /*  *++AR(IR0)  ARn += IR0, addr = ARn                            */
#define INDIRECT_PRE_DECR_IR0	    B_01011 /*  *--AR(IR0)  ARn -= IR0, addr = ARn                            */
#define INDIRECT_POST_INCR_IR0	    B_01100 /*  *AR++(IR0)              addr = ARn,      ARn += IR0           */
#define INDIRECT_POST_DECR_IR0	    B_01101 /*  *AR--(IR0)	        addr = ARn,      ARn -= IR0           */
#define INDIRECT_POST_INCR_CIRC_IR0 B_01110 /*  *AR++(IR0)%             addr = ARn,      ARn += IR0, ARn %= BK*/
#define INDIRECT_POST_DECR_CIRC_IR0 B_01111 /*  *AR--(IR0)%             addr = ARn,      ARn -= IR0, ARn %= BK*/
#define INDIRECT_PRE_ADD_IR1	    B_10000 /*  *+AR(IR1)               addr = ARn + IR1                      */
#define INDIRECT_PRE_SUB_IR1	    B_10001 /*  *-AR(IR1)               addr = ARn - IR1                      */
#define INDIRECT_PRE_INCR_IR1	    B_10010 /*  *++AR(IR1)  ARn += IR1, addr = ARn                            */
#define INDIRECT_PRE_DECR_IR1	    B_10011 /*  *--AR(IR1)  ARn -= IR1, addr = ARn                            */
#define INDIRECT_POST_INCR_IR1	    B_10100 /*  *AR++(IR1)              addr = ARn,      ARn += IR1           */
#define INDIRECT_POST_DECR_IR1	    B_10101 /*  *AR--(IR1)              addr = ARn,      ARn -= IR1           */
#define INDIRECT_POST_INCR_CIRC_IR1 B_10110 /*  *AR++(IR1)%             addr = ARn,      ARn += IR1, ARn %= BK*/
#define INDIRECT_POST_DECR_CIRC_IR1 B_10111 /*  *AR--(IR1)%             addr = ARn,      ARn -= IR1, ARn %= BK*/

#define INDIRECT_REL		    B_11000 /*  *AR	                 addr = ARn                           */
#define INDIRECT_POST_INCR_BITR	    B_11001 /*  *AR++(IR0)B	         addr = ARn,      ARn = B( ARn + IR0 )*/

#endif /* __mcdpriv_h */
