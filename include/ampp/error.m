--------------------------------------------------------------------------
--                                                                      --
--                      H E L I O S   K E R N E L                       --
--                      -------------------------                       --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- error.m                                                              --
--                                                                      --
--      Error codes                                                     --
--                                                                      --
--      Author: NHG 20-May-87						--
--                                                                      --
--	SCCS Id: %W% %G%						--
--------------------------------------------------------------------------

_report ['include error.m]
_def 'error.m_flag 1

_def 'ErrBit	[#80000000]

_def 'SS_Kernel	[#01000000]

_def 'EC_Recover	ErrBit			-- a retry might succeed
_def 'EC_Warn		[ErrBit+#20000000]	-- recover & retry
_def 'EC_Error		[ErrBit+#40000000]	-- client fatal
_def 'EC_Fatal		[ErrBit+#60000000]	-- system/server fatal

_def 'EG_Mask		[#00FF0000]	-- mask to isolate

_def 'EG_NoMemory	[#00010000]	-- memory allocation failure	
_def 'EG_Create		[#00020000]	-- failed to create		
_def 'EG_Delete		[#00030000]	-- failed to delete
_def 'EG_Protected	[#00040000]	-- object is protected		
_def 'EG_Timeout	[#00050000]	-- timeout			
_def 'EG_Unknown	[#00060000]	-- object not found		
_def 'EG_FnCode		[#00070000]	-- unknown function code	
_def 'EG_Name		[#00080000]	-- mal-formed name		
_def 'EG_Invalid	[#00090000]	-- invalid/corrupt object	
_def 'EG_InUse		[#000a0000]	-- object in use/locked
_def 'EG_Congested	[#000b0000]	-- server/route overloaded
_def 'EG_Broken		[#000d0000]	-- object broken in some way
_def 'EG_Exception	[#000e0000]	-- exception message

_def 'EO_Message	[#00008001]	-- error refers to a message
_def 'EO_Task		[#00008002]	-- error refers to a task
_def 'EO_Port		[#00008003]	-- error refers to a port
_def 'EO_Route		[#00008004]	-- error refers to a route
_def 'EO_Link		[#00008012]	-- error refers to a physical link

_def 'EE_Kill		[#00000004]	-- kill exception code
_def 'EE_Abort		[#00000005]	-- abort exception code
_def 'EE_Interrupt	[#00000008]	-- console interrupt

_defq 'Error['name 'code]
[
        _def [Err_$name] [ldc code]
]

Error 'Null		[0]
Error Timeout		[EC_Recover+SS_Kernel+EG_Timeout+EO_Message]
Error BadPort		[EC_Warn+SS_Kernel+EG_Invalid+EO_Port]
Error Aborted		[EC_Error+SS_Kernel+EG_Aborted+EO_Port]
Error InUse		[EC_Recover+SS_Kernel+EG_InUse+EO_Port]
Error BadSurrogate	[EC_Warn+SS_Kernel+EG_Invalid+EO_Port]
Error BadRoute		[EC_Warn+SS_Kernel+EG_Invalid+EO_Route]
Error NoMemory		[EC_Warn+SS_Kernel+EG_NoMemory]
Error Congestion	[EC_Recover+SS_Kernel+EG_Congested+EO_Route]
Error Kill		[EC_Fatal+SS_Kernel+EG_Exception+EE_Kill]
Error Abort		[EC_Error+SS_Kernel+EG_Exception+EE_Abort]
Error NotReady		[EC_Recover+SS_Kernel+EG_Congested+EO_Port]
Error BadLink		[EC_Warn+SS_Kernel+EG_Invalid+EO_Link]

-- End of error.m
