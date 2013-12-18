/* -> formatio/h
 * Title:               Formatted listing output
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef formatio_h
#define formatio_h

#include "nametype.h"

#define MaxVal 255

void SetWidth(char *line) ;

void SetLength(char *line) ;

void PageModeOn(void) ;

void PageModeOff(void) ;

void PageThrow(void) ;

void WriteCh(char ch) ;

void WriteChs(char *chs) ;

void WriteInteger(int i) ;

void WriteCardinal(CARDINAL c) ;

void WriteHexCardinal(CARDINAL c) ;

void SetTitle(Name t) ;

void SetSubtitle(Name t) ;

extern char *currentLinePointer ;

extern CARDINAL maxRows ;
extern CARDINAL maxCols ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF formatio/h */
