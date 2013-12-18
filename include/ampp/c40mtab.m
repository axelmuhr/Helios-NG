-- File:	c40mtab.m
-- Subsystem:	'C40 Helios AMPP macros
-- Author:	Paul Beskeen
-- Date:	June '92
--
-- Description: `C40 module table access macros
--
--
-- RcsId: $Id: c40mtab.m,v 1.3 1992/12/03 09:26:11 nickc Exp $
--
-- (C) Copyright 1992 Perihelion Software Ltd.


_report ['include c40mtab.m]
_def 'c40mtab.m_flag 1


include c40.m


-------------------------------------------------------------------------------
-- Module Table access macros
-- These macros assume a PCS complient environment


-- Call 'codelabel' function through the module table
-- This will work for codetable's of up to 255 entries and module slot numbers
-- of up to 255.

_def 'ExternCall['codelabel] [
	-- get codetable pointer for module containing `codelabel()'
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_'$codelabel)),
		ldi	*+R_MT(0), R_ATMP)
	-- get pointer to codelabel() from codetable
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, codesymb(_'$codelabel)),
		ldi	*+R_ATMP(0), R_ATMP)
	-- jump to fn()
	laju	R_ATMP
		nop
		nop
		nop
]

-- Delayed version of above - next three instructions executed before call
_def 'ExternCallDelayed['codelabel] [
	-- get codetable pointer for module containing `codelabel()'
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_'$codelabel)),
		ldi	*+R_MT(0), R_ATMP)
	-- get pointer to codelabel() from codetable
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, codesymb(_'$codelabel)),
		ldi	*+R_ATMP(0), R_ATMP)
	-- jump to fn()
	laju	R_ATMP
]


-- Branch to 'codelabel' function through the module table.
-- This will work for codetables of up to 64k entries (big enough I think)
-- and module slot numbers up to the same size (more than big enough I think).

_def 'ExternBigCall['codelabel] [
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_'$codelabel)),
		ldi	R_MT, R_ATMP)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_'$codelabel)),
		addi	1, R_ATMP)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*R_ATMP, R_ATMP)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, R_ATMP)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		laju	R_ATMP)
			nop
			nop
			nop
]

-- delayed version of above - next three instructions executed before call
_def 'ExternBigCallDelayed['codelabel] [
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_'$codelabel)),
		ldi	R_MT, R_ATMP)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_'$codelabel)),
		addi	1, R_ATMP)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*R_ATMP, R_ATMP)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, R_ATMP)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		laju	R_ATMP)
]


-- Branch to 'codelabel' function through the module table
-- This will work for codetable's of up to 255 entries and module slot numbers
-- of up to 255.

_def 'ExternBranch['codelabel] [
	-- get codetable pointer for module containing `codelabel()'
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_'$codelabel)),
		ldi	*+R_MT(0), R_ATMP)
	-- get pointer to codelabel() from codetable
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, codesymb(_'$codelabel)),
		ldi	*+R_ATMP(0), R_ATMP)
	-- jump to fn()
	b	R_ATMP
]

-- Branch to 'codelabel' function through the module table.
-- This will work for codetables of up to 64k entries (big enough I think)
-- and module slot numbers up to the same size (more than big enough I think).

_def 'ExternBigBranch['codelabel] [
	patchinstr(PATCHC40DATAMODULE1,
		shift(-2, datamodule(_'$codelabel)),
		ldi	R_MT, R_ATMP)
	patchinstr(PATCHC40DATAMODULE2,
		shift(-2, codesymb(_'$codelabel)),
		addi	1, R_ATMP)
	patchinstr(PATCHC40DATAMODULE3,
		modnum, 
		ldi	*R_ATMP, R_ATMP)
	patchinstr(PATCHC40DATAMODULE4,
		modnum, 
		addi	0, R_ATMP)
	patchinstr(PATCHC40DATAMODULE5,
		modnum, 
		bu	R_ATMP)
	b	R_ATMP
]


-- Get hold of pointer to `codelabel' function from modtab codetable and place
-- it into the address register `Areg'.
-- This will work for codetable's of up to 255 entries and module slot numbers
-- of up to 255.

_def 'GetFunctionPointer['codelabel 'Areg] [
	-- get codetable pointer for module containing `codelabel()'
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_'$codelabel)),
		ldi	*+R_MT(0), Areg)
	-- get pointer to codelabel() from codetable
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, codesymb(_'$codelabel)),
		ldi	*+Areg(0), Areg)
]

-- Get pointer to the 'datalabel' data item in its module's static data area and
-- place it into `reg'. This macro can cope with up to 256KB sized static data
-- areas, but the modules slot id must be less than 256.

_def 'StaticDataAddress['datalabel 'reg] [
	-- get pointer to module containing `datalabel' static data area
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_'$datalabel)),
		ldi	*+R_MT(0), reg)
	C40WordAddress reg
	-- get pointer to `datalabel' in static data area
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, datasymb(_'$datalabel)),
		addi	0, reg)
]

-- Same as above, but returns a C byte address, not a word address
_def 'StaticDataCAddress['datalabel 'reg] [
	-- get pointer to module containing `datalabel' static data area
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_'$datalabel)),
		ldi	*+R_MT(0), reg)
	-- get pointer to `datalabel' in static data area
	patchinstr(PATCHC40MASK16ADD,
		datasymb(_'$datalabel),
		addi	0, reg)
]


-- Get value of `datalabel' item of word data from module's static data area into
-- an address register. This macro can ONLY cope with up to 1KB sized static data
-- areas, and the modules slot id must be less than 256.

_def 'GetStaticDataWord['datalabel 'Areg] [
	-- get pointer to module containing `datalabel' static data area
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_'$datalabel)),
		ldi	*+R_MT(0), Areg)
	C40WordAddress Areg
	-- get pointer to `datalabel' in static data area
	-- and get data from it
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datasymb(_'$datalabel)),
		ldi	*+Areg(0), Areg)
]

-- Put `reg' registers contents into `datalabel' item of word data in its module's
-- static data area. This macro can ONLY cope with up to 1KB sized static data
-- areas, and the modules slot id must be less than 256.
-- It also corrupts the R_ATMP register which is left holding the address of the
-- start of the static data area for that module.

_def 'PutStaticDataWord['reg 'datalabel] [
	-- get pointer to module containing `datalabel' static data area
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datamodule(_'$datalabel)),
		ldi	*+R_MT(0), R_ATMP)
	C40WordAddress R_ATMP
	-- get pointer to `datalabel' in static data area
	-- and put data from `reg' into it
	patchinstr(PATCHC40MASK8ADD,
		shift(-2, datasymb(_'$datalabel)),
		sti	reg, *+R_ATMP(0))
]


-------------------------------------------------------------------------------
-- Internal Stub calls
-- Call to internal label that could just be a stub to an external call.


-- Call  'codelabel' function through stub (stubs are always prefixed with '.')
-- The stub may be +/- 128k away.

_def 'StubCall['codelabel] [
	patchinstr(PATCHC40MASK24ADD,
		shift(-2, codestub(.'$codelabel)),
		laj	0)
	nop
	nop
	nop
]

-- delayed version of above - next three instructions executed before call
_def 'StubCallDelayed['codelabel] [
	patchinstr(PATCHC40MASK24ADD,
		shift(-2, codestub(.'$codelabel)),
		laj	0)
]

-- direct branches to stubs, forcing any return to return directly to our caller
_def 'StubBranch['codelabel] [
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, codestub(.'$codelabel)),
		b	0)
	nop
	nop
	nop
]

-- delayed version of above - next three instructions executed before call
_def 'StubBranchDelayed['codelabel] [
	patchinstr(PATCHC40MASK16ADD,
		shift(-2, codestub(.'$codelabel)),
		bud	0)
]


-- end of c40mtab.m
