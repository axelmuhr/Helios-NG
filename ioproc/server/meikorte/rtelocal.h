/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1989, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  rtelocal.h                                                          --
--                                                                      --
--  Author:   BLV                                                       --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1989, Perihelion Software Ltd.        */

#ifndef Files_Module
extern struct stat searchbuffer;
#endif

#define exit(a) _exit(a)

#ifndef PATH_MAX
#define PATH_MAX MAXPATHLEN
#endif

typedef long clock_t;
#define CLK_TCK       1	
#define clock() get_unix_time()
#define SEEK_SET	0

#ifndef S_IFIFO
#define S_IFIFO 0160000
#endif


