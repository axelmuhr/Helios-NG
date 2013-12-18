
#include <salib.h>
#include <sysinfo.h>
#include <chanio.h>
#include <string.h>
#include <stdarg.h>

void IOputc(byte c)
{
	link_out_byte(_SYSINFO->bootlink,c);
}

void IOputs(byte *s)
{
	link_out_data(_SYSINFO->bootlink,s,strlen(s));
}

static void writenum1(unsigned int i, int base )
{
	char *digits = "0123456789abcdef";
	if( i == 0 ) return;
	writenum1(i/base,base);
	IOputc(digits[i%base]);
}

static void writenum(int i, int base)
{
	if( i<0 && base==10 ) { i=-i; IOputc('-'); }
	if( i == 0 ) IOputc('0');
	else writenum1(i,base);
}

static void writestr(char *s)
{
	while( *s ) IOputc(*s++);
}

void IOdebug(const char *str, ... )
{
	int base, i, *p;
	char *s;
	va_list a;

	va_start(a,str);

	while( *str ) 
	{
		if( *str == '%' )
		{
			base = 10;
			switch( *(++str) )
			{
			default:
				IOputc(*str); 
				break;

			case 0:				/* "...%" = no \n */
				goto nonl;
								
			case 'c':			/* char		*/
				i = va_arg(a,int);
				IOputc(i);
				break;

			case 'x': base = 16;		/* hex 		*/
			case 'd':			/* decimal	*/
				i = va_arg(a,int);
				writenum(i,base);
				break;
				
			case 's':			/* string	*/
				s = va_arg(a,char *);
				if( s == NULL ) s = "<NULL string>";
			putstr:
				writestr(s);
				break;

			}
			str++;
		}
		else IOputc(*str++);
	}
	
	IOputc('\n');
nonl:
	va_end(a);
}

