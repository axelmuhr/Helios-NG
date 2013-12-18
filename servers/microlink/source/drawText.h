/* $Header: drawText.h,v 1.1 91/01/31 13:52:21 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/source/drawText.h,v $ */

/*----------------------------------------------------------------------*/
/*                                                   source/drawText.h  */
/*----------------------------------------------------------------------*/

/* This file is the header file for a small utility which reads fonts   */
/*   and uses the draw package to draw them on the screen.              */

/*----------------------------------------------------------------------*/
/*                                                           Interlock  */
/*----------------------------------------------------------------------*/

#ifndef _TEXT_H
#define _TEXT_H

/*----------------------------------------------------------------------*/
/*                                                        Header Files  */
/*----------------------------------------------------------------------*/

#include "drawp/public.h"

/*----------------------------------------------------------------------*/
/*                                                               Macros */
/*----------------------------------------------------------------------*/


#define CHARSETFILE "/files/helios/charset"
#define LINEINC 8


/*----------------------------------------------------------------------*/
/*                                                              Globals */
/*----------------------------------------------------------------------*/

extern DpPixmap_t                   *charSet;

/*----------------------------------------------------------------------*/
/*                                                  Function prototypes */
/*----------------------------------------------------------------------*/

void writeText(char *string, int line);
void drawString(int x, int y, char *string);
void drawChar(int x, int y, char ch);
DpPixmap_t *readFile(const char *Name);

/*----------------------------------------------------------------------*/
/*                                                    End Of Interlock  */
/*----------------------------------------------------------------------*/

#endif  /* _TEXT_H */
