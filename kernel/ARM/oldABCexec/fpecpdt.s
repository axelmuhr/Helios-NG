; -> fpecpdt/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2cpdt        19/11/86 Martin Clemoes
;
;       Description:    This file contains all the cpdt operation routines.
;                       It is GETted by the file FPE2mid. It therefore
;                       implements the LDF and STF operations. STF requires
;                       conversion routines to convert a fp number from one
;                       form to another (eg. double precision to single).
;                       A set of conversion routines is also in this file,
;                       therefore.
;
;******************************************************************************
;
;       A couple of macros calling the convert routines if Z bit not set (NE)
;

        MACRO
        ensuretype
        ADR     r7, %FT20
        BNE     Convert
20
        MEND

        MACRO
        ensuretype_f                    ;saves 3 S-cycles if close enough
        ADR     r7, %FT30
        ADR     r5, conv_table
        ADDNE   r15, r5, r4, LSL #2     ;does switch from here
30
        MEND


        MACRO
        ensure_U_fast   $place          ;saves time if S to U conversion

        BEQ     $place                  ;go here if no conversion needed
        ADR     r7, %FT40               ;and here if a conversion not StoU done

        CMP     r4, #&8                 ;is a S to U conversion wanted?
        BNE     Convert                 ;no, so do another conversion

; We are now going to do a S to U conversion in-line (trivial) to save time

        MOV     r2, #0

; Test for NaN, infinity, zero or denormalised

        MOV     r3, r0, LSL #1          ;is exponent zero
        MOVS    r3, r3, LSR #24
        TEQNE   r3, #255                ;or 255?

; Normal S, so next transfer mantissa

        MOVNE   r1, r0, LSL #8          ;mantissa = 1.f
        ORRNE   r1, r1, #&80000000

; Now deal with exponent

        ADDNE   r3, r3, #&3f80          ;create exp. of new bias
        BEQ     funny_s_u               ;one of funnies, so branch to cope

40
; It is assumed that a writeback STM will follow this macro

        MEND


;******************************************************************************
;
;       Now for CPDT routines
;

|fp_pagef_bot|                          ;label to aid unix integration

cpdt00001

; LDF with post dec, but no writeback, was illegal for fp, now allowed

;       B       very_ill_instruction


cpdt00011

; LDF post decrement, 32/64 bit, writeback (hence not pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r5, r12, r11, LSR #8    ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.

        LDR     r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        SUB     r8, r10, r8, LSL #2     ;and post decrement with it
        STRNE   r8, [arm, r7, LSR #14]  ;then writeback if necessary

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00004000      ;then set bit to show packed
        MOV     r9, r9, LSL #15         ;then rotate bits into right place

        LDRT    r6, [r10], #4           ;load first word of variable
        LDRNET  r7, [r10], #4           ;and next word if necessary
        STMIA   r5, {r6-r9}             ;then store them in the fp reg.

        exit0to15_wb                    ;restore all regs. to allow for writeb.


cpdt00101

; LDF with post dec, but no writeback, was illegal for fp, now allowed

;       B       very_ill_instruction


cpdt00111

; LDF post decrement, 96 bit, writeback (hence not pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r5, r12, r11, LSR #8    ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.

        LDR     r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        SUB     r8, r10, r8, LSL #2     ;and post decrement with it
        STRNE   r8, [arm, r7, LSR #14]  ;then writeback if necessary

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00014000      ;then set bit to show packed
        MOV     r9, r9, LSL #15         ;then rotate bits into right place

        LDRT    r6, [r10], #4           ;load first word of variable
        LDRT    r7, [r10], #4           ;and next word
        LDRT    r8, [r10]               ;and next word
        STMIA   r5, {r6-r9}             ;then store them in the fp reg.

        exit0to15_wb                    ;restore all regs. to allow for writeb.


cpdt01001

; LDF with post inc, but no writeback, was illegal for fp, now allowed

;       B       very_ill_instruction


cpdt01011

; LDF post increment, 32/64 bit, writeback (hence not pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r5, r12, r11, LSR #8    ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.

        LDR     r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        ADD     r8, r10, r8, LSL #2     ;and post increment with it
        STRNE   r8, [arm, r7, LSR #14]  ;then writeback if necessary

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00004000      ;then set bit to show packed
        MOV     r9, r9, LSL #15         ;then rotate bits into right place

        LDRT    r6, [r10], #4           ;load first word of variable
        LDRNET  r7, [r10], #4           ;and next word if necessary
        STMIA   r5, {r6-r9}             ;then store them in the fp reg.

        exit0to15_wb                    ;restore all regs. to allow for writeb.


cpdt01101

; LDF with post inc, but no writeback, was illegal for fp, now allowed

;       B       very_ill_instruction


cpdt01111

; LDF post increment, 96 bit, writeback (hence not pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r5, r12, r11, LSR #8    ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.

        LDR     r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        ADD     r8, r10, r8, LSL #2     ;and post increment with it
        STRNE   r8, [arm, r7, LSR #14]  ;then writeback if necessary

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00014000      ;then set bit to show packed
        MOV     r9, r9, LSL #15         ;then rotate bits into right place

        LDRT    r6, [r10], #4           ;load first word of variable
        LDRT    r7, [r10], #4           ;and next word
        LDRT    r8, [r10]               ;and next word
        STMIA   r5, {r6-r9}             ;then store them in the fp reg.

        exit0to15_wb                    ;restore all regs. to allow for writeb.


cpdt10001

; LDF pre decrement, 32/64 bit, no writeback (hence pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r12, r12, r11, LSR #8   ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.
        TEQ     r7, #&000f0000          ;is this the pc? - only needed if pc OK

        LDRNE   r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        SUB     r10, r10, r8, LSL #2    ;and pre decrement with it

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00004000      ;then set bit to show packed
        MOV     r11, r9, LSL #15        ;then rotate bits into right place

        LDRT    r8, [r10], #4           ;load first word of variable
        LDRNET  r9, [r10], #4           ;and next word if necessary
        STMIA   r12, {r8-r11}           ;then store them in the fp reg.

        exit7to15


cpdt10011

; LDF pre decrement, 32/64 bit, writeback (hence not pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r5, r12, r11, LSR #8    ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.

        LDR     r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        SUB     r10, r10, r8, LSL #2    ;and pre decrement with it
        STRNE   r10, [arm, r7, LSR #14] ;then writeback if necessary

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00004000      ;then set bit to show packed
        MOV     r9, r9, LSL #15         ;then rotate bits into right place

        LDRT    r6, [r10], #4           ;load first word of variable
        LDRNET  r7, [r10], #4           ;and next word if necessary
        STMIA   r5, {r6-r9}             ;then store them in the fp reg.

        exit0to15_wb                    ;restore all regs. to allow for writeb.

                     
cpdt10101

; LDF pre decrement, 96 bit, no writeback (hence pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r12, r12, r11, LSR #8   ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.
        TEQ     r7, #&000f0000          ;is this the pc? - only needed if pc OK

        LDRNE   r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        SUB     r10, r10, r8, LSL #2    ;and pre decrement with it

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00014000      ;then set bit to show packed
        MOV     r11, r9, LSL #15        ;then rotate bits into right place

        LDRT    r8, [r10], #4           ;load first word of variable
        LDRT    r9, [r10], #4           ;and next word
        LDRT    r10, [r10]              ;and next word
        STMIA   r12, {r8-r11}           ;then store them in the fp reg.

        exit7to15


cpdt10111

; LDF pre decrement, 96 bit, writeback (hence not pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r5, r12, r11, LSR #8    ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.

        LDR     r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        SUB     r10, r10, r8, LSL #2    ;and pre decrement with it
        STRNE   r10, [arm, r7, LSR #14] ;then writeback if necessary

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00014000      ;then set bit to show packed
        MOV     r9, r9, LSL #15         ;then rotate bits into right place

        LDRT    r6, [r10], #4           ;load first word of variable
        LDRT    r7, [r10], #4           ;and next word
        LDRT    r8, [r10]               ;and next word
        STMIA   r5, {r6-r9}             ;then store them in the fp reg.

        exit0to15_wb                    ;restore all regs. to allow for writeb.


cpdt11001

; LDF pre increment, 32/64 bit, no writeback (hence pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r12, r12, r11, LSR #8   ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.
        TEQ     r7, #&000f0000          ;is this the pc? - only needed if pc OK

        LDRNE   r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        ADD     r10, r10, r8, LSL #2    ;and pre increment with it

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00004000      ;then set bit to show packed
        MOV     r11, r9, LSL #15        ;then rotate bits into right place

        LDRT    r8, [r10], #4           ;load first word of variable
        LDRNET  r9, [r10], #4           ;and next word if necessary
        STMIA   r12, {r8-r11}           ;then store them in the fp reg.

        exit7to15


cpdt11011

; LDF pre increment, 32/64 bit, writeback (hence not pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r5, r12, r11, LSR #8    ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.

        LDR     r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        ADD     r10, r10, r8, LSL #2    ;and pre increment with it
        STRNE   r10, [arm, r7, LSR #14] ;then writeback if necessary

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00004000      ;then set bit to show packed
        MOV     r9, r9, LSL #15         ;then rotate bits into right place

        LDRT    r6, [r10], #4           ;load first word of variable
        LDRNET  r7, [r10], #4           ;and next word if necessary
        STMIA   r5, {r6-r9}             ;then store them in the fp reg.

        exit0to15_wb                    ;restore all regs. to allow for writeb.

                     
cpdt11101

; LDF pre increment, 96 bit, no writeback (hence pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r12, r12, r11, LSR #8   ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.
        TEQ     r7, #&000f0000          ;is this the pc? - only needed if pc OK

        LDRNE   r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        ADD     r10, r10, r8, LSL #2    ;and pre increment with it

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00014000      ;then set bit to show packed
        MOV     r11, r9, LSL #15        ;then rotate bits into right place

        LDRT    r8, [r10], #4           ;load first word of variable
        LDRT    r9, [r10], #4           ;and next word
        LDRT    r10, [r10]              ;and next word
        STMIA   r12, {r8-r11}           ;then store them in the fp reg.

        exit7to15


cpdt11111

; LDF pre increment, 96 bit, writeback (hence not pc addressable)

        AND     r11, r9, #&00007000     ;get floating point reg. number
        ADD     r5, r12, r11, LSR #8    ;and hence point r12 at that register

;        savebaseinr11                   ;point r11 at base of saved user regs.
        AND     r7, r9, #&000f0000      ;base address arm reg.

        LDR     r10, [arm, r7, LSR #14] ;load r10 with base reg. contents
        ANDS    r8, r9, #&000000ff      ;get offset from instruction
        ADD     r10, r10, r8, LSL #2    ;and pre increment with it
        STRNE   r10, [arm, r7, LSR #14] ;then writeback if necessary

        ANDS    r9, r9, #&00008000      ;2nd. variable type bit
        ORR     r9, r9, #&00014000      ;then set bit to show packed
        MOV     r9, r9, LSL #15         ;then rotate bits into right place

        LDRT    r6, [r10], #4           ;load first word of variable
        LDRT    r7, [r10], #4           ;and next word
        LDRT    r8, [r10]               ;and next word
        STMIA   r5, {r6-r9}             ;then store them in the fp reg.

        exit0to15_wb                    ;restore all regs. to allow for writeb.


cpdt00000

; STF with post dec, but no writeback, was illegal for fp, now allowed

;       B       very_ill_instruction


cpdt00010

; STF post decrement, 32/64 bit, writeback (hence not pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #1              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number

        LDR     r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        SUB     r8, r10, r8, LSL #2     ;then post-decrement
        STRNE   r8, [arm, r7, LSR #14]  ;and write back if necessary

        STRT    r0, [r10], #4           ;store out 1st. word of register
        TST     r3, #&40000000          ;is this a 64 bit value?
        STRNET  r1, [r10]               ;yes, so save rest of it

        exit0to15_wb                    ;and go home


cpdt00100

; STF with post dec, but no writeback, was illegal for fp, now allowed

;       B       very_ill_instruction


cpdt00110

; STF post decrement, 96 bit, writeback (hence not pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #5              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number

        LDR     r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        SUB     r8, r10, r8, LSL #2     ;then post-decrement
        STRNE   r8, [arm, r7, LSR #14]  ;and write back if necessary

        STRT    r0, [r10], #4           ;store out 1st. word of register
        STRT    r1, [r10], #4           ;and next
        STRT    r2, [r10]               ;and next

        exit0to15_wb                    ;and go home


cpdt01000

; STF with post inc, but no writeback, was illegal for fp, now allowed

;       B       very_ill_instruction


cpdt01010

; STF post increment, 32/64 bit, writeback (hence not pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #1              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number

        LDR     r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        ADD     r8, r10, r8, LSL #2     ;then post-increment
        STRNE   r8, [arm, r7, LSR #14]  ;and write back if necessary

        STRT    r0, [r10], #4           ;store out 1st. word of register
        TST     r3, #&40000000          ;is this a 64 bit value?
        STRNET  r1, [r10]               ;yes, so save rest of it

        exit0to15_wb                       ;and go home


cpdt01100

; STF with post inc, but no writeback, was illegal for fp, now allowed

;       B       very_ill_instruction


cpdt01110

; STF post increment, 96 bit, writeback (hence not pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #5              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number

        LDR     r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        ADD     r8, r10, r8, LSL #2     ;then post-increment
        STRNE   r8, [arm, r7, LSR #14]  ;and write back if necessary

        STRT    r0, [r10], #4           ;store out 1st. word of register
        STRT    r1, [r10], #4           ;and next
        STRT    r2, [r10]               ;and next

        exit0to15_wb                    ;and go home


cpdt10000

; STF pre decrement, 32/64 bit, no writeback (hence pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #1              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number
        TEQ     r7, #&000f0000          ;is this pc - only needed if pc OK?

        LDRNE   r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        SUB     r10, r10, r8, LSL #2    ;then pre-decrement

        STRT    r0, [r10], #4           ;store out 1st. word of register
        TST     r3, #&40000000          ;is this a 64 bit value?
        STRNET  r1, [r10]               ;yes, so save rest of it

        exit0to15                       ;and go home


cpdt10010

; STF pre decrement, 32/64 bit, writeback (hence not pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #1              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number

        LDR     r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        SUB     r10, r10, r8, LSL #2    ;then pre-decrement
        STRNE   r10, [arm, r7, LSR #14] ;and write back if necessary

        STRT    r0, [r10], #4           ;store out 1st. word of register
        TST     r3, #&40000000          ;is this a 64 bit value?
        STRNET  r1, [r10]               ;yes, so save rest of it

        exit0to15_wb                    ;and go home


;******************************************************************************
;
;       Routine to convert fp value in r0 upwards into same value in
;       different form in r0 upwards. Used by STF, uses only IEEE
;       default rounding mode.
;

;       Inputs: r0-r3 contain copy of fp register containing input value
;               r4 specifies start form and end form:
;                  0000 0000 0000 0000 0000 0000 00ss seee
;                  sss/eee   =  0 -> unpacked
;                               1 -> single precision
;                               3 -> double precision
;                               5 -> extended precision
;                               7 -> packed decimal
;               r7 contains return address
;
;       Outputs: r0-r3 contain fp value in output form
;                r4, r5 smashed
;

Convert

        ADD     pc, pc, r4, LSL #2      ;switch

; Now for the table of routines to switch to for conversions

        BL      debug                   ;never used, as pc has extra 8
conv_table

        ret2                            ;U to U silly
        B       UtoS
        BL      debug                   ;U to non-existent type!
        B       UtoD
        BL      debug
        B       UtoE
        BL      debug
        B       UtoDEC

        B       StoU
        ret2
        BL      debug
        B       StoD
        BL      debug
        B       StoE
        BL      debug
        B       StoDEC

        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug

        B       DtoU
        B       DtoS
        BL      debug
        ret2
        BL      debug
        B       DtoE
        BL      debug
        B       DtoDEC

        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug

        B       EtoU
        B       EtoS
        BL      debug
        B       EtoD
        BL      debug
        ret2
        BL      debug
        B       EtoDEC

        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug
        BL      debug

        B       DECtoU
        B       DECtoS
        BL      debug
        B       DECtoD
        BL      debug
        B       DECtoE
        BL      debug
        ret2


;******************************************************************************
;
;       Some more cpdt ops. They shouldn't really be here, but are for speed...
;

cpdt10100

; STF pre decrement, 96 bit, no writeback (hence pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #5              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number
        TEQ     r7, #&000f0000          ;is this pc - only needed if pc OK?

        LDRNE   r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        SUB     r10, r10, r8, LSL #2    ;then pre-decrement

        STRT    r0, [r10], #4           ;store out 1st. word of register
        STRT    r1, [r10], #4           ;and next
        STRT    r2, [r10], #4           ;and next

        exit0to15                       ;and go home


cpdt10110

; STF pre decrement, 96 bit, writeback (hence not pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #5              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number

        LDR     r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        SUB     r10, r10, r8, LSL #2    ;then pre-decrement
        STRNE   r10, [arm, r7, LSR #14] ;and write back if necessary

        STRT    r0, [r10], #4           ;store out 1st. word of register
        STRT    r1, [r10], #4           ;and next
        STRT    r2, [r10]               ;and next

        exit0to15_wb                    ;and go home


cpdt11000

; STF pre increment, 32/64 bit, no writeback (hence pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #1              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number
        TEQ     r7, #&000f0000          ;is this pc - only needed if pc OK?

        LDRNE   r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        ADD     r10, r10, r8, LSL #2    ;then pre-increment

        STRT    r0, [r10], #4           ;store out 1st. word of register
        TST     r3, #&40000000          ;is this a 64 bit value?
        STRNET  r1, [r10]               ;yes, so save rest of it

        exit0to15                       ;and go home


cpdt11010

; STF pre increment, 32/64 bit, writeback (hence not pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #1              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number

        LDR     r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        ADD     r10, r10, r8, LSL #2    ;then pre-increment
        STRNE   r10, [arm, r7, LSR #14] ;and write back if necessary

        STRT    r0, [r10], #4           ;store out 1st. word of register
        TST     r3, #&40000000          ;is this a 64 bit value?
        STRNET  r1, [r10]               ;yes, so save rest of it

        exit0to15_wb                    ;and go home


cpdt11100

; STF pre increment, 96 bit, no writeback (hence pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #5              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number
        TEQ     r7, #&000f0000          ;is this pc - only needed if pc OK?

        LDRNE   r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        ADD     r10, r10, r8, LSL #2    ;then pre-increment

        STRT    r0, [r10], #4           ;store out 1st. word of register
        STRT    r1, [r10], #4           ;and next
        STRT    r2, [r10], #4           ;and next

        exit0to15                       ;and go home


cpdt11110

; STF pre increment, 96 bit, writeback (hence not pc addressable)

        AND     r5, r9, #&00007000      ;fp register number
        ADD     r5, r12, r5, LSR #8     ;hence point r5 at it
        LDMIA   r5, {r0-r3}             ;and get it in r0-r3

        MOV     r4, #&00000002
        AND     r4, r4, r9, LSR #14     ;get destination type bit (no. 15)
        ORR     r4, r4, #5              ;and add type bits implied by cpdt
        TEQ     r4, r3, LSR #29         ;is it right type already?

        ORR     r4, r4, r3, LSR #26     ;if not, indicate conversion wanted
        ensuretype_f                    ;and convert it

;        savebaseinr11                   ;make r11 point at saved user regs.
        AND     r7, r9, #&000f0000      ;arm register number

        LDR     r10, [arm, r7, LSR #14] ;get user's arm reg.
        ANDS    r8, r9, #&000000ff      ;and offset
        ADD     r10, r10, r8, LSL #2    ;then pre-increment
        STRNE   r10, [arm, r7, LSR #14] ;and write back if necessary

        STRT    r0, [r10], #4           ;store out 1st. word of register
        STRT    r1, [r10], #4           ;and next
        STRT    r2, [r10]               ;and next

        exit0to15_wb                    ;and go home

|fp_pagef_top|                          ;label to aid unix integration

;******************************************************************************
;
;       The routines that actually do the type conversions
;

EtoS
        BIC     r3, r0, #&80000000      ;convert E to U - 1 instruction!

UtoS
        AND     r0, r0, #&80000000      ;clear all but sign bit in r0

; Now check for NAN, infinity, zero or denormalised

        MOVS    r5, r3, LSL #17         ;is the exponent zero?
        ADDNES  r5, r5, #&00020000      ;or &3fff?
        BEQ     funny_u_s               ;yes, so one of funnies

; Normal U type, so first truncate mantissa (default IEEE rounding)

        TEQ     r2, #0                  ;are all bits except ms bit lost zero?
        ORREQS  r2, r2, r1, LSL #25

        AND     r2, r1, #&00000080      ;get ms bit lost
        ANDEQ   r2, r2, r1, LSR #1      ;and if yes, make sure round to even

        ADDS    r1, r1, r2              ;do rounding
        MOVCC   r1, r1, LSL #1          ;and if no overflow, lose 1. bit from U
        ADC     r3, r3, #0              ;but if has overflowed, increment exp.

; Now deal with exponent

        SUBS    r3, r3, #&3f80          ;try to change to 8 bit exponent
        BLS     underflow_u_s
        CMP     r3, #&ff

; And put the output together

        ORRCC   r0, r0, r3, LSL #23     ;put OK exponent in variable
        ORRCC   r0, r0, r1, LSR #9      ;and mantissa
        MOVCC   r3, #&20000000          ;then label as single precision

        MOVCC   r15, r7                 ;and go home
        B       overflow_u_s            ;exponent has overflowed

funny_u_s

; Deal with NaN, infinity, zero or denormalised as best we can

        MOVS    r4, r3, LSL #24         ;get 8 bits of exponent
        ORR     r0, r0, r4, LSR #1      ;and put them in exponent

        MOV     r3, #&20000000          ;then label as single precision

        BICNE   r1, r1, #&80000000      ;clear undefined bit
        ORRNE   r0, r0, r1, LSR #8      ;if exponent = 255, return
        MOVNE   r15, r7                 ;with as much mantissa as poss.
        ORRS    r2, r2, r1              ;else was U denormalised?
        MOVEQ   r15, r7                 ;no, so go home with zero
        B       und_u_s                 ;yes, so underflow definitely

overflow_u_s

; Exponent too big for single precision

        ORR     r0, r0, #&7f000000      ;put infinity in value
        ORR     r0, r0, #&00800000
        MOV     r3, #&20000000          ;then label as single precision

        MOV     r4, #OFL_E_MASK         ;and register an overflow

check_exep

; Check for exception due to event mask in r4

        LDR     r5, [r12, #16*8]        ;get status register
        ORR     r5, r5, r4              ;and set event bit from mask
        STR     r5, [r12, #16*8]        ;then put back new value

        TST     r4, r5, LSR #16         ;is an exception generated?

        MOVNE   r0, r4                  ;put mask in r0
        BNE     fp_exception            ;yes, so generate os exception

exep_disabled

; The exception tested for was disabled - if OFL, set INX bit etc

        CMP     r4, #OFL_E_MASK         ;is OFL disabled and just occurred?
        MOVNE   r15, r7                 ;if not, go home

        MOV     r4, #INX_E_MASK         ;if so, register an INX event
        B       check_exep

underflow_u_s

; Exponent too small for single precision, so first try gradual underflow

        CMN     r3, #23                 ;will gradual underflow cope?
        BLE     und_u_s                 ;no, so register an underflow

        MOV     r1, r1, LSR #10         ;shift mant. to fit r0, +1 to
        ORR     r1, r1, #&00400000      ;fit in implied 1. bit
        RSB     r3, r3, #0              ;negate r3
        ORR     r0, r0, r1, LSR r3      ;and put remains of mantissa in

        MOV     r3, #&20000000          ;label as single precision
        ret2                            ;and go home

und_u_s

; Underflow has definitely occurred

        MOV     r3, #&20000000          ;zero in r0, so label as sing. prec.
        MOV     r4, #UFL_E_MASK         ;and register an underflow
        B       check_exep


EtoD
        BIC     r3, r0, #&80000000      ;convert E to U - 1 instruction!

UtoD
        AND     r0, r0, #&80000000      ;clear all but sign bit in r0

; Now check for NAN, infinity, zero or denormalised

        MOVS    r5, r3, LSL #17         ;is the exponent zero?
        ADDNES  r5, r5, #&00020000       ;or &3fff?
        BEQ     funny_u_d               ;yes, so one of funnies

; Normal U type, so first truncate mantissa (default IEEE rounding)

        MOVS    r4, r2, LSL #22         ;are all bits except ms bit lost zero?

        AND     r4, r2, #&00000400      ;get ms bit lost
        ANDEQ   r4, r4, r2, LSR #1      ;and if yes, make sure round to even

        ADDS    r2, r2, r4              ;do rounding
        ADCS    r1, r1, #0

        ADC     r3, r3, #0              ;if has overflowed, increment exp.
        ADDS    r2, r2, r2              ;else lose 1. bit
        ADC     r1, r1, r1

; Now deal with exponent

        SUBS    r3, r3, #&3c00          ;try to change to 11 bit exponent
        BLS     underflow_u_d
        ADD     r4, r3, #1
        CMP     r4, #&800

; And put the output together

        ORRCC   r0, r0, r3, LSL #20     ;put OK exponent in variable
        ORRCC   r0, r0, r1, LSR #12     ;and mantissa
        MOVCC   r1, r1, LSL #20
        ORRCC   r1, r1, r2, LSR #12
        MOVCC   r3, #&60000000          ;then label as double precision

        MOVCC   r15, r7                 ;and go home
        B       overflow_u_d            ;exponent has overflowed

funny_u_d

; Deal with NaN, infinity, zero or denormalised as best we can

        MOVS    r4, r3, LSL #21         ;get 11 bits of exponent
        ORR     r0, r0, r4, LSR #1      ;and put them in exponent

        MOV     r3, #&60000000          ;then label as double precision

        BICNE   r1, r1, #&80000000      ;clear undefined bit
        ORRNE   r0, r0, r1, LSR #11     ;if exponent = 2047, return
        MOVNE   r1, r1, LSL #21
        ORRNE   r1, r1, r2, LSR #11
        MOVNE   r15, r7                 ;with as much mantissa as poss.
        ORRS    r2, r2, r1              ;else was U denormalised?
        MOVEQ   r15, r7                 ;no, so go home with zero
        B       und_u_d                 ;yes, so underflow definitely

overflow_u_d

; Exponent too big for double precision

        ORR     r0, r0, #&7f000000      ;put infinity in value
        ORR     r0, r0, #&00f00000
        MOV     r1, #0
        MOV     r3, #&60000000          ;then label as double precision

        MOV     r4, #OFL_E_MASK         ;and register an overflow
        B       check_exep

underflow_u_d

; Exponent too small for double precision, so first try gradual underflow

        CMN     r3, #52                 ;will gradual underflow cope?
        BLE     und_u_d                 ;no, so register an underflow

        MOV     r2, r2, LSR #13         ;shift mantissa to fit r0/1, +1 to
        ORR     r2, r2, r1, LSL #19
        MOV     r1, r1, LSR #13
        ORR     r1, r1, #&00080000      ;fit in implied 1. bit

        RSB     r3, r3, #0              ;negate r3
        SUBS    r4, r3, #32             ;is shift >31 ?

        MOVCS   r1, r1, LSR r4          ;yes, so leave r0, set up r1

        RSBCC   r4, r3, #32
        ORRCC   r0, r0, r1, LSR r3      ;else: fit appropriate bits in r0
        MOVCC   r1, r1, LSL r4          ;then set up r1 with bits from r1
        ORRCC   r1, r1, r2, LSR r3      ;and bits from r2

        MOV     r3, #&60000000          ;label as double precision
        ret2                            ;and go home

und_u_d

; Underflow has definitely occurred

        MOV     r1, #0
        MOV     r3, #&60000000          ;zero in r0, so label as double prec.
        MOV     r4, #UFL_E_MASK         ;and register an underflow
        B       check_exep


UtoE
        AND     r0, r0, #&80000000      ;clear all but sign bit in r0

        ORR     r0, r0, r3              ;and put exponent in r0
        MOV     r3, #&a0000000          ;and mark as extended format

        ret2                            ;then go home


StoU
        MOV     r2, #0

; Test for NaN, infinity, zero or denormalised

        MOV     r3, r0, LSL #1          ;is exponent zero
        MOVS    r3, r3, LSR #24
        TEQNE   r3, #255                ;or 255?

; Normal S, so next transfer mantissa

        MOVNE   r1, r0, LSL #8          ;mantissa = 1.f
        ORRNE   r1, r1, #&80000000

; Now deal with exponent

        ADDNE   r3, r3, #&3f80          ;create exp. of new bias

        MOVNE   r15, r7                 ;then go home

funny_s_u
        ORRS    r3, r3, r3, LSL #7      ;extend funny exponent to 15 bits

        MOVNE   r1, r0, LSL #8          ;move mantissa
        BICNE   r1, r1, #&80000000
        MOVNE   r15, r7                 ;and return if NaN or infinity

        MOVS    r1, r0, LSL #9          ;zero?
        MOVEQ   r15, r7                 ;yes, so return a zero

; To have got here, we have a denormalised S, so make normalised U

        MOV     r3, #&3f80              ;set exponent correctly for now

f_s_u_loop
        SUBPL   r3, r3, #1              ;if not normalised, decrement exp.
        MOVPLS  r1, r1, LSL #1          ;and shift mantissa left 1
        BPL     f_s_u_loop              ;and repeat as necessary

        ret2                            ;then go home


StoD

; First pretend normal S, so convert to D

        MOV     r1, r0, LSL #29         ;put 3 ls bits of mantissa in r1
        BIC     r3, r0, #&80000000      ;move everything but sign out of r0
        AND     r0, r0, #&80000000
        ORR     r0, r0, r3, LSR #3      ;then put it back
        MOV     r3, #&60000000          ;and label as D

; Now test for NaN, infinity, zero or denormalised

        ANDS    r2, r0, #&0ff00000      ;is exponent 0
        TEQNE   r2, #&0ff00000          ;or 255?

        ADDNE   r0, r0, #&38000000      ;finish off conversion if no
        MOVNE   r15, r7                 ;and go home

; If we got to here, we have one of the funnies

        ORR     r0, r0, r2, LSL #3      ;convert max or min exponent

        TEQ     r2, #0                  ;NaN or infinity?
        MOVNE   r15, r7                 ;yes, so job done

        ORRS    r2, r1, r0, LSL #12     ;zero?
        MOVEQ   r15, r7                 ;yes, so job done

; We have a denormalised no., oh dear

        MOV     r1, r1, LSR #20         ;put whole mantissa in r1
        ORRS    r1, r1, r0, LSL #12

        AND     r0, r0, #&80000000      ;and clear rubbish from r0
        ORR     r0, r0, #&38000000      ;setting exponent correct for now

f_s_d_loop
        SUBPL   r0, r0, #&00100000      ;decrement exp
        MOVPLS  r1, r1, LSL #1          ;and double mant. until ms bit set
        BPL     f_s_d_loop

        MOV     r1, r1, LSL #1          ;now lose set bit to leave implied 1.

        ORR     r0, r0, r1, LSR #12     ;then put back new mantissa
        MOV     r1, r1, LSL #20

        ret2                            ;and go home


StoE
        MOV     r2, #0

; Test for NaN, infinity, zero or denormalised

        MOV     r3, r0, LSL #1          ;is exponent zero
        MOVS    r3, r3, LSR #24
        TEQNE   r3, #255                ;or 255?

; Normal S, so next transfer mantissa

        MOV     r1, r0, LSL #8          ;mantissa = 1.f
        ORRNE   r1, r1, #&80000000

; Now deal with exponent

        ADDNE   r3, r3, #&3f80          ;create exp. of new bias

; Then put back into E type

        AND     r0, r0, #&80000000      ;clear all but sign bit in r0

        ORR     r0, r0, r3              ;and put exponent in r0
        MOV     r3, #&a0000000          ;and mark as extended format

        MOVNE   r15, r7                 ;then go home

funny_s_e
        ORR     r0, r0, r0, LSL #7      ;extend funny exponent to 15 bits
        TST     r0, #&ff                ;is this exponent zero?

        BICNE   r1, r1, #&80000000      ;restore old mantissa
        MOVNE   r15, r7                 ;and return if NaN or infinity

        MOVS    r1, r1, LSL #1          ;zero?
        MOVEQ   r15, r7                 ;yes, so return a zero

; To have got here, we have a denormalised S, so make normalised E

        ORR     r0, r0, #&3f80          ;set exponent correctly for now

f_s_e_loop
        SUBPL   r0, r0, #1              ;if not normalised, decrement exp.
        MOVPLS  r1, r1, LSL #1          ;and shift mantissa left 1
        BPL     f_s_e_loop              ;and repeat as necessary

        ret2                            ;then go home


DtoU

; Test for NaN, infinity, zero or denormalised

        MOV     r3, r0, LSL #1          ;is exponent zero
        MOVS    r3, r3, LSR #21
        ADDNE   r4, r3, #1
        TEQNE   r4, #&800               ;or max. value?

; Normal D, so next transfer mantissa

        MOV     r2, r1, LSL #11
        MOV     r1, r1, LSR #21
        ORR     r1, r1, r0, LSL #11     ;mantissa = 1.f
        ORRNE   r1, r1, #&80000000

; Now deal with exponent

        ADDNE   r3, r3, #&3c00          ;create exp. of new bias

        MOVNE   r15, r7                 ;then go home

funny_d_u
        ORRS    r3, r3, r3, LSL #4      ;extend funny exponent to 15 bits

        MOVNE   r15, r7                 ;and return if NaN or infinity

        ADDS    r2, r2, r2              ;move mantissa
        ADC     r1, r1, r1

        ORRS    r4, r2, r1              ;zero?
        MOVEQ   r15, r7                 ;yes, so return a zero

; To have got here, we have a denormalised D, so make normalised U

        MOV     r3, #&3c00              ;set exponent correctly for now
        MOVS    r1, r1                  ;see if already normalised

f_d_u_loop
        SUBPL   r3, r3, #1              ;if not normalised, decrement exp.
        MOV     r4, #0
        ORRPL   r4, r4, r2, LSR #31
        ORRPL   r4, r4, r1, LSL #1
        MOVPL   r2, r2, LSL #1
        MOVPLS  r1, r4                  ;and shift mantissa left 1
        BPL     f_d_u_loop              ;and repeat as necessary

        ret2                            ;then go home

DtoS

; Copy exponent to r3 and check for NaN, infinity, zero or denormalised

        MOV     r3, r0, LSL #1          ;lose sign bit
        MOVS    r3, r3, LSR #21         ;and mantissa - is exponent zero
        ADDNE   r4, r3, #1
        TEQNE   r4, #&800               ;or max value?
        BEQ     funny_d_s               ;yes, so one of funnies...

; Normal D type, so put truncated mantissa in r1

        MOVS    r4, r1, LSL #4          ;all bits except ms bit lost zero?

        MOV     r1, r1, LSR #20         ;arrange r1
        ORR     r1, r1, r0, LSL #12

        AND     r4, r1, #&00000100      ;get ms bit lost
        ANDEQ   r4, r4, r1, LSR #1      ;and if yes, round to even (IEEE)
        ADDS    r1, r1, r4              ;do rounding

        ADC     r3, r3, #0              ;if overflow, increment exponent

; Tidy up r0

        AND     r0, r0, #&80000000      ;leave just sign bit in r0

; Now deal with exponent

        SUBS    r3, r3, #&380           ;try to change to 8 bit exponent
        BLS     underflow_u_s
        CMP     r3, #&ff

; And put the output together

        ORRCC   r0, r0, r3, LSL #23     ;put OK exponent in variable
        ORRCC   r0, r0, r1, LSR #9      ;and mantissa
        MOVCC   r3, #&20000000          ;then label as single precision

        MOVCC   r15, r7                 ;and go home
        B       overflow_u_s            ;exponent has overflowed

funny_d_s

; Deal with NaN, infinity, zero or denormalised as best we can

        ORR     r2, r1, r0, LSL #12     ;leave to show if zero mantissa
        MOV     r1, r1, LSR #20         ;then create mantissa in r1
        ORR     r1, r1, r0, LSL #12
        AND     r0, r0, #&80000000      ;and clear rubbish from r0

        MOVS    r4, r3, LSL #24         ;get 8 bits of exponent
        ORR     r0, r0, r4, LSR #1      ;and put them in exponent

        MOV     r3, #&20000000          ;then label as single precision

        ORRNE   r0, r0, r1, LSR #9      ;if exponent = 255, return
        MOVNE   r15, r7                 ;with as much mantissa as poss.
        TEQ     r2, #0                  ;else was U denormalised?
        MOVEQ   r15, r7                 ;no, so go home with zero
        B       und_u_s                 ;yes, so underflow definitely


DtoE

; Test for NaN, infinity, zero or denormalised

        MOV     r3, r0, LSL #1          ;is exponent zero
        MOVS    r3, r3, LSR #21
        ADDNE   r4, r3, #1
        TEQNE   r4, #&800               ;or max. value?

; Normal D, so next transfer mantissa

        MOV     r2, r1, LSL #11
        MOV     r1, r1, LSR #21
        ORR     r1, r1, r0, LSL #11     ;mantissa = 1.f
        ORRNE   r1, r1, #&80000000

; Now deal with exponent

        ADDNE   r3, r3, #&3c00          ;create exp. of new bias
        AND     r0, r0, #&80000000      ;clear all but sign bit
        ORR     r0, r0, r3              ;and put in exponent

        MOV     r3, #&a0000000          ;and mark as E type
        MOVNE   r15, r7                 ;then go home

funny_d_e
        ORR     r0, r0, r0, LSL #4      ;extend funny exponent to 15 bits
        TST     r0, #255

        MOVNE   r15, r7                 ;and return if NaN or infinity

        ADDS    r2, r2, r2              ;move mantissa
        ADC     r1, r1, r1

        ORRS    r4, r2, r1              ;zero?
        MOVEQ   r15, r7                 ;yes, so return a zero

; To have got here, we have a denormalised D, so make normalised U

        ORR     r0, r0, #&3c00          ;set exponent correctly for now
        MOVS    r1, r1                  ;is it already normalised?

f_d_e_loop
        SUBPL   r0, r0, #1              ;if not normalised, decrement exp.
        MOV     r4, #0
        ORRPL   r4, r4, r2, LSR #31
        ORRPL   r4, r4, r1, LSL #1
        MOVPL   r2, r2, LSL #1
        MOVPLS  r1, r4                  ;and shift mantissa left 1
        BPL     f_d_e_loop              ;and repeat as necessary

        ret2                            ;then go home

EtoU
        BIC     r3, r0, #&80000000      ;1 instruction!
        ret2

base10factor
        DCD     &9a209a84               ;LOG10(2)
        DCD     &fbcff799
        DCD     &00003ffd

StoDEC
DtoDEC
EtoDEC

; First convert to U type, then use UtoDEC

        STMDB   r12!, {r7}              ;save return address
        ADR     r7, joinpoint           ;then set return address to here
        BIC     r4, r4, #7              ;make conversion to U type
        B       Convert                 ;then convert to U type
joinpoint
        LDMIA   r12!, {r7}              ;retrieve return address

UtoDEC

; Check for zero, denorm, infinity or NaN

        MOVS    r5, r3, LSL #17         ;is exponent zero?
        ADDNES  r5, r5, #&00020000      ;or max. value?
        BEQ     funnyUtoDEC             ;yes...

nzero_UtoDEC

        MOV     r5, #&4000              ;is magnitude < 1.0?
        SUB     r5, r5, #1
        CMP     a_exp, r5
        BCC     not_zero_exp            ;yes, so exp10 <> 0...
        ADD     r5, r5, #3
        CMP     a_exp, r5               ;is 1.0<=value<10.0
        CMPEQ   a1, #&a0000000

        MOVCC   w1, #0                  ;save 2 zero words
        MOVCC   w2, #0
        STMCCDB r12!, {w1,w2,r6-r11,r14} ;with work regs and lr
        BCC     zero_exp_join           ;then skip a lot of work

not_zero_exp

; Next save value and find exp10 = int. part (log10 (value2))

        STMDB   r12!, {a_sign,a1,a2,a_exp,r6-r11,r14} ;save a, work regs, lr

        MOV     a_sign, #&8000          ;get 2e+1 = unbiased exponent *2 + 1
        SUB     w1, a_sign, #3
        RSB     w1, w1, a_exp, LSL #1
        BisfloatW1                      ;float it
        SUB     b_exp, b_exp, #1        ;hence get e+0.5, which is approx.
                                        ;LOG2(a)
        ADR     w1, base10factor        ;hence get approx LOG10(a), by knowing
        LDMIA   w1, {a1,a2,a_exp}       ;LOG10(a) = LOG2(a) * LOG10(2)
        AisAtimesB

        MOV     w1, #&4000              ;number of integer binary digits
        SUB     w1, w1, #2
        SUB     w1, a_exp, w1           ;is exponent - &3ffe
        RSB     w1, w1, #32
        MOV     w5, a1, LSR w1          ;hence we have integer part of a in w5

        CMP     a1, w5, LSL w1          ;if a not precisely an integer
        CMPEQ   a2, #0
        ANDNES  b_exp, a_sign, #&80000000 ;and is negative,
        ADDNE   w5, w5, #1              ;round to -infinity by incrementing w5
        AND     b_exp, a_sign, #&80000000 ;exponent sign for later

; We have exponent in binary, also its sign. Now get mantissa in binary.

        LDMIA   r12!, {a_sign,a1,a2,a_exp} ;retrieve value
        STMDB   r12!, {w5,b_exp}        ;and save exponent magnitude, sign

        BL      bis10tow5               ;10^exponent in b

        LDR     w1, [r12, #4]           ;retrieve exponent sign
        MOVS    w1, w1                  ;positive or negative?
        AisAoverB       PL              ;positive, so divide
        AisAtimesB      MI              ;negative, so multiply

        MOV     r5, #&4000              ;is magnitude < 1.0?
        SUB     r5, r5, #1
        CMP     a_exp, r5
        BCC     fudge_exp_down          ;yes, so exp10 is 1 too big...
        ADD     r5, r5, #3
        CMP     a_exp, r5               ;is 1.0<=value<10.0?
        CMPEQ   a1, #&a0000000
        BCS     fudge_exp_up            ;no, so exp10 is 1 too small...

zero_exp_join

; Now we arrange the mantissa in a1,a2,a_exp so that the bicimal point is
; after the most significant nibble in a1 - so that this nibble is a valid
; decimal mantissa digit

        MOV     w1, #&4000              ;work out shift needed
        ADD     w1, w1, #2
        SUBS    w1, w1, a_exp           ;remembering if zero (ie >= 8)

        RSB     w2, w1, #32             ;now set up shifted integer
        MOV     a_exp, a2, LSL w2
        MOVNE   a2, a2, LSR w1          ;saving time if no shift
        ORRNE   a2, a2, a1, LSL w2
        MOVNE   a1, a1, LSR w1

; Now we have integer arranged, so extract ms 3 digits into w1 from 3 word
; integer in a1,a2,a_exp

        MOV     w1, a1, LSR #28         ;get ms digit
        MOV     w4, #3                  ;extract 3 digits before only a1,a2

three_word_loop

        BIC     a1, a1, #&f0000000      ;blank out extracted digit

        ADDS    a_exp, a_exp, a_exp     ;multiply int by 10, first by doubling
        ADCS    a2, a2, a2
        ADC     a1, a1, a1

        MOV     b1, a1, LSL #2          ;then by multiplying by 5
        ORR     b1, b1, a2, LSR #30
        MOV     b2, a2, LSL #2
        ORR     b2, b2, a_exp, LSR #30
        ADDS    a2, a2, b2
        ADC     a1, a1, b1

        SUBS    w4, w4, #1              ;are there any more to be extracted?

        MOVNE   w1, w1, LSL #4          ;if so, make room for next one,
        ORRNE   w1, w1, a1, LSR #28     ;and get the digit
        BNE     three_word_loop         ;and carry on

; Now we only need to worry about the 2 word integer in a1,a2 and put the
; answer in w2,w3

        ORRS    w2, a1, a2              ;are all the other digits zero?
        MOVEQ   w3, #0                  ;yes, so set the other digits to zero
        BEQ     no_roundup              ;and save time!

        MOV     w4, #15                 ;extract 15 digits

two_word_loop

        MOV     w2, w2, LSL #4          ;make room in w2,w3
        ORR     w2, w2, w3, LSR #28
        MOV     w3, w3, LSL #4

        ORR     w3, w3, a1, LSR #28     ;extract a digit
        BIC     a1, a1, #&f0000000      ;and clear it from a1,a2

        ADDS    a2, a2, a2              ;multiply by 10, first by doubling
        ADC     a1, a1, a1

        MOV     b1, a1, LSL #2          ;then multiplying by 5
        ORR     b1, b1, a2, LSR #30
        ADDS    a2, a2, a2, LSL #2
        ADC     a1, a1, b1

        SUBS    w4, w4, #1              ;then repeat as required
        BNE     two_word_loop

; We now just need the last digit, which should be rounded (I cheat here a
; teeny bit for speed!!)

        MOV     w2, w2, LSL #4          ;make room in w2,w3
        ORR     w2, w2, w3, LSR #28
        MOV     w3, w3, LSL #4

        ORR     w3, w3, a1, LSR #28     ;put in last digit

        MOVS    a1, a1, LSL #5          ;examine lost bits
        BCC     no_roundup              ;if ms bit lost clear, carry on...
        ADD     w3, w3, #1              ;if ms bit lost set, increment
        ORRS    a1, a1, a2              ;unless all others are clear,
        BICEQ   w3, w3, #1              ;in which case round to even

        AND     a1, w3, #&f             ;but if last digit 10,
        CMP     a1, #10
        SUBEQ   w3, w3, #1              ;bodge down again!!

no_roundup

; We now have the mantissa in w1-w3, so now extract exponent into decimal

        LDMIA   r12!, {a_exp}           ;exponent in binary in a_exp
        BL      digit_stretch           ;bcd exponent now in a1

; We now have all the DEC components, so put them together

        AND     a_sign, a_sign, #&80000000 ;just sign in a_sign

        ORR     a_sign, a_sign, a1, LSR #4 ;put exp magnitude in

        LDMIA   r12!, {w4}              ;retrieve exponent sign
        ORR     a_sign, a_sign, w4, LSR #1 ;install exp. sign

        ORR     a_sign, a_sign, w1      ;now install mantissa
        MOV     a1, w2
        MOV     a2, w3

        MOV     a_exp, #&e0000000       ;label as DEC
        LDMIA   r12!, {r6-r11,r14}      ;retrieve important regs,
        ret2                            ;and go home

fudge_exp_up

; The mantissa is slightly >= 10 (due to LOG inaccuracy), so increment exp10
; and divide mantissa by 10.

        LDMIA   r12!, {w1,w2}           ;retrieve exp10 magnitude, sign

        TST     w2, #&80000000          ;turn into signed integer
        RSBNE   w1, w1, #0

        ADD     w1, w1, #1              ;increment

        ANDS    w2, w1, #&80000000      ;turn back into magnitude, sign
        RSBNE   w1, w1, #0

        STMDB   r12!, {w1,w2}           ;and put back on stack

        MOV     b_sign, #&4000          ;put 10.0 in b
        MOV     b1, #&a0000000
        MOV     b2, #0
        ADD     b_exp, b_sign, #2
        AisAoverB                       ;hence divide a by 10

        B       zero_exp_join           ;and go back to action...

fudge_exp_down

; The mantissa is slightly < 1.0 (due to LOG inaccuracy), so decrement exp10
; and multiply mantissa by 10.

        LDMIA   r12!, {w1,w2}           ;retrieve exp10 magnitude, sign

        TST     w2, #&80000000          ;turn into signed integer
        RSBNE   w1, w1, #0

        SUB     w1, w1, #1              ;decrement

        ANDS    w2, w1, #&80000000      ;turn back into magnitude, sign
        RSBNE   w1, w1, #0

        STMDB   r12!, {w1,w2}           ;and put back on stack

        MOV     b_sign, #&4000          ;put 10.0 in b
        MOV     b1, #&a0000000
        MOV     b2, #0
        ADD     b_exp, b_sign, #2
        AisAtimesB                       ;hence multiply a by 10

        B       zero_exp_join           ;and go back to action...

digit_stretch

; Extract w4 bcd digits into ms nibbles of a1 from binary integer a_exp.
; To extract each digit, divide integer in a_exp by 10 and insert remainder.

        MOV     a2, a_exp               ;running integer into a2
        MOV     a_exp, #&10             ;calculate correct amount of answer

dv10_loop
        ADDS    a2, a2, a2              ;double remainder

        CMPCC   a2, #&a0000000          ;can we subtract 10 shifted from rem?

        SUBCS   a2, a2, #&a0000000      ;if we can, do this

        ADCS    a_exp, a_exp, a_exp     ;put bit in answer

        BCC     dv10_loop               ;and carry on...

; We now have a bcd digit in ms nibble of a2, so insert in a1 and continue

        MOV     a1, a2, LSR #12         ;put digit in a1

thous
        CMP     a_exp, #100             ;any more thousands?
        SUBGE   a_exp, a_exp, #100      ;yes, so subtract 100 from rem,
        ADDGE   a1, a1, #&10000000      ;and increment 1000 digit
        BGE     thous                   ;then try again

hunds
        CMP     a_exp, #10              ;any more hundreds?
        SUBGE   a_exp, a_exp, #10       ;yes, so subtract 10 from rem,
        ADDGE   a1, a1, #&01000000      ;and increment 100 digit
        BGE     hunds                   ;then try again

        ORR     a1, a1, a_exp, LSL #20  ;put in tens digit

        MOVS    r15, r14                ;then go home...

funnyUtoDEC

; We have max or min. exponent - zero, denorm, infinity or NaN

        CMP     r3, #0                  ;max. exponent?
        BNE     maxexp_UtoDEC           ;yes...

        ORRS    r5, r1, r2              ;zero?
        MOVEQ   r3, #&e0000000          ;if yes, just label as packed DEC,
        MOVEQ   r0, #0                  ;make sure +0,
        MOVEQ   r15, r7                 ;and go home...

; We have a denormalised no. - normalise and go back to conversion

        TST     r1, #&80000000          ;already normalised?
        BNE     nzero_UtoDEC

norm_UtoDEC
        ADDS    r2, r2, r2              ;double mantissa
        ADCS    r1, r1, r1
        SUB     r3, r3, #1              ;decrement exponent

        BPL     norm_UtoDEC             ;and repeat as needed
        B       nzero_UtoDEC            ;then go back to conversion

maxexp_UtoDEC

; Deal with infinity or NaN

        AND     r0, r0, #&80000000      ;just sign in r0

        ORR     r0, r0, r3, LSL #12     ;now set up max. exponent
        ORR     r0, r0, #&08000000

        ORR     r0, r0, r1, LSR #19     ;and deal with mantissa in case NaN
        MOV     r1, r1, LSL #13
        ORR     r1, r1, r2, LSR #19
        MOV     r2, r2, LSL #13

        MOV     r3, #&e0000000          ;label as DEC
        ret2                            ;and go home

pow18factor

;        DCD     &de0b6b3a               ;10^18
;        DCD     &76400000
;        DCD     &0000403a

power18factor

        DCD     &9392ee8e               ;1*10^-18
        DCD     &921d5d07               ;needed for DEC conversions
        DCD     &00003fc3

DECtoU
DECtoS
DECtoD
DECtoE

; First convert to unpacked precision and type, then reconvert as wanted

; Start by checking for zero, infinity or NaN

        ORRS    r5, r1, r2              ;zero or infinity?
        MOVEQS  r5, r0, LSL #20
        ANDNE   r5, r0, #&0f000000      ;or NaN?
        CMPNE   r5, #&0f000000
        BEQ     funnyDECtoU             ;yes, so deal with this...

; Next save important regs. and extract digits to binary

        STMDB   r12!, {r4,r6-r11,r14}   ;save some regs.

        MOV     w4, #0                  ;clear accumulated binary total
        MOV     w3, #0
        MOV     w1, a_sign, LSL #20     ;decimal word
        MOV     w2, #3                  ;number of nibbles to extract
        BL      grab                    ;put them in w3,w4

        MOV     w1, a1                  ;now get the nibbles from a1
        MOV     w2, #8                  ;ie 8 of them
        BL      grab

        MOV     w1, a2                  ;then a2
        MOV     w2, #8
        BL      grab

; Now start to install binary in a 

        MOV     a2, w4                  ;put back as mantissa in a
        MOVS    a1, w3
        MOV     a_exp, #&3e             ;start to arrange exponent in a
        ADD     a_exp, a_exp, #&4000

        BMI     normed                  ;and if needed, normalise
nor_loop
        ADDS    a2, a2, a2              ;double mantissa
        ADCS    a1, a1, a1
        SUB     a_exp, a_exp, #1        ;decrement exponent
        BPL     nor_loop                ;and repeat as necessary...
normed

; Now get power of 10 right

        MOV     w3, #0                  ;now get exponent in binary
        MOV     w4, #0
        MOV     w1, a_sign, LSL #4      ;from word in a_sign
        MOV     w2, #4                  ;(4 nibbles)
        BL      grab                    ;exponent now in w4
        MOV     w5, w4                  ;hence w5

        BL      bis10tow5               ;b is 10^w5, a preserved

; Return value = initial value * 10^exponent

        TST     a_sign, #&40000000      ;is exponent + or -?
        AisAoverB       NE              ;if -, divide,
        AisAtimesB      EQ              ;else if +, multiply

        ADR     b_sign, power18factor   ;divide by 10^18
        LDMIA   b_sign, {b1,b2,b_exp}
        AisAtimesB

        LDMIA   r12!, {r4,r6-r11,r14}   ;retrieve important regs

        ADDS    r5, r3, #1              ;have we underflowed?
        BLE     under_DECtoU            ;yes...
        MOVS    r5, r5, LSR #15         ;have we overflowed?
        BNE     over_DECtoU             ;yes...
   
        AND     r4, r4, #&7             ;change conversion to from U type
        B       Convert                 ;and convert to desired format...

bis10tow5

; We must calculate b = 10^exponent

        CMP     w5, #20                 ;is power small enough to look up?
        ADRLE   b1, tentables           ;yes, so do so
        ADDLE   w5, b1, w5, LSL #4
        LDMLEIA w5, {b_sign,b1,b2,b_exp} ;get answer
        MOVLES  r15, r14                ;and go home...

        STMDB   r12!, {a_sign,a1,a2,a_exp,r14} ;save initial value, lr

        MOV     a1, #&80000000          ;put 1.0 in a
        MOV     a2, #0
        MOV     a_sign, #&4000
        SUB     a_exp, a_sign, #1    

        CMP     w5, #0                  ;is this to be power zero?
        BEQ     result_in_a_c           ;yes, so return 1.0

        STMDB   r12!, {a_sign,a1,a2,a_exp} ;no, then put 1.0 on top of stack

        ADD     a_exp, a_exp, #3        ;put 10.0 in a
        MOVS    a1, #&a0000000          ;setting C flag also

clear_bit_c
        AisAsquared     CC              ;square running factor/drop through

int_pow_loop_c
        MOVS    w5, w5, LSR #1          ;test bit in power integer
        BCC     clear_bit_c             ;clear bit, so leave total alone

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;bit set, so retrieve total,
        STMNEDB r12!, {a_sign,a1,a2,a_exp} ;save running factor if needed again

        AisAtimesB                      ;total = total * running factor

        BEQ     result_in_a_c           ;calculation done...

        LDMIA   r12!, {b_sign,b1,b2,b_exp} ;retrieve running factor
        STMDB   r12!, {a_sign,a1,a2,a_exp} ;and save total

        MOV     a_sign, b_sign          ;a=b
        MOV     a1, b1
        MOV     a2, b2
        MOV     a_exp, b_exp
        AisAtimesB                      ;hence a=b^2

        B       int_pow_loop_c          ;and carry on...

result_in_a_c

        MOV     b_sign, a_sign          ;b=a
        MOV     b1, a1
        MOV     b2, a2
        MOV     b_exp, a_exp

        LDMIA   r12!, {a_sign,a1,a2,a_exp,r14} ;retrieve initial value, lr
        MOVS    r15, r14                ;then go home

tentables

; Powers of 10 from 0 up to 20 for fast look-up

        DCD     0                       ;positive
        DCD     &80000000               ;10^0 = 1.0
        DCD     0
        DCD     &3fff

        DCD     0                       ;positive
        DCD     &a0000000               ;10^1
        DCD     0
        DCD     &4002

        DCD     0                       ;positive
        DCD     &c8000000               ;10^2
        DCD     0
        DCD     &4005

        DCD     0                       ;positive
        DCD     &fa000000               ;10^3
        DCD     0
        DCD     &4008

        DCD     0                       ;positive
        DCD     &9c400000               ;10^4
        DCD     0
        DCD     &400c

        DCD     0                       ;positive
        DCD     &c3500000               ;10^5
        DCD     0
        DCD     &400f

        DCD     0                       ;positive
        DCD     &f4240000               ;10^6
        DCD     0
        DCD     &4012

        DCD     0                       ;positive
        DCD     &98968000               ;10^7
        DCD     0
        DCD     &4016

        DCD     0                       ;positive
        DCD     &bebc2000               ;10^8
        DCD     0
        DCD     &4019

        DCD     0                       ;positive
        DCD     &ee6b2800               ;10^9
        DCD     0
        DCD     &401c

        DCD     0                       ;positive
        DCD     &9502f900               ;10^10
        DCD     0
        DCD     &4020

        DCD     0                       ;positive
        DCD     &ba43b740                ;10^11
        DCD     0
        DCD     &4023

        DCD     0                       ;positive
        DCD     &e8d4a510               ;10^12
        DCD     0
        DCD     &4026

        DCD     0                       ;positive
        DCD     &9184e72a               ;10^13
        DCD     0
        DCD     &402a

        DCD     0                       ;positive
        DCD     &b5e620f4               ;10^14
        DCD     &80000000
        DCD     &402d

        DCD     0                       ;positive
        DCD     &e35fa931               ;10^15
        DCD     &a0000000
        DCD     &4030

        DCD     0                       ;positive
        DCD     &8e1bc9bf               ;10^16
        DCD     &04000000
        DCD     &4034

        DCD     0                       ;positive
        DCD     &b1a2bc2e               ;10^17
        DCD     &c5000000
        DCD     &4037

        DCD     0                       ;positive
        DCD     &de0b6b3a               ;10^18
        DCD     &76400000
        DCD     &403a

        DCD     0                       ;positive
        DCD     &8ac72304               ;10^19
        DCD     &89e80000
        DCD     &403e

        DCD     0                       ;positive
        DCD     &ad78ebc5               ;10^20
        DCD     &ac620000
        DCD     &4041

grab

; Take the w2 most significant nibbles of w1 as decimal digits, and insert them
; in the binary total in w3,w4

        ADDS    w4, w4, w4              ;double total
        ADC     w3, w3, w3

        ADDS    b1, w4, w4, LSL #2      ;then multiply by 5
        ADC     b2, w3, w4, LSR #30
        ADD     w3, b2, w3, LSL #2
        MOV     w4, b1

        ADDS    w4, w4, w1, LSR #28     ;add 1 nibble in total
        ADC     w3, w3, #0

        MOV     w1, w1, LSL #4          ;then remove it from w1

        SUBS    w2, w2, #1              ;and repeat as required
        BNE     grab

        MOVS    r15, r14

funnyDECtoU

; Max exponent or zero mantissa - NaN, infinity or zero

        MOV     a_exp, a_sign, LSL #5   ;set up exponent
        MOV     a_exp, a_exp, LSR #17

        MOV     a2, a2, LSR #13         ;in case a NaN, transfer mantissa
        ORR     a2, a2, a1, LSL #19
        MOV     a1, a1, LSR #13
        ORR     a1, a1, a_sign, LSL #19
        BIC     a1, a1, #&80000000      ;clearing bit copied from exponent
 
        AND     r4, r4, #&7             ;change conversion to from U type
        B       Convert                 ;and convert to desired format...

under_DECtoU

; We have number too small for U, unless denormed - if S or D, certain ufl

        ANDS    r4, r4, #7              ;change to conversion from U
        CMPNE   r4, #5
        BEQ     under_U                 ;conversion to U or E

        AND     a_sign, a_sign, #&80000000 ;return zero if untrapped
        CMP     r4, #1                  ;is this S precision?
        BEQ     und_u_s                 ;yes, so underflow for single
        B       und_u_d                 ;else underflow for double

under_U

        RSB     a_exp, a_exp, #0        ;no. of bits to be shifted
        CMP     a_exp, #64              ;is this going to underflow?
        BGE     und_u                   ;yes...

        RSBS    r5, r3, #32             ;is this greater than 32?

        SUBMI   r5, r3, #32             ;yes, so shift > 1 word
        MOVMI   r2, r1, LSR r5

        MOVPL   r2, r2, LSR r3          ;shift right by desired amount
        ORRPL   r2, r2, r1, LSL r5

        MOV     r1, r1, LSR r3

        MOV     r3, #0                  ;zero exponent

        AND     r4, r4, #&7             ;change conversion to from U type
        B       Convert                 ;and convert to desired format...

und_u
        AND     a_sign, a_sign, #&80000000 ;return 0 if untrapped
        MOV     a1, #0
        MOV     a2, #0
        MOV     a_exp, r4, LSL #29      ;with correct type

        MOV     r4, #UFL_E_MASK
        B       check_exep

over_DECtoU

; We have num,ber too big for U, so overflow, dealing with S or D returns

        ANDS    r4, r4, #7              ;change to conversion from U
        BEQ     over_U                  ;conversion to U...
        MOV     a_sign, #0
        CMP     r4, #3                  ;is it conversion to double?
        BEQ     overflow_u_d            ;yes, so overflow for double...
        BLT     overflow_u_s            ;no, single, so overflow for that

over_U

; Put something that is infinity in U or E type

        MOV     a1, #0                  ;return infinity if untrapped
        MOVS    a2, #0,2
        RSC     a_exp, a2, #&8000
        AND     a_sign, a_sign, #&80000000
        ORR     a_sign, a_sign, a_exp
        CMP     r4, #0
        MOVNE   a_exp, #&a0000000

        MOV     r4, #OFL_E_MASK
        B       check_exep

;******************************************************************************
;
;       Debugging code
;

debug
        [       debugging

        SUB     r13, r13, #100          ;protect stack
        STMDB   r12!, {r0-r11,r13,r14}

         [      filelevel = 0
         mess   ,"File level = 0 (top)",NL,NL
         ]
         [      filelevel = 1
         mess   ,"File level = 1 (mid)",NL,NL
         ]
         [      filelevel = 2
         mess   ,"File level = 2 (util)",NL,NL
         ]
         [      filelevel = 3
         mess   ,"File level = 3",NL,NL
         ]

        mess    ,"---r0--- ---r1--- ---r2--- ---r3--- ---r4--- ---r5--- ---r6--- ---r7---",NL
        wrhex   r0
        wrhex   r1
        wrhex   r2
        wrhex   r3
        wrhex   r4
        wrhex   r5
        wrhex   r6
        wrhex   r7
        mess    ,NL
        wrhex   r8
        wrhex   r9
        wrhex   r10
        wrhex   r11
        wrhex   r12
        wrhex   r13
        wrhex   r14
        wrhex   r15
        mess    ,NL,NL

        LDMIA   r12!, {r0-r11,r13,r14}
        ADD     r13, r13, #100

        ret


PHEX
        push    ,LR
        MOV     R1,#32-4
PHLOOP
        MOV     R0,R2,LSR R1
        AND     R0,R0,#&F
        CMPS    R0,#10
        ADDCC   R0,R0,#"0"
        ADDCS   R0,R0,#"A"-10
        SWI     WriteC
        SUBS    R1,R1,#4
        BPL     PHLOOP
        MOV     R0,#" "
        SWI     WriteC
        pull    ,,LR
        MOVS    pc,LR


        |
        B       debug                   ;if not debugging, this call stiffs
        ]


filelevel       SETA    filelevel - 1
        END
