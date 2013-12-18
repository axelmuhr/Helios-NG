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
		PUBLIC	_MV8013
 
GETPARAMS	MACRO
IFDEF	SMALL
		mov	si,[bp+4]		;setup source addr
		mov	ax, [bp+6]
		mov	ds, ax
		mov	di, [bp+8]		;setup destination addr
		mov	ax, [bp+10]
		mov	es, ax
		mov	cx, [bp+12]		;setup count
ENDIF
IFDEF	LARGE
		mov	si,[bp+6]		;setup source addr
		mov	ax, [bp+8]
		mov	ds, ax
		mov	di, [bp+10]		;setup destination addr
		mov	ax, [bp+12]
		mov	es, ax
		mov	cx, [bp+14]		;setup count
ENDIF
		ENDM

;
; name          mv8013
;
; synopsis      MV8013 (sptr, dptr, size)
;               int *sptr;   /* source ptr */
;               int *dptr;   /* destination ptr */
;               int size;    /* number of bytes */
;
IFDEF	SMALL
_mv8013		PROC	NEAR
ENDIF
IFDEF	LARGE
_mv8013		PROC	FAR
ENDIF
		push    bp			;save anything changed
                mov     bp,sp			;point to passed data
		push    ds
		push	es
		push	si
		push	di
		push	ax
		push	bx
		push	cx
		push	dx

		GETPARAMS
;
; all passed parameters are now secure
;
		inc	cx			; round up to even number
		and 	cx, 0fffeh		;   of bytes
		shr	cx, 1			; divide for num of words
		cld				; make sure we increment
;
; move block of data in words
;
		rep	movsw			; move the block
;
		pop	dx			; restore everything
		pop	cx
		pop	bx
		pop	ax
		pop	di
		pop	si
		pop	es
                pop     ds
                pop     bp
                ret
_mv8013		ENDP

_TEXT		ENDS
                END

