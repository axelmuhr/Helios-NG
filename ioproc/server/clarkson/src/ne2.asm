; Packet driver for Novell's NE/2
; Written by:
;	Eric Henderson
;	Brigham Young University
;	
; Based on the "generic" packet driver by Russell Nelson with help
; from the western digital pd by Russell Nelson.
; 80[123]86 processor support lifted from 3com driver by permission
;	from Russell Nelson
;
;   Portions (C) Copyright 1990 BYU

version	equ	5

.286
	include	defs.asm

code	segment	byte public
	assume	cs:code, ds:code

;*****************************************************************************
;
;	NE/2 controller board offsets
;	IO port definition (BASE in io_addr)
;*****************************************************************************
ADDROM  EQU	10h			; LAN Address ROM
RACK	EQU	10h			; NE2 Port Window 
NERESET EQU	20h			; Issue a read for reset

; 8390 LAN Controller (page0) register offset for read and write 
CMDR	EQU	00h			; command register for read & write
CLDA0	EQU	01h			; current local dma addr 0 for read
PSTART	EQU	01h			; page start register for write
CLDA1	EQU	02h			; current local dma addr 1 for read
PSTOP	EQU	02h			; page stop register for write
BNRY	EQU	03h			; boundary reg for rd and wr
TSR	EQU	04h			; tx status reg for rd
TPSR	EQU	04h			; tx start page start reg for wr	
NCR	EQU	05h			; number of collision reg for rd
TBCR0	EQU	05h			; tx byte count 0 reg for wr
FIFO	EQU	06h			; FIFO for rd
TBCR1	EQU	06h			; tx byte count 1 reg for wr
ISR	EQU	07h			; interrupt status reg for rd and wr
CRDA0	EQU	08h			; current remote dma address 0 for rd
RSAR0	EQU	08h			; remote start address reg 0  for wr
CRDA1	EQU	09h			; current remote dma address 1 for rd
RSAR1	EQU	09h			; remote start address reg 1 for wr
RBCR0	EQU	0Ah			; remote byte count reg 0 for wr
RBCR1	EQU	0Bh			; remote byte count reg 1 for wr
RSR	EQU	0Ch			; rx status reg for rd
RCRWD	EQU	0Ch			; rx configuration reg for wr
CNTR0	EQU	0Dh			; tally cnt 0 for frm alg err for rd
TCR	EQU	0Dh			; tx configuration reg for wr
CNTR1	EQU	0Eh			; tally cnt 1 for crc err for rd
DCR	EQU	0Eh			; data configuration reg for wr
CNTR2	EQU	0Fh			; tally cnt 2 for missed pkt for rd
IMR	EQU	0Fh			; interrupt mask reg for wr
; 8390 LAN Controller (page1) register offset for read and write 
PAR0	EQU	01h 			; physical addr reg 0 for rd and wr
PAR1	EQU	02h 			; physical addr reg 1 for rd and wr
PAR2	EQU	03h 			; physical addr reg 2 for rd and wr
PAR3	EQU	04h 			; physical addr reg 3 for rd and wr
PAR4	EQU	05h 			; physical addr reg 4 for rd and wr
PAR5	EQU	06h 			; physical addr reg 5 for rd and wr
CURR	EQU	07h			; current page reg for rd and wr
MAR0	EQU	08h			; multicast addr reg 0 fro rd and WR
MAR1	EQU	09h			; multicast addr reg 1 fro rd and WR
MAR2	EQU	0Ah			; multicast addr reg 2 fro rd and WR
MAR3	EQU	0Bh			; multicast addr reg 3 fro rd and WR
MAR4	EQU	0Ch			; multicast addr reg 4 fro rd and WR
MAR5	EQU	0Dh			; multicast addr reg 5 fro rd and WR
MAR6	EQU	0Eh			; multicast addr reg 6 fro rd and WR
MAR7	EQU	0Fh			; multicast addr reg 7 fro rd and WR

;***********************************************************************
;
;	8003 control register operations
;***********************************************************************

MSK_RESET	EQU	80h	        ; reset LAN controller
MSK_ENASH	EQU	40h		; enable PC access to shared mem
MSK_DECOD	EQU	3Fh 		; ???? memory decode bits, corresponding
					; to SA 18-13. SA 19 assumed to be 1
;***********************************************************************
;
;	8390 CMDR MASK
;***********************************************************************

MSK_STP		EQU	01h		; software reset, take 8390 off line
MSK_STA		EQU	02h		; activate the 8390 NIC
MSK_TXP		EQU	26h		; initial txing of a frm  (With DMA)
MSK_RD2		EQU	20h		; abort remote DMA
MSK_PG0		EQU	00h		; select register page 0
MSK_PG1		EQU	40h		; select register page 1
MSK_PG2		EQU	80h		; select register page 2
MSK_DMA_RD	EQU	0ah		; start DMA read
MSK_DMA_WR	EQU	12h		; start DMA write

;***********************************************************************
;
;	8390 ISR & IMR MASK
;***********************************************************************

MSK_PRX  EQU	01h		; rx with no error
MSK_PTX  EQU	02h		; tx with no error
MSK_RXE  EQU	04h		; rx with error
MSK_TXE  EQU	08h		; tx with error
MSK_OVW  EQU	10h		; overwrite warning
MSK_CNT  EQU	20h		; MSB of one of the tally counters is set
MSK_RDC  EQU	40h		; remote dma completed
MSK_RST	 EQU	80h		; reset state indicator
MaskByte		equ	0
UnmaskByte		equ	1fh
InterruptMask		equ	0fh

;***********************************************************************
;
;	8390 DCR MASK
;***********************************************************************

MSK_WTS EQU	01h		; word transfer mode selection
MSK_BOS	EQU	02h		; byte order selection
MSK_LAS	EQU	04h		; long addr selection
MSK_BMS	EQU	08h		; burst mode selection
MSK_ARM	EQU	10h		; atuoinitialize remote
MSK_FT00 EQU	00h		; burst lrngth selection
MSK_FT01 EQU	20h		; burst lrngth selection
MSK_FT10 EQU	40h		; burst lrngth selection
MSK_FT11 EQU	60h		; burst lrngth selection

;***********************************************************************
;
;	8390 RCR MASK
;***********************************************************************

MSK_SEP EQU	01h		; save error pkts
MSK_AR 	EQU	02h		; accept runt pkt
MSK_AB 	EQU	04h		; accept broadcast 
MSK_AM 	EQU	08h		; accept multicast 
MSK_PRO	EQU	10h		; promiscuous physical
				; accept all pkt with physical adr
MSK_MON EQU	20h		; monitor mode

;***********************************************************************
;
;	8390 TCR MASK
;***********************************************************************

MSK_CRC EQU	01h		; inhibit CRC, do not append crc
MSK_LB01 EQU	06h		; encoded loopback control
MSK_ATD	EQU	08h		; auto tx disable
MSK_OFST EQU	10h		; collision offset enable 

;***********************************************************************
;
;	8390 RSR MASK
;***********************************************************************

SMK_PRX  EQU	01h		; rx without error
SMK_CRC  EQU	02h		; CRC error
SMK_FAE  EQU	04h		; frame alignment error
SMK_FO   EQU	08h		; FIFO overrun
SMK_MPA  EQU	10h		; missed pkt
SMK_PHY  EQU	20h		; physical/multicase address
SMK_DIS  EQU	40h		; receiver disable. set in monitor mode
SMK_DEF	 EQU	80h		; deferring

;***********************************************************************
;
;	8390 TSR MASK
;***********************************************************************

SMK_PTX  EQU	01h		; tx without error
SMK_DFR  EQU	02h		; non deferred tx
SMK_COL  EQU	04h		; tx collided
SMK_ABT  EQU	08h		; tx aboort because of excessive collisions
SMK_CRS  EQU	10h		; carrier sense lost
SMK_FU   EQU	20h		; FIFO underrun
SMK_CDH  EQU	40h		; collision detect heartbeat
SMK_OWC	 EQU	80h		; out of window collision

;***********************************************************************
;
;	on board memory constant definition
;***********************************************************************
; for rcv buff ring of onboard mem
START_PG EQU	46h	      		; start at page 46
STOP_PG  EQU	80h			; end at page 80 
; for tx buff of shr mem
TB_SIZE EQU	1			; number of tb buff in shr mem
TB_PGNO EQU	6			; number of pages in one tb buff

EIGHTBITSLOT	db	0		;8-bit machine flag
Path		dw	?
RxPath		dw	?

extrn		is_at: byte

	public	int_no, io_addr
int_no		db	3,0,0,0		;must be four bytes long for get_number.
io_addr		dw	1000h,0		; I/O address for card (jumpers)
my_eaddr  	db	6 dup(?)	; 6 byte LAN address
m_channel 	dw 	0		; micro channel flag	
is_186		db	0		;=0 if 808[68], =1 if 80[123]86.

rd_NE	MACRO	port
	mov	DX, CS:io_addr
	add	DX, port		; DX contains address of port
	in	AL, DX			; AL contains data read from port
	ENDM

wr_NE	MACRO	port
	mov	DX, CS:io_addr
	add	DX, port		; DX contains address of port
	out	DX, AL			; AL contains data to be written to port
	ENDM

ReceiveHeaderStructure	struc
   RReceiveStatus	db	?
   RNextBuffer		db	?
   RByteCount		dw	?

   RDestinationAddress	db	6 dup(?)
   RSourceAddress	db	6 dup(?)
   RPacketLength	dw	?
   RChecksum		dw	?
   RRPacketLength	dw	?
   RTranControl		db	?
   RHPacketType		db	?
   RDestinationNet	db	4 dup(?)
   RDestinationNode	db	6 dup(?)
   RDestinationSocket	dw	?
ReceiveHeaderStructure	ends

ReceiveHeader	ReceiveHeaderStructure	<>

Current				equ	InterruptStatus
CurrentDMA0			equ	RemoteStartAddress0
CurrentDMA1			equ	RemoteStartAddress1
ForRSTBit			equ	80h

NIC	struc
	Command			db	?
	PageStart		db	?
	PageStop		db	?
	Boundry			db	?
	TransmitStatus		db	?
	TransmitByteCount0	db	?
	TransmitByteCount1	db	?
	InterruptStatus		db	?
	RemoteStartAddress0	db	?
	RemoteStartAddress1	db	?
	RemoteByteCount0	db	?
	RemoteByteCount1	db	?
	ReceiveConfiguration	db	?
	TransmitConfiguration	db	?
	DataConfiguration	db	?
	IntMask			db	?
	DataPort		db	?
				db	15 dup (?)
	Reset			db	?
NIC	ends

BLUEBOOK	equ	1
IEEE8023	equ	11
	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023,  0	;from the packet spec
driver_type	dw	0ffffh		;Wild card matches any type
driver_name	db	'NE/2',0	;name of the driver.
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
	dw	0	;Interrupt # to hook for post-EOI
			;processing, 0 == none,

mcast_list_bits db      0,0,0,0,0,0,0,0 ;Bit mask from last set_multicast_list
mcast_all_flag  db      0               ;Non-zero if hware should have all

	public	rcv_modes
rcv_modes	dw	7		;number of receive modes in our table.
		dw	0		;there is no mode 1.
		dw	rcv_mode_1
		dw	rcv_mode_2
		dw	rcv_mode_3
		dw	rcv_mode_4
		dw	rcv_mode_5
		dw	rcv_mode_6
rxcr_bits       db      MSK_AB		; Default to ours plus multicast


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
; get txblk length						  
	inc	cx
	and	cl, 0feh
	cmp	CX, RUNT
	jnb	length_ok
	mov	cx, RUNT
length_ok:
	cmp	cx, GIANT
	jbe	length1_ok
	mov	dh, NO_SPACE
	stc	
	jmp	count_out_err
length1_ok:
;new stuff
	mov	AX, CX
	wr_NE	TBCR0			; Transmit byte count
	mov	al, ah
	wr_NE	TBCR1
	mov	al, 0
	wr_NE	RSAR0
	mov	al, 40h
	wr_NE	RSAR1
	mov	ax, cx
	wr_NE	RBCR0			; Remote byte count
	mov	al, ah
	wr_NE	RBCR1

; Clear out DMA complete interrupt
	mov	al, MSK_PG0		
	wr_NE	CMDR
	mov	al, 40h
	wr_NE	ISR

	mov	al, MSK_DMA_WR
	wr_NE	CMDR

	mov	DX, CS:io_addr
	add	DX, RACK		; DX has address NE/2 Port window 

	shr	cx, 1
	rep	outsw

	xor	cx, cx			; Prevent infinite loop
WaitForDMAComplete:
	rd_NE	ISR
	test	al, 40h
	jnz	DMAComplete
	loop	WaitForDMAComplete

DMAComplete:
	mov	al, MSK_TXP
	wr_NE	CMDR
	clc
exit_now:
	ret


	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	cmp cx,	EADDR_LEN	; Caller wants a reasonable length?
	jb	get_addr_x	; No, fail.
	mov cx,	EADDR_LEN	; Yes. Set count for loop
	mov	si, offset cs:my_eaddr
	cld			; Make sure string mode is right
	rep	movsb
	mov cx,	EADDR_LEN	; Tell caller how many bytes we fed him
	clc			; Carry off says success
	ret
get_addr_x:
	stc			; Tell caller our addr is too big for him
	ret


	public	set_address
set_address:
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
;This proc will set the first CX bytes of the ethernet address, leaving
; the rest unchanged.
	assume	ds:nothing

	cmp	cx, 6
	jbe	set1
	mov	dh, BAD_ADDRESS
	stc
	ret
set1:
	push	es
	push	di
	mov	di, offset my_eaddr
	mov	ax, cs
	mov	es, ax

	mov	al, 61h
	wr_NE	CMDR
	mov	DX, CS:io_addr
	add	DX, PAR0		; DX has address 8390 Phys Address Reg
AddressToChip1:
	lodsb
	out	dx, al
	mov	es:[di], al
	inc	di
	inc	dx
	nop
	nop
	nop	
	nop
	loop	AddressToChip1
	pop	di
	pop	es

	mov 	al, 21h
	wr_NE 	CMDR
	clc
	ret

; Routines to set address filtering modes in the DS8390
;	This was lifted from R. Clements' WD PD
rcv_mode_1:     ; Turn off receiver
	mov al,	MSK_MON      ; Set to monitor for counts but accept none
	jmp short rcv_mode_set
rcv_mode_2:     ; Receive only packets to this interface
	mov al, 0               ; Set for only our packets
	jmp short rcv_mode_set
rcv_mode_3:     ; Mode 2 plus broadcast packets (This is the default)
	mov al,	MSK_AB     ; Set four ours plus broadcasts
	jmp short rcv_mode_set
rcv_mode_4:     ; Mode 3 plus selected multicast packets
	mov al,	MSK_AB+MSK_AM ; Ours, bcst, and filtered multicasts
	mov     mcast_all_flag,0
	jmp short rcv_mode_set
rcv_mode_5:     ; Mode 3 plus ALL multicast packets
	mov al,	MSK_AB+MSK_AM; Ours, bcst, and filtered multicasts
	mov     mcast_all_flag,1
	jmp short rcv_mode_set
rcv_mode_6:     ; Receive all packets (Promiscuous physical plus all multi)
	mov al,	MSK_AB+MSK_AM+MSK_PRO
	mov     mcast_all_flag,1
rcv_mode_set:
	push    ax              ; Hold mode until masks are right
	call    set_8390_multi  ; Set the multicast mask bits in chip
	pop     ax
	WR_NE	RCRWD
	mov     rxcr_bits,al    ; Save a copy of what we set it to
	ret

	public	set_multicast_list
set_multicast_list:
;enter with ds:si ->list of multicast addresses, cx = number of addresses.
;return nc if we set all of them, or cy,dh=error if we didn't.
	mov	dh,NO_MULTICAST
	stc
	ret

; Set the multicast filter mask bits in case promiscuous rcv wanted
;	This was lifted from R. Clements' WD PD
set_8390_multi:
	
	mov	al,MSK_RD2+MSK_PG1
	WR_NE	CMDR 
	mov	cx,8		; Eight bytes of multicast filter
	mov	si,offset mcast_list_bits  ; Where bits are, if not all ones
	push    cs
	pop     ds
	cli			; Protect from irq changing page bits

	mov	DX, CS:io_addr
	add	DX, MAR0
	mov	al,mcast_all_flag  ; Want all ones or just selected bits?
	or	al,al
	jz	set_mcast_2     ; z = just selected ones
	mov	al,0ffh		; Ones for filter
set_mcast_all:
	out	dx, al
	inc	dl		; Step to next one
	jmp	$+2		; limit chip access rate
	loop	set_mcast_all
	jmp short set_mcast_x

set_mcast_2:
	lodsb                   ; Get a byte of mask bits
	out	dx,al		; Write a mask byte
	inc	dl		; Step to next I/O register
	jmp	$+2		; limit chip access rate
	loop	set_mcast_2
set_mcast_x:
	mov	al, MSK_RD2+MSK_PG0
	WR_NE	CMDR
	sti			; OK for interrupts now
	ret


	public	terminate
terminate:
	ret


	public	get_multicast_list
get_multicast_list:
;return with nc, es:di ->list of multicast addresses, cx = number of bytes.
;return cy, NO_ERROR if we don't remember all of the addresses ourselves.
;return cy, NO_MULTICAST if we don't implement multicast.
	mov	dh,NO_MULTICAST
	stc
	ret


	public	reset_interface
reset_interface:
;reset the interface.
	assume	ds:code
	mov	al, MSK_STP + MSK_RD2	
	wr_NE	CMDR
	mov al,	0ffh		; Clear all pending interrupts
	wr_NE	ISR
	xor al,	al		; Turn off all enables
	wr_NE	IMR
	wr_NE	NERESET		; Hard reset NE/2
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET

	rd_NE	NERESET

	mov	al, 21h
	wr_NE	CMDR
	ret


;called when we want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type, dl = packet class.
;It returns with es:di = 0 if don't want this type or if no buffer available.	
	extrn	recv_find: near

;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
	extrn	recv_copy: near

	extrn	count_in_err: near
	extrn	count_out_err: near


	public	recv
recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Actually, not just receive, but all interrupts come here.
;Upon exit, the interrupt will be acknowledged.
	assume	ds:code

; read irq status register
	mov	al, MaskByte
	wr_NE	IMR
	sti
rd_isr:
	rd_NE	ISR			; read isr into AL
	and	AL, 3Fh			; check bit0-bit5, if all 1's no irq
	cmp	AL, 0			; if any irq
	jne	tst_ovw			; some irq
	cli
	mov	al, UnmaskByte
	wr_NE	IMR
	ret				; no more irq, exit

; process OVW	(OVW = 1)       
;    may report error here
tst_ovw:
	test	AL, MSK_OVW		; if OVW irq
	jnz	prcs_ov			; OVW (OVW = 1) 
	jmp	test_rx			; no OVW (OVW = 0) 

; **************************************************************
; follow the DP8390 datasheet addendum to handle the buff ring overflow
prcs_ov:
; 1. issue a STOP mode command
	mov	AL, MSK_STP + MSK_RD2
	wr_NE	CMDR
; 6. remove one packet from the ring
	rd_NE	BNRY			; BNRY in AL
	add	AL, 1			; start page of frm in AL
	cmp	AL, STOP_PG		; check boundary
        jne	get1
	mov	AL, START_PG		
; ring not empty
get1:
	mov	BH, AL			; BX has the rx_frm pointer
	mov	al, SIZE ReceiveHeader
	wr_NE	RBCR0
	xor	al, al
	wr_NE	RBCR1
	wr_NE	RSAR0
	mov	al, bh
	wr_NE	RSAR1
	mov	al, MSK_DMA_RD
	wr_NE	CMDR

	mov	DX, CS:io_addr
	add	DX, RACK		; DX has address NE/2 Port window 
	mov	di, OFFSET ReceiveHeader
	mov	ax, cs
	mov	es, ax
	mov	cx, SIZE ReceiveHeader
	shr	cx, 1	
Receive1:
rep	insw

SkipReceive1:
	mov	bx, offset ReceiveHeader
	mov	AL, CS:[BX]		; AL has the status byte
	test	AL, SMK_PRX		; if rx good
	jz	fd_bnr			; rx error, drop frm by forward bnry

; good frm, call _rcv_frm
	call 	_rcv_frm   		

fd_bnr:					;drop frm by forward BNRY
	mov	al, cs:ReceiveHeader.RNextBuffer	; al = next pointer 
	sub	AL, 1			; new BNRY in AL
	cmp	AL, START_PG		; check boundary
	jae	wrbnr			; unsigned arithmetic
	mov	AL, STOP_PG - 1		;
;	dec	AL			;
wrbnr:
	wr_NE	BNRY
; 2. clear the remote byte count registers (RBCR0,RBCR1)
	xor	AL, AL
	wr_NE	RBCR0
	wr_NE	RBCR1
; 3. poll the ISR for the RST bit
plisr:
	mov	cx, 0ffffh
	rd_NE	ISR
	test	AL, MSK_RST
	jnz	plisr_ok
	loop	plisr		; keep polling until the RST bit set
; 4. place the NIC in loopback mode (mode 1 or 2) by writing 02 or 04 to TCR
plisr_ok:
	mov	AL, 02h		; put it in mode 2 (internal loopback)
	wr_NE	TCR
; 5. issue start mode command
	mov	AL, MSK_STA + MSK_RD2
	wr_NE	CMDR
; 7. out from loopback mode by writing 00 to TCR
	xor	AL, AL
	wr_NE	TCR		; normal operation configuration
; clear OVW in ISR		      
	mov	AL, MSK_OVW 
	wr_NE	ISR			; clear OVW
	call	count_in_err		; increment overflow counter
	jmp	rd_isr			; back to the top


; end of the modification 
; *****************************************************
;	
;process PRX and RXE
;
test_rx:	    
	test 	AL, MSK_RXE		
	jnz	prcs_rxe		; RXE = 1
	test  	AL, MSK_PRX	
	jnz	prcs_rx	     		; PRX = 1
	jmp	test_tx

prcs_rxe:
	call	count_in_err
	mov	al, MSK_RXE
	wr_NE	ISR
	jmp	rd_isr
prcs_rx:
	mov	AL, MSK_PG1 + MSK_RD2	; read CURR reg
	wr_NE	CMDR
	rd_NE	CURR
	mov	BL, AL			; CURR in BL 
	mov	AL, MSK_PG0 + MSK_RD2	; read BNRY reg
	wr_NE  CMDR
	rd_NE	BNRY			; BNRY in AL
	add	AL, 1			; start page of frm in AL
	cmp	AL, STOP_PG	 	; check boundary
	jne	go_cmp
	mov	AL, START_PG 		; 		
go_cmp:
	cmp	AL, BL			
	jne	Ring_Not_Empty
	jmp	rd_isr			; buff ring empty

Ring_Not_Empty:
	wr_NE	RSAR1
	xor	al, al
	wr_NE	RBCR1
	wr_NE	RSAR0
	mov	al, SIZE ReceiveHeader
	wr_NE	RBCR0
	mov	al, MSK_DMA_RD
	wr_NE	CMDR

	mov	DX, CS:io_addr
	add	DX, RACK		; DX has address NE/2 Port window 
	mov	di, OFFSET ReceiveHeader
	mov	ax, cs
	mov	es, ax
	mov	cx, SIZE ReceiveHeader
	shr	cx, 1

Receive2:
rep	insw

SkipReceive2:
	mov	bx, offset ReceiveHeader
	mov	AL, CS:[BX]		; AL has the status byte
	test	AL, SMK_PRX		; if rx good
	jz	fd_bnry			; rx error, drop frm by forward bnry

; good frm, call _rcv_frm
   	call 	_rcv_frm   		

fd_bnry:				; drop frm by forward BNRY
	mov	al, CS:ReceiveHeader.RNextBuffer	; al = next pointer 
	sub	AL, 1			; new BNRY in AL
	cmp	AL, START_PG		; check boundary
	jae	wrbnry			; unsigned arithmetic
	mov	AL, STOP_PG - 1		;

wrbnry:
	wr_NE	BNRY
	mov	al, MSK_PRX
	wr_NE	ISR
	jmp	prcs_rx	 

;process PTX and TXE
test_tx:
	test  	AL, MSK_PTX	
	jnz	prc_ptx	     		; PTX = 1
	test 	AL, MSK_TXE		
	jnz	prc_txe			; TXE = 1
	jmp	test_cnt
      

; process tx good, update txok, lostcrs, collsn
prc_ptx:				; tx good 
	rd_NE	TSR
	test	AL, SMK_CRS		; is crs set in TSR
	jz	nocrs			; no		       
nocrs:	
	rd_NE	NCR			; read number of collision in AL
	mov	AL, MSK_PTX
	wr_NE	ISR			; clear PTX
	jmp	rd_isr

; process tx error, update .txbad .underrun
prc_txe:				; tx bad
	call	count_out_err	
	rd_NE	TSR
	test	AL, SMK_FU		; it fu set in TSR
	jz	nofu			; no		       
nofu:								  
    	mov	AL, MSK_TXE
	wr_NE	ISR			; clear PTX
	jmp	rd_isr

; process counter overflow, update .algerr .crcerr .???(missed pkt)
test_cnt:
	test  	AL, MSK_CNT	
	jnz	prc_cnt			; yes, process cnt
	jmp	rd_isr	     		; no CNT irq, back to the top
; process CNT			
prc_cnt:
	mov	AL, MSK_CNT
	wr_NE	ISR			; clear CNT
	jmp	rd_isr			; back to the top

; End of RECV

_rcv_frm:
	
; read byte count 
	mov 	cx, cs:ReceiveHeader.RByteCount	; Extract size of frame
	cmp	CX, 5dch
	jna	rxlen_ok
	jmp	rcv_ret
rxlen_ok:
	sub	CX, 4			; 4 control bytes
	mov	AX, cs
	mov	ES, AX
	mov 	di, offset cs:ReceiveHeader.RPacketLength
	mov	dl, BLUEBOOK		;default 
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	mov	di, offset cs:ReceiveHeader.RChecksum
	mov	dl, IEEE8023
BlueBookPacket:

	push	cx			; Save frame size
	push	es

	mov ax,	cs			; Set ds = code
	mov ds,	ax
	assume	ds:code
	call	recv_find		; See if type and size are wanted
					; 	CX = packet length
					;    ES:DI = packet type
					;    ES:DI = packet type

	pop	ds			; RX page pointer in ds now
	assume	ds:nothing
	pop	cx

	cld				; Copies below are forward
	mov ax,	es			; Did recv_find give us a null pointer?
	or  ax,	di			; ..
	je	no_buff			; If null, don't copy the data	
has_buf:  

;Tell DMA to copy the whole packet for us
	mov	ax, 4
	wr_NE	RSAR0		; Don't copy  4 8390 control bytes
	mov	ax, cx		; CX has byte count
	wr_NE	RBCR0		; LSB first
	mov	al, ah
	wr_NE	RBCR1		; Now MSB

	mov	al, MSK_DMA_RD  ; Issue DMA read command
	wr_NE	CMDR

; copy from NE/2 on board memory using the window RACK
; use IN and stosb to do the copy, IN -> AX -> ES:DI (CX has byte count)	       			 

copynow:
	push	cx		; We will want the count and pointer
	push	es		;  to hand to client after copying,
	push	di		;  so save them at this point

	mov	DX, CS:io_addr
	add	DX, RACK		; DX has address NE/2 Port window (?)

	mov	si, cx
	shr	cx, 1
	rep	insw
	test	si, 1
	jz	call_rc
	in	ax, dx
	stosb
call_rc:
	pop	si			; Recover pointer to destination
	pop	ds			; Tell client it's his source
	pop	cx			; And it's this long
	assume	ds:nothing
	call	recv_copy		; Give it to him
	jmp	short rcv_ret

; no system buff availble to hold rx frm
no_buff:
rcv_ret:
	push	cs		; Put ds back in code space
	pop	ds		; ..
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
usage_msg	db	"usage: NE2 <packet_int_no>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for NE/2 version "
		db	'0'+majver,".",'0'+version,CR,LF
		db	"Portions Copyright 1990, Eric Henderson, BYU",CR,LF,'$'
int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'
no_board_msg:
	db	"NE/2 apparently not present at this IO address.",CR,LF,'$'
HardwareFailure:
	db	"The NE/2 is not responding.",CR,LF,'$'
using_186_msg	db	"Using 80[123]86 I/O instructions.",CR,LF,'$'

	extrn	set_recv_isr: near


;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
;	extrn	decout: near
;	extrn	hexout: near

	public	parse_args
print_args:
	mov	dx,offset int_no_name
	mov	ah,9
	int	21h
	mov	di,offset int_no
	mov	ax,[di]			;print the number in hex.
	mov	dx,[di+2]
;	call	hexout
	mov	dl,' '
	mov	ah, 2
	int	21h
	mov	dl,'('
	mov	ah, 2
	int	21h
	mov	ax,[di]			;print the number in decimal.
	mov	dx,[di+2]
;	call	decout
	mov	dl,')'
	mov	ah, 2
	int	21h
	mov	dl,CR
	mov	ah, 2
	int	21h
	mov	dl,LF
	mov	ah, 2
	int	21h
	mov	dx,offset io_addr_name
	mov	ah,9
	int	21h
	mov	di,offset io_addr
	mov	ax,[di]			;print the number in hex.
	mov	dx,[di+2]
;	call	hexout
	mov	dl,' '
	mov	ah, 2
	int	21h
	mov	dl,'('
	mov	ah, 2
	int	21h
	mov	ax,[di]			;print the number in decimal.
	mov	dx,[di+2]
;	call	decout
	mov	dl,')'
	mov	ah, 2
	int	21h
	mov	dl,CR
	mov	ah, 2
	int	21h
	mov	dl,LF
	mov	ah, 2
	int	21h
parse_args:
	ret

	extrn	etopen_diagn: byte

BoardNotResponding:
	mov	dx, offset HardwareFailure
	mov	etopen_diagn, 35
	jmp	short error_wrt
bad_cksum:
no_memory:
	mov	dx,offset no_board_msg
	mov	etopen_diagn,37
error_wrt:
	mov	ah,9
	int	21h
	stc
	ret

SlotSelectRegister		equ	96h
POS0				equ	100h
POS1				equ	101h
POS2				equ	102h
NE2ID				equ	7154h

IRQConfiguration		label	byte	;possible IRQ settings
				db	3
				db	4
				db	5
				db	9

IOConfiguration  		label	word	;possible I/O Base addresses
				dw	1000h
				dw	2020h
				dw	8020h
				dw	0A0A0h
				dw	0B0B0h
				dw	0C0C0h
				dw	0C3D0h

	public	etopen
etopen:
;if all is okay,
	cli					;Find the NE/2 NIC
	mov 	cl, 7				; channel position (8=1st slot
						; F=8th slot)
NextSlot:
	inc	cl
	cmp	cl,10h				;check 8 I/O slots
	jne	NoHardwareFailure
	jmp	BoardNotResponding
NoHardwareFailure:
	mov	al, cl
	out	SlotSelectRegister, al		;select card slot
	mov 	dx, POS1
	in	al, dx				;check for NE2 ID
	mov	ah, al
	mov	dx, POS0
	in	al, dx
	cmp	ax, NE2ID
	jne	NextSlot

	mov	dx, POS2			;read configuration port
	in	al, dx					

	test	al, 01				;test to see if card enable
	jz	NextSlot			; bit is set

	mov	bl, al
	and	bx, 000Eh			;mask all but I/O base addr.

	jz	NextSlot			;check next slot if no option
						; setting in POS 2
	dec	bx
	dec	bx
	mov	dx, IOConfiguration[bx]		; IO address
	mov	io_addr, dx			;save the base I/O address

	test	al, 10h				;if boot prom enabled then

SetIRQ:
	shr	al, 5				;load the hardware config.
	and	ax, 3				; table with the IRQ line
	mov	bx, ax				; value and string
	mov	dl, IRQConfiguration[bx]
	mov	int_no, dl			;save interrupt level
;	call	print_args

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

	call	set_recv_isr	; Put ourselves in interrupt chain

	; reset NE/2 board 
	cli
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET
	wr_NE	NERESET

	rd_NE	NERESET

	mov	al, 21h
	wr_NE	CMDR

	; Test to see if we're OK
	rd_NE	CMDR
	cmp	al, 21h
	je	WeBeOK
	jmp	BoardNotResponding

WeBeOK:
	mov	al, 49h			;Word mode
	wr_NE	DCR		
	mov	al, 0
	wr_NE	RBCR0
	wr_NE	RBCR1
	mov	al, 4
	wr_NE	RCRWD
	mov	al, 02
	wr_NE	TCR		
	mov	al, START_PG
	wr_NE	PSTART		
	wr_NE	BNRY		
	mov	al, STOP_PG
	wr_NE	PSTOP		

	mov	al, 0ffh
	wr_NE	ISR		
	mov	al, 01fh
	wr_NE	IMR

	mov	al, 61h
	wr_NE	CMDR		
	mov	al, START_PG + 1
	wr_NE	CURR
	mov	al, 21h
	wr_NE	CMDR		



	mov	DX, CS:io_addr
	add	DX, RACK		; DX has address NE/2 Port window 

	mov 	bx, -1				;setup for Nic's memory test

	mov	bp, CS:io_addr
  	call	RAMTest  			;16 bit memory test

    	jz	memory_OK			;if error, report it
	jmp	no_memory

memory_OK:
	; set up my_eaddr from addr ROM 
	mov	al, 12			;read 6 bytes from PROM (word mode=12)
	wr_NE	RBCR0
	mov	al, 0
	wr_NE	RBCR1
	wr_NE	RSAR0
	wr_NE	RSAR1
	mov	al, MSK_DMA_RD
	wr_NE	CMDR

	mov	DX, CS:io_addr
	add	DX, RACK		; DX has address NE/2 Port window (?)
	mov	di, OFFSET my_eaddr
	mov	ax, cs
	mov	es, ax
	mov	cx, 6
GetEnetAddress:
	in	al, dx
	stosb
	loop	GetEnetAddress

ModeCheckDone:
	mov	ax, cs
	mov	ds, ax
	mov	si, OFFSET my_eaddr
	mov	cx, 6
	call	set_address

	mov 	al, 21h
	wr_NE 	CMDR

SetTXPage:
	mov     al,0C0h                		;C000h is used as TX
        wr_NE	TPSR				;page because of
        					;spurious writes to 
						;Ram during transmits
						;when collisions occur
	mov	al, 22h
	jmp	$+2
	wr_NE	CMDR	

	mov	al, 0
	wr_NE	TCR

	mov	al,0				;deselect all I/O channels
	out	SlotSelectRegister,al

	mov	dx,offset end_resident
	sti
	clc
	ret

;**********************************************************************
;
;	RAM Test
;
;	assumes:
;		BX	has the test value
;		BP	points to IOBase
;		NIC (8390) has been set to ignore activity on wire
;		  (loopback mode) and is set to page 0
;
;	returns:
;		Z flag set if no error
;		BP	preserved.
;		NIC set to page 0
;
;**********************************************************************

RAMTest	proc	near

	lea	dx, [bp].RemoteStartAddress0	;point at start of RAM
	xor	al, al
	out	dx, al
        inc     dx
	mov	al, 40h
	out	dx, al

	inc	dx				;set byte count=16k
	xor	al, al				; (both 8k RAM chips)
	out	dx, al
	inc	dx
	mov	al, 40h
	out	dx, al

	lea	dx, [bp].Command		;write in all RAM locations
	mov	al, MSK_DMA_WR			; what we send
	out	dx, al

	lea	dx, [bp].DataPort		;point at Nic's data port

	mov	cx, 2000h			;set count to 8k words

	mov	ax, bx				;read in test value

WriteLoop:
	inc	ax				;adjust test pattern
	out	dx, ax				;send it out to Nic
	loop	WriteLoop

						;now read back and compare
						; test pattern from NIC

	lea	dx, [bp].RemoteStartAddress0	;point at start of RAM
	xor	al, al
	out	dx, al
        inc     dx
	mov	al, 40h
	out	dx, al

	inc	dx				;set byte count = 16k
	xor	al, al
	out	dx, al
	inc	dx
	mov	al, 40h
	out	dx, al

	lea	dx, [bp].Command		;tell NIC to send back test
	mov	al, MSK_DMA_RD			; pattern
	out	dx, al

	lea	dx, [bp].DataPort		;point at Nic's data port

	mov	cx, 2000h			;setup loop

ReadLoop:
	in	ax, dx				;read in test pattern and
	inc	bx				; compare with original
	cmp	bx, ax

	loopz	ReadLoop

	ret					;zero flag set if memory error

RAMTest	endp

	public	print_parameters
print_parameters:
;echo our command-line parameters
	ret


code	ends

	end
