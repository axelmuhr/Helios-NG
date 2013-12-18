/*
 * ttymenu.c
 *
 * Incorporates the browser, for rummaging around on disks,
 * and the usual Emacs editing command menu
 *
 *	Copyright (c) 1986, Mike Meyer
 *	Mic Kaczmarczik did a few things along the way.
 *
 * Permission is hereby granted to distribute this program, so long as
 * this source file is distributed with it, and this copyright notice
 * is not removed from the file.
 *
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <intuition/intuition.h>
#undef	TRUE
#undef	FALSE
#include "def.h"
#ifndef	NO_MACRO
#include "macro.h"
#endif	

extern struct Menu		*AutoMenu ;
extern struct Window		*EmW ;

#ifdef	LATTICE
static VOID	Add_Devices(ULONG) ;
#else
static VOID	Add_Devices() ;
#endif

#define MNUM(menu,item,sub) (SHIFTMENU(menu)|SHIFTITEM(item)|SHIFTSUB(sub))

#ifdef	BROWSER
#define LONGEST_NAME	80	/* Longest file name we can deal with	*/

# ifdef	LATTICE
char *strchr(char *, int);
# else
char *index();			/* find first instance of c in s	*/
#define	strchr(s, c) index(s, c)
# endif

# ifdef	MENU
#define	FIRSTMENU	1
# else
#define	FIRSTMENU	0
# endif

#endif	BROWSER

#ifdef	MENU
/*
 * When ttgetc() sees a menu selection event, it stuffs the sequence
 * KMENU <menu><item><subitem> into the input buffer
 *
 * The menu item names are chosen to be relatively close to the extended
 * function names, so a user can usually figure out the key binding of
 * a menu item by searching through the "display-bindings" buffer
 * for something that's close.
 */

/*
 * Commands for managing files and buffers
 */

extern	int	filevisit();
extern	int	poptofile();
extern	int	fileinsert();
extern	int	filesave();
extern	int	filewrite();
#ifndef	NO_DIRED
extern	int	dired();
#endif
extern	int	usebuffer();
extern	int	poptobuffer();
extern	int	killbuffer();
extern	int	listbuffers();
extern	int	savebuffers();
extern	int	quit();

static struct MenuBinding FileItems[] = {
	{ "Find File         C-x C-f",	filevisit	},
	{ "Pop To File       C-x 4 f",	poptofile	},
	{ "Insert File       C-x i",	fileinsert	},
	{ "Save File         C-x C-s",	filesave	},
	{ "Write File        C-x C-w",	filewrite	},
#ifndef	NO_DIRED
	{ "Dired	     C-x d",	dired		},
#endif
	{ "Switch To Buffer  C-x b",	usebuffer	},
	{ "Pop To Buffer     C-x 4 b",	poptobuffer	},
	{ "Kill Buffer       C-x k",	killbuffer	},
	{ "List Buffers      C-x C-b",	listbuffers	},
	{ "Save Buffers      C-x s",	savebuffers	},
	{ "Save And Exit     C-x C-c",	quit		}
};

/*
 * Commands for various editing functions
 */

extern	int	yank();
extern	int	openline();
extern	int	killline();
extern	int	deblank();
extern	int	justone();
extern	int	indent();
extern	int	twiddle();
extern	int	quote();

static struct MenuBinding EditItems[] = {
	{ "Yank                 C-y",	yank		},
	{ "Blank Line           C-o ",	openline	},
	{ "Kill Line            C-k",	killline	},
	{ "Delete Blank Lines   C-x C-o",deblank	},
	{ "Delete Blanks        M-SPC",	justone		},
	{ "Newline And Indent   C-j",	indent		},
	{ "Transpose Characters C-t",	twiddle		},
	{ "Quoted Insert        C-q",	quote		}
};

/*
 * Movement commands
 */

extern	int	forwpage();
extern	int	backpage();
extern	int	gotobol();
extern	int	gotobob();
extern	int	gotoeol();
extern	int	gotoeob();
extern	int	gotoline();
extern	int	showcpos();

static struct MenuBinding MoveItems[] = {
	{ "Scroll Up       C-v",	forwpage	},
	{ "Scroll Down     M-v",	backpage	},
	{ "Start Of Line   C-a",	gotobol		},
	{ "Start Of Buffer M-<",	gotobob		},
	{ "End Of Line     C-e",	gotoeol		},
	{ "End Of Buffer   M->",	gotoeob		},
	{ "Goto Line",			gotoline	},
	{ "Show Cursor     C-x =",	showcpos	}
};

/*
 * Commands for searching and replacing
 */

extern	int	forwisearch();
extern	int	backisearch();
extern	int	searchagain();
extern	int	forwsearch();
extern	int	backsearch();
extern	int	queryrepl();

static struct MenuBinding SearchItems[] = {
	{ "I-Search Forward  C-s",	forwisearch	},
	{ "I-Search Backward C-r",	backisearch	},
	{ "Search Again",		searchagain	},
	{ "Search Forward    M-s",	forwsearch	},
	{ "Search Backward   M-r",	backsearch	},
	{ "Query Replace     M-%",	queryrepl	}
};

/*
 * Commands that manipulate words
 */
extern	int	forwword();
extern	int	backword();
extern	int	delfword();
extern	int	delbword ();
extern	int	capword();
extern	int	lowerword();
extern	int	upperword();

static struct MenuBinding WordItems[] = {
	{ "Forward Word       M-f",	forwword	},
	{ "Backward Word      M-b",	backword	},
	{ "Kill Word          M-d",	delfword	},
	{ "Backward Kill Word M-DEL",	delbword 	},
	{ "Capitalize Word    M-c",	capword		},
	{ "Downcase Word      M-l",	lowerword	},
	{ "Upcase Word        M-u",	upperword	}
};

/*
 * Commands relating to paragraphs
 */
extern	int	gotoeop();
extern	int	gotobop();
extern	int	fillpara();
extern	int	setfillcol();
extern	int	killpara();
extern	int	fillmode();

static struct MenuBinding ParaItems[] = {
	{ "Forward Paragraph  M-]",	gotoeop		},
	{ "Backward Paragraph M-[",	gotobop		},
	{ "Fill Paragraph     M-q",	fillpara	},
	{ "Set Fill Column    C-x f",	setfillcol	},
	{ "Kill Paragraph",		killpara	},
	{ "Auto Fill Mode",		fillmode	}
};

/*
 * Region stuff
 */
extern	int	setmark();
extern	int	swapmark();
extern	int	killregion();
extern	int	copyregion();
extern	int	lowerregion();
extern	int	upperregion();

static struct MenuBinding RegionItems[] = {
	{ "Set Mark            C-@",	setmark		},
	{ "Exch Point And Mark C-x C-x",swapmark	},
	{ "Kill Region         C-w",	killregion	},
	{ "Copy Region As Kill M-w",	copyregion	},
	{ "Downcase Region     C-x C-l",lowerregion	},
	{ "Upcase Region       C-x C-u",upperregion	}
};

/*
 * Commands for manipulating windows
 */

extern	int	splitwind();
extern	int	delwind();
extern	int	onlywind();
extern	int	nextwind();
#ifdef	PREVWIND
extern	int	prevwind();
#endif
extern	int	enlargewind();
extern	int	shrinkwind();
extern	int	refresh();
extern	int	reposition();
extern	int	togglewindow();
#ifdef	CHANGE_FONT
extern	int	setfont();
#endif

static struct MenuBinding WindowItems[] = {
	{ "Split Window         C-x 2", splitwind	},
	{ "Delete Window        C-x 0",	delwind		},
	{ "Delete Other Windows C-x 1",	onlywind	},
	{ "Next Window          C-x o",	nextwind	},
#ifdef	PREVWIND
	{ "Up Window",			prevwind	},
#endif
	{ "Enlarge Window       C-x ^",	enlargewind	},
	{ "Shrink Window",		shrinkwind	},
	{ "Redraw Display",		refresh		},
	{ "Recenter             C-l",	reposition	},
	{ "Toggle Border",		togglewindow	},
#ifdef	CHANGE_FONT
	{ "Set Font",			setfont		}
#endif
};

/*
 * Miscellaneous commands
 */

extern	int	definemacro();
extern	int	finishmacro();
extern	int	executemacro();
extern	int	extend();
extern	int	bindtokey();
extern	int	desckey();
extern	int	wallchart();
extern	int	showversion();
extern	int	spawncli();

static struct MenuBinding MiscItems[] = {
	{ "Start Kbd Macro   C-x (",	definemacro	},
	{ "End Kbd Macro     C-x )",	finishmacro	},
	{ "Call Kbd Macro    C-x e",	executemacro	},
	{ "Execute Command   M-x",	extend		},
	{ "Global Set Key",		bindtokey	},
	{ "Describe Key      C-h c",	desckey		},
	{ "Describe Bindings C-h b",	wallchart	},
	{ "Emacs Version",		showversion	},
	{ "New CLI           C-z",	spawncli	}
};

/*
 * The following table contains the titles, number of items, and
 * pointers to, the individual menus.
 */

static struct MenuInfo EMInfo[] = {
	{ "File  ",		NITEMS(FileItems),	&FileItems[0]	},
	{ "Edit  ",		NITEMS(EditItems),	&EditItems[0]	},
	{ "Move  ", 		NITEMS(MoveItems),	&MoveItems[0]	},
	{ "Search  ",		NITEMS(SearchItems),	&SearchItems[0] },
	{ "Word  ",		NITEMS(WordItems),	&WordItems[0]	},
	{ "Paragraph  ",	NITEMS(ParaItems),	&ParaItems[0]	},
	{ "Region  ",		NITEMS(RegionItems),	&RegionItems[0]	},
	{ "Window  ",		NITEMS(WindowItems),	&WindowItems[0] },
	{ "Miscellaneous  ",	NITEMS(MiscItems),	&MiscItems[0]	}
};

/* There are three cases to deal with; the menu alone, the Browser
 * alone, and both of them together.  We #define some things to make
 * life a little easier to deal with
 */
# ifdef	BROWSER
#  define Edit_Menu_Init() Menu_Add("Edit ", TRUE, FALSE) 
#  define Edit_Menu_Add(n) Menu_Item_Add(n,(USHORT)ITEMENABLED,0L,(BYTE)0,FALSE)
#  define Edit_Item_Add(n) Menu_SubItem_Add(n,(USHORT)ITEMENABLED,0L,(BYTE)0,FALSE)
# else
#  define Edit_Menu_Init() cinf = NULL	/* harmless */
#  define Edit_Menu_Add(n) n[strlen(n)-1] = '\0'; Menu_Add(n, TRUE, FALSE)
#  define Edit_Item_Add(n) Menu_Item_Add(n,(USHORT)ITEMENABLED,0L,(BYTE)0,FALSE)
# endif	BROWSER

#endif	MENU

/*
 * Initialize the Emacs menu
 */

struct Menu * InitEmacsMenu(EmW)
struct Window *EmW;
{
#ifdef	MENU
	register struct MenuInfo *cinf;
	register struct MenuBinding *lastbinding, *cb;
	struct MenuInfo *lastinfo;
#endif

	Menu_Init() ;			/* init the menu		*/

#ifdef	MENU
	Edit_Menu_Init() ;		/* Set up for editing menu	*/
	lastinfo = &EMInfo[NITEMS(EMInfo)];	/* loop sentinel	*/	
	for (cinf = EMInfo; cinf < lastinfo; cinf++) {
		Edit_Menu_Add(cinf->Name);
		lastbinding = &cinf->Items[cinf->NumItems];
		for (cb = cinf->Items; cb < lastbinding; cb++)
			Edit_Item_Add(cb->Command);
	}
#endif	MENU

#ifdef	BROWSER
	Menu_Add("Disks ", TRUE, FALSE) ;/* name is already there */
	Add_Devices(DLT_DEVICE);	/* devices first */
	Add_Devices(DLT_VOLUME);	/* mounted volume names next */
	Add_Devices(DLT_DIRECTORY);	/* assigned directories last */
#endif	BROWSER
	return 	AutoMenu ;
}

/*
 * amigamenu() -- handles a menu pick.
 */

amigamenu(f, n) {
	unsigned short		menunum, itemnum, subnum, Menu_Number;
	char			*name;
	register PF		fp;

#ifdef	BROWSER
	register unsigned short	level, i, dirp;
	register char		*cp;
	int			stat;
	struct MenuItem		*ItemAddress() ;

	/* State variables that describe the current directory */
	static char		Dir_Name[LONGEST_NAME] ;
	static unsigned short	Menu_Level = 0 ;
#endif

#ifndef	NO_MACRO
	if (inmacro) 
		return (FALSE);	/* menu picks aren't recorded */
#endif

	/* read the menu, item, and subitem codes from the input stream */
	menunum = getkey(FALSE) - MN_OFFSET;
	itemnum = getkey(FALSE) - MN_OFFSET;
	subnum = getkey(FALSE) - MN_OFFSET;

#ifndef	NO_MACRO
	if (macrodef) {		/* menu picks can't be practically recorded */
		ewprintf("Can't record menu selections");
		return (FALSE);
	}
#endif

	Menu_Number = (USHORT)
		(SHIFTMENU(menunum) | SHIFTITEM(itemnum) | SHIFTSUB(subnum));

#ifndef	BROWSER
# ifdef	MENU
	fp = EMInfo[menunum].Items[itemnum].Function;
	return (*(fp)(f, n));
# endif
#else	/* we're using the Browser */
# ifdef	MENU
	/* Handle commands from the Edit menu when using the Browser */
	if (0 == menunum) {
		fp = EMInfo[itemnum].Items[subnum].Function;
		return ((*fp)(f, n));
	}
# endif
	/* Here when a selection was made in a Browser menu */
	name = (char *)((struct IntuiText *)
		(ItemAddress(AutoMenu,(ULONG) Menu_Number) -> ItemFill))
		-> IText ;
	level = MENUNUM(Menu_Number) - FIRSTMENU;

	/* Got what we want, so clear the menu to avoid confusing the user */
	ClearMenuStrip(EmW) ;

	/* set dirp to FALSE if the name is not a directory or disk */
	dirp = (strchr(name, '/') != NULL || strchr(name, ':') != NULL) ;

	/* First, set the directory name right */
	if (level > Menu_Level)			/* Not possible, die */
		panic("impossible menu_level in amigamenu");
	else if (level == 0)			/* picked a new disk */
		Dir_Name[0] = '\0' ;
	else if (level < Menu_Level) {		/* Throw away some levels */
		for (i = 1, cp = strchr(Dir_Name, ':'); i < level; i++) {
			if (cp == NULL) return FALSE;
			cp = strchr(cp, '/') ;
			}
		if (cp == NULL) panic("broken file name in amigamenu");
		*++cp = '\0' ;
		}
	/* else Menu_Level == level, chose a file a current level */

	/* Now, fix up the menu and it's state variable */
	while (Menu_Level > level) {
		Menu_Level-- ;
		Menu_Pop() ;
		}

	/* If we added a file, visit it, else add a
	 * new directory level to the menu.
	 */
	if (!dirp)
		stat = Display_File(Dir_Name, name) ;
	else {
		Menu_Level++ ;
		(void) strncat(Dir_Name, name,
			LONGEST_NAME - strlen(Dir_Name) - 1) ;
		stat = Add_Dir(Dir_Name, name) ;
	}
	SetMenuStrip(EmW, AutoMenu) ;
	return stat ;
#endif	BROWSER
}

#ifdef	BROWSER
/*
 * Display_File - Go fetch a the requested file into a window.
 */
Display_File(dir, file) char *dir, *file; {
	register BUFFER	*bp, *findbuffer();
	int		s;
	char		File_Name[LONGEST_NAME], *fn, *adjustname();

	(void) strcpy(File_Name, dir);
	(void) strncat(File_Name, file, LONGEST_NAME - strlen(File_Name) - 1) ;
	fn = adjustname(File_Name);
	if ((bp = findbuffer(fn)) == NULL) return FALSE;
	curbp = bp;
	if (showbuffer(bp, curwp, WFHARD) != TRUE) return FALSE;
	if (bp->b_fname[0] == 0)
		return (readin(fn));		/* Read it in.	*/
	return TRUE;
	}
/*
 * Add_Dir - given a dir and a name, add the menu name with the files in
 *	dir as entries.  Use AllocMem() in order to make
 *      sure the file info block is on a longword boundary.
 */
static
Add_Dir(dir, name) char *dir, *name; {
	register char			*last_char ;
	register struct FileLock	*my_lock, *Lock() ;
	unsigned short			count ;
	int				stat = FALSE;
	static char			Name_Buf[LONGEST_NAME] ;
	char				*AllocMem();
	struct	FileInfoBlock		*File_Info;

	if ((File_Info = (struct FileInfoBlock *)
		AllocMem((LONG)sizeof(struct FileInfoBlock), 0L)) == NULL)
		return (FALSE);

	/* Fix up the trailing / if it needs it */
	last_char = &dir[strlen(dir) - 1] ;
	if (*last_char == '/') *last_char = '\0' ;

	/* Now, start on the directory */
	if ((my_lock = Lock(dir, ACCESS_READ)) == NULL) goto out;

	if (!Examine(my_lock, File_Info)) goto out;
	if (File_Info -> fib_DirEntryType < 0L)
		goto out;

	if (Menu_Add(name, TRUE, TRUE) == 0) goto out;
	for (count = 0; ExNext(my_lock, File_Info) 
			|| IoErr() != ERROR_NO_MORE_ENTRIES; count++)
		if (File_Info -> fib_DirEntryType < 0L) {
			if (Menu_Item_Add(File_Info -> fib_FileName,
				(USHORT)ITEMENABLED, 0L, (BYTE)0, TRUE)
					== MNUM(NOMENU, NOITEM, NOSUB))
					break ;
			}
		else {
			(void) strcpy(Name_Buf, File_Info -> fib_FileName) ;
			(void) strcat(Name_Buf, "/") ;
			if (Menu_Item_Add(Name_Buf,
				(USHORT) ITEMENABLED, 0L, (BYTE)0, TRUE)
					 == MNUM(NOMENU, NOITEM, NOSUB))
				break ;
			}
	if (count == 0) Menu_Item_Add("EMPTY", (USHORT)0, 0L, (BYTE)0, FALSE) ;

	/* Put everything back */
	if (*last_char == '\0') *last_char = '/' ;
	stat = TRUE;
out:
	UnLock(my_lock) ;
	FreeMem(File_Info, (LONG) sizeof(struct FileInfoBlock));
	return stat;
	}

/*
 * Add all the devices currently known by the system
 * to the current menu, based on the type of device
 * list entry desired.  Disable multitasking while
 * we look inside the device list, so we don't fly off
 * into space while traversing it.
 */
struct DosLibrary	*DosBase;
extern APTR		OpenLibrary();

static VOID
Add_Devices(devtype)
ULONG devtype;
{
	register struct DeviceList	*devlist;
	struct RootNode			*rootnode;
	struct DosInfo			*dosinfo;
	UBYTE				buffer[80];
	int				ramflag = 0;

	/* if you've gotten this far, none of these will be null. */
	DosBase = (struct DosLibrary *) OpenLibrary(DOSNAME,0L);

	Forbid();			/* let's be careful out there... */
	rootnode = (struct RootNode *) DosBase->dl_Root;
	dosinfo = (struct DosInfo *) BADDR(rootnode->rn_Info);
	devlist = (struct DeviceList *) BADDR(dosinfo->di_DevInfo);

	while (devlist) {
		/* select by specified device type */
		if (devlist->dl_Type != devtype) {
			devlist = (struct DeviceList *) BADDR(devlist->dl_Next);
			continue;
		}

		/* convert device's name into AmigaDOS name and concat a ":" */
		btocstr((BPTR) devlist->dl_Name,buffer,sizeof(buffer));
		strcat(buffer,":");

		/* Always add volumes and assigned directories. However,
		 * disks should be the only devices added to the list. Magic
		 * disk test courtesy of Phillip Lindsay, Commodore-Amiga Inc.
		 */
		if (devtype != DLT_DEVICE)
			Menu_Item_Add(buffer, (USHORT)ITEMENABLED,
					0L, (BYTE)0, TRUE);
		else if (devlist->dl_Task) {	/* why does this work? */
			Menu_Item_Add(buffer, (USHORT)ITEMENABLED,
					0L, (BYTE)0, TRUE);
			if (!strcmp(buffer,"RAM:")) ramflag = 1;
		}
		devlist = (struct DeviceList *) BADDR(devlist->dl_Next);
	}
	/* if ramdisk isn't loaded yet, add it anyway */
	if ((devtype == DLT_DEVICE) && !ramflag)
		Menu_Item_Add("RAM:",(USHORT)ITEMENABLED, 0L, (BYTE) 0, FALSE);
	Permit();
	CloseLibrary(DosBase);
}

btocstr(bp,buf,bufsiz)
BPTR bp;
char *buf;
int bufsiz;
{
	register UBYTE	*cp;
	register int	len, i;

	cp = (UBYTE *) BADDR(bp);
	len = (int) *(cp++);
	len = (len > bufsiz) ? bufsiz : len;	/* truncate if necessary */
	for (i = 0; i < len; i++)
		buf[i] = *(cp++);
	buf[i] = '\0';
	return (len < bufsiz);			/* return FALSE if truncated */
}

#endif	BROWSER
