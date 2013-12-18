#include "hydra.h"

hydra_conf config;      /* Holds the EEPROM configuration information */

extern void c_int14( void );
extern void c_int15( void );

main( void )
{
	unsigned long val;
	int i, j;
	

	GIEOff();

	SetIntTable( 0x40000c00 );
	SetIntVect( IIOF2, c_int14 );
	EnableInt( IIOF2 );

	SetTrapTable( 0x40000c00 );
	SetTrapVect( 0x7, c_int15 );

	ClearIIOF();

	GIEOn();

	SetupVICVACDefault();

	config.l_jtag_base = 0xbf7fc000;

	read_config( &config ); 
	setupVICVAC( config );

	InitHost( config );

}
