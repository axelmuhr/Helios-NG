;*---------------------------------------------------------------------------*;
;--                        HELIOS PC  I/O SYSTEM                            --;
;--                        =====================                            --;
;--                                                                         --;
;--             Copyright (C) 1993, Perihelion Software Ltd.                --;
;--               All Rights Reserved.                                      --;
;--                                                                         --;
;--                                                                         --;
;--                                                                         --;
;-- MODULE NAME: qpcas.cpp                                                  --;
;--                                                                         --;
;-- AUTHOR : BLV                                                            --;
;-----------------------------------------------------------------------------;
;-- RcsId: $Id: qpcas.cpp,v 1.1 1993/04/30 11:13:27 bart Exp $	--;
;-- Copyright (C) 1988, Perihelion Software Ltd.   			    --;
;

arg1	EQU	 6
arg2	EQU	 8

.286
; External variables, in qpc.c
DGROUP  GROUP _DATA
        ASSUME ds:DGROUP
_DATA  SEGMENT WORD PUBLIC 'DATA'

EXTRN	_QpcLinkData	: WORD
EXTRN	_QpcTxEmptyBit	: WORD
EXTRN	_QpcRxFullBit	: WORD
EXTRN	_QpcStatus	: WORD

_DATA	ENDS

CODE    SEGMENT WORD public 'CODE'
        ASSUME cs:CODE

	PUBLIC  _qpc_fetch_block
_qpc_fetch_block    PROC   FAR
	; Register usage:
	; es:di		buffer
	; dx,si,bp		link I/O address, status register
	; bx		mask for tests
	; cx + stack	iterations + count
	; ax temporary
	push	bp
	mov	bp,sp
	push	si
	push	di
	push	es
	push	ds

	; Get all the variables into registers. This assumes that instructions
	; are held in a cache, and the only external memory access will be
	; for the actual data.
	mov	si,_QpcLinkData
	mov	dx,_QpcStatus
	mov	bx,_QpcRxFullBit
	mov	cx,arg1[bp]	; count in bytes
	shr	cx,2		; -> count in words
	les	di,arg2[bp]	; buffer
	mov	bp,dx		; bp no longer needed, zap with QpcStatus addr
l1:
	or	cx,cx		; while (count > 0)
	je	l2
	push	cx		; save count on stack
	mov	cx,32767	; for (i = 32767; i ; i--)
l4:
	mov	dx,bp
	in	ax,dx		; ax = _inpw(QpcStatus)
	and	ax,bx		; if (ax & RxFull)
	je	l5

	mov	dx,si
	in	ax,dx		; ax = _inpw(QpcLinkData)
	stosw			; *buffer++ = ax			
	add	dx,2		; ax = _inpw(QpclinkData + 2)
	in	ax,dx
	stosw			; *buffer++ = ax

	pop	cx		; restore count
	dec	cx		; count--
	jmp	l1		; to start of while loop
l5:
	loop	l4		; end of for loop
	pop	ax		; return (count << 2), in bytes
	shl	ax,2
	jmp	l3
l2:
	xor	ax,ax		; return(0)
l3:
	pop	ds
	pop	es
	pop	di
	pop	si
	pop	bp
	ret
_qpc_fetch_block      ENDP

	PUBLIC  _qpc_send_block
_qpc_send_block     PROC  FAR
	; Register usage:
	; ds:si		buffer
	; dx,di,bp	link I/O address, status register
	; bx		mask for tests
	; cx + stack	iterations + count
	; ax temporary
	push	bp
	mov	bp,sp
	push	si
	push	di
	push	ds
	push	es

	; Get all the variables into registers. This assumes that instructions
	; are held in a cache, and the only external memory access will be
	; for the actual data.
	mov	di,_QpcLinkData
	mov	dx,_QpcStatus
	mov	bx,_QpcTxEmptyBit
	mov	cx,arg1[bp]	; count in bytes
	shr	cx,2		; -> count in words
	lds	si,arg2[bp]	; buffer, zaps ds != DGROUP
	mov	bp,dx		; QpcStatus into bp, bp no longer needed
l11:
	or	cx,cx		; while (count > 0)
	je	l12
	push	cx		; save count on stack
	mov	cx,32767	; for (i = 32767; i ; i--)
l14:
	mov	dx,bp
	in	ax,dx		; ax = _inpw(QpcStatus)
	and	ax,bx		; if (ax & TxEmpty)
	je	l15		; on 386sx this branch is rarely taken

	mov	dx,di
	lodsw			; _outpw(QpcLinkBase, *buffer++)
	out	dx,ax
	add	dx,2		; _outpw(QpcLinkBase+2, *buffer++)
	lodsw
	out	dx,ax

	pop	cx		; restore count
	dec	cx		; count--
	jmp	l11		; to start of while loop
l15:
	loop	l14		; end of for loop
	pop	ax		; return (count << 2), bytes failed to send
	shl	ax,2
	jmp	l13
l12:
	xor	ax,ax		; return(0)
l13:
	pop	es
	pop	ds
	pop	di
	pop	si
	pop	bp
	ret
_qpc_send_block      ENDP

CODE    ENDS

        END


