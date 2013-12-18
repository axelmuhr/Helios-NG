/* -> p2dirs/h
 * Title:               Directive handler for pass 2
 * Original author:     J.G.Thackray (Acorn)
 * Latest author:       JGSmith (Active Book Company)
 *                      Copyright (C) 1989 Acorn Computers Limited
 *                      Copyright (C) 1990 Active Book Company Limited
 */

#ifndef p2directives_h
#define p2directives_h

BOOLEAN P2Directive(char *line,
  BOOLEAN *wasLink,
  BOOLEAN *passEnded,
  char **linkPointer);
/*
The returned value indicates error OR handled (i.e. was directive)
*/

#endif

/* End p2dirs/h */
