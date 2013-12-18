;*---------------------------------------------------------------------------*;
;--             HELIOS PC  I/O SERVER  - SYNERGY BOARD INTERFACE            --;
;--             ================================================            --;
;--                                                                         --;
;--             Copyright (C) 1989, Perihelion Software Ltd.                --;
;--               All Rights Reserved.                                      --;
;--                                                                         --;
;--                                                                         --;
;--                                                                         --;
;-- MODULE NAME: SYN.ASM                                                    --;
;--                                                                         --;
;-- AUTHOR : BLV                                                            --;
;-- DATE : 9/1/89                                                           --;
;-----------------------------------------------------------------------------;
;-- SccsId: 3.8 28/3/90      Copyright (C) 1989, Perihelion Software Ltd.	    --;

;==============================================================================
;   Some manifests to keep the world tidy
;==============================================================================

arg1	EQU	 6
arg2	EQU	 8
arg3	EQU	10
arg4	EQU	12

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

;==============================================================================
; Externals are declared outside the code segment, as recommended in the link
; documentation. This avoids fixup overflow errors.
;==============================================================================

CODE    SEGMENT WORD public 'CODE'
        ASSUME cs:CODE

PUBLIC	_syn_inbyte, _syn_inword, _syn_outbyte, _syn_outword
PUBLIC	_syn_setaddr, _syn_writeword, _syn_readword, _syn_writewholeword
PUBLIC	_syn_readbuffer, _syn_writebuffer
PUBLIC	_synergy_base

_synergy_base	DW	0

control_off	EQU	0
status_off	EQU	0
LSB_off		EQU	1
CLI_off		EQU	2
STI_off		EQU	3
Addr_off	EQU	4
MSB_off		EQU	6

;==============================================================================
;   int syn_inbyte(int addr)						    --;
; 									    --;
;==============================================================================
_syn_inbyte	PROC FAR

	push	bp
	mov	bp,sp
	mov	dx,ss:arg1[bp]
	in	al,dx
	xor	ah,ah
	pop	bp

	ret

_syn_inbyte	ENDP

;==============================================================================
;   int syn_inword(int addr)						    --;
; 									    --;
;==============================================================================
_syn_inword	PROC FAR

	push	bp
	mov	bp,sp
	mov	dx,ss:arg1[bp]
	in	ax,dx
	pop	bp
	ret

_syn_inword	ENDP

;==============================================================================
;   void syn_outbyte(int addr, int val)				            --;
; 									    --;
;==============================================================================
_syn_outbyte	PROC FAR

	push	bp
	mov	bp,sp
	mov	dx,ss:arg1[bp]
	mov	ax,ss:arg2[bp]
	out	dx,al
	pop	bp	       	
	ret

_syn_outbyte	ENDP

;==============================================================================
;   void syn_outword(int addr, int val)					    --;
; 									    --;
;==============================================================================
_syn_outword	PROC FAR

	push	bp
	mov	bp,sp
	mov	dx,ss:arg1[bp]
	mov	ax,ss:arg2[bp]
	out	dx,ax
	pop	bp
	ret

_syn_outword	ENDP

;==============================================================================
;   void syn_setadd(int addr)						    --;
; 									    --;
;==============================================================================
_syn_setaddr	PROC FAR

	push	bp
	mov	bp,sp
	mov	dx,_synergy_base
	add	dx,Addr_off
	mov	ax,ss:arg1[bp]
	out	dx,ax
	pop	bp
	ret

_syn_setaddr	ENDP

;==============================================================================
;   void syn_writewholeword(WORD val)					    --;
; 									    --;
;==============================================================================
_syn_writewholeword	PROC FAR

	push	bp
	mov	bp,sp
	mov	dx,cs:_synergy_base
	add	dx,LSB_off
	mov	al,ss:arg1[bp]
	out	dx,al
	mov	al,ss:(arg1+1)[bp]
	mov	ah,ss:arg2[bp]
	add	dx,(MSB_off-LSB_off)
	out	dx,ax
	pop	bp
	ret

_syn_writewholeword	ENDP

;==============================================================================
;   int syn_readword(void)						    --;
; 									    --;
;==============================================================================
_syn_readword	PROC FAR

	mov	dx,_synergy_base
	add	dx,MSB_off
	in	ax,dx
	ret

_syn_readword	ENDP

;==============================================================================
;   void syn_writeword(int)						    --;
; 									    --;
;==============================================================================
_syn_writeword	PROC FAR

	push	bp
	mov	bp,sp
	mov	dx,_synergy_base
	add	dx,MSB_off
	mov	ax,ss:arg1[bp]
	out	dx,ax
	pop	bp

	ret

_syn_writeword	ENDP

;==============================================================================
;   void syn_readbuffer(int addr, int size, int which_bytes, byte *ptr)     --;
; 									    --;
;==============================================================================
_syn_readbuffer	PROC	FAR

	push	bp
	mov	bp,sp
	push	ds
	push	si

	mov	dx,cs:_synergy_base	; set up the address
	add	dx,Addr_off
	mov	ax,ss:arg1[bp]
	out	dx,ax

	add	dx,(MSB_off - Addr_off)	; move to buffer pointer

	mov	cx,ss:arg2[bp]		; extract the other arguments
	mov	ax,ss:arg3[bp]		; cx = size, ax = which_bytes
	lds	si,ss:arg4[bp]		; es:si = buffer

	cmp	ax,2
	jne	byte
					; read cx words, putting them into buffer
word_loop:
	in	ax,dx
	mov	ds:[si],ax
	inc	si
	inc	si
	loop	word_loop
	jmp	readbuf_end

byte:
	in	al,dx
	mov	ds:[si],al
	inc	si
	loop	byte

readbuf_end:
	pop	si
	pop	ds						
	pop	bp
	ret

_syn_readbuffer	ENDP

;==============================================================================
;   void syn_writebuffer(int addr, int size, int which_bytes, byte *ptr)     --;
; 									    --;
;==============================================================================
_syn_writebuffer	PROC	FAR

	push	bp
	mov	bp,sp
	push	ds
	push	si

	mov	dx,cs:_synergy_base	; set up the address
	add	dx,Addr_off
	mov	ax,ss:arg1[bp]
	out	dx,ax

	add	dx,(MSB_off - Addr_off)	; move to buffer pointer

	mov	cx,ss:arg2[bp]		; extract the other arguments
	mov	ax,ss:arg3[bp]		; cx = size, ax = which_bytes
	lds	si,ss:arg4[bp]		; ds:si = buffer

	cmp	ax,2
	jne	Wbyte
					; read cx words, putting them into buffer
Wword_loop:
	mov	ax,ds:[si]
	out	dx,ax
	inc	si
	inc	si
	loop	Wword_loop
	jmp	writebuf_end

Wbyte:
	mov	al,ds:[si]
	out	dx,al
	inc	si
	loop	Wbyte

writebuf_end:
	pop	si
	pop	ds
	pop	bp
	ret

_syn_writebuffer	ENDP
   
CODE    ENDS

        END
 

