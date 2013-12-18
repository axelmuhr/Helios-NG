/*
 * prime spawn.c for micrognuemacs
 */
#include       "def.h"

spawncli(f, n)
{
       fortran void comlv$();
       ttcolor(CTEXT);
       ttnowindow();
       ttmove(nrow-1, 0);
       if (epresf != FALSE) {
	       tteeol();
	       epresf = FALSE;
       }
       ttclose();
       comlv$();
       sgarbf = TRUE;		/* Force repaint.	*/
       ttopen();
#ifndef NO_DIR
       (void) dirinit();	/* current directory may have changed */
#endif
       return (TRUE);
}
