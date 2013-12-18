/* -> mactypes/h
 * Title:               Macro handling routines
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
#ifndef macrotypes_h
#define macrotypes_h

#include "getline.h"
#include "nametype.h"
#include "tables.h"

#define MaxMacro 65535

typedef struct {
  Name name,
       def;
} Parameter;/* record*/

typedef Parameter *ParameterBlock;

typedef struct {
  CARDINAL       number; /*the number of macro parameters*/
  ParameterBlock parameterBlock; /*An open array of them*/
} MacroDefaults;/* record*/

typedef struct MacroNameEntry *MacroNamePointer;

typedef struct MacroNameEntry {
  Name             macroName;
  MacroDefaults    macroDefaults;
  Name             macroBody;/*This is just a string and a length*/
  CARDINAL         macroLine;/*The line number at which the macro was defined*/
  MacroNamePointer next;
} MacroNameEntry; /* record */

BOOLEAN MacroSubstitute(char **string, char *line);

void InitMacroTable(void);

void DefineMacro(void);

void IgnoreMacroDefinition(void);

void ExpandMacro(char *line, Name name);

void ExitMacro(void);

SymbolPointer DefineLocalA(Name name);

SymbolPointer DefineLocalL(Name name);

SymbolPointer DefineLocalS(Name name);

SymbolPointer LookupLocalA(Name name);

SymbolPointer LookupLocalL(Name name);

SymbolPointer LookupLocalS(Name name);

SymbolPointer LookupLocal(Name name);

#endif

/* End mactypes/h */
