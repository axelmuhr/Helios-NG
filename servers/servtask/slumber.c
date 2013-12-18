/* RcsId: $Id: slumber.c,v 1.1 1990/10/17 13:43:41 bart Exp $ (C) Copyright 1988, Perihelion Software Ltd. */ 

#include <helios.h>
#include <message.h>
#include <root.h>

int main(void)
{ MCB mcb;
  RootStruct *Root = GetRoot();
  
  *((int *) &mcb)  = 0;
  mcb.MsgHdr.Dest  = ((Root->Links)[0])->RemoteIOCPort;
  mcb.MsgHdr.Reply = NullPort;
  mcb.MsgHdr.FnRc  = -2;
  mcb.Timeout      = -1;
  return(PutMsg(&mcb));
}
 
