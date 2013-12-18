/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- rmlib.h								--
--                                                                      --
--	Header file defining the Resource Management library		--
--                                                                      --
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/

/* $Header: /hsrc/include/RCS/rmlib.h,v 1.8 1993/07/09 09:22:04 nickc Exp $ */

#ifndef __rmlib_h
#define __rmlib_h

/**
*** The include files that are required
**/
#ifndef __helios_h
#include <helios.h>
#endif

#ifndef __queue_h
#include <queue.h>
#endif

#ifndef __syslib_h
#include <syslib.h>
#endif

#ifndef __RmStructs
typedef void	*RmProcessor;
typedef void	*RmNetwork;
typedef void	*RmTask;
typedef void	*RmTaskforce;
#endif

extern RmProcessor	RmNewProcessor(void);
extern int		RmFreeProcessor(RmProcessor);
extern unsigned long	RmGetProcessorMemory(RmProcessor);
extern int		RmSetProcessorMemory(RmProcessor, unsigned long);
extern const char	*RmGetProcessorId(RmProcessor);
extern int		RmSetProcessorId(RmProcessor, char *);
extern int		RmGetProcessorPurpose(RmProcessor);
extern int		RmSetProcessorPurpose(RmProcessor, int);
extern int		RmGetProcessorState(RmProcessor);
extern int		RmSetProcessorState(RmProcessor, int);
extern int		RmGetProcessorType(RmProcessor);
extern int		RmSetProcessorType(RmProcessor, int);
extern int		RmGetProcessorOwner(RmProcessor);
extern int		RmWhoAmI(void);
extern const char	*RmWhoIs(int);
extern const char	*RmGetProcessorNucleus(RmProcessor);
extern int		RmSetProcessorNucleus(RmProcessor, char *);
extern int		RmAddProcessorAttribute(RmProcessor, char *);
extern int		RmRemoveProcessorAttribute(RmProcessor, char *);
extern int		RmTestProcessorAttribute(RmProcessor, char *);
extern int		RmCountProcessorAttributes(RmProcessor);
extern int		RmListProcessorAttributes(RmProcessor, char **);

extern RmTask		RmNewTask(void);
extern int		RmFreeTask(RmTask);
extern unsigned long	RmGetTaskMemory(RmTask);
extern int		RmSetTaskMemory(RmTask, unsigned long);
extern const char	*RmGetTaskId(RmTask);
extern int		RmSetTaskId(RmTask, char *);
extern int		RmGetTaskType(RmTask);
extern int		RmSetTaskType(RmTask, int);
extern int		RmSetTaskNative(RmTask);
extern bool		RmIsTaskNative(RmTask);
extern int		RmSetTaskNormal(RmTask);
extern bool		RmIsTaskNormal(RmTask);
extern int		RmGetProgramType(char *);
extern const char	*RmGetTaskCode(RmTask);
extern int		RmSetTaskCode(RmTask, char *);
extern int		RmAddTaskAttribute(RmTask, char *);
extern int		RmRemoveTaskAttribute(RmTask, char *);
extern int		RmTestTaskAttribute(RmTask, char *);
extern int		RmCountTaskAttributes(RmTask);
extern int		RmListTaskAttributes(RmTask, char **);
extern int		RmAddTaskArgument(RmTask, int, char *);
extern const char	*RmGetTaskArgument(RmTask, int);
extern int		RmCountTaskArguments(RmTask);
extern int		RmGetTaskState(RmTask);
extern int		RmSetTaskState(RmTask, int);

extern RmNetwork	RmNewNetwork(void);
extern int		RmFreeNetwork(RmNetwork);
extern const char	*RmGetNetworkId(RmNetwork);
extern int		RmSetNetworkId(RmNetwork, char *);
extern RmProcessor	RmAddtailProcessor(RmNetwork, RmProcessor);
extern RmProcessor	RmAddheadProcessor(RmNetwork, RmProcessor);
extern RmProcessor	RmRemoveProcessor(RmProcessor);
extern RmProcessor	RmPreinsertProcessor(RmProcessor, RmProcessor new_processor);
extern RmProcessor	RmPostinsertProcessor(RmProcessor, RmProcessor new_processor);
extern RmProcessor	RmFirstProcessor(RmNetwork);
extern RmProcessor	RmLastProcessor(RmNetwork);
extern RmProcessor	RmNextProcessor(RmProcessor);
extern RmProcessor	RmPreviousProcessor(RmProcessor);
extern bool		RmIsNetworkEmpty(RmNetwork);
extern int		RmSizeofNetwork(RmNetwork);
extern int		RmCountProcessors(RmNetwork);
extern RmNetwork	RmParentNetwork(RmProcessor);
extern int		RmApplyNetwork(RmNetwork, int (*fn)(RmProcessor, ...), ...);
extern int		RmSearchNetwork(RmNetwork, int (*fn)(RmProcessor, ...), ...);
extern int		RmMergeNetworks(RmNetwork, RmNetwork);
extern int		RmMakeLink(RmProcessor, int, RmProcessor, int);
extern int		RmBreakLink(RmProcessor, int);
extern int		RmCountLinks(RmProcessor);
extern RmProcessor	RmFollowLink(RmProcessor, int, int *);
extern int		RmGetLinkFlags(RmProcessor, int);
extern RmNetwork	RmRootNetwork(RmProcessor);
extern bool		RmIsNetwork(RmProcessor);
extern bool		RmIsProcessor(RmProcessor);

extern RmTaskforce	RmNewTaskforce(void);
extern int		RmFreeTaskforce(RmTaskforce);
extern const char	*RmGetTaskforceId(RmTaskforce);
extern int		RmSetTaskforceId(RmTaskforce, char *);
extern int		RmGetTaskforceState(RmTaskforce);
extern int		RmSetTaskforceState(RmTaskforce, int);
extern RmTask		RmAddtailTask(RmTaskforce, RmTask);
extern RmTask		RmAddheadTask(RmTaskforce, RmTask);
extern RmTask		RmRemoveTask(RmTask);
extern RmTask		RmPreinsertTask(RmTask, RmTask new_task);
extern RmTask		RmPostinsertTask(RmTask, RmTask new_task);
extern RmTask		RmFirstTask(RmTaskforce);
extern RmTask		RmLastTask(RmTaskforce);
extern RmTask		RmNextTask(RmTask);
extern RmTask		RmPreviousTask(RmTask);
extern bool		RmIsTaskforceEmpty(RmTaskforce);
extern int		RmSizeofTaskforce(RmTaskforce);
extern int		RmCountTasks(RmTaskforce);
extern RmTaskforce	RmParentTaskforce(RmTask);
extern int 		RmApplyTaskforce(RmTaskforce, int (*fn)(RmTask, ...), ...);
extern int 		RmSearchTaskforce(RmTaskforce, int (*fn)(RmTask, ...), ...);
extern int		RmMakeChannel(RmTask, int, RmTask, int);
extern int		RmBreakChannel(RmTask, int);
extern int		RmCountChannels(RmTask);
extern RmTask		RmFollowChannel(RmTask, int, int *);
extern int		RmGetChannelFlags(RmTask, int);
extern int		RmConnectChannelToFile(RmTask, int, char *, int);
extern const char	*RmFollowChannelToFile(RmTask, int, int *);
extern RmTaskforce	RmRootTaskforce(RmTask);
extern bool		RmIsTaskforce(RmTask);
extern bool		RmIsTask(RmTask);

extern RmNetwork	RmGetNetwork(void);
extern RmNetwork	RmGetNetworkAndHardware(void);
extern int		RmLastChangeNetwork(void);
extern RmNetwork	RmGetDomain(void);
extern int		RmLastChangeDomain(void);
extern RmProcessor	RmObtainProcessor(RmProcessor);
extern int		RmSetProcessorShareable(RmProcessor);
extern int		RmSetProcessorExclusive(RmProcessor);
extern bool		RmIsProcessorShareable(RmProcessor);
extern bool		RmIsProcessorExclusive(RmProcessor);
extern int		RmSetProcessorTemporary(RmProcessor);
extern int		RmSetProcessorPermanent(RmProcessor);
extern bool		RmIsProcessorTemporary(RmProcessor);
extern bool		RmIsProcessorPermanent(RmProcessor);
extern int		RmSetProcessorCancelled(RmProcessor Processor);
extern int		RmSetProcessorBooked(RmProcessor Processor);
extern int		RmReleaseProcessor(RmProcessor);
extern RmNetwork	RmObtainNetwork(RmNetwork, bool exact, int *);
extern RmProcessor	RmFindMatchingProcessor(RmProcessor, RmNetwork);
extern int		RmReleaseNetwork(RmNetwork);

extern	int		RmRead(char *filename, RmNetwork *, RmTaskforce *);
extern	int		RmWrite(char *filename, RmNetwork, RmTaskforce);
extern	Object		*RmMapProcessorToObject(RmProcessor);
extern	char		*RmBuildProcessorName(char *buffer, RmProcessor);
extern	RmProcessor	RmLookupProcessor(RmNetwork, char *);
extern	RmProcessor	RmInsertProcessor(RmNetwork, RmProcessor);
extern	RmNetwork	RmGetNetworkHierarchy(void);

extern	int		RmSetProcessorPrivate(RmProcessor, int);
extern	int		RmGetProcessorPrivate(RmProcessor);
extern	int		RmSetTaskPrivate(RmTask, int);
extern	int		RmGetTaskPrivate(RmTask);
extern	int		RmSetNetworkPrivate(RmNetwork, int);
extern	int		RmGetNetworkPrivate(RmNetwork);
extern	int		RmSetTaskforcePrivate(RmTaskforce, int);
extern	int		RmGetTaskforcePrivate(RmTaskforce);
extern	int		RmSetProcessorPrivate2(RmProcessor, int);
extern	int		RmGetProcessorPrivate2(RmProcessor);
extern	int		RmSetTaskPrivate2(RmTask, int);
extern	int		RmGetTaskPrivate2(RmTask);
extern	int		RmSetNetworkPrivate2(RmNetwork, int);
extern	int		RmGetNetworkPrivate2(RmNetwork);
extern	int		RmSetTaskforcePrivate2(RmTaskforce, int);
extern	int		RmGetTaskforcePrivate2(RmTaskforce);

extern	int		RmMapTask(RmProcessor, RmTask);
extern	int		RmUnmapTask(RmProcessor, RmTask);
extern	RmProcessor	RmFollowTaskMapping(RmNetwork, RmTask);
extern	int		RmApplyMappedTasks(RmProcessor, int (*fn)(RmTask, ...), ...);
extern	int		RmSearchMappedTasks(RmProcessor, int (*fn)(RmTask, ...), ...);
extern	int		RmCountMappedTasks(RmProcessor);
extern	bool		RmIsMappedTask(RmProcessor, RmTask);

extern	int		RmGetLinkMode(RmProcessor, int link, int *mode);
extern	int		RmSetLinkMode(RmProcessor, int link, int mode);

extern	int		RmErrno;
extern	int		RmApplyProcessors(RmNetwork, int (*fn)(RmProcessor, ...), ...);
extern	int		RmApplyTasks(RmTaskforce, int (*fn)(RmTask, ...), ...);
extern	bool		RmAreProcessorsPossible(int, RmProcessor *, bool, bool);
extern	int		RmClearNetworkError(RmNetwork);
extern	int		RmClearProcessorError(RmProcessor);
extern	int		RmClearTaskError(RmTask);
extern	int		RmClearTaskError(RmTask);
extern	RmTask		RmExecuteTask(RmProcessor, RmTask, char **);
extern	RmTaskforce	RmExecuteTaskforce(RmNetwork, RmTaskforce, char **);
extern	int		RmGetNetworkError(RmNetwork);
extern	const	char	*RmGetProcessorAttribute(RmProcessor, char *);
extern	int		RmGetProcessorControl(RmProcessor);
extern	int		RmGetProcessorError(RmProcessor);
extern	const	char	*RmGetTaskAttribute(RmTask, char *);
extern	int		RmGetTaskError(RmTask);
extern	int		RmGetTaskforceError(RmTaskforce);
extern	int		RmGetTaskforceReturncode(RmTaskforce);
extern	int		RmGetTaskReturncode(RmTask);
extern	bool		RmIsLinkPossible(RmProcessor, int, RmProcessor, int);
extern	bool		RmIsNetworkPossible(RmNetwork, bool, bool);
extern	bool		RmIsProcessorFree(RmProcessor);
extern	bool		RmIsTaskRunning(RmTask);
extern	bool		RmIsTaskforceRunning(RmTaskforce);
extern	int		RmLeaveTask(RmTask);
extern	int		RmLeaveTaskforce(RmTaskforce);
extern	RmTask		RmFindMatchingTask(RmTask, RmTaskforce);
extern	RmTaskforce	RmConvertTaskToTaskforce(RmTask, RmTaskforce);

extern	const	char	*RmMapErrorToString(int);
extern	RmNetwork	RmObtainProcessors(int, RmProcessor *, bool exact, int *);
extern	int		RmReadfd(int, RmNetwork *, RmTaskforce *);
extern	int		RmReadfdNetwork(int, RmNetwork *);
extern	int		RmReadfdNetworkOnly(int, RmNetwork *);
extern	int		RmReadfdProcessor(int, RmProcessor *);
extern	int		RmReadfdTask(int, RmTask *);
extern	int		RmReadfdTaskforce(int, RmTaskforce *);
extern	int		RmReadfdTaskforceOnly(int, RmTaskforce *);
extern	int		RmRebootNetwork(RmNetwork);
extern	int		RmRebootProcessors(int, RmProcessor *);
extern	int		RmReconfigureNetwork(RmNetwork, bool, bool);
extern	int		RmReconfigureProcessors(int, RmProcessor *, bool, bool);
extern	int		RmResetNetwork(RmNetwork);
extern	int		RmResetProcessors(int, RmProcessor *);
extern	int		RmRevertNetwork(RmNetwork);
extern	int		RmRevertProcessors(int, RmProcessor *);
extern	int		RmSearchProcessors(RmNetwork, int (*fn)(RmProcessor, ...), ...);
extern	int		RmSearchTasks(RmTaskforce, int (*fn)(RmTask, ...), ...);
extern	int		RmSendTaskSignal(RmTask, int);
extern	int		RmSendTaskforceSignal(RmTaskforce, int);
extern	int		RmSetNetworkNative(RmNetwork);
extern	int		RmSetProcessorsNative(int, RmProcessor *);
extern	int		RmWaitforTask(RmTask);
extern	int		RmWaitforTaskforce(RmTaskforce);
extern	int		RmWaitforAnyTask(RmTaskforce);
extern	int		RmWaitforTasks(int, RmTask *);
extern	int		RmWritefd(int, RmNetwork, RmTaskforce);
extern	int		RmWritefdNetwork(int, RmNetwork);
extern	int		RmWritefdNetworkOnly(int, RmNetwork);
extern	int		RmWritefdProcessor(int, RmProcessor);
extern	int		RmWritefdTask(int, RmTask);
extern	int		RmWritefdTaskforce(int, RmTaskforce);
extern	int		RmWritefdTaskforceOnly(int, RmTaskforce);
extern	int		RmGetProcessorSession(RmProcessor);
extern	int		RmGetProcessorApplication(RmProcessor);
extern	int		RmGetSession(void);
extern	int		RmGetApplication(void);
extern	int		RmReportProcessor(RmProcessor);
extern	RmTaskforce	RmGetTaskforce(void);
extern	RmTask		RmGetTask(void);

/* Processor purposes */
#define RmP_System	0x001	/* reserved for system use only		*/
#define RmP_User	0x000	/* default, available to users		*/

#define RmP_Helios	0x002	/* Processor runs Helios nucleus	*/
#define RmP_Normal	0x002	/* An alias for the above		*/
#define RmP_IO		0x004	/* I/O Processor only, cannot run applications */
#define RmP_Native	0x006	/* Processor not running nucleus	*/
#define RmP_Router	0x008	/* Processor does message routing only	*/
#define RmP_Mask	0x00E	/* Mask off the purpose bits		*/

/* Processor types */
#define RmT_Unknown		 0
#define RmT_Default		 1
#define RmT_Known		12
#define RmT_T800		 2
#define RmT_T414		 3 
#define RmT_T425		 4
#define RmT_T400		 5
#define RmT_T212		 6
#define RmT_H1			 7
#define RmT_T9000		 7	/* alias */
#define RmT_i860		 8
#define RmT_Arm		 	 9
#define RmT_680x0		10
#define RmT_C40			11
extern  char *RmT_Names[RmT_Known];

/* Processor states */
#define RmS_ShouldBeReset	0x00010
#define RmS_Reset		0x00020
#define RmS_Booting		0x00040
#define	RmS_AutoBoot		0x00080
#define RmS_Running		0x00100
#define RmS_Suspicious		0x00200
#define	RmS_Crashed		0x00400
#define RmS_Dead		0x00800
#define RmS_Special		0x01000
#define RmS_Finished		0x02000

/* Processor control information */
#define RmC_Native		0x001
#define RmC_Reset		0x002
#define RmC_PossibleReset	0x004
#define RmC_FixedMapping	0x008
#define RmC_FixedLinks		0x010
#define RmC_Reclaim		0x020

/* RmLib error codes */
#define RmE_Success		    0
#define RmE_NotProcessor	0x0c0	/* processor argument is invalid */
#define	RmE_NotTask		0x0c1   /* ... */
#define RmE_NotNetwork		0x0c2
#define	RmE_NotTaskforce	0x0c3
#define RmE_WrongNetwork	0x0c4	/* Attempting to make invalid */
#define RmE_WrongTaskforce	0x0c5	/* connection */
#define RmE_InUse		0x0c6	/* processor already part of network */
#define RmE_Corruption		0x0c7	/* system memory corruption detected */
#define RmE_ReadOnly		0x0c8	/* write-operation illegal */
#define RmE_BadArgument		0x0c9	/* e.g. invalid processor type */
#define RmE_NoMemory		0x0ca	/* Malloc() failure */
#define RmE_NotFound		0x0cb	/* a search failed, e.g. IsAttr */
#define RmE_TooLong		0x0cc	/* a string argument is too long */
#define RmE_NotRootNetwork	0x0cd
#define RmE_NoAccess		0x0ce
#define RmE_OldStyle		0x0cf
#define RmE_BadFile		0x0d0
#define RmE_CommsBreakdown	0x0d1
#define RmE_Skip		0x0d2
#define	RmE_NotRootTaskforce	0x0d3
#define	RmE_MissingServer	0x0d4
#define	RmE_PartialSuccess	0x0d5
#define RmE_BadLink		0x0d6
#define RmE_BadProcessor	0x0d7
#define RmE_BadChannel		0x0d8
#define RmE_YouMustBeJoking	0x0d9
#define RmE_ServerMemory	0x0da	/* server ran out of memory */
#define RmE_NotPossible		0x0db	/* hardware limitation */
#define RmE_NoResource		0x0dc	/* insufficient mapping resource */

/* Miscellaneous constants */
#define RmM_ExternalProcessor	((RmProcessor) 1)
#define RmM_ExternalTask	((RmTask) 1)
#define RmM_NoProcessor		((RmProcessor) NULL)
#define RmM_NoTask		((RmTask) NULL)
#define RmM_AnyLink		(-1)
#define RmM_AnyChannel		(-1)
#define RmM_NoOwner		0

/* Flags of various sort */
#define RmF_Configurable	0x01
#define RmF_AnyLink		0x02
#define RmF_AnyChannel		0x02
#define RmF_ToFile		0x04
#define RmF_Suspicious		0x08
#define RmF_Delayed		0x10	/* link not yet ready */

/* Hardware facilities */
#define RmH_ResetDriver		0x01
#define RmH_ResetCommand	0x02
#define RmH_ConfigureDriver	0x03

/**
*** Link modes
**/
#define	RmL_NotConnected	0x000
#define	RmL_Dumb		0x001
#define RmL_Intelligent		0x002
#define RmL_Running		0x002	/* Alias */
#define RmL_Pending		0x003
#define RmL_Dead		0x004
#define RmL_Report		0x080

/**
*** Various special owners
**/
#define	RmO_SystemPool	-1
#define RmO_FreePool	-1	/* an alias */
#define	RmO_System	-2
#define	RmO_Cleaners	-3
#define RmO_Graveyard	-4

/**
*** The following stuff is not yet documented but is needed to generate
*** resource maps.
***
*** This structure is used to take care of device drivers and the like.
**/
typedef struct RmHardwareFacility {
	Node		Node;
	int		Type;	/* Reset driver, reset command, ... */
	int		NumberProcessors;
	void		*Device; 	/* for internal use */
	RmProcessor	Essential;	/* ditto */
	int		Spare[2];
	char		Name[64];	/* pa_ra.d, or -e /helios/net/tr_reset*/
	char		Option[64];	/* Processor name or driver option */
	RmProcessor	*Processors;
} RmHardwareFacility;

extern int		RmAddHardwareFacility(RmNetwork, RmHardwareFacility *);
extern int		RmApplyHardwareFacilities(RmNetwork,
			int (*fn)(RmHardwareFacility *, ...), ...);
extern int		RmSearchHardwareFacilities(RmNetwork,
			int (*fn)(RmHardwareFacility *, ...), ...);

#endif   /* __rmlib_h */


