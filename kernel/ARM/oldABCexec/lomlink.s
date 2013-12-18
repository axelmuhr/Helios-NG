        SUBT Executive microController communications               > lomlink/s
        ;    Copyright (c) 1991, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; Microlink Protocol
        ; ---------------------------------------------------------------------
	; The first byte of a microlink message to or from the
	; microcontroller is a header which determines the format of the
	; rest of the message (if any). There are 3 formats: `short',
	; `long' and `extended':
	;
	; A short message consists of only one byte:
	; 
	;   1    5       2
	;  +-+--------+------+
	;  |0|  type  | data |
	;  +-+--------+------+
	; 
	; A long message is up to 5 bytes in length:
	; 
	;   1     5       2
	;  +-+---------+-----+
	;  |1|  type   |lcode|  followed by 1, 2 or 4 data bytes 
	;  +-+---------+-----+
	; 
	;  `lcode' indicates how many data bytes follow the header:
	; 
	;   00:  1 byte
	;   01:  2 bytes
	;   10:  4 bytes
	;   11:  (used for extended messages: see below)
	; 
	; An extended message is up to 34 bytes long:
	; 
	;   1     5       2         3        5
	;  +-+---------+-----+  +-------+---------+
	;  |1|  type   | 1 1 |  | 0 0 0 | length  | then (length+1) data bytes
	;  +-+---------+-----+  +-------+---------+
	; 


	[	(hercules)
	[	(hercmlink)
	; FIQ is used in the microlink code for Hercules only, and
	; there only for reception.

        ; ---------------------------------------------------------------------
	; Microlink FIQ registers
	; 
	; DO NOT ALTER THESE REGISTER ALLOCATIONS WITHOUT MAKING 
	; CORRESPONDING CHANGES TO THE CALL OF local_AttachSFIQ FROM
	; code_exec_InitBackplane (IN loswi3.s).
        ; ---------------------------------------------------------------------

	; fiq_r8, fiq_r9 and fiq_r10 are used as work registers.

rxData		RN	fiq_r11		; address of microlink rx data reg

	; The progress of the reception of the current message is held in
	; two registers: rxBufPtr and rxBufEnd. During reception of a long
	; or extended message, rxBufEnd holds the address of the byte after
	; the one which will hold the end of the message, minimising time spent
	; in the FIQ handler for most bytes. rxBufEnd also has two special
	; values (which will always be less than the buffer pointer):
	;   0 when awaiting a header byte, and
	;  -1 when awaiting the length byte of an extended message

rxBufPtr	RN	fiq_r12		; ptr to position for next byte
rxBufEnd	RN	fiq_r13		; ptr to byte after expected end of msg

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; Microlink FIQ routine
	; ---------------------
	; The FIQ routine receives the individual bytes of a microlink
	; message, raising an IRQ when a complete message has been received.
	; When an IRQ is raised by the FIQ handler, a one-word argument
	; (ML_IRQArg) is set up to inform the IRQ handler why it has been
	; invoked. Values of this argument are:

MLI_NoMsg	*	0		; IRQ not caused by FIQ handler
					; (Initial value before FIQ enabled)
MLI_Msg		*	1		; Buffer contains one microlink message
MLI_SoftBreak	*	2		; Soft break (0 header byte received)

	; This FIQ routine is longer than the 19 words available in
	; low memory, so has to be split into two pieces. The code
	; between ML_RxFIQ and ML_RxFIQ_Low_End is copied down
	; into the fast RAM starting at the FIQ vector, and is
	; executed there only. The remainder (from ML_FIQ_RaiseIRQ) 
	; runs in high ROM or RAM, so the branches from the low part
	; to the high part are fabricated accordingly.

	; At the moment the "static" code (with branches) is copied down
	; using the "AttachSFIQ" code. However, we wish to generate the
	; branches dynamically. The ideal solution is to update the
	; the "AttachSFIQ" code to perform branch relocation (if the
	; branch instructions reference code outside the copied area).
	; This, however, poses the problem of data within the copied code
	; being treated instructions (and modified if they match the branch
	; instruction specification).

	; This handler receives bytes from the microcontroller via the
	; microlink interface and raises an IRQ when it has collected
	; a complete message of the microlink protocol used between Hercules
	; and the microcontroller.
	;
	; For simplicity and speed there is only one buffer: once a complete
	; message has been received, the microlink rx FIQ is masked out and
	; a microlink rx IRQ is raised by using the IRQ Test register.
	; The IRQ handler re-enables microlink rx FIQs as soon as it has
	; finished with the buffer.
	;
        ; ---------------------------------------------------------------------
        ; Reception FIQ handler
        ; ---------------------------------------------------------------------
ML_RxFIQ
	; Could check for frame error at this point. However, it is
	; extremely unlikely that one can occur inside the AB without there
	; being something fatally wrong which will be detected elsewhere, 
	; so it is not worth the expense of testing this on every FIQ.
	;
	; The code is ordered so that the critical case of handling data bytes
	; within a long or extended message completes in 4 instructions.

	LDRB	fiq_r8,[rxData]		; Read data byte & clear interrupt
	STRB	fiq_r8,[rxBufPtr],#1	; Store in buffer and step pointer
	CMP	rxBufPtr,rxBufEnd	; Received whole message?
	SUBLTS	pc,fiq_r14,#4		; More to come: exit immediately

	; Is this the final byte of a message? If so, raise IRQ.
	BEQ	ML_RxFIQ_MsgComplete	; Destination is in low memory

	; At this point rxBufEnd must have one of its special values 0 or -1

	CMP	rxBufEnd,#0		; Is this a header byte?
	BNE	ML_RxFIQ_LengthByte	; NO: must be length of extended msg

	; We have received a header byte: test its format

	TST	fiq_r8,#MLHdr_Long	; Long or extended message?
	BNE	ML_RxFIQ_LongOrExtended ; Yes

	; We have a short message header, or a soft break byte (all zero)

	[	(softbreak)
	CMP	fiq_r8,#0		; Soft break byte?
	MOVEQ	fiq_r8,#MLI_SoftBreak	; Yes: set arg for IRQ handler
	BEQ	ML_RxFIQ_RaiseIRQ	; Yes: invoke IRQ handler
	]	; EOF (softbreak)
	; Not soft break: drop through, leaving rxBufEnd zero

ML_RxFIQ_MsgComplete
	; Invoke the IRQ handler to process a complete message
	MOV	fiq_r8,#MLI_Msg		; Set reason in IRQ handler arg word
	B	ML_RxFIQ_RaiseIRQ 	; Continue in high memory

ML_RxFIQ_LowEnd
	; ----------------------------------------------------------
	; Code below this point is not copied down to the FIQ vector
	; ----------------------------------------------------------

ML_RxFIQ_RaiseIRQ
	; Use the Test register to raise an IRQ with MicroLink reception
	; as the apparent source. The fact that the argument word ML_Rx_IRQArg
	; is non-zero tells the IRQ handler that the FIQ handler caused
	; the interrupt.
	;
	; fiq_r8 contains the argument for the IRQ handler

	LDR	fiq_r10,=ML_Rx_IRQArg	; Address of argument word
	STR	fiq_r8,[fiq_r10]	; Store argument to IRQ handler

	MOV	fiq_r9,#INT_regs	; Get base of interrupt control regs
	LDR	fiq_r10,=hardware_regs	; Base of soft copies of h/w regs
	LDR	fiq_r8,[fiq_r10,#INTtest_data]; Get soft copy of int Test reg
	ORR	fiq_r8,fiq_r8,#INT_MRX	; Set microlink rx int flag
	STR	fiq_r8,[fiq_r10,#INTtest_data] ; Update soft copy
	STR	fiq_r8,[fiq_r9,#INT_status]; Update Interrupt Test register

	; Mask out microlink reception FIQs before exiting.
	; The IRQ handler will reenable them as soon as it has set up
	; another reception buffer.
	; r10 still contains address of hardware_regs.

	LDR	fiq_r8,[fiq_r10, #FIQ_data]; Get soft copy of FIQ mask register
	BIC	fiq_r8,fiq_r8,#INT_MRX	; Clear microlink reception int flag
	STR	fiq_r8,[fiq_r10, #FIQ_data]; Update soft copy
	MOV	fiq_r8,#INT_MRX		; Bit to clear in FIQ mask register
	STR	fiq_r8,[fiq_r9,#FIQ_control]; Update FIQ mask register

	SUBS	pc,fiq_r14,#4		; Return from FIQ handler
	
	; Decode header byte of long or extended message, setting rxBufEnd:
	;  fiq_r8:   header byte
	;  rxBufPtr: points to second byte of buffer
ML_RxFIQ_LongOrExtended
	AND	fiq_r8,fiq_r8,#MLHdr_LenCode; Extract length code
	ADD	fiq_r8,fiq_r8,#1	; Length code now 1 - 4
	CMP	fiq_r8,#3		; 1&2 OK, 3 means 4, 4 for extended msg
	MOVEQ	fiq_r8,#4		; Set to 4 if code was 3
	ADDLE	rxBufEnd,rxBufPtr,fiq_r8; Set rxBufEnd for long message
	MVNGT	rxBufEnd,#0		; Set rxBufEnd to -1 for extended msg
	
	SUBS	pc,fiq_r14,#4		; Return from FIQ

ML_RxFIQ_LengthByte
	; The received byte is the length byte of an extended message.
	;
	; fiq_r8:   length byte
	; rxBufPtr: points to third byte of buffer
	AND	fiq_r8,fiq_r8,#MLExt_LenMask; Extract length
	ADD	fiq_r8,fiq_r8,#1	; Add 1 to get number of bytes
	ADD	rxBufEnd,rxBufPtr,fiq_r8; Set rxBufEnd for extended message
	
	SUBS	pc,fiq_r14,#4		; Return from FIQ
	]	; EOF (hercmlink)
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; Microlink IRQ routines
	; ----------------------

	[	(hercules)
	[	(hercmlink)
        ; ---------------------------------------------------------------------
	; The reception IRQ is always a dummy one caused by the rx FIQ
	; handler to signal the end of a received microlink message or
	; a soft break. The value of the communication word indicates
	; the reason for the IRQ.
ML_Rx_IRQ
	; in:	SVC mode; IRQs disabled; FIQs undefined.
	;	svc_r14: current process return address
	;	svc_r13: FD stack (containing original SVC r13 and r14)
	;	All other registers must be preserved
	;	svc_r14 and svc_r13 should be restored on exit

	STMFD	sp!,{r0,r1,r2,r3,r4,r5}	; Free some work registers

	; Clear the bit in the interrupt test register which the FIQ
	; routine set to cause this IRQ. Note that this update of register
	; and soft copy is not FIQ-protected: however microlink should
	; be the only FIQ source, and its FIQ is masked out at present.

	MOV	r1,#INT_regs		; Get base of interrupt control regs
	LDR	r2,=hardware_regs	; Get base of soft copies of h/w regs
	LDR	r0,[r2,#INTtest_data]	; Get soft copy of interrupt Test reg
	BIC	r0,r0,#INT_MRX		; Clear microlink rx int flag
	STR	r0,[r2,#INTtest_data]	; Update soft copy
	STR	r0,[r1,#INT_status]	; Update interrupt Test register

	LDR	r2,=ML_RxBuf_Ptr	; Base address of rx variables
	LDR	r3,[r2,#(ML_Rx_IRQArg - ML_RxBuf_Ptr)]; Arg word from FIQ rtn
	CMP	r3,#MLI_SoftBreak	; Has a soft break been received?
	BEQ	ML_RxIRQ_SoftBreak	; Yes
	CMP	r3,#MLI_Msg		; Has a complete msg been received?
	BNE	ML_Exit_IRQ		; No (shouldn't happen)

	; A complete microlink message has been received.
	; Mask out microlink reception IRQs while we process the message:
	; this is not strictly necessary at present (as all IRQs are off 
	; anyway) but provides some defence against message handlers which
	; inadvertently reenable IRQs. Eventually, these handlers should
	; be called in a separate SVC thread with interrupts enabled.

	MOV	r1,#INT_regs		; Get base of interrupt control regs
	LDR	r2,=hardware_regs	; Get base of soft copies of h/w regs
	LDR	r0,[r2,#IRQ_data]	; Get soft copy of IRQ mask reg
	BIC	r0,r0,#INT_MRX		; Clear microlink rx int flag
	STR	r0,[r2,#IRQ_data]	; Update soft copy
	MOV	r0,#INT_MRX		; Value to clear bit in mask reg 
	STR	r0,[r1,#IRQ_control]	; Mask out microlink rx IRQ

	; Swap over the pointers to the "current" and "other" 
	; reception buffers.

	LDR	r2,=ML_RxBuf_Ptr	; Base address of rx variables
	LDR	r0,[r2,#(ML_Rx_CurBuf - ML_RxBuf_Ptr)]
	LDR	r1,[r2,#(ML_Rx_OtherBuf - ML_RxBuf_Ptr)]
	STR	r0,[r2,#(ML_Rx_OtherBuf - ML_RxBuf_Ptr)]
	STR	r1,[r2,#(ML_Rx_CurBuf - ML_RxBuf_Ptr)]

	; The reception FIQ handler can now be allowed to proceed,
	; using the other buffer.
	; First restore the FIQ registers.
	;  r0 = address of buffer containing received message
	;  r1 = new current buffer pointer

	MOV	r3,pc			; Save PSR bits
	TEQP	pc,#(Ibit :OR: FIQmode) ; FIQ mode; IRQs off
	NOP				; Wait for mode change

	MOV	rxBufPtr,r1		; New current buffer pointer
	MOV	r1,rxBufEnd		; Save end of message pointer
	MOV	rxBufEnd,#0		; Waiting for header byte

	TEQP	pc,r3			; Back to previous mode & int state
	NOP				; Wait for mode change 

	; Reenable the microlink reception FIQ
	;  r0 = address of buffer containing received message
	;  r1 = address of first byte after end of message

	MOV	r3,#INT_regs		; Get base of interrupt control regs
	LDR	r2,=hardware_regs	; Get base of soft copies of h/w regs
	LDR	r4,[r2,#FIQ_data]	; Get soft copy of FIQ mask reg
	ORR	r4,r4,#INT_MRX		; Enable microlink rx int flag
	STR	r4,[r2,#FIQ_data]	; Update soft copy
	MOV	r4,#INT_MRX
	ORR	r4,r4,#FIQ_set		; Value to set bit in mask reg 
	STR	r4,[r3,#FIQ_control]	; Enable microlink rx FIQ

	; Deal with received message, using code shared with FP version
	;  r0 = address of buffer containing received message
	;  r1 = address of first byte after end of message
	B	ML_ProcessRxMsg		; This returns to ML_Complete_Exit

	; Have now finished with this message, so can reenable rx IRQs
ML_Complete_Exit
	MOV	r1,#INT_regs		; Get base of interrupt control regs
	LDR	r2,=hardware_regs	; Get base of soft copies of h/w regs
	LDR	r0,[r2,#IRQ_data]	; Get soft copy of IRQ mask reg
	ORR	r0,r0,#INT_MRX		; Enable microlink rx int flag
	STR	r0,[r2,#IRQ_data]	; Update soft copy
	MOV	r4,#INT_MRX
	ORR	r4,r4,#IRQ_set		; Value to set bit in mask reg 
	STR	r4,[r1,#IRQ_control]	; Enable microlink rx IRQ

	B	ML_Exit_IRQ		; Return from IRQ

ML_RxIRQ_SoftBreak
	; Disable the microlink interface (as a received hardware break
	; would have done), then branch to the hard break handler.

	MOV	r1,#MLI_regs		; Base addr of microlink hardware
	MOV	r0,#MLI_ICP		; Value to disable microlink
	STRB	r0,[r1,#MLI_CON]	; Disable microlink interface

	; Reenable the microlink reception FIQ, which was disabled
	; when the FIQ routine raised this IRQ.
	MOV	r3,#INT_regs		; Get base of interrupt control regs
	LDR	r2,=hardware_regs	; Get base of soft copies of h/w regs
	LDR	r4,[r2,#FIQ_data]	; Get soft copy of FIQ mask reg
	ORR	r4,r4,#INT_MRX		; Enable microlink rx int flag
	STR	r4,[r2,#FIQ_data]	; Update soft copy
	MOV	r4,#INT_MRX
	ORR	r4,r4,#FIQ_set		; Value to set bit in mask reg 
	STR	r4,[r3,#FIQ_control]	; Enable microlink rx FIQ

	; Continue in hardware break handler.
	; r0 - r5 are stacked.

	B	ML_Break_Common		; Enter hardware break code

        ; ---------------------------------------------------------------------

ML_Tx_IRQ
	; A transmit interrupt is outstanding on the microlink.
	; in:	SVC mode; IRQs disabled; FIQs undefined.
	;	svc_r14: current process return address
	;	svc_r13: FD stack (containing original SVC r13 and r14)
	;	All other registers must be preserved
	;	svc_r14 and svc_r13 should be restored on exit

	STMFD	sp!,{r0,r1,r2,r3,r4,r5}	; Free some work registers
	
	LDR	r2,=ML_TxBuf_Ptr	; Base address of tx variables
	LDR	r3,[r2]			; Buffer pointer in r3
	LDR	r4,[r2,#(ML_TxBuf_End - ML_TxBuf_Ptr)]; End of buffer

	; Test for zero-sized buffer TBD

	LDRB	r0,[r3],#1		; Get next byte & step pointer
	MOV	r1,#MLI_regs		; Base addr of microlink hardware
	STRB	r0,[r1,#MLI_TXD]	; Write byte to link device

	STR	r3,[r2]			; Store updated buffer pointer
	CMP	r3,r4			; Reached end of buffer?
	BLT	ML_Exit_IRQ		; No: exit from IRQ

	; This transmission has finished: disable tx interrupt
	MOV	r0,#INT_MTX		; Microlink tx int mask
	MOV	r1,r14			; Preserve link register
	BL	local_DisableIRQ	; Disable tx IRQ
	MOV	r14,r1			; Restore link register

	; Resume waiting process: first get SaveState pointer (unsetting it
	; so break handling code cannot try to use it again).
	LDR	r1,[r2,#(ML_TxBuf_SaveState - ML_TxBuf_Ptr)]
	MOV	r0,#0
	STR	r0,[r2,#(ML_TxBuf_SaveState - ML_TxBuf_Ptr)]; Unset SaveState
	STR	r0,[r1,#SaveState_r0]	; Set r0 in resumed process to 0
					; to indicate successful transmission

	LDR	r0,=ROOT_start

	; r0 = `ExecRoot' data structure
	; r1 = pointer to SaveState of process to be restarted
	ADD	r3,r0,#ExecRoot_queues	; base of priority 0 process queue

	; Add SaveState to end of referenced queue
	LDR	r2,[r3,#ProcessQ_tail]	; load the current tail node
	STR	r1,[r2,#SaveState_next] ; reference the new tail node
	STR	r1,[r3,#ProcessQ_tail]	; update tail pointer

	B	ML_Exit_IRQ		; Return from IRQ

        ; ---------------------------------------------------------------------
ML_Break_IRQ
	; A break packet has been received
	; in:	SVC mode; IRQs disabled; FIQs undefined.
	;	svc_r14: current process return address
	;	svc_r13: FD stack (containing original SVC r13 and r14)
	;	All other registers must be preserved
	;	svc_r14 and svc_r13 should be restored on exit

	STMFD	sp!,{r0,r1,r2,r3,r4,r5}	; Free some work registers

ML_Break_Common
	; Enter here after receiving a soft break message (with r0-r5 stacked).

	; The microlink interface is automatically disabled when a break
	; interrupt is received (and is disabled by software on a soft break).
	; Set the driver's state to fend off any client requests during 
	; the break protocol.

	LDR	r1,=ML_State		; Get address of driver state word
	MOV	r0,#ML_State_GotBreak	; Break was sent from other end
	STR	r0,[r1]			; Update state word

	; Clean up any microlink transmissions and receptions which were
	; in progress at this end (and disable tx IRQs)

	MOV	r1,r14			; Preserve the link
	BL	ML_CleanUp		; Abort transmissions and receptions
	MOV	r14,r1			; Restore the link

	; The break interrupt is cleared by reenabling the microlink

	MOV	r0,#(MLI_ENA :OR: MLI_ICP) ; Enable with internal clock
	MOV	r1,#MLI_regs		; Base addr of microlink hardware
	STRB	r0,[r1,#MLI_CON]	; Write microlink control register

	[	((newbreak) :LAND: ((activebook) :LAND: (shutdown)))
	; Call the power-failure handler.
	; *** This should be done only if we did not cause the break ***
	!	0,"TODO: break protocol when Hercules initiated the break"
	
	MOV	r5,r14			; Save the return link
	BL	PM_PowerFail		; Call power failure handler

	; The power will have been turned off and on again here

	MOV	r14,r5			; Restore return link

	; Break handling is over: set the state back to normal
	LDR	r1,=ML_State		; Get address of driver state word
	MOV	r0,#ML_State_Normal	; Normal state
	STR	r0,[r1]			; Update state word
	]

	B	ML_Exit_IRQ		; Return from IRQ

        ; ---------------------------------------------------------------------
ML_CleanUp
	; Subroutine to abort any outstanding reception and transmission.
	; in:	SVC mode; IRQs undefined; FIQs undefined.
	;	r13: FD stack, r14: return link
	;       Microlink interface is disabled, so no interrupts will occur.
	; out:  All registers preserved.
	;	Microlink tx interrupt disabled.
	;	Process waiting for transmission (if any) resumed with
	;	an error code.
	;	Any processes waiting for reception are resumed with error
	;	codes and the corresponding reception requests are released.
	;	All other reception requests remain outstanding, but any that
	;	were previously satisfied are now marked empty again.
	;	The reception FIQ routine is reset to await a new
	;	message header.

	STMFD	r13!,{r0,r1,r2,r3,r4,r14}; Save some registers

	; Disable microlink tx interrupt, aborting any transmission in progress

	MOV	r0,#INT_MTX		; Microlink tx int mask
	BL	local_DisableIRQ	; Disable tx IRQ

	; Resume waiting tx process if any

	LDR	r2,=ML_TxBuf_Ptr	; Get base of tx variables
	MOV	r0,#0
	STR	r0,[r2]			; Clear buffer start pointer
	STR	r0,[r2,#(ML_TxBuf_End - ML_TxBuf_Ptr)]; Clear end pointer

	LDR	r1,[r2,#(ML_TxBuf_SaveState - ML_TxBuf_Ptr)]; SaveState ptr
	CMP	r1,#0			; Is there a non-0 SaveState pointer?
	BEQ	ML_CleanUpAbortRx	; No

	; r0 still contains 0
	STR	r0,[r2,#(ML_TxBuf_SaveState - ML_TxBuf_Ptr)]; Unset SaveState
	LDR	r0,=(EC_Error+SS_Kernel+EG_Broken+EO_Link)
					; Set r0 in resumed process to err code
	STR	r0,[r1,#SaveState_r0]	; to indicate tx aborted by break

	LDR	r0,=ROOT_start

	; r0 = `ExecRoot' data structure
	; r1 = pointer to SaveState of process to be restarted
	; r2 = address of ML_TxBuf_Ptr
	ADD	r0,r0,#ExecRoot_queues	; base of priority 0 process queue

	; Add SaveState to end of referenced queue
	LDR	r2,[r0,#ProcessQ_tail]	; load the current tail node
	STR	r1,[r2,#SaveState_next] ; reference the new tail node
	STR	r1,[r0,#ProcessQ_tail]	; update tail pointer

ML_CleanUpAbortRx
	; Abort the current reception (if any) by resetting the FIQ registers
	; to expect a message header byte.
	; r2 = address of ML_TxBuf_Ptr

	LDR	r0,[r2,#(ML_Rx_CurBuf - ML_TxBuf_Ptr)]; Addr of cur buf ptr
	MOV	r1,pc			; Save PSR bits
	TEQP	pc,#(Ibit :OR: FIQmode) ; FIQ mode; IRQs off
	NOP				; Wait for mode change

	LDR	rxBufPtr,[r0]		; Current buffer pointer
	MOV	rxBufEnd,#0		; Waiting for header byte

	TEQP	pc,r1			; Back to previous mode & int state
	NOP				; Wait for mode change 

	; Scan the list of reception requests.
	; Any which are already satisfied revert to being unsatisfied (so
	; their handles remain valid).
	; If a request has a process waiting, then the process is resumed
	; with an error code, and the request becomes invalid (like a timeout).

	LDR	r1,=ML_RxRequests	; Base of table of requests
	LDR	r2,=ML_RxRequestsEnd	; End of table

ML_CleanUpCheckReq
	LDR	r3,[r1,#ML_RxReq_Buf]	; Get buffer pointer
	CMP	r3,#0
	BEQ	ML_CleanUpChkSkip	; Zero => unused slot

	MOV	r3,#0
	STR	r3,[r1,#ML_RxReq_Satisfied]; Clear `satisfied' flag
	MVN	r3,#0			; Set timeout to -1 so that timeout
	STR	r3,[r1,#ML_RxReq_TimeLeft]; code will ignore this request

	; Now safe from timeout code in timer interrupt routine
	LDR	r3,[r1,#ML_RxReq_SaveState]; Get SaveState pointer
	CMP	r3,#0			; Is a process waiting?
	BEQ	ML_CleanUpChkSkip	; No

	; Resume the waiting process.
	; r1: rx request structure
	; r2: end of rx request array

	; Add SaveState to end of referenced queue
	LDR	r0,=ROOT_start		; Executive root structure
	LDR	r4,[r1,#ML_RxReq_SaveState]; Get SaveState pointer
	LDR	r3,=(EC_Error+SS_Kernel+EG_Broken+EO_Link)
					; Set r0 in resumed process to err code
	STR	r3,[r4,#SaveState_r0]	; to indicate rx aborted by break
	LDR	r3,[r0,#(ExecRoot_queues + ProcessQ_tail)]; current tail node
	STR	r4,[r3,#SaveState_next] ; reference the new tail node
	STR	r4,[r0,#(ExecRoot_queues + ProcessQ_tail)]; update tail pointer

	; Mark the reception request as free 
	; r1 points to request structure.
	MOV	r3,#0
	STR	r3,[r1,#ML_RxReq_Buf]	; Zero buffer marks free request slot
	STR	r3,[r1,#ML_RxReq_SaveState]; Clear other fields for safety
	STR	r3,[r1,#ML_RxReq_Satisfied]
	STR	r3,[r1,#ML_RxReq_Handle]
	STR	r3,[r1,#ML_RxReq_MsgType]
	; Timeout already set to -1 above

ML_CleanUpChkSkip
	; r1: rx request structure
	; r2; end of rx request array
	ADD	r1,r1,#ML_RxReq_sizeof	; Move on to next slot
	CMP	r1,r2			; Reached end of table?
	BLT	ML_CleanUpCheckReq	; No

	; Finished aborting transmission and reception

	LDMFD	r13!,{r0,r1,r2,r3,r4,pc}; Restore regs and return	

	]	; end (hercmlink)

		; ------------------------------------------------------------
	|	; middle (hercules)
		; ------------------------------------------------------------

	; Functional prototype `microlink' interrupt is actually from
	; transputer link 0.
	; At present only IRQ is used for this device to reduce the 
	; complexity. (Both tx and rx share an interrupt source, so the
	; FIQ->IRQ downgrade would be messy.)  It would go faster if
	; FIQ were used as well.
ML_Interrupt
	; in:	SVC mode; IRQs disabled; FIQs undefined.
	;	svc_r14: current process return address
	;	svc_r13: FD stack (containing original SVC r13 and r14)
	;	All other registers must be preserved
	;	svc_r14 and svc_r13 should be restored on exit

	STMFD	sp!,{r0,r1,r2,r3,r4,r5}	; Free some work registers

	; Determine which direction of link is interrupting.
	; Note that only one direction is processed per IRQ: this is 
	; probably unimportant.
	MOV	r1,#ml_link_base	; link 0 hardware address

        LDRB  	r0,[r1,#LINK_rstatus]	; rx status register
        TST  	r0,#LINK_intenable      ; are RX IRQs enabled
        TSTNE  	r0,#LINK_data           ; bit set marks ready state
	BNE	ML_Rx_Interrupt		; handle rx interrupt

        LDRB    r0,[r1,#LINK_wstatus]	; tx status register
        TST     r0,#LINK_intenable      ; are TX interrupts enabled
        TST     r0,#LINK_data           ; bit set marks ready state
        BEQ     ML_Exit_IRQ		; no tx interrupt (shouldn't happen)
	; drop through to tx interrupt handler

	; --------------------------------------------------------------------
ML_Tx_Interrupt
	; A transmit interrupt is outstanding on link 0
	; r0, r2, r3, r4, r5 available as work registers
	; r1 = base address of link adapter hardware
	LDR	r2,=ML_TxBuf_Ptr	; Base address of tx variables
	LDR	r3,[r2]			; Buffer pointer in r3
	LDR	r4,[r2,#(ML_TxBuf_End - ML_TxBuf_Ptr)]; End of buffer

	; Test for zero-sized buffer / break tx wanted TBD

	LDRB	r0,[r3],#1		; Get next byte & step pointer
	STRB	r0,[r1,#LINK_write]	; Write byte to link device

	STR	r3,[r2]			; Store updated buffer pointer
	CMP	r3,r4			; Reached end of buffer?
	BLT	ML_Exit_IRQ		; No: exit from IRQ

	; This transmission has finished: disable write interrupt
	MOV	r0,#0
	STRB	r0,[r1,#LINK_wstatus]	; Disable write int in link chip

	; Resume waiting process: first get SaveState pointer (unsetting it
	; so break handling code cannot try to use it again).
	LDR	r1,[r2,#(ML_TxBuf_SaveState - ML_TxBuf_Ptr)]
	MOV	r0,#0
	STR	r0,[r2,#(ML_TxBuf_SaveState - ML_TxBuf_Ptr)]; Unset SaveState
	STR	r0,[r1,#SaveState_r0]	; Set r0 in resumed process to 0
					; to indicate successful transmission

	LDR	r0,=ROOT_start

	; r0 = `ExecRoot' data structure
	; r1 = pointer to SaveState of process to be restarted
	ADD	r3,r0,#ExecRoot_queues	; base of priority 0 process queue

	; Add SaveState to end of referenced queue
	LDR	r2,[r3,#ProcessQ_tail]	; load the current tail node
	STR	r1,[r2,#SaveState_next] ; reference the new tail node
	STR	r1,[r3,#ProcessQ_tail]	; update tail pointer

	B	ML_Exit_IRQ		; Return from IRQ

	; --------------------------------------------------------------------
	; Message reception
	;
ML_Rx_Interrupt
	; A reception interrupt is outstanding on link 0
	; r0, r2, r3, r4, r5 available as work registers
	; r1 = base address of link adapter hardware
	;
	; Each message is received into the same static buffer in the 
	; executive: this buffer's address is passed when any registered
	; message handlers are called. When a client reception request is
	; satisfied, the message is copied into the client's buffer.
	; The overhead of this copy is completely insignificant for all
	; messages except digitiser coordinates arriving at the maximum
	; rate, and these are processed by a handler rather than reception
	; requests anyway.
	;
	LDRB	r0,[r1,#LINK_read]	; Read data byte and clear interrupt

	LDR	r2,=ML_RxBuf_Ptr	; Base address of rx variables
	LDR	r3,[r2]			; Buffer pointer in r3
	LDR	r4,[r2,#(ML_RxBuf_End - ML_RxBuf_Ptr)]; Buffer end

	; While receiving bytes in the middle of a message, the `buffer end'
	; pointer points to the byte after the expected end of the message.
	; This pointer also has two special values (which will always be less
	; than the buffer pointer):
	;   0 when awaiting a header byte, and
	;  -1 when awaiting the length byte of an extended message

	STRB	r0,[r3],#1		; Store in buffer and step pointer
	STR	r3,[r2]			; Store new buf ptr
	CMP	r3,r4			; Received whole message?
	BLT	ML_Exit_IRQ		; More to come: exit
	BEQ	ML_MsgComplete		; This is the final byte of the msg

	; Here r4 (buffer end) must have one of its special values 0 or -1

	CMP	r4,#0			; Is this a header byte?
	BNE	ML_LengthByte		; No: must be length of extended msg

	; We have received a header byte: test its format

	TST	r0,#MLHdr_Long		; Long or extended message?
	BNE	ML_LongOrExtended	; Yes

	; We have a short message header, or a soft break byte (all zero)

	CMP	r0,#0			; Soft break byte?
	BNE	ML_MsgComplete		; No: have complete short (1 byte) msg

	; Have a soft break byte
	!	0,"TODO: handling of received soft break byte"
	B	ML_Exit_IRQ		; Soft break handling TBD

	; Decode header byte of long or extended message, setting end pointer
	;  r0: header byte
	;  r2: points to ML_RxBuf_Ptr (updated)
	;  r3: points to second byte of buffer
ML_LongOrExtended
	AND	r0,r0,#MLHdr_LenCode	; Extract length code
	ADD	r0,r0,#1		; Length code now 1 - 4
	CMP	r0,#3			; 1&2 OK, 3 means 4, 4 for extended msg
	MOVEQ	r0,#4			; Set to 4 if code was 3
	ADDLE	r4,r3,r0		; Set end ptr for long message
	MVNGT	r4,#0			; Set end ptr to -1 for extended msg
	STR	r4,[r2,#(ML_RxBuf_End - ML_RxBuf_Ptr)] ; Save end ptr
	B	ML_Exit_IRQ		; Return from IRQ

ML_LengthByte
	; The received byte is the length byte of an extended message.
	;
	; r0: length byte
	; r2: points to ML_RxBuf_Ptr (updated)
	; r3: points to third byte of buffer
	;
	AND	r0,r0,#MLExt_LenMask	; Extract length
	ADD	r0,r0,#1		; Add 1 to get number of bytes
	ADD	r4,r3,r0		; Set end ptr for extended message
	STR	r4,[r2,#(ML_RxBuf_End - ML_RxBuf_Ptr)] ; Save end ptr
	B	ML_Exit_IRQ		; Return from IRQ

ML_MsgComplete
	; The buffer now contains a complete message.
	; Disable reception interrupts while the message is processed:
	; this should not be necessary, but offers some defence
	; against handlers which enable interrupts.
	; r2: points to ML_RxBuf_Ptr (updated)
	; r4: points to byte after end of message
	;
	MOV	r1,#ml_link_base	; Link 0 hardware address
	MOV	r0,#0
	STRB	r0,[r1,#LINK_rstatus]	; Disable rx int while calling hdlrs

	LDR	r0,=ML_RxBuffer1	; Get start of exec's rx buffer
	MOV	r1,r4			; Put end addr in expected register
	B	ML_ProcessRxMsg		; Use common code to process message
					; This returns to ML_Complete_Exit

ML_Complete_Exit
	; Finished processing complete message.
	; Reset buffer pointer and end for next message, and reenable
	; reception interrupt.
	LDR	r2,=ML_RxBuf_Ptr	; Base address of tx variables
	LDR	r0,=ML_RxBuffer1	; Start of reception buffer
	STR	r0,[r2]			; Reset ptr to start of buffer
	MOV	r0,#0			; Set end ptr to 0 (awaiting msg hdr)
	STR	r0,[r2,#(ML_RxBuf_End - ML_RxBuf_Ptr)]; Store end ptr

	MOV	r1,#ml_link_base	; Link 0 hardware address
	MOV	r0,#LINK_intenable	; Rx interrupt enable bit
	STRB	r0,[r1,#LINK_rstatus]	; Reenable rx interrupt

	B	ML_Exit_IRQ		; Return from interrupt
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------
	; Code common to FP and Hercules microlink drivers

	[	(fpmlink :LOR: hercmlink)
ML_ProcessRxMsg
	; The buffer now contains a complete message.
	; First call any interested message handlers, then see whether
	; the message satisfies any reception request.
	;
	; The break acknowledgement message from the microcontroller (MSQbreak)
	; is treated specially. If a break has just occurred, then all
	; other messages are discarded, and the reply to MSQbreak is 
	; generated here. If there has not been a break, then an MSQbreak
	; message is not expected, so no reply is issued. (However, it is
	; treated as a normal message, so could be received by a handler
	; or reception request.)
	;
	; On entry:
	;  r0: pointer to first byte of message
	;  r1: pointer to byte after last of message
	;  r2,r3,r4,r5 are available as work registers
	;
	; This common code is not a subroutine: it exits by branching
	; to ML_Complete_Exit.

	LDRB	r2,[r0]			; Get header byte of message

	[	(:LNOT: (newbreak))
	LDR	r3,=ML_State		; Get driver's state address
	LDR	r4,[r3]			; Get current state
	CMP	r4,#ML_State_Normal	; If not in normal state, then discard
	BNE	ML_ProcessMSQbreak	; all msgs except MSQbreak
	]

	; r2: message header byte
	TST	r2,#MLHdr_Long		; Is is a long/extended msg?
	MOVNE	r4,#MLHdr_LTypeMask	; Yes: set mask for long msg type field
	MOVEQ	r4,#MLHdr_STypeMask	; No: set mask for short msg type field
	AND	r2,r2,r4		; Extract the type field

	; r2 now contains just the type of the message.
	; r4 holds the mask for the message type
	; Look through the list of message handlers.
	LDR	r3,=ML_MsgHdlrList	; Address of start of list
	LDR	r3,[r3,#&00]		; Pointer to first msg handler struct
	B	ML_MHdlrLoopTest	; Jump to test at end of loop

ML_MHdlrLoop
	; r0: pointer to first byte of message
	; r1: pointer to byte after end of message
	; r2: type of received message (masked)
	; r3: current message handler structure
	; r4: mask for message type

	LDRB	r5,[r3,#ML_MsgHdlr_MsgType]; Get type byte of message handler
	AND	r5,r5,r4		; Mask out the non-type fields
	CMP	r2,r5			; Is this a suitable handler?
	BNE	ML_MHdlrSkip		; This handler doesn't want the msg

	; Have found a message handler to be called.
	; r0, r1, r2, r3, r12 and r14 may get corrupted during a standard PCS
	; call. r9, r10 and r11 need to contain sensible values. 
	; Save all the registers to be on the safe side.
	STMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,dp,sb,fp,ip,lk}
	LDR	dp,[r3,#ML_MsgHdlr_ModTab]; Use registered module table ptr
	MOV	fp,#&00000000		; No stack-frame structure
	; We should be able to use the callers stack-limit register
	; We call func(buf, arg);				
	; r0 already points to buffer containing received msg
	LDR	r1,[r3,#ML_MsgHdlr_Arg] ; Registered argument for call
	LDR	r5,[r3,#ML_MsgHdlr_Func]; Function address
	MOV	lk,pc			; remember return address
	MOV	pc,r5			; call function with current mode/psr
	; Return from message handler
	LDMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,dp,sb,fp,ip,lk}

	[	{FALSE}
	STMFD	sp!,{r0,lk}
	ADR	r0,mltxt1
	BL	local_Output
	LDMFD	sp!,{r0,lk}
	B	mlovr1
mltxt1	=	"ML_MHdlr: returned from handler function\n",&00
	ALIGN
mlovr1
	]	; EOF {boolean}

ML_MHdlrSkip
	LDR	r3,[r3,#ML_MsgHdlr_Next]; Get next handler on list
ML_MHdlrLoopTest
	CMP	r3,#&00000000		; Null msg handler pointer?
	BNE	ML_MHdlrLoop		; No - process next one
	
	; Have called all interested message handlers.
	; Now look through the reception requests to see if this message
	; satisfies any of them.
	; r0: pointer to first byte of message
	; r1: pointer to byte after end of message
	; r2: type of received message

	LDR	r3,=ML_RxRequests	; Base of table of requests
	LDR	r5,=ML_RxRequestsEnd	; End of table

ML_CheckRxReq
	LDR	r4,[r3,#ML_RxReq_Buf]	; Get buffer pointer
	CMP	r4,#0
	BEQ	ML_CheckRxSkip		; Zero => unused slot
	LDR	r4,[r3,#ML_RxReq_Satisfied]; Has req been satisfied already?
	CMP	r4,#0			; Zero if not satisfied
	BNE	ML_CheckRxSkip		; Already satisfied
	LDRB	r4,[r3,#ML_RxReq_MsgType]; Get msg type wanted (already masked)
	CMP	r4,r2			; Same as type of recvd msg?
	BEQ	ML_RxReqFound		; Yes - use this rx request

ML_CheckRxSkip
	ADD	r3,r3,#ML_RxReq_sizeof	; Move on to next slot
	CMP	r3,r5			; Reached end of table?
	BLT	ML_CheckRxReq		; No

	; No rx request found
	B	ML_Complete_Exit	; Finished with this received message

ML_RxReqFound
	; We have found an rx request which wants this message.
	; r0: start of executive's reception buffer
	; r1: pointer to byte after end of message
	; r3: rx request structure
	; r5; end of rx request array

	; Copy the message into the client's buffer.
	LDR	r2,[r3,#ML_RxReq_Buf]	; Get client buffer address

ML_RxMsgCopy
	LDRB	r4,[r0],#1		; Get byte and step pointer
	STRB	r4,[r2],#1		; Store byte and step pointer
	CMP	r0,r1			; Copied whole message?
	BLT	ML_RxMsgCopy		; No

	; If a process is waiting for this reception (i.e. SaveState pointer
	; is non-null), then reschedule it and release this rx request
	; structure.
	; Otherwise, just mark the rx request as being satisfied.

	LDR	r1,[r3,#ML_RxReq_SaveState]; Get SaveState pointer
	CMP	r1,#0			; Is a process waiting?
	BEQ	ML_RxMsgNoSaveState	; No

	; Resume waiting process
	LDR	r0,=ROOT_start

	; r0 = `ExecRoot' data structure
	; r1 = pointer to SaveState of process to be restarted
	; Set r0 in resumed process to 0 to show timeout did not occur
	MOV	r2,#0
	STR	r2,[r1,#SaveState_r0]	; Set saved r0 to 0

	; Add SaveState to end of referenced queue
	ADD	r5,r0,#ExecRoot_queues	; base of priority 0 process queue
	LDR	r2,[r5,#ProcessQ_tail]	; load the current tail node
	STR	r1,[r2,#SaveState_next] ; reference the new tail node
	STR	r1,[r5,#ProcessQ_tail]	; update tail pointer

	; Mark the reception request as free 
	; r3 points to request structure.
	MOV	r0,#0
	STR	r0,[r3,#ML_RxReq_Buf]	; Zero buffer marks free request slot
	STR	r0,[r3,#ML_RxReq_SaveState]; Clear other fields for safety
	STR	r0,[r3,#ML_RxReq_Handle]
	STR	r0,[r3,#ML_RxReq_MsgType]
	MVN	r0,#0
	STR	r0,[r3,#ML_RxReq_TimeLeft]	; Set timeout to -1

ML_ProcessRx_Exit
	B	ML_Complete_Exit	; Finished with this received message

ML_RxMsgNoSaveState
	; r3: rx request structure
	MOV	r0,#1			; Set `Satisfied' flag in req struct
	STR	r0,[r3,#ML_RxReq_Satisfied];
	B	ML_Complete_Exit	; Finished with this received message

	[	(:LNOT: (newbreak))
ML_ProcessMSQbreak
	; The driver is not in `normal' state, and is waiting for an MSQbreak
	; message from the microcontroller after a break.
	; Any other message is a protocol error and is ignored.
	; r2: message header byte
	; r3: address of driver's state word
	; r4: current state

	[	(hercules)
	AND	r5,r2,#MLHdr_STypeMask	; Get message type & format
	CMP	r5,#MLHdr_MSQbreak	; Is it a break acknowledgement?
	BNE	ML_Complete_Exit	; No: ignore it

	; Generate an appropriate ASYbreak reply to the MSQbreak.
	; Since the driver is not in `normal' state, the transmitter must
	; be idle so the single byte message can simply be written to
	; the tx data register. There is no need to enable tx interrupts:
	; the next client transmission will be delayed until this 
	; byte has been sent.

	CMP	r4,#ML_State_SentBreak	; Did this end send the break?
	MOVEQ	r4,#MLHdr_ASYbreak_local; Yes: admit that we did it
	MOVNE	r4,#MLHdr_ASYbreak_remote; No: deny it 
	MOV	r5,#MLI_regs		; Base addr of microlink hardware
	STRB	r4,[r5,#MLI_TXD]	; Write msg byte to link device

	MOV	r4,#ML_State_Normal	; Break protocol finished, so
	STR	r4,[r3]			; revert to normal state
	| ; middle (hercules)
	; No support for break reply on FP
	] ; EOF (hercules)

	B	ML_Complete_Exit	; Exit
	] ; EOF (:LNOT: (newbreak))

	; ---------------------------------------------------------------------
	; Routine called from clock interrupt routine (in loint.s) to see 
	; if any microlink receptions have timed out. It is called every
	; ML_TickInterval ticks of the main system clock.
ML_Timer
	; in:	SVC mode; IRQs disabled; FIQs undefined.
	; r0:      `ExecRoot' data structure
	; svc_r13: FD stack
	; svc_r14: return link
	; r1, r2, r3 and r4 may be corrupted.
	; All other registers must be preserved.

	LDR	r1,=ML_RxRequests	; Base of table of requests
	LDR	r2,=ML_RxRequestsEnd	; End of table

ML_TimerCheckReq
	LDR	r3,[r1,#ML_RxReq_Buf]	; Get buffer pointer
	CMP	r3,#0
	BEQ	ML_TimerChkSkip		; Zero => unused slot
	LDR	r3,[r1,#ML_RxReq_SaveState]; Get SaveState pointer
	CMP	r3,#0			; Is a process waiting?
	BEQ	ML_TimerChkSkip		; No
	LDR	r3,[r1,#ML_RxReq_TimeLeft]; Get remaining timeout
	CMP	r3,#0			; Negative if no timeout
	BLT	ML_TimerChkSkip		; No timeout

	; This request has an active timeout, so decrement it by
	; the number of microseconds between calls of this routine.
	SUBS	r3,r3,#(ML_TickInterval * TickSize); Reduce time left & check
	STRGT	r3,[r1,#ML_RxReq_TimeLeft]; Still +ve: store remaining time
	BGT	ML_TimerChkSkip		;              go on to next req

	; The timeout on the current request has just expired.
	; Resume the waiting process.
	; r0: `ExecRoot' data structure
	; r1: rx request structure
	; r2: end of rx request array

	; Add SaveState to end of referenced queue
	LDR	r4,[r1,#ML_RxReq_SaveState]; Get SaveState pointer
	LDR	r3,=(EC_Error+SS_Kernel+EG_Timeout+EO_Link)
					; Set r0 in resumed process to err code
	STR	r3,[r4,#SaveState_r0]	; to indicate timeout
	LDR	r3,[r0,#(ExecRoot_queues + ProcessQ_tail)]; current tail node
	STR	r4,[r3,#SaveState_next] ; reference the new tail node
	STR	r4,[r0,#(ExecRoot_queues + ProcessQ_tail)]; update tail pointer

	; Mark the reception request as free 
	; r1 points to request structure.
	MOV	r3,#0
	STR	r3,[r1,#ML_RxReq_Buf]	; Zero buffer marks free request slot
	STR	r3,[r1,#ML_RxReq_SaveState]; Clear other fields for safety
	STR	r3,[r1,#ML_RxReq_Handle]
	STR	r3,[r1,#ML_RxReq_MsgType]
	MVN	r3,#0
	STR	r3,[r1,#ML_RxReq_TimeLeft]	; Set timeout to -1

ML_TimerChkSkip
	; r0: `ExecRoot' data structure
	; r1: rx request structure
	; r2; end of rx request array
	ADD	r1,r1,#ML_RxReq_sizeof	; Move on to next slot
	CMP	r1,r2			; Reached end of table?
	BLT	ML_TimerCheckReq	; No

	; Exit from ML_Timer
	MOV	pc,svc_r14		; Return
	]	; end (fpmlink :LOR: hercmlink)

	; ---------------------------------------------------------------------
	; Common exit from microlink IRQ routines.
ML_Exit_IRQ	
	[	((debug2) :LAND: {FALSE})
	MOV	r1,lk
	ADR	r0,mlxtxt1
	BL	local_Output
	MOV	r0,pc
	BL	local_WriteHex8
	ADR	r0,mlxtxt2
	BL	local_Output
	MOV	r0,r1	; really lk
	BL	local_WriteHex8
	ADR	r0,mlxtxt3
	BL	local_Output
	MOV	r0,sp
	BL	local_WriteHex8
	ADR	r0,mlxtxt4
	BL	local_Output
	LDR	r0,[sp,#&00]
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	lk,r1
	B	mlex1
mlxtxt1	=	"ML_Exit_IRQ: pc = &",&00
mlxtxt2	=	" lk = &",&00
mlxtxt3	=	" sp = &",&00
mlxtxt4	=	" sp[0] (lk) = &",&00
	ALIGN
mlex1
	]	; EOF ((debug2) :LAND: {boolean})
	LDMFD	sp!,{r0,r1,r2,r3,r4,r5}	; Restore work regs
	B	Return_From_IRQ		; Continue IRQ processing

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; Power Management functions:
	; ---------------------------
	;
	; TO BE DONE
	;
	;    o  System and User shutdown timeouts (reset by callers)
	;	If timeout is tripped then send LCD off message to uController
	;	and enter IDLE with all IRQs enabled (no system shutdown).
	;	After further timeout without any events perform a full
	;	shutdown, terminating with a power-down request to the
	;	uController. A single "location" can be used for both the
	;	system and user timeouts. The timeout updating/checking will
	;	be performed as part of the normal timer interrupt event.
	;
	;    o  Shut-down request from user. Action depends on hardware
	;       activity. Possibly this should only be equivalent to a
	;	idle time-out (see above) since data transfers may still be
	;	occuring.
	;
        ; ---------------------------------------------------------------------

	[	((activebook) :LAND: (shutdown))
        ; ---------------------------------------------------------------------
	; Power-Fail event
	; ----------------
	; We should shutdown the system as quickly as possible.
	;
PM_PowerFail	; called when PowerFail event is received from the uController
	[	(newbreak)
	; in:	no register conditions
	|
	; in:	r0  : pointer to unsigned byte buffer
	;	r1  : private argument
	]	; EOF (newbreak)
	;	SVC mode; limited stack; IRQs/FIQs undefined
	; out:	does not return directly - calls PM_PowerDown
	;	the code will eventually return directly to our caller
	;	r0  : corrupted
	;	lk  : corrupted
	;	all other registers (and PSR state) will be preserved
	;
	; Notes
	; -----
	; The uController will have started complete DRAM refresh before
	; transmitting this message. The uController will NOT send any more
	; messages after it has transmitted this one. It will be waiting
	; purely for the "PowerDown" message that we send (which requires no
	; acknowledgment).
	;
	[	(debug2)
	STMFD	sp!,{r0,lk}
	ADRL	r0,pmtxt1
	BL	local_Output
	LDMFD	sp!,{r0,lk}
	B	pmovr1
pmtxt1	=	"PM_PowerFail: entered\n",&00
	ALIGN
pmovr1
	]	; EOF (debug2)

	MOV	r0,#&FFFFFFFF		; this is a PowerFail scenario
	; fall through to...
        ; ---------------------------------------------------------------------
	; Power-Down request
	; ------------------

PM_PowerDown	; called when we wish to switch ourselves off (processor off)
	; in:	r0  : &00000000 = normal shutdown; &FFFFFFFF = PowerFail
	;	SVC mode; IRQs/FIQs undefined
	; out:	does not return directly
	;	system is re-started via processor RESET vector
	;		restart code should send (MLHdr_LCDctrl :OR: LCD_on)
	;		wait for acknowledge
	;		check RAM and continue from where we left off
	;
	; NOTE: If we are entered via a PowerFail message reception then we
	;	are OK. However, we may not be able to do this, if we are a
	;	normal thread, performing a shutdown, since our RAM contains
	;	microlink state. This function is in a message reception
	;	thread, and we may not be able to re-start microlink
	;	communications after RESET without resetting the lo-level
	;	microlink handler world.

	; r0 = 0 if normal shutdown, -1 if PowerFail emergency shutdown
	[	(debug2)
	STMFD	sp!,{r0,lk}
	ADR	r0,pdtxt1
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	pdovr1
pdtxt1	=	"PowerDown: r0 = &",&00
	ALIGN
pdovr1
	]	; EOF (debug2)

	; disable processor interrupts
	TEQP	pc,#(SVCmode :OR: INTflags)	; SVC mode; IRQ/FIQ disabled
	; we have not changed processor mode, so we can directly use the
	; mapped registers
	STMFD	sp!,{r0,r1,r2,r3}		; temporary work registers

	; Mark the fact that we are about to start placing data into the
	; "CompleteState" structure.
	LDR	r1,=ROOT_start			; "ExecRoot" structure
	LDR	r2,[r2,#ExecRoot_cstimer]	; current centi-second timer
	LDR	r1,=RestartId1			; first validation word
	STR	r2,[r1,#&00]			; store timer value
	; Ensure the second (tail) ID value is invalid, before we start
	; writing the "CompleteState" information.
	ADD	r3,r2,#&01			; value definately different
	LDR	r1,=RestartId2			; second validation word
	STR	r3,[r1,#&00]			; store different value

	LDR	r0,=RestartState		; reference "CompleteState"
	; store the current (SVC mode) registers in the passed CompleteState
	ADD	r0,r0,#CompleteState_r4		; reference "r4" onwards
	STMIA	r0,{r4-r12}			; store r4 to r12
	LDMFD	sp!,{r4,r5,r6,r7}		; entry r0, r1, r2 and r3
	SUB	r0,r0,#(CompleteState_r4 - CompleteState_r0)
	STMIA	r0,{r4,r5,r6,r7}		; and store in correct slots
	ASSERT	(r13 = sp)			; ensure we are consistent
	[	(CompleteState_r0 <> 0)
	SUB	r0,r0,#CompleteState_r0		; back to start of structure
	]
	STR	sp,[r0,#CompleteState_SVC_r13]	; stack pointer
	STR	lk,[r0,#CompleteState_r15]	; return address
	LDR	r1,=&DEADC0DE			; novelty value
	STR	r1,[r0,#CompleteState_SVC_r14]	; and invalidate the link

	!	0,"TODO: PowerDown: Notify all device drivers of shutdown"
	[	{FALSE}
	... Notify all device drivers ... so they can stop cleanly ...
		... we need a method of calling the device drivers
		... where they will execute quickly, without performing
		... any IRQ/FIQ/DMA driven transfers
		... This will be similar to the actual interrupt calls
		... but we expect a failure/success return. Ideally we should
		... keep calling the handlers until all of them have returned
		... successfully. This action depends on wether this is a
		... PowerFail or a normal PowerDown. Normal PowerDowns can
		... hang around for a couple of seconds waiting for the
		... floppy transfer to finish... or the winchester to be
		... parked, however, PowerFail really requires everthing to
		... be done quickly. We should pass a parameter to the handlers
		... denoting which type of shutdown is being requested.
		... For a normal PowerDown the handler should perform much the
		... same code (except in this limited world) as a DevOperate
		... Close operation on the device would.
		...
		... We can use this opportunity to use the interrupt source
		... bit position as an interrupt handler ID. NOTE: we need to
		... be able to handle multiple active interrupt sources (so
		... treat the ID as a bitmask position) and multiple handlers
		... hanging off the same Hercules source bit (eg. link
		... adaptors and FDC).
		...
		... We should only call the root device driver handler.
		... This will route the call through to the relevant device
		... driver. We should only call device drivers for the bits
		... in the Hercules interrupt source register that are enabled.
	]

	LDR	r1,=ROOT_start
	LDR	r1,[r1,#ExecRoot_shutdownhand]	; address of Helios handler
	TEQ	r1,#&00000000			; is there one?
	BEQ	no_shutdown_function		; NO - so continue shutdown

	!	0,"TODO: PowerDown: Call shutdown handler function"
	[	{FALSE}
	... call the handler function (with a limited PCS world) ...

	... NOTE: we should possibly increase the SVC default stack to ensure
	...	  that there is always enough space for the shutdown code
	...	  (more likely have a dedicated stack area).
	]

no_shutdown_function
	!	0,"TODO: PowerDown: Disable all IRQ/FIQ sources"
	[	{FALSE}
	... Disable all IRQ/FIQ sources
		... Hopefully all the ones that were enabled by external
		... device drivers have already been masked-out by the calls
		... into the drivers above
	]

	!	0,"TODO: PowerDown: Disable processor IRQs/FIQs"
	[	{FALSE}
	... Disable processor IRQs/FIQs	... done above
		... the reason I wanted to do it here is so that we kill off
		... the hardware source, before disabling the interrupts
		... (so that we do not miss any interrupts from the device)
		... What we will probably have to do is disable the processor
		... IRQs/FIQs and then hope that the device drivers can clean
		... up properly (including removing any pending interrupts
		... as they mask out their source).
	]

	; Now that the interrupt sources are all disabled, we can copy the
	; mapped registers for the other processor modes.

	; store the USR mode mapped registers (using processor feature)
	ADD	r1,r0,#CompleteState_USR_r13	; base of USR mapped registers
	STMIA	r1,{r13,r14}^			; store USR mapped registers

	; store the FIQ mode mapped registers
	MOV	r1,#(INTflags :OR: FIQmode)
	TEQP	r1,#&00000000			; FIQ mode; IRQ/FIQ disabled
	NOP					; wait for register mapping
	ADD	r1,r0,#CompleteState_FIQ_r8	; base of FIQ mapped registers
	STMIA	r1,{r8,r9,r10,r11,r12,r13,r14}	; store FIQ mapped registers

	; store the IRQ mode mapped registers
	MOV	r1,#(INTflags :OR: IRQmode)
	TEQP	r1,#&0000000			; IRQ mode; IRQ/FIQ disabled
	NOP					; wait for register mapping
	ADD	r1,r0,#CompleteState_IRQ_r13	; base of IRQ mapped registers
	STMIA	r1,{r13,r14}			; store IRQ mapped registers

	TEQP	pc,#(INTflags :OR: SVCmode)	; SVC mode; IRQ/FIQ disabled

	; Store FastRAM into state buffer
	; We do not need to copy the first word or the last word of the FastRAM
	; since they are corrupted during RESET (by the ROM map-out code)
	MOV	r1,#SRAM_size			; size of the FastRAM
	MOV	r3,#SRAM_base			; reference the FastRAM
	ADD	r4,r0,#CompleteState_FastRAM	; reference the FastRAM store
FastRAMcopy_loop
	LDMIA	r3!,{r5,r6,r7,r8,r9,r10,r11,r12}; load 8 registers
	STMIA	r4!,{r5,r6,r7,r8,r9,r10,r11,r12}; store 8 registers
	SUBS	r1,r1,#(8 * word)		; decrement the counter
	BNE	FastRAMcopy_loop		; go around again
	ASSERT	((SRAM_size :MOD: 8) = 0)	; ensure ldm/stm will work

	; Store all non-soft-copied Hercules registers into state buffer.
	; At the moment we have soft-copies of all the registers we want
	; to preserve in Executive workspace RAM already.
	; -- add hardware state saving code here ------------------------------

	; Store all external hardware registers controlled directly by
	; Hercules. This is currently only the inmos link adaptors.
	; NOTE: At the moment it is possible for Rx bytes to be lost over a
	; 	PowerDown (ie. the reception occurs after processor interrupts
	;	have been disabled, and the RESET that re-starts the processor
	;	will clear the READ ONLY Rx buffer).
	LDR	r1,=LINK0_base
	LDRB	r3,[r1,#LINK_rstatus]		; Rx control/status
	STR	r3,[r0,#CompleteState_LINK0_rstatus]
	LDRB	r3,[r1,#LINK_wstatus]		; Tx control/status
	STR	r3,[r0,#CompleteState_LINK0_wstatus]
	LDR	r1,=LINK1_base
	LDRB	r3,[r1,#LINK_rstatus]		; Rx control/status
	STR	r3,[r0,#CompleteState_LINK1_rstatus]
	LDRB	r3,[r1,#LINK_wstatus]		; Tx control/status
	STR	r3,[r0,#CompleteState_LINK1_wstatus]

	; ---------------------------------------------------------------------
	; Stop all the DMA channels. NOTE: The LCD DMA enable is held in the
	; LCD control register and NOT the DMA routing register.

	MOV	r1,#DMA_regs		; DMA control register	
	MOV	r3,#(DMAchan1_off :OR: DMAchan2_off :OR: DMAchan3_off)
	STR	r3,[r1,#&00]		; and disable the DMA channels

	MOV	r1,#LCD_regs		; LCD registers
	;	       delay          latch pulse type     clock rate
	MOV	r3,#(LCD_DLY_80  :OR:   (&01 :SHL: 4)  :OR:   &09)
	ORR	r3,r3,#(LCD_WAI :OR: LCD_off :OR: LCD_ICP)
	STRB	r3,[r1,#LCD_control]	; and disable the LCD DMA channel

	; Save the DMA registers to "DMAch0base0". NOTE: This code assumes
	; information about the shape and positioning of the processor DMA
	; channels.
	MOV	r1,#DMA_base
	ADD	r3,r0,#CompleteState_DMAch0base0
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; channel 0 and channel 1
	STMIA	r3!,{r4,r5,r6,r7,r8,r9,r10,r11}
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; channel 2 and channel 3
	STMIA	r3!,{r4,r5,r6,r7,r8,r9,r10,r11}

	; Preserve the current MMU mapping state
	MOV	r1,#MMU_base
	ADD	r3,r0,#CompleteState_segment0
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; segments 0..7
	STMIA	r3!,{r4,r5,r6,r7,r8,r9,r10,r11}
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; segments 8..F
	STMIA	r3!,{r4,r5,r6,r7,r8,r9,r10,r11}

	; Highlight the fact that the "CompleteState" has been saved by writing
	; the second (tail) ID value. This is checked along with the first
	; (head) value when restarting the system.
	LDR	r1,=RestartId2			; second validation word
	STR	r2,[r1,#&00]			; store identical value

	[	{TRUE}
	!	0,"********** DEBUGGING DELAY *************"
	; PBond requested this delay before sending the "PowerDown" message so
	; that we could be guaranteed that he has re-enabled his micro-link
	; after he has sent the "break" event. We may still require a delay
	; in this code if it turns out that our shortest PowerFail execution
	; time is quicker than the uControllers maximum delay before enabling
	; his micro-link hardware. In practice however, the uController should
	; be capable of shutting the system down if it does NOT receive the
	; PowerDown message after a suitable time-out.
	MOV	r1,#&00100000	; a largish number
PFdelay_loop
	SUBS	r1,r1,#&01
	BNE	PFdelay_loop
	]	; EOF {boolean}

	; Send a PowerDown request to the uController.
	; WE CANNOT BE GUARANTEED HOW LONG AFTER THE MESSAGE IS SENT BEFORE
	; "vmain" DISAPPEARS. Since we are in a simple defined world (no
	; interrupts) we can poll across this single byte message. This will
	; turn off the "Power" LED.
	MOV	r1,#MLHdr_PowerDown	; send PowerDown message
	MOV	r2,#MLI_regs		; base address of microlink hardware
	STRB	r1,[r2,#MLI_TXD]	; write byte to microlink device

	[	(debug2)
	STMFD	sp!,{r0,lk}
	ADR	r0,pdtxt9
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	pdovr9
pdtxt9	=	"PowerDown: about to IDLE, CompleteState at (r0) &",&00
	ALIGN
pdovr9
	]	; EOF (debug2)

	; Enter the processor IDLE mode.
	; NOTE: no interrupts are enabled so we will never leave IDLE.
	; We enter IDLE here in SVC mode, without the process scheduler
	; flags changed. This is so that we can continue from where we are
	; when the RESET occurs.
	B	IdleProcess

	; ---------------------------------------------------------------------
	; PM_LCDoff
	; ---------
	; Used as part of the gradual shutdown procedure (see the comments
	; associated with resetting the timeout values in "ABClib.h").
	; This will be called from the just before the processor is placed
	; into the IDLE state (NOTE: timer interrupts will be disabled, the
	; uController will have been setup to awake the processor after the
	; stage 2 delay).
	;
PM_LCDoff
	[	{FALSE}
	... send a (MLHdr_LCDctrl :OR: LCDctrl_off) message to the uController
	... wait for acknowledgment (ie. uController refreshing DRAM)
	... disable the LCD display and DMA
	]

	; The uController will perform full DRAM refresh and start flashing the
	; "Power" LED.

	; ---------------------------------------------------------------------
	; PM_LCDon
	; --------
	; If a uController event wakes the processor before the stage 2
        ; timeout, or we have been re-awoken from a true PowerDown, we will
	; need to re-enable LCD DMAs (for DRAM refresh) and turn on the display
	; again.
	;
PM_LCDon
	[	{FALSE}
	... send a (MLHdr_LCDctrl :OR: LCDctrl_on) message to the uController
	... wait for acknowledgement (not really required)
	... enable the LCD DMA and display
	]

	; ---------------------------------------------------------------------
	; Literals used in the above code can be stored here.
	LTORG
	]	; EOF ((activebook) :LAND: (shutdown))

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        LNK     locard.s
