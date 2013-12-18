; $Header: /giga/HeliosRoot/Helios/servers/fdc/RCS/fdcfiq.s,v 1.3 91/03/03 23:04:24 paul Exp $
; $Source: /giga/HeliosRoot/Helios/servers/fdc/RCS/fdcfiq.s,v $

	TTL	FIQ handlers for Helios/ARM floppy disc driver

;************************************************************************
;* fdcfiq.a - FIQ handlers for Helios/ARM floppy disc driver on Active	*
;*	      Book Functional Prototype.				*
;*									*
;* Copyright 1990 Active Book Company Ltd., Cambridge, England		*
;*									*
;* Author: Brian Knight, September 1990					*
;************************************************************************


;************************************************************************
;* This code is for the NEC uPD72068 floppy disc controller.		*
;*									*
;* The FIQ routines rely on the FDC asserting both FIQ and IRQ lines to	*
;* the ARM. During data transfer only the FIQ will be serviced, but at  *
;* the end of the transfer the handler routine masks FIQs and exits	*
;* without clearing the interrupt, allowing the IRQ handler to take	*
;* over.								*
;************************************************************************

; ----------------------------------------------------------------------
; Header files

	GET	listopts.s
	GET	arm.s
	GET	exmacros.s
	GET	SWIinfo.s

; ----------------------------------------------------------------------
; Registers
;
	[	{FALSE}

; Only banked FIQ registers (and pc) are used in this code

r0		RN	0
r1		RN	1
r8_fiq		RN	8	; General work register
r9_fiq		RN	9  
r10_fiq		RN	10 
r11_fiq		RN	11
r12_fiq		RN	12
r13_fiq		RN	13
r14_fiq		RN	14
lr		RN	14
pc		RN	15
	]

; Symbolic names for registers which hold values between FIQs

bufPtr		RN	r10_fiq		; Data buffer pointer
statusReg	RN	r11_fiq		; Address of FDC status register
dataReg		RN	r12_fiq		; Address of FDC data register
fiqMaskReg	RN	r13_fiq		; Address of FP FIQ mask register

; ----------------------------------------------------------------------
; Constants

SR_NDM	   *	&20	; Non-DMA transfer flag in FDC status register
FIQMode	   *	&1	; PSR bits for FIQ mode

; ----------------------------------------------------------------------
; Read FIQ handler
;
; If the NDM flag is on, this reads a data byte from the FDC into the
; buffer on each call. If the flag is off, it clears the FIQ enable
; register and exits, allowing the IRQ to be handled.

	EXPORT	ReadFIQHandler

ReadFIQHandler
	LDRB	r8_fiq,[statusReg]	; Get floppy controller status
	TST	r8_fiq,#SR_NDM		; Is this a data transfer interrupt?
	LDRNEB	r8_fiq,[dataReg]	; Yes: Get data byte
	STRNEB	r8_fiq,[bufPtr],#1	;      Put in buffer and step ptr
	SUBNES	pc,r14_fiq,#4		;      Return from FIQ

; Not a data transfer interrupt, so let the IRQ show through
	MOV	r8_fiq,#0
	STRB	r8_fiq,[fiqMaskReg]	; Disable all FIQ sources
	SUBS	pc,r14_fiq,#4		; Return

	EXPORT	ReadFIQEnd
ReadFIQEnd				; End marker for copying code

; ----------------------------------------------------------------------
; Write FIQ handler
;
; If the NDM flag is on, this writes a data byte from the buffer into the
; FDC on each call. If the flag is off, it clears the FIQ enable
; register and exits, allowing the IRQ to be handled.

	EXPORT	WriteFIQHandler

WriteFIQHandler
	MOV	r9_fiq,#&740000		; Address of screen word
	STR	bufPtr,[r9_fiq]		; Make a mark
	LDRB	r8_fiq,[statusReg]	; Get floppy controller status
	TST	r8_fiq,#SR_NDM		; Is this a data transfer interrupt?
	LDRNEB	r8_fiq,[bufPtr],#1	; Yes: Get data byte and step ptr
	STRNEB	r8_fiq,[dataReg]	;      Send to controller
	SUBNES	pc,r14_fiq,#4		;      Return from FIQ

; Not a data transfer interrupt, so let the IRQ show through
	MOV	r8_fiq,#0
	STRB	r8_fiq,[fiqMaskReg]	; Disable all FIQ sources
	SUBS	pc,r14_fiq,#4		; Return

	EXPORT	WriteFIQEnd
WriteFIQEnd				; End marker for copying code

; ----------------------------------------------------------------------
; Routine to set up the six banked registers preserved between FIQ calls.
; Must be called in a non-user mode.
;
; SetFIQRegs(struct FIQRegs *fiqRegs);
;
; struct FIQRegs
; {
;   unsigned r8_fiq;
;   unsigned r9_fiq;
;   unsigned r10_fiq;
;   unsigned r11_fiq;
;   unsigned r12_fiq;
;   unsigned r13_fiq;
; }

	EXPORT	SetFIQRegs

SetFIQRegs
; Pointer to list of values in r0. Corrupts r1.
	MOV	r1,lr			; Put return in unbanked register
	SWI	exec_EnterSVC		; Enter SVC mode
	TEQP	pc,#FIQMode		; Enter FIQ mode
	MOVNV	r0,r0			; Wait for mode change
	LDMIA	r0,{r8_fiq-r13_fiq}	; Load the FIQ registers
	MOVS	pc,r1			; Return to caller in original mode

; ----------------------------------------------------------------------

	END
