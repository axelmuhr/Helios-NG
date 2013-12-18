/*=========================================================================
===		Synergy Interface Library				===
===									===
===	Copyright (C) 1989, Perihelion Software Ltd.			===
===	All Rights Reserved						===
===									===
===	Author : BLV, 13.1.89						===
=========================================================================*/

#include <stdio.h>
#include <stdarg.h>
#include <helios.h>
#include <syslib.h>
#include <posix.h>
#include "syndef.h"
#define eq      ==
#define ne      !=
#define FAIL    return(0)
#define SUCCEED return(1)

/**
*** Offsets within the synergy device for the various different parts of memory.
**/
#define Command_pos	0x00000000
#define	Status_pos	0x10000000
#define Buffer_pos	0x20000000
#define Word_pos	0x30000000
#define TopByte_pos	0x40000000
#define BottomByte_pos	0x50000000

#define max_args		 8	/* For a command	*/

#define no_of_lines		512
#define first_row		  0
#define no_cols			768
#define buffer_size		768		/* Size of data buffer  */

PRIVATE int syn_stream = -1;			/* Stream to /synergy device */

/**
*** Routine bindings. All but Init and Tidy map onto syn_Execute, possibly
*** with some additional reading or writing of buffers.
**/

int syn_Init(void)
{ if ((syn_stream = open("/synergy", O_ReadWrite)) eq -1)
   { fprintf(stderr, "Unable to open stream to synergy device.\n");
     FAIL;
   }
  SUCCEED;
}

int syn_Tidy(void)
{ if (syn_stream eq -1) FAIL;
  close(syn_stream);
  SUCCEED;
}

int syn_Restart(void)
{ return(syn_Execute(com_Restart));
}

int syn_Vector(int x_off, int y_off, int width, int operand, int opcode)
{ return(syn_Execute(com_Vector, x_off, y_off, width, operand, opcode));
}

int syn_Scalar(int x_off, int y_off, int width, int height, int operand,
			int opcode, int value)
{ return(syn_Execute(com_Scalar, x_off ,y_off, width, height, operand,
			opcode, value));
}
			
int syn_Statistics(int x_off, int y_off, int width, int height,
			int operand, int *buffer)
{ if (!syn_Execute(com_Statistics, x_off, y_off, width, height, operand))
   FAIL;
   
  return(syn_ReadBuffer(10, buffer));
}
			
int syn_Histogram(int x_off, int y_off, int width, int height,
			int operand, int *buffer)
{ if (!syn_Execute(com_Histogram, x_off, y_off, width, height, operand))
   FAIL;
   
  return(syn_ReadBuffer(256, buffer));
}
			
int syn_Convolution(int x_off, int y_off, int width, int height,
			int patch_size, int type, int *buffer)
{ if (!syn_WriteBuffer(patch_size * patch_size, buffer))
   FAIL;

  return(syn_Execute(com_Convolution, x_off, y_off, width, height, patch_size,
  			type));
}
			
int syn_Zoom(int x_off1, int y_off1, int width1, int height1, int x_off2,
			int y_off2, int width2, int height2)
{ return(syn_Execute(com_Zoom, x_off1, y_off1, width1, height1, x_off2,
			y_off2, width2, height2));
}
			
int syn_LoadInputLUT(int lut, int *buffer)
{ if (!syn_WriteBuffer(256, buffer))
   FAIL;

  return(syn_Execute(com_LoadInputLUT, lut));
}

int syn_LoadOutputLUT(int luts, int *buffer)
{ int size = 0;

  if ((luts & 0x08) ne 0)
   size = 256;
  else
   { if ((luts & 0x01) ne 0) size += 256;
     if ((luts & 0x02) ne 0) size += 256;
     if ((luts & 0x04) ne 0) size += 256;
   }

  if (!syn_WriteBuffer(size, buffer))
   FAIL;
   
  return(syn_Execute(com_LoadOutputLUT, luts));
}

int syn_LoadOverlayLUT(int *buffer)
{ if (!syn_WriteBuffer(9, buffer))
   FAIL;
   
  return(syn_Execute(com_LoadOverlayLUT));
}

int syn_RedefineCharacter(int ch, int *buffer)
{ if (!syn_WriteBuffer(8, buffer))
   FAIL;
   
  return(syn_Execute(com_RedefineCharacter, ch));
}

int syn_String(int x_off, int y_off, int plane, char *text)
{ int buffer[768];
  int i;
  
  for (i = 0; *text ne '\0'; i++) buffer[i] = *text++;
  if (!syn_WriteBuffer(i, buffer))
   FAIL;
   
  return(syn_Execute(com_String, x_off, y_off, plane, i));
}

int syn_CopyBlock(int x_off, int y_off, int plane, int width, 
			int height, int *buffer)
{ int no_words_per_row = ((width * 2) + 31) >> 5;
  if (!syn_WriteBuffer(height * no_words_per_row, buffer))
   FAIL;
   
  return(syn_Execute(com_CopyBlock, x_off, y_off, plane, width, height));
}
			
			
int syn_Scroll(int x_off, int y_off, int plane, int width, int height,
			int y_lines, int value)
{ return(syn_Execute(com_Scroll, x_off, y_off, plane, width, height, y_lines,
			value));
}
			
int syn_Wipe(int x_off, int y_off, int plane, int width, int height,
			int value)
{ return(syn_Execute(com_Wipe, x_off, y_off, plane, width, height, value));
}
			
int syn_Point(int x_off, int y_off, int plane, int value)
{ return(syn_Execute(com_Point, x_off, y_off, plane, value));
}

int syn_Line(int x_off, int y_off, int plane, int x1_off, int y1_off,
			int value)
{ return(syn_Execute(com_Line, x_off, y_off, plane, x1_off, y1_off, value));
}
			
int syn_StartTVGrab(void)
{ return(syn_Execute(com_StartTVGrab));
}

int syn_StopTVGrab(void)
{ return(syn_Execute(com_StopTVGrab));
}

int syn_TVFilter(int x_off, int y_off, int width, int height,
			int frame_delay, int A_coef, int B_coef)
{ return(syn_Execute(com_TVFilter, x_off, y_off, width, height, frame_delay,
			A_coef, B_coef));
}
			
int syn_ReinitTV(void)
{ return(syn_Execute(com_ReinitTV));
}

int syn_SetTVReplication(int display_base, int y_display)
{ return(syn_Execute(com_SetTVReplication, display_base, y_display));
}

int syn_SetSlowScanFilterCoef(int a_coef, int b_coef)
{ return(syn_Execute(com_SetSlowScanFilterCoef, a_coef, b_coef));
}

int syn_SetFilterMode(int movie, int sequence, int lut, int channel)
{ return(syn_Execute(com_SetFilterMode, movie, sequence, lut, channel));
}

int syn_SetFilterType(int ignored_pixel, int ignored_line, int type,
			int mode)
{ return(syn_Execute(com_SetFilterType, ignored_pixel, ignored_line, type, mode));
}
			
int syn_FilterStart(int x_off, int y_off, int width, int height,
			int type, int host)
{ return(syn_Execute(com_FilterStart, x_off, y_off, width, height, type, host));
}
			
int syn_FilterStop(void)
{ return(syn_Execute(com_FilterStop));
}

int syn_TVFormatSelect(int standard)
{ return(syn_Execute(com_TVFormatSelect, standard));
}

int syn_CopyVideoMemory(int x_off, int y_off, int width, int height,
			int x1_off, int y1_off, int src_type, int dest_type)
{ return(syn_Execute(com_CopyVideoMemory, x_off, y_off, width, height,
			x1_off, y1_off, src_type, dest_type));
}
			
int syn_PhotoOutput(int x_off, int y_off, int width, int height,
			 int host)
{ return(syn_Execute(com_PhotoOutput, x_off, y_off, width, height, host));
}

int syn_BootBoard(void)
{ return(syn_Execute(99));
}
			 
/**
*** Here are the routines which interact with the server.
***
*** Execute the Synergy command specified with the arguments given. This
*** involves a seek to the command buffer part of the device followed by
*** writing the command and its argument to that position.
**/

int syn_Execute(int code, ...)
{ va_list	temp;
  WORD		command_buffer[max_args + 1];
  int		i;
  
  va_start(temp, code);
  command_buffer[0] = code;  
  for (i = 1; i <= max_args; i++)
    command_buffer[i] = va_arg(temp, WORD);
  va_end(temp);

  if (lseek(syn_stream, Command_pos, SEEK_SET) eq -1) FAIL;
     
  if (write(syn_stream, (char *) command_buffer,
            ((max_args + 1) * sizeof(WORD))) eq -1)
     FAIL;

  SUCCEED;
}

/**
*** This routine allows you to read a number of words from the Synergy
*** status buffer, which may contain interesting values during scanning.
**/

int syn_ReadStatus(int amount, int *buffer)
{ 
  if (lseek(syn_stream, Status_pos, SEEK_SET) eq -1)
   FAIL;

  if (read(syn_stream, (char *) buffer, amount * sizeof(WORD)) ne (amount * sizeof(WORD)))
   FAIL;
 
  SUCCEED;
}

/**
**/

int syn_WriteBuffer(int amount, int *buffer)
{ if (lseek(syn_stream, Buffer_pos, SEEK_SET) eq -1)
   FAIL;
 
  if (write(syn_stream, (char *) buffer, amount * sizeof(WORD)) ne (amount * sizeof(WORD)))
   FAIL;

  SUCCEED;
}

/**
*** This routine provides the inverse operation to the above. It is useful
*** after e.g. executing a gather-statistics command which stores its
*** results in the buffer.
**/

int syn_ReadBuffer(int amount, int *buffer)
{ if (lseek(syn_stream, Buffer_pos, SEEK_SET) eq -1)
   FAIL;
 
  if (read(syn_stream, (char *) buffer, amount * sizeof(WORD)) ne (amount * sizeof(WORD)))
   FAIL;

  SUCCEED;
}

int syn_WriteVideo(int first_line, int no_lines, int operand, char *data)
{ WORD pos, amount;

  if (operand eq op_EntireWord)
   { pos = Word_pos + (first_line * 2 * no_cols);
     amount = no_lines * 2 * no_cols;
   }
  elif (operand eq op_LowerByte)
   { pos = BottomByte_pos + (first_line * no_cols);
     amount = no_lines * no_cols;
   } 
  elif (operand eq op_UpperByte)
   { pos = TopByte_pos + (first_line * no_cols);
     amount = no_lines * no_cols;
   }
  else
   FAIL;

  if (lseek(syn_stream, pos, SEEK_SET) eq -1)
   FAIL;
   
  if (write(syn_stream, data, amount) ne amount)
   FAIL;
  else
   SUCCEED;
}

int syn_ReadVideo(int first_line, int no_lines, int operand, char *data)
{ WORD pos, amount;

  if (operand eq op_EntireWord)
   { pos = Word_pos + (first_line * 2 * no_cols);
     amount = no_lines * 2 * no_cols;
   }
  elif (operand eq op_LowerByte)
   { pos = BottomByte_pos + (first_line * no_cols);
     amount = no_lines * no_cols;
   } 
  elif (operand eq op_UpperByte)
   { pos = TopByte_pos + (first_line * no_cols);
     amount = no_lines * no_cols;
   }
  else
   FAIL;

  if (lseek(syn_stream, pos, SEEK_SET) eq -1)
   FAIL;
   
  if (read(syn_stream, data, amount) ne amount)
   FAIL;
  else
   SUCCEED;
}



