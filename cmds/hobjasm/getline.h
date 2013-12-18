/* -> getline/h
 * Title:               The general line generation routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef getline_h
#define getline_h

#include "constant.h"

typedef enum {WholeFileLoad,ByteStream} FileReadMode ;

#define MaxLineLength 255

extern char *pointerInFile ;
extern char *oldPointerInFile ;

extern CARDINAL lineNumber ;

extern FileReadMode fileReadMode ;

BOOLEAN GetLine(char **string) ;

#endif

/*---------------------------------------------------------------------------*/
/* EOF getline/h */
