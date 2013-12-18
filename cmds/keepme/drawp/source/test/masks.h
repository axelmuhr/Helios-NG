/*-----------------------------------------------------------------*/
/*                                           source/tests/masks.h  */
/*-----------------------------------------------------------------*/


/* This is included by each test program which has the option that */
/*   everything that goes to the screen has a mask.                */

extern DpPixmap_t mpm,*msk;
extern int enm;
extern void setMask(DpPixmap_t*,int,int);
extern void waitSecond(void);
