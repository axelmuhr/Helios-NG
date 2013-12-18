;-------------------------------------------------------------------
;                                                lochdr/utility.ss
;-------------------------------------------------------------------

; This file contains utility defintions for all the assembler files
;   in the draw package.

;-------------------------------------------------------------------
;                                                 Register aliases
;-------------------------------------------------------------------

  [ :LNOT: heliosMode ; Only if not in helios mode
R0      RN     0
R1      RN     1
R2      RN     2
R3      RN     3
R4      RN     4
R5      RN     5
R6      RN     6
R7      RN     7
R8      RN     8
R9      RN     9
R10     RN    10
R11     RN    11
R12     RN    12
R13     RN    13
R14     RN    14
R15     RN    15

mp      RN     9
sl      RN    10
fp      RN    11
ip      RN    12
sp      RN    13
lr      RN    14
pc      RN    15

 ]

;-------------------------------------------------------------------
;                                 Exotic binary combination macros
;-------------------------------------------------------------------

; The following macros defined are all of the form MAC D,A,B where
;    D,A,B are registers, and expand out into one or two instructions
;    which perform a bitwise binary combination between registers
;    A and B, and deposits the result in register D. None of the
;    operations supplied are supported by ARM arithmetic operations.

        MACRO
$label  RBC    $D,$A,$B   ; Reverse Bit Clear   :   D = ~A . B
        BIC    $D,$B,$A
        MEND

        MACRO
$label  NOR    $D,$A,$B   ; Not OR              :   D = ~ (A + B)
        ORR    $D,$A,$B
        MVN    $D,$D
        MEND

        MACRO
$label  ENR    $D,$A,$B   ; Exclusive Not Or    :   D = ~ (A eor B)
        EOR    $D,$B,$A
        MVN    $D,$D
        MEND

        MACRO
$label  BST    $D,$A,$B   ; Bit SeT             :   D = A + ~B
        BIC    $D,$B,$A
        MVN    $D,$D
        MEND

        MACRO
$label  RBS    $D,$A,$B   ; Reverse Bit SeT     :   D = ~A + B
        BIC    $D,$A,$B
        MVN    $D,$D
        MEND

        MACRO
$label  NAN    $D,$A,$B   ; Not ANd             :   D = ~ (A . B)
        AND    $D,$A,$B
        MVN    $D,$D
        MEND

;-------------------------------------------------------------------
;                                                      End Of File
;-------------------------------------------------------------------

        END



