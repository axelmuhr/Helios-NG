; LANCE.INC
;
; lance registers
;
CSR0	equ	0		;control and status register 0
CSR1	equ	1
CSR2	equ	2
CSR3	equ	3
;
;-------------------------------------------------------------------------
; control and status register 0
; rap = 0
;
;	|---------------|---------------|---------------|---------------|
;	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| e | b | c | m	| m | r	| t | i	| i | i | r | t	| t | s	| s | i	|
;	| r | a	| e | i	| e | i	| i | d	| n | n | x | x	| d | t	| t | n	|
;	| r | b	| r | s	| r | n	| n | o	| t | e | o | o	| n | o	| r | i	|
;	|   | l	| r | s	| r | t	| t | n	| r | a | n | n	| d | p	| t | t	|
;	|---------------|---------------|---------------|---------------|
;
L_ERR	equ	8000h		;or of babl, cerr, miss, merr (read only)
L_BABL	equ	4000h		;transmit timeout error	(write 1 clear)
L_CERR	equ	2000h		;collision error (sqe error) (write 1 clear)
L_MISS	equ	1000h		;no buffer on receive (write 1 clear)
;
L_MERR	equ	0800h		;bus timeout error (write 1 clear)
L_RINT	equ	0400h		;receive interrupt (write 1 clear)
L_TINT	equ	0200h		;transmit interrupt (write 1 clear)
L_IDON	equ	0100h		;initialization done (write 1 clear)
;
L_INTR	equ	0080h	;interrupt (or of babl,miss,merr,rint,tint,idon
L_INEA	equ	0040h		;interrupt enable
L_RXON	equ	0020h		;receiver on
L_TXON	equ	0010h		;transmitter on
;
L_TDND	equ	0008h		;transmit demand
L_STOP	equ	0004h		;stop
L_STRT	equ	0002h		;start
L_INIT	equ	0001h		;initialize
;
L_MASK	equ	l_babl+l_cerr+l_miss+l_merr+l_rint+l_tint



;-----------------------------------------------------------------------
; control and status register 1
; rap = 1
;
;  Accessable when csr0 stop = 1
;
;	 ---------------------------------------------------------------
;	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                  iadr  (bits 1 - 15)                      | 0 |
;	 ---------------------------------------------------------------
;
; iadr - The low order 16 bits of the address of the first
;         in the initialization Block.
;
;----------------------------------------------------------------------
; control and status register 2
; rap = 2
;
;  Accessable when csr0 stop = 1
;
;	|---------------|---------------|---------------|---------------|
;	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |       iadr (bits 16 - 32)     |
;	 ---------------------------------------------------------------
;
; iadr - (bits 16 - 32) The hi order 8 bits of the address of the first
;         in the initialization Block.
;
;-------------------------------------------------------------------------
; control and status register 3
; rap = 3
;
;  Accessable when csr0 stop = 1
;
;	|---------------|---------------|---------------|---------------|
;	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | b	| a | b	|
;       |                                                   | s	| c | c	|
;	|                                                   | w	| o | o	|
;       |                                                   | p	| n | n	|
;	 ---------------------------------------------------------------
;
L_BSWP	equ	0004			;0 = <0:7> even addresses byte swap 
L_ACON	equ	0002			;0 = ale, 1 = /as
L_BCON	equ	0001			;0 = /bm1, bm0, /hold
;
BUS_MASTER	equ	0		;initialization word
;



;------------------------------------------------------------------------
; initialization block
;
init_block	struc
i_mode	dw	0		;mode register usually 0
i_padr	db	0,0,0,0,0,0	;ethernet address
i_ladrf	db	0,0,0,0,0,0,0,0	;logical address filter (mulitcast filter)
i_rdra	dw	0		;lo order pointer to receive descriptor ring
i_rlen	dw	0		;number of rec descriptors (power of 2)
i_tdra	dw	0		;lo order pointer to transmit descriptor ring
i_tlen	dw	0		;number of xmit descritport (power of 2)
init_block	ends

INITBLK_SZ	equ	SIZE 	init_block

;
;l_mode
;
; offset
;	|---------------|---------------|---------------|---------------|
;   00	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| p | 0	| 0 | 0	| 0 | 0	| 0 | 0	| 0 | i | d | c | d | l	| d | d	|
;	| r | 				    | n | t | o | t | o	| t | r	|
;	| o | 				    | t | r | l | c | o	| x | x	|
;	| m | 				    | l | y | l | r | p	|   |	|
;	 ---------------------------------------------------------------
;
L_PROM	equ	8000h		;promiscuous mode
L_INTL	equ	0040h		;internal loop back
L_DRTY	equ	0020h		;disable retry
L_COLL	equ	0010h		;force collision
L_DTCR	equ	0008h		;disable transmit crc
L_LOOP	equ	00040		;loop back
L_DTX	equ	0002h		;disable transmitter
L_DRX	equ	0001h		;disable receiver
;
RUN_MODE	equ	0		;run mode
XMIT_MODE	equ	l_drx		;disable receiver
RCV_MODE	equ	l_dtx		;disable xmitter
TEST_MODE	equ	l_intl+l_loop	;diagnostics mode
;



; physical address
;
; offset
;	 ---------------------------------------------------------------
;   02	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       bits 0 - 15                         | 0 |
;	 ---------------------------------------------------------------
;
;	 ---------------------------------------------------------------
;   04	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       bits 16 - 31                            |
;	 ---------------------------------------------------------------
;
;	 ---------------------------------------------------------------
;   06	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       bits 32 - 47			        |
;	 ---------------------------------------------------------------
;
;
; Multicast address
;
; offset
;	 ---------------------------------------------------------------
;   08	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       bits 0 - 15                             |
;	 ---------------------------------------------------------------
;
;	 ---------------------------------------------------------------
;   0a	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       bits 16 - 31                            |
;	 ---------------------------------------------------------------
;
;	 ---------------------------------------------------------------
;   0c	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       bits 32 - 47			        |
;	 ---------------------------------------------------------------
;
;	 ---------------------------------------------------------------
;   0e  |15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       bits 38 - 63			        |
;	 ---------------------------------------------------------------
;
;
; Receive Descriptor Ring Pointer
;
; offset
;	|---------------|---------------|---------------|---------------|
;   10	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|   rlen    | 0 | 0 | 0 | 0 | 0 |     rdra  (bits 16 - 23)      |
;	 ---------------------------------------------------------------
;
;	|---------------|---------------|---------------|---------------|
;   12	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       rdra  (bits 3 - 15)	    | 0 | 0 | 0 |
;	 ---------------------------------------------------------------
; 	note rings are aligned on quad word boundries
;
;	  		  number of entries
;      			  -----------------
RLEN1	equ	00h	; 1
RLEN2	equ	20h  	; 2
RLEN4	equ	40h  	; 4
RLEN8	equ	60h  	; 8
RLEN16	equ	80h 	; 16
RLEN32	equ	0a0h 	; 32
RLEN64	equ	0c0h 	; 64
RLEN128	equ	0e0h	; 128
;
; Transmit Descriptor Ring Pointer
;
; offset
;	|---------------|---------------|---------------|---------------|
;   14	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|   tlen    | 0 | 0 | 0 | 0 | 0 |     tdra  (bits 16 - 23)      |
;	 ---------------------------------------------------------------
;
;	|---------------|---------------|---------------|---------------|
;   16	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                       tdra  (bits 3 - 15)	    | 0 | 0 | 0 |
;	 ---------------------------------------------------------------
; 	note rings are aligned on quad word boundries
;
;			 number of entries
;       		 -----------------
TLEN1	equ	00h	; 1
TLEN2	equ	20h  	; 2
TLEN4	equ	40h  	; 4
TLEN8	equ	60h  	; 8
TLEN16	equ	80h 	; 16
TLEN32	equ	0a0h 	; 32
TLEN64	equ	0c0h 	; 64
TLEN128	equ	0e0h	; 128
;
;------------------------------------------------------------------------
;
; Receive descriptor ring
;
rec_desc_ring	struc
rd_addr	dw	0		;lo order address of the buffer attached
rd_stat	dw	0		;buffer status and hi order pointer
rd_bcnt	dw	0		;buffer byte count
rd_mcnt	dw	0		;message byte count
rec_desc_ring	ends
RDESCLEN	equ	SIZE rec_desc_ring
;
;
; rmd0
; rd_addr
;
;offset	 ---------------------------------------------------------------
;   00	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                     ladr  (bits 0 - 15)                       |
;	 ---------------------------------------------------------------
;
; lo order 16 bits of the buffer pointed to by this descriptor
;
;
; rmd1
; rd_stat
;
;	|---------------|---------------|---------------|---------------|
;   02	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| o | e | f | o	| c | b	| s | e	|     hadr  (bits 16 - 23)      |
;	| w | r	| r | f	| r | u	| t | n	| 				|
;	| n | r	| a | l	| c | f	| p | p	| 				|
;	|   | 	| m | o	|   | f	|   | 	| 				|
;	 ---------------------------------------------------------------
;
; status and hi order bits of buffer pointer to by this descriptor
;
R_OWN	equ	8000h		;0 = full, host set after read data
R_ERR	equ	4000h
;
; receive status
R_FRAM	equ	2000h		;framing error (cleared by host)
R_OFLO	equ	1000h		;silo overflow (cleared by host)
R_CRC	equ	0800h		;crc error (cleared by host)
R_BUFF	equ	0400h		;buffer error (cleared by host)
R_STP	equ	0200h		;Start of Packet (cleared by host)
R_ENP	equ	0100h		;End of Packet (cleared by host)
;
; rmd2
; rd_bcnt
;
;	|---------------|---------------|---------------|---------------|
;   04	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| 1 | 1 | 1 | 1	|               bcnt                            |
;	 ---------------------------------------------------------------
;
; bcnt - Length of buffer pointed to by this desctriptor, two's compliment
;
RD_MASK	equ	0F000h		;or mask
;
; rmd3
; rd_mcnt
;
;	|---------------|---------------|---------------|---------------|
;   06	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| 0 | 0 | 0 | 0	|               mcnt                            |
;	 ---------------------------------------------------------------
;
; mcnt - Length of message in buffer
;
mcnt_mask	equ	0fffh	;

;------------------------------------------------------------------------
;
; Transmit descriptor ring
;
xmt_desc_ring	struc
td_addr	dw	0		;lo order address of the buffer attached
td_stat	dw	0		;buffer status and hi order pointer
td_bcnt	dw	0		;buffer byte count
td_tdr	dw	0		;errors and tdr count
xmt_desc_ring	ends
TDESCLEN	equ	SIZE xmt_desc_ring
;
;
; tmd0
; td_addr
; 
;	|---------------|---------------|---------------|---------------|
;   00	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	|                     ladr  (bits 0 - 15)                       |
;	 ---------------------------------------------------------------
;
; lo order 16 bits of the buffer pointed to by this descriptor
;
;
; tmd1
; td_stat
;
;	|---------------|---------------|---------------|---------------|
;   02	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| o | e | 0 | m	| o | d	| s | e	|     hadr  (bits 16 - 23)      |
;	| w | r	|   | o	| n | e	| t | n	| 				|
;	| n | r	|   | r	| e | f	| p | p	| 				|
;	|   | 	|   | e	|   | 	|   | 	| 				|
;	 ---------------------------------------------------------------
;
; status and hi order bits of buffer pointer to by this descriptor
;
;
; transmit status
;
T_OWN	equ	8000h		;0 = empty host set when full
T_ERR	equ	4000h
;
T_MORE	equ	1000h		;16 retrys (set by lance, cleared by host)
T_ONE	equ	0800h		;one retry (set by lance, cleared by host)
T_DEF	equ	0400h		;deffered xsmit (setby lance, cleared by host)
T_STP	equ	0200h		;First buffer in packet (set/cleared by host)
T_ENP	equ	0100h		;End of Packet (set/cleared by host
;
;
;
; tmd2
; td_bcnt
;
;	|---------------|---------------|---------------|---------------|
;   04	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| 1 | 1 | 1 | 1	|               bcnt                            |
;	 ---------------------------------------------------------------
;
; bcnt - Length of buffer pointed to by this desctriptor, expressed as a
;        twos compliment negative number.
;
TD_BMASK	equ	0F000h		;or mask
;
;
;
; tmd3
; td_tdr
;
;	|---------------|---------------|---------------|---------------|
;   06	|15 |14 |13 |12 |11 |10 | 9 | 8 | 7 | 6	| 5 | 4 | 3 | 2 | 1 | 0	|
;	|---------------------------------------------------------------|
;	| b | u | 0 | l	| l | r |   tdr  (time domain reflectometery)   |
;	| u | f	|   | c	| c | t	|					|
;	| f | l	|   | o	| a | r	|					|
;	| f | o	|   | l	| r | y	|					|
;	 ---------------------------------------------------------------
;	only valid if the err bit in tmd1 is set
;
T_BUFF	equ	8000h	;buffer error (set by lance, uflo also set)
T_UFLO	equ	4000h	;silo underflow (set by lance,cleared by host)
T_LCOL	equ	1000h	;late collision (set by lance,cleared by host)
T_LCAR	equ	0800h	;loss of carrier (set by lance,cleared by host)
T_RTRY	equ	0400h	;tried 16 times (set by lance,cleared by host)

