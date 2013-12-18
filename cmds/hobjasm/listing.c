/* -> listing/c
 * Title:               Pretty listing stuff
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "osdepend.h"
#include "constant.h"
#include "errors.h"
#include "formatio.h"
#include "getline.h"
#include "globvars.h"
#include "listing.h"
#include "nametype.h"
#include "asmvars.h"
#include "p1hand.h"
#include "vars.h"
#include <string.h>

/*---------------------------------------------------------------------------*/

typedef int Hex; /* [0..15] */

/*
newLine used for options to print command,
and also buffered output line for printing
*/
char newLine[2*MaxLineLength+1];
#define NumberOfOptions 4
Name options[NumberOfOptions];
CARDINAL linePosition;
static BOOLEAN  initialised = FALSE;

#define PCStart       7
#define CodeStart     (PCStart + 9)
#define TextStart     (CodeStart + 9)
#define OpcodeStart   (TextStart + 11)
#define OperandStart  (OpcodeStart + 8)
#define CommentStart  (OperandStart + 16)
#define MaxFields     6

/*---------------------------------------------------------------------------*/

static void PutCh(char ch)
{
  if (ch != 0) newLine[linePosition++] = ch;
} /* End PutCh */

/*---------------------------------------------------------------------------*/

static void Tab(CARDINAL position)
{
  while (linePosition < position) PutCh(Space);
} /* End Tab */

/*---------------------------------------------------------------------------*/

void ListLine(void)
{
  if (printState && ((1 << ListPC) & listStatus)) PrintLine();
} /* End ListLine */

/*---------------------------------------------------------------------------*/

static void WriteComment(CARDINAL *i)
{
  CARDINAL tempPosition,
           length;
  if (currentLinePointer[*i] != CommentSymbol)
   {

    /*This is the silly line format case*/
    while (currentLinePointer[*i] != CR) PutCh(currentLinePointer[(*i)++]);
    PutLine();
  } else {
    if (linePosition <= TextStart) tempPosition = TextStart;
    else {
      tempPosition = CommentStart;
      PutCh(Space);
    }; /* if */
    Tab(tempPosition);
    if (linePosition > CommentStart) {
      length = 0;
      while (currentLinePointer[*i + length] != CR) length++;
      if ((linePosition + length > maxCols) &&
        (tempPosition + length <= maxCols)) {
        PutLine();
        Tab(CommentStart);
      }; /* if */
    }; /* if */
    do {
      while (linePosition < maxCols) {
        if (currentLinePointer[*i] == CR) break;
        PutCh(currentLinePointer[(*i)++]);
      }; /* while */
      if (linePosition < maxCols) break;
        /* The previous break was meant to leave the loop */
      PutLine();
      Tab(tempPosition);
    } while (1);
  }; /* if */
} /* End WriteComment */

/*---------------------------------------------------------------------------*/

static void WriteField(CARDINAL position, BOOLEAN allowSpace, 
                       CARDINAL *i)
{
  CARDINAL spaces;
  BOOLEAN  quoted = FALSE;
  /*Get to right position*/
  Tab(position);
  while ((!quoted && (!TermCheck(currentLinePointer[*i]) ||
    (allowSpace && (currentLinePointer[*i] == Space)))) ||
    (quoted && (currentLinePointer[*i] != CR))) {
    if (currentLinePointer[*i] == Space) {
      spaces = 0;
      while (currentLinePointer[*i + spaces] == Space) spaces++;

      if ((currentLinePointer[*i + spaces] == CR) || (!quoted && (currentLinePointer[*i + spaces] == CommentSymbol)))
       {
        i += spaces;
        return;
      }; /* if */
      do PutCh(currentLinePointer[(*i)++]);
      while (currentLinePointer[*i] == Space);
    } else {
      PutCh(currentLinePointer[*i]);
      if (currentLinePointer[*i] == Quotes) quoted = !quoted;
      (*i)++;
    }; /* if */
  }; /* while */
  while (currentLinePointer[*i] == Space) (*i)++;
} /* End WriteField */

/*---------------------------------------------------------------------------*/

void PrintLine(void)
{
  CARDINAL i = 0;
  if (!linePrinted) {
    if (TextStart >= maxCols) {
      /*This is the silly line format case*/
      if (linePosition > TextStart) PutLine();
      else Tab(TextStart);
      while (currentLinePointer[i] != CR) PutCh(currentLinePointer[i++]);
    } else {
      /*This is the sensible case*/
      if ((linePosition > TextStart) && (currentLinePointer[i] != Space))
        PutLine();
      do {

        if ((currentLinePointer[i] == CommentSymbol) || (OpcodeStart >= maxCols))
         break;
        /*Write out the label*/
        WriteField(TextStart, FALSE, &i);

        if ((currentLinePointer[i] == CR) || (currentLinePointer[i] == CommentSymbol) || (OperandStart >= maxCols))
         break ;

        /*We have an opcode*/
        if (linePosition >= OpcodeStart) PutLine();
        WriteField(OpcodeStart, FALSE, &i);

        if ((currentLinePointer[i] == CR) || (currentLinePointer[i] == CommentSymbol) || (CommentStart >= maxCols))
         break ;

        /*We have an operand*/
        if (linePosition >= OperandStart) PutLine();
        WriteField(OperandStart, TRUE, &i);
      } while (1);
      if (currentLinePointer[i] != CR) WriteComment(&i);
    }; /* if */
    PutLine();
  }; /* if */
  linePrinted = TRUE;
  linePosition = 0;
  if (PollEscape()) abortFlag = TRUE;
} /* End PrintLine */

/*---------------------------------------------------------------------------*/

static void PutChs(char *chs)
{
  CARDINAL i,
           j = strlen(chs);
  for (i = 0; i < j; i++) PutCh(chs[i]);
} /* End PutChs */

/*---------------------------------------------------------------------------*/

void ListLineNumber(void)
{
  if (printState && ((1 << ListPC) & listStatus)) PrintLineNumber();
} /* End ListLineNumber */

/*---------------------------------------------------------------------------*/

void PrintLineNumber(void)
{
  char     digits[7];
  int      i;
  CARDINAL j;
  BOOLEAN  suppress = FALSE;
  j = lineNumber;
  for (i = 5; i >= 0; i--) {
    digits[i] = (suppress) ? Space : '0' + (j % 10);
    j = j / 10;
    suppress = j == 0;
  }; /* for */
  if (linePosition > 0) PutLine();
  digits[6] = 0;
  PutChs(digits);
  PutCh(Space);
} /* End PrintLineNumber */

/*---------------------------------------------------------------------------*/

static void PutHexCh(Hex hex)
{
  PutCh((hex >= 10) ? hex - 10 + 'A' : hex + '0');
} /* End PutHexCh */

/*---------------------------------------------------------------------------*/

void ListRegValue(CARDINAL reg)
{
  if (((1 << ListPC) & listStatus) && printState) {
    PutChs("       ");
    PutHexCh(reg);
    PutCh(Space);
  }; /* if */
} /* End ListRegValue */

/*---------------------------------------------------------------------------*/

void ListAddress(void)
{
  if (printState && ((1 << ListPC) & listStatus)) PrintAddress();
} /* End ListAddress */

/*---------------------------------------------------------------------------*/

static void PutHexCardinal(CARDINAL w)
{
  int i;
  Hex digits[8];

  for (i = 0; i <= 7; i++) {
    digits[i] = w % 0x10;
    w = w / 0x10;
  }; /* for */
  for (i = 7; i >= 0; i--) PutHexCh(digits[i]);
} /* End PutHexCardinal */

/*---------------------------------------------------------------------------*/

void PrintAddress(void)
{
  Tab(PCStart);
  PutHexCardinal(programCounter);
  PutCh(Space);
} /* End PrintAddress */

/*---------------------------------------------------------------------------*/

void ListWordValue(CARDINAL w)
{
  if (printState && ((1 << ListPC) & listStatus)) {
    if (linePosition >= TextStart) PutLine();
    Tab(CodeStart);
    PutHexCardinal(w);
    PutCh(Space);
  }; /* if */
} /* End ListWordValue */

/*---------------------------------------------------------------------------*/

void ListByteValue(char b)
{
  if (printState && ((1 << ListPC) & listStatus)) {
    if (linePosition >= TextStart) PutLine();
    Tab(CodeStart);
    PutHexCh(b / 0x10);
    PutHexCh(b % 0x10);
    PutCh(Space);
  }; /* if */
} /* End ListByteValue */

/*---------------------------------------------------------------------------*/

void ListBoolValue(BOOLEAN b)
{
  if (printState && ((1 << ListPC) & listStatus)) {
    PutChs((b) ? "TRUE " : "FALSE");
    PutChs("    ");
  }; /* if */
} /* End ListBoolValue */

/*---------------------------------------------------------------------------*/
/* GSTrans character output */
static void EscapeWriteCh(char ch)
{
 if ((ch >= Space) && (ch < Del))
  PutCh(ch) ;
 else
  if (ch == Del)
   PutChs("|?") ;
  else
   if (ch > Del)
    {
     PutChs("|!") ;
     EscapeWriteCh(ch - 0x80) ;
    }
   else
    {
     PutCh('|') ;
     PutCh(ch + 0x40) ;
    }
} /* End EscapeWriteCh */

/*---------------------------------------------------------------------------*/

void ListStringValue(Name string)
{
 CARDINAL i ;

 if (printState && ((1 << ListPC) & listStatus))
  {
   for (i = 1; (i <= string.length); i++)
    {
     if (linePosition >= (MaxLineLength - 4))
      PutLine() ;
     Tab(CodeStart) ;
     EscapeWriteCh(string.key[i - 1]) ;
    }
  }
} /* End ListStringValue */

/*---------------------------------------------------------------------------*/

void InitLineList(void)
{
  linePosition = 0;
} /* End InitLineList */

/*---------------------------------------------------------------------------*/

static void SetOption(char *line, BOOLEAN *option)
{
  CARDINAL index,
           value;
  Name     optionName;

  if (!initialised) {
    CopyName(0, "OFF", options);
    CopyName(1, "ON", options);
    CopyName(2, "HELP", options);
    CopyName(3, "QUIT", options);
    initialised = TRUE;
  }; /* if */
  do
   {
    index = 0 ;
    if (!SymbolTest(line,&index,&optionName))
     {
      WriteChs("Bad option name\\N") ;
      newLine[0] = CR ;
      return ;
     }

    if (NameLookup(options, optionName, TRUE, &value, NumberOfOptions)) {
      switch (value) {
        case 0:
        case 1:
        *option = value != 0;
        line += index; /* Something odd here */
        return;

        case 2:
        WriteChs("Options available:\\NOn\\NOff\\NHelp\\NQuit\\N");
        break;

        case 3:
        return;
      }; /* case */
    } else {
      WriteChs("Unknown option\\N");
      newLine[0] = CR;
      return;
    }; /* if */
    *line = CR;
  } while (1);
} /* End SetOption */

/*---------------------------------------------------------------------------*/

void SetPrint(char *line)
{
  SetOption(line, &printState);
} /* End SetPrint */

/*---------------------------------------------------------------------------*/

void SetTerse(char *line)
{
  SetOption(line, &terseState);
} /* End SetTerse */

/*---------------------------------------------------------------------------*/

void SetXref(char *line)
{
  SetOption(line, &xrefOn);
} /* End SetXref */

/*---------------------------------------------------------------------------*/

void SetClose(char *line)
{
  SetOption(line, &closeExec);
} /* End SetClose */

/*---------------------------------------------------------------------------*/

void SetCache(char *line)
{
  SetOption(line, &caching);
} /* End SetCache */

/*---------------------------------------------------------------------------*/

void SetModule(char *line)
{
  SetOption(line, &module);
} /* End SetModule */

/*---------------------------------------------------------------------------*/

void PutLine(void)
{
  CARDINAL i;
  if (linePosition != 0) {
    PutCh(CR);
    newLine[linePosition] = 0;
    i = 0;
    while (newLine[i] != 0) WriteCh(newLine[i++]);
    linePosition = 0;
  }; /* if */
} /* End PutLine */

/*---------------------------------------------------------------------------*/

void CancelLineList(void)
{
  listStatus &= ~(1 << ListPC);
  linePosition = 0; /*Re-initialise line buffer*/
} /* End CancelLineList */

/*---------------------------------------------------------------------------*/
/* EOF listing/c */
