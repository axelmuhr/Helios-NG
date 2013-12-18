/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   A N S I   E M U L A T O R		--
--                 ---------------------------------------              --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      window.h                                                        --
--                                                                      --
--  Author:  BLV 19/4/89                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: window.h,v 1.1 1990/10/17 13:45:16 bart Exp $ (C) Copyright 1988, Perihelion Software Ltd. */ 
/* SccsId: 1.5 3/5/90  Copyright (C) 1989, Perihelion Software Ltd. */


/**
*** The following structures are used for handling windows and the console
*** device. Microwave is used to handle cooked input processing (pun
*** definitely intended), Screen is used by the ANSI emulator, and Window is
*** used to point at a window structure.
**/
#define       Console_limit 256

typedef struct Microwave {
	Semaphore lock;
        UBYTE  buffer[Console_limit];  /* where data is processed    */
        int    count;                  /* current position in buffer */
} Microwave;

#define Cooked_EOF  1               /* ctrl-D detected         */
#define Cooked_EOD  2               /* no more data in buffer  */   
#define Cooked_Done 3               /* Read has been satisfied */

                /* this structure is used to keep track of event handlers */
typedef struct { WORD port;
                 WORD *ownedby; /* to keep track of streams */
                 Semaphore lock;
} event_handler;

typedef struct Screen { BYTE          **map;
			BYTE	      *whole_screen;
                        int           Rows;
                        int           Cols;
                        int           Cur_x;
                        int           Cur_y;
                        int           mode;
                        int	      flags;
                        int           args[5];
                        int           *args_ptr;
                        int           gotdigits;
} Screen;

#define ANSI_in_escape     0x01
#define ANSI_escape_found  0x02
#define ANSI_firstdigit    0x04
#define ANSI_dirty         0x08

typedef struct Window { ObjNode		Node;
			Semaphore	in_lock;
			Semaphore	out_lock;
                        Attributes	attr;
                        event_handler	break_handler;
                        Microwave	cooker;
                        Screen		screen;
                        UBYTE		Table[Console_limit];
                        WORD		handle;
                        int		head, tail;
                        int		XOFF;
                        int		stream_count;
} Window;

