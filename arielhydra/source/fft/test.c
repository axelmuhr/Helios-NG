#include "pythag.h"
#include <math.h>
#include <stdlib.h>

#define FREQ		8.0

#define TWO_PI		6.283185307

#define ELAPSED_TIME( start, end )	(((end) - (start))*0.0000001)

#define GET_TIMER	(*(unsigned long *)0x00100024)
#define RESET_TIMER (*(unsigned long*)0x00100020 |= 960)

#define SET_PERIOD(X)	(*(unsigned long *)0x00100028 = (unsigned long) X)


float *in_addr = (float *)0x2ff800;
complex *out_addr = (complex *)0x2ffc00;
int start_flag=0;
int FFTSize;
float elapsed_time=0;

main()
{
	complex	*out;
	float	*in;
	int i;
	unsigned long timerStart,timerEnd;


	GIEOn();
	init();

	SET_PERIOD(0xFFFFFFFF);

	while( 1 )
	{
		while( !start_flag );

		start_flag = 0;

		in = in_addr;
		out = out_addr;

		RESET_TIMER;
		timerStart = GET_TIMER;

		rfft(in,out,FFTSize,1);

		timerEnd=GET_TIMER;

		elapsed_time = ELAPSED_TIME(timerStart,timerEnd);
	}

}
