        SUBT Helios Executive structures                           > exstruct/s
        ;    Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Helios Executive data structures
        ; --------------------------------
        ;
        ; If any external references to these structures are required (other
        ; than by assembler Executive routines) then a suitable C header must
        ; be generated. These headers must be kept in step with this file (and
        ; should ideally be generated from a single source).
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        ; ---------------------------------------------------------------------

        !       0,"Including exstruct.s"
                GBLL    exstruct_s
exstruct_s      SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; "ProcessQ" entry

        struct          "ProcessQ"
        struct_aptr     "head"          ; pointer to "SaveState" structure
        struct_aptr     "tail"          ; pointer to "SaveState" structure
        struct_null     "size"          ; size of the above structure
        struct_end

        ; To optimise queue accesses we assume that a ProcessQ entry is
        ; a power-of-2 wide.
ProcessQ_shift  *       3
        ASSERT  (ProcessQ_size = 8)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Executive flags:     ("ExecRoot_flags" field)

xb_rr_off       *       (1 :SHL: 0)  ; round robin scheduling disabled (unused)
xb_next         *       (1 :SHL: 1)  ; time to deschedule present lo-pri proc
xb_idle         *       (1 :SHL: 2)  ; indicates idle process
        ; all the other bits are currently undefined (and should be 0)

        ; ---------------------------------------------------------------------
        ; Executive root data structure:

        struct          "ExecRoot"
        ; --  time information ------------------------------------------------
        struct_word     "timer"         ; current time in ticks (1us)
        struct_word     "cstimer"       ; centi-second timer (10000us)
	struct_word	"idletimeout"	; decrementing system idle timeout
        struct_word     "idleLog"       ; logs idle process time in seconds
	[	{TRUE}	; Included for (speedup) work
	; -- fast structure access --------------------------------------------
	; These structure pointers are placed near the start of the structure
	; to make the numbers simpler when they are exported.
	struct_aptr	"helios_root"	; Helios "Root" structure
	struct_aptr	"helios_sys"	; Helios "SYSBASE" structure
	]	; EOF (speedup)
	; -- cached information -----------------------------------------------
	struct_word	"timeout_stage1"; cached "idletimeout" re-load value
	struct_word	"timeout_stage2"; cached "stage 2" uController value
        ; -- process control --------------------------------------------------
        struct_word     "timeslice"     ; max period for lo-priority processes
        struct_word     "pri"           ; current process priority
        struct_word     "fparea"        ; non-zero if FP workspace allocated
        struct_word     "initial_dp"    ; current process module table pointer
        struct_word     "memmap"        ; current MEMMAP state
        struct_word     "hipri"         ; highest pri process which could run
        struct_word     "flags"         ; scheduler and interrupt flags
        ; -- interrupt (IRQ) support ------------------------------------------
	struct_word	"IRQoffcount"	; count of IRQ disable calls
        struct_aptr     "devhand"       ; ptr to device driver handler function
	struct_aptr	"shutdownhand"	; ptr to dev. driver shutdown handlers
        ; -- removable CARD support -------------------------------------------
        struct_aptr     "cardevents"    ; root of "CardEvent" structure chain
        ; -- process queues ---------------------------------------------------
        struct_aptr     "timerQ"        ; for processes to be woken up by timer
        ; Process queues appended here
        struct_null     "queues"        ; base of the process queues
        ; The individual queue entries are "ProcessQ_size" long
        struct_null     "size"          ; used to calculate the structure size
	; ---------------------------------------------------------------------
        struct_end

        ; Note
        ; ----
        ; The "ExecRoot_size" is the size of the structure without the
        ; process queues.
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; process saved state:

        struct          "SaveState"
	; ---------------------------------------------------------------------
        struct_aptr     "next"          ; ptr to the next "SaveState" structure
        struct_word     "endtime"       ; endtime (in micro-seconds)
	struct_word	"timeslice"	; timeslice remaining to process
        struct_word     "pri"		; priority of thread
        struct_word     "fparea"        ; non-zero if FP workspace allocated
        struct_word     "initial_dp"    ; "dp" value given to "CreateProcess"
        struct_word     "memmap"        ; process MEMMAP state
	struct_word	"flags"		; SaveState flag information
	; -- registers of thread being saved ----------------------------------
        struct_null     "a1"		; argument 1
        struct_word     "r0"
        struct_null     "a2"		; argument 2
        struct_word     "r1"
        struct_null     "a3"		; argument 3
        struct_word     "r2"
        struct_null     "a4"		; argument 4
        struct_word     "r3"
        struct_null     "v1"		; variable 1
        struct_word     "r4"
        struct_null     "v2"		; variable 2
        struct_word     "r5"
        struct_null     "v3"		; variable 3
        struct_word     "r6"
        struct_null     "v4"		; variable 4
        struct_word     "r7"
        struct_null     "v5"		; variable 5
        struct_word     "r8"
        struct_null     "dp"            ; module table pointer
        struct_word     "r9"
        struct_null     "sl"            ; stack limit
        struct_word     "r10"
        struct_null     "fp"            ; frame pointer
        struct_word     "r11"
        struct_null     "ip"            ; intermediate ptr (temporary register)
        struct_word     "r12"
        struct_null     "sp"            ; users stack pointer (r13)
        struct_word     "r13"
        struct_null     "link"          ; users link register (r14)
        struct_word     "r14"
        struct_null     "pc"            ; full PC and PSR (r15)
        struct_aptr     "r15"
	; -- USR mode registers saved when SaveState describes SVC thread -----
	struct_aptr	"usr_r13"	; explicit USR mode sp
	struct_aptr	"usr_r14"	; explicit USR mode lk
	; -- end of the SaveState structure -----------------------------------
        struct_null     "size"          ; size of the above structure
        struct_end

	ASSERT	(SaveState_size <= sp_offset)	; check "sp_offset" big enough

	; The "SaveState_flags" field contains various state flags and
	; information.
SSflag_timerQ	bit	0	; SaveState has been placed on the TimerQ, used
				; by the TimedSuspend and Resume functions.
        ; Notes
        ; -----
        ; The process SaveState structure may need to contain a pointer to a
        ; task description structure (one per active task). This would be used
        ; to access the hardware memory map description if a process swap
        ; involves a task swap.
        ;
        ; Tasks execute in different memory map domains.
        ; Processes execute in a specific memory map domain (parent task).
        ;
	; The "endtime" field is used when a process is on the TimerQ.
	; It holds the delta (over the previous TimerQ entry) until this
	; process is re-scheduled.
	;
	; The "timeslice" field is used when normal processes are placed onto
	; a ProcessQ. It contains the amount of time remaining until the
	; process is re-scheduled. A value of zero (&00000000) means that the
	; process timeslice remaining should be reset to a full timeslice when
	; the process is next scheduled.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; FIQ array entry (used in the multiple FIQ handler code)

        struct          "FIQ"
        struct_word     "mask"          ; FIQ source mask (single bit)
        struct_word     "r8"            ; fiq_r8
        struct_word     "r9"            ; fiq_r9
        struct_word     "r10"           ; fiq_r10
        struct_word     "r11"           ; fiq_r11
        struct_word     "r12"           ; fiq_r12
        struct_word     "r13"           ; fiq_r13
        struct_null     "size"          ; size of non-user-supplied section
        struct_null     "code_start"    ; start of FIQ code (user-supplied)
        struct_end

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF exstruct/s
