/******************************************************************************
  This describes the WDM_GetCnfg interface structure.
      When passing control to the WDM_GetCnfg routine, DS:BP must
       point to the memory allocated for this structure.
      If no error occurs, this memory will be updated to reflect the
       configuration of the adapter defined by this structure.
******************************************************************************/
typedef	struct
{
	unsigned short cnfg_bid;	/* Board ID from GetBoardID */
	unsigned short cnfg_extra_info;	/* Extra Information from GetBoardID */
	unsigned short cnfg_bus;	/* 0=AT...1=MCA */
	unsigned short cnfg_base_io;	/* Adapter Base I/O Address */
	unsigned short cnfg_slot;	/* Micro Channel Slot Number */
	unsigned long  cnfg_ram_base;	/* 32-Bit Phys Address of Shared RAM */
	unsigned short cnfg_ram_size;	/* Shared RAM Size (# of 1KB blocks) */
	unsigned short cnfg_irq_line;	/* Adapter IRQ Interrupt Line */
	unsigned long  cnfg_rom_base;	/* 32-Bit Phys Address of Adapter ROM */
	unsigned short cnfg_rom_size;	/* Adapter ROM Size (# of 1KB blocks) */
	unsigned short cnfg_bio_new;	/* New Base I/O Address (for PutCnfg) */
	unsigned short cnfg_mode_bits1;	/* Mode bits for adapter (see below) */
} CNFG_Adapter;

/********* Prototype Declaration ***********/
int	WDM_GetCnfg (CNFG_Adapter far *);

/******************************************************************************
 This describes definitions in the WDM_GetCnfg interface structure.
******************************************************************************/
/******************************************************************************
 Definitions for the field:
	cnfg_mode_bits1
******************************************************************************/
#define	INTERRUPT_STATUS_BIT	0x8000	/* PC Interrupt Line: 0 = Not Enabled */
#define	BOOT_STATUS_MASK	0x6000	/* Mask to isolate BOOT_STATUS */
#define	BOOT_INHIBIT		0x0000	/* BOOT_STATUS is 'inhibited' */
#define	BOOT_TYPE_1		0x2000	/* Unused BOOT_STATUS value */
#define	BOOT_TYPE_2		0x4000	/* Unused BOOT_STATUS value */
#define	BOOT_TYPE_3		0x6000	/* Unused BOOT_STATUS value */
#define	ZERO_WAIT_STATE_MASK	0x1800	/* Mask to isolate Wait State flags */
#define	ZERO_WAIT_STATE_8_BIT	0x1000	/* 0 = Disabled (Inserts Wait States) */
#define	ZERO_WAIT_STATE_16_BIT	0x0800	/* 0 = Disabled (Inserts Wait States) */

