#include <stdio.h>
#include <stdarg.h>
#include <posix.h>
#include <fcntl.h>
#include <stdlib.h>
#include <syslib.h>
#include <string.h>

#include <linkio.h>

#include <module.h>

#include "worm.h"

typedef struct processor {
	bool			valid;		/* is node ok ?	*/
	word			systype;	/* system type	*/
	char			name[32];	/* actual name	*/
	word			wormid;		/* worm id	*/
	WormInfo		worminfo;	/* worm info	*/
} processor;

#define SYS_HELIOS	1
#define SYS_IO		2

processor *procmap = NULL;	/* array of processors found by worm */
int pmapsize = 0;
int nprocs = 0;

#define _ARG_ if( *arg == 0 ) arg = *++argv;

#define WORM_PATH	"/helios/local/lib/worm.sa" 
#define DEF_PFORMAT	"%02d"
#define DEF_NETNAME	"Cluster"
#define DEF_RADRIVER	"tram_ra.d"
#define DEF_NETWORK	"network"
#define DEF_PROCESSOR	"processor"

char *pformat = DEF_PFORMAT ;
char *netname = DEF_NETNAME ;
char *radriver = DEF_RADRIVER ;
bool debug = FALSE;

int error(char *f,...)
{
	va_list a;
	
	va_start(a,f);

	fprintf(stderr,"Error: ");	
	vfprintf(stderr,f,a);
	
	putc( '\n', stderr );
	
	exit(1);
}

int warn(char *f,...)
{
	va_list a;
	
	va_start(a,f);

	fprintf(stderr,"Warning: ");	
	vfprintf(stderr,f,a);
	
	putc( '\n', stderr );
}

/* getproc							*/
/* return pointer to proc structure of processor 'id'. Extend	*/
/* array if necessary.						*/

processor *getproc(int id)
{
	if( id <0 || id > 1000 ) error("unexpected id %d",id);
	
	if( id >= pmapsize ) 
	{
		processor *newmap;
		int newsize = pmapsize;
		while( id >= newsize) newsize += 8;
		newmap = malloc(sizeof(processor)*newsize);
		if( newmap == NULL ) error("cannot allocate procmap");
		memset(newmap,0,sizeof(processor)*newsize);
		memcpy(newmap,procmap,sizeof(processor)*pmapsize);
		free(procmap);
		procmap = newmap;
		pmapsize = newsize;
	}
	return &procmap[id];
}

/* worm								*/
/* run the worm through the given link, collect its results and	*/
/* enter them into the processor array.				*/

void worm(int link)
{
	int token = 0;
	WormLoadData data;
	int e;
	processor *p;

	_LinkTimeout = OneSec*2;
	
	/* grab the link and boot the worm through it		*/
	
	if( !link_open(link) ) error("cannot open link %d",link);
		
	if( (e=link_boot(link, WORM_PATH))!=0 ) goto done;
	
	data.parentsid = 0;
	data.parentslink = link;
	data.myid = ++nprocs;
	data.mylink = Unattached;

	p = getproc(0);
	p->worminfo.linkarray[link].proc = nprocs;
	p->worminfo.linkarray[link].link = -1;

	if( debug ) printf("  Parent    Daughter\n Id  Link   Id  Link\n");
	
	/* send load data to new worm			*/
	
	link_out_word(link,LoadDataTag);	
	link_out_struct(link,data);

	_LinkTimeout = OneSec*10;
	
	/* collect results as they come			*/
	
	while( token != ReturnControl )
	{
		if( link_in_data(link,&token,4) != 0 ) goto done;

		switch( token )
		{
		case LoadDataTag:
			link_in_struct(link,data);
			link_out_word(link,Synchronise);
			if( data.parentsid == 0 ) 
				p->worminfo.linkarray[link].link = data.mylink;
			if( debug ) 
			{
				char *fmt = "%3d  %3d   %3d  %3d\n";
				printf(fmt,data);
			}
			break;
			
		case ReturnControl:
			link_in_data(link,&nprocs,4);
			break;
		}
	}
	
	/* once the worm has explored the network, get the connection	*/
	/* map and fill in processor array.				*/
	
	if( debug ) 
	{
		int l ;
		printf("connection map\n");
		printf ("id   type  mem_size ") ;
		for (l = 0 ; l < 4 ; l ++)
			printf ("  Link %d", l) ;
		putchar ('\n') ;
	}
		
	token = 0;
	while( token != NoMoreData )
	{
		WormInfo info;
		int id, i, l ;

		if( link_in_data(link,&token,4) != 0 ) goto done;
		if( token != NetworkData ) break;
		link_in_data(link,&id,4);
		link_in_struct(link,info);
		if( debug )
		{
			char *fmt = "%02d   %4d  %8x ";
			printf(fmt,id,info.type,info.memsize,info.memok) ;
			for (l = 0 ; l < 4 ; l ++)
			{
				if (info.linkarray[l].proc < 0)
					printf ("    ----") ;
				else
					printf ("    %02d-%1d", 
						info.linkarray[l].proc, 
						info.linkarray[l].link) ;
			}
			putchar ('\n') ;
		}

		p = getproc(id);			
		p->valid = TRUE;
		p->systype = SYS_HELIOS;
		sprintf(p->name,pformat,id);
		p->wormid = id;
		p->worminfo = info;
		for( i = 0; i < 4; i++ )
		{
			switch(info.linkarray[i].link)
			{
			case Unattached: 
				break;

			case TimeOut:
				warn("timeout error on link %d of processor %d in stage %d",
					i,id,info.linkarray[i].proc);
				break;
				
			case TokenError:
				warn("token error on link %d of processor %d in stage %d",
					i,id,info.linkarray[i].proc);
				break;
			default:
				break;
			}
		}		
		if( info.memok != 0 ) 
		{
			p->valid = FALSE;
			warn("memory check for processor %d failed in stage %d",
				id,info.memok);
		}
	}
done:
	link_close(link);
}

/* explore							*/
/* explore the given link					*/
/* At present this tries the worm on dumb links, but only looks	*/
/* at the next processor through intelligent links. We really	*/
/* need a Helios equivalent of the worm. (This is left as an	*/
/* exercise for the reader.)					*/

void explore(int link)
{
	LinkInfo info;
	word e;

	if( debug ) printf("Try link %d...",link);
		
	if( (e=LinkData(link,&info)) != Err_Null ) error("cannot explore link %d: %x",link,e);
	
	/* if the link is intelligent, locate the processor through it	*/
	/* otherwise run the worm.					*/
	
	if( info.Mode == Link_Mode_Intelligent && 
	    info.State == Link_State_Running )
	{
		Object *o;
		char lname[32];
		processor *pp, *p;
		bool ioproc = info.Flags & Link_Flags_ioproc;
		char *name;
		int i;

		sprintf(lname,"/link.%d",link);
		o = Locate(NULL,lname);
		
		if( o == NULL ) error("failed to locate nucleus through link %d",link);

		if( debug ) printf("%s\n",o->Name);

		name = o->Name+strlen(o->Name);
		while( *name != '/' ) name--;
		name++;
		
		p = getproc(++nprocs);

		p->valid = TRUE;
		p->systype = ioproc?SYS_IO:SYS_HELIOS;
		strcpy(p->name,name);
		p->wormid = nprocs;
		p->worminfo.type = ioproc?0:800; /* no way to get mc type remotely */
		p->worminfo.memsize = 0;
		p->worminfo.memok = 0;

		for( i = 0; i < 4; i++ )
		{
			p->worminfo.linkarray[i].proc = 0;
			p->worminfo.linkarray[i].link = Unattached;
		}
		p->worminfo.linkarray[0].proc = 0;
		p->worminfo.linkarray[0].link = link;
		pp = getproc(0);
		pp->worminfo.linkarray[link].proc = nprocs;
		pp->worminfo.linkarray[link].link = 0;

		Close(o);
	}
	else 
	{
		if( debug ) printf("sending in worm\n");
		worm(link);
	}
}

/* initmap							*/
/* initialise processor 0 which is always this processor	*/

void initmap(void)
{
	int i;
	processor *p;

	p = getproc(0);
	p->valid = TRUE;
	p->systype = SYS_HELIOS;	
	sprintf(p->name,pformat,0);
	p->wormid = 0;
	p->worminfo.type = MachineType();
	p->worminfo.memsize = 0;
	p->worminfo.memok = 0;
	for( i = 0; i < 4; i++ )
	{
		p->worminfo.linkarray[i].proc = 0;
		p->worminfo.linkarray[i].link = Unattached;
	}
}

/* rmgen							*/
/* From an initialized processor array, generate a Helios	*/
/* resource map.						*/

void rmgen(void)
{
	int i;
	int l;
	processor *p = getproc(0);
	
	printf("%s /%s {\n", DEF_NETWORK, netname);
#ifdef OLD_FORMAT
	printf("\tCONTROL Rst_Anl [/%s/%s];\n\n",netname,p->name);
#else
	printf("\tReset { driver; ; %s }\n\n", radriver) ;
#endif

	for( i = 0; i <= nprocs ; i++ )
	{
		char *type;
		p = getproc(i);
		if( p->valid )
		{
			int num_links = (p->systype == SYS_IO)?1:4;

			printf("\t%s %s {", DEF_PROCESSOR, p->name);

			for( l = 0; l < num_links ; l++ )
			{
				if( p->worminfo.linkarray[l].link >= 0 )
				{
					processor *neighbour = getproc(p->worminfo.linkarray[l].proc);
					if( neighbour->valid )
						printf("\t~%s",neighbour->name);
					else putchar('\t');
				}
				else putchar('\t');
				if( l == (num_links - 1) ) putchar(';');
				else putchar(',');
			}
			
#ifdef OLD_FORMAT
			putchar('\n');
			switch( p->systype )
			{
			case SYS_HELIOS	:	type = "HELIOS"; break;
			case SYS_IO	:	type = "IO"; break;
			}
			printf("\t\t%s;\n",type);
#else
			if ( p->systype == SYS_IO )
			{
				type = "IO";
				printf("\t%s;\n",type);
			}
			else
				putchar('\n');
#endif

#ifdef OLD_FORMAT
			if( p->worminfo.type != 0 )
				printf("\t\tptype\tT%d;\n",p->worminfo.type);
			
			if( p->worminfo.memsize != 0 )
				printf("--\t\tmemory\t%d;\n",p->worminfo.memsize);
				
			if( (i == 0) && (p->systype != SYS_IO) && (radriver != NULL) ) 
				printf("\t\tMnode\tRst_Anl [%s];\n",radriver);
#endif
				
			printf("\t}\n");
		}
	}
	printf("}\n");
}

static void help()
{
	printf(
	"usage: explore <options>\n"
	"-d             generate debug output\n"
	"-r name        set name of reset-analyse driver (default \"%s\")\n"
	"-n name        set network name (default \"%s\")\n"
	"-f format      set format for processor names (default \"%s\")\n"
	"-h             help!\n",
	DEF_RADRIVER, DEF_NETNAME, DEF_PFORMAT
	);
}

int main(int argc, char **argv)
{
		
	argv++;
	
	while( *argv )
	{
		char *arg = *argv;
		if( *arg == '-' )
		{
			arg++;
			switch( *arg++ )
			{
			case 'd':
				debug = TRUE;
				break;
				
			case 'r':
				_ARG_;
				radriver = arg;
				break;
				
			case 'n':
				_ARG_;
				netname = arg;
				break;
				
			case 'f':
				_ARG_;
				pformat = arg;
				break;
				
			case 'h':
				help();
				exit(0);
			}
		}
		argv++;
	}

	initmap();
	
	explore(0);
	explore(1);
	explore(2);
	explore(3);

	rmgen();
	
	argc=argc;
}
