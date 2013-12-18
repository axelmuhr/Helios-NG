/* -> asm/h
 * Title:               Top level assembly routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef asm_h
#define asm_h

#include "constant.h"
#include "getline.h"

#include <stdio.h>

extern char currentFileName[MaxLineLength+1],
            codeFileName[MaxLineLength+1];

extern char       *stringTable;

extern BOOLEAN inFile;
extern char   *fileStore;
extern FILE   *codeStream,
              *inputStream;
extern BOOLEAN inputStreamExists;

void Asm(char *fileName);
/*Run an assembly starting at file fileName*/

BOOLEAN P1File(char *currentFileName, BOOLEAN stream);
/*Do the first pass, returning TRUE if END directive encountered*/

BOOLEAN P2File(char *currentFileName, BOOLEAN stream);
/*Do the second pass, returning TRUE if END directive encountered*/

void CopyFileName(char *pointer);
/* Get the new file name to be linked to and put it in currentFileName */

void Init_Asm(void);

#endif

/* End asm/h */
