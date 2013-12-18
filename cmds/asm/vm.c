/* $Id: vm.c,v 1.12 1994/08/09 16:43:25 al Exp $ */

#ifdef __HELIOS
#include <helios.h>
#include <posix.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <module.h>
/*#include "asm.h"*/
#else
#include "ttypes.h"
#ifdef __DOS386
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#else /* !__DOS386 */
#include <strings.h>
#include <sys/file.h>
#endif /* __DOS386 */
#endif

#ifndef O_BINARY
#define O_BINARY	0
#endif

#include <stdio.h>
#include <sys/types.h>

#ifdef __HELIOS
# define ellipsis ...
#else /* !__HELIOS */
# define ellipsis
# define byte		UBYTE
# define word		WORD
static int oserr;
# define wordlen(s) (((s)+3)&~3)
#ifndef __DOS386
extern int errno;
# define SEEK_SET	0
# define memcpy(s,d,z) bcopy(d,s,z)
# define memset(d,v,z) bzero(d,z)
#endif /* __DOS386 */
#endif /* __HELIOS */

#include "vm.h"

#ifdef __DOS386

extern void 	error(BYTE *str, ...);
extern void 	warn(BYTE *str, ...);
extern void 	report(BYTE *str, ...);
extern void 	_trace(BYTE *str, ...);
extern void 	recover(BYTE *str, ...);

#else /* !__DOS386 */

extern void error(ellipsis);
extern void report(ellipsis);
extern void _trace(ellipsis);

#endif /* __DOS386 */


extern FILE *verfd;

#define VMTabInc 64

VMpage *VMTable = NULL;

#ifdef __DOS386
char _VMfilename[128];
char *VMfilename = _VMfilename;
#else /* !__DOS386 */
char *VMfilename = "/helios/tmp/vmfile";
#endif /* __DOS386 */

VMRef NullVMRef;
int VMTabUpb = 1;
int VMPageSize = 8*1024;
int VMLeave = 8;
 
static int VMTabSize = VMTabInc;
static int VMmaxpages = 256;
static int VMfile = -1;
static int swapins = 0;
static int swapouts = 0;
static int resident = 0;
static int fileupb = 0;

#ifdef __STDC__
extern VMRef VMPage(void);
extern APTR VMswap(VMRef v);
static APTR VMfindblock(void);
extern VMRef VMalloc(int size, VMRef v);
#ifdef __DOS386
static void VMerror(BYTE *str, ...);
#else /* !__DOS386 */
static void VMerror(ellipsis);
#endif /* __DOS386 */
#else
extern VMRef VMPage();
extern APTR VMswap();
static APTR VMfindblock();
extern VMRef VMalloc();
static void VMerror();
#endif

extern int VMInit(vmfile,pagesize)
char *vmfile;
int pagesize;
{
	*(int *)&NullVMRef = 0;
	VMPageSize = pagesize;
#ifdef __HELIOS
	VMmaxpages = 30000;
#else
	VMmaxpages = 256;
#endif

#ifdef __DOS386
	/* Get a temporary file name */
	if (vmfile == NULL) {
		char *tmp;
		char *ch;

		/* Get location for temporary file */
		tmp = getenv((const char *)"TEMP");
		if (tmp == NULL) {
			tmp = getenv((const char *)"TMP");
			if (tmp == NULL) tmp = "\\";
		}

		/* Create the temporary file name string */
		strcpy(VMfilename, tmp);
		ch = &VMfilename[strlen(VMfilename)-1];
		if ((*ch != '\\') && (*ch != '/')) {
			/* Add directory seperator */
			ch++;
			*ch = '\\';
		}
		ch++;

		/* Patch VM file on the end */
		strcpy(ch, "VMFILE.ASM");
	} else {
		VMfilename = vmfile;
	}
#else
	if( vmfile != NULL ) VMfilename = vmfile;
#endif
		
	VMTable = (VMpage *)malloc(sizeof(VMpage)*VMTabSize);
	
	if(VMTable == NULL ) return FALSE;

	memset(VMTable,0,sizeof(VMpage)*VMTabSize);

	VMTable[0].addr = NULL;
	VMTable[0].filepage = -1;
	VMTable[0].left = 0;
	VMTable[0].status = VMswapped;
	VMTable[0].locks = 1;
	VMTable[0].age = 0;
	VMTable[0].swapsin = 0;
	VMTable[0].swapsout = 0;
		
	return TRUE;
}

extern void VMTidy()
{
	if( VMfile != -1 ) { close(VMfile); unlink(VMfilename); }
	
	if( VMTable != NULL ) free(VMTable);
}

extern void VMStats()
{
report("VM Statistics:   Pages Used     %8ld File Pages     %8ld",VMTabUpb-1,fileupb);
report("                 Resident       %8ld Page size      %8ld",resident,VMPageSize);
report("                 Swaps in       %8ld Swaps out      %8ld",swapins,swapouts);
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
		int 		newsize = VMTabSize + VMTabInc;
		VMpage *	newtab  = (VMpage *)malloc( sizeof(VMpage) * newsize );

		
		if ( newtab == NULL )
		  VMerror("Cannot extend page table beyond page %d",page);

		memset( newtab + VMTabSize, 0, sizeof (VMpage) * VMTabInc );

		memcpy( newtab, VMTable, sizeof(VMpage) * VMTabSize );

		free( VMTable );

		VMTable = newtab;
		VMTabSize = newsize;
	}

	/* set age bit to prevent it being swapped !! */
	VMTable[page].age = VMagebit;
	
	if( VMmaxpages-- > 0 )
	{
#ifdef __HELIOS
		block = malloc(VMPageSize*(resident/VMLeave+1));
		/* if I cannot allocate a single block of 1/VMLeave the */
		/* size of the memory I already have, dont get any more.*/
		if( block == NULL )
		{
			VMmaxpages = 0;
		}
		else {
			free(block);
			block = malloc(VMPageSize);
		}		
#else
		block = (APTR)malloc(VMPageSize);
		if( block == NULL ) VMmaxpages = 0;
#endif
	}
	
	if( block == NULL ) block = VMfindblock();
	else resident++;

	VMTable[page].addr = block;
	VMTable[page].left = VMPageSize;
	VMTable[page].status = 0;
	VMTable[page].filepage = -1;
	VMTable[page].locks = 0;
	VMTable[page].age = VMagebit;
	VMTable[page].swapsin = 0;
	VMTable[page].swapsout = 0;		
	
	memset(block,0,VMPageSize);
	
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
	if( 	(*(int *)&v <= 0)	|| 
		(page_(v) == 0)		|| 
		(page_(v) > VMTabUpb)	|| 
		(offset_(v) > VMPageSize) 
	  ) 
	{
		char *name = "VMSwap";
#if !defined(SUN4) && !defined(RS6000) && !defined(__DOS386)
		word *p = (word *)&v;
		Proc *proc;
		p = (word *)(p[-2]);
		p = (word *)((word)p & ~3);
		until( *p == 0x60f360f3 ) p--;
		proc = (Proc *)p;
		name = proc->Name;
#endif
		VMerror("invalid VMRef %x in %s",v,name);
	}

	if( (page->status&VMswapped) == 0 )
	{
		page->status |= VMtouched;
		return page->addr+offset_(v);
	}

	VMLRU();
	
	filepage = page->filepage;

	if( filepage == -1 ) VMerror("trying to swap in page %x without filepage",v);
	
	block = VMfindblock();

	swapins++;
	page->swapsin++;
	
	if( VMfile == -1 ) VMerror("vmfile not open!!");
	
	if( (pos = lseek(VMfile,filepage*VMPageSize,SEEK_SET)) != filepage*VMPageSize)
	{
		VMerror("swap in seek failure for page %d filepage %d oserr %x",
				page-VMTable,filepage,oserr);
	}
	
	if( ( rsize = read(VMfile,block,VMPageSize) ) != VMPageSize )
	{
		VMerror("read failure for page %d res %d oserr %x",
				page-VMTable,rsize,oserr);
	}

	page->status = 0;
	page->addr = block;
	page->age = VMagebit;
		
	return page->addr+offset_(v);
}

static APTR VMfindblock()
{
	int filepage;
	int wsize, pos;
	APTR block = NULL;
	VMpage *page = NULL;
	int p;
	int minref = (VMagebit-1)|VMagebit;
	
	/* first find a page to evict */
	
	for( p = 1 ; p < VMTabUpb ; p++ )
	{
		if( (VMTable[p].locks == 0) &&
		    ((VMTable[p].status&VMswapped) != VMswapped) )
		{
			if( page == NULL ) page = &VMTable[p];
			if ((VMTable[p].age) < minref )
			{
				page = &VMTable[p];
				minref = page->age;
			}
		}
	}

	if( page == NULL ) VMerror("no pages available for swapping!!");

	/* page should now point to the page with the highest reference count */

	block = page->addr;
	
	if( block == NULL ) VMerror("NULL block for page %d",page-VMTable);
		
	/* Now find a file page to evict it to.				*/
	/* If the page has a shadow, and it has not been dirtied, we can*/
	/* just resuse it without swapping out. 			*/
	/* If it has a shadow on disc, and it is dirty, swap it out.	*/
	/* Otherwise look for another page with an out of date shadow	*/
	/* and confiscate that.						*/
	/* As a last resort, we allocate a new page in the swap file.	*/
	
	/* The result of this is that the sum of resident and file pages*/
	/* will be greater than the number of pages used since some will*/
	/* be present in both places. However this reduces the number of*/
	/* writes we do since read-only pages will never need to be	*/
	/* swapped out.							*/

	if( page->filepage == -1 )
	{
		VMpage *pp;
		/* Look for a page whose shadow is out of date and	*/
		/* re-use that file page. 				*/
		for( p = 1; p < VMTabUpb ; p++ )
		{
			pp = &VMTable[p];
			if( (pp->status&(VMdirty|VMswapped)) == (VMdirty|VMswapped) ) 
				VMerror("page %d state invalid",p);
			if( pp->filepage != -1 && pp->status&VMdirty ) break;
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
			page->addr = NULL;
			return block;
		}
		else filepage = page->filepage;
	}

	swapouts++;
	page->swapsout++;
	
	if( VMfile == -1 )
		if( (VMfile = open(VMfilename,O_BINARY|O_RDWR|O_CREAT,0666) ) == -1 )
			VMerror("Cannot open VM file %s %d %x",VMfilename,errno,oserr);

	if( (pos = lseek(VMfile,filepage*VMPageSize,SEEK_SET)) != filepage*VMPageSize)
	{
		VMerror("swap out seek failure for page %d filepage %d oserr %x",
				page-VMTable,filepage,oserr);
	}

	if( ( wsize = write(VMfile,block,VMPageSize) ) != VMPageSize )
	{
		VMerror("write failure for page %d result %d oserr %x",
				page-VMTable,wsize,oserr);
	}

	page->status = VMswapped;
	page->addr = NULL;
	
	return block;
}

/* allocate the next size bytes of space from the referenced page */
extern VMRef VMalloc(size,v)
int size;
VMRef v;
{
	VMRef res;
	VMpage *page = &VMTable[page_(v)];

	if( page->status & VMswapped ) (void)VMswap(v);

	if( offset_(v) != 0 ) VMerror("VMalloc not called on base ref: %x",v);
	
	size = wordlen(size);
	
	if( page->left < size )
	{
		return NullVMRef;
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

	if( offset_(v) != 0 ) VMerror("VMnext not called on base ref: %x",v);
	
	MakeVMRef(res,page_(v),VMPageSize-page->left);

	return res;
}

extern int VMleft(v)
VMRef v;
{
	VMpage *page = &VMTable[page_(v)];

	if( offset_(v) != 0 ) VMerror("VMleft not called on base ref: %x",v);
	
	return page->left;	
}

extern void VMlock(v)
VMRef v;
{
	VMpage *p = &VMTable[page_(v)];
	if( p->status & VMswapped ) (void)VMswap(v);
	p->locks++;
}

extern void VMunlock(v)
VMRef v;
{
	if( VMTable[page_(v)].locks-- == 0)
		VMerror("Too many VMunlock's for page %d",page_(v));
}

extern void VMLRU()
{
	VMpage *page;
	VMpage *end = &VMTable[VMTabUpb];
	int bit = VMagebit;

	for( page = VMTable+1; page < end ; page++ )
	{
		page->age >>= 1;
		if (page->status&VMtouched)
			page->status &= ~VMtouched,
			page->age |= bit;
	}
}

extern void showvm()
{
	int i;
	report("page bits  address fpage left locks age        in out");
	for( i = 0; i < VMTabUpb ; i++ )
	{
		VMpage *p = &VMTable[i];
		char *s = (p->status&VMswapped)?"s":"-";
		char *d = (p->status&VMdirty)?"d":"-";
		char *t = (p->status&VMtouched)?"t":"-";
		report("%4d %s%s%s  %8x %4d  %4x  %4d %08x %3d %3d",
			i,s,d,t,p->addr,p->filepage,p->left,p->locks,p->age,
			p->swapsin,p->swapsout);
	}
}

#ifdef __DOS386

static void VMerror(BYTE *str, ...)
{
	extern char infile[];
	va_list	argptr;

        fprintf(verfd,"VM Error: %s :",infile);

	va_start(argptr, str);
        vfprintf(verfd,str,argptr);
	va_end(argptr);
        putc('\n',verfd);
#if 0 || defined(SUN4)
	{
		float x = 0.0;
		float y = 0.0;
		float z;

		z = x/y;		/* force fpe to get core dump	*/
	}
#endif
	showvm();
 	exit(20);
}

#else /* !__DOS386 */

static void VMerror(str,a,b,c,d,e,f )
BYTE *str;
INT a,b,c,d,e,f;
{
	extern char infile[];
        fprintf(verfd,"VM Error: %s :",infile);

        fprintf(verfd,str,a,b,c,d,e,f);
        putc('\n',verfd);
#if 0 || defined(SUN4)
	{
		float x = 0.0;
		float y = 0.0;
		float z;

		z = x/y;		/* force fpe to get core dump	*/
	}
#endif
	showvm();
 	exit(20);
}

#endif /* __DOS386 */
