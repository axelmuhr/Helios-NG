;put into the public domain by Russell Nelson, nelson@clutx.clarkson.edu

;movemem has one of three values: movemem_test, movemem_386, movemem_86.
;it's initialized to the first, and the code that it points to changes
;it to the appropriate move routine.

movemem	dw	movemem_test

movemem_test:
	pushf
	pop	ax
	or	ax,7000h		;the 386 lets us set these bits
	push	ax
	popf
	pushf
	pop	ax
	test	ax,7000h		;did the bits get set?
	mov	ax,offset movemem_86
	je	movemem_test_1		;no.
	mov	ax,offset movemem_386	;yes, use a 386-optimized move.
movemem_test_1:
	mov	cs:movemem,ax
	jmp	ax

movemem_386:
movemem_86:
;does the same thing as "rep movsb", only 50% faster.
;moves words instead of bytes, and handles the case of both addresses odd
;efficiently.  There is no way to handle one address odd efficiently.
;This routine always aligns the source address in the hopes that the
;destination address will also get aligned.  This is from Phil Karn's
;code from ec.c, a part of his NET package.  I bummed a few instructions
;out.
	jcxz	movemem_cnte		; If zero, we're done already.
	test	si,1			; Does source start on odd byte?
	jz	movemem_adre		; Go if not
	movsb				; Yes, move the first byte
	dec	cx			; Count that byte
movemem_adre:
	shr	cx,1			; convert to word count
	rep	movsw			; Move the bulk as words
	jnc	movemem_cnte		; Go if the count was even
	movsb				; Move leftover last byte
movemem_cnte:
	ret


