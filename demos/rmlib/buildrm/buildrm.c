/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- buildrm.c								--
--                                                                      --
--	Public command to build a network resource map.			--
--                                                                      --
--	Author:  BLV 10/9/90						--
--                                                                      --
------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <rmlib.h>

#define	NullHardware		0
#define	TramHardware		1
#define	ParsytecHardware	2
#define	TelmatHardware		3
#define	MeikoHardware		4

static	void	usage(void);
static	void	produce_text_map(char *, int, int, int);
static	void	produce_binary_map(char *, int, int, int);

	/* Main(), this just attempts to parse the arguments and call	*/
	/* other routines to do the real work.				*/
int	main(int argc, char **argv)
{ bool	textmode	= TRUE;
  char	*output_file	= (char *) NULL;
  int	number_rows	= 0;
  int	number_columns	= 0;
  int	hardware	= NullHardware;
   
  for (argc--, argv++; argc > 0; argc--, argv++)
   { if (**argv == '-')
      { if (!strcmp(*argv, "-rm"))
         { textmode = TRUE; continue; }
        if(!strcmp(*argv, "-map"))
         { textmode = FALSE; continue; }
        if ((*argv)[1] == 'o')
         { if ((*argv)[2] == '\0')
            { if (argc == 0) usage();
              output_file = *(++argv);
              argc--;
            }
           else
            output_file = &((*argv)[2]);
           continue;
         }
        usage();
      }
     if (isdigit(**argv))
      { char *str = *argv;
        for ( ; (*str != 'x') && (*str != '\0'); str++)
         { if (!isdigit(*str)) usage();
           number_rows = (10 * number_rows) + (*str - '0');
         }
        if (*str == '\0') usage();
        str++;
        if (!isdigit(*str)) usage();
        for ( ; *str != '\0'; str++)
         { if (!isdigit(*str)) usage();
           number_columns = (10 * number_columns) + (*str - '0');
         }
        continue;
      } 
     if (!strcmp(*argv, "tram"))
      { hardware = TramHardware; continue; }
     if (!strcmp(*argv, "parsytec"))
      { hardware = ParsytecHardware; continue; }
     if (!strcmp(*argv, "telmat"))
      { hardware = TelmatHardware; continue; }
     if (!strcmp(*argv, "meiko"))
      { hardware = MeikoHardware; continue; }
     fprintf(stderr, "buildrm: unknown option %s\n", *argv);
     usage();
  }

  if ((output_file == (char *) NULL) && !textmode)
   { fputs("buildrm: missing output file definition.\n", stderr);
     usage();
   }

  if ((number_rows <= 0) || (number_columns <= 0))
   { fputs("buildrm: missing network size.\n", stderr);
     usage();
   }
    
  if (textmode)
   produce_text_map(output_file, number_rows, number_columns, hardware);
  else
   produce_binary_map(output_file, number_rows, number_columns, hardware);
  return(EXIT_SUCCESS);  
}

/*----------------------------------------------------------------------*/
static	void	usage(void)
{ fputs(
  "buildrm: usage, buildrm -o <file> [-rm | -map] ROWSxCOLUMNS [hardware]\n", 
  	stderr);
  fputs("       : hardware should be one of tram, parsytec, telmat or meiko\n",
  	stderr);
  fputs("       : example, buildrm -o  default.map -map 5x4 telmat\n", stderr);
  exit(EXIT_FAILURE);
}

/*----------------------------------------------------------------------*/
/* Producing a text resource map is fairly easy.			*/
/* It just involves outputting fairly constant strings.			*/

static	void	produce_text_map(char *file, int rows, int cols, int hardware)
{ FILE	*output;
  int	i, j;
  
  if (file == (char *) NULL)
   output = stdout;
  else
   { output = fopen(file, "w");
     if (output == (FILE *) NULL)
      { fprintf(stderr, "buildrm: failed to open output file %s\n", file);
        exit(EXIT_FAILURE);
      }
   }
  fputs("Network /Net {\n", output);
  if (hardware == TelmatHardware)
   fputs("\tProcessor root { ~IO, ~00, , ext; system; }\n", output);
  else
   fputs("\tProcessor root { ~IO, ~00, , ; system; }\n", output);
  fputs("\tProcessor IO   { ~root; IO; }\n", output);
  fputs("\t{\n", output);
  switch(hardware)
   { case	NullHardware :
   	fputs("\t\tReset { driver; ; null_ra.d }\n", output);
   	break;
     case	TramHardware :
     	fputs("\t\tReset { driver; ; tram_ra.d }\n", output);
     	break;
     case	ParsytecHardware :
     	fputs("\t\tReset { driver; ; pa_ra.d }\n", output);
     	break;
     case	TelmatHardware :
     	fputs("\t\tReset { driver; silent; telmat_r.d }\n", output);
     	fputs("\t\tConfigure { driver; silent; telmat_c.d }\n", output);
     	break;
     case	MeikoHardware	:
     	fputs("\t\tReset { driver; ; rte_ra.d }\n", output);
     	fputs("\t\tConfigure { driver; ; rte_c.d }\n", output);
     	break;
   }

   for (i = 0; i < rows; i++)
    for (j = 0; j < cols; j++)
     { fprintf(output, "\t\tProcessor %02d { ", (i * cols) + j);

	/* link 0 */
       if ((i == 0) && (j == 0))
        fputs(" ~root,", output);
       elif (i == 0)
        fputs("      ,", output);
       else
        fprintf(output, "   ~%02d,", ((i - 1) * cols) + j);
        
       /* link 1 */
       if (j == (cols - 1))
        fputs("    ,", output);
       else
        fprintf(output, " ~%02d,", (i * cols) + j + 1);
        
       /* link 2 */
       if (i == (rows -1))
        fputs("    ,", output);
       else
        fprintf(output, " ~%02d,", ((i + 1) * cols) + j);
        
       /* and link 3 */
       if (j == 0)
        fputs("    ", output);
       else
        fprintf(output, " ~%02d", (i * cols) + j - 1);
        
       fputs("; }\n", output);
     }
  fputs("\t}\n}\n", output);
}	

/*----------------------------------------------------------------------*/
/* Producing binary files is slightly more advanced. First, the basic	*/
/* structures for the whole network and for the I/O and root processors	*/
/* are allocated and initialised. Then every processor in the main grid	*/
/* is allocated and initialised. Since the two dimensions of the grid	*/
/* are not yet known, two levels of indirection are required.		*/

static  int	ensure_link3(RmProcessor processor, ...);

static	void	produce_binary_map(char *file, int rows, int cols, int hardware)
{ RmNetwork		result	= RmNewNetwork();
  RmProcessor		IO	= RmNewProcessor();
  RmProcessor		Root	= RmNewProcessor();
  RmProcessor		**row_table;
  int			i, j;
  char			namebuf[16];
  RmHardwareFacility	reset;
  RmHardwareFacility	config;
      
  if ((result == (RmNetwork) NULL) || (IO == (RmProcessor) NULL) ||
      (Root == (RmProcessor) NULL))
   { fputs("buildrm: out of memory\n", stderr);
     exit(EXIT_FAILURE);
   }

  if (    
      (RmSetNetworkId(result, "Net")           != RmE_Success) ||
      (RmSetProcessorId(IO, "IO")              != RmE_Success) ||
      (RmSetProcessorPurpose(IO, RmP_IO)       != RmE_Success) || 
      (RmSetProcessorId(Root, "root")          != RmE_Success) ||
      (RmAddtailProcessor(result, Root)        == (RmProcessor) NULL) ||
      (RmAddtailProcessor(result, IO)          == (RmProcessor) NULL) ||
      (RmMakeLink(IO, 0, Root, 0)              != RmE_Success) ||
      (RmSetProcessorPurpose(Root, RmP_System) != RmE_Success))
    { fputs("buildrm : error building network\n", stderr);
      exit(EXIT_FAILURE);
    }
    
  row_table = (RmProcessor **) malloc(rows * sizeof(RmProcessor *));  
  if (row_table == (RmProcessor **) NULL)
   { fputs("buildrm : out of memory\n", stderr);
     exit(EXIT_FAILURE);
   }
  row_table[0] = (RmProcessor *) malloc(rows * cols * sizeof(RmProcessor));
  if (row_table[0] == (RmProcessor *) NULL)
   { fputs("buildrm : out of memory\n", stderr);
     exit(EXIT_FAILURE);
   }

  for (i = 1; i < rows; i++)
   row_table[i] = &((row_table[i-1])[cols]);

  for (i = 0; i < rows; i++)
   for (j = 0; j < cols; j++)
    { (row_table[i])[j] = RmNewProcessor();
      if ((row_table[i])[j] == (RmProcessor) NULL)
       { fputs("buildrm: out of memory\n", stderr);
         exit(EXIT_FAILURE);
       }
      sprintf(namebuf, "%02d", (cols * i) + j);
      if (
         (RmSetProcessorId((row_table[i])[j], namebuf) != RmE_Success) ||
         (RmSetProcessorPurpose((row_table[i])[j], RmP_Helios) != RmE_Success))
       { fputs("buildrm: error initialising grid\n", stderr);
         exit(EXIT_FAILURE);
       }
      if (RmAddtailProcessor(result, (row_table[i])[j]) == (RmProcessor) NULL)
       { fputs("buildrm: error adding processor to network\n", stderr);
         exit(EXIT_FAILURE);
       }
    }

	/* Having obtained all the processors, it is possible to build	*/
	/* the connections between the processors.			*/
  for (i = 0; i < rows; i++)
   for (j = 0; j < cols; j++)
    { 	/* first link 0, which may involve going to the root */
      if ((i == 0) && (j == 0))
       { if (RmMakeLink(Root, 1, (row_table[0])[0], 0) != RmE_Success)
          { fputs("buildrm: failed to make link.\n", stderr);
            exit(EXIT_FAILURE);
          }
       }
      else if (i != 0)
       { if (RmMakeLink((row_table[i])[j], 0, (row_table[i-1])[j], 2) !=
       	     RmE_Success)
       	  { fputs("buildrm: failed to make link.\n", stderr);
       	    exit(EXIT_FAILURE);
       	  }
       }
       
      	/* now link 1	*/
      if (j < (cols - 1))
       if (RmMakeLink((row_table[i])[j], 1, (row_table[i])[j+1], 3) !=
       	    RmE_Success)
       	{ fputs("buildrm: failed to make link.\n", stderr);
       	  exit(EXIT_FAILURE);
       	}
       	
       /* thanks to symmetry all links 2 and 3 will be made by the	*/
       /* above calls as well.						*/
    }

  switch(hardware)
   { case	NullHardware :
   	reset.Type		= RmH_ResetDriver;
   	reset.NumberProcessors	= (rows * cols);       
	strcpy(reset.Name, "null_ra.d");
	reset.Option[0]		= '\0';
	reset.Processors	= row_table[0];
	if (RmAddHardwareFacility(result, &reset) != RmE_Success)
	 { fputs("buildrm: failed to add reset driver.\n", stderr);
	   exit(EXIT_FAILURE);
	 }
	break;

      case	TramHardware :	
   	reset.Type		= RmH_ResetDriver;
   	reset.NumberProcessors	= (rows * cols);       
	strcpy(reset.Name, "tram_ra.d");
	reset.Option[0]		= '\0';
	reset.Processors	= row_table[0];
	if (RmAddHardwareFacility(result, &reset) != RmE_Success)
	 { fputs("buildrm: failed to add reset driver.\n", stderr);
	   exit(EXIT_FAILURE);
	 }
	break;

      case	ParsytecHardware :
   	reset.Type		= RmH_ResetDriver;
   	reset.NumberProcessors	= (rows * cols);       
	strcpy(reset.Name, "pa_ra.d");
	reset.Option[0]		= '\0';
	reset.Processors	= row_table[0];
	if (RmAddHardwareFacility(result, &reset) != RmE_Success)
	 { fputs("buildrm: failed to add reset driver.\n", stderr);
	   exit(EXIT_FAILURE);
	 }
	break;

      case	TelmatHardware	:
       	reset.Type		= RmH_ResetDriver;
   	reset.NumberProcessors	= (rows * cols);       
	strcpy(reset.Name, "telmat_r.d");
	strcpy(reset.Option, "silent");
	reset.Processors	= row_table[0];
	if (RmAddHardwareFacility(result, &reset) != RmE_Success)
	 { fputs("buildrm: failed to add reset driver.\n", stderr);
	   exit(EXIT_FAILURE);
	 }
   	config.Type		= RmH_ConfigureDriver;
   	config.NumberProcessors	= (rows * cols);       
	strcpy(config.Name, "telmat_c.d");
	strcpy(config.Option, "silent");
	config.Processors	= row_table[0];
	if (RmAddHardwareFacility(result, &config) != RmE_Success)
	 { fputs("buildrm: failed to add configure driver.\n", stderr);
	   exit(EXIT_FAILURE);
	 }
	break;

      case	MeikoHardware	:
       	reset.Type		= RmH_ResetDriver;
   	reset.NumberProcessors	= (rows * cols);       
	strcpy(reset.Name, "rte_ra.d");
	reset.Option[0]		= '\0';
	reset.Processors	= row_table[0];
	if (RmAddHardwareFacility(result, &reset) != RmE_Success)
	 { fputs("buildrm: failed to add reset driver.\n", stderr);
	   exit(EXIT_FAILURE);
	 }
   	config.Type		= RmH_ConfigureDriver;
   	config.NumberProcessors	= (rows * cols);       
	strcpy(config.Name, "rte_c.d");
	config.Option[0]	= '\0';
	config.Processors	= row_table[0];
	if (RmAddHardwareFacility(result, &config) != RmE_Success)
	 { fputs("buildrm: failed to add configure driver.\n", stderr);
	   exit(EXIT_FAILURE);
	 }
	break;
     }      

  (void) RmApplyNetwork(result, &ensure_link3);

  if (RmWrite(file, result, (RmTaskforce) NULL) != RmE_Success)
   fprintf(stderr, "buildrm: failed to write network to file %s\n", file);
}

	/* Ensure that every real processor has all four links declared. */
static int ensure_link3(RmProcessor processor, ...)
{
  if (RmIsNetwork(processor))
   return(RmApplyNetwork((RmNetwork) processor, &ensure_link3));
   
  if ((RmGetProcessorPurpose(processor) & RmP_Mask) == RmP_IO) return(0);
  if (RmCountLinks(processor) >= 4) return(0);
  RmMakeLink(processor, 3, RmM_ExternalProcessor, 0);
  RmBreakLink(processor, 3);
}
