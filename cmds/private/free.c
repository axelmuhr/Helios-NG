/*{{{  Header */

/*
 * memory display utility
 *
 * hacked from code for mem by NC 21/02/89
 *
 * This code is copyright (c) 1994 by Perihelion Software Ltd.
 */

#ifdef __TRAN
static char *rcsid = "$Header: /users/nickc/RTNucleus/cmds/private/RCS/free.c,v 1.11 1994/03/08 12:30:51 nickc Exp $";
#endif

/*}}}*/
/*{{{  Includes */

#include <stdio.h>
#include <stddef.h>
#include <syslib.h>
#include <root.h>
#include <stdio.h>
#include <memory.h>
#include <task.h>
#include <servlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*}}}*/
/*{{{  Macros */

# define FirstPoolBlock(pool) ((Memory *)(pool + 1))
# define LastPoolBlock(poo)  ((Memory *)(poo->Memory.Head))

/*}}}*/
/*{{{  Types */

typedef enum
  {
    Normal,
    Long,
    Short
  }
Output;

typedef struct block
  {
    char *	owner;
    uword	alloc;
    uword	program;
    uword	module;
    uword	fast;
  }
block;

/*}}}*/
/*{{{  Constants */

#define table_block_size	10

/*}}}*/
/*{{{  Variables */

block * 	blocks;
int		table_size = 0;
int		nextfree   = 0;
char *		ProgName;

/*}}}*/
/*{{{  Code */

/*{{{  find_program_block() */

int
find_program_block( char * name )
/*
 * finds a program block
 * creates a new entry if none exists
 * returns the index of the block in the table
 */
{
  int	len   = strlen( name );
  int	j;
  int 	i;
  char *	ptr;
  char *	lname = (char *)malloc( len + 1 );
  char *	lblock;
  
  
  if (lname == NULL)
    {
      return 0;
    }
  
  strcpy( lname, name );
  
  for (i = 0; i < len; i++)
    {
      lname[ i ] = tolower( lname[ i ] );
    }
  
  if ((ptr = strrchr( lname, '/' )) != NULL)
    {
      i = 0;
      
      ++ptr;
      
      while ((lname[ i++ ] = *ptr++) != '\0')
	;
    }
  
  for (i = 0; i < nextfree; i++)
    {
      len = strlen( blocks[ i ].owner );
      
      lblock = (char *)malloc( len + 1 );
      
      if (lblock == NULL)
	{
	  free( lname );
	  
	  return 0;
	}
      
      strcpy( lblock, blocks[ i ].owner );
      
      for (j = 0; j < len; j++)
	{
	  lblock[ j ] = tolower( lblock[ j ] );
	}
      
      if ((ptr = strchr( lblock, '.' )) != NULL)
	{
	  *ptr = '\0';
	}
      
      if (!strcmp( lblock, lname  ))
	{
	  free( lblock );
	  
	  break;
	}
      
      free( lblock );
    }
  
  free( lname );
  
  if (i == nextfree)
    {
      if (nextfree == table_size)
	{
	  table_size += table_block_size;
	  
	  if ((blocks = (block *)realloc( blocks, table_size * sizeof( block ) )) == NULL)
	    {
	      fprintf( stderr, "free: unable to reallocate table\n" );
	      
	      exit( -2 );
	    }
	}
      
      memset( &blocks[ nextfree ], 0, sizeof( block ) );
      
      blocks[ nextfree++ ].owner = name;
    }		
  
  return i;
  
} /* find_program_block */

/*}}}*/
/*{{{  find_block() */

int
find_block( char * name )
/*
 * locate a given block in the table
 * creates new entry if the block cannot be found
 * returns the index of the block in the table
 */
{
  int i;
  
  for (i = 0; i < nextfree; i++)
    {
      if (!strcmp( blocks[ i ].owner, name  ))
	break;
    }
  
  if (i == nextfree)
    {
      if (nextfree >= table_size)
	{
	  table_size += table_block_size;
	  
	  if ((blocks = (block *)realloc( blocks, table_size * sizeof( block ) )) == NULL)
	    {
	      fprintf( stderr, "free: unable to reallocate table\n" );
	      
	      exit( -3 );
	    }
	}
      
      memset( &blocks[ nextfree ], 0, sizeof( block ) );
      
      blocks[ nextfree++ ].owner = name;
    }
  
  return i;
} /* find_block */

/*}}}*/
/*{{{  compare() */

int
compare(
	const void *	a,
	const void *	b )
/*
 * compare two blocks and return an integer < 0, == 0, or > 0
 * depending upon whether a < b, a == b or a > b
 */
{
  block *	first  = (block *)a;
  block *	second = (block *)b;
  word		first_size;
  word		second_size;
  
  
  first_size  = first->alloc  + first->fast  + first->module  + first->program;
  second_size = second->alloc + second->fast + second->module + second->program;
  
  return (int)first_size - (int)second_size;
  
} /* compare */

/*}}}*/
/*{{{  approximate() */

char *
approximate( uword size )
/*
 * returns formatted string approximating 'size'
 */
{
  static char	buffer[ 128 ];
  
  
  buffer[ 0 ] = '\0';
  
  if (size < 1024)
    {
      sprintf( buffer, "%ld  bytes", size );
    }
  else if (size < 1024 * 1024)
    {
      sprintf( buffer, "%ld Kbytes", (size  + 1023) / 1024 );
    }
  else
    {
      sprintf( buffer, "%ld.%ld Mbytes",
	      size / (1024 * 1024), (size - ((size / (1024 * 1024)) * 1024 * 1024)) / (1024 * 1024 / 10) );
    }
  
  return (char *)buffer;
  
} /* approximate */

/*}}}*/
/*{{{  scan() */

bool
scan( Output output )
/*
 * performs the memory scan
 * return true upon success, false otherwise
 */
{
  RootStruct *	Root = GetRoot();
  char		machine_name[ 128 ];
  Pool *		p;
  Pool *		mypool;
  Pool *		freepool;
  Pool *		syspool;
#ifdef __RRD
  Pool *		rrdpool;
#endif
#if defined(__TRAN) || defined (__C40) || defined (__ABC)
  Pool *		fastpool;
#endif
  Memory *	m;
  Memory *	last;
  uword		alloc    = 0;
  uword		fast     = 0;
  uword		freefast = 0;
  uword		module   = 0;
  uword		program  = 0;	
  uword 		total    = 0;
  uword		maxblock = 0;
  bool		fast_ram;
  int		i;
  
  
  table_size += table_block_size;
  
  if ((blocks = (block *) calloc( table_size, sizeof( block ) )) == NULL)
    {
      fprintf( stderr, "%s: insufficient memory for table\n", ProgName );
      
      return false;
    }
  
  blocks[ nextfree++ ].owner = "Loader.1";
  blocks[ nextfree++ ].owner = "SysPool";
#ifdef __RRD
  blocks[ nextfree++ ].owner = "RRDPool";
#endif
  mypool   = &MyTask->MemPool;
  
  freepool = p = Root->FreePool;
  
  syspool  = &Root->SysPool;
  
#ifdef __RRD
  rrdpool	 = Root->RRDPool;
#endif
#if defined(__TRAN) || defined(__C40) || defined(__ABC)
  fastpool = &Root->FastPool;
#endif
  
  m = FirstPoolBlock(p);
  last = LastPoolBlock(p);
  
  until (m >= last)
    {
      uword 		size  = m->Size & ~Memory_Size_BitMask;
      word 		alloc = m->Size & Memory_Size_FwdBit;
      Memory *	next  = (Memory *)((word)m + size);
      
      
      if (size > 0x08000000)
	{
	  /*
	   * any block this big is unreal
	   * this can be caused by the 'steal' program
	   */
	  
	  m = (Memory *)((word)m + ((uword)m->Size & ~15));
	  
	  continue;
	}
      
      if (!alloc)
	{
	  total += size;
	  
	  if (size > maxblock)
	    maxblock = size;
	}
#if defined(__TRAN) || defined(__C40) || defined(__ABC)
      if (m->Size & 4) 
	{
	  Carrier *	c = (Carrier *)(m + 1);
	  
	  
	  size     = c->Size;	
	  
	  fast_ram = true;
	}
      else
#endif
	{
	  fast_ram = false;
	}
      
      if (m->Pool == freepool &&  alloc)
	{
	  printf( "%p block is INCONSISTENT - free and alloc'ed\n", m );
	}
      
      if (m->Pool != freepool && !alloc)
	{
	  printf( "%p block is INCONSISTENT - not free but not alloc'ed\n", m );
	}
      
      if (( alloc && (next->Size & Memory_Size_BwdBit) == 0) ||
	  (!alloc && (next->Size & Memory_Size_BwdBit) != 0) )
	{
	  printf( "%p BWD bit of next wrong\n", m );
	}
      
      if (alloc && (*(word *)(m + 1) == T_Program))
	{
	  Program *	prog = (Program *)(m + 1);
	  
	  
	  blocks[ find_program_block( prog->Module.Name ) ].program += size;
	}
      else if (alloc && (*(word *)(m + 1) == T_Module))
	{
	  blocks[ 0 ].alloc += size;
	}
      else if (alloc && (*(word *)(m + 1) == T_Device))
	{
	  blocks[ 0 ].alloc += size;
	}
      else if (m->Pool == freepool)
	{
	  ;
	}
      else if (m->Pool == syspool)
	{
	  blocks[ 1 ].alloc += size;
	}
#if defined(__TRAN) || defined(__C40) || defined(__ABC)
else if (m->Pool == fastpool)
  {
    freefast += size;
  }
#endif
#ifdef __RRD
else if (m->Pool == rrdpool)
  {
    blocks[ 2 ].alloc += size;
  }
#endif
else
  {
    Task *		task  = (Task*)((int)(m->Pool) - offsetof( Task, MemPool ));
    ObjNode *	entry = (ObjNode *)task->TaskEntry;
    int 		block;
    
    block = find_block( entry->Name );
    if (*(word *)(m + 1) == (word)(m + 1))
      {
	blocks[ block ].module += size;
      }
    else
      {
	if (fast_ram)
	  {
	    blocks[ block ].fast += size;
	  }
	else
	  {
	    blocks[ block ].alloc += size;
	  }
      }
  }
      
      m = (Memory *)((word)m + ((uword)m->Size & ~15));
    }
  
  qsort( blocks, nextfree, sizeof( block ), compare );
  
  for (i = 0; i < nextfree; i++)
    {
      alloc   += blocks[ i ].alloc;
      fast    += blocks[ i ].fast;
      module  += blocks[ i ].module;
      program += blocks[ i ].program;
    }
  
  MachineName( (string)&machine_name );
  
  printf( "\nMemory statistics for processor %s\n\n", machine_name );
  
  if (output == Short)
    {
      printf( "Free memory available:  %12s\n", approximate( total ) );
      printf( "Largest free block:     %12s\n", approximate( maxblock ) );
#if defined(__TRAN) || defined(__C40) || defined(__ABC)
      printf( "Fast RAM available:     %12s\n", approximate( freefast ) );
#endif
      printf( "Total memory allocated: %12s\n", approximate( alloc + fast + module + program ) );
      printf( "Total memory in system: %12s\n",
	     approximate( alloc + fast + freefast + module + program + total ) );
      
      return true;
    }
  else
    {
      printf( "Free memory available:  %8lu\n", total );
      printf( "Largest free block:     %8lu\n", maxblock );
#if defined(__TRAN) || defined(__C40) || defined(__ABC)
      printf( "Fast RAM available:     %8u\n", freefast );
#endif
      printf( "Total memory allocated: %8lu\n", alloc + fast + module + program );
      printf( "Total memory in system: %8lu\n",
	     alloc + fast + freefast + module + program + total );
    }
  
  printf( "\n" );
  printf( "Allocated Memory :-\n" );
  printf( "\n" );
  printf( " Dynamic  Fast   Static     Code    Total  Owner\n" );
  printf( " -------  ----   ------     ----    -----  -----\n" );
  
  for (i = 0; i < nextfree; i++)
    {
      printf( " %7lu  %4lu  %7lu  %7lu  %7lu  %s\n",
	     blocks[ i ].alloc,
	     blocks[ i ].fast,
	     blocks[ i ].module,
	     blocks[ i ].program,
	     blocks[ i ].alloc + blocks[ i ].fast + blocks[ i ].module + blocks[ i ].program,
	     blocks[ i ].owner );
    }
  
  printf( "  ------  ----   ------   ------  -------\n" );
  
  printf( " %7lu  %4lu  %7lu  %7lu  %7lu\n",
	 alloc,  fast,  module,  program,
	 alloc + fast + module + program );
  
  
  free( blocks );
  
  return true;
  
} /* scan */

/*}}}*/
/*{{{  usage() */

void
usage( void )
/*
 * explain the usage of the program
 */
{
  fprintf( stderr, "usage: %s [-s]\n", ProgName );
  
} /* usage */

/*}}}*/
/*{{{  main() */

int
main(
	int	argc,
	char **	argv )
/*
 * start up and run world
 */
{
  char *	arg;
  Output	output = Normal;
  
  
  ProgName = argv[ 0 ];
  
  while (argc > 1)
    {
      arg = *++argv;
      --argc;
      
      if (arg[ 0 ] == '-')
	{
	  switch (arg[ 1 ])
	    {
	    default:
	      usage();
	      return -2;
	      
	    case 's':
	      output = Short;
	      break;
	      
	    case 'l':
	      output = Long;
	      break;
	      
	    case 'n':
	      output = Normal;
	      break;					
	    }
	}
      else
	{
	  usage();
	  
	  return -2;
	}
    }
  
  if (!scan( output ))
    {
      return -1;
    }
  else
    {
      return 0;
    }
  
} /* main */

/*}}}*/

/*}}}*/

/* end of free.c */
