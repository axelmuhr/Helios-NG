version	equ	0

;  Russell Nelson, Clarkson University.  October 20, 1988
;  Copyright, 1988, 1989, Russell Nelson

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
 db "Packet receiver version ",'0'+majver,".",'0'+version," copyright 1990, Russell Nelson.",CR,LF
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
handle		dw	?
packet_flag	dw	0
first_count	dw	?
second_count	dw	?
signature	db	'PKT DRVR',0
signature_len	equ	$-signature
no_signature_msg	db	"No packet driver at that address",'$'
usage_msg	db	"usage: pktall <packet_int_no>",'$'
waiting_msg	label	byte
	db	"Now waiting for packets to be received.  A dot will be printed when one is",CR,LF
	db	"received.  Press any key to exit.",CR,LF,'$'
counts_bad_msg	db	"First and second counts didn't match",CR,LF,'$'
not_first_msg	db	"Driver maybe wrote too little",CR,LF,'$'
not_second_msg	db	"Driver wrote too much",CR,LF,'$'


usage_error:
	mov	dx,offset usage_msg
error:
	mov	ah,9
	int	21h
	int	20h

start_1:
	mov	dx,offset copyleft_msg
	mov	ah,9
	int	21h

	mov	si,offset phd_dioa+1
	cmp	byte ptr [si],CR	;end of line?
	je	usage_error

	mov	di,offset packet_int_no
	call	get_number

	mov	ah,35h			;get their packet interrupt.
	mov	al,packet_int_no
	int	21h
	mov	their_isr.offs,bx
	mov	their_isr.segm,es

	lea	di,3[bx]
	mov	si,offset signature
	mov	cx,signature_len
	repe	cmpsb
	mov	dx,offset no_signature_msg
	jne	error

	push	ds
	mov	ax,1ffh			;driver_info
	int_pkt
	pop	ds
	call	fatal_error

	mov	ah,2			;access all packets.
	mov	al,ch			;their class from driver_info().
	mov	bx,dx			;their type from driver_info().
	mov	dl,cl			;their number from driver_info().
	mov	cx,0			;type length of zero.
	push	cs			;es:di -> our receiver.
	pop	es
	mov	di,offset our_recv
	int_pkt
	call	fatal_error
	mov	handle,ax

	mov	dx,offset waiting_msg
	mov	ah,9
	int	21h

wait_for_key:
	cmp	packet_flag,0
	je	no_packet

	mov	ax,first_count		;do the counts match?
	cmp	ax,second_count
	je	counts_ok

	mov	ax,first_count
	call	wordout

	mov	al,' '
	call	chrout

	mov	ax,second_count
	call	wordout

	mov	dx,offset counts_bad_msg
	mov	ah,9
	int	21h
counts_ok:

	mov	bx,offset our_buffer	;find the end of the buffer.
	add	bx,first_count
	cmp	[bx-1],bl		;did we overwrite the first byte
	jne	wrote_first		;  of the magic value?
	mov	dx,offset not_first_msg
	mov	ah,9
	int	21h
wrote_first:

	cmp	[bx],bh			;did we preserve the second byte
	je	wrote_second		;  of the magic value?
	mov	dx,offset not_second_msg
	mov	ah,9
	int	21h
wrote_second:

	mov	al,'.'
	call	chrout

	mov	packet_flag,0
no_packet:
	mov	ah,1			;check for any key.
	int	16h
	je	wait_for_key		;no key -- keep waiting.

	mov	ah,0			;fetch the key.
	int	16h

	mov	ah,3
	mov	bx,handle
	int_pkt
	call	fatal_error

	int	20h


our_recv:
	or	ax,ax			;first or second call?
	jne	our_recv_1		;second -- bump the packet flag.
	cmp	cs:packet_flag,0	;Do we already have one?
	jne	our_recv_2		;yes - return zero.
	push	cs
	pop	es
	mov	di,offset our_buffer
	mov	bx,di			;find the end of the buffer.
	add	bx,cx
	mov	cs:[bx-1],bx		;store a magic value there.
	mov	cs:first_count,cx	;remember the first count.
	db	0cbh			;masm 4.0 doesn't grok "retf"
our_recv_2:
	xor	di,di
	mov	es,ax
	db	0cbh			;masm 4.0 doesn't grok "retf"
our_recv_1:
	mov	cs:second_count,cx	;remember the second count.
	inc	cs:packet_flag
	db	0cbh			;masm 4.0 doesn't grok "retf"


	include	pkterr.asm
	include	getnum.asm
	include	getdig.asm
	include	skipblk.asm
	include	digout.asm
	include	chrout.asm

our_buffer	label	byte

code	ends

	end	start
