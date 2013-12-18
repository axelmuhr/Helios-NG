/* -> llabel/h
 * Title:               Local label handling
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
#ifndef locallabels_h
#define locallabels_h

#include "tables.h"

void InitLocalLabels(void);

void LabelDef(char *line, CARDINAL *index);
/*
Define a local label from within the text
index assumed pointing at a decimal digit
*/

CARDINAL LabelUse(char *line,CARDINAL *index,BOOLEAN *found,CARDINAL *area);
/*
Look up the value of a local label
index assumed pointing past the %
*/

void NewRoutine(SymbolPointer symbolPointer);
/*Define a new routine name*/

void MacroUp(void);
/*Add an end of macro token to the table*/

void MacroDown(void);
/*Add a start of macro token to the table*/

#endif

/* End llabel/h */
