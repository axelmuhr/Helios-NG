/* -> symbol/c
 * Title:               Symbol table printing
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#include "code.h"
#include "constant.h"
#include "formatio.h"
#include "getline.h"
#include "globvars.h"
#include "osdepend.h"
#include "nametype.h"
#include "occur.h"
#include "stats.h"
#include "store.h"
#include "symbol.h"
#include "tables.h"
#include "vars.h"
#include <stdlib.h>
#include <string.h>

typedef BOOLEAN Comparator(SymbolPointer,SymbolPointer) ;

typedef BOOLEAN AllowSymbol(Status) ;

typedef SymbolPointer *SymbolArray ;

CARDINAL size ;

static CARDINAL CountTable(SymbolPointer *symbolTable)
/*Count the total number of entries in a symbol table*/
{
  CARDINAL      index,
                count = 0;
  SymbolPointer symbolPointer;

  for (index = 0; index < MaxSymbol; index++) {
    symbolPointer = symbolTable[index];
    while (symbolPointer != NULL) {
      count++;
      symbolPointer = symbolPointer->link;
    }; /* while */
  }; /* while */
  return count;
} /* End CountTable */

static void SetCTitle(char *t)
{
  Name name;

  name.length = strlen(t);
  name.key = t;
  SetTitle(name);
} /* End SetCTitle */

static void SetupTable(SymbolArray array,AllowSymbol allows,CARDINAL *length,BOOLEAN area,CARDINAL unused)
/*Set up the array of symbols to be sorted*/
{
  CARDINAL i;
  SymbolPointer symbolPointer;

  *length = 0;
  for (i = 0; i < MaxSymbol; i++) {
    symbolPointer = symbolTable[i];
    while (symbolPointer != NULL) {
      if (allows(symbolPointer->u) && (!area)) {
        array[(*length)++] = symbolPointer;
      };
      symbolPointer = symbolPointer->link;
    }; /* while */
  };
 return ;
 unused = unused ;
} /* End SetupTable */

static void Sort(SymbolArray data, CARDINAL length, 
                 Comparator precedes)
{
  CARDINAL      step,
                i,
                j,
                k;
  SymbolPointer x;

  if (length == 0) { return; };
  step = length;
  do {
    step = (step + 2) / 3;
    for (i = step+1; i <= length; i++) {
      j = i-1;
      do {
        if (j <= step-1) { break; };
        k = j - step;
        if (precedes (data[k], data[j])) { break; };
        x = data[j];
        data[j] = data[k];
        data[k] = x;
        j = k;
      } while (1);
    };
  } while (step > 1);
} /* End Sort */

static void SetCSubtitle(char *t)
{
  Name name;

  name.length = strlen(t);
  name.key = t;
  SetSubtitle(name);
} /* End SetCSubtitle */

static void PrintTable(SymbolArray table, CARDINAL length, 
                       char *subtitle)
{
  CARDINAL i,
           j,
           currHPos,
           colsPerSymbol,
           valueWidth,
           symHStart;

  #define SymWidth 23
  PageModeOn();
  SetCSubtitle(subtitle);
  currHPos = 0;
  i = table[0]->u.s.sdt;
  if (i == FixedSDT) {
    valueWidth = (table[0]->u.s.fst == RegisterRelativeFST) ? 11 : 9;
  } else 
  if (i == ExternalSDT) {
    valueWidth = 0;
  } else
  {
    valueWidth = 3;/*Register*/
  }; /* if */
  colsPerSymbol = SymWidth + valueWidth + 2;
  /*
  One for the pssible trailing space,
  the other for the possible leading star
  */
  for (i = 0; i <= length; i++) {
    if (((maxCols - currHPos < table[i]->key.length + valueWidth) ||
      (maxCols - currHPos < colsPerSymbol - 1)) && 
      (colsPerSymbol-1 <= maxCols) &&
      (table[i]->key.length + valueWidth <= maxCols)) {
      WriteChs("\\N");
      currHPos = 0;
    }; /* if */
    symHStart = currHPos;
    if (table[i]->u.s.srs == UnreferencedSRS) {
      WriteCh('*');
    } else {
      WriteCh(Space);
    }; /* if */
    for (j = 0; j < table[i]->key.length; j++) WriteCh(table[i]->key.key[j]);
    currHPos += table[i]->key.length + 1;
    if (valueWidth != 0) {
      WriteCh(Space);
      currHPos++;
      while (currHPos > symHStart + SymWidth) {
        symHStart += colsPerSymbol;
      }; /* while */
      while (currHPos < symHStart + SymWidth + 2) {
        currHPos++;
        WriteCh(Space);
      }; /* while */
      if (valueWidth == 9) {
        WriteHexCardinal(table[i]->value.card);
      } else if (valueWidth == 11) {
        j = table[i]->u.s.fsr;
        if (j >= 10) {
          WriteCh(j - 10 + 'A');
        } else {
          WriteCh(j + '0');
        }; /* if */
        WriteCh(',');
        WriteHexCardinal(table[i]->value.card);
      } else {
        j = table[i]->value.card;
        if (j >= 10) { WriteCh('1'); } else { WriteCh(Space); }; /* if */
        WriteCh(j % 10 + '0');
      }; /* if */
      currHPos += valueWidth-1;
    } else {
      while (currHPos > symHStart) {
        symHStart += colsPerSymbol;
      }; /* while */
      if (symHStart + colsPerSymbol - 1 > maxCols) {
        /*No more symbols on this line*/
        if (currHPos < maxCols) { currHPos = symHStart; };
      } else {
        while (currHPos <= symHStart - 1) {
          WriteCh(Space);
          currHPos++;
        }; /* while */
      }; /* if */
    }; /* if */
    if (currHPos >= maxCols) {
      if (currHPos > maxCols) { WriteChs("\\N"); }; /* if */
      currHPos = 0;
    } else {
      WriteCh(Space);
      currHPos++;
    }; /* if */
  }; /* for */
  if (currHPos != 0) { WriteChs("\\N"); }; /* if */
} /* End PrintTable */

static void SortTable(SymbolArray table,AllowSymbol allows,Comparator precedes,char *subtitle,BOOLEAN area)
{
  CARDINAL length;
  CARDINAL areaNum,
           total = (area) ? totalAreas : 1;
  for (areaNum = 1; areaNum <= total; areaNum++) {
    SetupTable(table, allows, &length, area, areaNum);
    if (length == 0) { continue; };
    Sort(table, length, precedes);
    PrintTable(table, length-1, subtitle);
    WriteCardinal(length);
    WriteChs(" symbol");
    if (length != 1) { WriteCh('s'); }; /* if */
    WriteChs("\\N");
  };
} /* End SortTable */

static BOOLEAN AllowAll(Status status)
{
 return(TRUE) ;
 status = status ;
}

static BOOLEAN AllowRelocatable(Status status)
{
  return (status.s.sdt == FixedSDT) && (status.s.fst == RelocatableFST);
} /* End AllowRelocatable */

static BOOLEAN AllowAbsolute(Status status)
{
  return (status.s.sdt == FixedSDT) && (status.s.fst == AbsoluteFST);
} /* End AllowAbsolute */

static BOOLEAN AllowRegisterRelative(Status status)
{
  return (status.s.sdt == FixedSDT) && (status.s.fst == RegisterRelativeFST);
} /* End AllowRegisterRelative */

static BOOLEAN AllowRegister(Status status)
{
  return (status.s.sdt == RegisterNameSDT);
} /* End AllowRegister */

static BOOLEAN AllowExternal(Status status)
{
  return (status.s.sdt == ExternalSDT);
} /* End AllowExternal */

/* Compare the names of two symbols */
static BOOLEAN AlphaCompare(SymbolPointer a,SymbolPointer b)
{
  CARDINAL i;
  Name     c = a->key,
           d = b->key;

  i = 0;
  do {
    if (i >= c.length) { return TRUE; };
    if ((i >= d.length) || (c.key[i] > d.key[i])) { return FALSE; };
    if (c.key[i] < d.key[i]) { return TRUE; };
    i++;
  } while (1);
} /* End AlphaCompare */

static BOOLEAN NumericCompare(SymbolPointer a, 
                              SymbolPointer 
              b)
{
  return a->value.card <= b->value.card;
} /* End NumericCompare */

/*---------------------------------------------------------------------------*/
/* Display the symbols we have encountered */

void SymbolDump(void)
{
 SymbolArray   tempTable ;
 SymbolPointer symbolPointer ;
 CARDINAL      loop ;
 CARDINAL      i ;

 size = CountTable(symbolTable) ;
 if (size == 0)
  WriteChs("No symbols in table\\N") ;

 tempTable = mymalloc(size * sizeof(SymbolPointer)) ;

 /* Alphabetic list */
 SetCTitle("Alphabetic symbol ordering") ;
 SortTable(tempTable,AllowRelocatable,AlphaCompare,"Relocatable symbols",TRUE) ;
 SortTable(tempTable,AllowAbsolute,AlphaCompare,"Absolute symbols",FALSE) ;
 SortTable(tempTable,AllowRegisterRelative,AlphaCompare,"Register relative symbols",FALSE) ;
 SortTable(tempTable,AllowRegister,AlphaCompare,"Register symbols",FALSE) ;
 SortTable(tempTable,AllowExternal,AlphaCompare,"External symbols",FALSE) ;

 /* Numeric list */
 SetCTitle("Numeric symbol ordering") ;
 SortTable(tempTable,AllowRelocatable,NumericCompare,"Relocatable symbols",TRUE) ;
 SortTable(tempTable,AllowAbsolute,NumericCompare,"Absolute symbols",FALSE) ;
 SortTable(tempTable,AllowRegisterRelative,NumericCompare,"Register relative symbols",FALSE) ;
 SortTable(tempTable,AllowRegister,NumericCompare,"Register symbols",FALSE) ;

 WriteCardinal(size) ;
 WriteChs(" symbol") ;
 if (size != 1)
  WriteCh('s') ;
 WriteChs(" in table\\N") ;

 /* Cross-Referenced list */
 SetCTitle("Cross-referenced symbol list") ;
 /* place all symbol pointers into the "tempTable" array */
 SortTable(tempTable,AllowAll,AlphaCompare,"XRef list",FALSE) ;
 WriteChs("\\N") ;
 /* now process the complete array */
 for (loop = 0; (loop < size); loop++)
  {
   WriteChs("\\N") ;
   symbolPointer = tempTable[loop] ;
   if ((symbolPointer == NULL) || (symbolPointer->u.s.sds != DefinedSDS))
    {
     if (symbolPointer->u.s.sds != DefinedSDS)
      {
       WriteChs("External (undefined) symbol: ") ;
       for (i = 0; (i < symbolPointer->key.length); i++)
        WriteCh(symbolPointer->key.key[i]) ;
       WriteChs("\\N") ;
      }
    }
   else
    {
     PrintSymbol(symbolPointer->key) ;
     WriteChs(" = ") ;
     if ((symbolPointer->u.s.sdt != VariableSDT) || (symbolPointer->u.s.vst == ArithmeticVST))
      WriteHexCardinal(symbolPointer->value.card) ;
     else
      if (symbolPointer->u.s.vst == LogicalVST)
       {
        if (symbolPointer->value.bool)
         WriteChs("TRUE") ;
        else
         WriteChs("FALSE") ;
       }
      else
       {
        for (i = 1; i <= symbolPointer->value.ptr->length; i++)
         WriteCh(symbolPointer->value.ptr->key[i-1]) ;
       } ; /* if */
     WriteChs("\\N") ;
     if (xrefOn && (symbolPointer->u.s.sdt == FixedSDT))
      PrintResults(symbolPointer) ;
    }
  }

 free(tempTable) ;
 PageModeOff() ;

} /* End SymbolDump */

/* End symbol/c */
