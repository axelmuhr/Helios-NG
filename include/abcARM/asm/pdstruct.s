        SUBT Helios Executive PowerDown structures                 > pdstruct/s
        ;    Copyright (c) 1989, Active Book Company, Cambridge, United Kingdom
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; Helios Executive PowerDown data structures
        ; ------------------------------------------
	;
	; At the moment this file is PRIVATE to the Executive.
        ;
        ; If any external references to these structures are required (other
        ; than by assembler Executive routines) then a suitable C header must
        ; be generated. These headers must be kept in step with this file (and
        ; should ideally be generated from a single source).
        ;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        ASSERT  (structs_s)     ; ensure "structs.s" is included

old_opt SETA    {OPT}
        OPT     (opt_off)

        ; ---------------------------------------------------------------------

        !       0,"Including pdstruct.s"
                GBLL    pdstruct_s
pdstruct_s      SETL    {TRUE}

        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
	; processor saved state:

	; This structure will need updating to provide storage space for
	; a COMPLETE soft-copy of the Hercules state (that is not already
	; stored in normal RAM). ie. FastRAM and any registers for which
	; we are not already holding soft-copies.

	[	((activebook) :LAND: (shutdown))
	!	0,"TODO: ******* update CompleteState structure definition"

        struct          "CompleteState"
	; -- Processor registers ----------------------------------------------
        struct_word     "r0"
        struct_word     "r1"
        struct_word     "r2"
        struct_word     "r3"
        struct_word     "r4"
        struct_word     "r5"
        struct_word     "r6"
        struct_word     "r7"
        struct_word     "r8"
        struct_word     "r9"
        struct_word     "r10"
        struct_word     "r11"
        struct_word     "r12"
	;
	; -- SVC mode mapped registers ----------------------------------------
	struct_word	"SVC_r13"
	struct_word	"SVC_r14"
	;
	; -- PC + PSR ---------------------------------------------------------
        struct_word     "r15"
	;
	; -- USR mode mapped registers ----------------------------------------
        struct_word     "USR_r13"
        struct_word     "USR_r14"
	;
	; -- FIQ mode mapped registers ----------------------------------------
	struct_word	"FIQ_r8"
	struct_word	"FIQ_r9"
	struct_word	"FIQ_r10"
	struct_word	"FIQ_r11"
	struct_word	"FIQ_r12"
	struct_word	"FIQ_r13"
	struct_word	"FIQ_r14"
	;
	; -- IRQ mode mapped registers ----------------------------------------
	struct_word	"IRQ_r13"
	struct_word	"IRQ_r14"
	;
	; -- FastRAM ----------------------------------------------------------
	; NOTE: This includes the current processor vector contents.
	struct_vec	SRAM_size,"FastRAM"
	;
	; -- Hercules registers -----------------------------------------------
	; Note: we cannot use the existing memory allocation for the Hercules
	; soft-copies, since this is initialised during the common RESET code
	; (ie. to allow RAM to be sized).

	! 0,"NOTE: MicroLink hardware state is NOT preserved over PowerDown"

	; -- External IO registers (not preserved elsewhere) ------------------
	; Normally the specific device driver will deal with saving and
	; restoring the hardware state that it deals with. This structure
	; should only hold information about hardware under the control of the
	; Executive.
	;
	; The "inmos link" registers should possibly be conditional on wether
	; the hardware exists. At the moment we can just preserve those
	; locations anyway.
	struct_word	"LINK0_rstatus"
	struct_word	"LINK0_wstatus"
	struct_word	"LINK1_rstatus"
	struct_word	"LINK1_wstatus"
	;
	; -- MMU segment mapping ----------------------------------------------
	struct_word	"segment0"	;  0- 2MB
	struct_word	"segment1"	;  2- 4MB
	struct_word	"segment2"	;  4- 6MB
	struct_word	"segment3"	;  6- 8MB
	struct_word	"segment4"	;  8-10MB
	struct_word	"segment5"	; 10-12MB
	struct_word	"segment6"	; 12-14MB
	struct_word	"segment7"	; 14-16MB
	struct_word	"segment8"	; 16-18MB
	struct_word	"segment9"	; 18-20MB
	struct_word	"segmentA"	; 20-22MB
	struct_word	"segmentB"	; 22-24MB
	struct_word	"segmentC"	; 24-26MB
	struct_word	"segmentD"	; 26-28MB
	struct_word	"segmentE"	; 28-30MB
	struct_word	"segmentF"	; 30-32MB
	;
	; -- DMA registers ----------------------------------------------------
	struct_word	"DMAch0base0"	; dedicated LCD DMA channel
	struct_word	"DMAch0ptr0"
	struct_word	"DMAch0base1"
	struct_word	"DMAch0ptr1"
	struct_word	"DMAch1base0"
	struct_word	"DMAch1ptr0"
	struct_word	"DMAch1base1"
	struct_word	"DMAch1ptr1"
	struct_word	"DMAch2base0"
	struct_word	"DMAch2ptr0"
	struct_word	"DMAch2base1"
	struct_word	"DMAch2ptr1"
	struct_word	"DMAch3base0"
	struct_word	"DMAch3ptr0"
	struct_word	"DMAch3base1"
	struct_word	"DMAch3ptr1"
	;
	; -- CheckSum ---------------------------------------------------------
	struct_word	"checksum"	; simple checksum of data held
	;
	; ---------------------------------------------------------------------
        struct_end	; EOF CompleteState structure definition
	]	; EOF ((activebook) :LAND: (shutdown))

	; Notes
	; -----
	; This structure is only ever used to store the complete processor
	; register set, when we are about to Power-Down the processor.
	; The SVC state is always returned to when restoring the registers
	; from this structure.
	;
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------
        ; ---------------------------------------------------------------------

        OPT     (old_opt)

        ; ---------------------------------------------------------------------
        END     ; EOF pdstruct/s
