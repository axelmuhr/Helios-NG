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
		PUBLIC	_get_sys_info_ptr

IFDEF	SMALL
_get_sys_info_ptr	proc	near
ENDIF
IFDEF	LARGE
_get_sys_info_ptr	proc	far
ENDIF
		push	es
		push	bx


		mov	ah, 0c0h			;get sys config params
		int	15h
		jc	_no_sys_info			;unsupported call
		mov	dx, es
		mov	ax, bx
		jmp	_exit_sys_info
 _no_sys_info:
		mov	ax, 0FFFFh			;show error
		mov	dx, 0FFFFh			;show error
 _exit_sys_info:
		pop	bx
		pop	es
		ret

_get_sys_info_ptr	endp

_TEXT		ENDS
                END       

