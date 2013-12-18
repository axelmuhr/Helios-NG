;*---------------------------------------------------------------------------*;
;--                        HELIOS PC  I/O SYSTEM                            --;
;--                        =====================                            --;
;--                                                                         --;
;--             Copyright (C) 1987, Perihelion Software Ltd.                --;
;--               All Rights Reserved.                                      --;
;--                                                                         --;
;--                                                                         --;
;--                                                                         --;
;-- MODULE NAME: GRAPH.ASM                                                  --;
;--                                                                         --;
;-- AUTHOR : BLV                                                            --;
;-- DATE : 7/12/89                                                          --;
;-----------------------------------------------------------------------------;

;==============================================================================
;    some macros 
;==============================================================================

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

EXTRN	_INTERRUPT_RTN:NEAR

_DATA	SEGMENT	WORD public 'DATA'
junk		DB	5000 DUP(0)
new_stack	DB	16 DUP(0)
_DATA	ENDS

_TEXT    SEGMENT WORD public 'CODE'
        ASSUME cs:_TEXT

default_data	DW	0
stack_pointer	DW	0
data_segment	DW	0
stack_segment	DW	0

PUBLIC _SET_INTERRUPT

_SET_INTERRUPT	PROC	NEAR
	pushall

	mov	cs:default_data,ds

	mov	ah,025h		; function 25 - set interrupt vector
	mov	al,060h		; interrupt vector
	mov	dx,cs
	mov	ds,dx
	mov	dx,offset trap_60
	int	21h

	popall
	ret

_SET_INTERRUPT	ENDP

trap_60	PROC	FAR
	mov	cs:stack_pointer,sp
	mov	cs:data_segment,ds
	mov	cs:stack_segment,ss
	
	mov	ax,cs:default_data
	mov	es,ax
	mov	ds,ax
	mov	ss,ax
	mov	sp,offset new_stack

	sti	; reenable interrupts

	mov	ax,cs:data_segment
	push	ax
	push	dx
	call	_INTERRUPT_RTN
	pop	cx
	pop	cx

	cli	; disable interrupts

	mov	ss,cs:stack_segment
	mov	ds,cs:data_segment
	mov	sp,cs:stack_pointer
	iret

trap_60	ENDP

	_TEXT	ENDS

        END
 
