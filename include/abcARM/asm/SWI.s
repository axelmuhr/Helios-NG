        SUBT    Helios Executive SWIs                   > SWI/s
        ;       (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; Helios Executive SWIs
        ;
        ; Author:               James G Smith
        ; History:	901219  (BJK) Add microlink support SWIs
	;	        900414  (BJK) Add DisableIRQ and EnableIRQ
        ;               900320  Add "screen" related SWIs
        ;               890629  Created (generate initial Executive SWIs)
        ;
        ; This file expects the global variable "SWItable" to be defined.
        ; If it is {FALSE} then the inclusion of this file will simply
        ; generate manifests for the SWIs. If {TRUE} then this file will
        ; generate a pure branch table representing the SWIs.
        ;
        ; ---------------------------------------------------------------------

	ASSERT	(arm_s)		; ensure "arm.s" is included
        ASSERT  (exmacros_s)    ; ensure "exmacros.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including SWI.s"
        GBLL    SWI_s
SWI_s   SETL    {TRUE}

        ; ---------------------------------------------------------------------

        [       (SWItable)
        !       0,"SWI.s: generating a SWI branch table"
        |
        !       0,"SWI.s: generating SWI manifests"
        ]

        ; ---------------------------------------------------------------------
        ; Description:
        ;
        ; SWI numbers are a 24bit field stored in the SWI instruction.
        ; We should define some usage of these bits to allow the current
        ; requirements and still allow for possible extensions in the future:
        ;
        ;  23.......20.19.18.17.16.........................................00
        ;  ------------------------------------------------------------------
        ;  |    os    |he|sp|eb|                  function                  |
        ;  ------------------------------------------------------------------
        ;
        ; "os"  bits 23..20     Operating System Identifier
        ;                               0       - Arthur/RISC OS
        ;                               1       - Arx
        ;                               2       - RISC iX
        ;                               3       - Helios
        ; 
        ; If the "os" field matches the Helios Operating System number then:
        ; "he"  bit 19          0 = Helios Executive    1 = non-Executive
        ;
        ; "sp   bit 18          expansion (spare) bit (should be zero)
        ; 
        ; "eb"  bit 17          Error Bit (unused in Helios, should be zero)
        ;                       This bit is used in Arthur/RISC OS SWIs to
        ;                       signify wether the OS kernel is to action
        ;                       errors. Helios will always return errors to
        ;                       the SWI caller.
        ; 
        ; "function" bits 16..0 function number.
        ;                       (&00000 => &1FFFF SWI number range)
        ; 
        ; Error state is always returned from the SWI call in V (bit 28 of
        ; the PSR):
        ;       set (1) = ERROR, unset (0) = OK
        ; 
        ; r0..r10 are defined as IO parameters for the individual SWI function
        ; r11,r12 are preserved over the SWI call
        ; svc_r13 is preserved over the SWI call
        ; svc_r14 is corrupted by the SWI call
        ; 
        ; Even though the Helios Executive will be the only operating system
        ; resident, it may be better to conform to the Acorn standard. This
        ; would allow future operating environments to provide multiple OS
        ; support (e.g. Helios executing Acorn Unix binaries).
        ; 
        ; If SVC mode is only ever used by system routines, then the SVC stack
        ; could be a small limited fixed address stack that was automatically
        ; loaded when a SWI occured. This however disallows nested SWI calls.
        ; ---------------------------------------------------------------------

        [       (:LNOT: SWItable)
swi_osmask      *       &00F00000
swi_os_helios   *       &00300000       ; ABC ARM Helios
swi_os_riscos   *       &00000000       ; Acorn RISC OS
swi_os_arx      *       &00100000       ; ARX
swi_os_riscix   *       &00200000       ; Acorn RISC iX

swi_exec        *       (0 :SHL: 19)    ; Executive SWI
swi_noexec      *       (1 :SHL: 19)    ; non-Executive SWI

swi_spare       *       (1 :SHL: 18)    ; Expansion flag

swi_Xbit        *       (1 :SHL: 17)    ; error state return flag (unused)

swi_function    *       &0001FFFF       ; function number mask
        ]

        ; ---------------------------------------------------------------------
        ; Interface between "hiexec" (Helios) and "Executive"
        ; ---------------------------------------------------

        defSWI  exec_FindExecRoot       ; return address of ExecRoot structure
        defSWI  exec_ExecHalt           ; stop Executive with fatal error

        defSWI  exec_SizeMemory         ; return memory size in bytes
	[	{FALSE}	; Removed for (speedup) work
        defSWI  exec_NucleusBase        ; find the nucleus code
	]	; EOF {boolean}
        defSWI  exec_RAMBase            ; available RAM base address
        defSWI  exec_FindROMItem        ; search for a ROM entry
        defSWI  exec_FindNEXTItem       ; return details on a specific index
        defSWI  exec_WCompare           ; wildcard compare

        defSWI  exec_IntsOff            ; disable IRQs
        defSWI  exec_IntsOn             ; enable IRQs

        defSWI  exec_NumPris            ; return the number of priority levels
        defSWI  exec_Scheduler          ; enter Scheduler
        defSWI  exec_SVCCall            ; execute function in SVC mode
        defSWI  exec_EnterSVC           ; enter SVC mode

        defSWI  exec_VectorPatch        ; update software vector contents
        defSWI  exec_DefineHandler      ; register root interrupt handler

        defSWI  exec_CPUTime            ; time elapsed since system startup
        defSWI  exec_InitClock          ; Initialise system timer
        defSWI  exec_InitBackplane      ; Initialise link adaptor

        defSWI  exec_LinkWriteC         ; write character to link adaptor
        defSWI  exec_LinkReadC          ; read character from link adaptor
        defSWI  exec_LinkWPoll          ; check if character can be written
        defSWI  exec_LinkRPoll          ; check if character can be read
        defSWI  exec_StartLinkTx        ; start link interrupt transfer
        defSWI  exec_StartLinkRx        ; start link interrupt transfer
        defSWI  exec_AbortLinkTx        ; abort the link interrupt transfer
        defSWI  exec_AbortLinkRx        ; abort the link interrupt transfer

        defSWI  exec_GenerateError      ; generate a Helios system error

        defSWI  exec_DisableIRQ         ; disable specified IRQ sources
        defSWI  exec_EnableIRQ          ; enable specified IRQ sources

        defSWI  exec_DisplayInfo        ; provide display (screen) information

        defSWI  exec_Version            ; Executive version information

        defSWI  exec_AttachSFIQ         ; define a single FIQ handler
        defSWI  exec_AttachFIQ          ; define a multiple FIQ handler
        defSWI  exec_ReleaseFIQ         ; release a FIQ handler
        defSWI  exec_DefaultFIQStack    ; read the default FIQ stack (r13)

        defSWI  exec_TerminalOut        ; single character write to 2nd link
        defSWI  exec_TerminalIn         ; single character read from 2nd link
        defSWI  exec_Output             ; string write to 2nd link
        defSWI  exec_WriteHex8          ; 32bit HEX string written to 2nd link
        defSWI  exec_WriteDecimal       ; 32bit decimal written to 2nd link
        defSWI  exec_NewLine            ; LF character written to 2nd link

        defSWI  exec_Disassemble        ; instruction disassembly to 2nd link
	defSWI	exec_SingleStep		; simulate an ARM instruction
	defSWI	exec_ResetARM		; initialise the simulator state
	defSWI	exec_SetNewPC		; update the simulator PC
	defSWI	exec_RegisterDump	; display the simulator registers
	defSWI	exec_FlagStatus		; decode the simulator PSR flags
	defSWI	exec_ModeStatus		; decode the simulator process mode

        defSWI  exec_HWRegisters        ; Hardware register manipulation
	defSWI	exec_ROHWRegisters	; Interrogate READ ONLY HW locations
        defSWI  exec_ClaimHWMemory      ; switch to HW memory map mode
        defSWI  exec_ReleaseHWMemory    ; switch to protected memory map mode

	defSWI	exec_SizeFastMemory	; find and size the FastRAM

	defSWI	exec_ML_StartTx		; Start transmission of microlink msg
	defSWI	exec_ML_SetUpRx		; Set up reception request
	defSWI	exec_ML_PollRx		; Test status of reception
	defSWI	exec_ML_ResumeAfterRx	; Supply resumption SaveState ptr
	defSWI	exec_ML_RegisterHandler	; Supply handler for recvd messages 
	defSWI	exec_ML_DetachHandler	; Remove handler 
	defSWI	exec_ML_Reset		; Send break signal to microcontroller

	defSWI	exec_VerifyCARD		; Verify CARD slot
	defSWI	exec_CARDAreaInfo	; Provide information on a CARD AREA
	defSWI	exec_ResetCPU		; reset the Helios/Executive system

        defSWI  exec_DefineShutdownHandler ; register root shutdown handler

	defSWI	exec_ResetIdleTimeout	; reset system idle timer value

	defSWI	exec_ClaimDMAchannel	; claim DMA channel for given use
	defSWI	exec_ReleaseDMAchannel	; release claimed DMA channel
	defSWI	exec_ClaimEXsource	; claim external interrupt source
	defSWI	exec_ReleaseEXsource	; release claimed interrupt source

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Related information
        ; -------------------
        ;
	; The Executive root structure should no longer be referenced via a SWI
	; by internal users. The SWI is still provided to allow external low
	; frequence callers binary portability. The fast access address is held
	; in a fixed location in low-memory.
	[	(:LNOT: SWItable)
fast_structure_pointer	*	&1F8	; this is a nasty constant
	]	; EOF (:LNOT: SWItable)
	;
        ; exec_FindExecRoot
        ; -----------------
        ; in:  no conditions
        ; out: a1 = address of Executive "ExecRoot" data structure
	;
        ; ---------------------------------------------------------------------
        ; exec_ExecHalt
        ; -------------
        ; The "exec_ExecHalt" SWI is vectored through the "vec_systemError"
        ; vector.
        ; The value passed in "a1" gives the actual fatal error number.
        ; in:  a1 = fatal error code
        ; out: SHOULD NOT RETURN

        [       (:LNOT: SWItable)
halt_executive  *       SIGTERM ; request to stop the Executive
halt_failed     *       SIGQUIT ; returned from "ExecHalt" call
stop_failed     *       SIGQUIT ; returned from "Stop" call
err_notimp      *       SIGABRT ; not implemented in this version
err_trap0       *       SIGABRT ; not a supervisor mode process
err_mode        *       SIGABRT ; a lo-priority process
err_readfailed  *       SIGABRT ; link adaptor byte read failed
err_bad_fnptr   *       SIGABRT ; invalid fn ptr to "CreateProcess"
err_bad_stack   *       SIGABRT ; invalid "CreateProcess" stack

err_unknownSWI  *       SIGABRT ; unknown SWI
err_SWInotimp   *       SIGABRT ; SWI extensions not implemented
err_divzero     *       SIGFPE  ; division by zero

sys_btz         *       SIGTRAP ; Branch Through Zero
sys_undefins    *       SIGILL  ; Undefined Instruction
sys_dataabort   *       SIGSEGV ; Data Fetch Abort
sys_prefetch    *       SIGSEGV ; Instruction PreFetch Abort
sys_address     *       SIGSEGV ; Address Exception

sys_inexact     *       SIGFPE  ; FP inexact operation
sys_invop       *       SIGFPE  ; FP invalid operation
sys_overflow    *       SIGFPE  ; FP overflow
sys_dvz         *       SIGFPE  ; FP division by zero
sys_underflow   *       SIGFPE  ; FP underflow
sys_nostack     *       SIGSTAK ; FP no stack for FP state
        ]

        ; ---------------------------------------------------------------------
        ; exec_LinkWriteC
        ; ---------------
        ; Write character to link adaptor.
        ; Note: This call will block until the adaptor is ready to accept the
        ;       character.
        ; in:  a1 = Character to transmit via link adaptor
        ; out: a1 = preserved
        ;      V clear => character written succesfully
        ;      V set   => unable to write character

        [       (:LNOT: SWItable)
tab             *       &09
lf              *       &0A
ff              *       &0C     ; formfeed
cr              *       &0D
space           *       &20
quote           *       &22
wcmult          *       "*"     ; multiple wildcard character
wcsing          *       "#"     ; single wildcard character
del             *       &7F
        ]

        ; ---------------------------------------------------------------------
        ; exec_LinkReadC
        ; --------------
        ; Read character from link adaptor.
        ; Note: This call will block until the adaptor is able to provide a
        ; character.
        ; in:   no conditions
        ; out:  V clear => a1 = character read from link adaptor.
        ;       V set   => a1 = undefined (unable to read character)
        ;
        ; ---------------------------------------------------------------------
        ; exec_cputime
        ; ------------
        ; Return the time elapsed since system startup.
        ; Note: This will be the number of centi-seconds elapsed since
        ;       "exec_InitClock" was called.
        ; in:   no conditions
        ; out:  a1 = centi-seconds elapsed
        ;
        ; ---------------------------------------------------------------------
        ; exec_NumPris
        ; ------------
        ; Return the number of priority levels (n).
        ; in:   no conditions
        ; out:  a1 = number of priority levels
        ;
        ; ---------------------------------------------------------------------
        ; exec_IntsOffCall
        ; ----------------
        ; Disable IRQs.
        ; in:   no conditions
        ; out:  all registers preserved
        ;
        ; ---------------------------------------------------------------------
        ; exec_IntsOnCall
        ; ---------------
        ; Enable IRQs.
        ; in:   no conditions
        ; out:  all registers preserved
        ;
        ; ---------------------------------------------------------------------
        ; exec_DisableIRQ
        ; ---------------
        ; Disable the specified IRQ sources.
        ; in:  r0  = mask with bits set for sources to be disabled
        ; out: r0  = old IRQ mask
        ;
        ; ---------------------------------------------------------------------
        ; exec_EnableIRQ
        ; --------------
        ; Enable the specified IRQ sources.
        ; in:  r0  = mask with bits set for sources to be enabled
        ; out: r0  = old IRQ mask
        ;
        ; ---------------------------------------------------------------------
        ; exec_Scheduler
        ; --------------
        ; Enter the Scheduler. Examine the process queues and start the next
        ; process due for execution.
        ; in:   a1 = SaveState structure pointer for current process
        ; out:  DOES NOT RETURN DIRECTLY
	;	r1 -> r15 as entry to SWI (see r0 NOTE below)
	;
        ; If no "SaveState *" is passed to this SWI (i.e. a1 == NULL) then
        ; this process is "lost". The process can also be "lost" if no
        ; reference is made to the "SaveState *" in a suitable management
        ; queue.
        ;
	; r0 NOTE
	; -------
	; Only r1-r15 are stored automatically by this call. Any "r0" value
	; that is required must be pre-saved into the "SaveState" structure.
	; The call is structured like this so that the full return register
	; set can be defined, where r0 is not simply restored as the
	; "SaveState" pointer used on entry to the SWI. This allows the call
	; to be used to create totally new process threads.
	;
        ; ---------------------------------------------------------------------
        ; exec_SVCCall
        ; ------------
        ; Execute the given function in SVC mode.
        ; in:   a1 = a1 for function
        ;       a2 = a2 for function
        ;       a3 = a3 for function
        ;       a4 = a4 for function
        ;       v1 = Address of function
        ; out:  a1 = dependent on function
        ;       a2 = undefined
        ;       a3 = undefined
        ;       a4 = undefined
        ;       v1 = preserved
        ;
        ; ---------------------------------------------------------------------
        ; exec_EnterSVC
        ; -------------
        ; Enter SVC mode for the current process.
        ; in:  no conditions
        ; out: SVC mode, svc_r14 undefined.
        ;
        ; The caller should return to their previous processor mode to recover
        ; their r14 (link) address.
        ;
        ; ---------------------------------------------------------------------
        ; exec_VectorPatch
        ; ----------------
        ; This SWI allows the nucleus/kernel to replace individual Executive
        ; routines with their own code.
        ; in:   a1 = vector number
        ;       a2 = address to place in vector (0 to read the current value)
        ; out:  a1 = preserved
        ;       a2 = old contents of vector
        ;
        [       (:LNOT: SWItable)
                        ^       0
vec_HeliosExecSWI       #       &01     ; Helios Executive SWI
vec_HeliosSWI           #       &01     ; Helios SWI (applications)
vec_otherSWI            #       &01     ; non-Helios SWI (other OS's)
vec_systemError         #       &01     ; system error vector (fatal errors)
vec_CARDHandler         #       &01     ; removable CARD handler
        ;
number_of_vectors       #       &00     ; (last vector number + 1)
	;
vec_IRQ_slots		#	&80	; -> &BF for IRQ slots
        ;
vec_special_slots	#	&C0	; -> &FF for special vectors
vec_undef_instruction   *       &FF     ; undefined instruction vector
        ;
        ASSERT  (number_of_vectors <= vec_undef_instruction)    ; maximum limit
        ]       ; EOF (:LNOT: SWItable)
        ;
        ; All of these vectors are entered with IRQs disabled. IRQs should be
        ; enabled early if possible (especially true for the system error
        ; vector). The rest of the processor state is undefined and depends
        ; on the particular vector.
        ; Note: The undefined instruction vector is a special case.
        ;       See the "notes" in the Executive source.
        ;
        ; ---------------------------------------------------------------------
        ; exec_InitClock
        ; --------------
        ; Initialise system timer.
        ; in:   no conditions
        ; out:  all registers preserved
        ;
        ; ---------------------------------------------------------------------
	; exec_ResetIdleTimeout
	; ---------------------
	; Resets the system idle timer to its configured start value.
	; in:	no conditions
	; out:	all registers preserved
	;
        ; ---------------------------------------------------------------------
        ; exec_InitBackplane
        ; ------------------
        ; Initialise link adaptor.
        ; in:   no conditions
        ; out:  all registers preserved
        ;
        ; ---------------------------------------------------------------------
        ; exec_SizeMemory
        ; ---------------
        ; Calculate the end of contiguous memory from the given base address or
        ; return the default base RAM address.
        ;
        ; in:   a1 = Base address
        ; out:  a1 = End address + 1
        ;
        ; in:   a1 = &FFFFFFFF (-1)
        ; out:  a1 = Default base address
        ;
        ; ---------------------------------------------------------------------
        ; exec_SizeFastMemory
        ; -------------------
	; Return the size and address of the FastRAM. Zero will be returned
        ; if there is no fast RAM available.
        ; in:   no conditions
        ; out:  a1 = size of the FastRAM area
	;	a2 = base address of the FastRAM area
	;
        ; ---------------------------------------------------------------------
	[	{FALSE}	; Removed for (speedup) work
        ; exec_NucleusBase
        ; ----------------
        ; in:   no conditions
        ; out:  a1 = Start address of the nucleus code (word-aligned)
        ;
	]	; EOF {boolean}
        ; ---------------------------------------------------------------------
        ; exec_RAMBase
        ; ------------
        ; in:   no conditions
        ; out:  a1 = First available RAM location
        ;
        ; ---------------------------------------------------------------------
        ; exec_FindROMItem
        ; ----------------
        ; in:   a1 = NULL terminated ASCII string of item to search for
        ; out:  V clear -> Item found
        ;                       a1 = item start address
        ;                       a2 = item length
        ;       V set   -> Item not found
        ;                       a1 = preserved
        ;                       a2 = preserved
        ;
        ; Searches the ROM and FlashEPROM images for the named ITEM.
        ; If the ITEM is found in the ROM, the FlashEPROM is still searched
        ; in-case a newer version is available.
        ;
        ; ---------------------------------------------------------------------
        ; exec_FindNEXTItem
        ; -----------------
        ; in:   a1 = ROM ITEM index
        ;       a2 = ROM BLOCK to enumerate
        ;               (currently "loc_internal" or "loc_internalFlash")
        ; out:  V clear -> ITEM found
        ;                       a1 = ROM ITEM index (updated)
        ;                       a2 = ITEM start address
        ;                       a3 = ITEM length
        ;       V set   -> ITEM not found
        ;                       a1 = preserved
        ;                       a2 = preserved
        ;                       a3 = preserved
        ;
        ; This can be used to enumerate the system ROM and internal FlashEPROM
        ; contents.
        ;
        ; ---------------------------------------------------------------------
        ; exec_WCompare
        ; -------------
        ; in:   a1 = NULL terminated wildcarded object name
        ;       a2 = NULL terminated full object name
        ; out:  a1 = preserved
        ;       a2 = preserved
        ;       V clear -> objects identical
        ;       V set   -> objects different
        ;
        ; This call is used to compare two ASCII strings for equivalence. One
        ; of the strings may contain wildcard characters.
        ;
        ; ---------------------------------------------------------------------
        ; exec_startLinkTx
        ; ----------------
        ; Preserve the arguments for use by the interrupt routine and enable
        ; link adaptor transmit buffer empty interrupts. Return immediately
        ; to the caller.
        ; At the moment it is unclear what should happen when there is an
        ; out-standing interrupt transfer. Ideally the system should not block,
        ; possibly only this process. At the moment it is assumed that the
        ; callers of this function perform simple link adaptor management, and
        ; if we are called while a transfer is out-standing then this is an
        ; error condition.
        ;
        ; in:   a1 = buffer address
        ;       a2 = amount of data to transfer
        ;       a3 = "SaveState *" for the process to be re-started after the
        ;            transfer has completed
        ; out:  a1 = preserved
        ;       a2 = preserved
        ;       a3 = preserved
        ;       V clear => OK
        ;       V set   => outstanding interrupt transfer
        ;
        ; ---------------------------------------------------------------------
        ; exec_abortLinkTx
        ; ----------------
        ; Terminate the interrupt driven transfer, disabling the link adaptor
        ; transmit buffer empty interrupt.
        ; in:   no conditions
        ; out:  a1 = buffer address of next byte to be transferred
        ;       a2 = number of bytes remaining to be transferred
        ;       a3 = "SaveState *" of the process to be resumed
        ;
        ; ---------------------------------------------------------------------
        ; exec_startLinkRx
        ; ----------------
        ; Preserve the arguments for use by the interrupt routine and enable
        ; link adaptor receive buffer full interrupts. Return immediately
        ; to the caller.
        ; At the moment it is unclear what should happen when there is an
        ; out-standing interrupt transfer. Ideally the system should not block,
        ; possibly only this process. At the moment it is assumed that the
        ; callers of this function perform simple link adaptor management, and
        ; if we are called while a transfer is out-standing then this is an
        ; error condition.
        ;
        ; in:   a1 = buffer address
        ;       a2 = amount of data to transfer
        ;       a3 = "SaveState *" for the process to be re-started after the
        ;            transfer has completed
        ; out:  a1 = preserved
        ;       a2 = preserved
        ;       a3 = preserved
        ;       V clear => OK
        ;       V set   => outstanding interrupt transfer
        ;
        ; ---------------------------------------------------------------------
        ; exec_abortLinkRx
        ; ----------------
        ; Terminate the interrupt driven transfer, disabling the link adaptor
        ; receive buffer full interrupt.
        ; in:   no conditions
        ; out:  a1 = buffer address of next byte to be transferred
        ;       a2 = number of bytes remaining to be transferred
        ;       a3 = "SaveState *" of the process to be resumed
        ;
        ; ---------------------------------------------------------------------
        ; exec_DefineHandler
        ; ------------------
        ; Register the root interrupt handler function. This is the routine
        ; called when an unknown interrupt occurs.
        ; in:   a1 = function address
        ; out:  a1 = preserved
        ;       V clear = OK            handler registered
        ;       V set   = FAILED        handler NOT registered
        ;
        ; ---------------------------------------------------------------------
        ; exec_DefineShutdownHandler
        ; --------------------------
        ; Register the root device driver shutdown handler function. This is
	; the routine called when a PowerDown event occurs.
	; NOTE: Ideally the functionality of this call will be merged with
	;	that of the similar call "exec_DefineHandler".
        ; in:   a1 = function address
        ; out:  a1 = preserved
        ;       V clear = OK            handler registered
        ;       V set   = FAILED        handler NOT registered
        ;
        ; ---------------------------------------------------------------------
        ; exec_GenerateError
        ; ------------------
        ; Generate a fatal system error. This SWI expects an error definition
        ; block to immediately follow the instruction. This method is used so
        ; that the register set active when the SWI is called will be preserved
        ; in the register dump area.
        ; in:   <no conditions>
        ; out:  DOES NOT RETURN
        ;
        ; Error block format:
        ;       32bit error number                      - as for exec_ExecHalt
        ;       NULL terminated ASCII error message     - fault specific
        ; ---------------------------------------------------------------------
        ; exec_DisplayInfo
        ; ----------------
        ; This call returns information to the user about the current display.
        ; This is required to provide information about the HARD default screen
        ; system. The actual screen driver may ignore parts of this information
        ; and provide its own screen modes if required/possible.
        ; in:   no conditions
        ; out:  V clear = OK     : a1 = display type and information
        ;                          a2 = display X (width pixels)
        ;                          a3 = display Y (height rasters)
        ;                          a4 = display base address (byte aligned)
	;			   v1 = display stride (between rasters)
        ;       V set   = FAILED : return parameters undefined
        ;
        [       (:LNOT: SWItable)
        ; display description
        ; ===================
        ;
        ; The X (width in pixels) value is an unsigned 16bit value stored in
        ; bits 15..0. The 16bit value in bits 31..16 encode the screen stride
        ; in pixels. ie. the value required to get to the top of the next
        ; raster.
        ;
        ; The Y (height in rasters) is an unsigned 16bit value stored in bits
        ; 15..0. The bits 31..16 are currently unallocated, and should be NULL.
        ;
        ; The screen base address is the byte-aligned address of the physical
        ; screen memory. Screen memory starts at this point and goes higher
        ; into memory. This is regardless of any logical pixel mapping that
        ; might be used.
        ;
        ; display information (r0)
        ; ========================
        ;       
        ;        31    24 23    16 15     8 7      0
        ;       +--------+--------+--------+--------+
        ;       | unused | unused |log2bpp |  type  |
        ;       +--------+--------+--------+--------+
        ;
display_type_mask       *       &000000FF       ; hardware description
display_type_shift      *       0
display_bpp_mask        *       &0000FF00       ; log2 of no. of bits per pixel
display_bpp_shift       *       8
display_unused_mask     *       &FFFF0000       ; unused (NULL) bits
display_unused_shift    *       16
        ;
        ; display type
        ; ============
        ;
        ;          7  6  5  4  3  2  1  0
        ;       +--+--+--+--+--+--+--+--+
        ;       |tt|b6|b5|b4|b3|b2|b1|b0|
        ;       +--+--+--+--+--+--+--+--+
        ;
        ;       tt = 0 = LCD display
        ;            1 = CRT (VIDC) display
display_type_LCD        *       &00
display_type_CRT        bit     7
display_type_bit        bit     7
        ;       
        ;            LCD
        ;            ==========================================================
        ;               b6 = <<UNALLOCATED>> should be 0
        ;               b5 = <<UNALLOCATED>> should be 0
        ;               b4 = <<UNALLOCATED>> should be 0
        ;               b3..b0 = Sub-type classification
                        ^       &00
display_type_mono       #       &01     ; single monochrome screen
display_type_monosplit  #       &01     ; dual (split) monochrome screen
        ;               ALL other values are currently UNDEFINED
        ;
        ;               The "dual" monochrome screen is implemented as adjacent
        ;               rasters. ie. ((X width bits 31..16) / 2) describes the
        ;               stride required to reach the second raster.
        ;
        ;            CRT (VIDC system)
        ;            ==========================================================
        ;               b6 = <<UNALLOCATED>> should be 0
        ;               b5 = <<UNALLOCATED>> should be 0
        ;               b4 = <<UNALLOCATED>> should be 0
        ;               b3 = <<UNALLOCATED>> should be 0
        ;               b2 = <<UNALLOCATED>> should be 0
        ;               b1 = <<UNALLOCATED>> should be 0
        ;               b0 = <<UNALLOCATED>> should be 0
        ;
        ]
        ;
        ; ---------------------------------------------------------------------
        ; exec_Version
        ; ------------
        ; Return a Executive version number and identity text. This information
        ; is directly read from the Executive ROM item.
        ; in:   no conditions
        ; out:  a1 = BCD version number and machine hardware identifier
	;		bits  0..15 : BCD version number of Executive ROM ITEM
	;		bits 16..23 : 8bit machine hardware identifier
	;		bits 24..31 : undefined (normally &00)
        ;       a2 = pointer to identity string
        ;       a3 = 32bit Unix style timestamp
	;
	[	(:LNOT: SWItable)
	; machine identifier number
AB1_machine	*	&AB		; Active Book 1 hardware
	; >= &F0 are allocated to non-released hardware systems
HEVAL_machine	*	&FE		; Hercules evaluation board
FP_machine	*	&FF		; Active Book Functional Prototype
	]	; EOF (:LNOT: SWItable)
	;
        ; ---------------------------------------------------------------------
	; General FIQ code notes:
	;	The routine that installs/copies the FIQ handler code performs
	;	branch destination modification (if necessary). This could
	;	cause problems if the copied code contains data-words that
	;	can be interpreted as valid ARM branch instructions. Since the
	;	FIQ code copied down should ALWAYS be written in assembler, we
	;	can strongly document this feature of the system, and hopefully
	;	people can work around the limitation manually. It is very
	;	unlikely that people will want to keep literal constants in the
	;	FIQ area.
	;
	;	FIQ handlers using the multiple handler system, should never
	;	use absolute workspace pointers, only PC relative. This is
	;	because the actual address where they will reside is NOT known
	;	at assemble/compile time. See the comments attached to the
	;	"exec_AttachFIQ" SWI.
	;
	;	The FIQ routines are currently only provided on Hercules
	;	architecture systems.
        ; ---------------------------------------------------------------------
        ; exec_AttachSFIQ
        ; ---------------
        ; Attach the defined code as the only FIQ handler. This will fail if
        ; the multiple handler system is already active. If attached this
        ; will lock-out any other FIQ users until it has been released.
	; Since this will be the only active FIQ handler, it can use the
	; r13 returned by "exec_DefaultFIQStack" as a fixed workspace pointer.
	;
        ; in:   a1      : source mask (single bit describing desired FIQ)
        ;       a2      : handler code address
        ;       a3      : handler code length
        ;       a4      : default register set
        ; out:  V clear : handler attached; a1 = undefined
        ;       V set   : handler attach failed; a1 = error code
        ;                                             -1 = FIQ handler active
        ;                                             -2 = code too large
        ;       a2      : preserved
        ;       a3      : preserved
        ;       a4      : preserved
	;
        ; ---------------------------------------------------------------------
        ; exec_AttachFIQ
        ; --------------
        ; Attach the defined function to the multiple FIQ handler system.
        ; The code is copied to a buffer in the system workspace, where it
        ; can be executed by the FIQ handler. If the r13 register in the set
        ; passed is the same as the value returned by "exec_DefaultFIQStack"
        ; then the code will initialise the r13 to the same offset above the
	; true code address as the value returned by "exec_DefaultFIQStack" is
	; above the fixed FIQ base address of &0000001C. NOTE: The FIQ code
	; should NOT contain code that uses absolute addresses within the
	; normal FIQ area.
	;
        ; in:   a1      : source mask (single bit describing desired FIQ)
        ;       a2      : handler code address
        ;       a3      : handler code length
        ;       a4      : default register set
        ; out:  V clear : handler attached; a1 = preserved
        ;       V set   : handler attach failed; a1 = error code
        ;                                             -1 = cannot share FIQ
        ;                                             -2 = code too large
	;				              -3 = FIQ already active
        ;       a2      : preserved
        ;       a3      : preserved
        ;       a4      : preserved
	;
        ; ---------------------------------------------------------------------
        ; exec_ReleaseFIQ
        ; ---------------
        ; Detach a FIQ handler (releasing its table entry).
        ; Note: The FIQ source should be disabled before calling this SWI.
	;
        ; in:   a1      : source mask (single bit describing desired FIQ)
        ; out:  V clear : handler released
        ;       V set   : handler unknown
        ;       a1      : preserved
	;
        ; ---------------------------------------------------------------------
        ; exec_DefaultFIQStack
        ; --------------------
        ; This call returns the default FIQ r13 register value. Since this
	; value is always at the end of the FIQ code/data area, and the start
	; of the FIQ workspace is fixed at &0000001C it can be used externally
	; to calculate the size of the FIQ area. Single FIQ users can
	; initialise their r13 to this value.
	;
        ; in:   <no conditions>
        ; out:  a1 = default FIQ r13 value (FD stack)
	;
        ; ---------------------------------------------------------------------
        ; exec_TerminalOut
        ; ----------------
        ; Write the single character to the 2nd (debugging) link adaptor.
        ; Note: This SWI will block if the 2nd link does NOT exist.
        ; in:   a1 = 8bit ASCII code
        ; out:  a1 = preserved
        ; ---------------------------------------------------------------------
        ; exec_TerminalIn
        ; ---------------
        ; Read a single character from the 2nd (debugging) link adaptor.
        ; Note: This SWI will block if the 2nd link does NOT exist.
        ; in:   a1 = undefined
        ; out:  a1 = 8bit ASCII code
        ; ---------------------------------------------------------------------
        ; exec_Output
        ; -----------
        ; Write the NULL terminated ASCII string to the 2nd (debugging) link
        ; adaptor.
        ; Note: This SWI will block if the 2nd link does NOT exist.
        ; in:   a1 = pointer to NULL terminated ASCII string
        ; out:  a1 = preserved
        ; ---------------------------------------------------------------------
        ; exec_WriteHex8
        ; --------------
        ; Display the "a1" register contents as a HEX/ASCII string.
        ; Note: This SWI will block if the 2nd link does NOT exist.
        ; in:   a1 = 32bit unsigned value
        ; out:  a1 = preserved
        ; ---------------------------------------------------------------------
        ; exec_WriteDecimal
        ; -----------------
        ; Display the "a1" register contents as a decimal number.
        ; Note: This SWI will block if the 2nd link does NOT exist.
        ; in:   a1 = 32bit unsigned value
        ; out:  a1 = preserved
        ; ---------------------------------------------------------------------
        ; exec_NewLine
        ; ------------
        ; Send a LF (or possibly CR/LF) to the 2nd (debugging) link.
        ; Note: This SWI will block if the 2nd link does NOT exist.
        ; in:   no conditions
        ; out:  preserved
        ; ---------------------------------------------------------------------
        ; exec_Disassemble
        ; ----------------
        ; Disassemble the instruction to the 2nd link adaptor.
        ; Note: This SWI will block if the 2nd link does NOT exist.
        ; in:   a1 = instruction
        ;       a2 = address
        ; out:  a1 = number of characters printed
        ;       a2 = corrupted
        ; ---------------------------------------------------------------------
	; exec_SingleStep
	; ---------------
	; in:   a1 = ARMstate structure pointer
	; out:  a1 = instruction execution state
        ; ---------------------------------------------------------------------
	; exec_ResetARM
	; -------------
	; in:	a1 = ARMstate structure pointer
	; out:  preserved
        ; ---------------------------------------------------------------------
	; exec_SetNewPC
	; -------------
	; in:	a1 = ARMstate structure pointer
	; 	a2 = new PC address
	; out:	preserved
	; ---------------------------------------------------------------------
	; exec_RegisterDump
	; -----------------
	; in:	a1 = ARMstate structure pointer
	; out:	preserved
        ; ---------------------------------------------------------------------
	; exec_FlagStatus
	; ---------------
	; in:	a1 = PC and PSR value to decode
	; out:	preserved
        ; ---------------------------------------------------------------------
	; exec_ModeStatus
	; ---------------
	; in:	a1 = PC and PSR value to decode
	; out:	preserved
        ; ---------------------------------------------------------------------
        ; exec_HWRegisters
        ; ----------------
        ; This SWI allows the soft-copies of the various Hercules write-only
        ; registers to be interrogated and updated.
        ;
        ; in:   a1 = index of the Hercules register we are interested in
	;	     or -1 to return the table base.
        ;       a2 = bits to clear or undefined if a1 == -1
        ;       a3 = bits to set or undefined if a1 == -1
        ; out:  V clear = OK
        ;               a1 = preserved
        ;               a2 = old value or base address of table if entry a1==-1
        ;               a3 = new value or table size (in bytes) if entry a1==-1
        ;       V set = FAILED (should never happen with entry a1 == -1)
        ;               a1 = preserved
        ;               a2 = old value
        ;               a3 = value that would have been written
        ;
        ; Where:
        ;       new value = (((old value) BIC a2) ORR (a3))
	;
	; The table returned is the actual soft-copy structure.
        ;
        [       (:LNOT: SWItable)
        ; These are the indices used to reference the hardware registers.
                        ^       &00     ; register width
HWReg_MEMMAP            #       &01     ;     8bits     READ/WRITE
HWReg_CLOCK             #       &01     ;     8bits     READ/WRITE
HWReg_BANK0             #       &01     ;     8bits     READ ONLY
HWReg_BANK1             #       &01     ;     8bits     READ ONLY
HWReg_BANK2             #       &01     ;     8bits     READ ONLY
HWReg_BANK3             #       &01     ;     8bits     READ ONLY
HWReg_BANK4             #       &01     ;     8bits     READ ONLY
HWReg_BANK5             #       &01     ;     8bits     READ ONLY
HWReg_BANK6             #       &01     ;     8bits     READ ONLY
HWReg_BANK7             #       &01     ;     8bits     READ ONLY
HWReg_IRQtest           #       &01     ;    16bits     READ/WRITE
HWReg_IRQ               #       &01     ;    16bits     READ ONLY
HWReg_FIQ               #       &01     ;    16bits     READ ONLY
HWReg_TIMER             #       &01     ;     8bits     READ/WRITE
HWReg_LCDcontrol        #       &01     ;    32bits     READ/WRITE
HWReg_LCDlinelength     #       &01     ;     8bits     READ/WRITE
HWReg_LCDlinerate       #       &01     ;     8bits     READ/WRITE
HWReg_LCDnumlines       #       &01     ;     8bits     READ/WRITE
HWReg_DMArouting        #       &01     ;    16bits     READ/WRITE
HWReg_Control		#	&01	;     8bits     READ/WRITE
        ;
HWReg_ListEnd           #       &00     ; end of the list
        ; The "READ ONLY" registers will always return V set. They are
        ; presented purely for program information. The IRQ mask soft-copy
        ; is marked as "READ ONLY" by this SWI, since it has its own special
        ; case update SWIs ("exec_DisableIRQ" and "exec_EnableIRQ").
        ]
        ;
        ; ---------------------------------------------------------------------
        ; exec_ROHWRegisters
        ; ------------------
        ; This SWI allows certain READ ONLY hardware locations to be
	; interrogated:
        ; in:   a1 = index of the Hercules register we are interested in
        ; out:  a1 = preserved
	;	V clear = OK
        ;               a2 = value read
        ;       V set = FAILED (invalid index)
        ;               a2 = preserved
        ;
        [       (:LNOT: SWItable)
        ; These are the indices used to reference the hardware registers.
                        ^       &00     ; register width
ROHWReg_Status          #       &01     ;     8bits
        ;
ROHWReg_ListEnd         #       &00     ; end of the list
        ]
        ;
        ; ---------------------------------------------------------------------
        ; exec_ClaimHWMemory
        ; ------------------
        ; Switch this process thread into the hardware (physical) memory map
        ; mode. NOTE: No mappings below 32MByte will be available.
        ; in:   <no conditions>
        ; out:  state preserved
        ; ---------------------------------------------------------------------
        ; exec_ReleaseHWMemory
        ; --------------------
        ; Switch this process thread into the normal (operating system mode 1)
        ; memory map mode. NOTE: no hardware devices will be directly
        ; accessible.
        ; in:   <no conditions>
        ; out:  state preserved
        ; ---------------------------------------------------------------------
	; exec_ML_StartTx
	; ---------------
        ; Initiate transmission of microlink message to microcontroller.
	; The transmission is performed under FIQ, with an IRQ at the end.
	; The calling process suspends itself using the SaveState structure
	; supplied as a parameter here; the IRQ handler can then
	; Resume the process when transmission is complete.
	;
        ; in:  a1 = pointer to buffer containing message
	;      a2 = address of SaveState structure
        ; out: a1, a2 preserved
        ;      V set if this transmission could not be started because
	;      another one was already in progress.
        ; ---------------------------------------------------------------------
	; exec_ML_SetUpRx
	; ---------------
        ; Set up a reception buffer for a microlink message.
	;
        ; in:  a1 = pointer to buffer for message
	;      a2 = type of message to be received
        ; out: V clear: reception set up
	;      a1  = handle to be passed to ML_WaitForRx
	;      V set: no space left to record this reception
        ; ---------------------------------------------------------------------
	; exec_ML_PollRx
	; --------------
        ; Test whether a reception request has been satisfied.
	;
        ; in:  a1 = handle returned by ML_SetUpRx
        ; out: V clear:   Reception complete; handle now invalid
	;      V set:     Reception not complete
	;      V & C set: Invalid handle
        ; ---------------------------------------------------------------------
	; exec_ML_ResumeAfterRx
	; ---------------------
        ; Record a SaveState pointer which the reception interrupt routine
	; will use to resume a waiting process after a reception completes.
	;
        ; in:  a1 = handle returned by ML_SetUpRx
	;      a2 = expiry time (seconds since 1969)
	;      a3 = pointer to SaveState struct of waiting process
        ; out: V clear: SaveState recorded for interrupt routine
	;      V set:   invalid handle
        ; ---------------------------------------------------------------------
	; exec_ML_RegisterHandler
	; -----------------------
        ; Register a message handler function
	;
        ; in:  a1 = address of ML_MsgHandler structure
	;      dp = caller's module table pointer
        ; out: V clear:   Handler installed
	;      V set:     Handler not installed
        ; ---------------------------------------------------------------------
	; exec_ML_DetachHandler
	; ---------------------
        ; Remove a previously registered message handler function
	;
        ; in:  a1 = address of ML_MsgHandler structure register earlier
        ; out: V clear:   Handler removed
	;      V set:     Handler not found
        ; ---------------------------------------------------------------------
	; exec_ML_Reset		
	; -------------
        ; Reset the communication channel with the microcontroller
	;
        ; in:  no arguments
        ; out: no results
        ; ---------------------------------------------------------------------
	; exec_VerifyCARD
	; ---------------
	; in:	a1 = slot number to examine
	; out:	V clear : a1 = bitmask of AREA types within the CARD
	;		  a2 = number of AREAs in the CARD
	;	V set   : a1 = error code (defined in "manifest.s")
	;		  a2 = undefined
        ; ---------------------------------------------------------------------
	; exec_CARDAreaInfo
	; -----------------
	; in:	a1 = CARD slot number
	;	a2 = AREA number to examine
	; out:	V clear : a1 = bitmask of AREA type (single bit)
	;	          a2 = size of the AREA in bytes
	;	          a3 = base address of the AREA
	;	V set   : a1 = error code (defined in "manifest.s")
	;		  a2 = undefined
	;		  a3 = undefined
        ; ---------------------------------------------------------------------
	; exec_ResetCPU
	; -------------
	; in:	no conditions
	; out:	no conditions (only returns if unable to reset system)
	; NOTE: This SWI is temporary and only provided for systems debugging.
        ; ---------------------------------------------------------------------
	; exec_ClaimDMAchannel
	; --------------------
	; Claim a DMA channel for the given user. The valid users are defined
	; below. These are different from the actual numbers used (defined in
	; "hardAB1.s") since certain source identifiers are over-loaded.
	; This SWI should NOT be called from interrupt threads.
	;
	; in:	a1 = source identifier
	; out:	V clear : channel claimed
  	;		  a1 = preserved
	;		  a2 = allocated channel number
	;		  a3 = allocation handle
	;	V set   : a1 = error code
	;			-1 = invalid source identifier
	;			-2 = no channel free
	;		  a2 = undefined
	;		  a3 = undefined 
	;
	[	(:LNOT: SWItable)
	; These initial values are restricted to explicit channel numbers
DMAsource_shift		*	(24)	; encoded channel number position
DMAsource_mask		*	(&FF)	; encoded channel number mask
	; NASTY constants - we should ASSERT these values somewhere
DMAsource_codecRX	*	(&01 :OR: (1 :SHL: DMAsource_shift))
DMAsource_codecTX       *	(&01 :OR: (2 :SHL: DMAsource_shift))
DMAsource_TEST		*	(&01 :OR: (3 :SHL: DMAsource_shift))
	;
	; And these values follow on, without the encoded channel number
			^	&02	; DMAchan_MLIRX	; starting index
DMAsource_MLIRX		#	&01
DMAsource_MLITX         #	&01
DMAsource_EXTA		#	&01
DMAsource_EXTB		#	&01
DMAsource_EXTC		#	&01
DMAsource_EXTD		#	&01
DMAsource_limit		#	&00		; maximum source number
	]	; EOF (:LNOT: SWItable)
	;
        ; ---------------------------------------------------------------------
	; exec_ReleaseDMAchannel
	; ----------------------
	; Release a DMA channel claimed with "exec_ClaimDMAchannel".
	; This SWI should NOT be called from interrupt threads.
	;
	; in:	a1 = source identifier
	;	a2 = channel number allocated by "exec_ClaimDMAchannel"
	;	a3 = allocation handle returned by "exec_ClaimDMAchannel"
	; out:	V clear : channel released
	;		  a1 = preserved
	;		  a2 = preserved
	;		  a3 = preserved
	;	V set   : a1 = error code
	;			-1 = invalid source identifier
	;			-2 = channel not allocated to given source
	;		  a2 = preserved
	;		  a3 = preserved
	;
        ; ---------------------------------------------------------------------
	; exec_ClaimEXsource
	; ------------------
	; Claim an external interrupt source. In the Hercules world the
	; external interrupt sources have multiple users. This call allows
	; users to claim the use of the interrupt line.
	;
	; in:	a1 = external interrupt source required
	; out:	V clear : external source claimed
	;		  a1 = preserved
	;		  a2 = allocation handle
	;	V set   : a1 = error code
	;			-1 = invalid external source identifier
	;			-2 = external source already allocated
	;		  a2 = undefined
	;
	[	(:LNOT: SWItable)
		^	&01		; starting index
	;
sourceEXA	#	1		; serial
sourceEXB	#	1		; FDC, HDC and Tlink0
sourceEXC	#	1		; serial DMA and FAX DMA
sourceEXD	#	1		; FDC DMA, HDC DMA and Tlink1
	;
extsourceMAX	*	sourceEXD	; number of sources available
	]	; EOF (:LNOT: SWItable)
	;
        ; ---------------------------------------------------------------------
	; exec_ReleaseEXsource
	; --------------------
	; Release the resource allocated by the "exec_ClaimEXsource" SWI.
	;
	; in:	a1 = external source allocated by "exec_ClaimEXsource"
	;	a2 = allocation handle returned by "exec_ClaimEXsource"
	; out:	V clear : external source released
	;		  a1 = preserved
	;		  a2 = preserved
	;	V set   : a1 = error code
	;			-1 = invalid external source identifier
	;			-2 = invalid allocation handle
	;		  a2 = preserved
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF SWI/s
