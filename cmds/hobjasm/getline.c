/* -> getline/c
 * Title:               The general line generation routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "asm.h"
#include "constant.h"
#include "errors.h"
#include "formatio.h"
#include "getline.h"
#include "globvars.h"
#include "mactypes.h"
#include "asmvars.h"
#include <string.h>
#include <stdio.h>

/*---------------------------------------------------------------------------*/

char  ownLine[MaxLineLength+1] ;
char *line = ownLine ;
char *pointerInFile ;
char *oldPointerInFile ;

CARDINAL lineNumber ;

FileReadMode fileReadMode ;

/*---------------------------------------------------------------------------*/
/* The returned value says whether string substitution is required */

BOOLEAN GetLine(char **string)
{
 CARDINAL  index ;
 char      termChar ;
 char      ch ;
 BOOLEAN   dollarFound = FALSE ;
 char     *endOfFile ;
 char     *local ;

 listStatus = nextListState ;
 linePrinted = FALSE ;
 lineNumber++ ;
 if (abortFlag)
  {
   /* Panic! */
   strcpy(ownLine,(inMacroDef) ? "  MEND\r" : (macroLevel == 0) ? "   END\r" : " MEXIT\r") ;
   *string = ownLine ;
   return FALSE ;
  }
 if (macroLevel != 0)
  {
   *string = pointerInFile ;
   currentLinePointer = *string ;
   oldPointerInFile = *string ;
   return MacroSubstitute(string,line) ;
  }
 switch (fileReadMode)
  {
   case WholeFileLoad : endOfFile = fileStore + fileSize ;

                        local = pointerInFile ; /* Return the start of the line */
                        *string = local ;
                        currentLinePointer = local ; /* For the purposes of printing */
                        oldPointerInFile = local ;
                        index = 0 ;
                        do
                         {
                          if (index >= MaxLineLength)
                           {
                            local[MaxLineLength] = CR ;
                            WarningReport("Line too long") ;
                            exception = EndOfInput ;
                            return FALSE ;
                           }
                          if (local + index >= endOfFile)
                           {
                            *string = ownLine ;
                            *ownLine = '\r' ;
                            currentLinePointer = *string ;
                            WarningReport("End of input file (whole)") ;
                            exception = EndOfInput ;
                            return FALSE ;
                           }
                          ch = local[index] ;
                          if ((ch >= Space) && (ch != Del))
                           {
                            if (ch == Dollar)
                             dollarFound = TRUE ;
                           }
                          else
                           if (ch == TabCh)
                            local[index] = Space ;
                           else
                            break ;
                          index++ ;
                         } while (1) ;
                        if (ch == LF)
                         {
                          termChar = LF ;
                          local[index] = CR ;
                         }
                        else
                         {
                          termChar = CR ;
                          if (ch != CR)
                           {
                            local[index] = CR ;
                            WarningReport("Bad character") ;
                            exception = EndOfInput ;
                            return FALSE ;
                           }
                         }
                        /* Now step past the line ready for the next one */
                        pointerInFile = local + index + 1 ;
                        if (((*pointerInFile == CR) && (termChar == LF)) || ((*pointerInFile == LF) && (termChar == CR)))
                         {
                          /* Allow CRLF or LFCR terminated lines */
                          pointerInFile++ ;
                         }
                        break ;

   case ByteStream    : index = (CARDINAL)ftell(inputStream) ;
                        oldPointerInFile = (char *)index ; /* For the benefit of WHILE/WEND */
                        *string = line ;
                        currentLinePointer = line ;
                        index = 0 ;
                        do
                         {
                          ch = fgetc(inputStream) ; /* Next character */
                          if (ferror(inputStream))
                           {
                            **string = CR ;
                            WarningReport("End of input file (byte)") ;
                            exception = EndOfInput ;
                            return FALSE ;
                           }
                          if ((ch >= Space) && (ch != Del))
                           {
                            ownLine[index] = ch ;
                            if (ch == Dollar)
                             dollarFound = TRUE ;
                           }
                          else
                           if (ch == TabCh)
                            ownLine[index] = Space ;
                           else
                            if ((ch == CR) || (ch == LF)) 
                             {
                              termChar = ch ;
                              ownLine[index] = CR ;
                              /* TRY */
                              ch = fgetc(inputStream) ;
                              if (!ferror(inputStream))
                               {
                                if (((termChar == CR) && (ch != LF)) || ((termChar == LF) && (ch != CR)))
                                 fseek(inputStream,-1,SEEK_CUR) ;
                                /* Don't ignore it */
                               }
                              break ; /* We've got a line now */
                             }
                            else
                             {
                              ownLine[index] = CR ;
                              WarningReport("Bad character") ;
                             }
                          index++ ;
                          if (index >= MaxLineLength)
                           {
                            WarningReport("Line too long") ;
                            exception = EndOfInput ;
                            return FALSE ;
                           }
                         } while (1) ;
                        break ;
  }
 currentLinePointer = *string ; /* Update so we list what we parse */
 return dollarFound ;
} /* End GetLine */

/*---------------------------------------------------------------------------*/
/* EOF getline/c */
