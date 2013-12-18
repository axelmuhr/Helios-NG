* Routines to read the size of the display.
* MicroEMACS works even on a screen that has been blessed
* by the "hi50" program.
* MicroEMACS version 30, for the Atari.
 
	.text
 
* getnrow() -- get number of rows.
 
	.globl	_getnrow
 
_getnrow:
 
	move.l	a2, -(sp)
	move.l	d2, -(sp)
	dc.w	$A000
	move.l	(sp)+, d2
	movea.l	(sp)+, a2
 
	move.w	-42(a0), d0
	addq.w	#1, d0
	ext.l	d0
 
	rts
 
* getncol() -- get number of columns.
 
	.globl	_getncol
 
_getncol:
	move.l	a2, -(sp)
	move.l	d2, -(sp)
	dc.w	$A000
	move.l	(sp)+, d2
	movea.l	(sp)+, a2
 
	move.w	-44(a0), d0
	addq.w	#1, d0
	ext.l	d0
 
	rts
