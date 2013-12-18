#include <helios.h>
#include <stdio.h>
#include <process.h>
#include <syslib.h>

Semaphore out;


void TimeIt(int time)
{
	forever {

	Delay(OneSec * time);

	Wait(&out);
	printf("%d Secs delay wakeup\n",time);
	Signal(&out);
	}
}

int
main(int argc, char **argv)
{
puts("Starting sleep test...");

InitSemaphore(&out,1);

Fork(1000, TimeIt, 4, 2);
Fork(1000, TimeIt, 4, 4);
Fork(1000, TimeIt, 4, 8);
Fork(1000, TimeIt, 4, 16);

TimeIt(32);

return 0;
}
