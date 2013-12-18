/*
** trace.c
**		This program can be used to examine the contents of
**	the trace vector while Helios is still up and running.
**
** Author: Bart Veer, 3.12.92
*/

#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <root.h>
#include <syslib.h>
#include <gsp.h>

int main(void)
{
  int *		trace_vec	= (int *) GetRoot()->TraceVec;
  int *		trace_end	= &(trace_vec[ (trace_vec[0] & 0x0FFF) / sizeof(int) ]);
  char		buf[ IOCDataMax ];

  
  MachineName( buf );
  
  printf( "Trace vector on processor %s\n", buf );

  for (++trace_vec; trace_vec < trace_end; trace_vec++)
    {
      switch(*trace_vec)
	{
	case 0x11111111 :	
#ifdef __TRAN
	  printf("Regs: T= %08x W= %08x I= %08x\n",
		 trace_vec[1], trace_vec[2], trace_vec[3]);
	  printf("      A= %08x B= %08x C= %08x\n",
		 trace_vec[4], trace_vec[5], trace_vec[6]);
	  trace_vec += 6;
#else
	  printf("Regs: T= %08x SP= %08x Caller= %08x Modtab %08x\n",
		 trace_vec[1], trace_vec[2], trace_vec[3], trace_vec[4]);
	  printf("      A= %08x B= %08x C= %08x\n",
		 trace_vec[5], trace_vec[6], trace_vec[7]);
	  trace_vec += 7;
#endif
	  break;
	  
	case 0x22222222 :
#ifdef __TRAN
	  printf("Mark: T= %08x W= %08x I= %08x\n",
		 trace_vec[1], trace_vec[2], trace_vec[3]);
	  trace_vec += 3;
#else
	  printf("Regs: T= %08x SP= %08x Caller= %08x Modtab %08x\n",
		 trace_vec[1], trace_vec[2], trace_vec[3], trace_vec[4]);
	  trace_vec += 4;
#endif
	  break;

	default:
	  printf("????: %08x\n", *trace_vec);
	}
    }

  return 0;
  
} /* main */
