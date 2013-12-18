/*-------------------------------------------------------------------------*/
/*--                                                                     --*/
/*--                     H E L I O S   N U C L E U S                     --*/
/*--                     ---------------------------                     --*/
/*--                                                                     --*/
/*--             Copyright (C) 1987, Perihelion Software Ltd.            --*/
/*--          Copyright (c) 1994, Perihelion Distribiuted Software.      --*/
/*--                        All Rights Reserved.                         --*/
/*--                                                                     --*/
/*-- codes.h                                                             --*/
/*--                                                                     --*/
/*--     Error and Function code encoding.                               --*/
/*--                                                                     --*/
/*--     Function codes:                                                 --*/
/*--     A 32-bit function code is interpreted as follows:               --*/
/*--             0 CC SSSSS RRRR GGGGGGGGGGGGGGGG FFFF                   --*/
/*--     where:                                                          --*/
/*--     C = function class:                                             --*/
/*--             00 = GSP request                                        --*/
/*--             11 = private protocol                                   --*/
/*--     S = Subsystem identifier, or zero if unknown                    --*/
/*--     R = Retry counter                                               --*/
/*--     G = General function code                                       --*/
/*--     F = Server specific subfunction                                 --*/
/*--                                                                     --*/
/*--     Error codes:                                                    --*/
/*--     A 32-bit error code is interpreted as follows:                  --*/
/*--             1 CC SSSSS GGGGGGGG EEEEEEEEEEEEEEEE                    --*/
/*--     where:                                                          --*/
/*--     C = Error class:                                                --*/
/*--             00 = Recoverable                                        --*/
/*--             01 = Warning                                            --*/
/*--             10 = Error                                              --*/
/*--             11 = Fatal                                              --*/
/*--     S = Subsystem identifier                                        --*/
/*--     G = General error code                                          --*/
/*--     E = Specific/object error code                                  --*/
/*--                                                                     --*/
/*--     WARNING: this file is generated automatically from the master   --*/
/*--     codes databse (faults/master.fdb). Any changes here will        --*/
/*--     be overwritten.                                                 --*/
/*--                                                                     --*/
/*--     Author:  NHG 11/9/87                                            --*/
/*--                                                                     --*/
/*-------------------------------------------------------------------------*/

#ifndef __codes_h
#define __codes_h

#ifndef __helios_h
#include <helios.h>
#endif

/*----------------------------------------------------------------------*/
/*-- Subsystems                                                       --*/
/*----------------------------------------------------------------------*/

#define	SS_Mask		    0x1f000000
#define	SS_Shift	    24

#define	SS_Unknown	    0x00000000	/* Unknown Subsystem		*/
#define	SS_Kernel	    0x01000000
#define	SS_SysLib	    0x02000000	/* System Library		*/
#define	SS_ProcMan	    0x03000000	/* Processor Manager		*/
#define	SS_Loader	    0x04000000
#define	SS_TFM		    0x05000000	/* Task Force Manager		*/
#define	SS_RamDisk	    0x06000000	/* Ram Disk			*/
#define	SS_HardDisk	    0x07000000	/* File System			*/
#define	SS_Fifo		    0x08000000	/* Fifo Server			*/
#define	SS_NameTable	    0x09000000	/* Name Server			*/
#define	SS_IOProc	    0x0a000000	/* I/O Processor		*/
#define	SS_Window	    0x0b000000	/* Window Server		*/
#define	SS_IOC		    0x0c000000	/* IO Controller		*/
#define	SS_NullDevice	    0x0d000000	/* Null Device			*/
#define	SS_Pipe		    0x0e000000	/* Pipe Subsystem		*/
#define	SS_Batch	    0x0f000000	/* Batch Server			*/
#define	SS_Unused1	    0x10000000
#define	SS_NetServ	    0x11000000	/* Network Manager		*/
#define	SS_SM		    0x12000000	/* Session Manager		*/
#define	SS_Device	    0x13000000	/* Device			*/
#define	SS_InterNet	    0x14000000	/* Internet Server		*/
#define	SS_RomDisk	    0x15000000	/* Rom Disk			*/
#define	SS_Executive	    0x16000000	/* Executive			*/
#define	SS_HServer	    0x17000000	/* /helios Server		*/
#define	SS_RmLib	    0x18000000	/* Resource Management Library	*/
#define	SS_FloppyDisk	    0x19000000	/* Floppy Disk			*/
#define	SS_Keyboard	    0x1A000000	/* Keyboard Server		*/
#define	SS_Logger	    0x1B000000	/* Error and Debug Message Logger*/
#define	SS_Pointer	    0x1C000000	/* Mouse or Stylus Server	*/
#define	SS_Clock	    0x1D000000	/* Clock Server			*/
#define	SS_Unused2	    0x1E000000
#define	SS_User		    0x1F000000	/* User Written Server		*/

/*----------------------------------------------------------------------*/
/*-- Function Classes                                                 --*/
/*----------------------------------------------------------------------*/

#define	FC_Mask		    0x60000000
#define	FC_Shift	    29

#define	FC_GSP		    0x00000000	/* General Server Protocol	*/
#define	FC_Private	    0x60000000	/* Private Protocol		*/

/*----------------------------------------------------------------------*/
/*-- Function Retry Counter                                           --*/
/*----------------------------------------------------------------------*/

#define	FR_Mask		    0x00F00000
#define	FR_Shift	    20

#define	FR_Inc		    0x00100000	/* Retry count increment	*/

/*----------------------------------------------------------------------*/
/*-- General Functions                                                --*/
/*----------------------------------------------------------------------*/

#define	FG_Mask		    0x000FFFF0
#define	FG_Shift	    4

#define	FG_Unknown	    0x00000000	/* Unknown Function		*/

/*IOC requests		    		   				*/
#define	FG_Open		    0x00000010
#define	FG_Create	    0x00000020
#define	FG_Locate	    0x00000030
#define	FG_ObjectInfo	    0x00000040
#define	FG_ServerInfo	    0x00000050
#define	FG_Delete	    0x00000060
#define	FG_Rename	    0x00000070
#define	FG_Link		    0x00000080
#define	FG_Protect	    0x00000090
#define	FG_SetDate	    0x000000a0
#define	FG_Refine	    0x000000b0
#define	FG_CloseObj	    0x000000c0
#define	FG_Revoke	    0x000000d0
#define	FG_Reserved1	    0x000000e0
#define	FG_Reserved2	    0x000000f0

#define FG_LastIOCFn   FG_Reserved2
#define IOCFns         15

/*direct server requests    		   				*/
#define	FG_Read		    0x00001010
#define	FG_Write	    0x00001020
#define	FG_GetSize	    0x00001030
#define	FG_SetSize	    0x00001040
#define	FG_Close	    0x00001050
#define	FG_Seek		    0x00001060
#define	FG_GetInfo	    0x00001070
#define	FG_SetInfo	    0x00001080
#define	FG_EnableEvents	    0x00001090
#define	FG_Acknowledge	    0x000010A0
#define	FG_NegAcknowledge   0x000010B0
#define	FG_Select	    0x000010C0
#define	FG_ReadAck	    0x000010D0
#define	FG_Abort	    0x000010E0

/*General Server Functions  		   				*/
#define	FG_ReStart	    0x00001FE0	/* restart server		*/
#define	FG_Terminate	    0x00001FF0	/* terminate server		*/

/*ProcMan->ProcMan private messages	   				*/
#define	FG_Search	    0x00002010	/* distributed search		*/
#define	FG_FollowTrail	    0x00002020	/* follow port trail		*/

/*Loader private message    		   				*/
#define	FG_ServerSearch	    0x00002030

/*Task->IOC private messages		   				*/
#define	FG_MachineName	    0x00003010	/* get processor name		*/
#define	FG_Debug	    0x00003020	/* set debug flags		*/
#define	FG_Alarm	    0x00003030	/* set sleep alarm		*/
#define	FG_Reconfigure	    0x00003040	/* reconfigure processor	*/
#define	FG_SetFlags	    0x00003050	/* set task flags		*/
#define	FG_SetSignalPort    0x00003060	/* set signal port		*/

/*Parent->TFM->ProcMan private messages	   				*/
#define	FG_SendEnv	    0x00004010	/* send environment to task	*/
#define	FG_Signal	    0x00004020	/* send signal to task		*/
#define	FG_ProgramInfo	    0x00004030	/* get task status		*/
#define	FG_RequestEnv	    0x00004040	/* request environment		*/

/*Task->LinkIOC private messages	   				*/
#define	FG_BootLink	    0x00005010	/* boot processor		*/

/*Network Control Functions 		   				*/
#define	FG_NetMask	    0x0000F000
#define	FG_NetStatus	    0x00006000	/* network status message	*/
#define	FG_NetReq	    0x00007000	/* network request message	*/

/*Socket Related Requests   		   				*/
#define	FG_Socket	    0x00008010	/* create socket		*/
#define	FG_Bind		    0x00008020	/* bind socket to address	*/
#define	FG_Listen	    0x00008030	/* set socket connection queue size*/
#define	FG_Accept	    0x00008040	/* accept a connection		*/
#define	FG_Connect	    0x00008050	/* connect to a remote service	*/
#define	FG_SendMessage	    0x00008060	/* send datagram or other message*/
#define	FG_RecvMessage	    0x00008070	/* receieve datagram or other message*/

/*Device Requests	    		   				*/
#define	FG_Format	    0x0000a010	/* format disc			*/
#define	FG_WriteBoot	    0x0000a020	/* write boot area		*/

/*RomDisk -> Loader private request	   				*/
#define	FG_RomCreate	    0x0000b010	/* Create loader entry with image in RomDisk*/

/*Range of function codes for prototyping/private interfaces		*/
#define	FG_PrivateFirst	    0x00010000	/* First private code		*/
#define	FG_PrivateLast	    0x0001fff0	/* Last private code		*/

/*----------------------------------------------------------------------*/
/*-- Sub Functions                                                    --*/
/*----------------------------------------------------------------------*/

#define	FF_Mask		    0x0000000f
#define	FF_Shift	    0

/*Set by ReOpen		    		   				*/
#define	FF_ReOpen	    0x00000001

/*Set by Load and Execute before calling Create				*/
#define	FF_LoadOnly	    0x00000001
#define	FF_Execute	    0x00000002

/*Info type codes for Set and GetInfo	   				*/
#define	FF_SendInfo	    0x00000001	/* send info in GetInfo		*/
#define	FF_Attributes	    0x00000000
#define	FF_Ioctl	    0x00000002	/* Unix style I/O control	*/
#define	FF_SocketInfo	    0x00000004	/* TCP/IP style Socket Info	*/

/*----------------------------------------------------------------------*/
/*-- Error Codes                                                      --*/
/*----------------------------------------------------------------------*/

#define ErrBit         (word)0x80000000        /* set for all error codes      */
#define Err_Null       0L              /* no error at all              */

/*----------------------------------------------------------------------*/
/*-- Error Classes                                                    --*/
/*----------------------------------------------------------------------*/

#define	EC_Mask		    (word)0xe0000000
#define	EC_Shift	    29

#define	EC_Recover	    (word)0x80000000/*Recoverable error		*/
#define	EC_Warn		    (word)0xa0000000/*Warning			*/
#define	EC_Error	    (word)0xc0000000/*Error			*/
#define	EC_Fatal	    (word)0xe0000000/*Fatal error		*/

/*----------------------------------------------------------------------*/
/*-- General Error codes                                              --*/
/*----------------------------------------------------------------------*/

#define	EG_Mask		    0x00FF0000
#define	EG_Shift	    16

#define	EG_UnknownError	    0x00000000	/* unknown error		*/
#define	EG_NoMemory	    0x00010000	/* memory allocation failure for*/
#define	EG_Create	    0x00020000	/* failed to create		*/
#define	EG_Delete	    0x00030000	/* failed to delete		*/
#define	EG_Protected	    0x00040000	/* protected			*/
#define	EG_Timeout	    0x00050000	/* timeout on			*/
#define	EG_Unknown	    0x00060000	/* failed to find		*/
#define	EG_FnCode	    0x00070000	/* unknown function code for	*/
#define	EG_Name		    0x00080000	/* mal-formed name for		*/
#define	EG_Invalid	    0x00090000	/* invalid or corrupt		*/
#define	EG_InUse	    0x000a0000	/* in use/locked		*/
#define	EG_Congested	    0x000b0000	/* congested			*/
#define	EG_WrongFn	    0x000c0000	/* inappropriate function for	*/
#define	EG_Broken	    0x000d0000	/* broken			*/
#define	EG_Exception	    0x000e0000	/* exception			*/
#define	EG_WrongSize	    0x000f0000	/* wrong size for		*/
#define	EG_ReBooted	    0x00100000	/* rebooted			*/
#define	EG_Open		    0x00110000	/* failed to open		*/
#define	EG_Execute	    0x00120000	/* failed to execute		*/
#define	EG_Boot		    0x00130000	/* failed to boot		*/
#define	EG_State	    0x00140000	/* wrong state for		*/
#define	EG_NoResource	    0x00150000	/* insufficient mapping resource for*/
#define	EG_Errno	    0x00160000	/* posix error			*/
#define	EG_CallBack	    0x00170000	/* call back in			*/
#define	EG_WriteProtected   0x00180000	/* write protected		*/
#define	EG_NewTimeout	    0x00190000	/* new timeout			*/
#define	EG_Parameter	    0x00ff0000	/* bad parameter		*/

/*----------------------------------------------------------------------*/
/*-- Object codes for general errors                                  --*/
/*----------------------------------------------------------------------*/

#define	EO_Mask		    0x0000ffff
#define	EO_Shift	    0

#define	EO_Unknown	    0x00008000	/* unknown object		*/
#define	EO_Message	    0x00008001
#define	EO_Task		    0x00008002
#define	EO_Port		    0x00008003
#define	EO_Route	    0x00008004
#define	EO_Directory	    0x00008005
#define	EO_Object	    0x00008006
#define	EO_Stream	    0x00008007
#define	EO_Program	    0x00008008
#define	EO_Module	    0x00008009
#define	EO_Matrix	    0x0000800a
#define	EO_Fifo		    0x0000800b
#define	EO_File		    0x0000800c
#define	EO_Capability	    0x0000800d
#define	EO_Name		    0x0000800e
#define	EO_Window	    0x0000800f
#define	EO_Server	    0x00008010
#define	EO_TaskForce	    0x00008011
#define	EO_Link		    0x00008012
#define	EO_Memory	    0x00008013
#define	EO_Pipe		    0x00008014
#define	EO_NetServ	    0x00008015
#define	EO_Network	    0x00008016
#define	EO_User		    0x00008017
#define	EO_Session	    0x00008018
#define	EO_Loader	    0x00008019
#define	EO_ProcMan	    0x0000801a
#define	EO_TFM		    0x0000801b
#define	EO_Attribute	    0x0000801c
#define	EO_NoProcessors	    0x0000801d	/* number of processors		*/
#define	EO_ProcessorType    0x0000801e
#define	EO_Processor	    0x0000801f
#define	EO_Socket	    0x00008020
#define	EO_Medium	    0x00008021	/* device medium		*/
#define	EO_Password	    0x00008022

/*----------------------------------------------------------------------*/
/*-- Exception codes                                                  --*/
/*----------------------------------------------------------------------*/

#define	EE_Mask		    0x0000ffff
#define	EE_Shift	    0

#define	EE_Null		    0x00000000
#define	EE_Kill		    0x00000004
#define	EE_Abort	    0x00000005
#define	EE_Suspend	    0x00000006
#define	EE_Restart	    0x00000007
#define	EE_Interrupt	    0x00000008
#define	EE_ErrorFlag	    0x00000009
#define	EE_StackError	    0x0000000a
#define	EE_Signal	    0x00007f00

/*----------------------------------------------------------------------*/
/*-- Kernel errors                                                    --*/
/*----------------------------------------------------------------------*/

#define EK_Timeout     (EC_Recover+SS_Kernel+EG_Timeout+EO_Message)

/*----------------------------------------------------------------------*/
/*-- Processor Manager Errors                                         --*/
/*----------------------------------------------------------------------*/

#define EP_Unknown     (EC_Error+SS_ProcMan+EG_Unknown)
#define EP_BadFunction (EC_Error+SS_ProcMan+EG_FnCode)
#define EP_Protected   (EC_Error+SS_ProcMan+EG_Protected)
#define EP_Name                (EC_Error+SS_ProcMan+EG_Name)

#endif

/*End of codes.h	    		   				*/

