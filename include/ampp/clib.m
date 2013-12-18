--------------------------------------------------------------------------
--                                                                      --
--                  H E L I O S   `C` RUNTIME LIBRARY                   --
--                  ---------------------------------                   --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- clib.h                                                               --
-- ======                                                               --
--      Header file for C runtime system. Defines stack frame etc.      --
--                                                                      --
--      Author: CHRG 6/10/87						--
--                                                                      --
--------------------------------------------------------------------------
--------------------------------------------------------------------------
-- UPDATES :                                                            --
--------------------------------------------------------------------------

_report ['include clib.m]
_def 'clib.m_flag 1

-- sccsid  [%W%    %G% Copyright (C) 1987, Perihelion Software Ltd.]

include basic.m
include procs.m
include structs.m
include root.m



-- Proceedure stack frame layout is as follows
-- ================
--
--words:        L         1    1    1    1                             
--	+--------------------------------------------------------+
--	|     Locals	| L  |*MD |Arg1|Arg2|  Arg3- Argn        |
--	+--------------------------------------------------------+
--						                 ^
--			^    				      Wptr before call
--	         Wptr after call 
--
-- Locals = local variables
-- L = Link (return address)
-- MD = Module descriptor 
-- Args = proceedure arguments
--
--
-- The module descriptor is a two word descriptor :
--
--      +---------+
--      | M  | V  |
--      +---------+
--
-- M = pointer to Task module table
-- V = proceedure vector stack pointer
--
--
--
--
--
-- Process stack frame layout is as follows
-- =============
--
--words:        L     1   1   1    A                  MD
--	+---------------------------------------------------+
--	|   Locals  |fn | L |*MD|  Args             | M | V |
--	+---------------------------------------------------+
--			        ^
--			^      ptr returned by NewProcess
--                      ^
--	         Wptr after RunProcess call (i.e of New Process) 
--
-- Locals = local variables
-- fn = entry point of New Process
-- L = Link (return address)
-- MD = Module descriptor
-- M = Module table pointer
-- V = Vector pointer
-- Args = proceedure arguments
--
--



-- STACK FRAME LITERALS

_def	'frame.size	[2]	-- stackframe size in words
_def	'link.ptr	[0]	-- offset of Link pointer
_def	'md.ptr		[1]	-- offset of Module Descriptor pointer
_def	'md.size   	[2]     -- size of module descriptor
_def	'module.ptr   	[0]     -- offset of module table pointer within md
_def	'vector.ptr	[1]	-- offset of Vector pointer within md



-- End clib.h
