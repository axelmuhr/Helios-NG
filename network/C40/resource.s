        align
        module  24
.ModStart:
        word    0x60f160f1
		word modsize
	blkb    31, "RmLib"
	byte 0
	word	modnum
	word	1000
		word	datasymb(.MaxData)
        init
	word	codesymb(.MaxCodeP)
RmLib.library:
		global	RmLib.library
		align
		init
				CMPI	2, R0
				Beq	_CodeTableInit
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(0), AR0)
	lsh	-2, AR0
	addi	IR0, AR0
				LDI	R11,	R7
			data _RmErrno, 4
			global _RmErrno 
				codetable _RmNewProcessor
				global _RmNewProcessor
				codetable _RmFreeProcessor
				global _RmFreeProcessor
				codetable _RmGetProcessorMemory
				global _RmGetProcessorMemory
				codetable _RmSetProcessorMemory
				global _RmSetProcessorMemory
				codetable _RmGetProcessorId
				global _RmGetProcessorId
				codetable _RmSetProcessorId
				global _RmSetProcessorId
				codetable _RmGetProcessorPurpose
				global _RmGetProcessorPurpose
				codetable _RmSetProcessorPurpose
				global _RmSetProcessorPurpose
				codetable _RmGetProcessorState
				global _RmGetProcessorState
				codetable _RmGetProcessorType
				global _RmGetProcessorType
				codetable _RmSetProcessorType
				global _RmSetProcessorType
				data _RmT_Names, 48
			global _RmT_Names 
				codetable _RmGetProcessorOwner
				global _RmGetProcessorOwner
				codetable _RmWhoAmI
				global _RmWhoAmI
				codetable _RmGetProcessorNucleus
				global _RmGetProcessorNucleus
				codetable _RmSetProcessorNucleus
				global _RmSetProcessorNucleus
				codetable _RmAddProcessorAttribute
				global _RmAddProcessorAttribute
				codetable _RmRemoveProcessorAttribute
				global _RmRemoveProcessorAttribute
				codetable _RmTestProcessorAttribute
				global _RmTestProcessorAttribute
				codetable _RmCountProcessorAttributes
				global _RmCountProcessorAttributes
				codetable _RmListProcessorAttributes
				global _RmListProcessorAttributes
				codetable _RmNewTask
				global _RmNewTask
				codetable _RmFreeTask
				global _RmFreeTask
				codetable _RmGetTaskMemory
				global _RmGetTaskMemory
				codetable _RmSetTaskMemory
				global _RmSetTaskMemory
				codetable _RmGetTaskId
				global _RmGetTaskId
				codetable _RmSetTaskId
				global _RmSetTaskId
				codetable _RmGetTaskCode
				global _RmGetTaskCode
				codetable _RmSetTaskCode
				global _RmSetTaskCode
				codetable _RmGetTaskType
				global _RmGetTaskType
				codetable _RmSetTaskType
				global _RmSetTaskType
				codetable _RmGetProgramType
				global _RmGetProgramType
				codetable _RmAddTaskAttribute
				global _RmAddTaskAttribute
				codetable _RmRemoveTaskAttribute
				global _RmRemoveTaskAttribute
				codetable _RmTestTaskAttribute
				global _RmTestTaskAttribute
				codetable _RmCountTaskAttributes
				global _RmCountTaskAttributes
				codetable _RmListTaskAttributes
				global _RmListTaskAttributes
				codetable _RmNewNetwork
				global _RmNewNetwork
				codetable _RmFreeNetwork
				global _RmFreeNetwork
				codetable _RmGetNetworkId
				global _RmGetNetworkId
				codetable _RmSetNetworkId
				global _RmSetNetworkId
				codetable _RmAddtailProcessor
				global _RmAddtailProcessor
				codetable _RmAddheadProcessor
				global _RmAddheadProcessor
				codetable _RmRemoveProcessor
				global _RmRemoveProcessor
				codetable _RmPreinsertProcessor
				global _RmPreinsertProcessor
				codetable _RmPostinsertProcessor
				global _RmPostinsertProcessor
				codetable _RmFirstProcessor
				global _RmFirstProcessor
				codetable _RmLastProcessor
				global _RmLastProcessor
				codetable _RmNextProcessor
				global _RmNextProcessor
				codetable _RmPreviousProcessor
				global _RmPreviousProcessor
				codetable _RmIsNetworkEmpty
				global _RmIsNetworkEmpty
				codetable _RmSizeofNetwork
				global _RmSizeofNetwork
				codetable _RmParentNetwork
				global _RmParentNetwork
				codetable _RmApplyNetwork
				global _RmApplyNetwork
				codetable _RmSearchNetwork
				global _RmSearchNetwork
				codetable _RmMergeNetworks
				global _RmMergeNetworks
				codetable _RmMakeLink
				global _RmMakeLink
				codetable _RmBreakLink
				global _RmBreakLink
				codetable _RmApplyProcessors
				global _RmApplyProcessors
				codetable _RmCountLinks
				global _RmCountLinks
				codetable _RmFollowLink
				global _RmFollowLink
				codetable _RmGetLinkFlags
				global _RmGetLinkFlags
				codetable _RmRootNetwork
				global _RmRootNetwork
				codetable _RmIsNetwork
				global _RmIsNetwork
				codetable _RmIsProcessor
				global _RmIsProcessor
				codetable _RmNewTaskforce
				global _RmNewTaskforce
				codetable _RmFreeTaskforce
				global _RmFreeTaskforce
				codetable _RmGetTaskforceId
				global _RmGetTaskforceId
				codetable _RmSetTaskforceId
				global _RmSetTaskforceId
				codetable _RmSetTaskNative
				global _RmSetTaskNative
				codetable _RmIsTaskNative
				global _RmIsTaskNative
				codetable _RmGetProcessorPrivate2
				global _RmGetProcessorPrivate2
				codetable _RmSetProcessorPrivate2
				global _RmSetProcessorPrivate2
				codetable _RmSetTaskNormal
				global _RmSetTaskNormal
				codetable _RmAddtailTask
				global _RmAddtailTask
				codetable _RmAddheadTask
				global _RmAddheadTask
				codetable _RmRemoveTask
				global _RmRemoveTask
				codetable _RmPreinsertTask
				global _RmPreinsertTask
				codetable _RmPostinsertTask
				global _RmPostinsertTask
				codetable _RmFirstTask
				global _RmFirstTask
				codetable _RmLastTask
				global _RmLastTask
				codetable _RmNextTask
				global _RmNextTask
				codetable _RmPreviousTask
				global _RmPreviousTask
				codetable _RmIsTaskforceEmpty
				global _RmIsTaskforceEmpty
				codetable _RmSizeofTaskforce
				global _RmSizeofTaskforce
				codetable _RmParentTaskforce
				global _RmParentTaskforce
				codetable _RmApplyTaskforce
				global _RmApplyTaskforce
				codetable _RmSearchTaskforce
				global _RmSearchTaskforce
				codetable _RmMakeChannel
				global _RmMakeChannel
				codetable _RmBreakChannel
				global _RmBreakChannel
				codetable _RmSearchProcessors
				global _RmSearchProcessors
				codetable _RmCountChannels
				global _RmCountChannels
				codetable _RmFollowChannel
				global _RmFollowChannel
				codetable _RmGetChannelFlags
				global _RmGetChannelFlags
				codetable _RmRootTaskforce
				global _RmRootTaskforce
				codetable _RmIsTaskforce
				global _RmIsTaskforce
				codetable _RmIsTask
				global _RmIsTask
				codetable _RmGetNetwork
				global _RmGetNetwork
				codetable _RmLastChangeNetwork
				global _RmLastChangeNetwork
				codetable _RmGetDomain
				global _RmGetDomain
				codetable _RmLastChangeDomain
				global _RmLastChangeDomain
				codetable _RmObtainProcessor
				global _RmObtainProcessor
				codetable _RmSetProcessorShareable
				global _RmSetProcessorShareable
				codetable _RmSetProcessorExclusive
				global _RmSetProcessorExclusive
				codetable _RmIsProcessorShareable
				global _RmIsProcessorShareable
				codetable _RmIsProcessorExclusive
				global _RmIsProcessorExclusive
				codetable _RmSetProcessorTemporary
				global _RmSetProcessorTemporary
				codetable _RmSetProcessorPermanent
				global _RmSetProcessorPermanent
				codetable _RmIsProcessorTemporary
				global _RmIsProcessorTemporary
				codetable _RmIsProcessorPermanent
				global _RmIsProcessorPermanent
				codetable _RmReleaseProcessor
				global _RmReleaseProcessor
				codetable _RmObtainNetwork
				global _RmObtainNetwork
				codetable _RmFindMatchingProcessor
				global _RmFindMatchingProcessor
				codetable _RmReleaseNetwork
				global _RmReleaseNetwork
				codetable _RmGetNetworkPrivate2
				global _RmGetNetworkPrivate2
				codetable _RmSetNetworkPrivate2
				global _RmSetNetworkPrivate2
				codetable _RmGetTaskPrivate2
				global _RmGetTaskPrivate2
				codetable _RmSetTaskPrivate2
				global _RmSetTaskPrivate2
				codetable _RmGetTaskforcePrivate2
				global _RmGetTaskforcePrivate2
				codetable _RmSetTaskforcePrivate2
				global _RmSetTaskforcePrivate2
				codetable _RmApplyTasks
				global _RmApplyTasks
				codetable _RmApplyHardwareFacilities
				global _RmApplyHardwareFacilities
				codetable _RmSearchHardwareFacilities
				global _RmSearchHardwareFacilities
				codetable _RmAddHardwareFacility
				global _RmAddHardwareFacility
				codetable _RmFindTableEntry
				global _RmFindTableEntry
				codetable _RmFindUid
				global _RmFindUid
				codetable _RmNextFreeUid
				global _RmNextFreeUid
				codetable _RmReleaseUid
				global _RmReleaseUid
				codetable _RmObtainUid
				global _RmObtainUid
				codetable _RmExtendFreeQ
				global _RmExtendFreeQ
				codetable _RmRead
				global _RmRead
				codetable _RmWrite
				global _RmWrite
				codetable _RmReadStream
				global _RmReadStream
				codetable _RmWriteStream
				global _RmWriteStream
				codetable _RmReadNetwork
				global _RmReadNetwork
				codetable _RmWriteNetwork
				global _RmWriteNetwork
				codetable _RmReadProcessor
				global _RmReadProcessor
				codetable _RmWriteProcessor
				global _RmWriteProcessor
				codetable _RmReadTaskforce
				global _RmReadTaskforce
				codetable _RmWriteTaskforce
				global _RmWriteTaskforce
				codetable _RmReadTask
				global _RmReadTask
				codetable _RmWriteTask
				global _RmWriteTask
				codetable _RmNewJob
				global _RmNewJob
				codetable _RmFinishedJob
				global _RmFinishedJob
				codetable _RmOpenServer
				global _RmOpenServer
				codetable _RmCloseServer
				global _RmCloseServer
				codetable _RmLockWrite
				global _RmLockWrite
				codetable _RmUnlockWrite
				global _RmUnlockWrite
				codetable _RmLockRead
				global _RmLockRead
				codetable _RmSwitchLockRead
				global _RmSwitchLockRead
				codetable _RmUnlockRead
				global _RmUnlockRead
			data _RmNetworkServer, 4
			global _RmNetworkServer 
			data _RmSessionManager, 4
			global _RmSessionManager 
			data _RmParent, 4
			global _RmParent 
				codetable _RmAddTaskArgument
				global _RmAddTaskArgument
				codetable _RmGetTaskArgument
				global _RmGetTaskArgument
				codetable _RmCountTaskArguments
				global _RmCountTaskArguments
				codetable _RmConnectChannelToFile
				global _RmConnectChannelToFile
				codetable _RmFollowChannelToFile
				global _RmFollowChannelToFile
				codetable _RmGetProcessorPrivate
				global _RmGetProcessorPrivate
				codetable _RmSetProcessorPrivate
				global _RmSetProcessorPrivate
				codetable _RmGetTaskPrivate
				global _RmGetTaskPrivate
				codetable _RmSetTaskPrivate
				global _RmSetTaskPrivate
				codetable _RmGetTaskforcePrivate
				global _RmGetTaskforcePrivate
				codetable _RmSetTaskforcePrivate
				global _RmSetTaskforcePrivate
				codetable _RmGetNetworkPrivate
				global _RmGetNetworkPrivate
				codetable _RmSetNetworkPrivate
				global _RmSetNetworkPrivate
				codetable _RmSetProcessorState
				global _RmSetProcessorState
				codetable _RmGetProcessorUid
				global _RmGetProcessorUid
				codetable _RmGetTaskUid
				global _RmGetTaskUid
				codetable _RmFindLink
				global _RmFindLink
				codetable _RmCountProcessors
				global _RmCountProcessors
				codetable _RmCountTasks
				global _RmCountTasks
				codetable _RmGetProcessorCapability
				global _RmGetProcessorCapability
				codetable _RmWhoIs
				global _RmWhoIs
				codetable _RmGetNetworkHierarchy
				global _RmGetNetworkHierarchy
				data _RmLib_Cap, 8
			global _RmLib_Cap 
				codetable _RmMapProcessorToObject
				global _RmMapProcessorToObject
				codetable _RmGetObjectAttribute
				global _RmGetObjectAttribute
				codetable _RmAddObjectAttribute
				global _RmAddObjectAttribute
				codetable _RmInsertProcessor
				global _RmInsertProcessor
			data _RmStartSearchHere, 4
			global _RmStartSearchHere 
				codetable _RmRemoveObjectAttribute
				global _RmRemoveObjectAttribute
				codetable _RmGetNetworkAndHardware
				global _RmGetNetworkAndHardware
				codetable _RmGetLinkMode
				global _RmGetLinkMode
				codetable _RmSetLinkMode
				global _RmSetLinkMode
				codetable _RmMapTask
				global _RmMapTask
				codetable _RmUnmapTask
				global _RmUnmapTask
				codetable _RmFollowTaskMapping
				global _RmFollowTaskMapping
				codetable _RmApplyMappedTasks
				global _RmApplyMappedTasks
				codetable _RmSearchMappedTasks
				global _RmSearchMappedTasks
				codetable _RmCountMappedTasks
				global _RmCountMappedTasks
				codetable _RmIsMappedTask
				global _RmIsMappedTask
				codetable _RmAreProcessorsPossible
				global _RmAreProcessorsPossible
				codetable _RmClearNetworkError
				global _RmClearNetworkError
				codetable _RmClearProcessorError
				global _RmClearProcessorError
			data _RmProgram, 4
			global _RmProgram 
				codetable _RmClearTaskError
				global _RmClearTaskError
				codetable _RmClearTaskforceError
				global _RmClearTaskforceError
				codetable _RmExecuteTask
				global _RmExecuteTask
				codetable _RmExecuteTaskforce
				global _RmExecuteTaskforce
				codetable _RmGetNetworkError
				global _RmGetNetworkError
				codetable _RmGetProcessorError
				global _RmGetProcessorError
				codetable _RmGetTaskError
				global _RmGetTaskError
				codetable _RmGetTaskforceError
				global _RmGetTaskforceError
				codetable _RmGetProcessorAttribute
				global _RmGetProcessorAttribute
				codetable _RmGetProcessorControl
				global _RmGetProcessorControl
				codetable _RmGetTaskAttribute
				global _RmGetTaskAttribute
				codetable _RmGetTaskReturncode
				global _RmGetTaskReturncode
				codetable _RmGetTaskforceReturncode
				global _RmGetTaskforceReturncode
				codetable _RmIsLinkPossible
				global _RmIsLinkPossible
				codetable _RmIsNetworkPossible
				global _RmIsNetworkPossible
				codetable _RmIsProcessorFree
				global _RmIsProcessorFree
				codetable _RmIsTaskNormal
				global _RmIsTaskNormal
				codetable _RmIsTaskRunning
				global _RmIsTaskRunning
				codetable _RmIsTaskforceRunning
				global _RmIsTaskforceRunning
				codetable _RmLeaveTask
				global _RmLeaveTask
				codetable _RmLeaveTaskforce
				global _RmLeaveTaskforce
				codetable _RmMapErrorToString
				global _RmMapErrorToString
				codetable _RmObtainProcessors
				global _RmObtainProcessors
				codetable _RmReadfd
				global _RmReadfd
				codetable _RmReadfdNetwork
				global _RmReadfdNetwork
				codetable _RmReadfdNetworkOnly
				global _RmReadfdNetworkOnly
				codetable _RmReadfdProcessor
				global _RmReadfdProcessor
				codetable _RmReadfdTask
				global _RmReadfdTask
				codetable _RmReadfdTaskforce
				global _RmReadfdTaskforce
				codetable _RmReadfdTaskforceOnly
				global _RmReadfdTaskforceOnly
				codetable _RmRebootNetwork
				global _RmRebootNetwork
				codetable _RmRebootProcessors
				global _RmRebootProcessors
				codetable _RmReconfigureNetwork
				global _RmReconfigureNetwork
				codetable _RmReconfigureProcessors
				global _RmReconfigureProcessors
				codetable _RmResetNetwork
				global _RmResetNetwork
				codetable _RmResetProcessors
				global _RmResetProcessors
				codetable _RmRevertNetwork
				global _RmRevertNetwork
				codetable _RmRevertProcessors
				global _RmRevertProcessors
				codetable _RmSearchTasks
				global _RmSearchTasks
				codetable _RmSendTaskSignal
				global _RmSendTaskSignal
				codetable _RmSendTaskforceSignal
				global _RmSendTaskforceSignal
				codetable _RmSetNetworkNative
				global _RmSetNetworkNative
				codetable _RmSetProcessorsNative
				global _RmSetProcessorsNative
				codetable _RmWaitforTask
				global _RmWaitforTask
				codetable _RmWaitforTaskforce
				global _RmWaitforTaskforce
				codetable _RmWaitforTasks
				global _RmWaitforTasks
				codetable _RmWritefd
				global _RmWritefd
				codetable _RmWritefdNetwork
				global _RmWritefdNetwork
				codetable _RmWritefdNetworkOnly
				global _RmWritefdNetworkOnly
				codetable _RmWritefdProcessor
				global _RmWritefdProcessor
				codetable _RmWritefdTask
				global _RmWritefdTask
				codetable _RmWritefdTaskforce
				global _RmWritefdTaskforce
				codetable _RmWritefdTaskforceOnly
				global _RmWritefdTaskforceOnly
				codetable _RmSetProcessorBooked
				global _RmSetProcessorBooked
				codetable _RmSetProcessorCancelled
				global _RmSetProcessorCancelled
				codetable _RmTx
				global _RmTx
				codetable _RmRx
				global _RmRx
				codetable _RmXch
				global _RmXch
				codetable _RmGetProcessorSession
				global _RmGetProcessorSession
				codetable _RmGetProcessorApplication
				global _RmGetProcessorApplication
				codetable _RmGetSession
				global _RmGetSession
				codetable _RmGetApplication
				global _RmGetApplication
			data _RmVersionNumber, 4
			global _RmVersionNumber 
			data _RmExceptionHandler, 4
			global _RmExceptionHandler 
				codetable _RmReportProcessor
				global _RmReportProcessor
				codetable _RmGetTaskState
				global _RmGetTaskState
				codetable _RmSetTaskState
				global _RmSetTaskState
				codetable _RmGetTaskforceState
				global _RmGetTaskforceState
				codetable _RmSetTaskforceState
				global _RmSetTaskforceState
				codetable _RmGetTaskforce
				global _RmGetTaskforce
				codetable _RmGetTask
				global _RmGetTask
				codetable _RmFindMatchingTask
				global _RmFindMatchingTask
				codetable _RmWaitforAnyTask
				global _RmWaitforAnyTask
				codetable _RmConvertTaskToTaskforce
				global _RmConvertTaskToTaskforce
				codetable _RmLookupProcessor
				global _RmLookupProcessor
			data _RmRootName, 4
			global _RmRootName 
			data _RmExceptionStack, 4
			global _RmExceptionStack 
				codetable _RmBuildProcessorName
				global _RmBuildProcessorName
				data _RmServerLock, 12
						patchinstr(PATCHc40MASK24ADD,
							shift(-2,labelref(._RmLib_Init)),
							br	0)
				b	R7		
				_CodeTableInit:
				patchinstr(PATCHC40MASK8ADD,
					shift(1, modnum),
					ldi	*+AR4(1), AR0)
	ldi	R11, AR5			
	laj	4
		nop				
		patchinstr(PATCHC40MASK16ADD,
			shift(-2, labelref(_FuncTableEnd)),
			addi	-2, R11)	
		ldi	R11, AR1
	ldi	AR5, R11			
				B	_Loop1Start		
				_Loop1:				
				ADDI	AR1, RS		
				STI	RS,	*AR0++(1)	
				_Loop1Start:			
				LDI *--AR1, RS	
				Bne	_Loop1	    		
				B	R11			
				_FuncTable:			
					int 0			
						int shift(-2, labelref(.RmBuildProcessorName))
						int shift(-2, labelref(.RmLookupProcessor))
						int shift(-2, labelref(.RmConvertTaskToTaskforce))
						int shift(-2, labelref(.RmWaitforAnyTask))
						int shift(-2, labelref(.RmFindMatchingTask))
						int shift(-2, labelref(.RmGetTask))
						int shift(-2, labelref(.RmGetTaskforce))
						int shift(-2, labelref(.RmSetTaskforceState))
						int shift(-2, labelref(.RmGetTaskforceState))
						int shift(-2, labelref(.RmSetTaskState))
						int shift(-2, labelref(.RmGetTaskState))
						int shift(-2, labelref(.RmReportProcessor))
						int shift(-2, labelref(.RmGetApplication))
						int shift(-2, labelref(.RmGetSession))
						int shift(-2, labelref(.RmGetProcessorApplication))
						int shift(-2, labelref(.RmGetProcessorSession))
						int shift(-2, labelref(.RmXch))
						int shift(-2, labelref(.RmRx))
						int shift(-2, labelref(.RmTx))
						int shift(-2, labelref(.RmSetProcessorCancelled))
						int shift(-2, labelref(.RmSetProcessorBooked))
						int shift(-2, labelref(.RmWritefdTaskforceOnly))
						int shift(-2, labelref(.RmWritefdTaskforce))
						int shift(-2, labelref(.RmWritefdTask))
						int shift(-2, labelref(.RmWritefdProcessor))
						int shift(-2, labelref(.RmWritefdNetworkOnly))
						int shift(-2, labelref(.RmWritefdNetwork))
						int shift(-2, labelref(.RmWritefd))
						int shift(-2, labelref(.RmWaitforTasks))
						int shift(-2, labelref(.RmWaitforTaskforce))
						int shift(-2, labelref(.RmWaitforTask))
						int shift(-2, labelref(.RmSetProcessorsNative))
						int shift(-2, labelref(.RmSetNetworkNative))
						int shift(-2, labelref(.RmSendTaskforceSignal))
						int shift(-2, labelref(.RmSendTaskSignal))
						int shift(-2, labelref(.RmSearchTasks))
						int shift(-2, labelref(.RmRevertProcessors))
						int shift(-2, labelref(.RmRevertNetwork))
						int shift(-2, labelref(.RmResetProcessors))
						int shift(-2, labelref(.RmResetNetwork))
						int shift(-2, labelref(.RmReconfigureProcessors))
						int shift(-2, labelref(.RmReconfigureNetwork))
						int shift(-2, labelref(.RmRebootProcessors))
						int shift(-2, labelref(.RmRebootNetwork))
						int shift(-2, labelref(.RmReadfdTaskforceOnly))
						int shift(-2, labelref(.RmReadfdTaskforce))
						int shift(-2, labelref(.RmReadfdTask))
						int shift(-2, labelref(.RmReadfdProcessor))
						int shift(-2, labelref(.RmReadfdNetworkOnly))
						int shift(-2, labelref(.RmReadfdNetwork))
						int shift(-2, labelref(.RmReadfd))
						int shift(-2, labelref(.RmObtainProcessors))
						int shift(-2, labelref(.RmMapErrorToString))
						int shift(-2, labelref(.RmLeaveTaskforce))
						int shift(-2, labelref(.RmLeaveTask))
						int shift(-2, labelref(.RmIsTaskforceRunning))
						int shift(-2, labelref(.RmIsTaskRunning))
						int shift(-2, labelref(.RmIsTaskNormal))
						int shift(-2, labelref(.RmIsProcessorFree))
						int shift(-2, labelref(.RmIsNetworkPossible))
						int shift(-2, labelref(.RmIsLinkPossible))
						int shift(-2, labelref(.RmGetTaskforceReturncode))
						int shift(-2, labelref(.RmGetTaskReturncode))
						int shift(-2, labelref(.RmGetTaskAttribute))
						int shift(-2, labelref(.RmGetProcessorControl))
						int shift(-2, labelref(.RmGetProcessorAttribute))
						int shift(-2, labelref(.RmGetTaskforceError))
						int shift(-2, labelref(.RmGetTaskError))
						int shift(-2, labelref(.RmGetProcessorError))
						int shift(-2, labelref(.RmGetNetworkError))
						int shift(-2, labelref(.RmExecuteTaskforce))
						int shift(-2, labelref(.RmExecuteTask))
						int shift(-2, labelref(.RmClearTaskforceError))
						int shift(-2, labelref(.RmClearTaskError))
						int shift(-2, labelref(.RmClearProcessorError))
						int shift(-2, labelref(.RmClearNetworkError))
						int shift(-2, labelref(.RmAreProcessorsPossible))
						int shift(-2, labelref(.RmIsMappedTask))
						int shift(-2, labelref(.RmCountMappedTasks))
						int shift(-2, labelref(.RmSearchMappedTasks))
						int shift(-2, labelref(.RmApplyMappedTasks))
						int shift(-2, labelref(.RmFollowTaskMapping))
						int shift(-2, labelref(.RmUnmapTask))
						int shift(-2, labelref(.RmMapTask))
						int shift(-2, labelref(.RmSetLinkMode))
						int shift(-2, labelref(.RmGetLinkMode))
						int shift(-2, labelref(.RmGetNetworkAndHardware))
						int shift(-2, labelref(.RmRemoveObjectAttribute))
						int shift(-2, labelref(.RmInsertProcessor))
						int shift(-2, labelref(.RmAddObjectAttribute))
						int shift(-2, labelref(.RmGetObjectAttribute))
						int shift(-2, labelref(.RmMapProcessorToObject))
						int shift(-2, labelref(.RmGetNetworkHierarchy))
						int shift(-2, labelref(.RmWhoIs))
						int shift(-2, labelref(.RmGetProcessorCapability))
						int shift(-2, labelref(.RmCountTasks))
						int shift(-2, labelref(.RmCountProcessors))
						int shift(-2, labelref(.RmFindLink))
						int shift(-2, labelref(.RmGetTaskUid))
						int shift(-2, labelref(.RmGetProcessorUid))
						int shift(-2, labelref(.RmSetProcessorState))
						int shift(-2, labelref(.RmSetNetworkPrivate))
						int shift(-2, labelref(.RmGetNetworkPrivate))
						int shift(-2, labelref(.RmSetTaskforcePrivate))
						int shift(-2, labelref(.RmGetTaskforcePrivate))
						int shift(-2, labelref(.RmSetTaskPrivate))
						int shift(-2, labelref(.RmGetTaskPrivate))
						int shift(-2, labelref(.RmSetProcessorPrivate))
						int shift(-2, labelref(.RmGetProcessorPrivate))
						int shift(-2, labelref(.RmFollowChannelToFile))
						int shift(-2, labelref(.RmConnectChannelToFile))
						int shift(-2, labelref(.RmCountTaskArguments))
						int shift(-2, labelref(.RmGetTaskArgument))
						int shift(-2, labelref(.RmAddTaskArgument))
						int shift(-2, labelref(.RmUnlockRead))
						int shift(-2, labelref(.RmSwitchLockRead))
						int shift(-2, labelref(.RmLockRead))
						int shift(-2, labelref(.RmUnlockWrite))
						int shift(-2, labelref(.RmLockWrite))
						int shift(-2, labelref(.RmCloseServer))
						int shift(-2, labelref(.RmOpenServer))
						int shift(-2, labelref(.RmFinishedJob))
						int shift(-2, labelref(.RmNewJob))
						int shift(-2, labelref(.RmWriteTask))
						int shift(-2, labelref(.RmReadTask))
						int shift(-2, labelref(.RmWriteTaskforce))
						int shift(-2, labelref(.RmReadTaskforce))
						int shift(-2, labelref(.RmWriteProcessor))
						int shift(-2, labelref(.RmReadProcessor))
						int shift(-2, labelref(.RmWriteNetwork))
						int shift(-2, labelref(.RmReadNetwork))
						int shift(-2, labelref(.RmWriteStream))
						int shift(-2, labelref(.RmReadStream))
						int shift(-2, labelref(.RmWrite))
						int shift(-2, labelref(.RmRead))
						int shift(-2, labelref(.RmExtendFreeQ))
						int shift(-2, labelref(.RmObtainUid))
						int shift(-2, labelref(.RmReleaseUid))
						int shift(-2, labelref(.RmNextFreeUid))
						int shift(-2, labelref(.RmFindUid))
						int shift(-2, labelref(.RmFindTableEntry))
						int shift(-2, labelref(.RmAddHardwareFacility))
						int shift(-2, labelref(.RmSearchHardwareFacilities))
						int shift(-2, labelref(.RmApplyHardwareFacilities))
						int shift(-2, labelref(.RmApplyTasks))
						int shift(-2, labelref(.RmSetTaskforcePrivate2))
						int shift(-2, labelref(.RmGetTaskforcePrivate2))
						int shift(-2, labelref(.RmSetTaskPrivate2))
						int shift(-2, labelref(.RmGetTaskPrivate2))
						int shift(-2, labelref(.RmSetNetworkPrivate2))
						int shift(-2, labelref(.RmGetNetworkPrivate2))
						int shift(-2, labelref(.RmReleaseNetwork))
						int shift(-2, labelref(.RmFindMatchingProcessor))
						int shift(-2, labelref(.RmObtainNetwork))
						int shift(-2, labelref(.RmReleaseProcessor))
						int shift(-2, labelref(.RmIsProcessorPermanent))
						int shift(-2, labelref(.RmIsProcessorTemporary))
						int shift(-2, labelref(.RmSetProcessorPermanent))
						int shift(-2, labelref(.RmSetProcessorTemporary))
						int shift(-2, labelref(.RmIsProcessorExclusive))
						int shift(-2, labelref(.RmIsProcessorShareable))
						int shift(-2, labelref(.RmSetProcessorExclusive))
						int shift(-2, labelref(.RmSetProcessorShareable))
						int shift(-2, labelref(.RmObtainProcessor))
						int shift(-2, labelref(.RmLastChangeDomain))
						int shift(-2, labelref(.RmGetDomain))
						int shift(-2, labelref(.RmLastChangeNetwork))
						int shift(-2, labelref(.RmGetNetwork))
						int shift(-2, labelref(.RmIsTask))
						int shift(-2, labelref(.RmIsTaskforce))
						int shift(-2, labelref(.RmRootTaskforce))
						int shift(-2, labelref(.RmGetChannelFlags))
						int shift(-2, labelref(.RmFollowChannel))
						int shift(-2, labelref(.RmCountChannels))
						int shift(-2, labelref(.RmSearchProcessors))
						int shift(-2, labelref(.RmBreakChannel))
						int shift(-2, labelref(.RmMakeChannel))
						int shift(-2, labelref(.RmSearchTaskforce))
						int shift(-2, labelref(.RmApplyTaskforce))
						int shift(-2, labelref(.RmParentTaskforce))
						int shift(-2, labelref(.RmSizeofTaskforce))
						int shift(-2, labelref(.RmIsTaskforceEmpty))
						int shift(-2, labelref(.RmPreviousTask))
						int shift(-2, labelref(.RmNextTask))
						int shift(-2, labelref(.RmLastTask))
						int shift(-2, labelref(.RmFirstTask))
						int shift(-2, labelref(.RmPostinsertTask))
						int shift(-2, labelref(.RmPreinsertTask))
						int shift(-2, labelref(.RmRemoveTask))
						int shift(-2, labelref(.RmAddheadTask))
						int shift(-2, labelref(.RmAddtailTask))
						int shift(-2, labelref(.RmSetTaskNormal))
						int shift(-2, labelref(.RmSetProcessorPrivate2))
						int shift(-2, labelref(.RmGetProcessorPrivate2))
						int shift(-2, labelref(.RmIsTaskNative))
						int shift(-2, labelref(.RmSetTaskNative))
						int shift(-2, labelref(.RmSetTaskforceId))
						int shift(-2, labelref(.RmGetTaskforceId))
						int shift(-2, labelref(.RmFreeTaskforce))
						int shift(-2, labelref(.RmNewTaskforce))
						int shift(-2, labelref(.RmIsProcessor))
						int shift(-2, labelref(.RmIsNetwork))
						int shift(-2, labelref(.RmRootNetwork))
						int shift(-2, labelref(.RmGetLinkFlags))
						int shift(-2, labelref(.RmFollowLink))
						int shift(-2, labelref(.RmCountLinks))
						int shift(-2, labelref(.RmApplyProcessors))
						int shift(-2, labelref(.RmBreakLink))
						int shift(-2, labelref(.RmMakeLink))
						int shift(-2, labelref(.RmMergeNetworks))
						int shift(-2, labelref(.RmSearchNetwork))
						int shift(-2, labelref(.RmApplyNetwork))
						int shift(-2, labelref(.RmParentNetwork))
						int shift(-2, labelref(.RmSizeofNetwork))
						int shift(-2, labelref(.RmIsNetworkEmpty))
						int shift(-2, labelref(.RmPreviousProcessor))
						int shift(-2, labelref(.RmNextProcessor))
						int shift(-2, labelref(.RmLastProcessor))
						int shift(-2, labelref(.RmFirstProcessor))
						int shift(-2, labelref(.RmPostinsertProcessor))
						int shift(-2, labelref(.RmPreinsertProcessor))
						int shift(-2, labelref(.RmRemoveProcessor))
						int shift(-2, labelref(.RmAddheadProcessor))
						int shift(-2, labelref(.RmAddtailProcessor))
						int shift(-2, labelref(.RmSetNetworkId))
						int shift(-2, labelref(.RmGetNetworkId))
						int shift(-2, labelref(.RmFreeNetwork))
						int shift(-2, labelref(.RmNewNetwork))
						int shift(-2, labelref(.RmListTaskAttributes))
						int shift(-2, labelref(.RmCountTaskAttributes))
						int shift(-2, labelref(.RmTestTaskAttribute))
						int shift(-2, labelref(.RmRemoveTaskAttribute))
						int shift(-2, labelref(.RmAddTaskAttribute))
						int shift(-2, labelref(.RmGetProgramType))
						int shift(-2, labelref(.RmSetTaskType))
						int shift(-2, labelref(.RmGetTaskType))
						int shift(-2, labelref(.RmSetTaskCode))
						int shift(-2, labelref(.RmGetTaskCode))
						int shift(-2, labelref(.RmSetTaskId))
						int shift(-2, labelref(.RmGetTaskId))
						int shift(-2, labelref(.RmSetTaskMemory))
						int shift(-2, labelref(.RmGetTaskMemory))
						int shift(-2, labelref(.RmFreeTask))
						int shift(-2, labelref(.RmNewTask))
						int shift(-2, labelref(.RmListProcessorAttributes))
						int shift(-2, labelref(.RmCountProcessorAttributes))
						int shift(-2, labelref(.RmTestProcessorAttribute))
						int shift(-2, labelref(.RmRemoveProcessorAttribute))
						int shift(-2, labelref(.RmAddProcessorAttribute))
						int shift(-2, labelref(.RmSetProcessorNucleus))
						int shift(-2, labelref(.RmGetProcessorNucleus))
						int shift(-2, labelref(.RmWhoAmI))
						int shift(-2, labelref(.RmGetProcessorOwner))
						int shift(-2, labelref(.RmSetProcessorType))
						int shift(-2, labelref(.RmGetProcessorType))
						int shift(-2, labelref(.RmGetProcessorState))
						int shift(-2, labelref(.RmSetProcessorPurpose))
						int shift(-2, labelref(.RmGetProcessorPurpose))
						int shift(-2, labelref(.RmSetProcessorId))
						int shift(-2, labelref(.RmGetProcessorId))
						int shift(-2, labelref(.RmSetProcessorMemory))
						int shift(-2, labelref(.RmGetProcessorMemory))
						int shift(-2, labelref(.RmFreeProcessor))
						int shift(-2, labelref(.RmNewProcessor))
				_FuncTableEnd:			
		ref	Kernel.library
		ref	SysLib.library
		ref	Util.library
		ref	ServLib.library
		ref	Posix.library
