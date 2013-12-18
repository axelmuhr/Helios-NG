/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- userdb.c								--
--                                                                      --
--	Additional module of the Session Manager			--
--                                                                      --
--	Author:  BLV 8/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/userdb.c,v 1.5 1993/08/11 10:56:06 bart Exp $*/

/**
*** BLV - most of this is not implemented yet
**/

/**
*** The Session Manager directory /sm contains a file /sm/UserDatabase.
*** This database controls all users, it contains stuff like passwords,
*** home directories with suitable capabilities, and so on. It is used
*** for the following operations:
*** 1) the Posix routines getpwnam() etc. access this file if the Session
***    Manager is running (I am not sure what they do if the Session Manager
***    is not running)
*** 2) when a new session is about to be created this module is consulted
***    to verify the password
*** 3) when a user wants to change a password the database must be changed
*** 4) when the system administrator adds a new user the database must
***    be changed.
***
*** For now I allow any program to access the file /sm/UserDatabase.
*** This results in routine OpenUserDatabse() being called, so this module
*** has full control over the protocols to be used. There are three other
*** entry points, InitUserDatabase() to initialise the ObjNode UserDatabase,
*** which is then added to the /sm directory, VerifyPassword()
*** to validate a user's password, and FillInSessionNode() to give
*** the required details of a SessionNode such as the CurrentDirectory.
**/

#include <stdio.h>
#include <syslib.h>
#include <servlib.h>
#include <sem.h>
#include <codes.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <posix.h>
#include <ctype.h>
#include <nonansi.h>
#include <attrib.h>
#include <pwd.h>
#include <signal.h>
#include "exports.h"
#include "private.h"
#include "netutils.h"
#include "rmlib.h"
#include "session.h"
#include "sessaux.h"

#if 0
/*----------------------------------------------------------------------------*/
/**
*** Variables : UserDatabase is added to the /sm directory by module session.c
*** if initialisation succeeds. 
*** LibraryLock is a semaphore for the whole of the Session Manager to restrict
*** access to the C and Posix libraries.
*** The PasswordChecking flag is set during startup by consulting the nsrc
*** file.
**/
ObjNode		UserDatabase;

/*----------------------------------------------------------------------------*/
/**
*** InitUserDatabase(), this is called when the Session Manager starts up.
*** It is responsible for initialising the ObjNode UserDatabase, which is
*** added to the /sm directory by module session.c
*** In addition this routine is probably going to be responsible for
*** reading in the real user database off disk.
**/
bool InitUserDatabase(void)
{
						/* rw:r:r:r */
  InitNode(&UserDatabase, "UserDatabase", Type_File, 
  		(PasswordChecking) ? RmFlags_PasswordChecking : 0,
  		 0x03010101);
  return(TRUE);
}

/*----------------------------------------------------------------------------*/
void OpenUserDatabase(ServInfo *servinfo, MCB *m, Port ReqPort)
{ IOdebug("in OpenUserDatabase");
}
#endif

/*----------------------------------------------------------------------------*/
/**
*** VerifyPassword : currently implemented in terms of getpwnam().
*** The name must always be valid, i.e. correspond to something in the
*** password file. Passwords are only required in certain circumstances.
**/
bool VerifyPassword(char *username, char *password)
{ struct passwd *pwd_entry;
  bool		result = FALSE;
  char		encoded[Passwd_Max];

  EncodePassword(password, encoded);

  Wait(&LibraryLock);

  pwd_entry = getpwnam(username);
  if (pwd_entry eq Null(struct passwd)) goto done;

  if (!PasswordChecking)  
   result = TRUE;
  elif (pwd_entry->pw_passwd eq Null(char))
   result = TRUE;
  elif (pwd_entry->pw_passwd[0] eq '\0')
   result = TRUE;
  elif (password eq Null(char))
   goto done;
  elif (!strcmp(encoded, pwd_entry->pw_passwd))
   result = TRUE;
   
done:   
  Signal(&LibraryLock);
  return(result);
}

/*---------------------------------------------------------------------------*/
/**
*** Given a user name that is known to be valid, fill in bits of the
*** corresponding SessionNode structure. In particular, the following fields
*** must be filled in (see session.h for the structure) :
*** 1) user name
*** 2) user uid		yes, I know it is a waste of time
*** 3) user gid		ditto
*** 4) comment		upto NameMax characters
*** 5) CurrentDirectory	Object structure, complete with capability
*** 6) Program		Object structure, complete with capability
***
*** The current implementation again uses getpwnam(), and this will have
*** to be changed to use the real user database.
**/

bool	FillInSessionNode(char *username, SessionNode *node)
{ struct passwd	*pwd_entry;
  bool	 result = FALSE;
  
  Wait(&LibraryLock);
  pwd_entry = getpwnam(username);
  if (pwd_entry eq Null(struct passwd)) goto done;

  strncpy(node->UserName, username, NameMax-1);
  node->UserName[NameMax-1] = '\0';

  strncpy(node->Comment, pwd_entry->pw_gecos, NameMax - 1);
  node->Comment[NameMax - 1] = '\0';

  node->ObjNode.Account = pwd_entry->pw_uid;
  node->Uid = pwd_entry->pw_uid;
  node->Gid = pwd_entry->pw_gid;

  node->CurrentDirectory = Locate(Null(Object), pwd_entry->pw_dir);
  if (node->CurrentDirectory eq Null(Object)) goto done;
  node->Program = Locate(Null(Object), pwd_entry->pw_shell);
  if (node->Program eq Null(Object)) goto done;

  result = TRUE;
  
done:
  Signal(&LibraryLock);
  return(result);
}

