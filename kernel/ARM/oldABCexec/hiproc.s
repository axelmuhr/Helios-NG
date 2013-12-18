        SUBT    Process manipulation routines                   > hiproc/s
        ; Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ARM HELIOS Executive: process manipulation functions
        ;
        ; Author:               James G Smith
        ; History:      891219  File split from main module "hiexec.s"
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; hi-level external interface -----------------------------------------
        ; ---------------------------------------------------------------------
        ; void SchedulerDispatch(SaveState *p) ;
        ; Dispatch the referenced process through the Scheduler.
SchedulerDispatch	FnHead
        MOV     ip,lk
	; NOTE: This will only save the registers, it does NOT place the
	;	the passed SaveState structure pointer onto any queues.
	;	**** IT DOES NOT SAVE ALL THE OTHER SaveState FIELDS ****
	;	a1 is NOT preserved over this function (it must already
	;	have been placed into the SaveState structure).
        SWI     exec_Scheduler          ; call the process scheduler
        MOVS    pc,ip

        ; ---------------------------------------------------------------------
        ; void Suspend(SaveState **pp) ;
        ; Suspend the current process.
        ; in:   a1 : pointer to a "SaveState *" variable
        ; out:  no conditions
        ;
        ; Construct a SaveState structure (for the current process).
        ; Set the "next" entry in the structure to NULL.
        ; Set the priority to 0.
        ; Store the addr of the new SaveState structure at the passed variable.
        ; Dispatch the process through the scheduler
        ;
        ; The SaveState structure can be taken from the stack (i.e. below it
        ; without modifying it)
        ;
        ; NOTE: The address of a non-static local variable is returned to the
        ;       calling function. This is used to pass the address of the
        ;       suspended processes SaveState structure to the caller. The
        ;       pointer is not used dynamically, only as a reference to the
        ;       process.
        ;       (Information is placed in the SaveState structure by the
        ;       Scheduler)
        ;
Suspend	FnHead
        MOV     ip,sp
        STMFD   sp!,{v1,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        SUB     ip,sp,#SaveState_size	; drop for stack overflow check only
        CMP     ip,sl
        BLLT	__stack_overflow_1
        ]
        ; we have enough stack to allocate the SaveState structure
        SUB     sp,sp,#SaveState_size

        ; a1 = pointer to location to hold "SaveState *" address.
        ; The actual structure has been allocated from our stack.
        MOV     v1,sp                           ; v1 = SaveState structure addr
        STR     v1,[a1,#&00]                    ; and remember it

        MOV     ip,#&00000000
        STR     ip,[v1,#SaveState_next]         ; no next process
        STR     ip,[v1,#SaveState_pri]          ; and this is a priority 0 job
	[	{TRUE}	; TimedWait support
	STR	ip,[v1,#SaveState_flags]	; clear state flags
	]	; EOF {boolean}

        ; This should not need protecting, since we should NOT perform any
        ; Floating Point operations during the saving of the process state
        ; (and any process swap between now and entering the scheduler will
        ; preserve our current FP state).
	[	(speedup)
	MOV	a1,#fast_structure_pointer
	LDR	a1,[a1,#&00]			; a1 = ExecRoot structure
	|
        SWI     exec_FindExecRoot
	]	; EOF (speedup)
        LDR     ip,[a1,#ExecRoot_fparea]        ; preserve FP state
        STR     ip,[v1,#SaveState_fparea]
        LDR     ip,[a1,#ExecRoot_initial_dp]	; preserve initial dp register
        STR     ip,[v1,#SaveState_initial_dp]
	[	(memmap)
	LDR	ip,[a1,#ExecRoot_memmap]	; preserve MEMMAP state
	STR	ip,[v1,#SaveState_memmap]
	]	; EOF (memmap)
	[	(fix0001)
	LDR	ip,[a1,#ExecRoot_timeslice]	; timeslice remaining
	STR	ip,[v1,#SaveState_timeslice]	; stored in the SaveState
	]	; EOF (fix0001)

        ; The Scheduler code explicitely disables IRQs (when in SVC mode).
        ; It should not matter what the IRQ state of our caller is.
        MOV     a1,v1
        ; a1 = SaveState structure pointer
        SWI     exec_Scheduler
        ; The above Scheduler call will have blocked. Any processes that
        ; are started will have their own IRQ state loaded. We will return
        ; to this point when we have been "Resume"d.

        ; return to caller
        LDMEA   fp,{v1,fp,sp,pc}^               ; recover callers state

        ; ---------------------------------------------------------------------
	[	{TRUE}	; part of the TimedWait support
        ; void TimedSuspend(SaveState **pp,word timeout) ;
        ; Suspend the current process, placing a suitable timeout watchdog on
	; the TimerQ.
	;
        ; in:   a1 : pointer to a "SaveState *" variable
	;	a2 : timeout in microseconds
	;	IRQs should be disabled
        ; out:  a1 = 1 (true) if process awoken via a Signal
	;	   = 0 (false) if process awoken via a time-out
        ;
TimedSuspend	FnHead
        MOV     ip,sp
        STMFD   sp!,{v1,v2,v3,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        SUB     ip,sp,#SaveState_size	; drop for stack overflow check only
        CMP     ip,sl
        BLLT	__stack_overflow_1
        ]
        ; we have enough stack to allocate the SaveState structure
        SUB     sp,sp,#SaveState_size

        ; a1 = pointer to location to hold "SaveState *" address.
	; a2 = timeout value
        ; The actual structure has been allocated from our stack.
        MOV     v1,sp                           ; v1 = SaveState structure addr
        STR     v1,[a1,#&00]                    ; and remember it

        MOV     ip,#&00000000
        STR     ip,[v1,#SaveState_next]         ; no next process
        STR     ip,[v1,#SaveState_pri]          ; and this is a priority 0 job

        ; This should not need protecting, since we should NOT perform any
        ; Floating Point operations in this thread during the saving of the
	; process state (and any process swap between now and entering the
	; scheduler will preserve our current FP state).
	[	(speedup)
	MOV	a1,#fast_structure_pointer
	LDR	a1,[a1,#&00]			; a1 = ExecRoot structure
	|
        SWI     exec_FindExecRoot
	]	; EOF (speedup)
        LDR     ip,[a1,#ExecRoot_fparea]        ; preserve FP state
        STR     ip,[v1,#SaveState_fparea]
        LDR     ip,[a1,#ExecRoot_initial_dp]	; preserve initial dp register
        STR     ip,[v1,#SaveState_initial_dp]
	[	(memmap)
	LDR	ip,[a1,#ExecRoot_memmap]	; preserve MEMMAP state
	STR	ip,[v1,#SaveState_memmap]
	]	; EOF (memmap)
	[	(fix0001)
	LDR	ip,[a1,#ExecRoot_timeslice]	; timeslice remaining
	STR	ip,[v1,#SaveState_timeslice]	; stored in the SaveState
	]	; EOF (fix0001)

	; Mark this SaveState as having a duplicate on the TimerQ
	MOV	ip,#SSflag_timerQ
	STR	ip,[v1,#SaveState_flags]

	; The Scheduler code does NOT save "a1/r0" into the SaveState, thus
	; allowing us to define the value we wish in "a1" when the process
	; is Resumed.
	MOV	ip,#0	; default is process awoken via a timeout
	STR	ip,[v1,#SaveState_a1]

				; v1 = our SaveState structure pointer
	MOV	v2,a1		; v2 = ExecRoot structure pointer
	MOV	v3,a2		; v3 = our wakeup delta
	B	Sleep_add_SaveState
	; The SaveState structure is allocated from the stack of this thread.
	; It will cease to exist when we return from this function.
	]	; EOF {boolean}

        ; ---------------------------------------------------------------------
        ; void Resume(SaveState *p) ;
        ; Resume the given process, it will not run until we either Suspend or
        ; Yield. Should really only be called by priority 0 processes.
        ; in:   a1 = pointer to SaveState structure
        ; out:  no conditions
        ;
        ; Read the priority level of the process to Resume.
        ; Address the process queues.
        ; Load our process priority
        ; Set the "next" SaveState structure pointer to NULL.
        ; If we are not priority 0 then disable IRQs.
        ; If the process queue at the new priority level is empty then
        ;       address their process queue head and tail at the passed process
        ; else
        ;       address their process queue tail and the tail of the next
        ;       process on their queue at the passed process
        ; If new priority is a higher priority process than the current
        ; process then make the current process priority level equal the new
        ; priority level.
        ; If our priority != 0 then
        ;       enable IRQs
        ;       set our priority level to 1
        ;
        ; In this code the following priorities exist:
        ; 0  = hi-priority (system)
        ; >0 = everyone else
        ;
Resume	FnHead
        MOV     ip,sp
        STMFD   sp!,{v1,v2,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        CMP     sp,sl
        BLLT	__stack_overflow
        ]

        ; Note: "Resume" may now be called from an IRQ mode signal routine.
        ;       In this case IRQs will already be disabled. We should
        ;       therefore not change the IRQ state. However, this does mean
        ;       that IRQs should be disabled by all levels of priority.

        ; a1 = pointer to the SaveState structure
        MOV     v1,a1                           ; v1 = SaveState structure addr
	[	(speedup)
	MOV	v2,#fast_structure_pointer
	LDR	v2,[v2,#&00]			; v2 = ExecRoot structure addr
	|
        SWI     exec_FindExecRoot
        ; a1 = ExecRoot structure
        MOV     v2,a1                           ; v2 = ExecRoot structure addr
	]	; EOF (speedup)

	[	{FALSE}	; TimedWait support
        MOV     a1,#&00000000
        STR     a1,[v1,#SaveState_next]         ; ensure no next process
	]	; EOF {boolean}

        ; If we are in SVC mode, the SWI above will have corrupted the "lk"
        ; register. However, the I and F state should have been preserved.
        ; Check if IRQs are already disabled.
        TST     lk,#Ibit                        ; check if IRQs disabled
        BNE     no_Resume_disable

        SWI     exec_IntsOff                    ; disable IRQs
no_Resume_disable
        ; a1 = undefined
	; a2 = undefined
	; a3 = undefined
	; a4 = undefined
        ; v1 = SaveState structure addr
        ; v2 = ExecRoot structure addr

	[	{TRUE}	; TimedWait support
	; This code relies on IRQs being disabled at this point in time.

	LDR	a2,[v1,#SaveState_flags]
	TST	a2,#SSflag_timerQ	; SaveState held on the TimerQ
	BEQ	Resume_notTimerQ	; NO - then normal processing

	; We should search for the SaveState on the TimerQ. It it does not
	; appear then it has already timed-out, and we should exit without
	; placing the SaveState onto any ProcessQ. It it does appear, we
	; should de-link the SaveState from the TimerQ and place the SaveState
	; onto a ProcessQ with a suitable return value (a1).
	; The fact that the "_TimedWait" and "Signal" functions are both
	; hi-priority, with IRQs disabled at suitable points should protect
	; against funny process re-start race conditions.
	; NOTEs: If we are in the Signal function (which does not have IRQs
	; 	 disabled) then we can have entered Resume with the SaveState
	;	 pointer. Before we get a chance to disable IRQs in Resume
	;	 (which performs queue. manipulations) a timer interrupt may
	;	 have placed the SaveState onto a processQ. In this case we
	;	 will not be able to find the SaveState on the TimerQ. If this
	;	 happens then we should return without Resuming the SaveState.
	;	 Signal should always be executing as a hi-prioirity process,
	;	 thus if we are in this thread after the SaveState has been
	;	 taken from the TimerQ onto a ProcessQ then we will execute
	;	 till completion, before the TimedSuspend SaveState thread
	;	 can be re-started and lose the SaveState memory (since it is
	;	 in the stack frame of the "_TimedWait" call).
	;
	; Search for the SaveState on the TimerQ.
	LDR	a2,[v2,#ExecRoot_timerQ]; SaveState at the head of the TimerQ
	MOV	a3,#&00000000		; no previous SaveState
Resume_search_loop
	; a2 = current SaveState pointer
	; a3 = previous SaveState pointer
	TEQ	a2,#&00000000			; check for the end of the list
	BEQ	Resume_completed		; process has already timed-out

	TEQ	a2,v1				; matches our SaveState?
	LDREQ	a1,[a2,#SaveState_endtime]	; YES : get out-standing delta
	MOVNE	a3,a2				; NO  : make previous SaveState
	LDR	a2,[a2,#SaveState_next]		; reference the next SaveState
	BNE	Resume_search_loop		; and go around the loop
	; a1 = wakeup delta for SaveState being removed
	; a2 = our next SaveState pointer
	; a3 = our previous SaveState pointer
	; v1 = our SaveState pointer as passed to Resume
	; v2 = ExecRoot structure
	TEQ	a3,#&00000000		; check for no previous pointer
	STREQ	a2,[v2,#ExecRoot_timerQ]; EQ - we have removed the TimerQ head
	STRNE	a2,[a3,#SaveState_next]	; NE - update the previous pointer

	TEQ	a2,#&00000000			; check for NULL next SaveState
	LDRNE	a3,[a2,#SaveState_endtime]	; load current wakeup delta
	ADDNE	a3,a3,a1			; add in old SaveState delta
	STRNE	a3,[a2,#SaveState_endtime]	; and update this wakeup delta

	MOV	a2,#1			; true
	STR	a2,[v1,#SaveState_a1]	; mark this thread as Signalled
Resume_notTimerQ
        MOV     a1,#&00000000
        STR     a1,[v1,#SaveState_next]	; ensure no next process
	]	; EOF {boolean}

        ; a1 = undefined
	; a2 = undefined
	; a3 = undefined
	; a4 = undefined
        ; v1 = SaveState structure addr
        ; v2 = ExecRoot structure addr
        ADD     a3,v2,#(ExecRoot_queues + (0 * ProcessQ_size) + ProcessQ_head)
        ; a3 = base of ready "ProcessQ" structs for the available priorities

        ; We should be accessing the queue for the process priority that we
        ; are resuming (i.e. [v1,#SaveState_pri])
        LDR     a4,[v1,#SaveState_pri]
        ADD     a3,a3,a4,LSL #ProcessQ_shift
        ; a3 = base of ready "ProcessQ" structure for the resuming priority

	; Place our SaveState at the end of the referenced queue
	LDR	a4,[a3,#ProcessQ_tail]	; load the current tail node
	STR	v1,[a4,#SaveState_next]	; reference the new tail node
	STR	v1,[a3,#ProcessQ_tail]	; and make the new tail node

        LDR     a3,[v1,#SaveState_pri]     ; get pri of process to be resumed
        LDR     a4,[v2,#ExecRoot_hipri]    ; and highest that could be run
        CMP     a3,a4
        STRCC   a3,[v2,#ExecRoot_hipri]    ; this one is higher priority

        ; At the moment resumed processes (e.g. those waiting on
        ; semaphores) will not be re-scheduled till the next interrupt
        ; (They have only been placed onto the queues above).
        ; If we are calling "Resume" from "HardenedSignal" then IRQs are
        ; disabled. We do NOT want to exit through the scheduler at this point,
        ; since we want this IRQ thread to complete. We should ensure that the
        ; IRQ exit code exits through the Scheduler. ie. normal threads may
	; Yield/Suspend after calling Resume.

	[	{TRUE}	; TimedWait support
Resume_completed
	]	; EOF {boolean}

        TST     lk,#Ibit		; check entry IRQ state
        BNE     no_Resume_enable

        SWI     exec_IntsOn		; enable IRQs
no_Resume_enable

        ; return to caller
        LDMEA   fp,{v1,v2,fp,sp,pc}^

        ; ---------------------------------------------------------------------
        ; void Stop(void) ;
        ; Stop is an irrevocable halt. Called only at priority 0 (hi-pri).
        ; in:   no conditions
        ; out:  no conditions (should NOT return)
        ;
        ; Construct a SaveState structure pointer
        ; Suspend the current process (should not return)
        ;
Stop	FnHead
        MOV     ip,sp
        STMFD   sp!,{a1,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        CMP     sp,sl
        BLLT	__stack_overflow
        ]

        ; Construct a "SaveState *" variable on the stack so that we can
        ; pass the address of it to "Suspend". This has been done by storing
        ; the undefined register "a1" on the stack on entry.
        MOV     a1,sp                           ; address of stacked "a1"
        BL      Suspend
        ; We call "Suspend" purely to enter the process scheduler. We do not
        ; attach this "SaveState" to any of the system structures. This means
        ; that the memory attached to this thread must be de-allocated before
        ; calling "Stop". This means that to stop the re-allocation of this
        ; memory before the "Suspend" occurs, we have to be in a hi-priority
        ; (un-interruptable) process.

        ; We used "BL" above, in-case it ever goes wrong, and we return
        ; from the "Suspend" call.
        ERROR   stop_failed,"** FATAL ** Return from Suspend in Stop"

        ; ---------------------------------------------------------------------
        ; void Yield(void) ;
        ; Yield the processor and let any other processes at this priority
        ; level (or higher) run. This function can be called from any priority
	; level.
        ; in:   no conditions
        ; out:  no conditions
        ;
Yield	FnHead
        MOV     ip,sp
        STMFD   sp!,{a1,a2,v1,v2,v3,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        SUB     ip,sp,#SaveState_size	; drop for check only
        CMP     ip,sl
        BLLT	__stack_overflow_1
        ]
        SUB     sp,sp,#SaveState_size

        ; "a1" and "a2" are preserved over the Yield function call. This is
        ; because it is called from within this file by a function that assumes
        ; they are preserved over this function call. This can be re-done if
	; it is shown that "Yield" is called more from external functions than
	; the internal one.

        ; We need to construct a "SaveState" structure, so that our process
        ; description can be saved when we dispatch any pending processes.
        MOV     v1,sp                           ; v1 = SaveState structure addr
	[	(speedup)
	MOV	v2,#fast_structure_pointer
	LDR	v2,[v2,#&00]			; v2 = ExecRoot structure addr
	|
        SWI     exec_FindExecRoot
        MOV     v2,a1                           ; v2 = ExecRoot structure addr
	]	; EOF (speedup)

        ; We need to place this process at the end of the correct ready queue.
	; NOTE: If we are a hi-priority process, and the hi-priority queue is
	;	empty then we can exit immediately. All other case should still
	;	go through the Scheduler.

        ; v2 = ExecRoot structure address
        LDR     a2,[v2,#ExecRoot_pri]		; current priority
        ADD     v3,v2,#ExecRoot_queues
        ADD     v3,v3,a2,LSL #ProcessQ_shift	; our priority queue

        ; a1 = ProcessQ_size
        ; a2 = current priority level
        ; v2 = ExecRoot structure address
        ; v3 = ProcessQ_head for current priority level

        SWI     exec_IntsOff                    ; queue manipulations coming

        ; Only if we are currently hi-pri (0) do we exit if we have a NULL Q
        LDR     a1,[v3,#ProcessQ_head]          ; next SaveState structure ptr
        CMP     a2,#&00000000			; check current priority
        CMPEQ   a1,#&00000000			; check SaveState pointer
        BEQ     Yield_completed                 ; empty hi-pri queue

        ; We YIELD by placing our process at the end of the queue and calling
	; the Scheduler. The Scheduler will start the highest-priority process
	; pending.
        MOV     a1,#&00000000			; NULL constant
        STR     a1,[v1,#SaveState_next]		; no "next" SaveState
	[	{TRUE}	; TimedWait support
	STR	a1,[v1,#SaveState_flags]	; clear state flags
	]	; EOF {boolean}
	; The following ensures that we get a full timeslice period when we
	; are eventually re-started. This is a reasonable operation to perform
	; here since we will definately be going through the Scheduler.
        STR     a1,[v1,#SaveState_timeslice]	; process timeslice remaining

        STR     a2,[v1,#SaveState_pri]		; remember our priority

        LDR     a1,[v2,#ExecRoot_fparea]        ; remember FP state
        STR     a1,[v1,#SaveState_fparea]

        LDR     a1,[v2,#ExecRoot_initial_dp]	; remember original "dp" value
        STR     a1,[v1,#SaveState_initial_dp]

	[	(memmap)
	LDR	a1,[v2,#ExecRoot_memmap]	; preserve MEMMAP state
	STR	a1,[v1,#SaveState_memmap]
	]	; EOF (memmap)

	; Place our SaveState onto the end of the relevant process queue.
        LDR     a1,[v3,#ProcessQ_tail]
        STR     v1,[a1,#SaveState_next]	; queue_tail->next = v1 ;
        STR     v1,[v3,#ProcessQ_tail]	; queue_tail = v1 ;

	; Update the "hipri" field if our process is of higher priority
	; a2 = our priority level
	LDR	a1,[v2,#ExecRoot_hipri]	; highest that could be run
	CMP	a2,a1
	STRCC	a2,[v2,#ExecRoot_hipri]	; our process has a higher priority

        MOV     a1,v1			; reference our constructed SaveState
        SWI     exec_Scheduler		; and re-schedule our process
	; We have re-started after allowing other processes to execute.
Yield_completed
        SWI     exec_IntsOn			; ensure interrupts are enabled
        LDMEA   fp,{a1,a2,v1,v2,v3,fp,sp,pc}^   ; recover callers state

        ; ---------------------------------------------------------------------
        ; void Sleep(word endtime) ;
        ; Suspend the currently executing process for the given amount of
        ; time (in micro-seconds).
        ; in:   a1 = delta to wakeup
        ; out:  no conditions
        ;
        ; The "ExecRoot_timerQ" contains a linked list of processes waiting
        ; to be woken up. The "SaveState_endtime" of the head process is
        ; decremented every clock tick. When this reaches/crosses zero the
        ; process will be re-started. All the remaining "SaveState_endtime"
        ; entries are deltas over the preceding queue entry.
        ; This function adds the current process into this list at the point
	; described by its wakeup delta provided (endtime).
	;
Sleep	FnHead
        MOV     ip,sp
        STMFD   sp!,{v1,v2,v3,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        SUB     ip,sp,#SaveState_size		; drop for stack check
        CMP     ip,sl
        BLLT	__stack_overflow_1
        ]
        SUB     sp,sp,#SaveState_size           ; take "SaveState" space

        MOV     v1,sp                           ; v1 = "SaveState" structure
        MOV     v3,a1                           ; our wakeup delta from now

	[	(speedup)
	MOV	v2,#fast_structure_pointer
	LDR	v2,[v2,#&00]			; v2 = ExecRoot structure addr
	|
        SWI     exec_FindExecRoot
        MOV     v2,a1                           ; v2 = "ExecRoot" structure
	]	; EOF (speedup)

        ; copy our process information into the new "SaveState"
        LDR     a1,[v2,#ExecRoot_pri]
        STR     a1,[v1,#SaveState_pri]
        LDR     a1,[v2,#ExecRoot_fparea]
        STR     a1,[v1,#SaveState_fparea]
        LDR     a1,[v2,#ExecRoot_initial_dp]
        STR     a1,[v1,#SaveState_initial_dp]
	[	(memmap)
	LDR	a1,[v2,#ExecRoot_memmap]	; preserve MEMMAP state
	STR	a1,[v1,#SaveState_memmap]
	]	; EOF (memmap)
	[	(fix0001)
	LDR	a1,[v2,#ExecRoot_timeslice]	; preserve timeslice remaining
	STR	a1,[v1,#SaveState_timeslice]
	]	; EOF (fix0001)

	[	{TRUE}	; TimedWait support
	MOV	a1,#&00000000
	STR	a1,[v1,#SaveState_flags]	; clear the state flags
Sleep_add_SaveState
	; We enter here from the "TimedSuspend" function
	; v1 = "SaveState" structure address
	; v2 = "ExecRoot" structure address
	; v3 = our wakeup delta
	]	; EOF {boolean}

        SWI     exec_IntsOff                    ; queue manipulation pending

        LDR     a1,[v2,#ExecRoot_timerQ]        ; head process pointer
        CMP     a1,#&00000000                   ; is the queue empty?
        BEQ     Sleep_at_front                  ; YES

        MOV     a2,#&00000000                   ; NULL previous "SaveState"
Sleep_insert_loop
        ; a1 = current "timerQ" process "SaveState" structure address
        ; a2 = previous "timerQ" process "SaveState" structure address
        ; v1 = our "SaveState" structure address
        ; v2 = "ExecRoot" structure address
        ; v3 = our wakeup delta

        ; Check if our delta is less than that of the current "timerQ" entry
        LDR     a3,[a1,#SaveState_endtime]
        CMP     v3,a3

        ; EQUAL
        MOVEQ   a3,#&00000000           ; zero the current "SaveState_endtime"
        BEQ     Sleep_in_front          ; insert our "SaveState" before current

        ; LESS
        SUBCC   a3,a3,v3                ; subtract our delta from current delta
        BCC     Sleep_in_front          ; insert our "SaveState" before current

        ; GREATER
        SUB     v3,v3,a3                ; subtract current delta from our delta
        MOV     a2,a1                   ; copy current entry to previous entry
        LDR     a1,[a2,#SaveState_next] ; load new current entry
        CMP     a1,#&00000000
        BNE     Sleep_insert_loop       ; next process is valid
        B       Sleep_insert            ; we are placed at the end of the queue

Sleep_in_front
        ; Our process can be placed in front of the current "timerQ" entry.
        ; a1 = current "timerQ" process entry
        ; a2 = previous "timerQ" process entry
        ; a3 = "SaveState_endtime" to place into current "timerQ" entry
        ; v1 = our process "SaveState" structure address
        ; v2 = "ExecRoot" structure address
        ; v3 = our wakeup delta

        STR     a3,[a1,#SaveState_endtime]
Sleep_insert
        ; We have to deal with "previous" being NULL (ie. we are being placed
        ; at the front of the queue).

        STR     a1,[v1,#SaveState_next]         ; reference the next entry
        CMP     a2,#&00000000
        STRNE   v1,[a2,#SaveState_next]         ; reference our "SaveState"
        STREQ   v1,[v2,#ExecRoot_timerQ]        ; we must be at the front

        B       Sleep_dispatch

Sleep_at_front
        ; Our process can be placed directly at the front of the "timerQ".
        ; This code assumes that the current "timerQ" head process
        ; "SaveState_endtime" has already been updated.
        ; a1 = "timerQ" head "SaveState" structure address
        ; v1 = our process "SaveState" structure address
        ; v2 = "ExecRoot" structure address
        ; v3 = our wakeup delta

        STR     a1,[v1,#SaveState_next]         ; reference the "timerQ" head
        STR     v1,[v2,#ExecRoot_timerQ]        ; new "timerQ" head process
        ; and fall through to...
Sleep_dispatch
        ; Enter the scheduler. The Scheduler will save our CPU state into the
        ; given SaveState structure. It will then start up the next process.
        ; v1 = our process "SaveState" structure address
        ; v2 = "ExecRoot" structure address
        ; v3 = our wakeup delta
        STR     v3,[v1,#SaveState_endtime]

        MOV     a1,v1
        SWI     exec_Scheduler			; and re-schedule this thread
        SWI     exec_IntsOn			; enabling IRQs on return
        LDMEA   fp,{v1,v2,v3,fp,sp,pc}^

        ; ---------------------------------------------------------------------
        ; word *CreateProcess(word *stack,VoidFnPtr entry,VoidFnPtr exit,
        ;                     word *descript,word argsize) ;
        ; in:  a1       = pointer to the process stack
        ;      a2       = process entry function address
        ;      a3       = process exit function address (actually return addr)
        ;      a4       = pointer to module table pointer and stack-base ptr
        ;      (sp + 0) = size of arguments data section
        ;
        ; Allocate space for the arguments
        ; Construct a SaveState structure for this process
        ; Place the process specific information into the created SaveState
        ; structure
        ; Return the address of the arguments area to the caller
        ;
        ; Stacks are assumed to be always Full Descending
        ;
        ; ------------- <===\ stack pointer passed into CreateProcess (hi-mem)
        ; |  "arg n"  |      \
        ; -------------       \
        ; |           |        \
        ; | ......... |         \ "argsize" bytes of argument data
        ; |           |         /
        ; -------------        /
        ; |  "arg 1"  |       /
        ; -------------      /
        ; |  "arg 0"  |     /
        ; ------------- <==/ arguments pointer returned to caller (lo-memory)
        ; | "argsize" |
        ; -------------
        ; |    "pc"   |
        ; |   "link"  |
        ; |    "sl"   |
        ; |    "dp"   |
        ; |           |
        ; |    "v1"   |
        ; |           |
        ; | SaveState |
        ; ------------- <=== Process SaveState structure pointer
        ; |  ID word  |
        ; ------------- <=== ID word (simple check to ensure struct validity)
        ;
        ; The "SaveState" is constructed below the arguments so as to allow the
        ; direct use of the stack-pointer to reference arguments that cannot be
        ; placed into registers (PCS specific). "argsize" is required to notify
        ; "EnterProcess" of the number of arguments that need to be loaded into
        ; registers. The "SaveState" structure will live immediately beneath
        ; the referenced argument data area and size word.
        ;
        ; The stack-base "sb" value given in "descript[1]" is used to calculate
        ; the stack-limit "sl" for the process. The "descript[1]" entry is
        ; also stored in the "v1" register. This value will automatically be
        ; stacked by called functions, but will be restored when the "EXIT"
        ; function is called.
        ;
        ; An "ID word" will be stored in the constructed structure. This can be
        ; checked by "EnterProcess" before being discarded in the PCS startup
        ; of the referenced process. ("ID word" = address of "ID word")
        ;
        !       0,"CreateProcess assumes word-aligned value for 'argsize'"

CreateProcess	FnHead
        MOV     ip,sp
        STMFD   sp!,{v1,v2,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        CMP     sp,sl
        BLLT	__stack_overflow
        ]

        LDR     v1,[fp,#&04]    ; (fp is set to (sp - 4) above)
        MOV     v1,v1,LSR #2    ; convert to number of words

        ; a1 = pointer to the process stack
        ; a2 = process entry function address
        ; a3 = process exit function address (actually return address)
        ; a4 = pointer to description array
        ; v1 = number of arguments in the data section

        [       (badaddress)
        !       0,"Complain if function ptr less than 64K"
	; We should really check for valid addresses between "UserRAMBase"
	; and "UserRAMTop".
	!	0,"TODO: badaddress checking to be done"
        CMP     a2,#&00010000           ; 64K limit for the moment
        MOVCC   a1,#err_bad_fnptr       ; Unique error code
        SWICC   exec_ExecHalt           ; should NOT return
        ]

        [       (badaddress)
        !       0,"Complain if stack not word aligned"
        TST     a1,#&03
        MOVNE   a1,#err_bad_stack       ; Unique error code
        SWINE   exec_ExecHalt           ; should NOT return
        ]

        ; The stack-pointer "sp" that will be placed in the process
        ; description depends on the number of arguments given "argsize".

        SUB     a1,a1,v1,LSL #2 ; a1 = base of argument data area
        SUB     v2,a1,#&04      ; allocate space for "argsize"
        STR     v1,[v2,#&00]    ; and remember the number of arguments

        ; We assume (knowing the PCS) that a maximum of 4 arguments can
        ; be placed in registers.
        CMP     v1,#&04
        MOVCS   v1,#&04

        ADD     v1,a1,v1,LSL #2 ; generate the real process stack-pointer

        ; a1 = base of argument data area
        ; v1 = updated process stack pointer
        ; v2 = address of "argsize" word

        ; Construct process description (SaveState)
        SUB     v2,v2,#SaveState_size
        ; v2 = SaveState structure address

        BIC     a2,a2,#PSRflags                 ; USR mode; IRQs/FIQs enabled
        STR     a2,[v2,#SaveState_pc]           ; entry function address
        STR     a3,[v2,#SaveState_link]         ; exit function address
        STR     v1,[v2,#SaveState_sp]           ; process stack-pointer
        LDMIA   a4,{a2,a3}                      ; load "descript" entries

        STR     a2,[v2,#SaveState_dp]           ; module table pointer
        STR     a2,[v2,#SaveState_initial_dp]   ; and a second copy
        STR     a3,[v2,#SaveState_v1]           ; the EXIT function wants this
        ADD     a3,a3,#sl_offset
        STR     a3,[v2,#SaveState_sl]           ; stack-limit pointer

	[	(memmap)
	; Read the MEMMAP_regs soft-copy. We take the current state and not
	; a default so that the created process has the same state as the
	; caller of CreateProcess.
	MOV	a4,a1				; preserve a1 over this SWI
	MOV	a1,#HWReg_MEMMAP		; MEMMAP soft-copy index
	MOV	a2,#&00000000			; no bits to clear
	MOV	a3,#&00000000			; no bits to set
	SWI	exec_HWRegisters
	; a2 = current MEMMAP state
	STR	a2,[v2,#SaveState_memmap]	; current MEMMAP state
	MOV	a1,a4				; restore a1
	]	; EOF (memmap)

        MOV     a2,#&00000000
        STR     a2,[v2,#SaveState_next]         ; no next process chain
        STR     a2,[v2,#SaveState_fparea]       ; no FP state (new process)
        STR     a2,[v2,#SaveState_fp]           ; no frame-pointer
	[	(fix0001)
	STR	a2,[v2,#SaveState_timeslice]	; ensure full timeslice given
	]	; EOF (fix0001)
	[	{TRUE}	; TimedWait support
	STR	a2,[v2,#SaveState_flags]	; clear the state flags
	]	; EOF {boolean}

        ; Note: we do not initialise the "endtime" here

        ; a1 = base of argument data area
        ; a2 = &00000000
        ; a3 = undefined
        ; a4 = undefined
        ; v1 = process stack pointer
        ; v2 = SaveState structure address (process information present)

        SUB     v1,v2,#&04                      ; reference the "ID word"
        STR     v1,[v1,#&00]                    ; and place in the contents

        ; a1 = base of argument data area
        LDMEA   fp,{v1,v2,fp,sp,pc}^

        ; ---------------------------------------------------------------------
        ; void EnterProcess(word *args,word pri) ;
        ; Initialise and start a new process.
        ; in:   a1       = pointer to the arguments structure (described above)
        ;       a2       = process priority
        ;
        ; Resume the process referenced by the SaveState structure
        ; Return to the caller
        ;
EnterProcess	FnHead
        MOV     ip,sp
        STMFD   sp!,{v1,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        CMP     sp,sl
        BLLT	__stack_overflow
        ]

        ; a1 = argument data area
        ; a2 = process priority

        ; We should really mask the given priority (a2) so that it is
        ; in our supported range.

        ; We need to copy the PCS arguments registers (maximum of 4)
        ; into the process description.

        LDR     a3,[a1,#-&04]           ; load "argsize" (number of words)
        CMP     a3,#&04
        MOVCS   a3,#&04
        ; a3 = number of arguments to load into registers

        MOV     a4,a1                   ; remember argument data area
        SUB     a1,a1,#(SaveState_size + &04)
        STR     a2,[a1,#SaveState_pri]  ; store process priority

        SUB     v1,a1,#&04              ; index the "ID word"
        LDR     a2,[v1,#&00]            ; and load its value
        CMP     a2,v1
        ; "NE" then we have a system error
        ; "EQ" we are OK
        !       0,"Currently no action on bad ID word"

        ADD     v1,a1,#SaveState_r0

        ; a1 = SaveState structure address
        ; a2 = undefined
        ; a3 = number of arguments to place into process registers
        ; a4 = argument data area address
        ; v1 = base address of SaveState registers

        ; The "SaveState" structure should contain a valid "sp" (one that
        ; has already been adjusted for the args loaded into registers)

        MOV     a3,a3,LSL #2            ; number of bytes
copy_args_loop
        SUBS    a3,a3,#&04              ; decrement a word
        LDRPL   a2,[a4,a3]              ; load argument
        STRPL   a2,[v1,a3]              ; store in process description
        BPL     copy_args_loop          ; and go around again

        ; a1 = SaveState structure address (process description present)
        BL      Resume                  ; and resume the process
        ; NOTE: It is unlikely that the process will have started when we
        ;       return here.

        LDMEA   fp,{v1,fp,sp,pc}^       ; recover callers state

        ; ---------------------------------------------------------------------
	[	(newsys)	; Provide new System call functionality
	; word System(WordFnPtr func,word arg0,word arg1,word arg2) ;
	;
	; Called only from Kernel routines to move from a lo-priority
	; (scheduled) process into a hi-priority (atomic) system process for
	; the duration of function call.
	;
	; Before returning to the caller, we should allow any higher-priority
	; processes that may be pending to be executed. The called process will
	; eventually be re-started with a new timeslice. If there are no higher
	; priority processes pending we should exit, and complete the remainder
	; of the process timeslice. If the timeslice has expired whilst the
	; process was executing as a hi-priority process then we should
	; re-schedule the process allowing other processes at the same priority
	; a timeslice.
	;
System	FnHead
        MOV     ip,sp
        STMFD   sp!,{a1,a2,a3,a4,v1,v2,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        CMP     sp,sl
        BLLT	__stack_overflow
        ]
	; In the function entry above, we preserve the argument registers on
	; the stack while setting up the function call.

	[	(speedup)
	MOV	v1,#fast_structure_pointer
	LDR	v1,[v1,#&00]		; v1 = pointer to ExecRoot structure
	|
        SWI     exec_FindExecRoot	; reference the ExecRoot data structure
        MOV     v1,a1                   ; v1 = pointer to ExecRoot
	]	; EOF (speedup)
        LDR     v2,[v1,#ExecRoot_pri]   ; v2 = current process priority level

        ; If we are already a hi-priority process, we should just call the
        ; function, and exit immediately.
        TEQ     v2,#&00000000           ; are we hi-pri process?
        BNE     System_lopri            ; NO - we are a lo-pri process

	; We are already a hi-priority process, so simply call the function and
	; exit as quickly as possible.
        LDMFD   sp!,{ip}                ; load the function address
        LDMFD   sp!,{a1,a2,a3}          ; and load the parameter values
        ; We do not need to preserve "a2","a3" and "a4" over the function
        ; call. The called function can use all the stack beneath "sp".
        MOV     lr,pc			; generate a suitably return address
        ; do NOT add any instructions between these two
        MOV     pc,ip                   ; call function (with priority 0)
        ; return to caller (with "a1" from the called function)
        LDMEA   fp,{v1,v2,fp,sp,pc}^    ; recover callers state

	; ---------------------------------------------------------------------
	; We need to place our process into hi-priority.
System_lopri
	; v1 = ExecRoot structure address
	; v2 = current process priority
        MOV     a1,#&00000000		; highest priority available
        STR     a1,[v1,#ExecRoot_pri]   ; priority is now level 0 (atomic)

	; We do not need to update the "ExecRoot_hipri" value here, since we
	; will continue to execute. If we are re-scheduled (for what-ever
	; reason), the Scheduler will ensure that "ExecRoot_hipri" is valid
	; when we are due to be re-started.
        LDMFD   sp!,{ip}                ; load the function address
        LDMFD   sp!,{a1,a2,a3}          ; and load the parameter values
        ; We do not need to preserve "a2","a3" and "a4" over the function
        ; call. The called function can use all the stack beneath "sp".
        MOV     lr,pc			; construct a return address
        ; do NOT add any instructions between these two
        MOV     pc,ip                   ; call function (with priority 0)

	; Restore our original process priority level.
        STR     v2,[v1,#ExecRoot_pri]   ; v2 = current process priority level

	SWI	exec_IntsOff		; disable IRQs over queue manipulations

	; Check "ExecRoot_hipri" to see if there are higher-priority processes
	; pending.
	LDR	a2,[v1,#ExecRoot_hipri]	; highest-pri process that could be run
	CMP	a2,v2			; check highests against ours
	BCC	System_higher_pri	; higher-priority processes pending

	; No higher priority processes are pending. We should check if there
	; are processes pending on our priority process queue.
	; v1 = ExecRoot structure address
	; v2 = our process priority
	ADD	a2,v1,#ExecRoot_queues	; address base of the process queues
	ADD	a2,a2,v2,LSL #ProcessQ_shift	; and reference our queue
	LDR	a3,[a2,#ProcessQ_head]	; next SaveState structure pointer
	TEQ	a3,#&00000000		; check for being NULL
	BEQ	System_exit		; empty process queue so exit

	; If our timeslice has expired whilst we were a hi-priority process
	; we should re-schedule.
	LDR	a2,[v1,#ExecRoot_timeslice]	; load timeslice remaining
	TEQ	a2,#&00000000		; and check for zero
	BNE	System_exit		; we have some timeslice remaining
	; and fall through to re-schedule process...
System_higher_pri
	; We should re-schedule our process. This code is called if higher
	; priority processes are pending or our timeslice expired whilst we
	; were a hi-priority process.
	; At the moment this functionality can be achieved by calling Yield.
	; This will entail a small performance hit, but makes the code less
	; confusing at the moment.
	; At the moment we rely on Yield preserving "a1"
	BL	Yield				; re-schedule our process
	; Other processes will have had a chance to execute...
System_exit
	SWI	exec_IntsOn			; re-enable IRQs
        LDMEA   fp,{v1,v2,fp,sp,pc}^            ; recover callers state

	; ---------------------------------------------------------------------
	|	; middle (newsys)
	; ---------------------------------------------------------------------
        ; word System(WordFnPtr func,word arg0,word arg1,word arg2) ;
        ; Called only from Kernel routines to move from user mode into system
        ; mode. System mode is implemented by giving the process maximum
        ; priority (0).
        ;
        ; in:  a1 : function pointer
        ;      a2 : arg0
        ;      a3 : arg1
        ;      a3 : arg2
        ; out: a1 : result of function execution
        ;
        ; Address the "ExecRoot" structure.
        ; Remember the current process priority and give new priority 0.
        ; Call the function with the passed arguments.
        ; If any hi-pri (priority 0) processes are waiting then execute them.
        ; Set the process priority back to its old level.
        ; Return the result to the caller.
        ;
        ; NOTE: System may be called from IRQ mode with IRQs already
        ;       disabled. We should NOT re-enable IRQs.

System	FnHead
        MOV     ip,sp
        STMFD   sp!,{a1,a2,a3,a4,v1,v2,fp,ip,lk,pc}
        SUB     fp,ip,#&04
        [       (stackcheck)
        CMP     sp,sl
        BLLT	__stack_overflow
        ]

        ; a1,a2,a3,a4 - are entry parameters
        ; dp          - module table pointer (must be preserved)
        ; a2,a3,a4    - may be corrupted during any procedure level
        ; a1          - return parameter
        ;               (Note: there is no real requirement to preserve ip)

        ; in the above entry sequence we also stack the entry parameters
        ; to preserve them while setting up the function call state

	[	(speedup)
	MOV	v1,#fast_structure_pointer
	LDR	v1,[v1,#&00]		; v1 = pointer to ExecRoot structure
	|
        SWI     exec_FindExecRoot
        MOV     v1,a1                   ; v1 = pointer to ExecRoot
	]	; EOF (speedup)
        LDR     v2,[v1,#ExecRoot_pri]   ; v2 = current process priority level

        ; If we are already a hi-priority process, we should just call the
        ; function, and exit immediately.
        TEQ     v2,#&00000000           ; are we hi-pri process?
        BNE     System_lopri            ; NO - we are a lo-pri process

        LDMFD   sp!,{ip}                ; load the function address
        LDMFD   sp!,{a1,a2,a3}          ; and load the parameter values
        ; We do not need to preserve "a2","a3" and "a4" over the function
        ; call. The called function can use all the stack beneath "sp".
        MOV     lr,pc
        ; do NOT add any instructions between these two
        MOV     pc,ip                   ; call function (with priority 0)
        ; return to caller (with modified "a1")
        LDMEA   fp,{v1,v2,fp,sp,pc}^    ; recover callers state

System_lopri
        ; Preserve old priority (v2) and set priority to 0
        MOV     a1,#&00000000
        STR     a1,[v1,#ExecRoot_pri]   ; priority is now level 0

	!	0,"TODO: check if 'hipri' needs updating"
	; We should possibly update the hipri value here. Check the Scheduler
	; and IRQ code to see if it matters.

        LDMFD   sp!,{ip}                ; load the function address
        LDMFD   sp!,{a1,a2,a3}          ; and load the parameter values
        ; We do not need to preserve "a2","a3" and "a4" over the function
        ; call. The called function can use all the stack beneath "sp".

        MOV     lr,pc
        ; do NOT add any instructions between these two
        MOV     pc,ip                   ; call function (with priority 0)

	; At the moment this code (if set to {TRUE}) solves the problem of
	; a short Delay program hogging the processor bandwidth at a particular
	; process priority. However, it seems to adversely affect the
	; re-starting of hi-priority processes. ie. a CTRL-C now takes a very
	; long time to be accepted and handled, as opposed to the instantaneous
	; response when this fix is set to {FALSE}.
	!	0,"TODO: sort out System call exit code"
	[	{FALSE}
	; Since we are currently a hi-priority process, calling Yield will
	; only schedule in other hi-priority processes. A better scheme will be
	; become a low-priority process again, and then call Yield. This will
	; ensure that any process higher, or equal to our own priority will be
	; run. It no longer matters that we are not "atomic" in operation,
	; since an interrupt between now and the Yield call will simply
	; re-schedule in any higher priority processes that may be waiting.
        STR     v2,[v1,#ExecRoot_pri]           ; restore old priority level
	; Our process can now be time-sliced and pre-empted.
	BL	Yield				; Yield requires no parameters
	|	; middle {boolean}
        [       (sysyield)
        ; Rather than checking explicitely for priority 0 processes, we
        ; should check for any higher process. The simplest method is to
        ; directly call "Yield" and let it deal with it.
        BL      Yield
        |
        ; while there are processes on Queue 0 call "Yield" to let them execute
        ; a1 = return value from called function
System_loop
        LDR     a2,[v1,#(ExecRoot_queues + (0 * ProcessQ_size))]
        ; a2 = priority 0 ProcessQ_head
        TEQ     a2,#&00000000                   ; NULL marks empty queue
        BLNE    Yield                           ; Yield requires no parameters

        ; PABeskeen wished this loop to be taken out.
        ; This however means that any hi-priority processes started when
        ; we were "Yield"ing will NOT be executed. This attempt is to
        ; stop the possible (though unproven) case of two "System" calls
        ; looping infinitely.
        ;BNE     System_loop
        ]
        STR     v2,[v1,#ExecRoot_pri]           ; restore old priority level
	]	; EOF {boolean}
        ; return to caller (with modified "a1")
        LDMEA   fp,{v1,v2,fp,sp,pc}^            ; recover callers state
	]	; EOF (newsys)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        LNK     hilinktx.s              ; link transfer functions
