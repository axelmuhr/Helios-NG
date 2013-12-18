/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils.h 								--
--									--
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/

/* Rcsid = $Header: /hsrc/network/RCS/netutils.h,v 1.10 1992/10/11 13:33:07 bart Exp $ */

#ifndef __netutils_h
#define __netutils_h

#ifndef __rmlib_h
#include "rmlib.h"
#endif

#ifndef __syslib_h
#include <syslib.h>
#endif

extern int	mystrcmp(char *, char *);

extern char	*get_config(char *pattern, char **environ);
extern int	get_int_config(char *string, char **environ);
#define		Invalid_config 0x5482ab32

	/* This routine can be called by itself			*/
extern void	PrintNetwork(RmNetwork Network);
	/* These routines can be used for individual processors	*/
extern void	init_PrintNetwork(bool filter);
extern void	tidy_PrintNetwork(void);
extern void	PrintProcessor(RmProcessor processor, ...);
extern void	PrintSubnet(RmNetwork network, int);

extern void	PrintTaskforce(RmTaskforce, RmNetwork domain);

extern Object	*PseudoObject(char *Name, Capability *cap);

extern void	PatchMalloc(void);
extern void	PatchObjects(void);
extern void	ShowHeap(void);
extern void	PatchPorts(void);

extern void	Init_qRoutines(int maxqThreads, int delay);
extern Object	*qLocate(Object *obj, string name);
extern int	qServerInfo(Object *, BYTE *);

typedef struct  MRSW_Info {
	int	ar;
	int	rr;
	int	aw;
	int	rw;
	int	sw;
} MRSW_Info;

extern void	MRSW_Init(void);
extern void	MRSW_GetRead(void);
extern void	MRSW_GetWrite(void);
extern void	MRSW_SwitchWrite(void);
extern void	MRSW_SwitchRead(void);
extern void	MRSW_FreeRead(void);
extern void	MRSW_FreeWrite(void);
extern void	MRSW_GetInfo(MRSW_Info *);

extern int	Util_CountLinks(Object *processor);

extern	void	PatchIOC(int);
extern	void	ShowIOC(void);

extern	void	DisplayInfo(RmNetwork);
extern	void	MonitorNetwork(RmNetwork, int delay);

extern	void	EncodePassword(char *text, char *output);
#define Passwd_Max	12
#endif

