/* -> vars/h
 * Title:               The string variable handling stuff
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef variables_h
#define variables_h

#include "nametype.h"

char *SubstituteString(char *string);
/*Substitute the string variables in an input line*/

/* Discover a symbol in line and return it in retSymbol */
BOOLEAN SymbolTest(char *string,CARDINAL *index,Name *retSymbol) ;

/* Discover a symbol in line and return it in retSymbol (non-destructively) */
BOOLEAN SymbolScan(char *string,CARDINAL index,Name *retSymbol) ;

BOOLEAN DirTest(char *line, CARDINAL *index, Name *name);
/* Like symbol test but less stringent */

extern BOOLEAN termin[256];

void Init_Variables(void);

#endif

/* End vars/h */
