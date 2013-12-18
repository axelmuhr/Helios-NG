/* $Header: phone.h,v 1.3 91/01/31 13:50:25 charles Locked $ */
/* $SOurce:$ */

/*------------------------------------------------------------------------*/
/*                                                     microlink/phone.h  */
/*------------------------------------------------------------------------*/

/* This is the header file for the 'phone dialling functions. These       */
/*   funcitons currently work by openeing a stream to 'microlink/general' */
/*   which is a microlink-message-stream, and sending and receiving       */
/*   appropriate messages down this link to control the 'phone on/off     */
/*   hook status as well as dialling and ring-detection.                  */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkPhone_h
#define MicrolinkPhone_h

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdlib.h>
#include <syslib.h>

/*------------------------------------------------------------------------*/
/*                                                         Phone_Channel  */
/*------------------------------------------------------------------------*/

/* The contents of this structure need not concern the client.            */

typedef struct Phone_Channel
{  Stream *stm;
} Phone_Channel;

/*------------------------------------------------------------------------*/
/*                                                 Function declarations  */
/*------------------------------------------------------------------------*/

Phone_Channel *Phone_OpenChannel  ( char *name,word *err );

word Phone_RefreshDeadmanTimeout   ( Phone_Channel *hdl               );
word Phone_IsRinging               ( Phone_Channel *hdl               );
word Phone_IsOnHook                ( Phone_Channel *hdl               );
word Phone_IsDialling              ( Phone_Channel *hdl               );
word Phone_AwaitRing               ( Phone_Channel *hdl, word timeout );
word Phone_TakeOffHook             ( Phone_Channel *hdl               );
word Phone_PutOnHook               ( Phone_Channel *hdl               );
word Phone_SelectPulseDialling     ( Phone_Channel *hdl               );
word Phone_SelectToneDialling      ( Phone_Channel *hdl               );
word Phone_SetPabxNumber           ( Phone_Channel *hdl, char *num    );
word Phone_SelectPabx              ( Phone_Channel *hdl               );
word Phone_SelectPstn              ( Phone_Channel *hdl               );
word Phone_DialNumber              ( Phone_Channel *hdl, char *num    );
word Phone_AwaitDialComplete       ( Phone_Channel *hdl, word timeout );
word Phone_UnblockChannel          ( Phone_Channel *hdl               );
word Phone_CloseChannel            ( Phone_Channel *hdl               );

/*------------------------------------------------------------------------*/
/*                                                  Return status values  */
/*------------------------------------------------------------------------*/

/* The various status values that can be returned by the 'phone-control   */
/*   functions are as follows:                                            */

# define Phone_No                 0
# define Phone_Yes                1

# define Phone_Ok                 0
# define Phone_Dead               2
# define Phone_Timedout           3
# define Phone_UnBlocked          4
# define Phone_Failed             5
# define Phone_BadNum             6
# define Phone_RejectedNum        7

/*------------------------------------------------------------------------*/
/*                                                      End-Of-Interlock  */
/*------------------------------------------------------------------------*/

#endif


