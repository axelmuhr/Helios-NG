/*------------------------------------------------------------------------
--                                                                      --
--      H E L I O S   P A R A L L E L   P R O G R A M M I N G		--
--	-----------------------------------------------------		--
--									--
--	  F A R M   C O M P U T A T I O N   L I B R A R Y		--
--	  -----------------------------------------------		--
--									--
--		Copyright (C) 1992, Perihelion Software Ltd.		--
--                        All Rights Reserved.                          --
--                                                                      --
-- farmlib.h								--
--                                                                      --
--	Author:  BLV 22/7/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: farmlib.h,v 1.2 1992/09/09 11:59:26 bart Exp $ */

#ifndef __helios_h
#include <helios.h>
#endif
#ifndef __rmlib_h
#include <rmlib.h>
#endif

/**
*** These are the library configuration options that can be set by the
*** application.
**/
extern	int		FmFloodOption;
#define Fm_Network	1
#define Fm_Domain	2
#define Fm_Fixed	3
#define Fm_Processors	4
extern	RmNetwork	FmSelectedNetwork;
extern	int		FmNumberWorkers;
extern	int		FmCountProcessors(int);

extern	bool		FmFastStack;
extern	bool		FmFastCode;
extern	bool		FmOverloadController;
extern	int		FmJobSize;
extern	int		FmReplySize;
extern	int		FmSeed;
extern	bool		FmFaultHandling;

extern	void		(*FmProducer)(void);
extern	int		FmProducerStack;
extern	void		(*FmConsumer)(void);
extern	int		FmConsumerStack;
extern	void		(*FmWorker)(void);
extern	int		FmWorkerStack;
extern	void		(*FmProvider)(void);
extern	int		FmProviderStack;
extern	void		(*FmControllerInitialise)(void);
extern	void		(*FmWorkerInitialise)(void);
extern	void		(*FmControllerExit)(void);
extern	void		(*FmWorkerExit)(void);

extern	void		FmInitialise(void);

/**
*** Information facilities
**/
extern	int		FmRunningWorkers;
extern	bool		FmIsRunning(int);
extern	int		FmRand(void);
#define Fm_MaxRand	0x7FFFFFFF

extern	int		FmWorkerNumber;
extern	bool		FmInWorker(void);

extern	bool		FmVerbose;
extern	char		*FmVersionNumber;
extern	int		FmDebugFlags;
#define	FmD_IO			0x0001
#define FmD_ControlInit		0x0002
#define FmD_WorkerInit		0x0004
#define FmD_Buffers	 	0x0008
#define FmD_Jobs	 	0x0010
#define FmD_Replies	 	0x0020
#define FmD_Packets	 	0x0040
#define FmD_Info	 	0x0080
#define FmD_Signals	 	0x0100
#define FmD_Distribute	 	0x0200
#define FmD_State	 	0x0400
#define FmD_Exit		0x0800

/**
*** Jobs and replies
**/
extern	void		*FmGetJobBuffer(int size);
extern	void		FmSendJob(int where, bool reply, void *);
#define Fm_All		-1
#define Fm_Any		-2
#define Fm_Ack		TRUE
#define Fm_Nak		FALSE

extern	void		*FmGetReply(int);
extern	void		*FmGetJob(void);
extern	void		*FmGetReplyBuffer(void *, int);
extern	void		FmSendReply(void *);
extern	void		*FmGetInfoRequest(void);
extern	void		*FmGetInfoReplyBuffer(void *, int);
extern	void		FmSendInfoReply(void *);
extern	void		*FmGetInfoRequestBuffer(int);
extern	void		*FmGetInfo(void *);
extern	void		FmFreeInfoReplyBuffer(void *);
