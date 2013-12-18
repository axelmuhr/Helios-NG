        SUBT Executive JEIDA CARD support                            > locard/s
        ;    Copyright (c) 1991, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; JEIDA ver.4 compliance
        ; ======================
        ;
        ; AB1 specific CARDs should have the following CIS information:
        ;
        ; Level 1:
        ;   Device information structure (multiple areas)
        ;       Speed   currently unused by AB1 - but required by JEIDA v.4
        ;       Type    non-extended types only
        ;       Size    of common memory
        ;   Optional CIS CheckSum structure
	;   Optional Level 1 version structure
	;	Version number major		0x04
	;	Version number minor		0x00
	;	Optional Manufacturer string	ISO 646 IRV string
	;	Optional Product Information	ISO 646 IRV string
	;	Optional Product Information 2	ISO 646 IRV string
	;	Optional Product Information 3	ISO 646 IRV string
        ;
        ; Level 2:      
        ;   Level 2 version structure
        ;       Structure version       0x00
        ;       Compliance Level        0x00
        ;       Data Index              Offset to Level 4 structures
        ;       Reserved field          0x0000
        ;       Vendor specific         ABC - hardware/processor identity
        ;       CIS copies              0x01
	;	Vendor name		"Active Book Company"
	;	CARD information	<format type|program that initialised>
        ;   CARD initialisation date structure (ie. when CIS created)
        ;   Battery replacement date structure (for RAM CARDs only) 
        ;
        ; Level 3:
        ;   Format structure
        ;       Memory-like single partition (level 4 structure)
        ;   Byte order structure
        ;       Little-endian
        ;       Byte 0 is LSB
        ;   Organisation structure for each area
        ;       ABC CARD type
        ;          ROMITEM structure
        ;          Helios Heap
        ;          RAMFS structure
        ;       ABC CARD name
        ;          NULL terminated ASCII CARD name (perverted use of the
	;	   data organisation string)
        ;
        ; Level 4:
        ;   Multiple contiguous areas of AB1 types:
        ;       ROM                     ROMitem structure
        ;       FlashEPROM              ROMitem structure
        ;       RAM (volatile)          Helios Heap
        ;       RAM (non-volatile)      RAMFS structure
        ;       
        ;
        ; NOTES: FlashEPROM, PROM and EPROM are all treated like masked-ROM.
        ;        IO devices are not currently supported.
        ;        DRAM is not yet supported by the hardware specification (ie.
        ;        use of the "refresh signal).
        ;        Battery presence/failure is present (for selecting Heap or
        ;        RAMFS usage of RAM CARDs).
        ;
        ;        Hopefully FlashEPROM CARDs will contain a small section of
        ;        ROM attribute memory, otherwise sizing FlashEPROM CARDs will
        ;        be a nightmare.
	;
	;	 If the level 2 vendor specific bytes (processor identity)
	;	 contains an unknown value, code will not be executed from
	;	 the CARD.
        ; ---------------------------------------------------------------------
        ; Default CARD event handler. This is called by the microController
        ; IRQ handler when a CARD bay door open/close event occurs.
        ;
        ; Read the comments in "ABClib.s" for a fuller description of the
        ; CARD handler sub-system.
        ;
defaultCARDHandler
        ; in:   SVC mode; IRQs disabled; FIQs undefined
        ;       r0  = 0=OPENED (removal); -1=CLOSED (insertion)
        ;       r14 = current process return address
        ;       r13 = FD stack (containing original svc_r14 and r0)
        ; out:  SVC mode; IRQs disabled; FIQs undefined
        ;       r14 = current process return address
        ;       r13 = FD stack (containing original svc_r14)
        ; NOTE: IRQs should not be enabled during this call at the moment.
        ;
        ; THIS COULD TAKE A VERY LONG TIME WITH IRQs DISABLED
        ; WE PROBABLY NEED THE INTERRUPTABLE SVC THREADS IF THE SYSTEM
        ; IS GOING TO HAVE ANY BANDWIDTH LEFT.
        ;
        ; This function calls the relevant registered CARD handlers
        ; (as defined using "CardDefineEventHandler").
	;
        ; This handler is called after either of two events:
        ;
        TEQ     r0,#&00000000
        BNE     CARDdoor_closed
CARDdoor_open
        ; CASE 1 = CARD BAY DOOR OPENED (entry r0 = 0)
        ; --------------------------------------------
        ; At this point we do not know if the user is going to insert or
        ; remove a CARD. We should immediately start emitting a noticeable
        ; tone to attract the users attention to the fact that the door is
        ; open. We then need to call ALL the CARD handlers with remove
        ; events for ALL the slots. These handlers are exactly like direct
        ; IRQ handlers and should execute quickly. Ideally their only function
        ; will be to signal suitable semaphores. The processes waiting on
        ; these semaphores should de-link any code/data that they are
        ; referencing on the relevant CARD. They should then call either the
        ; function "CardAcceptExtraction" to notify the system of successful
        ; completion, or "CardRefuseExtraction" after displaying a suitable
        ; screen message to notify failure. IF THE USER REMOVEs A CARD WHILE
        ; THE TONE IS PRESENT THE SYSTEM IS COMPROMISED AND A FULL RESET WILL
        ; PROBABLY BE REQUIRED.
        ;

        LDMFD   sp!,{r0}                                ; recover entry r0
        STMFD   sp!,{r0,r1,r2,r3,r4,r5,r9,r10,r11,r12,lk}

        !       0,"TODO: Start tone generation"

        MOV     r4,#loc_CARD1                   ; first CARD slot to look at
nextCARDslot_open
        CMP     r4,#(loc_limit + 1)             ; all the CARDs checked
        LDMCSFD sp!,{r0,r1,r2,r3,r4,r5,r9,r10,r11,r12,lk}
        BCS     Return_From_IRQ                 ; end Exit this handler thread

        ; Verify CARD presence in this slot with the hardware.
        ; Call verifyCARD to check that the CARD is formatted correctly
        ; and return the bitmask of area types on the CARD.
        !       0,"TODO: CARD presence and verification to be done"

        ; "ExecRoot_cardevents" references the start of the handler chain.
        LDR     r5,=ROOT_start
        LDR     r5,[r5,#ExecRoot_cardevents]
nextCARDhandler_open
        TEQ     r5,#&00000000                   ; any handlers left?
        BEQ     nextCARDslot_open               ; and go on to the next slot
        ; If no handler is defined then there are no CARD users, and hence
        ; it doesn't matter if the door is open or closed. However, if a
        ; handler is attached and the door is open, we should tell the user
        ; to close the door so that the CARDs can be installed (This case is
        ; handled by the higher level handler attach routines).
        ;
        ; r4 = slot number we are currently dealing with
        ; r5 = "CardEvent *" of first handler

        ; Call handler function with:
        ; a1:r0     insert/removal state                        (entry r0)
        ; a2:r1     slot number dealing with
        ; a3:r2     bitmask of CARD type                        (derived)
        ; dp:r9     loaded from (CardEvent *) structure
        ; sl:r10    as current
        ; fp:r11    frame-pointer (&00000000)
        ; ip:r12    undefined
        ; sp:r13    as current
        MOV     r0,#&00000000                   ; remove event
        MOV     r1,r5                           ; CARD slot
        LDR     r2,[r5,#CardEvent_Type]         ; type this handler wants

        LDR     dp,[r5,#CardEvent_ModTab]       ; module table
        MOV     fp,#&00000000                   ; empty frame-pointer
        LDR     ip,[r5,#CardEvent_Handler]      ; handler address
        MOV     lk,pc                           ; remember return address
        MOV     pc,ip                           ; and call handler

        ; and move onto the next registered handler
        LDR     r5,[r5,#CardEvent_Node_Next]
        B       nextCARDhandler_open

noCARDname      =       "no_card_name",&00
        ALIGN

        ; ---------------------------------------------------------------------

CARDdoor_closed
        ; CASE 2 = CARD BAY DOOR CLOSED (entry r0 = -1)
        ; ---------------------------------------------
        ; We can immediately turn off the warning tone (if present), and then
        ; call all the CARD handlers with insert events for the slots
        ; containing CARDs of the correct type. We should then exit
        ; immediately. (See comments attached to "RefuseCardExtraction"
        ; for special case). The CARD hardware and contents are memory mapped.
        ; If the hardware marks a CARD as present, a check for the CARD/ROM
        ; header will take place. If there is no header, then it will be
        ; assumed to be a RAM CARD. At this point, auto-sizing of the RAM
        ; should occur, with a suitable header being written into the CARD.
        ; Unless there is a hardware key, it will not be possible to tell the
        ; difference between volatile and non-volatile RAM CARDs.

        ; Search all slots, calling the handlers
        LDMFD   sp!,{r0}                                ; recover entry r0
        STMFD   sp!,{r0,r1,r2,r3,r4,r5,r9,r10,r11,r12,lk}

        !       0,"TODO: turn off tone generator"

        MOV     r4,#loc_CARD1                   ; first CARD slot to look at
nextCARDslot_closed
        CMP     r4,#(loc_limit + 1)             ; all the CARDs checked
        LDMCSFD sp!,{r0,r1,r2,r3,r4,r5,r9,r10,r11,r12,lk}
        BCS     Return_From_IRQ                 ; end Exit this handler thread

        ; Verify CARD presence in this slot with the hardware.
        ; Call verifyCARD to check that the CARD is formatted correctly
        ; and return the bitmask of area types on the CARD.
        !       0,"TODO: CARD presence and verification to be done"

        ; "ExecRoot_cardevents" references the start of the handler chain.
        LDR     r5,=ROOT_start
        LDR     r5,[r5,#ExecRoot_cardevents]
nextCARDhandler_closed
        TEQ     r5,#&00000000                   ; any handlers left?
        BEQ     nextCARDslot_closed             ; and go on to the next slot

        ; Check if this handler needs to be called for this CARD type.
        ; NO - then goto "nextCARDhandler_closed"

        ; Call handler function with:
        ; a1:r0     insert/removal state                        (entry r0)
        ; a2:r1     slot number dealing with
        ; a3:r2     bitmask of CARD type                        (derived)
        ; dp:r9     loaded from (CardEvent *) structure
        ; sl:r10    as current
        ; fp:r11    frame-pointer (&00000000)
        ; ip:r12    undefined
        ; sp:r13    as current
        MOV     r0,#&FFFFFFFF                   ; insert CARD
        MOV     r1,r5                           ; CARD slot
	; r2 = CARD type
        LDR     dp,[r5,#CardEvent_ModTab]       ; module table
        MOV     fp,#&00000000                   ; empty frame-pointer
        LDR     ip,[r5,#CardEvent_Handler]      ; handler address
        MOV     lk,pc                           ; remember return address
        MOV     pc,ip                           ; and call handler
        ; and move onto the next registered handler
        LDR     r5,[r5,#CardEvent_Node_Next]
        B       nextCARDhandler_closed

        ; ---------------------------------------------------------------------
        ; PC CARD access functions (see "include/abcARM/asm/PCcard.h" for full
        ; description of PC CARD layout).
        ; In the following AREA refers to a contiguous block of mapped memory
        ; of a particular type. 
        ;
        ; NOTE: The hardware is constructed such that the attribute and
        ;       common memory share the same address space. A hardware toggle
        ;       must be used to switch between the memory sections. The
        ;       attribute memory should only ever be switched-in within the
        ;       following functions (this requires that these functions
        ;       execute with IRQs disabled or be made hi-priority (since we
        ;       cannot allow other threads to be activated)).

		GBLL	carddebug
carddebug	SETL	{FALSE}		; Output debugging messages

        [       (activebook)
        ; ---------------------------------------------------------------------
	; This code should possibly be moved into one of the "loswiX.s" files
	; (Since that is where all the other SWI handlers live).
local_VerifyCARD
	STMFD	sp!,{r11,r12}		; work registers
	; This entry assumes IRQs are already disabled
	B	over_VerifyCARDentry	; jump over the SWI entry code
code_exec_VerifyCARD
        ; in:   r0  = CARD slot number
	;	r11 = undefined (work register)
	;	r12 = undefined (work register)
	;	r13 = FD stack (containing entry r11 and r12)
	;	r14 = callers return address
        ; out:  V clear : r0 = bitmask of AREA types within the CARD
        ;                 r1 = number of AREAs in CARD
        ;       V set   : r0 = "CARDerr" error code
        ;                 r1 = undefined
        ;
        ; This function can return:
        ;       CARDerr_badslot		- invalid slot number
	;	CARDerr_nocard		- no CARD present in the slot
        ;       CARDerr_badformat	- invalid CIS
        ;       CARDerr_badsum		- failed checksum on CIS
        ;
        ; This code is NOT interested in CARD hardware information (devices,
        ; speed, programming etc.) only in spotting valid JEIDA ver.4 CARDs and
        ; discovering the CARD AREAs.

	; Disable IRQs (or much better, make hi-priority).
	; This is required since we do not want any other threads accessing
	; the CARD memory space whilst we have the attribute memory mapped
	; in.
	MOV	r12,lk			; preserve "lk" register
	BL	local_disableIRQs	; disable IRQs
	MOV	lk,r12			; recover "lk" register

over_VerifyCARDentry
        CMP     r0,#&01                 ; 0 is an invalid slot number
        RSBGES  r1,r0,#CARD_limit       ; and check against the upper limit
        MOVLT   r0,#CARDerr_badslot     ; invalid CARD slot number
	LDMLTFD	sp!,{r11,r12}		; recover work registers
        ORRLTS  pc,lk,#Vbit             ; return with V set

        STMFD   sp!,{r2,r3,r4}		; preserve work registers

	; At the moment the hardware only has support for controlling a single
	; CARD slot (ie. CARD detect, WPS state, voltage level, etc.). This
	; code may require changing in the future.
	MOV	r3,#CONTROL_base	; base of hardware control registers
	LDRB	r1,[r3,#STATUS_reg]	; various status flags
	TST	r1,#STATUS_MCD		; CARD detection (active low)
        MOVNE   r0,#CARDerr_nocard      ; CARD not present in slot
        LDMNEFD sp!,{r2,r3,r4,r11,r12}	; recover work registers
        ORRNES  pc,lk,#Vbit             ; and return V set

        ; We have validated the CARD slot number range above
        ADRL    r2,(CARD_address_table - word)
        LDR     r2,[r2,r0,LSL #2]       ; r2 = base address of CARD slot r0
	[	(carddebug)
	STMFD	sp!,{r0,lk}
	ADR	r0,ccctxt
	BL	local_Output
	MOV	r0,r2
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	cccovr
ccctxt	=	"Card base address = &",&00
	ALIGN
cccovr
	]

	; We have also checked that a CARD is present in this slot
        MOV     r0,#&00000000           ; bitmask of CARD types
        MOV     r1,#&00000000           ; number of AREAs in the CARD

	; Switch to attribute memory: Note: we can update the soft-copy and
	; hardware register directly since we are currently executing with
	; IRQs disabled.
	LDR	r11,=hardware_regs	; base of soft-copies
	LDRB	r4,[r11,#Control_data]	; control register soft-copy
	ORR	r4,r4,#CONTROL_MCR	; bit set to access attribute memory
	STRB	r4,[r11,#Control_data]	; updating the soft-copy
	STRB	r4,[r3,#CONTROL_reg]	; and the TRUE copy

        ; A "CISTPL_DEVICE" structure MUST be the first descriptor in the
        ; attribute memory. This contains a "Device Info" structure for
        ; each distinct AREA within a CARD.
        LDRB    r3,[r2],#&02            ; "even" bytes only in attribute memory
        TEQ     r3,#CISTPL_DEVICE       ; structure we are looking for
        MOVNE   r0,#CARDerr_badformat   ; invalid CARD format
        LDMNEFD sp!,{r2,r3,r4,r11,r12}	; recover work registers
        ORRNES  pc,lk,#Vbit             ; and return V set

        LDRB    r3,[r2],#&02            ; load the link to the next structure
        TEQ     r3,#&FF                 ; check for CIS chain end
        ADDNE   r12,r2,r3,LSL #1        ; address the next CIS entry
        MOVEQ   r12,#&00000000          ; no next CIS entry
verifyCARD_loop
	[	(carddebug)
	STMFD	sp!,{r0,lk}
	ADR	r0,eeetxt
	BL	local_Output
	MOV	r0,r12
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	eeeovr
eeetxt	=	"Link to address &",&00
	ALIGN
eeeovr
	]

        CMP     r2,r12                  ; check for the end of this structure
        BCS     verifyCARD_loop_completed

        LDRB    r4,[r2],#&02            ; load the DeviceID byte
        TEQ     r4,#&FF                 ; Device Info list terminator
        BEQ     verifyCARD_loop_completed

	[	(carddebug)
	STMFD	sp!,{r0,lk}
	ADR	r0,dddtxt
	BL	local_Output
	MOV	r0,r4
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	dddovr
dddtxt	=	"DeviceTypeSpeed byte = &",&00
	ALIGN
dddovr
	]

        ; r4 = Device ID bitfield
        ; Even though we are not interested in the speed information, we must
        ; check for speed extension bytes (and step over them).
        AND     r11,r4,#DeviceSpeed_mask
        [       (DeviceSpeed_shift <> 0)        ; in-case the spec. changes
        MOV     r11,r11,LSR #DeviceSpeed_shift
        ]
        TEQ     r11,#DS_Extend          ; check for speed extension byte
        BNE     verifyCARD_devicetype
        ; load DeviceSpeed extension bytes
verifyCARD_devicespeed_loop
        CMP     r2,r12                  ; check for the end of this structure
        BCS     verifyCARD_loop_completed
        LDRB    r11,[r2],#&02
        TST     r11,#SpeedExtend_bit     
        BNE     verifyCARD_devicespeed_loop     ; another DeviceSpeed byte
        ; and fall through to process the device type
verifyCARD_devicetype
        [       (DeviceType_shift <> 0)	; in-case the specification changes
        MOV     r11,r4,LSR #DeviceType_shift
        ]
        AND     r11,r11,#DeviceType_mask
        TEQ     r11,#DT_EXTEND
        BNE     verifyCARD_devicetype_process
        ; Step over DeviceType extension bytes. We ignore unrecognised types
        ; (ie. do not set any bits in the bitmask), but we must still count
        ; them.
verifyCARD_devicetype_loop
        CMP     r2,r12                  ; check for the end of this structure
        BCS     verifyCARD_loop_completed

        LDRB    r11,[r2],#&02
        TST     r11,#TypeExtend_bit      
        BNE     verifyCARD_devicetype_loop      ; another DeviceType byte

        MOV     r11,#DT_Null            ; we do not know the type of this area
        ; and fall through to process the device type
verifyCARD_devicetype_process
        ; set the relevant bit in the bitmask (apart from DT_Null)
        TEQ     r11,#DT_Null            ; since we don't know its type
        MOVNE   r3,#&01                 ; single bit
        ORRNE   r0,r0,r3,LSL r11        ; r0 = r0 | (1 << r11)
verifyCARD_devicesize
        CMP     r2,r12                  ; check for the end of this structure
        BCS     verifyCARD_loop_completed
        LDRB    r4,[r2],#&02            ; load the DeviceSize byte
        ; r4 = Device Size bitfield (which we don't care about for the moment)

        ADD     r1,r1,#&01              ; increment the area count
        B       verifyCARD_loop         ; and go around for the next area

verifyCARD_loop_completed
        MOVS    r2,r12                  ; address the next CIS entry
        LDMEQFD sp!,{r2,r3,r4,r11,r12}	; no more CIS entries
        BICEQS  pc,lk,#Vbit             ; return with V clear

        [       {FALSE}
        ; Check the rest of the CIS chain.
        ; Switch to "common" memory if a suitable LONG_LINK is found.
        ; If there is a "CISTPL_CHECKSUM" structure then validate.
        ... todo ...
        ]
	!	0,"TODO: switch CARD back to common memory"

        LDMFD   sp!,{r2,r3,r4,r11,r12}	; recover work registers
        BICS    pc,lk,#Vbit             ; return with V clear

	; ---------------------------------------------------------------------
	; The internal FlashEPROM address is held in the word before the
	; normal table start, since its ID is not in the CARD slot range.

	&	FLASH_base		; start of internal FlashEPROM image
CARD_address_table
	&	CARD_base		; start of CARD_size area for slot 1
CARD_address_table_end
        ASSERT  (((CARD_address_table_end-CARD_address_table)/4) = CARD_limit)

        ; ---------------------------------------------------------------------
	|	; middle (activebook)
        ; ---------------------------------------------------------------------
	; We only provide CARD support on true Active Book hardware
code_exec_VerifyCARD
	MOV	r0,#CARDerr_badformat
	LDMFD	sp!,{r11,r12}		; recover work registers
	ORRS	pc,lk,#Vbit		; return with V set
        ]       ; (activebook)

        ; ---------------------------------------------------------------------

	[	(activebook)
	; Another SWI that should possibly be moved into a "loswiX.s" file.
local_CARDAreaInfo
	STMFD	sp!,{r11,r12}		; work registers
	; This entry assumes IRQs are already disabled
	B	over_CARDAreaInfo_entry	; jump over the SWI entry code
code_exec_CARDAreaInfo
        ; in:   r0  = CARD slot number
	;	r1  = AREA number to examine
	;	r11 = undefined (work register)
	;	r12 = undefined (work register)
	;	r13 = FD stack (containing entry r11 and r12)
	;	r14 = callers return address
        ; out:  V clear : r0 = bitmask of AREA type (single bit)
        ;                 r1 = size of the AREA in bytes
	;		  r2 = base address of the AREA
        ;       V set   : r0 = "CARDerr" error code
        ;                 r1 = undefined
	;		  r2 = undefined
        ;
        ; This function can return:
        ;       CARDerr_badslot		- invalid slot number
	;	CARDerr_nocard		- no CARD present in the slot
        ;       CARDerr_badformat	- invalid CIS
        ;       CARDerr_badsum		- failed checksum on CIS
        ;

	[	(carddebug)
	STMFD	sp!,{r0,lk}
	ADR	r0,CAItxta
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	ADR	r0,CAItxtb
	BL	local_Output
	MOV	r0,r1
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	CAIovr
CAItxta	=	"exec_CARDAreaInfo: r0 (slot) = &",&00
CAItxtb	=	" r1 (area) = &",&00
	ALIGN
CAIovr
	]

	; Disable IRQs (or much better, make hi-priority).
	; This is required since we do not want any other threads accessing
	; the CARD memory space whilst we have the attribute memory mapped
	; in.
	MOV	r12,lk			; preserve "lk" register
	BL	local_disableIRQs	; disable IRQs
	MOV	lk,r12			; recover "lk" register

over_CARDAreaInfo_entry

        CMP     r0,#&01                 ; 0 is an invalid slot number
        RSBGES  r12,r0,#CARD_limit      ; and check against the upper limit
        MOVLT   r0,#CARDerr_badslot     ; invalid CARD slot number
	LDMLTFD	sp!,{r11,r12}		; recover work registers
        ORRLTS  pc,lk,#Vbit             ; return with V set

        STMFD   sp!,{r3,r4}		; preserve work registers

	; At the moment the hardware only has support for controlling a single
	; CARD slot (ie. CARD detect, WPS state, voltage level, etc.). This
	; code may require changing in the future.
	MOV	r3,#CONTROL_base	; base of hardware control registers
	LDRB	r11,[r3,#STATUS_reg]	; various status flags
	TST	r11,#STATUS_MCD		; CARD detection (active low)
        MOVNE   r0,#CARDerr_nocard      ; CARD not present in slot
        LDMNEFD sp!,{r3,r4,r11,r12}	; recover work registers
        ORRNES  pc,lk,#Vbit             ; and return V set

        ; We have validated the CARD slot number range above
        ADRL    r4,(CARD_address_table - word)
        LDR     r4,[r4,r0,LSL #2]       ; r4 = base address of CARD slot r0
	; We have also checked that a CARD is present in this slot
	MOV	r0,#1			; number of AREA reached
					; r1 = number of AREA we want

	; Switch to attribute memory: Note: we can update the soft-copy and
	; hardware register directly since we are currently executing with
	; IRQs disabled.
	LDR	r11,=hardware_regs	; base of soft-copies
	LDRB	r2,[r11,#Control_data]	; control register soft-copy
	ORR	r2,r2,#CONTROL_MCR	; bit set to access attribute memory
	STRB	r2,[r11,#Control_data]	; updating the soft-copy
	STRB	r2,[r3,#CONTROL_reg]	; and the TRUE copy

        ; A "CISTPL_DEVICE" structure MUST be the first descriptor in the
        ; attribute memory. This contains a "Device Info" structure for
        ; each distinct AREA within a CARD.
        LDRB    r3,[r4],#&02            ; "even" bytes only in attribute memory
        TEQ     r3,#CISTPL_DEVICE       ; structure we are looking for
        MOVNE   r0,#CARDerr_badformat   ; invalid CARD format
        LDMNEFD sp!,{r3,r4,r11,r12}	; recover work registers
        ORRNES  pc,lk,#Vbit             ; and return V set

        LDRB    r3,[r4],#&02            ; load the link to the next structure
        TEQ     r3,#&FF                 ; check for CIS chain end
        ADDNE   r12,r4,r3,LSL #1        ; address the next CIS entry
        MOVEQ   r12,#&00000000          ; no next CIS entry
AreaInfo_loop				; process the CISTPL_DEVICE structure
        CMP     r4,r12                  ; check for the end of this structure
        BCS     AreaInfo_skip_loop_completed

        LDRB    r2,[r4],#&02            ; load the DeviceID byte
        TEQ     r2,#&FF                 ; Device Info list terminator
        BEQ     AreaInfo_loop_completed

	; r0 = current AREA number
	; r1 = AREA we are looking for
        ; r2 = Device ID bitfield

	TEQ	r0,r1			; is this the AREA we are interested in
	BEQ	AreaInfo_process
AreaInfo_skip
        ; Even though we are not interested in the speed information, we must
        ; check for speed extension bytes (and step over them).
        AND     r11,r2,#DeviceSpeed_mask
        [       (DeviceSpeed_shift <> 0)        ; in-case the spec. changes
        MOV     r11,r11,LSR #DeviceSpeed_shift
        ]
        TEQ     r11,#DS_Extend          ; check for speed extension byte
        BNE     AreaInfo_devicetype_skip
        ; load DeviceSpeed extension bytes
AreaInfo_devicespeed_skip_loop
        CMP     r4,r12                  ; check for the end of this structure
        BCS     AreaInfo_skip_loop_completed

        LDRB    r11,[r4],#&02
        TST     r11,#SpeedExtend_bit     
        BNE     AreaInfo_devicespeed_skip_loop	; another DeviceSpeed byte
AreaInfo_devicetype_skip
        [       (DeviceType_shift <> 0)	; in-case the specification changes
        MOV     r11,r2,LSR #DeviceType_shift
        ]
        AND     r11,r11,#DeviceType_mask
        TEQ     r11,#DT_EXTEND
        BNE     AreaInfo_devicetype_skip_over
        ; Step over DeviceType extension bytes. We ignore unrecognised types
        ; (ie. do not set any bits in the bitmask), but we must still count
        ; them.
AreaInfo_devicetype_skip_loop
        CMP     r4,r12                  ; check for the end of this structure
        BCS     AreaInfo_skip_loop_completed

        LDRB    r11,[r4],#&02
        TST     r11,#TypeExtend_bit      
        BNE     AreaInfo_devicetype_skip_loop	; another DeviceType byte
AreaInfo_devicetype_skip_over
        CMP     r4,r12                  ; check for the end of this structure
        BCS     AreaInfo_skip_loop_completed

        LDRB    r2,[r4],#&02            ; load the DeviceSize byte
        ADD     r0,r0,#&01              ; increment the AREA count
        B       AreaInfo_loop           ; and go around for the next area

AreaInfo_skip_loop_completed
	; The CISTPL_DEVICE structure has ended prematurely
	MOV	r0,#CARDerr_badformat	; bad CIS data
	LDMFD	sp!,{r3,r4,r11,r12}
        ORRS    pc,lk,#Vbit             ; return with V set

AreaInfo_loop_completed
	; The CISTPL_DEVICE structure has ended without us finding our AREA
	MOV	r0,#CARDerr_badarea	; CIS AREA not found
	LDMFD	sp!,{r3,r4,r11,r12}
        ORRS    pc,lk,#Vbit             ; return with V set

AreaInfo_process			; process the desired AREA information
	; r0  = AREA number count
	; r1  = AREA number we are looking for (should be same as r0)
	; r2  = workspace (undefined)
	; r3  = TPL_LINK value for the CISTPL_DEVICE structure
	; r4  = current attribute memory address (next byte)
	; r11 = workspace (undefined)
	; r12 = calculated end address of the CISTPL_DEVICE structure

	[	(carddebug)
	MOV	r11,lk
	ADR	r0,CAItxt2
	BL	local_Output
	MOV	lk,r11
	B	CAIovr2
CAItxt2	=	"AreaInfo_process: AREA numbers matched",&0A,&00
	ALIGN
CAIovr2
	]

        ; Even though we are not interested in the speed information, we must
        ; check for speed extension bytes (and step over them).
        AND     r11,r2,#DeviceSpeed_mask
        [       (DeviceSpeed_shift <> 0)        ; in-case the spec. changes
        MOV     r11,r11,LSR #DeviceSpeed_shift
        ]
        TEQ     r11,#DS_Extend          ; check for speed extension byte
        BNE     AreaInfo_devicetype_process
AreaInfo_devicespeed_loop
        CMP     r4,r12                  ; check for the end of this structure
        BCS     AreaInfo_skip_loop_completed

        LDRB    r11,[r4],#&02
        TST     r11,#SpeedExtend_bit     
        BNE     AreaInfo_devicespeed_loop	; another DeviceSpeed byte
AreaInfo_devicetype_process
        [       (DeviceType_shift <> 0)
        MOV     r11,r2,LSR #DeviceType_shift
        ]
        AND     r11,r11,#DeviceType_mask
        TEQ     r11,#DT_EXTEND
        BNE     AreaInfo_devicetype_found
        ; Step over DeviceType extension bytes. We ignore unrecognised types
        ; (ie. do not set any bits in the bitmask), but we must still count
        ; them.
AreaInfo_devicetype_loop
        CMP     r4,r12                  ; check for the end of this structure
        BCS     AreaInfo_skip_loop_completed

        LDRB    r11,[r4],#&02
        TST     r11,#TypeExtend_bit      
        BNE     AreaInfo_devicetype_loop	; another DeviceType byte
AreaInfo_devicetype_found
        ; set the relevant bit in the bitmask (apart from DT_Null)
        TEQ     r11,#DT_Null            ; since we don't know its type
        MOVNE   r3,#&01                 ; single bit
	MOVNE	r0,r3,LSL r11		; in the correct mask position

        CMP     r4,r12                  ; check for the end of this structure
        BCS     AreaInfo_skip_loop_completed

        LDRB    r11,[r4],#&02            ; load the DeviceSize byte
	AND	r1,r11,#SizeMantissa_mask		; r1 = MT
	ADD	r1,r1,#(1 :SHL: SizeMantissa_shift)	; r1 = MT + 1
	ASSERT	(SizeMantissa_shift <= 9)		; for following
	MOV	r1,r1,LSL #(9 - SizeMantissa_shift)	; r1 = (MT + 1) * 512
	AND	r11,r11,#SizeExponent_mask		; r11 = EXP
	ASSERT	(SizeExponent_shift <= 1)		; for following
	MOV	r11,r11,LSL #(1 - SizeExponent_shift)	; r11 = EXP * 2
	MOV	r3,#&01
	MOV	r11,r3,LSL r11
	MUL	r1,r11,r1
	; -- cheat on the data AREA base address --
	; -- it should actually be held in information in the CIS --
	BIC	r1,r1,#(word - 1)	; word-align the AREA size (down)
	ADD	r2,r12,#(word - 1)	; word-align the AREA address (up)
	BIC	r2,r2,#(word - 1)

	[	{FALSE}
	... this code needs to take into account the CIS structure ...
	... when calculating the AREA start and size               ...
	]

	!	0,"TODO: switch CARD back to common memory"
	; r0 = bitmask of AREA type (single bit)
	; r1 = AREA size in bytes
	; r2 = AREA base address
        LDMFD   sp!,{r3,r4,r11,r12}	; recover work registers
        BICS    pc,lk,#Vbit             ; return with V clear

        ; ---------------------------------------------------------------------
	|	; middle (activebook)
        ; ---------------------------------------------------------------------
	; We only provide CARD support on true Active Book hardware
code_exec_CARDAreaInfo
	MOV	r0,#CARDerr_badformat
	LDMFD	sp!,{r11,r12}		; recover work registers
	ORRS	pc,lk,#Vbit		; return with V set
	]	; (activebook)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	LNK	loswi1.s
