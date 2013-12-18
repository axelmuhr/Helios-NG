#include "hydra.h"

hydra_conf config;


main()
{
	setupVICVAC();

	config.uartA.baud = 7;
	config.uartB.baud = 7;
	config.uartA.parity = 1;
	config.uartB.parity = 1;
	config.uartA.bits = 7;
	config.uartB.bits = 7;
	config.cpu_clock = 40;
	config.sram1_size = 64;
	config.sram2_size = 64;
	config.sram3_size = 64;
	config.sram4_size = 64;
	config.dram_size = 1;
	config.daughter = 0;
	config.l_jtag_base = 0xdeadbeef;
	config.l_dram_base = 0x8d000000;

	test( config, 't' );
}
