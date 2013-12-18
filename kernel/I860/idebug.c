#include <stdarg.h>
#include <helios.h>
#include "i860.h"
#include "idebug.h"
#include "exec.h"

void adapteraddress(char ch)
{
	XIOdebug("adapter address ostat = %X\n",&(ADAPTER->LA_OStat));
}

void obyte(char ch)
{
	while( !(ADAPTER->LA_OStat & o_rdy) ) ;
	ADAPTER->LA_OData = ch;
}

char ibyte(void)
{
	while( !(ADAPTER->LA_IStat & i_rdy) ) ;
	return ADAPTER->LA_IData;
}

int outstring(char *s)
{	char ch;
	int i = 0;
	while( (ch = *s++) != 0 ) { obyte(ch); i++; }
	return i;
}

void oword(int w)
{
	obyte(w);
	obyte(w>>8);
	obyte(w>>16);
	obyte(w>>24);
}

int iword(void)
{	int r;
	r = ibyte();
	r += ibyte()<<8;
	r += ibyte()<<16;
	r += ibyte()<<24;
	return r;
}


static int ppnum(int n, int base)
{	int nchars;
#if 1
	char *chars = "0123456789abcdef";
#else
	char chars[16];
	chars[0]='0';chars[1]='1';chars[2]='2';chars[3]='3';chars[4]='4';
	chars[5]='5';chars[6]='6';chars[7]='7';chars[8]='8';chars[9]='9';
	chars[10]='a';chars[11]='b';chars[12]='c';chars[13]='d';chars[14]='e';
	chars[15]='f';
#endif
	if( n == 0 ) return 0;
	nchars = ppnum(n/base,base);
	obyte(chars[n%base]);
	return nchars+1;
}

int printnum(int n,int base)
{	int nchars = 0;
	if( n == 0 )
	{	obyte('0');
		return 1;
	}
	if( n < 0 ) { obyte('-'); n = -n; nchars = 1; }
	return nchars+ppnum(n,base);
}

static int ppunum(unsigned int n, int base)
{	unsigned int m;
	int nchars;
#if 1
	char *chars = "0123456789abcdef";
#else
	char chars[16];
	chars[0]='0';chars[1]='1';chars[2]='2';chars[3]='3';chars[4]='4';
	chars[5]='5';chars[6]='6';chars[7]='7';chars[8]='8';chars[9]='9';
	chars[10]='a';chars[11]='b';chars[12]='c';chars[13]='d';chars[14]='e';
	chars[15]='f';
#endif
	{	int z;

	if( n == 0 ) return 0;
#if 0
	obyte('X');
	obyte(1); oword(0x12345678); obyte(0);
	z = __multiply(3,5);
	obyte(1); oword(z); obyte(0);
	obyte(1); oword(0x88664422); obyte(0);
	z = __divide(32,16);
	oword(z);
	obyte(1); oword(0x11335577); obyte(0);
#endif
	m = n/base;
	}
	nchars = ppunum(m,base);
	obyte(chars[n%base]);
	return nchars+1;
}

int printunum(unsigned int n,int base)
{
	if( n == 0 )
	{	obyte('0');
		return 1;
	}
	return ppunum(n,base);
}

static char *printobj(char *s, va_list ap,int *nchars)
{	char ch = *s++;
	int nch = 0;
	switch( ch )
	{
	case 'd': nch = printnum(va_arg(ap,int),10); break;
	case 'D': nch = printunum(va_arg(ap,int),10); break;
	case 'x': nch = printnum(va_arg(ap,int),16); break;
	case 'X': nch = printunum(va_arg(ap,int),16); break;
	case 's': case 'S':
		nch = outstring(va_arg(ap,char *)); break;
	default:
		obyte(ch); nch = 1;
		break;
	}

	*nchars = *nchars + nch; 
	
	return s;
}

int XIOdebug(char *fmt, ...)
{	char ch;
	va_list ap;
	int nchars = 0;

	va_start(ap,fmt);
	while( (ch = *fmt++) != '\0')
	{
		if( ch == '%' ) fmt = printobj(fmt,ap,&nchars);
		else
		{
			obyte(ch);
			nchars ++;
		}
	}
	return nchars;
}

void peekpoke()
{
	while( 1 )
	{	int *addr;
		char ch = ibyte();
		switch( ch )
		{
		case 0:
			addr = (int *)iword();
			if( addr == 0 )
			{	iword();
				return;
			}
			else
				*addr = iword();
			break;
		case 1:
			addr = (int *)iword();
			oword(*addr);
			break;
		default:
			break;
		}
	}
}

/* extern struct TrapData TrapData; */

static void pad(int n, int m)
{
	for(; m<n; m++) obyte(' ');
}

void dumpregs(struct TrapData *td)
{	int i;
	XIOdebug("Registers dump:\n"); 
	XIOdebug("Trapdata is at %X\n",(int)td);
	for( i = 0; i< 32; i+=4)
	{	int j;
		for( j = i; j < i+4; j++)
		{	char * f;
			int fwidth;
			f = j< 10? "r%d  = %X":"r%d = %X";
			fwidth = XIOdebug(f,j,td->intregs[j]);
			pad(17,fwidth);
		}
 		XIOdebug("\n");
	}
	XIOdebug("psr = %X\n",td->PSR);
	XIOdebug("db = %X\n",td->DB);
	XIOdebug("dirbase = %X\n",td->DIRBASE);
	XIOdebug("fir = %X\n",td->FIR);
	XIOdebug("fsr = %X\n",td->FSR);
}

void dumpstate(struct TrapData *td)
{

	XIOdebug("psr = %X  ",td->PSR);
	XIOdebug("dirbase = %X  ",td->DIRBASE);
	XIOdebug("fir = %X  ",td->FIR);
	XIOdebug("fsr = %X  ",td->FSR);
	XIOdebug("\n");
}

