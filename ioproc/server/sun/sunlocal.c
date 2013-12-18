/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1987, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  (unix)local.c                                                          --
--                                                                      --
--  Author : BLV 13.4.89                                                --
--   (based very loosely on code by DJCH, Bath University)              --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: sunlocal.c,v 1.26 1994/06/29 13:46:19 tony Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd.        			*/

/*   C H A N G E S
 *
 * oct.89 - check for TERM environment variable having the value "sun-cmd"
 * and comment the check for standard streams redirection.
 * 06.11.89 - introduced termcap_ks and termcap_ke 
 * and uses it in initialise_dumb_terminal and restore_dumb_terminal.
 *
 */

#define Local_Module

#if (ARMBSD)
#include "helios.h"
#else
#include "../helios.h"
#endif

#if SOLARIS
#include <stdlib.h>
#include <unistd.h>
#include <utime.h>
#include <limits.h>
#include <sys/statfs.h>
#endif

#if SUN3
#include <termio.h>
#include <sys/vfs.h>
#endif

/*
 * Entries from curses.h/term.h
 */
#ifdef __cplusplus
extern "C"
{
#endif
int fn (tgetent, (char *, char *));
int fn (tgetflag, (char []));
int fn (tgetnum, (char []));
#ifdef __cplusplus
}
#endif

#if SOLARIS

#ifdef __cplusplus
extern "C" 
{
#endif
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
/*
 * Missing Entries from wait.h
 */
int fn (wait3, (void *, int, void *));
#ifdef __cplusplus
}
#endif

#endif  /* SOLARIS */

#if (0)
	/* Was needed for RISCiX 1.1 */
#include <pwd.h>
char *cuserid(buf)
char *buf;
{ struct passwd *pw = getpwuid(getuid());
  strcpy(buf, pw->pw_name);
  return(buf);
}
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#ifndef PATH_MAX
#define PATH_MAX MAXPATHLEN
#endif

#ifndef L_SET
#ifdef SEEK_SET
#define L_SET  SEEK_SET
#define L_XTND SEEK_END
#else
Error : missing a header file defining either L_SET or SEEK_SET
#endif
#endif

#if (TR5 || i486V4)
#define L_XTND SEEK_END
#endif

                   /* The particular transputer site on which we are running */
int transputer_site = 0;


/******************************************************************************
*******************************************************************************
******** Routines needed by all the local modules *****************************
*******************************************************************************
******************************************************************************/

/**
*** Device initialisation and tidying routines. This is mainly the windowing
*** initialisation.
***
*** Multiple windows support. There are three cases to consider. The first
*** case is a dumb_terminal. The second is a sunview system, where multiple
*** windows must run in separate processes. The final is an X-system.
*** The code at this level is fairly simple, because it just
*** calls additional routines depending on what is happening. The X-term
*** version works in the same way as the sunview one, by forking off another
*** process to do the nasty stuff.
***
*** Obviously if only dumb terminals are supported, there is no point
*** in compiling in lots of Sunview and X support.
***
*** The ANSI emulator is rather complicated, in that different outputs have
*** to be produced for the different windowing types. In all cases the output
*** is buffered in terminal.c, and written to the window at the end of the
*** string or when the buffer is full. This eliminates the need for a
*** check_window() routine for dumb terminal, since it is incorporated into
*** dumb_send_to_window().
***
*** Input is much more complicated than before, because all three windowing
*** systems can produce their own sequences for cursor keys, function keys,
*** debugging options, window resizing, etc. All windowing input uses the
*** termcap entries to map keys onto the Helios sequences.
**/

PRIVATE void fn( convert_error, (void));

#ifdef __cplusplus
extern "C"
{
#endif
/* extern char * fn( getenv,  (char *)); */
extern char * fn( tgetstr, (char *, char **));
#ifdef __cplusplus
}
#endif
extern char **environ;

#define WindowType_Sunview  0
#define WindowType_Dumb     1
#define WindowType_X        2
int window_type = WindowType_Dumb;
extern int terminal_wraps;     /* in ANSI emulator */

#define window_max 16          /* the maximum number of windows */
typedef struct unix_window {
        WORD in_use;
        WORD cols;       /* for use with real windows */
        WORD rows;
        WORD child;      /* pipe file descriptor when using sunview windows */
        BYTE in[16];     /* input buffer whilst processing escape sequences */
        BYTE out[16];    /* ditto */
        WORD matching;   /* the index of the escape sequence I am currently */
                         /* trying to match */
        int  how_many;
        int  buf_index;
        UBYTE buf[64];
} unix_window;
PRIVATE unix_window window_tab[window_max];

#ifdef __cplusplus
extern "C"
{
#endif
PRIVATE void fn( initialise_dumb_terminal,        (void));
PRIVATE void fn( restore_dumb_terminal,           (void));
PRIVATE unix_window *fn( dumb_create_window,      (char *));
PRIVATE void fn( dumb_close_window,               (unix_window *));
PRIVATE void fn( dumb_window_size,    (unix_window *, word *, word *));
PRIVATE WORD fn( dumb_read_char_from_keyboard,    (unix_window *));
PRIVATE void fn( dumb_send_to_window,             (char *, unix_window *));
#ifdef __cplusplus
}
#endif

#if use_separate_windows
PRIVATE char serverwin_name[256];
PRIVATE int its_log_dest   = Log_to_screen;
PRIVATE int its_debugflags = 0;

#ifdef __cplusplus
extern "C"
{
#endif
PRIVATE void fn( initialise_sunview_terminal,     (void));
PRIVATE void fn( restore_sunview_terminal,        (void));
PRIVATE unix_window *fn( sunview_create_window,   (char *));
PRIVATE void fn( sunview_close_window,            (unix_window *));
PRIVATE void fn( sunview_window_size,         (unix_window *, word *, word *));
PRIVATE WORD fn( sunview_read_char_from_keyboard, (unix_window *));
PRIVATE void fn( sunview_send_to_window,          (char *, unix_window *));
#ifdef __cplusplus
}
#endif

#endif	/* use_separate_windows */

#ifdef __cplusplus
typedef void	(*VoidCharFnPtr)(char *, ...);
typedef void	(*VoidWordargFnPtr)(word *, ...);
typedef word	(*WordCharFnPtr)(char *, ...);
typedef word	(*WordWordFnPtr)(word *, ...);
#else
typedef VoidFnPtr	VoidWordargFnPtr;
typedef VoidFnPtr	VoidCharFnPtr;
typedef WordFnPtr	WordCharFnPtr;
typedef WordFnPtr	WordWordFnPtr;
#endif

PRIVATE VoidWordargFnPtr window_size_fn;
PRIVATE VoidWordFnPtr close_window_fn;
PRIVATE WordWordFnPtr read_char_fn;
PRIVATE VoidCharFnPtr send_window_fn;
PRIVATE WordCharFnPtr open_window_fn;

#ifdef __cplusplus
extern "C"
{
#endif
PRIVATE void fn( initialise_keyboard_strings, (void));
PRIVATE void fn( switch_window, (int));
PRIVATE WORD fn( extract_from_window, (unix_window *));
PRIVATE void fn( initialise_signals, (void));

PRIVATE void fn(initialise_etc_directory, (void));
#ifdef __cplusplus
}
#endif

BYTE termcap_data[1024];
BYTE termcap_buffer[1024], *termcap_index;
PRIVATE string null_string = "";
string termcap_cls, termcap_ceol, termcap_move, termcap_inv, termcap_nor;
/*@@@ */
string termcap_bell, termcap_ks, termcap_ke;

void unix_initialise_devices()
{ char *term_env = getenv("TERM"), term[128];
  int  x;

#if ETC_DIR
  initialise_etc_directory();
#endif

  if (term_env eq (char *) NULL)
   { printf(" Helios: unable to get TERM environment variable.\n");
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

  switch(tgetent(termcap_data, term_env))
   { case -1 : printf("Unable to open termcap database.\n");
/*               longjmp(exit_jmpbuf, 1); */
               longjmp_exit;
     case  0 : printf("Terminal %s not found in termcap database.\n", term_env);
/*               longjmp(exit_jmpbuf, 1); */
               longjmp_exit;
   }   

  termcap_index = termcap_buffer;
  termcap_cls   = tgetstr("cl", &termcap_index);
  termcap_ceol  = tgetstr("ce", &termcap_index);
  termcap_move  = tgetstr("cm", &termcap_index);
  termcap_bell  = tgetstr("bl", &termcap_index);
  /*@@@*/
  termcap_ks	= tgetstr("ks", &termcap_index);
  termcap_ke	= tgetstr("ke", &termcap_index);
  termcap_inv   = tgetstr("mr", &termcap_index);
  termcap_nor   = tgetstr("me", &termcap_index);
  if ((termcap_inv eq NULL) || (termcap_nor eq NULL))  
   { termcap_inv = tgetstr("so", &termcap_index);
     termcap_nor = tgetstr("se", &termcap_index);
   }

  if (termcap_cls  eq NULL) termcap_cls  = null_string;
  if (termcap_move eq NULL) termcap_move = null_string;
  if (termcap_bell eq NULL) termcap_bell = "\007\0";

  if (termcap_inv  eq NULL) termcap_inv  = null_string;
  if (termcap_nor  eq NULL)
   { termcap_nor  = null_string;
     termcap_inv  = null_string;
   }

  while (isdigit(*termcap_cls))  termcap_cls++; /* strip out padding */
  if (termcap_ceol ne NULL)
    while (isdigit(*termcap_ceol)) termcap_ceol++;
  while (isdigit(*termcap_move)) termcap_move++;
  while (isdigit(*termcap_bell)) termcap_bell++;
  while (isdigit(*termcap_inv))  termcap_inv++;
  while (isdigit(*termcap_nor))  termcap_nor++;

  if (tgetflag("am"))
   terminal_wraps = 1;

  for (x = 0; x < window_max; x++) 
   { window_tab[x].in_use     = FALSE;
     window_tab[x].child      = -1;
     window_tab[x].in[0]      = '\0';
     window_tab[x].out[0]     = '\0';
     window_tab[x].matching   = -1;
     window_tab[x].how_many   = 0;
     window_tab[x].buf_index  = -1;
   }

  strcpy(term, term_env);      /* make sure there is plenty of space */
 
#if use_separate_windows

/*  x = term[5]; term[5] = '\0'; */
  if (strncmp(term, "xterm", 5) == 0 || strncmp (term, "sun-cmd", 7) == 0)
    {
      window_type = WindowType_X;
    }
/*  term[5] = x; */

#if (SUN)
  if (window_type != WindowType_X)
#if SOLARIS
    {
      window_type = WindowType_Sunview;
    }
#else
    if (getenv("WINDOW_PARENT"))	/* Open Look special	*/
      {
	window_type = WindowType_Sunview;	
      }
#endif
#endif

  if (window_type ne WindowType_Dumb)
   { struct stat buf;
     char        *config = get_config("serverwindow");

     if (config eq NULL)
       {
	 printf ("Failed to find serverwindow - window type is dumb\n");
	 window_type = WindowType_Dumb;
       }
     else
      {
        strcpy(serverwin_name, config);
        if (stat(serverwin_name, &buf) eq -1)
	 { printf("Cannot find window program %s, defaulting to dumb terminal mode\n",
		serverwin_name);
	   window_type = WindowType_Dumb;
         }
      }
   }
#endif

  initialise_keyboard_strings();

  initialise_signals();

#if use_separate_windows
  if ((window_type eq WindowType_Sunview) || (window_type eq WindowType_X))
   {
     real_windows = 1;
	/* Redefine termcap_cls to work with both X and Sunview	*/
     { static char buf[10];
       strcpy(buf, "x[Hx[2J");
       buf[0] = 0x1B; buf[3] = 0x1B;
       termcap_cls = buf;
     }

     initialise_sunview_terminal();

     close_window_fn = (VoidWordFnPtr)func(sunview_close_window);
     window_size_fn  = (VoidWordargFnPtr)func(sunview_window_size);
     read_char_fn    = (WordWordFnPtr)func(sunview_read_char_from_keyboard);
     open_window_fn  = (WordCharFnPtr)func(sunview_create_window);
     send_window_fn  = (VoidCharFnPtr)func(sunview_send_to_window);

     return;
   }
#endif

  real_windows = 0;
  initialise_dumb_terminal();
  close_window_fn = (VoidWordFnPtr)func(dumb_close_window);
  window_size_fn  = (VoidWordargFnPtr)func(dumb_window_size);
  read_char_fn    = (WordWordFnPtr)func(dumb_read_char_from_keyboard);
  open_window_fn  = (WordCharFnPtr)func(dumb_create_window);
  send_window_fn  = (VoidCharFnPtr)func(dumb_send_to_window);
}

void unix_restore_devices()
{ switch(window_type)
   { case WindowType_Dumb    : restore_dumb_terminal(); break;
#if use_separate_windows
     case WindowType_Sunview :
     case WindowType_X       : restore_sunview_terminal(); break;
#endif
   }
}

void send_to_window(data, handle)
char *data;
Window *handle;
{
  (*send_window_fn)(data, handle);
}

WORD create_a_window(name)
char *name;
{ return( (*open_window_fn)(name));
}

void close_window(handle)
WORD handle;
{ (*close_window_fn)((word *)handle); 
} 

void window_size(handle, x, y)
WORD handle;
WORD *x, *y;
{ (*window_size_fn)((word *)handle, x, y); 
}

int read_char_from_keyboard(andle)
WORD andle;
{ unix_window *handle = (unix_window *) andle;

  if (handle->out[0] ne '\0')
   return(extract_from_window(handle));
 
  return( (*read_char_fn)((word *)handle));
}

/**
*** Keyboard handling, this is really jukky !!!
*** I must be able to interpret arbitrary key sequences, TERMCAP function key
*** codes and the like, and translate them to the Helios standard. In addition
*** I must be able to interpret special keys for debugging purposes, e.g. to
*** reboot and to switch on debugging. These special keys are introduced by
*** an escape sequence which must be provided by the host.con file, either as
*** a TERMCAP name or as a termcap-style entry preceeded by '#'. The escape
*** sequence is followed by 0 for reboot, 9 for exit, m for message debugging
*** etc. Also 1 for switch window forwards and 2 for backwards. 
***
*** All the special sequences are held in the table key_table[],
*** fully sorted. All the keys defined by both Helios and termcap
*** are held in a second table termcap_table, and the initialise_keyboard()
*** routine goes through this second table putting all known keys into the
*** key definition table, with both the local and the Helios sequences and a
*** flag to determine whether or not it is a debugging key.
***
*** Finally initialise_keyboard_strings() puts all the debugging sequences into
*** the keydefinition table and sorts the table.
**/

#ifdef __cplusplus
extern "C"
{
#endif
PRIVATE BYTE *fn( parse_escape_sequence, (char *));
PRIVATE void fn( add_debug_sequence,     (char *, char, int));
#ifdef __cplusplus
}
#endif

typedef struct key_definition {
        char     local_sequence[16];
        char     *helios_sequence;
        int      debugging_option;
} key_definition;

#ifdef __cplusplus
typedef int	(*IntConstVoidFnPtr)(const void *, const void *);

/* void qsort (void *, int, int, IntKeyFnPtr); */
#define qsort_(a,b,c,d)	((void *)a, (int)b, (int)c, (IntConstVoidFnPtr)d)

#else

#define qsort_(a,b,c,d)	qsort(a,b,c,d)

#endif


PRIVATE int key_comp(a, b)
key_definition *a, *b;
{ register char *s1, *s2;
  s1 = a->local_sequence; s2 = b->local_sequence;
  for ( ; (*s1 ne '\0') && (*s2 ne '\0'); s1++, s2++)
   { if (*s1 < *s2) return(-1);
     if (*s1 > *s2) return(1);
   }
  if (*s1 ne '\0') return(-1);  /* longer string takes priority */
  if (*s2 ne '\0') return(1);

  if (a->debugging_option ne 0)  /* for identical strings, debugging takes */
   return(-1);                   /* priority */
  if (b->debugging_option ne 0)
   return(1);

  return(0); 
}

PRIVATE key_definition key_table[100];
PRIVATE int keyindex_max= 0;

#if (TR5 || i486V4)
typedef struct termcap_definition {
        unsigned	char     termcap_name[4];
        unsigned	char     helios_sequence[6];
} termcap_definition;
#else
typedef struct termcap_definition {
        char     termcap_name[4];
        char     helios_sequence[6];
} termcap_definition;
#endif

PRIVATE termcap_definition termcap_table[22] =
{ { "k1", { 0x9B, '0', '~', '\0' } },      /* function key 1 */
  { "k2", { 0x9B, '1', '~', '\0' } },      /* F2 */
  { "k3", { 0x9B, '2', '~', '\0' } },      /* F3 */
  { "k4", { 0x9B, '3', '~', '\0' } },      /* F4 */
  { "k5", { 0x9B, '4', '~', '\0' } },      /* F5 */
  { "k6", { 0x9B, '5', '~', '\0' } },      /* F6 */
  { "k7", { 0x9B, '6', '~', '\0' } },      /* F7 */
  { "k8", { 0x9B, '7', '~', '\0' } },      /* F8 */
  { "k9", { 0x9B, '8', '~', '\0' } },      /* F9 */
  { "k;", { 0x9B, '9', '~', '\0' } },      /* F10 */
  { "&8", { 0x9B, '1', 'z', '\0' } },      /* UNDO */
  { "@7", { 0x9B, '2', 'z', '\0' } },      /* END */
  { "kI", { 0x9B, '@', '\0' } },           /* INSERT */
  { "kN", { 0x9B, '4', 'z', '\0' } },      /* Page down */
  { "kP", { 0x9B, '3', 'z', '\0' } },      /* Page up */
  { "kh", { 0x9B, 'H', '\0' } },           /* home */
  { "kd", { 0x9B, 'B', '\0' } },           /* down arrow */
  { "kl", { 0x9B, 'D', '\0' } },           /* left arrow */
  { "kr", { 0x9B, 'C', '\0' } },           /* right arrow */
  { "ku", { 0x9B, 'A', '\0' } },           /* up arrow */
  { "%1", { 0x9B, '?', '~', '\0' } },      /* help key */
  { "", "" }
};


PRIVATE void initialise_keyboard_strings()
{ termcap_definition *def = termcap_table;
  char *temp;

  memset(key_table, 0, 100 * sizeof(key_definition));

  for ( ; def->termcap_name[0] ne '\0'; def++)
   { temp = tgetstr(def->termcap_name, &termcap_index);
     if (temp ne NULL)
      { key_definition *key = &(key_table[keyindex_max++]);
          /* Undocumented magic */
        if ((temp[0] eq 0x1B) && (temp[1] eq 'O'))
           temp[1] = 0x5B;
        strcpy(key->local_sequence, temp);
        key->helios_sequence  = def->helios_sequence;
        key->debugging_option = 0; 
      }
   }

	/* Problems with the return key. This should always generate	*/
	/* a carriage return ^M, 0x0D, since the I/O Server will 	*/
	/* do the necessary translations to linefeeds if appropriate.	*/
	/* In theory using the INLCR termios option will take care of	*/
	/* that but it is unreliable. Instead I add an automatic	*/
	/* translation here. This is unfortunate in that it is no longer*/
	/* possible to use the ctrl-J key, but apparently unavoidable	*/
     { key_definition *key;
       key = &(key_table[keyindex_max++]);
       strcpy(key->local_sequence, "\12\0");
       key->helios_sequence   = "\15\0";
       key->debugging_option  = 0;
     }

/**
*** WARNING : different C compilers appear to disagree about how to
*** represent octal numbers within strings. I am used to
*** "\003" for the octal number. However, the Sun C compiler interprets
*** this as '\0' followed by the digits 0 and 3. This may screw up some
*** of the stuff below.
**/

#if (SUN3 || SUN4 || SUN386 || TR5 || i486V4 || HP9000)
                     /* The various suns support the termios system */
                     /* but other unix machines may not. Those have */
                     /* to make do with the sgtty junk              */
  { struct termios t;
    if (ioctl(0, TCGETS, &t) eq 0)
     { key_definition *key;
       key = &(key_table[keyindex_max++]);
       key->local_sequence[0] = t.c_cc[VERASE];
       key->helios_sequence   = "\10\0";
       key->debugging_option  = 0;

       if ((t.c_cc[VINTR] ne 0x03) && (t.c_cc[VINTR] ne '\0'))   /* ctrl-C */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.c_cc[VINTR];
          key->helios_sequence   = "\3\0";
          key->debugging_option  = 0;
        }       
       if ((t.c_cc[VEOF] ne 0x04) && (t.c_cc[VEOF] ne '\0')) /* ctrl-D */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.c_cc[VEOF];
          key->helios_sequence   = "\4\0";
          key->debugging_option  = 0;
        }       
       if ((t.c_cc[VSTOP] ne 0x13) && (t.c_cc[VSTOP] ne '\0')) /* ctrl-S */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.c_cc[VSTOP];
          key->helios_sequence   = "\23\0";
          key->debugging_option  = 0;
        }       
       if ((t.c_cc[VSTART] ne 0x11) && (t.c_cc[VSTART] ne '\0')) /* ctrl-Q */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.c_cc[VSTART];
          key->helios_sequence   = "\21\0";
          key->debugging_option  = 0;
        }       
     }
  }

#else

#if !(SCOUNIX)

  { struct sgttyb t;
    if (ioctl(0, TIOCGETP, &t) eq 0)
     { key_definition *key = &(key_table[keyindex_max++]);
       key->local_sequence[0] = t.sg_erase;
       key->helios_sequence   = "\10\0";
       key->debugging_option  = 0;
     }
  }

  { struct tchars t;
    if (ioctl(0, TIOCGETC, &t) eq 0)
     { key_definition *key;
       if (t.t_intrc ne 0x03)   /* interrupt not ctrl-C */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.t_intrc;
          key->helios_sequence   = "\3\0";
          key->debugging_option  = 0;
        }
       if (t.t_startc ne 0x11)  /* ctrl-Q */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.t_startc;
          key->helios_sequence   = "\21\0";
          key->debugging_option  = 0;
        }
       if (t.t_stopc ne 0x13)  /* ctrl-S */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.t_stopc;
          key->helios_sequence   = "\23\0";
          key->debugging_option  = 0;
        }
       if (t.t_eofc ne 0x04)  /* ctrl-D */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.t_eofc;
          key->helios_sequence   = "\4\0";
          key->debugging_option  = 0;
        }
     }
  }
#endif

#endif

  if (window_type ne WindowType_Dumb) goto skip_sequences;
 
  temp = parse_escape_sequence("escape_sequence");
  if (temp eq NULL)
  { printf("An escape_sequence must be defined in the configuration file.\r\n");
    printf("Please edit the file host.con.\n");
/*    longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
  }

  if (temp ne NULL)      /* a sequence is defined */
   { int i;
     add_debug_sequence(temp, '0', -1);   /* reboot */
     add_debug_sequence(temp, '9', -2);   /* exit */
     add_debug_sequence(temp, '8', -3);   /* status */
     add_debug_sequence(temp, '7', -4);   /* debug */
     add_debug_sequence(temp, '2', -5);   /* switch back */
     add_debug_sequence(temp, '1', -6);   /* switch forward */
     add_debug_sequence(temp, 'a', -7);   /* toggle all debugging */
     add_debug_sequence(temp, '3', -8);   /* refresh current window */
     for (i = 0; options_list[i].flagchar ne '\0'; i++)
      add_debug_sequence(temp, options_list[i].flagchar, options_list[i].flag);
   }

  if ((temp = parse_escape_sequence("switch_forwards_key")) ne NULL)
     add_debug_sequence(temp, '\0', -6);
  if ((temp = parse_escape_sequence("switch_backwards_key")) ne NULL)
     add_debug_sequence(temp, '\0', -5);
  if ((temp = parse_escape_sequence("refresh_key")) ne NULL)
     add_debug_sequence(temp, '\0', -8);
  if ((temp = parse_escape_sequence("debugger_key")) ne NULL)
     add_debug_sequence(temp, '\0', -4);
  if ((temp = parse_escape_sequence("status_key")) ne NULL)
     add_debug_sequence(temp, '\0', -3);
  if ((temp = parse_escape_sequence("exit_key")) ne NULL)
     add_debug_sequence(temp, '\0', -2);
  if ((temp = parse_escape_sequence("reboot_key")) ne NULL)
     add_debug_sequence(temp, '\0', -1);

skip_sequences:

  qsort_(key_table, keyindex_max, sizeof(key_definition), key_comp); 

#ifdef NEVER

{ int i;
  for (i = 0; i < keyindex_max; i++)
  {
    printf("key_table[%d] = %x (%c) | %x (%c) | %x (%c) | %x (%c) | debug %d\n", 
    i,
    key_table[i].local_sequence[0],
    key_table[i].local_sequence[0],
    key_table[i].local_sequence[1],
    key_table[i].local_sequence[1],
    key_table[i].local_sequence[2],
    key_table[i].local_sequence[2],
    key_table[i].local_sequence[3],
    key_table[i].local_sequence[3],
    key_table[i].debugging_option);

    sleep (2);
  }

  sleep(30);
}
#endif

}

PRIVATE void add_debug_sequence(str, ch, flag)
char *str;
char ch;
int flag;
{ key_definition *key = &(key_table[keyindex_max++]); 
  char buf[2];

  strcpy(key->local_sequence, str);
  buf[0] = ch; buf[1] = '\0';
  strcat(key->local_sequence, buf);
  key->helios_sequence                 = NULL;
  key->debugging_option                = flag;
}

/**
*** This routine parses escape sequences defined in host.con for the various
*** special keys such as the window switching keys. The configuration name
*** is supplied, and if the entry exists it is parsed. Entries can be either
*** termcap names such as k1 or kd, or termcap-style entries introduced by
*** a '#'. If the latter, I try to parse it and store the result in the
*** termcap array, using termcap_index etc., just like tgetstr().
**/

PRIVATE char *parse_escape_sequence(name)
char *name;
{ char *entry, *dest;

  entry = get_config(name);   /* none defined */
  if (entry eq (char *) NULL)
   return(NULL);
 
  if (*entry eq '@')                   /* specified in config */
   { int parse_fail = 0;
     entry++;
     for (dest = termcap_index; *entry ne '\0'; entry++)
      switch(*entry) 
       { case '\0' : entry--; parse_fail++; break;  /* safety net */

         case '^'  : entry++;         /* control key */
                     if (*entry eq '\0') { parse_fail++; entry--; break; }
                     *dest++ = (*entry - '@'); break;

         case '\\' : entry++;        /* escape of some sort */
                     switch(*entry)
                     { case '\0' : parse_fail++; entry--; break; 
                       case 'E'  : *dest++ = 0x1B; break;
                       case '\\' : *dest++ = '\\'; break;
                       case '^'  : *dest++ = '^'; break;
                       case 'n'  : *dest++ = '\n'; break;
                       case 'r'  : *dest++ = '\r'; break;
                       case 't'  : *dest++ = '\t'; break;
                       case 'b'  : *dest++ = '\b'; break;
                       case 'f'  : *dest++ = '\f'; break;
                       case '0'  :
                       case '1'  :
                       case '2'  :            /* octal number */
                       case '3'  : { int x;
                                     x = (*entry - '0') * 64;
                                     entry++;
                                     if (!isdigit(*entry))
                                      { parse_fail++; entry--; break; }
                                     x += 8 * (*entry - '0');
                                     entry++;
                                     if (!isdigit(*entry)) 
                                      { parse_fail++; entry--; break; }
                                     x += (*entry - '0');
                                     *dest++ = x;
                                     break;
                                  }
                       default : parse_fail++; break;
                    }
                   break;
         default : *dest++ = *entry;
       }
     if (parse_fail)
       { printf("Failed to parse escape sequence %s in host.con file.\r\n",
                 name);
/*         longjmp(exit_jmpbuf, 1); */
	 longjmp_exit;
       }
     else
      { entry = termcap_index;  /* start of parsed string */
        *dest++ = '\0';         /* terminate string */
        termcap_index = dest;   /* update index */
        return(entry);
      }
   }

  if ((dest = tgetstr(entry, &termcap_index)) eq NULL)
   { printf("Error in configuration entry for %s\r\n", name);
     printf("Termcap entry %s not defined for this terminal.\r\n", entry);
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

  return(dest);
}
 
/**
*** Code for dealing with data typed into a window, translating special key
*** sequences and the like. Data arrives via routine add_to_window(). If the
*** window is currently in an escape sequence the character is added to the
*** buffer, and I check through the table to see if the complete sequence has
*** arrived. If the window is not in an escape sequence then the character may
*** be the start of an escape sequence and I have to start matching. Once an
*** entire sequence has been recognised the Helios output sequence is buffered
*** up, and read_char_from_keyboard() checks this output buffer. Data is
*** extracted from this buffer by the extract_from_window() routine.
**/

PRIVATE WORD extract_from_window(handle)
unix_window *handle;
{ WORD x = handle->out[0];
  BYTE *s1, *s2;

  s1 = s2 = handle->out;
  for (s2++; *s1 ne '\0'; ) *s1++ = *s2++;

  if (x eq 0)
   return(-1);
  else
   return(x);
}

PRIVATE void fn( found_match, (unix_window *, int));

PRIVATE WORD add_to_window(ch, handle)
WORD ch;
unix_window *handle;
{ int i;
  BYTE *in  = handle->in;
  BYTE *out = handle->out;

  if (ch eq 0)
   { if (in[0] eq '\0')
      return(-1);
     else
      goto match;
   }

  if (handle->matching eq -1)   /* not currently trying to match anything */
   { if ((ch < key_table[0].local_sequence[0]) ||
         (ch > key_table[keyindex_max-1].local_sequence[0]) )
        return(ch);
     in[0] = ch; in[1] = '\0'; handle->matching = 0;
   }
  else                          /* partly through matching */
   { char *temp;
     for (temp = in; *temp ne '\0'; temp++);
     *temp++ = ch; *temp = '\0';
   }

match:

                    /* try to match the current input */
  for (i = handle->matching; i < keyindex_max; i++)
   { char *input, *pattern;
     input = in; pattern = key_table[i].local_sequence;

     if (*input < *pattern)  /* because the key sequences are sorted the */
       break;                /* matching has failed */

     for ( ; (*input ne '\0') && (*pattern ne '\0'); input++, pattern++)
      if (*input ne *pattern)
         goto next;   /* does not match */

     if (*pattern ne '\0')    /* end of buffer but not of pattern */
      { handle->matching = i; return(-1); }

     found_match(handle, i);

     if (*input ne '\0')   /* end of pattern but not of buffer */
      { char *s2 = in;
        for ( ; *input ne '\0'; s2++, input++) *s2 = *input;  /* clear buffer */
        *s2 = '\0';
        handle->matching = 0;
      } 
     else
      { in[0] = '\0';
        handle->matching = -1;
      }

     if (out[0] ne '\0') return( extract_from_window(handle));
     if (handle->matching eq -1) return(-1);  /* nothing left in buffer */
     
next:
     continue;
   }
                    /* Failed to match, so I know that the first character */
                    /* is not part of an escape sequence */
  { WORD x = in[0];
    BYTE *s1, *s2;
    s1 = s2 = in;
    for (s2++; *s1 ne '\0'; s1++, s2++) *s1 = *s2;
    if (in[0] eq '\0')
     handle->matching = -1;    /* nothing left in buffer */
    else
     handle->matching = 0;     /* remainder of buffer may match */

    return(x);
  } 
}

PRIVATE void found_match(handle, i)
unix_window *handle;
int i;
{ key_definition *entry = &(key_table[i]);

  if (!entry->debugging_option)
    strcpy(handle->out, entry->helios_sequence);
  else
   { switch (entry->debugging_option)
      { case -1  : Special_Reboot = 1; break;
        case -2  : Special_Exit   = 1; break;
        case -3  : Special_Status = 1; break;
        case -4  : DebugMode = 1 - DebugMode; break;
        case -5  : switch_window(0); break;
        case -6  : switch_window(1); break;
        case -7  : if (!debugflags)
                    debugflags = All_Debug_Flags; 
                   else
                    debugflags = 0;
                   break;
        case -8  : switch_window(-1); break;
 
        default  : debugflags ^= entry->debugging_option;
      }
   }
}

/**
*** writing to and reading from sockets. I know at all times how much data to
*** read from a socket, and the other side knows how much I am writing. Hence
*** the following code guarantees delivery of that amount of data,
*** and who cares about the socket's buffering.
**/

PRIVATE void pipe_broken(name)
char *name;
{ PRIVATE int pipe_error = 0;

  if (!pipe_error)
   { printf("FATAL ERROR : pipe to %s has been broken (%d).\r\n", name, errno);
     printf("Attempting to exit safely.\r\n");
     Special_Exit = TRUE;
     pipe_error = 1;
     return;
   }
}

#if ANSI_prototypes
void socket_write (int fildes, BYTE * buf, int amount, char * name)
#else
void socket_write(fildes, buf, amount, name)
int fildes;
BYTE *buf;
int amount;
char *name;
#endif
{ int written = 0;
  int signals = 0;

  while (written < amount)
   { int x = write(fildes, &(buf[written]), amount - written);
     if (x <= 0)
      { if ((errno eq EINTR) && (signals < 5))
         { signals++; continue; }
        pipe_broken(name);
        return;
      }
     written += x;
   }
}

#if ANSI_prototypes
void socket_read (int fildes, BYTE * buf, int amount, char *name)
#else
void socket_read(fildes, buf, amount, name)
int fildes;
BYTE *buf;
int  amount;
char *name;
#endif
{ int read_so_far = 0;
  int signals = 0;

  while (read_so_far < amount)
   { int x = read(fildes, &(buf[read_so_far]), amount - read_so_far);
     if (x <= 0)
      { if ((errno eq EINTR) && (signals < 5))
         { signals++; continue; }
        pipe_broken(name);
        return;
      }
     read_so_far += x;
   }
}

/**
*** Dumb terminal support : this is fairly easy, most of the code being the
*** same as on the PC and ST versions of ServerWindows. The difference is
*** that check_windows has been eliminated, and the window-switching test
*** has been moved to dumb_send_to_window.
**/

#if (SUN3 || SUN4 || SUN386 || TR5 || i486V4 || HP9000 || SCOUNIX)
PRIVATE struct termios old_term, cur_term;
#else
PRIVATE struct sgttyb old_term, cur_term;
#endif
PRIVATE int    terminal_changed = 0;
PRIVATE int    window_lines, window_cols;

#define Server_handle  (&(window_tab[0]))
unix_window *current_handle;
PRIVATE WORD nopop;
extern  void fn( redraw_screen, (Window *, WORD));
extern  int  ANSI_buffer_count;
 
PRIVATE void initialise_dumb_terminal()
{ 
#if ARMBSD

  dumb_window_size(NULL, &window_cols, &window_lines);

#else

  window_lines = tgetnum("li");
  window_cols  = tgetnum("co");

#endif

/*@@@*/
/*
  if (!isatty(0) || !isatty(1))
  {
    printf("Standard streams may not be redirected.\n");
#ifdef NEVER
        longjmp(exit_jmpbuf, 1);
#endif
    longjmp_exit;
   }
*/

#if (SUN3 || SUN4 || SUN386 || TR5 || i486V4 || HP9000 || SCOUNIX)

  ioctl(0, TCGETS, &old_term);
  ioctl(0, TCGETS, &cur_term);
  cur_term.c_iflag &= ~(IXON + IXOFF);
  cur_term.c_iflag |= (IGNBRK | IGNPAR | INLCR); 
  cur_term.c_lflag &= ~(ISIG + ICANON + ECHO + ECHOE + ECHOK + 
                        ECHONL);
  cur_term.c_cc[VMIN]  = 0;
  cur_term.c_cc[VTIME] = 0;
  ioctl(0, TCSETS, &cur_term);

#else
                            /* set mode RAW ,ctrl \ gets out */
  ioctl(0,TIOCGETP,&old_term);
  ioctl(0,TIOCGETP,&cur_term);
  cur_term.sg_flags &=(~ECHO);     /* no echo */
  cur_term.sg_flags |=RAW;         /* raw */
  cur_term.sg_flags &=(~CRMOD);
  ioctl(0,TIOCSETP,&cur_term);

#endif
/*@@@*/
  /* set terminal to application mode */
  if (termcap_ks) printf("%s",termcap_ks);
  terminal_changed = 1;

  { char *text = get_config("SERVER_WINDOWS_NOPOP");
    if (text eq (char *) NULL)
     nopop = 0;
    else
     nopop = 1;
  }
}

PRIVATE void restore_dumb_terminal()
{
  if (terminal_changed)
#if (SUN3 || SUN4 || SUN386 || TR5 || i486V4 || HP9000 || SCOUNIX)
   ioctl(0, TCSETSW, &old_term); 
#else
   ioctl(0, TIOCSETP, &old_term);
/*@@@*/
  /* reset terminal to normal mode */
  if (termcap_ke) printf("%s",termcap_ke);
#endif

#if (SUN3 || SUN4 || SUN386 || TR5 || i486V4 || HP9000 || SCOUNIX)
  ioctl(1, TCSBRK, 100);  /* drain the output */
#endif
}

PRIVATE Window *find_window(handle)
unix_window *handle;
{ Window *window;

  if (handle eq Server_handle) return(&Server_window);

  for (window = (Window *) Window_List.list.head;
       window->node.node.next ne (Node *) NULL;
       window = (Window *) window->node.node.next)
   if (window->handle eq (WORD) handle)
    return(window);

  return((Window *) NULL);
}

PRIVATE unix_window *dumb_create_window(name)
char *name;
{ int i;
  for (i = 0; i < window_max; i++)
   if (!window_tab[i].in_use)
    { window_tab[i].in_use   = 1;
      window_tab[i].in[0]    = '\0';
      window_tab[i].out[0]   = '\0';
      window_tab[i].matching = -1;
      current_handle         = &(window_tab[i]);
      return(&(window_tab[i]));
    }

  use(name)
  return((unix_window *) NULL);
}

PRIVATE void dumb_close_window(handle)
unix_window *handle;
{ 
  if ((handle eq current_handle) && (handle ne Server_handle))
   { if (Special_Exit)
      { current_handle = Server_handle;
        redraw_screen(&Server_window, fileno(stdout));
      }
     else
      switch_window(0);
   }
  handle->in_use = 0;
}

PRIVATE void dumb_window_size(handle, x, y)
unix_window *handle;
WORD *x, *y;
{
#if ARMBSD

 struct winsize windz;
 ioctl(fileno(stdin), TIOCGWINSZ, &windz);
 *x = (int)windz.ws_col; *y = (int)windz.ws_row;

#else

 *x = window_cols; *y = window_lines;

#endif

 use(handle)
}

PRIVATE void switch_window(direction)
int direction;
{ Window *window;
 
  if (window_type ne WindowType_Dumb) return;

  if (direction eq -1)   /* redraw current window */
   { window = find_window(current_handle);
     if (window eq (Window *) NULL)
      return;
     goto redraw;
   }

  if (current_handle eq Server_handle)
   { if (direction)
      window = (Window *) Window_List.list.head;
     else
      window = (Window *) Window_List.list.tail;
   }
  else
   { window = find_window(current_handle);
     if (window eq (Window *) NULL)
      window = &Server_window;
     else
      { if (direction)
         { window = (Window *) window->node.node.next;
           if (window eq (Window *) NULL)
           window = &Server_window;
         }
        else
          window = (Window *) window->node.node.prev;
      }
    }

  if (window ne &Server_window)
   if ( (window->node.node.next eq (Node *) NULL) || 
        (window->node.node.prev eq (Node *) NULL) )
    window = &Server_window;

redraw:

  current_handle = (unix_window *) window->handle;
  redraw_screen(window, fileno(stdout));
}


PRIVATE void dumb_send_to_window(data, handle)
BYTE *data;
unix_window *handle;
{ if (handle ne current_handle)
   { if (nopop) return;
     if (handle eq Server_handle)
      { current_handle = Server_handle;
        redraw_screen(&Server_window, fileno(stdout));
      }
     else
      return;
   } 
  socket_write(1, data, ANSI_buffer_count, "window"); 
}

PRIVATE WORD dumb_read_char_from_keyboard(handle)
unix_window *handle;
{ PRIVATE int  how_many = 0, index = -1;
  PRIVATE BYTE buf[64];

  if (handle ne current_handle)
   { 
      if (handle eq Server_handle)
      { Window *window = find_window(current_handle);
        if (window ne (Window *) NULL)
         window->any_data = CoReady;
      } 
      return(-1);
   }

  if ((how_many eq 0) && (index eq -1))
   { how_many = read(0, buf, 64);
     index = 0;
   }

                  /* the escape key deserves a special case      */
                  /* A single escape character may introduce an  */
                  /* escape sequence or it may be the escape key */
                  /* To distinguish between them, wait 1/4 sec   */
                  /* for any more data, and if there is none     */
                  /* it really is the escape key. To achieve the */
                  /* required granularity I need to use select().*/
  if ((how_many eq 1) && (buf[0] eq 0x1b) && (handle->matching eq -1)) 
  { fd_set t_rd_mask;
    struct timeval timelim;

    FD_ZERO(&t_rd_mask);
    FD_SET(0, &t_rd_mask);
#if SM90
    timelim.tv_sec = 1;
#else
    timelim.tv_sec = 0;
#endif
    timelim.tv_usec = 250 * 1000;   /* 1/4 second, fine even at 50 baud */
    if (select(1, &t_rd_mask, NULL, NULL, &timelim) <= 0)
     {
       index = 0; 
       how_many = 0;
       return(0x1B);
     }
  }

  while (how_many > 0) 
    { int x = add_to_window(buf[index++], handle);
      how_many--; 
      if (x ne -1) 
	return(x);      
    } 

  { int x = add_to_window(0, handle);
    if (x eq -1)
     index = -1;
    return(x);
  }
}

/**
*** Sunview and X support
**/

#if use_separate_windows

PRIVATE void initialise_sunview_terminal()
{
}

PRIVATE void restore_sunview_terminal()
{
}

PRIVATE unix_window *sunview_create_window(name)
char *name;
{ int pipes[2];
  char child[6];
  int pid;
  int i;
  char *args[6];

  for (i = 0; i < window_max; i++)
   if (!window_tab[i].in_use)
    break;
  if (window_tab[i].in_use) return(NULL);

  window_tab[i].in_use    = 1;
  window_tab[i].rows      = 31;   /* BLV - this is the important value */
  window_tab[i].cols      = 79;
  window_tab[i].in[0]     = '\0';
  window_tab[i].out[0]    = '\0';
  window_tab[i].matching  = -1;
  window_tab[i].how_many  = 0;
  window_tab[i].buf_index = -1;
  
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, pipes) eq -1)
   { 
     window_tab[i].in_use = 0;
     return((unix_window *) NULL);
   }
  window_tab[i].child = pipes[1];

  sprintf(child, "%d", pipes[0]);

  args[0] = serverwin_name;

  if (i eq 0)  /* creating the server window */
   { 
     args[2] = "1";
#if debugger_incorporated
     args[4] = "1";
#else
     args[4] = "0";
#endif
   }
  else
   { args[2] = "0";
     args[4] = "0";
   }

  args[1]    = name;
  args[3]    = child;
  args[5]    = NULL;

  fcntl(pipes[1], F_SETFD, 1);

#if SOLARIS
  pid = fork1();
#else
  pid = vfork();
#endif

  if (!pid)
   { 
     execve(args[0], args, environ);
     printf("Internal error : execve of window failed with %d\n", errno); 
     fprintf(stderr, "Internal error : execve of window failed with %d\n", errno); 
     perror ("execve failed");
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }

/*  sleep (3); */

  close(pipes[0]);

  return(&(window_tab[i])); 
}

PRIVATE void sunview_close_window(handle)
unix_window *handle;
{ BYTE data[4];

  data[0] = FUNCTION_CODE;
  data[1] = WINDOW_KILL;
  data[2] = '\0';
  data[3] = '\0';

  write(handle->child, data, 4);

  close(handle->child);
  handle->in_use = 0;
  while(wait3(NULL, WNOHANG, NULL) <= 0);	/* and bury dead child */
}

PRIVATE void sunview_window_size(handle, x, y)
unix_window *handle;
WORD *x, *y;
{ *y = handle->rows; *x = handle->cols;
}

PRIVATE void fn( update_panel, (int));

PRIVATE void sunview_send_to_window(data, handle)
char *data;
unix_window *handle;
{ if (handle eq Server_handle)
   if ( (its_log_dest ne log_dest) || (its_debugflags ne debugflags))
    update_panel(handle->child);
 
  if (write(handle->child, data, ANSI_buffer_count) <= 0)
   pipe_broken("window");
}

PRIVATE void update_debug_menu(flag, winflag, handle)
int flag, winflag, handle;
{ BYTE mess[4];

  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;
  mess[2] = winflag;
  if (debugflags & flag)
   { mess[3] = 1; its_debugflags |= flag; }
  else
   { mess[3] = 0; its_debugflags &= ~flag; }
  if (write(handle, mess, 4) <= 0)
   pipe_broken("window");
}

PRIVATE void update_panel(handle)
int handle;
{ BYTE mess[4];
  mess[0] = FUNCTION_CODE;
  mess[1] = WINDOW_PANEL;

  mess[2] = '\0';
  mess[3] = '\0';

  if (its_log_dest ne log_dest)
   { 
     switch(log_dest)
      { case Log_to_screen : mess[2] = WIN_LOG_SCREEN; break;
        case Log_to_file   : mess[2] = WIN_LOG_FILE; break;
        case Log_to_both   : mess[2] = WIN_LOG_BOTH; break;
        default            : goto log_error;
      }
    if (write(handle, mess, 4) <= 0)
     { pipe_broken("window"); return; }
log_error:
    its_log_dest = log_dest;
   }

  if (its_debugflags eq debugflags) return;

  if ((its_debugflags & Message_Flag) ne (debugflags & Message_Flag))
   update_debug_menu(Message_Flag, WIN_MESSAGES, handle);

  if ((its_debugflags & Search_Flag) ne (debugflags & Search_Flag))
   update_debug_menu(Search_Flag, WIN_SEARCH, handle);
 
  if ((its_debugflags & Open_Flag) ne (debugflags & Open_Flag))
   update_debug_menu(Open_Flag, WIN_OPEN, handle);
 
  if ((its_debugflags & Close_Flag) ne (debugflags & Close_Flag))
   update_debug_menu(Close_Flag, WIN_CLOSE, handle);
 
  if ((its_debugflags & Name_Flag) ne (debugflags & Name_Flag))
   update_debug_menu(Name_Flag, IOWIN_NAME, handle);
 
  if ((its_debugflags & Read_Flag) ne (debugflags & Read_Flag))
   update_debug_menu(Read_Flag, WIN_READ, handle);
 
  if ((its_debugflags & Boot_Flag) ne (debugflags & Boot_Flag))
   update_debug_menu(Boot_Flag, WIN_BOOT, handle);
 
  if ((its_debugflags & Keyboard_Flag) ne (debugflags & Keyboard_Flag))
   update_debug_menu(Keyboard_Flag, WIN_KEYBOARD, handle);
 
  if ((its_debugflags & Init_Flag) ne (debugflags & Init_Flag))
   update_debug_menu(Init_Flag, WIN_INIT, handle);
 
  if ((its_debugflags & Write_Flag) ne (debugflags & Write_Flag))
   update_debug_menu(Write_Flag, WIN_WRITE, handle);
 
  if ((its_debugflags & Quit_Flag) ne (debugflags & Quit_Flag))
   update_debug_menu(Quit_Flag, WIN_QUIT, handle);
 
  if ((its_debugflags & Graphics_Flag) ne (debugflags & Graphics_Flag))
   update_debug_menu(Graphics_Flag, WIN_GRAPHICS, handle);

  if ((its_debugflags & Timeout_Flag) ne (debugflags & Timeout_Flag))
   update_debug_menu(Timeout_Flag, WIN_TIMEOUT, handle);

  if ((its_debugflags & OpenReply_Flag) ne (debugflags & OpenReply_Flag))
   update_debug_menu(OpenReply_Flag, WIN_OPENREPLY, handle);

  if ((its_debugflags & FileIO_Flag) ne (debugflags & FileIO_Flag))
   update_debug_menu(FileIO_Flag, WIN_FILEIO, handle);

  if ((its_debugflags & Delete_Flag) ne (debugflags & Delete_Flag))
   update_debug_menu(Delete_Flag, WIN_DELETE, handle);

  if ((its_debugflags & Directory_Flag) ne (debugflags & Directory_Flag))
   update_debug_menu(Directory_Flag, WIN_DIRECTORY, handle);

#ifdef NEVER
  if ((its_debugflags & Com_Flag) ne (debugflags & Com_Flag))
   update_debug_menu(COM_Flag, WIN_COM, handle);

  if ((its_debugflags & HardDisk_Flag) ne (debugflags & HardDisk_Flag))
   update_debug_menu(HardDisk_Flag, WIN_HARDDISK, handle);
#endif

  its_debugflags = debugflags;
}


PRIVATE void fn( handle_function_code, (UBYTE *, unix_window *));

PRIVATE word sunview_read_char_from_keyboard(handle)
unix_window *handle;
{
  register int f_ptr, cp_ptr;
  int no_to_check;

  if((handle->how_many eq 0) && (handle->buf_index eq -1))
    {
      handle->how_many  = read(handle->child, handle->buf, 60);
      no_to_check       = handle->how_many;
      handle->buf_index = 0;

      if (handle->how_many <= 0)
       { pipe_broken("window"); return -1; }

      for(f_ptr = 0; f_ptr <  no_to_check; f_ptr++)
        if ( handle->buf[f_ptr] == FUNCTION_CODE )
          {
            handle_function_code(&handle->buf[f_ptr+1],handle);
            cp_ptr = f_ptr + 4;                  /* next data */
            while( cp_ptr < no_to_check)         /* strip out function code */
              {
                 handle->buf[cp_ptr - 4] = handle->buf[cp_ptr];
                 cp_ptr++;
              }
            handle->how_many -= 4;
          } 
    }

         /* the escape key deserves a special case, see dumb_read for details */
  if ((handle->how_many eq 1)  && 
      (handle->buf[0] eq 0x1b) && 
      (handle->matching eq -1)) 
  { fd_set t_rd_mask;
    struct timeval timelim;
      
    FD_ZERO(&t_rd_mask);
    FD_SET(handle->child, &t_rd_mask);
#if SM90
    timelim.tv_sec = 1;
#else
    timelim.tv_sec = 0;
#endif
    timelim.tv_usec = 250 * 1000;   /* 1/4 second, fine even at 50 baud */

    if (select(handle->child+1, &t_rd_mask, NULL, NULL, &timelim) <= 0)
     { handle->buf_index = 0; handle->how_many = 0;  return(0x1B); }
  }

  while (handle->how_many > 0) 
    { int x = add_to_window(handle->buf[handle->buf_index++], handle);
      handle->how_many--; 
      if (x ne -1) return(x);      
    } 

  { int x = add_to_window(0, handle);
    if (x eq -1)
     handle->buf_index = -1;
    return(x);
  }
}

PRIVATE void panel_decode(panel_code,panel_value) 
UBYTE panel_code,panel_value;
{
  switch(panel_code)
  {
    case(WIN_DEBUG)     : if (!debugflags)
                            debugflags = All_Debug_Flags;
                          else
                            debugflags = 0;
                          break;

    case(WIN_MESSAGES)  : debugflags ^= Message_Flag;     break;
    case(WIN_SEARCH)    : debugflags ^= Search_Flag;      break;
    case(WIN_OPEN)      : debugflags ^= Open_Flag;        break;
    case(WIN_CLOSE)     : debugflags ^= Close_Flag;       break;
    case(IOWIN_NAME)      : debugflags ^= Name_Flag;        break;
    case(WIN_READ)      : debugflags ^= Read_Flag;        break;
    case(WIN_BOOT)      : debugflags ^= Boot_Flag;        break;
    case(WIN_MEMORY)    : debugflags |= Memory_Flag;      break;
    case(WIN_KEYBOARD)  : debugflags ^= Keyboard_Flag;    break;
    case(WIN_INIT)      : debugflags ^= Init_Flag;        break;
    case(WIN_WRITE)     : debugflags ^= Write_Flag;       break;
    case(WIN_QUIT)      : debugflags ^= Quit_Flag;        break;
    case(WIN_GRAPHICS)  : debugflags ^= Graphics_Flag;    break;
    case(WIN_RECONF)    : debugflags |= Reconfigure_Flag; break;
    case(WIN_TIMEOUT)   : debugflags ^= Timeout_Flag;     break;
    case(WIN_OPENREPLY) : debugflags ^= OpenReply_Flag;   break;
    case(WIN_FILEIO)    : debugflags ^= FileIO_Flag;      break;
    case(WIN_DELETE)    : debugflags ^= Delete_Flag;      break;
    case(WIN_DIRECTORY) : debugflags ^= Directory_Flag;   break;
#ifdef NEVER
    case(WIN_COM)       : debugflags ^= Com_Flag;         break;
    case(WIN_HARDDISK)  : debugflags ^= HardDisk_Flag;    break;
#endif    
    /* Ignore nopop, listall */
    
    case(WIN_REBOOT)    : Special_Reboot = 1;             break;
    case(WIN_DEBUGGER)  : DebugMode = 1 - DebugMode;      break;
    case(WIN_STATUS)    : Special_Status = 1;             break;
    case(WIN_LOG_FILE)  : log_dest = Log_to_file;         break;
    case(WIN_LOG_SCREEN): log_dest = Log_to_screen;       break;
    case(WIN_LOG_BOTH)  : log_dest = Log_to_both;         break;
    case(WIN_EXIT)      : Special_Exit = 1;               break;
  
  }
 use(panel_value)
}

PRIVATE void handle_function_code(code_buf,handle)
UBYTE *code_buf;
unix_window *handle;
{
     switch(code_buf[0])
       {
         case(WINDOW_SIZE)  : { Window *window = find_window(handle);
                                if (window ne (Window *) NULL)
                                 { handle->rows = code_buf[1];
                                   handle->cols = code_buf[2];
                                   Resize_Ansi(window, handle->child,
                                               handle->rows, handle->cols);
                                 }
                                break;
                              }                                  

         case(WINDOW_PANEL) : panel_decode(code_buf[1],code_buf[2]);
                              break;
       }
 use(handle)
}

#endif

/**
*** Signal handling. For now all signals are ignored. Not all machines support
*** all of the signals below, but that is sorted out by the conditional
*** compilation. If there are any new signals on your machine add them at
*** the bottom.
**/

PRIVATE void exit_interrupt()
{ Special_Exit = 1;
}

PRIVATE void alarm_handler()
{
}

#if internet_supported
extern void oobdata();
#endif

#ifdef __cplusplus
typedef void	(*VoidIntFnPtr)(int);
#else
typedef VoidFnPtr	VoidIntFnPtr;
#endif
#define sigfunc_(f)	(VoidIntFnPtr)(f)

PRIVATE void initialise_signals()
{
#ifdef   SIGHUP
  signal(SIGHUP   , sigfunc_(exit_interrupt));
#endif
#ifdef   SIGINT
  signal(SIGINT   , sigfunc_(exit_interrupt));
#endif
#if 0
#ifdef   SIGQUIT
  signal(SIGQUIT  , sigfunc_(exit_interrupt));	/* safety net for debugging */
#endif  
#endif
#ifdef   SIGILL
  signal(SIGILL   , SIG_IGN);
#endif
#ifdef   SIGTRAP
  signal(SIGTRAP  , SIG_IGN);
#endif
#ifdef   SIGEMT
  signal(SIGEMT   , SIG_IGN);
#endif
#ifdef   SIGFPE
  signal(SIGFPE   , SIG_IGN);
#endif
#ifdef   SIGBUS
  signal(SIGBUS   , SIG_IGN);
#endif
#if 0
				/* leave this one open */
#ifdef   SIGSEGV
  signal(SIGSEGV  , SIG_IGN);
#endif
#endif
#ifdef   SIGKILL
  signal(SIGKILL  , sigfunc_(exit_interrupt)); /* leave a back door for debugging */
#endif
#ifdef   SIGSYS
  signal(SIGSYS   , SIG_IGN);
#endif
#ifdef   SIGPIPE
  signal(SIGPIPE  , SIG_IGN);
#endif
#ifdef   SIGALRM
#if (SM90 || SOLARIS)
#else
  signal(SIGALRM  , sigfunc_(alarm_handler));
  alarm(2);   /* cause an alarm signal every second, this causes any */
              /* blocked I/O to unblock */
#endif
#endif
#ifdef   SIGTERM
  signal(SIGTERM  , sigfunc_(exit_interrupt));
#endif
#if internet_supported
#ifdef   SIGURG
  signal(SIGURG   , sigfunc_(oobdata));
#endif
#else
#ifdef   SIGURG
  signal(SIGURG   , SIG_IGN);
#endif
#endif
#ifdef   SIGIO
  signal(SIGIO    , SIG_IGN);
#endif
#ifdef   SIGUSR1
  signal(SIGUSR1  , SIG_IGN);
#endif
#ifdef   SIGUSR2
  signal(SIGUSR2  , SIG_IGN);
#endif
#ifdef   SIGABRT
  signal(SIGABRT  , SIG_IGN);
#endif
#ifdef   SIGSTOP
  signal(SIGSTOP  , sigfunc_(exit_interrupt));
#endif
#ifdef   SIGTSTP
  signal(SIGTSTP  , sigfunc_(exit_interrupt));
#endif
#ifdef   SIGCONT
  signal(SIGCONT  , SIG_IGN);
#endif
#ifdef   SIGCHLD
  signal(SIGCHLD  , SIG_IGN);
#endif
#ifdef   SIGTTIN
  signal(SIGTTIN  , SIG_IGN);
#endif
#ifdef   SIGTTOU
  signal(SIGTTOU  , SIG_IGN);
#endif
#ifdef   SIGXCPU
  signal(SIGXCPU  , SIG_IGN);
#endif
#ifdef   SIGXFSZ
  signal(SIGXFSZ  , SIG_IGN);
#endif
#ifdef   SIGVTALRM
  signal(SIGVTALRM, SIG_IGN);
#endif
#ifdef   SIGPROG
  signal(SIGPROF  , SIG_IGN);
#endif
#ifdef   SIGWINCH
  signal(SIGWINCH , SIG_IGN);
#endif
#ifdef   SIGLOST
  signal(SIGLOST  , SIG_IGN);
#endif
}

/**
*** The goto_sleep() routine, some machines are only accurate to a second,
*** others to microseconds.
**/
void goto_sleep(delay)
long delay;
{
#if (UNIX386 || SM90 || TR5 || i486V4 || HP9000 || SCOUNIX || SOLARIS)
  if (delay < OneSec)
   sleep(1);
  else
   sleep(delay / OneSec);
#else
  usleep(delay);
#endif
}

/**
*** The multiwait function to handle multi-tasking.
**/

PRIVATE int link_not_selectable = 0;  /* cope with Inmos standard */
PRIVATE struct timeval timelim;
PRIVATE int maxmask;
PRIVATE int maxfd = 0;
PRIVATE fd_set rd_mask, wr_mask, ex_mask;
PRIVATE fd_set blank_mask;
PRIVATE WORD **rd_wakeup, **wr_wakeup, **ex_wakeup;
#if internet_supported
PRIVATE WORD **fd_selval;
#endif

#if i486V4
#define getdtablesize() (OPEN_MAX)
#endif

#ifdef __cplusplus
extern "C"
{
#endif
int fn (getdtablesize, (void));
#ifdef __cplusplus
}
#endif

void InitMultiwait()
{ maxmask= getdtablesize();                 /* number of sig bits in mask */
  rd_wakeup = (WORD **) malloc(maxmask * sizeof(WORD));
  wr_wakeup = (WORD **) malloc(maxmask * sizeof(WORD));
  ex_wakeup = (WORD **) malloc(maxmask * sizeof(WORD));
#if internet_supported
  fd_selval = (WORD **) malloc(maxmask * sizeof(WORD));
#endif

  FD_ZERO(&blank_mask);
  if ( (rd_wakeup eq NULL) || (wr_wakeup eq NULL) || (ex_wakeup eq NULL)
#if internet_supported
	|| (fd_selval eq NULL)
#endif
	)
   { printf("Failed to initialise multiwait.\r\n");
/*     longjmp(exit_jmpbuf, 1); */
     longjmp_exit;
   }
}

void TidyMultiwait()
{ free((char *)rd_wakeup); free((char *)wr_wakeup); free((char *)ex_wakeup);
#if internet_supported
  free((char *)fd_selval);
#endif
}

void RestartMultiwait()
{ int i;
  link_not_selectable = 0;
  for (i = 0; i < maxmask; i++)
   { rd_wakeup[i] = NULL; wr_wakeup[i] = NULL; ex_wakeup[i] = NULL;
#if internet_supported
     fd_selval[i] = NULL;
#endif
   }

  FD_ZERO(&rd_mask);
  FD_ZERO(&wr_mask);
  FD_ZERO(&ex_mask);
}

void fn( WindowAddMultiwait,   (WORD *, unix_window *));
void fn( WindowClearMultiwait, (unix_window *));

#if (RS6000 || HP9000 || SCOUNIX)
void AddMultiwait(WORD code, WORD *addr, ...)
{ word extra1, extra2;
  va_list args;

  va_start(args, addr);
  extra1 = va_arg(args, word);
  extra2 = va_arg(args, word);
  va_end(args);

#else
#if ANSI_prototypes
void AddMultiwait (long code, long * addr, long extra1, long extra2)
#else
void AddMultiwait(code, addr, extra1, extra2)
long code;
long *addr;
long extra1, extra2;
#endif
{
#endif
  switch(code)
   { case Multi_LinkMessage :
        { int link_fd = link_table[extra1].fildes;
          if (link_table[extra1].flags & Link_flags_not_selectable)
           { link_not_selectable += 1;
             link_table[extra1].flags |= Link_flags_waiting;
           }
          else
           { FD_SET(link_fd, &rd_mask);
             rd_wakeup[link_fd] = addr;
           }
        }
        break;

     case Multi_MouseInput    :
     case Multi_KeyboardInput : 
             FD_SET(extra1, &rd_mask);
             rd_wakeup[extra1] = addr;
             break;

     case Multi_WindowInput :
        WindowAddMultiwait(addr, (unix_window *) extra1);
        break;

#if internet_supported
     case Multi_SocketInput:
             FD_SET(extra1, &rd_mask);
             rd_wakeup[extra1] = addr;
	     fd_selval[extra1] = (WORD *) extra2;
             break;

     case Multi_SocketOutput:
             FD_SET(extra1, &wr_mask);
             wr_wakeup[extra1] = addr;
	     fd_selval[extra1] = (WORD *) extra2;
             break;

     case Multi_SocketExcp:
             FD_SET(extra1, &ex_mask);
             ex_wakeup[extra1] = addr;
	     fd_selval[extra1] = (WORD *) extra2;
             break;
#endif

     default :
        ServerDebug("Unknown multiwait %d added", code);
        break;
   }

 for (maxfd = maxmask; maxfd > 0; maxfd--)
  if (FD_ISSET(maxfd, &rd_mask) || FD_ISSET(maxfd, &wr_mask) || FD_ISSET(maxfd, &ex_mask))
   break;
 
#if ! internet_supported
 use(extra2)
#endif
}

#if (RS6000 || HP9000 || SCOUNIX)
void ClearMultiwait(WORD code, ...)
{ WORD extra1, extra2;
  va_list args;

  va_start(args, code);
  extra1 = va_arg(args, WORD);
  extra2 = va_arg(args, WORD);
  va_end(args);
#else

#if ANSI_prototypes
void ClearMultiwait (long code, long extra1, long extra2)
#else
void ClearMultiwait(code, extra1, extra2)
long code;
long extra1, extra2;
#endif
{
#endif
  switch(code)
   { case Multi_LinkMessage :
       { int link_fd = link_table[extra1].fildes;
         if (link_table[extra1].flags & Link_flags_not_selectable)
          { link_not_selectable -= 1;
            link_table[extra1].flags &= ~Link_flags_waiting;
          }
         else
          { FD_CLR(link_fd, &rd_mask);
            rd_wakeup[link_fd] = NULL;
          }
       }
       break;

     case Multi_MouseInput    :
     case Multi_KeyboardInput : 
             FD_CLR(extra1, &rd_mask);
             rd_wakeup[extra1] = NULL;
             break;

     case Multi_WindowInput :
       WindowClearMultiwait((unix_window *) extra1);
       break;

#if internet_supported
     case Multi_SocketInput:
             FD_CLR(extra1, &rd_mask);
             rd_wakeup[extra1] = NULL;
	     fd_selval[extra1] = 0;
             break;

     case Multi_SocketOutput:
             FD_CLR(extra1, &wr_mask);
             wr_wakeup[extra1] = NULL;
	     fd_selval[extra1] = 0;
             break;

     case Multi_SocketExcp:
             FD_CLR(extra1, &ex_mask);
             ex_wakeup[extra1] = NULL;
	     fd_selval[extra1] = 0;
             break;
#endif

     default :
       ServerDebug("Unknown multiwait %d cleared", code);
       break;
   } 

 for (maxfd = maxmask; maxfd > 0; maxfd--)
  if (FD_ISSET(maxfd, &rd_mask) || FD_ISSET(maxfd, &wr_mask) || FD_ISSET(maxfd, &ex_mask))
   break;

 use(extra2)
}

void WindowAddMultiwait(addr, handle)
WORD *addr;
unix_window *handle;
{ switch(window_type)
   { case WindowType_Dumb    : if (handle eq Server_handle)
                                { FD_SET(fileno(stdin), &rd_mask);
                                  rd_wakeup[fileno(stdin)] = addr;
                                }
                               return;

     case WindowType_Sunview : 
     case WindowType_X       : FD_SET(handle->child, &rd_mask);
                               rd_wakeup[handle->child] = addr;
                               return;

     default                 : ServerDebug("bad addmulti for window");
   }
}

void WindowClearMultiwait(handle)
unix_window *handle;
{ switch(window_type)
   { case WindowType_Dumb    : if (handle eq Server_handle)
                                { FD_CLR(fileno(stdin), &rd_mask);
                                  rd_wakeup[fileno(stdin)] = NULL;
                                }
                               return;

     case WindowType_Sunview : 
     case WindowType_X       : FD_CLR(handle->child, &rd_mask);
                               rd_wakeup[handle->child] = NULL;
                               return;

     default                 : ServerDebug("bad addmulti for window");
   }
}

WORD Multiwait()
{ fd_set t_rd_mask, t_wr_mask, t_ex_mask;
  int    no_fds, i;
  bool   short_timeout = FALSE;

  no_fds = 0;

  memcpy(&t_rd_mask, &rd_mask, sizeof(fd_set));
  memcpy(&t_wr_mask, &wr_mask, sizeof(fd_set));
  memcpy(&t_ex_mask, &ex_mask, sizeof(fd_set));

  if (link_not_selectable)               /* cope with Inmos standard */
   { int i;                              /* link messages take priority */

     for (i = 0; i < number_of_links; i++)
      if (link_table[i].flags & Link_flags_waiting)
       { int old_link = current_link;
         current_link = i;
         if (xprdrdy())
          {
	    link_table[i].ready = CoReady;
            no_fds++;
          }
         current_link = old_link;
       }

     if (no_fds > 0)
       {
	 return(TRUE);
       }

    short_timeout = TRUE;
   }

   /* If there are coroutines which have merely released the CPU for */
   /* a while but do not want to be suspended for 1/2 second, they   */
   /* should have incremented Multi_nowait */
  if (Multi_nowait > 0)
   short_timeout = TRUE;

  timelim.tv_sec = 0;
#if SM90
  timelim.tv_sec = 1;
#endif

  if (short_timeout)
    {
      timelim.tv_usec = 1000;
    }
  else
    {
#if (TR5 || i486V4)
      timelim.tv_usec = 50 * 1000;
#else
      timelim.tv_usec = 500 * 1000;	/* 1/2 second default timeout */
#endif  
    }

  no_fds = select(maxfd + 1, &t_rd_mask, &t_wr_mask, &t_ex_mask, &timelim);

  if (no_fds eq 0)
    {
      return(FALSE);           /* no IO therefore useless to look */
    }

  if (no_fds eq -1)
    { if (errno ne EINTR)  /* no warning if signal */
       ServerDebug("Warning : select failed : errno is %d", errno);
      return(FALSE);
    }

    /* Anything happen on the file descriptors selected for reading ? */
  if (memcmp(&t_rd_mask, &blank_mask, sizeof(fd_set)))
   { 
     for (i = 0; (i < maxmask) && (no_fds > 0); i++)
     {
      if ((rd_wakeup[i] ne NULL) && (FD_ISSET(i, &t_rd_mask)))
       { 
         *(rd_wakeup[i]) = CoReady; no_fds--;
#if internet_supported
	 if(fd_selval[i])
	  *(fd_selval[i]) |= O_ReadOnly;
#endif
       }

      }
   }
   
  if (no_fds <= 0)
    {
      return(TRUE);
    }

    /* Some more left, try the ones selected for writing ? */ 
  if (memcmp(&t_wr_mask, &blank_mask, sizeof(fd_set)))
   {
     for (i = 0; (i < maxmask) && (no_fds > 0); i++)
      if ((wr_wakeup[i] ne NULL) && (FD_ISSET(i, &t_wr_mask)))
       { *(wr_wakeup[i]) = CoReady; no_fds--;
#if internet_supported
	if(fd_selval[i])
	 *(fd_selval[i]) |= O_WriteOnly;
#endif
       }
   }
   
  if (no_fds <= 0)
    {
      return(TRUE);
    }
    /* exceptions are the only thing left */
  if (memcmp(&t_ex_mask, &blank_mask, sizeof(fd_set)))
   {
     for (i = 0; (i < maxmask) && (no_fds > 0); i++)
      if ((ex_wakeup[i] ne NULL) && (FD_ISSET(i, &t_ex_mask)))
       { *(ex_wakeup[i]) = CoReady; no_fds--;
#if internet_supported
	if(fd_selval[i])
	 *(fd_selval[i]) |= O_Exception;
#endif
       }
   }
 
  if (no_fds > 0)
    ServerDebug("Strange : not all selected streams accounted for");

  return(TRUE);
}

/**
*** Get the current system time, as a Unix time stamp. This is needed all over
*** the place.
**/

WORD get_unix_time()
{
  return (time(NULL));
}



/*------------------------------------------------------------------------
--
-- The following code implements the local file IO routines
--
------------------------------------------------------------------------*/

typedef int stream;
  
WORD object_exists(name)
char *name;
#if SM90
{ if (stat(name, &searchbuffer) eq -1)
#else
{ if (stat(name, &searchbuffer) eq -1 && lstat(name, &searchbuffer) eq -1)
#endif
   { convert_error();
     searchbuffer.st_mode = S_IFREG;
     return(FALSE);
   }
  else
     return(TRUE);
}

WORD create_directory(name)
char *name;
{ if (mkdir(name, 0777) eq -1)
   { convert_error(); return(FALSE); }
  else
   return(TRUE);
}


WORD delete_directory(name)
char *name;
{ if (rmdir(name) ne 0)
   { convert_error(); return(FALSE); }
  else
   return(TRUE);
}
 
WORD delete_file(name)
char *name;
{
  int notsu = getuid();

  if(notsu)					/* not root */
  	if(searchbuffer.st_uid != notsu)	/* not owner */
		if((searchbuffer.st_mode & (S_IWGRP|S_IWOTH)) == 0)
			{ errno = EACCES; convert_errno(); return(FALSE); }
  if (unlink(name) ne 0)
   { convert_error(); return(FALSE); }
  else
   return(TRUE);
}

WORD rename_object(from, to)
char *from, *to;
{ 
  int notsu = getuid();

  if(notsu)					/* not root */
  	if(searchbuffer.st_uid != notsu)	/* not owner */
		if((searchbuffer.st_mode & (S_IWGRP|S_IWOTH)) == 0)
			{ errno = EACCES; convert_errno(); return(FALSE); }
  if (rename(from, to) ne 0)
   { convert_error(); return(FALSE); }
  else
   return(TRUE);
}


/**
*** There is a system call to do the create, but it leaves the resulting file
*** open so I need to close it.
**/
WORD create_file(name)
char *name;
{ int handle = open(name, O_RDWR | O_CREAT | O_TRUNC,0666); 
                              /* create if not exist, else truncate */
  if (handle < 0)             /* did an error occur ? */
   { convert_error();  return(FALSE); }

  close(handle);      /* Helios create does not leave the file open */
  return(TRUE);
}

/**
*** get_file_info() is called only after a successful object_exists() i.e.
*** a stat(), so that all the information required is available in
*** searchbuffer.
**/
WORD get_file_info(name, Heliosinfo)
char    *name;
ObjInfo *Heliosinfo;
{ WORD type;
  WORD time = searchbuffer.st_mtime; /* I guess modified time's what we */
                                     /* most often mean  */

  { char *p = name + strlen(name);
    while (*p ne '/') p--;
    strncpy(Heliosinfo->DirEntry.Name, ++p, 31);
  }
 
  switch(searchbuffer.st_mode & S_IFMT)
   { case S_IFDIR  : type = Type_Directory; break;

     case S_IFIFO  : type = Type_Fifo; break;

#ifdef S_IFSOCK
     case S_IFSOCK :   /* this type goes by different names */
#endif
#ifdef S_IFSOC
     case S_IFSOC  :
#endif

#if (UNIX)
     case S_IFCHR  :   /* allow access to the devices in /dev ? */
     case S_IFBLK  :   /* Depends on the customer ... */
#endif

     case S_IFREG  : type = Type_File; break;

#if !(UNIX)
     case S_IFCHR  :
     case S_IFBLK  :
#endif
     default       : type = Type_Device; 
   }
                          /* we have got the info, now convert and store it */
  Heliosinfo->DirEntry.Type = swap(type);
  Heliosinfo->DirEntry.Matrix = swap((type eq Type_Directory) ?
                                     DefDirMatrix : DefFileMatrix);

  Heliosinfo->DirEntry.Flags  = swap(0L);
  Heliosinfo->Account   = swap(0L);
  Heliosinfo->Size      = swap(searchbuffer.st_size);
  Heliosinfo->Creation  = swap(time);
  Heliosinfo->Access    = swap(time);
  Heliosinfo->Modified  = swap(time);

  return(TRUE);
}



WORD set_file_date(name,time) 
     char *name;
     long time;
{
#if SOLARIS
  struct utimbuf timestamps;
  timestamps.actime = time;
  timestamps.modtime = time; 

  return (utime (name,&timestamps) eq 0) ? TRUE : FALSE;
#else
  time_t timestamps[2]; 
  timestamps[0] = time;
  timestamps[1] =time;
  return (utime(name,timestamps) eq 0) ? TRUE : FALSE;
#endif
}

/**
*** statfs gets the info iff cwd is on the right drive
*** With NFS we may have a problem, which drive is meant
**/
WORD get_drive_info(name, reply)
char *name;
servinfo *reply;
{ 
#if (SM90 || TR5 || i486V4)                   /* statfs not supported */
  reply->size  = 0L;
  reply->used  = 0L;
  reply->alloc = 512;
  return(TRUE);

#else 

#if (UNIX386 || SCOUNIX || SOLARIS)
#define f_bavail f_bfree
#endif

  struct statfs buf;
  reply->type = swap(Type_Directory);
  if (statfs(name, &buf, sizeof(struct statfs), 0) ne 0)
   { convert_error();  return (FALSE); }
  reply->size  = swap((WORD) buf.f_bsize*buf.f_blocks);
  reply->used  = swap((WORD) buf.f_bsize*buf.f_bavail);
  reply->alloc = swap((WORD) buf.f_bsize);
  
  return(TRUE);
#endif
}

/**
*** This used to be fairly straightforward, apart from worrying about
*** whether or not to create/truncate the file. Sadly the object
*** concerned need not be a file.
**/
PRIVATE int fn( socket_open, (char *, WORD));
#define Blocking_bit     0x100
#define Blocking_mask    0x0FF

int open_file(name, Heliosmode)
char *name;
WORD Heliosmode;
{ int  handle, fifo = 0;
  word itsmode, mymode;

  switch(searchbuffer.st_mode & S_IFMT)
   { 
#ifdef S_IFSOCK   
     case S_IFSOCK : return(socket_open(name, Heliosmode));
#endif
#ifdef S_IFSOC
     case S_IFSOC  : return(socket_open(name, Heliosmode));
#endif

#if !(UNIX)
     case S_IFCHR  : /* depending on implementation, deny access to /dev */
     case S_IFBLK  : Server_errno = EC_Error + SS_IOProc + EG_WrongFn +
                     EO_Object;
                     return(0);
#else
     case S_IFCHR  : fifo = 1;  /* No blocking please */
#endif
     case S_IFIFO  : fifo = 1;
   }
     
  itsmode = Heliosmode & 0xF;  /* ignore create bit */
  if (itsmode eq O_ReadOnly)
    mymode = O_RDONLY;
  elif (itsmode eq O_WriteOnly)
    mymode = O_WRONLY;
  elif (itsmode eq O_ReadWrite)
    mymode = O_RDWR;
  else
    return(0);

  if ((O_Create & Heliosmode) ne 0)
    mymode |= O_CREAT;
  if ((O_Truncate & Heliosmode) ne 0)
    mymode |= O_TRUNC;
  
  handle = open(name,mymode,0666);
  
  if (handle < 0)
   { convert_error();  return(0); }

  fcntl(handle, F_SETFD, 1);  /* do not pass on to forked-off processes */

  if (fifo)
   { fcntl(handle, F_SETFL, FNDELAY);
     handle += Blocking_bit;
   }

  return(handle);
}

PRIVATE int socket_open(name, mode)
char *name;
WORD mode;
{ Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Object;
  return(-1);
  use(name)
  use(mode)
}

WORD close_file(strm)
int strm;
{ if (close(strm & Blocking_mask) ne 0)
   { convert_error(); return(FALSE); }
  else
   return(TRUE);
}

WORD read_from_file(handle, amount, buff)
stream handle;
WORD   amount;
BYTE   *buff;
{ int result = read(handle & Blocking_mask, buff, amount);
  if (result < 0)
   { convert_error(); return(-1); }
  else
   return(result);
}

WORD write_to_file(handle, amount, buff)
stream handle;
WORD   amount;
BYTE   *buff;
{ if (write(handle & Blocking_mask, buff, amount) < 0)
   { convert_error(); return(FALSE); }
  else
   return(TRUE);
}

#ifndef L_XTND
# define L_XTND 2
#endif

WORD seek_in_file(handle, offset, mode)
stream handle;
WORD offset, mode;
{ WORD result;

  if (handle & Blocking_bit)
   { Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Stream;
     return(-1);
   }
  result = (mode eq 0) ? lseek(handle, offset, L_SET) :
                              lseek(handle, offset, L_XTND);
  if (result eq -1)
   convert_error();
  return(result);
}

/**
*** get size is easy under unix, fstat is just what's needed
*** Still have to do a seek required by the spec
**/

WORD get_file_size(handle, old_pos)
int  handle;
WORD old_pos;
{ 
  struct stat buf;

  if (handle & Blocking_bit)
   { Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Stream;
     return(-1);
   }

  if ((fstat(handle,&buf)) ne 0)
   { convert_error();  return (-1l); }
  if ((lseek(handle,old_pos,L_SET)) <0)
   { convert_error();  return (-1l); }
  return ((WORD) buf.st_size);
}

/**
*** search_directory() is responsible for searching through the entire directory
*** whose name it is given, converting the information obtained to a
*** Helios DirEntryStructure, and storing that in the linked list passed as
*** argument. 
*** The routine returns -1 to indicate an error, otherwise the number of
*** entries in the list.
**/

#if (0)
	/* Was needed for RISCiX 1.1 */
#define dirent direct
#endif

PRIVATE WORD fn( add_node, (List *, char *, char *));

#if ANSI_prototypes
WORD search_directory (char * pathname, List * header)
#else
WORD search_directory(pathname, header)
char    *pathname;
List    *header;
#endif
{ PRIVATE char buf[PATH_MAX];
  char *end_ptr;
  int count = 0;
  DIR *dirp;
  struct dirent *entryp;

#if SOLARIS
  entryp = (struct dirent *)(malloc (sizeof (struct dirent) + _POSIX_PATH_MAX));

  if (entryp == NULL)
  {
    convert_errno ();
    return -1;
  }
#endif

  strcpy(buf, pathname);
  strcat(buf, "/");
  end_ptr = buf + strlen(buf);
 
  dirp = opendir(pathname);
  if (dirp eq (DIR *) NULL)
   {
     Debug (Directory_Flag, ("Failed to open directory %s", pathname));
     convert_error();
     return (-1); 
   }

#if SOLARIS
  while (readdir_r (dirp, entryp) != NULL)
#else
  for (entryp = (struct dirent *) readdir(dirp);
       entryp ne (struct dirent *) NULL;
       entryp = (struct dirent *) readdir(dirp))
#endif
  { 
    strcpy(end_ptr, entryp->d_name);
    count++;
    if(add_node(header,buf, entryp->d_name) eq FALSE) 
    {
      Debug (Directory_Flag, ("Add node failed for %s", pathname));
      FreeList(header);
      closedir(dirp);
      return(-1);
    }
  }

  closedir(dirp);
  return (count);
}

/**
*** When the search through the directory has revealed another entry, all
*** the information about this entry will be stored in searchbuffer. It
*** is necessary to convert this information to the Helios DirEntry
*** structure.
**/

#if ANSI_prototypes
PRIVATE word add_node (List * header, char * pathname, char * last_bit)
#else
PRIVATE word add_node(header,pathname, last_bit)
     List *header;
     char *pathname, *last_bit;
#endif
{
  DirEntryNode *newnode;
  word type;

#if SM90
  if (stat(pathname,&searchbuffer) eq -1)
#else
  if (stat(pathname,&searchbuffer) eq -1 && lstat(pathname,&searchbuffer) eq -1)
#endif
  {
    Debug (Directory_Flag, ("Failed to stat %s", pathname));
    convert_error(); return(FALSE);
  }
 
  switch(searchbuffer.st_mode & S_IFMT)
   { case S_IFDIR  : type = Type_Directory; break;
     case S_IFIFO  : type = Type_Fifo; break;
#ifdef S_IFSOCK
     case S_IFSOCK : 
#endif
#ifdef S_IFSOC
     case S_IFSOC  :
#endif
#if (UNIX)
     case S_IFCHR  :   /* allow access to /dev ? */
     case S_IFBLK  :
#endif
     case S_IFREG  : type = Type_File; break;
#if !(UNIX)
     case S_IFCHR  :
     case S_IFBLK  :
#endif     
     default       : type = Type_Device; 
   }

  newnode = (DirEntryNode *) malloc(sizeof(DirEntryNode));
  unless(newnode)
    {
      Debug (Directory_Flag, ("Failed to malloc memory for DirEntryNode"));
      Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Server;
      return(FALSE);
    }

  memset(newnode, 0, sizeof(DirEntryNode));
  (newnode->direntry).Type     = swap(type);
  (newnode->direntry).Flags    = swap(0L);
  (newnode->direntry).Matrix   = swap( (type eq Type_Directory) ? 
                                       DefDirMatrix : DefFileMatrix  );
  
  strncpy((newnode->direntry).Name, last_bit, 31);

  AddTail((Node *)newnode, header);    /* put the node at the end of the list */

  return(TRUE);
}



/**
*** Error conversion : this takes a unix error code held in errno and converts
*** it to a Helios error code in Server_errno. Not all machines define all
*** the errors, hence conditional compilation everywhere.
**/
PRIVATE void convert_error()
{ switch(errno)
   {
#ifdef    EPERM 
     case EPERM        :       /* Not owner */
           Server_errno = EC_Error + SS_IOProc + EG_Protected + EO_File;
           return;
#endif
#ifdef    ENOENT
     case ENOENT       :       /* No such file or directory */
           Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;
           return;
#endif
#ifdef    ENINTR
     case EINTR        :       /* Interrupted system call */
           Server_errno = EC_Warn + SS_IOProc + EG_Exception + EO_Stream;
           return;
#endif
#ifdef    EIO
     case EIO          :       /* I/O error */
           Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
           return;
#endif
#ifdef    ENXIO
     case ENXIO        :       /* No such device or address */
           Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;
           return;
#endif
#ifdef    EBADF
     case EBADF        :       /* Bad file number */
           Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_Stream;
           return;
#endif
#ifdef    ENOMEM
     case ENOMEM       :       /* Not enough core */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Server;
           return;
#endif
#ifdef    EACCES
     case EACCES       :       /* Permission denied */
           Server_errno = EC_Error + SS_IOProc + EG_Protected + EO_File;
           return;
#endif
#ifdef    EFAULT
     case EFAULT       :       /* Bad address */
           Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Server;
           return;
#endif
#ifdef    EBUSY
     case EBUSY        :       /* Mount device busy */
           Server_errno = EC_Warn + SS_IOProc + EG_InUse + EO_Object;
           return;
#endif
#ifdef    EEXIST
     case EEXIST       :       /* File exists */
           Server_errno = EC_Error + SS_IOProc + EG_Create + EO_Object;
           return;
#endif
#ifdef    EXDEV
     case EXDEV        :       /* Cross-device link */
           Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Object;
           return;
#endif
#ifdef    ENOTDIR
     case ENOTDIR      :       /* Not a directory*/
           Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_File;
           return;
#endif
#ifdef    EISDIR
     case EISDIR       :       /* Is a directory */
           Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Directory;
           return;
#endif
#ifdef    EINVAL
     case EINVAL       :       /* Invalid argument */
           Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Server;
           return;
#endif
#ifdef    EFBIG
     case EFBIG        :       /* File too large */
           Server_errno = EC_Error + SS_IOProc + EG_WrongSize + EO_File;
           return;
#endif
#ifdef    ENFILE
     case ENFILE       :       /* File table overflow */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Stream;
           return;
#endif
#ifdef    EMFILE
     case EMFILE       :       /* Too many open files */
                               /* DO NOT CHANGE THIS HELIOS ERROR */
           Server_errno = EC_Error + SS_IOProc + EG_NoResource + EO_Server;
           return;
#endif
#ifdef    ENOSPC
     case ENOSPC       :       /* No space left on device */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Route;
           return;
#endif
#ifdef    ESPIPE
     case ESPIPE       :       /* Illegal seek */
           Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Fifo;
           return;
#endif
#ifdef     EROFS
     case EROFS        :       /* Read-only file system */
           Server_errno = EC_Error + SS_IOProc + EG_Protected + EO_Route;
           return;
#endif
#ifdef    EMLINK
     case EMLINK       :       /* Too many links */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Link;
           return;
#endif
#ifdef    EPIPE
     case EPIPE        :       /* Broken pipe */
           Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Fifo;
           return;
#endif
#ifdef    EWOULDBLOCK
     case EWOULDBLOCK  :       /* Operation would block */
           Server_errno = EC_Warn + SS_IOProc + EG_InUse + EO_Object;
           return;
#endif
#ifdef    ELOOP
     case ELOOP        :       /* Too many levels of symbolic links */
           Server_errno = EC_Error + SS_IOProc + EG_WrongSize + EO_Link;
           return;
#endif
#ifdef    ENAMETOOLONG
     case ENAMETOOLONG :       /* File name too long */
           Server_errno = EC_Error + SS_IOProc + EG_Name + EO_File;
           return;
#endif
#ifndef RS6000
#ifdef    ENOTEMPTY
     case ENOTEMPTY    :       /* Directory not empty */
           Server_errno = EC_Error + SS_IOProc + EG_InUse + EO_Directory;
           return;
#endif
#endif

#ifdef    EDQUOT
     case EDQUOT       :       /* Disc quota exceeded */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Route;
           return;
#endif
      default : return;
   }
}

#if ETC_DIR
/**
*** File name translation. To allow Helios to be run over a network where
*** the bin and lib directories reside on the file server but the etc
*** directory is local, accesses to /helios/etc may involve an additional
*** name change. Also, /helios/local/spool and /helios/tmp require special
*** treatment.
**/
static	char	*etc_directory;
static	char	etc_name[128];
static	int	etc_length;
static	char	*tmp_directory;
static	char	tmp_name[128];
static	int	tmp_length;
static	char	*spool_directory;
static	char	spool_name[128];
static	int	spool_length;

static void initialise_etc_directory()
{
  etc_directory = get_config("etc_directory");
  if (etc_directory ne NULL)
   { strcpy(etc_name, Heliosdir);
     strcat(etc_name, "/etc");
     etc_length = strlen(etc_name);
   }

  tmp_directory = get_config("tmp_directory");
  if (tmp_directory ne NULL)
   { strcpy(tmp_name, Heliosdir);
     strcat(tmp_name, "/tmp");
     tmp_length = strlen(tmp_name);
   }

  spool_directory = get_config("spool_directory");
  if (spool_directory ne NULL)
   { strcpy(spool_name, Heliosdir);
     strcat(spool_name, "/local/spool");
     spool_length = strlen(spool_name);
   }
}
#endif
/**
*** This routine gets called whenever the user tries to access something in
*** /helios. The purpose is to allow multiple users on different sites to
*** share the same Helios binaries. To do this all accesses to the network
*** server are modified to include a site number, as are all accesses to the
*** etc and tmp directories.
**/
void check_helios_name(str)
char *str;
{
#if ETC_DIR

  if (etc_directory ne NULL)
   { if (!strncmp(str, etc_name, etc_length))
      { strcpy(misc_buffer1, etc_directory);
        strcat(misc_buffer1, &(str[etc_length]));
        strcpy(str, misc_buffer1);
        return;
      }
   }

  if (tmp_directory ne NULL)
   { if (!strncmp(str, tmp_name, tmp_length))
      { strcpy(misc_buffer1, tmp_directory);
        strcat(misc_buffer1, &(str[tmp_length]));
        strcpy(str, misc_buffer1);
        return;
      }
   }

  if (spool_directory ne NULL)
   { if (!strncmp(str, spool_name, spool_length))
      { strcpy(misc_buffer1, spool_directory);
        strcat(misc_buffer1, &(str[spool_length]));
        strcpy(str, misc_buffer1);
        return;
      }
   }
#else
 if (!strcmp(str, "lib/net_serv"))
   sprintf(str, "lib/net_serv%d", transputer_site);
  elif ((str[0] eq 'e') && (str[1] eq 't') && (str[2] eq 'c') && 
        ((str[3] eq '/') || (str[3] eq '\0')) )
   { sprintf(misc_buffer1, "etc%d%s", transputer_site, &(str[3]));
     strcpy(str, misc_buffer1);
   }
  elif ((str[0] eq 't') && (str[1] eq 'm') && (str[2] eq 'p') &&
        ((str[3] eq '/') || (str[3] eq '\0')) )
   { sprintf(misc_buffer1, "tmp%d%s", transputer_site, &(str[3]));
     strcpy(str, misc_buffer1);
   }
#endif
}

