#include <stdarg.h>
#include "iasm.h"

FILE *srecordfile = NULL;
FILE *objectfile  = NULL;

#define SRECMAX 0x20

uchar   srecord[SRECMAX];
uint32  srecaddr;
int     srecpos;
char    srecchk;

CodeSeg *code[NCODESEGS];

codeptr codep;       /* Index into code[] */
codeptr codestart;
uint32 bufstart;     /* PC at start of codebuf */

void outsrec(int type)
{  int i;
   char check = 0 ;
   char *hchars = "0123456789ABCDEF";

#define outhexbyte(ch) { fputc(hchars[((ch)>>4)&0xf],srecordfile); \
                         fputc(hchars[(ch)&0xf],srecordfile); \
                         check += ch; }

   fprintf(srecordfile,"S%1d",type);

   outhexbyte(srecpos+3);
   outhexbyte((char)(srecaddr>>8));
   outhexbyte((char)srecaddr);

   for( i=0; i < srecpos; i++ )
      outhexbyte(srecord[i]);
   outhexbyte(0xff-check);
   fputc('\n', srecordfile);
   srecaddr += srecpos;
   srecpos = 0;
}

bool flushsrec(uchar ch)
{
   outsrec(1);
   srecord[0] = ch;
   srecpos = 1;
   return 1;
}

bool setsrecfile(char *s)
{
   srecordfile = fopen(s,"w");
   if( srecordfile == NULL )
   {  parmerror("Unable to open S-record file\n");
      return 0;
   }
   return 1;
}

bool setobjectfile(char *s)
{
   objectfile = fopen(s,"wb");
   if( objectfile == NULL )
   {
      parmerror("Unable to open object file\n");
      return 0;
   }
   return 1;
}


#define putsrecbyte(ch) \
          ((srecpos == SRECMAX)? flushsrec(ch):(srecord[srecpos++] = (ch)) )
void output(int csize, ...)
{
   va_list ap;
   CodeSeg *cseg;

   if( pass == 1 )
   {  pcloc += csize;
      return;
   }

   va_start(ap,csize);

   if( (cseg = code[codep.seg]) == NULL)
      code[codep.seg] = cseg = aalloc(sizeof(CodeSeg));

   switch(csize)
   {
   case 1:
      {  char val = va_arg(ap,char);
         cseg->codebuf[codep.p].b1 = val;

         if( srecordfile )
            putsrecbyte((uchar)val);
      }
      break;
   case 2:
      {  short val = va_arg(ap,short);
         cseg->codebuf[codep.p].b2 = val;

         if( srecordfile )
         {
            putsrecbyte((uchar)val);val >>= 8;
            putsrecbyte((uchar)val);
         }
      }
      break;
   case 4:
      {  uint32 val = va_arg(ap,uint32);
         cseg->codebuf[codep.p].b4 = val;

         if( srecordfile )
         {
            putsrecbyte((uchar)val);val >>= 8;
            putsrecbyte((uchar)val);val >>= 8;
            putsrecbyte((uchar)val);val >>= 8;
            putsrecbyte((uchar)val);
         }
      }
      break;
   }

   cseg->codeflags[codep.p++] = csize;
   codep.p &= CODEBUFSIZEMASK;
   if(codep.p==0) codep.seg++;

   pcloc += csize;
   va_end(ap);
}

#define objbyte(ch) putc(ch,objectfile)
#define bitmask(n) ((1<<(n))-1)

static void objnum(int32 n)
{   int32 i;
    int32 nflag = (n<0)? n = -n, 1 : 0;
    int32 mask7 = bitmask(7);

#define NFLAG 0x40
#define MORE  0x80
/* The prefix notation expressed in this function is described */
/* in Nick Garnett's description of the Helios Link Format for */
/* Non-Transputer Processors                                   */

    for( i = 28; i != 0; i -= 7 )
      if ((n & (mask7 << i)) != 0) break;

    if( (n >> i) & NFLAG ) i += 7;

    objbyte( (char)((n >> i) | (nflag? NFLAG:0) | (i>0? MORE: 0)) );

    for( i -= 7; i >= 0; i -= 7)
       objbyte( (char)(((n >> i) & mask7) | (i>0? MORE: 0)) );
}

void objstring(char *s)
{  char ch;
   while((ch = *s++)!=0)
      objbyte(ch);
   objbyte(0);
}

void advancecodebuf(void)
{
   codestart.seg = codep.seg;
   codestart.p   = codep.p;
   bufstart     = pcloc;
}

void outputcodebuf()
{
   codeptr outcodep;

   if( pass == 1 ) return;

   if( pcloc-bufstart == 0 ) return;

   directive(D_CODE,pcloc-bufstart);

   for( outcodep.seg = codestart.seg; outcodep.seg <= codep.seg; outcodep.seg++)
   {  uint32 thismaxcodep;
      CodeSeg *cseg = code[outcodep.seg];

      thismaxcodep = outcodep.seg == codep.seg? codep.p : CODEBUFSIZE;

      for( outcodep.p = codestart.p; outcodep.p < thismaxcodep; outcodep.p++)
      {
         switch( cseg->codeflags[outcodep.p] )
         {
         case 4:
            {  uint32 val = cseg->codebuf[outcodep.p].b4;
               objbyte((char)val); val>>=8;
               objbyte((char)val); val>>=8;
               objbyte((char)val); val>>=8;
               objbyte((char)val);
            }
            break;
         case 2:
            {  short val = cseg->codebuf[outcodep.p].b2;
               objbyte((char)val); val>>=8;
               objbyte((char)val);
            }
            break;
         case 1:
            objbyte(cseg->codebuf[outcodep.p].b1);
            break;
         }
      }
   }
   advancecodebuf();
}

int32 vdirective(int dir, va_list *app )
{  int32 r = 0;

   switch( dir )
   {
   case D_MODULE:
   case D_BSS:
   case D_CODE:
      {  int n = va_arg(*app,int);
         objbyte(dir);
         objnum(n);
         break;
      }
      
   case D_IMAGESIZE:
   case D_MODNUM:
   case D_INIT:
      objbyte(dir);
      break;

   case D_BYTE:
   case D_SHORT:
   case D_WORD:
      {  int32 codeword;
         int size = dir == D_BYTE? 1:
                    dir == D_SHORT? 2: 4;

         objbyte(dir);
         dir = va_arg(*app,int);
         codeword = vdirective(dir,app);
         output(size,codeword);
         advancecodebuf();
         break;
      }
   case D_DATASYMB:
   case D_DATAMODULE:
   case D_LABELOFF:
   case D_LABEL:
   case D_GLOBAL:
      {  char *s = va_arg(*app,char *);
         objbyte(dir);
         objstring(s);
         break;
      }
   case D_COMMON:
   case D_DATA:
      {  int n = va_arg(*app,int);
         char *s = va_arg(*app,char *);
         objbyte(dir);
         objnum(n);
         objstring(s);
         break;
      }
   case D_SHIFT:
   case D_ADD:
      {  r = va_arg(*app,int32);

         objbyte(dir);
         objnum(r);
         dir = va_arg(*app,int);
         vdirective(dir,app);
         break;
      }
   default:
      fatal("Internal error - bad directive op %02x",dir);
   }
   return r;
}

void outputexpr(int size, Expression *expr)
{  outputcodebuf();
   switch( size )
   {
   case 1:
      objbyte(D_BYTE); break;
   case 2:
      objbyte(D_SHORT); break;
   case 4:
      objbyte(D_WORD);  break;
   default:
      error("Can't generate directives of size %d\n",size);
   }
   
   switch( expr->exprtype & E_OPMASK )
   {
   case E_IMAGESIZE:
      directive(D_IMAGESIZE);
      break;
   case E_MODNUM:
      directive(D_MODNUM);
      break;
   case E_SYMBOL:
      {  Symbol *sym = expr->e1.symbol;
         switch( sym->symtype )
         {
         case S_NULL:
            error("Type of symbol %s unknown",sym->name);
            break;
         case S_LABEL:
            directive(D_LABELOFF,sym->name);
            break;
         case S_DATA:
            directive(D_DATASYMB,sym->name);
            break;
         default:
            error("Attempt to output symbol of bad type");
            break;
         }
         break;
      }
   default:
      error("Can't output expression type %d yet!",expr->exprtype );
      break;
   }
   pcloc += size;
   advancecodebuf();
}

void endoutput(void)
{
   if( srecordfile )
   {  outsrec(1);
      srecaddr = 0;
      outsrec(9);
      fclose(srecordfile);
   }
   if( objectfile )
      outputcodebuf();
}

void directive(int dir, ... )
{
   va_list ap;

   if( pass == 1 ) return;
   va_start(ap,dir);

   if( dir != D_CODE ) outputcodebuf();

   vdirective(dir, &ap);

   va_end(ap);
}

void reinitcodebuf( uint32 newbufstart )
{
   codep.seg = 0;
   codep.p = 0;
   codestart.seg = 0;
   codestart.p = 0;
   listcodep.seg = 0;
   listcodep.p = 0;
   bufstart = newbufstart;
}

void initcodebuf(void)
{
   int i;
   for( i=0; i<NCODESEGS; i++) code[i] = NULL; 
   reinitcodebuf(0);
   srecpos = 0;
}
