#include "hydra.h"

void LED( int which, int on_off, hydra_conf config )
{

	switch( which )
	{
		case GREEN:
			switch( on_off )
			{
				case ON:
					*(unsigned long *)(config.l_jtag_base + 8) |= 0x80000000;
					break;
				case OFF:
					*(unsigned long *)(config.l_jtag_base + 8) &= 0x7FFFFFFF;
					break;
			};
			break;
		case RED:
			switch( on_off )
			{
				case ON:
					*(unsigned long *)(config.l_jtag_base + 8) |= 0x00200000;
					break;
				case OFF:
					*(unsigned long *)(config.l_jtag_base + 8) &= 0xFFDFFFFF;
					break;
			};
			break;
	}
}
