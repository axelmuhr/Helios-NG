;*---------------------------------------------------------------------------*;
;--                        HELIOS PC  I/O SYSTEM                            --;
;--                        =====================                            --;
;--                                                                         --;
;--             Copyright (C) 1987, Perihelion Software Ltd.                --;
;--               All Rights Reserved.                                      --;
;--                                                                         --;
;--                                                                         --;
;--                                                                         --;
;-- MODULE NAME: RRLOCK.ASM                                                 --;
;--                                                                         --;
;-- AUTHOR : BLV                                                            --;
;-- DATE : 2/2/90                                                           --;
;-----------------------------------------------------------------------------;

;==============================================================================
;   Some manifests to keep the world tidy
;==============================================================================

arg1	EQU	 6
arg2	EQU	 8
arg3	EQU	10
arg4	EQU	12
arg5	EQU	14
arg6	EQU	16

;==============================================================================
;    And some macros mainly for debugging
;==============================================================================
int_push	MACRO
	push	bp
	push	ds
	push	es
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	ENDM

int_pop		MACRO
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	es
	pop	ds
	pop	bp
	ENDM

pushall	MACRO
	push	bp
	mov	bp,sp
	push	ds
	push	es
	push	si
	push	di
	ENDM

popall	MACRO
	pop	di
	pop	si
	pop	es
	pop	ds
	pop	bp
	ENDM

CODE    SEGMENT WORD public 'CODE'
        ASSUME cs:CODE

PUBLIC	_unlock_gemini
_unlock_gemini	PROC	FAR
	mov	dx, 0120h
	mov	al,1
	out	dx,al
	ret 
_unlock_gemini	ENDP

CODE	ENDS
	END

