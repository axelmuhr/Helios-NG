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
patchinstr
(
	PATCHC40DATAMODULE2, shift( -2, codesymb( _main ) ),
	ADDI	1,       AR5				; add in offset of module containing _main
)
patchinstr
(
	PATCHC40DATAMODULE3, modnum, 
	LDI	*AR5, AR5	 			; get the AR1 to the function table
)
patchinstr
(
	PATCHC40DATAMODULE4, modnum,
	ADDI	0,       AR5				; and add to AR1
)
patchinstr
(
	PATCHC40DATAMODULE5, modnum,
	LDI	*AR5, AR5				; get the AR1 to the function
)
	Bu	AR5					; jump to main 
finished:
	; program has exited
	LDHI	0x0010, AR5				; point to peripheral memory map
	OR	0x0072, AR5				; point to output port of link 3
	STIK	4,     *AR5				; put word onto link
	; free up stack chunks
	LDI	*+AR6(1), R11	; get return address off stack
	Bu	R11					; return to our caller
mem_abort:
	; something has gone wrong will stack memory allocator
	; terminate program
	; issue error message to console
	; shut down
	LDHI	0x0010, AR5
	OR	0x0072, AR5	
	STIK	-3,    *AR5		
	B	forever
__stack_free:
;
;	void	__stack_free( void );
;
; Upon Entry:
; 	AR6	points to end of stack chunk header
; 	AR7	has already been reset to the callers stack frame
; 	IR1	end of current (to be freed) stack chunk
;	R11	return address
;
; Upon Exit
;	no return value
;
; Corrupts
;	R9, AR5, AR6, ST
;
; Purpose
; 	This routine is called whenever we have finished using a stack chunk
; 	This routine DOES NOT conform to the normal HELIOS C calling conventions
;	
	LAJ	4						; get PC into R11
	NOP							; XXX becuase of silicon bug
	ADDI3	_data - 2, R11, AR5				; add offset to private data area
	STI	AR6, *+AR5(0)		; save this chunk as next available chunk
							; fake link-and-jump takes place now
	LDI	*+AR6(4), R9			; get hold of previous chunk
	BeqD	mem_abort					; if there is none then abort (delayed)
	STI	R9, *+AR5(1)			; save AR1 to prev chunk as current chunk
	LDI	*+AR6(1   ), R11		; get hold of true return address
	LDI	*+AR6(2), IR1		; get hold of stack end AR1 of prev chunk
							; branch (might) take place now
	LDI	*+AR6(3    ), AR6		; and stack AR1 in previous chunk
	Bu	R11						; return
.__stack_overflow:
;
;	void	__stack_overflow( void );
;
; Upon Entry:
; 	RS	number of words on pushed onto current stack as part of function entry process
; 	RE	minimum number of words required
; 	RC	return address OF the function that called us
; 	R11	return address TO the function that called us
;
; Upon Exit
;	no return value
;
; Corrupts
;	R8, R9, R10, AR0, AR1, AR2, AR5, AR6, IR1, ST
;
; Purpose
; 	This routine is called when we need a new stack chunk.
; 	This routine DOES NOT conform to the normal Helios C calling conventions.
; 	NO user stack space can be used by this function.
;
	LDI	R11,      AR3				; save our return address
	LAJ	4						; get PC into R11
	LDI	_data - 3, AR0				; load offset to private data area
	ADDI	R11,      AR0				; and place in address register
	LDI 	*+AR0( 0 ), AR1		; get AR1 to next free stack chunk
							; fake link-and-jump takes place now
	BnzD 	chunk_available					; if there is one skip this bit (delayed)
	LDI	R0,      AR2				; save argument register
	LDI	*+AR0( 3 ), R8		; get address of malloc function
	LDI	RS,    R10					; save RS into R10
							; branch (might) take place now
	LAJu	R8						; call the malloc routine (delayed)
	ADDI3	*+AR0( 2 ), RE, R0 	; add minimum size of chunk to size required
	LDI	R0,      R9					; save this value
	ADDI	6, R0				; add in size of chunk header
							; link-and-jump to malloc takes place now
	LDI	R0,      AR1				; place AR1 to new chunk in AR1
	Beq	mem_abort					; if the AR1 is 0 then abort
	LDI	AR2,   R0					; restore argument register
	ADDI	R9,      AR1				; point to top of stack chunk
	STI	R9,    *+AR1( 6 )		; save size of chunk
	BRD	got_chunk					; carry on with chunk initialisation (delayed)
	LDI 	*+AR0( 1 ), AR2		; get AR1 to current stack chunk
	STIK	0,       *+AR1( 5 )		; initialise 5 AR1 to 0
	STI	AR2, *+AR1( 4 )		; initialise 4 AR1 to current chunk
							; branch takes place now
chunk_available:
	;
	; 0 is non-null
	;
	; R8	  - address of malloc routine
	; R9    -
	; R10    - num words placed on current stack
	; RS  -
	; RE  - num words required
	; RC  - return address of function that called us
	; AR0 - address of private data area
	; AR1 - address of next free chunk
	; AR2 - copy of R0
	; AR3 - return address to the function that called us
	; AR5  -
	;
	LDI	*+AR1( 6 ), R9			; get size of next free chunk
	CMPI	R9,      RE				; compare against number of words required
	Ble	use_free_chunk					; if it is sufficient then skip to next section
	LAJu	R8						; call malloc routine
	ADDI3	6, RE, R0			; place number of words needed in argument reg
	LDI	AR1,   AR2				; save address of next free chunk 
	LDI	RE,    R9					; place number of words in chunk in R9
							; link-and-jump takes place now
	LDI	R0,      AR1				; place address of block in address register
	Beq	mem_abort					; if the address is 0 then abort (delayed)
	ADDI	R9,      AR1				; point to top of stack chunk
	LDI	AR2,   R0					; restore argument register
	STI	R9,    *+AR1( 6 ) 		; save size of chunk into chunk header
	STI 	AR2, *+AR1( 5 )		; place address of next free chunk in new chunk
	STI	AR1, *+AR2( 4 )		; place address of new chunk in next free chunk
	BRD	got_chunk				; and carry on with chunk initialisation (delayed)
	LDI 	*+AR0( 1 ), AR2		; get AR1 to current stack chunk
	STI	AR1, *+AR2( 5 )		; place address of new chunk in current chunk
	STI	AR2, *+AR1( 4 )		; place address of current chunk in new chunk
							; branch takes place now
use_free_chunk:
	; we can use the next free chunk
	;
	; R8    -
	; R9	  - size of next free chunk
	; R10    - num words on placed on current stack
	; RS  -
	; RE  -
	; RC  - return address of function that called us
	; AR0 - address of private data area
	; AR1 - address of next free chunk
	; AR2 -
	; AR3 - return address to the function that called us
	; AR5  -
	;
	LDI	*+AR1( 5 ), RE			; get successor to next free chunk
	STI	RE, *+AR0( 0 )		; and save as the next free stack chunk
							; drop through into got_chunk code
got_chunk:
	; we now have a chunk
	;
	; R8    -
	; R9    - size of new chunk
	; R10    - num words placed on current stack
	; RS  -
	; RE  -
	; RC  - return address of function that called us
	; AR0 - address of private data area
	; AR1 - address of new chunk
	; AR2 -
	; AR3 - return address to the function that called us
	; AR5  -
	;
	STI	AR1, *+AR0( 1 )		; save address of new chunk
	ADDI	R10,      AR6				; add number of words on stack to current SP
	STI	AR6,   *+AR1( 3 )		; and save in 3 field 
	LAJ     4						; get PC into R11
	STI	IR1,    *+AR1( 2 )	; save stack end AR1 in new chunk
	ADDI	__stack_free - 2, R11				; add in offset of our free stack chunk routine
	STI	RC,  *+AR1( 1 )	; save return address into stack chunk
							; link and jump takes place now (to next instruction)
	BuD	AR3						; and return (delayed)
	SUBI3 	R9,      AR1, IR1			; point to end of stack chunk
	ADDI	64, IR1				; set up stack end AR1 
	SUBI3	R10,      AR1, AR6			; put some dummy words on the stack
							; return takes place now
.setjmp:
;
;      int setjmp( jmp_buf env )
;
; Upon Entry:
;	R0	byte offset of the jump buffer used to hold register states
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	R0	0 	if this was the original call to setjmp()
;		non-0	if this is the result of an invocation of longjmp()
;
; Corrupts
;	AR0, AR1, AR5, ST
;
; Purpose
;	This function preserves the state of the stack and registers
;	in the jump buffer, so that a later call to longjmp() can
;	effectivly cause execution to resume after this function.
;
;	Upon return from a function call the following registers are
;	allowed to have been corrupted:
;
;	The Argument registers    (R0, R1, R2, R3)
;	The Temporary registers   (R8, R9, R10)
;	The Address registers     (AR0, AR1, AR2, AR3)
;	The Status register       (ST)
;	The Universal temporaries (RS, RE, RC, AR5)
;
;	The following registers, however, must be preserved:
;
;	The Variable registers (R4,  R5,  R6,  R7, BK, DP)
;	The Return address     (R11)
;	The Frame Pointer      (AR7)
;	The User Stack Pointer (AR6)
;	The Stack End Pointer  (IR1)
;
;	The following registers should not have been changed (ever):
;
;	Module Table Pointer      (AR4)
;	The Base Address register (IR0)
;	The System Stack Pointer  (SP)
;
	LSH	-2,     R0, AR5			; convert buffer "AR1" to word offset
	Beq	R11					; urg a 0 AR1 !!!
	LDI	0,      R0			; return value
	ADDI	IR0, AR5				; AR5 now has word AR1 to jump buffer
	NOP						; padding
						; return (might) happen now
	LDI	R11,   AR0				; save link register
	LAJ	4					; get hold of PC
	LDI	_data - 3, AR1			; offset to private data area
	ADDI	R11,      AR1			; add in PC
	LDI	AR0,   R11				; restore link register
	LDI	*+AR1( 1 ), AR0	; get AR1 to current stack chunk
	STI	AR0, *AR5++(1)			; save AR1 to current chunk
	STF	R4, *AR5++(1)			; save register R4 (floating point)
	STI	R4,  *AR5++(1)			; save register R4
	STI	R5,  *AR5++(1)			; save register R5
	STF	R6, *AR5++(1)			; save register R6 (floating point)
	STI	R6,  *AR5++(1)			; save register R6
	STI	R7,  *AR5++(1)			; save register R7
	STI	BK,  *AR5++(1)			; save register BK
	STI	DP,  *AR5++(1)			; save register DP
	STI	R11,  *AR5++(1)			; save register R11
	BuD	R11					; return (delayed)
	STI	AR6, *AR5++(1)			; save register AR6
	STI	AR7,  *AR5++(1)			; save register AR7
	STI	IR1, *AR5++(1)			; save register IR1
						; return takes place now	
.longjmp:
;
;      void longjmp( jmp_buf env, int val )
;
; Upon Entry:
;	R0	byte offset of the jump buffer used to hold register states
;	R1	return value
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	Does not exit normally.
;
; Corrupts
;	R0, R1, R4, R5, R6, R7, BK, DP, AR6, AR7, IR1, ST
;	AR0, AR1
;
; Purpose
;	This function causes execution to resume as if a previous call
;	to setjmp() had just returned val.  (val must be non zero,
;	if it is zero, then the setjmp() will appear to return 1).
;
	LSH	-2,     R0, AR5			; convert buffer "AR1" to word offset
	Beq	R11					; urg a 0 AR1 !!!
	LDIeq	0,      R1				; return value
	ADDI	IR0, AR5				; AR5 now has word AR1 to jump buffer
	LDI	R1,   R0			; put return value into return register
						; return (might) happen now
	LDIeq	1, R0				; convert return value to 1 if it is 0
	LDI	R11,   AR0				; save link register
	LAJ	4					; get hold of PC
	LDI	_data - 3, AR1			; offset to private data area
	ADDI	R11,      AR1			; add in PC
	LDI	AR0,   R11				; restore link register
	LDI	*AR5++(1), R1			; get AR1 to original stack chunk
	LDI	*+AR1( 1 ), AR0	; get AR1 to current stack chunk
	CMPI	R1, AR0				; see if they are the same
	Beq	lj_ok_to_restore			; and skip next section if so
lj_previous:
	LDI	*+AR0( 4 ), AR0	; get previous chunk
	CMPI	R1, AR0				; see if it matches original chunk
	Bne	lj_previous				; and loop until it does
	LDI	*+AR0( 5 ), R1		; get next chunk
	STI	R1, *+AR1( 0 )	; and save as start of free chunk list
lj_ok_to_restore:
	LDF	*AR5++(1), R4			; restore register R4 (floating point)
	LDI	*AR5++(1), R4			; restore register R4
	LDI	*AR5++(1), R5			; restore register R5
	LDF	*AR5++(1), R6			; restore register R6 (floating point)
	LDI	*AR5++(1), R6			; restore register R6
	LDI	*AR5++(1), R7			; restore register R7
	LDI	*AR5++(1), BK			; restore register BK
	LDI	*AR5++(1), DP			; restore register DP
	LDI	*AR5++(1), R11			; restore register R11
	LDI	*AR5++(1), AR6			; restore register AR6
	LDI	*AR5++(1), AR7			; restore register AR7
	LDI	*AR5++(1), IR1			; restore register IR1
	Bu	R11					; return
._stacksize:
;
;      int _stacksize( void )
;
; Upon Entry:
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	R0	number of words left on the stack
;
; Corrupts
;	AR0, AR1, AR5, ST
;
; Purpose
;	This function returns the number of words
;	left in the current stack chunk
;
	SUBI3	IR1, AR6, R0		; calculate number of words
	Bu	R11					; return
.Malloc:
;
;      void * Malloc( unsigned long num_bytes )
;
; Upon Entry:
;	R0	number of bytes of memory requested
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	R0	byte offset from IR0 of the allocated block of memory
;		returns a byte offset of 0 upon failure
; Corrupts
;	R8, R9, AR5, ST
;
; Purpose
;	This function allocates a block of memory.  The allocated block
;	will contain at least the number of bytes requested, (it may
;	contain more).  The block will be contiguous in memory, and will
;	start on a word boundary.  The contents of the block will NOT
;	have been initialised to any particular value
;
	LDI	R11, R9				; save return address
	LAJ	4					; get PC into R11
	LDI	_data - 3, AR5			; point to private data area
	ADDI	R11,      AR5			; place AR1 in address register
	LDI	*+AR5( 3 ), R8	; get address of memory allocator
						; fake link-and-jump takes place now
	LAJu	R8					; call memory allocator func (delayed)
	ADDI	 3, R0				; round up number of bytes required
	LSH	-2, R0				; and convert to words
	NOP						; padding
						; branch to allocator takes place now
	BuD	R9					; return (delayed)
	SUBI	IR0, R0				; subtract word address from base register
	LDIlt	0,      R0				; if the result is negative put 0 in R0
	LSH	2,      R0				; convert to bytes
						; return takes place now
.Free:
;
;      void Free( void * AR1 )
;
; Upon Entry:
;	R0	byte offset of the block of memory to be freed
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	R0	unchanged
;
; Corrupts
;	nothing
;
; Purpose
;	Releases a block of memory previously allocated by a
;	call to Malloc()
;
	Bu	R11					; do not bother to do anything !
.LinkWriteWord:
;
;      void LinkWriteWord( unsigned long value );
;
; Upon Entry:
;	R0	value to be written
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	R0	unchanged
;
; Corrupts
;	AR5, ST
;
; Purpose
;	Writes a word to one of the C40s links - useful for 1 purposes
;
	BuD	R11					; return (delayed)
	LDHI	0x0010, AR5				; point to peripheral memory map
	OR	0x0072, AR5				; point to output port of link 3
	STI	R0,  *AR5				; put word onto link
						; return takes place now
.LinkWriteWords:
;
;      void LinkWriteWords( unsigned long * buffer, unsigned long num_words_to_send );
;
; Upon Entry:
;	R0	byte offset of the (word aligned) block of memory to be sent
;	R1	number of WORDS in the block
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	R0	unchanged
;
; Corrupts
;	R1, AR0, AR5, RC, RS, RE, ST
;
; Purpose
;	Writes a block of words to one of the C40s links -
;	useful for 1 purposes
;	The behaviour is undefined if the link ever fills up
;
	LSH3	-2,     R0, AR0			; convert to a word offset
	BeqD	R11					; if address is 0 then return (delayed)
	LDHI	0x0010, AR5				; point to peripheral memory map
	OR	0x0072, AR5				; point to output port of link 3
	SUBI3	1,      R1, RC			; decrement number of words by two and place in count
						; return (might) take place now
	Blt	R11					; negative => return 
	BeqD	lww_finished				; 0 => only one word
	ADDI	IR0, AR0				; and add in base of RAM to source word offset
	LDI	*AR0++( 1 ), R1			; load first word
						; branch (might) take place now
	SUBI	1,      RC				; remember that one word has laready been copied
	RPTS	RC					; and use as repeat AR3 for the next instruction
	LDI	*AR0++( 1 ), R1			; get word from memory
	|| STI	R1,  *AR5				; and put into link
lww_finished:
	STI	R1,  *AR5				; put last word into link
	Bu	R11					; return
.LinkWriteBytes:
;
;	void LinkWriteBytes( unsigned char * buffer, unsigned long num_bytes_to_write );
;
; Upon Entry:
;	R0	byte offset of the block of memory to be sent
;	R1	number of BYTES in the block
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	R0	corrupted
;
; Corrupts
;	R0, R1, R8, R9, R10, AR0, AR5, RC, RS, RE, ST
;
; Purpose
;	Writes a block of words to one of the C40s links -
;	useful for 1 purposes
;	The behaviour is undefined if the link ever fills up.
;	The protocol specifies that each byte is sent as a word with 0
;	in top three bytes.
;
	LSH3	-2,       R0, AR0			; convert to a word offset
	BeqD	R11					; if the address is 0 then return (delayed)
	LDHI	0x0010,   AR5			; point to peripheral memory map
	OR	0x0072,   AR5			; point to output port of link 3
	ADDI	IR0,   AR0			; and add in base of RAM
						; return (might) take place now
	LSH3	-2,       R1, RC			; put whole number of words in RC
	BeqD	remainder				; if there are none then skip (delayed)
	SUBI	1,        RC				; count in RC is one less than number of itterations
	STIK	0,      *-AR5(2)			; enable reading and writing
	NOP						; padding
						; branch (might) take place now
	RPTB	end_of_block				; repeat the foloowing section of code 
	LDI	*AR0++( 1 ), R8			; load word
					; the following code arrangement is because of a bug in simulator!
	LBU0	R8,     R9				; put first byte into reg
	LBU1	R8,     R10				; get second byte
	LBU2	R8,     R0				; get third byte
	LBU3	R8,     R1				; get final byte
	STI	R9,    *AR5			; send first byte down link
	STI	R10,    *AR5			; send second byte
	STI	R0,    *AR5			; send thrid byte
end_of_block:
	STI	R1, *AR5				; send fourth byte
remainder:
	AND	3,        R1				; get remaining number of bytes to send
	BeqD	R11					; skip if everything has been sent (delayed)
	LDIne	*AR0, R8				; get final word
	LBU0	R8,     R9				; get bottom byte
	CMPI	2,        R1				; check number of bytes to send
						; branch (might) take place now
	Blt	almost_finished				; if its is only one then skip to end
	BeqD	R11					; if we only have 2 bytes extra then return (delayed)
	LBU1	R8,     R10				; get second byte
	STI	R9,    *AR5			; send
	STI	R10,    *AR5			; send
						; branch (might) take place now
	LBU2	R8,     R9				; get third byte
almost_finished:
	STI	R9,    *AR5			; send
	Bu	R11					; return
.LinkReadWord:
;
;	unsigned long LinkReadWord( void );
;
; Upon Entry:
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	R0	word read
;
; Corrupts
;	R0, AR5, ST
;
; Purpose
;	Reads one word from one of the C40s links - useful for 1 purposes
;
	BuD	R11					; return (delayed)
	LDHI	0x0010,  AR5				; point to peripheral memory map
	OR	0x0071,  AR5				; point to input port of link 3
	LDI	*AR5, R0				; get word from link
						; return takes place now
.LinkReadWords:
;
;	void LinkReadWords( unsigned long * buffer, unsigned long num_words_to_receive );
;
; Upon Entry:
; 	R0 	contains byte offset of the block of memory where the data is to be stored
; 	R1 	contains the number of WORDS to receive
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	no return value	
;
; Corrupts
;	R1, AR0, AR5, ST, RC, RS, RE
;
; Purpose
;	Reads words from one of the C40s links into a buffer - useful for 1 purposes
;	The inputs are NOT verified
;
	LDHI	0x0010,  AR5				; point to peripheral memory map
	OR	0x0071,  AR5				; point to input port of link 3
	SUBI3	2,       R1, RC			; decrement number of words by two and place in count
	BleD	lwr_finished				; only one word ?
	LSH3	-2,      R0, AR0			; convert to a word offset
	ADDI	IR0,  AR0			; and add in base of RAM
	LDI	*AR5, R1				; load first word
						; branch (might) take place now
	RPTS	RC					; and use as repeat AR3 for the next instruction
	LDI	*AR5, R1				; get word from memory
	|| STI	R1,   *AR0++(1)			; and put into link
lwr_finished:
	STI	R1,   *AR0++(1)			; put last word into link
	Bu	R11					; return
.LinkReadBytes:
;
;	void LinkReadBytes( unsigned char * buffer, unsigned long num_bytes_to_read );
;
; Upon Entry:
; 	R0 	contains byte offset of the buffer to place received bytes
; 	R1 	contains the number of BYTES to receive
;	R11	return address
;	IR0	base address of data RAM
;
; Upon Exit
;	no return value	
;
; Corrupts
;	R1, R8, R9, R10, AR0, AR5, ST, RC, RS, RE
;
; Purpose
;	Reads bytes from one of the C40s links - useful for 1 purposes
;	The inputs are NOT verified.
;	The protocol specifies that each byte is sent as a word with 0 in top three bytes.
;
	LSH3	-2,      R0,  AR0			; convert buffer address to a word offset
	ADDI	IR0,  AR0			; and add in base of RAM
						; return (might) take place now
	LSH3	-2,      R1, RC			; put whole number of words in RC
	BeqD	lbr_remainder				; if there are none then skip (delayed)
	LDHI	0x0010,  AR5				; point to peripheral memory map
	OR	0x0071,  AR5				; point to input port of link 3
	SUBI	1,       RC				; count in RC is one less than number of itterations
						; branch (might) take place now
	RPTB	lbr_end_of_block			; repeat the following section of code
	MB0	*AR5, R8				; place bottom byte into first byte
	MB1	*AR5, R8				; place bottom byte into second byte
	MB2	*AR5, R8				; place bottom byte into third byte
	MB3	*AR5, R8				; place bottom byte into fourth byte
lbr_end_of_block:
	STI	R8,   *AR0++(1)			; and save in buffer
lbr_remainder:
	AND	3,       R1				; get remaining number of bytes to send
	BeqD	R11					; none left => return (delayed)
	LDI	0,       R8				; prepare register
	LDI	0,       R9				; prepare register
	LDI	0,       R10				; prepare register
						; return (might) take place now
	MB0	*AR5, R8				; get first "byte" into result
	CMPI	2,       R1				; how many bytes left to fetch ?
	LDIge	*AR5, R9				; 2 or more ? get second "byte"
	LDIgt	*AR5, R10				; 3 ? get third "byte"
	BuD	R11					; return (delayed)
	MB1	R9,    R8				; move bottom byte into second byte of result
	MB2	R10,    R8				; move bottom byte into third byte of result
	STI	R8,   *AR0			; save result
						; return takes place now
	; Pauls clock code
	;
	; clock interrupt functions
	;
	; initialise clock interrupt and handler
	; BEWARE uses the first 0x40 words of on-chip ram
	;
.ClockInit:	
	; assumes ssp set up correctly
	LDHI	0x002f, AR5		; set interrupt vectors to start of on-chip ram
	OR	0xf800, AR5		; AR5 = pcs temp
	LDPE	AR5, IVTP
	PUSH	R11			; set interrupt vector for clock 0 to ClockHandler
	LAJ	4			; get address of ClockHandler
	LDI	ClockHandler - 3, R8	;
	ADDI	R11,   R8		;
	POP	R11			;
	;
	STI	R8, *+AR5(2)	; place in interrupt vect.
	LDHI	0x002f, AR5		; zero timer location (0x002ff840)
	OR	0xf840, AR5		;
	STIK	0,     *AR5		;
	; clock zero period time
	; 32Mhz = 62.5000 ns H1 cycle time
	; period reg resolution = H1/2
	LDHI	0x0010, AR5		; AR5 = pcs temp
	OR 	0x0020, AR5		; tclk0 ctrl reg
	LDI	8000,   R8		; 8000 * 125.0000 ns = 1 millisecond intr
	STI	R8,   *+AR5(8)	; place in clk0 period reg
	; setup clk0 control reg
	; internal clock, reset and go, Tclk0 = Output, output low (local ram)
	LDI	0b01011000010, R8
	STI	R8,  *AR5		; place in clk0 period reg
	LDI	1,      IIE		; enable clock 0 interrupts (and no others)
	OR	0x2000, ST		; enable interrupts globally
	Bu	R11			; return
._cputime:	; C callable function to return number of clock ticks
	BuD 	R11			; return (delayed)
	LDHI	0x002f,  AR5		; point to
	OR	0xf840,  AR5		; clock 
	LDI	*AR5, R0	; return number of clicks
	; Timer 0 interrupt handler
ClockHandler:
	; increment AR3 value - 002ff840 location on-chip ram
	; just after IVT.
	PUSH	ST
	PUSH	AR0
	PUSH	AR1
	LDHI	0x002f, AR0
	OR	0xf840, AR0
	LDI	*AR0,   AR1
	ADDI	1,      AR1		; inc CLK_TCK count
	STI	AR1,   *AR0
	POP 	AR1
	POP 	AR0
	POP 	ST
	RETIu
	; end of 1 code
	; module initialisation code
	; upon entry:
	; R0   contains 0 (initialise data slots) or 2 (initialise function table)
	; R11   return address to code that called us
	; AR4	  word AR1 to module table
	; IR0 base of addressible RAM
	;
	; upon exit:
	; none of the above registers (except R11) can be altered
	;
	; Registers available for use are:
	; RS, AR0, AR2, AR5
init
	CMPI	2,    R0				; are we initialising function table ?
 	Bne	R11					; nope ? then return 
	LDI	R11, AR0				; save return address
 	LAJ	4					; get PC into R11
 	LDI	AR4, AR5				; get module table AR1 into temporary register
patchinstr
(
	PATCHC40MASK16ADD, shift ( 1 , modnum ), 
 	ADDI	1, AR5				; add in our module number
)
 	LDI	*AR5, AR5				; get AR1 to function table
here:						; fake link-and-jump takes place now
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .__stack_overflow, AR2		; load address of __stack_overflow routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .setjmp, AR2			; load address of setjmp routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .longjmp, AR2			; load address of longjmp routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - ._stacksize, AR2			; load address of _stacksize routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .Malloc, AR2			; load address of Malloc routine
	STI	AR2, *AR5++(1)			; save address in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .Free, AR2			; load address of Free routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .LinkWriteWords, AR2		; load address of LinkWriteWords routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .LinkWriteWord, AR2		; load address of LinkWriteWord routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .LinkWriteBytes, AR2		; load address of LinkWriteBytes routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .LinkReadWord, AR2		; load address of LinkReadWord routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .LinkReadWords, AR2		; load address of LinkReadWords routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .LinkReadBytes, AR2		; load address of LinkReadBytes routine
	STI	AR2, *AR5++(1)			; save in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - .ClockInit, AR2			; load address of ClockInit routine
	STI	AR2, *AR5++(1)			; save address in function table
	LDI	R11, AR2				; copy address of here into temporary register
	SUBI	here - ._cputime, AR2			; load address of clock routine
	STI	AR2, *AR5++(1)			; save address in function table
 ; 1
	Bu	AR0					; return
; export the stack overflow function
codetable ___stack_overflow
export    ___stack_overflow
codetable _setjmp
export    _setjmp
codetable _longjmp
export    _longjmp
codetable __stacksize
export    __stacksize
codetable _Malloc
export    _Malloc
codetable _Free
export    _Free
codetable _LinkWriteWords
export    _LinkWriteWords
codetable _LinkWriteWord
export    _LinkWriteWord
codetable _LinkWriteBytes
export    _LinkWriteBytes
codetable _LinkReadWord
export    _LinkReadWord
codetable _LinkReadWords
export    _LinkReadWords
codetable _LinkReadBytes
export    _LinkReadBytes
codetable _ClockInit
export    _ClockInit
codetable __cputime
export    __cputime
 ; ; finished
	data .MaxData, 0
	align		
.ModEnd:
; End of cstart.a
