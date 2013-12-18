;-------------------------------------------------------------------
;                                                          lines.s
;-------------------------------------------------------------------

; This file is the lines and point plotting file for the drawing
;   engine. It covers drawing of single points, and Bresenham
;   line drawing of zero width lines. The actual implementation
;   of the plot funcitons into the bit-images is realized in this
;   file for lines and points, whereas it is realized in the file
;   'rectangles.s' for any other graphics primitives.

;-------------------------------------------------------------------
;                                                    Print options
;-------------------------------------------------------------------

	OPT    32  ; SET,GBL,LCL directives not listed

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
$label
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
getCommand  SELECT heliosMode,"GET",";"
        $getCommand heliosHdr/listopts.s
        $getCommand heliosHdr/arm.s
        $getCommand heliosHdr/basic.s
        $getCommand heliosHdr/structs.s
        $getCommand heliosHdr/module.s
        
        [ heliosMode                ; If in Helios Mode ...
        OPT    32                   ; Reset printer options
        IMPORT dpLineDebug          ; Import rectangle debugging
        StartModule lines,-1,0      ; Start of Module
        static                      ; Define externally availiable locations :
        static_extern_func dpLookUpLineRoutineAddress
        DATA               dpLinesStart,4
        ADR   ip,dpRealLinesStart
        STR   ip,[R2,#:OFFSET:dpLinesStart]
        static_end                  ; End of static area
        
        |                           ; If not in Helios :
        AOUT                        ; Output format directive
        AREA   dpLinesCode,CODE
        IMPORT  |_dpLineDebug|      ; Simply import/export symbols ...
|_dpLinesStart| DCD  dpRealLinesStart
        EXPORT  |_dpLinesStart|
        EXPORT  |_dpLookUpLineRoutineAddress|
        ]

dpRealLinesStart ; Start of 'lines.s' code.

;-------------------------------------------------------------------
;                                                'structures' file
;-------------------------------------------------------------------

; The following file contains the block-layout directives to determine
;   various structures shared by 'C' code and assembley code.

       GET    lochdr/structures.ss

;-------------------------------------------------------------------
;                                                 'Utilities' file
;-------------------------------------------------------------------

; The following file contains some useful macro defitions, and the
;   register aliases.
; Look in this file for defintions of 'RBC', 'NOR', 'ENR'
;   etc.

        GET    lochdr/utility.ss

;--------------------------------------------------------------------
;                                           The diagnostics routine
;--------------------------------------------------------------------

; To place a breakpoint in the code, a macro compiles to some
;    instructions which save the pc and lr registers, loads
;    the return address (past the macro), without flags into lr,
;    and branches to this routine, such that the saved pc points
;    to the diagnositcs message and value code. This routine saves
;    all the rest of the registers in the correct space in the
;    blitter control block, restores the stack frame, calls a 'C'
;    code to implement the diagnostics, and on return from this routine,
;    restores the registers and returns to the code.

doDiagnostics
        ; No load/store multiples are used in order to avoid
        ;   having to change the value of sp in the process:
        STR    R0, [sp,#lcRegBlock+ 0*4]
        STR    R1, [sp,#lcRegBlock+ 1*4]
        STR    R2, [sp,#lcRegBlock+ 2*4]
        STR    R3, [sp,#lcRegBlock+ 3*4]
        STR    R4, [sp,#lcRegBlock+ 4*4]
        STR    R5, [sp,#lcRegBlock+ 5*4]
        STR    R6, [sp,#lcRegBlock+ 6*4]
        STR    R7, [sp,#lcRegBlock+ 7*4]
        STR    R8, [sp,#lcRegBlock+ 8*4]
        STR    R9, [sp,#lcRegBlock+ 9*4]
        STR    R10,[sp,#lcRegBlock+10*4]
        STR    R11,[sp,#lcRegBlock+11*4]
        STR    R12,[sp,#lcRegBlock+12*4]
        STR    R13,[sp,#lcRegBlock+13*4]

        ; Now to get some semblance of a proper stack frame, and
        ;    call the 'C' routine:
        MOV    ip,pc                    ; pc -> ip
        MOV    R0,#2_11                 ; Get flags mask in R0 ...
        ORR    R0,R0,#2_1111 :SHL: 28   ; ... continued
        AND    ip,ip,R0                 ; Get only pc flags in ip
        BIC    lr,lr,R0                 ; Clear flags in lr
        ORR    lr,lr,ip                 ; Save pc flags in lr
        STR    lr,[sp,#lcDiagReturn]    ; Save breakpoint return address
        MOV    R0,sp                    ; Parameter: control block pointer
        LDR    sp,[sp,#lcStkPtr]        ; Load old stack pointer
        LDMIA  sp,{R4-R9,sl,fp,lr}      ; Load old registers
        STMFD  sp!,{R0}                 ; Save control block pointer
        [ heliosMode
        BL     dpLineDebug              ; Call 'C' routine
        |
        BL     |_dpLineDebug|           ; Call 'C' routine
        ]
        LDMFD  sp!,{R0}                 ; Get control block pointer
        MOV    sp,R0                    ; Move it back into sp
        
        LDR    R0, [sp,#lcRegBlock+ 0*4]
        LDR    R1, [sp,#lcRegBlock+ 1*4]
        LDR    R2, [sp,#lcRegBlock+ 2*4]
        LDR    R3, [sp,#lcRegBlock+ 3*4]
        LDR    R4, [sp,#lcRegBlock+ 4*4]
        LDR    R5, [sp,#lcRegBlock+ 5*4]
        LDR    R6, [sp,#lcRegBlock+ 6*4]
        LDR    R7, [sp,#lcRegBlock+ 7*4]
        LDR    R8, [sp,#lcRegBlock+ 8*4]
        LDR    R9, [sp,#lcRegBlock+ 9*4]
        LDR    R10,[sp,#lcRegBlock+10*4]
        LDR    R11,[sp,#lcRegBlock+11*4]
        LDR    R12,[sp,#lcRegBlock+12*4]
        LDR    R13,[sp,#lcRegBlock+13*4]
        
        LDR    lr,[sp,#lcDiagReturn]      ; Return to breakpoint ...
        MOVS   pc,lr                      ; ... continued

;--------------------------------------------------------------------
;                                             The diagnostics macro
;--------------------------------------------------------------------

; The following macro is a generic macro which comiles a breakpoint
;   into the code. The parameter $value is an action code which is
;   picked up by the 'C' routine (in debug.c) which uses it to
;   determine the nature of the diagnostics printed, and the
;   parameter $message is a string which is printed out as the
;   diagnostics message.

        MACRO
$label  DIAGNOSTICS   $value,$message
        STR    lr,[sp,#lcRegBlock+14*4]  ; Save link register
        ADR    lr,%F01                   ; Get return address
        STR    pc,[sp,#lcRegBlock+15*4]  ; Save PC (points to message)
        B      doDiagnostics             ; Call wrapper code
        &      $value                    ; Value parameter goes here
        =      $message,0                ; Message goes here
        ALIGN                            ; Align to word boundary
01      ; Return here                    ; Return here
        LDR    lr,[sp,#lcRegBlock+14*4]  ; Reload link register
        MEND

;--------------------------------------------------------------------
;                                             The 'DIAG' etc macros
;--------------------------------------------------------------------

; These macros are invokations of the appropriate 'DIAGNOSTICS' macro
;   with the correct action code

; The following macros can be used at most places in the code
;   namely where the registers R4-R9,sl,fp, and lr have been
;   stored on a full decrementing stack for which the subsequent
;   value of the pointer has been stored in the offset 'lcStkPtr'
;   of a control block whose address is pointed to by sp.

; It accepts a string message and a numeric value as parameters
;    and essentially causes a branch to some 'C' code which 
;    prints out diagnostic information, and then acts according
;    to the parameter:
; 0 => Return to code
; 1 => Prompt then return to code
        
        MACRO
$label  BKPT   $msg        ; Breakpoint macro : prompt before continuing
        DIAGNOSTICS 0,$msg
        MEND
        
        MACRO
$label  DIAG   $msg       ; Diagnostics macro : continue immediately
        DIAGNOSTICS 1,$msg
        MEND
        
        MACRO
$label  SDIAG  $msg       ; Short diagnostics macro
        DIAGNOSTICS 2,$msg
        MEND

;-------------------------------------------------------------------
;                                               The 'SELECT' macro
;-------------------------------------------------------------------

; This macro is used for string selection: It assigns $t to $l if
;   $c is true, otherwise it assigns $e to $l.

       SHORT_MACRO
$l     SELECT  $c,$t,$e
       [ $c
$l     SETS   $t
       |
$l     SETS   $e
       ]
       MEND

;-------------------------------------------------------------------
;                                           How line-drawing works
;-------------------------------------------------------------------

; This file supports plotting of lines using Bresenham techniques.

; It only supports FillSolid line-plotting, in masked or unmasked
;   mode, any other plotting of zero width lines will have to be
;   implemented by thick-line drawing routines specially configured
;   to plot the same pixels as this file.

; This file exports one global symbol which is a table containing
;   the addresses of the 16 main routines exported by this file.
; These routines are divided into whether plotting is masked or
;   unmasked, whether the major axis is X or whether it is y, and
;   which (of the four) quadrants the line lies in. The routines
;   are each passed the address of a line-control-block: a structure
;   shared between this assembler code and the 'C' routines which
;   control it. The line-control-block contains (amongst other
;   information), the co-ordinates of the first point to be plotted,
;   the amount of points to be plotted, and the initial 'Bresenham
;   numerator'.

;-------------------------------------------------------------------
;                                    Choosing the pixels to colour
;-------------------------------------------------------------------

; For the purposes of this code we consider pixel centres to be
;   in the centres of the bounding co-ordinate boxes, so that the
;   centre of the top-left pixel of a pixmap has co-ordinates
;   of (0.5,0.5). Hence to look like an 'X' server, we must implicitly
;   add 0.5 to the start and end co-ordinates of any line we are
;   asked to plot. In both cases our origin is in the upper left corner
;   of the screen.

; Suppose we aim to draw a line from (x,y) to (ex,ey). Then our
;   deltas are Dx=abs(ex-x) and Dy=abs(ey-y). The major axis is
;   X if Dx>Dy and Y if Dy>Dx. It doesn't matter which we choose
;   to be our major axis if Dx=Dy.

; We use the term 'bounding box' of the pixel whose centre is 
;   (px+0.5,py+0.5) to mean the area of the plane for which
;   (px) <= (x) < (px+1) and (py) <= (y) < (py+1). Note the
;   inequality to the left and above is inclusive, whilst the
;   inequalty to the right is exclusive.

; The priciple of perfroming the plot of the line is to notionally
;   draw the extended line segment, and then examine the points where
;   the line segment intersects with the axes (x=px+0.5) for various
;   values of 'px' (if x is the major axis), or (y=py+0.5) for various
;   values of 'py' (if y is the major axis). We colour the pixels
;   whose bounding box these points lie within.

; For the sake of the following explanation, we assume the major axis
;   is x, then same priciple applies if the major axis is y simply
;   by swapping the 'x's for 'y's and vica-versa.

; The routines in this file are passed an 'initial co-ordinate' which
;   can be thought of as an (x,y) pair, and a 'Bresenham numerator'.
; The 'Bresenham numerator' will be a number in the range (-2*Dx) to
;   (-1) inclusive.
; If the line is in an increasing-y quadrant, then an intial value of
;   (px,py) for the location of the point along with an intial value
;   of Bn for the Bresenham numerator implies that the line to be 
;   plotted crosses the axis (x=px+0.5) at exactly the point
;   (px+0.5,py+1+(Bn/(2*Dx))). It is easy to see that provided Bn is 
;   within the range sepcified, this point will lie in the bounding 
;   box of the pixel whose upper left corner is (px,py). To compute 
;   the (precise) location of the next point along the major axis, we 
;   note that it will be the point (px(+/-)1+0.5,py+1+((Bn+2*Dy)/(2*Dx)) ). 
;   The (+/-) in the preceeding formula being according to whether x 
;   is increasing or decreasing. Now, if Bn' = (Bn+2*Dy) has remained 
;   in the range specified, then this point will be in the bounding 
;   box of the pixel with upper left co-ordinate (px+1,py), otherwise 
;   we decrement Bn' by (2*Dx) and correspondingly increment py by one 
;   and, given that Dy<=Dx, we will have that Bn' is in the range 
;   specified, and that the new point is in the bounding box of the 
;   pixel with upper-left co-ordinate (px+1,py+1).

; If the line is in an decreasing-y quadrant, then an initial value of
;   (px,py) for the location of the point along with an initial value
;   of Bn for the Bresenham numerator implies that the line to be
;   plotted crosses the axis (x=px+0.5) at exactly the point with
;   co-ordinates (px+0.5,py+((-Bn-1)/(2*Dx))). And again, if Bn is
;   within it's range then we get this point being within the bounding
;   box of the pixel with upper-left co-ordinate (px,py). The next pixel
;   will have co-ordinate (px(+/-)1+0.5,py+1+((-(Bn+2*Dy)-1)/(2*Dx))),
;   and again, if Bn'=Bn+(2*Dy) is within the required range the the
;   next point is in the pixel whose bounding box has upper-left
;   co-ordinate (px(+/-)1,py). Otherwise, again, decrment Bn by 2*Dx
;   and decrement py accordingly by 2 and the new point is then in the
;   pixel bounding box with upper-left co-ordinate (px(+/-)1,py-1).

;-------------------------------------------------------------------
;                                    Summary of Bresenham plotting
;-------------------------------------------------------------------

; The previous comments about choosing which pixels to plot boils
;   down to a generic method for manipulating the Bresenham
;   number: namely:
; 1. Plot the current point,
; 2. Move once in the correct direction along the major axis,
; 3. Increment the Bresenham number by 2*Dmin where Dmin is Dx or Dy
;    according to which one is the minor axis.
; 4. If the Bresenham number has overflowed (>=0) then decrement
;    it by 2*Dmaj, and move once in the appropriate direction along
;    the minor axis.
; 5. Go back to step 1.

;-------------------------------------------------------------------
;                                                 Overview of file
;-------------------------------------------------------------------

; Line plotting enters at one of sixteen routines according to whether
;  plotting masked or un-masked, whether the major axis is X or Y, and
;  which quadrant the line is to be plotted in. The outer most loop
;  of the routine cycles through the bit-planes of the destination
;  pix-map. For each bit-plane, the routine loads the operation to
;  be conducted in the plane for the even strokes of the line, and
;  the operation to be conducted for the odd strokes of the line,
;  these each being one of four operations (set 1, set 0, invert, or
;  no-op), and then cycles through the dot-dash strokes, for each
;  stroke, the routine determines the correct operation to perform,
;  and branches to one of the four routines associated with it to
;  Bresenham-reduce the stroke and plot it using that information, these
;  routines are called the 'drawStroke' routines, and it can be deduced
;  that there are 64 if them, 4 for each of the 16 main entry routines:
;  note that even a no-op stroke has to excecute just to compute
;  the correct end-pixel parameters for the next stroke.

;-------------------------------------------------------------------
;                              Look-up table : main entry routines
;-------------------------------------------------------------------

; This table is used for external routines to look-up the correct
;   main-entry routine to pass the line-control block to.

lookUpLineEntry

        & doMainEntryUmMxXpYp - lookUpLineEntry
        & doMainEntryUmMyXpYp - lookUpLineEntry
        & doMainEntryUmMxXpYn - lookUpLineEntry
        & doMainEntryUmMyXpYn - lookUpLineEntry
        & doMainEntryUmMxXnYp - lookUpLineEntry
        & doMainEntryUmMyXnYp - lookUpLineEntry
        & doMainEntryUmMxXnYn - lookUpLineEntry
        & doMainEntryUmMyXnYn - lookUpLineEntry
        
        & doMainEntryMsMxXpYp - lookUpLineEntry
        & doMainEntryMsMyXpYp - lookUpLineEntry
        & doMainEntryMsMxXpYn - lookUpLineEntry
        & doMainEntryMsMyXpYn - lookUpLineEntry
        & doMainEntryMsMxXnYp - lookUpLineEntry
        & doMainEntryMsMyXnYp - lookUpLineEntry
        & doMainEntryMsMxXnYn - lookUpLineEntry
        & doMainEntryMsMyXnYn - lookUpLineEntry

;-------------------------------------------------------------------
;                                   Looking up line entry routines
;-------------------------------------------------------------------

; This routine is used from 'C' to look-up the address of a main-
;    line-entry routine as listed in the above table. It has been
;    written in assembler to avoid casting difficulties with ANSI
;    C.

        ; At head of file : EXPORT |_dpLookUpLineRoutineAddress|

dpLookUpLineRoutineAddress externLabel

        ADR     ip,lookUpLineEntry      ; Get table address
        LDR     R0,[ip,R0,LSL#2]        ; Get table entry
        ADD     R0,ip,R0                ; Add to base of table
        MOVS    pc,lr                   ; Return

;-------------------------------------------------------------------
;                                            The 'MainEntry' macro
;-------------------------------------------------------------------

; This macro is used to compile the main entry routines to the
;   line drawing functions. It compiles to a routine which obeys
;   the procedure call standard.

; The macro accepts the following parameters:
; $ms  : "Um" => Un-masked    ; "Ms" => masked
; $ma  : "Mx" => x major axis ; "My" => y major axis
; $xq  : "Xp" => DX>=0        ; "Xn" => DX<=0
; $yq  : "Yp" => DY>=0        ; "Yn" => DX<=0

        MACRO
$label  MainEntry $ms,$ma,$xq,$yq
$label
        STMFD  sp!,{R4-R9,sl,fp,lr}  ; Save registers
        STR    sp,[R0,#lcStkPtr]     ; Save stack pointer in control block
        MOV    sp,R0                 ; Load control block address
        
        [ "$ms"="Um"
        DrawPlanesUm $ma,$xq,$yq
        |
        DrawPlanesMs $ma,$xq,$yq
        ]
        
        LDR    sp,[sp,#lcStkPtr]     ; Restore stack pointer
        LDMFD  sp!,{R4-R9,sl,fp,pc}^ ; Return with registers and flags

        MEND                         ; End of macro

;-------------------------------------------------------------------
;                              The 'drawPlanes' process : unmasked
;-------------------------------------------------------------------

; The following macro compiles to a section of code which loops
;   through the colour planes, re-drawing the line-segment in each
;   plane in the appropriate colour modes. It used the macro defined
;   ahead to implement the drawing of the stroke in the separate planes
; The macro expects sp to point to the control block

        MACRO
$label  DrawPlanesUm $ma,$xq,$yq
$label
        ; Create an id to make labels unique:
        LCLS   id
id      SETS   "Um" :CC: "$ma" :CC: "$xq" :CC: "$yq"

        LDR    R0,[sp,#lcDesBase]      ; Load destination raw pixel base
        LDR    R3,[sp,#lcDesWpl]       ; Load destination WPL
        LDR    R4,[sp,#lcDesY]         ; Load start Y
        MUL    R5,R3,R4                ; Get row offset into data
        ADD    R5,R0,R5,ASL#2          ; Get row start address
        LDR    R4,[sp,#lcDesX]         ; Load start X
        MOV    R6,R4,LSR#5             ; Get word offset into row
        ADD    R5,R5,R6,ASL#2          ; Add word offset into row
        AND    R4,R4,#2_11111          ; Get bit offset
        MOV    R6,#1                   ; Compute des mask for first pixel ...
        MOV    R6,R6,LSL R4            ; ... continued
        [ "$xq"="Xp"                   ; If in posotive X direction ...
        MOV    R6,R6,ROR#1             ; Apply alteration (see below)
        ]                              ; ... End Of Condition
        LDR    R10,[sp,#lcForeground]  ; Load foreground mode
        LDR    R11,[sp,#lcBackground]  ; Load background mode
        LDR    ip,[sp,#lcStrSign]      ; Load sign of first stroke
        CMPS   ip,#0                   ; Is it an odd stroke ?
        EORNE  R10,R10,R11             ; If so, swap colours ...
        EORNE  R11,R10,R11             ; ... continued ...
        EORNE  R10,R10,R11             ; ... continued
        LDR    R7,[sp,#lcDesDepth]     ; Get depth of des pixmap
;-------------------------------------------------------------------------+
planeLoop$id                           ; Once per bit-plane               |
        MOV    R1,R5                   ; Load address parameters ...      |
        MOV    R2,R6                   ;  ... continued ...               |
        AND    R8,R10,#1               ; Get lower bit of fgd plot mode   |
        AND    R9,R11,#1               ; Get lower bit of bgd plot mode   |
        TSTS   R10,#(1:SHL:16)         ; Get upper bit of fgd plot mode...|
        ORRNE  R8,R8,#2_10             ; ... continued                    |
        TSTS   R11,#(1:SHL:16)         ; Get upper bit of bgd plot mode...|
        ORRNE  R9,R9,#2_10             ; ... continued                    |
        MOV    R10,R10,LSR#1           ; Shift to next plane ...          |
        MOV    R11,R11,LSR#1           ; ... continued                    |
        STR    R10,[sp,#lcFgdSave]     ; And save ...                     |
        STR    R11,[sp,#lcBgdSave]     ; ... continued                    |
        DrawSegmentUm $ma,$xq,$yq      ; Draw segment                     |
        LDR    R10,[sp,#lcFgdSave]     ; Reload fgd and background modes  |
        LDR    R11,[sp,#lcBgdSave]     ; ... continued                    |
        LDR    ip,[sp,#lcDesWpv]       ; Load dest. words per vector ...  |
        ADD    R5,R5,ip,ASL#2          ; Go to next bit-plane             |
        SUBS   R7,R7,#1                ; Decrement and branch ...         |
        BNE    planeLoop$id            ; ... continued                    |
;-------------------------------------------------------------------------+
        MEND                           ; End of operation

;-------------------------------------------------------------------
;                                The 'drawPlanes' process : masked
;-------------------------------------------------------------------

; This is the version of the above routines for masked pixmaps

        MACRO
$label  DrawPlanesMs $ma,$xq,$yq
$label
        ; Create an id to make labels unique:
        LCLS   id
id      SETS   "Ms" :CC: "$ma" :CC: "$xq" :CC: "$yq"

        LDR    R0,[sp,#lcDesBase]      ; Load destination raw pixel base
        LDR    R3,[sp,#lcDesWpl]       ; Load destination WPL
        LDR    R4,[sp,#lcDesY]         ; Load start Y
        MUL    R1,R3,R4                ; Get row offset into data
        ADD    R1,R0,R1,ASL#2          ; Get row start address
        LDR    R4,[sp,#lcDesX]         ; Load start X
        MOV    R2,R4,LSR#5             ; Get word offset into row
        ADD    R1,R1,R2,ASL#2          ; Add word offset into row
        AND    R4,R4,#2_11111          ; Get bit offset
        MOV    R2,#1                   ; Compute des mask for first pixel ...
        MOV    R2,R2,LSL R4            ; ... continued
        [ "$xq"="Xp"                   ; If in posotive X direction ...
        MOV    R2,R2,ROR#1             ; Apply alteration (see below)
        ]                              ; ... End Of Condition
        LDR    R0,[sp,#lcMskBase]      ; Load mask raw pixel base
        LDR    R7,[sp,#lcMskWpl]       ; Load mask WPL
        LDR    R4,[sp,#lcMskOffY]      ; Load start Y ...
        LDR    ip,[sp,#lcDesY]         ; ... continued ...
        SUB    R4,ip,R4                ; ... continued
        MUL    R5,R7,R4                ; Get row offset into data
        ADD    R5,R0,R5,ASL#2          ; Get row start address
        LDR    R4,[sp,#lcMskOffX]      ; Load start X ...
        LDR    ip,[sp,#lcDesX]         ; ... continued ...
        SUB    R4,ip,R4                ; ... continued
        MOV    R6,R4,LSR#5             ; Get word offset into row
        ADD    R5,R5,R6,ASL#2          ; Add word offset into row
        AND    R4,R4,#2_11111          ; Get bit offset
        MOV    R6,#1                   ; Compute des mask for first pixel ...
        MOV    R6,R6,LSL R4            ; ... continued
        [ "$xq"="Xp"                   ; If in posotive X direction ...
        MOV    R6,R6,ROR#1             ; Apply alteration (see below)
        ]                              ; ... End Of Condition
        LDR    R10,[sp,#lcForeground]  ; Load foreground mode
        LDR    R11,[sp,#lcBackground]  ; Load background mode
        LDR    ip,[sp,#lcStrSign]      ; Load sign of first stroke
        CMPS   ip,#0                   ; Is it an odd stroke ?
        EORNE  R10,R10,R11             ; If so, swap colours ...
        EORNE  R11,R10,R11             ; ... continued ...
        EORNE  R10,R10,R11             ; ... continued
        LDR    ip,[sp,#lcDesDepth]     ; Get depth of des pixmap
        STR    ip,[sp,#lcPlaneSave]    ; Save in bit-plane countdown 
;-------------------------------------------------------------------------+
planeLoop$id                           ; Once per bit-plane               |
        AND    R8,R10,#1               ; Get lower bit of fgd plot mode   |
        AND    R9,R11,#1               ; Get lower bit of bgd plot mode   |
        TSTS   R10,#(1:SHL:16)         ; Get upper bit of fgd plot mode...|
        ORRNE  R8,R8,#2_10             ; ... continued                    |
        TSTS   R11,#(1:SHL:16)         ; Get upper bit of bgd plot mode...|
        ORRNE  R9,R9,#2_10             ; ... continued                    |
        MOV    R10,R10,LSR#1           ; Shift to next plane ...          |
        MOV    R11,R11,LSR#1           ; ... continued                    |
        STR    R10,[sp,#lcFgdSave]     ; And save ...                     |
        STR    R11,[sp,#lcBgdSave]     ; ... continued                    |
        STR    R1,[sp,#lcDesPtrSave]   ; Save des pointer                 |
        STR    R2,[sp,#lcDesMskSave]   ; Save des mask                    |
        STR    R5,[sp,#lcMskPtrSave]   ; Save msk pointer                 |
        STR    R6,[sp,#lcMskMskSave]   ; Save msk mask                    |
        DrawSegmentMs $ma,$xq,$yq      ; Draw segment                     |
        LDR    R1,[sp,#lcDesPtrSave]   ; Load des pointer                 |
        LDR    R2,[sp,#lcDesMskSave]   ; Load des mask                    |
        LDR    R5,[sp,#lcMskPtrSave]   ; Load msk pointer                 |
        LDR    R6,[sp,#lcMskMskSave]   ; Load msk mask                    |
        LDR    R10,[sp,#lcFgdSave]     ; Reload fgd and background modes  |
        LDR    R11,[sp,#lcBgdSave]     ; ... continued                    |
        LDR    ip,[sp,#lcDesWpv]       ; Load dest. words per vector ...  |
        ADD    R1,R1,ip,ASL#2          ; Go to next bit-plane             |
        LDR    ip,[sp,#lcPlaneSave]    ; Reload plane - countdown         |
        SUBS   ip,ip,#1                ; Decrement save and branch ...    |
        STR    ip,[sp,#lcPlaneSave]    ; ... continued ...                |
        BNE    planeLoop$id            ; ... continued                    |
;-------------------------------------------------------------------------+
        MEND                           ; End of operation

;-------------------------------------------------------------------
;                                          Point plotting strategy
;-------------------------------------------------------------------

; To plot a point in a certain bit-plane at a certain pixel location
;   we need to alter a single bit in a certain word in memory. The
;   general idea is to maintain a preloaded value for the destination
;   word to be written and alter that value as we plot the pixels,
;   then write-back the current and re-load the next destination word
;   only when the pixel co-ordinates change in such a way that the
;   co-ordinates point to a new destination word. Thus it is the 'step'
;   instructions where the co-ordinates are changed where writing and
;   reading of the destination words occur.

; The destination address is held in R1 and the currently pre-read
;   destination word in R0; R2 always has one bit set in the bit-position
;   of the bit to be read in the current point, except in the case
;   where DX is posotive, where R2 is implicitly rotated right by one bit
;   in order to make the induction routines for the X-axis more efficient.

; There is of course a similar operation in effect for pre-loading the
;   mask pixmap words (but post-writing is not here applicable), in
;   which case R4 contains the pre-loaded word, R5 the address of the
;   currently loaded word, R6 the mask (implicitly shifted if applicable),
;   and R7 the number of words per line in the mask pixmap, ie. the
;   increment required to get from one row to the next.

;-------------------------------------------------------------------
;                               The 'draw segment' macro, unmasked
;-------------------------------------------------------------------

; This macro is used to compile a section of code to draw the line
;  in the appropriate quadrant with the appropriate major axis in
;  one of the bit-planes only, in unmasked mode. A different macro
;  is used to control masked segment plotting.

; Register usage is as follows:

;          Entry                        Exit
; R0  -    Undefined                 Undefined
; R1  -    Des ptr                   Undefined
; R2  -    Des msk                   Undefined
; R3  -    Des WPL                     Preserved
; R4  -    Undefined                 Undefined
; R5  -    Undefined                   Preserved
; R6  -    Undefined                   Preserved
; R7  -    Undefined                   Preserved
; R8  -    Stroke 0 mode             Undefined
; R9  -    Stroke 1 mode             Undefined
; R10 -    Undefined                 Undefined
; R11 -    Undefined                 Undefined
; ip  -    Undefined                 Undefined
; sp  -    Control block pointer       Preserved
; lr  -    Undefined                 Undefined
; pc  -    macro start                 macro end

; The encoding of the mode is as follows:
; 2_00    - Set to zero
; 2_01    - Invert
; 2_10    - No-op
; 2_11    - Set to one

        MACRO
$label  DrawSegmentUm $ma,$xq,$yq
$label
        ; Create an id to make labels unique
        LCLS    id
id      SETS    "Um" :CC: "$ma" :CC: "$xq" :CC: "$yq"

        SUB    ip,pc,#4              ; Get mode table address with flags ...
        ADD    ip,ip,#modeTable$id-{PC} ; ... continued
        AND    R0,ip,#&03FFFFFC      ; ... and without flags
        LDR    lr,[R0,R8,ASL#2]      ; Get first-stroke plot mode addr
        LDR    R9,[R0,R9,ASL#2]      ; Get second-stroke plot mode addr
        ADD    lr,lr,ip              ; Add offset and flags
        ADD    R9,R9,ip              ; Add offset and flags
        EOR    R9,lr,R9              ; Compute EOR-alternator
        STR    R9,[sp,#lcStrAlt]     ; Save EOR-alternator
        LDR    R8,[sp,#lcRatioX]     ; Load delta X
        LDR    R9,[sp,#lcRatioY]     ; Load delta Y
        LDR    R10,[sp,#lcStrLen]    ; Load first stroke length
        LDR    R11,[sp,#lcNumer]     ; Load initial numerator
        LDR    R0,[R1]               ; Pre-load first destination word
        LDR    R4,[sp,#lcStrPtr]     ; Load next stroke length address
        LDR    ip,[sp,#lcPoints]     ; Load number of points
        B      strokeEntry$id        ; Branch into middle of loop
;--------------------------------------------------------------------+
strokeLoop$id                        ; Once per stroke               |
        LDR    R10,[sp,#lcStrAlt]    ; Load EOR alternator           |
        EOR    lr,lr,R10             ; Alternate dot/dash colour     |
        LDRB   R10,[R4],#1           ; Load next stroke length       |
        CMPS   R10,#&00              ; Is it end of stroke?          |
        LDREQ  R4,[sp,#lcStrBase]    ; If so, reset stroke pointer,  |
        LDREQB R10,[R4],#1           ;   and load first stroke length|
strokeEntry$id                       ; Enter loop in middle          |
        SUBS   ip,ip,R10             ; Compute segment remainder     |
        ADDLE  R10,ip,R10            ; If last, compute proper length|
        MOV    pc,lr                 ; Branch to correct draw process|
strokeReturn$id                      ; Return here                   |
        CMPS   ip,#0                 ; Test if more to do            |
        BGT    strokeLoop$id         ; Loop if more strokes to do    |
;--------------------------------------------------------------------+
        STR    R0,[R1]               ; Post-write last dest word
        MEND                         ; End of macro

;-------------------------------------------------------------------
;                                 The 'draw segment' macro, masked
;-------------------------------------------------------------------

; This macro is used to compile a section of code to draw the line
;  in the appropriate quadrant with the appropriate major axis in
;  one of the bit-planes only, in masked mode. A different macro
;  is used to control masked segment plotting.

; Register usage is as follows:

;          Entry                        Exit
; R0  -    Undefined                 Undefined
; R1  -    Des ptr                   Undefined
; R2  -    Des msk                   Undefined
; R3  -    Des WPL                     Preserved
; R4  -    Undefined                 Undefined
; R5  -    Msk ptr                   Undefined
; R6  -    Msk msk                   Undefined
; R7  -    Msk WPL                   Undefined
; R8  -    First stroke plot mode    Undefined
; R9  -    Secnd stroke plot mode    Undefined
; R10 -    Undefined                 Undefined
; R11 -    Undefined                 Undefined
; ip  -    Undefined                 Undefined
; sp  -    Control block pointer       Preserved
; lr  -    Undefined                 Undefined
; pc  -    macro start                 macro end

        MACRO
$label  DrawSegmentMs $ma,$xq,$yq
$label
        ; Create an id to make labels unique
        LCLS    id
id      SETS    "Ms" :CC: "$ma" :CC: "$xq" :CC: "$yq"

        SUB    ip,pc,#4              ; Get mode table address with flags ...
        ADD    ip,ip,#modeTable$id-{PC} ; ... continued
        AND    R0,ip,#&03FFFFFC      ; ... and without flags
        LDR    lr,[R0,R8,ASL#2]      ; Get first-stroke plot mode addr
        LDR    R9,[R0,R9,ASL#2]      ; Get second-stroke plot mode addr
        ADD    lr,lr,ip              ; Add offset and flags
        ADD    R9,R9,ip              ; Add offset and flags
        EOR    R9,lr,R9              ; Compute EOR-alternator
        STR    R9,[sp,#lcStrAlt]     ; Save EOR-alternator
        LDR    R8,[sp,#lcRatioX]     ; Load delta X
        LDR    R9,[sp,#lcRatioY]     ; Load delta Y
        LDR    R10,[sp,#lcStrLen]    ; Load first stroke length
        LDR    R11,[sp,#lcNumer]     ; Load initial numerator
        LDR    R4,[sp,#lcStrPtr]     ; Load next stroke length address
        STR    R4,[sp,#lcStrPtrSave] ; Save it in working location
        LDR    R0,[R1]               ; Pre-load first destination word
        LDR    R4,[R5]               ; Pre-load first mask word
        LDR    ip,[sp,#lcPoints]     ; Load number of points
        B      strokeEntry$id        ; Branch into middle of loop
;--------------------------------------------------------------------+
strokeLoop$id                        ; Once per stroke               |
        LDR    R8,[sp,#lcStrPtrSave] ; Load working stroke pointer   |
        LDR    R10,[sp,#lcStrAlt]    ; Load EOR alternator           |
        EOR    lr,lr,R10             ; Alternate dot/dash colour     |
        LDRB   R10,[R8],#1           ; Load next stroke length       |
        CMPS   R10,#&00              ; Is it end of stroke?          |
        LDREQ  R8,[sp,#lcStrBase]    ; If so, reset stroke pointer,  |
        LDREQB R10,[R8],#1           ;   and load first stroke length|
        STR    R8,[sp,#lcStrPtrSave] ; Save back stroke pointer,     |
        LDR    R8,[sp,#lcRatioX]     ; And restore R8                |
strokeEntry$id                       ; Enter loop in middle          |
        SUBS   ip,ip,R10             ; Compute segment remainder     |
        ADDLE  R10,ip,R10            ; If last, compute proper length|
        MOV    pc,lr                 ; Branch to correct draw process|
strokeReturn$id                      ; Return here                   |
        CMPS   ip,#0                 ; Test if more to do            |
        BGT    strokeLoop$id         ; Loop if more strokes to do    |
;--------------------------------------------------------------------+
        STR    R0,[R1]               ; Post-write last dest word
        MEND                         ; End of macro

;-------------------------------------------------------------------
;                                          The 'draw stroke' macro
;-------------------------------------------------------------------

; This macro is used to compile one of the 64 'drawStroke' routines,
;   which Bresenham reduce one stroke of a line and write it into
;   the destination pix-map using one of four plot-modes.

; For any particular routine compiled by this code, the return address
;   is always the same, and it's label is passed as a parameter to the
;   routine rather than using the link.

; The macro accepts the following parameters:

; $ms : Um => Un-masked       ; "Ms" => Masked
; $ma : Mx => Major-X         ; "My" => Major-Y
; $xq : "Xp" => x-increasing  ; "Xn" => x-decreasing
; $yq : "Yp" => y-increasing  ; "Yn" => y-decreasing
; $md : "0" => set-to-0 ; "1" => set-to-1 ; "i" => invert ; "x" => no-op
; $ra : label for the return address

;        Entry                           Exit
; R0   - Des preload                     Des postwrite
; R1   - Des ptr, first point            Des ptr, last+1 point
; R2   - Des msk, first point            Des msk, last+1 point
; R3   - WPL, destination                Preserved
; R4   - ( Msk preload          )        (Msk new pre-load     )
; R5   - ( Msk ptr, first point )        (Msk ptr, last+1 point)
; R6   - ( Msk msk, first point )        (Msk msk, last+1 point)
; R7   - ( WPL, mask            )        Preserved
; R8   - Delta X                         Preserved
; R9   - Delta Y                         Preserved
; R10  - No of pixels in stroke (>0)     Zero
; R11  - Bn for this point               Bn for last point +1
; ip   - Undefined                       Preserved
; sp   - Control block pointer           Preserved
; lr   - Undefined                       Preserved
; pc   - No special flags                Branch to $ra

; R4, R5,R6, and R7 will all be preserved if plotting in non-mask mode

        MACRO
$label  DrawStroke $ms,$ma,$xq,$yq,$md,$ra
$label

        ; Create an id to make labels unique
        LCLS    id
id      SETS    "$ms" :CC: "$ma" :CC: "$xq" :CC: "$yq" :CC: "$md"

        ; Compute minor and major axis step macros:
        LCLS    stepX
        LCLS    stepY
stepX   SELECT  "$xq"="Xp","IncX","DecX"
stepY   SELECT  "$yq"="Yp","IncY","DecY"
        LCLS    stepMaj
        LCLS    stepMin
stepMaj SELECT  "$ma"="Mx","$stepX","$stepY"
stepMin SELECT  "$ma"="Mx","$stepY","$stepX"

        ; Compute major and minor axis delta variables:
        LCLS    mjd
        LCLS    mnd
mjd     SELECT  "$ma"="Mx","R8","R9"
mnd     SELECT  "$ma"="Mx","R9","R8"

;------------------------------------------------------------------+
pointLoop$id                        ; Here every point             |
        Point$md$ms$xq              ; Plot current point           |
        ADDS   R11,R11,$mnd         ; Increment fraction           |
        SUBCS  R11,R11,$mjd         ; If overflow, fix fraction    |
        $ms$stepMin CS              ;  and inc minor axis          |
        $ms$stepMaj AL              ; Next major axis co-ordinate  |
        SUBS   R10,R10,#1           ; Decrement and branch ...     |
        BNE    pointLoop$id         ; ... continued                |
;------------------------------------------------------------------+
        B      $ra                  ; Return to correct point
        MEND                        ; End of macro
        
;-------------------------------------------------------------------
;                                     The 'Point' macros, unmasked
;-------------------------------------------------------------------

; These macros are used to plot a point in the appropriate one-of
;   four-modes into the destination, in un-masked mode

        SHORT_MACRO
$label  Point0UmXn ; Set to zero, DX<0, so R2 not implicitly shifted
$label  BIC    R0,R0,R2
        MEND

        SHORT_MACRO
$label  Point0UmXp ; Set to zero, DX>=0, so R2 implicitly shifted
$label  BIC    R0,R0,R2,ROR#31
        MEND

        SHORT_MACRO
$label  Point1UmXn ; Set to one, DX<0, so R2 not implicitly shifted
$label  ORR    R0,R0,R2
        MEND

        SHORT_MACRO
$label  Point1UmXp ; Set to one, DX>=0, so R2 implicitly shifted
$label  ORR    R0,R0,R2,ROR#31
        MEND

        SHORT_MACRO
$label  PointiUmXn ; Invert, DX<0, so R2 not implicitly shifted
$label  EOR    R0,R0,R2
        MEND

        SHORT_MACRO
$label  PointiUmXp ; Invert, DX>=0, so R2 implicitly shifted
$label  EOR    R0,R0,R2,ROR#31
        MEND

        SHORT_MACRO
$label  PointxUmXn ; No-op, DX<0, so R2 not implicitly shifted
$label 
        MEND

        SHORT_MACRO
$label  PointxUmXp ; No-op, DX>=0, so R2 implicitly shifted
$label 
        MEND

;-------------------------------------------------------------------
;                                       The 'Point' macros, masked
;-------------------------------------------------------------------

; These macros are used to plot a point in the appropriate one-of
;   four-modes into the destination, in un-masked mode.

        SHORT_MACRO
$label  Point0MsXn ; Set to zero, DX<0, so R2,R6 not implicitly shifted
$label  TSTS   R4,R6
        BICNE  R0,R0,R2
        MEND

        SHORT_MACRO
$label  Point0MsXp ; Set to zero, DX>=0, so R2,R6 implicitly shifted
$label  TSTS   R4,R6,ROR#31
        BICNE  R0,R0,R2,ROR#31
        MEND

        SHORT_MACRO
$label  Point1MsXn ; Set to one, DX<0, so R2,R6 not implicitly shifted
$label  TSTS   R4,R6
        ORRNE  R0,R0,R2
        MEND

        SHORT_MACRO
$label  Point1MsXp ; Set to one, DX>=0, so R2,R6 implicitly shifted
$label  TSTS   R4,R6,ROR#31
        ORRNE  R0,R0,R2,ROR#31
        MEND

        SHORT_MACRO
$label  PointiMsXn ; Invert, DX<0, so R2,R6 not implicitly shifted
$label  TSTS   R4,R6
        EORNE  R0,R0,R2
        MEND

        SHORT_MACRO
$label  PointiMsXp ; Invert, DX>=0, so R2,R6 implicitly shifted
$label  TSTS   R4,R6,ROR#31
        EORNE  R0,R0,R2,ROR#31
        MEND

        SHORT_MACRO
$label  PointxMsXn ; No-op, DX<0, so R2,R6 not implicitly shifted
$label 
        MEND

        SHORT_MACRO
$label  PointxMsXp ; No-op, DX>=0, so R2 implicitly shifted
$label 
        MEND

;-------------------------------------------------------------------
;                                        The step-macros, unmasked
;-------------------------------------------------------------------

; These macros are used to implement a change-in-co-ordinate during
;   Bresenham plotting of lines in un-masked mode. The condition
;   code must be exactly one of CS or AL due to the way in which
;   the macros operate

        SHORT_MACRO
$label  UmIncX  $c
$label  MOV$c.S R2,R2,ROR#31         ; Shift mask
        STRCS   R0,[R1],#4           ; Post-write and incr address if appl
        LDRCS   R0,[R1]              ; Pre-read next des word if appl
        MEND
        
        SHORT_MACRO
$label  UmDecX  $c
$label  MOV$c.S R2,R2,ROR#1          ; Shift mask
        STRCS   R0,[R1],#-4          ; Post-write and decr address if appl
        LDRCS   R0,[R1]              ; Pre-read next des word if appl
        MEND
        
        SHORT_MACRO
$label  UmIncY  $c
$label  STR$c   R0,[R1],R3,ASL#2     ; Write-back old des word and postinc
        LDR$c   R0,[R1]              ; Pre-read next des word
        MEND
        
        SHORT_MACRO
$label  UmDecY  $c
$label  STR$c   R0,[R1],-R3,ASL#2    ; Write-back old des word and postdec
        LDR$c   R0,[R1]              ; Pre-read next des word
        MEND

;-------------------------------------------------------------------
;                                          The step-macros, masked
;-------------------------------------------------------------------

; These macros to implement the step-operations in masked mode, again
;   the condition code may only be one of CC or AL

        SHORT_MACRO
$label  MsIncX  $c
$label  
        [ "$c"="CS"                  ; If conditional ...
        BCC     %FT01                ; Test and branch
        ]                            ; ... End of Condition
        MOVS    R2,R2,ROR#31         ; Shift mask for destination
        STRCS   R0,[R1],#4           ; Post-write and incr address if appl
        LDRCS   R0,[R1]              ; Pre-read next des word if appl
        MOVS    R6,R6,ROR#31         ; Shift mask for mask
        LDRCS   R4,[R5,#4]!          ; Inc and pre-read next msk word if appl
01                                   ; Branch past to here
        MEND
        
        SHORT_MACRO
$label  MsDecX  $c
$label  
        [ "$c"="CS"                  ; If conditional ...
        BCC     %FT01                ; Test and branch
        ]                            ; ... End of Condition
        MOVS    R2,R2,ROR#1          ; Shift mask for destination
        STRCS   R0,[R1],#-4          ; Post-write and decr address if appl
        LDRCS   R0,[R1]              ; Pre-read next des word if appl
        MOVS    R6,R6,ROR#1          ; Shift mask for mask
        LDRCS   R4,[R5,#-4]!         ; Dec and pre-load next msk word if appl
01                                   ; Rejoin here
        MEND
        
        SHORT_MACRO
$label  MsIncY  $c
$label  STR$c   R0,[R1],R3,ASL#2     ; Write-back old des word and postinc
        LDR$c   R0,[R1]              ; Pre-read next des word
        LDR$c   R4,[R5,R7,ASL#2]!    ; Inc and preload next msk word
        MEND
        
        SHORT_MACRO
$label  MsDecY  $c
$label  STR$c   R0,[R1],-R3,ASL#2    ; Write-back old des word and postdec
        LDR$c   R0,[R1]              ; Pre-read next des word
        LDR$c   R4,[R5,-R7,ASL#2]!   ; Dec and pre-load next msk word
        MEND

;-------------------------------------------------------------------
;                                                 Mode Table macro
;-------------------------------------------------------------------

; The following macro compiles a look-up table of size four to
;   point to the appropriate four 'draw stroke' routines associated
;   with a particular main routine for plotting a line masked or
;   un masked in the appropriate quadrant etc, it also compiles the
;   four associated 'draw stroke' routines ...

        MACRO 
$label  ModeTableMacro $ms,$ma,$xq,$yq
$label
        LCLS  id
id      SETS  "$ms" :CC: "$ma" :CC: "$xq" :CC: "$yq"
        &     doStroke$ms$ma$xq$yq.0 - $label
        &     doStroke$ms$ma$xq$yq.i - $label
        &     doStroke$ms$ma$xq$yq.x - $label
        &     doStroke$ms$ma$xq$yq.1 - $label
doStroke$ms$ma$xq$yq.0 DrawStroke $ms,$ma,$xq,$yq,0,strokeReturn$id
doStroke$ms$ma$xq$yq.i DrawStroke $ms,$ma,$xq,$yq,i,strokeReturn$id
doStroke$ms$ma$xq$yq.x DrawStroke $ms,$ma,$xq,$yq,x,strokeReturn$id
doStroke$ms$ma$xq$yq.1 DrawStroke $ms,$ma,$xq,$yq,1,strokeReturn$id
        MEND

;-------------------------------------------------------------------
;                      Macro invokations for main - entry routines
;-------------------------------------------------------------------

; Along with each main-entry routine we have to invoke macros
;   for the four combination-mode stroke-plotting routines which
;   go with it:

doMainEntryUmMxXpYp MainEntry        Um,Mx,Xp,Yp
modeTableUmMxXpYp   ModeTableMacro   Um,Mx,Xp,Yp

doMainEntryUmMxXpYn MainEntry        Um,Mx,Xp,Yn
modeTableUmMxXpYn   ModeTableMacro   Um,Mx,Xp,Yn

doMainEntryUmMxXnYp MainEntry        Um,Mx,Xn,Yp
modeTableUmMxXnYp   ModeTableMacro   Um,Mx,Xn,Yp

doMainEntryUmMxXnYn MainEntry        Um,Mx,Xn,Yn
modeTableUmMxXnYn   ModeTableMacro   Um,Mx,Xn,Yn

doMainEntryUmMyXpYp MainEntry        Um,My,Xp,Yp
modeTableUmMyXpYp   ModeTableMacro   Um,My,Xp,Yp

doMainEntryUmMyXpYn MainEntry        Um,My,Xp,Yn
modeTableUmMyXpYn   ModeTableMacro   Um,My,Xp,Yn

doMainEntryUmMyXnYp MainEntry        Um,My,Xn,Yp
modeTableUmMyXnYp   ModeTableMacro   Um,My,Xn,Yp

doMainEntryUmMyXnYn MainEntry        Um,My,Xn,Yn
modeTableUmMyXnYn   ModeTableMacro   Um,My,Xn,Yn

doMainEntryMsMxXpYp MainEntry        Ms,Mx,Xp,Yp
modeTableMsMxXpYp   ModeTableMacro   Ms,Mx,Xp,Yp

doMainEntryMsMxXpYn MainEntry        Ms,Mx,Xp,Yn
modeTableMsMxXpYn   ModeTableMacro   Ms,Mx,Xp,Yn

doMainEntryMsMxXnYp MainEntry        Ms,Mx,Xn,Yp
modeTableMsMxXnYp   ModeTableMacro   Ms,Mx,Xn,Yp

doMainEntryMsMxXnYn MainEntry        Ms,Mx,Xn,Yn
modeTableMsMxXnYn   ModeTableMacro   Ms,Mx,Xn,Yn

doMainEntryMsMyXpYp MainEntry        Ms,My,Xp,Yp
modeTableMsMyXpYp   ModeTableMacro   Ms,My,Xp,Yp

doMainEntryMsMyXpYn MainEntry        Ms,My,Xp,Yn
modeTableMsMyXpYn   ModeTableMacro   Ms,My,Xp,Yn

doMainEntryMsMyXnYp MainEntry        Ms,My,Xn,Yp
modeTableMsMyXnYp   ModeTableMacro   Ms,My,Xn,Yp

doMainEntryMsMyXnYn MainEntry        Ms,My,Xn,Yn
modeTableMsMyXnYn   ModeTableMacro   Ms,My,Xn,Yn

;-------------------------------------------------------------------
;                                                      End of File
;-------------------------------------------------------------------

        ; If in Helios Mode, send a directive ...
        
        [ heliosMode
        EndModule
        ]

        END
