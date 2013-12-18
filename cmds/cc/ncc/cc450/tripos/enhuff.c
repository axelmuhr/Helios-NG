/*                                                                      */
/*             This is a Huffman encoding routine                       */
/*                                                                      */
/* Written by: C.G.Selwyn                                               */
/* Address:  School of Elec. Eng.                                       */
/*             University of Bath                                       */
/*               Bath, Avon                                             */
/* Date:  22-MAR-88                                                     */
/*                                                                      */
/* It will encode ANY file (not just text files)                        */
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

typedef long CHARCODE;

typedef struct NODE {
   struct NODE *left;
   struct NODE *right;
   long          count;
} NODE;

typedef struct CHARNODE {
   NODE         chnode;
   char         ch;
   CHARCODE     code;
   int          codelength;
} CHARNODE;

CHARNODE *chars[256];
NODE     *sortvec[256];
FILE     *codedfile;
FILE     *sourcefile;

int compare_function( const void *arg1, const void *arg2)
{
   NODE **a1 = (NODE **)arg1;
   NODE **a2 = (NODE **)arg2;
   long diff = (*a2)->count - (*a1)->count;

   return (diff > 0)?1: (diff?-1: 0);
}

#ifdef never
void memexg(char *s1, char *s2, size_t size)
{
   size_t i;
   for( i = 0; i < size; i++)
   {
      char t = *s1;
      *s1++ = *s2;
      *s2++ = t;
   }
}
#endif

/* This is a sort routine with exactly the same spec. as qsort in    */
/* the ANSI runtime library, but not everybody has one of those.     */
/* The calls to ssort in the code below can be replaced with calls   */
/* to qsort.                                                         */
/* This is in fact a shell sort routine as described in              */
/*  Sedgewick P.97                                                   */

void ssort( void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
   int h = 1;
   char *bb = (char *)base;
   char *v = (char *)malloc(size);

   while( h <= nmemb )
      h = 3*h+1;

   if( h == 0 ) return;

   do
   {  int hh,i;
      h = h/3;
      hh = h*size;

      for( i = hh; i < nmemb*size; i += size )
      {  int jj = i;

         jj = i;
         memcpy(v,&bb[jj],size);
         while( compar(&bb[jj-hh], v) > 0 )
         {
            memcpy(&bb[jj], &bb[jj-hh], size);
            jj = jj-hh;
            if( jj < hh ) break;
         }
         memcpy(&bb[jj],v,size);
      }
   } while( h != 1 );
   free((void *)v);
}

static int       nbits;
static char      outchar;
void flushbits(void)
{
   if( (nbits&7) != 0 )
      putc(outchar, codedfile);
}

#define nbitmask(n) ((1L << (n))-1)

void sendbits( CHARCODE v, int n )
{
   int i;
   int j;

   for(;n > 0; n -= j)
   {
      i = 8 - (nbits & 7);      /* No. of bits can fit in this byte */

      if( n >= i )
         (outchar |= (char)((v >> (n-i)) & nbitmask(i))), j = i;
      else
         (outchar |= (char)((v & nbitmask(n)) << (i-n))), j = n;

      nbits += j;
      if( (nbits & 7) == 0)
      {  putc(outchar, codedfile);
         outchar = 0;
      }
   }
}

void assigncodes(NODE *tree, CHARCODE code, int depth)
{
   if( tree->left == NULL )
   {
      CHARNODE *t   = (CHARNODE *)tree;
      t->code       = code;
      t->codelength = depth;
      return;
   }

   assigncodes(tree->left, code << 1, depth+1);
   assigncodes(tree->right,(code << 1)+1, depth+1);
}

void dumpcodes(void)
{
   int i;
   CHARCODE w;

   for( i = 0; i <= 255; i++)
   {
      w = chars[i]->code + (1L << chars[i]->codelength);
      fwrite(&w, sizeof(w), 1, codedfile);
   }
}

void enhuff(void)
{
   int i,ch;
   NODE *newnode;
   long      bitlengthpos;

   for(i = 0; i <= 255; i++)
   {
      chars[i]     = (CHARNODE *)calloc(1,sizeof(CHARNODE));
      chars[i]->ch = i;
   }

   while( (ch = getc(sourcefile)) != EOF )
      chars[ch]->chnode.count++;

   memcpy(sortvec,chars,sizeof(chars));

   ssort((void *)sortvec, 256, sizeof(NODE *), compare_function);

   for( i = 255; i >= 0; i--)
      if( sortvec[i]->count != 0 ) break;

   for( ; i > 0 ; --i)
   {
      newnode = (NODE *)malloc( sizeof(NODE));

      newnode->left      = sortvec[i-1];
      newnode->right     = sortvec[i];
      newnode->count     = sortvec[i-1]->count + sortvec[i]->count;
      sortvec[i-1]       = newnode;
      ssort(sortvec, i, sizeof(NODE *), compare_function);
   }

   assigncodes(sortvec[0],0,(sortvec[0]->right == 0)? 1:0);

   dumpcodes();

   rewind(sourcefile);

   {  long zero = 0;
      bitlengthpos = ftell(codedfile);
      fwrite(&zero, sizeof(zero), 1, codedfile);
   }

   while( (ch = getc(sourcefile)) != EOF )
      sendbits( chars[ch]->code, chars[ch]->codelength);

   flushbits();

   fseek(codedfile, bitlengthpos, SEEK_SET);

   fwrite(&nbits,sizeof(nbits), 1, codedfile);
}

int main(int argc,char *argv[])
{  char *codedname;

   if(argc != 2)
   {
      printf("Wrong no. of args\n");
      exit(1);
   }

   sourcefile = fopen(argv[1],"r");
   if( sourcefile == NULL )
   {
      printf("Unable to open %s\n",argv[1]);
      exit(1);
   }

   codedname = (char *)malloc(strlen(argv[1])+3);
   strcpy(codedname, argv[1]);
   strcat(codedname, ".z");
   codedfile = fopen(codedname, "w");
   if( codedfile == NULL )
   {
      printf("Unable to open %s\n",codedname);
      exit(1);
   }

   enhuff();

   fclose(sourcefile);

   {  int r;

      if( !ferror(codedfile) )
      {  remove(argv[1]);
         r = 0;
      }
      else
      {
         fprintf(stderr,"There has been a fault on the coded file\n"
                     "%s not deleted\n",argv[1]);
         r = 1;
      }
      fclose(codedfile);
      return r;
   }
}
