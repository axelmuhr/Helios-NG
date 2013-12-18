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
		PUBLIC	_int_on
		PUBLIC	_int_off
 
IFDEF		SMALL
_int_on		proc	near
ENDIF
IFDEF		LARGE
_int_on		proc	far
ENDIF
		push	bp
		mov	bp,sp
		sti
		pop	bp
		ret
_int_on		endp

IFDEF		SMALL
_int_off	proc	near
ENDIF
IFDEF		LARGE
_int_off	proc	far
ENDIF
		push	bp
		mov	bp,sp
		cli
		pop	bp
		ret
_int_off	endp

_TEXT		ENDS
                END       
