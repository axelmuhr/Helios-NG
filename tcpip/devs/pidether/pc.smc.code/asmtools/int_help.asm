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
		PUBLIC	_set_vector
		PUBLIC	_get_vector
 
; macro to get the passed interrupt number off the stack
GETINT		MACRO
IFDEF	SMALL
		mov	ax,[bp+4]		; get int number
ENDIF
IFDEF	LARGE
		mov	ax,[bp+6]		; get int number
ENDIF
		ENDM
;
;macro to get the vector for the interrupt
GETVEC		MACRO
IFDEF	SMALL
		mov	ax,[bp+6]		; lsw of passed vec into DX
		mov	dx,ax
		mov	ax,[bp+8]		; msw of passed vec into DS
		mov	ds,ax
ENDIF
IFDEF	LARGE
		mov	ax,[bp+8]		; lsw of passed vec into DX
		mov	dx,ax
		mov	ax,[bp+10]		; msw of passed vec into DS
		mov	ds,ax
ENDIF
		ENDM

IFDEF	SMALL
_set_vector	proc	near
ENDIF
IFDEF	LARGE
_set_vector	proc	far
ENDIF
		push	bp
		mov	bp,sp
		push	ax
		push	bx
		push	dx
		push	ds
		push	es
		push	di

		GETVEC

		GETINT	         		; passed vec num into AL

IFDEF	EXETYPE
		mov	ah,25h			; id of INT 21's set vector
		int	21h
ENDIF
IFDEF	ROMTYPE
		mov	bx, 0
		mov	es, bx			; segment of vector table
		mov	ah, 0			; make sure no junk
		shl	ax, 1			; int * 4 = offset into vec tbl
		shl	ax, 1
		mov	di, ax
		mov	WORD PTR es:[di], dx	; load lsw of vector
		add	di, 2			; point to next word location
		mov	ax, ds			; load msw of vector
		mov	WORD PTR es:[di], ax
ENDIF

		pop	di
		pop	es
		pop	ds
		pop	dx
		pop	bx
		pop	ax
		pop	bp
		ret
_set_vector 	endp

IFDEF	SMALL
_get_vector	proc	near
ENDIF
IFDEF	LARGE
_get_vector	proc	far
ENDIF
		push	bp
		mov	bp,sp
		push	bx
		push	es
		push	si

		GETINT

IFDEF	EXETYPE
		mov	ah,35h			;           and AL = int num
		int	21h
		mov	ax,es			; INT 21H returned
		mov	dx,ax			;     ES:BX but MSC needs
		mov	ax,bx			;     DX:AX
ENDIF
IFDEF	ROMTYPE
		mov	bx, 0
		mov	es, bx			; segment of vector table
		mov	ah, 0			; make sure no junk
		shl	ax, 1			; int * 4 = offset into vec tbl
		shl	ax, 1
		mov	si, ax
		mov	ax, WORD PTR es:[si]	; get lsw of vector
		add	si, 2			; point to next table word
		mov	dx, WORD PTR es:[si]	; get msw of vector
ENDIF

		pop	si
		pop	es
		pop	bx
		pop	bp
		ret
_get_vector 	endp

_TEXT		ENDS
                END       
