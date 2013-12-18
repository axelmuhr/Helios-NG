/*------------------------------------------------------------------------
--                                                                      --
--         H E L I O S   I N P U T / O U T P U T   S E R V E R          --
--         ---------------------------------------------------          --
--                                                                      --
--              Copyright (C) 1989, Perihelion Software Ltd.            --
--                         All Rights Reserved.                         --
--                                                                      --
--  synergy.c                                                           --
--                                                                      --
--           A device for the synergy image processing subsystem        --
--                                                                      --
--  Author:  BLV 10/1/89                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90  Copyright (C) 1989, Perihelion Software Ltd.       */

#include <helios.h>

#define Synergy_InitStream      Ignore
#define Synergy_TidyStream      Ignore
#define Synergy_PrivateStream   Invalidfn_handler
PRIVATE void Synergy_Read(Conode *);
PRIVATE void Synergy_Write(Conode *);
PRIVATE void Synergy_Close(Conode *);
#define Synergy_GetSize         Invalidfn_handler
#define Synergy_SetSize         Invalidfn_handler
PRIVATE void Synergy_Seek(Conode *);
#define Synergy_GetAttr         Invalidfn_handler
#define Synergy_SetAttr         Invalidfn_handler
#define Synergy_EnableEvents    Invalidfn_handler
#define Synergy_Acknowledge     IgnoreVoid
#define Synergy_NegAcknowledge  IgnoreVoid

PRIVATE VoidFnPtr Synergy_Handlers[Stream_max] =
{ (VoidFnPtr) Synergy_InitStream,  (VoidFnPtr) Synergy_TidyStream,
  Synergy_PrivateStream,
  Synergy_Read,          Synergy_Write,        Synergy_Close,
  Synergy_GetSize,       Synergy_SetSize,      Synergy_Seek,
  Synergy_GetAttr,       Synergy_SetAttr,      Synergy_EnableEvents,
  Synergy_Acknowledge,   Synergy_NegAcknowledge };

/**
*** InitServer sets up the board address as per the host.con file, and
*** restarts the board. It is assumed that the board has been booted already,
*** so restarting merely involves executing the restart command. If the board
*** is in the middle of scanning, tough ! The definitions below specify
*** the board and the assembler routines used to interface with it.
**/
PRIVATE bool board_in_use;
extern  int  synergy_base;
PRIVATE int  synergy_board;
extern  int  syn_inbyte(uint addr);
extern  int  syn_inword(uint addr);
extern  void syn_outbyte(uint addr, uint val);
extern  void syn_outword(uint addr, uint val);
extern  void syn_setaddr(uint addr);
extern  void syn_writeword(uint value);
extern  int  syn_readword(void);
extern  int  syn_writewholeword(WORD);
extern  int  syn_writebuffer(int, int, int, BYTE *);
extern  int syn_readbuffer(int, int, int, BYTE *);

PRIVATE uint Command_Base = 0x0CF0;
PRIVATE uint Status_Base  = 0x0CE0;
PRIVATE uint Buffer_Base  = 0x0D00;
PRIVATE uint no_of_lines  = 512;
PRIVATE uint no_cols      = 768;
PRIVATE WORD ScreenSize;


#define max_args             8   /* For a command */
#define com_Restart          0L  /* The various commands available */
#define com_Vector           1L
#define com_Scalar           2L
#define com_Statistics       3L
#define com_Histogram        4L
#define com_Convolution      5L
#define com_Zoom             6L
#define com_LoadInputLUT     7L
#define com_LoadOutputLUT    8L
#define com_LoaderOverlayLUT 9L
#define com_Character       10L
#define com_String          11L
#define com_Copyblock       12L
#define com_Scroll          13L
#define com_Wipe            14L
#define com_Point           15L
#define com_Line            16L
#define com_StartTVGrab     17L
#define com_StopTVGrab      18L
#define com_TVFilter        19L
#define com_ReinitTV        20L
#define com_SetTVRep        21L
#define com_SetFilterCoef   22L
#define com_SetFilterMode   23L
#define com_SetFilterType   24L
#define com_FilterStart     25L
#define com_FilterStop      26L
#define com_ReadMem         27L
#define com_WriteMem        28L
#define com_TVSelect        29L
#define com_CopyVideo       30L
#define com_PhotoOut        31L
							  
/**
*** Declarations for the routines which actually interact with the board.
**/

PRIVATE WORD read_synergy_memory( int, int, WORD *);
PRIVATE void read_video_memory(Conode *, int, WORD, WORD);
PRIVATE int  execute_command(WORD *);
PRIVATE int  board_ready(void);
PRIVATE void write_synergy_memory( int, int, WORD *);
PRIVATE void write_video_memory(Conode *, int, WORD, WORD);
PRIVATE int  read_scan_line(int, int, BYTE *);
PRIVATE int  write_scan_line(int, int, BYTE *);
PRIVATE int  syn_bootboard(void);

/**
*** This routine is called when the Server starts up or is rebooted.
**/

void Synergy_Testfun(bool *result)
{ WORD board_address = get_int_config("synergy_base");

  if (board_address eq Invalid_config)
   { board_address = get_int_config("synergy2_base");
     if (board_address eq Invalid_config)
      { *result = FALSE;  return; }

     Command_Base = 0x17c0;
     Status_Base  = 0x17f0;
     Buffer_Base  = 0x1800;
     no_of_lines  = 1024;
     no_cols      = 1024;
   }   

  synergy_base  = (int) board_address;
  synergy_board = synergy_base;
  board_in_use  = FALSE;
  ScreenSize = ((WORD) no_of_lines) * ((WORD) no_cols);

  *result = TRUE;
}


/**
*** This routine is called when an open request is received from the
*** transputer.
***
*** Open's are guaranteed to succeed : I allow many open streams to
*** be open, but only one to be active. The only routines which interact with
*** the board itself are the read and write routines. Close is used merely
*** to keep the world tidy.
**/

void Synergy_Open(myco)
Conode *myco;
{ NewStream(Type_File, Flags_Closeable, NULL, Synergy_Handlers);
  use(myco)
}

PRIVATE void Synergy_Close(myco)
Conode *myco;
{ if (mcb->MsgHdr.Reply ne 0L)
   Request_Return(ReplyOK, 0L, 0L);
  Seppuku();
  use(myco)
}

/**
*** Seek's are fairly redundant, but I must support them anyway. Unfortunately
*** this means at least one redundant message transaction every time I try
*** to do something with the board.
**/
PRIVATE void Synergy_Seek(myco)
Conode *myco;
{ word mode         = mcb->Control[SeekMode_off];    /* beginning, end etc. */
  word newpos       = mcb->Control[SeekNewPos_off];  /* offset */
  word pos          = mcb->Control[SeekPos_off];     /* old position */

  if ((mode eq S_Relative) || (mode eq S_Beginning))
    { if (mode eq S_Relative) newpos += pos;
      mcb->Control[Reply1_off] = newpos;
      Request_Return(ReplyOK, 1L, 0L);
      return;
    }

  Request_Return(EC_Error + SS_IOProc + EG_Parameter + EO_Message, 0L, 0L);
                  /* invalid mode */
}

/**
*** Offsets within the synergy device for the various different parts of memory.
**/
#define Command_pos     0x00000000L
#define Status_pos      0x10000000L
#define Buffer_pos      0x20000000L
#define Word_pos        0x30000000L
#define TopByte_pos     0x40000000L
#define BottomByte_pos  0x50000000L

#define Control_off     0
#define Status_off      0
#define LSB_off         1
#define CLI_off         2
#define STI_off         3
#define Addr_off        4
#define MSB_off         6

#define No_Increment    0x8000

#define Control_Reset      0x0001
#define Control_Halt       0x0002
#define Control_Interrupt  0x0080
#define Status_Trapped     0x0080

#define lower_byte        0 /* which bit of the picture */
#define upper_byte        1
#define entire_word       2
#define first_line        0
#define first_row         0

PRIVATE void Synergy_Read(myco)
Conode *myco;
{ WORD pos     = mcb->Control[ReadPos_off];
  WORD size    = mcb->Control[ReadSize_off];
  int  synergy_addr;
  WORD temp;

  if (board_in_use)
   { Request_Return(EC_Warn + SS_IOProc + EG_InUse + EO_Stream, 0L, 0L);
     return;
   }

  if ((pos eq Command_pos) || (pos eq Status_pos) || (pos eq Buffer_pos))
   { if   (pos eq Command_pos) synergy_addr = Command_Base;
     elif (pos eq Status_pos)  synergy_addr = Status_Base;
     else synergy_addr = Buffer_Base;

     temp = read_synergy_memory(synergy_addr, (int) size, (WORD *) mcb->Data);
     Request_Return(ReadRc_EOD, 0L, temp);
     return;
   }          

  if ((pos >= Word_pos) && (pos <= (Word_pos + ScreenSize + ScreenSize)))
   read_video_memory(myco, entire_word, pos - Word_pos, size);
  elif ((pos >= TopByte_pos) && (pos <= (TopByte_pos + ScreenSize)))
   read_video_memory(myco, upper_byte, pos - TopByte_pos, size);
  elif ((pos >= BottomByte_pos) && (pos <= (BottomByte_pos + ScreenSize)))
   read_video_memory(myco, lower_byte, pos - BottomByte_pos, size);
  else
   Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_Stream, 0L, 0L);
}

PRIVATE void Synergy_Write(myco)
Conode *myco;
{ WORD pos     = mcb->Control[WritePos_off];
  WORD size    = mcb->Control[WriteSize_off];
  int  synergy_addr;
  WORD temp;

  if (board_in_use)
   { Request_Return(EC_Warn + SS_IOProc + EG_InUse + EO_Stream, 0L, 0L);
     return;
   }

  if (pos eq Command_pos) 
   { if (mcb->MsgHdr.DataSize eq 0)
      { Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_Message, 0L, 0L);
        return;
      }
     if (!execute_command((WORD *) mcb->Data))
      { Request_Return(EC_Error + SS_IOProc + EG_Broken + EO_Stream, 0L, 0L);
        return;
      }
     mcb->Control[Reply1_off] = size;
     Request_Return(WriteRc_Done, 1L, 0L);
     return;
   }
 
  if ((pos eq Status_pos) || (pos eq Buffer_pos))
   { if (pos eq Status_pos)
      synergy_addr = Status_Base;
     else
      synergy_addr = Buffer_Base;

     if (mcb->MsgHdr.DataSize ne 0)
      { write_synergy_memory(synergy_addr, (int) size, (WORD *) &(mcb->Data[0]));
        mcb->Control[Reply1_off] = size;
        Request_Return(WriteRc_Done, 1L, 0L);
        return;
      }

     if ((synergy_addr ne Buffer_Base) || (size >= (no_cols * sizeof(WORD))))
      { Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_Stream, 0L, 0L);
        return;
      }

     temp = mcb->MsgHdr.Reply;

                    /* send the first reply indicating how to send the data */
     (mcb->Control)[Reply1_off] = size; 
     (mcb->Control)[Reply2_off] = 0L;
     mcb->MsgHdr.Flags = MsgHdr_Flags_preserve; /* must preserve the route here */
     Request_Return(WriteRc_Sizes, 2L, 0L);  

     myco->timelimit  = Now + (60L * OneSec);  /* reset the time limit */
     Suspend();                                /* wait for the next lot of data */

     if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
      Seppuku();

	 write_synergy_memory(synergy_addr, (int) size, (WORD *) &(mcb->Data[0]));
     mcb->Control[Reply1_off] = size;
     mcb->MsgHdr.Reply = temp;
     Request_Return(WriteRc_Done, 1L, 0L);
     return;
    }

  if ((pos >= Word_pos) && (pos <= (Word_pos + ScreenSize + ScreenSize)))
   write_video_memory(myco, entire_word, pos - Word_pos, size);
  elif ((pos >= TopByte_pos) && (pos <= (TopByte_pos + ScreenSize)))
   write_video_memory(myco, upper_byte, pos - TopByte_pos, size);
  elif ((pos >= BottomByte_pos) && (pos <= (BottomByte_pos + ScreenSize)))
   write_video_memory(myco, lower_byte, pos - BottomByte_pos, size);
  else
   Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_Stream, 0L, 0L);
}

/**
*** The following routines are the local routines, which actually do the
*** work of interacting with the board. read_synergy_memory() and
*** write_synergy_memory() are used to access the command buffer, status
*** buffer, and video buffer, and execute_command() is used to run a command
*** in the 2100.
**/

PRIVATE int scanning = 0;

PRIVATE int board_ready()
{ int temp = syn_inbyte(synergy_board + Status_off);

  if (temp & Status_Trapped)
   { output("*** Serious : synergy board has executed Trap.\r\n");
     return(-1);
   }

  if (no_cols eq 1024)
   { if (temp & 0x40)
      return(1);
     else
      return(0);
   }
  else
  { if (temp & 0x01)
     return(0);
    else
     return(1);
  }
}

PRIVATE int execute_command(WORD *command)
{ int x;
  long counter;

  if (*command eq 99L)
   { x = syn_bootboard(); scanning = 0; return(x); }

  if (!scanning) 
   { for (counter = 0L, x = 0; (x eq 0) && (counter < 100000L); counter++)
      x = board_ready();
     if (x ne 1) { ServerDebug("board not ready"); return(0); }
   }

  if (scanning)
   if ((*command ne com_StopTVGrab) && (*command ne com_FilterStop))
     return(0);
 
  syn_outbyte(synergy_board + CLI_off, 0);

  write_synergy_memory(Command_Base, (max_args + 1) * sizeof(WORD), command);

  syn_setaddr(Command_Base | No_Increment);

  syn_outbyte(synergy_board + STI_off, 0);

  if ((*command eq com_StartTVGrab) ||
      (*command eq com_FilterStart))
    return(1);

  for (counter = 0L, x = 0; (x eq 0) && (counter < 100000L); counter++)
   x = board_ready();
  if (x ne 1) { ServerDebug("Command failed to complete"); return(0); }

  x = syn_readword();
  if (x ne 0)
   ServerDebug("Synergy board : execute error %d", x);
  return (!x);
}

PRIVATE WORD read_synergy_memory(int addr, int amount, WORD *buff)
{ int i;

  syn_setaddr(addr);
  for (i = 0; i < (amount >> 2); i++)
   *buff++ = (WORD) syn_readword();

  return((WORD) amount);
}


PRIVATE void write_synergy_memory(int addr, int amount, WORD *buff)
{ int i, temp;

  syn_setaddr(addr);
  for (i = 0; i < (amount >> 2); i++)
   { temp = (int) *buff; buff++;
     syn_writeword(temp);
   }
}

PRIVATE void read_video_memory(Conode *myco, int which_bytes, WORD base, WORD amount)
{ int first_scan_line, no_scan_lines, i;
  BYTE *buff;
  WORD reply_port = mcb->MsgHdr.Reply;
  WORD seq = 0L;
  WORD count = 0L;

  board_in_use = TRUE;

  if (which_bytes eq entire_word)
   { base >>= 1; amount >>= 1; }

  first_scan_line = (int) (base / no_cols);
  no_scan_lines   = (int) (amount / no_cols);
  count           = 0L;
  buff            = mcb->Data;

  AddTail(Remove(&(myco->node)), PollingCo);

  for ( i = 0; i < no_scan_lines; i++)
   { if (!read_scan_line(first_scan_line + i, which_bytes, buff))
      { mcb->MsgHdr.Reply = reply_port;
        Request_Return(EC_Error + SS_IOProc + EG_Broken + EO_Stream, 0L, 0L);
        board_in_use = FALSE;
        Seppuku();
      }
     buff += no_cols; count += (WORD) no_cols;
     if (which_bytes eq entire_word)
      { buff += no_cols; count += (WORD) no_cols; }

     if ( (i & 0x000f) eq 0x000f)       /* read 16 lines ? */
      { mcb->MsgHdr.Reply = reply_port; /* Send this data */
        mcb->MsgHdr.Flags = MsgHdr_Flags_preserve;
        Request_Return(seq, 0L, count);
        seq += 16L; count = 0L; buff = mcb->Data;   /* reset buffer */

        myco->timelimit = MAXINT;       /* Wait a bit */
        myco->type = CoReady;
        Suspend();
        if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
         { board_in_use = FALSE; Seppuku(); }
      }
   } 

  mcb->MsgHdr.Reply = reply_port;   /* Send final message */
  Request_Return(seq + ReadRc_EOD, 0L, count);
  PostInsert(Remove(&(myco->node)), Heliosnode);
  board_in_use = FALSE;
}


PRIVATE void write_video_memory(Conode *myco, int which_bytes, WORD base, WORD amount)
{ int first_scan_line, no_scan_lines, i;
  BYTE *buff;
  WORD reply_port = mcb->MsgHdr.Reply;
  WORD seq = 0L;
  WORD count = 0L;

  board_in_use = TRUE;

  if (which_bytes eq entire_word)
   { base >>= 1; amount >>= 1; }

  first_scan_line = (int) (base / no_cols);
  no_scan_lines   = (int) (amount / no_cols);
  count           = 0L;
  if (no_scan_lines eq 0)
   { board_in_use = FALSE;
     Request_Return(EC_Error + SS_IOProc + EG_WrongSize + EO_Message, 0L, 0L);
     return;
   }

  mcb->Control[Reply1_off] = (WORD) (16 * no_cols);
  if (which_bytes eq entire_word) mcb->Control[Reply1_off] <<= 1;
  mcb->Control[Reply2_off] = mcb->Control[Reply1_off];
  mcb->MsgHdr.Flags = MsgHdr_Flags_preserve; /* must preserve the route here */
  Request_Return(WriteRc_Sizes, 2L, 0L);  

  for (i = 0; i < no_scan_lines; i++)
   { if (count <= 0L)
      { myco->timelimit = MAXINT;
        Suspend();
        if ((myco->type eq CoSuicide) || (myco->type eq CoTimeout))
         { board_in_use = FALSE; Seppuku(); }
        count = (WORD) mcb->MsgHdr.DataSize;
        buff = mcb->Data;
      }
     if (!write_scan_line(first_scan_line + i, which_bytes, buff))
      { mcb->MsgHdr.Reply = reply_port;
        Request_Return(EC_Error + SS_IOProc + EG_Broken + EO_Stream, 0L, 0L);
        board_in_use = FALSE;
        Seppuku();
      }
     buff += no_cols; count -= (WORD) no_cols;
     if (which_bytes eq entire_word)
      { buff += no_cols; count -= (WORD) no_cols; }
   }

  mcb->MsgHdr.Reply = reply_port;
  mcb->Control[Reply1_off] = (which_bytes eq entire_word) ? (amount << 1) : amount;
  Request_Return(WriteRc_Done, 1L, 0L);

  board_in_use = FALSE;
}

static WORD com_buf[9];

PRIVATE int read_scan_line(int line, int which_bytes, BYTE *buff)
{ int i;

  com_buf[0] = com_Vector;
  com_buf[1] = 0L;
  com_buf[2] = (WORD) line;
  com_buf[3] = (WORD) no_cols;
  com_buf[4] = (WORD) which_bytes;
  com_buf[5] = 6L;   /* Put into buffer */

  if (!execute_command(&(com_buf[0]))) return(0);

  syn_readbuffer(Buffer_Base, no_cols, which_bytes, buff);

/*
  syn_setaddr(Buffer_Base);
  if (which_bytes eq entire_word)
   { int *int_buf = (int *) buff;
     for (i = 0; i < no_cols; i++)
      *int_buf++ = syn_readword();
   }
  else
   { for (i = 0; i < no_cols; i++)
      *buff++ = syn_readword();
   }
*/

  return(1);
}

PRIVATE int write_scan_line(int line, int which_bytes, BYTE *buff)
{ int i, temp;

/*
  syn_setaddr(Buffer_Base);
  if (which_bytes eq entire_word)
   { int *int_buf = (int *) buff;
     for (i = 0; i < no_cols; i++)
      syn_writeword(*int_buf++);
   }
  else
   { for (i = 0; i < no_cols; i++)
      syn_writeword(*buff++);
   }
*/

  syn_writebuffer(Buffer_Base, no_cols, which_bytes, buff);

  com_buf[0] = com_Vector;
  com_buf[1] = 0L;
  com_buf[2] = (WORD) line;
  com_buf[3] = (WORD) no_cols;
  com_buf[4] = (WORD) which_bytes;
  com_buf[5] = 7L;    /* Get from buffer */
  return(execute_command(&(com_buf[0])));
}

PRIVATE WORD get_hex(char *buf)
{ WORD result = 0L;

  for ( ; isxdigit(*buf); buf++)
   { if ((*buf >= '0') && (*buf <= '9'))
      result = (result << 4) + (*buf -'0');
     elif((*buf >= 'a') && (*buf <= 'f'))
      result = (result << 4) + (*buf + 0x0a - 'a');
     else
      result = (result << 4) + (*buf + 0x0A - 'A');
   }
  return(result);
}

PRIVATE int syn_bootboard()
{ FILE *syn_file;
  char buff[80], *file_name;
  int  addr = -666;
  long value;

  file_name = get_config("Synergy_file");
  if (file_name eq NULL)
    file_name = (no_cols eq 1024 ? "syntil8.cmd" : "synergy.cod");

  syn_file = fopen(file_name, "r");

  syn_outbyte(synergy_board + Control_off, 0);

  if (syn_file eq (FILE *) NULL)
   { ServerDebug("Failed to open file %s", file_name);
     return(0);
   }

  for ( ; ;)
   { if (fgets(buff, 80, syn_file) eq (char *) NULL) break;
     if (buff[0] eq 0x1B) 
	     { if ((buff[1] ne 0x1B) || ((buff[2] ne 'i') && (buff[2] ne 'o')))
         { ServerDebug("Error in synergy file, escape character expected.");
           fclose(syn_file);
           return(0);
         }
        if (buff[2] eq 'i') continue;  /* start of file */
        if (buff[2] eq 'o') break;     /* end of file */
      }
     if (buff[0] eq '@')               /* new address coming up */
      { addr = -555; continue; }
     if (buff[0] eq '#')               /* end of current batch of data */
      { addr = -666; continue; }

     if (addr eq -555)
      { if ((value = get_hex(buff)) eq -1L)
         { ServerDebug("Error in synergy file, address expected.");
           fclose(syn_file);
           return(0);
         }
        addr = (int) value;
        syn_setaddr(addr & 0x7fff);  /* Clear top bit for auto-increment */
        continue;
      }
     if (addr eq -666)
      { ServerDebug("Error in synergy file, address not set up.");
        fclose(syn_file);
        return(0);
      }
     if ((value = get_hex(buff)) eq -1L)
      { ServerDebug("Error in synergy file, hex data expected.");
        ServerDebug("Found %s instead.", buff);
        fclose(syn_file);
        return(0);
      }
     syn_writewholeword(value);
   }

  syn_outbyte(synergy_board + CLI_off, 0);
  syn_outbyte(synergy_board + Control_off, 0x01);
  syn_outbyte(synergy_board + Control_off, 0x03);

  fclose(syn_file);

  return(1);
}

