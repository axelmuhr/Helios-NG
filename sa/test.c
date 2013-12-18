

#include <time.h>
#include <setjmp.h>

jmp_buf j;

void sjt(int n, int delay)
{
	IOdebug("clock %d",clock());
	Delay(delay);
	if( n == 0 ) longjmp(j,1);
	sjt(n-1,delay);
	IOdebug("should not happen");
}

int main()
{
	if( setjmp(j) == 0 )
	{
		sjt(10,1000);
	}
	IOdebug("test done");
}
