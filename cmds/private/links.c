#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/links.c,v 1.3 1994/03/08 12:14:26 nickc Exp $";
#endif

#include <stdio.h>
#include <root.h>
#include <link.h>
#include <stdlib.h>

int
main()
{

	LinkInfo **linkv = GetRoot()->Links;

printf(
"             Transmit Receive         IOC Ports              Messages\n");
printf(
"Id Fl Md St  Channel  Channel Inc   Local   Remote       In      Out   Lost\n");
	while( *linkv != NULL )
	{
		LinkInfo *l = *linkv++;

		printf("%2d %2x %2x %2x %8lx %8lx %2ld %8lx %8lx %8ld %8ld   %-6ld",
			l->Id,l->Flags,l->Mode,l->State,l->TxChan,l->RxChan,
			l->Incarnation,l->LocalIOCPort,l->RemoteIOCPort,
			l->MsgsIn,l->MsgsOut,l->MsgsLost);

/*		printf(" %d",l->TxMutex.Count);*/
		putchar('\n');
	}

	return EXIT_SUCCESS;
}
