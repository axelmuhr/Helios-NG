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
/* RCSId: $Id: window.h,v 1.3 1993/09/15 08:33:48 paul Exp $ */


/* @@@ check we really need this! */
/*- ANSI memmove should be ok (see end window.c) */
#define memmove mymemmove	/* To avoid name clash with string.h */
GLOBAL void memmove(UBYTE *, UBYTE *, WORD);

#define eq 		==
#define ne 		!=
#define ReplyOK	        0
#define Name_Max        256
#define Command_Max     512
#define Console_Max     80	/* max chars output in one console_write */
#define preserve	1
#define release		2
#define Stacksize	3000
#define BootLink	0
#define LongTimeout	20 * OneSec
#define Message_Limit	1024		/* max size of read/write chunks */
/**
*** Error code : Server Task Timeout
**/
#define ST_Timeout (EC_Recover + SS_Window + EG_Timeout + EO_Link)


extern void _Trace(int, ...);
GLOBAL void Return(MCB *, WORD FnRc, WORD ContSize, WORD DataSize,
		    WORD Preserve);
GLOBAL void SendError(MCB *, WORD, WORD Preserve);
GLOBAL void SendOpenReply(MCB *, string name, WORD type, WORD flags,
			   Port reply);

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
#ifndef __TRAN
			Semaphore	CountSem;
#endif
			Port		SelectPort;
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


/* vdev device specific routines */
extern	void vdev_init(void) ;
extern	void vdev_info(short *rows, short *cols) ;
extern	void vdev_clear_screen(void) ;
extern	void vdev_putstr(char *text) ;
extern	void vdev_moveto(int row,int col) ;
extern	void vdev_beep(void) ;
extern	void vdev_backspace(void) ;
extern	void vdev_carriage_return(void) ;
extern	void vdev_linefeed(void) ;
extern	void vdev_set_foreground(int colour) ;
extern	void vdev_set_background(int colour) ;
extern	void vdev_set_inverse(bool flag) ;
extern	void vdev_set_bold(bool flag) ;
extern	void vdev_set_underline(bool flag) ;
extern	void vdev_set_italic(bool flag) ;
