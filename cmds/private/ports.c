/*{{{  Header */

#ifdef __TRAN
static char *rcsid = "$Header: /hsrc/cmds/private/RCS/ports.c,v 1.6 1994/03/07 13:40:57 nickc Exp $";
#endif

/*}}}*/
/*{{{  Includes */

#include <stdio.h>
#include <syslib.h>
#include <task.h>
#include <servlib.h>

#ifdef NEW_SYSTEM
#include <message.h>
#else
typedef struct PTE {
	byte		Type;
	byte		Cycle;
	byte		Flags;
	byte		Uses;
	word		Owner;
	word		TxId;
	word		RxId;
} PTE;

#define in_kernel 1	/* trick root.h into letting us define PTE	*/
#endif

#include <root.h>
#include <stdlib.h>
#include <string.h>	/* for memcpy */

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

#define PTNumBlocks	64	/* NB/ This must match definition in kernel.h */

/*}}}*/
/*{{{  Code */

/*{{{  main() */

int
main(
     int 	argc,
     char **	argv )
{
  RootStruct *	root = GetRoot();
  PTE *		p;
  PTE **	basetab;
  word		ptsize;
  word		freeq   = root->PTFreeq;
  word		i;
  bool		freeonly = false;
  bool		full = false;
#ifdef NEW_SYSTEM
  ExecInfo	sExecInfo;

  
  GetExecInfo( &sExecInfo );

  basetab = (PTE **)malloc( (int)root->PTSize * sizeof (PTE *) );
  
  if (basetab == NULL) 
    {
      fprintf( stderr, "no memory available\n" );
      return EXIT_FAILURE;
    }
  
  WaitMutex( sExecInfo.PortsLock );

  /* copy the PortTable */
  
  memcpy( basetab, root->PortTable, (int)root->PTSize );
  
  SignalMutex( sExecInfo.PortsLock );
  
  ptsize  = root->PTSize / sizeof (PTE *);
#else  

  basetab = (root->PortTable);
  ptsize  = root->PTSize / 4 - 1;

#endif

  
  if ( argc > 1 )
    {
      char *s = argv[1];

      do
	{
	  switch( *s )
	    {
	    case 'f': freeonly = true; break;
	    case 'a': full = true; break;
	    }
	}
      while ( *(++s) != '\0' );
    }

  if ( !freeonly )
    {
      word j;

      
      printf( "PortTable:\n" );
#ifdef NEW_SYSTEM
      printf( "  Port    Type  Rcv" );
#else
      printf( "  Port    Type  Age" );
      if ( full ) printf("   TxId     RxId    ");
#endif
      printf("         Owner\n");

      for ( j = 0; j < ptsize; j++ )
	{
	  p = basetab[ j ];

	  if ( p == 0 ) break;
	  if ( p == (PTE *)(MinInt) ) continue;

	  for ( i = 0; i < PTNumBlocks; i++ )
	    {
	      PTE *	pte = &p[i];
	      char *	state = pte->Type==T_Local     ? "Local" :
		                pte->Type==T_Free      ? "Free " :
		                pte->Type==T_Trail     ? "Trail" :
		                pte->Type==T_Permanent ? "Perm " :
		                                         "Surr " ;

	      if ( pte->Type != T_Free )
		{
		  printf("%02x%02x%02lx%02lx  %s %3d",
			 (pte->Type==T_Local)?0x80:0xe0,
			 pte->Cycle,j,i,state,
#ifdef NEW_SYSTEM
			 pte->Receiving
#else
			 pte->Uses
#endif			 
			 );
#ifndef NEW_SYSTEM
		  if ( full )
		    printf("   %08x %08x", pte->TxId,pte->RxId);
#endif		
				
		  if ( pte->Owner == NULL ) printf("        Kernel");
		  else 
		    {
		      Task *task = (Task *)pte->Owner;
		      ObjNode *entry = (ObjNode *)task->TaskEntry;
		      
		      if ( entry != NULL )
			printf("   %11s",entry->Name);
		      else
			printf("   task %p entry %p",task,entry);
		    }

		  if ( pte->Type == T_Local )
		    putchar('\n');
		  else
#ifdef NEW_SYSTEM
		    printf("   link %1d\n",pte->Link);
#else
		  printf("   link %1d -> %08x\n",pte->Flags,pte->TxId);
#endif				  
		}
	    }
	}
    }

  printf( "Free Queue :\n" );

  i = 0;
  
  while ( freeq > 0 )
    {
      i++;
      printf( "%04lx ", freeq & 0xffff );
      p = basetab[ (freeq >> 8) & 0xff ];
      freeq = p[ freeq & 0xff ].Owner;
    }

  printf( "\nFree Queue size = %ld\n", i );

#ifdef NEW_SYSTEM
  free( basetab );
#endif
  
  return EXIT_SUCCESS;
}

/*}}}*/

/*}}}*/
