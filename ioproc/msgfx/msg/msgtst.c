#include <stdio.h>
#include <graph.h>
#include <dos.h>

void interrupt far inrtn()
{
	_setvideomode(_MRES4COLOR);
	_ellipse(_GBORDER,10,10,50,50);
}

main()
{	union REGS in,out;
   	_dos_setvect(0x60, inrtn);
	system("dotrap");
}
 
		
