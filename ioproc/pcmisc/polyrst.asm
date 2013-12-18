;
; Resident program to activate the reset hardware on the 
; Perihelion Hardware DYGC board
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
; The FnCode should be encoded as follows:
;	Top byte  :   FC_Private
;	Next byte :   0 -> global reset
;		      1 -> individual reset
;		      2 -> individual analyse
;	Bottom byte : processor number (between 1 and 13)
;		      1 is the T212/C004 control block
;		      2-13 are the twelve processors

global	equ	01e0H
slow	equ	01e4H
fast	equ	01e5H

trap	proc	far
	sti			; make sure that interrupts are enabled

	mov	si,dx		; ds:xx[si] can now be used to get the
				; function code
	xor	ah,ah
	mov	al,ds:14[si]	; most significant byte but one
	cmp	al,0		; -> global reset
	jne	not_global

				; asserting a global reset -> writing to 1e0
	mov	al,1
	mov	dx,global
	out	dx,al
	mov	cx,8000		; same timeout as for b004_reset
wait1:
	loop	wait1
	xor	al,al
	out	dx,al
	jmp	done

not_global:
;	for now do not bother with analyse, only with reset
;	mov	al,ds:14[si]	; work out the control code;
;	cmp	al,1		; reset
;	jne	try_analyse	

				; code to perform an individual reset
	call	pulses
	mov	dx,fast		; generate 3 pulses on fast to assert reset
	mov	al,1		; 1st
	out	dx,al
	mov	al,0
	out	dx,al
	mov	al,1		; 2nd
	out	dx,al
	mov	al,0
	out	dx,al
	mov	al,1		; 3rd
	out	dx,al
	mov	al,0
	out	dx,al
	call	endpulses

	mov	cx,8000		; wait a while
wait2:
	loop	wait2

	call	pulses
	mov	dx,fast		; now generate a single pulse to lower reset
	mov	al,1
	out	dx,al
	mov	al,0
	out	dx,al
	call	endpulses

done:
	mov	dx,0		; return code
	mov	ax,0

	iret

trap	endp

;
; This routine generates the initial pulses, in accordance with the
; Polyhedron Users Manual page 8-4
;
pulses	proc	near
				; this is taken from Polyhedron Users Manual p8-4
	mov	dx,slow		; slow signal
	mov	al,0
	out	dx,al		; set it low
	mov	cx,26		; then generate 26 pulses
	mov	dx,fast		; on fast
loop1:		  
	mov	al,1
	out	dx,al
	mov	al,0
	out	dx,al
	loop	loop1

				; generate addressing pulses, part 1
	mov	dx,slow		; back to slow
	mov	al,1
	out	dx,al		; set high
	mov	dx,fast		; back to fast
	mov	al,1
	out	dx,al		; and pulse
	mov	al,0
	out	dx,al

				; generate addressing pulses, part 2
	mov	al,ds:12[si]	; work out (2 * nn) - 1
	mov	ah,0
	add	al,al
	dec	ax
	mov	dx,slow		; back to slow
	mov	al,0
	out	dx,al		; set low
	mov	dx,fast
	mov	cx,ax
loop2:
	mov	al,1
	out	dx,al		; (2 * nn) - 1 pulses
	mov	al,0
	out	dx,al
	loop	loop2

	mov	dx,slow
	mov	al,1
	out	dx,al

	ret

pulses	endp

;
; This is used to generate the final set of pulses
;
endpulses	proc	near
	mov	dx,slow		; set slow low again
	mov	al,0
	out	dx,al
	mov	dx,fast		; and generate 5 pulses on fast
	mov	cx,5
loop3:
	mov	al,1
	out	dx,al
	mov	al,0
	out	dx,al
	loop	loop3

	ret
endpulses	endp

;
; The sign-on message
;	
cr	equ	0dH
lf	equ	0AH

signon	db	cr,lf,'Polyhedron Reset TSR installed',cr,lf,'$'

Pgm_Len	equ	$-Init		; size of this program, needed to terminate
				; but stay resident
cseg	ends

	end	init
	
