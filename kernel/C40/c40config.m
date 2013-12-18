-- File:	c40idrom.m
-- Subsystem:	`C40 Helios executive
-- Author:	P.A.Beskeen
-- Date:	March '94
--
-- Description:
--
-- ROMed copy of Config structure.
--
-- Config is a Helios specific structure used to define information
-- required to initialise the Helios nucleus for this particular
-- board's environment. Its structure is defined in the include file
-- '/hsrc/include/ampp/config.m' in the Helios source include directory.
--
-- The size of the Config structure passed should be placed within its
-- own ImageSize element. The size is not fixed as the number of links
-- defined may be non-standard (and therefore more LinkConf structures
-- than normal used), or the names of the processor and I/O Server
-- may not be the same as the defaults (and therefore added to the end
-- of the Config structure). This use of ImageSize is only valid
-- during bootstrap and Helios initialisation, the kernel will alter
-- it at a later point.
--
-- This information was originally placed in c40rombt.a, but for easier
-- modification by users has been removed, and replaced by an include
-- statement
--

_report ['include c40config.m]

	byte	"Bootstrap Config Structure"

	align

BoardConfig:
		-- Size of the initial port table (and further increments)
	word	1024		-- PortTabSize

		-- Number of times booted this session - immaterial
	word	1		-- Incarnation

		-- Obsolete - Helios kernel now sets this value.
	word	0		-- LoadBase

		-- Set size of Config. If you increase the number of links
		-- from 6 (for example to provide a pseudo link implemented
		-- with shared memory), or require a processor name other
		-- than the default /00, then this size will change. size
		-- is defined in terms of bytes.
	word	(BoardConfigEnd - BoardConfig) * 4	-- ImageSize in bytes

		-- Current time to initialise Helios's internal clock and
		-- date from. This is a Unix style date stamp defined as
		-- seconds since 1970 GMT. You may wish to initialise this
		-- value from an RTC if your system supports one.
	word	0		-- Date			-- current system date

		-- The first program in the system to be executed. The number
		-- refers to the order of modules in the nucleus. Always
		-- the ProcMan!
	word	9		-- FirstProg

		-- Size of memory (Default specified by kernel if 0). This is
		-- an anacronism, the memory sizes should be always be defined
		-- in the IDROM structure. This should always be zero.
	word	0		-- MemSize

		-- Set Config flags:
		--	Root_Flags_rootnode:	This is the root processor in a
		--				network.
		--	Root_Flags_ROM:		The nucleus is ROMmed in this
		--				node.
		--
		-- If either of these flags is set, then the 'init' program
		-- will be run as the final phase of the nucleus's
		-- initialisation.
	word	Root_Flags_rootnode | Root_Flags_ROM	-- Flags

		-- reserved (spare) slot
	word	0		-- Spare

		-- Setup the processors name and possibly non-existent I/O
		-- Server. These two RPTRs (self relative pointers) can be
		-- -1 to signify /00 and /IO as acceptable defaults. If other
		-- names are required, then these should be appended to the
		-- end of the structure, and the two RPTRs changed to point at
		-- these names. If the defaults are not used then the size
		-- should be updated accordingly (in Config.ImageSize).
		-- Config.ParentName (/IO) is ignored if Link.Flags.Parent
		-- is not set in any LinkConf (see below).
	word	-1		-- MyName
	word	-1		-- ParentName

		-- Number of communication ports attached to the system. If
		-- this changes ImageSize and the following link configuration
		-- initialisation should be updated accordingly.
	word	6		-- NLinks

		-------------------------------------------------------------
		-- Setup the link configuration information. The LinkConf
		-- and Config structures and flags are defined in the ampp
		-- header file '/hsrc/include/ampp/link.m'.
		--
		-- For each link set:
		--
		--	The Flags to define various properties of the link.
		--
		--	The following flags should usually only be set if an
		--	I/O Server is expected to be found at the other end of
		--	the link - rare for embedded ROM booted systems.
		--
		--	Link.Flags.Parent	If this nucleus was booted down
		--				a link, then this flag indicates
		--				which link (NEVER set in ROM
		--				booted systems).
		--	Link.Flags.ioproc	Indicates an I/O Server is
		--				present at the other end of the
		--				the link.
		--	Link.Flags.debug	Indicates debugging link down
		--				which IOdebug()s can be sent.
		--	Link.Flags.HalfDuplex	If link is connected to a 
		--				hard/software combination that
		--				cannot cope with full-duplex
		--				comms port operation, use
		--				Helios's half-duplex protocol
		--				(most PC's running MSDOG).
		--
		--	The Mode defining a links use:
		--
		--	Link.Mode.Null		Link is not connected.
		--	Link.Mode.Dumb		Link is connected, but should
		--				not be used by Helios. Users
		--				should call the AllocLink() fn
		--				to gain access to this link.
		--	Link.Mode.Intelligent	Connected to other processors
		--				and will be used for Helios
		--				message passing.
		--	Link.Mode.Special	Link is non-standard comms port
		--				interface (probably a pseudo
		--				link implemented in shared mem).
		--				This requires a custom nucleus
		--				that understands its i/f type
		--				incorporated into Link.State.
		--
		--	The State defining the current status of a link.
		--
		--	Link.State.Null		The Link is not connected.
		--	Link.State.Dead		Link is connected, but not
		--				currently in use by Helios.
		--	Link.State.Running	Live network link - a processor
		--				on the otherside of the link is
		--				is expecting Helios protocols
		--				down this link.
		--
		--	The ID of the link (0..NLinks - 1)
		--

		-- The SetLinkConf macro simplifies setting a links 
		-- configuration info.

		_def	'SetLinkConf['id 'flags 'mode 'state] [

			_def 'TMPCONF [
				( (flags) << LinkConf.Flags.shift |
				mode     << LinkConf.Mode.shift |
				state    << LinkConf.State.shift |
				id       << LinkConf.Id.shift )
			]
			word	TMPCONF

			_undef 'TMPCONF
		]

		-- Examples:
		--
		-- To setup link 0 as completely unconnected:
		--
		--	SetLinkConf 0 0 Link.Mode.Null Link.State.Null
		--
		-- To setup link 1 as connected to an external device, or
		-- another processor, but not to be used for Helios message
		-- passing:
		--
		--	SetLinkConf 1 0 Link.Mode.Dumb Link.State.Dead
		--
		-- To setup link 2 as connected to another processor running
		-- Helios, though which Helios messages may be passed:
		--
		--	SetLinkConf 2 0 Link.Mode.Intelligent Link.State.Dead
		-- 
		-- To setup link 3 as connected to a waiting I/O Server:
		--
		--	SetLinkConf 3 [Link.Flags.ioproc | Link.Flags.debug]
		--		Link.Mode.Intelligent Link.State.Dead
		--
		-- Set each links configuration info. The link Id's must be
		-- set in assending order.
		--	    Id	Flags		Mode		State

		SetLinkConf 0	0		Link.Mode.Null	Link.State.Null

		SetLinkConf 1	0		Link.Mode.Null	Link.State.Null

		SetLinkConf 2	0		Link.Mode.Null	Link.State.Null

		SetLinkConf 3	[(Link.Flags.ioproc|Link.Flags.debug)] Link.Mode.Intelligent Link.State.Dead

		SetLinkConf 4	0		Link.Mode.Null	Link.State.Null

		SetLinkConf 5	0		Link.Mode.Null	Link.State.Null

		-- Extra pseudo link definitions go here.

MyName:
		-- If processor name need to be changed from "/00", then
		-- place text here.

		align
BoardConfigEnd:
