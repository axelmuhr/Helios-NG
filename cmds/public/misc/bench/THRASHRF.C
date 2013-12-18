/* THRASHRF.C								*/
/* ==================================================================== */
/* Tests speed of pseudo-random 32 bit accesses to a large block	*/
/* of memory.  Test is especially severe for virtual memory systems.	*/
/* Requires 2+ Mb to run.  I will post a smaller memory model if needed.*/
/* ==================================================================== */
/* By:	Hank Vaccaro					(505)-667-7777	*/
/*	BIX Mail hvacc							*/
/* ==================================================================== */
/*	Modified by Ron Fox to explore speed vs. time			*/
/*			10-Feb-1988					*/

 
#define INT	 long int		/* Or whatever is 32 bits	*/
#define MEMSIZE (1 << 21)		/* 8 Mbyte			*/
#define NUMACCESSES (1 << 24)		/* 16 M accesses (4 ld, 1 st ea.)*/
#define INITIALSIZE 4			/* Start at 4 words.		*/
static INT mem[MEMSIZE];		/* Size of memory	*/
main()
{
	register unsigned INT i, now, next, last, *ram;
	INT mask, cursize;		/* Current size and mask of memory */


 
	printf("Start memrnd Version 1.1.\n");

	cursize = INITIALSIZE;

	while(cursize <= MEMSIZE)
	{
	  ram = mem;
	  mask	  = cursize - 1;
	  for (i = cursize; i; i--)
		  *ram++ = i & mask;	  /* Fill the array	     */

	   ram = mem;		      /* Reset the array pointer */

	  last = 111111 & mask;
	  now = 333333	& mask;

	  /* Memory Access:  4 LOADS, 1 STORE per loop. */
	  /* 1 DECR, 1 BRANCH, 3 ADDS, 2 ANDS, 3 REGISTER ASSIGNS    */
	  /* Some pointer arithmetic too. */

	  LIB$INIT_TIMER();
	  for (i = NUMACCESSES; i ; i--) {
		  next = ram[ ram[ now + last & mask ] ];
		   ram[ next + last & mask ] = ram[now] + ram[last] & mask;
		  last = now;
		  now = next;
	  }

	  printf("Timing information for size = %d\n",cursize);
	  LIB$SHOW_TIMER();		   /* Show timing statistics */
	  cursize += cursize;		   /* Double sized next time aroune */
	}
}

	  LIB$SHOW_