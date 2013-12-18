#include <stdio.h>
#include <root.h>
#include <syslib.h>

int donowt(int a)
{
	int b;
		
	b = a * 42;
	return b;
}
int
main(int argc, char **argv)
{
	RootStruct *root = GetRoot();
	word maxav = 0;

	while (TRUE)
	{
		word av = root->LoadAverage;

		if (av > maxav) maxav = av;

	if (argc == 1)
	{
		printf("LowPriAv:  %8lu, HiPriLat:    %8lu, MaxLat: %8lu\n",
			av ,root->Latency,root->MaxLatency);
		printf("MaxAv: %lu\n", maxav);
		printf("LocalMsgs: %8lu, BuffMsgs:    %8lu, Time:  %8lu\n",
			root->LocalMsgs,root->BufferedMsgs,root->Time);
		printf("Timer:     %#8lx, %ld\n\n",
		       root->Timer, root->Timer);
	}
	else if (argc == 2)
	{
		IOdebug("LowPriAv:  %x, HiPriLat:    %x, MaxLat: %x",
			av ,root->Latency,root->MaxLatency);
		IOdebug("MaxAv: %x", maxav);
		IOdebug("LocalMsgs: %x, BuffMsgs:    %x, Time:  %x",
			root->LocalMsgs,root->BufferedMsgs,root->Time);
		IOdebug("Timer:  %x\n", root->Timer);
	}
	else
	{
		donowt(3);
	}
		Delay(OneSec);
	}
}
