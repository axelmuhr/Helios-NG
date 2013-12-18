/* -> code/c
 * Title:               Code file writing
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/

#include "asm.h"
#include "errors.h"
#include "helios.h"
#include "osdepend.h"
#include "occur.h"
#include "code.h"
#include "globvars.h"
#include "listing.h"
#include "nametype.h"
#include "asmvars.h"
#include "store.h"
#include "tables.h"
#include "version.h"

/*---------------------------------------------------------------------------*/

#define MaxBuffer 4096

#define RelocBufferSize 256
#define RelocsInBuffer 128

#define MaxChunks 7

#define NumChunks 5

#ifdef __NCCBUG
char     *buffer = NULL ;
char     *ptr = NULL ;
#else
char     buffer[MaxBuffer + 3] ;        /* +3 to account for word overflow */
char     *ptr = buffer ;
#endif

CARDINAL codePtr ;

CARDINAL               totalAreas ;
CARDINAL               relocPos ;
CARDINAL               endHeaderPtr ;
CARDINAL               endFixedHeaderPtr ;
CARDINAL               endAreaPtr ;
CARDINAL               endSymbolsPtr ;
CARDINAL               endStringsPtr ;
CARDINAL               endIdPtr ;

char                   relocBuffer[RelocBufferSize] ;
char                  *relocPtr = relocBuffer ;

static long int        relocFilePtr;

/*---------------------------------------------------------------------------*/

#define ENC_NEG         (0x40)
#define ENC_MORE        (0x80)
#define LO6MASK         (0x3F)
#define LO7MASK         (0x7F)

/*---------------------------------------------------------------------------*/
/* The following should ALL be "static" */

static unsigned CodeEncode(unsigned n,int nflag)
{
 if ((n >> 6) == 0)
  return((n & LO6MASK) | nflag) ;
  CodeByte((CodeEncode((n >> 7),nflag) | ENC_MORE)) ;
 return(n & LO7MASK) ;
}

/*---------------------------------------------------------------------------*/

static unsigned Encode(unsigned n,int nflag,FILE *fp)
{
 if ((n >> 6) == 0)
  return((n & LO6MASK) | nflag) ;
 putc(((Encode((n >> 7),nflag,fp) | ENC_MORE)),fp) ;
 return(n & LO7MASK) ;
}

/*---------------------------------------------------------------------------*/

/* write an encoded number */
static void write_encoded(int n,FILE *fp)
{
 putc(((n < 0) ? Encode(-n,ENC_NEG,fp) : Encode(n,0,fp)),fp) ;
}

/*---------------------------------------------------------------------------*/

static void write_string(char *text,int length,FILE *fp)
{
 for (; (length > 0); length--)
  putc(*text++,fp) ;
 putc('\0',fp) ;
}

/*---------------------------------------------------------------------------*/

static void write_stringtrans(char *text,int length,FILE *fp)
{
 int leadingbar = 0 ;

 if (traceon)
  {
   int loop ;
   printf("write_stringtrans: \"") ;
   for (loop = 0; (loop < length); loop++)
    printf("%c",text[loop]) ;
   printf("\"\n") ;
  }

#if 1
 /* Add a "_" character to symbols beginning with 128 */
 if ((unsigned char)(*text) == (unsigned char)128)
  {
   text++ ;
   length -- ;
   putc('_',fp) ;
  }
 else
  {
#if 1
   if (*text == '|')
    {
     leadingbar = -1 ;
     text++ ;
     length-- ;
    }
   else
    {
     if (*text == '.')
      {
       /* Remove the leading "." */
       text++ ;
       length-- ;
      }
     else
      /* Insert a leading "." */
      putc('.',fp) ;
    }
#else
   if (*text == '.')
    {
     /* Remove the leading "." */
     text++ ;
     length-- ;
    }
   else
    /* Insert a leading "." */
    putc('.',fp) ;
#endif
  }
 for (; (length > 0); length--)
#if 1
  if ((length != 1) || !((*text == '|') && (leadingbar)))
   putc(*text++,fp) ;
#else  
  putc(*text++,fp) ;
#endif
#else
 /* Remove the "|" characters from suitably encased symbols */
 /* Do not alter symbols beginning with "_" */
 if (*text != '_')
  {
   if (*text == '|')
    {
     /* Remove initial "|" character */
     text++ ;
     length-- ;
    }
   else
    {
     if (*text == '.')
      {
       /* Remove the leading "." */
       text++ ;
       length-- ;
      }
     else
      /* Insert a leading "." */
      putc('.',fp) ;
    }
  }
 for (; (length > 0); length--)
  {
   if ((length != 1) || (*text != '|')) /* do not output terminating "|" */
    putc(*text++,fp) ;
  }
#endif
 putc('\0',fp) ;
}

/*---------------------------------------------------------------------------*/

void RelocInit(void)
{
#ifdef __NCCBUG
 if (buffer == NULL)
  {
   buffer = (char *)mymalloc(MaxBuffer + 3) ;
   ptr = buffer ;
  }
#endif

 /* Helios objects contain no direct relocation info */

 codePtr = 0 ;
} /* End RelocInit */

/*---------------------------------------------------------------------------*/

void RelocEnd(void)
{
 fseek(codeStream,relocFilePtr,SEEK_SET) ;
 /* Helios objects contain no direct relocation info */
} /* End RelocEnd */

/*---------------------------------------------------------------------------*/

void CodeInit(void)
{
 fseek(codeStream,endHeaderPtr,SEEK_SET) ;
} /* End CodeInit */

/*---------------------------------------------------------------------------*/

/* "CodeEnd" is called to flush the code buffer */
void CodeEnd(void)
{
 /* This will write a non-word-aligned amount */
 if (codePtr != 0)
  {
   /* write the Helios CODE patch */
   putc(HOF_t_code,codeStream) ;
   write_encoded(codePtr,codeStream) ;
 
   fwrite(ptr,1,codePtr,codeStream) ;
  }

 codePtr = 0 ;
 return ;
}

/*---------------------------------------------------------------------------*/
/* "CodeByte" is called to place a byte value into the code buffer */
void CodeByte(char byte)
{
 if (noInitArea)
  {
   /* allow BSS */
   if (byte != 0)
    Warning(BadNoInit);
   return ;
  }

 ptr[codePtr++] = byte ;
 if (codePtr == MaxBuffer)
  CodeEnd() ;

 programCounter++ ;             /* increment assembler program counter */
 ListByteValue(byte) ;          /* and display value if requested */
 return ;
}

/*---------------------------------------------------------------------------*/
/* "CodeWord" is called to place a word value into the code buffer */
/* (assumes a word-aligned put) */
void CodeWord(CARDINAL value)
{
 CARDINAL  n ;
 char     *adr = ptr + codePtr ;

 if (noInitArea)
  {
   /* allow BSS */
   if (value != 0)
    Warning(BadNoInit) ;
   return ;
  }

 (*(int *)adr) = value ;
 codePtr += 4 ;
 if (codePtr >= MaxBuffer)              /* buffer overflow, so flush */
  {
   /* write the Helios CODE patch */
   putc(HOF_t_code,codeStream) ;
   write_encoded(MaxBuffer,codeStream) ;
   fwrite(ptr,1,MaxBuffer,codeStream) ;
   codePtr -= MaxBuffer ;
   for (n = 0; n <= 3; n++)
    ptr[n] = ptr[MaxBuffer + n] ;
  }

 programCounter += 4 ;          /* increment assembler program counter */
 ListWordValue(value) ;         /* and display value if requested */
 return ;
}

/*---------------------------------------------------------------------------*/

void AddSymbol(SymbolPointer symbolPointer)
{
#ifdef DEBUG
 printf("AddSymbol: \"") ;
 PrintSymbol(symbolPointer->key) ; 
 printf("\"\n") ;
#endif
 symbolPointer->aOFData.stringPosition = stringOffset ;
 symbolPointer->aOFData.symbolId = symbolId++ ;
 stringOffset += (symbolPointer->key.length + 4) & ~ 3 ;
} /* End AddSymbol */

/*---------------------------------------------------------------------------*/

void AddSymbolToTable(SymbolPointer symbolPointer,Name name,BOOLEAN external,BOOLEAN nonimp)
{
 CARDINAL i ;

#ifdef DEBUG
 printf("AddSymbolToTable: \"") ;
 PrintSymbol(symbolPointer->key) ; 
 printf("\" \"") ;
 PrintSymbol(name) ;
 printf("\"\n") ;
#endif

 /* GLOBAL definitions can occur at any time "EXPORT" */
 /* normal labels should be inserted into the code at
  * definition time.
  */
 if (external)
  write_label(HOF_t_global,symbolPointer->key.key,symbolPointer->key.length) ;
 else
  write_label(HOF_t_label,symbolPointer->key.key,symbolPointer->key.length) ;

 i = symbolPointer->aOFData.stringPosition;

  memcpy(stringTable+i,name.key,name.length) ;
  do
   stringTable[i+++name.length] = 0 ;
  while ((i+name.length) % 4 != 0) ;

 return ;
 nonimp = nonimp ;
} /* End AddSymbolToTable */

/*---------------------------------------------------------------------------*/

void P1InitAreas(void)
{
 segment_type = CodeST ;
 area_is_code = TRUE ;
 code_size = 0 ;
 data_size = 0 ;
 bss_size = 0 ;
} /* End P1InitAreas */

/*---------------------------------------------------------------------------*/

void P2InitAreas(void)
{
 segment_type = CodeST ;
 area_is_code = TRUE ;
 code_size = 0 ;
 data_size = 0 ;
 bss_size = 0 ;
} /* End P2InitAreas */

/*---------------------------------------------------------------------------*/

void DumpSymbolTable(void)
{
 endAreaPtr = (CARDINAL)ftell(codeStream) ;

 /* Helios symbols threaded in the object */

 endSymbolsPtr = (CARDINAL)ftell(codeStream) ;
} /* End DumpSymbolTable */

/*---------------------------------------------------------------------------*/

void DumpStringTable(void)
{
 /* Helios strings are threaded in the object */
 endStringsPtr = (CARDINAL)ftell(codeStream) ;

 /* Helios strings are threaded in the object */
} /* End DumpStringTable */

/*---------------------------------------------------------------------------*/

void DumpHeader(void)
{
 /* HELIOS object format */
 fseek(codeStream,0,SEEK_SET) ;

 if (!librarycode)
  {
   putc(HOF_t_module,codeStream) ;
   write_encoded(heliosHdr.helios_module,codeStream) ;
  }

 endFixedHeaderPtr = (CARDINAL)ftell(codeStream) ;
 endHeaderPtr = endFixedHeaderPtr ;
 relocFilePtr = endHeaderPtr ;
} /* End DumpHeader */

/*---------------------------------------------------------------------------*/

void DumpAreaDecs(void)
{
 CARDINAL l = (CARDINAL)ftell(codeStream) ;

 /* helios objects do not contain areas */

 fseek(codeStream,l,SEEK_SET) ;
} /* End DumpAreaDecs */

/*---------------------------------------------------------------------------*/

void write_dotlabel(int type,char *text,int length)
{
 CodeEnd() ;
 /* Force a "." prefixed symbol to be generated */
 putc(type,codeStream) ;
 putc('.',codeStream) ;
 for (; (length > 0); length--)
  putc(*text++,codeStream) ;
 putc('\0',codeStream) ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_purelabel(int type,char *text,int length)
{
 CodeEnd() ;
 putc(type,codeStream) ;
 write_string(text,length,codeStream) ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_label(int type,char *text,int length)
{
 CodeEnd() ;
 putc(type,codeStream) ;
 write_stringtrans(text,length,codeStream) ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_patch(int type,CARDINAL instruction)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;          /* all ARM patches are 32bits */
 putc(type,codeStream) ;
 write_encoded(instruction,codeStream) ;
 programCounter += 4 ;
 ListWordValue(instruction) ;   /* display if we are asked to */
 return ;
}

/*---------------------------------------------------------------------------*/

void write_bss(int size)
{
 CodeEnd() ;
 putc(HOF_t_bss,codeStream) ;
 write_encoded(size,codeStream) ;
 programCounter += size ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_init(void)
{
 CodeEnd() ;
 putc(HOF_t_init,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

/* The following generates no object code */
void write_staticarea(int type,int value,SymbolPointer sp)
{
 CodeEnd() ;

 putc(type,codeStream) ;

 if (type != HOF_t_codetable)
  write_encoded(value,codeStream) ;

 write_stringtrans(sp->key.key,sp->key.length,codeStream) ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_labeldir(SymbolPointer sp)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_labelref,codeStream) ;
 putc('.',codeStream) ;
 write_string(sp->key.key,sp->key.length,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_labeldirname(Name *name)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_labelref,codeStream) ;
 putc('.',codeStream) ;
 write_string(name->key,name->length,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_imagesize(void)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_imagesize,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_modnum(void)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_modnum,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_modnum1(SymbolPointer sp)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_datamod,codeStream) ;
 write_stringtrans(sp->key.key,sp->key.length,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_modnum2(SymbolPointer sp)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_patch_m68kshift,codeStream) ;
 if (clmake_SMT)
  write_encoded(-3,codeStream) ;
 else
  write_encoded(-2,codeStream) ;
 putc(HOF_t_datamod,codeStream) ;
 write_stringtrans(sp->key.key,sp->key.length,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_modnum4(void)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_patch_m68kshift,codeStream) ;
 if (clmake_SMT)
  write_encoded(-3,codeStream) ;
 else
  write_encoded(-2,codeStream) ;
 putc(HOF_t_modnum,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_modnum5(void)
{
 CodeEnd() ;
 putc(HOF_t_patch_m68kshift,codeStream) ;
 if (clmake_SMT)
  write_encoded(3,codeStream) ;
 else
  write_encoded(2,codeStream) ;
 putc(HOF_t_modnum,codeStream) ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_dataref(SymbolPointer sp)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_dataref,codeStream) ;
 write_stringtrans(sp->key.key,sp->key.length,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_datarefname(Name *name)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_dataref,codeStream) ;
 write_stringtrans(name->key,name->length,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_coderef(SymbolPointer sp)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_codesymbol,codeStream) ;
 write_stringtrans(sp->key.key,sp->key.length,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/

void write_coderefname(Name *name)
{
 CodeEnd() ;
 putc(HOF_t_word,codeStream) ;
 putc(HOF_t_codesymbol,codeStream) ;
 write_stringtrans(name->key,name->length,codeStream) ;
 programCounter += 4 ;
 return ;
}

/*---------------------------------------------------------------------------*/
/* EOF code/c */
