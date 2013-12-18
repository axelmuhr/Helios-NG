        SUBT    Executive interrupt handling                    > loint/s
        ; Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ARM HELIOS Executive (interrupt handlers)
        ;
        ; Author:               James G Smith
        ; History:      900810  Split from the main source file "loexec.s"
	;		901219	Added microlink simulation for FP
        ;
	!	0,"TODO: Spend time optimising IRQ handler register usage"
	;	It could make a lot of difference to the system performance
	;	shaving a few instructions of the IRQ thread (since its going
	;	to executing 200-300 times a second (100Hz timer,
	;	100samples/sec digitiser, all the other devices hanging off
	;	the standard Active Book)).
	;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
exec_IRQ
        ; in:   IRQ mode; FIQ preserved from interrupted process; IRQ disabled
        ;       lk : processor mode and (return address + 4)
        ;       All registers (other than "lk" and "pc") are preserved
        ;
        ; We wish to enter SVC mode as early as possible (executing IRQ
        ; handlers in SVC) since we can then enable IRQs quickly. To ensure we
        ; do NOT destroy the SVC thread (if any) we must preserve the current
        ; SVC r13 and r14 before over_loading the registers with our stack
	; and return address. When we have completed our SVC thread we should
	; restore the entry SVC state.
        ;
        ; Externally called interrupt handlers must be called with IRQs
        ; disabled since only they know how to clear their IRQ state. This
	; will affect interrupt latency dramatically over that offered by the
	; Executive based interrupt handlers.
	;
	; So that the IRQ handler system can be re-entered we must NOT use
	; the same stack as the current IRQ handler thread (which is executing
	; in SVC mode).
	;
	; NOTE: External IRQ handlers should not really enable IRQs until
	;	they are sure that another IRQ for them will NOT be generated
	;	in the time taken to exit the IRQ handler. They should however
	;	be capable of being re-entered, just that we may run out of
	;	IRQ stack if it recurses too deep.
	;

	!	0,"TODO: Calculate the amount of stack IRQ handlers can use"
	; Since we allow the SVC threads that are generated from an IRQ thread
	; to be interrupted (if they don't mind being re-entrant), plus we will
	; perform simple prioritisation (based on the IRQ source). This means
	; we must be capable of supporting at least one outstanding SVC thread
	; on each of the IRQ sources. We should place a limit on the stack
	; usage of device drivers, and ensure we have a suitable multiple of
        ; this amount of IRQ stack available.
	;
	;
        ; Modify the return address so that it is a true value
        SUB     irq_r14,irq_r14,#&04    ; preserving PSR and mode
	LDR	irq_r13,=top_IRQ_entry	; always setup on entry
	; NOTE: NO MAPPED REGISTERS should be stored on this entry stack
        STMFD   irq_r13!,{r0,r1,r2,r3,r4}
	ASSERT	((5 * &04) = size_IRQ_entry)

        ; search for the interrupt source
	[	(hercules)
        ; Hercules interrupt structure
        ; ----------------------------
        ; r0 and r1 are available as temporary registers
        ; The only expected IRQ sources here are:
	;
        ;   Timer 0         --> index 0
        ;   Link  1         --> index 1
	;   Microlink rx    --> index 2
	;   Microlink tx    --> index 3
	;   Microlink break --> index 4
	;
	MOV	r1,#INT_regs
	LDR	r0,[r1,#IRQ_control]	; IRQ request register (after mask)

	LDR	r1,=IRQ_vectors
	TST	r0,#INT_TIM		; timer interrupt
	BNE	timer_irq_found
	ADD	r1,r1,#&04		; step onto the next vector (index 1)
	TST	r0,#LINK_interrupt	; transputer link interrupt
	[	(hercmlink)
	BNE	link_irq_found
	ADD	r1,r1,#&04		; step onto the next vector (index 2)
	TST	r0,#INT_MRX		; microlink reception interrupt
	BNE	link_irq_found
	ADD	r1,r1,#&04		; step onto the next vector (index 3)
	TST	r0,#INT_MTX		; microlink transmission interrupt
	BNE	link_irq_found
	ADD	r1,r1,#&04		; step onto the next vector (index 4)
	TST	r0,#INT_MBK		; microlink break interrupt
	]	; end (hercmlink)
	BEQ	no_interrupt
link_irq_found
	; and fall through to process the relevant vector...
	|	; middle (hercules)
        ; AB functional prototype interrupt structure
        ; -------------------------------------------
        ; r0 and r1 are available as temporary registers
        ; The only expected IRQ sources here are:
	;
        ;   Timer 0 --> index 0
        ;   Link 1  --> index 1
	;   Link 0  --> index 2 (used as microlink substitute on FP)

        LDR     r1,=int_source          ; not 8bit constant for this board
        LDRB    r0,[r1,#0]              ; get interrupt source reg

        LDR     r1,=IRQ_vectors         ; base of table

        TST     r0,#intsrc_timer        ; timer interrupt request bit
        BNE     timer_irq_found         ; hi-level, interrupt asserted

        ; check for link1 IRQ
        ADD     r1,r1,#&04              ; step onto the next vector (index 1)
        ; we need to check the interrupt sources directly
        MOV     r0,#LINK0_base		; hardware base address
        LDRB    r0,[r0,#LINK_wstatus]
        TST     r0,#LINK_intenable      ; are TX interrupts enabled
        TSTNE   r0,#LINK_data           ; bit set marks ready state
        BNE     link_irq_found          ; we have a write interrupt

        MOV     r0,#LINK0_base          ; hardware base address
        LDRB    r0,[r0,#LINK_rstatus]
        TST     r0,#LINK_intenable      ; are RX interrupts enabled
        TSTNE   r0,#LINK_data           ; bit set marks data presence
        BNE     link_irq_found          ; we have a read interrupt

	[	(fpmlink)
        ; check for link0 IRQ (used as microlink substitute on FP)
        ADD     r1,r1,#&04              ; step onto the next vector (index 2)
        ; we need to check the interrupt sources directly
        MOV     r0,#ml_link_base     	; hardware base address
        LDRB    r0,[r0,#LINK_wstatus]
        TST     r0,#LINK_intenable      ; are TX interrupts enabled
        TSTNE   r0,#LINK_data           ; bit set marks ready state
        BNE     link_irq_found          ; we have a write interrupt

        MOV     r0,#ml_link_base        ; hardware base address
        LDRB    r0,[r0,#LINK_rstatus]
        TST     r0,#LINK_intenable      ; are RX interrupts enabled
        TSTNE   r0,#LINK_data           ; bit set marks data presence
        BEQ     no_interrupt            ; interrupt is active high
	|
	B	no_interrupt
	]	; EOF (fpmlink)
        ; and fall through, since we have a read interrupt
link_irq_found
	]	; EOF (hercules)
timer_irq_found
        ; and fall through (index 0 is at the base address)
        ; r0 = undefined
        ; r1 = vector address
        MOV     r0,irq_r14
        ORR     irq_r14,r1,#(Ibit :OR: SVCmode) ; SVC mode, IRQs disabled
        AND     r1,r0,#Fbit                     ; get callers FIQ state
        ORR     irq_r14,irq_r14,r1              ; and add in callers FIQ state
        ; r0  = code return address
        ; r1  = callers FIQ status
        ; r14 = IRQ vector address and new PSR status (SVC mode)

        ; We must ensure we keep IRQs disabled during the following
        ; processor mode changes (and preserve the entry FIQ state).
	; The reason we switch into SVC mode, and then back into IRQ mode
        ; is to setup the SVC world, and then use the IRQ r14 as vector
	; address for the desired handler.

        TEQP    pc,irq_r14              ; SVC mode, IRQs disabled
        NOP                             ; and wait for registers to be mapped

	LDR	r2,=entered_IRQ
	LDR	r3,[r2,#&00]		; current active IRQ thread count
	TEQ	r3,#&00000000		; are we the first?
	LDREQ	r4,=top_IRQ_stack	; only thread, so new stack for SVC
	MOVNE	r4,svc_r13		; otherwise preserve current SVC stack
	ADD	r3,r3,#&01		; update the active count
	STR	r3,[r2,#&00]		; and store back into its location
	; continue SVC thread setup
	STMFD   r4!,{svc_r13,svc_r14}	; preserve real SVC thread information
	MOV	svc_r13,r4		; and index the stack for SVC mode
        MOV     svc_r14,r0              ; load the entry "irq_r14" entry

        AND     r0,r0,#Fbit             ; create suitable FIQ mask
        TEQP    r0,#(Ibit :OR: IRQmode) ; and return to IRQ mode
        NOP                             ; and wait for registers to be mapped

        LDMFD   irq_r13!,{r0,r1,r2,r3,r4}	; restore work registers
	ASSERT	((5 * word) = size_IRQ_entry)

        ; The relevant handler is entered with the following state:
        ;   SVC mode
        ;   IRQs disabled
        ;   FIQs as interrupted process
        ;   svc_r14 - copy of modified irq_r14 (suitable for return to caller)
        ;   svc_r13 - valid FD stack (containing interrupted SVC r13 and r14)

        MOVS    pc,irq_r14      ; enter IRQ handler (updating PC mode and PSR)

        ; ---------------------------------------------------------------------
        ; Device driver, call Helios when unrecognised interrupt.
no_interrupt
        ; We do not recognise the IRQ source. Call the nucleus handler
        ; function. This is a C function, so we must set up a minimum
        ; world for it.
	; Interrupt functions should perform as little code as is possible
	; (ie. the only acceptable external call should be one to
	;  "HardenedSignal" (which may generate a "Resume" call)).
        ;
	; We perform a direct branch to "Return_From_IRQ" in this code.
        ; We must ensure that we keep IRQs disabled during the following
        ; processor mode changes (and preserve the entry FIQ state).

        MOV     r0,irq_r14			; preserve return address
        ORR     irq_r14,r1,#(Ibit :OR: SVCmode) ; SVC mode, IRQs disabled
        AND     r1,r0,#Fbit                     ; get callers FIQ state
        ORR     irq_r14,irq_r14,r1              ; and add in callers FIQ state

        TEQP    pc,irq_r14              ; SVC mode, IRQs disabled
        NOP                             ; and wait for registers to be mapped

	LDR	r2,=entered_IRQ
	LDR	r3,[r2,#&00]		; current active IRQ thread count
	TEQ	r3,#&00000000		; are we the first?
	LDREQ	r4,=top_IRQ_stack	; only thread, so new stack
	MOVNE	r4,svc_r13		; otherwise preserve current SVC stack
	ADD	r3,r3,#&01		; update the active count
	STR	r3,[r2,#&00]		; and store back into its location

	; continue SVC thread setup
	STMFD   r4!,{svc_r13,svc_r14}	; preserve real SVC thread information
	MOV	svc_r13,r4		; and index the stack for SVC mode

	; SVC mode; IRQs disabled; FIQs as interrupted process
	; r0 = "svc_r14" for "Return_From_IRQ" (ie. caller return address)
	STMFD	sp!,{r0}		; store callers address
	; SVC stack:
	;	+--------------+	top_IRQ_stack or interrupted SVC r13
	;	| original r14 |
	;	+--------------+
	;	| original r13 |
	;	+--------------+
	;	| return addr  |
	;	+--------------+	SVC r13/sp (lo-address)

	; We are in SVC mode here, so we need to directly access the location
	; where the work registers are stacked.
	LDR	r0,=top_IRQ_entry		; address the stored registers
	LDMDB	r0,{r0,r1,r2,r3,r4}		; and load the registers
	ASSERT	((5 * word) = size_IRQ_entry)

	LDR	lk,=ROOT_start
	LDR	lk,[lk,#ExecRoot_devhand]	; address of Helios handler
	TEQ	lk,#&00000000			; is there one?
	BEQ	no_handler_function

	; r0, r1, r2, r3, r9 and r12 may get corrupted during a standard PCS
	; call. r10 and r11 need to contain sensible values. 
	STMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,dp,sb,fp,ip}
	; Since we do not want to call the interrupt handler with the "dp" of
	; the interrupted process, we set "dp = -1". The system error
	; handler code can then spot a Fatal Executive error.
	MOV	dp,#&FFFFFFFF
	MOV	fp,#&00000000		; no stack-frame structure
	LDR	sb,=bottom_IRQ_stack	; stack limit (no spare room at all)

	; At the moment we make the handler call all the device drivers.
	; Eventually we will pass through the actual (hercules) interrupt ID
	; (We can then mask the interrupt source and enable IRQs before calling
	; Helios).

	MOV	r0,#&FFFFFFFF		; make Helios call all the handlers
	MOV	r1,lk			; Helios handler address
	MOV	lk,pc			; remember return address
	MOV	pc,r1			; call function (which can enable IRQs)
interrupt_handler_return
	; return from handler
	LDMFD	sp!,{r0,r1,r2,r3,r4,r5,r6,r7,r8,dp,sb,fp,ip}
no_handler_function	; Exit through "Return_From_IRQ" regardless
	LDMFD	sp!,{svc_r14}		; recover stacked "r0" above
	B	Return_From_IRQ

        ; ---------------------------------------------------------------------
        ; Link interrupt routine. This code is just ripped from the
        ; old FIQ code. It will not be amazingly fast compared to the
        ; original IRQ driven transfers (since each call copies one byte only).
	; THIS SHOULD BE SPEEDED UP.
LinkInterrupt
        ; in:   SVC mode; IRQs disabled; FIQs undefined.
        ;       svc_r14 : current process return address
	;	svc_r13 : FD stack (containing original SVC r13 and r14)
        ;       All other registers must be preserved
        ;       "svc_r13" and "svc_r14" should be restored on EXIT
        ;
        ; At the moment the Executive allows bi-directional link transfers
        ; to be active. This means that this IRQ code must discover whether
        ; we are reading or writing a byte.
        ;
        ; If we complete the transfer, then we should disable the particular
        ; link adaptor interrupt source and add the blocked process SaveState
        ; onto the relevant scheduler queue.

        STMFD   sp!,{r0,r1,r2,r3,r4}

	LDR	r1,=LINK0_base		; link hardware address
        LDR     r4,=link_IRQ_workspace  ; IRQ transfer descriptions
        LDMIA   r4,{r2,r3}              ; load TX description variables
        CMP     r3,#&00000000           ; check for NULL transfer count
	LDRNEB	r0,[r1,#LINK_wstatus]
	TSTNE	r0,#LINK_intenable	; are TX interrupts enabled
	TSTNE	r0,#LINK_data		; bit set marks ready state
        BNE     write_trans             ; we have a write interrupt

        ADD     r4,r4,#(rxbuffer_address - link_IRQ_workspace)
        LDMIA   r4,{r2,r3}              ; load RX description variables
        CMP     r3,#&00000000           ; check for NULL transfer count
	LDRNEB	r0,[r1,#LINK_rstatus]
	TSTNE	r0,#LINK_intenable	; are RX interrupts enabled
	TSTNE	r0,#LINK_data		; bit set marks ready state
        SUBEQ   r4,r4,#(rxbuffer_address - link_IRQ_workspace)
        BEQ     exit_linkIRQ
read_trans
        ; r0 = undefined
        ; r1 = base address of link adaptor hardware
        ; r2 = buffer address
        ; r3 = transfer count
        ; r4 = base address of receive transfer variables

        ; "r3 = 0" check is made above

	LDRB	r0,[r1,#LINK_read]	; get byte
        STRB    r0,[r2],#&01            ; and store in the buffer

        SUBS    r3,r3,#&01
        STMIA   r4,{r2,r3}              ; store updated address and count
        BNE     exit_linkIRQ           ; we require more data

        ; no more data required, so disable read interrupt
        MOV     r0,#&00                 ; disable read interrupts
	STRB	r0,[r1,#LINK_rstatus]

        ; load "SaveState *"
        LDR     r1,[r4,#(rxbuffer_savestate - rxbuffer_address)]
        LDR     r0,=ROOT_start
        ; r0 = "ExecRoot" data structure
        ; r1 = SaveState of process to be re-started
        ; r2 = undefined
        ; r3 = undefined
        ; r4 = undefined
	; The following code assumes "SaveState->next" is NULL.
	; This code assumes that we are dealing with priority 0 processes.
	; NOTE: This WILL update the head pointer if the queue is currently
	;	empty.
	LDR	r2,[r0,#(ExecRoot_queues + ProcessQ_tail)]
	STR	r1,[r2,#SaveState_next]
	STR	r1,[r0,#(ExecRoot_queues + ProcessQ_tail)]
        B       exit_linkIRQ

write_trans
        ; r0 = undefined
        ; r1 = base address of link adaptor hardware
        ; r2 = buffer address
        ; r3 = transfer count
        ; r4 = base address of transfer variables

        ; "r3 = 0" check is made above

        LDRB    r0,[r2],#&01            ; read from buffer (updating address)
	STRB	r0,[r1,#LINK_write]

        SUBS    r3,r3,#&01
        STMIA   r4,{r2,r3}              ; store updated address and count
        BNE     exit_linkIRQ            ; we have more data to transmit

        ; no more data left, so disable write interrupt
        MOV     r0,#&00                 ; disable write interrupts
	STRB	r0,[r1,#LINK_wstatus]

        ; load "SaveState *"
        LDR     r1,[r4,#(txbuffer_savestate - link_IRQ_workspace)]
        LDR     r0,=ROOT_start
        ; r0 = "ExecRoot" data structure
        ; r1 = SaveState of process to be re-started
        ; r2 = undefined
        ; r3 = undefined
        ; r4 = undefined
	; The following code assumes "SaveState->next" is NULL.
	; This code assumes that we are dealing with priority 0 processes.
	; NOTE: This WILL update the head pointer if the queue is currently
	;	empty.
	LDR	r2,[r0,#(ExecRoot_queues + ProcessQ_tail)]
	STR	r1,[r2,#SaveState_next]
	STR	r1,[r0,#(ExecRoot_queues + ProcessQ_tail)]
exit_linkIRQ
        LDMFD   sp!,{r0,r1,r2,r3,r4}
        B       Return_From_IRQ         ; continue IRQ processing

        ; ---------------------------------------------------------------------
        ; CLOCK interrupt routine: entered for every countdown completion
ClockInterrupt
        ; in:   SVC mode; IRQs disabled; FIQs undefined.
        ;       svc_r14 : current process return address
	;	svc_r13 : FD stack (containing original SVC r13 and r14)
        ;       All other registers must be preserved
        ;       "svc_r13" and "svc_r14" should be restored on EXIT

        ; "TickSize" should be tailored to produce the number of micro-seconds
        ; between clock interrupts.

        ; stack work registers
        STMFD   sp!,{r0,r1,r2,r3,r4}

        ; We already know that it was a clock interrupt,
        ; so clear the interrupt source (and re-enable the next interrupt).
	[	(hercules)
	MOV	r0,#TIMER_regs
	LDRB	r0,[r0,#TIMER_countCLR]		; read clears interrupt
	|	; middle (hercules)
        LDR     r0,=timer_intclr                ; Not an 8-bit constant
        STRB    r0,[r0,#0]                      ; Any write here clears int
	]	; EOF (hercules)

        ; Note: It should be possible for IRQs to be enabled at this point,
        ;       since we are now executing in SVC mode. All that is required
        ;       is that if we are interrupted, our svc_r14 is preserved.
	;	Note: A copy of SVC r14 is stored on the stack already.

        LDR     r0,=ROOT_start                  ; ExecRoot structure

        ; This should be the only place where the soft timer values are
        ; incremented.
	[	(shutdown)
	LDMIA	r0,{r1,r2,r3}		; load soft timers and counters
	ADD	r1,r1,#TickSize		; micro-second timer
	ADD	r2,r2,#&01		; centi-second timer
	SUBS	r3,r3,#&01		; centi-second idle timeout
	STMIA	r0,{r1,r2,r3}		; store soft timer values back
	BEQ	TimeoutOccurred		; enter IDLE timeout handler code
TimeoutReturn
	; We return to this point after the IDLE timeout sequence. The system
	; should already have dealt with the soft-timers.
	;
        ; Ensure that we do not update the ExecRoot structure without this code
        ASSERT  (ExecRoot_timer = &00)
        ASSERT  (ExecRoot_cstimer = (ExecRoot_timer + &04))
        ASSERT  (ExecRoot_idletimeout = (ExecRoot_cstimer + &04))
	|	; middle (shutdown)
        LDMIA   r0,{r1,r2}              ; load soft timers
        ADD     r1,r1,#TickSize         ; micro-second timer
        ADD     r2,r2,#&01              ; centi-second timer
        STMIA   r0,{r1,r2}              ; store soft timers
        ; ensure that we do not update the ExecRoot structure without this code
        ASSERT  (ExecRoot_timer = &00)
        ASSERT  (ExecRoot_cstimer = (ExecRoot_timer + &04))
	]	; EOF (shutdown)

	[	(hercmlink :LOR: fpmlink)
	; Call the microlink timeout handler every few (power of 2) ticks
	; as it does not need very fine resolution.
	; r0 points to ExecRoot structure.
	; ML_Timer corrupts r1, r2, r3 and r4 only.
	TST	r2,#ML_TickMask		; Suitable tick boundary?
	BNE	NotMLTick		; No

	; This code could be placed in-line for a slight performance gain.
	STMFD	r13!,{svc_r14}		; Must preserve SVC r14
	BL	ML_Timer		; Call microlink timeout routine
	LDMFD	r13!,{svc_r14}		; Restore SVC r14

NotMLTick
	] ; end (hercmlink :LOR: fpmlink)

	; r0 = ROOT_start - Executive ROOT data structure
	[	(fix0001)
	; This check should not occur here. If we are a hi-priority process
	; we should still decrement the timeslice remaining to zero, but not
	; act on the timeslice expiring. This will ensure that when/if the
	; process becomes a low-priority process it will be re-scheduled
	; immediately if its timeslice has expired.
	|
        LDR     r1,[r0,#ExecRoot_pri]
        TEQ     r1,#&00000000
        ; if hi-priority process then skip to next section
        BEQ     TimerQ
	]	; EOF (fix0001)

        LDR     r1,[r0,#ExecRoot_flags]
        TST     r1,#xb_idle
        ; if IDLE process skip to next section
        BNE     TimerQ

	[	(fix0001)
	!	0,"fix0001: update ClockInterrupt timeslicing code"
	; Load the current timeslice remaining value, and decrement it.
	; If it decrements below zero then the process should be re-scheduled.
	LDR	r1,[r0,#ExecRoot_timeslice]	; load timeslice remaining
	SUBS	r1,r1,#TickSize			; decrement by clock tick
	MOVCC	r1,#&00000000			; make zero if Carry occurred
	STR	r1,[r0,#ExecRoot_timeslice]	; update timeslice remaining
	; If we decremented past zero then set the flag to notify the
	; "Return_From_IRQ" code that this process thread should be
	; re-scheduled. NOTE: At the moment this code will slice the current
	; process EVEN if it is the only process. It should however be started
	; immediately when the Scheduler is entered. The following should be
	; an atomic operation, but we are still executing with IRQs off, so we
	; should be OK.
	LDRCC	r1,[r0,#ExecRoot_flags]		; current Scheduler flags
	ORRCC	r1,r1,#xb_next			; set the next process bit
	STRCC	r1,[r0,#ExecRoot_flags]		; and update the flags
	; The above code will have updated the timeslice remaining and possibly
	; the Scheduler flags. However, if we are a hi-priority process then
	; we will ignore the flag change and return to the process directly.
	;
	!	0,"TODO: check for possible problems"
	; We may have a problem if the "xb_next" bit has been set, since it
	; won't be cleared if we are a hi-pri process. This may cause problems
	; later... research
	;
	; Fall through to "TimerQ"...
	|	; middle (fix0001)

        LDR     r1,[r0,#ExecRoot_timeslice]	; load timeslice remaining
        TEQ     r1,#&00000000                   ; check for timeslicing
        BGT     DownSlice                       ; if greater than zero

        ; The following should be an atomic operation, but we are still
        ; executing with IRQs off so we should be OK.

        ; Set the flag to notify the "Return_From_IRQ" code that this process
        ; thread should be time-sliced...
        ; Note: At the moment this code will slice the current process EVEN
        ;       if it is the only process (it should however be started
        ;       immediately again when the scheduler is entered)

        LDR     r1,[r0,#ExecRoot_flags]
        ORR     r1,r1,#xb_next          ; set the bit
        STR     r1,[r0,#ExecRoot_flags] ; and place back into the structure

        B       TimerQ

DownSlice
        ; in:   r0 = ExecRoot structure
        ;       r1 = current "timeslice" value
        ;       r2 = undefined
        ;       r3 = undefined
        ;       r4 = undefined
        SUB     r1,r1,#TickSize         	; decrement timeslicing counter
        STR     r1,[r0,#ExecRoot_timeslice]	; updating timeslice remaining

	!	0,"TODO : check if we need to time-slice this process"
	; If we have just decremented the time-slice over zero, then this
	; process should possibly be re-scheduled.
	]	; EOF (fix0001)
TimerQ
        ; Remove all pending processes from the TimerQ
        LDR     r1,[r0,#ExecRoot_timerQ]
        TEQ     r1,#&00000000           ; any processes on timerQ?
        BEQ     ClockReturn             ; if not then exit quickly

        ; We have at least one process waiting on the "timerQ"
TimerQrecheck
        ; NOTE: non-optimal register usage over the following routines
        ; r0 = "ExecRoot" address
        ; r1 = "SaveState" address for the process at the start of the "timerQ"
        ;
        ; Load the "SaveState_endtime" value of the process at the head of the
        ; queue. Decrement "TickSize" from this value, if C is set or Z set
        ; then this process should be re-started.
        LDR     r2,[r1,#SaveState_endtime]      ; wakeup delta

        SUBS    r2,r2,#TickSize                 ; decrement the delta
        BEQ     TimerQwakeup                    ; wakeup this process

        STRCS   r2,[r1,#SaveState_endtime]      ; remember the new delta
        BCS     ClockReturn                     ; do not wakeup this process
        ; fall through to...
TimerQwakeup
        ; r0 = "ExecRoot" address
        ; r1 = "SaveState" address of the process to be re-started
        LDR     r2,[r1,#SaveState_next]		; reference our next SaveState
        STR     r2,[r0,#ExecRoot_timerQ]	; and update the TimerQ head

        LDR     r2,[r1,#SaveState_pri]  ; get priority of process to be resumed
        LDR     r4,[r0,#ExecRoot_hipri] ; and highest that could be run
        CMP     r2,r4
        STRCC   r2,[r0,#ExecRoot_hipri] ; this one is higher priority

        ; r0 = "ExecRoot" address
        ; r1 = "SaveState" address of the process to be re-started
        ; r2 = priority of process to be re-started
        ; r3 = undefined
        ; r4 = undefined
        ADD     r3,r0,#ExecRoot_queues
        ADD     r3,r3,r2,LSL #ProcessQ_shift    ; address of desired entry
        ; r0 = ExecRoot structure
        ; r1 = SaveState structure pointer of process to be re-started
        ; r2 = priority of process to be re-started
        ; r3 = ProcessQ structure pointer of queue where process to be placed
        ; r4 = undefined

        ; NULL the next pointer when taking from this queue onto the ProcessQ.
	; This is required since we rely on this when placing onto the
	; relevant ProcessQ.
        MOV     r4,#&00000000
        STR     r4,[r1,#SaveState_next]
	[	(fix0001)
	; Ensure that the process will receive a full time-slice period when
	; it re-started from the ProcessQ we are about to place it on.
	STR	r4,[r1,#SaveState_timeslice]
	]	; EOF (fix0001)

        ; add r1 (SaveState *) onto the referenced ProcessQ
        LDR     r2,[r3,#ProcessQ_tail]  ; load the current tail node
        STR     r1,[r2,#SaveState_next] ; reference the new tail node
        STR     r1,[r3,#ProcessQ_tail]  ; in both places

        ; We should now check that the next pointer does NOT also need to
        ; taken from the TimerQ.
        LDR     r1,[r0,#ExecRoot_timerQ]
        TEQ     r1,#&00000000                   ; any processes on timerQ?
        BNE     TimerQrecheck                   ; if not then exit quickly
        ; fall through to...
ClockReturn
        ; pop stacked entry registers
        LDMFD   sp!,{r0,r1,r2,r3,r4}
        ; all process registers must be preserved
        ; svc_r13 : FD stack (containing original "svc_r14")
        ; svc_r14 : IRQ return address

        ; and fall through to "Return_From_IRQ"

        ; ---------------------------------------------------------------------
        ; -- Return_From_IRQ --------------------------------------------------
        ; ---------------------------------------------------------------------

Return_From_IRQ
        ; in:   SVC mode, IRQs disabled, FIQs undefined
	; 	svc_r14 : return address (to original caller)
	;	svc_r13 : FD stack (containing original SVC r13 and r14)
	;
        ;       All the interrupted processes registers need to be saved into a
        ;       process "SaveState" structure if required.
        ; out:  We return to the current or the next process.
	;
	!	0,"TODO: change to only those that Re-Schedule"
        ; Used by all interrupt routines on exit (executes in SVC (NOT IRQ)
        ; mode). We are preserving SVC state when interrupted by using the
        ; same stack (assuming it is large enough) and preserving the
        ; original SVC r13 and r14 on this stack. When we have finished our IRQ
        ; thread we should restore these registers to the state when the IRQ
        ; occured. The SVC stack we are currently using us a special IRQ
	; handler only stack. It is of limited size.
        ;
	; Deal with direct return to interrupted SVC IRQ thread.
	; Note: this code relies on IRQs being disabled whilst it checks and
	;	modifies the out-standing IRQ count.
	;	If we protect this piece of code explicitly we should be able
	;	to change the "Return_From_IRQ" specification to say IRQs
	;	undefined (ie. device drivers can enable processor IRQs if
	;	they have masked/accepted their interrupt source).
	;
	;
	; NOTE: We could improve the register usage by storing {r2,r3} first
	;	and then adding {r1} and {r0} (in that order) if we need
	;	them later on. The register usage is a bit old at the moment
	;	and is definately non-optimal.
	;
	; These registers make the code a bit easier later on...
	STMFD	sp!,{r1,r2}
	LDR	r1,=entered_IRQ
	LDR	r2,[r1,#&00]		; outstanding IRQ thread count
	SUBS	r2,r2,#&01		; decrement count (for this return)
	STR	r2,[r1,#&00]
	BEQ	UserProcess		; NO more IRQ threads to return to
DirectSVCExit
	; sp ==> r1, r2, SVC r13, SVC r14
	LDR	r1,[sp,#&0C]		; r1 = original SVC r14
	STR	lk,[sp,#&0C]		; store current return address on stack
	MOV	lk,r1			; restore original SVC r14
	LDMFD	sp,{r1,r2,sp,pc}^	; and return to the interrupted thread

DirectSVCExit2
	; sp ==> r0, r1, r2, SVC r13, SVC r14
	LDR	r0,[sp,#&10]		; r0 = original SVC r14
	STR	lk,[sp,#&10]		; store current return address on stack
	MOV	lk,r0			; restore original SVC r14
	LDMFD	sp,{r0,r1,r2,sp,pc}^	; and return to the interrupted thread

	; ---------------------------------------------------------------------
	; This is the outermost interrupt level
UserProcess
	; in:	SVC mode; IRQs disabled; FIQs undefined
	;	svc_r14 : return address to interrupted thread
	;	svc_r13 : stacked r1, r2 and original SVC r13 and r14
	;
	; Only USR and SVC UserProcess threads will reach this point.
	TST	lk,#MODEmask		; mask out all but the processor mode
	BNE	UserProcessSVC		; Interrupted SVC mode UserProcess
	; 				; Interrupted USR mode UserProcess

	LDMFD	sp!,{r1,r2}		; recover entry r1 and r2
	ADD	sp,sp,#&08		; and dump original SVC r13 and SVC r14

	; Deal with direct return to hipriority processes and normal scheduling

        STMFD   sp!,{r1,r2,r3}

        LDR     r1,=ROOT_start

        LDR     r2,[r1,#ExecRoot_pri]
        TEQ     r2,#&00000000           ; if hi-priority then return
        LDMEQFD sp!,{r1,r2,r3}
        MOVEQS  pc,svc_r14              ; return immediately

        ; "lo-priority" or "IDLE" process interrupted

        ; The following code checks if the current process is to be
        ; re-scheduled due to the end of its time-slice, or because a new
        ; process has appeared on a higher priority ProcessQ (pre-emption).
	; Since a process whose timeslice has expired will go through the
	; Scheduler, it will startup any pending higher-priority processes
	; without the code performing an explicit check. If the code is
	; not to be timesliced then we need to check if the process is to
	; be pre-empted.

        LDR     r3,[r1,#ExecRoot_flags]
        TST     r3,#xb_next             ; check if due for timeslicing
        BNE     NextProcess             ; YES - then swap to next process

	; r2 = current process priority

        LDR     r3,[r1,#ExecRoot_hipri]	; highest priority process that can run
        CMP     r3,r2           ; check if higher priority process scheduled
        ; If the same or lower priority, then we do not need to do anything
        LDMGEFD sp!,{r1,r2,r3}
        MOVGES  pc,svc_r14      ; return to interrupted USR thread

        ; otherwise we should pre-empt this process
SaveSelf
        ; in:   r1 = ExecRoot structure
        ;       r2 = current priority level
        ;       r3 = highest priority that can be run
        ;       SVC mode, IRQs disabled, FIQs undefined

	LDR	r3,[r1,#ExecRoot_flags]	; re-load this value (again!!!!)
	TST	r3,#xb_idle		; check for interrupted IDLE process
        LDMNEFD sp!,{r1,r2,r3}          ; if IDLE process...
        BNE     Scheduler1              ; do not bother saving process state

        ; move the USR mode stack pointer into "r1"
        SUB     sp,sp,#&04              ; make space for the USR mode r13
        STMIA   sp,{usr_sp}^		; store USR mode r13 onto stack
        NOP				; wait for register re-mapping
        LDMFD   sp!,{r1}		; load USR mode r13 into r1

UserProcessEnterScheduler
	; r0 = preserved from entry
	; r1 = UserProcess stack pointer
	; r2 = undefined
	; r3 = undefined
	;
        ; Allocate space on the UserProcess stack for the process description
        SUB     r1,r1,#SaveState_size   ; see NOTES in the header files

        STR     r0,[r1,#SaveState_r0]   ; store "r0" for the process
        MOV     r0,r1                   ; and copy the pointer into r0

        ; The actual saved process priority should be derived from the
        ; ExecRoot structure. (NOTE: we are re-loading this value again!!!!)
        LDR     r1,=ROOT_start                  ; ExecRoot structure address
        ; code and time could be saved by using more registers and LDM/STMs
	MOV	r2,#&00000000
	STR	r2,[r0,#SaveState_flags]	; clear the state flags
        LDR     r2,[r1,#ExecRoot_fparea]        ; preserve FP state
        STR     r2,[r0,#SaveState_fparea]
        LDR     r2,[r1,#ExecRoot_initial_dp]    ; preserve initial "dp"
        STR     r2,[r0,#SaveState_initial_dp]
	[	(memmap)
	LDR	r2,[r1,#ExecRoot_memmap]	; preserve MEMMAP state
	STR	r2,[r0,#SaveState_memmap]
	]	; EOF (memmap)
        LDR     r2,[r1,#ExecRoot_timeslice]     ; timeslice remaining
        STR     r2,[r0,#SaveState_timeslice]	; store in the SaveState
        LDR     r2,[r1,#ExecRoot_pri]           ; and current process pri level
        STR     r2,[r0,#SaveState_pri]

        ; Add the process to the FRONT of the relevant queue.
        ; r0 = (SaveState *) of process
        ; r1 = ExecRoot structure address
        ; r2 = process priority
        ADD     r1,r1,#ExecRoot_queues          ; base of the ProcessQs
        ADD     r1,r1,r2,LSL #ProcessQ_shift    ; reference the correct queue
        LDR     r2,[r1,#ProcessQ_head]          ; load current head node
        STR     r2,[r0,#SaveState_next]         ; and reference in new node
	TEQ	r2,#&00000000			; head previously NULL
	STREQ	r0,[r1,#ProcessQ_tail]		; then update tail pointer
	STR	r0,[r1,#ProcessQ_head]		; and insert new head node

        ; de-stack any pushed registers
        LDMFD   sp!,{r1,r2,r3}
        ; IRQs disabled
        ; "r0" should contain a pointer to the "SaveState" structure
        ; "svc_r14" should contain the return address
        B       Scheduler               ; **** exit from IRQ handler ****

        ; ---------------------------------------------------------------------
        ; Add the current (interrupted) USR process onto the relevant
        ; priority ProcessQ
NextProcess
        ; in:   entry "r1","r2" and "r3" on the stack
        ;       r1 = ExecRoot structure
        ;       r2 = current process priority level
        ;       r3 = ExecRoot "flags" value
        ;       SVC mode, IRQs disabled, FIQs undefined

        ; Do not timeslice when we were executing in IDLE
	TST	r3,#xb_idle		; check if last process was IDLE
        ; If IDLE process, don't bother saving... just try to re-schedule
        LDMNEFD sp!,{r1,r2,r3}          ; if IDLE process...
        BNE     Scheduler1              ; do not bother saving process state

        ; Move the USR mode stack pointer into "r1"
        ; NOTE: This takes a copy of the USR mode stack pointer.
        SUB     sp,sp,#&04              ; make space for the USR mode r13
        STMIA   sp,{usr_sp}^            ; store USR mode "r13" on stack
        NOP				; wait for register re-mapping
        LDMFD   sp!,{r1}                ; and load into "r1"
        ; r1 = interrupted USR mode process stack pointer

        ; Allocate space in the User Stack for the process description.
        ; NOTES: This does NOT check for stack overflow... but we assume that
        ;        the size of the "SaveState" structure will fit into the
        ;        stack overflow buffer area.

        SUB     r1,r1,#SaveState_size

        STR     r0,[r1,#SaveState_r0]   ; save "r0" for the process description
        MOV     r0,r1                   ; and copy the (SaveState *) into r0

        ; NULL "next" pointer (this is required by code below).
        MOV     r1,#&00000000
        STR     r1,[r0,#SaveState_next]

        ; This code should really copy the current "ExecRoot" priority value
        ; (as long as it has not been corrupted) into the SaveState structure.
        LDR     r1,=ROOT_start
	; r2 = current priority level (from "NextProcess" entry)
	STR	r2,[r0,#SaveState_pri]		; store into the "SaveState"
	MOV	r3,#&00000000
	STR	r3,[r0,#SaveState_flags]	; clear the state flags
        LDR     r3,[r1,#ExecRoot_fparea]        ; current process FP state
        STR     r3,[r0,#SaveState_fparea]
	[	(fix0001)
	; Ensure that all the routes through the scheduler preserve the
	; timeslice remaining field.
        LDR     r3,[r1,#ExecRoot_timeslice]     ; timeslice remaining
        STR     r3,[r0,#SaveState_timeslice]	; store in the SaveState
	]	; EOF (fix0001)
	[	(memmap)
	LDR	r3,[r1,#ExecRoot_memmap]	; preserve MEMMAP state
	STR	r3,[r0,#SaveState_memmap]
	]	; EOF (memmap)
        LDR     r3,[r1,#ExecRoot_initial_dp]
        STR     r3,[r0,#SaveState_initial_dp]

        ; r0 = "SaveState *" of process to be suspended
        ; r1 = ExecRoot structure address
        ; r2 = current process priority
        ; r3 = initial dp register value of current process

        ; We need to place the "SaveState *" onto the correct priority queue
	; r2 - preserved from "NextProcess" entry
        ADD     r1,r1,#ExecRoot_queues          ; base address of the queues
        ADD     r1,r1,r2,LSL #ProcessQ_shift
        ; r0 = "SaveState *" of process to be suspended
        ; r1 = address of the correct priority ProcessQ
        ; r2 = priority of process
        ; r3 = undefined

        ; add r0 (SaveState *) onto the referenced ProcessQ
        LDR     r2,[r1,#ProcessQ_tail]          ; load the current tail node
        STR     r0,[r2,#SaveState_next]         ; reference the new tail node
        STR     r0,[r1,#ProcessQ_tail]		; in both places

        LDMFD   sp!,{r1,r2,r3}
        ; IRQs should still be disabled
        ; "r0" should contain a pointer to the "SaveState" structure
        ; "svc_r14" should contain the return address
        B       Scheduler               ; **** exit from IRQ handler ****

        ; ---------------------------------------------------------------------

UserProcessSVC
	; Interrupted SVC mode UserProcess.
	; in:	SVC mode; IRQs disabled; FIQs undefined
	;	svc_r14 : return address to interrupted thread
	;	svc_r13 : stacked r1, r2 and original SVC r13 and r14

	; Deal with direct return to hipriority processes and normal scheduling
        LDR     r1,=ROOT_start

        LDR     r2,[r1,#ExecRoot_pri]
        TEQ     r2,#&00000000           ; if hi-priority then return
	BEQ	DirectSVCExit		; exit restoring state

        ; "lo-priority" process interrupted (since IDLE is always a USR mode
	; process (at the moment)).

        ; The following code checks if the current process is to be
        ; re-scheduled due to the end of its time-slice, or because a new
        ; process has appeared on a higher priority ProcessQ (pre-emption).

	STMFD	sp!,{r0}		; add another register to the stack

	LDR	r0,[r1,#ExecRoot_flags]	; load scheduler flags (again!!!!)
	TST	r0,#xb_next		; check if due for timeslicing
        BNE     NextProcessSVC          ; YES - then swap to next process

        LDR     r0,[r1,#ExecRoot_hipri]
        CMP     r0,r2           ; check if higher priority process scheduled
        ; If the same or lower priority, then we do not need to do anything
	BGE	DirectSVCExit2		; exit restoring state

SaveSelfSVC
        ; Otherwise we should pre-empt this process.
        ;   r0 = undefined
	;   r1 = ExecRoot structure
        ;   r2 = current priority level
	;   sp = contains r0, r1, r2, SVC r13 and r14
	;
	; Since we are a SVC thread, we do not bother checking if we were
	; the IDLE process. Move the original SVC mode stack pointer into
	; "r2".
	;
	LDR	r2,[sp,#&0C]		; load the original SVC r13
        ; Allocate space on the user process stack for the process description
        SUB     r2,r2,#SaveState_size

	LDMFD	sp!,{r0}		; recover the process "r0"
        STR     r0,[r2,#SaveState_r0]   ; and store "r0" into the SaveState
        MOV     r0,r2                   ; and copy the pointer into r0
	; r0 = SaveState structure pointer

        ; The actual saved process priority should be derived from the
        ; ExecRoot structure.
	; r1 = ExecRoot structure address
        ; (code and time could be saved by using more registers and LDM/STMs)

	MOV	r2,#&00000000
	STR	r2,[r0,#SaveState_flags]	; clear the state flags

        LDR     r2,[r1,#ExecRoot_fparea]        ; preserve FP state
        STR     r2,[r0,#SaveState_fparea]

        LDR     r2,[r1,#ExecRoot_initial_dp]    ; preserve initial "dp"
        STR     r2,[r0,#SaveState_initial_dp]

	[	(memmap)
	LDR	r2,[r1,#ExecRoot_memmap]	; preserve MEMMAP state
	STR	r2,[r0,#SaveState_memmap]
	]	; EOF (memmap)

        LDR     r2,[r1,#ExecRoot_timeslice]     ; timeslice remaining
        STR     r2,[r0,#SaveState_timeslice]	; store in the SaveState

        LDR     r2,[r1,#ExecRoot_pri]           ; and current process pri level
        STR     r2,[r0,#SaveState_pri]		; (which we loaded above)

        ; Add the process to the FRONT of the relevant queue.
        ; r0 = (SaveState *) of process
        ; r1 = ExecRoot structure address
        ; r2 = process priority
        ADD     r1,r1,#ExecRoot_queues          ; base of the ProcessQs
        ADD     r1,r1,r2,LSL #ProcessQ_shift    ; reference the correct queue
        LDR     r2,[r1,#ProcessQ_head]          ; load current head node
        STR     r2,[r0,#SaveState_next]         ; and reference in new node
	TEQ	r2,#&00000000			; check for NULL head
	STREQ	r0,[r1,#ProcessQ_tail]		; update tail if head NULL
	STR	r0,[r1,#ProcessQ_head]		; and insert new head node

        LDMFD   sp!,{r1,r2}			; de-stack normal registers
	; r0  = pointer to SaveState structure
	; r13 = FD stack containing original SVC r13 and r14
	; r14 = return address to interrupted UserProcess
        B       SchedulerSVC		        ; *** exit from IRQ handler ***

        ; ---------------------------------------------------------------------
        ; Add the current (interrupted) SVC process onto the relevant
        ; priority ProcessQ.
NextProcessSVC
        ; in:   entry r0, r1, r2, SVC r13 and r14 on the stack
	;	r0 = ExecRoot "flags" value
        ;       r1 = ExecRoot structure
        ;       r2 = current priority level
        ;       SVC mode, IRQs disabled, FIQs undefined
	;
	; We ignore the IDLE state when we are dealing with SVC threads

	!	0,"TODO: **** major changes to NextProcessSVC ****"
	; This code should be changed to be like the USR mode equivalent.
	; We need to place the current process at the END of the queue (and
	; not at the FRONT like the SaveSelfSVC code).

	B	SaveSelfSVC		; schedule this UserProcess

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

	[	(shutdown)
TimeoutOccurred
	; We are entered here when the stage 1 centi-second timer reaches
	; zero. We should IDLE the processor (disabling all interrupt sources
	; apart from the uController). We need to set the uController to wake
	; us up after the stage2 configured delay. We can then perform the
	; full PowerDown. We should turn-off the display at this point
	; (which will make the uController refresh the DRAM).
	; When we are awoken from the IDLE (or PowerDown after IDLE) then we
	; should return to "TimeoutReturn" to let the system re-start.

	STMFD	sp!,{r0,lk}
	ADRL	r0,totxt1
	BL	local_Output
	LDMFD	sp!,{r0,lk}
	B	TimeoutReturn
totxt1	=	"TimeoutOccurred: code to be written\n",&00
	ALIGN
	]	; EOF (shutdown)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        LNK     lomlink.s
