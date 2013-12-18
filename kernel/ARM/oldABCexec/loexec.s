        TTL  ARM Helios Executive                               > loexec/s
        SUBT Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ARM HELIOS Executive
	; --------------------
	;
	;	Reset/Startup initialisation code
	;	System "abort" handlers
	;	Process scheduler
	;	System support SWIs
	;	System interrupt handlers
        ;
        ; ---------------------------------------------------------------------
        ; This file (and those "LNK"ed to) expect certain global variables
        ; to be defined by an external calling file:
        ;
	;	heval		: Produce a version for the Heval prototype
	;	activebook	: Produce a version for the Active Book
	;	hercules	: Produce a version for Hercules based systems
        ;
        ; ---------------------------------------------------------------------
        ;
        !       0,"TODO: Optimise register usage (minimise memory transfers)"
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        GET     listopts.s			; printing options

        ; ---------------------------------------------------------------------

        GET     fixes.s                         ; generic assembly control

        ; ---------------------------------------------------------------------
        ; Machine specific assembly control

		[	((activebook) :LOR: (heval))
hercules	SETL	{TRUE}		; Hercules processor based system
		|	; middle ((activebook) :LOR: (heval))
hercules	SETL	{FALSE}		; Standard ARM2 based system
		]	; EOF ((activebook) :LOR: (heval))

		[	(activebook)
dynlcd		SETL	{TRUE}	; dynamic LCD base addressing
		|	; middle (activebook)
dynlcd		SETL	{FALSE}	; fixed address LCD physical memory
		]	; EOF (activebook)

		[	(activebook)
shutdown	SETL	{TRUE}	; processor shutdown code included
		|
shutdown	SETL	{FALSE}	; shutdown code AB1 specific
		]	; EOF (activebook)

		[	(shutdown)
		; new DMA support include, so we can HALT the processor in IDLE
		[	{TRUE}	; debugging
haltmode	SETL	{FALSE}	; for the moment
	; The screen (LCD) DMA seems to do funny things with this enabled
		|
haltmode	SETL	{TRUE}	; HALT processor in the IdleProcess
		]	; EOF {boolean}
		|	; middle (shutdown)
haltmode	SETL	{FALSE}	; halt code AB1 specific
		]	; EOF (shutdown)

		; The "newpdcode" should only be included if a new uController
		; is available.
		[	((shutdown) :LAND: {FALSE})
		!	0,"**** new PowerDown/Up code included ****"
newpdcode	SETL	{TRUE}		; include the new PowerDown/Up code
		|
newpdcode	SETL	{FALSE}
		]	; EOF ((shutdown) :LAND: {boolean})

	; ---------------------------------------------------------------------
	; Debugging options

		GBLL	debug2		; 2nd link adaptor debugging
		[	(activebook)
debug2		SETL	{TRUE}
		|	; middle (activebook)
		[	(heval)
debug2		SETL	{FALSE}		; ALWAYS FALSE (no 2nd link on HEVAL)
		|	; middle (heval)
		[	(fpmlink)
debug2		SETL	{FALSE}		; 2nd link used for microlink
		|	; middle (fpmlink)
debug2		SETL	{TRUE}		; 2nd link adaptor debugging
		]	; EOF (fpmlink)
		]	; EOF (heval)
		]	; EOF (activebook)

		[	(release)
debug2		SETL	{FALSE}		; no 2nd link debugging for releases
		]

		[	(debug2)
		!	0,"**** ENSURE 2nd link adaptor connected ****"
		]

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        !       0,"Processing loexec.s (Helios Executive)"

        ; ---------------------------------------------------------------------

	GBLS	hwfile
	[	(activebook)
hwfile	SETS	"hardAB1.s"		; Active Book version 1
	|
	[	(heval)
hwfile	SETS	"hardHFP.s"		; HEVAL prototype
FLASH_base	*	ROM2_base	; FlashEPROM position on HEVAL boards
FLASH_size	*	(8 :SHL: 20)	; MaxSize of FlashEPROM on HEVAL boards
	|
hwfile	SETS	"hardABFP.s"		; ARM2 Functional Prototype
	]	; EOF (heval)
	]	; EOF (activebook)

	; ---------------------------------------------------------------------

        GET     basic.s				; information
        GET     arm.s          			; ARM description
        GET     $hwfile				; HW manifests etc.
        GET     exmacros.s     			; Executive MACROs
        GET     structs.s      			; structure MACROs
	GET	microlink.s			; Microlink structs
        GET     exstruct.s     			; Executive structures
	GET	pdstruct.s			; PowerDown data structures
		GBLL	SWItable
SWItable        SETL    {FALSE}                 ; define SWI manifests
        GET     SWI.s         			; SWI allocations
        GET     ROMitems.s     			; ROM structures
        GET     manifest.s     			; constants
	GET	timedate.s			; build time info
	GET	PCcard.s			; CARD information
		GBLL	make_SMT
make_SMT	SETL	{TRUE}			; SMT system build
	GET	module.s			; module structures
        GET     execwork.s                      ; Executive workspace allocs
	GET	simstate.s			; simulator definitions
	GET	error.s				; Helios error numbers

        ; ---------------------------------------------------------------------

	[	(activebook)
	! 0,"**** Generating code for an Active Book ****"
	|
	[	(heval)
	! 0,"**** Generating code for a HEVAL prototype ****"
	|
        ! 0,"**** Generating code for an Active Book functional prototype ****"
	]	; (heval)
	]	; (activebook)

        ; ---------------------------------------------------------------------

	[	(hercules)
	; 100Hz timer initialiser desired
centisecondtick	*	((TIMER_xclk / (100 * TIMER_multiplier)) - 1)
	|	; middle (hercules)
        ; Timer clock is "main system clock / 512".
        ; Therefore timer count for 10ms tick is:
centisecondtick *       (clockfreq / 512 / 100)
	]	; (hercules)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

	[	(hercules)
codeAddr	*	ROM_base	; where the code will reside (in ROM)
	|	; middle (hercules)
codeAddr	*	(ROM_base + (4 :SHL: 20))	; offset in FPs
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; The system ROM image ALWAYS contains a single branch instruction
	; before any ITEM identification code. It DOES NOT contain a valid
  	; JEDIA v4 header.

codeBase
        B       exec_reset		; Processor RESET code

	; ---------------------------------------------------------------------

        GBLS    objname
objname	SETS    "helios/lib/executive"	; where we appear in the ROMFS

	; ---------------------------------------------------------------------

ExecutiveITEM
	ROMITEM	"$objname",,&0010,MakeTime,defaultITEMaccess,,ITEMhdrBRANCH
	B	exec_continue	; entry point after Executive replacement

        ; ---------------------------------------------------------------------
        ; -- Machine RESET ----------------------------------------------------
        ; ---------------------------------------------------------------------
exec_reset
        ; in:   SVC mode; IRQ disabled; FIQ disabled;
        ;       All registers are undefined
        ;
        ; Notes: When we are entered after reset the "software_vectors" are
        ;        undefined. If a fatal error occurs before the vectors are
        ;        written then the software is likely to loop, or disappear
        ;        into the boonies. This would be solved by having hard-wired
        ;        values built into the ROM.

	; =====================================================================
	; =====================================================================
	; =====================================================================
	;
	; RESET options
	; top buttons (assuming touch-pad on left of display):
	;	LEFT	: Enter Helios "shell"
	;	MIDDLE	: Zero RAM (clear RAMFS)
	;	RIGHT	: Ignore PatchEPROM
	;
	; We must provide a scheme for configuring the system entered after
	; RESET. The "shell" entry will be controlled by "init" program and
	; the "preinitrc" and "initrc" scripts. The "Zero RAM" entry can be
	; performed by the kernel before performing any RAMFS recovery.
	; The "Ignore PatchEPROM" is the only operation we are interested in
	; during the system RESET. Since the "Executive" may appear in the
	; FlashEPROM, it will normally over-load the internal ROM version.
	; At the moment the RESET code checks for a FlashEPROM Executive early
	; in the RESET. This should be postponed until we are able to
	; interrogate the keypad button state. However, this has the
	; disadvantage that there is a higher probability of a bug appearing
	; in the system ROM RESET code before a new FlashEPROM version can be
	; started. On a related subject: another key/RESET sequence should
	; possibly be allocated to directly enter a FlashEPROM updating
	; system. This will allow the FlashEPROM to be updated, even when the
	; internal ROM is incapable of starting Helios. This will probably
	; be hardwired to download the data from the serial (or some other
	; SIMPLE mapped external IO device). * more thougths on this required *
	; =====================================================================
	; =====================================================================
	; =====================================================================

        ; The main RAM has physical addresses starting at RAM_base.
        ; Continue execution in ROM (which is always mapped at "ROM_base").

	; If we are a ROM system, then during RESET the complete ROM image is
	; mapped to &00000000. If the PC is within this area, then we assume
	; that we are a ROM system starting. If this is the case, we need to
	; perform a run-time calculated branch into the ROM image.
	; Unfortunately this piece of code will always be non-PIC, since we
	; need to know the base address of the ROM.
	CMP	pc,#ROM_size	; check against the maximum ROM size
	BGE	ROMentry	; PC is outside mapped ROM area, so continue
	; This code should only be executed if we are a ROM system starting up
	ADD	pc,pc,#ROM_base	; add in the ROM offset to the PC (at ROMentry)
	NOP			; simple padding instruction (for pipe-lining)

ROMentry
	; We are now executing at the true Executive address.

	[	(hercules)
	; Map ROM out (mapping everything else in).
	; This code SHOULD be the same as that used to initialise the
	; BTZ vector later in the RESET sequence.
	MOV	r0,#&00000000		; where the vectors live
	ADRL	r1,exec_branch_through_zero
	ADD	r0,r0,#&08		; source = source + 8
	SUB	r1,r1,r3		; offset = destination - source
	MOV	r1,r1,LSR #2		; offset = offset >> 2
	ORR	r1,r1,#(cond_AL :OR: op_b)
	; The following is a nasty constant:
	MOV	r0,#(&0200 - &0004)	; NOTE: &0200 is Hercules page size
	; Trash location &01FC and write the BTZ vector. It is the act of
	; writing over a page boundary that maps the ROM out from &00000000.
	; (The &01FC location could be used as a soft-reset flag).
	STMIA	r0,{r0,r1}		; write over page boundary
					; relies on the FastRAM ghosting
	]	; EOF (hercules)

	; Check FlashEPROM for Executive image. If found then start it up.
	; Note: We have no stack or RAM world at this point so cannot
	;	safely use the SWI calls for interrogating the FlashEPROM
	;	image. We can however use the complete register set.

	!	0,"TODO: FlashEPROM Executive disable switch"
	; In the Hercules version (and even in ROMmed ABFP versions) we
	; should check the switches state to see if we should Execute
	; a FlashEPROM based Executive. This will allow Executive development
	; to continue with a true ROM based system. ie. we need some way of
	; stopping the automatic startup of FlashEPROM Executives, in-case
	; they are duff.

	!	0,"TODO: access internal FlashEPROM during RESET"
	; Code to be written to access the internal FlashEPROM as a PCcard
	; device.

	; At the moment we just treat it like a collection of ITEMs.
	MOV	r1,#FLASH_base

	; r1 = start of the first ITEM in the FlashEPROM
	LDR	r2,=ITEMMagic		; r2 = the ITEM ID word
	; Search for the ITEM in the FlashEPROM
FlashSearch_loop
	LDR	r3,[r1,#ITEMID]		; read the ITEM magic word
	TEQ	r3,r2			; and check against the real value
	BNE	FlashSearch_failed	; ITEM is not in the FlashEPROM
        ; The ITEM magic word has been found, check that we are pointing at
	; a ROM ITEM.
	LDR	r3,[r1,#ITEMExtensions]	; extensions bitmask
	TST	r3,#ITEMhdrROM		; ROM ITEM
	BEQ	FlashSearch_failed	; invalid ITEM (end of chain)

	TST	r3,#ITEMhdrBRANCH	; ITEM has Executive BRANCH
	LDREQ	r3,[r1,#ITEMLength]	; load the length of this ITEM
	ADDEQ	r1,r1,r3		; step over this ITEM
	BEQ	FlashSearch_loop

        ; Check that the names match (identically)
        ADD     r3,r1,#ITEMName		; start of the full name

	; compare names
	; r0 = name we are searching for
	; r3 = name we are to check
	MOV	r6,#&00000000
FlashSearch_namecheck
	LDRB	r4,[r0,r6]
	LDRB	r5,[r3,r6]
	TEQ	r4,r5			; are they the same?
	LDRNE	r4,[r1,#ITEMLength]	; load the length of this ITEM
	ADDNE	r1,r1,r4		; and step onto the next ITEM
	BNE	FlashSearch_loop	; NO - look at the next ITEM
	TEQ	r4,#&00			; have we terminated
	ADDNE	r6,r6,#&01		; step onto the next character
	BNE	FlashSearch_namecheck	; and check the next character
	; -- names matched --
	; r1 = found ITEM pointer
	LDRB	r2,[r1,#ITEMNameLength]	; get ITEM name length
	ADD	r0,r1,#ITEMName		; reference the ROM specific header
	ADD	r0,r0,r2		; add in the length of the name
	ADD	r0,r0,#sizeof_ROMITEMstruct	; step over ROM specific header
	; r0 = address of branch instruction
	ORRS	pc,r0,#(SVCmode :OR: INTflags)	; continue from branch in ITEM
FlashSearch_failed				; Executive ITEM not found
exec_continue					; RESET continue point

	; ---------------------------------------------------------------------
	; -- Executive selected -----------------------------------------------
	; ---------------------------------------------------------------------

	[	(hercules)
	; In the current system "vmain" (the processor power) will be taken
        ; away when in sleep mode. This means that every RESET will set the
	; POR (Power-On RESET) bit. The original plan was to have the processor
	; permanently powered (and hence POR would signify exceptional RESETs).
	[	{FALSE}
	; Check the RESET state
	MOV	r0,#INT_regs		; interrupt status register
	LDR	r1,[r0,#INT_status]
	TST	r1,#INT_POR		; test Power-On RESET bit
	BEQ	soft_reset		; clear, then NOT POR
soft_reset				; at the moment we have no soft startup
	]	; {conditional}
power_on_reset				; processor power applied

	; ----------------- Re-initialise the complete system -----------------
	; NOTE: We should not touch the RAM or the external IO world until we
	;	have setup the BANK timing etc. Also the DRAM will not be
	;	refreshed until we have started the LCD DMAs. At this point
	;	the uController should be providing full DRAM refresh.
	; ---------------------------------------------------------------------

	; Disable the POR flag (by writing to "CLOCK_regs") along with the
	; grayscale and pre-scalers mode information.
	MOV	r0,#CLOCK_regs
	MOV	r1,#(CODEC_ON :OR: TIMER_ON :OR: MLI_ON)
	STR	r1,[r0,#&00]

	; ---------------------------------------------------------------------

	[	(hydra)
	; Initialise HYDRA (eventually this will be performed by the
	; micro-controller on the HYDRA sub-system. On HEVAL boards there
	; is no micro-controller).
	; This code is copied directly from a piece of DFlynn code since I
	; do not have a HYDRA specification.
HYDRA_address	*	(EXTRAM1_base)
HYDRA_data	*	(EXTIO2_base + (2 :SHL: 20))
	MOV	r0,#HYDRA_address
	MOV	r1,#HYDRA_data
hydra_init_loop
	MOV	r2,#&50		; refresh control address
	STRB	r2,[r0,#&00]	; onto HYDRA address bus
	MOV	r3,#&17		; program refresh (hercules DRAM control)
	STRB	r3,[r1,#&00]	; onto HYDRA data bus
	MOV	r3,#&51		; DRAM refresh register address
	STRB	r3,[r0,#&00]
	STRB	r3,[r1,#&00]	; cause refresh => out of standby mode
	STRB	r2,[r0,#&00]	; refresh control address
	MOV	r3,#&10		; HYDRA refresh and hercules access
	STRB	r3,[r1,#&00]	; cause refresh => out of standby mode
	LDRB	r3,[r1,#&00]	; load status
	TST	r3,#&40		; out of standby yet?
	BNE	hydra_init_loop	; NO - then try again
	]	; EOF (hydra)

	; ---------------------------------------------------------------------
	[	(shutdown)
	; Since we wish to keep the soft-copies from the previous session valid
	; until after we have decided wether this is a full or re-start RESET,
	; we must initialise the soft-copies later.
	; NOTE: This introduces a source management problem, since we cannot
	; 	use temporary memory (without having a permanent allocation)
	;	we must initialise the soft-copies with the same values as used
	;	in this code section.
	;
	;	**** ALWAYS UPDATE THE soft-copy INITIALISATION CODE ****
	;	**** IF THE DIRECT INITIALISATION CODE IS CHANGED    ****
	|
	; Load the address of the soft-copies (we cannot write to RAM until
	; the BANK timing has been set (below)).
	LDR	r11,=hardware_regs	; where all the soft-copies are kept
	]	; EOF (shutdown)

	; ---------------------------------------------------------------------
	; Initialise the HW BANK registers (bus width, etc.)
	ADRL	r0,BANK_table			; default values
	MOV	r1,#BANK_regs			; hardware registers
	LDR	r2,[r1,#BANK6_reg]		; current BANK 6 mapping
	AND	r2,r2,#BANK_WMASK		; current BANK BUS width
	ASSERT	(BANK_table_size = (8 * 4))	; 8 registers worth
	LDMIA	r0,{r3,r4,r5,r6,r7,r8,r9,r10}	; load default BANK timings
	ORR	r9,r9,r2			; retain external bus width
	STMIA	r1,{r3,r4,r5,r6,r7,r8,r9,r10}	; write BANK timing
	[	(:LNOT: shutdown)
	; we can now write to the RAM
	ADD	r12,r11,#BANK0_data		; address the soft-copies
	STMIA	r12,{r3,r4,r5,r6,r7,r8,r9,r10}	; and store the soft-copies
	]	; EOF (:LNOT: shutdown)

	; ---------------------------------------------------------------------
	[	(:LNOT: shutdown)
	; This should be set to the value loaded into the register above.
	MOV	r1,#(CODEC_ON :OR: TIMER_ON :OR: MLI_ON)
	STR	r1,[r11,#CLOCK_data]		; clock soft-copy
	]	; EOF (:LNOT: shutdown)

	; ---------------------------------------------------------------------
	; Define the memory map view

	MOV	r0,#MEMMAP_regs			; control register address
	MOV	r1,#(MAPEN_USR :OR: OS_MODE1)	; Enable OS mode 1
	STRB	r1,[r0,#&00]
	[	(:LNOT: shutdown)
	STR	r1,[r11,#MEMMAP_data]		; word write to clear top
	]	; EOF (:LNOT: shutdown)

	; ---------------------------------------------------------------------
	[	(:LNOT: dynlcd)
	; Initialise the DMA registers (All off except LCD DMA (for DRAM
	; refresh)). NOTE: The default RESET state is that the DMA channels
	; are disabled. The LCD DMA transfer is controlled by the LCD ON bits
	; in the register "(LCD_regs + LCD_control)".
	; 
	ADRL	r0,DMA_table			; default values (in table)
	MOV	r1,#DMA_base			; hardware registers
	ASSERT	(DMA_table_size = (4 * 4))	; only program channel 0
	LDMIA	r0,{r3,r4,r5,r6}
	STMIA	r1,{r3,r4,r5,r6}
	[	(:LNOT: shutdown)
	; The DMA channels can be read, so they do not need a soft-copy state
	]	; EOF (:LNOT: shutdown)
	[	{TRUE}
	; Disable all DMA transfers
	MOV	r0,#DMA_regs
	MOV	r1,#&00000000
	STR	r1,[r0,#DMA_routing]
	]	; EOF {boolean}
	]	; EOF (:LNOT: dynlcd)
	; ---------------------------------------------------------------------
	; Initialise the MMU (default memory mapping).
	ADRL	r0,MMU_table			; default values
	MOV	r1,#MMU_base			; hardware registers
	ASSERT	(MMU_table_size = (16 * 4))	; 16 registers worth
	LDMIA	r0!,{r3,r4,r5,r6,r7,r8,r9,r10}	; first 8
	STMIA	r1!,{r3,r4,r5,r6,r7,r8,r9,r10}
	LDMIA	r0!,{r3,r4,r5,r6,r7,r8,r9,r10}	; second 8
	STMIA	r1!,{r3,r4,r5,r6,r7,r8,r9,r10}
	[	(:LNOT: shutdown)
	; The MMU can be read, so a soft-copy state is not required.
	]	; EOF (:LNOT: shutdown)

	; ---------------------------------------------------------------------
	; Initialise the LCD (shape and size information)
	MOV	r0,#LCD_regs			; LCD control registers

	MOV	r1,#((LCD_displaywidth - 1) :AND: LCD_LLMASK)
	STR	r1,[r0,#LCD_linelength]		; set the line length
	[	(:LNOT: shutdown)
	STR	r1,[r11,#LCDlinelength_data]	; and initialise the soft-copy
	]	; EOF (:LNOT: shutdown)

	MOV	r1,#(((20 / 4) - 1) :AND: LCD_LRMASK)
	STR	r1,[r0,#LCD_linerate]		; set the line rate
	[	(:LNOT: shutdown)
	STR	r1,[r11,#LCDlinerate_data]	; and initialise the soft-copy
	]	; EOF (:LNOT: shutdown)

	MOV	r1,#(((LCD_tier_height / 4) - 1) :AND: LCD_NLMASK)
	STR	r1,[r0,#LCD_numlines]		; set the number of lines
	[	(:LNOT: shutdown)
	STR	r1,[r11,#LCDnumlines_data]	; and initialise the soft-copy
	]	; EOF (:LNOT: shutdown)

	[	(:LNOT: dynlcd)
	;	       delay          latch pulse type     clock rate
	MOV	r1,#(LCD_DLY_80  :OR:   (&01 :SHL: 4)  :OR:   &09)
	ORR	r1,r1,#(LCD_WAI :OR: LCD_normal :OR: LCD_ICP)
	STR	r1,[r0,#LCD_control]		; force write to resync world
	[	(:LNOT: shutdown)
	STR	r1,[r11,#LCDcontrol_data]	; and initialise the soft-copy
	]	; EOF (:LNOT: shutdown)
	]	; EOF (:LNOT: dynlcd)

	; ---------------------------------------------------------------------

	[	(activebook)
	; This is a problem in the Active Book. The FAX, FDC and any LINK
	; adaptors share the same interrupt bit. The interrupt logic is
	; active low. If the FAX is not powered it will hold this line low
	; generating a permanent interrupt.
	!	0,"NOTE: currently enabling the FAX hardware during RESET"
	MOV	r1,#CONTROL_base	; miscellaneous control port
	MOV	r2,#CONTROL_FPE		; FAX  Power Enable
	STRB	r2,[r1,#CONTROL_reg]	; and write the information
	]	; EOF (activebook)

	; ---------------------------------------------------------------------
	; Disable all IRQ/FIQ sources and initialise the interrupt world

	MOV	r1,#INT_regs		; interrupt control registers
	MOV	r2,#&00000000		; a nice constant

	LDR	r0,=FIQ_allsources
	STR	r0,[r1,#FIQ_control]	; disable all FIQ sources
	[	(:LNOT: shutdown)
	STR	r2,[r11,#FIQ_data]	; and clear the soft-copy
	]	; EOF (:LNOT: shutdown)

	LDR	r0,=IRQ_allsources
	STR	r0,[r1,#IRQ_control]	; disable all IRQ sources
	[	(:LNOT: shutdown)
	STR	r2,[r11,#IRQ_data]	; and clear the soft-copy
	]	; EOF (:LNOT: shutdown)

	STR	r2,[r1,#INT_status]	; clear the interrupt test register
	[	(:LNOT: shutdown)
	STR	r2,[r11,#INTtest_data]	; and the soft copy
	]	; EOF (:LNOT: shutdown)

	; ---------------------------------------------------------------------
	; Ensure that the link adaptors will NOT generate interrupts for the
	; moment. Note: This pre-empts the "InitBackplane" call.
	LDR	r0,=LINK0_base
	MOV	r1,#&00
	STRB	r1,[r0,#LINK_rstatus]	; disable read interrupts
	STRB	r1,[r0,#LINK_wstatus]	; disable write interrupts

	; ---------------------------------------------------------------------

	[	(:LNOT: shutdown)
	MOV	r0,#&00000000
	STR	r0,[r11,#TIMER_data]	; clear TIMER soft-copy
	]	; EOF (:LNOT: shutdown)

	; ---------------------------------------------------------------------

	[	(activebook)
	!	0,"TODO: disable the CODEC explicitly"
	; This may be moved until after we have decided if we are performing
	; a total system RESET or restoring the last active world. We should
	; definately put "silence" into the CODEC TX buffer.
	]	; EOF (activebook)

	; ---------------------------------------------------------------------

	[	(monitor)
	; First decide if we are executing in ROM or RAM.
	CMP	pc,#ROM_base
	BLT	Helios_startup	; we are executing in RAM so start Helios
	;
	[	(activebook)
	; We should decide at this point if we are boot-strapping Helios
	; directly, or entering the monitor. Normally the tamper switch
	; will be in non-tampered mode (state 0). If the case is opened then
	; the switch will read state 1.
	MOV	r0,#CONTROL_base
	LDRB	r0,[r0,#STATUS_reg]		; load status register
	TST	r0,#STATUS_MB0			; check test/tamper switch
	BLNE	monitor_reset			; if set then enter monitor
	|
	BL	monitor_reset		; always enter the monitor on HEVALs
	]	; EOF (activebook)
Helios_startup	; Helios system startup code...
	]	; EOF (monitor)

	; ---------------------------------------------------------------------

	; Perform non-destructive memory sizing of the main RAM. We do not
	; size the FastRAM at this point (since it is implicit in the
	; processor description).
	;
        ; Define the "data abort" handler to deal with touching ROM.
	ADRL	r0,mem_abort		; branch destination address
	MOV	r1,#vec_data_abort	; branch source address
	ADD	r2,r1,#&08		; source = source + 8
	SUB	r0,r0,r2		; offset = destination - source
	MOV	r0,r0,LSR #2		; offset = offset >> 2
	ORR	r0,r0,#(cond_AL :OR: op_b)
	STR	r0,[r1,#&00]		; and write into vector

        MOV     r0,#RAM_base		; physical RAM base address
        MOV     r1,#(128 * &0400)       ; use 128K steps
        ADD     r2,r1,r0                ; base search address (starting)
        LDR     r3,=&12345678           ; a funny bit pattern
        ADR     r6,ramsize_end          ; where we go when we abort
        ORR     r6,r6,#(INTflags :OR: SVCmode)	; and ensure processor state

	; Note: We change the data-bus contents between the write and read.
	;	This code may also abort if we are touching ROM.
        LDR     r9,[r0,#&00]            ; load (and preserve) the base word
        STR     r3,[r0,#&00]            ; and store bit pattern into the RAM
        LDR     r4,=&AAAAAAAA           ; another funny bit pattern
        LDR     r8,[r0,#&00]            ; reload the bit pattern
        CMP     r8,r3                   ; and check for ROM at dataRAMblk

	MOVNE	r0,#&FFFFFFFF		; very bad number
	ADRNEL	r1,fatal_noRAM		; error message
        BNE     FatalError              ; this is a very bad condition

        ; now look for a ghost copy (or abort in the process)
ramsize_loop
        ; check for reaching RAM limit (currently the base of the ROM)
	; Since we simulate ROM systems with RAM loaded ones, we need to
	; stop sizing the RAM when we reach our system ROM image.
	ADRL	r8,codeBase		; where we are executing from
	CMP	r2,r8			; top of possible system RAM
        BEQ     ramsize_end		; assumes RAM beneath ROM

        LDR     r8,[r2,#&00]		; load the word at the next address
        CMP     r8,r3                   ; check for bit pattern
        ADDNE   r2,r2,r1                ; step upto the next RAM bank
        BNE     ramsize_loop		; and around again

        ; and then check that it is a true ghost copy
        STR     r4,[r0,#&00]		; write our second bit pattern
        LDR     r3,=&12345678           ; change the data-bus
        LDR     r8,[r2,#&00]		; and reload the second bit pattern
        CMP     r8,r4                   ; check for bit pattern
        ADDNE   r2,r2,r1                ; step upto the next RAM bank
        BNE     ramsize_loop		; since the memory does not ghost

				        ; memory has ghosted at "r2" or   
ramsize_end				; memory has aborted at "r2"

        STR     r9,[r0,#&00]            ; restore original word at RAM base

	; r0 = base of RAM (implicit in hardware description)

	; All the RAM (other than the Executive requirement) is available to
	; the user.
	ADD	r0,r0,#workspace_size

	[	(dynlcd)
	[	{TRUE}
	; Disable all DMA transfers (other than channel 0)
	MOV	r1,#DMA_regs
	MOV	r4,#&00000000
	STR	r4,[r1,#DMA_routing]
	]	; EOF {boolean}

	; Place the LCD physical memory at the top of RAM
	SUB	r2,r2,#LCD_size		; amount of LCD display RAM required
					; dictates the top of user RAM

	; setup the LCD DMA transfer registers
	MOV	r1,#DMA_base			; hardware registers
	ASSERT	(LCDscreen_base0 = 0)		; ensure offset into DMA_base
	; Default transfer pointer for both tiers of the LCD DMA
	LDR	r4,=((RAM_base :SHL: DMA_src_shift) :OR: DMALCD_xfer)
	MOV	r6,r4
	MOV	r7,#DMALCD_xfer			; default transfer control
	ORR	r3,r7,r2,LSL #DMA_src_shift	; build tier1 base address
	ADD	r5,r2,#(LCD_size / 2)		; build tier2 physical address
	ORR	r5,r7,r5,LSL #DMA_src_shift	; build tier2 base address
	STMIA	r1,{r3,r4,r5,r6}		; and initialise the DMA regs

	; set the MMU address mapping (logical screen at &00740000 : segment 3)
	MOV	r1,#MMU_base			; hardware registers
	SUB	r3,r2,#&00140000		; remove offset into segment 3
	MOV	r3,r3,LSR #9			; clear bottom bits
	LDR	r4,=MMU_basemask		; masking is probably not
	AND	r3,r3,r4			; needed to getV into range
	ORR	r3,r3,#(MMU_mapup :OR: (4096 - 4096) :OR: MMU_mapRWE)
	STR	r3,[r1,#(3 * 4)]

	; Enable the LCD display (should start LCD DMA requests)
	;	       delay          latch pulse type     clock rate
	[	(shutdown)
	MOV	r3,#LCD_regs		; LCD control registers
	MOV	r1,#(LCD_DLY_80  :OR:   (&01 :SHL: 4)  :OR:   &09)
	ORR	r1,r1,#(LCD_WAI :OR: LCD_normal :OR: LCD_ICP)
	STR	r1,[r3,#LCD_control]		; force write to resync world
	|
	MOV	r3,#LCD_regs		; LCD control registers
	LDR	r11,=hardware_regs	; where all the soft-copies are kept
	MOV	r1,#(LCD_DLY_80  :OR:   (&01 :SHL: 4)  :OR:   &09)
	ORR	r1,r1,#(LCD_WAI :OR: LCD_normal :OR: LCD_ICP)
	STR	r1,[r3,#LCD_control]		; force write to resync world
	STR	r1,[r11,#LCDcontrol_data]	; and initialise the soft-copy
	]	; EOF (shutdown)

	; This piece of code should probably only be included when talking to
	; the latest uController software. Since we still want to perform
	; releases, this code is only enabled if it is not a release.
	[	((newpdcode) :LOR: (newbreak))
	!	0,"**** LCDon message will be sent ****"
	MOV	r3,#MLI_regs
	MOV	r1,#&00
	STRB	r1,[r3,#MLI_CON]		; clear-down microlink iface
	; We need to ensure the the micro-link is active before attempting to
	; talk to the uController.
	MOV	r1,#(MLI_ENA :OR: MLI_ICP)	; enable with internal clock
	STRB	r1,[r3,#MLI_CON]		; and enable the micro-link
	[	(newpdcode)
	; Send the message to the uController saying that we are now
	; refreshing DRAM (with LCD DMAs).
	MOV	r1,#(MLHdr_LCDctrl :OR: LCDctrl_on :OR: LCDctrl_onPWR)
	[	{TRUE}
	LDR	sp,=DummySpace
	STMFD	sp!,{r0}
	ADR	r0,lotxt1
	BL	local_Output
	MOV	r0,r1
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0}
	B	loovr1
lotxt1	=	"about to send LCD on message &",&00
	ALIGN
loovr1
	]	; EOF {boolean}
	STRB	r1,[r3,#MLI_TXD]
	; We need to loop here until we receive the uController acknowledgement
LCDon_ack_loop
	LDRB	r1,[r3,#MLI_STA]	; micro-link status
	TST	r1,#MLI_RIF		; byte pending
	BEQ	LCDon_ack_loop		; no byte pending, so loop
	; The acknowledgement byte contains information about the "vback"
	; preservation state since the last PowerDown.
	LDRB	r10,[r3,#MLI_RXD]	; load acknowledgement byte
	; r10 = LCDon_vback if "vback" was stable whilst we were away
	; r10 = LCDon_bad   if "vback" was unstable whilst we were away
	[	{TRUE}
	LDR	sp,=DummySpace
	STMFD	sp!,{r0}
	ADR	r0,lotxt2
	BL	local_Output
	MOV	r0,r10
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0}
	B	loovr2
lotxt2	=	"received LCD on acknowledgement &",&00
	ALIGN
loovr2
	]	; EOF {boolean}
	]	; EOF (newpdcode)
	]	; EOF ((newpdcode) :LOR: (newbreak))
	]	; EOF (dynlcd)

        B       size_memory_completed

	[	(dynlcd)
	LTORG	; literal pool
	]	; EOF (dynlcd)

	; ---------------------------------------------------------------------
	; This table contains the initialisation values for the BANK registers.
	; It assumes the mapping of the registers at "BANK_regs".
BANK_RAMMAP	*	(BANK_RAMMODE1 :OR: BANK_NOPC)
	[	(activebook)
BANK_FIOMAP	*	(BANK_RAMMODE1 :OR: BANK_NOPC)	; Fast IO
BANK_SIOMAP	*	(BANK_RAMMODE1 :OR: BANK_PC)	; Slow IO
BANK_EXTMAP	*	(BANK_ROMMODE0 :OR: BANK_PC)	; external ROM
BANK_ROMMAP	*	(BANK_ROMMODE0 :OR: BANK_NOPC)	; internal ROM
	|	; middle (activebook)
BANK_IOMAP	*	(BANK_ROMMODE1 :OR: BANK_PC)
	]	; EOF (activebook)
BANK_table
	[	(activebook)
	& (              BANK_FIOMAP :OR: BANK_T1_0 :OR: BANK_T2_0) ; EXTIO1
	& (BANK_W32 :OR: BANK_SIOMAP :OR: BANK_T1_2 :OR: BANK_T2_4) ; EXTIO2
	|	; middle (activebook)
	& (                               BANK_T1_1 :OR: BANK_T2_1) ; EXTIO1
	& (BANK_W32 :OR: BANK_IOMAP  :OR: BANK_T1_2 :OR: BANK_T2_4) ; EXTIO2
	]	; EOF (activebook)
	& (BANK_W32 :OR:                  BANK_T1_0 :OR: BANK_T2_0) ; EXTMEM1
	& (BANK_W32 :OR:                  BANK_T1_0 :OR: BANK_T2_0) ; EXTMEM2
	& (BANK_W32 :OR: BANK_RAMMAP :OR: BANK_T1_1 :OR: BANK_T2_0) ; DRAM1
	[	(activebook)
	& (BANK_W8  :OR: BANK_EXTMAP :OR: BANK_T1_4 :OR: BANK_T2_2) ; EXTCARD
	& (BANK_W32 :OR: BANK_ROMMAP :OR: BANK_T1_2 :OR: BANK_T2_2) ; ROM
	& (BANK_W8  :OR: BANK_EXTMAP :OR: BANK_T1_2 :OR: BANK_T2_2) ; FLASH
	|	; middle (activebook)
	& (BANK_W32 :OR:                  BANK_T1_1 :OR: BANK_T2_1) ; PSRAM2
	& (BANK_W0  :OR:                  BANK_T1_2 :OR: BANK_T2_1) ; SLOWROM1
	& (BANK_W32 :OR:                  BANK_T1_1 :OR: BANK_T2_1) ; SLOWROM2
	]	; EOF (activebook)
BANK_table_end
BANK_table_size	*	(BANK_table_end - BANK_table)

	; ---------------------------------------------------------------------
	[	(:LNOT: dynlcd)
	; This table contains the default initialisation values for the
	; DMA system. It assumes the mapping of the registers at "DMA_base".
	; Note: At the moment this table only defines channel 0 (LCD).
DMA_table
	[	(activebook)
	&	(LCD_base_tier1 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(RAM_base       :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(LCD_base_tier2 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(RAM_base       :SHL: DMA_src_shift) :OR: DMALCD_xfer
	|
	&	(LCD_base_tier1 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(RAM1_base      :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(LCD_base_tier2 :SHL: DMA_src_shift) :OR: DMALCD_xfer
	&	(RAM1_base      :SHL: DMA_src_shift) :OR: DMALCD_xfer
	]
DMA_table_end
DMA_table_size	*	(DMA_table_end - DMA_table) ;
	]	; EOF (:LNOT: dynlcd)
	; ---------------------------------------------------------------------
	; This table contains the default initialisation values for the
	; MMU segment control registers. It assumes the mapping of the
	; registers at "MMU_base".
MMU_table
	[	(page0trap)
	; Do not allow any "page 0" USR mode accesses; Trap NULL de-referencing
	MMUVAL	   1,SRAM_base,             NONE, UP    ;  0 ; FastSRAM
	|	; middle (page0trap)
	MMUVAL	   1,SRAM_base,             WRITE,UP	;  0 ; FastSRAM
	]	; EOF (page0trap)
	MMUVAL	2049,MMU_base,              WRITE,UP	;  1 ; MMU & DMA
	MMUVAL	4096,(IO_base + zeromeg),   WRITE,UP	;  2 ; IO
	[	(dynlcd)
	MMUVAL	   0,&00000000,             NONE, UP	;  3 ; for mapped LCD
	|
	MMUVAL	4096,(LCD_base - &140000),  WRITE,UP	;  3 ; mapped LCD
	]	; EOF (dynlcd)
	MMUVAL	   0,&00000000,             NONE, UP	;  4 ; not mapped
	MMUVAL  4096,(EXTIO1_base + twomeg),WRITE,UP	;  5 ; Serial IO (etc.)
	[	(activebook)
	MMUVAL	4096,EXTIO2_base,           WRITE,UP    ;  6 ; AB1 FAX IO
	|
	MMUVAL	   0,&00000000,             NONE, UP	;  6 ; not mapped
	]	; EOF (activebook)
	[	(heval)
	MMUVAL	4096,(EXTIO2_base + twomeg),WRITE,UP	;  7 ; Heval FAX IO
	|
	MMUVAL     0,&00000000,             NONE, UP    ;  7 ; not mapped
	]	; EOF (heval)
	MMUVAL	   0,&00000000,             NONE, UP	;  8 ; not mapped
	MMUVAL	   0,&00000000,             NONE, UP	;  9 ; not mapped
	MMUVAL	   0,&00000000,             NONE, UP	; 10 ; not mapped
	MMUVAL	   0,&00000000,             NONE, UP	; 11 ; not mapped
	MMUVAL	   0,&00000000,             NONE, UP	; 12 ; not mapped
	MMUVAL	   0,&00000000,             NONE, UP	; 13 ; not mapped
	MMUVAL	   0,&00000000,             NONE, UP	; 14 ; not mapped
	MMUVAL	   0,&00000000,             NONE, UP	; 15 ; not mapped
MMU_table_end
MMU_table_size	*	(MMU_table_end - MMU_table)

	; ---------------------------------------------------------------------
	|	; ---- middle (hercules) --------------------------------------
	; ---------------------------------------------------------------------
        ; Disable all hardware IRQ/FIQ sources.
        ; (Note: the link adaptor present on the prototype does NOT have
        ;        a software accessible control register (speed, reset, etc.))

        ; Set both interrupt enable registers and their soft copies
	; Note that InitBackPlane will set both the IRQ register and
	; its soft copy later.

	MOV	r0,#&00000000		; No FIQs or IRQs enabled at startup
        LDR     r1,=irq_mask            ; Not 8bit value on this prototype
        STRB    r0,[r1,#0]              ; Disable all IRQs
        LDR     r1,=IRQ_mask_copy       ; Address of soft copy
        STR     r0,[r1,#0]              ; Set soft copy

        LDR     r1,=fiq_mask            ; Not 8bit value on this prototype
        STRB    r0,[r1,#0]              ; Disable all FIQs
        LDR     r1,=FIQ_mask_copy       ; Address of soft copy
        STR     r0,[r1,#0]              ; Clear soft copy

        ; Ensure the link adaptors will NOT generate interrupts for the moment
        ; Note: This pre-empts the "InitBackplane" call.
        MOV     r1,#&00
        MOV     r0,#LINK0_base		; link to IO server
        STRB    r1,[r0,#LINK_rstatus]   ; disable read interrupts
        STRB    r1,[r0,#LINK_wstatus]   ; disable write interrupts

	MOV	r0,#ml_link_base	; link used as microlink substitute
	STRB	r1,[r0,#LINK_rstatus]	; disable read interrupts
	STRB	r1,[r0,#LINK_wstatus]	; disable write interrupts

        ; Ensure that the RAM is NOT write-protected
        MOV     r0,#mmumap_base         ; required for the following MACROs
        ; Set up the MMU to give OS mode 1, with straight-through access to
        ; fast RAM and devices on the Functional Prototype board.
	[	(page0trap)
	; Protect the locations around &00000000 to trap code that attempts to
	; de-reference NULL. This protects a complete 2MB chunk.
	MMUMAP	 0,   1,&00000000,NONE ,UP
	|	; middle (page0trap)
        MMUMAP   0,   1,&00000000,WRITE,UP
	]	; EOF (page0trap)
        MMUMAP   1,   0,&00000000,NONE ,UP
        MMUMAP   2,4096,&00400000,WRITE,UP
        MMUMAP   3,4096,&00600000,WRITE,UP
        MMUMAP   4,   0,&00000000,NONE ,UP
        MMUMAP   5,   0,&00000000,NONE ,UP
        MMUMAP   6,   0,&00000000,NONE ,UP
        MMUMAP   7,   0,&00000000,NONE ,UP
        MMUMAP   8,   0,&00000000,NONE ,UP
        MMUMAP   9,   0,&00000000,NONE ,UP
        MMUMAP  10,   0,&00000000,NONE ,UP
        MMUMAP  11,   0,&00000000,NONE ,UP
        MMUMAP  12,   0,&00000000,NONE ,UP
        MMUMAP  13,   0,&00000000,NONE ,UP
        MMUMAP  14,   0,&00000000,NONE ,UP
        MMUMAP  15,   0,&00000000,NONE ,UP

        ; Enable OS mode 1
        LDR     r0,=mmu_mode            ; control register address
	[	{TRUE}			; bodge test for MJackson
	MOV	r1,#&00			; physical memory map
	; This drives the FastRAM at 2cycles access, whereas the line below
	; drives it at 8cycles access (which actually makes it slower than
	; the normal DRAM).
	|
        MOV     r1,#(mmumode_mapen :OR: mmumode_osmode :OR: mmumode_mode1)
	]
        STRB    r1,[r0,#&00]

	; ---------------------------------------------------------------------

	[	(monitor)
	; Check if we are executing in RAM or ROM
	CMP	pc,#ROM_base	; check against the base of the system ROM
	BLT	Helios_startup	; we are running in RAM, so start Helios
	;
	; When executing in ROM, we always enter the monitor.
	BL	monitor_reset			; enter monitor system
Helios_startup	; Helios system startup code...
	]	; EOF (monitor)

	; ---------------------------------------------------------------------
        ; Functional prototypes have 3 areas of RAM:
        ;
        ; - 4KB static RAM at &00000000 used for vectors and small
        ;   amounts of time-critical code. In Hercules this will reduce to 
        ;   512 bytes.
        ;
        ; - Up to 16MB DRAM starting at &02000000. We must find the
        ;   size of this.
        ;
        ; - 256KB of video RAM starting at &00740000. The first 100KB
        ;   of this is the LCD memory (400 lines * 2 planes * (80+48) bytes).
        ;
        ; sysRAMtop     = top of memory from &02000000
        ; userRAMbase   = base address of memory available to user
        ; userRAMtop    = top of memory available to user

        ; RAM sizing must be checked by ghosting and aborting
        ; We are currently in SVC mode (IRQs/FIQs disabled) with the
        ; complete register set available. Note: The hardware vectors
        ; have NOT yet been written to the RAM.

	; RAM sizing should be NON-Destructive

        ; Define the "data abort" handler
	ADRL	r0,mem_abort		; branch destination address
	MOV	r1,#vec_data_abort	; branch source address
	ADD	r2,r1,#&08		; source = source + 8
	SUB	r0,r0,r2		; offset = destination - source
	MOV	r0,r0,LSR #2		; offset = offset >> 2
	ORR	r0,r0,#(cond_AL :OR: op_b)
	STR	r0,[r1,#&00]		; and write into vector

        MOV     r0,#dataRAMblk          ; base RAM address
        MOV     r1,#(128 * &0400)       ; use 128K steps
        ADD     r2,r1,r0                ; base search address
        LDR     r3,=&12345678           ; a funny bit pattern
        LDR     r4,=&AAAAAAAA           ; another funny bit pattern
        ADR     r6,sysmem_end           ; where we go when we abort
        ORR     r6,r6,#(INTflags :OR: SVCmode)

        LDR     r9,[r0,#&00]            ; load (and preserve) the word
        STR     r3,[r0,#&00]            ; and store bit pattern into the RAM
        LDR     r8,[r0,#&00]            ; reload the bit pattern
        CMP     r8,r3                   ; and check for ROM at dataRAMblk

	MOVNE	r0,#&FFFFFFFF		; very bad number
	ADRNEL	r1,fatal_noRAM		; error message
        BNE     FatalError              ; this is a very bad condition

        ; now look for a ghost copy
sizesysmem_loop
        ; check for reaching RAM limit (currently ROM_base)
	ADRL	r8,codeBase		; where our ROM image starts
	CMP	r2,r8			; top of possible system RAM
        BEQ     sysmem_end

        LDR     r8,[r2,#&00]
        CMP     r8,r3                   ; check for bit pattern
        ADDNE   r2,r2,r1                ; step upto the next RAM bank
        BNE     sizesysmem_loop
        ; and then check that it is a true ghost copy
        STR     r4,[r0,#&00]
        LDR     r8,[r2,#&00]
        CMP     r8,r4                   ; check for bit pattern
        ADDNE   r2,r2,r1                ; step upto the next RAM bank
        BNE     sizesysmem_loop
        ; memory has ghosted at "r2"    
sysmem_end
        STR     r9,[r0,#&00]            ; restore original word

	; r0 = base of RAM
	; All the RAM (other than the Executive requirement) is available to
	; the user.
	ADD	r0,r0,#workspace_size
	B	size_memory_completed	; continue initialisation
	]	; EOF (hercules)

        ; ---------------------------------------------------------------------

fatal_noRAM
	=	"No RAM found",&00
fatal_badAck
	=	"uController \"vback\" acknowledgement invalid",&00
	ALIGN

	; ---------------------------------------------------------------------

mem_abort
        ; entered when a "data abort" has occured during RAM sizing
        ; r0 = base address
        ; r1 = size of RAM bank
        ; r2 = address being read
        ; r3 = bit pattern 1
        ; r4 = bit pattern 2 if first RAM write succeeded (otherwise undefined)
        ; r5 = ROM start address
        ; r6 = return address (and processor state)
        ; r8 = temporary work register
        ; r9 = original "r0" contents

        ; The data abort marked the end of the RAM (since it must appear
        ; in multiples of "r1")
        MOVS    pc,r6

        ; ---------------------------------------------------------------------
        ; -- Startup RESET branching (QUICK or NORMAL) ------------------------
        ; ---------------------------------------------------------------------

size_memory_completed
        ; r0  = "userRAMbase" and "sysRAMtop" value
        ; r2  = "userRAMtop" value
	[	(newpdcode)
	; r10 = "vback" state from the uController
	]	; EOF (newpdcode)

	[	{TRUE}
	LDR	sp,=DummySpace
	STMFD	sp!,{r0}
	ADR	r0,rstxt1
	BL	local_Output
	LDMFD	sp,{r0}
	BL	local_WriteHex8
	ADR	r0,rstxt2
	BL	local_Output
	MOV	r0,r2
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0}
	B	rsovr1
rstxt1	=	"after RAMsize: userRAMbase (r0) = &",&00
rstxt2	=	" userRAMtop (r2) = &",&00
	ALIGN
rsovr1
	]	; EOF {boolean}

	[	(shutdown)
	[	(newpdcode) ; needs new uController software for this
	; The uController returns the "vback" state to as the LCD on message
	; acknowledgement. During startup we need to know (from the
	; uController) if "vback" was preserved since the last system
	; PowerDown. If it wasn't then we cannot trust the RAM state and must
	; perform a FULL system RESET.
	!	0,"**** acting on LCD on acknowledgement ****"
	TEQ	r10,#LCDon_vback	; check "vback" preserved state
	BNE	complete_system_RESET	; uController says RAM invalidated
	]	; EOF (newpdcode)

	; We check for a QUICK system re-start here.
	; We have sized the RAM above (so know at least some is present)
	; and the uController has maintained DRAM refresh while we were
	; away. See if the RAM shape has changed (possible Executive change).
	; NOTE: we should not corrupt "r0" or "r2" over this code.

	LDR	r1,=sysRAMtop		; base of the memory addresses
	LDMIA	r1,{r3,r4,r5}		; load the memory addresses
	TEQ	r0,r3			; sysRAMtop
	TEQEQ	r0,r4			; userRAMbase
	TEQEQ	r2,r5			; userRAMtop
	BNE	complete_system_RESET	; different, so RESET system fully
	ASSERT	((sysRAMtop + 4) = userRAMbase)		; ensure data order
	ASSERT	((userRAMbase + 4) = userRAMtop)	; ensure data order

	; So far NO RAM locations have been modified during the RESET sequence.

	; Verify the "CompleteState" structure
	LDR	r4,=RestartId1		; address of first check word
	LDR	r6,[r4,#&00]		; and load contents
	LDR	r5,=RestartId2		; address of second check word
	LDR	r7,[r5,#&00]		; and load contents
	TEQ	r6,r7			; check for equality
	BNE	complete_system_RESET	; NO, then we must RESET the system
	LDR	r8,=InvalidRestartMarker; validity check word
	TEQ	r6,r8
	BEQ	complete_system_RESET	; invalid ID marker words

	; -- QUICK RE-START ---------------------------------------------------

	LDR	r0,=RestartState		; CompleteState structure

	ADD	r1,r0,#CompleteState_USR_r13	; base of USR mapped registers
	LDMIA	r1,{r13,r14}^			; load USR mapped registers
	MOV	r1,#(INTflags :OR: FIQmode)	; PSR state required
	TEQP	r1,#&00000000			; FIQ mode; IRQ/FIQ disabled
	NOP					; wait for register mapping
	ADD	r1,r0,#CompleteState_FIQ_r8	; base of FIQ mapped registers
	LDMIA	r1,{r8,r9,r10,r11,r12,r13,r14}	; load FIQ mapped registers
	MOV	r1,#(INTflags :OR: IRQmode)	; PSR state required
	TEQP	r1,#&00000000			; IRQ mode; IRQ/FIQ disabled
	NOP					; wait for register mapping
	ADD	r1,r0,#CompleteState_IRQ_r13	; base of IRQ mapped registers
	LDMIA	r1,{r13,r14}			; load IRQ mapped registers
	TEQP	pc,#(INTflags :OR: SVCmode)	; SVC mode; IRQ/FIQ disabled

	; Copy the FastRAM contents back to SRAM_base
	ADD	r1,r0,#CompleteState_FastRAM	; data source
	MOV	r2,#SRAM_base			; data destination
	MOV	r3,#SRAM_size			; number of bytes to transfer
FastRAM_restore_loop
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; load 8 registers worth
	STMIA	r2!,{r4,r5,r6,r7,r8,r9,r10,r11}	; store 8 registers worth
	SUBS	r3,r3,#(8 * 4)			; decrement count
	BNE	FastRAM_restore_loop		; and around for more
	ASSERT	((SRAM_size :MOD: 8) = 0)	; ensure code will work

	; Restore the MMU mapping in force before the PowerDown
	ADD	r1,r0,#CompleteState_segment0	; base of MMU saved registers
	MOV	r2,#MMU_base
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; segments 0..7
	STMIA	r2!,{r4,r5,r6,r7,r8,r9,r10,r11}
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; segments 8..F
	STMIA	r2!,{r4,r5,r6,r7,r8,r9,r10,r11}

	; Restore the DMA registers (assumes knowledge of the processor shape)
	ADD	r1,r0,#CompleteState_DMAch0base0; data source
	MOV	r2,#DMA_base			; data destination
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; channel 0 and channel 1
	STMIA	r2!,{r4,r5,r6,r7,r8,r9,r10,r11}
	LDMIA	r1!,{r4,r5,r6,r7,r8,r9,r10,r11}	; channel 2 and channel 3
	STMIA	r2!,{r4,r5,r6,r7,r8,r9,r10,r11}
	; NOTE: The DMA routing register and the LCD control registers will
	;	be restored in the following code.

	; Restore register copies kept in the "CompleteState" structure

	; Inmos link adaptors
	LDR	r1,=LINK0_base
	LDRB	r3,[r0,#CompleteState_LINK0_rstatus]
	STRB	r3,[r1,#LINK_rstatus]
	LDRB	r3,[r0,#CompleteState_LINK0_wstatus]
	STRB	r3,[r1,#LINK_wstatus]
	LDR	r1,=LINK1_base
	LDRB	r3,[r0,#CompleteState_LINK1_rstatus]
	STRB	r3,[r1,#LINK_rstatus]
	LDRB	r3,[r0,#CompleteState_LINK1_wstatus]
	STRB	r3,[r1,#LINK_wstatus]

	; Restore all the Hercules registers (that we possibly can)
	; from the soft-copies we hold in Executive workspace RAM.
	; NOTE: We required special code to restore the IRQ/FIQ enable
	;	registers (since we need to define the control bit).
	LDR	r5,=hardware_regs		; address the RAM copies
	ADRL	r1,hwregs_table			; hardware register description
	ADRL	r2,hwregs_table_end		; end of the descriptions
	; Update the IRQ and FIQ saved state information to enable the
	; relevant interrupt sources.
	LDR	r3,[r5,#IRQ_data]
	ORR	r3,r3,#IRQ_set
	STR	r3,[r5,#IRQ_data]
	LDR	r3,[r5,#FIQ_data]
	ORR	r3,r3,#FIQ_set
	STR	r3,[r5,#FIQ_data]
HWRegister_restore_loop
	LDR	r3,[r1],#&04			; load the register description
	TST	r3,#hwreg_width_8		; check for byte wide register
	BIC	r3,r3,#(:NOT: hwreg_addr_mask)	; and get the register address
	LDR	r4,[r5],#&04			; load the soft-copy value
	STRNEB	r4,[r3,#&00]			; byte store
	STREQ	r4,[r3,#&00]			; halfword/word store
	TEQ	r1,r2				; check for completion
	BNE	HWRegister_restore_loop		; and around again

	; Invalidate the "CompleteState" check markers (in-case we have another
	; RESET before this startup code has completed).
	LDR	r1,=InvalidRestartMarker
	LDR	r3,=RestartId1
	STR	r1,[r3,#&00]		; and over-write the current value
	LDR	r3,=RestartId2
	STR	r1,[r3,#&00]
	[	(newpdcode)
	; We should send the uController the "vback" state acknowledgement.
	MOV	r3,#MLI_regs			; address micro-link hardware
	MOV	r1,#(MLHdr_VBack :OR: VBack_ok)	; set "vback" OK flag
	STRB	r1,[r3,#MLI_TXD]		; and send to the uController
	; We need to loop here until we receive the uController acknowledgement
QR_ack_loop
	LDRB	r1,[r3,#MLI_STA]		; micro-link status
	TST	r1,#MLI_RIF			; byte pending
	BEQ	QR_ack_loop			; no byte pending, so loop
	LDRB	r1,[r3,#MLI_RXD]		; load acknowledgement byte
	; The acknowledge should be (and only be) "MLHdr_Ack". If we have not
	; received this byte then we are in an undefined reset state and
	; should abort.
	TEQ	r1,#MLHdr_Ack
	MOVNE	r0,#&FFFFFFFF			; very bad number
	ADRNEL	r1,fatal_badAck			; error message
	BNE	FatalError			; invalid acknowledgement
	]	; EOF (newpdcode)

	; NOTE: The "r0" value stored in the "CompleteState" structure defines
	;	which type of PowerDown preserved the state.
	;		r0 = &00000000 (0)	; normal shutdown
	;		r0 = &FFFFFFFF (-1)	; emergency PowerFail

	[	(CompleteState_r0 <> 0)
	ADD	r0,r0,#CompleteState_r0		; reference the stored state
	]
	LDMIA	r0,{r0-r15}^	; re-start from the stored processor state

	; ---------------------------------------------------------------------
	; -- Complete system RESET --------------------------------------------
	; ---------------------------------------------------------------------
	; We reach this point if the RAM contents were compromised whilst the
	; processor was powered off, the RAM size has changed during PowerDown
	; (very unlikely) or the CompleteState (describing the processor) has
	; failed its validity checks.
complete_system_RESET
	; Invalidate the "CompleteState" check markers (in-case we have another
	; RESET before this startup code has completed).
	LDR	r1,=InvalidRestartMarker
	LDR	r3,=RestartId1
	STR	r1,[r3,#&00]		; and over-write the current value
	LDR	r3,=RestartId2
	STR	r1,[r3,#&00]

	[	(newpdcode)
	; We should send the uController the "vback" state acknowledgement.
	MOV	r3,#MLI_regs			; address micro-link hardware
	MOV	r1,#(MLHdr_VBack :OR: VBack_ok)	; set "vback" OK flag
	STRB	r1,[r3,#MLI_TXD]		; and send to the uController
	; We need to loop here until we receive the uController acknowledgement
FR_ack_loop
	LDRB	r1,[r3,#MLI_STA]		; micro-link status
	TST	r1,#MLI_RIF			; byte pending
	BEQ	FR_ack_loop			; no byte pending, so loop
	LDRB	r1,[r3,#MLI_RXD]		; load acknowledgement byte
	; The acknowledge should be (and only be) "MLHdr_Ack". If we have not
	; received this byte then we are in an undefined reset state and
	; should abort.
	TEQ	r1,#MLHdr_Ack
	MOVNE	r0,#&FFFFFFFF			; very bad number
	ADRNEL	r1,fatal_badAck			; error message
	BNE	FatalError			; invalid acknowledgement
	]	; EOF (newpdcode)

	; and continue with the system RESET code
	;
	; -- INITIALISE -------------------------------------------------------
	; Setup all the soft-copies ...
	; NOTE: we should always ensure that these values are the same as
	;	were programmed into the real hardware above
	; 	Do not use r0 or r2 over the following code.
	;
	; Load the address of the soft-copies
	LDR	r11,=hardware_regs		; where the soft-copies are
	; CLOCK_regs
	MOV	r1,#(CODEC_ON :OR: TIMER_ON :OR: MLI_ON)
	STR	r1,[r11,#CLOCK_data]		; clock soft-copy
	; BANK timing registers
	MOV	r1,#BANK_regs			; hardware registers
	LDR	r1,[r1,#BANK6_reg]		; current BANK 6 mapping
	AND	r1,r1,#BANK_WMASK		; current BANK BUS width
	ADRL	r12,BANK_table			; table of default values
	LDMIA	r12,{r3,r4,r5,r6,r7,r8,r9,r10}	; load default BANK timings
	ORR	r9,r9,r1			; retain external bus width
	ADD	r12,r11,#BANK0_data		; address the soft-copies
	STMIA	r12,{r3,r4,r5,r6,r7,r8,r9,r10}	; and store the soft-copies
	; MMUMAP_regs
	MOV	r1,#(MAPEN_USR :OR: OS_MODE1)	; Enable OS mode 1
	STR	r1,[r11,#MEMMAP_data]		; word write to clear top
	; LCD registers
	MOV	r1,#((LCD_displaywidth - 1) :AND: LCD_LLMASK)
	STR	r1,[r11,#LCDlinelength_data]	; and initialise the soft-copy
	MOV	r1,#(((20 / 4) - 1) :AND: LCD_LRMASK)
	STR	r1,[r11,#LCDlinerate_data]	; and initialise the soft-copy
	MOV	r1,#(((LCD_tier_height / 4) - 1) :AND: LCD_NLMASK)
	STR	r1,[r11,#LCDnumlines_data]	; and initialise the soft-copy
	;	       delay          latch pulse type     clock rate
	MOV	r1,#(LCD_DLY_80  :OR:   (&01 :SHL: 4)  :OR:   &09)
	ORR	r1,r1,#(LCD_WAI :OR: LCD_normal :OR: LCD_ICP)
	STR	r1,[r11,#LCDcontrol_data]	; and initialise the soft-copy
	; Interrupt control
	MOV	r1,#INT_regs		; interrupt control registers
	MOV	r3,#&00000000		; a nice constant
	STR	r3,[r11,#FIQ_data]	; and clear the soft-copy
	STR	r3,[r11,#IRQ_data]	; and clear the soft-copy
	STR	r2,[r11,#INTtest_data]	; and the soft copy
	; TIMER control
	MOV	r1,#&00000000
	STR	r1,[r11,#TIMER_data]	; clear TIMER soft-copy
	[	(activebook)
	MOV	r1,#CONTROL_FPE		; FAX  Power Enable
	STR	r1,[r11,#Control_data]	; control port initialisation
	]	; EOF (activebook)
	]	; EOF (shutdown)

	; In the (shutdown) code above r1 is zeroed at the end. When the
	; conditionals are removed this "MOV" should be optimised out.
	MOV	r1,#&00000000
	LDR	r3,=FIQ_single		; single/multiple flag word
	STR	r1,[r3,#&00]		; initialise multiple FIQ handler flag

	[	((activebook) :LAND: {TRUE})
	; r1  = &00000000
	; r11 = hardware_regs
	STR	r1,[r11,#DMArouting_data]	; ensure soft-copy is clear
	; DMA allocation support. This could be optimised to be a store
	; multiple of zero.
	LDR	r3,=DMAchannel_states
	STR	r1,[r3],#word		; DMA channel 1 allocation state
	STR	r1,[r3],#word		; DMA channel 2 allocation state
	STR	r1,[r3,#&00]		; DMA channel 3 allocation state
	]	; EOF ((activebook) :LAND: {boolean})

        LDR     r1,=sysRAMtop		; where Executive RAM top is stored
        STR     r0,[r1,#0]              ; top of system RAM is base of user RAM
        LDR     r1,=userRAMbase		; where user RAM base is stored
        STR     r0,[r1,#&00]            ; base of the user RAM
        LDR     r1,=userRAMtop		; where user RAM top is stored
        STR     r2,[r1,#0]              ; top of user RAM is top of memory
	; r0 = base of USER RAM (userRAMbase and sysRAMtop)
	; r2 = end of USER RAM (userRAMtop)

	; Do not zero memory. If it is required it will be performed by the
	; Helios kernel initialisation. This is because the kernel memory
	; initialisation will perform any necessary RAM data (RAMFS) recovery.

        ; setup the RAM copy of the hardware vectors
	; We need to this by hand if we wish the code to be completely PIC.
	; This leads to longer (slower) initialisation code, but does mean
	; that the system can be placed anywhere in memory.
	MOV	r0,#&00000000		; where the vectors live

	ADRL	r1,exec_branch_through_zero
	ADD	r2,r0,#&08		; source = source + 8
	SUB	r1,r1,r2		; offset = destination - source
	MOV	r1,r1,LSR #2		; offset = offset >> 2
	ORR	r1,r1,#(cond_AL :OR: op_b)
	STR	r1,[r0],#&04		; branch-through-zero vector

	LDR	r1,vector_undef_instruction
	STR	r1,[r0],#&04		; undefined instruction vector

	ADRL	r1,exec_SWI
	ADD	r2,r0,#&08		; source = source + 8
	SUB	r1,r1,r2		; offset = destination - source
	MOV	r1,r1,LSR #2		; offset = offset >> 2
	ORR	r1,r1,#(cond_AL :OR: op_b)
	STR	r1,[r0],#&04		; SWI vector

	ADRL	r1,exec_prefetch_abort
	ADD	r2,r0,#&08		; source = source + 8
	SUB	r1,r1,r2		; offset = destination - source
	MOV	r1,r1,LSR #2		; offset = offset >> 2
	ORR	r1,r1,#(cond_AL :OR: op_b)
	STR	r1,[r0],#&04		; prefetch abort vector

	ADRL	r1,exec_data_abort
	ADD	r2,r0,#&08		; source = source + 8
	SUB	r1,r1,r2		; offset = destination - source
	MOV	r1,r1,LSR #2		; offset = offset >> 2
	ORR	r1,r1,#(cond_AL :OR: op_b)
	STR	r1,[r0],#&04		; data abort vector

	ADRL	r1,exec_address_exception
	ADD	r2,r0,#&08		; source = source + 8
	SUB	r1,r1,r2		; offset = destination - source
	MOV	r1,r1,LSR #2		; offset = offset >> 2
	ORR	r1,r1,#(cond_AL :OR: op_b)
	STR	r1,[r0],#&04		; address exception vector

	ADRL	r1,exec_IRQ
	ADD	r2,r0,#&08		; source = source + 8
	SUB	r1,r1,r2		; offset = destination - source
	MOV	r1,r1,LSR #2		; offset = offset >> 2
	ORR	r1,r1,#(cond_AL :OR: op_b)
	STR	r1,[r0],#&04		; IRQ vector

	LDR	r1,vector_FIQ_instruction
	STR	r1,[r0],#&04		; FIQ vector

        ; setup the IRQ vector table (at "IRQ_vectors")
        ; (this is IO system dependent)
        ; most of the entries will be NULL operations so write the complete
        ; table as NULL entries

        LDR     r0,=IRQ_vectors         ; address the IRQ vector table
        MOV     r1,#num_IRQs            ; and the number of IRQ table entries
        MOV     r1,r1,LSL #2            ; r1 = (r1 * 4)
        ADRL    r2,null_instruction
        LDR     r2,[r2,#&00]
build_IRQ_table
        SUBS    r1,r1,#&04              ; decrement the table index
        STR     r2,[r0,r1]              ; store the NULL entry
        BNE     build_IRQ_table

        ; The proto-type does NOT have easily accessible IRQ sources like
        ; an IOC system. For the moment we will allocate specific IRQ slots
        ; for the required sources.
        ;
        ; Slot          Usage
        ; ---------------------------------------------------------------------
        ;  0            timer clock 0 (10milli-second tick for scheduler)
        ;  1            link1 when link transfer is IRQ-driven
	;  2		link0 used as substitute for microlink
        ;  3..15        <<UNALLOCATED>>
        ; ---------------------------------------------------------------------
        ; Add into the table the "timer" interrupt handler address (entry 0)
        ; r1 = constructed branch instruction to "ClockInterrupt"
        ; (cond_AL | op_b | ((destination - (source + 8)) >> 2))

	; change to call the new "local_VectorPatch" code

        ADRL    r3,ClockInterrupt
        ADD     r4,r0,#&08              ; source = source + 8
        SUB     r3,r3,r4                ; offset = destination - source
        MOV     r1,r3,LSR #2            ; offset = offset >> 2
        ORR     r1,r1,#(cond_AL :OR: op_b)
        STR     r1,[r0],#&04		; write table slot 0

        ; add table entry for link1 interrupts
        ADRL    r3,LinkInterrupt
        ADD     r4,r0,#&08              ; source = source + 8
        SUB     r3,r3,r4                ; offset = destination - source
        MOV     r1,r3,LSR #2            ; offset = offset >> 2
        ORR     r1,r1,#(cond_AL :OR: op_b)
        STR     r1,[r0],#&04		; write table slot 1

	[	(hercules)
	[	(hercmlink)
        ; add table entry for microlink reception interrupts
        ADRL    r3,ML_Rx_IRQ		; microlink rx IRQ handler
        ADD     r4,r0,#&08              ; source = source + 8
        SUB     r3,r3,r4                ; offset = destination - source
        MOV     r1,r3,LSR #2            ; offset = offset >> 2
        ORR     r1,r1,#(cond_AL :OR: op_b)
        STR     r1,[r0],#&04		; write table slot 2

        ; add table entry for microlink transmission interrupts
        ADRL    r3,ML_Tx_IRQ		; microlink tx IRQ handler
        ADD     r4,r0,#&08              ; source = source + 8
        SUB     r3,r3,r4                ; offset = destination - source
        MOV     r1,r3,LSR #2            ; offset = offset >> 2
        ORR     r1,r1,#(cond_AL :OR: op_b)
        STR     r1,[r0],#&04		; write table slot 3

        ; add table entry for microlink break interrupts
        ADRL    r3,ML_Break_IRQ		; microlink break IRQ handler
        ADD     r4,r0,#&08              ; source = source + 8
        SUB     r3,r3,r4                ; offset = destination - source
        MOV     r1,r3,LSR #2            ; offset = offset >> 2
        ORR     r1,r1,#(cond_AL :OR: op_b)
        STR     r1,[r0],#&04		; write table slot 4
	]	; EOF (hercmlink)

	; ---------------------------------------------------------------------
	|	; middle (hercules)
	; ---------------------------------------------------------------------

	[	(fpmlink)
        ; add table entry for link0 interrupts
        ADRL    r3,ML_Interrupt		; link0 is microlink substitute
        ADD     r4,r0,#&08              ; source = source + 8
        SUB     r3,r3,r4                ; offset = destination - source
        MOV     r1,r3,LSR #2            ; offset = offset >> 2
        ORR     r1,r1,#(cond_AL :OR: op_b)
        STR     r1,[r0],#&04		; write table slot 2
	]	; EOF (fpmlink)
	]	; EOF (hercules)

        ; Setup software vectors (SWI vectors and error vector).
	ADRL	r0,SOFTvectors		; offsets from start of Executive
	LDR	r1,=software_vectors	; location of vectors in workspace
	MOV	r2,#number_of_vectors	; number of vectors in table
	ADRL	r3,codeBase		; start of the Executive in memory
vec_copy_loop
	SUBS	r2,r2,#&01		; decrement the index counter
	LDR	r4,[r0],#&04		; load the offset
	ADD	r4,r3,r4		; get the true address
	STR	r4,[r1],#&04		; and store in the workspace
	BNE	vec_copy_loop

	; Setup the undefined instruction vector
	LDR	r0,SOFTvector_undef		; initial offset from start
	ADD	r0,r3,r0			; convert to a true address
	MOV	r1,#software_vector_undef	; get vector address
	STR	r0,[r1,#&00]			; and initialise it

        ; After this point it is safe to generate aborts, etc.
        ; We have the software vectors in place.

        ; Define USR/IRQ/FIQ and SVC stacks (etc.)
        ; This depends heavily on the memory map:
        ;       FIQ FD stack at top of FIQ memory
        ;       IRQ FD stack in the system workspace area
        ;       SVC FD stack somewhere in memory
        ;       USR FD stack (in user process address space)

        ; NOTE: The IRQ stack is a small fixed memory stack. It need not be
        ;       large since no Executive external routines will execute in
        ;       IRQ mode. The device drivers (and handlers) will be called
	;	in a minimum SVC environment.

        MOV     r0,#(INTflags :OR: FIQmode)
        TEQP    r0,#&00000000           ; enter FIQ mode, IRQs/FIQs disabled
        NOP                             ; wait for registers to be remapped
        LDR     fiq_r13,=top_FIQ_stack

        MOV     r0,#(INTflags :OR: IRQmode)
        TEQP    r0,#&00000000           ; enter IRQ mode, IRQs/FIQs disabled
        NOP                             ; wait for registers to be remapped
	LDR	irq_r13,=top_IRQ_entry	; IRQ handler entry stack

        TEQP    pc,#PSRflags            ; SVC mode, IRQs/FIQs disabled
        NOP                             ; wait for registers to be remapped

        ; Define startup USR mode stack. This is dependent on the requirements
        ; of the C run-time system used to initialise and startup Helios.
	; At the moment we will use the screen memory.
	[	(dynlcd)
	; load sp(r13) with the tier 1 LCD DMA base address.
	MOV	sp,#DMA_base			; DMA hardware registers
	LDR	sp,[sp,#LCDscreen_base0]	; tier 1 DMA descriptor
	MOV	sp,sp,LSR #DMA_src_shift	; get into correct range
	ADD	sp,sp,#LCD_size			; and get the end address

	; ---------------------------------------------------------------------
	|	; middle (dynlcd)
	LDR	sp,=LCD_end
	]	; EOF (dynlcd)

	; The following code sequence must be used since we cannot use
	; write-back on forced USR mode transfers
	LDR	r0,=entryword		; temporary location
	STR	sp,[r0,#&00]		; store desired value into location
	LDMFD	r0,{sp}^		; load into USR r13
	NOP

	; r1 = 0 for the following initialisation code segments
        MOV     r1,#&00000000

        ; Zero link adaptor variables
        ; (Only the "count" variables are important)
        LDR     r0,=link_IRQ_workspace
        STR     r1,[r0,#(txbuffer_count - link_IRQ_workspace)]
        STR     r1,[r0,#(rxbuffer_count - link_IRQ_workspace)]

	; Initialise the IRQ handler thread active count
	LDR	r0,=entered_IRQ
	STR	r1,[r0,#&00]	; no IRQ handler threads active

        ; Ensure that any interrupts that occur during the bootstrap phase
        ; return immediately to the main Helios startup thread.
        ; NOTE: IRQs are NOT enabled at this point.
        LDR     r0,=ROOT_start
        STR     r1,[r0,#ExecRoot_pri]		; this thread is hi-priority
	[	(speedup)
	; Initialise the structure pointers whose addresses we hold in the
	; ExecRoot structure.

	; Reference the Root structure
	LDR	r2,=userRAMbase			; loExecutive variable
	LDR	r2,[r2,#&00]			; start of user RAM
	STR	r2,[r0,#ExecRoot_helios_root]	; as used by "GetRoot()"

	; Reference the SYSBASE structure
	LDR	r2,=SYSBASE_start		; address of SYSBASE structure
	STR	r2,[r0,#ExecRoot_helios_sys]	; as used by "GetSysBase()"

	; Initialise "fast_structure_ptr" to reference the ExecRoot structure
	MOV	r2,#fast_structure_ptr	; directly addressable low address
	STR	r0,[r2,#&00]		; and initialise contents
	]	; EOF (speedup)

	[	(fpmlink :LOR: hercmlink)
	; Initialise microlink variables (by setting the whole area to 0)
	LDR	r0,=ML_Workspace_Start
	LDR	r2,=ML_Workspace_End
ml_ws_init_loop
	STR	r1,[r0],#4		; Clear one word and step
	CMP	r0,r2			; Reached end?
	BLT	ml_ws_init_loop		; No

        [       (hercules)
	; Set up the two buffer pointers
	LDR	r0,=ML_RxBuffer1	; First buffer
	LDR	r1,=ML_Rx_CurBuf	; Address of current buffer pointer
	STR	r0,[r1]			; Set up current pointer
	ADD	r0,r0,#(ML_RxBuffer2 - ML_RxBuffer1); Addr of second buffer
	STR	r0,[r1,#(ML_Rx_OtherBuf - ML_Rx_CurBuf)]; Set up other ptr

        |       ; middle (hercules)

	LDR	r0,=ML_RxBuffer1	; Address of static rx buffer
	LDR	r1,=ML_RxBuf_Ptr	; Buffer pointer variable
	STR	r0,[r1]			; Initialise pointer
        ]       ; EOF (hercules)
	]	; EOF (fpmlink :LOR: hercmlink)

        ; ---------------------------------------------------------------------
	; -- end of (r1 == 0) phase -------------------------------------------
        ; ---------------------------------------------------------------------

	[	(newbreak)
	; We no longer need to register a PowerFail handler with the micro-link
	; device driver.
	|	; middle (newbreak)
	[	((activebook) :LAND: (shutdown))
	; Register the Power-Fail handler
	; -------------------------------
	; The microlink FIQ handler has not yet been initialised, however,
	; when it is attached it will throw away packets for which there
	; is no handler. Since we do not want to lose the Power-Fail event
	; we register our handler here. This relies on the uController end of
	; the protocol blocking messages until the FIQ handler has started.
	;
	; Note: we are still in SVC mode at this point so we can jump into
	;	the microlink control SWIs.
	;
	; We provide an assembler function in "lomlink.s" to deal with
	; the Power-Fail event. The "ML_MsgHdlr" structure we pass must be
	; in RAM (since the microlink handler SWIs/FIQs/IRQs write private
	; information into the structure).
	;
	LDR	r0,=ML_PowerFailBuffer		; message handler structure

	MOV	r1,#MLHdr_PowerFail		; message type (short)
	STR	r1,[r0,#ML_MsgHdlr_MsgType]

	ADRL	r1,PM_PowerFail			; handler function address
	STR	r1,[r0,#ML_MsgHdlr_Func]

	MOV	r1,#&00000000			; no private argument for now
	STR	r1,[r0,#ML_MsgHdlr_Arg]

	MOV	r9,#&FFFFFFFF			; dp = -1 for our function
	; This function expects SVC r13 to hold a valid FD stack. At the
	; moment we just use the same area of memory as we have referenced
	; in the USR r13 register.
	; sp (r13) preserved from setup above
	[	{TRUE}
	MOV	r1,r0
	MOV	r2,r13
	ADR	r0,mltxt3
	BL	local_Output
	MOV	r0,r2
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	r0,r1
	MOV	r13,r2
	B	mlovr3
mltxt3	=	"before local_ML_RegisterHandler: r13 = &",&00
	ALIGN
mlovr3
	]	; EOF {boolean}
	;
	BL	local_ML_RegisterHandler
	!	0,"TODO: error return from ML_RegisterHandler to be coded"
	[	{TRUE}
	MOV	r1,r0
	MOV	r2,r13
	ADR	r0,mltxt2
	BL	local_Output
	MOV	r0,r2
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	r0,r1
	MOV	r13,r2
	B	mlovr2
mltxt2	=	"after local_ML_RegisterHandler: r13 = &",&00
	ALIGN
mlovr2
	]	; EOF {boolean}
	]	; EOF ((activebook) :LAND: (shutdown))
	]	; EOF (newbreak)

	; We do not need an explicit SVC stack in the new world.
	MOV	r13,#&FFFFFFF0		; make sure errors are generated

        ; ---------------------------------------------------------------------
        ; -- RUN-TIME SYSTEM STARTUP ------------------------------------------
        ; ---------------------------------------------------------------------
        ; Enter USR mode for Helios initialisation

        TEQP    pc,#USRmode		; USR mode; IRQs enabled; FIQs enabled
	; We have enabled processor IRQs and FIQs, however, there should not
	; be any interrupt sources enabled at this point.

        MOV     r0,#&00000000
        LDR     r1,=fp_old_vector
        STR     r0,[r1,#&00]		; reset the FPE vector

        ADRL    r0,FPEItemName
        SWI     exec_FindROMItem	; and search for the FPE ROM ITEM
        BVS     no_FPE_item		; if not found then step-over setup

	LDRB	r7,[r0,#ITEMNameLength]	; length of the name field
	ADD	r7,r7,#ITEMName		; plus the offset to the name field
	ADD	r7,r0,r7		; r7 = start of the ROM specific header
	LDR	r9,[r7,#OBJECTInit]	; load the initialisation code offset

        TEQ     r9,#&00000000		; check if code provided
        ADDNE   r9,r9,r0                ; calculate the real address
        MOVNE   r14,pc                  ; remember the return address
        MOVNE   pc,r9                   ; and execute the initialisation code
        ; **** NO INSTRUCTIONS TO BE ADDED HERE ****
no_FPE_item
	; The Floating Point Emulator should now be providing emulation of the
	; Floating Point Co-Processor instructions.
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

	; This code deals with constructing the SYSBASE vector. This is an
	; array of relative pointers to the start of the seperate "Nucleus"
	; objects.
	LDR	r2,=SYSBASE_start	; address of the SYSBASE structure
	ADRL	r3,ObjectNameTable	; offsets to the object names
	MOV	r4,r3			; and keep a copy of the base
	MOV	r5,#&00000000		; a useful constant
	STR	r5,[r2],#&04		; zero the IVecISize word

	; We need to construct a NULL terminated list of relative pointers
	; to the start of the objects. We have an ordered list of the objects
	; we must reference in this "SYSBASE" structure, which we use with
	; "SWI exec_FindROMItem" to access the mapped address of the object.
SYSBASE_build_loop
	LDR	r0,[r3],#&04		; load this table entry
	CMP	r0,#&FFFFFFFF		; check for the end of the list
	BEQ	SYSBASE_build_done
	ADD	r0,r0,r4		; get the actual name text address
	SWI	exec_FindROMItem	; r0 = NULL terminated ASCII filename
	BVS	SYSBASE_build_done	; terminate SYSBASE at this point
	LDR	r1,[r0,#OBJECTOffset]	; get the offset to the actual data
        ADD     r0,r0,r1
	; r0 = address of the actual object file
	; r2 = address of this SYSBASE entry
	SUB	r0,r0,r2		; r0 = RPTR to actual object file
	STR	r0,[r2],#&04		; and step onto the next IVec entry
	B	SYSBASE_build_loop

SYSBASE_build_done
	STR	r5,[r2],#&04		; and fill in the terminating NULL

	; and enter the Nucleus...
        MOV     r0,#&00000000		; a1 = &00000000 ((Channel *)bootlink)
        LDR     r1,=SYSBASE_start	; a2 = SYSBASE   ((word *)loadbase)
	MOV	r2,#&00000000		; a3 = &00000000 (word bootaddr)
        MOV     sl,#&00000000		; zero stack-limit
        MOV     fp,#&00000000		; zero frame-pointer
        MOV     lr,#&00000000           ; USR mode (all PSR bits cleared)
	; sp is already defined as a small FD USR stack (currently screen)
        ; USR mode, IRQs and FIQs enabled
	LDR	r3,[r1,#&04]		; load Kernel RPTR (not offset of 4)
	ADD	r3,r3,r1		; add in the SYSBASE address
	ADD	r3,r3,#(Module_sizeof + 4)	; 4 for offset above

        ; USR mode, IRQs and FIQs enabled
        MOV     pc,r3                   ; and enter the loaded code

	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; ---------------------------------------------------------------------
	; This list of "files" should be in identical order to the entry
	; indices defined in "config.h"/"manifest.s". We should generate some
	; scheme for ensuring this.
	; This is a list of relative addresses to the object name.
	; At the moments there are no checks on the validity of this structure.
ObjectNameTable
	&	(ObjectKernelName - ObjectNameTable)
	&	(ObjectSysLibName - ObjectNameTable)
	&	(ObjectServLibName - ObjectNameTable)
	&	(ObjectUtilName - ObjectNameTable)
	&	(ObjectABClibName - ObjectNameTable)
	&	(ObjectPosixName - ObjectNameTable)
	&	(ObjectCLibName - ObjectNameTable)
	&	(ObjectFaultName - ObjectNameTable)
	&	(ObjectFPLibName - ObjectNameTable)
	&	(ObjectPatchLibName - ObjectNameTable)
	&	(ObjectProcManName - ObjectNameTable)
	&	(ObjectLoaderName - ObjectNameTable)
	[	(kbdserv)
	&	(ObjectKeyboardName - ObjectNameTable)
	]	; EOF (kbdserv)
	&	(ObjectWindowName - ObjectNameTable)
	&	(ObjectRomName - ObjectNameTable)
	&	(ObjectRamName - ObjectNameTable)
	&	(ObjectNullName - ObjectNameTable)
	&	(ObjectHeliosName - ObjectNameTable)
	&	&FFFFFFFF				; list terminator
ObjectNameTableEnd

	; This is the list of names as found in the system ROM.
ObjectKernelName	=	"helios/lib/kernel",null
ObjectSysLibName	=	"helios/lib/syslib",null
ObjectServLibName	=	"helios/lib/servlib",null
ObjectUtilName		=	"helios/lib/util",null
ObjectABClibName	=	"helios/lib/ABClib",null
ObjectPosixName		=	"helios/lib/Posix",null
ObjectCLibName		=	"helios/lib/Clib",null
ObjectFaultName		=	"helios/lib/Fault",null
ObjectFPLibName		=	"helios/lib/FpLib",null
ObjectPatchLibName	=	"helios/lib/patchlib",null
ObjectProcManName	=	"helios/lib/procman",null
ObjectLoaderName	=	"helios/lib/loader",null
	[	(kbdserv)
ObjectKeyboardName	=	"helios/lib/keyboard",null
	]	; EOF (kbdserv)
ObjectWindowName	=	"helios/lib/window",null
ObjectRomName		=	"helios/lib/rom",null
ObjectRamName		=	"helios/lib/ram",null
ObjectNullName		=	"helios/lib/null",null
ObjectHeliosName	=	"helios/lib/helios",null
FPEItemName		=	"helios/lib/FPEmulator",null
        ALIGN		; at the end of the table

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; The default software vector contents. These should be consistent with
        ; the vector numbering scheme defined in "SWI.s".
SOFTvectors
	; These values are the offsets from the start of the Executive
	&	(HeliosExecutiveSWI		- codeBase)
	&	(HeliosSWI			- codeBase)
	&	(otherSWI			- codeBase)
	&	(FatalError			- codeBase)
	&	(defaultCARDHandler		- codeBase)
SOFTvectors_end
	; Ensure all the vectors have default handlers.
	ASSERT	((SOFTvectors_end - SOFTvectors) = (number_of_vectors * 4))

        ; ---------------------------------------------------------------------
	; The "undefined instruction" vector is a special case.
SOFTvector_undef
	; Offset from the start of the Executive
	&	(exec_undefined_instruction	- codeBase)

        ; ---------------------------------------------------------------------

null_instruction
        MOVS    pc,link	; this forms the null entry in the IRQ vectors table

        ; ---------------------------------------------------------------------

vector_undef_instruction	; index through the RAM location
	LDR	pc,[pc,#(software_vector_undef - &04 - &08)]

vector_FIQ_instruction		; default FIQ code (direct return)
	SUBS	pc,fiq_r14,#&04

        ; ---------------------------------------------------------------------

zeroes  ; 8words (32bytes) of fixed NULLs
        &       &00000000
        &       &00000000
        &       &00000000
        &       &00000000
        &       &00000000
        &       &00000000
        &       &00000000
        &       &00000000

        ; ---------------------------------------------------------------------

        LTORG	; storage for PC relative constants

        ; ---------------------------------------------------------------------
        ; -- Software Interrupt (SWI) -----------------------------------------
        ; ---------------------------------------------------------------------
exec_SWI
        ; SWIs can be viewed as a continuation of the current process thread.
        ; Therefore it is valid to use the same stack as the SWI caller.
        ; This system may NOT work very well in the Hercules memory mapped
        ; world. SWI instructions will execute in system mode (with a different
        ; memory map to that of the usual SWI caller).
        ; We need to discover the processor mode of the SWI caller, plus
        ; utilise their stack. We need to preserve svc_r14 as the return
        ; address.
        ; Note: Beware of calling "exec_EnterSVC" from within an SVC thread.
	;
        ; IRQs are currently disabled, and FIQs should never call SWIs,
        ; therefore this code is safe from external register manipulations.
        ; We do not allow IRQ mode code to call SWIs to make this handler
        ; entry a bit simpler, anyway the whole point of these modifications
        ; is to allow interrupt code to run in SVC mode, so no-one but
        ; the initial interrupt handler will be executing in IRQ mode.
	;
        ; switch (processor mode)
        ;  case USR     utilise usr_r13 <= can corrupt svc_r13 during SWI entry
        ;  case FIQ     <<FATAL SYSTEM ERROR>>
        ;  case IRQ     <<FATAL SYSTEM ERROR>>
        ;  case SVC     utilise svc_r13 <= cannot corrupt during SWI entry
	;
	; NOTE: At the moment the code assumes caller SVC mode if not USR mode
	;
        ; check if the bottom 2bits of the return PSR == USRmode
        TST     svc_r14,#SVCmode        ; mask out all but the processor mode
        BNE     caller_SVC              ; not in USR mode when called
caller_USR
        ; We need a SVC stack to perform the entry processing (copy callers).
        ; Note: We use a word dedicated to this task in the static system
	;	workspace. IRQs are still disabled, so no other SVC threads can
        ;       be created until we enable IRQs.
        LDR     sp,=entryword           ; temporary word in system workspace
        STMIA   sp,{usr_sp}^            ; store USR mode r13 at temp word
        NOP                             ; wait for registers to re-map
        LDR     sp,[sp,#&00]            ; and load USR r13 into SVC r13
caller_SVC
        ; We have a FD SVC stack, so continue...
        ; in:   SVC mode; FIQ preserved; IRQ disabled.
        ;       svc_r14 processor mode and return address
        ;       svc_r13 USR mode FD stack

        STMFD   sp!,{r10,r11,r12}       ; stack work registers

        BIC     r12,svc_r14,#PSRflags   ; mask out the PSR
        SUB     r11,r12,#4              ; get address of SWI instruction
	; A processor mode test was performed on entry, the PSR should still
	; be valid here.
        LDREQT  r12,[r11],#0            ; get SWI instruction from user space
        LDRNE   r12,[r11,#0]            ; get SWI instruction unmapped
        BIC     r12,r12,#SWI_insmask    ; and mask out the SWI instruction

	[	{FALSE}		; SWI call debugging
	STMFD	sp!,{r0,lk}
	ADR	r0,estxt1
	BL	local_Output
	MOV	r0,r11			; instruction address
	BL	local_WriteHex8
	ADR	r0,estxt2
	BL	local_Output
	MOV	r0,r12
	BL	local_WriteHex8
	BL	local_NewLine
	LDMFD	sp!,{r0,lk}
	B	esovr1
estxt1	=	"exec_SWI: &",&00
estxt2	=	" &",&00
	ALIGN
esovr1
	]	; EOF {boolean}

        ; Execute through a software vector depending on the setting of the
        ; OS bits and the HE bit.
        MOV     r11,#vec_HeliosExecSWI  ; assume Helios Executive SWI

        TST     r12,#swi_noexec         ; check Executive/non-Executive bit
        MOVNE   r11,#vec_HeliosSWI      ; bit set = Helios application SWI

        AND     r10,r12,#swi_osmask     ; mask out all but the OS identifier
        CMP     r10,#swi_os_helios      ; check OS number
        MOVNE   r11,#vec_otherSWI       ; not a Helios SWI

        BIC     r12,r12,#swi_osmask     ; clear the OS bits

        ; r11 = software vector number
        ; r12 = SWI comment field

        LDR     r10,=software_vectors
        ADD     r11,r10,r11,LSL #2

        LDMFD   sp!,{r10}

        ; r0..r10 : preserved from caller
        ; r11     : software vector entry address
        ; r12     : SWI number (suitably modified for vector)
        ; svc_r13 : FD stack (with entry r11 and r12)
        ; svc_r14 : callers return address
        ; SVC mode; IRQs disabled; FIQs undefined.

        ; and enter the relevant vector
        LDR     pc,[r11,#&00]

        ; ---------------------------------------------------------------------
        ; -- Branch Through Zero ----------------------------------------------
        ; ---------------------------------------------------------------------

exec_branch_through_zero
        ; in:   All registers undefined (idea is to preserve as many as
        ;       possible).
        ;       Processor mode and interrupt state undefined.
        ;
        ; Generate a suitable fatal error:
        ;  We have either wrapped around in memory, or someone has illegally
        ;  jumped here.

	[	((debug2) :LAND: (activebook))
	; corrupt some registers for debugging information
	LDR	sp,=DummySpace
	MOV	r1,r14	; preserve r14
	ADR	r0,btztxt1
	BL	local_Output
	MOV	r0,r1	; really r14
	BL	local_WriteHex8
	BL	local_NewLine
	MOV	r14,r1	; restore r14
	B	btzovr1
btztxt1	=	"Branch_Through_Zero: entered r14 = &",&00
	ALIGN
btzovr1
	]	; EOF ((debug2) :LAND: (activebook))

        ; Assume not in SVC mode (if we are we will corrupt "svc_r14")
        STR     r14,[r0,-r0]            ; over-write the BTZ instruction
        SWI     exec_EnterSVC           ; enter SVC mode from USR mode
					; will corrupt SVC r14 as a result

	[	((debug2) :LAND: (activebook))
	; corrupt some registers for debugging information
	LDR	sp,=DummySpace
	ADR	r0,btztxt2
	BL	local_Output
	MOV	r0,pc
	BL	local_WriteHex8
	BL	local_NewLine
	B	btzovr2
btztxt2	=	"Branch_Through_Zero: SVC pc = &",&00
	ALIGN
btzovr2
	]	; EOF ((debug2) :LAND: (activebook))

        ; Disable IRQs (preserving FIQs)
        MOV     svc_r14,#(Ibit :OR: SVCmode)
        TEQP    svc_r14,#&00            ; SVC mode; IRQs disabled.

        ; Place all the un-mapped registers into the register dump area
        LDR     svc_r14,=register_dump
        STMIA   svc_r14,{r0-r7}

        ; SVC mode; IRQs disabled; FIQs undefined.
        ADRL    r1,abort_btz
        MOV     r0,#sys_btz

        B       generic_handler         ; and generate the error

        ; ---------------------------------------------------------------------
        ; -- Undefined Instruction --------------------------------------------
        ; ---------------------------------------------------------------------

exec_undefined_instruction
        ; in:   SVC mode, FIQ preserved, IRQ disabled
        ;       svc_r14 processor mode and return address (after failed
        ;       instruction)
        ;       all registers (other than svc_r14 and svc_r15) are preserved
        ;
        ; Generate a fatal error for the moment

        TEQP    pc,#PSRflags            ; SVC mode, IRQ and FIQ disabled

        ; temporarily store the svc_r14 (corrupting the reset vector)
        STR     svc_r14,[r0,-r0]

        ; place all the un-mapped registers into the register dump area
        LDR     svc_r14,=register_dump
        STMIA   svc_r14,{r0-r7}

        ADRL    r1,abort_instruction
        MOV     r0,#sys_undefins

        B       generic_handler

        ; ---------------------------------------------------------------------
        ; -- PreFetch Abort ---------------------------------------------------
        ; ---------------------------------------------------------------------

exec_prefetch_abort
        ; in:   SVC mode, FIQ preserved (from aborted process), IRQ disabled
        ;       svc_r14 mode and (return address + 8), after failed instruction
        ;       all registers (other than svc_r14 and svc_r15) are preserved.
        ;
        ; Instruction not present. Page protected or RAM not present.
        ;
        ; At the moment this abort is treated as fatal (since we have no memory
        ; management system). In future Executives we may have to deal with
        ; re-starting instructions that have aborted.

        TEQP    pc,#PSRflags            ; SVC mode, IRQ and FIQ disabled

        ; modify the callers PC (so we can return directly to it)
        SUB     svc_r14,svc_r14,#8

        ; temporarily store the svc_r14 (corrupting the reset vector)
        STR     svc_r14,[r0,-r0]

        ; place all the un-mapped registers into the register dump area
        LDR     svc_r14,=register_dump
        STMIA   svc_r14,{r0-r7}

        ADRL    r1,abort_prefetch
        MOV     r0,#sys_prefetch

        B       generic_handler

        ; ---------------------------------------------------------------------
        ; -- Data Abort -------------------------------------------------------
        ; ---------------------------------------------------------------------

exec_data_abort
        ; in:   SVC mode, FIQ preserved (from aborted process), IRQ disabled
        ;       svc_r14 mode and return (address + 4), after failed instruction
        ;       all registers (other than svc_r14 and svc_r15) are preserved
        ;
        ; Data read/write failed. Page protected or RAM not present.
        ;
        ; See the comment for "exec_prefetch_abort".

        TEQP    pc,#PSRflags            ; SVC mode, IRQ and FIQ disabled

        ; modify the callers PC (so we can return directly to it)
        SUB     svc_r14,svc_r14,#4

        ; temporarily store the svc_r14 (corrupting the reset vector)
        STR     svc_r14,[r0,-r0]

        ; place all the un-mapped registers into the register dump area
        LDR     svc_r14,=register_dump
        STMIA   svc_r14,{r0-r7}

        ADRL    r1,abort_data
        MOV     r0,#sys_dataabort

        B       generic_handler

        ; ---------------------------------------------------------------------
        ; -- Address Exception ------------------------------------------------
        ; ---------------------------------------------------------------------

exec_address_exception
        ; in:  SVC mode, FIQ preserved (from aborted process), IRQ disabled
        ;      svc_r14 mode and (return address + 4) (after failed instruction)
        ;      all registers (other than svc_r14 and svc_r15) are preserved
        ;
        ; Address exception is a constant FATAL error (i.e. there is nothing
        ; to be gained from re-starting the failed code) generate a suitable
        ; error.

        TEQP    pc,#PSRflags            ; SVC mode, IRQ and FIQ disabled

        ; modify the callers PC (so we can return directly to it)
        SUB     svc_r14,svc_r14,#4

        ; temporarily store the svc_r14 (corrupting the reset vector)
        STR     svc_r14,[r0,-r0]

        ; place all the un-mapped registers into the register dump area
        LDR     svc_r14,=register_dump
        STMIA   svc_r14,{r0-r7}

        ADRL    r1,abort_address
        MOV     r0,#sys_address         ; address exception error code

        ; and fall through to the "generic_handler"

        ; ---------------------------------------------------------------------

generic_handler
        ; Preserve mapped registers and enter the system error handler.
        ; in:   r0  = 32bit error number
        ;       r1  = NULL terminated ASCII error message
        ;       r14 = register_dump
        ;       Aborted r0..r7 are stored at "register_dump"
        ;       SVC mode; IRQ disabled; FIQ undefined.

        ; place the aborted PC into the save structure
        LDR     r2,[r0,-r0]
        STR     r2,[svc_r14,#(dumped_r15 - register_dump)]

        LDR     r3,=dumped_r8           ; index for the (possibly) mapped regs

        TST     r2,#SVCmode
        STMEQIA r3,{r8-r14}^            ; USR mode aborted
        BEQ     continue

        TST     r2,#IRQmode
        TSTNE   r2,#FIQmode
        BNE     wasSVCmode
        ; IRQ or FIQ mode abort
        ORR     r2,r2,#INTflags         ; ensure IRQ/FIQ disabled
        TEQP    r2,#&00                 ; and enter the aborted mode
        NOP                             ; allow time for register remapping
        STMIA   r3,{r8-r14}             ; and stack the correct registers
        TEQP    pc,#PSRflags            ; and re-enter SVC mode (IRQ/FIQ off)
        B       continue

wasSVCmode      ; we were in SVC mode when the abort occured
        STMIA   r3,{r8-r13}             ; and stack the easy registers
        LDR     r2,=&DEADDEAD           ; we have corrupted svc_r14
        STR     r2,[r3,#(dumped_r14 - dumped_r8)]
        LDR     r2,[r2,-r2]             ; recover the aborted PC+PSR
        STR     r2,[r3,#(dumped_r15 - dumped_r8)]
continue
        ; registers dumped into "register_dump"
        ; SVC mode; IRQs and FIQs disabled

        ; We can restore the aborted processes FIQ mode, since if we were
        ; in FIQ mode when the abort occured, FIQs should have been disabled.
        ; Otherwise it is better to preserve the callers FIQ state.
        ; (NOTE: r3 currently points to the r8 value)
        LDR     r2,[r3,#(dumped_r15 - dumped_r8)]

        ; Also we should enable IRQs again when we enter the error vector
        ; since we (used to) need to refresh RAM. 
        ORR     r2,r2,#(Ibit :OR: SVCmode)
        TEQP    r2,#&00                 ; SVC; IRQs disabled; FIQs as caller

        LDR     svc_r14,[r0,-r0]        ; and recover return PC and PSR

        ; and restore the "branch through zero" code
	; This code should be the same as that used to initialise the
	; vector during RESET.
	MOV	r3,#&00000000		; where the vectors live
	ADRL	r2,exec_branch_through_zero
	ADD	r4,r3,#&08		; source = source + 8
	SUB	r2,r2,r4		; offset = destination - source
	MOV	r2,r2,LSR #2		; offset = offset >> 2
	ORR	r2,r2,#(cond_AL :OR: op_b)
	STR	r2,[r3],#&04		; branch-through-zero vector

	CMP	r0,#sys_btz		; branch through zero error
	MOVEQ	r3,#-1			; this abort is non-recoverable
	MOVNE	r3,#0			; all others are recoverable (possibly)
        B       enter_error_vector

        ; ---------------------------------------------------------------------

	GBLS	conditional
	[	(monitor)
conditional	SETS	""
	|	; middle (monitor)
conditional	SETS	";"
	]	; EOF (monitor)
	$conditional	GET	lomon.s	; include the ROM monitor code

        ; ---------------------------------------------------------------------
        ; -- software abort messages ------------------------------------------
        ; ---------------------------------------------------------------------

message_unknownSWI
        =       "Unknown Executive SWI",null

message_heliosSWI
message_otherSWI
        =       "No SWI extensions implemented",null

message_halted
        =       "Executive halted",null

        ; ---------------------------------------------------------------------
        ; -- hardware abort messages ------------------------------------------
        ; ---------------------------------------------------------------------

abort_btz
        =       "Branch through zero",null

abort_instruction
        =       "Undefined instruction",null

abort_address
        =       "Address exception",null

abort_data
        =       "Data abort",null

abort_prefetch
        =       "Instruction pre-fetch abort",null

        ALIGN           ; after all the messages

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        LNK     loint.s
