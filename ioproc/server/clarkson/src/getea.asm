;put into the public domain by Russell Nelson, nelson@clutx.clarkson.edu

ether_byte	db	?,?,?,?

get_eaddr:
;enter with ds:si -> Ethernet address to parse, es:di -> place to put it.
	mov	cx,EADDR_LEN
get_eaddr_2:
	push	cx
	push	di
	mov	di,offset ether_byte
	call	get_hex
	mov	al,cl			;remember the number in al.
	pop	di
	pop	cx
	jc	get_eaddr_3		;exit if no number at all.
	stosb				;store a byte.
	cmp	byte ptr [si],':'	;skip colons between bytes.
	jne	get_eaddr_4
	inc	si
get_eaddr_4:
	loop	get_eaddr_2
	clc
get_eaddr_3:
	ret


