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

_TEXT    SEGMENT WORD public 'code'
        ASSUME cs:_TEXT

;==============================================================================
;   The simple interrupt-handling stuff
;==============================================================================
  
; External functions called
PUBLIC _set_interrupts, _tsr

;=============================================================================;
; void tsr(void)
;=============================================================================;
_tsr	PROC	NEAR
	mov	ah,031H
	mov	al,0
	mov	dx,1000H
	int	21H
	ret
_tsr	ENDP

;=============================================================================;
; void set_interrupts(void)
;=============================================================================;

Wakeup_int	EQU	061H
Floppy_int	EQU	062H
General_int	EQU	063H

old_Floppy	EQU	013H
old_General	EQU	021H

GetInt		EQU	035H
SetInt		EQU	025H
DoInt		EQU	021H
MAGIC		EQU	4321H

Default_DS	DW	0
Default_SS	DW	0
Default_SP	DW	0

_set_interrupts PROC NEAR
	pushall

	mov	ah,GetInt	; see if already installed
	mov	al,old_Floppy
	int	DoInt
	xor	ax,ax
	mov	di,bx
	mov	bx,es:[di-2]
	cmp	bx,MAGIC
	je	done
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
	int	DoInt

	mov	ah,GetInt
	mov	al,old_Floppy		; floppy bios
	int	DoInt
	mov	dx,es
	mov	ds,dx
	mov	dx,bx
	mov	al,Floppy_int
	mov	ah,SetInt
	int	DoInt

	mov	ah,GetInt
	mov	al,old_General
	int	DoInt
	mov	dx,es
	mov	ds,dx
	mov	dx,bx
	mov	al,General_int
	mov	ah,SetInt
	int	DoInt

	mov	dx,cs
	mov	ds,dx
	mov	dx,offset New_floppy
	mov	ah,SetInt
	mov	al,old_Floppy
	int	DoInt

	mov	dx,cs
	mov	ds,dx
	mov	dx,offset New_general
	mov	ah,SetInt
	mov	al,old_General
	int	DoInt

	mov	ax,1
done:
	popall
	ret
_set_interrupts ENDP

;==============================================================================
; Replacement interrupt routines for everything
;==============================================================================

junk	DW	MAGIC

New_floppy	PROC	NEAR
	call	_increase
	sti
	int	Floppy_int
	cli
	call	_decrease
	iret
New_floppy	ENDP

disable_flags	DB	0	; 00 Terminate
		DB	0	; 01 keyboard input
		DB	0	; 02 Character output
		DB	0	; 03 Auxiliary input
		DB	0	; 04 Auxiliary output
		DB	0	; 05 Printer output
		DB	0	; 06 direct console I/O
		DB	0	; 07 unfiltered character input
		DB	0	; 08 character input
		DB	0	; 09 output string
		DB	0	; 0A buffered input
		DB	0 	; 0B get input status
		DB	0	; 0C reset input buffer
		DB	1	; 0D disk reset
		DB	1	; 0E set default disk drive
		DB	1	; 0F open file
		DB	1	; 10 close file
		DB	1	; 11 search for first
		DB	1	; 12 search for next
 	   	DB	1	; 13 Delete file
		DB	1	; 14 sequential read
		DB	1	; 15 sequential write
		DB	1	; 16 create file
		DB	1	; 17 rename file
		DB	1	; 18 reserved
		DB	1	; 19 get default disk drive
		DB	1	; 1A set disk transfer area address
		DB	1	; 1B get allocation info
		DB	1	; 1C "" for specified drive
		DB	1	; 1D reserved
		DB	1	; 1E reserved
		DB	1	; 1F reserved
		DB	1	; 20 reserved
		DB	1	; 21 random read
		DB	1	; 22 random write
		DB	1	; 23 get file size
		DB	1	; 24 set record number
		DB	1	; 25 set interrupt vector
		DB	0	; 26 create PSP
		DB	1	; 27 random block read
		DB	1	; 28 random block write
		DB	1	; 29 parse filename
		DB	0	; 2A get system date
		DB	0	; 2B set system date
		DB	0	; 2C get system time
		DB	0	; 2D set system time
		DB	1	; 2E set verify flag
		DB	0	; 2F Get disk transfer area
		DB	0	; 30 get MSdos version number
		DB	0	; 31 TSR
		DB	1	; 32 reserved
		DB	0	; 33 get or set break flag
		DB	1	; 34 reserved
		DB	0	; 35 get interrupt vector
		DB	1	; 36 get free disk space
		DB	1	; 37 reserved
		DB	0	; 38 get or set country
		DB	1	; 39 create subdirectory
		DB	1	; 3A delete subdirectory
		DB	1	; 3B set current directory
		DB	1	; 3C create file
		DB	1	; 3D open file
		DB	1	; 3E close file
		DB	1	; 3F read file or device
		DB	1	; 40 write to file or device
		DB	1	; 41 delete file
		DB	1	; 42 seek
		DB	1	; 43 get or set file attributes
		DB	1	; 44 ioctl
		DB	1	; 45 duplicate handle
		DB	1	; 46 force duplicate
		DB	1	; 47 get current directory
		DB	0	; 48 allocate memory
		DB	0	; 49 release memory
		DB	0	; 4A modify allocation
		DB	1	; 4B execute program
		DB	1	; 4C terminate
		DB	0	; 4D get return code
		DB	1	; 4E search for first
		DB	1	; 4F search for next
		DB	1	; 50 reserved		
		DB	1	; 51 reserved		
		DB	1	; 52 reserved		
		DB	1	; 53 reserved
		DB	0	; 54 get verify flag
		DB	1	; 55 reserved
		DB	1	; 56 rename file
		DB	1	; 57 set file date or time
		DB	0	; 58 get or set allocation strategy
		DB	0	; 59 get extended error info
		DB	1	; 5A create temporary file
		DB	1	; 5B create new file
		DB	1	; 5C control record access
		DB	1	; 5D reserved
		DB	1	; 5E networking
		DB	1	; 5F networking
		DB	1	; 60 reserved
		DB	1	; 61 reserved
		DB	0	; 62 get PSP
		DB	0	; 63 get lead byte table
		DB      0,0,0,0,0,0,0,0,0,0,0,0	; spare 64 - 6F
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; 70-7F
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; 80-8F
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; 90-9F
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; A0-AF
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; B0-BF
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; C0-CF
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; D0-DF
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; E0-EF
		DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	; F0-FF

New_general	PROC	NEAR
	push	bp
	mov	bp,sp
	sub	sp,2

	push	ax
	push	di
	mov	di,offset disable_flags
	xor	ah,ah
	add	di,ax
	mov	al,cs:[di]
	mov	ss:[bp-2],ax
	pop	di

	cmp	al,0	
	je	skip_increase
	call	_increase	; disable the link interrupts
skip_increase:
	pop	ax

	sti
	int	General_int

	push	ax		; zap the flags as stored on the stack
	pushf			; with the current flags
	pop	ax
	mov	ss:[bp+6],ax

	cli
	mov	ax,ss:[bp-2]
	cmp	ax,0
	je	skip_decrease
	call	_decrease
skip_decrease:
	pop	ax

	mov	sp,bp
	pop	bp
	iret
New_general	ENDP

call_level	DW	1
call_count	DW	0

PUBLIC	_increase
_increase	PROC	NEAR
	push	ax
	mov	ax,cs:call_count
	inc	ax
	mov	cs:call_count,ax
	mov	ax,cs:call_level
	inc	ax
	mov	cs:call_level,ax
	cmp	ax,1
	jne	increase_skip
	push	dx

	pop	dx
increase_skip:
	pop	ax
	ret
_increase	ENDP

PUBLIC _decrease
_decrease	PROC	NEAR
	push	ax
	mov	ax,cs:call_level
	dec	ax
	mov	cs:call_level,ax
	cmp	ax,0
	jne	decrease_skip
	push	dx

	pop	dx
decrease_skip:
	pop	ax
	ret
_decrease	ENDP

;==============================================================================
; Wakeup routine
;
; Installed at int 61, invoked by the program wakeup.com
;==============================================================================

old_ss	DW	0
old_sp	DW	0

Wakeup_rtn	PROC	NEAR
	int_push
	mov	dx,ss
	mov	cs:old_ss,dx
	mov	dx,sp
	mov	cs:old_sp,dx
	mov	dx,cs:Default_DS
	mov	ds,dx
	mov	es,dx
	mov	dx,cs:Default_SS
	mov	ss,dx
	mov	dx,cs:Default_SP
	mov	sp,dx

;	call	_wakeup

	mov	dx,cs:old_sp
	mov	sp,dx
	mov	dx,cs:old_ss
	mov	ss,dx

	int_pop
	iret
Wakeup_rtn	ENDP


_TEXT	ENDS
	END
