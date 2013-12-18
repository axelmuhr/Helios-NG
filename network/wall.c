/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- wall.c								--
--                                                                      --
--	Send a broadcast message to all users.				--
--	Uses the X/Open standard without arguments, rather than the Sun	--
--	version which has options -a to broadcast to background		--
--	sessions and which can take a file name.			--
--                                                                      --
--	Author:  BLV 13/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/wall.c,v 1.6 1994/03/10 17:13:55 nickc Exp $*/

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

typedef struct TextLine {
	Node	Node;
	char	Text[1];
} TextLine;

static void usage(void);
static word free_node(Node *);
static word WalkDir(Object *sm, List *);
static void SendText(TextLine *Line, Stream *Window);

int main(int argc, char **argv)
{ Object	*SessionManager = Null(Object);
  int		rc = EXIT_FAILURE;
  char		login_id[NameMax];    
  List		TextList;
  
  if (argc ne 1) usage();
  argv = argv;		/* suppress warning */
  
  InitList(&TextList);

  SessionManager = RmGetSessionManager();
  if (SessionManager eq Null(Object))
   { fprintf(stderr, "wall : failed to locate session manager.\n");
     goto done;
   }

  unless(RmGetNames(Null(char), login_id))
   { fprintf(stderr, "wall : unable to validate user name.\n");
     goto done;
   }

  { char	*timebuf;
    time_t	now;
    char	buffer[256];
    TextLine	*Line;
    
    now		= time(Null(time_t));
    timebuf	= ctime(&now);
    sprintf(buffer, "\r\nBroadcast Message from %s at %.5s ...\n",
    		 login_id, &(timebuf[11]));

    Line = (TextLine *) malloc(sizeof(TextLine) + strlen(buffer));
    if (Line eq Null(TextLine))
     { fprintf(stderr, "wall : not enough memory for text.\n");
       goto done;
     }
    strcpy(Line->Text, buffer);
#ifdef SYSDEB
    Line->Node.Next = Line->Node.Prev = &Line->Node;
#endif
    AddTail(&TextList, &(Line->Node));
  }

  { char	buffer[256];
    TextLine	*Line;  	   

    while (fgets(buffer, 255, stdin) ne Null(char))
     { Line = (TextLine *) malloc(sizeof(TextLine) + strlen(buffer));
       if (Line eq Null(TextLine))
        { fprintf(stderr, "wall : not enough memory for text.\n");
          goto done;
        }
       strcpy(Line->Text, buffer);
#ifdef SYSDEB
       Line->Node.Next = Line->Node.Prev = &Line->Node;
#endif
       AddTail(&TextList, &(Line->Node));
     }
  }

  rc = WalkDir(SessionManager, &TextList);
  
done:
  if (SessionManager ne Null(Object)) Close(SessionManager);

  WalkList(&TextList, &free_node);	/* no FreeList in kernel */
				       /* (kernel does not know about free() )*/

  return(rc);
}

static void usage(void)
{ fprintf(stderr, "wall : usage, wall\n");
  exit(EXIT_FAILURE);
}

static word free_node(Node *node)
{ Remove(node);
  free(node);
  return(0);
}

static word WalkDir(Object *SessionManager, List *Text)
{ Stream	*SM = Null(Stream);
  int		Size;
  int		rc = EXIT_SUCCESS;
  DirEntry	*Entries = Null(DirEntry);
  DirEntry	*Current;

  SM = Open(SessionManager, Null(char), O_ReadOnly);
  if (SM eq Null(Stream))
   { fprintf(stderr, "wall : failed to open Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }

  Size = GetFileSize(SM);
  if (Size <= 0)
   { fprintf(stderr, "wall : error accessing Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }

  Entries = (DirEntry *) malloc(Size);
  if (Entries eq Null(DirEntry))
   { fprintf(stderr, "wall : out of memory when examing user sessions.\n");
     rc = EXIT_FAILURE;
     goto done;
   }
   
  if (Read(SM, (BYTE *) Entries, Size, -1) < Size)
   { fprintf(stderr, "wall : error reading Session Manager.\n");
     rc = EXIT_FAILURE;
     goto done;
   }
   
  for (Current = Entries; Size > 0; Current++, Size -= sizeof(DirEntry))
   { Object	*Session;
     Stream	*Window;

     if (!strcmp(Current->Name, ".")) continue;
     if (!strcmp(Current->Name, "..")) continue;
     if (Current->Type ne Type_Session) continue;
     
     Session = Locate(SessionManager, Current->Name);
     if (Session eq Null(Object))
      { fprintf(stderr, "wall : failed to access session %s, fault 0x%08x\n",
      		 Current->Name, Result2(SessionManager));
        rc = EXIT_FAILURE;
        continue;
      }
     Window = RmGetWindow(Session, Null(WORD));
     if (Window eq Null(Stream))
      { fprintf(stderr,
                "wall : failed to access window for session %s, fault 0x%08x\n",
                  Current->Name, Result2(Session));
        rc = EXIT_FAILURE;
        Close(Session);
        continue;
      }
     if ((Window->Flags & Flags_Interactive) ne 0)
      WalkList(Text, (WordFnPtr) &SendText, Window);
     Close(Window);
     Close(Session);
   }

done:
  if (SM ne Null(Stream)) Close(SM);
  if (Entries ne Null(DirEntry)) free(Entries);
  return(rc);
}

static void SendText(TextLine *Line, Stream *Window)
{ (void) Write(Window, (BYTE *) Line->Text, strlen(Line->Text), -1);
} 
