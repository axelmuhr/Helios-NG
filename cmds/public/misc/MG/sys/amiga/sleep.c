/*
 * Name:	MicroEmacs
 *		AmigaDOS sleep function
 * Version:	31
 * Last Edit:	18-Apr-86
 * Created:	18-Apr-86 ...!ihnp4!seismo!ut-sally!ut-ngp!mic
 */

/* There are really 60 ticks/second, but I don't want to wait that 	*/
/* long when matching parentheses... */
#define	TICKS	45
extern	long Delay();

#ifdef	LATTICE
void
#endif
sleep(n)
int n;
{
	if (n > 0)
		Delay((long) n * TICKS);
}
