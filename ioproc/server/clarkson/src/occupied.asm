; Subroutine to check if an address range is available for allocation
; as shared memory.
; Input:  bx = segment start address
;	  di = # of bytes / 16
; Returns with carry set if range is occupied
; All registers saved and restored
;
	public	occupied_chk
occupied_chk:
	push	ax			; save registers
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	ds
;
; first check if there is any ROM in our range
;
	add	di,bx			; end of range to check
	mov	dx,0c000h-(2048 shr 4)	; start of possible ROM area
occ_next2k:
	add	dx,2048 shr 4		; next 2k segment
occ_nextROM:
	cmp	dx,di			; ROM addr above our range?
	jae	occ_chk_RAM		; -yes, go check for RAM
	mov	ds,dx			; -no, check for ROM
	xor	si,si
	mov	cx,0aa55h		; ROM signature
	mov	ax,[si]
	cmp	ax,cx			; any signature?
	jne	occ_next2k
	mov	ch,[si+2]		; ROM length in 512 byte blocks
	xor	cl,cl
	push	cx
	shl	cx,1			; # of bytes to checksum
	xor	ah,ah			; clear accumulated checksum
occ_chksum:
	lodsb				; get a byte
	add	ah,al			; add modulo 100h
	loop	occ_chksum		; repeat for all bytes
	pop	cx
	or	ah,ah			; is checksum zero?
	jnz	occ_next2k		; -no, check next 2k addr 
	shr	cx,1			; -yes, valid ROM found
	shr	cx,1
	shr	cx,1
	add	dx,cx			; end of ROM
	cmp	dx,bx			; does it reach into our area?
	ja	occ_is_ROM
	jmp	occ_nextROM
;
; no ROM found in our range, check for RAM
;
occ_chk_RAM:
	mov	ds,bx			; point segment at start of range
	sub	di,bx			; compute # of bytes
	mov	cl,4
	shl	di,cl
	xor	bx,bx			; clear word pointer
	mov	ax,0a5a5h		; test pattern
	mov	si,05a5ah		;   and its complement
occ_nextword:
	pushf
	cli				; disable interrupts
	mov	cx,[bx] 		; save original contents
	mov	dx,[bx+2]		; save original contents
	mov	[bx],si 		; put our pattern
	mov	[bx+2],ax		; drain any capacitive memory
	cmp	[bx],si 		; is our pattern still there?
	mov	[bx],cx 		; restore original contents
	mov	[bx+2],dx		; restore original contents
	jne	occ_no_RAM
	mov	[bx],ax 		; put the complement pattern
	mov	[bx+2],si		; drain capacitive memory
	cmp	[bx],ax 		; our pattern still there?
	mov	[bx],cx 		; restore original contents
	mov	[bx+2],dx		; restore original contents
	je	occ_is_RAM		; -yes, range is occupied
occ_no_RAM:
	popf				; enable interrupt
	add	bx,2			; check next word
	cmp	bx,di			; at end of range?
	jb	occ_nextword
	clc				; clear carry - range is available
	jmp	short occ_return
occ_is_RAM:
	popf				; enable interrupts
occ_is_ROM:
	stc				; set carry - range is occupied
occ_return:
	pop	ds			; restore registers
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	ret

