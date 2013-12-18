/* -> vars/c
 * Title:               The string variable handling stuff
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#include "constant.h"
#include "errors.h"
#include "getline.h"
#include "globvars.h"
#include "mactypes.h"
#include "nametype.h"
#include "asmvars.h"
#include "tables.h"
#include "vars.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

BOOLEAN termin[256];

char     substLine[MaxLineLength+1],
        *substLinePointer = substLine;

char *SubstituteString(char *string)
/*Substitute the string variables in an input line*/
{
  CARDINAL      i = 0,
                j = 0,
                l,
                m;
  char         *line = substLinePointer,
                ch,
                stringValue[8];
  Name          name;
  SymbolPointer symbolPointer;
  StringName    stringName;
  int           k;

  while (ch = string[i], ch != CR) {
    if (ch != Dollar) {
      line[j++] = ch;
      i++;
      if (j >= MaxLineLength) {
        Warning(SubstLong);
        exception = EndOfInput;
        return line;
      }; /* if */
      if (ch == Bar) {
        do {
          ch = string[i++];
          line[j++] = ch;
          if (j >= MaxLineLength) {
            Warning(SubstLong);
            exception = EndOfInput;
            return line;
          }; /* if */
        } while ((ch != Bar) && (ch != CR));
        if (ch == CR) { i--; j--; }; /* if */
      }; /* if */
    } else {
      k = i++; /* past $*/
      if (string[i] == Dollar) {
        line[j++] = Dollar;/*contract*/
        i++;
        if (j >= MaxLineLength) {
          Warning(SubstLong);
          exception = EndOfInput;
          return line;
        }; /* if */
      } else if (SymbolTest(string, &i, &name)) {
        symbolPointer = LookupLocal(name);
        if (symbolPointer == NULL) symbolPointer = LookupRef(name, TRUE);
        if ((symbolPointer != NULL) &&
          (symbolPointer->u.s.sdt == VariableSDT)) {
          /*Here we substitute*/
          stringName.key = stringValue;
          if (symbolPointer->u.s.vst == ArithmeticVST) {
            stringName.length = 8;
            l = symbolPointer->value.card;
            for (k = 7; k >= 0; k--) {
              m = l % 0x10;
              l /= 0x10;
              stringValue[k] = (m >= 10) ? m + 'A' - 10 : m + '0';
            }; /* for */
          } else if (symbolPointer->u.s.vst == LogicalVST) {
            strcpy(stringValue, (symbolPointer->value.bool) ? "T" : "F");
            stringName.length = 1;
          } else if (symbolPointer->u.s.vst == StringVST)
            stringName = *symbolPointer->value.ptr;
          if (stringName.length > 0) {
            for (k = 0; k < stringName.length; k++) {
              line[j++] = stringName.key[k];
              if (j >= MaxLineLength) {
                Warning(SubstLong);
                exception = EndOfInput;
                return line;
              }; /* if */
            };/* for */
          }; /* if */
          /*allow terminating dot*/
          if (string[i] == Dot) i++;
        } else {
          /*Not a string symbol*/
          for (l = k; l < i; l++) {
            line[j++] = string[l];
            if (j >= MaxLineLength) {
              Warning(SubstLong);
              exception = EndOfInput;
              return line;
            }; /* if */
          }; /* for */
        }; /* if */
      } else {
        /*No symbol found*/
        line[j++] = Dollar;
        if (j >= MaxLineLength) {
          Warning(SubstLong);
          exception = EndOfInput;
          return line;
        }; /* if */
      }; /* if */
    }; /* if */
  }; /* while */
  line[j] = CR;
  return line;
} /* End SubstituteString */

/*---------------------------------------------------------------------------*/
/* Discover a symbol in line and return it in retSymbol */

BOOLEAN SymbolTest(char *string,CARDINAL *index,Name *retSymbol)
{
 char *sy = string + *index ;
 char  ch = *sy ;

 /* Allow all symbols to begin with UnderLine characters */
 if (!(isalpha(ch) || (ch == UnderLine)))
  {
   /* Doesn't start with alphabetic, so can only be a free format symbol */
   if ((ch != Bar) || (sy[1] == Bar) || (sy[1] == CR))
    return FALSE ;
   sy++;
   while ((*sy != Bar) && (*sy != CR))
    sy++ ;
   if (*sy == Bar)
    {
     sy++ ;
     retSymbol->length = sy - string - 2 - *index ;
     retSymbol->key = string + *index + 1 ;
     *index = sy - string ;
     return(TRUE) ;
    } ; /* if */
   return(FALSE) ;
  } ;

 sy++ ;
 while (isalnum(*sy) || (*sy == UnderLine))
  sy++ ;
 retSymbol->key = string + *index ;
 retSymbol->length = sy - retSymbol->key ;
 *index = sy - string ;

 return(TRUE) ;
} /* End SymbolTest */

/*---------------------------------------------------------------------------*/
/* Discover a symbol in line and return it in retSymbol (non-destructively) */

BOOLEAN SymbolScan(char *string,CARDINAL index,Name *retSymbol)
{
 char *sy = string + index ;    /* starting index */
 char  ch = *sy ;               /* initial character */

 /* Allow all symbols to begin with UnderLine characters */
 if (!(isalpha(ch) || (ch == UnderLine)))
  {
   /* Doesn't start with alphabetic, so can only be a free format symbol */
   if ((ch != Bar) || (sy[1] == Bar) || (sy[1] == CR))
    return FALSE ;
   sy++;
   while ((*sy != Bar) && (*sy != CR))
    sy++ ;
   if (*sy == Bar)
    {
     sy++ ;
     retSymbol->length = sy - string - 2 - index ;
     retSymbol->key = string + index + 1 ;
     return(TRUE) ;
    } ; /* if */
   return(FALSE) ;
  } ;

 sy++ ;
 while (isalnum(*sy) || (*sy == UnderLine))
  sy++ ;
 retSymbol->key = string + index ;
 retSymbol->length = sy - retSymbol->key ;
 return(TRUE) ;
} /* End SymbolScan */

/*---------------------------------------------------------------------------*/

BOOLEAN DirTest(char *line, CARDINAL *index, Name *name)
/* Like symbol test but less stringent */
{
/*
  CARDINAL new = *index;

  name->key = line + new;
  if (termin[line[new]]) return FALSE;
  new++;
  while (!(termin[line[new]])) new++;
  name->length = new - *index;
  while (line[new] == Space) new++;
  *index = new;
  return TRUE;
*/
  line = line+*index;
  name->key = line;
  if (termin[*line]) return FALSE;
  line++;
  while (!(termin[*line])) line++;
  name->length = line - name->key;
  while (*line == Space) line++;
  *index += line - name->key;
  return TRUE;
} /* End DirTest */

void Init_Variables(void)
{
  CARDINAL i;
  for (i = 0; i <= 255; i++) termin[i] = FALSE;
  termin[CR] = TRUE;
  termin[Space] = TRUE;
  termin[CommentSymbol] = TRUE;
}

/* End vars/c */
