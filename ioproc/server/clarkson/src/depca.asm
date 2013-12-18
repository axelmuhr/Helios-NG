version	equ	0

	include	defs.asm

NI_CSR		equ	0
RBI		equ	2
DATA_REG	equ	4
ADDR_REG	equ	6
	CSR0		equ	0
	CSR1		equ	1
	CSR2		equ	2
	CSR3		equ	3
EBASE		equ	0ch

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

lance_seg	segment at 0

;
;the LANCE requires that the descriptor pointers be on a qword boundary.
;
	align	8

transmit_dscps	xmit_msg_dscp	TRANSMIT_BUF_COUNT dup(<>)
receive_dscps	rcv_msg_dscp	RECEIVE_BUF_COUNT dup(<>)

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

transmit_bufs	equ	$
receive_bufs	equ	transmit_bufs + TRANSMIT_BUF_COUNT * TRANSMIT_BUF_SIZE

lance_seg	ends

code	segment	para public
	assume	cs:code, ds:code

		public	int_no
int_no		db	2,0,0,0			;must be four bytes long for get_number.
io_addr		dw	-1,-1
base_addr	dw	-1,-1
depca_pattern	db	0FFh, 00h, 55h, 0AAh, 0FFh, 00h, 55h, 0AAh

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	66		;from the packet spec
driver_name	db	'DEPCA',0	;name of the driver.
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

transmit_head	dw	transmit_dscps	;->next packet to be filled by host.
receive_head	dw	receive_dscps	;->next packet to be filled by LANCE.

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
	mov	es,base_addr
	assume	es:lance_seg

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
	mov	transmit_dscps[bx].tmd2,ax	;store the negative of the cnt.

	mov	ax,transmit_dscps[bx].tmd0	;store the packet.
	mov	dx,transmit_dscps[bx].tmd1
	call	phys_to_segmoffs
	assume	es:nothing
	rep	movsb

	mov	es,base_addr
	assume	es:lance_seg
	or	transmit_dscps[bx].tmd1,XMIT_OWN	;give it to the lance chip.

;Inform LANCE that it should poll for a packet.
	loadport
	mov	ax,CSR0_INEA or CSR0_TDMD
	outport	CSR0
	clc
	ret


detect_board:
;test to see if a board is located at io_addr.
;setup to read first byte of ethernet address rom when successful.
;return nz if not.
	assume	cs:code, ds:code

	loadport
	setport	EBASE
	mov	si, offset depca_pattern
	lodsb				;get first char of test pattern
	mov	ah,al			;move string byte to ah
	mov	di,0
	mov	cx,32			;do 32 reps
detect_board_1:
	in	al,dx			;input byte
	cmp	ah,al
	jne	detect_board_2
	lodsb				;get next char
	mov	ah,al
	inc	di
	cmp	di, 8			;need to match eight chars
	je	detect_board_done
detect_board_2:
	loop	detect_board_1
	or	sp,sp			;guaranteed to be nonzero.
detect_board_done:
	ret


	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code, es:nothing
	cmp	cx,EADDR_LEN		;make sure that we have enough room.
	jb	get_address_2
	push	di
	call	detect_board
	pop	di
	mov	cx,EADDR_LEN
	loadport			; Get our Ethernet address base.
	setport	EBASE+1
	cld
get_address_1:
	in	al,dx			; get a byte of the eprom address
	stosb
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

	mov	es,base_addr
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
	mov	es,base_addr
	mov	es:init_mode,ax

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
	mov	es,base_addr
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
	jne	recv_RINT		;yes.
	jmp	recv_done		;no, we're done.
recv_RINT:

	mov	es,base_addr
	assume	es:lance_seg
	mov	bx,receive_head

recv_search:
	test	lance_seg:[bx].rmd1,RCV_OWN	;do we own this buffer?
	je	recv_own		;yes - process it.
	call	inc_recv_ring		;go to the next one.
	cmp	bx,receive_head		;did we get back to the beginning?
	jne	recv_search		;not yet.
	jmp	recv_done		;yes -- spurious interrupt!
recv_own:
	test	lance_seg:[bx].rmd1,RCV_ERR	;Any errors in this buffer?
	jne	recv_err		;yes -- ignore this packet.

	mov	cx,lance_seg:[bx].rmd3
	and	cx,0fffh		;strip off the reserved bits

	mov	ax,lance_seg:[bx].rmd0	;fetch the packet.
	mov	dx,lance_seg:[bx].rmd1
	call	phys_to_segmoffs
	assume	es:nothing

	push	es
	push	di
	push	bx

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
	assume	ds:nothing, es:nothing

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
	mov	es,base_addr
	assume	es:lance_seg

;clear any error bits.
	and	lance_seg:[bx].rmd1,not (RCV_ERR or RCV_FRAM or RCV_OFLO or RCV_CRC or RCV_BUF_ERR)
	or	lance_seg:[bx].rmd1,RCV_OWN	;give it back to the lance.
	call	inc_recv_ring			;go to the next one.
	test	lance_seg:[bx].rmd1,RCV_OWN	;Do we own this one?
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
	assume	ds:nothing
  if 0
	shl	dx,16-4			;move the upper four bits into position.
	mov	di,ax			;now get the low 12 bits of the segment.
	shr	di,4
	or	dx,di			;combine them.
	mov	es,dx
	mov	di,ax
	and	di,0fh			;now compute the offset.
  else
	mov	es,base_addr
	mov	di,ax
  endif
	ret

	include	timeout.asm

end_resident	label	byte

	public	usage_msg
usage_msg	db	"usage: depca [-n] [-d] [-w] <packet_int_no> <int_no> <io_addr> <mem_addr>",CR,LF,'$'
no_board_msg	db	"No DEPCA detected.",CR,LF,'$'
io_addr_funny_msg	label	byte
		db	"No DEPCA detected, continuing anyway.",CR,LF,'$'
bad_reset_msg	db	"Unable to reset the DEPCA.",CR,LF,'$'
bad_init_msg	db	"Unable to initialize the DEPCA.",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for a Digital Equipment Corporation DEPCA, version ",'0'+majver,".",'0'+version,CR,LF
		db	'$'

int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'
base_addr_name	db	"Memory address ",'$'

	extrn	set_recv_isr: near
	extrn	maskint: near

;enter with si -> argument string, di -> dword to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with si -> argument string, di -> dword to print.
	extrn	print_number: near

	public	parse_args
parse_args:
;exit with nc if all went well, cy otherwise.
	assume	ds:code
	mov	di,offset int_no
	call	get_number
	mov	di,offset io_addr
	call	get_number
	mov	di,offset base_addr
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
	mov	io_addr,300h		;Search for the Ethernet address at 300h
	mov	io_addr+2,0
	call	detect_board
	je	find_board_found
	mov	io_addr,200h		;Search at 200h
	call	detect_board
	je	find_board_found

	mov	dx,offset no_board_msg	;Tell them that we can't find it.
	mov	ah,9
	int	21h

	stc
	ret
find_board_found:

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
	mov	ax,CSR0_STOP		;reset the INIT bit.
	outport	CSR0

;set up transmit descriptor ring.
	mov	es,base_addr
	assume	es:lance_seg
	mov	cx,TRANSMIT_BUF_COUNT
	mov	bx,offset transmit_dscps
	mov	di,offset transmit_bufs
setup_transmit:
	call	segmoffs_to_phys

	or	dx,XMIT_START or XMIT_END
	mov	lance_seg:[bx].tmd0,ax		;points to the buffer.
	mov	lance_seg:[bx].tmd1,dx

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
	mov	lance_seg:[bx].rmd0,ax		;points to the buffer.
	mov	lance_seg:[bx].rmd1,dx

	mov	lance_seg:[bx].rmd2,-RECEIVE_BUF_SIZE
	mov	lance_seg:[bx].rmd3,0

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
	mov	ax,2			;write the bus config register.
	outport	CSR3

	pop	ax			;write the low word.
	outport	CSR1

	pop	ax			;write the high word.
	outport	CSR2

	xor	ax,ax			;don't accept any multicast frames.
	call	initialize_multi
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

	loadport
	setport	NI_CSR
	mov	al,82h			;disable ROM, enable the interrupt line
	out	dx,al

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
	mov	di,offset base_addr
	mov	dx,offset base_addr_name
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
	mov	cl,13
	shl	ax,cl
	mov	cx,ax
	ret


segmoffs_to_phys:
;enter with es:di -> buffer.
;exit with dx:ax as the physical address of the buffer,

  if 0	; The DEPCA doesn't use system memory, it uses its own.
	mov	dx,es			;get the high 4 bits of the segment,
	shr	dx,16-4
	mov	ax,es			;and the low 12 bits of the segment.
	shl	ax,4
	add	ax,di			;add in the offset.
	adc	dx,0
  else
	xor	dx,dx			;the offset is the only part of the
	mov	ax,di			;  address.
  endif
	ret


code	ends

	end
