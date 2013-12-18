	TTL  Helios Kernel assembler support functions                  > asm/s
	;    Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom
	; ---------------------------------------------------------------------
	; history:	910120	JGS	Seperated from "asm.a" (ampp/as
	;				version) since required major work on
	;				stack overflow handlers.
	;
	; We should also check (and ensure) that the code uses the PCS
	; correctly (ie. when indexing from the frame-pointer "fp").
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; -- Standard Helios include files ------------------------------------
	GET	listopts.s	; assembly listing control directives
	GET	arm.s		; ARM processor description
	GET	basic.s		; default labels and variables
	GET	structs.s	; structure construction MACROs
	GET	module.s	; Helios object module construction MACROs
	GET	error.s		; Helios error manifests
	GET	queue.s		; Helios queue structures
	GET	memory.s	; Helios memory structures
	GET	task.s		; Helios Task structure
	; -- Executive specific include files ---------------------------------
	GET	exmacros.s	; Executive MACROs
	GET	exstruct.s	; Executive structures
        GET     SWIinfo.s	; Executive SWI definitions
        GET     ROMitems.s	; BLOCK and ITEM formats
        GET     manifest.s	; Executive manifests
	GET	PCSregs.s	; standard PCS register definitions

	; ---------------------------------------------------------------------
	; This file should be assembled with the "-library" argument to hobjasm
	LIB			; we are part of a library
	ALIGN			; ensure word-aligned code

	; ---------------------------------------------------------------------
	; -- Register interrogation -------------------------------------------
	; ---------------------------------------------------------------------
	; Return original link register value from callers caller
_linkreg	FnHead
	; PCS defined fp points to return link of calling function
	LDR	a1,[fp,#-4]		; load the return address
	MOVS	pc,lr

	; ---------------------------------------------------------------------
	; Return current module table pointer
_dpreg		FnHead
	MOV	a1,dp			; return the current module table ptr
	MOVS	pc,lr

	; ---------------------------------------------------------------------
	; Return current frame-pointer
_fpreg		FnHead
	MOV	a1,fp			; return the current frame-pointer
	MOVS	pc,lr

	; ---------------------------------------------------------------------
	; Return original stack pointer value from callers caller
_spreg		FnHead
	; PCS defined fp points to lr of calling fn, sp, fp, args...
	LDR	a1,[fp,#-8]		; recover stacked "sp" of caller
	MOVS	pc,lr

	; ---------------------------------------------------------------------
	; Return current module table pointer
_GetModTab	FnHead			; identical to "_dpreg"
	MOV	a1,dp
	MOVS	pc,lr

	; ---------------------------------------------------------------------
	; -- Error codes ------------------------------------------------------
	; ---------------------------------------------------------------------
	; Return pointer to errorcode array
InitErrorCodes	FnHead
	ADR	a1,ErrorCodes		; a1 = address of "ErrorCodes" table
	MOVS	pc,lr

ErrorCodes	; internal label of table of Nasty constants
	&	&81058001	; 0 Err_Timeout 
	&	&A1098003	; 1 Err_BadPort 
	&	&A1098004	; 2 Err_BadRoute 
	&	&C1098012	; 3 Err_DeadLink 
	&	&A1010000	; 4 Err_NoMemory 
	&	&810B8004	; 5 Err_Congestion 
	&	&E10E0004	; 6 Err_Kill 
	&	&C10E0005	; 7 Err_Abort 
	&	&810B8003	; 8 Err_NotReady 
	&	&A1098012	; 9 Err_BadLink 

	; ---------------------------------------------------------------------
	; -- Stack Extension --------------------------------------------------
	; ---------------------------------------------------------------------
	
	LABREF	_Task_		; pointer to current task structure
	IMPORT	CallException	; function to raise C signal
	IMPORT	AllocMem	; Helios Kernel memory allocator "malloc"
	IMPORT	FreeMem		; Helios Kernel memory allocator "free"
	; We need to work out the MAXIMUM stack usage possible by the above
	; functions. They need to be compiled with stack overflow checking
	; DISABLED. At the moment the complete Kernel is compiled with stack
	; checking disabled, but this should be changed to only those functions
	; that actually require no stack-checking.

	; ---------------------------------------------------------------------

stack_increment	*	(4096)	; Initial default value.
				; We should really base this value on the
				; original stack allocated to the process when
				; it was started.
				; The Helios memory allocator we are using is
				; optimised for fairly large chunks, so this
				; should actually be the minimum extension
				; chunk given.

	; ---------------------------------------------------------------------

__stack_overflow	FnHead
	; Called when we have a standard register saving only stack-limit
	; check.
	MOV	ip,sp	; ensure we can calculate the stack required
	; and fall through to...
__stack_overflow_1
	; Called when we have a large local stack allocation (>256bytes with
	; the current Helios C compiler (ncc349 901024)) which will overflow.
	; This version is called when the compiler generated PCS has checked
	; its requirement against the stack-limit.
	;
	; in:	sp = current stack-pointer (beneath stack-limit)
	;	sl = current stack-limit
	;	ip = low stack point we require for the current function
	;	lk = return address into the current function
	;	fp = frame-pointer
	;		original sp -->	+----------------------------------+
	;				| pc (12 ahead of PCS entry store) |
	;		current fp ---> +----------------------------------+
	;				| lr (on entry) pc (on exit)       |
	;				+----------------------------------+
	;				| sp ("original sp" on entry)      |
	;				+----------------------------------+
	;				| fp (on entry to function)        |
	;				+----------------------------------+
	;				|                                  |
	;				| ..argument and work registers..  |
	;				|                                  |
	;		current sp ---> +----------------------------------+
	;
	; The "current sl" is somewhere between "original sp" and "current sp"
	; but above "true sl". The "current sl" should be "sl_offset" bytes
	; above the "true sl". The value "sl_offset" should be large enough
	; to deal with the worst case function entry stacking (160bytes) plus
	; the stack overflow handler stacking requirements, plus the stack
	; required for the memory allocation routines.
	;
	; Normal PCS entry (before stack overflow check) can stack 16 standard
	; registers (64bytes) and 8 floating point registers (96bytes). This
	; gives a minimum "sl_offset" of 160bytes (excluding the stack
	; required for the code). (Actually only a maximum of 14standard
	; registers are ever stacked on entry to a function).
	;
	; NOTE: Structure returns are performed by the caller allocating a
	;	dummy space on the stack and passing in a "phantom" arg1 into
	;	the function. This means that we do not need to worry about
	;	preserving the stack under "sp" even on function return.
	;
	; THINGS TO NOTE:
	; 	We should ensure that every function that calls
	;	"__stack_overflow" or "__stack_overflow_1" does so with a
	;	valid PCS. ie. they should use "fp" to de-stack on exit (see
	;	notes above).
	;
	;	"sp_offset" bytes should always be available beneath the
	;	"sp" when IRQs are enabled (required for process SaveState).
	;	This means that code should never poke values beneath sp,
	;	sp should always be "dropped" first to cover the data.
	;

	SUB	ip,sp,ip		; extra stack required for function
	STMFD	sp!,{v1,v2,lk}		; temporary work registers
	MOV	v1,sp			; address stored work registers
	ADD	v2,ip,#stack_increment	; amount of stack required

	LDR	ip,[fp,#-4]
	STR	ip,[sp,#-4]!		; original return address	"pc"
	LDR	ip,[fp,#-8]
	STR	ip,[sp,#-4]!		; original stack-pointer	"sp"
	STR	sl,[sp,#-4]!		; original stack-limit		"sl"

	STR	sp,[fp,#-8]		; stack describing return state
	ADRL	ip,__stack_overflow_exit
	STR	ip,[fp,#-4]		; return address (exit handler)

	; This code relies on the process having a valid "dp" (module table)
	STMFD	sp!,{a1,a2,a3,a4}	; preserve over memory allocation
	MOV	a1,v2			; a1 = amount of memory we require
	LDR	a2,[dp,#:MODOFF: _Task_]
	LDR	a2,[a2,#:OFFSET: _Task_]
	ADD	a2,a2,#Task_MemPool	; a2 = &(_Task_->MemPool) ;
	BL	AllocMem		; provided by Helios Kernel
	MOVS	sl,a1			; new true "sl"
	LDMFD	sp!,{a1,a2,a3,a4}	; recover work registers
	LDMEQFD	v1,{v1,v2,lk}		; recover work registers
	BEQ	__stack_overflow_failed	; failed to allocate memory
	ADD	sp,sl,v2		; new stack-pointer
	ADD	sl,sl,#sl_offset	; new stack-limit
	LDMFD	v1,{v1,v2,pc}^		; and step back into the function

	; ---------------------------------------------------------------------
	; This code is called when the function with the extended stack
	; completes. We should free the stack memory block allocated and then
	; return to the original caller.

__stack_overflow_exit
	; in:	sp = stack-pointer in original stack frame
	;	     containing stack-limit, stack-pointer and return address
	;	sl = stack-limit of extended stack-frame
	;	ip = undefined (work register)
	;	lk = undefined (work register)
	;	fp = frame-pointer of original caller (at instance of call)
	STMFD	sp!,{a1}		; preserve a1
	SUB	a1,sl,#sl_offset	; a1 = base of the allocated memory
	BL	FreeMem			; and release the memory block
	LDMFD	sp,{a1,sl,sp,pc}^	; recover a1 and exit back to caller

	; ---------------------------------------------------------------------

__stack_overflow_failed
	STMFD	sp!,{lk}		; "lk" corrupted by function call
	; We could not claim any memory for the extended stack. Raise a
	; suitable C signal.
	LDR	ip,[dp,#:MODOFF: _Task_] ; module of MyTask (_Task_)
	LDR	a1,[ip,#:OFFSET: _Task_] ; get task pointer into first arg
	MOV	a2,#sys_nostack		; SIGSTAK (keep in step with signal.h)
	BL	CallException		; raise signal
	LDMFD	sp!,{pc}^		; return to caller

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; NCC Compiler support functions
	; mostly taken from Acorn RISCOS library

__multiply	FnHead
	; a1 = a1 * a2, uses a3
	MOV	a3,#&00000000
xxxmultiplyit
	MOVS	a2,a2,LSR #1
	ADDCS	a3,a3,a1
	ADD	a1,a1,a1
	BNE	xxxmultiplyit
	MOV	a1,a3
	MOVS	pc,lr

	; ---------------------------------------------------------------------
	; Division by Zero test

__divtest	FnHead
	CMP	a1,#&00000000
	MOVNES	pc,lr
__divtest_part2	; may also be called at this point by div/udiv
	STMFD	sp!,{lr}		; corrupted by function calls

	LDR	ip,[dp,#:MODOFF: _Task_] ; module of MyTask (_Task_)
	LDR	a1,[ip,#:OFFSET: _Task_] ; get task pointer into first arg

	MOV	a2,#err_divzero		; SIGFPE (keep in step with signal.h)
	BL	CallException		; raise the signal
	LDMFD	sp!,{pc}^		; return to caller

	; ---------------------------------------------------------------------

__divide	FnHead
	; Signed divide of a2 by a1: returns quotient in a1, remainder in a2
	; Quotient is truncated (rounded towards zero).
	; Sign of remainder = sign of dividend.
	; Destroys a3, a4 and ip
	; Negates dividend and divisor, then does an unsigned divide; signs
	; get sorted out again at the end.
	; Code mostly as for udiv, except that the justification part is
	; slightly simplified by knowledge that the dividend is in the range
	; (0..&80000000) (one register may be gained thereby).

	MOVS	ip,a1
        BEQ	__divtest_part2
        RSBMI	a1,a1,#&00000000	; absolute value of divisor
        EOR	ip,ip,a2
        ANDS	a4,a2,#&80000000
        ORR	ip,a4,ip,LSR #1
	; ip bit 31  sign of dividend (= sign of remainder)
	;    bit 30  sign of dividend EOR sign of divisor (= sign of quotient)
        RSBNE	a2,a2,#&00000000	; absolute value of dividend

        MOV	a3,a1
        MOV     a4,#&00000000
s_loop
	;CMP	a2,a3,ASL #0
	CMP	a2,a3
        BLS     s_shifted0mod8
        CMP     a2,a3,ASL #1
        BLS     s_shifted1mod8
        CMP     a2,a3,ASL #2
        BLS     s_shifted2mod8
        CMP     a2,a3,ASL #3
        BLS     s_shifted3mod8
        CMP     a2,a3,ASL #4
        BLS     s_shifted4mod8
        CMP     a2,a3,ASL #5
        BLS     s_shifted5mod8
        CMP     a2,a3,ASL #6
        BLS     s_shifted6mod8
        CMP     a2,a3,ASL #7
        MOVHI   a3,a3,ASL #8
        BHI     s_loop
s_loop2
	CMP	a2,a3,ASL #7
	ADC	a4,a4,a4
        SUBHS   a2,a2,a3,ASL #7
        CMP     a2,a3,ASL #6
s_shifted6mod8
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #6
        CMP     a2,a3,ASL #5
s_shifted5mod8
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #5
        CMP     a2,a3,ASL #4
s_shifted4mod8
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #4
        CMP     a2,a3,ASL #3
s_shifted3mod8
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #3
        CMP     a2,a3,ASL #2
s_shifted2mod8
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #2
        CMP     a2,a3,ASL #1
s_shifted1mod8
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #1
	;CMP	a2,a3,ASL #0
	CMP	a2,a3
s_shifted0mod8
        ADC     a4,a4,a4
	;SUBHS	a2,a2,a3,ASL #0
        SUBHS	a2,a2,a3
        CMP     a1,a3,LSR #1
        MOVLS	a3,a3,LSR #8
        BLS	s_loop2
        MOV	a1,a4
        TST	ip,#&40000000
        RSBNE	a1,a1,#0
        TST	ip,#&80000000
        RSBNE	a2,a2,#0
        MOVS	pc,lr

	; ---------------------------------------------------------------------

__remainder	FnHead
	; returns signed remainder of a2/a1 in a1
	STMFD	sp!,{lr}
	BL	__divide		; cheat a little!
	MOV	a1,a2
	LDMFD	sp!,{pc}^

	; ---------------------------------------------------------------------

__uremainder	FnHead
	; returns unsigned remainder of a2/a1 in a1
	STMFD	sp!,{lr}
	BL	__udivide		; cheat a little!
	MOV	a1,a2
	LDMFD	sp!,{pc}^

	; ---------------------------------------------------------------------

__udivide	FnHead
	; Signed divide of a2 by a1, returns quotient in a1, remainder in a2
	; Unsigned divide of a2 by a1: returns quotient in a1, remainder in a2
	; Destroys a3, a4 and ip

        MOVS	a3,a1
        BEQ	__divtest_part2
        MOV     a4,#&00000000
        MOV	ip,#&80000000
        CMP     a2,ip
        MOVLO   ip,a2
u_loop
	;CMP	ip,a3,ASL #0
        CMP     ip,a3
        BLS     u_shifted0mod8
        CMP     ip,a3,ASL #1
        BLS     u_shifted1mod8
        CMP     ip,a3,ASL #2
        BLS     u_shifted2mod8
        CMP     ip,a3,ASL #3
        BLS     u_shifted3mod8
        CMP     ip,a3,ASL #4
        BLS     u_shifted4mod8
        CMP     ip,a3,ASL #5
        BLS     u_shifted5mod8
        CMP     ip,a3,ASL #6
        BLS     u_shifted6mod8
        CMP     ip,a3,ASL #7
        MOVHI   a3,a3,ASL #8
        BHI     u_loop
u_loop2
u_shifted7mod8
        CMP     a2,a3,ASL #7
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #7
u_shifted6mod8
        CMP     a2,a3,ASL #6
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #6
u_shifted5mod8
        CMP     a2,a3,ASL #5
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #5
u_shifted4mod8
        CMP     a2,a3,ASL #4
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #4
u_shifted3mod8
        CMP     a2,a3,ASL #3
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #3
u_shifted2mod8
        CMP     a2,a3,ASL #2
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #2
u_shifted1mod8
        CMP     a2,a3,ASL #1
        ADC     a4,a4,a4
        SUBHS   a2,a2,a3,ASL #1
u_shifted0mod8
	;CMP	a2,a3,ASL #0
        CMP     a2,a3
        ADC     a4,a4,a4
	;SUBHS	a2,a2,a3,ASL #0
        SUBHS	a2,a2,a3
        CMP     a1,a3,LSR #1
        MOVLS	a3,a3,LSR #8
        BLS     u_loop2
        MOV     a1,a4
        MOVS    pc,lr

	; ---------------------------------------------------------------------

	LTORG

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	END	; EOF asm.s
