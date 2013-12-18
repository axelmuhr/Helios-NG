version	equ	0
;History:232,1

;  Russell Nelson, Clarkson University.  December 24, 1989
;  Copyright, 1989, Russell Nelson

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

	org	80h
phd_dioa	label	byte

	org	100h
start:
	jmp	start_1

copyleft_msg	label	byte
 db "Packet ringer version ",'0'+majver,".",'0'+version," copyright 1990, Russell Nelson.",CR,LF
 db "This program is free software; see the file COPYING for details.",CR,LF
 db "NO WARRANTY; see the file COPYING for details.",CR,LF
crlf_msg	db	CR,LF,'$'

int_pkt	macro	num
	pushf
	cli
	call	their_isr_&num
	endm

their_isr_1	dd	?
their_isr_2	dd	?
packet_int_1	db	?,?,?,?
packet_int_2	db	?,?,?,?
send_count	dw	RUNT
handle		dw	?
transmit_count	dw	?
receive_count	dw	?
packet_mode	dw	?
this_mode	dw	?

signature	db	'PKT DRVR',0
signature_len	equ	$-signature

no_signature_msg	db	"No packet driver at that address",'$'
usage_msg	db	"usage: pktring [-m] <packet_int_1> <packet_int_2>",'$'
sending_msg	db	"Sending from ",'$'
to_msg		db	" to ",'$'

not_implemented	db	"xx ",'$'
current_mode	db	"-> ",'$'
two_spaces	db	"   ",'$'

test_modes	db	?		;nonzero if we should test the modes.

mode_names	dw	mode_1_msg,mode_2_msg,mode_3_msg,mode_4_msg
		dw	mode_5_msg,mode_6_msg

mode_1_msg	db	"1) Turn off receiver",'$'
mode_2_msg	db	"2) Receive only packets sent to this interface",'$'
mode_3_msg	db	"3) Mode 2 plus broadcast",'$'
mode_4_msg	db	"4) Mode 3 plus limited multicast",'$'
mode_5_msg	db	"5) Mode 3 plus all multicast",'$'
mode_6_msg	db	"6) All packets",'$'

mode_code	dw	mode_1_tester,mode_2_tester,mode_3_tester,mode_4_tester
		dw	mode_5_tester,mode_6_tester

mode_good_msg	db	" passed",CR,LF,'$'
mode_bad_msg	db	" FAILED!",7,CR,LF,'$'

usage_error:
	mov	dx,offset usage_msg
error:
	mov	ah,9
	int	21h
	int	20h

start_1:
	cld

	mov	dx,offset copyleft_msg
	mov	ah,9
	int	21h

	mov	si,offset phd_dioa+1
	cmp	byte ptr [si],CR	;end of line?
	je	usage_error

another_switch:
	call	skip_blanks

	cmp	al,'-'			;did they specify a switch?
	jne	not_switch
	cmp	byte ptr [si+1],'m'	;did they specify '-m'?
	je	got_mode_switch
	jmp	usage_error		;no, must be an error.
got_mode_switch:
	mov	test_modes,1
	add	si,2
	jmp	another_switch

not_switch:
	mov	di,offset packet_int_1
	call	get_number
	mov	di,offset packet_int_2
	call	get_number

	call	skip_blanks		;check for the end of the line.
	cmp	al,CR
	jne	usage_error

	mov	sp,offset start		;now that we're finished with
					;the parameters, put our stack there.

	mov	ah,35h			;get their packet interrupt.
	mov	al,packet_int_1
	int	21h
	mov	their_isr_1.offs,bx
	mov	their_isr_1.segm,es

	lea	di,3[bx]
	mov	si,offset signature
	mov	cx,signature_len
	repe	cmpsb
	je	signature_1_ok
	jmp	no_signature_err
signature_1_ok:

	mov	ah,35h			;get their packet interrupt.
	mov	al,packet_int_2
	int	21h
	mov	their_isr_2.offs,bx
	mov	their_isr_2.segm,es

	lea	di,3[bx]
	mov	si,offset signature
	mov	cx,signature_len
	repe	cmpsb
	je	signature_2_ok
	jmp	no_signature_err
signature_2_ok:

	mov	dx,offset sending_msg
	mov	ah,9
	int	21h

	push	ds
	mov	ax,1ffh			;driver_info
	int_pkt	2
	jc	saying_2_1
	pushf
saying_2:
	lodsb
	call	chrout
	cmp	byte ptr ds:[si],0
	jne	saying_2

	popf
saying_2_1:
	pop	ds
	call	fatal_error

	mov	dx,offset to_msg
	mov	ah,9
	int	21h

	push	ds
	mov	ax,1ffh			;driver_info
	int_pkt	1
	jc	saying_1_1
	pushf
	push	ax
saying_1:
	lodsb
	call	chrout
	cmp	byte ptr ds:[si],0
	jne	saying_1

	pop	ax
	popf
saying_1_1:
	pop	ds
	call	fatal_error

	push	ax
	push	dx
	mov	dx,offset crlf_msg
	mov	ah,9
	int	21h
	pop	dx
	pop	ax

	mov	ah,2			;access all packets.
	mov	al,ch			;their class from driver_info().
	mov	bx,dx			;their type from driver_info().
	mov	dl,cl			;their number from driver_info().
	mov	cx,0			;type length of zero.
	push	cs			;es:di -> our receiver.
	pop	es
	mov	di,offset our_recv
	int_pkt	1
	call	fatal_error
	mov	handle,ax

	cmp	test_modes,0		;should we test the receive modes?
	je	get_their_address	;no.

	mov	ah,21			;get the receive mode.
	mov	bx,handle
	int_pkt	1
	jc	bad_1
	mov	packet_mode,ax

	mov	this_mode,1		;start trying with mode 1.
try_mode:
	cmp	this_mode,4		;skip modes 4 and 5 (don't work yet).
	jne	try_mode_1
	mov	this_mode,6
try_mode_1:
	cmp	this_mode,6		;have we hit the last mode?
	ja	no_more_modes		;yes.

	mov	ah,20			;set the receive mode.
	mov	bx,handle
	mov	cx,this_mode
	int_pkt	1
	jc	tried_mode		;we tried it, and it didn't work.

	mov	bx,this_mode		;print the name of this mode.
	shl	bx,1
	mov	ah,9
	mov	dx,mode_names[bx-2]
	int	21h

	mov	bx,this_mode		;do the test for this mode.
	shl	bx,1
	call	mode_code[bx-2]
	mov	dx,offset mode_good_msg	;report success.
	jnc	mode_good
	mov	dx,offset mode_bad_msg	;report failure.
mode_good:
	mov	ah,9
	int	21h

tried_mode:

	inc	this_mode		;try the next mode.
	jmp	try_mode

no_more_modes:

	mov	ah,20			;set the receive mode.
	mov	bx,handle
	mov	cx,packet_mode
	int_pkt	1
bad_1:
	jnc	get_their_address
	jmp	bad

get_their_address:

	mov	ah,6			;get the destination's address.
	mov	bx,handle
	mov	cx,EADDR_LEN
	push	cs			;es:di -> to address.
	pop	es
	mov	di,offset our_buffer
	int_pkt	1
	jc	bad_1

wait_for_key:
	inc	send_count
	cmp	send_count,GIANT	;too big?
	jbe	length_ok
	mov	transmit_count,0	;zero the counts.
	mov	receive_count,0
	mov	send_count,RUNT		;yes, reset it back down again.
length_ok:

	mov	cx,send_count
	call	send_packet
	jc	bad

	inc	transmit_count

	mov	ax,transmit_count
	call	wordout

	mov	al,' '
	call	chrout

	mov	ax,receive_count
	call	wordout

	mov	al,CR
	call	chrout

	mov	ah,1			;check for any key.
	int	16h
	je	wait_for_key		;no key -- keep waiting.

	mov	ah,0			;read a key.
	int	16h

	mov	ah,3			;release the handle.
	mov	bx,handle
	int_pkt	1
	jc	bad

	int	20h

all_done:
	xor	bx,bx			;only release the handle once.
	xchg	bx,handle
	or	bx,bx
	je	all_done_1		;we've already released it.
	mov	ah,3			;release_type
	int_pkt	1
	jc	bad
all_done_1:
	int	20h

bad:
	call	print_error
okay:
	jmp	all_done


no_signature_err:
	mov	dx,offset no_signature_msg
	mov	ah,9
	int	21h
	int	20h


	assume	ds:nothing
our_recv:
	or	ax,ax			;first or second call?
	jne	our_recv_1		;second -- we ignore the packet
	push	cs
	pop	es
	mov	di,offset our_buffer + GIANT
	db	0cbh			;masm 4.0 doesn't grok "retf"
our_recv_1:
	add	si,EADDR_LEN+EADDR_LEN
	lodsw
	mov	cx,ax
	sub	cx,EADDR_LEN+EADDR_LEN+2
our_recv_2:
	lodsb
	cmp	al,cl
	loope	our_recv_2
	jne	our_recv_3		;if they don't match, don't increment the count.
	inc	receive_count
our_recv_3:
	db	0cbh			;masm 4.0 doesn't grok "retf"

;two macros to return an error code.

rz	macro
	stc
	jne	$+3
	ret
	endm

rnz	macro
	stc
	je	$+3
	ret
	endm

mode_1_tester:
	call	send_bcast		;don't receive broadcast,
	rnz
	call	send_to_them		;don't receive their address,
	rnz
	call	send_random		;don't receive other addresses,
	rnz
	call	send_multi		;don't receive multicasts.
	rnz
	clc
	ret
mode_2_tester:
	call	send_bcast		;don't receive broadcast,
	rnz
	call	send_random		;don't receive other addresses,
	rnz
	call	send_multi		;don't receive multicasts.
	rnz
	call	send_to_them		;do    receive their address,
	rz
	clc
	ret
mode_3_tester:
	call	send_bcast		;do    receive broadcast,
	rz
	call	send_random		;don't receive other addresses,
	rnz
	call	send_multi		;don't receive multicasts.
	rnz
	call	send_to_them		;do    receive their address,
	rz
	clc
	ret
mode_4_tester:
;;; set up 01:01:01:01:01:01 in the multicast list.
	call	send_bcast		;do    receive broadcast,
	rz
	call	send_random		;don't receive other addresses,
	rnz
	call	send_multi		;do    receive multicasts.
	rz
	call	send_to_them		;do    receive their address,
	rz
	clc
	ret
mode_5_tester:
;;; set up an empty mullticast list.
	call	send_bcast		;do    receive broadcast,
	rz
	call	send_random		;don't receive other addresses,
	rnz
	call	send_multi		;do    receive multicasts.
	rz
	call	send_to_them		;do    receive their address,
	rz
	clc
	ret
mode_6_tester:
	call	send_bcast		;do    receive broadcast,
	rz
	call	send_random		;do    receive other addresses,
	rz
	call	send_multi		;do    receive multicasts.
	rz
	call	send_to_them		;do    receive their address,
	rz
	clc
	ret

send_bcast:
	mov	dl,'B'
	mov	ah,6
	int	21h
	mov	al,0ffh			;all-ones is broadcast.
	jmp	short send_fixed_addr
send_random:
	mov	dl,'R'
	mov	ah,6
	int	21h
	mov	al,002h			;all twos is an okay address.
	jmp	short send_fixed_addr
send_multi:
	mov	dl,'M'
	mov	ah,6
	int	21h
	mov	al,001h			;first bit set is multicast.
send_fixed_addr:
	push	cs
	pop	es
	mov	di,offset our_buffer
	mov	cx,EADDR_LEN
	rep	stosb
	jmp	short send_to_addr
send_to_them:
	mov	dl,'T'
	mov	ah,6
	int	21h

	mov	ah,6			;get the destination's address.
	mov	bx,handle
	mov	cx,EADDR_LEN
	push	cs			;es:di -> to address.
	pop	es
	mov	di,offset our_buffer
	int_pkt	1
	jnc	send_to_addr
send_to_addr_bad:
	jmp	bad
send_to_addr:
;send the packet in our_buffer.
;return zr if we didn't receive it, and ne if we did.

;we'll know we got a packet if the receive count gets bumped.
	mov	receive_count,0

	mov	cx,100
	call	send_packet
	jc	send_to_addr_3

	mov	ax,2
	call	set_timeout
send_to_addr_1:
	cmp	receive_count,0		;did we receive it?
	jne	send_to_addr_2
	call	do_timeout
	jne	send_to_addr_1
	mov	dl,'0'
	mov	ah,6
	int	21h
	xor	al,al
	ret
send_to_addr_2:
	mov	dl,'1'
	mov	ah,6
	int	21h
	or	sp,sp
	ret
send_to_addr_3:
	xor	al,al
	ret


send_packet:
;enter with cx = length of packet.
;stuff the standard contents into our_buffer.
;return the results of sending the packet.
	push	cx

	push	cs			;es:di -> our buffer.
	pop	es
	mov	di,offset our_buffer+EADDR_LEN+EADDR_LEN
	mov	ax,cx
	stosw
	sub	cx,EADDR_LEN+EADDR_LEN+2
send_packet_1:
	mov	al,cl
	stosb
	loop	send_packet_1

	pop	cx

	mov	ah,4			;send_pkt
	mov	si,offset our_buffer	;ds:si -> buffer.
	int_pkt	2
	ret


	include	timeout.asm
	include	pkterr.asm
	include	getnum.asm
	include	getdig.asm
	include	skipblk.asm
	include	chrout.asm
	include	digout.asm

our_buffer	label	byte

code	ends

	end	start
