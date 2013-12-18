/*------------------------------------------------------------------------
--                                                                      --
--                     H E L I O S   N U C L E U S                      --
--                     ---------------------------                      --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- codes.h								--
--                                                                      --
--	Error and Function code encoding.				--
--									--
--	Function codes:							--
--	A 32-bit function code is interpreted as follows:		--
--		0 CC SSSSS GGGGGGGGGGGGGGGGGGG FFFF			--
--	where:								--
--	C = function class:						--
--		00 = GSP request					--
--		11 = private protocol					--
--	S = Subsystem identifier, or zero if unknown			--
--	G = General function code					--
--	F = Server specific subfunction					--
--									--
--	Error codes:							--
--	A 32-bit error code is interpreted as follows:			--
--		1 CC SSSSS GGGGGGGG EEEEEEEEEEEEEEEE			--
--	where:								--
--	C = Error class:						--
--		00 = Recoverable					--
--		01 = Warning						--
--		10 = Error						--
--		11 = Fatal						--
--	S = Subsystem identifier					--
--	G = General error code						--
--	E = Specific/object error code					--
--                                                                      --
--	NOTE:								--
--		Any changes/additions made here should also be made	--
--		in fault.c, or at least recompile it.			--
--		In a perfect world both these files would be generated	--
--		from a single source.					--
--									--
--	Author:  NHG 11/9/87						--
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: %W%	%G% Copyright (C) 1987, Perihelion Software Ltd.	*/

#ifndef __codes_h
#define __codes_h

#ifndef __helios_h
#include <helios.h>
#endif


/*----------------------------------------------------------------
-- 			Subsystems				--
----------------------------------------------------------------*/

#define SS_Mask		0x1f000000
#define SS_Shift	24

#define SS_Kernel	0x01000000
#define SS_SysLib	0x02000000
#define SS_ProcMan	0x03000000
#define SS_Loader	0x04000000
#define SS_TFM		0x05000000
#define SS_RamDisk	0x06000000
#define SS_HardDisk	0x07000000
#define SS_Fifo		0x08000000
#define SS_NameTable	0x09000000
#define SS_IOProc	0x0a000000
#define SS_Window	0x0b000000
#define SS_IOC		0x0c000000

/*----------------------------------------------------------------
-- 			Function Codes				--
----------------------------------------------------------------*/

#define FF_Mask		0x0000000f	/* mask for sub-function	*/

/*----------------------------------------------------------------
-- Function Classes						--
----------------------------------------------------------------*/

#define FC_Mask		0x60000000
#define FC_GSP		0x00000000
#define FC_Private	0x60000000

/*----------------------------------------------------------------
-- General Functions						--
----------------------------------------------------------------*/

#define FG_Mask		0x00FFFFF0
#define FG_Shift	4

/* IOC requests */
#define FG_Open		0x00000010
#define FG_Create	0x00000020
#define FG_Locate	0x00000030
#define FG_ObjectInfo	0x00000040
#define FG_ServerInfo	0x00000050
#define FG_Delete	0x00000060
#define FG_Rename	0x00000070
#define FG_Link		0x00000080
#define FG_Protect	0x00000090
#define FG_SetDate	0x000000a0
#define FG_Refine	0x000000b0
#define FG_CloseObj	0x000000c0

#define IOCFns		(FG_CloseObj>>FG_Shift)

/* direct server requests */
#define FG_Read		0x00001010
#define FG_Write	0x00001020
#define FG_GetSize	0x00001030
#define FG_SetSize	0x00001040
#define FG_Close	0x00001050
#define FG_Seek		0x00001060
#define FG_GetInfo	0x00001070
#define FG_SetInfo	0x00001080
#define FG_EnableEvents	0x00001090
#define FG_Acknowledge	0x000010A0
#define FG_NegAcknowledge 0x0010B0

/* Distributed search codes */
#define FG_Search	0x00002010

/* Task/IOC private messages	*/
#define FG_MachineName	0x00003010
#define FG_Debug	0x00003020
#define FG_Alarm	0x00003030

/* Parent->TFM->Task private messages	*/
#define FG_SendEnv	0x00004010
#define FG_Signal	0x00004020

/*----------------------------------------------------------------
-- 			Error Codes				--
----------------------------------------------------------------*/

#define ErrBit		0x80000000	/* set for all error codes	*/
#define Err_Null	0L		/* no error at all		*/

/*----------------------------------------------------------------
-- Error Classes						--
----------------------------------------------------------------*/

#define EC_Mask		0xe0000000
#define EC_Shift	29

#define EC_Recover	ErrBit			/* a retry might succeed */
#define EC_Warn		ErrBit+0x20000000	/* recover & try again	 */
#define EC_Error	ErrBit+0x40000000	/* client fatal		 */
#define EC_Fatal	ErrBit+0x60000000	/* system fatal		 */

/*----------------------------------------------------------------
-- General Error codes						--
----------------------------------------------------------------*/

#define EG_Mask		0x00FF0000	/* mask to isolate		*/
#define EG_Shift	24

#define	EG_NoMemory	0x00010000	/* memory allocation failure	*/
#define EG_Create	0x00020000	/* failed to create		*/
#define EG_Delete	0x00030000	/* failed to delete		*/
#define EG_Protected	0x00040000	/* object is protected		*/
#define EG_Timeout	0x00050000	/* timeout			*/
#define EG_Unknown	0x00060000	/* object not found		*/
#define EG_FnCode	0x00070000	/* unknown function code	*/
#define EG_Name		0x00080000	/* mal-formed name		*/
#define EG_Invalid	0x00090000	/* invalid/corrupt object	*/
#define EG_InUse	0x000a0000	/* object in use/locked		*/
#define EG_Congested	0x000b0000	/* server/route overloaded	*/
#define EG_WrongFn	0x000c0000	/* fn inappropriate to object	*/
#define EG_Broken	0x000d0000	/* object broken in some way	*/
#define EG_Exception	0x000e0000	/* exception message		*/

#define EG_Parameter	0x00ff0000	/* bad parameter value		*/

/*----------------------------------------------------------------
-- Object codes for general errors				--
----------------------------------------------------------------*/

#define EO_Mask		0x0000ffff

#define EO_Message	0x00008001	/* error refers to a message	*/
#define	EO_Task		0x00008002	/* error refers to a task	*/
#define EO_Port		0x00008003	/* error refers to a port	*/
#define EO_Route	0x00008004	/* error refers to a route	*/
#define EO_Directory	0x00008005	/* error refers to a directory	*/
#define EO_Object	0x00008006	/* error refers to Object struct*/
#define EO_Stream	0x00008007	/* error refers to Stream	*/
#define EO_Program	0x00008008
#define EO_Module	0x00008009
#define EO_Matrix	0x0000800a	/* access matrix		*/
#define EO_Fifo		0x0000800b
#define EO_File		0x0000800c
#define EO_Capability	0x0000800d
#define EO_Name		0x0000800e	/* name in name table		*/
#define EO_Window	0x0000800f
#define EO_Server	0x00008010	
#define EO_TaskForce	0x00008011	/* error refers to Task Force   */
#define EO_Link		0x00008012
#define EO_Memory	0x00008013

/*----------------------------------------------------------------
-- Exception codes						--
----------------------------------------------------------------*/

#define EE_Mask		0x0000ffff	/* isolation mask		*/

#define EE_Kill		0x00000004	/* kill exception code		*/
#define EE_Abort	0x00000005	/* abort exception code		*/
#define EE_Suspend	0x00000006	/* suspend task(s)		*/
#define EE_Restart	0x00000007	/* restart suspended task	*/
#define EE_Interrupt	0x00000008	/* console interrupt		*/
#define EE_ErrorFlag	0x00000009	/* error flags was set by task	*/
#define EE_StackError	0x0000000a	/* task has overflowed its stack*/

/*----------------------------------------------------------------
-- Task Force Status Masks					--
----------------------------------------------------------------*/

#define TF_CMASK	0x000000FF	/* TF completed Mask		*/
#define TF_EMASK	0x00000FFF	/* TF executing Mask		*/
#define TF_LMASK	0xFF000000	/* TF Loaded Mask		*/

/*----------------------------------------------------------------
-- Kernel errors						--
----------------------------------------------------------------*/

#define EK_Timeout	(EC_Recover+SS_Kernel+EG_Timeout+EO_Message)

/*----------------------------------------------------------------
-- Processor Manager Errors					--
----------------------------------------------------------------*/

#define EP_Unknown	(EC_Error+SS_ProcMan+EG_Unknown)
#define EP_BadFunction	(EC_Error+SS_ProcMan+EG_FnCode)
#define EP_Protected	(EC_Error+SS_ProcMan+EG_Protected)
#define EP_Name		(EC_Error+SS_ProcMan+EG_Name)

#endif

/* -- End of codes.h */
