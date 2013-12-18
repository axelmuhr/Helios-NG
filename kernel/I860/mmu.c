#include <helios.h>
#include "exec.h"
#include "lowlevel.h"
#include "idebug.h"
#include "clock.h"
#include "linkio.h"
#include "itypes.h"

/* #define BUG33 */
/* #define BUG17 */

#define NOEPSR

#ifdef BUG33
#define MEM_ATTRIBUTES (pte_p+pte_w+pte_d+pte_a+pte_cd)
#else
#define MEM_ATTRIBUTES (pte_p+pte_w+pte_u+pte_d+pte_a)  
#endif


#if 0
#define DEBUG
#endif

#define MEMBASE 0xf0000000

#define variable 1

#define K (1024)
#define Meg (K*K)

typedef int PTE;
typedef PTE PT[1024];
typedef struct PD {
	PTE	entry[1024];
} PD;
typedef struct PTA {
	PT	table[variable];
} PTA;

#define AD_dirshift 22
#define AD_pageshift 12
#define AD_pagemask 0x3ff

/* 
 * This system has 8M.
 * Each second level page table can control 4M (1024*4096)
 * therefore we need 1 first level page table and
 * 2 second level ones.
 * We also need pages for various other things
 *  Name		Logical address		Physical address
 * the link adapter	    0xf8000000		   0xf8000000
 * the tick clock	    0xfa000000		   0xfa000000
 * the trap handler	    0xfffff000		   0xf0002000
 *
 * Thus we need
 *
 * FLUSHSPACE		1 page
 * TRAPSPACE		1 page  + 1 page_table
 * page_directory	1 page
 * main memory                    2 page_tables
 * ADAPTER			  1 page_table
 * CLOCK			  1 page_table
 *
 * Total: 8 pages 
 * The first usable address for code is therefore 0xf0008000
 *
 */

#define page_dir ((PD *)(MEMBASE+0x2000))
#define page_table ((PTA *)(MEMBASE+0x3000))
#define FLUSHSPACE (MEMBASE)

#define TRAPSPACE  (0xf0001000)

void clear_pt(PTA *pt, int n)
{	int i;
	while(n--)
		for( i = 0; i < sizeof(PT)/sizeof(PTE); i++)
			pt->table[n][i] = 0;
}

#define form_PTE(address,bits) ((PTE)(address) + bits)
#if 0
int nextpt;
#endif

PT *get_page_table()
{	word nextpt = GetNextPage();
	PT *r = &(page_table->table[nextpt++]);
	PutNextPage(nextpt);
	clear_pt((PTA *)r,1);
	return r;
}

PTE *find_PTE(ADDR address)
{	ADDR dir = address >> AD_dirshift;
	ADDR page = (address >> AD_pageshift) & AD_pagemask;
	PTE *dirp = &(page_dir->entry[dir]);
	PT  *table;
	
#if 0
	XIOdebug("debug: address = %X, dirp = %X, *dirp = %X\n",address,dirp,*dirp);
#endif
	if( (*dirp & pte_p) == 0 )
	{
		*dirp = form_PTE(get_page_table(),pte_p+pte_w+pte_u+pte_cd);
#if 0
		XIOdebug("allocated %X for dir %X\n",*dirp,dir,dirp);
#endif
	}
	table = (PT *)(*dirp & 0xfffff000);
	return &(table[0][page]);
}

void set_ATE(int flag, PD *where)
{	UWORD	db;
	XIOdebug("Entering from set_ate\n");
	db = GetDirBase();
	if( flag )
	{
		db &= ~db_dtbmsk;
		db |= db_ate + (UWORD)where;
	}
	else
		db &= ~db_ate;
	SetDirBase(db|db_iti);
	XIOdebug("Returning from set_ate\n");
}

void unset_map(ADDR logical, unsigned long size)
{
	unsigned long i;
	for( i = 0; i < size ; i += 4*K )
	{
		PTE *p = find_PTE(logical+i);
		*p = 0;
	}
}

void set_map(ADDR logical, ADDR physical, unsigned long size, int attributes)
{	unsigned long i;
	for( i = 0; i < size ; i += 4*K )
	{
		PTE *p = find_PTE(logical+i);
#if 0
		XIOdebug("Page table entry address for %X = %X\n",i,p);		
#endif
		*p = form_PTE(physical+i,attributes);
	}

}

static void print_mem(word *p, word size)
{
	word i,j;
	for (i=0; i<size; i+= 8)
		{
		for (j=i; j<i+8; j++)
			XIOdebug(" %X",p[j]);
		XIOdebug("\n");	
		}
	XIOdebug(" \n");	
}

extern void cpu_state(TrapData *td);
extern void Clear_Fsr(void);

void MmuInit(void)
{
	TrapData td;
	
	PutNextPage(0);

#ifdef DEBUG
	XIOdebug("mmu starting\n");
	cpu_state(&td);
	dumpregs(&td); 
#endif
	FlushData(FLUSHSPACE); obyte('\n');
	clear_pt((PTA *)page_dir,1);

/*
 * First the system memory
 * Physical memory page 0 appears at logical 0 and at MEMBASE 
 * I hope it doesn't matter since it is only used for the
 * dummy stores anyway.
 */
	set_map(0xf0000000,0xf0000000,8*Meg,MEM_ATTRIBUTES);

#if 0
/* Now the dummy page for BUG17 */
	set_map(0,BUG17SPACE,4*K,MEM_ATTRIBUTES);
#endif

/* Now the tick clock */
	set_map((ADDR)CLOCK,(ADDR)CLOCK,4*K,pte_p+pte_w+pte_d+pte_a+pte_cd+pte_u);

/* Now the link adapter */
	set_map((ADDR)ADAPTER,(ADDR)ADAPTER,4*K,pte_p+pte_w+pte_d+pte_a+pte_cd+pte_u);

/* Now the trap space */
	unset_map(TRAPSPACE,4*K);
	set_map(0xfffff000,TRAPSPACE,4*K,MEM_ATTRIBUTES);

Clear_Fsr();
#ifdef DEBUG
XIOdebug("About to set ATE\n");
	cpu_state(&td);
	dumpregs(&td); 
#endif
	set_ATE(1,page_dir);
#ifdef DEBUG
XIOdebug("ATE set\n");  
#endif

#ifdef DEBUG
XIOdebug("Leaving MmuInit \n");
#endif

}

