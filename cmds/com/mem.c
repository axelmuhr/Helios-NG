/*{{{  Header */

#ifdef __TRAN
static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/com/RCS/mem.c,v 1.12 1994/06/07 12:30:45 nickc Exp $";
#endif

/*}}}*/
/*{{{  Includes */

#include <stddef.h>
#include <syslib.h>
#include <root.h>
#include <stdio.h>
#include <memory.h>
#include <task.h>
#include <servlib.h>
#include <string.h>

/*}}}*/
/*{{{  Constants */

#define tabmax 100

/*}}}*/
/*{{{  Macros */

#define FirstPoolBlock(pool) ((Memory *)(pool + 1))
#define LastPoolBlock(poo)  ((Memory *)(poo->Memory.Head))

/*}}}*/
/*{{{  Variables */

Memory * old[ tabmax + 1 ];
Memory * New[ tabmax + 1 ];
char     lbuf[ 80 ];

/*}}}*/
/*{{{  Code */

/*{{{  IndentifyCode() */

const char *
IdentifyCode( char * desc, MPtr m )
{
  word type = ModuleWord_( m, Type );
  char name[ 32 ];
	

  if  ( type == T_Program ) strcat( desc, " Program: " );
  elif( type == T_Module )  strcat( desc, " Module : " );
  elif( type == T_Device )  strcat( desc, " Device : " );
  else return NULL;
  
  ModuleName_( name, m );
  strcat( desc, name );
	
  return "loader";
}

/*}}}*/
/*{{{  main() */

int
main(int argc)
{
  RootStruct *	Root = GetRoot();
  Pool *	p;
  Pool *	mypool;
  Pool *	freepool;
  Pool *	syspool;
  Pool *	codepool;
  Pool **	specials;
#if defined(__TRAN) || defined(__C40) 
  Pool *	fastpool;
#endif
#ifdef __RRD
  Pool *	rrdpool;
#endif
  Memory *	m;
  Memory *	last;
  bool		monitor = argc > 1;

  
  mypool   = &MyTask->MemPool;
  freepool = p = Root->FreePool;
  syspool  = &(Root->SysPool);
  codepool = Root->LoaderPool;
  specials = Root->SpecialPools;
  
#if defined(__TRAN) || defined(__C40)
  fastpool = &(Root->FastPool);
#endif		
#ifdef __RRD
  rrdpool = Root->RRDPool;
#endif
  
  old[ 0 ] = NULL;
  
  forever
    {
      word 	x        = 0;
      word	total    = 0;
      word	maxblock = 0;
      char	c;
#ifdef NEW_SYSTEM
      ExecInfo	sExecInfo;

      GetExecInfo( &sExecInfo );
#endif      
      
      printf( "  Block :   Size  State     Owner\n" );
      
#ifdef NEW_SYSTEM
      WaitMutex( sExecInfo.MemoryLock );
#endif
      
      m    = FirstPoolBlock( p );
      last = LastPoolBlock(  p );

      until ( m >= last )
	{
	  char 		desc[ 60 ];
	  const char *	owner = NULL;
	  word		size  = m->Size & (~15);
	  MPtr		caddr = NULL;
	  word		csize = -1;
	  word		alloc = m->Size & Memory_Size_FwdBit;
	  Memory *	next  = (Memory *)((word)m + size);

	  
	  *desc = 0;
	  
	  if (++x < tabmax) New[ x ] = m;
	  
	  if (!alloc)
	    {
	      total += size;
	      if (size > maxblock) maxblock = size;
	    }
	  
#if 0		
	  if (m->Pool == mypool)
	    strcat( desc, " mem's data" );
	  elif ((Program *)(m + 1) == MyTask->Program)
	    strcat( desc," mem's code" );
#endif
	  
	  if ((m->Pool == freepool &&  alloc) ||
 	     ( m->Pool != freepool && !alloc) ) 
	    {
	      strcat( desc, " INCONSISTENT" );
	    }
	  
	  if ((alloc && (next->Size & Memory_Size_BwdBit) == 0) ||
	     (!alloc && (next->Size & Memory_Size_BwdBit) != 0))
	    {
	      strcat( desc, " BWD bit of next wrong" );
	    }
	  
	  if (alloc && ((*(word *)(m + 1)) & T_Mask) == T_Valid)
	    {
	      owner = IdentifyCode( desc, CtoM_( m + 1 ) );
	    }
	  
	  if (alloc && (*(word *)(m+1) == (word)(m + 1)))
	    {
	      strcat( desc, " Module Table" );
	    }
	  
	  if (owner == NULL && alloc && m->Pool == codepool)
	    {
	      Carrier *	c = (Carrier *)(m + 1);
	      MPtr	w = c->Addr;

	      
	      caddr = w;
	      csize = c->Size;
#ifdef __C40
	      csize *= sizeof( word );
#endif
	      if ((ModuleWord_( w, Type ) & T_Mask) == T_Valid)
		{
		  owner = IdentifyCode( desc, w );
		}
	    }
	  
	  if (owner == NULL && (m->Size & Memory_Size_Carrier))
	    {
	      int 	i;
	      Carrier *	c = (Carrier *)(m + 1);

	      
	      caddr = c->Addr;
	      csize = c->Size;
#ifdef __C40
	      csize *= sizeof (word);
#endif
	      for (i = 0 ; i < 8 ; i++)
		{
		  if (m->Pool == specials[ i ])
		    {
		      strcat( desc, (i & 1) ? " fast"     : "" );
		      strcat( desc, (i & 2) ? " indirect" : "" );
		      strcat( desc, (i & 4) ? " global"   : " local" );
		      owner = "Special";
		      alloc = 0;
		      break;
		    }
		}
	      
	    }
	  
	  if (monitor)
	    {
	      word y;
	      
	      for (y = 0; old[y] != NULL; y++)
		if (old[ y ] == m)
		  break;
	      
	      if (old[ y ] == NULL) strcat( desc," new" );
	    }

	  if (owner == NULL)
	    {
	      if   ( m->Pool == freepool ) owner = "FreePool";
	      elif ( m->Pool == syspool )  owner = "SysPool";
	      elif ( m->Pool == codepool ) owner = "Loader";
#if defined(__TRAN) || defined(__C40) || defined(__ABC)
	      elif ( m->Pool == fastpool ) owner = "FastPool";
#endif
#ifdef __RRD
	      elif ( m->Pool == rrdpool )  owner = "RRDPool";
#endif
              else
		{
		  Task *	task = (Task*)((int)(m->Pool) - offsetof( Task, MemPool ));
		  ObjNode *	entry = (ObjNode *)task->TaskEntry;
		  
		  owner = entry->Name;
		}
	    }
	  
#ifdef NEW_SYSTEM
	  SignalMutex( sExecInfo.MemoryLock );
#endif
	  printf( "%08p: %7ld %s %10s ", m, size, alloc ? "Alloc" : "Free ", owner );
	  
	  if ( csize != -1 ) printf( " %08lx: %7ld", caddr, csize );

	  printf( " %s\n", desc );
	  
	  m = (Memory *)((word)m + (m->Size & ~15));
	  
#ifdef NEW_SYSTEM
	  WaitMutex( sExecInfo.MemoryLock );
#endif
	}
      
#ifdef NEW_SYSTEM
      SignalMutex( sExecInfo.MemoryLock );
#endif
      
      printf( "%ld Available, %ld Largest\n", total, maxblock );
      
      if ( !monitor ) break;
      
      printf("Hit CR to repeat, 'q' to quit..."); fflush(stdout);
      c = getchar();
      putchar('\n');
      
      if( c == 'q' ) break;
      
      New[x] = NULL;
      memcpy(old,New,tabmax*sizeof(Memory *));
    }
  
  return 0;
}

/*}}}*/

/*}}}*/
