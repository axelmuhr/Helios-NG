#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include <string.h>

#define eq ==
#define ne !=

int main(int argc, char **argv)
{ FILE *data, *out;
  int val; char *buf, *cur;
  int i, len, size;
  
  if (argc != 3)
   { fprintf(stderr,"Usage : decode <source> <dest>\n"); exit(1); }


  data = fopen(argv[1], "rb");
  if (data == Null(FILE))
   { fprintf(stderr, "Cannot open %s\n", argv[1]); exit(1); }

  out = fopen(argv[2], "wb");
  if (out == Null(FILE))
   { fprintf(stderr, "Cannot open %s\n", argv[2]); exit(1); }
  
  fread((void *) &i, sizeof(int), 1, data);
  size = ((i >> 16) & 0x0FFFF) * (i & 0x0FFFF);
  
  if ((buf = malloc(size)) eq Null(char))
   { fprintf(stderr, "Cannot allocate %d bytes\n", size); exit(1); }
   

  cur = buf;
  for (val = fgetc(data); val != EOF; val = fgetc(data) )
   { if (val == 0x00FF)
      { len = 256 * fgetc(data);
        len += fgetc(data);
        val = fgetc(data);
        for (i = 0; i < len; i++)
         *cur++ = val;
      }
     else
      *cur++ = val;
   }
  fwrite(buf, 1, size, out);
  
  fclose(data); fclose(out);
}
