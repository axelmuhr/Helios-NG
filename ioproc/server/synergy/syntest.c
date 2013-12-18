/*=========================================================================
===		Synergy Interface Test Program				===
===									===
===	Copyright (C) 1989, Perihelion Software Ltd.			===
===	All Rights Reserved						===
===									===
===	Author : BLV, 13.1.89						===
=========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <helios.h>
#include "syndef.h"
#define eq ==
#define ne !=
#define failwith(a) { fprintf(stderr, a); return(1); }

int main(void)
{ BYTE *buffer;
  int i, j, temp;
  BYTE *start, *end;
  int *wbuff;
  
  if ((buffer = malloc(512 * 768)) eq (BYTE *) NULL)    
    failwith("Failed to allocate buffer.\n")
   
  if (!syn_Init(Synergy1))
    failwith("Failed to initialise interface library.\n")

  printf("Rebooting board.\n");
  if (!syn_BootBoard())
   failwith("Failed to boot board.\n")
  printf("Board booted.\n");
return(0);

  if (!syn_Restart())
   failwith("Failed to restart board.\n")

  printf("Reading picture.\n");
  
  if (!syn_ReadVideo(0, 512, op_UpperByte, buffer))
   failwith("Failed to read video memory.\n")

  printf("Picture read.\n");

  start = buffer; end = (buffer + (768 * 512) - 1);
  for (i = 0; i < (768 * 256); i++)
   { temp = *start; *start++ = *end; *end-- = temp; }

  printf("Picture inverted.\n");

  if (!syn_WriteVideo(0, 512, op_UpperByte, buffer))
   failwith("Failed to write video memory.\n")
  
  printf("Picture written.\n");

  if (!syn_Tidy())
   failwith("Failed to tidy interface library.\n")

  free(buffer);
     
  printf("Success.\n");
  return(0);
}
