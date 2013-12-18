;
;									;
;		      H E L I O S   R U N T I M E			;
;		      
;									;
;	      Copyright (c) 1991, 1992 Perihelion Software Ltd.		;
;			 All Rights Reserved.				;
;									;
; cstart.a								;
;									;
;	Run time code necessary to support stand alone			;
;       Helios C programs						;
;									;
;	Author:	 NOJC October 91					;
;									;
;
; RCS Id: Header: /users/nickc/ncc/cc350/ccc40/src/RCS/cstart.a,v 1.20 1992/03/03 09:22:48 nickc Exp nickc 
;
;
; To Do
;
; 	set up argument vector, and environment vector
; 	free stack chunks on return from _main
; 	provide alloca()
; 	put code in FAST RAM ?
;
; get necessary header files
; define this if cstart is to contain code for stand alone support
; the STACK_GUARD constant must agree with the one defined in c40/gen.c
		; number of bytes in 1K
		; minimum number of words to be left free on stack
		; number of words in a stack chunk header
	; size of an individual stack chunk
; the following represent word offsets from the start of a stack chunk
; a few more constants
; word offsets of fields in the Module Structure
; word offsets of fields in the Program structure
; name a few registers after variables used in task.c
	; maximum size of function tables
	; maximum size of data slots
	; maximum module number
	; size of memory to be allocated
	; number of modules in program
	; temporary pointer
	; temporary register
	; copy of initial value of pointer
	; function table pointer
	; data slots pointer
	; (word) AR1 into code space
	; AR3 based on R5
	; init chain AR1
	; upon entry NO registers are defined
	LDHI	0x0010, AR5				; point to peripheral memory map
	OR	0x0072, AR5				; point to output port of link 3
	STIK	1, *AR5					; put word onto link
	LAJ	4					; get PC into R11
	LDI	.program_start - 3, AR1		; get offset of the end of this code
	ADDI	R11,    AR1				; and place in an address register
	OR	0x9800, ST				; enable cache, enable all condition flags for all regs
	; 						  fake link-and-jump takes place now
	LDI	0,  R1				; initialise code size count
	LDI	0,  R2				; initialise data size count
	LDI	0,  R3				; initialise maximum module Id
	LDI	-1, R5				; initialise count of number of modules found
	LDI	AR1, R8			; and save a copy of it							
	; 		drop straight into the first pass as there will always be at least one module
	; first pass through the modules - calculate size of data and code segments
	;
	; AR1	(AR1)	word AR1 to first module of program
	; R1	(R1)	 0
	; R2	(R2)	 0
	; R3	(R3)	 0
	; R5	(R5)	-1
	; R8	(R8)	copy of AR1 (AR1)
	;
first_pass:
	;
	LDI	*+AR1( 10 ), R7		; get modules Id
	CMPI	R3, R7				; is it greater than current largest module Id ?
	LDIgt	R7,    R3				; then update maximum module Id
	ADDI	*+AR1( 14 ), R1	; accumulate code size
	ADDI	*+AR1( 12 ),  R2	; accumulate data size
	ADDI	1, R5				; increment count of number of modules found
	LDI	*+AR1( 1 ), R7		; get size of this module
	LSH	-2,  R7				; convert to words
	ADDI	R7, AR1				; and advance AR1 to next module
	LDI	*+AR1( 0 ), R7		; get the module type
	BneAT 	first_pass				; if the type is not 0 repeat loop (delayed)
	;
	; first pass ended
	;
	; AR1	(AR1)	word AR1 to end of program
	; R1	(R1)	total number of bytes required for all the function tables combined
	; R2	(R2)	total number of bytes required for all the data areas      combined
	; R3	(R3)	largest module number
	; R5	(R5)	number of modules in the program
	; R8	(R8)	word AR1 to first module of program
	;
	ADDI3	 4, AR1, SP				; stack system stack just beyond end of code
	ADDI 	 1, R3				; increment maximum module number
	ADDI	 3, R2				; convert data slot size to ...
						; first pass branch takes place now
	LSH	-2, R2				; ... number of words
	ADDI	 3, R1				; convert function table size to ...
	LSH	-2, R1				; ... number of words
	ADDI3	R2, R1, R4		; calculate number of words needed by these tables
	LSH	 1, R3 				; 2 words needed for every module entry
	;
	; now malloc blocksize words
	;
	LAJ	_sa_malloc				; get stand alone malloc routine (delayed)
	ADDI	R3, R4			; add size of module table to R4
	LDI	R4, R0				; place number of words required in argument registter
	SUBI3	1, R4, RC			; put R4 - 1 into repeat count
	;
	; 		link-and-jump takes place now
	;
	;                SP	start of system stack
	; AR1	(AR1)	word AR1 to end of program
	; R0		 R0	AR1 to allocated block of RAM
	; R1	(R1)	total number of words required for all the function tables combined
	; R2	(R2)	total number of words required for all the data areas      combined
	; R3	(R3)	double the (largest module number plus one)
	; R4	(R4)	size of alloacted block of RAM in words
	; R5	(R5)	number of modules in the program
	; R8	(R8)	word AR1 to first module of program
	;
	; initialise entire block to 0
	LDI	R0, IR0				; save result into base of free RAM register
	LDI	R0, AR1				; get address into an address register
	RPTS	RC					; repeat next instruction R4 times
	STIK	0, *AR1++(1)			; place 0 in word
	LDI	IR0, AR4				; build module table at start of data RAM
	STI	AR4,  *AR4				; first word of module table points to itself
	LDI	R8, AR1			; point at first module
	ADDI3	R3, AR4, R9			; start function pointers after module table
	ADDI3	R9, R1, R10		; start data slots after function tables
	SUBI	IR0, R10			; calculate offset from memory base
	LSH	2, R10				; and convert to byte offset
	LDI	R5, AR3			; load AR3
	LDI	2, R0					; fist init sequence has argument set to 2
	;
	;                SP	start of system stack
	; IR0	 IR0	start of addressible data RAM
	; AR1	(AR1)	word AR1 to first module in program
	; AR3	(AR3)	number of modules in program
	; AR4		(AR4)	start of module table
	; R0		 R0	2
	; R1	(R1)	total number of words required for all the function tables combined
	; R2	(R2)	total number of words required for all the data areas      combined
	; R3	(R3)	double the (largest module number plus one)
	; R4	(R4)	size of alloacted block of RAM in words
	; R5	(R5)	number of modules in the program
	; R8	(R8)	word AR1 to first module of program
	; R9	(R9)	word AR1 to first word after end of module table
	; R10	(R10)	byte offset of  first word after end of function tables
	;
	; fall through into body of second pass
	;
second_pass:
	LDI	*+AR1( 10 ), IR1		; get module ID
	LSH	1, IR1					; double
	STI	R10, *+AR4( IR1 )			; save AR1 to data slots in module table
	ADDI	1, IR1					; point to next word in module table
	STI	R9, *+AR4( IR1 )			; save AR1 to function table in module table
	LDI	*+AR1( 14 ), R7	; get number of bytes used in function table
	LSH	-2, R7					; convert to words
	ADDI	R7, R9				; and add to function table AR1
	BRD	init_sequence_start2			; branch to init sequence start (delayed)
	LDI	*+AR1( 12 ), R7	; get number of bytes of data used by module
	ADDI	R7, R10				; and add to data slots AR1
	ADDI3	13, AR1, AR6		; get address of head of init chain
	; 		branch takes place now
init_sequence2:
	NOP	; XXX WHY WHY WHY ???
	LAJu	AR2 					; call init routine (delayed)
	NOP						; padding
	NOP						; padding
	NOP						; padding
	;		link-and-jump takes place now
	;
	; NB/
	; the init code is given:	R0,  R11, AR4, IR0
	; it is expected to use:	AR0, AR2, AR5, RS, ST
	; it must NOT alter:		R0,  R5,  R8,  R9, R10, AR1, AR3, AR4, AR6
	; it may alter (if necessary):	R1,  R2,  R3,  R4, R6,  R7,  IR1, RC,  RE
	;
init_sequence_start2:
	LDI	*AR6, R6			; get AR1 to next link of modules init chain
	BneD	init_sequence2				; if it is 0 we have finished (delayed)
	LSH	-2, R6				; convert offset to words
	ADDI	R6, AR6			; add self relative AR1 to its address
	ADDI3	1, AR6, AR2 			; next word is start of init code
						; branch (might) take place now
	; end of init sequence 2
	DBuD	AR3, second_pass			; decrement AR3 and repeat loop if not zero (delayed)
	LDI	*+AR1( 1 ), R7		; get size of this module
	LSH	-2, R7					; convert to words
	ADDI	R7, AR1				; and advance AR1 to next module
						; branch (might) take place now
	; end of second pass
	LDI	R8, AR1			; point to program stucture again
	BRD	init_sequence_start0			; branch to init sequence start (delayed)
	LDI	0, R0					; second init sequence has argument set to 0
	LDI	R5, AR3			; load AR3
	ADDI3	13, AR1, AR6		; get address of head of init chain
						; branch takes place now
third_pass:
	BRD	init_sequence_start0			; branch to init sequence start (delayed)
	ADDI3	13, AR1, AR6		; get address of head of init chain
	NOP						; padding
	NOP						; padding
						; branch takes place now
init_sequence0:
	NOP	; XXX WHY WHY WHY ???
	LAJu	AR2 					; call init routine (delayed)
	NOP						; padding
	NOP						; padding
	NOP						; padding
						; link-and-jump (might) take place now
init_sequence_start0:
	LDI	*AR6, R6			; get AR1 to next link of modules init chain
	BneD	init_sequence0				; if it is 0 we have finished (delayed)
	LSH	-2, R6				; convert offset to words
	ADDI	R6, AR6			; add self relative AR1 to its address
	ADDI3	1, AR6, AR2 			; next word is start of init code
						; branch (might) take place now
	; end of init sequence 0
	DBuD	AR3, third_pass			; decrement AR3 and repeat loop if not zero (delayed)
	LDI	*+AR1( 1 ), R7		; get size of this module
	LSH	-2, R7					; convert to words
	ADDI	R7, AR1				; and advance AR1 to next module
						; branch (might) take place now
	; end of third pass
	LDI	R8, AR1			; point to program stucture again
	LDI	1, R0					; third init sequence has argument set to 1
	LDI	R5, AR3			; initialise AR3
	ADDI3	17, R8, AR1	; get AR1 to Main field of program structure
	LDI	*AR1, R7				; get self relative AR1 to "Main"
	LSH	-2, R7					; convert to a word offset
	ADDI	AR1, R7				; add the address of the program structure
	LAJu	R7					; and call the first function (delayed)
	LDI	*-AR1( 17 - 15 ), R0
							; size of stack is passed in R0
	SUBI3	.program_start - _sa_malloc + 17 , AR1, R1
							; place address of sa_alloc function in second argument
	NOP						; padding
						; link and jump takes place now
	;
	; program has exited
	; inform outside world somehow
	; and then hang
	;
	LDHI	0x0010, AR5				; point to peripheral memory map
	OR	0x0072, AR5				; point to output port of link 3
	STI	R0,  *AR5				; put return value onto link
	STIK	-2,    *AR5				; put word onto link
forever:
	IDLE						; do nothing
	B	forever					; carry on doing nothing
.sa_abort:
	; something has gone wrong
	; inform the outside world somehow
	; and then hang
	LDHI	0x0010, AR5				; point to peripheral memory map
	OR	0x0072, AR5				; point to output port of link 3
	STIK	-1,    *AR5				; put word onto link
	B	forever					; do nothing
.sa_data:
	word	0x80080000	; next free word of RAM
				; 0x8008000 is 2 megabytes into global bus
	word	0x00010000	; number of words of RAM left
_sa_malloc:
;
;	void *	_sa_malloc( unsigned long num_words );
;
; Upon Entry:
; 	R0 	number of words of RAM required
; 	R11	return address of caller
;
; Upon Exit
;	R0	word AR1 to block of memory or 0
;
; Corrupts
;	RS, RE, AR5, ST
;
; Purpose
; 	Stand Alone memory Allocator
; 	NO user stack space can be used by this function.
;
	LDI	R11, RS				; save return address
	LAJ	4					; get PC into R11
	NOP						; XXX because of silicon bug
	ADDI3	.sa_data - 2, R11, AR5		; get address of private data
	LDI	*+AR5( 1 ), RE	; get number of words available
						; fake link-and-jump takes place now
	CMPI	R0, RE				; check against number required
	BltD	RS					; if not enough are available return (delayed)
	LDIlt	0,    R0				; and put 0 in result register
	SUBI	R0, RE				; otherwise subtract number of words required
	NOP						; padding
						; branch (might) take place now
	STI	RE, *+AR5( 1 )	; and save result
	LDI	*+AR5( 0 ), RE	; get AR1 to free RAM
	BuD	RS					; return (delayed)
	ADDI3	R0, RE, RS			; increment free RAM AR1 by number of words used
	STI	RS, *+AR5( 0 )	; save in private data area
	LDI	RE, R0				; place original free RAM AR1 in result register
						; return takes place now
.program_start:		; NB/ this MUST be the last label in this section

 ; end of 1 code
; build a fake program structure
        align
        module  -1
.ModStart:
        word    0x60f160f1
		word modsize
	        blkb    31, "cstart"
		byte 0
	word	modnum
	word	1
		word datasymb(.MaxData)
        init
	word 2 * 1024		; StackSize
	word 0			; (Fake) HeapSize
	word (.EntryPoint * 4)	; Self relative AR1 to start of executable code (byte offset)
_data:	; data used by the stack allocator
	word 0			; AR1 to next free stack chunk (word just below header)
	word 0			; AR1 to current stack chunk (word just below header)
	word 2 * 1024		; minimum size of a stack chunk
	word 0			; word AR1 to word malloc function
	; constant offsets from _data
	; start of code
.EntryPoint:
	; execution entry point
	; upon entry the following registers are set up ...
	;
	; R0 (R0)	- number of words of stack required
	; R1 (R1)	- address of word memory allocator
	;
	; IR0 (IR0)	- start of addressible RAM 
	; R11 (R11)	- return address
	; AR4 (AR4)	- start of module table
	;
	; functions performed by this code are:
	;  create initial stack chunk
	;  build argument vector
	;  call main()
	LDHI	0x0010, AR5				; point to peripheral memory map
	OR	0x0072, AR5				; point to output port of link 3
	STIK	2,     *AR5				; put word onto link
	;
	; initialise stack chunk
	;
	LDI	R11, R8				; save return address
	LAJ	4					; get PC into R11
	NOP						; XXX because of silicon bug
	SUBI3	(- _data) + 2, R11, AR0		; get AR1 to private data area
	STI	R0,  *+AR0( 2 )	; save size of first stack chunk
						; fake link-and-jump takes place now
	LDI	R0,    R2				; save the size of stack
	LAJu	R1					; call memory allocator to get stack (delayed)
	STI	R1,  *+AR0( 3 )	; save address of malloc routine
	ADDI	6, R0			; plus size of chunk header in memory allocated
	NOP						; padding
						; link-and-jump to memory allocator takes place now
	ADDI3	R0,    R2, AR6 			; point to top of stack
	STI	AR6, *+AR0( 1 )	; and save in private data area as well
	ADDI3	64, R0, IR1		; stack end is 64 words above end of stack
	STIK	0,     *+AR0( 0 )	; no next free chunk
	STI	R8,  *+AR6( 1 )	; save out return address
	STIK	0,     *+AR6( 2 )	; 0 stack end AR1
	STIK	0,     *+AR6( 3 )	; 0 stack AR1
	STIK	0,     *+AR6( 4 )		; 0 previous chunk
	STIK	0,     *+AR6( 5 )		; 0 next chunk
	STI	R2,  *+AR6( 6 )		; size of this chunk
	; initialise clock
	LAJ	.ClockInit
	NOP
	NOP
	NOP
	; build argument vector
	LDI	0, R0					; no arguments
	LDI	0, R1					; no argument vector
	; call main
	LDHI	0x0010, AR5				; point to peripheral memory map
	OR	0x0072, AR5				; point to output port of link 3
	STIK	3,     *AR5				; put word onto link
	LAJ	4					; get PC into R11
	NOP						; XXX because of silicon bug
	ADDI	finished - 2, R11			; add in offset to our finish code
patchinstr
(
	PATCHC40DATAMODULE1, shift( -2, datamodule( _main ) ), 
	LDI	AR4,    AR5				; get address of module table
)

