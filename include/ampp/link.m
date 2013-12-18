--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- link.m								--
--                                                                      --
--	Link control structures						--
--                                                                      --
--      Author: NHG 28-July-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include link.m]
_def 'link.m_flag 1

include structs.m
include sem.m
include queue.m
include message.m

-- link control structure

struct Link [
	byte		Flags		-- flag byte
	byte		Mode		-- link mode/type
	byte		State		-- link state
	byte		Id		-- link id used in ports etc.
	word		TxChan		-- address of transmission channel
	word		RxChan		-- address of reception channel
	word		TxUser		-- Id for tx channel
	word		RxUser		-- Id for rx channel
	word		MsgsIn		-- number of input messages
	word		MsgsOut		-- number of output messages
	word		TxQueue		-- queue of waiting transmitters
	struct Sem	TxMutex		-- Mutual exclusion for transmitters
	word		RxSync		-- sync channel for direct link rx
	word		LocalIOCPort	-- port to be used by our LinkIOC
	word		RemoteIOCPort	-- port to remote IOC
	word		Incarnation	-- remote processors incarnation number
	word		MsgsLost	-- messages lost/destroyed
	word		DBInfo		-- double buffer process info
	word		Timeout
	_if _not _defp 'helios.TRAN [
		word	TxThread	-- Thread of LinkTx 
		word	RxThread	-- Thread of LinkRx
		_if _defp 'helios.C40 [
			word	DMAEng		-- control reg WPTR
		]
		_if _or _defp 'helios.C40 _defp 'helios.arm [
			struct Sem HalfDuplex	-- signaled when link ok to wr.
		]
	]
]

-- Flag bits
_test _defp 'helios.TRAN [
	_def 	Link.Flags.bootable	[#80]	-- set if remote processor is
						-- bootable (NO LONGER USED)
	_def	Link.Flags.parent	[#40]	-- set if this link booted us
	_def	Link.Flags.ioproc	[#20]	-- indicates an io processor
	_def	Link.Flags.debug	[#10]	-- debugging link for IOdebug
	_def	Link.Flags.report	[#8]	-- report state changes
	_def	Link.Flags.stopped	[#4]	-- link traffic has been stopped
][
	_def	Link.Flags.parent	[0x40]	-- set if this link booted us
	_def	Link.Flags.ioproc	[0x20]	-- indicates an io processor
	_def	Link.Flags.debug	[0x10]	-- debugging link for IOdebug
	_def	Link.Flags.report	[0x08]	-- report state changes
	_def	Link.Flags.stopped	[0x04]	-- link traffic has been stopped
	_def	Link.Flags.HalfDuplex	[0x80]	-- use half duplex protocol
]

-- Link Modes

_def	Link.Mode.Null		0	-- the link is not connected to anything
_def	Link.Mode.Dumb		1	-- connected to a dumb device
_def	Link.Mode.Intelligent	2	-- connected to an intelligent device
_def	Link_Mode_Special	3	-- link is a non-std comms link

-- Link states

_def	Link.State.Null		0	-- link is not connected
_def	Link.State.Booting	1	-- link is booting remote processor
_def	Link.State.Dumb		2	-- serving a dumb device
_def	Link.State.Running	3	-- serving as network driver
_def	Link.State.Timedout	4	-- timeout on link has expired
_def	Link.State.Crashed	5	-- the link has crashed
_def	Link.State.Dead		6	-- there is nothing on this link

-- Timeout values

_def	Timeout.Idle		[10000000]	-- 10 second timeout
_def	Timeout.Short		[1000000]	-- 1 second timeout
_def	Timeout.Tx		[11000000]	-- transmission timeout

_if _defp 'helios.TRAN [
	_def	LGStackSize		[1024]	-- size of link guardian stack

	_def	Proto.Null	[3]		-- initial value of type word
	_def	Proto.Sync	[#F0F0F0F0]	-- link sync value
	_def	Proto.Write	[0]		-- debug write command
	_def	Proto.Read	[1]		-- debug read command
	_def	Proto.Info	[#F0]		-- first byte of sync value
	_def	Proto.Msg	[2]		-- message header
	_def	Proto.Dead	[#61]		-- result of probing a dead link
	_def	Proto.Alive	[#9E]		-- result of probing a live link
	_def	Proto.Term	[4]		-- machine terminate message
	_def	Probe.Value	[#61616161]	-- value for probes
]

struct InfoMsg [
	byte		TxInc		-- transmitters incarnation number
	byte		RxInc		-- receivers incarnation number
	byte		ReplyReq	-- non zero if reply required
	byte		Spare
	word		IOCPort		-- port desc for his Link IOC
]

-- Pending message buffer structure
-- This is the size of a pool buffer, but this
-- structure is also mapped onto buffers of other sizes.

struct MsgBuf [
	struct Node	Node		-- queuing node
	word		Type		-- 0=pool 1=special
	struct MCB	MCB		-- control structure
	vec    1056	Msg 		-- buffer
]

_def 	MsgBuf.Overhead [_add Node.sizeof _add MCB.sizeof 4]



-- End of link.m
