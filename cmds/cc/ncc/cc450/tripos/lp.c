#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   FILE *f, *lp;
   int c,i;

   lp = fopen("prt:","w");
   if( lp == NULL )
   {
      printf("Cannot open line printer");
      exit(20);
   }

   fprintf(lp,"\033#3");

   for( i = 1; i <= argc-1; i++)
   {
      f = fopen(argv[i],"r");
      if( f == NULL )
      {
         printf("Cannot find file \"%s\"\n",argv[i]);
         continue;
      }

      printf("Printing %s\n",argv[i]);

      while( ( c = getc(f)) != EOF )
         putc(c, lp);
      putc('\f',lp);
      fclose(f);
   }

   fclose(lp);
   return 0;
}
