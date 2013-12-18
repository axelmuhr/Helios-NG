;*---------------------------------------------------------------------------*;
;--                        HELIOS PC  I/O SYSTEM                            --;
;--                        =====================                            --;
;--                                                                         --;
;--             Copyright (C) 1987, Perihelion Software Ltd.                --;
;--               All Rights Reserved.                                      --;
;--                                                                         --;
;--                                                                         --;
;--                                                                         --;
;-- MODULE NAME: PCASM.ASM                                                  --;
;--                                                                         --;
;-- AUTHOR : BLV                                                            --;
;-- DATE : 30/5/88                                                          --;
;-----------------------------------------------------------------------------;
;-- RcsId: $Id: pcasm.cpp,v 1.20 1993/12/01 17:34:17 bart Exp $	--;
;-- Copyright (C) 1988, Perihelion Software Ltd.   			    --;
;
; BLV - Please note that if this file is compiled for Windows then it needs
; masm 5.0 or later.

;------------------------------------------------------------------------------
; This file is passed through the C preprocessor, to eliminate unnecessary
; options. For example, the rs232 support in this module is eliminated if
; it is not currently compiled into the I/O Server.
;------------------------------------------------------------------------------
#include "../defines.h"

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
;    And some macros
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

; DPMI support
RealModeRegs	STRUC
	r_DI		DW	0	; used to return buffer
	r_DI_E		DW	0	; not relevant
	r_ESI		DD	0	; not relevant
	r_EBP		DD	0	; not relevant
	r_RESERVED	DD	0	; reserved, should be zero
	r_EBX		DD	0	; not relevant
	r_EDX		DD	0	; not relevant
	r_CX		DW	0	; contains size
	r_CX_E		DW	0	; not relevant
	r_AX		DW	0	; 0 or 1, as per Clarkson spec
	r_AX_E		DW	0	; not relevant
	r_FLAGS		DW	0	; not relevant
	r_ES		DW	0	; used to return buffer
	r_DS		DW	0	; not relevant
	r_FS		DW	0	; not relevant
	r_GS		DW	0	; not relevant
	r_IP		DW	0	; update to simulate return
	r_CS		DW	0	; update to simulate return
	r_SP		DW	0	; update to simulate return
	r_SS		DW	0	; not relevant
RealModeRegs	ENDS

;==============================================================================
; External functions are declared outside the code segment, as recommended in
; the linker documentation. This avoids fixup overflow errors.
;==============================================================================
#if (mouse_supported && !MSWINDOWS)
EXTRN	_mouse_something:FAR
#endif
#if keyboard_supported
EXTRN	_keyboard_something:FAR
#endif
#if RS232_supported
EXTRN	_RS232_InterruptHandler:FAR
#if (MSWINDOWS)
EXTRN	GlobalDosAlloc:FAR, GlobalDosFree:FAR
#endif
#endif

#if  !(use_own_memory_management)
#define _get_mem	_malloc
#define _free_mem	_free
#endif
EXTRN	_get_mem:FAR, _free_mem:FAR

;==============================================================================
; Static data for the default data segment, all variables should end up here
; eventually to cope with protected mode operation. External variables in the
; default data segment should also be declared here
;==============================================================================

DGROUP  GROUP _DATA
        ASSUME ds:DGROUP
_DATA  SEGMENT WORD PUBLIC 'DATA'

#if (Ether_supported)
EXTRN	_pkt_rcvd 	: WORD	; incremented when packet received
EXTRN	_pkt_len  	: DWORD	; packet length vector
EXTRN	_pkt_too_long  	: WORD	; error flag (will contain pkt length)

#if (!MSWINDOWS)
EXTRN	_pkt_table	: DWORD	; packet buffer vector
#else
EXTRN   _pkt_table_segment : WORD	; segment for packet buffer
#endif

	; ethernet local data
rcvr_index      dw 0            ; index into packet buffer

#endif

		; Space for the coroutine library variables
PUBLIC	_CurrentCo
_CurrentCo	DW	0
		DW	0
RootCo		DW	0
		DW	0
func		DW	0	; to store function addresses during
		DW	0	; coroutine switch

		; The Windows version of the coroutine library is very nasty.
		; There is no legal way of switching stacks, as Windows keeps
		; track of stack base and limit and checks these during
		; all calls. Hence it is necessary to copy the stack into a
		; buffer on main()'s stackframe with every coroutine switch.
		; This is also necessary with Microsoft C 7.0
		;
		; coroutine_buf is set to point at this buffer by main()
PUBLIC	_coroutine_buf
_coroutine_buf		DD	0

		; Similar support for an interrupt stack
PUBLIC	_interrupt_buf
_interrupt_buf		DD	0

		; Space to save current SS:SP when switching to interrupt stack
interrupt_save_ss	DW	0
interrupt_save_sp	DW	0

PUBLIC _version_number		; needed for incompatibility problems,
_version_number	DW	0	; particularly with the rawdisk device

		; These variables are used to exchange error information,
		; they may be set by the DOS critical error handler.
#if floppies_available
PUBLIC _floppy_errno
_floppy_errno		DW	0
#endif
#if Centronics_supported
PUBLIC _Centronics_errno
_Centronics_errno	DW	0
#endif
#if Printer_supported
PUBLIC _Printer_errno
_Printer_errno		DW	0
#endif

		; These variables are used to save old interrupt vectors,
		; as the I/O Server traps and ignores certain interrupts. 
zero_off	DW	0
zero_seg	DW	0
overflow_off	DW	0
overflow_seg	DW	0
prtscrn_off	DW	0
prtscrn_seg	DW	0

#if keyboard_supported
seg_keyint	DW	0	; space to save yet another interrupt vector
off_keyint	DW	0
#endif

#if !MSWINDOWS
PUBLIC _vbios_attr
_vbios_x	DB	0		; for the video bios interface
_vbios_y	DB	0
video_mode	DB	0
_vbios_attr	DB	default_attr
#endif

#if gem_supported
		; junk for GEM, not really needed anymore
PUBLIC _mouse_x, _mouse_y, _button_state ; external variables for GEM
PUBLIC _mouse_changed, _button_changed, _cursor_changed
PUBLIC _syscursor

_syscursor	DD 0
_mouse_x	DW 0
_mouse_y	DW 0
_button_state	DW 0
_mouse_changed	DW 0
_button_changed	DW 0
_cursor_changed	DW 0
#endif

#if Rawdisk_supported
transfer_buffer	DB	0,0,0,0,0,0,0,0,0,0	; 10 byte buffer
#endif

	; Link I/O variables
PUBLIC _link_base, _link_read, _link_write, _link_in_status
PUBLIC _link_out_status
PUBLIC _link_reset, _link_analyse
PUBLIC _control_write
PUBLIC	_dma_request, _int_enable, _dma_channel
PUBLIC _reset_timeout, _analyse_timeout
  
control_mask    	DB 0        	; last value written to control reg.
_link_base		DW	0150H
_link_read		DW	0150H
_link_write		DW	0151H
_link_in_status		DW	0152H 
_link_out_status	DW	0153H
_link_reset		DW	0160H
_link_analyse		DW	0161H
_dma_request		DW	0162H
_int_enable		DW	0163H
_dma_channel		DW	0
_control_write		DW	0104H
_reset_timeout		DW	8000
_analyse_timeout	DW	4000

	; DOS DEV Data Areas
_dos_handle		DW	0	; handle to the link name
_dos_status		DW	0	; last link statistic read
			DW 	0
_dos_dev_word		DW	0	; Dos function command area
			DW	0

; TMS320C40 specific variables
PUBLIC _hunt_timeout, _hunt_hiperf_seg
_hunt_hiperf_seg	DW	0d000H	; default to 0xc0000
_hunt_fifo_size		DW	04000H	; 16k fifo
_hunt_timeout		DW	157H	; bit7 = 1 == timeout on fifo transfer

_tdb_wordsaved		DW	0	; false = word buffer not in use
_tdb_wordbuffer1	DW	0	; used to store blocking 16 bit word
_tdb_wordbuffer2	DW	0	; used to store blocking 2nd 16 bit word
_tdb_bytesaved		DW	0	; false = byte buffer not in use
_tdb_bytebuffer		DB	0	; used to store extra byte read.

_DATA  ENDS

;==============================================================================
; Code segment. To cope with running in protected mode, mainly for Windows,
; all variables that used to be in the code segment are now in the data
; segment.
;==============================================================================

CODE    SEGMENT WORD public 'CODE'
        ASSUME cs:CODE

#if !(MSWINDOWS)        
;==============================================================================
;   int critic_error(void)						    --;
; 									    --;
;   This routine is called when a critical error is called, e.g. when there --
;   is no floppy in the drive.   
;==============================================================================
_critic_error	PROC FAR
	test	ah,80h			; check top bit, disk error
	jne	endcrit			; not a disk error

	mov	ax,0002h
	cmp	di,00h			; write protected
	je	setcrit
	mov	al,01h			; default error

setcrit:	
	push	dx
	push	ds
	mov	dx,DGROUP
	mov	ds,dx
	mov	_floppy_errno,ax
	pop	ds
	pop	dx

endcrit:
	mov	al,3		; fail the system call

	iret

_critic_error	ENDP
#endif

;==============================================================================
;   void dummy_interrupt(void)
;
;   This routine is installed as the interrupt handler for certain exceptions,
;   divide-by-0, overflow, and print-screen
;==============================================================================
_dummy_interrupt	PROC	FAR
	iret
_dummy_interrupt	ENDP

_divzero	PROC	FAR
	sti
	int_push

	mov	dx,seg warn
	mov	ds,dx
	mov	dx,offset warn
	mov	ah,9
	int	21H

	int_pop
	iret
_divzero	ENDP

warn	db	'*** Internal error : Divide by zero detected.',0dh, 0ah, '$'

;=============================================================================;
; void set_interrupts(void)
;
; Set some of the interrupts:
;     1) the critical error interrupt which normally puts Abort, Retry, Cancel
;	 on the screen. Ineffective under Windows.
;     2) the divide-by-zero and overflow internal interrupts, to stop runtime
;	 unwanted exits from the server
;     3) the print-screen interrupt, which could be very unpleasant.
;        Also ineffective under Windows.
;
; void restore_interrupts(void)
;
; undo the damage done by set_interrupts
;=============================================================================;
PUBLIC _set_interrupts, _restore_interrupts
_set_interrupts PROC FAR
	pushall

	push	ds		; save the default data segment

	mov	ah,030H		; obtain the MSdos version number
	mov	al,0H
	int	21H
	xor	ah,ah
	pop	ds
	push	ds
	mov	_version_number,ax

#if !(MSWINDOWS)
	mov	ah,025h		; function 25 - set interrupt vector
	mov	al,024h		; interrupt vector for critical error
	mov	dx,seg _critic_error	; this will be restored by MSdos
	mov	ds,dx			; so I do not need to save the old
	mov	dx,offset _critic_error	; vector
	int	21h

	pop	ds
	push	ds
	mov	ah,035h		; save the old interrupt vector for print
	mov	al,005h		; screen
	int	21h
	pop	ds
	push	ds
	mov	prtscrn_off,bx
	mov	prtscrn_seg,es

	mov	ah,025h
	mov	al,005h		; interrupt vector for print screen
	mov	dx,seg	_dummy_interrupt
	mov	ds,dx
	mov	dx,offset _dummy_interrupt
	int	21h
#endif

	pop	ds
	push	ds
	mov	ah,035h		; save the old interrupt vector for divide by
	mov	al,000h		; zero
	int	21h
	pop	ds
	push	ds
	mov	zero_off,bx
	mov	zero_seg,es

	mov	ah,025h		; install my own interrupt vector
	mov	al,000h		
 	mov	dx,seg	_divzero
	mov	ds,dx
	mov	dx,offset _divzero
	int	21h

	pop	ds
	push	ds
	mov	ah,035h		; save the old interrupt vector for overflow
	mov	al,004h
	int	21h
	pop	ds
	push	ds
	mov	overflow_off,bx
	mov	overflow_seg,es

	mov	ah,025h
	mov	al,004h		; interrupt vector for overflow
	mov	dx,seg	_dummy_interrupt
	mov	ds,dx
	mov	dx,offset _dummy_interrupt
	int	21h

	pop	ds

	popall
	ret

_set_interrupts ENDP

_restore_interrupts	PROC	FAR
	pushall

	push	ds		; save default data segment
	mov	ax,zero_seg
	mov	dx,zero_off
	mov	ds,ax
	mov	ah,025H
	mov	al,000H
	int	21h

	pop	ds
	push	ds
	mov	ax,overflow_seg
	mov	dx,overflow_off
	mov	ds,ax
	mov	ah,025H
	mov	al,004H
	int	21h

#if !(MSWINDOWS)
	pop	ds
	push	ds
	mov	ax,prtscrn_seg
	mov	dx,prtscrn_off
	mov	ds,ax
	mov	ah,025H
	mov	al,005H
	int	21h
#endif

	pop	ds
	popall
	ret
_restore_interrupts	ENDP

;==============================================================================
;   int keyboard_rtn(void)
; 
;   perform an int 21 with function 06 (direct console I/O		    --;
;   returns 0 if no key waiting, otherwise the key. for the extended codes  --;
;   I read both keys and put non-zero in top byte			    --;
; 
;==============================================================================

PUBLIC _keyboard_rtn

_keyboard_rtn PROC FAR
	pushall

	mov	ah,6		; the function number
	mov	dl,0ffh		; parameter for read
	int	21h		; DOS call
	jz	nokey

	mov	ah,0		; clear top byte
	test	al,-1		; check for byte 0 (extended key)
	jne	endx
	mov	ah,6		; get another key
	mov	dl,0ffh
	int	21h
	mov	ah,080h
	jmp	short endx
nokey:
	mov	ax,0
endx:
	popall
	ret

_keyboard_rtn	ENDP

;------------------------------------------------------------------------------
; Interface to the mouse device via int 33
;------------------------------------------------------------------------------

#if (mouse_supported && !MSWINDOWS)
mouse_trap	EQU	33H

PUBLIC _reset_mouse, _enable_mouse, _disable_mouse, _set_mouse_resolution

_reset_mouse	PROC	FAR
	pushall

	mov	ah,35h		; check that the interrupt routine is not NULL
	mov	al,mouse_trap
	int	21h
	mov	ax,es
	or	ax,bx
	je	resetm_end

	mov	ax,0
	int	mouse_trap	; this leaves 0 in ax if not installed
	
resetm_end:
	popall
	ret
_reset_mouse	ENDP

_enable_mouse	PROC	FAR
	pushall

	mov	ax,19
	mov	dx,1000h
	int	mouse_trap

	mov	ax,12		; enable mouse_event as an event_handler
	mov	cx,07fh
	mov	dx,seg mouse_event
	mov	es,dx
	mov	dx,offset mouse_event
	int	mouse_trap
	
	popall
	ret
_enable_mouse	ENDP

_disable_mouse	PROC	FAR
	pushall

	mov	ax,12		; disable all events
	mov	cx,0
	int	mouse_trap

	popall
	ret
_disable_mouse	ENDP

_set_mouse_resolution	PROC	FAR
	pushall

	mov	ax,0fh			; fn for set mouse resolution
	mov	cx,arg1[bp]	; x resolution
	mov	dx,cx		 ; y resolution = 2 * x
;	add	dx,cx
	int mouse_trap

	popall
	ret
_set_mouse_resolution	ENDP
	
mouse_event	PROC	FAR
	int_push

	push	bx			; buttons
        mov 	ax,11		
	int	mouse_trap		
        push    dx			; y
        push    cx			; x
	mov	ax,DGROUP
	mov	ds,ax
	call	_mouse_something
	add	sp,6

	int_pop
	ret
mouse_event	ENDP

#endif

;------------------------------------------------------------------------------
; Interface to the keyboard interrupt routine
;------------------------------------------------------------------------------

#if keyboard_supported
PUBLIC _enable_keyboard, _disable_keyboard

_enable_keyboard	PROC	FAR
	pushall

	mov	ax,3509H	; save the old keyboard interrupt vector
	int	21h
	mov	seg_keyint,es
	mov	off_keyint,bx

	mov	ax,seg keyboard_int
	mov	dx,offset keyboard_int
	mov	ds,ax
	mov	ax,2509h	; and install my own
	int	21h

	popall
	ret
_enable_keyboard	ENDP

_disable_keyboard	PROC	FAR
	pushall

	mov	dx,off_keyint
	mov	ax,seg_keyint	; restore the old interrupt vector
	mov	ds,ax
	mov	ax,2509h
	int	21h
	
	popall
	ret
_disable_keyboard	ENDP

eoi	EQU	20h
pictrl	EQU	20h
picmsk	EQU	21h
kb_data	EQU	60h
kb_ctl	EQU	61h

keyboard_int		PROC	FAR
	int_push

	mov	dx,DGROUP		; restore the data segment
	mov	ds,dx

	xor ah,ah			; get the scan code
	in	al,kb_data
	push	ax			; put code on stack for C routine

	mov	al,eoi
	out	pictrl,al

	call	FAR PTR _keyboard_something	; process and buffer the key
	pop	ax

	int_pop
	iret
keyboard_int		ENDP
#endif

;------------------------------------------------------------------------------
; Interface to the BIOS video routines
;------------------------------------------------------------------------------

#if !MSWINDOWS
video_trap	EQU	10H
set_video	EQU	 0 
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
default_video	EQU	2	; 80*25 black and white text
default_attr	EQU	7	; normal plain video
default_page	EQU	0

PUBLIC _vbios_init, _vbios_tidy
PUBLIC _vbios_cls, _vbios_movecursor, _vbios_outputch
PUBLIC _vbios_scroll, _vbios_bell


; void vbios_init(void)
; This is currently a no-op
_vbios_init	PROC	FAR
	ret
_vbios_init	ENDP

; void vbios_tidy(void)
; This is currently a no-op
_vbios_tidy	PROC	FAR
	ret
_vbios_tidy	ENDP

; void vbios_cls(void)
; _vbios_cls clears the screen using Initialise_window, and moves the cursor
; to the top-left corner
_vbios_cls	PROC	FAR
	pushall
	mov	ah,scroll_up
	mov	al,0
	mov	bh,_vbios_attr
	mov	cx,0	    	; top left
	mov	dx,184FH	; bottom right
	int	video_trap
	mov	ah,move_cursor
	mov	bh,default_page
	mov	dh,0
	mov	dl,0
	mov	_vbios_x,dl
	mov	_vbios_y,dh
	int	video_trap
	popall
	ret	
_vbios_cls	ENDP

; void vbios_movecursor(int y, int x)
_vbios_movecursor	PROC	FAR
	pushall
	mov	ah,move_cursor
	mov	bh,default_page
	mov	dh,arg1[bp]
	mov	dl,arg2[bp]
	mov	_vbios_x,dl
	mov	_vbios_y,dh
	int	video_trap
	popall
	ret
	
_vbios_movecursor	ENDP

; void vbios_outputch(int x)
; This call outputs a single character at the current position with the
; current attributes, without moving the cursor. This means lots of calls
; to vbios_movecursor, at least until I optimise it.
_vbios_outputch	PROC	FAR
	pushall
	mov	ax,arg1[bp]
	mov	ah,write_charattr
	mov	bh,default_page
	mov	bl,_vbios_attr
	mov	cx,1
	int	video_trap
	mov	dl,_vbios_x
	cmp	dl,79
	je	output_end
	add	dl,1
	mov	_vbios_x, dl
	mov	dh,_vbios_y
	mov	ah,move_cursor
	mov	bh,default_page
	int	video_trap

output_end:
	popall
	ret
_vbios_outputch	ENDP

; void vbios_scroll(void)
; scroll up one line
_vbios_scroll	PROC	FAR
	pushall
	mov	ah,scroll_up
	mov	al,1		; scroll up a single line
	mov	bh,_vbios_attr
	mov	cx,0	    	; top left
	mov	dx,184FH	; bottom right
	int	video_trap
	popall
	ret
_vbios_scroll	ENDP

; void vbios_bell(void)	
; Ring ring !!!
_vbios_bell	PROC	FAR
	pushall
	mov	ah,write_text
	mov	bh,default_page
	mov	al,7		; bell character
	int	video_trap
	popall
	ret
_vbios_bell	ENDP

#endif /* MSWINDOWS */

;------------------------------------------------------------------------------
;-- GEM VDI vectors
;------------------------------------------------------------------------------

#if gem_supported
PUBLIC _mousemove_vector, _mousebutton_vector, _cursorchange_vector

	; N.B. These are interrupt routines 
_mousemove_vector PROC FAR
	push	dx
	push	ds
	mov	dx,DGROUP
	mov	ds,dx
	mov _mouse_x,bx
	mov _mouse_y,cx
	mov _mouse_changed, 1
	pop	ds
	pop	dx
	ret
_mousemove_vector ENDP

_mousebutton_vector PROC FAR
	push	dx
	push	ds
	mov	dx,DGROUP
	mov	ds,dx
	mov	_button_state,ax
	mov	_button_changed,1
	pop	ds
	pop	dx
	ret
_mousebutton_vector ENDP

_cursorchange_vector PROC FAR
	int_push
	mov	dx,DGROUP
	mov	ds,dx
	mov	_mouse_x,bx
	mov	_mouse_y,cx
	mov	_cursor_changed,1
	xor si,si
	call _syscursor[si]
	int_pop
	ret
_cursorchange_vector ENDP

#endif

;-----------------------------------------------------------------------------;
;-- WORD call_a_trap(MCB *mcb)                                              --;
;--                                                                         --;
;-- Activate trap 0x60, with the mcb in dx. It should                    --;
;-- return a reply in dx:ax in less than two seconds, or else...            --;
;--                                                                         --;
;-----------------------------------------------------------------------------;

#if interaction_supported
PUBLIC _call_a_trap

_call_a_trap	PROC	FAR
	pushall
	mov	dx,arg2[bp]		; extract the mcb
	mov	ds,dx
	mov	dx,arg1[bp]
        int	60H
	popall
	ret
_call_a_trap	ENDP
#endif

;-----------------------------------------------------------------------------;
;--                                                                         --;
;-- RS232 support                                                           --;
;--                                                                         --;
;-----------------------------------------------------------------------------;

#if RS232_supported 

PUBLIC _RS232_interrupt

#if (MSWINDOWS && 1)

_RS232_interrupt	PROC FAR
; ==============================================================================
; -- DPMI real mode callback
; -- Under Windows the interrupt routine is invoked via DPMI callbacks and
; -- must perform a DPMI return.
; -- ENTRY:
; --	DS:SI = real mode SS:SP
; --	ES:DI = real mode call structure
; -- EXIT:
; --	ES:DI = real mode call structure
; -- refer DPMI spec. V1.0 pp. 34-38
; ==============================================================================

; The callback function is passed a real mode register data structure.
;	cs:ip	must be filled in with return address on stack
	
	; Install the return value in the RealModeRegs structure
	; See DPMI spec., example on page 38
	cld
	lodsw
	mov	es:[di.r_IP], ax
	lodsw
	mov	es:[di.r_CS], ax
	add	es:[di.r_SP], 6
	
	; Preserve es:di across the C call
	push	es
	push	di

	; Switch to default data segment to access I/O Server's statics
	mov	ax, DGROUP
	mov	ds, ax

	; Store SS:SP prior to switching to interrupt stack
	mov	ds:interrupt_save_sp, sp
	mov	ds:interrupt_save_ss, ss

	; Switch to the interrupt stack
	les	di,ds:_interrupt_buf
	mov	sp,di
	mov	di,es
	mov	ss,di

	; We are now in a standard C world, with SS == DS == default data segment
	; The actual interrupt can now be handled entirely in C	
	call	FAR PTR _RS232_InterruptHandler

	; Restore the stack, and return back to DPMI
	mov	sp,ds:interrupt_save_sp
	mov	ss,ds:interrupt_save_ss
	
	pop	di
	pop	es
	iret
_RS232_interrupt	ENDP

#else

; Under DOS the interrupt routine is a real interrupt handler and
; must perform an interrupt return.
_RS232_interrupt	PROC FAR
	int_push
	cld

	; Switch to default data segment to access I/O Server's statics
	mov	ax, DGROUP
	mov	ds, ax

	; Store SS:SP prior to switching to interrupt stack
	mov	ds:interrupt_save_sp, sp
	mov	ds:interrupt_save_ss, ss

	; Switch to the interrupt stack
	les	di,ds:_interrupt_buf
	mov	sp,di
	mov	di,es
	mov	ss,di

	; We are now in a standard C world, with SS == DS == default data segment
	; The actual interrupt can now be handled entirely in C	
	call	FAR PTR _RS232_InterruptHandler

	; Restore the stack, and return from interrupt
	mov	sp,ds:interrupt_save_sp
	mov	ss,ds:interrupt_save_ss
	
	int_pop
	sti	; This appears to be needed
	iret
_RS232_interrupt	ENDP
#endif

#endif

;==============================================================================
; 
;  Raw disk support
;
;==============================================================================

#if Rawdisk_supported
PUBLIC _disk_read, _disk_write

read_int	EQU	025h
write_int	EQU	026h

;==============================================================================
; _disk_read(int drive, int no_sects, WORD first_sec, byte *buf)
;==============================================================================

_disk_read	PROC	FAR
	pushall

	mov	ax,arg1[bp]	; drive id, 0 = A, 1 = B etc.
	cmp	ax,2
	jne	rdrive_ok
	mov	ax,0310H
	jmp	short readdone

rdrive_ok:	
	mov	bx,_version_number
	cmp	bx,4
	jge	msdos4_read

	mov	cx,arg2[bp]	; no_sects to read
	mov	dx,arg3[bp]	; starting sector	; low 2 bytes
	mov	bx,arg6[bp]	; segment of buffer
	mov	ds,bx
	mov	bx,arg5[bp]	; offset of buffer
	int	read_int	; do the read

	jc	read_error	; carry flag set if error, with code in ax
	xor	ax,ax		; no error so return 0
read_error:
	add	sp,2		; as per instructions
	jmp	short readdone

msdos4_read:
	mov	cx,-1
	mov	bx,DGROUP
	mov	ds,bx
	mov	si,offset dgroup:transfer_buffer
	mov	bx,arg3[bp]	; first sector, low 2 bytes
	mov	[si],bx
	mov	bx,arg4[bp]	; first sector, high bytes
	mov	[si+2],bx
	mov	bx,arg2[bp]	; number of sectors
	mov	[si+4],bx
	mov	bx,arg5[bp]	; offset of buffer
	mov	[si+6],bx
	mov	bx,arg6[bp]	; segment of buffer
	mov	[si+8],bx
	mov	bx,si		; bx now contains structure
	int	read_int	; do the read

	jc	read_error2	; carry flag set if error, with code in ax
	xor	ax,ax		; no error so return 0
read_error2:
	add	sp,2		; as per instructions

readdone:
	popall
	ret
_disk_read	ENDP

;==============================================================================
; _disk_write(int drive, int no_sects, WORD first_sec, byte *buf)
;==============================================================================

_disk_write	PROC	FAR
	pushall

	mov	ax,arg1[bp]	; drive id
	cmp	ax,2
	jne	wdrive_ok
	mov	ax,0310H
	jmp	short writedone

wdrive_ok:
	mov	bx,_version_number
	cmp	bx,4
	jge	msdos4_write

	mov	cx,arg2[bp]	; no_sects
	mov	dx,arg3[bp]	; first_sector
	mov	bx,arg6[bp]	; segment of buffer
	mov	ds,bx
	mov	bx,arg5[bp]	; offset of buffer

	cmp	al,2		; check that we are not writing to disk C
	jne	write_safe
	mov	ax,0300h	; write protect error
	jmp	short write_error
write_safe:

	int	write_int

	jc	write_error	; carry set if error, with error code in ax
	xor	ax,ax
write_error:
	add	sp,2
	jmp	short writedone

msdos4_write:
	mov	cx,-1
	mov	bx,DGROUP
	mov	ds,bx
	mov	si,offset dgroup:transfer_buffer
	mov	bx,arg3[bp]	; first sector, low 2 bytes
	mov	[si],bx
	mov	bx,arg4[bp]	; first sector, high bytes
	mov	[si+2],bx
	mov	bx,arg2[bp]	; number of sectors
	mov	[si+4],bx
	mov	bx,arg5[bp]	; offset of buffer
	mov	[si+6],bx
	mov	bx,arg6[bp]	; segment of buffer
	mov	[si+8],bx
	mov	bx,si		; bx now contains structure
	int	write_int	; do the read

	jc	write_error2	; carry flag set if error, with code in ax
	xor	ax,ax		; no error so return 0
write_error2:
	add	sp,2		; as per instructions

writedone:
	popall
	ret
_disk_write	ENDP

#endif
 
;-----------------------------------------------------------------------------
; The coroutine library. This comes in two flavours. The standard version for
; DOS is a fairly straight-forward implementation. Unfortunately it does not
; work for Windows because Windows does not support multiple stacks. In
; particular Windows appears to keep track of the stack base and limit
; for every program running under it, and if a program calls into Windows
; these are checked. Coroutines would result in unrecoverable application
; errors.
;
; With Microsoft C 7.0 some of the C library routines appear to have built-in
; stack checking. Hence the same problems occur.
;
; To get around this I have implemented an alternative version of the
; coroutine library. This assumes that there is a suitably large buffer
; on the stackframe of main(). Whenever a child coroutine is activated
; its stack is copied from the usual place into this buffer. When the
; coroutine suspends the buffer is copied back onto the stack. Paul Beskeen
; is responsible for thinking of this hack.
;
; BLV, 12.5.92
;-----------------------------------------------------------------------------
 
PUBLIC  _InitCo, _CreateCo, _CallCo, _WaitCo, _DeleteCo
  
; Co-routine data structure
; longword *co_sp       stack pointer for this routine
; longword *co_base	base address of the coroutine
; longword *co_parent   parent coroutine
; longword *co_func     initial entry point to coroutine
; int      co_size      size of stack allocated to routine
; int      magic        magic number to check for overflow
;
; All longwords are saved as offset:segment
; It is assumed that all pointers are long pointers.
 
co_sp           EQU 0
co_base		EQU 4
co_parent       EQU 8
co_func         EQU 12
co_size         EQU 16
co_magic        EQU 18
co_SIZEOF       EQU 20
magic_value     EQU 07654h 
NULL            EQU 0
 

;==============================================================================
; coroutine = CreateCo(function, size)
;  long *function
;  long size
;
; Creates and adds a co-routine with the required stacksize and start function.
;  Size is given in bytes.
;==============================================================================
_CreateCo PROC FAR
        push    bp                      ; save bp
        mov     bp,sp                   ; stack frame ptr
        push    ds                      ; save segment registers
	push	di
	push	si
 
        mov     cx,arg3[bp]                ; lower 16 bits of size
 
        add     cx,co_SIZEOF            ; add length of coroutine structure
        push    cx
        call    _get_mem		; get required amount of stack
	pop	cx
	sub	cx,co_SIZEOF		; restore stack size

        test    ax,-1                   ; NULL ptr ?
        jne     gotmem
        test    dx,-1 
        jne     gotmem

	pop	si
	pop	di
	pop	ds
	pop	bp 
        ret

gotmem:
        mov     bx,ds                   ; avoid corrupting ds
 
        mov     es,dx
        mov     di,ax
        lds     si,DWORD PTR _CurrentCo
					; es:di = new coroutine (also dx:ax)
					; si = current coroutine (parent)
 
        mov     es:co_size[di],cx       ; size of coroutine stack

	add	ax,co_SIZEOF		; base of coroutine stack
	mov	es:co_base[di],ax
	mov	es:co_base[di+2],dx

        mov     es:co_parent[di],si
        mov     es:co_parent[di+2],ds   ; parent = current coroutine

	mov	ax,magic_value
	mov	es:co_magic[di],ax

        mov     ax,arg1[bp]
        mov     dx,arg2[bp]
        mov     es:co_func[di],ax       ;
        mov     es:co_func[di+2],dx     ; coroutine entry point
 
		; At this point it is necessary to make the new coroutine
		; current, setting up the stack frame appropriately.
		; es:di = new coroutine, si = old coroutine,
		; cx = stacksize, bx = default ds
	call	FAR PTR save_stack

        mov     co_sp[si],sp		; save the stack pointer for the old
        mov     co_sp[si+2],ss		; coroutine
 
		; Update currentCo
        mov     ds,bx			; restore ds
        mov     _CurrentCo,di
        mov     _CurrentCo+2,es	; make new coroutine current

		; The coroutine base address is determined by _coroutine_buf
	les	di, DWORD PTR _coroutine_buf
	add	cx,di			; go to top of stack
	push	es			; use the stack to switch the stack.
	cli
	pop	ss
	mov	sp,cx
	sti

		; This call to WaitCo will cause the parent coroutine to be
		; reactivated, with es:di as the result. This should be the
		; coroutine base address, still held in CurrentCo
	les	di, DWORD PTR _CurrentCo
        push    es
        push    di
 
cocoenter:  call    FAR PTR _WaitCo         ; Waitco back to parent
        add     sp,4
 
		; At this point the parent coroutine has been reactivated
		; and has done a StartCo on the newly-created coroutine.
		; dx:ax, the result of the WaitCo(), correspond to the
		; argument of the StartCo. This code now uses the
		; function passed to CreateCo, held in the coroutine
		; structure, and calls it. The coroutine is now up and running
        push    dx                      ; Parameter for coroutine entry
        push    ax			; == result of WaitCo
        les     di,DWORD PTR _CurrentCo

        mov     dx,es:co_func[di]       ; Entry point offset
	mov	func,dx
	mov	dx,es:co_func+2[di]	; Entry point segment
	mov	func+2,dx		; into CS memory
	lea	bx,func
	call    DWORD PTR [bx]
        jmp     short cocoenter                   ; loop forever
_CreateCo ENDP
 
	; Routine to copy the stack from coroutine_buf to the save
	; space associated with the coroutine. If the current coroutine
	; is the root coroutine then this is a no-op
	; N.B. no assumptions should be made about register values, although
	; the current stack is usable
save_stack PROC FAR
	push	ds
	push	es
	push	di
	push	si
	push	cx
	push	dx
	push	ax

	mov	dx, DGROUP	; restore default data segment
	mov	ds,dx
	les	di,DWORD PTR RootCo
	lds	si,DWORD PTR _CurrentCo

	mov	ax,ds
	mov	cx,es
	cmp	ax,cx		; if (CurrentCo eq RootCo) skip the save
	jne	must_save
	cmp	di,si
	je	skip_save
must_save:

	mov	cx,co_size[si]		; get size from CurrentCo
	les	di,co_base[si]		; and stack base into es:di
	mov	ds,dx			; restore data segment again
	lds	si,_coroutine_buf	; si is now the coroutine buffer

	add	di,cx			; es:di := top of stack buffer
	add	si,cx			; ds:si := top of current stack
	mov	cx,si			; cx	:= offset within current stack
	sub	cx,sp			; cx    := amount of stack used
	sub	di,cx			; es:di := target stack pointer
	sub	si,cx			; ds:si := current stack pointer

	rep	movsb			; this does the copy

skip_save:
	pop	ax
	pop	dx
	pop	cx
	pop	si
	pop	di
	pop	es
	pop	ds
	ret
save_stack ENDP

;==============================================================================
; Result = CallCo( coroutine,arg)
; int *coroutine
; long arg
;
; Starts up a coroutine that was just created or did a WaitCo to return an arg.
;==============================================================================
_CallCo PROC FAR
        push    bp
        mov     bp,sp
        push    ds
	push	di
	push	si
 
        les     di,arg1[bp]			; new coroutine
        lds     si,DWORD PTR _CurrentCo	; current coroutine
                                       
        mov     es:co_parent[di],si
        mov     es:co_parent[di+2],ds   ; adopt the coroutine

        mov     ax,arg3[bp]               ; argument
        mov     dx,arg4[bp]
 
		; At this point we assume
		; si = parent coroutine
		; es:di = daughter coroutine
		; dx:ax = argument
coenter:
	push	ax			; check for coroutine stack overflow
	mov	ax,co_magic[si]	; on current and new coroutine
	cmp	ax,magic_value
	jne	coroutines_crashed
	mov	ax,es:co_magic[di]
	cmp	ax,magic_value
	jne	coroutines_crashed
	pop	ax

	call	FAR PTR save_stack

        mov     co_sp[si],sp		; now save the stack pointer
        mov     co_sp[si+2],ss

	cli				; critical section
        mov     sp,es:co_sp[di]         ; switch to new daughter stack
        mov     ss,es:co_sp[di+2]
	sti
		; N.B. we are now running in the WaitCo() stackframe,
		; not in CallCo() !!!!
		;
		; For Windows coroutines things are somewhat confused
		; now. The stack has been switched, probably into the
		; coroutine buffer. Interrupts can be handled safely.
		; However, the contents of the stack has not yet been
		; copied into the buffer. I do not believe this to be
		; a problem until I re-enter the user code.

	push	dx			; restore default data segment
	mov	dx,DGROUP
	mov	ds,dx
	pop	dx

        mov     _CurrentCo,di	; update current coroutine
        mov     _CurrentCo+2,es
 
		; unless (CurrentCo eq RootCo), it is now necessary to
		; copy the whole stack into coroutine_buf.
		; N.B. dx:ax must be preserved without using the stack, as
		; the whole stack is about to be overwritten
	les	di, DWORD PTR RootCo
	lds	si, DWORD PTR _CurrentCo
	mov	bx, ds
	mov	cx, es
	cmp	bx,cx
	jne	must_restore
	cmp	di,si
	je	skip_restore

must_restore:
	mov	cx,co_size[si]	; get size from CurrentCo
	les	di,co_base[si]	; and stack bas into es:di
	mov	bx,DGROUP		; restore data segment again
	mov	ds,bx
	lds	si,_coroutine_buf	; si is now the coroutine buffer

	push	es			; source and dest are the wrong
	push	ds			; way around, whoops
	pop	es
	pop	ds
	push	si
	push	di
	pop	si
	pop	di

	add	di,cx			; es:di := top of current stack
	add	si,cx			; ds:si := top of stack buffer
	mov	cx,di			; cx	:= offset of current stack
	sub	cx,sp			; cx    := amount of stack used
	sub	di,cx			; es:di := current stack pointer
	sub	si,cx			; ds:si := stack buffer

	rep	movsb			; this does the copy
skip_restore:

	pop	si
	pop	di
        pop     ds
        pop     bp			; restore registers of daughter
	ret				; enter daughter
_CallCo ENDP
 
 
;==============================================================================
; coroutines_crashed
;
; a magic number is maintained in the coroutine structure, which will get
; zapped if the coroutine stack overflows. If that happens the coroutine
; library will jump here where I try to exit.
;==============================================================================

coroutines_crashed:
	mov	dx,seg cowarn
	mov	ds,dx
	mov	dx,offset cowarn
	mov	ah,9
	int	21H
	mov	dx,seg cowarn2
	mov	ds,dx
	mov	dx,offset cowarn2
	mov	ah,9
	int	21h

	mov	ah,04ch
	mov	al,100
	int	21h

cowarn	db	'*** Internal error : coroutine stack overflow detected.',0dh, 0ah, '$'
cowarn2 db	'***                : attempting to exit safely.',0dh, 0ah, '$'

;==============================================================================
; Arg = WaitCo( arg)
; long arg;
;
; Returns control back to the parent with required argument/return code in ax.
; Arg will eventually be returned when the coroutine doing the WaitCo is called
; again with CallCo_(coroutine,ARG)
;==============================================================================
_WaitCo PROC FAR
        push    bp
        mov     bp,sp
        push    ds
	push	di
	push	si
 
	lds     si,DWORD PTR _CurrentCo ; si = current coroutine
        les     di,co_parent[si]        ; es:di = parent coroutine
 
        mov     ax,arg1[bp]             ;
        mov     dx,arg2[bp]             ; ax/bx = argument
 
        jmp     coenter
_WaitCo ENDP

;==============================================================================
; success = DeleteCo_( coroutine )
; int *coroutine
;
; Deletes the stack area being used by a coroutine that is no longer needed.
;==============================================================================
_DeleteCo PROC FAR
	pushall
 
        mov     bx,arg1[bp]
        mov     cx,arg2[bp]		; cx:bx is now the coroutine

        test    bx,-1                   ; coroutine passed = NULL ptr ?
        jne     valptr
        test    cx,-1                   ; coroutine passed = NULL ptr ?
        je      deletefail              ; YES -> failed  (invalid arg)
valptr:
	mov	es,cx
	mov	di,bx
	mov	ax,es:co_magic[di]
	cmp	ax,magic_value
	je	deleteok
	jmp	coroutines_crashed

deleteok:
	xor	ax,ax
	mov	es:co_magic[di],ax	; prevent coroutines from being deleted twice

        push    cx
        push    bx                      ; coroutine to delete
        call    _free_mem
        add     sp,4
 
deletefail:
	popall
        ret
 
_DeleteCo ENDP
 
 
;=============================================================================
; Success = InitCo_()
;
; Initialises a root co-routine that never goes away. It corresponds directly
; to the main level of the program and is really just a list header for all
; the other co-routines that get started.
;=============================================================================
_InitCo PROC FAR
        push    es
	push	di

        mov     cx,co_SIZEOF
        push    cx
        call    _get_mem		; get required amount of stack
        add     sp,2
        test    ax,-1			; NULL ptr ?
        jne     gotok
        test    dx,-1			; NULL ptr ?
        je      initfail		; YES -> failed
 
gotok:
        mov     _CurrentCo,ax
        mov     _CurrentCo+2,dx	; current coroutine
	mov	RootCo,ax
	mov	RootCo+2,dx
 
	les	di,DWORD PTR _CurrentCo	; es:di = current coroutine
                                        
        mov     es:co_parent[di],ax
        mov     es:co_parent+2[di],dx      ; I'm my own parent
	mov	ax,magic_value
	mov	es:co_magic[di],ax
 					; stackbase is not required
					; stack pointer etc. will be set up
					; when switching
        mov     ax,-1
        mov     dx,-1                   ; return success

initfail:
 	pop	di
        pop     es
        ret
 
_InitCo ENDP
 

;*---------------------------------------------------------------------------*;
;--                                                                         --;
;-- MODULE NAME: link.asm                                                   --;
;--                                                                         --;
;-- AUTHOR : B. Veer (based on original code by C. Grimsdale)		    --;
;-- DATE : 10/4/88                                                          --;
;-- UPDATES: 15/7/91 A. Schuilenburg (Handle DOS Devs)                      --;
;                    TMB16 board excluded in word mode                      --;
;--									    --;
;-----------------------------------------------------------------------------;

;
; All the public data. First the routines that are the same for all
; transputer hardware. 
        PUBLIC _fetch_block, _send_block, _byte_to_link
	PUBLIC _byte_from_link, _rdrdy, _wrrdy
; Next routines specific to particular bits of hardware, B004 and Meiko boards
        PUBLIC _b004_reset, _b004_analyse, _b004_init_link	
	PUBLIC _mk026_reset, _mk026_analyse, _mk026_init_link
	PUBLIC _dma_init, _dma_send, _dma_fetch
; Routines specific to DOS
        PUBLIC _dos_fetch_block, _dos_send_block, _dos_byte_to_link
	PUBLIC _dos_byte_from_link, _dos_rdrdy, _dos_wrrdy
	PUBLIC _dos_reset, _dos_init_link, _dos_close_link
	PUBLIC _tmb16_mode

; TMS320C40 HARDWARE
;
; All the public data. First the routines that are the same for all
; TMS320C40 hardware. 
	PUBLIC _tim40_fetch_block, _tim40_send_block, _tim40_byte_to_link
	PUBLIC _tim40_byte_from_link, _tim40_rdrdy, _tim40_wrrdy

; Next routines specific to particular bits of hardware: TIM-40 boards
	PUBLIC _tim40_reset, _tim40_init_link

; Hunt board also has hi-speed fifo interface for large transfers that can
; be used at the same time as the simple TIM-40 interface.
	PUBLIC _hunt_fetch_block, _hunt_send_block

; Transtec TDB416 board has an 8bit TIM-40 std interface and a 16 bit
; high speed interface. The high speed interface is simply a 16bit
; version of the TIM-40 standard 8 bit interface and can be used
; interchangebly with it.

	PUBLIC _tdb_wrrdy, _tdb_rdrdy, _tdb_fetch_block, _tdb_send_block

; @@@ add dos device driver functions

DOSTimeout		EQU	2		; timeout flag only

reset_mask	EQU	1	; Mk026, reset = bit 0
analyse_mask	EQU	2	; Mk026 again
 
;  number of iterations for link timeout
D500Msec	EQU    6500 
 
 
Bit0               EQU 1
Bit1               EQU 2
Bit2               EQU 4
Bit3               EQU 8
Bit7               EQU 0080H

/* The following allows us to cope with TIM-40 boards requiring Bit7 to go */
/* Hi-Lo (Hunt) and boards that want bit 0 to go Lo-Hi (Transtec) */
TIM40_ASRT_RESET   EQU 00fH	; assert reset
TIM40_RELS_RESET   EQU 0f0H	; release reset

 
SUCCESS            EQU 0
FAIL               EQU 1


;==============================================================================
; success = rdrdy ()
; int success;
;
; Check link for data, If byte ready return TRUE else return FALSE.
;
;==============================================================================
; Note, this macro preserves nothing, if you wish to keep, then save it
GetDOSstatus	MACRO
	mov	ax, 4402h		;status function
	mov	bx, _dos_handle	;handle to device driver
	mov	cx, 4			;size of data
	mov	dx, offset dgroup:_dos_status
	int	21h
	mov	ax, _dos_status	;macro gives result in ax
ENDM

_rdrdy    PROC FAR
	mov     dx,_link_in_status   ; input status register
        in      al, dx                  ; Read status
        and     al, Bit0                ; Test bit zero
        ret
_rdrdy	    ENDP 		 

_dos_rdrdy    PROC FAR
	push	ds
	GetDOSstatus			; status in AX
	pop	ds
	and	al, Bit3		; ready to read ?
        ret
_dos_rdrdy	    ENDP 		 


;==============================================================================
; success = wrrdy ()
; int success;
;
; Check link for output status, If ready to Tx return TRUE else return FALSE.
;
;==============================================================================
_wrrdy    PROC FAR
        mov     dx,_link_out_status ; output status register
        in      al,dx                  ; Read status
        and     al,Bit0                ; Test bit zero
        ret
_wrrdy	    ENDP 		 

_dos_wrrdy    PROC FAR
	push	ds
	GetDOSstatus			; status in AX
	pop	ds
	and	al, Bit2		; ready to write ?
        ret
_dos_wrrdy	    ENDP 		 

 
;==============================================================================
; success = byte_to_link (x)
; int success;
; int x;  lower 8 bits output
;
; Output the given byte down the link. Return SUCCESS if the byte is output,
; else FAIL if the link adaptor is not ready after 500 Msec.
;
;==============================================================================
_byte_to_link   PROC FAR
        push    bp                      ; C stack manipulation
        mov     bp, sp

	mov	bx,0100H
l21:
        mov     cx,D500Msec
l1:
        mov     dx, _link_out_status	; inner loop, 65000 iterations
        in      al, dx
        and     al, Bit0
        jnz     l2
        loop    l1

	dec	bx		; outer loop, 256 iterations
	cmp	bx,0
	jne	l21

        mov     ax,FAIL
        jmp     short l3
l2:
        mov     ax,arg1[bp]
        mov     dx,_link_write
        out     dx,al
        mov     ax,SUCCESS
l3:
        pop     bp
        ret
 
_byte_to_link   ENDP

_dos_byte_to_link   PROC FAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
	push	ds

	mov	bx,0100H
dl21:
        mov     cx,D500Msec
dl1:
	push	bx		; keep these two
	push	cx		;	"
	GetDOSstatus		; status in AX
	pop	cx		; and restore
	pop	bx		;	"
	and	al, Bit2	; ready to write ?
        jnz     dl2
        loop    dl1

	dec	bx		; outer loop, 256 iterations
	cmp	bx,0
	jne	dl21

db2lerror:
        mov     ax,FAIL
        jmp     short dl3
dl2:
	mov	bx, _dos_handle	; handle to device driver
	push	ss
	pop	ds
	lea	dx, arg1[bp]		; address of data
	mov	cx, 1			; only 1 byte
	mov	ah, 40h			; write data
	int 	21h
	jc	db2lerror		; carry set on error
	mov	ax,SUCCESS
dl3:
	pop	ds
        pop     bp
        ret

_dos_byte_to_link   ENDP


;==============================================================================
; success = byte_from_link (x)
; int success;
; char *x;
;
; Read a single byte from the link. Returns SUCCESS, or FAIL if the link
; timedout after 500Msec.
;
;==============================================================================
_byte_from_link   PROC FAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
        push    si
        push    ds

	mov	bx,0100H
l20:
        mov     cx,D500Msec
l4:
        mov     dx,_link_in_status	; inner loop, 65000 iterations
        in      al,dx
        and     al,Bit0
        jnz     l5
        loop    l4

	dec	bx			; outer loop, 256 iterations
    	cmp	bx,0
	jne	l20

        mov     ax,FAIL
        jmp     short l6
l5:
        mov     dx,_link_read
        in      al,dx
        lds     si,[bp+arg1]
        mov     [si],al
        mov     ax,SUCCESS
l6:                                       ; read byte within 500 Msec.
        pop     ds 
        pop     si
        pop     bp
        ret
 
_byte_from_link   ENDP

_dos_byte_from_link   PROC FAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
        push    si
        push    ds

	mov	bx,0100H
dl20:
        mov     cx,D500Msec
dl4:
	push 	bx			; save
	push	cx			;  "
	GetDOSstatus
	pop 	cx			; restore
	pop	bx			;    "
	and	al, Bit3		; Anything to read ?
        jnz     dl5
        loop    dl4

	dec	bx			; outer loop, 256 iterations
    	cmp	bx,0
	jne	dl20

db4lerror:
        mov     ax,FAIL
        jmp     short dl6
dl5:
	mov	bx, _dos_handle	; handle to device driver
	lds	dx,[bp+arg1]		; dx is address of data
	mov	cx, 1			; only 1 byte
	mov	ah, 3Fh			; read data
	int 	21h
	jc	db4lerror		; carry set on error
	mov	ax,SUCCESS

dl6:                                       ; read byte within 500 Msec.
        pop     ds 
        pop     si
        pop     bp
        ret

_dos_byte_from_link   ENDP

 
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
; N.B ASSUME long pointers
;==============================================================================
_send_block PROC FAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
        push    si
        push    ds

        mov     bx,arg4[bp]
l22:
	mov	cx,D500Msec
l7:
        mov     dx,_link_out_status  ; wait "timeout" loops for linkout ready
        in      al,dx
        and     al,Bit0
        jnz     l8
        loop    l7

	dec	bx
	cmp	bx,0
	jne	l22

        mov     ax,arg1[bp]             ; amount of data failed to write
        jmp     short l12notime

l8:
	mov	bx,arg4[bp]		; get timeout
	push	bx			; save for later

        xchg    ah,dl                   ; store lower byte of link_out_stat in ah
        mov     dx,_link_write       ; I can now xchg ah,dl to switch from
                                        ; status port to write port
        mov     cx,arg1[bp]             ; amount of data to write
        lds     si,arg2[bp]             ; and the buffer
l9:
        mov     bx,cx                   ; store count remaining
        xchg    ah,dl                   ; check link out status
l101:
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
	jmp	short l12

l11:
        loop    l10
	pop	cx			; recall timeout
	dec	cx
	push	cx			; keep for later
	cmp	cx,0			; timed out?
	jnz	l101			; nope
        mov     ax,bx                   ; return count remaining
l12:
	pop	cx			; restore stack, kicking off timeout 
l12notime:
        pop     ds
        pop     si
        pop     bp
 
        ret
 
_send_block   ENDP

_dos_send_block PROC FAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
        push    si
        push    ds

        mov     bx,arg4[bp]	; wait "timeout" loops for linkout ready
dl22:
	mov	cx,D500Msec	
dl7:
	push	bx		; keep these two
	push	cx		;	"
	GetDOSstatus		; status in AX
	pop	cx		; and restore
	pop	bx		;	"
	and	al, Bit2	; ready to write ?
        jnz     dl8
        loop    dl7

	dec	bx
	cmp	bx,0
	jne	dl22

dlsfail:
        mov     ax,arg1[bp]             ; amount of data failed to write
        jmp     short dl10

dl8:
	mov	bx, _dos_handle	; handle to device driver
        mov     cx, arg1[bp]            ; amount of data to write
	lds     dx, arg2[bp]            ; and dx is the buffer address
	mov	ah, 40h			; write data
	int 	21h
	jc	dlsfail			; carry set on failure
	mov	ax,SUCCESS
dl10:
        pop     ds
        pop     si
        pop     bp
        ret

_dos_send_block   ENDP


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
_fetch_block PROC FAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
	push	di
        push    es

        mov     bx,arg4[bp]
l23:
	mov	cx,D500Msec
l13:
        mov     dx,_link_in_status   ; wait "timeout" loops for linkout ready
        in      al,dx
        and     al,Bit0
        jnz     l14
        loop    l13

	dec	bx
	cmp	bx,0
	jne	l23

        mov     ax,arg1[bp]             ; amount of data failed to write
        jmp     short l18
l14:
        xchg    ah,dl                   ; store lower byte of link_out_stat in ah
        mov     dx,_link_read        ; I can now xchg ah,dl to switch from
                                        ; status port to write port
        mov     cx,arg1[bp]             ; amount of data to write
        les     di,arg2[bp]             ; and the buffer
l15:
        mov     bx,cx                   ; store count remaining
        xchg    ah,dl                   ; check link out status
        mov     cx,D500Msec
l16:
        in      al,dx
        and     al,Bit0
        jz      l17
        xchg    ah,dl                   ; switch to link_write
        in      al,dx                   ; get byte
	stosb
        mov     cx,bx                   ; restore count remaining
	loop    l15                     ; and loop for next byte
        mov     ax,SUCCESS              ; finished
	jmp	short l18

l17:
        loop    l16
        mov     ax,bx                   ; return count remaining
l18:
        pop     es
        pop     di
        pop     bp
 
        ret
  
_fetch_block   ENDP

_dos_fetch_block PROC FAR
        push    bp                      ; C stack manipulation
        mov     bp, sp
	push	di
        push    ds

        mov     bx,arg4[bp]
dl23:
	mov	cx,D500Msec
dl13:
	push	bx
	push	cx
	GetDOSstatus			; ready to receive ?
	pop	cx
	pop	bx
        and     al,Bit3
        jnz     dl14
        loop    dl13

	dec	bx
	cmp	bx,0
	jne	dl23

dlffail:
        mov     ax,arg1[bp]             ; amount of data failed to read
        jmp     short dl16
dl14:
	mov	bx, _dos_handle	; handle to device driver
        mov     cx,arg1[bp]             ; amount of data to read
	lds     dx,arg2[bp]             ; and the buffer address
	mov	ah, 3Fh			; read data
	int 	21h
	jc	dlffail			; carry set on failure
	mov	ax,SUCCESS
dl16:
        pop     ds
        pop     di
        pop     bp
        ret

_dos_fetch_block   ENDP

 
;=============================================================================
; void dos_reset ()
;
; Reset root Transputer through dos device
;
;=============================================================================
_dos_reset   PROC FAR
	push	ds			; keep ds
	mov	ax, _dos_handle	; get current handle
	cmp	ax, 0			; is it set ? 
	jnz	dos_open
	pop	ds			; oops, not open, failed
	ret	

dos_open :
	; put into binary mode
	mov	dx, 0020h
	mov	bx, _dos_handle
	mov	ax, 4401h
	int	21h

	; reset Transputer
	mov	bx, _dos_handle
	mov	cx, 4			; 0 bytes
	mov	dx, offset dgroup:_dos_dev_word
	mov	ax, 0			; load reset instruction
	mov	[_dos_dev_word + 0], ax	;	"
	mov	ax, 0			;	"
	mov	[_dos_dev_word + 2], ax	;	"
	mov	ax, 4403h		; dos fn 44-03	(device ctrl write)
	int 	21h

	pop 	ds			; Restore ds
	ret

_dos_reset   ENDP

_tmb16_mode	PROC FAR
	push	bp			; C stack manipulation
	mov	bp, sp
	push	ds

	mov	bx, _dos_handle
	mov	cx, 4			; 0 bytes
	mov	dx, offset dgroup:_dos_dev_word
	mov	ax, arg1[bp]		; load mode data	(low byte 1st)
	mov	[_dos_dev_word + 0], ax	;	"
	mov	ax, arg2[bp]		;	" (high byte)
	mov	[_dos_dev_word + 2], ax	;	"
	mov	ax, 4403h		; dos fn 44-03	(device ctrl write)
	int 	21h

	pop	ds
	pop	bp
	ret
_tmb16_mode	ENDP

_dos_init_link	PROC FAR
	push	bp			; C stack manipulation
	mov	bp, sp
	push	si
	push	ds			; keep ds
	mov	ax, _dos_handle	; get current handle
	cmp	ax, 0			; is it set ? 
	jnz	dos_link_exit			; already open

	mov	dx, arg1[bp]	; Get ptr to name
	mov	ds, arg2[bp]	;	"	
	mov	al, 2			; open for i/o
	mov	ah, 3Dh			; open file
	int 	21h
	jc	no_dev
	mov	_dos_handle, ax	; store handle
	
	mov	bx, ax			; ensure not a file
	mov	ax, 4400h		; IOCTL call, fn 0
	int	21h
	test	dl, 80h
	jz	no_dev

	pop	ds
	push	ds
	call	_dos_reset		; reset the transputer as well
	mov	ax, SUCCESS
	jmp	short dos_link_exit

no_dev :
	mov	ax, FAIL

dos_link_exit :	
	pop	ds
	pop	si
	pop	bp
	ret
	
_dos_init_link	ENDP

_dos_close_link	PROC FAR
	mov	bx, _dos_handle	; get current handle
	cmp	bx, 0			; is it set ? 
	jz	dos_close_exit		; not open

	mov	ax, 3E00h			; close file
	int	21h
	mov	_dos_handle, 0	; reset handle

dos_close_exit:
	ret

_dos_close_link	ENDP

;=============================================================================
; void b004_reset ()
;
; Reset root Transputer on a B004 board
;
;=============================================================================
_b004_reset   PROC FAR
 
        mov     dx, _link_reset         ; address of reset reg.
        mov     al, 1                   ; assert reset
        out     dx,al
        mov     cx, _reset_timeout        ; wait ? Msec
waitr:
        loop    waitr
        xor     al, 1                   ; release reset
        out     dx,al

	mov	cx,_reset_timeout	
wait10:
	loop	wait10

        ret
 
_b004_reset   ENDP
 
;=============================================================================
; void b004_analyse ()
;
; Analyse root Transputer
;
;=============================================================================
_b004_analyse   PROC FAR
        mov     dx, _link_analyse       ; address of analyse register
        mov     al, 1                   ; assert analyse
        out     dx,al
        mov     cx, _analyse_timeout    ; wait ? Msec
waita:
        loop    waita
 
        mov     dx, _link_reset         ; assert reset
        out     dx,al
        mov     cx, _reset_timeout      ; wait ? Msec
waitrs:
        loop    waitrs
 
        xor     al,1                    ; release reset
        out     dx,al

	mov	cx,_reset_timeout
waitmore:
	loop	waitmore

        mov     dx, _link_analyse
        out     dx,al

	mov	cx,_reset_timeout
wait11:
	loop	wait11
        ret
_b004_analyse ENDP
 
;=============================================================================
; void b004_init_link()
;
; This forces the analyse line down, instead of leaving it floating
;
;=============================================================================
_b004_init_link	PROC	FAR
	mov	dx, _link_analyse
	xor	ax,ax
	out	dx, al
	mov	dx,_link_in_status	; disable link interrupts
	out	dx,al
	ret
_b004_init_link	ENDP
	
;=============================================================================
; void mk026_reset ()
;
; Reset root Transputer on a Meiko board
;
;=============================================================================
_mk026_reset   PROC FAR
 
        mov     dx, _control_write      ; address of control reg.
        mov     al, control_mask        ; current control mask
        or      al, reset_mask          ; assert reset
        out     dx,al
        mov     cx, _reset_timeout      ; wait ? Msec
mkwaitr:
        loop    mkwaitr
        xor     al,reset_mask           ; release reset
        out     dx,al
        mov     control_mask,al
 
        mov	cx,D500MSec             ; the Meiko surface has to drain
mkwait2:                                ; the National Grid before it kicks
        loop    mkwait2                 ; off
        ret
 
_mk026_reset   ENDP
 
 
;=============================================================================
; void mk026_analyse ()
;
; Analyse root Transputer on a Meiko board
;
;=============================================================================
_mk026_analyse   PROC FAR
 
        mov     dx, _control_write      ; address of control reg.
        mov     al, control_mask        ; current control mask
        or      al, analyse_mask        ; assert analyse
        out     dx,al
        mov     cx, _analyse_timeout    ; wait ? Msec
mkwaita:
        loop    mkwaita
 
        or      al, reset_mask          ; assert reset
        out     dx,al
        mov     cx, _reset_timeout      ; wait ? Msec
mkwaitrs:
        loop    mkwaitrs
 
        xor     al,reset_mask           ; release reset
        out     dx,al
        xor     al,analyse_mask         ; release analyse
        out     dx,al
        mov     control_mask,al

        ret
_mk026_analyse ENDP
 
 
 
;=============================================================================
; void mk026_init_link ()
;
; Initialise the link Interface on a B004 board
;
;=============================================================================
_mk026_init_link PROC FAR
 
        mov     dx, _control_write      ; address of control reg.
        mov     al, 00H                 ; Disable DMA & soft/hard error int.
        mov     control_mask,al         ; initialise control mask
        out     dx,al                   ; output to control register

	mov	dx,_link_in_status	; disable link interrupts
	out	dx,al
 
        call    _mk026_reset            ; reset the root Transputer
 
        ret
 
_mk026_init_link ENDP

;==============================================================================
; dma_init()                       - disable DMA interrupts
; dma_send(count, page, offset)    - transmit data
; dma_receive(count, page, offset) - receive data
;
; These routines are based on the documentation in the B008 manual. All the
; really nasty address manipulation is done in pclocal.c, because it is far
; too unpleasant to implement in assembler
;==============================================================================

dma_mastctrl	EQU	0dH
dma_pagebase	EQU	080H
dma_base	EQU	0
dma_status	EQU	08H
dma_command	EQU	08H
dma_mode	EQU	0bH
dma_allmask	EQU	0fH

_dma_init	PROC FAR
	mov	dx,_int_enable	; disable all interrupts
	xor	ax,ax			; dma is handled by polling
	out	dx,al			; to avoid interrupts clashing
	ret
_dma_init	ENDP
 
_dma_send	PROC FAR
	push	bp
	mov	bp,sp
	
	xor	ax,ax
	out	dma_mastctrl,al	; reset the DMAC

	mov	ax,_dma_channel	; set up the DMA page
	mov	dx,082H
	cmp	ax,3			; channel 3 uses 082H
	je	send_gotpage
	dec	dx			; channel 2 uses 081H
	cmp	ax,2
	je	send_gotpage
	mov	dx,083H			; channels 0 and 1 use 083H	
send_gotpage:	
	mov	ax,arg2[bp]
	out	dx,al

	mov	dx,_dma_channel	; now for the transfer base
	add	dx,dx			; at offsets 0, 2, 4, and 6
	add	dx,dma_base
	mov	ax,arg3[bp]
	out	dx,al			; low byte
	mov	al,ah
	out	dx,al			; middle byte

	inc	dx			; count follows after base register
	mov	ax,arg1[bp]
	dec	ax
	out	dx,al			; low byte of count
	mov	al,ah
	out	dx,al

	mov	ax,_dma_channel	; dma mode
	add	ax,08H			; -> trannie
	out	dma_mode,al

	xor	al,al			; dma command is 0
	out	dma_command,al
	out	dma_allmask,al		; that finishes the DMAC
					; now for the board itself
 	mov	dx,_link_in_status
	mov	ax,2
	out	dx,al
	mov	dx,_link_out_status
	out	dx,al

	mov	dx,_dma_request
	xor	ax,ax
	out	dx,al			; force write

	mov	cx,_dma_channel	; prepare to poll for DMA completion
	mov	ax,1			; which bit to check...
	shl	ax,cl
	mov	ah,al
	mov	dx,dma_status		; in which register...

	mov	bx,10			; couple of seconds delay
send_outer:
	mov	cx,65000
send_inner:
	in	al,dx
	and	al,ah
	jne	dmasend_finished
	loop	send_inner
	dec	bx
	cmp	bx,0
	jne	send_outer

dmasend_finished:
	xor	ax,ax		; reset DMA
	out	dma_mastctrl,al
	mov	dx,_link_in_status	; reset C012
	out	dx,al
	mov	dx,_link_out_status
	out	dx,al

	mov	ax,bx		; contains success/failure
	pop	bp			
	ret
_dma_send ENDP

_dma_fetch	PROC FAR
	push	bp
	mov	bp,sp
	
	xor	ax,ax
	out	dma_mastctrl,al	; reset the DMAC

	mov	ax,_dma_channel	; set up the DMA page
	mov	dx,082H
	cmp	ax,3			; channel 3 uses 082H
	je	fetch_gotpage
	dec	dx			; channel 2 uses 081H
	cmp	ax,2
	je	fetch_gotpage
	mov	dx,083H			; channels 0 and 1 use 083H	
fetch_gotpage:	
	mov	ax,arg2[bp]
	out	dx,al

	mov	dx,_dma_channel	; now for the transfer base
	add	dx,dx			; at offsets 0, 2, 4, and 6
	add	dx,dma_base
	mov	ax,arg3[bp]
	out	dx,al			; low byte
	mov	al,ah
	out	dx,al			; middle byte

	inc	dx			; count follows after base register
	mov	ax,arg1[bp]
	dec	ax
	out	dx,al			; low byte of count
	mov	al,ah
	out	dx,al

	mov	ax,_dma_channel	; dma mode
	add	ax,04H			; <- trannie
	out	dma_mode,al

	xor	al,al			; dma command is 0
	out	dma_command,al
	out	dma_allmask,al		; that finishes the DMAC
					; now for the board itself
 	mov	dx,_link_in_status
	mov	ax,2
	out	dx,al
	mov	dx,_link_out_status
	out	dx,al

	mov	dx,_dma_request
	mov	ax,1
	out	dx,al			; force read

	mov	cx,_dma_channel	; prepare to poll for DMA completion
	mov	ax,1			; which bit to check...
	shl	ax,cl
	mov	ah,al
	mov	dx,dma_status		; in which register...

	mov	bx,10			; couple of seconds delay
fetch_outer:
	mov	cx,65000
fetch_inner:
	in	al,dx
	and	al,ah
	jne	dmafetch_finished
	loop	fetch_inner
	dec	bx
	cmp	bx,0
	jne	fetch_outer
dmafetch_finished:
	xor	ax,ax			; reset DMA
	out	dma_mastctrl,al
	mov	dx,_link_in_status	; reset C012
	out	dx,al
	mov	dx,_link_out_status
	out	dx,al

	mov	ax,bx
	pop	bp			
	ret
_dma_fetch ENDP

;==============================================================================
; TMS320C40 specfic link functions

;==============================================================================
; TIM-40 standard link interface support

;==============================================================================
; success = rdrdy ()
; int success;
;
; Check link for data, If byte ready return TRUE else return FALSE.
;
;==============================================================================
_tim40_rdrdy    PROC FAR
	mov     dx,_link_in_status   ; input status register
	in      al, dx                  ; Read status
	and     al, Bit7                ; Test bit seven
	ret
_tim40_rdrdy	    ENDP 		 


;==============================================================================
; success = wrrdy ()
; int success;
;
; Check link for output status, If ready to Tx return TRUE else return FALSE.
;
;==============================================================================
_tim40_wrrdy    PROC FAR
	mov     dx,_link_out_status ; output status register
	in      al,dx                  ; Read status
	and     al,Bit7                ; Test bit seven
	ret
_tim40_wrrdy	    ENDP 		 


;==============================================================================
; success = byte_from_link (x)
; int success;
; char *x;
;
; Read a single byte from the link. Returns SUCCESS, or FAIL if the link
; timedout after 500Msec.
;
;==============================================================================
_tim40_byte_from_link   PROC FAR
	push    bp                      ; C stack manipulation
	mov     bp, sp
	push    si
	push    ds

	mov	bx,0100H
t40l20:
	mov     cx,D500Msec
t40l4:
	mov     dx,_link_in_status	; inner loop, 65000 iterations
	in      al,dx
	and     al,Bit7
	jnz     t40l5
	loop    t40l4

	dec	bx			; outer loop, 256 iterations
    	cmp	bx,0
	jne	t40l20

	mov     ax,FAIL
	jmp     short t40l6
t40l5:
	mov     dx,_link_read
	in      al,dx
	lds     si,[bp+arg1]
	mov     ds:[si],al
	mov     ax,SUCCESS
t40l6:                                       ; read byte within 500 Msec.
	pop     ds 
	pop     si
	pop     bp
	ret
 
_tim40_byte_from_link   ENDP

 
;==============================================================================
; success = byte_to_link (x)
; int success;
; int x;  lower 8 bits output
;
; Output the given byte down the link. Return SUCCESS if the byte is output,
; else FAIL if the link adaptor is not ready after 500 Msec.
;
;==============================================================================
_tim40_byte_to_link   PROC FAR
	push    bp                      ; C stack manipulation
	mov     bp, sp

	mov	bx,0100H
t40l21:
	mov     cx,D500Msec
t40l1:
	mov     dx, _link_out_status	; inner loop, 65000 iterations
	in      al, dx
	and     al, Bit7
	jnz     t40l2
	loop    t40l1

       	dec	bx			; outer loop, 256 iterations
	cmp	bx,0
	jne	t40l21

	mov     ax,FAIL
	jmp     short t40l3
t40l2:
	mov     ax,arg1[bp]
	mov     dx,_link_write
	out     dx,al
	mov     ax,SUCCESS
t40l3:
	pop     bp
	ret
 
_tim40_byte_to_link   ENDP

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
_tim40_fetch_block PROC FAR
	push    bp                      ; C stack manipulation
	mov     bp, sp
	push	di
	push    es

	mov     bx,arg4[bp]
t40l23:
	mov	cx,D500Msec
t40l13:
	mov     dx,_link_in_status   ; wait "timeout" loops for linkout ready
	in      al,dx
	and     al,Bit7
	jnz     t40l14
	loop    t40l13

	dec	bx
	cmp	bx,0
	jne	t40l23

	mov     ax,arg1[bp]             ; amount of data failed to write
	jmp     short t40l18
t40l14:
	mov     cx,arg1[bp]             ; amount of data to write
	les     di,arg2[bp]             ; and the buffer
	xchg    ah,dl                   ; store lower byte of link_out_stat in ah
	mov     dx,_link_read           ; I can now xchg ah,dl to switch from
	                                ; status port to write port
t40l15:
	mov     bx,cx                   ; store count remaining
	xchg    ah,dl                   ; check link out status
	mov     cx,D500Msec
t40l16:
	in      al,dx
	and     al,Bit7
	jz      t40l17
	xchg    ah,dl                   ; switch to link_write
	in      al,dx                   ; get byte
	stosb
	mov     cx,bx                   ; restore count remaining
	loop    t40l15                     ; and loop for next byte
	mov     ax,SUCCESS              ; finished
	jmp	short t40l18

t40l17:
	loop    t40l16
	mov     ax,bx                   ; return count remaining
t40l18:
	pop     es
	pop     di
	pop     bp
 
	ret
  
_tim40_fetch_block   ENDP

 
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
; N.B ASSUME long pointers
;==============================================================================
_tim40_send_block PROC FAR
	push    bp                      ; C stack manipulation
	mov     bp, sp
	push    si
	push    ds

	mov     bx,arg4[bp]
t40l22:
	mov	cx,D500Msec
t40l7:
	mov     dx,_link_out_status  ; wait "timeout" loops for linkout ready
	in      al,dx
	and     al,Bit7
	jnz     t40l8
	loop    t40l7

	dec	bx
	cmp	bx,0
	jne	t40l22

	mov     ax,arg1[bp]             ; amount of data failed to write
	jmp     short t40l12notime

t40l8:
	mov	bx,arg4[bp]		; get timeout
	push	bx			; save for later

	xchg    ah,dl                   ; store lower byte of link_out_stat in ah
	mov     dx,_link_write          ; I can now xchg ah,dl to switch from
	                                ; status port to write port
	mov     cx,arg1[bp]             ; amount of data to write
	lds     si,arg2[bp]             ; and the buffer
t40l9:
	mov     bx,cx                   ; store count remaining
	xchg    ah,dl                   ; check link out status
t40l101:
	mov     cx,D500Msec
t40l10:
	in      al,dx
	and     al,Bit7
	jz      t40l11
	xchg    ah,dl                   ; switch to link_write
	lodsb				; get next byte
	out     dx,al                   ; and output it
	mov     cx,bx                   ; restore count remaining
	loop    t40l9                   ; and loop for next byte
	mov	ax,SUCCESS		; finished
	jmp	short t40l12

t40l11:
	loop    t40l10
	pop	cx			; recall timeout
	dec	cx
	push	cx			; keep for later
	cmp	cx,0			; timed out?
	jnz	t40l101			; nope
	mov     ax,bx                   ; return count remaining
t40l12:
	pop	cx			; restore stack, kicking off timeout 
t40l12notime:
	pop     ds
	pop     si
	pop     bp
 
	ret
 
_tim40_send_block   ENDP

;=============================================================================
; void tim40_reset ()
;
; Reset root 'C40
;
;=============================================================================
_tim40_reset   PROC FAR
 
	mov     dx, _link_reset      ; address of reset reg.
/*	mov     al, 0	                ; assert reset */
	mov     al, TIM40_ASRT_RESET    ; assert reset
	out     dx,al
	mov     cx, _reset_timeout	; wait ? Msec
t40waitr:
	loop    t40waitr
/*	mov	al, Bit7                ; release reset */
	mov	al, TIM40_RELS_RESET    ; release reset
	out     dx,al

	mov	cx,_reset_timeout	
t40wait10:
	loop	t40wait10

	ret
 
_tim40_reset   ENDP
 
;=============================================================================
; void tim40_init_link()
;
; Presently does nothing
;
;=============================================================================
_tim40_init_link	PROC	FAR
	ret
_tim40_init_link	ENDP


;==============================================================================
; Hunt HEPC2 high performance interface support

;==============================================================================
; success = fetch_block (count, data, timeout)
; int success;
; far char *data;
; int timeout;
;
; Read the required no. of bytes into the given buffer. Return SUCCESS if the
; block is input without error, else FAIL if the link adaptor is not ready
; within timeout.
;
; N.B ASSUME long pointers
;==============================================================================
;
; The hunt board implements a 16k fifo. All reads from this area of memory
; read from the link. The IORDY line is used to throttle the transfer so you
; don't have to poll status bits. The benefit of this design is that the 8086
; REP MOVSB instructions can be used to move the data at bus speed. A status
; register simply notes if the entire transfer timed out or succeeded.
;
;
;
_hunt_fetch_block PROC FAR
	pushall				; save all regs and bp = args

	mov	cx,arg1[bp]             ; get amount of data to read
	les	di,arg2[bp]             ; get the buffer to read into
					; into ES:DI
	mov	bx,_hunt_fifo_size
	mov	dx,_hunt_timeout
	mov	si,_hunt_hiperf_seg
	mov	ds,si

	; ignore timeout argument - this is fixed in the hardware

hunt_reset_src:

	; the hiperf memory is only 16k in size, so separate
	; reads into up to 4 chunks
	cmp	cx, bx			; _hunt_fifo_size
	ja	hunt_rd_chunk

	xor	si,si			; zero offset for fifo
	rep	movsb			; move up to fifo_size (16k) of data

	xor	ax,ax			; check if transfer timed out
	in      al,dx			; (bit 7 = 1 == timed out)
	and     al,Bit7			; will return TRUE/FALSE indication

hunt_read_fail:
	popall				; restore all PCS caller saves regs
 
	ret

hunt_rd_chunk:
	mov	ax, bx			; temporary for _hunt_fifo_size
	sub	cx, ax			; cx reduced by fifo size
	xchg	ax, cx			; ax = remaining count of chars

	xor	si,si			; zero offset for fifo
	rep	movsb			; move fifo size (16k) of data

	xchg	ax, cx			; cx = remaining count of chars

	; @@@ the following could be removed and the timeout only
	; checked at the end of the complete transfer.
	xor	ax,ax			; check if transfer timed out
	in      al,dx			; (bit 7 = 1 == timed out)
	and     al,Bit7			; will return TRUE/FALSE indication

	jz	hunt_reset_src		; reset si to start of fifo and repeat

	xchg	ax, cx			; ret remaining count of chars to read
					; not accurate, so only used as an
					; indication
	jmp	short hunt_read_fail

_hunt_fetch_block   ENDP


;==============================================================================
; int success = send_block (count, data, timeout)
; int count
; far char *data;
; int timeout;
;
; Output the given data block down the link. Return SUCCESS if the block  is
; output, else FAIL if the link adaptor is not ready within timeout.
;
; N.B ASSUME long pointers
;==============================================================================
; See notes above about the Hunt Eng fifo.
; Writing to any address in the fifo sends that data down the link to a C40.
;

_hunt_send_block PROC FAR
	pushall				; save all regs and bp = args

	mov	cx,arg1[bp]             ; get amount of data to write
	mov	di,_hunt_hiperf_seg	; es:00 == fifo
	mov	es,di
	mov	bx,_hunt_fifo_size
	mov	dx,_hunt_timeout

	lds	si,arg2[bp]             ; get the buffer to read from
					; into DS:SI
	; ignore timeout argument - this is fixed in the hardware

hunt_reset_dst:

	; the hiperf memory is only 16k in size, so separate
	; writes into up to 4 chunks
	cmp	cx, bx
	ja	hunt_wr_chunk

	xor	di,di			; zero offset for fifo
	rep	movsb			; move up to fifo_size (16k) of data

	xor	ax,ax			; check if transfer timed out
	in      al,dx			; (bit 7 = 1 == timed out)
	and     al,Bit7			; will return TRUE/FALSE indication

hunt_write_fail:
	popall				; restore all PCS caller saves regs
 
	ret

hunt_wr_chunk:
	mov	ax,bx			; ax == temporary for fifo size
	sub	cx, ax			; cx reduced by fifo size
	xchg	ax, cx			; bx = remaining count of chars

	xor	di,di			; zero offset of fifo
	rep	movsb			; move fifo size (16k) of data

	xchg	ax, cx			; cx = remaining count of chars

	; @@@ the following could be removed and the timeout only
	; checked at the end of the complete transfer.
	xor	ax,ax			; check if transfer timed out
	in      al,dx			; (bit 7 = 1 == timed out)
	and     al,Bit7			; will return TRUE/FALSE indication

	jz	hunt_reset_dst		; reset di to start of fifo and repeat

	xchg	ax, cx			; ret remaining count of chars to read
					; not accurate, so only used as an
					; indication
	jmp	short hunt_write_fail
_hunt_send_block   ENDP


;==============================================================================
; Transtec TDB416 board (16bit interface) support

; Transtec TDB416 board has an 8bit TIM-40 std interface and a 16 bit
; high speed interface. The high speed interface is simply a 16bit
; version of the TIM-40 standard 8 bit interface and can be used
; interchangebly with it. The board has a design problem in that it only has
; 16 bit latches. This means that if the C40 asynchronously sends a word
; during a PC write, that it will grab the token send 2 bytes and then
; hang the interface until that word is read. The Helios half duplex protocol
; guarantees that only one word will be sent before the PC signals that it
; is ready to receive a full message. To get around this problem, if we ever
; timeout on a write, we immediately try to read a word of data and then
; attempt to continue writing. The read ready and read routines always
; check the buffer we stored the word away in before checking the link
; directly. Another problem associated with the board is that of non word
; (16 bit) reads. A read request may read a byte multiple, rather than a
; word multiple amount of data. In this case, the next byte must also be read
; and stored away as the board can only cope with word reads on word boundaries.
; The saved word then being provided to the next read. The only case where
; a non word aligned read will take place is for reading the padding bytes and
; then the next data block. All other reads will be 32 bit word multiples.

;==============================================================================
; success = rdrdy ()
; int success;
;
; Check link for data, If byte ready return TRUE else return FALSE.
;
;==============================================================================
_tdb_rdrdy    PROC FAR
	mov     ax,_tdb_wordsaved	; check if word is in buffer
	cmp	ax, 0			; if so return true
	jne	word_avail
					; otherwise, check the link
	mov     dx,_link_in_status   ; input (16bit) status register
	in      ax, dx                  ; Read status
	and     ax, Bit7                ; Test bit seven
word_avail:
	ret
_tdb_rdrdy	    ENDP 		 


;==============================================================================
; success = wrrdy ()
; int success;
;
; Check link for output status, If ready to Tx return TRUE else return FALSE.
;
;==============================================================================
_tdb_wrrdy    PROC FAR
	mov     dx,_link_out_status ; (16bit) output status register
	in      ax,dx                  ; Read status
	and     ax,Bit7                ; Test bit seven
	ret
_tdb_wrrdy	    ENDP 		 


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
_tdb_fetch_block PROC FAR
	push    bp                      ; C stack manipulation
	mov     bp, sp
	push	di
	push    es

	mov     bx,arg4[bp]		; get timeout

	; check if last read routine had to read non byte multiple
	; and therfore saved extra byte read into buffer
	mov	dx,_tdb_bytesaved
	cmp	dx, 0
	jz	tdb_checkword

	; pseudo read byte previously read from link
	mov     cx,arg1[bp]             ; amount of data to write
	sub	cx, 1
	les     di,arg2[bp]             ; the buffer to write

	mov	al,_tdb_bytebuffer	; get saved data
	stosb

	mov	ax, 0
	mov	_tdb_bytesaved, ax	; note empty byte buffer

	shr	cx,1			; convert to 16bit word count
	jnc	tdbl15			; ignore initial timeout - we have
					; already started reading data
	mov	ax, 1
	mov	_tdb_bytesaved, ax	; note we should fill byte buffer

	; no write can take place between non byte multiple reads
	; (only non word multiple case is reading padding bytes)
	jz	tdbl15			; ignore initial timeout - we have
					; already started reading data

tdb_checkword:
	; check if write routine had to read blocking word into buffer
	mov	dx,_tdb_wordsaved
	cmp	dx, 0
	jz	tdbl23

	; pseudo read 2 16bit words previously read from link to unblock
	; the last write
	mov     cx,arg1[bp]             ; amount of data to write
	shr	cx,1			; convert to 16bit word count

	jnc	not_byte_mult		; check for byte mult read request.
	mov	ax, 1
	mov	_tdb_bytesaved, ax	; note we should fill byte buffer

not_byte_mult:
	les     di,arg2[bp]             ; the buffer to write

	sub	cx,2
	mov	ax,_tdb_wordbuffer1	; get saved data
	stosw
	mov	ax,_tdb_wordbuffer2	; get saved data 2nd 16bit word
	stosw
	mov	ax, 0
	mov	_tdb_wordsaved, ax	; note empty word buffer

	cmp	cx, 0
	jz	readcomplete

	jmp	short tdbl15		; ignore initial timeout - we have
					; already started reading data

tdbl23:
	mov	cx,D500Msec
tdbl13:
	mov     dx,_link_in_status   ; wait "timeout" loops for linkout ready
	in      ax,dx
	and     ax,Bit7
	jnz     tdbl14
	loop    tdbl13

	dec	bx
	cmp	bx, 0
	jne	tdbl23

	mov     ax,arg1[bp]             ; amount of data failed to write
	jmp     short tdbl18

tdbl14:
	les     di,arg2[bp]             ; the buffer to write
	mov     cx,arg1[bp]             ; amount of data to write
	shr	cx,1			; convert to 16bit word count

	jnc	tdbl15			; check for byte mult read request.
	mov	ax, 1
	mov	_tdb_bytesaved, ax	; note we should fill byte buffer

	cmp	cx, 0			; if only one byte requested.
	jz	tryreadbytes		; just read it and dummy

tdbl15:
	mov     bx,cx                   ; store count remaining
	mov     cx,D500Msec
	mov     dx,_link_in_status	; check link in status
tdbl16:
	in      ax,dx
	and     ax,Bit7
	jz      tdbl17
	mov     dx,_link_read
	in      ax,dx                   ; get 16bit word 
	stosw
	mov     cx,bx                   ; restore count remaining
	loop    tdbl15                  ; and loop for next 16bit word
readcomplete:

	mov	dx,_tdb_bytesaved
	cmp	dx, 0
	jz	tdb_wordmultok

	; must read extra word for non word multiple data
tryreadbytes:
	mov     cx,D500Msec
	mov     dx,_link_in_status	; check link in status
	in      ax,dx
	and     ax,Bit7
	jz      nobyteread
	mov     dx,_link_read
	in      ax,dx                   ; get 16bit word 
	; one byte to data buffer, one byte to save buffer to be read later
	stosb				; lower order byte to buffer
	mov	_tdb_bytebuffer, ah	; higher order byte to save

tdb_wordmultok:
	mov     ax,SUCCESS              ; finished
	jmp	short tdbl18

nobyteread:
	loop    tryreadbytes
	mov     ax,1			; return count remaining
	jmp	short tdbl18			; fail

tdbl17:
	loop    tdbl16
	mov     ax,bx                   ; return count remaining
	shl	ax,1			; adjust from word to byte count
tdbl18:
	pop     es
	pop     di
	pop     bp
 
	ret
  
_tdb_fetch_block   ENDP

 
;==============================================================================
; success = send_block (count, data, timeout)
; int success;
; int count
; char *data;
; int timeout;
;
; *Warning*: This function assumes that transfer requests will always be for
; an even number of bytes.
;
; Output the given data block down the link. Return SUCCESS if the block  is
; output, else FAIL if the link adaptor is not ready within timeout.
;
; N.B ASSUME long pointers
;==============================================================================
_tdb_send_block PROC FAR
	push    bp                      ; C stack manipulation
	mov     bp, sp
	push    di
	push	es

	mov     bx,arg4[bp]
tdbl22:
	mov	cx,D500Msec
tdbl7:
	mov     dx,_link_out_status  ; wait "timeout" loops for linkout ready
	in      ax,dx
	and     ax,Bit7
	jnz     tdbl8
	loop    tdbl7

	dec	bx
	cmp	bx, 0
	jne	tdbl22

	mov     ax,arg1[bp]             ; amount of data failed to write
	jmp     short tdbl12notime

	; some data available
tdbl8:

	mov	bx,arg4[bp]		; get timeout
	push	bx			; save for later

	mov     cx,arg1[bp]             ; amount of data to write
	shr	cx,1			; convert to 16 bit word count
	les     di,arg2[bp]             ; and the buffer

tdbl9:
	mov     bx,cx                   ; store count remaining
tdbl101:
	mov     dx,_link_out_status	; check link tx status
	mov     cx,D500Msec
tdbl10:
	in      ax,dx			; 16 bit status reg
	and     ax,Bit7
	jz	tdbl11

	mov     dx,_link_write
	mov	ax,es:[di]		; get next 16 bit word
	add	di,2
	out     dx,ax                   ; and output it
	mov     cx,bx                   ; restore count remaining
	loop    tdbl9                   ; and loop for next word
	mov	ax,SUCCESS		; finished
	jmp	short tdbl12

tdbl11:
	loop    tdbl10

	; check link for word coming in - it will block our write if
	; it is present and is not immediately read.
	mov     dx,_link_in_status
	in      ax,dx
	and     ax,Bit7
	jz	tdbnord

	mov     _tdb_wordsaved, ax
	mov     dx, _link_read
	in	ax,dx
	mov	_tdb_wordbuffer1, ax	; read first 16bits of blocking word

	mov	cx,D500Msec
	mov     dx,_link_in_status
tdbtryrd:
	in      ax,dx
	and     ax,Bit7
	jnz	tdbrd2
	loop	tdbtryrd		; if we have one 16 bit word
					; the another must come soon.
tdbnord:
	pop	cx			; recall timeout
	dec	cx
	push	cx			; keep for later
	cmp	cx, 0			; timed out?
	jnz	tdbl101			; nope
	mov     ax,bx                   ; return count remaining
	shl	ax,1			; adjust from word to byte count

tdbl12:
	pop	cx			; restore stack, kicking off timeout 
tdbl12notime:
	pop	es
	pop     di
	pop     bp
 
	ret
 
tdbrd2:
	mov     dx,_link_read
	in	ax,dx			; read remaining 16bits of blocking word
	mov	_tdb_wordbuffer2, ax
	jmp	tdbl101			; try to write again

_tdb_send_block   ENDP


#if Ether_supported
; ==============================================================================
; -- Support for Clarkson Packet Drivers
; -- crf: May 1992
; ==============================================================================
; The following note was extracted from packet_d.109 ("PC/TCP Version 1.09 
; Packet Driver Specification") :
; "When a packet is received, receiver is called twice by the packet driver.
; The first time it is called to request a buffer from the application to
; copy the packet into. AX == 0 on this call. The application should return
; a pointer to the buffer in ES:DI. If the application has no buffers, it
; may return 0:0 in ES:DI, and the driver should throw away the packet and
; not perform the second call.
; ...
; On the second call, AX == 1. This call indicates that the copy has been
; completed, and the application may do as it wishes with the buffer. The
; buffer that the packet was copied into is pointed to by SI."
; ------------------------------------------------------------------------------
; There is some useful example code supplied with the Clarkson sources. An
; example receiver routine is given in pktall.asm. The routine that actually 
; calls the receiver can be found in head.asm.
; ------------------------------------------------------------------------------
#define MAXETHERPKT		1514		/* maximum size of packet */
#define MAX_PKT_TABLE		8		/* size of buffer table   */

#if !(MSWINDOWS)
	/* Keep in step with ether.c !!!                                  */
PUBLIC _receiver
_receiver PROC FAR

	push ds
	push dx
	mov dx, DGROUP	; default data segment
	mov ds, dx

	;======================================================================
	;Which call is this ? If 1st (AX == 0), return pointer to buffer in
	;ES:DI. If second (AX == 1), the packet has been copied into the 
	;buffer
	;======================================================================
	cmp ax, 0
	je assign_buffer

	;======================================================================
	;2nd call (AX == 1)
	;Packet has been copied into buffer
	;Increment index (with wrap around) and packet count
	;======================================================================
	inc rcvr_index
	cmp rcvr_index, MAX_PKT_TABLE
	jne got_pkt
	mov rcvr_index, 0
got_pkt:
	inc _pkt_rcvd
	jmp short done

	;======================================================================
	;1st call (AX == 0)
	;Enter with CX == packet length
	;Return pointer to buffer in ES:DI
	;======================================================================
assign_buffer:
	cmp _pkt_rcvd, MAX_PKT_TABLE	; Check for buffer overflow
	je short discard_pkt

	cmp cx, MAXETHERPKT		; Check if packet will fit into buffer
	jle ok_2
	mov _pkt_too_long, cx
	jmp short discard_pkt

ok_2:
	push bx				; Put packet length into pkt_len vector
	mov bx, rcvr_index
	shl bx, 1
	les di, _pkt_len
	mov es:[di+bx], cx
	pop bx

	les di, _pkt_table		; Set pointer to buffer in ES:DI
	push ax
	mov ax, MAXETHERPKT
	mul rcvr_index
	add di, ax	
	pop ax

	jmp short done

discard_pkt:
	xor di, di		; Error - return 0:0 in ES:DI
	xor dx, dx		; (pkt driver will throw packet away ...)
	mov es, dx

done:
	pop dx
	pop ds
	ret

_receiver	ENDP
#else /* MSWINDOWS */

; ==============================================================================
; -- DPMI real mode callback
; -- ENTRY:
; --	DS:SI = real mode SS:SP
; --	ES:DI = real mode call structure
; -- EXIT:
; --	ES:DI = real mode call structure
; -- refer DPMI spec. V1.0 pp. 34-38
; ==============================================================================

; The callback function is passed a real mode register data structure.
; Fortunately most of the fields can be ignored. The important ones are:
;	ax	0 or 1, as per the Clarkson spec
;	cx	size of packet
;	es:di	buffer for the new packet
;	cs:ip	must be filled in with return address on stack
; Note that the data structure tends to use the full 32 bit registers EDI,
; EAX etc. The packet driver works in terms of 16 bits, so the extensions
; are irrelevant.
	
PUBLIC _receiver
_receiver PROC FAR

	; Install the return value in the RealModeRegs structure
	; See DPMI spec., example on page 38
	cld
	lodsw
	mov	es:[di.r_IP], ax
	lodsw
	mov	es:[di.r_CS], ax
	add	es:[di.r_SP], 4
	
	; Switch to default data segment to access I/O Server's statics
	mov	dx, DGROUP
	mov	ds, dx

	; Extract the AX and CX values from the structure
	mov	ax, es:[di.r_AX]
	mov	cx, es:[di.r_CX]
	cmp	ax,0
	je	assign_buffer

	; 2nd call from Clarkson driver, packet has been copied into buffer
	; increment index (with wrap around) and packet count
	inc	rcvr_index
	cmp	rcvr_index, MAX_PKT_TABLE
	jne	got_pkt
	mov	rcvr_index, 0
got_pkt:
	inc	_pkt_rcvd
	jmp	short done

	; 1st call, cx == packet length, return buffer pointer in es:di
assign_buffer:
	cmp	_pkt_rcvd, MAX_PKT_TABLE	; Check for buffer overflow
	je	short discard_pkt

	cmp	cx, MAXETHERPKT		; Check if packet will fit into buffer
	jle	ok_2
	mov	_pkt_too_long, cx
	jmp	short discard_pkt

ok_2:
	; pkt_len[rcvr_index] = size (cx)
	mov bx, rcvr_index
	shl bx, 1			; short -> byte indexing
	lds si, _pkt_len		; do not corrupt es:di !
	mov ds:[si+bx], cx
	mov dx, DGROUP			; instead use ds:si and restore ds
	mov ds, dx
	
	; set up ES:DI in the realregs structure
	; ES == segment of packet buffer (not selector!)
	; DI == correct offset
	mov	ax, _pkt_table_segment
	mov	es:[di.r_ES], ax
	mov	ax, MAXETHERPKT
	mul	rcvr_index
	mov	es:[di.r_DI], ax

	jmp short done

discard_pkt:
	xor	ax,ax
	mov	es:[di.r_ES], ax
	mov	es:[di.r_DI], ax

done:
	iret

_receiver	ENDP
#endif /* MSWINDOWS */


#endif /* Ether_supported */

CODE    ENDS

        END
 
