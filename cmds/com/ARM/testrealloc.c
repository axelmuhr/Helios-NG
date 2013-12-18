#define MAX 30

#include <stdio.h>
#include <memory.h>
#include <syslib.h>

int main()
{
	void *mp[MAX+1];
	int i,t=0;
	Pool *testpool = &MyTask->MemPool;
	
	puts("Testing ReAllocMem() fn.\n");

	printf("AllocMem(1000000); [1000]\n");
	if ((mp[0] = AllocMem(1000000, testpool)) == NULL) {
		printf("Failed to allocate 1000000 bytes\n");
		return(1);
	}
	t++;

	printf("LowAllocMem(1000000); [1000]\n");
	if ((mp[1] = LowAllocMem(1000000, testpool)) == NULL) {
		printf("Failed to allocate 1000000 bytes\n");
		return(1);
	}
	t++;

	puts("\nHit return to test HIGH reallocs...\n");
	getchar();

	printf("ReAllocMem(10000);\n");
	if (!ReAllocMem(10000, mp[0])) {
		printf("Failed to reallocate 10000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("ReAllocMem(100000);\n");
	if (!ReAllocMem(100000, mp[0])) {
		printf("Failed to reallocate 100000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("ReAllocMem(1000);\n");
	if (!ReAllocMem(1000, mp[0])) {
		printf("Failed to reallocate 1000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("ReAllocMem(1000000);\n");
	if (!ReAllocMem(1000000, mp[0])) {
		printf("Failed to reallocate 1000000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("ReAllocMem(6000000);\n");
	if (!ReAllocMem(6000000, mp[0])) {
		printf("Failed to reallocate 6000000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("ReAllocMem(10000000);\n");
	if (!ReAllocMem(10000000, mp[0])) {
		printf("Failed to reallocate 10000000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("ReAllocMem(1000);\n");
	if (!ReAllocMem(1000, mp[0])) {
		printf("Failed to reallocate 1000 bytes\n");
	}

	puts("\nHit return to test LOW Reallocs...\n");
	getchar();

	printf("LOW ReAllocMem(100000);\n");
	if (!ReAllocMem(100000, mp[1])) {
		printf("Failed to reallocate 100000 bytes\n");
	}


	puts("\nHit return...\n");
	getchar();

	printf("LOW ReAllocMem(1000);\n");
	if (!ReAllocMem(1000, mp[1])) {
		printf("Failed to reallocate 1000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("LOW ReAllocMem(1000000);\n");
	if (!ReAllocMem(1000000, mp[1])) {
		printf("Failed to reallocate 1000000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("LOW ReAllocMem(6000000);\n");
	if (!ReAllocMem(6000000, mp[1])) {
		printf("Failed to reallocate 6000000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("LOW ReAllocMem(10000000);\n");
	if (!ReAllocMem(10000000, mp[1])) {
		printf("Failed to reallocate 10000000 bytes\n");
	}

	puts("\nHit return...\n");
	getchar();

	printf("LOW ReAllocMem(1000);\n");
	if (!ReAllocMem(1000, mp[1])) {
		printf("Failed to reallocate 1000 bytes\n");
	}

	puts("\nHit return to free mem...\n");
	getchar();


	puts("\nFreeing memory...\n");
	for(i=0; i < t; i++)
	{
		if(mp[i] != NULL) {
			printf("FreeMem();\n");
			FreeMem(mp[i]);
		}
	}

	return 0;
}
