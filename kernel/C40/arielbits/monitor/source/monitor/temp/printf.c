#include "hydra.h"
#include "ctype.h"
#include "stdarg.h"
#include "math.h"
	
void c40_printf( char *fmt, ... )
{
	va_list ap;
	char *p, *sval, cval;
	long ival;
	float fval;
	char buf[16];

	va_start( ap, fmt );
	for( p=fmt ; *p ; p++ )
	{
		switch( *p )
		{
			case '\n' :
				c40_putchar( (int) 10 );
				c40_putchar( (int) 13 );
				break;
			case '%' :
				switch( *++p )
				{
					case 'd' :
						ival = va_arg( ap, int );
						ltoa( ival, buf, 10 );
						putstr( buf );
						break;
					case 'f' :
						fval = va_arg( ap, float );
						ftoa( fval, buf );
						putstr( buf );
						break;
					case 'x' :
						ival = va_arg( ap, int );
						xtoa( ival, buf );
						putstr( buf );
						break;
					case 'c' :
						cval = va_arg( ap, char );
						c40_putchar( cval );
						break;
					case 's' :
						sval = va_arg( ap, char * );
						putstr( sval );
						break;
					default :
						c40_putchar( *p );
						break;
				}
				break;
			default:
				c40_putchar( *p );
				break;
		}
	}
	va_end( ap );
}



void putstr( char *buf )
{
	int i;

	for( i=0 ; buf[i] != '\0' ; i++ )
		c40_putchar( buf[i] );
}




void xtoa( unsigned long hexval, char *buf )
{
	unsigned long mask=0x0F0000000, i;
	unsigned long temp;

	for( i=0 ; i < 8 ; i++, mask >>= 4 )
	{
		temp = hexval & mask;
		temp >>= (7-i)*4;
		buf[i] = (temp < 10) ? 48+temp : 55+temp;
	}
	buf[8] = '\0';
}





void ftoa( float fval, char *buf )
{
	int index=0, count, exponent;
	double temp1, temp2;

	if( fval < 0 )
	{
		buf[index++] = '-';
		fval = -fval;
	}

	exponent = fval!=0.0?log10( fval ):0;

	fval /= pow( (double)10.0, (double) exponent );


	if( (fval > -1.0) && (fval < 1.0) && (fval != 0.0) )
	{
		fval *= 10;
		exponent--;
      buf[index++] = '0' + (int)fval;
		fval -= (int)fval;
		buf[index++] = '.';
	}
	else
	{
		buf[index++] = '0' + (int)fval;
		fval -= (int)fval;
		buf[index++] = '.';
	}
	for( count=0 ; count < 4 ; count++ )
	{
		fval *= 10;
		buf[index++] = '0' + (int)fval;
      fval -= (int)fval;
	}

	if( exponent )
	{
		buf[index++] = 'x';
		buf[index++] = '1';
		buf[index++] = '0';
		buf[index++] = 'e';
		ltoa( exponent, buf+index );
	}
	else
	{
		buf[index] = '\0';
	}
}



void send_host( unsigned long data )
{
}