#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <string.h>

#define eq ==
#define ne !=

int results[256];

int main(int argc, char **argv)
{ FILE *data;
  char val;
  int  i, total;
  
  if (argc != 2)
   { fprintf(stderr,"Usage : hist <filename>\n"); exit(1); }


  data = fopen(argv[1], "rb");
  if (data == Null(FILE))
   { fprintf(stderr, "Cannot open %s\n", argv[1]); exit(1); }

  memset(results, 0, 1024);
  
  i = 0;
  for (val = getc(data); (val ne EOF) && (i++ < 512 * 512); val = getc(data))
   results[val]++;
   
  total = 0;
  for (i = 0; i < 256; i++)
   if (results[i] != 0)
    { printf("Byte %x : %d occurrences\n", i, results[i]);
      total += results[i];
    }
  printf("Total is %d\n", total);
}
