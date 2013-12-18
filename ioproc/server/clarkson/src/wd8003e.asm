;History:225,1
version	equ	7

	include	defs.asm

;  Russell Nelson, Clarkson University.
;  Copyright, 1988-1991, Russell Nelson
;  The following people have contributed to this code: David Horne, Eric
;  Henderson, Bob Clements, and Jan Engvald LDC.

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

code	segment	byte public
	assume	cs:code, ds:code

; Stuff specific to the Western Digital WD8003E Ethernet controller board
; C version by Bob Clements, K1BC, May 1988 for the KA9Q TCP/IP package

; The EN registers - First, the board registers

EN_CMD		equ	000h	; Board's command register
EN_REG1		equ	001h	; 8013 bus size register
EN_REG5		equ	005h	; New command register (REGISTER 5)
EN_SAPROM	equ	008h	; Window on station addr prom
EN_REGE		equ	00eh	; Board Id (code) byte

EN_OFF		equ	10h

ENDCFG_BM8	equ	48h

	include	8390.inc

; Board commands in EN_CMD
EN_RESET	equ	080h	; Reset the board
EN_MEMEN	equ	040h	; Enable the shared memory
EN_MEM_MASK	equ	03fh	; B18-B13 of address of the shared memory

; Bits in REG1
ENR1_BUS16BIT	equ	001h	; Bus is 16 bits

; Commands for REG5 register
ENR5_MEM16EN	equ	080h	; Enable 16 bit memory access from bus (8013)
ENR5_LAN16EN	equ	040h	; Enable 16 bit memory access from chip (8013)
ENR5_MEM_MASK	equ	01fh	; B23-B19 of address of the memory (8013)
ENR5_LA19	equ	001h	; B19 of address of the memory (8013)
ENR5_EIL	equ	004h	; Enable 8390 interrupts to bus (microchannel)

; Bits in the REGE register
ENRE_MICROCHANEL equ	080h	; Microchannel bus (vs. PC/AT)
ENRE_LARGERAM	equ	040h	; Large RAM
ENRE_SOFTCONFIG	equ	020h	; Soft config
ENRE_REVMASK	equ	01eh	; Revision mask
ENRE_ETHERNET	equ	001h	; Ethernet (vs. Starlan)

; Shared memory management parameters

SM_TSTART_PG	equ	0	; First page of TX buffer
SM_RSTART_PG	equ	6	; Starting page of ring
SM_RSTOP_PG	equ	32	; Last page +1 of ring

; End of WD8003E parameter definitions

board_features	db	0	; Board features
BF_MEM16EN	equ	1	; 16-bit board, enable 16-bit memory
BF_16K		equ	2	; Board has 16 KB or shared memory

pause_	macro
;	jmp	$+2
endm

ram_enable	macro
	endm


reset_8390	macro
	loadport		; First, pulse the board reset
	setport	EN_CMD
	mov	al,EN_RESET		;Turn on board reset bit
	out	dx,al
	jmp	$+2			;I guess two are enough.
	jmp	$+2
	xor	al,al			;Turn off board reset bit
	out	dx,al
	setport	EN_REG5
	mov	al,ENR5_EIL
	test	sys_features,MICROCHANNEL
	jz	reset_no_mc
	out	dx,al			; enable 8390 interrupts to bus
reset_no_mc:
	endm


terminate_board	macro
	endm

; The following three values may be overridden from the command line.
; If they are omitted from the command line, these defaults are used.

	public	int_no, io_addr, mem_base
int_no		db	3,0,0,0		; Interrupt level
io_addr		dw	0280h,0		; I/O address for card (jumpers)
mem_base	dw	0d000h,0	; Shared memory addr (software)

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	14		;from the packet spec
driver_name	db	'WD8003E',0	;name of the driver.
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

	extrn	sys_features: byte

block_output:
;enter with cx = byte count, ds:si = buffer location, ax = buffer address
	assume	ds:nothing
	mov	es,mem_base		; Set up ES:DI at the shared RAM
	mov	di,ax			; ..
; Can't use movemem which word aligns to the source, but needs to word
; align to the destination writing to WD8003ET/A. Fortunately works for
; all cards.
	inc	cx		; if odd bytes round up.
	and	cx,not 1
	call	rcopy_subr

	clc
	ret			; End of transmit-start routine

block_input:
;enter with cx = byte count, es:di = buffer location, ax = board address.
; Old version checked size, memory space, queue length here. Now done
; in higher level code.
; Set cx to length of this frame.
	assume	ds:nothing,es:nothing
	push	ds
	mov	ds,mem_base		; ds:si points at first byte to move
	mov	si,ax

	cmp	cx,GIANT
	jbe	rcv_size_ok		; is the size sane? 
	cmp	ch,cl			; is it starlan bug (dup of low byte)
	jz	rcv_starlan_bug
	mov	cx,1514			; cap the length
	jmp	rcv_size_ok
rcv_starlan_bug:			; fix the starlan bug
	mov	ch,ds:[si+EN_RBUF_NXT_PG] ; Page after this frame
	cmp	ch,bl
	ja	rcv_frm_no_wrap
	add	ch,byte ptr cs:sm_rstop_ptr		; Wrap if needed
	dec	ch
rcv_frm_no_wrap:
	sub	ch,bl
	dec	ch
rcv_size_ok:

	add	ax,cx			; Find the end of this frame.
	cmp	ah,byte ptr cs:sm_rstop_ptr ; Over the top of the ring?
	jb	rcopy_one_piece		; Go move it

rcopy_wrap:
; Copy in two pieces due to buffer wraparound.
	mov	ah,byte ptr cs:sm_rstop_ptr ; Compute length of first part
	xor	al,al
	sub	ax,si			;  as all of the pages up to wrap point
	sub	cx,ax			; Move the rest in second part
	push	cx			; Save count of second part
	mov	cx,ax			; Count for first move
	call	rcopy_subr
	mov	si,SM_RSTART_PG*256	; Offset to start of first receive page
	pop	cx			; Bytes left to move
rcopy_one_piece:
	call	rcopy_subr
	pop	ds
	ret


rcopy_subr:
	test	board_features,BF_MEM16EN; Is this a WD8013?
	je	sm_wd8003		; no, no need to enable 16-bit access.
	loadport			; Base of device
	setport	EN_REG5			; Enable 16-bit access
	mov	al,ENR5_MEM16EN+ENR5_LAN16EN+ENR5_LA19
	out	dx,al
	shr	cx,1			; convert byte count to word count
	rep	movsw			; Copy packet
	mov	al,ENR5_LAN16EN+ENR5_LA19 ; Disable 16-bit access to WD8013
	out	dx,al
	jmp	short sm_copied
sm_wd8003:
	shr	cx,1			; convert byte count to word count
	rep	movsw
sm_copied:
	jnc	rcv_wrap_even		; odd byte left over?
	lodsw				;   yes, word fetch
	stosb				;   and byte store
rcv_wrap_even:
	ret


	include	8390.asm

	public	usage_msg
usage_msg	db	"usage: WD8003E [-n] [-d] [-w] <packet_int_no> [-o] <int_level> <io_addr> <mem_base>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for Western Digital WD8003 E EBT EB ET/A and E/A, version "
		db	'0'+majver,".",'0'+version,".",'0'+dp8390_version,CR,LF
		db	"Portions Copyright 1988, Robert C. Clements, K1BC",CR,LF,'$'

cfg_err_msg	db	"WD8003E Configuration failed. Check parameters.",CR,LF,'$'
no_board_msg	label	byte
	db	"WD8003E apparently not present at this memory address.",CR,LF,'$'
bad_cksum_msg	label	byte
	db	"WD8003E address PROM contents are invalid.",CR,LF,'$'
bad_board_msg	label	byte
	db	"Suggested WD8003E memory address is invalid.",CR,LF,'$'
occupied_msg	label	byte
	db	"Suggested WD8003E memory address already occupied.",CR,LF,'$'
using_16bits	db	"Accessing the board using 16 bits of data.",CR,LF,'$'
int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'
mem_base_name	db	"Memory address ",'$'

occupied_switch	db	0		;if zero, don't use occupied test.

	extrn	set_recv_isr: near
	extrn	skip_blanks: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

	public	parse_args
parse_args:
	call	skip_blanks
	cmp	al,'-'			;did they specify a switch?
	jne	not_switch
	cmp	byte ptr [si+1],'o'	;did they specify '-o'?
	je	got_occupied_switch
	stc				;no, must be an error.
	ret
got_occupied_switch:
	mov	occupied_switch,1
	add	si,2			;skip past the switch's characters.
	jmp	parse_args		;go parse more arguments.
not_switch:
; ** start of code added by Reinhard Strebler Universitaet Karlsruhe 1/10/91
	test	sys_features,MICROCHANNEL
	jnz	do_mc_defaults
	jmp	just_parse_args
do_mc_defaults:

; channel selector resides at io 96h
; POS register base is at io 100h
; WD8003E ID is one of 6FC0h, 6FC1h or 6FC2h

; search thro' the slots for a wd8003e card
	mov	cx, 8			; for all channels(slots)

; channel select value for slots 0,1,2.. is 8,9,A etc
; start with slot 0, and then 7,6,5,4,3,2,1
get_05:
	mov	ax, cx			; channel number
	or	ax, 08h 		; reg. select value
	mov	dx, 96h 		; channel select register
	out	dx, al			; select channel

; read adapter id
	mov	dx, 101h
	in	al, dx			; adapter id - ms byte
	mov	ah, al
	dec	dx
	in	al, dx			; adapter id - ls byte

; Check if wd8003e
	cmp	ax, 06FC0h
	je	get_10
	cmp	ax, 06FC1h
	je	get_10
	cmp	ax, 06FC2h
	je	get_10
	loop	get_05

	mov	dx,offset no_WD8003E_msg
	mov	etopen_diagn,37
	jmp	error_wrt

no_WD8003E_msg:
	db	"WD8003E apparently not found.",CR,LF,'$'

int_xlate db	03,04,10,15

get_10:
; found our Adapter

; Get WD8003E I/O Address ( read POS Register 0 )
	xor	ax,ax
	mov	dx,102h
	in	al,dx
	and	al,0FEh
	mov	cl,4
	shl	ax,cl
	mov	io_addr,ax
; Get WD8003E shared RAM memory address (read POS Register 1 )
	xor	ax,ax
	mov	dx,103h
	in	al,dx
	and	al,0FCh
	xchg	al,ah
	mov	mem_base,ax
; Get WD8003E IRQ (read POS Register 1 )
	xor	ax,ax
	mov	dx,105h
	in	al,dx
	and	al,003h
	mov	bx,offset int_xlate
	xlat	cs:int_xlate
	mov	int_no,al
; ** end of code added by Reinhard Strebler Universitaet Karlsruhe 1/10/91

just_parse_args:
	mov	di,offset int_no
	call	get_number
	mov	di,offset io_addr
	call	get_number
	mov	di,offset mem_base
	call	get_number
	clc
	ret

	extrn	etopen_diagn: byte
addr_not_avail:
	mov	dx,offset occupied_msg
	mov	etopen_diagn,34
bad_cksum:
	mov	dx,offset bad_cksum_msg
	mov	etopen_diagn,37
	jmp	short error_wrt
bad_memory:
	mov	dx,offset bad_board_msg
	mov	etopen_diagn,37
no_memory:
	mov	dx,offset no_board_msg
	mov	etopen_diagn,37
error_wrt:
	mov	ah,9
	int	21h
	stc
	ret


init_card:
; Now get the board's physical address from on-board PROM into card_hw_addr
	assume ds:code

	test	sys_features,MICROCHANNEL
	jz	etopen_no_mc
	or	board_features,BF_16K
	or	endcfg,ENDCFG_WTS
	loadport
	setport	EN_REG5
	mov	al,ENR5_EIL
	out	dx,al		; enable 8390 interrupts to bus
	jmp	etopen_have_id
etopen_no_mc:			; Check for WD8013EBT
	loadport		; WD8013EBT doesn't have register alaasing
	setport	EN_CMD		; Register 0 may be aliased to Register 8
	mov bx,	dx
	setport	EN_SAPROM
	mov cx,	EN_SAPROM-EN_CMD ; Check 8 bytes
alias_loop:
	in al,	dx		; Get one register
	mov ah,	al
	xchg bx, dx		; Switch to other register
	in al,	dx		; Get other register
	cmp al,	ah		; Are they the same?
	jne	not_aliased	; Nope, not aliased
	inc	bx		; Increment register pair
	inc	dx
	dec	cx		; Decrement loop counter
	jne	alias_loop	; Finished?
	jmp	etopen_have_id	; Aliased; not WD8013EBT
not_aliased:			; Not aliased; Check for 16-bit board
	loadport
	setport	EN_REG1		; Bit 0 must be unmodifiable
	in al,	dx		; Get register 1
	mov bl,	al		; Store original value
	xor al,	ENR1_BUS16BIT	; Flip bit 0
	out dx,	al		; Write it back
	and al,	ENR1_BUS16BIT	; Throw other bits away
	mov ah,	al		; Store bit value
	in al,	dx		; Read register again
	and al,	ENR1_BUS16BIT	; Throw other bits away
	cmp al,	ah		; Was it modified?
	jne	board16bit	; No; board is a WD8013EBT !
	mov al,	bl		; Get original value
	out dx,	al		; Write it back
	jmp	etopen_have_id
board16bit:			; But is it plugged into a 16-bit slot?
	and al,	ENR1_BUS16BIT	; Throw other bits away
	je	etopen_have_id	; Nope; silly board installer!
	mov	dx,offset using_16bits
	mov	ah,9
	int	21h
	or	board_features,BF_MEM16EN+BF_16K
	or	endcfg,ENDCFG_WTS
	loadport
	setport	EN_REG5
	mov	al,ENR5_LAN16EN+ENR5_LA19 ; Write LA19 now, but not MEM16EN
	out	dx,al		; enable 8390 interrupts to bus

etopen_have_id:

	loadport			; base of device
	setport	EN_SAPROM		; Where the address prom is
	cld				; make sure string mode is right
	push	cs			; Point es:di at local copy space
	pop	es
	mov di,	offset curr_hw_addr
	mov cx,	EADDR_LEN		; Set count for loop
	xor bx,	bx			; Clear the addr ROM checksum
ini_addr_loop:
	in	al,dx			; Get a byte of address
	stosb				; Feed it to caller
	add	bl,al			; Compute the checksum
	inc	dx			; Next byte at next I/O port
	loop	ini_addr_loop		; Loop over six bytes

	in al,	dx			; Get seventh byte
	add bl,	al			; Add it in
	inc	dx			; Step to eighth byte
	in al,	dx			; Get last byte
	add bl,	al			; Final checksum
	cmp bl, 0ffh			; Correct?
	je	good_cksum
	jmp	bad_cksum		; No, board is not happy
good_cksum:

; Check if the shared memory address range is available to us
	mov	bx,mem_base
	cmp	occupied_switch,0	; did they insist?
	jne	no_lim_chk		; yes, don't check.
	cmp	bh,080h			; low limit is 8000
	jae	fr_8000
	jmp	bad_memory
fr_8000:
	cmp	bh,0f0h			; upper limit is F000
	jb	to_f000
	jmp	bad_memory
to_f000:
	test	bx,01ffh		; must be on a 8 k boundary
	jz	eightk
	jmp	bad_memory
eightk:
no_lim_chk:
	mov	di,8*1024/16		; 8 kbyte
	mov	sm_rstop_ptr,32
	test	board_features,BF_16K
	jz	just_8k
	mov	cx,16*1024/16		; 16 kbytes
	mov	sm_rstop_ptr,64
just_8k:
	cmp	occupied_switch,0	; did they insist?
	jne	is_avail		; yes, don't check.
	call	occupied_chk		; check if address range is available
	jnc	is_avail
	jmp	addr_not_avail		; we HAVE to have at least 8/16 kbyte
is_avail:
	test	board_features,BF_16K
	jnz	not_32k
	mov	di,32*1024/16		; may be there is space for 32 kbyte
	call	occupied_chk
	jc	not_32k			; no, then don't try it later either
	and	bh,7
	jnz	not_32k			; must be on a 32k boundary
	mov	sm_rstop_ptr,128	; yes, there is space for a WD8003EBT
not_32k:

; Turn on the shared memory block
	loadport
	setport	EN_CMD		; Point at board command register
	mov ax,	mem_base	; Find where shared memory will be mapped
	mov al,	ah		; Shift to right location
	shr al,	1		;  in the map control word
	and al,	EN_MEM_MASK	; Just these bits
	or al,	EN_MEMEN	; Command to turn on map
	test	sys_features,MICROCHANNEL
	jz	AT_card
	mov	al,EN_MEMEN	; membase handled different for MC card
AT_card:
	out dx,	al		; Create that memory

; Find how much memory this card has (without destroying other memory)
	mov	si,ax			; save bord command value
	mov	es,mem_base
	mov	bl,0FFh			; first try 32 kbyte (WD8003EBT)
	mov	bh,sm_rstop_ptr		;   or what is available
	dec	bh
memloop:
	dec	bx			; use even address
	cli				; disable interrupts
	mov	cx,es:[bx]		; save old memory contents
	mov	word ptr es:[bx],05A5Ah	; put testpattern
	loadport
	setport	EN_CCMD			; drain the board bus for any
	in	al,dx			;   capacitive memory
	cmp	word ptr es:[bx],05A5Ah	; any real memory there?
	jne	not_our_mem		;   no
	setport	EN_CMD			;   yes
	mov	ax,si
	and	al,0FFh xor EN_MEMEN
	out	dx,al			; turn off our memory
	jmp	short $+2
	or	al,EN_MEMEN
	cmp	word ptr es:[bx],05A5Ah	; was it OUR memory?
	out	dx,al			; turn on our memory.
	jmp	short $+2
	mov	es:[bx],cx		; restore the original contents.
	sti
	jne	our_mem			;   yes, it wasn't there any more
not_our_mem:				;   no, it was still there
	shr	bx,1			; test if half as much memory
	cmp	bx,1FFFh		; down to 8 kbyte
	jae	memloop
	jmp	no_memory		; no memory at address mem_base
our_mem:				; it IS our memory!
	inc	bh
	mov	sm_rstop_ptr,bh		; # of 256 byte ring bufs + 1
	mov	ch,bh
	xor	cl,cl
	mov	ax,mem_base
	call	memory_test		; check all of that memory
	je	mem_ok
	jmp	no_memory
mem_ok:
	ret

	public	print_parameters
print_parameters:
	mov	di,offset int_no
	mov	dx,offset int_no_name
	call	print_number
	mov	di,offset io_addr
	mov	dx,offset io_addr_name
	call	print_number
	mov	di,offset mem_base
	mov	dx,offset mem_base_name
	call	print_number
	ret

	include	memtest.asm
	include	occupied.asm

code	ends

	end
