version	equ	0

	include	defs.asm

;Ported from Tim Krauskopf's micnet.asm, an assembly language
;driver for the MICOM-Interlan NI5210, by Russell Nelson.  Any bugs
;are due to Russell Nelson.
;Updated to version 1.08 Feb. 17, 1989 by Russell Nelson.
;Updated to support 1500 byte MTU April 27, 1989 By Brad Clements.

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

code	segment	word public
	assume	cs:code, ds:code

;
;  Equates for controlling the ubnicps2 board
;
SCR		equ	byte ptr 0ffffh
SCR_GREEN	equ	1
SCR_CA		equ	2
SCR_LOOPBACK	equ	4
SCR_RESET	equ	8

;
;  Data segment
;

	public	int_no
int_no		db	3,0,0,0		; interrupt number. 
base_addr	dw  	0d000h,0	; base segment for board (jumper set)
enet_addr	db	EADDR_LEN dup(?)

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	11		;from the packet spec
driver_name	db	"NCR ET105",0	;name of the driver.
driver_function	db	2
parameter_list	label	byte
	db	1	;major rev of packet driver
	db	9	;minor rev of packet driver
	db	14	;length of parameter list
	db	EADDR_LEN	;length of MAC-layer address
	dw	GIANT	;MTU, including MAC headers
	dw	MAX_MULTICAST * EADDR_LEN	;buffer size of multicast addrs
	dw	0	;(# of back-to-back MTU rcvs) - 1
	dw	0	;(# of successive xmits) - 1
int_num	dw	0	;Interrupt # to hook for post-EOI
			;processing, 0 == none,

enable_network:
;  connect to network
	push	es
	mov	es,base_addr
	or	es:[SCR],SCR_LOOPBACK
	pop	es
	ret


reset_586:
;  Reset the chip
	push	es
	mov	es,base_addr
	or	es:[SCR],SCR_RESET
	jmp	$+2
	jmp	$+2
	jmp	$+2
	jmp	$+2
	and	es:[SCR],not SCR_RESET
	pop	es
	ret


	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	cmp	cx,EADDR_LEN		;make sure that we have enough room.
	jb	get_address_2
	mov	cx,EADDR_LEN
	mov	si,offset enet_addr
	rep	movsb
	mov	cx,EADDR_LEN
	clc
	ret
get_address_2:
	stc
	ret


doca:
;we may be called from places in which ds is unknown.
	assume	ds:nothing
	push	es
	mov	es,base_addr
	or	es:[SCR],SCR_CA
	and	es:[SCR],not SCR_CA
	xor	es:[SCR],SCR_GREEN	;blink the led on each CA.
	pop	es
	ret
	assume	ds:code
;yet, we really should assume ds==code for the rest of this stuff.

;
; Here we include the code that is common between 82586 implementations.
; Everything above this is resident.
	include	82586.asm
; Everything below this is discarded upon installation.

	public	usage_msg
usage_msg	db	"usage: ncret105 [-n] [-d] [-w] <packet_int_no> <int_no> <base_addr> <Ethernet_address>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for the NCR ET105, version ",'0'+majver,".",'0'+version,".",'0'+i82586_version,CR,LF
		db	"Portions Copyright 1988 The Board of Trustees of the University of Illinois",CR,LF,'$'

no_mem_msg	db	"No ET 105 found at ",'$'
no_eaddr_msg	db	"You must specify an Ethernet address for the ET 105",CR,LF
		db	"Your card came with a pre-printed label giving the address assigned",CR,LF
		db	"to this card.",CR,LF,'$'

check_board:
	mov	SCP,0			;16-bit bus internally.
	mov	ax,base_addr
	mov	cx,4000h-1		;test all 16K minus the SCR.
	call	memory_test
	jz	have_mem
	pop	dx			;drop our return address
	mov	di,offset base_addr
	mov	dx,offset no_mem_msg
	call	print_number
	stc
	ret
have_mem:
	mov	si,offset enet_addr
	mov	cx,EADDR_LEN
	xor	ah,ah
check_eaddr:
	lodsb
	or	ah,al
	loop	check_eaddr
	or	ah,ah			;did we have all zeroes?
	jne	have_eaddr
	pop	dx			;return to the caller of etopen.
	mov	dx,offset no_eaddr_msg	;print this error first, though.
	mov	ah,9
	int	21h
	stc
	ret
have_eaddr:
	ret


	public	parse_args
parse_args:
	mov	di,offset int_no
	call	get_number
	mov	di,offset base_addr
	call	get_number
	push	ds
	pop	es
	mov	di,offset enet_addr
	call	get_eaddr
	clc
	ret


int_no_name	db	"Interrupt number ",'$'
base_addr_name	db	"Memory address ",'$'


	public	print_parameters
print_parameters:
	mov	di,offset int_no
	mov	dx,offset int_no_name
	call	print_number
	mov	ax,memory_begin
	mov	cl,4
	shr	ax,cl
	add	base_addr,ax
	push	ax
	mov	di,offset base_addr
	mov	dx,offset base_addr_name
	call	print_number
	pop	ax
	sub	base_addr,ax
	ret

	include	memtest.asm
	extrn	get_hex: near
	include	getea.asm

code	ends

	end
