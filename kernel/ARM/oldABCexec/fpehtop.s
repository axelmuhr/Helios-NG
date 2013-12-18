        TTL     Helios OS dependent FPE code                    > fpehtop/s
        ; ---------------------------------------------------------------------
        ; fpehtop.s     890727  JGSmith, Active Book Company, United Kingdom.
        ;
        ; Derived from "asm2.fpe2ltop" source of MClemoes, Acorn Computers Ltd.
        ;
        ; This is the top level file of the FPE assembler code. This file
        ; includes (via "GET") all the mathematical routines. The code
        ; produced by OBJASMing this file is intended to be linked with code
        ; providing the functionality of the "asm2.fpe2stub" file.
        ;
        ; Modified:     900413  BJKnight Modified for functional prototype
        ;               891205  JGSmith Allow code to be included directly or
        ;                               constructed seperately.
        ;               891128  JGSmith Add individual process FP state
        ;               891123  JGSmith Initial work
        ;
        ; ---------------------------------------------------------------------

old_opt SETA    {OPT}

	GBLL	fpspace
fpspace	SETL	{TRUE}		; use new FP state space code (not stack based)

	GBLL	fpdebug
fpdebug	SETL	{FALSE}		; FPE "Output" debugging included

        ; ---------------------------------------------------------------------

                GBLL    arm2            ; Do we have a MUL instruction?
arm2            SETL    {TRUE}
                GBLL    Arm2
Arm2            SETL    arm2            ; synonym for above

        ; ---------------------------------------------------------------------
        ; The following variables are required to support other versions
        ; of the FPE object... they should not be used in Helios/ARM.

                GBLA    filelevel
                GBLL    debugging
debugging       SETL    {FALSE}
        ASSERT  (:LNOT: debugging)      ; ensure debugging never enabled

        ; ---------------------------------------------------------------------

        GBLA    SysId
SysId   SETA    0		; system identity

        GBLA    Verno
Verno   SETA    &280            ; base Acorn version taken

        ; ---------------------------------------------------------------------
        ; Now for the macros used by lower level files to do OS dependent stuff
        
        MACRO
        LoadFPRegPointer
        ; This should load "r12" with the FP workspace/register allocation
        ; for the active process
        LDR     r12,=ROOT_start
        LDR     r12,[r12,#ExecRoot_fparea]
        MEND

        MACRO
        ReloadFPRegPointer      $cc
        ; This should conditionally load "r12" with the FP workspace/register
        ; allocation for the active process
        LDR$cc  r12,=ROOT_start
        LDR$cc  r12,[r12,#ExecRoot_fparea]
        MEND

	; ---------------------------------------------------------------------
	; FP state description

                        ^       &00
fp_workspace            #       &00             ; base address of FP workspace
	[	{FALSE}
fp_spare		#	&20		; padding
	; We should NOT need this padding, but with-out it we end up with
	; a corrupted memory pool chain. Either the AllocMem uses part of the
	; memory it returns to you... or the FPE uses more than it claims to.
	|
	; This could have been caused because I had the stack size allocation
	; defined as "fp_stack # &0100", and the calculation below worked from
	; backwards from the "fp_stack" offset.
	]	; EOF {boolean}
fp_stack_bottom		#	&0100		; 256bytes of stack space
fp_stack                #       &00		; top of the FPE stack
fp_regs                 #       &00             ; FP register set
fp_reg0                 #       (4 * &04)	; fp0
fp_reg1                 #       (4 * &04)	; fp1
fp_reg2                 #       (4 * &04)	; fp2
fp_reg3                 #       (4 * &04)	; fp3
fp_reg4                 #       (4 * &04)	; fp4
fp_reg5                 #       (4 * &04)	; fp5
fp_reg6                 #       (4 * &04)	; fp6
fp_reg7                 #       (4 * &04)	; fp7
fp_status               #       &04		; FP status register
fp_workspace_end        #       &00

fp_stack_size           *       (fp_stack - fp_workspace)
fp_workspace_size       *       (fp_workspace_end - fp_workspace)

        ; ---------------------------------------------------------------------
        ; Return macro for small routines which have not corrupted r0-r6
        ; (nor r13s)

        MACRO
        exit7to15
        B       |finish7to15|           ; we exit immediately
        LTORG
        MEND

        ; ---------------------------------------------------------------------
        ; Return macro for routines which have corrupted r0-r6,
        ; (not arm or lr) but have left "arm" pointing at the 16 word space
        ; storing the register copies

        MACRO
        exit0to15
        B       |finish0to15|           ; we exit immediately
        MEND

        ; ---------------------------------------------------------------------
        ; Return macro for routines that may have written back to arm reg.
        ; copies, so these copies all have to be loaded back into user arm
        ; regs.
        ; Note that it is not worth attempting to re-enter from this exit as
        ; we would have to do a lot of unstacking of user arm regs in case we
        ; later leave by exit7to15.

        MACRO
        exit0to15_wb
        B       |finish0to15_wb|
        LTORG
        MEND


        ; ---------------------------------------------------------------------
        ; -- Helios Floating Point ROM item -----------------------------------
        ; ---------------------------------------------------------------------

	ROMITEM	"helios/lib/FPEmulator",FPinit,Verno,MakeTime,defaultITEMaccess

        ; ---------------------------------------------------------------------
        ; -- System Floating Point initialisation -----------------------------
        ; ---------------------------------------------------------------------
        ; We need to attach ourselves to the "undefined instruction" vector
FPinit
        ; in:   r14 = return address
        ;       no other conditions

        STMFD   sp!,{r0,r1,r2,link}

        LDR     r2,=fp_old_vector
        LDR     r0,[r2,#&00]            ; check if already initialised
        TEQ     r0,#&00000000
        LDMNEFD sp!,{r0,r1,r2,pc}^      ; YES... then exit quickly

        ; attach "fp_decode" to the "vec_undef_instruction" vector
        MOV     r0,#vec_undef_instruction
        ADRL    r1,fp_decode
        SWI     exec_VectorPatch
        STR     r1,[r2,#&00]            ; remembering the old contents

        LDMFD   sp!,{r0,r1,r2,pc}^

        ; ---------------------------------------------------------------------

very_ill_instruction
        ; We will be re-entered here if the FPE does NOT recognise the
        ; instruction. We should exit through the undefined instruction
        ; handler that existed before we did.

        ADD     r9,sp,#(16 * &04)               ; original SVC mode "sp"
        MOV     r10,link                        ; preserve USR mode PC

        LDR     r11,=fp_old_vector
        LDR     r11,[r11,#&00]                  ; old vector contents
        ORR     r11,r11,#(Ibit :OR: SVCmode)    ; SVC mode, IRQs disabled
        AND     r7,r10,#HIflags                 ; USR mode CC flags
        ORR     r11,r11,r7                      ; r11 = handler address

        SUB     r8,r9,#(3 * &04)                ; entry sp,link and PC
        STMIA   r8,{r9,r10,r11}                 ; replace entry values
        LDMDB   r9,{r7-pc}^                     ; and onto the old handler

        ; ---------------------------------------------------------------------

finish7to15
        ; Return when r0-r6 preserved.

	[	(fpdebug)
	; we are going to restore r7-r12 here
	MOV	r12,r0
	MOV	r11,r14
	ADR	r0,fintxt1
	SWI	exec_Output
	MOV	r0,r11
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	ADR	r0,fintxtA
	SWI	exec_Output
	MOV	r0,sp
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	MOV	r14,r11
	MOV	r0,r12
	B	finovr1
fintxt1	=	"FPE: finish7to15: r14 = &",&00
fintxtA	=	"FPE: current sp (r13) = &",&00
	ALIGN
finovr1
	]	; EOF (fpdebug)

        ADD     sp,sp,#(7 * 4)  ; ditch USR r0 to r6
        LDMIA   sp!,{r7-r12}    ; load USR r7 to r12
        ADD     sp,sp,#(3 * 4)  ; remove r13,r14 and pc copies from stack
        MOVS    pc,link         ; and return

        ; ---------------------------------------------------------------------

finish0to15
        ; Return when r0-r6 corrupted.

	[	(fpdebug)
	; we are going to restore r0-r12 here
	MOV	r11,r14
	ADR	r0,fintxt2
	SWI	exec_Output
	MOV	r0,r11
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	ADR	r0,fintxtA
	SWI	exec_Output
	MOV	r0,sp
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	MOV	r14,r11
	B	finovr2
fintxt2	=	"FPE: finish0to15: r14 = &",&00
	ALIGN
finovr2
	]	; EOF (fpdebug)

        LDMIA   sp!,{r0-r12}    ; recover USR mode registers
        ADD     sp,sp,#(3 * 4)  ; remove r13,r14 and pc copies from stack
        MOVS    pc,link         ; and return

        ; ---------------------------------------------------------------------

finish0to15_wb
        ; Return when FPE routines have written back to the stored register
        ; set.

	[	(fpdebug)
	; we are going to restore USR r0-r14 (and the PC from the stack)
	MOV	r11,r14			; return address

	ADR	r0,fintxt3
	SWI	exec_Output
	MOV	r0,r11			; return address
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	ADR	r0,fintxtA
	SWI	exec_Output
	MOV	r0,sp			; stack-pointer
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	ADR	r0,fintxt4
	SWI	exec_Output
	LDR	r0,[sp,#(15 * 4)]	; return address stored on stack
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	ADR	r0,fintxt5
	SWI	exec_Output
	LDR	r0,[sp,#(13 * 4)]	; entry USR stack-pointer
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	MOV	r14,r11			; restore return address
	B	finovr3
fintxt3	=	"FPE: finish0to15_wb: r14 = &",&00
fintxt4	=	"FPE: finish0to15_wb: sp[15] = &",&00
fintxt5	=	"FPE: finish0to15_wb: sp[13] = &",&00
	ALIGN
finovr3
	]	; EOF (fpdebug)

	; We should be able restore the USR sp (even though this is above ours)
	; since we any IRQs that occur at this point will use the SVC r13
	; which should still be a valid stack-pointer.

        LDMIA   sp,{r0-r14}^	; load USR mode registers (no write-back)
	NOP			; wait for register re-mapping
	ADD	sp,sp,#(15 * 4)	; bump up the stack-pointer
	[	{TRUE}	; don't perform write-back to the SVC r13
	LDMIA	sp,{pc}^	; and load PC (plus PSR) to exit
	|
        LDMIA   sp!,{pc}^       ; and load PC (plus PSR) to exit
	]	; EOF {TRUE}

        ; ---------------------------------------------------------------------

fp_exception
        ; Entered when one of the following floating point exceptions has
        ; occured:
        ;       Inexact operation
        ;       Underflow
        ;       Divide by zero
        ;       Overflow
        ;       Invalid operation
        ;       Not enough stack (for FP register set and workspace)
        ;
        ; in:  r0 = exception mask
        ;      sp = SVC mode FD stack containing USR mode r0 - r15
        ;      link = callers return address (USR mode process)

	[	(fpdebug)
	STMFD	sp!,{r0,r14}
	ADR	r0,errtxt1
	SWI	exec_Output
	LDMFD	sp,{r0}	
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	LDMFD	sp!,{r0,r14}
	B	errovr1
errtxt1	=	"FPE exception: r0 = &",&00
	ALIGN
errovr1
	]	; EOF (fpdebug)

        ADD     sp,sp,#&04      ; step over the stacked "r0"
        LDMIA   sp!,{r1-r12}    ; and recover most of the USR mode registers
        ADD     sp,sp,#(3 * 4)  ; dump USR sp,link and PC from SVC stack
        ; sp   = empty SVC mode FD stack (as far as this level)
        ; link = callers return address (FP instruction + 4)
        ; r1-r12 = USR mode non-mapped registers
        ; r0 = FP exception mask (USR mode r0 corrupted)

        !       0,"TODO: USR mode r0 and FPE return address corrupted here"

        TEQP    pc,#&00000000           ; return to USR mode

        ; ---------------------------------------------------------------------
        ; -- Floating Point Error generation ----------------------------------
        ; ---------------------------------------------------------------------

        TST     r0,#stk_exception
        BNE     abort_nostack

        TST     r0,#ivo_exception
        BNE     abort_invop

        TST     r0,#ofl_exception
        BNE     abort_overflow

        TST     r0,#dvz_exception
        BNE     abort_dvz

        TST     r0,#ufl_exception
        BNE     abort_underflow

        ; therefore it must be "inexact operation"
abort_inexact
        ERROR   sys_inexact,"FP inexact operation"

abort_invop
        ERROR   sys_invop,"FP invalid operation"

abort_overflow
        ERROR   sys_overflow,"FP overflow"

abort_dvz
        ERROR   sys_dvz,"FP division by zero"

abort_underflow
        ERROR   sys_underflow,"FP underflow"

abort_nostack
	[	(fpspace)
	; NOT really a SIGSTAK, but it will do for the moment
	ERROR	sys_nostack,"Not enough memory for FP state"
	|
        ERROR   sys_nostack,"Not enough stack for FP state"
	]

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Now for the code jumped to in supervisor mode when illegal
        ; instruction tripped over. This is critical path code, of course...
        ;
	; in:	r13 = undefined
	;	r14 = user PC + 4 (including PSR state)
|fp_bot|                                ; base of our code
        ; Align this code on a boundary of 8 for best MEMC system performance
|fp_decode|
	[	(fpspace)
	; If this process does not have a FP workspace/register allocation
	; then claim the space from the task pool using the kernel routines.
	|
        ; If this process does not have a FP workspace/register allocation
        ; then claim the space from the "sl" register and initialise the FPSR.
	]
	; FPEmulator calls should only occur from USR mode (otherwise SVC r14
	; is corrupted). We RELY on this feature here.
	LDR	sp,=entryword		; temporary word in system workspace
	STMIA	sp,{usr_sp}^		; store USR mode r13 at temp. word
	NOP				; wait for registers to re-map
	LDR	sp,[sp,#&00]		; and load USR r13 into SVC r13
	; we have a FD SVC stack, so continue...

	ASSERT	(sp = arm)		; ensure naming convention valid
	[	{TRUE}
	; Since we are now using the USR r13/sp as our own sp AND floating
	; point operations can write-back to the stack-pointer (and under-neath
	; the stack-pointer) in "atomic" operations we need to drop the stack
	; we use by the maximum amount that can be written by a floating
	; point instruction. eg. the following instruction type is used during
	; the PCS function entry:
	;	STFE	f7,[sp,#-12]!
	; We should not need to worry about large offsets being used under the
	; "sp", since the PCS (and current Helios system) states that after
	; any instruction a valid stack should exist under "sp" (where no
	; user required data is stored). However, this could lead to problems
	; with code that performs instructions like the following:
	;	STFE	f7,[sp,#-4096]!
	; but for the moment I assume that nobody would be silly enough to
	; generate such an instruction, since it may be larger than "sl_offset"
	; away from the current stack-pointer.
	;
	; We only drop the stack by the amount that can be written in a single
	; floating point register/memory transfer. This code assumes that the
	; user will never use a floating pointer register/memory transfer
	; instruction to allocate stack space above the value transferred.
	SUB	sp,sp,#((16 * 4) + (4 * 4))
	|
        SUB     sp,sp,#(16 * 4)         ; STMDB (^ doesn't write back properly)
	]	; EOF {boolean}
        STMIA   sp,{r0-r14}^            ; save USR regs on supervisor stack
	NOP				; and wait for registers to re-map
        STR     r14,[sp,#(15 * 4)]      ; save lr in place to load in r15u
        AND     r10,r14,#&0C000000      ; entry IRQ and FIQ state
        TEQP    r10,#&03                ; preserve SVC mode

	[	(fpdebug)
	; Do not use r9 or r10
	ASSERT	(r9 = dp)		; check register naming
	ASSERT	(r10 = sl)		; check register naming
	; r7, r8 and r12 are available
	MOV	r12,r0			; preserve r0
	MOV	r8,r14			; preserve lk

	ADR	r0,fptxt1a
	SWI	exec_Output
	MOV	r0,r8			; entry r14
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	ADR	r0,fptxt1b
	SWI	exec_Output
	MOV	r0,sp			; current stack-pointer
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	ADR	r0,fptxt1g
	SWI	exec_Output
	LDR	r0,[sp,#(15 * 4)]	; stacked return address
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	ADR	r0,fptxt1h
	SWI	exec_Output
	LDR	r0,[sp,#(13 * 4)]	; stack USR mode stack-pointer
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	ADR	r0,fptxt1d
	SWI	exec_Output
	BIC	r7,r8,#&FC000003	; PC only (no PSR)
	SUB	r7,r7,#4		; take into account pre-fetch
	MOV	r0,r7
	SWI	exec_WriteHex8		; and write the address

	ADR	r0,fptxt1e
	SWI	exec_Output
	LDRT	r0,[r7],#0		; get instruction from USR space
	SWI	exec_WriteHex8

	ADR	r0,fptxt1f
	SWI	exec_Output
	LDR	r0,=ROOT_start
	SWI	exec_WriteHex8
	SWI	exec_NewLine

	MOV	r14,r8			; restore lk
	MOV	r0,r12			; restore r0
	B	fpovr1
fptxt1a	=	"FPE: entry r14 = &",&00
fptxt1b	=	"FPE: current sp (r13) = &",&00
fptxt1d	=	"FPE: address = &",&00
fptxt1e	=	" instruction = &",&00
fptxt1f	=	" ROOT_start = &",&00
fptxt1g	=	"FPE: sp[15] = &",&00
fptxt1h	=	"FPE: sp[13] = &",&00
	ALIGN
fpovr1
	; r7  - corrupted from entry
	; r8  - corrupted from entry
	; r12 - corrupted from entry
	]	; EOF (fpdebug)

        LoadFPRegPointer                ; point at stack and fp registers
        TEQ     r12,#&00000000          ; do we have a workspace allocation
        BNE     valid_workspace

	[	(fpspace)
	; We should claim "fp_workspace_size" bytes of memory.
	LABREF	_Task_			; Task *_Task_ ;
	IMPORT	AllocMem,EXCEPTION	; void *AllocMem(word size,Pool *pool)

	MOV	r0,#fp_workspace_size	; amount of memory we require
	; r0 = amount to malloc
	LDR	r1,[dp,#:MODOFF: _Task_]
	LDR	r1,[r1,#:OFFSET: _Task_]
	ADD	r1,r1,#Task_MemPool	; r1 = &(_Task_->MemPool) ;
	; r1 = memory pool descriptor
	; r9 = "dp" of process FP instruction appears in (from entry)
	; Calling AllocMem will corrupt a1, a2, a3, a4, ip and lk (PCS defined)
	; We only care about a1-a4 since ip and lk are always preserved.
	; We assume that AllocMem can be called in SVC mode.
	ASSERT	(r12 = ip)		; check register naming
	BL	AllocMem		; provided by Helios Kernel
	MOVS	r12,r0			; r12 = FP workspace base address

	[	(fpdebug)
	MOV	r8,r14			; preserve r14
	ADR	r0,fptxt2
	SWI	exec_Output
	MOV	r0,r12
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	MOV	r14,r8			; recover r14
	TEQ	r12,#&00000000		; ensure PSR state preserved
	B	fpovr2
fptxt2	=	"FPE: AllocMem returned &",&00
	ALIGN
fpovr2
	]	; EOF (fpdebug)

	LDMIA	sp,{r0,r1,r2,r3}	; recover in-case corrupted in AllocMem
        LDR     r14,[sp,#(15 * 4)]      ; recover lk corrupted over AllocMem
	MOVEQ	r0,#stk_exception	; failed to allocate memory
	BEQ	fp_exception		; stack contains entry state

	|	; middle (fpspace)

        ; "r9" and "r10" can be used as work registers at this point
        ; "r12" should be modified to contain the workspace base address

        ; At the moment the "sl" register contains an address "sl_offset"
        ; above the TRUE stack limit. We should increment this address
        ; by "fp_workspace_size" returning the TRUE stack limit
        ; as the base of the FP workspace, with "sl" pointing "sl_offset"
        ; bytes above the end of the FP workspace.

        LDR     r9,[sp,#(sl * &04)]     ; r9  = processes "sl"
        LDR     r10,[sp,#(sp * &04)]    ; r10 = processes "sp"
        SUB     r10,r10,#sl_offset      ; ensure process has some stack left
        SUB     r12,r9,#sl_offset       ; r12 = FP workspace base address
        ADD     r9,r9,#fp_workspace_size
        CMP     r9,r10                  ; check that we have not overflowed

        MOVCS   r0,#stk_exception       ; not enough stack for the FP workspace
        BCS     fp_exception            ; stack contains entry state
	]	; EOF (fpspace)

        ; remember the FP workspace pointer
        ADD     r12,r12,#fp_stack_size  ; "fparea" points at the registers
	[	(:LNOT: fpspace)
        STR     r9,[sp,#(sl * &04)]     ; update the processes "sl" register
	]	; EOF (:LNOT: fpspace)
        LDR     r10,=ROOT_start
        STR     r12,[r10,#ExecRoot_fparea]
	[	(fpspace)		; -- NOTE -- NOTE -- NOTE -- NOTE --
	; When we wish to release the FP state memory, we should recover the
	; address from "ExecRoot_fparea" and subtract "fp_stack_size" before
	; passing the address to "FreeMem".
	]	; EOF (fpspace)		; -- NOTE -- NOTE -- NOTE -- NOTE --

        [       {TRUE}
        ; Do not zero the registers before using
        |
        ; This code is flawed... we use "r5" and "r6" as work registers,
        ; whereas the code after this point assumes that only r7..r15 are
        ; used during instruction decoding... and uses this fact on exit
        ; to only re-load a subset of the user registers...

        ; zero the FP register set just allocated
        MOV     r5,r12                  ; FP register set base address
        MOV     r6,#&08                 ; number of FP registers
        ADRL    r7,fp_zeroes
        LDMIA   r7,{r8,r9,r10,r11}      ; load zeroes
fp_zero_regs
        STMIA   r5!,{r8,r9,r10,r11}     ; zero this FP register
        SUBS    r6,r6,#&01
        BNE     fp_zero_regs
        ]

        ; initialise this FP state
        MOV     r0,#&00070000           ; set default bits in FPSR
        STR     r0,[r12,#(8 * 4 * 4)]
valid_workspace
        ; r12 = FP workspace pointer
	[	(fpdebug)
	MOV	r8,r0			; preserve r0
	MOV	r7,r14			; preserve r14
	ADR	r0,fptxt3
	SWI	exec_Output
	MOV	r0,r12
	SWI	exec_WriteHex8
	SWI	exec_NewLine
	MOV	r14,r7			; recover r14
	MOV	r0,r8			; recover r0
	B	fpovr3
fptxt3	=	"FPE: workspace pointer = &",&00
	ALIGN
fpovr3
	]	; EOF (fpdebug)

        ; Check that this is an instruction for this emulator.
        BIC     r10,r14,#&FC000003      ; turn into just PC
        SUB     r10,r10,#4              ; no pre-indexded LDRT
        LDRT    r9,[r10],#8             ; get instruction from user space

        GET     fpemid.s                ; floating point instruction decode
        ; this file actually emulates the instruction.
        ; If it doesn't recognise the opcode then it will jump to
        ; "very_ill_instruction".
        ; Otherwise, it'll return after doing the operation with an exit macro

        ; ---------------------------------------------------------------------

	[	{FALSE}	; required if we zero registers during initialisation
        ; some zeroes that can be loaded quickly (used to initialise memory)
fp_zeroes
        &       &00000000
        &       &00000000
        &       &00000000
        &       &00000000       
	]	; EOF {boolean}

        ; ---------------------------------------------------------------------

	[	{FALSE}	; now performed in-line
        ; Come here in supervisor mode to initialise workspace.
        ; No input or output information (no registers altered).
|fp_initialise|
        STMDB   sp!,{r0,r12,r14}        ; preserve work reg., fp pointer, lr

        ReloadFPRegPointer              ; nested fp space in r12
        MOV     r0,#&00070000           ; and set default bits in FPSR
        STR     r0,[r12,#(8 * 4 * 4)]

        LDMIA   sp!,{r0,r12,pc}^        ; then return
	]	; EOF {boolean}

        ; ---------------------------------------------------------------------
        LNK     $fpendfile
