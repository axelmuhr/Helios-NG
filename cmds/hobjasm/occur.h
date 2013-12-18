/* -> occur/h
 * Title:               Handle symbol cross-referencing
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef occurrences_h
#define occurrences_h

#include "tables.h"

void PrintSymbol(Name name) ;   /* display a symbols text */

void Results(void);
/*Print out the results of the cross-reference*/

void AddUse(SymbolPointer ptr);
/*Add a usage of a symbol to the reference chains*/

void AddDef(SymbolPointer ptr);
/*Add a definition to the reference chains*/

void PrintResults(SymbolPointer ptr);
/*Print the cross reference result for one symbol*/

#endif

/*---------------------------------------------------------------------------*/
/* End occur/h */
