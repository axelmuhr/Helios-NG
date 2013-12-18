        SUBT    Helios Kernel Microlink structures and constants  > microlink/s
        ;       (c) 1990, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; --------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; started:      901219  Brian Knight
	;		910207  Brian Knight Extended msgs now up to 64 bytes
        ;
        ; Microlink structures and constants
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        !       0,"Including microlink.s"
                GBLL    microlink_s
microlink_s     SETL    {TRUE}

        ; ---------------------------------------------------------------------
	; Structure used to hold reception requests

        struct          "ML_RxReq"
        struct_word     "Buf"           ; buffer address
        struct_word     "MsgType"       ; type of message to be received
	struct_word	"Handle"	; handle passed back to client
        struct_word     "TimeLeft"	; amount of timeout left (us)
					; -1 for infinite timeout
        struct_word     "Satisfied"     ; set if message received
	struct_word	"SaveState"	; ptr to SaveState to be resumed
					; (0 if none)
        struct_end

        ; ---------------------------------------------------------------------
	; Structure used to describe message handler functions
	; This is the C structure ML_MsgHandler in ABClib.h, so these
	; offsets could be produced by `genhdr'.

        struct          "ML_MsgHdlr"
        struct_word     "MsgType"	; type of message to be handled
        struct_word     "Func"          ; function to be called
        struct_word     "Arg"		; argument to be passed to function
        struct_word     "ModTab"        ; module table pointer for func call
	struct_word	"Next"		; next message handler in list
        struct_end

        ; ---------------------------------------------------------------------
	; Fields in microlink message header byte

MLHdr_Long	*	&80		; Bit set for long/extended message
MLHdr_LenCode	*	&03		; Length code in long/ext header
MLHdr_LTypeMask	*	&FC		; Format bit & type field for long msg
MLHdr_STypeMask	*	&FC		; Format bit & type field for short msg

	; Message header bytes needed in break protocol

MLHdr_MSQbreak		*	&18	; Msg from microcontroller after break
MLHdr_ASYbreak_local	*	&19	; Hercules' response if caused break
MLHdr_ASYbreak_remote	*	&18	; Hercules' response if didn't cause it

	; Fields in second header byte of extended messages

; MLExt_LenMask	*	&1F		; Length field
MLExt_LenMask	*	&3F		; Length field (for 64 byte msgs)

	; Timeout control

ML_TickInterval	*	8		; Check timeouts every 8 clock ticks
					; (Must be a power of 2)
ML_TickMask	*	(ML_TickInterval - 1)
					; Mask for cheap test of tick count

	; Values of the driver state word (ML_State in execwork.s)

ML_State_Normal		*	0	; Normal running (must be 0 for
					; correct initialisation)
ML_State_GotBreak 	*	1	; Received (hard or soft) break:
					; awaiting MSQbreak message
ML_State_SentBreak	*	2	; Sent break: awaiting MSQbreak msg


	; Other constants

;ML_MaxMsgLen	*	(1 + 1 + 32)	; Total size of longest message
;					; (Header + len + 32 data bytes)
ML_MaxMsgLen	*	(1 + 1 + 64)	; Total size of longest message
					; (Header + len + 64 data bytes)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; Acknowledgments returned from the uController.

MLHdr_Ack	*	&20		; good acknowledgement
MLHdr_NAck	*	&21		; failed acknowledgment

        ; ---------------------------------------------------------------------
	; Message protocol bytes required by the low-level handlers (Executive)

	!	0,"TODO: tidy up microlink.s header file"
	; These should appear as manifests (along with all the other
	; allocations) in a suitable header file.

	; ---------------------------------------------------------------------
	; The uController now signifies PowerFail by generated a microlink
	; break.
					; From Hercules to uController
MLHdr_PowerDown	*	&1A		; PowerDown message
					; short message; no data
					; We send this message when we are
					; happy for the uController to remove
					; Hercules power.

	; ---------------------------------------------------------------------
					; From Hercules to uController
MLHdr_LCDctrl	*	&68		; LCD status changing
LCDctrl_on	*	(1 :SHL: 0)	; LCD going on  (LCD refresh)
LCDctrl_onPWR	*	(1 :SHL: 1)	; PoWeR on reset LCD on message
LCDctrl_onOK	*	(0 :SHL: 1)	; normal LCD on message
					; From uController to Hercules
LCDon_vback	*	(MLHdr_Ack)	; "vback" OK state
LCDon_bad	*	(MLHdr_NAck)	; "vback" failed since last LCD off

					; From Hercules to uController
LCDctrl_off	*	(0 :SHL: 0)	; LCD going off (uController refresh)

	; These messages are sent to the uController. Enabling the
	; LCD after a LCD_on message can occur at any time (since the
	; uController will continue to refresh DRAM), however we
	; should wait for acknowledgment from the uController before
	; disabling the LCD after we have sent a LCD_off message.

	; ---------------------------------------------------------------------


					; From Hercules to uController
MLHdr_VBack	*	&78		; This message allows is to
					; notify the uController of the value
					; we wish the "vback" flag to take (for
					; the next LCDctrl_on acknowledgement).
					; In all normal cases this should be
					; "VBack_ok" since we have powered
					; and taken our desired Reset route.
VBack_ok	*	(0 :SHL: 0)	; LCDctrl_on will return "LCDon_vback"
VBack_failed	*	(1 :SHL: 0)	; LCDctrl_on will return "LCDon_bad"

	; The uController acknowledges receipt of this message with a standard
	; "MLHdr_Ack" message.

	; ---------------------------------------------------------------------
	; **** NOTE: this message name/number/functionality has changed
	; ****       there are now two calls that read/set multiple locations
MLHdr_LineRate	*	&C0		; LCD Line Rate change
					; long message; 1byte
					; single byte parameter giving value
					; to reprogram LCD LineRate register
					; with (should be acknowledged)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF microlink/s
