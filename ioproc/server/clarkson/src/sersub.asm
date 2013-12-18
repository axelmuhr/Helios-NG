open:
	;Set line control register: 8 bits, no parity
	mov	al,LCR_8BITS
	mov	dx,io_addr	; point to chip 
	add	dx,LCR		; add offset to line control register
	out	dx,al		; set characteristics

	;Set modem control register: assert DTR, RTS
	mov	al,MCR_DTR or MCR_RTS
	mov	dx,io_addr		; point to chip 
	add	dx,MCR			; add offset to modem control register
	out	dx,al			; set characteristics

;compute the divisor given the baud rate.
	mov	dx,baudclk+2
	mov	ax,baudclk
	mov	bx,0
asy_speed_1:
	inc	bx
	sub	ax,baud_rate
	sbb	dx,baud_rate+2
	jnc	asy_speed_1
	dec	bx
	add	ax,baud_rate
	adc	dx,baud_rate+2
	or	ax,dx

	;Purge the receive data buffer
	mov	dx,io_addr			;point to chip
	add	dx,RBR
	in	al,dx

	mov	ah,LCR_DLAB		;Turn on divisor latch access bit
	mov	dx,io_addr			;point to chip
	add	dx,LCR
	call	setbit

	mov	al,bl			;Load the two bytes of the divisor.
	mov	dx,io_addr			;point to chip
	add	dx,DLL
	out	dx,al
	mov	al,bh
	mov	dx,io_addr			;point to chip
	add	dx,DLM
	out	dx,al

	mov	ah,LCR_DLAB		;Turn off divisor latch access bit
	mov	dx,io_addr			;point to chip transmit 
	add	dx,LCR
	call	clrbit

	clc				;indicate no errors.
	ret

pr_ch:
	push	ds	; save ds so we can restore later
	push	cs	; push cs and pop into ds
	pop	ds
	push	dx	; save dx so we can use it to point at i/o port
	pushf		; save flags then inhibit interrupts
	cli
	push	ax	; save ax (al contains or character)
;
;	Now wait for holding register to empty
;
	mov	dx,io_addr	;point to chip line status register
	add	dx,LSR		; set up to check for an empty buffer
xmit_top:
	in	al, dx		; get line status
	test	al,LSR_THRE	; check if holding register empty
	jnz	xmit_done	; yes so we can go and output
	jmp	xmit_top	; holding register full still so loop

xmit_done:	; end of loop - holding register now empty

	pop	ax		; now get our character
	mov	dx,io_addr	; point to chip
	add	dx,THR		; add offset to transmit holding register
; xmit a character
	out	dx, al		; and output our character
	popf			; restore flags (and interrupt state)
	pop	dx		; restore dx
	pop	ds		; restore ds to original value
	ret

;
;	NOW SOME DATA AREAS
;

io_addr		dw	03f8h,0		; I/O address for COM1.
baud_rate	dw	4b00h,0		; We support baud rates higher than 65535.
baudclk		label	word
		dd	115200		;1.8432 Mhz / 16



