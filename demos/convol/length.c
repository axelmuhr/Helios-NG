#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <string.h>

#define eq ==
#define ne !=

int results[1024];

int main(int argc, char **argv)
{ FILE *data;
  char val, newval;
  int  i, len;
  
  if (argc != 2)
   { fprintf(stderr,"Usage : hist <filename>\n"); exit(1); }


  data = fopen(argv[1], "rb");
  if (data == Null(FILE))
   { fprintf(stderr, "Cannot open %s\n", argv[1]); exit(1); }

  memset(results, 0, 1024 * 4);
  
  val = getc(data); len = 1;
  for (i = 1; i < 512 * 512; i++)
   { newval = getc(data);
     if (val == newval)
      len++;
     else
      { if (len >= 1024)
         fprintf(stderr, "Char %x, length %d\n", val, len);
        else
         results[len]++;
        len = 1;
        val = newval;
      }
   }
   
  for (i = 0; i < 1024; i++)
   if (results[i] != 0)
     printf("Length %d : %d occurrences\n", i, results[i]);

}
