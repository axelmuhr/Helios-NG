--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- config.m								--
--                                                                      --
--	Configuration Structure.					--
--                                                                      --
--	This is loaded by the booting processor into a well known	--
--	memory location where the kernel can get at it.			--
--	In the generic executive it is located directly after the	--
--	Root structure. Normally this data will be derived from a	--
--	configuration file on the booting processor.			--
--                                                                      --
--      Author: NHG 30-July-87						--
--                                                                      --
--	RCS Id: $id$							--
--------------------------------------------------------------------------

_report ['include config.m]
_def 'config.m_flag 1

include structs.m

-- the LinkConf structures must be in the same order as the link
-- channels in low memory.
struct LinkConf [
	byte	Flags			-- initial flags
	byte	Mode			-- link mode
	byte	State			-- initial state
	byte	Id			-- link id
]

struct Config [
	word	PortTabSize		-- Number of slots in port table
	word	Incarnation		-- what booter believes our incarnation is
	word	LoadBase		-- address at which system was loaded
	word	ImageSize		-- size of system image
	word	Date			-- current system date
	word	FirstProg		-- offset of initial program
	word	MemSize			-- if > 0, first byte of unused mem
	word	Flags			-- Initial root struct flags
	_test _defp 'helios.arm [
		word	Speed		-- RS232 link comms baud rate
	][
		word	Spare		-- reserved (spare) slot
	]
	word	MyName			-- full path name
	word	ParentName		-- ditto
	word	NLinks			-- number of links

	-- NLinks LinkConf structs (4 for tranny, 6 for C40).
	_test _defp 'helios.C40 [
		vec [_mul 6 _eval LinkConf.sizeof] LinkConf0
	][
		vec [_mul 4 _eval LinkConf.sizeof] LinkConf0
	]
]


_if _defp 'helios.C40 [
	-- define C40 specific Hardware Config flags in initial word
	-- sent to the C40 bootstrap
	_def	HW_NucleusLocalS0	0    -- load nuc. into local bus
					     -- strobe 0 (the default)
	_def	HW_NucleusLocalS1	1    -- load nuc. on local bus strobe 1
	_def	HW_NucleusGlobalS0	2    -- load nuc. on global bus strobe 0
	_def	HW_NucleusGlobalS1	4    -- load nuc. on global bus strobe 1
	_def	HW_PseudoIDROM		8    -- download and use pseudo IDROM
	_def	HW_ReplaceIDROM		16   -- download and replace IDROM
	_def	HW_CacheOff		32   -- dont enable cache
]


-- Image vector
struct IVec [
	word		Size		-- size of load image in bytes
	word		Kernel		-- RPTR to kernel module
	word		Syslib		-- RPTR to system library module
	word		Servlib		-- RPTR to server library module
	word		Util		-- RPTR to Util library
	_if _not _defp 'helios.arm [
		word		Boot		-- RPTR to bootstrap
	]
	word		ProcMan		-- RPTR to processor manager
	word		Tasks		-- first task
]

	
-- End of config.m
