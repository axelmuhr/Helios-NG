; -> fpemid/s
filelevel       SETA    filelevel + 1

;
;******************************************************************************
;
;       FPE2mid         5/11/86  Martin Clemoes
;
;       Description :   This file contains the medium level of the source for
;                       the 2 micron arm fpe, and is assembled as a file
;                       included by the top level. It includes the low-level
;                       maths routines, and its code decodes the fp instrs.
;
;******************************************************************************
;
;       First some definitions
;

NAN_E_MASK      *       &00000001       ;status reg. bit for nan event
IVO_E_MASK      *       &00000001       ;status reg. bit for inv. op. event
DVZ_E_MASK      *       &00000002       ;status reg. bit for dvz. event
OFL_E_MASK      *       &00000004       ;status reg. bit for ov. event
UFL_E_MASK      *       &00000008       ;status reg. bit for und event
INX_E_MASK      *       &00000010       ;status reg. bit for inexact event

;******************************************************************************
;
;
;       Decode fp instruction - critical path code...
;

decode

; Produce switch value from op-code in r9.

        AND     r8, r9, #&03f80000      ;get 7 suitable bits

        ANDS    r7, r9, #&00000010      ;get bit 4 (and set Z on it)
        AND     r7, r7, r9, LSR #21     ;then and it with bit 25
        EOR     r8, r8, r7, LSL #20     ;and put in bit 24 to show CPRT/CPST

        ANDEQ   r7, r9, #&02000000      ;if bit 4 clear, get bit 25
        ANDEQ   r7, r7, r9, LSL #10     ;then and with bit 15 to show monadic
        ADDEQ   r8, r8, r7              ;and if it is, put 100 in bits 26-24

; We demand that CP#=1

        AND     r7, r9, #&00000f00      ;get bits showing CP identity
        TEQ     r7, #&00000100          ;and check that this is for CP 1

; Now check for condition codes, also that bit 27 is set - if not, illegal in.
; The most common cond. is AL, ie top nibble = &e

        ANDEQ   r7, r9, #&f8000000      ;cond. bits and bit 27 (must be set)
        TEQEQ   r7, #&e8000000          ;legal & AL?

cond_passed
        ADDEQ   pc, pc, r8, LSR #17     ;yes, so switch on r8 value

        B       cond_test               ;no, so test legal & conditions

; Now for the switch tables

; CPDT's - 32 different ones, each in table twice as last switch bit rubbish
; Bits 000xxxxx or 001xxxxx in switch -> CPDT

        B       cpdt00000
        B       cpdt00000
        B       cpdt00001
        B       cpdt00001
        B       cpdt00010
        B       cpdt00010
        B       cpdt00011
        B       cpdt00011
        B       cpdt00100
        B       cpdt00100
        B       cpdt00101
        B       cpdt00101
        B       cpdt00110
        B       cpdt00110
        B       cpdt00111
        B       cpdt00111
        B       cpdt01000
        B       cpdt01000
        B       cpdt01001
        B       cpdt01001
        B       cpdt01010
        B       cpdt01010
        B       cpdt01011
        B       cpdt01011
        B       cpdt01100
        B       cpdt01100
        B       cpdt01101
        B       cpdt01101
        B       cpdt01110
        B       cpdt01110
        B       cpdt01111
        B       cpdt01111
        B       cpdt10000
        B       cpdt10000
        B       cpdt10001
        B       cpdt10001
        B       cpdt10010
        B       cpdt10010
        B       cpdt10011
        B       cpdt10011
        B       cpdt10100
        B       cpdt10100
        B       cpdt10101
        B       cpdt10101
        B       cpdt10110
        B       cpdt10110
        B       cpdt10111
        B       cpdt10111
        B       cpdt11000
        B       cpdt11000
        B       cpdt11001
        B       cpdt11001
        B       cpdt11010
        B       cpdt11010
        B       cpdt11011
        B       cpdt11011
        B       cpdt11100
        B       cpdt11100
        B       cpdt11101
        B       cpdt11101
        B       cpdt11110
        B       cpdt11110
        B       cpdt11111
        B       cpdt11111

; CPDO's - every odd entry is same as last but with one of new 3 word sizes
; Bits 010xxxxx in switch -> dyadic CPDO
; 32 dyadic ones

        B       add
        B       add3
        B       muf
        B       muf3
        B       suf
        B       suf3
        B       rsf
        B       rsf3
        B       dvf
        B       dvf3
        B       rdf
        B       rdf3
        B       pow
        B       pow3
        B       rpw
        B       rpw3
        B       rmf
        B       rmf3
        B       fml
        B       fml3
        B       fdv
        B       fdv3
        B       frd
        B       frd3
        B       pol
        B       pol3
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction

; CPRT's and CPST's - this assumes these all have different abc opcode parts.
; Bits 011xxxxx in switch -> CPRT or CPST

        B       mfrom_arm
        B       mfrom_arm3
        B       mto_arm
        B       mto_arm
        B       wstat
        B       wstat
        B       rstat
        B       rstat
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction

        B       very_ill_instruction
        B       very_ill_instruction
        B       cmf
        B       cmf
        B       very_ill_instruction
        B       very_ill_instruction
        B       cnf
        B       cnf
        B       very_ill_instruction
        B       very_ill_instruction
        B       cmfe
        B       cmfe
        B       very_ill_instruction
        B       very_ill_instruction
        B       cnfe
        B       cnfe

; CPDO's - every odd entry is same as last but with one of new 3 word sizes
; Bits 100xxxxx in switch -> monadic CPDO 
; 32 monadic ones

        B       mvf
        B       mvf3
        B       mnf
        B       mnf3
        B       abs
        B       abs3
        B       rnd
        B       rnd3
        B       sqt
        B       sqt3

        B       log
        B       log3
        B       lgn
        B       lgn3
        B       exp
        B       exp3
        B       sin
        B       sin3
        B       cos
        B       cos3
        B       tan
        B       tan3
        B       asn
        B       asn3
        B       acs
        B       acs3
        B       atn
        B       atn3
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction
        B       very_ill_instruction

;******************************************************************************
;
;       Some general macros
;

; obvious

        MACRO
        ret
        MOV     r15,r14
        MEND

; funny return

        MACRO
        ret2
        MOV     r15, r7
        MEND


;******************************************************************************
;
;
;       Come here to make sure bit 27 set (illegal if not) and condition met
;

;       Inputs: r14 - user's lr
;               r9  - instruction causing fuss
;               r7  - r9 and #&f800 0000
;
;       Outputs r11, r7 smashed

cond_test

; First check that CP#=1

        TST     r9, #&00000e00          ;if CP#=1, bits 9-11 clear
        BNE     very_ill_instruction    ;if not, illegal...
        TST     r9, #&00000100          ;and bit 8 set

; Next test that bit 27 is set - ie reject CU1 or CU2 as illegal

        TSTNE   r9, #&08000000          ;this tested here for speed reasons
        BEQ     very_ill_instruction

; Now make sure that cp instruction condition is met - if not, give up

        ADR     r11, cond_table - 2     ;address will have 2 added due to b. 27
        LDR     r7, [r11, r7, LSR #26]  ;load word to go with instr. cond.

        MOV     r11, #16                ;and r7 with itself shifted right by 16
        ADD     r11, r11, r14, LSR #28  ;+ number from user status bits
        TST     r7, r7, LSR r11

        BEQ     cond_passed             ;zero means cond. passed, so execute

        exit7to15                       ;not zero means do not execute...

cond_table

; Below is a word of data for each of the 16 possible instruction conditions.
; The 2 ms bytes are &8000 for each condition - only 1 bit set - so that this
; part of the word can be rotated and anded with the less significant part.

; Bit = 0 -> execute, Bit = 1 -> ignore.                NZCV
;                                                       ----
  &  2_10000000000000001111000011110000         ; 0 EQ  x1xx
  &  2_10000000000000000000111100001111         ; 1 NE  x0xx

  &  2_10000000000000001100110011001100         ; 2 CS  xx1x
  &  2_10000000000000000011001100110011         ; 3 CC  xx0x

  &  2_10000000000000001111111100000000         ; 4 MI  1xxx
  &  2_10000000000000000000000011111111         ; 5 PL  0xxx

  &  2_10000000000000001010101010101010         ; 6 VS  xxx1
  &  2_10000000000000000101010101010101         ; 7 VC  xxx0

  &  2_10000000000000001100111111001111         ; 8 HI  x01x
  &  2_10000000000000000011000000110000         ; 9 LS  x00x x10x x11x

  &  2_10000000000000000101010110101010         ; a GE  0xx0 1xx1
  &  2_10000000000000001010101001010101         ; b LT  1xx0 0xx1

  &  2_10000000000000000101111110101111         ; c GT  00x0 10x1
  &  2_10000000000000001010000001010000         ; d LE  10x0 00x1 x1xx

  &  2_10000000000000000000000000000000         ; e AL  xxxx
  &  2_10000000000000001111111111111111         ; f NV  never


        GET     fpeutils.s      ; subroutines for trig

        GET     fpecpdt.s       ; cpdt operation routines

        GET     fpecprt.s       ; cprt operation routines

        GET     fpecpst.s       ; cpst operation routines

        GET     fpecpdom.s      ; cpdo monadic operation routines

        GET     fpecpdod.s      ; cpdo dyadic operation routines

        GET     fpelogs.s       ; implements LOG, LGN, EXP

        GET     fpetrig.s       ; implements SIN, COS, TAN,
                                ; ASN, ACS, ATN and POL (ATN2)

;******************************************************************************
;
;       Un-implemented commands - branch to illegal instruction routine
;


;        B       very_ill_instruction

;******************************************************************************

filelevel SETA  filelevel - 1
        END
