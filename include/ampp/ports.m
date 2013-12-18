--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- ports.m                                                              --
--                                                                      --
--      Port defintion macros                                           --
--                                                                      --
--      Author: NHG 20-May-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include ports.m]
_def 'ports.m_flag 1


-- Port descriptor

struct Port [
        byte    IndexLo                 -- 16 bit index into table
        byte    IndexHi
        byte    Cycle                   -- table entry cycle
        byte    Flags                   -- flag byte
]

_test _defp 'helios.TRAN [
	_def Port.Index.mask    [#FFFF]         -- mask for index
	_def Port.Cycle.shift   [16]            -- shift for cycle field
	_def Port.Flags.mb1     [#80]		-- so zero is never a valid port
	_def Port.Flags.Tx	[#40]		-- 0=rx port, 1=tx port
	_def Port.Flags.Remote	[#20]		-- set if port is surrogate
][
	_def Port.Index.mask    [0xFFFF]         -- mask for index

	_def Port.Cycle.shft    [16]            -- shift for cycle field
	_def Port.Flags.mb1     [0x80]		-- so zero is never a valid port
	_def Port.Flags.Tx	[0x40]		-- 0=rx port, 1=tx port
	_def Port.Flags.Remote	[0x20]		-- set if port is surrogate
]

-- Port table entry

struct PTE [
        byte    Type                    -- entry type
        byte    Cycle                   -- entry  cycle
        byte    Flags                   -- flag byte
        byte    Age                     -- GCTicks since last use
        word    Owner			-- owning task or surrogate port
	word	TxId			-- pointer to transmitter process
	word	RxId			-- pointer to receiver process
]

-- Table entry types

_def T_Free             0               -- unused slot
_def T_Local            1               -- a local port
_def T_Surrogate        2               -- a surrogate port
_def T_Trail            3               -- an intermediate route entry

-- Elements in the flags field

_test _defp 'helios.TRAN [
	_def PTE.Flags.TxState  [#C0]           -- transmit state
	_def PTE.Flags.TxIdle   [#00]           -- no activity on transmitter
	_def PTE.Flags.TxHdr    [#40]           -- waiting for header
	_def PTE.Flags.TxTfr    [#C0]           -- in transfer

	_def PTE.Flags.RxState  [#30]           -- transmit state
	_def PTE.Flags.RxIdle   [#00]           -- no activity on receiver
	_def PTE.Flags.RxHdr    [#10]           -- waiting for header
	_def PTE.Flags.RxTfr    [#30]           -- in transfer

	_def PTE.Flags.State	[#F0]		-- mask for state bits
	_def PTE.Flags.Link	[#0F]		-- link id in surrogate entry
][
	_def PTE.Flags.TxState  [0xC0]           -- transmit state
	_def PTE.Flags.TxIdle   [0x00]           -- no activity on transmitter
	_def PTE.Flags.TxHdr    [0x40]           -- waiting for header
	_def PTE.Flags.TxTfr    [0xC0]           -- in transfer

	_def PTE.Flags.RxState  [0x30]           -- transmit state
	_def PTE.Flags.RxIdle   [0x00]           -- no activity on receiver
	_def PTE.Flags.RxHdr    [0x10]           -- waiting for header
	_def PTE.Flags.RxTfr    [0x30]           -- in transfer

	_def PTE.Flags.State	[0xF0]		-- mask for state bits
	_def PTE.Flags.Link	[0x0F]		-- link id in surrogate entry
]

_def PTBlock.Size	[1024]		-- size of port table block

-- Derive a port table entry pointer from a port
-- assumes a local variable "root" point to the root structure

_if _defp 'helios.TRAN [
	_defq 'GetPTE['getpte_port]
	[
		ptr getpte_port Port.IndexHi		-- base table index
		root Root.PortTable wsub ldnl 0		-- subscript & pick up table ptr
		ptr getpte_port Port.IndexLo		-- port table index
		ldc PTE.sizeof prod rev wsub		-- subscript
	]
]


-- End of ports.m
