;*---------------------------------------------------------------------------*;
;--                        HELIOS PC  MINISERVER                            --;
;--                        =====================                            --;
;--                                                                         --;
;--             Copyright (C) 1988, Perihelion Software Ltd.                --;
;--               All Rights Reserved.                                      --;
;--                                                                         --;
;--                                                                         --;
;--                                                                         --;
;-- MODULE NAME: MINIASM.ASM                                                --;
;--                                                                         --;
;-- AUTHOR : BLV                                                            --;
;-- DATE : 24/8/88                                                          --;
;-----------------------------------------------------------------------------;
;-- SccsId: 2.2 2/10/88      Copyright (C) 1988, Perihelion Software Ltd.	    --;

;==============================================================================
;   Some manifests to keep the world tidy
;==============================================================================

; small model
arg1	EQU	 4
arg2	EQU	 6
arg3	EQU	 8
arg4	EQU	10
arg5	EQU	12

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

DGROUP	GROUP _DATA
	ASSUME ds:DGROUP
_DATA	SEGMENT WORD public 'DATA'

PUBLIC _vbios_attr, _vbios_x, _vbios_y, _link_base, _int_level
PUBLIC _save_x,_save_y, _silent_mode

_vbios_x		DB	0
_vbios_y		DB	0
_vbios_attr	        DB	7
_save_x			DB	0
_save_y			DB	0
save_page		DB	0
#ifdef GEMINI
_link_base		DW	0100H
#else
_link_base		DW	0150H
#endif
_int_level		DW	3
_silent_mode		DW	0

_DATA	ENDS

EXTRN	_wakeup:NEAR, _handle_data:NEAR
 
_TEXT    SEGMENT WORD public 'code'
        ASSUME cs:_TEXT

screen_chars:
		DB	2000 DUP (?)
screen_attr:
 		DB	2000 DUP (?)

bg_pending	DW	0
bg_active	DW	0
bg_timer	DW	0
bg_showing	DW	0
bg_30sec	EQU	546
bg_2sec		EQU	36
bg_silent	DW	0

;==============================================================================
;   The simple interrupt-handling stuff
;==============================================================================
; External functions called
PUBLIC _set_interrupts, _tsr

Wakeup_int	EQU	061H
GetInt		EQU	035H
SetInt		EQU	025H
GetDTA		EQU	02FH
SetDTA		EQU	01AH
GetPSP		EQU	051H
SetPSP		EQU	050H
GetInDOS	EQU	034H
Int21		EQU	021H
TimerInt	EQU	008H
IdleInt		EQU	028H
KeyboardInt	EQU	016H
video_trap	EQU	10H
MAGIC		EQU	4321H

set_video	EQU	 0 	; manifests for the video bios
set_cursortype	EQU	 1
move_cursor	EQU      2
read_cursor	EQU	 3
read_pen	EQU	 4
select_page	EQU	 5
scroll_up	EQU	 6
scroll_down	EQU	 7
read_char	EQU	 8
write_charattr	EQU	 9
write_char	EQU	10
set_palette	EQU	11
write_pixel	EQU	12
read_pixel	EQU	13
write_text	EQU	14
get_mode	EQU	15
set_palreg	EQU	16
write_string	EQU	19
default_attr	EQU	007H	; normal plain video
inverse_attr	EQU	070H
default_page	EQU	0


Default_DS	DW	?
Default_SS	DW	?
Default_SP	DW	?
PSP_Seg		DW	?	; Segment only is required
DTA_Seg		DW	?
DTA_Off		DW	?
InDOS_Seg	DW	?
InDOS_Off	DW	?
link_pending	DW	0
asleep		DW	0
oldTimerInt	DD	?
oldIdleInt	DD	?
oldKeyboardInt	DD	?
oldInt21	DD	?
save_SS		DW	?
save_SP		DW	?
save_PSP_Seg	DW	?
save_DTA_Seg	DW	?
save_DTA_Off	DW	?

enable_ints	MACRO
	sti
	ENDM

disable_ints	MACRO
	cli
	ENDM

;==============================================================================
; switch_C
; switch_back
;
; These routines do most of the work of switching environments. switch_C()
; stores the current environment, and switches most of it to the miniserver's
; environment. switch_back() restores the environment to the parent's. The
; following assumptions are made :
;    1) interrupts are currently disabled, so it is safe to hack everything
;    2) either a) the PC is not in an MS-DOS call, or
;              b) the PC is the MS-DOS idle interrupt
;    3) all the registers may be zapped, they have been saved with an int_push
;
; The routines do not switch the stack, because if they did that it would
; not be possible to return back to the calling routine...
;==============================================================================

switch_C	PROC	NEAR
	enable_ints			; so that I can call the system
	cld				; set string operations to auto-increment

	mov	ah,GetPSP		; save current PSP
	int	Int21
	mov	cs:save_PSP_Seg,bx

	mov	ah,GetDTA		; and current DTA
	int	Int21
	mov	cs:save_DTA_Off,bx
	mov	bx,es
	mov	cs:save_DTA_Seg,bx

	mov	ah,SetPSP		; install my PSP
	mov	bx,cs:PSP_Seg
	int	Int21

	mov	ah,SetDTA		; and my DTA
	mov	dx,cs:DTA_Seg
	mov	ds,dx
	mov	dx,cs:DTA_Off
	int	Int21

	disable_ints			; to hack the segments and return
	mov	dx,cs:Default_DS	; set up segment registers
	mov	ds,dx
	mov	es,dx

	ret
switch_C	ENDP

switch_back	PROC	NEAR
	enable_ints		; to call the system
	mov	bx,cs:save_PSP_Seg
	mov	ah,SetPSP
	int	Int21

	mov	dx,cs:save_DTA_Seg
	mov	ds,dx
	mov	dx,cs:save_DTA_Off
	mov	ah,SetDTA
	int	Int21

	disable_ints		; to return in the same state
	ret
switch_back	ENDP

;=============================================================================;
; switch_C_stack
; switch_back_stack
; 
; These macros are used to switch the stacks in-line
;=============================================================================;
switch_C_stack	MACRO
	mov	dx,sp
	mov	cs:save_SP,dx
	mov	dx,ss
	mov	cs:save_SS,dx
	mov	cx,cs:Default_SP
	mov	dx,cs:Default_SS
	mov	ss,dx
	mov	sp,cx
	ENDM

switch_back_stack	MACRO
	mov	cx,cs:save_SP
	mov	dx,cs:save_SS
	mov	ss,dx
	mov	sp,cx
	ENDM
	
;=============================================================================;
; void tsr(void)
;
; Terminate, keeping all of the code and data segments resident
; This assumes small memory model
;=============================================================================;
_tsr	PROC	NEAR
	mov	dx,ss	; work out number of paragraphs between top of stack
	mov	ax,cs	; and start of code segment
	sub	dx,ax
	mov	ax,sp	; allowing for the stack pointer
	mov	cl,4	; change offset to paragraphs
	shr	ax,cl
	add	dx,ax
	add	dx,0040H ; and 1 K extra for luck

	mov	ah,031H
	mov	al,0
	int	21H
	ret
_tsr	ENDP

;=============================================================================;
; void set_interrupts(void)
;=============================================================================;

_set_interrupts PROC NEAR
	pushall

	mov	ax,ds:_silent_mode
	mov	cs:bg_silent,ax

	mov	ah,GetInt	; see if already installed
	mov	al,Wakeup_int
	int	Int21
	xor	ax,ax
	mov	di,bx
	mov	bx,es:[di-2]
	cmp	bx,MAGIC
	jne	set_continue

	popall
	ret

set_continue:
			; store the segment registers for a wakeup
	mov	dx,ds
	mov	cs:Default_DS,dx
	mov	dx,ss
	mov	cs:Default_SS,dx
	mov	dx,sp
	mov	cs:Default_SP,dx

			; install the wakeup routine at int 61H
	mov	ah,SetInt
	mov	al,Wakeup_int
	mov	dx,cs
	mov	ds,dx
	mov	dx,offset Wakeup_rtn
	int	Int21

	mov	ah,GetDTA
	int	Int21
	mov	cs:DTA_Off,bx
	mov	bx,es
	mov	cs:DTA_Seg,bx

	mov	ah,GetPSP
	int	Int21
	mov	cs:PSP_Seg,bx

	mov	ah,GetInDOS
	int	Int21
	mov	cs:InDOS_Off,bx
	mov	bx,es
	mov	cs:InDos_Seg,bx

	mov	ah,GetInt	; store the current timer, idle keyboard
	mov	al,IdleInt	; and Int21 interrupts
	int	Int21
	mov	word ptr cs:oldIdleInt,bx
	mov	word ptr cs:oldIdleInt+2,es

	mov	ah,GetInt
	mov	al,TimerInt
	int	Int21
	mov	word ptr cs:oldTimerInt,bx
	mov	word ptr cs:oldTimerInt+2,es

	mov	ah,GetInt
	mov	al,KeyboardInt
	int	Int21
	mov	word ptr cs:oldKeyboardInt,bx
	mov	word ptr cs:oldKeyboardInt+2,es

	mov	ah,GetInt
	mov	al,Int21
	int	Int21
	mov	word ptr cs:oldInt21,bx
	mov	word ptr cs:oldInt21+2,es


	mov	ah,SetInt
	mov	al,IdleInt
	mov	dx,cs
	mov	ds,dx
	mov	dx,offset idle_rtn
	int	Int21

	mov	ah,SetInt
	mov	al,TimerInt
	mov	dx,cs
	mov	ds,dx
	mov	dx,offset timer_rtn
	int	Int21

	mov	ah,SetInt
	mov	al,KeyboardInt
	mov	dx,cs
	mov	ds,dx
	mov	dx,offset keyboard_int
	int	Int21

	mov	ah,SetInt
	mov	al,Int21
	mov	dx,cs
	mov	ds,dx
	mov	dx,offset MyInt21
;	int	Int21


	mov	ax,1
	popall
	ret
_set_interrupts ENDP

;==============================================================================
; Wakeup routine
;
; Installed at int 61, invoked by the program wakeup.com. The junk field is
; examined by set_interrupts above and by the hstart program, using offset
; -2 from the interrupt vector at 061H, to check whether or not the
; miniserver has been installed.
;==============================================================================

Ints	DW	0
Clashes	DW	0
timer	DW	0
timerc	DW	0
idle	DW	0

junk	DW	MAGIC

Wakeup_rtn	PROC	NEAR
	int_push

	mov	ah,0FH		; get current video mode
	int	video_trap	; and check for 80 characters per line
	cmp	ah,80
	je	wakeup_ok
	int_pop
	iret

wakeup_ok:
	call	disable_link_int
	mov	cs:link_pending,0

	mov	cs:asleep,0	; affects keyboard hot-key handling

	mov	cs:bg_pending,0
	mov	cs:bg_active,0
	cmp	cs:bg_showing,0
	je	w_donebg
	call	bg_undraw

w_donebg:
	switch_C_stack
	call	switch_C

	enable_ints		; when inside C
	call	_wakeup
	disable_ints
	mov	cs:asleep,ax

	call	switch_back
	switch_back_stack

	mov	ax,cs:asleep	; iff going to sleep, enable interrupts
	cmp	ax,0
	je	skip_enable
	call	enable_link_int
	mov	cs:bg_pending,0	; restart background message timing
	mov	cs:bg_timer,bg_30sec
	mov	cs:bg_active,1
	cmp	cs:bg_silent,0	; bg messages active iff !silent
	je	skip_enable	
	mov	cs:bg_active,0

skip_enable:

	int_pop
	iret
Wakeup_rtn	ENDP

;==============================================================================
; Timer interrupt routine
;
; This is my own timer interrupt routine. First, it calls the system timer
; routine, which will take care of the interrupt controller etc.
; Next it checks whether or not background messages are currently active.
; If so, the background message timer is decremented and when it reaches
; zero bg is set to pending. Then the timer interrupt routine checks for
; either pending link I/O or pending background messages. If either is
; true the routine checks whether or not the InDos flag is set, i.e. whether
; or not I/O is safe, and if safe the appropriate handling routines are
; called.
;==============================================================================

timer_rtn	PROC	NEAR
	pushf		; simulate an interrupt
	call	dword ptr cs:oldTimerInt	; the old timer routine
	disable_ints		; disable the interrupt again, after the iret

	cmp	cs:bg_active,0		; check the background message state
	je	tskip_bg		; if not active, ignore
	dec	cs:bg_timer		; active, decrement counter
	jnz	tskip_bg		; if zero, set pending and deactivate
	mov	cs:bg_pending,1
	mov	cs:bg_active,0
	inc	cs:Ints
	inc	cs:Clashes
	
tskip_bg:
	push	ax
	mov	ax,cs:link_pending	; something pending on the link ?
	or	ax,cs:bg_pending
	cmp	ax,0	
	jne	timer_something
	pop	ax
	iret

timer_something:
	int_push	; save all the registers, stop worrying

	mov	bx,cs:InDOS_Seg
	mov	ds,bx
	mov	bx,cs:InDos_Off
	mov	al,ds:[bx]	; get the InDOS flag
	cmp	al,0
	je	io_safe		; if InDOS, cannot handle the link

	inc	cs:timerc
	jmp	timer_skip

io_safe:
	cmp	cs:link_pending,0
	je	t_nolinkio
	inc	cs:timer

				; Link I/O Pending, not in DOS, so let's do it
	mov	cs:link_pending,0	; clear link_pending flag, before
					; enabling interrupts

	switch_C_stack
	call	switch_C
	enable_ints
	call	_handle_data
	disable_ints
	call	switch_back
	switch_back_stack
	call	enable_link_int
	
t_nolinkio:
	cmp	cs:bg_pending,0
	je	timer_skip
	inc	cs:timer
	call	handle_bg

timer_skip:
	int_pop
	pop	ax
	iret
timer_rtn	ENDP

;==============================================================================
; Idle interrupt routine
;
; This routine is similar to the timer interrupt above. If MS-DOS has been
; called with a function that polls, e.g. keyboard reads, then MS-DOS will
; do int 028H at regular intervals. During this time the system is guaranteed
; stable, and I can handle link I/O if there is any pending. The InDos flag
; can be ignored because MS-DOS is already polling. Also, if there is a
; background message pending this can be handled safely.
;==============================================================================

idle_rtn	PROC	NEAR
	pushf		; simulate an interrupt
	call	dword ptr cs:oldIdleInt	; chain interrupts
	disable_ints	; disable the interrupt again, after the iret

	push	ax
	mov	ax,cs:link_pending	; something pending on the link ?
	or	ax,cs:bg_pending
	cmp	ax,0
	je	idle_done

	int_push	; save all the registers, stop worrying

	cmp	cs:link_pending,0
	je	idle_nolink
					; Link I/O Pending
	mov	cs:link_pending,0	; clear link_pending flag, before
					; enabling interrupts

	inc	cs:idle
	switch_C_stack
	call	switch_C
	enable_ints
	call	_handle_data
	disable_ints
	call	switch_back
	switch_back_stack
	call	enable_link_int

idle_nolink:
	cmp	cs:bg_pending,0
	je	idle_nobg
	inc	cs:idle
	call	handle_bg

idle_nobg:
	int_pop
idle_done:
	pop	ax
	iret
idle_rtn	ENDP

;==============================================================================
; keyboard_int
;
; Replacement for the BIOS keyboard routine, int 016H
; Detects the special case of Alt-F5 in background mode, and uses it to
; switch to foreground
;==============================================================================

keyboard_int	PROC	NEAR
	cmp	ah,0			; only take special action for request
	je	keyboard_special	; 00, read next character
	jmp	dword ptr cs:oldKeyboardInt	; iret will give required
						; result
keyboard_special:
	disable_ints		; ensure interrupts disabled
	pushf
	xor	ah,ah
	call	dword ptr cs:oldKeyboardInt	; get actual result
	disable_ints		; disable interrupts again after iret

	cmp	ax,06C00h
	je	keyboard_gotcha1
	iret

keyboard_gotcha1:
	cmp	cs:asleep,0	; if not asleep, ignore the special case
	jne	keyboard_gotcha2
	iret
			; it should be safe to switch straight to Helios
			; inside the bios call
keyboard_gotcha2:
	int	Wakeup_int	; call Wakeup_int in the standard way

keyboard_pause:
	mov	ah,1		; Now, poll until there is another character
	pushf			; to avoid locking up the link interrupt
	disable_ints		; routines
	call	dword ptr cs:oldKeyboardInt	; simulate keyboard int
	jnz	keyboard_special	; OK, there is data
					; read it, and return it
	push	cx
	mov	cx,0100H	; a short delay
keyboard_pause2:
	loop	keyboard_pause2
	pop	cx
	int	IdleInt
	jmp	keyboard_pause
keyboard_int	ENDP

;==============================================================================
; MyInt21
;
; Intercept all int 0x21 calls, i.e. all calls to the system. At the start
; and end of the call check for link activity
;==============================================================================

MyInt21	PROC	NEAR
	pushf			; simulate an Int21
	call	dword ptr cs:OldInt21

	push	ax		; After the int21 the flags must be set
	pushf			; in spite of the future iret that pops
	pop	ax		; the flag
	or	ax,0200h	; set the Int bit, to allow interrupts
	mov	ss:6[bp],ax	; overwrite the old flags on the stack
	pop	ax

;	int_push
;	call	_vbios_bell
;	int_pop

	iret

MyInt21	ENDP

;==============================================================================
; background messages
;
; When running in the background, every 30 seconds a little message should
; flash up on the screen to show that Helios is running, for about 2
; seconds. The timer interrupt routine takes care of this. When the time
; is reached, a flag is set to indicate that the background handling should
; happen. Next time it is safe to handle it, either from the timer interrupt
; routine or from the idle routine, handle_bg will be called. First it
; has to ensure that the PC is currently in an 80-column text mode. If not,
; the timer is reset to 30 seconds. Otherwise a draw or undraw routine will
; be called
;==============================================================================

handle_bg	PROC	NEAR

	mov	ah,get_mode		; get current video mode
	int	video_trap	; and check for 80 characters per line
;	cmp	ah,80
;	je	handlebg_ok
	cmp	al,02H		; or check for 25*80 b/w or colour
	je	handlebg_ok	; or Monochrome adapter
	cmp	al,03H
	je	handlebg_ok
	cmp	al,07H
	je	handlebg_ok

	mov	cs:bg_timer,bg_30sec
	mov	cs:bg_pending,0
	mov	cs:bg_active,1
	ret

handlebg_ok:
	cmp	cs:bg_showing,0
	je	bg_notshowing
	call	bg_undraw
	mov	cs:bg_timer,bg_30sec
	mov	cs:bg_pending,0
	mov	cs:bg_active,1
	ret

bg_notshowing:
	call	bg_draw
	mov	cs:bg_timer,bg_2sec
	mov	cs:bg_pending,0
	mov	cs:bg_active,1
	ret
handle_bg	ENDP

bg_message:
	DB	"BACKGROUND"
bg_length	EQU	10
bg_save:
	DW	bg_length DUP (?)

bg_draw	PROC	NEAR
	mov	ah,read_cursor	; get cursor position
	mov	bh,default_page
	int	video_trap
	push	dx	; store current coordinates on stack

	mov	dh,0	; row 0
	mov	dl,80-bg_length
	mov	cx,bg_length
	mov	si,offset bg_message
	mov	di,offset bg_save
draw_loop:
	push	dx
	push	cx

	mov	ah,move_cursor
	int	video_trap

	mov	ah,read_char	; read character+attrib
	mov	bh,0
	int	video_trap

	mov	cs:[di],ah	; store attribute
	inc	di
	mov	cs:[di],al	; and character
	inc	di

	mov	al,cs:[si]	; get character from string
	inc	si
	mov	bh,default_page
	mov	cx,1		; 1 character
	mov	bl,inverse_attr
	mov	ah,write_charattr
	int	video_trap

	pop	cx
	pop	dx
	inc	dl
	loop	draw_loop

	pop	dx
	mov	ah,move_cursor
	mov	bh,default_page
	int	video_trap

	mov	cs:bg_showing,1
	ret
bg_draw	ENDP

bg_undraw	PROC	NEAR

	mov	ah,read_cursor
	mov	bh,default_page
	int	video_trap
	push	dx	; store current coordinates on stack

	mov	dh,0	; row 0
	mov	dl,80-bg_length
	mov	cx,bg_length
	mov	di,offset bg_save
	mov	si,offset bg_message
undraw_loop:
	push	dx
	push	cx

	mov	ah,move_cursor
	int	video_trap

	mov	ah,read_char	; check that the message character has not
	mov	bh,default_page	; been overwritten
	int	video_trap

	cmp	al,cs:[si]
	jne	undraw_skip

	mov	bl,cs:[di]	; get attribute from saved memory
	inc	di
	mov	al,cs:[di]	; and character
	inc	di
	mov	bh,0		; page 0
	mov	cx,1		; 1 character
	mov	ah,09H		; write character and attribute
	int	video_trap

undraw_skip:
	inc	si	; next character in message
	pop	cx
	pop	dx
	inc	dl
	loop	undraw_loop

	pop	dx
	mov	ah,02H	; set cursor position
	mov	bh,0	; page 0
	int	video_trap
	mov	cs:bg_showing,0
	ret
bg_undraw	ENDP

;==============================================================================
; int my_dos_open(char *name, int mode)
;
; I do not trust the Microsoft library routine dos_open, it does not seem to
; restore the data segment correctly before storing the result
;==============================================================================

PUBLIC _my_dos_open
_my_dos_open	PROC	NEAR
	pushall

	mov	dx,arg1[bp]
	mov	al,arg2[bp]
	mov	ah,03dH
	int	021H
	jnc	open_done
	mov	ax,-1
open_done:
	popall
	ret
_my_dos_open	ENDP  

;==============================================================================
;   int keyboard_rtn(void)
; 
;   perform an int 21 with function 06 (direct console I/O		    --;
;   returns 0 if no key waiting, otherwise the key. for the extended codes  --;
;   I read both keys and put non-zero in top byte			    --;
; 
;==============================================================================

PUBLIC _keyboard_rtn

_keyboard_rtn PROC NEAR
	mov	ah,01H
	int	KeyboardInt
	jz	nokey
	mov	ah,00H
	int	KeyboardInt
	cmp	al,0
	jne	normal_key

	mov	al,ah
	mov	ah,080H
	jmp	endkey

normal_key:
	xor	ah,ah
	jmp	endkey

nokey:
	mov	ax,0
endkey:
	ret

_keyboard_rtn	ENDP

;------------------------------------------------------------------------------
; Interface to the BIOS video routines
;------------------------------------------------------------------------------

PUBLIC _vbios_cls, _vbios_movecursor, _vbios_outputch
PUBLIC _vbios_scroll, _vbios_bell
PUBLIC _vbios_save, _vbios_restore

; void vbios_save(void)
; First, save the current cursor position, screen mode and screen state
; Then move cursor to Helios position vbios_y, vbios_x
_vbios_save	PROC	NEAR
	pushall

	mov	ah,read_cursor
	mov	bh,default_page
	int	video_trap
	mov	ds:_save_y,dh
	mov	ds:_save_x,dl

	mov	bx,24	; for (y = 24; y >= 0; y--)
	mov	si,offset screen_chars	; BYTE *chs = &(chars[0])
	mov	di,offset screen_attr	; BYTE *att = &(attr[0])
save_outer:
	mov	cx,79	; for (x = 79; x >= 0; x--)
save_inner:
	push	bx
	push	cx
			; move the cursor to the current loc
	mov	dh,bl	; row position
	mov	dl,cl	; column
	mov	ah,move_cursor
	mov	bh,default_page
	int	video_trap
			; extract character and attribute at that loc
	mov	ah,read_char
	mov	bh,default_page
	int	video_trap
			; now AH = attribute, AL = ascii character
	mov	cs:[di],ah	; *chs++ = ah
	mov	cs:[si],al	; *att++ = al
	inc	di
	inc	si
	pop	cx
	pop	bx
	dec	cx
	jns	save_inner
	dec	bx
	jns	save_outer
		
	mov	ah,move_cursor
	mov	bh,default_page
	mov	dh,ds:_vbios_y
	mov	dl,ds:_vbios_x
	int	video_trap

	popall
	ret
_vbios_save	ENDP

; vbios_restore(void)
; restore the screen to its old state
_vbios_restore	PROC	NEAR
	pushall

	mov	bx,24
	mov	si,offset screen_chars
	mov	di,offset screen_attr
restore_outer:
	mov	cx,79
restore_inner:
	push	bx
	push	cx

			; move the cursor to the current loc
	mov	dh,bl	; row position
	mov	dl,cl	; column
	mov	ah,move_cursor
	mov	bh,default_page
	int	video_trap
			; write character and attribute at that loc
	mov	bl,cs:[di]
	mov	al,cs:[si]
	inc	di
	inc	si
	mov	cx,1
	mov	ah,write_charattr
	mov	bh,default_page
	int	video_trap
	pop	cx
	pop	bx
	dec	cx
	jns	restore_inner
	dec	bx
	jns	restore_outer
	
	mov	ah,move_cursor
	mov	bh,default_page
	mov	dh,ds:_save_y
	mov	dl,ds:_save_x
	int	video_trap

	popall
	ret
_vbios_restore	ENDP

; void vbios_cls(void)
; _vbios_cls clears the screen using Initialise_window, and moves the cursor
; to the top-left corner
_vbios_cls	PROC	NEAR
	pushall
	mov	ah,scroll_up
	mov	al,0
	mov	bh,ds:_vbios_attr
	mov	cx,0	    	; top left
	mov	dx,184FH	; bottom right
	int	video_trap
	mov	ah,move_cursor
	mov	bh,default_page
	xor	dx,dx
	mov	ds:_vbios_x,dl
	mov	ds:_vbios_y,dh
	int	video_trap
	popall
	ret	
_vbios_cls	ENDP

; void vbios_movecursor(int y, int x)
_vbios_movecursor	PROC	NEAR
	pushall
	mov	ah,move_cursor
	mov	bh,default_page
	mov	dh,arg1[bp]
	mov	dl,arg2[bp]
	mov	ds:_vbios_x,dl
	mov	ds:_vbios_y,dh
	int	video_trap
	popall
	ret
	
_vbios_movecursor	ENDP

; void vbios_outputch(int x)
; This call outputs a single character at the current position with the
; current attributes, without moving the cursor. This means lots of calls
; to vbios_movecursor
_vbios_outputch	PROC	NEAR
	pushall
	mov	ax,arg1[bp]
	mov	ah,write_charattr
	mov	bh,default_page
	mov	bl,ds:_vbios_attr
	mov	cx,1
	int	video_trap
	mov	dl,ds:_vbios_x
	cmp	dl,79
	je	output_end
	inc	dl
	mov	ds:_vbios_x, dl
	mov	dh,ds:_vbios_y
	mov	ah,move_cursor
	mov	bh,default_page
	int	video_trap

output_end:
	popall
	ret
_vbios_outputch	ENDP

; void vbios_scroll(void)
; scroll up one line
_vbios_scroll	PROC	NEAR
	pushall
	mov	ah,scroll_up
	mov	al,1		; scroll up a single line
	mov	bh,ds:_vbios_attr
	mov	cx,0	    	; top left
	mov	dx,184FH	; bottom right
	int	video_trap
	popall
	ret
_vbios_scroll	ENDP

; void vbios_bell(void)	
; Ring ring !!!
_vbios_bell	PROC	NEAR
	pushall
	mov	ah,write_text
	mov	bh,default_page
	mov	al,7		; bell character
	int	video_trap
	popall
	ret
_vbios_bell	ENDP

;*--------------------------------------------------------------------------*;
;--                                                                         --;
;-- MODULE NAME: link.asm                                                   --;
;--                                                                         --;
;-- AUTHOR : B. Veer (based on original code by C. Grimsdale)		    --;
;-- DATE : 10/4/88                                                          --;
;-- UPDATES:                                                                --;
;--		10.1.90 changed to miniserv format, b004 only, small model  --;
;--									    --;
;-----------------------------------------------------------------------------;

;
; All the public data. First the routines that are the same for all
; transputer hardware. 
        PUBLIC _fetch_block, _send_block, _byte_to_link
	PUBLIC _byte_from_link, _rdrdy
        PUBLIC _reset, _init_link	
  
_link_read		DW	0150H
_link_write		DW	0151H
_link_in_status		DW	0152H 
_link_out_status	DW	0153H
_link_reset		DW	0160H
_link_analyse		DW	0161H
_link_DMArequest	DW	0162H
_link_interrupt_control	DW	0163H

_reset_timeout		EQU	9999

;  number of iterations for link timeout
D500Msec	EQU    65000 
 
 
Bit0               EQU 1
C_DATA_READY       EQU 1
C_TEST             EQU 3
 
 
SUCCESS            EQU 0
FAIL               EQU 1
 
;==============================================================================
; success = rdrdy ()
; int success;
;
; Check link for data, If byte ready return TRUE else return FALSE.
;
;==============================================================================
_rdrdy    PROC NEAR

	xor	ax,ax 
        mov     dx, cs:_link_in_status     ; output status register
        in      al, dx                  ; Read status
        and     al, Bit0                ; Test bit zero
        ret

_rdrdy	    ENDP 		 
 
;==============================================================================
; success = byte_to_link (x)
; int success;
; int x;  lower 8 bits output
;
; Output the given byte down the link. Return SUCCESS if the byte is output,
; else FAIL if the link adaptor is not ready after 500 Msec.
;
;==============================================================================
_byte_to_link   PROC NEAR
        push    bp                      ; C stack manipulation
        mov     bp, sp

	mov	bx,01H
l21:
        mov     cx,D500Msec
l1:
        mov     dx, cs:_link_out_status
        in      al, dx
        and     al, Bit0
        jnz     l2
        loop    l1

	dec	bx
	cmp	bx,0
	jne	l21

        mov     ax,FAIL
        jmp     l3
l2:
        mov     ax,arg1[bp]
        mov     dx,cs:_link_write
        out     dx,al
        mov     ax,SUCCESS
l3:
        pop     bp
        ret
 
_byte_to_link   ENDP


;==============================================================================
; success = byte_from_link (x)
; int success;
; char *x;
;
; Read a single byte from the link. Returns SUCCESS, or FAIL if the link
; timedout after 500Msec.
;
;==============================================================================
_byte_from_link   PROC NEAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
        push    si

	mov	bx,0100H
l20:
        mov     cx,D500Msec
l4:
        mov     dx,cs:_link_in_status
        in      al,dx
        and     al,Bit0
        jnz     l5
        loop    l4

	dec	bx
	cmp	bx,0
	jne	l20

        mov     ax,FAIL
        jmp     l6
l5:
        mov     dx,cs:_link_read
        in      al,dx
        mov     si,[bp+arg1]
        mov     ds:[si],al
        mov     ax,SUCCESS
l6:                                       ; read byte within 500 Msec.
        pop     si
        pop     bp
        ret
 
_byte_from_link   ENDP
 
;==============================================================================
; success = send_block (count, data, timeout)
; int success;
; int count
; char *data;
; int timeout;
;
; Output the given data block down the link. Return SUCCESS if the block  is
; output, else FAIL if the link adaptor is not ready within timeout.
;
;==============================================================================
_send_block PROC NEAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
        push    si

        mov     bx,arg3[bp]
l22:
	mov	cx,D500Msec
l7:
        mov     dx,cs:_link_out_status  ; wait "timeout" loops for linkout ready
        in      al,dx
        and     al,Bit0
        jnz     l8
        loop    l7

	dec	bx
	cmp	bx,0
	jne	l22

        mov     ax,arg1[bp]             ; amount of data failed to write
        jmp     l12

l8:
        mov     cx,arg1[bp]             ; amount of data to write
        mov     si,arg2[bp]             ; and the buffer
        xchg    ah,dl                   ; store lower byte of link_out_stat in ah
        mov     dx,cs:_link_write       ; I can now xchg ah,dl to switch from
                                        ; status port to write port
l9:
        mov     bx,cx                   ; store count remaining
        xchg    ah,dl                   ; check link out status
        mov     cx,D500Msec
l10:
        in      al,dx
        and     al,Bit0
        jz      l11
        xchg    ah,dl                   ; switch to link_write
	lodsb				; get next byte
        out     dx,al                   ; and output it
        mov     cx,bx                   ; restore count remaining
        loop    l9                      ; and loop for next byte
	mov	ax,SUCCESS		; finished
	jmp	l12

l11:
        loop    l10

        mov     ax,bx                   ; return count remaining
        jmp     l12
l12:
        pop     si
        pop     bp
 
        ret
 
_send_block   ENDP
 
;==============================================================================
; success = fetch_block (count, data, timeout)
; int success;
; char *data;
; int timeout;
;
; Read the required no. of bytes into the given buffer. Return SUCCESS if the
; block is input without error, else FAIL if the link adaptor is not ready
; within timeout.
;
; N.B ASSUME long pointers
;==============================================================================
_fetch_block PROC NEAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
	push	di

        mov     bx,arg3[bp]
l23:
	mov	cx,D500Msec
l13:
        mov     dx,cs:_link_in_status   ; wait "timeout" loops for linkout ready
        in      al,dx
        and     al,Bit0
        jnz     l14
        loop    l13

	dec	bx
	cmp	bx,0
	jne	l23

        mov     ax,arg1[bp]             ; amount of data failed to write
        jmp     l18
l14:
        mov     cx,arg1[bp]             ; amount of data to write
        mov     di,arg2[bp]             ; and the buffer
        xchg    ah,dl                   ; store lower byte of link_out_stat in ah
        mov     dx,cs:_link_read        ; I can now xchg ah,dl to switch from
                                        ; status port to write port
l15:
        mov     bx,cx                   ; store count remaining
        xchg    ah,dl                   ; check link out status
        mov     cx,D500Msec
l16:
        in      al,dx			; try once
        and     al,Bit0
	jz	l17
        xchg    ah,dl                   ; switch to link_write
        in      al,dx                   ; get byte
	stosb
        mov     cx,bx                   ; restore count remaining
	loop    l15                     ; and loop for next byte
        mov     ax,SUCCESS              ; finished
	jmp	l18

l17:
        loop    l16
        mov     ax,bx                   ; return count remaining
l18:
        pop     di
        pop     bp
 
        ret
  
_fetch_block   ENDP
 
 
;=============================================================================
; void b004_reset ()
;
; Reset root Transputer on a B004 board
;
;=============================================================================
_reset   PROC NEAR

#ifdef GEMINI
	mov	dx, 0120h
	mov	al,1
	out	dx,al
#endif
        mov     dx, cs:_link_reset         ; address of reset reg.
        mov     al, 1                   ; assert reset
        out     dx,al
        mov     cx, _reset_timeout        ; wait ? Msec
waitr:
        loop    waitr
        xor     al, 1                   ; release reset
        out     dx,al
 
        ret
 
_reset   ENDP
 
;=============================================================================
; void b004_init_link()
;
; This forces the analyse line down, instead of leaving it floating
;
;=============================================================================

_init_link	PROC	NEAR
	mov	dx, ds:_link_base	; 0150H
	mov	cs:_link_read,dx
	inc	dx
	mov	cs:_link_write,dx	; 0151H
	inc	dx
	mov	cs:_link_in_status,dx	; 0152H
	inc	dx
	mov	cs:_link_out_status,dx	; 0153H
	add	dx,0dH
	mov	cs:_link_reset,dx	; 0160H
	inc	dx
	mov	cs:_link_analyse,dx	; 0161H
	inc	dx
	mov	cs:_link_DMArequest,dx	; 0162H
	inc	dx
	mov	cs:_link_interrupt_control,dx	; 0163H
	
	mov	dx, cs:_link_analyse
	mov	al, 0
	out	dx, al
	ret
_init_link	ENDP

;==============================================================================
; link interrupt routines
;==============================================================================

MASK8259	EQU	0021H	; 8259 interrupt controller
CTRL8259	EQU	0020H
EOI		EQU	0020H

; This routine installs the interrupt routine at the appropriate offset.
; Then the link interrupt register is zapped to enable link-in interrupts.
; Note : the Parsytec hardware does not have this hardware.
; Finally the interrupt is enabled in the PC's 8259 interrupt controller.
; This leaves one bit that must be set to enable link interrupts :
; the bit in the C012 link adapter. This bit is controlled by enable_link_int
; and disable_link_int below.

PUBLIC _install_link_int
_install_link_int	PROC	NEAR
	push	ds

	mov	ax,ds:_int_level	; install interrupt routine
	add	ax,08H			; hardware interrupts are offset by 8
	mov	ah,025H
	mov	dx,cs
	mov	ds,dx
	mov	dx,offset link_interrupt
	int	21H

	pop	ds
#ifndef PARSY
			; not for Parsytec hardware
	mov	dx,cs:_link_interrupt_control
	mov	al,08H	; bit 3 controls link-in interrupts
	out	dx,al
#endif

	mov	bx,1	; enable the interrupt in the 8259
	mov	cx,ds:_int_level
	shl	bx,cl
	mov	dx,MASK8259
	not	bx
	in	al,dx
	and	ax,bx
	out	dx,al

	ret
_install_link_int	ENDP

enable_link_int	PROC	NEAR
	push	dx
	mov	dx,cs:_link_in_status
	mov	al,02h
	out	dx,al
	pop	dx
	ret
enable_link_int	ENDP

disable_link_int	PROC	NEAR
	push	dx
	mov	dx,cs:_link_in_status
	mov	al,0
	out	dx,al
	pop	dx
	ret
disable_link_int	ENDP

link_interrupt	PROC	NEAR
	int_push

	mov	dx,cs:_link_in_status	; check the link adapter
	in	al,dx			; to see if there is any data
	and	al,01H
	jnz	data_waiting
	mov	dx,CTRL8259	; send end-of-interrupt to 8259 controller
	mov	al,EOI		; even though it is not a link interrupt
	out	dx,al		; Any better suggestions ?
	jmp	interrupt_done

data_waiting:
	mov	dx,cs:_link_in_status	; disable the link interrupt
	mov	al,0
	out	dx,al

	mov	dx,CTRL8259	; send end-of-interrupt to 8259 controller
	mov	al,EOI		; whether it is a link interrupt or not
	out	dx,al

	inc	cs:Ints	; keep track

	mov	bx,cs:InDOS_Seg	; is there an MS-DOS call in progress ?
	mov	ds,bx
	mov	bx,cs:InDos_Off
	mov	al,ds:[bx]	; get the InDOS flag
	cmp	al,0
	je	handling_safe

	inc	cs:Clashes

	mov	cs:link_pending,1	; set the flag
	jmp	interrupt_done		; and do nothing else
		; on the next idle interrupt, or the next timer interrupt
		; after leaving DOS, the request will be handled

handling_safe:				; the request can be handled now
	switch_C_stack
	call	switch_C
	enable_ints
	call	_handle_data
	disable_ints
	call	switch_back
	switch_back_stack
	
	mov	dx,cs:_link_in_status	; reenable the link interrupt
	mov	al,02H
	out	dx,al
interrupt_done:

	int_pop
	iret
link_interrupt	ENDP
	
_TEXT    ENDS

        END
 
