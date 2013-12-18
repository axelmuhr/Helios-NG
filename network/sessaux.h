/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1991, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- sessaux.h								--
--                                                                      --
--	Additional odds and ends needed by the Session Manager		--
--									--
--	Author:  BLV 5/12/91						--
--                                                                      --
------------------------------------------------------------------------*/
/* $Header: /hsrc/network/RCS/sessaux.h,v 1.4 1993/08/11 10:48:46 bart Exp $ */

/**
*** Stack sizes
**/
#ifndef STACKEXTENSION
#define Monitor_Stack	2000
#define	Getty_Stack	2000
#define	SM_Stack	2000
#else
#define	Monitor_Stack	1000
#define	Getty_Stack	1000
#define	SM_Stack	1000
#endif

/**
*** A superset of ObjNode used to hold details of an active session.
**/
typedef struct SessionNode {
	ObjNode		ObjNode;
	char		UserName[NameMax];
	int		Uid;
	int		Gid;
	char		Comment[NameMax];
	Object		*Window;
	Stream		*WindowStream;
	Object		*CurrentDirectory;
	Object		*Program;
	Object		*TFM;
	Object		*TFMProgram;
	Stream		*TFMStream;
} SessionNode;

/**
*** A superset of ObjNode used to hold details of registered windows.
**/
typedef struct WindowNode {
	ObjNode		ObjNode;
	Object		*Window;
	Stream		*WindowStream;
	Object		*LoginProgram;
	Stream		*LoginStream;
	Object		*WindowServer;
	char		DefaultUser[NameMax];
} WindowNode;

/**
*** Globals for module session.c, the main module of the Session Manager
**/
	/* Restrict access to the Posix and C libraries	*/
extern Semaphore	LibraryLock;
extern bool		PasswordChecking;
extern void		report(char *format, ...);
extern void		fatal(char *format, ...);
extern void		debug(char *format, ...);
extern char		**nsrc_environ;

/**
*** Maintaining the user database is handled by a separate module
*** userdb.c. This is responsible for handling all interaction with the
*** password file.
**/
extern ObjNode	UserDatabase;
extern bool	InitUserDatabase(void);
extern void	OpenUserDatabase(ServInfo *, MCB *, Port);
extern bool	VerifyPassword(char *username, char *password);
extern bool	FillInSessionNode(char *username, SessionNode *);

/**
*** Details of the login program which needs to interact fairly closely
*** with the Session Manager.
**/
#define LoginName	"/helios/bin/login"
#define LoginArgv0	"login"
#define TfmName		"/helios/lib/tfm"

/**
*** Diagnostics options
**/
/**
*** Debugging options. N.B. the top bit must always be clear, or
*** there will be confusion between sending masks around and
*** error messages. dbg_Inquire is exempt because this is passed only
*** in the control vector.
**/
#define		dbg_Inquire		-1
#define		dbg_Redirect		-2
#define		dbg_Revert		-3
#define		dbg_Create		0x0001
#define		dbg_Monitor		0x0002
#define		dbg_Delete		0x0004
#define		dbg_Memory		0x0008
#define		dbg_IOC			0x0010

#define Debug(a, b) if (DebugOptions & a) report b

