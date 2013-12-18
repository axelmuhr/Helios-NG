
/* external interface functions and variables  */

extern FILE *outfd;                     /* output stream                */
extern UBYTE gbyte();                   /* get a byte from the code     */
extern void ungbyte();                  /* step back in code            */
extern WORD curpos;                     /* current code location        */
                                        /* updated by (un)gbyte         */
extern struct symb *curcode;            /* next label in code           */
extern struct fix  *curfix;             /* next patch to be applied     */

