version	equ	0
;History:349,1

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
;	hppclan controller board offsets
;	IO port definition (BASE in io_addr)
;*****************************************************************************
NE_DATAPORT	EQU	0ch		; hppclan Port Window.
HP_PROM		equ	00h
HP_OPTION	equ	08h
EN_OFF		equ	10h

ENDCFG_BM8	equ	48h

OPTION_RUN	equ	01h
OPTION_DATA	equ	10h

	include	8390.inc

; Shared memory management parameters

SM_TSTART_PG	EQU	0h		; First page of TX buffer
SM_RSTART_PG	EQU	6h		; start at page 6
SM_RSTOP_PG	EQU	80h		; end at page 80

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

mc_chan_on	macro
	mov	al,chan_sel
	mov	dx,96h
	out	dx,al			; select channel

	mov	dx,94h
	mov	al,0a0h			
	out	dx,al			; protect system board

	mov	dx,102h			; point dx at option reg
endm

mc_chan_off	macro
	mov	dx,96h
	xor	al,al
	out	dx,al
endm

reset_8390	macro
	local   reset_8390_1
	local   is_mc_b
	local	not_mc_c
	local	not_mc_d

	test	sys_features,MICROCHANNEL
	jne	is_mc_b
	jmp	not_mc_c
is_mc_b:
	mc_chan_on
	in	al,dx			;capture and save option register
	and	al, not OPTION_RUN	;reset the 8390
	out	dx,al
	longpause
	or	al, OPTION_RUN		;turn it back on
	out	dx,al		
	mc_chan_off
	loadport			;Set DX the way we expect it
	setport	HP_OPTION		
	jmp	not_mc_d

not_mc_c:
	loadport
	setport	HP_OPTION		;hard reset 8390.
	xor	al,al			;reset the 8390
	out	dx,al
	longpause
	mov	al,int_no		;get our interrupt number, and
	cmp	al,9			;is it really interrupt 2?
	jne	reset_8390_1
	mov	al,2
reset_8390_1:
	shl	al,1			;put it where we need it
	or	al, OPTION_RUN		;turn it back on
	out	dx,al		

not_mc_d:
	endm

terminate_board	macro
	local is_mc_a
	local not_mc_a
	local not_mc_b
	test	sys_features,MICROCHANNEL
	jne	is_mc_a
	jmp	not_mc_a

is_mc_a:
	in	al,dx			;capture and save option register
	and	al, not OPTION_RUN	;reset the 8390
	out	dx,al
	mc_chan_off
	loadport			;Set DX the way we expect it
	setport	HP_OPTION		
	jmp	not_mc_b

not_mc_a:
	loadport
	setport	HP_OPTION		;hard reset 8390.
	xor	al,al
	out	dx,al
not_mc_b:
	endm

	extrn	sys_features: byte

	extrn	set_recv_isr: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

;enter with al = single character
	extrn	chrout: near

	public	int_no, io_addr
int_no		db	3,0,0,0		;must be four bytes long for get_number.
io_addr		dw	0300h,0		; I/O address for card (jumpers)

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	?		;Wild card matches any type
driver_name	db	"HP ",20 dup(?)	;name of the driver.
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

chan_sel	db	0	;Channel select for MICROCHANNEL machines
;
;	Special case Block input routine. Used on extra memory
;	space for board ID etc. DMA count is set X2,
;	CX = byte count, es:si = buffer location, ax = buffer address
;
sp_block_input:
;	Nothing special needed for hppclan
;
;	Block input routine
;	CX = byte count, es:si = buffer location, ax = buffer address

	public	block_input
block_input:
	push	ax			; save buffer address

	loadport
	setport	HP_OPTION		
	test	sys_features,MICROCHANNEL
	je	not_mc_1
	jmp	is_mc_1
not_mc_1:
	in	al,dx
	or	al,OPTION_DATA		;Enable the data port except on MCA
	out	dx,al

is_mc_1:
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
	shr	cx,1
read_loop:
	in	ax,dx		; get a word
	stosw			; save it
	loop	read_loop
	jnc	read_done
	in	al,dx
	stosb
	jmp	short read_done
read_186:
	shr	cx,1
	db	0f3h, 06dh	;masm 4.0 doesn't grok "rep insw"
	jnc	read_done
	db	06ch		;masm 4.0 doesn't grok "insb"
read_done:
	setport	HP_OPTION
	test	sys_features,MICROCHANNEL
	je	not_mc_2
	ret
not_mc_2:
	in	al,dx
	and	al,not OPTION_DATA
	out	dx,al
	ret
;
;	Block output routine
;	CX = byte count, ds:si = buffer location, ax = buffer address


block_output:
	assume	ds:nothing
	push	ax		; save buffer address

	loadport
	setport	HP_OPTION
	test	sys_features,MICROCHANNEL
	je	not_mc_3
	jmp	is_mc_3
not_mc_3:
	in	al,dx
	or	al,OPTION_DATA	; enable the data port except on MCA
	out	dx,al

is_mc_3:
	inc	cx		; make even
	and	cx,0fffeh
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
	shr	cx,1
	cmp	is_186,0
	jnz	write_186
write_loop:
	lodsw			; get a word
	out	dx,ax		; save it
	loop	write_loop
	jmp	short write_done
write_186:
	db	0f3h, 06fh		;masm 4.0 doesn't grok "rep outsw"
write_done:
	mov	cx,0
	setport	EN0_ISR
tx_check_rdc:
	in	al,dx
	test	al,ENISR_RDC	; dma done ???
	clc
	jnz	tx_start
	loop	tx_check_rdc
	stc
tx_start:
	setport	HP_OPTION
	test	sys_features,MICROCHANNEL
	je	not_mc_4
	ret
not_mc_4:
	lahf				;save cy.
	in	al,dx
	and	al,not OPTION_DATA
	out	dx,al
	sahf				;restore cy.
	ret

	include	8390.asm

	public	usage_msg
usage_msg	db	"usage: hppclan [-n] [-d] [-w] <packet_int_no> [<int_no> [<io_addr>]]",CR,LF

	public	copyright_msg
copyright_msg	db	"Packet driver for HP PC LAN cards, version "
		db	'0'+majver,".",'0'+version,".",'0'+dp8390_version,CR,LF,'$'

no_hp_msg	db	"No HP PC LAN card found (matching io_addr or int_no if spedified).",CR,LF,'$'
cfg_err_msg	db	"Configuration failed. Check parameters.",CR,LF,'$'
int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'

comment \

Here are the ID byte values for all released cards.  Bit 7 of the ID byte
for AT cards indicates bus width (as well as RAM buffer size).

The ID byte was added to give a unique number to every revision of every card
produced.  However, it is a little more than just a flat assignment; there is
some fields.  For driver software it is assumed the driver knows what bus it
is on (eg. AT verses MC):

The format of the ID byte (offset 0x07 from I/O base) is:

        7  6  5  4  3  2  1  0
        a  b  b  b  c  c  c  c
where,
        "a"   is the AT bus width bit
         0 =  8 bit AT cards (32K bytes of buffer RAM)
         1 = 16 bit AT cards (64K bytes of buffer RAM)

        "bbb" is for board revisions

        "ccc" is the LAN Media type
         000 = HP StarLAN-10
         001 = 10-Base-T (Ethertwist)
         002 = 10-Base-2 (ThinLAN)
         ??? = reserved for future

The current ID bytes assigned are:
        0x00 = (obsolete) HP27240 AT8  StarLAN
        0x10 = (obsolete) HP24240 AT8  StarLAN rev B
        0x00 = (obsolete) HP27241 MC16 StarLAN
        0x01 = HP27245 AT8  10-Base-T
        0x01 = HP27246 MC16 10-Base-T
        0x02 = HP27250 AT8  ThinLAN
        0x81 = HP27247 AT16 10-Base-T

NOTE:   All Microchannel cards are 16 bits wide.
        All 8 bit cards have 32K bytes of buffer RAM.
        All 16 bit cards (AT & MC) have 64K bytes of buffer RAM.
\

this_board_msg	db	"This board is an HP",'$'

ISA_name_list	dw	board_00ISA, board_10, board_01ISA, board_02, board_81
		dw	board_unk
MCA_name_list	dw	board_00MCA, board_01MCA
		dw	board_unk

board_00ISA	db	00h, "27240",0,60	;AT8  StarLAN
board_10	db	10h, "24240",0,60	;AT8  StarLAN rev B
board_00MCA	db	00h, "27241",0,60	;MC16 StarLAN
board_01ISA	db	01h, "27245",0,60	;AT8  10-Base-T
board_01MCA	db	01h, "27246",0,60	;MC16 10-Base-T
board_02	db	02h, "27250",0,60	;AT8  ThinLAN
board_81	db	81h, "27247",0,60	;AT16 10-Base-T
board_unk	db	-1h, "Unknown HP272xx",0,60	;just a guess.

; ISA DEFAULTS for io_addr and int_no,
; MCA DEFAULTS to first found.

DEF_IO_ADDR	equ	0300h	
DEF_INT_NO	equ	3

MC_DEF_IO	equ	1
MC_DEF_INT	equ	2
MC_DEF_BOTH	equ	MC_DEF_IO+MC_DEF_INT

mc_def_req	db	0

int_no_table	db	3, 4, 5, 7, 9, 10, 11, 12

; First 3 bytes of the LAN Addres will identify the board as HP

hp_vendor_id	db	08h,00h,09h

	public	parse_args
parse_args:
;exit with nc if all went well, cy otherwise.
	test	sys_features,MICROCHANNEL
	jne	parse_args_mc
	jmp	parse_args_1

parse_args_mc:

; first determine if non-default values have been requested
; get_number will return requested value, -1, or carry flag set

	mov	di,offset int_no
	call	get_number
	mov	BYTE PTR [di+1], 0
	mov	WORD PTR [di+2], 0
	jnc	parse_args_mc2
	mov	BYTE PTR [di], 0ffh	; error return, use -1 value

parse_args_mc2:
	mov	di,offset io_addr
	call	get_number
	mov	WORD PTR [di+2], 0
	jnc	parse_args_mc3
	mov	WORD PTR [di], 0ffffh	; error return, use -1 value

parse_args_mc3:
	
	xor	al,al
	cmp	io_addr,0ffffh
	jne	parse_args_mc4
	mov	al,MC_DEF_IO

parse_args_mc4:
	cmp	int_no,0ffh
	jne	parse_args_mc5
	or	al,MC_DEF_INT

parse_args_mc5:
	mov	mc_def_req,al

;The following code to read the POS registers is courtesy of Racal-Interlan.

; channel selector resides at io 96h
; POS register base is at io 100h

; search thro' the slots for a 9210 card
	mov	cx, 8			; for all channels(slots)

; channel select value for slots 0,1,2.. is 8,9,A etc
; start with slot 0, and then 7,6,5,4,3,2,1

get_01:
	mov	ax, cx			; channel number
	or	ax, 08h			; reg. select value
	mov	chan_sel,al		; save each one, the LAST will right
	mov	dx, 96h			; channel select register
	out	dx, al			; select channel

	mov	dx, 94h			; protect system board
	mov	al, 0a0h
	out	dx, al			

; read adapter id
	mov	dx, 100h
	in	al, dx			; adapter id - ms byte
	mov	ah, al
	inc	dx
	in	al, dx			; adapter id - ls byte

; Check if HP
	cmp	ax, 0ca63h
	je	get_03
	
get_02:
	; come back to here if card found does not match user 
	; requirements below

	loop	get_01

	mc_chan_off
	mov	dx,offset no_hp_msg
	jmp	error

get_03:		; found an HP adapter, is it the right one?

comment \

The only POS registers defined for the Microchannel card (HP27246) are:

        0x100 = Adapter Identification LSB (0xCA) Read only
        0x101 = Adapter Identification MSB (0x63) Read only
        0x102 = Option Register Read/Write

The Option register bits are defined as:

        7 6 5 4 3 2 1 0
        a a a - b b b c

where,
        "aaa" is the I/O Base Address Relocation bits
         000 = 0x0400 to 0x041f
         001 = 0x1400 to 0x141f
         010 = 0x2400 to 0x241f
         011 = 0x3400 to 0x341f
         100 = 0x4400 to 0x441f
         101 = 0x5400 to 0x541f
         110 = 0x6400 to 0x641f
         111 = 0x7400 to 0x741f


        "bbb" is the Interrupt Level bits
         000 = INT 3
         001 = INT 4
         010 = INT 5
         011 = INT 7
         100 = INT 9
         101 = INT 10
         110 = INT 11
         111 = INT 12

        "c" is the Card Enable / NIC Reset bit
         0 = NIC held in reset (eg. cannot access its registers)
         1 = NIC not reset (eg. regster can be accessed)
         NOTE: Access to the PROM should only occur while NIC is held in reset.

\

	mov	dx,102h
	in	al,dx
	and	ax,00e0h		;leave only the base address bits
	push	cx
	mov	cl,12-5			;now ends at bit 5, should end at bit 12
	shl	ax,cl
	pop	cx
	or	ax,400h
	mov	di,ax		; temporary save

	in	al,dx
	shr	al,1
	and	al,7
	mov	bl,al
	xor	bh,bh
	mov	al,int_no_table[bx]

	; check for matches with user selects

	mov	ah,mc_def_req
	test	ah,MC_DEF_IO
	jz	get_04
	mov	io_addr,di	; user wants default, copy temp address

get_04:
	test	ah,MC_DEF_INT
	jz	get_05
	mov	int_no,al	; user wants default, copy temp int number

get_05:	
	cmp	di,io_addr
	je 	get_06
	jmp	get_02		; correct io_addr not found, keep looking

get_06:	
	cmp	al,int_no
	je 	get_07
	jmp	get_02		; correct io_addr not found, keep looking

get_07:		; if we get here we now have the io_addr and int_no
		; that the user wanted
	mc_chan_off
	jmp parse_args_5

;
; Much simpler code for ISA machine parse_args
;
parse_args_1:
	mov	di,offset int_no
	call	get_number
	jc	parse_args_2
	cmp	WORD PTR [di],0
	jl	parse_args_2
	jmp	parse_args_3

parse_args_2:
	mov	WORD PTR [di],DEF_INT_NO
	mov	WORD PTR [di+2],0

parse_args_3:
	mov	di,offset io_addr
	call	get_number
	jc	parse_args_4
	cmp	WORD PTR [di],0
	jl	parse_args_4
	jmp	parse_args_5

parse_args_4:
	mov	WORD PTR [di],DEF_IO_ADDR
	mov	WORD PTR [di+2],0

parse_args_5:	;Finally verify HP Vendor ID in 1st 3 bytes of station addr
	loadport
	setport	HP_PROM
	mov	di,offset hp_vendor_id
	mov	cx,3
id_card_loop:
	in	al,dx
	cmp	al, BYTE PTR [di]
	je	id_card_loop1
	mov	dx,offset no_hp_msg
	jmp	error

id_card_loop1:
	inc	di
	inc	dx
	loop	id_card_loop
	clc
	ret


	extrn	etopen_diagn: byte

init_card:
;Read PROM contents
	loadport
	setport	HP_PROM
	mov	di,offset curr_hw_addr
	mov	ax,cs
	mov	es,ax
	mov	cx,EADDR_LEN
	xor	ah,ah
init_card_1:
	in	al,dx
	add	ah,al
	inc	dx
	stosb
	loop	init_card_1
	in	al,dx			;now get the checksum.
	xor	ah,al			;add it in.
	cmp	ah,-1			;now it should have all ones set.
;ugh.  The checksum was botched on some boards, so we can't use it.
;	jne	init_card_2		;did it match?  Nope, return error.
	inc	dx
	in	al,dx			;get the hardware revision number.

	test	sys_features,MICROCHANNEL	;ISA or MCA?
	jne	is_16bit		;MCA - all MCA cards are 16-bit cards.
	test	al,80h			;16-bit system?
	je	check_16bit
is_16bit:
	or	endcfg,ENDCFG_WTS	;yes, use word transfer mode.

check_16bit:

	mov	bx,offset MCA_name_list		;assume MCA
	test	sys_features,MICROCHANNEL	;ISA or MCA?
	jne	check_board_name		;MCA.
	mov	bx,offset ISA_name_list		;otherwise ISA.
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
	mov	di,offset driver_name+3
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

	clc
	ret
init_card_2:
	stc
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
