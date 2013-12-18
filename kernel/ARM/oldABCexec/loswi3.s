        SUBT    Executive SWI handler and routines                   > loswi3/s
        ; Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom.
        ; ---------------------------------------------------------------------
        ; -- exec_InitBackplane -----------------------------------------------
        ; ---------------------------------------------------------------------
        ; Initialise link adaptors.
        ; in:  SVC mode; IRQs undefined; FIQs undefined
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: processor mode preserved
        ;      V clear => Backplane initialised OK
        ;      V set   => failed to initialise backplane (no link adaptor!)
code_exec_InitBackplane
        AND     r11,svc_r14,#Fbit               ; entry FIQ state
        TEQP    r11,#(Ibit :OR: SVCmode)        ; disable IRQs (stay in SVC)

	LDR	r11,=LINK0_base
        MOV     r12,#&00
        STRB    r12,[r11,#LINK_rstatus]         ; disable read interrupts
        STRB    r12,[r11,#LINK_wstatus]         ; disable write interrupts

        ; ensure processor FIQs are enabled
        BIC     link,link,#Fbit

        [       (hercules)
	[	(hercmlink)
	[	(:LNOT: newbreak)	; now performed during Hercules RESET
	; reset the microlink interface
	MOV	r11,#MLI_regs		; base of microlink regs
	MOV	r12,#&00
	STRB	r12,[r11,#MLI_CON]	; clear microlink interface
	MOV	r12,#(MLI_ENA :OR: MLI_ICP); then reenable with
	STRB	r12,[r11,#MLI_CON]	;    internal clock
	]	; EOF (:LNOT: newbreak)

	; Install the microlink reception FIQ handler. 
	; This must be done before enabling the microlink rx IRQ,
	; otherwise the IRQ routine may be entered to deal with a
	; received byte.

	STMFD	sp!,{r0,r1,r2,r3,r14}	; Save some more regs
	SUB	sp,sp,#(6*4)		; Make room for FIQ register list
	; Initialise structure used to set FIQ regs r8-r13. DO NOT ALTER
	; THE ALLOCATION OF THE FIQ REGISTERS WITHOUT MAKING CORRESPONDING
	; CHANGES TO THE FIQ HANDLER (IN lomlink.s).
	MOV	r0,#0
	STR	r0,[sp,#(0*4)]		; fiq_r8:  work register
	STR	r0,[sp,#(1*4)]		; fiq_r9:  work register
	STR	r0,[sp,#(2*4)]		; fiq_r10: work register
	STR	r0,[sp,#(5*4)]		; fiq_r13: buffer end ptr or flag
					; (0 means msg header expected)
	MOV	r0,#(MLI_regs + MLI_RXD)
	STR	r0,[sp,#(3*4)]		; fiq_r11: microlink rx data reg addr
	LDR	r0,=ML_Rx_CurBuf	; address of current buffer pointer
	LDR	r0,[r0]			; current buffer pointer
	STR	r0,[sp,#(4*4)]		; fiq_r12: start of buffer

	MOV	r0,#INT_MRX		; FIQ source bit
	ADRL	r1,ML_RxFIQ		; Start of FIQ handler code
	MOV	r2,#(ML_RxFIQ_LowEnd - ML_RxFIQ); Len of code to be copied down
	MOV	r3,sp			; Initial set of FIQ regs (on stack)
	[	{FALSE}	; multiple FIQ support
	; Allow other FIQ sources to reside with this handler
	BL	local_AttachFIQ		; Install FIQ handler
	|
	!	0,"TODO: make microlink use the multiple FIQ handler system"
	BL	local_AttachSFIQ	; Install FIQ handler
	]	; EOF {boolean}

	; V is set if the FIQ handler was not installed; r0 has error code
	ADRVS	r1,InitBackplane_Msg1	; Address of error message
	LDRVS	r2,=register_dump	; Set ptr even though contents invalid
	STRVS	r0,[r2,#0]		; FatalError fails to print err code
	BVS	FatalError		; Kill the whole system

	; FIQ handler was installed
	ADD	sp,sp,#(6*4)		; Restore sp
	LDMFD	sp!,{r0,r1,r2,r3,r14}	; Restore regs

        ; Enable IRQs for the link adapter and 2 of the 3 microlink interrupts.
	; (Microlink tx is enabled only when needed.)
        MOV     r11,#INT_regs
	MOV	r12,#(IRQ_set :OR: INT_MBK)
	ORR	r12,r12,#(INT_MRX :OR: LINK_interrupt)
        STR     r12,[r11,#IRQ_control]	; enable the IRQs
        LDR     r11,=hardware_regs
        LDR     r12,[r11,#IRQ_data]     ; current bitmask
        ORR     r12,r12,#INT_MBK	; one bit we have added
	ORR	r12,r12,#(INT_MRX :OR: LINK_interrupt); more bits
        STR     r12,[r11,#IRQ_data]     ; and update the soft-copy

	|	; middle (hercmlink)

        ; enable IRQs for link adaptor only
        MOV     r11,#INT_regs
        LDR     r12,=(IRQ_set :OR: LINK_interrupt)
        STR     r12,[r11,#IRQ_control]
        LDR     r11,=hardware_regs
        LDR     r12,[r11,#IRQ_data]     ; current bitmask
        ORR     r12,r12,#LINK_interrupt ; bit we have added
        STR     r12,[r11,#IRQ_data]     ; and update the soft-copy

	]	; end (hercmlink)
        |       ; middle (hercules)
        ; enable link1 IRQs (for IO server) and link0 (microlink substitute)
        LDR     r11,=irq_mask           ; IRQ enable register
        MOV     r12,#(int_link0 :OR: int_link1); enable both link IRQs
        STRB    r12,[r11,#&00]
        LDR     r11,=IRQ_mask_copy      ; and ensure the soft copy is set too
        STR     r12,[r11,#&00]

	; Enable link0 reception interrupts: microlink messages are
	; received at all times.
        MOV     r11,#ml_link_base	; r11 = link adaptor base address
        MOV     r12,#LINK_intenable     ; enable read interrupts
        STRB    r12,[r11,#LINK_rstatus]
        ]       ; EOF (hercules)

        ; We should call the device drivers to perform a soft reset
        ; on their hardware. This will allow "InitBackplane" to be called
        ; whenever hardware devices need reset (without resorting to resetting
        ; the whole machine). Unfortunately... the Executive does not know
        ; about IO devices other than its link. The device drivers loaded
        ; by the Nucleus should ensure that their hardware is sensible
        ; before performing any operations.

        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

	[	(hercmlink)
InitBackplane_Msg1	=	"AttachSFIQ failed",&00
	]	; end (hercmlink)

        ; ---------------------------------------------------------------------
        ; -- exec_LinkWriteC --------------------------------------------------
        ; ---------------------------------------------------------------------
        ; in:  r0  = ASCII character to output
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = callers return address
        ;      SVC mode; IRQs undefined; FIQ undefined
        ; out: r0  = preserved
        ;      V clear => character written succesfully
        ;      V set   => unable to write character
WriteCharacter
        ; local call: so stack r11 and r12 to simulate SWI entry
        STMFD   sp!,{r11,r12}
code_exec_LinkWriteC
	LDR	r11,=LINK0_base

        ; Include check for out-standing interrupt transfer
        ; This exits with an error because we cannot guarantee that the
        ; transfer will succeed (even if we disable IRQs), since an interrupt
        ; driven transfer may start after the check, but before we poll.
        ; If an interrupt then occurs, the IRQ handler will process the
        ; interrupt and send the data, and we may corrupt that transfer.
        LDR     r12,=txbuffer_count
        LDR     r12,[r12,#&00]
        CMP     r12,#&00000000
        BNE     no_character_write

        ; r0  = ASCII character
        ; r11 = hardware base address
link_writeC_wait
        ; Poll "write status" until "write register" free.
        LDRB    r12,[r11,#LINK_wstatus]
        TST     r12,#LINK_data          ; bit set marks ready state
        BEQ     link_writeC_wait

        ; We can place the character into the write register.
        STRB    r0,[r11,#LINK_write]

        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- exec_LinkReadC ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; in:  r13 = FD stack (containing entry r11 and r12)
        ;      r14 = callers return address
        ; out: V clear => r0 = byte read from link adaptor
        ;      V set   => r0 = undefined (unable to read character)
ReadCharacter
        ; local call, so simulate SWI entry
        STMFD   sp!,{r11,r12}
code_exec_LinkReadC
	LDR	r11,=LINK0_base

        ; Include check for out-standing interrupt transfer
        ; This exits with an error because we cannot guarantee that the
        ; transfer will succeed (even if we disable IRQs), since an interrupt
        ; driven transfer may start after the check, but before we poll/read.
        ; If an interrupt then occurs, the IRQ handler will process the
        ; interrupt and eat the data, and we may possibly read invalid data.
        LDR     r12,=rxbuffer_count
        LDR     r12,[r12,#&00]
        CMP     r12,#&00000000
        BNE     no_character_read

link_readC_wait
        ; Poll "read status" until "read register" full.
        LDRB    r0,[r11,#LINK_rstatus]
        TST     r0,#LINK_data
        BEQ     link_readC_wait

        ; We can now read the character.
        LDRB    r0,[r11,#LINK_read]
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

no_character_read
no_character_write
        ; error return (set V)
        LDMFD   sp!,{r11,r12}
        ORRS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- exec_LinkWPoll ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; in:  r13 = FD stack (containing entry r11 and r12)
        ;      r14 = callers return address
        ; out: V clear => C clear => link adaptor write buffer empty
        ;                 C set   => buffer not empty
        ;      V set   => no link adaptor
code_exec_LinkWPoll
        ; If there is a transmit interrupt process going then return C set
        LDR     r11,=txbuffer_count
        LDR     r11,[r11,#&00]
        TEQ     r11,#&00000000
        LDMNEFD sp!,{r11,r12}
        ORRNE   link,link,#Cbit         ; "non-zero" so return C set
        BICNES  pc,link,#Vbit           ; and return with V clear

	LDR	r11,=LINK0_base
        LDRB    r12,[r11,#LINK_wstatus]
        TST     r12,#LINK_data

        LDMFD   sp!,{r11,r12}
        BICNE   link,link,#Cbit         ; flag set so return C clear
        ORREQ   link,link,#Cbit         ; flag clear so return C set
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        ; -- exec_LinkRPoll ---------------------------------------------------
        ; ---------------------------------------------------------------------
        ; in:  r13 = FD stack (containing entry r11 and r12)
        ;      r14 = callers return address
        ; out: V clear => C clear => link adaptor read buffer is full
        ;                 C set   => no character present in buffer
        ;      V set   => no link adaptor
code_exec_LinkRPoll
        ; If there is a receive interrupt process going then return C set
        LDR     r11,=rxbuffer_count
        LDR     r11,[r11,#&00]
        TEQ     r11,#&00000000
        LDMNEFD sp!,{r11,r12}
        ORRNE   link,link,#Cbit         ; "non-zero" so return C set
        BICNES  pc,link,#Vbit           ; and return with V clear

	LDR	r11,=LINK0_base
        LDRB    r12,[r11,#LINK_rstatus]
        TST     r12,#LINK_data

        LDMFD   sp!,{r11,r12}
        BICNE   link,link,#Cbit         ; flag set so return C clear
        ORREQ   link,link,#Cbit         ; flag clear so return C set
        BICS    pc,link,#Vbit           ; clear V flag

        ; ---------------------------------------------------------------------
        ; -- exec_StartLinkTx -------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Place the arguments into memory accessible from the link adaptor
        ; interrupt handler. Enable link adaptor buffer transmit empty
        ; interrupts.
        ; in:  SVC mode; IRQs undefined; FIQs undefined
        ;      r0  = transfer buffer address
        ;      r1  = transfer data amount (bytes)
        ;      r2  = (SaveState *) reference to process to re-start
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: r0  = preserved
        ;      r1  = preserved
        ;      r2  = preserved
        ;      processor mode restored
        ;      V clear => interrupt handler ready
        ;      V set   => interrupt transfer out-standing (not replaced)
        ;                 or no link adaptor podule
        ;
        ; An out-standing interrupt transfer is marked by the location
        ; "txbuffer_count" containing a non-zero value. If this contains zero
        ; then we can over-write the variables with our new values.
        ; (This is because the interrupt handler executes with IRQs disabled,
        ; so we cannot possibly update the variables when the interrupt
        ; routine is executing, and the interrupt routine will check for a
        ; transfer count of zero before exiting).
code_exec_StartLinkTx
        LDR     r11,=txbuffer_address
        LDR     r12,[r11,#(txbuffer_count - txbuffer_address)]
        CMP     r12,#&00000000          ; is transfer count NULL?

        ; "NE" then we have an out-standing TX interrupt
        LDMNEFD sp!,{r11,r12}           ; NO: then recover entry state
        ORRNES  pc,link,#Vbit           ;     and return V set

        STMIA   r11,{r0,r1,r2}          ; YES: then update registers

	LDR	r11,=LINK0_base
        MOV     r12,#LINK_intenable
        STRB    r12,[r11,#LINK_wstatus] ; enable TX interrupts

        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; return V clear

        ; ---------------------------------------------------------------------
        ; -- exec_AbortLinkTx -------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Place the arguments into memory accessible from the link adaptor
        ; interrupt handler. Enable link adaptor receive buffer full
        ; interrupts.
        ; in:  SVC mode; IRQs undefined; FIQs undefined
        ;      r0  = undefined (really "LinkInfo *")
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: r0  = buffer address of next byte to be written
        ;      r1  = number of bytes remaining to be written
        ;      r2  = pointer to the SaveState structure for the blocked process
        ;      processor mode restored
        ;      V clear => information valid
        ;      V set   => no link adaptor podule
code_exec_AbortLinkTx
	LDR	r11,=LINK0_base
        MOV     r12,#&00
        STRB    r12,[r11,#LINK_wstatus]

        ; We can now update the "LinkTx" control variables since the
        ; hardware should not be generating any interrupts.

        ; return the current "txbuffer" variable contents
        LDR     r11,=txbuffer_address
        LDMIA   r11,{r0,r1,r2}
        STR     r12,[r11],#&04          ; "r12" holds zero from above
        STR     r12,[r11],#&04
        STR     r12,[r11,#&00]

        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; return V clear

        ; ---------------------------------------------------------------------
        ; -- exec_StartLinkRx -------------------------------------------------
        ; ---------------------------------------------------------------------
        ; in:  SVC mode; IRQs undefined; FIQs undefined
        ;      r0  = transfer buffer address
        ;      r1  = transfer data amount (bytes)
        ;      r2  = (SaveState *) reference to process to re-start
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: r0  = preserved
        ;      r1  = preserved
        ;      r2  = preserved
        ;      processor mode restored
        ;      V clear => interrupt handler ready
        ;      V set   => interrupt transfer out-standing (not replaced)
        ;                 or no link adaptor podule
code_exec_StartLinkRx
        LDR     r11,=rxbuffer_address
        LDR     r12,[r11,#(rxbuffer_count - rxbuffer_address)]
        CMP     r12,#&00000000          ; is transfer count NULL?

        ; "NE" => there is a Rx interrupt transfer already active
        LDMNEFD sp!,{r11,r12}
        ORRNES  pc,link,#Vbit

        STMIA   r11,{r0,r1,r2}          ; YES: then update registers

	LDR	r11,=LINK0_base
        MOV     r12,#LINK_intenable
        STRB    r12,[r11,#LINK_rstatus]

        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; return V clear

        ; ---------------------------------------------------------------------
        ; -- exec_AbortLinkRx -------------------------------------------------
        ; ---------------------------------------------------------------------
        ; in:  SVC mode; IRQs undefined; FIQs undefined
        ;      r11 = undefined (work register)
        ;      r12 = undefined (work register)
        ;      r13 = FD stack (containing entry r11 and r12)
        ;      r14 = return address
        ; out: r0  = buffer address of next byte to be read
        ;      r1  = number of bytes remaining to be read
        ;      r2  = pointer to the SaveState structure of the blocked process
        ;      processor mode restored
code_exec_AbortLinkRx
	LDR	r11,=LINK0_base
        MOV     r12,#&00
        STRB    r12,[r11,#LINK_rstatus]

        ; We can now update the "LinkRx" control variables since the
        ; hardware should not be generating any interrupts.

        ; return the current "rxbuffer" variable contents (zeroing contents)
        LDR     r11,=rxbuffer_address
        LDMIA   r11,{r0,r1,r2}
        STR     r12,[r11],#&04          ; "r12" holds zero from above
        STR     r12,[r11],#&04
        STR     r12,[r11,#&00]

        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit           ; return V clear

        ; ---------------------------------------------------------------------
        ; -- Debugging --------------------------------------------------------
        ; ---------------------------------------------------------------------

local_TerminalOut
        STMFD   sp!,{r11,r12}   ; simulate SWI entry
code_exec_TerminalOut
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = 8bit ASCII character to be displayed
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = preserved
        ;       processor mode restored
        ;       always returns with V clear

        [       (debug2)
	LDR	r11,=LINK1_base		; base address of 2nd link adaptor
TerminalOut_poll
        LDRB    r12,[r11,#LINK_wstatus] ; load status flag
        TST     r12,#LINK_data          ; set PSR on mask bit
        BEQ     TerminalOut_poll        ; clear, then buffer is full
        ; buffer is empty, so write the byte
        STRB    r0,[r11,#LINK_write]
	|
        ; We do not have a 2nd link adaptor
        !       0,"Note: VOID function TerminalOut"
        ]
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------

local_TerminalIn
        STMFD   sp!,{r11,r12}   ; simulate SWI entry
code_exec_TerminalIn
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  V clear : r0 = 8bit ASCII character read
        ;       V set   : r0 = undefined (no character present)
        ;       processor mode restored

        [       (debug2)
        LDR     r11,=LINK1_base         ; base address of 2nd link adaptor
        LDRB    r12,[r11,#LINK_rstatus] ; load status flag
        TST     r12,#LINK_data          ; set PSR on mask bit
        LDRNEB  r0,[r11,#LINK_read]     ; buffer full, so read byte
        LDMFD   sp!,{r11,r12}           ; recover work registers
        BICNES  pc,link,#Vbit           ; byte read, so return V clear
        ORRS    pc,link,#Vbit           ; no byte present, so return V set
	|
        ; We do not have a 2nd link adaptor on HEVAL boards
        !       0,"Note: VOID function TerminalIn"
        MOV     r0,#&00         ; simulate NULL character
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit
        ]       ; (debug2)

        ; ---------------------------------------------------------------------

local_Output
        STMFD   sp!,{r11,r12}
code_exec_Output
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = NULL terminated ASCII string
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = preserved
        ;       processor mode restored
        ;       always returns with V clear
        [       (debug2)
        STMFD   sp!,{r0,r1}
	LDR	r1,=LINK1_base		; base address of 2nd link adaptor
Output_loop
        LDRB    r12,[r0],#&01           ; load character
        CMP     r12,#&00                ; NULL terminator?
        LDMEQFD sp!,{r0,r1}             ; (this can be merged with the next)
        LDMEQFD sp!,{r11,r12}
        BICEQS  pc,link,#Vbit           ; YES, then exit with V clear
        ; non-NULL character
Output_poll
        LDRB    r11,[r1,#LINK_wstatus]  ; load status flag
        TST     r11,#LINK_data          ; set PSR on mask bit
        BEQ     Output_poll             ; clear, then buffer is full
        ; buffer is empty, so write the byte
        STRB    r12,[r1,#LINK_write]
        B       Output_loop
	|
        ; We do not have a 2nd link adaptor
        !       0,"Note: VOID function Output"
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit
        ]       ; EOF (debug2)

        ; ---------------------------------------------------------------------

local_WriteDecimal
        STMFD   sp!,{r11,r12}
code_exec_WriteDecimal
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = 32bit unsigned value
        ;       r1  = number of characters printed
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = preserved
        ;       r1  = updated
        ;       processor mode restored
        ;       always returns with V clear

        STMFD   sp!,{r0,r2,r3,lk}
        SUB     sp,sp,#16                       ; reserve space for the number
        MOV     r3,sp                           ; storage index
print_decimal_convert
        MOV     r1,#10 :SHL: 28
        MOV     r2,#0
print_decimal_divide
        CMPS    r0,r1
        SUBCS   r0,r0,r1
        ADC     r2,r2,r2
        MOV     r1,r1,LSR #1
        CMPS    r1,#10
        BHS     print_decimal_divide

        ORR     r0,r0,#"0"
        STRB    r0,[r3],#1                      ; store character
        MOVS    r0,r2
        BNE     print_decimal_convert
print_decimal_loop
        LDRB    r0,[r3,#-1]!                    ; load character
        BL      local_TerminalOut               ; display character
        ADD     r11,r11,#1                      ; count character
        CMP     r3,sp
        BNE     print_decimal_loop
        ADD     sp,sp,#16
        LDMFD   sp!,{r0,r2,r3,lk}
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------

local_WriteHex8
        STMFD   sp!,{r11,r12}
code_exec_WriteHex8
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r0  = 32bit unsigned value
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  r0  = preserved
        ;       processor mode restored
        ;       always returns with V clear

        STMFD   sp!,{r0,r1,r2,r3,r4,link}

        MOV     r1,r0
        MOV     r2,#&00000020           ; number of bits in a word
        ADR     r3,HexString            ; and the character set we use
WriteHex_loop
        SUB     r2,r2,#&04              ; the size of a nybble

        MOV     r4,r1,LSR r2
        AND     r4,r4,#&0000000F        ; and mask out all but the nybble

        LDRB    r0,[r3,r4]              ; get the ASCII equivalent
        BL      local_TerminalOut       ; display the character

        CMP     r2,#&00000000           ; are we at the end of the word
        BNE     WriteHex_loop

        LDMFD   sp!,{r0,r1,r2,r3,r4,link}
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

HexString
        =       "0123456789ABCDEF"
        ALIGN

        ; ---------------------------------------------------------------------

local_NewLine
        STMFD   sp!,{r11,r12}
code_exec_NewLine
        ; in:   SVC mode; IRQs undefined; FIQs undefined
        ;       r11 = undefined (work register)
        ;       r12 = undefined (work register)
        ;       r13 = FD stack (containing entry r11 and r12)
        ;       r14 = return address
        ; out:  processor mode restored
        ;       always returns with V clear

        STMFD   sp!,{r0,link}
        MOV     r0,#&0A
        BL      local_TerminalOut
        LDMFD   sp!,{r0,link}
        LDMFD   sp!,{r11,r12}
        BICS    pc,link,#Vbit

        ; ---------------------------------------------------------------------
        LNK     loswi4.s
