#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <string.h>

#define eq ==
#define ne !=

int main(int argc, char **argv)
{ FILE *data, *out;
  char val, newval;
  int  i, len;
  
  if (argc != 3)
   { fprintf(stderr,"Usage : encode <source> <dest>\n"); exit(1); }


  data = fopen(argv[1], "rb");
  if (data == Null(FILE))
   { fprintf(stderr, "Cannot open %s\n", argv[1]); exit(1); }

  out = fopen(argv[2], "wb");
  if (out == Null(FILE))
   { fprintf(stderr, "Cannot open %s\n", argv[2]); exit(1); }
  
  i = (512 << 16) + 512; fwrite((void *) &i, sizeof(int), 1, out);
   
  val = getc(data); len = 1;
  for (i = 1; i < 512 * 512; i++)
   { newval = getc(data);
     if (val == newval)
      len++;
     else
      { if ((len <= 4 ) && (val != 0x00FF))
         { for ( ; len > 0; len--)
            fputc(val, out);
         }
        else
         { fputc(0x00FF, out);
           fputc(len / 256, out);
           fputc(len & 0x00FF, out);
           fputc(val, out);
         }
        val = newval; len = 1;
      }
   }
   
  fclose(data); fclose(out);
}
