/* THRASH.C - Hank Vaccaro's memory Thrash benchmark, original form.    */
/* ==================================================================== */
/* Tests speed of pseudo-random 32 bit accesses to a large block	*/
 /* of memory.	Test is especially severe for virtual memory systems.	 */
/* Requires 2+ Mb to run.  I will post a smaller memory model if needed.*/
/* ==================================================================== */
/* By:	Hank Vaccaro					(505)-667-7777	*/
/*	BIX Mail hvacc							*/
/* ==================================================================== */
#define INT	 long int		/* Or whatever is 32 bits	*/
#define MEMSIZE (1 << 19)		/* 2 Mbyte			*/
#define NUMACCESSES (1 << 22)		/* 4 M accesses (4 ld, 1 st ea.)*/
#define MASK	524287			/*((unsigned)(~0) >> 13)	*/
main()
{
	 register unsigned INT i, now, next, last, *ram;
	char	*calloc();

	/* LIB$INIT_TIMER(); */ 	/* VMS Only */

	printf("Start memrnd Version 1.0.\n");

	ram = (unsigned INT * )calloc(MEMSIZE, sizeof(INT));

	for (i = MEMSIZE; i; i--)
		*ram++ = i & MASK;	/* Fill the array	   */

	 ram -= MEMSIZE;		 /* Reset the array pointer */

	last = 111111;
	now = 333333;

	/* Memory Access:  4 LOADS, 1 STORE per loop. */
	/* 1 DECR, 1 BRANCH, 3 ADDS, 2 ANDS, 3 REGISTER ASSIGNS    */
	/* Some pointer arithmetic too. */

	for (i = NUMACCESSES; i ; i--) {
		next = ram[ ram[ now + last & MASK ] ];
		ram[ next + last & MASK ] = ram[now] + ram[last] & MASK;
		 last = now;
		now = next;
	}
	printf("Finish.\n");
	if (now != 435030) printf("Error in source, compiler or RAM.\n");

	/* LIB$SHOW_TIMER();*/		/* VMS Only */
}
