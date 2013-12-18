_report ['include exec.m]

include /d/helios/include/ampp/structs.m
include /d/helios/include/ampp/queue.m


_def 'exec.m_flag 1

	struct	ExecPtrs [
	word	SavePSR
	word	SaveREG
	word	TRAPEXIT
	word	SysStack
	word	Execroot
	word	TRAPRTNP
	]

_def EXECPTRS [_sub 0 ExecPtrs.sizeof]

	struct	TrapData [
	vec	[_mul 32 4] IREGS
	vec	[_mul 32 4] FPREGS
	word	SystemStack
	word	PSR
	word	EPSR
	word	DB
	word	DIRBASE
	word	FIR
	word	FSR
	word	KR
	word	KI
	word	T
	word	MERGE
	]

_def FLUSHSPACE [0xf0000000]

	struct	SaveState [
	word	RunqNext
	word	Pri
	word	EndTime
	word	State
	word	SysStack
	word	Display
	struct  TrapData TrapData
	]

	struct	ProcessQ [
	word	head
	word	tail
	]

_def MAXPRI 2

	struct	EXECROOT [
	vec	[_mul MAXPRI ProcessQ.sizeof] Queues
	vec	[_mul MAXPRI ProcessQ.sizeof] IQueues
	word	CurrentP
	word	TimerQ
	word	WakeUp
	word	CurrentT
	word	Lo_Pri_Counter
	word	CurrentPri
	word	PriRange
	word	IntFlag
--	word	FreeStates
--	struct	Pool FreeStatePool
--	word	FreeSystemStacks
--	struct	Pool FreeSystemStackPool
--	word	TxData
--	word	TxSize
--	word	TxSaveState
--	word	RxData
--	word	RxSize
--	word	RxSaveState
	]

_def	init_stack_pointer [0xf0009000]
_def    loadbase	   [0xf000a000]
_def    rte_data_area	   [0xf0009c00] 	
_def    next_page_location [0xf0009f00]
