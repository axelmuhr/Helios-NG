version	equ	4

	include	defs.asm
	include	8250defs.asm
;
;	Dave Price. October 30th 1990. 12:35.
;	Things much better now. I have had my standard
;	'FTP' test running at 2.5 Kbytes per second.
;	There is still a transmit bug though.. It appears
;	as the transmission of giant packets. I think I know
;	the source of the bug. I believe it occurs when we are
;	transmitting the last few bytes of a packet. Before
;	we change the interrupt mask to expect only TXDONE and
;	TXURUN, the TX fifo drains so that the TX419 condition
;	becomes true. The result is that the 8952 is already
;	waiting to interrupt us with the TX419. My code does not
;	expect anymore of these and acts incorrectly when
;	one arrives!
;
;	Dave Price. October 30th 1990 10:08.
;	After several minor changes the driver was almost
;	working. Some problems still existed however and
;	transferring data with FTP for instance
;	seemed to take a very long time. It appeared that
;	data was being lost, or that the routers or distant
;	host were so busy that packets were being dropped.
;	A further possible cause was that the timers used
;	by the TCP protocol engines were inappropriate or
;	in some sense incompatible. A carefull reading of
;	the data sheets implies that one can use the 'FIFO empty'
;	bits providing you make sure that at least 2 cycles
;	of a 2mHz clock occur after you read data and before you
;	read the interrupt flag register. This only amounts to
;	16 bus cycles on a 16Mhz PC. This is really very few
;	instructions. I have talked to Jeremy Bicknall at MITEL
;	and he seems to agree (I reported a potential design
;	bug with the 8952 generating false RX1519s - he will persue).
;	I have thus decided to use the RXBYTE bits to decide how
;	to process each item of data but use the RXFIFO empty state
;	to cease reading data.
;
;
;	Dave Price. October 24th 1990 11:10.
;	More changes again. The main idea now is 
;	to only have two states in the RX protocol engine.
;	It is either 'building' a frame or 'skipping'
;	to the next one. The actual interrupts will just
;	be used to indicate the point at which you should
;	stop processing the RXFIFO. There will be several
;	items to help. A minimum numbers of bytes to read,
;	a maximum number of bytes to read and a stop condition.
;	Processing the FIFO will cease when either the maximum
;	number of bytes have been processed, or BOTH the stop
;	condition and the minimum number have been processed.
;	On considering a new item of data a mask will be built
;	containing 5 bits that reflect a condition implied
;	by the data. Four bits are used to simply indicate
;	a packet byte, first byte, good last byte or bad last
;	byte. The fifth bit is used to indicate a frame abort
;	condition; this can only be determined by deciding
;	that the byte about to be read is a 'first' byte and
;	we already BUILDING a packet.
;	This change is a radical departure from previous
;	approaches to the RX code and might perhaps work
;	(HA, Ha!)
;
;	Dave Price. October 23rd 1990 16:06.
;	Some change of thought again.. I hate 8952s!
;	I am moving to four states in RX protocol engine.
;	'idle' will mean - finished one packet, awaiting next
;	'skipping' will mean we failed to get a buffer so
;	we are awaiting this packet to go by before trying
;	again for a buffer. I.E. we are discarding all input
;	waiting for an FA or EOPD etc etc
;	'found' means that the NEXT byte in the fifo
;	is a 'first' byte. I.E. here comes the packet...
;	'building' means we have a buffer and we are off
;	making up the next packet.
;
;	Dave Price. October 23rd 1990 09:40.
;	Having got completely fed up with lots of minor bugs
;	in the RX code, I am now carrying on with the changes
;	started earlier on 17th to attempt to have
;	some more clean code for the RX side. Most of the
;	code has been developed over the weekend but is 
;	handwriiten on the last listing. Problems
;	mainly arise with odd combinations of events
;	rather than simple circumstances. A major change is
;	that the RX code will now longer go and get itself
;	a buffer until the 'first byte' has been located. In
;	particular the completion of the collection of one
;	packet was immediately followed by the allocation
;	of a new buffer. This will now not happen.
;	I also intend at a later date to add fields to the
;	hdlc datastructure to hold port addresses etc. This
;	will start to pave the way for making the driver handle
;	multiple channels. It will require other changes as
;	well though (mainly stopping the code use constructions
;	like hdlc0.fred and instead move to set bx; [bx].fred.
;	This is not straightforward though as bs is already
;	used as a pointer. It will imply lots of pushing and
;	popping probably. All this is the next fix NOT
;	this change anyway.
;
;	October 17th 1990 20:30. Work starts to alter RX data
;	structures with a view to adding a 'state' variable
;	and dealing with input quite differently.
;
;	October 17th 1990.
;	Several new patches of code added to try to
;	the remaining bugs. Most bugs are caused by too long
;	packets being received (possibly because rx fails to
;	deal with FAs and RXOFLOWs correctly).
;
;	October 16th 1990. The code has now been used
;	fairly successfully. Some files have been transfered
;	using FTP from a sun via one NOS router over a 64Kbps
;	link from a second NOS PC. The central router had to
;	be rebooted once during the transfer as the driver
;	ran out of receive buffers! Amazingly the file
;	transfered o.k! The file was a 43Kbyte binary
;	of a virus checking program.
;	Code has been added to cope with RXofloe and Frame
;	abort, but bugs exist.
;
;	October 11th 1990.  The code has been running now
;	used by NOS. Some problems had occurred with 
;	events like txdone also having tx419 set.
;	Even though only txdone was enabled as an interrupt,
;	reading the 'interrupt flag register' showed both
;	bits set. As the code allows for several conditions
;	to be true it obeyed the one set of code and then 
;	attempted to handle the other condition too! This
;	resulted in errors.
;	The code now carefully processes txdone and then avoids
;	the tx419 condition!
;	Similar problems exist with eopd and fa!
;
;	October 1st 1990. Code is now in place to handle
;	rx and tx interrupts. user can also specify -n
;	so board acts as an NT.
;	Only RX1519, and EOPD handled on receive and TXDONE and
;	TX419 handles on transmit.
;
;	Buffer strategy Changed again. 25 September 1990. Dave Price
;	The idea now is that there will be a ring of
;	structures, each structure containing a little control
; 	information plus a Data Unit in which will be placed
;	an IP frame (or potentially any other type of frame).
;	There will be two such rings, one for transmission
;	and one for reception.
;	The rings will be statically allocated.
;	The Data Units will be set at 1500 bytes, the same
;	as the maximum MTU for ethernet packet drivers.
;
;	Simple byte-ring-buffer has proved awkward to
;	code. One often seems to be fighting the INTEL CPU.
;	On reflection a 'frame' based approach might be better.
;
;	Added Code for RJG suggested Buffer Management. The
;	idea is described in an ARUW?? document. There will
;	be a circular ring buffer of bytes (like the slip
;	drivers) with an associated structure to hold the
;	state of the buffers.
;
;	More bits from Dave Price to initialize
;	MITEL express card. Just plugs voice so far.
;	30/8/90
;
; This is a hacked version of slip8250 packet driver.
; The hack is beginning on 28/8/90.
; First attempts are just to change messages etc!
;
;	Changes started by Dave Price
;
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

code	segment	byte public
	assume	cs:code, ds:code
;
;	Constants etc from MITEL Express card
;
;	First the DX (8980)
;
board_w_dx	dw	0300h

dx_b_con		equ	0000h
dx_b_cm_base	equ	0400h

dx_con_cmh		equ	00011000b
dx_con_cml		equ	00010000b
dx_cmh_mchan	equ	00000100b
dx_cmh_oe		equ	00000001b

;
;	Now the stream and channel assignments
;
snic_stream		equ	6
snic_d_channel	equ	0
snic_c_channel	equ	1
snic_b1_channel	equ	2
snic_b2_channel	equ	3

hdlc_stream		equ	6
hdlc_d_channel	equ	4
hdlc_c_channel	equ	5
hdlc_b1_channel	equ	6
hdlc_b2_channel	equ	7
hdlc_b3_channel	equ	8

dphone_stream		equ	7
dphone_unused_channel	equ	4
dphone_c_channel		equ	5
dphone_b1_channel		equ	6
dphone_b2_channel		equ	7
dphone_b3_channel		equ	8

;
;	Now some snic values etc
;
board_w_snic		dw	0b00h

snic_b_master		equ	0000h
snic_b_stbus		equ	0001h

snic_master_irqenable	equ	00000000b
snic_master_msdisable	equ	00000010b
snic_master_cstenable	equ	00000000b

snic_stbus_all	equ	0ffh

snic_c_ar		equ	10000000b
snic_c_dr		equ	01000000b
snic_c_dinb		equ	00100000b
snic_c_priority	equ	00010000b
snic_c_dreq		equ	00001000b
snic_c_txmch	equ	00000100b
snic_c_clrdia	equ	00000010b
snic_c_regsel	equ	00000001b

;
;	now some dphone values
;
board_w_dphone	dw	1700h

dphone_b_c		equ	0000h
dphone_b_time		equ	0005h
dphone_b_wdog		equ	0006h
dphone_b_tone1		equ	0007h
dphone_b_tone2		equ	0008h
dphone_b_dsp		equ	0009h
dphone_b_trans		equ	000ah
dphone_b_rgain		equ	000bh
dphone_b_sddata	equ	000ch
dphone_b_sddir		equ	000dh
dphone_b_test		equ	000eh

dphone_sddir_allout	equ	0ffh
dphone_sddata_te	equ	0b0h
dphone_sddata_nt	equ	0b8h

dphone_time_pcmb1	equ	00000001b
dphone_time_pcmb2	equ	00000100b
dphone_time_pcmb3	equ	00010000b

dphone_time_c		equ	10000000b

dphone_tone_697		equ	59h
dphone_tone_1209		equ	9bh

dphone_test_disable	equ	00h

dphone_dsp_cpcmen		equ	01000000b
dphone_dsp_dpcmen		equ	00100000b
dphone_dsp_dual		equ	00001000b
dphone_dsp_tone		equ	00010000b
dphone_dsp_speaker	equ	00011000b
dphone_dsp_cadence	equ	00000100b
dphone_dsp_warble16	equ	00000010b
dphone_dsp_dspen		equ	00000001b

dphone_trans_dial		equ	00100000b
dphone_trans_side		equ	00010000b
dphone_trans_hsmic	equ	00001000b
dphone_trans_spmic	equ	00000100b
dphone_trans_spskr	equ	00000010b
dphone_trans_hsskr	equ	00000001b

dphone_rgain_hpf		equ	10000000b
dphone_rgain_rfg_m7	equ	01110000b

;
;	now the hdlcs relative to the board base
;
board_w_hdlc0	dw	0f00h
board_w_hdlc1	dw	1300h
;
;	Now register offsets in the hdlc chips
;
hdlc_br_fifostatus	equ	00h
hdlc_br_receive		equ	01h
hdlc_bw_transmit		equ	01h
hdlc_b_control		equ	02h
hdlc_b_raddress		equ	03h
hdlc_b_cchancontrol	equ	04h
hdlc_b_time			equ	05h
hdlc_br_intflag		equ	06h
hdlc_bw_wdog		equ	06h
hdlc_bw_intenable		equ	07h
hdlc_br_genstatus		equ	08h
hdlc_br_cchanstatus	equ	09h

;
;	Now some values for the registers of the hdlc's
;
hdlc_fifostatus_RXBYTE		equ	11000000b
hdlc_fifostatus_packet		equ	00000000b
hdlc_fifostatus_first		equ	01000000b
hdlc_fifostatus_good		equ	10000000b
hdlc_fifostatus_bad		equ	01000000b
hdlc_fifostatus_last		equ	10000000b

hdlc_fifostatus_RXFIFO		equ	00110000b
hdlc_fifostatus_rxempty		equ	00000000b
hdlc_fifostatus_rxle14		equ	00010000b
hdlc_fifostatus_rxfull		equ	00100000b
hdlc_fifostatus_rxge15		equ	00010000b

hdlc_fifostatus_TXFIFO		equ	00001100b
hdlc_fifostatus_txfull		equ	00000000b
hdlc_fifostatus_txge5		equ	00000100b
hdlc_fifostatus_txempty		equ	00001000b
hdlc_fifostatus_txle4 		equ	00000100b

hdlc_control_txen		equ	10000000b
hdlc_control_rxen		equ	01000000b
hdlc_control_rxad		equ	00100000b
hdlc_control_ra6		equ	00010000b
hdlc_control_iftf1	equ	00001000b
hdlc_control_iftf0	equ	00000100b
hdlc_control_fa		equ	00000010b
hdlc_control_eop		equ	00000001b

hdlc_control_idle		equ	00000000b
hdlc_control_iftf		equ	00000100b
hdlc_control_trans	equ	00001000b
hdlc_control_goahead	equ	00001100b

hdlc_time_rst		equ	10000000b
hdlc_time_ic		equ	01000000b
hdlc_time_c1en		equ	00100000b
hdlc_time_brck		equ	00010000b
hdlc_time_tc		equ	00001111b

hdlc_time_c2bits8		equ	00000011b
hdlc_time_c3bits8		equ	00000100b
hdlc_time_c4bits8		equ	00000101b
hdlc_time_c23bits16		equ	00000110b
hdlc_time_c234bits24	equ	00000111b

hdlc_intflag_ga	equ	10000000b
hdlc_intflag_eopd	equ	01000000b
hdlc_intflag_txdone	equ	00100000b
hdlc_intflag_fa	equ	00010000b
hdlc_intflag_tx419	equ	00001000b
hdlc_intflag_txurun	equ	00000100b
hdlc_intflag_rx1519	equ	00000010b
hdlc_intflag_rxoflw	equ	00000001b

hdlc_intenable_ga		equ	10000000b
hdlc_intenable_eopd		equ	01000000b
hdlc_intenable_txdone	equ	00100000b
hdlc_intenable_fa		equ	00010000b
hdlc_intenable_tx419	equ	00001000b
hdlc_intenable_txurun	equ	00000100b
hdlc_intenable_rx1519	equ	00000010b
hdlc_intenable_rxoflw	equ	00000001b


hdlc_genstatus_rxoflw	equ	10000000b
hdlc_genstatus_txurun	equ	01000000b
hdlc_genstatus_ga	equ	00100000b
hdlc_genstatus_abrt	equ	00010000b
hdlc_genstatus_irq	equ	00001000b
hdlc_genstatus_idle	equ	00000100b


;
;	Now the overall interrupt register
;
board_w_intreg	dw	1b00h

intreg_hdlc0	equ	00000010b	;bit for hdlc0 interrupt
intreg_hdlc1	equ	00000001b	;bit for hdlc1 interrupt
intreg_snic	equ	00000100b	;bit for snic interrupt
intreg_hphone	equ	00001000b	;bit for d/hphone interrupt
;
;	now a few usefull macros
;
out_chip_reg_value	macro	chip,reg,value
	mov	dx,chip
	add	dx,reg
	mov	al,value
	out	dx,al
	endm

in_chip_reg		macro	chip,reg
	mov	dx,chip
	add	dx,reg
	in	al,dx
	endm

dx_message	macro	stream,channel,value
	out_chip_reg_value	board_w_dx,dx_b_con,<dx_con_cmh or stream>

	out_chip_reg_value	board_w_dx,<dx_b_cm_base or channel>,<dx_cmh_mchan or dx_cmh_oe>

	out_chip_reg_value	board_w_dx,dx_b_con,<dx_con_cml or stream>

	out_chip_reg_value	board_w_dx,<dx_b_cm_base or channel>,value

	endm

dx_source	macro	d_stream,d_channel,s_stream,s_channel

	out_chip_reg_value	board_w_dx,dx_b_con,<dx_con_cmh or d_stream>
	out_chip_reg_value	board_w_dx,<dx_b_cm_base or d_channel>,dx_cmh_oe
	out_chip_reg_value	board_w_dx,dx_b_con,<dx_con_cml or d_stream>
	out_chip_reg_value	board_w_dx,<dx_b_cm_base or d_channel>,<s_stream shl 5 or s_channel>

	endm

	public	int_no
int_no		db	7,0,0,0		; interrupt number.
NT_switch	db	0	;if 0 be a TE else be an NT

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	6,0,0,0		;from the packet spec
driver_type	db	0,0,0,0		;from the packet spec
driver_name	db	'EXPRESS',0	;name of the driver.
driver_function	db	2
parameter_list	label	byte
	db	1	;major rev of packet driver
	db	0	;minor rev of packet driver
	db	14	;length of parameter list
	db	EADDR_LEN	;length of MAC-layer address
	dw	GIANT	;MTU, including MAC headers
	dw	MAX_MULTICAST * EADDR_LEN	;buffer size of multicast addrs
	dw	0	;(# of back-to-back MTU rcvs) - 1
	dw	0	;(# of successive xmits) - 1
	dw	0	;Interrupt # to hook for post-EOI
			;processing, 0 == none,

;
;	Packet Buffer Structure

owner_k_empty	equ	0
owner_k_queue	equ	1
owner_k_isr	equ	2
info_k_size	equ	1500	;size of info area in buffer
;
buff		struc
buff_w_next	dw	0	;pointer to next buffer
buff_w_prev	dw	0	;pointer to previous buffer
buff_w_size	dw	0	;size of frame in info area in bytes
buff_w_owner	dw	owner_k_empty	;current owner of buffer
buff_info	db	info_k_size dup (0)	;area for the Transfer Unit
buff		ends
;
;	Structure for shared Queue Information
;	Now contains the info for the associated isr routines
;	as well. 17th October 1990.
;
;	First some constants.
;
state_k_skipping	equ	00000001b
state_k_building	equ	00000010b
upcall_k_idle		equ	0
upcall_k_active	equ	1
;
hdlc_data		struc
txq_w_front	dw	0	;pointer to front of tx queue
txq_w_back	dw	0	;pointer to back of tx queue
rxq_w_front	dw	0		;pointer to front of rx queue
rxq_w_back	dw	0		;pointer to back of rx queue
rxupcall_w_state	db	0	;state of any upcall
rxisr_w_state	db	0	;the current state of the isr
rxisr_w_pkt	dw	0	;pointer to the packet being used by rx ISR
rxisr_w_byte	dw	0	;pointer to the byte the rx ISR will use next	
rxisr_w_count	dw	0	;count of bytes inserted so far
txisr_w_pkt	dw	0	;pointer to the packet being used by tx ISR
txisr_w_byte	dw	0	;pointer to the byte tx ISR will use next
txisr_w_count	dw	0	;count of bytes remaining
copy_intflag	db	0	;copy of latest value from int flag
copy_intenable	db	0	;copy of latest value sent int enable
hdlc_data		ends

hdlc0_data	hdlc_data	<offset t1_buff, offset t1_buff, offset r1_buff, offset r1_buff,upcall_k_idle,state_k_skipping>			

;
;	Names for the bits in the byte_status_mask
;	rint_status_mask
;	and the stop_status_mask
;
mask_k_packet	equ	00000001b
mask_k_first	equ	00000010b
mask_k_good	equ	00000100b
mask_k_bad		equ	00001000b
mask_k_fabort	equ	00010000b

byte_status_mask	db	0	;used to save status implied
					;by the current bytes
;rint_status_mask	db	0	;used to save status implied
					;by the bytes so far in this
					;segment in the fifo
;stop_status_mask	db	0	;used to specify when we wish
					;	to stop
;
;	Locations to hold counters of bytes read in
;	one particular call of the interrupt code.
;

;number_read		db	0	;number read so far
;minimum_read	db	0	;minimum number that MUST be read
					; the interrupt style sets this
;maximum_read	db	0	;maximum available

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

send_pkt:
;enter with es:di->upcall routine, (0:0) if no upcall is desired.
;  (only if the high-performance bit is set in driver_function)
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
;called from telnet layer via software interrupt
	assume	ds:nothing

	push	cs
	pop	es
	assume	es:code
	sti				; enable interrupts
;
;	NOTE Using ES as segment for data accesses
;
	mov	bx,es:hdlc0_data.txq_w_back	;get pointer to back of queue
	cmp	es:[bx].buff_w_owner,owner_k_empty	;is it empty?
	jne	no_buffers_left		;no so error return...

	cmp	cx,info_k_size		;	check packet size o.k.
	jle	send_pkt_size_ok	; its fine
	pr_ch_al	'a'			;	error trace message
	mov	dh,CANT_SEND	;return an error code
	cli
	stc
	ret

send_pkt_size_ok:
	mov	es:[bx].buff_w_size,cx	;save size
	lea	di,es:[bx].buff_info		;
	rep	movsb				;and copy the packet

	mov	es:[bx].buff_w_owner,owner_k_queue	 ;give it to queue
	mov	bx,es:[bx].buff_w_next	;point to buffer
	mov	es:hdlc0_data.txq_w_back,bx		;adjust back of the queue
;
;	NOW WE NEED TO PROVOKE LOADING OF TXFIFO
;	IF WE THINK ISR GONE QUIET
;
; structure is at zero no ints active
	cli		;block interrupts starting before we exit
	cmp	es:hdlc0_data.txisr_w_pkt,0
	je	send_pkt_int_quiet	; jump if interrupts quiet
	clc					;clear carry because all o.k.
	ret
;
;	Else we now need to kick the interrupt code
;
send_pkt_int_quiet:
	push	ds		;save old ds and make it point to code
	push	cs
	pop	ds
	pr_ch_al	'b'
	call	tint_new		;manually call the tint routine !
	pop	ds			;restore old ds and
	ret				;return


no_buffers_left:
	pr_ch_al	'c'
	mov	dh,NO_SPACE
	cli	;block interrupts before we exit
	stc			;signal its an error - no more buffers
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

recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
	assume	ds:code
recv_2:
	in_chip_reg		board_w_intreg,0	; get interrupt source

	test	al,intreg_hdlc0		;check if HDLC0
	jne	not_hdlc0

	jmp	which_reg		;Jump and check the chip

not_hdlc0:
	pr_ch_al	'+'
	ret			;Not this chip so give up

which_reg:
	in_chip_reg		board_w_hdlc0,hdlc_br_intflag
	mov	hdlc0_data.copy_intflag,al	;save the interrupt flags
;
;	We now analyse for the different interrupts that
;	may be present. We first make some tests for styles
;	of interrupts so we dont need to check everything
;	each time.
;
	test	hdlc0_data.copy_intflag,hdlc_intflag_eopd or hdlc_intflag_fa or hdlc_intflag_rx1519 or hdlc_intflag_rxoflw
	je	test_tint	;No receive style interrupts so jump

; There is a receive event
;
;	We recognise 8 receive events.
;	1/.	RX1519
;	2/.	RX1519 + RXOFLW
;	3/.	EOPD
;	4/.	EOPD+RX1519
;	5/.	EOPD + RX1519 + RXOFLW
;	6/.	FA + EOPD
;	7/.	FA + EOPD + RX1519
;	8/.	FA + EOPD + RX1519 + RXOFLW
;	We believe that other (potential) RX events cannot
;	occur. 
;	We recognise only three (major) states for the RX
;	protocol engine.
;	1/.	SKIPPING for the start of a packet; all data
;		is essentially ignored in this state. No
;		receive buffer will have been allocated.
;		We are in this state between packets or perhaps
;		because we have just run out of buffers!
;		See also state 3 below.
;	2/.	BUILDING a packet. Generally speaking,
;		providing we have enough room, 'packet' bytes
;		are just added into the buffer, 'good last' bytes
;		terminating the building of a packet and provoke
;		its delivery to the upper layer. Other data such
;		as 'bad last' or 'first' bytes cause the packet
;		being built to be discarded and the engine to
;		move to SKIPPING state. A first byte implies
;		a frame abort has been received of course.
;	3/.	It is also possible to be SKIPPING&BUILDING !
;		This occurs after we have read data that
;		would have moved us to BUILDING state but
;		we could not get buffer space. We need this
;		to properly detect frame aborts (see below).
;
;	EVENT PROCESSING in a little more detail.
;
;	1/. RX1519
;	We are required to clear 14 bytes from the fifo.
;	It is possible to show that in pathological
;	circumstances bytes other than packet bytes can
;	be in the FIFO!
;
;	2/.	RX1519 + RXOFLW
;	Regardless of state, we read 19 bytes from the FIFO.
;	We do this as we do not want to accidently
;	discard the front of a following packet.
;
;	3/.	EOPD
;	We process bytes up to a bad/good last byte.
;	We then deliver or discard the packet.
;	Clearly should not process more than 19 and
;	if RX1519 set it would be surprising if we
;	processed more than 14!
;
;	4/.	EOPD + RX1519
;	We are required to clear 14 bytes from the fifo.
;	Process as in 3/. above but make sure we clear
;	at LEAST 14 bytes. We might read more than 15
;	if for instance the EOPD occurred as the 16 byte
;	which had arrived while we were responding to
;	the event which interrupted us which was a 'packet'
;	byte in number 15. It is also possible that the 'last'
;	byte might be number 14 but a 'first' byte also arrived
;	in 15 creating the RX1519 to occur as well.
;
;	5/.	EOPD + RX1519 + RXOFLW
;	The RXOFLW implies we have missed stuff and some
;	data failed to get into the end of the FIFO. The
;	RX FIFO is supposed to enter a FLAG search mode after
;	overflowing. Thus is we process as in 3/. but EMPTY
;	the whole buffer(i.e. read 19 bytes)., 
;
;	6,7,8/.	I.E. any FA condition.
;	We will read upto 19 bytes of data. It would
;	be surprising if we read more than 14 unless RX1519
;	was also set. We will end when we discover a Frame
;	Abort condition. This is implied by a first byte
;	when building but ALSO by a first byte if skipping
;	and we have already seen a first byte!
;
;
;	Now set up some values. The maximum and minimum
;	count values, the current number read
;	and we clear the stop_status_mask.

;	mov	minimum_read,1	;often increased below
;	mov	maximum_read,14	;typically we end before this
;	mov	stop_status_mask,0	;clear the stop status mask
;
;test_fa:
;	test	hdlc0_data.copy_intflag,hdlc_intflag_fa
;	je	test_eopd
	; We have an FA so change stop_status_mask
	; and change the maximum read to 19 (?)
;	mov	stop_status_mask,mask_k_fabort
;	mov	maximum_read,19	;typically we end before this
;	jmp	test_rxoflw	;can avoid the eopd check
;
;test_eopd:
;	test	hdlc0_data.copy_intflag,hdlc_intflag_eopd
;	je	test_rxoflw
;	; Its eopd so update stop_status_mask
;	; and change the maximum read to 19 (?)
;	mov	stop_status_mask,mask_k_good or mask_k_bad
;	mov	maximum_read,19	;typically we end before this
;
;test_rxoflw:
;	test	hdlc0_data.copy_intflag,hdlc_intflag_rxoflw
;	je	test_rx1519
;	; Its Overflow so adjust minimum read number
;	; and change the maximum read to 19 (?)
;	mov	minimum_read,19
;	mov	maximum_read,19	;typically we end before this
;	jmp	test_rx_end	;can avoid the rx1519 check
;
;test_rx1519:
;	test	hdlc0_data.copy_intflag,hdlc_intflag_rx1519
;	je	test_rx_end
;	; Its Rx1519 so adjust the minimum read number
;	mov	minimum_read,14
;
test_rx_end:
;
;WE DONT CARE WHAT CAUSED THE INTERRUPT NOW!
;
	; Now we call the routine to process the RX fifo having
	; hopefully set up all the correct conditions.

	call	rint_process

test_tint:
	mov	al,hdlc0_data.copy_intflag
	and	al,hdlc0_data.copy_intenable ; ignore any not expected
	mov	hdlc0_data.copy_intflag,al

	test	hdlc0_data.copy_intflag,hdlc_intflag_txdone or hdlc_intflag_tx419 or hdlc_intflag_txurun
	je	test_ga	;No transmit style interrupts so jump

; Its some sort of transmit event

	test	hdlc0_data.copy_intflag,hdlc_intflag_txurun
	je	test_txdone
	call	tint_txurun; its an underrun event
;	Now need to avoid the txdone and tx419 code etc ....
	jmp	test_ga

test_txdone:
	test	hdlc0_data.copy_intflag,hdlc_intflag_txdone
	je	test_tx419
	call	tint_txdone; its packet trans. complete event
	jmp test_ga			; NOTE tx419 will always be set
		;when txdone is set! As we have processed
		;the outgoing packet we must now NOT go
		;through the tx419 code as well! IMPORTANT!

test_tx419:
	test	hdlc0_data.copy_intflag,hdlc_intflag_tx419
	je	test_ga
	call	tint_tx419; its a tx fifo low event

; and carry on...

test_ga:
	test	hdlc0_data.copy_intflag,hdlc_intflag_ga
	je	int_fin

;	Its a Go-ahead .. We should not get these..

	pr_ch_al	'-'

int_fin:

;	Now we have finished.. Just output H so we can check

	ret

;
;Process 8952 Receive interrupts
;

;	Process all RX FIFO data

rint_process:	
;	pr_ch_al	'A'

;	mov	number_read,0	;none so far...
;	mov	rint_status_mask,0	;clear the rint status mask

	push	ds			; get set up for the routine
	pop	es

rint_loop:
;	NEED FIXES IN HERE
;	mov	al,number_read
;	cmp	maximum_read,al	;check if data still due
;	jg	rint_some_due	;yes there is
;	pr_ch_al	'B'
;	ret				;no we have finished

;rint_some_due:

	;get fifo status and build into a mask
	;must first check that there is still some data left..
	; NOTE it is important that we dont get here less than
	; one microsecond after we last removed data.

	in_chip_reg		board_w_hdlc0,hdlc_br_fifostatus
	test	al,hdlc_fifostatus_RXFIFO
	jne	rint_fifo_not_empty
	ret					; we have now emptied the RX FIFO
						; so we return...
rint_fifo_not_empty:		;still some data so analyze...
	mov	cl,6		;number of bits to shift
	and	al,hdlc_fifostatus_RXBYTE ;get RX byte status
	shr	al,cl		;shift to lower two bits
	mov	cl,al		;transfer to cl
	mov	al,00000001b	;set low bit in al
	shl	al,cl			;and shift to correct bit for mask
	mov	byte_status_mask,al	;save byte status mask
;	or	rint_status_mask,al	;save rint segment status mask

;
;	Now check our state
;
	test	hdlc0_data.rxisr_w_state,state_k_skipping
	jz	rint_building	;must be building alone
	jmp	rint_skipping	;skipping or skipping&building

;
;	Definitely building a packet
;
rint_building:

	test	byte_status_mask,mask_k_good;good last byte ?		
	jnz	rint_build_good_last			;deal with good last byte

	test	byte_status_mask,mask_k_bad;bad last byte ?		
	jnz	rint_build_bad_last			;deal with bad last byte

	test	byte_status_mask,mask_k_first;first byte ?		
	jnz	rint_build_first			;deal with first byte

;
;	must be packet byte
;
	in_chip_reg		board_w_hdlc0,hdlc_br_receive
	mov	di,hdlc0_data.rxisr_w_byte ;get ptr to next byte
	stosb			;store char into buffer
	mov	hdlc0_data.rxisr_w_byte,di ;save ptr to next byte
	
	inc	hdlc0_data.rxisr_w_count			;count for all isr calls
;	inc	number_read			;count for this isr

	jmp	rint_end

rint_build_good_last:
;
;	good last byte coming, get it, then close
;	the packet and assign it to queue ownership etc.
;	then bump the pointers
;
;	pr_ch_al	'C'
	in_chip_reg		board_w_hdlc0,hdlc_br_receive
	mov	di,hdlc0_data.rxisr_w_byte ;get ptr to next byte
	stosb			;store char into buffer
	mov	hdlc0_data.rxisr_w_byte,di ;save ptr to next byte
	inc	hdlc0_data.rxisr_w_count			;count for all isr calls
;	inc	number_read			;count for this isr

	mov	bx,hdlc0_data.rxisr_w_pkt	;get pointer to buffer

	mov	ax,hdlc0_data.rxisr_w_count ;save count in buffer

	mov	[bx].buff_w_size,ax ;via ax

	mov	[bx].buff_w_owner,owner_k_queue	;mark in q
	mov	bx,[bx].buff_w_next	;get pointer to next
	mov	hdlc0_data.rxq_w_back,bx	;and save as back of queue
;
;	and change state
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping

	jmp	rint_end

rint_build_bad_last:

;
;	Now deal with bad FCS, read the bad byte and then
;	then discard the packet we have got so far.
;
	pr_ch_al	'D'
	in_chip_reg		board_w_hdlc0,hdlc_br_receive
				;get char but ignore
;	inc	number_read			;count for this isr
	mov	bx,hdlc0_data.rxisr_w_pkt	;get ptr to buffer
	call	rint_reset	;release the buffer etc
;
;	and change state
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping
	jmp rint_end

rint_build_first:
;
;	Now deal with first byte, this must be a frame abort!
;	DONT read the byte, leave it there for next iteration
;	to use as part of the next packet.
;	Discard the packet we have got so far.
;
	pr_ch_al	'E'
	mov	bx,hdlc0_data.rxisr_w_pkt	;get ptr to buffer
	call	rint_reset	;release the buffer etc
;
;	Record the Frame Abort in the rint_status_mask
;
;	or	rint_status_mask,mask_k_fabort
;
;	and change state
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping
	jmp rint_end

rint_skipping:
;
;	If we are here then we must be in between packets.
;	It is possible that we may have run out of buffers
;	so we may be actually discarding data that would
;	otherwise have been good. If this second situation
;	exists then both the building and skipping bits
;	are set in the state mask.
;

	test	byte_status_mask,mask_k_good;good last byte ?		
	jnz	rint_skip_good_last			;deal with good last byte

	test	byte_status_mask,mask_k_bad;bad last byte ?		
	jnz	rint_skip_bad_last			;deal with bad last byte

	test	byte_status_mask,mask_k_first;first byte ?		
	jnz	rint_skip_first			;deal with first byte

;
;	must be packet byte
;
	in_chip_reg		board_w_hdlc0,hdlc_br_receive
				;get char but ignore
;	inc	number_read			;count for this isr
;
;	Well we are certainly skipping past bytes now.
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping or state_k_building

	jmp	rint_end

rint_skip_good_last:
;
;	good last byte coming, get it, and ignore
;
	pr_ch_al	'F'
	in_chip_reg		board_w_hdlc0,hdlc_br_receive
				;get char but ignore
;	inc	number_read			;count for this isr

;
;	change state (might have been skipping and building)
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping

	jmp	rint_end

rint_skip_bad_last:

;
;	Now deal with bad FCS, read the bad byte and ignore
;
	pr_ch_al	'G'
	in_chip_reg		board_w_hdlc0,hdlc_br_receive
				;get char but ignore
;	inc	number_read			;count for this isr
;
;	change state (might have been skipping and building)
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping
	jmp rint_end

rint_skip_first:

;
;	Now deal with first byte, this must be the start
;	of the next packet.
;
	test	hdlc0_data.rxisr_w_state,state_k_building
	jnz	rint_skip_build	;skipping&building
;
;	As we are here we are just skipping at the moment.
;	As we have found a first byte, we now try to
;	get a buffer in which to build the new packet.
;
	call	rint_get_buffer
	jnc	rint_skip_first_got_buffer
;
;	We failed to get buffer. We thus must discard
;	the incoming data, count it and move to the
;	skipping&building state.
;
	pr_ch_al	'H'
	in_chip_reg		board_w_hdlc0,hdlc_br_receive
				;get char but ignore
;	inc	number_read			;count for this isr
;
;	and change state
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping or state_k_building
	jmp rint_end

rint_skip_first_got_buffer:
;	pr_ch_al	'I'
	in_chip_reg		board_w_hdlc0,hdlc_br_receive
	mov	di,hdlc0_data.rxisr_w_byte ;get ptr to next byte
	stosb			;store char into buffer
	mov	hdlc0_data.rxisr_w_byte,di ;save ptr to next byte
	
	inc	hdlc0_data.rxisr_w_count			;count for all isr calls
;	inc	number_read			;count for this isr
;
;	and change state
;
	mov	hdlc0_data.rxisr_w_state,state_k_building
	jmp	rint_end

rint_skip_build:
;
;	We are skipping and building. I.e. we are
;	discarding data that probably would have been
;	good but we had no buffers available.
;	Thus if we find a first byte this must be a frame
;	abort. Deal with it as such, leaving the byte in
;	the FIFO to be picked up on the next cycle.
;
;	Record the Frame Abort in the rint_status_mask
;
	pr_ch_al	'J'
;	or	rint_status_mask,mask_k_fabort
;
;	and change state
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping

	jmp rint_end

rint_end:
;
;	Now is the time to tidy up at the end of
;	the rint loop. Several things to check. For instance,
;	if we are still building and not skipping then
;	if the packet is already full we have a problem.
;	The best policy must be to discard and move
;	to skipping&building state.
;
;	We must also check if we need to stop iterating.
;	If we have not yet read the minimum_read number
;	of bytes we must go again. If we have, then
;	unless we have got a rint_status_mask that has 
;	at least one bit in common with our stop_status_mask
;	we must also loop again.

	test	hdlc0_data.rxisr_w_state,state_k_skipping
	jnz	rint_info_size_ok	;dont care...

	cmp	hdlc0_data.rxisr_w_count,info_k_size	;check if room for more info
	jl	rint_info_size_ok	;and branch if ok
;
;	TOO much data in this packet, the next byte
;	even if it were a last byte would overfill
;	the info area. We must therefore discard the packet
;	and move to the skipping&building state.
	pr_ch_al	'K'
	mov	bx,hdlc0_data.rxisr_w_pkt	;get ptr to buffer
	call	rint_reset	;release the buffer etc
;
;	and change state
;
	mov	hdlc0_data.rxisr_w_state,state_k_skipping or state_k_building

rint_info_size_ok:
;	mov	al,number_read
;	cmp	minimum_read,al	;have we read enough?
;	jle	rint_enough			;yes
	jmp	rint_loop				; no go read some more..

;rint_enough:
;	mov	al,stop_status_mask
;	test	rint_status_mask,al	;stop state?
;	jnz	rint_stop_state		;yes
;	jmp	rint_loop				; no go read some more..

;rint_stop_state:
;
;	Well that seems to be it for this call to the RX
;	interrupt service routine so bye bye...

;	ret
	
;
;	Routine to get a buffer (or not)
;

rint_get_buffer:
;	pr_ch_al	'L'

	mov	bx,hdlc0_data.rxq_w_back	;get back of queue
	cmp	[bx].buff_w_owner,owner_k_empty	;is it empty?
	jne	cant_get_buffer

	call rint_grab
	clc		;all o.k.
	ret
cant_get_buffer:
	pr_ch_al	'M'
	stc		;signal error (well at least no buffers left)
	ret

rint_grab:
	or	bx,bx
	jz	rint_grab_problem
	mov	[bx].buff_w_owner,owner_k_isr	;mark inuse by isr
	mov	hdlc0_data.rxisr_w_pkt,bx	;save for isr to use next
	lea	ax,[bx].buff_info	;get address of new info area
	mov	hdlc0_data.rxisr_w_byte,ax	;save for isr
	mov	hdlc0_data.rxisr_w_count,0	;and clear count
;	Now set the state to mark as BUILDING
	mov	hdlc0_data.rxisr_w_state,state_k_building
	clc
	ret
rint_grab_problem:
	pr_ch_al	'N'
	stc
	ret

rint_reset:
	or	bx,bx
	jz	rint_reset_problem
	mov	[bx].buff_w_owner,owner_k_empty	;mark empty
	clc
	ret
rint_reset_problem:
	pr_ch_al	'O'
	stc
	ret

; --------------------------------------------------------------
;
;  recv_exiting
;
	public	recv_exiting
recv_exiting:
	push	ax
	push	bx
	mov	bx,hdlc0_data.rxq_w_front		;get pointer to next buffer
	cmp	[bx].buff_w_owner,owner_k_queue	;belongs to q?
	jne	recv_exiting_exit           ; no - skip to end
	push	cx
	push	dx
	push	ds
	push	es
	push	bp
	push	di
	push	si
	push	cs			; point ds properly
	pop	ds
	cmp	hdlc0_data.rxupcall_w_state,upcall_k_idle	;is receive frame already active?
	jne	already_active		;frame will be caught so jump
	mov	hdlc0_data.rxupcall_w_state,upcall_k_active	;else mark recv_frame starting
	sti				; enable interrupts

	call	recv_frame

	cli
already_active:
	pop	si
	pop	di
	pop	bp
	pop	es
	pop	ds
	pop	dx
	pop	cx
recv_exiting_exit:
	pop	bx
	pop	ax
	ret


; --------------------------------------------------------------
;
;  recv_frame
;
  ifdef debug
	public recv_frame
  endif
recv_frame:
;	pr_ch_al	'P'

	mov	bx,hdlc0_data.rxq_w_front		;get pointer to next buffer

recv_frame_2:
	lea	si,[bx].buff_info		;point to data
	mov	cx,[bx].buff_w_size	;get its size
	jcxz	recv_frame_3		;count zero? yes,just free frame.
;we don't need to set the type because none are defined for our HDLC encoding.
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
;	pr_ch_al	'Q'
	push	cx
	push	es			;remember where the buffer pointer is.
	push	di

	rep	movsb	;and copy our packet into users buffer

	pop	si			;now give the frame to the client.
	pop	ds
	pop	cx
;	pr_ch_al	'R'
	assume	ds:nothing

	call	recv_copy
	push	cs
	pop	ds
	pr_ch_al	'S'
	assume	ds:code

recv_frame_3:
	mov	[bx].buff_w_owner,owner_k_empty	;free the buffer
	mov	bx,[bx].buff_w_next		;get pointer to next
	mov	hdlc0_data.rxq_w_front,bx		;adjust front of q

	cmp	[bx].buff_w_owner,owner_k_queue	;belongs to q?
	je	recv_frame_2		; yes so process this one.
	mov	hdlc0_data.rxupcall_w_state,upcall_k_idle	;else mark recv_frame as inactive
;	pr_ch_al	'T'
	ret

;Handle 8952 transmitter interrupts

; --------------------------------------------------------------

tint_txurun:
	pr_ch_al	'd'
	mov	bx,hdlc0_data.txisr_w_pkt	;point to the packet buffer
	call	tint_reset	;reset pointers etc...
	ret


tint_tx419:
;
; - for MT8952B fifo stuff up to 15 chars at a time
;
;	NOW NEED TO POINT DX at TX FIFO
;
	mov	dx,board_w_hdlc0		;make dx point at transmit fifo
	add	dx,hdlc_bw_transmit
	mov	cx,15                   ;fifo fill-loop counter
	mov	bx,hdlc0_data.txisr_w_count	;get count of bytes left
	mov	si,hdlc0_data.txisr_w_byte	;get pointer to next byte
tint_next:
	lodsb				;fetch next char
	dec	bx			;reduce count of remaining bytes
	jne	not_send_last
	push	dx				;save dx
	mov	dx,board_w_hdlc0
	add	dx,hdlc_b_control
	push	ax
	in	al,dx				;get current hdlc control reg
	or	al,hdlc_control_eop	;mark as end of packet
	out	dx,al				;and tell the 8952
	pop	ax				; restore al
	pop	dx				;and dx

not_send_last:
	out	dx,al			;output char
	or	bx,bx		;any more chars to output
	je	tint_no_more        ;none...
	loop	tint_next		;loop while fifo not full
;
;	Still some more so select TXURUN and TX419 ints enabled
;
	pr_ch_al	'e'
	mov	dx,board_w_hdlc0
	add	dx,hdlc_bw_intenable
;	in	al,dx				;get current hdlc intenable
	mov	al,hdlc0_data.copy_intenable
;
;	switch off txdone interrupt and enable tx419 and txurun
;
	and	al,not hdlc_intenable_txdone
	or	al,hdlc_intenable_tx419 or hdlc_intenable_txurun
	out	dx,al				;and tell the 8952
	mov	hdlc0_data.copy_intenable,al		;and save it

	mov	hdlc0_data.txisr_w_count,bx	;save count of bytes left
	mov	hdlc0_data.txisr_w_byte,si	;save pointer to next byte
	clc		;clear carry, all ok
	ret		;and exit

tint_no_more:
;
;	No more so select TXDONE and TXURUN ints enabled only
;
	pr_ch_al	'f'
	mov	dx,board_w_hdlc0
	add	dx,hdlc_bw_intenable
;	in	al,dx				;get current hdlc intenable
	mov	al,hdlc0_data.copy_intenable
;
;	switch off tx419 interrupt and enable txurun and txdone 
;
	and	al,not hdlc_intenable_tx419
	or	al,hdlc_intenable_txdone or hdlc_intenable_txurun
	out	dx,al				;and tell the 8952
	mov	hdlc0_data.copy_intenable,al		;and save it

	mov	hdlc0_data.txisr_w_count,bx	;save count of bytes left
	mov	hdlc0_data.txisr_w_byte,si	;save pointer to next byte
	clc		;clear carry if all ok
	ret		;and exit

;No more characters to transmit -- disable transmit interrupts.

tint_txdone:
;	pr_ch_al	'g'
	mov	bx,hdlc0_data.txisr_w_pkt	;get pointer to isrs pkt
	or	bx,bx				;check if pkt pointer is 0
	je	tint_skip
	mov	[bx].buff_w_owner,owner_k_empty;set owned by empty
tint_skip:
	call	tint_new	;try to move to next buffer
	jc	tint_all_empty	;if carry set then none left
	clc
	ret

tint_all_empty:
;
;	No more so set all tx ints off
;
;	pr_ch_al	'h'
	mov	dx,board_w_hdlc0
	add	dx,hdlc_bw_intenable
;	in	al,dx				;get current hdlc intenable
	mov	al,hdlc0_data.copy_intenable
;
;	switch off tx419, txurun and txdone 
;
	and	al,not (hdlc_intenable_tx419 or hdlc_intenable_txdone or hdlc_intenable_txurun)
	out	dx,al				;and tell the 8952
	mov	hdlc0_data.copy_intenable,al		;and save it

	xor	ax,ax		;clear ax
	mov	hdlc0_data.txisr_w_pkt,ax	;save in pointer to buffer
	ret

;
;	Routine to get info for next packet from queue
;
tint_new:
	mov	bx,hdlc0_data.txq_w_front	;get pointer to front of queue
	cmp	[bx].buff_w_owner,owner_k_queue	;belongs to q?
	jne	tint_no_buffers_in_queue
;
;	now bump the front of queue
;
	pr_ch_al	'i'
	mov	ax,[bx].buff_w_next	;point to buffer
	mov	hdlc0_data.txq_w_front,ax	;adjust front of queue
;
	call	tint_reset	;and set the pointers etc.
	ret

tint_reset:
	or	bx,bx
	jz	tint_reset_problem
	mov	[bx].buff_w_owner,owner_k_isr;set owned by the isr
	mov	ax,[bx].buff_w_size	;get size of data unit
	mov	hdlc0_data.txisr_w_count,ax	;and save it for us
	mov	hdlc0_data.txisr_w_pkt,bx	;save pointer to buffer
	lea	ax,[bx].buff_info	;get address of data unit
	mov	hdlc0_data.txisr_w_byte,ax	;and save for us
	call	tint_tx419		;and pretend we had a tx419
	ret
tint_reset_problem:
	stc
	ret
tint_no_buffers_in_queue:
	pr_ch_al	'j'
	stc		;set carry - could not do it
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

;
;	Now define some buffers for the rings.
;	Do it statically now because its easier.
;
;	First the transmit ring
;
t1_buff	buff	<offset t8_buff, offset t2_buff>
t2_buff	buff	<offset t1_buff, offset t3_buff>
t3_buff	buff	<offset t2_buff, offset t4_buff>
t4_buff	buff	<offset t3_buff, offset t5_buff>
t5_buff	buff	<offset t4_buff, offset t6_buff>
t6_buff	buff	<offset t5_buff, offset t7_buff>
t7_buff	buff	<offset t6_buff, offset t8_buff>
t8_buff	buff	<offset t7_buff, offset t1_buff>
;
;	Now the Receive ring
;
r1_buff	buff	<offset r8_buff, offset r2_buff>
r2_buff	buff	<offset r1_buff, offset r3_buff>
r3_buff	buff	<offset r2_buff, offset r4_buff>
r4_buff	buff	<offset r3_buff, offset r5_buff>
r5_buff	buff	<offset r4_buff, offset r6_buff>
r6_buff	buff	<offset r5_buff, offset r7_buff>
r7_buff	buff	<offset r6_buff, offset r8_buff>
r8_buff	buff	<offset r7_buff, offset r1_buff>
;
;	include the serial trace output subroutines
;

	include	sersub.asm

;any code after this will not be kept after initialization.
end_resident	label	byte

	public	usage_msg
usage_msg	db	"usage: EXPRESS packet_int_no [-n] [driver_class] [int_no] ",CR,LF
		db	"   -n instructs card to be an NT",CR,LF
		db	"   The driver_class should be SLIP or a number.",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for MITEL EXPRESS CARD, version ",'0'+majver,".",'0'+version,CR,LF
		db	"Portions Copyright 1988 Phil Karn",CR,LF
		db	"ISDN bits by Dave Price and Bob Gautier",CR,LF,'$'

class_name_ptr	dw	?
class_name	db	"Interface class ",'$'
slip_name	db	"SLIP",CR,LF,'$'
int_no_name	db	"Interrupt number ",'$'
express_start	db	"starting to initial EXPRESS card",CR,LF,'$'
express_finish	db	"completed initialization of EXPRESS card",CR,LF,'$'

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
;
;	Only SUPPORT -n SWITCH ARGUMENT AT THE MOMENT
;
	cmp	byte ptr [si+1],'n'	;did they specify '-n'?
	je	got_NT_switch
	stc				;no, must be an error.
	ret
got_NT_switch:
	mov	NT_switch,1
	add	si,2			;skip past the switch's characters.
	jmp	parse_args		;go parse more arguments.
not_switch:
	or	al,20h			;convert to lower case (assuming letter).
parse_args_2:
	cmp	al,'s'
	jne	parse_args_3
	mov	driver_class,6		;SLIP, from packet spec.
	mov	dx,offset slip_name
	jmp	short parse_args_1
parse_args_3:
	mov	di,offset driver_class
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
;
;	Might get number of buffer in TX queue here.
;
	clc
	ret


; --------------------------------------------------------------
;
;  etopen
;

	public	etopen
etopen:
	pushf
	cli

	call	open	; open the serial port for traces

	;let user know we are about to initial
	; the express card

	pr_ch_al	'$'
;
;Now set up the Mitel Express Card
;
;
;	First the dphone - this controls timing as well
;
	out_chip_reg_value	board_w_dphone,dphone_b_test,dphone_test_disable
;
;	Now the board timing via the dphone sense/drive port
;
	out_chip_reg_value	board_w_dphone,dphone_b_sddir,dphone_sddir_allout
	cmp	NT_switch,1	;has user selected NT operation
	jne	act_as_te		;no so set as TE
	out_chip_reg_value	board_w_dphone,dphone_b_sddata,dphone_sddata_nt
	jmp	te_nt_set

act_as_te:
	out_chip_reg_value	board_w_dphone,dphone_b_sddata,dphone_sddata_te

te_nt_set:

;
;	Now set use of st-bus timeslots
;
	out_chip_reg_value	board_w_dphone,dphone_b_time,<dphone_time_c or dphone_time_pcmb1>
;
;	Now stop the watchdog
;
	out_chip_reg_value	board_w_dphone,dphone_b_wdog,0
;
;	Now set the tone values
;
	out_chip_reg_value	board_w_dphone,dphone_b_tone1,dphone_tone_697
	out_chip_reg_value	board_w_dphone,dphone_b_tone2,dphone_tone_1209
;
;	Now set up the dsp
;
	out_chip_reg_value	board_w_dphone,dphone_b_dsp,<dphone_dsp_cpcmen or dphone_dsp_dpcmen or dphone_dsp_dual>
;
;	Now set up the transducers
;
	out_chip_reg_value	board_w_dphone,dphone_b_trans,<dphone_trans_side or dphone_trans_hsmic or dphone_trans_hsskr>
;
;	Finally the Receive gain control
;
	out_chip_reg_value	board_w_dphone,dphone_b_rgain,dphone_rgain_rfg_m7
;
;	Second the snic
;
	out_chip_reg_value	board_w_snic,snic_b_master,<snic_master_cstenable or snic_master_msdisable or snic_master_irqenable>
	out_chip_reg_value	board_w_snic,snic_b_stbus,snic_stbus_all
;
;	Now set up the hdlc controller
;
	out_chip_reg_value	board_w_hdlc0,hdlc_b_time,hdlc_time_rst
;
;	NOTE you are required to clear reset TWICE
;
	out_chip_reg_value	board_w_hdlc0,hdlc_b_time,<hdlc_time_ic or hdlc_time_brck or hdlc_time_c2bits8>
	out_chip_reg_value	board_w_hdlc0,hdlc_b_time,<hdlc_time_ic or hdlc_time_brck or hdlc_time_c2bits8>

	out_chip_reg_value	board_w_hdlc0,hdlc_b_control,<hdlc_control_rxen or hdlc_control_txen>

	out_chip_reg_value	board_w_hdlc0,hdlc_b_raddress,00
	out_chip_reg_value	board_w_hdlc0,hdlc_bw_wdog,00

	out_chip_reg_value	board_w_hdlc0,hdlc_bw_intenable,<hdlc_intenable_eopd or hdlc_intenable_fa or hdlc_intenable_rx1519 or hdlc_intenable_rxoflw>
	mov	hdlc0_data.copy_intenable,al		;and save it

;
;	Now the DX; plug up the channels and send messages etc,
;

	dx_source	snic_stream,snic_b2_channel,dphone_stream,dphone_b1_channel
	dx_source	dphone_stream,dphone_b1_channel,snic_stream,snic_b2_channel

	dx_source	snic_stream,snic_b1_channel,hdlc_stream,hdlc_b1_channel
	dx_source	hdlc_stream,hdlc_b1_channel,snic_stream,snic_b1_channel

;
;	MIGHT NEED TO CHANGE THE NEXT FOR NT OPERATION
;

	dx_message	snic_stream,snic_c_channel,<<snic_c_ar or snic_c_clrdia>>

	;let user know we have finished
	; initializing the express card

	pr_ch_al	'%'

;Set interrupt vector to EXPRESS handler

	call	set_recv_isr

	mov	dx,offset end_resident

	push	dx			;save the ending address.

	pop	dx			;return the ending address.
	popf
	clc				;indicate no errors.
	ret


	public	print_parameters
print_parameters:
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

	ret

code	ends

	end
