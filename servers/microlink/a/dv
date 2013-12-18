; $Header: divide.s,v 1.1 90/12/21 20:52:31 charles Locked $
; $Source: /server/usr/users/b/charles/world/microlink/RCS/source/divide.s,v $

;-------------------------------------------------------------------------
;                                          Fixed point division algoritm
;-------------------------------------------------------------------------

; This is a fixed point division function.

;-------------------------------------------------------------------------
;                                                 Getting helios headers
;-------------------------------------------------------------------------

        ; If in helios mode, the following header files have to be
        ;    included ...
        
        GET listopts.s
        GET arm.s
        GET basic.s
        GET structs.s
        GET module.s
        GET exmacros.s
        GET SWIinfo.s
        GET hardABFP.s

;-------------------------------------------------------------------------
;                                                          Start of file
;-------------------------------------------------------------------------

        ; Start-of-module macro

        StartModule hcrElastic,-1,100

;-------------------------------------------------------------------------
;                                           Export of external functions 
;-------------------------------------------------------------------------

        ; At this point, all externally accessible symbols are exported
        
        static
        static_extern_func DigDivide
        static_end

;-------------------------------------------------------------------------
;                                                   Fixed point division
;-------------------------------------------------------------------------

; From 'C'  :  int DigDivide(int n,int d);

; Computes the value of n divided by d, but to 16 significant binary digits:
;    i.e. the result is implicitly multiplied up by 2^16.

DigDivide   FnHead

        MOV     ip,sp                   ; Save stack-pointer
        STMFD   sp!,{fp,ip,lr,pc}       ; Save some registers
        SUB     fp,ip,#4                ; fp points to pc on stack

        MOVS    lr,#0                   ; Sign of result
        MOVS    R0,R0                   ; Test R0
        EORMI   lr,lr,#1                ; Alter result sign
        RSBMI   R0,R0,#0                ; Get absolute numerator
        MOVS    R1,R1                   ; Test R1
        EORMI   lr,lr,#1                ; Alter result sign
        RSBMI   R1,R1,#0                ; Get absoulte denominator
        
        MOV     R2,R0,ASR#16            ; Get numerator high word
        MOV     R3,R0,LSL#16            ; Get numerator low  word
        MOV     R0,#1                   ; Result register
        
;--------------------------------------------------------------------------+
divLoop                                 ; Once per result bit              |
        ADDS    R3,R3,R3                ; Shift numerator left ...         |
        ADC     R2,R2,R2                ; ... continued                    |
        CMPS    R2,R1                   ; Compare numerator with denom.    |
        SUBCS   R2,R2,R1                ; Subtract if possible             |
        ADCS    R0,R0,R0                ; Shift result bit                 |
        BCC     divLoop                 ; Continue until sentinel shifted  |
;--------------------------------------------------------------------------+
        CMPS    lr,#0                   ; Is result negative?
        RSBNE   R0,R0,#0                ; Reverse result sign if so

        LDMEA   fp,{fp,sp,pc}^          ; Reload registers, return w/ flags

;-------------------------------------------------------------------------
;                                                          End of Module
;-------------------------------------------------------------------------

        EndModule


 END
