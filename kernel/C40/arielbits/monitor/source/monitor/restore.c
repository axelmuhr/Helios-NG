#include "hydra.h"


void restore( hydra_conf config )
{
	int i=0, j;


	SetupVICVACDefault();

	WriteEeprom( i++, 0x01, config.l_jtag_base + 8 ); 	/* Address 0x27 */	
	WriteEeprom( i++, 0xEA, config.l_jtag_base + 8 ); 	/* Address 0x47 */	
	WriteEeprom( i++, 0x0F, config.l_jtag_base + 8 );	/* Address 0x57 */
	WriteEeprom( i++, 0x56, config.l_jtag_base + 8 );	/* Address 0xA7 */	
	WriteEeprom( i++, 0x70, config.l_jtag_base + 8 );	/* Address 0xB3 */	
	WriteEeprom( i++, 0x0D, config.l_jtag_base + 8 );	/* Address 0xB7 */
	WriteEeprom( i++, 0x10, config.l_jtag_base + 8 );	/* Address 0xC3 */
	WriteEeprom( i++, 0xFF, config.l_jtag_base + 8 );	/* Address 0xC7 */
	WriteEeprom( i++, 0x14, config.l_jtag_base + 8 );	/* Address 0xCB */
	WriteEeprom( i++, 0xFF, config.l_jtag_base + 8 );	/* Address 0xCF */
	i++;

	WriteEepromWord( i, 0xFFFF, config );	/* Address 0x00 */
	i += 2;
	WriteEepromWord( i, 0xFFFE, config );	/* Address 0x01 */
	i += 2;
	WriteEepromWord( i, 0xFF00, config );	/* Address 0x02 */
	i += 2;
	WriteEepromWord( i, 0x3400, config );	/* Address 0x03 */
	i += 2;
	WriteEepromWord( i, 0xF0F1, config );	/* Address 0x04 */
	i += 2;
	WriteEepromWord( i, 0x0001, config );	/* Address 0x05 */
	i += 2;
	WriteEepromWord( i, 0x0002, config );	/* Address 0x06 */
	i += 2;
	WriteEepromWord( i, 0xFDFF, config );	/* Address 0x07 */
	i += 2;
	WriteEepromWord( i, 0x7F00, config );	/* Address 0x08 */
	i += 2;
	WriteEepromWord( i, 0x2C00, config );	/* Address 0x09 */
	i += 2;
	WriteEepromWord( i, 0x4C00, config );	/* Address 0x0A */
	i += 2;
	WriteEepromWord( i, 0x0400, config );	/* Address 0x0B */
	i += 2;
	WriteEepromWord( i, 0xF810, config );	/* Address 0x0C */
	i += 2;
	WriteEepromWord( i, 0xF810, config );	/* Address 0x0D */
	i += 2;
	WriteEepromWord( i, 0xF110, config );	/* Address 0x0E */
	i += 2;
	WriteEepromWord( i, 0xF810, config );	/* Address 0x0F */
	i += 2;
	WriteEepromWord( i, 0xE014, config );	/* Address 0x10 */
	i += 2;
	WriteEepromWord( i, 0xF810, config );	/* Address 0x11 */
	i += 2;
	WriteEepromWord( i, 0xF810, config );	/* Address 0x12 */
	i += 2;
	WriteEepromWord( i, 0xF810, config );	/* Address 0x13 */
	i += 2;
	WriteEepromWord( i, 0xEBFE, config );	/* Address 0x14 */
	i += 2;
	WriteEepromWord( i, 0x0010, config );	/* Address 0x16 */
	i += 2;
	WriteEepromWord( i, 0x4000, config );	/* Address 0x1A */
	i += 2;
	WriteEepromWord( i, 0x1FFF, config );	/* Address 0x1B */
	i += 2;
	WriteEepromWord( i, 0x4200, config );	/* Address 0x1C */
	i += 2;
	WriteEepromWord( i, 0x77C0, config );	/* Address 0x1D */
	i += 2;
	WriteEepromWord( i, 0x77C0, config );	/* Address 0x1F */
	i += 2;
	WriteEepromWord( i, 0x8000, config );	/* Address 0x23 */
	i += 2;
	WriteEepromWord( i, 0x1, config );	/* DRAM size */
	i += 2;
	WriteEepromWord( i, 0, config );	/* Daughter present */
	i += 2;
	WriteEepromWord( i, 16, config );	/* SRAM1 size */
	i += 2;
	WriteEepromWord( i, 16, config );	/* SRAM2 size */
	i += 2;
	WriteEepromWord( i, 16, config );	/* SRAM3 size */
	i += 2;
	WriteEepromWord( i, 16, config );	/* SRAM4 size */
	i+=2;
}
