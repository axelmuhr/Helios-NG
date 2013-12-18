#include <dos.h>

main()
{	union REGS in,out;
	int86(0x60,&in,&out);
}
