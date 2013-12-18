main()
{
	int i=0, j;


	SetupVICVACDefault();

	WriteEeprom( i++, 0x01 ); 	/* Address 0x27 */	
	WriteEeprom( i++, 0xEA ); 	/* Address 0x47 */	
	WriteEeprom( i++, 0x0F );	/* Address 0x57 */
	WriteEeprom( i++, 0x56 );	/* Address 0xA7 */	
	WriteEeprom( i++, 0x70 );	/* Address 0xB3 */	
	WriteEeprom( i++, 0x0D );	/* Address 0xB7 */
	WriteEeprom( i++, 0x10 );	/* Address 0xC3 */
	WriteEeprom( i++, 0xFF );	/* Address 0xC7 */
	WriteEeprom( i++, 0x14 );	/* Address 0xCB */
	WriteEeprom( i++, 0xFF );	/* Address 0xCF */
	i++;

	WriteEepromWord( i, 0xFFFF );	/* Address 0x00 */
	i += 2;
	WriteEepromWord( i, 0xFFFE );	/* Address 0x01 */
	i += 2;
	WriteEepromWord( i, 0xFF00 );	/* Address 0x02 */
	i += 2;
	WriteEepromWord( i, 0x3400 );	/* Address 0x03 */
	i += 2;
	WriteEepromWord( i, 0xF0F1 );	/* Address 0x04 */
	i += 2;
	WriteEepromWord( i, 0x0001 );	/* Address 0x05 */
	i += 2;
	WriteEepromWord( i, 0x0002 );	/* Address 0x06 */
	i += 2;
	WriteEepromWord( i, 0xFDFF );	/* Address 0x07 */
	i += 2;
	WriteEepromWord( i, 0x7F00 );	/* Address 0x08 */
	i += 2;
	WriteEepromWord( i, 0x2C00 );	/* Address 0x09 */
	i += 2;
	WriteEepromWord( i, 0x4C00 );	/* Address 0x0A */
	i += 2;
	WriteEepromWord( i, 0x0400 );	/* Address 0x0B */
	i += 2;
	WriteEepromWord( i, 0xF810 );	/* Address 0x0C */
	i += 2;
	WriteEepromWord( i, 0xF810 );	/* Address 0x0D */
	i += 2;
	WriteEepromWord( i, 0xF110 );	/* Address 0x0E */
	i += 2;
	WriteEepromWord( i, 0xF810 );	/* Address 0x0F */
	i += 2;
	WriteEepromWord( i, 0xE014 );	/* Address 0x10 */
	i += 2;
	WriteEepromWord( i, 0xF810 );	/* Address 0x11 */
	i += 2;
	WriteEepromWord( i, 0xF810 );	/* Address 0x12 */
	i += 2;
	WriteEepromWord( i, 0xF810 );	/* Address 0x13 */
	i += 2;
	WriteEepromWord( i, 0xEBFE );	/* Address 0x14 */
	i += 2;
	WriteEepromWord( i, 0x0010 );	/* Address 0x16 */
	i += 2;
	WriteEepromWord( i, 0x4000 );	/* Address 0x1A */
	i += 2;
	WriteEepromWord( i, 0x1FFF );	/* Address 0x1B */
	i += 2;
	WriteEepromWord( i, 0x4200 );	/* Address 0x1C */
	i += 2;
	WriteEepromWord( i, 0x77C0 );	/* Address 0x1D */
	i += 2;
	WriteEepromWord( i, 0x77C0 );	/* Address 0x1F */
	i += 2;
	WriteEepromWord( i, 0x8000 );	/* Address 0x23 */
	i += 2;
	WriteEepromWord( i, 0x1 );	/* DRAM size */
	i += 2;
	WriteEepromWord( i, 0 );	/* Daughter present */
	i += 2;
	WriteEepromWord( i, 64 );	/* SRAM1 size */
	i += 2;
	WriteEepromWord( i, 64 );	/* SRAM2 size */
	i += 2;
	WriteEepromWord( i, 64 );	/* SRAM3 size */
	i += 2;
	WriteEepromWord( i, 64 );	/* SRAM4 size */
	i+=2;
	WriteEeprom( i, 'B' );	/* Board revision */
}



void WriteEepromWord( int WordAddress, unsigned long data )
{
	int i;
	
	for( i=0 ; i < 2 ; i++, data >>= 8 )
		WriteEeprom( WordAddress++, data & 0xff ); 
}