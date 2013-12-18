/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- who.c								--
--                                                                      --
--	display all users currently logged in				--
--	This command is not X/OPEN compliant. The X/Open version	--
--	corresponds to that running on the HP machines and can get	--
--	quite complicated. It includes options such as showing the	--
--	run-level of init and the last time the clock was changed.	--
--                                                                      --
--	Author:  BLV 13/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/who.c,v 1.5 1994/03/10 17:14:00 nickc Exp $*/

/**
*** BLV - somebody should make this command X/Open compliant.
**/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <queue.h>
#include "session.h"
#include "netutils.h"
#include "exports.h"

#define eq ==
#define ne !=

typedef struct User {
	Node	Node;
	WORD	Date;
	char	Name[1];
} User;

static void usage(void);
static void do_whoami(void);
static word free_node(Node *);
static word ReadDir(Object *sm, List *);
static void SortList(List *);
static void ShowUser(User *);

int main(int argc, char **argv)
{ Object	*SessionManager = Null(Object);
  int		rc = EXIT_FAILURE;
  List		UserList;
  
  if (argc eq 3)
   { if ( (!strcmp(argv[1], "am")) &&
          ( (!strcmp(argv[2], "i")) || (!strcmp(argv[2], "I")) ) )
      { do_whoami(); return(EXIT_SUCCESS); }
   }

  if (argc ne 1) usage();   
     
  InitList(&UserList);

  SessionManager = RmGetSessionManager();
  if (SessionManager eq Null(Object))
   { fprintf(stderr, "who : failed to locate session manager.\n");
     goto done;
   }

  rc = ReadDir(SessionManager, &UserList);
  if (rc ne EXIT_FAILURE)
   { WalkList(&UserList, (WordFnPtr) &ShowUser);
     putchar('\n');
   }
      
done:
  if (SessionManager ne Null(Object)) Close(SessionManager);

  WalkList(&UserList, &free_node);	/* no FreeList in kernel */
				       /* (kernel does not know about free() )*/

  return(rc);
}

static void usage(void)
{ fprintf(stderr, "who : usage, who\n");
  exit(EXIT_FAILURE);
}

static word free_node(Node *node)
{ Remove(node);
  free(node);
  return(0);
}

/**
BLV currently I assume that user names can be up to 31 characters,
BLV NameMax - 1, which means that the output takes up quite a bit of
BLV space. Perhaps the command should assume a different width.
**/
static void ShowUser(User *user)
{ char		*timebuf;
  time_t	now = user->Date;

  timebuf = ctime(&now);
  if (timebuf eq Null(char)) timebuf = "                          ";
  printf("%-31.31s %.12s\n", user->Name, &(timebuf[4]));
}

static word ReadDir(Object *SessionManager, List *Users)
{ Stream	*SM = Null(Stream);
  int		Size;
  int		rc = EXIT_SUCCESS;
  DirEntry	*Entries = Null(DirEntry);
  DirEntry	*Current;
  int		NumberUsers = 0;
  
  SM = Open(SessionManager, Null(char), O_ReadOnly);
  if (SM eq Null(Stream))
   { fprintf(stderr, "who : failed to open Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }

  Size = GetFileSize(SM);
  if (Size <= 0)
   { fprintf(stderr, "who : error accessing Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }

  Entries = (DirEntry *) malloc(Size);
  if (Entries eq Null(DirEntry))
   { fprintf(stderr, "who : out of memory when examing user sessions.\n");
     rc = EXIT_FAILURE;
     goto done;
   }
   
  if (Read(SM, (BYTE *) Entries, Size, -1) < Size)
   { fprintf(stderr, "who : error reading Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }
   
  for (Current = Entries; Size > 0; Current++, Size -= sizeof(DirEntry))
   { User	*ThisUser;
     ObjInfo	Info;
     int	rc;
          
     if (!strcmp(Current->Name, ".")) continue;
     if (!strcmp(Current->Name, "..")) continue;
     if (Current->Type ne Type_Session) continue;
    
     if ((rc = ObjectInfo(SessionManager, Current->Name, (BYTE *) &Info))
          < Err_Null)
      { fprintf(stderr, "who : error accessing session %s, fault 0x%08x\n",
      		Current->Name, rc);
      	continue;
      }

     ThisUser = (User *) malloc(sizeof(User) + strlen(Current->Name));
     if (ThisUser eq  Null(User))
      { fprintf(stderr, "who : out of memory when building list of users.\n");
        return(EXIT_FAILURE);
      }
     strcpy(ThisUser->Name, Current->Name);
     ThisUser->Date = Info.Dates.Creation;
#ifdef SYSDEB
     ThisUser->Node.Next = ThisUser->Node.Prev = &ThisUser->Node;
#endif
     AddTail(Users, &(ThisUser->Node));
     NumberUsers++;
   }

  free(Entries); Entries = Null(DirEntry);
  Close(SM); SM = Null(Stream);
  
  if (NumberUsers eq 0)
   { fprintf(stderr, "who : nobody is logged in.\n");
     return(EXIT_SUCCESS);
   }

  SortList(Users);
     
done:
  if (SM ne Null(Stream)) Close(SM);
  if (Entries ne Null(DirEntry)) free(Entries);
  return(rc);
}

/**
*** Something vaguely resembling bubble sort. Very inefficient but
*** the number of users will be small.
**/
static void SortList(List *Users)
{ int		changes = 1;
  User		*Current, *Next;

  until (changes eq 0)
   { Current = (User *) Users->Head;
     Next    = (User *) Current->Node.Next;
     changes = 0;
     
     until (Next->Node.Next eq Null(Node))
      { 
        if (strcmp(Current->Name, Next->Name) > 0)
         { Remove(&(Current->Node));
           PostInsert(&(Next->Node), &(Current->Node));
           changes = 1;
           Next    = (User *) Current->Node.Next;
         }
        else
         { Current = Next;
           Next    = (User *) Current->Node.Next;
         }
      }
   }
}

/**
*** Code from whoami.c
**/

static void use_environment(void);

static void do_whoami(void)
{ char		SessionName[NameMax];
  char		UserName[NameMax];

  unless(RmTestSessionManager())
   { fprintf(stderr, "who : warning, failed to find Session Manager\n");
     use_environment();
     exit(EXIT_SUCCESS);
   }

  unless(RmGetNames(UserName, SessionName))
   { fprintf(stderr, "who : warning, failed to validate names.\n");
     use_environment();
     exit(EXIT_SUCCESS);
   }

	/* Both are known so I can compare the two values. If they	*/
	/* are the same only the user name is printed. Otherwise the	*/
	/* user name is printed followed by the session name in brackets*/
  if (!strcmp(UserName, SessionName))
   puts(UserName);
  else
   printf("%s (%s)\n", UserName, SessionName);
  exit(EXIT_SUCCESS);
}

static void use_environment(void)
{ char *SessionName	= getenv("SESSION");
  char *UserName	= getenv("USER");
  
	/* If neither the Session name nor the user name is known then	*/
	/* I am stuck.							*/  
  if ((SessionName eq Null(char)) && (UserName eq Null(char)))
   { fprintf(stderr, "who : I do not know who you are.\n");
     exit(EXIT_FAILURE);
   }

	/* If only the Session Name or only the user name is known then */
	/* that entry is printed.					*/
  if (SessionName eq Null(char))
   { printf("who : name is probably %s\n", UserName); 
     return;
   }

  if (UserName eq Null(char))
   { printf("who : name is probably %s\n",SessionName);
     return;
   }

	/* If both are known then I can compare the two values. If they */
	/* are the same only the user name is printed. Otherwise the	*/
	/* user name is printed followed by the session name in brackets*/
  if (!strcmp(UserName, SessionName))
   printf("who : name is probably %s\n", UserName);
  else
   printf(
        "who : user name is probably %s, and session name is probably %s\n",
          UserName, SessionName);
  return;
}


