/*
 * Name:	MG
 * Version:	2x
 *		Spawn an AmigaDOS subprocess
 * Last edit:	05-Sep-1987
 * By:		...!ihnp4!seismo!ut-sally!ut-ngp!mic
 */

#include <libraries/dos.h>
#include <libraries/dosextens.h>
#undef TRUE
#undef FALSE
#include "def.h"		/* AFTER system files to avoid redef's */

/*
 * Create a subjob with a copy of the command intrepreter in it.
 * This is really a way to get a new copy of the CLI, because
 * we don't wait around for the new process to quit.  Note the use
 * of a file handle to nil: to avoid the "endcli" message going out
 * to Emacs's standard output.
 */

spawncli(f, n)
{
	struct FileHandle *nil, *Open();
	
	ewprintf("[Starting new CLI]");
	nil = Open("NIL:", MODE_NEWFILE);
	if (nil == (struct FileHandle *) 0) { /* highly unlikely */
		ewprintf("Can't create nil file handle");
		return (FALSE);
	}
	Execute("NEWCLI \"CON:0/0/640/200/MicroEmacs Subprocess\"",nil,nil);
	Close(nil);
	return (TRUE);
}


