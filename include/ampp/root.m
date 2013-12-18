--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- root.m                                                               --
--                                                                      --
--      Root data structure                                             --
--                                                                      --
--      Author: NHG 16-July-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include root.m]
_def 'root.m_flag 1

include memory.m
include queue.m
include sem.m
include ports.m

-- Processor specific includes to get value of 'InterruptVectors'
_if _defp 'helios.C40 [
	include c40.m
]
_if _defp 'helios.arm [
	include arm.m
]

_if _defp 'helios.TRAN [
	--_def	'MemStart   [18]	-- T414
	_def	'MemStart   [28]	-- T800

	_defq 'GetRoot [
		mint ldnl #400
		mint ldnlp #400
		add
	]
]

_if _defp 'helios.C40 [
	_defq 'GetRoot['Areg] [
		ldep	tvtp, Areg		--*Warning* ExecRoot.KernelRoot
		-- C40WordAddress *+Areg(3), Areg
		lsh	-2, *+Areg(3), Areg	-- mustn't change position (3)
		addi	R_BASE, Areg
	]
]


struct Root [
        word    	Flags		-- flag bits 
        word    	PortTable       -- pointer to port table
        word    	PTSize          -- size of port table in words
        word    	PTFreeq         -- port table free queue
	word		Links		-- pointer to link table
	struct Pool	SysPool		-- allocated system memory
	word		FreePool	-- free memory list
	word		Incarnation	-- our incarnation number
	struct List	BufferPool	-- pending delivery buffer pool
	word		BuffPoolSize	-- no. of free slots in pool
	word		LoadAverage	-- low pri load average
	word		Latency		-- hi pri scheduling latency
	word		TraceVec	-- pointer to trace vector
	_if _defp 'helios.TRAN [
		struct List	EventList	-- list of event routines
	]
	_if _defp 'helios.C40 [		-- list of interrupt handlers
		-- vec 	_mul InterruptVectors List.sizeof EventList
		-- vec 	_mul 6                12          EventList
		vec 	72 EventList
	]
	_if _defp 'helios.arm [
		vec [_mul _eval InterruptVectors _eval List.sizeof] EventList
	]
	_if _defp '__ABC [
		-- vec 	_mul UserVectors List.sizeof EventList
		vec	264	UserEventList
	]
	word		EventCount	-- number of events seen
	word		Time		-- current system time
	struct Pool	FastPool	-- fast RAM pool
	word		MaxLatency	-- maximum latency seen
	struct Sem	UIDebugLock	-- lock for all IOdebugs
	word		MachineType	-- processor type code
	word		BufferCount	-- number of kernel buffers used
	word		MaxBuffers	-- max number of buffers allowed
	word		Timer		-- system timer value (MHz)
	word		Errors		-- number of errors seen
	word		LocalMsgs	-- local message traffic	
	word		BufferedMsgs	-- messages buffered by kernel	
	word		Flags		-- system flags			
	word		LoaderPool	-- pointer to loader pool	
	word		Configuration	-- pointer to config structure	
	word		ErrorCodes	-- array of kernel error codes	
	struct Port	IODebugPort	-- intercept on IOdebug messages
	word		GCControl	-- control of port garbage collector 
	_if _not _defp 'helios.TRAN [
		vec 80		IODBuffer	-- static buffer for IOdebug()s	
		word		IODBufPos	-- Position in buffer		
	]
	_if _defp '__SMT [
		word		cpi		-- SMT code pointer index	
		word		cpislots	-- number of slots in cpi	
		struct Sem	cpi_op		-- atomic ops on cpi		
	]
	_if _defp '__ABC [
		word		MISysMem	-- Memory Indirect(ion) table 	
		word		RRDScavenged	-- Number of blocks found	
		word		RRDPool;	-- Robust Ram Disk pool		
	]
	_if _and _defp 'helios.C40 _defp 'ALLOCDMA [
		word		DMAReqQhead	-- DMA engine alloc req Q
		word		DMAReqQtail	-- DMA engine alloc req Q
		word		DMAFreeQhead	-- first DMA engine in DMAFreeQ
		vec 48		DMAFreeQ	-- list of free DMA engines
	]

	-- Spare slots to be used when adding new fields to the root structure.
	-- If the root structure grows beyond this size, then all the (shared
	-- memory) bootstraps that download the config structure. will have to
	-- be re-assembled, and distributed with the new nucleus.

	_test _true [
		word		SpecialPools
	][
		word		spare1
	]
	_test _or _defp 'helios.arm _defp '__VY86PID [
		-- VLSI PID INTC interrupt mask is write only so its contents
		-- should always be read from here and written here and to INTC.
		word	IRQM_softcopy
	][
		word	spare2
	]
	word		spare3
	word		spare4
	word		spare5
	word		spare6
	word		spare7
	word		spare8
]

_test _defp 'helios.TRAN [
	_def Root_Flags_rootnode #00000001	-- set if this is rootnode
	_def Root_Flags_special	 #00000002	-- set if this is special nuc.
	_def Root_Flags_ROM	 #00000004	-- set if this is ROMm'ed nuc.
	_def Root_Flags_xoffed	 #00000100	-- set if links xoffed
][
	_def Root_Flags_rootnode 0x00000001	-- set if this is rootnode
	_def Root_Flags_special	 0x00000002	-- set if this is special nuc.
	_def Root_Flags_ROM	 0x00000004	-- set if this is ROMm'ed nuc.
	_def Root_Flags_xoffed	 0x00000100	-- set if links xoffed
]

_if _defp 'helios.C40 [
	_def Root_Flags_CacheOff 0x00000200	-- Dont enable cache
]


--  End of root.m
