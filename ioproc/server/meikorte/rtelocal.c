/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--               Copyright (C) 1989, Perihelion Software Ltd.           --
--                            All Rights Reserved.                      --
--                                                                      --
--  rtelocal.c                                                          --
--                                                                      --
--  Author : BLV 23.8.89                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1989, Perihelion Software Ltd.        */

#define Local_Module

#include "../helios.h"


/******************************************************************************
*******************************************************************************
******** Routines needed by all the local modules *****************************
*******************************************************************************
******************************************************************************/

/**
*** Device initialisation and tidying routines. This is mainly the windowing
*** initialisation.
***
*** Multiple windows support. Only dumb terminals are supported at present.
***
*** The ANSI emulator is rather complicated, in that different outputs have
*** to be produced for the different windowing types. In all cases the output
*** is buffered in terminal.c, and written to the window at the end of the
*** string or when the buffer is full. This eliminates the need for a
*** check_window() routine for dumb terminal, since it is incorporated into
*** send_to_window().
***
*** Input is much more complicated than before, because all three windowing
*** systems can produce their own sequences for cursor keys, function keys,
*** debugging options, window resizing, etc. All windowing input uses the
*** termcap entries to map keys onto the Helios sequences.
**/

PRIVATE void fn( convert_error, (void));

extern char * fn( getenv,  (char *));
extern char * fn( tgetstr, (char *, char **));
extern char **environ;
extern int  errno;

extern int terminal_wraps;     /* in ANSI emulator */

#define window_max 16          /* the maximum number of windows */
typedef struct unix_window {
        WORD in_use;
        BYTE in[16];     /* input buffer whilst processing escape sequences */
        BYTE out[16];    /* ditto */
        WORD matching;   /* the index of the escape sequence I am currently */
                         /* trying to match */
        int  how_many;
        int  buf_index;
        UBYTE buf[64];
} unix_window;
PRIVATE unix_window window_tab[window_max];


PRIVATE void fn( initialise_dumb_terminal,        (void));
PRIVATE void fn( restore_dumb_terminal,           (void));
PRIVATE void fn( initialise_keyboard_strings,     (void));
        void fn( switch_window,                   (int));
PRIVATE WORD fn( extract_from_window,             (unix_window *));
PRIVATE void fn( initialise_signals,              (void));

BYTE termcap_data[1024];
BYTE termcap_buffer[1024], *termcap_index;
PRIVATE string null_string = "";
string termcap_cls, termcap_ceol, termcap_move, termcap_inv, termcap_nor;
string termcap_bell;

void unix_initialise_devices()
{ char *term_env = getenv("TERM");
  int  x;

  if (term_env eq (char *) NULL)
   { printf("Unable to get TERM environment variable.\n");
     longjmp(exit_jmpbuf, 1);
   }

  switch(tgetent(termcap_data, term_env))
   { case -1 : printf("Unable to open termcap database.\n");
               longjmp(exit_jmpbuf, 1);
     case  0 : printf("Terminal %s not found in termcap database.\n", term_env);
               longjmp(exit_jmpbuf, 1);
   }   

  termcap_index = termcap_buffer;
  termcap_cls   = tgetstr("cl", &termcap_index);
  termcap_ceol  = tgetstr("ce", &termcap_index);
  termcap_move  = tgetstr("cm", &termcap_index);
  termcap_bell  = tgetstr("bl", &termcap_index);
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
     window_tab[x].in[0]      = '\0';
     window_tab[x].out[0]     = '\0';
     window_tab[x].matching   = -1;
     window_tab[x].how_many   = 0;
     window_tab[x].buf_index  = -1;
   }

  initialise_keyboard_strings();

  initialise_signals();

  real_windows = 0;
  initialise_dumb_terminal();
}

void unix_restore_devices()
{
  restore_dumb_terminal();
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

PRIVATE BYTE *fn( parse_escape_sequence, (char *));
PRIVATE void fn( add_debug_sequence,     (char *, int, int));

typedef struct key_definition {
        char     local_sequence[16];
        char     *helios_sequence;
        int      debugging_option;
} key_definition;

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

typedef struct termcap_definition {
        char     termcap_name[4];
        char     helios_sequence[6];
} termcap_definition;

/**
*** RTE : there are different ways of finding out about the various special keys
*** currently being used for e.g. backspace. TRY should be 0 or 1
**/
#define TRY 1

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
      { key_definition *key   = &(key_table[keyindex_max++]);
        strcpy(key->local_sequence, temp);
        key->helios_sequence  = def->helios_sequence;
        key->debugging_option = 0; 
      }
   }

#if (TRY==0)
                     /* The various suns support the termios system */
                     /* but other unix machines may not. Those have */
                     /* to make do with the sgtty junk              */
  { struct termios t;
    if (ioctl(0, TCGETS, &t) eq 0)
     { key_definition *key = &(key_table[keyindex_max++]);
       key->local_sequence[0] = t.c_cc[VERASE];
       key->helios_sequence   = "\010\0";
       key->debugging_option  = 0;

       if (t.c_cc[VINTR] ne 0x03)   /* ctrl-C */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.c_cc[VINTR];
          key->helios_sequence   = "\003\0";
          key->debugging_option  = 0;
        }       
       if (t.c_cc[VEOF] ne 0x04)   /* ctrl-D */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.c_cc[VEOF];
          key->helios_sequence   = "\004\0";
          key->debugging_option  = 0;
        }       
       if (t.c_cc[VSTOP] ne 0x13)  /* ctrl-S */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.c_cc[VSTOP];
          key->helios_sequence   = "\023\0";
          key->debugging_option  = 0;
        }       
       if (t.c_cc[VSTART] ne 0x11) /* ctrl-Q */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.c_cc[VSTART];
          key->helios_sequence   = "\021\0";
          key->debugging_option  = 0;
        }       
     }
  }

#endif
#if (TRY==1)
  { struct sgttyb t;
    if (ioctl(0, TIOCGETP, &t) eq 0)
     { key_definition *key = &(key_table[keyindex_max++]);
       key->local_sequence[0] = t.sg_erase;
       key->helios_sequence   = "\010\0";
       key->debugging_option  = 0;
     }
  }

  { struct tchars t;
    if (ioctl(0, TIOCGETC, &t) eq 0)
     { key_definition *key;
       if (t.t_intrc ne 0x03)   /* interrupt not ctrl-C */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.t_intrc;
          key->helios_sequence   = "\003\0";
          key->debugging_option  = 0;
        }
       if (t.t_startc ne 0x11)  /* ctrl-Q */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.t_startc;
          key->helios_sequence   = "\021\0";
          key->debugging_option  = 0;
        }
       if (t.t_stopc ne 0x13)  /* ctrl-S */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.t_stopc;
          key->helios_sequence   = "\023\0";
          key->debugging_option  = 0;
        }
       if (t.t_eofc ne 0x04)  /* ctrl-D */
        { key = &(key_table[keyindex_max++]);
          key->local_sequence[0] = t.t_eofc;
          key->helios_sequence   = "\004\0";
          key->debugging_option  = 0;
        }
     }
  }
#endif

  temp = parse_escape_sequence("escape_sequence");
  if (temp eq NULL)
   { printf("An escape_sequence must be defined in the configuration file.\r\n");
     printf("Please edit the file host.con.\n");
     longjmp(exit_jmpbuf, 1);
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

  qsort(key_table, keyindex_max, sizeof(key_definition), key_comp); 
#ifdef never
{ int i;
  for (i = 0; i < keyindex_max; i++)
   printf("local %x %x %x %x : debug %d\n",
    key_table[i].local_sequence[0],
    key_table[i].local_sequence[1],
    key_table[i].local_sequence[2],
    key_table[i].local_sequence[3],
    key_table[i].debugging_option);

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
 
  if (*entry eq '#')                   /* specified in config */
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
       { printf("Failed to parse escape sequence %s in host.con file.\r\n", name); 
         longjmp(exit_jmpbuf, 1);
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
     longjmp(exit_jmpbuf, 1);
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

     if (*input < *pattern)  /* because the key sequences are sorted the matching */
       break;                /* has failed */

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
*** Dumb terminal support : this is fairly easy, most of the code being the
*** same as on the PC and ST versions of ServerWindows. The difference is
*** that check_windows has been eliminated, and the window-switching test
*** has been moved to dumb_send_to_window.
**/


#if (TRY==0)
PRIVATE struct termios old_term, cur_term;
#endif
#if (TRY==1)
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
  window_lines = tgetnum("li");
  window_cols  = tgetnum("co");

  if (!isatty(0) || !isatty(1))
   { printf("Standard streams may not be redirected.\n");
     longjmp(exit_jmpbuf, 1);
   }

#if (TRY==0)

  ioctl(0, TCGETS, &old_term);
  ioctl(0, TCGETS, &cur_term);
  cur_term.c_lflag &= ~(ISIG + ICANON + ECHO + ECHOE + ECHOK + 
                        ECHONL);
  cur_term.c_cc[VMIN]  = 0;
  cur_term.c_cc[VTIME] = 0;
  ioctl(0, TCSETS, &cur_term);
  
#endif

#if (TRY==1)
                            /* set mode RAW ,ctrl \ gets out */
  ioctl(0,TIOCGETP,&old_term);
  ioctl(0,TIOCGETP,&cur_term);
  cur_term.sg_flags &=(~ECHO);     /* no echo */
  cur_term.sg_flags |=RAW;         /* raw */
  cur_term.sg_flags &=(~CRMOD);
  ioctl(0,TIOCSETP,&cur_term);
  terminal_changed = 1;
#endif

  { char *text = get_config("SERVER_WINDOWS_NOPOP");
    if (text eq (char *) NULL)
     nopop = 0;
    else
     nopop = 1;
  }
/**
*** RTE : this ioctl should put the termninal into non-blocking mode,
***       i.e. reads from the terminal always return immediately. This is
***       essential. Sadly, under the RTE the ioctl may not work and some
***       other approach may have to be used.
**/  

  if (ioctl(0, FIONBIO, 0) ne 0)
   { printf("FIONBIO ioctl failed : initialise dumb terminal, rtelocal.c\n");
     longjmp(exit_jmpbuf, 1);
   }
}

PRIVATE void restore_dumb_terminal()
{ if (terminal_changed)
#if (TRY==0)
   ioctl(0, TCSETSW, &old_term); 
#endif
#if (TRY==1)
   ioctl(0, TIOCSETP, &old_term);
#endif
}

#undef TRY

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

WORD create_a_window(name)
char *name;
{ int i;
  for (i = 0; i < window_max; i++)
   if (!window_tab[i].in_use)
    { window_tab[i].in_use   = 1;
      window_tab[i].in[0]    = '\0';
      window_tab[i].out[0]   = '\0';
      window_tab[i].matching = -1;
      current_handle         = &(window_tab[i]);
      return((WORD) &(window_tab[i]));
    }

  return((WORD) NULL);
  use(name)  
}

void close_window(handle)
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

void window_size(handle, x, y)
unix_window *handle;
WORD *x, *y;
{
 *x = window_cols; *y = window_lines;
 use(handle)
}

PRIVATE void switch_window(direction)
int direction;
{ Window *window;
 
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


void send_to_window(data, handle)
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
 
  write(1, data, ANSI_buffer_count); 
}

WORD read_char_from_keyboard(handle)
unix_window *handle;
{ PRIVATE int  how_many = 0, index = -1;
  PRIVATE BYTE buf[64];

  if (handle->out[0] ne '\0')
   return(extract_from_window(handle));

  if (handle ne current_handle)
   return(-1);

  if ((how_many eq 0) && (index eq -1))
   { how_many = read(0, buf, 64);
     if (how_many eq -1) how_many = 0;
     index = 0;
   }

                  /* the escape key deserves a special case      */
                  /* A single escape character may introduce an  */
                  /* escape sequence or it may be the escape key */
                  /* To distinguish between them, wait 1/4 sec   */
                  /* for any more data, and if there is none     */
                  /* it really is the escape key.                */
  if ((how_many eq 1) && (buf[0] eq 0x1b) && (handle->matching eq -1)) 
   { int temp;
     goto_sleep(OneSec / 4);
     temp = read(0, &(buf[1]), 63);
     if (temp <= 0)
      { index = 0; how_many = 0; return(0x1B); }
     else
      how_many += temp;

   }

  while (how_many > 0) 
    { int x = add_to_window(buf[index++], handle);
      how_many--; 
      if (x ne -1) return(x);      
    } 

  { int x = add_to_window(0, handle);
    if (x eq -1)
     index = -1;
    return(x);
  }
}


/**
*** RTE : Signal handling. The RTE does not support signals at present so this
*** routine is a no-op. If you put them in be warned, not all signals are 
*** available on all machines.
**/

#ifdef NEVER
PRIVATE void exit_interrupt()
{ Special_Exit = 1;
}
#endif

PRIVATE void initialise_signals()
{
#ifdef NEVER	 
  signal(SIGHUP   , func(exit_interrupt));
  signal(SIGINT   , func(exit_interrupt));
  signal(SIGQUIT  , func(exit_interrupt));
  signal(SIGILL   , SIG_IGN);
  signal(SIGTRAP  , SIG_IGN);
  signal(SIGEMT   , SIG_IGN);
  signal(SIGFPE   , SIG_IGN);
  signal(SIGBUS   , SIG_IGN);
  signal(SIGSEGV  , SIG_IGN);
  signal(SIGKILL  , func(exit_interrupt));
  signal(SIGSYS   , SIG_IGN);
  signal(SIGPIPE  , SIG_IGN);
  signal(SIGALRM  , SIG_IGN);
  signal(SIGTERM  , func(exit_interrupt));
  signal(SIGURG   , SIG_IGN);
  signal(SIGIO    , SIG_IGN);
  signal(SIGUSR1  , SIG_IGN);
  signal(SIGUSR2  , SIG_IGN);
  signal(SIGABRT  , SIG_IGN);
  signal(SIGSTOP  , func(exit_interrupt));
  signal(SIGTSTP  , func(exit_interrupt));
  signal(SIGCONT  , SIG_IGN);
  signal(SIGCHLD  , SIG_IGN);
  signal(SIGTTIN  , SIG_IGN);
  signal(SIGTTOU  , SIG_IGN);
  signal(SIGXCPU  , SIG_IGN);
  signal(SIGXFSZ  , SIG_IGN);
  signal(SIGVTALRM, SIG_IGN);
  signal(SIGPROF  , SIG_IGN);
  signal(SIGWINCH , SIG_IGN);
  signal(SIGLOST  , SIG_IGN);
#endif
}

/**
*** The goto_sleep() routine
**/
void goto_sleep(delay)
long delay;
{
  if (delay < OneSec)
   sleep(1);
  else
   sleep(delay / OneSec);
}

/**
*** RTE : gettimeofday() is the most likely time routine to work.
***
*** Get the current system time, as a Unix time stamp. This is needed all over
*** the place.
**/

WORD get_unix_time()
{ struct timeval t;

  gettimeofday(&t, NULL);
  return(t.tv_sec);
}



/*------------------------------------------------------------------------
--
-- The following code implements the local file IO routines
--
------------------------------------------------------------------------*/

typedef int stream;

WORD object_exists(name)
char *name;
{ if (stat(name, &searchbuffer) eq -1)
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
{ if (unlink(name) ne 0)
   { convert_error(); return(FALSE); }
  else
   return(TRUE);
}

WORD rename_object(from, to)
char *from, *to;
{ if (rename(from, to) ne 0)
   { convert_error(); return(FALSE); }
  else
   return(TRUE);
}

WORD create_file(name)
char *name;
{ int handle = open(name, O_RDWR | O_CREAT | O_TRUNC,0666); 
                              /* create if not exist, else truncate */
  if (handle < 0)                                  /* did an error occur ? */
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
     case S_IFSOCK : 
     case S_IFREG  : type = Type_File; break;
     case S_IFCHR  :
     case S_IFBLK  :
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
{ time_t timestamps[2]; 
  timestamps[0] = time;
  timestamps[1] =time;
  return (utime(name,timestamps) eq 0) ? TRUE : FALSE;
}

/**
*** RTE : statfs may not be available
**/
WORD get_drive_info(name, reply)
char *name;
servinfo *reply;
{ 
#if (0)                   /* statfs not supported */
  reply->size  = 0L;
  reply->used  = 0L;
  reply->alloc = 512;
  return(TRUE);

#else 

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
   { case S_IFSOCK : return(socket_open(name, Heliosmode));
     case S_IFCHR  :
     case S_IFBLK  : Server_errno = EC_Error + SS_IOProc + EG_WrongFn +
                     EO_Object;
                     return(0);
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
***
*** RTE : there is some confusion as to whether the routines use
*** struct dirent or struct direct.
**/

#if (1)
#define dirent direct
#endif

PRIVATE WORD fn( add_node, (List *, char *, char *));

WORD search_directory(pathname, header)
char    *pathname;
List    *header;
{ PRIVATE char buf[PATH_MAX];
  char *end_ptr;
  int   count = 0;
  int   dirp, result;
  struct dirent dirbuf;
 
  strcpy(buf, pathname);
  strcat(buf, "/");
  end_ptr = buf + strlen(buf);
 
  dirp = open(pathname, O_RDONLY);
  if (dirp eq -1)
   { convert_error();  return (-1); }

  for (result = read(dirp, (BYTE *) &dirbuf, sizeof(struct dirent));
       result >= 0;
       result = read(dirp, (BYTE *) &dirbuf, sizeof(struct dirent)) )
  { 
    strcpy(end_ptr, dirbuf.d_name);
    count++;
    if(add_node(header,buf, dirbuf.d_name) eq FALSE) 
    { FreeList(header);
      closedir(dirp);
      return(-1);
    }
  }

  close(dirp);
  return (count);
}

/**
*** When the search through the directory has revealed another entry, all
*** the information about this entry will be stored in searchbuffer. It
*** is necessary to convert this information to the Helios DirEntry
*** structure.
**/

PRIVATE word add_node(header,pathname, last_bit)
     List *header;
     char *pathname, *last_bit;
{
  DirEntryNode *newnode;
  word type;
 
  if (stat(pathname,&searchbuffer) ne 0)
  { convert_error(); return(FALSE); }
 
  switch(searchbuffer.st_mode & S_IFMT)
   { case S_IFDIR  : type = Type_Directory; break;
     case S_IFIFO  : type = Type_Fifo; break;
     case S_IFSOCK : 
     case S_IFREG  : type = Type_File; break;
     case S_IFCHR  :
     case S_IFBLK  :
     default       : type = Type_Device; 
   }

  newnode = (DirEntryNode *) malloc(sizeof(DirEntryNode));
  unless(newnode)
   { Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Server;
     return(FALSE);
   }
  
  memset(newnode, 0, sizeof(DirEntryNode));
  (newnode->direntry).Type     = swap(type);
  (newnode->direntry).Flags    = swap(0L);
  (newnode->direntry).Matrix   = swap( (type eq Type_Directory) ? 
                                       DefDirMatrix : DefFileMatrix  );
  
  strncpy((newnode->direntry).Name, last_bit, 31);

  AddTail(newnode, header);    /* put the node at the end of the list */
  
  return(TRUE);
}



/**
*** Error conversion : this takes a unix error code held in errno and converts it
*** to a Helios error code in Server_errno.
**/
PRIVATE void convert_error()
{ switch(errno)
   { 
     case EPERM        :       /* Not owner */
           Server_errno = EC_Error + SS_IOProc + EG_Protected + EO_File;
           return;
     case ENOENT       :       /* No such file or directory */
           Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;
           return;
     case EINTR        :       /* Interrupted system call */
           Server_errno = EC_Warn + SS_IOProc + EG_Exception + EO_Stream;
           return;
     case EIO          :       /* I/O error */
           Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_File;
           return;
     case ENXIO        :       /* No such device or address */
           Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_File;
           return;
     case EBADF        :       /* Bad file number */
           Server_errno = EC_Error + SS_IOProc + EG_Unknown + EO_Stream;
           return;
     case ENOMEM       :       /* Not enough core */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Server;
           return;
     case EACCES       :       /* Permission denied */
           Server_errno = EC_Error + SS_IOProc + EG_Protected + EO_File;
           return;
     case EFAULT       :       /* Bad address */
           Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Server;
           return;
     case EBUSY        :       /* Mount device busy */
           Server_errno = EC_Warn + SS_IOProc + EG_InUse + EO_Object;
           return;
     case EEXIST       :       /* File exists */
           Server_errno = EC_Error + SS_IOProc + EG_Create + EO_Object;
           return;
     case EXDEV        :       /* Cross-device link */
           Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Object;
           return;
     case ENOTDIR      :       /* Not a directory*/
           Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_File;
           return;
     case EISDIR       :       /* Is a directory */
           Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Directory;
           return;
     case EINVAL       :       /* Invalid argument */
           Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Server;
           return;
     case EFBIG        :       /* File too large */
           Server_errno = EC_Error + SS_IOProc + EG_WrongSize + EO_File;
           return;
     case ENFILE       :       /* File table overflow */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Stream;
           return;
     case EMFILE       :       /* Too many open files */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Stream;
           return;
     case ENOSPC       :       /* No space left on device */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Route;
           return;
     case ESPIPE       :       /* Illegal seek */
           Server_errno = EC_Error + SS_IOProc + EG_WrongFn + EO_Fifo;
           return;
     case EROFS        :       /* Read-only file system */
           Server_errno = EC_Error + SS_IOProc + EG_Protected + EO_Route;
           return;
     case EMLINK       :       /* Too many links */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Link;
           return;
     case EPIPE        :       /* Broken pipe */
           Server_errno = EC_Error + SS_IOProc + EG_Broken + EO_Fifo;
           return;
     case EWOULDBLOCK  :       /* Operation would block */
           Server_errno = EC_Warn + SS_IOProc + EG_InUse + EO_Object;
           return;
     case ELOOP        :       /* Too many levels of symbolic links */
           Server_errno = EC_Error + SS_IOProc + EG_WrongSize + EO_Link;
           return;
     case ENAMETOOLONG :       /* File name too long */
           Server_errno = EC_Error + SS_IOProc + EG_Name + EO_File;
           return;
     case ENOTEMPTY    :       /* Directory not empty */
           Server_errno = EC_Error + SS_IOProc + EG_InUse + EO_Directory;
           return;
     case EDQUOT       :       /* Disc quota exceeded */
           Server_errno = EC_Error + SS_IOProc + EG_NoMemory + EO_Route;
           return;
   }
}

/**
*** network support routines
**/

bool network_reset(processor)
WORD processor;
{ (void) superReset(processor + 1);
  return(TRUE);
}

bool network_analyse(processor)
WORD processor;
{ (void) superAnalyse(processor + 1);
  return(TRUE);
}

bool network_connect(source, slink, dest, dlink)
WORD source, slink, dest, dlink;
{ (void) superConnect(source + 1, slink, dest+1, dlink);
  return(TRUE);
}

bool network_disconnect(source, slink)
WORD source, slink;
{ (void) superDisconnect(source +1, slink);
  return(TRUE);
}

bool network_enquire(source, slink)
WORD source, slink;
{ WORD *vec = (WORD *) mcb->Data;

  vec[0] = -1L; vec[1] = -1L; vec[2] = ND_SOFTWIRED;  
  return(TRUE);
}

