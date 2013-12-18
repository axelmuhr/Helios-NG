--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- sem.m                                                                --
--                                                                      --
--      Sempahore structure definition                                  --
--                                                                      --
--      Author: NHG 14-Jun-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include sem.m]
_def 'sem.m_flag 1

include structs.m

struct Sem [
        word Count              -- semaphore counter
        word Head               -- head of process list
        word Tail               -- tail of process list
]

-- End of sem.m
