/*------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- romsupp.c								--
--                                                                      --
--	ROM support functions.						--
--	#define ROMRESIDENT for ROM version of Helios.			--
--                                                                      --
--	Author:  PAB 25/6/90						--
--                                                                      --
--------------------------------------------------------------------------
--
-- RCSId: $Id$							
--
-- RCSLog: $Log$
--
*/

#define __in_romsupp 1	/* flag that we are in this module */

#include "../kernel.h"
#include <root.h>
#include <config.h>
#include <protect.h>

#ifndef ROMRESIDENT


/* Note we are NOT ROM resident, so get config info from link */

int GetROMConfig(Config *config)
{
	return 0;
}


/* Notes no ROM items available */

#if 0
bool GetROMItem(word *index, char **name, word *size, Matrix *m, char **data)
{
	return FALSE; /* no items */
}
#endif

/* Following two functions are also defined as the macros GetSysBase() */
/* and GetRoot() in gexec.h solely for use within the kernel */

/* Returns standard position of the base of the nucleus */

MPtr GetNucleusBase()
{
#ifdef __TRAN
	return ((word *)0x80001000); /* first addr. in external ram */
#else
	return ( GetSysBase() );
#endif
}

/* Returns standard position of the start of RAM */
/* In RAM loaded systems this is directly after the nucleus */ 

RootStruct *GetRootBase()
{
#ifdef __TRAN
	/* first word of nucleus is the size in bytes of the nucleus */
	return ( GetNucleusBase() + (*(GetNucleusBase())/sizeof(word)) );
#endif
	return ( GetRoot() );
}

#else	/* ROM support functions */


/* If system image contains any ROM files, give their positions for loading */
/* into a ROM disk - *Warning*, may not be used depending on ROM disk version */
 
bool GetROMItem(word *index, char **name, word *size, Matrix *m, char **data)
{
	return FALSE; /* no items */
}


/* Returns start addr. of the nucleus */

word *GetNucleusBase(void)
{
	return ((word *)(NUCLEUSROMBASE))
}


/* Returns addr. of the root structure */
/* In ROM based systems this is directly after the ExecRoot, etc, not */
/* after the nucleus */

word *GetRootBase(void)
{
#ifdef __TRAN
	return ((word *)(0x80001000))	/* start of external RAM */
#else
	return ( (word *)GetRoot() );
#endif
}


/* Copies system specific boot info into configuration struct */
/* and returns its size */

int GetROMConfig(Config *config)
{
	int i;

	/* Size of initial port table (and further increments) */
	config->PortTabSize = 1024L;

	/* Number of times booted this session - immaterial */
	config->Incarnation = 1;

#if 0
	/* Obselete - Executive now returns these values */
	config->LoadBase    = 0;
	config->ImageSize   = 0;
#endif

	/* Current time to initialise Helios's internal clock and date from */
	/* This is a unix style date stamp, as seconds since Jan 1st 1970 GMT */
	config->Date        = 0; /* get_unix_style_time();*/

	/* Obselete - Used to be the the first program in the system image to run */
	config->FirstProg   = 6;

	/* Size of memory (Default specified by executive if zero) */
	config->MemSize      = 0;

	/* Note we are ROM loaded */
#if 1
	config->Flags       = Root_Flags_rootnode|Root_Flags_special|Root_Flags_ROM;
#else
	config->Flags       = Root_Flags_rootnode|Root_Flags_special;
#endif
	/* Reserved locations */
	config->Spare[0]    = 0;

#ifdef __C40
	/* Number of transputer style links attached to system */
	config->NLinks      = 6;
#elif defined(__TRAN)
	/* Number of transputer style links attached to system */
	config->NLinks      = 4;
#endif

	/* Set up link configuration information */

	for (i=0; i < config->NLinks; i++) {
		/* link not connected to IO processor or accepting IOdebugs and didn't boot us */
		config->LinkConf[0].Flags = 0x00;

		/* Note that link should be part of network */
		config->LinkConf[0].Mode  = Link_Mode_Intelligent;

		/* but currently is not communicating (dead) */
		config->LinkConf[0].State = Link_State_Dead;

		/* link config for link ID zero */
		config->LinkConf[i].Id    = i;	/* Link 0 */
	}

	/* setup names of nonexistant IO processor and this processors name */
	/* These two RPTRs can be -1 to signify /00 and /IO as acceptable defaults */
#if 1
	config->MyName      = -1;		/* default to /00 */
	config->ParentName  = -1;		/* default to /IO */

	return(sizeof(Config));
#else
	{
		char *procname = "/00";
		char *myname = "IO";
	
		config->MyName      = ( (word) (&(config->names[0])) )
			- ( (word) (&(config->MyName)) );
		config->ParentName  = ( (word) (&(config->names[strlen(procname)+1])) )
			- ( (word) (&(config->ParentName)) );

		strcpy(config->names, procname);
		strcpy(&(config->names[strlen(procname) + 1]), myname);

		return(sizeof(Config) + strlen(procname) + strlen(myname));
	}
#endif
}
#endif /* ROMRESIDENT */
