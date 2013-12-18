        SUBT    Process scheduler SWI and internal code         > losched/s
        ; Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ARM HELIOS Executive: process scheduler
        ;
        ; Author:               James G Smith
        ;
        ; ---------------------------------------------------------------------

        ASSERT  (ProcessQ_head = &00)   ; the "Scheduler" code assumes this

        ; ---------------------------------------------------------------------
        ; -- exec_Scheduler ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; in:   r0 = current process description (SaveState *)
        ;       sp = FD stack (containing entry r11 and r12)
	;	lk = return address to SWI caller
        ;       SVC mode; IRQs undefined; FIQs undefined.
        ; out:  Never exits back to caller since it will have started another
        ;       process or the idle process.
	;
code_exec_Scheduler
        AND     r11,lk,#Fbit			; entry FIQ state
        TEQP    r11,#(Ibit :OR: SVCmode)        ; disable IRQs explicitly

        LDMFD   sp!,{r11,r12}                   ; recover entry r11 and r12

	; We must provide special code when we are Scheduling an SVC thread.
	; If we are a SWI called from SVC mode then the original lk has been
	; trashed by the SWI entry. In this case we have no way of preserving
	; the original SVC lk over the Scheduler call.
	TST	lk,#MODEmask			; check for USR mode
	BNE	exec_SchedulerSVC		; perform SVC scheduler entry

        ; and fall through to the "Scheduler"

        ; ---------------------------------------------------------------------
        ; Enter the Scheduler. Examine the process queues and start the next
        ; process due for execution.
        ; in:  r0 = current process description (SaveState *)
        ;      sp = FD stack
        ;      lk = return address and PSR of caller
        ;      SVC mode; IRQs disabled; FIQs undefined.
        ;      Assumes process "r0" has been stored in the SaveState structure.
        ;
        ; We are either called through the SWI handler or from an IRQ. We
        ; need to remember the callers state (including PC and PSR) in the
        ; process description structure given (SaveState *).
        ;
        ; We need to access the registers belonging to the process that is
        ; being re-scheduled. If the process being re-scheduled is a USR
	; mode one, the code uses a feature of the ARM processor to save these
	; registers easily. SVC mode processes being re-scheduled will be
        ; returned to directly by the code. This code will not work if IRQ
        ; or FIQ processes are placed into the process queues.
        ;
        ; Process queue updates CAN occur in interrupt handlers. This requires
        ; that all queue manipulations are performed with IRQs disabled (ie.
        ; calling "HardenedSignal" to notify the foreground process, will
        ; "Resume" the process, which updates the process queues).
        ;
        ; NOTE: This code loses the reference to the SaveState structure passed
        ;       into the routine (r0 on entry). It relies on the caller
	;	preserving a reference to this SaveState. Since we do not
	;	return to the caller after this routine, we can end up with a
	;	phantom process if the caller has not placed the SaveState in
	;	some management queue. This feature is used by the "Stop"
	;	function provided by the Hi-Executive.
Scheduler
        CMP     r0,#&00000000
        BEQ     Scheduler1              ; invalid SaveState structure pointer

        ADD     r0,r0,#SaveState_r1     ; address register area in structure
        STMIA   r0,{r1-r14}^            ; and store all the USR mode registers
        SUB     r0,r0,#SaveState_r1     ; recover the "SaveState" base address
        STR     lk,[r0,#SaveState_pc]   ; store PC and PSR of process

        ; We can now use the USR mode registers (r0-r12) since they have been
	; saved. NOTE: we do NOT use the return address value now (lk).
Scheduler1
        ; in:   SVC mode; IRQs disabled; FIQs undefined
        ;       lk : undefined and should NOT be used when IRQs enabled
        ;       This entry point to the Scheduler does NOT preserve a
        ;       processes state.
        ;       (use "Scheduler" directly to perform process preservation)

        LDR     r3,=ROOT_start                  ; ExecRoot structure address
        LDR     r1,[r3,#ExecRoot_hipri]         ; highest priority pending
        ADD     r2,r3,#ExecRoot_queues          ; base of the ProcessQs
        ADD     r2,r2,r1,LSL #ProcessQ_shift    ; base of the pending ProcessQ
try_next_queue
        LDR     r0,[r2,#ProcessQ_head]          ; load the head (SaveState *)
        TEQ     r0,#&00000000                   ; is it NULL?
        BNE     LoadProcess                     ; NO - then start this process

        ADD     r1,r1,#&01                      ; index onto the next ProcessQ
        CMP     r1,#NumberPris                  ; number of available queues
        ADDCC   r2,r2,#ProcessQ_size            ; step onto the next queue
        BCC     try_next_queue
        ; no more queues, so start the "IDLE" process

        ; set the "xb_idle" flag (ignoring the previous state)
	[	{TRUE}
	; This piece of code assumes information about the "flags" structure.
	MOV	r1,#xb_idle		; set the "xb_idle" flag, others clear
	STR	r1,[r3,#ExecRoot_flags]
	|
        LDR     r1,[r3,#ExecRoot_flags]
        ORR     r1,r1,#xb_idle          ; set the "xb_idle" flag
        BIC     r1,r1,#xb_next          ; clear "next" timeslice flag for IDLE
        STR     r1,[r3,#ExecRoot_flags]
	]	; EOF {boolean}

	; We want the IDLE process to have the lowest possible priority number
	MOV	r1,#NumberPris		; 1 beneath lowest priority number
	STR	r1,[r3,#ExecRoot_pri]
	; And the highest that can be run to be the lowest acceptable priority
	MOV	r1,#(NumberPris - 1)	; lowest user priority
        STR     r1,[r3,#ExecRoot_hipri] ; is the highest priority that can run

        ; The IDLE process does not have any FP state or module table pointer
        MOV     r1,#&00000000
        STR     r1,[r3,#ExecRoot_fparea]
        STR     r1,[r3,#ExecRoot_initial_dp]
	[	(memmap)
	[	(hercules)
	; Define the default MEMMAP state (updating the soft-copy)
	MOV	r1,#(MAPEN_USR :OR: OS_MODE1)	; enable OS mode 1
	MOV	r0,#MEMMAP_regs
	STRB	r1,[r0,#&00]
	LDR	r0,=hardware_regs
	STR	r1,[r0,#MEMMAP_data]
	; Place the default MEMMAP state into the "ExecRoot"
	|	; middle (hercules)
	[	{TRUE}			; bodge test for MJackson
	MOV	r1,#&00			; physical memory map
	|
	MOV	r1,#(mmumode_mapen :OR: mmumode_osmode :OR: mmumode_mode1)
	]	; EOF {boolean}
	LDR	r0,=mmu_mode
	STRB	r1,[r0,#&00]		; define the default MEMMAP state
	; Place the default MEMMAP state into the "ExecRoot"
	]	; EOF (hercules)
	STR	r1,[r3,#ExecRoot_memmap]
	]	; EOF (memmap)

	; This code ASSUMES that the IDLE process is always a USR mode one.
	MOV	r1,#&F000000F		; no IDLE stack (invalid address)
        STMFD   sp!,{r1}                ; store on the stack
        LDMIA   sp,{usr_sp}^            ; to be loaded into the USR mode "r13"
        NOP                             ; wait for registers to be re-mapped
        ADD     sp,sp,#&04              ; and claim back the stack location

        AND     r0,pc,#Fbit             ; r0 = state of entry FIQ bit
        ; get the PC (and PSR) of the IDLE process
        ADR     lk,IdleProcess
        ; at the moment all the PSR flags are clear (USR mode, IRQ/FIQ enabled)
        ; we should set the FIQ flag to our state (i.e. preserving)
        ORR     lk,lk,r0      		; copy FIQ bit into process address
        MOVS    pc,lk			; leave scheduler and enter IDLE

        ; ---------------------------------------------------------------------

LoadProcess
        ; in:   r0 = SaveState structure for process to be loaded
        ;       r1 = priority of the process being started
        ;       r2 = ProcessQ of the process being started
        ;       r3 = ExecRoot structure
        ;       SVC mode, IRQs disabled, FIQs undefined
        ; out:  does not return (enters process referenced by "r0")

	[	(fix0001)
	!	0,"TODO : possibly update process timeslicing"
	; We should really keep a count of the amount of time a process has
	; been scheduled in for (in real timer ticks). This can be achieved
	; since the only point at which the executing process is changed is
	; through this code. This would allow us to keep a much more accurate
	; account of how long a process has executed for.
	; If practice however, this will probably not be required. The current
	; code still decrements the timeslice value (to zero) when a hi-pri
	; process is being executed, ignoring the fact that it may reach the
	; end of its slice. However, when the process becomes lo-priority
	; again, it should be immediately re-scheduled since its timeslice has
	; expired. This will require code changes to the Scheduler to always
	; check the "timeslice" remaining field and decide if the process
	; should be re-scheduled.
	]	; EOF (fix0001)

        LDR     r4,[r0,#SaveState_next]         ; reference the next SaveState
        STR     r4,[r2,#ProcessQ_head]          ; make this the new head
        TEQ     r4,#&00000000
        STREQ   r2,[r2,#ProcessQ_tail]          ; queue is now empty
        ADDEQ   r1,r1,#&01                      ; so step onto next queue
        CMPEQ   r1,#NumberPris                  ; check for overflow
        MOVEQ   r1,#(NumberPris - 1)            ; overflowed, then use limit
        STR     r1,[r3,#ExecRoot_hipri]         ; highest priority that can run

        ; This is a good place to use load and store multiples for speed
        LDR     r4,[r0,#SaveState_pri]          ; pri of process about to waken
        STR     r4,[r3,#ExecRoot_pri]           ; set priority

        LDR     r4,[r0,#SaveState_fparea]       ; recover the FP state
        STR     r4,[r3,#ExecRoot_fparea]

        LDR     r4,[r0,#SaveState_initial_dp]   ; initial module table pointer
        STR     r4,[r3,#ExecRoot_initial_dp]

	[	(memmap)
	LDR	r4,[r0,#SaveState_memmap]	; required MEMMAP state for
	STR	r4,[r3,#ExecRoot_memmap]	; this process thread
	[	(hercules)
	MOV	r1,#MEMMAP_regs
	STRB	r4,[r1,#&00]			; set the MEMMAP state
	LDR	r1,=hardware_regs
	STR	r4,[r1,#MEMMAP_data]		; and update the soft-copy
	|	; middle (hercules)
	LDR	r1,=mmu_mode
	STRB	r4,[r1,#&00]			; set the MEMMAP state
	]	; EOF (hercules)
	]	; EOF (memmap)

	[	(fix0001)
	!	0,"fix0001: update timeslice remaining from SaveState"
	; If the timeslice field is zero, then this process requires a
	; new timeslice allocation. This is not really required if the
	; process is a hi-priority one, but the code to check and not
	; update if hi-priority would steal bandwidth, for no real gain.
	LDR	r4,[r0,#SaveState_timeslice]	; load timeslice remaining
	TEQ	r4,#&00000000			; check if zero
	MOVEQ	r4,#TicksPerSlice		; the full process timeslice
	STR	r4,[r3,#ExecRoot_timeslice]	; process timeslice remaining
	|
        MOV     r4,#TicksPerSlice               ; this is not really required
        STR     r4,[r3,#ExecRoot_timeslice]     ; if the process is priority 0
	]	; EOF (fix0001)

	; We are starting a real process.
        LDR     r4,[r3,#ExecRoot_flags]
        BIC     r4,r4,#(xb_idle :OR: xb_next)   ; clear IDLE and "next" flags
        STR     r4,[r3,#ExecRoot_flags]

        ; load the PC (& PSR) of the process to be started
        LDR     lk,[r0,#SaveState_pc]

        ; restore registers from process description
        ADD     r0,r0,#SaveState_r0     ; address the base of the registers
	; We now need to deal with returning to SVC threads...
	TST	lk,#MODEmask
	; Restore the USR mode r13 and r14 if we are restarting a SVC thread.
	ADDNE	r1,r0,#(SaveState_usr_r13 - SaveState_r0)
	LDMNEIA	r1,{r13,r14}^		; force USR mode transfer
	; the following instruction will act as the NOP
	LDMEQIA	r0,{r0-link}^		; load most USR mode registers
	LDMNEIA	r0,{r0-pc}^		; load *ALL* SVC mode registers
	; NOTE: the second LDM acts as a NOP if we actually perform the first.
        ; Leave the scheduler and enter the process (restoring IRQ state)
        MOVS    pc,lk

        ; ---------------------------------------------------------------------
        ; -- SVC Scheduler ----------------------------------------------------
        ; ---------------------------------------------------------------------

exec_SchedulerSVC
	; in:	r0 = pointer to SaveState structure
	;	sp = FD stack (empty)
	;	lk = return address (to SWI caller)
	; We need to preserve the original SVC r13 and r14 on the stack for
	; the following scheduler entry. However, since we were called from
	; a SWI we do not have the original SVC r14 (at the instance of the
	; SWI call). We just stack any old value at this point.
	STMFD	sp!,{sp,pc}		; stack original r13 and INVALID r14
	; The above instruction RELIES on the ARM feature that the first
	; register stored even if it is the base will NOT be modified.
	;
	; and fall through to ...
	; ---------------------------------------------------------------------
SchedulerSVC
	; in:	r0 = pointer to SaveState structure
	;	lk = return address to interrupted thread
	;	sp = FD stack containing original SVC r13 and r14

        ADD     r0,r0,#SaveState_r1     ; address register area in structure
        STMIA   r0!,{r1-r12}            ; store all non-mapped registers
	LDMFD	sp!,{r1,r2}		; recover original SVC r13 and r14
	STMIA	r0!,{r1,r2}		; store the original SVC r13 and r14

	; Preserve the USR mode r13 and r14 in our SaveState structure
	; (r0 (from above) is pointing at SaveState_r15).
	ADD	r0,r0,#(SaveState_usr_r13 - SaveState_r15)
	STMIA	r0,{r13,r14}^		; force USR mode transfer
	; this instruction should NOT use mapped registers
	SUB	r0,r0,#SaveState_usr_r13

	; r0 = pointer to SaveState structure

	!	0,"TODO: *** merge with the code above ***"
        STR     r14,[r0,#SaveState_pc]  ; store PC and PSR of process
	B	Scheduler1		; remainder of Scheduler is same
	
        ; ---------------------------------------------------------------------
        ; -- IDLE process -----------------------------------------------------
        ; ---------------------------------------------------------------------

IdleProcess
        ; in:   USR mode; IRQs enabled; FIQs undefined
        ;       all registers are undefined (ie. no stack or link address)
        ;
	[	((haltmode) :LAND: (activebook))
	!	0,"NOTE: IDLE HALT code included"
	[	{TRUE}	; better DMA channel handling code
	; This code loops until "DMAsource_TEST" becomes available. When it
	; does it uses the DMA channel to force the processor to enter
	; low-power mode. This code relies on the fact that the DMA channel
	; claiming functions are NOT called from IRQ/FIQ threads. This is to
	; save us the over-head of claiming and releasing the DMA channel
	; through the legal scheme (which would mean modifying the Scheduler
	; to perform the DMA release). We do not actually update the state,
	; so it doesn't matter if we are re-scheduled whilst setting up the
	; transfer.
	MOV	r0,#(DMAsource_TEST :SHR: DMAsource_shift)
	LDR	r1,=(DMAchannel_states - word)	; reference the state table
	LDR	r2,[r1,r0,LSL #2]		; load the relevant status
	TST	r2,#DMAchannel_allocated	; bit set if in use
	BNE	IdleProcess			; YES - then around again
	; r0 = channel number available for use
	MOV	r1,#DMA_base			; base of DMA registers
	; setup DMA source
	ADR	r2,halt_data			; data to be DMA'ed
	MOV	r2,r2,LSL #DMA_src_shift
	ORR	r2,r2,#(DMA_32bit :OR: DMA_inc_count)
	STR	r2,[r1,#DMAchan3_b0src]		; set DMA source information
	; setup DMA destination
	MOV	r2,#(CLOCK_regs :SHL: DMA_src_shift)
	ORR	r2,r2,#(((:NOT: 1) + 1) :AND: &FF)	; 2's complement
	STR	r2,[r1,#DMAchan3_b0dest]	; set destination information
	; and start the actual DMA transfer
	MOV	r1,#DMA_regs			; DMA routing register
	LDR	r2,=hardware_regs		; reference the soft-copies
	LDR	r3,[r2,#DMArouting_data]	; current routing state
	BIC	r3,r3,#DMAchan3_mask		; clear chan3 routing bits
	ORR	r3,r3,#DMAchan3_TEST		; chan 3 : memory<->memory copy
	STR	r3,[r2,#DMArouting_data]	; update the soft-copy
	STR	r3,[r1,#&00]			; enable the DMA transfer
	; ---------------------------------------------------------------------
	|	; middle {boolean}
	; ---------------------------------------------------------------------

	!	0,"TODO: *** FIX *** so that it doesn't disable other DMAs ***"
	; Halt mode can only be entered by proviledged system code.
	; Programming the HALT mode register under DMA is the only reliable
	; way of entering processor IDLE mode at the moment.
	; At the moment we use a fixed DMA channel to perform this operation
	; (the code will need to change when generic DMA transfer support is
	; provided by the Executive).

	; We HAVE to use channel 3, since it is the only DMA channel with
	; memory<->memory capability.

	MOV	r1,#DMA_base		; base of DMA registers

	; setup DMA source
	ADR	r0,halt_data		; data to be DMA'ed
	MOV	r0,r0,LSL #DMA_src_shift
	ORR	r0,r0,#(DMA_32bit :OR: DMA_inc_count)
	STR	r0,[r1,#DMAchan3_b0src]	; and set the DMA source information

	; setup DMA destination
	MOV	r0,#(CLOCK_regs :SHL: DMA_src_shift)
	ORR	r0,r0,#(((:NOT: 1) + 1) :AND: &FF)	; 2's complement
	STR	r0,[r1,#DMAchan3_b0dest]; and set the destination information

	MOV	r1,#DMA_regs		; DMA routing register
	MOV	r0,#DMAchan3_TEST	; channel 3 ; memory<->memory copy
	STR	r0,[r1,#&00]		; and enable the DMA transfer
	]	; EOF {boolean}
	; -- THE PROCESSOR IS NOW IN IDLE MODE --------------------------------
	; When the wakeup event occurs, the processor will continue executing
	; from this point.

	; Since processor IRQs are enabled, and FIQs are as entry to the
	; scheduler we should simply be able to continue around the loop
	; (since the scheduler will kill this thread if necessary). Interrupts
	; should be able to get in before we hit the HALT mode again.

	; We should possibly update our soft idle ticker (though we have
	; not ascertained here how synchronous the wakeup event is).
	B	IdleProcess

	; This is the value that is written to the mode register
halt_data	&	(CPU_OFF :OR: MLI_ON)
			; halt        pre-scalers on (MLI and Timer active)
	|	; middle ((haltmode) :LAND: (activebook))
	;
        ; Another type of process (priority level) could be provided by
        ; the IDLE routine. At the moment the idle process just increments
        ; the "ExecRoot_idleLog" field every second.

        LDR     r3,=ROOT_start
        LDR     r1,[r3,#ExecRoot_cstimer]       ; centi-second counter
        ADD     r1,r1,#100                      ; wait for a second
IdleProcess1
        LDR     r2,[r3,#ExecRoot_cstimer]       ; centi-second counter
        CMP     r1,r2
        BGE     IdleProcess1

        LDR     r2,[r3,#ExecRoot_idleLog]       ; idle seconds
        ADD     r2,r2,#1
        STR     r2,[r3,#ExecRoot_idleLog]

        ADD     r1,r1,#100                      ; wait for next second

        B       IdleProcess1
	]	; EOF ((haltmode) :LAND: (activebook))

        ; ---------------------------------------------------------------------
        ; storage area for PC relative constants
        LTORG

        ; ---------------------------------------------------------------------
        LNK     loend.s
