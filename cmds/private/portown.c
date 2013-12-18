/*{{{  Includes */

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
#include <task.h>

/*}}}*/
/*{{{  Constants */

#define T_Free		0
#define T_Local 	1
#define T_Surrogate	2
#define T_Trail		3
#define T_Permanent	4

#define TABLE_SIZE	100

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

typedef struct owner {
	word	owner;
	word	surr;
	word	local;
} owner;

/*}}}*/
/*{{{  Variables */

char lbuf[80];
static owner table[ TABLE_SIZE ];

/*}}}*/
/*{{{  Code */

int
main(int argc,char **argv)
{
  RootStruct *root = GetRoot();
  PTE *p;
  PTE **basetab = (root->PortTable);
  word ptsize = root->PTSize/4 - 1;
  word i,j, k;
  char name[512];
#ifdef NEW_SYSTEM
  ExecInfo	sExecInfo;

  
  GetExecInfo( &sExecInfo );
#endif
  
  for (i = 0; i < TABLE_SIZE; i++)
    {
      table[i].owner = 0;
      table[i].surr = 0;
      table[i].local = 0;
    }
	 
  MachineName( name );

#ifdef NEW_SYSTEM
  WaitMutex( sExecInfo.PortsLock );
#endif
  
  for ( j = 0; j < ptsize; j++ )
    {
      p = basetab[j];
      if ( p == 0 ) break;
      if ( p == (PTE *)(MinInt) ) continue;

      for ( i = 0; i < 64; i++ )
	{
	  PTE *	pte = &p[i];
	  word	owner;

	  owner = pte->Owner;
	  if (owner == NULL) owner = 1;   /* kernel */
	  if ((pte->Type == T_Local) || (pte->Type == T_Surrogate))
	    {
	      for (k = 0; 
		   (table[k].owner != 0) && (table[k].owner != owner);
		   k++)
		;

	      table[k].owner = owner;

	      if (pte->Type == T_Local)
		table[k].local++;
	      else
		table[k].surr++;
	    }
	}
    }
  
#ifdef NEW_SYSTEM
  SignalMutex( sExecInfo.PortsLock );
#endif
            
  printf( "Ports on %s\n", name );
  
  for (i = 0; table[i].owner != 0; i++)
    {
      Task *	task;
      ObjNode *	entry;
      char *	name;

      
      if (table[i].owner == 1)
	name = "Kernel";
      else
	{
	  task  = (Task *) table[i].owner;
	  entry = (ObjNode *) task->TaskEntry;

	  if (entry == Null(ObjNode))
	    name = "Unknown";
	  else
	    name = entry->Name;
	}   

      printf( "Program %12s : %4ld local ports, %4ld surrogate ports\n", name,
	     table[i].local, table[i].surr);
    }
}

/*}}}*/
