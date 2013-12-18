#ifdef NOPOSIX
# include <helios.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
#else
# ifdef NORCROFT
#  include <helios.h>
#  include <posix.h>
#  include <string.h>
#  include <stdlib.h>
#  include <unistd.h>
# else
#  include <ttypes.h>
#  include <strings.h>
# endif
# include <fcntl.h>
# include <stdio.h>
# include <sys/types.h>
#endif

#ifdef NORCROFT
#define ANSI
#define ellipsis ...
#else
#define ellipsis
#define byte		UBYTE
#define word		WORD
#define SEEK_SET	0
#define O_CREAT		0
extern int errno;
static int oserr;
#define wordlen(s) (((s)+3)&~3)
#define memcpy(s,d,z) bcopy(d,s,z)
#define memset(d,v,z) bzero(d,z)

#endif

#include "vm.h"

extern void error(ellipsis);
extern void report(ellipsis);
extern void _trace(ellipsis);

extern FILE *verfd;

#define VMTabInc 64

VMpage *VMTable = NULL;
char *VMfilename = "vmfile";
VMRef NullVMRef;

static int VMTabSize = VMTabInc;
static int VMmaxpages = 256;
static int VMPageSize = 8*1024;
static int VMTabUpb = 0;
#ifdef NOPOSIX
static FILE *VMfile = 0;
#else
static int VMfile = -1;
#endif
static int swapins = 0;
static int swapouts = 0;
static int resident = 0;
static int fileupb = 0;

#ifdef ANSI
extern VMRef VMPage(void);
extern APTR VMswap(VMRef v);
static APTR VMfindblock(void);
extern VMRef VMalloc(int size, VMRef v);
#else
extern VMRef VMPage();
extern APTR VMswap();
static APTR VMfindblock();
extern VMRef VMalloc();
#endif

extern int VMInit(vmfile,pagesize,maxpages)
char *vmfile;
int pagesize;
int maxpages;
{
	*(int *)&NullVMRef = -1;
	VMPageSize = pagesize;
	VMmaxpages = maxpages;

	if( vmfile != NULL ) VMfilename = vmfile;
		
	VMTable = malloc(sizeof(VMpage)*VMTabSize);
	
	if(VMTable == NULL ) return FALSE;

	memset(VMTable,0,sizeof(VMpage)*VMTabSize);

	return TRUE;
}

extern void VMTidy()
{
#ifdef NOPOSIX
	if( VMfile != NULL )
	{ fclose(VMfile); remove(VMfilename); }
#else
	if( VMfile != -1 )
	{ close(VMfile); unlink(VMfilename); }
#endif
	
	if( VMTable != NULL ) free(VMTable);
}

extern void VMStats()
{
report("VM Statistics: Pages Used %8ld File Pages %8ld",VMTabUpb,fileupb);
report("               Resident   %8ld Page size  %8ld",resident,VMPageSize);
report("               Swaps in   %8ld Swaps out  %8ld",swapins,swapouts);
}

/* get a whole new page from VM system */
extern VMRef VMPage()
{
	VMRef res;
	int page;
	APTR block = NULL;

	VMLRU();

	page = VMTabUpb++;

	if( page == VMTabSize ) 
	{
		int newsize = VMTabSize+VMTabInc;
		VMpage *newtab = malloc(sizeof(VMpage)*newsize);

		if( newtab == NULL ) error("Cannot extend page table");

		memset(newtab,0,sizeof(VMpage)*newsize);

		memcpy(newtab,VMTable,sizeof(VMpage)*VMTabSize);

		free(VMTable);

		VMTable = newtab;
		VMTabSize += VMTabInc;
	}
#ifdef NORCROFT	
	if( VMmaxpages-- > 0 )
	{
		block = malloc(VMPageSize*(resident/8+1));
		/* if I cannot allocate a single block of 1/8 the size	*/
		/* of the memory I already have, dont get any more.	*/
		if( block == NULL )
		{
			VMmaxpages = 0;
		}
		else {
			free(block);
			block = malloc(VMPageSize);
		}		
	}
#else
	block = malloc(VMPageSize);
#endif
	if( block == NULL ) block = VMfindblock();
	else resident++;

	VMTable[page].addr = block;
	VMTable[page].left = VMPageSize;
	VMTable[page].status = 0;
	VMTable[page].filepage = -1;

	MakeVMRef(res,page,0);

	return res;	
}

/* swap the page containing the given reference in */
extern APTR VMswap(v)
VMRef v;
{
	VMpage *page = &VMTable[page_(v)];
	APTR block;
	int filepage;
	int rsize, pos;
	if( (*(int *)&v < 0) || (page_(v) > VMTabUpb) || (offset_(v) > VMPageSize) )
		error("VM invalid VMRef: %x",v);

	if( (page->status&VMswapped) == 0 )
	{
		page->status |= VMtouched;
/*fprintf(verfd,"VMswap %x -> %x\n",v,page->addr+offset_(v));*/
		return page->addr+offset_(v);
	}

	VMLRU();
	
	filepage = page->filepage;

	if( filepage == -1 ) error("VM: trying to swap in page without filepage");
	
	block = VMfindblock();

/*printf("Swap in page %d from filepage %d to %x\n",page_(v),filepage,block);*/
putc('R',verfd); fflush(verfd);
	swapins++;

#ifdef NOPOSIX
	if( VMfile == NULL )
#else
	if( VMfile == -1 )
#endif
            error("VM: vmfile not open!!");

#ifdef NOPOSIX	
	if( fseek(VMfile,filepage*VMPageSize,SEEK_SET) != 0)
	{
		error("VM seek failure: %d %d %d",filepage,pos,errno);
	}
	else
		pos = filepage*VMPageSize;
	if( (rsize = fread(block, sizeof(char), VMPageSize, VMfile) ) != 
                                                             VMPageSize )
	{
		error("VM read failure: %d %d %d",VMPageSize,rsize,errno);
	}
#else
	if( (pos = lseek(VMfile,filepage*VMPageSize,SEEK_SET)) != filepage*VMPageSize)
	{
		error("VM seek failure: %d %d %d",filepage,pos,errno);
	}
	if( ( rsize = read(VMfile,block,VMPageSize) ) != VMPageSize )
	{
		error("VM read failure: %d %d %d %x",VMPageSize,rsize,errno,oserr);
	}
#endif	

	page->status = 0;
	page->addr = block;
	
	return page->addr+offset_(v);
}

static APTR VMfindblock()
{
	int filepage;
	int wsize, pos;
	APTR block = NULL;
	VMpage *page = NULL;
	int p;
	int maxref = 0;

	/* first find a page to evict */
	
	for( p = 0 ; p < VMTabUpb ; p++ )
	{
		if( ((VMTable[p].status&VMlocked) != VMlocked) &&
		    ((VMTable[p].status&VMswapped) != VMswapped) )
		{
			if( page == NULL ) page = &VMTable[p];
			if ((VMTable[p].status&VMrefcnt) > maxref )
			{
				page = &VMTable[p];
				maxref = page->status&VMrefcnt;
			}
		}
	}

	if( page == NULL ) error("VM: no pages available for swapping!!");

	/* page should now point to the page with the highest reference count */

	block = page->addr;
		
	/* Now find somewhere to evict it to, if the page already has a	*/
	/* shadow on disc, use that, else allocate the next page in the	*/
	/* file. Note that file pages do not correspond numerically to	*/
	/* the VM page they shadow.					*/
	/* If the page has a shadow, and the page has not been dirtied  */
	/* don't bother to evict it.					*/

#define NEWSWAP
#ifdef NEWSWAP
	if( page->filepage == -1 )
	{
		VMpage *pp;
		/* Look for a page whose shadow is out of date and	*/
		/* re-use that file page. This should reduce the number */
		/* of pages we have to use in the swap file.		*/
		for( p = 0; p < VMTabUpb ; p++ )
		{
			pp = &VMTable[p];
			if( ((pp->status&VMmask) == VMdirty) &&
			    (pp->filepage != -1 )
			  ) break;
		}

		/* if we failed to find a re-usable filepage, allocate	*/
		/* a new one at the end of the file.			*/
		if( p == VMTabUpb ) page->filepage = filepage = fileupb++;
		else 
		{
			/* reuse pp's filepage & disconnect him from it */
			page->filepage = filepage = pp->filepage;
			pp->filepage = -1;
		}
	}
	else 
	{
		if( (page->status & VMdirty) == 0 ) 
		{
			/* the filepage is up-to-date, no need to evict */
			page->status = VMswapped;
			return block;
		}
		else filepage = page->filepage;
	}
#else
	if((filepage = page->filepage) == -1 )
		page->filepage = filepage = fileupb++;
	else if( (page->status & VMdirty) == 0 ) 
	{
		page->status = VMswapped;
		return block;
	}
#endif

/*printf("Evicting page %d to file page %d from %x\n",page-VMTable,filepage,block);*/
putc('W',verfd); fflush(verfd);
	swapouts++;

#ifdef NOPOSIX	
	if( VMfile == NULL )
		if( (VMfile = fopen(VMfilename,"w+") ) == NULL )
			error("Cannot open VM file %s %d",VMfilename,errno);

	if( fseek(VMfile,filepage*VMPageSize,SEEK_SET) != 0 )
	{
		error("VM seek failure: %d %d %d",filepage,pos,errno);
	}
	else
		pos = filepage*VMPageSize;

	if( ( wsize = fwrite(block,sizeof(char),VMPageSize, VMfile) ) != VMPageSize )
	{
		error("VM write failure: %d %d %d",VMPageSize,wsize,errno);
	}
#else	
	if( VMfile == -1 )
		if( (VMfile = open(VMfilename,O_RDWR|O_CREAT) ) == -1 )
			error("Cannot open VM file %s %d %x",VMfilename,errno,oserr);

	if( (pos = lseek(VMfile,filepage*VMPageSize,SEEK_SET)) != filepage*VMPageSize)
	{
		error("VM seek failure: %d %d %d %x",filepage,pos,errno,oserr);
	}

	if( ( wsize = write(VMfile,block,VMPageSize) ) != VMPageSize )
	{
		error("VM write failure: %d %d %d %x",VMPageSize,wsize,errno,oserr);
	}
#endif
	page->status = VMswapped;

	return block;
}

/* allocate the next size bytes of space from the referenced page */
extern VMRef VMalloc(size,v)
int size;
VMRef v;
{
	VMRef res;
	VMpage *page = &VMTable[page_(v)];

	if( offset_(v) != 0 ) error("VM: VMalloc not called on base ref: %x",v);
	
	VMLRU();

	size = wordlen(size);
	
	if( page->left < size )
	{
		*(int *)&res = -1;
		return res;
	}
	
	MakeVMRef(res,page_(v),VMPageSize-page->left);

	page->left -= size;

	page->status |= VMdirty;	/* ensure it is dirtied */
	
	return res;
}

extern VMRef VMnext(v)
VMRef v;
{
	VMRef res;
	VMpage *page = &VMTable[page_(v)];
	
	if( offset_(v) != 0 ) error("VM: VMnext not called on base ref: %x",v);
	
	VMLRU();
	
	MakeVMRef(res,page_(v),VMPageSize-page->left);

	return res;
}

extern int VMleft(v)
VMRef v;
{
	VMpage *page = &VMTable[page_(v)];

	if( offset_(v) != 0 ) error("VM: VMleft not called on base ref: %x",v);
	
	VMLRU();
	
	return page->left;	
}

extern void VMlock(v)
VMRef v;
{
	VMpage *p = &VMTable[page_(v)];
	VMLRU();
	(void)VMswap(v);
	p->status|=VMlocked;
}

extern void VMunlock(v)
VMRef v;
{
	VMLRU();
	VMTable[page_(v)].status&=~VMlocked;
}

extern void VMLRU()
{
	VMpage *page;
	VMpage *end = &VMTable[VMTabUpb];
	
	for( page = VMTable; page < end ; page++ )
		if ((page->status&VMtouched) == VMtouched)
			page->status &= ~VMtouched;
		else    page->status += VMrefinc;
}
