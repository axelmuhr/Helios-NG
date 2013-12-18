
#include <helios.h>                     /* standard header              */
#include <syslib.h>
#include <root.h>
#include <config.h>                     /* for LinkConf                 */
#include <codes.h>                      /* for Err_Null                 */
#include <module.h>

#include <posix.h>
#include <fcntl.h>

#include <stdio.h>

extern word _LinkTimeout;

extern bool link_open (word linkno);
extern bool link_close(word linkno);
extern word link_boot (word linkno, char *file);

#define link_in_byte(linkno,b) LinkIn(1,linkno,&b,_LinkTimeout)
#define link_in_word(linkno,w) LinkIn(4,linkno,&w,_LinkTimeout)
#define link_in_data(linkno,buf,size) LinkIn(size,linkno,buf,_LinkTimeout)
#define link_in_struct(l,d) link_in_data(l,&(d),sizeof(d))

#define link_out_byte(linkno,b) { char __x = b; LinkOut(1,linkno,&__x,_LinkTimeout); }
#define link_out_word(linkno,w) { int __x = w; LinkOut(4,linkno,&__x,_LinkTimeout); }
#define link_out_data(linkno,buf,size) LinkOut(size,linkno,buf,_LinkTimeout)
#define link_out_struct(l,d) link_out_data(l,&(d),sizeof(d))


word _LinkTimeout = OneSec*2;

static LinkConf oldconf[4];

#define BUFSIZE	1024

extern bool link_open(word linkno)
{
        LinkInfo	linkinfo;
        LinkConf	c;
	RootStruct	*root = GetRoot();
	
	/* check linkno is valid and it is not already open */
	if( linkno < 0 || linkno > 3 || oldconf[linkno].Mode != 0 ) return FALSE;
	
        /* First save current link state */
        if( LinkData(linkno,&linkinfo) != Err_Null ) return FALSE;

        /* get the link into dumb mode */
        c.Id = linkno;                          /* link to change       */
        c.Mode = Link_Mode_Dumb;                /* new mode             */
        c.State = 0;                            /* unchanged            */
        c.Flags = 0;                            /* unchanged            */
        
        /* change mode */
        if( Configure(c) != Err_Null ) return FALSE;
        
        /* get control of the link */
        if( AllocLink(linkno) != Err_Null ) return FALSE;

	/* V1.1 bug fix... */
	root->Links[linkno]->TxUser = NULL;
	root->Links[linkno]->RxUser = NULL;

	/* save old conf, since first 4 fields of LinkInfo == LinkConf 	*/
	/* we can cheat a little here.					*/
	
        oldconf[linkno] = *(LinkConf *)&linkinfo;

	/* some links have initial mode of 0, fix this */
	if(oldconf[linkno].Mode == 0 ) oldconf[linkno].Mode = Link_Mode_Dumb;
	
        return TRUE;
}

extern bool link_close(word linkno)
{
	/* check linkno is valid and it is open */
	if( linkno < 0 || linkno > 3 || oldconf[linkno].Mode == 0 )
	{
		IOdebug("bad link %d %x",linkno,oldconf[linkno]);
		return FALSE;
	}

        /* relinquish control of the link */
        if( FreeLink(linkno) != Err_Null ) 
        {
        	IOdebug("free link failed");
        	return FALSE;
        }

        /* restore old state */
        if( Configure(oldconf[linkno]) != Err_Null )
        {
        	IOdebug("configure failed");
        	return FALSE;
        }

	oldconf[linkno].Mode = 0;
       
	return TRUE;
}

char buf[20000];

int main(int argc, char **argv)
{
	Stream *s;
	int size;
	int linkno;
	
	if( argc != 2 )
	{
		printf("usage: %s linkno\n",argv[0]);
		exit(1);
	}

	linkno = atoi(argv[1]);
	
	s = Open(CurrentDir,"/helios/lib/b5multi.b2",O_ReadOnly);
	
	if( s == NULL ) 
	{
		printf("Cannot open multi\n");
		exit(1);
	}
	
	size = Read(s,buf,20000,-1);
	
	if( size <= 0 )
	{
		printf("Cannot read multi %d %x\n",errno,Result2(s));
		exit(1);		
	}
	
	if( !link_open(linkno) )
	{
		printf("cannot open link\n");
		exit(1);
	}
	
	if(link_out_data(linkno,buf,size) != 0 ) 
		printf("cannot write data to link\n");
	
	link_close(linkno);
}
