version	equ	1

	include	defs.asm	;SEE ENCLOSED COPYRIGHT MESSAGE

; PC/FTP Packet Driver Source, conforming to version 1.08 of spec
; Krishnan Gopalan and Gregg Stefancik, Clemson Univesity Engineering
; Computer Operations.
; Date: September 1, 1989
; Portions of the code have been adapted from the 3c505 driver for NCSA
; Telnet by Bruce Orchard and later modified by Warren Van Houten and krus
; @diku.dk.

; Permission is granted to any individual or institution to use,copy,
; modify or redistribute this software provided this notice is retained.
; The authors make no guarantee to the suitability of the software for
; any purpose. Any damage caused by using this program is the responsibility
; of the user and not the authors. 

code	segment	word public
	assume	cs:code, ds:code

;	3c505 card definitions
;control register	bit definitions
EN_ATTN			equ	0200q
EN_FLSH_DATA		equ	0100q
EN_DMA_ENABLE		equ	0040q
EN_TO_HOST		equ	0020q
EN_TERMINAL_CNT_ENBLE 	equ	0010q
EN_COMMAND_ENABLE	equ	0004q
EN_HSF2			equ	0002q
EN_HSF1			equ	0001q

;status register	bit definitions

EN_DATA_READY		equ	0200q
EN_HOST_COMMAND_EMPTY	equ	0100q
EN_ADAPTER_COMMAND_FULL equ	0040q
EN_TO_HOST		equ	0020q
EN_DMA_DONE		equ	0010q
EN_ASF3			equ	0004q
EN_ASF2			equ	0002q
EN_ASF1			equ	0001q
; auxiliary dma register  bit definition
EN_BURST		equ	0001q

; timeout values
SECOND		EQU	36
RESDEL		EQU	6
RESTO		EQU	15*SECOND
CMDBTO		EQU	6
CMDCTO		EQU	6
RETRYDELAY  	EQU	6
RCMDTO		EQU	6
RESPTO		EQU	6

;port addresses
ECOMMAND	equ	0
ESTATUS 	equ	2
EDATA 		equ	4
ECONTROL 	equ	6

recvbuf		db  4096 dup(?)		; size of receive buffer
rbufct		dw	?		; recv buffer count

pcblen		dw 	?		; length of pcb
pcbaddr		dw	?		; address of pcb

lastcon		db	?		; last control to board
cmdlen		dw	?		; length of command

cbsh		equ	50
cbs		equ	cbsh*2	

icmdb		db	cbs dup(?)
icmd		db	cbsh dup(?)

fconc		db	0	; flag configure 82586
fgeth		db	0	; flag:get ethernet address
fseth		db	0	; flag:set ethernet address
fxmit		db	0	; flag:transmit packet

cconc		db	02h	; command configure 82586
		db	2	; 2 more bytes
		dw	1	; receive broadcasts

rconc		db	2 dup(?);Response; configure
rconc_st	dw	?	; status

cgeth		db	03h	; command get ethernet address
		db	00	; no more bytes

txmit		db	09h	; command ; transmit packet
		db	06	; 6 more bytes
tx_offset	dw	?	; offset of host transmit buffer 
tx_segment	dw	?	; segment of host transmit buffer 
tx_length	dw	?	; packet length

rxmit		db	2 dup(?); Response tx packet
rx_offset	dw	?	; buffer offset
rx_segment	dw	?	; buffer segment
rx_status	dw	?	; completion status
rx_cstatus	dw	?	; 82586 status

rgeth		db	2 dup(?); Response get Ethernet address
rgeth_ad	db	6 dup(?); -- address

cseth		db	10h	; command :set station address
		db	6	; 6 more bytes
cseth_ad	db	6 dup(?); ethernet address

rseth		db	2 dup(?); response set ethernet address
rseth_status	dw	?	; status

crecv		db	08h	; command receive
		db	08	; 8 more bytes
crecv_offset	dw	?	; buffer offset
crecv_segment	dw	?	; buffer segment
crecv_length	dw	?	; buffer length
crecv_timeout	dw	?	; timeout

rr		db	2 dup(?); Response; receive
rr_offset	dw	?	; buffer offset
rr_segment	dw	?	; buffer segment
rr_dmalen	dw	?	; buffer dmalen
rr_length	dw	?	; packet length
rr_status	dw	?	; completion status
rr_rstatus	dw	?	; 82586 receive status
rr_time		dd	?	; time tag
		
public	int_no,io_addr,mem_base

int_no 	db	2,0,0,0			;must be four bytes long for get_number.
io_addr dw	0300h,0			; io addr for card(jumpers)
mem_base dw	0d000h,0		; shared memory address(software)

public	driver_class, driver_type, driver_name, driver_function, parameter_list

driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec 
driver_type	db	2		;from the packet spec 
driver_name	db	'3C505',0	;name of the driver.  
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
rcv_modes	dw	4		;number of receive modes in our table.
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
send_pkt:
;enter with es:di->upcall routine, (0:0) if no upcall is desired.
;  (only if the high-performance bit is set in driver_function)
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
	assume	ds:nothing

	push	si			; save si for  lodsw
	push	ds			; save ds
	push	es			; save es for timer
	push	cs			;
	pop	ds			; cs = ds align

	mov	tx_segment,ds		;tx->data
	mov	tx_offset,si		;tx_offset ->segment offset 

	mov	ax,cx			; save cx
	cmp	ax,60			; compare with minimum limit
	jnb	pkt_ok			; go if ok 
	mov	ax,60			; make length minimum allowed
pkt_ok:
	inc	ax			; round up
	sar	ax,1			; divide by 2
	shl	ax,1			; multiply by 2
	mov	tx_length,ax		; put in request
	
	mov	fxmit,0			; clear response received flag

	mov	si, offset txmit	; si->request(pcb)
	mov	ax,8			;length of pcb 

	call	outpcb			; send command to board

	mov	cx,tx_length
	sar	cx,1

	pop	es
	pop	ds
	pop	si

tx_1:

	lodsw

	loadport
	setport	EDATA
	out	dx,ax			; output word
	setport	ESTATUS
tx_2:
	in	al,dx			; get status
	test	al, EN_DATA_READY	; ready for next word ?
	jz	tx_2			;no

	dec	cx			; count word
	jnz	tx_1			; loop thru buffer

tx_3:
	test	fxmit,0ffh		; is transmit over
	jz	tx_3			; no
	ret

	public	get_address


get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	push	es
	push	di

	cmp	cx, EADDR_LEN		; does caller want reasonable length?
	jb	get_addr_fail		; no , fails
	mov	si,offset cgeth		; si->pcb address
	mov	ax,2			; length of pcb
	mov	fgeth,0			; clear response received flag

	call	outpcb		

	mov	ax,RESTO		; get wait time
	call	set_timeout
get_addr_1:
	test	fgeth,0ffh		; answered?
	jnz	get_addr_2		; yes
	call	do_timeout
	jnz	get_addr_1		; test again
	jmp	get_addr_fail		; go return

get_addr_2:
	pop	di
	pop	es
	cld				; make sure of string operation
	mov	si,offset rgeth_ad	; load source of address
	mov	cx,EADDR_LEN		; return length of address
	rep	movsb			; copy address
	mov	cx,EADDR_LEN		; return length of address
	clc
	ret
get_addr_fail:
	add sp,4
	stc
	ret

	public	set_address

set_address:
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
	assume	ds:nothing

	cmp	cx, EADDR_LEN		; check if address ok
	je	set_addr_1	
	mov	dh,BAD_ADDRESS		; don't like length
	stc
	ret
set_addr_1:
	mov	di,offset cseth_ad	;di->destination offset
	rep	movsb			; return address
	mov	si,offset cseth		; si->request ethernet address
	mov	ax,8			; request length -> ax
	mov	fseth,0			; clear response received flag
	call 	outpcb			; send the pcb

	mov	ax,RESTO		; get response time
	call	set_timeout
set_addr_2:
	test	fseth,0ffh		; pcb answered?
	jnz	set_addr_3
	call	do_timeout		; has time expired
	jne	set_addr_2		; no, go back
	stc				; error
	ret
set_addr_3:
	mov	cx,EADDR_LEN		;return their address length.
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


	public	terminate
terminate:
	ret

	public	reset_interface
reset_interface:
;reset the interface.
	assume	ds:code

	mov	al, EN_ATTN OR EN_FLSH_DATA	; hard reset command
	loadport
	setport	ECONTROL
	out	dx,al				; do reset
	mov	ax,RESDEL			; get reset delay
	call	set_timeout
rst1:
	call	do_timeout
	jne	rst1				; wait for reset
	mov	al,EN_COMMAND_ENABLE		; command interrupt enable
	mov	lastcon,al			; save last command
	out	dx,al				; release reset
	mov	ax,RESDEL
	call	set_timeout
rst2:
	call	do_timeout
	jne	rst2				; wait to start reset
	mov	ax,RESTO			; add time out
	call	set_timeout
rst3:
	call getstat				; getstatus	
	and	ax,EN_ASF1 OR EN_ASF2		; 
	cmp	ax,EN_ASF1 OR EN_ASF2		;  both on ?
	jne 	resdone
	call	do_timeout			; long enough?
	jne	rst3				; no
	ret
resdone:
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
; all interrupts come here
;Upon exit, the interrupt will be acknowledged.
	assume	ds:code
	;sti
	cld
recv1:
	loadport
	setport	ESTATUS
	in	al,dx				; read flags
	and	al,EN_ADAPTER_COMMAND_FULL	; 
	jnz	recv2				; yes,full
	jmp	interrupt_done	
;there might be a response, clear flags to check for response
recv2:
	mov	ax,RCMDTO			; incoming command time out
	call	set_timeout

	mov	al,lastcon			; last control->ax
	and	al,NOT (EN_HSF1 OR EN_HSF2)	; clear flags
	mov	lastcon,al

	loadport
	setport	ECONTROL
	out	dx,al				; clear flags in cntl reg

	mov	di,offset icmdb			; di->incoming command buffer

recv3:
	loadport
	setport	ESTATUS
	in	al,dx				; status->al

	mov	cx,ax				; status->cx
	test	al,EN_ADAPTER_COMMAND_FULL	; is adapter register full
	jnz	recv4				; yes go ahead to read from
	call	do_timeout			; is time up ? 
	jne	recv3				; give some more time
	jmp	int1				; give up
recv4:
	loadport
	setport	ECOMMAND
	in	al,dx				; get command byte
	and	cl,EN_ASF1 OR EN_ASF2		
	cmp	cl,EN_ASF1 OR EN_ASF2		; both on?
	je	recv5				; end of command
	mov	[di],al				;save byte
	inc	di				;inc	di 
	mov	ax,di				; current pointer->ax
	sub	ax,offset icmdb			; - start of buffer
	cmp	ax,cbs				; full?
	jl	recv3

	mov	si,(offset icmdb)+cbsh		; si->middle of buffer
	mov	di,offset icmdb			; di->start of buffer
	mov	cx,cbsh				; size of half buffer->cx
	push	ds
	pop	es
	rep	movsb

	jmp	recv3				; loop for more bytes
recv5:
	push	ds
	pop	es
	mov	ah,0
	mov	cmdlen,ax			; save cmdlen
	mov	si,di 
	sub	si,cmdlen
	mov	di,offset icmd	
	mov	cx,cmdlen

	rep movsb				; copy

	mov	al,icmd				; check

	cmp	al,32h				; Configure?
	je	respconfig

	cmp	al,33h				; get ethernet address?
	je	respgetaddr			; yes

	cmp	al,39h				; transmit complete?
	je	transmit			; yes

	cmp	al,40h				; set ethernet address
	je	respsetaddr			; yes
	
	cmp	al,38h				; receive complete?
	je	resprecv

	jmp	int1

respconfig:

	push	ds
	pop	es
	mov	si,offset icmd
	mov	di,offset rconc
	mov	cx,2
	
	rep	movsw
	mov	fconc,1
	jmp	int1
	
respgetaddr:
	push	ds
	pop	es

	mov	si,offset icmd
	mov	di,offset rgeth
	mov	cx,4

	rep	movsw

	mov	fgeth,1
	jmp	int1

respsetaddr:

	mov	si,offset icmd		; si->command received
	mov	di,offset cseth		; di->set ethernet address resp
	push	ds
	pop	es
	mov	cx,2

	rep	movsw
	mov	fseth,1
	jmp	int1

transmit:
	mov	si,offset icmd			; si->command received
	mov	di,offset rxmit			; response,transmit packet
	push	ds
	pop	es
	mov	cx,5				; response length->cx
	rep	movsw

	mov	fxmit,1				; response received
	jmp	int1				; return from interrupt

resprecv:
	mov	si,offset icmd			; si->command received
	mov	di,offset rr			; di->receive response
	push	ds
	pop	es
	mov	cx,9				; response length->cx
	
	rep	movsw				; move response
	mov	di,offset recvbuf		; di->receive buffer
	mov	ax,rr_length			; message size->ax
	inc	ax				; round up
	shr	ax,1				; convert to words
	shl	ax,1				; convert to characters
	mov	rr_length,ax			; ax->message length

	mov	cx,ax				; message length->cx
	shr	cx,1				; convert to words

	mov	al,lastcon			; lastcontrol->al
	or	al,EN_TO_HOST OR EN_HSF1	; set direction & ack	
	mov	lastcon,al			; response

	loadport
	setport	ECONTROL
	out	dx,al				; pass direction
	
	setport ESTATUS

resprecv_1:

	in	al,dx				; get status
	test	al,EN_DATA_READY		; data ready ?
	jz	resprecv_1

	setport	EDATA				; data word->ax
	in	ax,dx				; get word
	stosw					; store word in buffer

	setport	ESTATUS
	dec	cx				; count word
	jnz	resprecv_1			; loop if more words

	mov	al,lastcon
	and	al,NOT (EN_TO_HOST OR EN_HSF1)
	mov	lastcon,al

	loadport				; dx->control register
	setport	ECONTROL
	out	dx,al

	mov	cx,rr_length			; cx->packet length

	push	cs				; align cs and ds
	pop	ds

	mov	di,offset recvbuf		; reset di to beginning
	add	di,EADDR_LEN+EADDR_LEN		; point to type field
						; of buffer
	assume	ds:code

	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	inc	di			;set di to 802.2 header
	inc	di
	mov	dl, IEEE8023
BlueBookPacket:

	call	recv_find			; first call

	assume	ds:nothing
	mov	ax,es				; ax->es
	or	ax,di				; is es=di=0?
	je	rcv_no_copy

	mov	cx,rr_length			; cx->packet length
	push	cx
	push	es
	push	di

	mov	si,offset recvbuf		; prepare to copy the packet
	rep	movsb				; copy

	pop	si
	pop	ds
	pop	cx

	call	recv_copy			; second call 
rcv_no_copy:
	push	cs
	pop	ds
	call	anotherrecv			; start another recv
int1:
	jmp	recv1

interrupt_done:
	ret

;*****************************OUTPCB***********************************
;outpcb	 send pcb to board, retry until accepted
; entry	ax = number of bytes in pcb
;	si = address of pcb
outpcb	proc 	near
	mov	pcblen,ax		; save pcb length
	mov	pcbaddr,si		; save pcb address
pcb_0:
	mov	cx,pcblen		;length->cx
	mov	si,pcbaddr		; address->si
	cli				; stop interrupts
	mov	al,lastcon		; save last command
	and	al, NOT (EN_HSF1 OR EN_HSF2); clear flags
	mov	lastcon,al
	sti				; enable interrupts
	loadport
	setport	ECONTROL
	out	dx,al			; send control
	setport	ECOMMAND
pcb_1:
	mov	al,[si]			; 
	out	dx,al			; send command byte
	mov	ax,CMDBTO		; get time out
	call	set_timeout
chk_hcre:
	call	getstat
	and	al, EN_HOST_COMMAND_EMPTY	; command accepted
	jne	pcb_2				; go on
	call	do_timeout
	jne	chk_hcre			; time is still left
	jmp	cmdretry			; retry command
pcb_2:
	inc	si				; increment source pointer
	dec	cx				; count byte
	jg	pcb_1				; loop 		
	loadport
	setport	ECONTROL
	cli					; disable interrupts
	mov	al,lastcon			; last control -> al
	or	al,(EN_HSF1 OR EN_HSF2)		; set end of command
	mov	lastcon,al			; save last control
	out	dx,al				; send flag bits
	setport	ECOMMAND
	mov	ax,pcblen
	out	dx,al				; send pcb length
	sti					; enable interrupts
	mov	ax,CMDCTO			; time out for acceptance
	call	set_timeout
pcb_3:
	call	getstat
	and	al,(EN_ASF1 OR EN_ASF2)		; just keep status flags
	cmp	al,1				; accepted
	je	cmdaccept
	cmp	al,2
	je	cmdretry
	call	do_timeout
	jne	pcb_3

cmdretry:
	mov	ax,RETRYDELAY			;add retry delay
	call	set_timeout
pcb_4:
	call	do_timeout
	jne	pcb_4
	jmp	pcb_0

cmdaccept:
	cli
	mov	al,lastcon
	and	al, NOT (EN_HSF1 OR EN_HSF2)	; turn off end of command flag
	mov	lastcon,al			; save last control
	sti					; enable interrupts
	loadport
	setport	ECONTROL
	out	dx,al				; pass control byte
	ret	
outpcb	endp
;*************get status of board***********************
getstat		proc	near
	push	bx
	push	dx
	loadport
	setport	ESTATUS

stat_1:
	in	al,dx			; get status into al
	mov	bl,al			; save al
	in	al,dx			; get status again
	cmp	al,bl			; same status ?
	jne	stat_1
	pop	dx
	pop	bx
	ret
getstat	endp

; UPDATE BUFFER POINTERS AND ISSUE ANOTHER RECV
;	update recv 
anotherrecv	proc	near
	mov	ax,rbufct
	mov	crecv_offset,ax
	inc	ax
	mov	rbufct,ax
	mov	ax,10
	mov	si,offset crecv

	call	outpcb
	ret
anotherrecv	endp

	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret

	include	timeout.asm

;any code after this will not be kept after initialization.
end_resident	label	byte

	public	usage_msg
usage_msg  db  "usage: 3C505 [-n] [-d] [-w] <packet_int_no> <int_level> <io_addr> <mem_base>",CR,LF,'$'
	public	copyright_msg
copyright_msg    db    "Packet Driver for 3c505, version ",'0'+majver, ".",CR,LF
		 db	"Portions copyright 1989, Krishnan Gopalan & Gregg Stefancik.",CR,LF
		 db	 "Clemson University Engineering Comp Ops.",CR,LF,'$'
no_board_msg:
	db	"3c505 apparently not present at this address.",CR,LF,'$'
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
	call 	get_number
	mov	di,offset io_addr
	call	get_number
	mov	di,offset mem_base
	call	get_number
	clc
	ret

	public	etopen
etopen:
;if all is okay,
; reset the 3c505 board takes about 15-20 seconds
	loadport
	setport	ECONTROL 
	mov	al,EN_ATTN OR EN_FLSH_DATA	; reset command
	out	dx,al				; do reset
  if 0
	mov	ax,1
	call	set_timeout
reset1:
	call	do_timeout
	jne	reset1				; wait for reset to propagate
  else
  endif

	mov	al,EN_COMMAND_ENABLE		; command interrupt enable
	mov	lastcon,al			; save last command
	out	dx,al				; release reset
	mov	ax,RESDEL			; time to wait
	call	set_timeout
reset2:
	call	do_timeout
	jne	reset2

	mov	ax,RESTO			;this is the long wait.
	call	set_timeout
reset3:
	call	getstat
	and	ax, EN_HSF1 OR EN_HSF2
	cmp	ax,EN_HSF1 OR	EN_HSF2
	jne	resetdone
	call	do_timeout
	jne	reset3
	jmp	openfail			; open failed

resetdone:
	call	set_recv_isr			;install the interrupt handler

;Tell the 3c505 board to receive packets
	mov	si,offset cconc
	mov	ax,4
	mov	fconc,0

	call	outpcb

	mov	ax,RESTO
	call	set_timeout
open_1:
	test	fconc,0ffh
	jnz	setuprecv

	call	do_timeout
	jne	open_1

; set up the recv buffers

setuprecv:
	mov	rbufct,0			; clear buffer counter
	mov	crecv_length,1600		; buffer length,1600

	mov	crecv_timeout,0

startrecv:
	mov	ax,rbufct			; buffer counter->ax
	mov	crecv_offset,ax			; buf no used as offset
	inc	ax				; count buffer
	mov	rbufct,ax
	mov	ax,10				; length of pcb
	mov	si,offset crecv			;si->command receive

	call	outpcb				; pass the pcb
	mov	ax,rbufct
	cmp	ax,10
	jl	startrecv

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
	call 	print_number
	mov	di,offset io_addr
	mov	dx,offset io_addr_name
	call	print_number
	mov	di,offset mem_base
	mov	dx,offset mem_base_name
	call	print_number
	ret

;if we got an error,
openfail:
	stc
	ret
code	ends
	end
