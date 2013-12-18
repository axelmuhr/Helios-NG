/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- rboot.c								--
--                                                                      --
--	Boot a C40 processor.						--
--                                                                      --
--	Author:  BLV 23/7/92						--
--                                                                      --
------------------------------------------------------------------------*/
static char *rcsid = "$Header: /hsrc/network/C40/RCS/rboot.c,v 1.6 1993/11/10 14:20:21 bart Exp $";

/*{{{  Header files and administration */
#include <stdio.h>
#include <stdlib.h>
#include <syslib.h>
#include <stdarg.h>
#include <ctype.h>
#include <task.h>
#include <codes.h>
#include <nonansi.h>
#include <string.h>
#include <root.h>
#include <config.h>
#include <link.h>
#include "../private.h"
#include "../rmlib.h"
#include "../netaux.h"
#include "../netutils.h"

/*}}}*/
/*{{{  statics etc. */

	/* The target link indicates which link on the remote processor	*/
	/* we are booting into.						*/
static int	TargetLink = -1;

	/* The hardware configuration word controls IDROM overwriting,	*/
	/* loading code into global memory, etc.			*/
static word	HWConfig	= 0;

	/* If the remote processor does not have an IDROM then it is	*/
	/* possible for C40 rboot to read in an idrom file.		*/
static IDROM	IdRom	= {
     /*{{{  default values */
         	sizeof(IDROM) / sizeof(word),	/* self inclusive size of this block		*/
         	0,				/* TIM-40 module manufacturer's ID		*/
         	0,				/* CPU type (00 = C40				*/
         	49,				/* CPU cycle time (49 = 50ns = 40MHz)		*/
         	0,				/* manufacturer's module type			*/
         	0,				/* module revision level			*/
         	0,				/* reserved byte				*/
         	0x80000000,			/* address base of global bus strobe 0		*/
         	0xffffffff,			/* address base of global bus strobe 1		*/
         	0x00300000,			/* address base of local bus strobe 0		*/
         	0xffffffff,			/* address base of local bus strobe 1		*/
         	0x00100000, /*(4Mb, 1Mw)*/	/* size of memory on global bus strobe 0	*/
         	0x00000000,			/* size of memory on global bus strobe 1	*/
         	0x00100000,			/* size of memory on local bus strobe 0		*/
         	0x00000000,			/* size of memory on local bus strobe 1		*/
         	0x00000800,			/* size of fast ram pool (inc. on-chip RAM	*/
         	0x22,				/* wait states within page on global bus	*/
         	0x22,				/* wait states within page on local bus		*/
         	0x55,				/* wait states outside page on global bus	*/
         	0x55,				/* wait states outside page on local bus	*/
         	0x2710,				/* period time for 1ms interval on timer 0	*/
         	0x80,				/* period for DRAM refresh timer		*/
         	0x2c2,				/* contents set TCLK0 to access RAM not IDROM	*/
         	0,				/* sets up timer to refresh DRAM		*/
         	0x3e39fff0,			/* global bus control register			*/
         	0x3e39fff0,			/* local bus control register			*/
         	0				/* total size of auto-initialisation data	*/
         };
     /*}}}*/

/*}}}*/
/*{{{  usage() and diagnostics */
static void report(char *str, ...)
{
	va_list		args;

	va_start(args, str);
	fputs("rboot: ", stderr);
	vfprintf(stderr, str, args);
	fputs("\n", stderr);
	va_end(args);
}

static void fatal(char *str, ...)
{
	va_list		args;

	va_start(args, str);
	fputs("rboot: ", stderr);
	vfprintf(stderr, str, args);
	fputs("\n", stderr);
	va_end(args);
	exit(EXIT_FAILURE);
}

static void usage(void)
{ 
  report("usage, rboot [-l<link>] [-i<idrom file>] link <myname> <itsname> [<link modes> <memory limit>]");
  fatal("for example, rboot 1 /Cluster/00 /Cluster/01");
}
/*}}}*/
/*{{{  determine image */
/**
*** figure out the system image to use. This involves checking the size
*** of the current nucleus and of the file /helios/lib/nucleus.
*** If they are the same, use the current nucleus. Otherwise load the
*** file off disk and use that.
**/
static MPtr	determine_image(void)
{
	Object	*nuc	= Locate(Null(Object), "/helios/lib/nucleus");
	ObjInfo	 info;
	MPtr	 image	= GetSysBase();
	Stream	*s;
	void	*buffer;
      
	if (nuc == Null(Object))
		return(image);
	if (ObjectInfo(nuc, Null(char), (BYTE *) &info) < Err_Null)
		return(image);
	if (info.Size == MWord_(image,0))
		return(image);

	buffer = Malloc(info.Size);
	if (buffer == Null(BYTE))
		fatal("not enough memory to load nucleus off disk.");

	s = Open(nuc, Null(char), O_ReadOnly);
	if (s == Null(Stream))
		fatal("failed to open nucleus file /helios/lib/nucleus");

	if (Read(s, buffer, info.Size, -1) != info.Size)
		fatal("failed to read all of nucleus file /helios/lib/nucleus");
	Close(s);

	return(CtoM_(buffer));  
}
/*}}}*/
/*{{{  build configuration vector for C40 */
/**
*** Build a configuration vector in memory. This involves allocating a
*** vector big enough for the main vector plus the two names (including
*** terminators). Most of the configuration vector can be filled in with
*** default information. The ImageSize field is patched later on. 
**/

static Config	*build_config(char *my_name, char *its_name, char *link_modes,
				int memory_limit, int *confsize)
{ 
	int	 size;
	Config	*result;
	int	 i;
	BYTE	*temp;

	size	=   sizeof(Config) + strlen(my_name) + strlen(its_name) + 2;
	if (strlen(link_modes) > 4)
		size += (4 * (strlen(link_modes) - 4));

	result = (Config *) Malloc(size);  
	if (result == Null(Config))
		fatal("not enough memory to build configuration vector.");

	result->PortTabSize	= 32;
	result->Incarnation	= 1;
	result->LoadBase	= 0; /* Gets overwritten by kernel */
	result->ImageSize	= 0;
	result->Date		= GetDate();
	result->FirstProg	= IVecProcMan; 
	result->MemSize		= memory_limit;
	result->Flags		= 0;
	result->Spare[0]	= 0;
	result->NLinks		= strlen(link_modes);
	for (i = 0; link_modes[i] != '\0'; i++)
	{
		result->LinkConf[i].Flags = 0;
		switch(link_modes[i])
		{
		case '0' :
			result->LinkConf[i].Mode	= Link_Mode_Null;
			result->LinkConf[i].State	= Link_State_Null;
			break;
		case '1' :
			result->LinkConf[i].Mode	= Link_Mode_Dumb;
			result->LinkConf[i].State	= Link_State_Dumb;
			break;
		case '2' :
			result->LinkConf[i].Mode	= Link_Mode_Intelligent;
			result->LinkConf[i].State	= Link_State_Running;
			break;
		case '3' :
			result->LinkConf[i].Mode	= Link_Mode_Intelligent;
			result->LinkConf[i].State	= Link_State_Dead;
			break;
		}
		result->LinkConf[i].Id		= i;
		result->LinkConf[i].Flags	= 0;
	}

	result->LinkConf[TargetLink].Mode	= Link_Mode_Intelligent;
	result->LinkConf[TargetLink].Flags	= Link_Flags_debug + Link_Flags_parent;
	result->LinkConf[TargetLink].State	= Link_State_Running;

		/* Take care of the two RPTRs */
	temp			 = (BYTE *) result;
	temp			 = &(temp[sizeof(Config)]);
	result->MyName		 = temp - (BYTE *) &(result->MyName);
	strcpy(temp, its_name);
	temp			+= (strlen(its_name) + 1);
	result->ParentName	 = temp - (BYTE *) &(result->ParentName);
	strcpy(temp, my_name);
  
		/* and return the results */
	*confsize = size;
	return(result);
}
/*}}}*/
/*{{{  read idrom file */
/**
*** IdRom support. Some C40 networks contain processors without IdRoms,
*** and Helios is still expected to run on them. To implement this it
*** is possible to specify an IdRom file on the command line to provide
*** or replace the necessary information, for example
***
*** rboot -l3 -i/helios/etc/id1.rom 0 /00 /01
***
*** The file id1.rom contains the same information as the host.con file,
*** and works in effectively the same way:
***
*** # IdRom for DSP1 board with 1Mb SRAM and 4Mb DRAM on local,
*** # plus 1Mb SRAM on global.
***
*** c40_idrom_cpu_clk	=	59
*** c40_idrom_lbase0	=	0x00300000
*** c40_idrom_lbase1	=	0x00400000
*** c40_idrom_lsize0	=	0x00040000	# 1Mb SRAM
*** c40_idrom_lsize1	=	0x00100000	# 4Mb DRAM
*** c40_idrom_lbcr	=	0x154d4010
***
*** In addition to the IdRom information it is necessary to specify
*** a hardware config word. This is used to disable the cache etc.
**/

static void	read_idrom(char	*filename)
{
	FILE		*idrom_file	= NULL;
	char		 buf[IOCDataMax];

	idrom_file	= fopen(filename, "r");
	if (idrom_file == NULL)
		fatal("cannot open idrom file %s", filename);

	while (fgets(buf, IOCDataMax - 1, idrom_file) ne NULL)
/*{{{  process idrom info */
{
	char	*tmp;
	char 	*parameter;
	char	*value_pos;
	int	 config_value = 0;
	char	 ch;

	enum
	{ 
		ERROR_NONE,
		ERROR_EXTRANEOUS_VALUE,
		ERROR_NO_VALUE
	} error;
   
	/* Step 1, search the string for a # and strip out the comment	*/
	for (tmp = buf; *tmp != '\0'; tmp++)
		if (*tmp == '#')
			{ *tmp = '\0'; break; }
  
	/* Step 2, ignore leading space and identify the parameter	*/
	for (parameter = buf; isspace(*parameter) && (*parameter ne '\0'); parameter++);
	if (*parameter eq '\0')	/* blank line ? */
		continue;

 	/* Step 3, terminate the parameter string			*/
	for (value_pos = parameter; !(isspace(*value_pos)) && (*value_pos ne '=') && (*value_pos ne '\0'); value_pos++);
	ch = *value_pos;
	*value_pos = '\0';

  	/* Step 4, skip white space between parameter and value		*/
	if (isspace(ch))
	{ 
		for (value_pos++; isspace(*value_pos) && (*value_pos ne '\0'); value_pos++);
		ch = *value_pos;
	}

 	/* Step 5, if ch is =, identify the start of the value field	*/
	if (ch != '=')
		value_pos = NULL;
	else
	{
		for (value_pos++; isspace(*value_pos) && (*value_pos ne '\0'); value_pos++);
		if (*value_pos eq '\0')
			fatal("IDRom file %s, no value for %s", filename, parameter);
	
		/* Step 6, determine the end of the value field		*/
		for (tmp = value_pos; !(isspace(*tmp)) && (*tmp ne '\0'); tmp++);
		*tmp = '\0';
	}


	/* Step 7, turn value into a number				*/
	if (value_pos ne NULL)
	{
		if ((value_pos[0] eq '0') && ((value_pos[1] eq 'x') || (value_pos[1] eq 'X')))
		{ 
			value_pos += 2;
			if (*value_pos eq '\0')
				fatal("IDRom file %s, invalid value for %s", filename, parameter);

			for (config_value = 0; *value_pos ne '\0'; value_pos++)
				if ((*value_pos >= '0') && (*value_pos <= '9'))
					config_value = (16 * config_value) + *value_pos - '0';
				elif ((*value_pos >= 'a') && (*value_pos <= 'f'))
					config_value = (16 * config_value) + *value_pos + 10 - 'a';
				elif ((*value_pos >= 'A') && (*value_pos <= 'F'))
					config_value = (16 * config_value) + *value_pos + 10 - 'A';
				else
					fatal("IDRom file %s, invalid value for %s", filename, parameter);
		}
		else
			for (config_value = 0; *value_pos ne '\0'; value_pos++)
				if ((*value_pos >= '0') && (*value_pos <= '9'))
					config_value = (10 * config_value) + *value_pos - '0';
				else
					fatal("IDRom file %s, invalid value for %s", filename, parameter);
	}

	/* At this point "parameter" is the name of the configuration	*/
	/* option and either value_pos is NULL or config_value holds	*/
	/* the numerical value associated with this option.		*/
	/* It is now possible to match the option with the known	*/
	/* strings and update the IDRom or the hardware configuration	*/
	/* word.							*/

	error = ERROR_NONE;
   
	if   (!mystrcmp(parameter, "c40_disable_cache"))
	{
		HWConfig |= HW_CacheOff;
		if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
	}
	elif (!mystrcmp(parameter, "c40_load_nucleus_local_s0"))
	{
		 HWConfig |= HW_NucleusLocalS0;
		if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
	}
	elif (!mystrcmp(parameter, "c40_load_nucleus_local_s1"))
	{
		HWConfig |= HW_NucleusLocalS1;
		if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
	}
	elif (!mystrcmp(parameter, "c40_load_nucleus_global_s0"))
	{
		HWConfig |= HW_NucleusGlobalS0;
		if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
	}
	elif (!mystrcmp(parameter, "c40_load_nucleus_global_s1"))
	{
		HWConfig |= HW_NucleusGlobalS1;
		if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
	}
	elif (!mystrcmp(parameter, "c40_replace_idrom"))
	{
		HWConfig |= HW_ReplaceIDROM;
		if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
	}
	elif (!mystrcmp(parameter, "c40_use_pseudo_idrom"))
	{
		HWConfig |= HW_PseudoIDROM;
		if (value_pos ne NULL) error = ERROR_EXTRANEOUS_VALUE;
	}
	elif (!mystrcmp(parameter, "c40_idrom_man_id"))
	{ 
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.MAN_ID = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_cpu_id"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.CPU_ID = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_cpu_clk"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.CPU_CLK = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_model_no"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.MODEL_NO = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_rev_lvl"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.REV_LVL = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_reserved"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.RESERVED = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_gbase0"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.GBASE0 = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_gbase1"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.GBASE1 = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_lbase0"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.LBASE0 = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_lbase1"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.LBASE1 = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_gsize0"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.GSIZE0 = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_gsize1"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.GSIZE1 = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_lsize0"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.LSIZE0 = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_lsize1"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.LSIZE1 = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_fsize"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.FSIZE = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_wait_g0"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.WAIT_G = (IdRom.WAIT_G & 0xf0) | config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_wait_g1"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.WAIT_G = (IdRom.WAIT_G & 0x0f) | (config_value << 4);
	}
	elif (!mystrcmp(parameter, "c40_idrom_wait_l0"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.WAIT_L = (IdRom.WAIT_L & 0xf0) | config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_wait_l1"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.WAIT_L = (IdRom.WAIT_L & 0x0f) | (config_value << 4);
	}
	elif (!mystrcmp(parameter, "c40_idrom_pwait_g0"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.PWAIT_G = (IdRom.PWAIT_G & 0xf0) | config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_pwait_g1"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.PWAIT_G = (IdRom.PWAIT_G & 0x0f) | (config_value << 4);
	}
	elif (!mystrcmp(parameter, "c40_idrom_pwait_l0"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.PWAIT_L = (IdRom.PWAIT_L & 0xf0) | config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_pwait_l1"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.PWAIT_L = (IdRom.PWAIT_L & 0x0f) | (config_value << 4);
	}
	elif (!mystrcmp(parameter, "c40_idrom_timer0_period"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.TIMER0_PERIOD = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_timer1_period"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.TIMER1_PERIOD = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_timer0_ctrl"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.TIMER0_CTRL = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_timer1_ctrl"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.TIMER1_CTRL = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_gbcr"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.GBCR = config_value;
	}
	elif (!mystrcmp(parameter, "c40_idrom_lbcr"))
	{
		if (value_pos eq NULL)
			error = ERROR_NO_VALUE;
		else
			IdRom.LBCR = config_value;
	}
	else
		fatal("IDRom file %s, unknown parameter %s", filename, parameter);

	if (error == ERROR_NO_VALUE)
		fatal("IDRom file %s, missing value for %s", filename, parameter);
	elif (error == ERROR_EXTRANEOUS_VALUE)
		fatal("IDRom file %s, extraneous value for parameter %s", filename, parameter);
   
}
/*}}}*/

  
	if (idrom_file != NULL)
		fclose(idrom_file);
}

/*}}}*/
/*{{{  bootlink */
/*{{{  init_link */
/**
*** If I am about to attempt a bootstrap down a link, that link had better
*** be in a sensible state. 
**/
static void init_link(int link)
{
	LinkInfo info;
	LinkConf conf;
  
	if (LinkData(link, &info) != Err_Null)
		fatal("failed to determine current link status.");

	if (info.Mode == Link_Mode_Dumb) return;
	conf.Mode 	= Link_Mode_Dumb;
	conf.State	= Link_State_Dumb;
	conf.Id		= info.Id;
	conf.Flags	= info.Flags;
	(void) Configure(conf);
}

/*}}}*/
/*{{{  set_link */
/**
*** After a bootstrap, ensure that the link is in a sensible state
**/
static void set_link(int link)
{
	LinkInfo	info;
	LinkConf	conf;

	if (LinkData(link, &info) != Err_Null)
		fatal("failed to determine new link status.");
   
	if ((info.Mode == Link_Mode_Intelligent) &&
	    (info.State == Link_State_Running))
		return;
	conf.Mode 	= Link_Mode_Intelligent;
	conf.State	= Link_State_Dead;
	conf.Id		= info.Id;
	conf.Flags	= info.Flags;
	Configure(conf);
}
/*}}}*/
/*{{{  check_link */
/**
*** After a bootstrap check that the link has been enabled from the
*** other side. That means that the kernel on the other side has
*** started running, a good start although not necessarily sufficient.
**/
static	word	c40_check_link(int link)
{
	LinkInfo	info;
	word		rc;
    
	if ((rc = LinkData(link, &info)) != Err_Null) return(rc);
	if ((info.Mode != Link_Mode_Intelligent) ||
	    (info.State != Link_State_Running))
		return(EC_Error + SS_NetServ + EG_Boot + EO_Processor);

	return(Err_Null);
}
/*}}}*/
/*{{{  bootstrap */
/**
*** Manual bootstrap. This involves the following stages.
*** 1) Send in magic numbers for the memory
*** 2) Send in C40boot.i, held in a slot in the system image
*** 3) Send in more magic numbers
*** 4) Receive bootstrap executing acknowledgement
*** 5) Send hardware configuration flags to booter
*** 6) Possible send in a pseudo idrom
*** 7) Send in the whole system image
*** 8) Send in the configuration vector
**/
static word My_BootLink(word link, MPtr image, Config *config, word confsize)
{
	WORD	temp[4];
	MPtr	c40boot;
	word	c40boot_size;
	word	image_size;
	word	rc;

	/* Send C40 boot protocol header.		*/
	c40boot		= MRTOA_(MInc_(image,IVecBootStrap*sizeof(WORD)));
	c40boot_size	= MWord_(image,IVecProcMan*sizeof(WORD)) 
				- MWord_(image,IVecBootStrap*sizeof(WORD)) + 4;

	temp[0] = 0x3e39fff0;			/* Global bus memory control word */
	temp[1] = 0x3e39fff0;			/* Local  bus memory control word */
	temp[2] = c40boot_size / sizeof(WORD);	/* Block size			  */
	temp[3] = 0x002ffc00;			/* Load address			  */
	if (HWConfig & (HW_PseudoIDROM | HW_ReplaceIDROM))
	{
		temp[0]	= IdRom.GBCR;
		temp[1]	= IdRom.LBCR;
	}
	if ((rc = LinkOut(16, link, temp, 2 * OneSec)) != Err_Null)
		return(Boot_BootstrapSize | EG_Timeout | EC_Error | SS_NetServ);

	/* Now send in the actual bootstrap code	*/
	if (MP_LinkOut(c40boot_size/sizeof(WORD), link, c40boot, 2 *OneSec) != Err_Null)
		return(Boot_BootstrapCode | EG_Timeout);

	Delay(1000);		/* One millisecond for nboot.i to settle down */

	/* Send C40 boot protocol tail */
	temp[0] = 0;				/* terminator	*/
	temp[1] = 0;				/* IVTP		*/
	temp[2] = 0;				/* TVTP		*/
	temp[3] = 0x00300000;			/* IACK		*/
	if ((rc = LinkOut(16, link, temp, 2 * OneSec)) != Err_Null)
		return(Boot_ProtocolTail | EG_Timeout | EC_Error | SS_NetServ);

	/* Receive bootstrap acknowledgement from bootstrap code */
	if ((rc = LinkIn(4, link, temp, 2 * OneSec)) != Err_Null)
		return(Boot_Acknowledgement | EG_Timeout | EC_Error | SS_NetServ);
	if (temp[0] != 1) /* bootstrap ack is always 1 */
		return(Boot_Acknowledgement2 | EG_Timeout | EC_Error | SS_NetServ);

	/* Send hardware configuration flags to bootstrap code */
	temp[0] = HWConfig; 
	if ((rc = LinkOut(4, link, temp, 2 * OneSec)) != Err_Null)
		return(Boot_Hwconfig | EG_Timeout | EC_Error | SS_NetServ);

	/* Possibly send in idrom info.				*/
	if (HWConfig & (HW_ReplaceIDROM | HW_PseudoIDROM))
		if ((rc = LinkOut(sizeof(IDROM), link, (BYTE *) &IdRom, 2 * OneSec)) != Err_Null)
			return(Boot_Idrom | EG_Timeout | EC_Error | SS_NetServ);
	
	/* Now send in the actual nucleus.			*/
	image_size = MWord_(image,0);
	if (MP_LinkOut(image_size/sizeof(WORD), link, image, 2 * OneSec) != Err_Null)
		return(Boot_Image | EG_Timeout | EC_Error | SS_NetServ);

	/* And the configuration vector, size + data		*/
	if (LinkOut(sizeof(WORD), link, (BYTE *) &confsize, 2 * OneSec) != Err_Null)
		return(Boot_ConfigSize | EG_Timeout | EC_Error | SS_NetServ);
	if (LinkOut(confsize, link, (BYTE *) config, 2 * OneSec) != Err_Null)
		return(Boot_ConfigVector | EG_Timeout | EC_Error | SS_NetServ);

	/* Hopefully everything is happy and the remote kernel is going	*/
	/* to initialise correctly and enable the link. This will be	*/
	/* detected by check_link(), and it marks a successful bootstrap*/
	return(Err_Null);
}
/*}}}*/
/*}}}*/

/*{{{  main() */

int main(int argc, char **argv)
{
	int		 link;
	char		*my_name;
	char		*its_name;
	char		*link_modes;
	int		 memory_limit	= 0;
	int		 config_size;
	Config		*config;
	MPtr		 image;
	word		 rc;
	int		 i;
	
	/* The hardware configuration word is inherited partly from the	*/
	/* settings for the current processor, although inheriting	*/
	/* flags like replace_idrom may be a bad idea.			*/
	HWConfig	 = GetHWConfig();
	HWConfig	&= ~(HW_PseudoIDROM | HW_ReplaceIDROM);

	/* There must be at least three args, link no. + 2 names	*/
	if (argc < 4) usage();
	while (argv[1][0] == '-')
	{
			/* Test for -l3, specifying the target link no.	*/
		if ((argv[1][1] == 'l') || (argv[1][1] == 'L'))
		{
			if ((argv[1][2] < '0') || (argv[1][2] > '5') || (argv[1][3] != '\0'))
				usage();
			TargetLink = atoi(&(argv[1][2]));
		}
			/* Test for -iIDROM_file			*/
		elif ((argv[1][1] == 'i') || (argv[1][1] == 'I'))
		{
			if (argv[1][2] == '\0') usage();
			read_idrom(&(argv[1][2]));
		}
		else
			usage();

		argc--; argv++;
	}

	/* There must still be at least three args.			*/
	if ((argc < 4) || (argc >> 6)) usage();

	if ((argv[1][0] < '0') || (argv[1][0] > '5') || (argv[1][1] != '\0'))
		usage();
	link = argv[1][0] - '0';

	my_name = argv[2];
	if (my_name[0] != '/') usage();

	its_name = argv[3];
	if (its_name[0] != '/') usage();

	if (argc > 4)
		link_modes = argv[4];
	else
		link_modes = "111111";
	if (strlen(link_modes) != 6) usage();
	for (i = 0; i < 6; i++)
		if ((link_modes[i] < '0') || (link_modes[i] > '3'))
			usage();

	if (argc > 5)
	{
		memory_limit = (int) strtol(argv[5], NULL, 0);
		if (memory_limit == 0) usage();
	}

	if (TargetLink == -1) TargetLink = 0;

	config = build_config(my_name, its_name, link_modes, memory_limit, 
  			&config_size);
	image = determine_image();
	config->ImageSize = MWord_(image,0);

	init_link(link);

	rc = My_BootLink(link, image, config, config_size);
	if (rc == Err_Null)
	{
		set_link(link);
		Delay(OneSec/2);
		rc = c40_check_link(link);
		if (rc == Err_Null) return(EXIT_SUCCESS);
	}
    
	if ((rc & EG_Mask) != EG_Timeout)
		fatal("BootLink failed, bad link");

	if ((rc & EO_Mask) == EO_Message)
		fatal("BootLink failed, timeout sending message to link IOC");

	switch (rc & EO_Mask)
	{
	case Boot_SoftResetSize	:
	case Boot_SoftResetCode	:
		fatal("BootLink failed to send software reset");
     	
	case Boot_BootstrapSize	:
		fatal("BootLink failed to send bootstrap size");
     	
	case Boot_BootstrapCode	:
		fatal("BootLink failed to send bootstrap code");
     	
	case Boot_Clear1 :
	case Boot_Clear2 :
	case Boot_Clear3 :
		fatal("BootLink failed to send memory clear");
     	
	case Boot_ControlByte	:
		fatal("BootLink failed to send control byte");
     	
	case Boot_Image		:
		fatal("BootLink failed to send system image");
     	
	case Boot_ConfigSize	:
		fatal("BootLink failed to send configuration size");
     	
	case Boot_ConfigVector	:
		fatal("BootLink failed to send configuration vector");

	case Boot_ProtocolTail	:
		fatal("BootLink failed to send protocol tail");

	case Boot_Acknowledgement:
		fatal("BootLink failed to receive acknowledgement");
		
	case Boot_Acknowledgement2:
		fatal("BootLink received incorrect acknowledgement");
		
	case Boot_Hwconfig	:
		fatal("BootLink failed to send hardware configuration word");

	case Boot_Idrom		:
		fatal("BootLink failed to send idrom information");
     	
	default :
		fatal("BootLink failed");
	}
}  

/*}}}*/






