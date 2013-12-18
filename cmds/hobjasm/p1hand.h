/* -> p1hand/h
 * Title:               The general line processing routine
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef p1linehandler_h
#define p1linehandler_h

#include "constant.h"

BOOLEAN P1LineHandler(char **linkPointer, BOOLEAN *wasLink);

BOOLEAN AllComment(char *line, CARDINAL *index);
/*Check for line all comment*/

BOOLEAN TermCheck(char ch);
/*Check character is a valid element terminator*/

#endif

/* End p1hand/h */
