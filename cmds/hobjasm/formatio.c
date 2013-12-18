/* -> formatio/c
 * Title:               Produce formatted listings
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "formatio.h"
#include "osdepend.h"
#include "constant.h"
#include "getline.h"
#include "nametype.h"
#include <ctype.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

#define DefaultCols 131
#define DefaultRows 60

char *currentLinePointer ;
CARDINAL maxCols = DefaultCols ;
CARDINAL maxRows = DefaultRows ;
CARDINAL currCols ;
CARDINAL currRows ;
CARDINAL page ;
BOOLEAN  paging ;
char     title[MaxLineLength + 1] = "" ;
char     subtitle[MaxLineLength + 1] = "" ;

/*---------------------------------------------------------------------------*/

static BOOLEAN GetValue(char *line,CARDINAL *value)
{
 CARDINAL index;

 if (!isdigit(*line))
  {
   WriteChs("Bad value\\N") ;
   return(FALSE) ;
  }

 *value = *line - '0' ;
 index = 1 ;
 while (isdigit(line[index]))
  {
   *value = *value*10 + line[index++] - '0' ;
   if (*value >= MaxVal)
    {
     WriteChs("Value too large\\N") ;
     return(FALSE) ;
    }
  }
 return(TRUE) ;
} /* End GetValue */

/*---------------------------------------------------------------------------*/
/* Set formatted output width */
void SetWidth(char *line)
{
 CARDINAL value ;
 if (GetValue(line,&value))
  maxCols = value ;
} /* End SetWidth */

/*---------------------------------------------------------------------------*/
/* Set formatted output length */
void SetLength(char *line)
{
 CARDINAL value ;
 if (GetValue(line,&value))
  maxRows = value ;
} /* End SetLength */

/*---------------------------------------------------------------------------*/
/* Initialise the output stream */
void PageModeOn(void)
{
 page = 0 ; /* Ready to increment */
 PageThrow() ;
 paging = TRUE ;
} /* End PageModeOn */

/*---------------------------------------------------------------------------*/

void PageModeOff(void)
{
 paging = FALSE ;
} /* End PageModeOff */

/*---------------------------------------------------------------------------*/

static void NewPage(void)
{
 printf("%c",FF) ;
 PageThrow() ;
} /* End NewPage */

/*---------------------------------------------------------------------------*/

void PageThrow(void)
{
 currRows = maxRows ; /* New page required */
} /* End PageThrow */

/*---------------------------------------------------------------------------*/
/* Write a character to the printed output stream */
void WriteCh(char ch)
{
 CARDINAL index ;

 if ((currRows == maxRows) && paging)
  {
   page++ ;
   NewPage() ;
   printf("\n\n\nARM Macro Assembler    Page %u\n",page) ;
   /* Now output the title */
   index = 0 ;
   while ((index <= MaxLineLength) && (title[index] != '\0'))
    printf("%c",title[index++]) ;
   printf("\n") ;
   index = 0 ;
   while ((index <= MaxLineLength) && (subtitle[index] != '\0'))
    printf("%c",subtitle[index++]) ;
   printf("\n\n") ;
   currRows = 7 ;
   currCols = 0 ;
  }
 if ((ch == CR) || (ch == LF))
  {
   printf("\n") ;
   if (paging)
    {
     currRows++ ;
     currCols = 0 ;
    }
  }
 else
  {
   if (paging)
    {
     if (currCols < maxCols)
      {
       printf("%c",ch) ;
       currCols++ ;
      }
     else
      {
       WriteCh(CR) ; /* Wrap */
       WriteCh(ch) ;
      }
    }
   else
    printf("%c",ch) ;
  }
} /* End WriteCh */

/*---------------------------------------------------------------------------*/
/* See if character is decimal/hexadecimal */
static BOOLEAN CharIsDigit(char ch,BOOLEAN Hex,CARDINAL *n)
{
 if (isdigit(ch))
  {
   *n = *n * 16 + ch - '0' ;
   return(TRUE) ;
  }
 if (Hex && isxdigit(ch))
  {
   *n = *n*16 + 10 + ch - 'A' ;
   return(TRUE) ;
  }
 return(FALSE) ;
} /* End CharIsDigit */

/*---------------------------------------------------------------------------*/

char TransEscape(char *chs,CARDINAL *index)
{
 CARDINAL n ;
 if (*index < strlen(chs))
  {
   (*index)++ ;
   switch (chs[*index])
    {
     case 'N' :
     case 'n' :
                return(CR) ;

     case 'P' :
     case 'p' :
                return(FF) ;

     case 'S' :
     case 's' :
                return(Space) ;

     case 'T' :
     case 't' :
                return(TabCh) ;

     case 'C' :
     case 'c' :
                return(LF) ;

     case 'X' :
     case 'x' :
                if (*index < strlen(chs))
                 {
                  n = 0 ;
                  if (CharIsDigit(chs[*index + 1],TRUE,&n))
                   {
                    (*index)++ ;
                    if (*index < strlen(chs))
                     {
                      if (CharIsDigit(chs[*index + 1],TRUE,&n))
                       (*index)++ ;
                     }
                    return(n) ;
                   }
                 }
                (*index)-- ;
                return(chs[*index - 1]) ;

     default  : /* no translation */
                return(chs[*index]) ;
    }
  }
 else
  return(EscapeCh) ;
} /* End TransEscape */

/*---------------------------------------------------------------------------*/
/* Write a string of characters to the terminal stream */
void WriteChs(char *chs)
{
 CARDINAL index ;

 index = 0 ;
 while (index < strlen(chs))
  {
   WriteCh((chs[index] != EscapeCh) ? chs[index] : TransEscape(chs,&index)) ;
   index++ ;
  }
} /* End WriteChs */

/*---------------------------------------------------------------------------*/

void WriteInteger(int i)
{
 char digits[12] ;
 sprintf(digits,"%i",i) ;
 WriteChs(digits) ;
} /* End WriteInteger */

/*---------------------------------------------------------------------------*/

void WriteCardinal(CARDINAL c)
{
 char digits[12] ;
 sprintf(digits,"%u",c) ;
 WriteChs(digits) ;
} /* End WriteCardinal */

/*---------------------------------------------------------------------------*/

void WriteHexCardinal(CARDINAL c)
{
 char digits[12] ;
 int  index ;
 int  digit ;

 for (index = 7; index >= 0; index--)
  {
   digit = c % 16 ;
   digits[index] = (digit < 10) ? digit + '0' : digit - 10 + 'A' ;
   c = c >> 4 ;
  }
 digits[8] = '\0' ;
 WriteChs(digits) ;
} /* End WriteHexCardinal */

/*---------------------------------------------------------------------------*/

void SetTitle(Name t)
{
 CARDINAL index = 0 ;

 while (index < t.length)
  {
   title[index] = t.key[index] ;
   index++ ;
  }
 if (index <= MaxLineLength)
  title[index] = '\0' ;
} /* End SetTitle */

/*---------------------------------------------------------------------------*/

void SetSubtitle(Name t)
{
 CARDINAL index = 0 ;

 while (index < t.length)
  {
   subtitle[index] = t.key[index] ;
   index++ ;
  }
 if (index <= MaxLineLength)
  subtitle[index] = '\0' ;
} /* End SetSubtitle */

/*---------------------------------------------------------------------------*/
/* EOF formatio/c */
