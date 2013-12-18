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

