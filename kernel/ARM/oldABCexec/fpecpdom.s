; -> fpecpdom/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2cpdom       8/12/86 Martin Clemoes
;
;       Description:    This file contains all the monadic cpdo operations.
;                       It is GETted by the file FPE2mid. It therefore
;                       implements among others the MVF operations, which
;                       may truncate values to S or D precision (still U type).
;                       Therefore truncation routines are in this file.
;
;******************************************************************************
;
;       A NaN or two returned by untrapped invalid operations
;

negsqtNaN       *       &C5000000       ;a square root of negative number

;******************************************************************************
;
;       First the extended prec. dest. moves - no rounding. Start with
;       routines moving constants, so in range for constants in FPE2cpst.
;

rnd_half

; Deal with problem case of a half - underflow unless round to +inf.

        AND     r9, r9, #&00000060      ;unless rounding mode is to +infinity
        CMP     r9, #&00000020
        MOVEQ   r7, #&90000000          ;when move 1.0
        BEQ     move_constant

        MOV     r0, #0
        ADD     r11, r12, r10, LSR #8   ;else point at dest. reg.
        ADDR    r7, exit_rnd            ;and underflow
        B       und_uTRs

rnd_constant

; A round to integer where fm is constant indicated by r7 - only 0.5 a problem

        CMP     r7, #&e0000000          ;is it 0.5?
        BEQ     rnd_half

move_constant

; A move where fm is constant indicated by r7, dest. reg. by r10. No rounding.

        ADR     r8, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r7, r8, r7, LSR #24     ;hence constant wanted
        ADD     r11, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r7-r10}            ;load up constant
        STMIA   r11, {r7-r10}           ;save without rounding as never needed
        exit7to15                       ;and go home

move_n_constant

; Neg. move where fm is constant indicated by r7, dest. reg. by r10. No round.

        ADR     r8, U_type_constants - 8*16+4 ;point at table in FPE2cpst file
        ADD     r7, r8, r7, LSR #24     ;hence constant wanted
        ADD     r11, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r8-r10}            ;load up constant
        MOV     r7, #&80000000          ;and make it negative
        STMIA   r11, {r7-r10}           ;save without rounding as never needed
        exit7to15                       ;and go home

constant_add

; An add where fm is constant indicated by r8.

        ADR     r7, U_type_constants - 8*16+4 ;point at table in FPE2cpst file
        ADD     r8, r7, r8, LSR #24     ;hence constant wanted

        LDMIA   r8, {r8,r10,r11}        ;load up constant
        MOV     r4, #0                  ;indicate positive
        B       got_Fm_add

constant_suf

; A subtract where fm is constant indicated by r8.

        ADR     r7, U_type_constants - 8*16+4 ;point at table in FPE2cpst file
        ADD     r8, r7, r8, LSR #24     ;hence constant wanted

        LDMIA   r8, {r8,r10,r11}        ;load up constant
        MOV     r4, #0                  ;indicate positive
        B       got_Fm_suf

constant_muf

; A multiply where fm is constant indicated by r8.

        ADR     r7, U_type_constants - 8*16+4 ;point at table in FPE2cpst file
        ADD     r8, r7, r8, LSR #24     ;hence constant wanted

        LDMIA   r8, {r8,r10,r11}        ;load up constant
        B       got_Fm_muf

constant_dvf

; A divide where fm is constant indicated by r8.

        ADR     r7, U_type_constants - 8*16+4 ;point at table in FPE2cpst file
        ADD     r8, r7, r8, LSR #24     ;hence constant wanted

        LDMIA   r8, {r8,r10,r11}        ;load up constant
        B       got_Fm_dvf

constant_rdf

; A divide where fm is constant indicated by r8.

        ADR     r7, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r6, r7, r6, LSR #24     ;hence constant wanted

        LDMIA   r6, {r0-r3}             ;load up constant
        B       got_Fm_rdf

constant_pow

; A power where fm is constant indicated by r8.

        CMP     r8, #&80000000          ;are we doing x^0?
        BEQ     return_1_pow            ;yes... (Z bit must be set to work OK)

        ADR     r7, U_type_constants - 8*16+4 ;point at table in FPE2cpst file
        ADD     r8, r7, r8, LSR #24     ;hence constant wanted

        LDMIA   r8, {r8,r10,r11}        ;load up constant
        B       got_Fm_pow

constant_rpw

; A power where fm is constant indicated by r8.

        ADR     r7, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r6, r7, r6, LSR #24     ;hence constant wanted

        LDMIA   r6, {r0-r3}             ;load up constant
        B       got_Fm_rpw

sqt_constant

; A sqt where fm is constant indicated by r7, dest. reg. by r10.

        ADR     r8, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r8, r8, r7, LSR #24     ;hence constant wanted
        ADD     r11, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r8, {r0-r3}             ;load up constant

        CMP     r7, #&90000000          ;0 or 1?
        BLS     exit_sqt2               ;yes, so just return unaltered

        B       got_const_sqt

log_constant

; A log where fm is constant indicated by r7, dest. reg. by r10.

        ADR     r8, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r8, r8, r7, LSR #24     ;hence constant wanted

        LDMIA   r8, {r0-r3}             ;load up constant

        B       got_const_log

exp_constant

; An exp where fm is constant indicated by r7, dest. reg. by r10.

        ADR     r8, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r8, r8, r7, LSR #24     ;hence constant wanted

        LDMIA   r8, {r0-r3}             ;load up constant

        B       got_const_exp

tan_constant

; A tan where fm is constant indicated by r7, dest. reg. by r10.

        ADR     r8, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r8, r8, r7, LSR #24     ;hence constant wanted

        LDMIA   r8, {r0-r3}             ;load up constant

        B       got_const_tan

sin_constant

; A sin where fm is constant indicated by r7, dest. reg. by r10.

        ADDR    r8, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r8, r8, r7, LSR #24     ;hence constant wanted

        LDMIA   r8, {r0-r3}             ;load up constant

        B       got_const_sin

asin_constant

; An asin where fm is constant indicated by r7, dest. reg. by r10.

        CMP     r7, #&80000000          ;is constant 0.0?
        CMPNE   r7, #&90000000          ;or 1.0?
        CMPNE   r7, #&e0000000          ;or 0.5?
        BNE     bad_asin                ;no, so won't be OK...

        ADDR    r8, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r8, r8, r7, LSR #24     ;hence constant wanted

        LDMIA   r8, {r0-r3}             ;load up constant

        B       got_const_asin

atn_constant

; An atn where fm is constant indicated by r7, dest. reg. by r10.

        ADDR    r8, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r8, r8, r7, LSR #24     ;hence constant wanted

        LDMIA   r8, {r0-r3}             ;load up constant

        B       got_const_atn

pol_constant

; A pol where fm is constant indicated by r7, dest. reg. by r10.

        ADDR    r7, U_type_constants - 8*16 ;point at table in FPE2cpst file
        ADD     r6, r7, r6, LSR #24     ;hence constant wanted

        LDMIA   r6, {r0-r3}             ;load up constant

        B       got_const_pol

constant_rmf

; A remainder where fm is constant indicated by r8.

        ADDR    r7, U_type_constants - 8*16+4 ;point at table in FPE2cpst file
        ADD     r8, r7, r8, LSR #24     ;hence constant wanted

        LDMIA   r8, {r8,r10,r11}        ;load up constant
        B       got_Fm_rmf

mvf3

; MVF instruction, 3 word destination precision, so no rounding.
; First check destination precision is not ef = 11 : illegal

        TST     r9, #&00000080          ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

; Now just move fm to fd - first, is fm a constant?

        MOVS    r7, r9, LSL #28         ;separate fm bits
        AND     r10, r9, #&00007000     ;and fd bits
        BMI     move_constant           ;we've got a constant...

        ADD     r7, r12, r7, LSR #24    ;hence point at source reg.
        ADD     r11, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r7-r10}            ;load up source
        STMIA   r11, {r7-r10}           ;save without rounding as not needed
        exit7to15                       ;and go home

mnf3

; MNF instruction, 3 word destination precision, so no rounding.
; First check destination precision is not ef = 11 : illegal

        TST     r9, #&00000080          ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

; Now just move fm to fd - first, is fm a constant?

        MOVS    r7, r9, LSL #28         ;separate fm bits
        AND     r10, r9, #&00007000     ;and fd bits
        BMI     move_n_constant         ;we've got a constant...

        ADD     r7, r12, r7, LSR #24    ;hence point at source reg.
        ADD     r11, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r7-r10}            ;load up source
        EOR     r7, r7, #&80000000      ;negate it
        STMIA   r11, {r7-r10}           ;save without rounding as not needed
        exit7to15                       ;and go home

abs3

; ABS instruction, 3 word destination precision, so no rounding.
; First check destination precision is not ef = 11 : illegal

        TST     r9, #&00000080          ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

; Now just move fm to fd - first, is fm a constant?

        MOVS    r7, r9, LSL #28         ;separate fm bits
        AND     r10, r9, #&00007000     ;and fd bits
        BMI     move_constant           ;we've got a constant...

        ADD     r7, r12, r7, LSR #24    ;hence point at source reg.
        ADD     r11, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r7-r10}            ;load up source
        BIC     r7, r7, #&80000000      ;make it positive
        STMIA   r11, {r7-r10}           ;save without rounding as not needed
        exit7to15                       ;and go home


;******************************************************************************
;
;       S or D precision moves - may involve truncation
;

mvf

; Move register to S or D destination - may involve truncation to r. mode

        MOVS    r7, r9, LSL #28         ;separate fm bits
        AND     r10, r9, #&00007000     ;and fd bits
        BMI     move_constant           ;we've got a constant...

        ADD     r7, r12, r7, LSR #24    ;hence point at source reg.
        ADD     r10, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r0-r3}             ;load up source

; Now look to see if a loss of precision is implied

        AND     r8, r9, #&00000080      ;get dest. prec. bit, ie. f
        ORR     r8, r8, #&00000040
        CMP     r8, r3, LSR #23         ;is there a loss of precision?
        MOVCSS  r4, r3, LSL #3
        BCC     mvf_truncate            ;yes, so truncate...

; No loss of precision, so just copy

        STMIA   r10, {r0-r3}            ;save to dest. reg.
;        savebaseinr11
        exit0to15                       ;and go home

mnf

; Move neg. register to S or D destination - may involve truncation to r. mode

        MOVS    r7, r9, LSL #28         ;separate fm bits
        AND     r10, r9, #&00007000     ;and fd bits
        BMI     move_n_constant         ;we've got a constant...

        ADD     r7, r12, r7, LSR #24    ;hence point at source reg.
        ADD     r10, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r0-r3}             ;load up source
        EOR     r0, r0, #&80000000      ;negate it

; Now look to see if a loss of precision is implied

        AND     r8, r9, #&00000080      ;get dest. prec. bit, ie. f
        ORR     r8, r8, #&00000040
        CMP     r8, r3, LSR #23         ;is there a loss of precision?
        MOVCSS  r4, r3, LSL #3
        BCC     mvf_truncate            ;yes, so truncate...

; No loss of precision, so just copy

        STMIA   r10, {r0-r3}            ;save to dest. reg.
;        savebaseinr11
        exit0to15                       ;and go home

abs

; ABS register to S or D destination - may involve truncation to r. mode

        MOVS    r7, r9, LSL #28         ;separate fm bits
        AND     r10, r9, #&00007000     ;and fd bits
        BMI     move_constant           ;we've got a constant...

        ADD     r7, r12, r7, LSR #24    ;hence point at source reg.
        ADD     r10, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r0-r3}             ;load up source
        BIC     r0, r0, #&80000000      ;make positive

; Now look to see if a loss of precision is implied

        AND     r8, r9, #&00000080      ;get dest. prec. bit, ie. f
        ORR     r8, r8, #&00000040
        CMP     r8, r3, LSR #23         ;is there a loss of precision?
        MOVCSS  r4, r3, LSL #3
        BCC     mvf_truncate            ;yes, so truncate...

; No loss of precision, so just copy
copy
        STMIA   r10, {r0-r3}            ;save to dest. reg.
;        savebaseinr11
        exit0to15                       ;and go home

mvf_truncate

; Some precision will be lost, so truncate to rounding rule

        MOV     r4, r8, LSR #6          ;type to truncate to
        ORR     r4, r4, r3, LSR #26     ;and type it is now

        ADR     r7, copy                ;set truncate return to copy address

; Now we are going to perform a truncate by dropping into main truncator rout.


;******************************************************************************
;
;       Routine to convert fp numbers in r0-r3 to U type numbers in r0-r3,
;       truncated to S or D precision using rounding mode indicated by r9.
;       Only expected to be called for U or E to S or D , or, D to S.
;

;       Inputs: r0-r3 contain copy of fp register containing input value
;               r4 specifies start form and end precision.
;                  0000 0000 0000 0000 0000 0000 00ss seee
;                  sss/eee   =  0 -> unpacked
;                               1 -> single precision
;                               3 -> double precision
;                               5 -> extended precision
;               r7 contains return address
;               r9 contains copy of fp instruction
;
;       Outputs: r0-r3 contain fp value in output form
;                r4, r5 smashed
;

truncator

        CMP     r4, #2_011001           ;is this D to S?
        BEQ     DtruncS                 ;yes, so do that

        BICGT   r3, r0, #&80000000      ;if from E, first convert to U

        TST     r4, #&00000002          ;is it to S?
        BNE     UtruncD                 ;no...
                                        ;yes, so drop through to U to S

UtruncS

; Truncate U to S precision, U type
; First check for NAN, infinity, zero or denormalised

        MOVS    r5, r3, LSL #17         ;is the exponent zero?
        ADDNES  r5, r5, #&00020000      ;or &3fff?
        BEQ     funny_uTRs              ;yes, so one of funnies

; Normal U type, so first truncate mantissa (default IEEE rounding)

        ANDS    r5, r9, #&00000060      ;is it default rounding?
        BNE     n_default_uTRs          ;no...

        ORRS    r5, r2, r1, LSL #25     ;are all bits except ms bit lost zero?

        AND     r2, r1, #&00000080      ;get ms bit lost
        ANDEQ   r2, r2, r1, LSR #1      ;and if yes, make sure round to even

        ADDS    r1, r1, r2              ;do rounding
        MOVCS   r1, #&80000000          ;if overflow, correct mantissa
        ADC     r3, r3, #0              ;but if has overflowed, increment exp.

rounded_uTRs

; Rounding has been done, so do actual truncation of mantissa

        MOV     r2, #0
        BIC     r1, r1, #&ff

; Now deal with exponent

        SUBS    r5, r3, #&3f80          ;try to change to 8 bit exponent
        BLS     underflow_uTRs
        CMP     r5, #&ff

        MOVCC   r15, r7                 ;and go home
        B       overflow_uTRs           ;exponent has overflowed

n_default_uTRs

        CMP     r5, #&00000060          ;round to zero?
        ORRNES  r2, r2, r1, LSL #24     ;are we going to lose any bits?

        EORNES  r5, r0, r5, LSL #26     ;+ and +inf or - and -inf?
        BPL     rounded_uTRs            ;no, so just truncate

; We must round mantissa up

        ADDS    r1, r1, #&00000100      ;do rounding
        MOVCS   r1, #&80000000          ;if overflow, correct mantissa
        ADC     r3, r3, #0              ;but if has overflowed, increment exp.

; Rounding has been done, so do actual truncation of mantissa

        MOV     r2, #0
        BIC     r1, r1, #&ff

; Now deal with exponent

        SUBS    r5, r3, #&3f80          ;try to change to 8 bit exponent
        BLS     underflow_uTRs
        CMP     r5, #&ff

        MOVCC   r15, r7                 ;and go home
        B       overflow_uTRs           ;exponent has overflowed

funny_uTRs

; Deal with NaN, infinity, zero or denormalised as best we can

        TST     r3, #&ff
        MOVNE   r15, r7                 ;if exponent is max., return
        ORRS    r2, r2, r1              ;else was U denormalised?
        MOVEQ   r15, r7                 ;no, so go home with zero
        B       und_uTRs                ;yes, so underflow definitely

overflow_uTRs

; Exponent too big for single precision

        MOV     r3, #&ff
        ORR     r3, r3, #&00007f00      ;put infinity in value
        MOV     r1, #0

        MOV     r4, #OFL_E_MASK         ;and register an overflow
        B       check_exep

underflow_uTRs

; Exponent too small for single precision, so first try gradual underflow

        CMN     r5, #23                 ;will gradual underflow cope?
        BLE     und_uTRs                ;no, so register an underflow

        RSB     r5, r5, #9              ;negate r5 and add 9
        MOV     r1, r1, LSR r5          ;and put remains of mantissa in

        MOV     r1, r1, LSL r5
        ret2                            ;and go home

und_uTRs

; Underflow has definitely occurred

        MOV     r3, #0                  ;return 0 if exception disabled
        MOV     r2, #0
        MOV     r1, #0

        MOV     r4, #UFL_E_MASK         ;and register an underflow
        B       check_exep

UtruncD

; Truncate U to D precision, U type
; First check for NAN, infinity, zero or denormalised

        MOVS    r5, r3, LSL #17         ;is the exponent zero?
        ADDNES  r5, r5, #&00020000      ;or &3fff?
        BEQ     funny_uTRd              ;yes, so one of funnies

; Normal U type, so first truncate mantissa (default IEEE rounding)

        ANDS    r5, r9, #&00000060      ;is it default rounding?
        BNE     n_default_uTRd          ;no...

        MOVS    r4, r2, LSL #22         ;are all bits except ms bit lost zero?

        AND     r4, r2, #&00000400      ;get ms bit lost
        ANDEQ   r4, r4, r2, LSR #1      ;and if yes, make sure round to even

        ADDS    r2, r2, r4              ;do rounding
        ADCS    r1, r1, #0

        ADC     r3, r3, #0              ;if has overflowed, increment exp.
        MOVCS   r1, #&80000000          ;and set up correct mantissa

rounded_uTRd

; Rounding has been done, so now do actual truncation of mantissa

        MOV     r2, r2, LSR #11
        MOV     r2, r2, LSL #11

; Now deal with exponent

        SUBS    r5, r3, #&3c00          ;try to change to 11 bit exponent
        BLS     underflow_uTRd
        ADD     r4, r5, #1
        CMP     r4, #&800

        MOVCC   r15, r7                 ;and go home
        MOV     r2, #0
        B       overflow_uTRs           ;exponent has overflowed

n_default_uTRd

        CMP     r5, #&00000060          ;round to zero?
        MOVNES  r4, r2, LSL #21         ;are no bits lost?

        EORNES  r5, r0, r5, LSL #26     ;+ and +inf or - and -inf?
        BPL     rounded_uTRd            ;no, so just truncate

; We must round mantissa up

        ADDS    r2, r2, #&00000800      ;do rounding
        ADCS    r1, r1, #0
        MOVCS   r1, #&80000000          ;if overflow, correct mantissa
        ADC     r3, r3, #0              ;but if has overflowed, increment exp.

; Rounding has been done, so do actual truncation of mantissa

        MOV     r2, r2, LSR #11
        MOV     r2, r2, LSL #11

; Now deal with exponent

        SUBS    r5, r3, #&3c00          ;try to change to 11 bit exponent
        BLS     underflow_uTRd
        ADD     r4, r5, #1
        CMP     r4, #&800

        MOVCC   r15, r7                 ;and go home
        MOV     r2, #0
        B       overflow_uTRs           ;exponent has overflowed

funny_uTRd

; Deal with NaN, infinity, zero or denormalised as best we can

        TST     r3, #&ff
        MOVNE   r15, r7                 ;if max. exponent, just return
        ORRS    r2, r2, r1              ;else was U denormalised?
        MOVEQ   r15, r7                 ;no, so go home with zero
        B       und_uTRs                ;yes, so underflow definitely

underflow_uTRd

; Exponent too small for double precision, so first try gradual underflow

        CMN     r5, #52                 ;will gradual underflow cope?
        BLE     und_uTRs                ;no, so register an underflow

        RSB     r5, r5, #12             ;negate r5 and add 12 (to truncate)
        SUBS    r4, r5, #32             ;is shift >31 ?

        MOVCS   r2, r1, LSR r4          ;yes, so leave r0, set up r1

        MOVCC   r2, r2, LSR r5          ;else rotate r2 by r5

        MOVCS   r1, r2, LSL r4          ;if big shift, move r1 back

        MOV     r2, r2, LSL r5

        ret2                            ;and go home

DtruncS

; Truncate D to S precision, U type
; Copy exponent to r3 and check for NaN, infinity, zero or denormalised

        MOV     r3, r0, LSL #1          ;lose sign bit
        MOVS    r3, r3, LSR #21         ;and mantissa - is exponent zero
        ADDNE   r4, r3, #1
        TEQNE   r4, #&800               ;or max value?
        BEQ     funny_dTRs              ;yes, so one of funnies...

        ADD     r3, r3, #&3c00          ;convert exp. to U type

; Normal D type, so put rounded mantissa in r1

        ANDS    r5, r9, #&00000060      ;default rounding mode?
        BNE     n_default_dTRs          ;no...

        MOVS    r4, r1, LSL #4          ;all bits except ms bit lost zero?

        MOV     r1, r1, LSR #21         ;arrange r1
        ORR     r1, r1, r0, LSL #11
        ORR     r1, r1, #&80000000      ;including implicit 1. bit

        AND     r4, r1, #&00000080      ;get ms bit lost
        ANDEQ   r4, r4, r1, LSR #1      ;and if yes, round to even (IEEE)
        ADDS    r1, r1, r4              ;do rounding

        MOVCS   r1, #&80000000          ;if overflow, correct mantissa
        ADC     r3, r3, #0              ;if overflow, increment exponent

rounded_dTRs

; Rounding done, so do actual truncate

        BIC     r1, r1, #&ff
        MOV     r2, #0

; Now deal with exponent

        SUBS    r5, r3, #&3f80          ;try to change to 8 bit exponent
        BLS     underflow_uTRs
        CMP     r5, #&ff

        MOVCC   r15, r7                 ;and go home
        B       overflow_uTRs           ;exponent has overflowed

n_default_dTRs

        MOVS    r4, r1, LSL #3          ;all bits lost zero?

        MOV     r1, r1, LSR #21         ;arrange r1
        ORR     r1, r1, r0, LSL #11
        ORR     r1, r1, #&80000000      ;including implicit 1. bit

        CMPNE   r5, #&00000060          ;round to zero?

        EORNES  r5, r0, r5, LSL #26     ;+ and +inf or - and -inf?
        BPL     rounded_dTRs            ;no, so just truncate

; We must round mantissa up

        ADDS    r1, r1, #&00000100      ;do rounding
        MOVCS   r1, #&80000000          ;if overflow, correct mantissa
        ADC     r3, r3, #0              ;but if has overflowed, increment exp.

; Rounding has been done, so do actual truncation of mantissa

        MOV     r2, #0
        BIC     r1, r1, #&ff

; Now deal with exponent

        SUBS    r5, r3, #&3f80          ;try to change to 8 bit exponent
        BLS     underflow_uTRs
        CMP     r5, #&ff

        MOVCC   r15, r7                 ;and go home
        B       overflow_uTRs           ;exponent has overflowed

funny_dTRs

; Deal with NaN, infinity, zero or denormalised as best we can

        ORR     r3, r3, r3, LSL #4      ;extend exponent to 15 bits
        ORRS    r2, r1, r0, LSL #12     ;was D zero or infinity?
        MOVEQ   r15, r7                 ;yes, so go home

        MOV     r2, r1, LSL #11         ;just in case NaN, transfer mantissa
        MOV     r1, r1, LSR #21
        ORR     r1, r1, r0, LSL #11
        TST     r3, #&ff
        MOVNE   r15, r7                 ;if exponent is max., return
        B       und_uTRs                ;denormalised, so underflow definitely


;******************************************************************************
;
;       Some macros using the above truncation routines
;

        MACRO
        truncate $cond

        ADR     r7, %FT40
        B$cond  truncator
40
        MEND


        MACRO
        truncate_f

        ADR     r7, %FT50

        CMP     r4, #2_011001           ;is this D to S?
        BEQ     DtruncS                 ;yes, so do that

        BICGT   r3, r0, #&80000000      ;if from E, first convert to U

        TST     r4, #&00000002          ;is it to S?
        BEQ     UtruncS                 ;yes...
        B       UtruncD
50
        MEND

;******************************************************************************
;
;       RND operation - round to integer value, U type and put in fp reg.
;

rnd3

        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

rnd

; First check to see if source a const., and if not, load source reg.

        MOVS    r7, r9, LSL #28         ;separate fm bits
        AND     r10, r9, #&00007000     ;and fd bits
        BMI     rnd_constant            ;we've got a constant...

        ADD     r7, r12, r7, LSR #24    ;hence point at source reg.
        ADD     r11, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r7, {r0-r3}             ;load up source

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U

; Now check for NaN, infinity, zero or denormalised

        MOV     r4, r3, LSL #17         ;is exponent
        ADDS    r4, r4, #&00020000      ;maximum value?
        BEQ     funny_rnd               ;yes, so must be one of funnies...

; Now do the job using subroutine in utils

        STMDB   r12!, {r14}
        AisrndA
        LDMIA   r12!, {r14}
        ADR     r7, exit_rnd            ;underflow, so check exception
        BNE     check_exep              ;(mask in r4)

return_rnd

; We have truncated to integer - truncate to S precision if wanted, then return

        ORR     r8, r9, r9, LSR #12     ;is dest. precision S?
        TST     r8, #&80
        BEQ     UtruncS                 ;yes, so truncate to S precision
        TST     r9, #&80                ;is dest. precision D?
        BNE     UtruncD                 ;yes, so truncate to D precision

exit_rnd

        STMIA   r11, {r0-r3}            ;save value to fp. reg.
;        savebaseinr11
        exit0to15                       ;and go home...

funny_rnd

; Deal with NaN or infinity - leaves arm regs alone and causes exception as ap.

        ORRS    r8, r2, r1              ;infinity?
        BEQ     overflow_rnd            ;yes, so overflow...

; We have a NaN - great!

        ORR     r1, r1, #&40000000      ;make sure NaN is non-trapping
        ADR     r7, exit_rnd            ;return to a handy exit0to15 if no exn.
        MOV     r4, #NAN_E_MASK         ;register event
        B       check_exep

overflow_rnd

; We have overflowed - if no exception, return infinity

        MOV     r2, #0                  ;return infinity if no exception
        ADR     r7, exit_rnd
        B       overflow_uTRs

;******************************************************************************
;
;       Square root routines
;

sqt3

        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

sqt

; First check to see if source a const., and if not, load source reg.

        MOVS    r7, r9, LSL #28         ;separate fm bits
        AND     r10, r9, #&00007000     ;and fd bits
        BMI     sqt_constant            ;we've got a constant...

        ADD     r8, r12, r7, LSR #24    ;hence point at source reg.
        ADD     r11, r12, r10, LSR #8   ;and point at dest. reg.

        LDMIA   r8, {r0-r3}             ;load up source

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U
        STMNEIA r8, {r0-r3}             ;and write back if converted

; Now check for NaN, infinity, zero or denormalised

        MOVS    r4, r3, LSL #17         ;is exponent zero?
        ADDNES  r4, r4, #&00020000      ;or maximum value?
        BEQ     funny_sqt               ;yes, so must be one of funnies...

normal_sqt

; Now check for square root of negative number

        TST     r0, #&80000000          ;is sign negative (zero already done)
        BNE     neg_sqt                 ;yes, oh dear...

got_const_sqt

; We have an acceptable normalised no. to sqt - first deal with exponent

        ADD     r3, r3, #&4000          ;make bias double
        SUB     r3, r3, #1
        MOVS    r3, r3, LSR #1          ;then halve exponent, C if odd

; Initialise remainder

        MOV     r5, r2                  ;remainder in r4,r5
        SUBCC   r4, r1, #&80000000      ;even exponent
        SUBCS   r4, r1, #&40000000      ;odd exponent

; Initialise rotating bit pattern

        MOVCC   r7, #&10000000          ;even exponent -> MS bits 10
        MOVCS   r7, #&20000000          ;odd exponent -> MS bits 1x

; Initialise result (to be returned in r1,r2)

        MOV     r1, #&80000000

sqt_loop1

; The first main sqt loop - when finished we will have 31 bits of result

        ADDS    r5, r5, r5              ;double remainder
        ADCS    r4, r4, r4              ;hence C -> rem>result||01
        
        EOR     r6, r1, r7              ;result||01 in r6
        CMPCC   r4, r6                  ;hence will this subtract?
        SUBCS   r4, r4, r6              ;yes, so do so
        EORCS   r1, r1, r7, LSL #1      ;and put a 1 in result

        MOVS    r7, r7, ROR #1          ;then rotate pattern
        BPL     sqt_loop1               ;and carry on...

; We have 31 bits of result in r1 - if precision S or exact, finish

        ORRS    r2, r4, r5              ;is remainder zero?

        ORRNE   r2, r9, r9, LSR #12     ;or is destination single prec.?
        ANDNES  r2, r2, #&00000080
        MOVEQ   r7, #0                  ;least SW of remainder = 0
        BEQ     rooted                  ;yes, so finish...

        MOV     r2, #0                  ;clear r2 for lower result

sqt_loop2

; The second main sqt loop - when finished we will have 63 bits of result

        ADDS    r5, r5, r5              ;double remainder
        ADCS    r4, r4, r4              ;hence C -> rem>result||01
        ADC     r0, r0, r0              ;so save C for later

        EOR     r6, r2, r7              ;result||01 in r1,r6
        SUBS    r10, r5, r6             ;attempt to subtract result||01
        SBCS    r8, r4, r1              ;from remainder
        MOVCCS  r0, r0, LSR #1          ;retrieve C bit if clear

        MOVCS   r5, r10                 ;if subtraction successful, use it
        MOVCS   r4, r8
        EORCS   r2, r2, r7, LSL #1      ;and put a 1 in result
        EORCS   r1, r1, r7, LSR #31     ;making sure of last bit in r1

        MOVS    r7, r7, LSR #1          ;rotate pattern
        BNE     sqt_loop2               ;and carry on...

; We now have 63 bits of result, so calculate the last bit

        ADDS    r5, r5, r5              ;double remainder
        ADCS    r4, r4, r4              ;hence C -> rem>result||01
        ADCS    r0, r7, r7              ;so save C for later, and clear C

        SBCS    r10, r5, r2             ;attempt to subtract result||01
        SBCS    r8, r4, r1              ;from remainder
        MOVCCS  r0, r0, LSR #1          ;retrieve C bit if clear

        MOVCS   r5, r10                 ;if subtraction successful, use it
        MOVCS   r4, r8
        EORCS   r2, r2, #1              ;and put a 1 in result
        MOVCS   r7, #&80000000          ;and use r7 as extension of remainder
        MOV     r0, #0                  ;zero r0 again to indicate positive

rooted

; We now have a 64 bit result, with a 96 bit remainder in r4,r5,r7

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_sqt            ;yes...

        ORRS    r4, r5, r4              ;any remainder?
        ORREQS  r4, r4, r7
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines
        ADR     r7, exit_sqt1
        CMP     r3, #0
        BLE     und_uTRs

        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

no_trunc_sqt

; Extended precision, so no truncation required - just round correctly

        ANDS    r6, r9, #&00000060      ;default rounding?
        BNE     n_d_rnd_sqt             ;no...

        ADDS    r7, r7, r7
        ADCS    r5, r5, r5              ;double remainder
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        BCS     num_biggerRs            ;hence result||01 can be subtracted

        CMP     r4, r1                  ;result||01 > remainder?
        CMPEQ   r5, r2
        CMPEQ   r7, #&40000000

num_biggerRs
        ADCS    r2, r2, #0              ;add in carry to do rounding
return_sqt
        ADCS    r1, r1, #0

        ADC     r3, r3, #0              ;if overflow, increment exponent
        MOVCS   r1, #&80000000          ;and restore mantissa

exit_sqt1

; Register inexact event, store result and go home

        LDR     r5, [r12, #8*16]        ;get fpsr
        TST     r5, #INX_E_MASK         ;inexact event already?
        BEQ     register_inx_mon        ;no, so register one...

exit_sqt2

        STMIA   r11, {r0-r3}            ;save value to fp. reg.
;        savebaseinr11
        exit0to15                       ;and go home...

register_inx_mon

; Inexact event hasn't happened before, so register one

        ORR     r5, r5, #INX_E_MASK     ;set bit,
        STR     r5, [r12, #16*8]        ;and write fpsr back

        TST     r5, #INX_E_MASK*&10000  ;do we need an exception?
        BEQ     exit_sqt2               ;no, so carry on...

        MOV     r0, #INX_E_MASK         ;yes, so cause one...
        B       fp_exception

n_d_rnd_sqt

; Round extended precision, but not default mode

        ORRS    r5, r4, r5              ;any remainder?
        ORREQS  r5, r5, r7
        CMPNE   r6, #&00000060
        BEQ     exit_sqt1               ;no, so exact number...

        TEQ     r0, r6, LSL #25         ;+ and round to + inf or - and -inf?
        BMI     exit_sqt1               ;no, so ignore remainder

        ADDS    r2, r2, #1              ;round up
        B       return_sqt

neg_sqt

; We are trying to take a square root of a negative non-zero number

        MOV     r1, #negsqtNaN          ;if untrapped, return NaN
        MOV     r2, #0
        MOV     r3, #&ff
        ORR     r3, r3, r3, LSL #7

nan_sqt
        ORR     r1, r1, #&40000000      ;make sure NaN is untrapping
        ADR     r7, exit_sqt2
        MOV     r4, #NAN_E_MASK
        B       check_exep

funny_sqt

; We are taking the square root of +/-0, infinity, NaN or denormalised

        CMP     r3, #0                  ;NaN or infinity?
        BNE     max_exp_sqt             ;yes...

        ORRS    r4, r1, r2              ;zero?
        BEQ     exit_sqt2               ;yes, so return unaltered
        MOVS    r1, r1                  ;denormalised?
        BMI     normal_sqt              ;no, so go back

norm_sqt
        ADDS    r2, r2, r2              ;double mantissa
        ADCS    r1, r1, r1
        SUB     r3, r3, #1              ;decrement exponent
        BPL     norm_sqt

        B       normal_sqt

max_exp_sqt
        ORRS    r4, r1, r2              ;infinity?
        BNE     nan_sqt                 ;no, so must be a NaN
        TST     r0, #&80000000          ;yes, so is it negative?
        BEQ     exit_sqt1               ;no, so OK
        B       neg_sqt                 ;yes...


;******************************************************************************

filelevel       SETA    filelevel - 1
        END
