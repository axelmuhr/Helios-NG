#include <syslib.h>
#include <stdio.h>
#include <process.h>

int main()
{
printf("Processor implementation has %ld physical priority levels.\n",GetPhysPriRange()+1);
printf("This program's priority is %ld (logical) / %ld (physical).\n",GetPriority(),LogToPhysPri(GetPriority()));
printf("\nLogical priorities range from %d (HighestPri), through %d (StandardPri),\n",HighestPri,StandardPri);
printf("to %d (IdlePri).\n",IdlePri);
printf("\nThe logical (processor/version implementation independent) levels are\n");
printf("mapped at runtime into the actual physical priority levels available.\n");
}
