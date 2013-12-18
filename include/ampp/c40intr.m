-- File:	c40intr.m
-- Subsystem:	'C40 Helios AMPP macros
-- Author:	Paul Beskeen
-- Date:	Feb '92
--
-- Description: `C40 interrupt vector offsets
--
--
-- RcsId: $Id: c40intr.m,v 1.2 1992/07/01 09:03:58 paul Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.


_report ['include c40intr.m]
_def 'c40intr.m_flag 1


_def	'iv_reset	0
_def	'iv_nmi		1
_def	'iv_tint0	2
_def	'iv_iiof0	3
_def	'iv_iiof1	4
_def	'iv_iiof2	5
_def	'iv_iiof3	6

_def	'iv_icfull0	[0xd]
_def	'iv_icrdy0	[0xe]
_def	'iv_ocrdy0	[0xf]
_def	'iv_ocempty0	[0x10]

_def	'iv_icfull1	[0x11]
_def	'iv_icrdy1	[0x12]
_def	'iv_ocrdy1	[0x13]
_def	'iv_ocempty	[0x14]

_def	'iv_icfull2	[0x15]
_def	'iv_icrdy2	[0x16]
_def	'iv_ocrdy2	[0x17]
_def	'iv_ocempty2	[0x18]

_def	'iv_icfull3	[0x19]
_def	'iv_icrdy3	[0x1a]
_def	'iv_ocrdy3	[0x1b]
_def	'iv_ocempty3	[0x1c]

_def	'iv_icfull4	[0x1d]
_def	'iv_icrdy4	[0x1e]
_def	'iv_ocrdy4	[0x1f]
_def	'iv_ocempty4	[0x20]

_def	'iv_icfull5	[0x21]
_def	'iv_icrdy5	[0x22]
_def	'iv_ocrdy5	[0x23]
_def	'iv_ocempty5	[0x24]

_def	'iv_dmaint0	[0x25]
_def	'iv_dmaint1	[0x26]
_def	'iv_dmaint2	[0x27]
_def	'iv_dmaint3	[0x28]
_def	'iv_dmaint4	[0x29]
_def	'iv_dmaint5	[0x2a]

_def	'iv_tint1	[0x2b]


-- link intrrupt vector offsets
_def	'iv_icfull	[0x0]
_def	'iv_icrdy	[0x1]
_def	'iv_ocrdy	[0x2]
_def	'iv_ocempty	[0x3]



-- end of c40intr.m
