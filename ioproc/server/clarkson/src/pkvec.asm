	.MODEL	MEMMOD,C
	%MACS
	extrn	pkint:proc

	.DATA
	extrn	Stktop

	.CODE
sssave	dw	?
spsave	dw	?
dbase	dw	@Data

; pkvec0 - Packet driver receive call handler #0
	public	pkvec0
	label	pkvec0	far
	pushf			; save his interrupt state
	cli			; no distractions
	mov	cs:sssave,ss	; stash user stack context
	mov	cs:spsave,sp

	mov	ss,cs:dbase	; set up interrupt stack
	lea	sp,Stktop

	; save regs, making them available to pkint()
	push	es
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push	si
	push	di

	mov	ax,ss
	mov	ds,ax
	mov	ax,0	; device #0
	push	ax
	call	pkint
	jmp	pkret

; pkvec1 - Packet driver receive call handler #1
	public	pkvec1
	label	pkvec1	far
	pushf			; save his interrupt state
	cli			; no distractions
	mov	cs:sssave,ss	; stash user stack context
	mov	cs:spsave,sp

	mov	ss,cs:dbase	; set up interrupt stack
	lea	sp,Stktop

	; save regs, making them available to pkint()
	push	es
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push	si
	push	di

	mov	ax,ss
	mov	ds,ax
	mov	ax,1	; device #1
	push	ax
	call	pkint
	jmp	pkret

; pkvec2 - Packet driver receive call handler #2
	public	pkvec2
	label	pkvec2	far
	pushf			; save his interrupt state
	cli			; no distractions
	mov	cs:sssave,ss	; stash user stack context
	mov	cs:spsave,sp

	mov	ss,cs:dbase	; set up interrupt stack
	lea	sp,Stktop

	; save regs, making them available to pkint()
	push	es
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	push	bp
	push	si
	push	di

	mov	ax,ss
	mov	ds,ax
	mov	ax,2	; device #2
	push	ax
	call	pkint
	jmp	pkret

; common return for all packet drivers
	label	pkret	near
	pop	ax	; pop dev # arg
	pop	di
	pop	si
	pop	bp
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	es

	mov	ss,cs:sssave
	mov	sp,cs:spsave	; restore original stack context
	popf
	retf

	end
