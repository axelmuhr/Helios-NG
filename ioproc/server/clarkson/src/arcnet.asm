;History:132,1
;Tue Feb 27 10:56:15 1990 send_pkt wasn't timing out properly.
version	equ	0

	include	defs.asm

;Ported from Philip Prindeville's arcnet driver for PCIP
;by Russell Nelson.  Any bugs are due to Russell Nelson.

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


;Registers:

;the following I/O addresses are mapped to the COM 9026
IMASK		equ	0		; writeable
STATUS		equ	0		; readable
COMMAND		equ	1
;the following I/O addresses are mapped to the 8253 counter/timer
CNTR0		equ	4
CNTR1		equ	5
CNTR2		equ	6
MODE		equ	7
;reading the following I/O addresse performs a software reset.
SW_RST		equ	8

; time needed to do various things (in clock ticks)
RST_IVAL	equ	4		;reset
SEND_IVAL	equ	4		;send

; ARP type for ARCnet
ARP_ARC		equ	007h

; broadcast address is nid 0
ARC_BCAST	equ	0

; packet sizes
ARC_MTU		equ	253
ARC_MnTU	equ	257
ARC_XMTU	equ	508
;
;status/interrupt mask bit fields
;
ST_TA		equ	001h		; transmitter available
ST_TMA		equ	002h		; transmitted msg. ackd
ST_RECON	equ	004h		; system reconfigured
ST_TEST		equ	008h		; test flag
ST_POR		equ	010h		; power-on-reset
ST_ETS1		equ	020h		; unused
ST_ETS2		equ	040h		; unused
ST_RI		equ	080h		; receiver inhibited

;
;in the command register, the following bits have these meanings:
;		0-2	command
;		3-4	page number (enable rvc/xmt)
;		 7	rcv b'casts


DSBL_XMT	equ	001h		; disable transmitter
DSBL_RCV	equ	002h		; disable receiver
ENBL_XMT	equ	003h		; enable transmitter
ENBL_RCV	equ	004h		; enable receiver
DFN_CONF	equ	005h		; define configuration
CLR_FLGS	equ	006h		; clear flags
LD_TST_FLG	equ	007h		; load test flags

; flags for clear flags operation

FL_POR		equ	008h		; power-on-reset
FL_RECON	equ	010h		; system reconfigured

; flags for load test flags operation

FL_TST		equ	008h		; test flag (diagnostic)

; byte deposited into first address of buffers when POR
TSTWRD		equ	0321Q

; handy macros for enable receiver/transmitter

BCAST		equ	080h		; receiver only
;PAGE		equ(nn)	((nn)<<3)

; flags for define configuration

CONF_NORM	equ	000h		; 1-253 byte packets
CONF_XTND	equ	008h		; 256-508 byte packets

; macros to access buffers
;BUF	equ(page)	(((custom.c_basemem) + 512 * (page)))

; ARCnet pseudo header -- note that syscode must occupy last byte...

arc_hdr	struc
arc_sid		db	?		; source, valid on rcv
arc_did		db	?		; destination, 0 = b'cast
arc_cp		db	?		; continuation pointer. zero
					; for extended packets
arc_xcp		db	?		; extended cp, see above
arc_syscode	db	?		; system code/pkt type
arc_hdr	ends

; designations for receiver/transmitter buffers.  sorry, no cleverness here
RCVPAGE		equ	2
XMTPAGE		equ	3

my_arcnet_id	db	0			;my arcnet ID.

	public	int_no
int_no		db	5,0,0,0		; interrupt number.
io_addr		dw	02e0h,0		; I/O address for card (jumpers)
mem_base	dw	0d800h,0

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	8,0		;ARCnet (from the packet spec)
driver_type	db	1		;Datapoint RIM (from the packet spec)
driver_name	db	'ARCnet',0	;name of the driver.
driver_function	db	2
parameter_list	label	byte
	db	1	;major rev of packet driver
	db	9	;minor rev of packet driver
	db	14	;length of parameter list
	db	1	;length of MAC-layer address
	dw	507	;MTU, including MAC headers
	dw	MAX_MULTICAST * EADDR_LEN	;buffer size of multicast addrs
	dw	0	;(# of back-to-back MTU rcvs) - 1
	dw	0	;(# of successive xmits) - 1
int_num	dw	0	;Interrupt # to hook for post-EOI
			;processing, 0 == none,

	public	rcv_modes
rcv_modes	dw	4		;number of receive modes in our table.
		dw	0,0,0,rcv_mode_3

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
;exit with nc if ok, or else cy if error, dh set to error number.
	assume	ds:nothing

;Wait for transmitter ready.
	loadport
	setport	STATUS

	mov	ax,SEND_IVAL		;only wait this long for it.
	call	set_timeout		;  otherwise we can't send.
send_pkt_3:
	in	al,dx			;if not busy, exit.
	and	al,ST_TA
	jne	send_pkt_2
	call	do_timeout		;did we time out yet?
	jne	send_pkt_3		;no, not yet.

	setport	COMMAND			;stop the transmit.
	mov	al,DSBL_XMT
	out	dx,al
	mov	dh,CANT_SEND		;timed out, can't send.
	stc
	ret

send_pkt_2:
;store the packet on the board.
	mov	es,mem_base
	mov	di,XMTPAGE * 512
	movsw				;move the SID and DID to the board.
	sub	cx,2			;leave them out of the count.

	cmp	cx,ARC_XMTU		;is this one too big?
	ja	send_pkt_toobig		;yes, can't store it.
	cmp	cx,ARC_MTU		;is this one small enough
	jbe	send_pkt_1		;yes, just move it in.
	cmp	cx,ARC_MnTU		;is it *too* large AND *too* small?
	jae	send_pkt_5		;no.
	mov	cx,ARC_MnTU		;yes - use the larger size.
send_pkt_5:
	xor	al,al			;use a zero cp to indicate xcp.
	mov	ah,cl			;store the length in xcp.
	neg	ah
	stosw
	jmp	short send_pkt_4
send_pkt_1:
	mov	al,cl			;store the length in cp.
	neg	al
	stosb
send_pkt_4:
	mov	ax,di			;continue the put.
	mov	al,cl			;  advance the cp to its proper place.
	neg	al
	mov	di,ax
	call	movemem

;start the transmit.
	mov	al,ENBL_XMT or (XMTPAGE shl 3)
	loadport
	setport	COMMAND
	out	dx,al

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
	cmp	cx,ARCADDR_LEN		;make sure that we have enough room.
	mov	dh,NO_SPACE
	jb	get_address_2

	mov	al,my_arcnet_id		;store our address.
	stosb

	mov	cx,ARCADDR_LEN
	clc
	ret
get_address_2:
	stc
	ret


;Set address on controller
	public	set_address
set_address:
	assume	ds:nothing
;enter with ds:si -> address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
	mov	dh,CANT_SET
	stc
	ret


rcv_mode_3:
;receive mode 3 is the only one we support, so we don't have to do anything.
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
	assume	ds:code
	loadport
	setport	IMASK
	mov	al,0
	out	dx,al

	setport COMMAND
	mov	al,DSBL_RCV
	out	dx,al
	mov	al,DSBL_XMT
	out	dx,al

	setport	STATUS			;do we need to do this [rnn]?
	in	al,dx

	ret


	public	reset_interface
reset_interface:
;reset the interface.
;we don't do anything.
	ret


	include	timeout.asm

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

recv_1:
	loadport			;get the status to see if we got
	setport	STATUS			;  a false alarm.
	in	al,dx
	test	al,ST_RI
	je	recv_2			;yup, exit now.

	mov	es,mem_base
	mov	bx,RCVPAGE * 512

;decode data size.

	mov	cx,256			;compute the actual length.
	mov	al,es:[bx].arc_cp
	or	al,al			;is this a normal or continuation pkt?
	jne	recv_3			;go if normal.
	mov	cx,512			;extended packets have 512 max.
	mov	al,es:[bx].arc_xcp
recv_3:
	xor	ah,ah
	sub	cx,ax
	add	cx,2			;add in SID and DID.
	mov	bl,al			;use al as the low byte of the address.
	mov	di,bx
	mov	dl,driver_class
	call	recv_find		;look up our type.

	mov	ax,es			;is this pointer null?
	or	ax,di
	je	recv_isr_9		;yes - just free the frame.

	push	es			;remember where the buffer pointer is.
	push	di

	mov	ds,mem_base		;copy the packet into their buffer.
	assume	ds:nothing
	mov	si,RCVPAGE * 512	;  (don't worry about ds.
	movsw				;move SID and DID.
	mov	ax,si
	lodsb				;get arc_cp.
	or	al,al			;extended?
	jne	recv_5			;no.
	lodsb				;yes - get arc_xcp.
recv_5:
	mov	si,ax			;set the new pointer.
	push	cx			;move the data part of the packet.
	sub	cx,2			;don't move the two we've already
	call	movemem			;  moved.
	pop	cx

	pop	si
	pop	ds
	assume	ds:nothing
	call	recv_copy		;tell them that we copied it.

	mov	ax,cs			;restore our ds.
	mov	ds,ax
	assume	ds:code

recv_isr_9:

	loadport			;enable reception again.
	setport	COMMAND
	mov	al,ENBL_RCV or (RCVPAGE shl 3) or BCAST
	out	dx,al

	jmp	recv_1
recv_2:
	ret


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret


end_resident	label	byte

	public	usage_msg
usage_msg	db	"usage: arcnet [-n] [-d] [-w] <packet_int_no> <int_no> <io_addr> <mem_base>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for the DataPoint RIM (ARCnet), version ",'0'+majver,".",'0'+version,CR,LF
		db	"Portions Copyright 1988 Philip Prindeville",CR,LF,'$'

no_arcnet_msg	db	"No ARCnet found at that address.",CR,LF,'$'
failed_test_msg	db	"Failed self test.",CR,LF,'$'

int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'
mem_base_name	db	"Memory address ",'$'

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
	mov	di,offset mem_base
	call	get_number
	clc
	ret


no_arcnet_error:
	mov	dx,offset no_arcnet_msg
	mov	ah,9
	int	21h
	jmp	short error
failed_test_error:
	mov	dx,offset failed_test_msg
	mov	ah,9
	int	21h
error:
	stc
	ret


	public	etopen
etopen:
;reset the board via the I/O reset port, then wait for it to become sane again.

	mov	ax,mem_base		;test the memory first.
	mov	cx,2048
	call	memory_test
	jne	no_arcnet_error

	mov	es,mem_base

	loadport
	setport SW_RST
	in	al,dx

	mov	ax,RST_IVAL
	call	set_timeout
etopen_1:
	call	do_timeout
	jne	etopen_1

	setport	STATUS
	in	al,dx

;since we've just reset:
;	reset the POR flag,
;	check the diagnostic byte in the buffer,
;	grab the node ID, and assign it to the host number.

	test	al,ST_POR
	je	etopen_2

	setport	COMMAND
	mov	al,CLR_FLGS or FL_POR or FL_RECON
	out	dx,al

	mov	al,es:[0]
	cmp	byte ptr es:[0],TSTWRD
	je	etopen_3
	jmp	failed_test_error	;failed power on self-test.
etopen_3:
	mov	al,es:[1]
	mov	my_arcnet_id,al
etopen_2:

;another simple diagnostic:
;	force test flag on in RIM,
;	check to see that it is set,
;	reset it.

	loadport
	setport	COMMAND
	mov	al,LD_TST_FLG or FL_TST
	out	dx,al

	setport STATUS
	in	al,dx

	test	al,FL_TST
	jne	etopen_4
	jmp	failed_test_error	;failed forced self-test.
etopen_4:
	setport	COMMAND
	mov	al,LD_TST_FLG
	out	dx,al
	setport STATUS
	in	al,dx

	pushf
	cli

	call	set_recv_isr

;now we enable the board to interrupt
;us on packet received.  Not transmiter available
;(i.e. transmission complete).  We don't have
;any control over POR, since it is NMI...
;RECON seems useless.

	loadport
	setport	IMASK
	mov	al,ST_RI
	out	dx,al

	; we should allow extended packets
	setport	COMMAND
	mov	al,DFN_CONF or CONF_XTND
	out	dx,al

	mov	al,ENBL_RCV or (RCVPAGE shl 3) or BCAST;
	out	dx,al

	popf

	mov	al, int_no		; Get board's interrupt vector
	add	al, 8
	cmp	al, 8+8			; Is it a slave 8259 interrupt?
	jb	set_int_num		; No.
	add	al, 70h - 8 - 8		; Map it to the real interrupt.
set_int_num:
	xor	ah, ah			; Clear high byte
	mov	int_num, ax		; Set parameter_list int num.

	mov	dx,offset end_resident
	clc
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

code	ends

	end
