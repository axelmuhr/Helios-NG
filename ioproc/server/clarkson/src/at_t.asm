version	equ	1

	include	defs.asm

;Ported from Tim Krauskopf's micnet.asm, an assembly language
;driver for the MICOM-Interlan NI5210, by Russell Nelson.  Any bugs
;are due to Russell Nelson.
;Updated to version 1.08 Feb. 17, 1989 by Russell Nelson.
;Updated to support 1500 byte MTU April 27, 1989 By Brad Clements.
;converted to an AT&T driver Feb 16, 1990 by Russell Nelson

;  Copyright, 1988, 1989, 1990, Russell Nelson

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
;  Equates for controlling the AT&T boards
;
;  I/O addresses, writing anything in AL trips these gates
;
;  First six addresses are the EPROM board Ether address (read)
;
IORESET	EQU	0			; reset the board
IOCA	EQU	1			; Channel Attention
IOREV	EQU	6			; Board Revision/Type
IOATTR	EQU	7			; Attribute Register.

;Attribute register contents:
attr_mem	equ	03h		;mask for memory size
attr_mem_64	equ	0
attr_mem_16	equ	1
attr_mem_32	equ	2
attr_mem_8	equ	3
;
attr_bw		equ	04h		;Bus width (0=8 bit, 1=16 bit)
attr_speed	equ	08h		;Board speed (0=10 Mhz, 1=1 Mhz)
attr_c		equ	10h		;Manchester code (0=Manchester, 1=NRZ)
attr_hw		equ	20h		;Host bus width (0=16, 1=8 bit)
attr_es		equ	40h		;Ethernet/Starlan (0 = E, 1 = S)
attr_b		equ	80h		;Boot Rom (0=No ROM, 1=ROM)


;
;  Data segment
;

	public	int_no
int_no		db	2,0,0,0		; interrupt number. 
io_addr		dw	0360h,0		; I/O address for card (jumpers)
base_addr	dw  	0d000h,0	; base segment for board (jumper set)

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	48		;from the packet spec
driver_name	db	20 dup(?)	;name of the driver.
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
usage_msg	db	"usage: at&t [-n] [-d] [-w] <packet_int_no> <int_no> <io_addr> <base_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for the AT&T Starlan/Ethernet boards, version ",'0'+majver,".",'0'+version,".",'0'+i82586_version,CR,LF
		db	"Portions Copyright 1988 The Board of Trustees of the University of Illinois",CR,LF,'$'

board_name_list	dw	board_00, board_01, board_11, board_21
		dw	board_02, board_03, board_unk

board_00	db	00h, "StarLAN 1",0,47
board_01	db	01h, "StarLAN 10 NAU",0,48
board_11	db	11h, "StarLAN 10 R2 NAU",0,48
board_21        db      21h, "Starlan 10 R3 NAU",0,48           ; new board
board_02	db	02h, "EN100",0,49
board_03	db	03h, "StarLAN Fiber NAU",0,55
board_unk	db	-1h, "Unknown AT&T",0,48		;just a guess.

	extrn	chrout: near

check_board:
	assume	ds:code
	loadport
	setport	IOREV
	in	al,dx
	mov	bx,offset board_name_list
check_board_name:
	mov	si,[bx]			;get a pointer to a string.
	add	bx,2
	cmp	byte ptr [si],-1	;is this the end?
	je	check_board_found
	cmp	al,[si]			;is this the right one?
	jne	check_board_name
check_board_found:
	inc	si			;skip the board revision number.

	mov	dx,offset this_board_msg
	mov	ah,9
	int	21h

	mov	ax,ds			;copy the driver name to where
	mov	es,ax			;  we need it.
	mov	di,offset driver_name
check_board_copy:
	lodsb
	stosb
	or	al,al
	je	check_board_done_print
	call	chrout			;print the character.
	jmp	check_board_copy
check_board_done_print:
	lodsb				;copy the driver type number over
	mov	driver_type,al
	mov	al,CR
	call	chrout
	mov	al,LF
	call	chrout

	loadport
	setport	IOATTR
	in	al,dx			;get the bus width bit into bit zero.
	shr	al,1
	shr	al,1
	not	al
	and	al,1			;and negate it.
	mov	SCP,al			;that makes it into the bus width flag.

	in	al,dx			;get the manchester code
	shr	al,1
	shr	al,1
	not	al			;the bit is flipped.
	and	al,4			;isolate just the manchester/~nrz bit.
	mov	byte ptr CBCONF_FLAGS,al

	mov	dx,io_addr	; i/o address
	in	al,dx
	mov	bl,al		; assemble pattern to check
	inc	dx
	in	al,dx
	mov	bh,al
	cmp	bx,0008h	; pattern known to be there in ROM
	jz	have_att_io
	pop	dx			;drop our return address
	mov	dx,offset no_att_io_msg
	jmp	error
have_att_io:

	mov	ax,base_addr
	mov	cx,2000h		;test only what we are going to use.
	call	memory_test
	jz	have_att_mem
	pop	dx			;drop our return address
	mov	dx,offset no_att_mem_msg
	jmp	error
have_att_mem:
	ret


this_board_msg	db	"This board is an AT&T ",'$'
no_att_io_msg	db	"No AT&T board found at that I/O address.",CR,LF,'$'
no_att_mem_msg	db	"No AT&T board found at that memory address.",CR,LF,'$'

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
