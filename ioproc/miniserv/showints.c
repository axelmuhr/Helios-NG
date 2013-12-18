#include <stdio.h>

extern int  test_int(void);
extern void *get_int(int *);

int main(void)
{ int *ptr;
  int array[7];
 
  if (test_int())
   { puts("The Miniserver has not been installed.\n");
     return(1);
   }

  get_int(array);

  printf("Total number of link interrupts : %d\n", array[0]);
  printf("Clashes with MS-DOS calls : %d\n", array[1]);
  printf("I/O handled during timer interrupt : %d\n", array[2]);
  printf("Timer interrupts clashing with MS-DOS : %d\n", array[3]);
  printf("Idle interrupts with outstanding link I/O : %d\n", array[4]);
  printf("Magic number check : %x\n", array[5]);
  printf("InDOS flag : %x\n", array[6]);

  printf("\n%d interrupts handled immediately, %d during timer interrupt\n",
         array[0] - array[1], array[2]);
  printf("%d interrupts handled during idle time.\n", array[4]);
  printf("Total handled is %d out of %d\n", (array[0] - array[1]) +
         array[2] + array[4], array[0]);

  return(0);
}
