;*---------------------------------------------------------------------------*;
;--                        HELIOS PC  I/O SYSTEM                            --;
;--                        =====================                            --;
;--                                                                         --;
;--             Copyright (C) 1987, Perihelion Software Ltd.                --;
;--               All Rights Reserved.                                      --;
;--                                                                         --;
;--                                                                         --;
;--                                                                         --;
;-- MODULE NAME: rawdisk.ASM                                                  --;
;--                                                                         --;
;-- AUTHOR : BLV                                                            --;
;-- DATE : 23.8.89                                                          --;
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

PUBLIC _version_number

_version_number	DW	0

PUBLIC	_setversion
_setversion	PROC	FAR
	pushall

	mov	ah,030H
	mov	al,0H
	int	21H
	xor	ah,ah
	mov	cs:_version_number,ax

	popall
	ret
_setversion	ENDP

;==============================================================================
; 
;  Raw disk support
;
;==============================================================================


PUBLIC _disk_read, _disk_write

read_int	EQU	025h
write_int	EQU	026h

;==============================================================================
; _disk_read(int drive, int no_sects, WORD first_sec, byte *buf)
;==============================================================================

transfer_buffer	DB	0,0,0,0,0,0,0,0,0,0	; 10 byte buffer

_disk_read	PROC	FAR
	pushall

	mov	ax,arg1[bp]	; drive id, 0 = A, 1 = B etc.
	cmp	ax,2
	jne	rdrive_ok
	mov	ax,0310H
	jmp	readdone

rdrive_ok:	
	mov	bx,cs:_version_number
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
	jmp	readdone

msdos4_read:
	mov	cx,-1
	mov	bx,cs
	mov	ds,bx
	mov	si,offset transfer_buffer
	mov	bx,arg3[bp]	; first sector, low 2 bytes
	mov	ds:[si],bx
	mov	bx,arg4[bp]	; first sector, high bytes
	mov	ds:[si+2],bx
	mov	bx,arg2[bp]	; number of sectors
	mov	ds:[si+4],bx
	mov	bx,arg5[bp]	; offset of buffer
	mov	ds:[si+6],bx
	mov	bx,arg6[bp]	; segment of buffer
	mov	ds:[si+8],bx
	mov	bx,si		; ds:bx now contains structure
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
	jmp	writedone

wdrive_ok:
	mov	bx,cs:_version_number
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
	jmp	write_error
write_safe:

	int	write_int

	jc	write_error	; carry set if error, with error code in ax
	xor	ax,ax
write_error:
	add	sp,2
	jmp	writedone

msdos4_write:
	mov	cx,-1
	mov	bx,cs
	mov	ds,bx
	mov	si,offset transfer_buffer
	mov	bx,arg3[bp]	; first sector, low 2 bytes
	mov	ds:[si],bx
	mov	bx,arg4[bp]	; first sector, high bytes
	mov	ds:[si+2],bx
	mov	bx,arg2[bp]	; number of sectors
	mov	ds:[si+4],bx
	mov	bx,arg5[bp]	; offset of buffer
	mov	ds:[si+6],bx
	mov	bx,arg6[bp]	; segment of buffer
	mov	ds:[si+8],bx
	mov	bx,si		; ds:bx now contains structure
	int	write_int	; do the read

	jc	write_error2	; carry flag set if error, with code in ax
	xor	ax,ax		; no error so return 0
write_error2:
	add	sp,2		; as per instructions

writedone:
	popall
	ret
_disk_write	ENDP
  
CODE    ENDS

        END
 	
