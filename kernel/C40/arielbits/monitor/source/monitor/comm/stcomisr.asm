	.globl	_SetISRs

	.text
;  void SetISRs( fcn_ptr stubs ) 
_SetISRs:	
	ldi	@CONST+1,r0
	ldpe	r0,ivtp

	ldi	sp,ar0
	ldi	*-ar0(1),ar0	; Get pointer to function pointer array
	ldep	ivtp,ar1	; Get base address of interrupt vector table

	addi	0eh,ar1		; Make it point to the first comm ready int.
	ldi	5,rc		; Repeat 6 times
	rptb	INSTALL
	ldi	*ar0++,r0	; Get input function pointer
	sti	r0,*ar1++	; Install into vector table
	ldi	*ar0++,r0	; Get output function pointer
	sti	r0,*ar1++	; Install into vector table
INSTALL	addi	2,ar1		; Point to next set of vectors 
				; (See Figure 6-5, pg. 6-26)

  	ldi	@CONST,r0	; Enable the comm port input interrupts
   	or	r0,iie

	or	2000h,st	; Enable interrupts

	rets

	.bss	CONST,2
	.sect ".cinit"
	.word	2,CONST
	.word	0444444h	; Enables the input  ready
				; for all 6 comm ports.
	.word	04000000h	; Local SRAM base
	.end
