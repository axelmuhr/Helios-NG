        SUBT Executive SWI handler and routines                      > loswi5/s
        ;    Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; -- Constant ---------------------------------------------------------
        ; ---------------------------------------------------------------------

ML_RxSerShift	*	16		; Left shift of rx handle serial field

        ; ---------------------------------------------------------------------
        ; -- Microlink SWI routines -------------------------------------------
        ; ---------------------------------------------------------------------
	; IRQs are disabled for the duration of these SWIs to protect against
	; state changes caused by incoming (hard or soft) breaks. It would
	; be sufficient to mask out just the microlink rx and break IRQs,
	; but this would take longer, and would result in IRQs being disabled
	; for a similar period anyway.

        ; ---------------------------------------------------------------------
        ; -- exec_ML_StartTx --------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Initiate transmission of microlink message to microcontroller.
	; The transmission is performed under IRQ.
	; The calling process suspends itself using the SaveState structure
	; supplied as a parameter here; the IRQ handler can then
	; Resume the process when transmission is complete.
	;
        ; in:  SVC mode; IRQs undefined; FIQs undefined
	;      r0  = pointer to buffer containing message
	;      r1  = address of SaveState structure
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
	;
        ; out: processor mode preserved.
	;      r0, r1 preserved
        ;      V set if this transmission could not be started because
	;      another one was already in progress.
	;
	; A microlink transmission is controlled by the contents of the
	; locations "ML_TxBuf_End" and "ML_TxBuf_Ptr". If these
	; are not equal, another transfer is already in progress.
	;
code_exec_ML_StartTx
	MOV	r11,link		; Save return link
	BL	local_disableIRQs	; Disable IRQs during SWI routine
	MOV	link,r11		; Restore return link

        LDR     r11,=ML_TxBuf_Ptr	; Get address of tx variables
	LDR	r12,[r11,#(ML_State - ML_TxBuf_Ptr)]; Get driver state
	CMP	r12,#ML_State_Normal	; In normal running state?

	; Load and check buffer pointers only if in `normal' state
        LDREQ   r12,[r11,#(ML_TxBuf_End - ML_TxBuf_Ptr)]; End ptr
	LDREQ	r11,[r11]		; Start ptr
        CMPEQ   r12,r11		        ; Are the pointers equal?

        ; "NE" then not in `normal' state or a transmission is outstanding 
        LDMNEFD sp!,{r11,r12}           ; Recover entry state
        ORRNES  pc,link,#Vbit           ; and return with V set

	; The transmission can go ahead
        LDR     r11,=ML_TxBuf_Ptr
	STR	r0,[r11]		; Store buffer & SaveState addresses
	STR	r1,[r11,#(ML_TxBuf_SaveState - ML_TxBuf_Ptr)]

	; Work out transfer length from message header
	LDRB	r12,[r0]		; Get header byte
	TST	r12,#MLHdr_Long		; Is is short or long/extended?
	MOVEQ	r12,#1			; Short: length is 1 byte
	BEQ	ML_StartTx_GotLen	; Short: start transfer

	AND	r12,r12,#MLHdr_LenCode	; Extract length code
	ADD	r12,r12,#2		; Add two as 0,1 mean total len 2,3
	CMP	r12,#4			; 4 (was 2) means total len 5
	MOVEQ	r12,#5			; Was 4: change to 5
	BLE	ML_StartTx_GotLen	; Have dealt with total lens 2, 3 & 5

	; This is an extended message with the length in the second byte.
	LDRB	r12,[r0,#1]		; Get length byte
	AND	r12,r12,#MLExt_LenMask	; Extract size field
	ADD	r12,r12,#3		; Total len is 3 more

ML_StartTx_GotLen
	; r0  = buffer address
	; r1  = SaveState pointer
	; r11 = address of ML_TxBuf_Ptr
	; r12 = number of bytes to transmit

	; Save buffer start and end addresses for interrupt routine
	STR	r0,[r11]		; buffer start
	ADD	r12,r12,r0		; calculate buffer end
	STR	r12,[r11,#(ML_TxBuf_End - ML_TxBuf_Ptr)]; store end

	[	(hercules)
	; Hercules microlink interface: enable microlink tx IRQ
	STMFD	sp!,{r0,link}		; Save regs about to be corrupted
	MOV	r0,#INT_MTX		; Bit in IRQ register
	BL	local_EnableIRQ		; Enable microlink tx IRQ
	LDMFD	sp!,{r0,link}		; Restore regs
	|	; middle (hercules)
	; Functional prototype: use link adapter
        MOV     r11,#ml_link_base	; r11 = link adaptor base address
        MOV     r12,#LINK_intenable     ; enable write interrupts
        STRB    r12,[r11,#LINK_wstatus]
        ]	; EOF (hercules)

        LDMFD   sp!,{r11,r12}		; Restore work registers
        BICS    pc,link,#Vbit           ; return V clear

        ; ---------------------------------------------------------------------
        ; -- exec_ML_SetUpRx --------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Set up a reception buffer for a microlink message.
	;
        ; in:  SVC mode; IRQs undefined; FIQs undefined
	;      r0  = pointer to buffer for message
	;      r1  = type of message to be received
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
	;
        ; out: processor mode preserved
        ;      V clear: reception set up
	;      r0  = handle to be passed to ML_WaitForRx
	;      V set: no space left to record this reception
	;
code_exec_ML_SetUpRx
	STMFD	sp!,{r2}		; Need another work reg

	MOV	r11,link		; Save return link
	BL	local_disableIRQs	; Disable IRQs during SWI routine
	MOV	link,r11		; Restore return link

        LDR     r12,=ML_State		; Get address of state variable
	LDR	r12,[r12]		; Get driver state
	CMP	r12,#ML_State_Normal	; In normal running state?
        LDMNEFD sp!,{r11,r12}           ; No: recover entry state
        ORRNES  pc,link,#Vbit           ;     and return with V set

	; The outstanding requests are held in an array. First look for
	; a free slot (which has the buffer field set to zero).
	; Note that this means that requests for the same message type
	; are not necessarily satisfied in the order they were submitted.
	; This should not be important.

	LDR	r11,=ML_RxRequests	; Base of table of requests
	LDR	r2,=ML_RxRequestsEnd	; End of table

ML_SetUpRxLoop
	LDR	r12,[r11,#ML_RxReq_Buf]	; Get buffer pointer
	CMP	r12,#0
	BEQ	ML_SetUpFree		; Zero => free slot
	ADD	r11,r11,#ML_RxReq_sizeof; Move on to next slot
	CMP	r11,r2			; Reached end of table?
	BLT	ML_SetUpRxLoop		; No

	; No free slots to record request, so return with V set
	LDMFD	sp!,{r2,r11,r12}	; Restore work registers
        ORRS    pc,link,#Vbit           ; return V set

ML_SetUpFree
	; r11 = address of free rx request slot
	; Store the buffer field last, so the interrupt routine will not
	; pick a half-completed slot.
	; Extract just the type field from the type byte supplied by the caller
	TST	r1,#MLHdr_Long		; Long/extended message type?
	ANDNE	r1,r1,#MLHdr_LTypeMask	; Yes: extract long/extended type field
	ANDEQ	r1,r1,#MLHdr_STypeMask	; No:  extract short msg type field
	STR	r1,[r11,#ML_RxReq_MsgType]; Store (masked) type field first
	STR	r0,[r11,#ML_RxReq_Buf]	; and buffer address last

	; Clear other structure fields
	MOV	r0,#0
	STR	r0,[r11,#ML_RxReq_Satisfied]; Request not satisfied yet
	STR	r0,[r11,#ML_RxReq_SaveState]; No SaveState pointer yet

	; The handle returned is just the byte offset of this slot in the
	; table, with a serial number in the high-order bits as a validity
	; check.
	LDR	r12,=ML_SerialNo	; Get address of serial no. word
	LDR	r0,[r12]		; Get current serial number
	ADD	r0,r0,#1		; Increment it
	STR	r0,[r12]		; Put it back

	LDR	r12,=ML_RxRequests	; Get table base address
	SUB	r12,r11,r12		; Offset of slot in table
	ORR	r0,r12,r0,LSL #ML_RxSerShift; Put serial no. in top part
	BIC	r0,r0,#&80000000	; Ensure handle not negative
	STR	r0,[r11,#ML_RxReq_Handle]; Store handle in request entry

	; Make sure that the receiver interrupt is enabled
	; r0 = handle to be kept for result
	
        [       (hercules)
	; Hercules support TBD
        |
	; Functional prototype: use link adapter
        MOV     r11,#ml_link_base	; r11 = link adaptor base address
        MOV     r12,#LINK_intenable     ; enable read interrupts
        STRB    r12,[r11,#LINK_rstatus]
        ]	; EOF (hercules)

	; Return with V clear and handle in r0
        LDMFD   sp!,{r2,r11,r12}	; Restore work registers
        BICS    pc,link,#Vbit           ; return V clear

        ; ---------------------------------------------------------------------
        ; -- exec_ML_ResumeAfterRx --------------------------------------------
        ; ---------------------------------------------------------------------
        ; Record a SaveState pointer which the reception interrupt routine
	; will use to resume a waiting process after a reception completes.
	;
        ; in:  SVC mode; IRQs undefined; FIQs undefined
	;      (In practice, IRQs will be off, as the caller must
	;      indivisibly call ML_PollRx and wait on the SaveState.)
	;      r0  = handle returned by ML_SetUpRx
	;      r1  = timeout in microseconds (-1 if no timeout)
	;      r2  = pointer to SaveState struct of waiting process
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
	;
        ; out: processor mode preserved
        ;      V clear: SaveState recorded for interrupt routine
	;      V set:   invalid handle
	;
code_exec_ML_ResumeAfterRx
	STMFD	sp!,{link}		; Save return link
	BL	local_disableIRQs	; Disable IRQs during SWI routine
	BL	ML_FindRxReq		; Get rx request in r11
	LDMFD	sp!,{link}		; Restore return link
	LDMVSFD	sp!,{r11,r12}		; V set => bad handle: restore regs
	ORRVSS	pc,link,#Vbit		; V set: return with V set

	; Note that it is unnecessary to check the driver's state in this call

	STR	r2,[r11,#ML_RxReq_SaveState]; Store SaveState ptr in request
	STR	r1,[r11,#ML_RxReq_TimeLeft] ; Store timeout in request

	LDMFD	sp!,{r11,r12}		; restore work regs
	BICS	pc,link,#Vbit		; return with V clear

        ; ---------------------------------------------------------------------
        ; -- exec_ML_PollRx ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Test whether a reception request has been satisfied.
	;
        ; in:  SVC mode; IRQs undefined; FIQs undefined
	;      (In practice, IRQs will be off, as the caller must
	;      indivisibly call ML_PollRx and wait on the SaveState.)
	;      r0  = handle returned by ML_SetUpRx
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
	;
        ; out: processor mode preserved
        ;      V clear:   Reception complete; handle now invalid
	;      V set:     Reception not complete
	;      V & C set: Invalid handle
	;
code_exec_ML_PollRx
	STMFD	sp!,{link}		; Save return link
	BL	local_disableIRQs	; Disable IRQs during SWI routine
	BL	ML_FindRxReq		; Get rx request in r11
	LDMFD	sp!,{link}		; Restore return link

	LDMVSFD	sp!,{r11,r12}		; V set => bad handle: restore regs
	ORRVSS	pc,link,#(Vbit :OR: Cbit); V set: return with V & C set

	; Note that it is unnecessary to check the driver's state in this call

	BIC	link,link,#(Cbit :OR: Vbit); Clear C & V for exit
	LDR	r12,[r11,#ML_RxReq_Satisfied]; Get `satisfied' field
	CMP	r12,#0			; Has request been satisfied?
	ORREQ	link,link,#Vbit		; Set V bit for exit if not
	BEQ	ML_PollRx_Exit

	; Request has been satisfied, so release the request structure
	; by setting its `buffer' field to zero.
	MOV	r12,#0
	STR	r12,[r11,#ML_RxReq_MsgType]; Clear other fields first
	STR	r12,[r11,#ML_RxReq_Handle];
	STR	r12,[r11,#ML_RxReq_SaveState];
	STR	r12,[r11,#ML_RxReq_Satisfied];
	STR	r12,[r11,#ML_RxReq_Buf]	; Do this last: frees structure

ML_PollRx_Exit
	LDMFD	sp!,{r11,r12}		; Restore work regs
	MOVS	pc,link			; Exit with V & C set as above

        ; ---------------------------------------------------------------------
        ; -- exec_ML_RegisterHandler ------------------------------------------
        ; ---------------------------------------------------------------------
        ; Register a message handler function
	;
        ; in:  SVC mode; IRQs undefined; FIQs undefined
	;      r0  = address of ML_MsgHandler structure
	;      r9  = caller's module table pointer
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
	;
        ; out: processor mode preserved
        ;      V clear:   Handler installed
	;      V set:     Handler not installed (never happens in fact...)
	;
local_ML_RegisterHandler	; internal entry
	STMFD	sp!,{r11,r12}	; stack the work registers
code_exec_ML_RegisterHandler
	MOV	r11,link		; Save return link
	BL	local_disableIRQs	; Disable IRQs during SWI routine
	MOV	link,r11		; Restore return link

	; Note that it is unnecessary to check the driver's state in this call

	LDR	r11,=ML_MsgHdlrList	; Get start of handler list
	TEQP	pc,#(Ibit :OR: SVCmode)	; Disable IRQs
	LDR	r12,[r11,#&00]		; Address of first handler
	STR	r12,[r0,#ML_MsgHdlr_Next]; Store in `next' field of new hdlr
	STR	r0,[r11,#&00]		; Put new handler at head of list
	STR	dp,[r0,#ML_MsgHdlr_ModTab]; Store module table pointer in hdlr

	LDMFD	sp!,{r11,r12}		; Restore work regs
	BICS	pc,link,#Vbit		; Return with V clear, old IRQ state

        ; ---------------------------------------------------------------------
        ; -- exec_ML_DetachHandler --------------------------------------------
        ; ---------------------------------------------------------------------
        ; Remove a previously registered message handler function
	;
        ; in:  SVC mode; IRQs undefined; FIQs undefined
	;      r0  = address of ML_MsgHandler structure registered earlier
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
	;
        ; out: processor mode preserved
        ;      V clear:   Handler removed
	;      V set:     Handler not found
	;
code_exec_ML_DetachHandler
	MOV	r11,link		; Save return link
	BL	local_disableIRQs	; Disable IRQs during SWI routine
	MOV	link,r11		; Restore return link

	; Note that it is unnecessary to check the driver's state in this call

	LDR	r11,=ML_MsgHdlrList	; Get start of handler list
	TEQP	pc,#(Ibit :OR: SVCmode)	; Disable IRQs while we search
	B	ML_DetLoopEnd		; Jump to test at end of loop

	; In this loop, r12 points to the current handler structure
	;		r11 points to the pointer to the current structure
ML_DetachLoop
	CMP	r12,r0			; Is this the sought handler?
	BEQ	ML_DetFound		; Yes: dequeue it
	ADD	r11,r12,#ML_MsgHdlr_Next; Get address of `next' field
ML_DetLoopEnd
	LDR	r12,[r11]		; Get address of next handler
	CMP	r12,#0			; End of list?
	BNE	ML_DetachLoop		; No - go round again

	; Handler not found
	LDMFD	sp!,{r11,r12}		; Restore work regs
	ORRS	pc,link,#Vbit		; Return with V set, old IRQ state

	; Handler found, so remove it from the list.
ML_DetFound
	LDR	r12,[r12,#ML_MsgHdlr_Next]; Get address of next handler
	STR	r12,[r11]		; Remove current handler from list
	LDMFD	sp!,{r11,r12}		; Restore work regs
	BICS	pc,link,#Vbit		; Return with V clear, old IRQ state

        ; ---------------------------------------------------------------------
        ; -- exec_ML_Reset ----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Reset the communication channel with the microcontroller
	;
        ; in:  SVC mode; IRQs undefined; FIQs undefined
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
	;
        ; out: processor mode preserved
	;      This SWI returns when the microlink break packet has been
	;      sent (i.e. the reset has been initiated). The rest of the
	;      reset protocol will happen asynchronously.	
	;
	; The current state of the driver is not checked, as this call 
	; must be allowed to work in all circumstances.
	;
code_exec_ML_Reset
	[ 	(hercules)
	[	(hercmlink)
	; Transmit a microlink break. 
	; The microlink interface is disabled during transmission, as this
	; is the only way to be sure that the next packet received was sent
	; after the other end had seen the break. We have to poll for
	; completion of transmission as the disabled interface will not
	; produce a tx interrupt: however transmission will take only a
	; few tens of microseconds.

	; Disable the microlink to prevent further interrupts
	MOV	r11,#MLI_regs		; Base address of microlink hardware
	MOV	r12,#MLI_ICP		; Disable, internal clk
	STRB	r12,[r11,#MLI_CON]	; Write control byte

	; Update the driver's state to prevent any new requests being
	; accepted during the break sequence.
	LDR	r11,=ML_State		; Get address of driver state word
	MOV	r12,#ML_State_SentBreak ; Record that break was sent from here
	STR	r12,[r11]		; Update state word

	; Start sending the break with microlink still disabled
	MOV	r11,#MLI_regs		; Base address of microlink hardware
	MOV	r12,#(MLI_TXB :OR: MLI_ICP); Disable, internal clk, send break
	STRB	r12,[r11,#MLI_CON]	; Write control byte

	; Clean up any microlink transmissions and receptions which were
	; in progress at this end (and disable tx IRQs)

	MOV	r12,link		; Preserve the link
	BL	ML_CleanUp		; Abort transmissions and receptions
	MOV	link,r12		; Restore the link

	; Busy wait for the break packet transmission to complete.

ML_BreakTx_Wait
	MOV	r11,#MLI_regs		; Base address of microlink hardware
	LDRB	r12,[r11,#MLI_STA]	; Read status register
	TST	r12,#MLI_TGB		; "Transmitting break" flag
	BNE	ML_BreakTx_Wait		; Loop while still transmitting

	; Have finished sending break packet, so reenable the microlink
	; and return.

	MOV	r12,#(MLI_ENA :OR: MLI_ICP); Enable, internal clock
	STRB	r12,[r11,#MLI_CON]	; Update control register

	]	; EOF (hercmlink)
	| 	; middle (hercules)

	LDR	r11,=ML_SendSoftBreak	; Address of flag for int routine
	MOV	r12,#1
	STR	r12,[r11]		; Tell tx int routine to send break

	; Functional prototype: make sure link tx interrupt is enabled.
        MOV     r11,#ml_link_base	; r11 = link adaptor base address
        MOV     r12,#LINK_intenable     ; enable write interrupts
        STRB    r12,[r11,#LINK_wstatus]
	]	; EOF (hercules)

	LDMFD	sp!,{r11,r12}		; Restore work regs
	MOVS	pc,link			; Return

        ; ---------------------------------------------------------------------
        ; -- ML_FindRxReq -----------------------------------------------------
        ; ---------------------------------------------------------------------
	; Internal routine used to find a reception request from its handle
	;
        ; in:  SVC mode; IRQs undefined; FIQs undefined
	;      r0  = handle returned by ML_SetUpRx
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
	;
        ; out: processor mode preserved
	;      r12 corrupted
        ;      V clear:   Valid handle; request found
	;      r11 = reception request address if V clear
	;      V set:     Invalid handle
	;
ML_FindRxReq
	; The handle consists of a serial number in the high bits and
	; the byte offset into the table in the low bits.
	MOV	r12,r0,LSL #(32 - ML_RxSerShift); Shift out serial number
	LDR	r11,=(ML_RxRequests)	; Get base of request table
	ADD	r11,r11,r12,LSR #(32 - ML_RxSerShift); Get address of request
 	LDR	r12,[r11,#ML_RxReq_Handle]; Get handle stored in request
	CMP	r0,r12			; Check against supplied handle
	ORRNES	pc,link,#Vbit		; Bad handle - return with V set

	; Handle OK, so check that the request is in use (buf ptr not zero)
	LDR	r12,[r11,#ML_RxReq_Buf]	; Request buffer pointer
	CMP	r12,#0			; Is buf ptr zero?
	BICNES	pc,link,#Vbit		; OK - return with V clear
	ORRS	pc,link,#Vbit		; Bad handle - return with V set

        ; ---------------------------------------------------------------------
        LNK     loswidbg.s
