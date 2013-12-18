        SUBT    Link transfer functions                         > hilinktx/s
        ; Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ARM HELIOS Executive: transputer link transfer routines
        ;
        ; Author:               James G Smith
        ; History:      891219  File split from main module "hiexec.s"
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; void TranLinkTx(word size,struct LinkInfo *link,void *buf) ;
        ; Start up the "LinkTxProc" function as a process. This process will
        ; transmit the passed buffer via the link adaptor.
        ;
        ; Note: Since this proto-type does not specify the interrupt handler
        ;       function we can provide a fixed routine in the Executive. This
        ;       can be directly tied to the Executive IRQ and Return_from_IRQ
        ;       setup. Simple checks on interrupt validity can be performed on
        ;       entry to the link adaptor handler. Link adaptor interrupts
        ;       should only be enabled when there is an outstanding request
        ;       through one of these functions. The link adaptor polling
        ;       routines should be locked out while there is an active
        ;       interrupt transfer in progress.
        ;
        ; Construct a SaveState structure
        ; Start the interrupt routine
        ;      (with a reference to our SaveState structure)
        ; "Dispatch" our "SaveState" structure through the scheduler,
        ;      effectively yielding control.
        ; We will not return from "Dispatch" until the interrupt routine
        ;      has completed (and "Resume"d our process), or the interrupt
        ;      was aborted and the aborter "Resume"s our process.
        ;      Our process is referenced with the data passed to the
        ;      interrupt routine.
        ;
TranLinkTx	FnHead
        MOV     ip,sp
        STMFD   sp!,{v1,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        SUB     ip,sp,#SaveState_size
        CMP     ip,sl
        BLLT	__stack_overflow_1
        ]
        SUB     sp,sp,#SaveState_size
        MOV     v1,sp

        ; a1 = size of the buffer in bytes
        ; a2 = pointer to a "LinkInfo" structure
        ; a3 = pointer to the buffer
        ; v1 = address of "SaveState" structure

        MOV     a4,#&00000000
        STR     a4,[v1,#SaveState_next]	; no other referenced SaveStates
        STR     a4,[v1,#SaveState_pri]	; hi-priority process
	[	{TRUE}	; TimedWait support
	STR	a4,[v1,#SaveState_flags]	; state flags unset
	]	; EOF {boolean}

        ; Ensure zero-length transfers do not occur
        CMP     a1,#&00000000
        BEQ     no_tx_transfer

        ; We should poll and write bytes until unable to transfer, before
        ; starting the interrupt routine
        MOV     a2,a1                   ; remember byte count
imm_Tx_transfer
        ; Ensure zero-length transfers do not occur
        CMP     a2,#&00000000
        BEQ     no_tx_transfer          ; transfer completed.

        SWI     exec_LinkWPoll          ; see if able to write a byte
        BCS     no_Tx_character

        LDRB    a1,[a3],#&01
        ; a1 = character read from link adaptor
        SWI     exec_LinkWriteC
        ; NOTE: we are NOT checking any possible error state here

        SUB     a2,a2,#&01              ; and decrement count
        B       imm_Tx_transfer

no_Tx_character
        ; a2 = size of the buffer in bytes
        ; a3 = pointer to the buffer
        ; v1 = address of "SaveState" structure

        ; The "LinkTxProc" started by this function in our world needs to
        ; be an interrupt process, entered only when the relevant device
        ; interrupt occurs. In our world this process will be tied directly
        ; to the interrupt handler. If the interrupt routine finishes
        ; correctly it "Resume"s the process "SaveState" passed into the
        ; function. If the interrupt routine is aborted the abort routine
        ; returns NULL if the transfer had completed, otherwise it returns
        ; the referenced process "SaveState"

        ; we need to pass the interrupt routine:
        ;      buffer address
        ;      amount of data
        ;      SaveState structure for this process

        ; IRQs should be disabled while we define the interrupt transfer
        ; This is required since if IRQs are enabled we may (though
        ; very unlikely) complete the transfer before the process
        ; has been sent through the "Dispatch"er. If this happened
        ; the interrupt routine would attempt to "Resume" a process
        ; which does NOT exist (i.e. the SaveState structure would
        ; NOT contain a valid register set).

        SWI     exec_IntsOff
	[	(speedup)
	MOV	a1,#fast_structure_pointer
	LDR	a1,[a1,#&00]			; a1 = ExecRoot structure addr
	|
        SWI     exec_FindExecRoot
	]	; EOF (speedup)
        LDR     a1,[a1,#ExecRoot_fparea]        ; preserve FP state
        STR     a1,[v1,#SaveState_fparea]

        MOV     a1,a3                   ; buffer address
        ; "a2" number of bytes to transfer (preserved from above)
        MOV     a3,v1                   ; SaveState structure pointer
        SWI     exec_StartLinkTx
        !       0,"Error return from 'exec_startLinkTx' is ignored"
        BVS     invalid_tx_transfer     ; AARRGGHH!! THERE IS A RUNNING LinkTx

        MOV     a1,v1                   ; SaveState structure pointer
        SWI     exec_Scheduler		; and re-schedule this thread
invalid_tx_transfer
        SWI     exec_IntsOn
no_tx_transfer
        LDMEA   fp,{v1,fp,sp,pc}^

        ; ---------------------------------------------------------------------
        ; void TranLinkRx(word size,struct LinkInfo *link,void *buf) ;
        ; Start up the "LinkRxProc" function as a process. This process will
        ; fill the passed buffer with data from the link adaptor under
        ; interrupt.
        ;
TranLinkRx	FnHead
        MOV     ip,sp
        STMFD   sp!,{v1,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        SUB     ip,sp,#SaveState_size
        CMP     ip,sl
        BLLT	__stack_overflow_1
        ]
        SUB     sp,sp,#SaveState_size
        MOV     v1,sp

        ; a1 = size of the buffer in bytes
        ; a2 = pointer to a "LinkInfo" structure
        ; a3 = pointer to the buffer
        ; v1 = address of "SaveState" structure

	[	{FALSE}
	STMFD	sp!,{a1}
	ADR	a1,lrxtxt
	SWI	exec_Output
	LDMFD	sp,{a1}
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	LDMFD	sp!,{a1}
	B	lrxdbg
lrxtxt 	=	"TranLinkRx: count &",&00
lrxdbg
	]	; EOF {debugging}

        MOV     a4,#&00000000
        STR     a4,[v1,#SaveState_next]	; no other referenced SaveStates
        STR     a4,[v1,#SaveState_pri]	; hi-priority process
	[	{TRUE}	; TimedWait support
	STR	a4,[v1,#SaveState_flags]	; clear state flags
	]	; EOF {boolean}

        ; We should poll read bytes until none available before starting
        ; the interrupt routine
        MOV     a2,a1                   ; remember byte count
imm_Rx_transfer
        ; Ensure zero-length transfers do not occur
        CMP     a2,#&00000000
        BEQ     no_rx_transfer          ; transfer completed.

        SWI     exec_LinkRPoll          ; see if any bytes pending
        BCS     no_Rx_character

        SWI     exec_LinkReadC
        ; a1 = character read from link adaptor

        ; NOTE: we are NOT checking any possible error state here

        STRB    a1,[a3],#&01            ; store (incrementing buffer ptr)
        SUB     a2,a2,#&01              ; and decrement count

	[	{FALSE}			; debugging
	STMFD	sp!,{a1}
	ADRL	a1,lrxtxt
	SWI	exec_Output
	LDMFD	sp,{a1}
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	LDMFD	sp!,{a1}
	B	lrxovr
lrxtxt	=	"LinkPol: byte &",&00
	ALIGN
lrxovr
	]	; EOF {debugging}

        B       imm_Rx_transfer

no_Rx_character
        ; a2 = size of the buffer in bytes
        ; a3 = pointer to the buffer
        ; v1 = address of "SaveState" structure

        SWI     exec_IntsOff
	[	(speedup)
	MOV	a1,#fast_structure_pointer
	LDR	a1,[a1,#&00]			; a1 = ExecRoot structure addr
	|
        SWI     exec_FindExecRoot
	]	; EOF (speedup)
        LDR     a1,[a1,#ExecRoot_fparea]        ; preserve FP state
        STR     a1,[v1,#SaveState_fparea]

        MOV     a1,a3                   ; buffer address
        MOV     a3,v1                   ; SaveState structure pointer
        SWI     exec_StartLinkRx
        !       0,"Error return from 'exec_startLinkRx' is ignored"
        BVS     invalid_rx_transfer     ; AARRGGHH!! THERE IS A RUNNING LinkRx

        MOV     a1,v1                   ; SaveState structure pointer
        SWI     exec_Scheduler		; and re-schedule this thread
invalid_rx_transfer
        SWI     exec_IntsOn
no_rx_transfer
        LDMEA   fp,{v1,fp,sp,pc}^

        ; ---------------------------------------------------------------------
        ; SaveState *AbortTranLinkTx(struct LinkInfo *link) ;
        ; Abandon the transmitting process.
        ;
AbortTranLinkTx	FnHead
        MOV     ip,lk
        ; terminate the current interrupt driven data transfer
        SWI     exec_AbortLinkTx
        ; a1 = buffer address of next byte to be written
        ; a2 = number of bytes remaining to be written
        ; a3 = "SaveState *" for the process to be resumed

        ; If ("a2" == &00000000) then all the bytes have been transferred
        ; (i.e. there is NO interrupt transfer to abort). In this case we
        ; must return NULL as the process SaveState, since the interrupt
        ; handler will have resumed the process.

        CMP     a2,#&00000000
        MOVEQ   a1,#&00000000           ; NULL process SaveState
        MOVNE   a1,a3                   ; real process SaveState

        MOVS    pc,ip

        ; ---------------------------------------------------------------------
        ; SaveState *AbortTranLinkRx(struct LinkInfo *link) ;
        ; Abandon the receiving process.
        ;
AbortTranLinkRx	FnHead
        MOV     ip,lk

        ; terminate the current interrupt driven data transfer
        SWI     exec_AbortLinkRx
        ; a1 = buffer address of next byte to be read
        ; a2 = number of bytes remaining to be read
        ; a3 = "SaveState *" for the process to be resumed

        ; If ("a2" == &00000000) then all the bytes have been transferred
        ; (i.e. there is NO interrupt transfer to abort). In this case we
        ; must return NULL as the process SaveState, since the interrupt
        ; handler will have resumed the process.

        CMP     a2,#&00000000
        MOVEQ   a1,#&00000000           ; NULL process SaveState
        MOVNE   a1,a3                   ; real process SaveState

        MOVS    pc,ip

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	END	; of the hi-level Executive code
