/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : printnet							--
--									--
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsID: $Header: /hsrc/network/RCS/nuprtnet.c,v 1.10 1993/08/11 10:40:06 bart Exp $*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <message.h>
#include <queue.h>
#include <syslib.h>
#include <nonansi.h>
#include <root.h>
#include <posix.h>
#include "private.h"
#include "exports.h"
#include "rmlib.h"

typedef	struct	OwnerDetails {
	Node	Node;	
	int	Uid;
	char	Name[NameMax];
} OwnerDetails;

static List	OwnerList;
static FILE	*output;
static bool	popened = FALSE;

static void spaces(int x)
{ for ( ; x > 0; x--) fputc(' ', output);
}

static char	*find_owner(int uid)
{ OwnerDetails	*node;
  char		*temp;
  
  for (node = Head_(OwnerDetails, OwnerList);
       !EndOfList_(node);
       node = Next_(OwnerDetails, node))
   if (node->Uid eq uid)
    return(node->Name);

  node = New(OwnerDetails);
  if (node eq Null(OwnerDetails))
   return("<unknown>");
  node->Uid	= uid;
  temp		= (char *) RmWhoIs(uid);
  if (temp eq Null(char))
   strcpy(node->Name, "<unknown>");
  else
   strcpy(node->Name, temp);
  AddTail(&OwnerList, &(node->Node));
  return(node->Name);
}

void PrintProcessor(RmProcessor Processor, int level)
{ int i, ptype, conns, state, purpose, owner, control;

	/* Line one : processor root : system, T800, memory 0x00200000 */
	/* or       : processor IO   : IO */
  spaces(8 * level);
  fprintf(output, "Processor %9s : ", RmGetProcessorId(Processor));
  ptype = RmGetProcessorType(Processor);
  if ((ptype >= RmT_Known) || (ptype < 0)) ptype = RmT_Unknown;
  purpose = RmGetProcessorPurpose(Processor);
  if (purpose & RmP_System)
   fputs("system ", output);
   
  switch(purpose & RmP_Mask)
   { case RmP_Helios : fputs("helios", output); break;
     case RmP_IO     : fputs("IO",     output); break;
     case RmP_Native : fputs("native", output); break;
     case RmP_Router : fputs("router", output); break;
     default	: fprintf(output, "<unknown purpose %d>", purpose);
   }
  if ((purpose & RmP_Mask) eq RmP_Helios)
   fprintf(output, ", %s, memory 0x%08lx", RmT_Names[ptype],
           RmGetProcessorMemory(Processor));
  fputc('\n', output);

	/* Second line : Current processor state : running */
  spaces((8 * level) + 2);
  fputs("Current processor state : ", output);
  state = RmGetProcessorState(Processor);
  if (state & RmS_Dead)
   fputs("dead", output);
  elif (state & RmS_Crashed)
   fputs("crashed", output);
  elif (state & RmS_Suspicious)
   fputs("suspicious", output);
  elif (state & RmS_Running)
   fputs("running", output);
  fputc(' ', output);
  if (state & RmS_Booting)
   fputs("booting ", output);
  if (state & RmS_AutoBoot)
   fputs("auto-reboot ", output);
  if (state & RmS_Reset)
   fputs("reset ", output);
  if (state & RmS_ShouldBeReset)
   fputs("should-be-reset", output);
  fputc('\n', output);

	/* third line	: current owner : bart */
  spaces((8 * level) + 2);
  fputs("Current owner : ", output);
  owner = RmGetProcessorOwner(Processor);
  fputs(find_owner(owner), output);
   
  fputc('\n', output);

#if 1
	/* Fourth line, amount of control */
  spaces((8 * level) + 2);
  control = RmGetProcessorControl(Processor);
  fputs("Control : ", output);
  if (control eq (RmC_FixedMapping + RmC_FixedLinks))
   fputs("none, ", output);
  if (control & RmC_Native)
   fputs("native possible, ", output);
  if (control & RmC_Reset)
   fputs("definite reset, ", output);
  if (control & RmC_PossibleReset)
   fputs("possible reset only, ", output);
  unless(control & RmC_FixedMapping)
   fputs("mapping flexible, ", output);
  if (control & RmC_FixedLinks)
   fputs("links not configurable", output);
  else
   fputs("links configurable", output);
  fputc('\n', output);
#endif

	/* Fourth/Fifth line : 4 link connections */
  spaces((8 * level) + 2);
  conns = RmCountLinks(Processor);
  fprintf(output, "%d link connections\n", conns);

	/* For every link: link 0 : processor root, link 2 */
  for (i = 0; i < conns; i++)
   { RmProcessor Target;
     int destlink;
     spaces((8 * level) + 4);
     fprintf(output, "link %d : ", i);
     Target = RmFollowLink(Processor, i, &destlink);
     if (Target eq RmM_NoProcessor)
      fputs("not connected", output);
     elif (Target eq RmM_ExternalProcessor)
      { fputs("external", output);
        fprintf(output, "[%d]", destlink);
      }
     elif (Target eq (RmProcessor)NULL)
      fputs("!!!NULL!!!", output);
     else
      { fprintf(output, "processor %s", RmGetProcessorId(Target));
        fprintf(output, ", link %d", destlink);
      }
     fputc('\n', output);
   }
}

static int PrintHardware(RmHardwareFacility *hardware, ...)
{ va_list	args;
  int		level;
  int		i;
    
  va_start(args, hardware);
  level = va_arg(args, int);
  va_end(args);

  spaces(8 * level);
  fputs("Hardware facility : ", output);
  switch(hardware->Type)
   { case RmH_ResetDriver : fputs("reset driver ", output); break;
     case RmH_ConfigureDriver : fputs("configuration driver ", output); break;
     case RmH_ResetCommand : fputs("reset command, run ", output); break;
     default : fputs("!!! unknown type !!!\n", output); return(1);
   }
   
  fputs(hardware->Name, output);
  if (hardware->Option[0] ne '\0')
   { fputs(", option ", output);
     fputs(hardware->Option, output);
   }
  fputs("\n", output);
  spaces(8 * level);
  
  fprintf(output, "processor%s affected (%d) : ", 
          (hardware->NumberProcessors > 1) ? "s" : "",
          hardware->NumberProcessors);

  for (i = 0; i < hardware->NumberProcessors; i++)
   { if (i ne 0) fputs(", ", output);
     fputs(RmGetProcessorId(hardware->Processors[i]), output);
   }
  fputs("\n", output);
     
  return(0);
} 

void PrintSubnet(RmNetwork Subnet, int level)
{ RmProcessor Processor;
  spaces(8 * level);
  if (level eq 0)
   fputs("Network", output);
  else
   fputs("Subnet", output);
  fprintf(output, " %s\n", RmGetNetworkId(Subnet));

  (void) RmApplyHardwareFacilities(Subnet, &PrintHardware, level+1);
  
  for (Processor = RmFirstProcessor(Subnet);
       Processor ne (RmProcessor)NULL;
       Processor = RmNextProcessor(Processor) )
   { if (RmIsNetwork(Processor))
      PrintSubnet((RmNetwork) Processor, level+1);
     else
      PrintProcessor(Processor, level+1);
   }
}

void init_PrintNetwork(bool filter)
{ InitList(&OwnerList);
    
  if (isatty(1) && filter)
   { output = popen("/helios/bin/more", "w");
     if (output eq (FILE *) NULL)
      output = stdout;
     else
      popened = TRUE;
   }
  else
   output = stdout;
}

void tidy_PrintNetwork(void)
{  
 if (popened)
  { fflush(output); pclose(output); }
}

void PrintNetwork(RmNetwork Network)
{ init_PrintNetwork(TRUE); 
  PrintSubnet(Network, 0);
  tidy_PrintNetwork();
}


