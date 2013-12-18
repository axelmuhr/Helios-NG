        SUBT    Executive SWI handler and routines                   > loswi4/s
        ; Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; -- exec_CPUTime -----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Return the time elapsed since system startup.
        ; in:  r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = callers return address
        ; out: r0  = centi-seconds elapsed since system initialised
code_exec_CPUTime
        ; load the "ticks" time from the "ExecRoot" structure
        LDR     r0,=ROOT_start
        LDR     r0,[r0,#ExecRoot_cstimer]
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- exec_NumPris -----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Return the number of lo-priority levels (1..n).
        ; in:  r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = callers return address
        ; out: r0  = number of lo-priority levels
code_exec_NumPris
        MOV     r0,#NumberPris
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- exec_InitClock ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Timer (clock) initialisation.
        ; in:  r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ;      SVC mode; IRQs undefined; FIQs undefined
        ; out: <no conditions>
code_exec_InitClock
        AND     r11,svc_r14,#Fbit               ; entry FIQ state
        TEQP    r11,#(Ibit :OR: SVCmode)        ; disable IRQs

        [       (hercules)
        MOV     r11,#TIMER_regs                 ; address the TIMER hardware
        MOV     r12,#(TIMER_enable :OR: (centisecondtick :AND: TIMER_psn_mask))
        STRB    r12,[r11,#TIMER_control]        ; start timer
        LDR     r11,=hardware_regs              ; address the soft-copies
        STR     r12,[r11,#TIMER_data]           ; and update the soft-copy
        ; now enable the timer interrupt (directly accessing the soft-copy)
        LDR     r12,[r11,#IRQ_data]             ; current bitmask
        ORR     r12,r12,#INT_TIM                ; set timer interrupt bit
        STR     r12,[r11,#IRQ_data]             ; and update soft-copy
        MOV     r11,#INT_regs
        MOV     r12,#(IRQ_set :OR: INT_TIM)     ; timer interrupt source bit
        STR     r12,[r11,#IRQ_control]          ; enable timer interrupt
        |       ; middle (hercules)
        ; Functional Prototype
        ; Program timer clock 0 with our delay (10milli-second tick).
        ; Set timer into mode 2 (free-running, with pulse every time
        ; count reaches 0). Use binary count.

        MOV     r11,#timer_base
        MOV     r12,#(timer_con_mode2 :OR: timer_con_rwlm :OR: timer_con_sc0)
        STRB    r12,[r11,#timer_control]        ; Set control word

        ; Now write the two bytes of the initial count (lo then hi)
        MOV     r12,#((centisecondtick :AND: &00FF) :SHR: 0)
        STRB    r12,[r11,#timer_ctr0_data]
        MOV     r12,#((centisecondtick :AND: &FF00) :SHR: 8)
        STRB    r12,[r11,#timer_ctr0_data]

        ; Enable the timer interrupt (IRQ)
        LDR     r11,=IRQ_mask_copy
        LDR     r12,[r11,#0]
	ORR	r12,r12,#int_timer0	; Enable timer0 interrupt
        LDR     r11,=irq_mask           ; IRQ enable register
        STRB    r12,[r11,#0]
        LDR     r11,=IRQ_mask_copy      ; Set soft copy too
        STR     r12,[r11,#0]
        ]	; EOF (hercules)

        ; Recover entry state
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- exec_ResetIdleTimeout --------------------------------------------
        ; ---------------------------------------------------------------------
	; This call resets the system idle timeout value "ExecRoot_idletimeout"
code_exec_ResetIdleTimeout
        ; in:  r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ;      SVC mode; IRQs undefined; FIQs undefined
        ; out: <no conditions>

	[	(shutdown)
        LDR     r11,=ROOT_start			; Executive workspace
	LDR	r12,[r11,#ExecRoot_timeout_stage1]	; cached timeout value
	; The default value for the stage1 timeout is stored in the EEPROM.
	; However, since reading from the EEPROM will be a slow operation,
	; we cache the timeout value in normal workspace (making sure that
	; the EEPROM update code will keep the value upto date: the value
	; (0 <= n <= 255) is used as follows "timeout = ((n + 1) * 30 * 100)"
	; to give a centi-second value used internally).
        STR	r12,[r11,#ExecRoot_idletimeout]
	]	; EOF (shutdown)

	LDMFD	sp!,{r11,r12}
	BICS	pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- exec_SizeMemory --------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Calculate the size of memory in bytes.
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = base address or &FFFFFFFF
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = end address + 1
        ;       processor mode restored
localSizeMemory
        ; provide a local entry to the "SizeMemory" code
        STMFD   sp!,{r11,r12}
code_exec_SizeMemory
	; At the moment we do not use the "base address" other than as a flag
	; to return the actual "base address".
        ADDS    r11,r0,#&01             ; if r0 was &FFFFFFFF, Z flag is set
        LDREQ   r11,=userRAMbase        ; start of user RAM
        LDRNE   r11,=userRAMtop         ; end of user RAM + 1
        LDR     r0,[r11,#&00]
exit_SizeMemory
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; return V clear, restoring callers
                                        ; IRQ and FIQ state

        ; ---------------------------------------------------------------------
        ; -- exec_SizeFastMemory ----------------------------------------------
        ; ---------------------------------------------------------------------
        ; Find and size the FastRAM.
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = size (in bytes) of FastRAM
	;	r1  = base address of FastRAM
        ;       processor mode restored
code_exec_SizeFastMemory
	MOV	r0,#user_fast_ram_size	; amount of user FastRAM
	MOV	r1,#user_fast_ram_base	; base address of FastRAM
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; return V clear

	[	(:LNOT: speedup)	; not required when using direct vector
        ; ---------------------------------------------------------------------
        ; -- exec_NucleusBase -------------------------------------------------
        ; ---------------------------------------------------------------------
        ; This call returns the address of the nucleus code.
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = Nucleus start address
        ;       processor mode restored
localNucleusBase
        ; provide a local entry to the "NucleusBase" routine
        STMFD   sp!,{r11,r12}
code_exec_NucleusBase
	; In the debundled world we do not have a single object containing the
	; Nucleus. This call is actually named wrong... it should be called
	; "exec_SysBase".
	LDR	r0,=SYSBASE_start	; address of SYSBASE structure
        LDMFD   sp!,{r11,r12}           ; restore entry registers
        BICS    pc,link,#Vbit           ; return V clear
	]	; EOF (:LNOT: speedup)

        ; ---------------------------------------------------------------------
        ; -- exec_RAMBase -----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; This call returns the address of the first available RAM location.
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = Available RAM start address
        ;       processor mode restored
code_exec_RAMBase
        LDR     r11,=userRAMbase        ; start of user RAM
        LDR     r0,[r11,#&00]
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; return V clear, restoring callers

        ; ---------------------------------------------------------------------
        ; -- exec_IntsOff -----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Disable IRQs.
        ; Update the callers PSR to have IRQs disabled (Ibit set).
        ; in:  r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: no conditions
code_exec_IntsOff
        LDMFD   sp!,{r11,r12}
        BIC     svc_r14,svc_r14,#Vbit           ; clear V
local_disableIRQs
        ORRS    pc,svc_r14,#Ibit                ; set I

        ; ---------------------------------------------------------------------
        ; -- exec_IntsOn ------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Enable IRQs.
        ; Update the callers PSR to have IRQs enabled (Ibit clear).
        ; in:  r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: no conditions
code_exec_IntsOn
        LDMFD   sp!,{r11,r12}
local_enableIRQs
        BICS    pc,svc_r14,#(Ibit :OR: Vbit)    ; clear I and V

        ; ---------------------------------------------------------------------
        ; -- exec_DisableIRQ --------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Disable the specified IRQ sources.
        ; in:  r0  = mask with bits set for sources to be disabled
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: r0  = old IRQ mask
local_DisableIRQ
        ; provide a local entry to the "DisableIRQ" routine
        STMFD   sp!,{r11,r12}
code_exec_DisableIRQ
        [       (hercules)
        MOV     r12,lk                  ; preserve callers return address
        BL      local_disableIRQs       ; disable IRQs over update
        MOV     lk,r12                  ; restore callers return address

        STMFD   sp!,{r1}                ; work register
        MOV     r1,r0                   ; copy mask argument
        LDR     r11,=hardware_regs      ; address of soft-copies of HW regs
        LDR     r0,[r11,#IRQ_data]      ; load the old mask into result reg
        BIC     r12,r0,r1               ; clear the specified bits
        STR     r12,[r11,#IRQ_data]     ; update the soft-copy
        BIC     r12,r1,#IRQ_set         ; clear the relevant bits
        MOV     r11,#INT_regs           ; address of hardware interrupt regs
        STR     r12,[r11,#IRQ_control]  ; and update the true-copy
        ; Exit, restoring callers interrupt state
        LDMFD   sp!,{r1,r11,r12}        ; recover work registers
        BICS    pc,link,#Vbit           ; Return with V clear
        |	; middle (hercules)
        MOV     r12,r0                  ; Copy mask argument
        LDR     r11,=IRQ_mask_copy      ; Address of mask soft copy
        LDR     r0,[r11,#0]             ; Get old mask in result reg
        BIC     r12,r0,r12              ; Clear the specified bits
        STR     r12,[r11,#0]            ; Update the soft copy
        LDR     r11,=irq_mask           ; Address of h/w mask register
        STRB    r12,[r11,#0]            ; Store (byte) mask
        ; Exit, restoring callers interrupt state
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; Return with V clear
        ]       ; EOF (hercules)

        ; ---------------------------------------------------------------------
        ; -- exec_EnableIRQ ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Enable the specified IRQ sources.
        ; in:  r0  = mask with bits set for sources to be enabled
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: r0  = old IRQ mask
local_EnableIRQ
        ; provide a local entry to the "EnableIRQ" routine
        STMFD   sp!,{r11,r12}
code_exec_EnableIRQ
        [       (hercules)
        MOV     r12,lk                  ; preserve callers return address
        BL      local_disableIRQs       ; disable IRQs over update
        MOV     lk,r12                  ; restore callers return address

        STMFD   sp!,{r1}                ; work register
        MOV     r1,r0                   ; copy mask argument
        LDR     r11,=hardware_regs      ; address of soft-copies of HW regs
        LDR     r0,[r11,#IRQ_data]      ; load the old mask into result reg
        ORR     r12,r0,r1               ; set the specified bits
        STR     r12,[r11,#IRQ_data]     ; update the soft-copy
        MOV     r11,#INT_regs           ; hardware interrupt registers
        ORR     r12,r1,#IRQ_set         ; set the relevant bits
        STR     r12,[r11,#IRQ_control]  ; store mask
        ; Exit, restoring the callers interrupt state
        LDMFD   sp!,{r1,r11,r12}        ; recover work registers
        BICS    pc,link,#Vbit           ; Return with V clear
        |	; middle (hercules)
        MOV     r12,r0                  ; Copy mask argument
        LDR     r11,=IRQ_mask_copy      ; Address of mask soft copy
        LDR     r0,[r11,#0]             ; Get old mask in result reg
        ORR     r12,r0,r12              ; Set the specified bits
        STR     r12,[r11,#0]            ; Update the soft copy
        LDR     r11,=irq_mask           ; Address of h/w mask register
        STRB    r12,[r11,#0]            ; Store (byte) mask
        ; Exit, restoring the callers interrupt state
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; Return with V clear
        ]       ; EOF (hercules)

        ; ---------------------------------------------------------------------
        ; -- exec_FindROMItem -------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Search for a named ITEM within the ROM or FlashEPROM. The ROM is
        ; searched first. If the ITEM appears in the ROM then the FlashEPROM
        ; is checked in-case a newer copy is provided. If the ITEM is not
        ; found in the ROM then the FlashEPROM is checked anyway.
        ; The system ROM starts at "ROM_base".
        ; The FlashEPROM starts at "FLASH_base".
        ; We do not need to know the size of these areas, since they should
        ; always have a valid structure.
local_FindROMItem
        STMFD   sp!,{r11,r12}
code_exec_FindROMItem
        ; in:   r0  : NULL terminated ASCII string
        ;       r11 : undefined (work register)
        ;       r12 : undefined (work register)
        ;       r13 : FD stack (containing entry r11 and r12)
        ;       r14 : return address
        ; out:  V clear -> item found           r0 = item base address
        ;                                       r1 = item length
        ;       V set   -> item not found       r0 = preserved
        ;                                       r1 = preserved

        STMFD   sp!,{r0,r1,r2,link}     ; preserve work registers

        [       ((debug2) :LAND: {FALSE})
        MOV     r2,r0
        ADR     r0,txtFRIdbg
        BL      local_Output
        MOV     r0,r2
        BL      local_Output
        BL      local_NewLine
        B       ovrFRIdbg
txtFRIdbg
        =       "FindROMItem: Searching for ",&00
        ALIGN
ovrFRIdbg
        ]	; EOF ((debug2) :LAND: {enable/disable switch})

        MOV     r2,#&00000000           ; system ROM ITEM find flag

	; Normally we would load r11 directly with "ROM_base". At the moment
	; I assume the start of the Executive is at the start of the ROM. This
	; allows RAM loaded systems to be developed that treat the RAM image
	; like the system ROM image.
	ADRL	r11,codeBase		; start of the system ROM BLOCK
        ; internal ROM is special-case
        ; search all other ROMs as PCcard structures
        ADD     r11,r11,#word           ; step-over the branch instruction

        ; r11 = start of the first ITEM
        LDR     r12,=ITEMMagic          ; r12 = the ITEM ID word
        ; Search for the ITEM in the system ROM
ROMfind_loop
        LDR     r1,[r11,#ITEMID]        ; read the ITEM magic word
        TEQ     r1,r12                  ; and check against the real value
        BNE     ROMfind_failed          ; ITEM is not in the system ROM

        ; The ITEM magic word has been found, check that we are pointing at
        ; a ROM ITEM.
        LDR     r1,[r11,#ITEMExtensions]        ; extensions bitmask
        TST     r1,#ITEMhdrROM                  ; is this a ROM item?
        BEQ     ROMfind_failed                  ; invalid ITEM (end of chain)
        ; These checks are NOT fool-proof. However they should guard against
        ; accidental generation of an invalid ROM image, since we follow the
        ; information given in the ITEM headers and do not search individual
        ; OBJECTs for the marker word.

        ; Check that the names match (we do not care about case)
        ADD     r1,r11,#ITEMName        ; start of the full name
        BL      local_WCompare
        ; VS = names are different
        LDRVS   r1,[r11,#ITEMLength]    ; complete length of this ITEM
        ADDVS   r11,r11,r1              ; step over this ITEM
        BVS     ROMfind_loop            ; and see if the next ITEM matches

        ; If we reach this point the ITEM names are the same.
        MOV     r2,r11                  ; remember the ROM ITEM base address

ROMfind_failed
        ; Search for the ITEM in the FlashEPROM. r2 will either be &00000000
        ; or the address of the system ROM ITEM found.

	MOV	r11,#FLASH_base		; start of the FlashEPROM block

        ; treat the internal FlashEPROM as a PCcard structure

        ; r11 = start of the first ITEM in the internal FlashEPROM
        LDR     r12,=ITEMMagic          ; r12 = the ITEM ID word
        ; Search for the ITEM in the FlashEPROM
FlashEPROMfind_loop
        LDR     r1,[r11,#ITEMID]        ; read the ITEM magic word
        TEQ     r1,r12                  ; and check against the real value
        BNE     FlashEPROMfind_failed   ; ITEM is not in the FlashEPROM

        ; The ITEM magic word has been found, check that we are pointing at
        ; a ROM ITEM.
        LDR     r1,[r11,#ITEMExtensions]        ; extensions bitmask
        TST     r1,#ITEMhdrROM                  ; is this a ROM item?
        BEQ     FlashEPROMfind_failed           ; invalid ITEM (end of chain)
        ; These checks are NOT fool-proof. However they should guard against
        ; accidental generation of an invalid ROM image, since we follow the
        ; information given in the ITEM headers and do not search individual
        ; OBJECTs for the marker word.

        ; Check that the names match (we do not care about case)
        ADD     r1,r11,#ITEMName        ; start of the full name
        BL      local_WCompare
        ; VS = names are different
        LDRVS   r1,[r11,#ITEMLength]    ; complete length of this ITEM
        ADDVS   r11,r11,r1              ; step over this ITEM
        BVS     FlashEPROMfind_loop     ; and see if the next ITEM matches
        MOV     r2,r11                  ; ITEM found in FlashEPROM
FlashEPROMfind_failed

        TEQ     r2,#&00000000           ; ITEM found in the system ROM
        BEQ     find_failed             ; NO - then ITEM has NOT been found

        MOV     r0,r2                   ; copy base address over entry name
        LDR     r1,[r0,#ITEMLength]
        ; r0 = ITEM base address
        ; r1 = ITEM length
        ADD     sp,sp,#&08              ; dump the entry r0 and r1
        LDMFD   sp!,{r2,link}           ; recover the entry r2 and link
        LDMFD   sp!,{r11,r12}           ; and also entry r11 and r12
        BICS    pc,link,#Vbit           ; ensure V clear

find_failed
        ; The specified object could NOT be found
        LDMFD   sp!,{r0,r1,r2,link}     ; recover entry r0, r1 and r2
        LDMFD   sp!,{r11,r12}           ; and also entry r11 and r12
        ORRS    pc,link,#Vbit           ; ensure V set

        ; ---------------------------------------------------------------------
        ; -- exec_FindNEXTItem ------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Enumerate the contents of the system ROM or FlashEPROM.
        ; If the request is for the system ROM, the FlashEPROM will still be
        ; searched to perform file over-loading. If the ITEM is found in the
        ; system ROM, the FlashEPROM is searched to see if an identically named
        ; ITEM exists. If it does, the next ITEM is stepped onto internally
        ; (ie. the ROM ITEM is not returned). When the ROM has been searched,
        ; all of the FlashEPROM contents are returned. The FlashEPROM only can
        ; be enumerated by passing a suitable parameter.
        ; Note: This scheme relies on the caller NOT placing any content
        ;       onto the "index" variable, since it bears no relation to
        ;       the number of times the routine has been called.
        ;       The only operations the caller should perform on the index
        ;       is to set it to zero when they wish to start the enumeration,
        ;       and to check for -1 marking the end of the search.
        ;
        ; THIS CODE NEEDS TO BE EXTENDED TO DEAL WITH EXTERNAL CARDs
        ; If we get a request for an external CARD slot we can assume that a
        ; ROM CARD is present, and simply enumerate the common memory.
        ;
code_exec_FindNEXTItem
        ; in:   r0  : ROM ITEM index (0 for start, -1 for end, or direct index)
        ;       r1  : ROM location
        ;       r11 : undefined (work register)
        ;       r12 : undefined (work register)
        ;       r13 : FD stack (containing entry r11 and r12)
        ;       r14 : return address
        ; out:  V clear -> ITEM found           r0 = ROM ITEM index (updated)
        ;                                       r1 = ITEM base address
        ;                                       r2 = ITEM length
        ;       V set   -> ITEM not found       r0 = &FFFFFFFF
        ;                                       r1 = preserved
        ;                                       r2 = preserved
	;
	; The ROM locations are:
	;	loc_internal		system ROM
	;	loc_internalFlash	system FlashEPROM
	;	CARD slot number	for the relevant CARD
	;
	; We need to take account of CARDs with multiple ROM areas.
	;
        MOV     r11,#&FFFFFFFF
        TEQ     r0,r11                  ; check for invalid call
        ORREQS  pc,link,#Vbit           ; and return immediately (with error)

        STMFD   sp!,{r0,r1,r2,r3,link}

        LDR     r12,=ITEMMagic          ; r12 = the ITEM ID word

        TEQ     r1,#loc_internal
        BNE     FindROM_checktype

        ; enumerate the internal ROM and then the FlashEPROM
        TEQ     r0,#&00000000
        BNE     FindROM_continue

	; Reference the system ROM image.
	ADRL	r0,codeBase		; see comments above (about ROM_base)
        ADD     r0,r0,#word             ; step over branch instruction
        ; r0 = start of the first ITEM
FindROM_continue
        ; r0 = address of ITEM to return information about
        ; We need to check if this index lies in the FlashEPROM and not
        ; the system ROM.
        CMP     r0,#FLASH_base          ; base address of FlashEPROM
        RSBGES  r3,r0,#FLASH_size       ; size of the FlashEPROM
        BGE     EnumerateFlashEPROM     ; continue listing FlashEPROM

FindROM_continue_loop
        ; r0 = address of ITEM to return information about
        LDR     r3,[r0,#ITEMID]         ; read the ITEM magic number
        TEQ     r3,r12                  ; and check against the real value
        MOVNE   r0,#&00000000           ; reset index
        BNE     EnumerateFlashEPROM     ; end of ROM, so list FlashEPROM

        ; The ITEM magic word has been found, check that we are pointing at
        ; a ROM ITEM.
        LDR     r3,[r0,#ITEMExtensions] ; extensions bitmask
        TST     r3,#ITEMhdrROM          ; is this a ROM ITEM?
        MOVEQ   r0,#&00000000           ; reset index
        BEQ     EnumerateFlashEPROM     ; clear - not ROM

        ; check if this ITEM is duplicated in the FlashEPROM
        !       0,"TODO: PCcard style ROM interrogation"
        MOV     r2,#FLASH_base          ; start of the FlashEPROM BLOCK 

        !       0,"TODO: PCcard style FlashEPROM interrogation"
        ; r2 = start of the first ITEM in the internal FlashEPROM
        ; search for the named ITEM in the FlashEPROM
FindFlashEPROM_loop
        LDR     r3,[r2,#ITEMID]         ; read the ITEM magic word
        TEQ     r3,r12                  ; and check against the real value
        BNE     FindROM_ok              ; ITEM not found in FlashEPROM

        ; Check that we are pointing at a ROM ITEM
        LDR     r3,[r2,#ITEMExtensions] ; extensions bitmask
        TST     r3,#ITEMhdrROM          ; is this a ROM ITEM?
        BEQ     FindROM_ok              ; ITEM not found in FlashEPROM

        ; Check that the names match
        ADD     r0,r0,#ITEMName         ; ITEM name we are looking for
        ADD     r1,r2,#ITEMName         ; start of the full name
        BL      local_WCompare
        SUB     r0,r0,#ITEMName         ; index back to start of ITEM
        ; VS = names are different
        LDRVS   r3,[r2,#ITEMLength]     ; complete length of FlashEPROM ITEM
        ADDVS   r2,r2,r3                ; step over this FlashEPROM ITEM
        BVS     FindFlashEPROM_loop     ; and see if the next ITEM matches

        ; names matched, so do not return this ROM ITEM
        LDR     r3,[r0,#ITEMLength]     ; complete length of ROM ITEM
        ADD     r0,r0,r3                ; step over this ROM ITEM
        B       FindROM_continue_loop   ; and check the next ROM ITEM

        ; ---------------------------------------------------------------------

FindROM_checktype
        ; r0  = ROM ITEM index (or 0 if at start)
        ; r1  = ROM location
        ; r2  = undefined
        ; r3  = undefined
        ; r12 = ITEMMagic       

	[	{TRUE}
	MOV	r2,r0			; preserve r0 over the debugging
	MOV	r3,lk
	ADRL	r0,frtxt1a
	BL	local_Output
	MOV	r0,r2
	BL	local_WriteHex8
	ADRL	r0,frtxt1b
	BL	local_Output
	MOV	r0,r1
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	lk,r3
	MOV	r0,r2
	B	frovr1
frtxt1a	=	"FindROM_checktype: r0 = &",&00
frtxt1b	=	&0A," r1 = &",&00
	ALIGN
frovr1
	]

        TEQ     r1,#loc_internalFlash
        BEQ     EnumerateFlashEPROM	; internal FlashEPROM is special case

	[	(activebook)
	; Assume that if the "r0" value is non-zero then we can continue
	; since the CARD slot number will already have been validated.
	TEQ	r0,#&00000000		; are we at the start?
	BNE	EnumerateROM_continue	; NO - then continue from this point

	; "exec_CARDAreaInfo" will validate the slot number for us here
	MOV	r0,r1			; slot number
	MOV	r1,#&01			; AREA number
	BL	local_CARDAreaInfo
	BVS	EnumerateROM_failed	; invalid CARD in slot

	[	{TRUE}
	STMFD	sp!,{r0,lk}
	ADRL	r0,frtxt2
	BL	local_Output
	MOV	r0,r2
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	frovr2
frtxt2	=	"EnumerateROM: CARD AREA r0 = &",&00
	ALIGN
frovr2
	]

	MOV	r0,r2			; r0 = AREA base address
	; reference the first ITEM in the referenced ROM (r0)
	B	EnumerateROM_continue	; and enumerate the contents
	|
	B	EnumerateROM_failed	; CARDs not supported
	]

	; ---------------------------------------------------------------------

EnumerateFlashEPROM
        ; r0  = ROM ITEM index (or 0 if at start)
	; r1 = undefined
	; r2 = CARD_address_table
	; r3 = undefined
        ; r12 = ITEMMagic       

        TEQ     r0,#&00000000		; are we at the start?
        BNE     EnumerateROM_continue	; NO - then continue from this point

	[	(activebook)
	; load the address from the word beneath the CARD address table
	ADRL	r2,CARD_address_table	; memory mapped CARD addresses
	LDR	r0,[r2,#-4]		; internal FlashEPROM address
	|
	MOV	r0,#FLASH_base		; internal FlashEPROM address
	]

        !       0,"TODO: PCcard style ROM interrogation"
	; reference the first ITEM in the internal FlashEPROM

	; and fall through to...
	; ---------------------------------------------------------------------
	; Enumerate the ROM contents
EnumerateROM_continue
        ; r0 = address of ITEM to return information about

        LDR     r3,[r0,#ITEMID]         ; read the ITEM magic number
        TEQ     r3,r12                  ; and check against the real value
        BNE     EnumerateROM_failed     ; end of ROM, so exit

        ; The ITEM magic word has been found, check that we are pointing at
        ; a ROM ITEM.
        LDR     r3,[r0,#ITEMExtensions] ; extensions bitmask
        TST     r3,#ITEMhdrROM          ; is this a ROM ITEM?
        BEQ     EnumerateROM_failed     ; clear - not ROM
        ; return this ITEM
FindROM_ok
        MOV     r1,r0                   ; r1 = ITEM base address
        LDR     r2,[r1,#ITEMLength]     ; r2 = ITEM length
        LDR     r11,[r0,#ITEMLength]    ; get the length of this ITEM
        ADD     r0,r0,r11               ; r0 = next ITEM index

        ADD     sp,sp,#&0C              ; dump the entry r0, r1 and r2
        LDMFD   sp!,{r3,link}           ; recover the entry r3 and link
        LDMFD   sp!,{r11,r12}           ; and recover entry r11 and r12
        BICS    pc,link,#Vbit           ; ensure V clear

	; ---------------------------------------------------------------------

EnumerateROM_failed
        ; The specified object could NOT be found
        LDMFD   sp!,{r0,r1,r2,r3,link}  ; recover entry r0, r1, r2 and r3
        MOV     r0,#&FFFFFFFF           ; but set r0 to be -1
        LDMFD   sp!,{r11,r12}           ; and also entry r11 and r12
        ORRS    pc,link,#Vbit           ; ensure V set

        ; ---------------------------------------------------------------------
        ; -- exec_WCompare ----------------------------------------------------
        ; ---------------------------------------------------------------------
        ; NOTE: This code has been converted from a piece of 'C'. It should be
        ;       tidied up and optimised.
local_WCompare
        ; simulate the SWI entry
        STMFD   sp!,{r11,r12}
        ; perform the operation
code_exec_WCompare
        ; in:   r0 = NULL terminated wildcarded object name
        ;       r1 = NULL terminated full object name
gptr    RN      r0	; wild-carded object
fptr    RN      r1	; full (complete) object
        ; out:  r0 = preserved
        ;       r1 = preserved
        ;       V clear -> objects match
        ;       V set   -> objects different
        STMFD   sp!,{gptr,fptr,link}    ; we are going to call sub-functions

        LDRB    r12,[fptr,#&00]         ; check for fullname termination
        CMP     r12,#null
        BNE     check_gptr              ; NO, then check wildcard name

        LDRB    r11,[gptr,#&00]
        CMP     r11,#wcmult             ; multiple wildcard character
        BNE     check_end_state

        ADD     gptr,gptr,#&01          ; increment wildcard string index
        BL      local_WCompare          ; and compare from here
        BVS     objects_different
        BVC     objects_matched

check_end_state 
        ; r12 = current fullname character
        ; r11 = current wildcard character
        CMP     r11,#null
        BEQ     objects_matched         ; wildcard termination
        BNE     objects_different       ; different characters

check_gptr
        ; r12 = fullname character
        LDRB    r11,[gptr,#&00]         ; check for wildcard name termination
        CMP     r11,#null
        BEQ     objects_different       ; wildcard terminates early

        CMP     r11,#wcsing
        BEQ     check_next              ; single wildcard always matches

        ; make r11 lower-case (corrupts link)
        CMP     r11,#"A"
        RSBGES  link,r11,#"Z"   ; use "link" register as temporary
        ORRGE   r11,r11,#&20

        ; make r12 lower-case
        CMP     r12,#"A"
        RSBGES  link,r12,#"Z"   ; use "link" register as temporary
        ORRGE   r12,r12,#&20
        CMP     r11,r12
        BNE     check_multiple          ; characters do not match
check_next
        ADD     fptr,fptr,#&01          ; increment fullname string index
        ADD     gptr,gptr,#&01          ; increment wildcard string index
        BL      local_WCompare          ; and compare from here
        BVS     objects_different
        BVC     objects_matched

check_multiple
        CMP     r11,#wcmult
        BNE     objects_different       ; not a wildcard character

        ADD     gptr,gptr,#&01          ; increment wildcard string index
        BL      local_WCompare          ; and compare from here
        BVC     objects_matched
        ; FALSE
        ADD     fptr,fptr,#&01          ; increment fullname string index
        SUB     gptr,gptr,#&01          ; decrement wildcard string index
        BL      local_WCompare          ; and compare from here
        BVS     objects_different
        ; and fall through to...
objects_matched
        ; The objects are identical, so return with V clear
        LDMFD   sp!,{gptr,fptr,link}
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

objects_different
        ; The objects are different, so return with V set
        LDMFD   sp!,{gptr,fptr,link}
        LDMFD   sp!,{r11,r12}
        ORRS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; Return the Executive version information.
code_exec_Version
        ; in:   no conditions
        ; out:  a1 = BCD version number and machine hardware identifier
        ;       a2 = pointer to identity string
        ;       a3 = external date stamp
	;
        ; We do the search this way rather than directly accessing the header
        ; at the start of this ITEM, so that Executive replacement works
        ; correctly.
        ADR     r0,search_name			; ITEM we are looking for
	MOV	r11,lk				; preserve r14
        BL      local_FindROMItem
	MOV	lk,r11				; and recover r14
        LDMVSFD sp!,{r11,r12}
        ORRVSS  pc,link,#Vbit                   ; error - ROM ITEM not found

        ; r0 = base address of ROM ITEM
        ; r1 = length of ROM ITEM
        LDR     r2,[r0,#(ITEMDate + &04)]       ; external Unix style timestamp
        LDRB    r1,[r0,#ITEMNameLength]         ; length of the name field
        ADD     r1,r1,#ITEMName                 ; plus the offset to the name
        ADD     r0,r0,r1                        ; start of ROM specific header
        LDR     r0,[r0,#ITEMVersion]            ; BCD version number

	[	{TRUE}
	STMFD	sp!,{r0,lk}
	ADR	r0,vtxt1
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	vovr1
vtxt1	=	"exec_Version: version (r0) &",&00
	ALIGN
vovr1
	]	; EOF {boolean}

	; Add in the machine identifier information
	[	(activebook)
	MOV	r1,#AB1_machine			; Active Book 1 hardware
	|
	[	(heval)
	MOV	r1,#HEVAL_machine		; HEVAL prototype
	|
	MOV	r1,#FP_machine			; Functional prototype
	]	; EOF (heval)
	]	; EOF (activebook)
	ORR	r0,r0,r1,LSL #16		; move ID into the top 16bits
        ADR     r1,return_information           ; textual description

	[	{TRUE}
	STMFD	sp!,{r0,lk}
	ADR	r0,vtxt2
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	vovr2
vtxt2	=	"exec_Version: machine info (r0) &",&00
	ALIGN
vovr2
	]	; EOF {boolean}

        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

search_name
        =       "$objname",&00
return_information
        =       "Active Book Executive (ARM Helios)",&00
        ALIGN

        ; ---------------------------------------------------------------------
        ; -- exec_DisplayInfo -------------------------------------------------
        ; ---------------------------------------------------------------------

code_exec_DisplayInfo
        ; in:   r11 : undefined (work register)
        ;       r12 : undefined (work register)
        ;       r13 : FS stack (containing entry r11 and r12)
        ;       r14 : return address
        ; out:  V clear : OK     : r0 = display type and description
        ;                          r1 = display X (width pixels)
        ;                          r2 = display Y (height rasters)
        ;                          r3 = display base address (byte aligned)
	;			   r4 = display stride (between rasters)
        ;       V set   : FAILED : return parameters undefined
        ;
        ; The default screens are assumed to be 640x400 1bpp

        [       (hercules)
        MOV     r1,#(LCD_displaywidth * 8)      ; X width in pixels
        MOV     r2,#LCD_height
        ; r0 = LCD split-screen (1bpp)
        MOV     r0,#(display_type_LCD :OR: display_type_mono)
        ORR     r0,r0,#((0 :SHL: display_bpp_shift) :AND: display_bpp_mask)
	[	(dynlcd)
	; load r3 with the tier 1 LCD DMA base address.
	MOV	r3,#DMA_base			; DMA hardware registers
	LDR	r3,[r3,#LCDscreen_base0]	; tier 1 DMA descriptor
	MOV	r3,r3,LSR #DMA_src_shift	; and get into correct range
	|	; middle (dynlcd)
        LDR     r3,=LCD_base                    ; physical LCD screen address
	]	; EOF (dynlcd)
        ; NOTE: r3 = the physical address (not the logical mapping)
	MOV	r4,#LCD_stride			; stride between rasters
        |	; middle (hercules)
        MOV     r1,#640         ; X width in pixels
        MOV     r2,#400         ; Y height in rasters

        ; AB1 Functional-prototype
        ; r0 = LCD split-screen (1bpp)
        MOV     r0,#(display_type_LCD :OR: display_type_mono)
        ORR     r0,r0,#((0 :SHL: display_bpp_shift) :AND: display_bpp_mask)
        ; r3 = base address
        MOV     r3,#LCD_base            ; start of physical LCD memory
	MOV	r4,#LCD_stride		; stride between rasters
        ]       ; EOF (hercules)
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- FIQ support ------------------------------------------------------
        ; ---------------------------------------------------------------------
	;
	; *********************************************************************
	; Read the comments about these SWIs in "abcARM/asm/SWI.s"
	; *********************************************************************
        ;
local_AttachSFIQ
        STMFD   sp!,{r11,r12}		; simulate SWI entry
code_exec_AttachSFIQ
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = source mask (single bit describing desired FIQ)
        ;       r1  = handler code address
        ;       r2  = handler code length
        ;       r3  = default register set address      (r8..r13)
        ;       r11 = undefined                         (work register)
        ;       r12 = undefined                         (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r1  = preserved
        ;       r2  = preserved
        ;       r3  = preserved
        ;       processor mode restored
        ;       V clear => FIQ handler attached; r0 = undefined
        ;       V set   => attach failed; r0 = error code
        ;                                       = -1 : FIQ handlers active
        ;                                       = -2 : code too large
        ;
        ; Attach a single FIQ user (locking out other users). This will fail
        ; if anybody currently has a FIQ handler defined.

        STMFD   sp!,{r7,r8,r9,r10}	; even more work registers

	[	{TRUE}
	STMFD	sp!,{r0,lk}
	ADR	r0,asftxt1
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	asfovr1
asftxt1	=	"AttachSFIQ: r0 = &",&00
	ALIGN
asfovr1
	]	; EOF {boolean}

        [       (hercules)
	; Disable IRQs over the following code (allowing the code to be
	; called from IRQ, SVC and USR mode threads).
	STMFD	sp!,{r14}		; preserve return address and PSR
	BL	local_disableIRQs	; disable processor IRQs
	; We do not explicitly re-enable processor IRQs, since the exit
	; code should restore the callers IRQ state.

	; If any bits are set in the "FIQ_data" soft copy then there is at
	; least one active FIQ handler.
	LDR	r11,=hardware_regs	; reference the soft-copy workspace
	LDR	r11,[r11,#FIQ_data]	; and load the current active mask
	TEQ	r11,#&00000000		; any active FIQ sources?
	BNE	FIQ_active_exit		; cannot claim FIQ

	LDR	r11,=FIQ_single		; marker flag
	MOV	r10,#&FFFFFFFF		; suitable non-zero value
	STR	r10,[r11,#&00]		; and mark as single user FIQ system

        ; Check that the supplied code is NOT too large for the FIQ space.
        CMP     r2,#(top_FIQ_stack - single_FIQ_code)
        BCS     FIQ_codesize_exit

        ; Copy most of the code to (single_FIQ_code + &04)
        MOV     r10,#single_FIQ_code	; destination address
	MOV	r11,#&04		; starting loop index
	BL	move_FIQcode		; copy the code down
	; the "move_FIQcode" function corrupts r7,r8,r9,r11 and r12

        MOV     r12,#(Fbit :OR: FIQmode)
        TEQP    r12,#&00000000          ; enter FIQ mode, FIQs disabled
        NOP                             ; wait for registers to be remapped

        ; Copy first word of code to &0000001C
        MOV     r11,#single_FIQ_code    ; address of the first word
        LDR     r12,[r1,#&00]           ; first word of referenced code
	AND	r7,r12,#op_mask		; get the major opcode group
	TEQ	r7,#op_b		; branch instruction
	TEQNE	r7,#op_bl		; branch-with-link instruction
	BNE	store_FIQ_first_word
	; the instruction is a branch
	LDR	r9,=offset_mask		; branch offset mask value
	AND	r8,r12,r9		; existing offset value

	ADD	r7,r1,r8,LSL #2		; branch offset value
	ADD	r7,r7,#&08		; deal with pipe-lining
	BIC	r7,r7,#HIflags		; remove "address exception" bits
	; r7 = original branch instruction destination address
	ADD	r8,r1,r2		; r8 = end address of region
	CMP	r7,r1			; check against region start address
	RSBGES	r8,r7,r8		; and the region end address
	BGT	store_FIQ_first_word	; within the region, so do not modify

	BIC	r12,r12,r9		; r12 = empty branch instruction
	ADD	r8,r11,#&08		; source = source + 8
	SUB	r7,r7,r8		; offset = destination - source
	MOV	r7,r7,LSR #2		; word-align the offset
	AND	r7,r7,r9		; ensure the offset is in range
	ORR	r12,r12,r7		; and add offset into the instruction
store_FIQ_first_word
        STR     r12,[r11,#&00]          ; and store into the FIQ area

        ; Enable the relevant FIQ (updating the soft-copy in the process)
        LDR     r11,=INT_regs           ; interrupt control registers
        ORR     r12,r0,#FIQ_set         ; ensure we set this bit
        STR     r12,[r11,#FIQ_control]  ; enable this FIQ
        LDR     r11,=hardware_regs
        LDR     r12,[r11,#FIQ_data]     ; current value
        ORR     r12,r12,r0              ; set the bit we have just enabled
        STR     r12,[r11,#FIQ_data]     ; update the soft-copy

        LDMIA   r3,{r8-r13}		; load FIQ r8..r13 from passed values

        TEQP    pc,#PSRflags            	; SVC mode; IRQs/FIQs disabled
        NOP                             	; wait for re-mapping
	LDMFD	sp!,{r14}			; recover return address
        LDMFD   sp!,{r7,r8,r9,r10,r11,r12}	; recover work registers
        BICS    pc,link,#Vbit           	; and exit

FIQ_codesize_exit
	LDMFD	sp!,{r14}			; recover return address
        MOV     r0,#-2  	                ; supplied code is too large
        LDMFD   sp!,{r7,r8,r9,r10,r11,r12}	; recover work registers
        ORRS    pc,link,#Vbit
FIQ_active_exit
	LDMFD	sp!,{r14}			; recover return address
        ]       ; EOF (hercules)
        MOV     r0,#-1                  	; FIQ already in use
        LDMFD   sp!,{r7,r8,r9,r10,r11,r12}	; recover work registers
        ORRS    pc,link,#Vbit

        ; ---------------------------------------------------------------------

local_AttachFIQ ; simulate the SWI entry
        STMFD   sp!,{r11,r12}
code_exec_AttachFIQ
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = source mask (single bit describing desired FIQ)
        ;       r1  = handler code address
        ;       r2  = handler code length
        ;       r3  = default register set address
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r1  = preserved
        ;       r2  = preserved
        ;       r3  = preserved
        ;       processor mode restored
        ;       V clear => FIQ handler attached; r0 = preserved
        ;       V set   => attach failed; r0 = error code
        ;                                       = -1 : cannot share FIQ
        ;                                       = -2 : code too large
	;					= -3 : source already active
        ;
        ; Attach a multiple FIQ handler. This will fail if there is a single
        ; FIQ user, the code is too large or the array entry is already used.

	STMFD	sp!,{r7,r8,r9,r10}	; stack work registers

	[	{TRUE}
	STMFD	sp!,{r0,lk}
	ADR	r0,fatxt1
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	faovr1
fatxt1	=	"AttachFIQ: r0 = &",&00
	ALIGN
faovr1
	]	; EOF {boolean}

	[	(hercules)
	; Disable IRQs over the following code (allowing the code to be
	; called from IRQ, SVC and USR mode threads).
	STMFD	sp!,{r14}		; preserve return address and PSR
	BL	local_disableIRQs	; disable processor IRQs
	; We do not explicitly re-enable processor IRQs, since the exit
	; code should restore the callers IRQ state.

	; Check if we are allowed to register another FIQ handler
	LDR	r11,=FIQ_single
	LDR	r11,[r11,#&00]		; load the flag word
	TEQ	r11,#&00000000		; and check for multiple users
	BNE	FIQ_active_exit		; single user FIQ active

        ; Check that the supplied code is NOT too large for the FIQ space.
        CMP     r2,#(top_FIQ_stack - single_FIQ_code)
        BCS     FIQ_codesize_exit	; code too large for array entry

	; Check if the desired FIQ source is already active
	LDR	r11,=hardware_regs
	LDR	r11,[r11,#FIQ_data]	; current active FIQ sources/handlers
	TST	r11,r0			; check for our FIQ source being active
	BNE	FIQ_used_exit		; this source already has a handler

	; Calculate the array entry where this code will be placed
	LDR	r7,=FIQ_array		; address the base of the vector array
	MOV	r8,r0			; copy the bit-mask
AttachFIQ_find_loop			; loop around finding the correct entry
	MOVS	r8,r8,LSR #1		; set Carry if we find bit
	ADDCC	r7,r7,#FIQ_array_entry_size	; step onto next entry
	BCC	AttachFIQ_find_loop	; and go around the loop again
	; r7 = array entry address

	[	{TRUE}
	STMFD	sp!,{r0,lk}
	ADR	r0,aftxt99
	BL	local_Output
	MOV	r0,r7
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	afovr99
aftxt99	=	"AttachFIQ: base address (r7) = &",&00
	ALIGN
afovr99
	]	; EOF {boolean}

	STMFD	sp!,{r7}		; preserve over the copying code
	ADD	r10,r7,#FIQ_regs	; step over the FIQ register area
	MOV	r11,#&00		; starting loop index
	BL	move_FIQcode		; copy the code down
	; the "move_FIQcode" function corrupts r7,r8,r9,r11 and r12
	LDMFD	sp!,{r7}		; recover array entry address

	MOV	r12,#(Fbit :OR: FIQmode)
	TEQP	r12,#&00000000			; enter FIQ mode; FIQs disabled
	NOP					; wait for register re-mapping

	LDMIA	r3,{r8-r13}			; load passed FIQ reg. values
	TEQ	r13,#top_FIQ_stack
	ADDEQ	r13,r7,#(FIQ_regs + (top_FIQ_stack - single_FIQ_code))
	STMIA	r7,{r8-r13}			; store into the array entry

	; Place our router instruction at "single_FIQ_code"
	ADRL	r7,multiple_FIQ_handler		; our FIQ router
	MOV	r8,#single_FIQ_code		; destination address
	ADD	r9,r8,#&08			; source = source + 8
	SUB	r7,r7,r9			; offset = destination - source
	MOV	r7,r7,LSR #2			; offset = offset >> 2
	ORR	r7,r7,#(cond_AL :OR: op_b)	; branch instruction
	STR	r7,[r8,#&00]			; update FIQ vector contents

	[	{TRUE}
	LDR	sp,=DummySpace
	STMFD	sp!,{r0,lk}
	ADR	r0,aftxt88
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	afovr88
aftxt88	=	"AttachFIQ: source (r0) = &",&00
	ALIGN
afovr88
	]	; EOF {boolean}

	; Enable the relevant FIQ source (updating the soft-copy)
	LDR	r11,=INT_regs			; interrupt control registers
	ORR	r12,r0,#FIQ_set			; ensure we set this bit
	STR	r12,[r11,#FIQ_control]		; and enable this FIQ

	LDR	r11,=hardware_regs		; address the soft-copies
	LDR	r12,[r11,#FIQ_data]		; current FIQ value
	ORR	r12,r12,r0			; set the bit we have enabled
	STR	r12,[r11,#FIQ_data]		; and update the soft-copy

	TEQP	pc,#PSRflags			; SVC mode, IRQs/FIQs disabled
	NOP					; wait for register re-mapping
	LDMFD	sp!,{r14}			; recover return address
        LDMFD   sp!,{r7,r8,r9,r10,r11,r12}	; recover work registers
        BICS    pc,link,#Vbit			; and return to the caller

FIQ_used_exit
	LDMFD	sp!,{r14}			; recover return address
        MOV     r0,#-3  	                ; FIQ source already in use
        LDMFD   sp!,{r7,r8,r9,r10,r11,r12}	; recover work registers
        ORRS    pc,link,#Vbit			; and return to the caller
	|
	; NOT supported on non-Herculer architectures
	MOV	r0,#-1				; FIQ already in use
        LDMFD   sp!,{r7,r8,r9,r10,r11,r12}	; recover work registers
        ORRS    pc,lk,#Vbit			; and return V set
        ]	; EOF (hercules)

        ; ---------------------------------------------------------------------
	[	(hercules)
	; Copy a piece of memory, performing branch instruction relocation.
move_FIQcode
	; in:	r1  : source address
	;	r2  : number of bytes to move
	;	r7  : undefined work register
	;	r8  : undefined work register
	;	r9  : undefined work register
	;	r10 : destination address
	;	r11 : loop index
	;	r12 : undefined work register
	[	{TRUE}
	STMFD	sp!,{r0,lk}
	ADR	r0,mfctxt1
	BL	local_Output
	MOV	r0,r1
	BL	local_WriteHex8
	ADR	r0,mfctxt2
	BL	local_Output
	MOV	r0,r10
	BL	local_WriteHex8
	ADR	r0,mfctxt3
	BL	local_Output
	MOV	r0,r2
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	mfcovr1
mfctxt1	=	"move_FIQcode: r1 = &",&00
mfctxt2	=	" r10 = &",&00
mfctxt3 =	" r2 = &",&00
	ALIGN
mfcovr1
	]	; EOF {boolean}
move_FIQcode_loop
        LDR     r12,[r1,r11]            ; load instruction from source
	; We need to modify branch instructions that reference code external
	; to the copied region. The registers r7, r8 and r9 are available
	; for temporary work.

	AND	r7,r12,#op_mask		; get the major opcode group value
	TEQ	r7,#op_b		; branch instruction
	TEQNE	r7,#op_bl		; branch-with-link instruction
	BNE	move_FIQ_code_not_branch

	; the instruction is a branch
	LDR	r9,=offset_mask		; branch offset mask value
	AND	r8,r12,r9		; existing offset value
	ADD	r7,r1,r11		; calculate instruction address
	ADD	r7,r7,r8,LSL #2		; add in branch offset value
	ADD	r7,r7,#&08		; deal with pipe-lining
	BIC	r7,r7,#HIflags		; remove "address exception" bits
	; r7 = original branch instruction destination address
	ADD	r8,r1,r2		; r8 = end address of region
	CMP	r7,r1			; check against region start address
	RSBGES	r8,r7,r8		; and the region end address
	BGT	move_FIQ_code_not_branch; within the region, so do not modify

	BIC	r12,r12,r9		; r12 = empty branch instruction
	ADD	r8,r10,r11		; destination instruction address
	ADD	r8,r8,#&08		; source = source + 8
	SUB	r7,r7,r8		; offset = destination - source
	MOV	r7,r7,LSR #2		; word-align the offset
	AND	r7,r7,r9		; ensure the offset is in range
	ORR	r12,r12,r7		; and add offset into the instruction
move_FIQ_code_not_branch
        STR     r12,[r10,r11]           ; store at destination

        ADD     r11,r11,#&04            ; onto the next word
        CMP     r11,r2                  ; check for completion
        BCC     move_FIQcode_loop	; and around again if not

	; code copy completed
	MOVS	pc,lk			; return to caller

        ; ---------------------------------------------------------------------
	; This code deals with routing through to the relevant FIQ handler.
multiple_FIQ_handler
	; in:	FIQ mode; IRQs/FIQs disabled
	; 	r14 : processor mode and (return address + 4)
	; FIQs are definately single threaded and atomic. This allows us to
	; use an area of fixed RAM as a temporary storage area over the FIQ
	; call.

	GBLL	mfdebug
mfdebug	SETL	{TRUE}

	LDR	r8,=FIQ_multiple_regs	; reference our private workspace
	STMIA	r8,{r0,r14}		; store work registers

	[	(mfdebug)
	MOV	r9,r14
	LDR	r13,=DummySpace
	ADR	r0,mftxt1
	BL	local_Output
	MOV	r14,r9
	B	mfovr1
mftxt1	=	"multiple FIQ handler entered\n",&00
	ALIGN
mfovr1
	]	; EOF (mfdebug)

	; Calculate the array entry where this code will be placed
	LDR	r0,=FIQ_array			; base of the vector array
	MOV	r14,#INT_regs			; address the FIQ registers
	LDRB	r14,[r14,#FIQ_control]		; active FIQ request register

	; This code could be OPTIMISED by having a large branch table.
	; We could load the "byte" FIQ status, multiply up by a word
	; and load the address of the handler. This way we could prioritise
	; the handlers in a order other than the 0..7 (or 7..0) bit order.
	; The table could be hard-wired in the ROM, or even built dynamically
	; at run-time. This uses 1K of table space, and a smaller number of
	; instructions for referencing the correct entry.
	!	0,"TODO: optimise the multiple FIQ handler entry"

	[	(mfdebug)
	MOV	r10,r0
	MOV	r9,r14
	ADR	r0,mftxt3
	BL	local_Output
	MOV	r0,r9
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	r14,r9
	MOV	r0,r10
	B	mfovr3
mftxt3	=	"mFIQ: FIQ_control (r14) = &",&00
	ALIGN
mfovr3
	]	; EOF (mfdebug)

	; Check for the lowest active FIQ bit
	ASSERT	(FIQ_sources = 8)		; following code assumes this
	; check bit 0
	MOVS	r14,r14,LSR #1			; set Carry if we find bit
	ADDCC	r0,r0,#FIQ_array_entry_size	; step onto next entry
	; check bit 1
	MOVCCS	r14,r14,LSR #1			; and set Carry if we find bit
	ADDCC	r0,r0,#FIQ_array_entry_size	; step onto next entry
	; check bit 2
	MOVCCS	r14,r14,LSR #1			; and set Carry if we find bit
	ADDCC	r0,r0,#FIQ_array_entry_size	; step onto next entry
	; check bit 3
	MOVCCS	r14,r14,LSR #1			; and set Carry if we find bit
	ADDCC	r0,r0,#FIQ_array_entry_size	; step onto next entry
	; check bit 4
	MOVCCS	r14,r14,LSR #1			; and set Carry if we find bit
	ADDCC	r0,r0,#FIQ_array_entry_size	; step onto next entry
	; check bit 5
	MOVCCS	r14,r14,LSR #1			; and set Carry if we find bit
	ADDCC	r0,r0,#FIQ_array_entry_size	; step onto next entry
	; check bit 6
	MOVCCS	r14,r14,LSR #1			; and set Carry if we find bit
	ADDCC	r0,r0,#FIQ_array_entry_size	; step onto next entry
	; if CC then it MUST be bit 7 (the final source bit)
	; if EQ (Z set) then this is the only ACTIVE (out-standing) FIQ
	; r0 = array entry base address

	[	(mfdebug)
	MOV	r10,r0
	MOV	r9,r14
	ADR	r0,mftxt4
	BL	local_Output
	MOV	r0,r10
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	r14,r9
	MOV	r0,r10
	B	mfovr4
mftxt4	=	"mFIQ: entry base address (r0) = &",&00
	ALIGN
mfovr4
	]	; EOF (mfdebug)

	STR	r0,[r8,#(2 * word)]	; place after the stored r0 and r14
	LDMIA	r0,{r8-r13}		; load the required FIQ registers

	ADD	r0,r0,#FIQ_regs		; and reference the handler code
	; NOTE: FIQ handlers normally exit with "SUBS pc,lk,#&04", we must
	;	remember to add the "&04" into our fabricated return address.
	ADRL	r14,(multiple_FIQ_handler_return + &04)
	ORR	r14,r14,#(Fbit :OR: Ibit :OR: FIQmode)
	MOV	pc,r0			; and enter the handler code
multiple_FIQ_handler_return
	; We return to this point after the FIQ handler code has executed
	LDR	r14,=FIQ_multiple_regs	; reference our private workspace
	LDR	r0,[r14,#(2 * word)]	; after the stored r0 and r14
	STMIA	r0,{r8-r13}		; store registers after FIQ handler
	; We should possibly loop around again if there were out-standing
	; FIQs requests active.

	[	(mfdebug)
	LDR	r13,=DummySpace
	MOV	r8,r14		; preserve r14
	ADR	r0,mftxt2
	BL	local_Output
	MOV	r0,#INT_regs			; address the FIQ registers
	LDRB	r0,[r0,#FIQ_control]		; active FIQ request register
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	r14,r8		; recover r14
	B	mfovr2
mftxt2	=	"multiple FIQ handler leaving: FIQ_control = &",&00
	ALIGN
mfovr2
	]	; EOF (mfdebug)

	LDMIA	r14,{r0,r14}		; recover entry r0 and r14
	SUBS	pc,r14,#&04		; and return to the caller
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------

local_ReleaseFIQ	; Simulate the SWI entry
	STMFD	sp!,{r11,r12}
code_exec_ReleaseFIQ
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = source mask (single bit describing desired FIQ)
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = preserved
        ;       processor mode restored
        ;       V clear => FIQ handler released
        ;       V set   => FIQ handler not found
        ;
	[	(hercules)
	; Disable IRQs over this code. Ensuring that we can be called from
	; USR, IRQ and SVC threads.
	MOV	r11,r14			; preserve return address and PSR
	BL	local_disableIRQs	; disable processor IRQs
	MOV	r14,r11			; and recover return address and PSR

	; Clear the desired FIQ source
	LDR	r11,=INT_regs		; interrupt control registers
	STR	r0,[r11,#FIQ_control]	; clear the specified bit
	; and update the soft-copy
	LDR	r11,=hardware_regs	; hardware register soft-copies
	LDR	r12,[r11,#FIQ_data]	; active FIQ sources
	BICS	r12,r12,r0		; clear the specified bit
	STR	r12,[r11,#FIQ_data]	; and update the soft-copy
	; If EQ (Z set) then there are no active FIQ sources. To be safe
	; though we store a NULL FIQ handler at the FIQ vector. We do not
	; disable FIQs to perform this, since there are no active FIQ
	; sources (which means the vector should never be called anyway).
	ADRL	r12,vector_FIQ_instruction
	LDR	r12,[r12,#&00]		; load the NULL instruction
	MOV	r11,#single_FIQ_code	; address the FIQ handler address
	STR	r12,[r11,#&00]		; and write the default instruction

	LDR	r11,=FIQ_single		; single FIQ user flag
	MOV	r12,#&00000000		; multiple FIQ handlers available
	STR	r12,[r11,#&00]		; and update the flag

	; We do not need to do anything with the information stored in the
	; multiple FIQ array entry, since it the "FIQ_data" bits that
	; reflect the array entry usage.

	; Return with V clear restoring callers state (including IRQ state)
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit
	|	; middle (hercules)
	; Code not provided on non-Hercules systems.
	LDMFD	sp!,{r11,r12}
	ORRS	pc,link,#Vbit
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------

local_DefaultFIQStack	; Simulate the SWI entry
	STMFD	sp!,{r11,r12}
code_exec_DefaultFIQStack
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = default FIQ r13 (FD stack)
        ;       processor mode restored
        ;       always returns with V clear

        LDR     r0,=top_FIQ_stack
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- Hercules hardware support ----------------------------------------
        ; ---------------------------------------------------------------------
        ; Hercules registers:
        ;  This is a contiguous list of location descriptions for the
        ;  Hercules write-only registers.
        ;  The address is encoded as follows:
        ;                               Address [25..0]
        ;  +-----------+-----------------------------------------------+---+
        ;  |R|x|x|x|x|x|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|A|T|T|
        ;  +-----------+-----------------------------------------------+---+
        ;
        ;       TT = width:  00 - 32bit
        ;                    01 - 16bit
        ;                    10 - 8bit
        ;                    11 - reserved
        ;        R = access: 0 - READ/WRITE     caller can write this location
        ;                    1 - READ only      caller can only read soft-copy
        ;        x = unused: 0 - always
        ;
hwreg_width_32          *       2_00000000000000000000000000000000
hwreg_width_16          *       2_00000000000000000000000000000001
hwreg_width_8           *       2_00000000000000000000000000000010
hwreg_width_mask        *       2_00000000000000000000000000000011
hwreg_access_rw         *       2_00000000000000000000000000000000
hwreg_access_ro         *       2_10000000000000000000000000000000
hwreg_unused_mask       *       2_01111100000000000000000000000000
hwreg_addr_mask         *       2_00000011111111111111111111111100
        ;
code_exec_HWRegisters
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = index of register to modify or -1 for address request
        ;       r1  = bits to clear or undefined if a1 == -1
        ;       r2  = bits to set or undefined if a1 == -1
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  V clear = OK
        ;               r0 = preserved
        ;               r1 = old value or base address of table if entry a1==-1
        ;               r2 = new value or table size (in bytes) if entry a1==-1
        ;       V set = FAILED (should never happen with entry a1 == -1)
        ;               r0 = preserved
        ;               r1 = old value
        ;               r2 = value that would have been written
        ;       Processor mode and PSR restored
        [       (hercules)
        ; new_value = ((old_value BIC r1) ORR r2)

        STMFD   sp!,{r3}

	[	{TRUE}
	MOV	r3,r0
	MOV	r12,lk
	ADR	r0,HWtxt1a
	BL	local_Output
	MOV	r0,r3
	BL	local_WriteHex8
	ADR	r0,HWtxt1b
	BL	local_Output
	MOV	r0,r1
	BL	local_WriteHex8
	ADR	r0,HWtxt1c
	BL	local_Output
	MOV	r0,r2
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	lk,r12
	MOV	r0,r3
	B	HWovr1
HWtxt1a	=	"HWRegisters: index &",&00
HWtxt1b	=	" bic &",&00
HWtxt1c	=	" orr &",&00
	ALIGN
HWovr1
	]

        MOV     r3,#&FFFFFFFF
        TEQ     r0,r3                           ; special case
        LDREQ   r1,=hardware_regs               ; table address
        MOVEQ   r2,#hardware_regs_list_end      ; table size in bytes
        LDMEQFD sp!,{r3,r11,r12}
        BICEQS  pc,link,#Vbit                   ; return to the caller

        MOV     r3,r1                           ; bitmask of bits to clear

        CMP     r0,#HWReg_ListEnd               ; check for valid index
        BCS     HWRegister_ReadOnly             ; no such register

	[	{TRUE}
	MOV	r11,r0
	MOV	r12,lk
	ADR	r0,HWtxt2
	BL	local_Output
	MOV	lk,r12
	MOV	r0,r11
	B	HWovr2
HWtxt2	=	"HWRegisters: register number in range",&0A,&00
	ALIGN
HWovr2
	]

        ADR     r11,hwregs_table
        LDR     r11,[r11,r0,LSL #2]             ; r11 = index[r11 + (r0 * 4)]
        ; r11 = register description

        MOV     r12,lk                          ; preserve return address
        BL      local_disableIRQs               ; ensure atomic operation
        MOV     lk,r12                          ; restore return address

        LDR     r12,=hardware_regs
        LDR     r1,[r12,r0,LSL #2]              ; load current soft-copy value

        BIC     r3,r1,r3                        ; clear the required bits
        ORR     r2,r2,r3                        ; set the required bits

        ; r0 = index
        ; r1 = old value
        ; r2 = new value

        TST     r11,#hwreg_access_ro            ; Read Only register copy
        BNE     HWRegister_ReadOnly             ; cannot write this register

	[	{TRUE}
	STMFD	sp!,{r0,lk}
	ADR	r0,HWtxt3
	BL	local_Output
	LDMFD	sp!,{r0,lk}
	B	HWovr3
HWtxt3	=	"HWRegisters: not READ ONLY",&0A,&00
	ALIGN
HWovr3
	]

        STR     r2,[r12,r0,LSL #2]		; update the soft-copy

        AND     r3,r11,#hwreg_width_mask        ; get the width information
        BIC     r11,r11,#(:NOT: hwreg_addr_mask); get the true hardware address
        TEQ     r3,#hwreg_width_8
        STREQB  r2,[r11,#&00]                   ; write 8bit true copy
        STRNE   r2,[r11,#&00]                   ; write 16/32bit true copy

        ; Exit, restoring callers interrupt state
        LDMFD   sp!,{r3,r11,r12}
        BICS    pc,link,#Vbit           ; return to the caller

HWRegister_ReadOnly
        LDMFD   sp!,{r3,r11,r12}
        ORRS    pc,link,#Vbit           ; return to the caller with FAILED

	; ---------------------------------------------------------------------
	!	0,"TODO: assemble time linking of tables with SWI.s values"

hwregs_table
        & hwreg_access_rw :OR: MEMMAP_regs                  :OR: hwreg_width_8
        & hwreg_access_rw :OR: CLOCK_regs                   :OR: hwreg_width_8
        & hwreg_access_ro :OR: (BANK_regs + BANK0_reg)      :OR: hwreg_width_8
        & hwreg_access_ro :OR: (BANK_regs + BANK1_reg)      :OR: hwreg_width_8
        & hwreg_access_ro :OR: (BANK_regs + BANK2_reg)      :OR: hwreg_width_8
        & hwreg_access_ro :OR: (BANK_regs + BANK3_reg)      :OR: hwreg_width_8
        & hwreg_access_ro :OR: (BANK_regs + BANK4_reg)      :OR: hwreg_width_8
        & hwreg_access_ro :OR: (BANK_regs + BANK5_reg)      :OR: hwreg_width_8
        & hwreg_access_ro :OR: (BANK_regs + BANK6_reg)      :OR: hwreg_width_8
        & hwreg_access_ro :OR: (BANK_regs + BANK7_reg)      :OR: hwreg_width_8
        & hwreg_access_rw :OR: (INT_regs + INT_status)      :OR: hwreg_width_16
        & hwreg_access_ro :OR: (INT_regs + IRQ_control)     :OR: hwreg_width_16
        & hwreg_access_ro :OR: (INT_regs + FIQ_control)     :OR: hwreg_width_16
        & hwreg_access_rw :OR: (TIMER_regs + TIMER_control) :OR: hwreg_width_8
        & hwreg_access_rw :OR: (LCD_regs + LCD_control)     :OR: hwreg_width_32
        & hwreg_access_rw :OR: (LCD_regs + LCD_linelength)  :OR: hwreg_width_8
        & hwreg_access_rw :OR: (LCD_regs + LCD_linerate)    :OR: hwreg_width_8
        & hwreg_access_rw :OR: (LCD_regs + LCD_numlines)    :OR: hwreg_width_8
        & hwreg_access_rw :OR: (DMA_regs + DMA_routing)     :OR: hwreg_width_16
	[	(activebook)
	& hwreg_access_rw :OR: (CONTROL_base + CONTROL_reg) :OR: hwreg_width_8
	|
	& &00000000	; keep the table size information
	]
hwregs_table_end
hwregs_table_size       *       (hwregs_table_end - hwregs_table)
        ASSERT  (hwregs_table_size = (hardware_regs_end - hardware_regs))
        ASSERT  ((hwregs_table_size / 4) = HWReg_ListEnd)

	; ---------------------------------------------------------------------
        |	; middle (hercules)
	; ---------------------------------------------------------------------        ; Not implemented on non-Hercules machines.
        MOV     r1,#&00000000           ; dummy value read
        MOV     r2,#&00000000           ; dummy value written
        LDMFD   sp!,{r11,r12}
        ORRS    pc,link,#Vbit
        ]	; EOF (hercules)

	; ---------------------------------------------------------------------
	; Provide access to the true read-only hardware locations.
code_exec_ROHWRegisters
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = index of register to read
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = preserved
	;	V clear : OK     r1 = value read
	;	V set   : FAILED r1 = preserved (invalid r0 index on entry)
        ;       Processor mode and PSR restored
	[	(hercules)
        CMP     r0,#ROHWReg_ListEnd             ; check for valid index
        BCS     ROHWRegister_BadIndex

        ADR     r11,rohwregs_table
        LDR     r11,[r11,r0,LSL #2]             ; r11 = index[r11 + (r0 * 4)]
        ; r11 = register description

        AND     r12,r11,#hwreg_width_mask       ; get the width information
        BIC     r11,r11,#(:NOT: hwreg_addr_mask); get the true hardware address
        TEQ     r12,#hwreg_width_8
	LDREQB	r1,[r11,#&00]			; byte read
	LDRNE	r1,[r11,#&00]			; word read

        ; Exit, restoring callers interrupt state
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; return to the caller

ROHWRegister_BadIndex
        LDMFD   sp!,{r11,r12}
        ORRS    pc,link,#Vbit           ; return to the caller with FAILED

	; Table of READ ONLY hardware locations
rohwregs_table
	[	(activebook)
	&	(CONTROL_base + STATUS_reg)                 :OR: hwreg_width_8
	|
	&	&00000000	; keep the table size information
	]	; EOF (activebook)
rohwregs_table_end
rohwregs_table_size	*	(rohwregs_table_end - rohwregs_table)
	ASSERT	((rohwregs_table_size / 4) = ROHWReg_ListEnd)

	; ---------------------------------------------------------------------
        |	; middle (hercules)
	; ---------------------------------------------------------------------        ; Not implemented on non-Hercules machines.
        MOV     r1,#&00000000           ; dummy value read
        LDMFD   sp!,{r11,r12}
        ORRS    pc,link,#Vbit
        ]	; EOF (hercules)

        ; ---------------------------------------------------------------------
        ; The "Claim" and "Release" HardWare Memory calls need to store
        ; information in the "ExecRoot" and "SaveState" structures to ensure
        ; that the correct mode is always in place for the relevant process
        ; threads. We should ensure that the soft-copy of the "MEMMAP_regs"
        ; is kept upto-date with the relevant threads.
code_exec_ClaimHWMemory
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  Processor mode and PSR restored

        ; Disable IRQs so that the hardware, soft-copy and ExecRoot state
        ; are all identical
        MOV     r12,lk                          ; preserve return address
        BL      local_disableIRQs               ; ensure atomic operation
        MOV     lk,r12                          ; restore return address

        [       (hercules)
        MOV     r11,#MAPEN_PHYS
        ; Enter physical memory map mode
        MOV     r12,#MEMMAP_regs
        STRB    r11,[r12,#&00]

        ; Update the MEMMAP_regs soft-copy register
        LDR     r12,=hardware_regs
        STR     r11,[r12,#MEMMAP_data]

        ; Update the ExecRoot structure
        LDR     r12,=ROOT_start
        STR     r11,[r12,#ExecRoot_memmap]
        |	; middle (hercules)
        ; Functional Prototype
        MOV     r11,#&00                ; set physical mode map
        ; Enter physical memory map mode
        LDR     r12,=mmu_mode
        STRB    r11,[r12,#&00]

        ; Update the ExecRoot structure
        LDR     r12,=ROOT_start
        STR     r11,[r12,#ExecRoot_memmap]
        ]	; EOF (hercules)

        ; Return to the caller, restoring their interrupt state
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------

code_exec_ReleaseHWMemory
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  Processor mode and PSR restored

        ; Disable IRQs so that the hardware, soft-copy and ExecRoot state
        ; are all identical
        MOV     r12,lk                          ; preserve return address
        BL      local_disableIRQs               ; ensure atomic operation
        MOV     lk,r12                          ; restore return address

        [       (hercules)
        MOV     r11,#(MAPEN_USR :OR: OS_MODE1)  ; enable OS mode 1
        ; Enter Operating System Mode 1
        MOV     r12,#MEMMAP_regs
        STRB    r11,[r12,#&00]

        ; Update the MEMMAP_regs soft-copy
        LDR     r12,=hardware_regs
        STR     r11,[r12,#MEMMAP_data]

        ; Update the ExecRoot structure
        LDR     r12,=ROOT_start
        STR     r11,[r12,#ExecRoot_memmap]
        |	; middle (hercules)
        ; Functional prototype
        [       {TRUE}                  ; bodge test for MJackson
        MOV     r11,#&00                ; physical memory map
        |
        MOV     r11,#(mmumode_mapen :OR: mmumode_osmode :OR: mmumode_mode1)
        ]
        ; Enter Operating System Mode 1
        LDR     r12,=mmu_mode
        STRB    r11,[r12,#&00]

        ; Update the ExecRoot structure
        LDR     r12,=ROOT_start
        STR     r11,[r12,#ExecRoot_memmap]
        ]	; EOF (hercules)

        ; Return to the caller, restoring their interrupt state
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------

code_exec_ResetCPU
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  Processor mode and PSR restored

	; At the moment this code relies on the Executive startup resetting
	; all the necessary hardware devices.

	; Re-enter THIS Executive image (ie. relative to this SWI instruction).
	ADRL	r11,exec_continue		; code address
	ORRS	pc,r11,#(INTflags :OR: SVCmode)	; IRQ/FIQ disabled, SVC mode

        ; ---------------------------------------------------------------------
        ; -- DMA support ------------------------------------------------------
        ; ---------------------------------------------------------------------

code_exec_ClaimDMAchannel
	; in:	r0 = DMA source identifier
	; out:	V clear : channel claimed
	;		  r0 = preserved
	;		  r1 = allocated channel number
	;		  r2 = allocation handle
	;	V set   : r0 = error code
	;			-1 = invalid DMA source identifier
	;			-2 = no free channel for this DMA source
	;		  r1 = undefined
	;		  r2 = undefined
	;
	[	(hercules)
	; This SWI should NOT be called from interrupt threads. The IDLE
	; process (USR mode) code accesses the DMA allocation table
	; directly.
	;
	ASSERT	(DMAchannel_maximum = 3)	; The code assumes this
	ASSERT	(DMAchan_MLIRX   = &02)		; assumed in "SWI.s"
	ASSERT	(DMAchan_codecRX = &01)		; assumed in "SWI.s"
	ASSERT	(DMAchan_codecTX = &01)		; assumed in "SWI.s"
	ASSERT	(DMAchan_TEST    = &01)		; assumed in "SWI.s"
	; The source list in "SWI.s" should have an encoded channel number of
	; zero. We rely on the following assert to assure this.
	ASSERT	((DMAchan_EXTD :AND: &FF000000) = &00)
	;
	; Acceptable DMA source identifiers:
	;	DMAsource_codecRX	CODEX RX
	;	DMAsource_codecTX       CODEX TX
	;	DMAsource_TEST          TEST MODE
	;	DMAsource_MLIRX         MLI RX
	;	DMAsource_MLITX         MLI TX
	;	DMAsource_EXTA		external channel A
	;	DMAsource_EXTB          external channel B
	;	DMAsource_EXTC          external channel C
	;	DMAsource_EXTD		external channel D
	;
	; For the general purpose DMA users the channels will be allocated in
	; the order 1, 2 and then 3. If channel 3 is required then the Helios
	; IDLE process will NOT place Hercules into processor IDLE mode
	; (low-power). This means that the battery life of the system will be
	; shorter if all three channels are used.
	;
	; NOTE: The following identifiers can only be used with the given DMA
	;       channels:
	;		DMAchan_codecRX		channel 1 only
	;		DMAchan_codecTX         channel 2 only
	;		DMAchan_TEST            channel 3 only
	; 	The identifiers passed to this function encode the channel
	;	number that the source is limited to.
	MOVS	r1,r0,LSR #DMAsource_shift	; get encoded channel number
	BNE	claimDMAspecial		; non-zero encoded channel
	; The "r0" DMA source can be attached to any of the available DMA
	; channels.
	CMP	r0,#DMAsource_MLIRX		; check against lowest value
	RSBGES	r12,r0,#DMAsource_EXTD		; and against highest value
	BLT	claimDMA_invalid_identifier	; error if out-of-range

	; We allocate the channels from 1..3 (so that the IDLE code has a
	; chance to shutdown the processor).
	MOV	r1,#0				; initial channel reference
	LDR	r12,=(DMAchannel_states - word)	; reference the table
DMAclaim_loop
	ADD	r1,r1,#1			; step onto the next channel
	CMP	r1,#DMAchannel_maximum		; and check if > than limit
	BGT	claimDMA_no_free_channels	; YES - then error exit

	LDR	r2,[r12,r1,LSL #2]		; load this table entry
	TST	r2,#DMAchannel_allocated	; is this channel in use?
	BNE	DMAclaim_loop			; YES - then around the loop

	ADD	r2,r2,#1			; increment the handle
	ORR	r2,r2,#DMAchannel_allocated	; mark this channel as used
	STR	r2,[r12,r1,LSL #2]		; and store back into the table
	; r0 = identifier (preserved from input)
	; r1 = channel number allocated
	; r2 = allocation handle (with "DMAchannel_allocated" bit set)
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit

claimDMAspecial
	; r0 = entry source identifier
	; r1 = channel number desired
	BIC	r2,r0,#(DMAsource_mask :SHL: DMAsource_shift)
	; r2 = the actual source identifier
	; The code assumes that these special channels occupy the same
	; identifier.
	ASSERT	(DMAchan_codecRX = DMAchan_codecTX)
	ASSERT	(DMAchan_codecRX = DMAchan_TEST)
	TEQ	r2,#DMAchan_codecRX		; check for valid source
	BNE	claimDMA_invalid_identifier	; NO - then quick exit
	LDR	r12,=(DMAchannel_states - word)	; index from 1 into the table
	LDR	r2,[r12,r1,LSL #2]		; and load the relevant state
	TST	r2,#DMAchannel_allocated	; check if channel in use
	BNE	claimDMA_no_free_channels	; YES - channel in use
	ADD	r2,r2,#1			; increment the handle
	ORR	r2,r2,#DMAchannel_allocated	; mark this channel as used
	STR	r2,[r12,r1,LSL #2]		; and store back into the table
	; r0 = identifier (preserved from input)
	; r1 = channel number allocated
	; r2 = allocation handle (with "DMAchannel_allocated" bit set)
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit

claimDMA_invalid_identifier
	MOV	r0,#-1		; invalid source identifier
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit

claimDMA_no_free_channels
	]	; EOF (hercules)
	MOV	r0,#-2		; no free channels
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit

        ; ---------------------------------------------------------------------

code_exec_ReleaseDMAchannel
	; in:	r0 = DMA source identifier
	;	r1 = channel number allocated
	;	r2 = allocation handle
	; out:	V clear : channel released
	;		  r0 = preserved
	;		  r1 = preserved
	;		  r2 = preserved
	;	V set   : r0 = error code
	;			-1 = invalid DMA source identifier
	;			-2 = invalid allocation handle
	;			-3 = channel not allocated to given source
	;		  r1 = preserved
	;		  r2 = preserved
	;
	[	(hercules)
	; This SWI should NOT be called from interrupt threads.
	;
	MOVS	r11,r0,LSR #DMAsource_shift	; get encoded channel number
	BNE	releaseDMAspecial		; non-zero encoded channel
	; not a special source
	CMP	r0,#DMAsource_MLIRX		; check against lowest value
	RSBGES	r12,r0,#DMAsource_EXTD		; and against highest value
	BLT	claimDMA_invalid_identifier	; error if out-of-range

	B	releaseDMAchannel		; and continue with common code

releaseDMAspecial
	; r0 = entry source identifier
	; r11 = channel number encoded in r0
	BIC	r12,r0,#(DMAsource_mask :SHL: DMAsource_shift)
	; r12 = the actual source identifier
	; The code assumes that these special channels occupy the same
	; identifier.
	ASSERT	(DMAchan_codecRX = DMAchan_codecTX)
	ASSERT	(DMAchan_codecRX = DMAchan_TEST)
	TEQ	r12,#DMAchan_codecRX		; check for valid source
	BNE	claimDMA_invalid_identifier	; NO - then quick exit

	; Check that the channel allocation number is valid
	TEQ	r1,r11
	BNE	releaseDMA_not_ours

releaseDMAchannel
	; r0  = validated source identifier
	; r1  = allocated channel number
	; r2  = allocation handle
	; Check that the allocation handle matches the current DMA allocation
	; state for the referenced channel.
	LDR	r12,=(DMAchannel_states - word)
	LDR	r11,[r12,r1,LSL #2]		; load the current state word
	TEQ	r2,r11				; check for validity
	BNE	releaseDMA_not_ours		; invalid handle
	; The DMA channel was allocated to this thread, so...		
	BIC	r11,r11,#DMAchannel_allocated	; clear the allocated bit
	STR	r11,[r12,r1,LSL #2]		; and update the state word
	; r0, r1 and r2 preserved from entry
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit

releaseDMA_not_ours
	MOV	r0,#-2			; channel not allocated to this thread
	; r1 and r2 preserved from entry
	]	; EOF (hercules)
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit

        ; ---------------------------------------------------------------------

code_exec_ClaimEXsource
	; in:	r0 = external interrupt source required
	; out:	V clear : external source claimed
	;		  r0 = preserved
	;		  r1 = allocation handle
	;	V set   : r0 = error code
	;			-1 = invalid external source identifier
	;			-2 = external source already allocated
	;		  r1 = undefined
	;
	[	(hercules)
	; Acceptable source identifiers are:
	;	sourceEXA
	;	sourceEXB
	;	sourceEXC
	;	sourceEXD
	;	extsourceMAX is the limit

	CMP	r0,#&00000001		; lower limit
	RSBGES	r11,r0,#extsourceMAX	; upper limit
	BLT	source_invalid		; outside range

	BL	local_disableIRQs		; ensure atomic operation

	; r0 = table entry
	LDR	r12,=(extsource_states - word)	; reference the table
	LDR	r1,[r12,r0,LSL #2]		; load this table entry
	TST	r1,#extsource_allocated		; is this channel in use?
	BNE	source_allocated		; YES - then exit with error

	ADD	r1,r1,#1			; increment the handle
	ORR	r1,r1,#extsource_allocated	; mark this channel as used
	STR	r1,[r12,r0,LSL #2]		; and store back into the table
	; r0 = identifier (preserved from input)
	; r1 = allocation handle (with "extsource_allocated" bit set)
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit			; return restoring IRQ state

source_invalid
	MOV	r0,#-1				; invalid source identifier
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit			; return restoring IRQ state

source_allocated
source_badallocation
	MOV	r0,#-2				; source already allocated
	]	; EOF (hercules)
	LDMFD	sp!,{r11,r12}			; return restoring IRQ state
	ORRS	pc,lk,#Vbit

        ; ---------------------------------------------------------------------

code_exec_ReleaseEXsource
	; in:	r0 = external source allocated
	;	r1 = allocation handle
	; out:  V clear : external source released
	;		  r0 = preserved
	;		  r1 = preserved
	;	V set   : r0 = error code
	;			-1 = invalid external source identifier
	;			-2 = invalid allocation handle
	;		  r1 = preserved
	;
	[	(hercules)
	; check for valid source identifier
	CMP	r0,#&00000001			; lower limit
	RSBGES	r11,r0,#extsourceMAX		; upper limit
	BLT	source_invalid			; outside range

	BL	local_disableIRQs		; ensure atomic operation

	; r0 = table entry
	LDR	r12,=(extsource_states - word)	; reference the table
	LDR	r11,[r12,r0,LSL #2]		; load this table entry
	TST	r11,#extsource_allocated	; is this channel in use?
	BEQ	source_badallocation		; NO - then exit with error

	TEQ	r1,r11				; handles match?
	BNE	source_badallocation		; NO - then exit with
	BIC	r11,r11,#extsource_allocated	; mark this channel as clear
	STR	r11,[r12,r0,LSL #2]		; and store back into the table
	; r0 = identifier (preserved from input)
	; r1 = allocation handle (preserved from input)
	LDMFD	sp!,{r11,r12}
	BICS	pc,lk,#Vbit			; return restoring IRQ state
	|
	LDMFD	sp!,{r11,r12}
	ORRS	pc,lk,#Vbit
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------
        LNK     loswi5.s
