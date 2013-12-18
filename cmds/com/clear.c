
static char *rcsid = "$Header: /hsrc/cmds/com/RCS/clear.c,v 1.3 1992/05/05 08:07:16 nickc Exp $";

#include <stdio.h>

int main()
{	
        putchar('L' - '@'); 	/* XXX - NC - this ought to be "putchar( '\f' );" */
	return 0;
}
