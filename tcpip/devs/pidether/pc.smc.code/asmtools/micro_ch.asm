;******************************************************************************
;******************************************************************************
;	A George Kalwitz Production, 1990
;******************************************************************************
;******************************************************************************
;
_TEXT	SEGMENT  WORD PUBLIC 'CODE'
_TEXT	ENDS
_DATA	SEGMENT  WORD PUBLIC 'DATA'
_DATA	ENDS
CONST	SEGMENT  WORD PUBLIC 'CONST'
CONST	ENDS
_BSS	SEGMENT  WORD PUBLIC 'BSS'
_BSS	ENDS
DGROUP	GROUP	CONST, _BSS, _DATA
	ASSUME  CS: _TEXT, DS: DGROUP, SS: DGROUP

_TEXT      	SEGMENT
		ASSUME	CS: _TEXT
		PUBLIC	_micro_channel

IFDEF	SMALL
_micro_channel	proc	near
ENDIF
IFDEF	LARGE
_micro_channel	proc	far
ENDIF
		push	bp
		mov	bp, sp
		push	es
		push	bx


		mov	ah, 0c0h    ;return system configuration parameter
		int	15h

		jc	not_ch
		cmp	ah, 0
		jnz	not_ch
;	BIOS returns pointer to system descriptor in ES:BX
		mov	al, es:[bx+5]	    ;feature information byte 1
		test	al, 02h 	    ;bit 1 = 1 --micro channel
		jz	not_ch		    ;not micro channel
		mov	ax, 1		    ;yes
		jmp	mch_out
 not_ch:	mov	ax, 0		    ;not micro channel machine
 mch_out:
		or	ax, ax
		pop	bx
		pop	es
		pop	bp
		ret

_micro_channel	endp

_TEXT		ENDS
                END       

