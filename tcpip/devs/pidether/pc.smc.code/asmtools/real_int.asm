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

IFDEF	SMALL
		EXTRN	_lan_service_rtn:near
		EXTRN	_set_vector:near
ENDIF
IFDEF	LARGE
		EXTRN	_lan_service_rtn:far
		EXTRN	_set_vector:far
ENDIF

REAL_STACK_SIZE	equ	0400h

_DATA      SEGMENT
_real_stack	DB	REAL_STACK_SIZE dup(?)
_real_ss_save	DW	?
_real_sp_save	DW	?
_DATA      ENDS

_TEXT      	SEGMENT
		ASSUME	CS: _TEXT
		PUBLIC	_install_rlan
 
GETINT		MACRO
IFDEF	SMALL
		mov	ax,[bp+4]		; get vector num from stack
ENDIF
IFDEF	LARGE
		mov	ax,[bp+6]		; get vector num from stack
ENDIF
		ENDM

; this is the real LAN interrupt vector for processing packets
IFDEF	SMALL
_rlan_vector	proc	near
ENDIF
IFDEF	LARGE 
_rlan_vector	proc	far
ENDIF
		push	ax
		push	bp
		push	si
		push	di
		push	ds
		push	es
		push	bx
		push	cx
		push	dx

		mov	ax, DGROUP
		mov	ds, ax
		mov	ax, ss
		mov	ds:_real_ss_save, ax
		mov	ax, DGROUP
		mov	ss, ax
		mov	ax, sp
		mov	ds:_real_sp_save, ax
		mov	ax, OFFSET DGROUP:_real_stack
		add	ax, REAL_STACK_SIZE
		mov	sp, ax

		call	_lan_service_rtn
	
		mov	ax, ds:_real_sp_save
		mov	sp, ax
		mov	ax, ds:_real_ss_save
		mov	ss, ax

		pop	dx
		pop	cx
		pop	bx
		pop	es
		pop	ds
		pop	di
		pop	si
		pop	bp
		pop	ax
		iret
_rlan_vector	endp

IFDEF	SMALL
_install_rlan	proc	near
ENDIF
IFDEF	LARGE
_install_rlan	proc	far
ENDIF
		push	bp
		mov	bp,sp
		push	cs			; put msw on stack
		mov	ax, OFFSET _rlan_vector	; put lsw on stack
		push	ax

		GETINT

		push	ax
		call	_set_vector
		pop	ax			; clean up stack
		pop	ax
		pop	ax
		pop	bp
		ret
_install_rlan	endp

_TEXT		ENDS
                END       
