/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1992, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  hlocal.h                                                            --
--                                                                      --
--  Author:  BLV                                                        --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1987, Perihelion Software Ltd.        */

#ifndef Files_Module
extern struct stat searchbuffer;
#endif

extern void check_helios_name(char *);
extern void unix_initialise_devices(void);

#define InitList	my_InitList
#define WalkList	my_WalkList
#define AddHead		my_AddHead
#define AddTail		my_AddTail
#define Wander		my_Wander
#define PreInsert	my_PreInsert
#define PostInsert	my_PostInsert
#define InitSemaphore	my_InitSemaphore
#define Wait		my_Wait
#define Signal		my_Signal

