/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- sendto.c								--
--                                                                      --
--	Send a message to one or more users.				--
--                                                                      --
--	Author:  BLV 21/8/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/sendto.c,v 1.6 1994/03/10 17:13:50 nickc Exp $*/

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
static bool sendto(Object *, char *user, List *);
static void SendText(TextLine *Line, Stream *Window);

/**
*** 1) Process the arguments, extracting the subject if any.
*** 2) Check that there is a Session Manager, and determine the current
***    username.
*** 3) Construct the first line of the message, something like
***    Message from bart.1 at 22.36 about Getting some grub
*** 4) Read in the rest of the message from stdin
*** 5) For every user name listed, send the message to that user
**/
int main(int argc, char **argv)
{ Object	*SessionManager = Null(Object);
  int		rc = EXIT_FAILURE;
  char		login_id[NameMax];    
  List		TextList;
  char		*subject = Null(char);
  
  if (argc < 2) usage();
  argv++; argc--;		/* Switch to first argument */
  if (**argv eq '-')
   { char *temp = *argv;
     if (temp[1] ne 's') usage();
     if (temp[2] ne '\0')
      { subject = &(temp[2]);
        argc--;
        argv++;
      }
     else
      { if (argc == 1) usage();
      	argv++;
        subject = *argv;
        argv++;
        argc -= 2;
      }
   }
  
  if (argc < 1) usage();	/* no user id */
  
  InitList(&TextList);

  SessionManager = RmGetSessionManager();
  if (SessionManager eq Null(Object))
   { fprintf(stderr, "sendto: failed to locate session manager.\n");
     goto done;
   }

  unless(RmGetNames(Null(char), login_id))
   { fprintf(stderr, "sendto: unable to validate user name.\n");
     goto done;
   }

  { char	*timebuf;
    time_t	now;
    char	buffer[256];
    TextLine	*Line;
    
    now		= time(Null(time_t));
    timebuf	= ctime(&now);
    if (subject ne Null(char))
     sprintf(buffer, "\r\nMessage from %s at %.5s about %s\n",
     		login_id, &(timebuf[11]), subject);
    else
     sprintf(buffer, "\r\nMessage from %s at %.5s ...\n",
    		 login_id, &(timebuf[11]));

    Line = (TextLine *) malloc(sizeof(TextLine) + strlen(buffer));
    if (Line eq Null(TextLine))
     { fprintf(stderr, "sendto: not enough memory for text.\n");
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
        { fprintf(stderr, "sendto: not enough memory for text.\n");
          goto done;
        }
       strcpy(Line->Text, buffer);
#ifdef SYSDEB
       Line->Node.Next = Line->Node.Prev = &Line->Node;
#endif
       AddTail(&TextList, &(Line->Node));
     }
  }

  rc = EXIT_SUCCESS;
  for ( ; argc > 0; argc--, argv++)
   unless(sendto(SessionManager, *argv, &TextList))
    rc = EXIT_FAILURE;
  
done:
  if (SessionManager ne Null(Object)) Close(SessionManager);

  WalkList(&TextList, &free_node);	/* no FreeList in kernel */
				       /* (kernel does not know about free() )*/

  return(rc);
}

static void usage(void)
{ fprintf(stderr, "sendto: usage, sendto [-s subject] user [user] [user]...\n");
  exit(EXIT_FAILURE);
}

static word free_node(Node *node)
{ Remove(node);
  free(node);
  return(0);
}

/**
*** Try to get a window for the specified user, if it is an interactive
*** session, and send the message to that user.
**/
static bool sendto(Object *SessionManager, char *user, List *Text)
{ Object	*Session = Locate(SessionManager, user);
  Stream	*Window  = Null(Stream);
  
  if (Session eq Null(Object))
   { fprintf(stderr, "sendto: failed to locate session %s\n", user);
     return(FALSE);
   }
  unless(RmTestInteractiveSession(Session))
   { fprintf(stderr, "sendto: session %s is not interactive\n", user);
     return(FALSE);
   }

  Window = RmGetWindow(Session, Null(WORD));
  if (Window eq Null(Stream))
   { fprintf(stderr, "sendto: failed to access window for session %s\n", user);
     Close(Session);
     return(FALSE);
   }
  WalkList(Text, (WordFnPtr) &SendText, Window);
  Close(Window);
  Close(Session);
  return(TRUE);
}

static void SendText(TextLine *Line, Stream *Window)
{ (void) Write(Window, (BYTE *) Line->Text, strlen(Line->Text), -1);
} 
