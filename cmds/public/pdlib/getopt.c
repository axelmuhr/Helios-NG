#ifndef lint
static char *RCSid = "$Header: getopt.c,v 1.1 87/08/21 16:33:11 rnovak Exp $";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source: /u3/syseng/rnovak/src/lib/RCS/getopt.c,v $
 * $Revision: 1.1 $
 * $Date: 87/08/21 16:33:11 $
 * $State: Exp $
 * $Author: rnovak $
 * $Locker:  $
 *
 *------------------------------------------------------------------
 * $Log:	getopt.c,v $
 * Revision 1.1  87/08/21  16:33:11  rnovak
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

#ifndef lint
static char rcsid[] = "$Header: getopt.c,v 1.1 87/08/21 16:33:11 rnovak Exp $";
#endif

/* got this off net.sources */
#include <stdio.h>

/*
 * get option letter from argument vector
 */
int	opterr = 1,	/* if set to zero no message for bad option */
	optind = 1,	/* index into parent argv vector */
	optopt;		/* character checked for validity */
char	*optarg;	/* argument associated with option */

#define BADCH	(int)'?'
#define EMSG	""

getopt(nargc,nargv,ostr)
int	nargc;
char	**nargv,
	*ostr;
{
    static char	*place = EMSG;	/* option letter processing */
    register char	*oli;		/* option letter list index */
    char	*index();

    if(!*place) {			/* update scanning pointer */
	if(optind >= nargc || *(place = nargv[optind]) != '-' || !*++place)
	    return(EOF);
	if (*place == '-') {	/* found "--" */
	    ++optind;
	    return(EOF);
	}
    }				/* option letter okay? */
    if ((optopt = (int)*place++) == (int)':' || !(oli = index(ostr,optopt))) {
	if(!*place) ++optind;
	if (opterr)
	    fprintf(stderr, "%s: illegal option -- %c\n", *nargv, optopt);
	return(BADCH);
    }
    if (*++oli != ':') {		/* don't need argument */
	optarg = NULL;
	if (!*place) ++optind;
    }
    else {				/* need an argument */
	if (*place) optarg = place;	/* no white space */
	else if (nargc <= ++optind) {	/* no arg */
	    place = EMSG;
	    if (opterr)
		fprintf(stderr, "%s: option requires an argument -- %c\n",
		    *nargv, optopt);
	    return(BADCH);
	}
	 else optarg = nargv[optind];	/* white space */
	place = EMSG;
	++optind;
    }
    return(optopt);			/* dump back option letter */
}
