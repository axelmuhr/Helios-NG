/* -> llabel/c
 * Title:               Local label handling
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "constant.h"
#include "errors.h"
#include "globvars.h"
#include "llabel.h"
#include "nametype.h"
#include "asmvars.h"
#include "p1hand.h"
#include "store.h"
#include "tables.h"
#include "tokens.h"
#include "vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

typedef enum {
  Label, /*Just an ordinary label*/
  Rout,  /*A routine start*/
  Start, /*Start of table*/
  End,   /*End of table*/
  MacUp, /*Going up a macro level (towards source)*/
  MacDown/*Going down a macro level (deeper into macros)*/
  } LLType;

typedef struct LocalLabel *LLPointer;

typedef struct LocalLabel {
  LLType    type;
  CARDINAL  value, /*The PC value possibly associated with the local label*/
            key;   /*The label name*/
  CARDINAL  area;  /* The area defining the label, only relevant for as mode */
  LLPointer next,  /*Forward chain*/
            prev;  /*Backwards chain*/
} LocalLabel;

typedef enum {
  Back,
  Forwards,
  Both
  } SearchType;

typedef enum {
  Above,
  This,
  Any
  } MacroLevelType;

static BOOLEAN       initialised = FALSE;
LLPointer     lLPointer,/*The current position*/
              lLStart;  /*The start of the table*/
SymbolPointer routine;

/*---------------------------------------------------------------------------*/

void InitLocalLabels(void)
/*Initialise the table ready for an assembly*/
{
  LLPointer pointer,
            newPointer;
  if (pass == 1) {
    if (initialised) {
      pointer = lLStart;
      while (pointer != NULL) {
        newPointer = pointer->next;
        free(pointer);
        pointer = newPointer;
      }; /* while */
    };
    lLStart = mymalloc(sizeof(*lLStart));
    lLStart->type = Start;
    lLStart->prev = NULL;
    lLStart->next = mymalloc(sizeof(*lLStart));
    lLStart->next->type = End;
    lLStart->next->next = NULL;
    lLStart->next->prev = lLStart;
    initialised = TRUE;
  };
  lLPointer = lLStart->next;/*Pointing at the end of the table*/
  routine = NULL;
} /* End InitLocalLabels */

/*---------------------------------------------------------------------------*/

static BOOLEAN MacroLevelCheck(CARDINAL currentMacroLevel, 
                               MacroLevelType levelType)
{
  switch (levelType) {
    default:
    case Any:
    return TRUE;
    case This:
    return currentMacroLevel == macroLevel;
    case Above:
    return currentMacroLevel <= macroLevel;
  };
} /* End MacroLevelCheck */

/*---------------------------------------------------------------------------*/

static BOOLEAN SearchBack(CARDINAL key,CARDINAL *value,CARDINAL *area,MacroLevelType levelType)
{
  CARDINAL  currentMacroLevel;
  LLPointer pointer;

  currentMacroLevel = macroLevel;
  pointer = lLPointer->prev;
  do {
    switch (pointer->type) {
      case Label:
      if ((pointer->key == key) &&
        MacroLevelCheck(currentMacroLevel, levelType)) {
        *value = pointer->value;
        *area = pointer->area;
        return TRUE;
      };
      break;

      case Rout:
      case Start:
      case End:
      return FALSE;

      case MacUp:
      currentMacroLevel++;
      break;

      case MacDown:
      currentMacroLevel--;

    };
  pointer = pointer->prev;/*Back along the chain*/
  } while (1);
} /* End SearchBack */

/*---------------------------------------------------------------------------*/

static BOOLEAN SearchForwards(CARDINAL key,CARDINAL *value,CARDINAL *area,MacroLevelType levelType)
{
  CARDINAL  currentMacroLevel;
  LLPointer pointer;

  currentMacroLevel = macroLevel;
  pointer = lLPointer;
  do {
    switch (pointer->type) {
      case Label:
      if ((pointer->key == key) &&
        MacroLevelCheck(currentMacroLevel, levelType)) {
        *value = pointer->value;
        *area = pointer->area;
        return TRUE;
      };
      break;

      case Rout:
      case Start:
      case End:
      return FALSE;

      case MacUp:
      currentMacroLevel--;
      break;

      case MacDown:
      currentMacroLevel++;

    };
    pointer = pointer->next;/*Forwards along the chain*/
  } while (1);
} /* End SearchForwards */

/*---------------------------------------------------------------------------*/

static BOOLEAN CheckRoutineName(char *line, CARDINAL *index)
/*
Check the routine name if given
Returns TRUE if ok, FALSE if error reported
*/
{
  Name     name;
  if ((routine != NULL) && SymbolTest(line, index, &name)) {
    /*Non-null routine name exists so check name given*/
    if ((name.length != routine->key.length) ||
      (memcmp(name.key, routine->key.key, name.length) != 0)) {
      Warning(WrongRout);
      return FALSE;
    };
  };
  return TRUE;
} /* End CheckRoutineName */

/*---------------------------------------------------------------------------*/

CARDINAL LabelUse(char *line,CARDINAL *index,BOOLEAN *found,CARDINAL *area)
/*
 * Look up the value of a local label
 * index assumed pointing past the %
 */
{
  char           ch = toupper(line[*index]);
  CARDINAL       value,
                 key;
  SearchType     searchType;
  MacroLevelType macroLevelType;

  /*Now look for the search type*/
  searchType = Both;
  if (ch == 'F') {
    searchType = Forwards;
    ch = toupper(line[++(*index)]);
  } else if (ch == 'B') {
    searchType = Back;
    ch = toupper(line[++(*index)]);
  };

  /*Now look for macro level type*/
  macroLevelType = Above;
  if (ch == 'T') {
    macroLevelType = This;
    ch = line[++(*index)];
  } else if (ch == 'A') {
    macroLevelType = Any;
    ch = line[++(*index)];
  };

  /*Now look for the local label number*/
  if (!isdigit(ch)) {
    Warning(BadLocNum);
    return 0;
  };
  key = DecimalNumber(line, index);
  if (errorFound || !CheckRoutineName(line, index)) return 0;

  /*Now search for the defining occurrence*/
  switch (searchType) {
    case Both:
    *found = SearchBack(key,&value,area,macroLevelType) ||
      SearchForwards(key,&value,area,macroLevelType) ;
    break;

    case Back:
    *found = SearchBack(key,&value,area,macroLevelType) ;
    break;

    case Forwards:
    *found = SearchForwards(key,&value,area,macroLevelType) ;
    break;
  };
  return value;
} /* End LabelUse */

/*---------------------------------------------------------------------------*/

void AddEntry(LLType type)
{
  LLPointer temp  = lLPointer->prev;
  temp->next = mymalloc(sizeof(*temp)); /*Create a new element*/
  temp->next->prev = temp;            /*Insert its back link*/
  temp->next->next = lLPointer;       /*Insert its forward link*/
  lLPointer->prev = temp->next;       /*Point back to it from the end*/
  temp->next->type = type;
} /* End AddEntry */

/*---------------------------------------------------------------------------*/

void LabelDef(char *line, CARDINAL *index)
/*
Define a local label from within the text
index assumed pointing at a decimal digit
*/
{
  CARDINAL  name;
  LLPointer temp;

  /*First check the syntax*/
  name = DecimalNumber(line, index);
  if (errorFound || !CheckRoutineName(line, index)) return;
  if (!TermCheck(line[*index]))
   {
    Warning(SynAfterLocLab) ;
    return ;
   }
  else
   {
    /* Valid terminator before : implies ObjAsm style or error */
    if (line[*index] == AtSymbol)
     {
      Warning(BadLocNum);
      return;
     } ; /* if */
   } ;
  while (line[*index] == Space) (*index)++;

  /*Now handle according to pass*/
  if (pass == 1) {
    /*Define the label*/
    AddEntry(Label);
    temp = lLPointer->prev;   /*The new element*/
    temp->value = programCounter;/*The value of the local label*/
    temp->key = name;            /*The name of the local label*/
    temp->area = 1 ;
  } else
    /*Pass the definition in the table*/
    lLPointer = lLPointer->next;
} /* End LabelDef */

/*---------------------------------------------------------------------------*/

void NewRoutine(SymbolPointer symbolPointer)
/*Define a new routine name*/
{
  routine = symbolPointer;
  if (pass == 1)
    /*Remember new name*/

    /*Generate a new entry for it*/
    AddEntry(Rout);
  else lLPointer = lLPointer->next;
} /* End NewRoutine */

/*---------------------------------------------------------------------------*/

void MacroUp(void)
/*Add an end of macro token to the table*/
{
  if (pass == 1) AddEntry(MacUp);
  else lLPointer = lLPointer->next;
} /* End MacroUp */

/*---------------------------------------------------------------------------*/

void MacroDown(void)
/*Add a start of macro token to the table*/
{
  if (pass == 1) AddEntry(MacDown);
  else lLPointer = lLPointer->next;
} /* End MacroDown */

/*---------------------------------------------------------------------------*/
/* EOF llabel/c */
