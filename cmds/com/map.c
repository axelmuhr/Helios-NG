static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/com/RCS/map.c,v 1.28 1993/11/24 14:14:38 nickc Exp $";

#define in_kernel 1	/* trick root.h into letting us define PTE	*/
#include <stdio.h>
#include <nonansi.h>
#include <attrib.h>
#include <memory.h>
#include <ctype.h>
#include <root.h>
#include <posix.h>
#include <stdlib.h>
#include <task.h>
#include <stddef.h>
#include <servlib.h>
#include <module.h>
#include <link.h>
#include <string.h>
#include <syslib.h>
#include <process.h>	/* for System() */


typedef struct PTE {
	byte		Type;
	byte		Link;
	byte		Cycle;
	byte		Age;
	word		Owner;
	word		TxId;
	word		RxId;
} PTE;

#if defined __C40 || defined __ARM
/* dont use internal kernel GetRoot() just because we have defined in_kernel */
# define GetRoot() ((RootStruct *)GetRootBase())
#endif

#define	namesize	12
#define	mapmargin	2
#define loadx		7
#if defined __ARM && defined IDLEMON
#define loadmax		100000
#else
#define loadmax		2000
#endif

#if defined(__ARM) && defined (__MI)
# define FirstPoolBlock(pool) ((Memory *)((word)(pool + 1) + ((sizeof(Pool) + sizeof(MIInfo) + 15) & ~15)))
#else
# define FirstPoolBlock(pool) ((Memory *)(pool + 1))
#endif

int screenx, screeny;
int mapx, mapy;
int mapsize;
int physx, physy;
int poolx;
int flash = 0;
int mode;

char spin[5] = "-\\|/";

int mainbase;
int maintop;

char machine[100];

Attributes ostate, nstate;

RootStruct *root;
Pool *pool;
int memmax;
int grain;
int allocsize;
int freesize;
int oldallocsize;
int oldfreesize;

int loadaverage = 0;
int loadsize;
int maxload;
#if defined(__ARM) && defined(IDLEMON)
int maxidle;
int idleaverage = 0;
#endif
int gettime = (int)OneSec/4;

char *memmap[2];
int mapno;

typedef struct PoolInfo {
	bool	valid;
	Pool	*pool;
	MPtr	prog;
	bool	referenced;
	bool	displayed;
} PoolInfo;

PoolInfo *poolmap;
Pool *syspool;
Pool *fastpool;
Pool *loaderpool;
Pool *freepool;
Pool *rrdpool;

void ansiopen()
{
	GetAttributes(Heliosno(stdin),&ostate);
	nstate = ostate;
	AddAttribute(&nstate,ConsoleRawInput);
	AddAttribute(&nstate, ConsoleRawOutput);
	RemoveAttribute(&nstate, ConsolePause);
	RemoveAttribute(&nstate, ConsoleIgnoreBreak);
	RemoveAttribute(&nstate, ConsoleBreakInterrupt);
	RemoveAttribute(&nstate, ConsoleEcho);
	SetAttributes(Heliosno(stdin),&nstate);
}

void ansiclose()
{
	SetAttributes(Heliosno(stdin),&ostate);
}

void putch(int c) 
{
  putchar( c ); 

  if ( isprint( c ) )
    physx++;

  return;  
}

void ansiparm(int    n)
{
        register int q,r;

        q = n/10;
        if (q != 0) {
		r = q/10;
		if (r != 0) {
			putch((r%10)+'0');
		}
		putch((q%10) + '0');
        }
        putch((n%10) + '0');
}

void poscurs(int x, int y)
{
	if( physx == x && physy == y ) return;
	fflush(stdout);
	putch(0x1b);
	putch('[');
	ansiparm(y+1);
	putch(';');
	ansiparm(x+1);
	putch('H');
	fflush(stdout);
	physx = x; 
	physy = y;
}

int getch(int timeout)
{
	char c = 0;
	Read(fdstream(0),&c,1,timeout);
	return c;
}


void clrscrn()
{
	putch(12);	
	physx = physy = 0;
}

void getxy()
{
	Attributes state;
	GetAttributes(Heliosno(stdin),&state);
	screenx = state.Time;
#if defined  __ARM && defined IDLEMON
	screeny = state.Min - 1;
#else
	screeny = state.Min;
#endif
}

void initpoolmap()
{
	int i;
	

	for( i = 0; i < mapy ; i++ )
	{
		poolmap[i].valid = FALSE;
		poolmap[i].pool = NULL;
		poolmap[i].prog = NULL;
		poolmap[i].referenced = FALSE;
		poolmap[i].displayed = FALSE;
	}

	freepool = root->FreePool;	
	syspool = &root->SysPool;
	fastpool = &root->FastPool;
	loaderpool = root->LoaderPool;
#ifdef __RRD
	rrdpool = root->RRDPool;
#endif
	poolx = mapmargin + mapx+1;
}

char mappool(Memory *m)
{
	Pool *pool = m->Pool;
	int i;

	if( pool == freepool ) return '.';
	if( pool == syspool ) return '#';
	if( pool == fastpool ) return '$';
#ifdef __RRD
	if( pool == rrdpool ) return '+';
#endif	
	for( i = 0 ; i < mapy ; i++ )
	{
		unless( poolmap[i].valid ) continue;
		if( poolmap[i].pool == pool ) 
		{ poolmap[i].referenced = TRUE; return 'A' + i; }
		if( poolmap[i].prog == CtoM_(m+1) ) 
		{ return 'a' + i; }
	}

	if( pool == loaderpool ) 
	{
		if( m->Size & Memory_Size_Carrier )
		{
			return '@';
		}
		else
		{
			Module *mod = (Module *)(m+1);
			if( mod->Type == T_Module && mod->Id < 10 )
					return (char)('0'+mod->Id);
			return '@';
		}
	}

	if( m->Size & Memory_Size_Carrier )
	{
		return '-';
	}

	/* unknown pool */
	for( i = 0 ; i < mapy ; i++ )
	{
		unless( poolmap[i].valid )
		{
			poolmap[i].valid = TRUE;
			poolmap[i].pool = pool;
			poolmap[i].prog = NULL;
			poolmap[i].referenced = TRUE;
			poolmap[i].displayed = FALSE;
			return 'A' + i;
		}
	}

	return '?';
		
}

void showpools()
{
	int i;
	for( i = 0 ; i < mapy ; i++ )
	{
		if( poolmap[i].valid && !poolmap[i].referenced )
			poolmap[i].valid = FALSE;

		poolmap[i].referenced = FALSE;
			
		unless( poolmap[i].valid )
		{
			if( poolmap[i].displayed )
			{
				int j;
				poscurs(poolx, mapmargin + i);
				for( j = 1; j < namesize + 3 ; j++ ) putch(' ');
				poolmap[i].prog = NULL;
				poolmap[i].displayed = FALSE;
			}
			continue;
		}
		
		if( poolmap[i].valid && !poolmap[i].displayed )
		{
			Task *task = (Task*)((int)(poolmap[i].pool) - offsetof(Task,MemPool));
			ObjNode *entry = (ObjNode *)task->TaskEntry;
			
			poolmap[i].prog = task->Program;
			
			poscurs(poolx, mapmargin + i);
#if 0
			printf("%c %12s",'A'+i,entry->Name);
#else
			printf("%c ",'A'+i);
			{
				int xx;
				char *cp = (char *)&entry->Name;

	/* MJT - On startup, map gets a little confused if other tasks */
	/* are appearing and disappearing beneath it.                  */
	/* So, have a quick check to see if address of Name is valid.  */
	/* If not, print <unknown>. Even if the address is valid, the  */
	/* data pointed to by it may not be, but such is life          */

				if(cp < (char *)root->FreePool ||
				   cp > (char *)root->FreePool->Memory.Head)
						cp = "<unknown>";

				for (xx=0;xx<12;xx++)
				{
					if (*cp == '\0')
						break;
					putchar(*cp < ' '? ' ' : *cp);
					cp++;
				}
			}
#endif
			poolmap[i].displayed = TRUE;
		}
	}
}

int showload(int av, int oldav, int *max, int lx, int ly, int lsize, int lmax)
{
	int diff;
	int i;
		
	av = av * lsize / lmax;

	for( i = 2 ; av > (lsize - lsize/i) && (lsize/i > 0) ; i = i*2 )
	{
		int ls = lsize - lsize/i;
		av = ls + (av - ls)/i;
	}
	
	if( av >= lsize ) av = lsize-1;
		
	diff = av - oldav;
	
	if( diff < 0 )
	{
		poscurs(lx+av,ly);
		for( i = av; i < oldav; i++ ) putch(' ');
	}
	elif( diff > 0 )
	{
		poscurs(lx+oldav,ly);
		for( i = oldav; i < av; i++ ) putch('=');
	}
	if( max != NULL && av > *max ) 
	{
		poscurs(lx+av,ly);
		putch('|');
		*max = av;
	}

	return av;
}

void initmap(void)
{
	char *map0 = memmap[0];
	char *map1 = memmap[1];
	int i;
	
	mapno = 0;
	
	for( i = 0; i < mapsize; i++ ) map0[i] = ' ', map1[i] = 'z';
}

void getmem(void)
{
	Memory *m = (Memory *)(pool+1);
	char *newmap = memmap[(mapno+1)&1];
	
	allocsize = 0;
	freesize = 0;
	
	until( m == (Memory *)pool->Memory.Head )
	{
		int size  = (int)(m->Size & 0xfffffff0);
		int alloc = (int)(m->Size & Memory_Size_FwdBit);
		int start = (int)m - (int)(pool+1);
		int end   = start + size;
		int i;
		char c = mappool(m);
		
		if( alloc ) allocsize += size;
		else freesize += size;
						
		/* adjust start and end to grain boundaries */
		start = start - (start % grain);
		end = end + grain - 1;
		end = end - (end % grain);
	
		start /= grain;
		end /= grain;

		for( i = start; i <= end; i++ ) newmap[i] = c;
			
		m = (Memory *)((int)m + size);
	}
}

#if defined(__TRAN)

#include <asm.h>

typedef word SaveState;

#define P_NullState	((SaveState *)MinInt)
#define NullStateP(p)	((p) == (SaveState *)MinInt)

#define P_InstPtr(p)	((VoidFnPtr)  ((p)[-1]))
#define P_RunqNext(p)	((SaveState *)((p)[-2]))
#define P_BufAddr(p)	((byte *)     ((p)[-3]))
#define P_TimerNext(p)	((SaveState *)((p)[-4]))
#define P_EndTime(p)	((word)       ((p)[-5]))

#define RunqPtrs(x)	savel_(x);

#define MaxAct	64

struct
{
	int	pc;
	int	w;
} active[MaxAct];

Semaphore actlock;
int actpos;

void monitor(void)
{
	for(;;)
	{
		struct { SaveState *head, *tail; } runq;
		SaveState *p;

		Wait(&actlock);		
	
		RunqPtrs(&runq);
	
		p = runq.head;

		if( !NullStateP(p) )
		{
			SaveState *pp;
			
			/* The following loop must NOT contain an unconditional	*/
			/* jump to prevent it being timesliced.			*/
		again:
			if( ((int)p < mainbase || (int)p > maintop) &&
			    actpos < MaxAct )
			{
				active[actpos].pc = (int)P_InstPtr(p);
				active[actpos].w = (int)p;
				actpos++;
			}
			pp = p;
			p = P_RunqNext(p);
			if( pp != runq.tail ) goto again;
			
		}

		Signal(&actlock);
		
		Delay(gettime/50);
	}
}

void showactive(void)
{
	char *newmap = memmap[(mapno+1)&1];
	int i;
	
	Wait(&actlock);
	
	for( i = 0; i < actpos; i++ )
	{
		int pc = active[i].pc;
		int w = active[i].w;		
		if( pc > (int)pool ) newmap[(pc - (int)pool)/grain] = '!';
		if( w > (int)pool )  newmap[(w  - (int)pool)/grain] = '*';
	}

	actpos = 0;

	Signal(&actlock);
}

#endif

void updatemap(void)
{
	char *oldmap = memmap[mapno];
	char *newmap = memmap[(mapno+1)&1];
	int i;
	
	if( allocsize != oldallocsize ) 
	{ 
		poscurs(10,0); 
		printf("%9d",allocsize); 
		oldallocsize = allocsize;
	}
	if( freesize != oldfreesize ) 
	{ 
		poscurs(25,0); 
		printf("%9d",freesize); 
		oldfreesize = freesize;
	}
	
	for( i = 0; i < mapsize ; i++ )
	{
		if( newmap[i] != oldmap[i] ) 
		{
			poscurs( mapmargin + i % mapx, mapmargin + i / mapx );
			putch(newmap[i]);
		}
	}
	
	mapno = (mapno+1)&1;
}	


int displaymem()	
{
	int c;
	
	getxy();

	clrscrn();

	pool = root->FreePool;
	
	memmax = (int)(pool->Memory.Head) - (int)pool; 

	mapx = screenx - 2*mapmargin - 2 - namesize;
	mapy = screeny - 2*mapmargin;

	mapsize = mapx * mapy;		/* size of map			*/

	grain = memmax/mapsize + 1;	/* bytes per map pos		*/

	loadsize = screenx - loadx;
	loadaverage = 0;
#if defined(__ARM) && defined(IDLEMON)
	idleaverage = 0;
	maxidle = 0;
#endif
	maxload = 0;
			
	oldfreesize = 0;
	oldallocsize = 0;
	
	memmap[0] = (char *)malloc(mapsize+10);
	memmap[1] = (char *)malloc(mapsize+10);

	poolmap = (PoolInfo *) malloc(mapy*sizeof(PoolInfo));

	initmap();
	initpoolmap();

	poscurs(0,0); 		printf("Allocated");
	poscurs(20,0); 		printf("Free");
	poscurs(35,0); 		printf("Grain %d",grain);
	poscurs((4+mapx-strlen(machine))/2,1);
		 		printf("%s",machine);
#if defined(__ARM) && defined(IDLEMON)
	poscurs(0,screeny-1); 	printf(" Idle: ");
#endif
	poscurs(0,screeny); 	printf(" Load: ");

	forever
	{
#if 0 /*def __C40 -- may take too long for this */
		/* safely read memory block layout */
		System( (WordFnPtr)getmem);
#else
		getmem();		/* read memory block layout	*/
#endif
		
#if defined(__TRAN)
		showactive();		/* add active stack/PCs		*/
#endif
		updatemap();		/* redraw map			*/

		showpools();		/* display ownership info	*/

#if defined(__ARM) && defined(IDLEMON)
/* idle monitor                             NULL = none  start           max bar    */
/* newoldav              target     oldav     peak bar  x     y            length   =100% */
idleaverage = showload(root->Idle,idleaverage, &maxidle,loadx,screeny-1,loadsize,100);
#endif
/* load average */
loadaverage = showload((int)(root->LoadAverage),loadaverage,&maxload,loadx,screeny,loadsize,loadmax);
		
		poscurs(0,screeny);
		flash++;
		putch( spin[flash&0x3] );
		poscurs(0,screeny);	

		fflush(stdout);

		c = getch(gettime);
		
		  if( c == '-' ) gettime *= 2;
		elif( c == '+' ) gettime /= 2;
		elif( c != 0 ) break;

	} 
	
	free(memmap[0]);
	free(memmap[1]);
	free(poolmap);

	return c;
}

#if defined __HELIOSTRAN

#define MAX_NUM_LINKS	4
#define ly( i )		(i * 4 + 3)
#define IN_X		0
#define IN_Y		1
#define OUT_X		0
#define OUT_Y		2
#define MAX_WIDTH	screenx

#elif defined __C40

#define MAX_NUM_LINKS	7
#define ly( i )		(i * link_spacing + 3)
#define IN_X		0
#define IN_Y		1
#define OUT_X		(screenx/2)
#define OUT_Y		1
#define MAX_WIDTH	(screenx/2)

#elif defined __ARM

#define MAX_NUM_LINKS	4
#define ly( i )		(i * 4 + 3)
#define IN_X		0
#define IN_Y		1
#define OUT_X		0
#define OUT_Y		2
#define MAX_WIDTH	screenx

#else

#error Unknown Number of Links

#endif

int
displaylinks( void )
{
  LinkInfo **	lv = root->Links;
  int		ln;
  int		localloadmax;
  int		localload;
  int		oldlocalload;
  int		i;
  int		c;
  int		inload[     MAX_NUM_LINKS ];
  int		outload[    MAX_NUM_LINKS ];
  int		oldinload[  MAX_NUM_LINKS ];
  int		oldoutload[ MAX_NUM_LINKS ];
  int		inmax[      MAX_NUM_LINKS ];
  int		outmax[     MAX_NUM_LINKS ];
  int		mode[       MAX_NUM_LINKS ];
  int		state[      MAX_NUM_LINKS ];
  int		id[         MAX_NUM_LINKS ];
  int		link_spacing = 3;  
  int		numlinks;
  
  getxy();
  
  clrscrn();
  
  poscurs( (screenx - strlen( machine )) / 2, 1 );
  
  printf( "%s", machine );

	/* Work out the actual number of links, which may be less than	*/
	/* the maximum.							*/
  for (i = 0; lv[ i ] != NULL; i++)
   { 
      if (i >= MAX_NUM_LINKS)
       { fprintf(stderr, "\r\n\nMap: too many links.\r\n");
	 return 'q';
       }
   }
  numlinks = i;
	/* Adjust the spacing between links if necessary.		*/
  if (numlinks > 6) link_spacing = 2;

  for( i = 0; lv[ i ] != NULL; i++) 
    {
      oldinload[  i ] = (int)(lv[ i ]->MsgsIn);
      oldoutload[ i ] = (int)(lv[ i ]->MsgsOut);
      inload[     i ] = outload[  i ] = 0;
      inmax[      i ] = outmax[   i ] = 0;
      id[         i ] = -1;
      mode[       i ] = state[ i ] = -1;
      
      poscurs( IN_X,  ly( i ) + IN_Y  );	printf("In : ");
      poscurs( OUT_X, ly( i ) + OUT_Y );	printf("Out: ");
    }
  
  localload    = 0;
  oldlocalload = (int)(root->LocalMsgs);
  localloadmax = 0;
  
  poscurs( 0, ly( numlinks ) );       printf("Local Message Traffic");
  
  loadsize    = screenx - loadx;
  loadaverage = 0;
  
#if defined(__ARM) && defined(IDLEMON)
  idleaverage = 0;
  maxidle     = 0;
#endif
  
  maxload     = 0;
  
#if defined(__ARM) && defined(IDLEMON)
  poscurs( 0, screeny - 1 ); 	printf( " Idle: " );
#endif
  
  poscurs( 0, screeny ); 	printf( " Load: " );
  
  forever
    {	
      for(ln = 0; lv[ ln ] != NULL; ln++ )
	{
	  LinkInfo *	link = lv[ ln ];
	  
	  
	  if ( link->Id != id[ ln ] )
	    {
	      id[ ln ] = link->Id;
	      
	      poscurs( 0, ly( ln ) ); printf( "Link: %2d ", id[ ln ] );
	    }
	  
	  if ( link->Mode != mode[ ln ] )
	    {
	      char *	s;

	      
	      mode[ ln ] = link->Mode;
	      
	      switch ( mode[ ln ] )
		{
		case Link_Mode_Dumb:	    s = "   Dumb    "; break;
		case Link_Mode_Intelligent: s = "Intelligent"; break;
		case 0:			    s = "   Null    "; break;
		default:		    s = "   ????    "; break;
		}
	      
	      poscurs( 9, ly( ln ) ); printf( "Mode: %s ", s );
	    }
	  
	  if ( link->State != state[ ln ] )
	    {
	      char *	s;

	      
	      state[ ln ] = link->State;
	      
	      switch( state[ ln ] )
		{
		case Link_State_Booting:  s = "Booting";  break;
		case Link_State_Dumb:	  s = "Dumb";     break;
		case Link_State_Running:  s = "Running";  break;
		case Link_State_Timedout: s = "Timedout"; break;
		case Link_State_Crashed:  s = "Crashed";  break;
		case Link_State_Dead:	  s = "Dead";     break;
		default:                  s = "????";     break;
		}
	      
	      poscurs( 27, ly( ln ) ); printf( "State: %s", s );
	    }
	  
	  inload[ ln ] = showload( (int)(link->MsgsIn) - oldinload[ ln ],
				  inload[ ln ],
				  &inmax[ ln ],
				  IN_X + 5,
				  ly( ln ) + IN_Y,
				  MAX_WIDTH - 5,
				  10000 );
	  
	  oldinload[ln] = (int)(link->MsgsIn);			
	  
	  outload[ln] = showload( (int)(link->MsgsOut) - oldoutload[ ln ],
				 outload[ ln ],
				 &outmax[ ln ],
				 OUT_X + 5,
				 ly( ln ) + OUT_Y,
				 MAX_WIDTH - 5,
				 10000 );
	  
	  oldoutload[ln] = (int)(link->MsgsOut);
	}
      
      localload = showload( (int)(root->LocalMsgs) - oldlocalload,
			   localload,
			   &localloadmax,
			   0,
			   ly( numlinks ) + 1,
			   screenx,
			   10000 );
      
      oldlocalload = (int)(root->LocalMsgs);
      
#if defined(__ARM) && defined(IDLEMON)
      /* idle monitor */
      /* newoldav              target     oldav     peak bar  x     y      length   =100% */
      idleaverage = showload(root->Idle,idleaverage, &maxidle,    loadx,screeny-1,loadsize,100);
#endif
      
      loadaverage = showload( (int)(root->LoadAverage),
			     loadaverage,
			     &maxload,
			     loadx,
			     screeny,
			     loadsize,
			     loadmax );		
      
      poscurs( 0, screeny );
      
      flash++;
      
      putch( spin[ flash & 0x3 ] );
      
      poscurs( 0, screeny );	
      
      fflush( stdout );
      
      c = getch( gettime );
      
        if ( c == '+' ) gettime *= 2;
      elif ( c == '-' ) gettime /= 2;
      elif ( c != 0   ) break;
    }
  
  return c;
}

void getports(char *map)
{
	int x,y;

	allocsize = 0;
	freesize = 0;
		
	for( y = 0; root->PortTable[y] != (PTE *)MinInt; y++ )
	{
		PTE *tab = root->PortTable[y];
		
		if(tab == NULL) break;			/* end of table */

		for( x = 0; x < 64; x++ )
		{
			char type = '.';
			if( mode == 1 )
			{
				switch( tab[x].Type )
				{
				case T_Free:      type = '.'; break;
				case T_Local:	  type = 'L'; break;
				case T_Surrogate: type = 'S'; break;
				case T_Trail:     type = 'T'; break;
				case T_Permanent: type = 'P'; break;
				default:          type = '?'; break;
				}		
				if( type == '.' ) freesize++;
				else allocsize++;
			}
			elif( mode == 2 )
			{
				if( tab[x].Type != T_Free )
				{
					if( tab[x].Type == T_Local && 
					    tab[x].TxId != NULL ) type = 'T';
					elif( tab[x].RxId != NULL ) type = 'R';
					else type = '.';
					allocsize++;
				}
				else freesize++;
			}
			elif( mode == 3 )
			{
				if( tab[x].Type == T_Free ) freesize++,type = '.';
				else 
				{
					allocsize++;
					if( tab[x].Age > 9 ) type = 'O';
					else type = tab[x].Age+'0';
				}
			}
			
			map[y*mapx+x] = type;
		}
	}
}

void showports(char **pmap)
{
	char *oldmap = pmap[mapno];
	char *newmap = pmap[(mapno+1)&1];
	int i;
	
	if( allocsize != oldallocsize ) 
	{ 
		poscurs(10,0); 
		printf("%9d",allocsize); 
		oldallocsize = allocsize;
	}
	if( freesize != oldfreesize ) 
	{ 
		poscurs(25,0); 
		printf("%9d",freesize); 
		oldfreesize = freesize;
	}
	
	for( i = 0; i < mapsize ; i++ )
	{
		if( newmap[i] != oldmap[i] ) 
		{
			poscurs( mapmargin + i % mapx, mapmargin + i / mapx );
			putch(newmap[i]);
		}
	}
	
}	

int displayports()
{
	char *pmap[2];
	int i, c;
		
	getxy();
	
	clrscrn();
	
	mapx = 64;
	mapy = (int)(root->PTSize/4);
	
	mapsize = mapx*mapy;
	
	loadsize = screenx - loadx;
	loadaverage = 0;
#if defined(__ARM) && defined(IDLEMON)
	idleaverage = 0;
	maxidle = 0;
#endif
	maxload = 0;
			
	oldfreesize = 0;
	oldallocsize = 0;
	
	pmap[0] = (char *)malloc(mapsize+10);
	pmap[1] = (char *)malloc(mapsize+10);

	for( i = 0; i < mapsize; i++ ) pmap[0][i] = ' ', pmap[1][i] = ' ';
	
	mapno = 1;

	poscurs(0,0); 		printf("Allocated");
	poscurs(20,0); 		printf("Free");
	poscurs((4+mapx-strlen(machine))/2,1);
		 		printf("%s",machine);
#if defined(__ARM) && defined(IDLEMON)
	poscurs(0,screeny-1); 	printf(" Idle: ");
#endif
	poscurs(0,screeny); 	printf(" Load: ");
		
	mode = 1;
	
	forever
	{
		getports(pmap[(mapno+1)&1]);
		
		showports(pmap);
		
#if defined(__ARM) && defined(IDLEMON)
		/* idle monitor */
		/* newoldav              target     oldav     peak bar  x     y      length   =100% */
		idleaverage = showload(root->Idle,idleaverage, &maxidle,loadx,screeny-1,loadsize,100);
#endif
		loadaverage = showload( (int)(root->LoadAverage), loadaverage,&maxload,loadx,screeny,loadsize,loadmax);	

		poscurs(0,screeny);
		flash++;
		putch( spin[flash&0x3] );
		poscurs(0,screeny);

		fflush(stdout);
		
		c = getch(gettime);

		  if( c == '+' ) gettime *= 2;
		elif( c == '-' ) gettime /= 2;
		elif( c == 'w' ) mode = 2;
		elif( c == 't' ) mode = 1;
		elif( c == 'a' ) mode = 3;
		elif( c != 0 ) break;

		mapno = (mapno+1)&1;
	}

	free(pmap[0]);
	free(pmap[1]);		
	
	return c;
}
	
char *mainhelp[] = 
{
	"             Map V1.2",
	"             ========",
	"",
	"    Command          Action",
	"",
	"    m                Display memory",
	"    l                Display links",
	"    p                Display ports",
	"",
	"    q,Q,ESC          Quit",
	"    h,H,?            Help",
	"    -                Halve sample rate",
	"    +                Double sample rate",
	"    Any other key    Resize/Redraw",
	0
};

char *memhelp[] =
{
	"               Memory Display",
	"               ==============",
	"",
	"Top Row	Total bytes allocated and free",
	"                     Grain = bytes/char in map",
	"",
	"Next Row	Shows processor name",
	"",
	"Main Map	Shows allocation of system heap",
	"                     Upper case letters show data areas",
	"                     Lower case letters show code",
	"                     Digits show shared libraries,",
	"                     .   = Free",
	"                     #   = System",
#ifdef __RRD
	"                     +   = Ram Disk",
#endif
	"                     @,? = Unidentified",
	"                     !,* = Active PCs and stacks",
	"",
	"Screen Right   Shows Tasks currently running",
	"                     letters match with main map",
	"",
	"Bottom Line    Shows current processor load/maximum",
	0
};

char *linkhelp[] =
{
	"             Link Display",
	"             ============",
	"",
	"First Line     Shows processor name",
	"",
	"For each Link:",
	"",
	"First line     Shows link number, mode and state",
	"Second line    Shows link input traffic/maximum",
	"Third line     Shows link output traffic/maximum",
	0
};

char *porthelp[] = 
{
	"             Port Display",
	"             ============",
	"",
	"Top Line       Shows ports allocated and free",
	"",
	"Next Line      Shows processor name",
	"",
	"Main Map       Shows state of each port in table:",
	"               .     Free",
	"               L     Local",
	"               S     Surrogate",
	"               T     Trail",
	"               P     Permanent",
	"               ?     Unknown",
	"",
	"Bottom Line    Shows current processor load/maximum",
	0
};

int
puthelp( char ** help )
{
  int i;
  int helpsize;
  int maxlen = 0;
  int len;
  int y;
  int x;
  int c;
  
  
  for (helpsize = 0; help[ helpsize ] != NULL ; helpsize++ )
    if ((len = strlen(help[ helpsize ])) > maxlen ) maxlen = len;
  
  y = (screeny - helpsize) / 2;
  x = (screenx - maxlen)   / 2;
  
  if ( y < 0 ) y = 0;
  if ( x < 0 ) x = 0;
  
  clrscrn();
  
  for ( i = 0; i < helpsize ; i++ ) 
    {
      poscurs( x, y + i );
      
      printf( help[ i ] );
    }
  
  poscurs( 0, screeny );
  
  fflush( stdout );
  
  while ( (c = getch((int)(OneSec * 10))) == 0 )
    ;
  
  return c;

} /* puthelp */

int
main(
     int	argc,
     char **	argv )
{
  int 		c    = 'h';
  int 		oldc = c;
  char **	help = mainhelp;
  int		vsp[ 3 ];
  

  if (argc >= 2)
    {
      if (argc == 2 && argv[ 1 ][ 0 ] == '-')
	{
	  oldc = c = argv[ 1 ][ 1 ];
	}
      else
	{
	  fprintf( stderr, "usage: map [-m|-l|-p]\n" );
      
	  return EXIT_FAILURE;
	}
    }
  
  mainbase = (int)&vsp;
  maintop  = (int)&oldc;
  
  root = GetRoot();
  
  MachineName( machine );
  
  ansiopen();
  
  getxy();
  
#ifdef __TRAN
  InitSemaphore( &actlock, 1 );
  
  actpos = 0;
  
  Fork( 2000, monitor, 0 );
#endif
  
  forever
    {
      switch( c )
	{
	case 'm':
	  oldc = c;
	  help = memhelp;			
	  c    = displaymem();
	  break;

	case 'l':
	  oldc = c;
	  help = linkhelp;
	  c    = displaylinks();
	  break;
	  
	case 'p':
	  oldc = c;
	  help = porthelp;
	  c    = displayports();
	  break;
	  
	case 'Q':
	case 'q':
	case 0x1B: 
	case 0x3:
	  goto done;
	  
	case '?':
	case 'h':
	case 'H':
	  c = puthelp(help);

	  if ( c == '\r' )
	    c = oldc;
	  else
	    oldc = c;
	  
	  help = mainhelp;
	  break;
	  
	default:
	  c    = oldc;
	  oldc = 'h';
	  break;
	}
    }
  
 done:
  poscurs( 0, screeny );

  ansiclose();

  putchar( '\n' );

  exit( EXIT_SUCCESS );

  return 0;

  rcsid = rcsid;	

} /* main */
