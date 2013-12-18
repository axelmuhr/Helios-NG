main( )
{
	unsigned long fail_addr;

	c40_printf("Testing SRAM....  ");
 	if (fail_addr=dmemtest(0x40000000,0xc0000000,16000)
	 	c40_printf("fail at address %x\n",fail_addr);
	else 
		c40_printf ("Passed\n");
}

