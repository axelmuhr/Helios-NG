/*{{{  Incldues */

#ifdef NEW_SYSTEM
#include <message.h>
#else
#define in_kernel 1	/* trick root.h into letting us define PTE	*/
#if defined __C40 || defined __ARM
# define GetRoot() ((RootStruct *)GetRootBase())
#endif
#endif

#include <root.h>
#include <stdio.h>
#include <syslib.h>
#include <task.h>
#include <servlib.h>

/*}}}*/
/*{{{  Variables */

char lbuf[80];

/*}}}*/
/*{{{  Constants */

#define T_Free		0
#define T_Local 	1
#define T_Surrogate	2
#define T_Trail		3
#define T_Permanent	4

/*}}}*/
/*{{{  Types */

#ifndef NEW_SYSTEM
typedef struct PTE {
	byte		Type;
	byte		Cycle;
	byte		Flags;
	byte		Uses;
	word		Owner;
	word		TxId;
	word		RxId;
} PTE;
#endif

/*}}}*/
/*{{{  Code */

int
main(int argc,char **argv)
{
	RootStruct *root = GetRoot();
	PTE *p;
	PTE **basetab = (root->PortTable);
	word ptsize = root->PTSize/4 - 1;
	word i,j;
	int ports_local = 0;
	int ports_free  = 0;
	int ports_trail = 0;
	int ports_permanent = 0;
	int ports_surr = 0;
	char name[512];
#ifdef NEW_SYSTEM
  ExecInfo	sExecInfo;

  
  GetExecInfo( &sExecInfo );
#endif
	
	MachineName(name);
#ifdef NEW_SYSTEM
  WaitMutex( sExecInfo.PortsLock );
#endif
		
	for( j = 0; j < ptsize; j++ )
	{
		p = basetab[j];
		if( p == 0 ) break;
		if( p == (PTE *)(MinInt) ) continue;
		for( i = 0; i < 64; i++ )
		{
			PTE *pte = &p[i];
			switch (pte->Type)
			 { case T_Local : ports_local++; break;
			   case T_Free	: ports_free++; break;
			   case T_Trail : ports_trail++; break;
			   case T_Permanent : ports_permanent++; break;
			   case T_Surrogate : ports_surr++; break;
			}
		}
	}
	
#ifdef NEW_SYSTEM
  SignalMutex( sExecInfo.PortsLock );
#endif
            
	printf("Ports on %s : local %d, free %d, trail %d, permanent %d, surrogate %d\n",
	     name, ports_local, ports_free, ports_trail, ports_permanent, ports_surr);

}

/*}}}*/
