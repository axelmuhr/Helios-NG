/* $Header: hermv.h,v 1.1 90/12/21 20:53:07 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/hermv.h,v $ */

/*------------------------------------------------------------------------*/
/*                                                 hdr/microlink/hermv.h  */
/*------------------------------------------------------------------------*/

/* This is the header file which should be accessed by clients to the     */
/*   microlink server which need to access the 'hermv' virtual device.    */
/*   In here is defined the structure of the hermes version number        */
/*   structure returned by a call to GetInfo() on an open stream to the   */
/*   'hermv' object.                                                      */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkHermv_h
#define MicrolinkHermv_h

/*------------------------------------------------------------------------*/
/*                                               The HermvInfo structure  */
/*------------------------------------------------------------------------*/

/* This structure is returned by calls to GetInfo() on the '/microlink/   */
/*   hermv' object.                                                       */

typedef struct HermvInfo
{  int min;   /* Hermes minor version number                              */
   int maj;   /* Hermes major version number                              */
} HermvInfo;

/*------------------------------------------------------------------------*/
/*                                                      End Of Interlock  */
/*------------------------------------------------------------------------*/

#endif
