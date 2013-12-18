/*{{{  Banner */

/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S			                --
--                     -----------                                      --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- maxlat.c								--
--	   Interrupt and latency monitor for the ARM/VY86PId.		--
--                                                                      --
--	Author:  BLV 10/12/93						--
--                                                                      --
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /dsl/HeliosRoot/Helios/cmds/private/C40/RCS/maxlat.c,v 1.3 1994/02/16 15:04:13 nickc Exp $";

/*}}}*/
/*{{{  #include's */
#include <helios.h>
#include <syslib.h>
#include <root.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*}}}*/
/*{{{  usage() */

static void usage(void)
{
    fputs("maxlat: usage, maxlat [-z | -l]\n", stderr);
    fputs("        -z    reset current values\n", stderr);
    fputs("        -l    loop forever\n", stderr);
}

/*}}}*/
/*{{{  main() */

int main(int argc, char **argv) 
{
    bool	 loop	= FALSE;
    bool	 reset	= FALSE;
    ExecInfo	 exec_info;
    LatencyInfo	*info;

    if (argc > 2) usage();
    if (argc == 2) 
    {
	if (!strcmp(argv[1], "-z"))
	    reset = TRUE;
	elif (!strcmp(argv[1], "-l"))
	    loop = TRUE;
	else
	    usage();
    }

    GetExecInfo(&(exec_info));
    info	= exec_info.Latency;

    if (reset)
    {
#ifdef __ARM
	EnterSVCMode();
#endif
	info->MaxInterruptLat		= 0;
	info->MaxSoftwareInterruptLat	= 0;
	info->MaxDispatchLat		= 0;
	info->MaxInterruptPC		= 0;
#ifdef __ARM
	EnterUserMode();
#endif	
	return(EXIT_SUCCESS);
    }

    forever
    {
	printf("Maximum interrupt latency detected: %ld us (%ld ticks)\n",
	       (info->MaxInterruptLat * Latency_NsPerUnit) / 1000, info->MaxInterruptLat);
	printf("Program counter of interrupted thread was 0x%08lx\n", info->MaxInterruptPC);
#if 0	
	printf("Overhead of interrupt handling software: %ld us (%ld ticks)\n", 
	       (info->MaxSoftwareInterruptLat * Latency_NsPerUnit) / 1000, info->MaxSoftwareInterruptLat);
#endif
	printf("Maximum dispatch latency (including interrupt latency) : %ld us (%ld ticks)\n", 
	       (info->MaxDispatchLat * Latency_NsPerUnit) / 1000, info->MaxDispatchLat);

	if (loop)
	    Delay(10 * OneSec);
	else
	    break;
    }
    
    return(EXIT_SUCCESS);
}

/*}}}*/
