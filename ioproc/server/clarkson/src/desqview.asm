IBMTRDV0.ASM
------------

data	segment at	0
		org	5Ch*4
netbios_int	dw 	?	; netbios interface interrupt to DESQview hook
desqview_call	equ	5	; offset to real driver call in front end hook
		org	0F1h*4
saver_int	dw	?	; interrupt used to save data
		org	4A1h
netbios_flags	db	?	; NETBIOS interface work flags
data		ends





IBMTRDV1.ASM
------------

; Memory patches to allow running IBMTOKEN in a DESQview application.
; Patch 1/3, before loading DXMA0MOD.SYS
; A. Pirard. 13 aug 1990.

	include t0.asm

code	segment	byte public

	assume	cs:code
	org	100h
start:

	mov	ax,0
	mov	ds,ax
	assume	ds:data
	pushf
	cli
	les	bx,dword ptr netbios_int
	mov	saver_int,bx		; save DV's int 5C
	mov	saver_int+2,es
	mov	netbios_int,0		; allowing DXMA0MOD new start requires
	mov	netbios_int+2,0		; untouched looking interrupt 5C
        mov	netbios_flags,0		; and initially zero flags
	popf
	ret

code	ends

	end	start




IBMTRDV2.ASM
------------

; Memory patches to allow running IBMTOKEN in a DESQview application.
; Patch 2/3, after loading DXMA0MOD.SYS
; A. Pirard. 13 aug 1990.

	include t0.asm

code	segment	byte public

	assume	cs:code
	org	100h
start:

	mov	ax,0
	mov	ds,ax
	assume	ds:data
	pushf
	cli
	les	bx,dword ptr saver_int	; get back DV's Int 5C handler
	mov	cx,netbios_int		; get DXMA0MOD's one
	mov	dx,netbios_int+2	; and swap it with the one DV calls
	mov	ax,es: desqview_call [bx]
	mov	es: desqview_call [bx],cx
	mov	cx,es: desqview_call+2 [bx]
	mov	es: desqview_call+2 [bx],dx
	mov	saver_int,ax		; save address DV's int 5C called
	mov	saver_int+2,cx
	mov	netbios_int,bx		; restore DV's Int 5C handler
	mov	netbios_int+2,es
	popf
	ret

code	ends

	end	start




IBMTRDV3.ASM
------------

; Memory patches to allow running IBMTOKEN in a DESQview application.
; Patch 3/3, before closing DESQview application.
; A. Pirard. 13 aug 1990.

	include t0.asm

code	segment	byte public

	assume	cs:code
	org	100h
start:

	mov	ax,0
	mov	ds,ax
	assume	ds:data
	pushf
	cli
	les	bx,dword ptr netbios_int    ; get DV's int 5C
	lds	ax,dword ptr saver_int	    ; and handler it called
	assume	ds:nothing
	mov	es: desqview_call [bx],ax   ; restore
	mov	es: desqview_call+2 [bx],ds
	popf
	ret

code	ends

	end	start

	mov	saver_int+2,es
	mov	netbios_int,0		; allowing DXMA0MOD new start requires
	mov	netbios_int+2,0		; untouched looking interrupt 5C
        mov	netbios_flags,0		; and initially zero flags
	popf
	ret

code	ends

	end	start

