;History:617,1

dp8390_version	equ	0	;version number of the generic 8390 driver.

;  Russell Nelson, Clarkson University.
;  Copyright, 1988-1991, Russell Nelson
;  The following people have contributed to this code: David Horne, Eric
;  Henderson, and Bob Clements.

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

; This driver is the work of several people: Bob Clements, Eric Henderson,
; Dave Horne, and Russell Nelson.


sm_rstop_ptr	db	SM_RSTOP_PG

rxcr_bits       db      ENRXCR_BCST     ; Default to ours plus multicast


	public	curr_hw_addr, mcast_list_bits, mcast_all_flag
curr_hw_addr	db	0,0,0,0,0,0	;Address set into the 8390
mcast_list_bits db      0,0,0,0,0,0,0,0 ;Bit mask from last set_multicast_list
mcast_all_flag  db      0               ;Non-zero if hware should have all
					; ones in mask rather than this list.
mcast_sw_filter	db	0		; set if software filter is required.
mcast_sw_fin	dw	0
mcast_sw_fout	dw	0

	public	rcv_modes
rcv_modes	dw	7		;number of receive modes in our table.
		dw	0               ;There is no mode zero
		dw	rcv_mode_1
		dw	rcv_mode_2
		dw	rcv_mode_3
		dw	rcv_mode_4
		dw	rcv_mode_5
		dw	rcv_mode_6

	public	mcast_tab
mcast_hcount	dw	0		; multicast header count
mcast_tab_b	db	0ffh,0ffh,0ffh,0ffh,0ffh,0ffh ; entry for broadcast
mcast_tab	db	(MAX_MULTICAST*EADDR_LEN) dup (0)
;
;	a temp buffer for the received header
;
RCV_HDR_SIZE	equ	26		; 2 ids @6 + protocol @2+8, + header @4
rcv_hdr		db	RCV_HDR_SIZE dup(0)

;
;	The board data
;
		public	board_data
BOARD_DATA_SIZE equ	32
board_data	db 	BOARD_DATA_SIZE dup(0)
soft_tx_errors		dw	0,0
soft_tx_err_bits	db	0
soft_rx_errors		dw	0,0
soft_rx_err_bits	db	0



ifdef	debug			; Include a very useful logging mechanism.  

; The log entry structure.  Log entries include useful data such as
; a type (each place a log entry is made uses a different type), various
; chip status, ring buffer status, log entry dependent data, and optionally
; 8259 interrupt controller status.
logentry	struc
le_type		db	0	; Log entry type
le_ccmd		db	?	; Value of CCMD register
le_isr		db	?	; Value of ISR register
le_tsr		db	?	; Value of TSR register
le_tcur		dw	?	; Value of sm_tcur
le_tboundary	dw	?	; Value of sm_tboundary
le_tnum		dw	?	; Value of sm_tnum
le_dw		dw	?	; Log type specific dw data
ifndef	mkle8259		; Log 8259 status?
le_dd		dd	?	; Log type specific dd data
else
le_irr1		db	?	; Value of 8259-1 IRR register
le_isr1		db	?	; Value of 8259-1 ISR register
le_irr2		db	?	; Value of 8259-2 IRR register
le_isr2		db	?	; Value of 8259-2 ISR register
endif
logentry	ends

; The types of log entries.
LE_SP_E		equ	0	; send_pkt entry
LE_SP_X		equ	1	; send_pkt exit
LE_ASP_E	equ	2	; as_send_pkt entry
LE_ASP_X	equ	3	; as_send_pkt exit
LE_RBALLOC_E	equ	4	; tx_rballoc entry
LE_RBALLOC_X	equ	5	; tx_rballoc exit
LE_COPY_E	equ	6	; sm_copy entry
LE_COPY_X	equ	7	; sm_copy exit
LE_START_E	equ	8	; tx_start entry
LE_START_X	equ	9	; tx_start exit
LE_XMIT_E	equ	0ah	; xmit entry
LE_XMIT_X	equ	0bh	; xmit exit
LE_TXISR_E	equ	0ch	; txisr entry
LE_TXISR_X	equ	0dh	; txisr exit
LE_RECV_E	equ	0eh	; recv entry
LE_RECV_X	equ	0fh	; recv exit
LE_RCVFRM_E	equ	10h	; rcv_frm entry
LE_RCVFRM_X	equ	11h	; rcv_frm exit
LE_COPY_L	equ	12h	; sm_copy loop
LE_TIMER_E	equ	13h	; timer entry
LE_TIMER_X	equ	14h	; timer exit

	public	log, log_index
log		logentry 256 dup (<>) ; The log itself
log_index	db	0	; Index to current log entry

; The macro used to create log entries.
mkle	macro	letype, ledw, ledd, ledd2 ; Make an entry in the log
	pushf			; Save interrupt enable state
	cli			; Disable interrupts
	push	dx		; Save registers
	push	bx
	push	ax
	mov bl,	log_index	; Get current log_index
	xor bh,	bh		; Clear high byte
	shl bx,	1		; Multiply by sixteen
	shl bx,	1
	shl bx,	1
	shl bx,	1
	mov log[bx].le_type, letype ; Store log entry type
	loadport		; Base of device
	setport EN_CCMD	; Point at chip command register
	in al,	dx		; Get chip command state
	mov log[bx].le_ccmd, al	; Store CCMD value
	setport EN0_ISR		; Point at chip command register
	in al,	dx		; Get chip command state
	mov log[bx].le_isr, al	; Store ISR value
	setport EN0_TSR		; Point at chip command register
	in al,	dx		; Get chip command state
	mov log[bx].le_tsr, al	; Store TSR value
	mov ax,	sm_tcur		; Get current sm_tcur
	mov log[bx].le_tcur, ax	; Store sm_tcur value
	mov ax,	sm_tboundary	; Get current sm_tboundary
	mov log[bx].le_tboundary, ax ; Store sm_tboundary value
	mov ax,	sm_tnum		; Get current sm_tnum
	mov log[bx].le_tnum, ax	; Store sm_tnum value
	mov log[bx].le_dw, ledw	; Store log entry dw
ifndef	mkle8259		; Include extra per-type data
	mov word ptr log[bx].le_dd, ledd ; Store low word of log entry dd
	mov word ptr log[bx].le_dd+2, ledd2 ; Store high word of log entry dd
else				; Include 8259 status
	mov	al,0ah		; read request register from
	out	0a0h,al		; secondary 8259
	nop			; settling delay
	nop
	nop
	in	al,0a0h		; get it
	mov log[bx].le_irr2, al
	mov	al,0bh		; read in-service register from
	out	0a0h,al		; secondary 8259
	nop			; settling delay
	nop
	nop
	in	al,0a0h		; get it
	mov log[bx].le_isr2, al
	mov	al,0ah		; read request register from
	out	020h,al		; primary 8259
	nop			; settling delay
	nop
	nop
	in	al,020h		; get it
	mov log[bx].le_irr1, al
	mov	al,0bh		; read in-service register from
	out	020h,al		; primary 8259
	nop			; settling delay
	nop
	nop
	in	al,020h		; get it
	mov log[bx].le_isr1, al
endif
ifdef	screenlog		; Log the entry type to the screen too
	push	es
	mov ax,	0b800h		; Color screen only...
	mov es,	ax
	mov bl,	log_index	; Get current log_index
	xor bh,	bh		; Clear high byte
	shl bx,	1		; Multiply by sixteen
	add bx,	3360
	mov byte ptr es:[bx-1], 07h
	mov byte ptr es:[bx], letype+30h
	mov byte ptr es:[bx+1], 70h
	pop	es
endif
	inc	log_index	;
	pop	ax		; Restore registers
	pop	bx
	pop	dx
	popf			; Restore interrupt enable state
	endm

else
mkle	macro	letype, ledw, ledd, ledd2 ; Define an empty macro
	endm
endif

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
	mkle LE_SP_E, cx, si, ds
;ne1000 checks the packet size at this point, which is probably more sensible.
	loadport		; Point at chip command register
	setport EN_CCMD		; ..
	pause_
;ne1000 fails to check to see if the transmitter is still busy.
	mov bx,	8000h		; Avoid infinite loop
tx_wait:
	in al,	dx		; Get chip command state
	test al,ENC_TRANS	; Is transmitter still running?
	jz	tx_idle		; Go if free
	dec	bx		; Count the timeout
	jnz	tx_wait		; Fall thru if TX is stuck
	call	count_out_err	; Should count these error timeouts
				; Maybe need to add recovery logic here
tx_idle:
	cmp	cx,GIANT	; Is this packet too large?
	ja	send_pkt_toobig

	cmp cx,	RUNT		; Is the frame long enough?
	jnb	tx_oklen	; Go if OK
	mov cx,	RUNT		; Stretch frame to minimum allowed
tx_oklen:
	push	cx		; Hold count for later
	loadport		; Set up for address
	setport EN0_ISR
	pause_
	mov	al,ENISR_RDC	; clear remote interrupt int.
	out	dx,al
	setport	EN0_TCNTLO	; Low byte of TX count
	pause_
	mov al,	cl		; Get the count
	out dx,	al		; Tell card the count
	setport	EN0_TCNTHI	; High byte of TX count
	pause_
	mov al,	ch		; Get the count
	out dx,	al		; Tell card the count
	xor ax,	ax		; Set up ax at base of tx buffer
	mov ah,	SM_TSTART_PG	; Where to put tx frame
	pop	cx		; Get back count to give to board
	call	block_output
	jc	tx_no_rdc
	loadport
	setport	EN0_TPSR	; Transmit Page Start Register
	pause_
	mov al,	SM_TSTART_PG
	out dx,	al		; Start the transmitter
	setport	EN_CCMD		; Chip command reg
	pause_
	mov al,	ENC_TRANS+ENC_NODMA+ENC_START
	out dx,	al		; Start the transmitter
	mkle LE_SP_X, cx, 1, 0
	clc			; Successfully started
	sti
	ret			; End of transmit-start routine
send_pkt_toobig:
	mov	dh,NO_SPACE
	stc
	sti
	ret
tx_no_rdc:
	mov	dh,CANT_SEND
	mkle LE_SP_X, cx, 0, 0
	stc
	sti
	ret

count_soft_err:
	add	word ptr soft_tx_errors,1
	adc	word ptr soft_tx_errors+2,0
	or	byte ptr soft_tx_err_bits,al
	ret


	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
; Give caller the one currently in the 8390, not necessarily the one in PROM.
	assume ds:code
	cmp cx,	EADDR_LEN	; Caller wants a reasonable length?
	jb	get_addr_x	; No, fail.
	mov cx,	EADDR_LEN	; Move one ethernet address from our copy
	mov si, offset curr_hw_addr     ; Copy from most recent setting
	rep     movsb
	mov cx,	EADDR_LEN	; Tell caller how many bytes we fed him
	clc			; Carry off says success
	ret
get_addr_x:
	stc			; Tell caller our addr is too big for him
	ret


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
	push    cs              ; Copy from them to our RAM copy
	pop     es              ; Destination of move
	mov di, offset curr_hw_addr
	rep     movsb           ; Move their address
	call    set_8390_eaddr  ; Put that address in the chip
set_address_okay:
	mov	cx,EADDR_LEN		;return their address length.
	clc
set_address_done:
	push	cs
	pop	ds
	assume	ds:code
	ret

; Copy our Ethernet address from curr_hw_addr into the DS8390
set_8390_eaddr:
	cld
	push    cs              ; Get it from our local RAM copy
	pop     ds
	mov si, offset curr_hw_addr
	mov cx,	EADDR_LEN	; Move one ethernet address from our copy
	loadport
	setport	EN_CCMD		; Chip command register
	pause_
	cli			; Protect from irq changing page bits
	mov al,	ENC_NODMA+ENC_PAGE1	;+ENC_STOP
	out dx,	al		; Switch to page one for writing eaddr
	setport	EN1_PHYS	; Where it goes in 8390
set_8390_1:
	lodsb
	out	dx,al
	pause_
	inc	dx
	loop	set_8390_1
	loadport
	setport	EN_CCMD		; Chip command register
	pause_
	mov al,	ENC_NODMA+ENC_PAGE0	;+ENC_STOP
	out dx,	al		; Restore to page zero
	sti			; OK for interrupts now
	ret

; Routines to set address filtering modes in the DS8390
rcv_mode_1:     ; Turn off receiver
	mov al,	ENRXCR_MON      ; Set to monitor for counts but accept none
	jmp short rcv_mode_set
rcv_mode_2:     ; Receive only packets to this interface
	mov al, 0               ; Set for only our packets
	jmp short rcv_mode_set
rcv_mode_3:     ; Mode 2 plus broadcast packets (This is the default)
	mov al,	ENRXCR_BCST     ; Set four ours plus broadcasts
	jmp short rcv_mode_set
rcv_mode_4:     ; Mode 3 plus selected multicast packets
	mov al,	ENRXCR_BCST+ENRXCR_MULTI ; Ours, bcst, and filtered multicasts
	mov     mcast_all_flag,0	; need to do sw filter.
	mov	mcast_sw_filter,1	; because chip filter is not 100%
	jmp short rcv_mode_set
rcv_mode_5:     ; Mode 3 plus ALL multicast packets
	mov al,	ENRXCR_BCST+ENRXCR_MULTI ; Ours, bcst, and filtered multicasts
	mov     mcast_all_flag,1
	jmp short rcv_mode_set
rcv_mode_6:     ; Receive all packets (Promiscuous physical plus all multi)
	mov al,	ENRXCR_BCST+ENRXCR_MULTI+ENRXCR_PROMP
	mov     mcast_all_flag,1
rcv_mode_set:
	push    ax              ; Hold mode until masks are right
	call    set_8390_multi  ; Set the multicast mask bits in chip
	pop     ax
	loadport
	setport	EN0_RXCR	; Set receiver to selected mode
	pause_
	out dx,	al
	mov     rxcr_bits,al    ; Save a copy of what we set it to
	ret


	public	set_multicast_list
set_multicast_list:
;enter with ds:si ->list of multicast addresses, cx = number of addresses.
;return nc if we set all of them, or cy,dh=error if we didn't.
	assume ds:nothing
	push	cs
	pop	es		; set es to destination
	mov	di,offset mcast_tab
	mov	ax,cx		; save byte count
	repz	movsb
	mov	dx,0
	mov	cx,6
	div	cx
	mov	mcast_hcount,ax
;
	mov	word ptr mcast_list_bits,0
	mov	word ptr mcast_list_bits+2,0
	mov	word ptr mcast_list_bits+4,0
	mov	word ptr mcast_list_bits+6,0
;
	mov	cx,mcast_hcount
	inc	cx
	mov	di,offset mcast_tab_b
set_mcl_1:
	call	add_mc_bits
	add	di,6
	loop	set_mcl_1
	call    set_8390_multi  ; Set the multicast mask bits in chip
	clc
	mov	dh,0
	ret

;
;	multicast is at es:di
	assume	ds:nothing
add_mc_bits:
	push	cx
	push	di
	mov	cx,6
	mov	dx,0ffffh			; this is msw.
	mov	bx,0ffffh			; set 32 bit number
add_mcb_1:
	mov	al,es:[di]
	inc	di
	call	upd_crc			; update crc
	loop	add_mcb_1		; and loop.
	mov	ah,0
	mov	al,dh			; get ms 8 bits,
	rol	al,1
	rol	al,1
	rol	al,1			; put 3 bits at bottom
	and	al,7
	mov	dl,al			; save in dl
	mov	al,dh			; get ms 8 bits,
	ror	al,1
	ror	al,1			; but at bottom
	and	al,7
	mov	cl,al			; save in cl
	mov	al,1
	rol	al,cl			; set the correct bit,
	mov	di,offset mcast_list_bits
	mov	dh,0
	add	di,dx
	or	cs:[di],al
	pop	di
	pop	cx
	ret

;
;	dx is high,
;	bx is low.
;	al is data

upd_crc:
	push	cx
	mov	cx,8		; do 8 bits
	mov	ah,0
upd_crc1:
	shl	bx,1		; shift bx
	rcl	dx,1		; through dx
	rcl	ah,1		; carry is at bottom of ah
	xor	ah,al		; xor with lsb of data
	rcr	ah,1		; and put in carry bit
	jnc	upd_crc2
;
;	autodin is x^32+x^26+x^23x^22+x^16+
;	x^12+x^11+x^10+x^8+x^7+x^5+x^4+x^2+x^1+1

	xor	dx,0000010011000001b
	xor	bx,0001110110110111b
upd_crc2:
	shr	al,1		; shift the data
	loop	upd_crc1
	pop	cx
	ret

; Set the multicast filter mask bits in case promiscuous rcv wanted
set_8390_multi:
	push    cs
	pop     ds
	assume	ds:code
	loadport
	setport	EN_CCMD		; Chip command register
	pause_
	mov cx,	8		; Eight bytes of multicast filter
	mov si, offset mcast_list_bits  ; Where bits are, if not all ones
	cli			; Protect from irq changing page bits
	mov al,	ENC_NODMA+ENC_PAGE1+ENC_STOP
	out dx,	al		; Switch to page one for writing eaddr
	setport	EN1_MULT	; Where it goes in 8390
	pause_
	mov al, mcast_all_flag  ; Want all ones or just selected bits?
	or al,  al
	je      set_mcast_2     ; Just selected ones
	mov al,	0ffh		; Ones for filter
set_mcast_all:
	out dx,	al		; Write a mask byte
	inc	dl		; Step to next one
	loop	set_mcast_all	; ..
	jmp short set_mcast_x

set_mcast_2:
	lodsb                   ; Get a byte of mask bits
	out dx,	al		; Write a mask byte
	inc	dl		; Step to next I/O register
	loop	set_mcast_2 	; ..
set_mcast_x:
	loadport
	setport	EN_CCMD		; Chip command register
	pause_
	mov al,	ENC_NODMA+ENC_PAGE0+ENC_START
	out dx,	al		; Restore to page zero
	sti			; OK for interrupts now
	ret


	public	reset_board
reset_board:
	assume ds:nothing
	reset_8390
	setport	EN_CCMD		; Chip command reg
	pause_
	mov al,	ENC_STOP+ENC_NODMA
	out dx,	al		; Stop the DS8390
	setport EN0_ISR
	pause_
	mov	ax,18			;half a second ought be to enuf.
	call	set_timeout
reset_board_loop:
	in	al,dx		; get isr
	and	al,ENISR_RESET
	jnz	reset_board_done
	call	do_timeout
	jne	reset_board_loop
reset_board_done:
	ret

	public	terminate
terminate:
	terminate_board
	ret

	public	reset_interface
reset_interface:
	assume ds:code
	call	reset_board
	loadport		; Base of I/O regs
	setport	EN0_ISR		; Interrupt status reg
	pause_
	mov al,	0ffh		; Clear all pending interrupts
	out dx,	al		; ..
	setport	EN0_IMR		; Interrupt mask reg
	pause_
	xor al,	al		; Turn off all enables
	out dx,	al		; ..
	ret

; Linkages to non-device-specific routines
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
;ne1000 and ne2000 routines are identical to this point (except that ne2000
;  masks off interrupts).
	assume	ds:code
	mkle LE_RECV_E, 0, 0, 0

check_isr:			; Was there an interrupt from this card?
	loadport		; Point at card's I/O port base
	ram_enable
	setport EN0_IMR		; point at interrupt masks
	pause_			; switch off, this way we can
	mov	al,0		; leave the chip running.
	out	dx,al		; no interrupts please.
	setport	EN0_ISR		; Point at interrupt status register
	pause_
	in al,	dx		; Get pending interrupts
	and al,	ENISR_ALL	; Any?
	jnz	isr_test_overrun
	mkle LE_RECV_X, 0, 0, 0
	jmp	interrupt_done	; Go if none
; First, a messy procedure for handling the case where the rcvr
; over-runs its ring buffer.  This is spec'ed by National for the chip.
; This is handled differently in sample code from 3Com and from WD.
; This is close to the WD version.  May need tweaking if it doesn't
; work for the 3Com card.

isr_test_overrun: 
	test al,ENISR_OVER	; Was there an overrun?
	jnz	recv_overrun	; Go if so.
	jmp	recv_no_overrun	; Go if not.
recv_overrun:
	setport	EN_CCMD		; Stop the chip
	pause_
	mov al,	ENC_STOP+ENC_NODMA
	out dx,	al		; Write "stop" to command register

	mov al, ENC_NODMA+ENC_PAGE1	; Could be in previous out, but
	out dx,al		; was only tested this way
	setport EN1_CURPAG	; Get current page
	in al,dx
	mov bl,al		; save it
	setport	EN_CCMD		;
	mov al, ENC_NODMA+ENC_PAGE0
	out dx,al		; Back to page 0

; Remove one frame from the ring
	setport	EN0_BOUNDARY	; Find end of this frame
	pause_
	in al,	dx		; Get memory page number
	inc	al		; Page plus 1
	cmp al,	sm_rstop_ptr	; Wrapped around ring?
	jnz	rcv_ovr_nwrap	; Go if not
	mov al,	SM_RSTART_PG	; Yes, wrap the page pointer
rcv_ovr_nwrap:

	cmp	al,bl		; Check if buffer emptry
	je	rcv_ovr_empty	; Yes ? Don't receive anything

;ne1000 and ne2000 routines are identical to this point (except that ne2000
;  masks off interrupts).
	mov	ah,al		; make a byte address. e.g. page
	mov	bl,ah		; and save in bl
	mov	al,0		; 46h becomes 4600h into buffer
	mov	cx,RCV_HDR_SIZE	; size of rcv_hdr
	mov	di,offset rcv_hdr ;point to header
	push	ds
	pop	es		; set es to right place
	call	block_input
	mov al,	rcv_hdr+EN_RBUF_STAT	; Get the buffer status byte
	test al,ENRSR_RXOK	; Is this frame any good?
	jz	rcv_ovr_ng	; Skip if not
 	call	rcv_frm		; Yes, go accept it
rcv_ovr_ng:
	mov	al,rcv_hdr+EN_RBUF_NXT_PG ; Get pointer to next frame
	dec	al		; Back up one page
	cmp	al,SM_RSTART_PG	; Did it wrap?
	jae	rcv_ovr_nwr2
	mov	al,sm_rstop_ptr	; Yes, back to end of ring
	dec	al
rcv_ovr_nwr2:
	loadport		; Point at boundary reg
	setport	EN0_BOUNDARY	; ..
	pause_
	out dx,	al		; Set the boundary
rcv_ovr_empty:
	loadport		; Point at boundary reg
	setport	EN0_RCNTLO	; Point at byte count regs
	pause_
	xor al,	al		; Clear them
	out dx,	al		; ..
	setport	EN0_RCNTHI
	pause_
	out dx,	al
	setport	EN0_ISR		; Point at status reg
	pause_
	mov cx,	8000h		; Timeout counter
rcv_ovr_rst_loop:
	in al,	dx		; Is it finished resetting?
	test al,ENISR_RESET	; ..
	jmp	$+2		; limit chip access rate
	loopnz	rcv_ovr_rst_loop; Loop til reset, or til timeout
	loadport		; Point at Transmit control reg
 	setport	EN0_TXCR	; ..
	pause_
	mov al,	ENTXCR_LOOP	; Put transmitter in loopback mode
	out dx,	al		; ..
	setport	EN_CCMD		; Point at Chip command reg
	pause_
	mov al,	ENC_START+ENC_NODMA
	out dx,	al		; Start the chip running again
	setport	EN0_TXCR	; Back to TX control reg
	pause_
	xor al,	al		; Clear the loopback bit
	out dx,	al		; ..
	setport	EN0_ISR		; Point at Interrupt status register
	pause_
	mov al,	ENISR_OVER	; Clear the overrun interrupt bit
	out dx,	al		; ..
	call	count_in_err	; Count the anomaly
 	jmp	check_isr	; Done with the overrun case

recv_no_overrun:
; Handle receive flags, normal and with error (but not overrun).
	test al,ENISR_RX+ENISR_RX_ERR	; Frame received without overrun?
	jnz	recv_frame	; Go if so.
	jmp	recv_no_frame	; Go if not.
recv_frame:
	loadport		; Point at Chip's Command Reg
	setport	EN0_ISR		; Point at Interrupt status register
	pause_
	mov al,	ENISR_RX+ENISR_RX_ERR
	out dx,	al		; Clear those requests
 	setport	EN_CCMD		; ..
	pause_
	mov al,	ENC_NODMA+ENC_PAGE1+ENC_START
	out dx,	al		; Switch to page 1 registers
	setport	EN1_CURPAG	;Get current page of rcv ring
	pause_
	in al,	dx		; ..
	mov ah,	al		; Hold current page in AH
 	setport	EN_CCMD		; Back to page zero registers
	pause_
	mov al,	ENC_NODMA+ENC_PAGE0+ENC_START
	out dx,	al		; Switch back to page 0 registers
	setport	EN0_BOUNDARY	;Get boundary page
	pause_
	in al,	dx		; ..
	inc	al		; Step boundary from last used page
	cmp al,	sm_rstop_ptr	; Wrap if needed
	jne	rx_nwrap3	; Go if not
	mov al,	SM_RSTART_PG	; Wrap to first RX page
rx_nwrap3:
	cmp al,	ah		; Read all the frames?
	je	recv_frame_break	; Finished them all

	mov	ah,al		; make a byte address. E.G. page
	mov	al,0		; 46h becomes 4600h into buffer
	mov	bl,ah
	mov	cx,RCV_HDR_SIZE
	mov	di,offset rcv_hdr
	push	ds
	pop	es		; set es to right place
	call	block_input
	mov al,	rcv_hdr+EN_RBUF_STAT	; Get the buffer status byte
	test al,ENRSR_RXOK	; Good frame?
	jz	recv_err_no_rcv
	call	rcv_frm		; Yes, go accept it
	jmp	recv_no_rcv
recv_err_no_rcv:
	or	byte ptr soft_rx_err_bits,al
	add	word ptr soft_rx_errors,1
	adc	word ptr soft_rx_errors+2,0
recv_no_rcv:
	mov al,	rcv_hdr+EN_RBUF_NXT_PG	; Start of next frame
	dec	al		; Make previous page for new boundary
	cmp al,	SM_RSTART_PG	; Wrap around the bottom?
	jge	rcv_nwrap4
	mov al,	sm_rstop_ptr	; Yes
	dec al
rcv_nwrap4:
	loadport		; Point at the Boundary Reg again
 	setport	EN0_BOUNDARY	; ..
	pause_
	out dx,	al		; Set new boundary
	jmp	recv_frame	; See if any more frames

recv_frame_break:
	loadport		; Point at Command register
 	setport	EN_CCMD		; ..
	pause_
	mov al,	ENC_NODMA+ENC_PAGE0+ENC_START
	out	dx,al
	jmp	check_isr	; See if any other interrupts pending

recv_no_frame:				; Handle transmit flags.
	test al,ENISR_TX+ENISR_TX_ERR	; Frame transmitted?
	jnz	isr_tx		; Go if so.
	jmp	isr_no_tx	; Go if not.
isr_tx:
	mov ah,	al		; Hold interrupt status bits
	loadport		; Point at Transmit Status Reg
 	setport	EN0_TSR		; ..
	pause_
	in al,	dx		; ..
	test ah,ENISR_TX	; Non-error TX?
	jz	isr_tx_err	; No, do TX error completion
	call	count_soft_err	; soft error ??
	test al,ENTSR_COLL16	; Jammed for 16 transmit tries?
	jz	isr_tx_njam	; Go if not
	call	count_out_err	; Yes, count those
isr_tx_njam:
	setport	EN0_ISR		; Clear the TX complete flag
	pause_
	mov al,	ENISR_TX	; ..
	out dx,	al		; ..
	jmp	isr_tx_done
isr_tx_err:
	test al,ENTSR_FU	; FIFO Underrun?
	jz	isr_txerr_nfu
	call	count_out_err	; Yes, count those
isr_txerr_nfu:
	loadport		; Clear the TX error completion flag
	setport	EN0_ISR		; ..
	pause_
	mov al,	ENISR_TX_ERR	; ..
	out dx,	al		; ..
isr_tx_done:
; If TX queue and/or TX shared memory ring buffer were being
; used, logic to step through them would go here.  However,
; in this version, we just clear the flags for background to notice.

 	jmp	check_isr	; See if any other interrupts on

isr_no_tx:
; Now check to see if any counters are getting full
	test al,ENISR_COUNTERS	; Interrupt to handle counters?
	jnz	isr_stat	; Go if so.
	jmp	isr_no_stat	; Go if not.
isr_stat:
; We have to read the counters to clear them and to clear the interrupt.
; Version 1 of the PC/FTP driver spec doesn't give us
; anything useful to do with the data, though.
; Fix this up for V2 one of these days.
	loadport		; Point at first counter
 	setport	EN0_COUNTER0	; ..
	pause_
	in al,	dx		; Read the count, ignore it.
	setport	EN0_COUNTER1
	pause_
	in al,	dx		; Read the count, ignore it.
	setport	EN0_COUNTER2
	pause_
	in al,	dx		; Read the count, ignore it.
	setport	EN0_ISR		; Clear the statistics completion flag
	pause_
	mov al,	ENISR_COUNTERS	; ..
	out dx,	al		; ..
isr_no_stat:
 	jmp	check_isr	; Anything else to do?

interrupt_done:
	ret

; Do the work of copying out a receive frame.
; Called with bl/ the page number of the frame header in shared memory

	public	rcv_frm
rcv_frm:
	mkle LE_RCVFRM_E, 0, 0, 0

; first do a software multicast filter.
	push	bx			; save page.
	cmp	mcast_sw_filter,1	; do software check of mcast ?
	jnz	rcv_frm_ok		; no, accept.
	mov	ax,word ptr rcv_hdr+EN_RBUF_NHDR ; get first word of address
	test al,1			; odd first byte
	jz	rcv_frm_ok		; must be our address if even
	inc	word ptr mcast_sw_fin

	mov	bx,word ptr rcv_hdr+EN_RBUF_NHDR+2 ; get second word of address
	mov	dx,word ptr rcv_hdr+EN_RBUF_NHDR+4 ; get third word of address

	mov	di,offset mcast_tab_b	; point to table and broadcast
	mov	cx,mcast_hcount		; get number in table
	inc	cx			; plus the broadcast
rcv_loop:
	mov	si,di			; save this table entry
	cmp	ax,[di]
	jnz	rcv_trynext
	inc	di
	inc	di
	cmp	bx,[di]
	jnz	rcv_trynext
	inc	di
	inc	di
	cmp	dx,[di]
	jz	rcv_mc_ok		; got it.
rcv_trynext:
	mov	di,si			; get table back,
	add	di,6
	loop	rcv_loop
	pop	bx			; restore bx
	jmp	rcv_no_copy

rcv_mc_ok:
	inc	word ptr mcast_sw_fout
rcv_frm_ok:
; Set cx to length of this frame.
	mov ch,	rcv_hdr+EN_RBUF_SIZE_HI	; Extract size of frame
	mov cl,	rcv_hdr+EN_RBUF_SIZE_LO	; Extract size of frame
	sub cx,	EN_RBUF_NHDR		; Less the header stuff
; Set es:di to point to Ethernet type field.
	mov di,	offset rcv_hdr+EN_RBUF_NHDR+EADDR_LEN+EADDR_LEN
	push	cx			; Save frame size
	push	es
	mov ax,	cs			; Set ds = code
	mov ds,	ax
	mov es,ax
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

	call	recv_find		; See if type and size are wanted
	pop	ds			; RX page pointer in ds now
	assume	ds:nothing
	pop	cx
	pop	bx
	cld			; Copies below are forward, please
	mov ax,	es		; Did recv_find give us a null pointer?
	or ax,	di		; ..
	je	rcv_no_copy	; If null, don't copy the data

	push	cx		; We will want the count and pointer
	push	es		;  to hand to client after copying,
	push	di		;  so save them at this point
	mov	ah,bl		; set ax to page to start from
	mov	al,EN_RBUF_NHDR	; skip the header stuff
	call	block_input
	pop	si		; Recover pointer to destination
	pop	ds		; Tell client it's his source
	pop	cx		; And it's this long
	assume	ds:nothing
	call	recv_copy	; Give it to him
rcv_no_copy:
	push	cs		; Put ds back in code space
	pop	ds		; ..
	assume	ds:code
	mkle LE_RCVFRM_X, 0, 0, 0
	ret			; That's it for rcv_frm


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	push	dx
	loadport
	setport	EN0_IMR		; Tell card it can cause these interrupts
	pause_
	mov al,	ENISR_ALL
	out dx,	al
	pop	dx
	ret

	include	timeout.asm

;any code after this will not be kept after initialization.
end_resident	label	byte

using_186_msg	db	"Using 80[123]86 I/O instructions.",CR,LF,'$'

;standard EN0_DCFG contents:
endcfg	db	048h			; Set burst mode, 8 deep FIFO

cfg_error:
	mov	dx,offset cfg_err_msg
error:
	mov	ah,9		; Type the msg
	int	21h
	stc			; Indicate error
	ret			; Return to common code

; Called once to initialize the NE2000 card

	public	etopen
etopen:				; Initialize interface

;Determine the processor type.  The 8088 and 8086 will actually shift ax
;over by 33 bits, while the 80[123]86 use a shift count mod 32.
;This bit lifted from NI5010 driver.

	mov	cl,33
	mov	ax,0ffffh
	shl	ax,cl
	jz	not_186
	mov	is_186,1
	mov	dx,offset using_186_msg
	mov	ah,9
	int	21h
not_186:

;Step 1. Reset and stop the 8390.

	call	reset_board

;Step 2. Init the Data Config Reg.

	loadport
	mov	al,endcfg
	setport	EN0_DCFG
	pause_
	out	dx,al

;Step 3. Clear Remote Byte Count Regs.

	mov	al, 0
	setport	EN0_RCNTLO
	pause_
	out	dx,al
	setport	EN0_RCNTHI
	pause_
	out	dx,al

;Step 4. Set receiver to monitor mode

	mov	al, ENRXCR_MON
	setport	EN0_RXCR
	pause_
	out	dx,al

;Step 5. Place NIC into Loopback Mode 1.

	mov	al,ENTXCR_LOOP
	setport	EN0_TXCR
	pause_
	out	dx,al

;Step 6. Do anything special that the card needs.

	call	init_card

;Step 7. Re-init endcfg in case they put it into word mode.

	loadport
	mov	al,endcfg
	setport	EN0_DCFG
	pause_
	out	dx,al

;Step 8. Init EN0_STARTPG to same value as EN0_BOUNDARY

	loadport
	mov	al,SM_RSTART_PG
	setport	EN0_STARTPG
	pause_
	out	dx,al
	mov	al,SM_RSTART_PG
	setport	EN0_BOUNDARY
	pause_
	out	dx,al
	mov	al,sm_rstop_ptr
	setport	EN0_STOPPG
	pause_
	out	dx,al

;Step 9. Write 1's to all bits of EN0_ISR to clear pending interrupts.

	mov	al, 0ffh
	setport	EN0_ISR
	pause_
	out	dx,al

;Step 10. Init EN0_IMR as desired.

	mov	al, ENISR_ALL
	setport	EN0_IMR
	pause_
	out	dx,al

;Step 11. Init the Ethernet address and multicast filters.

	call	set_8390_eaddr  ; Now set the address in the 8390 chip
	call	set_8390_multi  ; Put the right stuff into 8390's multicast masks

;Step 12. Program EN_CCMD for page 1.

	loadport
	mov	al, ENC_PAGE1 + ENC_NODMA + ENC_STOP
	setport	EN_CCMD
	pause_
	out	dx,al

;Step 13. Program the Current Page Register to same value as Boundary Pointer.

	mov	al,SM_RSTART_PG
	setport	EN1_CURPAG
	pause_
	out	dx,al

;Step 14. Program EN_CCMD back to page 0, and start it.

	mov	al, ENC_NODMA + ENC_START
	setport	EN_CCMD
	pause_
	out	dx,al

	mov	al, 0			;set transmitter mode to normal.
	setport	EN0_TXCR
	pause_
	out	dx,al

	call	set_recv_isr	; Put ourselves in interrupt chain

	loadport
	setport	EN0_RXCR	; Tell it what frames to accept
	pause_
	mov al,	rxcr_bits       ; As most recently set by set_mode
	out dx,	al

	ram_enable

	mov	al, int_no		; Get board's interrupt vector
	add	al, 8
	cmp	al, 8+8			; Is it a slave 8259 interrupt?
	jb	set_int_num		; No.
	add	al, 70h - 8 - 8		; Map it to the real interrupt.
set_int_num:
	xor	ah, ah			; Clear high byte
	mov	int_num, ax		; Set parameter_list int num.

	mov dx,	offset end_resident	; Report our size
	clc				; Say no error
	ret				; Back to common code

