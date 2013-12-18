;*****************************************************************************;
;*                                                                           *;
;*     File:   IBMTOKEN.ASM                                                  *;
;*     Auth:   Brian Fisher                                                  *;
;*             Queens University                                             *;
;*             Computing and Communications Services                         *;
;*             Rm 2-50 Dupuis Hall                                           *;
;*             Kingston Ontario                                              *;
;*                                                                           *;
;*     Date:   September 3 1989                                              *;
;*                                                                           *;
;*     Purp:   Ethernet (3C501) packet driver for IBM Token Ring.  This      *;
;*             driver uses the IBM LAN support program DIR interface.        *;
;*             The current implementation supports IP and ARP.               *;
;*                                                                           *;
;*             (C) 1989 Queens University                                    *;
;*             Computing and Communications Services.  This portion of       *;
;*             the program remains the property of Queens University.        *;
;*                                                                           *;
;*===========================================================================*;
;*     Program Logic:                                                        *;
;*                                                                           *;
;*     IBMTOKEN extracts IP and ARP data from ethernet datagrams passed to   *;
;*     it by the ULP, makes any adjustments required, then builds a token    *;
;*     ring datagram and sends it out over the network.  The process is      *;
;*     reversed when packets are received.  IBMTOKEN supports token ring     *;
;*     source routing and functions in a bridged environment.                *;
;*                                                                           *;
;*     The ULP uses Ethernet 'Blue Book' encapsulation.  IBMTOKEN uses       *;
;*     802.2 LLC format for transmission on token ring.  The IBM (tm)        *;
;*     LAN Support Program is required.  IBMTOKEN uses the DIR interface     *;
;*     to send/receive packets on token ring.                                *;
;*                                                                           *;
;*     IP packets do not require any modifications, and are sent 'as is.'    *;
;*     ARP packets have an 'hwr' field that must be changed fro 1h to 6h     *;
;*     during send_pkt, and changed back to 1h from 6h during _receive.      *;
;*                                                                           *;
;*     The minimum size of an ethernet packet is 60 bytes (minus the 4 byte  *;
;*     ethernet checksum).  Token ring has no lower limit.  The ULP will     *;
;*     reject 'short' packets, so IBMTOKEN lies and tells it there are 60    *;
;*     bytes.                                                                *;
;*                                                                           *;
;*     ARP packets are 28 bytes in length.  The ULP rounds the size of a     *;
;*     packet up to 60 bytes, so there is extra data on the end of the ARP   *;
;*     packet.  IBMTOKEN trims this off, because token ring hosts reject     *;
;*     'too long' ARP packets.                                               *;
;*                                                                           *;
;*     The minimum size of an ethernet packet is 60 bytes.  The maximum      *;
;*     is 1514 bytes.  The buffer sizes for token ring send/receive are      *;
;*     set to accomodate this.                                               *;
;*                                                                           *;
;*     Source Routing:                                                       *;
;*                                                                           *;
;*     The ethernet entrenched logic of the ULP does not accomodate source   *;
;*     routing.  IBMTOKEN provides a mechanism for source routing that is    *;
;*     transparent to the ULP.  The RIF information associated with token    *;
;*     ring source addresses are placed in a cache.  When the ULP sends to   *;
;*     that address, the RIF information is extracted from the cache and     *;
;*     used to route the packet to the target host.                          *;
;*                                                                           *;
;*     RIF Cache Logic:                                                      *;
;*                                                                           *;
;*        The cache logic uses an LRU algorithm.  Each entry is time         *;
;*        stamped.  If all the slots in the cache are being used, the oldest *;
;*        entry (assumed to be the LRU) is replaced (bumped from the cache). *;
;*                                                                           *;
;*        If the ULP uses an address whose RIF has been bumped from the      *;
;*        cache, a mechanism exists to rediscover the source route.          *;
;*        IBMTOKEN attaches a phony RIF, with the broadcast bit set and      *;
;*        and empty route table.  This packet travels to all rings.  When    *;
;*        the foreign host responds, the RIF info is placed back in the      *;
;*        cache.  Subsequent transmissions will use the rediscovered RIF.    *;
;*                                                                           *;
;*        The size of the RIF cache should be set to handle the maximum      *;
;*        number of concurrent hosts.  This will minimize the bumping        *;
;*        effect and minimize all rings broadcasts.                          *;
;*                                                                           *;
;*        I chose this algorithm so the ULP would not require any special    *;
;*        coding to work.  It eliminates the need for special syncronization *;
;*        of the ARP and RIF cache's.                                        *;
;*                                                                           *;
;*===========================================================================*;
;*                                                                           *;
;*      Revs:  Sep 29 1989     Successful tests with TN3270, FTP, and FTPBIN.*;
;*             B. Fisher       Some fine tuning required.                    *;
;*                                                                           *;
;*             Oct 1  1989     Cleaning up code.  Found bug that prevented   *;
;*                             buffer1/buffer2..n copies from working.       *;
;*                                                                           *;
;*             Oct 15 1989     Rearranged buffer sizes to accomodate Adapt I *;
;*                                                                           *;
;*             Nov 15 1989     Added missing popf at end of recv_complt      *;
;*                             Fixed missing popf at end of recv_complt      *;
;*                                                                           *;
;*             Nov 19 1989     Added marker to show interrupt activity       *;
;*                             for debugging.                                *;
;*                                                                           *;
;*             Nov 20 1989     Add changes for Release 5 compatibility       *;
;*                                                                           *;
;*             Nov 21 1989     Fix es/bx reversal at get_size                *;
;*                                                                           *;
;*             Nov 22 1989     RIF not working because of confusion with     *;
;*                             SA bit acquired via ARP and SA bit acquired   *;
;*                             through Token Receive.  Send_Pkt now looks    *;
;*                             up all dest addresses in RIF table.           *;
;*                                                                           *;
;*             Nov 23 1989     Have DEBUG mode report RIF table address when *;
;*                             get_multicast_ call made.                     *;
;*                                                                           *;
;*             Nov 28 1989     Add phony RIF to 'local' addresses so the     *;
;*                             bridge will pass them on.  ARP acquired HW    *;
;*                             addresses need RIF to cross bridge.  This is  *;
;*                             done in send_pkt.                             *;
;*                                                                           *;
;*             Nov 29 1989     Fixed REPT macro in SA_blk so size of SA info *;
;*                             is correct.  Didn't hurt function, however.   *;
;*                                                                           *;
;*             Nov 29 1989     Clear broadcast bit in RIF before entry in    *;
;*                             table.  This prevents bridges from altering   *;
;*                             existing route.  FF,FF,FF,FF,FF,FF address    *;
;*                             uses broadcast bit so ARP will work.          *;
;*                                                                           *;
;*             Nov 29 1989     Predefined broadcast RIF entry direction bit  *;
;*                             polarity reversal prevented broadcast from    *;
;*                             going over the bridge.                        *;
;*                                                                           *;
;*             Version 1.A -   Production Version released at Queens         *;
;*                                                                           *;
;*             Jan 02 1990     Fix missing CLD in _receiver, shows up when   *;
;*                             TOKREUI is used instead of Lan Support Pgm... *;
;*                                                                           *;
;*             Version 1.B                                                   *;
;*             Jan 15 1990     Begin modifications for LRU algorithm in the  *;
;*             (alpha 0)       RIF cache.  3rd parm on cmd line sets number  *;
;*                             of entries in the cache, which is allocated   *;
;*                             using INT 21, Function 48h.  The cache        *;
;*                             routines are modified to handle the new addr  *;
;*                             mode for accessing the table.                 *;
;*                                                                           *;
;*             Jan 16 1990     Adding time mark info to SA_table structure.  *;
;*             (alpha 1)       add_entry initializes t_mark                  *;
;*                             loc_entry refreshes t_mark when match found   *;
;*                             add_entry uses LRU slot in table when slots   *;
;*                             are all in use.                               *;
;*                                                                           *;
;*             Jan 18 1990     Updated the program documentation to reflect  *;
;*                             the operation of the RIF cache.               *;
;*                                                                           *;
;*             Version 1.C     Add code for compatibility with Release 6 of  *;
;*             Mar 18 1990     Clarkson drivers, including TERMINATE call.   *;
;*                                                                           *;
;*             Version 1.D     Fix bug that prevents get_address from        *;
;*                             returning adapter 1's hardware address.       *;
;*                                                                           *;
;*****************************************************************************;

version        equ     14h                     ; Version D = 13h

       include defs.asm

debug          = 1                             ; set 1 for IRM marks on RX

alpha          = 1                             ; ALPHA Test Version = 1
alpha_ver      = 1                             ; Which ALPHA Version...

hrs24          = 1573040                       ; ticks in 24 hour day

toke_vect      = 5Ch                           ; LAN Support Pgm Vector
toke_wait      = 0FFh                          ; completion code wait status
pool_size      = 512                           ; 8K space for buffer pool area

num_rcv_buff   = 2                             ; <2 defaults to 8
recv_size      = 1536                          ; max size of card recieve buffer
tran_size      = 1536                          ; max size of transmit buffer

STATION_ID     = 0;                            ; talk using direct station

RECV_OPTIONS   = 0C0h                          ; contiguous mac/data in bufs

open_cmd       =       3h                      ; LAN Support OpCodes
interrupt_cmd  =       0h
initialize_cmd =       20h
close_cmd      =       4h
modify_cmd     =       1h
restore_cmd    =       2h
trans_dir_cmd  =       0Ah
free_ccb_cmd   =       27h
recv_can_cmd   =       29h
open_recv_cmd  =       28h
get_status_cmd =       21h

SA_table_size  = 32                            ; default size of the RIF cache
SA_min_size    = 2                             ; minimum size
SA_max_size    = 1024                          ; maximum size

LLC_AC         = 10h                           ; access control LLC
LLC_FC         = 40h                           ; frame control  LLC
LLC_SSAP_DSAP  = 0AAAAh                        ; reversed for network <grin>
LLC_CON        = 03h

MAC_hdr_size   = 22                            ; MAC+LLC+SNAP header length

not_SA_mask    = 7Fh
SA_mask        = 80h
RIF_size_mask  = 1Fh
RIF_dir_bit    = 80h

broadcast_byte = 0FFh                          ; broadcast is FF,FF,FF,FF,FF,FF

ARP_type       = 0608h                         ; reversed for network order
ARP_Eth_hwr    = 0100h                         ; reversed for network order
ARP_Tok_hwr    = 0600h                         ; reversed for network order
ARP_packet_size= 28                            ; fixes Ethernet assumption...
phony_RIF      = 2082h                         ; used for bridges...



LLC_info_size  = 8                             ; LLC (3) + SNAP(5) = 8

mark           = 0F90h                         ; marker debug pos on screen 25

marker         macro   st,nd

               IF  debug NE 0                    ; do marker if debug <> 0

               pushf                             ; show 2 char marker on
               push es                           ; 25th line, 1st column
               push ax
               mov  ax,0B800h
               mov  es,ax
               mov  al,'&st&'
               mov  byte ptr es:[mark],al
               mov  al,byte ptr es:[mark+1]      ; get color value
               inc  al
               and  al,0Fh
               or   al,1
               mov  byte ptr es:[mark+1],al      ; advance it to show activity
               mov  al,'&nd'
               mov  byte ptr es:[mark+2],al
               mov  al,byte ptr es:[mark+3]
               inc  al
               and  al,0Fh
               or   al,1
               mov  byte ptr es:[mark+3],al
               pop  ax
               pop  es
               popf

               ENDIF

               endm

call_token     macro   ccb
;--------------------------------------;
; loads address of CCB in ES:BX, then  ;
; calls the Token Ring Interface.      ;
;--------------------------------------;
               mov     ax,cs
               mov     es,ax
               mov     bx,offset &ccb&
               int     toke_vect
               endm


call_wait      macro
               local   wait_loop
;---------------------------------------;
; assumes ES:BX points to a CCB.  Waits ;
; for a pending command to terminate.   ;
;---------------------------------------;
wait_loop:
               mov     al,es:[bx+2]
               cmp     al,toke_wait
               je      wait_loop
               endm


ticks          macro
;---------------------------------------;
; get system timer ticks in CX:DX       ;
;---------------------------------------;
               mov     ah,0
               int     1Ah
               endm


set_ptr        macro   dest,addr
;---------------------------------------;
; loads a far pointer with a near       ;
; address offset and CS:                ;
;---------------------------------------;
               mov     ax,offset &addr&
               mov     word ptr [&dest&],ax
               mov     ax,cs
               mov     word ptr [&dest&+2],ax
               endm


print$         macro   string
;---------------------------------------;
;  sends $ terminated string to screen  ;
;---------------------------------------;
               mov     ah,9
               mov     dx,offset &string&      ; print $ terminated string
               int     21h
               endm


clr_struc      macro   name
;---------------------------------------;
;  clear all bytes of the named struc   ;
;  to zero.  Useful for Token calls.    ;
;---------------------------------------;
               mov     cx,SIZE &name&          ; uses ax, es:di, cx
               mov     ax,ds
               mov     es,ax
               mov     di,offset &name&
               xor     al,al
               cld
               rep     stosb                   ; set all elements to 0
               endm


rdupb          macro   l,n
&l&            db      ?
               rept    &n&-1
               db      ?
               endm
               endm

       IF debug NE 0

regs   struc                           ; from head.asm, [bp] references the
_RES   dw      ?                       ; IRQ service parameters with these.
_RDS   dw      ?
_RBP   dw      ?
_RDI   dw      ?
_RSI   dw      ?
_RDX   dw      ?
_RCX   dw      ?
_RBX   dw      ?
_RAX   dw      ?
_RIP   dw      ?
_RCS   dw      ?
_RCF   dw      ?               ;flags
regs   ends

       ENDIF


ccb_blk        struc                   ; CCB used for local token ring calls.
adapter        db      ?               ; adapter 0 or 1
command        db      ?               ; command code
returncode       db      ?               ; command completion code
work           db      ?               ; scratch
pointer        dd      ?               ; queue pointer
complt         dd      ?               ; command complete appendage
parms          dd      ?               ; pointer to CCB parameter table
ccb_blk ends


init_parms     struc                   ; parameters for DIR_INITIALIZE
bring_ups      dw      ?               ; result of bring up tests
sram_addr      dw      ?               ; shared ram address
reserved       dd      ?               ; not used...
chk_exit       dd      ?               ; adapter check exit
status         dd      ?               ; ring status exit
error          dd      ?               ; PC error exit
init_parms     ends


o_parms        struc                   ; ccb parms for DIR_OPEN
adapt          dd      ?               ; ptr to adapter parms
direct         dd      ?               ; ptr to direct parms
dlc            dd      ?               ; ptr to dlc parms
msg            dd      ?               ; ptr to msg parms
o_parms        ends


ada_blk        struc                   ; adapter open parms table format
err_code       dw      ?               ; open errors detected
options        dw      ?               ; various options
node_addr      dd      ?               ; node address ( 4 bytes here +  )
               dw      ?               ; rest of node address ( 2 here  )
group_add      dd      ?               ; group address
func_add       dd      ?               ; functional address
rcv_buff       dw      ?               ; number of receive buffers
rcv_len        dw      ?               ; receive buffer length
dhb_len        dw      ?               ; length of transmit buffers
hold_buf       db      ?               ; number of tx buffers
               db      ?               ; reserved
lock_code      dw      ?               ; lock code
id_addr        dd      ?               ; address of id code
ada_blk        ends


dir_blk        struc                   ; dir interfce open parms table format
buf_size       dw      ?               ; size of direct station buffer
pool_blks      dw      ?               ; number of 16 byte blocks in buff pool
pool_addr      dd      ?               ; address of direct station buffer pool
chk_exit       dd      ?               ; the rest can be 0 (defaults) for now
stat_exit      dd      ?               ; ring status appendage pointer
err_exit       dd      ?               ; PC error exit
work_addr      dd      ?               ; work area segment value
len_req        dw      ?               ; work area length requested
len_act        dw      ?               ; actual length obtained
dir_blk        ends


dlc_blk        struc                   ; dlc interface open parms table format
max_sap        db      ?               ; max number of saps
max_sta        db      ?               ; max number of stations
max_gsap       db      ?               ; max group saps
max_gmem       db      ?               ; max members per group
t1_tick1       db      ?               ; DLC timer t1 interval
t2_tick1       db      ?               ; DLC timer t2 interval
TI_tick1       db      ?               ; DLC timer TI interval
t1_tick2       db      ?               ; DLC timer t1 group 2
t2_tick2       db      ?               ; DLC timer t2 group 2
TI_tick2       db      ?               ; DLC timer TI group 2
dlc_blk        ends


mod_blk        struc                   ; modify open parms parameter block
buf_size       dw      ?               ; size of SAP buffers
pool_blks      dw      ?               ; length in 16-byte, of buffer pool
pool_adrs      dd      ?               ; address of direct interface buf pool
chkt_exit      dd      ?               ; appendage, adapter check
stat_exit      dd      ?               ; appendage, ring status
err_exit       dd      ?               ; appendage, PC error exit
new_opts       dw      ?               ; new options (wrap is ignored)
mod_blk        ends


tx_blk         struc                   ; transmit.dir.frame parameter block
station        dw      ?               ; defines station sending data
trans_fs       db      ?               ; * stripped FS field (returned)
rsap           db      ?               ; RSAP (remote sap value)
queue_1        dd      ?               ; address of TX queue 1
queue_2        dd      ?               ; address of TX queue 2
buf_len_1      dw      ?               ; length of buffer 1
buf_len_2      dw      ?               ; length of buffer 2
buffer_1       dd      ?               ; address of the first transmit buffer
buffer_2       dd      ?               ; address of the second transmit buffer
tx_blk         ends


free_blk       struc                   ; buffer.free parameters
st_id          dw      ?               ; station id
buf_left       dw      ?               ; returns number of buffers left
resrvd         dd      ?               ;
first_buf      dd      ?               ; address of 1st buffer to free
free_blk       ends


get_blk        struc                   ; buffer.get parameters
stn_id         dw      ?               ; station id
buffr_lef      dw      ?               ; buffers left
buff_get       db      ?               ; number of buffers to get
resrv1         db      ?               ; 3 bytes not used
resrv2         db      ?
resrv3         db      ?
frst_buff      dd      ?               ; address of buffer obtained
get_blk        ends


rx_blk         struc                   ; receive parms
id             dw      ?               ; station id
user_len       dw      ?               ; size of user space in buffer
receiver       dd      ?               ; appendage for received data
first_bf       dd      ?               ; pointer to 1st buffer
opts           db      ?               ; receive options
rx_blk         ends


status_blk     struc                   ; parameter table for dir.status
encoded_addr   dw      ?               ; adapters permanent ring address
               dd      ?
node_adrs      dw      ?               ; ring address set by open
               dd      ?
group_adrs     dd      ?
func_adrs      dd      ?
               REPT    16
               db      ?
               ENDM
               REPT    3
               dd      ?
               ENDM
               dw      ?
status_blk     ends





mac_header     struc                   ; token ring mac header format
AC             db      ?               ; access control byte
FC             db      ?               ; frame control byte
               rdupb   dest,EADDR_LEN
               rdupb   source,EADDR_LEN
dsap           db      ?               ; LLC dsap
ssap           db      ?               ; LLC ssap
control        db      ?               ; control byte
ptype          db      ?
               db      ?               ; SNAP header
               db      ?
ethtype        dw      ?               ; EtherType
rif            dw      ?               ; optional RIF information
               REPT    8               ; segment information
               dw      ?               ; this is not its real position.
               ENDM                    ; this is a variable length field...
mac_header     ends


SA_blk         struc
               rdupb   s_addr,EADDR_LEN
RCF            db      ?
               db      ?
route          dw      ?
               REPT    7
               dw      ?
               ENDM    ?
t_mark         dd      ?               ; LRU time marker...
SA_blk         ends


ether_hdr      struc                           ; ethernet header format
               rdupb   ether_dest,EADDR_LEN
               rdupb   ether_src,EADDR_LEN
ether_type     dw      ?
ether_hdr      ends


first_buffer   struc                           ; Token receive 1st buffer
next_buffer    dd      ?                       ; ptr to next buffer
xx1            dw      ?
data_size      dw      ?                       ; size of user data
               REPT    12
               db      ?
               ENDM
user_data      db      ?
first_buffer   ends

user_data2     =       12                      ; 2nd buffer slightly different



;=============================================================================;
;======================= START OF PROGRAM CODE ===============================;
;=============================================================================;

code           segment word public
               assume  cs:code, ds:code

Token_address  db      EADDR_LEN dup (0)       ; holds my address after init.

ccb            ccb_blk <>                      ; scratch CCB
free_ccb       ccb_blk <>
tx_parms       tx_blk  <>                      ; transmit.dir.frame parms
iparms         init_parms <>                   ; init parms
pool_buff      db (pool_size+1)*16 dup (?)     ; buffer pool
free_parms     free_blk <>                     ; buffer.free parameters
get_parms      get_blk  <>                     ; buffer.get parameters

rx_ccb         ccb_blk <>                      ; receive CCB
rx_parms       rx_blk  <>                      ; receive parms


id_code        db 18 dup (0)   ; phony product id code
                               ; (a real space saver...)

ada_parm       ada_blk <0,0,0,0,0,0,num_rcv_buff,recv_size,tran_size,1,0,0,0>

dir_parm       dir_blk <0,pool_size,0,0,0,0,0,0,0>
dlc_parm       dlc_blk <0,0,0,0,0,0,0,0,0,0>
init_parm      o_parms <0,0,0,0>
initccb        ccb_blk <0,0,0,0,0,0,0>
toke_status    status_blk <>

modparms mod_blk <0,pool_size,0,0,0,0,0>
modccb ccb_blk < 0,modify_cmd,0,0,0,0,0>

lan_header  mac_header  <LLC_AC,LLC_FC>


;      SA RIF table has 1 entry in it by default, for the broadcast address.
;
SA_index dw            1
SA_size  dw            SA_table_size           ; size of table
         dw            0                       ; parsed from cmd line

SA_delta dw            0                       ; delta t for LRU find
         dw            0
SA_ord   dw            0                       ; ordinal for LRU entry

memory_needed  dw      0                       ; scratch for memory calculation


dir_interrupt  proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;  Exit: AX = CCB return code           ;
;---------------------------------------;
               clr_struc ccb                   ; initialize CCB
               mov     ax,token_card
               mov     [ccb.adapter],al
               mov     al,interrupt_cmd
               mov     [ccb.command],al
               call_token ccb                  ; call the interface
               call_wait  ccb                  ; wait for completion
               xor     ah,ah
               mov     al,[ccb.returncode]        ; return the result in AX
               ret
dir_interrupt  endp


dir_initialize proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;  Exit:                                ;
;       Ah = bring up return code       ;
;       Al = ccb return code            ;
;---------------------------------------;
               clr_struc ccb                   ; clear ccb
               clr_struc iparms                ; clear init parms (defaults)

               mov     ax,token_card           ; assign parameters
               mov     [ccb.adapter],al
               mov     al,initialize_cmd       ; command....
               mov     [ccb.command],al

               set_ptr ccb.parms,iparms        ; link to init parms

               call_token ccb                  ; call the interface
               call_wait  ccb                  ; wait for completion

               mov     ah,byte ptr iparms.bring_ups
               mov  al,[ccb.returncode]

               ret
dir_initialize endp


dir_open_adapter proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;  Exit: AX = CCB return code           ;
;---------------------------------------;
               mov     ax,token_card           ; get adapter number
               mov     [initccb.adapter],al

               mov     al,open_cmd
               mov     [initccb.command],al    ; 03h open_adapter command

               set_ptr ada_parm.id_addr,id_code
               set_ptr dir_parm.pool_addr,pool_buff
               set_ptr init_parm.adapt,ada_parm
               set_ptr init_parm.direct,dir_parm
               set_ptr init_parm.dlc,dlc_parm
               set_ptr initccb.parms,init_parm

               xor     al,al
               mov     [initccb.returncode],al

               call_token initccb              ; call the interface
               call_wait  initccb              ; wait for completion

               xor  ah,ah
               mov  al,[initccb.returncode]

               ret
dir_open_adapter endp


dir_close_adapter proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;  Exit: AX = CCB return code           ;
;---------------------------------------;
               clr_struc ccb                   ; clear CCB

               mov     ax,token_card           ; initialize local CCB
               mov     [ccb.adapter],al
               mov     al,close_cmd            ; close command
               mov     [ccb.command],al

               call_token ccb                  ; call the interface
               call_wait  ccb                  ; wait for completion

               xor     ah,ah
               mov     al,[ccb.returncode]        ; return the result
               ret
dir_close_adapter endp


dir_modify_open proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;  Exit: AX = CCB return code           ;
;---------------------------------------;
               mov     ax,token_card           ; get adapter number
               mov     [modccb.adapter],al     ; command is a constant in ccb

               set_ptr modparms.pool_adrs,pool_buff
               set_ptr modccb.parms,modparms

               xor     al,al
               mov     [modccb.returncode],al

               call_token modccb       ; call the interface
               call_wait  modccb       ; wait for completion

               xor     ah,ah
               mov     al,[modccb.returncode]
               ret
dir_modify_open endp


dir_restore_open proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;  Exit: AX = CCB return code           ;
;---------------------------------------;
               clr_struc ccb                   ; initialize CCB

               mov     ax,token_card
               mov     [ccb.adapter],al
               mov     al,restore_cmd          ; dir.restore.open.parms
               mov     [ccb.command],al

               call_token ccb                  ; call the interface
               call_wait  ccb                  ; wait for completion

               xor     ah,ah
               mov     al,[ccb.returncode]        ; return the result in AX
               ret
dir_restore_open endp


transmit_dir_frame proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;                                       ;
; Stack:                                ;
;       sp+14  dw [usr seg]             ;
;       sp+12  dw [usr ofs]             ;
;       sp+10  dw [usrsize]             ;
;       sp+8   dw [hdr seg]             ;
;       sp+6   dw [hdr ofs]             ;
;       sp+4   dw [hdrsize]             ;
;       sp+2   dw [ret ofs]             ;
;       sp+0   dw [  bp   ]             ;
;                                       ;
;  Exit:                                ;
;       Ah = stripped FS field          ;
;       al = CCB return code            ;
;---------------------------------------;

trans_frame    struc                           ; transmit_dir_frame parms
_x_bp          dw      ?                       ; old bp register
_x_ret_ofs     dw      ?                       ; near return address
header_size    dw      ?                       ; MAC header size
llc_header     dd      ?                       ; address of header
user_dsize     dw      ?                       ; user data size
user_daddress  dd      ?                       ; user data address
trans_frame    ends

               push    bp
               mov     bp,sp

               mov     ax,cs
               mov     ds,ax

               clr_struc ccb                   ; zero the CCB
               clr_struc tx_parms              ; zero parameters

               mov     ax,token_card           ; initialize local CCB
               mov     [ccb.adapter],al
               mov     al,trans_dir_cmd        ; transmit.dir.frame
               mov     [ccb.command],al
;
;      Set up transmit parameters, and link them to
;      the CCB...
;
               mov     ax,offset tx_parms
               mov     word ptr [ccb.parms],ax

               mov     ax,cs
               mov     word ptr [ccb.parms+2],ax
;
               mov     ax,STATION_ID

               mov     word ptr [tx_parms.station],ax

               mov     ax,[bp][user_dsize]
               mov     word ptr [tx_parms.buf_len_2],ax

               mov     ax,word ptr [bp][user_daddress]
               mov     word ptr [tx_parms.buffer_2],ax
               mov     ax,word ptr [bp][user_daddress+2]
               mov     word ptr [tx_parms.buffer_2+2],ax

               mov     ax,[bp][header_size]
               mov     word ptr [tx_parms.buf_len_1],ax

               mov     ax,word ptr [bp][llc_header]
               mov     word ptr [tx_parms.buffer_1],ax
               mov     ax,word ptr [bp][llc_header+2]
               mov     word ptr [tx_parms.buffer_1+2],ax

               call_token ccb                  ; call the interface
               call_wait  ccb                  ; wait for completion

               mov     ah,[tx_parms.trans_fs]
               mov     al,[ccb.returncode]

               pop     bp

               ret     (SIZE trans_frame)-4    ; (forget bp and ret address)
transmit_dir_frame endp


buffer_free    proc
;---------------------------------------;
; Entry:                                ;
;      sp+4    [ seg 1st buffer ]       ;
;      sp+2    [ ofs 1st buffer ]       ;
;      sp+0    [ bp             ]       ;
;                                       ;
;  Exit: AX = CCB return code           ;
;---------------------------------------;
               mov     ax,cs
               mov     ds,ax

               push    bp
               mov     bp,sp

               clr_struc free_ccb
               clr_struc free_parms

               mov     ax,token_card           ; initialize local CCB
               mov     [free_ccb.adapter],al

               mov     al,free_ccb_cmd         ; buffer.free
               mov     [free_ccb.command],al

               mov     ax,offset free_parms
               mov     word ptr [free_ccb.parms],ax

               mov     ax,cs
               mov     word ptr [free_ccb.parms+2],ax
;
;      Load free_parms to release buffer
;
               mov     ax,STATION_ID
               mov     word ptr [free_parms.st_id],ax

               mov     ax,word ptr [bp][4]
               mov     word ptr [free_parms.first_buf],ax
               mov     ax,word ptr [bp][6]
               mov     word ptr [free_parms.first_buf+2],ax

               call_token free_ccb             ; call the interface

               xor     ah,ah
               mov     al,[free_ccb.returncode]   ; return the result in AX

               pop     bp

               ret     4
buffer_free    endp


recv_cancel    proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;                                       ;
;  Exit:                                ;
;       AX = CCB return code            ;
;                                       ;
;---------------------------------------;
               mov     ax,token_card           ; initialize local CCB
               mov     [ccb.adapter],al

               mov     al,recv_can_cmd         ; receive.cancel
               mov     [ccb.command],al

               xor     ax,ax                   ; word/bytes set to 0
               mov     [ccb.returncode],al
               mov     [ccb.work],al
               mov     word ptr [ccb.pointer],ax
               mov     word ptr [ccb.pointer+2],ax
               mov     word ptr [ccb.complt],ax
               mov     word ptr [ccb.complt+2],ax
               mov     word ptr [ccb.parms+2],ax

               mov     ax,STATION_ID
               mov     word ptr [ccb.parms],ax

               call_token ccb                  ; call the interface
               call_wait  ccb                  ; wait for completion

               xor     ah,ah
               mov     al,[ccb.returncode]        ; return the result in AX
               ret

recv_cancel    endp


open_receive proc
;---------------------------------------;
; Entry: token_card (0 or 1)            ;
;                                       ;
; Stack:                                ;
;       sp+6   dw [ret ofs]             ;
;       sp+4   dw [  bp   ]             ;
;       sp+2   dw [  ds   ]             ;
;       sp+0   dw [  es   ]             ;
;                                       ;
;  Exit:                                ;
;       AX = CCB return code            ;
;---------------------------------------;
               mov     ax,cs
               mov     ds,ax

               clr_struc rx_ccb
               clr_struc rx_parms;

               mov     ax,token_card           ; initialize local CCB
               mov     [rx_ccb.adapter],al

               mov     al,open_recv_cmd        ; receive
               mov     [rx_ccb.command],al

               mov     ax,offset recv_complt
               mov     word ptr [rx_ccb.complt],ax
               mov     ax,cs
               mov     word ptr [rx_ccb.complt+2],ax
;
;      Link receive parameters table
;
               mov     ax,offset rx_parms
               mov     word ptr [rx_ccb.parms],ax
               mov     ax,cs
               mov     word ptr [rx_ccb.parms+2],ax

               mov     ax,STATION_ID
               mov     word ptr [rx_parms.id],ax

               mov     al,RECV_OPTIONS
               mov     rx_parms.opts,al

               mov     ax,offset _receiver
               mov     word ptr [rx_parms.receiver],ax
               mov     ax,cs
               mov     word ptr [rx_parms.receiver+2],ax
;
;       Note: The receive request is submitted, but it doesn't 'complete'
;             here.  So DON'T wait for a completion code!
;
               call_token rx_ccb               ; call the interface

               xor     ah,ah
               mov     al,[rx_ccb.returncode]     ; return the result
               ret
open_receive endp


recv_complt proc
;
;      This procedure is an interrupt routine called by the Token Card
;      when receive completion occurs after an error.  All it should have
;      to do is resubmit the receive request, then exit.
;
               pushf
               push ax
               push bx
               push cx
               push dx
               push si
               push di
               push ds
               push es
               push bp
               sti

               marker E,R
               call open_receive               ; restart communications

               pop  bp
               pop  es
               pop  ds
               pop  di
               pop  si
               pop  dx
               pop  cx
               pop  bx
               pop  ax
               popf
               iret
recv_complt    endp


recv_frame     struc                           ; stack frame for receiver
               RIF_size dw     ?               ; size of RIF in packet.
               user_buff dd    ?               ; pointer to users buffer
               user_size dw    ?               ; size of buffer needed/used
               _BP     dw      ?               ; register set on stack
               _ES     dw      ?
               _DS     dw      ?
               _DI     dw      ?
               _SI     dw      ?
               _DX     dw      ?
               _CX     dw      ?
               _BX     dw      ?
               _AX     dw      ?
recv_frame     ends


_receiver      proc
;
;      This interrupt procedure is called when the Token Ring
;      card has data.
;
;      On entry, ds:si points to the CCB
;                es:bx points to the receive first_buffer in the chain.
;
;
                pushf                          ; save CPU environment
                push   ax
                push   bx
                push   cx
                push   dx
                push   si
                push   di
                push   ds
                push   es
                push   bp

                push   ax                      ; local variables: size of receive data
                push   ax                      ; segment of user supplied buffer
                push   ax                      ; offset of user supplied buffer
                push   ax

                mov    bp,sp                   ; set stack frame reference

                sti                            ; enable further int activity

                marker R,X
;
;      If the source address is mine, ignore the transmission!
;
               mov     ax,cs
               mov     ds,ax
               mov     si,offset Token_address
               mov     di,bx
               add     di,user_data+source     ; ds:si -> token address
                                               ; es:di -> pkt
               mov     cx,EADDR_LEN
               push    word ptr es:[di]        ; save msb of address

               mov     al,es:[di]
               and     al,not_SA_mask          ; drop SA indicator
               mov     es:[di],al              ; fix the bit during compare
;
;      patch to fix direction bit problem...
;
               cld
               repe    cmpsb                   ; do string compare now
               jne     not_mine

               mov     di,bx
               add     di,user_data+source
               pop     word ptr es:[di]        ; restore token address

               jmp     drop_buffer             ; from me, ignore it!

;      es:bx -> 1st receive buffer
not_mine:
               mov     di,bx
               add     di,user_data+source
               pop     word ptr es:[di]        ; save MSB of source address

               xor     ax,ax
               mov     cx,ax
               mov     word ptr [bp][RIF_size],ax ; zero RIF size value

get_size:
               mov     ax,es
               or      ax,bx
               je      got_size
               add     cx,word ptr es:[bx][data_size]
               push    word ptr es:[bx][next_buffer]
               push    word ptr es:[bx][next_buffer+2]
               pop     es                      ; ************************
               pop     bx                      ; ************************
               jmp     get_size

got_size:
               sub     cx,LLC_info_size        ; adjust size for Token

               mov     ax,word ptr [bp][_ES]
               mov     es,ax
               mov     bx,word ptr [bp][_BX]   ; get buffer address
               mov     al,es:[bx][user_data+source]
               and     al,SA_mask              ; if SA, adjust total
               je      got_no_fix

;
;      How big is the RIF?
;
               mov     ax,word ptr es:[bx][user_data+source+EADDR_LEN]
               and     ax,RIF_size_mask
               mov     word ptr [bp][RIF_size],ax
;
;      Take away size of RIF.  Upper level doesn't get it.
;
               sub     cx,ax                   ; subtract RIF size

got_no_fix:
;
;      Min Ethersize is RUNT, so check it first, round up if necessary
;
               cmp     cx,RUNT
               jge     no_adjust_cx
               mov     cx,RUNT

no_adjust_cx:
               mov     word ptr [bp][user_size],cx ; save size of receive data
;
;      Call recv_find to determine if the receiver wants the data,
;      and where it should be stored.
;
               mov     ax,word ptr [bp][_ES]   ; reload segment/offset
               mov     es,ax
               mov     di,word ptr [bp][_BX]   ; points to E-type in Token
               add     di,40                   ; assumes Buffer 1 format,
               add     di,word ptr [bp][RIF_size] ; RIF size added now...
;
;      if ARP type, convert hardware address space value back to Ethernet...
;
               mov     ax,word ptr es:[di]     ;  -> EtherType in SNAP
               cmp     ax,ARP_type             ; ARP packet?
               jne     not_rx_arp
               mov     ax,ARP_Eth_hwr
               mov     word ptr es:[di][2],ax  ; fix 1st field of data

not_rx_arp:
               mov     dl,BLUEBOOK
               call    recv_find               ; es:di -> Snap E-Type, CX = data size

               mov     word ptr [bp][user_buff],di
               mov     ax,es
               mov     word ptr [bp][user_buff+2],ax

               or      ax,di
               jne     do_copy
               jmp     drop_buffer             ; if ptr is 0, doesn't want it!
;
;      Copy Token Ring data to users Ethernet format receive buffer.
;
do_copy:
;
;      Since the upper level wants it, better keep the RIF info in case we
;      want to send a return message.
;
               mov     ax,word ptr [bp][_ES]
               mov     ds,ax
               mov     si,word ptr [bp][_BX]
               add     si,user_data+source     ; 1st byte of source address
               call    add_entry               ; do table maintenance
;
;      Copy data to the users buffer
;
               mov     di,word ptr [bp][user_buff] ; Ethernet destination buffer
               mov     ax,word ptr [bp][user_buff+2]
               mov     es,ax

               mov     si,word ptr [bp][_BX]   ; Token Ring Source buffer
               mov     bx,si
               mov     ax,word ptr [bp][_ES]
               mov     ds,ax
               add     si,user_data            ; offset to user data
;
;      ds:si points to 1st token buffer
;      es:di points to ethernet format buffer

;      1. Copy Token/Ether  dest/source field to users buffer
;         after stripping off the SA bit in the source address.

               mov    al,byte ptr ds:[si+source]    ;****************
               and    al,07Fh                       ;***** NEW ******
               mov    byte ptr ds:[si+source],al    ;****************

               mov    cx,EADDR_LEN*2           ; 2 address fields
               add    si,2                     ; offset to dest field
               cld
               rep     movsb

               add     si,LLC_info_size-2      ; assume no RIF,
               add     si,word ptr [bp][RIF_size] ; compensate for RIF

               movsw                           ; get ethertype

               mov     cx,[bx][data_size]      ; get length of data
               sub     cx,MAC_hdr_size         ; drop MAC header stuff
               sub     cx,[bp][RIF_size]       ; compensate for RIF

copy_buffer:
               cld
               rep     movsb                   ; copy rest of data to Ether..
               push    word ptr [bx][next_buffer]
               push    word ptr [bx][next_buffer+2]
               pop     ds
               pop     si
               mov     bx,si                   ; bx is base of buffer
               add     si,user_data2           ; si offsets to user data
               mov     cx,[bx][data_size]      ; length of user data
               mov     ax,ds
               or      ax,bx                   ; NULL pointer?
               jne     copy_buffer             ; no, keep copying...
;
;      Tell Upper layer I copied the data into his buffer...
;
               mov     si,word ptr [bp][user_buff]
               mov     ax,word ptr [bp][user_buff+2]
               mov     ds,ax
               mov     cx,word ptr [bp][user_size]

               call    recv_copy
;
;      _ES:_BX points to 1st receive buffer, drop it, then exit
;
drop_buffer:
               mov     ax,word ptr [bp][_ES]   ; tell Token, drop buffer
               mov     bx,word ptr [bp][_BX]
               push    ax
               push    bx
               call    buffer_free             ; seg:offs of 1st buff on stack

               pop     ax                      ; vacuum local variables off
               pop     ax                      ; the stack
               pop     ax
               pop     ax

               pop     bp
               pop     es
               pop     ds
               pop     di
               pop     si
               pop     dx
               pop     cx
               pop     bx
               pop     ax
               popf

               marker  r,x

               iret

_receiver      endp


comp_adr       proc
;
;      Compare two token address values, set flags without affecting any
;      other registers.  JE or JNE to test result.
;
               push    si
               push    di
               push    cx
               cld
               mov     cx,EADDR_LEN
               repe    cmpsb
               pop     cx
               pop     di
               pop     si
               ret
comp_adr       endp


make_adr       proc
;
;      Given CX as an ordinal (1..n) into the RIF_cache, build the address
;      in ES:DI
;
               push    ax
               push    cx
               dec     cx
               xor     di,di
               cmp     cx,0
               je      make_adr1
make_adr0:
               add     di,SIZE SA_blk          ; build index into SA_table
               loop    make_adr0

make_adr1:
               mov     ax,cs                   ; segment of table
               mov     es,ax
               add     di,offset RIF_cache     ; + offset to base of table
               pop     cx
               pop     ax
               ret
make_adr       endp



add_entry      proc
;
;      DS:SI -> Token Address, make an entry in the SA routing table
;
               mov     cx,EADDR_LEN            ; don't add broadcasts to table
               push    si                      ; leave the default intact
               cld

add_entry0:
               lodsb                           ; Broadcast?
               cmp     al,broadcast_byte
               loope   add_entry0
               pop     si
               je      add_done                ; yes...

               mov     al,ds:[si]              ; is it a source route address?
               and     al,SA_mask
               je      add_done
;
;      Is it in the table?

               call    loc_entry
               jc      add_done                ; c = 1, its in the table.
;
;      if its not in the table, try to put it in a 'new' slot...
;
               mov     cx,word ptr cs:[SA_index]
               cmp     cx,word ptr cs:[SA_size]
               jne     lotsa_room              ; room in table, make flat entry
;
;      If no new slots available, determine LRU entry and overwrite it...
;
               push    ds                      ; save initial pointer
               push    si
               call    find_lru                ; returns cx ordinal into table
               pop     si                      ; restore initial pointer
               pop     ds
               jmp     do_entry
;
;      Update entry pointer, overwrite a new slot in the table...
;
lotsa_room:    inc     cx
               mov     word ptr cs:[SA_index],cx ; advance the table index

;      before entry is made, the broadcast bit must be cleared so bridges
;      will leave the segment values alone.

do_entry:      mov     al,byte ptr ds:[si+EADDR_LEN]
               and     al,07Fh                 ; clear broadcast bit
               mov     byte ptr ds:[si+EADDR_LEN],al

               call    make_adr                ; build destination address
               push    si
               push    di
               push    es
               mov     cx,SIZE SA_blk
               cld
               rep     movsb                   ; copy RIF into table
               pop     es
               pop     di
               pop     si
;
;      Put time stamp on the new entry for LRU algorithm
;
               ticks                           ; cx:dx = ticks count
               mov     word ptr es:[di+t_mark],dx
               mov     word ptr es:[di+t_mark+2],cx

add_done:
               ret
add_entry      endp


find_lru       proc
;
;      locates oldest entry in the RIF_cache, returns the ordinal number in
;      CX.  The first entry is NEVER chosen because it contains the broadcast
;      route predefined in the table!
;
;      CX = 2 .. cs:[SA_size]  ( range to be searched )
;
;      SA_delta  dword contains time difference for last one checked
;      SA_ord    word  contains the ordinal of the least recently used
;
       xor     ax,ax                           ; zero search variables
       mov     word ptr cs:[SA_delta],ax
       mov     word ptr cs:[SA_delta+2],ax
       mov     word ptr cs:[SA_ord],ax

       mov     cx,2                            ; 1st entry is NEVER touched!

find_loop:
       call    make_adr                        ; es:di -> current entry
       push    cx                              ; save index
       ticks                                   ; cx:dx = current ticks
       sub     cx,word ptr es:[di+t_mark+2]    ; ticks = mark
       sbb     dx,word ptr es:[di+t_mark]
;
;      if the result < 0, then ticks went past midnight.  Add 24 hrs in
;      ticks to get the real delta t.
;
       jge     _no_fix
       add     cx,((hrs24 shr 16) and 0ffffh)  ; longint to fix delta t
       adc     dx,(hrs24 and 0ffffh)           ; add 24 hours in ticks to

_no_fix:
;
;      If the result is greater than the stored SA_delta, replace it with
;      this entry, then continue the loop...
;
       push    cx                              ; save hi/low delta t result
       push    dx
       sub     cx,word ptr cs:[SA_delta+2]
       sbb     dx,word ptr cs:[SA_delta]
       jl      _no_replace

;
;      replace SA_ord and SA_delta with entry that has been around longer
;
       pop     cs:[SA_delta]                   ; pull lo/hi delta t result
       pop     cs:[SA_delta+2]
       pop     cx                              ; get ordinal...
       mov     word ptr cs:[SA_ord],cx         ; replace SA_ord...
       jmp     _next_find

_no_replace:
       pop     cx
       pop     cx
       pop     cx

_next_find:
       inc     cx
       cmp     cx,word ptr cs:[SA_size]
       jle     find_loop
       jmp     _find_exit

_find_exit:
       mov     cx,SA_ord                       ; return ordinal of LRU
       ret

find_lru       endp



loc_entry      proc
;
;      DS:SI -> Token Address, locate entry in the SA routing table
;      ES:DI -> SA information in table, if carry is set.
;
               mov     cx,word ptr cs:[SA_index]  ; any entries in table?
               cmp     cx,0
               je      loc_done


loc_entry0:
               call    make_adr                ; build address into table
               call    comp_adr                ; compare Token Addresses
               je      loc_found               ; if matches, in table
               loop    loc_entry0

loc_done:
               clc
               ret

loc_found:                                     ; ES:DI points to matching entry
               ticks                           ; cx:dx = ticks value for
               mov     word ptr es:[di+t_mark],dx   ; refresh of existing
               mov     word ptr es:[di+t_mark+2],cx ; entry...

               stc
               ret

loc_entry endp


;==============================================================================
;
;=================== DRIVER CODE  =============================================
;
;==============================================================================
;
;
;      Token Ring Card number, parsed from command line
;
token_card     dw      0                       ; default adapter number 0
               dw      0                       ; this prevents type error.

toke_ad_name   db      "Adapter (0 or 1) ",'$'
toke_sa_name   db      " RIF Cache Size: ",'$'

dir_open_mode  db      0                       ; set <> 0 if modify_open used
msg_initialize db      " INIT: Initializing the adapter...",CR,LF,'$'
msg_opening    db      "       Opening the adapter...",CR,LF,'$'
msg_complete   db      "       IBMTOKEN - Initialization complete.",CR,LF,'$'
msg_bad_adapter db     "ERROR: adapter must be 0 or 1.",CR,LF,'$'
no_mem_cache   db      "ERROR: RIF_cache value out of range!",CR,LF
msg_bad_init   db      "ERROR: Initialization Failed.",CR,LF
               db      "       Installation aborted!",CR,LF,'$'
msg_receiving  db      "       Starting receiver process...",CR,LF,'$'

       public  int_no
int_no db      0,0,0,0                 ;must be four bytes long for get_number.

       public  driver_class, driver_type, driver_name, driver_function, parameter_list
driver_class   db      BLUEBOOK,0      ;from the packet spec
driver_type    db      1               ;from the packet spec
driver_name    db      'IBMTokenR',0   ;name of the driver.
driver_function db      2

parameter_list  label   byte
        db      1       ;major rev of packet driver
        db      9       ;minor rev of packet driver
        db      14      ;length of parameter list
        db      EADDR_LEN       ;length of MAC-layer address
        dw      GIANT   ;MTU, including MAC headers
        dw      MAX_MULTICAST * EADDR_LEN       ;buffer size of multicast addrs
        dw      0       ;(# of back-to-back MTU rcvs) - 1
        dw      0       ;(# of successive xmits) - 1
        dw      0       ;Interrupt # to hook for post-EOI
                        ;processing, 0 == none,

       public  rcv_modes
rcv_modes      dw      4               ;number of receive modes in our table.
               dw      0,0,0,rcv_mode_3


sp_frame       struc
__bp           dw      ?
u_size         dw      ?
u_data         dd      ?
u_snap         dw      ?
sp_frame       ends


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
               sti
               marker  T,X

;
;      construct local stack frame
;
               mov     ax,word ptr ds:[si][ether_type]
               push    ax
               push    ds                      ; pointer to user data packet
               mov     ax,si
               add     ax,SIZE ether_hdr
               push    ax                      ; adjusted userdata pointer
               sub     cx,SIZE ether_hdr       ; adjust size for send
               push    cx
               push    bp
               mov     bp,sp                   ; use sp_frame for reference


               mov     ax,cs                   ; set dest pointer to LLC header
               mov     es,ax

               mov     di,offset lan_header.dest
               mov     cx,EADDR_LEN*2          ; copy dest|source fields
               cld
               rep  movsb
;
;      ds:si -> dest address, locate it in the SA RIF table
;
               mov     ax,cs
               mov     ds,ax
               mov     si,offset lan_header.dest ; building LAN header info here

; Set bit 7 of address, force lookup in table.  If it is found, add RIF,
; otherwise, assume the local net.

               mov     al,byte ptr ds:[si]
               or      al,080h
               mov     byte ptr ds:[si],al

               call loc_entry
               jc   send_pkt1                  ; yes, copy out RIF information
;
;      add the rest of the fields to the LLC header, and set the size
;      for the transmit call.
;
               mov     al,byte ptr ds:[si]     ; dest address 1st byte,
                                               ; lop off SA bit
               and     al,not_SA_mask
               mov     byte ptr ds:[si],al

               mov     al,byte ptr ds:[si][EADDR_LEN]  ; force U bit high
               or      al,080h
               mov     byte ptr ds:[si][EADDR_LEN],al

               add     si,EADDR_LEN*2          ; point after dest/source addrs.
               mov     ax,es
               mov     bx,ds
               mov     ds,ax
               mov     es,bx
               xchg    si,di

;      addresses acquired from ARP default to broadcast mode, using phony
;      RIF info so the bridge will pass the packet along.
;
               mov     ax,phony_RIF            ; copy phony RIF to LLC buffer,
               cld                             ; so 'local' can cross bridge..
               stosw
               jmp     send_pkt2

send_pkt1:
;
;      source/dest address need to be adjusted for routing.
;
               mov     cx,EADDR_LEN
               push    si

send_pkt3:
               mov     al,byte ptr ds:[si]
               inc     si
               cmp     al,broadcast_byte
               loope   send_pkt3
               pop     si
               je      send_pkt4               ; broadcast address?


               mov     al,byte ptr ds:[si]     ; dest, lop off SA bit
               and     al,not_SA_mask
               mov     byte ptr ds:[si],al

send_pkt4:
               mov     al,byte ptr ds:[si][EADDR_LEN]
               or      al,SA_mask              ; set SA bit for RIF
               mov     byte ptr ds:[si][EADDR_LEN],al

               add     si,EADDR_LEN*2          ; ds:si -> dest for RIF info.
               add     di,EADDR_LEN            ; es:di -> RIF info, addr fld

               mov     bx,si                   ; keep RIF pos in LLC header

               mov     ax,word ptr es:[di]     ; get rif info
               and     ax,RIF_size_mask        ; get length of RIF
               mov     cx,ax
               mov     ax,es
               mov     dx,ds
               mov     ds,ax
               mov     es,dx
               xchg    si,di
               cld
               rep     movsb

;
;      Got RIF, fix direction bit.
;
               mov     al,es:[bx][1]           ; RIF+1 = dir bit location
               xor     al,RIF_dir_bit          ; change direction
               mov     es:[bx][1],al

send_pkt2:
;
;      es:di -> target in LLC for the rest of the header,
;      add LLC and SNAP information.
;
               mov     ax,LLC_SSAP_DSAP        ; build LLC portion
               stosw
               mov     al,LLC_CON
               stosb
               xor     ax,ax                   ; 3 bytes for Ptype in SNAP
               stosw
               stosb
               mov     ax,word ptr [bp][u_snap]
               stosw
;
;      header is complete.
;
               sub     di,offset lan_header    ; calculate size of LLC header
;
;      stack parameters for Transmit
;
               pop     bp

               mov     ax,cs
               mov     ds,ax                   ; set ds for transmit entry
               push    ax
               mov     ax,offset lan_header
               push    ax
               push    di

;--------------------------------------------------------------------
;       If Users data is an ARP packet, 0806 type, then the 1st field
;       must be changed from 0001 to 0006 for Token Ring broadcast, or
;       no one will respond.  Funny, eh?  Another thing.  Since an ARP
;       packet is smaller than the min size for Ethernet, the caller rounds
;       up the size.  I have to change it back, or some Token hosts won't
;       ARP for me.  The nerve!
;
               push    bp
               mov     bp,sp
               mov     ax,word ptr [bp][14]    ; offset to ARP type on stack...
               pop     bp

               cmp     ax,ARP_type             ; network byte order is backwards
               jne     not_tx_arp
;
;      The first field of the ARP data has to be changed from 0001 to 0006
;
               push    bp                      ; use trans_frame
               push    bp                      ; dummy entry on stack...
               mov     bp,sp
               mov     ax,word ptr [bp][user_daddress+2] ; addr of user data
               mov     es,ax
               mov     di,word ptr [bp][user_daddress]

               mov     ax,ARP_Tok_hwr
               mov     word ptr es:[di],ax     ; change ARP type to 0006

               mov     ax,ARP_packet_size      ; 28  bytes in an ARP packet
               mov     word ptr [bp][user_dsize],ax ; reset users data size

               pop     bp                      ; pop fake return address
               pop     bp                      ; pop real bp

not_tx_arp:
               call    transmit_dir_frame      ; send the packet

               pop     ax                      ; vacuum
               clc
               ret


               public  get_address
get_address:
;get the address of the interface.
;enter with es:di -> place to get the address, cx = size of address buffer.
;exit with nc, cx = actual size of address, or cy if buffer not big enough.
               assume  ds:code
;
               push    di
               push    es
               push    cx

               mov     ax,cs
               mov     ds,ax
               clr_struc ccb
               clr_struc toke_status

               mov     ax,token_card           ; set adapter number
               mov     [ccb.adapter],al
               mov     al,get_status_cmd       ; get status returns address
               mov     ccb.command,al
               mov     ax,offset toke_status
               mov     word ptr [ccb.parms],ax
               mov     ax,ds
               mov     word ptr [ccb.parms+2],ax

               call_token ccb
               call_wait  ccb

               mov     si,offset toke_status.node_adrs
               pop     cx
               pop     es
               pop     di
               cmp     cx,EADDR_LEN            ; got the room for it?
               jb      get_address_fail

               cld
               mov     cx,EADDR_LEN
               rep     movsb
               mov     cx,EADDR_LEN
               clc
               ret

get_address_fail:
               stc
               ret


               public  set_address
set_address:
;enter with ds:si -> Ethernet address, CX = length of address.
;exit with nc if okay, or cy, dh=error if any errors.
               assume  ds:nothing
               clc
               ret


rcv_mode_3:
;receive mode 3 is the only one we support, so we don't have to do anything.
       ret


       public  set_multicast_list
set_multicast_list:
;enter with es:di ->list of multicast addresses, cx = number of bytes.
;return nc if we set all of them, or cy,dh=error if we didn't.
       mov     dh,NO_MULTICAST
       stc
       ret


       public  get_multicast_list
get_multicast_list:
;return with nc, es:di ->list of multicast addresses, cx = number of bytes.
;return cy, NO_ERROR if we don't remember all of the addresses ourselves.
;return cy, NO_MULTICAST if we don't implement multicast.
       mov     dh,NO_MULTICAST
       stc
;
;        DEBUG: get_multicast_list reports RIF accress instead, so I can
;               check its contents from UL.
;
       IF debug NE 0

       mov     ax,cs
       mov     _RES[bp],ax
       mov     ax,offset RIF_cache
       mov     _RDI[bp],ax
       mov     ax,word ptr cs:SA_index
       mov     _RCX[bp],ax
       clc
       ENDIF
       ret


               public  reset_interface
reset_interface:
;reset the interface.
               assume  ds:code
               ret


;called when we want to determine what to do with a received packet.
;enter with cx = packet length, es:di -> packet type, dl = packet class.
               extrn   recv_find: near

;called after we have copied the packet into the buffer.
;enter with ds:si ->the packet, cx = length of the packet.
               extrn   recv_copy: near

               extrn   count_in_err: near
               extrn   count_out_err: near

               public  recv
recv:
;called from the recv isr.  All registers have been saved, and ds=cs.
;Upon exit, the interrupt will be acknowledged.
               assume  ds:code
               ret

        public  terminate
terminate:
        sti
        call    recv_cancel             ; shut down call-backs
        call    dir_close_adapter       ; close out the adapter
        ret

       public  recv_exiting
recv_exiting:
;called from the recv isr after interrupts have been acknowledged.
;Only ds and ax have been saved.
       assume  ds:nothing
       ret


;=======================================================================;
;============================== RIF Cache ==============================;
;=======================================================================;
;      The RIF_cache starts here.  Code appearing after the RIF cache   ;
;      will be lost after initialization.  It becomes part of the RIF   ;
;      cache area, or is released back to DOS.  The program code and    ;
;      RIF_cache form a contiguous block in memory.  The first entry    ;
;      in the cache is for the broadcast address.  This entry is NEVER  ;
;      bumped from the cache!                                           ;
;=======================================================================;

RIF_cache:
               SA_blk <0ffh,0ffh,0ffh,0ffh,0ffh,0ffh,82h,0A0h,0>



               public  usage_msg
usage_msg      db      CR,LF
               db      "usage: IBMTOKEN <packet_int_no> <adapter> <RIF_cache>",CR,LF,LF
               db      "where: packet_int_no     is the interrupt vector",CR,LF
               db      "       adapter           is adapter 0 or 1",CR,LF
               db      "       RIF_cache         is number of hosts in RIF cache",CR,LF,CR,LF
               db      " note: IBM (tm) LAN Support Program required.",CR,LF,'$'

               public  copyright_msg
copyright_msg  db      CR,LF,"Token Ring Driver (3C501 emulation) Version "
               db      '0'+majver,".",'0'+version,CR,LF
               IF alpha NE 0
               db      "ALPHA TEST VERSION ",'0'+alpha_ver,CR,LF
               ENDIF
               db      "portions -",CR,LF
               db      "Copyright 1989, Queens University",CR,LF
               db      "Computing and Communications Services",CR,LF
               db      "Written by Brian Fisher",CR,LF
               IF  debug NE 0
               db      "*** DEBUG VERSION ***",CR,LF
               ENDIF
               db      LF,LF
               db      '$'

               extrn   set_recv_isr: near

;enter with si -> argument string, di -> wword to store.
;if there is no number, don't change the number.
               extrn   get_number: near

               public  parse_args
parse_args:
;
;      si points to next argument of command line...
;
               mov     di,offset token_card    ; next argument is card number
               call    get_number
               jc      _parse_exit
               mov     di,offset SA_size       ; see if cache size ok...
               call    get_number
_parse_exit:   clc
               ret

et_error:                              ; token adapter number out of range
;if we got an error,
               print$  msg_bad_adapter
               stc
               ret

sa_error:                              ; RIF cache allocation problem
; if no memory for SA table,
               print$  no_mem_cache
               stc
               ret

et_init_error:
               call    dir_close_adapter       ; make sure adapter is closed
               print$  msg_bad_init
               stc
               ret

               public  etopen
;
;      Initialize the IBM Token Ring Adapter, determine memory requirements
;      for the RIF_cache.
;
etopen:
               mov     ax,cs
               mov     ds,ax
               assume cs:code, ds:code

               mov     ax,token_card           ; check card parm for range
               cmp     ax,1
               jg      et_error
               cmp     ax,0
               jl      et_error                ; less than zero?

               mov     ax,cs:[SA_size]         ; is the requested RIF cache
               cmp     ax,SA_min_size          ; size in range?
               jl      SA_error
               cmp     ax,SA_max_size
               jg      SA_error

               mov     bx,SIZE SA_blk          ; multiply to get bytes needed
               imul    bx
               add     ax,(offset RIF_cache)+15    ; offset + round up value
               adc     dx,0
               jne     SA_error                ; dx <> 0, way out of range!

               mov     cs:[memory_needed],ax   ; bytes needed for code+RIF

;      Note: If a memory allocation error occurs when this init routine
;      returns, the system will crash because the call backs are already
;      defined for LAN Support.  I do a resize here, just to verify that
;      it will work.
;
               add     ax,15
               mov     cl,4
               shr     ax,cl
               mov     bx,ax
               mov     ax,cs                   ; get segment
               mov     es,ax
               mov     ah,4Ah                  ; resize memory block
               int     21h
               jc      SA_error                ; c=1, error doing memory

do_init:
               print$  msg_initialize          ; initialize the adapter
               call    dir_initialize
               cmp     ax,0
               jne     et_init_error

               print$  msg_opening             ; open the adapter
               call    dir_open_adapter
               cmp     ax,0
               jne     et_init_error

no_init:
               print$  msg_receiving           ; set up receive routines
               call    open_receive
               cmp     ax,0ffh
               jne     et_init_error           ; no receive open...

               print$  msg_complete            ; card ready to go

               mov     ax,cs
               mov     es,ax                   ; get Token Address for _receiver
               mov     di,offset Token_address
               mov     ds,ax
               mov     cx,6
               call    get_address

all_ok:
;if all is okay, tell caller how much memory I want...
               mov     dx,cs:[memory_needed]
               clc
               ret

	       public	print_parameters
print_parameters:
               ret

code           ends
               end

;******************************************************************************
;
;*     End of file: IBMTOKEN.ASM                                              *
;
;******************************************************************************
;
