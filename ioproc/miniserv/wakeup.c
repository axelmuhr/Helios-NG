#include <stdio.h>
#include <dos.h>

int main(void)
{ union REGS regs;

  int86(0x61, &regs, &regs);
  printf("Wakeup exiting.\n");
}

