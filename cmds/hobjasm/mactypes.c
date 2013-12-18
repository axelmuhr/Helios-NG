/* -> mactypes/c
 * Title:               MACRO manipulation
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#include "asm.h"
#include "conds.h"
#include "constant.h"
#include "errors.h"
#include "formatio.h"
#include "getline.h"
#include "globvars.h"
#include "initdir.h"
#include "listing.h"
#include "llabel.h"
#include "mactypes.h"
#include "nametype.h"
#include "occur.h"
#include "p1hand.h"
#include "store.h"
#include "tables.h"
#include "vars.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

#define MaxMacroLength 4095

typedef struct ActualMacroBlock *ActualMacro;

typedef struct ActualMacroBlock {
  MacroDefaults parameters;
  ActualMacro   prev,/*Where it was called from*/
                next;/*To aid with deallocation*/
  SymbolPointer vars;
} ActualMacroBlock; /* record */

MacroNamePointer macroTable = NULL;
ActualMacro      currentMacro = NULL,/*The current object*/
                 actualMacro = NULL; /*The bottom of the chain*/

/*---------------------------------------------------------------------------*/

static BOOLEAN CompareNames(Name name1, Name 
         name2, 
                            BOOLEAN *equal)
/*Returns true if name1 <= name2*/
{
  CARDINAL i;
  *equal = FALSE;
  for (i = 0; i < name2.length; i++) {
    if (i >= name1.length) return TRUE;
    if (name1.key[i] < name2.key[i]) return TRUE;
    else if (name1.key[i] > name2.key[i]) return FALSE;
  }; /* for */
  *equal = name1.length == name2.length;
  return *equal;
} /* End CompareNames */

/*---------------------------------------------------------------------------*/

BOOLEAN MacroSubstitute(char **string, char 
      *line)
/*
string is a pointer to the source,
and line is a buffer if needed for parameter substitution if needed
*/
{
  CARDINAL iIndex = 0,
           oIndex = 0,
           oldI,
           i,
           j;
  BOOLEAN  equal,
           dollarFound = FALSE;
  char     ch;
  Name     name;
  do {
    ch = (*string)[iIndex];
    if (ch == Dollar) {
      oldI = iIndex++; /* past $*/
      if ((*string)[iIndex] == Dollar) {
        dollarFound = TRUE;
        line[oIndex++] = Dollar;/*contract*/
        iIndex++;
        if (oIndex >= MaxLineLength) {
          Warning(SubstLong);
          UnwindToGet();
          exception = EndOfInput;
          return FALSE;
        }; /* if */
      } else if (SymbolTest(*string, &iIndex, &name)) {
        /*look for symbol in list of macro parameters*/
        i = 0;
        do {
          if ((i >= currentMacro->parameters.number) ||
            (CompareNames(currentMacro->parameters.parameterBlock[i].name, name, &equal) && equal)) break;
          i++;
        } while (1); /* loop */
        if (i < currentMacro->parameters.number) {
          /*Here we substitute*/
          Name *def = &currentMacro->parameters.parameterBlock[i].def;
          if (def->length > 0) {
            for (j = 0; j < def->length; j++) {
              line[oIndex++] = def->key[j];
              if (def->key[j] == Dollar) dollarFound = TRUE;
              if (oIndex >= MaxLineLength) {
                Warning(SubstLong);
                UnwindToGet();
                exception = EndOfInput;
                return FALSE;
              }; /* if */
            };/* for */
          }; /* if */
          /*allow terminating dot*/
          if ((*string)[iIndex] == Dot) iIndex++;
        } else {
          /*Not a string symbol*/
          dollarFound = TRUE;
          for (i = oldI; i < iIndex; i++) {
            line[oIndex++] = (*string)[i];
            if (oIndex >= MaxLineLength) {
              Warning(SubstLong);
              UnwindToGet();
              exception = EndOfInput;
              return FALSE;
            }; /* if */
          }; /* for */
        }; /* if */
      } else {
        /*No symbol found*/
        line[oIndex++] = Dollar;
        if (oIndex >= MaxLineLength) {
          Warning(SubstLong);
          UnwindToGet();
          exception = EndOfInput;
          return FALSE;
        }; /* if */
      }; /* if */
    } else {
      line[oIndex++] = ch;
      iIndex++;
      if (oIndex >= MaxLineLength-1) {
        Warning(SubstLong);
        UnwindToGet();
        exception = EndOfInput;
        return FALSE;
      }; /* if */
      if (ch == Bar) {
        do {
          ch = (*string)[iIndex];
          line[oIndex++] = ch;
          iIndex++;
          if (oIndex >= MaxLineLength-1) {
            Warning(SubstLong);
            UnwindToGet();
            exception = EndOfInput;
            return FALSE;
          }; /* if */
        } while ((ch != Bar) && (ch != CR));
        if (ch == CR) { i--; j--; }; /* if */ /* Something odd here */
      }; /* if */
    }; /* if */
  } while (ch != CR);
  pointerInFile = *string + iIndex;
  *string = line;
  currentLinePointer = line;
  return dollarFound;
} /* End MacroSubstitute */

/*---------------------------------------------------------------------------*/

static void ReleaseParameterBlock(MacroDefaults parm)
{
  CARDINAL i;
  for (i = 0; i < parm.number; i++) {
    /*
    Parameter name block points into the macro definition area
    so it's already been deallocated
    */
    if (parm.parameterBlock[i].def.key != NULL)
      free(parm.parameterBlock[i].def.key);
  }; /* for */
  if (parm.parameterBlock != NULL) free(parm.parameterBlock);
} /* End ReleaseParameterBlock */

/*---------------------------------------------------------------------------*/

void ExitMacro(void)
/*Remove current macro, and go back to previous*/
{
  ActualMacro ptr;
  MacroUp();
  ptr = currentMacro;
  currentMacro = ptr->prev;
  ReleaseParameterBlock(ptr->parameters);
  /* Release store allocated to local variables */
  hRemove(ptr->vars);
  free(ptr);
  if (macroLevel == 0) actualMacro = NULL;
} /* End ExitMacro */

/*---------------------------------------------------------------------------*/

void ExpandMacro(char *line, Name name)
{
  MacroNamePointer      macro = macroTable;
  ActualMacro           ptr;
  BOOLEAN               quoted,
                        badLine,
                        equal;
  CARDINAL              index,
                        oldIndex,
                        parms,
                        parmLen,
                        endParm,
                        i;
  StructureStackElement s;

  /* first see if macro exists */
  do {
    if ((macro == NULL) || !CompareNames(macro->macroName, name, &equal)) {
      Warning(UnkOpc);
      return;
    }; /* if */
    if (equal) break;
    macro = macro->next;
  } while (1); /* loop */
  
  /*Now handle actual parameter settings*/
  ptr = mymalloc(sizeof(*ptr));/*Set up a new block*/
  ptr->prev = currentMacro;
  ptr->next = NULL;
  ptr->vars = NULL;/*No local variables yet*/
  ptr->parameters.number = macro->macroDefaults.number;
  ptr->parameters.parameterBlock =
    mymalloc(ptr->parameters.number*
      sizeof(Parameter));
  if (currentMacro == NULL) actualMacro = ptr;
  else currentMacro->next = ptr;
  for (i = 0; i < ptr->parameters.number; i++) {
    /*Point the name part at the macro definition section*/
    ptr->parameters.parameterBlock[i].name =
      macro->macroDefaults.parameterBlock[i].name;
    /*And set up the values to be NULL*/
    ptr->parameters.parameterBlock[i].def.length = 0;
    ptr->parameters.parameterBlock[i].def.key = NULL;
  }; /* for */
  /*Now analyse the macro call statement*/
  index = 0;
  while (line[index] != Space) index++;
  if (index != 0) {
    /*Here we have a non-null label parameter*/
    ptr->parameters.parameterBlock[0].def.length = index;
    ptr->parameters.parameterBlock[0].def.key = mymalloc(index);
    memcpy(ptr->parameters.parameterBlock[0].def.key, line, index);
  }; /* if */
  while (line[index] == Space) index++;
  /*Pass the macro name*/
  equal = SymbolTest(line, &index, &name);
  parms = 1;
  if (!TermCheck(line[index])) {
    badLine = TRUE;
    Warning(BadMacroParms);
  } else badLine = FALSE;
  if (!AllComment(line, &index)) {
    do {
      if (parms >= macro->macroDefaults.number) {
        Warning(TooManyParms);
        badLine = TRUE;
      }; /* if */
      if (badLine) {
        /*Give back all the store*/
        ReleaseParameterBlock(ptr->parameters);
        free(ptr);
        if (currentMacro == NULL) actualMacro = NULL;
        else currentMacro->next = NULL;
        return;
      }; /* if */
      /*Ignore leading spaces*/
      while (line[index] == Space) index++;
      quoted = line[index] == Quotes;
      parmLen = 0;
      if (quoted) index++;
      oldIndex = index;
      do {
        if ((line[index] == CR) && quoted) {
          badLine = TRUE;
          break;
        }; /* if */
        if ((line[index] == CR) || ((!quoted) &&
          ((line[index] == CommentSymbol) || (line[index] == Comma)))) {
          endParm = index;
          /*Ignore trailing spaces*/
          while ((endParm > oldIndex) && (line[endParm - 1] == Space)) {
            endParm--;
            parmLen--;
          }; /* while */
          break;
        }; /* if */
        if ((line[index] == Quotes) && quoted) {
          index++;
          if (line[index] != Quotes) {
            if (!AllComment(line, &index) && (line[index] != Comma))
              badLine = TRUE;
            break;
          }; /* if */
        }; /* if */
        index++;
        parmLen++;
      } while (1); /* loop */
      if (badLine) Warning(BadMacroParms);
      if ((parmLen == 1) && (line[oldIndex] == Bar) && !quoted) {
        ptr->parameters.parameterBlock[parms].def.length = 
          macro->macroDefaults.parameterBlock[parms].def.length;
        if (ptr->parameters.parameterBlock[parms].def.length != 0) {
          ptr->parameters.parameterBlock[parms].def.key =
            mymalloc(ptr->parameters.parameterBlock[parms].def.length);
          memcpy(ptr->parameters.parameterBlock[parms].def.key,
            macro->macroDefaults.parameterBlock[parms].def.key,
            ptr->parameters.parameterBlock[parms].def.length);
        }; /* if */
      } else {
        ptr->parameters.parameterBlock[parms].def.length = parmLen;
        if (ptr->parameters.parameterBlock[parms].def.length != 0) {
          ptr->parameters.parameterBlock[parms].def.key =
            mymalloc(ptr->parameters.parameterBlock[parms].def.length);
          for (i = 0; i < ptr->parameters.parameterBlock[parms].def.length;
            i++) {
            if ((line[oldIndex] == Quotes) && quoted) oldIndex++;
            ptr->parameters.parameterBlock[parms].def.key[i] = line[oldIndex++];
          }; /* for */
        }; /* if */
      }; /* if */
      parms++;
      if (line[index] != Comma) break;
      index++;/*Past comma*/
    } while (1); /* loop */
  }; /* if */
  currentMacro = ptr;
  
  /*Now set up to read out of the current macro*/
  s.type = MacroSSET;
  s.u.macro.lineNumber = lineNumber;
  if ((fileReadMode == WholeFileLoad) || (macroLevel != 0)) {
    s.u.macro.inputMode = WholeFileLoad;
    s.u.macro.u.storePointer = pointerInFile;
  } else s.u.macro.inputMode = ByteStream;
  s.u.macro.state = listStatus;
  if (!((1 << ListMacExpPC) & listStatus)) nextListState &= ~ (1 << ListPC);
  if (!((1 << ListMacCallPC) & listStatus)) CancelLineList();
  s.u.macro.name = macro->macroName;/*Point into the table*/
  if (!Stack(s)) return; /* Caller will deal with exception */
  pointerInFile = macro->macroBody.key;/*Point to the text*/
  macroLevel++;
  lineNumber = macro->macroLine;
  MacroDown();
} /* End ExpandMacro */

/*---------------------------------------------------------------------------*/

static void GetDefault(char *line, CARDINAL *index, char *key, 
                       CARDINAL *length)
{
  BOOLEAN quoted;
  char    ch;

  *length = 0;
  ch = line[*index];
  if (ch == Quotes) {
    (*index)++;
    quoted = TRUE;
  } else quoted = FALSE;
  do {
    ch = line[*index];
    if (ch == CR) {
      if (quoted) {
        Warning(BadMacParmDef);
        exception = EndOfInput;
        return;
      }; /* if */
      return;
    }; /* if */
    if (quoted) {
      if (ch == Quotes) {
        (*index)++;
        if (line[*index] != Quotes) return;
      }; /* if */
    } else {
      if ((ch == CommentSymbol) || (ch == Comma)) return;
      if (ch == Quotes) {
        (*index)++;
        if (line[*index] != Quotes) {
          Warning(BadMacParmDef);
          exception = EndOfInput;
          return;
        }; /* if */
      }; /* if */
    }; /* if */
    key[(*length)++] = ch;
    (*index)++;
  } while (1); /* loop */
} /* End GetDefault */

/*---------------------------------------------------------------------------*/

void DefineMacro(void)
{
  Parameter        block[MaxLineLength+1];
  /*To hold the parameters while we analyse them*/
  char             tempMacro[MaxMacroLength+1],
  /*To hold the body while we're defining the macro */
                   tempDefault[MaxLineLength+1];
  /*An array to hold a temporary parameter default value*/
  Name             parm,
                   newName;
  char             *line;
  CARDINAL         macroLength,
                   numberOfParms,
                   i,
                   index;
  DirectiveNumber  directiveNumber;
  MacroNamePointer macro,
                   ptr;
  BOOLEAN          bool,
                   par0Exists;
  /* first handle prototype statement */
  ListLine();
  bool = GetLine(&line);
  if (exception == EndOfInput) return;
  InitLineList();
  ListLineNumber();
  ListAddress();
  index = 0;
  if (*line == Dollar) {
    par0Exists = TRUE;
    index++;
    if (!SymbolTest(line, &index, &parm)) {
      Warning(IllLabParm);
      exception = EndOfInput;
      return;
    };
  } else par0Exists = FALSE;
  if (line[index] != Space) {
    Warning(IllLabParm);
    exception = EndOfInput;
    return;
  }; /* if */
  while (line[index] == Space) index++;
  if (!SymbolTest(line, &index, &newName)) {
    WarningReport("Bad MACRO name");
    exception = EndOfInput;
    return;
  }; /* if */
  
  /*Now look up the correct position in the table for the name given*/
  ptr = macroTable;
  macro = ptr;
  if (ptr == NULL) {
    /*the table is empty*/
    macroTable = mymalloc(sizeof(*macroTable));
    ptr = macroTable;
    macro = macroTable;
    ptr->next = NULL;
  } else if (CompareNames(newName, ptr->macroName, &bool)) {
    if (bool) {
      WarningReport("MACRO already exists");
      exception = EndOfInput;
      return;
    }; /* if */
    /*new name goes at table start*/
    macroTable = mymalloc(sizeof(*macroTable));
    macroTable->next = ptr;
    macro = macroTable;
  } else {
    do {
      ptr = macro->next;
      if (ptr == NULL) break;
      if (CompareNames(newName, ptr->macroName, &bool)) {
        if (bool) {
          /*The name is the same as an already existing one*/
          WarningReport("MACRO already exists");
          exception = EndOfInput;
          return;
        }; /* if */
        break;
      }; /* if */
      macro = ptr;
    } while (1); /* loop */
    macro->next = mymalloc(sizeof(*macro));
    macro = macro->next;
    macro->next = ptr;
  }; /* if */
  macro->macroLine = lineNumber;
  /*Now macro points to the new element*/
  macro->macroName.length = newName.length;
  macro->macroName.key = mymalloc(newName.length);
  memcpy(macro->macroName.key, newName.key, newName.length);
  
  /*Ensure rest of record is consistent in case we fault*/
  macro->macroBody.key = NULL;
  macro->macroDefaults.parameterBlock = NULL;
  macro->macroDefaults.number = 0;
  
  /*Now handle parameters and defaults*/
  numberOfParms = 1;
  block[0].def.length = 0;
  block[0].def.key = NULL;
  if (par0Exists) block[0].name = parm;
  else {
    block[0].name.length = 0;
    block[0].name.key = NULL;
  }; /* if */
  
  if (!AllComment(line, &index)) {
    do {
      if (line[index] != Dollar) {
        WarningReport("Illegal parameter start in MACRO prototype");
        exception = EndOfInput;
        return;
      }; /* if */
      index++;
      if (!SymbolTest(line, &index, &parm)) {
        WarningReport("Illegal parameter in MACRO prototype");
        exception = EndOfInput;
        return;
      }; /* if */
      block[numberOfParms].name = parm;
      while (line[index] == Space) index++;
      if (line[index] == Equals) {
        index++;
        /*Now calculate length of default value*/
        block[numberOfParms].def.key = line + index;
        GetDefault(line, &index, tempDefault,
          &block[numberOfParms].def.length);
        if (exception) return;
      } else block[numberOfParms].def.length = 0;
      numberOfParms++;
      if (AllComment(line, &index)) break;
      if (line[index] != Comma) {
        WarningReport("Invalid parameter separator in MACRO prototype");
        exception = EndOfInput;
        return;
      }; /* if */
      index++;
      /*Past comma */
      while (line[index] == Space) index++;
    } while (1); /* loop */
  }; /* if */
  
  /*Now build the genuine macro parameter and default table*/
  macro->macroDefaults.parameterBlock =
    mymalloc(numberOfParms*sizeof(Parameter));
  macro->macroDefaults.number = numberOfParms;
  for (i = 0; i < numberOfParms; i++) {
    macro->macroDefaults.parameterBlock[i].name = block[i].name;
    if (macro->macroDefaults.parameterBlock[i].name.key != NULL) {
      macro->macroDefaults.parameterBlock[i].name.key =
        mymalloc(macro->macroDefaults.parameterBlock[i].name.length);
      memcpy(macro->macroDefaults.parameterBlock[i].name.key,
        block[i].name.key, block[i].name.length);
    }; /* if */
    if (block[i].def.length == 0) {
      macro->macroDefaults.parameterBlock[i].def.key = NULL;
      macro->macroDefaults.parameterBlock[i].def.length = 0;
    } else {
      index = 0;
      macro->macroDefaults.parameterBlock[i].def.key =
        mymalloc(block[i].def.length);
      GetDefault(block[i].def.key, &index,
        macro->macroDefaults.parameterBlock[i].def.key,
        &macro->macroDefaults.parameterBlock[i].def.length);
      if (exception) return;
    }; /* if */
  }; /* for */
  
  /* now do macro body */
  inMacroDef = TRUE;
  macroLength = 1;
  macro->macroBody.length = 0;
  macro->macroBody.key = NULL;
  do {
    ListLine();
    bool = GetLine(&line);
    if (exception == EndOfInput) {
      exception = FileNotFound;
      return;
    };
    InitLineList();
    ListLineNumber();
    ListAddress();
    index = 0;
    while (line[index] != CR) {
      tempMacro[macroLength-1] = line[index++];
      macroLength++;
      if (macroLength >= MaxMacroLength) {
        WarningReport("MACRO definition too big");
        exception = EndOfInput;
        return;
      }; /* if */
    }; /* while */
    tempMacro[(macroLength++)-1] = CR;
    index = 0;
    if (!AllComment(line, &index) && (index != 0) &&
      DirTest(line, &index, &parm) &&
      NameDirective(&directiveNumber, parm) &&
      AllComment(line, &index)) {
      if (directiveNumber == TMEND) break;
        /* This is the end of the macro definition */
      if (directiveNumber == TMACRO) {
        WarningReport("MACRO definitions cannot be nested");
        exception = EndOfInput;
        return;
      }; /* if */
    };
  } while (1); /* loop */
  if (!errorFound) {
    macro->macroBody.length = macroLength+1;
    macro->macroBody.key = mymalloc(macro->macroBody.length);
    memcpy(macro->macroBody.key, tempMacro, macroLength);
    macro->macroBody.key[macroLength] = EOFChar;/*Ensure well terminated*/
  }; /* if */
  inMacroDef = FALSE;
} /* End DefineMacro */

/*---------------------------------------------------------------------------*/

void IgnoreMacroDefinition(void)
{
  char           *line;
  CARDINAL        index;
  DirectiveNumber directiveNumber;
  Name            name;

  /* first handle prototype statement */
  ListLine();
  (void) GetLine(&line);
  if (exception == EndOfInput) return;
  InitLineList();
  ListLineNumber();
  ListAddress();
  /* now do macro body */
  inMacroDef = TRUE;
  do {
    ListLine();
    (void) GetLine(&line);
    if (exception == EndOfInput) {
      exception = FileNotFound;
      return;
    };
    InitLineList();
    ListLineNumber();
    ListAddress();
    index = 0;
    if (!AllComment(line, &index) && (index != 0) &&
      DirTest(line, &index, &name) &&
      NameDirective(&directiveNumber, name) &&
      AllComment(line, &index)) {
      if (directiveNumber == TMEND) break;
        /*This is the end of the macro definition */
      if (directiveNumber == TMACRO) {
        WarningReport("MACRO definitions cannot be nested");
        exception = EndOfInput;
        return;
      }; /* if */
    };
  } while (1); /* loop */
  inMacroDef = FALSE;
} /* End IgnoreMacroDefinition */

/*---------------------------------------------------------------------------*/

void InitMacroTable(void)
{
  CARDINAL         i;
  MacroNamePointer ptr = macroTable,
                   next;
  ActualMacro      actMacro,
                   actNext;
  SymbolPointer    locVarPtr,
                   locVarNext;

  /*First initialise macro table*/
  while (ptr != NULL) {
    if (ptr->macroName.key != NULL) free(ptr->macroName.key);
    if (ptr->macroBody.key != NULL) free(ptr->macroBody.key);
    for (i = 0; i < ptr->macroDefaults.number; i++) {
      if (ptr->macroDefaults.parameterBlock[i].name.key != NULL)
        free(ptr->macroDefaults.parameterBlock[i].name.key);
    }; /* for */
    ReleaseParameterBlock(ptr->macroDefaults);
    next = ptr->next;
    free(ptr);
    ptr = next;
  }; /* while */
  macroTable = NULL;
  
  /*Now deallocate any existing macros*/
  actMacro = actualMacro;
  while (actMacro != NULL) {
    actNext = actMacro->next;
    ReleaseParameterBlock(actMacro->parameters);
    locVarPtr = actMacro->vars;
    while (locVarPtr != NULL) {
      locVarNext = locVarPtr->link;
      if ((locVarPtr->u.s.vst == StringVST) &&
        (locVarPtr->value.ptr->key != NULL)) {
        free(locVarPtr->value.ptr->key);
        free(locVarPtr->value.ptr);
      }; /* if */
      free(locVarPtr->key.key);
      free(locVarPtr);
      locVarPtr = locVarNext;
    }; /* while */
    free(actMacro);
    actMacro = actNext;
  }; /* while */
  currentMacro = NULL;
  actualMacro = NULL;
} /* End InitMacroTable */

/*---------------------------------------------------------------------------*/

static SymbolPointer Insert(Name name, CARDINAL status)
{
/* Assumes name not already in table */
  SymbolPointer locVarPtr;

  if (currentMacro == NULL) AssemblerError("No current macro for insert");
  locVarPtr = currentMacro->vars;
  if (locVarPtr == NULL) {
    currentMacro->vars = mymalloc(sizeof(*currentMacro->vars));
    locVarPtr = currentMacro->vars;
  } else {
    do {
      if (locVarPtr->link == NULL) {
        locVarPtr->link = mymalloc(sizeof(*locVarPtr->link));
        locVarPtr = locVarPtr->link;
        break;
      }; /* if */
      locVarPtr = locVarPtr->link;
    } while (1); /* loop */
  }; /* if */
  locVarPtr->key.length = name.length;
  locVarPtr->key.key = mymalloc(name.length);
  memcpy(locVarPtr->key.key, name.key, name.length);
  locVarPtr->u.status = status;
  locVarPtr->length = 0;
  locVarPtr->link = NULL;
  locVarPtr->defPtr = NULL;                 /*No cross reference stuff yet*/
  locVarPtr->usePtr = NULL;
  AddDef(locVarPtr);
  return locVarPtr;
} /* End Insert */

/*---------------------------------------------------------------------------*/

SymbolPointer DefineLocalA(Name name)
{
  SymbolPointer result;

  /*Lookup and enter if not there*/
  result = LookupLocal(name);
  if (result == NULL) {
    result = Insert(name, 0);
    result->u.s.sdt = VariableSDT;
    result->u.s.vst = ArithmeticVST;
    result->u.s.sds = DefinedSDS;
  } else if (result->u.s.vst != ArithmeticVST) return NULL;
  else AddUse(result);
  result->value.card = 0;
  return result;
} /* End DefineLocalA */

/*---------------------------------------------------------------------------*/

SymbolPointer DefineLocalL(Name name)
{
  SymbolPointer result;

  /*Lookup and enter if not there*/
  result = LookupLocal(name);
  if (result == NULL) {
    result = Insert(name, 0);
    result->u.s.sdt = VariableSDT;
    result->u.s.vst = LogicalVST;
    result->u.s.sds = DefinedSDS;
  } else if (result->u.s.vst != LogicalVST) return NULL;
  else AddUse(result);
  result->value.bool = FALSE;
  return result;
} /* End DefineLocalL */

/*---------------------------------------------------------------------------*/

SymbolPointer DefineLocalS(Name name)
{
  SymbolPointer result;

  /*Lookup and enter if not there*/
  result = LookupLocal(name);
  if (result == NULL) {
    result = Insert(name, 0);
    result->u.s.sdt = VariableSDT;
    result->u.s.vst = StringVST;
    result->u.s.sds = DefinedSDS;
    result->value.ptr = mymalloc(sizeof(*result->value.ptr));
    result->value.ptr->length = 0;
    result->value.ptr->key = NULL;
    result->value.ptr->maxLength = 0;
    return result;
  };
  if (result->u.s.vst != StringVST) return NULL;
  else AddUse(result);
  result->value.ptr->length = 0;
  return result;
} /* End DefineLocalS */

/*---------------------------------------------------------------------------*/

SymbolPointer LookupLocal(Name name)
{
  SymbolPointer locVarPtr;
  BOOLEAN       equal;

  if (currentMacro == NULL) return NULL;
  locVarPtr = currentMacro->vars;
  do {
    if (locVarPtr == NULL) return NULL;
    if (CompareNames(name, locVarPtr->key, &equal) && equal) {
      AddUse(locVarPtr);
      return locVarPtr;
    }; /* if */
    locVarPtr = locVarPtr->link;
  } while (1); /* loop */
} /* End LookupLocal */

/*---------------------------------------------------------------------------*/

SymbolPointer LookupLocalA(Name name)
{
  SymbolPointer result = LookupLocal(name);
  if ((result != NULL) && (result->u.s.sdt == VariableSDT) &&
    (result->u.s.vst == ArithmeticVST)) return result;
  /*Symbol was there but wrong type*/
  return NULL;
} /* End LookupLocalA */

/*---------------------------------------------------------------------------*/

SymbolPointer LookupLocalL(Name name)
{
  SymbolPointer result = LookupLocal(name);
  if ((result != NULL) && (result->u.s.sdt == VariableSDT) &&
    (result->u.s.vst == LogicalVST)) return result;
  /*Symbol was there but wrong type*/
  return NULL;
} /* End LookupLocalL */

/*---------------------------------------------------------------------------*/

SymbolPointer LookupLocalS(Name name)
{
  SymbolPointer result = LookupLocal(name);
  if ((result != NULL) && (result->u.s.sdt == VariableSDT) &&
    (result->u.s.vst == StringVST)) return result;
  /*Symbol was there but wrong type*/
  return NULL;
} /* End LookupLocalS */

/*---------------------------------------------------------------------------*/
/* EOF mactypes/c */
