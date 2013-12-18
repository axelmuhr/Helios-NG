/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1992, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- passadm.c								--
--		Simple password administration utility.			--
--									--
--	Author:  BLV 5/10/92						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/passadm.c,v 1.4 1994/03/09 17:15:16 nickc Exp $*/

/*{{{  header files etc. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <pwd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <posix.h>
#include <helios.h>
#include <queue.h>
#include <syslib.h>
#include <attrib.h>
#include <gsp.h>
#include <codes.h>
#include <nonansi.h>
#include "netutils.h"

#ifndef eq
#define eq ==
#define ne !=
#endif
/*}}}*/
/*{{{  data structures */
/**
*** During initialisation all the password file entries are read in and
*** held internally in a linked list, allowing them to be displayed and 
*** edited. 
**/
	/* SunOs imposes a limit of eight characters on a user name.	*/
	/* This seems a sensible de facto standard. N.B the terminator	*/
	/* is stored as well.						*/
#define	MaxNameLen	9

	/* The maximum length of certain fields is limited by what	*/
	/* can be squeezed into an 80 column screen.			*/
#define	MaxFieldLen	61

typedef struct	PasswordNode {
	Node		Node;
	char		UserName[MaxNameLen];	/* What is the limit ?	*/
	char		Password[Passwd_Max];
	uid_t		Uid;
	uid_t		Gid;
	char		Gecos[MaxFieldLen];
	char		HomeDir[MaxFieldLen];
	char		Shell[MaxFieldLen];
} PasswordNode;

static	List	PasswordList;

	/* Screen sizes							*/
static	int	ScreenHeight;
static	int	ScreenWidth;

	/* Has any of the data been changed ?				*/
static	bool	Dirty	= FALSE;

/*}}}*/
/*{{{  screen manipulation */
/**
*** This contains the usual routines to initialise the screen and move
*** the cursor.
**/
static void	initialise_screen(void)
{ Attributes	attr;

  unless (isatty(0) && isatty(1))
   { fputs("passadm: not running interactively.\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (GetAttributes(fdstream(0), &attr) < Err_Null)
   { fputs("passadm: failed to get keyboard details.\n", stderr);
     exit(EXIT_FAILURE);
   }

  AddAttribute(&attr, ConsoleRawInput);
  RemoveAttribute(&attr, ConsoleEcho);
  RemoveAttribute(&attr, ConsoleRawOutput);
  RemoveAttribute(&attr, ConsoleBreakInterrupt);
  if (SetAttributes(fdstream(0), &attr) < Err_Null)
   { fputs("passadm: failed to initialise keyboard.\n", stderr);
     exit(EXIT_FAILURE);
   }
  ScreenHeight	= attr.Min;
  ScreenWidth	= attr.Time;

  setvbuf(stdin, NULL, _IONBF, 0);
}

static void move_to(int y, int x)
{ printf("\033[%d;%dH", y, x);
}

static void clear_screen()
{ putchar('\f');
}

static void waitfor_user()
{ int	x;

  move_to(ScreenHeight, 1);
  fputs("\033[KPress any key to continue.", stdout);
  fflush(stdout);
  x = getchar();
  move_to(ScreenHeight, 1);
  fputs("\033[K", stdout);
  fflush(stdout);
}

	/* Some miscellaneous output routines.			*/
static void display_banner(void)
{ clear_screen();
  printf("\t\t\t\033[7mPassword Administration\033[0m\r\n");
}

	/* N.B. this routine assumes that the cursor is in a sensible place.	*/
static void fatal(char *str, ...)
{ va_list	args;

  va_start(args, str);
  printf("passadm: ");
  vprintf(str, args);
  va_end(args);
  printf("\r\n");
  fflush(stdout);
  exit(EXIT_FAILURE);
}
/*}}}*/
/*{{{  read in password file */
/**
*** Reading in the password file. A typical password file contains lines
*** like the following:
***
*** root::0:0::/helios/users/root:/helios/bin/shell
***
*** The fields, separated by colons, are:
***  1) user name
***  2) encrypted password or blank, passwords are always 11 characters
***  3) numeric user id
***  4) numeric group id
***  5) "gecos", used by mail programs etc. This contains additional info
***     about the user such as full name
***  6) home directory
***  7) login shell, or other command
***
*** This code assumes that the file /helios/etc/passwd is currently in
*** the correct format. The entire file is read in and the data is held
*** in internal data.
**/

static	void	read_password_file(void)
{ FILE		*passwd_file;
  char		 line_buf[IOCDataMax];
  PasswordNode	*node;
  int		 line_number;

  move_to(3,1);		/* reasonable place for error messages.		*/

  /*{{{  initial check */
  	/* 1) check that the password file exists and is writable.	*/
  	/*    Otherwise a warning is given.				*/
    { Object	*passwd_obj;
  
      passwd_obj	= Locate(NULL, "/helios/etc/passwd");
      if (passwd_obj eq NULL)
       fatal("cannot find file /helios/etc/passwd");
  
      if ((passwd_obj->Access.Access & AccMask_W) eq 0)
       { fputs("passadm: warning, password file is read-only.", stdout);
         waitfor_user();
  	 move_to(3, 1);
  	 fputs("\033[K", stdout); fflush(stdout);
       }
      Close(passwd_obj);
    }
  /*}}}*/

  InitList(&PasswordList);

  passwd_file	= fopen("/helios/etc/passwd", "r");
  if (passwd_file eq NULL)
   fatal("cannot open file /helios/etc/passwd");

  line_number	= 0; 
  while (fgets(line_buf, IOCDataMax, passwd_file) ne NULL)
   { char	*current;
     char	*end;

     line_number++;

	/* Check that this is not a blank line. Allowing # as a comment	*/
	/* character is probably a bad move because the security	*/
	/* features of SunOS make use of # as a special character.	*/
     for (current = line_buf; isspace(*current) && (*current ne '\0'); current++);
     if (*current eq '\0') continue;

	/* Allocate a new password node structure.			*/
     node	= New(PasswordNode);
     if (node eq NULL)
      fatal("out of memory reading password file");

     node->Node.Next = node->Node.Prev = &node->Node;
     
	/* Field 1: user name.						*/
     current	= line_buf;
     end	= strchr(current, ':');
     if (end eq NULL)
      fatal("missing user name in password file on line %d", line_number);
     *end = '\0';
     if (strlen(current) >= MaxNameLen)
      fatal("user name too long on line %d", line_number);
     strcpy(node->UserName, current);

	/* Field 2: password or blank.					*/
     current	= end + 1;
     end	= strchr(current, ':');
     if (end eq NULL)
      fatal("missing password on line %d of password file", line_number);
     *end = '\0';
     if (current eq end)	/* blank password field.		*/
      node->Password[0]	= '\0';
     elif (strlen(current) ne (Passwd_Max - 1))
      fatal("invalid password on line %d of password file", line_number);
     else
      strcpy(node->Password, current);

	/* Field 3: uid							*/
     current	= end + 1;
     end	= strchr(current, ':');
     if (end eq NULL)
      fatal("missing uid field in line %d of password file", line_number);
     *end	= '\0';
     unless (isdigit(*current))
      fatal("invalid uid field in line %d of password file", line_number);
     node->Uid	= atoi(current);

	/* Check that root has user id 0				*/
     if (!strcmp(node->UserName, "root") && (node->Uid ne 0))
      { printf( "passadm: warning, user root should have uid 0 not %d, corrected.",
	       node->Uid);
        waitfor_user();
	move_to(3,1);
	fputs("\033[K", stdout); fflush(stdout);
        node->Uid = 0;
      }

	/* Field 4: gid							*/
     current	= end + 1;
     end	= strchr(current, ':');
     if (end eq NULL)
      fatal("missing gid field in line %d of password file", line_number);
     *end = '\0';
     unless (isdigit(*current))
      fatal("invalid gid field in line %d of password file", line_number);
     node->Gid	= atoi(current);

	/* Field 5 : "gecos"						*/
     current	= end + 1;
     end	= strchr(current, ':');
     if (end eq NULL)
      fatal("missing gecos field in line %d of password file", line_number);
     *end = '\0';
     if (strlen(current) >= MaxFieldLen)
      fatal("gecos field too long on line %d of password file", line_number);
     strcpy(node->Gecos, current);

	/* Field 6 : home directory					*/
     current	= end + 1;
     end	= strchr(current, ':');
     if (end eq NULL)
      fatal("missing home directory in line %d of password file", line_number);
     *end = '\0';
     if (strlen(current) >= MaxFieldLen)
      fatal("home directory field too long on line %d of password file", line_number);
     if (current[0] ne '/')
      fatal("invalid home directory on line %d of password file", line_number);
     strcpy(node->HomeDir, current);

	/* Check that the home directory actually exists, and give	*/
	/* a warning if not.						*/
     { Object	*tmp = Locate(NULL, node->HomeDir);
       if (tmp eq NULL)
        { printf("passadm: warning, line %d, cannot find home directory %s for user %s",
			line_number, node->HomeDir, node->UserName);
	  waitfor_user();
	  move_to(3,1);
	  printf("\033[K\n\033[K");
	  move_to(3,1);
	  fflush(stdout);
	}
       else
	Close(tmp);
     }

	/* Final field, the login shell.				*/
     current	= end + 1;
     if (current[0] ne '/')
      fatal("invalid login shell on line %d of password file", line_number);
     end = current + strlen(current) - 1;
     while(isspace(*end)) end--;	/* Strip blanks at end of line	*/
     *(++end) = '\0';
     if (strlen(current) >= MaxFieldLen)
      fatal("login shell field too long on line %d of password file", line_number);
     strcpy(node->Shell, current);

	/* Check that the specified program exists, or give a warning	*/
     { Object	*tmp = Locate(NULL, node->Shell);
       if (tmp eq NULL)
        { printf("passadm: warning, line %d, cannot find login shell %s for user %s",
			line_number, node->Shell, node->UserName);
	  waitfor_user();
	  move_to(3,1);
	  printf("\033[K\n\033[K");
	  move_to(3,1);
	  fflush(stdout);
	}
       else
	Close(tmp);
     }

     AddTail(&PasswordList, &(node->Node));
   }

  if (EmptyList_(PasswordList))
   fatal("there are no entries in the current password file");

	/* BLV - at this point the entire password file should have	*/
	/* been read in. Some checks, for example that there is a user	*/
	/* root, might seem like a good idea.				*/

  fclose(passwd_file);
}
/*}}}*/
/*{{{  write out password file */
/**
*** This routine is responsible for writing out the password file.
*** It works as follows:
***
***  1) rename the old password file to passwd.bak, deleting the previous
***	backup if any. This rename should fail if the password file is
***	read-only. Note that Helios calls have to be used here because
***	of the usual problems with renaming absolute filenames under Posix.
***  2) open the password file
***  3) walk down the list of password nodes and write out the data
***  4) close the password file
***
*** After that it would be very nice if the system could set up the
*** protection on the new password file...
**/
/*{{{  write_entry() */
static word	write_entry(PasswordNode *node, ...)
{ va_list	 args;
  FILE		*passwd_file;

  va_start(args, node);
  passwd_file	= va_arg(args, FILE *);
  va_end(args);

  if (fprintf(passwd_file, "%s:%s:%d:%d:%s:%s:%s\n", 
		node->UserName, node->Password, node->Uid, node->Gid,
		node->Gecos, node->HomeDir, node->Shell)
	< 0)
   return(1);
  return(0);
}
/*}}}*/

static	void write_password_file(void)
{ Object	*etc	= Locate(NULL, "/helios/etc");
  Object	*backup;
  FILE		*passwd_file;

  display_banner();
  move_to(3,1);
  printf("Writing out new password file...");
  move_to(5,1);
  fflush(stdout);

  if (etc eq NULL)
   fatal("cannot find directory /helios/etc");

  backup = Locate(etc, "passwd.bak");
  if (backup ne NULL)
   { if (Delete(backup, NULL) < Err_Null)
      fatal("cannot delete previous backup file");
     Close(backup);
   }

  if (Rename(etc, "passwd", "passwd.bak") < Err_Null)
   fatal("insufficient access, cannot replace password file");

  passwd_file	= fopen("/helios/etc/passwd", "w");
  if (passwd_file eq NULL)
   { (void) Rename(etc, "passwd.bak", "passwd");
     fatal("insufficient access, cannot create new password file");
   }

  if (SearchList(&PasswordList, (WordFnPtr) &write_entry, passwd_file) ne 0)
   { fclose(passwd_file);
     (void) Delete(etc, "passwd");
     (void) Rename(etc, "passwd.bak", "passwd");
     fatal("I/O error writing out new password file");
   }

  fclose(passwd_file);  

  Dirty = FALSE;
  printf("Done.\r\n\n");
}
/*}}}*/
/*{{{  forms stuff */
/**
*** This code provides a very simple forms package for administering the
*** password file.
**/
/*{{{  data structures */
typedef enum  { int_type, string_type } FormEntryType;
typedef union { int *int_ptr; char *string; } FormEntryValue;

typedef struct {
	int		 Y_Pos;
	int		 X_Pos;
	int		 Index;
	char		*Label;
	FormEntryType	 Type;
	FormEntryValue	 Value;
	int		 MaxLen;
	VoidFnPtr	 Change;
	word		 ChangeArg;
} FormEntry;

typedef struct {
	char		*Title;
	int		 NumberEntries;
	int		 CurrentEntry;
	FormEntry	*FormEntries;
} Form;


	/* These are the return codes for the edit_field() option	*/
#define	Edit_Redraw		0x0001	/* Flag	*/
#define Edit_Noop		0x0000
#define Edit_Prev		0x0010
#define Edit_Next		0x0020
#define Edit_Done		0x0030
#define Edit_Abort		0x0040
/*}}}*/
/*{{{  edit_field() */
/**
*** This routine deals with handling user input and editing a field.
**/
static int edit_field(FormEntry *entry)
{ int		 x_offset;
  char		 num_buffer[16];
  char		*edit_buffer;
  char		 input[4];
  int	 	 index;
  int		 length;
  int		 rc;
  bool		 changed = FALSE;
  int		 i;

  x_offset	= entry->X_Pos + strlen(entry->Label);
  index		= entry->Index;

  if (entry->Type eq int_type)
   { sprintf(num_buffer, "%d", *(entry->Value.int_ptr));
     edit_buffer = num_buffer;
   }
  else
   edit_buffer = entry->Value.string;
  length = strlen(edit_buffer);
  if (index eq -1)	/* no current index		*/
   index = length;	/* cursor follows existing data	*/

  forever
   { move_to(entry->Y_Pos, x_offset + index); 
     fflush(stdout);
     input[0] = getchar();
     switch(input[0])
      { 
	case 0x1B	: 	/* ESC		*/
			  rc = Edit_Done; goto done;

	case 0x07	: 	/* ctrl-G	*/
			  rc = Edit_Abort; goto done;

	case 0x0A	:	/* return	*/
	case 0x0D	:	/* also return	*/	
	case 0x0E	: 	/* ctrl-N	*/
			  rc = Edit_Next;
			  goto done;

	case 0x09	:	/* tab		*/
	case 0x10	:	/* ctrl-P	*/
			  rc = Edit_Prev;
			  goto done;

	case 0x0C	:	/* ctrl-L	*/
			  rc = Edit_Noop | Edit_Redraw; goto done;

	case 0x09B	:
	case 0xffffff9b	:	/* CSI		*/
                          /*{{{  process CSI */
                          	input[1] = getchar();
                          	if (input[1] eq 'A')		/* up	*/
                          	 { rc = Edit_Prev;
                          	   goto done;
                          	 }
                          	elif (input[1] eq 'B')		/* down	*/
                          	 { rc = Edit_Next;
                                   goto done;
                                 }
                          	elif (input[1] eq 'C')		/* right	*/
                          	 { if (index < length) index++;
                          	   break;
                          	 }
                          	elif (input[1] eq 'D')		/* left		*/
                          	 { if (index > 0) index--;
                          	   break;
                          	 }
                          	else	/* Discard this CSI, including further data	*/
                          	 { (void) Read(fdstream(0), input, 4, OneSec);
                          	   break;
                          	 }
                          /*}}}*/

	case 0x02	:	/* ctrl-B	*/
			  if (index > 0) index--;
			  break;

	case 0x06	:	/* ctrl-F	*/
			  if (index < length) index++;
			  break;

	case 0x01	:	/* ctrl-A	*/
			  index = 0;
			  break;

	case 0x05	:	/* ctrl-E	*/
			  index = length;
			  break;

	case 0x15	:	/* ctrl-U	*/
			  index = 0;
			  move_to(entry->Y_Pos, x_offset);
				/* and drop through to...	*/
	case 0x0B	:	/* ctrl-K	*/
			  for (i = index; i < length; i++)
			   { edit_buffer[index + i] = ' ';
			     putchar(' ');
			   }
			  length  = index;
			  changed = TRUE;
			  break;

	case 0x08	:	/* backspace	*/
			  if (index eq 0) break;
			  index--;
			  move_to(entry->Y_Pos, x_offset + index);
				/* and drop through to...	*/

	case 0x7F	:	/* DEL		*/
	case 0x04	:	/* ctrl-D	*/
			  if (index eq length) break;
			  for (i = index; i < (length - 1); i++)
			   { edit_buffer[i] = edit_buffer[i+1];
			     putchar(edit_buffer[i]);
			   }
			  putchar(' ');
			  length--;
			  changed = TRUE;
			  break;

	default		:	/* insert a character	*/
				/* Check that the character is valid	*/
			  if (entry->Type eq int_type)
			   { if ((input[0] < '0') || (input[0] > '9'))
			      break;
			   }
			  else	/* Allow any character except ctrl keys	  */
				/* or keys that have been filtered already*/
			   { if (input[0] < ' ') break;
			   }

				/* Check the maximum field length	*/
			  if (length > (entry->MaxLen - 1)) break;

			  for (i = length; i >= index; i--)
			   edit_buffer[i] = edit_buffer[i - 1];

			  edit_buffer[index] = input[0];
			  length++;

			  for (i = index; i < length; i++)
			   putchar(edit_buffer[i]);

			  index++;
			  changed = TRUE;
			  break;
			   
      }
   }

done:
  entry->Index	= index;
  edit_buffer[length] = '\0';
  if (entry->Type eq int_type)
   *(entry->Value.int_ptr) = atoi(edit_buffer);

	/* If there is a change function associated with this field,	*/
	/* invoke it now.						*/
  if ((changed) && (entry->Change ne NULL))
   { (*(entry->Change))(entry->ChangeArg);
     rc |= Edit_Redraw;
   }
  return(rc);
}
/*}}}*/
/*{{{  process_form() */
/**
*** This code implements a simple forms package. It returns TRUE if
*** the form has been accepted, false otherwise.
**/
static	bool	process_form(Form *form)
{ int	i;
  int	rc;

redraw:
  clear_screen();
  move_to(1, 40 - (strlen(form->Title) / 2));
  fputs(form->Title, stdout);

  for (i = 0; i < form->NumberEntries; i++)
   { FormEntry	*form_entry = &(form->FormEntries[i]);
     move_to(form_entry->Y_Pos, form_entry->X_Pos);
     fputs(form_entry->Label, stdout);
     if (form_entry->Type eq int_type)
      printf("%d", *(form_entry->Value.int_ptr));
     else
      fputs(form_entry->Value.string, stdout);
   }

  move_to(ScreenHeight - 1, 1);
  printf("Use ESC    to accept the current data\r\n");
  printf(" or ctrl-G to abort.");

  forever
   { rc = edit_field(&(form->FormEntries[form->CurrentEntry]));
     switch (rc & ~Edit_Redraw)
      { case Edit_Prev :
			if (form->CurrentEntry > 0)
			 form->CurrentEntry--;
			else
			 form->CurrentEntry = form->NumberEntries - 1;
			break;

	case Edit_Next : 
			if (form->CurrentEntry < (form->NumberEntries - 1))
			 form->CurrentEntry++;
			else
			 form->CurrentEntry = 0;
			break;

	case Edit_Done		: return(TRUE);
	case Edit_Abort		: return(FALSE);

	default			: break;
      }

     if (rc & Edit_Redraw) goto redraw;
   }
}
/*}}}*/
/*}}}*/
/*{{{  menu */

/**
*** This code displays some or all of the password file entries currently
*** held in memory, with all the problems that entails in terms of
*** starting position, end position, current entry, different screen sizes,
*** etc. Then it displays a simple menu and acts accordingly.
**/
static	PasswordNode	*First		= NULL;
static	PasswordNode	*Current	= NULL;
static	PasswordNode	*Last		= NULL;
static	int		 Current_YPos;
static	int		 NumberToDisplay;

/*{{{  next page */
static	void	next_page(void)
{ if (Last eq Tail_(PasswordNode, PasswordList))
   return;
  First = Current = Next_(PasswordNode, Last);
}
/*}}}*/
/*{{{  previous page */
static	void	previous_page(void)
{ int	i;

  if (First eq Head_(PasswordNode, PasswordList))
   return;
  for (i = 0; i < NumberToDisplay; i++)
   { First = Prev_(PasswordNode, First);
     if (First eq (PasswordNode *) &PasswordList)
      { First = Next_(PasswordNode, First); break; }
   }
  Current = First;
}
/*}}}*/
/*{{{  first */
static	void	first_password(void)
{ First = Current = Head_(PasswordNode, PasswordList);
}
/*}}}*/
/*{{{  last */
static void last_password(void)
{ int i;

  First = Current = Tail_(PasswordNode, PasswordList);
	/* Walk back NumberToDisplay passwords, then forwards one	*/
  for (i = 0; i < NumberToDisplay; i++)
   { First = Prev_(PasswordNode, First);
     if (First eq (PasswordNode *) &PasswordList) break;
   }
  First	= Next_(PasswordNode, First);
}
/*}}}*/
/*{{{  down */
	/* Move down one line. Returns TRUE if screen OK, FALSE for	*/
	/* complete redraw.						*/
static bool down_line(void)
{ 
  if (Current eq Tail_(PasswordNode, PasswordList))
   return(TRUE);

  if (Current eq Last)
   { next_page(); return(FALSE); }

  move_to(Current_YPos, 1);
  printf("%-8.8s", Current->UserName);
  Current_YPos	+= 3;
  move_to(Current_YPos, 1);
  Current	 = Next_(PasswordNode, Current);
  printf("\033[7m%-8.8s\033[0m", Current->UserName);
  return(TRUE);
}
/*}}}*/
/*{{{  up */
	/* Similar for moving up a line.				*/
static bool up_line(void)
{
  if (Current eq Head_(PasswordNode, PasswordList))
   return(TRUE);

  if (Current eq First)
   { int i;
     for (i = 0; i < NumberToDisplay; i++)
      { First = Prev_(PasswordNode, First);
        if (First eq (PasswordNode *) &PasswordList)
	 { First = Next_(PasswordNode, First); break; }
      }
     Current = Prev_(PasswordNode, Current);
     return(FALSE);
   }

  move_to(Current_YPos, 1);
  printf("%-8.8s", Current->UserName);
  Current_YPos	-= 3;
  move_to(Current_YPos, 1);
  Current	 = Prev_(PasswordNode, Current);
  printf("\033[7m%-8.8s\033[0m", Current->UserName);
  return(TRUE);
}
/*}}}*/
/*{{{  help */
static char	*text1	= "\
Passadm - help information\n\n\
The main display is dedicated to some or all of the passwords currently\n\
in the password file. Each password entry contains the following fields:\n\
  Name	   : the name typed in when a user logs in.\n\
  Password : if there is a password associated with this user then the\n\
             encrypted password will be shown. The system does not store\n\
             clear-text versions of these passwords.\n\
  Uid      : the Unix user id. It is relevant mainly when\n\
             accessing certain filing systems, particularly NFS.\n\
  Gid      : similarly, this is the Unix group id. Currently Helios\n\
             does not maintain a groups database and a user can be\n\
             in only one group.\n\
  Gecos    : this contains additional information about the user,\n\
             usually the full name. This information may be used by some\n\
             applications, for example mail programs.\n\
  Home     : the user's home directory.\n\
  Shell    : the program started up when this user logs in, usually\n\
             a shell.\n\n\
";

static char	*text2 = "\
If there are too many passwords to display on one screen then it is\n\
possible to move from one screen to the next or to the previous one.\n\
In addition it is possible to move directly to the first or the last\n\
password entry.\n\n\
";

static char *text3 = "\
The cursor up and cursor down keys can be used to move between passwords.\n\
The MicroEmacs keys ctrl-N and ctrl-P are also supported.\n\n\
";

static char *text4 = "\
It is possible to escape out of the program into another shell using !.\n\
Pressing H or ? brings up this help screen. Q is used to exit the program.\n\n\
";

static char *text5 = "\
The Add new user option will bring up a screen requesting the necessary\n\
information. The home directory will be created automatically if\n\
requested. The new password entry will be added at the end of the file.\n\
Alternatively the current password entry can be deleted or edited.\n\n\
";

static void help(void)
{ FILE	*output;

  output = popen("/helios/bin/more", "w");
  if (output eq NULL)
   output = stdout;
  fputs(text1, output);
  fputs(text2, output);
  fputs(text3, output);
  fputs(text4, output);
  fputs(text5, output);
  if (output ne stdout)
   pclose(output);
  initialise_screen();
  waitfor_user();
}
/*}}}*/
/*{{{  cursor keys */
	/* Handle a cursor key or other CSI sequence		*/
static bool cursor_key(void)
{ BYTE	buf[16];

  if (Read(fdstream(0), buf, 1, 2 * OneSec) ne 1)
	/* What's up doc ?	*/
   return(TRUE);

  if (buf[0] eq 'B') return(down_line());
  if (buf[0] eq 'A') return(up_line());

  if (buf[0] eq 'H') 	/* HOME	*/
   { first_password(); return(FALSE); }
  if (buf[0] eq '?')	/* HELP	*/
   { (void) Read(fdstream(0), buf, 1, OneSec);
     help();
     return(FALSE);
   }
  if ((buf[0] >= '2') && (buf[0] <= '4'))	/* END, PgUp, PgDn	*/
   { (void) Read(fdstream(0), &(buf[1]), 1, OneSec);
     if (buf[1] eq 'z')		/* Check it is not a function key	*/
      { if (buf[0] eq '2')	/* END					*/
         last_password();
	elif (buf[0] eq '3')	/* PgUp					*/
	  previous_page();
	else
	  next_page();		/* PgDn					*/
	return(FALSE);
      }
     elif (buf[1] eq '~')
      return(TRUE);
   }
	/* Ignore other CSI sequences, discarding excess data	*/
  (void) Read(fdstream(0), buf, 16, OneSec);
  return(TRUE);
}
/*}}}*/
/*{{{  add user */

/*{{{  change name */
/**
*** This routine is invoked by the forms code whenever the name field
*** changes. If no home directory has been specified yet then this field
*** is filled in.
**/
static	void	change_name(PasswordNode *node)
{ 
  if ((strlen(node->UserName) > 0) && (strlen(node->HomeDir) eq 0))
   { strcpy(node->HomeDir, "/helios/users/");
     strcat(node->HomeDir, node->UserName);
   }
}

/*}}}*/

static FormEntry PasswordEntries[7] = {
 {  3, 1, 0, "Name     : ", string_type, NULL, MaxNameLen,  NULL, NULL },
 {  5, 1, 0, "Password : ", string_type, NULL, Passwd_Max,  NULL, NULL },
 {  7, 1, 0, "Uid      : ", int_type,    NULL, 8,	    NULL, NULL },
 {  9, 1, 0, "Gid      : ", int_type,    NULL, 8,	    NULL, NULL },
 { 11, 1, 0, "Gecos    : ", string_type, NULL, MaxFieldLen, NULL, NULL },
 { 13, 1, 0, "Home     : ", string_type, NULL, MaxFieldLen, NULL, NULL },
 { 15, 1, 0, "Shell    : ", string_type, NULL, MaxFieldLen, NULL, NULL } 
};

static Form PasswordForm = { NULL, 7, 1, PasswordEntries };

static void add_user(void)
{ PasswordNode	*new_password;
  int		 i;

  new_password = (PasswordNode *) malloc(sizeof(PasswordNode));
  if (new_password eq NULL)
   { move_to(ScreenHeight, 1);
     printf("\033[KSorry, out of memory.\r\n");
     fflush(stdout);
     exit(EXIT_FAILURE);
   }
  new_password->UserName[0]	= '\0';
  new_password->Password[0]	= '\0';
  new_password->Uid		= 100;
  new_password->Gid		= 0;
  new_password->Gecos[0]	= '\0';
  new_password->HomeDir[0]	= '\0';
  strcpy(new_password->Shell, "/helios/bin/shell");

  PasswordForm.Title			= "Add New User";
  PasswordForm.CurrentEntry		= 0;
  PasswordEntries[0].Value.string	= new_password->UserName;
  PasswordEntries[0].Change		= (VoidFnPtr) &change_name;
  PasswordEntries[0].ChangeArg		= (word) new_password;
  PasswordEntries[1].Value.string	= new_password->Password;
  PasswordEntries[2].Value.int_ptr	= (int *) &(new_password->Uid);
  PasswordEntries[3].Value.int_ptr	= (int *) &(new_password->Gid);
  PasswordEntries[4].Value.string	= new_password->Gecos;
  PasswordEntries[5].Value.string	= new_password->HomeDir;
  PasswordEntries[6].Value.string	= new_password->Shell;
  for (i = 0; i <= 6; i++)
   PasswordEntries[i].Index = -1;

back:
  unless (process_form(&PasswordForm))
   { free(new_password);
     return;
   }

	/* Check that all the required information is present	*/
  if (strlen(new_password->UserName) == 0)
   { PasswordForm.CurrentEntry	= 1;
     goto back;
   }

  if (!strcmp(new_password->UserName, "root"))
    new_password->Uid	= 0;

  if (strlen(new_password->HomeDir) eq 0)
   { PasswordForm.CurrentEntry = 5;
     goto back;
   }

  if (strlen(new_password->Shell) eq 0)
   { PasswordForm.CurrentEntry = 6;
     goto back;
   }

	/* Check that the specified shell or other command	*/
	/* actually exists. If not, error.			*/
  { Object	*tmp = Locate(NULL, new_password->Shell);
    if (tmp eq NULL)
     { move_to(ScreenHeight, 1);
       printf("\033[KCannot find command %s\n", new_password->Shell);
       fflush(stdout);
       PasswordForm.CurrentEntry = 6;
       goto back;
     }
    else
     Close(tmp);
  }

	/* Check that the specified home directory exists.	*/
	/* If not give the user a chance to create and		*/
	/* initialise it.					*/
  { Object	*home = Locate(NULL, new_password->HomeDir);
    int		 answer;

    if (home eq NULL)
     { move_to(ScreenHeight, 1);
       printf("\033[KCreate home directory (y/n) ? ");
       fflush(stdout);

       for (answer = '\0'; (answer ne 'Y') && (answer ne 'y') && (answer ne 'n') && (answer ne 'N'); answer = getchar());
       if ((answer eq 'y') || (answer eq 'Y'))
        { if (mkdir(new_password->HomeDir, 0666) eq 0)
           { char	 buf[MaxFieldLen + 10];
	     FILE	*tmp;
	     sprintf(buf, "%s/.cshrc", new_password->HomeDir);
	     tmp = fopen(buf, "w");
	     if (tmp ne NULL)
	      { fputs("set history=20\n", tmp);
	 	fputs("set savehist=20\n", tmp);
		fputs("alias h history\n", tmp);
		fputs("alias ls ls -F\n", tmp);
		fputs("alias me emacs\n", tmp);
		fputs("set path=( . /helios/local/bin /helios/bin )\n", tmp);
		fclose(tmp);
	      }
	     sprintf(buf, "%s/.login", new_password->HomeDir);
	     tmp = fopen(buf, "w");
	     if (tmp ne NULL)
	      { fputs("\n", tmp);
		fclose(tmp);
	      }
           }
	  else
	   { move_to(ScreenHeight - 1, 1);
	     printf("\033[KWarning, failed to create home directory.\n");
	     waitfor_user();
           }
        }
     }
    else	/* Home directory already exists	*/
     Close(home);
  }     
	/* The supplied information appears to be sufficient.	*/
	/* Incorporate the new password into the list and make	*/
	/* it current. If there is a password then it should	*/
	/* be encrypted.					*/
  if (strlen(new_password->Password) > 0)
   EncodePassword(new_password->Password, new_password->Password);
  AddTail(&PasswordList, &(new_password->Node));
  last_password();	/* Sorts out screen positioning etc.	*/
  Dirty = TRUE;
}

/*}}}*/
/*{{{  delete user */
static void delete_user(void)
{ PasswordNode	*new_current;

	/* Check that this is not the last entry in the password file.	*/
  if ((Current eq Head_(PasswordNode, PasswordList)) &&
      (Current eq Tail_(PasswordNode, PasswordList)))
   { move_to(ScreenHeight - 1, 1);
     printf("\033[KThere must always be at least one entry in the password file.");
     waitfor_user();
     return;
   }

  new_current = Next_(PasswordNode, Current);
  if (EndOfList_(new_current))
   new_current = Prev_(PasswordNode, Current);

  if (First eq Current)
   First = new_current;

  Remove(&(Current->Node));
  Free(Current);
  Current = new_current;
  Dirty = TRUE;
}
/*}}}*/
/*{{{  edit user */

/**
*** Editing an existing entry is similar to adding a new one.
**/
static void edit_user(void)
{ PasswordNode	*new_password;
  int		 i;

  new_password	= (PasswordNode *) malloc(sizeof(PasswordNode));
  if (new_password eq NULL)
   { move_to(ScreenHeight, 1);
     printf("\033[KSorry, out of memory.\r\n");
     fflush(stdout);
     exit(EXIT_FAILURE);
   }

  strcpy(new_password->UserName, Current->UserName);
  strcpy(new_password->Password, Current->Password);
  new_password->Uid	= Current->Uid;
  new_password->Gid	= Current->Gid;
  strcpy(new_password->Gecos, Current->Gecos);
  strcpy(new_password->HomeDir, Current->HomeDir);
  strcpy(new_password->Shell, Current->Shell);

  PasswordForm.Title			= "Edit User";
  PasswordForm.CurrentEntry		= 0;
  PasswordEntries[0].Value.string	= new_password->UserName;
  PasswordEntries[0].Change		= NULL;
  PasswordEntries[0].ChangeArg		= NULL;
  PasswordEntries[1].Value.string	= new_password->Password;
  PasswordEntries[2].Value.int_ptr	= (int *) &(new_password->Uid);
  PasswordEntries[3].Value.int_ptr	= (int *) &(new_password->Gid);
  PasswordEntries[4].Value.string	= new_password->Gecos;
  PasswordEntries[5].Value.string	= new_password->HomeDir;
  PasswordEntries[6].Value.string	= new_password->Shell;
  for (i = 0; i <= 6; i++)
   PasswordEntries[i].Index = -1;

back:
  unless (process_form(&PasswordForm))
   { free(new_password);
     return;
    }
	/* Check that all the required information is present	*/
  if (strlen(new_password->UserName) == 0)
   { PasswordForm.CurrentEntry	= 1;
     goto back;
   }

  if (!strcmp(new_password->UserName, "root"))
    new_password->Uid	= 0;

  if (strlen(new_password->HomeDir) eq 0)
   { PasswordForm.CurrentEntry = 5;
     goto back;
   }

  if (strlen(new_password->Shell) eq 0)
   { PasswordForm.CurrentEntry = 6;
     goto back;
   }

	/* Check that the specified shell or other command	*/
	/* actually exists. If not, error.			*/
  { Object	*tmp = Locate(NULL, new_password->Shell);
    if (tmp eq NULL)
     { move_to(ScreenHeight, 1);
       printf("\033[KCannot find command %s\n", new_password->Shell);
       fflush(stdout);
       PasswordForm.CurrentEntry = 6;
       goto back;
     }
    else
     Close(tmp);
  }

	/* The edited information appears to be sufficient.	*/
	/* Incorporate the changed information into Current.	*/
	/* Dirty is set only if anything has changed.		*/
  if (strcmp(Current->UserName, new_password->UserName))
   { strcpy(Current->UserName, new_password->UserName); Dirty = TRUE; }
  if (strcmp(Current->Password, new_password->Password))
   { EncodePassword(new_password->Password, Current->Password); Dirty = TRUE; }
  if (Current->Uid ne new_password->Uid)
   { Current->Uid = new_password->Uid; Dirty = TRUE; }
  if (Current->Gid ne new_password->Gid)
   { Current->Gid = new_password->Gid; Dirty = TRUE; }
  if (strcmp(Current->Gecos, new_password->Gecos))
   { strcpy(Current->Gecos, new_password->Gecos); Dirty = TRUE; }
	/* BLV - rename directory, copy and delete, create new	?	*/
  if (strcmp(Current->HomeDir, new_password->HomeDir))
   { strcpy(Current->HomeDir, new_password->HomeDir); Dirty = TRUE; }
  if (strcmp(Current->Shell, new_password->Shell))
   { strcpy(Current->Shell, new_password->Shell); Dirty = TRUE; }
}

/*}}}*/
/*{{{  shell escape */
static void shell_escape(void)
{ int	pid, rc;

  printf("\r\n"); fflush(stdout);

  if ((pid = vfork()) eq 0)
   { execl("/helios/bin/shell", "shell", NULL);
     _exit(0);
   }
  else
   waitpid(pid, &rc, 0);
  initialise_screen();
}
/*}}}*/

static void menu(void)
{ int		 number_to_display	= (ScreenHeight - 5) / 3;
  int		 i;
  PasswordNode	*tmp;
  static char	*inverse		= "\033[7m";
  static char	*normal			= "\033[0m";
  int		 request;

  NumberToDisplay	= (ScreenHeight - 5) / 3;
  First = Current	= Head_(PasswordNode, PasswordList);

  forever
   { display_banner();
     move_to(3,1);

     tmp	= First;
     for (i = 0, tmp = First; i < number_to_display; i++)
      { if (tmp eq Current) Current_YPos = 3 + (3 * i);
        printf("%s%-8.8s%s ",
	 (tmp eq Current) ? inverse : "",
 	 tmp->UserName,
	 (tmp eq Current) ? normal : "");

        printf("%-14.14s ", 
		(tmp->Password[0] eq '\0') ? "<no password>" : tmp->Password);
        printf("Uid: %-4.4d  Gid: %-4.4d  Gecos: %-.26s\r\n", tmp->Uid, tmp->Gid, tmp->Gecos);
        printf("         Home: %-30.30s Shell: %-.26s\r\n\n", tmp->HomeDir, tmp->Shell);

        tmp = Next_(PasswordNode, tmp);
	if (EndOfList_(tmp)) break;
      }

	/* At the end of the loop tmp points to the password node	*/
	/* after the last one displayed, or to the earth field of the	*/
	/* list.							*/
     Last	= Prev_(PasswordNode, tmp);

     move_to(ScreenHeight - 1, 1);
     printf("%sF%sirst, %sL%sast, %sN%sext page, %sP%srev page, %s!%s, %sH%selp, %s?%s, %sQ%suit\r\n",
	inverse, normal, inverse, normal, inverse, normal, inverse, normal,
	inverse, normal, inverse, normal, inverse, normal, inverse, normal);
     printf("%sA%sdd new user, %sD%selete current, %sE%sdit current, %sS%save changes  ",
	inverse, normal, inverse, normal, inverse, normal, inverse, normal);

back:
     move_to(ScreenHeight, 60);
     fflush(stdout);

     request	= getchar();
     switch(request)
      { case 'q'	:     
	case 'Q'	:
	case 0x03	:	/* ctrl-C	*/
	case 0x04	:	/* ctrl-D	*/
	case 0x07	:	/* ctrl-G	*/
	case 0x1B	: return;
	
	case 0x0C	: continue;	/* ctrl-L, redraw	*/

	case 'f'	:
	case 'F'	: first_password(); continue;
	case 'l'	:
	case 'L'	: last_password(); continue;
	case 0x16	:	/* ^V	*/
	case 'n'	:
	case 'N'	: next_page(); continue;
	case 0x1A	:	/* ^Z	*/
	case 'p'	:
	case 'P'	: previous_page(); continue;

	case 0x0E	: if (down_line())	/* ^N	*/
				break;
			  else
				continue;

	case 0x10	: if (up_line())	/* ^P	*/
				break;
			  else
				continue;

	case 0x09B	:
	case 0xFFFFFF9B	: if (cursor_key())
				break;
			  else
				continue;

	case '!'	: shell_escape(); continue;

	case '?'	:
	case 'h'	:
	case 'H'	: help(); continue;

	case 'd'	:
	case 'D'	: delete_user(); continue;

	case 's'	:
	case 'S'	: if (Dirty)
			   { write_password_file();
			     waitfor_user();
			     continue;
			   }
			  else
			   break;

	case 'a'	:
	case 'A'	: add_user(); continue;

	case 'e'	:
	case 'E'	: edit_user(); continue;
			   
	default		: break;
      }

	/* by default no redrawing is done. Requests that corrupt	*/
	/* the screen should use continue to go back to the top of the	*/
	/* forever loop.						*/
     goto back;
     request = request;
   }
}

/*}}}*/
/*{{{  main() */
int	main(int argc, char **argv)
{
  argc = argc; argv = argv;	/* Ignore arguments	*/

	/* Set up the screen parameters and display the banner	*/
  initialise_screen();
  if ((ScreenHeight < 24) || (ScreenWidth < 80))
   { fputs("passadm: screen too small.\n", stdout);
     exit(EXIT_FAILURE);
   }
  display_banner();
 
  read_password_file();

  menu();

  if (Dirty)
   { char input[4];

     display_banner();
     move_to(3, 1);
     printf("Save modified data (Y/N) ? ");
     fflush(stdout);

     for (input[0] = ' '; (input[0] ne 'y') && (input[0] ne 'Y') && (input[0] ne 'n') && (input[0] ne 'N'); )
      while (Read(fdstream(0), input, 1, -1) < 1);

     if ((input[0] eq 'y') || (input[0] eq 'Y'))
      write_password_file();
   }

  printf("\r\n\n");
  return(EXIT_SUCCESS);
}

/*}}}*/
