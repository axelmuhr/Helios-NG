#include <stdio.h>
#include <dos.h>

extern int set_interrupts(void);
extern void tsr(void);

int main(void)
{ if (!set_interrupts())
   { printf("Apparently already installed.\n");
     return(1);
   }

  tsr();
  return(0);
}

