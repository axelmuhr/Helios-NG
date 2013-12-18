version	equ	1

	include	defs.asm	;SEE ENCLOSED COPYRIGHT MESSAGE

; Packet driver for BICC Data Networks' ISOLAN 4110 ethernet
; controller, written by
;	Rainer Toebbicke
;	European Organisation of Nuclear Research (CERN)
;	Geneva, Switzerland
; based on the "generic" packet driver by Russell Nelson.



; BICC ISOLAN card constants

IS_SAP	struc			;Service Access Point channel
IS_S_Data	db	?	;data semaphore
IS_S_Taken	db	?	;taken semaphore
		dw	?

IS_S_Event	db	?

IS_S_SrcEaddr	db	6 dup(?)	;source Ethernet address
IS_S_SrcLsap	db	?
IS_S_DstEaddr	db	6 dup(?)
IS_S_DstLsap	db	?
		db	?
IS_S_Status	dw	?
		db	?
IS_S_SDUptr	dw	?
IS_SAP		ends

; Bits defined in IS_S_Event
IS_M_Init	equ	0ch
IS_M_DataRq	equ	09h
IS_M_DataInd	equ	0ah
IS_Board_Init	equ	0dh


IS		segment	at 0
		org	8000h
IS_Sign		db	?		;test for card presence
IS_Reset	db	?
IS_Diagok	db	?
IS_Error	db	?
IS_ErrCode	db	?
		org	800eh
IS_PgmStart	dw	?

IS_IER		db	?		;interrupt enable register
IS_I_Tx		equ	01h		;transmit interrupt
IS_I_Rx		equ	02h		;receive interrupt

IS_ISR		db	?		;interrupt status reg
		org	8014h
IS_BlueBook	db	?
IS_LLC1		db	?
IS_Precv	db	?		;promiscuous receive flag
IS_LanceRev	db	?
IS_MacReflSup	db	?		;MAC reflection suppression
		org	8020h
IS_Xmit		IS_SAP	<>		;Transmit channel
		org	8040h
IS_Rcv		IS_SAP	<>		;Receive channel
		org	8060h
IS_Eaddr	db	6 dup(?)	;This card's Ethernet address

IS		ends


IS_SDU		struc
IS_SDU_DataOff	db	?		;offset to data
IS_SDU_L	dw	?		;length of packet
IS_SDU		ends


ENET_HDR	equ	EADDR_LEN*2+2	;length of ethernet header



code	segment	word public
	assume	cs:code, ds:code

	public	int_no,	MemBase
int_no	db	2,0,0,0			;must be four bytes long for get_number.
MemBase	dw	0b800h,0

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	4+128		;from the packet spec
driver_name	db	'ISOLAN',0	;name of the driver.
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

	cld				;moves go forward
	mov	ax,MemBase
	mov	es,ax			;address the card
	assume	es:IS
	push	cx			;save length

; Wait for the transmit channel to become free
	mov	cx,0ffffh		;avoid infinite loop
tx_wait:
	test	IS_Xmit.IS_S_Data,0ffh
	jnz	tx_idle
	loop	tx_wait
	pop	cx			;remove from stack
	mov	dh,CANT_SEND		;transmit error
	stc
	ret

tx_idle:
	mov	IS_Xmit.IS_S_Data,0		;our channel now
	mov	IS_Xmit.IS_S_Event,IS_M_DataRq	;sending Data

; copy destination ethernet addr
	mov	cx,EADDR_LEN/2
	lea	di,IS_Xmit.IS_S_DstEaddr
	rep	movsw

; copy source ethernet addr
	mov	cx,EADDR_LEN/2
	lea	di,IS_Xmit.IS_S_SrcEaddr
	rep	movsw

; copy type field
	lodsw
	xchg	ah,al				;8086 order
	mov	IS_Xmit.IS_S_Status,ax

	pop	cx				;restore count
	sub	cx,ENET_HDR			;minus header

	mov	di,IS_Xmit.IS_S_SDUptr		;point to SDU
	mov	es:IS_SDU_L[di],cx		;set count
	xor	ah,ah
	mov	al,es:IS_SDU_DataOff[di]
	add	di,ax				;point to data

	call	movemem				; now copy the buffer

	mov	IS_Xmit.IS_S_Taken,1		;release channel
	clc					;no error occurred
	ret
	assume	es:nothing


	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	cmp	cx,EADDR_LEN
	jnb	get_addr_ok		;buffer ok
	stc
	ret

get_addr_ok:
	push	ds
	push	si
	mov	ax,MemBase		;point to interface
	mov	ds,ax
	assume	ds:IS

	mov	si,offset IS_Eaddr	;point to our E-net addr
	mov	cx,EADDR_LEN/2
	rep	movsw
	pop	si
	pop	ds
	mov	cl,EADDR_LEN
	clc
	ret



	public	set_address
set_address:
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
	assume	ds:nothing
	cmp	cx,EADDR_LEN
	jnb	set_addr_ok		;buffer ok
	mov	dh,BAD_ADDRESS
	stc
	ret

set_addr_ok:
	push	es
	push	di
	mov	ax,MemBase		;point to interface
	mov	es,ax
	assume	es:IS

	mov	di,offset IS_Eaddr	;point to our E-net addr
	mov	cx,EADDR_LEN/2
	rep	movsw

; reset the board's software, don't really know if this is needed
	mov	IS_Xmit.IS_S_Data,0		;our channel now
	mov	IS_Xmit.IS_S_Event,IS_Board_Init ;reboot
	mov	IS_Xmit.IS_S_Taken,1		;issue the command

; wait for request to complete
	mov	cx,0ffffh		;avoid infinite loop
set_wait_2:
	test	IS_Xmit.IS_S_Data,0ffh
	jnz	set_done
	loop	set_wait_2

set_done:
	mov	cx,EADDR_LEN		;return their address length.
	pop	di
	pop	es
	assume	es:nothing
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
	push	ds
	mov	ax,MemBase
	mov	ds,ax
	assume	ds:IS
	mov	IS_IER,0
	pop	ds
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
	mov	ax,MemBase
	mov	ds,ax
	assume	ds:IS

	mov	al,IS_ISR		;get interrupt status
	test	al,IS_I_RX		;receive interrupt?
	jnz	rcv_test_errors		;yes...
	jmp	rcv_done

rcv_test_errors:

; some thorough testing missing here
; anyway, don't know what to test


rcv_no_errors:
	mov	IS_Rcv.IS_S_Data,0	;block channel
	mov	di,IS_Rcv.IS_S_SDUptr	;point to SDU
	mov	cx,IS_SDU_L[di]		;get length
	add	cx,ENET_HDR		;plus ethernet header


; These following lines correctly point to the type field.
; However, addressability to the whole buffer on the 'receiver' call
; would be lost, a feature useful to some applications

;	mov 	ax,IS_Rcv.IS_S_Status ;get type field
;	xchg	ah,al		 ;in network byte order
;	push	ax		 ;have to place it somewhere
;	mov	di,sp
;	push	ss
;	pop	es			;es:di at type field

; The following 'hack' keeps addressability to the packet
; on the 'receiver' call but stays compatible with the
; packet driver skeleton.
; This feature is indicated by a bit in the type number (ugly)
; which the application can examine

	mov	al,IS_SDU_DataOff[di]	;offset to to data
	sub	al,2			;offset to type field
	cbw
	add	di,ax			;point to type field
	push	ds
	pop	es			;es:di->type field

	push	cs
	pop	ds			;don't confuse recv_find
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
	call	recv_find		;do we want this packet?
;	add	sp,2		   	;remove type field

	mov	ax,es
	or	ax,di
	jz	rcv_done_0		;pointer zero, give up

	push	es
	push	di			;save pointer

	mov	ax,MemBase
	mov	ds,ax
	assume	ds:IS

; set up ethernet header
	mov	cx,EADDR_LEN/2
	mov	si,offset IS_Rcv.IS_S_DstEaddr
	rep	movsw
	mov	cx,EADDR_LEN/2
	mov	si,offset IS_Rcv.IS_S_SrcEaddr
	rep	movsw
	mov	ax,IS_Rcv.IS_S_Status		;type field
	xchg	ah,al				;in network byte order
	stosw

	mov	si,IS_Rcv.IS_S_SDUptr		;point to SDU
	mov	cx,IS_SDU_L[si]			;get length
	push	cx				;save for later
	mov	al,IS_SDU_DataOff[si]
	xor	ah,ah
	add	si,ax				;point to data

	call	movemem

	pop	cx			;restore count
	pop	si			;restore pointer
	pop	ds			;restore pointer
	assume	ds:nothing

	add	cx,ENET_HDR		;adjust length
	call	recv_copy		;wake up client

rcv_done_0:
	mov	ax,MemBase		;point to interface
	mov	ds,ax
	assume	ds:IS

rcv_done:
	mov	IS_Rcv.IS_S_Taken,1	;release channel
	mov	IS_ISR,0		;clear interrupt
	push	cs
	pop	ds
	assume	ds:code
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
usage_msg db "usage: ISOLAN [-n] [-d] [-w] <packet_int_no> <int_level>	<mem_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet	driver for BICC	ISOLAN device, version ",'0'+majver,".",'0'+version,CR,LF
		db	"Portions Copyright 1989, R.Toebbicke, CERN, Switzerland",CR,LF
		db	'$'

errmsg1	db	"No BICC Isolan	board found at this address.",CR,LF,'$'
diag_errmsg	db	"Error - ISOLAN	Diagnostics failed.$"

diag_start_msg	db	"ISOLAN	Diagnostics running... $"
diag_end_msg	db	" done.$"

int_no_name	db	"Interrupt number $"
MemBaseName	db	"Shared	Memory address $"
MemPrt	dw	0c000h,0



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
	mov	di,offset MemPrt
	call	get_number
	mov	di,MemPrt
	sub	di,800h			;BICC standard notation
	mov	MemBase,di
	clc
	ret


bad_board:
	mov	dx,offset errmsg1
err_msg:
	assume	ds:nothing
	push	cs
	pop	ds
	mov	ah,9
	int	21h
	stc
	ret


	public	etopen
etopen:
	mov	ax,MemBase
	mov	ds,ax
	assume	ds:IS

; test if board exists
	mov	IS_Sign,42
	cmp	IS_Sign,42
	jne	bad_board

	mov	IS_Sign,0ffh-42
	cmp	IS_Sign,0ffh-42
	jne	bad_board

	cmp	IS_Reset,01h		;already running?
	je	diag_ok			;yes, let it run

	mov	IS_Reset,0
	mov	cx,0ffffh		;avoid infinite loop
diag_w_0:
	cmp	IS_Reset,0fah
	je	diag_1
	loop	diag_w_0

diag_1:
	mov	IS_Reset,0ffh
	mov	cx,0ffffh		;avoid infinite loop
diag_w_1:
	cmp	IS_Reset,0fah
	je	diag_2
	loop	diag_w_1

diag_2:
	mov	IS_Reset,0e5h
	mov	cx,0ffffh		;avoid infinite loop
diag_w_2:
	cmp	IS_Reset,0fah
	je	diag_3
	loop	diag_w_2

diag_3:
	mov	IS_Reset,09dh		;start diagnostics

	push	ds
	push	cs
	pop	ds
	mov	dx,offset diag_start_msg
	mov	ah,9
	int	21h
	pop	ds

	mov	bx,050h			;double loop, takes some time
diag_w_3_0:
	mov	cx,0ffffh		;avoid infinite loop
diag_w_3:
	cmp	IS_Diagok,01h
	je	diag_4
	loop	diag_w_3
	dec	bx
	jnz	diag_w_3_0
	mov	dx,offset diag_errmsg

diag_4:
	push	ds
	push	cs
	pop	ds
	mov	dx,offset diag_end_msg
	mov	ah,9
	int	21h
	pop	ds

	mov	ax,0800h
	mov	IS_PgmStart,ax
	mov	IS_Reset,01h		;run the program


; enable the board's interrupts
diag_ok:
	push	ds
	push	cs
	pop	ds			;set cs=ds
	call	set_recv_isr
	pop	ds

	mov	IS_BlueBook,1		;enable Blue Book MAC
	mov	IS_LLC1,1		;disable LLC1 service
	mov	IS_MacReflSup,1		;suppress MAC reflection
	mov	IS_IER,IS_I_RX		;enable for receive
	mov	IS_ISR,0		;clear previous

; reset the board's software, just in case...
	mov	IS_Xmit.IS_S_Data,0		;our channel now
	mov	IS_Xmit.IS_S_Event,IS_Board_Init ;reboot
	mov	IS_Xmit.IS_S_Taken,1		;issue the command

; again wait for request to complete
	mov	cx,0ffffh		;avoid infinite loop
in_wait_2:
	test	IS_Xmit.IS_S_Data,0ffh
	jnz	in_done
	loop	in_wait_2

in_done:

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
	push	cs
	pop	ds
	clc
	ret

	public	print_parameters
print_parameters:
;echo our command-line parameters
	mov	di,offset int_no
	mov	dx,offset int_no_name
	call	print_number
	mov	di,offset MemPrt
	mov	dx,offset MemBaseName
	call	print_number
	ret


code	ends

	end
