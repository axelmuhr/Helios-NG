#define RED	1
#define GREEN	2
#define ON	1
#define OFF	0



void LED( int which, int on_off )
{

	switch( which )
	{
		case GREEN:
			switch( on_off )
			{
				case ON:
					*((unsigned long *)0xBF7FC008) |= 0x80000000;
					break;
				case OFF:
					*((unsigned long *)0xBF7FC008) &= 0x7FFFFFFF;
					break;
			};
			break;
		case RED:
			switch( on_off )
			{
				case ON:
					*((unsigned long *)0xBF7FC008) |= 0x00200000;
					break;
				case OFF:
					*((unsigned long *)0xBF7FC008) &= 0xFFDFFFFF;
					break;
			};
			break;
	}
}