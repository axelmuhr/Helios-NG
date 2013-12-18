	include	defs.asm

;  Copyright, 1988-9, 1990, Russell Nelson

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

code	segment word public
	assume	cs:code, ds:code

	public	phd_environ
	org	2ch
phd_environ	dw	?

	public	phd_dioa
	org	80h
phd_dioa	label	byte

	org	100h
start:
	jmp	start_1
	extrn	start_1: near
	even				;put the stack on a word boundary.

;we use our dioa for a stack space.  Very hard usage has shown that only
;  27 bytes were being used, so 128 should be sufficient.
our_stack	label	byte


	extrn	int_no: byte
	public	packet_int_no, is_at, sys_features, flagbyte
packet_int_no	db	?,?,?,?		; interrupt to communicate.
is_at		db	0		; =1 if we're on an AT.
sys_features	db	0		; 2h = MC   40h = 2nd 8259
flagbyte	db	0
	even

functions	label	word
	dw	f_not_implemented	;0
	dw	f_driver_info		;1
	dw	f_access_type		;2
	dw	f_release_type		;3
	dw	f_send_pkt		;4
	dw	f_terminate		;5
	dw	f_get_address		;6
	dw	f_reset_interface	;7
	dw	f_stop			;8
	dw	f_not_implemented	;9
	dw	f_get_parameters	;10
	dw	f_not_implemented	;11
	dw	f_as_send_pkt		;12
	dw	f_drop_pkt		;13
	dw	f_not_implemented	;14
	dw	f_not_implemented	;15
	dw	f_not_implemented	;16
	dw	f_not_implemented	;17
	dw	f_not_implemented	;18
	dw	f_not_implemented	;19
	dw	f_set_rcv_mode		;20
	dw	f_get_rcv_mode		;21
	dw	f_set_multicast_list	;22
	dw	f_get_multicast_list	;23
	dw	f_get_statistics	;24
	dw	f_set_address		;25

	extrn	driver_class: byte
	extrn	driver_type: byte
	extrn	driver_name: byte
	extrn	driver_function: byte
	extrn	parameter_list: byte

	extrn	send_pkt: near
	extrn	as_send_pkt: near
	extrn	drop_pkt: near
	extrn	get_address: near
	extrn	set_address: near
	extrn	terminate: near
	extrn	reset_interface: near
	extrn	xmit: near
	extrn	recv: near
	extrn	recv_exiting: near
	extrn	etopen: near

	extrn	rcv_modes: word		;count of modes followed by mode handles.

	extrn	set_multicast_list: near

linc	macro	n			; inc a 32 bit integer
	local	a
	inc	n			;increment the low word
	jne	a			;go if not overflow
	inc	n+2			;increment the high word
a:
	endm

per_handle	struc
in_use		db	0		;non-zero if this handle is in use.
packet_type	db	MAX_P_LEN dup(0);associated packet type.
packet_type_len	dw	0		;associated packet type length.
receiver	dd	0		;receiver handler.
receiver_sig	db	8 dup(?)	;signature at the receiver handler.
class		db	?		;interface class
per_handle	ends

handles		per_handle MAX_HANDLE dup(<>)
end_handles	label	byte

	public	multicast_count, multicast_addrs, multicast_broad
multicast_count	dw	0		;count of stored multicast addresses.
multicast_broad	db	0ffh,0ffh,0ffh,0ffh,0ffh,0ffh	; entry for broadcast
multicast_addrs	db	MAX_MULTICAST*EADDR_LEN dup(?)

have_my_address	db	0		;nonzero if our address has been set.
my_address	db	MAX_ADDR_LEN dup(?)
my_address_len	dw	?

rcv_mode_num	dw	3

free_handle	dw	0		; temp, a handle not in use
found_handle	dw	0		; temp, handle for our packet
receive_ptr	dd	0		; the pkt receive service routine

	public	send_head, send_tail
send_head	dd	0		; head of transmit queue
send_tail	dd	0		; tail of transmit queue

statistics_list	label	dword
packets_in	dw	?,?
packets_out	dw	?,?
bytes_in	dw	?,?
bytes_out	dw	?,?
errors_in	dw	?,?
errors_out	dw	?,?
packets_dropped	dw	?,?		;dropped due to no type handler.

savess		dw	?		;saved during the stack swap.
savesp		dw	?

regs	struc				; stack offsets of incoming regs
_ES	dw	?
_DS	dw	?
_BP	dw	?
_DI	dw	?
_SI	dw	?
_DX	dw	?
_CX	dw	?
_BX	dw	?
_AX	dw	?
_IP	dw	?
_CS	dw	?
_F	dw	?			; flags, Carry flag is bit 0
regs	ends

CY	equ	0001h
EI	equ	0200h


bytes	struc				; stack offsets of incoming regs
	dw	?			; es, ds, bp, di, si are 16 bits
	dw	?
	dw	?
	dw	?
	dw	?
_DL	db	?
_DH	db	?
_CL	db	?
_CH	db	?
_BL	db	?
_BH	db	?
_AL	db	?
_AH	db	?
bytes	ends

	public	our_isr, their_isr
their_isr	dd	0		; original owner of pkt driver int

our_isr:
	jmp	our_isr_0		;the required signature.
	db	'PKT DRVR',0
our_isr_0:
	assume	ds:nothing
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
	push	ds
	push	es
	cld
	mov	bx,cs			;set up ds.
	mov	ds,bx
	assume	ds:code
	mov	bp,sp			;we use bp to access the original regs.
	and	_F[bp],not CY		;start by clearing the carry flag.

	test	flagbyte,CALLED_ETOPEN	; have we initialized the card?
	jnz	our_isr_cont		; yes
	push	ax			; save lots of registers
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	bp
	push	ds
	push	es

	call	etopen			; init the card

	pop	es			; restore lots of registers
	pop	ds
	pop	bp
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	mov	dh,CANT_RESET		; (actually can't initialize)
	jc	our_isr_error
	or	flagbyte,CALLED_ETOPEN	; remember this fact
 our_isr_cont:

	mov	bl,ah			;jump to the correct function.
	mov	bh,0
	cmp	bx,25			;only twenty five functions right now.
	mov	dh,BAD_COMMAND		;in case we find a bad number.
	ja	our_isr_error
	add	bx,bx			;*2
	call	functions[bx]
	jnc	our_isr_return
our_isr_error:
	mov	_DH[bp],dh
	or	_F[bp],CY		;return their carry flag.
our_isr_return:
	pop	es
	pop	ds
	pop	bp
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	iret

	public	re_enable_interrupts
re_enable_interrupts:
; Possibly re-enable interrupts.  We put this here so that other routines
; don't need to know how we put things on the stack.
	test	_F[bp], EI		; Were interrupts enabled on pkt driver entry?
	je	re_enable_interrupts_1	; No.
	sti				; Yes, re-enable interrupts now.
re_enable_interrupts_1:
	ret


f_not_implemented:
	mov	dh,BAD_COMMAND
	stc
	ret


f_driver_info:
;	As of 1.08, the handle is optional, so we no longer verify it.
;	call	verify_handle
	cmp	_AL[bp],0ffh		; correct calling convention?
	jne	f_driver_info_1		; ne = incorrect, fail

					;For enhanced PD, if they call
	cmp	_BX[bp],offset handles	;with a handle, give them the
					;class they think it is
	jb	default_handle
	cmp	_BX[bp],offset end_handles ;otherwise default to first class
	jae	default_handle
	mov	bx, _BX[bp]
	cmp	[bx].in_use,0		;if it's not in use, it's bad.
	je	default_handle
	mov	al, [bx].class
	mov	_CH[bp], al
	jmp	short got_handle

default_handle:
	mov	al,driver_class
	mov	_CH[bp],al
got_handle:

	mov	_BX[bp],majver		;version
	mov	al,driver_class
	mov	_CH[bp],al
	mov	al,driver_type
	cbw
	mov	_DX[bp],ax
	mov	_CL[bp],0		;number zero.
	mov	_DS[bp],ds		; point to our name in their ds:si
	mov	_SI[bp],offset driver_name
	mov	al,driver_function
	mov	_AL[bp],al
	clc
	ret
f_driver_info_1:
	stc
	ret


f_set_rcv_mode:
	call	verify_handle

	cmp	cx,rcv_mode_num		;are we already using that mode?
	je	f_set_rcv_mode_4	;yes, no need to check anything.

	mov	dx,bx			;remember our handle.
	mov	bx,offset handles	; check that all handles are free
f_set_rcv_mode_2:
	cmp	bx,dx			; is this our handle?
	je	f_set_rcv_mode_3	; yes, of course it's not free.
	cmp	[bx].in_use,0		; is this handle free?
	jne	f_set_rcv_mode_1	; ne = no, can't change
f_set_rcv_mode_3:
	add	bx,(size per_handle)	; next handle
	cmp	bx,offset end_handles	; examined all handles?
	jb	f_set_rcv_mode_2	; b = no, continue examination

	mov	cx,_CX[bp]		;get the desired receive mode.
	cmp	cx,rcv_modes		;do they have this many modes?
	jae	f_set_rcv_mode_1	;no - must be a bad mode for us.
	mov	bx,cx
	add	bx,bx			;we're accessing words, not bytes.
	mov	ax,rcv_modes[bx]+2	;get the handler for this mode.
	or	ax,ax			;do they have one?
	je	f_set_rcv_mode_1	;no - must be a bad mode for us.
	mov	rcv_mode_num,cx		;yes - remember the number and
	call	ax			;  call it.
f_set_rcv_mode_4:
	clc
	ret
f_set_rcv_mode_1:
	mov	dh,BAD_MODE
	stc
	ret


f_get_rcv_mode:
	call	verify_handle
	mov	ax,rcv_mode_num		;return the current receive mode.
	mov	_AX[bp],ax
	clc
	ret


f_set_multicast_list:
;	mov	cx,_CX[bp]		;Tell them how much room they have.

;verify that they supplied an even number of EADDR's.
	mov	ax,cx
	xor	dx,dx
	mov	bx,EADDR_LEN
	div	bx
	or	dx,dx			;zero remainder?
	jne	f_set_multicast_list_2	;no, we don't have an even number of
					;  addresses.

	cmp	ax,MAX_MULTICAST	;is this too many?
	ja	f_set_multicast_list_3	;yes - return NO_SPACE
f_set_multicast_list_1:
	mov	multicast_count,ax	;remember the number of addresses.
	push	cs
	pop	es
	mov	di,offset multicast_addrs
	push	ds
	mov	ds,_ES[bp]		; get ds:si -> new list.
	mov	si,_DI[bp]
	push	cx
	rep	movsb
	pop	cx
	pop	ds

	mov	si,offset multicast_addrs
	call	set_multicast_list
	ret
f_set_multicast_list_2:
	mov	dh,BAD_ADDRESS
	stc
	ret
f_set_multicast_list_3:
	mov	dh,NO_SPACE
	stc
	ret


f_get_multicast_list:
	mov	_ES[bp],ds		;return what we have remembered.
	mov	_DI[bp],offset multicast_addrs
	mov	ax,EADDR_LEN		;multiply the count by the length.
	mul	multicast_count
	mov	_CX[bp],ax		;because they want total bytes.
	clc
	ret


f_get_statistics:
	call	verify_handle		;just in case.
	mov	_DS[bp],ds
	mov	_SI[bp],offset statistics_list
	clc
	ret


access_type_class:
	mov	dh,NO_CLASS
	stc
	ret

access_type_type:
	mov	dh,NO_TYPE
	stc
	ret

access_type_number:
	mov	dh,NO_NUMBER
	stc
	ret

access_type_bad:
	mov	dh,BAD_TYPE
	stc
	ret

;register caller of pkt TYPE
f_access_type:
	mov	bx, offset driver_class
access_type_9:
	mov	al, [bx]		;get the next class.
	inc	bx
	or	al,al			;end of the list?
	je	access_type_class	;class failed (story of my life)
	cmp	_AL[bp],al		;our class?
	jne	access_type_9		;no, try again
access_type_1:
	cmp	_BX[bp],-1		;generic type?
	je	access_type_2		;yes.
	mov	al,driver_type
	cbw
	cmp	_BX[bp],ax		;our type?
	jne	access_type_type	;no.
access_type_2:
	cmp	_DL[bp],0		;generic number?
	je	access_type_3
	cmp	_DL[bp],1		;our number?
	jne	access_type_number
access_type_3:
	cmp	_CX[bp],MAX_P_LEN	;is the type length too long?
	ja	access_type_bad		;yes - can't be ours.

; now we do two things--look for an open handle, and check the existing
; handles to see if they're replicating a packet type.

	mov	free_handle,0		;remember no free handle yet.
	mov	bx,offset handles
access_type_4:
	cmp	[bx].in_use,0		;is this handle in use?
	je	access_type_5		;no - don't check the type.
	mov	al, _AL[bp]		;is this handle the same class as
	cmp	al, [bx].class		;  they're want?
	jne	short access_type_6
	mov	es,_DS[bp]		;get a pointer to their type
	mov	di,_SI[bp]		;  from their ds:si to our es:di
	mov	cx,_CX[bp]		;get the minimum of their length
					;  and our length.  As currently
					;  implemented, only one receiver
					;  gets the packets, so we have to
					;  ensure that the shortest prefix
					;  is unique.
	cmp	cx,[bx].packet_type_len	;Are we less specific than they are?
	jb	access_type_8		;no.
	mov	cx,[bx].packet_type_len	;yes - use their count.
access_type_8:
	lea	si,[bx].packet_type
	or	cx,cx			; pass-all TYPE? (zero TYPE length)
	jne	access_type_7		; ne = no
	mov	bx,offset handles+(MAX_HANDLE-1)*(size per_handle)
	jmp	short access_type_5	; put pass-all last
access_type_7:
	repe	cmpsb
	jne	short access_type_6	;go look at the next one.
access_type_inuse:
	mov	dh,TYPE_INUSE		;a handle has been assigned for TYPE
	stc				;and we can't assign another
	ret
access_type_5:				;handle is not in use
	cmp	free_handle,0		;found a free handle yet?
	jne	access_type_6		;yes.
	mov	free_handle,bx		;remember a free handle
access_type_6:
	add	bx,(size per_handle)	;go to the next handle.
	cmp	bx,offset end_handles	;examined all handles?
	jb	access_type_4		;no, continue.

	mov	bx,free_handle		;did we find a free handle?
	or	bx,bx
	je	access_type_space	;no - return error.

	mov	[bx].in_use,1		;remember that we're using it.

	mov	ax,_DI[bp]		;remember the receiver type.
	mov	[bx].receiver.offs,ax
	mov	ax,_ES[bp]
	mov	[bx].receiver.segm,ax

	push	ds
	mov	ax,ds
	mov	es,ax
	mov	ds,_DS[bp]		;remember their type.
	mov	si,_SI[bp]
	mov	cx,_CX[bp]
	mov	es:[bx].packet_type_len,cx	; remember the TYPE length
	lea	di,[bx].packet_type
	rep	movsb

	lds	si,es:[bx].receiver	;copy the first 8 bytes
	lea	di,[bx].receiver_sig	; to the receiver signature.
	mov	cx,8/2
	rep	movsw

	pop	ds

	mov	al, _AL[bp]
	mov	[bx].class, al

	mov	_AX[bp],bx		;return the handle to them.

	clc
	ret


access_type_space:
	mov	dh,NO_SPACE
	stc
	ret

f_release_type:
	call	verify_handle		;mark this handle as being unused.
	mov	[bx].in_use,0
	clc
	ret


f_send_pkt:
;ds:si -> buffer, cx = length
; XXX Should re-enable interrupts here, but some drivers are broken.
; Possibly re-enable interrupts.
;	test _F[bp], EI		; Were interrupts enabled on pkt driver entry?
;	je	f_send_pkt_1	; No.
;	sti			; Yes, re-enable interrupts now.
;f_send_pkt_1:
	push	ds		; set up proper ds for the buffer
	mov	ds,_DS[bp]	; address of buffer from caller's ds.
	assume	ds:nothing, es:nothing
;following two instructions not needed because si and cx haven't been changed.
;	mov	si,_SI[bp]
;	mov	cx,_CX[bp]	; count of bytes in the packet.
	linc	packets_out
	add	bytes_out.offs,cx	;add up the received bytes.
	adc	bytes_out.segm,0

; If -n option take Ethernet encapsulated Novell IPX packets (from BYU's 
; PDSHELL) and change them to be IEEE 802.3 encapsulated.
EPROT_OFF	equ	EADDR_LEN*2
	test	cs:flagbyte,N_OPTION
	jz	f_send_pkt_2
	cmp	ds:[si].EPROT_OFF,3781h ; if not Novell (prot 8137)
	jne	f_send_pkt_2		;  don't tread on it
	push	ax			; get scratch reg
	mov	ax,[si].EPROT_OFF+4	; get len
	xchg	ah,al
	inc	ax			; make even (rounding up)
	and	al,0feh
	xchg	ah,al
	mov	ds:[si].EPROT_OFF,ax	; save in prot field
	pop	ax			; restore old contents
f_send_pkt_2:
	call	send_pkt
	pop	ds
	assume	ds:code
	ret


f_as_send_pkt:
;es:di -> iocb.
	test	driver_function,4	; is this a high-performance driver?
	je	f_as_send_pkt_2		; no.
; Possibly re-enable interrupts.
	test _F[bp], EI			; Were interrupts enabled on pkt driver entry?
	je	f_as_send_pkt_1		; No.
	sti				; Yes, re-enable interrupts now.
f_as_send_pkt_1:
	push	ds			; set up proper ds for the buffer
	lds	si,es:[di].buffer	; ds:si -> buffer
	assume	ds:nothing
	mov	cx,es:[di].len		; cx = length
	linc	packets_out
	add	bytes_out.offs,cx	; add up the received bytes.
	adc	bytes_out.segm,0

;ds:si -> buffer, cx = length, es:di -> iocb.
	call	as_send_pkt
	pop	ds
	assume	ds:code
	ret
f_as_send_pkt_2:
	mov dh,	BAD_COMMAND		; return an error.
	stc
	ret


f_drop_pkt:
; es:di -> iocb.
	test	driver_function,4	; is this a high-performance driver?
	je	f_as_send_pkt_1		; no.
	push	ds			; Preserve ds
	mov	si,offset send_head	; Get head offset
dp_loop:
	mov	ax,ds:[si]		; Get offset
	mov	dx,ds:[si+2]		; Get segment
	mov	bx,ax
	or	bx,dx			; End of list?
	je	dp_endlist		; Yes
	cmp	ax,di			; Offsets equal?
	jne	dp_getnext		; No
	mov	bx,es
	cmp	dx,bx			; Segments equal?
	jne	dp_getnext		; No
	call	drop_pkt		; Pass to driver
	les	di,es:[di].next		; Get next segment:offset
	mov	ds:[si],di		; Set next offset
	mov	ds:[si+2],es		; Set next segment
	pop	ds			; Restore ds
	clc
	ret
dp_getnext:
	mov	ds,dx			; Get next segment
	mov	si,ax			; Get next iocb offset
	lea	si,ds:[si].next		; Get next iocb next ptr offset
	jmp	dp_loop			; Try again
dp_endlist:
	pop	ds			; Restore ds
	mov	dh,BAD_IOCB		; Return error
	stc				; Set carry
	ret


f_terminate:
	call	verify_handle		; must have a handle

f_terminate_1:
	mov	[bx].in_use,0		; mark handle as free
	mov	bx,offset handles	; check that all handles are free
f_terminate_2:
	cmp	[bx].in_use,0		; is this handle free?
	jne	f_terminate_4		; ne = no, so can't exit completely
	add	bx,(size per_handle)	; next handle
	cmp	bx,offset end_handles	; examined all handles?
	jb	f_terminate_2		; b = no, continue examination

;
; Now disable interrupts
;
	mov	al,int_no
	or	al,al			;are they using a hardware interrupt?
	je	f_terminate_no_irq	;no.
	call	maskint

;
; Now return the interrupt to their handler.
;
	mov	ah,25h			;get the old interrupt into es:bx
	mov	al,int_no
	add	al,8
	cmp	al,8+8			;is it a slave 8259 interrupt?
	jb	f_terminate_3		;no.
	add	al,70h - (8+8)		;map it to the real interrupt.
f_terminate_3:
	push	ds
	lds	dx,their_recv_isr
	int	21h
	pop	ds

f_terminate_no_irq:
	call	terminate		;terminate the hardware.

	mov	al,packet_int_no	;release our_isr.
	mov	ah,25h
	push	ds
	lds	dx,their_isr
	int	21h
	pop	ds

;
; Now free our memory
;
	push	cs
	pop	es
	mov	ah,49h
	int	21h
	clc
	ret
f_terminate_4:
	mov	dh, CANT_TERMINATE
	stc
	ret



f_get_address:
;	call	verify_handle
;	mov	es,_ES[bp]		; get new one
;	mov	di,_DI[bp]		; get pointer, es:di is ready
	mov	cx,_CX[bp]		;Tell them how much room they have.
	cmp	have_my_address,0	;has our address been set?
	jne	get_address_set		;yes - go report it.
	call	get_address		;no, can we get the address?
	jc	get_address_space	;no - we must not have enough space.
	mov	_CX[bp],cx		;Tell them how long our address is.
	clc
	ret
get_address_set:
	cmp	cx,my_address_len	;is there enough room?
	jb	get_address_space	;no.
	mov	cx,my_address_len	;yes - get our address length.
	mov	_CX[bp],cx		;Tell them how long our address is.
	mov	si,offset my_address	;copy it into their area.
	rep	movsb
	clc
	ret

get_address_space:
	mov	dh,NO_SPACE
	stc
	ret


f_set_address:
	mov	bx,offset handles
	mov	cl,0			;number of handles in use.
f_set_address_1:
	add	cl,[bx].in_use		;is this handle in use?
	add	bx,(size per_handle)	;go to the next handle.
	cmp	bx,offset end_handles
	jb	f_set_address_1

	cmp	cl,1			;more than one handle in use?
	ja	f_set_address_inuse	;yes - we can't set the address

	mov	ds,_ES[bp]		; set new one
	assume	ds:nothing
	mov	si,_DI[bp]		; set pointer, ds:si is ready
	mov	cx,_CX[bp]		;Tell them how much address is being set.
	call	set_address
;set_address restores ds.
	jc	f_set_address_exit	;Did it work?
	mov	_CX[bp],cx		;yes - return our address length.

	cmp	cx,MAX_ADDR_LEN		;is it too long for us to remember?
	ja	f_set_address_too_long	;yes, return a too-long error.

	mov	ds,_ES[bp]		; set new one
	mov	si,_DI[bp]		; set pointer, ds:si is ready
	mov	ax,cs
	mov	es,ax
	mov	my_address_len,cx	;remember how long our address is.
	mov	di,offset my_address
	rep	movsb
	mov	have_my_address,1
	mov	ds,ax			;restoer ds.
	assume	ds:code
	clc
	ret
f_set_address_inuse:
	mov	dh,CANT_SET
	stc
	ret
f_set_address_too_long:
	mov	dh,NO_SPACE
	stc
f_set_address_exit:
	ret


f_reset_interface:
	call	verify_handle
	call	reset_interface
	clc
	ret


; Stop the packet driver doing upcalls. Also a following terminate will
; always succed (no in use handles any longer).
f_stop:
	mov	bx,offset handles
f_stop_2:
	mov	[bx].in_use,0
	add	bx,(size per_handle)	; next handle
	cmp	bx,offset end_handles
	jb	f_stop_2
	clc
	ret


f_get_parameters:
;strictly speaking, this function only works for high-performance drivers.
	test	driver_function,4	;is this a high-performance driver?
	jne	f_get_parameters_1	;yes.
	mov	dh,BAD_COMMAND		;no - return an error.
	stc
	ret
f_get_parameters_1:
	mov	_ES[bp],cs
	mov	_DI[bp],offset parameter_list
	clc
	ret


verify_handle:
;Ensure that their handle is real.  If it isn't, we pop off our return
;address, and return to *their* return address with cy set.
	mov	bx,_BX[bp]		;get the handle they gave us
	cmp	bx,offset handles
	jb	verify_handle_bad	;no - must be bad.
	cmp	bx,offset end_handles
	jae	verify_handle_bad	;no - must be bad.
	cmp	[bx].in_use,0		;if it's not in use, it's bad.
	je	verify_handle_bad
	ret
verify_handle_bad:
	mov	dh,BAD_HANDLE
	add	sp,2			;pop off our return address.
	stc
	ret


	public	set_recv_isr
set_recv_isr:
	mov	ah,35h			;get the old interrupt into es:bx
	mov	al,int_no		; board's interrupt vector
	or	al,al
	je	set_isr_no_irq
	add	al,8
	cmp	al,8+8			;is it a slave 8259 interrupt?
	jb	set_recv_isr_1		;no.
	add	al,70h - 8 - 8		;map it to the real interrupt.
set_recv_isr_1:
	int	21h
	mov	their_recv_isr.offs,bx	;remember the old seg:off.
	mov	their_recv_isr.segm,es

	mov	ah,25h			;now set our recv interrupt.
	mov	dx,offset recv_isr
	int	21h

	mov	al,int_no		; Now enable interrupts
	call	unmaskint

set_isr_no_irq:
	ret

	public	count_in_err
count_in_err:
	assume	ds:nothing
	linc	errors_in
	ret

	public	count_out_err
count_out_err:
	assume	ds:nothing
	linc	errors_out
	ret

their_recv_isr	dd	0		; original owner of board int

recv_isr:
; In order to achieve back-to-back packet transmissions, we handle the
; latency-critical portion of transmit interrupts first.  The xmit
; interrupt routine should only start the next transmission, but do
; no other work.  It may only touch ax and dx (the only register necessary
; for doing "out" instructions) unless it first pushes any other registers
; itself.
	push	ax
	push	dx
	call	xmit

; Now switch stacks, push remaining registers, and do remaining interrupt work.
	push	ds
	mov	ax,cs			;ds = cs.
	mov	ds,ax
	assume	ds:code

	mov	savesp,sp
	mov	savess,ss

	mov	ss,ax
	mov	sp,offset our_stack
	cld

	push	bx
	push	cx
	push	si
	push	di
	push	bp
	push	es

; The following comment is wrong in that we now do a specific EOI command,
; and because we don't enable interrupts (even though we should).

; Chips & Technologies 8259 clone chip seems to be very broken.  If you
; send it a Non Specific EOI command, it clears all In Service Register
; bits instead of just the one with the highest priority (as the Intel
; chip does and clones should do).  This bug causes our interrupt
; routine to be reentered if: 1. we reenable processor interrupts;
; 2. we reenable device interrupts; 3. a timer or other higher priority
; device interrupt now comes in; 4. the new interrupting device uses
; a Non Specific EOI; 5. our device interrupts again.  Because of
; this bug, we now completely mask our interrupts around the call
; to "recv", the real device interrupt handler.  This allows us
; to send an EOI instruction to the 8259 early, before we actually
; reenable device interrupts.  Since the interrupt is masked, we
; are still guaranteed not to get another interrupt from our device
; until the interrupt handler returns.  This has another benefit:
; we now no longer prevent other devices from interrupting while our
; interrupt handler is running.  This is especially useful if we have
; other (multiple) packet drivers trying to do low-latency transmits.
	mov	al,int_no	; Disable further device interrupts
	call	maskint

; The following is from Bill Rust, <wjr@ftp.com>
; this code dismisses the interrupt at the 8259. if the interrupt number
;  is > 8 then it requires fondling two PICs instead of just one.
	mov	al, int_no	; get hardware int #
	cmp	al, 8		; see if its on secondary PIC
	jg	recv_isr_4
	add	al, 60h		; make specific EOI dismissal
	out	20h, al
	jmp	recv_isr_3	; all done
recv_isr_4:
	add	al,60h - 8	; make specific EOI (# between 9 & 15).
	out	0a0h,al		; Secondary 8259 (PC/AT only)
	mov	al,62h		; Acknowledge on primary 8259.
	out	20h,al
recv_isr_3:

;	sti				; Interrupts are now completely safe
	call	recv

	cli				;interrupts *must* be off between
					;here and the stack restore, because
					;if we have one of our interrupts
					;pending, we would trash our stack.
	mov	al,int_no	; Now reenable device interrupts
	call	unmaskint

	pop	es
	pop	bp
	pop	di
	pop	si
	pop	cx
	pop	bx

	mov	ss,savess
	mov	sp,savesp

	call	recv_exiting		;this routine can enable interrupts.
; DDP - This is a BIG mistake.  This routine SHOULD NOT enable interrupts.
;	doing so can cause interrupt recursion and blow your stack.
;	Processor interrupts SHOULD NOT be enabled after enabling device
;	interrupts until after the "iret".  You will lose atleast 12 bytes
;	on the stack for each recursion.

	pop	ds
	assume	ds:nothing
	pop	dx
	pop	ax
	iret


	public	maskint
maskint:
	or	al,al			;are they using a hardware interrupt?
	je	maskint_1		;no, don't mask off the timer!

	assume	ds:code
	mov	dx,21h			;assume the master 8259.
	cmp	al,8			;using the slave 8259 on an AT?
	jb	mask_not_irq2
	mov	dx,0a1h			;go disable it on slave 8259
	sub	al,8
mask_not_irq2:
	mov	cl,al

	in	al,dx			;disable them on the correct 8259.
	mov	ah,1			;set the bit.
	shl	ah,cl
	or	al,ah
	out	dx,al
maskint_1:
	ret


	public	unmaskint
unmaskint:
	assume	ds:code
	mov	dx,21h			;assume the master 8259.
	mov	cl,al
	cmp	cl,8			;using the slave 8259 on an AT?
	jb	unmask_not_irq2		;no
	in	al,dx			;get master mask
	and	al,not (1 shl 2)	; and clear slave cascade bit in mask
	out	dx,al			;set new master mask (enable slave int)
	mov	dx,0a1h			;go enable int on slave 8259
	sub	cl,8
unmask_not_irq2:

	in	al,dx			;enable interrupts on the correct 8259.
	mov	ah,1			;clear the bit.
	shl	ah,cl
	not	ah
	and	al,ah
	out	dx,al

	ret


	public	recv_find
recv_find:
;called when we want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type, dl = packet class.
;exit with es:di = 0 if the packet is not desired, or es:di -> packet buffer
;  to be filled by the driver.
	assume	ds:code, es:nothing
	push	cx

; If -n option take IEEE 802.3 encapsulated packets that could be Novell IPX 
; and make them Ethernet encapsulated Novell IPX packets (for PDSHELL).
	test	flagbyte,N_OPTION
	jz	not_n_op

; Make IEEE 802.3-like packets that could be Novell IPX into BlueBook class
; Novell type 8137 packets.
	cmp	dl,IEEE8023		;Is this an IEEE 802.3 packet?
	jne	recv_not_802_3		;no
	cmp	word ptr es:[di],0ffffh	;if this word not ffff
	jne	recv_not_8137		;  then not Novell
	sub	di,2			; back it up to the 8137 word.
	mov	es:[di],3781h		; fake as Novell protocol (8137)
	mov	dl,BLUEBOOK
	jmp	short recv_not_8137
recv_not_802_3:
; Convert incoming Ethernet type 8137 IPX packets to type 8138, as with -n in 
; effect we can't send type 8137, and it will only confuse Netware.
	cmp	dl,BLUEBOOK		;Is this a BLUEBOOK packet?
	jne	recv_not_8137		;no, don't change it.
	cmp	word ptr es:[di],3781h	;Is it an 8137 packet?
	jne	recv_not_8137		;no, don't change it.
	mov	es:[di],word ptr 3881h	;yes, mung it slightly.
recv_not_8137:
not_n_op:

	mov	bx,offset handles
recv_find_1:
	cmp	[bx].in_use,0		;is this handle in use?
	je	recv_find_2		;no - don't check the type.

	mov	ax,[bx].receiver.offs	;do they have a receiver?
	or	ax,[bx].receiver.segm
	je	recv_find_2		;no - they're not serious about it.

	mov	cx,[bx].packet_type_len	;compare the packets.
	lea	si,[bx].packet_type
	jcxz	recv_find_3		;if cx is zero, they want them all.

	cmp	[bx].class, dl		;is this the right class?
	jne	recv_find_2		;no- don't bother

	push	di
	repe	cmpsb
	pop	di
	je	recv_find_3		;we've got it!
recv_find_2:
	add	bx,(size per_handle)	;go to the next handle.
	cmp	bx,offset end_handles
	jb	recv_find_1

	linc	packets_dropped

	pop	cx			;we didn't find it -- discard it.
recv_find_5:
	xor	di,di			;"return" a null pointer.
	mov	es,di
	ret
recv_find_3:
	pop	cx			; the packet_length

	linc	packets_in
	add	bytes_in.offs,cx	;add up the received bytes.
	adc	bytes_in.segm,0

	les	di,[bx].receiver	;remember the receiver upcall.
	mov	receive_ptr.offs,di
	mov	receive_ptr.segm,es

	test	flagbyte,W_OPTION	;did they select the Windows option?
	je	recv_find_6		;no, don't check for the upcall.

; does the receiver signature match whats currently in memory?  if not,
; jump to fake return
	push	si
	push	cx
	lea	si,[bx].receiver_sig
	mov	cx,8/2
	repe	cmpsw
	pop	cx
	pop	si
	jne	recv_find_5
recv_find_6:

	mov	found_handle,bx		;remember what our handle was.
	mov	ax,0			;allocate request.
	stc				;with stc, flags must be an odd number
	push	ax			; save a number that cant be flags
	pushf				;save flags in case iret used.
	call	receive_ptr		;ask the client for a buffer.
	; on return, flags should be at top of stack. if an IRET has been used,
	; then 0 will be at the top of the stack
	pop	bx
	cmp	bx,0
	je	recv_find_4		;0 is at top of stack
	add	sp,2
recv_find_4:
	ret


	public	recv_copy
recv_copy:
;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
;preserve bx.
	assume	ds:nothing, es:nothing

	push	bx
	mov	bx,found_handle
	mov	ax,1			;store request.
	clc				;with clc, flags must be an even number
	push	ax			; save a number that can't be flags
	pushf				;save flags incase iret used.
	call	receive_ptr		;ask the client for a buffer.
	pop	bx
	cmp	bx,1			;if this is a 1, IRET was used.
	je	recv_copy_1
	pop	bx
recv_copy_1:
	pop	bx
	ret

	public	send_queue
send_queue:
; Queue an iocb.
; Enter with es:di -> iocb, interrupts disabled.
; Destroys ds:si.
	assume	ds:nothing, es:nothing
	mov	es:[di].next.offs,0	; Zero next offset
	mov	es:[di].next.segm,0	; Zero next segment
	mov	si,send_head.offs	; Queue empty?
	or	si,send_head.segm
	jnz	sq_notempty		; No
	mov	send_head.offs,di	; Set head offset
	mov	send_head.segm,es	; Set head segment
	jmp	sq_settail
sq_notempty:				; Queue is not empty
	lds	si,send_tail		; Get tail segment:offset
	mov	ds:[si].next.offs,di	; Set next offset
	mov	ds:[si].next.segm,es	; Set next segment
sq_settail:
	mov	send_tail.offs,di	; Set tail offset
	mov	send_tail.segm,es	; Set tail segment
	ret


	public	send_dequeue
send_dequeue:
; Dequeue an iocb and possibly call its upcall.
; Enter with device or processor interrupts disabled, ah = return code.
; Exits with es:di -> iocb; destroys ds:si, ax, bx, cx, dx, bp.
	assume	ds:nothing, es:nothing
	les	di,send_head		; Get head segment:offset
	lds	si,es:[di].next		; Get next segment:offset
	mov	send_head.offs, si	; Set head offset
	mov	send_head.segm, ds	; Set head segment
	or	es:flags[di], DONE	; Mark done
	mov	es:ret_code[di], ah	; Set retcode
	test	es:[di].flags,CALLME	; Does he want an upcall?
	je	send_dequeue_1		; No.
	push	es			; Push iocb segment
	push	di			;  and offset
	clc				; Clear carry.
	mov	ax,1			; Push a number that cant be flags.
	push	ax
	pushf				; Save flags in case iret used.
	call	es:[di].upcall		; Call the client.
	pop	ax			; Pop first word.
	cmp	ax,1			; If this is a 1, IRET was used.
	je	send_dequeue_2		; Far return used.
	add	sp,2			; Pop flags.
send_dequeue_2:
	pop	di			; Pop iocb segment
	pop	es			;  and offset
send_dequeue_1:
	ret


code	ends

	end	start
