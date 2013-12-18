#include <stdio.h>
#include <stdlib.h>

#define OBJCODE 0x01
#define OBJBSS  0x02
#define OBJINIT 0x03
#define OBJBYTE 0x09
#define OBJSHORT 0x0a
#define OBJWORD  0x0c
#define OBJMODULE 0x20
#define OBJGLOBAL 0x22
#define OBJLABEL  0x23
#define OBJDATA   0x24
#define OBJCOMMON 0x25
#define OBJLABELOFF 0x0f
#define OBJDATASYMB 0x10
#define OBJDATAMODULE 0x11
#define OBJMODNUM     0x12
#define PATCHMIN      0x13
#define PATCHMAX      0x1f

#define MORE 0x80
#define NFLAG 0x40

FILE *f;
int pc = 0;

void tidyup(int n)
{
   if(f)fclose(f);
   exit(n);
}

int readobjnum(void)
{
   int ch = getc(f);
   int nflag  = (ch & NFLAG) != 0;
   int r = ch & 0x3f;

   while( (ch & MORE) != 0 )
   {
      if(ch==EOF)
      {  printf("Unexpected EOF in readobjnum\n");
         tidyup(1);
      }
      ch = getc(f);
      r  = (r<<7) + (ch&0x7f);
   }
   return nflag? -r: r;
}

void show_string(void)
{
   char ch;
   while( (ch = getc(f)) != 0 )
   {  if(ch==EOF)
      {  printf("Unexpected EOF in show_string\n");
         tidyup(1);
      }
      putchar(ch);
}

void show_code(int n)
{
#define CODEMAX 20
   int m = n;
   int i;

   if( m > CODEMAX ) m = CODEMAX;

   printf("CODE %d :",n);

   for( i = 1; i <= m; i++)
   {
      int ch = getc(f);
      if(ch==EOF)
      {
         printf("Unexpected EOF in show_code\n");
         tidyup(1);
      }
      printf(" %02x",ch);

   putchar('\n');

   fseek(f,ftell(f)+n-m,SEEK_SET);
}

void show_patch(char *s)
{
   int ch = getc(f);

   printf("%s ",s);

   if( PATCHMIN <= ch && ch <= PATCHMAX )
   {  int n = readobjnum();
      printf("PATCH %d : %d ",ch-PATCHMIN+1,n);
      show_patch("");
   }
   else
   {
      switch( ch )
      {
      case OBJLABELOFF:
         printf("LABELOFFSET ");
         show_string();
         break;
      case OBJDATASYMB:
         printf("DATASYMB ");
         show_string();
         break;
      case OBJDATAMODULE:
         printf("DATAMODULE ");
         show_string();
         break;
      case OBJMODNUM:
         printf("MODNUM");
         break;
      default:
         printf("Bad object format\n");
         tidyup(1);
      }
   }
}

int main(int argc, char *argv[])
{

   if( argc != 2 )
   {
      printf("Incorrect no. of args\n");
      tidyup(1);
   }

   if( (f = fopen(argv[1],"r")) == 0)
   {
      printf("Unable to open input file\n");
      tidyup(1);
   }

   while(1)
   {
      int ch = getc(f);

      if( ch == EOF ) break;

      printf("PC = 0x%04x ",pc);

      switch( ch )
      {
      case OBJCODE:
         {  int n = readobjnum();
            show_code(n);
            pc += n;
            break;
         }

      case OBJBSS:
         {  int n = readobjnum();
            printf("BSS: %d\n",n);
            pc += n;
            break;
         }

      case OBJINIT:
            printf("INIT\n");
            pc += 4;
            break;

      case OBJBYTE:
         {  show_patch("BYTE");
            putchar('\n');
            pc += 1;
            break;
         }

      case OBJSHORT:
         {  show_patch("SHORT");
            putchar('\n');
            pc += 2;
            break;
         }

      case OBJWORD:
         {  show_patch("WORD");
            putchar('\n');
            pc += 4;
            break;
         }

      case OBJMODULE:
         {  int n = readobjnum();
            printf("MODULE %d\n",n);
            break;
         }

      case OBJGLOBAL:
         {  printf("GLOBAL ");
            show_string();
            putchar('\n');
            break;
         }

      case OBJLABEL:
         {  printf("LABEL ");
            show_string();
            putchar('\n');
            break;
         }

      case OBJDATA:
         {  int n = readobjnum();
            printf("DATA %d ",n);
            show_string();
            putchar('\n');
            break;
         }

      case OBJCOMMON:
         {  int n = readobjnum();
            printf("COMMON %d ",n);
            show_string();
            break;
         }
      default:
         printf("Bad object format\n");
         tidyup(1);
      }
   }
   tidyup(0);
}
