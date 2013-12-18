/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1993, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      parsy.c                                                         --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: parsy.c,v 1.1 1993/09/28 14:24:42 bart Exp $ */
/* Copyright (C) 1989, Perihelion Software Ltd.        			*/

/**
*** THIS CODE IS NOT PART OF THE STANDARD SOURCE RELEASE. IT SHOULD BE
*** USED ONLY IN-HOUSE AND FOR ANY PARSYTEC SOURCE RELEASES.
**/
#define Linklib_Module

#include "../helios.h"
#include "../parsy/link.h"

#define link_fd (link_table[current_link].fildes)

/**
*** These variables are used in module parsy/link.c. The basic idea
*** was that system administrators could easily rebuild the iserver
*** (not the I/O Server) using a small source file lconf.c and the
*** appropriate object files. In the case of the I/O Server it is
*** of course possible to use host.con entries instead.
**/
char	*config_file	= "/usr/etc/transp/trans_conf";
char	*env_config	= "TRANSP_CONFIG";
char	*vme_group_name	= "vmebus";
char	*vme_dev_name	= "/dev/vme32d16";
char	*sbus_dev_name	= "/dev/sbus";
key_t	 sem_link_key	= ((key_t) 0x42424242);
int	 default_tenth	= 88000;
unsigned reset_raise	= 20000;
unsigned reset_fall	= 20000;

/**
*** The Parsytec link I/O code is extremely general, far more so
*** than would be required by all but a very few customers.
*** For example it is possible to plug in several different VME
*** boards into a single Sun. Unfortunately this does not necessarily
*** work. For example, if Hydra had to support both bbk-v1 and bbk-v2
*** boards at once things could get rather confused.
***
*** The actual support is as follows:
*** a) BBK-V1   : one  link adapter  per board, 4 boards
*** b) BBK-V2   : one  link adapter  per board, 4 boards
*** c) VMTM     : four link adapters per board, 1 board
*** d) BBK-V4   : four link adapters per board, 1 board
*** e) MTM-SUN1 : five link adapters per board, 1 board
*** f) MTM-SUN2 : nine link adapters per board, 1 board
*** g) BBK-S4   : four link adapters per board, 1 board
***
*** If you have two types of board in a machine then you need
*** two separate Helios configurations. 
**/

#define Parsy_Max_Link	9
PRIVATE Trans_link	Parsy_links[Parsy_Max_Link];

void parsy_init_link()
{
	char	*tmp;
	int	 tmp_int;
	char	*box;
	int	 i;
	int	 error;

	/* 1) fill in Parsytec-specific stuff from host.con	*/
	if ((tmp = get_config("trans_conf_file")) != NULL)
		config_file	= tmp;
	if ((tmp = get_config("env_config")) != NULL)
		env_config	= tmp;
	if ((tmp = get_config("vme_group_name")) != NULL)
		vme_group_name	= tmp;
	if ((tmp = get_config("vme_dev_name")) != NULL)
		vme_dev_name	= tmp;
	if ((tmp = get_config("sbus_dev_name")) != NULL)
		sbus_dev_name	= tmp;
	if ((tmp_int = get_int_config("sem_link_key")) != Invalid_config)
		sem_link_key	= (key_t) tmp_int;
	if ((tmp_int = get_int_config("default_tenth")) != Invalid_config)
		default_tenth	= tmp_int;
	if ((tmp_int = get_int_config("reset_raise")) != Invalid_config)
		reset_raise	= (unsigned) tmp_int;
	if ((tmp_int = get_int_config("reset_fall")) != Invalid_config)
		reset_fall	= (unsigned) tmp_int;

	/* 2) for each board type, fill in the names of the board.	*/
	/*    Also give an upper limit on the number of boards. Note	*/
	/*    that this name is given to the OpenLink() routine.	*/
	link_table = &(Parsy_links[0]);
	box	= get_config("box");
	if (!mystrcmp(box, "bbk-v1"))
	{
		number_of_links	= 4;
		for (i = 0; i < number_of_links; i++)
			sprintf(Parsy_links[i].link_name, "BBKV1 B%d", i);
	}
	elif (!mystrcmp(box, "bbk-v2"))
	{
		number_of_links	= 4;
		for (i = 0; i < number_of_links; i++)
			sprintf(Parsy_links[i].link_name, "BBKV2 B%d", i);
	}
	elif (!mystrcmp(box, "vmtm"))
	{
		number_of_links	= 4;
		for (i = 0; i < number_of_links; i++)
			sprintf(Parsy_links[i].link_name, "VMTM L%d", i);
	}
	elif (!mystrcmp(box, "bbk-v4"))
	{
		number_of_links	= 4;
		for (i = 0; i < number_of_links; i++)
			sprintf(Parsy_links[i].link_name, "BBKV4 L%d", i);
	}
	elif (!mystrcmp(box, "mtm-sun1"))
	{
		number_of_links	= 5;
		for (i = 0; i < number_of_links; i++)
			sprintf(Parsy_links[i].link_name, "MTMSUN1 L%d", i);
	}
	elif (!mystrcmp(box, "mtm-sun2"))
	{
		number_of_links	= 9;
		for (i = 0; i < number_of_links; i++)
			sprintf(Parsy_links[i].link_name, "MTMSUN2 L%d", i);
	}
	elif (!mystrcmp(box, "bbk-s4"))
	{
		number_of_links	= 4;
		for (i = 0; i < number_of_links; i++)
			sprintf(Parsy_links[i].link_name, "BBKS4 L%d", i);
	}
	else
	{
		ServerDebug("Internal error, inconsistency when initialising board");
		longjmp(exit_jmpbuf, 1);
	}


	/* 3) Fill in flags etc. for every link.				*/
	for (i = 0; i < number_of_links; i++)
	{
		Parsy_links[i].flags		= Link_flags_unused + 
				Link_flags_uninitialised + Link_flags_firsttime;
		Parsy_links[i].connection	= -1;
		Parsy_links[i].fildes		= -1;
		Parsy_links[i].state		= Link_Reset;
		Parsy_links[i].ready		= 0;
	}

	/* 4) Now check that all of these boards/links are actually available.	*/
	/*    More accurately, find out the last one that is available.		*/
	for (i = (number_of_links - 1); i >= 0; i--)
	{
		error = parsy_OpenLink(Parsy_links[i].link_name);
		if ((error >= 0) || (error == EINUSE))
		{
			if (error >= 0) parsy_CloseLink(error);
			number_of_links = i + 1;
			break;
		}
		elif (debugflags & Init_Flag)
			ServerDebug("Parsytec initialisation: error %d opening link %s",
				error, Parsy_links[i].link_name);
			
	}
	if (i < 0)
	{
		ServerDebug("Parsytec initialisation: unable to access any link devices");
		longjmp(exit_jmpbuf, 1);
	}
}
