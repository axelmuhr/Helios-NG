#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>

int main(argc, argv)
int argc;
char **argv;
{ 
  int *x = malloc(600);
  int *y = malloc(600);
  int i;
  fprintf(stderr,"x = %p, header = %x\n", x, x[-1]);
  fprintf(stderr,"y = %p, header = %x\n", y, y[-1]);
  for(i = 0; i<20; i++) {
    free(x);
    fprintf(stderr,"x = %p, header = %x\n", x, x[-1]);
    x = malloc(600);
    y = malloc(600);
    fprintf(stderr,"x = %p, header = %x\n", x, x[-1]);
  }
}
