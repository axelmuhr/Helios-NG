version	equ	1

;History:25,1 0

; Copyright, 1990, Russell Nelson

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

;some generic information about buffer lengths, etc
XMITBUF_LEN	equ	800h		;Transmit buffer length
RPAGE_LEN	equ	128		;receive page is 128 bytes
NUM_RPAGES	equ	96		;There are 96 receive pages
NUM_XBUFS	equ	2		;Number of xmit buffers
NUM_XBUFS_MASK	equ	1

ub_seg	segment at 0

	org	10h
eaddr_rom	db	6 dup(?)	;Has serial # in 3 & 4th bytes

	org	2080h
tsa_msb		db	?	;transmitter start address, high order 4
				;bits on write, start send on read
tsa_lsb		db	?	;Transmitter start address, least
				;significant byte on write, clear PAV
				;on read

;Control / status register (CSR)

csr		db	?	;Control on write, status on read port

;Command bits
TxRdyIntEn	equ	080h		;allow TxRdy interrupts
PAVIntEn	equ	040h		;Allow PAV to gen. intr.
SFTInt		equ	020h		;generate an interrupt reg.
TimIntEn	equ	010h		;allow Time interrupt req.
;status bits
NTxRdyInt	equ	080h		;Not req. TxRdy inter.
NPAVInt		equ	040h		;Not req. Pav intr.
NSFTInt		equ	020h		;Not req. SFTInt inter.
NTimInt		equ	010h		;Not req. timer inter.
NRINT		equ	008h		;Not req. recv inter.
NTINT		equ	004h		;Not req. xmit inter.
TPOK		equ	002h		;Packet okay of some type
TXDONE		equ	001h		;Transmit done

;Page pointer register definitions

ffp		label	byte		;ffp - first free page register
epp		label	byte		;epp - empty page pointer register
pp		db	?		;ffp on write/epp on read


;ffp bits
NIC_IE		equ	080h		;NIC interrupts enable

;epp bits
PAV		equ	080h		;Page available
RPAGE_BITS	equ	07fh


	org	2100h
page_ram	db	NUM_RPAGES dup(?);Receiver buffer pages

	org	2180h
tstatus		db	?		;Transmitter status 80

;Transmit status bits - EDLC
TX_READY	equ	080h		;
TX_BUSY		equ	040h		;Carrier detect
TX_TPR		equ	020h		;Self received packet sent
TX_SHORTED	equ	010h		;Carrier lost
TX_UF		equ	008h		;Underflow
TX_COL		equ	004h		;Collision
TX_C16		equ	002h		;16 collisions
TX_PE		equ	001h		;Parity error

TXERRMASK	equ	TX_PE or TX_C16 or TX_COL or TX_UF or TX_SHORTED or TX_BUSY

tmask		db	?		;Transmitter mask 81
rstatus		db	?		;Receiver status
rmask		db	?		;Receiver mask 83
tmode		db	?		;Transmitter mode 84
rmode		db	?		;Receiver mode 85
reset		db	?		;Reset line - high on bit 7, 86
tdrlsb		db	?		;TDR lsb  87
eaddr_ram	db	6 dup(?)	;current Ethernet address.
		db	?		;Reserved 8e
tdrmsb		db	?		;TDR msb 8f

	org	4000h
receive_buf	label	byte

	org	7000h
transmit_buf_1	label	byte

	org	7800h
transmit_buf_2	label	byte

ub_seg	ends

code	segment	word public
	assume	cs:code, ds:code

	public	int_no
int_no		db	3,0,0,0		;must be four bytes long for get_number.
base_addr	dw	0d000h,0

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	8		;from the packet spec
driver_name	db	'PC-NIC',0	;name of the driver.
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

	public	rcv_modes
rcv_modes	dw	7		;number of receive modes in our table.
		dw	0               ;There is no mode zero
		dw	rcv_mode_1
		dw	0		;don't want to bother.
		dw	rcv_mode_3
		dw	0		;haven't set up perfect filtering yet.
		dw	rcv_mode_5
		dw	rcv_mode_6

current_page	db	0		;current receiver page

active_buffer	dw	transmit_buf_1
inactive_buffer	dw	transmit_buf_2

	include	movemem.asm

	public	as_send_pkt
; The Asynchronous Transmit Packet routine.
; Enter with es:di -> i/o control block, ds:si -> packet, cx = packet length,
;   interrupts possibly enabled.
; Exit with nc if ok, or else cy if error, dh set to error number.
;   es:di and interrupt enable flag preserved on exit.
as_send_pkt:
	ret

	public	drop_pkt
; Drop a packet from the queue.
; Enter with es:di -> iocb.
drop_pkt:
	assume	ds:nothing
	ret

	public	xmit
; Process a transmit interrupt with the least possible latency to achieve
;   back-to-back packet transmissions.
; May only use ax and dx.
xmit:
	assume	ds:nothing
	ret


	public	send_pkt
send_pkt:
;enter with es:di->upcall routine, (0:0) if no upcall is desired.
;  (only if the high-performance bit is set in driver_function)
;enter with ds:si -> packet, cx = packet length.
;if we're a high-performance driver, es:di -> upcall.
;exit with nc if ok, or else cy if error, dh set to error number.
	assume	ds:nothing

	mov	es,base_addr
	assume	es:ub_seg

	cmp	cx,GIANT		; Is this packet too large?
	ja	send_pkt_toobig

	cmp	cx,RUNT			; minimum length for Ether
	jnb	oklen
	mov	cx,RUNT			; make sure size at least RUNT
oklen:

	mov	di,XMITBUF_LEN		;compute where we're going to
	sub	di,cx			;  be putting the packet.

;games above ensure that the last byte of data to go out is at the
;end of the ub xmit buffer, pos is really the offset from the
;start of that buffer

	mov	ax,active_buffer	;swap the buffer pointers.
	xchg	ax,inactive_buffer
	mov	active_buffer,ax
	add	di,ax
	push	di
	call	movemem
	pop	di

;Wait for previous packet to finish first
;could do this with interrupts, etc, but with only 2 xmit buffers
;doesn't seem worth the effort

send_pkt_1:
	mov	al,csr
	and	al,TPOK or TXDONE
	cmp	al,TPOK or TXDONE
	jne	send_pkt_1

	mov	al,tstatus		;get xmiter status ...
	mov	tstatus,0fh		;... and clear status bits

	test	al,TXERRMASK		;any errors?
	je	send_pkt_2
	call	count_out_err		;yes, count them.
send_pkt_2:

;all done above, we can play with TSA now!
;give offset

	mov	ax,di
	mov	tsa_lsb,al
	mov	tsa_msb,ah

	cmp	al,tsa_msb		;Start the send in action (SIDEEFFECT)

	clc
	ret
send_pkt_toobig:
	mov	dh,NO_SPACE
	stc
	ret

	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	cmp	cx,EADDR_LEN		;make sure that we have enough room.
	jb	get_address_2

	push	ds
	mov	ds,base_addr
	mov	si,offset eaddr_rom
	mov	cx,EADDR_LEN
	rep	movsb
	pop	ds

	mov	cx,EADDR_LEN
	clc
	ret
get_address_2:
	stc
	ret


	public	set_address
set_address:
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
	assume	ds:nothing
	cmp	cx,EADDR_LEN		;ensure that their address is okay.
	je	set_address_4
	mov	dh,BAD_ADDRESS
	stc
	jmp	short set_address_done
set_address_4:

	mov	es,base_addr
	mov	di,offset eaddr_ram
	rep	movsb
;	call	initialize		;initialize with our new address.

set_address_okay:
	mov	cx,EADDR_LEN		;return their address length.
	clc
set_address_done:
	push	cs
	pop	ds
	assume	ds:code
	ret


;	0 - accept no packets
rcv_mode_1:
	mov	es,base_addr
	assume	es:ub_seg

	mov	rmode,0
	ret

rcv_mode_3:
;	1 - accept node id packets, multicasts which match first three
;		bytes of node id, and broadcasts
	mov	es,base_addr
	assume	es:ub_seg

	mov	rmode,1
	ret

;	2 - accept node id packets, all multicasts and broadcasts
rcv_mode_5:
	mov	es,base_addr
	assume	es:ub_seg

	mov	rmode,2
	ret

;	3 - take all, promiscous
rcv_mode_6:
	mov	es,base_addr
	assume	es:ub_seg

	mov	rmode,3
	ret

	public	set_multicast_list
set_multicast_list:
;enter with ds:si ->list of multicast addresses, cx = number of addresses.
;return nc if we set all of them, or cy,dh=error if we didn't.
	mov	dh,NO_MULTICAST
	stc
	ret


	public	terminate
terminate:
	mov	es,base_addr
	assume	es:ub_seg

	mov	ffp,0			;set ffp to zero - disabling PAV
	mov	csr,0			;Turn off interrupt masks here and ...
	mov	tmask,0			;reset edlc interrupt masks
	mov	rmask,0
	mov	reset,80h		;Turn on reset line

	ret

	public	reset_interface
reset_interface:
;reset the interface.
	assume	ds:code
	ret


;called when we want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type, dl = packet class.
	extrn	recv_find: near

;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
	extrn	recv_copy: near

	extrn	count_in_err: near
	extrn	count_out_err: near


	public	recv
recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
	assume	ds:code

	mov	es,base_addr
	assume	es:ub_seg

recv_3:
	test	epp,PAV
	jne	recv_4
	jmp	recv_2			;yes.
recv_4:

;checking against epp is a pain actually...
	xor	cx,cx			;start with zero length
	mov	bl,current_page		;get the first page.
	xor	bh,bh
	mov	dx,bx			;remember the first page.
count_pages_1:
	mov	al,page_ram[bx]		;Get page descriptor
	and	al,RPAGE_BITS		;Only page offset bits PLEASE
	inc	al			;Add 1 to offset to make length
	xor	ah,ah
	add	cx,ax			;Add in current page length

	mov	al,page_ram[bx]		;Get page descriptor
	inc	bx			;go to the next page
	cmp	bx,NUM_RPAGES		;wrap around if necessary.
	jb	count_pages_2
	xor	bx,bx
count_pages_2:

	test	al,80h			;End page?
	je	count_pages_1		;yes, go add this one's length.

	mov	current_page,bl

	mov	ax,dx			;multiply dx by RPAGE_LEN.
	xchg	ah,al
	ror	ax,1
	mov	di,ax
	add	di,offset receive_buf	;get the starting address.
	add	di,EADDR_LEN+EADDR_LEN	;skip the ethernet addreses and
					;  point to the packet type.
	push	bx
	push	cx
	push	dx

	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	inc	di			;set di to 802.2 header
	inc	di
	mov	dl, IEEE8023
BlueBookPacket:

	call	recv_find		;look up our type.
	assume	es:nothing
	pop	dx
	pop	cx
	pop	bx

	mov	ax,es			;is this pointer null?
	or	ax,di
	je	just_discard		;yes - just free the frame.

	push	cx
	push	di
	push	es

	cmp	dx,bx			;did we wrap around (first >last?)
	jbe	no_wrap			;no, just do one move.

;we wrapped around the buffer, so we have to move in two halves.
;move the first part.
	mov	ax,dx			;multiply dx by RPAGE_LEN.
	xchg	ah,al
	ror	ax,1
	mov	si,ax
	add	si,offset receive_buf	;get the starting address.

	mov	ax,NUM_RPAGES		;Compute the number of pages to move.
	sub	ax,dx
	xchg	ah,al			;convert to pages (*RPAGE_LEN).
	ror	ax,1
	cmp	cx,ax			;are we moving less than that?
	jbe	no_wrap			;yes, just move it.
	sub	cx,ax			;reduce the total count by this count.
	push	cx			;preserve the second part's count
	mov	cx,ax			;get the first part's count.

	push	ds
	mov	ds,base_addr
	assume	ds:nothing
	call	movemem
	assume	ds:code
	pop	ds

	pop	cx			;get back the count

	xor	dx,dx

no_wrap:
	mov	ax,dx			;multiply dx by 128.
	xchg	ah,al
	ror	ax,1
	mov	si,ax
	add	si,offset receive_buf
	mov	ds,base_addr
	assume	ds:nothing
	call	movemem
	assume	ds:code

	pop	ds
	pop	si
	pop	cx
	call	recv_copy
	push	cs
	pop	ds

just_discard:
	mov	es,base_addr
	assume	es:ub_seg
	mov	al,epp			;Any more packets left?
	and	al,RPAGE_BITS
	cmp	al,current_page
	je	recv_5
	jmp	recv_3
recv_5:

	cmp	al,tsa_lsb		;Clearing pav bit (SIDEEFFECT)

recv_2:

;Ack the interrupt by turning bit on and off
	mov	al,current_page		;Set first free page pointer
	mov	ffp,al
	or	al,NIC_IE		;set interrupt bit, was off
	mov	ffp,al
recv_1:
	ret


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret


;any code after this will not be kept after initialization.
end_resident	label	byte


	public	usage_msg
usage_msg	db	"usage: ubnic [-n] [-d] [-w] <packet_int_no> <int_no> <base_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for the UB PC/NIC, version ",'0'+majver,".",'0'+version,CR,LF
		db	'$'

int_no_name	db	"Interrupt number ",'$'
base_addr_name	db	"Memory address ",'$'
our_address	db	6 dup(?)	;temporarily hold our address

	extrn	set_recv_isr: near

;enter with si -> argument string, di -> wword to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with si -> argument string, di -> wword to print.
	extrn	print_number: near

	public	parse_args
parse_args:
;exit with nc if all went well, cy otherwise.
	mov	di,offset int_no
	call	get_number
	mov	di,offset base_addr
	call	get_number
	clc
	ret


	public	etopen
etopen:
	cli

	cmp	base_addr,-1		;did they ask for auto-configure?
	jne	no_auto_config

	mov	bx,8000h		;yes, they *could* address it there.
auto_config_1:
	mov	es,bx
	assume	es:ub_seg
	cmp	word ptr es:eaddr_rom[0],00h + 0ddh * 256
	jne	auto_config_2
	cmp	byte ptr es:eaddr_rom[2],01h
	je	auto_config_3
auto_config_2:
	add	bx,800h			;move up by 32K
	jnc	auto_config_1		;go if we didn't overflow.
	sti
	stc				;say that we couldn't init.
	ret

auto_config_3:
	mov	base_addr,bx		;remember where we found it.
	mov	base_addr+2,0		;and nuke the rest of the -1.

no_auto_config:
	mov	es,base_addr
	assume	es:ub_seg

	mov	reset,80h		;Turn on reset line to tie down NIC
	mov	rmode,0			;no packets for now
	mov	tmode,0			;Turn on loopback

	mov	al,epp			;start ffp at epp - eliminates bogus ....
	and	al,07fh
	mov	current_page,al		;... packets if we've been started before
	mov	ffp,al			;Make sure interrupt enable is dead

	cmp	al,tsa_lsb		;Clearing pav bit (SIDEEFFECT)

	mov	tstatus, 0fh		;Clear xmit status bits
	mov	tmask,0			;No xmit intrs
	mov	rmask,0			;'Packet okay' only?
	mov	rstatus,0ffh		;Clear receiver status bits

	push	ds
	pop	es
	mov	di,offset our_address
	mov	cx,EADDR_LEN
	call	get_address
	mov	si,offset our_address
	mov	cx,EADDR_LEN
	call	set_address

;Since first packet will get sent no matter what we do, let's make it
;something everyone (include me) will ignore
;set length to zero

	mov	tsa_lsb,0ffh		;Get last 8 bits
	mov	tsa_msb,0fh		;and top 4 bits

	mov	reset,0			;Clear reset - expect packet to go
;Wait for packet to go!
etopen_1:
	mov	al,csr
	and	al,TPOK or TXDONE
	cmp	al,TPOK or TXDONE
	jne	etopen_1

	mov	tstatus,0fh		;clear xmiter status
	mov	tmode,0ah		;Clear loopbk, set parity enable

	mov	csr,0			;make sure this interrupt was off first
	mov	csr,PAVIntEn		;Page frame available interrupt only
	mov	al,current_page		;Set first free page register ...
	or	al,NIC_IE
	mov	ffp,al			;... and make sure interrupts enabled

	mov	al,epp			;get epp again
	test	al,PAV
	je	etopen_2
	cmp	al,tsa_lsb		;Try clearing pav bit (SIDEEFFECT)
etopen_2:

;enable receiving now
;	1 - accept node id packets, multicasts which match first three
;		bytes of node id, and broadcasts
	mov	rmode,1

;
; Now hook in our interrupt
;
	call	set_recv_isr

	sti

	mov	al, int_no		; Get board's interrupt vector
	add	al, 8
	cmp	al, 8+8			; Is it a slave 8259 interrupt?
	jb	set_int_num		; No.
	add	al, 70h - 8 - 8		; Map it to the real interrupt.
set_int_num:
	xor	ah, ah			; Clear high byte
	mov	int_num, ax		; Set parameter_list int num.

;if all is okay,
	mov	dx,offset end_resident
	clc
	ret
;if we got an error,
	stc
	ret

	public	print_parameters
print_parameters:
;echo our command-line parameters
	mov	di,offset int_no
	mov	dx,offset int_no_name
	call	print_number
	mov	di,offset base_addr
	mov	dx,offset base_addr_name
	call	print_number
	ret

code	ends

	end
