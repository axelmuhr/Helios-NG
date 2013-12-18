#define MAX 30

#include <stdio.h>
#include <memory.h>
#include <syslib.h>

int main()
{
	void *mp[MAX+1];
	int a,i,t=0;
	Pool *testpool = &MyTask->MemPool;
	
	puts("Testing LowAllocMem() fn.\n");

	for(i=a=1; i <= MAX; i++, a+=a)
	{
		printf("LowAllocMem(%10d); [%10d] = [[%10d]\n",a,t, a+t);
		if ((mp[i] = LowAllocMem(a, testpool)) == NULL)
			printf("Failed to allocate %10d bytes\n",a);
		else
			t+=a;
	}

	puts("\nHit return...\n");
	(void) getchar();

	for(i=a=1; i <= MAX; i++, a+=a)
	{
		if(mp[i] != NULL) {
			printf("FreeMem(%d);\n",a);
			FreeMem(mp[i]);
		}
	}

	return 0;
}
