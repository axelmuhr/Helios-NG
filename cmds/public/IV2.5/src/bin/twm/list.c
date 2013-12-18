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

/**********************************************************************
 *
 * $Header: list.c,v 1.9 88/10/13 07:19:06 toml Exp $
 *
 * TWM code to deal with the name lists for the NoTitle list and
 * the AutoRaise list
 *
 * 11-Apr-88 Tom LaStrange        Initial Version.
 *
 **********************************************************************/

#ifndef lint
static char RCSinfo[]=
"$Header: list.c,v 1.9 88/10/13 07:19:06 toml Exp $";
#endif lint

#include <stdio.h>
#include "twm.h"
#include "gram.h"

typedef struct name_list name_list;

struct name_list
{
    name_list *next;		/* pointer to the next name */
    char *name;			/* the name of the window */
    char *ptr;			/* list dependent data */
};

name_list *NoTitle = NULL;	/* list of window names with no title bar */
name_list *AutoRaise = NULL;	/* list of window names to auto-raise */
name_list *Icons = NULL;	/* list of window names and icons */
name_list *NoHighlight = NULL;	/* list of windows no to highlight */
name_list *DontIconify = NULL;	/* don't iconify by unmapping */
name_list *IconMgrNoShow = NULL;/* don't show in the icon manager */

/***********************************************************************
 *
 *  Procedure:
 *	AddToList - add a window name to the appropriate list
 *
 *  Inputs:
 *	list	- a #define to identify the list
 *	name	- a pointer to the name of the window 
 *	ptr	- pointer to list dependent data
 *
 *  Special Considerations
 *	If the list does not use the ptr value, a non-null value 
 *	should be placed in it.  LookInList returns this ptr value
 *	and procedures calling LookInList will check for a non-null 
 *	return value as an indication of success.
 *
 ***********************************************************************
 */

void
AddToList(list, name, ptr)
int list;
char *name;
char *ptr;
{
    name_list *nptr;

    nptr = (name_list *)malloc(sizeof(name_list));
    if (nptr == NULL)
    {
	fprintf(stderr, "twm: out of memory\n");
	Done();
    }

    nptr->name = name;

    switch (list)
    {
    case ICONMGR_NOSHOW:
	nptr->next = IconMgrNoShow;
	nptr->ptr = (char *)TRUE;
	IconMgrNoShow = nptr;
	break;

    case NO_HILITE:
	nptr->next = NoHighlight;
	nptr->ptr = (char *)TRUE;
	NoHighlight = nptr;
	break;

    case AUTO_RAISE:
	nptr->next = AutoRaise;
	nptr->ptr = (char *)TRUE;
	AutoRaise = nptr;
	break;

    case NO_TITLE:
	nptr->next = NoTitle;
	nptr->ptr = (char *)TRUE;
	NoTitle = nptr;
	break;

    case ICONS:
	nptr->next = Icons;
	nptr->ptr = ptr;	/* this is the pixmap */
	Icons = nptr;
	break;

    case DONT_ICONIFY_BY_UNMAPPING:
	nptr->next = DontIconify;
	nptr->ptr = (char *)TRUE;
	DontIconify = nptr;
	break;
    }
}

/***********************************************************************
 *
 *  Procedure:
 *	LookInList - look through a list for a window name, or class
 *
 *  Returned Value:
 *	the ptr field of the list structure or NULL if the name 
 *	or class was not found in the list
 *
 *  Inputs:
 *	list	- a #define to identify the list
 *	name	- a pointer to the name to look for
 *	class	- a pointer to the class to look for
 *
 ***********************************************************************
 */

char *
LookInList(list, name, class)
int list;
char *name;
XClassHint *class;
{
    name_list *l;
    name_list *nptr;

    switch (list)
    {
    case ICONMGR_NOSHOW:
	l = IconMgrNoShow;
	break;

    case NO_HILITE:
	l = NoHighlight;
	break;

    case AUTO_RAISE:
	l = AutoRaise;
	break;

    case NO_TITLE:
	l = NoTitle;
	break;

    case ICONS:
	l = Icons;
	break;

    case DONT_ICONIFY_BY_UNMAPPING:
	l = DontIconify;
	break;
    }

    for (nptr = l; nptr != NULL; nptr = nptr->next)
    {
	int len;

	len = strlen(nptr->name);
	if (strncmp(name, nptr->name, len) == 0 ||
	    (class && strncmp(class->res_name, nptr->name, len) == 0) ||
	    (class && strncmp(class->res_class, nptr->name, len) == 0))
	    return (nptr->ptr);
    }
    return (NULL);
}

char *
LookInNameList(list, name)
int list;
char *name;
{
    return (LookInList(list, name, NULL));
}
