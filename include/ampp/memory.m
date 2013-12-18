--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- memory.m                                                             --
--                                                                      --
--      Memory control structures                                       --
--                                                                      --
--      Author: NHG 20-May-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include memory.m]
_def 'memory.m_flag 1

include structs.m
include queue.m

struct Pool [
        struct Node     Node            -- queueing node
        struct List     Memory          -- list of memory blocks
        word            Blocks          -- number of blocks in pool
        word            Size            -- total size of pool
        word            Max             -- largest block in pool
]

struct Memory [
        struct Node     Node            -- link in pool list
        word            Size            -- block size + alloc bits
        word            Pool            -- pointer to owning pool
]

_defq 'Memory.Size.BitMask  [15]         -- mask for allocation bits
_defq 'Memory.Size.BwdBit   [2]         -- allocation state of predecessor
_defq 'Memory.Size.FwdBit   [1]         -- allocation state of this block
_defq 'Memory.Size.Fast	    [4]		-- block is for a fast RAM area

_def 'epsilon [Memory.sizeof]


-- Fast Ram carrier

struct Carrier [
	word		Addr		-- Fast RAM block address
	word		Size		-- and its size
]

-- End of memory.m
