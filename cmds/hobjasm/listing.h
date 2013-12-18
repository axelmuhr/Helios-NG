/* -> listing/h
 * Title:               Pretty listing control
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef listing_h
#define listing_h

#include "nametype.h"

void ListStringValue(Name string);

void ListRegValue(CARDINAL reg);

void ListBoolValue(BOOLEAN b);

void PrintLineNumber(void);

void ListLineNumber(void);

void ListLine(void);

void PrintLine(void);

void ListWordValue(CARDINAL w);

void ListByteValue(char b);

void InitLineList(void);

void SetPrint(char *line);

void SetTerse(char *line);

void SetXref(char *line);

void SetClose(char *line);

void SetStamp(char *line);

void SetCache(char *line);

void SetModule(char *line);

void ListAddress(void);

void PrintAddress(void);

void CancelLineList(void);

void PutLine(void);

#endif

/* End listing/h */
