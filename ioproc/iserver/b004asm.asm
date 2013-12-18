
; --   ---------------------------------------------------------------------------
; --
; --      ISERVER  -  INMOS standard file server
; --
; --      b004asm.asm
; --
; --      8086 assembler routines for b004link.c
; --
; --      Copyright (c) INMOS Ltd., 1988.
; --      All Rights Reserved.
; --
; --   ---------------------------------------------------------------------------


		DOSSEG
		.MODEL  SMALL
		
		
		.DATA

extrn           _C012_idr:word
extrn           _C012_odr:word
extrn           _C012_isr:word
extrn           _C012_osr:word

		.CODE

		ASSUME CS:_TEXT, DS:DGROUP

PUBLIC          _ReadLink

timeout         dw      0
timeout_fl      db      0
last_count      dw      0

_ReadLink       proc    ;ReadLink( int linkid, char *buffer, int count, int timeout)
		push    bp
		mov     bp, sp
		push    ds
		push    si
		push    di
		
		push    ds
		pop     es
		mov     ax, [bp+10]     ;timeout
		mov     bx, 182         ;multiply timeout by 1.82 to get time in tenth
		mul     bx
		mov     bx, 100
		div     bx     
		mov     cs:[timeout], ax
	       
		mov     cx, [bp+8]      ;count
		mov     bx, [bp+6]      ;buffer
		
		mov     cs:[last_count], ax
		
		
		
non_dma_read:   mov     dx, _C012_isr
		mov     di, bx
		mov     bx, _C012_idr
		mov     ax, 40h
		mov     ds, ax
		mov     ax, ds:[6Ch]
		add     ax, cs:[timeout]
		dec     ax
		mov     cs:[timeout_fl], 0
		mov     si, ax
rd1:            in      al, dx
		test    al, 1
		jz      rot_ready
		xchg    dx, bx
		in      al, dx
		stosb
		xchg    bx, dx
		loop    rd1             ; and do next
		jmp     rd2
rot_ready:     
	       ;If a timeout hasn't happened, get back into the polling loop

		cmp     si, ds:[6Ch]
		jne     rd1
		
	;Is the count the same as it was at the last timeout
	
		cmp     cx, cs:[last_count]
		je      r_get_out       ;Yes  -- exit
		
	;Record the new count and reset the timeout
	
		mov     cs:[last_count], cx
		mov     ax, ds:[6Ch]
		add     ax, cs:[timeout]
		dec     ax
		mov     si, ax
		jmp     rd1
		
r_get_out:      mov     cs:[timeout_fl], 2
		pop     di
		pop     si
		pop     ds
		mov     ax, [bp+8]
		sub     ax, cx
		pop     bp
		ret
rd2:            mov     ax, [bp+8]
		pop     di
		pop     si
		pop     ds
		pop     bp
		ret
_ReadLink       endp

PUBLIC          _WriteLink
_WriteLink       proc    ;Writelink( int linkid, char *buffer, int count, int timeout)
		push    bp
		mov     bp, sp
		
		push    es
		push    si
		push    di
		push    ds
		pop     es
		mov     ax, [bp+10]     ;timeout
		mov     bx, 182         ;multiply timeout by 1.82 to get time in tenth
		mul     bx
		mov     bx, 100
		div     bx     
		mov     cs:[timeout], ax
	       
		mov     cx, [bp+8]      ;count
		mov     bx, [bp+6]      ;buffer
		
		mov     cs:[last_count], cx
		
non_dma_write:  mov     dx, _C012_osr
		mov     si, bx
		mov     bx, _C012_odr
		mov     ax, 40h
		mov     es, ax
		mov     ax, es:[6Ch]
		add     ax, cs:[timeout]
		dec     ax
		mov     cs:[timeout_fl], 0
		mov     di, ax
wr1:            in      al, dx
		test    al, 1
		jz      not_ready
		xchg    dx, bx
		lodsb
		out     dx, al
		xchg    bx, dx
		loop    wr1             ; and do next
		jmp     wr2
not_ready:  
		cmp     di, es:[6Ch]
		jne     wr1
		
	;Is the count the same as it was at the last timeout
	
		cmp     cx, cs:[last_count]
		je      wr_get_out      ;Yes  -- exit
		
	;Record the new count and reset the timeout
	
		mov     cs:[last_count], cx
		mov     ax, es:[6Ch]
		add     ax, cs:[timeout]
		dec     ax
		mov     di, ax
		jmp     wr1
		
wr_get_out:   
		mov     cs:[timeout_fl], 2
		mov     ax, [bp+8]
		sub     ax, cx
		jmp     wrterm
wr2:          
		mov     ax, [bp+8]
wrterm:         pop     di
		pop     si
		pop     es
		pop     bp
		ret
_WriteLink       endp

		END


;
;   Eof
;
