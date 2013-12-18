/* test memory relocation via kernel compaction fn */

#include <stdio.h>
#include <root.h>
#include <memory.h>

int main(int argc, char **argv)
{
	word totalsize, totalfree, largestfree, percfree;

	printf("Original Memory Statistics:\n");
	StatMem(&totalsize, &totalfree, &largestfree, &percfree);
	printf("total size %d, total used %d,\ntotal free %d, largest free block %d, %% free %d\n", totalsize, totalsize-totalfree, totalfree, largestfree, percfree);

	printf("\nForcing memory compaction..."); fflush(stdout);
	MICompact(GetRoot()->MISysMem);
	printf(" Completed.\n");

	printf("\nNew Memory Statistics:\n");
	StatMem(&totalsize, &totalfree, &largestfree, &percfree);
	printf("total size %d, total used %d,\ntotal free %d, largest free block %d, %% free %d\n", totalsize, totalsize-totalfree, totalfree, largestfree, percfree);
}
