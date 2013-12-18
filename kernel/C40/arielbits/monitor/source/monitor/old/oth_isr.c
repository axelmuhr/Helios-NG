#include "hydra.h"
#include "host.h"

static HostMessage *msg;


void c_int01( void )
{
	switch( msg->WhatToDo )
	{
		case CopyStuff:
			copy( msg->Parameters );
			msg->WhatToDo = SUCCESS;
			break;
		case Run:
			msg->WhatToDo = SUCCESS;
			RunForHost( msg->Parameters[0] );
			break;
	}
}


void InitHost( int WhoAmI, unsigned long DramAddr, unsigned long DramLength )
{
	msg = (HostMessage *)(DramAddr + DramLength - ((WhoAmI - 1) * sizeof(HostMessage)));
}
