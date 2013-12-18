/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- netutils : printtf							--
--									--
--	Author:  BLV 1/5/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/nuprttf.c,v 1.7 1993/08/11 10:40:19 bart Exp $*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <message.h>
#include <syslib.h>
#include <nonansi.h>
#include "private.h"
#include "exports.h"
#include "rmlib.h"

void PrintTask(RmTask Task, RmNetwork domain, int level)
{ int	i, ptype, conns;
  int	args;
  unsigned long mem;

  for (i = 0; i < level; i++) putchar(' ');
  ptype = RmGetTaskType(Task);
  if ((ptype >= RmT_Known) || (ptype < 0))
   ptype = RmT_Unknown;
  printf("Task %8s : ", RmGetTaskId(Task));

  if ((ptype ne RmT_Unknown) && (ptype ne RmT_Default))
   printf("processor type %s, ", RmT_Names[ptype]);
  if (RmIsTaskNative(Task))
   printf("(native)");
  mem = RmGetTaskMemory(Task);
  if (mem ne 0L)
   printf(" , memory requirement 0x%08lx ", mem);
  putchar('\n');
  for (i = 0; i <= level; i++) putchar(' ');
  printf("Code %s\n", RmGetTaskCode(Task));
  for (i = 0; i <= level; i++) putchar(' ');

  args = RmCountTaskArguments(Task);
  if (args > 1)
   { printf("args: ");
     for (i = 1; i < args; i++)
      printf("(%d) %s ", i, RmGetTaskArgument(Task, i));
     putchar('\n');
     for (i = 0; i <= level; i++) putchar(' ');
   }

  conns = RmCountChannels(Task);
  printf("%d channels :", conns);
  for (i = 0; i < conns; i++)
   { RmTask Target;
     int destchannel;
     printf(" (%d) ", i);
     Target = RmFollowChannel(Task, i, &destchannel);
     if (Target eq RmM_NoTask) 
      continue;
     elif (Target eq RmM_ExternalTask)
      { const char	*filename;
        int		mode;
        filename = RmFollowChannelToFile(Task, i, &mode);
        if (filename eq Null(const char))
         { printf("Error: component %s, channel %d, should be file, RmErrno %d\n",
         	RmGetTaskId(Task), i, mode);
         }
        else
         printf("file %s, mode %x", filename, mode);
      }
     elif (Target eq (RmTask)NULL)
      printf("!!!NULL!!!");
     else
      { printf("%s", RmGetTaskId(Target));
        if (destchannel ne RmM_AnyChannel)
         printf("<%d>", destchannel);
      }
   }

  if (domain ne (RmNetwork) NULL)
   { RmProcessor processor = RmFollowTaskMapping(domain, Task);
     if (processor ne (RmProcessor) NULL)
      { putchar('\n');
        for (i = 0; i <= level; i++) putchar(' ');
        printf("Mapped to processor %s", RmGetProcessorId(processor));
      }
   }

  putchar('\n');
}

void PrintSubTaskforce(RmTaskforce Taskforce, RmNetwork domain, int level)
{ int		i;
  RmTask	Task;
  
  for (i = 0; i < level; i++) putchar(' ');
  if (level eq 0)
   printf("Taskforce");
  else
   printf("Sub-Taskforce");
  printf(" %s\n", RmGetTaskforceId(Taskforce));
  
  for (Task = RmFirstTask(Taskforce);
       Task ne (RmTask)NULL;
       Task = RmNextTask(Task) )
   { if (RmIsTaskforce(Task))
      PrintSubTaskforce((RmTaskforce) Task, domain, level+1);
     else
      PrintTask(Task, domain, level+1);
   }
}

void PrintTaskforce(RmTaskforce Taskforce, RmNetwork domain)
{ PrintSubTaskforce(Taskforce, domain, 0);
}

