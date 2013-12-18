        SUBT Lo-level debugger                                      > lodebug/s
        ;    Copyright (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ARM Helios Executive: debugger
        ;
        ; Author:               James G Smith
        ; History:      900720  Created
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; FATAL error (write direct to screen) error handler.
	; THIS SHOULD BE REMOVED, since the Executive should not know
	; about the screen shape, position etc. However, if it turns out
	; that we can read this information from some other source (uLink)
	; then it may be worth keeping.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; To try and minimise the amount of information lost when we display
        ; FATAL error messages, we will only write into the last 32 character
        ; columns of the display.
        ;
        ; At the moment, to avoid affecting the Executive workspace, this
        ; code uses the unused LCD display memory as its workspace.
        ; ---------------------------------------------------------------------
	[	(:LNOT: dynlcd)
        ; "LCD_base"            base address of the screen memory
	]	; EOF (:LNOT: dynlcd)
        ; "LCD_displaywidth"    displayed raster width in bytes
        ; "LCD_planewidth"      complete raster width
        ; "LCD_planes"          number of planes
        ; "LCD_height"          number of rasters
        ; "LCD_stride"          step between displayed rasters (on a plane)
        ; "LCD_size"            complete size of the screen in bytes
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Extended debugger
        ; -----------------
        ;       This should be provided at a very lo-level initially, to 
        ;       allow it to be used in debugging the OS and system startup.
        ;       It will be accessed via the 2nd (debugging) link adaptor
        ;       present on functional prototypes.
        ;
        ;       The "Debugger" program should provide the following features:
        ;               Memory editor (byte/word/ASCII based)
        ;               Memory disassembler (allowing branches to be followed)
        ;               Memory Search/Replace/Copy/Verify
        ;               Single-Step code execution
        ;               BreakPoints (even in ROM)
        ;               Processor/System RESET (restart the system)
        ;
        ;       The "Debugger" should be present in the Executive so that
        ;       it can provide specific (version dependant) information and
        ;       direct access to certain Executive operations.
        ;
        ;       It would be nice if the "Debugger" was capable of the
        ;       following scenario:
        ;
        ;               HALTing the system on the next Scheduler entry.
        ;               Interrogating and manipulating the process queues.
        ;               Continuing the system as normal.
        ;               Single-stepping a particular process (whilst the
        ;               rest of the system is enabled or disabled).
        ;               Single-stepping an IRQ or FIQ (with the rest of
        ;               the system disabled).
        ;               BreakPointing code sections (optionally stopping
        ;               the complete system when a BreakPoint is hit).
        ;
        ;       RAM BreakPoints are easy. ROM BreakPoints require that the
        ;       complete system be simulated or that aborts can be generated
        ;       on particular addresses (or blocks).
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; This code provides yet another character bitmap. A proper fatal
        ; error notification system will need to be developed. The ideal is
        ; for the Executive to be as hardware independant as possible. It
        ; should therefore NOT know about screen characteristics directly.
        ; Seperate and remote video display drivers are provided at a higher
        ; level.
        ; Some piece of code that can be started after the display driver can
        ; provide a local screen error system (ie. what is performed below).
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

FatalError
        ; in:   r0 = 32bit error number
        ;       r1 = pointer to NULL terminated error message
        ;       r2 = pointer to "register_dump"
        ;       SVC mode; IRQs as caller; FIQs as caller

        ; This code is only called when Helios has DIED. We should disable
        ; all interrupt sources before dumping as much useful information as
        ; possible to the screen.

        MOV     r3,#(INTflags :OR: SVCmode)	; I and F flag set, SVC mode
        TEQP    r3,#&00000000			; SVC mode, IRQs/FIQs disabled

		GBLL	upsidedown
upsidedown	SETL	{FALSE}

        ; Initialise error display area (clear, with marking column).
	[	(upsidedown)
        ; Due to the upside down screen, we actually clear the left most
        ; 32 columns.
	]	; EOF (upsidedown)
	[	(dynlcd)
	; load r3 with the tier 1 LCD DMA base address.
	MOV	r3,#DMA_base			; DMA hardware registers
	LDR	r3,[r3,#LCDscreen_base0]	; tier 1 DMA descriptor
	MOV	r3,r3,LSR #DMA_src_shift	; and get into correct range
	|
	LDR	r3,=LCD_base
	]	; EOF (dynlcd)
	MOV	r4,#LCD_height
        ADRL    r5,zeroes
	[	(upsidedown)
        LDMIA   r5,{r5,r6,r7,r8,r9,r10,r11}     ; 28bytes (== 28 characters)
        MOV     r12,#&00FF0000                  ; column guide at the 31st char
	|	; middle (upsidedown)
	LDMIA	r5,{r6,r7,r8,r9,r10,r11,r12}
        MOV     r5,#&0000FF00 	                ; column guide at the 31st char
	ADD	r3,r3,#(LCD_displaywidth - 32)
	]	; EOF (upsidedown)
debug_clear_loop
        STMIA   r3,{r5,r6,r7,r8,r9,r10,r11,r12}
	ADD	r3,r3,#LCD_stride
        SUBS    r4,r4,#&01
        BNE     debug_clear_loop

	[	(upsidedown)
	SUB	r12,r3,#LCD_stride		; last raster
        ADD     r12,r12,#28                     ; starting character position
        ; r12 = character writing point
	SUB	r11,r12,#(8 * LCD_stride)	; next column guide
	|	; middle (upsidedown)
	[	(dynlcd)
	; load r12 with the tier 1 LCD DMA base address.
	MOV	r12,#DMA_base			; DMA hardware registers
	LDR	r12,[r12,#LCDscreen_base0]	; tier 1 DMA descriptor
	MOV	r12,r12,LSR #DMA_src_shift	; and get into correct range
	|	; middle (dynlcd)
	LDR	r12,=LCD_base
	]	; EOF (dynlcd)
	ADD	r12,r12,#(LCD_displaywidth - 28); starting character position
	ADD	r11,r12,#(8 * LCD_stride)	; next column guide
	]	; EOF (upsidedown)

        ADRL    r0,initial_text
        BL      debug_WriteS

        MOV     r0,r1
        BL      debug_WriteS
        BL      debug_NewLine

        MOV     r1,#0
debug_regs
        MOV     r0,r1
        BL      debug_RegisterOut
        LDR     r0,[r2,r1,LSL #2]       ; load the register contents
        BL      debug_WriteHex8
        BL      debug_NewLine
        ADD     r1,r1,#1
        CMP     r1,#16
        BCC     debug_regs

        LDR     r4,[r2,#(&0F * 4)]
        AND     r3,r4,#MODEmask
        ADRL    r0,debug_modes
        ADD     r0,r0,r3,LSL #2
        BL      debug_WriteS
        MOV     r0,#" "
        BL      debug_WriteC

        ADRL    r3,debug_flags
        MOV     r4,r4,LSR #26           ; move flags into the bottom 6 bits
        MOV     r5,#(1 :SHL: 5)         ; index
debug_flags_loop
        TST     r4,r5
        LDR     r0,[r3],#&01            ; character
        ANDNE   r0,r0,#&DF              ; upper-case if bit set
        BL      debug_WriteC
        MOVS    r5,r5,LSR #1
        BNE     debug_flags_loop
        BL      debug_NewLine
FatalErrorLoop
        B       FatalErrorLoop

        ; ---------------------------------------------------------------------

initial_text
        =       "Fatal Error",&0A
        =       "-----------------------------",&0A,null
debug_modes
        =       "USR",null
        =       "FIQ",null
        =       "IRQ",null
        =       "SVC",null
debug_flags
	; These are the "unset" values. Use (char AND &DF) for "set" value.
        =       "nzcvif",null
        ALIGN

        ; ---------------------------------------------------------------------

debug_WriteC
        ; in:   r0  = ASCII character code to write.
        ;       r10 = work register
        ;       r11 = next column guide
        ;       r12 = memory address where character is to be written
        ; out:  r0  = corrupted
        ;       r10 = corrupted
        ;       r11 = next column guide
        ;       r12 = updated
        ;       all other registers should be preserved
        ;
        ; Characters are forced to 7bit. All characters between &20..&7E
        ; (inclusive) are printed. The ASCII value &0A will force the character
        ; writing to the next line. All other ASCII values will generate a
        ; filled block.

        AND     r0,r0,#&7F
        CMP     r0,#&0A
        BEQ     debug_donewline

        CMP     r0,#&7F
        BEQ     debug_dofilled

        SUBS    r0,r0,#&20
        BCC     debug_dofilled

        ADRL    r10,charset             ; default character set
        ADD     r10,r10,r0,LSL #3       ; and index the character we want
        B       debug_dochar

debug_dofilled
        ADRL    r10,charfilled
	[	(upsidedown)
debug_dochar
        ; r10 = 8bytes of character data
        LDR     r0,[r10,#&00]           ; first 4bytes
        STRB    r0,[r12,#&00]           ; first byte
	SUB	r12,r12,#LCD_stride	; onto the next raster
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; second byte
	SUB	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; third byte
	SUB	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; fourth byte
	SUB	r12,r12,#LCD_stride

        LDR     r0,[r10,#&04]           ; second 4bytes
        STRB    r0,[r12,#&00]           ; fifth byte
	SUB	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; sixth byte
	SUB	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; seventh byte
	SUB	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; eighth byte
	SUB	r12,r12,#LCD_stride

        ; move the writing position to the next character column
	ADD	r12,r12,#(LCD_stride * 8)	; back up 8 rasters
        SUB     r12,r12,#&01                    ; and right a byte

	MOVS	pc,lk

debug_donewline
        ; now move to the LH display column
        MOV     r12,r11
	SUB	r11,r11,#(8 * LCD_stride)
debug_WriteC_exit
        MOVS    pc,lk
	|	; middle (upsidedown)
debug_dochar
        ; r10 = 8bytes of character data
        LDR     r0,[r10,#&00]           ; first 4bytes
        STRB    r0,[r12,#&00]           ; first byte
	ADD	r12,r12,#LCD_stride	; onto the next raster
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; second byte
	ADD	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; third byte
	ADD	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; fourth byte
	ADD	r12,r12,#LCD_stride

        LDR     r0,[r10,#&04]           ; second 4bytes
        STRB    r0,[r12,#&00]           ; fifth byte
	ADD	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; sixth byte
	ADD	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; seventh byte
	ADD	r12,r12,#LCD_stride
        MOV     r0,r0,LSR #8
        STRB    r0,[r12,#&00]           ; eighth byte
	ADD	r12,r12,#LCD_stride

        ; move the writing position to the next character column
	SUB	r12,r12,#(LCD_stride * 8)	; back up 8 rasters
        ADD     r12,r12,#&01                    ; and right a byte

	MOVS	pc,lk

debug_donewline
        ; now move to the LH display column
        MOV     r12,r11
	ADD	r11,r11,#(8 * LCD_stride)
debug_WriteC_exit
        MOVS    pc,lk
	]	; EOF (upsidedown)

        ; ---------------------------------------------------------------------

debug_WriteS
        ; in:   r0  = NULL terminated ASCII string
        ;       r11 = next column guide
        ;       r12 = memory address where string is to be written
        MOV     r13,r14
        MOV     r6,r0
debug_writeloop
        LDRB    r0,[r6],#&01
        CMP     r0,#&00
        BLNE    debug_WriteC
        BNE     debug_writeloop
        MOVS    pc,r13

        ; ---------------------------------------------------------------------

debug_NewLine
        ; in:   r12 = current memory address
        ;       r11 = next column guide
        MOV     r13,r14
        MOV     r0,#&0A
        BL      debug_WriteC
        MOVS    pc,r13

        ; ---------------------------------------------------------------------

debug_WriteHex8
        ; in:   r0  = 32bit value to be displayed as HEX ASCII
        ;       r11 = next column guide
        ;       r12 = memory address where number is to be written
        MOV     r13,r14
        MOV     r6,r0
        MOV     r7,#&00000020
        ADRL    r8,HexString
debug_hexloop
        SUB     r7,r7,#&04
        MOV     r9,r6,LSR r7
        AND     r9,r9,#&0000000F
        LDRB    r0,[r8,r9]
        BL      debug_WriteC    
        CMP     r7,#&00000000
        BNE     debug_hexloop
        MOVS    pc,r13

        ; ---------------------------------------------------------------------

debug_RegisterOut
        ; in:   r0  = register number to provide text prefix for
        ;       r11 = next column guide
        ;       r12 = memory address where text is to be written
        ; e.g.
        ;       " r0 = &"
        ;       "r10 = &"
        MOV     r13,r14
        MOV     r6,r0

        CMP     r6,#&0A
        MOVCC   r0,#" "
        BLCC    debug_WriteC

        MOV     r0,#"r"
        BL      debug_WriteC

        CMP     r6,#&0A
        MOVCS   r0,#"1"
        BLCS    debug_WriteC
        SUBCS   r6,r6,#&0A

        ADD     r0,r6,#"0"
        BL      debug_WriteC
        MOV     r0,#" "
        BL      debug_WriteC
        MOV     r0,#"="
        BL      debug_WriteC
        MOV     r0,#" "
        BL      debug_WriteC
        MOV     r0,#"&"
        BL      debug_WriteC
        MOVS    pc,r13

        ; ---------------------------------------------------------------------

	LTORG

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Helios_monitor:
        ;
        ; This function is linked into the abort/error mechanism by updating
        ; the system error vector using "exec_VectorPath".
        ;
Helios_monitor
        ; in:   r0  = int signum
        ;       r1  = NULL terminated ASCII error message
        ;       r2  = ptr to "register_dump" of registers at instance of error
        ;       r3  = recoverable
        ;       SVC mode; IRQs disabled; FIQs as caller.
        ;       lk  = callers return address and PSR
        ; out:  All registers restored from the register dump area
        ;
        ; Notes:
        ;  If the callers processor mode was SVC then we CANNOT return since
        ;  we lost  their svc_r14 when we were called.
        ;
        ;  A proper interrupt driven link adaptor handler should be written
        ;  allowing Helios to be debugged interactively down the link.
        ;
        ;  We are executing under the Helios Server, therefore we must conform
        ;  to the Helios message sending protocol.
        ;
        ;  We are SVC mode here, yet we are calling the USR mode character
        ;  output routines. This will work as long as the link register is
        ;  preserved around the SWI calls.
        ;
        MOV     r4,r1
        MOV     r5,r0
        MOV     r6,r2

	SWI	exec_NewLine

        MOV     r0,r4
        SWI     exec_Output                     ; display error message
        SWI     exec_NewLine

        ADR     r0,abort_error
        SWI     exec_Output

        MOV     r0,r5
        SWI     exec_WriteHex8
        SWI     exec_NewLine

        ADRL    r0,abort_message
        SWI     exec_Output

        MOV     r1,r6                   ; r1 = pointer to register dump
        MOV     r0,r6                   ; r0 = value to print
        SWI     exec_WriteHex8

        ADRL    r0,abort_message2
        SWI     exec_Output
        SWI     exec_NewLine

        ; display the dumped registers ("r6" holds the register dump address)
        MOV     r7,#&00                 ; start with register 0
display_regs_loop
        MOV     r2,r7
        MOV     r1,r6
        BL      RegisterOut
        ADD     r7,r7,#&01              ; step onto the next register

        ANDS    r0,r7,#&00000003        ; Z = r7 MOD 4 state

        MOVNE   r0,#" "                 ; padding space
        SWINE   exec_TerminalOut
        SWIEQ   exec_NewLine

        CMP     r7,#&10                 ; have we done all 16
        BCC     display_regs_loop

        SWI     exec_NewLine

        LDR     r8,[r6,#(&0F * 4)]      ; r15 index within the register dump

        ; display the PSR state ("r8" holds the PC+PSR value)
        ADR     r0,abort_mode
        SWI     exec_Output

        AND     r2,r8,#SVCmode          ; r2 = processor mode index
        ADRL    r3,debug_modes          ; r3 = textual descriptor address
        ADD     r0,r3,r2,LSL #2         ; r0 = r3 + (r2 * 4)
        SWI     exec_Output

        ADR     r0,abort_flags
        SWI     exec_Output

        MOV     r8,r8,LSR #26           ; r8 = flags (bottom 6 bits)
        ADRL    r5,debug_flags          ; r5 = unset flags text
        MOV     r7,#(1 :SHL: 5)         ; flags bit index
next_flag_loop
        TST     r8,r7
        LDR     r0,[r5],#&01            ; load textual descriptor
        ANDNE   r0,r0,#&DF              ; and make set if bit set
        SWI     exec_TerminalOut
        MOVS    r7,r7,LSR #1            ; shift mask bit down
        BNE     next_flag_loop          ; until it falls off the end

        SWI     exec_NewLine
        SWI     exec_NewLine

        ; display memory around the PC
        LDR     r4,[r6,#(&0F * &04)]    ; PC is register 15
        BIC     r4,r4,#PSRflags         ; use the address only
        SUBS    r5,r4,#&00000100        ; and display at most 64 words before
        MOVMI   r5,#&00000000           ; else the memory from &00000000
dmem_loop
        MOV     r0,r5
        SWI     exec_WriteHex8          ; display the address
        MOV     r0,#":"
        SWI     exec_TerminalOut
        MOV     r0,#" "
        SWI     exec_TerminalOut

        LDR     r0,[r5],#&04
        SWI     exec_WriteHex8
        SWI     exec_NewLine

        CMP     r5,r4                   ; check for termination
        BCC     dmem_loop

        SWI     exec_NewLine

        ; Note: This code is NOT perfect in that it will attempt to generate
        ;       "task" information for all exceptions. If the register dump
        ;       is not that of a Helios "task" then further exceptions can
        ;       be generated. The code should interrogate the hardware memory
        ;       system present and trap all illegal "pointer" values before
        ;       attempting to de-reference them.
        ;
        ;       Also, any pointer values derived from this operation must
        ;       lie within the global Helios heap.
        ;
        ; r6 = pointer to register dump
        ; r0,r1,r2,r3,r4,r5,r7,r8,r9 are available as work registers
        MOV     r9,#&00000000           ; if we generate an abort, this will
                                        ; stop further "dp" de-referencing.

        LDR     r4,[r6,#(&09 * 4)]      ; "dp" is register 9
        TEQ     r4,#&00000000
        BEQ     invalid_dp

        ADRL    r0,dp_message
        SWI     exec_Output

        ; ***** VERY TACKY WAY OF REFERENCING THE MODULE NAME *****
        LDR     r4,[r4,#(1 * 4)]        ; entry 1
        LDR     r4,[r4,#(48 * 4)]       ; entry 48
        LDR     r4,[r4,#(2 * 4)]        ; entry 2

        ADD     r4,r4,#&08              ; reference the start of the name
next_name_character
        LDR     r0,[r4],#&01
        CMP     r0,#&00
        SWINE   exec_TerminalOut
        BNE     next_name_character

        SWI     exec_NewLine
        SWI     exec_NewLine
invalid_dp

        ; Note: This code is NOT perfect in that it will attempt to generate
        ;       a backtrace listing for all exceptions. If the register dump
        ;       is not that of a C program further exceptions can be
        ;       generated. The code should interrogate the hardware memory
        ;       system present and trap all illegal "fp" values before
        ;       attempting to indirect into the saved frame.
        ;
        ; r6 = pointer to register dump
        ; r0,r1,r2,r3,r4,r5,r7,r8,r9 are available as work registers

        LDR     r4,[r6,#(&0B * 4)]      ; "fp" is register 11
        CMP     r4,#&04000000           ; would generate "address exception"
        BCS     no_fp_list

        LDR     r5,[r6,#(&0D * 4)]      ; "sp" is register 13
        LDR     r7,[r6,#(&0F * 4)]      ; "pc" is register 15

        ADRL    r0,backtrace_message
        SWI     exec_Output

        ADRL    r0,backtrace_fp
        SWI     exec_Output
backtrace_loop
        CMP     r4,#&00000000
        BEQ     fp_at_end
        CMP     r4,#&04000000           ; would generate "address exception"
        BCS     fp_at_end

        LDR     r8,[r4,#0]              ; load word at "fp[0]"
        BIC     r8,r8,#PSRflags         ; get the pure address
        SUB     r8,r8,#&0C              ; and step back 3 instructions

        LDR     r5,[r4,#-8]             ; load word at "fp[-2]"

        ADRL    r0,backtrace_notfound   ; default name
        MOV     r1,#&0B                 ; look upto 10 words back for STM
search_STM_loop
        SUBS    r1,r1,#&01
        BEQ     search_STM_loop_end

        SUB     r8,r8,#&04              ; step back a word
        LDR     r2,[r8,#&00]            ; and load the word

        MOV     r3,#&FF000000
        ORR     r3,r3,#&00FF0000
        AND     r9,r2,r3
        CMP     r9,#&FF000000
        BNE     search_STM_loop

        MOV     r3,r3,LSR #16           ; create the bottom mask
        AND     r2,r2,r3                ; mask out top
        SUB     r0,r8,r2
search_STM_loop_end
        SWI     exec_Output

        MOV     r0,#","                 ; print spacer
        SWI     exec_TerminalOut

        LDR     r4,[r4,#-12]            ; "fp = fp[-3]"
        B       backtrace_loop

fp_at_end
        ADRL    r0,backtrace_top
        SWI     exec_Output
        SWI     exec_NewLine
no_fp_list

        ADRL    r0,message_debugger
        SWI     exec_Output

        ; loop waiting for RESET
        ; This is where the interactive server debugger would be entered
termination_loop
        B       termination_loop

        ; ---------------------------------------------------------------------

PSRout
        ; in:  no state
        ; out: all state preserved
        ; Display the current processor mode and PSR status

        STMFD   sp!,{r0,r1,r2,r3,r4,link}

        MOV     r4,pc

        ADR     r0,abort_mode
        SWI     exec_Output

        AND     r2,r4,#SVCmode          ; r2 = processor mode index
        ADRL    r3,debug_modes          ; r3 = textual descriptor address
        ADD     r0,r3,r2,LSL #2         ; r0 = r3 + (r2 * 4)
        SWI     exec_Output

        ADR     r0,abort_flags
        SWI     exec_Output

        MOV     r4,r4,LSR #26           ; r4 = flags (bottom 6 bits)

        ADRL    r1,debug_flags          ; r1 = unset flags text

        MOV     r3,#(1 :SHL: 5)         ; flags bit index
next_PSRflag_loop
        TST     r4,r3
        LDR     r0,[r1],#&01            ; load textual descriptor
        ANDNE   r0,r0,#&DF              ; and make set if bit set
        SWI     exec_TerminalOut
        MOVS    r3,r3,LSR #1            ; shift mask bit down
        BNE     next_PSRflag_loop       ; until it falls off the end

        SWI     exec_NewLine

        LDMFD   sp!,{r0,r1,r2,r3,r4,pc}^

	; ---------------------------------------------------------------------

RegisterOut
        ; in:  r1 = base address of register dump area
        ;      r2 = register number (0..15) of register to be displayed
        ; out: r0 = preserved

        STMFD   sp!,{r0,r4,r5,r6,r7,lk}

        MOV     r4,r1                   ; register dump area address
        MOV     r6,r2                   ; remember the register number
        MOV     r5,r6
        ADRL    r7,HexString            ; we use the first 10 digits

        MOV     r0,#"r"                 ; register prefix
        SWI     exec_TerminalOut

        CMP     r6,#&0A
        ; if register number > 10 then display leading "1"
        MOVCS   r0,#"1"
        SWICS   exec_TerminalOut
        SUBCS   r6,r6,#&0A
        LDR     r0,[r7,r6]              ; load the relevant character
        SWI     exec_TerminalOut
        ; if register number < 10 then we need to display an extra space
        MOVCC   r0,#" "
        SWICC   exec_TerminalOut

        ADR     r0,abort_equals
        SWI     exec_Output

        MOV     r6,r5,LSL #2            ; register address = number * 4
        LDR     r0,[r4,r6]
        SWI     exec_WriteHex8

        LDMFD   sp!,{r0,r4,r5,r6,r7,pc}^

	; ---------------------------------------------------------------------

DisplayQueue
        ; in:   r0 = Queue head
        ; out:  all state preserved
        STMFD   sp!,{r0,r1,r2,r3,lk}
        MOV     r1,r0
        LDR     r1,[r1,#SaveState_next]
DisplayQueue_loop
        CMP     r1,#&00000000
        BEQ     DisplayQueue_tail
        MOV     r0,r1
        SWI     exec_WriteHex8
        MOV     r0,#","
        SWI     exec_TerminalOut                        ; seperator
        LDR     r1,[r1,#SaveState_next]
        B       DisplayQueue_loop
DisplayQueue_tail
        ; r1 = NULL
        MOV     r0,r1
        SWI     exec_WriteHex8
        SWI     exec_NewLine
        LDMFD   sp!,{r0,r1,r2,r3,pc}^   

        ; ---------------------------------------------------------------------

DisplayQueueEndtime
        ; in:   r0 = Queue head
        ; out:  all state preserved
        STMFD   sp!,{r0,r1,r2,r3,lk}
        MOV     r1,r0
        LDR     r1,[r1,#SaveState_next]
DisplayQueueEndTime_loop
        CMP     r1,#&00000000
        BEQ     DisplayQueue_tail
        LDR     r0,[r1,#SaveState_endtime]
        SWI     exec_WriteHex8
        MOV     r0,#","
        SWI     exec_TerminalOut                ; seperator
        LDR     r1,[r1,#SaveState_next]
        B       DisplayQueueEndTime_loop
DisplayQueueEndTime_tail
        ADR     r0,DisplayQueueEndTime_Message
        SWI     exec_Output
        LDMFD   sp!,{r0,r1,r2,r3,pc}^   
DisplayQueueEndTime_Message
        =       "<END>",&0A,&00
        ALIGN

        ; ---------------------------------------------------------------------
        ; error handler messages ----------------------------------------------
        ; ---------------------------------------------------------------------

abort_error
        =       "Error number &",null

abort_message
        =       "Register dump (stored at &",null
abort_message2
        =       ") is:",null
abort_equals
        =       " = ",null
abort_mode
        =       "Mode ",null
        ALIGN

abort_flags
        =       ", PSR flags: ",null

message_debugger
        =       "Executive halted (RESET HELIOS computer)",cr,lf,null

eroot_message
        =       "ExecRoot structure at &",null

dp_message
        =       "current task = ",null

nobacktrace_message
        =       "no "                   ; terminated by following message
backtrace_message
        =       "stack backtrace:",cr,lf,null
backtrace_fp
        =       "fp = [",null
backtrace_notfound
        =       "<name not found>",null
backtrace_top
        =       "<top of frame>]",cr,lf,null

        ALIGN           ; after all the messages

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; Character bitmaps for direct screen blitting

charfilled
        =       &FF,&FF,&FF,&FF,&FF,&FF,&FF,&FF ; special case
charset
        [       (upsidedown)
        ; non-reversed
        =       &00,&00,&00,&00,&00,&00,&00,&00 ; &20 " "
        =       &18,&18,&18,&18,&18,&00,&18,&00 ; &21 "!"
        =       &6C,&6C,&6C,&00,&00,&00,&00,&00 ; &22 """
        =       &6C,&6C,&FE,&6C,&FE,&6C,&6C,&00 ; &23 "#"
        =       &18,&3E,&68,&3C,&16,&7C,&18,&00 ; &24 "$"
        =       &62,&66,&0C,&18,&30,&66,&46,&00 ; &25 "%"
        =       &70,&D8,&D8,&70,&DA,&CC,&76,&00 ; &26 "&"
        =       &0C,&0C,&18,&00,&00,&00,&00,&00 ; &27 "'"
        =       &0C,&18,&30,&30,&30,&18,&0C,&00 ; &28 "("
        =       &30,&18,&0C,&0C,&0C,&18,&30,&00 ; &29 ")"
        =       &00,&18,&7E,&3C,&7E,&18,&00,&00 ; &2A "*"
        =       &00,&18,&18,&7E,&18,&18,&00,&00 ; &2B "+"
        =       &00,&00,&00,&00,&00,&18,&18,&30 ; &2C ","
        =       &00,&00,&00,&FE,&00,&00,&00,&00 ; &2D "-"
        =       &00,&00,&00,&00,&00,&18,&18,&00 ; &2E "."
        =       &00,&06,&0C,&18,&30,&60,&00,&00 ; &2F "/"
        =       &7C,&C6,&CE,&D6,&E6,&C6,&7C,&00 ; &30 "0"
        =       &18,&38,&18,&18,&18,&18,&7E,&00 ; &31 "1"
        =       &7C,&C6,&0C,&18,&30,&60,&FE,&00 ; &32 "2"
        =       &7C,&C6,&06,&1C,&06,&C6,&7C,&00 ; &33 "3"
        =       &1C,&3C,&6C,&CC,&FE,&0C,&0C,&00 ; &34 "4"
        =       &FE,&C0,&FC,&06,&06,&C6,&7C,&00 ; &35 "5"
        =       &3C,&60,&C0,&FC,&C6,&C6,&7C,&00 ; &36 "6"
        =       &FE,&06,&0C,&18,&30,&30,&30,&00 ; &37 "7"
        =       &7C,&C6,&C6,&7C,&C6,&C6,&7C,&00 ; &38 "8"
        =       &7C,&C6,&C6,&7E,&06,&0C,&78,&00 ; &39 "9"
        =       &00,&00,&18,&18,&00,&18,&18,&00 ; &3A ":"
        =       &00,&00,&18,&18,&00,&18,&18,&30 ; &3B ";"
        =       &06,&1C,&70,&C0,&70,&1C,&06,&00 ; &3C "<"
        =       &00,&00,&FE,&00,&FE,&00,&00,&00 ; &3D "="
        =       &C0,&70,&1C,&06,&1C,&70,&C0,&00 ; &3E ">"
        =       &7C,&C6,&C6,&0C,&18,&00,&18,&00 ; &3F "?"
        =       &7C,&C6,&DE,&D6,&DC,&C0,&7C,&00 ; &40 "@"
        =       &7C,&C6,&C6,&FE,&C6,&C6,&C6,&00 ; &41 "A"
        =       &FC,&C6,&C6,&FC,&C6,&C6,&FC,&00 ; &42 "B"
        =       &7C,&C6,&C0,&C0,&C0,&C6,&7C,&00 ; &43 "C"
        =       &F8,&CC,&C6,&C6,&C6,&CC,&F8,&00 ; &44 "D"
        =       &FE,&C0,&C0,&FC,&C0,&C0,&FE,&00 ; &45 "E"
        =       &FE,&C0,&C0,&FC,&C0,&C0,&C0,&00 ; &46 "F"
        =       &7C,&C6,&C0,&CE,&C6,&C6,&7C,&00 ; &47 "G"
        =       &C6,&C6,&C6,&FE,&C6,&C6,&C6,&00 ; &48 "H"
        =       &7E,&18,&18,&18,&18,&18,&7E,&00 ; &49 "I"
        =       &3E,&0C,&0C,&0C,&0C,&CC,&78,&00 ; &4A "J"
        =       &C6,&CC,&D8,&F0,&D8,&CC,&C6,&00 ; &4B "K"
        =       &C0,&C0,&C0,&C0,&C0,&C0,&FE,&00 ; &4C "L"
        =       &C6,&EE,&FE,&D6,&D6,&C6,&C6,&00 ; &4D "M"
        =       &C6,&E6,&F6,&DE,&CE,&C6,&C6,&00 ; &4E "N"
        =       &7C,&C6,&C6,&C6,&C6,&C6,&7C,&00 ; &4F "O"
        =       &FC,&C6,&C6,&FC,&C0,&C0,&C0,&00 ; &50 "P"
        =       &7C,&C6,&C6,&C6,&CA,&CC,&76,&00 ; &51 "Q"
        =       &FC,&C6,&C6,&FC,&CC,&C6,&C6,&00 ; &52 "R"
        =       &7C,&C6,&C0,&7C,&06,&C6,&7C,&00 ; &53 "S"
        =       &FE,&18,&18,&18,&18,&18,&18,&00 ; &54 "T"
        =       &C6,&C6,&C6,&C6,&C6,&C6,&7C,&00 ; &55 "U"
        =       &C6,&C6,&6C,&6C,&38,&38,&10,&00 ; &56 "V"
        =       &C6,&C6,&D6,&D6,&FE,&EE,&C6,&00 ; &57 "W"
        =       &C6,&6C,&38,&10,&38,&6C,&C6,&00 ; &58 "X"
        =       &C6,&C6,&6C,&38,&18,&18,&18,&00 ; &59 "Y"
        =       &FE,&0C,&18,&30,&60,&C0,&FE,&00 ; &5A "Z"
        =       &7C,&60,&60,&60,&60,&60,&7C,&00 ; &5B "["
        =       &00,&60,&30,&18,&0C,&06,&00,&00 ; &5C "\"
        =       &3E,&06,&06,&06,&06,&06,&3E,&00 ; &5D "]"
        =       &10,&38,&6C,&C6,&82,&00,&00,&00 ; &5E "^"
        =       &00,&00,&00,&00,&00,&00,&00,&FF ; &5F "_"
        =       &3C,&66,&60,&FC,&60,&60,&FE,&00 ; &60 "`"
        =       &00,&00,&7C,&C6,&C6,&C6,&7F,&00 ; &61 "a"
        =       &C0,&C0,&FC,&C6,&C6,&C6,&FC,&00 ; &62 "b"
        =       &00,&00,&7C,&C6,&C0,&C6,&7C,&00 ; &63 "c"
        =       &06,&06,&7E,&C6,&C6,&C6,&7E,&00 ; &64 "d"
        =       &00,&00,&7C,&C6,&FE,&C0,&7C,&00 ; &65 "e"
        =       &3E,&60,&60,&FC,&60,&60,&60,&00 ; &66 "f"
        =       &00,&00,&7E,&C6,&C6,&7E,&06,&7C ; &67 "g"
        =       &C0,&C0,&FC,&C6,&C6,&C6,&C6,&00 ; &68 "h"
        =       &18,&00,&78,&18,&18,&18,&7E,&00 ; &69 "i"
        =       &18,&00,&38,&18,&18,&18,&18,&70 ; &6A "j"
        =       &C0,&C0,&C6,&CC,&F8,&CC,&C6,&00 ; &6B "k"
        =       &78,&18,&18,&18,&18,&18,&7E,&00 ; &6C "l"
        =       &00,&00,&EC,&FE,&D6,&D6,&C6,&00 ; &6D "m"
        =       &00,&00,&FC,&C6,&C6,&C6,&C6,&00 ; &6E "n"
        =       &00,&00,&7C,&C6,&C6,&C6,&7C,&00 ; &6F "o"
        =       &00,&00,&FC,&C6,&C6,&C6,&FC,&C0 ; &70 "p"
        =       &00,&00,&7E,&C6,&C6,&C6,&7E,&07 ; &71 "q"
        =       &00,&00,&DC,&F6,&C0,&C0,&C0,&00 ; &72 "r"
        =       &00,&00,&7E,&C0,&7C,&06,&FC,&00 ; &73 "s"
        =       &30,&30,&FC,&30,&30,&30,&1E,&00 ; &74 "t"
        =       &00,&00,&C6,&C6,&C6,&C6,&7E,&00 ; &75 "u"
        =       &00,&00,&C6,&C6,&6C,&38,&10,&00 ; &76 "v"
        =       &00,&00,&C6,&D6,&D6,&FE,&C6,&00 ; &77 "w"
        =       &00,&00,&C6,&6C,&38,&6C,&C6,&00 ; &78 "x"
        =       &00,&00,&C6,&C6,&C6,&7E,&06,&7C ; &79 "y"
        =       &00,&00,&FE,&0C,&38,&60,&FE,&00 ; &7A "z"
        =       &1C,&30,&30,&E0,&30,&30,&1C,&00 ; &7B "{"
        =       &00,&18,&18,&18,&18,&18,&18,&00 ; &7C "|"
        =       &70,&18,&18,&0E,&18,&18,&70,&00 ; &7D "}"
        =       &31,&6B,&46,&00,&00,&00,&00,&00 ; &7E "~"
        |
        ; reversed
        =       &00,&00,&00,&00,&00,&00,&00,&00 ; &20 " "
        =       &18,&18,&18,&18,&18,&00,&18,&00 ; &21 "!"
        =       &36,&36,&36,&00,&00,&00,&00,&00 ; &22 """
        =       &36,&36,&7F,&36,&7F,&36,&36,&00 ; &23 "#"
        =       &18,&7C,&16,&3C,&68,&3E,&18,&00 ; &24 "$"
        =       &46,&66,&30,&18,&0C,&66,&62,&00 ; &25 "%"
        =       &0E,&1B,&1B,&0E,&5B,&33,&6E,&00 ; &26 "&"
        =       &30,&30,&18,&00,&00,&00,&00,&00 ; &27 "'"
        =       &30,&18,&0C,&0C,&0C,&18,&30,&00 ; &28 "("
        =       &0C,&18,&30,&30,&30,&18,&0C,&00 ; &29 ")"
        =       &00,&18,&7E,&3C,&7E,&18,&00,&00 ; &2A "*"
        =       &00,&18,&18,&7E,&18,&18,&00,&00 ; &2B "+"
        =       &00,&00,&00,&00,&00,&18,&18,&0C ; &2C ","
        =       &00,&00,&00,&7F,&00,&00,&00,&00 ; &2D "-"
        =       &00,&00,&00,&00,&00,&18,&18,&00 ; &2E "."
        =       &00,&60,&30,&18,&0C,&06,&00,&00 ; &2F "/"
        =       &3E,&63,&73,&6B,&67,&63,&3E,&00 ; &30 "0"
        =       &18,&1C,&18,&18,&18,&18,&7E,&00 ; &31 "1"
        =       &3E,&63,&30,&18,&0C,&06,&7F,&00 ; &32 "2"
        =       &3E,&63,&60,&38,&60,&63,&3E,&00 ; &33 "3"
        =       &38,&3C,&36,&33,&7F,&30,&30,&00 ; &34 "4"
        =       &7F,&03,&3F,&60,&60,&63,&3E,&00 ; &35 "5"
        =       &3C,&06,&03,&3F,&63,&63,&3E,&00 ; &36 "6"
        =       &7F,&60,&30,&18,&0C,&0C,&0C,&00 ; &37 "7"
        =       &3E,&63,&63,&3E,&63,&63,&3E,&00 ; &38 "8"
        =       &3E,&63,&63,&7E,&60,&30,&1E,&00 ; &39 "9"
        =       &00,&00,&18,&18,&00,&18,&18,&00 ; &3A ":"
        =       &00,&00,&18,&18,&00,&18,&18,&0C ; &3B ";"
        =       &60,&38,&0E,&03,&0E,&38,&60,&00 ; &3C "<"
        =       &00,&00,&7F,&00,&7F,&00,&00,&00 ; &3D "="
        =       &03,&0E,&38,&60,&38,&0E,&03,&00 ; &3E ">"
        =       &3E,&63,&63,&30,&18,&00,&18,&00 ; &3F "?"
        =       &3E,&63,&7B,&6B,&3B,&03,&3E,&00 ; &40 "@"
        =       &3E,&63,&63,&7F,&63,&63,&63,&00 ; &41 "A"
        =       &3F,&63,&63,&3F,&63,&63,&3F,&00 ; &42 "B"
        =       &3E,&63,&03,&03,&03,&63,&3E,&00 ; &43 "C"
        =       &1F,&33,&63,&63,&63,&33,&1F,&00 ; &44 "D"
        =       &7F,&03,&03,&3F,&03,&03,&7F,&00 ; &45 "E"
        =       &7F,&03,&03,&3F,&03,&03,&03,&00 ; &46 "F"
        =       &3E,&63,&03,&73,&63,&63,&3E,&00 ; &47 "G"
        =       &63,&63,&63,&7F,&63,&63,&63,&00 ; &48 "H"
        =       &7E,&18,&18,&18,&18,&18,&7E,&00 ; &49 "I"
        =       &7C,&30,&30,&30,&30,&33,&1E,&00 ; &4A "J"
        =       &63,&33,&1B,&0F,&1B,&33,&63,&00 ; &4B "K"
        =       &03,&03,&03,&03,&03,&03,&7F,&00 ; &4C "L"
        =       &63,&77,&7F,&6B,&6B,&63,&63,&00 ; &4D "M"
        =       &63,&67,&6F,&7B,&73,&63,&63,&00 ; &4E "N"
        =       &3E,&63,&63,&63,&63,&63,&3E,&00 ; &4F "O"
        =       &3F,&63,&63,&3F,&03,&03,&03,&00 ; &50 "P"
        =       &3E,&63,&63,&63,&53,&33,&6E,&00 ; &51 "Q"
        =       &3F,&63,&63,&3F,&33,&63,&63,&00 ; &52 "R"
        =       &3E,&63,&03,&3E,&60,&63,&3E,&00 ; &53 "S"
        =       &7F,&18,&18,&18,&18,&18,&18,&00 ; &54 "T"
        =       &63,&63,&63,&63,&63,&63,&3E,&00 ; &55 "U"
        =       &63,&63,&36,&36,&1C,&1C,&08,&00 ; &56 "V"
        =       &63,&63,&6B,&6B,&7F,&77,&63,&00 ; &57 "W"
        =       &63,&36,&1C,&08,&1C,&36,&63,&00 ; &58 "X"
        =       &63,&63,&36,&1C,&18,&18,&18,&00 ; &59 "Y"
        =       &7F,&30,&18,&0C,&06,&03,&7F,&00 ; &5A "Z"
        =       &3E,&06,&06,&06,&06,&06,&3E,&00 ; &5B "["
        =       &00,&06,&0C,&18,&30,&60,&00,&00 ; &5C "\"
        =       &7C,&60,&60,&60,&60,&60,&7C,&00 ; &5D "]"
        =       &08,&1C,&36,&63,&41,&00,&00,&00 ; &5E "^"
        =       &00,&00,&00,&00,&00,&00,&00,&FF ; &5F "_"
        =       &3C,&66,&06,&3F,&06,&06,&7F,&00 ; &60 "`"
        =       &00,&00,&3E,&63,&63,&63,&FE,&00 ; &61 "a"
        =       &03,&03,&3F,&63,&63,&63,&3F,&00 ; &62 "b"
        =       &00,&00,&3E,&63,&03,&63,&3E,&00 ; &63 "c"
        =       &60,&60,&7E,&63,&63,&63,&7E,&00 ; &64 "d"
        =       &00,&00,&3E,&63,&7F,&03,&3E,&00 ; &65 "e"
        =       &7C,&06,&06,&3F,&06,&06,&06,&00 ; &66 "f"
        =       &00,&00,&7E,&63,&63,&7E,&60,&3E ; &67 "g"
        =       &03,&03,&3F,&63,&63,&63,&63,&00 ; &68 "h"
        =       &18,&00,&1E,&18,&18,&18,&7E,&00 ; &69 "i"
        =       &18,&00,&1C,&18,&18,&18,&18,&0E ; &6A "j"
        =       &03,&03,&63,&33,&1F,&33,&63,&00 ; &6B "k"
        =       &1E,&18,&18,&18,&18,&18,&7E,&00 ; &6C "l"
        =       &00,&00,&37,&7F,&6B,&6B,&63,&00 ; &6D "m"
        =       &00,&00,&3F,&63,&63,&63,&63,&00 ; &6E "n"
        =       &00,&00,&3E,&63,&63,&63,&3E,&00 ; &6F "o"
        =       &00,&00,&3F,&63,&63,&63,&3F,&03 ; &70 "p"
        =       &00,&00,&7E,&63,&63,&63,&7E,&E0 ; &71 "q"
        =       &00,&00,&3B,&6F,&03,&03,&03,&00 ; &72 "r"
        =       &00,&00,&7E,&03,&3E,&60,&3F,&00 ; &73 "s"
        =       &0C,&0C,&3F,&0C,&0C,&0C,&78,&00 ; &74 "t"
        =       &00,&00,&63,&63,&63,&63,&7E,&00 ; &75 "u"
        =       &00,&00,&63,&63,&36,&1C,&08,&00 ; &76 "v"
        =       &00,&00,&63,&6B,&6B,&7F,&63,&00 ; &77 "w"
        =       &00,&00,&63,&36,&1C,&36,&63,&00 ; &78 "x"
        =       &00,&00,&63,&63,&63,&7E,&60,&3E ; &79 "y"
        =       &00,&00,&7F,&30,&1C,&06,&7F,&00 ; &7A "z"
        =       &38,&0C,&0C,&07,&0C,&0C,&38,&00 ; &7B "{"
        =       &00,&18,&18,&18,&18,&18,&18,&00 ; &7C "|"
        =       &0E,&18,&18,&70,&18,&18,&0E,&00 ; &7D "}"
        =       &8C,&D6,&62,&00,&00,&00,&00,&00 ; &7E "~"
        ]	; EOF (upsidedown)

        ; ---------------------------------------------------------------------
        END
