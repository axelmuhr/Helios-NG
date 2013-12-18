/**
*** This file describes various changes needed to Helios, particularly
*** the header files.
***
*** Module table slots: the Resource Management library uses slot 24,
*** previously used by the Network Control library. Slot 23, previously
*** used by the support library, is now free again.
***
*** $Header: /hsrc/network/RCS/exports.h,v 1.3 1991/03/01 17:20:22 bart Exp $
***
**/

/**
*** codes.h
***
*** The subsystem SS_Login is redundant. SS_Batch is not needed for 1.2,
*** but will be reused for 1.3
***
*** I would like a new EG, EG_RmLib, for returning Resource Management
*** library errors.
***
*** Also new error objects, EO_Password etc.
***
*** All Charlie's private protocol codes for use with the TFM and
*** Network Server are redundant as far as I am concerned. I assume that
*** the nucleus does not use them.
***
*** FG_SendEnv
*** FG_Signal
*** FG_ProgramInfo
*** FG_RequestEnv
***
*** FG_NetMask
*** FG_NetStatus
*** FG_NetReq
***
*** TF_TMASK
**/
#define EG_RmLib	0x00160000

#define EO_Taskforce	EO_TaskForce

/* reboot - like FG_Terminate */
#define FG_Reboot	0x00002FF0

/**
*** Syslib.h
**/


/**
*** Servlib.h
***
*** probably unchanged
**/

/**
*** GSP.h
***
*** There is a bug in the server library, which only recognises
*** Type_Directory and not supersets of this type. Hence Type_Network
*** and Type_Taskforce have to be hacked for now
**/

/**
*** I prefer an alternative spelling for Type_Taskforce
**/
#ifdef Type_TaskForce
#undef Type_TaskForce
#endif

#if 0
#define Type_Taskforce  (0x80 | Type_Directory)
#else
#define Type_Taskforce (Type_Directory)
#endif

/**
*** There is a new header file, RmLib.h, currently exported from this
*** directory
**/

/**
*** Link.h, I use the following additional state
**/
#define Link_State_NotConnected	0

/**
*** Module.h, unfortunately Charlie used a different magic number for
*** taskforces to distinguish them from ordinary programs. I have to
*** stick to that in order to maintain binary compatibility for
*** CDL binaries.
**/
#define Taskforce_Magic	0x12345677L
