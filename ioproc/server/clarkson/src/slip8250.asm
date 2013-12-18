version	equ	5

	include	defs.asm

;Ported from Phil Karn's asy.c and slip.c, a C-language driver for the IBM-PC
;8250 by Russell Nelson.  Any bugs are due to Russell Nelson.
;16550 support ruthlessly stolen from Phil Karn's 8250.c. Bugs by Denis DeLaRoca

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

;8250 definitions
;Control/status register offsets from base address
THR	equ	0		;Transmitter holding register
RBR	equ	0		;Receiver buffer register
DLL	equ	0		;Divisor latch LSB
DLM	equ	1		;Divisor latch MSB
IER	equ	1		;Interrupt enable register
IIR	equ	2		;Interrupt ident register
FCR	equ	2		;16550 FIFO control register
LCR	equ	3		;Line control register
MCR	equ	4		;Modem control register
LSR	equ	5		;Line status register
MSR	equ	6		;Modem status register

;8250 Line Control Register
LCR_5BITS	equ	0	;5 bit words
LCR_6BITS	equ	1	;6 bit words
LCR_7BITS	equ	2	;7 bit words
LCR_8BITS	equ	3	;8 bit words
LCR_NSB		equ	4	;Number of stop bits
LCR_PEN		equ	8	;Parity enable
LCR_EPS		equ	10h	;Even parity select
LCR_SP		equ	20h	;Stick parity
LCR_SB		equ	40h	;Set break
LCR_DLAB	equ	80h	;Divisor Latch Access Bit

;16550 FIFO control register values
FIFO_ENABLE     equ     001h    ;Enable TX and RX fifo
FIFO_CLR_RX     equ     002h    ;Clear RX fifo
FIFO_CLR_TX     equ     004h    ;Clear TX fifo
FIFO_START_DMA  equ     008h    ;Enable TXRDY/RXRDY pin DMA handshake
FIFO_SIZE_1     equ     000h    ;RX fifo trigger levels
FIFO_SIZE_4     equ     040h
FIFO_SIZE_8     equ     080h
FIFO_SIZE_14    equ     0c0h
FIFO_SIZE_MASK  equ     0c0h

FIFO_TRIGGER_LEVEL equ FIFO_SIZE_4
FIFO_SETUP         equ FIFO_ENABLE+FIFO_CLR_RX+FIFO_CLR_TX+FIFO_TRIGGER_LEVEL
OUTPUT_FIFO_SIZE   equ 16

;8250 Line Status Register
LSR_DR	equ	1	;Data ready
LSR_OE	equ	2	;Overrun error
LSR_PE	equ	4	;Parity error
LSR_FE	equ	8	;Framing error
LSR_BI	equ	10h	;Break interrupt
LSR_THRE equ	20h	;Transmitter line holding register empty
LSR_TSRE equ	40h	;Transmitter shift register empty

;8250 Interrupt Identification Register
IIR_IP		equ	1	;0 if interrupt pending
IIR_ID		equ	6	;Mask for interrupt ID
IIR_RLS		equ	6	;Receiver Line Status interrupt
IIR_RDA		equ	4	;Receiver data available interrupt
IIR_THRE	equ	2	;Transmitter holding register empty int
IIR_MSTAT	equ	0	;Modem status interrupt
IIR_FIFO_TIMEOUT  equ   008h    ;FIFO timeout interrupt pending - 16550 only
IIR_FIFO_ENABLED  equ   080h    ;FIFO enabled (FCR0 = 1) - 16550 only

;8250 interrupt enable register bits
IER_DAV	equ	1	;Data available interrupt
IER_TxE	equ	2	;Tx buffer empty interrupt
IER_RLS	equ	4	;Receive line status interrupt
IER_MS	equ	8	;Modem status interrupt

;8250 Modem control register
MCR_DTR	equ	1	;Data Terminal Ready
MCR_RTS	equ	2	;Request to Send
MCR_OUT1 equ	4	;Out 1 (not used)
MCR_OUT2 equ	8	;Master interrupt enable (actually OUT 2)
MCR_LOOP equ	10h	;Loopback test mode

;8250 Modem Status Register
MSR_DCTS equ	1	;Delta Clear-to-Send
MSR_DDSR equ	2	;Delta Data Set Ready
MSR_TERI equ	4	;Trailing edge ring indicator
MSR_DRLSD equ	8	;Delta Rx Line Signal Detect
MSR_CTS equ	10h	;Clear to send
MSR_DSR equ	20h	;Data set ready
MSR_RI	equ	40h	;Ring indicator
MSR_RLSD equ	80h	;Received line signal detect

;Slip Definitions
FR_END		equ	0c0h		;Frame End
FR_ESC		equ	0dbh		;Frame Escape
T_FR_END	equ	0dch		;Transposed frame end
T_FR_ESC	equ	0ddh		;Transposed frame escape

	public	int_no
int_no		db	4,0,0,0		; interrupt number.
io_addr		dw	03f8h,0		; I/O address for COM1.
baud_rate	dw	12c0h,0		; We support baud rates higher than 65535.
baudclk		label	word
		dd	115200		;1.8432 Mhz / 16
hardware_switch	db	0		;if zero, don't use hardware handshaking.
is_16550        db      0               ;0=no, 1=yes (try using fifo)

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	6,0,0,0		;from the packet spec
driver_type	db	0,0,0,0		;from the packet spec
driver_name	db	'SLIP8250',0	;name of the driver.
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

  ifdef debug
	public recv_buf_size, recv_buf,	recv_buf_end, recv_buf_head
	public recv_buf_tail, recv_pkt_ready
  endif
recv_buf_size	dw	3000,0		;receive buffer size
recv_buf	dw	?		;->receive buffer
recv_buf_end	dw	?		;->after end of buffer
recv_buf_head	dw	?		;->next character to get
recv_buf_tail	dw	?		;->next character to store
recv_pkt_ready	dw	0		; flag indicating a packet is ready

  ifdef debug
	public send_buf
  endif
send_buf_size	dw	3000,0		;send buffer size
send_buf	dw	?		;->send buffer
send_buf_end	dw	?		;->after end of buffer
send_buf_head	dw	?		;->next character to get
send_buf_tail	dw	?		;->next character to store

  ifdef debug
	public packet_sem, pkt_send_sem, xmit_time
  endif
packet_sem	dw	0		; semaphore for	packets received
pkt_send_sem	dw	0		; semaphore for	packets xmitted
asyrxint_cnt	dw	0		; loop counter in asyrxint
xmit_time	dw	0		; loop timer for asyrxint

	public	rcv_modes
rcv_modes	dw	4		;number	of receive modes in our table.
		dw	0,0,0,rcv_mode_3


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
;
; mod 7/25/89 John Grover
; - operates with interrupts on. Xmits one byte per interrupt
; - only turns transmitter buffer empty interrupt off when
; - all bytes of all packets are transmitted.

send_pkt:
;enter with es:di->upcall routine, (0:0) if no upcall is desired.
;  (only if the high-performance bit is set in driver_function)
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
;called from telnet layer via software interrupt
	assume	ds:nothing

	push	cs
	pop	es
	mov	di,send_buf_tail
	sti				; enable interrupts

	mov	al,FR_END		;Flush out any line garbage
	call	send_char

;Copy input to output, escaping special characters
send_pkt_1:
	lodsb
	cmp	al,FR_ESC		;escape FR_ESC with FR_ESC and T_FR_ESC
	jne	send_pkt_2
	mov	al,FR_ESC
	call	send_char
	mov	al,T_FR_ESC
	jmp	short send_pkt_3
send_pkt_2:
	cmp	al,FR_END		;escape FR_END with FR_ESC and T_FR_END
	jne	send_pkt_3
	mov	al,FR_ESC
	call	send_char
	mov	al,T_FR_END
send_pkt_3:
	call	send_char
	loop	send_pkt_1
	mov	al,FR_END		;terminate it with a FR_END
	call	send_char
	mov	send_buf_tail,di

	inc	pkt_send_sem		; increment the semaphore
	cmp	pkt_send_sem, 1		; see if we need to enable
					; xmit buffer empty interrupt
	ja	send_pkt_end

	mov	ah,IER_TxE		; enable xmit buffer empty interrupt
	cmp	hardware_switch,0	; should we do hardware handshaking?
	je	send_pkt_4		; no, just enable TxE.

;Enable modem status and (maybe) transmitter buffer empty interrupt.
	loadport
	setport	MSR
	mov	ah, IER_MS		; always enable modem status interrupt
	in	al, dx			; check if clear to send
	test	al, MSR_CTS
	jz	send_pkt_4		; no - won't enable xmit buffer empty interrupt

	or	ah,IER_TxE		; yes - enable xmit buffer empty interrupt

send_pkt_4:
	loadport
	setport	IER
	call	setbit			; enable
	cli
send_pkt_end:
	clc
	ret

;
; mod 7/25/89 John Grover
; - utilizes buffer pointers to ascertain if the
; - buffer has room for	another character
;

send_char:
;stuff the character in al into the transmit buffer, but only if there
;is enough room, otherwise ignore the char.
	assume	ds:nothing

	cmp	di, send_buf_head	;are we out of buffer?
	jne	send_char_ok		;no - continue
	cmp	pkt_send_sem, 0		;maybe - if no packets then no
	jne	send_char_1		;if there are packets then yes

send_char_ok:
	stosb				;store the char.
	cmp	di,send_buf_end		;do we need to wrap around?
	jne	send_char_1		;no.
	mov	di,send_buf		;yes - reload with beginning.
send_char_1:
	ret


	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	mov	cx,0
	clc
	ret


	public	set_address
set_address:
;set the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:nothing
	clc
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


	public	get_multicast_list
get_multicast_list:
;return with nc, es:di ->list of multicast addresses, cx = number of bytes.
;return	cy, NO_ERROR if we don't remember all of the addresses ourselves.
;return cy, NO_MULTICAST if we don't implement multicast.
	mov	dh,NO_MULTICAST
	stc
	ret


	public	terminate
terminate:
	ret

	public	reset_interface
reset_interface:
;reset the interface.
	assume	ds:code
	ret


;called	when we	want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type, dl = packet class.
	extrn	recv_find: near

;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
	extrn	recv_copy: near

	extrn	count_in_err: near
	extrn	count_out_err: near

	public	recv

;
; mod 7/25/89 John Grover
;
; - added code to check modem status change interrupt. If CTS is
; - low  turn off transmitter buffer empty interrupt. If CTS is
; - high turn it on.

recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
	assume	ds:code
recv_2:
	loadport
	setport	IIR
	in	al,dx			;any interrupts at all?
	test	al,IIR_IP
	jne	recv_1			;no.
	and	al,IIR_ID
	cmp	al,IIR_RDA		;Receiver interrupt
	jne	recv_3
	call	asyrxint
	jmp	recv_2
recv_3:
	cmp	al,IIR_THRE		;Transmit interrupt
	jne	recv_5
	call	asytxint
	jmp	recv_2
recv_5:
;process IIR_MSTAT here.
;  If CTS and packet ready then
;    enable the	transmit buffer empty interrupt
;  else
;    disable the transmit buffer empty interrupt
;
	cmp	al, IIR_MSTAT
	jne	recv_4
	setport	MSR			; make sure of CTS status
	mov	ah, IER_TxE		; get ready to alter xmit buffer empty interrupt
	in	al, dx
	test	al, MSR_CTS		; is CTS bit set
	jz	recv_5_1		; no - disable xmit buffer empty int
	cmp	pkt_send_sem, 0		; yes - is there a packet to xmit
	jbe	recv_2			; no - all done here
	setport	IER			; yes - enable xmit buffer empty int
	call	setbit
	jmp	recv_2

recv_5_1:
	setport	IER
	call	clrbit
	jmp	recv_2

recv_4:
;process IIR_RLS here
recv_1:
	ret


;Process 8250 receiver interrupts
;
; mod 7/25/89 John Grover
; - this branches off when bps < 9600. See asyrxint_a.
; - Above 9600 bps we go into a loop to process a packet at
; - a time. If not data ready for a certain amount of time,
; - the process exits and waits for the next byte. This certain
; - amount of time to wait depends on the bps and CPU processor speed
; - and is determined in the initialization of the driver.
; - Upon receiving the FR_END character for the first frame in the
; - buffer a semaphore is set which tells recv_frame to run.

asyrxint:

	push	ds			; get set up for the routine
	pop	es
	xor	bx, bx
	cmp	baud_rate, 9600         ; below 9600 we're strictly
	jbe	asyrxint_a              ; interrupt driven
	mov	bx, xmit_time
asyrxint_a:
	mov	di,recv_buf_tail
	xor	bp, bp			; set flag to indicate 1st char
					; processed
	mov	si, packet_sem          ; optimization
	loadport
	mov	ah, LSR_DR

asyrxint_again:
	xor	cx, cx			; initialize counter
	setport	LSR
asyrxint_in:
	in	al,dx			; check for data ready
	test	al,LSR_DR
	jnz	asyrxint_gotit		; yes - break out of loop
	inc	cx			; no - increase loop counter
	cmp	cx, bx			; timeout?
	jae	asyrxint_exit		; yes - leave
	jmp	asyrxint_in		; no - keep looping

asyrxint_gotit:
	setport	RBR
	in	al,dx

;Process incoming data;
; If buffer is full, we have no choice but
; to drop the character

	cmp	di,recv_buf_head	; check for buffer collision
	jne	asyrxint_ok		; none - continue
	or	si, si                  ; maybe - if there are packets
	jnz	asyrxint_exit		; yes exit

asyrxint_ok:
	stosb

	cmp	di,recv_buf_end		; did we hit the end of the buffer?
	jne	asyrxint_3		; no.
	mov	di,recv_buf		; yes - wrap around.

asyrxint_3:
	cmp	al,FR_END		; might	this be	the end of a frame?
	jne	asyrxint_reset		; no - reset flag and loop
	inc	si                      ; yes - indicate packet ready
	cmp	si, 1                   ; determine if semaphore is <> 1
	jne	asyrxint_chk_flg        ; yes - recv_frame must be active
	inc	recv_pkt_ready          ; no - set flag to start recv_frame

asyrxint_chk_flg:
	cmp	bp, 0                   ; was this the first char?
	jne	asyrxint_1              ; no - exit handler
asyrxint_reset:
	inc	bp			; set 1st character flag
	jmp	asyrxint_again		; get another character

asyrxint_exit:
asyrxint_1:
	mov	recv_buf_tail,di
	mov	packet_sem, si

	ret


; --------------------------------------------------------------
;
;  recv_exiting
;
	public	recv_exiting
recv_exiting:
	cmp	recv_pkt_ready, 1       ; is a packet ready?
	jne	recv_isr_exit           ; no - skip to end
	push	ax
	push	bx
	push	cx
	push	dx
	push	ds
	push	es
	push	bp
	push	di
	push	si
	push	cs			; point ds properly
	pop	ds
	mov	recv_pkt_ready,	0	; reset flag
	sti				; enable interrupts

	call	recv_frame

	cli
	pop	si
	pop	di
	pop	bp
	pop	es
	pop	ds
	pop	dx
	pop	cx
	pop	bx
	pop	ax
recv_isr_exit:
	ret


; --------------------------------------------------------------
;
;  recv_frame
;
; mod 7/25/89 John Grover
;
; - recv_frame now operates with interrupts on. It is triggered
; - by the recv_pkt_ready flag and continues until all bytes
; - in all packets in the buffer have been transmitted to the upper
; - layer.
;
  ifdef debug
	public recv_frame
  endif
recv_frame:
	cmp	packet_sem, 0		; should we do this?
	jz	recv_frame_end		; no - exit
	mov	si,recv_buf_head	;process characters.
	xor	cx,cx			;count up the size here.
recv_frame_1:

	call	recv_char		;get a char.
	je	recv_frame_2		;go if no more chars.
	cmp	al,FR_ESC		;an escape?
	je	recv_frame_1		;yes - don't count this char.
	inc	cx			;no - count this one.
	jmp	recv_frame_1
recv_frame_2:

	jcxz	recv_frame_3		;count zero? yes - just free the frame.
;we don't need to set the type because none are defined for SLIP.
	push	si			;save si in case we reject it.
	push	bx
	mov	di,0			;but we avoid any segment end bullshit.
	mov	dl,cs:driver_class
	call	recv_find		;look up our type.
	pop	bx
	pop	si

	mov	ax,es			;is this pointer null?
	or	ax,di
	je	recv_frame_3		;yes - just free the frame.

	push	cx
	push	es			;remember where the buffer pointer is.
	push	di

	mov	si,recv_buf_head	;process characters.
recv_frame_4:
	call	recv_char
	je	recv_frame_6		;yes - we're all done.
	cmp	al,FR_ESC		;an escape?
	jne	recv_frame_5		;no - just store it.

	call	recv_char		;get the next character.
	je	recv_frame_6
	cmp	al,T_FR_ESC
	mov	al,FR_ESC		;assume T_FR_ESC
	je	recv_frame_5		;yup, that's it	- store FR_ESC
	mov	al,FR_END		;nope, store FR_END
recv_frame_5:
	stosb				;store the byte.
	jmp	recv_frame_4
recv_frame_6:
	mov	recv_buf_head,si	;we're skipped to the end.

	pop	si			;now give the frame to the client.
	pop	ds
	pop	cx
	assume	ds:nothing

	call	recv_copy
	push	cs
	pop	ds
	assume	ds:code
	jmp	recv_frame_end

recv_frame_3:
	mov	recv_buf_head,si	;remember the new starting point.
recv_frame_end:
	dec	packet_sem
	cmp	packet_sem, 0		; are there more packets ready?
	ja	recv_frame              ; yes - execute again
	ret


; --------------------------------------------------------------
;
;  recv_char
;
; mod 7/25/89 John Grover
; - Now	uses buffer pointers to determine if there are
; - characters left.
;

recv_char:
;enter with si -> receive buffer, bx = receive count.  Wrap around if needed.
;return with nz, al = next char.  Return zr if there are no more chars in
;  this frame.
;
	lodsb
	cmp	si,recv_buf_end
	jb	recv_char_1
	mov	si,recv_buf
recv_char_1:
	mov	bx, recv_buf_tail
	cmp	si, bx
	je	recv_char_2
	cmp	al,FR_END
recv_char_2:
	ret


;Handle 8250 transmitter interrupts

; --------------------------------------------------------------
;
;  asytxint
;
; mod 7/25/89
; - Transmits one character and then exits. Upon last character
; - to transmit modem status and transmitter buffer empty interrupt
; - are disabled.
;

asytxint:

asytxint_2:

;
; mod  3/16/90  Denis DeLaRoca
; - for 16550 fifo uart stuff up to 16 chars at a time
; - for  8250 uart can only output one char at a time
;
	cmp	is_16550,1              ;have 16550 uart
	jne	asytxint_8250           ;no, handle as 8250

asytxint_16550:
	loadport
	setport	THR
	mov	cx,16                   ;fifo fill-loop counter
	mov     si,send_buf_head        ;current head of buffer
asytxint_next:
	lodsb				;fetch next char
	cmp	si,send_buf_end		;past end of buffer
	jne     asytxint_fill
	mov	si,send_buf		;yes, wrap around 
asytxint_fill:
	out	dx,al			;output char
	cmp	si,send_buf_tail	;any more chars to output
	je      asytxint_no_more        ;none...
	loop	asytxint_next		;loop while fifo not full
	mov	send_buf_head,si   	;save last char position used
	jmp	asytxint_1		;and exit
asytxint_no_more:
	mov     send_buf_head,si        ;save last char position used
	jmp	asytxint_disable	;go disable status interrupts

asytxint_8250:
	mov	si,send_buf_head	; yes - load one
	lodsb				;
	cmp	si,send_buf_end		;have we hit the end yet?
	jne	asytxint_3		;no.
	mov	si,send_buf		;yes - wrap around
asytxint_3:
	loadport
	setport	THR
	out	dx,al
	mov	send_buf_head,si	; store our location
	cmp	si, send_buf_tail	; more to xmit?
	jne	asytxint_1		; yes just exit

;No more characters to transmit -- disable transmit interrupts.

asytxint_disable:
	setport	IER			;Disable transmit and modem
	mov	ah,IER_TxE or IER_MS	; status interrupts
	call	clrbit
	mov	pkt_send_sem, 0		; indicate we're finished
asytxint_1:
	ret


;Set bit(s) in I/O port
setbit:
;enter with dx = port, ah = bit to set.
	in	al,dx
	or	al,ah
	out	dx,al
	ret


;Clear bit(s) in I/O port
clrbit:
;enter with dx = port, ah = bit to set.
	in	al,dx
	not	al			;perform an and-not using DeMorgan's.
	or	al,ah
	not	al
	out	dx,al
	ret


;any code after this will not be kept after initialization.
end_resident	label	byte

	public	usage_msg
usage_msg	db	"usage: SLIP8250 [-n] [-d] [-w] packet_int_no [-h] [driver_class] [int_no]",CR,LF
		db	"   [io_addr] [baud_rate]",CR,LF
		db	"   [send_buf_size] [recv_buf_size]",CR,LF
		db	"   -h enables hardware handshaking",CR,LF
		db	"   The driver_class should be SLIP, KISS, AX.25, or a number.",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for SLIP8250, version ",'0'+majver,".",'0'+version,CR,LF
		db	"Portions Copyright 1988 Phil Karn",CR,LF,'$'

approximate_msg	db	"Warning: This baud rate can only be approximated using the 8250",CR,LF
		db	"because it is not an even divisor of 115200",CR,LF,'$'

is_16550_msg    db      "16550 Uart detected, FIFO will be used",CR,LF,'$'

class_name_ptr	dw	?
class_name	db	"Interface class ",'$'
kiss_name	db	"KISS",CR,LF,'$'
ax25_name	db	"AX.25",CR,LF,'$'
slip_name	db	"SLIP",CR,LF,'$'
int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'
baud_rate_name	db	"Baud rate ",'$'
send_buf_name	db	"Send buffer size ",'$'
recv_buf_name	db	"Receive buffer size ",'$'
unusual_com1	db	"That's unusual!  Com1 (0x3f8) usually uses interrupt 4!",CR,LF,'$'
unusual_com2	db	"That's unusual!  Com2 (0x2f8) usually uses interrupt 3!",CR,LF,'$'

	extrn	set_recv_isr: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

;enter with si -> argument string.
;skip spaces and tabs.  Exit with si -> first non-blank char.
	extrn	skip_blanks: near


	public	parse_args
parse_args:
;exit with nc if all went well, cy otherwise.
	call	skip_blanks
	cmp	al,'-'			;did they specify a switch?
	jne	not_switch
	cmp	byte ptr [si+1],'h'	;did they specify '-h'?
	je	got_hardware_switch
	stc				;no, must be an error.
	ret
got_hardware_switch:
	mov	hardware_switch,1
	add	si,2			;skip past the switch's characters.
	jmp	parse_args		;go parse more arguments.
not_switch:
	or	al,20h			;convert to lower case (assuming letter).
	cmp	al,'k'
	jne	parse_args_2
	mov	driver_class,10		;KISS, from packet spec.
	mov	dx,offset kiss_name
	jmp	short parse_args_1
parse_args_2:
	cmp	al,'s'
	jne	parse_args_3
	mov	driver_class,6		;SLIP, from packet spec.
	mov	dx,offset slip_name
	jmp	short parse_args_1
parse_args_3:
	cmp	al,'a'
	jne	parse_args_4
	mov	driver_class,9		;AX.25, from packet spec.
	mov	dx,offset ax25_name
	jmp	short parse_args_1
parse_args_4:
	mov	di,offset driver_class
	mov	bx,offset class_name
	call	get_number
	mov	class_name_ptr,0
	jmp	short parse_args_6
parse_args_1:
	mov	class_name_ptr,dx
parse_args_5:
	mov	al,[si]			;skip to the next blank or CR.
	cmp	al,' '
	je	parse_args_6
	cmp	al,CR
	je	parse_args_6
	inc	si			;skip the character.
	jmp	parse_args_5
parse_args_6:
	mov	di,offset int_no
	call	get_number
	mov	di,offset io_addr
	call	get_number
	mov	di,offset baud_rate
	call	get_number
	mov	di,offset send_buf_size
	call	get_number
	mov	di,offset recv_buf_size
	call	get_number
	clc
	ret


; --------------------------------------------------------------
;
;  etopen
;
; mod 7/25/89 John Grover
; - Contains a loop to determine a pseudo timeout for asyrxint.
; - The value is determined by transmitting characters in a
; - loop whose clock cycles are nearly the same as the "sister"
; - loop in asyrxint. The per character, maximum time used
; - basis which is then multiplied by a factor to achieve a timeout
; - value for the particular bps and CPU speed of the host.

	public	etopen
etopen:
	pushf
	cli
;
; mod  3/16/90  Denis DeLaRoca
; - determine if 16550 uart is present
; - if so initialize fifo buffering
;
	loadport
	setport	FCR
	mov	al,FIFO_ENABLE
	out	dx,al                   ;outportb(base+FCR,(char) FIFO_ENABLE)
	setport	IIR
	in	al,dx                   ;inportb(base+IIR)
	and	al,IIR_FIFO_ENABLED     ;     & IIR_FIFO_ENABLED
	cmp	al,IIR_FIFO_ENABLED	;both bits must be on   NEW, 11/20/90
	jnz	not_16550               ;nope, we don't have 16550 chip
	mov	is_16550,1              ;yes, note fact
	mov	al,FIFO_SETUP           ;and setup FIFO
	setport	FCR
	out	dx,al                   ;outportb(base+FCR,(char) FIFO_SETUP)

	mov	dx,offset is_16550_msg
	mov	ah,9
	int	21h			;let user know about 16550

not_16550:
	loadport			;Purge the receive data buffer
	setport	RBR
	in	al,dx

	;Set line control register: 8 bits, no parity
	mov	al,LCR_8BITS
	setport	LCR
	out	dx,al

	;Turn on receive interrupt enable in 8250, leave transmit
	; and modem status interrupts turned off for now
	mov	al,IER_DAV
	setport	IER
	out	dx,al

	;Set modem control register: assert DTR, RTS, turn on 8250
	; master interrupt enable (connected to OUT2)

	mov	al,MCR_DTR or MCR_RTS or MCR_OUT2
	setport	MCR
	out	dx,al

;compute the divisor given the baud rate.
	mov	dx,baudclk+2
	mov	ax,baudclk
	mov	bx,0
asy_speed_1:
	inc	bx
	sub	ax,baud_rate
	sbb	dx,baud_rate+2
	jnc	asy_speed_1
	dec	bx
	add	ax,baud_rate
	adc	dx,baud_rate+2
	or	ax,dx
	je	asy_speed_2

	mov	dx,offset approximate_msg
	mov	ah,9
	int	21h

asy_speed_2:

	loadport			;Purge the receive data buffer
	setport	RBR
	in	al,dx

	mov	ah,LCR_DLAB		;Turn on divisor latch access bit
	setport	LCR
	call	setbit

	mov	al,bl			;Load the two bytes of the divisor.
	setport	DLL
	out	dx,al
	mov	al,bh
	setport	DLM
	out	dx,al

	mov	ah,LCR_DLAB		;Turn off divisor latch access bit
	setport	LCR
	call	clrbit

	call	set_recv_isr		;Set interrupt vector to SIO handler

;set up the various pointers.
	mov	dx,offset end_resident
	mov	send_buf,dx
	mov	send_buf_head,dx
	mov	send_buf_tail,dx
	add	dx,send_buf_size
	mov	send_buf_end,dx

	mov	recv_buf,dx
	mov	recv_buf_head,dx
	mov	recv_buf_tail,dx
	add	dx,recv_buf_size
	mov	recv_buf_end,dx
	push	dx			;save the ending address.

	; the following code attempts to determine a pseudo timeout
	; value	to use in the loop that waits for an incoming character
	; in asyrxint. The value returned in xmit_time is the number of
	; loops processed between characters - therefore the loop used below
	; is and should	remain similar to the loop used in asyrxint.

	xor	ax, ax			; we'll send a 0
	mov	ah, LSR_THRE
	mov	cx, 10h			; take the highest of 16 runs
	xor	si, si			; will hold highest value

xmit_time_start:

	xor	di, di			; initialize counter
	loadport
	setport	THR			; xmit a character
	out	dx, al
	setport	LSR		       ; set up	to check for an empty buffer

	; next is the loop actually being timed

xmit_time_top:
	in	al, dx
	test	al, ah
	jnz	xmit_time_done
	inc	di
	cmp	cx, cx			; these next few instructions do nothing
	jmp	xmit_time_1		;  except maintain similarity with the
					;  "sister" loop in asyrxint
xmit_time_1:
	jmp	xmit_time_top

xmit_time_done:				; end of timed loop



	cmp	si, di			; compare highest value with new value
	ja	xmit_time_end		; no bigger - just loop
	mov	si, di			; bigger - save it

xmit_time_end:
	loop	xmit_time_start		; bottom of outer loop

	shl	si, 1			; we'll wait 8 characters worth
	shl	si, 1
	shl	si, 1
	mov	xmit_time, si		; retain largest value

	; end of pseudo timer determination

	mov	al, int_no		; Get board's interrupt vector
	add	al, 8
	cmp	al, 8+8			; Is it a slave 8259 interrupt?
	jb	set_int_num		; No.
	add	al, 70h - 8 - 8		; Map it to the real interrupt.
set_int_num:
	xor	ah, ah			; Clear high byte
	mov	int_num, ax		; Set parameter_list int num.

	pop	dx			;return the ending address.
	popf
	clc				;indicate no errors.
	ret

	public	print_parameters
print_parameters:
;echo our command-line parameters
	cmp	class_name_ptr,0
	je	echo_args_1

	mov	dx,offset class_name
	mov	ah,9
	int	21h
	mov	dx,class_name_ptr
	mov	ah,9
	int	21h
	jmp	short echo_args_2
echo_args_1:
	mov	di,offset driver_class
	mov	dx,offset class_name
	call	print_number
echo_args_2:

	mov	di,offset int_no
	mov	dx,offset int_no_name
	call	print_number
	mov	di,offset io_addr
	mov	dx,offset io_addr_name
	call	print_number

	cmp	io_addr,03f8h		;is this com1?
	jne	ia_com2
	mov	dx,offset unusual_com1
	cmp	int_no,4		;com1 usually uses int 1.
	jne	ia_unusual
	jmp	short ia_usual
ia_com2:
	cmp	io_addr,02f8h		;is this com2?
	jne	ia_usual		;no.
	mov	dx,offset unusual_com2
	cmp	int_no,3		;com2 usually uses int 3.
	je	ia_usual
ia_unusual:
	mov	ah,9
	int	21h
ia_usual:

	mov	di,offset baud_rate
	mov	dx,offset baud_rate_name
	call	print_number
	mov	di,offset send_buf_size
	mov	dx,offset send_buf_name
	call	print_number
	mov	di,offset recv_buf_size
	mov	dx,offset recv_buf_name
	call	print_number
	ret

code	ends

	end
