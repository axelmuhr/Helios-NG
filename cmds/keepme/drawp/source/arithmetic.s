;----------------------------------------------------------------------
;                                                        arithmetic.s
;----------------------------------------------------------------------

; $Header: arithmetic.s,v 1.7 90/07/01 17:27:06 charles Locked $
; $Source: /server/usr/users/a/charles/world/drawp/RCS/source/arithmetic.s,v $

; This assembler file contains some fixed-point and some high-precision
;   arithmetic routines that the 'C' code would fine a little hard
;   to handle, hence here it is in assembler

;-------------------------------------------------------------------
;                                               The 'SELECT' macro
;-------------------------------------------------------------------

; This macro is used for string selection: It assigns $t to $l if
;   $c is true, otherwise it assigns $e to $l.

       MACRO
$l     SELECT  $c,$t,$e
       [ $c
$l     SETS   $t
       |
$l     SETS   $e
       ]
       MEND

;-------------------------------------------------------------------
;                                                externLabel macro
;-------------------------------------------------------------------

        MACRO
$label  externLabel
        [ heliosMode
$label  procsym
        |
        LCLS quotedLabel
quotedLabel SETS "$label"
quotedLabel SETS "|_" :CC: quotedLabel :CC: "|"
$quotedLabel
        ]
        MEND

;-------------------------------------------------------------------
;                                                   Helios control
;-------------------------------------------------------------------

; This file must assemble differently according to whether we are 
;    assembling for Helios or Unix: In Helios a proper Helios
;    module must be generated.

        GET    objects/giveMode.ss  ; Find out if in Helios mode
        
	; Conditionally include helios headers: We have to do this
	;    in the following way because you cannot have a 'GET' in
	;    the middle of a consditional peice of assembley:
	
        GBLS getCommand
getCommand SELECT heliosMode,"GET",";"
        $getCommand heliosHdr/listopts.s
        $getCommand heliosHdr/arm.s
        $getCommand heliosHdr/basic.s
        $getCommand heliosHdr/structs.s
        $getCommand heliosHdr/module.s
        
        [ heliosMode                ; If in Helios Mode ...
        OPT    32                   ; Reset printer options
        StartModule arithmetic,-1,0 ; Start of Module
        static                      ; Define externally availiable locations :
        static_extern_func dpGetOrientation
        static_extern_func dpVectorLength
        static_extern_func dpUnitComponent
        static_extern_func dpNormalizeLength
        static_extern_func dpAddLengths
        static_extern_func dpDecodeLength
        static_extern_func dpMeasureComponent
        static_end                  ; End of static area
        
        |                           ; If not in Helios :
        AOUT                        ; Output format directive
        EXPORT |_dpGetOrientation|  ; Export various symbols ...
        EXPORT |_dpVectorLength|
        EXPORT |_dpUnitComponent|
        EXPORT |_dpNormalizeLength|
        EXPORT |_dpAddLengths|
        EXPORT |_dpDecodeLength|
        EXPORT |_dpMeasureComponent|
        AREA   arithmeticCode,CODE
        ]

;----------------------------------------------------------------------
;                                                         Other files
;----------------------------------------------------------------------

; 'utility.ss' contains register bindings, and 'constants.ss' contains
;    copies of some manifest constants source-defined in the 'C' code:

        GET    lochdr/utility.ss
        GET    objects/unix/constants.ss

;----------------------------------------------------------------------
;                                                         Validate Bf
;----------------------------------------------------------------------

; Bf cannot be greater than 11 due to the current limitations in
;   accuracy dictated by the algorithms:
; The origin of definition for this value is the definition of
;   'BinaryFigures' in 'coordinates.h' and reaches this file
;   by inclusion of 'objects/constants.ss' which is generated
;   by the file 'source/constantXfer.c'

       [ Bf>11
       ! " 'BinaryFigures' or 'Bf' cannot be greater than 11 "
       ]

;----------------------------------------------------------------------
;                           The 'SMult' macro : Signed multiplication
;----------------------------------------------------------------------

; This macro multiplies two 32-bit signed numbers to obtain a 64-bit
;    signed number. All the registers passed to this routine must be
;    distinct.
; The routine multiplies register $x by register $y and leaves the
;    result in registers ($rl,$rh) with the higher-significant bits
;    in $rh.
; The idea is that if we interpret the lower sixteen bits of $x
;    in an unsigned way as a value B and the higher sixteen bits
;    in a signed way as value A and similarly for $y with C (higher),
;    and D (lower), then:
; $x = (A*2^16) + B     ;    $y = (C*2^16) + D
; And it follows by multiplying out that:
; $x * $y = (A*C*2^32) + (A*D*2^16) + (B*C*2^16) + B*D
; Where:
;        -2^15 <= A < 2^15
;            0 <= B < 2^16
;        -2^15 <= C < 2^15
;            0 <= D < 2^16
; So:
;        -2^30 < A*C < 2^30
;        -2^31 < A*D < 2^31
;        -2^31 < B*C < 2^31
;           0 <= B*D < 2^32
; Are certainly true, hence all intermediate results may be held in
;    intermediate variables, but we must be extremely careful as to
;    how sign extension occurs.

        MACRO
$label  SMult $x,$y,$rl,$rh,$t1,$t2
$label
        
        MOV   $t1,$x,ASR#16     ; t1 <- A   ( = x ASR 16 )
        SUB   $x,$x,$t1,LSL#16  ; x  <- B   ( = x LOW 16 )
        MOV   $t2,$y,ASR#16     ; t2 <- C   ( = y ASR 16 )
        SUB   $y,$y,$t2,LSL#16  ; y  <- D   ( = y LOW 16 )
        MUL   $rl, $x, $y       ; rl <- B.D ( = x.y      )
        MUL   $rh,$t1,$t2       ; rh <- A.C ( = t1.t2    )
        MUL   $x,$t2,$x         ; x  <- C.B ( = t2.x     )
        MUL   $y,$t1,$y         ; y  <- A.D ( = t1.y     )
        ADDS  $rl,$rl,$y,ASL#16 ; (rl,rh) <- (rl,rh) + (A.D*2^16) ...
        ADC   $rh,$rh,$y,ASR#16 ; ... continued
        ADDS  $rl,$rl,$x,ASL#16 ; (rl,rh) <- (rl,rh) + (C.B*2^16) ...
        ADC   $rh,$rh,$x,ASR#16 ; ... continued
        MEND

;----------------------------------------------------------------------
;                                The 'UDiv' macro : Unsigned Division
;----------------------------------------------------------------------

; This macro compiles code to divide a 32-bit signed quantity by a 32-bit
;    unsigned quantity on the assumption that the denominator is greater
;    than the numerator. The result returned is the numerator divided by
;    the denominator, then  multiplied by 2^(16+Bf).
; If the denominator is greater than or equal to the numerator, then a 
;    result of the extreme value is returned.
; On entry, $n is in the range -(2^16) <= n < 2^16
;           $d is in the range 0       <= d < 2^32
; The result, $r, is in the range -(2^(16+Bf)) <= r < 2^(16+Bf).
; As mentioned, if on entry $d>=abs($n) then a value of + or -
;     2^(16+Bf)-1 will be returned.
; $n  is the  numerator   of the fraction to be computed
; $d  is the  denominator
; $r  is where the remainder is stored (always +ve)
; $q  is where the quaotient result is stored
; All the registers must be distinct. $d and $n are preserved

        MACRO
$label  UDiv   $n,$d,$r,$q

        MOVS   $r,$n               ; Load rem with num and check sign
        RSBMI  $r,$r,#0            ; Negate rem if applicable
        MOV    $q,#1:SHL:(16-Bf)   ; Get sentinel bit
;--------------------------------------------------------------------+
01                                 ; Once per bit-shift              |
        MOV    $r,$r,ASL#1         ; Shift remainder by one bit      |
        CMPS   $r,$d               ; Compare denom with remainder    |
        SUBCS  $r,$r,$d            ; If n>=d, subtract q properly,   |
        ADCS   $q,$q,$q            ; Shift quotient bit into result  |
        BCC    %BT01               ; Loop until sentinel shifted out |
;--------------------------------------------------------------------+
        MOVS   $n,$n               ; Check sign of numerator again
        RSBMI  $q,$q,#0            ; Negate $r if appl.
        MEND

;----------------------------------------------------------------------
;                                      getOrientation : Documentation
;----------------------------------------------------------------------

; In C -
; int dpGetOrientation(int x1,int y1,int x2,int y2);

; Used by routines in 'joinStyles.c' this routine computes the relative
;   orientations of the two vectors. The background is as follows:

; Two fat lines must be joined together by a particular join style. 
; The left- and right- hand corner points of the relevant ends of the
;   two lines are availiable. The left-corner is the one you would see
;   to your left if you were standing on that end of the line looking
;   outwards.
; It is necassary to determine which endpoints are essentially the
;   'outer' endpoints of the line. Is it the left-hand point on
;   the first line and the right-hand point on the second line, or the
;   right-hand point on the first line to the left-hand point on the
;   second line?
; We take two vectors: (x1,y1) is the vector from the left-hand point
;   of the first line to it's right hand point. (x2,y2) is the vector
;   from the left-hand point of the second line to the right hand point,
;   call these vectors l1 and l2, and consider them as vectors in
;   three-dimentional space where the third component is implicitly 0.
; Imagine rotating the co-ordinate axis so that the vector l1 is
;    horizontal pointing from left-to-right.
; Then we wish to know if the other vector l2 so-rotated points up into
;    the negative-y half plane in this co-ordinate system, or down into
;    the posotive-y half plane. In the first case, the routine
;    returns a negative and the outer points are the right-hand corner of
;    line 1, and the left-hand corner of line 2. In the other case, the
;    routine returns a posotive number, and the outer points are the
;    left-hand corner of line1 and the right-hand corner of line2.
; It is a mathmatical fact that if we take l1^l2, the cross-product of
;    l1 and l2, then the result is a vector whose only component is
;    parallel to the third axis and is posotive if and only if the
;    vector l1 is in the posotive perpendicular half plane of l2.
; This routine performs a multiplication which is double-precision:
;    two signed 32-bit numbers are multiplied to form a double precision
;    signed 64-bit number during this calculation.

;----------------------------------------------------------------------
;                                                    dpGetOrientation
;----------------------------------------------------------------------

; This routine is described in the section above.
; It's declaration in 'C' is:
; int dpGetOrientation(int x1,int y1,int x2,int y2);

        ; At head of file : EXPORT |_dpGetOrientation|
        ; There should be a text version of the function name here,
        ;   but I don't know how to put it in

dpGetOrientation  externLabel
        MOV    ip,sp                    ; PCS stuff ...
        STMFD  sp!,{R4-R7,fp,ip,lr,pc}  ; ... continued ...
        SUB    fp,ip,#4                 ; ... continued
        SMult  R0,R3,R4,R5,R6,R7        ; Get x1*y2 in (R4,R5)
        SMult  R1,R2,R6,R7,R0,R3        ; Get x2*y1 in (R6,R7)
        SUBS   R1,R4,R6                 ; Get x1*y2-x2*y1 in (R1,R0) ...
        SBC    R0,R5,R7                 ; ... continued
        LDMDB  fp,{R4-R7,fp,sp,pc}^     ; And return
        
;----------------------------------------------------------------------
;                                                      dpVectorLength
;----------------------------------------------------------------------

; This vector computes a special 32 bit floating point representation
;    for the length of a vector. The entry to the routine are the two
;    sides of the vector in sub-pixel units. These values are squared,
;    added, and the square root of the result is then computed.

; The result floating point representation is stored in a 32-bit
;    unsigned integer result. The lower 5 bits are the exponent (unsigned),
;    and the next 16+Bf bits are the mantissa representing a number
;    in the range [0,1) which is multiplied by 2^(the exponent)

; Declaration from 'C':

; int dpVectorLength(int dx,int dy);

; The return length should be normalised
          
        ; At head of file : EXPORT |_dpVectorLength|

dpVectorLength externLabel

        MOV    ip,sp                     ; PCS stuff ...
        STMFD  sp!,{R4,fp,ip,lr,pc}      ; ... continued ...
        SUB    fp,ip,#4                  ; ... continued
        
        MOVS   R0,R0                     ; Get abs(dx) ...
        RSBMI  R0,R0,#0                  ; ... continued
        MOVS   R1,R1                     ; Get abs(dy) ...
        RSBMI  R1,R1,#0                  ; ... continued
        
        MOV    ip,R0,LSR#16              ; Get upper 16 bits of dx
        SUB    R0,R0,ip,LSL#16           ; Get lower 16 bits of dx
        MUL    R2,R0,R0                  ; Get LO(dx)^2
        MUL    R3,ip,ip                  ; Get HI(dx)^2
        MUL    R0,ip,R0                  ; Get HI(dx)*LO(dx)
        ADDS   R2,R2,R0,LSL#17           ; Add in HI(dx)*LO(dx)*2 ...
        ADC    R3,R3,R0,LSR#15           ; ... continued
        MOV    ip,R1,LSR#16              ; Get upper 16 bits of dy
        SUB    R1,R1,ip,LSL#16           ; Get lower 16 bits of dy
        MUL    R0,R1,R1                  ; Add in LO(dy)^2 ...
        ADDS   R2,R2,R0                  ; ... continued
        MUL    R0,ip,ip                  ; Add in HI(dy)^2 ...
        ADC    R3,R3,R0                  ; ... continued
        MUL    R0,ip,R1                  ; Get HI(dy)*LO(dy)
        ADDS   R2,R2,R0,LSL#17           ; Add in HI(dy)*LO(dy)*2 ...
        ADCS   R3,R3,R0,LSR#15           ; ... continued
        
        MOVEQ  R3,R2                     ; Multiply result up by
        MOVEQ  R2,#0                     ;   2^32 if small, and
        MOVEQ  ip,#16                    ;   record change
        MOVNE  ip,#32                    ; Else record no change
        MOV    R4,#0                     ; Start bit-shift count
        
        MOVS   R0,R3,LSR#16              ; Check upper 16 bits of R3
        EOREQ  R0,R3,R0,LSL#16           ; Get lower bits if they're zero
        ADDEQ  R4,R4,#16                 ; And increment R4
        MOVS   R1,R0,LSR#8               ; Check upper 8 bits of remainder
        EOREQ  R1,R0,R1,LSL#8            ; Get lower bits if they're zero
        ADDEQ  R4,R4,#8                  ; And increment R4
        MOVS   R0,R1,LSR#4               ; Check upper 4 bits of remainder
        EOREQ  R0,R1,R0,LSL#4            ; Get lower bits if they're zero
        ADDEQ  R4,R4,#4                  ; And increment R4
        MOVS   R1,R0,LSR#2               ; Check upper 2 bits of remainder
        ADDEQ  R4,R4,#2                  ; Increment R4 appropriately

        RSB    lr,R4,#32                 ; Get 32-<shift count>
        MOV    R3,R3,LSL R4              ; Shift (R2,R3) ...
        ORR    R3,R3,R2,LSR lr           ; ... continued ...
        MOV    R2,R2,LSL R4              ; ... continued
        
        SUB    R4,ip,R4,LSR#1            ; Compute exponent

        ; Now all the significant bits of (R2,R3) have been shifted
        ;    in to the top most significant bits so that there is a '1'
        ;    in bit 30 or 31 of R3. The corresponding exponent adjustment
        ;    is in R4
        
        ; Now to compute the square root of the number in (R2,R3).
        ; Since the square of the number is a 2*Bf+32 bit number,
        ;   the result of the square root is going to be a
        ;   Bf+16 bit number, which fits in a single word.

        MOV    R0,#0                      ; Prepare 'D'
        MOV    R1,#1                      ; Prepare 'E'
        MOV    lr,#1                      ; Set up constant
        
        MOV    ip,#16                     ; Set up countdown
;-----------------------------------------------------------------------+
sqrtHiLoop                                ; Once per 2 bits from R3     |
        MOV    R0,R0,LSL#2                ; Shift bits into R0 ...      |
        ORR    R0,R0,R3,LSR#30            ; ... continued               |
        MOV    R3,R3,LSL#2                ; Shift R3                    |
        CMPS   R0,R1                      ; Compare 'D' with 'E'        |
        SUBGE  R0,R0,R1                   ; If D>=E take off E, and     |
        ADDGE  R1,R1,#2                   ;   increment E by 2          |
        RSB    R1,lr,R1,ASL#1             ; E := 2*E + 1                |
        SUBS   ip,ip,#1                   ; Decrement and branch ...    |
        BGT    sqrtHiLoop                 ; ... continued               |
;-----------------------------------------------------------------------+

        MOV    ip,#Bf                     ; Set up countdown
;-----------------------------------------------------------------------+
sqrtLoLoop                                ; Once per 2 bits from R2     |
        MOV    R0,R0,LSL#2                ; Shift bits into R0 ...      |
        ORR    R0,R0,R2,LSR#30            ; ... continued               |
        MOV    R3,R3,LSL#2                ; Shift R3                    |
        CMPS   R0,R1                      ; Compare 'D' with 'E'        |
        SUBGE  R0,R0,R1                   ; If D>=E take off E, and     |
        ADDGE  R1,R1,#2                   ;   increment E by 2          |
        RSB    R1,lr,R1,ASL#1             ; E := 2*E + 1                |
        SUBS   ip,ip,#1                   ; Decrement and branch ...    |
        BGT    sqrtLoLoop                 ; ... continued               |
;-----------------------------------------------------------------------+

        MOVS   R0,R1,LSR#2                ; Get result in R0
        MOVEQ  R4,#0                      ; If result is 0, so is exponent
        ORR    R0,R4,R0,LSL#5             ; Merge in exponent
        
        LDMDB  fp,{R4,fp,sp,pc}^          ; Return

;----------------------------------------------------------------------
;                                                     dpUnitComponent
;----------------------------------------------------------------------

; This routine is called from 'C' as:

; int dpUnitComponent(int c1,int l)

; 'c1' is a number in the range [-2^16,2^16) multiplied up by 2^Bf.
; 'l'  is a special floating point representation of a number,
;    the lower 5 bits are an unsigned bit-field representing an exponent
;    in the range 0..31, and the next 16+Bf bits represent a number
;    in the range [0,1) which when multiplied by 2^<the exponent>
;    represent the number to be interpreted for l.
; If 'l' is zero, c1 is returned untouched.
; The routine computes the value of c/l, multiplied up by 2^Bf.

        ; At head of file : EXPORT |_dpUnitComponent|

dpUnitComponent externLabel

        MOV    ip,sp                     ; PCS stuff ...
        STMFD  sp!,{fp,ip,lr,pc}         ; ... continued ...
        SUB    fp,ip,#4                  ; ... continued

        AND    R2,R1,#2_11111            ; Get exponent
        MOVS   R1,R1,LSR#5               ; Get mantissa
        BEQ    ucLenIsZero               ; Branch if l is zero
        RSBS   R2,R2,#(16+Bf)            ; Get c1 adjustment
        MOVPL  R0,R0,ASL R2              ; Adjust c1 if applicable
        UDiv   R0,R1,ip,R3               ; Get c/l in R3.
        RSBS   R2,R2,#0                  ; Test c1 adjustment
        MOV    R0,R3                     ; Get result in R0
        MOVPL  R0,R0,ASR R2              ; Adjust result if applicable
ucLenIsZero                              ; Here result is in R0
        LDMDB   fp,{fp,sp,pc}^           ; Return

;----------------------------------------------------------------------
;                                                  dpNormalizeLengths
;----------------------------------------------------------------------

; This routine is used to normalise the result a floating point length.

; Declaration from 'C' is:

; int dpNormalizeLength(int l);

        ; At head of file : EXPORT |_dpNormalizeLength|

dpNormalizeLength externLabel

        MOV    ip,sp                     ; PCS stuff ...
        STMFD  sp!,{fp,ip,lr,pc}         ; ... continued ...
        SUB    fp,ip,#4                  ; ... continued
        
        CMPS   R0,#(1:SHL:(5+16+Bf-1))   ; Is mantissa already big enough?
        LDMCSDB fp,{fp,sp,pc}^           ; Return if so
        ANDS   R1,R0,#2_11111            ; Get exponent
        LDMEQDB fp,{fp,sp,pc}^           ; Return if minimal
        BICS   R0,R0,#2_11111            ; Get mantissa
        LDMEQDB fp,{fp,sp,pc}^           ; Return with 0 if it's zero
;----------------------------------------------------------------------+
normalizeLoop                            ; Once per shift              |
        MOV    R0,R0,LSL#1               ; Shift mantissa              |
        SUBS   R1,R1,#1                  ; Adjust exponent             |
        BEQ    normalizeEnd              ; Exit if exp minimal         |
        CMPS   R0,#(1:SHL:(5+16+Bf-1))   ; Is mantissa big enough?     |
        BCC    normalizeLoop             ; Branch if not               |
;----------------------------------------------------------------------+
normalizeEnd                             ; Exit here
        ORR    R0,R0,R1                  ; Combine mantissa and exponent
        LDMDB  fp,{fp,sp,pc}^            ; Return

;----------------------------------------------------------------------
;                                                        dpAddLengths
;----------------------------------------------------------------------

; This function is used to add two lengths together, in the special
;    floating point representation

; 'C' declaration:

; int dpAddLengths(int l1,int l2);

; On entry, both lengths should be normalized, by use of 'dpNormalizeLength'
;   the result is always normalised


        ; At head of file : EXPORT |_dpAddLengths|

dpAddLengths externLabel

        MOV    ip,sp                     ; PCS stuff ...
        STMFD  sp!,{fp,ip,lr,pc}         ; ... continued ...
        SUB    fp,ip,#4                  ; ... continued
        
        AND    R2,R0,#2_11111            ; Get l1 exponent
        AND    R3,R1,#2_11111            ; Get l2 exponent
        SUB    R0,R0,R2                  ; Get l1 mantissa
        SUB    R1,R1,R3                  ; Get l2 mantissa
        SUBS   ip,R2,R3                  ; Compare exponents
        BLT    l1Lower                   ; Branch if lower
        ADD    R0,R0,R1,LSR ip           ; Add adjusted mantissas
        CMPS   R0,#(1:SHL:(5+16+Bf))     ; Test result for overflow
        MOVCS  R0,R0,LSR#1               ; Adjust if overflowed ...
        ADDCS  R2,R2,#1                  ; ... continued
        BIC    R0,R0,#2_11111            ; Clear exponent bits in result
        ORR    R0,R0,R2                  ; Combine with exponent
        LDMDB  fp,{fp,sp,pc}^            ; Return
l1Lower                                  ; Here if l1 exp lower
        SUB    ip,R3,R2                  ; Get abs diff in exponents
        ADD    R0,R1,R0,LSR ip           ; Add adjusted mantissas
        CMPS   R0,#(1:SHL:(5+16+Bf))     ; Test result for overflow
        MOVCS  R0,R0,LSR#1               ; Adjust if overflowed ...
        ADDCS  R3,R3,#1                  ; ... continued
        BIC    R0,R0,#2_11111            ; Clear exponent bits in result
        ORR    R0,R0,R3                  ; Combine with exponent
        LDMDB  fp,{fp,sp,pc}^            ; Return

;----------------------------------------------------------------------
;                                                      dpDecodeLength
;----------------------------------------------------------------------

; This function decodes the special floating point representation for
;    the length of a vector into a rounded fixed point representaion
;    in sub-pixel units.

; 'C' declaration:

; int dpDecodeLength(int l);

        ; At head of file : EXPORT |_dpDecodeLength|

dpDecodeLength externLabel

        MOV    ip,sp                     ; PCS stuff ...
        STMFD  sp!,{fp,ip,lr,pc}         ; ... continued ...
        SUB    fp,ip,#4                  ; ... continued
        
        AND    R1,R0,#2_11111            ; Get exponent
        MOV    R0,R0,LSR#5               ; Get mantissa
        SUBS   R1,R1,#16                 ; Shift left by exp-16 ...
        RSBMI  R1,R1,#0                  ; ... continued ...
        MOVPL  R0,R0,LSL R1              ; ... continued ...
        MOVMI  R0,R0,LSR R1              ; ... continued
        
        LDMDB  fp,{fp,sp,pc}^            ; Return

;----------------------------------------------------------------------
;                                                  dpMeasureComponent
;----------------------------------------------------------------------

; This function is declared in 'C' as:

; int dpMeasureComponent ( int l, int u )

; The idea is that l is the desired length of a vector, and u is the length
;   of one of the co-ordinates of the unit component of the vector. u is
;   a signed number with (16+Bf) significant bits set so that u*2^(-16-Bf)
;   is the value to be interpreted for u. Similarly, l has Bf fixed point
;   binary figures so l*2^(-Bf) is the value to be interpreted for l.

; The routine computes the value of l*u as a fixed point signed 32 bit
;   number with Bf binary figures, in other words, it computes the vector
;   component in sub-pixel units

        ; At head of file : EXPORT |_dpMeasureComponent|

dpMeasureComponent externLabel

        MOV    ip,sp                     ; PCS stuff ...
        STMFD  sp!,{fp,ip,lr,pc}         ; ... continued ...
        SUB    fp,ip,#4                  ; ... continued
        
        SMult  R0,R1,R2,R3,ip,lr         ; Get l*u in (R2,R3)
        MOV    R0,R2,LSR#(16+Bf)         ; Divide result by 2^(16+Bf) ...
        ORR    R0,R0,R3,LSL#(16-Bf)      ; ... continued
        
        LDMDB  fp,{fp,sp,pc}^            ; Return

;-------------------------------------------------------------------
;                                                      End-Of-File
;-------------------------------------------------------------------

        ; If in Helios Mode, send a directive ...
        
        [ heliosMode
        EndModule
        ]

        END
