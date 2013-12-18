        SUBT ARM CPU hardware description                               > arm/s
        ;    Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ;
        ; started:      890418  JGSmith
        ; updated:      901122  JGSmith even more detail added
        ;               890720  JGSmith Added stack-limit description
        ;               890630  JGSmith Added more detail
        ;
        ; This file describes the ARM processor in software terms.
        ;
        ; ---------------------------------------------------------------------

old_opt SETA    {OPT}
        OPT     (opt_off)

        ; ---------------------------------------------------------------------

        !       0,"Including arm.s"
        GBLL    arm_s
arm_s   SETL    {TRUE}          ; ARM processor constants have been defined

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ARM registers

r0              RN      0       ; never banked
r1              RN      1       ; never banked
r2              RN      2       ; never banked
r3              RN      3       ; never banked
r4              RN      4       ; never banked
r5              RN      5       ; never banked
r6              RN      6       ; never banked
r7              RN      7       ; never banked
r8              RN      8       ; only FIQ
r9              RN      9       ; only FIQ
r10             RN      10      ; only FIQ
r11             RN      11      ; only FIQ
r12             RN      12      ; only FIQ
r13             RN      13      ; FIQ, IRQ and SVC
r14             RN      14      ; FIQ, IRQ and SVC
r15             RN      15      ; never banked

        ; PCS register definitions
        ; Note: "a1-v6" are not defined here so that we can assemble the
        ;       Acorn FP Emulator source directly with the default header
        ;       files. The file "PCSregs.s" contains the r0..r8 PCS register
        ;       definitions.

dp              RN      r9      ; module table pointer

sb              RN      r10     ; stack-base? this is a bit misleading
sl              RN      r10     ; stack-limit is better

fp              RN      r11     ; frame-pointer
ip              RN      r12     ; <temporary>
sp              RN      r13     ; stack-pointer

lr              RN      r14     ; link-register
lk              RN      lr      ; link-register
link            RN      lr      ; link-register

pc              RN      r15     ; program-counter

; banked registers (provided for assembler clarity)

usr_r13         RN      r13
usr_r14         RN      r14
usr_sp          RN      usr_r13
usr_link        RN      usr_r14

irq_r13         RN      r13
irq_r14         RN      r14
irq_sp          RN      irq_r13
irq_link        RN      irq_r14

fiq_r8          RN      r8
fiq_r9          RN      r9
fiq_r10         RN      r10
fiq_r11         RN      r11
fiq_r12         RN      r12
fiq_r13         RN      r13
fiq_r14         RN      r14
fiq_sp          RN      fiq_r13
fiq_link        RN      fiq_r14

svc_r13         RN      r13
svc_r14         RN      r14
svc_sp          RN      svc_r13
svc_link        RN      svc_r14

r8_fiq		RN	r8
r9_fiq		RN	r9
r10_fiq         RN      r10
r11_fiq         RN      r11
r12_fiq         RN      r12
r13_fiq         RN      r13
r14_fiq         RN      r14

r13_irq         RN      r13
r14_irq         RN      r14

r13_svc         RN      r13
r14_svc         RN      r14

wp              RN      r12     ; standard "workspace pointer" register
sp_irq          RN      r13
sp_svc          RN      r13
stack           RN      r13
arm             RN      r13     ; points at base of store of user ARM registers
lr_usr          RN      lr
lr_irq          RN      lr
lr_fiq          RN      lr
lr_svc          RN      lr
psr             RN      pc

R0              RN      r0
R1              RN      r1
R2              RN      r2
R3              RN      r3
R4              RN      r4
R5              RN      r5
R6              RN      r6
R7              RN      r7
R8              RN      r8
R9              RN      r9
R10             RN      r10
R11             RN      r11
R12             RN      r12
R13             RN      r13
R14             RN      r14
R15             RN      r15

WP              RN      r12
STACK           RN      r13
LR              RN      r14
LINK            RN      r14
PC              RN      r15
PSR             RN      r15

Error           RN      r0
WsPtr           RN      r12
Stack           RN      r13

f0              FN      0
f1              FN      1
f2              FN      2
f3              FN      3
f4              FN      4
f5              FN      5
f6              FN      6
f7              FN      7

F0              FN      f0
F1              FN      f1
F2              FN      f2
F3              FN      f3
F4              FN      f4
F5              FN      f5
F6              FN      f6
F7              FN      f7

cp0		CP	0
cp1		CP	1
cp2		CP	2
cp3		CP	3
cp4		CP	4
cp5		CP	5
cp6		CP	6
cp7		CP	7
cp8		CP	8
cp9		CP	9
cp10		CP	10
cp11		CP	11
cp12		CP	12
cp13		CP	13
cp14		CP	14
cp15		CP	15

fp_coproc	CP	cp1	; Floating Point co-processor

c0		CN	0
c1		CN	1
c2		CN	2
c3		CN	3
c4		CN	4
c5		CN	5
c6		CN	6
c7		CN	7

        ; ---------------------------------------------------------------------
        ; processor flags

Nbit            *       (1 :SHL: 31)    ; miNus
Zbit            *       (1 :SHL: 30)    ; Zero
Cbit            *       (1 :SHL: 29)    ; Carry
Vbit            *       (1 :SHL: 28)    ; oVerflow
Ibit            *       (1 :SHL: 27)    ; processor IRQ enable/disable
Fbit            *       (1 :SHL: 26)    ; processor FIQ enable/disable

USRmode         *       2_00            ; standard USER/SYSTEM process mode
FIQmode         *       2_01            ; fast interrupt handler mode
IRQmode         *       2_10            ; standard interrupt handler mode
SVCmode         *       2_11            ; system priviledge mode

INTflags        *       (Ibit :OR: Fbit)                        ; interrupts
Aflags          *       (Nbit :OR: Zbit :OR: Cbit :OR: Vbit)    ; arithmetic
HIflags         *       (Aflags :OR: INTflags)
PSRflags        *       (HIflags :OR: SVCmode)                  ; full PSR mask
MODEmask        *       (2_11)                                  ; MODE mask

        ; ---------------------------------------------------------------------
        ; hardware vectors

vec_reset                       *       &00000000
vec_undefined_instruction       *       &00000004
vec_SWI                         *       &00000008
vec_prefetch_abort              *       &0000000C
vec_data_abort                  *       &00000010
vec_address_exception           *       &00000014
vec_IRQ                         *       &00000018
vec_FIQ                         *       &0000001C

        ; -------------------------------------------------------------------
        ; ARM instruction set
        ; -------------------
        ;
        ; ---------------------------------------------------
        ; | Cond | 0001 | xBxx | Rn | Rd | xxxx | 1xx1 | Rm | Semaphore
        ; ---------------------------------------------------
        ; | Cond | 001 | ddddS | Rn | Rd |    Operand 2     | Data Processing
        ; ---------------------------------------------------
        ; | Cond |   000000AS  | Rd | Rn |  Rs  | 1001 | Rm | Multiply
        ; ---------------------------------------------------
        ; | Cond | 0001 |   xxxxxxxxxxxxxxxx  | 1xx1 | xxxx | Undefined
        ; ---------------------------------------------------
        ; | Cond |  01IPUBWL   | Rn | Rd |     offset       | Data Transfer
        ; ---------------------------------------------------
        ; | Cond | 011 |  xxxxxxxxxxxxxxxxxxxx   | 1 | xxxx | Undefined
        ; ---------------------------------------------------
        ; | Cond | 100PUSWL    | Rn |     register list     | Block Transfer
        ; ---------------------------------------------------
        ; | Cond | 101 | L |           offset               | Branch
        ; ---------------------------------------------------
        ; | Cond | 110PUNWL    | Rn | CRd | CP# |  offset   | Coproc Data Trans
        ; ---------------------------------------------------
        ; | Cond | 1110 | dddd | CRn | CRd | CP#|CP| 0 | CRm| Coproc Data Op
        ; ---------------------------------------------------
        ; | Cond | 1110 | dddL | CRn | Rd  | CP#|CP| 1 | CRm| Coproc Reg Trans
        ; ---------------------------------------------------
        ; | Cond | 1111 |           comment                 | SWI
        ; ---------------------------------------------------
        ;
        ; Note: The above map is NOT completely correct since the semaphore
        ; instruction lies within the Undefined instruction scope.
        ;
        ; ---------------------------------------------------------------------
        ; condition codes
        ; ---------------
cond_EQ         *       &00000000       ; Z
cond_NE         *       &10000000       ; Z
cond_CS         *       &20000000       ; C
cond_HS         *       &20000000       ; C synonym for CS
cond_CC         *       &30000000       ; C
cond_LO         *       &30000000       ; C synonym for CC
cond_MI         *       &40000000       ; N
cond_PL         *       &50000000       ; N
cond_VS         *       &60000000       ; V
cond_VC         *       &70000000       ; V
cond_HI         *       &80000000       ; CZ
cond_LS         *       &90000000       ; CZ
cond_GE         *       &A0000000       ; NV
cond_LT         *       &B0000000       ; NV
cond_GT         *       &C0000000       ; NVZ
cond_LE         *       &D0000000       ; NVZ
cond_AL         *       &E0000000       ; always
cond_NV         *       &F0000000       ; never

cond_mask       *       &F0000000
cond_shift      *       28

        ; major opcode groups
        ; -------------------
op_regxreg1     *       &00000000
op_regxreg2     *       &01000000
op_regximm1     *       &02000000
op_regximm2     *       &03000000
op_postimm      *       &04000000       ; load/store immediate post offset
op_preimm       *       &05000000       ; load/store immediate pre offset
op_postreg      *       &06000000       ; load/store register post offset
op_prereg       *       &07000000       ; load/store register pre offset
op_ldmstm1      *       &08000000
op_ldmstm2      *       &09000000
op_b            *       &0A000000
op_bl           *       &0B000000
op_cppost       *       &0C000000       ; co-processor data post transfer
op_cppre        *       &0D000000       ; co-processor data pre transfer
op_cpop         *       &0E000000       ; co-processor data op or reg trans
op_swi          *       &0F000000

op_mask         *       &0F000000       ; opcode type mask
op_shift        *       24              ; opcode type shift

        ; data-processing operations
        ; --------------------------
dp_mask         *       &01E00000
dp_shift        *       21
dp_and          *       &0              ; Rd  = Op1 AND Op2
dp_eor          *       &1              ; Rd  = Op1 EOR Op2
dp_sub          *       &2              ; Rd  = Op1  -  Op2
dp_rsb          *       &3              ; Rd  = Op2  -  Op1
dp_add          *       &4              ; Rd  = Op1  +  Op2
dp_adc          *       &5              ; Rd  = Op1  +  Op2 + C
dp_sbc          *       &6              ; Rd  = Op1  -  Op2 + C
dp_rsc          *       &7              ; Rd  = Op2  -  Op1 + C
dp_tst          *       &8              ; PSR = Op1 AND Op2
dp_teq          *       &9              ; PSR = Op1 EOR Op2
dp_cmp          *       &A              ; PSR = Op1  -  Op2
dp_cmn          *       &B              ; PSR = Op1  +  Op2
dp_orr          *       &C              ; Rd  = Op1 OR  Op2
dp_mov          *       &D              ; Rd  = Op2
dp_bic          *       &E              ; Rd  = Op1 AND NOT Op2
dp_mvn          *       &F              ; Rd  = NOT Op2

        ; branches
        ; --------
	; offset = ((destination_address - (instruction_address + 8)) >> 2)
offset_mask     *       &00FFFFFF

        ; SWI
        ; ---
	; The SWI value is a 24bit constant. The meaning of this value is
	; completely under software control.
SWI_constant    *       &00FFFFFF
SWI_insmask     *       (cond_mask :OR: &0F000000)

        ; load/store multiple operations (stack)
        ; --------------------------------------
op_LDMFA        *       &08100000
op_LDMEA        *       &09100000
op_LDMFD        *       &08900000
op_LDMED        *       &09900000

op_STMFA        *       &09800000
op_STMEA        *       &08800000
op_STMFD        *       &09000000
op_STMED        *       &08000000

        ; load/store multiple operations (non-stack)
        ; ------------------------------------------
op_LDMDA        *       &08100000
op_LDMDB        *       &09100000
op_LDMIA        *       &08900000
op_LDMIB        *       &09900000

op_STMIB        *       &09800000
op_STMIA        *       &08800000
op_STMDB        *       &09000000
op_STMDA        *       &08000000

        ; floating point operations
        ; -------------------------
f_single        *       &00000100
f_double        *       &00400100       ; ((1 << 22) | (1 << 8))

        ; opcodes for CPRT group
        ; ----------------------
f_FLTS          *       &00000110
f_FIXS          *       &00100110
f_TRNS          *       &00300110
f_FLTD          *       &00400110
f_FIXD          *       &00500110
f_TRND          *       &00700110
f_FPSWR         *       &00800110
f_RFPSW         *       &00900110
f_RTOF          *       &00A00110
f_FTOR          *       &00B00110
f_RTOF1         *       &00C00110
f_FTOR1         *       &00D00110
f_RTOF2         *       &00E00110
f_FTOR2         *       &00F00110

        ; opcodes for more floating point operations
        ; ------------------------------------------
f_regop         *       &00000000
f_constop       *       &00800000

f_CMF           *       &0010F030
f_CNF           *       &0030F030

f_ADF           *       &00000000
f_SUF           *       &00000020
f_RSF           *       &00000040
f_ASF           *       &00000060
f_MUF           *       &00100000
f_DIF           *       &00100020
f_RDF           *       &00100040
f_spare1        *       &00100060
f_CVT           *       &00200000
f_ABS           *       &00200020
f_SQT           *       &00200040
f_MVF           *       &00200060
f_MNF           *       &00300000
f_LOG           *       &00300020
f_LGN           *       &00300040
f_EXP           *       &00300060
f_POW           *       &00000080
f_RPW           *       &000000A0
f_spare2        *       &000000C0
f_spare3        *       &000000E0
f_spare4        *       &00100080
f_spare5        *       &001000A0
f_spare6        *       &001000C0
f_spare7        *       &001000E0
f_SIN           *       &00200080
f_COS           *       &002000A0
f_TAN           *       &002000C0
f_ASN           *       &002000E0
f_ACS           *       &00300080
f_ATN           *       &003000A0
f_spare8        *       &003000C0
f_spare9        *       &003000E0

        ; shift constructions
        ; -------------------
reg_shiftmask   *       &00000FF0       ; register shift mask
reg_shiftshift  *       4
imm_shiftmask   *       &00000F00       ; immediate value rotation
imm_shiftshift  *       7               ; generates (rotator * 2)
imm_valuemask   *       &000000FF       ; immediate value

shift_reg       *       (1 :SHL: 4)     ; immediate shift/register shift

simm_mask       *       &00000F80       ; immediate shift value mask
simm_shift      *       &00000060       ; immediate shift operation

        ; NOTE: register/register shifts have bit7 permanently unset since
        ;       bit7 set for register shifts encodes the MUL/MLA instructions

sreg_mask       *       &00000F00       ; register shift register mask
sreg_shift      *       &00000060       ; register shift operation

shift_lsl       *       &00000000       ; logical shift left
shift_lsr       *       &00000020       ; logical shift right
shift_asr       *       &00000040       ; arithmetic shift right
shift_ror       *       &00000060       ; rotate right

        ; NOTE: "ROR #0" encodes "RRX" (Rotate Right with eXtend)
        ;       "ASR #0" encodes "ASR #32"
        ;       "LSR #0" encodes "LSR #32"

        ; instruction flags
scc             *       (1 :SHL: 20)    ; Set Condition Codes
ab              *       (1 :SHL: 21)    ; Accumulate Bit
lsb             *       (1 :SHL: 20)    ; Load/Store Bit
wbb             *       (1 :SHL: 21)    ; Write Back Bit
bwb             *       (1 :SHL: 22)    ; Byte/Word Bit
udb             *       (1 :SHL: 23)    ; Up/Down Bit
ppi             *       (1 :SHL: 24)    ; Pre/Post Indexing bit
immoff          *       (1 :SHL: 25)    ; IMMediate OFFset bit
psrfu           *       (1 :SHL: 22)    ; PSR and Force User mode bit

        ; co-processor instructions
regtrans        *       (1 :SHL: 4)     ; co-proc register transfer
cpnum_mask      *       (&F :SHL: 8)    ; co-proc number
cpinfo_mask     *       (&7 :SHL: 5)    ; co-processor information
cpopcode_mask   *       (&F :SHL: 20)   ; co-processor opcode (data op)
cpopcode2_mask  *       (&7 :SHL: 21)   ; co-processor opcode (reg trans)
        ; co-proc data transfer flags same as normal instruction flags
cp_offset_mask  *       &000000FF
ls_offset_mask  *       &00000FFF       ; load store immediate offset

regmask         *       &0000FFFF       ; register bit mask in LDM/STM

        ; co-processor number = ((instruction & cpnum_mask) :SHR: 8)
        ; co-processor opcode = ((instruction & cpopcode_mask) :SHR: 20)
        ; co-processor info   = ((instruction & cpinfo_mask) :SHR: 5)

        ; Rn = ((instruction :SHR: 16) :AND: &F)
        ; Rd = ((instruction :SHR: 12) :AND: &F)
        ; Rs = ((instruction :SHR:  8) :AND: &F)
        ; Rm = ((instruction :SHR:  0) :AND: &F)
Rm_regshift     *       0
Rs_regshift     *       8
Rd_regshift     *       12
Rn_regshift     *       16
Rm_regmask      *       (&F :SHL: Rm_regshift)
Rs_regmask      *       (&F :SHL: Rs_regshift)
Rd_regmask      *       (&F :SHL: Rd_regshift)
Rn_regmask      *       (&F :SHL: Rn_regshift)

        ; ---------------------------------------------------------------------
        ; Software considerations:
        ;
        ; The stack-limit register "sl" is initialised to a value a fixed
        ; offset above the true stack-base "sb". This is to allow the stack
        ; overflow handler code a reasonable amount of workspace.
        ;
        ; This size of this chunk depends on the stack overflow handler code
        ; and will probably have to change with development. This chunk must
        ; also be able to cope with a complete process SaveState aswell as the
        ; stack overflow handler requirements.

sp_offset	*	&00000080		; process SaveState guaranteed
						; (should always be larger than
						;  the SaveState structure,
						;  but we should check at
						;  build-time to be sure).
	[	{TRUE}
	; This version if the compiler does NOT assume 256bytes of stack space
sl_offset       *       (&00000200 + sp_offset)	; stack extension code
	|
sl_offset       *       (&00000300 + sp_offset)	; stack extension code
	]

        ; NOTES: The current process scheduler uses the space beneath r13/sp
        ;        as a temporary storage area for the process SaveState. This
        ;        means that ALL processes that have interrupts enabled MUST
        ;        have a r13/sp register with at least "sp_offset" bytes
        ;        available beneath it. The code that performs the state save
        ;        does NOT check the stack-limit register, since it assumes
        ;        that a process SaveState will fit in the stack-overflow
        ;        workspace (This includes a process swap occuring during the
        ;        stack overflow handler).

	[	{TRUE}
	; Compiler updated by JGS : 910213
	|
	;	 The current Helios C compiler (ncc349 901024) assumes a
	;	 sl_offset of at least 256bytes for normal stack allocations
	;	 (ie. it will not check for overflow for temporary stack
	;	  allocations of 256bytes or less). This must be taken into
	;	 account along with the worst case procedure entry of 160bytes
	;	 used for register stacking.
	]

        ; ---------------------------------------------------------------------
	; standard C Signal codes (see "include/signal.h")

SIGZERO	*	0	; no signal	
SIGABRT	*	1	; abort
SIGFPE	*	2	; floating point (arithmetic) exception
SIGILL	*	3	; illegal (undefined) instruction
SIGINT	*	4	; interrupt (attention) request
SIGSEGV	*	5	; segmentation fault (bad memory access)
SIGTERM	*	6	; termination request
SIGSTAK	*	7	; stack overflow
SIGALRM	*	8	; alarm/timeout signal
SIGHUP	*	9	; hangup
SIGPIPE	*	10	; pipe signal
SIGQUIT	*	11	; interactive termination (quit signal)
SIGTRAP	*	12	; trace trap (branch through zero on ARM)
SIGUSR1	*	13	; user defined signal 1
SIGUSR2	*	14	; user defined signal 2
SIGCHLD	*	15	; child termination
SIGURG	*	16	; urgent data available
SIGCONT	*	17	; continue
SIGSTOP	*	18	; stop
SIGTSTP	*	19	; interactive stop
SIGTTIN	*	20	; background read
SIGTTOU	*	21	; background write
SIGWINCH *	22	; window changed
SIGSIB	*	23	; sibling crashed
SIGKILL	*	31	; termination

        ; ---------------------------------------------------------------------
        ; Floating Point Emulator exception values:

nan_exception   *       (1 :SHL: 0)     ; nan event
ivo_exception   *       (1 :SHL: 0)     ; invalid operator
dvz_exception   *       (1 :SHL: 1)     ; division by zero
ofl_exception   *       (1 :SHL: 2)     ; overflow
ufl_exception   *       (1 :SHL: 3)     ; underflow
inx_exception   *       (1 :SHL: 4)     ; inexact operation
        ; Added by JGSmith to support Helios version of FPE system
stk_exception   *       (1 :SHL: 5)     ; not enough stack for FP state 

        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF arm/s
