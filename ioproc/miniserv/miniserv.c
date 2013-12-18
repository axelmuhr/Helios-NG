/*------------------------------------------------------------------------
--                                                                      --
--                 H E L I O S   M I N I   S E R V E R                  --
--                 -----------------------------------                  --
--                                                                      --
--             Copyright (C) 1988, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
--      miniserv.c                                                      --
--                                                                      --
--  Author:  BLV 24/8/88                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 1.4 14/3/90  Copyright (C) 1988, Perihelion Software Ltd. */

/**
*** The usual #include's
**/
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <dos.h>
#include <bios.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <stddef.h>

/**
*** standard type definitions etc., from iohelios.h
**/
typedef  long			WORD    ;	/* a machine word, 32 bits      */
typedef  unsigned long  UWORD   ;	/* a machine word, 32 bits      */
typedef	 WORD			word	;	/* another synonym		*/
typedef  short int		SHORT	;	/* a 16 bit word 		*/
typedef  unsigned short USHORT	;	/* an unsigned 16 bit value	*/
typedef  unsigned char  BYTE    ;	/* a byte, used in place of char*/
typedef  BYTE			byte	;	/* a synonym			*/
typedef  unsigned char  UBYTE   ;	/* an unsigned byte             */
typedef  char			*STRING	;	/* character string             */
typedef  char 			*string	;	/* synonym			*/
typedef  int			bool	;	/* boolean value		*/

typedef  void		(*VoidFnPtr)();	/* pointer to void function	*/
typedef  word		(*WordFnPtr)();	/* pointer to word function	*/

#define PUBLIC		extern   	/* an exported symbol		*/
#define PRIVATE		static		/* an unexported symbol		*/
#define FORWARD		static		/* forward static proc reference	*/

#define forever		for(;;)
#define unless(x)	if(!(x))
#define until(x)	while(!(x))
#define elif(x)		else if(x)
#define eq			==
#define ne			!=

#define TRUE		1
#define true		1
#define FALSE		0
#define false		0

/**
*** These are the routines in miniasm.asm. Sadly, some link IO routines return
*** 0 for success, non-0 for failure.
**/
PUBLIC	bool fetch_block(int count, void *data, int timeout);
PUBLIC	bool send_block(int count, void *data, int timeout);
PUBLIC	bool byte_to_link(int x);
PUBLIC	bool byte_from_link(int *ptr);
PUBLIC	bool rdrdy(void);
PUBLIC	void reset(void);
PUBLIC	void init_link(void);
PUBLIC  int  link_base;
PUBLIC	int  int_level;
PUBLIC	int  silent_mode;
PUBLIC	int  set_interrupts(void);
PUBLIC  void vbios_save(void);
PUBLIC  void vbios_restore(void);
PUBLIC  void vbios_cls(void);
PUBLIC 	void vbios_movecursor(int, int);
PUBLIC  void vbios_outputch(int);
PUBLIC  void vbios_scroll(void);
PUBLIC  void vbios_bell(void);
PUBLIC  BYTE vbios_attr, vbios_x, vbios_y;
PUBLIC	void tsr(void);

/**
*** Some manifests for link IO
**/
#define LongTimeout		32000
#define ShortTimeout	50

/**
*** Here is a definition of the protocol
**/
#define Config_flags_Slave 0x02
#define Config_flags_Nopop 0x04
#define PC				1
#define DP2				2
#define yyMessage_Limit	16384
#define Message_Limit	8192
#define xxMessage_Limit	4096
#define Pro_Synch	0
#define Pro_IOServ	1
#define Pro_Command	2
#define Pro_Message	3
#define Pro_Poll	4
#define Pro_Die		5
#define Pro_Sleep	6
#define Pro_Wakeup	7

#define Fun_OpenFile	  1
#define OpenMode_ReadOnly	1
#define OpenMode_WriteOnly	2
#define OpenMode_ReadWrite	3
#define Fun_CloseFile	  2
#define Fun_ReadFile	  3
#define Fun_WriteFile	  4
#define Fun_SeekInFile	  5
#define Seek_start		  1
#define Seek_end		  2
#define Fun_CreateFile	  6
#define Fun_DeleteFile	  7
#define Fun_Rename		  8
#define Fun_Locate		  9
#define Fun_ReadDir		 10
#define Fun_CreateDir	 11
#define Fun_RemoveDir	 12
#define Fun_FileInfo	 13
#define Fun_DiskUsage	 14
#define Fun_ScreenWrite	 15
#define Fun_ChangeDate   16

#define Rep_Success		128
#define Rep_NotFound	129

/**
*** Polling device identifiers. At present only the console is defined, but
*** others may be added later.
**/
#define Poll_Console      1

typedef struct FullHead {
	BYTE	protocol;
	BYTE	fncode;
	BYTE	extra;
	UBYTE	highsize;
	UBYTE	lowsize;
} FullHead;
#define sizeofFullHead	5
PRIVATE FullHead	fullhead;

typedef struct Head	{
	UBYTE	fncode;
	UBYTE	extra;
	UBYTE	highsize;
	UBYTE	lowsize;
} Head;
#define sizeofHead		4
PRIVATE Head		head;

#define Nothing_t	0
#define File_t		1
#define Dir_t		2

PRIVATE BYTE big_buffer[Message_Limit + 100];
#define local_name (&(big_buffer[0]))
PRIVATE char Helios_Directory[64];
PRIVATE char drives[32];
PRIVATE int nopop  = 0;
PRIVATE int background = 0;
 
/**
*** Forward declarations
**/
FORWARD int boot_transputer(void);
FORWARD int miniserver(void);
FORWARD WORD unixtime(void);
FORWARD WORD get_unixtime(int date, int time);
FORWARD void ioserver_protocol(void);
FORWARD void command_protocol(void);
FORWARD void message_protocol(void);
FORWARD void init_files(void);
FORWARD void tidy_files(void);
FORWARD void outputch(int ch); 
FORWARD void outputstr(char *);
FORWARD void outputhex(int);
FORWARD void outputaddr(void far *);
FORWARD int  send_wakeup(void);

int  handle_data(void);
PUBLIC  int  my_dos_open(char *, int);

/**
*** And at long last here is some code. main() intercepts some of the
*** interrupts like critical error and divide by zero, boots up the
*** transputer, and enters the miniserver. It should never return from the
*** miniserver, I think.
**/

int main(int argc, char **argv)
{
  strcpy(Helios_Directory, "c:\\helios");
  strcpy(drives, "c");

  fclose(stdaux); fclose(stdprn);

  for (; argc > 1; argc--)
   { argv++;

     if (!strcmp(*argv, "nopop"))
      nopop = TRUE;
     elif (!strcmp(*argv,"silent"))
      silent_mode = 1;
     elif (!strncmp(*argv, "base=", 5))
        link_base = strtol(&((*argv)[5]), NULL, 0);
     elif (!strncmp(*argv, "drives=",7))
        strcpy(drives, &((*argv)[7]));
     elif (!strncmp(*argv, "int=",4))
      { int_level = strtol(&((*argv)[4]), NULL, 0);
        if ((int_level ne 3) && (int_level ne 5))
         { outputstr("Link interrupt level must be 3 or 5.\r\n");
           return(0);
         }
      }
     else
       strcpy(Helios_Directory, *argv);
   }

  if ((Helios_Directory[1] eq ':') && (Helios_Directory[2] ne '\\'))
   { char tmpbuf[64];
     tmpbuf[0] = Helios_Directory[0];
     strcpy(&(tmpbuf[1]), ":\\");
     strcat(&(tmpbuf[3]), &(Helios_Directory[2]));
     strcpy(&(Helios_Directory[0]), &(tmpbuf[0]));
   }

  init_link();
  reset();
  { long x;
    if (byte_to_link(0)) goto no_trannie;
    x = 0x80000100;
    if (send_block(4, &x, 32000)) goto no_trannie;
    x = 0x12345678;
    if (send_block(4, &x, 32000)) goto no_trannie;
    if (byte_to_link(1)) goto no_trannie;
    x = 0x80000100;
    if (send_block(4, &x, 32000)) goto no_trannie;
    if (fetch_block(4, &x, 32000)) goto no_trannie;
    if (x ne 0x12345678) goto no_trannie;
  }

  init_files();
  if (!set_interrupts())
   { outputstr("\r\nThe miniserver appears to have been installed already.\r\n");
     return(1);
   }
  install_link_int();
  tsr();

no_trannie:
  outputstr("Failed to access transputer.\r\n");
  return(0);
}

/**
*** This routine is called from assembler, when the hstart program triggers
*** int 0x61. 
**/
int wakeup(void)
{ vbios_save();   /* save the current screen state */

  if (background)
   { if (!send_wakeup())
      { background = 0; goto done; }
   }
  else
   { if (!boot_transputer())
      { background = 0; goto done; }
   }

  background = FALSE;

  forever
   { 
     if (!miniserver())
      break;
     
     if (!boot_transputer())
      break;
   }
done:

  vbios_restore();

  return(background);
}


PRIVATE int miniserver()
{ int x, shift, i, j;
  WORD  starttime = unixtime();

  forever
   {          /* Messages from the link take priority over keyboard */
     if (rdrdy())
      if(handle_data())
       continue;
      else
       break;
 
     if ((x = keyboard_rtn()) ne 0)
	  { if (x < 0)
         { shift = _bios_keybrd(_KEYBRD_SHIFTSTATUS);
           x = (x & 0x007F) + 0x0080;
           if (shift & 0x04)
  		    { if (x eq 0x00E7)
               { tidy_files(); outputstr("\r\nRebooting...\r\n"); return(1); }
              if (x eq 0x00E6)
               { tidy_files(); return(0); }
              if (x eq 0x00E5)
               { outputstr("Miniserver alive and well.\r\n"); continue; }
             }

		    if ((x < 0x00BB) || (x > 0x00F1))
             continue;
            x = x - 0x3B;
         }
        (void) byte_to_link(Pro_Poll);
		head.fncode   = Poll_Console;
		head.highsize = 0;
		head.lowsize  = 0;
        head.extra = x;
		(void) send_block(sizeofHead, (BYTE *) &head, LongTimeout);
        continue;
      }
  }

 return(0);
}

int handle_data(void)
{ int x = 0;

  (void) byte_from_link(&x);

  switch(x)
   { case Pro_Synch		: break;
                              
     case Pro_IOServ	: ioserver_protocol(); break;

	 case Pro_Die		: tidy_files(); return(0);

     case Pro_Sleep		: background = TRUE; return(0);

     default			: break;
   }
  return(1);
}

/**
*** The IOServer protocol
**/
FORWARD void open_file(void);
FORWARD void close_file(void);
FORWARD void read_file(void);
FORWARD void write_file(void);
FORWARD void seek_in_file(void);
FORWARD void create_file(void);
FORWARD void delete_file(void);
FORWARD void rename_file(void);
FORWARD void locate_file(void);
FORWARD void read_dir(void);
FORWARD void create_dir(void);
FORWARD void remove_dir(void);
FORWARD void file_info(void);
FORWARD void disk_usage(void);
FORWARD void screen_write(void);
FORWARD void change_date(void);
FORWARD void ioserv_reply(int, int);

PRIVATE void ioserver_protocol()
{ int  size, x;

  if (fetch_block(sizeofHead, (BYTE *) &head, LongTimeout) ne 0)
	{ /* outputstr("\r\nIncomplete packet received.\r\n");*/
	  return;
	}

  size = (256 * head.highsize) + head.lowsize;
  if ((x = fetch_block(size, &(big_buffer[0]), LongTimeout)) ne 0)
	{ /*outputstr("\r\nFailed to receive all of data vector.\r\n");*/
      return;
	}

  switch(head.fncode)
	{ case Fun_OpenFile		: open_file(); break;

	  case Fun_CloseFile	: close_file(); break;

	  case Fun_ReadFile		: read_file(); break;

	  case Fun_WriteFile	: write_file(); break;

	  case Fun_SeekInFile	: seek_in_file(); break;

	  case Fun_CreateFile	: create_file(); break;

	  case Fun_DeleteFile	: delete_file(); break;

	  case Fun_Rename		: rename_file(); break;

	  case Fun_Locate		: locate_file(); break;

	  case Fun_ReadDir		: read_dir(); break;

	  case Fun_CreateDir	: create_dir(); break;

	  case Fun_RemoveDir	: remove_dir(); break;

	  case Fun_FileInfo		: file_info(); break;

	  case Fun_DiskUsage	: disk_usage(); break;

	  case Fun_ScreenWrite	: screen_write(); break;

      case Fun_ChangeDate   : change_date(); break;

	  default				: /*outputstr("\r\nIllegal IOServer fncode received.\r\n");*/
                              break;
    }
  head.fncode = 0xFF;
  head.extra  = 0xFF;
  head.highsize = 0xFF;
  head.lowsize  = 0xFF;
}

#define MaxStreams        17

PRIVATE int  stream_array[MaxStreams];

PRIVATE void init_files(void)
{ int i;

  for (i = 0; i < MaxStreams; i++)
   stream_array[i] = -1;
}

PRIVATE void tidy_files(void)
{ int i;
  for (i = 0; i < MaxStreams; i++)
   if (stream_array[i] ne -1)
    { _dos_close(stream_array[i]); stream_array[i] = -1; }
}

PRIVATE void open_file()
{ int stream, index;
  int itsmode = head.extra, localmode;

  for (index = 0; index < MaxStreams; index++)
   if (stream_array[index] eq -1) break;

  if (index eq MaxStreams)
   { ioserv_reply(Rep_NotFound, 0); return; }

  localmode = (itsmode eq OpenMode_ReadOnly) ? O_RDONLY :
			  (itsmode eq OpenMode_WriteOnly) ? O_WRONLY : O_RDWR;

  if (itsmode eq OpenMode_WriteOnly)
	{ if (_dos_creat(local_name, _A_NORMAL, &stream) ne 0)
	  	{ ioserv_reply(Rep_NotFound, 0); return; }
    }
  elif ((stream = my_dos_open(local_name, localmode)) eq -1)
    { ioserv_reply(Rep_NotFound, 0); return; }

  stream_array[index] = stream;
  big_buffer[0] = (index >> 8) & 0x00FF;
  big_buffer[1] = index & 0x00FF;
  ioserv_reply(Rep_Success, 2);
}

PRIVATE void close_file()
{ int stream;

  stream = (big_buffer[0] << 8) + big_buffer[1];
  (void) _dos_close(stream_array[stream]);
  stream_array[stream] = -1;
  ioserv_reply(Rep_Success, 0);
}

PRIVATE void read_file()
{ int stream, amount, actual;

  stream = stream_array[((big_buffer[0] << 8) + big_buffer[1])];
  amount = (big_buffer[2] << 8) + big_buffer[3];

  if (_dos_read(stream, &(big_buffer[0]), amount, &actual) ne 0)
	ioserv_reply(Rep_NotFound, 0);
  else
	ioserv_reply(Rep_Success, actual);
}

PRIVATE void write_file()
{ int stream, amount, temp;

  amount = (head.highsize << 8) + head.lowsize - 2;
  stream = stream_array[((big_buffer[0] << 8) + big_buffer[1])];

  if (_dos_write(stream,  &(big_buffer[2]), amount, &temp) ne 0)
	ioserv_reply(Rep_NotFound, 0);
  else
    ioserv_reply(Rep_Success ,0);
}

PRIVATE void seek_in_file()
{ int stream, mode;
  WORD newpos;

  stream = stream_array[((big_buffer[4] << 8) + big_buffer[5])];
  newpos = (big_buffer[0] * 0x01000000L) + (big_buffer[1] * 0x00010000L) +
           (big_buffer[2] * 0x00000100L) + big_buffer[3];
  mode = (head.extra eq Seek_start) ? SEEK_SET : SEEK_END;

  newpos = lseek(stream, newpos, mode);
  if (newpos eq -1L)
    ioserv_reply(Rep_NotFound, 0);
  else
   { big_buffer[0] = (newpos >> 24) & 0x00FF;
     big_buffer[1] = (newpos >> 16) & 0x00FF;
     big_buffer[2] = (newpos >> 8)  & 0x00FF;
     big_buffer[3] = (newpos & 0x00FF);
     ioserv_reply(Rep_Success, 4);
   }
}

PRIVATE void create_file()
{ int handle;

  if (_dos_creat(local_name, _A_NORMAL, &handle) ne 0)
   ioserv_reply(Rep_NotFound, 0);
  else
   { (void) _dos_close(handle);
     ioserv_reply(Rep_Success, 0);
   }
}

PRIVATE void delete_file()
{
  if (remove(local_name) ne 0)
	ioserv_reply(Rep_NotFound, 0);
  else
    ioserv_reply(Rep_Success, 0);
}

PRIVATE void rename_file()
{ STRING fromname = local_name, toname;
  for (toname = local_name; *toname ne '\0'; toname++);
  toname++;

  if (rename(fromname, toname) ne 0)
   ioserv_reply(Rep_NotFound, 0);
  else
   ioserv_reply(Rep_Success, 0);
}

PRIVATE void locate_file()
{ struct find_t	temp;

  if (strlen(local_name) < 3)
   { head.extra = Dir_t;
     ioserv_reply(Rep_Success, 0);
     return;
   }

  if (_dos_findfirst(local_name, _A_NORMAL + _A_SUBDIR, &temp) ne 0)
    { ioserv_reply(Rep_NotFound, 0);
      return;
    }

  head.extra = (temp.attrib & _A_SUBDIR) ? Dir_t : File_t;
  ioserv_reply(Rep_Success, 0);
}

PRIVATE void read_dir()
{ BYTE *ptr = &(big_buffer[2]), *tempptr;
  int  amount = 2, result, count;
  struct find_t	temp;

  count = 0;
  head.extra = 0;

  strcat(local_name, "\\*.*");
  if ((result = _dos_findfirst(local_name, _A_NORMAL + _A_SUBDIR, &temp))
      ne 0)
   { ioserv_reply(Rep_NotFound, 0); return; }

  while (result eq 0)
   { count++;
     if ((temp.attrib & _A_SUBDIR) ne 0)
	  *ptr++ = Dir_t;
     else
      *ptr++ = File_t;
     amount++;
     for (tempptr = &(temp.name[0]); *tempptr ne '\0'; tempptr++)
	  if (*tempptr ne ' ')
        { *ptr++ = tolower(*tempptr); amount++; }
     *ptr++ = '\0'; amount++;
     result = _dos_findnext(&temp);
   }

  big_buffer[0] = (count >> 8) & 0x00FF; big_buffer[1] = count & 0x00FF;

  ioserv_reply(Rep_Success, amount); 
}

PRIVATE void create_dir()
{ 
  if (mkdir(local_name) ne 0)
   ioserv_reply(Rep_NotFound, 0);
  else
   ioserv_reply(Rep_Success, 0); 
}

PRIVATE void remove_dir()
{ 
  if (rmdir(local_name) ne 0)
   ioserv_reply(Rep_NotFound, 0);
  else
   ioserv_reply(Rep_Success, 0);
}

PRIVATE void file_info()
{ struct find_t temp;

  if (_dos_findfirst(local_name, _A_NORMAL, &temp) ne 0)
   ioserv_reply(Rep_NotFound, 0);
  else
   { WORD size = temp.size, unix_date;
     unsigned int  MSdate = temp.wr_date, MStime = temp.wr_time;
     big_buffer[0] = (size >> 24) & 0x00FF;
     big_buffer[1] = (size >> 16) & 0x00FF;
     big_buffer[2] = (size >> 8) & 0x00FF;
     big_buffer[3] = size & 0x00FF;

     unix_date = get_unixtime(MSdate, MStime);

     big_buffer[4] = (unix_date >> 24) & 0x00FF;
     big_buffer[5] = (unix_date >> 16) & 0x00FF;
     big_buffer[6] = (unix_date >> 8) & 0x00FF;
     big_buffer[7] = unix_date & 0x00FF;
     ioserv_reply(Rep_Success, 8);
   }  
}

PRIVATE void disk_usage()
{ struct diskfree_t data;
  int  drive  = tolower(local_name[0]) - 'a' + 1;      /* drive identifier, A=1 etc.*/
  WORD size, avail;

  if (_dos_getdiskfree(drive, &data))
   { ioserv_reply(Rep_NotFound, 0); return; }

  size = (word) data.total_clusters *
         (word) data.sectors_per_cluster * (word) data.bytes_per_sector;
  avail = (word) data.avail_clusters *					   
         (word) data.sectors_per_cluster * (word) data.bytes_per_sector;

  size /= 1024; avail /= 1024;
  big_buffer[0] = (size >> 24) & 0x00FF;
  big_buffer[1] = (size >> 16) & 0x00FF;
  big_buffer[2] = (size >> 8) & 0x00FF;
  big_buffer[3] = size & 0x00FF;
  big_buffer[4] = (avail >> 24) & 0x00FF;
  big_buffer[5] = (avail >> 16) & 0x00FF;
  big_buffer[6] = (avail >> 8) & 0x00FF;
  big_buffer[7] = avail & 0x00FF;

  ioserv_reply(Rep_Success, 8);
}


PRIVATE void change_date()
{ int MSdate, MStime;
  int handle;

  { struct dosdate_t tempdate;
    _dos_getdate(&tempdate);
    MSdate = ((tempdate.year - 1980) << 9) +
             (tempdate.month << 5) +
             tempdate.day;
  }
  { struct dostime_t temptime;
    _dos_gettime(&temptime);
    MStime = (temptime.hour << 11) + 
             (temptime.minute << 5) +
             (temptime.second / 2);
  }

  if ((handle = my_dos_open(local_name,  O_RDONLY)) eq -1)
   { ioserv_reply(Rep_NotFound, 0); return; }

  _dos_setftime(handle, MSdate, MStime);
                           /* close the file again now that I am finished */
  _dos_close(handle);

  ioserv_reply(Rep_Success, 0);
}

PRIVATE void screen_write()
{ int i, amount = (256 * head.highsize) + head.lowsize;
  for (i = 0; (i < amount) && (big_buffer[i] ne '\0'); i++)
   outputch(big_buffer[i]);
  ioserv_reply(Rep_Success, 0);
}

PRIVATE void outputch(int ch)
{
#define Screen_Normal 0
#define Screen_Escape 1
#define Screen_Move   2
#define Screen_Y      3
  PRIVATE int screen_mode = Screen_Normal;
  PRIVATE int y_coord;

  if (screen_mode eq Screen_Normal)
   { switch(ch)
      { case 0x0C : vbios_cls(); return;
        case 0x07 : vbios_bell(); return;
        case 0x08 : if (vbios_x > 0)
                     vbios_movecursor(vbios_y, vbios_x - 1);
                    return;
        case 0x0A : 
                    if (vbios_y eq 24)  /* bottom line */
                     vbios_scroll();
                    else
                     vbios_movecursor(vbios_y + 1, vbios_x);
                    return;
        case 0x0D : 
                    vbios_movecursor(vbios_y, 0);
                    return;
        case 0x1B : screen_mode = Screen_Escape; return;

        default   : vbios_outputch(ch);
                    return;
      }
   }
  else
   { switch (screen_mode)
      { case Screen_Escape : if (ch eq 'P')
                              { screen_mode = Screen_Normal;
                                vbios_attr  = 0x70;
                              }
                             elif (ch eq 'U')
                              { screen_mode = Screen_Normal;
                                vbios_attr = 0x07;
                              }
                             elif (ch eq 'Y')
                              screen_mode = Screen_Move;
                             else
                              screen_mode = Screen_Normal;
                             return;

        case Screen_Move   : y_coord = ch - 0x20;
                             screen_mode = Screen_Y;
                             return;
        case Screen_Y      : vbios_movecursor(y_coord, ch - 0x20);
        default            : screen_mode = Screen_Normal; return;
      }
   }
}

PRIVATE void outputstr(char *str)
{ while (*str ne '\0')
   outputch(*str++);
}

#if 0
PRIVATE void outputhex(int x)
{ int y = (x >> 4) & 0x0F;
  if (y <= 9)
   outputch('0' + y);
  else
   outputch('a' - 10 + y);

  y = x & 0x0F;
  if (y <= 9)
   outputch('0' + y);
  else
   outputch('a' - 10 + y);
}

PRIVATE void outputaddr(void far *addr)
{ int seg = FP_SEG(addr);
  int off = FP_OFF(addr);

  outputhex(seg >> 8); outputhex(seg);
  outputhex(off >> 8); outputhex(off);
}
#endif

PRIVATE void ioserv_reply(int code, int size)
{ int i;

  if ((i = byte_to_link(Pro_IOServ)) ne 0)
   { /*outputstr("\r\nFailed to send Pro_IOServ byte.\r\n");*/
     return;
   }

  head.fncode = code;
  head.highsize = (size >> 8) & 0x00FF;
  head.lowsize  = size & 0x00FF;

  if ((i=send_block(sizeofHead, (BYTE *) &head, 100)) ne 0)
	{ /*outputstr("\r\nFailed to send IOServ reply header.\r\n");*/
 	  return;
    }

  if (size > 0)
   if (send_block(size, &(big_buffer[0]), 100))
	{ /*outputstr("\r\nFailed to send data vector.\r\n");*/
	  return;
    }
}
   
/**
*** This code deals with booting up the root transputer. Wakeup() is used
*** if the transputer has issued a Pro_Sleep byte, and the user has used
*** the -w command line option. It does not reboot the transputer, but just
*** sends a Wakeup byte to the transputer. Boot_Transputer() boots up the
*** transputer from scratch.
**/

PRIVATE int send_wakeup(void)
{ outputstr("\rSending wakeup byte...\r\n");

  if (byte_to_link(Pro_Wakeup))
   { outputstr("Failed to send wakeup byte.\r\n");
     return(0);
   }
  else
   { outputstr("Transputer has accepted wakeup byte.\r\n");
     return(1);
   }
}

FORWARD int  timeout(char *), send_boot(void);
FORWARD int  send_image(void), send_config(void);
FORWARD int  handle_byte_F0(void), handle_byte_F1(void);

PRIVATE BYTE	system_size[4];

PRIVATE int boot_transputer()
{ 
  reset();

  { time_t delay = clock() + CLK_TCK;
    while (delay > clock());
  }

  if (!send_boot()) goto failed;

  if (!send_image()) goto failed;

  if (!send_config()) goto failed;

  if (!handle_byte_F1()) goto failed;

  return(TRUE);

failed:

  outputstr("Failed to boot transputer.\r\n");
  return(FALSE);
}

PRIVATE int timeout(str)
string str;
{ outputstr("Link timeout when sending ");
  outputstr(str);
  outputstr(".\r\n");
  return(0);
}

PRIVATE int send_boot()
{ int stream;
  BYTE data[256];
  unsigned int size;

  strcpy(data, Helios_Directory);
  strcat(data, "\\lib\\nboot.i");
  stream = my_dos_open(data, O_RDONLY);

  if (stream eq -1)
   { outputstr("Unable to open nboot.i.\r\n");
	 return(0);
   }

  _dos_read(stream, (void far *) &(data[0]), 256, &size);
  size -= 12;

  _dos_close(stream);

  if (byte_to_link((int) size ))
    return(timeout("bootstrap size"));

  if (send_block((int) size, &(data[12]), LongTimeout))
   return(timeout("bootstrap code"));

  return(1);
}

PRIVATE int send_image()
{ int  stream;  unsigned int  size;
  bool firsttime = TRUE;

  strcpy(big_buffer, Helios_Directory);
  strcat(big_buffer, "\\lib\\nucleus.srv");

  stream = my_dos_open(big_buffer, O_RDONLY);

  if (stream eq -1)
   { outputstr("Unable to open system image nucleus.srv.\r\n");
     return(0);
   }

  if (byte_to_link(4))
   { _dos_close(stream); 
     return(timeout("byte 4 (receive system image)"));
   }

  forever
   { if (_dos_read(stream, (void far *) &(big_buffer[0]), Message_Limit, &size) < 0)
      break;

     if (firsttime)
	  { memcpy(&(system_size[0]), &(big_buffer[0]), 4); firsttime = FALSE; }
     if (send_block(size, &(big_buffer[0]), LongTimeout))
      { _dos_close(stream); 
        return(timeout("system image"));
      }
     if (size < Message_Limit) break;
   }

  _dos_close(stream);
  return(1);
}

PRIVATE int send_config()
{ PRIVATE BYTE config_info[] = {
	0x48, 0x00, 0x00, 0x00,		/* Size = 48 bytes */
	0x00, 0x04, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00,
    0x00, 0x10, 0x00, 0x80,
    0x00, 0x00, 0x00, 0x00,		/* Size of system image, needs patching */
    0x00, 0x00, 0x00, 0x00,		/* Unix time stamp, needs patching		*/
    0x06, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x1C, 0x00, 0x00, 0x00,
    0x1C, 0x00, 0x00, 0x00,
	0x04, 0x00, 0x00, 0x00,
    0x70, 0x01, 0x06, 0x00,
    0x00, 0x00, 0x06, 0x01,
    0x00, 0x00, 0x06, 0x02,
    0x00, 0x00, 0x06, 0x03,
    0x2F, 0x30, 0x30, 0x00,
    0x2F, 0x49, 0x4F, 0x00 };

  WORD now = unixtime();

  memcpy(&(config_info[16]), &(system_size[0]), 4);

  config_info[20] = now & 0x00FF;
  config_info[21] = (now >> 8) & 0x00FF;
  config_info[22] = (now >> 16) & 0x00FF;
  config_info[23] = (now >> 24) & 0x00FF;

  if (send_block(76, &(config_info[0]), LongTimeout))
    return(timeout("configuration"));
  return(1);
}

PRIVATE int handle_byte_F1()
{ int size;
  int temp = 0;
  BYTE buffer[256], *ptr, *src;

  { time_t delay = clock() + CLK_TCK;
    while (delay > clock());
  }

  for (temp = 0; temp < 30000; temp++)
   if (rdrdy()) break;

  if (!rdrdy())
   { outputstr("Failed to receive byte F1 from Server Task.\r\n");
     return(0);
   }

  if (byte_from_link(&temp))
   { outputstr("Failed to receive byte F1 from Server Task.\r\n");
     return(0);
   }

  if (temp ne 0x00F1)
   { outputstr("Received unexpected byte instead of F1.\r\n");
     return(0);
   }

  buffer[0] = PC;
  buffer[1] = 0;
  if (nopop)
   buffer[1] |= Config_flags_Nopop;
  buffer[2] = 1;
  buffer[3] = (Message_Limit >> 24) & 0x00FF;
  buffer[4] = (Message_Limit >> 16) & 0x00FF;
  buffer[5] = (Message_Limit >> 8)  & 0x00FF;
  buffer[6] = Message_Limit & 0x00FF;

  size = 7; ptr = &(buffer[7]);

  for (src = Helios_Directory; ; src++)
   { *ptr++ = *src; size++; if (*src eq '\0') break; }

  temp = 0;
  for (src = drives; *src ne '\0'; src++)
   { *ptr++ = *src; *ptr++ = '\0'; size += 2; temp++; }
  buffer[2] = temp;  /* number of drives */

  if (byte_to_link(0x00F2))
   return(timeout("server reply byte F2"));

  if (byte_to_link(size))
   return(timeout("size of server reply"));

  if (send_block(size, &(buffer[0]), LongTimeout))
   return(timeout("server reply"));          

  outputstr("Booted...\r\n");
  return(1);
}

/**
*** Some utilities.
**/

PRIVATE word unixtime(void)
{ int  date, time;

  { struct dosdate_t tempdate;
    _dos_getdate(&tempdate);
    date = ((tempdate.year - 1980) << 9) +
           (tempdate.month << 5) +
           tempdate.day;
  }
  { struct dostime_t temptime;
    _dos_gettime(&temptime);
    time = (temptime.hour << 11) + 
           (temptime.minute << 5) +
           (temptime.second / 2);
  }

  return(get_unixtime(date, time));
}


                 /* this routine takes an MSdos time stamp and converts it to */
                 /* Unix seconds since 1970 */
PRIVATE word monthlen[12] = { 0L, 31L, 28L, 31L, 30L, 31L, 30L, 31L, 31L, 30L,
							 31L, 30L };

PRIVATE word get_unixtime(int MSdate, int MStime)
{ int  years, months;
  word days, hours, minutes, seconds;
  word count; int i;


  years   = ( MSdate >> 9) & 0x7F;	/* since 1980 */
  months  = ( MSdate & 0x01E0) >> 5;
  days    = ( MSdate & 0x001F);
  hours	  = ( MStime >> 11) & 0x1F;
  minutes = ( MStime & 0x07E0) >> 5;
  seconds = ( MStime & 0x001F) * 2;

  count = (((word)years + 10L) * 365L) + 2L;  /* days since 1970, 2 leapyears */
                                              /* between 1970 and 1980 */
  for (i=0; i < years; i++)
    if (i%4 eq 0)	count++;		/* leap years since 1980 */
									/* year 2000 ??? */
  for (i=1; i < months; i++)
	count = count + monthlen[i];				/* add months */

  if ((years%4 eq 0) && (months > 2))	/* is year a leap year ? */
	count++;

  count += days-1L;			/* this should be days since 1970 */

  count = (24L * count) + hours;
  count = (60L * count) + minutes;
  count = (60L * count) + seconds;
  return(count);
}

