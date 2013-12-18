/******************************************************************************
*******************************************************************************
	A George Kalwitz Production, 1990
*******************************************************************************
******************************************************************************/

#include	"common.h"
#include	"board_id.h"
#include	"diagdata.h"
#include	"params.h"
#include	"err_cods.h"
#include	"misc.h"
#include	"eth_data.h"
#include	"getcnfg.h"

#if	defined(MCDGS)
extern	CNFG_Adapter	config_table;
#else
CNFG_Adapter	config_table;
#endif

#if	defined(ATDGS)
extern	int	zero_wait_state;
#endif
/****************************************************************************

this gathers information from the POS registers 

****************************************************************************/
#if	defined(ATDGS)
#else
get_POS_info()
{
#if	defined(DIAG)
    if (batch_type & B_PARM)
        return;
#endif

	config_table.cnfg_bus = 1;
	config_table.cnfg_slot = channel_pos+1;
	WDM_GetCnfg (&config_table);
	baseio = (unsigned int) config_table.cnfg_base_io;
	irq = config_table.cnfg_irq_line;
	RAMbase = config_table.cnfg_ram_base << 12;
	RAMsize = (unsigned long) config_table.cnfg_ram_size * 0x400;
	ROMbase = config_table.cnfg_rom_base << 12;
	ROMsize = (unsigned long) config_table.cnfg_rom_size * 0x400;

#if	defined(DIAG)
    if ((ROMsize == 0) && ((ROMbase < 0xC0000000) || (ROMbase >= 0xE0000000)))
        ROMbase = 0xC0000000;
#endif
}
#endif

#if	defined(MCDGS)
#else
/****************************************************************************

this gathers information from the 583

***************************************************************************/
get_at_info ()
{
	config_table.cnfg_bus = 0;
	config_table.cnfg_base_io = (unsigned int) baseio;
	WDM_GetCnfg (&config_table);
	if (config_table.cnfg_mode_bits1 & INTERRUPT_STATUS_BIT)
		interrupt_status = 1;
	else
		interrupt_status = 0;
#if	defined(DIAG)
	if (batch_type & B_PARM)
		return;
#endif
	irq = config_table.cnfg_irq_line;
	RAMbase = config_table.cnfg_ram_base << 12;
	RAMsize = (unsigned long) config_table.cnfg_ram_size * 0x400;
	ROMbase = config_table.cnfg_rom_base << 12;
	ROMsize = (unsigned long) config_table.cnfg_rom_size * 0x400;
#if	defined(ATDGS)
	if (config_table.cnfg_mode_bits1 & BOOT_STATUS_MASK)
		allow_boot = 1;
	else
		allow_boot = 0;
#endif
#if	defined(DIAG)
	if ((ROMsize == 0) &&
			((ROMbase < 0xC0000000) || (ROMbase >= 0xE0000000)))
		ROMbase = 0xC0000000;
#endif
#if	defined(ATDGS)
/*	if (config_table.cnfg_mode_bits1 & ZERO_WAIT_STATE_MASK)
		zero_wait_state = 1;
	else
*/
		zero_wait_state = 0;
#endif
}
#endif

#if	defined(DIAG) || defined(ATDGS)
/******************************************************************************


******************************************************************************/
install_at_info ()
{
        is_ram_enabled = 0;
#if	defined(ATDGS)
	RAMsize = 0x2000;
	ROMsize = 0x4000;
	config_table.cnfg_mode_bits1 &= ~BOOT_STATUS_MASK;
	if (allow_boot)
		config_table.cnfg_mode_bits1 |= BOOT_TYPE_1;
#endif
	interrupt_status = 1;
	config_table.cnfg_mode_bits1 &= ~INTERRUPT_STATUS_BIT;
	if (interrupt_status)
		config_table.cnfg_mode_bits1 |= INTERRUPT_STATUS_BIT;
#if	defined(ATDGS)
	config_table.cnfg_mode_bits1 &= ~ZERO_WAIT_STATE_MASK;
/*	if (zero_wait_state)
		config_table.cnfg_mode_bits1 |= ZERO_WAIT_STATE_MASK;
*/
#endif
	config_table.cnfg_bus = 0;
	config_table.cnfg_base_io = (unsigned int) old_baseio;
	config_table.cnfg_bio_new = (unsigned int) baseio;
	config_table.cnfg_irq_line = irq;
	config_table.cnfg_ram_base = (unsigned long) RAMbase >> 12;
	config_table.cnfg_ram_size = (unsigned int) (RAMsize / 0x400);
	config_table.cnfg_rom_base = (unsigned long) ROMbase >> 12;
	config_table.cnfg_rom_size = (unsigned int) (ROMsize / 0x400);
	WDM_PutCnfg (&config_table);
}
#endif

#if	defined(DIAG)
/******************************************************************************


******************************************************************************/
install_POS_info ()
{
	config_table.cnfg_bus = 1;
	config_table.cnfg_slot = channel_pos+1;
	config_table.cnfg_base_io = (unsigned int) baseio;
	config_table.cnfg_irq_line = irq;
	config_table.cnfg_ram_base = (unsigned long) RAMbase >> 12;
	config_table.cnfg_ram_size = (unsigned int) (RAMsize / 0x400);
	config_table.cnfg_rom_base = (unsigned long) ROMbase >> 12;
	config_table.cnfg_rom_size = (unsigned int) (ROMsize / 0x400);
	WDM_PutCnfg (&config_table);
}
#endif
