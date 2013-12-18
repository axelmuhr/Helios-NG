/*                                                                      */
/*             This is a Huffman decoding routine                       */
/*                                                                      */
/* Written by: C.G.Selwyn                                               */
/* Address:  School of Elec. Eng.                                       */
/*             University of Bath                                       */
/*               Bath, Avon                                             */
/* Date:  22-MAR-88                                                     */
/*                                                                      */
/* It will decode a file encoded by enhuff                              */
/* The format of the encoded file is                                    */
/*    256 * CODE                                                        */
/*    No. of stored bits                                                */
/*    Packed encoded bits                                               */
/*                                                                      */
/* The MSB of a CODE value is not part of the code for that character   */
/* but merely serves to mark the end of the bit string.                 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int CHARCODE;

typedef struct NODE {
   struct NODE *left;
   struct NODE *right;
} NODE;

typedef struct CHARNODE {
   NODE         chnode;
   char         ch;
} CHARNODE;

FILE *sourcefile;
FILE *decodedfile;
char  inchar;
int   nbits;
int   nreadbits = -1;

#define bit(x,n) (((x) >> n) & 1)

NODE *insert(NODE *t, CHARCODE v, int l, int ch)
{
   NODE **t1;

   if( t == NULL )
      t = (NODE *)calloc(1, l == 0? sizeof(CHARNODE): sizeof(NODE) );

   if( l == 0 )
   {  ((CHARNODE *)t)->ch = ch;
      return(t);
   }

   t1 = ( bit(v,l-1) == 0 )? &t->left: &t->right;

   *t1 = insert(*t1, v, l-1, ch);

   return( t );
}

unsigned int swapword(unsigned int w)
{
	unsigned int r;

	int i = 0x12345678;
	if (*((char *)(&i)) == 0x78)
	{
		r = (w&0xff)<<24;
		r += (w&0xff00)<<8;
		r += (w&0xff0000)>>8;
		r += (w&0xff000000)>>24;
		return r;
	}
	else return w;
}

NODE *readtree(void)
{
   int i;
   NODE *r=NULL;

   for( i = 0; i < 256; i++)
   {  CHARCODE w;
      int length;

      fread(&w, sizeof(w), 1, sourcefile);

      w = swapword(w);

      {  CHARCODE t = w;
         length = 31;
         while( (t&0x80000000) == 0 ) t <<= 1, length--;
      }

      if(length != 0) r = insert(r,w,length,i);
   }
   fprintf(stderr,"Read tree\n");
   return( r );
}

int getbit(void)
{
   int r;
   nreadbits++;

   if( (nreadbits & 7) == 0 )
      inchar = getc(sourcefile);

   r = (inchar & 0x80) != 0;
   inchar <<= 1;
   return(r);
}

void dehuff(void)
{
   NODE *tree = readtree();
   NODE *t=tree;

   fread(&nbits, sizeof(nbits), 1, sourcefile);
   nbits = swapword(nbits);
   fprintf(stderr,"nbits = %d\n",nbits);
   while( nbits-1 > nreadbits )
   {  t = (getbit() == 0)? t->left: t->right;

      if( t->left == NULL)
      {
         putc(((CHARNODE *)t)->ch, decodedfile);
         t = tree;
      }
   }
}

int main(int argc,char *argv[])
{
   char *sourcename;
   int len;

   
   if(argc != 2)
   {
   printf("usage: %s <name of file>\n", argv[0] );
	   
      exit(20);
   }

   sourcename = (char *)malloc((len = strlen(argv[1]))+3);
   strcpy(sourcename, argv[1]);
   if (sourcename[len - 1] != 'z' ||
       sourcename[len - 2] != '.')
   {
	   strcat(sourcename, ".z");
   }
   else
   {
	   argv[1][len - 2] = '\0';
   }
   
   sourcefile = fopen(sourcename,"r");
   if( sourcefile == NULL )
   {
      printf("Unable to open %s\n",sourcename);
      exit(20);
   }

   decodedfile = fopen(argv[1],"w");
   if( decodedfile == NULL )
   {
      printf("Unable to open %s\n",argv[1]);
      exit(20);
   }

   dehuff();

   {  int r;

      fclose(sourcefile);
      if( !ferror(decodedfile) )
      {  remove(sourcename);
         r = 0;
      }
      else
      {  fprintf(stderr,"There has been an error on %s\n"
                     "%s not deleted\n",argv[1], sourcename);
         r = 1;
      }
      fclose(decodedfile);
      return r;
   }
}
