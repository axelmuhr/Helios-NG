; -> fpecpdod/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2cpdod       22/12/86 Martin Clemoes
;
;       Description:    This file contains or gets all the dyadic cpdo ops.
;                       It is GETted by the file FPE2mid. It therefore
;                       implements among others the ADF, SUF and RSF operations
;
;******************************************************************************
;
;       Definitions of some NaN's returned by arithmetic routines
;

subinfNaN       *       &c1000000       ;NaN returned if untrapped inf - inf
muf0infNaN      *       &c2000000       ;NaN returned if untrapped 0 * inf
a0div0NaN       *       &c3000000       ;NaN returned if untrapped 0/0
infdivinfNaN    *       &c4000000       ;NaN returned if untrapped inf/inf
badpowNaN       *       &c7000000       ;NaN returned if illegal POW or RPW
badrmfNaN       *       &cc000000       ;NaN returned if illegal RMF operand(s)

;******************************************************************************
;
;       Add and subtract routines (interlinked for obvious reasons)
;

add3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

add
        MOVS    r8, r9, LSL #28         ;get bits indicating Fm
        BMI     constant_add            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r8, r12, r8, LSR #24    ;point at fm store
        LDMIA   r8, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_add       ;if not, convert

        STMIA   r8, {r0-r3}             ;write back in U type if converted
nocon_add
        MOV     r11, r3                 ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        MOV     r8, r1

        AND     r4, r0, #&80000000      ;save sign for later

got_Fm_add

; We have loaded up fm, so now load fn and convert to U if necessary

        AND     r6, r9, #&00070000      ;get bits indicating fn,
        ADD     r6, r12, r6, LSR #12    ;point at fn store
        LDMIA   r6, {r0-r3}             ;and load it up

        TEQ     r4, r0                  ;are signs different?
        BMI     suf_entry               ;yes, so subtract...

add_entry
        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_add2      ;if not, convert

        STMIA   r6, {r0-r3}             ;write back in U type if converted
nocon_add2
; Now check to see if either value is a NaN

        MOV     r6, #&00020000          ;convenient bit
        ADDS    r5, r6, r3, LSL #17     ;has fn got maximum exponent?
        ADDCCS  r5, r6, r11, LSL #17    ;if not, has fm got max. exponent?
        BCS     funny_add               ;yes, so either nan or infinity

; We have now got numbers ready to add, so first make sure bigger in r0-r3

        SUBS    r4, r3, r11             ;difference in exponents
        BMI     Fm_bigger_add           ;negative, so swap round

; We have smaller no. mantissa in r8 and r10, so shift and add

        RSBS    r5, r4, #32             ;is shift > 1 word?
        BLE     big_sh_add              ;yes...

        ADDS    r2, r2, r10, LSR r4     ;add shifted smaller no.
        ADDCS   r2, r2, r8, LSL r5
        ADDCCS  r2, r2, r8, LSL r5      ;taking care to get C right

        ADCS    r1, r1, r8, LSR r4

        MOV     r8, r10, LSL r5         ;ms. bit of r8 = ms. bit lost
        MOV     r10, r8, LSL #1         ;and r10 shows if any remainder

        BCS     add_round_CS            ;overflow, so increment exponent...

; No overflow, so truncate if necessary

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_add            ;yes...

        ORRS    r10, r10, r8            ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines

        ADR     r7, exit_add
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

big_sh_add

; A shift of 32 bits or more needed - first deal with shift >63 - totally lost

        CMP     r4, #64                 ;will smaller be totally shifted away?
        ORRGT   r10, r10, r8            ;yes, so maintain record of any rem.
        MOVGT   r8, #0                  ;if shift > 64, ms. bit lost = 0
        BGE     add_round_CC

; A shift of 32 - 63 bits - some adding to do

        SUB     r4, r4, #32
        RSB     r5, r4, #32

        ADDS    r2, r2, r8, LSR r4      ;do addition
        ADCS    r1, r1, #0

        MOV     r8, r8, LSL r5          ;set up ms. bit lost
        ORR     r10, r10, r8, LSL #1    ;and any rem. word

add_round_CS

; If we branched to here, C set, so increment exponent, and shift 1 bit in ms.

        ADC     r3, r3, #0              ;increment exponent

        ORRCS   r10, r10, r8            ;update anything left word
        MOVCS   r8, r2, LSL #31         ;and ms. bit lost

        MOVCS   r2, r2, LSR #1          ;shift mantissa right 1
        ORRCS   r2, r2, r1, LSL #31
        MOVCS   r1, r1, RRX             ;setting ms. bit at same time

add_round_CC

; No increment of exponent, so just truncate to required precision

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_add            ;yes...

        ORRS    r10, r10, r8            ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines

        ADR     r7, exit_add
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

no_trunc_add

; Extended precision, so no truncation required - just round correctly

        ANDS    r7, r9, #&00000060      ;default rounding?
        BNE     n_d_rnd_add             ;no...

        ORRS    r10, r10, r8, LSL #1    ;are all bits except ms. bit lost zero?
        ANDEQ   r8, r8, r2, LSL #31     ;if yes, round to even (IEEE)
        ADDS    r2, r2, r8, LSR #31     ;and perform round
return_add
        ADCS    r1, r1, #0

        ADC     r3, r3, #0              ;if overflow, increment exponent
        MOVCS   r1, #&80000000          ;and restore mantissa

; Now we must make sure extended precision hasn't overflowed

        ADDS    r5, r6, r3, LSL #17     ;has exponent overflowed?
        BEQ     overflow_add            ;yes...

exit_add

; Register inexact event, store result and go home

        LDR     r5, [r12, #8*16]        ;get fpsr
        TST     r5, #INX_E_MASK         ;inexact event already?
        BEQ     register_inx            ;no, so register one...

exit_add2
        AND     r9, r9, #&00007000      ;destination reg. no.
        ADD     r9, r12, r9, LSR #8     ;hence its address
        STMIA   r9, {r0-r3}             ;so save its contents

;        savebaseinr11                   ;and go home...
        exit0to15

register_inx

; Inexact event hasn't happened before, so register one

        ORR     r5, r5, #INX_E_MASK     ;set bit,
        STR     r5, [r12, #16*8]        ;and write fpsr back

        TST     r5, #INX_E_MASK*&10000  ;do we need an exception?
        BEQ     exit_add2               ;no, so carry on...

        MOV     r0, #INX_E_MASK         ;yes, so cause one...
        B       fp_exception

n_d_rnd_add

; Round extended precision, but not default mode

        ORRS    r10, r10, r8            ;any remainder?
        CMPNE   r7, #&00000060
        BEQ     exit_add                ;no, so exact number...

        TEQ     r0, r7, LSL #25         ;+ and round to + inf or - and -inf?
        BMI     exit_add                ;no, so ignore remainder

        ADDS    r2, r2, #1              ;round up
        B       return_add

Fm_bigger_add

; Fm is bigger, so we'll have to swap them round (compactness v. speed)

        RSB     r4, r4, #0              ;negate r4

        MOV     r3, r11                 ;move exponent

        MOV     r5, r1                  ;move mantissa msw.
        MOV     r1, r8
        MOV     r8, r5

        MOV     r5, r2                  ;move mantissa lsw.
        MOV     r2, r10
        MOV     r10, r5

; We have smaller no. mantissa in r8 and r10, so shift and add

        RSBS    r5, r4, #32             ;is shift > 1 word?
        BLE     big_sh_add              ;yes...

        ADDS    r2, r2, r10, LSR r4     ;add shifted smaller no.
        ADDCS   r2, r2, r8, LSL r5
        ADDCCS  r2, r2, r8, LSL r5      ;taking care to get C right

        ADCS    r1, r1, r8, LSR r4

        MOV     r8, r10, LSL r5         ;ms. bit of r8 = ms. bit lost
        MOV     r10, r8, LSL #1         ;and r10 shows if any remainder

        BCS     add_round_CS            ;overflow, so increment exponent...

; No overflow, so truncate if necessary

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_add            ;yes...

        ORRS    r10, r10, r8            ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines

        ADR     r7, exit_add
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

funny_add

; One of operands had max. exponent - NaN or infinity

        ADDS    r5, r6, r3, LSL #17     ;was fn causing trouble?
        BNE     funny_fm_add            ;no, so it must have been fm...

        ORRS    r5, r2, r1              ;yes, so was it infinity?
        BNE     NaN_add                 ;no, NaN - cause trouble

funny_fm_add

        ADDS    r5, r6, r11, LSL #17    ;was fm causing trouble?
        BNE     exit_add                ;no, so return infinity (fn)

        MOV     r3, r11                 ;move fm to fn position
        MOV     r1, r8
        MOV     r2, r10

        ORRS    r5, r1, r2              ;infinity?
        BEQ     exit_add

NaN_add

; We have come across a NaN - make it non-trapping if exception disabled

        ORR     r1, r1, #&40000000      ;make it a non-trapping nan

        ADR     r7, exit_add            ;and cause exception if enabled
        MOV     r4, #NAN_E_MASK
        B       check_exep

overflow_add

        MOV     r3, #&ff                ;return infinity if trap disabled
        ORR     r3, r3, r3, LSL #7
        MOV     r2, #0
        MOV     r1, #0

        ADR     r7, exit_add            ;and cause exception if enabled
        MOV     r4, #OFL_E_MASK
        B       check_exep

switch_to_add

; Was subtract, but signs same so add

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_add3      ;if not, convert

        STMIA   r6, {r0-r3}             ;write back in U type if converted
nocon_add3
        EOR     r0, r0, r9, LSL #11     ;negate fn if rsb
        B       nocon_add2

; Subtract (and reverse subtract) routines

suf3
rsf3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

suf
rsf
        MOVS    r8, r9, LSL #28         ;get bits indicating Fm
        BMI     constant_suf            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r8, r12, r8, LSR #24    ;point at fm store
        LDMIA   r8, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_suf       ;if not, convert

        STMIA   r8, {r0-r3}             ;write back in U type if converted
nocon_suf
        MOV     r11, r3                 ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        MOV     r8, r1

        AND     r4, r0, #&80000000      ;save sign for later

got_Fm_suf

; We have loaded up fm, so now load fn and convert to U if necessary

        AND     r6, r9, #&00070000      ;get bits indicating fn,
        ADD     r6, r12, r6, LSR #12    ;point at fn store
        LDMIA   r6, {r0-r3}             ;and load it up

        TEQ     r4, r0                  ;are signs same?
        BMI     switch_to_add           ;no, so add...

suf_entry
        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_suf2      ;if not, convert

        STMIA   r6, {r0-r3}             ;write back in U type if converted
nocon_suf2
        EOR     r0, r0, r9, LSL #11     ;negate fn if rsb

; Now check to see if either value is a NaN

        MOV     r6, #&00020000          ;convenient bit
        ADDS    r5, r6, r3, LSL #17     ;has fn got maximum exponent?
        ADDCCS  r5, r6, r11, LSL #17    ;if not, has fm got max. exponent?
        BCS     funny_suf               ;yes, so either nan or infinity

; We have now got numbers ready to suf, so first make sure bigger in r0-r3

        SUBS    r4, r3, r11             ;difference in exponents
        CMPEQ   r1, r8
        CMPEQ   r2, r10
        BEQ     same_suf                ;equal, so return zero
        BCC     Fm_bigger_suf           ;negative, so swap round

; We have smaller no. mantissa in r8 and r10, so shift and suf

        MOV     r7, #0                  ;create 2 word extension for result

        RSBS    r5, r4, #32             ;is shift > 1 word?
        BLE     big_sh_suf              ;yes...

        SUBS    r6, r7, r10, LSL r5     ;r6 and r7 now contain remainder
        SBCS    r2, r2, r10, LSR r4     ;now calculate result
        SUBCC   r2, r2, r8, LSL r5
        SUBCSS  r2, r2, r8, LSL r5

        SBCS    r1, r1, r8, LSR r4

        BPL     suf_round_norm          ;unnormalised, so re normalise

; No overflow, so truncate if necessary

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_suf            ;yes...

        ORRS    r7, r7, r6              ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines

        ADR     r7, exit_suf1
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

huge_sh_suf

; A shift of 64 or more bits needed

        MOVNE   r6, #&80000000          ;if shift > 64 then ms rem bit 1
        MOV     r7, #1                  ;and something else left as well

        ORRS    r5, r8, r10             ;are we subtracting zero?
        BEQ     exit_suf1               ;yes, so return unchanged value

        SUBS    r2, r2, #1              ;subtract 1 from mantissa
        SBCS    r1, r1, #0

        BPL     suf_round_norm          ;normalise if necessary
        B       suf_round               ;otherwise just carry on...

big_sh_suf

; A shift of 32 bits or more needed - first deal with shift >63 - totally lost

        MOV     r6, #0
        CMP     r4, #64                 ;will smaller be totally shifted away?
        BGE     huge_sh_suf

; A shift of 32 - 63 bits - some sufing to do

        SUB     r4, r4, #32
        RSB     r5, r4, #32

        SUBS    r7, r7, r10, LSL r5     ;r6 and r7 now contain remainder
        SBCS    r6, r6, r10, LSR r4
        SUBCC   r6, r6, r8, LSL r5
        SUBCSS  r6, r6, r8, LSL r5

        SBCS    r2, r2, r8, LSR r4      ;now calculate result
        SBCS    r1, r1, #0

        BMI     suf_round               ;if normalisation not needed, jump

suf_round_norm

; MS bit zero after subtract, so normalisation needed - usually by only 1 bit

;        MOVS    r3, r3                  ;if zero exponent,
;        BEQ     suf_round               ;leave as denormalised number

        ADDS    r7, r7, r7              ;double mantissa
        ADCS    r6, r6, r6
        ADCS    r2, r2, r2
        ADCS    r1, r1, r1

        SUB     r3, r3, #1              ;decrement exponent

        BPL     suf_round_norm          ;and repeat if necessary

suf_round

; Now deal with truncation as necessary

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_suf            ;yes...

        CMP     r3, #0                  ;has exponent underflowed?
        BLE     und_suf                 ;yes, so underflow...

        ORRS    r7, r7, r6              ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines

        ADR     r7, exit_suf1
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

no_trunc_suf

; Extended precision, so no truncation required - just round correctly

        ANDS    r5, r9, #&00000060      ;default rounding?
        BNE     n_d_rnd_suf             ;no...

        ORRS    r7, r7, r6, LSL #1      ;are all bits except ms. bit lost zero?
        ANDEQ   r6, r6, r2, LSL #31     ;if yes, round to even (IEEE)
        ADDS    r2, r2, r6, LSR #31     ;and perform round
return_suf
        ADCS    r1, r1, #0

        ADC     r3, r3, #0              ;if overflow, increment exponent
        MOVCS   r1, #&80000000          ;and restore mantissa

exit_suf

; Now we must make sure extended precision hasn't underflowed

        MOVS    r3, r3                  ;has exponent underflowed?
        BMI     denorm_suf              ;yes...

exit_suf1

; Register inexact event, store result and go home

        LDR     r5, [r12, #8*16]        ;get fpsr
        TST     r5, #INX_E_MASK         ;inexact event already?
        BEQ     register_inx            ;no, so register one...

exit_suf2
        AND     r9, r9, #&00007000      ;destination reg. no.
        ADD     r9, r12, r9, LSR #8     ;hence its address
        STMIA   r9, {r0-r3}             ;so save its contents

;        savebaseinr11                   ;and go home...
        exit0to15

n_d_rnd_suf

; Round extended precision, but not default mode

        ORRS    r7, r7, r6              ;any remainder?
        CMPNE   r5, #&00000060
        BEQ     exit_suf                ;no, so exact number...

        TEQ     r0, r5, LSL #25         ;+ and round to + inf or - and -inf?
        BMI     exit_suf                ;no, so ignore remainder

        ADDS    r2, r2, #1              ;round up
        B       return_suf

Fm_bigger_suf

; Fm is bigger, so we'll have to swap them round (compactness v. speed)

        RSB     r4, r4, #0              ;negate r4

        EOR     r0, r0, #&80000000      ;and negate result

        MOV     r3, r11                 ;move exponent

        MOV     r5, r1                  ;move mantissa msw.
        MOV     r1, r8
        MOV     r8, r5

        MOV     r5, r2                  ;move mantissa lsw.
        MOV     r2, r10
        MOV     r10, r5

; We have smaller no. mantissa in r8 and r10, so shift and suf

        MOV     r7, #0                  ;create 2 word extension for result

        RSBS    r5, r4, #32             ;is shift > 1 word?
        BLE     big_sh_suf              ;yes...

        SUBS    r6, r7, r10, LSL r5     ;r6 and r7 now contain remainder
        SBCS    r2, r2, r10, LSR r4     ;now calculate result
        SUBCC   r2, r2, r8, LSL r5
        SUBCSS  r2, r2, r8, LSL r5

        SBCS    r1, r1, r8, LSR r4

        BPL     suf_round_norm          ;unnormalised, so re normalise

; No overflow, so truncate if necessary

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_suf            ;yes...

        ORRS    r7, r7, r6              ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines

        ADR     r7, exit_suf1
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

funny_suf

; One of operands had max. exponent - NaN or infinity

        ADDS    r5, r6, r3, LSL #17     ;was fn causing trouble?
        BNE     funny_fm_suf            ;no, so it must have been fm...

        ORRS    r5, r2, r1              ;yes, so was it infinity?
        BNE     NaN_add                 ;no, NaN - cause trouble

; We have +/-(infinity - something), so if something = infinity, return NaN

        CMP     r3, r11                 ;infinity - infinity?
        ORREQS  r5, r8, r10

        MOVEQ   r1, #subinfNaN          ;yes, so return correct NaN
        BEQ     NaN_add

funny_fm_suf

        ADDS    r5, r6, r11, LSL #17    ;was fm causing trouble?
        BNE     exit_suf1               ;no, so return infinity (fn)

        EOR     r0, r0, #&80000000      ;+/-(const - infinity) so negate and
        MOV     r3, r11                 ;move fm to fn position,
        MOV     r1, r8                  ;ie return -/+(infinity)
        MOV     r2, r10

        ORRS    r5, r1, r2              ;infinity?
        BEQ     exit_suf1               ;yes, so return infinity
        B       NaN_add                 ;no, so deal with NaN

same_suf

        MOV     r3, #0                  ;return zero
        MOV     r2, #0
        MOV     r1, #0

        B       exit_suf1

und_suf

        MOV     r3, #0                  ;return zero if trap disabled
        MOV     r2, #0
        MOV     r1, #0

        ADR     r7, exit_suf1           ;and cause exception if enabled
        MOV     r4, #UFL_E_MASK
        B       check_exep

denorm_suf

; Number too small for normalised no., so denormalise

        RSB     r3, r3, #0              ;shift = negated problem exponent

        RSBS    r4, r3, #32             ;is this greater than 32?

        SUBMI   r5, r3, #32             ;yes, so shift > 1 word
        MOVMI   r2, r1, LSR r5

        MOVPL   r2, r2, LSR r3          ;shift right by desired amount
        ORRPL   r2, r2, r1, LSL r4

        MOV     r1, r1, LSR r3

        MOV     r3, #0                  ;zero exponent
        B       exit_suf1               ;and go home...

;******************************************************************************
;
;       Multiply routines
;

; Multiply routines

muf3
fml3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

muf
fml
        MOVS    r8, r9, LSL #28         ;get bits indicating Fm
        BMI     constant_muf            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r8, r12, r8, LSR #24    ;point at fm store
        LDMIA   r8, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_muf       ;if not, convert

        STMIA   r8, {r0-r3}             ;write back in U type if converted
nocon_muf
        AND     r0, r0, #&80000000      ;tidy to just sign bit in r0
        ORR     r11, r3, r0             ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        MOV     r8, r1

got_Fm_muf

; We have loaded up fm, so now load fn and convert to U if necessary

        AND     r6, r9, #&00070000      ;get bits indicating fn,
        ADD     r6, r12, r6, LSR #12    ;point at fn store
        LDMIA   r6, {r0-r3}             ;and load it up

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_muf2      ;if not, convert

        STMIA   r6, {r0-r3}             ;write back in U type if converted
nocon_muf2
        EOR     r0, r0, r11             ;sign of result
        BIC     r11, r11, #&80000000    ;hence tidy up exponent

; Now check to see if either value is zero (0*infinity not allowed)

        MOV     r6, #&00020000          ;convenient bit
        CMP     r3, #0                  ;could fn be zero?
        CMPNE   r11, #0                 ;or fm be zero?
        BEQ     zero_muf                ;yes...

not_zero_muf

; Now check to see if either value is a NaN

        ADDS    r5, r6, r3, LSL #17     ;has fn got maximum exponent?
        ADDCCS  r5, r6, r11, LSL #17    ;if not, has fm got max. exponent?
        BCS     funny_add               ;yes, so either nan or infinity

; We have now got numbers ready to muf, so first deal with exponent

        MOVS    r5, #&ff, 8             ;put useful mask in r11, setting C
        ADC     r3, r3, r11             ;in order to add exponents +1 (2*bias)
        ORR     r11, r5, r5, LSR #8

        SUB     r3, r3, r11, LSR #18    ;correct exponent to 1 * bias

; Result sign and exponent now calculated, so multiply mantissae if necessary

        ORRS    r4, r10, r8, LSL #1     ;is b just a power of 2?
        ORRNES  r4, r2, r1, LSL #1      ;or is a just a power of 2?
        BEQ     leave_mant              ;yes, so no multiply needed...

; This is done, in order to take advantage of the 2 micron arm MUL instruction,
; by considering each operand as made up of 4 16 bit numbers, a1-a4 and b1-b4.
; These parts are all multiplied together (16 multiplies) and the 32 bit 
; results recombined (results p1-p16):
;
; The 128 bit answer:   | answ1 | answ2 | answ3 | answ4 |
;                       ---------------------------------
;                       |**p1***|**p3***|
;                           |**p2***|**p4***|
;                           |**p5***|**p7***|
;                               |**p6***|**p8***|
;                               |**p9***|**p11**|
;                                   |**p10**|**p12**|
;                                   |**p13**|**p15**|
;                                       |**p14**|**p16**|
;
; The unaligned parts are calculated and added up first, then rotated by 16
; bits, and then the aligned parts are calculated and added on.
;

        STMFD   r12!, {r0,r3,r9,r14}    ;workspace needed - bummer for speed

        MOV     r4, r1, LSR #16         ;b1 in r4
        BIC     r5, r1, r11             ;b2 in r5
        MOV     r6, r2, LSR #16         ;b3 in r6
        BIC     r7, r2, r11             ;b4 in r7

        BIC     r9, r8, r11             ;a2 in r9
        MOV     r8, r8, LSR #16         ;a1 in r8
        BIC     r11, r10, r11           ;a4 in r11
        MOV     r10, r10, LSR #16       ;a3 in r10

        mult    r3, r6, r11             ;p15 in r3
        mult    r2, r4, r11             ;p13 in r2
        mult    r1, r4, r9              ;p5 in r1

        mult    r0, r7, r10             ;p12 in r0
        ADDS    r3, r3, r0              ;so add p12 in r1-r3
        mult    r0, r5, r10             ;p10 in r0
        ADCS    r2, r2, r0              ;so add p10 in r1-r3
        mult    r0, r5, r8              ;p2 in r0
        ADCS    r1, r1, r0              ;so add p2 in r0-r3
        MOVCC   r14, #0                 ;move carry bit into r14
        MOVCS   r14, #&1

        mult    r0, r6, r9              ;p7 in r0
        ADDS    r2, r2, r0              ;so add p7 in r1-r3
        ADCS    r1, r1, #0
        ADC     r14, r14, #0

        mult    r0, r7, r8              ;p4 in r0
        ADDS    r2, r2, r0              ;so add p4 in r1-r3
        ADCS    r1, r1, #0
        ADC     r14, r14, #0

; We now have all the unaligned words added up, so rotate them and continue
; with the aligned ones

        ORR     r14, r14, r3, LSL #16   ;rotate answer right 16, using r14
        MOV     r3, r3, LSR #16
        ORR     r3, r3, r2, LSL #16
        MOV     r2, r2, LSR #16
        ORR     r2, r2, r1, LSL #16
        MOV     r1, r1, LSR #16
        ORR     r1, r1, r14, LSL #16
        BIC     r14, r14, #&ff

        mult    r0, r5, r11             ;p14 in r0
        mult    r11, r7, r11            ;p16 in r11 (a4 now discarded)
        ADDS    r11, r14, r11           ;so add p16 in answer
        ADCS    r3, r3, r0              ;so add p14 in answer
        mult    r0, r6, r8              ;p3 in r0
        ADCS    r2, r2, r0              ;so add p3 in answer
        mult    r0, r4, r8              ;p1 in r0
        ADC     r1, r1, r0              ;so add p1 in answer

        mult    r0, r4, r10             ;p9 in r0
        mult    r10, r6, r10            ;p11 in r10 (a3 now discarded)
        ADDS    r10, r10, r3            ;so add p11 in answer, rem in r10,r11
        ADCS    r2, r2, r0              ;so add p9 in answer
        ADC     r1, r1, #0

        mult    r0, r7, r9              ;p8 in r0
        ADDS    r10, r10, r0            ;so add p8 in answer
        mult    r0, r5, r9              ;p6 in r0
        ADCS    r2, r2, r0              ;so add p6 in answer
        ADCS    r1, r1, #0

        LDMFD   r12!, {r0,r3,r9,r14}    ;restore workspace regs

; Mantissae have now been multiplied ***********************************

        BPL     muf_round_norm          ;unnormalised, so re normalise

; No overflow, so truncate if necessary

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_muf            ;yes...

        ORRS    r11, r11, r10           ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines
        CMP     r3, #0
        BLE     und_suf

        ADR     r7, exit_muf1
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

muf_round_norm

; MS bit zero after multiply, so normalisation needed - usually by only 1 bit

        ADDS    r11, r11, r11           ;double mantissa
        ADCS    r10, r10, r10
        ADCS    r2, r2, r2
        ADCS    r1, r1, r1

        SUB     r3, r3, #1              ;decrement exponent

        BPL     muf_round_norm          ;and repeat if necessary

; Now deal with truncation as necessary

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_muf            ;yes...

        CMP     r3, #0                  ;has exponent underflowed?
        BLE     und_suf                 ;yes, so underflow...

        ORRS    r11, r11, r10           ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines

        ADR     r7, exit_muf1
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

no_trunc_muf

; Extended precision, so no truncation required - just round correctly

        ANDS    r5, r9, #&00000060      ;default rounding?
        BNE     n_d_rnd_muf             ;no...

        ORRS    r11, r11, r10, LSL #1   ;are all bits except ms. bit lost zero?
        ANDEQ   r10, r10, r2, LSL #31   ;if yes, round to even (IEEE)
        ADDS    r2, r2, r10, LSR #31    ;and perform round
return_muf
        ADCS    r1, r1, #0

        ADC     r3, r3, #0              ;if overflow, increment exponent
        MOVCS   r1, #&80000000          ;and restore mantissa

exit_muf

; Now we must make sure extended precision hasn't underflowed

        ADDS    r4, r3, #1              ;has exponent underflowed?
        BLE     denorm_muf              ;yes...
        MOVS    r4, r4, LSR #15         ;or overflowed?
        BNE     overflow_add            ;yes...

exit_muf1

; Register inexact event, store result and go home

        LDR     r5, [r12, #8*16]        ;get fpsr
        TST     r5, #INX_E_MASK         ;inexact event already?
        BEQ     register_inx            ;no, so register one...

exit_muf2
        AND     r9, r9, #&00007000      ;destination reg. no.
        ADD     r9, r12, r9, LSR #8     ;hence its address
        STMIA   r9, {r0-r3}             ;so save its contents

;        savebaseinr11                   ;and go home...
        exit0to15

leave_mant

; One of operands was just a power of 2, so don't multiply mantissae

        ORRS    r4, r10, r8, LSL #1     ;is it b that is power of 2?
        MOVNE   r2, r10                 ;no, it was a, so use b mantissa
        MOVNE   r1, r8

        SUB     r3, r3, #1              ;tidy exponent

        TST     r9, #&00080000          ;extended precision?
        BNE     exit_muf                ;yes, so just return answer

        CMP     r3, #0
        BLE     und_suf

        ADR     r7, exit_muf1
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

n_d_rnd_muf

; Round extended precision, but not default mode

        ORRS    r11, r11, r10           ;any remainder?
        CMPNE   r5, #&00000060
        BEQ     exit_muf                ;no, so exact number...

        TEQ     r0, r5, LSL #25         ;+ and round to + inf or - and -inf?
        BMI     exit_muf                ;no, so ignore remainder

        ADDS    r2, r2, #1              ;round up
        B       return_muf

denorm_muf

; Number too small for normalised no., so denormalise

        RSB     r3, r3, #0              ;shift = negated problem exponent

        CMP     r3, #64                 ;will we underflow completely?
        BGE     und_suf                 ;yes...

        RSBS    r4, r3, #32             ;is this greater than 32?

        SUBMI   r5, r3, #32             ;yes, so shift > 1 word
        MOVMI   r2, r1, LSR r5

        MOVPL   r2, r2, LSR r3          ;shift right by desired amount
        ORRPL   r2, r2, r1, LSL r4

        MOV     r1, r1, LSR r3

        MOV     r3, #0                  ;zero exponent
        B       exit_muf1               ;and go home...

zero_muf

; One of exponents was zero, so we may be multiplying by zero or denorm. no.

        CMP     r3, #0                  ;is fn the doubtful one?
        BNE     fm_prob_muf             ;no, so the problem is with fm

        ORRS    r5, r1, r2              ;is fn denormalised?
        BEQ     fn_zero_muf             ;no, so must be zero...

        CMP     r11, #0                 ;yes, but is fm possibly zero as well?
        BNE     not_zero_muf            ;no, so go back where we came from...

fm_prob_muf
        ORRS    r5, r8, r10             ;is fm denormalised?
        BNE     not_zero_muf            ;yes, so go home...

muf_by_0

; Fm is zero, so we must deal with infinite or NaN fn, or just return zero

        ADDS    r5, r6, r3, LSL #17     ;could fn be NaN or infinity?
        MOVCC   r3, #0                  ;no, so return zero
        MOVCC   r2, #0
        MOVCC   r1, #0
        BCC     exit_muf1

        ORRS    r5, r1, r2              ;is fn a NaN?
        BNE     NaN_add                 ;yes, so deal with it
muf_0inf
        MOV     r1, #muf0infNaN         ;0 * infinity, so return right NaN
        B       NaN_add

fn_zero_muf

; Fn is zero, so we must deal with infinite or NaN fm, or just return zero

        ADDS    r5, r6, r11, LSL #17    ;could fm be NaN or infinity?
        BCC     exit_muf1               ;no, so return zero

        ORRS    r5, r8, r10             ;is fm a NaN?
        BNE     funny_add               ;yes, so deal with it

        MOV     r3, r11                 ;no, so 0 * infinity...
        B       muf_0inf


;******************************************************************************
;
;       Divide routines (dead slow!)
;

frd3
rdf3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

frd
rdf

; First load fn, convert to U if necessary, and put in normal place for fm

        AND     r6, r9, #&00070000      ;get bits indicating fn,
        ADD     r6, r12, r6, LSR #12    ;point at fn store
        LDMIA   r6, {r0-r3}             ;and load it up

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_rdf       ;if not, convert

        STMIA   r6, {r0-r3}             ;write back in U type if converted
nocon_rdf
        AND     r0, r0, #&80000000      ;tidy to just sign bit in r0
        ORR     r11, r3, r0             ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        MOV     r8, r1

; We have got fn, so now load up fm

        MOVS    r6, r9, LSL #28         ;get bits indicating Fm
        BMI     constant_rdf            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r6, r12, r6, LSR #24    ;point at fm store
        LDMIA   r6, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   reverse_entry   ;if not, convert

        STMIA r6, {r0-r3}             ;write back in U type if converted

        B       reverse_entry           ;then go and do work


dvf3
fdv3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

dvf
fdv
        MOVS    r8, r9, LSL #28         ;get bits indicating Fm
        BMI     constant_dvf            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r8, r12, r8, LSR #24    ;point at fm store
        LDMIA   r8, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   nocon_dvf       ;if not, convert

        STMIA   r8, {r0-r3}             ;write back in U type if converted
nocon_dvf
        AND     r0, r0, #&80000000      ;tidy to just sign bit in r0
        ORR     r11, r3, r0             ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        MOV     r8, r1

got_Fm_dvf

; We have loaded up fm, so now load fn and convert to U if necessary

        AND     r6, r9, #&00070000      ;get bits indicating fn,
        ADD     r6, r12, r6, LSR #12    ;point at fn store
        LDMIA   r6, {r0-r3}             ;and load it up

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensure_U_fast   reverse_entry   ;if not, convert

        STMIA   r6, {r0-r3}             ;write back in U type if converted

got_Fm_rdf
reverse_entry
        EOR     r0, r0, r11             ;sign of result
        BIC     r11, r11, #&80000000    ;hence tidy up exponent

; Now check to see if either value is zero or denormalised or infinity or NaN

        MOVS    r4, r3, LSL #17         ;could fn be zero?
        MOVNES  r5, r11, LSL #17        ;or fm be zero?
        ADDNES  r4, r4, #&00020000      ;could fn be infinity or NaN?
        ADDNES  r5, r5, #&00020000      ;could fm be infinity or NaN?
        BEQ     funny_dvf               ;yes...

not_zero_dvf

; We have now got numbers ready to dvf, so first deal with exponent

        SUB     r3, r3, r11             ;calculate exponent
        ADD     r3, r3, #&4000          ;and correct bias
        SUB     r3, r3, #1

; Result sign and exponent now calculated, so divide mantissae

        MOV     r11, #0                 ;handy zero for later

        SUBS    r5, r2, r10             ;if num > den, subtract num from den,
        SBCS    r4, r1, r8

        MOVCS   r1, #3                  ;get normalised result by
        BCS     div_loop_ent            ;jumping to right place

        MOV     r5, r2                  ;otherwise copy numerator,
        MOV     r4, r1
        MOV     r1, #1                  ;calculating 32 more bits
        SUB     r3, r3, #1              ;and decrementing exponent

div_loop

; This is the first of 2 main loops doing the division of the mantissae
; It contains repetitive code to minimise branching

        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        ADC     r11, r11, #0            ;hence storing it in r11

        SUBS    r7, r5, r10             ;attempt to subtract denominator
        SBCS    r6, r4, r8
        MOVCCS  r11, r11, LSR #1        ;set C if numerator > denominator

        MOVCS   r5, r7                  ;move in subtracted value if OK
        MOVCS   r4, r6
        ADC     r1, r1, r1              ;and insert C in result and rotate

div_loop_ent
        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        ADC     r11, r11, #0            ;hence storing it in r11

        SUBS    r7, r5, r10             ;attempt to subtract denominator
        SBCS    r6, r4, r8
        MOVCCS  r11, r11, LSR #1        ;set C if numerator > denominator

        MOVCS   r5, r7                  ;move in subtracted value if OK
        MOVCS   r4, r6
        ADC     r1, r1, r1              ;and insert C in result and rotate


        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        ADC     r11, r11, #0            ;hence storing it in r11

        SUBS    r7, r5, r10             ;attempt to subtract denominator
        SBCS    r6, r4, r8
        MOVCCS  r11, r11, LSR #1        ;set C if numerator > denominator

        MOVCS   r5, r7                  ;move in subtracted value if OK
        MOVCS   r4, r6
        ADC     r1, r1, r1              ;and insert C in result and rotate


        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        ADC     r11, r11, #0            ;hence storing it in r11

        SUBS    r7, r5, r10             ;attempt to subtract denominator
        SBCS    r6, r4, r8
        MOVCCS  r11, r11, LSR #1        ;set C if numerator > denominator

        MOVCS   r5, r7                  ;move in subtracted value if OK
        MOVCS   r4, r6
        ADCS    r1, r1, r1              ;and insert C in result and rotate


        BCC     div_loop                ;and loop again

; We have produced 32 bits of result. If exact result or single precision
; wanted, this is enough, so next deal with these possibilities.

        ORRS    r2, r4, r5              ;is remainder zero?

        ORRNE   r2, r9, r9, LSR #12     ;is it single precision?
        ANDNES  r2, r2, #&00000080
        BEQ     divided                 ;yes, so 32 bits will do (r2=0)

        MOV     r2, #1                  ;initialise lower word of result

div_loop2

; This is the second of 2 main loops doing the division of the mantissae
; It contains repetitive code to minimise branching

        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        ADC     r11, r11, #0            ;hence storing it in r11

        SUBS    r7, r5, r10             ;attempt to subtract denominator
        SBCS    r6, r4, r8
        MOVCCS  r11, r11, LSR #1        ;set C if numerator > denominator

        MOVCS   r5, r7                  ;move in subtracted value if OK
        MOVCS   r4, r6
        ADC     r2, r2, r2              ;and insert C in result and rotate


        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        ADC     r11, r11, #0            ;hence storing it in r11

        SUBS    r7, r5, r10             ;attempt to subtract denominator
        SBCS    r6, r4, r8
        MOVCCS  r11, r11, LSR #1        ;set C if numerator > denominator

        MOVCS   r5, r7                  ;move in subtracted value if OK
        MOVCS   r4, r6
        ADC     r2, r2, r2              ;and insert C in result and rotate


        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        ADC     r11, r11, #0            ;hence storing it in r11

        SUBS    r7, r5, r10             ;attempt to subtract denominator
        SBCS    r6, r4, r8
        MOVCCS  r11, r11, LSR #1        ;set C if numerator > denominator

        MOVCS   r5, r7                  ;move in subtracted value if OK
        MOVCS   r4, r6
        ADC     r2, r2, r2              ;and insert C in result and rotate


        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        ADC     r11, r11, #0            ;hence storing it in r11

        SUBS    r7, r5, r10             ;attempt to subtract denominator
        SBCS    r6, r4, r8
        MOVCCS  r11, r11, LSR #1        ;set C if numerator > denominator

        MOVCS   r5, r7                  ;move in subtracted value if OK
        MOVCS   r4, r6
        ADCS    r2, r2, r2              ;and insert C in result and rotate


        BCC     div_loop2               ;and loop again

divided

; We have produced a 64 (or 32 if S precision) bit result

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_dvf            ;yes...

        ORRS    r4, r5, r4              ;any remainder?
        ORRNE   r2, r2, #1              ;yes, so trick truncation routines
        CMP     r3, #0
        BLE     und_suf

        ADR     r7, exit_dvf1
        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

no_trunc_dvf

; Extended precision, so no truncation required - just round correctly

        ANDS    r7, r9, #&00000060      ;default rounding?
        BNE     n_d_rnd_dvf             ;no...

        ADDS    r5, r5, r5              ;double numerator
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        BCS     num_biggerR             ;hence denominator can be subtracted

        CMP     r4, r8                  ;numerator >= denominator?
        CMPEQ   r5, r10
        TSTEQ   r5, r2, LSR #1          ;if equal, round to even (IEEE) with C

num_biggerR
        ADCS    r2, r2, #0              ;add in carry to do rounding
return_dvf
        ADCS    r1, r1, #0

        ADC     r3, r3, #0              ;if overflow, increment exponent
        MOVCS   r1, #&80000000          ;and restore mantissa

exit_dvf

; Now we must make sure extended precision hasn't underflowed

        ADDS    r4, r3, #1              ;has exponent underflowed?
        BLE     denorm_muf              ;yes...
        MOVS    r4, r4, LSR #15         ;or overflowed?
        BNE     overflow_add            ;yes...

exit_dvf1

; Register inexact event, store result and go home

        LDR     r5, [r12, #8*16]        ;get fpsr
        TST     r5, #INX_E_MASK         ;inexact event already?
        BEQ     register_inx            ;no, so register one...

exit_dvf2
        AND     r9, r9, #&00007000      ;destination reg. no.
        ADD     r9, r12, r9, LSR #8     ;hence its address
        STMIA   r9, {r0-r3}             ;so save its contents

;        savebaseinr11                   ;and go home...
        exit0to15

n_d_rnd_dvf

; Round extended precision, but not default mode

        ORRS    r5, r4, r5              ;any remainder?
        CMPNE   r7, #&00000060
        BEQ     exit_dvf                ;no, so exact number...

        TEQ     r0, r7, LSL #25         ;+ and round to + inf or - and -inf?
        BMI     exit_dvf                ;no, so ignore remainder

        ADDS    r2, r2, #1              ;round up
        B       return_dvf

funny_dvf

; We have max or min exponent - infinity, NaN, zero or denormalised

        MOVS    r4, r3, LSL #17         ;could fn be zero or denormalised?
        BNE     fn_not_zero             ;no...

        ORRS    r5, r1, r2              ;fn denormalised?
        BNE     normalise_fn            ;yes, so normalise...

; Fn is zero, so check fm for zero or NaN

        MOVS    r5, r11, LSL #17        ;is fm zero?
        ORREQS  r6, r8, r10

        MOVEQ   r1, #a0div0NaN          ;yes, so return suitable NaN
        MOVEQ   r3, #&ff
        ORREQ   r3, r3, r3, LSL #7
        BEQ     NaN_add

        ADDS    r5, r5, #&00020000      ;could fm be a NaN?
        BNE     exit_dvf2               ;no, so return 0
        ORRS    r6, r8, r10             ;is fm infinity?
        BEQ     exit_dvf2               ;yes, so just return 0

        MOV     r1, r8                  ;fm is a NaN, so return fm
        MOV     r2, r10
        MOV     r3, r11
        B       NaN_add

normalise_fn

; Fn is denormalised, so normalise so that division will work

        ADDS    r2, r2, r2              ;double mantissa
        ADCS    r1, r1, r1
        SUB     r3, r3, #1              ;decrement exponent
        BPL     normalise_fn            ;and repeat as necessary

fn_not_zero

        MOVS    r5, r11, LSL #17        ;could fm be zero or denormalised?
        BNE     fm_not_zero             ;no...

        ORRS    r6, r8, r10             ;fm denormalised?
        BNE     normalise_fm            ;yes, so normalise

dvf_by_0

; Fm is zero, so unless fn is a NaN, return DVZ

        ADDS    r4, r4, #&00020000      ;has fn got max exponent?
        BNE     dvz                     ;no...

        ORRS    r6, r1, r2              ;hence is fn a NaN?
        BNE     NaN_add                 ;yes, so return that...

dvz

; Divide by zero - if untrapped, return correct infinity

        MOV     r1, #0                  ;put infinity in r0-r3
        MOV     r2, #0
        MOV     r3, #&ff
        ORR     r3, r3, r3, LSL #7

        ADR     r7, exit_dvf2
        MOV     r4, #DVZ_E_MASK
        B       check_exep

normalise_fm

; Fm denormalised, so normalise it to make division work

        ADDS    r10, r10, r10           ;double mantissa
        ADCS    r8, r8, r8
        SUB     r11, r11, #1            ;decrement exponent
        BPL     normalise_fm            ;and carry on as required

fm_not_zero

; Neither fn or fm is zero or denormalised, but deal with infinity or NaN

        ADDS    r6, r5, #&00020000      ;could fn or fm be infinity or NaN?
        ADDNES  r6, r4, #&00020000
        BNE     not_zero_dvf            ;no, so go back to action

        ADDS    r6, r4, #&00020000      ;was it fn?
        BNE     fm_trouble_dvf          ;no, so fm must be trouble

        ORRS    r6, r1, r2              ;is fn infinity?
        BNE     NaN_add                 ;no, so must be NaN...

; Fn is infinity - OK unless fm infinity or NaN as well

        ADDS    r6, r5, #&00020000      ;is fm troublesome?
        BNE     exit_dvf1               ;no, so return fn

        ORRS    r6, r8, r10             ;is fm infinity?
        MOVEQ   r1, #infdivinfNaN       ;yes, so return correct NaN
        BEQ     NaN_add

fm_trouble_dvf

        ORRS    r6, r8, r10             ;is fm a NaN?
        BEQ     und_suf                 ;no - infinity - so underflow

        MOV     r1, r8                  ;yes, so return this NaN
        MOV     r2, r10
        MOV     r3, r11
        B       NaN_add

;******************************************************************************
;
;       Power routines (using LGN and EXP routines in file FPE2logs)
;


rpw3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

rpw

; First load fn, convert to U if necessary, and put in normal place for fm

        AND     r6, r9, #&00070000      ;get bits indicating fn,
        ADD     r6, r12, r6, LSR #12    ;point at fn store
        LDMIA   r6, {r0-r3}             ;and load it up

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r6, {r0-r3}             ;write back in U type if converted

        AND     r0, r0, #&80000000      ;tidy to just sign bit in r0
        ORR     r11, r3, r0             ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        MOV     r8, r1

; We have got fn, so now load up fm

        MOVS    r6, r9, LSL #28         ;get bits indicating Fm
        BMI     constant_rpw            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r6, r12, r6, LSR #24    ;point at fm store
        LDMIA   r6, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r6, {r0-r3}             ;write back in U type if converted

        B       rev_entry               ;then go and do work


pow3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

pow
        MOVS    r8, r9, LSL #28         ;get bits indicating Fm
        BMI     constant_pow            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r8, r12, r8, LSR #24    ;point at fm store
        LDMIA   r8, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r8, {r0-r3}             ;write back in U type if converted

        AND     r0, r0, #&80000000      ;tidy to just sign bit in r0
        ORR     r11, r3, r0             ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        MOV     r8, r1

got_Fm_pow

; We have loaded up fm, so now load fn and convert to U if necessary

        AND     r6, r9, #&00070000      ;get bits indicating fn,
        ADD     r6, r12, r6, LSR #12    ;point at fn store
        LDMIA   r6, {r0-r3}             ;and load it up

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r6, {r0-r3}             ;write back in U type if converted

got_Fm_rpw
rev_entry

; Retrieve sign of b again

        MOV     r7, r11
        BIC     r11, r11, #&80000000

; Now check to see if either value is zero or denormalised or infinity or NaN

        MOVS    r4, r3, LSL #17         ;could fn be zero?
        ADDNES  r4, r4, #&00020000      ;could fn be infinity or NaN?
        MOVNES  r5, r11, LSL #17        ;or fm be zero?
        ADDNES  r5, r5, #&00020000      ;could fm be infinity or NaN?
        BEQ     funny_pow               ;yes...

not_zero_pow

; We now have arguaments loaded - answer = a^b. First decide if integer power.

        STMDB   r12!, {r9,r14}          ;stack away needed regs.

        BPL     not_int_pow             ;too small to be integer (as not zero)

        SUB     w1, b_exp, #&3f00       ;number of bits left
        SUB     w1, w1, #&fe

        CMP     w1, #16                 ;have we more than 16 bits left?
        BGT     large_pow               ;yes - may be integer, but too big for
                                        ;integer bodge
        ORRS    w2, b2, b1, LSL w1      ;is it an integer?
        BNE     not_int_pow             ;no...

; We have an integer power, where integer can fit in 16 bits

        RSB     w5, w1, #32
        MOV     w5, b1, LSR w5          ;integer in w5
        AND     a_sign, a_sign, w5, LSL #31 ;negative OK for integer power

        STMDB   r12!, {b_sign}          ;remember if negative power

        MOV     b1, #&80000000          ;put 1.0 in b
        MOV     b2, #0
        MOV     b_sign, #&4000
        SUBS    b_exp, b_sign, #1       ;setting C flag also
        STMDB   r12!, {b_sign,b1,b2,b_exp} ;then put 1.0 on top of stack

clear_bit
        AisAsquared     CC              ;square running factor/drop through

int_pow_loop
        MOVS    w5, w5, LSR #1          ;test bit in power integer
        BCC     clear_bit               ;clear bit, so leave total alone

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;bit set, so retrieve total,
        STMNEDB r12!, {a_sign,a1,a2,a_exp} ;save running factor if needed again

        AisAtimesB                      ;total = total * running factor

        ADD     w1, a_exp, #1

        BEQ     result_in_a             ;calculation done...

        CMP     w1, #&8000              ;has intermediate calculation overf.?
        BGE     overflow_pow            ;yes...

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve running factor
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save total

        MOV     a_sign, b_sign          ;a=b
        MOV     a1, b1
        MOV     a2, b2
        MOV     a_exp, b_exp
        AisAtimesB                      ;hence a=b^2

        B       int_pow_loop            ;and carry on...

result_in_a

; We have the result of an integer power in a - first check for overflow, then
; tidy stack and return result.

        LDMIA   r12!, {b_sign}          ;was power negative?
        TST     b_sign, #&80000000
        AisAinverted    NE              ;yes, so invert answer

        LDMIA   r12!, {r9,r14}          ;retrieve important regs.
        ADDS    w1, a_exp, #1           ;has underflow occurred?
        BLE     denorm_muf              ;yes...
        MOVS    w1, w1, LSR #15         ;has overflow occurred?
        BNE     overflow_add            ;yes...

        B       exit_log                ;and retrun result

overflow_pow

        ADD     r12, r12, #4*4          ;tidy stack
        LDMIA   r12!, {b_sign,r9,r14}   ;retrieve important regs.
        TST     b_sign, #&80000000
        BNE     und_suf                 ;if underflow, deal with this
        B       overflow_add            ;and use another handy routine

large_pow

; We are not going to do the integer bodge, but if a is a large integer, we
; must allow negative sign (propagated if integer power is odd).

        CMP     w1, #64                 ;have we a very big exponent?
        BCS     v_v_large_int           ;yes - so big we must have integer

        SUBS    w2, w1, #32             ;no. bits to left of point -32
        MOVCSS  w3, b2, LSL w2          ;all the rest zero? (C=last bit lost)
        BEQ     v_large_int             ;yes, so we have an integer

        ORRS    w3, b2, b1, LSL w1      ;all bits to right of point zero?
        BEQ     large_int               ;yes, so we have an integer

not_int_pow

; We don't have an integer power, so negative a implies complex (bad) answer

        TST     a_sign, #&80000000      ;is a negative?
        BNE     complex_pow             ;yes, oh dear...

calc_pow

; Now calculate a = a^b using LGN and EXP

        STMDB   r12!, {a_sign}          ;save sign of answer
        MOV     a_sign, #0              ;and clear sign for calculations

        STMDB   r12!, {b_sign,b1,b2,b_exp} ;save power

        BIC     r9, r9, #&00100000      ;make sure LGN not LOG!
        BL      log_rout                ;a = LGN(a)
    
        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve power
        AisAtimesB                      ;a = b*LGN(a)

        BL      exp_rout                ;a= EXP(b*LGN(a)), Z clear = trouble

        LDMIA   r12!, {a_sign,r9,r14}   ;retrieve sign, r9, r14

        BEQ     exit_log                ;if no trouble, return result

; We have overflow or underflow, so deal with this

        ADR     r7, exit_log1
        B       check_exep              ;check for exceptions.

v_v_large_int

        MOVNES  w2, #0,2                ;if >65 bits, even power, so
                                        ;block sign propagation (clear C)

v_large_int
large_int

        MOVCC   a_sign, #0              ;change sign to positive if C clear
        B       calc_pow                ;then go and do job...

complex_pow

; We have a complex result - illegal - so return NaN

        LDMIA   r12!, {r9,r14}          ;tidy stack
        MOV     a1, #badpowNaN
        MOV     a2, #0
        MOV     a_exp, #&ff
        ORR     a_exp, a_exp, a_exp, LSL #7
        B       NaN_add

funny_pow

; We have a max. or min. exponent - a zero, denormalised, infinity or NaN

        CMP     r4, #0                  ;is a causing trouble?
        BNE     b_trouble               ;no, so must be b...

        CMP     r3, #0                  ;max. or min exponent?
        BEQ     min_exp_a               ;min...

        ORRS    r6, r1, r2              ;is a a NaN?
        BNE     NaN_add                 ;yes...
        B       a_infinite              ;no, a is infinity

min_exp_a

        ORRS    r6, r1, r2              ;is a denormalised?
        BNE     denorm_a                ;yes...

a_infinite

; A is infinite or zero - return this unless b is a NaN or zero

        MOVS    r5, r11, LSL #17        ;is b zero or denorm.?
        BNE     max_exp_b               ;no, so check for NaN...

        ORRS    r6, r8, r10             ;is b zero?
return_1_pow
        MOVEQ   r1, #&80000000          ;yes, so return 1.0
        MOVEQ   r2, #0
        MOVEQ   r0, #0
        MOVEQ   r3, #&4000
        SUBEQ   r3, r3, #1
        B       exit_log

max_exp_b
        ADDS    r5, r5, #&00020000      ;has b max. exponent?
        BNE     exit_log                ;no, so can't be a NaN...

        ORRS    r6, r8, r10             ;is b a NaN?
        BEQ     exit_log                ;no, infinity, so return a

        MOV     r1, r8                  ;yes, so return NaN
        MOV     r2, r10
        MOV     r3, r11
        B       NaN_add

denorm_a

; We have a denormalised a - normalise it, then pretend a is normal

        ADDS    r2, r2, r2              ;double mantissa
        ADCS    r1, r1, r1

        SUB     r3, r3, #1              ;decrement exponent
        BPL     denorm_a                ;and repeat if necessary...

        MOVS    r5, r11, LSL #17        ;is b a funny?
        ADDNES  r5, r5, #&00020000
        BNE     not_zero_pow            ;no, so go back to work...

b_trouble

; We have a b with max. or min. exponent

        CMP     r11, #0                 ;is b_exp zero?
        BEQ     min_exp_b               ;yes...

        MOV     r1, r8                  ;copy b to a
        MOV     r2, r10
        MOV     r0, r7           
        MOV     r3, r11

        ORRS    r6, r8, r10             ;is b a NaN?
        BNE     NaN_add                 ;yes, so return that

        TST     r0, #&80000000          ;is b -infinity?
        MOVNE   r3, #0                  ;yes, so return 0
        B       exit_log

min_exp_b

; b is either zero or denormalised

        ORRS    r6, r8, r10             ;is b zero?
        BEQ     return_1_pow            ;yes, so return 1...

denorm_b

        ADDS    r10, r10, r10           ;double mantissa
        ADCS    r8, r8, r8

        SUB     r11, r11, #1            ;decrement exponent
        BPL     denorm_b

        STMDB   r12!, {r9,r14}          ;set up stack
        B       not_int_pow             ;and jump to appropriate place

;******************************************************************************
;
;       RMF remainder routines
;

rmf3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

rmf
        MOVS    r8, r9, LSL #28         ;get bits indicating Fm
        BMI     constant_rmf            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r8, r12, r8, LSR #24    ;point at fm store
        LDMIA   r8, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r8, {r0-r3}             ;write back in U type if converted

        AND     r0, r0, #&80000000      ;tidy to just sign bit in r0
        ORR     r11, r3, r0             ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        MOV     r8, r1

got_Fm_rmf

; We have loaded up fm, so now load fn and convert to U if necessary

        AND     r6, r9, #&00070000      ;get bits indicating fn,
        ADD     r6, r12, r6, LSR #12    ;point at fn store
        LDMIA   r6, {r0-r3}             ;and load it up

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r6, {r0-r3}             ;write back in U type if converted

; Retrieve sign of b again

        MOV     r7, r11
        BIC     r11, r11, #&80000000

; Now check to see if either value is zero or denormalised or infinity or NaN

        MOVS    r4, r3, LSL #17         ;could fn be zero?
        ADDNES  r4, r4, #&00020000      ;could fn be infinity or NaN?
        MOVNES  r5, r11, LSL #17        ;or fm be zero?
        ADDNES  r5, r5, #&00020000      ;could fm be infinity or NaN?
        BEQ     funny_rmf               ;yes...

not_zero_rmf

        STMDB   r12!, {a_sign,a1,a2,a_exp,r14} ;save a,lr
        STMDB   r12!, {b_sign,b1,b2,b_exp} ;and save b

        AisAoverB                       ;do division
        AisrndA                         ;round to integer

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve b
        AisAtimesB                      ;form amount to be subtracted

        EOR     a_sign, a_sign, #&80000000 ;negate it,
        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;get original operand
        AisAplussignB                   ;and subtract from it to get remainder

        LDMIA   r12!, {r14}             ;retrieve lr

        B       exit_log                ;and return result

funny_rmf

; We have max or min exponent - infinity, NaN, zero or denormalised

        MOVS    r4, r3, LSL #17         ;could a be zero or denormalised?
        BNE     a_not_zero_r            ;no...

        ORRS    r5, r1, r2              ;a denormalised?
        BNE     normalise_a_r           ;yes, so normalise...

; a is zero, so check b for zero or NaN

        MOVS    r5, r11, LSL #17        ;is b zero?
        ORREQS  r6, r8, r10

        BEQ     bad_rmf                 ;yes, 0/0, so object

        ADDS    r5, r5, #&00020000      ;could b be a NaN?
        BNE     underflow_rmf           ;no, so division underflows
        ORRS    r6, r8, r10             ;is b infinity?
        BEQ     underflow_rmf           ;yes, so division underflows

        MOV     r1, r8                  ;b is a NaN, so return b
        MOV     r2, r10
        MOV     r3, r11
        B       NaN_add

normalise_a_r

; a is denormalised, so normalise so that division will work

        ADDS    r2, r2, r2              ;double mantissa
        ADCS    r1, r1, r1
        SUB     r3, r3, #1              ;decrement exponent
        BPL     normalise_a_r           ;and repeat as necessary

a_not_zero_r

        MOVS    r5, r11, LSL #17        ;could b be zero or denormalised?
        BNE     b_not_zero_r            ;no...

        ORRS    r6, r8, r10             ;b denormalised?
        BNE     normalise_b_r           ;yes, so normalise

; b is zero, so unless a is a NaN, return badrmfNaN

        ADDS    r4, r4, #&00020000      ;has a got max exponent?
        BNE     bad_rmf                 ;no...

        ORRS    r6, r1, r2              ;hence is a a NaN?
        BNE     NaN_add                 ;yes, so return that...

bad_rmf

; We have an error in the division part of rmf, so return NaN

        MOV     a1, #badrmfNaN          ;return NaN
        MOVS    a2, #0,2
        RSC     a_exp, a2, #&8000
        B       NaN_add

underflow_rmf

; return zero

        MOV     a_sign, #0
        MOV     a1, #0
        MOV     a2, #0
        MOV     a_exp, #0
        B       exit_log

normalise_b_r

; b denormalised, so normalise it to make division work

        ADDS    r10, r10, r10           ;double mantissa
        ADCS    r8, r8, r8
        SUB     r11, r11, #1            ;decrement exponent
        BPL     normalise_b_r           ;and carry on as required

b_not_zero_r

; Neither a or b is zero or denormalised, but deal with infinity or NaN

        ADDS    r6, r5, #&00020000      ;could a or b be infinity or NaN?
        ADDNES  r6, r4, #&00020000
        BNE     not_zero_rmf            ;no, so go back to action

        ADDS    r6, r4, #&00020000      ;was it a?
        BNE     b_trouble_rmf           ;no, so b must be trouble

        ORRS    r6, r1, r2              ;is a infinity?
        BNE     NaN_add                 ;no, NaN, so return this
        MOVEQ   r1, #badrmfNaN          ;yes, so return badrmfNaN
        ADDS    r6, r5, #&00020000      ;unless b could be a NaN
        BNE     NaN_add                 ;no, so return NaN in a...
        ORRS    r6, r8, r10             ;is b a NaN?
        MOVNE   r1, r8                  ;yes, so return that instead
        MOVNE   r2, r10
        B       NaN_add

b_trouble_rmf

        ORRS    r6, r8, r10             ;is b a NaN?
        BEQ     exit_log                ;no - infinity - so return a

        MOV     r1, r8                  ;yes, so return this NaN
        MOV     r2, r10
        MOV     r3, r11
        B       NaN_add

;******************************************************************************

filelevel       SETA    filelevel - 1
        END
