
/* external interface functions and variables  */

extern FILE *outfd;                     /* output stream                */
extern UBYTE gbyte();                   /* get a byte from the code     */
extern void ungbyte();                  /* step back in code            */
extern WORD curpos;                     /* current code location        */
                                        /* updated by (un)gbyte         */


/* the following tags are chosen to be values which will not usually    */
/* be valid code.                                                       */

#define t_module        0x60f160f1      /* module structure tag         */
#define t_resref        0x60f260f2      /* resident module reference    */
#define t_proc          0x60f360f3      /* procedure entry point        */
#define t_code          0x60f460f4      /* general code symbol          */
#define t_stack         0x60f560f5      /* stack offset symbol          */
#define t_static        0x60f660f6      /* static data symbol           */

struct module {
        struct Node     node;           /* module list node             */
        char            name[32];       /* module name                  */
        WORD            base;           /* module base address          */
        WORD            size;           /* size of module in words      */
        struct List     procsyms;       /* list of procedures           */
        struct List     staticsyms;     /* list of static data symbols  */
};

struct proc {
        struct Node     node;           /* list node                    */
        char            name[32];       /* procedure name               */
        WORD            value;          /* address of proc entry        */
        struct List     codesyms;       /* code symbols in proc         */
        struct List     stacksyms;      /* stack symbols in proc        */
};

struct symb {
        struct Node     node;           /* list node                    */
        char            name[32];       /* symbol name                  */
        WORD            value;          /* symbol value                 */
};
