#include <helios.h>
WORD xpinit(int l, int isT2);
WORD xptidy(void);

WORD dbrdword(WORD address);		/* Not swapped */
WORD dbrdint(WORD address);		/* Swapped (if necessary) */
void dbwrword(WORD address, WORD data);	/* Not swapped */
void dbwrint(WORD address, WORD data);	/* Swapped (if necessary) */

void xpwrbyte(UBYTE b);
WORD xpwrrdy(void);
UBYTE xprdbyte(void);
WORD xprdrdy(void);

void xpwrword(WORD data);		/* Not swapped */
WORD xprdword(void);

void xpwrint(WORD data);		/* Swapped if necessary */
WORD xprdint(void);

void xpwrdata(UBYTE *buf, WORD size);

void error(char *s,...);

extern WORD linkno;
