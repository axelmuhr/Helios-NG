; -> fpecprt/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2cprt        27/11/86 Martin Clemoes
;
;       Description:    This file contains all the cprt operation routines.
;                       It is GETted by the file FPE2mid. It therefore
;                       implements the FLT, FIX, WFS and RFS operations.
;
;******************************************************************************
;
;       Routines to perform FIX - floating point to integer conversion
;

mto_arm

; First set up pointers to copy of arm reg. to be written to

;        savebaseinr11
        AND     r10, r9, #&0000f000     ;[arm, r10, LSR #10] points at reg.

; Now test for a constant being transferred - an odd thing to do, but allowed

        TST     r9, #&00000008          ;constant?
        BNE     constant_fix            ;yes, so deal with it

; Next get contents of fp register Rm in r0-r3

        AND     r5, r9, #&00000007      ;get bits containing reg. no.
        ADD     r5, r12, r5, LSL #4     ;hence point at reg.
        LDMIA   r5, {r0-r3}             ;and load it up

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U

; Now check for NaN, infinity, zero or denormalised

        MOVS    r4, r3, LSL #17         ;is exponent zero?
        ADDNES  r4, r4, #&00020000      ;or maximum value?
        BEQ     funny_fix               ;yes, so must be one of funnies...

; Now check for underflow or overflow

        BPL     pr_uflow_fix            ;if exponent+1 < &4000 underflow prob.
        CMP     r4, #&003e0000          ;if exponent+1 >= &401f
        AND     r4, r4, #&00ff0000      ;useful later
        BMI     pr_oflow_fix            ;then overflow probably
no_over

; In range, so truncate according to rounding rules

        MOV     r4, r4, LSR #17         ;r4 = number of bits left - 1

        ANDS    r9, r9, #&00000060      ;rounding mode bits
        BNE     n_default_r             ;not default...

; Default IEEE rounding (to nearest, mid-way to even bit pattern)

        ADD     r6, r4, #2              ;r6 = number of bits left + 1
        ORRS    r2, r2, r1, LSL r6      ;all bits except ms bit lost zero?
        MOV     r2, #&40000000
        AND     r2, r1, r2, LSR r4      ;get ms bit lost
        ANDEQ   r2, r2, r1, LSR #1      ;if so, round to even (IEEE)

; Now chop r1 by shifting

        RSB     r4, r4, #31             ;calculate shift
        MOV     r1, r1, LSR r4          ;and perform it

; And add 1 if rounding indicates it

        TEQ     r2, #0
        ADDNE   r1, r1, #1

; And return 2's complement integer (ie. install sign)

        TST     r0, #&80000000          ;negative?
        RSBNE   r1, r1, #0              ;if so, negate

        EORS    r2, r1, r0              ;has rounding overflowed +ve into -ve?
        BMI     overflow_fix            ;yes, so overflow...

        STR     r1, [arm, r10, LSR #10] ;save value to arm reg. copy
        exit0to15_wb                    ;and go home...

n_default_r

; First test to see if round to zero, and if so, do it

        CMP     r9, #&00000060          ;round to zero?
        BNE     infinity_r              ;no, so round to + or - infinity

truncate

; Now chop r1 by shifting

        RSB     r4, r4, #31             ;calculate shift
        MOV     r1, r1, LSR r4          ;and perform it

; And return 2's complement integer (ie. install sign)

        TST     r0, #&80000000          ;negative?
        RSBNE   r1, r1, #0              ;if so, negate

        STR     r1, [arm, r10, LSR #10] ;save value to arm reg. copy
        exit0to15_wb                    ;and go home...

infinity_r

; Round to + or - infinity

        EORS    r7, r0, r9, LSL #26     ;+ and +inf or - and -inf?
        BPL     truncate                ;no, so round towards zero

        ADD     r6, r4, #1              ;r6 = no. bits left
        ORRS    r2, r2, r1, LSL r6      ;hence are all bits lost zero?
        BEQ     truncate                ;yes, so precise number

; Now chop r1 by shifting

        RSB     r4, r4, #31             ;calculate shift
        MOV     r1, r1, LSR r4          ;and perform it
        ADD     r1, r1, #1              ;then do rounding

; And return 2's complement integer (ie. install sign)

        TST     r0, #&80000000          ;negative?
        RSBNE   r1, r1, #0              ;if so, negate

        EORS    r2, r1, r0              ;has rounding overflowed +ve into -ve?
        BMI     overflow_fix            ;yes, so overflow...

        STR     r1, [arm, r10, LSR #10] ;save value to arm reg. copy
        exit0to15_wb                    ;and go home...

funny_fix

; Deal with NaN, infinity, denormalised number or zero (only meaningful one)

        TST     r3, #&ff                ;NaN or infinity?
        BNE     bonk_fix                ;yes, so bonk process...

        ORRS    r1, r2, r1              ;is this zero?
        BNE     denorm_fix              ;no - denormalised, so underflow

        STR     r1, [arm, r10, LSR #10] ;store zero in arm reg. copy
        exit0to15_wb                    ;and go home

bonk_fix

; Deal with NaN or infinity - leaves arm regs alone and causes exception as ap.

        ORRS    r1, r2, r1              ;infinity?
        BEQ     overflow_fix            ;yes, so overflow...

; We have a NaN - great!

        ADR     r7, u_f_ret             ;return to a handy exit0to15 if no exn.
        MOV     r4, #NAN_E_MASK         ;register event
        B       check_exep

denorm_fix

; Denormalised no. - underflow unless mantissa rounded up to 1

        EOR     r7, r9, r9, LSR #1      ;is rounding mode 01 or 10?
        TST     r7, #&00000020
        BEQ     underflow_fix           ;no, so underflow
        B       inf_r                   ;yes, so test for rounding up

pr_uflow_fix

; We have probably underflowed, but may round up to 1 - depending on mode

        ANDS    r9, r9, #&00000060      ;rounding mode bits
        BEQ     nearest                 ;round to nearest, so is > 0.5?

        CMP     r9, #&00000060          ;round to zero?
        BEQ     underflow_fix           ;yes, so underflow...

inf_r
        EORS    r7, r0, r9, LSL #26     ;+ and +inf or - and -inf?
        BPL     underflow_fix           ;no, so underflow
one

; We are rounding up to one...

        MOV     r1, #1
        TST     r0, #&80000000
        RSBNE   r1, r1, #0              ;include sign
        STR     r1, [arm, r10, LSR #10] ;and return value
        exit0to15_wb

nearest
        ADDS    r7, r4, #&00020000      ;is mantissa only just shifted away?
        BPL     underflow_fix           ;no, so underflow

        CMP     r1, #&80000000          ;is value > 0.5?
        CMPEQ   r2, #0
        BHI     one                     ;yes, so return 1

underflow_fix

; We have underflowed - store 0 in arm reg. and cause exception if enabled

        MOV     r1, #0                  ;store 0 in arm reg.
        STR     r1, [arm, r10, LSR #10]

        B       u_f_ret                 ;return to a handy exit0to15 if no exn.

;        MOV     r4, #UFL_E_MASK         ;register event
;        B       check_exep

pr_oflow_fix

; We have probably overflowed, but may have maximum negative number - check

        CMP     r4, #&003e0000          ;was exponent that of max. neg. no.?
        CMPEQ   r1, #&80000000          ;was mantissa right if so?
        AND     r0, r0, #&80000000
        CMPEQ   r0, #&80000000          ;was sign negative if so?
        BNE     overflow_fix            ;no, so overflow

        CMP     r2, #0                  ;is any rounding needed?
        BEQ     no_over                 ;no, so reprieve from overflow...

        ANDS    r9, r9, #&00000060      ;get rounding mode bits
        BEQ     def_of                  ;default rounding mode
        CMP     r9, #&00000060          ;round to zero?
        BEQ     no_over                 ;yes, so reprieve from overflow...

; We have rounding to + or - infinity - may overflow or be OK

        EORS    r7, r0, r9, LSL #26     ;+ and +inf or - and - inf?
        BPL     no_over                 ;no, so reprieve from overflow...
        B       overflow_fix

def_of

; We have default rounding - overflow if r2 > &80000000

        CMP     r2, #&80000000
        BLS     no_over

overflow_fix

; We have overflowed - leave arm regs alone and cause exception in enabled

        ADR     r7, u_f_ret             ;return to a handy exit0to15 if no exn.
        MOV     r4, #IVO_E_MASK         ;register event
        B       check_exep

constant_fix

; We are asked to put constant in arm reg. - optimise by doing directly

        AND     r1, r9, #7              ;just part saying which constant
        CMP     r1, #6                  ;is it one of trickier constants?
        BGE     large_const             ;yes, so deal with it elsewhere

        STR     r1, [arm, r10, LSR #10] ;otherwise save value to arm copy
u_f_ret
        exit0to15_wb                    ;and go home...

large_const

; 7 means 10 - easy - but 6 means 0.5 - 0 (underflow) or 1 (OK) depending on
; rounding mode

        ADD     r1, r1, #3              ;convert 7 to 10

        CMP     r1, #10                 ;but is it 0.5?
        MOVNE   r1, #1                  ;yes - let's be optimistic for now
        AND     r9, r9, #&00000060      ;get rounding mode
        CMPNE   r9, #&00000020          ;is it round to + infinity?
        BNE     underflow_fix           ;no, so underflow

        STR     r1, [arm, r10, LSR #10] ;save value to arm reg. copy
        exit0to15_wb                    ;and go home...


;******************************************************************************
;
;       Routines to perform FLT - integer to floating point conversion
;

mfrom_arm

; If bits ef = 01, this indicates double dest. size, so no rounding needed

        TST     r9, #&00000080          ;does bit f = 1?
        BNE     mfa_noround             ;yes, so no need to round to S prec.

; We will make a U type fp. no. of single precision (not type)

;        savebaseinr11                   ;r11 points at saved copy of user arm
        AND     r7, r9, #&0000f000      ;arm reg. no.
        LDR     r8, [arm, r7, LSR #10]  ;hence get integer in question in r8

; We now have integer in r8, so make U type fp no. in r7-r10 from it

        AND     r11, r9, #&00070000     ;fp. reg. no.
        ADD     r11, r12, r11, LSR #12  ;hence its address in r11

        MOVS    r7, r8                  ;sign bit in r7
        RSBMI   r8, r8, #0              ;mantissa in r8
        MOV     r10, #127+31            ;exponent in r10
        ADD     r10, r10, #&3f80

; Now we must normalise

        MOVEQ   r10, #31                ;if zero mantissa, zero exponent

        MOVS    r12, r8, LSR #16        ;can we shift left by 16?
        MOVEQ   r8, r8, LSL #16         ;yes, so do so
        SUBEQ   r10, r10, #16           ;and reduce exponent

        MOVS    r12, r8, LSR #24        ;can we shift left by 8?
        MOVEQ   r8, r8, LSL #8          ;yes, so do so
        SUBEQ   r10, r10, #8            ;and reduce exponent

        MOVS    r12, r8, LSR #28        ;can we shift left by 4?
        MOVEQ   r8, r8, LSL #4          ;yes, so do so
        SUBEQ   r10, r10, #4            ;and reduce exponent

        MOVS    r12, r8, LSR #30        ;can we shift left by 2?
        MOVEQ   r8, r8, LSL #2          ;yes, so do so
        SUBEQ   r10, r10, #2            ;and reduce exponent

        MOVS    r12, r8, LSR #31        ;can we shift left by 1?
        MOVEQ   r8, r8, LSL #1          ;yes, so do so
        SUBEQ   r10, r10, #1            ;and reduce exponent

; Now we must check if any rounding needed

        MOVS    r12, r8, LSL #24        ;are there any bits lost by truncate?
        BNE     roundit                 ;yes...

        MOV     r9, #0                  ;then set ms. part of mantissa to 0

        STMIA   r11, {r7-r10}           ;and save in fp. register

        exit7to15                       ;then go home

roundit

; Come here to round mantissa to single prec. (24 bits) by correct mode

        ANDS    r9, r9, #&00000060      ;what rounding mode - Z bit if default
        BNE     notdefault              ;not default...

; Perform default IEEE rounding to 24 bits (implicit 1. bit included as U type)

        MOVS    r12, r12, LSL #1        ;are all bits except ms. bit lost zero?
        AND     r12, r8, #&00000080     ;get ms. bit lost
        ANDEQ   r12, r12, r8, LSR #1    ;and if yes, make sure round to even
        ADDS    r8, r8, r12             ;do rounding
        ADC     r10, r10, #0            ;and if overflow, increment exponent
        MOVCS   r8, #&80000000          ;and put in correct mantissa

trunc

; Rounding done, so do truncate and return no. in reg.

        BIC     r8, r8, #&ff            ;lose precision down to single
        STMIA   r11, {r7-r10}           ;and save in fp. register
        exit7to15                       ;then go home

notdefault

; We must be rounding to zero or + or - infinity : first try round to zero.

        EORS    r9, r9, #&00000060      ;is it round to zero?
        BEQ     trunc                   ;yes, so just truncate

; We must be rounding to + or - infinity

        EOR     r12, r7, r9, LSL #25    ;ms bit 1 if +inf and +ve or -inf and -
        AND     r12, r12, #&80000000    ;isolate this bit
        ADDS    r8, r8, r12, LSR #23    ;and use it to round
        ADC     r10, r10, #0            ;incrementing exp. if overflow
        MOVCS   r8, #&80000000          ;and correcting mantissa if necessary

; Rounding done, so do truncate and return no. in reg.

        MOV     r9, #0
        BIC     r8, r8, #&ff            ;lose precision down to single
        STMIA   r11, {r7-r10}           ;and save in fp. register
        exit7to15                       ;then go home


mfrom_arm3

; If bits ef = 11, this indicates unknown dest. size, so reject

        TST     r9, #&00000080          ;does bit f = 1?
        BNE     very_ill_instruction    ;yes, so reject

mfa_noround

; No rounding is needed to convert integer to double or extended precision
; so for these types just convert to type U (more useful later than D or E).

;        savebaseinr11                   ;r11 points at saved copy of user arm
        AND     r7, r9, #&0000f000      ;arm reg. no.
        LDR     r8, [arm, r7, LSR #10]  ;hence get integer in question in r8

; We now have integer in r8, so make U type fp no. in r7-r10 from it

        AND     r11, r9, #&00070000     ;fp. reg. no.
        ADD     r11, r12, r11, LSR #12  ;hence its address in r11

        MOVS    r7, r8                  ;sign bit in r7
        RSBMI   r8, r8, #0              ;mantissa in r8
        MOV     r10, #127+31            ;exponent in r10
        ADD     r10, r10, #&3f80

; Now we must normalise

        MOVEQ   r10, #31                ;if zero mantissa, zero exponent

        MOVS    r9, r8, LSR #16         ;can we shift left by 16?
        MOVEQ   r8, r8, LSL #16         ;yes, so do so
        SUBEQ   r10, r10, #16           ;and reduce exponent

        MOVS    r9, r8, LSR #24         ;can we shift left by 8?
        MOVEQ   r8, r8, LSL #8          ;yes, so do so
        SUBEQ   r10, r10, #8            ;and reduce exponent

        MOVS    r9, r8, LSR #28         ;can we shift left by 4?
        MOVEQ   r8, r8, LSL #4          ;yes, so do so
        SUBEQ   r10, r10, #4            ;and reduce exponent

        MOVS    r9, r8, LSR #30         ;can we shift left by 2?
        MOVEQ   r8, r8, LSL #2          ;yes, so do so
        SUBEQ   r10, r10, #2            ;and reduce exponent

        MOVS    r9, r8, LSR #31         ;can we shift left by 1?
        MOVEQ   r8, r8, LSL #1          ;yes, so do so
        SUBEQ   r10, r10, #1            ;and reduce exponent

        MOV     r9, #0                  ;then set ms. part of mantissa to 0

        STMIA   r11, {r7-r10}           ;and save in fp. register

        exit7to15                       ;then go home

;******************************************************************************
;
;       Routines to write to and read from fp status reg. to arm
;

wstat

; FPSR := Rd

;        savebaseinr11                   ;r11 points at saved copy of user arm
        AND     r7, r9, #&0000f000      ;arm reg. no.

        LDR     r10, [arm, r7, LSR #10] ;hence get new contents for FPSR

; We have new contents for FPSR - make sure they are legal before writing them

        BIC     r8, r10, #&0000001f     ;clear legal bits
        BICS    r8, r8, #&001f0000      ;and if any left
        BNE     very_ill_wstat          ;register illegal

        ORR     r10, r10, #SysId        ;install system ID - 0 at first
        STR     r10, [r12, #8*16]       ;and save in FPSR

wstat_exit
        exit7to15                       ;and go home

very_ill_wstat

        ADR     r7, wstat_exit
        MOV     r4, #NAN_E_MASK
        B       check_exep

rstat

; Rd := FPSR

;        savebaseinr11                   ;r11 points at saved copy of user arm
        AND     r7, r9, #&0000f000      ;arm reg. no.

        LDR     r10, [r12, #8*16]       ;get contents of FPSR
        STR     r10, [arm, r7, LSR #10] ;and write them out to arm reg.

        exit0to15_wb                    ;and go home


;******************************************************************************

filelevel       SETA    filelevel - 1
        END
