--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- message.m                                                            --
--                                                                      --
--      Message and message control block structures                    --
--                                                                      --
--      Author: NHG 20-May-87						--
--                                                                      --
-- 	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include message.m]
_def 'message.m_flag 1

-- Message header

struct MsgHdr [
        byte    DataSizeLo              -- 16 bit data size
        byte    DataSizeHi
        byte    ContSize                -- control vector size
        byte    Flags                   -- flag byte
        word    Dest                    -- destination port descriptor
        word    Reply                   -- reply port descriptor
        word    FnRc                    -- function/return code
]


_test _defp 'helios.TRAN [
	_def    MsgHdr.DataSize.mask    [#FFFF] -- mask for data size field

	_def    MsgHdr.Flags.nothdr     [#80]	-- used by kernel
	_def    MsgHdr.Flags.preserve   [#40]	-- preserve destination route
	_def    MsgHdr.Flags.exception  [#20]	-- excption message type
	_def    MsgHdr.Flags.uselink    [#10]	-- reveive data from link
	_def    MsgHdr.Flags.link       [#0F]	-- link id
][
	_def    MsgHdr.DataSize.mask    [0xFFFF] -- mask for data size field

	_def    MsgHdr.Flags.nothdr     [0x80]	-- used by kernel
	_def    MsgHdr.Flags.preserve   [0x40]	-- preserve destination route
	_def    MsgHdr.Flags.exception  [0x20]	-- excption message type
	_def    MsgHdr.Flags.uselink    [0x10]	-- reveive data from link
	_def    MsgHdr.Flags.link       [0x0F]	-- link id
]


_def    Max.Cont.size           [16]    -- maximum size of control vector
_def    Max.Data.size           [1024]  -- maximum size of data vector

-- Message Control Block
-- note that the first 4 words are identical to a MsgHdr

struct MCB [
        byte    DataSizeLo              -- 16 bit data size
        byte    DataSizeHi
        byte    ContSize                -- control vector size
        byte    Flags                   -- flag byte
        word    Dest                    -- destination port descriptor
        word    Reply                   -- reply port descriptor
        word    FnRc                    -- function/return code

        word    Timeout                 -- message timeout
        word    Control                 -- pointer to control buffer
        word    Data                    -- pointer to data buffer
]


-- End of message.m
