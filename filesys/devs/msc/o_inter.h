/* Header for Helios occam interface library */




/* Header structure */
/* preceeds the occam object code in a standalone occam program under Helios */

typedef struct O_HEADER {

	word	csize;		/* code size			*/	
	word	wspace;		/* workspace required		*/
	word	vspace;		/* vector space required	*/
	word	entry;		/* offset of entry point	*/

} O_HEADER;


/* Default parameters for an occam .EXE program			*/

typedef struct PARAMETERS {
	
	Channel	keyboard;
	Channel	screen;
	Channel from_user_filer;
	Channel to_user_filer;
	Channel from_fold_manager;
	Channel to_fold_manager;
	Channel from_filer;
	Channel to_filer;
	Channel from_kernel;
	Channel to_kernal;
	Channel freespace;

} PARAMETERS;


/* Literals */

#define  bytesperword	4	



/* Function declarations */

extern int O_Run(byte *, byte *, word, byte *, word, ...);
extern int O_Call(VoidFnPtr, ...);
