#include <root.h>
#include <memory.h>
#include <stdio.h>

#define LOOPS 3
#define TESTNUMBER 130
#define BLOCKSIZE 1024*8
#define TRIMAMOUNT 1024*4

bool waiting = FALSE;

void Pause()
{
	if (waiting)
		puts("Hit return to continue...");getchar();
}

int main(int argc, char **argv)
{
	int i, loops;
	word handles[TESTNUMBER];	
	RootStruct *root = GetRoot();
	MIInfo *mi = root->MISysMem;

	if(argc > 1 && !strcmp(argv[1],"-p"))
		waiting = TRUE;

	printf("Testing Relocatable memory manager\nBlocksize = %d\n\n",BLOCKSIZE);

	for (loops = LOOPS; loops > 0; loops--)
	{
		printf("Test loops to go: %d\n",loops);
		
		printf("\nAllocating blocks\n");
		Pause();
		for ( i = 0; i < TESTNUMBER; i++)
		{
			handles[i] = MIAlloc(mi, BLOCKSIZE);
			printf("Alloc'ed handle = %d, size %d\n", handles[i], BLOCKSIZE);
#if 0
			/*DBG*/printf("freehandles = %d\n",root->MIFreeSlots);
			/*DBG*/printf("MITable = %8x\n",root->MITable);
			/*DBG*/printf("MITableSize = %d\n",root->MITableSize);
#endif
		}

		printf("\nLocking blocks\n");
		Pause();
		for ( i = 0; i < TESTNUMBER; i++)
		{
			printf("Locking handle %d, addr = %8x\n",handles[i],MILock(mi,handles[i]));
		}

		printf("\nUnLocking blocks\n");
		Pause();
		for ( i = 0; i < TESTNUMBER; i++)
		{
			printf("UnLocking handle %d\n",handles[i]);
			MIUnLock(mi,handles[i]);
		}

		printf("\nTrimMIng blocks\n");
		Pause();
		for ( i = 0; i < TESTNUMBER; i++)
		{
			printf("TrimMIng handle %d by %d\n",i,TRIMAMOUNT);
			MITrim(mi,handles[i],TRIMAMOUNT);
		}

		printf("\nFreeing blocks\n");
		Pause();
		for ( i = 0; i < TESTNUMBER; i++)
		{
			printf("Freeing handle %d\n",handles[i]);
			MIFree(mi,handles[i]);
#if 0
			/*DBG*/printf("freehandles = %d\n",root->MIFreeSlots);
#endif
		}
	}
}
