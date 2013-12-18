;History:175,1

version	equ	0

	include	defs.asm

comment \

Questions:

Why two sets of documentation?

	Program to old spec, keeping new in mind...

Why does arlan.cfg contain the base address and irq number?

	Read the irq first.  If it's nonzero, it's true, otherwise
	use the config file.

	just look for telesystems

If I give a node id to arlan.cfg, will it override the switches?

	Only if it's non-zero.

Note from Tom says System Parameters are at 200h

Should I read all 512 bytes of arlan.cfg into 100h?

	No, only the last 256.

Is the configured status byte only present on machines w/ NV RAM?

\


arlan_segment	segment at 0

		org	000h
ar_signature	db	?

		org	030h
ar_reset	db	?

		org	031h
ar_diagnostics	db	?

		org	040h
ar_node_id	db	EADDR_LEN dup(?)	;6 byte node address field.

		org	046h
ar_node_bcast	db	EADDR_LEN dup(?)	;6 byte broadcast address.

		org	04ch
ar_type		db	?		;1 byte hardware type
ar_type_A450	equ	00h
ar_type_A650	equ	01h
ar_type_A670	equ	0bh
ar_type_A670E	equ	0ch
ar_type_A650E	equ	0dh
ar_type_A440LT	equ	0eh

		org	04dh
ar_version	label	word		;Version number.
ar_version_maj	db	?
ar_version_min	db	?

		org	080h
ar_interrupt	db	?		;not used by LANCPU

		org	081h
ar_control_i	db	?		;image of the control register.

		org	090h
ar_command	db	?

COM_CONF	equ	1
COM_RX_ENABLE	equ	3
COM_RX_ABORT	equ	4
COM_TX_ENABLE	equ	5
COM_TX_ABORT	equ	6
COM_NOP		equ	7
COM_INT		equ	80h

		org	0a0h
ar_rx_status	db	?

		org	0a2h
ar_rx_offset	dw	?		;start of received datagram

		org	0a4h
ar_rx_length	dw	?		;length of received datagram

		org	0a6h
ar_rx_src	db	EADDR_LEN dup(?)	;RX source address.

		org	0ach
ar_rx_bcast	db	?		;<>0 if received frame was bcast.

		org	0adh
ar_rx_quality	db	?		;indicates quality of received packet.

		org	0b0h
ar_tx_status	db	?

		org	0b1h
ar_tx_quality	db	?

		org	100h
ar_sys_params	label	byte

		org	108h
ar_irq_level	db	?		;IRQ level

		org	109h
ar_spreading	db	3 dup(?)	;Spread spectrum code ID.

		org	10ch
ar_NID		dw	?		;Radio address of LAN card.

		org	11dh
ar_tx_atten	db	?		;attenuation of radio transmitter in db.

		org	11eh
ar_system_id	dd	?		;system ID.

		org	128h
ar_MDS		dw	?		;Maximum Datagram Size.

		org	12ah
ar_MFS		dw	?		;Maximum Frame Size.

		org	12ch
ar_max_retry	db	?

		org	162h
ar_register	db	?		;indicates if card must register w/ router.

		org	164h
ar_poll_rate	dw	?		;<>0 if power saving is used.

		org	166h
ar_refresh_rate	dw	?		;tens of msecs between registration
					;refreshes.
		org	168h
ar_name		db	16 dup(?)

		org	400h
ar_tx_buffer	label	byte

		org	0c00h
ar_rx_buffer	label	byte

		org	1fffh
ar_control	db	?
CONTROL_RESET	equ	1
CONTROL_CA	equ	2
CONTROL_IE	equ	4
CONTROL_CLRI	equ	8


arlan_segment	ends

code	segment	word public
	assume	cs:code, ds:code

	public	int_no
int_no	db	0,0,0,0			;must be four bytes long for get_number.

base_addr	dw	?		;The base address of the board.

xmit_bdcast	db	?

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK,0	;from the packet spec
driver_type	db	99		;from the packet spec
driver_name	db	'ARLAN 450',0	;name of the driver.
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


	include	timeout.asm
	include	movemem.asm

	public	send_pkt
send_pkt:
;enter with es:di->upcall routine, (0:0) if no upcall is desired.
;  (only if the high-performance bit is set in driver_function)
;enter with ds:si -> packet, cx = packet length.
;if we're a high-performance driver, es:di -> upcall.
;exit with nc if ok, or else cy if error, dh set to error number.
	assume	ds:nothing

	mov	es,base_addr
	assume	es:arlan_segment

	cmp	cx,ar_MDS		;Is this packet too large?
	mov	dh,NO_SPACE
	ja	send_pkt_toobig		;yes, don't bother sending it.

  if 0
	mov	bx,2			;count the number of times around...
wait_again:
	mov	ax,10			;don't wait too long...
	call	set_timeout
wait_for_xmit:
	sti
	cmp	ar_tx_status,0		;is the transmit done?
	jne	xmit_done		;yes, exit now.
	call	do_timeout
	jne	wait_for_xmit		;no, wait for it to finish.
	cli

	inc	ar_interrupt		;note that we had to crap out.

	mov	ar_command,COM_TX_ABORT
	call	doca
	mov	ar_tx_status,1
  else
	sti
wait_for_xmit:
	cmp	ar_tx_status,0		;is the transmit done?
	jne	xmit_done		;yes, exit now.
	jmp	wait_for_xmit		;no, wait for it to finish.
  endif
	clc				;pretend we actually sent it.
	ret
send_pkt_toobig:
	stc
	ret
xmit_done:
	cli

	mov	di,offset ar_command + 5

	mov	xmit_bdcast,0

;check to see if it's an Ethernet broadcast (all ones).
	push	ds
	push	si
	push	cx
	mov	cx,EADDR_LEN
send_pkt_1:
	lodsb
	cmp	al,0ffh
	loope	send_pkt_1
	jne	send_pkt_2		;not Ethernet broadcast.

	inc	xmit_bdcast		;remember that it was a broadcast.

	mov	si,offset ar_node_bcast	;use our broadcast address.
	mov	ds,base_addr

	movsw				;move the broadcast address over.
	movsw
	movsw

	pop	cx
	pop	si
	pop	ds

	add	si,EADDR_LEN		;skip the addresses.
	jmp	short send_pkt_3
send_pkt_2:
	pop	cx
	pop	si
	pop	ds

	movsw				;move the destination address over.
	movsw
	movsw

send_pkt_3:
	add	si,EADDR_LEN		;skip the addresses.
	sub	cx,EADDR_LEN+EADDR_LEN	;. .

	mov	di,offset ar_tx_buffer
	mov	al,xmit_bdcast
	stosb
	push	cx
	call	movemem
	pop	cx

	inc	cx			;include the broadcast byte.

	mov	ar_command,COM_TX_ENABLE
	mov	word ptr ar_command[1],offset ar_tx_buffer
	mov	word ptr ar_command[3],cx

	mov	ar_tx_status,0		;let the board fill it in.

	call	doca

	clc
	ret


	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	cmp	cx,EADDR_LEN		; Caller wants a reasonable length?
	jb	get_addr_x		; No, fail.
	mov	cx,EADDR_LEN		; Move one ethernet address from our copy
	mov	si,offset ar_node_id	; Copy from the board.
	push	ds
	mov	ds,base_addr
	rep     movsb
	pop	ds
	mov	cx,EADDR_LEN		; Tell caller how many bytes we fed him
	clc				; Carry off says success
	ret
get_addr_x:
	stc				; Tell caller our addr is too big for him
	ret


	public	set_address
set_address:
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
	assume	ds:nothing
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
	clc
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
	assume	ds:code, es:arlan_segment
	mov	es,base_addr

;clear the interrupt request.

	mov	al,ar_control_i	;drop the clear interrupt bit.
	and	al,not CONTROL_CLRI
	mov	ar_control,al
	or	al,CONTROL_CLRI		;raise the clear interrupt bit.
	mov	ar_control,al

	cmp	ar_rx_status,0		;was this our receive interrupt?
	jne	recv_recv
	cmp	ar_tx_status,0		;was this our transmit interrupt?
	jne	recv_xmit
	jmp	recv_exit
recv_xmit:
	jmp	recv_exit

recv_recv:
	mov	di,ar_rx_offset		;get a pointer to the packet.
	mov	cx,ar_rx_length		;get the length.
	dec	cx			;omit the "was broadcast" flag.
	inc	di			;. .
	add	cx,EADDR_LEN+EADDR_LEN	;add the two headers in.
	push	es
	push	di
	push	cx
	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	call	recv_find
	pop	cx
	pop	si
	pop	ds
	assume	ds:arlan_segment, es:nothing

	mov	ax,es			;is this pointer null?
	or	ax,di
	je	recv_free		;yes - just free the frame.

	push	es
	push	di
	push	cx
	sub	cx,EADDR_LEN+EADDR_LEN
	mov	al,ds:[si-1]		;get the "was broadcast" flag.
	cmp	al,0			;was it a broadcast?
	je	recv_us			;no.
	mov	ax,0ffffh		;yes, stuff an Ethernet broadcast in.
	stosw
	stosw
	stosw
	jmp	short recv_source
recv_us:
	mov	si,offset ar_node_id
	movsw
	movsw
	movsw
recv_source:
	mov	si,offset ar_rx_src	;move the source address over.
	movsw
	movsw
	movsw
	mov	si,ar_rx_offset		;get a pointer to the packet.
	inc	si
	rep	movsb
	pop	cx
	pop	si
	pop	ds
	assume	ds:nothing

	call	recv_copy

recv_free:

	push	cs
	pop	ds
	assume	ds:code
	mov	es,base_addr
	assume	es:arlan_segment

enable_receive:

	mov	ar_rx_status,0		;clear the current status.
	mov	ar_command,COM_RX_ENABLE+COM_INT
	mov	ar_command+1,1		;receive broadcasts.
	call	doca

	cmp	ar_rx_status,0		;is there another packet?
	jne	recv_recv		;yes, receive it now.

recv_exit:
	ret


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret

doca:
;toggle the CA bit in the control register.
;must be executed with interrupts off to protect ar_control_i.
;return cy if we had a horrible failure.
	assume	es:arlan_segment
	pushf				;make up a fake iret stack frame.
	cli
	mov	al,ar_control_i	;toggle the bit in the image
	xor	al,CONTROL_CA
	mov	ar_control_i,al	;and store them both.
	mov	ar_control,al
	popf

	mov	ax,5			;wait about 1/7th of a second.
	call	set_timeout
doca_2:
	cmp	ar_command,0		;wait for the command to finish.
	je	doca_1			;it did.
	call	do_timeout
	jne	doca_2
	stc
	ret
doca_1:
	clc
	ret

;any code after this will not be kept after initialization.
end_resident	label	byte


	extrn	error: near

	public	usage_msg
usage_msg	db	"usage: ar450 [-n] [-d] [-w] <packet_int_no>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for an ARLAN 450, version ",'0'+majver,".",'0'+version,CR,LF
		db	"Portions Copyright 1990, Russell Nelson",CR,LF,'$'

	extrn	set_recv_isr: near

;enter with si -> argument string, di -> wword to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> argument string, di -> wword to print.
	extrn	print_number: near

	public	parse_args
parse_args:
;exit with nc if all went well, cy otherwise.
	clc
	ret


signature	db	"TELESYSTEM"
signature_len	equ	$-signature

no_board_msg	db	"Cannot locate an ARLAN board.",'$'
self_test_msg	db	"ARLAN board self-tests bad.",'$'
bad_mem_msg	db	"The on-card memory tests as bad.",'$'
file_not_found	db	"File not found",'$'
read_trouble	db	"Trouble reading the file",'$'
timeout_msg_xx	db	"x"
timeout_msg_x	db	"x"
timeout_msg	db	"Timed out waiting for the board",'$'
configure_bad	db	"The configure attempt failed",'$'

arlan_cfg	db	"arlan.cfg",0
handle		dw	?

int_no_name	db	"Interrupt number ",'$'

	public	etopen
etopen:
	assume	ds:code, es:arlan_segment

;look for the arlan card in memory
	mov	dx,0c000h
etopen_1:
	mov	si,offset signature
	mov	es,dx
	mov	di,offset ar_signature
	mov	cx,signature_len
	repe	cmpsb
	je	etopen_2

	add	dx,200h
	cmp	dx,0de00h
	jb	etopen_1
	mov	dx,offset no_board_msg
	jmp	error
etopen_2:
	mov	base_addr,dx

	mov	ar_control,1		;reset the board.

	mov	ax,base_addr		;test the memory.
	mov	cx,2000h-3
	call	memory_test
	je	memory_ok
	mov	dx,offset bad_mem_msg
	jmp	error
memory_ok:

	xor	di,di			;zero all the memory.
	mov	cx,2000h-1
	xor	al,al
	rep	stosb

	mov	ar_reset,1		;set the reset flag.
	mov	ar_control,0		;remove the reset.

	mov	ax,36
	call	set_timeout
wait_for_reset:
	cmp	ar_reset,0		;did it finish resetting yet?
	je	wait_for_reset_1	;yes, exit.
	call	do_timeout
	jne	wait_for_reset
	mov	dx,offset timeout_msg
	jmp	error
wait_for_reset_1:

;set the reset flag again, so that we can detect if we somehow got reset.

	mov	ar_reset,1

	cmp	ar_diagnostics,0ffh	;Did it self-check okay?
	je	self_test_ok
	mov	dx,offset self_test_msg
	jmp	error
self_test_ok:

	mov	ar_command,COM_NOP	;do a NOP.
	call	doca
	jnc	wait_for_first_nop_1
	mov	dx,offset self_test_msg
	jmp	error
wait_for_first_nop_1:

;;; They say to do another with with COM_INT set...

	mov	al,ar_irq_level		;copy the interrupt number out of
	mov	int_no,al		;  the configuration file.

	mov	ax,3d00h		;open for reading.
	mov	dx,offset arlan_cfg
	int	21h
	jnc	file_found
	mov	dx,offset file_not_found
	jmp	error

file_found:
	mov	handle,ax

	mov	ax,4200h
	mov	bx,handle
	xor	cx,cx
	mov	dx,100h			;skip past the first 100h bytes.
	int	21h

	mov	ah,3fh			;read the system parameters.
	mov	bx,handle
	mov	cx,100h
	mov	dx,offset ar_sys_params
	push	ds
	mov	ds,base_addr
	int	21h
	pop	ds
	jnc	no_trouble
	mov	dx,offset read_trouble
	jmp	error

no_trouble:

	mov	ah,3eh			;close the file.
	mov	bx,handle
	int	21h

	cmp	int_no,0		;Does the board know its interrupt
	jne	set_int_no		;  number?  go if it does.
	mov	al,ar_irq_level		;No, so use the one
	mov	int_no,al		;  in the configuration file.
set_int_no:

; do the configure.

	mov	ar_command,COM_CONF
	call	doca
	jnc	wait_for_configure_1	;it did.
	mov	dx,offset timeout_msg_x
	jmp	error
wait_for_configure_1:

	cmp	ar_diagnostics,0ffh	;did the configure succeed?
	je	configure_ok
	mov	dx,offset configure_bad
	jmp	error
configure_ok:

;wait a short while for the AR450.  For the AR440, wait up to 15 seconds.

	mov	ax,36
	call	set_timeout
wait_for_address:
	mov	cx,EADDR_LEN		;see if our address is still zeroes.
	mov	si,offset ar_node_id
	xor	al,al
wait_for_address_2:
	or	al,es:[si]
	inc	si
	loop	wait_for_address_2
	or	al,al			;do we have an address yet?
	jne	wait_for_address_1	;yes.
	call	do_timeout
	jne	wait_for_address
	mov	dx,offset timeout_msg_xx
	jmp	error
wait_for_address_1:

;say that the max we'll send is an Ethernet GIANT packet, less the two
;  Ethernet addresses that we don't include in the datagram, plus the one
;  broadcast byte that we *do* include.

	mov	ax,GIANT - EADDR_LEN*2 + 1
	mov	ax,1023
	mov	ar_MDS,ax
	mov	ar_MFS,ax

; reduce the number of retries

	mov	ar_max_retry,16

;enable reception

	call	enable_receive

;mark transmission as done

	mov	ar_tx_status,1

;enable our interrupts.

	mov	al,ar_control_i
	or	al,CONTROL_IE or CONTROL_CLRI
	mov	ar_control_i,al
	mov	ar_control,al

; Now hook in our interrupt

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


	public	print_parameters
print_parameters:
;echo our command-line parameters
	mov	dx,offset int_no_name
	mov	di,offset int_no
	call	print_number

	ret

	include	memtest.asm

code	ends

	end
