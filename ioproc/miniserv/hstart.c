#include <stdio.h>

extern int  test_int(void);
extern void call_int(void);

int main(void)
{ 
  if (test_int())
   { puts("The Miniserver has not been installed.\n");
     return(1);
   }

  call_int();
  return(0);
}
