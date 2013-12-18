-- File:	c40mmap.m
-- Subsystem:	'C40 Helios AMPP macros
-- Author:	P.A.Beskeen
-- Date:	Dec '91
--
-- Description: `C40 memory map and on-chip periperal addresses
--
--
-- RcsId: $Id: c40mmap.m,v 1.2 1992/06/10 15:34:14 paul Exp $
--
-- (C) Copyright 1991 Perihelion Software Ltd.


_report ['include c40mmap.m]
_def 'c40mmap.m_flag 1


-- Place base address of on-chip RAM into register.
_def 'OnChipRAM['AddrReg] [
	ldhi	0x2f, AddrReg
	or	0xf800, AddrReg	
]

-- Place base address of on-chip RAM bank 1 into register.
_def 'OnChipRAM1['AddrReg] [
	ldhi	0x2f, AddrReg
	or	0xfbc0, AddrReg	
]

-- Get first accessable address of local bus after on-chip RAM
_def 'LocalBus['AddrReg] [
	ldhi	0x30, AddrReg
]

-- Get address of global bus into reg
_def 'GlobalBus['AddrReg] [
	ldhi	0x8000, AddrReg
]


-- on-chip peripheral access macros
--
-- To get address of particular periperal register use:
-- 	ldaperimap periname reg
--
-- To get contents of particular periperal register use:
-- 	ldperimap Areg
--	...
--	LDI *+Areg(periname), reg

_def 'HiPeriAddr	[0x0010]

-- get base address of peripheral map
_def 'ldperimap['reg] [
	ldhi	HiPeriAddr, reg
]

-- Load a register with a peripheral address
_defq 'ldaperi['periname 'reg] [
	ldhi	HiPeriAddr, reg
	or	periname, reg
]	


-- on-chip peripheral addresses

-- Global memory control register
_def 'gmem_control	[0x0000]

-- Local memory control register
_def 'lmem_control	[0x0004]


-- Analysis module registers
_def 'ana_data		[0x0010]
_def 'ana_control	[0x0011]
_def 'ana_status	[0x0012]
_def 'ana_task		[0x0013]


-- Timer registers

-- relative offsets
_def 'timer_control	[0x0]
_def 'timer_count	[0x4]
_def 'timer_period	[0x8]

-- addresses relative to periperal base
_def 'timer0_control	[0x0020]
_def 'timer0_count	[0x0024]
_def 'timer0_period	[0x0028]

_def 'timer1_control	[0x0030]
_def 'timer1_count	[0x0034]
_def 'timer1_period	[0x0038]

-- timer control register bits
_def 'tcr_func		1
_def 'tcr_io		[(1 << 1)]
_def 'tcr_datout	[(1 << 2)]
_def 'tcr_datin		[(1 << 3)]
_def 'tcr_go		[(1 << 6)]
_def 'tcr_hld		[(1 << 7)]
_def 'tcr_cp		[(1 << 8)]
_def 'tcr_clksrc	[(1 << 9)]
_def 'tcr_inv		[(1 << 10)]
_def 'tcr_tstat		[(1 << 11)]

-- Undocumented Functions
-- Set these to make timer continue even when c40 is in debug mode
_def 'tcr_dbgcont1	[(1 << 30)]
_def 'tcr_dbgcont2	[(1 << 31)]


-- Communication port registers

-- relative offsets
_def 'port_control	[0x0]
_def 'port_input	[0x1]
_def 'port_output	[0x2]

_def 'port0_control	[0x0040]
_def 'port0_input	[0x0041]
_def 'port0_output	[0x0042]

_def 'port1_control	[0x0050]
_def 'port1_input	[0x0051]
_def 'port1_output	[0x0052]

_def 'port2_control	[0x0060]
_def 'port2_input	[0x0061]
_def 'port2_output	[0x0062]

_def 'port3_control	[0x0070]
_def 'port3_input	[0x0071]
_def 'port3_output	[0x0072]

_def 'port4_control	[0x0080]
_def 'port4_input	[0x0081]
_def 'port4_output	[0x0082]

_def 'port5_control	[0x0090]
_def 'port5_input	[0x0091]
_def 'port5_output	[0x0092]

-- communication port control register
_def cpcr_port_dir	[(1 << 2)]
_def cpcr_ich		[(1 << 3)]	-- input channel halt
_def cpcr_och		[(1 << 4)]	-- output channel halt

_def cpcr_output_levelM	[(~0b0000111100000)]	-- masks for input and output
_def cpcr_input_levelM	[(~0b1111000000000)]	-- fifo levels
_def cpcr_output_levelB	[5]			-- start bit
_def cpcr_input_levelB	[9]			-- start bit


-- DMA channel registers

-- relative offsets
_def dma_control	[0]
_def dma_srcaddr	[1]
_def dma_srcindex	[2]
_def dma_count		[3]
_def dma_dstaddr	[4]
_def dma_dstindex	[5]
_def dma_link		[6]
_def dma_auxcount	[7]
_def dma_auxlink	[8]


-- offsets from 0x100000
_def dma0_control	[0x00a0]
_def dma0_srcaddr	[0x00a1]
_def dma0_srcindex	[0x00a2]
_def dma0_count		[0x00a3]
_def dma0_dstaddr	[0x00a4]
_def dma0_dstindex	[0x00a5]
_def dma0_link		[0x00a6]
_def dma0_auxcount	[0x00a7]
_def dma0_auxlink	[0x00a8]

_def dma1_control	[0x00b0]
_def dma1_srcaddr	[0x00b1]
_def dma1_srcindex	[0x00b2]
_def dma1_count		[0x00b3]
_def dma1_dstaddr	[0x00b4]
_def dma1_dstindex	[0x00b5]
_def dma1_link		[0x00b6]
_def dma1_auxcount	[0x00b7]
_def dma1_auxlink	[0x00b8]

_def dma2_control	[0x00c0]
_def dma2_srcaddr	[0x00c1]
_def dma2_srcindex	[0x00c2]
_def dma2_count		[0x00c3]
_def dma2_dstaddr	[0x00c4]
_def dma2_dstindex	[0x00c5]
_def dma2_link		[0x00c6]
_def dma2_auxcount	[0x00c7]
_def dma2_auxlink	[0x00c8]

_def dma3_control	[0x00d0]
_def dma3_srcaddr	[0x00d1]
_def dma3_srcindex	[0x00d2]
_def dma3_count		[0x00d3]
_def dma3_dstaddr	[0x00d4]
_def dma3_dstindex	[0x00d5]
_def dma3_link		[0x00d6]
_def dma3_auxcount	[0x00d7]
_def dma3_auxlink	[0x00d8]

_def dma4_control	[0x00e0]
_def dma4_srcaddr	[0x00e1]
_def dma4_srcindex	[0x00e2]
_def dma4_count		[0x00e3]
_def dma4_dstaddr	[0x00e4]
_def dma4_dstindex	[0x00e5]
_def dma4_link		[0x00e6]
_def dma4_auxcount	[0x00e7]
_def dma4_auxlink	[0x00e8]

_def dma5_control	[0x00f0]
_def dma5_srcaddr	[0x00f1]
_def dma5_srcindex	[0x00f2]
_def dma5_count		[0x00f3]
_def dma5_dstaddr	[0x00f4]
_def dma5_dstindex	[0x00f5]
_def dma5_link		[0x00f6]
_def dma5_auxcount	[0x00f7]
_def dma5_auxlink	[0x00f8]

_def 'dcr_dmapriB	0
_def 'dcr_dmapriM	[0b11]
_def 'dcr_dmapri['p]	[(p & 0b11)]

_def 'dcr_tranmodeB	2
_def 'dcr_tranmodeM	[(0b11 << dcr_tranmodeB)]
_def 'dcr_tranmode['x]	[((x & 0b11) << dcr_tranmodeB)]

_def 'dcr_auxtranmodeB	4
_def 'dcr_auxtranmodeM	[(0b11 << dcr_auxtranmodeB)]
_def 'dcr_auxtranmode['x]	[((x & 0b11) << dcr_auxtranmodeB)]

_def 'dcr_syncmodeB	6
_def 'dcr_syncmodeM	[(0b11 << dcr_syncmodeB)]
_def 'dcr_syncmode['x]	[((x & 0b11) << dcr_syncmodeB)]

_def 'dcr_autoinitstatic	[(1 << 8)]
_def 'dcr_auxautoinitstatic	[(1 << 9)]
_def 'dcr_autoinitsync		[(1 << 10)]
_def 'dcr_auxautoinitsync	[(1 << 11)]

_def 'dcr_readbitrev	[(1 << 12)]
_def 'dcr_writebitrev	[(1 << 13)]

_def 'dcr_splitmode	[(1 << 14)]

_def 'dcr_comportB	15
_def 'dcr_comportM	[(0b111 << dcr_comportB)]
_def 'dcr_comport['x]	[((x & 0b11) << dcr_comportB)]

_def 'dcr_tcc		[(1 << 18)]
_def 'dcr_auxtcc	[(1 << 19)]

_def 'dcr_tcintflag	[(1 << 20)]
_def 'dcr_auxtcintflag	[(1 << 21)]

_def 'dcr_startB	22
_def 'dcr_startM	[(0b11 << dcr_startB)]
_def 'dcr_start['x]	[((x & 0b11) << dcr_startB)]

_def 'dcr_auxstartB	24
_def 'dcr_auxstartM	[(0b11 << dcr_auxstartB)]
_def 'dcr_auxstart['x]	[((x & 0b11) << dcr_auxstartB)]

_def 'dcr_statusB	26
_def 'dcr_statusM	[(0b11 << dcr_statusB)]
_def 'dcr_status['x]	[((x & 0b11) << dcr_statusB)]

_def 'dcr_auxstatusB	28
_def 'dcr_auxstatusM	[(0b11 << dcr_auxstatusB)]
_def 'dcr_auxstatus['x]	[((x & 0b11) << dcr_auxstatusB)]

_def 'dcr_prioritymode	[(1 << 30)]



-- end of c40mmap.m
