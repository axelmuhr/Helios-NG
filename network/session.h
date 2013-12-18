/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- session.h								--
--                                                                      --
--	Header file defining the interface to the Session Manager	--
--                                                                      --
--	Author:  BLV 9/7/90						--
--                                                                      --
------------------------------------------------------------------------*/

/* $Header: /hsrc/network/RCS/session.h,v 1.5 1992/10/11 13:33:21 bart Exp $ */

#ifndef __session_h
#define __session_h

#ifndef __helios_h
#include <helios.h>
#endif

#ifndef __protect_h
#include <protect.h>
#endif

#ifndef __gsp_h
#include <gsp.h>
#endif

#ifndef __servlib_h
#include <servlib.h>
#endif

/**
*** This structure is used as the Info vector for RmRegisterWindow().
*** This is implemented as:
*** Create("/sm/Windows", "pc.window.console", Type_Device, , WindowInfo)
**/
typedef	struct	RmWindowInfo {
	RPTR		UserName;	/* Default user name or -1	*/
	WORD		Flags;		/* in window Stream structure	*/
	WORD		Pos;		/* ditto			*/
	Capability	WindowCap;	/* ditto			*/
	RPTR		WindowName;	/* from Stream structure	*/
		/* It is necessary to inherit this from the environment */
	Capability	WindowServerCap;/* so that it can passed on to	*/
	RPTR		WindowServerName;/* the tfm			*/
} RmWindowInfo;

/**
*** This structure can be read back from the Session Manager to find out
*** which window a user used to log in.
**/
typedef struct	RmLoginWindow {
	Capability	Cap;
	char		WindowName[1];
} RmLoginWindow;

/**
*** This structure is passed to the Session Manager when attempting to
*** create a new session. Essentially the call is:
*** Create("/sm", <user>, Type_Session, , Info)
*** Obviously the Session Manager needs to know the user name, and a password
*** for validation. The password sent in the message is not encrypted,
*** otherwise any user can extract the encrypted password from the password
*** file and use. Some public key encryption will be required in future.
*** The net result is that any program which intercepts messages or link
*** traffic could get hold of unencrypted passwords.
*** Less obviously the Session Manager
*** needs to have details of the login window so that other users can access
*** it.
**/
typedef struct RmLoginInfo {
	char		UserName[NameMax];
	char		Password[NameMax];
	Capability	Cap;	/* For the window */
	char		WindowName[1];
} RmLoginInfo;

/**
*** A successful session Create() returns an object which can be
*** Open()'ed and read to give details of the session. These details
*** are read into a Malloc()'ed buffer, and the appropriate bits
*** are extracted. N.B. some of the information like User ID is
*** totally redundant but is sent anyway.
**/

typedef struct	RmSessionInfo {
	char		UserName[NameMax];
	int		Uid;
	int		Gid;
	char		Comment[NameMax];
	Object		*TFM;	
	Object		*CurrentDirectory;
	Object		*Program;
} RmSessionInfo;

/**
*** The actual data sent for the above structure is shown below. Note that
*** a capability is sent even for the default program. This allows
*** special user id's such as powerdown which can invoke programs not
*** generally accessible.
**/
typedef struct RmSessionAux {
	char		UserName[NameMax];
	int		Uid;
	int		Gid;
	char		Comment[NameMax];
	Capability	TFMCap;
	RPTR		TFMName;
	Capability	CurrentDirCap;
	RPTR		CurrentDirName;
	Capability	ProgramCap;
	RPTR		ProgramName;
} RmSessionAux;

	/* Flag set for the  /sm/UserDatabase object */
#define RmFlags_PasswordChecking	0x10000000

extern bool	RmTestSessionManager(void);
extern Object	*RmGetSessionManager(void);
extern bool	RmTestPasswordsRequired(void);
extern bool	RmRegisterWindow(Object *, Stream *, char *, WORD *);
extern Object	*RmCreateSession(char *username, char *password, 
				 Stream *, WORD *);
extern bool	RmGetCapabilities(Object *, RmSessionInfo *, WORD *);
extern bool	RmTestInteractiveSession(Object *Session);
extern Stream	*RmGetWindow(Object *Session, WORD *error);
extern bool	RmGetNames(char *UserName, char *SessionName);
extern Object	*RmGetTfm(void);
extern void	RmAbortSession(Object *Session);

#endif
