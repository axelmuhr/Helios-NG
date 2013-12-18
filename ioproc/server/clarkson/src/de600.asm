PAGE  ,132
   .286c
version equ     0

	include defs.asm        ;SEE ENCLOSED COPYRIGHT MESSAGE

;/* PC/FTP Packet Driver source, conforming to version 1.09 of the spec
;*  Portions (C) Copyright 1990 D-Link, Inc.
;*
;*  Permission is granted to any individual or institution to use, copy,
;*  modify, or redistribute this software and its documentation provided
;*  this notice and the copyright notices are retained.  This software may
;*  not be distributed for profit, either in original form or in derivative
;*  works.  D-Link, inc. makes no representations about the suitability
;*  of this software for any purpose.  D-LINK GIVES NO WARRANTY,
;*  EITHER EXPRESS OR IMPLIED, FOR THE PROGRAM AND/OR DOCUMENTATION
;*  PROVIDED, INCLUDING, WITHOUT LIMITATION, WARRANTY OF MERCHANTABILITY
;*  AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE.
;*/

BIT0            EQU     01H
BIT1            EQU     02H
BIT2            EQU     04H
BIT3            EQU     08H
BIT4            EQU     10H
BIT5            EQU     20H
BIT6            EQU     40H
BIT7            EQU     80H

code    segment byte public
	assume  cs:code, ds:code

; DE-600's I/O port Table
DAT             equ     0
STAT            equ     1
CMD             equ     2

; DE-600's DATA port Command
WRITE           equ     0004h   ;write memory
READ            equ     0104h   ;read  memory
STATUS          equ     0204h   ;read  status register
COMMAND         equ     0304h   ;write command register
NUL_CMD         equ     0ch     ;null command
RX_LEN          equ     0504h   ;read  Rx packet length
TX_ADR          equ     0604h   ;write Tx address
RW_ADR          equ     0704h   ;write memory address

;< COMMAND   bits 7-0 >
RXEN            equ     08h    ; bit 3
TXEN            equ     04h    ; bit 2
LOOPBACK        equ     0Ch    ; RXEN=1, TXEN=1
RX_NONE         equ     00h    ; M1=0, M0=0 (bit 1,0)
RX_ALL          equ     01h    ; M1=0, M0=1
RX_BP           equ     02h    ; M1=1, M0=0
RX_MBP          equ     03h    ; M1=1, M0=1
RESET           equ     80h    ; set bit 7 high
STOP_RESET      equ     00h    ; set bit 7 low
;  bit 6   -- IRQ inverse
;  bit 5,4 -- Rx Page Number  ( RA12=1, RA11=0 or 1 )

;< TX_ADR   bit 7, bit 4 >
;  bit 7   -- Tx Page Number  ( TA11=0 or 1 )
;  bit 4   -- Tx Page Number  ( TA12=0 )
PAGE0           equ     00h
PAGE1           equ     08h
PAGE2           equ     10h
PAGE3           equ     18h

;< RW_ADR   bit 7, bit 5,4 >
;  bit 7   -- RW Page Number  ( H11 =? )
;  bit 4   -- RW Page Number  ( H12 =? )
;  bit 5   -- Address Maping  ( HA13=0 => Memory, HA13=1 => Node Number )
HA13            equ     020h

; DE-600's CMD port Command
SLT_NIC         equ     004h  ;select Network Interface Card
SLT_PRN         equ     01Ch  ;select Printer
NML_PRN         equ     0ECh  ;normal Printer situation
IRQEN           equ     010h  ;enable IRQ line

; DE-600's STAT port bits 7-4
RXBUSY          equ     80h
GOOD            equ     40h
RESET_FLAG      equ     20h
T16             equ     10h
TXBUSY          equ     08h

BFRSIZ          equ     2048    ;number of bytes in a buffer
PRNTABADD       equ     408h    ;DOS printer table address
RX_MIN_LEN      equ     18      ;= EADDR_LEN + EADDR_LEN + TYPE_LEN + CRC

write_sub_delay	macro	reg
	mov     al,reg			;output the low nibble.
	shl     al,cl			;cl must be four.
	or      al,ch
	xor     al,08h			;raise the write line.
	out     dx,al
	call    delay

	xor     al,08h			;lower the write line
	out     dx,al
	call    delay

	mov     al,reg			;output the high nibble.
	and     al,not 0fh		;get us some zero bits.
	or      al,ch
	out     dx,al			;(write line is low).
	call    delay

	xor     al,08h			;raise the write line.
	out     dx,al
	endm

write_sub_fast	macro	reg
	mov     al,reg			;output the low nibble.
	shl     al,cl			;cl must be four.
	or      al,ch
	out     dx,al

	mov     al,reg			;output the high nibble.
	and     al,not 0fh		;get us some zero bits.
	or      al,ch
	xor     al,08h			;raise the write line.
	out     dx,al			;(write line is low).
	endm

write_sub_slow	macro	reg
	mov     al,reg			;output the low nibble.
	shl     al,cl			;cl must be four.
	or      al,ch
	out     dx,al
	call	delay

	mov     al,reg			;output the high nibble.
	and     al,not 0fh		;get us some zero bits.
	or      al,ch
	xor     al,08h			;raise the write line.
	out     dx,al			;(write line is low).
	endm

read_sub_fast	macro	reg
	setport DAT
	mov     al,ch
	out     dx,al
	pause

	setport STAT
	in      al,dx
	mov     reg,al
	setport DAT
	mov     al,ch
	xor     al,08h
	out     dx,al
	pause

	setport STAT
	in      al,dx
	shr     reg,cl
	and     al,0f0h
	or      reg,al
	endm


read_sub_slow	macro	reg
	setport DAT
	mov     al,ch
	out     dx,al
	call    delay

	setport STAT
	in      al,dx
	mov     reg,al
	setport DAT
	mov     al,ch
	xor     al,08h
	out     dx,al
	call    delay

	setport STAT
	in      al,dx
	shr     reg,cl
	and     al,0f0h
	or      reg,al
	endm


read_sub_delay	macro	reg, first
	setport DAT
	mov     al,ch
  if first
	xor     al,08h
  endif
	out     dx,al
	call    delay

  if first
	xor     al,08h
	out     dx,al
	call    delay
  endif

	setport STAT
	in      al,dx
	mov     reg,al
	setport DAT
	mov     al,ch
	xor     al,08h
	out     dx,al
	call    delay

	setport STAT
	in      al,dx
	shr     reg,cl
	and     al,0f0h
	or      reg,al
	endm


pause	macro
	jmp	$+2
	endm

	public  int_no
int_no          db      7,0,0,0         ; IRQ interrupt number
io_addr         dw      03bch,0         ; I/O address for card (jumpers)

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class    db      BLUEBOOK, IEEE8023, 0 ;from the packet spec
driver_type     db      31              ;from the packet spec
driver_name     db      'DE600',0      ;name of the driver
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
		dw	0
		dw	rcv_mode_3
		dw	0
		dw	rcv_mode_5
		dw	rcv_mode_6

CurTxPage       db      08h             ;the BL value when OUT DATA,TX_ADR
CurRxPage       db      20h             ;the BL value when OUT DATA,COMMAND
TxStartAdd      dw      ?
RxStartAdd      dw      ?
InitRxTxReg     dw      ?
RxPktLen        dw      ?
TxPktLen        dw      ?
Mode_RxPg       db      ?
Mode            db      RX_BP
IRQinverse      db      0               ; = 40h for XT printer adapter
NICstatus       db      0
In_ISR          db      0
In_Tx           db      0
printer         dw      408h
PS2             db      0

our_type        dw      ?
our_address     db      EADDR_LEN + EADDR_LEN dup(0) ;temporarily hold our address


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


	public  send_pkt
send_pkt:
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
	assume  ds:nothing
;select DE-600
	mov     al,SLT_NIC
;*** CMD sub ***
	loadport
	setport CMD
	out     dx,al
;*** End CMD sub ***
	mov     In_Tx,1
	call    delay

	cmp     cx,RUNT         ; minimum length for Ether
	jae     LengthOK
	mov     cx,RUNT         ; make sure size at least RUNT
LengthOK:
	inc     cx
	and     cx,not 1

	mov     di,cx
;change Tx page to another free buffer
	xor     CurTxPage,08h

	mov     bx,offset send_pkt_pointer
	jmp     cs:[bx]

send_pkt_pointer        dw      offset send_pkt0

send_pkt0:
;set Tx Pointer for moving packet
	mov     ax,BFRSIZ
	sub     ax,cx           ;AX = the pointer to TX
	or      ah,CurTxPage
	mov     TxStartAdd,ax   ;save Current Tx Packet Start Address
	mov     bx,ax           ;write memory address
	mov     cx,RW_ADR
	setport DAT
	write_sub_fast	bl
	write_sub_fast	bh
	cld
	mov     cx,WRITE        ;write packet into memory
	mov     ah,ch
	xor     ah,08h
write_mem:
	lodsb
	mov     bl,al
	shl     al,cl
	or      al,ch
	out     dx,al

	mov     al,bl
	and     al,0f0h
	or      al,ah
	dec     di
	out     dx,al
	jnz     write_mem

	mov     cx,4000h
	setport STAT
wait_Tx_idle:
	in      al,dx
	test    al,TXBUSY       ; Is the previous Tx successful ?
	jz      command_to_Tx   ; Yes, TXBUSY is low. Then could Tx next packet.
	loop    wait_Tx_idle

command_to_Tx:
;set Tx Pointer at beginning of packet
	mov     bx,TxStartAdd
	mov     cx,TX_ADR
	loadport
	setport DAT
	write_sub_fast	bl
	write_sub_fast	bh

;Enable interrupt and start Tx
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	write_sub_fast	bl
	or      bl,TXEN
	write_sub_fast	bl

Exit_Send_Packet:
	mov     In_Tx,0

	cmp     In_ISR,0
	jne     using_NIC_now
	mov     al,SLT_PRN
	loadport
	setport CMD
	out     dx,al

	cmp     PS2,0
	jnz     using_NIC_now
	setport STAT
	in      al,dx
	and     al,40h
	xor     al,IRQinverse
	jz      using_NIC_now
	call    trigger_int
using_NIC_now:
	clc
	ret


send_pkt1:
;set Tx Pointer for moving packet
	mov     ax,BFRSIZ
	sub     ax,cx           ;AX = the pointer to TX
	or      ah,CurTxPage
	mov     TxStartAdd,ax   ;save Current Tx Packet Start Address
	mov     bx,ax           ;write memory address
	mov     cx,RW_ADR
	loadport
	setport DAT
	write_sub_slow	bl
	call	delay
	write_sub_slow	bh

	cld
	mov     cx,WRITE        ;write packet into memory
	mov     ah,ch
	xor     ah,08h
write_mem1:
	lodsb
	mov     bl,al
	shl     al,cl
	or      al,ch
	out     dx,al
	call    delay

	mov     al,bl
	and     al,0f0h
	or      al,ah
	dec     di
	out     dx,al
	call    delay
	jnz     write_mem1

	mov     cx,4000h
	setport STAT
wait_Tx_idle1:
	in      al,dx
	test    al,TXBUSY       ; Is the previous Tx successful ?
	jz      command_to_Tx1  ; Yes, TXBUSY is low. Then could Tx next packet.
	loop    wait_Tx_idle1

command_to_Tx1:
;set Tx Pointer at beginning of packet
	mov     bx,TxStartAdd
	mov     cx,TX_ADR
	loadport
	setport DAT
	write_sub_slow	bl
	call	delay
	write_sub_slow	bh
	call    delay

;Enable interrupt and start Tx
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	write_sub_slow	bl
	call    delay
	or      bl,TXEN
	write_sub_slow	bl
	jmp     Exit_Send_Packet


send_pkt2:
;set Tx Pointer for moving packet
	mov     ax,BFRSIZ
	sub     ax,cx           ;AX = the pointer to TX
	or      ah,CurTxPage
	mov     TxStartAdd,ax   ;save Current Tx Packet Start Address
	mov     bx,ax           ;write memory address
	mov     cx,RW_ADR
	loadport
	setport DAT
	write_sub_delay	bl
	call	delay
	write_sub_delay	bh
	cld
	mov     cx,WRITE        ;write packet into memory
write_mem2:
	lodsb
	mov     bl,al			;except for this line,
	shl     al,cl			;  it's write_sub_delay
	or      al,ch
	xor     al,08h
	out     dx,al
	call    delay

	xor     al,08h
	out     dx,al
	call    delay

	mov     al,bl
	and     al,0f0h
	or      al,ch
	out     dx,al
	call    delay

	xor     al,08h
	out     dx,al
	call    delay
	dec     di
	jnz     write_mem2

	mov     cx,4000h
	setport STAT
wait_Tx_idle2:
	in      al,dx
	test    al,TXBUSY       ; Is the previous Tx successful ?
	jz      command_to_Tx2  ; Yes, TXBUSY is low. Then could Tx next packet.
	loop    wait_Tx_idle2

command_to_Tx2:
;set Tx Pointer at beginning of packet
	mov     bx,TxStartAdd
	mov     cx,TX_ADR
	loadport
	setport DAT
	write_sub_delay	bl
	call	delay
	write_sub_delay	bh
	call    delay

;Enable interrupt and start Tx
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	write_sub_delay	bl
	call    delay
	or      bl,TXEN
	write_sub_delay	bl
	jmp     Exit_Send_Packet


trigger_int:
	mov     al,SLT_NIC
	call    CMD_sub

	mov     bl,Mode_RxPg
	xor     bl,BIT6
	mov     cx,COMMAND
	call    Write_sub

	mov     al,SLT_PRN
	call    CMD_sub

	call    delay
	call    delay

	mov     al,SLT_NIC
	call    CMD_sub

	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	call    Write_sub
	ret


	public  get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
	assume  ds:code
	cmp     cx,EADDR_LEN            ;make sure that we have enough room.
	jb      cant_get_address

;select DE-600
	cld
	mov     si,offset our_address
	rep     movsb
	mov     cx,EADDR_LEN
	clc
	ret
cant_get_address:
	stc
	ret


rcv_mode_1:
	mov     cl,RX_NONE
	jmp     short set_RXCR
rcv_mode_3:
	mov     cl,RX_BP
	jmp     short set_RxCR
rcv_mode_5:
	mov     cl,RX_MBP
	jmp     short set_RxCR
rcv_mode_6:
	mov     cl,RX_ALL
set_RxCR:
	mov     Mode,cl
	mov     bl,cl
	or      bl,CurRxPage            ; Add original Rx Page
	mov     Mode_RxPg,bl            ; Save Rx Mode & Rx Page
;select DE-600
	loadport
	setport CMD
	in      al,dx
	pause
	test    al,BIT4
	jz      in_IC_mode
;not active, we have to put a wrapper around it.
	mov     al,int_no
	call    maskint

	mov     al,SLT_NIC		;turn on the NIC.
	call    CMD_sub
	call	in_IC_mode		;now we're in the right mode.
	mov     al,SLT_PRN		;turn on the PRN.
	call    CMD_sub

	mov     al,int_no
	call    unmaskint
	ret

in_IC_mode:
	mov     cx,COMMAND              ; Set new Rx Mode
	call    Write_sub
	ret


	extrn   maskint : near
	extrn   unmaskint : near

	public  set_address
set_address:
;Set Ethernet address on controller
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
;
	assume  ds:nothing
	cmp     cx,EADDR_LEN            ;make sure that we have enough room.
	je      can_set_address
	mov     dh,BAD_ADDRESS
	stc
	jmp     set_address_none

;select DE-600
can_set_address:
	loadport
	setport CMD
	in      al,dx
	push    ax
	test    al,BIT4
	jz      IC_mode

	mov     al,int_no
	call    maskint
	mov     al,SLT_NIC
	call    CMD_sub
IC_mode:
	mov     cx,RW_ADR
	xor     bl,bl
	call    Write_sub
	or      bl,HA13
	call    Write_sub

	cld
	mov     bp,cs
	mov     es,bp
	mov     bp,si

	mov     cx,EADDR_LEN
	mov     di,offset our_address
	rep     movsb

	mov     si,bp
	mov     di,EADDR_LEN
	mov     cx,WRITE
set_our_address:
	lodsb
	mov     bl,al
	call    Write_sub
	dec     di
	jnz     set_our_address

	pop     ax
	test    al,BIT4
	jz      IC_mode1

	mov     al,SLT_PRN
	call    CMD_sub
	mov     al,int_no
	call    unmaskint
IC_mode1:
	clc
set_address_none:
	push    cs
	pop     ds
	assume  ds:code
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


	public  reset_interface
reset_interface:
	mov     al,int_no
	call    maskint
;select DE-600

	mov     al,SLT_NIC
	call    CMD_sub

; Pulse IE_RESET
	mov     bl,RESET
	mov     cx,COMMAND
	call    Write_sub
	mov     bl,STOP_RESET
	call    Write_sub

; Initialize Rx buffer pointer, and start receive
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	call    Write_sub
	or      bl,RXEN
	call    Write_sub

; Enable Printer Adapter IRQ line
	mov     al,SLT_PRN
	call    CMD_sub

	mov     al,int_no
	call    unmaskint
	ret


	assume  ds:nothing
Write_sub:
	loadport
	setport DAT
	write_sub_delay	bl
	ret

Read_sub:
	loadport
	setport DAT
	mov     al,ch
	xor     al,08h
	out     dx,al
	call    delay

	xor     al,08h
	out     dx,al
	call    delay

	setport STAT
	in      al,dx
	mov     bl,al
	test    ch,BIT1
	jz      not_READ_STATUS
	dec     dx
	mov     al,NUL_CMD
	xor     al,08h
	out     dx,al
	call    delay

	xor     al,08h
	out     dx,al
	jmp     short End_read
not_READ_STATUS:
	setport DAT
	mov     al,ch
	xor     al,08h
	out     dx,al
	call    delay

	setport STAT
	in      al,dx
	shr     bl,cl
	and     al,0f0h
	or      bl,al
End_read:
	ret

CMD_sub:
	loadport
	setport CMD
	out     dx,al
	ret

delay:
	nop     ; pointer 0
	nop     ;         1
	nop     ;         2
	nop     ;         3
	nop     ;         4
	nop     ;         5
	nop     ;         6
	nop     ;         7
	nop     ;         8
	nop     ;         9
	ret

;called when we want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type, dl = packet class.
	extrn   recv_find: near

;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
	extrn   recv_copy: near

	extrn   count_in_err: near
	extrn   count_out_err: near

recv_pointer    dw      offset recv0

	public  recv
recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
	assume  ds:code
;select DE-600
	mov     al,SLT_NIC
;*** CMD sub ***
	loadport
	setport CMD
	out     dx,al
;*** End CMD sub ***
;set watch dog
	mov     In_ISR,1
	call    delay

;Check the interrupt source, Rx or Tx ?
	mov     cx,STATUS               ; Read NIC Status Register
;*** Read sub ***
	setport DAT
	mov     al,ch
	out     dx,al
	pause
	setport STAT
	in      al,dx
;*** End Read sub ***

	jmp	recv_pointer

recv0:
	mov     NICstatus,al            ; save NIC status
	setport DAT
	test    al,GOOD                 ; Is Rx generating interrupt ?
	mov     al,NUL_CMD
	out     dx,al
	jnz     Rx_Good_Pkt             ; Yes, take care of this situation.
	mov     al,NICstatus
	test    al,RXBUSY
	jz      Enable_Rx
	jmp     CheckTx

Enable_Rx:
;change Rx page & enable NIC to Rx
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	setport DAT
	write_sub_fast	bl
	or      bl,RXEN
	write_sub_fast	bl
	jmp     CheckTx

Rx_Good_Pkt:
;Put it on the receive queue
	mov     cx,RX_LEN               ; read Rx Packet Length
	loadport
	read_sub_fast	bl
	read_sub_fast	bh

	sub     bx,4                    ;subtrate 4 CRC Byte Count
	mov     RxPktLen,bx             ;save Rx Packet Length

;change Rx page & enable NIC to Rx
	xor     Mode_RxPg,10h
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	setport DAT
	write_sub_fast	bl
	or      bl,RXEN
	write_sub_fast	bl
	xor     bx,bx
	mov     bh,CurRxPage            ;BL = Current Rx Page
	xor     CurRxPage,10h           ;change to next page for Rx
	shr     bx,1                    ;shift BX to real memory address
	mov     RxStartAdd,bx           ;save just Rx Packet Start Address

	add     bx,EADDR_LEN+EADDR_LEN  ;seek to the TYPE word
	mov     cx,RW_ADR
	write_sub_fast	bl
	write_sub_fast	bh
	pause
	mov     cx,READ                 ;read the TYPE word
	read_sub_fast	bl
	read_sub_fast	bh

	mov     our_type,bx             ;save the TYPE word

	mov     ax,ds
	mov     es,ax
	mov     di,offset our_type
	mov     cx,RxPktLen

	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	inc	di			;set di to 802.2 header
	inc	di
	mov	dl, IEEE8023
BlueBookPacket:
	call    recv_find               ;request a Rx buffer to store Rx data

	mov     ax,es                   ;is this pointer null?
	or      ax,di
	jnz     find_buffer
	jmp     short CheckTx                 ;yes - just free the frame.
find_buffer:
	push    es
	push    di                   ;remember where the buffer pointer is.
	assume  ds:nothing
	mov     bx,RxStartAdd
	mov     cx,RW_ADR
	loadport
	setport DAT
	write_sub_fast	bl
	write_sub_fast	bh

	cld
	mov     bp,RxPktLen             ;CX = the byte count of Rx Packet
	setport STAT
	mov     cx,READ
	mov     ah,ch
	xor     ah,08h
read_mem:
	dec     dx
	mov     al,ch
	out     dx,al
	pause

	inc     dx
	in      al,dx
	mov     bl,al
	dec     dx
	mov     al,ah
	out     dx,al
	pause

	inc     dx
	in      al,dx
	shr     bl,cl
	and     al,0f0h
	or      al,bl
	stosb
	dec     bp
	jnz     read_mem

RxCopy_CheckTx:
	pop     si
	pop     ds
	mov     cx,RxPktLen

	call    recv_copy               ;tell them that we copied it.

	mov     ax,cs                   ;restore our ds.
	mov     ds,ax
	assume  ds:code

CheckTx:
	test    NICstatus,T16           ; Is pending a Tx Packet ?
	jz      return                  ; No, then return.
	                                ; Yes, send this packet.
;set Tx Pointer at beginning of packet
	mov     bx,TxStartAdd
	mov     cx,TX_ADR
	call    Write_sub
	xchg    bh,bl
	call    Write_sub

;Enable interrupt and start Tx
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	call    Write_sub
	or      bl,TXEN
	call    Write_sub
return:
	mov     al,SLT_PRN
;*** CMD sub ***
	loadport
	setport CMD
	out     dx,al
;*** End CMD sub ***

	cmp     PS2,0
	jnz     Rx_another_pkt
	setport STAT
	in      al,dx
	and     al,40h
	xor     al,IRQinverse
	jz      Rx_another_pkt
	call    trigger_int
Rx_another_pkt:
	mov     In_ISR,0
	ret


recv1:
	assume  ds:code
	mov     NICstatus,al            ; save NIC status
	loadport
	setport DAT
	test    al,GOOD                 ; Is Rx generating interrupt ?
	mov     al,NUL_CMD
	out     dx,al
	jnz     Rx_Good_Pkt1             ; Yes, take care of this situation.
	mov     al,NICstatus
	test    al,RXBUSY
	jz      Enable_Rx1
	jmp     CheckTx                 ; No, go to check Tx.

Enable_Rx1:
;change Rx page & enable NIC to Rx
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	setport DAT
	write_sub_slow	bl
	call    delay
	or      bl,RXEN
	write_sub_slow	bl
	jmp     CheckTx

Rx_Good_Pkt1:
;Put it on the receive queue
	mov     cx,RX_LEN               ; read Rx Packet Length
	loadport
	read_sub_slow	bl
	read_sub_slow	bh

	sub     bx,4                    ;subtrate 4 CRC Byte Count
	mov     RxPktLen,bx             ;save Rx Packet Length

;change Rx page & enable NIC to Rx
	xor     Mode_RxPg,10h
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	setport DAT
	write_sub_slow	bl
	call    delay
	or      bl,RXEN
	write_sub_slow	bl
	call    delay
	xor     bx,bx
	mov     bh,CurRxPage            ;BL = Current Rx Page
	xor     CurRxPage,10h           ;change to next page for Rx
	shr     bx,1                    ;shift BX to real memory address
	mov     RxStartAdd,bx           ;save just Rx Packet Start Address

	add     bx,EADDR_LEN+EADDR_LEN  ;seek to the TYPE word
	mov     cx,RW_ADR
	write_sub_slow	bl
	call    delay
	write_sub_slow	bh
	call    delay
	mov     cx,READ                 ;read the TYPE word

	read_sub_slow	bl
	read_sub_slow	bh

	mov     our_type,bx             ;save the TYPE word

	mov     ax,ds
	mov     es,ax
	mov     di,offset our_type
	mov     cx,RxPktLen

	call    recv_find               ;request a Rx buffer to store Rx data

	mov     ax,es                   ;is this pointer null?
	or      ax,di
	jnz     find_buffer1
	jmp     CheckTx                 ;yes - just free the frame.
find_buffer1:
	push    es
	push    di                   ;remember where the buffer pointer is.
	assume  ds:nothing
	mov     bx,RxStartAdd
	mov     cx,RW_ADR
	loadport
	setport DAT
	write_sub_slow	bl
	call    delay
	write_sub_slow	bh
	call    delay

	cld
	mov     bp,RxPktLen             ;CX = the byte count of Rx Packet
	setport STAT
	mov     cx,READ
	mov     ah,ch
	xor     ah,08h
read_mem1:
	dec     dx
	mov     al,ch
	out     dx,al
	call    delay

	inc     dx
	in      al,dx
	mov     bl,al
	dec     dx
	mov     al,ah
	out     dx,al
	call    delay

	inc     dx
	in      al,dx
	shr     bl,cl
	and     al,0f0h
	or      al,bl
	stosb
	dec     bp
	jnz     read_mem1
	jmp     RxCopy_CheckTx


recv2:
	assume  ds:code
	mov     NICstatus,al            ; save NIC status
	loadport
	setport DAT
	test    al,GOOD                 ; Is Rx generating interrupt ?
	mov     al,NUL_CMD
	out     dx,al
	jnz     Rx_Good_Pkt2             ; Yes, take care of this situation.
	mov     al,NICstatus
	test    al,RXBUSY
	jz      Enable_Rx2
	jmp     CheckTx                 ; No, go to check Tx.

Enable_Rx2:
;change Rx page & enable NIC to Rx
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	setport DAT
	write_sub_delay	bl
	call    delay
	or      bl,RXEN
  if 0
	write_sub_delay	bl
  else
	mov     al,bl
	shl     al,cl
	or      al,ch
	xor     al,08h
	out     dx,al
	call    delay

	xor     al,08h
	out     dx,al
	call    delay

	mov     al,bl
	and     al,0f0h
	or      al,ch
	xor     al,08h
	out     dx,al
  endif
	jmp     CheckTx

Rx_Good_Pkt2:
;Put it on the receive queue
	mov     cx,RX_LEN               ; read Rx Packet Length
	loadport

	read_sub_delay	bl, 1
	read_sub_delay	bh, 0

	sub     bx,4                    ;subtrate 4 CRC Byte Count
	mov     RxPktLen,bx             ;save Rx Packet Length

;change Rx page & enable NIC to Rx
	xor     Mode_RxPg,10h
	mov     bl,Mode_RxPg
	mov     cx,COMMAND
	setport DAT
	write_sub_delay	bl
	call    delay
	or      bl,RXEN
	write_sub_delay	bl
	call    delay

	xor     bx,bx
	mov     bh,CurRxPage            ;BL = Current Rx Page
	xor     CurRxPage,10h           ;change to next page for Rx
	shr     bx,1                    ;shift BX to real memory address
	mov     RxStartAdd,bx           ;save just Rx Packet Start Address

	add     bx,EADDR_LEN+EADDR_LEN  ;seek to the TYPE word
	mov     cx,RW_ADR
	write_sub_delay	bl
	call    delay
	write_sub_delay	bh
	call    delay

	mov     cx,READ                 ;read the TYPE word

	read_sub_delay	bl, 1
	read_sub_delay	bh, 0
	mov     our_type,bx             ;save the TYPE word

	mov     ax,ds
	mov     es,ax
	mov     di,offset our_type
	mov     cx,RxPktLen

	call    recv_find               ;request a Rx buffer to store Rx data

	mov     ax,es                   ;is this pointer null?
	or      ax,di
	jnz     find_buffer2
	jmp     CheckTx                 ;yes - just free the frame.
find_buffer2:
	push    es
	push    di                   ;remember where the buffer pointer is.
	assume  ds:nothing
	mov     bx,RxStartAdd
	mov     cx,RW_ADR
	loadport
	setport DAT
	write_sub_delay	bl
	call    delay
	write_sub_delay	bh
	call    delay

	cld
	mov     bp,RxPktLen             ;CX = the byte count of Rx Packet
	setport STAT
	mov     cx,READ
	mov     ah,ch
	xor     ah,08h
read_mem2:
	dec     dx
	mov     al,ch
	out     dx,al
	call    delay

	inc     dx
	in      al,dx
	mov     bl,al
	dec     dx
	mov     al,ah
	out     dx,al
	call    delay

	inc     dx
	in      al,dx
	shr     bl,cl
	and     al,0f0h
	or      al,bl
	stosb
	dec     bp
	jnz     read_mem2
	jmp     RxCopy_CheckTx


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret


	public  end_resident
end_resident    label   byte
;any code after this will not be kept after initialization.

	public  usage_msg
usage_msg       db      "usage: DE600PD <packet_int_no>",CR,LF,'$'

	public  copyright_msg
copyright_msg   db      "Packet driver for the D-Link DE-600, "
	        db      "version ",'0'+majver,".",'0'+version,CR,LF,'$'
	        db      "Portions Copyright 1988, Robert C. Clements, K1BC"
	        db      CR,LF,'$'

CableErr        db      "Bad cable connection.",07,CR,LF,'$'
mem_error_msg   db      "Adapter memory buffer failure, or bad printer "
	        db      "port connection.",07,CR,LF,'$'
irq_error_msg   db      "IRQ unavailable, please check other hardware in "
	        db      "computer.",07,CR,LF,'$'
no_NIC_err      db      "Adapter not found, or AC adapter power is off.",07,CR,LF,'$'

	extrn   set_recv_isr: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

	assume  ds:code
	public  parse_args
parse_args:
	ret


	public  print_parameters
print_parameters:
	ret

cable_err:
	mov     dx,offset CableErr
	jmp     short show_msg
IRQ_error:
	mov     dx,offset irq_error_msg
	jmp     short show_msg
no_our_NIC:
	mov     dx,offset no_NIC_err
	jmp     short show_msg
mem_error:
	mov     dx,offset mem_error_msg
show_msg:
	mov     ah,9
	int     21h
	stc
	ret

	public  etopen
etopen:
;  Initialize the Ethernet board.
	call    check_PS2

	xor     ax,ax
	mov     es,ax
	mov     si,PRNTABADD-2          ; point to printer table at low memory
next_prn_port:
	add     si,2
	mov     bx,es:[si]              ; get LPTx's I/O Base
	or      bx,bx                   ; Does LPTx really exist ?
	jz      Chk_out_of_range
	mov     io_addr,bx              ; save the I/O Base number
	mov     printer,si              ; memorize it's LPTx now
	call    Check_DE600             ; Yes, BX = I/O Base, then check.
	jnc     IO_good                 ; If carry flag is clear, so go to
	mov     al,NML_PRN
	call    CMD_sub
Chk_out_of_range:
	cmp     si,PRNTABADD+6          ; We still miss our card from LPT1 to
	jb      next_prn_port
	jmp     short no_our_NIC        ; We still miss our card from LPT1 to
	                                ; LPT4. We give it up.
IO_good:
; Copy our Ethernet address from PROM into DE600.
	push    ds
	pop     es
	mov     di,offset our_address
	mov     cx,RW_ADR
	xor     bl,bl
	call    Write_sub
	or      bl,HA13
	call    Write_sub

	cld
	mov     bp,EADDR_LEN
	mov     cx,READ
get_our_address:
	call    Read_sub
	mov     al,bl
	stosb
	dec     bp
	jnz     get_our_address

	mov     si,offset our_address	;make sure it's got the right magic
	cmp     word ptr es:[si],0de00h	;  number.
	jne     no_our_NIC
	cmp     byte ptr es:[si+2],15h
	jne     no_our_NIC

	mov     word ptr es:[si],8000h	;now modify it to the address assigned
	mov     byte ptr es:[si+2],0c8h	;  by Xerox.
	and     byte ptr es:[si+3],0fh
	or      byte ptr es:[si+3],070h

	mov     cx,EADDR_LEN
	call    set_address

; Check DE600's IRQ enviroment
	call    Check_IRQ
	cmp     bx,1
	je      cable_OK
	jmp     cable_err

cable_OK:
	mov     bx,offset delay
	mov     byte ptr [bx],0c3h     ;  ret

; test 8 KBytes memory
;********** write mode 1 *************
	mov     cx,700h
	mov     al,PAGE3
	call    Write_LoopBack_Data
	xor     bx,bx
	mov     si,bx
write_next_page:
	call    Tx_Data
	push    bx
	mov     cx,RW_ADR
	loadport
	setport DAT
	write_sub_fast	bl
	write_sub_fast	bh

	mov     bx,si
	mov     bp,800h
	mov     cx,WRITE
	mov     ah,ch
	xor     ah,08h
wr_this_page:
	mov     al,bl
	shl     al,cl
	or      al,ch
	out     dx,al
	mov     al,bl
	and     al,0f0h
	or      al,ah
	out     dx,al
	inc     bl
	dec     bp
	jnz     wr_this_page
	inc     si
	pop     bx
	add     bx,800h
	cmp     bx,1800h
	ja      read_memory
	jmp     short write_next_page

;************ read mode 1 ************
read_memory:
	loadport
	setport DAT
	xor     bx,bx
	mov     si,bx
read_next_page:
	push    bx
	mov     cx,RW_ADR
	write_sub_fast	bl
	write_sub_fast	bh

	mov     bx,si
	mov     bp,800h
	mov     cx,READ
	mov     ah,ch
	xor     ah,08h
rd_this_page:
	mov     al,ch
	out     dx,al
	pause
	inc     dx
	in      al,dx
	mov     bh,al
	dec     dx
	mov     al,ah
	out     dx,al
	pause
	inc     dx
	in      al,dx
	shr     bh,cl
	and     al,0f0h
	or      bh,al
	cmp     bh,bl
	jne     memory_test
	dec     dx
	inc     bl
	dec     bp
	jnz     rd_this_page
	inc     si
	pop     bx
	add     bx,800h
	cmp     bx,1800h
	ja      mem_is_OK
	push    si
	push    bx
	mov     cx,700h
	mov     al,PAGE0
	call    Write_LoopBack_Data
	call    Tx_Data
	pop     bx
	pop     si
	jmp     short read_next_page
mem_is_OK:
	jmp     mem_OK

;************ write mode 2 ***************
memory_test:
	pop     ax
memory_test1:
	mov     cx,700h
	mov     al,PAGE3
	call    Write_LoopBack_Data
	xor     bx,bx
	mov     si,bx
write_next_page1:
	call    Tx_Data
	push    bx
	mov     cx,RW_ADR
	loadport
	setport DAT
	write_sub_slow	bl
	call    delay
	write_sub_slow	bh
	call    delay

	mov     bx,si
	mov     bp,800h
	mov     cx,WRITE
	mov     ah,ch
	xor     ah,08h
wr_this_page1:
	mov     al,bl
	shl     al,cl
	or      al,ch
	out     dx,al
	call    delay

	mov     al,bl
	and     al,0f0h
	or      al,ah
	out     dx,al
	call    delay
	inc     bl
	dec     bp
	jnz     wr_this_page1
	inc     si
	pop     bx
	add     bx,800h
	cmp     bx,1800h
	ja      read_memory1
	jmp     short write_next_page1

;************* read mode 2 **************
read_memory1:
	loadport
	setport DAT
	xor     bx,bx
	mov     si,bx
read_next_page1:
	push    bx
	mov     cx,RW_ADR
	write_sub_slow	bl
	call    delay
	write_sub_slow	bh
	call    delay

	mov     bx,si
	mov     bp,800h
	mov     cx,READ
	mov     ah,ch
	xor     ah,08h
rd_this_page1:
	mov     al,ch
	out     dx,al
	call    delay

	inc     dx
	in      al,dx
	mov     bh,al
	dec     dx
	mov     al,ah
	out     dx,al
	call    delay

	inc     dx
	in      al,dx
	shr     bh,cl
	and     al,0f0h
	or      bh,al
	cmp     bh,bl
	jne     memory_test2
	dec     dx
	inc     bl
	dec     bp
	jnz     rd_this_page1
	inc     si
	pop     bx
	add     bx,800h
	cmp     bx,1800h
	ja      mem_is_OK1
	push    si
	push    bx
	mov     cx,700h
	mov     al,PAGE0
	call    Write_LoopBack_Data
	call    Tx_Data
	pop     bx
	pop     si
	jmp     read_next_page1
mem_is_OK1:
	jmp     mem_OK1

;************* write mode 3 ***************
memory_test2:
	pop     ax
memory_test3:
	mov     cx,700h
	mov     al,PAGE3
	call    Write_LoopBack_Data
	xor     bx,bx
	mov     si,bx
write_next_page3:
	call    Tx_Data
	push    bx
	mov     cx,RW_ADR
	loadport
	setport DAT
	write_sub_delay	bl
	call    delay
	write_sub_delay	bh
	call    delay

	mov     bx,si
	mov     bp,800h
	mov     cx,WRITE
wr_this_page3:
	mov     al,bl
	shl     al,cl
	or      al,ch
	xor     al,08h
	out     dx,al
	call    delay

	xor     al,08h
	out     dx,al
	call    delay

	mov     al,bl
	and     al,0f0h
	or      al,ch
	out     dx,al
	call    delay

	xor     al,08h
	out     dx,al
	call    delay
	inc     bl
	dec     bp
	jnz     wr_this_page3
	inc     si
	pop     bx
	add     bx,800h
	cmp     bx,1800h
	ja      read_memory3
	jmp     write_next_page3

;************* read mode 3 ***************
read_memory3:
	loadport
	setport DAT
	xor     bx,bx
	mov     si,bx
read_next_page3:
	push    bx
	mov     cx,RW_ADR
	write_sub_delay	bl
	call    delay
	write_sub_delay	bh

	call    delay

	mov     bx,si
	mov     bp,800h
	mov     cx,READ
	mov     ah,ch
	xor     ah,08h

	mov     al,ch
	xor     al,08h
	out     dx,al
	call    delay
rd_this_page3:
	mov     al,ch
	out     dx,al
	call    delay

	setport STAT
	in      al,dx
	mov     bh,al
	setport DAT
	mov     al,ah
	out     dx,al
	call    delay

	setport STAT
	in      al,dx
	shr     bh,cl
	and     al,0f0h
	or      bh,al
	cmp     bh,bl
	jne     mem_err
	setport	DAT
	inc     bl
	dec     bp
	jnz     rd_this_page3
	inc     si
	pop     bx
	add     bx,800h
	cmp     bx,1800h
	ja      mem_OK2
	push    si
	push    bx
	mov     cx,700h
	mov     al,PAGE0
	call    Write_LoopBack_Data
	call    Tx_Data
	pop     bx
	pop     si
	jmp     read_next_page3

pointer         dw      0

mem_err:
	pop     bx
	cmp     pointer,9		;too slow?  Must not be working.
	ja      mem_real_err
	mov     bx,offset delay		;append another NOP and RET in.
	add     bx,pointer
	mov     [bx],0c390h
	inc     pointer			;slow it down a little more and try
	jmp     memory_test3		;  again.
mem_real_err:
	jmp     mem_error

change_routine:
	mov     ax,offset recv1
	mov     recv_pointer,ax
	mov     ax,offset send_pkt1
	mov     send_pkt_pointer,ax
	ret

change_routine2:
	mov     ax,offset recv2
	mov     recv_pointer,ax
	mov     ax,offset send_pkt2
	mov     send_pkt_pointer,ax
	ret

;********** memory test passed ****************
mem_OK1:
	call    change_routine
	jmp     short mem_OK
mem_OK2:
	call    change_routine2
mem_OK:
	call    speed_test

	push    es
	xor     ax,ax
	mov     es,ax
	mov     si,printer
	mov     word ptr es:[si],ax     ; Zero-out Printer Port
	pop     es

; Initialize Rx buffer pointer, and start receive
	mov     bl,Mode
	or      bl,IRQinverse
	or      bl,CurRXPage
	mov     Mode_RxPg,bl
	mov     cx,COMMAND
	call    Write_sub
	or      bl,RXEN
	call    Write_sub

; Put our Receive routine in interrupt chain
	call    set_recv_isr

; We didn't need to enable the receive & transmit interrupts, they were
; set by hardware already. (accept GOOD, SUC & T16 to generate interrupt)

; Enable Printer Adapter IRQ line
	mov     al,SLT_PRN
	call    CMD_sub

	mov     dx,offset end_resident
	clc
	ret


;*********** sub-routine *************
; Check DE-600 routine
Check_DE600:
	mov     al,SLT_NIC
	call    CMD_sub
	call    delay

	loadport
	setport DAT
	mov     al,NUL_CMD
	out     dx,al
	call    delay

	mov     bl,RESET
	mov     cx,COMMAND
	call    Write_sub
	call    delay
	mov     bl,STOP_RESET
	call    Write_sub
	call    delay

	mov     cx,STATUS
	call    Read_sub
	test    bl,0f0h
	jz      Check_OK
	stc
	ret
Check_OK:
	clc
	ret

OldIRQ5 dd      0
OldIRQ7 dd      0

NewIRQ5:
	push    ax
	push    bx
	push    cx
	push    dx
	push    ds
	mov     al,20h
	out     20h,al
	mov     ax,cs
	mov     ds,ax
	cmp     LB,0
	jz      DisCare_IRQ5
	mov     bh,5
	call    Clear_int
DisCare_IRQ5:
	pop     ds
	pop     dx
	pop     cx
	pop     bx
	pop     ax
	iret

NewIRQ7:
	push    ax
	push    bx
	push    cx
	push    dx
	push    ds
	mov     al,20h
	out     20h,al
	mov     ax,cs
	mov     ds,ax
	cmp     LB,0
	jz      DisCare_IRQ7
	mov     bh,7
	call    Clear_int
DisCare_IRQ7:
	pop     ds
	pop     dx
	pop     cx
	pop     bx
	pop     ax
	iret

replace_IRQ5_7:
	xor     cx,cx
	mov     es,cx
	mov     di,034h
	mov     ax,es:[di]                      ;save old interrupt vector
	mov     word ptr OldIRQ5,ax
	mov     ax,es:[di]+2
	mov     word ptr OldIRQ5+2,ax
	mov     ax,offset NewIRQ5
	stosw
	mov     ax,cs
	stosw

	mov     di,03ch
	mov     ax,es:[di]                      ;save old interrupt vector
	mov     word ptr OldIRQ7,ax
	mov     ax,es:[di]+2
	mov     word ptr OldIRQ7+2,ax
	mov     ax,offset NewIRQ7
	stosw
	mov     ax,cs
	stosw

	in      al,21h
	mov     intmask,al
	pause
	pause
	and     al,5fh
	out     21h,al
	ret

intmask         db      0
INT_come        db      0
T16_flag        db      0
LB              db      0

restore_IRQ5_7:
	xor     cx,cx
	mov     es,cx
	mov     di,034h
	mov     ax,word ptr OldIRQ5
	mov     es:[di],ax                      ;save old interrupt vector
	mov     ax,word ptr OldIRQ5+2
	mov     es:[di]+2,ax

	mov     di,03ch
	mov     ax,word ptr OldIRQ7
	mov     es:[di],ax                      ;save old interrupt vector
	mov     ax,word ptr OldIRQ7+2
	mov     es:[di]+2,ax

	mov     al,intmask
	out     21h,al
	ret

Write_LoopBack_Data:
	mov     si,offset our_address
	mov     di,si
;set Tx Pointer for moving packet
	mov     bx,BFRSIZ
	sub     bx,cx           ;CX= Packet Length
	or      bh,al           ;AL= Page Number
	mov     TxStartAdd,bx   ;BX= the pointer to TX
	mov     cx,RW_ADR       ;write memory address
	call    Write_sub
	mov     bl,bh
	call    Write_sub
	cld
	loadport
	setport DAT
	mov     bp,12
	mov     cx,WRITE
write_our_node_ID:
	lodsb
	mov     bl,al
	call    Write_sub
	cmp     bp,7
	jne     not_second_ID
	mov     si,di
not_second_ID:
	dec     bp
	jnz     write_our_node_ID
	ret

Tx_Data:
;Check TXIDLE, if high then wait for previous Tx end, if low then Tx it
	mov     cx,800h        ; Avoid infinite loop
	loadport
	setport STAT
wait_Txidle0:
	in      al,dx
	test    al,TXBUSY       ; Is the previous Tx successful ?
	jz      Tx_next0        ; Yes, TXBUSY is low. Then could Tx next packet.
	loop    wait_Txidle0
Tx_next0:
;set Tx Pointer at beginning of packet
	push    bx
	mov     cx,TX_ADR
	mov     bx,TxStartAdd
	call    Write_sub
	mov     bl,bh
	call    Write_sub
;Enable interrupt and start Tx
	mov     cx,COMMAND
	mov     bl,RX_NONE
	call    Write_sub
	or      bl,TXEN
	call    Write_sub
	pop     bx
	ret

LoopBack_Tx:
;set Tx Pointer at beginning of packet
	mov     bx,TxStartAdd
	mov     cx,TX_ADR
	call    Write_sub
	mov     bl,bh
	call    Write_sub
;Enable interrupt and start Tx
	mov     bl,RX_BP
	or      bl,IRQinverse
	mov     cx,COMMAND
	call    Write_sub
	or      bl,LOOPBACK
	call    Write_sub

	mov     LB,1
	mov     al,SLT_PRN
	call    CMD_sub

	xor     bx,bx
	mov     cx,8000h
wait_int:
	cmp     INT_come,0
	jz      have_T16
	mov     bx,1
	jmp     short exit_LoopBack
have_T16:
	cmp     T16_flag,0
	jz      still_wait
	mov     bx,-1
	jmp     short exit_LoopBack
still_wait:
	loop    wait_int

exit_LoopBack:
	mov     LB,0
	mov     al,SLT_NIC
	call    CMD_sub

	push    bx
	mov     cx,STATUS
	call    Read_sub
	pop     bx
	ret

Clear_int:
	mov     al,SLT_NIC
	call    CMD_sub
	pause
	mov     cx,STATUS
	call    Read_sub

	mov     T16_flag,0
	test    bl,GOOD         ; Is Rx generating interrupt ?
	jz      chk_T16
	mov     INT_come,1
	mov     int_no,bh
	jmp     short exit_Clear_int
chk_T16:
	test    bl,T16          ; Is pending a Tx Packet ?
	jz      exit_Clear_int
	mov     T16_flag,1
exit_Clear_int:
	ret

Check_IRQ:
	call    replace_IRQ5_7
	sti
	mov     cx,RUNT
	mov     al,PAGE0
	call    Write_LoopBack_Data
	call    LoopBack_Tx     ; check IRQ= 7 or 5 but IRQ not inverse
	cmp     bx,0
	jnz     IRQ_OK

;Check TXIDLE, if high then wait for previous Tx end, if low then Tx it
	mov     cx,800h        ; Avoid infinite loop
	loadport
	setport STAT
wait_Txidle:
	in      al,dx
	test    al,TXBUSY       ; Is the previous Tx successful ?
	jz      Tx_next         ; Yes, TXBUSY is low. Then could Tx next packet.
	loop    wait_Txidle
Tx_next:
	mov     IRQinverse,40h  ; check IRQ= 7 or 5 but IRQ inverse
	mov     PS2,0
	call    LoopBack_Tx
IRQ_OK:
	cli
	call    restore_IRQ5_7
	ret

check_PS2:
	mov     ax,0c400h
	int     15h
	jc      not_PS2
	mov     PS2,1
not_PS2:
	ret

speed_test:
	xor     ax,ax
	mov     es,ax
	mov     si,20h
	mov     ax, es:[si]
	mov     cs:old_int8, ax
	mov     ax, es:[si+2]
	mov     cs:old_int8[2], ax
	cli
	mov     ax,offset new_int8
	mov     es:[si],ax
	mov     es:[si+2],cs
	sti

next_test1:
	mov     ticks_start,0
next_test:
	cmp     ticks_start,0
	jz      next_test
	mov     ticks,0
	xor     bx,bx
loop_again:
	mov     cx,6
	loop    $
	cmp     ticks,2
	jae     End_count
	inc     bx
	jmp     short loop_again

End_count:
	cli
	xor     ax,ax
	mov     es,ax
	mov     si,20h
	mov     ax,old_int8
	mov     es:[si],ax
	mov     ax,old_int8[2]
	mov     es:[si+2],ax
	sti
	cmp     bx,0a000h
	jb      low_speed
	mov     ax,offset recv1         ; special for high speed EISA
	mov     recv_pointer,ax
low_speed:
	ret

ticks_start     db      0
ticks           db      0
old_int8        dw      ?
	        dw      ?
new_int8:
	inc     ticks
	inc     ticks_start
	jmp     dword ptr cs:old_int8

	code    ends
	end
