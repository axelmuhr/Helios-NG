version	equ	1

;  Russell Nelson, Clarkson University.  September 14, 1989
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
 db "Packet address version ",'0'+majver,".",'0'+version," copyright 1990, Russell Nelson.",CR,LF
 db "This program is free software; see the file COPYING for details.",CR,LF
 db "NO WARRANTY; see the file COPYING for details.",CR,LF
crlf_msg	db	CR,LF,'$'

int_pkt	macro
	pushf
	cli
	call	their_isr
	endm

their_isr	dd	?
packet_int_no	db	?,?,?,?
ether_bdcst	db	EADDR_LEN dup(-1)	;ethernet broadcast address.
ether_addr	db	EADDR_LEN dup(-1)

handle		dw	?

bogus_type	db	0,0		;totally bogus type code.

signature	db	'PKT DRVR',0
signature_len	equ	$-signature
no_signature_msg	db	"No packet driver at that address",'$'
usage_msg	db	"usage: pktaddr <packet_int_no> [<addr>]",'$'

eaddr_msg	db	"My Ethernet address is ",'$'

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

	mov	di,offset packet_int_no
	call	get_number

	mov	di,offset ether_addr
	push	ds
	pop	es
	call	get_eaddr

	mov	ah,35h			;get their packet interrupt.
	mov	al,packet_int_no
	int	21h
	mov	their_isr.offs,bx
	mov	their_isr.segm,es

	lea	di,3[bx]
	mov	si,offset signature
	mov	cx,signature_len
	repe	cmpsb
	je	have_signature
	mov	dx,offset no_signature_msg
	jmp	error
have_signature:

	push	ds
	pop	es
	mov	cx,EADDR_LEN
	mov	si,offset ether_addr
	mov	di,offset ether_bdcst
	repe	cmpsb
	je	get_mode		;no address specified.

	mov	ah,25			;set the ethernet address.
	mov	di,offset ether_addr
	mov	cx,EADDR_LEN
	int_pkt
	call	fatal_error
	jmp	okay
get_mode:
	mov	ah,2			;access all packets.
	mov	al,1			;Ethernet class.
	mov	bx,-1			;generic type.
	mov	dl,0			;generic number.
	mov	cx,2			;use a type length of 2
	mov	si,offset bogus_type
	push	cs			;es:di -> our receiver.
	pop	es
	mov	di,offset our_recv
	int_pkt
	call	fatal_error
	mov	handle,ax

	mov	ah,6			;get the ethernet address.
	mov	di,offset ether_addr
	mov	cx,EADDR_LEN
	mov	bx,handle
	int_pkt
	jc	bad

	mov	dx,offset eaddr_msg
	mov	ah,9
	int	21h

	mov	si,offset ether_addr
	call	print_ether_addr

	mov	dx,offset crlf_msg	;can't depend on DOS to newline for us.
	mov	ah,9
	int	21h
	jmp	short now_close
bad:
	call	print_error
now_close:
	mov	ah,3			;release_type
	mov	bx,handle
	int_pkt
	call	fatal_error

okay:
	int	20h


our_recv:
	or	ax,ax			;first or second call?
	jne	our_recv_1		;second -- we ignore the packet
	push	cs
	pop	es
	mov	di,offset our_buffer
our_recv_1:
	db	0cbh			;masm 4.0 doesn't grok "retf"


	include	printea.asm

	assume	ds:code

	include	pkterr.asm
	include	getea.asm
	include	getnum.asm
	include	skipblk.asm
	include	getdig.asm
	include	digout.asm
	include	chrout.asm

our_buffer	label	byte

code	ends

	end	start
