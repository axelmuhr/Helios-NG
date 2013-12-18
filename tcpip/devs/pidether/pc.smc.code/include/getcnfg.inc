;******************************************************************************
; This describes the WDM_GetCnfg interface structure.
;     When passing control to the WDM_GetCnfg routine, DS:BP must
;      point to the memory allocated for this structure.
;     If no error occurs, this memory will be updated to reflect the
;      configuration of the adapter defined by this structure.
;******************************************************************************
CNFG_Adapter    struc

	cnfg_bid        dw     0  ; Board ID from GetBoardID
	cnfg_extra_info dw     0  ; Extra Information from GetBoardID
	cnfg_bus        dw     0  ; 0=AT...1=MCA
	cnfg_base_io    dw     0  ; Adapter Base I/O Address
	cnfg_slot       dw     0  ; Micro Channel Slot Number
	cnfg_ram_base	dd     0  ; 32-Bit Physical Address of Shared RAM
	cnfg_ram_size   dw     0  ; Size of Shared RAM (num of 1KB blocks)
	cnfg_irq_line   dw     0  ; Adapter IRQ Interrupt Line
	cnfg_rom_base   dd     0  ; 32-Bit Physical Address of Adapter ROM
	cnfg_rom_size   dw     0  ; Size of Adapter ROM (num of 1KB blocks)
	cnfg_bio_new	dw     0  ; New Base I/O Address (for PutCnfg)
	cnfg_mode_bits1	dw     0  ; Mode bits for adapter (see GETCNFG.EQU)
CNFG_Adapter    ends

;******************************************************************************
; This describes definitions in the WDM_GetCnfg interface structure.
;******************************************************************************
; Definitions for the field:
;	cnfg_mode_bits1
INTERRUPT_STATUS_BIT	equ	08000h	; PC Interrupt Line: 0 = Not Enabled
BOOT_STATUS_MASK	equ	06000h	; Mask to isolate BOOT_STATUS
BOOT_INHIBIT		equ	00000h	; BOOT_STATUS is 'inhibited'
BOOT_TYPE_1		equ	02000h	; Unused BOOT_STATUS value
BOOT_TYPE_2		equ	04000h	; Unused BOOT_STATUS value
BOOT_TYPE_3		equ	06000h	; Unused BOOT_STATUS value
ZERO_WAIT_STATE_MASK	equ	01800h	; Mask to isolate Wait State flags
ZERO_WAIT_STATE_8_BIT	equ	01000h	; 0 = Disabled (Inserts Wait States)
ZERO_WAIT_STATE_16_BIT	equ	00800h	; 0 = Disabled (Inserts Wait States)
