
#include <root.h>
#include <link.h>
#include <posix.h>
#include <linkio.h>
#include <module.h>
#include <fcntl.h>

word _LinkTimeout = OneSec*2;

static LinkConf oldconf[4];

#define BUFSIZE	1024

extern word link_boot(word link, char *sysfile)
{	
	int fd;
	char bootcmd = 4;
	int size;
	static byte ibuf[BUFSIZE];
	ImageHdr bhdr;
	
	fd = open("/helios/lib/nboot.i",O_RDONLY);

	if( fd == -1 ) return 1;
	
	read(fd,(byte*)&bhdr,sizeof(ImageHdr));
		
	if( read(fd,ibuf,bhdr.Size) != bhdr.Size ) return 2;

	close(fd);

	fd = open(sysfile,O_RDONLY);
	
	if( fd == -1 ) return 3;
	
	if( link_out_data(link,&bhdr.Size,1) != 0 ) return 5;
	if( link_out_data(link,ibuf,bhdr.Size) != 0 ) return 6;

	Delay(100000);
		
	if( link_out_data(link,&bootcmd,1) != 0 ) return 7;
	
	while((size=read(fd,ibuf,BUFSIZE))>0)
	{
		if( link_out_data(link,ibuf,size) != 0 ) return 8;
	}

	close(fd);
	
	return 0;
}

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

