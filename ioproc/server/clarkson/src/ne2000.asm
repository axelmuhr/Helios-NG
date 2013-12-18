version	equ	4
;History:4,1 0

;  Russell Nelson, Clarkson University.
;  Copyright, 1988-1991, Russell Nelson
;  The following people have contributed to this code: David Horne, Eric
;  Henderson, and Bob Clements.

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

code	segment	word public
	assume	cs:code, ds:code

;*****************************************************************************
;
;	NE2000 controller board offsets
;	IO port definition (BASE in io_addr)
;*****************************************************************************
NE_DATAPORT	EQU	10h		; NE2000 Port Window.
NE_RESET	EQU	1fh		; Issue a read for reset
EN_OFF		equ	0h

	include	8390.inc

; Shared memory management parameters

SM_TSTART_PG	equ	040h	; First page of TX buffer
SM_RSTART_PG	equ	046h	; Starting page of RX ring
SM_RSTOP_PG	equ	080h	; Last page +1 of RX ring

pause_	macro
;	jmp	$+2
endm

longpause macro
	push	cx
	mov	cx,0
	loop	$
	pop	cx
endm

ram_enable	macro
	endm

reset_8390	macro
	loadport
	setport	NE_RESET
	in	al,dx
	longpause
	out	dx,al		; should set command 21, 80
	longpause

	endm

terminate_board	macro
	endm

	public	int_no, io_addr
int_no		db	2,0,0,0		;must be four bytes long for get_number.
io_addr		dw	0300h,0		; I/O address for card (jumpers)

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	dw	54		;from the packet spec
driver_name	db	'NE2000',0	;name of the driver.
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

is_186		db	0

;
;	Block input routine
;	CX = byte count, es:di = buffer location, ax = buffer address

	public	block_input
block_input:
	push	ax		; save buffer address
	loadport
	setport EN_CCMD
	pause_
	mov	al,ENC_NODMA+ENC_PAGE0+ENC_START
	out	dx,al
	setport	EN0_RCNTLO	; remote byte count 0
	pause_
	mov	al,cl
	out	dx,al
	setport	EN0_RCNTHI
	pause_
	mov	al,ch
	out	dx,al
	pop	ax		; get our page back
	setport	EN0_RSARLO
	pause_
	out	dx,al		; set as hi address
	setport	EN0_RSARHI
	pause_
	mov	al,ah
	out	dx,al
	setport EN_CCMD
	pause_
	mov	al,ENC_RREAD+ENC_START	; read and start
	out	dx,al
	setport	NE_DATAPORT
	pause_
	cmp	is_186,0
	jnz	read_186
read_loop:
	in	al,dx		; get a byte
	stosb			; save it
	loop	read_loop
	ret
read_186:
	inc	cx		; make even
	shr	cx,1		; word count
	db	0f3h, 06dh	;masm 4.0 doesn't grok "rep insw"
	ret
;
;	Block output routine
;	CX = byte count, ds:si = buffer location, ax = buffer address

block_output:
	assume	ds:nothing
	push	ax		; save buffer address
	inc	cx		; make even
	and	cx,0fffeh
	loadport
	setport EN_CCMD
	pause_
	mov	al,ENC_NODMA+ENC_START
	out	dx,al		; stop & clear the chip
	setport	EN0_RCNTLO	; remote byte count 0
	pause_
	mov	al,cl
	out	dx,al
	setport	EN0_RCNTHI
	pause_
	mov	al,ch
	out	dx,al
	pop	ax		; get our page back
	setport	EN0_RSARLO
	pause_
	out	dx,al		; set as lo address
	setport	EN0_RSARHI
	pause_
	mov	al,ah
	out	dx,al
	setport EN_CCMD
	pause_
	mov	al,ENC_RWRITE+ENC_START	; write and start
	out	dx,al
	setport	NE_DATAPORT
	pause_
	cmp	byte ptr is_186,0
	jnz	write_186
write_loop:
	lodsb			; get a byte
	out	dx,al		; save it
	loop	write_loop
	jmp	short block_output_1
write_186:
	shr	cx,1		; word count
	db	0f3h, 06fh	;masm 4.0 doesn't grok "rep outsw"
block_output_1:
	mov	cx,0
	setport	EN0_ISR
tx_check_rdc:
	in	al,dx
	test	al,ENISR_RDC	; dma done ???
	jnz	tx_start
	loop	tx_check_rdc
	stc
	ret
tx_start:
	clc
	ret


	include	8390.asm

	public	usage_msg
usage_msg	db	"usage: NE2000 [-n] [-d] [-w] <packet_int_no> <int_level> <io_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for NE2000, version "
		db	'0'+majver,".",'0'+version,".",'0'+dp8390_version,CR,LF,'$'

cfg_err_msg	db	"NE2000 Configuration failed. Check parameters.",CR,LF,'$'
int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'

	extrn	set_recv_isr: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

	public	parse_args
parse_args:
;exit with nc if all went well, cy otherwise.
	mov	di,offset int_no
	call	get_number
	mov	di,offset io_addr
	call	get_number
	clc
	ret

	extrn	etopen_diagn: byte

init_card:
;get the board data. This is (16) bytes starting at remote
;dma address 0. Put it in a buffer called board_data.
	assume	ds:code

	or	endcfg,ENDCFG_WTS

	loadport
	mov	al,endcfg
	setport	EN0_DCFG
	pause_
	out	dx,al

	mov	cx,10h		; get 16 bytes,
	push	ds
	pop	es		; set es to ds
	mov	di,offset board_data

	setport EN_CCMD
	pause_
	mov	al,ENC_NODMA+ENC_PAGE0+ENC_START
	out	dx,al
	setport	EN0_RCNTLO	; remote byte count 0
	pause_
	mov	al,20h		; count is actually doubled.
	out	dx,al
	setport	EN0_RCNTHI
	pause_
	xor	al,al		; high byte of count is zero.
	out	dx,al

	mov	ax,0		; from address 0

	setport	EN0_RSARLO
	pause_
	out	dx,al		; set as hi address
	setport	EN0_RSARHI
	pause_
	mov	al,ah
	out	dx,al
	setport EN_CCMD
	pause_
	mov	al,ENC_RREAD+ENC_START	; read and start
	out	dx,al
	setport	NE_DATAPORT
	pause_
sp_read_loop:
	in	al,dx		; get a byte
	stosb			; save it
	loop	sp_read_loop

	push    ds              ; Copy from card's address to current address
	pop     es

	mov si, offset board_data	; address is at start
	mov di, offset curr_hw_addr
	mov cx, EADDR_LEN       ; Copy one address length
	rep     movsb           ; ..

	ret

	public	print_parameters
print_parameters:
;echo our command-line parameters
	mov	di,offset int_no
	mov	dx,offset int_no_name
	call	print_number
	mov	di,offset io_addr
	mov	dx,offset io_addr_name
	call	print_number
	ret

code	ends

	end
