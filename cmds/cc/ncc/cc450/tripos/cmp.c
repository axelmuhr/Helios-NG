#include <stdio.h>

main(argc, argv)
int argc;
char *argv[];
{
   FILE *f1, *f2;
   int c1,c2;
   int n = 0;

   if (argc != 3)
   {
      printf("Wrong no. of args\n");
      exit(10);
   }

   f1 = fopen(argv[1],"r");
   f2 = fopen(argv[2],"r");

   if( f1 == NULL | f2 == NULL)
   {
      printf("One of the files wont open\n");
      exit(10);
   }

   for(;;)
   {
      c1 = getc(f1);
      c2 = getc(f2);

      if(c1 != c2)
      {
         printf("Difference at $%x\n",n);
         break;
      }
      if(c1 == EOF) break;
      n++;
   }

   if( c1 == c2 && c2 == EOF )
      printf("No differences\n");
   fclose(f1);
   fclose(f2);
}
