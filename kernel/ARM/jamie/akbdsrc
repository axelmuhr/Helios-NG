; > $.Source.PMF.Key

; ARTHUR keyboard code

; Authors       Tim Dobson, Jon Thackray
; Started       13-Oct-86

; ************************************************************
; ***    C h a n g e   L i s t  (better late than never!)  ***
; ************************************************************

; Date       Description
; ----       -----------
; 17-Feb-88  Added Sam's code to call the callback vector in RDCH/INKEY
;             idle loop
; 02-Mar-88  Initialise KeyVec to NewKeyStruct before the keyboard has told
;             us its ID, so Sam can call INKEY(-ve) with no keyboard
; 13-Apr-88  Fixed RDCH from RS423 if treating as keyboard, getting NUL+char
;             (didn't try to reenable RTS on 2nd char)
; 11-Jun-88  Put input redirection where it was needed really. SKS
; 12-Aug-88  Read mouse position from buffer forces inside bounding box
; 02-Sep-88  Buffered mouse coords stored in absolute terms, not relative to
;            origin at time of click. Made relative after reading out and
;            clipping to bounding box. Mouse event coords are still relative
;            to origin at time of click.

        GBLL    PollMouse
PollMouse SETL  {FALSE}

        GBLL    MouseBufferFix
MouseBufferFix SETL {TRUE}


        [ PollMouse
K1ack   *       K1sack
        |
K1ack   *       K1smak
        ]

        [ :LNOT: AssemblingArthur
        GBLL    redirectinkey
redirectinkey   SETL    {FALSE}
        ]

;
; The IOC registers
;
        ^       &04, R12
KARTTx  #       0
KARTRx  #       0

        ^       &20, R12
IRQStatusB #    4
IRQReqB    #    4
IRQMaskB   #    4

        ^       &70, R12
Timer3Low   #   4
Timer3High  #   4
Timer3Go    #   4
Timer3Latch #   4

; Register bits

KARTRxBit   *   &80
KARTTxBit   *   &40
KARTIRQBits *   KARTTxBit :OR: KARTRxBit

        MACRO
$lab    RXonTXoff  $reg, $cond
$lab
        LDR$cond.B $reg, IRQMaskB
        BIC$cond   $reg, $reg, #KARTTxBit
        ORR$cond   $reg, $reg, #KARTRxBit
        STR$cond.B $reg, IRQMaskB
        MEND

        MACRO
$lab    TXonRXoff  $reg, $cond
$lab
        LDR$cond.B $reg, IRQMaskB
        BIC$cond   $reg, $reg, #KARTRxBit
        ORR$cond   $reg, $reg, #KARTTxBit
        STR$cond.B $reg, IRQMaskB
        MEND

        MACRO
$lab    RXon       $reg, $cond
$lab
        LDR$cond.B $reg, IRQMaskB
        ORR$cond   $reg, $reg, #KARTRxBit
        STR$cond.B $reg, IRQMaskB
        MEND

        MACRO
$lab    TXon       $reg, $cond
$lab
        LDR$cond.B $reg, IRQMaskB
        ORR$cond   $reg, $reg, #KARTTxBit
        STR$cond.B $reg, IRQMaskB
        MEND

        GBLS    irqregs
irqregs SETS    """R4-R10, PC"""

; *****************************************************************************
;
;       Start of code

ArthurKeyDriver

; *****************************************************************************
;
;       Entry point for keyboard code - Initialisation
;

KeyInit ROUT
        MOV     R11, #KeyWorkSpace
        Push    R14

        [ AssemblingArthur
        TEQP PC,#I_bit :OR: SVC_mode
        |
        SWI     OS_IntOff
        SWI     OS_EnterOS
        ]

; Initialise the baud rate generator

        MOV     R12, #IOC
        MOV     R0, #1
        STRB    R0, Timer3Low
        MOV     R0, #0
        STRB    R0, Timer3High
        STRB    R0, Timer3Go

        STRB    R0, KARTTx              ; Write dummy byte

        MOV     R0, #&800
10
        SUBS    R0, R0, #1              ; copy Jon's loop
        BNE     %BT10

        LDRB    R0, KARTRx              ; Read dummy byte

        [ :LNOT: AssemblingArthur
        MOV     R0, #RdchV
        ADRL    R1, NewRdch
        SWI     OS_Claim

        MOV     R0, #MouseV
        ADRL    R1, ReadMouse
        SWI     OS_Claim
        ]

        [ {TRUE}
        MOV     R0, #MouseStepCMOS      ; setup mouse multipliers from CMOS
        BL      Read
        MOV     R0, R0, LSL #24         ; sign extend it
        MOVS    R0, R0, ASR #24
        MOVEQ   R0, #1                  ; if would be zero, set to 1
        |
        MOV     R0, #1                  ; before, we used 1 always
        ]
        STR     R0, MouseXMult
        STR     R0, MouseYMult

        [ {FALSE}                       ; mouse rectangle and position
        MOV     R0, #0                  ; initialised in VDU code now
        STR     R0, MouseBoundLCol      ; default bounding box is screen
        STR     R0, MouseBoundBRow

        MOV     R0, #640
        STR     R0, MouseX

        MOV     R0, #512
        STR     R0, MouseY

        MOV     R0, #&FF
        ORR     R0, R0, #&400           ; 1279
        STR     R0, MouseBoundRCol
        SUB     R0, R0, #&100           ; 1023
        STR     R0, MouseBoundTRow
        ]

        ADRL    R0, NewKeyStruct        ; point to new structure for now
        STR     R0, KeyVec

        MOV     R0, #0                  ; indicate modules not yet initialised
                                        ; and service not yet offered
        STRB    R0, ModulesOK

        MOV     R0, #&FF                ; indicate no previous keyboard id
        STRB    R0, LastKbId

        BL      ResetHW

        [ :LNOT: AssemblingArthur
        TEQP    PC, #I_bit
        MOVNV   R0, R0
        ]

        Pull    PC                      ; go back to user

; *****************************************************************************
;
;       KeyPostInit - Called after modules have initialised
;

KeyPostInit ROUT
        Push    R14
        MOV     R11, #KeyWorkSpace
        PHPSEI                          ; disable interrupts round this bit
        Push    R14                     ; save I_bit indication
        MOV     R0, #1                  ; indicate modules initialised
        STRB    R0, ModulesOK           ; but service not yet offered
        LDRB    R0, KbId                ; have we had keyboard id yet ?
        TEQ     R0, #&FF
        BLNE    IssueKeyboardService    ; if so, then issue keyboard service
        Pull    R14                     ; restore I_bit indication
        PLP                             ; set I_bit from this

        LDROSB  R1, LastBREAK           ; is it a soft reset ?
        TEQ     R1, #0
        Pull    PC, EQ                  ; if so, then exit

        MOV     R0, #OsbyteSetCountry
        LDROSB  R1, Country
        SWI     XOS_Byte
        Pull    PC

; *****************************************************************************

ResetHW ROUT
        Push    R14

        MOV     R0, #HRDRESET
        STRB    R0, ResetState

        ASSERT  HRDRESET = &FF
        STRB    R0, KeyRow              ; no key being received
        STRB    R0, Reply
        STRB    R0, KbId
        STRB    R0, KbIdHalf
        STRB    R0, CurrKey             ; no current key
        STRB    R0, OldKey
        STRB    R0, RequestLED

        MOV     R0, #K1rqid
        STRB    R0, RequestKbId

; Set up keyboard table

        MOV     R0, #0                  ; All keys up
        STRB    R0, MouseCount          ; start with an X coordinate
        STRB    R0, MouseButtons
        STRB    R0, SPDRec
        STRB    R0, RequestSPD
        STRB    R0, RequestMouse

        STR     R0, KeysDown            ; zero 160 bits = 5 words
        STR     R0, KeysDown +4
        STR     R0, KeysDown +8
        STR     R0, KeysDown +12
        STR     R0, KeysDown +16

        TXonRXoff       R0              ; enable Tx IRQ, disable Rx IRQ

        Pull    PC

; *****************************************************************************
;
;       UpdateLEDs - Update the LED(s) from the keyboard status byte
;
; in:   R11 -> keyboard workspace
;       R12 -> IOC
;
; out:  R0, R1 corrupted
;

UpdateLEDs ROUT
        LDRB    R0, KbId                ; get keyboard id
        TEQ     R0, #&FF                ; if not found yet
        MOVEQ   PC, R14                 ; then exit

        CMP     R0, #1                  ; if id >= 1 then new (C=1)
        LDROSB  R1, KeyBdStatus
        TST     R1, #KBStat_NoCapsLock  ; doesn't affect carry
        MOVNE   R0, #LEDOFF
        MOVEQ   R0, #LEDON
        BCC     %FT10                   ; [old keyboard]

        MOVNE   R0, #K1leds+0           ; caps lock is bit 0
        MOVEQ   R0, #K1leds+1
        TST     R1, #KBStat_NoNumLock
        ORREQ   R0, R0, #2              ; num lock is bit 1
        TST     R1, #KBStat_ScrollLock
        ORRNE   R0, R0, #4              ; scroll lock is bit 2
10
        STRB    R0, RequestLED
        TXon    R0
        MOV     PC, R14

; *****************************************************************************
;
;       IRQ routine
;
; in:   R2 = IOC request B flags
;       R0-R3, R11, R12 already saved, R14 irrelevant


        [ AssemblingArthur
IrqRx   ROUT
        Push    "R4-R10, R14"           ; stack regs if new MOS IRQ vector
        |
KeyIrq  ROUT
        TST     R2, #KARTTxBit          ; transmit empty ?
        BNE     IrqTx
        Push    "R4-R10"
        MOV     R12, #IOC               ; already set up in new IRQ scheme
        ]
        MOV     R11, #KeyWorkSpace

; Keyboard receive interrupt

; We now have to wait around for a period of 16 microseconds (or so they say)
; because the 'hardware engineers' can't design their hardware properly.

; This is doubly annoying because I have no idea what speed this processor is
; going at, so I don't know how many S-cycles this is, and there aren't enough
; hardware timers around to waste one doing this thankless task.

; In addition, because I am on the IRQ vector, the other IRQ users have
; probably wasted at least 16 microseconds anyway - at least the code seems
; to work perfectly well without this delay loop.

; Nevertheless, until the time Acorn can afford to employ some REAL
; hardware engineers, I shall do a delay of (about) 16*8 S-cycles,
; just to put their small minds at rest.

        MOV     R0, #16*8/5             ; delay for an unspecified period
IrqRxDelayLoop
        SUBS    R0, R0, #1              ; this breaks my heart,
        BNE     IrqRxDelayLoop          ; it really does !

        LDRB    R0, KARTRx              ; get data byte
        LDRB    R1, ResetState          ; and what we sent last

        CMP     R0, #K1rak2             ; is it a reset thingy ?
        BCS     ProcessReset            ; [yes, so check it]

        CMP     R1, #K1rak2             ; are we resetting anyway ?
        BCS     IrqBadRx                ; if so then bad reset

        AND     R2, R0, #&F0            ; get reason code

        LDRB    R1, KbId                ; get keyboard ID
        TEQ     R1, #&FF                ; is it valid yet ?
        BNE     ValidKbId               ; [yes, so we know what to expect]

        TEQ     R2, #IDTYPE             ; is it old keyboard id
        BEQ     IsOldKeyboard           ; [is old keyboard]

        BIC     R2, R2, #K1kbidmask     ; check for new keyboard id
        TEQ     R2, #K1kbid
        BNE     IrqBadRx                ; not a keyboard id, so reset

        AND     R1, R0, #K1kbidmask     ; get relevant bits
        STRB    R1, KbId
        ADRL    R0, NewKeyStruct
        B       AcknowledgeId

IsOldKeyboard
        AND     R0, R0, #&0F            ; get ID part
        LDRB    R1, KbIdHalf
        TST     R1, #&80
        STRNEB  R0, KbIdHalf            ; got half of keyboard id
        MOVNE   R0, #K1nack
        BNE     IrqRxAck

        ORR     R1, R1, R0, LSL #4      ; get full keyboard id
        STRB    R1, KbId
        MOV     R0, #&FF
        STRB    R0, KbIdHalf
        ADRL    R0, OldKeyStruct

AcknowledgeId
        LDRB    R8, LastKbId            ; get last keyboard id
        TEQ     R8, R1                  ; is it same
        STRB    R1, LastKbId            ; store new one anyway

        LDREQ   R0, KeyVec              ; if same, preserve old keyvec
        STRNE   R0, KeyVec              ; if different, set up ours

        LDRNEB  R8, ModulesOK           ; and if modules are initialised
        TEQNE   R8, #0
        BLNE    IssueKeyboardService    ; then offer service

        LDR     R8, [R0, #KVInit]
        ADD     R8, R8, R0
        BL      CallUserKeyCode         ; initialise user now we know who he is

        BL      UpdateLEDs              ; we can set the LEDs now
        B       IrqRxAckScan            ; start getting keys (and mouse if OK)

; R1 = keyboard ID - now dispatch code

ValidKbId
        TEQ     R1, #0
        BNE     NewKeyboardDispatch

OldKeyboardDispatch
        TST     R0, #MMOVED             ; is it mouse data ?
        BNE     ProcessOldMouseData
        TEQ     R2, #KEYDOWN            ; is it key down ?
        BEQ     ProcessOldKeyDown
        TEQ     R2, #KEYUP              ; is it key up ?
        BEQ     ProcessOldKeyUp
        TEQ     R2, #SPDDONE            ; is it SPD data ?
        BEQ     ProcessOldSPDData
        B       IrqBadRx                ; must be crap

NewKeyboardDispatch
        TST     R2, #K1notmousedata     ; is it mouse data ?
        BEQ     ProcessNewMouseData
        TEQ     R2, #K1kdda             ; is it key down ?
        BEQ     ProcessNewKeyDown
        TEQ     R2, #K1kuda             ; is it key up ?
        BEQ     ProcessNewKeyUp
        TEQ     R2, #K1pdat             ; is it SPD data ?
        BEQ     ProcessNewSPDData
        B       IrqBadRx                ; must be crap


; *****************************************************************************
;
;       ProcessReset - Process reset code from keyboard
;
; in:   R0 = code from keyboard
;       R1 = ResetState

ProcessReset ROUT

; Check sequencing

        TEQ     R1, R0                  ; is reply what was expected
        BNE     IrqBadRx                ; no, so reset

; Now continue the sequence

        TEQ     R1, #K1rak2             ; end of sequence ?
        MOVEQ   R1, #K1nack             ; then send a nack
        SUBNE   R1, R1, #1              ; else next thing to go
        STRB    R1, ResetState          ; store back

        TXonRXoff R0

        Pull    $irqregs

IrqBadRx

; Restart the reset sequence

        BL      ResetHW
        Pull    $irqregs

; *****************************************************************************

ProcessOldSPDData
ProcessNewSPDData
        LDRB    R1, SPDRec
        SUBS    R1, R1, #1
        STRCSB  R1, SPDRec                      ; dec number to go (if not 0)

        LDRCS   R1, SPDoutput
        MOVCS   R1, R1, LSR #4
        ORRCS   R1, R1, R0, LSL #28             ; put in new data
        STRCS   R1, SPDoutput

        B       IrqRxAckScan

; *****************************************************************************

ProcessOldMouseData                     ; R0 = 01xx xxxx
        TST     R0, #&20                ; get sign bit of data (bit 5)
        BICEQ   R0, R0, #&40            ; move to bit 6 (where it is on new)
ProcessNewMouseData
        LDRB    R1, MouseCount
        ADR     R2, MouseDelta
        STRB    R0, [R2, R1]            ; no need to clear top bit

        EORS    R1, R1, #1              ; move to other coordinate
        STRB    R1, MouseCount

        MOVNE   R0, #K1back
        BNE     IrqRxAck

        LDRB    R0, [R2]                ; get delta X
        MOV     R0, R0, LSL #25         ; shove up to top
        MOV     R0, R0, ASR #(25-16)    ; now at bottom of top 16 bits

        LDR     R3, MouseXMult
        CMP     R3, #0                  ; make sure multiplier is positive
        RSBMI   R3, R3, #0
        RSBMI   R0, R0, #0

        MULTIPLY R1, R3, R0             ; should work !

        LDR     R0, MouseX
        ADD     R1, R1, R0, LSL #16     ; add signed value in top 16 bits
        MOV     R0, R1, ASR #16         ; sign extend result to 32 bits

; now check for being inside bounding box

        LDR     R1, MouseBoundLCol
        CMP     R0, R1
        MOVLT   R0, R1                  ; if less then put on left boundary
        LDR     R1, MouseBoundRCol
        CMP     R1, R0
        MOVLT   R0, R1                  ; if more then put on right boundary
        STR     R0, MouseX

; now process Y offset

        LDRB    R0, [R2, #1]            ; get delta Y
        MOV     R0, R0, LSL #25         ; shove up to top
        MOV     R0, R0, ASR #(25-16)    ; now at bottom of top 16 bits

        LDR     R3, MouseYMult
        CMP     R3, #0                  ; make sure multiplier is positive
        RSBMI   R3, R3, #0
        RSBMI   R0, R0, #0

        MULTIPLY R1, R3, R0             ; should work !

        LDR     R0, MouseY
        ADD     R1, R1, R0, LSL #16     ; add signed value in top 16 bits
        MOV     R0, R1, ASR #16         ; sign extend result to 32 bits

; now check for being inside bounding box

        LDR     R1, MouseBoundBRow
        CMP     R0, R1
        MOVLT   R0, R1                  ; if less then put on left boundary
        LDR     R1, MouseBoundTRow
        CMP     R1, R0
        MOVLT   R0, R1                  ; if more then put on right boundary
        STR     R0, MouseY

        B       IrqRxAckScan

; *****************************************************************************

; in:   R1 = keyboard id

ProcessOldKeyDown
ProcessNewKeyDown ROUT
        LDRB    R2, KeyRow
        TEQ     R2, #&FF                ; have we had a row already ?
        STREQB  R0, KeyRow              ; no so store row
        MOVEQ   R0, #K1back
        BEQ     IrqRxAck                ; and acknowledge Rx

        EOR     R3, R0, R2              ; test if save movement type
        TST     R3, #&F0
        BNE     IrqBadRx                ; not same, so reset

        AND     R0, R0, #&0F            ; get new data
        AND     R2, R2, #&0F            ; and row data

        TEQ     R1, #0
        ORREQ   R2, R2, R0, LSL #4      ; old keyboard number
        ORRNE   R2, R0, R2, LSL #4      ; new key number

        MOV     R0, #&FF
        STRB    R0, KeyRow              ; reset 'had row' flag

        CMP     R2, #&A0
        ADRCC   R0, KeysDown
        MOVCC   R1, R2, LSR #5
        LDRCC   R1, [R0, R1, LSL #2]!   ; load appropriate word
        MOVCC   R3, #&80000000          ; index 0 is in top bit
        ORRCC   R1, R1, R3, ROR R2      ; set bit in bit array
        STRCC   R1, [R0]                ; store back

        MOV     R1, #1                  ; indicate key down
        BL      KeyboardEvent           ; generate key up/down event

        BL      CheckForShiftingKey
        BCC     %FT10                   ; [not shifting key]

        BL      CallSpecialReturnNChars
        B       IrqRxAckScan

10
        LDRB    R0, CurrKey
        TEQ     R0, #&FF                ; have we got a current key ?
        BEQ     IrqRxCurrentUpdate

        LDRB    R1, OldKey
        TEQ     R1, #&FF                ; have we got an old key ?
        BNE     IrqRxAckScan            ; ignore new - we've got 2 down already

        STRB    R0, OldKey              ; make current key old
IrqRxCurrentUpdate
        STRB    R2, CurrKey             ; update current
        MOV     R0, #2
        STRB    R0, Debouncing
        STRB    R0, AutoRepeatCount     ; generate char after 2 100Hz ticks

IrqRxAckScan

; Re-enable Tx interrupts and queue an acknowledge

        MOV     R0, #K1ack              ; either sack or smak as appropriate
IrqRxAck
        STRB    R0, Reply
        TXonRXoff       R0
        Pull    $irqregs                ; claimed irq, so grab link and PC

; *****************************************************************************

; in:   R1 = keyboard id

ProcessOldKeyUp
ProcessNewKeyUp ROUT
        LDRB    R2, KeyRow
        TEQ     R2, #&FF                ; have we had a row already ?
        STREQB  R0, KeyRow              ; no so store row
        MOVEQ   R0, #K1back
        BEQ     IrqRxAck                ; and acknowledge Rx

        EOR     R3, R0, R2              ; test if save movement type
        TST     R3, #&F0
        BNE     IrqBadRx                ; not same, so reset

        AND     R0, R0, #&0F            ; get new data
        AND     R2, R2, #&0F            ; and row data

        TEQ     R1, #0
        ORREQ   R2, R2, R0, LSL #4      ; old key number
        ORRNE   R2, R0, R2, LSL #4      ; new key number

        MOV     R0, #&FF
        STRB    R0, KeyRow              ; reset 'had row' flag

        CMP     R2, #&A0
        ADRCC   R0, KeysDown
        MOVCC   R1, R2, LSR #5
        LDRCC   R1, [R0, R1, LSL #2]!   ; load appropriate word
        MOVCC   R3, #&80000000          ; index 0 is in top bit
        BICCC   R1, R1, R3, ROR R2      ; clear bit in bit array
        STRCC   R1, [R0]                ; store back

        MOV     R1, #0                  ; indicate key up
        BL      KeyboardEvent           ; generate key up/down event

        BL      CheckForShiftingKey
        BCC     %FT10                   ; [not shifting key]

        BL      CallSpecialReturnNChars
        B       IrqRxAckScan

10
        LDRB    R0, OldKey
        TEQ     R0, R2                  ; is it old key going up ?
        BNE     NotOldKeyUp

; Old key going up

        LDRB    R0, CurrKey             ; current key is one to ignore in scan
        BL      ScanKeys

        STRPLB  R0, OldKey              ; found key, so current -> old
        BPL     IrqRxCurrentUpdate      ; and R2 -> current

        MOV     R0, #&FF                ; else mark old key invalid
        STRB    R0, OldKey
        B       IrqRxAckScan            ; and return

NotOldKeyUp
        LDRB    R1, CurrKey
        TEQ     R1, R2                  ; is it current key going up ?
        BNE     IrqRxAckScan            ; not interested if not

        BL      ScanKeys                ; R0 was OldKey
        BPL     IrqRxCurrentUpdate      ; was a key so make that current

        STRB    R2, CurrKey             ; mark current key up (R2 = -1)

        B       IrqRxAckScan

; *****************************************************************************
;
;       KeyboardEvent - Generate key up/down event
;
; in:   R1 = 0 for up, 1 for down
;       R2 = key index
;

KeyboardEvent ROUT
        LDRB    R3, KbId                ; tell event user the keyboard id
        MOV     R0, #Event_Keyboard
        B       OSEVEN

; *****************************************************************************
;
;       Scan keyboard for keys down, ignoring key number R0 and shifting keys
;
; in:   R0 = key number to ignore
;
; out:  N=0 => R2 = key number found
;       N=1 => no key found; R2 = -1
;       R0 preserved
;

ScanKeys ROUT
        Push    "R0, R14"
        ADR     R1, KeysDown
        MOV     R2, #4
10
        LDR     R3, [R1, R2, LSL #2]    ; get the word
        TEQ     R3, #0                  ; if any keys in this down, skip
        BNE     %FT20
15
        SUBS    R2, R2, #1              ; N=1 last time round
        BPL     %BT10
        Pull    "R0, PC"

20
        MOV     R2, R2, LSL #5          ; multiply by 32
        ADD     R2, R2, #32             ; and add 32
30
        TEQ     R3, #0                  ; no more bits ?
        MOVEQ   R2, R2, LSR #5          ; then reset R2 to word offset
        BEQ     %BT15                   ; and continue word loop
        SUB     R2, R2, #1              ; decrement key number
        MOVS    R3, R3, LSR #1          ; shift out bit
        BCC     %BT30

        CMP     R2, R0                  ; is it old key (C=1 if it is)
        BLNE    CheckForShiftingKeyR0R3 ; check that it's not shifting key
        BCS     %BT30                   ; C=1 => invalid, so loop

        TEQ     R2, #0                  ; N := 0
        Pull    "R0, PC"

; *****************************************************************************
;
;       CheckForShiftingKey - either going down or going up
;
; in:   R2 = key number
;
; out:  C=1 <=> is shifting key, so don't set current key etc
;       R0 -> key structure
;       R4 = shifting key index, or 0 if not shifting key
;       R3,R5 undefined
;       R1,R2,R6-R12 preserved
;

CheckForShiftingKeyR0R3 ROUT              ; version that saves R0, for ScanKeys
        Push    "R0,R3,R14"
        BL      CheckForShiftingKey
        Pull    "R0,R3,PC"

CheckForShiftingKey ROUT
        LDR     R0, KeyVec
        LDR     R3, [R0, #KVKeyTranSize] ; maximum internal key number +1
        CMP     R2, R3                  ; is it outside table ?
        LDRCC   R3, [R0, #KVKeyTran]    ; no, R3 := offset to keytran
        ADDCC   R3, R3, R0              ; R3 -> keytran
        LDRCC   R3, [R3, R2, LSL #2]    ; R3 = table word for this key
        CMNCC   R3, #1                  ; C=1 <=> outside table or is special
        MOVCC   PC, R14                 ; can't be shifting key

        LDR     R3, [R0, #KVShiftingList] ; R3 = offset to shifting key list
        LDRB    R4, [R3, R0]!           ; R4 = length of shifting key list
        TEQ     R4, #0
10
        LDRNEB  R5, [R3, R4]
        TEQNE   R5, R2
        SUBNES  R4, R4, #1
        BNE     %BT10

        CMP     R4, #1                  ; C=1 <=> shifting key
        MOV     PC, R14                 ; not one of the shifting keys

; *****************************************************************************
;
;       CallSpecialCode - Call code for a special key
;
; in:   R0 -> Key structure
;       R1 = 0 for up, 1 for down (shifting keys); 2 for first, 3 for repeat
;       R2 = key number
;

CallSpecialCode ROUT
        ADR     R6, NullCharList
        LDR     R3, [R0, #KVSpecialList] ; R3 = offset to special list
        LDRB    R4, [R3, R0]!           ; R4 = length of special list
        TEQ     R4, #0
        MOVEQ   PC, R14                 ; no special keys, so can't be one
10
        LDRB    R5, [R3, R4]
        TEQ     R5, R2
        BEQ     %FT20
        SUBS    R4, R4, #1
        BNE     %BT10
        MOV     PC, R14

20
        LDR     R3, [R0, #KVSpecialCodeTable] ; R3 = offset to special table
        ADD     R3, R3, R0              ; R3 -> special code table
        SUB     R5, R3, #4              ; 0th entry is for 1st special

        LDR     R8, [R5, R4, LSL #2]    ; R8 = offset to code for this special
        ADD     R8, R8, R3              ; R8 = address of code for this special
        ADR     R3, ReturnVector

; and drop thru to ...

CallUserKeyCode ROUT
        Push    R14
        LDROSB  R5, KeyBdStatus
        LDRB    R7, PendingAltType
        BL      %FT10
        STRB    R7, PendingAltType
        STROSB  R5, KeyBdStatus, R12
        Pull    R14
        MOV     R12, #IOC
        B       UpdateLEDs

10
        ADRL    R12, UserKeyWorkSpace
        MOV     PC, R8

NullCharList
        =       0
        ALIGN

ReturnVector
        B       MouseButtonChange
        B       DoBreakKey

; *****************************************************************************

IrqTx   ROUT
        [ AssemblingArthur
        Push    "R4-R10, R14"           ; stack regs if new MOS IRQ vector
        |
        Push    "R4-R10"
        MOV     R12, #IOC               ; already set up in new IRQ scheme
        ]
        MOV     R11, #KeyWorkSpace

; First see if we're in a reset sequence

        LDRB    R0, ResetState          ; are we in a reset ?
        TEQ     R0, #0
        BEQ     %FT05                   ; not in a reset

        CMP     R0, #K1rak2             ; are we sending the reset nack ?
        BCS     %FT25                   ; no, just send reset code
        MOV     R1, #0                  ; yes, zero the reset state
        STRB    R1, ResetState
        STRB    R0, KARTTx
        Pull    $irqregs                ; don't disable TX

; Now see if any outstanding requests

05
        LDRB    R0, RequestSPD          ; is there an SPD request ?
        TEQ     R0, #0
        BEQ     %FT10                   ; [no SPD request]

        MOV     R1, #K1prst             ; code to send keyboard
        MOV     R2, #0                  ; no further SPD request
        STRB    R2, RequestSPD
        MOV     R2, #8
        STRB    R2, SPDRec              ; nibbles still to be sent/received
        STRB    R1, KARTTx              ; send the byte
        Pull    $irqregs                ; exit without disabling Tx

10
        LDRB    R0, RequestKbId         ; is there a pending keyboard request ?
        TEQ     R0, #0
        MOVNE   R1, #0
        STRNEB  R1, RequestKbId         ; no further request
        STRNEB  R0, KARTTx              ; send the byte
        Pull    $irqregs, NE            ; exit without disabling Tx

        LDRB    R0, RequestMouse        ; is there a pending mouse request ?
        TEQ     R0, #0
        MOVNE   R1, #0
        STRNEB  R1, RequestMouse        ; no further request
        STRNEB  R0, KARTTx
        Pull    $irqregs, NE            ; exit without disabling Tx

        LDRB    R0, RequestLED          ; is there a pending LED request ?
        TEQ     R0, #&FF
        MOVNE   R1, #&FF
        STRNEB  R1, RequestLED
        STRNEB  R0, KARTTx
        Pull    $irqregs, NE            ; exit without disabling Tx

        LDRB    R0, SPDRec              ; are we converting some SPD data
        TEQ     R0, #0
        BEQ     %FT20                   ; branch if not

        LDR     R1, SPDinput
        AND     R2, R1, #&F             ; get nybble to be sent
        ORR     R2, R2, #K1rqpd
        MOV     R1, R1, LSR #4          ; shift out the nybble sent
        STR     R1, SPDinput
        STRB    R2, KARTTx
        B       %FT30                   ; disable Tx, so we don't send another
                                        ; nibble before we get the conversion
20
        LDRB    R0, Reply
        TEQ     R0, #&FF
        BEQ     %FT30                   ; no reply to send
25
        STRB    R0, KARTTx              ; send the reply
        MOV     R0, #&FF
        STRB    R0, Reply               ; nothing else to send
30
        RXonTXoff R0
        Pull    $irqregs

; *****************************************************************************
;
;       Centisecond tick routine
;
; out:  R12 corrupted

CentiSecondTick ROUT
        Push    "R11, R14"
        MOV     R11, #KeyWorkSpace
        MOV     R12, #IOC

        [ PollMouse
        MOV     R0, #K1rqmp
        STRB    R0, RequestMouse
        TXon    R0
        ]

        LDR     R0, InkeyCounter
        SUBS    R0, R0, #1              ; decrement
        STRCS   R0, InkeyCounter        ; store back unless was frozen at 0

        LDRB    R2, CurrKey
        TEQ     R2, #&FF
        Pull    "R11,PC", EQ            ; no current key, so no auto-repeat

        BL      UpdateLEDs              ; update LEDs from keyboard status

        LDRB    R0, AutoRepeatCount
        SUBS    R0, R0, #1              ; decrement count (if frozen then C:=0)
        STRHIB  R0, AutoRepeatCount ; store back if now non-zero and not frozen
        Pull    "R11,PC", NE            ; return if non-zero or was frozen

        LDRB    R1, Debouncing          ; get debounce flag
        TEQ     R1, #0

        STRNEB  R0, Debouncing          ; if not zero, zero it
        LDROSB  R0, KeyRepDelay, NE     ; and load delay
        MOVNE   R1, #2                  ; indicate first time

        LDROSB  R0, KeyRepRate, EQ      ; if zero, then load repeat
        MOVEQ   R1, #3                  ; indicate subsequent time

        STRB    R0, AutoRepeatCount     ; in any case, store back

        Push    "R4-R10"                ; save registers
        BL      GenerateChar            ; R2 = key number
        Pull    "R4-R11,PC"

; *****************************************************************************
;
;       MouseButtonChange - Called by keyboard handler when mouse button change
;
; in:   R0 = state of buttons (bit0=R, bit1=C, bit2=L)
;       R11 -> KeyWorkSpace
;

MouseButtonChange ROUT
        Push    "R0-R5, R12, R14"

        VDWS    WsPtr
        STRB    R0, MouseButtons        ; save it for ReadMouse calls
        MOV     R3, R0

        LDR     R1, MouseX
        LDR     R0, [WsPtr, #OrgX]
        SUB     R1, R1, R0              ; mouse X

        LDR     R2, MouseY
        LDR     R0, [WsPtr, #OrgY]
        SUB     R2, R2, R0              ; mouse Y

        [ AssemblingArthur :LOR: Module
        MOV     R4, #0
        LDR     R4, [R4, #MetroGnome]   ; use monotonic variable now
        |
        BYTEWS  WsPtr
        LDR     R4, RealTime            ; doesn't exist in my world
        ]

        MOV     R0, #Event_Mouse
        BL      OSEVEN
        MOV     WsPtr, #IOC
        
        [ MouseBufferFix
        LDR     R0, MouseX
        |
        MOV     R5, R2                  ; save mouse Y
        MOV     R0, R1
        ]
        BL      MouseInsert             ; send mouse X low
        BCS     %FT10                   ; buffer full, so don't send rest

        MOV     R0, R0, LSR #8          ; send mouse X high
        BL      MouseInsert

        [ MouseBufferFix
        LDR     R0, MouseY
        |
        MOV     R0, R5
        ]
        BL      MouseInsert             ; send mouse Y low

        MOV     R0, R0, LSR #8          ; send mouse Y high
        BL      MouseInsert

        MOV     R0, R3
        BL      MouseInsert             ; send buttons

        MOV     R0, R4
        BL      MouseInsert             ; send realtime(0)

        MOV     R0, R4, LSR #8
        BL      MouseInsert             ; send realtime(1)

        MOV     R0, R4, LSR #16
        BL      MouseInsert             ; send realtime(2)

        MOV     R0, R4, LSR #24
        BL      MouseInsert             ; send realtime(3)
10
        Pull    "R0-R5, R12, PC"

MouseInsert
        Push    "R10,R12,R14"
        MOV     R10, #INSV
        MOV     R1, #Buff_Mouse
        B       GoVec

; *****************************************************************************
;
;       DoBreakKey - Called by key handler when break key up or down
;
; in:   R0 -> key structure
;       R1 = 0 for up, 1 for down (shouldn't be 2 or 3)
;       R2 = ARM internal key number
;       R3 = address of ReturnVector
;       R4 = special number 1..n
;       R5 = keyboard status
;
; out:  R6 -> list of chars to return
;

DoBreakKey ROUT
        TST     R5, #KBStat_ShiftEngaged        ; shift down ?
        MOVEQ   R3, #31
        MOVNE   R3, #29

        TST     R5, #KBStat_CtrlEngaged         ; ctrl down ?
        BICNE   R3, R3, #4

        LDROSB  R2, BREAKvector
        MOVS    R2, R2, LSL R3                  ; put relevant bits in C,N

        MOVCS   PC, R14                         ; 2 or 3 => ignore
        BPL     %FT10                           ; 0 => do a reset

        TEQ     R1, #1                          ; is it key down ?
        ADREQ   R6, EscList                     ; yes, return ESCAPE
        MOV     PC, R14                         ; else just return

10
        [ AssemblingArthur
        TEQ     R1, #0                          ; is it key up ?
        MOVNE   PC, R14                         ; no, then return

; offer the pre-reset service

        TEQP    PC, #ARM_CC_Mask                ; set FI bits, SVC mode

        MOV     R1, #Service_PreReset
        IssueService

        TEQP    PC, #ARM_CC_Mask                ; set FI bits, SVC mode
                                                ; just in case!
        B       CONT
        |
        MOV     PC, R14                         ; do nowt
        ]

EscList
        =       1, &1B
        ALIGN

; *****************************************************************************
;
;       Generate a character in keyboard buffer, if necessary
;
; in:   R1 = 2 if first press; 3 if repetition
;       R2 = key number
;       R12 -> IOC
;

GenerateChar ROUT
        Push    R14
        LDR     R0, KeyVec
        LDR     R3, [R0, #KVKeyTranSize]        ; get size
        CMP     R2, R3                          ; if outside table
        BCS     %FT04                           ; then assume special

        LDR     R3, [R0, #KVKeyTran]            ; R3 = offset to KeyTran
        ADD     R3, R3, R0                      ; R3 -> KeyTran

; now modify for CTRL and SHIFT

        LDROSB  R5, KeyBdStatus

        TST     R5, #KBStat_CtrlEngaged
        ADDNE   R3, R3, #2

        TST     R5, #KBStat_ShiftEngaged
        ADDNE   R3, R3, #1

        LDRB    R3, [R3, R2, LSL #2]            ; get real code

; apply CAPS lock modifying

        BIC     R6, R3, #&20                    ; get upper-case code
        CMP     R6, #"A"
        RSBCSS  R6, R6, #"Z"                    ; is it alphabetic ?
        BCC     %FT20

        TST     R5, #KBStat_ShiftEnable         ; if SHCAPS
        EORNE   R3, R3, #&20                    ; then swap case

        TSTEQ   R5, #KBStat_NoCapsLock          ; else if CAPS
        BICEQ   R3, R3, #&20                    ; force upper case
20
        TEQ     R3, #&FF                        ; is it a special ?
        BEQ     %FT04                           ; [yes, so skip]

        LDROSB  R6, ESCch                       ; if ESCAPE character
        TEQ     R3, R6
        LDROSB  R6, ESCaction, EQ               ; and normal ESCAPE action
        TEQEQ   R6, #0
        LDROSB  R6, ESCBREAK, EQ                ; and ESCAPE not disabled
        TSTEQ   R6, #1
        BICEQ   R5, R5, #KBStat_PendingAlt      ; then cancel pending alt
        STROSB  R5, KeyBdStatus, R6, EQ         ; and store back

        TST     R5, #KBStat_PendingAlt          ; is there a pending Alt ?
        BNE     ProcessPendingAlt

        TEQ     R3, #0                          ; is it NUL ?
        BNE     %FT10                           ; no, so skip

        ADR     R6, NULNULList                  ; then insert NUL NUL
        B       ReturnNChars

CallSpecialReturnNChars
        Push    R14
04
        BL      CallSpecialCode

ReturnNChars
        LDRB    R3, [R6], #1                    ; R1 = count of characters

; TMD 25-Sep-89: Fix bug which resulted in Break key (acting as Escape) not
; working if buffer was full - only count spaces if more than 1 character going
; into buffer

 [ {TRUE}
        CMP     R3, #1
        Pull    PC, CC                          ; no chars, so exit now
        BEQ     %FT05                           ; only 1 char, don't count
 |
        TEQ     R3, #0                          ; no chars?
        Pull    PC, EQ                          ; then exit now
 ]

        MOV     R1, #Buff_Key
        CMP     PC, #0                          ; C=1, V=0 so count spaces
        BL      CnpEntry
        ORR     R1, R1, R2, LSL #8              ; R1 = number of spaces
        CMP     R3, R1                          ; are there enough ?
        Pull    PC, HI                          ; no, then forget them

05
        LDRB    R2, [R6], #1                    ; send chars
        BL      InsertKeyZCOE                   ; one at a time
        SUBS    R3, R3, #1
        BNE     %BT05
        Pull    PC

10
        Pull    R14                             ; restore stacked R14
        MOV     R2, R3

; and drop thru to ...

; *****************************************************************************
;
;       InsertKeyZCOE - Insert key zeroing count on escape
;
; in:   R2 = character
;

InsertKeyZCOE
        LDROSB  R0, KeyBdDisable                ; disable insertion of codes ?
        TEQ     R0, #0
        MOVNE   PC, R14                         ; [disabled]
        LDROSB  R0, ESCch                       ; escape character
        TEQ     R0, R2                          ; if is esc char
        LDROSB  R0, ESCaction, EQ
        TEQEQ   R0, #0                          ; and FX229,0

        STREQB  R0, AutoRepeatCount             ; then zero repeat counter

; and drop thru to ...

; *****************************************************************************
;
;       RDCHS - Insert character into keyboard buffer
;
; in:   R2 = character
;

RDCHS   ROUT
        MOV     R1, #Buff_Key                   ; keyboard buffer id

; Insert character R2 into buffer R1, checking for escape character

        B       DoInsertESC

; *****************************************************************************

NULNULList                                      ; list for returning NUL NUL
        =       2, 0, 0
        ALIGN

; *****************************************************************************

ProcessPendingAlt
        ADR     R6, NullCharList
        LDR     R8, [R0, #KVPendingAltCode]
        ADD     R8, R8, R0
        BL      CallUserKeyCode
        B       ReturnNChars

; *****************************************************************************
;
;       Read character entry point
;
; in:   -
; out:  R0 = character
;       C=1 => ESCAPE
;       R1-R13 preserved
;

NewRdch
        Push    "R1-R4,R11"
        MOV     R11, #KeyWorkSpace
        MOV     R4, #0                  ; indicate RDCH not INKEY
        BL      RdchInkey
        Pull    "R1-R4,R11,PC"

; *****************************************************************************
;
;       RDCH/INKEY
;
; in:   R4 = 0  => RDCH
;       R4 <> 0 => INKEY
;
; out:  V=1 => error (and possibly R0 -> error block if you're lucky!)
;

RdchInkey ROUT

        Push    R14

; Enable interrupts so that keyboard can work properly

        TEQP    pc, #SVC_mode

 [ redirectinkey
        MOV     r1, #0
        LDRB    r1, [r1, #RedirectInHandle]
        TEQ     r1, #0
        BEQ     %FT10

; Tutu doesn't believe that an escape condition should break redirection
; - similar to exec if you turn off escape ack side-effects

        SWI     XOS_BGet                ; get byte from redirection handle
        BVS     RedirectBadExit
        BCC     ReturnChar              ; (C=0)

; EOF, so close redirect file and read from exec file or keyboard

; stop redirecting, BEFORE closing file, in case the CLOSE gets an error

        MOV     r0, #0
        STRB    r0, [r0, #RedirectInHandle] ; Convenient, huh ?
        SWI     XOS_Find                ; close file (R0=0, R1=handle)
        Pull    pc, VS

10
 ]

; First check for EXEC file

        LDROSB  R1, ExecFileH           ; read EXEC handle
        TEQ     R1, #0
        BEQ     RdchLoop                ; no exec file

        SWI     XOS_BGet                ; get byte from exec handle
        BVS     ExecBadExit
        BCC     ReturnChar              ; (C=0)

; EOF, so close exec file and read from keyboard

; stop EXECing, BEFORE closing file, in case the CLOSE gets an error

        STROSB  R0, ExecFileH, R0       ; (STROSB sets temp reg to 0)
        SWI     XOS_Find                ; close file (R0=0, R1=handle)
        Pull    pc, VS

RdchLoop
        MOV     R0, #0
        LDRB    R0, [R0, #ESC_Status]
        MOVS    R0, R0, LSL #(32-6)     ; shift relevant bit into carry
        MOVCS   R0, #27                 ; escape detected
        BCS     ReturnChar

        LDROSB  R1, InputStream         ; 0 => keyboard, 1 => RS423
        BL      RDCHG
        BCC     ReturnChar

; Sam's hack to call the callback vector if appropriate

        [ AssemblingArthur
        MOV     R0, #0
        LDRB    R14, [R0, #CallBack_Flag]
        TST     R14, #CBack_VectorReq
        BLNE    process_callback_chain
        ]

; here endeth the hack

        TEQ     R4, #0
        BEQ     RdchLoop                ; RDCH not INKEY, so loop

        LDR     R0, InkeyCounter
        TEQ     R0, #0                  ; or count not expired
        BNE     RdchLoop                ; then loop

        MOV     R0, #&FF                ; indicate timeout
        SEC                             ; and set carry
ReturnChar
        CLRPSR  V_bit, R14
        Pull    PC

ExecBadExit                             ; got an error from BGET
        Push    R0                      ; save error pointer
        STROSB  R0, ExecFileH, R0       ; (STROSB sets temp reg to 0)
        SWI     XOS_Find                ; close file (R0=0, R1=handle)
        Pull    "R1, R14"               ; pull registers
        MOVVC   R0, R1                  ; if closed OK, then restore old error
        ORRS    PC, R14, #V_bit         ; still indicate error

 [ redirectinkey
RedirectBadExit                         ; got an error from BGET
        BL      RemoveOscliCharJobs     ; preserves r0
        Pull    "R14"                   ; pull register
        ORRS    PC, R14, #V_bit         ; still indicate error
 ]

; *****************************************************************************
;
;       RDCHG - Fetch character from input buffer
;       Expand soft keys as necessary
;       Pass cursor control keys to VDU driver
;       Return carry set if character not available
;
; in:   R1 = input buffer id (0 => keyboard, 1 => RS423)

RDCHG   ROUT
        Push    R14

; insert check here for ECONET interception of RDCH

RDCHNM
        LDROSB  R0, SoftKeyLen          ; are we expanding a soft key
        TEQ     R0, #0
        BEQ     RDCHG1                  ; not expanding

        LDROSB  R2, RS423mode
        TST     R1, R2                  ; if RS423 and 8 bit data
        BNE     RDCHG1                  ; ignore soft keys

        LDR     R2, SoftKeyPtr
        LDRB    R2, [R2, -R0]           ; get character out of buffer

        SUB     R0, R0, #1              ; decrement character count
        STROSB  R0, SoftKeyLen, R3      ; store back

        MOV     R0, R2                  ; put character in R0
        CLC                             ; and exit with carry clear
        Pull    PC

RDCHG1
        BL      KeyREMOVECheckRS423     ; remove character, if none, exit CS

        LDROSB  R2, RS423mode           ; 0 => treat RS423 as keyboard
        TST     R1, R2                  ; NZ => let RS423 deliver 8-bit codes
        BNE     RDCHGCLC

        TEQ     R0, #0                  ; is it NUL ?
        BNE     %FT10

        BL      KeyREMOVECheckRS423     ; get another char, if none then
                                        ; spurious, so ignore

        TEQ     R0, #0                  ; is it NUL NUL ?
        BNE     RDCHGCLC                ; no, then return this character

        LDRB    R2, [R0, #OsbyteVars + :INDEX: IPbufferCh]!
                                        ; R0 was 0, so now -> 1st of 8 keybases
        ADD     R3, R0, #8
05
        TEQ     R2, #2                  ; is this key base = 2 ?
        MOVEQ   R0, #0                  ; if so then return NUL NUL
        BEQ     ReturnNULR0
        LDRB    R2, [R0, #1]!           ; load next key base
        TEQ     R0, R3                  ; if not tried all of them
        BNE     %BT05                   ; then loop
        MOV     R0, #0                  ; no special key bases,
                                        ; so just return NUL
10
        TST     R0, #&80
        BEQ     RDCHGCLC

; now check for cursor key movement

        AND     R3, R0, #&0F            ; save bottom nybble
        CMP     R3, #&0B                ; is it a cursor key ?
        BCC     NotCursorKey

        TST     R0, #&40                ; don't let Cx-Fx be cursor keys
        BNE     NotCursorKey

        LDROSB  R2, CurEdit             ; FX 4 state
        CMP     R2, #1
        ADDLS   R0, R3, #&87-&0B        ; 0 or 1 => force in range &87-&8B
        BCC     ItsCursorEdit           ; 0 => cursor edit
        BEQ     RDCHGCLC                ; 1 => return these codes

NotCursorKey
        MOV     R0, R0, LSR #4
        EOR     R0, R0, #&0C            ; 4..7, 0..3
        LDRB    R2, [R0, #OsbyteVars+IPbufferCh-OSBYTEFirstVar]
                                        ; get key variable
        CMP     R2, #1                  ; is it 0 (ignore) or 1 (softkey)
        BCC     RDCHG1                  ; get another char if 0

        BEQ     ExpandSoftKey           ; expand soft key if 1

        TEQ     R2, #2                  ; is it special Compact option ?
        EOREQ   R0, R0, #&0C            ; undo that mangling !
        ORREQ   R0, R3, R0, LSL #4      ; if so, then return NUL <code>
        BEQ     ReturnNULR0

        ADD     R0, R2, R3              ; add offset to base
        AND     R0, R0, #&FF            ; make it wrap

RDCHGCLC
        CLC
        Pull    PC


ItsCursorEdit
        LDROSB  R2, WrchDest
        TST     R2, #2                  ; if wrch not to VDU
        BNE     RDCHG1                  ; then ignore character

        Push    "R1,R4-R12"
        [ AssemblingArthur
        VDWS    WsPtr
        BL      DoCursorEdit
        |
        BL      DCE10
        ]
        Pull    "R1,R4-R12"

        BCS     RDCHG1                  ; no character yet, so loop
        Pull    PC                      ; NB carry clear - no ESCAPE !

        [ :LNOT: AssemblingArthur
DCE10
        MOV     R1, #VduDriver
        ADD     PC, R1, #CursorEdit
        ]

; *****************************************************************************
;
;       ReturnNULR0 - Return NUL followed by R0 from RDCH
;

ReturnNULR0 ROUT
        ADR     R2, SoftKeyExpand       ; store code in SoftKeyExpand +0
        STRB    R0, [R2], #1            ; and set ptr to SoftKeyExpand +1
        STR     R2, SoftKeyPtr
        MOV     R2, #1                  ; set key length to 1
        STROSB  R2, SoftKeyLen, R0      ; (sets R0 to 0!)
        B       RDCHGCLC                ; return NUL as first character

; *****************************************************************************

KeyREMOVECheckRS423 ROUT
        Push    R14
        BL      KeyREMOVE
        Pull    "R14, PC", CS           ; pull stacked R14 if CS

; new code inserted here 14/8/87 to try to reenable RTS for RS423 input

        TEQ     R1, #1                  ; RS423 input ?
        Pull    PC, NE                  ; no, then exit
        Push    "R0,R1,R12"             ; preserve char + buffer id + R12
        BYTEWS  R12
        BL      RSETX                   ; reenable RTS if now enough spaces
        Pull    "R0,R1,R12, PC"         ; restore char, buffer id and exit

; *****************************************************************************

KeyREMOVE
        Push    "R10,R12,R14"
        CLRV                                    ; do remove not examine
        MOV     R10, #REMV
        B       GoVec


; expand a soft key as a variable (R3 = key number)

ExpandSoftKey ROUT
        Push    "R1,R4"
        BL      SetupKeyName
        ADR     R1, SoftKeyExpand
        MOV     R2, #255                        ; max length of string
        MOV     R3, #0                          ; no name pointer
        MOV     R4, #VarType_Expanded
        SWI     XOS_ReadVarVal

        Pull    "R1,R4", VS
        BVS     RDCHG1                          ; no string or bad

        STROSB  R2, SoftKeyLen, R0              ; store length (may be zero)
        ADD     R1, R1, R2                      ; R1 -> last char+1
        STR     R1, SoftKeyPtr
        Pull    "R1,R4"
        B       RDCHNM                          ; try to expand it

KeyName
        =       keyprefix,0
        ALIGN

; *****************************************************************************
;
;       SetupKeyName - Set up the name <keyprefix><n><0> in SoftKeyName
;
; in:   R11 -> KeyWS
;       R3 = key number
;
; out:  R0 -> SoftKeyName, which contains <keyprefix><n><0>
;       R2-R4 corrupted
;

SetupKeyName ROUT
        ADR     R2, KeyName
        ADR     R0, SoftKeyName
10
        LDRB    R4, [R2], #1                    ; copy keyprefix in
        TEQ     R4, #0
        STRNEB  R4, [R0], #1
        BNE     %BT10                           ; now put digits at R0

        ORR     R3, R3, #"0"
        CMP     R3, #"9"+1

        MOVCS   R2, #"1"                        ; if >=10 then put in "1"
        STRCSB  R2, [R0], #1
        SUBCS   R3, R3, #10                     ; and subtract 10

        STRB    R3, [R0], #1
        STRB    R4, [R0]                        ; (R4=0) terminate

        ADR     R0, SoftKeyName
        MOV     PC, R14

; *****************************************************************************
;
;       DoInkeyOp - Perform INKEY

DoInkeyOp
        TST     R2, #&80                ; INKEY(+ve) ?
        BNE     NewInkeyNeg

NewInkeyPos
        Push    R4
        MOV     R11, #KeyWorkSpace
        AND     R1, R1, #&FF            ; no funny business
        AND     R2, R2, #&FF            ; ditto
        ORR     R1, R1, R2, LSL #8      ; get combined count
        STR     R1, InkeyCounter

        MOV     R4, #1
        BL      RdchInkey

        MOV     R1, R0                  ; make X the character
        MOVCC   R2, #0                  ; Y := 0 if normal exit
        MOVCS   R2, R0                  ; Y := &1B or &FF for ESC or timeout

        Pull    "R4,PC"                 ; return preserving V and R0

NewInkeyNeg
        EOR     R1, R1, #&7F            ; invert bits for scan call
        BL      BBCScanKeys
        Pull    PC

; *****************************************************************************
;
;       BBCScanKeys - Test individual key or scan for key depression
;
; in:   R1 = 0..&7F => scan keyboard from BBC internal key R1
; out:  C=0 => R1 = BBC internal key found
;       C=1 => R1 = &FF (no key found)
;
; in:   R1 = &80..&FF => test if BBC internal key (R1 EOR &80) is down
; out:  C=0, R1=R2=&00 => key is up
;       C=1, R1=R2=&FF => key is down
;

BBCScanKeys ROUT
        Push    R11
        MOV     R11, #KeyWorkSpace
        AND     R1, R1, #&FF            ; trap wallies

        LDR     R0, KeyVec
        LDR     R2, [R0, #KVInkeyTran]
        ADD     R0, R0, R2              ; R0 -> InkeyTran or InkeyTran2

        TST     R1, #&80                ; >=&80 => test single key
                                        ; < &80 => scan for key
        BEQ     DoBBCScan               ; [is scanning not testing]

        ADD     R0, R0, #4 * &FF        ; R0 -> InkeyTran+4*&FF
        LDR     R0, [R0, -R1, LSL #2]   ; get word of indexes into KeysDown
        MOV     R2, #&FF000000
02
        CMP     R0, #-1                 ; is it all FF's
        MOVEQ   R1, #0                  ; if so then none of keys down
        BEQ     %FT04

        AND     R1, R0, #&FF            ; just get bottom byte
        ADR     R3, KeysDown            ; look up in KeysDown
        MOV     R1, R1, LSR #5
        LDR     R3, [R3, R1, LSL #2]    ; get word of 32 bits
        AND     R1, R0, #31
        MOV     R3, R3, LSL R1          ; put relevant bit into top bit
        MOVS    R1, R3, LSR #31         ; R1 = 0 if up, 1 if down
        ORREQ   R0, R2, R0, LSR #8      ; shift down, putting FF in top byte
        BEQ     %BT02
04
        CMP     R1, #1                  ; C=1 <=> at least one of keys down
        MOVCC   R1, #0
        MOVCS   R1, #&FF
        MOV     R2, R1
        Pull    R11
        MOV     PC, R14

DoBBCScan
        Push    "R4, R5"
        ADD     R0, R0, #4 * &7F        ; R0 -> InkeyTran+4*&7F
        MOV     R4, #&FF000000
10
        LDR     R3, [R0, -R1, LSL #2]   ; get word of indexes into KeysDown
15
        CMP     R3, #-1                 ; all FFs ?
        BEQ     %FT18                   ; then not one of these keys

        AND     R5, R3, #&FF
        ADR     R2, KeysDown
        MOV     R5, R5, LSR #5
        LDR     R2, [R2, R5, LSL #2]    ; get word of bits
        AND     R5, R3, #31
        MOV     R2, R2, LSL R5          ; put relevant bit into top bit
        MOVS    R5, R2, LSR #31         ; R5 = 0 for up, 1 for down
        BNE     %FT20                   ; [down, so stop]
        ORR     R3, R4, R3, LSR #8      ; up -> shift down putting FF in top
        B       %BT15
18
        ADD     R1, R1, #1              ; go to next key
        TEQ     R1, #&80                ; if not run out of keys
        BNE     %BT10                   ; then loop
        MOV     R1, #&FF                ; indicate no key
20
        CMP     R1, #&FF                ; C=0 <=> found key
        Pull    "R4,R5,R11"
        MOV     PC, R14

; *****************************************************************************
;
;       Write keys down information
;
; in:   R1 = Current key (in BBC internal key format)
;       R2 = Old key     (------------""------------)
;
; out:  R1, R2 preserved
;

WriteKeysDown ROUT
        Push    R14
        MOV     R11, #KeyWorkSpace
        MOV     R0, R1
        BL      ConvertInternalKey
        STRB    R0, CurrKey
        MOV     R0, R2
        BL      ConvertInternalKey
        STRB    R0, OldKey
        Pull    PC

ConvertInternalKey
        TST     R0, #&80                ; if not in range &80..&FF
        MOVEQ   R0, #&FF                ; return value &FF (key not valid)
        MOVEQ   PC, R14

        EOR     R0, R0, #&7F            ; else convert to inkey value
        Push    R4
        LDR     R3, KeyVec
        LDR     R4, [R3, #KVInkeyTran]
        ADD     R3, R3, R4              ; R3 -> InkeyTran or InkeyTran2
        Pull    R4

        SUB     R3, R3, #&80*4          ; R3 -> InkeyTran-4*&80

        LDRB    R0, [R3, R0, LSL #2]    ; convert to ARM internal key
                                        ; (just get 1st key for this key)
        MOV     PC, R14

; *****************************************************************************
;
;       Read mouse position
;

ReadMouse ROUT
        Push    "R4-R6,R10-R12"
        MOV     R11, #KeyWorkSpace

        MOV     R1, #Buff_Mouse
        BL      KeyREMOVE
        BCS     %FT10                   ; MouseAhead buffer empty

        MOV     R4, R2, LSL #16         ; Mouse X Low
        BL      KeyREMOVE
        ORR     R4, R4, R2, LSL #24     ; R4 := Mouse X << 16

        BL      KeyREMOVE
        MOV     R5, R2, LSL #16         ; Mouse Y Low
        BL      KeyREMOVE
        ORR     R5, R5, R2, LSL #24     ; R5 := Mouse Y << 16

        BL      KeyREMOVE
        MOV     R6, R2                  ; Button state

        BL      KeyREMOVE               ; get realtime
        MOV     R3, R2
        BL      KeyREMOVE
        ORR     R3, R3, R2, LSL #8
        BL      KeyREMOVE
        ORR     R3, R3, R2, LSL #16
        BL      KeyREMOVE
        ORR     R3, R3, R2, LSL #24

        MOV     R0, R4, ASR #16         ; sign extend mouse coords
        MOV     R1, R5, ASR #16
        MOV     R2, R6

; code inserted here 12-Aug-88 to force position read from buffer to be inside
; CURRENT bounding box; this removes the need to flush buffer when changing
; the bounding box.

        ADR     R4, MouseBounds
        LDMIA   R4, {R4-R6,R10}         ; R4=LCol; R5=BRow; R6=RCol; R10=TRow;
        CMP     R0, R4
        MOVLT   R0, R4
        CMP     R0, R6
        MOVGT   R0, R6
        CMP     R1, R5
        MOVLT   R1, R5
        CMP     R1, R10
        MOVGT   R1, R10

        [ MouseBufferFix
        B       %FT20                   ; correct for origin after clipping
        |
        Pull    "R4-R6,R10-R12,PC"
        ]

10
        LDRB    R2, MouseButtons

        [ AssemblingArthur :LOR: Module
        MOV     R3, #0
        LDR     R3, [R3, #MetroGnome]           ; use monotonic variable now
        |
        BYTEWS  WsPtr
        LDR     R3, RealTime                    ; doesn't exist in my world
        ]

        LDR     R0, MouseX
        LDR     R1, MouseY
20
        VDWS    WsPtr

        LDR     R4, [WsPtr, #OrgX]
        SUB     R0, R0, R4

        LDR     R4, [WsPtr, #OrgY]
        SUB     R1, R1, R4

        Pull    "R4-R6,R10-R12,PC"

; *****************************************************************************
;
;       InstallKeyHandler - Install user key handler
;
; in:   R0 = new key handler
;        0 => just read old key handler
;        1 => just read keyboard id
;
; out:  R0 = old key handler, or
;       R0 = keyboard id if R0 was 1 on entry (&FF => no keyboard id yet)
;

InstallKeyHandler ROUT
        MOV     R11, PC
        TST     R11, #I_bit
        TEQEQP  R11, #I_bit             ; disable IRQs

        MOV     R11, #KeyWorkSpace
        TEQ     R0, #1                  ; asking for keyboard id ?
        LDREQB  R0, KbId                ; then load it
        ExitSWIHandler EQ               ; and exit

        LDR     R10, KeyVec             ; R10 -> old key handler
        TEQ     R0, #0                  ; if not just reading it
        STRNE   R0, KeyVec              ; then store new one
        MOV     R0, R10                 ; R0 -> old key handler
        ExitSWIHandler EQ               ; exit if just reading

        Push    "R0-R2,R14"
        MOV     R12, #IOC
        BL      ResetHW                 ; else reset the world
        Pull    "R0-R2,R14"
        ExitSWIHandler

; *****************************************************************************
;
;       IssueKeyboardService - Issue keyboard handler service
;
; in:   R11 -> KeyWorkSpace
;
; out:  R0,PSR preserved
;

IssueKeyboardService
        Push    "R0,R14"
        MOV     R1, #Service_KeyHandler
        LDRB    R2, KbId
        IssueService
        MOV     R0, #3                          ; indicate modules inited
        STRB    R0, ModulesOK                   ; and service offered
        Pull    "R0,PC",,^


        LNK     Source.PMF.Key2
