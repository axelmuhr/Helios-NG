        SUBT Executive SWI handler and routines                      > loswi1/s
        ;    Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ARM HELIOS Executive (process scheduler and interrupt handlers)
        ;
        ; Author:               James G Smith
        ; History:      891024  Split from main source file "loexec.s"
        ;               900413  (Brian Knight) Functional Prototype version
        ;
        ; ---------------------------------------------------------------------
        ; -- Executive SWIs ---------------------------------------------------
        ; ---------------------------------------------------------------------

HeliosExecutiveSWI
        ; in:   r0..r10 : from caller
        ;       r11     : undefined
        ;       r12     : SWI comment field (bits 18..0)
        ;       svc_r13 : FD stack (containing entry r11 and r12)
        ;       svc_r14 : return address and entry PSR state
        ;       SVC mode; IRQs disabled; FIQs undefined.
        ; out:  r0..r10 : modified as per SWI reason code
        ;       r11     : entry r11
        ;       r12     : entry r12
        ;       svc_r13 : FD stack
        ;       svc_r14 : undefined
        ;       callers mode, IRQ and FIQ state.
        ;
        ; The "swi_exec/swi_noexec" bit must be clear for this vector to have
        ; been taken.
        ; Unknown SWIs should exit through the system error vector.
        ;
        ; NOTE: The SWI branch table needs to be kept in step with the SWI
        ;       numbering scheme in "SWI.a"

	[	((debug2) :LAND: {FALSE})
	STMFD	sp!,{r0,lk}
	ADRL	r0,swtxt0a
	BL	local_Output
	LDR	r0,[sp,#&04]	; stacked lk
	BL	local_WriteHex8
	ADRL	r0,swtxt0b
	BL	local_Output
	ADD	r0,sp,#&08	; pre-stack sp
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	swovr0
swtxt0a	=	"HeliosExecutiveSWI: lk = &",&00
swtxt0b	=	" sp = &",&00
	ALIGN
swovr0
	]	; EOF ((debug2) :LAND: {FALSE})

        AND     r11,svc_r14,#INTflags           ; entry IRQ and FIQ state
        TEQP    r11,#SVCmode                    ; preserve SVC mode

	[	((debug2) :LAND: {FALSE})
	STMFD	sp!,{r0,lk}
	ADR	r0,swtxt1
	BL	local_Output
	MOV	r0,r12
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	swovr1
swtxt1	=	"HeliosExecutiveSWI: r12 = &",&00
	ALIGN
swovr1
	]	; EOF ((debug2) :LAND: {FALSE})

        BIC     r12,r12,#(swi_Xbit :OR: swi_spare)
        ; error and spare bits are unused in Executive SWIs

        ADR     r11,exec_SWI_table

	[	((debug2) :LAND: {FALSE})
	STMFD	sp!,{r0,lk}
	ADR	r0,swtxt2
	BL	local_Output
	ADD	r0,r11,r12,LSL #2	; new address
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	swovr2
swtxt2	=	"HeliosExecutiveSWI: jump address = &",&00
	ALIGN
swovr2
	]	; EOF ((debug2) :LAND: {FALSE})

        CMP     r12,#numSWIs
        ADDCC   pc,r11,r12,LSL #2       ; pc = r11 + (r12 * 4)
        B       exec_UndefinedSWI       ; bad SWI number
exec_SWI_table
SWItable        SETL    {TRUE}          ; build the branch table
        GET     /hsrc/include/abcARM/asm/SWI.s
end_exec_SWI_table
numSWIs         *       ((end_exec_SWI_table - exec_SWI_table) / 4)

        ; ---------------------------------------------------------------------
        ; -- Helios SWIs ------------------------------------------------------
        ; ---------------------------------------------------------------------

HeliosSWI       ; there are none at the moment (generate a system error)
        ; in:   r0..r10 : parameters
        ;       r11     : undefined
        ;       r12     : SWI comment (OS field NULL and HE bit set)
        ;       r13     : FD stack (containing entry r11 and r12)
        ;       r14     : callers return address
        ;       SVC mode; IRQs disabled; FIQs undefined.

        ; temporarily store the svc_r14 (corrupting the reset vector)
        STR     svc_r14,[r0,-r0]

        ; recover the entry r11 and r12
        LDMFD   sp!,{r11,r12}

        ; place all the un-mapped registers into the register dump area
        LDR     svc_r14,=register_dump
        STMIA   svc_r14,{r0-r7}

	ADRL	r1,message_heliosSWI
	MOV	r0,#err_SWInotimp
        B       generic_handler

        ; ---------------------------------------------------------------------
        ; -- non-Helios SWIs --------------------------------------------------
        ; ---------------------------------------------------------------------

otherSWI        ; there are none at the moment (generate a system error)
        ; in:   r0..r10 : parameters
        ;       r11,r12 : general work registers
        ;       r13     : FD stack (containing entry r11 and r12)
        ;       r14     : callers return address
        ;       SVC mode; IRQs disabled; FIQs undefined.

        ; temporarily store the svc_r14 (corrupting the reset vector)
        STR     svc_r14,[r0,-r0]

        ; recover the entry r11 and r12
        LDMFD   sp!,{r11,r12}

        ; place all the un-mapped registers into the register dump area
        LDR     svc_r14,=register_dump
        STMIA   svc_r14,{r0-r7}

	ADRL	r1,message_otherSWI
	MOV	r0,#err_SWInotimp
        B       generic_handler

        ; ---------------------------------------------------------------------
        ; -- exec_UndefinedSWI ------------------------------------------------
        ; ---------------------------------------------------------------------

exec_UndefinedSWI
        ; in:  r0..r10 : parameters
        ;      r11,r12 : general work registers
        ;      r13     : FD stack (containing entry r11 and r12)
        ;      r14     : callers return address
        ;      SVC mode; IRQs undefined; FIQs undefined.

        AND     r11,svc_r14,#Fbit               ; entry FIQ state
        TEQP    r11,#(Ibit :OR: SVCmode)        ; disable IRQs

        LDMFD   sp!,{r11,r12}   ; recover entry r11 and r12

        ; temporarily store the svc_r14 (corrupting the reset vector)
        STR     svc_r14,[r0,-r0]

        ; place all the un-mapped registers into the register dump area

        LDR     svc_r14,=register_dump
        STMIA   svc_r14,{r0-r7}

	ADRL	r1,message_unknownSWI
	MOV	r0,#err_unknownSWI
        B       generic_handler

        ; ---------------------------------------------------------------------
        LNK     loswi2.s
