; -> fpecpst/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2cpst        3/12/86 Martin Clemoes
;
;       Description:    This file contains all the cpst operation routines.
;                       It is GETted by the file FPE2mid. It therefore
;                       implements the CMF and CNF operations.
;
;******************************************************************************
;
;       CMFE operation
;

cmfe
cnfe                                    ;throw away information - no speed cost

; CMFE   fn,(-)fm  (fm may be a constant) - first check if fm is a constant

        MOVS    r8, r9, LSL #28         ;put fm bits in r8
        BMI     constant_comp           ;bit 3 set, so a constant

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r8, r12, r8, LSR #24    ;point at fm store
        LDMIA   r8, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        AND     r7, r9, #&00070000      ;preserve fn number for later

        MOV     r11, r3                 ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        EOR     r8, r0, r9, LSL #10     ;if cnfe, alter sign bit during copy
        MOV     r9, r1

got_Fm

; We have loaded up fm, so now load fn and convert to U if necessary

        ADD     r12, r12, r7, LSR #12   ;point at fn store
        LDMIA   r12, {r0-r3}            ;and load it up

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r12, {r0-r3}            ;write back in U type if converted

; Now check to see if either value is a NaN

        MOV     r4, #&00030000          ;convenient bits
        ADDS    r5, r4, r3, LSL #17     ;has fn got maximum exponent?
        ADDCCS  r5, r4, r11, LSL #17    ;if not, has fm got max. exponent?
        BCS     pr_nan_found            ;yes, so either nan or infinity
not_nan

; Now perform compare of r0-r3 with r8-r11

        EORS    r5, r0, r8              ;Are the signs different?
        BMI     diff_signs              ;yes...

; Same sign, so compare exponents

        CMP     r3, r11

; Compare mantissae if exponents the same

        CMPEQ   r1, r9
        CMPEQ   r2, r10

; Now adjust user's status flags from comparison results

        AND     r7, r4, r15, LSR #13    ;get Z and C from comparison
        ORRCC   r7, r7, #&00040000      ;and set N if carry clear

        TSTNE   r0, #&80000000          ;if not equal, and both signs -ve,
        EORNE   r7, r7, #&00050000      ;reverse C and N bits

;        savebaseinr11                   ;get base of saved arm regs.
        BIC     r14, r14, #&f0000000    ;and clear from user pc old status bits
        ORR     r14, r14, r7, LSL #13   ;and replace them with the new ones
        STR     r14, [arm, #15*4]       ;then save back
        exit0to15_wb                    ;and go home

diff_signs

; We have different signs, so unless we have +0 and -0, not equal

        ORRS    r4, r11, r3             ;have we got +0 and -0?
        ORREQS  r4, r10, r2
        ORREQS  r4, r9, r1
        BEQ     both_zero               ;yes...

;        savebaseinr11                   ;get base of saved arm regs.
        BIC     r14, r14, #&f0000000    ;and clear from user pc old status bits
        TST     r0, #&80000000          ;is fn negative?
        ORRNE   r14, r14, #&80000000    ;yes, so set up for less than
        ORREQ   r14, r14, #&20000000    ;no, so set up for greater than
        STR     r14, [arm, #15*4]       ;then save back
        exit0to15_wb                    ;and go home

both_zero

; Both numbers zero, so return equal condition flags

;        savebaseinr11                   ;get base of saved arm regs.
        BIC     r14, r14, #&f0000000    ;and clear from user pc old status bits
        ORR     r14, r14, #&60000000    ;and replace them with the new ones
        STR     r14, [arm, #15*4]       ;then save back
anexit
        exit0to15_wb                    ;and go home

pr_nan_found

; We have one number with max. exponent - NaN or infinity (the latter OK)

        ADDS    r5, r4, r3, LSL #17     ;is it fn with max. exponent?
        BCC     suspect_fm              ;no, so it must be fm...

        ORRS    r5, r1, r2              ;is fn a NaN?
        BNE     nan_found               ;yes...

; Fn is not a NaN, so check fm to see if that is

suspect_fm
        ADDS    r5, r4, r11, LSL #17    ;has fm got max. exponent?
        BCC     not_nan                 ;no, so go back to comparison

        ORRS    r5, r9, r10             ;is fm infinity?
        BEQ     not_nan                 ;yes, so go back to comparison

nan_found

; We have found a NaN - return NZCV = 0001 and cause exception if enabled

;        savebaseinr11                   ;get base of saved arm regs.
        BIC     r14, r14, #&f0000000    ;and clear from user pc old status bits
        ORR     r14, r14, #&10000000    ;and replace them with the new ones
        STR     r14, [arm, #15*4]       ;then save back

        ReloadFPRegPointer              ;point r12 at base of fp regs. again
        ADR     r7, anexit              ;cause exception if enabled, else exit
        MOV     r4, #NAN_E_MASK
        B       check_exep

constant_comp

; Fm is one of the fp constants, so load up r8-r11 with right U type value

        AND     r7, r9, #&00070000      ;save fn bits for later
        AND     r5, r9, #&00200000      ;save cnfe bit for later

        ADR     r6, U_type_constants - 8*16 ;point at lookup table
        ADD     r6, r6, r8, LSR #24     ;hence correct value
        LDMIA   r6, {r8-r11}            ;hence load value
        EOR     r8, r8, r5, LSL #10     ;and negate it if cnfe

        B       got_Fm                  ;and go back to comparison

U_type_constants

        DCD     0                                       ; 0.0
        DCD     0
        DCD     0
        DCD     0

        DCD     0                                       ; 1.0
        DCD     2_10000000000000000000000000000000
        DCD     0
        DCD     2_00000000000000000011111111111111

        DCD     0                                       ; 2.0
        DCD     2_10000000000000000000000000000000
        DCD     0
        DCD     2_00000000000000000100000000000000

        DCD     0                                       ; 3.0
        DCD     2_11000000000000000000000000000000
        DCD     0
        DCD     2_00000000000000000100000000000000

        DCD     0                                       ; 4.0
        DCD     2_10000000000000000000000000000000
        DCD     0
        DCD     2_00000000000000000100000000000001

        DCD     0                                       ; 5.0
        DCD     2_10100000000000000000000000000000
        DCD     0
        DCD     2_00000000000000000100000000000001

        DCD     0                                       ; 0.5
        DCD     2_10000000000000000000000000000000
        DCD     0
        DCD     2_00000000000000000011111111111110

        DCD     0                                       ; 10.0
        DCD     2_10100000000000000000000000000000
        DCD     0
        DCD     2_00000000000000000100000000000010

constant_comp2

; Fm is one of the fp constants, so load up r8-r11 with right U type value

        AND     r7, r9, #&00070000      ;save fn bits for later
        AND     r5, r9, #&00200000      ;save cnfe bit for later

        ADR     r6, U_type_constants - 8*16 ;point at lookup table
        ADD     r6, r6, r8, LSR #24     ;hence correct value
        LDMIA   r6, {r8-r11}            ;hence load value
        EOR     r8, r8, r5, LSL #10     ;and negate it if cnfe

        B       got_Fm2                 ;and go back to comparison


;******************************************************************************
;
;       CMF operation
;

cmf
cnf                                     ;throw away information - no speed cost

; CMF   fn,(-)fm  (fm may be a constant) - first check if fm is a constant

        MOVS    r8, r9, LSL #28         ;put fm bits in r8
        BMI     constant_comp2          ;bit 3 set, so a constant

; Not a constant, so load up from register fm and convert to U if necessary

        ADD     r8, r12, r8, LSR #24    ;point at fm store
        LDMIA   r8, {r0-r3}             ;and load into arm

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        AND     r7, r9, #&00070000      ;preserve fn number for later

        MOV     r11, r3                 ;and shuffle regs. - necessary for
        MOV     r10, r2                 ;ensuretype
        EOR     r8, r0, r9, LSL #10     ;if cnfe, alter sign bit during copy
        MOV     r9, r1

got_Fm2

; We have loaded up fm, so now load fn and convert to U if necessary

        ADD     r12, r12, r7, LSR #12   ;point at fn store
        LDMIA   r12, {r0-r3}            ;and load it up

        MOVS    r4, r3, LSR #26         ;is it type U already?
        ensuretype                      ;if not, convert

        STMNEIA r12, {r0-r3}            ;write back in U type if converted

; Now check to see if either value is a NaN

        MOV     r4, #&00030000          ;convenient bits
        ADDS    r5, r4, r3, LSL #17     ;has fn got maximum exponent?
        ADDCCS  r5, r4, r11, LSL #17    ;if not, has fm got max. exponent?
        BCS     pr_nan_found2           ;yes, so either nan or infinity
not_nan2

; Now perform compare of r0-r3 with r8-r11

        EORS    r5, r0, r8              ;Are the signs different?
        BMI     diff_signs              ;yes...

; Same sign, so compare exponents

        CMP     r3, r11

; Compare mantissae if exponents the same

        CMPEQ   r1, r9
        CMPEQ   r2, r10

; Now adjust user's status flags from comparison results

        AND     r7, r4, r15, LSR #13    ;get Z and C from comparison
        ORRCC   r7, r7, #&00040000      ;and set N if carry clear

        TSTNE   r0, #&80000000          ;if not equal, and both signs -ve,
        EORNE   r7, r7, #&00050000      ;reverse C and N bits

;        savebaseinr11                   ;get base of saved arm regs.
        BIC     r14, r14, #&f0000000    ;and clear from user pc old status bits
        ORR     r14, r14, r7, LSL #13   ;and replace them with the new ones
        STR     r14, [arm, #15*4]       ;then save back
        exit0to15_wb                    ;and go home

pr_nan_found2

; We have one number with max. exponent - NaN or infinity (the latter OK)

        ADDS    r5, r4, r3, LSL #17     ;is it fn with max. exponent?
        BCC     suspect_fm2             ;no, so it must be fm...

        ORRS    r5, r1, r2              ;is fn a NaN?
        BNE     nan_found2              ;yes...

; Fn is not a NaN, so check fm to see if that is

suspect_fm2
        ADDS    r5, r4, r11, LSL #17    ;has fm got max. exponent?
        BCC     not_nan2                ;no, so go back to comparison

        ORRS    r5, r9, r10             ;is fm infinity?
        BEQ     not_nan2                ;yes, so go back to comparison

nan_found2

; We have found a NaN - return NZCV = 0001 and cause exception if enabled

;        savebaseinr11                   ;get base of saved arm regs.
        BIC     r14, r14, #&f0000000    ;and clear from user pc old status bits
        ORR     r14, r14, #&10000000    ;and replace them with the new ones
        STR     r14, [arm, #15*4]       ;then save back

        exit0to15_wb

;******************************************************************************

filelevel       SETA    filelevel - 1
        END
