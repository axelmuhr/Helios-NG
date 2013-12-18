

/* Stand-Alone Kernel Stubs */

#undef CODES

#include "kernel.h"
#include <message.h>
#include <event.h>
#include <link.h>
#include <config.h>
#include <protect.h>

Port NewPort(void) { return NullPort; }
Code FreePort(Port p) { p=p; return Err_BadPort; }
Code PutReady(Port p) { p=p; return Err_BadPort; }
Code GetReady(Port p) { p=p; return Err_BadPort; }
Code AbortPort(Port p, Code c) { p=p,c=c; return Err_BadPort; }
Code GetPortInfo(Port p, PortInfo *i) { p=p,i=i; return Err_BadPort; }
Code _GetMsg(MCB *m) { m=m; return Err_BadPort; }
Code _PutMsg(MCB *m) { m=m; return Err_BadPort; }
Code _XchMsg(MCB *t, MCB *r) {t=t,r=r; return Err_BadPort; }

Code MultiWait(MCB *m, word n, Port *p) { m=m,n=n,p=p; return Err_Null; }
void SendException(Port p, Code c) { p=p,c=c; }

Code KillTask( Task *t ) { t=t; return Err_Null; }

Code _BootLink(word l, void *i, Config *c, word s) { l=l,i=i,c=c,s=s; return Err_BadRoute; }
Code EnableLink(word l) { l=l; return Err_BadRoute; }
Code AllocLink(word l) { l=l; return Err_BadRoute; }
Code FreeLink(word l) { l=l; return Err_BadRoute; }
Code Reconfigure(LinkConf *c) { c=c; return Err_BadRoute; }
Code Configure( LinkConf c) { c=c; return Err_BadRoute; }
Code LinkData(word l, LinkInfo *i) { l=l,i=i; return Err_BadRoute; }
Code SoftReset(word l) { l=l;return Err_BadRoute; }

Code SetEvent(Event *e) { e=e; return Err_Null; }
Code RemEvent(Event *e) { e=e; return Err_Null; }

void PortInit(Config *config) {config=config;}
void EventInit(Config *config) {config=config;}

void LinkInit(Config *config, Task *procman) 
{
#define bootlink	0	
	procman=procman;	
	config->LinkConf[(((word)bootlink&0x1f)-16)/4].Flags |= Link_Flags_parent|Link_Flags_debug;
}


Code LinkIn(word size, word linkid, void *buf, word timeout)
{
	in_(size,linkid,buf);
	return Err_Null;
	timeout=timeout;
}

Code LinkOut(word size, word linkid, void *buf, word timeout)
{
	out_(size,linkid,buf);
	return Err_Null;
	timeout=timeout;
}

void Terminate(void)
{
	start_();
}


int GetROMConfig(Config *config)
{
	int i;

	/* Size of initial port table (and further increments) */
	config->PortTabSize = 1024L;

	/* Number of times booted this session - immaterial */
	config->Incarnation = 1;


	/* Current time to initialise Helios's internal clock and date from */
	/* This is a unix style date stamp, as seconds since Jan 1st 1970 GMT */
	config->Date        = 0; /* get_unix_style_time();*/

	/* The the first program in the system image to run */
	config->FirstProg   = 8;

	/* Size of memory (Default specified by executive if zero) */
	config->MemSize      = 0;

	config->Flags       = Root_Flags_rootnode|Root_Flags_special;

	/* Reserved locations */
	config->Spare[0]    = 0;

	/* Number of transputer style links attached to system */
	config->NLinks      = 4;

	/* Set up link configuration information */

	for (i=0; i < config->NLinks; i++) 
	{
		/* link not connected to IO processor or accepting IOdebugs and didn't boot us */
		config->LinkConf[0].Flags = 0x00;

		/* Note that link should be part of network */
		config->LinkConf[0].Mode  = Link_Mode_Dumb;

		/* but currently is not communicating (dead) */
		config->LinkConf[0].State = Link_State_Dead;

		/* link config for link ID zero */
		config->LinkConf[i].Id    = i;	/* Link 0 */
	}

	/* setup names of nonexistant IO processor and this processors name */
	/* These two RPTRs can be -1 to signify /00 and /IO as acceptable defaults */

	config->MyName      = -1;		/* default to /00 */
	config->ParentName  = -1;		/* default to /IO */

	return(sizeof(Config));
}


/* Notes no ROM items available */

bool GetROMItem(word *index, char **name, word *size, Matrix *m, char **data)
{
	return FALSE; /* no items */
#ifdef __TRAN
	data  = data;
	m     = m;
	size  = size;
	name  = name;
	index = index;
#endif
}


/* Returns standard position of the base of the nucleus */

MPtr GetNucleusBase()
{
	return ((MPtr)0x80001000); /* first addr. in external ram */
}


/* Returns standard position of the start of RAM */
/* In RAM loaded systems this is directly after the nucleus */ 

RootStruct *GetRootBase()
{
	/* first word of nucleus is the size in bytes of the nucleus */
	return ( (RootStruct *)GetNucleusBase() + (*((word *)GetNucleusBase())/sizeof(word)) );
}

