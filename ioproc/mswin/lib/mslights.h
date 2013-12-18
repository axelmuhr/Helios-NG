#define WNAME "MsLights"
#define WSTYLE (WS_OVERLAPPED | WS_BORDER | WS_CAPTION | WS_SYSMENU)
#define DisplayFont ANSI_VAR

#ifdef __C40
   #define NLINKS 7	/* 7 links incase processor has shared memory link back to host */
#else
   #define NLINKS 4
#endif

#define border 		2
#define	bar_height 	2
#define	loadbar_y	border + bar_height/2
#define	linkbar_y	loadbar_y + bar_height + border
#define	membar_y	linkbar_y + bar_height + border
#define getinfoptr(a,b) ((Proc_Info *)(a + (IOCDataMax*b)))

 
typedef struct NewProcStats {
	word	Type;			/* processor type		*/
	word	Load;			/* usec/process av. cpu usage	*/
	word	Latency;		/* average interrupt latency	*/
	word	MaxLatency;		/* max interrupt latency seen	*/
	word	MemMax;			/* memory available on processor*/
	word	MemFree;		/* amount currently free	*/
	word	LocalTraffic;		/* bytes sent in local messages	*/
	word	Buffered;		/* data buffered by kernel	*/
	word	Errors;			/* number of errors raised	*/
	word	Events;			/* number of events		*/
	word	NLinks;			/* number of links following	*/
	struct {
		LinkConf	Conf;	/* link configuration		*/
		word		In;	/* bytes transmitted thru link	*/
		word		Out;	/* bytes received through link	*/
		word		Lost;	/* messages lost or sunk	*/
	} Link[NLINKS];			/* for each link		*/
	char	Name[Variable];		/* current network name		*/
					/* note that the exact position */
					/* of the Name field depends on	*/
					/* the value of NLinks.		*/
} NewProcStats;

typedef struct Proc_Info {
	char            *Name;
	Object          *Proc;
	int             LoadBar;
	int             LinkBar;
	int             MemBar;
	BYTE		OldInfo[IOCDataMax];
} Proc_Info;
   
void    	bar( HDC, int, int, HPEN, int, int);
int    		newbar( int, int);
void    	init_info( void );
void    	draw_bars(int i);
Proc_Info* 	CreateProcTable( int no_processors);
int 		InitProcTable(int position, char * proc_name);
static	int  	NetworkWalk(RmProcessor Processor, ...);

