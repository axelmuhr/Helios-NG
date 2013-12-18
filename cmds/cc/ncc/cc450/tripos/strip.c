#include <stdio.h>
#include <ctype.h>

int printable(ch)
char ch;
{
   if( ch > 0x7f ) return 0;
   return( isprint(ch) || (ch == '\n') || (ch == '\t')  );
}

main(argc,argv)
int argc;
char *argv[];
{
   FILE *infile;
   FILE *outfile;
   int ch;

   infile = fopen(argv[1],"r");
   if(infile == NULL)
   {
      fprintf(stderr,"Unable to open input\n");
      exit(1);
   }

   outfile = fopen(argv[2],"w");
   if(outfile == NULL)
   {
      fprintf(stderr,"Unable to open output\n");
      exit(1);
   }

   while(( ch = getc(infile)) != EOF)
   {
      if( !printable(ch) ) continue;
      putc(ch, outfile);
   }
   fclose(infile);
   fclose(outfile);
}

