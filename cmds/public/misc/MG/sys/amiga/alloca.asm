*
* alloca()
*
* This routine was written by Stephen Walton, swalton@solar.stanford.edu,
* a rank asm amateur, to attempt to provide the alloca() function for Manx
* Aztec C on the Amiga.  It probably does horrible illegal things to the
* stack but seems to mostly work.
*
* This subroutine expects a single int as argument, and returns a pointer
* to a block of memory of that size allocated off the local stack.  Thus,
* this memory is automatically freed when the current subroutine exits.
*
* This version for the default Manx settings (int = 16 bits).  To use
* with int=32 bits, simply change both the ".w" to ".l"
*
	xdef	_alloca
_alloca
	move.l	a7,a1		; Save current value of stack pointer
	move.w	4(a7),d0	; Number of bytes needed
	suba.w	d0,a7		; Move stack up that many places
	move.l	(a1),(a7)	; Place return address in proper place
	move.l	a7,d0		; Return value into d0
	add.l	#6,d0		;  plus amount by which stack is popped
	rts			; And back we go
