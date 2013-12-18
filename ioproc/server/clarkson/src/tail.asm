;   PC/FTP Packet Driver source, conforming to version 1.05 of the spec
;   Russell Nelson, Clarkson University.  July 20, 1988
;   Updated to version 1.08 Feb. 17, 1989.
;   Copyright 1988,1989,1990,1991 Russell Nelson

;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation, version 1.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	include	defs.asm

code	segment word public
	assume	cs:code, ds:code

	extrn	phd_dioa: byte
	extrn	phd_environ: word
	extrn	flagbyte: byte

	include	printnum.asm
	include	decout.asm
	include	digout.asm
	include	chrout.asm

end_tail_1	label	byte		; end of the delayed init driver

;usage_msg is of the form "usage: driver [-d -n] <packet_int_no> <args>"
	extrn	usage_msg: byte

;copyright_msg is of the form:
;"Packet driver for the foobar",CR,LF
;"Portions Copyright 19xx, J. Random Hacker".
	extrn	copyright_msg: byte

copyleft_msg	label	byte
 db "Packet driver skeleton copyright 1988-91, Russell Nelson.",CR,LF
 db "This program is free software; see the file COPYING for details.",CR,LF
 db "NO WARRANTY; see the file COPYING for details.",CR,LF
 db CR,LF
crlf_msg	db	CR,LF,'$'

no_resident_msg	label	byte
 db CR,LF,"*** Packet driver failed to initialize the board ***",CR,LF,'$'

;parse_args should parse the arguments.
;called with ds:si -> immediately after the packet_int_no.
	extrn	parse_args: near

;print_parameters should print the arguments.
	extrn	print_parameters: near

	extrn	our_isr: near, their_isr: dword
	extrn	packet_int_no: byte
	extrn	is_at: byte, sys_features: byte
	extrn	int_no: byte
	extrn	driver_class: byte

location_msg	db	"Packet driver is at segment ",'$'

packet_int_no_name	db	"Packet interrupt number ",'$'
eaddr_msg	db	"My Ethernet address is ",'$'
aaddr_msg	db	"My ARCnet address is ",'$'

already_msg	db	CR,LF,"There is already a packet driver at ",'$'
int_msg		db	CR,LF
		db	"Error: <int_no> should be between 0 and "
int_msg_num	label	word
		db	"15 inclusive", '$'

our_address	db	EADDR_LEN dup(?)
	public	etopen_diagn
etopen_diagn	db	0		; errorlevel from etopen if set

;etopen should initialize the device.  If it needs to give an error, it
;can issue the error message and quit to dos.
	extrn	etopen: near

;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	extrn	get_address: near

already_error:
	mov	dx,offset already_msg
	mov	di,offset packet_int_no
	call	print_number
	mov	ax,4c05h		; give errorlevel 5
	int	21h

usage_error:
	mov	dx,offset usage_msg
	public	error
error:
	mov	ah,9
	int	21h
	mov	ax,4c0ah		; give errorlevel 10
	int	21h

	include	timeout.asm

	public	start_1
start_1:
	cld

	mov	dx,offset copyright_msg
	mov	ah,9
	int	21h

	mov	dx,offset copyleft_msg
	mov	ah,9
	int	21h

;
; Get the feature byte (if reliable) so we can know if it is a microchannel
; computer and how many interrupts there are.
;
	mov	ah,0c0h
	int	15h			; es:bx <- sys features block
	jc	look_in_ROM		; error, must use rom.
	or	ah,ah
	jnz	look_in_ROM
	mov	dx,es:[bx]		; # of feature bytes
	cmp	dx,4			; do we have the feature byte we want?
	jae	got_features		;yes.
look_in_ROM:
	mov	dx,0f000h		;ROM segment
	mov	es,dx
	cmp	byte ptr es:[0fffeh],0fch;is this an AT?
	jne	identified		;no.
	or	sys_features,TWO_8259	; ATs have 2nd 8259
	jmp	identified		; assume no microchannel
got_features:
	mov	ah,es:[bx+2]		; model byte
	cmp	ah,0fch
	je	at_ps2
	ja	identified		; FD, FE and FF are not ATs
	cmp	ah,0f8h
	je	at_ps2
	ja	identified		; F9, FA and FB are not ATs
	cmp	ah,09ah
	jbe	identified		; old non-AT Compacs go here
at_ps2:					; 9B - F8 and FC are assumed to
	mov	ah,es:[bx+5]		;   have reliable feature byte
	mov	sys_features,ah
identified:

	mov	si,offset phd_dioa+1
	call	skip_blanks		;end of line?
	cmp	al,CR
	je	usage_error_j_1

;print the location we were loaded at.
	mov	dx,offset location_msg
	mov	ah,9
	int	21h

	mov	ax,cs			;print cs as a word.
	call	wordout

	mov	dx,offset crlf_msg
	mov	ah,9
	int	21h

chk_options:
	call	skip_blanks
	cmp	al,'-'			; any options?
	jne	no_more_opt
	inc	si			; skip past option char
	lodsb				; read next char
	or	al,20h			; convert to lower case
	cmp	al,'d'
	jne	not_d_opt
	or	flagbyte,D_OPTION
	jmp	chk_options
not_d_opt:
	cmp	al,'n'
	jne	not_n_opt
	or	flagbyte,N_OPTION
	jmp	chk_options
not_n_opt:
	cmp	al,'w'
	jne	not_w_opt
	or	flagbyte,W_OPTION
	jmp	chk_options
not_w_opt:
usage_error_j_1:
	jmp	usage_error
no_more_opt:

	mov	di,offset packet_int_no	;parse the packet interrupt number
	mov	bx,offset packet_int_no_name
	call	get_number		;  for them.

	call	parse_args
	jc	usage_error_j_1

	call	skip_blanks		;end of line?
	cmp	al,CR
	jne	usage_error_j_1

	call	verify_packet_int
	jnc	packet_int_ok
	jmp	error
packet_int_ok:
	jne	packet_int_unused
	jmp	already_error		;give an error if there's one there.
packet_int_unused:

;
; Verify that the interrupt number they gave is valid.
;
	cmp	int_no,15		;can't possibly be > 15.
	ja	int_bad
	test	sys_features,TWO_8259	; 2nd 8259 ?
	jnz	int_ok			;yes, no need to check for <= 7.
	mov	int_msg_num,'7'+' '*256	;correct the error message, just in case.
	cmp	int_no,7		;make sure that the packet interrupt
	jbe	int_ok			;  number is in range.
int_bad:
	mov	dx,offset int_msg
	jmp	error
int_ok:

;
; Map IRQ 2 to IRQ 9 if needed.
;
	test	sys_features,TWO_8259	; 2nd 8259 ?
	je	no_mapping_needed	;no, no mapping needed
	cmp	int_no,2		;map IRQ 2 to IRQ 9.
	jne	no_mapping_needed
	mov	int_no,9
no_mapping_needed:

; If they chose the -d option, don't call etopen when we are loaded,
; but when we are called for the first time
;
; Save part of the tail, needed by delayed etopen
	test	flagbyte,D_OPTION
	jnz	delayed_open
	call	etopen			;init the driver.  If any errors,
					;this routine returns cy.
	jnc	yes_resident
	jmp	no_resident
delayed_open:
	mov	dx,offset end_tail_1	; save first part of tail
	push	dx			;remember where they want to end.
	call	take_packet_int
	jmp	delayed_open_1

yes_resident:
	push	dx			;remember where they want to end.

	call	print_parameters	;echo our parameters.
	or	flagbyte,CALLED_ETOPEN

	call	take_packet_int

	cmp	driver_class,1		;Ethernet?
	jne	print_addr_2		;no, don't print what we don't have.

	push	ds
	pop	es
	mov	di,offset our_address
	mov	cx,EADDR_LEN
	call	get_address

	mov	dx,offset eaddr_msg
	mov	ah,9
	int	21h

	mov	si,offset our_address
	call	print_ether_addr

	mov	dx,offset crlf_msg	;can't depend on DOS to newline for us.
	mov	ah,9
	int	21h

print_addr_2:

	cmp	driver_class,8		;ARCnet?
	jne	print_addr_3		;no, don't print what we don't have.

	push	ds
	pop	es
	mov	di,offset our_address
	mov	cx,ARCADDR_LEN
	call	get_address

	mov	dx,offset aaddr_msg
	mov	ah,9
	int	21h

	mov	al,our_address
	mov	cl,' '			;Don't eliminate leading zeroes.
	call	byteout

	mov	dx,offset crlf_msg	;can't depend on DOS to newline for us.
	mov	ah,9
	int	21h

print_addr_3:
delayed_open_1:

	mov	ah,49h			;free our environment, because
	mov	es,phd_environ		;  we won't need it.
	int	21h

	mov	bx,1			;get the stdout handle.
	mov	ah,3eh			;close it in case they redirected it.
	int	21h

	pop	dx			;get their ending address.
	add	dx,0fh			;round up to next highest paragraph.
	mov	cl,4
	shr	dx,cl
	mov	ah,31h			;terminate, stay resident.
	mov	al,etopen_diagn		; errorlevel (0 - 9, just diagnostics)
	int	21h

no_resident:
	mov	dx,offset no_resident_msg
	mov	ah,9
	int	21h

	mov	ax,4c00h + 32		; give errorlevel 32
	cmp	al,etopen_diagn
	ja	no_et_diagn		; etopen gave specific reason?
	mov	al,etopen_diagn		; yes, use that for error level
no_et_diagn:
	int	21h

; 			Suggested errorlevels:
;
; _____________________  0 = normal
; 			 1 = unsuitable memory address given; corrected
; In most cases every-	 2 = unsuitable IRQ level given; corrected
; thing should work as	 3 = unsuitable DMA channel given; corrected
; expected for lev 1-5	 4 = unsuitable IO addr given; corrected (only 1 card)
; _____________________	 5 = packet driver for this int # already loaded
; External errors, when	20 = general cable failure (but pkt driver is loaded)
; corrected normal	21 = network cable is open             -"-
; operation starts	22 = network cable is shorted          -"-
; _____________________ 23 = 
; Packet driver not	30 = usage message
; loaded. A new load	31 = arguments out of range
; attempt must be done	32 = unspecified device initialization error
;			33 = 
;			34 = suggested memory already occupied
;			35 = suggested IRQ already occupied
;			36 = suggested DMA channel already occupied
;			37 = could not find the network card at this IO address


take_packet_int:
	mov	ah,35h			;remember their packet interrupt.
	mov	al,packet_int_no
	int	21h
	mov	their_isr.offs,bx
	mov	their_isr.segm,es

	mov	ah,25h			;install our packet interrupt
	mov	dx,offset our_isr
	int	21h
	ret

	include	verifypi.asm
	include	getnum.asm
	include	getdig.asm
	include	skipblk.asm
	include	printea.asm

code	ends

	end
