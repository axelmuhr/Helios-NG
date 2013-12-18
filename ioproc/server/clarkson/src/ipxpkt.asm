version	equ	2

; Packet driver to simulate Ethernet on Novell IPX protocol.
;
; Paul Kranenburg
; Department of Computer Science
; University of Leiden
; Niels Bohrweg 1
; PO Box 9512
; 2300 RA Leiden
; The Netherlands
; e-mail: pk@cs.leidenuniv.nl
;
;
; File: ipxpkt.asm
;
;
; General description:
;
; Take destination from the Ethernet packet and feed it to IPX
; in the Event Control Block Immediate Address field.
;
; IPX packets are 576 bytes at most, 30 are needed for the IPX header
; leaving 546 bytes of user data. Another 4 bytes are used to describe
; fragments.
; If NO_OF_SND_BUFS is set to 1, this yields an MTU for this driver of 528.
; (546 - 4 - sizeof(Ether header)[=14]).
; If IPX trail is used another 32 bytes are lost (MTU = 496).
;
; If NO_OF_SND_BUFS is set to 3, the Ethernet packet is broken into at most
; 3 fragments. These are tagged with a Fragment id and shipped.
;
; On reception, fragments are kept on a linked list ordered by fragment number
; and keyed by source node address and fragment id.
; An IPX event is scheduled to allow for timeout of pending reassembly queues.
;
; If all fragments are reassembled, the client is called to provide a buffer for
; the packet.
;
; [ To save on buffer space, the driver could conceivably do with some minimum
;  number of buffers and call recv_find as soon as a fragment arrives, copy
;  the contents, and only call recv_copy when all fragments have arrived. However,
;  I don't think there is a way to notify the client in case a fragment gets lost.]
;
; In this code, the number of receive buffers (NO_OF_RCV_BUFS) has been set
; to 6 (a wild guess).
; This driver has yet to be tested in a gateway under heavy load. One probably
; needs more buffers in this case.
;
; Buffer space for the receive buffers is allocated after the "end_resident"
; label. There is a potential problem here: we start listening for packets
; using these buffers while still in the initialisation code, which is overlaid
; by the receive buffers. This is why interrupts are turned off wherever possible.
;
;
; CHANGELOG.
;
; 07/16/90
; Decoupled simulated ethernet address from IPX node address in routing tables,
; allowing for unique addresses in nets like ARCNET with only a one byte
; node address.
; Thanks to Robert Roll and Reid Sweatman of the University of Utah
; for pointing this out.
; IPXPKT accepts a command line option of the from `-n [<no_bytes>]' to specify
; the number of significant bytes in the IPX node address. If less then EADDR_LEN,
; the presented ethernet address will get supplemented with some bytes from
; the IPX node address to (hopefully) create a unique address among the nodes
; in the network involved.
; If `<no_bytes>' is omitted, the number of significant bytes will be set as the
; number of bytes left in the node address after stripping leading zeroes.
;
; 07/12/90
; Reorganize initialisation; do GET_ADDESS etc., before posting LISTEN's
;
; 05/16/90
; New socket number used when TRAIL enabled (0x6181).
; Enables co-existance of original and "TRAIL" versions of packet driver.
; You can start both an old and a new experimental driver in your gateway,
; but watch those IP subnet addresses.
;
; 05/15/90
; Corrected byte-order of IPX socket.
; Socket number NOT actually changed (== 0x6180, in dynamic range)
;
; 05/15/90
; Add dummy GET_LOCAL_TARGET call to force some traffic to IPX from a bridge.
; This will get the local net address to IPX. Define TRY_GET_LOCAL_TARGET if
; you want to use this.
;
; 05/07/90
; Add statistics gathering on various table lookups.
; Compile option STAT. Use ipxstat.c for display (derivative of version 5.0 `stat.c').
;
; 05/04/90
; Fixed case of register trashing in route code.
; Add IPX 32-byte trailer for bridge routing.
; Compile option TRAIL.
;
; 05/03/90
; Add routing table.
; Net/node addresses of incoming packets are put routing table along with
; immediate address field in ecb struct.
; Outgoing packets have the their net- and immediate address looked up
; in the table with the node address as key.
; Special case: broadcast packets are sent with packet type 0x14 through
; permanent entry in routing table.
;
; 05/03/90
; **REMOVED** 07/17/90
; Add compile option to declare receive buffer space at compile time.
; Compile option NOT_SO_SAVE; if defined, buffer space is allocated beginning at
; `end_resident' else space is reserved by compiler
;
; 05/02/90
; Merge `fill_ipxhdr' into `route'.
;

;DEBUG			EQU	1
TRY_GET_LOCAL_TARGET	EQU	1
STAT			EQU	1
TRAIL			EQU	1
ETH_CONSTR		EQU	3

	include	defs.asm

MAX_IPX_LEN	=	576		; Maximum packet size that can be
					; shipped through IPX
ifdef TRAIL
IP_socket	=	08161h		; Socket allocated for Blue book Ether
					; on IPX with TRAILS (BYTE-SWAPPED)
else
IP_socket	=	08061h		; Socket allocated for
					; Blue book Ether on IPX (BYTE-SWAPPED)
endif

PEP		=	4		; Packet Exchange Packet (ipx_type)
GBP		=	014h		; Global Broadcast Packet (ipx_type)
RTE_TICK	=	37		; Interval between calls to rte_ticker,
					; 37 is approx. 2 seconds

ipx_header	struc
ipx_chksum	dw	?		; Checksum, network byte order
ipx_len		dw	?		; Packet length,   "
ipx_prot	db	?		; Transport protocol
ipx_type	db	?		; Packet type
ipx_destnet	db	4 dup(?)	; Destination network
ipx_destnode	db	6 dup(?)	; Destination node
ipx_destsock	dw	?		; Destination socket
ipx_srcnet	db	4 dup(?)	; Source network
ipx_srcnode	db	6 dup(?)	; Source node
ipx_srcsock	dw	?		; Source socket
ifdef TRAIL
ipx_trail	db	8 * 4 dup(?)	; IPX gateway trail
endif
ipx_header	ends


frag_dscr	struc
frag_addr	dd	?		; Fragment address
frag_size	dw	?		; Fragment size
frag_dscr	ends

ecb		struc
ecb_link	dd	?		;
ecb_esr		dd	?		; Event Service Routine
ecb_inuse	db	?		; In Use field
ecb_cmplt	db	?		; Completion Code
ecb_sock	dw	?		; Socket Number
ecb_ipxwork	db	4 dup (?)	; IPX reserved workspace
ecb_drvwork	db	12 dup (?)	; Driver reserved workspace
ecb_ia		db	6 dup (?)	; Immediate Address
ecb_fragcnt	dw	?		; Fragment count
;ecb_dscr	=	$		; Start of Fragment descriptor list
ecb		ends

aes_ecb		struc
aes_link	dd	?		;
aes_esr		dd	?		; Event Service Routine
aes_inuse	db	?		; In Use field
aes_work	db	5 dup (?)	; Driver reserved workspace
aes_ecb		ends


ether_frag	struc
ef_fragno	db	?		; This fragment number
ef_fragtot	db	?		; Total number of fragments comprising the packet
ef_fragid	dw	?		; Fragment Id
ether_frag	ends

queue_entry	struc
q_aes		db (size aes_ecb) dup(?); AES structure, used for reassembly timeouts
q_filler	db	0
q_count		db	0		; Number of fragments currently queued here
q_net		db	SIZE ipx_srcnet dup(?)
q_node		db	SIZE ipx_srcnode dup(?)	; Source node
q_fragid	dw	?			; Fragment Id
q_len		dw	?			; Total length of user data queued here
q_ecb		dd	?			; Ecb pointer to fragment
queue_entry	ends

u_buf		struc
u_ecb		db (size ecb) dup(?)
u_ipx_frag	db (size frag_dscr) dup(?)
u_frag_frag	db (size frag_dscr) dup(?)
u_data_frag	db (size frag_dscr) dup(?)
u_ipx		db (size ipx_header) dup(?)
u_ether_frag	db (size ether_frag) dup(?)
;u_data		LABEL		BYTE
u_buf		ends

MAX_PAYLOAD	=	MAX_IPX_LEN - SIZE ipx_header - SIZE ether_frag

;routing table entry
rt_ent		struc
rt_ether	db	EADDR_LEN dup(?)	; Ethernet address of target: lookup key
rt_net		db	SIZE ipx_srcnet dup(?)	; Net address of target
rt_node		db	SIZE ipx_srcnode dup(?)	; Node address of target
rt_gate		db	SIZE ecb_ia dup(?)	; First hop on route to above
rt_x_pkt	db	?		; IPX packet type to send packet with
;;rt_trail	db	?		; This node uses IPX trail
rt_age		dw	?		; Usage indicator for this entry
rt_ent		ends

print$		macro	string
;---------------------------------------;
;  sends $ terminated string to screen  ;
;---------------------------------------;
		push	dx
		mov     ah,9
		mov     dx,offset &string&      ; print $ terminated string
		int     21h
		pop	dx
		endm

; ipx function numbers
OPEN_SOCKET		=	0
CLOSE_SOCKET		=	1
GET_LOCAL_TARGET	=	2
SEND_PACKET		=	3
LISTEN			=	4
SCHEDULE_EVENT		=	5
CANCEL_EVENT		=	6
SCHEDULE_SPECIAL_EVENT	=	7
GET_NODE_ADDRESS	=	9
RELINQUISH		=	0Ah

call_ipx	macro	opcode,reg1,reg2,reg3,reg4,reg5,reg6,reg7,reg8
		irp N, <reg1,reg2,reg3,reg4,reg5,reg6,reg7,reg8>
			ifnb <N>
				push	N
			endif
		endm
		mov	bx, opcode
;better be save here and use Code segment explicitly
		call	cs:IPXentry
		irp N, <reg8,reg7,reg6,reg5,reg4,reg3,reg2,reg1>
			ifnb <N>
				pop	N
			endif
		endm
		endm

repmov		macro	count
		rept	(count) / 2
		movsw
		endm
		rept	(count) MOD 2
		movsb
		endm
		endm

ifdef STAT
statinc		macro	loc
		local	a
;affects flags register (NOT carry)
		inc	cs:loc
		jnz	a
		inc	cs:loc+2
	a:
		endm
else
statinc		macro	loc
		endm
endif

code	segment	word public
	assume	cs:code, ds:code

IPXentry	dd	?
FragmentID	dw	?

my_net_address	db	4 dup(?)	; contiguous 10 byte addrss-area as IPX wants it
my_node_address	db	6 dup(?)

no_bytes	dw	EADDR_LEN, 0	; number of significant bytes in IPX node address
					; set as command line option.
init_cmplt	dw	0

dummy		db	10 dup(0ffh)	; dummy addr/socket
		dw	05104h		; what socket to use (???)
		db	6 dup(?)	; returned address


NO_OF_FRAGMENTS	=	3
NO_OF_RCV_BUFS	=	6
NO_OF_SND_BUFS	=	3
NO_OF_QUEUES	=	NO_OF_RCV_BUFS		; ????
NO_OF_RTES	=	30

reass_queues	queue_entry	NO_OF_QUEUES dup(<>)

rcv_bufs	u_buf		NO_OF_RCV_BUFS dup(<>)
snd_bufs	u_buf		NO_OF_SND_BUFS dup(<>)

ifdef STAT
;;		org	($ + 1) and 0fffeh
; keep ID string an even number of bytes (including terminating zero and count)
		db	"RTE_TABL", 0, NO_OF_RTES
endif
rte		rt_ent		NO_OF_RTES dup(<>)
rte_end		dw		0
rte_scache	dw		0
rte_rcache	dw		0
rte_aes		aes_ecb		<>

ifdef STAT
;;		org	($ + 1) and 0fffeh
		db	"IPXSTAT", 0	; keep this string an even number of bytes
queue_full	dw	2 dup(0)
route_drop	dw	2 dup(0)
scache_miss	dw	2 dup(0)
rcache_miss	dw	2 dup(0)
route_loops	dw	2 dup(0)
route_lookups	dw	2 dup(0)
endif

;-------------------------------------------------------------------------------
;
; local functions
;
; A NOTE on style:
;
; the functions below seem to liberally load and reload pointers into
; a register pair involving the ds segment register.
; In fact, ds almost always contains the code segment as "assumed" above.
; Also, the distinction between pointers to ecb's and ubuf's / queue's is not made
; most of the time. This is alright as long as the ecb structures remain the first
; ones declared in u_buf and queue.
; Need to work out a consistent register usage some day...
;

find_queue	proc	near
	;
	; Find/allocate a queue-entry where an ether fragment can be stored.
	; On entry: es:di -> source node address.
	;           dx == fragment Id.
	; Out: si == 0 if no queue entry available,
	;      otherwise: (ds:)si -> allocated queue-entry.
	; Must be called with interrupts disabled.

	push	cx
	push	bx
	mov	cx, NO_OF_QUEUES
	lea	si, reass_queues
	mov	bx, 0

fq_loop:
	mov	al, [si].q_count
	or	al, al
	jnz	fq_1
	or	bx, bx		;
	jne	fq_2		; remember first entry not in use
	mov	bx, si		;
	jmp	short fq_2

fq_1:
	mov	ax, cx		; can use AX in stead of `push cx'
	push	si
	push	di
	add	si, q_net
	mov	cx, SIZE q_net + SIZE q_node
	cld
	repe	cmpsb
	pop	di
	pop	si
	mov	cx, ax
	jne	fq_2
	cmp	dx, [si].q_fragid
	jne	fq_2
	jmp	short fq_x

fq_2:
	add	si, SIZE queue_entry
	loop	fq_loop

	mov	si, bx

ifdef STAT
	or	si, si
	jnz	fq_stat_1
	statinc	queue_full
fq_stat_1:
endif

fq_x:
	pop	bx
	pop	cx
	ret
find_queue	endp

enqueue		proc	near
	; Queue an etherpacket fragment on appropriate queue
	; On entry: es:si -> received ecb.
	;           cx = length of data in this fragment
	; Out: carry set if no space available.
	;      zero flag set if packet on queue complete.
	;      ds:si -> queue_entry on which fragment was queued.

	push	si
	push	es
	mov	ax, 0
	mov	es:[si].u_ecb.ecb_link.offs, ax		; clear link-field
	mov	es:[si].u_ecb.ecb_link.segm, ax
	mov	di, si					; es:di -> ecb
	mov	dx, es:[si].u_ether_frag.ef_fragid
	push	di
;	lea	di, es:[si].u_ipx.ipx_srcnode
	lea	di, es:[si].u_ipx.ipx_srcnet
	call	find_queue
	pop	di

	or	si, si
	jnz	enq_0
	add	sp, 4
	stc
	ret

enq_0:
	mov	dl, es:[di].u_ether_frag.ef_fragno
	mov	dh, es:[di].u_ether_frag.ef_fragtot
	cmp	[si].q_count, 0
	jne	enq_3

;this is the first fragment we receive
	call	rte_enter		; record their route
	pop	[si].q_ecb.segm
	pop	[si].q_ecb.offs
	mov	[si].q_len, cx
	mov	[si].q_count, 1
	cmp	dh, 1			;
	jne	enq_1			; short cut if fragment count == 1.
	clc
	ret

;initialise queue structure a bit more...
enq_1:
	mov	ax, es:[di].u_ether_frag.ef_fragid
	mov	[si].q_fragid, ax

;copy source node address
	mov	bx, SIZE q_net + SIZE q_node - 1

enq_2:
;	mov	al, es:[di+bx].u_ipx.ipx_srcnode
	mov	al, es:[di+bx].u_ipx.ipx_srcnet
	mov	ds:[si+bx].q_net, al
	sub	bx, 1
	jnc	enq_2

	mov	ax, cs
	mov	[si].q_aes.aes_esr.segm, ax
	mov	[si].q_aes.aes_esr.offs, offset reass_timeout
	mov	ax, ds
	mov	es, ax
	mov	ax, 2			; two ticks to timeout
	call_ipx	SCHEDULE_SPECIAL_EVENT,si,dx
	cmp	dh, [si].q_count
	clc
	ret

; add ecb to existing queue, keep list ordered by fragment number.
enq_3:
	lea	ax, [si].q_ecb
	push	ax			; put link field address on stack
	push	ds
	les	di, [si].q_ecb

enq_4:
	mov	ax, es			; are we at end of list?
	or	ax, di
	jz	enq_5
	cmp	dl, es:[di].u_ether_frag.ef_fragno	; link after this frag?
	jb	enq_5
	add	sp, 4
;	lea	ax, es:[di].u_ecb.ecb_link
;	push	ax
	push	di					; push `prev'-link
	push	es
	les	di, es:[di].u_ecb.ecb_link		; load `next'-link
	jmp	enq_4

; enter here with two addresses on the stack:
;	1) address of ecb to link in
;	2) address of link field after which to link
; es:di contains the "next" link.

enq_5:
	mov	ax, es					; temp = next
	mov	bx, di
	pop	es					; get prev
	pop	di
	pop	es:[di].segm				; prev->next = this one
	pop	es:[di].offs
	les	di, es:[di]				; load `this one'
	mov	es:[di].u_ecb.ecb_link.segm, ax		; `this one'->next = temp
	mov	es:[di].u_ecb.ecb_link.offs, bx
	add	[si].q_len, cx				; update total queued data
	inc	[si].q_count				; update fragcount
	cmp	dh, [si].q_count			; return `zero' if all there
	clc
	ret

enqueue		endp

dequeue		proc	near
	; Send reassembled packet to client and reschedule receive buffers.
	; On entry: ds:si -> queue.

	mov	cx, [si].q_len
	les	di, [si].q_ecb
	les	di, es:[di].u_data_frag.frag_addr
	add	di, 2 * EADDR_LEN

	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	inc	di			;set di to 802.2 header
	inc	di
	mov	dl, IEEE8023
BlueBookPacket:

	push	si
	call	recv_find
	pop	si
	mov	ax, es
	or	ax, di
	jz	deq_2

	mov	dh, [si].q_count
	mov	cx, [si].q_len
	push	si			; save our queue address
	push	ds
	push	di			; save their buffer address
	push	es
	push	cx
	lds	si, ds:[si].q_ecb
	cld

;all set, es:di -> user buffer, ds:si -> first fragment
;??? save count and source pointer for call to recv_copy

deq_1:
	mov	cx, ds:[si].u_ipx.ipx_len
	xchg	cl, ch
	sub	cx, (SIZE ipx_header + SIZE ether_frag)
	push	si
	push	ds
	lds	si, ds:[si].u_data_frag.frag_addr
	rep	movsb
	pop	ds
	pop	si
	lds	si, ds:[si].u_ecb.ecb_link
	dec	dh
	jnz	deq_1

	pop	cx			; recover packet length and address
	pop	ds			; for completion call
	pop	si			;
	call	recv_copy

	pop	ds			; recover queue address
	pop	si			;

deq_2:
	mov	ax, ds
	mov	es, ax
	call_ipx	CANCEL_EVENT,si

	push	si
	mov	dh, [si].q_count
	les	si, ds:[si].q_ecb

deq_3:
	mov	bx, es:[si].ecb_link.offs
	mov	cx, es:[si].ecb_link.segm
	call	listen_proc
	mov	si, bx
	mov	es, cx
;	les	si, es:[si].u_ecb.ecb_link
	dec	dh
	jnz	deq_3
	pop	si
	mov	[si].q_count, 0
	ret
dequeue		endp

reass_timeout	proc	far
	; Called by AES when reassembly timeout occurs.
	; On entry: es:si pointer to ecb.
	;

	push	ds
	mov	ax, cs
	mov	ds, ax
	push	si
	push	es
	mov	dh, es:[si].q_count
	les	si, es:[si].q_ecb

reass_to_3:
	mov	bx, es:[si].ecb_link.offs
	mov	cx, es:[si].ecb_link.segm
	call	listen_proc
	mov	si, bx
	mov	es, cx
	dec	dh
	jnz	reass_to_3

	pop	es
	pop	si
	mov	es:[si].q_count, 0

	pop	ds
	ret
reass_timeout	endp

receiver	proc	far
	;
	; On entry: es:si -> ecb.
	;

	push	ds
	mov	ax, cs
	mov	ds, ax
	mov	al, es:[si].u_ecb.ecb_cmplt
	or	al, al
	jnz	receiver_err

	cmp	es:[si].u_ecb.ecb_fragcnt, NO_OF_FRAGMENTS
	jne	receiver_err

;IPX receives its own broadcasts because
;source and destination sockets are the same
;if ours, ignore

	mov	ax, si
	mov	di, si
	add	di, ecb_ia
	lea	si, my_node_address
	mov	cx, SIZE my_node_address
	cld
	repe	cmpsb
	mov	si, ax
	jz	receiver_x

	mov	cx, es:[si].u_ipx.ipx_len
	xchg	cl, ch
	sub	cx, (SIZE ipx_header + SIZE ether_frag)
	jbe	receiver_err

	call	enqueue
	jc	receiver_err
	jnz	rec_1
	call	dequeue

rec_1:
	pop	ds
	cli
	ret

receiver_err:
	call	count_in_err

receiver_x:
	call	listen_proc		; post listen again
	pop	ds
	cli			; must return with interrupts disabled, says Novell.
	ret
receiver	endp

listen_proc	proc	near
	;
	; Post to u_buf for reception.
	; On entry: es:si -> receive-ecb
	;

	push	bx

;fill in ecb
	mov	es:[si].u_ecb.ecb_esr.offs, offset receiver
	mov	ax, cs
	mov	word ptr es:[si].u_ecb.ecb_esr.segm, ax
	mov	es:[si].u_ecb.ecb_sock, IP_socket
	call_ipx	LISTEN,es,si,di,dx,cx

	pop	bx
	ret

listen_proc	endp

rte_ticker	proc	far
	;
	; ESR service routine called by AES
	; Ages all entries in routing table
	; executes entirely with disabled interrupts
	;
	; On entry: es:si -> ecb

	push	ds
	mov	ax, cs
	mov	ds, ax

	mov	dx, rte_end
	lea	di, rte

rtick_1:
	add	di, SIZE rt_ent		; leave broadcast entry alone
	cmp	di, dx
	jae	rtick_done
	mov	ax, [di].rt_age
	add	ax, 1
	jo	rtick_1
	mov	[di].rt_age, ax
	jmp	rtick_1

;re-schedule ecb
rtick_done:
	mov	ax, RTE_TICK
	call_ipx	SCHEDULE_SPECIAL_EVENT
	pop	ds
	ret
rte_ticker	endp

rte_enter	proc	near
	;
	; Enter route to table
	; On entry: es:di -> ecb
	;
	assume	ds:nothing, es:nothing

	push	bx
	push	cx
	push	dx
	push	si
	push	ds
	push	di
	push	es

	les	di, es:[di].u_data_frag.frag_addr
	add	di, EADDR_LEN			; es:di -> src node address
	mov	bx, rte_rcache			; global, last succesful entry
	call	rte_find			; will set DS to code seg
	jc	ert_1

ifdef STAT
	cmp	rte_rcache, si
	je	ert_stat_1
	statinc	rcache_miss
ert_stat_1:
endif

	mov	rte_rcache, si
	jmp	ert_x				; we already have route to src

ert_1:
;not in table, find free entry
;	mov	ax, cs
;	mov	ds, ax				; ds == code

	lea	dx, rte
	add	dx, SIZE rte			; dx -> beyond rte
	mov	bx, rte_end			;
	add	rte_end, SIZE rt_ent		; take the chance
	cmp	bx, dx				; check whether table full
	jb	ert_2
	sub	rte_end, SIZE rt_ent		; undo guess
	call	kill_route			; will leave freed entry in bx

ert_2:
;insert addresses in ecb and ipx into routing table
; $%#@, must swap registers for string operations
;
	cld
	mov	ax, ds
	mov	cx, es
	mov	es, ax
	mov	ds, cx
	mov	si, di				; ds:si -> source address
	mov	di, bx				; es:di -> rt_ent

	add	di, rt_ether

	repmov  <SIZE rt_ether>

	pop     ds				; recover ecb from stack
	pop     si				;
	push    si
	push    ds

	mov	ax, si

	add	si, u_ipx + ipx_srcnet		; record source net+node
	repmov	<SIZE rt_net + SIZE rt_node>

	mov	si, ax				; restore si to start of ecb
	add	si, ecb_ia			; record immediate address
	repmov	<SIZE rt_gate>

	mov	es:[bx].rt_x_pkt, PEP		; packet type
	mov	es:[bx].rt_age, 10		; usage count (XXX)

ert_x:
	pop	es
	pop	di
	pop	ds
	pop	si
	pop	dx
	pop	cx
	pop	bx
	ret
rte_enter	endp

kill_route	proc	near
	;
	; Delete entry in route table with highest rt_age field
	; Out: bx -> route entry freed
	;

	push	cx
	push	dx
	push	di
	lea	di, rte
	add	di, SIZE rt_ent		; leave broadcast entry alone

	mov	dx, rte_end
	mov	bx, di
	mov	cx, 0

krt_1:
	cmp	di, dx
	je	krt_done
	mov	ax, [di].rt_age
	cmp	ax, cx
	jbe	krt_2
	mov	cx, ax
	mov	bx, di

krt_2:
	add	di, SIZE rt_ent
	jmp	krt_1

krt_done:
ifdef STAT
	statinc	route_drop
endif
	pop	di
	pop	dx
	pop	cx
	ret
kill_route	endp


rte_find	proc	near
	;
	; Find a route for address
	; On entry: es:di -> target address we are looking for (EADDR_LEN bytes)
	;           bx -> entry to start search
	; Out: ds:si -> route table entry, or carry set if not found
	;
	assume	ds:code, es:nothing

	push	bx
	push	dx
	mov	ax, cs		; cs == code segment
	mov	ds, ax

	mov	dx, rte_end	; global, points to first invalid rte entry
	mov	ax, bx		; ax == stopper
	cld

frt_1:
ifdef STAT
	statinc	route_loops
endif
	mov	si, bx		;
	add	si, rt_ether	; si -> rt_ether field (= key) of current rte-entry
	push	di		; save di
	mov	cx, SIZE rt_ether
	repe	cmpsb
	pop	di
	je	frt_x		; compare ok, report succes
	add	bx, SIZE rt_ent	; next rte-entry
	cmp	bx, dx
	jb	frt_2
	lea	bx, rte		; end of valid entries, wrap around
frt_2:
	cmp	bx, ax
	jne	frt_1
	stc			; back where we started, report failure

;found, update cache and prepare output
frt_x:
	mov	si, bx

ifdef STAT
	statinc	route_lookups
endif
	pop	dx
	pop	bx
	ret
rte_find	endp

route	proc	near
	;
	; Determine where to send the packet
	; On entry: ds:si -> user data, es:di -> ecb
	;
	assume	ds:nothing, es:nothing

	push	bx
	push	cx
	push	ds
	push	si

;find entry in routing table, in: es:di, out: ds:si -> pointer to table entry

	push	di
	push	es
	mov	ax, ds			;
	mov	es, ax			; es:di -> target address
	mov	di, si			;

	mov	bx, rte_scache		; global, last succesful entry
	call	rte_find
	pop	es
	pop	di
	jc	route_x

ifdef STAT
	cmp	rte_scache, si
	je	route_stat_1
	statinc	scache_miss
route_stat_1:
endif

	mov	rte_scache, si		; remember this entry
	mov	ax, ds:[si].rt_age	;
	sub	ax, 1			; decrement usage
	jc	route_1
	mov	ds:[si].rt_age, ax

route_1:
	cld
	mov	bx, di			; remember ecb in BX

ifdef TRAIL
;clear ipx trail fields
	add	di, u_ipx.ipx_trail
	mov	ax, 0
	mov	cx, (SIZE ipx_trail) / 2
	rep	stosw

	mov	di, bx
endif

;fill in packet type and destination socket
	mov	al, ds:[si].rt_x_pkt
	mov	es:[di].u_ipx.ipx_type, al	;PEP
	mov	es:[di].u_ipx.ipx_destsock, IP_socket

;fill in full destination adress
	mov	ax, si			; save si
	add	si, rt_net
	add	di, u_ipx.ipx_destnet
	repmov	<SIZE ipx_destnet + SIZE ipx_destnode>
	mov	si, ax			; restore si, di
	mov	di, bx			;

;fill in immediate address
	add	di, ecb_ia
	add	si, rt_gate
	repmov	<SIZE ecb_ia>
	mov	di, bx			;

route_x:
	pop	si
	pop	ds
	pop	cx
	pop	bx
	ret
route	endp

	public	int_no
int_no	db	0,0,0,0			;must be four bytes long for get_number.

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type	db	1		;from the packet spec
driver_name	db	'IPX',0		;name of the driver.
driver_function	db	2
parameter_list	label	byte
	db	1	;major rev of packet driver
	db	9	;minor rev of packet driver
	db	14	;length of parameter list
	db	EADDR_LEN	;length of MAC-layer address
  if NO_OF_SND_BUFS eq 1
	dw	MAX_PAY_LOAD - 2 * EADDR_LEN - 2	;MTU, including MAC headers
  else
	dw	GIANT	;MTU, including MAC headers
  endif
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
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
	assume	ds:nothing
	push	es
	push	di
	mov	ax, cs
	mov	es, ax

;first, compute number of fragments needed, keep in dx
	mov	dx, 0
	mov	ax, cx

snd_1:
	inc	dx
	sub	ax, MAX_PAYLOAD
	jnc	snd_1

;can we handle this amount?
	cmp	dx, NO_OF_SND_BUFS
	jbe	snd_frags_ok

snd_err:
	call	count_out_err
	pop	di
	pop	es
	mov	dh, CANT_SEND
	stc
	ret

snd_frags_ok:
	lea	di, snd_bufs
	push	cx
	mov	cx, dx
	mov	bx, 0
	mov	al, 0

snd_free_chk:
	or	al, es:[di+bx].u_ecb.ecb_inuse
	add	bx, SIZE u_buf
	loop	snd_free_chk

	pop	cx
	or	al, al
	jnz	snd_err

	mov	dh, dl
	mov	dl, 1
	mov	bx, 0
	inc	FragmentID
	push	di

snd_next_frag:
;
; dh = total number of fragments to send
; dl = current fragment
; bx = offset into client buffer (ds:si) for this fragment
; cx = bytes to go
; es:di = address of current fragment's ecb
;

;determine next hop

	call	route		; XXX, should be done for first fragment only!
	jnc	snd_frag1
	pop	di
	jmp	snd_err

snd_frag1:
;fill in ecb
	mov	ax, 0
	mov	es:[di].u_ecb.ecb_esr.offs, ax
	mov	es:[di].u_ecb.ecb_esr.segm, ax

	mov	es:[di].u_ecb.ecb_sock, IP_socket

	mov	es:[di].u_ether_frag.ef_fragtot, dh
	mov	es:[di].u_ether_frag.ef_fragno, dl
	mov	ax, FragmentID
	mov	es:[di].u_ether_frag.ef_fragid, ax

	mov	ax, ds
	mov	es:[di].u_data_frag.frag_addr.segm, ax

	mov	ax, MAX_PAYLOAD
	sub	cx, ax
	jnc	snd_frag2
	add	ax, cx

snd_frag2:
	mov	es:[di].u_data_frag.frag_size, ax
	push	si
	add	si, bx
	mov	es:[di].u_data_frag.frag_addr.offs, si
	add	bx, ax

;
; es:si -> ecb to ship
;

	mov	si, di
	call_ipx	SEND_PACKET,es,di,dx,cx,bx
	pop	si	; ds:si -> client buffer once more

	add	di, SIZE u_buf	; next send buffer
	inc	dl
	cmp	dl, dh
	jbe	snd_next_frag

	pop	di

;simple timeout on sends
	mov	cx, 0ffffh

snd_wait:
;wait until sends are done
	sti
	mov	bx, 0
	push	cx
	mov	ch, 0
	mov	cl, dh		; dh still has fragment count
	mov	al, 0
snd_wait1:
	or	al, es:[di+bx].u_ecb.ecb_inuse
	add	bx, SIZE u_buf
	loop	snd_wait1
	pop	cx

	or	al, al
	jz	snd_done
	call_ipx	RELINQUISH,es,di,dx,cx
	loop	snd_wait

;arrive here on timeout, cancel IPX sends
	mov	ch, 0
	mov	cl, dh
	mov	si, di

snd_cancel:
	call_ipx	CANCEL_EVENT,es,si,cx
	add	si, SIZE u_buf
	loop	snd_cancel
	jmp	snd_err

snd_done:
;check completion status of send buffers
	mov	bx, 0
	mov	ch, 0
	mov	cl, dh
	mov	al, 0
snd_done1:
	or	al, es:[di+bx].u_ecb.ecb_cmplt
	add	bx, SIZE u_buf
	loop	snd_done1

	or	al, al
	jz	snd_ok
	jmp	snd_err

snd_ok:
	pop	di
	pop	es
	ret

	public	get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
	assume	ds:code
	cmp	cx, EADDR_LEN
	jnb	ga_0

	stc
	ret

ga_0:
	push	si
	cmp	init_cmplt, 1
	jnc	ga_1
	call	listen_init
	mov	init_cmplt, 1

ga_1:
	lea	si, my_node_address	; first copy IPX node address
	cld
	repmov	<EADDR_LEN>

	mov	cx, EADDR_LEN		; if not significant enough,
	cmp	no_bytes, 0ffffh	; magic in no_bytes?
	jne	ga_3
	lea	bx, my_node_address	; they don't know
	mov	dx, bx			; figure out something for no_bytes
	add	dx, SIZE my_node_address	; savety
	mov	no_bytes, EADDR_LEN

ga_2:
	cmp	word ptr [bx], 0
	jnz	ga_3
	dec	no_bytes
	inc	bx
	cmp	bx, dx
	je	ga_3
	jmp	ga_2

ga_3:
	sub	cx, no_bytes		; copy some of IPX net address
	jcxz	get_addr_x
	cmp	cx, SIZE my_net_address
	jbe	ga_4
	mov	cx, SIZE my_net_address
ga_4:
	lea	si, my_net_address
	sub	di, EADDR_LEN

;next are three different methods for constructing the reported ethernet address
;from a less-than-6-bytes-long IPX node adress and the IPX net address.
if ETH_CONSTR eq 1

	rep	movsb

elseif ETH_CONSTR eq 2

ga_21:
	mov	bx, SIZE my_net_address - 1
	mov	al, [si+bx]
	stosb
	dec	bx
	loop	ga_21

elseif ETH_CONSTR eq 3

	push	di
	push	si
	push	es
	mov	ax, ds
	mov	es, ax
	lea	di, dummy
	repmov	<SIZE my_net_address>
	pop	es
	pop	si
	pop	di

	lea	si, dummy

ga_3_loop:
	mov	bx, 0
	mov	dx, 0
	mov	al, 0
ga_31:
	cmp	bx, SIZE my_net_address
	jz	ga_33
	mov	ah, [si+bx]
	cmp	al, ah
	jnc	ga_32
	mov	al, ah
	mov	dx, bx
ga_32:
	inc	bx
	jmp	ga_31
ga_33:
	stosb
	mov	bx, dx
	mov	word ptr [si+bx], 0
	loop	ga_3_loop
	
endif


get_addr_x:
	mov	cx, EADDR_LEN
	pop	si
	clc
	ret

listen_init	proc	near
;and start listening on each receive buffer
	push	si
	push	cx
	push	es
	mov	ax, cs
	mov	es, ax
	mov	cx, NO_OF_RCV_BUFS
	lea	si, rcv_bufs
li_1:
	call	listen_proc
	add	si, SIZE u_buf
	loop	li_1
	pop	es
	pop	cx
	pop	si
	ret
listen_init	endp


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
;enter with es:di ->list of multicast addresses, cx = number of bytes.
;return nc if we set all of them, or cy,dh=error if we didn't.
	mov	dh,NO_MULTICAST
	stc
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
	ret


	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret

	public	terminate
terminate:
;called when this driver should cease operation.
	assume	ds:nothing

;close socket, outstanding listens should be cancelled automatically
	mov	dx, IP_socket
	call_ipx	CLOSE_SOCKET

;	mov	ax, cs
;	mov	es, ax
;	mov	cx, NO_OF_RCV_BUFS
;	lea	si, rcv_bufs
;
;terminate_1:
;	call_ipx	CANCEL_EVENT,es,si,cx
;	add	si, SIZE u_buf
;	loop	terminate_1

	ret


;any code after this will not be kept after initialization.
end_resident	label	byte


	public	usage_msg
usage_msg	db	"usage: ipx_pkt [-n] [-d] [-w] <packet_int_no>",CR,LF,'$'

	public	copyright_msg
copyright_msg	db	"Packet driver for IPX, version ",'0'+majver,".",'0'+version,CR,LF
		db	"Portions Copyright 1990, P. Kranenburg",CR,LF,'$'

no_ipx_msg	db	"IPX not present",CR,LF, '$'
wrong_sock_msg	db	"IPX has no good socket",CR,LF, '$'
ifdef DEBUG
debugmsg1	db	"Doing a GET TARGET",CR,LF, '$'
debugmsg2	db	"Past GET TARGET",CR,LF, '$'
debugmsg3	db	"Past GET ADDRESS",CR,LF, '$'
endif

	extrn	set_recv_isr: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
	extrn	get_number: near
	extrn	skip_blanks: near

	public	parse_args
parse_args:

	call	skip_blanks
	cmp	byte ptr [si], CR
	je	parse_args_x
	cmp	byte ptr [si], '-'
	jne	parse_err
	cmp	byte ptr [si+1],'n'	; '-n'?
	je	parse_args_1

parse_err:
	stc				;no, must be an error.
	ret

parse_args_1:
	add	si, 2
	call	skip_blanks
	cmp	byte ptr [si], CR
	jne	parse_args_2
	mov	no_bytes, 0ffffh	;try heuristic
	jmp	parse_args_x

parse_args_2:
	mov	di,offset no_bytes
	call	get_number
	cmp	no_bytes, 0
	jb	parse_err
	cmp	no_bytes, EADDR_LEN
	ja	parse_err

parse_args_x:
	clc
	ret


	public	etopen
etopen:

;first see if IPX is there
	mov	ax, 07A00h
	int	2fh
	cmp	al, 0ffh
	je	ipx_is_here
	print$	no_ipx_msg
	stc
	ret

ipx_is_here:
	mov	ax, es
	mov	IPXentry.offs, di
	mov	IPXentry.segm, ax

;close socket first, since "head" won't notify us on termination
	mov	dx, IP_socket
	call_ipx	CLOSE_SOCKET

;next open socket
	mov	al, 0ffh		; stay open until explicitly closed
	mov	dx, IP_socket
	call_ipx	OPEN_SOCKET
	or	al, 0
	jnz	wrong_socket
	cmp	dx, IP_socket
	je	good_socket

;close socket and exit
wrong_socket:
	call_ipx	CLOSE_SOCKET
	print$	wrong_sock_msg
	stc
	ret

good_socket:
;init send buffer fragment list
	mov	ax, cs
	mov	es, ax

	mov	cx, NO_OF_SND_BUFS
	lea	si, snd_bufs

et_1:
	mov	es:[si].u_ecb.ecb_fragcnt, NO_OF_FRAGMENTS

	mov	bx, si			; bx = offset ipx_header
	add	bx, u_ipx
	mov	es:[si].u_ipx_frag.frag_addr.offs, bx
	mov	es:[si].u_ipx_frag.frag_addr.segm, ax
	mov	es:[si].u_ipx_frag.frag_size, SIZE ipx_header

	mov	bx, si			; bx = offset ether_frag
	add	bx, u_ether_frag
	mov	es:[si].u_frag_frag.frag_addr.offs, bx
	mov	es:[si].u_frag_frag.frag_addr.segm, ax
	mov	es:[si].u_frag_frag.frag_size, SIZE ether_frag
;
; NOTE: u_data_frag is initialised send_pkt with address of user data buffer
;
	add	si, SIZE u_buf
	loop	et_1

;initialise receive buffer ipx data fragment addresses
;and start listening on each
	mov	cx, NO_OF_RCV_BUFS
	lea	si, rcv_bufs
	lea	di, end_resident

et_2:
	mov	es:[si].u_ecb.ecb_fragcnt, NO_OF_FRAGMENTS

	mov	bx, si
	add	bx, u_ipx
	mov	es:[si].u_ipx_frag.frag_addr.offs, bx
	mov	es:[si].u_ipx_frag.frag_addr.segm, ax
	mov	es:[si].u_ipx_frag.frag_size, SIZE ipx_header

	mov	bx, si
	add	bx, u_ether_frag
	mov	es:[si].u_frag_frag.frag_addr.offs, bx
	mov	es:[si].u_frag_frag.frag_addr.segm, ax
	mov	es:[si].u_frag_frag.frag_size, SIZE ether_frag

	mov	es:[si].u_data_frag.frag_addr.offs, di		; di = offset data buffer
	mov	es:[si].u_data_frag.frag_addr.segm, ax
	mov	es:[si].u_data_frag.frag_size, MAX_PAYLOAD

	add	si, SIZE u_buf
	add	di, MAX_PAYLOAD
	loop	et_2

;heuristic to force network traffic which seems to necessary before
;the GET_NODE_ADDRESS call will work properly (boo,hiss)

;get our address
	sti
	lea	si, my_net_address
	call_ipx	GET_NODE_ADDRESS,es,si
	lea	si, my_net_address
	cmp	byte ptr es:[si], 0
	jne	et_3
	cmp	byte ptr es:[si+1], 0
	jne	et_3
	cmp	byte ptr es:[si+2], 0
	jne	et_3
	cmp	byte ptr es:[si+3], 0
	jne	et_3

ifdef TRY_GET_LOCAL_TARGET
ifdef DEBUG
	push	es
	print$	debugmsg1
	pop	es
endif
	lea	si, dummy
	mov	di, si
	add	di, 12
	call_ipx	GET_LOCAL_TARGET,es
ifdef DEBUG
	push	es
	print$	debugmsg2
	pop	es
endif

;get our address
	lea	si, my_net_address
	call_ipx	GET_NODE_ADDRESS,es,si
ifdef DEBUG
	push	es
	push	si
	print$	debugmsg3
	pop	si
	pop	es
endif
endif

et_3:
;initialise routing table with the broadcast address
	cld
	lea	di, rte
	mov	rte_end, di			; rte_end = second entry
	add	rte_end, SIZE rt_ent		;
	mov	rte_scache, di			; rte_cache = first (and only) entry
	mov	rte_rcache, di			; 
	mov	es:[di].rt_age, 0
	mov	es:[di].rt_x_pkt, GBP		; broadcast packet type
;;	mov	es:[di].rt_trail, 1		; uses trail

	add	di, rt_ether			; broadcast address
	mov	al, 0ffh
	mov	cx, SIZE rt_ether
	rep	stosb

	push	ds
	mov	ax, cs
	mov	ds, ax
	mov	cx, SIZE rt_net
	rep	movsb				; net field set from my_net_address
	pop	ds

	mov	al, 0ffh
	mov	cx, SIZE rt_node + SIZE rt_gate
	rep	stosb				; all FF's

;schedule periodic aging of route table entries
	lea	si, rte_aes
	mov	ax, cs
	mov	es:[si].aes_esr.segm, ax
	mov	es:[si].aes_esr.offs, offset rte_ticker
	mov	ax, RTE_TICK
	call_ipx	SCHEDULE_SPECIAL_EVENT, es


;if all is okay,
	mov	dx,offset end_resident
	add	dx, NO_OF_RCV_BUFS * MAX_PAYLOAD
	clc
	ret

;if we got an error,
	stc
	ret

	public	print_parameters
print_parameters:
	ret

code	ends

	end
