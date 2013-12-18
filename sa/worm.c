
#include <salib.h>
#include <trace.h>
#include <thread.h>
#include <chanio.h>
#include <sysinfo.h>
#include "worm.h"

#pragma -s1		/* switch off stack checking	*/
#pragma -f0		/* switch off vector stack	*/

byte stacks[4][2100];

#define Timeout		(500)

int memsize(void)
{
	return (int)((int)memtop() - (int)_SYSINFO->freestore );
}

void linkbuffer(int link, Channel *nucleus)
{
	word tag;
	WormLoadData data;
	WormLinkData l;

	forever
	{
		chan_in_word(nucleus,tag);

		switch( tag )
		{
		case ProbeNeighbour:
			link_out_byte(link,0);
			link_out_word(link,PeekAddress);
			link_out_word(link,PeekValue);
			link_out_byte(link,1);
			link_out_word(link,PeekAddress);
			break;
			
		case BootNeighbour:
			chan_in_struct(nucleus,data);
			boot(link);
			link_out_word(link,LoadDataTag);
			link_out_struct(link,data);
			break;
			
		case LinkId:
		case AlreadyLoaded:
			chan_in_struct(nucleus,l);
			link_out_word(link,tag);
			link_out_struct(link,l);
			break;
			
		case Synchronise:
			link_out_word(link,tag);
			break;
		}
	}
}

void worm(int bootlink)
{
	int i;
	WormLoadData data;
	WormInfo info;
#if 0
	SysInfo *sysinfo = _SYSINFO;
#endif
	bool download[4];
	bool trylink[4];
	int nprocessors;
	int myid;
	Channel tobuffer[4];
	Channel *linkin[4];
	Channel dummy[4];
	int token;

	link_in_word(bootlink,i);
	link_in_struct(bootlink,data);

	data.mylink = bootlink;
	link_out_word(bootlink,LoadDataTag);
	link_out_struct(bootlink,data);
	link_in_word(bootlink,token);
		
	for( i = 0; i < 4; i++ )
	{
		if( i != bootlink ) 
		{
			tobuffer[i] = MinInt;

			thread_create(stacks[i]+2000,1,
					linkbuffer,8,
					i,&tobuffer[i]);
		}
		download[i] = FALSE;
		trylink[i] = TRUE;
		linkin[i] = InLink(i);
		dummy[i] = MinInt;
	}
	trylink[bootlink] = FALSE;
	linkin[bootlink] = &dummy[bootlink];
	info.linkarray[bootlink].proc = data.parentsid;
	info.linkarray[bootlink].link = data.parentslink;
	myid = nprocessors = data.myid;
	nprocessors++;
	
	tin_(ldtimer_()+1);		/* let buffers get going */
	
	for( i = 0; i < 4 ; i++ )
	{
		int select;
		int stage = 1;
		bool waiting = TRUE;
		int timeout = Timeout;
						
		if( !trylink[i] ) continue;

		chan_out_word(&tobuffer[i],ProbeNeighbour);
		
		while( waiting )
		{
			select = alt(timeout,4,linkin);

			if( select < 0 ) 		/* timeout	*/
			{
				if( stage == 1 )
				{
					info.linkarray[i].proc = Unattached;
					info.linkarray[i].link = Unattached;
				}
				else
				{
					info.linkarray[i].proc = stage;
					info.linkarray[i].link = TimeOut;
				}
				waiting = FALSE;
				continue;
			}
			
			if( select != i )		/* probe from link */
			{
				byte probe[14];

				link_in_data(select,probe,14);
				if( stage == 1 )
				{
					info.linkarray[i].proc = myid;
					info.linkarray[i].link = select;
					info.linkarray[select].proc = myid;
					info.linkarray[select].link = i;
					waiting = FALSE;
				}
				else
				{
					WormLinkData l;
					l.proc = myid;
					l.link = select;
					chan_out_word(&tobuffer[select],AlreadyLoaded);
					chan_out_struct(&tobuffer[select],l);
					link_in_word(select,token);
					link_in_struct(select,info.linkarray[select]);
					trylink[select] = FALSE;
					linkin[select] = &dummy[select];
				}
				continue;
			} /* if( select != i ) */
			
			/* only remaining case is a response from child */
			
			link_in_word(i,token);

			switch( token+stage )
			{
			case PeekValue+1:	/* poke-peek worked	*/
			{
				WormLoadData l;
				l.parentsid = myid;
				l.parentslink = i;
				l.myid = nprocessors;
				l.mylink = Unattached;
				chan_out_word(&tobuffer[i],BootNeighbour);
				chan_out_struct(&tobuffer[i],l);
				stage = 2;
				break;
			}
			
			case LoadDataTag+2:
			case LoadDataTag+3:
			{
				WormLoadData l;
				link_in_struct(i,l);
				link_out_word(bootlink,LoadDataTag);
				link_out_struct(bootlink,l);
				link_in_word(bootlink,token);
				chan_out_word(&tobuffer[i],Synchronise);
				if( stage == 2 )
				{
					info.linkarray[i].proc = l.myid;
					info.linkarray[i].link = l.mylink;
					stage = 3;
					timeout = 0;
				}
				break;
			}
			
			case AlreadyLoaded+1:
			{
				WormLinkData l;
				l.proc = myid;
				l.link = i;
				link_in_struct(i,info.linkarray[i]);
				chan_out_word(&tobuffer[i],LinkId);
				chan_out_struct(&tobuffer[i],l);
				waiting = FALSE;
				break;
			}
			case ReturnControl+3:
				link_in_word(i,nprocessors);
				download[i] = TRUE;
				waiting = FALSE;
				break;
				
			default:
				info.linkarray[i].proc = stage;
				info.linkarray[i].link = TokenError;
				waiting = FALSE;
			} /* switch */

		} /* while( waiting ) */
		
		trylink[i] = FALSE;
		linkin[i] = &dummy[i];
	} /* for(...) */

	/* return control to parent */
	link_out_word(bootlink,ReturnControl);
	link_out_word(bootlink,nprocessors);

	/* second phase is to send WormInfo's back to parent */

	info.type = mctype();

	info.memsize = memsize();

#if 0
	info.memok = memtest((word *)sysinfo->freestore,
		(byte *)(MinInt+info.memsize)-sysinfo->freestore);
#else
	info.memok = 0;
#endif

	link_out_word(bootlink,NetworkData);
	link_out_word(bootlink,myid);
	link_out_struct(bootlink,info);
	
	for( i = 0; i < 4; i++ )
	{
		bool reading = TRUE;
		if( !download[i] ) continue;
		
		while( reading )
		{
			int passonid;
			WormInfo passoninfo;
			
			link_in_word(i,token);
			
			switch( token )
			{
			case NetworkData:
				link_in_word(i,passonid);
				link_in_struct(i,passoninfo);
				link_out_word(bootlink,token);	
				link_out_word(bootlink,passonid);
				link_out_struct(bootlink,passoninfo);
				break;
			case NoMoreData:
				reading = FALSE;
			}
		}		
	}
	
	link_out_word(bootlink,NoMoreData);
	
}


int main()
{
	int i;
	
	worm(_SYSINFO->bootlink);
	
	/* if the processor has an activity light we flash it	*/
	/* five times to indicate that we are finished.		*/
	for(i=0;i<5;i++)
	{
		int i;
		tin_(ldtimer_()+2000);
		for( i = 0 ; i < 100000; i++);
	}
	
	
	exit(0);
}

