#include <stdio.h>
#include <dos.h>
#include <graph.h>

extern void set_interrupt(void);

void INTERRUPT_RTN(void far *pointer)
{
	_setvideomode(_MRES4COLOR);
	_ellipse(_GFILLINTERIOR,10,10,50,50);	
}
 
int main()
{
  SET_INTERRUPT();

  system("inttest");
  printf("All done\n");
}
   
