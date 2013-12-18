/*------------------------------------------------------------------------
--                                                                      --
--             H E L I O S   U N I X  L I N K  I / O   S Y S T E M      --
--             ---------------------------------------------------      --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      smmouse.c                                                        --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90\ Copyright (C) 1989, Perihelion Software Ltd.        */

#include "../helios.h"

/**
*** NB : all of this code is untested. It is also unfinished, since the
*** code to convert host mouse and keyboard events to Helios events is
*** not supplied. It does, however, give a good basis with which to start.
**/
/**
*** This code implements support for the mouse and raw keyboard devices.
*** There are 5 main routines for each device. The initialise routine is
*** called when the Server starts up or reboots. The tidy routine is called
*** when the Server exits or just before a reboot. The start routine is
*** called when Helios wants mouse events. The stop routine is called if
*** Helios no longer wants mouse messages. The read_events routine is
*** a polling routine to check for new data.
***
*** At present the init routine is a no-op, but it may have to reset some
*** more statics. The tidy routine closes the file descriptor to the device
*** if it is currently open. The start routine opens the device, and
*** if successful it enables a MultiWait - the Server will now select() on
*** the file descriptor as well as the link and any other devices being
*** polled. The stop routine disables the MultiWait and closes the file
*** descriptor.
**/

PRIVATE int  mouse_fd    = -1;
PRIVATE WORD mouse_ready = 0L;

void initialise_mouse()
{
}

void tidy_mouse()
{ if (mouse_fd ne -1)
   close(mouse_fd);
  mouse_fd = -1;
  mouse_ready = 0L;
}

void start_mouse()
{ 
  mouse_fd = open("/dev/mouse", O_RDWR);
  if (mouse_fd ne -1)
   { AddMultiwait(Multi_MouseInput, &mouse_ready, mouse_fd);
     mouse_ready = 0L;
   }
}

void stop_mouse()
{ if (mouse_fd ne -1)
   { ClearMultiwait(Multi_MouseInput, &mouse_ready, mouse_fd);
     close(mouse_fd);
     mouse_fd = -1;
   }
  mouse_ready = 0L;
}

/**
*** This is the routine which involves some work. The flag mouse_ready
*** will be set if there is some data outstanding on the file descriptor.
*** This data should be read in and analysed, and the flag should be
*** cleared. The results should be made known to the main I/O Server
*** by calls to new_mouse(). That routine takes three arguments,
*** a 16-bit x position, a 16-bit y position, and a 32-bit value
*** indicating the current button state. The positions are pseudo-absolute,
*** the Helios mouse device works in a 32768*32768 coordinate system
*** which wraps around. This may seem a strange way of doing it, but
*** it allows several different ways of recovering from errors if messages
*** get lost.
***
*** For more details of the mouse mechanism, see the PC implementation
*** in ibm/pclocal.c
**/

extern void fn( new_mouse, (SHORT x, SHORT Y, WORD buttons));

void read_mouse_events()
{ if (mouse_ready)
   { mouse_ready = 0L;
   }
}

/**
*** Similar code for the keyboard. Some additional work may be needed
*** when opening the stream, to put the keyboard into the right mode.
*** Helios needs scancodes and key-up key-down events for all keys,
*** without any translation. X-windows handles all the keyboard table
*** translation, shift keys, auto-repeat, etc. Of course your keyboard
*** is unlikely to correspond to the default X keyboard, and you may have
*** to inform X about the correct keyboard mappings.
**/
PRIVATE int  keyboard_fd    = -1;
PRIVATE WORD keyboard_ready = 0L;

void initialise_keyboard()
{
}

void tidy_keyboard()
{ if (keyboard_fd ne -1)
   close(keyboard_fd);
  keyboard_fd    = -1;
  keyboard_ready = 0L;
}

void start_keyboard()
{ 
  keyboard_fd = open("/dev/kbd", O_RDWR);
  if (keyboard_fd ne -1)
   { AddMultiwait(Multi_KeyboardInput, &keyboard_ready, keyboard_fd);
     keyboard_ready = 0L;
   }
}

void stop_keyboard()
{ if (keyboard_fd ne -1)
   { ClearMultiwait(Multi_KeyboardInput, &keyboard_ready, keyboard_fd);
     close(keyboard_fd);
     keyboard_fd = -1;
   }
  keyboard_ready = 0L;
}

/**
*** read_keyboard_events() is a polling routine to handle raw keyboard
*** events. Like the mouse polling routine, it should read data from
*** the stream, convert this data to Helios events, and call a
*** new_keyboard() routine with the data. new_keyboard() takes two arguments,
*** an event which can be up or down, and a key which should be the
*** scan-code. Again, consult the PC sources for more details.
***
*** It is very desirable for read_keyboard_events() to be able to cope
*** with the debugging options, e.g. reboot or enable message debugging.
*** Unfortunately I have not thought of a good way of doing it. If it
*** is possible to take the events and map them to ASCII internally,
*** they could be filtered through the same or similar finite state
*** machine in sun/sunlocal.c, which handles the debugging options.
*** The work involved is non-trivial.
**/
extern void fn( new_keyboard, (WORD event, WORD key));

void read_keyboard_events()
{ if (keyboard_ready)
   { keyboard_ready = 0L;
   }
}

