#include <syslib.h>
#include <nonansi.h>
#include <stdio.h>

int main()
{
	Port a;
	MCB m;
	word timeout = OneSec*5;
	word start, end;
	word e;

	a = NewPort();

	forever
	{
		InitMCB(&m,MsgHdr_Flags_preserve,a,NullPort,1); /* IOCTimeout */
		m.Timeout = timeout;

		start = _cputime(); /* times in centiseconds */
		e = PutMsg(&m);
		end = _cputime();

		if ((end - start) > ((timeout/10000) + 500) || (end - start) < 0)
		{
			printf("PutMsg TIMEOUT ERROR returned %lx - ", e);
			IOdebug("PutMsg TIMEOUT ERROR 500 != %x centisecs\n",end-start);
		}
		printf("Timeout in %ld centisecs\n",end-start);
	}
}
