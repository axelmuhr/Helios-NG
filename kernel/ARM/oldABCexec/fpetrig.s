; -> fpetrig/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2trig        19/2/87  Martin Clemoes
;
;       Description:    This file contains implementations of the SIN, COS and 
;                       TAN instructions, making use of subroutines in
;                       FPE2utils (so must be assembled after this).
;
;******************************************************************************
;
;       A NaN or two returned by untrapped invalid operations
;

badtanNaN       *       &C8000000       ;a tangent of too large arguament
badsinNaN       *       &C9000000       ;a SIN or COS of too large argument
badasinNaN      *       &CA000000       ;a SIN or COS of too large argument
badpolNaN       *       &CB000000       ;a POL of 0/0 or inf/inf.

;******************************************************************************
;
;       TAN routines
;
tan3

        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

tan

; First check to see if source a const., and if not, load source reg.

        MOVS    r7, r9, LSL #28         ;separate fm bits
        BMI     tan_constant            ;we've got a constant...

        ADD     r8, r12, r7, LSR #24    ;hence point at source reg.

        LDMIA   r8, {r0-r3}             ;load up source

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U
        STMNEIA r8, {r0-r3}             ;and write back if converted

; Now check for NaN, infinity

        MOV     r4, r3, LSL #17
        ADDS    r4, r4, #&00020000      ;is exponent maximum value?
        BEQ     max_exp_tan             ;yes, so must be one of funnies...

got_const_tan

        STMDB   r12!, {r14}
        BL      tan_rout                ;get tangent, Z cleared if ofl/ufl
        LDMIA   r12!, {r14}
        BEQ     exit_log                ;no overflow/underflow, so carry on...

; We have overflow or underflow, so deal with this

        BADDR   r7, exit_log1           ;return to here if no exception
        B       check_exep              ;and check for exception (mask in r4)

max_exp_tan

; The arguament has max. exponent value - infinity or NaN

        ORRS    r4, r1, r2              ;have we infinity?
        MOVEQ   r1, #badtanNaN          ;yes, so return correct NaN
        B       NaN_add                 ;must be a NaN...

tan_rout

; Subroutine that does a = tan a, assuming nan and infinity 
; already weeded out. Calculation performed completely in extended precision.
; The algorithm implemented is described in Cody & Waite, chapter 9.

; First check not too big for accuracy

        ADR     w1, max_check_tan       ;check that no overflow
        LDMIA   w1!, {b1,b2,b_exp}      ;load max magnitude value
        CMP     a_exp, b_exp            ;is this exceeded?
        CMPEQ   a1, b1
        CMPEQ   a2, b2
        BHI     problem_a_tan           ;yes, so object

        MOVS    a1, a1                  ;If denormalised,
        BPL     tan_norm                ;normalise...

norm_tan

; We have a suitable value, so next calculate F

        STMDB   r12!, {a_sign,a1,a2,a_exp,w1,r9,r14} ;save X, r9, r14 and 1 gap
        TST     r9, #&00100000          ;clear Z if being used as COS subrout.

        ADR     w5, tan_coeff           ;get 2/PI
        MOV     b_sign, #0              ;(which is positive)
        LDMIA   w5!, {b1,b2,b_exp}
        AisAtimesB                      ;form X*2/PI
        BisintrndA                      ;hence b = XN, w1 = ABS(N)

        BLNE    cos_bodge               ;if COS, go and do PI/4 bodge
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
        AisAplussignB                   ;form F = X1-C1*XN+X2-C2*XN

; We have got F, so form G and then Q(G), first testing for too tiny F

        MOV     w1, #&3f00              ;w1=min. allowable exponent
        ORR     w1, w1, #&df
        CMP     a_exp, w1               ;have we an allowable exponent?
        BLT     tiny_f                  ;no, so deal with this

        STMIA   r12, {a_sign,a1,a2,a_exp} ;save F, tidying stack at same time
        AisAsquared                     ;form G
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save G

        MOV     b_sign, #0              ;load q4 (which is positive)
        LDMIA   w5!, {b1,b2,b_exp}
        AisAtimesB                      ;form q4*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q3
        AisAminusB                      ;form q4*G+q3

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (q4*G+q3)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q2
        AisAplusB                       ;form (q4*G+q3)*G+q2

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form ((q4*G+q3)*G+q2)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q1
        AisAminusB                      ;form ((q4*G+q3)*G+q2)*G+q1

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (((q4*G+q3)*G+q2)*G+q1)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q0
        AisAplusB                       ;and form Q(G)

; We now have Q(G), so make F*P(G)

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrive G before stack drops
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save Q(G)
        MOV     a_sign, #&80000000      ;get p3 (which is negative)
        LDMIA   w5!, {a1,a2,a_exp}
        AisAtimesB                      ;form p3*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p2
        AisAplusB                       ;form p3*G+p2

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (p3*G+p2)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p1
        AisAminusB                      ;form (p3*G+p2)*G+p1

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form P(G)-1

        ADD     b_sign, r12, #8*4       ;point at F
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve F

        AisAtimesB                      ;and form F*P(G)-F

        ADD     b_sign, r12, #8*4       ;point at F
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve F

        AisAplussignB                   ;and form F*P(G)

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve Q(G)

; We now have b=Q(G) and a=F*P(G), so form result = +/- [F*P(G)/Q(G)]^+/-1

numandden
        ADD     r12, r12, #8*4          ;tidy stack (remove G and F)
        LDMIA   r12!, {w1}              ;retrieve N
        TST     w1, #1                  ;is it even?
        EORNE   a_sign, a_sign, #&80000000 ;no, so negate
        swapAandB       NE              ;and swap a and b

        ADDS    w1, b_exp, #&8000       ;make sure no divide by tiny amount
        BMI     overflow_tn             ;if it is, overflow
        CMP     b_exp, #0               ;ditto divide by zero
        ORREQS  w1, b1, b2
        BEQ     overflow_tn

        AisAoverB                       ;then form result

        LDMIA   r12!, {r9,r14}          ;retrieve regs.,
        MOVS    w1, #0                  ;set Z bit to indicate success
        MOV     r15, r14                ;and go home

tiny_f

; We have a tiny f, so no need to calculate series, just set up and jump

        SUB     r12, r12, #4*4          ;set stack level correctly
        MOV     b_sign, #0              ;xden=b=1.0
        MOV     b1, #&80000000
        MOVS    b2, #0,2
        RSC     b_exp, b2, #&4000
        B       numandden               ;and jump back into action...

tan_norm
        ORRS    w1, a1, a2              ;if 0
        BEQ     norm_tan                ;go back...
tan_norm1
        SUB     a_exp, a_exp, #1        ;decrement exponent

        ADDS    a2, a2, a2              ;double mantissa
        ADCS    a1, a1, a1
        BPL     tan_norm1
        B       norm_tan

problem_a_tan

; We have operand too big for accuracy, so return NaN, load with correct event
; mask and clear Z 

        MOV     a1, #badtanNaN          ;mantissa contains NaN
        MOVS    a2, #0,2
        RSCS    a_exp, a2, #&00008000   ;max. exponent, Z bit cleared
        MOV     r4, #IVO_E_MASK         ;indicate NaN coming
        MOV     r15, r14                ;and go home

overflow_tn

        MOV     a1, #0                  ;if untrapped, return infinity
        MOVS    a2, #0,2
        RSCS    a_exp, a_exp, #&8000
        MOV     r4, #OFL_E_MASK
        LDMIA   r12!, {r9,r14}
        MOV     r15, r14

cos_bodge

; We are calculating t for cos, so we want t=tan(X/2 + PI/4) - fiddle with N
; and XN to make X apparently PI/4 bigger

        ADD     w1, w1, #1              ;alter sign
        CMP     b_exp, #0               ;was XN zero?
        MOVEQ   b1, #&80000000          ;if so, return 0.5, sign as X
        MOV     w3, #&4000
        SUBEQ   b_exp, w3, #2
        MOVEQ   r15, r14                ;and go home

        SUB     w3, w3, #2              ;exponent of 0.5 now in w3
        SUB     w3, b_exp, w3           ;hence difference in exponents
        MOV     w2, #&80000000          ;mantissa of 0.5

        CMP     a_exp, b_exp            ;now see if reduced range arg will be
        CMPEQ   a1, b1                  ;positive
        CMPEQ   a2, b2

        ADDCS   b1, b1, w2, LSR w3      ;if so, subtr. PI/4 by adding 0.5 to XN
        MOVCS   r15, r14

        SUBS    b1, b1, w2, LSR w3      ;else add PI/4 by subtr. 0.5 from XN
        MOVPL   b1, b1, LSL #1          ;making sure normalised
        SUBPL   b_exp, b_exp, #1
        SUB     w1, w1, #1              ;making sure sign correct
        MOV     r15, r14                ;then go home

;******************************************************************************
;
;       Data for calculating tan(X)
;

max_check_tan

        DCD     &c90fdaa2               ;YMAX = INT(PI/2 * 2^30)
        DCD     &0
        DCD     &0000401d

tan_coeff

        DCD     &a2f9836e               ;2/PI
        DCD     &4e44152a               ;=0.63661 97723 67581 34308
        DCD     &00003ffe


        DCD     &95777a5c               ;-C2
        DCD     &f72cece6               ;=4.4544 55103 38076 86783 08 E-6
        DCD     &00003fed

        DCD     &c9100000               ;C1
        DCD     &0                      ;=1.57080 07812 5
        DCD     &00003fff


        DCD     &85bba783               ;q4
        DCD     &b3c748a9               ;=0.49819 43399 37865 12270 E-6
        DCD     &00003fea

        DCD     &a37b24c8               ;-q3
        DCD     &4a42092e               ;=0.31181 53190 70100 27307 E-3
        DCD     &00003ff3

        DCD     &d23cf50b               ;q2
        DCD     &f10aca84               ;=0.25663 83228 94401 12864 E-1
        DCD     &00003ff9

        DCD     &eef5823f               ;-q1
        DCD     &decea969               ;=0.46671 68333 97552 94240 E+0
        DCD     &00003ffd

        DCD     &80000000               ;q0
        DCD     &0                      ;=1.0
        DCD     &00003fff


        DCD     &95d5b975               ;-p3
        DCD     &16391da8               ;=0.17861 70734 22544 26711 E-4
        DCD     &00003fef

        DCD     &e0741531               ;p2
        DCD     &dd56f650               ;=0.34248 87823 58905 89960 E-2
        DCD     &00003ff6

        DCD     &8895af2a               ;-p1
        DCD     &6847fcd5               ;=0.13338 35000 64219 60681 E+0
        DCD     &00003ffc

;******************************************************************************
;
;       SIN and COS routine
;
sin3
cos3

        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

sin
cos

; SIN and COS are calculated using t=tan(x/2), SIN(x) = 2t/(1+t^2),
; COS(x) = SIN(x + PI/2) = -SIN(x - PI/2)

; First check to see if source a const., and if not, load source reg.

        MOVS    r7, r9, LSL #28         ;separate fm bits
        BMI     sin_constant            ;we've got a constant...

        ADD     r8, r12, r7, LSR #24    ;hence point at source reg.

        LDMIA   r8, {r0-r3}             ;load up source

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U
        STMNEIA r8, {r0-r3}             ;and write back if converted

; Now check for NaN, infinity

        MOV     r4, r3, LSL #17
        ADDS    r4, r4, #&00020000      ;is exponent maximum value?
        BEQ     max_exp_sin             ;yes, so must be one of funnies...

got_const_sin

; If COS, +/-SIN(x +/- PI/2) wanted, but bodge in tan_rout takes care of this
; However, use COS(-x) = COS(x) first

        BIC     a_sign, a_sign, r9, LSL #11 ;if COS, make X positive
        STMDB   r12!, {r14}             ;then save r14

; Now halve x and calculate t=tan(x/2)

        ORRS    w1, a1, a2              ;if not zero,
        SUBNE   a_exp, a_exp, #1        ;halve by decrementing exponent

        BL      tan_rout                ;get tangent, Z cleared if ofl/ufl

        BNE     problem_sin             ;tan_rout had too big operand...

; We have t, so calculate 2t/(1+t^2)

        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save t for later
        AisAsquared                     ;make t^2

        MOV     b1, #&80000000          ;put 1.0 in b
        MOVS    b2, #0,2
        RSC     b_exp, b2, #&4000
        AisAplusB                       ;so we have a=1+t^2

        MOV     b_sign, a_sign          ;put 1+t^2 in b
        MOV     b1, a1
        MOV     b2, a2
        MOV     b_exp, a_exp
        LDMIA   r12!, {a_sign,a1,a2,a_exp} ;retrieve t
        AisAoverB                       ;form t/(1+t^2)
        ORRS    w1, a1, a2
        ADDNE   a_exp, a_exp, #1        ;hence answer

; Tidy up and return answer

        LDMIA   r12!, {r14}             ;retrieve lr
        B       exit_log                ;if no underflow, return

max_exp_sin

        ORRS    r4, r1, r2              ;is this a NaN?
        MOVEQ   r1, #badsinNaN          ;if not, infinity, so make one
        B       NaN_add                 ;yes, so return this...

problem_sin

; Either too big an arguament, or t overflowed - when return 0

        LDMIA   r12!, {r14}             ;tidy stack & retrieve lr
        CMP     r4, #OFL_E_MASK         ;did t overflow?
        MOVEQ   r3, #0                  ;yes, so return 0
        MOVEQ   r0, #0
        BEQ     exit_log

; The arguament has value so big that accuracy lost - return NaN

        MOV     r1, #badsinNaN          ;yes, so return correct NaN
        MOVS    r2, #0,2
        RSC     r3, r2, #&8000
        B       NaN_add                 ;must be a NaN...

;******************************************************************************
;
;       Data for calculating SIN & COS
;


;        DCD     &c90fdaa2               ;PI/2
;        DCD     &2168c235               ;=1.57079 63267 94896 61923 132
;        DCD     &00003fff


;******************************************************************************
;
;       ASIN and ACOS routine
;
asn3
acs3

        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

asn
acs

; First check to see if source a const., and if not, load source reg.

        MOVS    r7, r9, LSL #28         ;separate fm bits
        BMI     asin_constant           ;we've got a constant...

        ADD     r8, r12, r7, LSR #24    ;hence point at source reg.

        LDMIA   r8, {r0-r3}             ;load up source

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U
        STMNEIA r8, {r0-r3}             ;and write back if converted

; Now check for NaN, infinity or operand > 1.0

        MOVS    r4, r3, LSL #17         ;is exponent too big?
        BEQ     zero_exp_asin           ;no, too small...
        ADDPLS  r4, r4, #&00020000
        BMI     max_exp_asin            ;yes, so must be one of funnies...
                                        ;C bit will be clear on jump
got_const_asin

        STMDB   r12!, {a_sign,r9,r14}   ;save important regs.

; The algorithm for ASIN and ACOS is described in Cody & Waite, ch. 10

; Start with Y = |X|

        MOV     a_sign, #0              ;Y = |X|

; Compare with 0.5

        MOV     r5, #&4000
        SUB     r5, r5, #2
        CMP     r3, r5                  ;is Y >= 0.5?
        BGE     big_asin                ;yes, so range reduction needed...

; We have less than 0.5, so no range reduction needed, but check not tiny

        AND     w1, r9, #&00400000      ;get bit indicating ACOS
        STMDB   r12!, {w1}              ;and push it (i in C&W)

        MOV     w1, #&4000
        SUB     w1, w1, #33             ;minimum exponent allowed
        CMP     a_exp, w1               ;for series expansion
        BLT     tiny_asin               ;less, so just return Y

; We have Y, so set up Y and G=Y*Y on stack, then go and expand series

        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save Y
        AisAsquared                     ;form G
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save G
        B       series_asin             ;go and expand series...

big_asin

; Range reduction needed before expanding series

        AND     w1, r9, #&00400000      ;get bit indicating ACOS
        EOR     w1, w1, #&00400000      ;reverse it
        STMDB   r12!, {w1}              ;and push it (i in C&W)

        MOV     a_sign, #&80000000      ;a = -Y
        MOV     b1, #&80000000          ;b = 1
        MOVS    b2, #0,2
        RSC     b_exp, b2, #&4000
        AisAplusB                       ;hence form 1-Y

        ORRS    w1, a1, a2              ;have we zero?
        BEQ     tiny_asin               ;yes, so don't expand series...

        SUB     a_exp, a_exp, #1        ;and G = (1-Y)/2
        SUB     r12, r12, #4*4          ;leave room on stack for Y
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save G

        AisrootA
        EOR     a_sign, a_sign, #&80000000 ;Y = -2*SQT(G)
        ADD     a_exp, a_exp, #1
        ADD     w1, r12, #4*4           ;point at gap on stack
        STMIA   w1, {a_sign,a1,a2,a_exp} ;hence save Y on stack

        LDMIA   r12, {a_sign,a1,a2,a_exp} ;reload a with G

series_asin

; We have G and Y, so evaluate Q(G) next (q5=1.0, so don't bother with that)

        ADR     w5, asin_coeff          ;point at series coefficients

        LDMIA   w5!, {b1,b2,b_exp}      ;get q4
        AisAminusB                      ;form q5*G+q4

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (q5*G+q4)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q3
        AisAplusB                       ;form (q5*G+q4)*G+q3

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form ((q5*G+q4)*G+q3)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q2
        AisAminusB                      ;form ((q5*G+q4)*G+q3)*G+q2

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (((q5*G+q4)*G+q3)*G+q2)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q1
        AisAplusB                       ;form (((q5*G+q4)*G+q3)*G+q2)*G+q1

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form ((((q5*G+q4)*G+q3)*G+q2)*G+q1)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q0
        AisAminusB                      ;and form Q(G)

; We have Q(G), so next form G*P(G)

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrive G before stack drops
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save Q(G)
        MOV     a_sign, #&80000000      ;get p5 (which is negative)
        LDMIA   w5!, {a1,a2,a_exp}
        AisAtimesB                      ;form p5*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p4
        AisAplusB                       ;form p5*G+p4

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (p5*G+p4)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p3
        AisAminusB                      ;form (p5*G+p4)*G+p3

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form ((p5*G+p4)*G+p3)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p2
        AisAplusB                       ;form ((p5*G+p4)*G+p3)*G+p2

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (((p5*G+p4)*G+p3)*G+p2)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p1
        AisAminusB                      ;form (((p5*G+p4)*G+p3)*G+p2)*G+p1

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form G*P(G)

; We have Q(G) and G*P(G), so form R(G)=G*P(G)/Q(G), then Y+Y*R(G)

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve Q(G)
        ADD     r12, r12, #4*4          ;and tidy G off stack
        AisAoverB                       ;form R(G)

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve Y
        AisAtimesB                      ;form Y*R(G)

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve Y
        AisAplussignB                   ;form Y+Y*R(G)

tiny_asin

; We have got an initial result in a, so tidy up for ASIN or ACOS as wanted

        LDMIA   r12!, {w1,w2,r9}        ;retrieve i,sign of X,r9
        TST     r9, #&00400000          ;is this ACOS?
        BEQ     asin_end                ;no, ASIN...

        TST     w2, #&80000000          ;was X negative?
        ADREQ   w2, a_array             ;no, so use a_array
        ADRNE   w2, b_array             ;yes, so use b_array
        ADD     w2, w2, w1, LSR #18     ;point at i'th element
        EOREQ   a_sign, a_sign, #&80000000 ;if X positive, negate result

        LDMIA   w2, {b1,b2,b_exp}       ;get array element
        AisAplusB                       ;and add it in
;        LDMIA   w2, {b1,b2,b_exp}    
;        AisAplusB                       ;twice

        LDMIA   r12!, {r14}             ;retrieve r14
        B       exit_log                ;if no underflow, return

asin_end

; We have ASIN, so follow exit path for this

        TST     w2, #&80000000          ;was X negative?

        ADR     w2, a_array             ;use a_array
        ADD     w2, w2, w1, LSR #18     ;point at i'th element

        LDMIA   w2, {b1,b2,b_exp}       ;get array element
        AisAplusB                       ;and add it in
;        LDMIA   w2, {b1,b2,b_exp}   
;        AisAplusB                       ;twice

        EORNE   a_sign, a_sign, #&80000000 ;if X negative, negate answer

        LDMIA   r12!, {r14}             ;retrieve r14
        B       exit_log                ;if no underflow, return

zero_exp_asin

        ORRS    r5, r1, r2              ;is this zero or denormalised?
        BEQ     got_const_asin          ;zero, so carry on...
        MOVS    a1, a1                  ;is this denormalised?
        BMI     got_const_asin          ;no, so carry on...

norm_asin

        SUB     a_exp, a_exp, #1        ;decrement exponent

        ADDS    a2, a2, a2              ;double mantissa
        ADCS    a1, a1, a1
        BPL     norm_asin               ;until ms bit set...
        B       got_const_asin          ;then go back...

max_exp_asin

        RSCS    r5, r3, #&4000          ;is exponent &3fff?
        ORREQS  r5, r2, r1, LSL #1      ;is X 1.0 exactly?
        BEQ     got_const_asin          ;yes, so go back...

        ADDS    r4, r4, #&00020000      ;have we max. exponent?
        BNE     bad_asin
        ORRS    r4, r1, r2              ;is this a NaN?
        BNE     NaN_add                 ;yes, so return this...

bad_asin

        MOV     r1, #badasinNaN         ;make a NaN
        MOVS    r2, #0,2
        RSC     r3, r2, #&8000
        B       NaN_add                 ;return this...

;******************************************************************************
;
;       Data for calculating ASIN, ACOS
;

;tinyy_asin
;
;        DCD     &80000000               ;eps, =2^-32
;        DCD     &0
;        DCD     &00003fdf

asin_coeff

        DCD     &be974377               ;-q4
        DCD     &cc30f9e6               ;=0.23823 85915 36702 38830 E+2
        DCD     &00004003

        DCD     &96f3e4b2               ;q3
        DCD     &c8e37cbc               ;=0.15095 27084 10306 04719 E+3
        DCD     &00004006

        DCD     &beee77e2               ;-q2
        DCD     &b5423cf3               ;=0.38186 30336 17501 49284 E+3
        DCD     &00004007

        DCD     &d0927880               ;q1
        DCD     &f5c2170b               ;=0.41714 43024 82604 12556 E+3
        DCD     &00004007

        DCD     &a43601f1               ;-q0
        DCD     &5c3e6196               ;=0.16421 09671 44985 60795 E+3
        DCD     &00004006


        DCD     &b25dedaf               ;-p5
        DCD     &30f3242c               ;=0.69674 57344 73506 46411 E+0
        DCD     &00003ffe

        DCD     &a270bb27               ;p4
        DCD     &61c93957               ;=0.10152 52223 38064 63645 E+2
        DCD     &00004002

        DCD     &9ec1654d               ;-p3
        DCD     &36d4f820               ;=0.39688 86299 75048 77339 E+2
        DCD     &00004004

        DCD     &e4d539b0               ;p2
        DCD     &56a451ad               ;=0.57208 22787 78917 31407 E+2
        DCD     &00004004

        DCD     &daf2ad41               ;-p1
        DCD     &d05311c4               ;=0.27368 49452 41642 55994 E+2
        DCD     &00004003

a_array

        DCD     &0                      ;a(0) = 0.0
        DCD     &0
        DCD     &0
        DCD     &0

        DCD     &c90fdaa2               ;a(1) = PI/2
        DCD     &2168c235
        DCD     &00003fff

b_array
pi                                      ;pi for atan as well

        DCD     &c90fdaa2               ;b(0) = PI
        DCD     &2168c235
        DCD     &00004000
        DCD     &0

        DCD     &c90fdaa2               ;b(1) = PI/2
        DCD     &2168c235
        DCD     &00003fff

;******************************************************************************
;
;       ATN and POL (ATAN2) routines
;

pol3
        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

pol

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
        BMI     pol_constant            ;it's a constant...

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r6, r12, r6, LSR #24    ;point at fm store
        LDMIA   r6, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r6, {r0-r3}             ;write back in U type if converted

got_const_pol

; Retrieve sign of b again

        MOV     r7, r11
        BIC     r11, r11, #&80000000

; Now check to see if either value is zero or denormalised or infinity or NaN

        MOVS    r4, r3, LSL #17         ;could fn be zero?
        ADDNES  r4, r4, #&00020000      ;could fn be infinity or NaN?
        MOVNES  r5, r11, LSL #17        ;or fm be zero?
        ADDNES  r5, r5, #&00020000      ;could fm be infinity or NaN?

not_zero_pol

        STMNEDB r12!, {a_sign,b_sign,r9,r14}
        AisAoverB       NE              ;no...
        BNE     atan_entry              ;no...

; We have max or min exponent - infinity, NaN, zero or denormalised

        MOVS    r4, r3, LSL #17         ;could a be zero or denormalised?
        BNE     a_not_zero              ;no...

        ORRS    r5, r1, r2              ;a denormalised?
        BNE     normalise_a             ;yes, so normalise...

; a is zero, so check b for zero or NaN

        MOVS    r5, r11, LSL #17        ;is b zero?
        ORREQS  r6, r8, r10

        BEQ     bad_pol                 ;yes, 0/0, so object

        ADDS    r5, r5, #&00020000      ;could b be a NaN?
        BNE     underflow_pol           ;no, so division underflows
        ORRS    r6, r8, r10             ;is b infinity?
        BEQ     underflow_pol           ;yes, so division underflows

        MOV     r1, r8                  ;b is a NaN, so return b
        MOV     r2, r10
        MOV     r3, r11
        B       NaN_add

normalise_a

; a is denormalised, so normalise so that division will work

        ADDS    r2, r2, r2              ;double mantissa
        ADCS    r1, r1, r1
        SUB     r3, r3, #1              ;decrement exponent
        BPL     normalise_a             ;and repeat as necessary

a_not_zero

        MOVS    r5, r11, LSL #17        ;could b be zero or denormalised?
        BNE     b_not_zero              ;no...

        ORRS    r6, r8, r10             ;b denormalised?
        BNE     normalise_b             ;yes, so normalise

; b is zero, so unless a is a NaN, division overflows

        ADDS    r4, r4, #&00020000      ;has a got max exponent?
        BNE     overflow_pol            ;no...

        ORRS    r6, r1, r2              ;hence is a a NaN?
        BNE     NaN_add                 ;yes, so return that...

overflow_pol

; result = pi/2, then jump to atan_entryC

        MOV     w1, a_sign              ;sign of V in w1
        MOV     w2, b_sign              ;sign of U in w2
        ADR     a_sign, piby2           ;a=pi/2
        LDMIA   a_sign, {a1,a2,a_exp}
        B       atan_entryC

underflow_pol

; result = 0, then jump to atan_entryB

        MOV     w1, a_sign              ;sign of V in w1
        MOV     w2, b_sign              ;sign of U in w2
        MOV     a_sign, #0
        MOV     a1, #0
        MOV     a2, #0
        MOV     a_exp, #0
        B       atan_entryB

bad_pol

; We have an error in the division part of pol, so return NaN

        MOV     a1, #badpolNaN          ;return NaN
        MOVS    a2, #0,2
        RSC     a_exp, a2, #&8000
        B       NaN_add

normalise_b

; b denormalised, so normalise it to make division work

        ADDS    r10, r10, r10           ;double mantissa
        ADCS    r8, r8, r8
        SUB     r11, r11, #1            ;decrement exponent
        BPL     normalise_b             ;and carry on as required

b_not_zero

; Neither a or b is zero or denormalised, but deal with infinity or NaN

        ADDS    r6, r5, #&00020000      ;could a or b be infinity or NaN?
        ADDNES  r6, r4, #&00020000
        BNE     not_zero_pol            ;no, so go back to action

        ADDS    r6, r4, #&00020000      ;was it a?
        BNE     b_trouble_pol           ;no, so b must be trouble

        ORRS    r6, r1, r2              ;is a infinity?
        BNE     NaN_add                 ;no, so must be NaN...

; a is infinity - overflow unless b infinity or NaN as well

        ADDS    r6, r5, #&00020000      ;is b troublesome?
        BNE     overflow_pol            ;no, so return a

        ORRS    r6, r8, r10             ;is b infinity?
        MOVEQ   r1, #badpolNaN          ;yes, so return correct NaN
        BEQ     NaN_add

b_trouble_pol

        ORRS    r6, r8, r10             ;is b a NaN?
        BEQ     underflow_pol              ;no - infinity - so underflow

        MOV     r1, r8                  ;yes, so return this NaN
        MOV     r2, r10
        MOV     r3, r11
        B       NaN_add


atn3

        TST     r9, #&80                ;illegal dest. precision?
        BNE     very_ill_instruction    ;yes...

atn

; First check to see if source a const., and if not, load source reg.

        MOVS    r7, r9, LSL #28         ;separate fm bits
        BMI     atn_constant            ;we've got a constant...

        ADD     r8, r12, r7, LSR #24    ;hence point at source reg.

        LDMIA   r8, {r0-r3}             ;load up source

; Now make sure of type U, to make integer conversion easy

        MOVS    r4, r3, LSR #26         ;test for U type, and if not, set up r4
        ensuretype                      ;if not U, convert to U
        STMNEIA r8, {r0-r3}             ;and write back if converted

; Now check for NaN, infinity, zero or denorm

        MOVS    r4, r3, LSL #17
        ADDNES  r4, r4, #&00020000      ;is exponent extreme value?
        BEQ     funny_atn               ;yes, so must be one of funnies...

got_const_atn

        STMDB   r12!, {a_sign,b_sign,r9,r14} ;save sign X, gap, r9 & r14

atan_entry

; We have now got to meeting of pol and atn.
; The following algorithm is as described in Cody & Waite, ch. 11

        MOVS    a_sign, #0,2            ;f = |x|, C=0

        RSCS    w1, a_exp, #&4000       ;is f>=1?
        AisAinverted    LE              ;yes, so a = 1/a,
        MOVLE   w1, #2                  ;N=2
        MOVGT   w1, #0                  ;else N=0

        ADR     w5, twoMroot3           ;get 2-root3 in b
        LDMIA   w5!, {b1,b2,b_exp}
        CMP     a_exp, b_exp            ;and compare with f
        CMPEQ   a1, b1
        CMPEQ   a2, b2

        ADDGT   w1, w1, #1              ;if greater, N = N + 1
        STMDB   r12!, {w1}              ;save N

        BLE     small_f_atn             ;if less than or equal, no fiddle

        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save f
        LDMIA   w5, {b1,b2,b_exp}       ;get root 3
        AisAplusB                       ;and form root3 + f
        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve f
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save root3 + f

        MOV     a_sign, #0              ;get root3 again
        LDMIA   w5!, {a1,a2,a_exp}      ;which is positive
        AisAtimesB                      ;hence form root3 * f
        MOV     b1, #&80000000          ;b = 1.0
        MOVS    b2, #0,2
        RSC     b_exp, b2, #&4000
        AisAminusB                      ;hence form (root3 * f) - 1
        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve root3 + f
        AisAoverB                       ;and form f = ((root3*f) - 1)/(root3+f)

small_f_atn

; We now have f as to be used for series expansion, so check not too tiny next

        MOV     w1, #&4000
        SUB     w1, w1, #33             ;minimum exponent allowed
        CMP     a_exp, w1               ;for series expansion
        BLT     tiny_atn                ;less, so just return f

; Now form G = f^2 and evaluate series Q(G) (q4 = 1.0)

        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save f
        AisAsquared                     ;form G
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;save G

        ADR     w5, atn_coeff           ;point at series coefficients

        LDMIA   w5!, {b1,b2,b_exp}      ;get q3
        AisAplusB                       ;form q4*G+q3

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (q4*G+q3)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q2
        AisAplusB                       ;form (q4*G+q3)*G+q2

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form ((q4*G+q3)*G+q2)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q1
        AisAplusB                       ;form ((q4*G+q3)*G+q2)*G+q1

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (((q4*G+q3)*G+q2)*G+q1)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get q0
        AisAplusB                       ;form (((q4*G+q3)*G+q2)*G+q1)*G+q0

; We have Q(G), so next form G*P(G)

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrive G before stack drops
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save Q(G)
        MOV     a_sign, #&80000000      ;get p3 (which is negative)
        LDMIA   w5!, {a1,a2,a_exp}
        AisAtimesB                      ;form p3*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p2
        AisAminusB                      ;form p3*G+p2

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form (p3*G+p2)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p1
        AisAminusB                      ;form (p3*G+p2)*G+p1

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form ((p3*G+p2)*G+p1)*G

        LDMIA   w5!, {b1,b2,b_exp}      ;get p0
        AisAminusB                      ;form ((p3*G+p2)*G+p1)*G+p0

        ADD     b_sign, r12, #4*4       ;point at G
        LDMIA   b_sign, {b_sign,b1,b2,b_exp} ;retrieve G
        AisAtimesB                      ;form G*P(G)

; We have Q(G) and G*P(G), so form R(G)=G*P(G)/Q(G), then f+f*R(G)

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve Q(G)
        ADD     r12, r12, #4*4          ;and tidy G off stack
        AisAoverB                       ;form R(G)

        LDMIA   r12, {b_sign,b1,b2,b_exp} ;retrieve f
        AisAtimesB                      ;form f*R(G)

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve f
        AisAplussignB                   ;form f+f*R(G)

tiny_atn

; We have an initial result, so next adjust for value of N

        ADR     w5, atn_array           ;point at array in case jumped here

        LDMIA   r12!, {w1}              ;retrieve N

        EOR     a_sign, a_sign, w1, LSL #30 ;if N=2 or 3, negate

        ADD     w5, w5, w1, LSL #4      ;point at A(N)
        LDMIA   w5, {b1,b2,b_exp}       ;get it
        AisAplusB                       ;and add it in

; Now retrieve information from stack and tidy up

        LDMIA   r12!, {w1,w2,r9,r14}    ;get sign V/X,sign U,r9 and lr

        TST     r9, #&00100000          ;are we doing a pol?

        EORNE   a_sign, a_sign, w1      ;no, so if x was negative, negate,
        BNE     exit_log                ;then go home

atan_entryB

; We are doing a pol, so tidy up for this

        TST     w2, #&80000000          ;is U negative?
        BEQ     atan_entryC             ;no, so jump past fiddle...

        STMDB   r12!, {w1,r14}          ;save sign of V, lr

        EOR     a_sign, a_sign, #&80000000 ;negate result so far
        ADR     b_sign, pi              ;get pi (positive)
        LDMIA   b_sign, {b1,b2,b_exp}
        AisAplusB                       ;result = pi - result

        LDMIA   r12!, {w1,r14}          ;retrieve sign of V, lr

atan_entryC

        EOR     a_sign, a_sign, w1      ;if V negatve, negate result
        B       exit_log                ;before going home

funny_atn

        CMP     r3, #0                  ;zero or denormalised?
        BEQ     exit_log                ;yes, so just return this

; The arguament has max. exponent value - infinity or NaN

        ORRS    r4, r1, r2              ;have we infinity?
        BNE     NaN_add                 ;must be a NaN...

        ADR     r0, piby2               ;yes, so return pi/2
        LDMIA   r0, {r1-r3}
        B       exit_log

;******************************************************************************
;
;       Data for ATN routines
;

twoMroot3

        DCD     &8930a2f4               ;2 - root(3)
        DCD     &f66ab18a               ;=0.26794 91924 31122 70647 E+0
        DCD     &00003ffd

        DCD     &ddb3d742               ;root(3)
        DCD     &c265539e               ;=1.73205 08075 68877 29353 E+0
        DCD     &00003fff

atn_coeff

        DCD     &f0624f0a               ;q3
        DCD     &56388310               ;=0.15024 00116 00285 76121 E+2
        DCD     &00004002

        DCD     &ee505190               ;q2
        DCD     &6d1eb4e8               ;=0.59578 43614 25973 44465 E+2
        DCD     &00004004

        DCD     &ac509020               ;q1
        DCD     &5b6d243b               ;=0.86157 34959 71302 42515 E+2
        DCD     &00004005

        DCD     &a443e5e6               ;q0
        DCD     &24ad4b90               ;=0.41066 30668 25757 81263 E+2
        DCD     &00004004


        DCD     &d66bd6cd               ;-p3
        DCD     &8c3de934               ;=0.83758 29936 81500 59274 E+0
        DCD     &00003ffe

        DCD     &87e9fae4               ;-p2
        DCD     &6b531a29               ;=0.84946 24035 13206 83534 E+1
        DCD     &00004002

        DCD     &a40bfdcf               ;-p1
        DCD     &15e65691               ;=0.20505 85519 58616 51981 E+2
        DCD     &00004003

        DCD     &db053288               ;-p0
        DCD     &30e70eb4               ;=0.13688 76889 41919 26929 E+2
        DCD     &00004002

atn_array

        DCD     &0                      ;A(0)=0.0
        DCD     &0
        DCD     &0
        DCD     &0

        DCD     &860a91c1               ;A(3) = PI/6
        DCD     &6b9b2c23
        DCD     &00003ffe
        DCD     &0

piby2
        DCD     &c90fdaa2               ;A(2) = PI/2
        DCD     &2168c235
        DCD     &00003fff
        DCD     &0

        DCD     &860a91c1               ;A(3) = PI/3
        DCD     &6b9b2c23
        DCD     &00003fff


;******************************************************************************

filelevel       SETA    filelevel - 1
        END
