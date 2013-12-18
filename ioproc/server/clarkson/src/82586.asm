i82586_version	equ	2
  ifndef DAN
DAN	equ	4			; 4 seems to work best.
  endif

;
; Code that is common between 82586 implementations.
;

; Ported from Tim Krauskopf's micnet.asm, an assembly language
; driver for the MICOM-Interlan NI5210, by Russell Nelson.  Any bugs
; are due to Russell Nelson.
; Updated to version 1.08 Feb. 17, 1989 by Russell Nelson.
; Updated to support 1500 byte MTU April 27, 1989 By Brad Clements.

; Copyright, 1988-90, Russell Nelson

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

;
;  Structure elements specific to the Intel 82586 chip
;

; System Configuration Pointer
scp_struc	struc
scp_bus		db	?,?,?, ?,?,?	; bus use flag (0=16 bit, 1=8 bit).
scp_ptr		dd	?		; 24pointer to iscp
scp_struc	ends

; Intermediate System Configuration Pointer
iscp_struc	struc
iscp_busy	db	?,?		; busy flag (zeroed after init).
iscp_offset	dw	?		; 16pointer to iscp
iscp_base	dd	?		; base for all 16 pointers, lo, hi
iscp_struc	ends

; System Control Block
scb_struc	struc
scb_stat	dw	?		; status
scb_com		dw	?		; command
scb_cbl		dw	?		; 16pointer to command block list
scb_rfa		dw	?		; 16pointer to receive frame list
scb_serrs	dw	4 dup(?)	; 4 words of error counts
scb_struc	ends

; (Received) Frame Descriptor
fd_struc	struc
fd_status	dw	?		; status word for frame
fd_eol		dw	?		; end of FD list flag
fd_link		dw	?		; 16pointer to next FD
fd_ptr		dw	?		; 16pointer to list of RBD's
fd_dest		db	EADDR_LEN dup(?); 48 bits of destination
fd_source	db	EADDR_LEN dup(?); 48 bits of source
fd_cnt		dw	?		; length field of frame.
fd_struc	ends

; Receive Buffer Descriptor
rbd_struc	struc
rbd_status	dw	?		; status word in RBD
rbd_link	dw	?		; 16pointer to next RBD
rbd_ptr		dd	?		; 24pointer to actual buffer
rbd_size	dw	?		; size of the buffer
rbd_struc	ends

; Transmit Command Block
tcb_struc	struc
tcb_status	dw	?		; status word for xmit
tcb_com		dw	?		; command to transmit
tcb_link	dw	?		; 16pointer to next command
tcb_ptr		dw	?		; 16pointer to xmit TBD
tcb_addr	db	EADDR_LEN dup(?); destination address
tcb_len		dw	?
tcb_struc	ends

; Transmit Buffer Descriptor
tbd_struc	struc
tbd_status	dw	?		; bit 15=EOF, 13-0=actual count
tbd_link	dw	?		; 16pointer to next TBD
tbd_ptr		dd	?		; 24pointer to buffer
tbd_struc	ends

; all commands have at least the following:
cmd_struc	struc
cmd_status	dw	?		; status word
cmd_com		dw	?		; command word.
cmd_struc	ends

; MC-SETUP Command Block
mcb_struc	struc
mcb_status	dw	?		; status word for multicast
mcb_com		dw	?		; command to setup multicast
mcb_link	dw	?		; 16pointer to next command
mcb_cnt		dw	?		; number of multicast addresses.
mcb_struc	ends

; TDR Command Block
tdr_struc	struc
tdr_status	dw	?		; status word for TDR
tdr_com		dw	?		; command to setup TDR
tdr_link	dw	?		; 16pointer to next command
tdr_time	dw	?		; error bits and time
tdr_struc	ends

;Memory allocation.

SCPTR	EQU	0fff6h			; hardwired address for SCP
ISCPTR	EQU	0ffeeh			; my address for ISCP, points to SCB
SCB	EQU	ISCPTR - 16		; system control block base
CCBPTR	EQU	SCB - 18		; offset of configure command block
TBDPTR	EQU	CCBPTR - 8		; xmit BD offset
TCBPTR	EQU	TBDPTR - 16		; xmit CB offset
TBUFPTR	EQU	TCBPTR - GIANT		; xmit buffer offset
;the receive buffers appear at lower addresses than TBUFPTR.
RBUFLEN	EQU	200
RBUF_TOTAL	equ	(size fd_struc) + (size rbd_struc) + RBUFLEN
FDBASE		equ	TBUFPTR - RBUF_TOTAL

memory_begin	dw	?

	public	rcv_modes
rcv_modes	dw	7		;number of receive modes in our table.
		dw	0               ;There is no mode zero
		dw	0
		dw	rcv_mode_2
		dw	rcv_mode_3
		dw	rcv_mode_4	;haven't set up perfect filtering yet.
		dw	0
		dw	rcv_mode_6

firstfd		dw	FDBASE		; start of FD queue
lastfd		dw	0		; end of the FD chain
lastbd		dw	0		; end of the BD chain
flag		dw	0


;
; Configuration block for 82586, this comprises one config command
;  Parameters taken from MICOM driver
;
CBCONF	DW	0		; status word
	DW	8002H		; end of command list + configure command
	DW	0ffffh		; link to next command (not used)
	DW	080CH		; fifo=8, byte count=C
	DW	2E00H		; important! Addr (AL) not inserted on the fly!
	DW	6000H		; IFS = 60h
	DW	0F200H		; retry=F, slot time=200h
CBCONF_FLAGS	label	byte
	DW	0		; flags, set to 1 for promiscuous
CBCONF_MINLEN	label	byte
	DW	40H		; min frame length=40h


doca_wait:
;enter with ax = command to execute, es = base_addr.
;exit with nc if the command ran to completion.
;exit with cy if the command timed out.  Eventually we'll also reset the chip.
	mov	es:[SCB].scb_com,ax	;set the command.

	mov	si,es:[SCB].scb_cbl	;
	mov	es:[si].cmd_status,0	; status word of specific command
	and	ax,0700h
	cmp	ax,0100h		; is it an action command?
	jne	doca_wait_a		; no, any other

	call	doca

comment \
Quoting from the D-Step Errata Revision 2.0:

The value for the deadman timer should be greater than the longest
command execution time.  The command which can take the longest time
to execute is the transmit command, assuming maximum retries.  To
determine the maximum amount of time the transmit command may take,
one must use the following equation: 7143 ST + 14 f, where ST stands
for Slot Time and f = Maximum Frame Size + IFS + Preamble.  For
Ethernet/IEEE 802.3 where ST = 512 bits, f = 12144 bits, Preamble =
64 bits, IFS  96 bits, and one bit = 0.1 usec, the deadman timeout
should be greater than 0.369 seconds.

\

	mov	ax,14			;36.4 ticks / seconds * .369 seconds
	call	set_timeout
doca_wait_1:
	test	es:[si].cmd_status,8000h	; is command complete?
	jnz	doca_wait_2		;yes.
	call	do_timeout		;did we time out yet?
	jne	doca_wait_1		;not yet.
;reset the chip here, then Configure, IA-Setup, and MC-Setup.
	stc				;timeout -- uh-oh.
	ret
doca_wait_2:
	clc
	ret

doca_wait_a:
	call	doca
doca_wait_a_0:
	cmp	es:[SCB].scb_com,0	; has the command been accepted?
	jnz	doca_wait_a_0		; not yet.
	clc
	ret


	include	timeout.asm
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
	mov	es,base_addr		; base for board

	cmp	cx,GIANT		; Is this packet too large?
	ja	send_pkt_toobig

	mov	dx,cx			; save a copy, might be less than 60, ok

	cmp	dx,RUNT			; minimum length for Ether
	jnb	oklen
	mov	dx,RUNT			; make sure size at least RUNT
oklen:
	mov	di,TBUFPTR		; start of xmit buffer

;
;  check for previous xmit
;
xwait:
	mov	bx,es:[SCB].scb_com	; has previous command been accepted?
	or	bx,bx
	jnz	xwait			; not there yet, wait for it
wait_for_transmit_to_complete:
	test	word ptr es:[TCBPTR],4000h
	jnz	wait_for_transmit_to_complete
;
;  move the data using word moves.
;
	call	movemem
;
;  put the correct size into the TBD
;
	or	dx,08000h			; end of frame bit flag
	mov	es:[TBDPTR].tbd_status,dx	; store it
	mov	es:[TCBPTR].tcb_status,0	; zero status wd
	mov	es:[TCBPTR].tcb_com,8004h	; xmit command in TCB
	mov	es:[SCB].scb_com,0100h		; execute command
	mov	es:[SCB].scb_cbl,TCBPTR	; say where xmit command is

	call	doca

	clc
	ret
send_pkt_toobig:
	mov	dh,NO_SPACE
	stc
	ret


rcv_mode_2:
	and	CBCONF_FLAGS,not 3
	or	CBCONF_FLAGS,2		;disable broadcasts.
	mov	CBCONF_MINLEN,40h
	jmp	short reconfigure
rcv_mode_4:
rcv_mode_3:
	and	CBCONF_FLAGS,not 3	;clear promiscuous mode.
	mov	CBCONF_MINLEN,40h
	jmp	short reconfigure
rcv_mode_6:
	and	CBCONF_FLAGS,not 3
	or	CBCONF_FLAGS,1		;set promiscuous mode.
	mov	CBCONF_MINLEN,0		;allow runts.
reconfigure:
	mov	es,base_addr		;get the base address for the board.
	mov	si,offset CBCONF	; configure command
	mov	di,CCBPTR		; where command will reside
	mov	cx,9
	rep	movsw			; copy to board
;
;  issue the configure command
;
	mov	es:[SCB].scb_cbl,CCBPTR	; where conf command is
	mov	es:[SCB].scb_serrs[0],0	; zero errs field
	mov	es:[SCB].scb_serrs[2],0	; zero errs field
	mov	es:[SCB].scb_serrs[4],0	; zero errs field
	mov	es:[SCB].scb_serrs[6],0	; zero errs field
	mov	ax,100h			; do-command command
	call	doca_wait
	ret


	public	set_multicast_list
set_multicast_list:
;enter with ds:si ->list of multicast addresses, cx = number of addresses.
;return nc if we set all of them, or cy,dh=error if we didn't.
	assume	ds:code
	mov	es,base_addr
	mov	es:[SCB].scb_cbl,TBUFPTR	;use the transmit buffer.

	mov	es:[TBUFPTR].mcb_status,0	;status word
	mov	es:[TBUFPTR].mcb_com,08003h	;command word for mc-setup + EL
	mov	es:[TBUFPTR].mcb_link,-1	;no command link.
	mov	di,TBUFPTR + mcb_cnt
	mov	ax,cx			;store the count.
	stosw
	rep	movsb

comment \ avoid deferred execution of a CU command during reception.
If a command is executed with a length of 18 to 20 bytes, and a frame
is received, the 82586 may deadlock with HOLD active.  We avoid this
problem by suspending the receiver. \

	mov	ax,30h			;suspend frame receiving.
	call	doca_wait

	mov	ax,100h			; do-command command
	call	doca_wait
	jnc	set_multicast_2

	mov	ax,20h			;resume frame receiving.
	call	doca_wait

	mov	dh,NO_MULTICAST		;for some reason we can't do multi's.
	stc
	ret
set_multicast_2:
	mov	ax,20h			;resume frame receiving.
	call	doca_wait

	clc
	ret


	public	terminate
terminate:
	assume	ds:code
	ret

	public	reset_interface
reset_interface:
;reset the interface.
;we don't do anything.
	assume	ds:nothing
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
  ifdef IO_INTCLR
	loadport			;clear the interupt latch.
	setport	IO_INTCLR
	out	dx,al
  endif

	mov	flag, 1
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
recv1:
	mov	ds,base_addr	; base for board
	assume	ds:nothing

	mov	ax,ds:[SCB].scb_stat	;get the status.
recv_isr_1:
	cmp	ds:[SCB].scb_com,0	;has previous command been accepted?
	jne	recv_isr_1		;no -- keep waiting.

	and	ax,0f000h		;isolate the ACK bits to make a
					;  command to ack the interrupt.
  if DAN and 1
	jz	recv_isr_2
  endif

	mov	ds:[SCB].scb_com,ax	;set the command.
	call	doca
recv_isr_2:
	cmp	ds:[SCB].scb_com,0	; has the command been accepted?
	jnz	recv_isr_2		; not yet.

;  Get whatever packets are on the board
;
	mov	bx,firstfd	; get addr of first FD in list
	mov	ax,[bx].fd_status	; status word of frame
	test	ax,08000h	; frame written?
	jnz	okframe

	jmp	ru_start	; no, restore receiver if necessary
frame_bad:
	call	count_in_err
ptrupdate_j_1:
	jmp	ptrupdate

;  we have a frame, read it in
;
okframe:
	test	ax,02000h		;check frame OK bit
	jz	frame_bad		;bad, fix it.
	mov	si,[bx].fd_ptr		;get pointer to buffer descriptor
	xor	cx,cx			;start with zero bytes.
countbuf:				;es:di is already set to receive packet
	mov	dx,si			;save a copy of current BD ptr
	mov	ax,[si].rbd_status	;get status and count word for BD
	test	ax,04000h		;is count field there?
	jz	ptrupdate_j_1		;no - we give up here.
	add	cl,al			;add the count into cx.
	adc	ch,0
	mov	si,[si].rbd_link	;go to next BD in list
	test	ax,8000h		;is this the last frame?
	je	countbuf		;no - keep counting.

	push	bx
	push	cx

	mov	ax,cs			;we need ds = code.
	mov	ds,ax
	assume	ds:code

	mov	es,base_addr		;get a pointer to their type.
	mov	di,es:[bx].fd_ptr	;get pointer to buffer descriptor
	mov	di,es:[di].rbd_ptr.offs	;get offset of data
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
	call	recv_find		;look up our type.

	pop	cx
	pop	bx
	mov	ds,base_addr		;restore ds to the board.
	assume	ds:nothing

	mov	ax,es			;is this pointer null?
	or	ax,di
	je	ptrupdate		;yes - just free the frame.

	push	cx
	push	es			;remember where the buffer pointer is.
	push	di
  if DAN and 2
	push	cx
  endif

	mov	si,[bx].fd_ptr		;get pointer to buffer descriptor
copybuf:
	mov	dx,si			;save a copy of current BD ptr
	xor	ch,ch			;200 bytes is largest this can be
	mov	cl,byte ptr [si].rbd_status;get count word for BD
	mov	si,[si].rbd_ptr.offs	;get offset of data
  if DAN and 2
	pop	ax
	sub	ax, cx
	jc	copydone
	push	ax
  ENDIf
	call	movemem
	mov	si,dx			;get back current BD ptr
	test	[si].rbd_status,8000h	;check EOF bit
	mov	si,[si].rbd_link	;go to next BD in list
	jz	copybuf			;not done, keep copying it.

  if DAN and 2
	pop	cx
copydone:
  endif
	pop	si			;now give the frame to the client.
	pop	ds
	pop	cx
	assume	ds:nothing

	call	recv_copy
;
;  we are done with the frame, do the list management
;
ptrupdate:
	push	cs
	pop	ds
	assume	ds:code
	mov	es,base_addr		; reload board segment

	mov	si,es:[bx].fd_ptr	; first BD in frame list
nextbd:
	mov	cx,es:[si].rbd_status	; count word for BD, EOF bit
	test	cx,08000h		; EOF bit, if set, save si in lastbd
	jnz	dolastbd
	mov	es:[si].rbd_status,0	; clear status word, EOF bit
	cmp	si,lastbd		; see if we are wrapping
	jz	dolastbd		; yes, just undo it
	mov	si,es:[si].rbd_link	; follow link
	jmp	nextbd
dolastbd:
	mov	di,lastbd		; where end of BD list is now
	mov	lastbd,si		; store last known BD
	mov	es:[si].rbd_size,08000h+200; end of list here
	mov	es:[si].rbd_status,0	; clear status word, EOF bit
; size field for not end of list
	mov	es:[di].rbd_size,200	; remove old end-of-list

;
;  update the FD list flags, new end-of-list
;
	mov	es:[bx].fd_eol,08000h	; store new EOL
	mov	es:[bx].fd_status,0	; clear status word for frame
	mov	di,lastfd		; get old end-of-list
	mov	es:[di].fd_eol,0	; zero old one
	mov	lastfd,bx		; update stored pointer
	mov	si,es:[bx].fd_link	; where next fd is
	mov	firstfd,si		; store that info for next time
  if DAN and 4
	jmp	recv1
  endif

ru_start:
; re-start receive unit
;
;  check to see if the receiver went off because of no resources
;  and restart receiver if necessary
;
	push	cs
	pop	ds
	mov	es,base_addr
	mov	ax,es:[SCB].scb_stat	; status word for SCB
	and	ax,070h		; receiver status
	cmp	al,020h		; receiver has no resources
	jnz	hasres
  if DAN and 8
	cmp	flag, 1
	jnz	ru_start1
	mov	flag, 0
	jmp	recv1
  endif

ru_start1:
	call	count_out_err
;
;  setup lists for starting the RU on the chip
;  we know that there isn't anything in the buffer that we want
;

	mov	bx,firstfd		; get first FD on free list (assume free)
	mov	es:[SCB].scb_rfa,bx	; put into SCB
	mov	si,lastbd		; pointer to a BD, end of chain
	mov	ax,es:[si].rbd_link	; pointer to next BD
	mov	es:[bx].fd_ptr,ax	; set to start of BDs
;
;
;  Start the RU, doesn't need CB, only SCB parms.
;   command, to start receiving again
;
	mov	ax,10h			; start RU
	call	doca_wait
hasres:
;I don't we need to wait here because we haven't done anything to wait for.
	ret


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret


	public	set_address
set_address:
	assume	ds:nothing
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
	cmp	cx,EADDR_LEN		;ensure that their address is okay.
	je	set_address_4
	mov	dh,BAD_ADDRESS
	stc
	jmp	short set_address_done
set_address_4:

;  Next step, load our address into the board
;     reuses the space that the configure command used, with different command
;
	mov	es,base_addr		; set to base address
	mov	es:[SCB].scb_cbl,CCBPTR	; say where conf command is

	mov	di,CCBPTR		; start of config command block
	xor	ax,ax
	stosw				; zero status word for commmand
	mov	ax,8001h		; IA setup command + EL
	stosw
	xor	ax,ax
	dec	ax
	stosw				; set link value to -1 (unused)

	rep	movsb			; move their ethernet address in.
;
;  start the IA setup command
;
	mov	ax,100h			; do-command command
	call	doca_wait
	jnc	set_address_okay
	mov	dh,-1			; no error in the list applies.
	jmp	short set_address_done
set_address_okay:
	mov	cx,EADDR_LEN		;return their address length.
	clc
set_address_done:
	push	cs
	pop	ds
	assume	ds:code
	ret


end_resident	label	byte

timeout_msg	db	"Timed out while initializing the board.",CR,LF,'$'
our_address	db	6 dup(?)	;temporarily hold our address

tdr_warn_msg	db	"TDR: ",'$'
tdr_ok_msg	db	"Ok",CR,LF,'$'
tdr_none_msg	db	"Ethernet card doesn't seem to be plugged in.",CR,LF,'$'
tdr_open_msg	db	" clocks away is an OPEN (not completely reliable)",CR,LF,'$'
tdr_short_msg	db	" clocks away is a SHORT (not completely reliable)",CR,LF,'$'

mem8_16		db	2		; 1 for 16k, 2 for 8k

	extrn	set_recv_isr: near
	extrn	maskint: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

;enter with ax = number to print.
	extrn	decout: near

timeout_error:
	mov	dx,offset timeout_msg
	jmp	short error
error:
	mov	ah,9
	int	21h
	stc
	ret

;
;  data for configuring and setting up the board
;
;  chip always looks at SCP for config info which points to ISCP for the
;  pointer to the CONTROL BLOCK which handles everything from there.
;  Kind of indirect, but it works.
;
SCP	DB	0			; bus use flag (0=16 bit, 1=8 bit).

	public	etopen
etopen:
	mov	al,int_no
	call	maskint			;disable these interrupts.

;  Initialize the Ethernet board, set receive type.
;
;  check for correct EPROM location
;
	call	check_board

;
;  Turn off interrupts, I don't want them
;
  ifdef IOINTOF
	loadport
	setport IOINTOF
	out	dx,al
  endif
;
;  Disconnect from network
;
  ifdef IODIS
	loadport
	setport	IODIS
	out	dx,al
  endif

;
;  Initialize the Ethernet board.
;
	sub	base_addr,0e00h
	mov	di,0e000h		;our initial base address.
	mov	si,ISCPTR-2		;try the init down a little.

;
;  Now discern the end of memory by repeatedly re-initializing the board
;  until the BUSY flag in the ISCP gets reset.
;
re_discern:
	mov	es,base_addr		;remember where we think it starts.
	call	init_root		;did we find our memory size?
	jc	confng			;no, keep trying.
	inc	si			;yes, see if we found the real one.
	inc	si
	call	init_root		;try initializing it in a different locn.
	jnc	confok			;it worked!  we've found the root.
	dec	si			;it didn't work, keep trying.
	dec	si
confng:
	or	di,di			;did we try all 64K?
	je	confbad			;yes.

	add	base_addr,200h		;advance the segment by 2000h bytes.
	sub	di,2000h		;retreat the offset by 2000h bytes.
	jmp	re_discern		;try this next higher address.
confbad:
	sti
	jmp	timeout_error

confok:
	mov	memory_begin,di
	call	reconfigure
	jc	confbad

;
;  Ask the board for the Ethernet address, and then use set_address to set it.
;
	push	ds
	pop	es
	mov	di,offset our_address
	mov	cx,EADDR_LEN
	call	get_address

	mov	si,offset our_address
	mov	cx,EADDR_LEN
	call	set_address
	jnc	store_address_2
	sti
	jmp	timeout_error
store_address_2:
;
;  IA sent, setup all of the other data structures on the board
;  start with xmit command descriptors
;
	mov	di,TCBPTR
	mov	es:[di].tcb_status,0
	mov	es:[di].tcb_com,08004h
	mov	es:[di].tcb_link,-1
	mov	es:[di].tcb_ptr,TBDPTR

	add	di,(size tcb_struc)

	mov	es:[di].tbd_status,0
	mov	es:[di].tbd_link,0
	mov	es:[di].tbd_ptr.offs,TBUFPTR
	mov	es:[di].tbd_ptr.segm,0

; Note that we allocate fd's, rbd's, and buffers all at the same time.  This
; doesn't mean that each pair of fd's and rbd's necessarily have anything to
; do with each other.  We just allocate them together because we want to have
; the same number of each, and it's easier to compute that way.

	mov	di,TBUFPTR		;get the last buffer.

	mov	ax,di			;compute the amount of free memory.
	sub	ax,memory_begin
	xor	dx,dx
	mov	bx,RBUF_TOTAL		;each buffer takes this much.
	div	bx
	mov	cx,ax			;put the number of buffers into cx.

init_rbuff_0:
	sub	di,RBUF_TOTAL		;back the pointer down by a little.

;init the FD.
	mov	es:[di].fd_status,0
	mov	es:[di].fd_eol,0
	mov	es:[di].fd_ptr,-1
	lea	ax,[di]-RBUF_TOTAL	;get the address of the next buffer.
	mov	es:[di].fd_link,ax

;init the BD.
	lea	bx,[di + (size fd_struc)]
	mov	es:[bx].rbd_status,0
	lea	ax,[bx-RBUF_TOTAL]	;make a pointer to the next BD
	mov	es:[bx].rbd_link,ax
	lea	ax,[bx+(size rbd_struc)]	;make a pointer to the buffer.
	mov	es:[bx].rbd_ptr.offs,ax
	mov	es:[bx].rbd_ptr.segm,0
	mov	es:[bx].rbd_size,RBUFLEN	;length of the buffer.

	loop	init_rbuff_0

init_rbuff_1:
;patch the parameters of the last FD and BD so they link around to the head.
	mov	es:[di].fd_eol,8000h
	mov	es:[di].fd_link,FDBASE
	mov	lastfd,di

	lea	bx,[di + (size fd_struc)]
	mov	es:[bx].rbd_link,FDBASE + (size fd_struc)
	mov	es:[bx].rbd_size,RBUFLEN + 8000h
	mov	lastbd,bx

;now put the location of the first rbd into the first fd.
	mov	es:[FDBASE].fd_ptr,FDBASE  + (size fd_struc)

	call	enable_network

	mov	dx,offset tdr_warn_msg	;warn them that the 82586 TDR sucks.
	mov	ah,9
	int	21h

;
; Test to see if the network is okay.
;
	mov	di,CCBPTR		; start of config command block
	xor	ax,ax			; zero status word for commmand
	stosw
	mov	ax,8005h		; TDR command + EL
	stosw
	xor	ax,ax
	dec	ax
	stosw				; set link value to -1 (unused)
	inc	ax
	stosw				; zero time result.

	mov	ax,100h			; do-command command
	call	doca_wait
	jnc	do_tdr_2		; finished okay.
	mov	ax,2000h		; treat a timeout as an open
	jmp	short do_tdr_5
do_tdr_2:
	mov	ax,word ptr es:[CCBPTR].tdr_time
do_tdr_5:
	mov	dx,offset tdr_ok_msg
	test	ax,8000h
	jne	do_tdr_3
	mov	dx,offset tdr_short_msg
	test	ax,2000h
	je	do_tdr_4
	mov	dx,offset tdr_none_msg
	cmp	ax,2000h
	je	do_tdr_3
	mov	dx,offset tdr_open_msg
do_tdr_4:
	push	dx
	and	ax,2048-1
	xor	dx,dx
	call	decout
	pop	dx
do_tdr_3:
	mov	ah,9
	int	21h

;
;  Start the RU, doesn't need CB, only SCB parms.
;   command, to start receiving
;
	mov	es:[SCB].scb_rfa,FDBASE	; set to frame descriptors
	mov	ax,10h			; start RU
	call	doca_wait
;
; Now reset CX, FR, CNA, and RNR so that we don't get a spurious interrupt.
;
	mov	ax,es:[SCB].scb_stat	;get the status.
	and	ax,0f000h		;isolate the ACK bits.
	mov	es:[SCB].scb_com,ax	;make a command to
					;acknowledge the interrupt.
	call	doca
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

	mov	dx,offset end_resident
	clc
	ret


init_root:
;enter with es:di -> beginning of our system memory window,
;  si -> place to put ISC.
;exit with nc if it worked, cy if it didn't work.

	mov	al,SCP
	mov	es:[SCPTR].scp_bus,al
	mov	es:[SCPTR].scp_ptr.offs,si
	mov	es:[SCPTR].scp_ptr.segm,0

	mov	es:[si].iscp_busy,1		;set busy bit.
	mov	es:[si].iscp_offset,SCB		;point to the SCB.
	mov	es:[si].iscp_base.offs,0	;scb base.
	mov	es:[si].iscp_base.segm,0	;scb base.

	call	reset_586		; reset the 586, hardware-specific.

;
;  Issue a CA to initialize the chip after reset
;
	call	doca

	mov	ax,2			;don't wait too long.
	call	set_timeout
init_root_2:
	cmp	es:[si].iscp_busy,0	;did it clear the busy flag?
	je	init_root_1		;yes.
	call	do_timeout
	jne	init_root_2
	stc
	ret
init_root_1:
	clc
	ret
