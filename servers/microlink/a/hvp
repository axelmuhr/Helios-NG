/* $Header: hermv.h,v 1.2 91/01/09 12:30:34 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/private/hermv.h,v $ */

/*------------------------------------------------------------------------*/
/*                                             microlink/private/hermv.h  */
/*------------------------------------------------------------------------*/

/* This header file contains definitions which are local and specific to  */
/*   the 'hermv' part of the microlink server. These structures and       */
/*   macros and so-forth need not be accessed by any other modules.       */
/* The structures (for GetInfo() and SetInfo()) required by clients of    */
/*   the 'hermv' server are stored in "microlink/hermv.h".                */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

#ifndef MicrolinkPrivateHermv_h
#define MicrolinkPrivateHermv_h

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

# include "microlink/hermv.h"

/*------------------------------------------------------------------------*/
/*                          Microlink messages wrt hermes version number  */
/*------------------------------------------------------------------------*/

/* The following macro creates the short message byte for the message     */
/*   to bew sent over the microlink to the microcontroller to request the */
/*   version number of the microcontroller software. A three bit tag may  */
/*   be sent with the message which gets echoed in the reply.             */

# define ASQhermv(tag) (0x08|(tag))

/* Hermes software verion reply. The Microcontroller transmits it's       */
/*   software version in this message in response to an ASQhermv message  */
/*   from Hercules. The microcontroller software version number consists  */
/*   of a major and minor version number. An increment in the minor       */
/*   version number indicates a change in the microcontroller binary due  */
/*   to debugging, speeding up, or whatever it is. An increment in the    */
/*   major version number involves an extension of the protocol.          */
/* Also returned is the tag sent in the request message.                  */

#define MLYhermv         0x85
#define MLYhermvMin(buf) ((buf)[1])
#define MLYhermvMaj(buf) ((buf)[2]&0x1F)
#define MLYhermvTag(buf) ((buf)[3]>>5)

/*------------------------------------------------------------------------*/
/*                                         Hermes Version Number context  */
/*------------------------------------------------------------------------*/

/* This following structure defines a context structure, one of which is  */
/*   created for each open stream to the 'hermv' element of the           */
/*   microlink directory.                                                 */
/* This is used in the source file 'source/hermv.c'                       */

typedef struct HermvContext
{  HermvInfo info;
   char      txt[100];
} HermvContext;

/*------------------------------------------------------------------------*/
/*                                                      End Of Interlock  */
/*------------------------------------------------------------------------*/

#endif
