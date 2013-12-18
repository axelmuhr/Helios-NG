/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   S Y N E R G Y  I N T E R F A C E		        --
--          ----------------------------------------------              --
--                                                                      --
--             Copyright (C) 1989, Perihelion Software Ltd.             --
 --                        All Rights Reserved.                          --
--                                                                      --
--      synboot.c                                                       --
--                                                                      --
--         A program to bootstrap the synergy board                     --
--                                                                      --
--     Author:  BLV 9/1/89                                              --
--                                                                      --
------------------------------------------------------------------------*/
/* SccsId: 3.8 28/3/90 Copyright (C) 1989, Perihelion Software Ltd.         */

#include <stdio.h>
#include <time.h>
#define eq ==
#define ne !=
#define TRUE  1
#define FALSE 0

extern int  synergy_base;
extern int  syn_inbyte(int addr);
extern int	syn_inword(int addr);
extern void syn_outbyte(int addr, int val);
extern void syn_outword(int addr, int val);
extern void syn_setaddr(int addr);
extern void syn_writeword(int value);
extern int  syn_readword(void);
extern void syn_writewholeword(long value);

#define synergy_board   0x02e0
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
#define Status_Idle        0x0001
#define Status_Trapped     0x0080

#define Command_Base       0x0CF0
#define Status_Base        0x0CE0
#define Buffer_Base        0x0D00

#define com_Restart            0
#define com_Vector             1
#define com_Scalar			   2
#define com_Statistics		   3
#define com_Histogram		   4
#define com_Convolution		   5
#define com_Zoom			   6
#define com_LoadInputLUT	   7
#define com_LoadOutputLUT	   8
#define com_LoadOverlayLUT     9
#define com_Character		  10
#define com_String			  11
#define com_Copyblock		  12
#define com_Scroll			  13
#define com_Wipe			  14
#define com_Point			  15
#define com_Line			  16
#define com_StartTVGrab		  17
#define com_StopTVGrab		  18
#define com_TVFilter		  19
#define com_ReinitTV		  20
#define com_SetTVRep		  21
#define com_SetFilterCoef	  22
#define com_SetFilterMode	  23
#define com_SetFilterType	  24
#define com_FilterStart		  25
#define com_FilterStop		  26
#define com_ReadMem			  27
#define com_WriteMem		  28
#define com_TVSelect		  29
#define com_CopyVideo		  30
#define com_PhotoOut		  31

#define op_plus                0
#define op_minus			   1
#define op_and				   2
#define op_or				   3
#define op_times			   4
#define op_device			   5
#define op_put				   6
#define op_get				   7
#define op_swap				   8
#define op_remap			   9
#define op_copy				  10

static int  boot_synergy(void);
static int  board_ready(void);
static int  execute(int, int, int, int, int, int, int, int);
static void write_buffer(int);
static int  buffer[256];

int main()
{ int i;
  synergy_base = synergy_board;

  if (!boot_synergy())
   return(1);

  printf("Synergy card booted...");
  fflush(stdout);

  if (!execute(com_Restart, 0, 0, 0, 0, 0, 0, 0))
   { fprintf(stderr, "Failed to restart board.\n");
     exit(1);
   }

/*
  for (i = 0; i < 256; i++) buffer[i] = (i + (i << 8));
  write_buffer(256);
  if (!execute(com_LoadOutputLUT, 8, 0, 0, 0, 0, 0, 0))
   { fprintf(stderr, "Failed to set output lookup table.\n");
     exit(1);
   }
*/

/*
  for (i = 0; i < 10; i++) buffer[i] = -1;
  write_buffer(9);
  if (!execute(com_LoadOverlayLUT, 0, 0, 0, 0, 0, 0, 0))
   { fprintf(stderr, "Failed to set overlay lookup table.\n");
     exit(1);
   }

  if (!execute(com_Scalar, 0, 0, 768, 512, 2, op_put, 0))
   { fprintf(stderr, "Failed to wipe display.\n");
     exit(1);
   }
*/

  if (!execute(com_Wipe, 0, 0, 2, 768, 512, -1, 0))
   { fprintf(stderr, "Failed to wipe overlays.\n");
     exit(1);
   }

  printf(" and initialised.\n");
  return(0);
}

static int boot_synergy()
{ FILE *syn_file = fopen("synergy.cod", "r");
  char buffer[80];
  int  addr = -666;
  long value;

  syn_outbyte(synergy_board + Control_off, 0);

  if (syn_file eq (FILE *) NULL)
   { fprintf(stderr, "Failed to open file synergy.cod.\n");
     return(0);
   }

  for ( ; ;)
   { if (fgets(buffer, 80, syn_file) eq (char *) NULL) break;
     if (buffer[0] eq 0x1B) 
	  { if ((buffer[1] ne 0x1B) || ((buffer[2] ne 'i') && (buffer[2] ne 'o')))
         { fprintf(stderr, "Error in synergy file, escape character expected.\n");
           fclose(syn_file);
           return(0);
         }
        if (buffer[2] eq 'i') continue;  /* start of file */
        if (buffer[2] eq 'o') break;     /* end of file */
      }
     if (buffer[0] eq '@')               /* new address coming up */
      { addr = -555; continue; }
     if (buffer[0] eq '#')               /* end of current batch of data */
      { addr = -666; continue; }

     if (addr eq -555)
      { if (sscanf(buffer, "%x", &addr) ne 1)
         { fprintf(stderr, "Error in synergy file, address expected instead of %s\n", buffer);
           fclose(syn_file);
           return(0);
         }
        syn_setaddr(addr & 0x7fff);  /* Clear top bit for auto-increment */
        continue;
      }
     if (addr eq -666)
      { fprintf(stderr, "Error in synergy file, address not set up.\n");
        fclose(syn_file);
        return(0);
      }
     if (sscanf(buffer, "%lx", &value) ne 1)
      { fprintf(stderr, "Error in synergy value, hex data expected instead of %s\n", buffer);
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

static int board_ready()
{ int temp = syn_inbyte(synergy_board + Status_off);

  if (temp & Status_Trapped)
   { fprintf(stderr, "Serious : 2100 has executed Trap.\n");
     exit(1);
   }
  return(!(temp & Status_Idle));
}

static int execute(int com, int b, int c, int d, int e, int f, int g, int h)
{ int result;

  while (!board_ready());

  syn_outbyte(synergy_board + CLI_off, 0);

  syn_setaddr(Command_Base);
  syn_writeword(com);
  syn_writeword(b);
  syn_writeword(c);
  syn_writeword(d);
  syn_writeword(e);
  syn_writeword(f);
  syn_writeword(g);
  syn_writeword(h);

  syn_setaddr(Command_Base | No_Increment);

  syn_outbyte(synergy_board + STI_off, 0);

  while (!board_ready());

  result = syn_readword();
  if (result eq 0)
   return(TRUE);
  else
   { printf("\nCommand failed, error code %d\n", result);
     return(FALSE);
   }
} 

static void write_buffer(int amount)
{ int i;

  while (!board_ready());

  syn_setaddr(Buffer_Base);
  for (i = 0; i < amount; i++)
   syn_writeword(buffer[i]);
}

  
