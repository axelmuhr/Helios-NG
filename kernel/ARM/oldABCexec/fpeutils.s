; -> fpeutils/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2utils       2/2/86   Martin Clemoes
;
;       Description:    This file contains low level subroutines to perform
;                       addition, subtraction, multiplication and division
;                       for use in trig, log, etc routines (not ADF, SUF, DVF
;                       and MUF, however, as these have in-line routines).
;
;******************************************************************************
;
;       Some register names to help with programming at higher levels than
;       this file (in my view, they obscure algorithms at this level)
;

a_sign  RN      r0
a1      RN      r1
a2      RN      r2
a_exp   RN      r3

b_sign  RN      r7
b1      RN      r8
b2      RN      r10
b_exp   RN      r11

w1      RN      r4
w2      RN      r5
w3      RN      r6
w4      RN      b_sign
w5      RN      r9      ;note: convention is to use this reg. for fp instruc.

;******************************************************************************
;
;       Macro to allow MUL instruction to be emulated on 3 micron arm
;

        MACRO
        mult    $destreg, $reg1, $reg2  ;note r12, r14 cannot be destreg

        [       arm2
        MUL     $destreg, $reg1, $reg2  ;quick and easy!
        |

        STMFD   r12!, {$reg1,$reg2}     ;save values to multiply
        STMFD   r12!, {r0-r2,r14}       ;workspace and lr

        BL      mult_16bit              ;do multiply, result in r0

        STR     r0, [r12, #16]          ;insert result on stack
        LDMFD   r12!, {r0-r2,r14}       ;restore workspace
        LDMFD   r12!, {$destreg}        ;load result
        ADD     r12, r12, #4            ;tidy stack
        ]

        MEND

; Subroutine to do 16 bit multiply, only used if 3 micron arm

        [       arm2
        |
mult_16bit

; Rips 2 16 bit values off stack, returns result in r0, cond codes unchanged

        LDR     r2, [r12, #20]          ;get 2 16 bit values to multiply
        LDR     r1, [r12, #16]

        MOV     r0, #0                  ;initialise result

        MOVS    r1, r1, LSL #17         ;test bits 15 and 14
        ADDCS   r0, r0, r2, LSL #15     ;if bit 15 set, add as app.
        MOVEQS  r15, r14                ;if rest zero, go home
        ADDMI   r0, r0, r2, LSL #14     ;deal with next bit

        MOVS    r1, r1, LSL #2          ;test next two bits
        ADDCS   r0, r0, r2, LSL #13
        ADDMI   r0, r0, r2, LSL #12

        MOVS    r1, r1, LSL #2          ;test next two bits
        ADDCS   r0, r0, r2, LSL #11     ;if bit 11 set, add as app.
        MOVEQS  r15, r14                ;if rest zero, go home
        ADDMI   r0, r0, r2, LSL #10     ;deal with next bit

        MOVS    r1, r1, LSL #2          ;test next two bits
        ADDCS   r0, r0, r2, LSL #9
        ADDMI   r0, r0, r2, LSL #8

        MOVS    r1, r1, LSL #2          ;test next two bits
        ADDCS   r0, r0, r2, LSL #7      ;if bit 7 set, add as app.
        MOVEQS  r15, r14                ;if rest zero, go home
        ADDMI   r0, r0, r2, LSL #6      ;deal with next bit

        MOVS    r1, r1, LSL #2          ;test next two bits
        ADDCS   r0, r0, r2, LSL #5
        ADDMI   r0, r0, r2, LSL #4

        MOVS    r1, r1, LSL #2          ;test next two bits
        ADDCS   r0, r0, r2, LSL #3      ;if bit 3 set, add as app.
        MOVEQS  r15, r14                ;if rest zero, go home
        ADDMI   r0, r0, r2, LSL #2      ;deal with next bit

        MOVS    r1, r1, LSL #2          ;test next two bits
        ADDCS   r0, r0, r2, LSL #1
        ADDMI   r0, r0, r2

        MOVS    r15, r14                ;go home...

        ]

;******************************************************************************
;
;       Add and subtract routines (interlinked for obvious reasons)
;

        MACRO
        AisAplusB       $cond
        BL$cond add_rout
        MEND

        MACRO
        AisAplussignB   $cond
        BL$cond signadd_rout
        MEND

signadd_rout

; Add a signed b to a (add_rout assumes positive b)

        TEQ     a_sign, b_sign          ;are signs the same?
        BPL     add_entry_r             ;yes, so add
        B       sub_entry_r

add_rout

        TST     a_sign, #&80000000      ;are signs different?
        BNE     sub_entry_r             ;yes, so subtract...

add_entry_r

; We have now got numbers ready to add, so first make sure bigger in a

        SUBS    w1, a_exp, b_exp        ;difference in exponents
        BMI     b_bigger_add            ;negative, so swap round

; We have smaller no. mantissa in b1 and b2, so shift and add

        RSBS    w2, w1, #32             ;is shift > 1 word?
        BLE     big_sh_add_r            ;yes...

        ADDS    a2, a2, b2, LSR w1      ;add shifted smaller no.
        ADDCS   a2, a2, b1, LSL w2
        ADDCCS  a2, a2, b1, LSL w2      ;taking care to get C right

        ADCS    a1, a1, b1, LSR w1

        MOV     w1, b2, LSL w2          ;w1 = ms. word lost

        BCS     over_add_r

        ADDS    a2, a2, w1, LSR #31     ;round
        ADCS    a1, a1, #0
        MOVCCS  r15, r14                ;and go home...

        MOV     a1, #&80000000          ;overflow due to round, so correct
        ADD     a_exp, a_exp, #1
        MOV     w1, w1, LSR #1
        MOVS    r15, r14

over_add_r

; We have an overflow, so shift everything right 1 to insert carry

        ADD     a_exp,a_exp,#1          ;increment exponent

        MOV     w1, w1, LSR #1          ;and rotate mantissa 1 right
        ORR     w1, w1, a2, LSL #31
        MOV     a2, a2, LSR #1
        ORR     a2, a2, a1, LSL #31
        MOV     a1, a1, RRX

        ADDS    a2, a2, w1, LSR #31     ;round
        ADC     a1, a1, #0
        MOVS    r15, r14                ;and go home...

huge_sh_add_r

; A shift of 64 or more bits - totally lost

        MOV     w1, b1, LSR w2          ;set up remainder

        ADDS    a2, a2, w1, LSR #31     ;round
        ADCS    a1, a1, #0

        MOVCS   a1, #&80000000          ;if overflow, restore mantissa
        ADDCS   a_exp, a_exp, #1        ;and increment exponent
        MOVS    r15, r14                ;and go home...

big_sh_add_r

; A shift of 32 bits or more needed - first deal with shift >63 - totally lost

        SUBS    w2, w1, #64             ;will smaller be totally shifted away?
        BGE     huge_sh_add_r           ;yes...

; A shift of 32 - 63 bits - some adding to do

        SUB     w1, w1, #32
        RSB     w2, w1, #32

        ADDS    a2, a2, b1, LSR w1      ;do addition
        ADCS    a1, a1, #0

        MOV     w1, b1, LSL w2          ;set up ms. bit lost
        ORR     w1, w1, b2, LSR w1

        BCS     over_add_r

        ADDS    a2, a2, w1, LSR #31     ;round
        ADCS    a1, a1, #0
        MOVCCS  r15, r14                ;and go home...

        MOV     a1, #&80000000          ;overflow due to round, so correct
        ADD     a_exp, a_exp, #1
        MOV     w1, w1, LSR #1
        MOVS    r15, r14

b_bigger_add

; b is bigger, so we'll have to swap them round (compactness v. speed)

        RSB     w1, w1, #0              ;negate w1

        MOV     a_exp,b_exp             ;move exponent

        MOV     w2, a1                  ;move mantissa msw.
        MOV     a1, b1
        MOV     b1, w2

        MOV     w2, a2                  ;move mantissa lsw.
        MOV     a2, b2
        MOV     b2, w2

; We have smaller no. mantissa in b1 and b2, so shift and add

        RSBS    w2, w1, #32             ;is shift > 1 word?
        BLE     big_sh_add_r            ;yes...

        ADDS    a2, a2, b2, LSR w1      ;add shifted smaller no.
        ADDCS   a2, a2, b1, LSL w2
        ADDCCS  a2, a2, b1, LSL w2      ;taking care to get C right

        ADCS    a1, a1, b1, LSR w1

        MOV     w1, b2, LSL w2          ;w1 = ms. word lost

        BCS     over_add_r

        ADDS    a2, a2, w1, LSR #31     ;round
        ADCS    a1, a1, #0
        MOVCCS  r15, r14                ;and go home...

        MOV     a1, #&80000000          ;overflow due to round, so correct
        ADD     a_exp, a_exp, #1
        MOV     w1, w1, LSR #1
        MOVS    r15, r14

; Subtract routines

        MACRO
        AisAminusB      $cond
        BL$cond sub_rout
        MEND

sub_rout

        TST     a_sign, #&80000000      ;are signs different?
        BNE     add_entry_r             ;yes, so add...

sub_entry_r

; We have now got numbers ready to sub, so first make sure bigger in a

        SUBS    w3, a_exp, b_exp        ;difference in exponents
        BMI     b_bigger_sub            ;negative, so swap round
        CMPEQ   a1, b1
        CMPEQ   a2, b2
        BEQ     same_sub                ;equal, so return zero
        BCC     b_bigger_sub            ;negative, so swap round

; We have smaller no. mantissa in b1 and b2, so shift and sub

        MOV     w1, #0                  ;create 1 word extension for result

        RSBS    w2, w3, #32             ;is shift > 1 word?
        BLE     big_sh_sub              ;yes...

        SUBS    w1, w1, b2, LSL w2      ;w1 now contains remainder
        SBCS    a2, a2, b2, LSR w3      ;now calculate result
        SUBCC   a2, a2, b1, LSL w2
        SUBCSS  a2, a2, b1, LSL w2

        SBCS    a1, a1, b1, LSR w3

        BPL     sub_round_norm1         ;if not normalised, normalise

        ADDS    a2, a2, w1, LSR #31     ;round
        ADC     a1, a1, #0
        MOVS    r15, r14                ;and go home...

sub_round_norm1

; MS bit zero after subtract, so normalisation needed - usually by only 1 bit

        ADDS    w1, w1, w1              ;double mantissa
        ADCS    a2, a2, a2
        ADCS    a1, a1, a1

        SUB     a_exp, a_exp, #1        ;decrement exponent

        BPL     sub_round_norm1         ;and repeat if necessary

        ADDS    a2, a2, w1, LSR #31     ;round
        ADCS    a1, a1, #0
        MOVCCS  r15, r14                ;and go home...

        MOV     a1, #&80000000          ;overflow due to round, so correct
        ADD     a_exp, a_exp, #1
        MOV     w1, w1, LSR #1
        MOVS    r15, r14

huge_sh_sub

; A shift of 64 or more bits needed

        SUBS    w1, w1, b1, LSR w2      ;set up remainder
        MOVMIS  r15, r14                ;no round as ms. bit lost = 1 

        SBCS    a2, a2, #0              ;otherwise round down as needed
        SBCS    a1, a1, #0

        MOVMIS  r15, r14                ;return if normalised
        B       sub_round_norm1         ;otherwise normalise

big_sh_sub

; A shift of 32 bits or more needed - first deal with shift >63 - totally lost

        MOV     w1, #0
        SUBS    w2, w3, #64             ;will smaller be totally shifted away?
        BGE     huge_sh_sub

; A shift of 32 - 63 bits - some subing to do

        SUB     w3, w3, #32
        RSB     w2, w3, #32

        SUBS    w4, w1, b2, LSL w2      ;put remainder in w4 and w1
        SBCS    w1, w1, b2, LSR w3
        SUBCC   w1, w1, b1, LSL w2
        SUBCSS  w1, w1, b1, LSL w2

        SBCS    a2, a2, b1, LSR w3      ;now calculate result
        SBCS    a1, a1, #0

        BPL     sub_round_norm          ;not normalised, so normalise

        ADDS    a2, a2, w1, LSR #31     ;round
        ADC     a1, a1, #0

        MOVS    r15, r14                ;if normalised, go home

sub_round_norm

; MS bit zero after subtract, so normalisation needed - usually by only 1 bit

        ADDS    w4, w4, w4              ;double mantissa
        ADCS    w1, w1, w1
        ADCS    a2, a2, a2
        ADCS    a1, a1, a1

        SUB     a_exp, a_exp, #1        ;decrement exponent

        BPL     sub_round_norm          ;and repeat if necessary

        ADDS    a2, a2, w1, LSR #31     ;round
        ADCS    a1, a1, #0
        MOVCCS  r15, r14                ;and go home...

        MOV     a1, #&80000000          ;overflow due to round, so correct
        ADD     a_exp, a_exp, #1
        MOV     w1, w1, LSR #1
        MOVS    r15, r14

b_bigger_sub

; Fm is bigger, so we'll have to swap them round (compactness v. speed)

        RSB     w3, w3, #0              ;negate w3

        EOR     a_sign, a_sign, #&80000000 ;and negate result

        MOV     a_exp, b_exp            ;move exponent

        MOV     w2, a1                  ;move mantissa msw.
        MOV     a1, b1
        MOV     b1, w2

        MOV     w2, a2                  ;move mantissa lsw.
        MOV     a2, b2
        MOV     b2, w2

; We have smaller no. mantissa in b1 and b2, so shift and sub

        MOV     w1, #0                  ;create 1 word extension for result

        RSBS    w2, w3, #32             ;is shift > 1 word?
        BLE     big_sh_sub              ;yes...

        SUBS    w1, w1, b2, LSL w2      ;w1 now contains remainder
        SBCS    a2, a2, b2, LSR w3      ;now calculate result
        SUBCC   a2, a2, b1, LSL w2
        SUBCSS  a2, a2, b1, LSL w2

        SBCS    a1, a1, b1, LSR w3

        BPL     sub_round_norm1         ;if not normalised, normalise

        ADDS    a2, a2, w1, LSR #31     ;round
        ADC     a1, a1, #0
        MOVS    r15, r14                ;and go home...

zero_div_r
zero_mul
same_sub
zero_sqt_r

        MOV     a_exp, #0               ;return zero
        MOV     a2, #0
        MOV     a1, #0
        MOV     w1, #0

        MOVS    r15, r14

;******************************************************************************
;
;       Multiply routines
;

        MACRO
        AisAtimesB      $cond
        BL$cond mult_rout
        MEND

        MACRO
        AisAsquared     $cond
        BL$cond square_rout
        MEND

square_rout

; Just copy a to b then a = a*b with mult_rout

        MOV     b_sign, a_sign          ;copy a to b
        MOV     b1, a1
        MOV     b2, a2
        MOV     b_exp, a_exp            ;before dropping into mult_rout

mult_rout

; Start by working out sign of result

        EOR     a_sign, a_sign, b_sign  ;sign of result

; Now check to see if either value is zero (0*infinity not allowed)

        ORRS    w1, a1, a2              ;is a zero?
        ORRNES  w1, b1, b2              ;or b zero?
        BEQ     zero_mul                ;yes...

; We have now got numbers ready to mul, so first deal with exponent

        MOVS    w2, #&ff, 8             ;put useful mask in b_exp, setting C
        ADC     a_exp, a_exp, b_exp     ;in order to add exponents +1 (2*bias)
        ORR     b_exp, w2, w2, LSR #8

        SUB     a_exp, a_exp, b_exp, LSR #18    ;correct exponent to 1 * bias

; Result sign and exponent now calculated, so multiply mantissae

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

        STMFD   r12!, {a_sign,a_exp,r9,r14} ;workspace needed - bad 4 speed

        MOV     w1, a1, LSR #16         ;b1 in w1
        BIC     w2, a1, b_exp           ;b2 in w2
        MOV     w3, a2, LSR #16         ;b3 in w3
        BIC     w4, a2, b_exp           ;b4 in w4

        BIC     r9, b1, b_exp           ;a2 in r9
        MOV     r8, r8, LSR #16         ;a1 in r8
        BIC     r11, b2, b_exp          ;a4 in r11
        MOV     r10, r10, LSR #16       ;a3 in r10

        mult    a_exp, w3, r11          ;p15 in a_exp
        mult    a2, w1, r11             ;p13 in a2
        mult    a1, w1, r9              ;p5 in a1

        mult    a_sign, w4, r10         ;p12 in a_sign
        ADDS    a_exp, a_exp, a_sign    ;so add p12 in a1-a_exp
        mult    a_sign, w2, r10         ;p10 in a_sign
        ADCS    a2, a2, a_sign          ;so add p10 in a1-a_exp
        mult    a_sign, w2, r8          ;p2 in a_sign
        ADCS    a1, a1, a_sign          ;so add p2 in a_sign-a_exp
        MOVCC   r14, #0                 ;move carry bit into r14
        MOVCS   r14, #&1

        mult    a_sign, w3, r9          ;p7 in a_sign
        ADDS    a2, a2, a_sign          ;so add p7 in a1-a_exp
        ADCS    a1, a1, #0
        ADC     r14, r14, #0

        mult    a_sign, w4, r8          ;p4 in a_sign
        ADDS    a2, a2, a_sign          ;so add p4 in a1-a_exp
        ADCS    a1, a1, #0
        ADC     r14, r14, #0

; We now have all the unaligned words added up, so rotate them and continue
; with the aligned ones

        ORR     r14, r14, a_exp, LSL #16 ;rotate answer right 16, using r14
        MOV     a_exp, a_exp, LSR #16
        ORR     a_exp, a_exp, a2, LSL #16
        MOV     a2, a2, LSR #16
        ORR     a2, a2, a1, LSL #16
        MOV     a1, a1, LSR #16
        ORR     a1, a1, r14, LSL #16
        BIC     r14, r14, #&ff

        mult    a_sign, w2, r11         ;p14 in a_sign
        mult    r11, w4, r11            ;p16 in r11 (a4 now discarded)
        ADDS    r11, r14, r11           ;so add p16 in answer
        ADCS    a_exp, a_exp, a_sign    ;so add p14 in answer
        mult    a_sign, w3, r8          ;p3 in a_sign
        ADCS    a2, a2, a_sign          ;so add p3 in answer
        mult    a_sign, w1, r8          ;p1 in a_sign
        ADC     a1, a1, a_sign          ;so add p1 in answer

        mult    a_sign, w1, r10         ;p9 in a_sign
        mult    r10, w3, r10            ;p11 in r10 (a3 now discarded)
        ADDS    r10, r10, a_exp         ;so add p11 in answer, rem in r10,r11
        ADCS    a2, a2, a_sign          ;so add p9 in answer
        ADC     a1, a1, #0

        mult    a_sign, w4, r9          ;p8 in a_sign
        ADDS    w1, r10, a_sign         ;so add p8 in answer, rem in w1
        mult    a_sign, w2, r9          ;p6 in a_sign
        ADCS    a2, a2, a_sign          ;so add p6 in answer
        ADCS    a1, a1, #0

        LDMFD   r12!, {a_sign,a_exp,r9,r14} ;restore workspace regs

; Mantissae have now been multiplied ***********************************

        BMI     normalised_mul          ;already normalised...

mul_round_norm

; MS bit zero after multiply, so normalisation needed - usually by only 1 bit

        ADDS    r11, r11, r11           ;double mantissa
        ADCS    w1, w1, w1
        ADCS    a2, a2, a2
        ADCS    a1, a1, a1

        SUB     a_exp, a_exp, #1        ;decrement exponent

        BPL     mul_round_norm          ;and repeat if necessary

normalised_mul

        ADDS    a2, a2, w1, LSR #31     ;round
        ADCS    a1, a1, #0
        MOVCCS  r15, r14                ;go home

        MOV     a1, #&80000000          ;rounding overflowed
        ADD     a_exp, a_exp, #1
        MOV     w1, w1, LSR #1
        MOVS    r15, r14


;******************************************************************************
;
;       Divide routines (dead slow!)
;

        MACRO
        AisAoverB       $cond
        BL$cond div_rout
        MEND

        MACRO
        AisAinverted    $cond
        BL$cond invert_rout
        MEND

invert_rout

; We want a = 1/a, so put a in b, 1 in a, and drop into div_rout

        MOV     b_sign, a_sign          ;b=a
        MOV     b1, a1
        MOV     b2, a2
        MOV     b_exp, a_exp

        MOV     a_sign, #0              ;a=1
        MOV     a1, #&80000000
        MOVS    a2, #0,2                ;(clear C)
        RSC     a_exp, a2, #&4000       ;(exp=&3fff)

div_rout

; Start by working out sign of result

        EOR     a_sign, a_sign, b_sign  ;sign of result

; Now check to see if either value is zero

        ORRS    w1, a1, a2              ;is a zero?
        ORRNES  w1, b1, b2              ;or b zero?
        BEQ     zero_div_r              ;yes...

; We have now got numbers ready to dvf, so first deal with exponent

        SUB     a_exp, a_exp, b_exp     ;calculate exponent
        ADD     a_exp, a_exp, #&4000    ;and correct bias
        SUB     a_exp, a_exp, #1

; Result sign and exponent now calculated, so divide mantissae

        MOV     b_exp, #0               ;handy zero for later

        SUBS    w2, a2, b2              ;if num > den, subtract num from den,
        SBCS    w1, a1, b1

        MOVCS   a1, #3                  ;get normalised result by
        BCS     div_loop_ent_r          ;jumping to right place

        MOV     w2, a2                  ;otherwise copy numerator,
        MOV     w1, a1
        MOV     a1, #1                  ;calculating 32 more bits
        SUB     a_exp, a_exp, #1        ;and decrementing exponent

div_loop_r

; This is the first of 2 main loops doing the division of the mantissae
; It contains repetitive code to minimise branching

        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        ADC     b_exp, b_exp, #0        ;hence storing it in b_exp

        SUBS    w4, w2, b2              ;attempt to subtract denominator
        SBCS    w3, w1, b1
        MOVCCS  b_exp, b_exp, LSR #1    ;set C if numerator > denominator

        MOVCS   w2, w4                  ;move in subtracted value if OK
        MOVCS   w1, w3
        ADC     a1, a1, a1              ;and insert C in result and rotate

div_loop_ent_r
        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        ADC     b_exp, b_exp, #0        ;hence storing it in b_exp

        SUBS    w4, w2, b2              ;attempt to subtract denominator
        SBCS    w3, w1, b1
        MOVCCS  b_exp, b_exp, LSR #1    ;set C if numerator > denominator

        MOVCS   w2, w4                  ;move in subtracted value if OK
        MOVCS   w1, w3
        ADC     a1, a1, a1              ;and insert C in result and rotate


        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        ADC     b_exp, b_exp, #0        ;hence storing it in b_exp

        SUBS    w4, w2, b2              ;attempt to subtract denominator
        SBCS    w3, w1, b1
        MOVCCS  b_exp, b_exp, LSR #1    ;set C if numerator > denominator

        MOVCS   w2, w4                  ;move in subtracted value if OK
        MOVCS   w1, w3
        ADC     a1, a1, a1              ;and insert C in result and rotate


        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        ADC     b_exp, b_exp, #0        ;hence storing it in b_exp

        SUBS    w4, w2, b2              ;attempt to subtract denominator
        SBCS    w3, w1, b1
        MOVCCS  b_exp, b_exp, LSR #1    ;set C if numerator > denominator

        MOVCS   w2, w4                  ;move in subtracted value if OK
        MOVCS   w1, w3
        ADCS    a1, a1, a1              ;and insert C in result and rotate


        BCC     div_loop_r              ;and loop again

; We have produced 32 bits of result. If exact result or single precision
; wanted, this is enough, so next deal with these possibilities.

        ORRS    a2, w1, w2              ;is remainder zero?
        MOVEQS  r15, r14                ;yes, so 32 bits will do (a2=0)

        MOV     a2, #1                  ;initialise lower word of result

div_loop2_r

; This is the second of 2 main loops doing the division of the mantissae
; It contains repetitive code to minimise branching

        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        ADC     b_exp, b_exp, #0        ;hence storing it in b_exp

        SUBS    w4, w2, b2              ;attempt to subtract denominator
        SBCS    w3, w1, b1
        MOVCCS  b_exp, b_exp, LSR #1    ;set C if numerator > denominator

        MOVCS   w2, w4                  ;move in subtracted value if OK
        MOVCS   w1, w3
        ADC     a2, a2, a2              ;and insert C in result and rotate


        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        ADC     b_exp, b_exp, #0        ;hence storing it in b_exp

        SUBS    w4, w2, b2              ;attempt to subtract denominator
        SBCS    w3, w1, b1
        MOVCCS  b_exp, b_exp, LSR #1    ;set C if numerator > denominator

        MOVCS   w2, w4                  ;move in subtracted value if OK
        MOVCS   w1, w3
        ADC     a2, a2, a2              ;and insert C in result and rotate


        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        ADC     b_exp, b_exp, #0        ;hence storing it in b_exp

        SUBS    w4, w2, b2              ;attempt to subtract denominator
        SBCS    w3, w1, b1
        MOVCCS  b_exp, b_exp, LSR #1    ;set C if numerator > denominator

        MOVCS   w2, w4                  ;move in subtracted value if OK
        MOVCS   w1, w3
        ADC     a2, a2, a2              ;and insert C in result and rotate


        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        ADC     b_exp, b_exp, #0        ;hence storing it in b_exp

        SUBS    w4, w2, b2              ;attempt to subtract denominator
        SBCS    w3, w1, b1
        MOVCCS  b_exp, b_exp, LSR #1    ;set C if numerator > denominator

        MOVCS   w2, w4                  ;move in subtracted value if OK
        MOVCS   w1, w3
        ADCS    a2, a2, a2              ;and insert C in result and rotate


        BCC     div_loop2_r             ;and loop again

; We have produced a 64 bit result - calculate 65th bit & round with it

        ADDS    w2, w2, w2              ;double numerator
        ADCS    w1, w1, w1              ;setting C if MS bit lost
        BCS     num_biggerR_r           ;hence denominator can be subtracted

        CMP     w1, b1                  ;numerator >= denominator?
        CMPEQ   w2, b2

num_biggerR_r
        ADCS    a2, a2, #0              ;add in carry to do rounding
        ADCS    a1, a1, #0

        MOVCCS  r15, r14                ;and go home if no overflow

        ADC     a_exp, a_exp, #0        ;if overflow, increment exponent
        MOV     a1, #&80000000          ;and restore mantissa
        MOVS    r15, r14                ;and go home

;******************************************************************************
;
;       Square root routine
;

        MACRO
        AisrootA        $cond
        BL$cond root_rout
        MEND

root_rout

; First check for zero

        MOVS    r4, r3, LSL #17         ;is exponent zero?
        BEQ     zero_sqt_r              ;yes, so return zero...

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

sqt_loop1_r

; The first main sqt loop - when finished we will have 31 bits of result

        ADDS    r5, r5, r5              ;double remainder
        ADCS    r4, r4, r4              ;hence C -> rem>result||01
        
        EOR     r6, r1, r7              ;result||01 in r6
        CMPCC   r4, r6                  ;hence will this subtract?
        SUBCS   r4, r4, r6              ;yes, so do so
        EORCS   r1, r1, r7, LSL #1      ;and put a 1 in result

        MOVS    r7, r7, ROR #1          ;then rotate pattern
        BPL     sqt_loop1_r             ;and carry on...

; We have 31 bits of result in r1 - if precision S or exact, finish

        ORRS    r2, r4, r5              ;is remainder zero?
        MOVEQS  r15, r14                ;yes, so finish...

        MOV     r2, #0                  ;clear r2 for lower result

sqt_loop2_r

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
        BNE     sqt_loop2_r             ;and carry on...

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

; We now have a 64 bit result, with a 96 bit remainder in r4,r5,r7

        ADDS    r7, r7, r7
        ADCS    r5, r5, r5              ;double remainder
        ADCS    r4, r4, r4              ;setting C if MS bit lost
        BCS     num_biggerRs_r          ;hence result||01 can be subtracted

        CMP     r4, r1                  ;result||01 > remainder?
        CMPEQ   r5, r2
        CMPEQ   r7, #&40000000

num_biggerRs_r
        ADCS    r2, r2, #0              ;add in carry to do rounding
        ADCS    r1, r1, #0

        ADC     r3, r3, #0              ;if overflow, increment exponent
        MOVCS   r1, #&80000000          ;and restore mantissa

        MOVS    r15, r14                ;then go home

;******************************************************************************
;
;       Routine to do Float function
;

        MACRO
        BisfloatW1      $cond
        BL$cond float_rout
        MEND

float_rout

; Put FLOAT(w1) in b, corrupting only w1 - first move into b

        MOVS    b_sign, w1              ;set up sign
        RSBMI   w1, w1, #0              ;and if negative, negate
        MOV     b_exp, #&4000           ;set up exponent for integer

        MOV     b2, #0                  ;set up mantissa
        MOV     b1, w1

; Normalisation needed

        MOVS    w1, b1, LSR #16         ;can we shift left 16 bits?
        MOVEQ   b1, b1, LSL #16         ;yes, so do so
        ADDNE   b_exp, b_exp, #16       ;and reduce exponent

        MOVS    w1, b1, LSR #24         ;can we shift left 8 bits?
        MOVEQ   b1, b1, LSL #8          ;yes, so do so
        ADDNE   b_exp, b_exp, #8        ;and reduce exponent

        MOVS    w1, b1, LSR #28         ;can we shift left 4 bits?
        MOVEQ   b1, b1, LSL #4          ;yes, so do so
        ADDNE   b_exp, b_exp, #4        ;and reduce exponent

        MOVS    w1, b1, LSR #30         ;can we shift left 2 bits?
        MOVEQ   b1, b1, LSL #2          ;yes, so do so
        ADDNE   b_exp, b_exp, #2        ;and reduce exponent

        MOVS    w1, b1, LSR #31         ;can we shift left 1 bit?
        MOVEQ   b1, b1, LSL #1          ;yes, so do so
        SUBEQ   b_exp, b_exp, #1        ;and reduce exponent

        MOVS    r15, r14                ;go home


;******************************************************************************
;
;       Routines to return integer part or round to integer
;

        MACRO
        BisintpartA     $cond
        BL$cond int_rout
        MEND

int_rout

; First copy sign and exponent

        MOV     b_sign, a_sign
        MOV     b_exp, a_exp

; Now truncate mantissa (this is integer part rather than round)

        SUB     w1, b_exp, #&fe
        SUBS    w1, w1, #&3f00          ;number of bits to be left alone

        BLE     int_zero                ;none, so return zero

        RSBS    w2, w1, #32             ;are there more than 32?
        BMI     big_number              ;yes, so deal with this

; 32 or less bits of mantissa to be left, so clear others

        MOV     b2, #0
        MOV     b1, a1, LSR w2          ;clear correct number of bits
        MOV     b1, b1, LSL w2
        MOVS    r15, r14                ;and go home

big_number

; 33 or more bits of mantissa to be left, so clear as needed

        MOV     b1, a1                  ;start by moving a to b
        MOV     b2, a2

        RSBS    w2, w1, #64             ;are there more than 64?
        MOVMIS  r15, r14                ;yes, so leave whole thing alone

        MOV     b2, b2, LSR w2          ;clear desired number of bits
        MOV     b2, b2, LSL w2
        MOVS    r15, r14                ;and go home

int_zero_r

; The integer part is zero, so return this unless rounding up to 1 (Z set)

        MOVEQ   w1, #1
        MOVEQ   b1, #&80000000          ;if number >= 0.5, Z set, so return 1
        MOV     b2, #0
        ADDEQ   b_exp, b_exp, #1
        MOVEQS  r15, r14

int_zero

; The integer part is zero, so return this

        MOV     w1, #0
        MOV     b1, #0
        MOV     b2, #0
        MOV     b_exp, #0
        MOVS    r15, r14


        MACRO
        BisintrndA      $cond
        BL$cond intrnd_rout
        MEND

intrnd_rout

; First copy sign and exponent

        MOV     b_sign, a_sign
        MOV     b_exp, a_exp

; Now truncate mantissa 

        SUB     w1, b_exp, #&fe
        SUBS    w1, w1, #&3f00          ;number of bits to be left alone

        BLE     int_zero_r              ;none, so return zero unless round up

        RSBS    w2, w1, #32             ;are there more than 32?
        BMI     big_number_r            ;yes, so deal with this

; 32 or less bits of mantissa to be left, so clear others

        MOV     b2, #0
        MOVS    w1, a1, LSR w2          ;clear correct number of bits
        ADC     w1, w1, #0              ;and round up if ms bit lost = 1
        MOVS    b1, w1, LSL w2
        MOVCCS  r15, r14                ;and go home if no overflow

        MOV     b1, b1, RRX             ;put carry bit back in
        ADD     b_exp, b_exp, #1        ;increment exponent
        MOVS    r15, r14                ;and go home

big_number_r

; 33 or more bits of mantissa to be left, so clear as needed

        MOV     b1, a1                  ;start by moving a to b
        MOV     b2, a2

        RSBS    w2, w1, #64             ;are there more than 64?
        MOV     w1, #-1                 ;indicate 32 bit integer meaningless
        MOVMIS  r15, r14                ;yes, so leave whole thing alone

        MOVS    b2, b2, LSR w2          ;clear desired number of bits
        ADC     b2, b2, #0              ;and round up if ms bit lost = 1
        MOVS    b2, b2, LSL w2
        ADCS    b1, b1, #0
        MOVCCS  r15, r14                ;and go home

        MOV     b2, b2, LSR #1
        ORR     b2, b2, b1, LSL #31
        MOV     b1, b1, RRX             ;put carry bit back in
        ADD     b_exp, b_exp, #1        ;increment exponent
        MOVS    r15, r14                ;and go home


        MACRO
        AisrndA $cond
        BL$cond rnd_rout
        MEND

rnd_rout

        ORR     r14, r14, #&40000000    ;Z set for return lr

; Now truncate mantissa 

        SUB     w1, a_exp, #&fe
        SUBS    w1, w1, #&3f00          ;number of bits to be left alone

        BLE     int_zero_rnd            ;none, so return zero unless round up

        RSBS    w2, w1, #32             ;are there more than 32?
        BMI     big_number_rnd          ;yes, so deal with this

; 32 or less bits of mantissa to be left, so clear others

        MOV     b1, a1, LSL w1          ;put remainder in b1
        MOVEQ   b1, a2                  ;taking care of w1 = 32 case
        MOVNE   b2, a2                  ;and b2
        MOVEQ   b2, #0
        MOV     a2, #0
        MOV     a1, a1, LSR w2          ;clear correct number of bits

        ANDS    w1, r9, #&00000060      ;rounding mode bits
        BNE     n_def_rnd               ;not default...

        ORRS    b2, b2, b1, LSL #1      ;are all bits except ms lost zero?
        ANDEQ   b1, b1, a1, LSL #31     ;if yes, round to even (IEEE)
        ADDS    a1, a1, b1, LSR #31     ;add in rounding bit

tidy_small2
        MOVCCS  a1, a1, LSL w2          ;then rotate correctly

        MOVCCS  r15, r14                ;and go home if no overflow

        MOV     a1, a1, RRX             ;put carry bit back in
        ADD     a_exp, a_exp, #1        ;increment exponent
        MOVS    r15, r14                ;and go home

n_def_rnd

; Not default rounding mode - truncate unless round away from zero

        CMP     w1, #&60                ;round to zero?
        ORRNES  b2, b1, b2              ;or no remainder?
        BEQ     tidy_small              ;yes, so just truncate...

        EORS    w1, a_sign, w1, LSL #26 ;+ and +inf or - and -inf?
        BPL     tidy_small
        ADDS    a1, a1, #1              ;if yes, add 1 bit
        B       tidy_small2             ;then tidy up

tidy_small
        MOV     a1, a1, LSL w2          ;then rotate correctly
        MOVS    r15, r14                ;and go home

big_number_rnd

; 33 or more bits of mantissa to be left, so clear as needed

        RSBS    w2, w1, #64             ;are there 64 or more?
        MOVLES  r15, r14                ;yes, so leave whole thing alone

        SUB     w1, w1, #32             ;no. of bits left in 2nd word
        MOV     b1, a2, LSL w1          ;save bits lost
        MOVS    a2, a2, LSR w2          ;clear desired number of bits

        ANDS    w1, r9, #&00000060      ;rounding mode bits
        BNE     n_def_rnd_big           ;not default...

        MOVS    b2, b1, LSL #1          ;are all bits except ms lost zero?
        ANDEQ   b1, b1, a2, LSL #31     ;if yes, round to even (IEEE)
        ADD     a2, a2, b1, LSR #31     ;add in rounding bit

tidy_big
        MOVS    a2, a2, LSL w2
        ADCS    a1, a1, #0
        MOVCCS  r15, r14                ;and go home

        MOV     a2, a2, LSR #1
        ORR     a2, a2, a1, LSL #31
        MOV     a1, a1, RRX             ;put carry bit back in
        ADD     a_exp, a_exp, #1        ;increment exponent
        MOVS    r15, r14                ;and go home

n_def_rnd_big

; Not default rounding mode - truncate unless round away from zero

        CMP     w1, #&60                ;round to zero?
        CMPNE   b1, #0                  ;or no remainder?
        BEQ     tidy_big                ;yes, so just truncate...

        EORS    w1, a_sign, w1, LSL #26 ;+ and +inf or - and -inf?
        ADDMI   a2, a2, #1              ;if yes, add 1 bit
        B       tidy_big                ;then tidy up

int_zero_rnd

; We have less than 1.0

        BEQ     above_half_rnd          ;we have more than a half

        ANDS    w1, r9, #&60            ;get rounding mode bits
        BNE     n_def_rnd_bhalf         ;not default mode...

return_zero_r
        ORRS    b1, a1, a2              ;do we already have zero?
        MOVEQS  r15, r14                ;yes, so return this without ufl

        MOV     a1, #0                  ;return zero
        MOV     a2, #0
        MOV     a_exp, #0
        MOV     r4, #UFL_E_MASK         ;and underflow event
        MOV     r15, r14                ;signalled by Z clear

n_def_rnd_bhalf

        ORRS    b1, a1, a2              ;do we already have zero?
        CMPNE   w1, #&60                ;or is it round to zero?
        BEQ     return_zero_r           ;yes, so return zero

        EORS    w1, a_sign, w1, LSL #26 ;+ and +inf or - and -inf?
        BPL     return_zero_r           ;no, so return zero...

return_one_r
        MOV     a1, #&80000000
        MOVS    a2, #0,2
        RSC     a_exp, a2, #&4000
        MOVS    r15, r14

above_half_rnd

; We have half to under one - return 1 unless truncating

        ANDS    w1, r9, #&60            ;get rounding mode bits
        BNE     n_def_rnd_bhalf         ;not default mode
        ORRS    w1, a2, a1, LSL #1      ;have we exactly a half?
        BNE     return_one_r            ;no, so return one
        B       return_zero_r           ;yes, so return zero

;******************************************************************************
;
;       Routine to swap a and b round
;

        MACRO
        swapAandB       $cond
        BL$cond swap_rout
        MEND

swap_rout

; Swap round a and b - corrupt only w1

        MOV     w1, a_sign
        MOV     a_sign, b_sign
        MOV     b_sign, w1

        MOV     w1, a1
        MOV     a1, b1
        MOV     b1, w1

        MOV     w1, a2
        MOV     a2, b2
        MOV     b2, w1

        MOV     w1, a_exp
        MOV     a_exp, b_exp
        MOV     b_exp, w1

        MOVS    r15, r14

;******************************************************************************

filelevel       SETA    filelevel - 1
        END
