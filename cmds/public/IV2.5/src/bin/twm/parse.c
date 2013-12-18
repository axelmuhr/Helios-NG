/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    name  of Evans & Sutherland  not be used in advertising or publi-    **/
/**    city pertaining to distribution  of the software without  specif-    **/
/**    ic, written prior permission.                                        **/
/**                                                                         **/
/**    EVANS  & SUTHERLAND  DISCLAIMS  ALL  WARRANTIES  WITH  REGARD  TO    **/
/**    THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILI-    **/
/**    TY AND FITNESS, IN NO EVENT SHALL EVANS &  SUTHERLAND  BE  LIABLE    **/
/**    FOR  ANY  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY  DAM-    **/
/**    AGES  WHATSOEVER RESULTING FROM  LOSS OF USE,  DATA  OR  PROFITS,    **/
/**    WHETHER   IN  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS    **/
/**    ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE  OR PER-    **/
/**    FORMANCE OF THIS SOFTWARE.                                           **/
/*****************************************************************************/

/***********************************************************************
 *
 * $Header: parse.c,v 1.16 88/07/19 13:49:08 toml Exp $
 *
 * parse the .twmrc file
 *
 * 17-Nov-87 Thomas E. LaStrange		File created
 *
 ***********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$Header: parse.c,v 1.16 88/07/19 13:49:08 toml Exp $";
#endif

#include <stdio.h>
#include "twm.h"
#include "menus.h"
#include "util.h"

#define BUF_LEN 300

static FILE *twmrc;
static int ptr = 0;
static int len = 0;
static char buff[BUF_LEN+1];
extern int yylineno;

/***********************************************************************
 *
 *  Procedure:
 *	ParseTwmrc - parse the .twmrc file
 *
 *  Inputs:
 *	filename  - the filename to parse.  A NULL indicates $HOME/.twmrc
 *
 ***********************************************************************
 */

void
ParseTwmrc(filename)
char *filename;
{
    char *home;
    char init_file[200];

    InitMenus();

    if (filename == NULL)
    {
	home = (char *)getenv("HOME");
	strcpy(init_file, home);
	strcat(init_file, "/.twmrc");
    }
    else
	strcpy(init_file, filename);

    if ((twmrc = fopen(init_file, "r")) == NULL)
    {
	fprintf(stderr, "twm: couldn't open \"%s\"\n", init_file);
    	return;
    }

    ptr = 0;
    len = 0;
    yylineno = 0;
    ParseError = FALSE;

    yyparse();

    fclose(twmrc);

    if (ParseError)
    {
	fprintf(stderr, "twm: errors found in \"%s\", twm aborting\n",
	    init_file);
	Done();
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	TwmInput - redefinition of the lex input routine
 *
 *  Returned Value:
 *	the next input character
 *
 ***********************************************************************
 */

int
TwmInput()
{
    while (ptr == len)
    {
	if (fgets(buff, BUF_LEN, twmrc) == NULL)
	    return NULL;

	yylineno++;

	ptr = 0;
	len = strlen(buff);
    }
    return ((int)buff[ptr++]);
}

/***********************************************************************
 *
 *  Procedure:
 *	TwmUnput - redefinition of the lex unput routine
 *
 *  Inputs:
 *	c	- the character to push back onto the input stream
 *
 ***********************************************************************
 */

void
TwmUnput(c)
{
    buff[--ptr] = c;
}

/***********************************************************************
 *
 *  Procedure:
 *	TwmOutput - redefinition of the lex output routine
 *
 *  Inputs:
 *	c	- the character to print
 *
 ***********************************************************************
 */

void
TwmOutput(c)
{
    putchar(c);
}
