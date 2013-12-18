/*------------------------------------------------------------------------
--                                                                      --
--			H E L I O S   S E R V E R S			--
--			---------------------------			--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- logger.h								--
--                                                                      --
--	Protocol between logctrl and the logger server.			--
--	Definition of the logger device driver interface.		--
--                                                                      --
--	Author:  BLV 19.3.91						--
--                                                                      --
------------------------------------------------------------------------*/

/* $Header: /hsrc/servers/logger/RCS/logger.h,v 1.1 1991/05/10 11:28:31 bart Exp $ */

#ifndef __device_h
#include <device.h>
#endif

#define	Logger_Revert		1
#define Logger_Redirect		2
#define Logger_Clear		3
#define Logger_Abort		4

typedef struct	LoggerDCB {
	DCB		DCB;
	int		Spare[20];
} LoggerDCB;
