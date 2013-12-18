version	equ	0

	include	defs.asm
;		ntinet.mac
;
;  macros used for calling c routines.
;
pusharg	macro	seg, off
	ifnb	<seg>
	  push	seg
	$$cnt = $$cnt+2
	endif
	ifnb	<off>
	  push	off
	$$cnt = $$cnt+2
	endif
	endm


ccall	macro	rtn, p1, p2, p3, p4, p5
$$cnt = 0
	ifnb	<p5>
	  pusharg	p5
	endif
	ifnb	<p4>
	  pusharg	p4
	endif
	ifnb	<p3>
	  pusharg	p3
	endif
	ifnb	<p2>
	  pusharg	p2
	endif
	ifnb	<p1>
	  pusharg	p1
	endif
	  call	rtn
	if $$cnt
	  add	sp,$$cnt
	endif
	endm

save	macro	regs
	  irp	arg,<regs>
	  push	arg
	  endm
	endm
	
restore	macro	regs
	  irp	arg,<regs>
	  pop	arg
	  endm
	endm

;

	include	lance.inc

;
;  GLBL.INC
;
;  global equates for lance dumb board
;
pt_status	equ	0		;read status port
pt_clrclk	equ	0		;write to clear clock interrupt
pt_etaddr	equ	1		;read ether address rom offset
pt_resetl	equ	1		;write to reset lance
pt_ldata	equ	2		;lance data port 2,3
pt_laddr	equ	4		;lance address port 4,5

st_mask		equ	0ch		;00001100B for (is1 is0)

;int selects
irqn_10	equ	00h			;(is0 is1) = (0 0)
irqn_11	equ	04h			;(0 1)
irqn_12	equ	08h			;(1 0)
irqn_15	equ	0ch			;(1 1)

IRQ10		equ	10

; status port bits

NET_INT	equ	01h		;bit for net interrupt status
CLK_INT	equ	02h		;bit for clock interrupt status
IPL	equ	04h		;ipl bit
HILO	equ	08h		;ram at 0000, or ram at 8000
MINT	equ	net_int+clk_int		;interrupt mask
ADDR	equ	0F0h		;mask for dual port segment


DPLEN	equ	8000h		;length of dual port ram
BUFSIZE equ	1518		;should change to GIANT + 4(CRC)

; Packet sizes = source(6) + destination(6) + type (2) + I field. (803.2 spec)
;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;Following is the list of variables that need to be adjusted if the dp
;usage is changed:
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

XAREA		equ	5F78H		;start of transmit descriptors
RAREA		equ	0018h		;start of receive descriptors

NUMTDESC	equ	4		;number of transmit descriptors
NUMRDESC	equ	16 		;number of receive descriptors

TBAREA	equ	XAREA+(NUMTDESC*tdesclen)	;xmit buf pool
RBAREA	equ	RAREA+(NUMRDESC*rdesclen)   	;rcv buffer pool

;our data segment is a constant distance from the es!
OUROFFSET	equ	TBAREA + NUMTDESC*BUFSIZE

RLEN	equ	RLEN16			;lance mask for rcv-descriptors
TLEN	equ	TLEN4			;lance mask for snd-descriptors

;the total dp size reserved for lance use
LANCE_SZ equ	INITBLK_SZ+ NUMRDESC*(RDESCLEN+BUFSIZE)+ NUMTDESC*(TDESCLEN+BUFSIZE)

;the end of RAERA
OUTOF_BOUND	equ 	RBAREA		;98; for wrap up checking of r-descs


code	segment	word public
	assume	cs:code, ds:code

	extrn	maskint:near
	extrn	set_recv_isr: near


	public	int_no, io_addr, base_addr
int_no		db	3,0,0,0		; interrupt number. 
io_addr		dw	0338h,0		; I/O address for card (jumpers)
base_addr	dw  	0d000h,0	; base segment for board (jumper set)

int_no_name	db	"Interrupt number ",'$'
io_addr_name	db	"I/O port ",'$'
base_addr_name	db	"Memory address ",'$'

nosdesc_msg	db	"no send descriptor.",CR,LF,'$'
chperr_msg	db	"chip doesn't want to finish init",CR,LF,'$'
porterr_msg	db	"wrong port #",CR,LF,'$'
hwerr_msg	db	"hardware error",CR,LF,'$'
initerr_msg	db 	"can't init lance chip",CR,LF,'$'
good_msg	db	"Lance initialized successfully.",CR,LF,'$'
reset_msg	db	"Can't reset the interface",CR,LF,'$'
interr_msg	db	"IRQ# parameter wrong.",CR,LF,'$'

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	100		;--mz?
driver_name	db	'nti',0,'$'	;name of the driver.
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


	;begin importing data
	public	UNITID, sdesc_cnt, rdesc_cnt, actsdesc, actrdesc
UNITID	db	6 dup (0)		;current ether address of the interface
sdesc_cnt	dw	2		;
rdesc_cnt	dw	8
actsdesc	dw	0
actrdesc	dw	0



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; send_pkt:
;
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
	assume	ds:nothing
	push	es
	call	set_dp

	push	ds
	call	set_ds			;set out ds-->cs

	mov	dx,cx			;a copy of length

	cmp	dx,GIANT		; Is this packet too large?
	ja	send_pkt_toobig

	cmp	dx,RUNT			; minimum length for Ether
	jnb	oklen
	mov	dx,RUNT			; make sure size at least RUNT
oklen:
	mov	bx,[actsdesc]		;get pointer to active snd desc
	mov	cx,0			;set up count down parameters
td10:
	test	es:td_stat[bx],t_own	;have empty buffer?
	jz	dox0			;yes, then send message
	loop	td10			;otherwise, wait for it
	mov	dx,offset nosdesc_msg
	ccall	_dispMSG,dx
	mov	dh,CANT_SEND
	stc				;error ret
	pop	ds
	pop	es
	ret

dox0:
	pop	ds
	mov	cx,dx			;get back the real length
	call	fill			;ds:si - source, es:bx - dest, cx:length
	;ax has the return code, either 0 or 0ffh

	;is chip functioning?
	cmp	ax,0ffh			;fatal error happened last time?
	je	td15			;error!
	clc				;good ret
	pop es
	ret
td15:
	mov	dh,CANT_SEND
	stc
	pop	es
	ret

send_pkt_toobig:
	mov	dh,NO_SPACE
	stc
	pop	ds
	pop	es
	ret

	include	movemem.asm

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; fill - proc the xerror  & copy message to xmit buffer
;
; entry - es:bx points to free xmit descriptor (dest)
;	  ds:si --> source buf pointer
;  	  cx:length of data
;
; exit  - xmit buffer (bx) filled
;	  ax = 0: OK; 0ffh: fatal error happened last send
;	  bx changed
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	public	fill
fill:
	assume	ds:nothing
	push	ds				;save theirs

	call	set_ds				;set ours

	call	xmt_errs			;process previous errors
	cmp	ax,0
	je	success
	pop	ds
	ret					;ax == ffh

success:	
	;ds:si - source, es:bx - dest, cx - length
	;simply send whatever given by the application, assuming the
	;destination address and everything has been taken care of

	pop	ds
	push	cx			;save it
	mov	di,es:td_addr[bx]	;get dest buf pointer 
	call	movemem			;ds:si --> es:di
	pop	cx
	
	push	ds
	call	set_ds
	;get the sdesc ready and pump the data out
	neg	cx			;two's compliment
	or	cx,td_bmask		;set these bits
	mov	es:td_bcnt[bx],cx	;set in length to descriptor
	mov	ax,0			;set descriptor status for lance
	or	ax,t_own		;he now owns it
	or	ax,t_stp		;start of packet
	or	ax,t_enp		;end of packet
	mov	es:td_stat[bx],ax	;give it away		
	mov	ax,l_tdnd+l_inea	;tell lance we have data
	call	w_csr	    		;send it

	;all done, update the actsdesc
	add	bx,tdesclen		;point at next descriptor
	dec	word ptr[sdesc_cnt]		;dec counter
	jnz	tfl				;used all?
	mov	word ptr[sdesc_cnt],NUMTDESC	;yes start at begining	
	mov	bx,XAREA
tfl:
	mov	[actsdesc],bx		;set pointer

	mov	ax,0			
	pop	ds
	ret				;good ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; xmt_errs:
;	Check at if the previous send has any errors or not.
; 	entry - es:bx sdesc
;	exit  - ax:0 - ok; ffh:fatal error
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	xmt_errs
xmt_errs:
	mov	ax,es:td_tdr[bx]	;get rest of status
	test	ax,t_more		;more than one retry needed
	jz	xisr2
	call	count_out_err
	jmp	xisr3
xisr2:
	test	ax,t_one		;one retry?
	jz	xisr3			;no
	call	count_out_err
xisr3:
	test	ax,t_lcol		;late collision
	jz	xisr4			;no
	call	count_out_err
xisr4:
	test	ax,t_def		;deferred xmit?
	jz	xisr5
	call	count_out_err
xisr5:
	test	ax,t_lcar		;loss of carrier
	jz	xisr6
	call	count_out_err
xisr6:
	test	ax,t_uflo		;silo underflow?
	jz	xisr7	
	jmp	fatal_err
xisr7:	
	test	ax,t_buff		;buffer error no end
	jz	all_rite		;last sent all right
fatal_err:
	call	count_out_err
	mov	ax,0ffh			;fatal error
	ret				;

all_rite:
	mov	ax,0
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;get_address:
;	We get the interface address from our internal version UNITID. 
;	set_address() could change the value of UNITID.
;
;enter with es:di -> place to put the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	get_address
get_address:
	assume	ds:code
	cmp	cx,EADDR_LEN		;make sure that we have enough room.
	jb	get_address_2
	mov	cx,EADDR_LEN
	lea	si,UNITID
get_address_1:
	call	movemem
	mov	cx,EADDR_LEN
	clc
	ret
get_address_2:
	stc
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set_address:
;	set current ethernet address in UNITID, also change the init block
;   	
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	set_address
set_address:
	assume	ds:nothing
	push 	ax
	push	di
	push	es
	mov	ax,cs
	mov	es,ax
	
	cmp	cx,EADDR_LEN
	jne	set_address2

	push	si				;save it for later use
	push	cx

	mov	di,offset UNITID
	call	movemem

	;should also change the init_block
	call	set_dp
	mov	di,offset i_padr			
	pop	cx
	pop	si				;get back the old pointer
	call	movemem

	pop	es
	pop	di
	pop	ax
	clc
	ret

set_address2:
	mov	dh,BAD_ADDRESS
	stc
	pop	es
	pop	di
	pop	ax
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
	call	hardware		;reset the UNITID
	cmp	al,0
	jne	port_err

	cli

	call	lance_init		;init_blk, xmt, rcv
	cmp	ax,0			;init ok?
	jne	lance_err
	sti
	clc
	ret

lance_err:
	mov	dx,offset initerr_msg
	ccall	_dispMSG,dx
	jmp	error

port_err:	
	mov	dx,offset reset_msg
	ccall	_dispMSG,dx
error:
	mov	dh,CANT_RESET
	stc
	ret

;called when we want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type, dl = packet class.
	extrn	recv_find: near

;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
	extrn	recv_copy: near

	extrn	count_in_err: near
	extrn	count_out_err: near

;called from the net_isr isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; risr 
; receive interrupt service routine 
; exit: es,bx not changed
; registers: si,ax
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	risr
risr	proc	near
	assume	ds:code
	save	<es,bx>
	call	set_dp			;set es to dual port
begin:
	mov	si,[actrdesc]		;active receive descriptor
	mov	ax,es:rd_stat[si]	;get descriptor status
	test	ax,r_own		;do we own it?
	jnz	risr_done		;this one is not filled

	call	dp_rcv			;lance chip has rev'd a new packet
					;reserve si 
	;One rint might indicate multiple recv's, check at the next one
	add	si,SIZE rec_desc_ring		;point at next descriptor
	cmp	si,OUTOF_BOUND			;RAREA+(numrxbuf-1)*(SIZE rec_desc_ring) = XAREA
	jl	within
	mov	si,RAREA
within:
	mov	[actrdesc],si			;set active pointer
	jmp	begin
risr_done:
	restore <bx,es>
	ret
risr	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  dp_rcv:
;	lance chip has one buffer filled, process it.
;	enter: es:si = actrdesc
;	exit:  actrdesc updated to next desc, si unchanged
;	registers: di,ax,bx
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	dp_rcv
dp_rcv 	proc 	near
	push	si    			;save the old actdesc
	push	es			;gonna be changed in recv_find()

	mov	ax,es:rd_stat[si]	;is it errored packet?
	test	ax,r_err		;
	jz	noerr	     		;no err
	call 	rcverror		;handle the err 
	jmp	clrup

noerr:
	mov 	di,es:rd_addr[si]	;received packet pointer es:di
	add	di,EADDR_LEN + EADDR_LEN;stript off the two ether addresses
					; & get to the packet type field
	mov	cx,es:rd_mcnt[si]	;everything (icl'g addresses)
	and	cx,mcnt_mask		;mask out the reserved bits

	push	ds			;entry value saved

	push	es			;es:di --> rcv'd data (icl'ing type)
	push	di

	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	inc	di			;set di to 802.2 header
	inc	di
	mov	dl, IEEE8023
BlueBookPacket:

	call	recv_find		;ask for buffer from application es:di
	mov	ax,es			;is this pointer null?
	or	ax,di
	jz	too_bad			;yes - just free the frame.
	pop	si			;put rcv'd data in ds:si, di->si
	sub	si,EADDR_LEN + EADDR_LEN;we hand everything to the client
	pop	ds			;es->ds

	push	cx
	push	es			;for recv_copy(), es:di is user buf
	push	di			

	call	movemem			;do the copy (including ether header)

	pop	si			;di->si, es->ds
	pop	ds			;ds:si --> user buffer now
	pop	cx			;get back the length
	call	recv_copy

	pop	ds			;original ds restored
	jmp	clrup

too_bad:
	pop	di
	pop	es
	pop	ds
clrup:
	;either a bad or a good packet, we're done with it now
	pop	es			;good old dp seg
	pop	si
	mov	es:rd_stat[si],r_own	;desc reusable now
	ret
dp_rcv	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; eisr
; for lance error service routine
;
; entry - bx has status
; exit  - bx unchanged, ax,cx changed
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	eisr
eisr	proc	near
	test	bx,l_babl		;transmission timeout?
	jz	eisr1			;no
	call	count_out_err
eisr1:
	test	bx,l_cerr    
	jz	eisr2			;no
	call	count_out_err
eisr2:
	test	bx,l_miss		;missed packet?
	jz	eisr3
	call	count_out_err
eisr3:
	test	bx,l_merr		;bus timeout?
	jz	eisr4			;no
	call	count_out_err		;this is a fatal error
eisr4:
	ret
eisr	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; rcverror:
;	entry	ax has rd_stat
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	rcverror
rcverror	proc	near

	test	ax,r_fram		;framing error?
	jz	rerr0
	call 	count_in_err
rerr0:
	test	ax,r_crc		;crc error?
	jz	rerr1
	call 	count_in_err
rerr1:
	test	ax,r_oflo	  	;silo overflow?
	jz	rerr2
	call 	count_in_err
rerr2:
	test	ax,r_buff		;buffer error?
	jz	rerr3
	call 	count_in_err
rerr3:
	ret
rcverror	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; init_csr:
;
; initialize lance registers csr1, csr2, csr3
;  entry - csr0 stop bit must have been set
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	init_csr
init_csr	proc	near
	mov	al,csr1			;set rap to csr1
	call	set_rap
	mov	ax,i_mode		;init block starts at 0
	call	w_csr

	mov	al,csr2			;set rap to csr2
	call	set_rap
	mov	ax,0			;init block upper 8 bits are 0
	call	w_csr

	mov	al,csr3			;set rap for csr3
	call	set_rap
	mov	ax,bus_master		;bswp,acon,bcon
	call	w_csr

	mov	al,csr0			;leave rap at csr0
	call	set_rap
	ret
init_csr	endp	

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; r_csr:
;
; read lance csr already selected into ax
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	r_csr
r_csr	proc	near
	push	dx
	mov	dx,io_addr		;now data port
	add	dx,pt_ldata
	in	ax,dx
	pop	dx
	ret
r_csr	endp
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; r_csr0
;
; read lance csr0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	r_csr0
r_csr0	proc	near
	push	dx
	mov	al,csr0			;set rap to csr1
	call	set_rap
	mov	dx,io_addr		;now data port
	add	dx,pt_ldata
	in	ax,dx
	pop	dx
	ret
r_csr0	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set_initblk:  
;
; set up lance initialization block in the dual port  
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;				       ;

	public	set_initblk,si11
set_initblk  proc	near
	push	es
	call	set_dp			;get lance data seg

	mov	ax,run_mode		;mode
	mov	es:[i_mode],ax		;(es = dpseg);1st word in the dp
	lea	si,UNITID		;set in physical address
	mov	di,i_padr		; from the board
si11:
	mov	cx,6
	rep	movsb
;
;forget about the logical address filter for now --mz??
;
	mov	ax,RAREA		;set in pointer to 1ST receive desc.
	mov	es:[i_rdra],ax		;set lo addr (0-15)
	mov	ah,rlen			;set in rlen parameter & hi addr (00s)
	mov	al,0
	mov	es:[i_rlen],ax		;4 rcv descriptors
	mov	ax,XAREA		;pointer to xmit descriptors
	mov	es:[i_tdra],ax
	mov	ah,tlen			;set in tlen parameter 1 = 2
	mov	al,0
	mov 	es:[i_tlen],ax
	pop	es
	ret
set_initblk	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set_dp
;	es<-- cs:base_address
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	set_dp
set_dp:
	assume	cs:code
	push	ax
	mov	ax,cs:[base_addr]
	mov	es,ax
	pop	ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set_ds:
; 	ds<--cs
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
set_ds:
	push	ax
	mov	ax,cs
	mov	ds,ax
	pop	ax
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set up transmit message descriptors
;
; entry - cx has number of descriptors and buffers to set up
;         bx has pointer of start of descriptors
;         di has pointer to start of buffer area
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	su_tmd
su_tmd	proc	near
	push	es
	call	set_dp			;get lance data seg

	;first set the relative parameters
	mov	[sdesc_cnt],cx		;set in count
	mov	[actsdesc],bx

su_t0:
	mov	es:td_addr[bx],di	;set in pointer
	mov	ax,BUFSIZE		;total size
	neg	ax			;two's compliment
	or	ax,td_bmask		;set these bits
	mov	es:td_bcnt[bx],ax	;set in length to descriptor
	mov	ax,0			;set descriptor status for lance
	or	ax,t_stp		;start of packet
	or	ax,t_enp		;end of packet
	mov	es:td_stat[bx],ax	;give it away		
	mov	es:td_tdr[bx],0
	add	bx,tdesclen		;point at next descriptor
	add	di,BUFSIZE		;and next buffer
	loop	su_t0			;do all
	pop	es
	ret
su_tmd	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set up receive message descriptors
;
; entry - cx has number of descriptors and buffers to set up
;         bx has pointer of start of descriptors
;         di has pointer to start of buffer area
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	su_rmd
su_rmd	proc	near
	push	es
	call	set_dp	;get lance data seg

	mov	[actrdesc],bx  		;pointer to active descriptor
	mov	[rdesc_cnt],cx		;descriptor count
su_r0:
	mov	es:rd_addr[bx],di   	;set in pointer to pointer
	mov	ax,r_own		;set up descriptor status 
	mov	es:rd_stat[bx],ax
	xor	ax,ax			;zero out count
	mov	es:rd_mcnt[bx],ax
	mov	ax,BUFSIZE		;set in length of buffer
	neg	ax			;must be two's compliment
	or	ax,rd_mask		;need these bits set
	mov	es:rd_bcnt[bx],ax
	add	bx,rdesclen		;point at next descriptor
	add	di,BUFSIZE		;and next buffer
	loop	su_r0			;do all
	
	pop	es
	ret
su_rmd	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; lance_reset:
;
; 	reset lance chip; select csr0 and set the stop bit
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;		

	public	lance_reset
lance_reset	proc	near
	mov	dx,io_addr		;pop the reset line
	add	dx,pt_resetl	
	out	dx,al
	mov	al,csr0			;set lance address port to csr0
	call	set_rap
	mov	ax,l_stop 		;and set stop bit
	call	w_csr
	ret
lance_reset	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; w_csr:
;
; output ax to lance csr already selected
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	w_csr
w_csr	proc	near
	push	dx
	mov	dx,io_addr		;now data port
	add	dx,pt_ldata
	out	dx,ax
	pop	dx
	ret	
w_csr	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; set_rap:
;
; set lance address port to register in al
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	set_rap
set_rap	proc	near
	push	dx
	mov	dx,io_addr		;get base address
	add	dx,pt_laddr		        ;point at lance address port
	xor	ah,ah			;point at csr
	out	dx,ax
	pop	dx
	ret
set_rap		endp

; hardwre() and lance_init() are TSR just for the 
; support of reset_interface();

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; hardware: 
;	verify the port #;
;	set ethernet addr in _UNITID
;
; exit:
;	[_UNITID] = board ethernet address
;       al = 0 (ok)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	hardware
hardware	proc	near
	;verify the port #
	mov	dx,io_addr
	call	chkaddr
	cmp	al,0			;ok?
	jz	hw1			;yes, use #1
	mov	ax,offset porterr_msg
	ccall	_dispMSG,ax
	jmp	hw_error

hw1:
	;ok, now zero the ram out so that we can use it
	call	set_dp
	call 	zeroram

	;get ether address
	mov	cx,6			;get the ethernet address
	add	dx,pt_etaddr
	mov	bx,offset UNITID	;to here
hw3:
	in	al,dx
	mov	byte ptr[bx],al
	inc	bx
	loop	hw3

	;validate int_no (only support 10,11,12,15)
	
	mov	bl,IRQ10		;assume it's irq10
	mov	dx,io_addr		;port base
	add 	dx,pt_status		;status port
	in	al,dx			;get status
	and	al,st_mask		;get the 2 bits
	cmp	al,irqn_10		;(IS0 IS1)=(0 0) selects irq10
	je	alset
	inc	bl			;try irq11
	cmp	al,irqn_11
	je	alset
	inc	bl			;try irq12
	cmp	al,irqn_12
	je	alset
	mov	bl,15			;try irq15
	cmp	al,irqn_15
	jne	hw_error

alset:
	;did we get the right int_no in bl?
	cmp	bl,int_no
        jmp     ok                      ; #### temp patch, BKC
;	je	ok
	;otherwise, error
	mov	ax,offset interr_msg
	ccall	_dispMSG,ax
hw_error:
	mov	al,0ffh
	ret
ok:
	mov	al,0			;good return code
	ret
hardware	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; chkaddr: check if we are using the right i/o port
;          this is done by reading the port status and comparing
;	   with cs - 800h
;
; entry: 	io_addr == i/o port #
; exit: 	ah -- dual port segment
;		al -- 00h (succ) or ffh (error)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	chkaddr
chkaddr	proc	near
	push	dx

	mov	dx,io_addr
	in	al,dx			;read in status byte
	and	al,addr			;get dp seg, addr = 0f0h
	mov	ah,al			;save

	mov	dx,base_addr
	and	dh,addr
	cmp	al,dh
	je	chk2
	mov	al,0ffh			;error
	pop	dx
	ret
chk2:
	mov	al,0			;ok
	pop	dx
	ret
chkaddr	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; zeroram - zero dual port ram
;    seg in [dpseg]
;    registers: di,cx,al
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	zeroram
zeroram	proc	near
	cld				;auto increment
zram1:	
	mov	di,0			;start at 0;dpseg:di
	mov	cx,dplen		;dual port length
	mov	al,0			;data to go
	rep	stosb			;zero em all
	ret
zeroram	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; lance_init:
;
; init lance chip; first setup init_blk, r & s descriptors in dp
;		   then set the chip to go.
;	exit: ax = 1: error; 0 OK. 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	lance_init, il0, il1
lance_init	proc	near
	call	set_initblk		;set up lance initialization block

	;set up transmit descriptors & buffers
	mov	cx,NUMTDESC		;get no. of transmit blocks to build
	mov	bx,XAREA		;point at descriptor memory pool
	mov	di,TBAREA		;and buffer pool
	call	su_tmd			;set up transmit descriptors

	;set up receive descriptors & buffers
	mov	cx,NUMRDESC		;set up receive descriptors
	mov	bx,RAREA		;descriptor area
	mov	di,RBAREA		;buffer area
	call	su_rmd			;set up receive descriptors

	call	lance_reset		;ensure reset
	call	init_csr		;initialize lance registers
					;also set rap to csr0 with stop set
	;now start the lance initialization action
	CLI
	mov	ax,l_init+l_strt	;write init & start to csr0
	call	w_csr

	;rap = csr0 now, let's check at csr0
   	mov	cx,0
	dec	cx
il0:
	call	r_csr			;get lance status register
	test	ax,l_idon		;init done set?
	jnz	il1
	loop	il0
	;sth wrong with the chip, can't init
	mov	ax,offset chperr_msg
	ccall	_dispMSG,ax
	mov	ax,1			;error return
	ret
il1:
	mov	ax,l_idon+l_inea	;this drives /intr pin to low
	call	w_csr			;enable network ints
	STI
	mov	ax,0			;good return
	ret
lance_init	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;  tickscreen
;
;  For debug this routine can be called to tick over the screen at the 
;  specified location.
;
;	c callable:	tickscreen(loc);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	_tickscreen
_tickscreen	proc near
	push	bp
	mov	bp,sp
	save	<ax,bx,ds>
	mov	bx,04[bp]		;get screen position
	add	bx,bx			;*2
IFDEF COLOR
	mov	ax,0b800h
ELSE
	mov	ax,0b000h
ENDIF
	mov	ds,ax
	inc	byte ptr [bx]			;tick it
	restore	<ds,bx,ax>
	pop	bp
	ret
_tickscreen	endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; _dispMSG:
;	c call: dispMSG(&msg);
;	put the '$' terminated message to screen
;	entry: dx - point to the message
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	_dispMSG
_dispMSG proc near
	push	bp
	mov	bp,sp
	push	dx
	mov	dx,4[bp]

	push	ax
	mov	ah,9
	int	21h
	pop	ax

	pop	dx
	pop	bp
	ret
_dispMSG endp


	public	recv
recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
	assume	ds:code

recv_0:
	mov	dx,io_addr		;get the iobase
	add	dx,pt_status		;the read status port
	in	al,dx			;get board status

	test	al,clk_int		;look for clock interrupt
	jz	ni10			;no clock int
	mov	dx,base_addr
	add	dx,pt_clrclk		;reset the ckock
	out 	dx,al
ni10:
	test	al,net_int		;network int bit cleared?
	jnz	nisr5			;look for 0 here, set means none

	call	r_csr			;get lance status register -- in ax
	mov	bx,ax
	and	ax,l_mask	 	;clear out ints and mask any more
	call	w_csr

	test	bx,l_err		;check for errors first
	jz	nisr2			;no error
	call	eisr
nisr2:
	test	bx,l_rint		;test for receive active
	jz	nisr6			;no r_int
	call	risr
nisr6:
	mov 	ax,l_inea		;re-enable the int
	call	w_csr
	jmp	recv_0			;check again for ints
nisr5:
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
usage_msg	db	"usage: nti16 [-n] [-d] [-w] <packet_int_no> <irq_no> <port_no> <base_addr>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for nti network device, version ",'0'+majver,".",'0'+version,CR,LF
		db	"Copyright 1990, Michael Zheng",CR,LF,'$'


;enter with si -> argument string, di -> wword to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

	public	parse_args
parse_args:
	mov	di,offset int_no
	call	get_number
	mov	di,offset io_addr
	call	get_number
	mov	di,offset base_addr
	call	get_number
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; etopen:
;	initialize the interface in order to function
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	public	etopen
etopen:


	call 	hardware		;decide port #; _UNITID; zeroram
					;and 16-bit board specific:
					;decide the int_no validity --mz!!
	cmp	al,0
	jz	beg0			;ok
	mov	ax,offset hwerr_msg	;error return
	ccall	_dispMSG,ax
	mov	ax, 4c00H
	int	21h			;terminate it
beg0:

	mov	al,int_no
	call	maskint
	call	lance_init		;init the lance chip
	cmp	ax,1			;init error?
	jne	good			;ok

	mov	ax,offset initerr_msg	;can't init message
	ccall	_dispMSG,ax
	stc	
	ret

good:
	call	set_recv_isr

	mov	al, int_no		; Get board's interrupt vector
	add	al, 8
	cmp	al, 8+8			; Is it a slave 8259 interrupt?
	jb	set_int_num		; No.
	add	al, 70h - 8 - 8		; Map it to the real interrupt.
set_int_num:
	xor	ah, ah			; Clear high byte
	mov	int_num, ax		; Set parameter_list int num.

	mov	ax,offset good_msg
	ccall	_dispMSG,ax

	mov	dx,offset end_resident	;in paragraphs
	clc
	ret

	public	print_parameters
print_parameters:
;echo our command-line parameters
	mov	di,offset int_no
	mov	bx,offset int_no_name
	call	get_number
	mov	di,offset io_addr
	mov	bx,offset io_addr_name
	call	get_number
	mov	di,offset base_addr
	mov	bx,offset base_addr_name
	call	get_number
	ret

code	ends

	end
