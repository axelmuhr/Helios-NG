/* $Header: microlink.h,v 1.4 91/01/31 13:50:31 charles Locked $ */
/* $Source: /server/usr/users/b/charles/world/microlink/RCS/ext/private/microlink.h,v $ */

/*------------------------------------------------------------------------*/
/*                                           micrlink/source/microlink.h  */
/*------------------------------------------------------------------------*/

/* This is the header file for the various microlink server modules.      */

/*------------------------------------------------------------------------*/
/*                                                             Interlock  */
/*------------------------------------------------------------------------*/

# ifndef microlinkMicrolink_h
# define microlinkMicrolink_h

/*------------------------------------------------------------------------*/
/*                                         Overriding 'TRUE' and 'FALSE'  */
/*------------------------------------------------------------------------*/

/* In Helios-ARM the 'TRUE' and 'FALSE' macros define long constants but  */
/*   'word' is typedef'd as int. The following definitions overrid the    */
/*   definitions of TRUE and FALSE in the helios headers in order to      */
/*   compenstate for this.                                                */

#define TRUE  ((word)1)
#define FALSE ((word)0)

/*------------------------------------------------------------------------*/
/*                                                          Header Files  */
/*------------------------------------------------------------------------*/

#include <helios.h>
#include <string.h>
#include <codes.h>
#include <stddef.h>
#include <stdlib.h>
#include <syslib.h>
#include <servlib.h>
#include <task.h>
#include <message.h>
#include <attrib.h>
#include <stdio.h>
#include <process.h>
#include <ioevents.h>
#include <signal.h>
#include <abcARM/ABClib.h>

/*------------------------------------------------------------------------*/
/*                                        Diagnostics printing semaphore  */
/*------------------------------------------------------------------------*/

/* Interlocks calls to printf and the like for diagnostics: */
extern Semaphore diag;
#define diag(m) { Wait(&diag); printf m ; Signal(&diag); }
extern int dgDiag;

/*------------------------------------------------------------------------*/
/*                                                    Request stack size  */
/*------------------------------------------------------------------------*/

/* The following macro defines the size of stack to allocate to any       */
/*    particular request server.                                          */

# define ReqStackSize (16*1024)

/*------------------------------------------------------------------------*/
/*                                                  MaxMachineNameLength  */
/*------------------------------------------------------------------------*/

/* The following macro defines the maximum length that is expected in the */
/*    machine name read from the operating system.                        */

# define MaxMachineNameLength 1024

/*------------------------------------------------------------------------*/
/*                                      Enumeration for various services  */
/*------------------------------------------------------------------------*/

/* The following codes the various services supplid in the microlink      */
/*    server directory.                                                   */

#define MlkDigitiser    1
#define MlkHermv        2
#define MlkGeneral      3
#define MlkRawDig       4

/*------------------------------------------------------------------------*/
/*                                      Nodes in the microlink directory  */
/*------------------------------------------------------------------------*/

/* Inserted into the microlink directory is a 'node' for each service     */
/*    provided by the microlink, as enumerated above. The beggining of    */
/*    these nodes must have exactly the same layout as an 'ObjNode'       */
/*    structrue as defined in '/helios/inlcude/servlib.h', this is to     */
/*    allow the server library funcitons to work properly. After that     */
/*    there may be contextual information about the object itself. In     */
/*    this case, this is simply the enumeration value indicating which    */
/*    type of service the object denotes.                                 */

typedef struct MicrolinkNode
{  ObjNode               obj;
   int                objTyp;
} MicrolinkNode;

/*------------------------------------------------------------------------*/
/*                                                  Intallation routines  */
/*------------------------------------------------------------------------*/

/* The following routines are resposible for intaslling the respective    */
/*    microlink protocol servers into the microlink server directory.     */

void mlkAddDigitiserEntry  (DirNode *microlinkDir);
void mlkAddHermvEntry      (DirNode *microlinkDir);
void mlkAddGeneralEntry    (DirNode *microlinkDir);
void mlkAddRawDigEntry     (DirNode *microlinkDir);

/*------------------------------------------------------------------------*/
/*                                                Microlink node servers  */
/*------------------------------------------------------------------------*/

/* The following routines service the requests that occur for opened      */
/*    streams to the respective microlink servers.                        */

void mlkServeDigitiser ( ServInfo *si, MicrolinkNode *nde, MsgBuf *rply );
void mlkServeHermv     ( ServInfo *si, MicrolinkNode *nde, MsgBuf *rply );
void mlkServeGeneral   ( ServInfo *si, MicrolinkNode *nde, MsgBuf *rply );
void mlkServeRawDig    ( ServInfo *si, MicrolinkNode *nde, MsgBuf *rply );

/*------------------------------------------------------------------------*/
/*                               Sending requests to the microcontroller  */
/*------------------------------------------------------------------------*/

/* Microlink request/receive transactions are interlocked in this system, */
/*    and are all routed through the following function. The funcion      */
/*    accepts the context structure which contains pointers to the        */
/*    and transmit buffers (NULL if the corresponding part of the message */
/*    should not occur), and also a semaphore. The sempahore is           */
/*    initialised by the funciton and it then forkes a process to deal    */
/*    with the transmission and returns immediately. Thus the user of     */
/*    this function simply calls it then waits on the semaphore for the   */
/*    receive process to finish. As can be seen, a sempahore is declared  */
/*    below which is used by the request/receive function to interlock    */
/*    request/receive processes over the microlink.                       */
/* On calling the below funciton, the first byte of the receive buffer    */
/*    should contain the code for the type of message expected in reply.  */

typedef struct MlkLinkRequestControl
{  ubyte     *rxBuf;             /* Receive  buffer (or NULL)             */
   ubyte     *txBuf;             /* Transmit buffer (or NULL)             */
   Semaphore    sem;             /* Control semaphore                     */
   int       status;             /* Result status of transfer             */
   word        code;             /* Result code (see below)               */
} MlkLinkRequestControl;

/* The '->status' member is encoded in exactly the same way as for the    */
/*   status returned in the 'general' part of the microlink server: See   */
/*   the files "ext/general.h", "ext/private/general.h" and "source/      */
/*   general.c".                                                          */

void MlkPerformLinkRequest ( MlkLinkRequestControl *ctl );

/* The following Semaphore used by the funciton to interlock requests ... */
extern Semaphore MlkLinkRequestInterlock;

/*------------------------------------------------------------------------*/
/*                                                      End Of Interlock  */
/*------------------------------------------------------------------------*/

#endif
