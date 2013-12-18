/*------------------------------------------------------------------------
--	memsearch.c							--
--	BLV, 03/03/88							--
--	Update PAB 10/3/92						--
--	A nasty little program which looks through memory searching for --
--      RCS/SCCS id's.							--
------------------------------------------------------------------------*/

static char *rcsid = "$Header: /hsrc/cmds/private/RCS/memsearch.c,v 1.3 1992/03/11 14:17:02 paul Exp $";

#ifdef __TRAN
# define STARTMEM 0x80000000
#else
# define STARTMEM 0
#endif

#define ENDMEM (STARTMEM + 1024 * 1024 * 32)	/* allow for 32 megabytes */

#include <stdio.h>

int main(int argc, char **argv)
{
	char *ptr = STARTMEM;

	for (; (long)ptr < ENDMEM  ; ptr++) {
		/* SCCS id's begin with @(#) */
		if (*ptr == '@' && *++ptr == '(' && \
		    *++ptr == '#' && *++ptr == ')') {
				/* found one - I hope */
			 	ptr++;
				puts(ptr);
		}
		/* RCS id's begin with @(#) */
		if (*ptr == '$' && *++ptr == 'H' && \
		    *++ptr == 'e' && *++ptr == 'a' &&
		    *++ptr == 'd' && *++ptr == 'e' &&
		    *++ptr == 'r' && *++ptr == ':' ) {
				/* found one - I hope */
			 	ptr++;
				puts(ptr);
		}
	}
}
