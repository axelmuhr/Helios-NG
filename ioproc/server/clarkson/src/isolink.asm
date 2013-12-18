	page 54,132
; Packet driver for BICC Data Networks' ISOLINK 4110-2 ethernet
; controller, written by
;       Rainer Toebbicke
;       European Organisation of Nuclear Research (CERN)
;       Geneva, Switzerland
; based on the "generic" packet driver by Russell Nelson.

version equ     1               ;this is the minor version

        include defs.asm        ;SEE ENCLOSED COPYRIGHT MESSAGE



; BICC ISOLINK card constants



        .286c                   ; we are at least on a 80286!


code    segment para public
        assume  cs:code, ds:code
        public  begin   ;makes us appear in the link map
begin   equ     $       ;used for alignment operations below

;Lance initialisation block, must be on word boundary

IBmode          dw      00h             ;mode word
IBpadr          db      6 dup (0)       ;physical addr
IBladrf         db      8 dup (0ffh)    ;logical addr filter
IBrdraL         dw      0               ;Receive Descr Ring ptr
IBrdraH         dw      0
IBrdraHF        equ     byte ptr IBrdraH+1
IBtdraL         dw      0               ;Transmit Descr Ring ptr
IBtdraH         dw      0
IBtdraHF        equ     byte ptr IBtdraH+1


CSR0            equ     0
c0_FIX          equ     0000h
c0_ERR          equ     8000h
c0_BABL         equ     4000h
c0_CERR         equ     2000h
c0_MISS         equ     1000h
c0_MERR         equ     0800h
c0_RINT         equ     0400h
c0_TINT         equ     0200h
c0_ErrClear     equ     c0_BABL+c0_CERR+c0_MISS+c0_MERR

c0_IDON         equ     0100h
c0_INTR         equ     0080h
c0_INEA         equ     0040h
c0_RXON         equ     0020h
c0_TXON         equ     0010h
c0_TDMD         equ     0008h
c0_STOP         equ     0004h
c0_STRT         equ     0002h
c0_INIT         equ     0001h


CSR1            equ     1
CSR2            equ     2
CSR3            equ     3



RD              struc                   ;Receive descriptor
RBadrL          dw      0
RBadrH          dw      0
RBbcnt          dw      0               ;buffer size
RBmcnt          dw      0               ;packet size
RD              ends

RBflags         equ     byte ptr RBadrH+1
RBown           equ     080h            ;1=owned by Lance, 0=by host
RBerr           equ     040h            ;error summary bit
RBfram          equ     020h
RBoflo          equ     010h
RBcrc           equ     008h
RBbuff          equ     004h
RBstp           equ     002h            ;start of packet
RBenp           equ     001h            ;end of packet

TD              struc                   ;Transmit descriptor
TBadrL          dw      0
TBadrH          dw      0
TBbcnt          dw      0               ;buffer size
TBtdr           dw      0               ;more flags
TD              ends

TBflags         equ     byte ptr TBadrH+1
TBown           equ     080h            ;1=owned by Lance, 0=by host
TBerr           equ     040h            ;error summary bit
TBstp           equ     002h            ;Start of packet
TBenp           equ     001h            ;End of packet
TBbcntF         equ     byte ptr TBbcnt+1


        public  int_no
int_no  dw      10,0                     ;must be four bytes long for get_number.

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class	db	BLUEBOOK, IEEE8023, 0		;from the packet spec
driver_type     db      5               ;from the packet spec
driver_name     db      'ISOLINK',0     ;name of the driver.
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

	extrn	sys_features: byte

        public  rcv_modes
rcv_modes       dw      7         ;number of receive modes in our table.
                dw      0
                dw      rcv_mode_1
                dw      0
                dw      rcv_mode_3
                dw      0,0
                dw      rcv_mode_6



rbfstart        dw      FirstDescr      ;1st Rcv buffer descr
rbfcurr         dw      ?
tbfstart        dw      ?               ;1st Xmit buffer descr
tbfcurr         dw      ?
tbfend          dw      ?               ;end of Xmit buffers

xmt_buffsz_r    dw      ?               ;xmit buffer sizes
xmt_buffsz_rn   dw      ?               ;same, but times -1
pklen           dw      ?               ;length of packet
pklen_rem       dw      ?               ;remaining to be received
rcv_csr0        dw      ?
rcvbuffp        dd      ?


RAP             dw      0               ;Register Address Port
RDP             dw      0               ;Register Data Port

options         db      0
options_HMA     equ     40h             ;running in HMA

word_16         dw      16


                extrn   count_out_err:near, count_in_err:near


seg2lin         proc    near
; convert (ax:dx=offset:segment) to linear address (ax:dx=low:high)
                push    bx
                push    ax              ;save offset
                mov     ax,dx
                mul     word_16         ;segment to linear address
                pop     bx
                add     ax,bx           ;plus offset
                adc     dx,0
                pop     bx              ;restore
                ret
seg2lin         endp


lin2seg         proc    near
; convert linear addr (ax:dx=low:high) to (ax:dx=offset:segment)
; with minimal offset to avoid wrap-around problems

; the 80286 can address 0-10ffefh in real mode:
                cmp     dl,010h         ;over 1 Megabyte?
                je      l2sHMA          ;yes, segment is 0FFFFh
                push    ax
                shr     ax,4            ;convert to paragraphs
                shl     dx,4+8
                or      dx,ax           ;segment ok
                pop     ax
                and     ax,0fh          ;offset
                ret


l2sHMA:
; effectively subtract 0ffff0h from the linear address to form the
; offset:
                mov     dx,0ffffh       ;this we know already
                sub     ax,0fff0h       ;only have to do low order word
                ret
lin2seg         endp


; write bx to Lance control & status reg [ax]
wrcsr0          proc    near
                xor     ax,ax           ;write to CSR0
wrcsr:                                  ;CSR in ax
                mov     dx,RAP          ;address CSR
                out     dx,ax
                mov     dx,RDP          ;data port
                mov     ax,bx
                out     dx,ax
                ret
wrcsr0          endp


; read Lance control reg [ax]
rdcsr0          proc    near
                xor     ax,ax           ;read CSR 0
rdcsr:                                  ;CSR in ax
                mov     dx,RAP
                out     dx,ax
                mov     dx,RDP
                in      ax,dx
                ret
rdcsr0          endp


rcv_nxt_d       proc    near
; release current and advance to next receive descriptor
                mov     RBflags[bx],RBown       ;give buffer to Lance
                add     bx,8
                cmp     bx,tbfstart     ;end of ring?
                jb      rcv_nxt_ok      ;no...
                mov     bx,rbfstart     ;restart at first descriptor
rcv_nxt_ok:
                mov     rbfcurr,bx
                ret
rcv_nxt_d       endp


xmt_nxt_d       proc    near
xmt_nxt_d       endp


wait_own_0      proc    near
; wait for "own" bit in descriptor [bx] to clear
                test    TBflags[bx],TBown
                jnz     wo_wait                 ;no, have to wait
                ret

wo_wait:
                push    cx                      ;save reg
                mov     cx,0ffffh
wo_lp1:
                test    TBflags[bx],TBown
                jz      wo_clear                ;ok...
                loop    wo_lp1

wo_clear:
                pop     cx
                ret
wait_own_0      endp


rcv_getaddr     proc    near
; obtain buffer addr from descriptor [bx]
                mov     ax,RBadrL[bx]
                mov     dx,RBadrH[bx]
                call    lin2seg
                mov     word ptr rcvbuffp,ax
                mov     word ptr rcvbuffp+2,dx
                ret
rcv_getaddr     endp

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
send_pkt        proc    near
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
        assume  ds:nothing
                push    ds              ;save packet segment
                push    cs
                pop     ds
                assume  ds:code

; packet must be at least 64 bytes long (with fcs)...
                cmp     cx,RUNT
                jnl     send_L_ok
                mov     cl,RUNT
send_L_ok:

; get next buffer descriptor
                mov     bx,tbfcurr
                call    wait_own_0      ;wait until it's free

; If the next buffer is big enough to hold the packet, we will
; copy the packet and send it out.
; Otherwise we send it from the user's buffer but then have to
; wait until it is sent out.
; This may speed up fast machines on the expense of some memory

                cmp     cx,xmt_buffsz_r ;longer than our buffers?
                jng     send_copy       ;no, copy the packet

send_user_buff:                         ;send from user's buffer
                mov     ax,si           ;input buffer addr
                pop     dx              ;segment
                call    seg2lin         ;convert to linear addr
                push    TBadrL[bx]      ;save original buffer address
                push    TBadrH[bx]      ;...
                mov     TBadrL[bx],ax
                mov     TBadrH[bx],dx
                jmp     short send_send

send_copy:
                mov     ax,TBadrL[bx]
                mov     dx,TBadrH[bx]   ;buffer address
                call    lin2seg         ;convert to offset:segment
                mov     es,dx
                mov     di,ax
                pop     ds              ;restore packet segment
                assume  ds:nothing
                push    cx              ;save length

; The packet is copied two bytes at a time, starting with the even(!)
; address of the destination buffer.
                shr     cx,1            ;convert to words, can't be zero
                rep     movsw           ;copy buffer
                jnc     send_moved      ;was original length even?
                movsb
send_moved:
                pop     cx
                push    cs
                pop     ds              ;restore ds
                assume  ds:code

send_send:
                neg     cx              ;length in two's complement
                mov     TBbcnt[bx],cx
                or      TBflags[bx],TBown+TBstp+TBenp
                push    bx              ;save ring entry address
                mov     bx,c0_TDMD+c0_INEA
                call    wrcsr0          ;start transmitter immediately
                pop     bx              ;restore ring entry addr
                xor     al,al           ;assume no error
                cmp     cx,xmt_buffsz_rn ;was this a big buffer?
                jnl     send_next       ;no, all done

                call    wait_own_0      ;wait for send ok
                mov     al,TBflags[bx]  ;save flags
                mov     cx,xmt_buffsz_rn ;original buffer size negated
                pop     TBadrH[bx]
                pop     TBadrL[bx]
                mov     TBbcnt[bx],cx



; advance to next transmit descriptor
send_next:
                add     bx,8
                cmp     bx,tbfend       ;end of ring?
                jb      xmt_nxt_ok      ;no...
                mov     bx,tbfstart     ;restart at first descriptor
xmt_nxt_ok:
                mov     tbfcurr,bx

send_done:
                test    al,TBerr        ;were there problems?
                jnz     send_err
                clc
                ret

send_err:
                call    count_out_err
                stc
                ret
send_pkt        endp


        public  get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
        assume  ds:code
        cmp     cx,EADDR_LEN
        jnb     get_addr_ok             ;buffer ok
        stc
        ret

get_addr_ok:
        push    si
        mov     si,offset IBpadr
        rep     movsb
        pop     si
        mov     cl,EADDR_LEN
        clc
        ret



        public  set_address
set_address:
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
        assume  ds:nothing
        cmp     cx,EADDR_LEN
        jnb     set_addr_ok             ;buffer ok
        mov     dh,BAD_ADDRESS
        stc
        ret

set_addr_ok:
        assume  es:nothing
        clc
        ret


reset_mode      proc    near
; stop and restart the receiver
                call    reset_interface         ;stop it
                mov     bx,c0_INIT              ;re-init
                call    wrcsr0
                mov     cx,-1                   ;wait until done
rm_lp:
                call    rdcsr0
                test    ax,c0_IDON              ;ok?
                jnz     rm_IDON                 ;yes...
                loop    rm_lp                   ;else continue

rm_IDON:
                mov     bx,c0_STRT+c0_INEA+c0_IDON+c0_RXON+c0_TXON
                call    wrcsr0                  ;start the receiver
                ret
reset_mode      endp


rcv_mode_1:
                mov     IBmode,0003h
                jmp     reset_mode


rcv_mode_3:
                mov     IBmode,0
                jmp     reset_mode

rcv_mode_6:
                mov     IBmode,8000h
                jmp     reset_mode


        public  set_multicast_list
set_multicast_list:
;enter with es:di ->list of multicast addresses, cx = number of bytes.
;return nc if we set all of them, or cy,dh=error if we didn't.
        mov     dh,NO_MULTICAST
        stc
        ret


	public	terminate
terminate:
	ret


        public  get_multicast_list
get_multicast_list:
;return with nc, es:di ->list of multicast addresses, cx = number of bytes.
;return cy, NO_ERROR if we don't remember all of the addresses ourselves.
;return cy, NO_MULTICAST if we don't implement multicast.
        mov     dh,NO_MULTICAST
        stc
        ret



        public  reset_interface
reset_interface proc    near
;reset the interface.
                assume  ds:code
                mov     bx,c0_STOP+c0_FIX
                call    wrcsr0
                ret
reset_interface endp


;called when we want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type.
        extrn   recv_find: near

;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
        extrn   recv_copy: near


        public  recv
recv    proc    near
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
        assume  ds:code

        call    rdcsr0                  ;obtain interrupting status
        mov     rcv_csr0,ax             ;save
        and     ax,0ffffh-c0_INEA       ;disable interrupts
        mov     bx,ax
        call    wrcsr0                  ;clear acknowledged i-sources
        test    bx,c0_MISS              ;out of buffers?
        jz      rcv_noerr               ;no...
        call    count_in_err

rcv_noerr:
        test    rcv_csr0,c0_RINT        ;received a packet
        jz      rcv_norint
rcv_rint:
        call    do_rcv                  ;process...

rcv_norint:
        mov     bx,c0_INEA              ;re-enable interrupts
        call    wrcsr0


;The following is currently disabled because of perhaps
;unrelated problems: the clearing of the Int-sources is still
;done immediately after the interrupt....
;
;The interrupt sources are cleared very late increasing the
;possibility that we may handle several incoming packets without
;the processor being interrupted. However, this makes it possible
;that we miss an interrupt - so we have to check twice.

;       mov     bx,rcv_csr0             ;re-enable interrupts and
;       call    wrcsr0                  ;clear acknowledged i-sources
;       mov     bx,rbfcurr              ;current receive descriptor
;       test    RBflags[bx],RBown       ;could have missed incoming pkt
;       jnz     rcv_ret                 ;no...
;       call    do_rcv                  ;process it, int already acked

rcv_ret:
        ret
recv    endp



do_rcv          proc    near
                mov     bx,rbfcurr              ;address descriptor

;The following loop continues to check the next descriptor for
;work. It can therefore happen that a we already check descriptors
;while the Lance has not yet finished receiving the whole packet on
;remaining descriptors!

rcv_lp:
                mov     al,RBflags[bx]
                test    al,RBown                ;something in buffer?
                jz      rcv_dobuf               ;yes...
                ret                             ;else return

rcv_dobuf:
                test    al,RBstp                ;start of packet?
                jz      rcv_next_bx             ;no, unrolling err chain

rcv_GetEnp:
                test    al,RBerr                ;anything wrong with it?
                jz      rcv_ge_noerr            ;no, seems ok.
                mov     dl,al                   ;copy flags
                and     dl,RBenp+RBoflo
                cmp     dl,RBenp+RBoflo         ;this is no error!
                je      rcv_enp                 ;get length
                jmp     short rcv_do_err
rcv_ge_noerr:
                test    al,RBenp                ;end of packet?
                jnz     rcv_enp                 ;yes, get length

; not on last segment
;               test    al,RBoflo               ;possible error
;               jnz     rcv_do_err_0
                add     bx,8                    ;to next descriptor
                cmp     bx,tbfstart             ;wrap around?
                jb      rcv_ge_bx               ;no...
                mov     bx,rbfstart
rcv_ge_bx:
                cmp     bx,rbfcurr              ;make sure we don't loop!
                je      rcv_do_err_0            ;this is nonsense
                mov     al,RBflags[bx]          ;get new flags
                test    al,RBown                ;must be ours!
                jz      rcv_GetEnp              ;ok, go ahead
; when we arrive here, we are processing a packet that has not yet
; been completely received. Leave everything as it is, we are
; guaranteed to check again, when the interrupt comes in at the
; latest.
                ret
rcv_do_err_0:
; should count it somewhere else, but where?

rcv_do_err:
                call    count_in_err

rcv_next:
                mov     bx,rbfcurr              ;restore current desc
rcv_next_bx:
                call    rcv_nxt_d               ;next descriptor
                jmp     rcv_lp


;end of packet, check errors and obtain length
rcv_enp:
;               test    al,RBfram+RBcrc         ;possible errors?
;               jnz     rcv_do_err
                mov     cx,RBmcnt[bx]           ;message length
                sub     cx,4                    ;strip FCS
                mov     bx,rbfcurr              ;restore descriptor addr
                cmp     cx,GIANT                ;reasonable size?
                jg      rcv_do_err              ;rubbish!
                jcxz    rcv_next                ;nothing to do...
                mov     pklen,cx                ;save packet length
                mov     pklen_rem,cx            ;remains to be rcvd

                call    rcv_getaddr

                mov     di,ax
                mov     es,dx
                add     di,EADDR_LEN*2          ;point to type field

	mov	dl, BLUEBOOK		;assume bluebook Ethernet.
	mov	ax, es:[di]
	xchg	ah, al
	cmp 	ax, 1500
	ja	BlueBookPacket
	inc	di			;set di to 802.2 header
	inc	di
	mov	dl, IEEE8023
BlueBookPacket:

                call    recv_find               ;want this packet?

                mov     ax,es
                or      ax,di
                mov     bx,rbfcurr              ;restore in any case!
                jz      rcv_next_bx             ;no buffer, give up

                push    es
                push    di


rcv_seg_lp:
; the packet's segments are copied into the client's buffer.
; The calculated length did not include the 4 byte fcs. Care must
; be taken not to copy the fcs, which may (in the worst case) be
; part of the last two segments!

                test    RBflags[bx],RBenp       ;packet ends here?
                jz      rcv_seg_l_buf           ;no, use buffer size
                mov     cx,pklen_rem            ;remaining msg length
                or      cx,cx                   ;still data?
                jle     rcv_done                ;no, just part of fcs
                jmp     short rcv_seg_l_ok

rcv_seg_l_buf:
                mov     cx,RBbcnt[bx]           ;buffer size
                neg     cx                      ;it's in two's complement
                sub     pklen_rem,cx            ;correct remaining len
                jnl     rcv_seg_l_ok            ;use whole buffer
; the current buffer contains part of the fcs...
                add     cx,pklen_rem            ;use only data part

rcv_seg_l_ok:
                lds     si,rcvbuffp             ;point to buffer
                assume  ds:nothing

; The segment is copied two bytes at a time; we know that the input
; buffer starts on an even address.
                shr     cx,1            ;CF set when not even length!
                jz      rcv_last
                rep     movsw                   ;copy the packet
rcv_last:
                jnc     rcv_moved
                movsb                           ;copy last byte

rcv_moved:
                push    cs
                pop     ds
                assume  ds:code

                test    RBflags[bx],RBenp       ;end of packet?
                jnz     rcv_done                ;yes...

                call    rcv_nxt_d               ;to next descriptor
                call    rcv_getaddr             ;next buffer addr
                jmp     rcv_seg_lp

rcv_done:
                mov     cx,pklen
                pop     si                      ;restore buffer addr
                pop     ds
                assume  ds:nothing

                call    recv_copy               ;wake up client

                push    cs
                pop     ds
                assume  ds:code

;test           jmp     rcv_next                ;do next one
                mov     bx,rbfcurr
                call    rcv_nxt_d
                ret

do_rcv          endp


        public  recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
        assume  ds:nothing
        ret


; Most of the initialisation code is used only once and can be
; dicarded when init finishes, but not this:
; we formatted the buffer descriptors (a maximum space for
; that could be provided), but where could we place this code
; if buffers are allocated immediately afterwards?
; Too dangerous in the buffer area, since we will start the receiver now.
; Possible alternative: relocate to an address after the buffers, but
; since it's only a few bytes...

eto_go:
        assume  ds:code
        call    wrcsr           ;start the Lance

        pop     dx              ;get our ending paragraph
        mov     ah,31h
        int     21h             ;terminate, stay resident


; Provide maximum space for buffer descriptors before more
; discardable initialisation code starts, since the decriptors
; must be formatted. Buffers may follow immediately afterwards,
; but will not be used until the receiver is started.

; All resident code and data must be before this label!
; on quadword boundary...
                org     begin+( (($-begin+7)/8)*8 )
FirstDescr      db      ((128+128)*8) dup (0)



        public  usage_msg
usage_msg   db  "usage: ISOLINK [-n] [-d] [-w] <packet_int_no> <options>",CR,LF
        db      "options:       [defaults]",CR,LF
        db      "  /D<DMA channel #> [0]",CR,LF
        db      "  /I<hw-int-level> [10]",CR,LF
        db      "  /P<I/O port address> [(8)280]",CR,LF
        db      "  /R<# recv buffs> <recv buff size> [16 256]",CR,LF
        db      "  /T<# xmit buffs> <xmit buff size> [1 0]",CR,LF
        db      "  /X (requires himem.sys installed)",CR,LF
        db      "$"




        public  copyright_msg
copyright_msg   db  "Packet driver for BICC 4110-2/3 ISOLINK controllers,"
		db  "version ",'0'+majver,".",'0'+version,CR,LF
                db  "Written by R.Toebbicke, CERN, Switzerland",CR,LF
 db "*     Copyright CERN, Geneva 1990 - Copyright and any other",CR,LF
 db "*     appropriate legal protection of these computer programs",CR,LF
 db "*     and associated documentation reserved in all countries",CR,LF
 db "*     of the world.",CR,LF,'$'



int_no_name     db      "Interrupt number $"
BaseName        db      "I/O Port address $"
BasePrt         dw      08280h,0
DMAName         db      "DMA channel number $"
DMAPrt          dw      0,0

        extrn   our_isr: near, their_isr: dword
        extrn   packet_int_no: byte
        extrn   phd_environ: word
        extrn   decout: near

rcv_buffno      dw      16,0
rcv_buffnoname  db      "Number of receive buffers $"
rcv_bufflog     db      4
rcv_buffsz      dw      256,0
rcv_buffszname  db      "Receive buffer size $"
xmt_buffno      dw      1,0
xmt_buffnoname  db      "Number of transmit buffers $"
xmt_bufflog     db      0
xmt_buffsz      dw      0,0
xmt_buffszname  db      "Transmit buffer size $"

orgseg          dw      ?

linaddrL        dw      0
linaddrH        dw      0

; keep these two together!
HMAaddr         dw      00010h
HMAaddrS        dw      0FFFFh

XMSctl          dd      ?

PS2IO           dw      08280h, 09250h, 0a390h, 0b1d0h
PS2INT          db      9, 10, 11, 15, 3, 4, 5, 0


        extrn   set_recv_isr: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
	extrn	get_number: near

;enter with dx -> name of word, di -> dword to print.
	extrn	print_number: near

        extrn   skip_blanks: near

argerr          db      "Error in options: '"
argerrc         db      " "
                db      "'.",CR,LF,"$"

parse_err       db      0


        public  parse_args
parse_args      proc    near
; If we run on a PS/2, read the default configuration from the POS regs

	test	sys_features,MICROCHANNEL
	jnz	do_mc_defaults
	jmp	parse_args_l
do_mc_defaults:

                mov     bx,08h                  ;1st slot
pa_slot_l:
                mov     dx,96h
                mov     ax,bx
                out     dx,al                   ;put in setup mode
                mov     dx,101h
                in      al,dx
                xchg    al,ah                   ;high id byte
                dec     dx
                in      al,dx                   ;low id byte
                cmp     ax,0808h                ;ISOLINK card?
                je      pa_found_iso            ;yes...
                inc     bx                      ;next slot
                cmp     bx,0fh                  ;too far?
                jna     pa_slot_l               ;no, try next
                mov     dx,96h
                xor     al,al
                out     dx,al                   ;exit setup mode
                jmp     parse_args_l

pa_found_iso:
                mov     dx,102h
                in      al,dx                   ;get I/O address code
                xor     ah,ah
                and     al,6
                mov     bx,ax
                mov     ax,PS2IO[bx]            ;I/O port address
                mov     BasePrt,ax

                mov     dx,104h
                in      al,dx
                xor     ah,ah
                shr     al,5                    ;isolate interrupt bits
                mov     bx,ax
                mov     al,PS2INT[bx]           ;interrupt number
                mov     int_no,ax


parse_args_l:
        call    skip_blanks
        cmp     al,CR           ;end of args?
        je      pa_ret          ;yes...
        cmp     al,'/'          ;option following?
        je      pa_doopt        ;yes

pa_err:
        mov     argerrc,al
        mov     dx,offset argerr

pa_err2:
        mov     ah,9
        int     21h             ;print error message
        mov     parse_err,1     ;indicate error
pa_ret:
        ret

pa_doopt:
        inc     si
        lodsb                   ;get option byte
        cmp     al,'a'          ;in lowercase range?
        jna     pa_upper
        sub     al,20h          ;convert to uppercase
pa_upper:
        cmp     al,'I'
        je      pa_doInt
        cmp     al,'P'
        je      pa_doPort
        cmp     al,'D'
        je      pa_doDMA
        cmp     al,'R'
        je      pa_doRcv
        cmp     al,'T'
        je      pa_doTrans
        cmp     al,'X'                  ;into extended memory?
        je      pa_doext
        jmp     pa_err

pa_doInt:
        mov     di,offset int_no
        call    get_number
        jmp     parse_args_l

pa_doPort:
        mov     di,offset BasePrt
        call    get_number
        jmp     parse_args_l

pa_doDMA:
        mov     di,offset DMAPrt
        call    get_number
        jmp     parse_args_l


pa_doRcv:
        mov     di,offset rcv_buffno
        call    get_number

        mov     di,offset rcv_buffsz
        call    get_number
        jmp     parse_args_l


pa_doTrans:
        mov     di,offset xmt_buffno
        call    get_number

        mov     di,offset xmt_buffsz
        call    get_number
        jmp     parse_args_l


pa_doext:                       ;set up for HMA
        or      options,options_HMA   ;indicate we want to run up there
        jmp     parse_args_l


parse_args      endp


	public	print_parameters
print_parameters	proc
        mov     di,offset int_no
        mov     dx,offset int_no_name
        call    print_number

        mov     di,offset BasePrt
        mov     dx,offset BaseName
        call    print_number

        mov     di,offset DMAPrt
        mov     dx,offset DMAName
        call    print_number

        mov     di,offset rcv_buffno
        mov     dx,offset rcv_buffnoname
        call    print_number

        mov     di,offset rcv_buffsz
        mov     dx,offset rcv_buffszname
        call    print_number

        mov     di,offset xmt_buffno
        mov     dx,offset xmt_buffnoname
        call    print_number

        mov     di,offset xmt_buffsz
        mov     dx,offset xmt_buffszname
        call    print_number

	ret

print_parameters	endp




inv_DMA         db      "Invalid DMA channel number.",CR,LF,"$"
init_errmsg     db      "Lance initialisation failed.",CR,LF,"$"
inv_buff        db      "Invalid buffer specification.",CR,LF,"$"
HMAnoHIMEM      db      "HMA not available: HIMEM.SYS not installed.",CR,LF,"$"
HMAnotavail     db      "HMA not available.",CR,LF,"$"
HMAalloc        db      "Using extended memory (HMA).",CR,LF,"$"
HMAtoobig       db      "Buffers exceed HMA size.",CR,LF,"$"
HMAmsg_1        db      "/X configured $"
HMAmsg_2        db      " receive buffers of length $"
HMAmsg_3        db      "."
crlf_msg	db	CR,LF,"$"

really_bad_msg	db	"This driver may be buggy.  Do you really want to run it (y/n)? ",'$'

                public  etopen
etopen          proc    near
                assume  ds:code

		mov	dx,offset really_bad_msg
		mov	ah,9
		int	21h

		mov	ah,7
		int	21h

		cmp	al,'y'
		je	etopen_1
		cmp	al,'Y'
		je	etopen_1
		int	20h
etopen_1:

		mov	dx,offset crlf_msg
		mov	ah,9
		int	21h

                test    parse_err,0ffh  ;any parsing error?
                jnz     eto_errexit2    ;yes, give up

                cmp     packet_int_no,0
                mov     dx,offset usage_msg
                jz      eto_errexit



; check DMA port
                mov     ax,DMAPrt
                or      ax,ax
                jb      eto_invalid_DMA ;should not be negative
                cmp     ax,3
                jna     eto_DMA_ok

eto_invalid_DMA:
                mov     dx,offset inv_DMA

eto_errexit:
                mov     ah,9
                int     21h
eto_errexit2:
                stc
                ret

eto_DMA_ok:
; check receive buffers
                mov     bx,rcv_buffno
                call    do_power_of_2
                cmp     ax,128
                jg      eto_inv_Buff
                or      ax,ax                   ;must have something!
                jz      eto_inv_Buff
                mov     rcv_bufflog,cl
                mov     rcv_buffno,ax

                mov     ax,rcv_buffsz   ;check buffer size
                cmp     ax,100          ;at least that big
                jg      eto_rcvsz_1
                mov     ax,100
eto_rcvsz_1:
                cmp     ax,GIANT+4      ;biggest packet plus fcs
                jl      eto_rcvsz_2     ;does not exceed
                mov     ax,GIANT+4      ;take maximum
eto_rcvsz_2:
                mov     rcv_buffsz,ax

; check transmit buffers
                mov     bx,xmt_buffno
                call    do_power_of_2
                or      ax,ax                   ;must have something!
                jz      eto_inv_Buff
                cmp     ax,128
                jna     eto_xmt_buffok

eto_inv_Buff:
                mov     dx,offset inv_Buff
                jmp     eto_errexit

eto_xmt_buffok:
                mov     xmt_bufflog,cl
                mov     xmt_buffno,ax

                mov     ax,xmt_buffsz   ;check buffer size
                cmp     ax,RUNT         ;at least that big
                jg      eto_xmtsz_1
                mov     ax,0            ;else don't use
eto_xmtsz_1:
                cmp     ax,GIANT        ;biggest packet w/o fcs
                jl      eto_xmtsz_2     ;does not exceed
                mov     ax,GIANT        ;take maximum
eto_xmtsz_2:
                mov     xmt_buffsz,ax
                mov     xmt_buffsz_r,ax ;again for our resident part
                neg     ax
                mov     xmt_buffsz_rn,ax ;times -1 for resident part

; set RAP & RDP addresses
                mov     ax,BasePrt
                add     ax,0ch
                mov     RDP,ax
                add     ax,02h
                mov     RAP,ax


; this code is from Russell Nelson's packet driver skeleton.
; Need it here, since we won't return from etopen: as soon as the
; receiver is started, this place may get clobbered with Ethernet packets

        mov     ah,35h                  ;remember their packet interrupt.
        mov     al,packet_int_no
        int     21h
        mov     their_isr.offs,bx
        mov     their_isr.segm,es

        mov     ah,25h                  ;install our packet interrupt
        mov     dx,offset our_isr
        int     21h

; First of all: stop the Lance, sets required bits in CSR3 as well
                call    reset_interface

                call    set_recv_isr    ;intercept interrupts


; Now we decide whether we will run in the HMA (above 1 MB) before
; we do any calculations involving real addresses

                test    options,options_HMA  ;will we?
                jnz     eto_tryHMA      ;yes
                jmp     eto_HMAok       ;no...

; first check of HIMEM.SYS is installed and if we can globally
; enable the HMA
eto_tryHMA:
                mov     ax,4300h
                int     2Fh             ; Is an XMS Driver installed?
                mov     dx,offset HMAnoHIMEM    ;error message in case...
                cmp     al,80h
                jne     eto_noHMA       ;no...
                mov     ax,4310h
                int     2fh             ;obtain XMS control addr
                mov     word ptr XMSctl,bx      ;save entry point
                mov     word ptr XMSctl+2,es

                mov     dx,0ffffh       ;allocate whole HMA
                mov     ah,1
                call    XMSctl
                mov     dx,offset HMAnotavail   ;error msg in case...
                cmp     ax,1
                jne     eto_noHMA               ;not available
                mov     ah,3                    ;global enable A20 line
                call    XMSctl
                cmp     ax,1                    ;ok?
                je      eto_HMA_enabled

eto_noHMA:
                mov     ah,9
                int     21h                     ;error msg addr in dx!
                and     options,not options_HMA ;turn off HMA bit
                jmp     eto_HMAok               ;continue as usual

eto_HMA_enabled:
;               mov     dx,offset HMAalloc
;               mov     ah,9
;               int     21h                     ;confirm


; maximize the receive buffer space to utilise the whole HMA:
;  1. calculate remaining space excluding receive buffers and descriptors
;  2. start with 128 receive buffers
;  3. calculate remaining space for buffers
;  4. divide by number of buffers to obtain (even) buffer size
;  5. if size inferior to defaulted or specified size half number
;     of buffers and restart on step 2.

                mov     bx,0fff0h       ;total HMA size
                sub     bx,offset FirstDescr    ;minus resident code

                mov     ax,xmt_buffsz   ;xmit buffer size
                add     ax,8            ;plus descriptor
                mul     xmt_buffno      ;=total xmit space
                or      dx,dx           ;not more than 64k!
                jnz     try_err         ;ooops

                sub     bx,ax           ;remaining space without recv
                jc      try_err         ;must be positive!

                mov     cx,128          ;try 128 buffers
try_lp:
                mov     ax,cx           ;number of buffers
                shl     ax,3            ;8 bytes long
                sub     ax,bx
                neg     ax              ;remaining space for buffers

                xor     dx,dx           ;accumulator extension
                div     cx              ;calculate buffer length
                and     ax,0fffeh       ;even length
                cmp     ax,rcv_buffsz   ;long enough?
                jnb     try_ok          ;yes...
                shr     cx,1            ;else half number of buffers
                jnz     try_lp          ;and retry

try_err:
                mov     dx,offset HMAtoobig
                jmp     eto_errexit

try_ok:
                cmp     ax,GIANT+4      ;maximum size
                jng     try_ok_2        ;does not exceed
                mov     ax,GIANT+4      ;else maximum length
try_ok_2:
                mov     rcv_buffno,cx
                mov     rcv_buffsz,ax
                mov     bx,cx
                call    do_power_of_2   ;have to do it again
                mov     rcv_bufflog,cl



                mov     dx,offset HMAmsg_1
                mov     ah,9
                int     21h

                mov     ax,rcv_buffno
                xor     dx,dx
                call    decout

                mov     dx,offset HMAmsg_2
                mov     ah,9
                int     21h

                mov     ax,rcv_buffsz
                xor     dx,dx
                call    decout

                mov     dx,offset HMAmsg_3
                mov     ah,9
                int     21h


; la demarche a suivre:
; we copy ourselves into the HMA completely, format all the
; descriptors, correct the interrupt addr and exit to DOS

                mov     ax,cs
                mov     orgseg,ax       ;save original segment
                les     di,dword ptr HMAaddr ;that's where we will be
                mov     cx,offset end_code  ;length(!) of code to be copied
                mov     si,10h          ;start at offset 10
                sub     cx,si           ;correct length
                rep     movsb           ;copy us
                push    HMAaddrS        ;our new segment
; careful: here comes a jump!
                call   setcs            ;we're in the HMA
                push    cs
                pop     ds              ;our data as well



eto_HMAok:
; get linear address for receive buffer descriptor start
                push    ds
                mov     ax,offset FirstDescr
                pop     dx
                call    seg2lin

; Store in Init Block
                mov     IBrdraL,ax
                mov     IBrdraH,dx

; calculate linear address of first buffer
                mov     bx,offset FirstDescr
                mov     rbfstart,bx
                mov     rbfcurr,bx

                mov     ax,rcv_buffno
                push    ds
                pop     dx
                add     ax,xmt_buffno   ;result is still in segment
                shl     ax,3            ;times eight
                add     ax,bx           ;won't leave segment!
                call    seg2lin         ;convert to linear addr
                mov     linaddrL,ax     ;first available buffer addr
                mov     linaddrH,dx

; format receive buffer descriptors
                mov     si,rcv_buffsz   ;buffer size
                mov     cx,rcv_buffno   ;number of buffers
                mov     dl,RBown        ;flags
eto_rcvloop:
                call    format_descr
                add     bx,8
                loop    eto_rcvloop


;set transmit descriptor ring address
                push    ds
                mov     ax,bx
                pop     dx
                call    seg2lin
                mov     IBtdraL,ax
                mov     IBtdraH,dx

; format transmit buffer descriptors
                mov     tbfstart,bx             ;remember start
                mov     tbfcurr,bx
                mov     dl,0                    ;initial flags
                mov     cx,xmt_buffno           ;number of buffers
                mov     si,xmt_buffsz           ;buffer size
eto_xmtloop:
                call    format_descr
                add     bx,8
                loop    eto_xmtloop
                mov     tbfend,bx               ;remember end


; finish initialisation block
                mov     al,rcv_bufflog  ;# of rcv buffers
                shl     al,5
                mov     IBrdraHF,al

                mov     al,xmt_bufflog  ;# of xmt buffers
                shl     al,5
                mov     IBtdraHF,al

; copy Ethernet address out of ROM
                mov     cx,6
                push    ds
                pop     es
                mov     di,offset IBpadr
                mov     dx,BasePrt
eto_addrloop:
                in      ax,dx
                inc     dx
                inc     dx
                stosb
                loop    eto_addrloop

; init DMA controller
                test    sys_features,MICROCHANNEL ;running on a PS/2?
                jnz     eto_DMA_done    ;yes, no DMA
                mov     al,0d0h         ;mode reg bits
                mov     dx,0bh          ;addr of mode reg
                or      al,byte ptr DMAPrt   ;or in DMA number
                out     dx,al
                jmp     $+2
                and     al,3            ;leave only DMA channel #
                mov     dx,0ah          ;mask register addr
                out     dx,al           ;clear the DMA channel
eto_DMA_done:

; set initialisation block addr
                push    ds
                pop     dx
                mov     ax,offset IBmode
                call    seg2lin
                push    dx              ;don't loose it
                mov     bx,ax
                mov     ax,CSR1
                call    wrcsr           ;low order 16 bits of addr
                pop     bx
                mov     ax,CSR2
                call    wrcsr           ;high order bits

; init the controller and wait for completion
                mov     bx,c0_FIX+c0_INIT
                call    wrcsr0

                mov     cx,-1           ;wait for completion
eto_initlp:
                call    rdcsr0
                test    ax,c0_IDON      ;done?
                jnz     eto_init_ok     ;good!
                loop    eto_initlp

                mov     dx,offset init_errmsg
                call    outofHMA        ;get out of HMA
                jmp     eto_errexit

eto_init_ok:
                test    options,options_HMA ;are we in HMA?
                jz      eto_tsr         ;no, use TSR code
                xor     ax,ax
; correct the packet and hardware interrupt addresses
                mov     es,ax
                xor     bh,bh
                mov     bl,packet_int_no
                shl     bx,2            ;point to interrupt vector
                push    cs
                pop     es:[bx+2]

                mov     bx,int_no       ;card interrupt
                add     bx,68h          ;assume above 8
                cmp     bx,8+68h        ;is it?
                jnb     eto_HMA_int_ok  ;yes...
                sub     bx,60h          ;PS/2 can use interrupt < 8
eto_HMA_int_ok:
                shl     bx,2            ;point to interrupt vector
                push    cs
                pop     es:[bx+2]

; Now start the receiver and exit to DOS
        call    outofHMA                ;out of danger zone (buffers!)
        mov     bx,c0_STRT+c0_INEA+c0_IDON+c0_RXON+c0_TXON  ; ...SET...
        call    wrcsr0                                      ; ...GO!
        int     20h                     ;exit...


eto_tsr:
;calculate end of resident part (relative to pgm start)
                mov     ax,linaddrL
                mov     dx,linaddrH
                add     ax,15           ;round to paragraph
                adc     dl,0
                shr     ax,4
                shl     dl,4
                or      ah,dl
                mov     dx,ds           ;minus pgm start addr
                sub     ax,dx
                push    ax              ;save the address


; again some code from the packet driver skeleton...

        mov     ah,49h                  ;free our environment, because
        mov     es,phd_environ          ;  we won't need it.
        int     21h

        mov     bx,1                    ;get the stdout handle.
        mov     ah,3eh                  ;close it in case they redirected it.
        int     21h

        mov     ax,csr0                                     ; READY...
        mov     bx,c0_STRT+c0_INEA+c0_IDON+c0_RXON+c0_TXON  ; ...SET...
        jmp     eto_go                                      ; ...GO!
etopen          endp


do_power_of_2   proc    near
; adjust bx to valid # of buffers.
; on exit: ax=fitting number, cx=log2(ax)
                mov     cx,7            ;allow 7 shifts
                mov     ax,bx
dpw2_loop:
                cmp     ax,1
                je      dpw2_done
                shr     ax,1
                loop    dpw2_loop
dpw2_done:
                mov     ax,7
                sub     ax,cx           ;get number of shifts
                mov     cx,ax

                mov     ax,1
                jcxz    dpw2_ret
                shl     ax,cl
dpw2_ret:
                ret
do_power_of_2   endp





format_descr    proc    near
; format buffer descriptor pointed to by [bx]
; si=buffer size, dl=flags
                push    dx              ;save flags
                mov     ax,linaddrL     ;next available buffer addr
                mov     dx,linaddrH
                mov     RBadrL[bx],ax   ;buffer addr
                mov     RBadrH[bx],dx
                add     ax,si           ;point to next available addr
                adc     dx,0
                mov     linaddrL,ax
                mov     linaddrH,dx
                mov     ax,si           ;get buffer size
;               or      ah,0f0h         ;reserved bits to 1
                neg     ax              ;two's complement
                mov     RBbcnt[bx],ax
                pop     dx              ;restore flags
                mov     RBflags[bx],dl
                ret
format_descr    endp


outofHMA        proc    near    ;leave the HMA
                test    options,options_HMA ;are we in it?
                jz      ooHret          ;no...
                push    orgseg
                call    setcs
                push    cs
                pop     ds
ooHret:
                ret
outofHMA        endp


setcs           proc    near
                db      0cbh            ;RET FAR (!)
setcs           endp

end_code        equ     $

code    ends
        end


