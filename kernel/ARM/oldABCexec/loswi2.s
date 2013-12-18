        SUBT    Executive SWI handler and routines                   > loswi2/s
        ; Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; -- exec_SVCCall -----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Execute the given function in SVC mode (hi-priority processes only).
        ; in:  r4  : address of function to call in SVC mode
        ;      r0  : arg0
        ;      r1  : arg1
        ;      r2  : arg2
        ;      r3  : arg3
        ;      SVC mode; IRQs undefined; FIQs undefined
        ;      r11,r12 scratch registers
        ;      r13 : FD stack (containing entry r11 and r12)
        ; out: All registers preserved (other than possible return argument in
        ;      r0).
        ;      Callers processor state preserved (apart from any
        ; flags/registers modified by the called function)

code_exec_SVCCall
        ; recover entry r11 and r12
        LDMFD   sp!,{r11,r12}

        ; SVC mode; IRQs undefined; FIQs undefined
        ; r0..r3 as caller
        ; r4..r12 undefined (should be preserved)
        ; svc_r13 : FD stack
        ; svc_r14 : return address
        MOV     pc,r4                           ; and call the code

        ; ---------------------------------------------------------------------
        ; -- exec_GenerateError -----------------------------------------------
        ; ---------------------------------------------------------------------
        ; Generate a system error. This call exits through the system error
        ; vector. It is assumed that higher level run-time systems will place
        ; suitable handler functions onto this vector.
        ; in:  r0..r10 : registers active at instance of SWI instruction
        ;      SVC mode; IRQs undefined; FIQs undefined
        ;      r11,r12 : scratch registers
        ;      r13 : FD stack (containing entry r11 and r12)
        ;      r14 : return address (and PSR of caller)
        ; out: <does NOT return>
        ;
        ; Dump registers to the register save area and execute through the
        ; system error vector.
code_exec_GenerateError
        [       (generrIRQoff)
        AND     r11,svc_r14,#Fbit               ; entry FIQ state
        TEQP    r11,#(Ibit :OR: SVCmode)        ; disable IRQs
        |
        AND     r11,svc_r14,#(Fbit :OR: Ibit)   ; entry FIQ/IRQ state
        TEQP    r11,#SVCmode
        ]
        ; The return address actually points to a system error block.

        LDR     r11,=register_dump
        STMIA   r11,{r0-r10}
        ADD     r0,r11,#(11 * &04)              ; address r11-r15 space
        LDMFD   sp!,{r11,r12}                   ; recover entry r11 and r12
        STMIA   r0!,{r11-r14}^                  ; USR mode registers written
        NOP
        SUB     r1,r14,#&04                     ; PC of SWI instruction
        STR     r1,[r0,#&00]                    ; dumped_r15

        BIC     r0,r1,#PSRflags                 ; remove PSR flags
        ADD     r1,r0,#&08                      ; address the error text
        LDR     r0,[r0,#&04]                    ; and load the error number
        MOV     r3,#&00                         ; recoverable

        B       enter_error_vector      

        ; ---------------------------------------------------------------------
        ; -- exec_ExecHalt ----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Terminate executive (enter debugging mode).
        ; in:  r0 : error condition
        ;      SVC mode; IRQs undefined; FIQs undefined
        ;      r11,r12 : scratch registers
        ;      r13 : FD stack (containing entry r11 and r12)
        ;      r14 : return address (and PSR of caller)
        ; out: <does NOT normally return>
        ;
        ; Dump registers to the register save area and execute through the
        ; system error vector
code_exec_ExecHalt
        AND     r11,svc_r14,#Fbit               ; entry FIQ state
        TEQP    r11,#(Ibit :OR: SVCmode)        ; disable IRQs

        LDMFD   sp!,{r11,r12}           ; recover entry r11 and r12

        ; The value in "r0" marks the type of error we have had.
        ; At the moment we provide a generic message, but preserve the error
        ; number. This system should be upgraded to actually decode the
        ; error number into a message.

        STMFD   sp!,{r0}                ; user error number

        ; We should store the USER registers in the register dump
        LDR     r0,=register_dump
        STMIA   r0,{r0-r14}^            ; USR mode registers written
        NOP
        ; and the PC + PSR of the instruction after the SWI instruction
        STR     r14,[r0,#(dumped_r15 - register_dump)]

        LDMFD   sp!,{r0}                ; user error number
        ADRL    r1,message_halted

        ; and fall through to generate the error

        ; ---------------------------------------------------------------------
        ; -- enter_error_vector -----------------------------------------------
        ; ---------------------------------------------------------------------

enter_error_vector
        ; in:   r0      = 32bit error number
        ;       r1      = error message pointer
        ;       r2      = undefined
        ;       r3      = non-recoverable boolean
        ;       r4->r12 = undefined
        ;       svc_r13 = valid FD stack
        ;       svc_r14 = callers return address and PSR
        ;       SVC mode; IRQs undefined; FIQs as caller
        ; out:  <does not return>

        ; If the error occurred in SVC, FIQ or IRQ it is unrecoverable.
        ; If the error occurred in USR mode it may be recoverable. Note: This
        ; may cause further errors if register/memory corruption has occurred
        ; ("dp" lost or address exception in the stack).
        ; If the "sp" of the USR thread is beneath "sl" then the process
        ; is un-recoverable (we assume that the error has occurred in the
        ; stack-overflow handler).
        ;
        ; Recoverable errors are passed as signals to the run-time system
        ; of the currently executing task. They ARE the thread of the error
        ; process. We can use their stack to continue the processing.
        ; USR mode un-recoverable errors are passed through the handler (all
        ; they do is terminate the task the error thread was active in).
        ;
        ; For IRQ mode errors we should look at the "dp" register. If it
        ; is != -1 then we have a handle onto the task that owns the interrupt
        ; handler. We can call the error handler with the un-recoverable
        ; state so that the IRQ task is killed. If the "dp == -1" then the
        ; error has occurred in the Executive. The best we can do is to
        ; stop Helios immediately (without calling the handler) and provide a
        ; register dump for the user.
        ; If FIQ code is viewed as a system specific feature (Executive
        ; only) then we treat FIQ errors like the IRQ "dp == -1" errors.
        ; If we ever provide FIQ externally (like IRQ), the same scheme would
        ; have to be applied.
        ; All SVC code exists in the Executive. No external code should
        ; execute in SVC mode. This allows SVC code to be treated like the
        ; IRQ "dp == -1" case.
        ; The major flaw with this scheme is that "r9/dp" becomes an unusable
        ; register, or code that does use it guarantees never to generate
        ; any system errors.

        ; If error thread had IRQs/FIQs enabled, that is what we want
        AND     r2,svc_r14,#INTflags            ; entry IRQ and FIQ state
        TEQP    r2,#SVCmode                     ; preserve SVC mode

        LDR     r2,=register_dump       ; always address the register dump
        AND     ip,svc_r14,#SVCmode     ; ip == error thread processor mode

        ADD     pc,pc,ip,LSL #2         ; branch through handler table
        NOP                             ; padding (DO NOT REMOVE)
        B       handleUSR
        B       handleFIQ
        B       handleIRQ
        B       handleSVC

handleIRQ       ; call the handler if "dp != -1"
        MOV     r3,#-1                  ; ensure error is unrecoverable
        LDR     dp,[r2,#(dumped_r9 - register_dump)]
        ; we should call the handler in SVC mode
        CMP     dp,#-1                  ; can we reference the task?
        BNE     call_error_handler      ; YES - so call the handler
        ; otherwise it is a serious Executive error
handleSVC
handleFIQ
        ; Fatal system error. There is nothing we can do but report it to
        ; the user and then stop the system.
        ; We should disable all interrupt sources, and then loop forever.

        ; DISABLE INTERRUPT SOURCES

        B       FatalError      ; provides screen register dump

handleUSR       ; we can always call the handler for these cases

        !       0,"TODO: sort out why 'dp' restoration isn't working"
        [       {FALSE}
        ; load the "dp" given to the process during "CreateProcess"
        ; (Note: IRQs are enabled if the error thread had them enabled)
        LDR     dp,=ROOT_start
        LDR     dp,[dp,#ExecRoot_initial_dp]
        |
        !       0,"TODO: error handling should restore original dp"
        LDR     dp,[r2,#(dumped_r9 - register_dump)]
        ]       ; {TRUE/FALSE}
        ; we should call the handler in USR mode
call_error_handler
        ADD     ip,r2,#(dumped_r4 - register_dump)
        LDMIA   ip,{r4,r5,r6,r7,r8}
        ADD     ip,r2,#(dumped_r10 - register_dump)
        LDMIA   ip,{sl,fp}
        ; Since we are in SVC mode, the error thread "sp" and "lk" have not
        ; been corrupted.

        ; We should enter the Error handler function with the following state:
        ; a1 = int signum
        ; a2 = char *errtext
        ; a3 = word *regset
        ; a4 = word  recoverable
        ; v1 = preserved
        ; v2 = preserved
        ; v3 = preserved
        ; v4 = preserved
        ; v5 = preserved
        [       {FALSE}
        ; dp = loaded from "ExecRoot_initial_dp"
        |
        !       0,"TODO: load dp from original dp value"
        ; dp = loaded from register_dump
        ]
        ; sl = preserved
        ; fp = preserved
        ; ip = UNDEFINED
        ; sp = preserved
        ; lk = preserved

        LDR     ip,=software_vectors
        TEQ     r3,#&00000000           ; recoverable

        ; Call the handler in USR mode (even if it is non-recoverable)
        LDR     ip,[ip,#(vec_systemError * &04)]
        LDR     r3,[r2,#(dumped_r14 - register_dump)]
        AND     r3,r3,#HIflags
        ORR     ip,ip,r3                ; error thread PSR state
        BIC     ip,ip,#SVCmode          ; and ensure in USR mode
        MOVEQ   r3,#&00000000           ; mark as recoverable again
        MOVNE   r3,#&FFFFFFFF           ; mark as non-recoverable again
        MOVS    pc,ip                   ; and enter the handler function

        ; ---------------------------------------------------------------------
        ; -- SystemError ------------------------------------------------------
        ; ---------------------------------------------------------------------

        LTORG
        GET     lodebug.s               ; lo-level debugger

        ; ---------------------------------------------------------------------
        ; -- exec_VectorPatch -------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Update a software vector.
        ; in:   r0  = vector number
        ;       r1  = address to place in vector (&00000000 to read old value)
        ;       r11 = undefined
        ;       r12 = undefined
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = callers return address (and PSR)
        ; out:  r0  = preserved
        ;       r1  = old contents of the vector
        ;
        ; This allows the software vectors (installed by the Executive) to be
        ; patched at run_time. Hardware vectors can be changed by specific
        ; routines (since these live at a known fixed address, and should not
        ; really be changed except in extreme circumstances).
        ;
        ; Note: The undefined instruction vector is in low memory for
        ;       addressability reasons, not with the others.
        ;
local_VectorPatch	; simulated entry for internal code
	STMFD	sp!,{r11,r12}
code_exec_VectorPatch
        LDR     r12,=software_vectors
	CMP	r0,#vec_IRQ_slots
	BCS	vectorpatch_IRQslot
	ADD	r12,r12,r0,LSL #2	; vector in table: r12 = r12 + (r0 * 4)
vectorpatch_normal
        ; r12 = vector table entry address
        CMP     r1,#&00000000

        MOV     r11,r1

        ; Note: We should NOT need to protect this code with IRQs off, since
        ;       the vector update occurs with a single instruction. It could,
        ;       though, lead to the wrong "old contents" being returned.
        ;       However, this is a problem anyway, since it requires the
        ;       users to restore the "old contents" in the order they patched
        ;       the vector to make any real use of the "old contents" value.
        LDR     r1,[r12,#&00]           ; old contents
        STRNE   r11,[r12,#&00]          ; and store new value if non-zero

exit_vectorpatch
        LDMFD   sp!,{r11,r12}           ; recover entry r11 and r12
        BICS    pc,link,#Vbit           ; and return with V clear

vectorpatch_IRQslot
	CMP	r0,#vec_special_slots
	BCS	vectorpatch_special

	; r1 = address of the routine to be referenced
	STMFD	sp!,{r2,r3,r4}		; generate some work registers
	SUBS	r0,r0,#vec_IRQ_slots	; get the absolute IRQ vector slot
	LDR	r2,=IRQ_vectors		; where the IRQ vectors live in memory
	ADD	r2,r2,r0,LSL #2		; reference the desired slot
	TEQ	r1,#&00000000		; check if we are just reading
        ADDNE   r4,r2,#&08              ; source = source + 8
        SUBNE   r3,r1,r4                ; offset = destination - source
        MOVNE   r3,r3,LSR #2            ; offset = offset >> 2
        ORRNE   r3,r3,#(cond_AL :OR: op_b)
	LDR	r1,[r2,#&00]		; read the old contents
        STRNE   r3,[r2,#&00]		; write relevant table slot 0
	; r1 = old vector contents (as a branch)
	BIC	r1,r1,#(cond_AL :OR: op_b)	; clear instruction information
	ADD	r1,r2,r1,LSL #2		; and add onto the original address
	ADD	r1,r1,#&08		; remembering to add in the pipe-line
	; r1 = old vector contents (as an address)
        LDMFD   sp!,{r2,r3,r4,r11,r12}  ; recover entry registers
        BICS    pc,link,#Vbit           ; and return with V clear

vectorpatch_special
        TEQ     r0,#vec_undef_instruction  ; Test this one specially
        MOVEQ   r12,#software_vector_undef ; Vector in low memory
	BEQ	vectorpatch_normal
	; special patch not understood
        LDMFD   sp!,{r11,r12}           ; recover entry r11 and r12
        ORRS    pc,link,#Vbit           ; and return with V set

        ; ---------------------------------------------------------------------
        ; -- exec_DefineHandler -----------------------------------------------
        ; ---------------------------------------------------------------------
        ; Define the root interrupt handler. This is a C function called
        ; within the IRQ handler.
        ; in:   r0  = function address
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = callers return address (and PSR)
        ; out:  r0  = preserved
        ;       V clear = OK, handler registered
        ;       V set   = FAILED, handler already defined
        ;
code_exec_DefineHandler
        LDR     r11,=ROOT_start
        LDR     r12,[r11,#ExecRoot_devhand]
        TEQ     r12,#&00000000
        LDMNEFD sp!,{r11,r12}           ; recover entry r11 and r12
        ORRNES  pc,link,#Vbit           ; and return with V set

        STR     r0,[r11,#ExecRoot_devhand]

        LDMFD   sp!,{r11,r12}           ; recover entry r11 and r12
        BICS    pc,link,#Vbit           ; and return with V clear

        ; ---------------------------------------------------------------------
        ; -- exec_DefineShutdownHandler ---------------------------------------
        ; ---------------------------------------------------------------------
        ; Define the root device driver shutdown handler. This is a C function
	; called within the PowerDown handler.
	; NOTE: Ideally the functionality of this call should be merged with
	;	the similar call "exec_DefineHandler".
        ; in:   r0  = function address
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = callers return address (and PSR)
        ; out:  r0  = preserved
        ;       V clear = OK, handler registered
        ;       V set   = FAILED, handler already defined
        ;
code_exec_DefineShutdownHandler
	[	(shutdown)
        LDR     r11,=ROOT_start
        LDR     r12,[r11,#ExecRoot_shutdownhand]
        TEQ     r12,#&00000000
        LDMNEFD sp!,{r11,r12}           ; recover entry r11 and r12
        ORRNES  pc,link,#Vbit           ; and return with V set

        STR     r0,[r11,#ExecRoot_shutdownhand]
	]	; EOF (shutdown)
        LDMFD   sp!,{r11,r12}           ; recover entry r11 and r12
        BICS    pc,link,#Vbit           ; and return with V clear

        ; ---------------------------------------------------------------------
        ; -- exec_EnterSVC ----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Enter SVC mode for the caller.
        ; in:   all registers undefined
        ; out:  all registers preserved
        ;
        ; This call returns to the caller with SVC mode
        ; (as RISC OS "OS_EnterOS")
code_exec_EnterSVC
        LDMFD   sp!,{r11,r12}           ; recover entry r11 and r12
        BIC     svc_r14,svc_r14,#Vbit   ; clear V
        ORRS    pc,svc_r14,#SVCmode     ; return setting SVC mode bits

        ; ---------------------------------------------------------------------
        ; -- exec_FindExecRoot ------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Return the address of the ExecRoot data structure.
        ; in:  r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: r0  = address of Executive "ExecRoot" data structure
code_exec_FindExecRoot
        LDR     r0,=ROOT_start
        LDMFD   sp!,{r11,r12}           ; recover entry r11 and r12
        BICS    pc,link,#Vbit           ; and return with V clear

        ; ---------------------------------------------------------------------
        LNK     loswi3.s
