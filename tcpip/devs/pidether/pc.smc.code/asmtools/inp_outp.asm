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
		PUBLIC	_G_inp
		PUBLIC	_G_outp
		PUBLIC	_lpbk_tggl
 
GETPORT		MACRO
IFDEF	SMALL
		mov	dx,word ptr [bp+4]	; get I/O port address
ENDIF
IFDEF	LARGE
		mov	dx,word ptr [bp+6]	; get I/O port address
ENDIF
		ENDM

GETVAL		MACRO
IFDEF	SMALL
		mov	al,byte ptr [bp+6]
ENDIF
IFDEF	LARGE
		mov	al,byte ptr [bp+8]
ENDIF
		ENDM


IFDEF	SMALL
_G_inp		proc	near
ENDIF
IFDEF	LARGE
_G_inp		proc	far
ENDIF
		push	bp
		mov	bp,sp

		GETPORT

		in	al,dx
		jmp	$+2
		xor	ah,ah
		pop	bp
		ret
_G_inp		endp
	
IFDEF	SMALL
_G_outp		proc	near
ENDIF
IFDEF	LARGE
_G_outp		proc	far
ENDIF
		push	bp
		mov	bp,sp

		GETPORT
		GETVAL

		out	dx,al
		jmp	$+2
		mov	ah,0
		pop	bp
		ret
_G_outp		endp

IFDEF	SMALL
_lpbk_tggl	proc	near
ENDIF
IFDEF	LARGE
_lpbk_tggl	proc	far
ENDIF

		push	bp
		mov	bp,sp

		GETPORT
		GETVAL

		mov	ah, al		; save real TCR value
		xor	al, al		; write out a zero
		cli
		out	dx,al
		jmp	$+2
		mov	al, ah		; now write out real TCR value
		out	dx, al
		sti

		mov	ah,0
		pop	bp
		ret

_lpbk_tggl	endp

_TEXT	ENDS
	END       

