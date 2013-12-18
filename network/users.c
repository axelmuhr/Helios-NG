/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- users.c								--
--                                                                      --
--	display all users currently logged in on a single line		--
--                                                                      --
--	Author:  BLV 13/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/users.c,v 1.5 1994/03/10 17:13:00 nickc Exp $*/


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
	char	Name[1];
} User;

static void usage(void);
static word free_node(Node *);
static word ReadDir(Object *sm, List *);
static void SortList(List *);
static void ShowUser(User *);

int main(int argc, char **argv)
{ Object	*SessionManager = Null(Object);
  int		rc = EXIT_FAILURE;
  List		UserList;
  
  if (argc ne 1) usage();
  argv = argv;		/* suppress warning */
  
  InitList(&UserList);

  SessionManager = RmGetSessionManager();
  if (SessionManager eq Null(Object))
   { fprintf(stderr, "users: failed to locate session manager.\n");
     goto done;
   }

  rc = (int) ReadDir(SessionManager, &UserList);
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
{ fprintf(stderr, "users : usage, users\n");
  exit(EXIT_FAILURE);
}

static word free_node(Node *node)
{ Remove(node);
  free(node);
  return(0);
}

static void ShowUser(User *user)
{ fputs(user->Name, stdout); putchar(' ');
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
   { fprintf(stderr, "users : failed to open Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }

  Size = (int) GetFileSize(SM);
  if (Size <= 0)
   { fprintf(stderr, "users : error accessing Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }

  Entries = (DirEntry *) malloc(Size);
  if (Entries eq Null(DirEntry))
   { fprintf(stderr, "users : out of memory when examing user sessions.\n");
     rc = EXIT_FAILURE;
     goto done;
   }
   
  if (Read(SM, (BYTE *) Entries, Size, -1) < Size)
   { fprintf(stderr, "users : error reading Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }
   
  for (Current = Entries; Size > 0; Current++, Size -= sizeof(DirEntry))
   { User	*ThisUser;

     if (!strcmp(Current->Name, ".")) continue;
     if (!strcmp(Current->Name, "..")) continue;
     if (Current->Type ne Type_Session) continue;
    
     ThisUser = (User *) malloc(sizeof(User) + strlen(Current->Name));
     if (ThisUser eq  Null(User))
      { fprintf(stderr, "users : out of memory when building list of users.\n");
        return(EXIT_FAILURE);
      }
     strcpy(ThisUser->Name, Current->Name);
#ifdef SYSDEB
     ThisUser->Node.Next = ThisUser->Node.Prev = &ThisUser->Node;
#endif
     AddTail(Users, &(ThisUser->Node));
     NumberUsers++;
   }

  free(Entries); Entries = Null(DirEntry);
  Close(SM); SM = Null(Stream);
  
  if (NumberUsers eq 0)
   { fprintf(stderr, "users : nobody is logged in.\n");
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
