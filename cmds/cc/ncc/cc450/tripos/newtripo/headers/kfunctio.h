/* Include file for pre-declaration of Tripos functions.      */
#ifndef kfunctions
#define kfunctions 1

#ifndef kstructures
   #include "kstructures.h"
#endif

/*----------------------------------------------------------------------*/
/*                                                                      */
/* Part 1. Kernel Functions.                                            */
/*                                                                      */
/*   This section describes the functions provided by the TRIPOS        */
/* kernel. Each function is arranged alphabetically under the           */
/* following headings:                                                  */
/* -   Memory Management,                                               */
/* -   Task Management,                                                 */
/* -   Device Management,                                               */
/* -   Message Passing, and                                             */
/* -   Miscellaneous.                                                   */
/*                                                                      */
/*----------------------------------------------------------------------*/
/* Memory Management.                                                   */

/* FreeMem.                                                             */
/*   Free memory previously acquired by GetMem.                         */
extern void FreeMem(LONG *);

/* GetMem.                                                              */
/*   Allocate memory from the system.                                   */
extern LONG *GetMem(LONG);

/*----------------------------------------------------------------------*/
/* Task Management.                                                     */

/* AddTask.                                                             */
/*   Allocate a task control structure from free memory and             */
/* initialise it. Use CreateProc below instead.                         */
extern LONG AddTask(LONG **, LONG, LONG, char *);

/* ChangePri.                                                           */
/*   Change the priority of a task.                                     */
extern BOOL ChangePri(LONG, LONG);

/* Forbid.                                                              */
/*   Stop any other task from running until a Permit is performed.      */
extern void Forbid(NOARGS);

/* Hold.                                                                */
/*   Prevent a task from running until a Release is performed           */
/* on that task.                                                        */
extern BOOL Hold(LONG);

/* Permit.                                                              */
/*   Undo the effect of Forbid allowing other tasks to run.             */
extern void Permit(NOARGS);

/* Release.                                                             */
/*   Undo the effect of a Hold on a task, allowing it to run.           */
extern BOOL Release(LONG);

/* RemTask.                                                             */
/*   Remove a task from the system.                                     */
extern BOOL RemTask(LONG);

/* SetFlags.                                                            */
/*   Set the attention flags for a task.                                */
extern BOOL SetFlags(LONG, ULONG);

/* SuperMode.                                                           */
/*   Enter supervisor mode.                                             */
extern void SuperMode(NOARGS);

/* TestFlags.                                                           */
/*   Test the attention flag word for this task.                        */
extern BOOL TestFlags(ULONG);

/* UserMode. (in manual, usermode).                                     */
/*   Exit from supervisor mode to user mode.                            */
extern void UserMode(NOARGS);

/*----------------------------------------------------------------------*/
/* Device Management.                                                   */

/* AddDevice.                                                           */
/*   Add a new device to the system.                                    */
extern LONG AddDevice(LONG *);

/* RemDevice.                                                           */
/*   Remove a device from the system.                                    */
extern void RemDevice(LONG);

/*----------------------------------------------------------------------*/
/* Message Passing.                                                     */

/* DQPkt.                                                               */
/*   Reclaim a packet from the work queue of another task.              */
extern BOOL DQPkt(Packet *, LONG);

/* FindTask.                                                            */
/*   Return the identity of the current task.                           */
extern TCB *FindTask(NOARGS);

/* QPkt.                                                                */
/*   Send a packet.                                                     */
extern BOOL QPkt(Packet *, LONG);

/* TaskWait.                                                            */
/*   Wait for the next packet to arrive.                                */
extern Packet *TaskWait(NOARGS);

/* TestWkQ.                                                             */
/*   Test if there is anything on the work queue.                       */
extern BOOL TestWkQ(TCB *);

/*----------------------------------------------------------------------*/
/* Miscellaneous.                                                       */

/* FindDOS.                                                             */
/*   Return the DOS library base pointer.                               */
extern LONG *FindDOS(NOARGS);

/* RootStruct                                                           */
/* Return the address of the RootNode                                   */
extern APTR RootStruct(NOARGS);
#endif
