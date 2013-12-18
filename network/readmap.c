/*------------------------------------------------------------------------
--                                                                      --
--           H E L I O S   N E T W O R K I N G   S O F T W A R E	--
--           ---------------------------------------------------	--
--                                                                      --
--             Copyright (C) 1990, Perihelion Software Ltd.             --
--                        All Rights Reserved.                          --
--                                                                      --
-- readmap.c								--
--                                                                      --
--	Purpose : read an old-style resource map binary and convert 	--
--	it to a RmNetwork structure.					--
--                                                                      --
--	Author:  BLV 26/7/90						--
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Header: /hsrc/network/RCS/readmap.c,v 1.10 1994/03/10 17:12:36 nickc Exp $*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <message.h>
#include <syslib.h>
#include <nonansi.h>
#include <posix.h>
#include "private.h"
#include "exports.h"
#include "netutils.h"
#include "rmlib.h"

#ifdef Malloc
#undef Malloc
#endif

/**
*** Code to read the resource map. This is tricky. It has to cope
*** with the current format of resource maps, as produced by the
*** Resource Management library, and the old format used by Charlie.
***
***
*** The resource map is a binary file, so I do not want to give detailed
*** diagnostics when some read error occurs. Instead a simple message
*** is generated and the program exits.
***
*** The following variables should be defined in the parent program,
*** i.e. rmgen or rmtrans
**/
extern	char		*ProgramName;
extern	RmNetwork	Network;
extern	RmTaskforce	Taskforce;
extern	void		fatal(void);

#ifdef __TRAN
static void maperror(int number)
{
  printf("%s : error 0x%x reading resource map.\n", ProgramName, number);
  fatal();
}
#endif

static void mapmemory(int number)
{
  printf("%s : insufficient memory to read the resource map (%d).\n",
  	 ProgramName, number);
  fatal();
}

/**
*** Reading in the resource map. First, an attempt is made to use
*** the Resource Management library routine, RmRead(), which will
*** work if the resource map has been recompiled with the new rmgen.
*** If this fails with the error code RmE_oldStyle, an attempt is
*** made to read in an oldstyle resource map. These had the number 6
*** as the first four bytes, so they are fairly easy to detect.
**/
#ifdef __TRAN
static void read_oldstyle_map(char *filename);
#endif
static void RmResolveLinks(RmNetwork);

void read_resource_map(char *filename)
{ char realname[128];
  int  result;
  
  if (filename[0] eq '/')
   strcpy(realname, filename);
  else
   { Object *temp = Locate(cdobj(), filename);
     if (temp eq Null(Object))
      { strcpy(realname, "/helios/etc/");
        strcat(realname, filename);
        temp = Locate(Null(Object), realname);
        if (temp eq Null(Object))
	 { fprintf(stderr, "%s : failed to locate resource map %s\n",
	 		ProgramName, filename);
	   fatal();
	 }
	strcpy(realname, temp->Name);
	Close(temp);
      }
     else
      { strcpy(realname, temp->Name);
        Close(temp);
      }
   }

  if ((result = RmRead(realname, &Network, &Taskforce)) eq RmE_Success)
   { RmResolveLinks(Network); return; }

  switch (result)
   { case	RmE_NotFound :
                  printf("%s : cannot find resource map %s\n", 
                  		ProgramName, realname);
                  fatal();
#ifdef  __TRAN
     case	RmE_OldStyle :
		  read_oldstyle_map(realname);
		  return;
#endif
     case	RmE_BadFile :
     		  printf("%s : invalid resource map %s\n", ProgramName, 
     		  		realname);
     		  fatal();
     case	RmE_NoMemory :
     		  mapmemory(1);
     default :
     		  printf("%s : RmLib error reading resource map, 0x%x\n",
			 ProgramName, result);
		  fatal();
   }     		       		  
}

#ifdef __TRAN
/**
*** Reading in an old-style map is non-trivial. First the whole file
*** is read into memory, to keep things slightly simple. Then the data
*** is analysed to give a network of processors and/or subnets. As the
*** buffer is processed a pointer is maintained of the current position,
*** and the amount left in the buffer is remembered. This allows
*** continuous checking to avoid accidental buffer overflow. After
*** the call to extract_subnet() the global variable Network will
*** contain the entire network as defined by the resource map. However,
*** certain pieces of information cannot be resolved conveniently
*** until the whole network has been processed. This is done by a
*** call to resolve_subnet(). In particular, the connectivity between
*** the processors cannot be worked out sensibly until all processors
*** are known.
**/
static void extract_subnet(char **buffer, int *left, RmNetwork Network);
static void extract_processor(char **buffer, int *left, RmProcessor Processor);
static void resolve_subnet(RmNetwork Network);
static int  maxtag = 0;

static void read_oldstyle_map(char *filename)
{ Object *map_obj;
  Stream *map;
  int	 size;
  char   *data;

  map_obj = Locate(Null(Object), filename);
  if (map_obj eq Null(Object))
   { printf("%s : failed to locate resource map %s\n", ProgramName, filename);
     fatal();
   }
   
  map = Open(map_obj, Null(char), O_ReadOnly);
  if (map eq Null(Stream))
   { printf("%s : failed to open resource map %s\n", ProgramName, 
   		map_obj->Name);
     fatal();
   }
  Close(map_obj);
  
  size = GetFileSize(map);
  if (size < 0)  maperror(1);
  
  if ((data = Malloc(size)) eq Null(char))
   mapmemory(2);

  if (Read(map, data, size, -1) ne size)
   { printf("%s : error reading resource map %s, fault 0x%08x\n",
            ProgramName, map->Name, Result2(map));
     fatal();
   }
  Close(map);

  if (*((int *) data) ne 0x06)   /* magic number */
   { printf("%s : file %s is not a resource map.\n", ProgramName, filename);
     fatal();
   }
   
  if ((Network = RmNewNetwork()) eq (RmNetwork)NULL)
   mapmemory(3);

  { char *realmap = &(data[4]);
    size -= 4;
    
     /* There is a context string at the start, which I ignore. As far as */
     /* I can tell this string is always empty. */
    while ((*realmap ne '\0') && (size > 0))	/* find the terminator */
     { realmap++; size--; }
    realmap++; size--;		/* and skip past it */

    extract_subnet(&realmap, &size, Network);
    if (size > 0)
     printf(
      "%s : warning, ignoring unrecognised data at end of resource map.",
      	ProgramName);

    resolve_subnet(Network);  /* figure out the connectivity and control */
  }
   
  Free(data);
  return;  
}

/**
*** This is roughly what the old style resource maps contained. The same
*** data structure was used for processors and subnets, even though
*** they have very different requirements. This consisted of a single
*** block, as shown below, some connections, the name, control information,
*** attributes, and a null string to terminate it. In the case of a
*** subnet this data is followed immediately by its contents. Most of the
*** code below is for masochists only.
**/
typedef struct old_block {
	int	type;
	int	no_subnets;
	int	tag;
	int	status;
	int	connectivity;
	void	*connections;
	int	ptypes[6];  /* for every processor, honest !!! */
	int	function;   /* how many 68000's, how many T2's, etc. */
	int	memory;
	int	spare[2];
} old_block;

typedef struct old_connection {
	char	t_type;		/* the LinkType substructure */
	char	t_mode;
	char	t_configtype;
	char	t_dest;
	char	c_flags;	/* kernel's LinkConf structure */
	char	c_mode;
	char	c_state;
	char	c_id;
	void	*target_subptr;
	int	target_tag;
	int	target_linkno;
	void	*source_subptr;
	int	source_tag;
	int	source_linkno;
} old_connection;

/**
*** This routine is used to extract a subnet from the buffer which has
*** been used to read in the entire resource map. It can be called
*** recursively, if the network contains nested subnets. Various pieces
*** of information are left unresolved until the whole network has been
*** processed. In particular, the network interconnectivity and the
*** reset and configuration control cannot be sorted out until I have
*** the entire network in my own data structures. Resolving these is
*** left to routine resolve_subnet(), called after extract_subnet()
*** has dealt with the entire network.
***
*** To manage this, various pieces of information are cached in the
*** RmNetwork and RmProcessor structures:
***   RmNetwork   : reset Mnode in 	DirNode.Dates.Creation
***		    configuration Mnode in DirNode.Dates.Modified
***   RmProcessor : tag in          	ObjNode.Key
***		    connectivity in	ObjNode.Matrix
***		    reset driver in	ObjNode.Dates.Access
***		    Configuration driver in ObjNode.Dates.Creation
***		    CONNECTION pointer in ObjNode.Dates.Modified
**/
static void extract_subnet(char **bufptr, int *leftptr, RmNetwork Network)
{ old_block block;
  char      *netname;
  char	    *buffer = *bufptr;
  int	    left   = *leftptr;
  char	    *Reset_mnode, *Configure_mnode;

  if (left <= 0) maperror(2);
  
     /* The context is followed by a block of data. The block may not */
     /* be aligned correctly, so I have to copy it. It is a multiple  */
     /* of four bytes so there are no problems with sizeof()          */
  if (left ne sizeof(old_block)) maperror(3);
  memcpy((void *) &block, (void *) buffer, sizeof(old_block));
  buffer = &(buffer[sizeof(old_block)]);	/* adjust pointer */
  left  -= sizeof(old_block);

  if (block.type ne 0) maperror(4);  /* type_subnet is 0 */

	/* The block is followed by the subnet connectivity. This has	*/
	/* something to do with the old external link scheme, with the	*/
	/* first connection corresponding to EXT[0] and so on. As far	*/
	/* as I am concerned this should be ignored.			*/
  if (block.connectivity > 0)
   {
     if (left < (block.connectivity * sizeof(old_connection))) maperror(5);
     buffer = &(buffer[block.connectivity * sizeof(old_connection)]);
     left  -= block.connectivity * sizeof(old_connection);
   }

	/* After the connectivity comes the subnet name. This information  */
	/* is actually useful. Unfortunately the full name, e.g. /net/subA */
	/* instead of just the useful bit subA, is given and I have to do  */
	/* some stripping.						   */
  if (*buffer ne '/') maperror(6);	/* safetycheck */
  while ((*buffer ne '\0') && (left > 0))
   { buffer++; left--; }
  netname = buffer;
  while (*(--netname) ne '/'); netname++; /* get back to just after the / */
  buffer++; left--;			  /* skip the terminator */
  RmSetNetworkId(Network, netname);
  

	/* The next field holds the control information. There is a single */
	/* word: 0=>no control, 1=>reset only, 2=>configuration only, and  */
	/* 3=>both. This is followed by two strings, possibly empty, giving*/
	/* the Mnode. Unfortunately the device driver name is held with    */
	/* the processor, and I have to do some nasty stuff later on to get*/
	/* the information in a useful form. For now the code just notes   */
	/* the MNodes.							   */
  if (left < ((int) sizeof(int) + 2)) maperror(7);/* integer plus two strings*/
  buffer += sizeof(int); left -= sizeof(int);	/* ignore the integer	   */
  Reset_mnode = buffer;				/* Reset Mnode comes first */
  while ((*buffer ne '\0') && (left > 0))
   { buffer++; left--; }
  if (left < 1) maperror(8);
  Configure_mnode = ++buffer; left--;		/* Then the Configure Mnode */
  while ((*buffer ne '\0') && (left > 0))
   { buffer++; left--; }
  buffer++; left--;				/* past the terminator */
  if (left <= 0) maperror(9);
  if (*Reset_mnode ne '\0')
   Network->DirNode.Dates.Creation = (Date) Reset_mnode;
  else
   Network->DirNode.Dates.Creation = (Date) 0;

  if (*Configure_mnode ne '\0')
   Network->DirNode.Dates.Modified = (Date)Configure_mnode;
  else
   Network->DirNode.Dates.Modified = (Date)0;
      
   
	/* Following the control information comes the Network		*/
	/* attributes. These are just strings followed by integers	*/
	/* terminated by an empty string. All subnet attributes are	*/
	/* ignored.	*/
  while (*buffer ne '\0')    /* until the null string is reached */
   {
     for (buffer++, left--; (*buffer ne '\0') && (left > 0);
          buffer++, left--);
     buffer += 5; left -= 5;	/* skip past the terminator and the integer*/
     if (left <= 0) maperror(10);
   }
  buffer++; left--;  /* skip past the null string */
  
	/* And that finishes off the subnet information. Every subnet is */
	/* followed immediately by its subnets or terminals, and these   */
	/* are extracted one by one, using a recursive call to this	 */
	/* routine if necessary. The first word specifies the type, 0	 */
	/* for subnet, 1 for terminal, but may not be aligned correctly. */
  for ( ; block.no_subnets > 0; block.no_subnets--)
   { int temp;

     if (left < (int) sizeof(old_block)) maperror(11);
     memcpy((void *) &temp, (void *) buffer, sizeof(int));
     if (temp eq 0)  /* another subnet */
      { RmNetwork SubNetwork = RmNewNetwork();
        if (SubNetwork eq (RmNetwork)NULL) mapmemory(4);
        extract_subnet(&buffer, &left, SubNetwork);
        unless(RmAddtailProcessor(Network, (RmProcessor) SubNetwork))
	         maperror(12);
      }
     elif (temp eq 1)  /* a processor */
      { RmProcessor Processor = RmNewProcessor();
        if (Processor eq (RmProcessor)NULL) mapmemory(5);
        extract_processor(&buffer, &left, Processor);
        unless(RmAddtailProcessor(Network, Processor))
         maperror(13);
      }
     else
      maperror(14);
   }
   
  *leftptr = left;
  *bufptr  = buffer;
}

/**
*** This routine is used to extract a processor from the buffer. The
*** current buffer points at an old_block structure, which is known to
*** correspond to a processor. This is followed by connectivity, the
*** network address (/net/00), control information (probably empty),
*** and zero or more string attributes terminated by a null string.
*** The connectivity and control information are left until later.
**/

static void extract_processor(
    char **bufptr, int *restptr, RmProcessor Processor)
{ char *buffer = *bufptr;
  int  left    = *restptr;
  old_block block;
  char	    *processor_name;
  char	    *reset_driver;
  char	    *configure_driver;
  int       result;
  
     /* Copy the old_block structure onto the stack, to guarantee a */
     /* sensible alignment. Extract_subnet() has already checked    */
     /* that the buffer contains this much information.		    */
  memcpy((void *) &block, buffer, sizeof(old_block));
  buffer = &(buffer[sizeof(old_block)]);
  left  -= sizeof(old_block);
  if (block.no_subnets ne 0) maperror(15);   /* safety check */

     /* Use the information held in the block : memory, purpose, type ... */
  if ((result = RmSetProcessorMemory(Processor, block.memory)) ne RmE_Success)
   { fprintf(stderr, "%s : RmLib error for RmSetProcessorMemory : 0x%x\n",
             ProgramName, result);
     maperror(31);
   }
   
  { int purpose = 0;
    switch (block.function)
     { case 1  : purpose = RmP_Native | RmP_System; break;
       case 2  : purpose = RmP_Helios; break;
       case 4  : purpose = RmP_IO;     break;
       case 8  : purpose = RmP_Helios | RmP_System; break;
       default : maperror(16);
     }
    if ((result = RmSetProcessorPurpose(Processor, purpose)) ne RmE_Success)
     { fprintf(stderr, "%s : RmLib error on RmSetProcessorPurpose, 0x%x\n",
     	       ProgramName, result);
       maperror(32);
     }
  }
  { int ptype;
    if (block.ptypes[1] ne 0) ptype = RmT_T212;
    elif (block.ptypes[2] ne 0) ptype = RmT_T414;
    elif (block.ptypes[3] ne 0) ptype = RmT_T800;
    elif (block.ptypes[4] ne 0) ptype = RmT_680x0;
    else ptype = RmT_Unknown;
    if (RmGetProcessorPurpose(Processor) eq RmP_IO)
     ptype = RmT_Unknown;
     
    if ((result = RmSetProcessorType(Processor, ptype)) ne RmE_Success)
     { fprintf(stderr, "%s : RmLib error on RmSetProcessorType, 0x%x\n",
     		ProgramName, result);
     	maperror(33);
     }
  }

  Processor->ObjNode.Key    = (Key) block.tag;	/* store some data to be resolved */
  Processor->ObjNode.Matrix = (Matrix) block.connectivity; /* later */
  Processor->ObjNode.Dates.Modified = (Date) buffer;
  if (maxtag < block.tag) maxtag = block.tag;
    
     /* After the main block comes the connectivity. The information */
     /* is held in the RmProcessor for now, and resolved later on.   */
  if (block.connectivity > 0)
   { if (left < (block.connectivity * (int)sizeof(old_connection)))
      maperror(17);
     buffer = &(buffer[block.connectivity * sizeof(old_connection)]);
     left  -= (block.connectivity * sizeof(old_connection));
   }
   
     /* Now comes the processor name. This is a full name, e.g. /at/tom */
     /* so it has to be stripped down to its useful fields.		*/
  if (*buffer ne '/') maperror(18);	/* safetycheck */
  while ((*buffer ne '\0') && (left > 0))
   { buffer++; left--; }
  processor_name = buffer;
  while (*(--processor_name) ne '/');     /* get back to just after the / */
  processor_name++;
  buffer++; left--;			  /* skip the terminator */
  if ((result = RmSetProcessorId(Processor, processor_name)) ne RmE_Success)
   { fprintf(stderr, "%s : RmLib error on RmSetProcessorID, 0x%x\n",
   		ProgramName, result);
     maperror(34);
   }

	/* The next field holds the control information. There is a single */
	/* word: 0=>no control, 1=>reset only, 2=>configuration only, and  */
	/* 3=>both. This is followed by two strings, possibly empty, giving*/
	/* the drivers.							   */
  if (left < ((int)sizeof(int) + 2)) maperror(19);/* integer plus two strings*/
  buffer += sizeof(int); left -= sizeof(int);	/* ignore the integer	   */
  reset_driver = buffer;			/* Reset driver comes first */
  while ((*buffer ne '\0') && (left > 0))
   { buffer++; left--; }
  if (left < 1) maperror(20);
  configure_driver = ++buffer; left--;		/* Then the Configure driver */
  while ((*buffer ne '\0') && (left > 0))
   { buffer++; left--; }
  buffer++; left--;				/* skip past the terminator */
  if (left <= 0) maperror(21);
  if (*reset_driver ne '\0')
   Processor->ObjNode.Dates.Access = (Date) reset_driver;
  if (configure_driver ne '\0')
   Processor->ObjNode.Dates.Creation = (int) configure_driver;
   
	/* Following the control information comes the Network	*/
	/* attributes. These are strings and integer pairs,	*/
	/* terminated by an empty string.			*/
  while (*buffer ne '\0')    /* until the null string is reached */
   { 
     RmAddProcessorAttribute(Processor, buffer);
     for (buffer++, left--; (*buffer ne '\0') && (left > 0);
          buffer++, left--);
     buffer += 5; left -= 5;	/* skip past the terminator and the integer */
     if (left <= 0) maperror(22);
   }
  buffer++; left--;	/* skip past the null string; */
        	
  *bufptr  = buffer;	/* Update the buffer and remaining pointers */
  *restptr = left;
}

/**
*** This routine resolves outstanding problems, in particular the network
*** connectivity and the control information. There are three separate
*** phases: resolve_resets(), resolve_configure() and resolve_links().
*** The integer argument specifies the level within the hierarchy, 1
*** meaning the top-level. This is important because the routines are
*** recursive.
**/
static void resolve_resets(RmNetwork Network, int);
static void resolve_configure(RmNetwork Network, int);
static void resolve_links(RmNetwork Network);

static void resolve_subnet(RmNetwork Network)
{
  resolve_resets(Network, 1);
  resolve_configure(Network, 1);
  resolve_links(Network);
}

/**
*** Resolve the resets for the given network (which may be a subnet).
*** If the binary resource map specified a reset mnode then the
*** Reset.Earth field will point to this.
***
*** It is very possible that the top level network does not have a reset
*** driver associated with it. If so, the routine walks through the
*** network in case there is a sub-network, and recursively resolves the
*** resets.
***
*** Otherwise, Reset.Head gives the Mnode. This is looked up, to give
*** the corresponding driver. Next the number of affected processors is
*** found by a simple RmApplyNetwork(). A driver controls all processors
*** inside the network and subnets, unless a subnet defines its own
*** reset mnode. Given this information it is possible to allocate
*** enough space for the RmHardwareFacility structure, which can be filled in.
*** Filling it in requires another Apply. The same function is used
*** for both Apply's, with the second optional argument being NULL
*** first time around.
**/

static int resets_aux(RmProcessor Processor, ...)
{ RmHardwareFacility *Reset;
  va_list junk;
  va_start(junk, Processor);
  Reset = va_arg(junk, RmHardwareFacility *);
  va_end(junk);
  
  if (RmIsNetwork(Processor)) 
   { RmNetwork Subnet = (RmNetwork) Processor;
	/* does it have its own resets ? */
     if (Subnet->DirNode.Dates.Creation ne (Date) 0)
      return(0);
     else				/* check for processors within */
      return(RmApplyNetwork(Subnet, &resets_aux, Reset));
   }
 
  if (Reset) 		/* Filling in the ResetFacility ? */
   { if (RmGetProcessorPurpose(Processor) ne RmP_IO)
       { Reset->Processors[Reset->NumberProcessors++] = Processor;
         return(1);
       }
      else
       return(0);
   }
  else
   return((RmGetProcessorPurpose(Processor) ne RmP_IO) ? 1 : 0);
}

static void resolve_resets(RmNetwork Network, int level)
{ char *Mnode, *driver, *proc_name;
  RmProcessor  realMnode;
  RmHardwareFacility *Reset;
  int number_affected;

	/* There may be separate reset schemes in lower level subnets.	*/
	/* Hence it is necessary to recurse through all subnets,	*/
	/* resolving these lower level reset schemes.			*/
  { RmProcessor Processor = RmFirstProcessor(Network);
    while (Processor ne (RmProcessor)NULL)
     { if (RmIsNetwork(Processor))
        { RmNetwork Subnet = (RmNetwork) Processor;
          resolve_resets(Subnet, level+1);
        }
       Processor = RmNextProcessor(Processor);
     }
  }
 	/* The current level of the hierarchy may not have a reset scheme */
  if (Network->DirNode.Dates.Creation eq (Date) 0)
   return;
   
    /* If there is a reset scheme at this level then the Account field */
    /* contains a pointer to the Mnode. It is necessary to find this   */
    /* Mnode in order to obtain the device driver name. The Mnode is   */
    /* something like /Cluster/00. The Lookup call needs just 00.      */
    /* Hence it is necessary to do some stripping, depending on the level */
  Mnode = (char *) Network->DirNode.Dates.Creation;
  Network->DirNode.Dates.Creation = (Date) 0;
  proc_name = &(Mnode[1]);
  for ( ; level > 0; level--)  /* figure out how many slashes to skip */
   { while ((*proc_name ne '\0') && (*proc_name ne '/'))
      proc_name++;
     if (*proc_name eq '\0') maperror(23);
     proc_name++;		/* should now be 00 */
     realMnode = (RmProcessor)
                  Lookup(&(Network->DirNode), proc_name, FALSE);          
     if (realMnode eq (RmProcessor)NULL)
      { printf("%s : Reset Mnode %s is not part of subnet /%s\n",
               ProgramName, Mnode, Network->DirNode.Name);
        maperror(24);
      }
     driver = (char *) realMnode->ObjNode.Dates.Access;
   }
   
	/* Now, the MNode string is known, as is the device driver name. */
	/* It remains to find out how many processors are affected.      */
  number_affected = RmApplyNetwork(Network, &resets_aux, NULL);
  
 	/* The ResetFacility structure can now be allocated and filled in */
  Reset = (RmHardwareFacility *) Malloc(sizeof(RmHardwareFacility)
                 + (number_affected * sizeof(RmProcessor)));
  if (Reset eq Null(RmHardwareFacility))
   mapmemory(6);
  Reset->Type		  = RmH_ResetDriver;
  Reset->NumberProcessors = 0;
  Reset->Processors	  = (RmProcessor *)
    (&((BYTE *) Reset)[sizeof(RmHardwareFacility)]);
  strncpy(Reset->Option, Mnode, 63);
  Reset->Option[63] = '\0';
  strncpy(Reset->Name, driver, 63);
  Reset->Name[63] = '\0';
  if (RmApplyNetwork(Network, &resets_aux, Reset) ne number_affected)
   maperror(25);
#ifdef SYSDEB
  Reset->Node.Next = Reset->Node.Prev = &Reset->Node;
#endif
  AddTail(&(Network->Hardware), &(Reset->Node));
}

/**
*** Resolving configuration schemes. This is very similar to
*** resolving the reset schemes.
**/

static int configure_aux(RmProcessor Processor, ...)
{ RmHardwareFacility *Driver;
  va_list junk;
  va_start(junk, Processor);
  Driver = va_arg(junk, RmHardwareFacility *);
  va_end(junk);
  
  if (RmIsNetwork(Processor)) 
   { RmNetwork Subnet = (RmNetwork) Processor;
	     /* does it have its own driver ? */
     if (Subnet->DirNode.Dates.Modified ne (Date) 0)
      return(0);
     else				/* check for processors within */
      return(RmApplyNetwork(Subnet, &configure_aux, Driver));
   }
 
  if (Driver) 		/* Filling in the Driver ? */
   { if (RmGetProcessorPurpose(Processor) ne RmP_IO)
       { Driver->Processors[Driver->NumberProcessors++] = Processor;
         return(1);
       }
      else
       return(0);
   }
  else
   return( (RmGetProcessorPurpose(Processor) ne RmP_IO) ? 1 : 0);
}

static void resolve_configure(RmNetwork Network, int level)
{ char *Mnode, *driver, *proc_name;
  RmProcessor  realMnode;
  RmHardwareFacility *Configure;
  int number_affected;

	/* There may be separate schemes in lower level subnets. */
	/* Hence it is necessary to recurse through all subnets, */
	/* resolving these lower level schemes.			 */
  { RmProcessor Processor = RmFirstProcessor(Network);
    while (Processor ne (RmProcessor)NULL)
     { if (RmIsNetwork(Processor))
        { RmNetwork Subnet = (RmNetwork) Processor;
          resolve_configure(Subnet, level+1);
        }
       Processor = RmNextProcessor(Processor);
     }
  }
 	/* The current level of the hierarchy may not have a scheme */
  if (Network->DirNode.Dates.Modified eq (Date) 0)
    return;

    /* If there is a scheme at this level then the ConfigureDriver field */
    /* contains a pointer to the Mnode. It is necessary to find this   */
    /* Mnode in order to obtain the device driver name. The Mnode is   */
    /* something like /Cluster/00. The Lookup call needs just 00.      */
    /* Hence it is necessary to do some stripping, depending on the level */
  Mnode = (char *) Network->DirNode.Dates.Modified;
  Network->DirNode.Dates.Modified = (Date) 0;
  
  proc_name = &(Mnode[1]);
  for ( ; level > 0; level--)  /* figure out how many slashes to skip */
   { while ((*proc_name ne '\0') && (*proc_name ne '/'))
      proc_name++;
     if (*proc_name eq '\0') maperror(26);
     proc_name++;		/* should now be 00 */
     realMnode = (RmProcessor)
                  Lookup(&(Network->DirNode), proc_name, FALSE);          
     if (realMnode eq (RmProcessor)NULL)
      { printf("%s : CONFIGURE Mnode %s is not part of subnet /%s\n",
               ProgramName, Mnode, Network->DirNode.Name);
        maperror(27);
      }
     driver = (char *) realMnode->ObjNode.Dates.Creation;
   }
   
	/* Now, the MNode string is known, as is the device driver name. */
	/* It remains to find out how many processors are affected.      */
  number_affected = RmApplyNetwork(Network, &configure_aux, NULL);
  Configure = Malloc(sizeof(RmHardwareFacility) +
  		(number_affected * sizeof(RmProcessor)));
  if (Configure eq Null(RmHardwareFacility))
   mapmemory(7);
  Configure->Type = RmH_ConfigureDriver;
  Configure->NumberProcessors = 0;
  Configure->Processors = (RmProcessor *)
     (&((BYTE *)Configure)[sizeof(RmHardwareFacility)]);
  strncpy(Configure->Option, Mnode, 63);
  Configure->Option[63] = '\0';
  strncpy(Configure->Name, driver, 63);
  Configure->Name[63] = '\0';
  if (RmApplyNetwork(Network, &configure_aux, Configure) ne number_affected)
   maperror(25);
#ifdef SYSDEB
  Configure->Node.Next = Configure->Node.Prev = &Configure->Node;
#endif
  AddTail(&(Network->Hardware), &(Configure->Node));
}

/**
*** And resolving the links. I build a table mapping all tags onto
*** processors, using a simply RmApply. This table has to be allocated
*** because the maximum tag is not known until the whole network has
*** been processed. Then another RmApply is used to add all the connections to
*** the network.
**/

static RmProcessor *ProcessorTab;
 
static int build_tags(RmProcessor, ...);
static int make_connections(RmProcessor, ...);

static void resolve_links(RmNetwork Subnet)
{
  ProcessorTab = Malloc((maxtag+1) * sizeof(RmProcessor));
  if (ProcessorTab eq Null(RmProcessor)) mapmemory(8);

  (void) RmApplyNetwork(Subnet, &build_tags);
  (void) RmApplyNetwork(Subnet, &make_connections);
  RmResolveLinks(Subnet);
}

static int build_tags(RmProcessor Processor,...)
{ int tag;

  if (RmIsNetwork(Processor))
   return(RmApplyNetwork((RmNetwork) Processor, &build_tags));
  tag = Processor->ObjNode.Key;
  ProcessorTab[tag] = Processor;
  return(0);
}

static int make_connections(RmProcessor Processor, ...)
{ int number_connections, i;
  old_connection *conn_ptr, conn;

  if (RmIsNetwork(Processor))
   return(RmApplyNetwork((RmNetwork) Processor, &make_connections));
       
  number_connections = Processor->ObjNode.Matrix;
  conn_ptr = (old_connection *) Processor->ObjNode.Dates.Modified;
  for (i = 0; i < number_connections; i++, conn_ptr++)
   { memcpy((void *) &conn, (void *) conn_ptr, sizeof(old_connection));
     if (conn.t_mode ne 0)
      { if (conn.t_mode eq 3)
         { if (RmMakeLink(Processor, i, RmM_ExternalProcessor, RmM_AnyLink) ne
         		  RmE_Success)
            maperror(28);
         }
        elif (conn.t_mode eq 2)
         { RmProcessor target; int target_tag;
           target_tag = conn.target_tag;
           target  = ProcessorTab[target_tag];
           if (RmMakeLink(Processor, i, target, RmM_AnyLink) ne RmE_Success)
            maperror(29);
         }
        else
         maperror(30);
      }
   }
  return(0);
}
#endif

/**
*** Resolving connections. The Set is guaranteed to be valid and writeable.
*** 1) in the case of networks or native taskforces, it is necessary to test
***    the number of connections between objects.
***    If too few have been specified, fill in the remainder to be not
***    connected.
*** 2) for all connections that have been specified completely, do a
***    validation
*** 3) for all connections about which information is scanty, see if there
***    is any more info to be had.
**/

static int RmResolve1(RmProcessor Processor, ...)
{ int ptype = RmGetProcessorType(Processor);
  int links;
  int sum = 0;

  if ((Processor->ObjNode.Type & Type_Flags) eq Type_Directory)
   return(RmApplyNetwork((RmNetwork) Processor, &RmResolve1));
     
 	/* First figure out how many connections this object should have */
  switch(ptype)
   { case RmT_T800  :
     case RmT_T414  :
     case RmT_T425  :
     case RmT_T212  :
     case RmT_T9000 :	/* probably */
#ifdef __TRAN
     default	    :
#endif
     			links = 4; break;
     case RmT_T400  :	links = 2; break;
     case RmT_C40   :
#ifdef __C40
     default	    :
#endif
			links = 6; break;
   }

  if ((RmGetProcessorPurpose(Processor) & RmP_Mask) eq RmP_IO)
   links = 1;

	/* If the object has too few connections, add some more.	*/
  while (links > Processor->Connections)
   {
     if (RmMakeLink(Processor, Processor->Connections, RmM_NoProcessor, 0)
	 ne RmE_Success)
      return(-100000);
     sum++;
   }
  
  return(sum);
}

static int RmResolve2(RmProcessor Processor, ...)
{ int count = RmCountLinks(Processor);
  int i;
  int sum = 0;

  if ((Processor->ObjNode.Type & Type_Flags) eq Type_Directory)
   return(RmApplyNetwork((RmNetwork) Processor, &RmResolve2));
   
	/* Examine every link, one by one. */    
  for (i = 0; i < count; i++)
   { RmLink *Link = RmFindLink(Processor, i);
     RmProcessor Target;
      
     	/* Unconnected links and external ones can be ignored, there */
     	/* is no verification to be done.			     */
     if ((Link->Target eq RmL_NoUid) || (Link->Target eq RmL_ExtUid))
      continue;
     
    	/* Check that the object at the other end of the connection */
    	/* really does exist. If not, break the connection.	    */
     Target = RmFindProcessor(Processor->Root, Link->Target);
     if (Target eq RmM_NoProcessor) goto error;

	/* This code should never get executed. It implies a funny UID */
     if (Target eq RmM_ExternalProcessor)
      { Link->Target = RmL_ExtUid;
        Link->Destination = 0;
        Link->Flags	 &= ~RmF_AnyLink;
        continue;
      }

	/* If the two objects are in different sets then there is something */
	/* very funny going on. However, there is little that can be done   */
	/* to recover.							    */
     if (Target->Root ne Processor->Root) goto error;

	/* If the connecting link on the target processor is known, it is */
	/* possible to check the other end immediately.			  */
     if (Link->Destination ne RmM_AnyLink)
      { RmLink *TargetLink = RmFindLink(Target, Link->Destination);

	/* Failing this test means that Destination is wrong. */
        if (TargetLink eq Null(RmLink)) goto error;
        
       	/* This is an inconsistency error. ProcA goes to ProcB but not */
       	/* vice versa */
        if (TargetLink->Target ne Processor->Uid) goto error;

	/* If the other object knows which link it is connected to, but this */
	/* object does not, the other object must be confused. */
        if (TargetLink->Flags & RmF_AnyLink)
         { Link->Destination = RmM_AnyLink;
           goto skip;
         }

       	/* If the other end does not know which link it is connected to but */
       	/* this end is happy, then the other end can be updated. */
        if (TargetLink->Destination eq RmM_AnyLink)
         { if ((Link->Flags & RmF_AnyLink) eq 0)
            { TargetLink->Destination = i;
              sum++;
            }
         }
        /* If the other end thinks it is connected to a different link than */
        /* this end, there is something wrong. */
        elif(TargetLink->Destination ne i)
         goto error;

        /* And I cannot think of any more tests. */
        continue;
      }
skip:         
	/* I know that link i of object Processor is connected to some link */
	/* of object Target, but not which link. */
     { int count = RmCountLinks(Target);
       int j;
       int possibles = 0;
       RmLink *TargetLink;
	/* Follow all the connections in Target, to see which one(s) match */
       for (j = 0; j < count; j++)
        { TargetLink = RmFindLink(Target, j);
          if (TargetLink->Target ne Processor->Uid) continue;
		/* There is a possibility that the other side knows */
		/* all the required information. */
          if (TargetLink->Destination eq i)
           { possibles = -100000;
             if (TargetLink->Flags & RmF_AnyLink)
              break;	/* no more information to be had */
             Link->Destination = j;
             sum++;
             break;
           }
           	/* This may be the right link, but it is only worth */
           	/* worrying if it is non-floating */
          if ((TargetLink->Destination eq RmM_AnyLink) &&
              ((TargetLink->Flags & RmF_AnyLink) eq 0))
           possibles++;
        }
        	/* If there is exactly one fixed link connected to this */
        	/* Processor then some information can be filled in.	*/
       if (possibles eq 1)
        { int count = RmCountLinks(Target);
          int j;
          RmLink *TargetLink;

          	/* Find it again */
          for (j = 0; j < count; j++)
           { TargetLink = RmFindLink(Target, j);
             if ((TargetLink->Target eq Processor->Uid) &&
                 (TargetLink->Destination eq RmM_AnyLink) &&
                 ((TargetLink->Flags & RmF_AnyLink) eq 0))
              { /* And fill it in */
                Link->Destination = j; sum++;
                if ((Link->Flags & RmF_AnyLink) eq 0)
                 { TargetLink->Destination = i; sum++; }
                break;
              }
           }
        }
     }
     continue;
error:
     Link->Destination = RmM_AnyLink;
     (void) RmBreakLink(Processor, i);
     sum++;
   }
  return(sum);
}

static void RmResolveLinks(RmNetwork Set)
{   
  if (Set->DirNode.Nentries <= 0) return;
  
  (void) RmApplyNetwork(Set, &RmResolve1);
  (void) RmApplyNetwork(Set, &RmResolve2);
}

