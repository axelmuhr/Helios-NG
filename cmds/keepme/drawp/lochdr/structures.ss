;-------------------------------------------------------------------
;                                                    structures.ss
;-------------------------------------------------------------------

; This file can be included from any ObjAsm format file using 'GET'
; It contains block-layout directives to describe the layout of
;   structures which are passed between 'C' and machine code
;   functions. The corresponding 'C' file is called 
;   'code_interfaces.h', and it is important that these two files
;   get updated in step.

;-------------------------------------------------------------------
;                                            BlitterControlBlock_t
;-------------------------------------------------------------------

; The corresponding structure for this is called:
;   typedef struct BlitterControlBlock_s { ... } BlitterControlBlock_t ;
; in the file 'lochdr/code_interface.h'

dpMaxBpp        *   8   ; Maximum bits per pixel

              ^   0

; ---------- Graphics context information  -------------------------

              ; Colour and mode control information:
cbCombModes   #  dpMaxBpp*4 ; Combination routine vector
cbCtlRtn      #  4        ; Control routine address
              ; ^ The combination routines vector must come at the
              ;     beginning of the block, this is required
              ;     by the assembley code.

              ; Source pixmap information:
cbSrcBase     #  4        ; Pointer to first row of raw data
cbSrcLast     #  4        ; Pointer to last row of raw data
cbSrcWPV      #  4        ; Words per vector (or line if stippled)
cbSrcWPL      #  4        ; Words per line
cbSrcSizX     #  4        ; X size of source pix-map
cbSrcSizY     #  4        ; Y size of source pix-map
cbSrcOffX     #  4        ; Sub fr des co-ord to get source co-ord
cbSrcOffY     #  4        ; Sub fr des co-ord to get source co-ord

              ; Mask pixmap information:
cbMskBase     #  4        ; Pointer to first row of raw data
cbMskWPV      #  4        ; Words per vector (or line if stippled)
cbMskOffX     #  4        ; Sub fr des co-ord to get source co-ord
cbMskOffY     #  4        ; Sub fr des co-ord to get source co-ord

              ; Destnation pixmap information:
cbDesBase     #  4        ; Pointer to first row of raw data
cbDesBPP      #  4        ; Number of bit-planes
cbDesWPV      #  4        ; Words per vector

; ---------- Rectangle to be copied  ---------------------------------

              ; These are co-ordinates in the destination pix-map
cbDesLftX     #  4        ; Left   X    of rectangle to be copied
cbDesRgtX     #  4        ; Right  X +1 of rectangle to be copied
cbDesTopY     #  4        ; Top    Y    of rectangle to be copied
cbDesBotY     #  4        ; Bottom Y +1 of rectangle to be copied

; ---------- Temporary storage space for use during the operation ---

              ; Alot of this information depends on the orientation
              ;  (left-right versus right-left and top-bottom versus
              ;  bottom-top), and in these cases the labels have been
              ;  given generic names.
              
              ; System storage area:
cbStkPtr      #  4        ; Stack pointer after saving registers

              ; General tiling block information:
cbBlkSizX     #  4        ; Block X size
cbBlkSizY     #  4        ; Block Y size
cbBlkRemX     #  4        ; Amount left to plot : X (columns)
cbBlkRemY     #  4        ; Amount left to plot : Y (rows)
              
              ; Source and destination tiling control information:
cbSrcFstY     #  4        ; Co-ord of fst src row to be plotted ( B-T +1 )*
cbSrcFirst    #  4        ; Addr of first src row to be plotted ( NOT +1 )*
cbDesFirst    #  4        ; Addr of first des row to be plotted ( NOT +1 )*
cbMskFirst    #  4        ; Addr of first msk row to be plotted ( NOT +1 )*
cbSrcIncDec   #  4        ; Source increment/decrement (tertiary routines)
cbDesIncDec   #  4        ; Des    increment/decrement (tertiary routines)
cbSrcSave     #  4        ; Save space for src ptr     (tertiary routines)
cbMskSave     #  4        ; Save space for msk ptr     (tertiary routines)
cbRowCnt      #  4        ; Space for row countdown    (tertiary routines)
cbSrcPtrRld   #  4        ; Used for vertical tiling
cbDesPosX     #  4        ; Start X of next destination column
              ; * The comment ( B-T +1 ) means that in the case of
              ;   plotting from bottom-to-top, this value holds the
              ;   co-ordinate plus 1, the comment ( NOT +1 ) means
              ;   that even when plotting from bottom-to-top, the
              ;   valus held is NOT for the row +1.
              
;---------------- Diagnostics save area

cbRegBlock    #  4*16     ; Saved register set
cbDiagReturn  #  4        ; Return context

;-------------------------------------------------------------------
;                                               LineControlBlock_t
;-------------------------------------------------------------------

; This control block is set-up to control the plotting of lines
;   and points in the destination pixmap:

; By 'non-static' we mean components which should be reloaded from
;   line-to-line whereas the other components can generally be set
;   up at the graphics context stage.

lcMaxDotDash  *   20 ; Maximum size of dot/dash list before one has
                     ;   to be separately calloc'd. (Must be a multiple
                     ;   of four) Keep in step with 'code_interface.h'

              ^   0

; ------ Non static Destination Co-ordinate information ------------

lcDesX        #  4   ; Start X position, destination
lcDesY        #  4   ; Start Y position, destination
lcPoints      #  4   ; Number of points to plot this line
lcNumer       #  4   ; Bresenham numerator

; -------- Non static dot/dash stroke information --------------------

lcStrOff      #  4   ; Stroke offset : Not used by the Assembly code
lcStrLen      #  4   ; Length of first stroke
lcStrSign     #  4   ; Sign of first stroke 0=> even, 1=>odd
lcStrPtr      #  4   ; Pointer into dot/dash list, next stroke length

; ---------------- Destination pixmap information ---------------------

lcDesBase     #  4   ; Raw data base
lcDesWpv      #  4   ; Words per vector
lcDesWpl      #  4   ; Words per line
lcDesDepth    #  4   ; Depth of destination pixmap

; --------------- Mask pixmap infromation  --------------------------

lcMskBase     #  4   ; Raw data base
lcMskWpl      #  4   ; Words per line
lcMskOffX     #  4   ; Subtract this offset vector from the destination
lcMskOffY     #  4   ;  co-ordinates to get the mask co-ordinates

; --- Graphics context and dot/dash stroke information --------------

lcForeground  #  4   ; Foreground colour mode
lcBackground  #  4   ; Background colour mode
lcRatioX      #  4   ; X axis offset (made posotive)
lcRatioY      #  4   ; Y axis offset (made posotive)
lcMainGroup   #  4   ; Group of main routine (not used in assembler)
lcStrBase     #  4   ; Base of dot/dash stroke length array
lcStrPeriod   #  4   ; Total dot/dash period : Not used by Assembly Code
lcStrList     #  lcMaxDotDash ; Place to store dot/dash list if small enough
lcStrAllocd   #  4   ; Whether local list : Not used in Assembler.

; --------------- Other ---------------------------------------------

lcStrAlt      #  4   ; Stroke control scratchpad
lcStrPtrSave  #  4   ; Save-space for stroke pointer
lcFgdSave     #  4   ; Foreground colour save location
lcBgdSave     #  4   ; Background colour save location
lcPlaneSave   #  4   ; Bit-plane countdown save location
lcDesPtrSave  #  4   ; Destination pointer save location
lcDesMskSave  #  4   ; Destination mask save location
lcMskPtrSave  #  4   ; Mask pointer save location
lcMskMskSave  #  4   ; Mask mask save location
lcRegBlock    # 4*16 ; For use by the diagnostics routines
lcDiagReturn  #  4   ; Return point for diagnostics
lcStkPtr      #  4   ; Stack pointer storage space

;-------------------------------------------------------------------
;                                                      End of File
;-------------------------------------------------------------------

        END

