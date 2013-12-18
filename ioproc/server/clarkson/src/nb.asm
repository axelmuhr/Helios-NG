version equ     3

        include defs.asm        ;SEE ENCLOSED COPYRIGHT MESSAGE
NCB_NOWAIT         equ 80h         ;  Command wait flag

NCB_RESET          equ 32h         ;  Reset adapter
NCB_CANCEL         equ 35h         ;  Cancel command
NCB_STATUS         equ 33h         ;  Get NETBIOS intf status
NCB_UNLINK         equ 70h         ;  Unlink   (RPL)
NCB_ADDNAME        equ 30h         ;  Add name
NCB_ADDGNAME       equ 36h         ;  Add group name
NCB_DELNAME        equ 31h         ;  Delete name
NCB_FINDNAME       equ 78h         ;  Find name
NCB_CALL           equ 10h         ;  Call
NCB_LISTEN         equ 11h         ;  Listen
NCB_HANGUP         equ 12h         ;  Hang up
NCB_SEND           equ 14h         ;  Send
NCB_CHSEND         equ 17h         ;  Chain send
NCB_RECEIVE        equ 15h         ;  Receive
NCB_RECANY         equ 16h         ;  Receive any
NCB_SESSTATUS      equ 34h         ;  Get session status
NCB_SDATAGRAM      equ 20h         ;  Send datagram
NCB_SBROADCAST     equ 22h         ;  Send broadcast
NCB_RDATAGRAM      equ 21h         ;  Receive datagram
NCB_RBROADCAST     equ 23h         ;  Receive broadcast
NCB_TRACE          equ 79h         ;  Start trace

ncb             struc
ncb_command     db ?
ncb_retcode     db ?
ncb_lsn         db ?
ncb_num         db ?
ncb_buffer      dd ?
ncb_length      dw ?
ncb_callname    db 16 dup(?)
ncb_name        db 16 dup(?)
ncb_rto         db ?
ncb_sto         db ?
ncb_post        dd ?
ncb_lana_num    db ?
ncb_cmd_cplt    db ?
ncb_reserve     db 14 dup(?)
ncb             ends

code    segment word public
        assume  cs:code, ds:code
;***************************************************************
prefix          db "TCPIP"

        public  int_no
int_no  db      0,0,0,0
ip_adress       dw      2 dup(0)
rq_size         dw      3
padding         dd      0

	public	driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class    db      6,0             ;from the packet spec
driver_type     db      0               ;from the packet spec
driver_name     db      'netbios',0     ;name of the driver.
driver_function	db	2
parameter_list	label	byte
	db	1	;major rev of packet driver
	db	9	;minor rev of packet driver
	db	14	;length of parameter list
	db	EADDR_LEN	;length of MAC-layer address
	dw	512	;MTU, including MAC headers
	dw	MAX_MULTICAST * EADDR_LEN	;buffer size of multicast addrs
	dw	0	;(# of back-to-back MTU rcvs) - 1
	dw	0	;(# of successive xmits) - 1
int_num	dw	0	;Interrupt # to hook for post-EOI
			;processing, 0 == none,

	public	rcv_modes
rcv_modes	dw	4		;number of receive modes in our table.
		dw	0,0,0,rcv_mode_3

lana_num        db 0
local_ncb_num   db 0  ; handle returned by add_name


        public  nbsend, nbrcv1, nbrcv2, rcv_buffer1, rcv_buffer2 ; MAP
nbsend          ncb <>
nbrcv1          ncb <>
nbrcv2          ncb <>


MAX_DG          equ 512
rcv_buffer1     db MAX_DG dup(?)
rcv_buffer2     db MAX_DG dup(?)

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
        assume ds: nothing
;enter with ds:si -> packet, cx = packet length.
;exit with nc if ok, or else cy if error, dh set to error number.
        push    es
        push    bx
        push    ds

        push    cs
        pop     es
        mov     bx,offset cs:nbsend

        mov     cs:[bx].ncb_buffer.offs,si
        mov     ax,ds
        mov     cs:[bx].ncb_buffer.segm,ds
        mov     cs:[bx].ncb_length,cx

        push    bx
        lea     di,cs:nbsend.ncb_callname
        mov     dh,ds:[si+16]
        mov     dl,ds:[si+17]
        mov     bh,ds:[si+18]
        mov     bl,ds:[si+19]
        push    cs
        pop     ds
        call    ip_to_nbname
        pop     bx

        int     5ch

        cmp     al,0
        jz      send_ok
        add     ax,32
        call    tty
        mov     al,'S'
        call    tty
        stc
        jmp     send_done
send_ok:
        mov     al,'s'
        call    tty
        clc
send_done:
        pop     ds
        pop     bx
        pop     es
        ret



        public  get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
        clc
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
	ret

        public  reset_interface
reset_interface:
;reset the interface.
        ret


;called when we want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type, dl = packet class.
        extrn   recv_find: near

;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
        extrn   recv_copy: near

        extrn   count_in_err: near
        extrn   count_out_err: near
        extrn   dwordout: near
        extrn   error: near

        public  recv
recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
        assume  ds:code
        ret

        public nbint  ; MAP
nbint   proc far
        push ds
        push cs
        pop  ds
        mov     al,'r'
        call    tty
        mov  al,[bx].ncb_retcode
        cmp  al,0               ; ok
        jnz  bad_nbint
        push bx
        mov  cx,[bx].ncb_length
        mov  di,0
	mov  dl,cs:driver_class
        call recv_find
        pop  bx
        mov  ax,es
        or   ax,di
        je   nbint_done

;                   cx, di already set
        mov     si,[bx].ncb_buffer.offs
        rep     movsb
        call    recv_copy

        mov     [bx].ncb_length,MAX_DG  ; reestablish dg-length

        push    ds
        pop     es
        int 5ch
        cmp     al,0ffh
        jz      nbint_pending_ok1
        mov     al,'?'
        call    tty
        cmp     al,00h
        jz      nbint_pending_ok1
        add     ax,32
        call    tty
        mov     al,'.'
        call    tty

nbint_pending_ok1:
        jmp     short nbint_done

bad_nbint:
        cmp     ax,17h          ; name deleted
        add     ax,32
        call    tty
        mov     al,'R'
        call    tty

nbint_done:
        pop ds
        iret
nbint   endp

        public  nb_stop
nb_stop:
        ;                   unregister name
        push    cs
        pop     es
        lea      bx,nbrcv1
        mov     [bx].ncb_command,NCB_DELNAME
;        mov     al,lana_num
;        mov     [bx].ncb_lana_num,al
        int     5ch
        cmp     al,0
        jz      good_delete
        add     ax,32
        call    tty
        mov     al,'I'
        call    tty
good_delete:
        ret



tty:
        push    bx
        mov     ah,14
        int     10h
        pop     bx
        ret

ip_to_nbname:
        ;      call with es:di = *ncb_name
        ;            bx and dx = ip_address

        mov     cx,5
        mov     si,offset prefix
        rep     movsb

        mov     [di],bx
        mov     [di+2],dx
        ret

	public	recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
	assume	ds:nothing
	ret


;any code after this will not be kept after initialization.
        public end_resident ; MAP
end_resident    label   byte


        public  usage_msg
usage_msg       db      "usage: nb [-n] [-d] [-w] <packet_int_no> <ip.ad.dr.ess> [receive queue size]",CR,LF,'$'

        public  copyright_msg
copyright_msg   db      "Packet driver for a netbios device, version ",'0'+majver,".",'0'+version,CR,LF
                db      "Portions Copyright 1990, Michael Haberler",CR,LF,'$'

ip_adress_name  db      "IP Adress ",'$'
rq_size_name    db      "Receive Queue ",'$'
bad_name_msg    db      " bad returncode from nb add_name",CR,LF,'$'
bad_rcv_msg     db      " bad returncode from nb receive dg",CR,LF,'$'
good_name_msg   db      " good returncode from nb add_name",CR,LF,'$'
good_rcv_msg    db      " good returncode from nb receive dg",CR,LF,'$'
temp_dw         dw      ?
        extrn   set_recv_isr: near

;enter with si -> argument string, di -> word to store.
;if there is no number, don't change the number.
        extrn   get_number: near

        public  parse_args
parse_args:
        mov     di, offset temp_dw
        mov     bx, offset ip_adress_name
        call    get_number
        mov     ax,temp_dw
        mov     byte ptr ip_adress+3,al
        inc     si
        mov     di, offset temp_dw
        mov     bx, offset ip_adress_name
        call    get_number
        mov     ax,temp_dw
        mov     byte ptr ip_adress+2,al
        inc     si
        mov     di, offset temp_dw
        mov     bx, offset ip_adress_name
        call    get_number
        mov     ax,temp_dw
        mov     byte ptr ip_adress+1,al
        inc     si
        mov     di, offset temp_dw
        mov     bx, offset ip_adress_name
        call    get_number
        mov     ax,temp_dw
        mov     byte ptr ip_adress+0,al

        mov     di, offset rq_size
        mov     bx, offset rq_size_name
        call    get_number
        ret



        public  etopen
etopen:
        pushf
        cld

;if all is okay,
;        mov     bx,offset nbsend
        lea     bx,nbsend
        mov     [bx].ncb_command,NCB_ADDNAME
        mov     al,lana_num
        mov     [bx].ncb_lana_num,al

        push    ds
        push    cs
        push    cs
        pop     ds
        pop     es

        push    bx
        lea     di,nbsend.ncb_name
        mov     bx,ip_adress
        mov     dx,ip_adress+2
        call    ip_to_nbname
        pop     bx

        mov     cx,16
        mov     di,offset nbrcv1.ncb_name
        mov     si,offset nbsend.ncb_name
        rep     movsb
        mov     cx,16
        mov     di,offset nbrcv2.ncb_name
        mov     si,offset nbsend.ncb_name
        rep     movsb

        int     5ch     ; add_name, -> returns rc in al
        cmp     al,0
        jz      good_name
        mov     dx,0
        mov     ah,0
        call    dwordout
        mov     dx,offset bad_name_msg
        pop     ds
        jmp     error

good_name:
;        mov     dx,offset good_name_msg
;        call    say

        mov     al,[bx].ncb_num
        mov     local_ncb_num,al
        mov     [bx].ncb_command,NCB_SDATAGRAM  ;set send command code

        ;       start receive operations
;        mov     bx,offset nbrcv1
        lea     bx,nbrcv1
        mov     al,NCB_RDATAGRAM+NCB_NOWAIT  ;set rcv command
        mov     [bx].ncb_command,al

        mov     al,local_ncb_num
        mov     [bx].ncb_num,al

        mov     ax,MAX_DG
        mov     [bx].ncb_length,ax

        mov     ax,ds
        mov     [bx].ncb_buffer.segm,ax
        mov     [bx].ncb_buffer.offs,offset rcv_buffer1

        mov     [bx].ncb_post.segm,ax
        mov     [bx].ncb_post.offs,offset nbint

        mov     al,lana_num
        mov     [bx].ncb_lana_num,al
        int     5ch

        cmp     al,0ffh
        jz      pending_ok1
        cmp     al,00h
        jnz     bad_init

pending_ok1:
        mov     dx,offset good_rcv_msg
        call    say

        mov     dx,offset end_resident
        pop     ds
        popf
        clc
        ret
;if we got an error,
bad_init:
        mov     ah,0
        mov     dx,0
        call    dwordout
        mov     dx,offset bad_rcv_msg
        call    say
        call    nb_stop
        pop     ds
        popf
        stc
        ret

	public	print_parameters
print_parameters:
	ret

say:
        mov     ah,9
        int     21h
        ret


code    ends

        end

