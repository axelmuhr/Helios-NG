version	equ	0

	include	defs.asm

;Ported from Tim Krauskopf's micnet.asm, an assembly language
;driver for the MICOM-Interlan NI5210, by Russell Nelson.  Any bugs
;are due to Russell Nelson.
;Updated to version 1.08 Feb. 17, 1989 by Russell Nelson.
;Updated to support 1500 byte MTU April 27, 1989 By Brad Clements.
;Mangled to become a NI9210 driver, September 1989 by Russell Nelson

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
;  Equates for controlling the MICOM board
;
;  I/O addresses, writing anything in AL trips these gates
;
;  First six addresses are the EPROM board Ether address (read)
;
IORESET	EQU	0			; reset the board
IOCA	EQU	2			; execute command which is in SCB
IODIS	EQU	4			; disable network connect
IOENA	EQU	6			; enable network

;
;  Data segment
;

	public	int_no
int_no		db	2,0,0,0		; interrupt number. 
io_addr		dw	0360h,0		; I/O address for card (pos)
base_addr	dw  	0d000h,0	; base segment for board (pos)

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	11		;from the packet spec
driver_name	db	"NI9210",0	;name of the driver.
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
	loadport
	setport	IOENA			; enable network
	out	dx,al			; any al value
	ret


reset_586:
;  Reset the chip
	loadport
	setport	IORESET
	out	dx,al
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
	mov	dx,io_addr		; Get our IO base address.
	cld
get_address_1:
	in	al,dx			; get a byte of the eprom address
	stosb				; put it away
	add	dx,2			; next register
	loop	get_address_1		; go back for rest
	mov	cx,EADDR_LEN
	clc
	ret
get_address_2:
	stc
	ret


doca:
;we may be called from places in which ds is unknown.
	assume	ds:nothing
	loadport
	setport	IOCA
	out	dx,al			; send it
	ret
	assume	ds:code
;yet, we really should assume ds==code for the rest of this stuff.

;
; Here we include the code that is common between 82586 implementations.
; Everything above this is resident.
	include	82586.asm
; Everything below this is discarded upon installation.

	public	usage_msg
usage_msg	db	"usage: ni9210 [-n] [-d] [-w] <packet_int_no> <int_no> <io_addr> <base_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for the MICOM-Interlan NI9210, version ",'0'+majver,".",'0'+version,".",'0'+i82586_version,CR,LF
		db	"Portions Copyright 1988 The Board of Trustees of the University of Illinois",CR,LF,'$'

IRQ_MASK	equ	06h
IRQ_TABLE	db	9			; Interrupt Value 0
		db	10			; Interrupt Value 1
		db	11			; Interrupt Value 2
		db	3			; Interrupt Value 3

check_board:
	mov	SCP,0			; 16 bit bus type in scb.

;The following code to read the POS registers is courtesy of Racal-Interlan.

; channel selector resides at io 96h
; POS register base is at io 100h
; 9210 ID is 0DF0Fh

; search thro' the slots for a 9210 card
	mov	cx, 8			; for all channels(slots)

; channel select value for slots 0,1,2.. is 8,9,A etc
; start with slot 0, and then 7,6,5,4,3,2,1
get_05:
	mov	ax, cx			; channel number
	or	ax, 08h			; reg. select value
	mov	dx, 96h			; channel select register
	out	dx, al			; select channel

; read adapter id
	mov	dx, 100h
	in	al, dx			; adapter id - ms byte
	mov	ah, al
	inc	dx
	in	al, dx			; adapter id - ls byte

; Check if 9210
	cmp	ax, 0DF0Fh
	je	get_10
	loop	get_05

	mov	dx,offset no_9210_msg
	jmp	error

get_10:
; found our Adapter

; Get Ni9210 I/O Address ( read POS Register 1 )
	xor	ax,ax
	mov	dx,103h
	in	al,dx
	xor	ah,ah
	mov	cl,5
	shl	ax,cl
	mov	io_addr,ax

; Get Ni9210 IRQ ( read POS Register 0 )
	mov	dx,102h
	in	al,dx
	mov	bl,al
	and	bx,IRQ_MASK
	shr	bx,1
	mov	al,IRQ_TABLE[bx]
	mov	int_no,al

; Get Memory Address ( read POS Registers 2 and 3 )
	mov	dx,105h
	in	al,dx
	mov	ah,al
	dec	dx
	in	al,dx
	and	ax,3fe0h
	mov	cl,5
	shl	ax,cl
	mov	base_addr,ax

	mov	dx, 102h
	in	al,dx
	or	al,1			;enable the card.
	out	dx,al

	mov	dx, 96h			;deselect the card.
	xor	al,al
	out	dx,al

	mov	dx,io_addr		; i/o address
	add	dx,EADDR_LEN*2		; look past the ethernet address.
	in	al,dx
	mov	bl,al			; assemble pattern to check
	add	dx,2
	in	al,dx
	mov	bh,al
	cmp	bx,05500h		; pattern known to be there in ROM
	jz	have_9210_io
	pop	dx			;drop our return address
	mov	dx,offset no_9210_io_msg
	jmp	error
have_9210_io:

	mov	ax,base_addr
	mov	cx,2000h		;test only what we are going to use.
	call	memory_test
	jz	have_9210_mem
	pop	dx			;drop our return address
	mov	dx,offset no_9210_mem_msg
	jmp	error
have_9210_mem:
	ret

no_9210_msg	db	"No 9210 found.",CR,LF,'$'


no_9210_io_msg	db	"No 9210 found at that I/O address.",CR,LF,'$'
no_9210_mem_msg	db	"No 9210 found at that memory address.",CR,LF,'$'

	public	parse_args
parse_args:
	mov	di,offset int_no
	call	get_number
	mov	di,offset io_addr
	call	get_number
	mov	di,offset base_addr
	call	get_number
	clc
	ret


int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'
base_addr_name	db	"Memory address ",'$'


	public	print_parameters
print_parameters:
	mov	di,offset int_no
	mov	dx,offset int_no_name
	call	print_number
	mov	di,offset io_addr
	mov	dx,offset io_addr_name
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

code	ends

	end
