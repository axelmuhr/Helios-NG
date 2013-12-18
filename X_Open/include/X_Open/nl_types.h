/******************************************************************************
**
**
**	FILE		:	X_Open/nl_types.h
**
**
**	DESCRIPTION	:	X-Open header file : <nl_types.h>
**
**
******************************************************************************/

typedef FILE *		nl_catd;
typedef unsigned int	nl_item;

#define NL_SETD		01	/* set par defaut */
#define NL_SETMAX	10 	/* nb max de set */
#define NL_MSGMAX	60	/* nb max de messages par set */
#define NL_TEXTMAX	80	/* longueur max d'un message en octets */

extern	int catclose(nl_catd catd);
extern	char *catgets(nl_catd catd, int set_id, int msg_id, char *s)
extern	nl_catd catopen(char *name, int oflag)
