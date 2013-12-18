;
; Resident program to test the Helios call-trap facility.
;
; Install a simple routine at interrupt 0x60, to be called by the Server
; when it receives a private protocol message for the /pc device. The 
; routine gets a pointer to the Server MCB in registers ds:dx, allowing it
; to manipulate the message before it is sent back to the client - make sure
; that the data size and control size entries in the MCB are set correctly.
; The routine should return in under two seconds, with a 32-bit reply code
; in dx:ax - this is sent back as the message FnRc to the client.
;

cseg	segment	para	public	'CODE'

	org	100H

	assume	cs:cseg, ds:cseg, es:cseg, ss:cseg

Init	proc	near

	mov	dx,cs		; force all segment registers to a sensible
	mov	ds,dx		; value
	mov	es,dx
	mov	dx,offset trap	; install routine "trap" as the interrupt
	mov	ax,2560H	; vector for trap 0x60
	int	21H

	mov	dx,offset signon	; display a message to show that
	mov	ah,9			; the routine is installed
	int	21H

	mov	dx,((offset Pgm_Len+15)/16)+20H	; terminate but stay resident
	mov	ax,3100H
	int	21H

Init	endp

;
; This is the routine that will be activated by the Server
;
trap	proc	far
	sti			; make sure that interrupts are enabled

	mov	ax,cs		; set the data segment register
	mov	ds,ax
	mov	dx,offset warn	; display a message to show that the trap has
	mov	ah,9		; been activated
	int 21h
	mov	dx,8765H	; return code 0x87654321
	mov	ax,4321H

	iret

trap	endp

cr	equ	0dH
lf	equ	0AH

signon	db	cr,lf,'Trap 60 handler installed',cr,lf,'$'
warn	db	cr,lf,'Trap 60 activated',cr,lf,'$'

Pgm_Len	equ	$-Init		; size of this program, needed to terminate
				; but stay resident
cseg	ends

	end	init
	
