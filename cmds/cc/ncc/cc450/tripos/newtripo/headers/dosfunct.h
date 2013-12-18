#ifndef dosfunctions
#define dosfunctions 1

#ifndef dosstructures
   #include "dosstructures.h"
#endif

/*----------------------------------------------------------------------*/
/*                                                                      */
/* Part 2. D.O.S. Functions.                                            */
/*                                                                      */
/*   This section describes the functions provided by the TRIPOS        */
/* Disk Operating System. Each function is arranged alphabetically      */
/* under the following headings:                                        */
/* -   File Handling,                                                   */
/* -   Process Handling, and                                            */
/* -   Loading Code.                                                    */
/*                                                                      */

/*----------------------------------------------------------------------*/
/* File Handling.                                                       */

/* Close.                                                               */
/*   Close a file.                                                      */
extern void Close(FileHandle *);

/* CreateDir.                                                           */
/*   Create a new directory.                                            */
extern FileLock *CreateDir(const char *);

/* CurrentDir.                                                          */
/*   Change the current directory to that associated with a lock.       */
extern FileLock *CurrentDir(FileLock *);

/* DeleteFile.                                                          */
/*   Delete a file or directory.                                        */
extern BOOL DeleteFile(const char *);

/* DupLock.                                                             */
/*   Duplicate a lock.                                                  */
extern FileLock *DupLock(FileLock *);

/* Examine.                                                             */
/*   Obtain the FileInfoBlock data associated with a lock.              */
extern BOOL Examine(FileLock *, FileInfoBlock *);

/* ExNext.                                                              */
/*   Examine the next entry in a directory.                             */
extern BOOL ExNext(FileLock *, FileInfoBlock *);

/* Info.                                                                */
/*   Obtain the Info_Data details for a disk.                           */
extern BOOL Info(FileLock *, InfoData *);

/* Input.                                                               */
/*   Obtain the address of the file handle for the standard input       */
/* to this task. (Provided anyway by global variable stdin.)            */
extern FileHandle *Input(NOARGS);

/* IoErr. (IOERR?)                                                      */
/*   Obtain more information on an I/O error or a secondary             */
/* result from the DeviceProc function.                                 */
extern LONG IoErr(NOARGS);

/* IsInteractive.                                                       */
/*   Indicate whether a file is connected to a virtual terminal         */
/* or not.                                                              */
extern BOOL IsInteractive(FileHandle *);

/* Lock.                                                                */
/*   Lock a directory or file ?What about a disc or a device?           */
extern FileLock *Lock(const char *, LONG);

/* Open.                                                                */
/*   Open a file.                                                       */
extern FileHandle *Open(const char *, LONG);

/* Output.                                                              */
/*   Identify the file handle for the process's standard output         */
/* (already in global variable stdout).                                 */
extern FileHandle *Output(NOARGS);

/* ParentDir.                                                           */
/*   Obtain the lock for the parent of a directory or file.             */
extern FileLock *ParentDir(FileLock *);

/* Read.                                                                */
/*   Read bytes of data from a file.                                    */
extern LONG Read(FileHandle *, BYTE *, LONG);

/* Rename.                                                              */
/*   Re-name a directory or file.                                       */
extern BOOL Rename(const char *, const char *);

/* Seek.                                                                */
/*   Move to a logical position in a file.                              */
extern LONG Seek(FileHandle *, LONG, LONG);

/* SetComment.                                                          */
/*   Apply a comment (file note) to a file or directory.                */
extern BOOL SetComment(const char *, const char *);

/* SetProtection.                                                       */
/*   Set the protection attributes of a file.                           */
extern BOOL SetProtection(const char *, ULONG);

/* UnLock.                                                              */
/*   Remove a lock.                                                     */
extern void UnLock(FileLock *);

/* WaitForChar.                                                         */
/*   Indicate whether character input is received within a time         */
/* limit.                                                               */
extern BOOL WaitForChar(FileHandle *, LONG);

/* Write.                                                               */
/*   Write bytes of data to a file.                                     */
extern LONG Write(FileHandle *, BYTE *, LONG);

/*----------------------------------------------------------------------*/
/* Process Handling.                                                    */

/* CreateProc.                                                          */
/*   Create a new process.                                              */
extern LONG CreateProc(char *, LONG, LONG *, LONG);

/* DateStamp.                                                           */
/*   Obtain date and time.                                              */
extern DateVec *DateStamp(DateVec *);

/* Delay.                                                               */
/*   Delay the process for a given time.                                */
extern void Delay(LONG);

/* DeviceProc.                                                          */
/*   Obtain the process identifier for a device.                        */
extern LONG DeviceProc(char *);

/* Exit.                                                                */
/*   Exit from a program.                                               */
extern void Exit(ULONG);

/*----------------------------------------------------------------------*/
/* Loading Code.                                                        */

/* Execute.                                                             */
/*   Execute a CLI command.                                             */
extern BOOL Execute(char *, FileHandle *, FileHandle *);

/* LoadSeg.                                                             */
/*   Load a load module into memory.                                    */
extern LONG *LoadSeg(char *);

/* UnLoadSeg.                                                           */
/*   Unload a load module previously loaded by LoadSeg.                 */
extern void UnLoadSeg(LONG *);

/*----------------------------------------------------------------------*/
/* Miscellaneous.                                                       */

/* VDU.                                                                 */
/*   Full screen support                                                */
extern LONG VDU( LONG, LONG, LONG *, LONG * );

/*----------------------------------------------------------------------*/
#endif
