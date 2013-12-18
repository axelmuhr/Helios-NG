#include "hydra.h"



main()
{
	char i;


	WriteEeprom( 0xf, 1 );

	i = ReadEeprom( 1 );


	WriteEeprom( 0xf, 2 );

	i = ReadEeprom( 2 );


	WriteEeprom( 0x55, 3 );

	i = ReadEeprom( 3 );


	WriteEeprom( 0xaa, 0xff );

	i = ReadEeprom( 0xff );


	WriteEeprom( 120, 10 );

	i = ReadEeprom( 10 );
}
