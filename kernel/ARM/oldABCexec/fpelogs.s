; -> fpelogs/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2logs        6/2/87  Martin Clemoes
;
;       Description:    This file contains implementations of the LOG, LGN and 
;                       EXP instructions, making use of subroutines in
;                       FPE2utils (so must be assembled after this).
;
;******************************************************************************
;
;       A NaN or two returned by untrapped invalid operations
;

badlogNaN       *       &C6000000       ;a logarithm of negative number


;******************************************************************************
;
;       Logarithm (LOG and LGN) routines
;

log3
lgn3

        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

log
lgn

; First check to see if source a const., and if not, load source reg.

        MOVS    r7, r9, LSL #28         ;separate fm bits
        BMI     log_constant            ;we've got a constant...

        ADD     r8, r12, r7, LSR #24    ;hence point at source reg.

        LDMIA   r8, {r0-r3}             ;load up source

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U
        STMNEIA r8, {r0-r3}             ;and write back if converted

; Now check for NaN, infinity, zero or denormalised

        MOVS    r4, r3, LSL #17         ;is exponent zero?
        ADDNES  r4, r4, #&00020000      ;or maximum value?
        BEQ     funny_log               ;yes, so must be one of funnies...

normal_log

; Now check for logarithm of negative number

        TST     r0, #&80000000          ;is sign negative (zero already done)
        BNE     neg_log                 ;yes, oh dear...

got_const_log

        STMDB   r12!, {r14}
        BL      log_rout                ;do the main job (always works)
        LDMIA   r12!, {r14}

exit_log

; We now have a 64 bit result

        TST     r9, #&00080000          ;extended precision?
        BNE     no_trunc_log            ;yes...

        ADR     r7, exit_log1
        CMP     r3, #0
        BLE     und_uTRs

        TST     r9, #&80                ;double precision?
        BEQ     UtruncS                 ;no, so truncate to single
        B       UtruncD                 ;yes, so truncate to double

no_trunc_log

        CMP     r3, #0                  ;have we underflowed?
        BLT     underflow_log           ;yes...

exit_log1

; Register inexact event, store result and go home

        LDR     r5, [r12, #8*16]        ;get fpsr
        TST     r5, #INX_E_MASK         ;inexact event already?
        BEQ     register_inx            ;no, so register one...

exit_log2
        AND     r9, r9, #&00007000      ;destination reg. no.
        ADD     r9, r12, r9, LSR #8     ;hence its address
        STMIA   r9, {r0-r3}             ;save value to fp. reg.

;        savebaseinr11
        exit0to15                       ;and go home...

underflow_log

; We have an exponent of less than 0, so denormalise if possible, otherwise und

        CMP     r3, #-64                ;can we denormalise?
        BGT     denorm_suf              ;yes, so do so
        B       und_uTRs                ;no, so underflow

neg_log
overflow_log

; We are trying to take a logarithm of a negative number or zero

        MOV     r1, #badlogNaN          ;if untrapped, return NaN
        MOV     r2, #0
        MOV     r3, #&ff
        ORR     r3, r3, r3, LSL #7
        B       NaN_add

funny_log

; We are taking the square root of +/-0, infinity, NaN or denormalised

        CMP     r3, #0                  ;NaN or infinity?
        BNE     max_exp_log             ;yes...

        ORRS    r4, r1, r2              ;zero?
        BEQ     overflow_log            ;yes, so return NaN
        MOVS    r1, r1                  ;denormalised?
        BMI     normal_log              ;no, so go back

norm_log
        ADDS    r2, r2, r2              ;double mantissa
        ADCS    r1, r1, r1
        SUB     r3, r3, #1              ;decrement exponent
        BPL     norm_log

        B       normal_log

max_exp_log
        ORRS    r4, r1, r2              ;infinity?
        BNE     NaN_add                 ;no, so must be a NaN
        TST     r0, #&80000000          ;yes, so is it negative?
        BEQ     exit_log1               ;no, so OK
        B       neg_log                 ;yes...

log_rout

; The algorithm used for LOG, LGN is described in Cody & Waite, chapter 5

; We have an acceptable normalised no. to log in register set a (see FPE2utils)
; So start by comparing mantissa with root of a half and estimating zden, znum.

        ADR     w1, root_half           ;load b with mantissa of root of 0.5
        LDMIA   w1, {b1, b2}

        CMP     a1, b1                  ;is mantissa of a > root 0.5?
        CMPEQ   a2, b2

        MOV     b2, a2, LSR #1          ;zden = f * 0.5 + 0.5
        ORR     b2, b2, a1, LSL #31
        MOV     b1, a1, LSR #1
        ORR     b1, b1, #&80000000

        MOV     b_exp, #&fe             ;set up exponent of zden
        ORR     b_exp, b_exp, b_exp, LSL #6

        SUB     w1, a_exp, b_exp        ;remove bias from power of 2
        SUBLS   w1, w1, #1              ;and if <= root 0.5, decrement

        STMDB   r12!, {w1,r9,r14}       ;save necessary regs.

        MOV     a_exp, b_exp            ;put f in a
        BHI     gt_root2                ;and continue

; f<=root 0.5, so adjust znum and zden accordingly

        BIC     b1, b1, #&40000000      ;zden = zden - 0.25

        ADDS    a2, a2, a2              ;znum = 2 * znum - 0.5
        ADCS    a1, a1, a1
        SUB     a_exp, a_exp, #1        ;divided by 2 again
        BPL     norm_znum               ;normalise if necessary
        B       got_fract_log           ;otherwise carry on...

gt_root2

; f>root(0.5), so znum = f-1 = -(1-f)

        MOV     a_sign, #&80000000      ;sign negative
        RSBS    a2, a2, #0              ;and negate mantissa
        RSCS    a1, a1, #0              ;negated mantissa won't be normalised

norm_znum
        CMPEQ   a2, #0                  ;have we znum = 0?
        BEQ     zero_znum               ;yes...

        ADDS    a2, a2, a2              ;double mantissa
        ADCS    a1, a1, a1
        SUB     a_exp, a_exp, #1        ;decrement exponent
        BPL     norm_znum               ;and repeat as necessary...

got_fract_log

; We now have a=znum, b=zden, so calculate z = znum/zden, w=z squared

        MOV     b_sign, #0
        AisAoverB                       ;do division
        STMDB   r12!, {a_sign, a1, a2, a_exp} ;and stack z away

        AisAsquared                     ;hence a= a squared = w
        STMDB   r12!, {a_sign, a1, a2, a_exp} ;and stack w away

; Next we evaluate B(W) using the coefficient constants

        ADR     w5, coefficients        ;point at table of coefficients

        LDMIA   w5!, {b1,b2,b_exp}      ;and get first, b2
        AisAminusB                      ;then form w-b2

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve w
        AisAtimesB                      ;then form (w-b2)*w

        LDMIA   w5!, {b1,b2,b_exp}      ;get b1
        AisAplusB                       ;then form (w-b2)*w+b1

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve w
        AisAtimesB                      ;then form ((w-b2)*w+b1)*w

        LDMIA   w5!, {b1,b2,b_exp}      ;get b0
        AisAminusB                      ;then form ((w-b2)*w+b1)*w+b0

; We have evaluated B(W), so save B(W) and evaluate A(W)

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve w before stack drops
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save B(W) on stack

        MOV     a_sign, #&80000000      ;put a2 in a (negative)
        LDMIA   w5!, {a1,a2,a_exp}
        AisAtimesB                      ;so get a2*w

        LDMIA   w5!, {b1,b2,b_exp}      ;get a1
        AisAplusB                       ;then form a2*w+a1

        ADD     w1, r12, #4*4           ;retrieve w
        LDMIA   w1, {b_sign,b1,b2,b_exp}
        AisAtimesB                      ;then form (a2*w+a1)*w

        LDMIA   w5!, {b1,b2,b_exp}      ;get a0
        AisAminusB                      ;then form (a2*w+a1)*w+a0

; We have evaluated A(W), so evaluate R(Z) = Z + Z*W*A(W)/B(W)

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve B(W)
        AisAoverB                       ;then form A(W)/B(W)

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve W
        AisAtimesB                      ;then form W*A(W)/B(W)

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve Z
        AisAtimesB                      ;then form Z*W*A(W)/B(W)

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve Z again
        AisAplussignB                   ;then form Z + Z*W*A(W)/B(W)

rz_evaluated

; We now have R(Z), so evaluate (c1+c2)*N + R(Z)

        LDMIA   r12!, {w1}              ;retrieve N
        BisfloatW1                      ;and make a floating point no. of it

        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save R(Z)

        MOV     a_sign, #0              ;then get C1+C2
        LDMIA   w5!, {a1,a2,a_exp}
        AisAtimesB                      ;and form (C1+C2)*N

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve R(Z)
        AisAplussignB                   ;and add it in

; We now have a natural log, so if LOG rather than LGN, convert

        LDMIA   r12!, {r9}              ;retrieve FP instruction
        TST     r9, #&00100000          ;is is LOG 10?

        ADR     b_sign, log10_factor    ;set positive
        LDMNEIA b_sign, {b1,b2,b_exp}   ;and if LOG10, get factor
        AisAtimesB      NE              ;and multiply it in

        LDMIA   r12!, {r15}             ;retrieve LR in pc to go home

zero_znum

; znum = 0, so R(Z)=Z=0, so set a to 0 and go to where R(Z) has been evaluated

        MOV     a_sign, #0              ;a1=a2=0, so set up a_sign
        MOV     a_exp, #0               ;and exponent

        ADR     w5, C1plusC2            ;set up coefficient pointer
        B       rz_evaluated            ;and pretend R(Z) evaluated


;******************************************************************************
;
;       Data for calculating logs
;

root_half                               ;mantissa for extended prec. root(0.5)
        DCD     &b504f333
        DCD     &f9de6484

coefficients                            ;coefficients for series expansion

        DCD     &8eac025b               ;B2 in unpacked form, sign missing
        DCD     &3e7076bb               ;=0.35667 97773 90346 46171 E+2
        DCD     &00004004

        DCD     &9c041fd0               ;B1
        DCD     &a933ef60               ;=0.31203 22209 19245 32844 E+3
        DCD     &00004007

        DCD     &c05ff4e0               ;B0
        DCD     &6c83bb96               ;=0.76949 93210 84948 79777 E+3
        DCD     &00004008


        DCD     &ca20ad9a               ;A2
        DCD     &b5e946e9               ;=0.78956 11288 74912 57267 E+0
        DCD     &00003ffe

        DCD     &83125100               ;A1
        DCD     &b57f6509               ;=0.16383 94356 30215 34222 E+2
        DCD     &00004003

        DCD     &803ff895               ;A0
        DCD     &9dacd228               ;=0.64124 94342 37455 81147 E+2
        DCD     &00004005

C1plusC2
        DCD     &b17217f7               ;C1+C2
        DCD     &d1cf79ac
        DCD     &00003ffe

log10_factor                            ;LOG(x) = LGN(x) * LOG(e)

        DCD     &de5bd8a9               ;so LOG(e)
        DCD     &37287195               ;=0.43429 44819 03251 82765 E+0
        DCD     &00003ffd


;******************************************************************************
;
;       EXP routines
;
exp3

        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

exp

; First check to see if source a const., and if not, load source reg.

        MOVS    r7, r9, LSL #28         ;separate fm bits
        BMI     exp_constant            ;we've got a constant...

        ADD     r8, r12, r7, LSR #24    ;hence point at source reg.

        LDMIA   r8, {r0-r3}             ;load up source

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U
        STMNEIA r8, {r0-r3}             ;and write back if converted

; Now check for NaN, infinity

        MOV     r4, r3, LSL #17
        ADDS    r4, r4, #&00020000      ;is exponent maximum value?
        BEQ     max_exp_exp             ;yes, so must be one of funnies...

got_const_exp

        STMDB   r12!, {r14}
        BL      exp_rout                ;get exponent, Z cleared if ofl/ufl
        LDMIA   r12!, {r14}
        BEQ     exit_log                ;no overflow/underflow, so carry on...

; We have overflow or underflow, so deal with this

        ADR     r7, exit_log1           ;return to here if no exception
        B       check_exep              ;and check for exception (mask in r4)

max_exp_exp

; The arguament has max. exponent value - infinity or NaN

        ORRS    r4, r1, r2              ;have we infinity?
        BNE     NaN_add                 ;no, so must be a NaN...
        TST     r0, #&80000000          ;yes, so is it negative?
        BEQ     exit_log1               ;no, so OK
        MOV     r0, #0                  ;negative inf.,so return 0
        MOV     r3, #0
        B       exit_log1

exp_rout

; Subroutine that does a = exp a, assuming nan and infinity 
; already weeded out. Calculation performed completely in extended precision.
; The algorithm implemented is described in Cody & Waite, chapter 6.

; First check that result will be in range to fit in extended precision

        ADR     w1, max_check_exp       ;check that no overflow 
        TST     a_sign, #&80000000      ;unless a input is negative,
        ADRNE  w1, min_check_exp       ;when check that no underflow

        LDMIA   w1!, {b1,b2,b_exp}      ;load max magnitude value
        CMP     a_exp, b_exp            ;is this exceeded?
        CMPEQ   a1, b1
        CMPEQ   a2, b2
        BHI     problem_a               ;yes, so overflow/underflow

; Next check to see if a is so small that 1 will be returned

        LDMIA   w1, {b1,b2,b_exp}       ;get minimum magnitude
        CMP     a_exp, b_exp            ;have we even less than this?
        CMPEQ   a1, b1
        CMPEQ   a2, b2
        BCC     return_1_exp            ;yes, so return 1

; We have a suitable value, so next calculate G

        STMDB   r12!, {a_sign,a1,a2,a_exp,w1,r9,r14} ;save X, r9, r14 and 1 gap

        ADR     w5, exp_coeff           ;get 1/LGN(2)
        MOV     b_sign, #0              ;(which is positive)
        LDMIA   w5!, {b1,b2,b_exp}
        AisAtimesB                      ;form X/LGN(2)
        BisintrndA                      ;hence b = XN, w1 = ABS(N)

        TST     b_sign, #&80000000      ;is N negative?
        RSBNE   w1, w1, #0              ;yes, so now w1 = N
        STR     w1, [r12, #4*4]         ;hence store N in gap on stack

        STMDB   r12!, {b_sign,b1,b2,b_exp} ;save XN on stack
        MOV     a_sign, #0              ;get -C2 (which is positive)
        LDMIA   w5!, {a1,a2,a_exp}
        AisAtimesB                      ;form -C2*XN

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;get XN again
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and store -C2*XN away
        LDMIA   w5!, {a1,a2,a_exp}      ;get -C1
        MOV     a_sign, #&80000000      ;which is negative
        AisAtimesB                      ;form -C1*XN
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and store it away

        ADD     w1, r12, #8*4           ;point at X on stack
        LDMIA   w1, {a_sign,a1,a2,a_exp} ;and get X
        BisintpartA                     ;hence X1
        TST     b_sign, #&80000000
        AisAminusB      EQ              ;and X2
        AisAplusB       NE

        MOV     w1, r12                 ;point at -C1*XN
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;push X2
        LDMIA   w1, {a_sign,a1,a2,a_exp} ;and retrieve -C1*XN
        AisAplussignB                   ;form X1 - C1*XN

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve X2
        AisAplussignB                   ;form X1-C1*XN+X2
        ADD     r12, r12, #4*4          ;and tidy stack

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve -C2*XN
        AisAplussignB                   ;form G = X1-C1*XN+X2-C2*XN

; We have got G, so form Z and then Q(Z)

        STMIA   r12, {a_sign,a1,a2,a_exp} ;save G, tidying stack at same time
        AisAsquared                     ;form Z
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save Z

        MOV     b_sign, #0              ;load q3 (which is positive)
        LDMIA   w5!, {b1,b2,b_exp}
        AisAtimesB                      ;form q3*z

        LDMIA   w5!, {b1,b2,b_exp}      ;get q2
        AisAplusB                       ;form q3*z+q2

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve z
        AisAtimesB                      ;form (q3*z+q2)*z

        LDMIA   w5!, {b1,b2,b_exp}      ;get q1
        AisAplusB                       ;form (q3*z+q2)*z+q1

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve z
        AisAtimesB                      ;form ((q3*z+q2)*z+q1)*z

        LDMIA   w5!, {b1,b2,b_exp}      ;get q0
        AisAplusB                       ;and form Q(Z)

; We now have Q(Z), so make G*P(Z)

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrive z before stack drops
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save Q(Z)
        MOV     a_sign, #0              ;get p2 (which is positive)
        LDMIA   w5!, {a1,a2,a_exp}
        AisAtimesB                      ;form p2*Z

        LDMIA   w5!, {b1,b2,b_exp}      ;get p1
        AisAplusB                       ;form p2*z+p1

        ADD     b_sign, r12, #4*4       ;point at z
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrive z
        AisAtimesB                      ;form (p2*z+p1)*z

        LDMIA   w5!, {b1,b2,b_exp}      ;get p0
        AisAplusB                       ;and form P(Z)

        ADD     b_sign, r12, #8*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G

        AisAtimesB                      ;and form G*P(Z)

; We now have Q(Z) and G*P(Z), so form R(G) = 0.5+G*P(Z)/(Q(Z)-G*P(Z))

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve Q(Z)
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save G*P(Z)
        EOR     a_sign, a_sign, #&80000000 ;a = -G*P(Z)
        AisAplussignB                   ;form Q(Z)-G*P(Z)

        MOV     b_sign, a_sign          ;move a to b
        MOV     b1, a1
        MOV     b2, a2
        MOV     b_exp, a_exp

        LDMIA   r12!, {a_sign,a1,a2,a_exp} ;retrieve G*P(Z)
        AisAoverB                       ;form G*P(Z)/(Q(Z)-G*P(Z))

        LDMIA   w5!, {b1,b2,b_exp}      ;load 0.5
        AisAplusB                       ;and form R(G)
        ADD     r12, r12, #8*4          ;tidy stack (remove Z and G)

; We now have R(G), so retrieve N and use it to set up exponent

        LDMIA   r12!, {w1,r9,r14}       ;retrieve N, r9 and LR
        ADD     w1, w1, #1              ;increment N
        ADD     a_exp, a_exp, w1        ;set up exponent

        MOVS    w1, #0                  ;set Z bit to indicate success
        MOV     r15, r14                ;and go home

return_1_exp

; We have such a small arguament that we wish to return 1

        MOVS    a_sign, #0              ;set positive and Z for success
        MOV     a1, #&80000000          ;set up mantissa for 1
        MOV     a2, #0
        MOV     a_exp, #&4000           ;and set up exponent
        SUB     a_exp, a_exp, #1
        MOV     r15, r14                ;and go home

problem_a

; We have overflow/underflow, so load with correct event mask and clear Z 

        TST     a_sign, #&80000000      ;have we overflow or underflow?

        MOV     a_sign, #0              ;if either, sign +ve,
        MOV     a1, #0                  ;mantissa zero
        MOV     a2, #0

        MOVNE   a_exp, #0               ;if underflow, return 0 if no trap
        MOVNE   r4, #UFL_E_MASK         ;and indicate underflow

        MOVEQ   a_exp, #&ff             ;if overflow, return inf. if no trap
        ORREQ   a_exp, a_exp, a_exp, LSL #7
        MOVEQS  r4, #OFL_E_MASK         ;indicate overflow and clear Z

        MOV     r15, r14                ;before going home


;******************************************************************************
;
;       Data for calculating EXP(X)
;

max_check_exp

        DCD     &b17217f7               ;LGN(max. fp. no.)
        DCD     &d1cf79ab
        DCD     &0000400c

        DCD     &80000000
        DCD     &0
        DCD     &00003fbe

min_check_exp

        DCD     &b21dfe7f               ;-LGN(min. fp. no.)
        DCD     &09e2baa9
        DCD     &0000400c

        DCD     &80000000
        DCD     &0
        DCD     &00003fbe

exp_coeff

        DCD     &b8aa3b29               ;1/LGN(2)
        DCD     &5c17f0bc               ;=1.4426 95040 88896 34074
        DCD     &00003fff


        DCD     &de8082e3               ;-C2
        DCD     &08654362               ;=2.1219 44400 54690 58277 E-4
        DCD     &00003ff2

        DCD     &b1800000               ;C1
        DCD     &0                      ;=0.543 octal
        DCD     &00003ffe


        DCD     &c99b1867               ;q3
        DCD     &2822a93e               ;=0.75104 02839 98700 46114 E-6
        DCD     &00003fea

        DCD     &a57862e1               ;q2
        DCD     &46a6fb39               ;=0.63121 89437 43985 03557 E-3
        DCD     &00003ff4

        DCD     &e8b9428e               ;q1
        DCD     &fecff592               ;=0.56817 30269 85512 21787 E-1
        DCD     &00003ffa

        DCD     &80000000               ;q0
        DCD     &0
        DCD     &00003ffe


        DCD     &845a2157               ;p2
        DCD     &3490f106               ;=0.31555 19276 56846 46356 E-4
        DCD     &00003ff0

        DCD     &f83a5f91               ;p1
        DCD     &50952c99               ;=0.75753 18015 94227 76666 E-2
        DCD     &00003ff7

        DCD     &80000000               ;p0
        DCD     &0                      ;=0.25 E+0
        DCD     &00003ffd


        DCD     &80000000               ;0.5
        DCD     &0
        DCD     &00003ffe

;******************************************************************************

filelevel       SETA    filelevel - 1
        END
