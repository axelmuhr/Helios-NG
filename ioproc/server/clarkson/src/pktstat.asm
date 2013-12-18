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
 db "Packet statistics version ",'0'+majver,".",'0'+version," copyright 1990, Russell Nelson.",CR,LF
 db "This program is free software; see the file COPYING for details.",CR,LF
 db "NO WARRANTY; see the file COPYING for details.",CR,LF
crlf_msg	db	CR,LF,'$'

int_pkt	macro
	pushf
	cli
	call	their_isr
	endm

their_isr	dd	?
packet_int_no	db	60h,0,0,0
packet_int_end	db	7fh,0,0,0

handle		dw	?

bogus_type	db	0,0		;totally bogus type code.

signature	db	'PKT DRVR',0
signature_len	equ	$-signature
usage_msg	db	"usage: pktstat <packet_int_no> [packet_int_no_end]",'$'

stat_names	db	"	pkt_in	pkt_out	byt_in	byt_out	err_in	err_out	pk_drop",CR,LF,'$'
printed_names	db	0		;haven't printed the names yet.

statistics_list	label	dword
packets_in	dw	?,?
packets_out	dw	?,?
bytes_in	dw	?,?
bytes_out	dw	?,?
errors_in	dw	?,?
errors_out	dw	?,?
packets_dropped	dw	?,?		;dropped due to no type handler.
statistics_count	equ	$ - statistics_list

check_packet_no:
	or	word ptr [di+2],0
	jne	usage_error
	cmp	word ptr [di],60h
	jb	usage_error
	cmp	word ptr [di],7fh
	ja	usage_error
	ret


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

	mov	di,offset packet_int_no
	call	get_number
	jc	start_2			;if they entered a number, change the default.
	mov	word ptr packet_int_end+0,cx
	mov	word ptr packet_int_end+2,bx
start_2:
	call	check_packet_no

	mov	di,offset packet_int_end
	call	get_number

	call	check_packet_no

	call	skip_blanks
	cmp	al,CR
	jne	usage_error

	mov	al,packet_int_no	;make sure they're in the right order.
	cmp	al,packet_int_end
	ja	usage_error

chk_loop:
	call	chk_int
	mov	al,packet_int_no
	cmp	al,packet_int_end
	jz	all_done
	inc	packet_int_no		; increment
	jmp	chk_loop
all_done:
	mov	al,0
	mov	ah,04ch
	int	21h			; exit with errorlevel 0

chk_int:
;check packet_int_no for a packet driver.  If there's one there, print
;the statistics and return nc.  If not, return cy.
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
	stc
	ret
have_signature:

	push	ds
	mov	ax,1ffh			;driver_info
	int_pkt
	pop	ds
	call	fatal_error

	mov	ah,2			;access_type
	mov	al,ch			;their class from driver_info().
	mov	bx,dx			;their type from driver_info().
	mov	dl,cl			;their number from driver_info().
	mov	cx,2			;use a type length of 2.
	mov	si,offset bogus_type
	push	cs			;es:di -> our receiver.
	pop	es
	mov	di,offset our_recv
	int_pkt
	call	fatal_error
	mov	handle,ax

	mov	ah,24			;get the statistics
	mov	bx,handle
	int_pkt
	jc	bad
	assume	ds:nothing
;ds:si now points to the statistics list.
	mov	cx,statistics_count/2
	mov	ax,cs
	mov	es,ax
	mov	di,offset statistics_list
	rep	movsw
	mov	ds,ax			;restore ds to cs.

	cmp	printed_names,0		;have we printed the names yet?
	jne	did_print_names

	inc	printed_names
	mov	dx,offset stat_names
	mov	ah,9
	int	21h
did_print_names:

	mov	al,packet_int_no	;print the packet interrupt number.
	call	byteout
	mov	al,'I'-40h		;tab over.
	call	chrout

	mov	bx,offset statistics_list
	mov	cx,statistics_count/4
print_stats:
	mov	ax,[bx]
	mov	dx,[bx+2]
	push	bx
	push	cx
	call	decout
	pop	cx
	pop	bx
	add	bx,4
	mov	al,'I'-40h		;tab over.
	call	chrout
	loop	print_stats

	mov	dx,offset crlf_msg
	mov	ah,9
	int	21h

	mov	ah,3			;release_type
	mov	bx,handle
	int_pkt
	call	fatal_error

	clc
	ret


bad:
	push	cs
	pop	ds
	call	print_error

	mov	ah,3			;release_type
	mov	bx,handle
	int_pkt
	call	fatal_error

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
	include	getnum.asm
	include	skipblk.asm
	include	getdig.asm
	include	decout.asm
	include	digout.asm
	include	chrout.asm

our_buffer	label	byte

code	ends

	end	start
