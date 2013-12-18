version	equ	2

	include	defs.asm

;Ported from Bill Doster's il.c, a C-language driver for the Interlan NI5010
;by Russell Nelson.  Any bugs are due to Russell Nelson.
;Updated to version 1.08 Feb. 17, 1989 by Russell Nelson.

;   Copyright, 1988, 1989, Russell Nelson

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

; The various IE command registers
EDLC_XSTAT	equ	0		; EDLC transmit csr
EDLC_XCLR	equ	0		; EDLC transmit "Clear IRQ"
EDLC_XMASK	equ	1		; EDLC transmit "IRQ Masks"
EDLC_RSTAT	equ	2		; EDLC receive csr
EDLC_RCLR	equ	2		; EDLC receive "Clear IRQ"
EDLC_RMASK	equ	3		; EDLC receive "IRQ Masks"
EDLC_XMODE	equ	4		; EDLC transmit Mode
EDLC_RMODE	equ	5		; EDLC receive Mode
EDLC_RESET	equ	6		; EDLC RESET register
EDLC_TDR1	equ	7		; "Time Domain Reflectometry" reg1
EDLC_ADDR	equ	8		; EDLC station address, 6 bytes
					; 0Eh doesn't exist for r/w
EDLC_TDR2	equ	0fh		; "Time Domain Reflectometry" reg2
IE_GP		equ	10h		; GP pointer (word register)
					; 11h is 2nd byte of GP Pointer
IE_RCNT		equ	10h		; Count of bytes in rcv'd packet
					; 11h is 2nd byte of "Byte Count"
IE_MMODE	equ	12h		; Memory Mode register
IE_DMA_RST	equ	13h		; IE DMA Reset.  write only
IE_ISTAT	equ	13h		; IE Interrupt Status.  read only
IE_RBUF		equ	14h		; IE Receive Buffer port
IE_XBUF		equ	15h		; IE Transmit Buffer port
IE_SAPROM	equ	16h		; window on station addr prom
IE_RESET	equ	17h		; any write causes Board Reset

; bits in EDLC_XSTAT, interrupt clear on write, status when read
XS_TPOK		equ	80h		; transmit packet successful
XS_CS		equ	40h		; carrier sense
XS_RCVD		equ	20h		; transmitted packet received
XS_SHORT	equ	10h		; transmission media is shorted
XS_UFLW		equ	08h		; underflow.  iff failed board
XS_COLL		equ	04h		; collision occurred
XS_16COLL	equ	02h		; 16th collision occurred
XS_PERR		equ	01h		; parity error

XS_CLR_UFLW	equ	08h		; clear underflow
XS_CLR_COLL	equ	04h		; clear collision
XS_CLR_16COLL	equ	02h		; clear 16th collision
XS_CLR_PERR	equ	01h		; clear parity error

; bits in EDLC_XMASK, mask/enable transmit interrupts.  register is r/w
XM_TPOK		equ	80h		; =1 to enable Xmt Pkt OK interrupts
XM_RCVD		equ	20h		; =1 to enable Xmt Pkt Rcvd ints
XM_UFLW		equ	08h		; =1 to enable Xmt Underflow ints
XM_COLL		equ	04h		; =1 to enable Xmt Collision ints
XM_COLL16	equ	02h		; =1 to enable Xmt 16th Coll ints
XM_PERR		equ	01h		; =1 to enable Xmt Parity Error ints
					; note: always clear this bit

; bits in EDLC_RSTAT, interrupt clear on write, status when read
RS_PKT_OK	equ	80h		; received good packet
RS_RST_PKT	equ	10h		; RESET packet received
RS_RUNT		equ	08h		; Runt Pkt rcvd.  Len < 64 Bytes
RS_ALIGN	equ	04h		; Alignment error. not 8 bit aligned
RS_CRC_ERR	equ	02h		; Bad CRC on rcvd pkt
RS_OFLW		equ	01h		; overflow for rcv FIFO
RS_VALID_BITS	equ	RS_PKT_OK or RS_RST_PKT or RS_RUNT or RS_ALIGN or RS_CRC_ERR or RS_OFLW
					; all valid RSTAT bits

RS_CLR_PKT_OK	equ	80h		; clear rcvd packet interrupt
RS_CLR_RST_PKT	equ	10h		; clear RESET packet received
RS_CLR_RUNT	equ	08h		; clear Runt Pckt received
RS_CLR_ALIGN	equ	04h		; clear Alignment error
RS_CLR_CRC_ERR	equ	02h		; clear CRC error
RS_CLR_OFLW	equ	01h		; clear rcv FIFO Overflow

; bits in EDLC_RMASK, mask/enable receive interrupts.  register is r/w
RM_PKT_OK	equ	80h		; =1 to enable rcvd good packet ints
RM_RST_PKT	equ	10h		; =1 to enable RESET packet ints
RM_RUNT		equ	08h		; =1 to enable Runt Pkt rcvd ints
RM_ALIGN	equ	04h		; =1 to enable Alignment error ints
RM_CRC_ERR	equ	02h		; =1 to enable Bad CRC error ints
RM_OFLW		equ	01h		; =1 to enable overflow error ints

; bits in EDLC_RMODE, set Receive Packet mode.  register is r/w
RMD_TEST	equ	80h		; =1 for Chip testing.  normally 0
RMD_ADD_SIZ	equ	10h		; =1 5-byte addr match.  normally 0
RMD_EN_RUNT	equ	08h		; =1 enable runt rcv.  normally 0
RMD_EN_RST	equ	04h		; =1 to rcv RESET pkt.  normally 0

RMD_PROMISC	equ	03h		; receive *all* packets.  unusual
RMD_MULTICAST	equ	02h		; receive multicasts too.  unusual
RMD_BROADCAST	equ	01h		; receive broadcasts & normal. usual
RMD_NO_PACKETS	equ	00h		; don't receive any packets. unusual

; bits in EDLC_XMODE, set Transmit Packet mode.  register is r/w
XMD_COLL_CNT	equ	0f0h		; coll's since success.  read-only
XMD_IG_PAR	equ	008h		; =1 to ignore parity.  ALWAYS set
XMD_T_MODE	equ	004h		; =1 to power xcvr. ALWAYS set this
XMD_LBC		equ	002h		; =1 for loopbakc.  normally set
XMD_DIS_C	equ	001h		; =1 disables contention. normally 0

; bits in EDLC_RESET, write only
RS_RESET	equ	80h		; =1 to hold EDLC in reset state

; bits in IE_MMODE, write only
MM_EN_DMA	equ	80h		; =1 begin DMA xfer, Cplt clrs it
MM_EN_RCV	equ	40h		; =1 allows Pkt rcv.  clr'd by rcv
MM_EN_XMT	equ	20h		; =1 begin Xmt pkt.  Cplt clrs it
MM_BUS_PAGE	equ	18h		; =00 ALWAYS.  Used when MUX=1
MM_NET_PAGE	equ	06h		; =00 ALWAYS.  Used when MUX=0
MM_MUX		equ	01h		; =1 means Rcv Buff on system bus
					; =0 means Xmt Buff on system bus

; bits in IE_ISTAT, read only
IS_TDIAG	equ	80h		; =1 if Diagnostic problem
IS_EN_RCV	equ	20h		; =1 until frame is rcv'd cplt
IS_EN_XMT	equ	10h		; =1 until frame is xmt'd cplt
IS_EN_DMA	equ	08h		; =1 until DMA is cplt or aborted
IS_DMA_INT	equ	04h		; =0 iff DMA done interrupt. 
IS_R_INT	equ	02h		; =0 iff unmasked Rcv interrupt
IS_X_INT	equ	01h		; =0 iff unmasked Xmt interrupt

BFRSIZ		equ	2048	;number of bytes in a buffer

	public	int_no
int_no		db	3,0,0,0		; interrupt number.
io_addr		dw	0300h,0		; I/O address for card (jumpers)
ipkt_size	dw	?
opkt_size	dw	?

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	3		;from the packet spec
driver_name	db	'NI5010',0	;name of the driver.
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

is_186		db	0		;=0 if 808[68], =1 if 80[123]86.
our_type	dw	?,?
intstat		db	?

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
	cmp	cx,RUNT		; minimum length for Ether
	jae	oklen
	mov	cx,RUNT		; make sure size at least RUNT
oklen:
	inc	cx			;round size up to next even number.
	and	cx,not 1

;Wait for transmitter ready, if necessary. IE_XMTBSY is valid
;only in the transmit mode, hence the initial check.

	loadport
	setport	IE_ISTAT
	in	al,dx
	and	al,IS_EN_XMT		;on-going xmit
	je	send_pkt_2

	mov	bx,20000		;try this many times.
send_pkt_3:
	in	al,dx			;if not busy, exit.
	and	al,IS_EN_XMT
	je	send_pkt_2
	dec	bx
	jne	send_pkt_3
	mov	dh,CANT_SEND		;timed out, can't send.
	stc
	ret
send_pkt_2:

	pushf			; No distractions from the receiver
	cli

; Disable the Receiver
	mov	al, 0		; Mask all Receive Interrupts
	loadport
	setport	EDLC_RMASK
	out	dx,al

	mov	al, 0		; Put Xmt Buffer on System Bus
	setport	IE_MMODE
	out	dx,al

	mov	al, 0FFh	; Clear out any pending Rcv Ints
	setport	EDLC_RCLR
	out	dx,al

	mov	ax, BFRSIZ	; Point GP at beginning of packet
	sub	ax,cx
	setport	IE_GP
	out	dx,ax

	setport	IE_XBUF

	mov	opkt_size,cx	; opkt_size = cx;
	cmp	is_186,0	; Can we use rep outsb?
	je	out86		; no - have to do it slowly.
	db	0f3h, 06eh	;masm 4.0 doesn't grok "rep outsb"
	jmp	short ocnteven
out86:
	test	si,1		; (buf & 1) ?
	jz	obufeven	; no
	lodsb			; al = *si++;
	out	dx,al		; out(dx,al);
	dec	cx		; cx--;
obufeven:
	mov	di,cx		; save for later test
	shr	cx,1		; cx = cnt >> 1; (convert to word count)
; Do the bulk of the buffer, a word at a time
	jcxz	onobuf		; if(cx != 0){
xb:	lodsw			; do { ax = *si++; (si is word pointer)
	out	dx,al		; out(dx,lowbyte(ax));
	mov	al,ah
	out	dx,al		; out(dx,hibyte(ax));
	loop	xb		; } while(--cx != 0); }
; now check for odd trailing byte
onobuf:	shr	di,1		; if (di & 1)
	jnc	ocnteven
	lodsb			;   out(dx,*si++);
	out	dx,al
ocnteven:

; Rewrite where packet starts
	mov	ax,BFRSIZ
	sub	ax,opkt_size
	loadport
	setport	IE_GP
	out	dx,ax

; Flip Xmt Buffer to EDLC Bus so it can be transmitted
	mov	al, MM_MUX
	setport	IE_MMODE
	out	dx,al

; Begin transmission.
	mov	al, MM_EN_XMT or MM_MUX
	setport	IE_MMODE
	out	dx,al

; Cause interrupt after completion (or if something fails)
	mov	al, XM_TPOK or XM_RCVD or XM_UFLW or XM_COLL or XM_COLL16
	setport	EDLC_XMASK
	out	dx,al

	popf
	clc
	ret


	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	cmp	cx,EADDR_LEN			;make sure that we have enough room.
	jb	get_address_2
	cmp	cx,9			;do we have room for all of it?
	jb	get_address_3		;no.
	mov	cx,9			;yes - get the whole thing.
	xor	bx,bx
	jmp	short get_address_1
get_address_3:
	mov	cx,EADDR_LEN
	xor	bx,bx
get_address_1:
	mov	ax,bx
	loadport
	setport	IE_GP
	out	dx,ax

	setport	IE_SAPROM
	in	al,dx
	stosb
	inc	bx
	loop	get_address_1

	mov	cx,EADDR_LEN
	clc
	ret
get_address_2:
	stc
	ret


;Set Ethernet address on controller
	public	set_address
set_address:
	assume	ds:nothing
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
;
	cmp	cx,EADDR_LEN		;ensure that their address is okay.
	je	set_address_4
	mov	dh,BAD_ADDRESS
	stc
	jmp	short set_address_done
set_address_4:

	loadport
	setport	EDLC_ADDR
set_address_1:
	lodsb
	out	dx,al
	inc	dx
	loop	set_address_1
set_address_okay:
	mov	cx,EADDR_LEN		;return their address length.
	clc
set_address_done:
	push	cs
	pop	ds
	assume	ds:code
	ret


rcv_mode_1:
;Set up the receiver interrupts and flush status
	mov	al,RMD_NO_PACKETS
	loadport
	setport	EDLC_RMODE
	out	dx,al
	in	al,dx			;flush status.

	ret


rcv_mode_3:
;Set up the receiver interrupts and flush status
	mov	al,RMD_BROADCAST
	loadport
	setport	EDLC_RMODE
	out	dx,al
	in	al,dx			;flush status.

	ret


rcv_mode_5:
;Set up the receiver interrupts and flush status
	mov	al,RMD_MULTICAST
	loadport
	setport	EDLC_RMODE
	out	dx,al
	in	al,dx			;flush status.

	ret


rcv_mode_6:
;Set up the receiver interrupts and flush status
	mov	al,RMD_PROMISC
	loadport
	setport	EDLC_RMODE
	out	dx,al
	in	al,dx			;flush status.

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
	; put card in held-RESET state
	mov	al,0
	loadport
	setport	IE_MMODE
	out	dx,al
	mov	al,RS_RESET
	loadport
	setport	EDLC_RESET
	out	dx,al

	ret


	public	reset_interface
reset_interface:
;reset the interface.
;we don't do anything.
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

	loadport
	setport	IE_ISTAT
	in	al,dx
	mov	intstat,al

; DMA Complete Interrupt.  We don't use DMA, but just in case ...
	test	al,IS_DMA_INT
	jne	recv_isr_3

	mov	al, 0		 ; Reset DMA Interrupt
	setport	IE_DMA_RST
	out	dx,al

recv_isr_3:

; Transmit Complete/Fail Interrupt
	test	intstat,IS_X_INT
	jne	recv_isr_1

	loadport
	setport	EDLC_XSTAT
	in	al,dx
	mov	ah,al

	mov	al, 0			; Disable Xmt IRQ's
	setport	EDLC_XMASK
	out	dx,al

	mov	al, 0FFh		; clr all Xmt IRQ's
	setport	EDLC_XCLR
	out	dx,al

	test	ah,XS_COLL
	je	recv_isr_1
	;Crank counter back to beginning and restart xmt

	mov	ax,BFRSIZ		; Point GP at beginning of packet
	sub	ax,opkt_size
	setport	IE_GP
	out	dx,ax

	mov	al, 0		; De-assert MM_EN_RCV bit Just In Case
	setport	IE_MMODE
	out	dx,al

				; Flip Xmt Buffer to EDLC Bus and restart xmt
	mov	al, MM_EN_XMT or MM_MUX
	setport	IE_MMODE
	out	dx,al

			; Interrupt for all Transmit errors along with TPOK
	mov	al, XM_TPOK or XM_RCVD or XM_UFLW or XM_COLL or XM_COLL16
	setport	EDLC_XMASK
	out	dx,al

	ret			; Wait for it to complete again

recv_isr_1:
	; Is this a Receive Packet Interrupt?
	test	intstat,IS_R_INT
	jne	recv_isr_9_j_1		;no.

	loadport
	setport	EDLC_RSTAT		;get the status of this packet
	in	al,dx
	and	al,RS_VALID_BITS
	cmp	al,RS_PKT_OK		;is it ok?
	jne	recv_isr_7		;yes.

	; Clear the interrupt
	mov	al, 0FFh
	loadport
	setport	EDLC_RCLR
	out	dx,al

	; Flip Rcv Buffer onto the system bus
	mov	al, MM_MUX
	setport	IE_MMODE
	out	dx,al

	; Get the size of the packet.
	setport	IE_RCNT
	in	ax,dx
	mov	ipkt_size,ax

	cmp	ax,GIANT		;greater than GIANT?
	jbe	recv_isr_8		;no.
recv_isr_7:
	call	count_in_err
recv_isr_9_j_1:
	jmp	recv_isr_9
recv_isr_8:
;Put it on the receive queue

	mov	ax,EADDR_LEN+EADDR_LEN			;seek to the type word.
	setport	IE_GP
	out	dx,ax

	setport	IE_RBUF
	in	al,dx			;read the type word out of the board.
	mov	ah,al
	in	al,dx
	xchg	al,ah			;should be in network byte order.
	mov	our_type,ax
	in	al,dx			;read the type word out of the board.
	mov	ah,al
	in	al,dx
	xchg	al,ah			;should be in network byte order.
	mov	our_type+2,ax

	mov	ax,ds			;look up our type.
	mov	es,ax
	mov	di,offset our_type
	mov	cx,ipkt_size

	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	inc	di			;set di to 802.2 header
	inc	di
	mov	dl, IEEE8023
BlueBookPacket:
	call	recv_find

	mov	ax,es			;is this pointer null?
	or	ax,di
	je	recv_isr_9		;yes - just free the frame.

	push	es			;remember where the buffer pointer is.
	push	di

	xor	ax,ax			;seek to the beginning again.
	loadport
	setport	IE_GP
	out	dx,ax

	mov	cx,ipkt_size
	setport	IE_RBUF

	cmp	is_186,0	; Can we use rep insb?
	je	in86		; no - have to do it slowly.
	db	0f3h, 06ch	;masm 4.0 doesn't grok "rep insb"
	jmp	short icnteven
in86:
; If buffer doesn't begin on a word boundary, get the first byte
	test	di,1	; if(buf & 1){
	jz	ibufeven ;
	in	al,dx	; al = in(dx);
	stosb		; *di++ = al
	dec	cx	; cx--;
ibufeven:
	mov	si,cx	; size = cx;
	shr	cx,1	; cx = cnt >> 1; (convert to word count)
; Do the bulk of the buffer, a word at a time
	jcxz	inobuf	; if(cx != 0){
rb:	in	al,dx	; do { al = in(dx);
	mov	ah,al
	in	al,dx	; ah = in(dx);
	xchg	al,ah
	stosw		; *si++ = ax; (di is word pointer)
	loop	rb	; } while(--cx != 0);
; now check for odd trailing byte
inobuf:	shr	si,1
	jnc	icnteven
	in	al,dx
	stosb		; *di++ = al
icnteven:

	pop	si
	pop	ds
	assume	ds:nothing
	mov	cx,ipkt_size
	call	recv_copy		;tell them that we copied it.

	mov	ax,cs			;restore our ds.
	mov	ds,ax
	assume	ds:code

recv_isr_9:

; Prime Interlan card for another Receive
il_rcv_reset:

	; Rcv packet at start of Boards buffer
	mov	ax,0
	loadport
	setport	IE_GP
	out	dx,ax

	; Clear any remaining Interrupt conditions
	mov	al,0FFh
	setport	EDLC_RCLR
	out	dx,al

	; Set MUX to allow EDLC to access Rcv Buffer
	mov	al,0
	setport	IE_MMODE
	out	dx,al

;
;	Next section commented out to make promiscous mode work.
;	It makes no sense to reset the receive mode after each
;	received packet. M.K.
;
	; Enable Receive of Normal and Broadcast Packets only.
;	mov	al,RMD_BROADCAST
;	setport	EDLC_RMODE
;	out	dx,al

	; Enable Receive Interrupts
	mov	al,MM_EN_RCV
	setport	IE_MMODE
	out	dx,al

	; Unmask *all* Receive related interrupts
	mov	al,0FFh
	setport	EDLC_RMASK
	out	dx,al

	ret


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret


end_resident	label	byte

	public	usage_msg
usage_msg	db	"usage: NI5010 [-n] [-d] [-w] <packet_int_no> <int_no> <io_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for the Interlan NI5010, version ",'0'+majver,".",'0'+version,CR,LF
		db	"Portions Copyright 1988 Bill Doster",CR,LF,CR,LF
		db	"Promiscous mode fixed by Martin Knoblauch (22-Aug-90).",CR,LF
		db	"Flame <XBR2D96D@DDATHD21.BITNET> for related problems",CR,LF,CR,LF,'$'

using_186_msg	db	"Using 80[123]86 I/O instructions.",CR,LF,'$'
no_ni5010_msg	db	"No NI5010 found at that address.",CR,LF,'$'
ether_bdcst	db	EADDR_LEN dup(-1)	;ethernet broadcast address.
our_address	db	EADDR_LEN+3 dup(?)	;temporarily hold our address

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


no_ni5010_error:
	mov	dx,offset no_ni5010_msg
	mov	ah,9
	int	21h
	stc
	ret


	public	etopen
etopen:
;  Initialize the Ethernet board, set receive type.
;
;  check for correct EPROM location
;
	pushf		; No distractions from the receiver
	cli

	; Hold up EDLC RESET while the board is configured
	mov	al,RS_RESET
	loadport
	setport	EDLC_RESET
	out	dx,al

	; Hardware reset of Interlan Board
	mov	al,0
	loadport
	setport	IE_RESET
	out	dx,al

	call	set_recv_isr

	popf		; Shouldn't be any interrupts from here on out

	mov	al,XMD_LBC
	loadport
	setport	EDLC_XMODE
	out	dx,al

;Determine the processor type.  The 8088 and 8086 will actually shift ax
;over by 33 bits, while the 80[123]86 use a shift count mod 32.
	mov	cl,33
	mov	ax,0ffffh
	shl	ax,cl
	jz	not_186
	mov	is_186,1
	mov	dx,offset using_186_msg
	mov	ah,9
	int	21h
not_186:

	push	ds
	pop	es
	mov	di,offset our_address
	mov	cx,EADDR_LEN+3		;get three extra signature bytes.
	call	get_address
	mov	si,offset our_address
	mov	cx,EADDR_LEN
	call	set_address

;See if there really is a ni5010 there.
	cmp	our_address+EADDR_LEN+0,0
	jne	no_ni5010
	cmp	our_address+EADDR_LEN+1,055h
	jne	no_ni5010
	cmp	our_address+EADDR_LEN+2,0aah
	jne	no_ni5010

	mov	cx,EADDR_LEN
	mov	di,offset ether_bdcst
	repe	cmpsb
	jne	have_ni5010		;not broadcast address -- must be real.
no_ni5010:
	jmp	no_ni5010_error		;not there -- no ni5010.
have_ni5010:

	; Only enable Transmit-type interrupts while Transmitting
	mov	al,0
	loadport
	setport	EDLC_XMASK
	out	dx,al

	; Establish generic Transmit mode
	mov	al,XMD_IG_PAR or XMD_T_MODE or XMD_LBC
	setport	EDLC_XMODE
	out	dx,al

	; Clear any startup related Transmit interrupts
	mov	al,0FFh
	setport	EDLC_XCLR
	out	dx,al

	; Establish generic Receive mode
	mov	al,RMD_BROADCAST
	setport	EDLC_RMODE
	out	dx,al

	; Reset Rcv State to allow receives
	call	il_rcv_reset

	; Finally un-reset the EDLC
	mov	al,0
	loadport
	setport	EDLC_RESET
	out	dx,al

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
