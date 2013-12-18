version	equ	1

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

comment /

I found a problem when using the Interlan NI6510 Packet Driver with 16 bit DMA.
I looked at the source and I think I found the bug.  I've tested the fixed code
and it seems to work fine for all possible DMA settings, at least over here.

Anto Prijosoesilo,
Network & Microcomputer Consultant,
University of North Texas Computing Center,
Denton, Texas

/

	.286				;the 6510 requires a 286.

	include	defs.asm

DATA_REG	equ	0
ADDR_REG	equ	2
	CSR0		equ	0
	CSR1		equ	1
	CSR2		equ	2
	CSR3		equ	3
RESET		equ	4
CONFIG		equ	5
EBASE		equ	8

DMA_8MASK_REG	equ	0Ah
DMA_16MASK_REG	equ	0D4h

DMA_8MODE_REG	equ	0Bh
DMA_16MODE_REG	equ	0D6h

CASCADE_MODE      equ	0C0h
SET_DMA_MASK      equ	4
DMA_CHANNEL_FIELD equ	3


outport	macro	reg
	push	ax
	setport	ADDR_REG
	mov	ax,reg
	out	dx,ax
	in	ax,dx			;always follow a write by a read

	setport	DATA_REG
	pop	ax
	out	dx,ax
	in	ax,dx			;always follow a write by a read

	endm


;
; 	Control and Status Register 0 (CSR0) bit definitions
;
CSR0_ERR	equ 	8000h	; Error summary
CSR0_BABL	equ 	4000h	; Babble transmitter timeout error
CSR0_CERR	equ	2000h	; Collision Error
CSR0_MISS	equ	1000h	; Missed packet
CSR0_MERR	equ	0800h	; Memory Error
CSR0_RINT	equ	0400h	; Reciever Interrupt
CSR0_TINT       equ	0200h	; Transmit Interrupt
CSR0_IDON	equ	0100h	; Initialization Done
CSR0_INTR	equ	0080h	; Interrupt Flag
CSR0_INEA	equ	0040h	; Interrupt Enable
CSR0_RXON	equ	0020h	; Receiver on
CSR0_TXON	equ	0010h   ; Transmitter on
CSR0_TDMD	equ	0008h	; Transmit Demand
CSR0_STOP	equ	0004h 	; Stop
CSR0_STRT	equ	0002h	; Start
CSR0_INIT	equ	0001h	; Initialize

;
; 	Initialization Block  Mode operation Bit Definitions.
;
M_PROM		equ	8000h	; Promiscuous Mode
M_INTL		equ	0040h   ; Internal Loopback
M_DRTY		equ	0020h   ; Disable Retry
M_COLL		equ	0010h	; Force Collision
M_DTCR		equ	0008h	; Disable Transmit CRC)
M_LOOP		equ	0004h	; Loopback
M_DTX		equ	0002h	; Disable the Transmitter
M_DRX		equ	0001h   ; Disable the Reciever


;
; 	Receive message descriptor bit definitions.
;
RCV_OWN		equ	8000h	; owner bit 0 = host, 1 = lance
RCV_ERR		equ	4000h	; Error Summary
RCV_FRAM	equ 	2000h	; Framing Error
RCV_OFLO	equ	1000h	; Overflow Error
RCV_CRC		equ	0800h	; CRC Error
RCV_BUF_ERR	equ 	0400h	; Buffer Error
RCV_START	equ	0200h	; Start of Packet
RCV_END		equ	0100h	; End of Packet


;
;	Transmit  message descriptor bit definitions.
;
XMIT_OWN	equ	8000h	; owner bit 0 = host, 1 = lance
XMIT_ERR	equ	4000h   ; Error Summary
XMIT_RETRY	equ	1000h   ; more the 1 retry needed to Xmit
XMIT_1_RETRY	equ	0800h	; one retry needed to Xmit
XMIT_DEF	equ	0400h	; Deferred
XMIT_START	equ	0200h	; Start of Packet
XMIT_END	equ	0100h	; End of Packet

;
;	Miscellaneous Equates
;

TRANSMIT_BUF_COUNT	equ	1
RECEIVE_BUF_COUNT	equ	8
TRANSMIT_BUF_SIZE	equ	1518
RECEIVE_BUF_SIZE	equ	1518

;
;	Receive Message Descriptor
;
rcv_msg_dscp struc
	rmd0	dw	?	; Rec. Buffer Lo-Address
	rmd1	dw	?	; Status bits / Hi-Address
	rmd2	dw	?	; Buff Byte-length (2's Comp)
	rmd3	dw	?	; Receive message length
rcv_msg_dscp ends


;
;	Transmit Message Descriptor
;
xmit_msg_dscp struc
	tmd0	dw	?	; Xmit Buffer Lo-Address
	tmd1	dw	?	; Status bits / Hi-Address
	tmd2 	dw	?	; Buff Byte-length (2's Comp)
	tmd3	dw	?	; Buffer Status bits & TDR value
xmit_msg_dscp ends

code	segment	para public
	assume	cs:code, ds:code

	public	int_no
int_no	db	2,0,0,0			;must be four bytes long for get_number.
io_addr	dw	-1,-1
dma_no	db	?

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	35		;from the packet spec
driver_name	db	'NI6510',0	;name of the driver.
driver_function	db	2		;basic, extended
parameter_list	label	byte
	db	1	;major rev of packet driver
	db	9	;minor rev of packet driver
	db	14	;length of parameter list
	db	EADDR_LEN	;length of MAC-layer address
	dw	GIANT	;MTU, including MAC headers
	dw	MAX_MULTICAST * EADDR_LEN	;buffer size of multicast addrs
	dw	RECEIVE_BUF_COUNT-1	;(# of back-to-back MTU rcvs) - 1
	dw	TRANSMIT_BUF_COUNT-1	;(# of successive xmits) - 1
int_num	dw	0	;Interrupt # to hook for post-EOI
			;processing, 0 == none,

	public	rcv_modes
rcv_modes	dw	7		;number of receive modes in our table.
		dw	0               ;There is no mode zero
		dw	rcv_mode_1
		dw	0		;only ours.
		dw	rcv_mode_3	;ours plus broadcast
		dw	0		;some multicasts
		dw	rcv_mode_5	;all multicasts
		dw	rcv_mode_6	;all packets

;
;the LANCE requires that the descriptor pointers be on a qword boundary.
;
	align	8

transmit_dscps	xmit_msg_dscp	TRANSMIT_BUF_COUNT dup(<>)
receive_dscps	rcv_msg_dscp	RECEIVE_BUF_COUNT dup(<>)

transmit_head	dw	transmit_dscps	;->next packet to be filled by host.
receive_head	dw	receive_dscps	;->next packet to be filled by LANCE.

;
; 	 LANCE Initialization Block
;
	align	2
init_block		label	byte
init_mode		dw	0
init_addr		db	EADDR_LEN dup(?)	; Our Ethernet address
init_filter		db	8 dup(0)	;Multicast filter.
init_receive		dw	?,?		;Receive Ring Pointer.
init_transmit	  	dw	?,?	  	;Transmit Ring Pointer.

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
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
	assume	ds:nothing

	xor	bx,bx

	mov	ax,18
	call	set_timeout
send_pkt_1:
	test	transmit_dscps[bx].tmd1,XMIT_OWN	;Did the lance chip give it back?
	je	send_pkt_2
	call	do_timeout
	jne	send_pkt_1
	mov	dh,CANT_SEND
	stc
	ret
send_pkt_2:
;reset error indications.
	and	transmit_dscps[bx].tmd1,not (XMIT_ERR or XMIT_DEF or XMIT_1_RETRY or XMIT_RETRY)	;Did the lance chip give it back?
	mov	transmit_dscps[bx].tmd3,0	;reset all error bits.

	mov	ax,cx			;store the count.
	cmp	ax,RUNT			; minimum length for Ether
	ja	oklen
	mov	ax,RUNT			; make sure size at least RUNT
oklen:
	neg	ax
	mov	transmit_dscps[bx].tmd2,ax

	mov	ax,transmit_dscps[bx].tmd0	;store the packet.
	mov	dx,transmit_dscps[bx].tmd1
	call	phys_to_segmoffs
	rep	movsb

	or	transmit_dscps[bx].tmd1,XMIT_OWN	;give it to the lance chip.

;Inform LANCE that it should poll for a packet.
	loadport
	mov	ax,CSR0_INEA or CSR0_TDMD
	outport	CSR0
	clc
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
	loadport			; Get our Ethernet address base.
	setport	EBASE
	cld
get_address_1:
	insb				; get a byte of the eprom address
	inc	dx			; next register
	loop	get_address_1		; go back for rest
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

	push	cs
	pop	es
	mov	di,offset init_addr
	rep	movsb
	call	initialize		;initialize with our new address.

set_address_okay:
	mov	cx,EADDR_LEN		;return their address length.
	clc
set_address_done:
	push	cs
	pop	ds
	assume	ds:code
	ret


rcv_mode_1:
	mov	ax,M_DRX or M_DTX	;disable the receiver and transmitter.
	jmp	initialize_nomulti
rcv_mode_3:
	xor	ax,ax			;don't accept any multicast frames.
	call	initialize_multi
	mov	ax,0			;non-promiscuous mode
	jmp	short initialize_nomulti
rcv_mode_5:
	mov	ax,-1			;accept any multicast frames.
	call	initialize_multi
	mov	ax,0			;non-promiscuous mode
	jmp	short initialize_nomulti
rcv_mode_6:
	mov	ax,M_PROM	;promiscuous mode
initialize_nomulti:
initialize:
	mov	init_mode,ax
	loadport
	mov	ax,CSR0_STOP		;reset the INIT bit.
	outport	CSR0

	mov	ax,CSR0_INEA or CSR0_STRT or CSR0_INIT	;reinit and restart.
	outport	CSR0

	setport	DATA_REG

	mov	ax,36			;wait one second for the board
	call	set_timeout		;  to timeout.
initialize_1:
	in	ax,dx
	test	ax,CSR0_IDON
	jne	initialize_2
	call	do_timeout
	jne	initialize_1
	stc
	ret
initialize_2:
	clc
	ret


initialize_multi:
;enter with ax = value for all multicast hash bits.
	push	cs
	pop	es
	mov	di,offset init_filter
	mov	cx,8/2
	rep	stosw
	ret


	public	set_multicast_list
set_multicast_list:
;enter with ds:si ->list of multicast addresses, cx = number of addresses.
;return nc if we set all of them, or cy,dh=error if we didn't.
	mov	dh,NO_MULTICAST		;for some reason we can't do multi's.
	stc
	ret


	public	terminate
terminate:
	call	rcv_mode_1		;don't receive any apckets.

;This routine will remove the (host) DMA controller from
;cascade mode of operation.
	mov	al,dma_no
	or	al,SET_DMA_MASK
	cmp	dma_no,4		;If channel 5 or 6,
	ja	terminate_16		;  use sixteen bit dma.
terminate_8:
	out	DMA_8MASK_REG,al
	jmp	short terminate_done
terminate_16:
	out	DMA_16MASK_REG,al
terminate_done:
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

LANCE_ISR_ACKNOWLEDGE equ (CSR0_INEA or CSR0_TDMD or CSR0_STOP or CSR0_STRT or CSR0_INIT)

	public	recv
recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
	assume	ds:code

	loadport
	setport	ADDR_REG
	mov	ax,CSR0
	out	dx,ax
	in	ax,dx
	setport	DATA_REG
	in	ax,dx
	mov	bx,ax			;make a copy.

; Acknowledge the Interrupt from the controller, but disable further
; controller Interrupts until we service the current interrupt.
;
;(CSR0_INEA or CSR0_TDMD or CSR0_STOP or CSR0_STRT or CSR0_INIT)
;
	and	ax,not LANCE_ISR_ACKNOWLEDGE
	out	dx,ax
	in	ax,dx		; follow all writes by a read

	test	bx,CSR0_RINT		;receive interrupt?
	je	recv_done		;no, we're done.

	mov	bx,receive_head

recv_search:
	test	code:[bx].rmd1,RCV_OWN	;do we own this buffer?
	je	recv_own		;yes - process it.
	call	inc_recv_ring		;go to the next one.
	cmp	bx,receive_head		;did we get back to the beginning?
	jne	recv_search		;not yet.
	jmp	short recv_done		;yes -- spurious interrupt!
recv_own:
	test	code:[bx].rmd1,RCV_ERR	;Any errors in this buffer?
	jne	recv_err		;yes -- ignore this packet.

	mov	ax,code:[bx].rmd0	;fetch the packet.
	mov	dx,code:[bx].rmd1
	call	phys_to_segmoffs

	push	es
	push	di
	push	bx

	mov	cx,code:[bx].rmd3
	and	cx,0fffh		;strip off the reserved bits
	add	di,EADDR_LEN+EADDR_LEN	;skip the ethernet addreses and
					;  point to the packet type.
	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	inc	di			;set di to 802.2 header
	inc	di
	mov	dl, IEEE8023
BlueBookPacket:
	push	cx
	call	recv_find
	pop	cx

	pop	bx
	pop	si
	pop	ds
	assume	ds:nothing

	mov	ax,es			;is this pointer null?
	or	ax,di
	je	recv_free		;yes - just free the frame.

	push	es
	push	di
	push	cx
	rep	movsb
	pop	cx
	pop	si
	pop	ds
	assume	ds:nothing

	call	recv_copy

	jmp	short recv_free

recv_err:
	call	count_in_err
recv_free:
	push	cs
	pop	ds
	assume	ds:code

;clear any error bits.
	and	code:[bx].rmd1,not (RCV_ERR or RCV_FRAM or RCV_OFLO or RCV_CRC or RCV_BUF_ERR)
	or	code:[bx].rmd1,RCV_OWN	;give it back to the lance.
	call	inc_recv_ring		;go to the next one.
	test	code:[bx].rmd1,RCV_OWN	;Do we own this one?
	je	recv_own
	mov	receive_head,bx		;remember where the next one starts.
recv_done:
	loadport			;enable interrupts again.
	setport	DATA_REG
	mov	ax,CSR0_INEA
	out	dx,ax
	ret


inc_recv_ring:
;advance bx to the next receive ring descriptor.
	assume	ds:nothing
	add	bx,(size rcv_msg_dscp)
	cmp	bx,offset receive_dscps + RECEIVE_BUF_COUNT * (size rcv_msg_dscp)
	jb	inc_recv_ring_1
	mov	bx,offset receive_dscps
inc_recv_ring_1:
	ret


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret


phys_to_segmoffs:
;enter with dx:ax as the physical address of the buffer,
;exit with es:di -> buffer.
	shl	dx,16-4			;move the upper four bits into position.
	mov	di,ax			;now get the low 12 bits of the segment.
	shr	di,4
	or	dx,di			;combine them.
	mov	es,dx
	mov	di,ax
	and	di,0fh			;now compute the offset.
	ret

	include	timeout.asm

;we use this memory for buffers once we've gone resident.
transmit_bufs	equ	$
receive_bufs	equ	transmit_bufs + TRANSMIT_BUF_COUNT * TRANSMIT_BUF_SIZE
end_resident	equ	receive_bufs + RECEIVE_BUF_COUNT * RECEIVE_BUF_SIZE

	public	usage_msg
usage_msg	db	"usage: ni6510 [-n] [-d] [-w] <packet_int_no> <int_no> <io_addr>",CR,LF,'$'
no_board_msg	db	"No NI6510 detected.",CR,LF,'$'
io_addr_funny_msg	label	byte
		db	"No NI6510 detected, continuing anyway.",CR,LF,'$'
bad_reset_msg	db	"Unable to reset the NI6510.",CR,LF,'$'
bad_init_msg	db	"Unable to initialize the NI6510.",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for a Racal Interlan NI6510, version ",'0'+majver,".",'0'+version,CR,LF
		db	'$'

int_nos		db	9, 12, 15, 5	;interrupt numbers.
dma_nos		db	0, 3, 5, 6	;dma channel numbers

int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'

	extrn	set_recv_isr: near
	extrn	maskint: near

;enter with si -> argument string, di -> dword to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

	public	parse_args
parse_args:
;exit with nc if all went well, cy otherwise.
	assume	ds:code
	mov	di,offset int_no
	call	get_number
	mov	di,offset io_addr
	call	get_number
	clc
	ret


	public	etopen
etopen:
	assume	ds:code

	cmp	io_addr,-1		;Did they ask for auto-detect?
	je	find_board

	call	detect_board		;no, just verify its existance.
	je	find_board_found

	mov	dx,offset io_addr_funny_msg
	mov	ah,9
	int	21h

	jmp	find_board_found

find_board:
	mov	io_addr,300h		;Search for the Ethernet address.
	mov	io_addr+2,0
find_board_0:
	call	detect_board
	je	find_board_found
find_board_again:
	add	io_addr,20h		;not at this port, try another.
	cmp	io_addr,360h
	jbe	find_board_0

	mov	dx,offset no_board_msg	;Tell them that we can't find it.
	mov	ah,9
	int	21h

	stc
	ret
find_board_found:

	loadport			;get the configuration register and
	setport	CONFIG			;  determine the interrupt number.
	in	al,dx
	shr	al,2
	and	al,3
	mov	bl,al
	xor	bh,bh
	mov	al,int_nos[bx]
	mov	int_no,al

;This routine will put the (host) DMA controller into
;cascade mode of operation.

	in	al,dx			;get the dma channel field.
	and	al,3
	mov	bl,al
	xor	bh,bh
	mov	al,dma_nos[bx]
	mov	dma_no,al
	mov	ah,al			;save a copy.
	and	al,DMA_CHANNEL_FIELD
	or	al,SET_DMA_MASK
	cmp	ah,4			;If channel 5 or 6,
	ja	dma_16			;  use sixteen bit dma.
dma_8:
	out	DMA_8MASK_REG,al
	mov	al,ah
	or	al,CASCADE_MODE
	out	DMA_8MODE_REG,al
	mov	al,ah
	out	DMA_8MASK_REG,al
	jmp	short dma_done
dma_16:
	out	DMA_16MASK_REG,al
	and	ah,DMA_CHANNEL_FIELD
	mov	al,ah
	or	al,CASCADE_MODE
	out	DMA_16MODE_REG,al
	mov	al,ah
	out	DMA_16MASK_REG,al
dma_done:

	mov	al, int_no		; Get board's interrupt vector
	add	al, 8
	cmp	al, 8+8			; Is it a slave 8259 interrupt?
	jb	set_int_num		; No.
	add	al, 70h - 8 - 8		; Map it to the real interrupt.
set_int_num:
	xor	ah, ah			; Clear high byte
	mov	int_num, ax		; Set parameter_list int num.

	mov	al,int_no
	call	maskint			;disable these interrupts.

	loadport
	setport	RESET
	out	dx,al

	setport	CSR0
	in	ax,dx
	cmp	ax,CSR0_STOP
	je	reset_ok

	mov	dx,offset bad_reset_msg
	mov	ah,9
	int	21h

	stc
	ret
reset_ok:

;set up transmit descriptor ring.
	push	ds
	pop	es
	mov	cx,TRANSMIT_BUF_COUNT
	mov	bx,offset transmit_dscps
	mov	di,offset transmit_bufs
setup_transmit:
	call	segmoffs_to_phys

	or	dx,XMIT_START or XMIT_END
	mov	[bx].tmd0,ax		;points to the buffer.
	mov	[bx].tmd1,dx

	add	bx,(size xmit_msg_dscp)
	add	di,TRANSMIT_BUF_SIZE
	loop	setup_transmit

;set up receive descriptor ring.
	mov	cx,RECEIVE_BUF_COUNT
	mov	bx,offset receive_dscps
	mov	di,offset receive_bufs
setup_receive:
	call	segmoffs_to_phys

	or	dx,RCV_OWN
	mov	[bx].rmd0,ax		;points to the buffer.
	mov	[bx].rmd1,dx

	mov	[bx].rmd2,-RECEIVE_BUF_SIZE
	mov	[bx].rmd3,0

  if 0
	push	di			;initialize the buffers to 55aa.
	push	cx
	mov	cx,RECEIVE_BUF_SIZE/2
	mov	ax,55aah
	rep	stosw
	pop	cx
	pop	di
  endif

	add	bx,(size rcv_msg_dscp)
	add	di,RECEIVE_BUF_SIZE
	loop	setup_receive

;initialize the board.
	mov	cx,EADDR_LEN		;get our address.
	mov	di,offset init_addr
	call	get_address

	mov	cx,RECEIVE_BUF_COUNT
	call	compute_log2

	mov	di,offset receive_dscps
	call	segmoffs_to_phys
	or	dx,cx			;include the buffer size bits.
	mov	init_receive[0],ax
	mov	init_receive[2],dx

	mov	cx,TRANSMIT_BUF_COUNT
	call	compute_log2

	mov	di,offset transmit_dscps
	call	segmoffs_to_phys
	or	dx,cx			;include the buffer size bits.
	mov	init_transmit[0],ax
	mov	init_transmit[2],dx

	mov	di,offset init_block	;now tell the board where the init
	call	segmoffs_to_phys	;  block is.

	push	dx
	push	ax

	loadport
	mov	ax,0			;write the bus config register.
	outport	CSR3

	pop	ax			;write the low word.
	outport	CSR1

	pop	ax			;write the high word.
	outport	CSR2

	mov	ax,0			;non-promiscuous mode
	call	initialize
	jnc	init_ok

	mov	dx,offset bad_init_msg
	mov	ah,9
	int	21h

	stc
	ret

init_ok:
;
; Now hook in our interrupt
;
	call	set_recv_isr

	mov	dx,offset end_resident
	clc
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

compute_log2:
;enter with cx = number of buffers.
;exit with cx = log2(number of buffers) << 13.
	mov	ax,-1
compute_log2_1:
	inc	ax
	shr	cx,1
	jne	compute_log2_1
	shl	ax,13
	mov	cx,ax
	ret


segmoffs_to_phys:
;enter with es:di -> buffer.
;exit with dx:ax as the physical address of the buffer,

	mov	dx,es			;get the high 4 bits of the segment,
	shr	dx,16-4
	mov	ax,es			;and the low 12 bits of the segment.
	shl	ax,4
	add	ax,di			;add in the offset.
	adc	dx,0
	ret


detect_board:
;test to see if a board is located at io_addr.
;return nz if not.
	loadport
	setport	EBASE
	in	al,dx			;Check for Interlan's prefix word.
	cmp	al,2
	jne	detect_board_exit

	setport	EBASE+1
	in	al,dx
	cmp	al,7
	jne	detect_board_exit

	setport	EBASE+EADDR_LEN		;first byte following should be 0
	in	al,dx
	cmp	al,0
	jne	detect_board_exit

	setport	EBASE+EADDR_LEN+1	;second byte should be 55h
	in	al,dx
	cmp	al,55h
detect_board_exit:
	ret

code	ends

	end
