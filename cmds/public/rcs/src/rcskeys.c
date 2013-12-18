/*
 *                     RCS keyword table and match operation
 */
#ifndef lint
static char rcsid[]= "$Id: rcskeys.c,v 4.5 90/01/02 11:20:37 chris Exp $ Purdue CS";
#endif

/* Copyright (C) 1982, 1988, 1989 Walter Tichy
   Distributed under license by the Free Software Foundation, Inc.

This file is part of RCS.

RCS is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 1, or (at your option)
any later version.

RCS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RCS; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

Report problems and direct all questions to:

    rcs-bugs@cs.purdue.edu

*/



/* $Log:	rcskeys.c,v $
 * Revision 4.5  90/01/02  11:20:37  chris
 * Helios port
 * 
 * Revision 4.3  89/05/01  15:13:02  narten
 * changed copyright header to reflect current distribution rules
 * 
 * Revision 4.2  87/10/18  10:36:33  narten
 * Updating version numbers. Changes relative to 1.1 actuallyt
 * relative to 4.1
 * 
 * Revision 1.2  87/09/24  14:00:10  narten
 * Sources now pass through lint (if you ignore printf/sprintf/fprintf 
 * warnings)
 * 
 * Revision 1.1  84/01/23  14:50:32  kcs
 * Initial revision
 * 
 * Revision 4.1  83/05/04  10:06:53  wft
 * Initial revision.
 * 
 */


#include "rcsbase.h"



struct { char * keyword; enum markers marker;} markertable[] =
        {{AUTHOR,   Author  },
         {DATE,     Date    },
         {HEADER,   Header  },
         {IDH,      Id      },
         {LOCKER,   Locker  },
         {LOG,      Log     },
         {RCSFILE,  RCSfile },
         {REVISION, Revision},
         {SOURCE,   Source  },
         {STATE,    State   },
         {nil,      Nomatch }};



enum markers trymatch(string,onlyvdelim)
char * string;
/* function: Checks whether string starts with a keyword followed
 * by a KDELIM or a VDELIM. If onlyvdelim==true, only a VDELIM
 * may follow the keyword.
 * If successful, returns the appropriate marker, otherwise Nomatch.
 */
{
        register int j;
	register char * p, * s;
        for (j=0; markertable[j].keyword!=nil; j++ ) {
		/* try next keyword */
		p = markertable[j].keyword; s = string;
		while (*p!='\0' && *s!='\0' && *p == *s) {
			p++; s++;
		}
		if (*p != '\0') continue; /* no match */
		if ((*s == VDELIM) || (!onlyvdelim && (*s == KDELIM)))
			return(markertable[j].marker);
        }
        return(Nomatch);
}

