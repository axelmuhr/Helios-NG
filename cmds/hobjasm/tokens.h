/* -> tokens/h
 * Title:               Expression tokens
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */
/*---------------------------------------------------------------------------*/

#ifndef tokens_h
#define tokens_h

#include "extypes.h"

/*---------------------------------------------------------------------------*/

/* Read a decimal number from the input line, returning the index past the
 * number and any terminating padding. Assumes that it is currently pointing
 * at a value decimal digit.
 */
CARDINAL DecimalNumber(char *line,CARDINAL *index) ;

/* Read a hexadecimal number from the input line, returning the index past the
 * number and terminating padding. Assumes that it is currently pointing at a
 * valid hexadecimal digit.
 */
CARDINAL HexNumber(char *line,CARDINAL *index) ;

/* Get a string from the input line. It assumes that we have passed the
 * opening "
 */
char *GetString(char *line,CARDINAL *index,CARDINAL *length) ;

void InitTokens(void) ;

/* Read a word type operator. ie. one delimited by colons */
Operator GetWordOperator(char *line,CARDINAL *index,EHeliosPatch *hDIR) ;

void Token(char *line,CARDINAL *index,EStackEntry *result,BOOLEAN *defined,EHeliosPatch *hDIR) ;

#endif

/*---------------------------------------------------------------------------*/
/* End tokens/h */
