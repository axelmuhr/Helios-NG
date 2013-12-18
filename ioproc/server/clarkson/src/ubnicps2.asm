version	equ	1

	include	defs.asm

;Ported from Tim Krauskopf's micnet.asm, an assembly language
;driver for the MICOM-Interlan NI5210 by Russell Nelson.  Any bugs
;are due to Russell Nelson.
;3c523 version Dan Lanciani ddl@harvard.* (received 5-18-89)
;Added Brad Clements' 1500 byte MTU, Russell Nelson.
;Changed it into a UB NIC-PS/2 driver, 10/90

;  Copyright 1990, Russell Nelson

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
IO_CSR	equ	0
IO_CA	equ	1

	public	int_no
int_no		db	3,0,0,0		; interrupt number
io_addr		dw	300h,0		; I/O address for card
base_addr	dw  	0d000h,0	; RAM segment for board
eprom_addr	dw  	0c000h,0	; EPROM segment for board

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	41		;from the packet spec
driver_name	db	"UB NIC-PS2",0	;name of the driver.
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
	setport	IO_CSR			; enable network
	in	al,dx
	and	al,not 2		;turn off loopback.
	or	al,8			;turn on 82586 interrupts.
	out	dx,al
	ret


reset_586:
;  Reset the chip
	loadport
	setport	IO_CSR
	in	al,dx
	or	al,1
	out	dx,al		; reset the chip
	jmp	$+2
	jmp	$+2
	jmp	$+2
	and	al,not 1
	out	dx,al		; done resetting the chip
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
	push	ds			;save ds.
	mov	ds,eprom_addr		;point to the eprom.
	mov	si,10h
	mov	cx,EADDR_LEN
	cld
get_address_1:
	lodsw				;we have to get words,
	stosb				;  and store bytes.
	loop	get_address_1		; go back for rest
	pop	ds
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
	setport IO_CA
	out	dx, al
	ret
	assume	ds:code
;yet, we really should assume ds==code for the rest of this stuff.


;
; Here we include the code that is common between 82586 implementations.
; Everything above this is resident.
	include	82586.asm
; Everything below this is discarded upon installation.

	public	usage_msg
usage_msg	db	"usage: ubnicps2 [-n] [-d] [-w] <packet_int_no> <int_no> <io_addr> <base_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for the ubnicps2, version ",'0'+majver,".",'0'+version,".",'0'+i82586_version,CR,LF,'$'

check_board:
	mov	SCP,0			; 16 bit bus type in scb.

; search all slots for a ubnicps2 card
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

; Check if ubnicps2
	cmp	ax, 0eff5h
	je	get_10
	loop	get_05

	mov	dx,offset no_ubnicps2_msg
	jmp	error

get_10:
; found our Adapter

; Get ubnicps2 I/O address
	mov	dx,102h
	in	al,dx
	mov	bl,al
	xor	bh,bh
	shl	bx,1			;should start at bit 2, now at bit 1.
	and	bx,1ch
	or	bx,1540h
	mov	io_addr,bx

; Set ubnicps2 IRQ.  Unbelievably, it only uses interrupt number 3.
	mov	int_no,3

; Get ubnicps2 RAM address
	mov	dx,103h
	in	al,dx
	xor	bl,bl
	mov	bh,al			;put it in the high byte.
	mov	cl,4
	shl	bx,cl
	mov	base_addr,bx

; Get ubnicps2 EPROM address
	mov	dx,104h
	in	al,dx
	xor	bl,bl
	mov	bh,al			;put it in the high byte.
	mov	cl,4
	shl	bx,cl
	mov	eprom_addr,bx

; Get ubnicps2 RAM address bits.
	mov	dx,105h
	in	al,dx
	mov	bl,al
	xor	bh,bh
	mov	cl,10-0			;should start at bit 10, now at bit 0.
	shl	bx,cl
	and	bx,0c00h
	or	base_addr,bx

; Get ubnicps2 EPROM address bits.
	mov	dx,105h
	in	al,dx
	mov	bl,al
	xor	bh,bh
	mov	cl,10-2			;should start at bit 10, now at bit 2.
	shl	bx,cl
	and	bx,0c00h
	or	eprom_addr,bx

	mov	dx, 102h
	in	al,dx
	or	al,1			;enable the card.
	out	dx,al

	xor	al,al
	out	96h,al			;deselect the card.

  if 0
	loadport
	setport	IO_CSR
	mov	al,23h
	out	dx,al		; reset the chip
	jmp	$+2
	jmp	$+2
	jmp	$+2
	mov	al,63h
	out	dx,al		; reset the chip
	jmp	$+2
	jmp	$+2
	jmp	$+2
	mov	al,23h
	out	dx,al		; reset the chip
	jmp	$+2
	jmp	$+2
	jmp	$+2
  endif

	ret

no_ubnicps2_msg	db	"No ubnicps2 found at that memory address.",CR,LF,'$'

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
