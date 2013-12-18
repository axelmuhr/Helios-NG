version	equ	1

	include	defs.asm

;Ported from Tim Krauskopf's micnet.asm, an assembly language
;driver for the MICOM-Interlan NI5210 by Russell Nelson.  Any bugs
;are due to Russell Nelson.
;3c523 version Dan Lanciani ddl@harvard.* (received 5-18-89)
;Added Brad Clements' 1500 byte MTU, Russell Nelson.

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
;  Equates for controlling the 3c523 board
;
IOC	EQU	6

	public	int_no
int_no		db	3,0,0,0		; interrupt number
io_addr		dw	300h,0		; I/O address for card
base_addr	dw  	0c000h,0	; base segment for board

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	13		;from the packet spec
driver_name	db	"3C523",0	;name of the driver.
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

ca_high	db	0a3h + 40h
ca_low	db	0a3h


enable_network:
	mov	ca_high,087h + 40h
	mov	ca_low,087h
	ret


reset_586:
;  Reset the chip
	loadport
	setport	IOC
	mov	al,0a3h
	out	dx,al		; reset the chip
	jmp	$+2
	jmp	$+2
	jmp	$+2
	mov	al,23h
	out	dx,al		; reset the chip
	jmp	$+2
	jmp	$+2
	jmp	$+2
	mov	al,0a3h
	out	dx,al		; reset the chip
	jmp	$+2
	jmp	$+2
	jmp	$+2
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
	inc	dx			; next register
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
	setport IOC
	mov	al,ca_high
	pushf
	cli
	out	dx, al
	jmp	$+2
	jmp	$+2
	jmp	$+2
	mov	al,ca_low
	out	dx, al
	popf
	ret
	assume	ds:code
;yet, we really should assume ds==code for the rest of this stuff.


;
; Here we include the code that is common between 82586 implementations.
; Everything above this is resident.
	include	82586.asm
; Everything below this is discarded upon installation.

	public	usage_msg
usage_msg	db	"usage: 3c523 [-n] [-d] [-w] <packet_int_no> <int_no> <io_addr> <base_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for the 3C523, version ",'0'+majver,".",'0'+version,".",'0'+i82586_version,CR,LF
		db	"Portions Copyright 1988 The Board of Trustees of the University of Illinois",CR,LF,'$'

comment \

	The 3c523 responds with adapter code 0x6042 at slot
registers xxx0 and xxx1.  The setup register is at xxx2 and
contains the following bits:

0: card enable
2,1: csr address select
	00 = 0300
	01 = 1300
	10 = 2300
	11 = 3300
4,3: shared memory address select
	00 = 0c0000
	01 = 0c8000
	10 = 0d0000
	11 = 0d8000
5: set to disable on-board thinnet
7,6: (read-only) shows selected irq
	00 = 12
	01 = 7
	10 = 3
	11 = 9

The interrupt-select register is at xxx3 and uses one bit per irq.

0: int 12
1: int 7
2: int 3
3: int 9

	Again, the documentation stresses that the setup register
should never be written.  The interrupt-select register may be
written with the value corresponding to bits 7.6 in
the setup register to insure corret setup.

\

IRQ_MASK	equ	0c0h
IRQ_TABLE	db	12			; Interrupt Value 0
		db	7			; Interrupt Value 1
		db	3			; Interrupt Value 2
		db	9			; Interrupt Value 3

check_board:
	mov	SCP,0			; 16 bit bus type in scb.

; search all slots for a 3c523 card
	mov	cx, 8			; for all channels(slots)

; channel select value for slots 0,1,2.. is 8,9,A etc
; start with slot 0, and then 7,6,5,4,3,2,1
get_05:
	mov	ax, cx			; channel number
	or	ax, 08h			; reg. select value
	out	96h, al			; select channel

; read adapter id
	mov	dx, 101h
	in	al, dx			; adapter id - ms byte
	mov	ah, al
	dec	dx
	in	al, dx			; adapter id - ls byte

; Check if 3c523
	cmp	ax, 06042h
	je	get_10
	loop	get_05

	mov	dx,offset no_3c523_msg
	jmp	error

get_10:
; found our Adapter

; Get 3c523 IRQ ( read POS Register 0 )
	mov	dx,102h
	in	al,dx
	mov	bl,al
	and	bx,IRQ_MASK
	mov	cl,6
	shr	bx,cl
	mov	al,IRQ_TABLE[bx]
	mov	int_no,al

; Get 3c523 memory address ( read POS Register 0 )
	mov	dx,102h
	in	al,dx
	mov	bl,al
	xor	bh,bh
	mov	cl,11-3
	shl	bx,cl
	and	bx,01800h
	or	bx,0c000h
	mov	base_addr,bx

; Get 3c523 I/O address ( read POS Register 0 )
	mov	dx,102h
	in	al,dx
	mov	bl,al
	xor	bh,bh
	mov	cl,12-1			;should start at bit 12, now at bit 1.
	shl	bx,cl
	and	bx,3000h
	or	bx,0300h
	mov	io_addr,bx

	mov	dx, 102h
	in	al,dx
	or	al,1			;enable the card.
	out	dx,al

	xor	al,al
	out	96h,al			;deselect the card.

	ret

no_3c523_msg	db	"No 3c523 found in your system.",CR,LF,'$'

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

