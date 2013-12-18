void CommFlush( hydra_conf config, int Which )
{
	unsigned long ContRegVal, val;

	ContRegVal = (*(unsigned long *)(config.l_jtag_base + 8));

	/* Return if processor is not running (examine Which's reset line in the control reg) */
	if( !(ContRegVal & (0x10000000<<Which)) )
		return;

	/* Halt input channel */
	*(unsigned long *)(0x100040+(Which*0x10)) = 0x8;

	/* Set the NMI high */
	*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<Which);

	/* Now set them low, thusly triggering an interrupt */
	*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal & ~(0x00040000<<Which);

	/* Set the NMI high again */
	*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<Which);

	/* Flush input section */
	while( *(unsigned long *)(0x100040+(Which*0x10)) & 0x00001E00 )
		val = *(unsigned long *)(0x100041+(Which*0x10));
	/* Output garbage to insure that DSP 1 owns the token */
	*(unsigned long *)(0x100042+(Which*0x10)) = 0;

	/* Wait until the FIFO is empty */
	while( *(unsigned long *)(0x100040+(Which*0x10)) & 0x000001E0 );

	/* Now set them low, thusly triggering an interrupt */
	*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal & ~(0x00040000<<Which);

	/* Set the NMI high again */
	*(unsigned long *)(config.l_jtag_base + 8) = ContRegVal | (0x00040000<<Which);

	/* Unhalt input channel */
	*(unsigned long *)(0x100040+(Which*0x10)) = 0x0;
}
