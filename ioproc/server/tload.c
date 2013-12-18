/*------------------------------------------------------------------------
--                                                                      --
--          H E L I O S   I N P U T / O U T P U T   S E R V E R         --
--          ---------------------------------------------------         --
--                                                                      --
--             Copyright (C) 1987, Perihelion Software Ltd.             --
--          Copyright (c) 1994, Perihelion Distributed Software.        --
--                        All Rights Reserved.                          --
--                                                                      --
--      tload.c                                                         --
--                                                                      --
--              This module is responsible for bootstrapping the        --
--                                                                      --
--      root processor on start-up, and interacting with the            --
--                                                                      --
--      processor network.                                              --
--                                                                      --
--  Author:  BLV 8/10/87                                                --
--                                                                      --
------------------------------------------------------------------------*/
/* RcsId: $Id: tload.c,v 1.49 1994/07/15 11:05:19 nickc Exp $ */
/* Copyright (C) 1987, Perihelion Software Ltd. 			*/

#define Tload_Module

#include "helios.h"

#include <stdlib.h>
#include <sys\types.h>
#include <sys\stat.h>

/*{{{  description */
/**
*** The bits below are pretty horrible. The code for booting up the processor
*** used to be a separate program tload, but it is necessary for the server
*** and the debugger to be able to do this themselves. Hence the code from
*** tload.c was inserted here and fiddled about to make it work. Things become
*** very nasty because the Server and the debugger have rather different
*** requirements.
**/
/*}}}*/
/*{{{  statics and prototypes */

PRIVATE int bootstrap_mode;
PRIVATE word processor_memory;
PRIVATE char *box_type;

#define MAXREAD  0xffffL
#if (UNIX)
#define MAXCHUNK 0x20000L
#define C40MAXCHUNK MAXCHUNK
#else
#define C40MAXCHUNK 30720 	/* try to stop mem probs - 30k */
				/*  else revert to 0xD000L = ~50k */
#define MAXCHUNK 0x1000L
#endif

PRIVATE List  system_list;
List  *Image_List = (List *) NULL;
word  isize;

FILE  *infd;

PRIVATE void fn( timeout,       (char *));
PRIVATE void fn( send_boot,     (void));
PRIVATE void fn( send_idrom,    (void));
PRIVATE void fn( send_nucleus,  (void));
PRIVATE void fn( send_conf,     (void));
PRIVATE void fn( handle_info_request, (void));
PRIVATE int  fn( send_info_request, (void));
PRIVATE int  fn( send_resync, (void));
PRIVATE void fn( server_helios, (void));
PRIVATE int  fn( test_processor, (void));
PRIVATE void fn( load_boot,      (void));
/*}}}*/
/*{{{  init_boot() */
/**
*** This code deals with selecting the appropriate parts of the bootstrap,
*** depending on the processor types, and invoking them in turn.
**/
int init_boot()
{ char *conf;

	/* The default processor type is transputer			*/
  target_processor = Processor_Trannie;
  bootstrap_mode   = B_Reset_Processor | B_Send_Bootstrap | B_Send_Image |
                      B_Send_Config | B_Wait_Sync | B_Check_Processor;

  processor_memory = get_int_config("transputer_memory");
  if (processor_memory eq Invalid_config)
   processor_memory = get_int_config("processor_memory");

	/* The particular board type can affect the configuration	*/
	/* vector and ID rom.						*/
  box_type = get_config("box");

  conf = get_config("target_processor");
  if (conf ne (char *) NULL)
  { if (!mystrcmp(conf, "T414") ||
        !mystrcmp(conf, "T800") ||
        !mystrcmp(conf, "T425") ||
        !mystrcmp(conf, "T400"))
     target_processor = Processor_Trannie;
    elif (!mystrcmp(conf, "ARM"))
     { target_processor = Processor_Arm;
       /* R140 Ram based system defaults */
       bootstrap_mode = B_Reset_Processor | B_Send_Image | B_Send_Config |
         B_Wait_Sync;
     }
    elif (!mystrcmp(conf, "I860"))
     target_processor = Processor_i860;
    elif (!mystrcmp(conf, "68000"))
     target_processor = Processor_68000;
    elif ((!mystrcmp(conf, "C40")) || (!mystrcmp(conf, "320C40")) ||
	  (!mystrcmp(conf, "TMS320C40")))
     { target_processor = Processor_C40;
       bootstrap_mode = B_Reset_Processor | B_Send_Bootstrap | B_Send_IdRom |
		B_Send_Image | B_Send_Config | B_Wait_Sync;
     }
    else
     { ServerDebug("Unknown processor type : %s", conf);
       ServerDebug(
    "Supported processors are : T414, T800, T425, T400, ARM, i860, 68000 and TMS320C40.");
/*       longjmp(exit_jmpbuf, 1); */
       longjmp_exit;
     }
  }

	/* Primarily for debugging purposes, parts of the bootstrap	*/
	/* can be suppressed.						*/
  if (get_config("no_reset_target") ne (char *) NULL)
   bootstrap_mode &= ~B_Reset_Processor;          
  if (get_config("no_bootstrap") ne (char *) NULL)
   bootstrap_mode &= ~B_Send_Bootstrap;
  if (get_config("no_image") ne (char *) NULL)
   bootstrap_mode &= ~B_Send_Image;   
  if (get_config("no_config") ne (char *) NULL)
   bootstrap_mode &= ~B_Send_Config;
  if (get_config("no_sync") ne (char *) NULL)
   bootstrap_mode &= ~B_Wait_Sync;
  if (get_config("no_idrom"))
   bootstrap_mode &= ~B_Send_IdRom;
  if (get_config("attach") ne (char *) NULL)
  {  bootstrap_mode &= ~B_Wait_Sync;
     bootstrap_mode |= B_Send_Sync;
  }
  if (get_config("no_check_processor") ne (char *) NULL)
   bootstrap_mode &= ~B_Check_Processor;

  if ((get_config("just_attach") ne (char *) NULL) ||
      (get_config("enable_link") ne (char *) NULL) ||
       EnableThatLink )
  {
   /* do not attempt to reset or boot system */
   /* allows IO processor to attach to running system */

   /* bootstrap_mode &= ~B_Reset_Processor; */
   /* bootstrap_mode &= ~B_Send_Bootstrap;  */
   /* bootstrap_mode &= ~B_Send_Image;      */
   /* bootstrap_mode &= ~B_Send_Config;	    */
   /* bootstrap_mode &= ~B_Wait_Sync;       */
   /* bootstrap_mode &= ~B_Check_Processor; */
   /* bootstrap_mode |= B_Send_Sync;	    */
   /* The above is equivalent to: 	    */
   
   bootstrap_mode = B_Send_Sync;

   /* Confusion here between ROM based systems you want to RESET before
    * attaching to, and IO Server as a fileserver/console type system where you
    * dont. Also in the case of some inmos boards where the processor reset
    * is actually shared with the link chipreset, the procreset line has to be
    * cut and the "reset_target" directive used with "just attach".
    * (The just_attach directive defaults to (no_reset_target").
    */

   if (get_config("reset_target") ne NULL)
	bootstrap_mode |= B_Reset_Processor;
  }

  	/* The half duplex protocol is on by default for C40 and ARM, unless	*/
  	/* c40_disable_halfduplex is specified.					*/
  if (((target_processor eq Processor_C40) || (target_processor eq Processor_Arm)) &&
       (get_config("c40_disable_halfduplex") == NULL))
   C40HalfDuplex = TRUE;

	/* For transputers the default bootlink is 0. For C40s it is  */
	/* normally 3, unless a special link interface is needed in   */
        /* which case the bootlink is the seventh.                    */
  if (target_processor eq Processor_C40)
   { Default_BootLink = 3;
     if ((!mystrcmp(box_type, "DSP1")) || (!mystrcmp(box_type, "HYDRA")))
      Default_BootLink = 6;
   }

  InitList(&system_list);
  Image_List = &system_list;

  if (bootstrap_mode & B_Send_Bootstrap)
    {
      load_boot();
    }

  if (bootstrap_mode & B_Check_Processor)
  {
    return (test_processor ());
  }

  return(1);
}
/*}}}*/
/*{{{  tidy_boot() */

void tidy_boot()
{
  if (bootstrap ne NULL)
  { iofree(bootstrap); bootstrap = NULL; }

  if ((Image_List ne (List *) NULL) && (TstList(Image_List)) )
    FreeList(Image_List);
}
/*}}}*/
/*{{{  test_processor() */
/**
*** This routine checks that the processor really is available
**/
PRIVATE int  test_processor()
{
  if (target_processor eq Processor_Trannie)
   { int  retries;
     word start_time = get_unix_time();

#if SOLARIS
     for (retries = 0; retries < 10; retries++)
#else
     for (retries = 0; (retries < 3) && ((get_unix_time() - start_time) < 5L);
          retries++)
#endif
      { 
	resetlnk();
	xpreset();
	unless(xpwrbyte(0L))          continue;
        unless(xpwrint(0x80000100L))  continue;
        unless(xpwrint(0x12345678L))  continue;
        unless(xpwrbyte(1L))          continue;
        unless(xpwrint(0x80000100L))  continue;
        if (xprdint() ne 0x12345678L) continue; 
	   /* And again for good measure */
        unless(xpwrbyte(0L))          continue;
        unless(xpwrint(0x80000104L))  continue;
        unless(xpwrint(0x87654321L))  continue;
        unless(xpwrbyte(1L))          continue;
        unless(xpwrint(0x80000104L))  continue;
        if (xprdint() ne 0x87654321L) continue;

           /* Two successful exchanges is good enough  */
        return(1);
      }

     return(0);
   }

  return 1;
}
/*}}}*/
/*{{{  boot_processor */
/**
*** When the Server or the debugger wants to boot the processor it calls
*** this routine in either mode debugboot or serverboot. The routine loads
*** the bootstrap code and the system image, and sends them off to the
*** processor. Then the space take up by the bootstrap code is freed, and
*** if the system is in Server mode the space taken up by the system image
*** is freed as well because this lets me have a couple more coroutines before
*** running out of memory, and that is rather important on a 640K PC.
**/

#if SOLARIS

static jmp_buf * failure_buf;	/* memory assigned in boot_processor () */
#define longjmp_fail	longjmp (*failure_buf, 1)

#else

static jmp_buf failure_buf;
#define longjmp_fail	longjmp (failure_buf, 1)

#endif

/*{{{  timeout() */
/**
*** Timeout() is called when any of the boot writes to the processor fails.
*** It longjmps back to boot_processor() above, which either recovers or
*** exits depending on what the user is doing.
**/
PRIVATE void timeout(when)
char *when;
{ ServerDebug("Timed out when sending %s.", when);
/*  longjmp(failure_buf, 1); */
  longjmp_fail;
}
/*}}}*/

void boot_processor(mode)
int mode;
{
#if SOLARIS
  failure_buf = (jmp_buf *)(malloc (256));

  if (setjmp (*failure_buf) ne 0) goto failed;
#else
  if (setjmp(failure_buf) ne 0) goto failed;
#endif

  infd = (FILE *)NULL;

  if ( (bootstrap_mode & B_Send_Image) && (!loadimage()) )
  {
    goto failed;
  }

  send_boot();

  if (bootstrap_mode & B_Send_IdRom)   send_idrom();
  if (bootstrap_mode & B_Send_Image)   send_nucleus();
  if (bootstrap_mode & B_Send_Config)  send_conf();

  if ((mode ne debugboot) && TstList(Image_List))
   FreeList(Image_List);

#if multi_tasking
#if SOLARIS
    /* last 0 in call is a dummy value to keep the C++ compiler happy */
    AddMultiwait(Multi_LinkMessage, &link_table[current_link].ready,
                 current_link, 0);
#else
    AddMultiwait(Multi_LinkMessage, &link_table[current_link].ready,
                 current_link);
#endif
    link_table[current_link].ready = 0L;
#endif

  if (mode ne debugboot)
   server_helios();

#if (UNIX && !MEIKORTE)
  { extern void fn( Hydra_SwitchMode, (void));
    if (Server_Mode eq Mode_Remote)
     Hydra_SwitchMode();
  }
#endif

  return;

failed:                      /* to bootstrap the processor for some reason */
  if (mode eq debugboot)
   return;                   /* continue debugging */
  else
   output("Processor bootstrap has failed.\r\n");
}
/*}}}*/
/*{{{  disk I/O */
/**
*** The following bits of code deal with loading items off disk. The file name
*** corresponding to the system image is known because the system cannot
*** start up without it - either it is in the configuration file or it is on
*** the command line. Also, the system image may be loaded already if in
*** debugging mode so I check for that. The system image is loaded in chunks
*** because some systems object to loading vast quantities of data in one go.
*** On some processors the system image may be too large to hold in memory.
***
*** loadboot() is called once only, from main() in module server.c . A bootfile
*** may be specified in the configuration file, with the default being nboot.i
*** The space for the bootstrap code is freed at the end of main().
**/
 
int loadimage()
{ word l;
  int  cur;
  GenData *cur_data;
  struct stat sbuf;

  /* If the nucleus is already loaded or the processor is not a transputer */
  /* wait for send_boot() to load and send system image in small chunks */
  /* Most processors nuclei can get too large to load in one go. */
  if (target_processor ne Processor_Trannie || TstList(Image_List))
	return(1);

  if (system_image[0] eq '\0')
   { ServerDebug("No system image defined in host.con file.");
/*     longjmp(exit_jmpbuf, 1); */
       longjmp_exit;
   }

  Debug(Boot_Flag, ("Loading system image %s.", system_image) );

#if (UNIX)
  infd = fopen(system_image,"r");
#else
  infd = fopen(system_image,"rb");
#endif
  if( infd eq (FILE *) NULL ) 
   { ServerDebug("Cannot open image %s for input.", system_image);
     return(0);
   }

#if 0	/* Old style of finding size of nucleus image. */
	/* This requires a standardised format for the nucleus */
	/* header. This can no longer be relied on in the V2 world. */
	
  if (fread((byte *) &isize,1,4,infd) ne 4)
   { ServerDebug("Cannot read image header.");
     goto done;
   }
  isize = swap(isize);                   /* extract size of system image */

  if (fseek(infd, 0L, SEEK_SET) ne 0)    /* and reset file to beginning */
   { ServerDebug("Failed to seek in image.");
     goto done;
   }
#else
	/* New style. */
  if (stat(system_image, &sbuf) ne 0)
   { ServerDebug("stat failed on %s.", system_image);
     goto done;
   }

  isize = sbuf.st_size;

  ServerDebug("Image size returned from stat = %d.",isize);

  if (isize <= 0)
   { ServerDebug("Bad image size returned from stat.");
     goto done;
   }
#endif

  for (l = 0L; l < isize; l += MAXCHUNK)
   { cur = ((isize - l) > MAXCHUNK) ? (int) MAXCHUNK : (int) (isize - l);
     cur_data = (GenData *) malloc(sizeof(GenData) + cur);
     if (cur_data eq (GenData *) NULL)
      { ServerDebug("Insufficient memory to store system image.");
        goto done;
      }
     if (fread(&(cur_data->data[0]), 1, cur, infd) eq 0)
      { ServerDebug("Failed to read system image.");
        goto done;
      }
     cur_data->size = cur;
     AddTail(&(cur_data->node), Image_List);
   }

  fclose( infd );                           /* finished successfully */
  infd = (FILE *) NULL;
  return(1);

done:
  fclose( infd );
  infd = (FILE *) NULL;
  FreeList(Image_List);
  return(0);  /* return error */
}

#if SUN3
extern int	VC40_Type;

extern int	vc40_settype ();
extern void	vc40_enint ();
#endif

PRIVATE void load_boot()
{ int s;
  word ihdr[3];
  char *name = get_config("bootfile");    /* get boot name from configuration */

  if (name eq (char *) NULL)              /* use default bootfile */     
   { if (target_processor eq Processor_C40)
      { 	/* Some C40 systems need special versions of the bootstrap */
        if (!mystrcmp(box_type, "dsp1"))
         name = "c40hboot.i";
#if SUN3
        else if( !mystrcmp(box_type, "VC40"))
	  {
	    /* HYDRA I or HYDRA II ??? */
	    if (VC40_Type == 0)	VC40_Type = vc40_settype ();

	    if (VC40_Type == 1)		name = "c40ah1boot.i";
	    else if (VC40_Type == 2) 	name = "c40ah2boot.i";
	  }
#endif
        else if( !mystrcmp(box_type, "SPIRIT40"))
         name = "c40sboot.i";
        else
         name = "c40boot.i";
      }
     else
      name = "nboot.i";
   }

   if( !mystrcmp(box_type, "VC40") || !mystrcmp(box_type, "SPIRIT40") )
   {
        	
        	/* For shared memory boards, the bootstrap is actually sent	*/
        	/* when the processor is reset. We need B_Send_Bootstrap	*/
        	/* to cause load_boot to be called, but not send_boot.		*/
        	bootstrap_mode &= ~B_Send_Bootstrap;
   }

  Debug(Boot_Flag, ("Loading bootstrap program %s", name) );

#if (UNIX)
  infd = fopen(name,"r");
#else
  infd = fopen(name,"rb");
#endif
  if( infd eq (FILE *) NULL ) 
   { ServerDebug("Cannot open boot program %s for input.",name);
     goto fail;
   }

  if( (s=fread((byte *) &(ihdr[0]),1,12,infd)) ne 12)
   { ServerDebug("Cannot read boot header %d %d.",s,ferror(infd));
     goto fail;
   }

   {
	/* Check bootstrap's image magic */
	word magic = 0x12345678L; /* default to transputer */

	if (target_processor eq Processor_C40)
		magic = 0xc4045601L;
	else if (target_processor eq Processor_Arm)
		magic = 0x0a245601L;
	else if (target_processor eq Processor_i860)
		magic = 0x86045601L;
	
	if( swap(ihdr[0]) ne magic) {
		ServerDebug("Error: bootstrap is not an executable Helios file.");
		goto fail;
	}
   }

  bootsize = swap(ihdr[2]);

  bootstrap =  (char *)(malloc((uint) bootsize));
  if( bootstrap eq NULL )
   { ServerDebug("Cannot get image buffer."); goto fail; }

  if( (s=fread(bootstrap,1,(int)bootsize,infd)) ne (int) bootsize)
   { ServerDebug("Image too small %d, error %d.",s,ferror(infd)); goto fail; }

  fclose( infd );
  infd = (FILE *) NULL; 
  return;

fail:
  if (infd ne (FILE *) NULL)
   { fclose( infd );
     infd = (FILE *) NULL;
   }
/*  longjmp(exit_jmpbuf, 1); */
  longjmp_exit;
}
/*}}}*/
/*{{{  send bootstrap */
PRIVATE void send_boot()
{ char *temp;

  Debug(Boot_Flag, ("Resetting link and processor.") );
  resetlnk();

   /* Confusion here between ROM based systems you want to RESET before
    * attaching to, and IO Server as a fileserver/console type system where you
    * dont. Also in the case of some inmos boards where the processor reset
    * is actually shared with the link chipreset, the procreset line has to be
    * cut and the "reset_target" directive used with "just attach".
    * (The just_attach directive defaults to no_reset_target").
    */
  if (bootstrap_mode & B_Reset_Processor)
   {
#if TR5
     poll(0,0,1000);
#endif
     xpreset();
   }
   
  if (bootstrap_mode & B_Send_Bootstrap)
   { Debug(Boot_Flag, ("Sending bootstrap, size is %ld", bootsize) );

     if (target_processor eq Processor_C40) 
/*{{{  C40 bootstrap */
      {
      /*
		Raw 'C40 bootstrap consists of:
			Global Bus memory control word:		0x3e39fff0
			Local Bus memory constrol word:		0x3e39fff0
			(can be any value bootstrap will use IDROM values)
			Block size of bootstrap to be loaded
			Address where boostrap is to be loaded

			The bootstrap...

			End of blocks word (0x0000 0000)
			Interrupt vector pointer
			Trap vector pointer
			Local Memory address for dummy IACK instruction
	*/

        if (!xpwrint(0x3e39fff0))		 /* global bus cntrl */
         timeout("global bus ctrl (first word of bootstrap)");
        if (!xpwrint(0x3e39fff0))		 /* local bus cntrl */
         timeout("local bus ctrl (second word of bootstrap)");

        if (!xpwrint(bootsize / 4L))       /* bootstrap size */
         timeout("bootstrap size");
        if (!xpwrint(0x002ffc00))          /* load address (internal ram blk 1) */
         timeout("load address");
        if (!xpwrdata(bootstrap,bootsize)) /* bootstrap */
         timeout("bootstrap code");
        if (!xpwrint(0x0)) 	         /* end of blocks to load */
         timeout("end block marker");
        if (!xpwrint(0x0)) 	         /* IVTP address */
         timeout("IVTP address");
        if (!xpwrint(0x0)) 	         /* TVTP address */
         timeout("TVTP address");
        if (!xpwrint(0x00300000)) 	 /* dummy IACK address */
         timeout("IACK address");	 /* start of local bus */

 	/* 'C40 should now be executing bootstrap */
	if (xprdint() != 1) {
	 ServerDebug("Failed to receive bootstrap acknowledge");
/*	 longjmp(failure_buf, 1); */
	 longjmp_fail;
	}

      }
/*}}}*/
     elif (target_processor eq Processor_Trannie)
/*{{{  transputer bootstrap */
      {
	if (!xpwrbyte(bootsize))              /* bootstrap size */
         timeout("bootstrap size");
        if (!xpwrdata(bootstrap,bootsize))    /* bootstrap */
         timeout("bootstrap code");

        temp = get_config("BOX");          /* This entry is known to exist */
        if (!mystrcmp(temp, "MK026"))      /* or anything else with parity memory */
         { word size = processor_memory;
           if (size eq Invalid_config) size = 0x300000L;
           xpwrbyte(5L);
           xpwrint(size);
           while (!xprdrdy());           /* wait for the clear to finish */
           (void) xprdbyte();
         }
        xpwrbyte(4L);
      }	/* end of transputer version */
/*}}}*/
   }
}
/*}}}*/
/*{{{  send ID Rom */
static void send_idrom()
{ 
  if (target_processor eq Processor_C40)
   {   	word C40_HW_Config = 0;
		/* set defaults for pseudo IDROM */
	static IDROM ID_ROM = {
					/* self inclusive size of this block */
			sizeof(IDROM) / sizeof(word),
			0,		/* TIM-40 module manufacturers ID */
			0,		/* CPU type (00 = C40) */
			49,		/* CPU cycle time (49 = 50ns = 40Mhz) */
			0,		/* manufactures module type */
			0,		/* module revision level */
			0,		/* reserved byte */

			0x80000000,	/* address base of global bus strobe 0 */
	/* none */	0xffffffff,	/* address base of global bus strobe 1 */
			0x300000,	/* address base of local bus strobe 0 */
	/* none */	0xffffffff,	/* address base of local bus strobe 1 */

					/* sizes are in words */
	/* 4Mb */	0x100000,	/* size of memory on global bus strobe 0 */
			0,		/* size of memory on global bus strobe 1 */
	/* 4Mb */	0x100000,	/* size of memory on local bus strobe 0 */
			0,		/* size of memory on local bus strobe 1 */
			0x800,		/* size of fast ram pool (inc. on-chip RAM) */

			/* assume fast DRAM here */
			0x22,		/* within page on global bus */
			0x22,		/* within page on local bus */
			0x55,		/* outside page on global bus */
			0x55,		/* outside page on local bus */

			/* 1ms on 40Mhz C40 */
			0x2710,		/* period time for 1ms interval on timer 0 */

			/* no refresh required */
			0x80,		/* period for DRAM refresh timer (optional) */
			0x2c2,		/* contents set TCLK0 to access RAM not IDROM */

			0,		/* sets up timer to refresh DRAM (optional) */

/* same as C40 reset */	0x3e39fff0,	/* global bus control register */
/* same as C40 reset */	0x3e39fff0,	/* local bus control register */

	/* none */	0		/* total size of auto-initialisation data */
		};


	/* Send a hardware configuration word to the bootstrap. */
	/* This allows it to configure itself for different C40 */
	/* hardware environments and nucleus load positions. */
	
	if (get_config("c40_disable_cache") != NULL)
	{
		C40_HW_Config |= HW_CacheOff;
	}

	if (get_config("c40_load_nucleus_local_S0") != NULL)
	{
		C40_HW_Config |= HW_NucleusLocalS0;
	}
	if (get_config("c40_load_nucleus_local_S1") != NULL)
	{
		C40_HW_Config |= HW_NucleusLocalS1;
	}
	if (get_config("c40_load_nucleus_global_S0") != NULL)
	{
		C40_HW_Config |= HW_NucleusGlobalS0;
	}
	if (get_config("c40_load_nucleus_global_S1") != NULL)
	{
		C40_HW_Config |= HW_NucleusGlobalS1;
	}

	if ((!mystrcmp(box_type, "DSP1")) ||
	    (!mystrcmp(box_type, "VC40")) ||
	    (!mystrcmp(box_type, "SPIRIT40")) ||
            (get_config("c40_replace_idrom") != NULL) ||
	    (get_config("c40_use_pseudo_idrom") != NULL)) {
		word i;

		/* now let the users overload any part of the IDROM */
		if ((i = get_int_config("c40_idrom_man_id")) != Invalid_config)
		{
			ID_ROM.MAN_ID = (short)i;
		}

		if ((i = get_int_config("c40_idrom_cpu_id")) != Invalid_config)
		{
			ID_ROM.CPU_ID = (byte)i;
		}

		if ((i = get_int_config("c40_idrom_cpu_clk")) != Invalid_config)
		{
			ID_ROM.CPU_CLK = (byte)i;
		}

		if ((i = get_int_config("c40_idrom_model_no")) != Invalid_config)
		{
			ID_ROM.MODEL_NO = (short)i;
		}
		if ((i = get_int_config("c40_idrom_rev_lvl")) != Invalid_config)
		{
			ID_ROM.REV_LVL = (byte)i;
		}
		if ((i = get_int_config("c40_idrom_reserved")) != Invalid_config)
		{
			ID_ROM.RESERVED = (byte)i;
		}

		if ((i = get_int_config("c40_idrom_gbase0")) != Invalid_config)
		{
			ID_ROM.GBASE0 = (word)i;
		}
		if ((i = get_int_config("c40_idrom_gbase1")) != Invalid_config)
		{
			ID_ROM.GBASE1 = (word)i;
		}
		if ((i = get_int_config("c40_idrom_lbase0")) != Invalid_config)
		{
			ID_ROM.LBASE0 = (word)i;
		}
		if ((i = get_int_config("c40_idrom_lbase1")) != Invalid_config)
		{
			ID_ROM.LBASE1 = (word)i;
		}

		if ((i = get_int_config("c40_idrom_gsize0")) != Invalid_config)
		{
			ID_ROM.GSIZE0 = (word)i;
		}
		if ((i = get_int_config("c40_idrom_gsize1")) != Invalid_config)
		{
			ID_ROM.GSIZE1 = (word)i;
		}
		if ((i = get_int_config("c40_idrom_lsize0")) != Invalid_config)
		{
			ID_ROM.LSIZE0 = (word)i;
		}
		if ((i = get_int_config("c40_idrom_lsize1")) != Invalid_config)
		{
			ID_ROM.LSIZE1 = (word)i;
		}

		if ((i = get_int_config("c40_idrom_fsize")) != Invalid_config)
		{
			ID_ROM.FSIZE = (word)i;
		}

		if ((i = get_int_config("c40_idrom_wait_g0")) != Invalid_config)
		{
			ID_ROM.WAIT_G = (ID_ROM.WAIT_G & 0xf0) | (byte)i;
		}
		if ((i = get_int_config("c40_idrom_wait_g1")) != Invalid_config)
		{
			ID_ROM.WAIT_G = (ID_ROM.WAIT_G & 0x0f) | (byte)i << 4;
		}

		if ((i = get_int_config("c40_idrom_wait_l0")) != Invalid_config)
		{
			ID_ROM.WAIT_L = (ID_ROM.WAIT_L & 0xf0) | (byte)i;
		}
		if ((i = get_int_config("c40_idrom_wait_l1")) != Invalid_config)
		{
			ID_ROM.WAIT_L = (ID_ROM.WAIT_L & 0x0f) | (byte)i << 4;
		}

		if ((i = get_int_config("c40_idrom_pwait_g0")) != Invalid_config)
		{
			ID_ROM.PWAIT_G = (ID_ROM.PWAIT_G & 0xf0) | (byte)i;
		}
		if ((i = get_int_config("c40_idrom_pwait_g1")) != Invalid_config)
		{
			ID_ROM.PWAIT_G = (ID_ROM.PWAIT_G & 0x0f) | (byte)i << 4;
		}

		if ((i = get_int_config("c40_idrom_pwait_l0")) != Invalid_config)
		{
			ID_ROM.PWAIT_L = (ID_ROM.PWAIT_L & 0xf0) | (byte)i;
		}
		if ((i = get_int_config("c40_idrom_pwait_l1")) != Invalid_config)
		{
			ID_ROM.PWAIT_L = (ID_ROM.PWAIT_L & 0x0f) | (byte)i << 4;
		}

		if ((i = get_int_config("c40_idrom_timer0_period")) != Invalid_config)
		{
			ID_ROM.TIMER0_PERIOD = (word)i;
		}
		if ((i = get_int_config("c40_idrom_timer1_period")) != Invalid_config)
		{
			ID_ROM.TIMER1_PERIOD = (word)i;
		}
		if ((i = get_int_config("c40_idrom_timer0_ctrl")) != Invalid_config)
		{
			ID_ROM.TIMER0_CTRL = (short)i;
		}
		if ((i = get_int_config("c40_idrom_timer1_ctrl")) != Invalid_config)
		{
			ID_ROM.TIMER1_CTRL = (short)i;
		}

		if ((i = get_int_config("c40_idrom_gbcr")) != Invalid_config)
		{
			ID_ROM.GBCR = (word)i;
		}
		if ((i = get_int_config("c40_idrom_lbcr")) != Invalid_config)
		{
			ID_ROM.LBCR = (word)i;
		}

		if (get_config("c40_use_pseudo_idrom") != NULL)
		{
			C40_HW_Config |= HW_PseudoIDROM;
		}

	 	if (get_config("c40_replace_idrom") != NULL)
		{
			C40_HW_Config |= HW_ReplaceIDROM;
		}

		/* For SML type links, adjust the size of the appropriate */
		/* global strobe by the size of the shared RAM area	*/

		/* ... Except for Ariel's HYDRA II board, in which case the */
		/* SML link always extends upwards from 0x80000000	    */
		if( mystrcmp(box_type, "VC40") == 0 ||
		    mystrcmp(box_type, "SPIRIT40") == 0 )
		{
#if SUN3
			if (mystrcmp (box_type, "VC40") == 0)
			{
				if (VC40_Type == -1)	VC40_Type = vc40_settype ();
			}
#endif

			if (    mystrcmp (box_type, "SPIRIT40") == 0 
#if SUN3
			     || VC40_Type != 2
#endif
			   )
			{
				word sramsize = get_int_config("c40_sml_size");
				switch( sramsize )
				{
				default:
					ServerDebug("Invalid size for shared RAM, 8k assumed");
				case Invalid_config:
					sramsize = 8;
				case 8:
				case 16:
				case 32:
				case 64:
					sramsize *= 256;	/* size in WORDS */
				}
			
				if( get_config("c40_sml_g1") )
				{
					ID_ROM.GSIZE1 -= sramsize;
				}
				else
				{
					ID_ROM.GSIZE0 -= sramsize;
				}
			}
		}

		if( mystrcmp(box_type, "VC40") == 0 )
		{
			/* For the VC40 we have to swap the whole ROM.	*/
			/* Because the structure contains word,short and*/
			/* byte fields, we have to swap each field	*/
			/* individually (*SIGH*).			*/

#define swapword(x) swap(x)
#define swapshort(x) (swap(x)>>16)
#define swapbyte(x) (x)
			ID_ROM.SIZE 		= swapword(ID_ROM.SIZE);
			ID_ROM.MAN_ID 		= swapshort(ID_ROM.MAN_ID);
			ID_ROM.CPU_ID 		= swapbyte(ID_ROM.CPU_ID);
			ID_ROM.CPU_CLK 		= swapbyte(ID_ROM.CPU_CLK);
			ID_ROM.MODEL_NO 	= swapshort(ID_ROM.MODEL_NO);
			ID_ROM.REV_LVL 		= swapbyte(ID_ROM.REV_LVL);
			ID_ROM.RESERVED 	= swapbyte(ID_ROM.RESERVED);
			ID_ROM.GBASE0 		= swapword(ID_ROM.GBASE0);
			ID_ROM.GBASE1 		= swapword(ID_ROM.GBASE1);
			ID_ROM.LBASE0 		= swapword(ID_ROM.LBASE0);
			ID_ROM.LBASE1 		= swapword(ID_ROM.LBASE1);
			ID_ROM.GSIZE0 		= swapword(ID_ROM.GSIZE0);
			ID_ROM.GSIZE1 		= swapword(ID_ROM.GSIZE1);
			ID_ROM.LSIZE0 		= swapword(ID_ROM.LSIZE0);
			ID_ROM.LSIZE1 		= swapword(ID_ROM.LSIZE1);
			ID_ROM.FSIZE 		= swapword(ID_ROM.FSIZE);
			ID_ROM.WAIT_G 		= swapbyte(ID_ROM.WAIT_G);
			ID_ROM.WAIT_L 		= swapbyte(ID_ROM.WAIT_L);
			ID_ROM.PWAIT_G 		= swapbyte(ID_ROM.PWAIT_G);
			ID_ROM.PWAIT_L 		= swapbyte(ID_ROM.PWAIT_L);
			ID_ROM.TIMER0_PERIOD 	= swapword(ID_ROM.TIMER0_PERIOD);
			ID_ROM.TIMER1_PERIOD 	= swapword(ID_ROM.TIMER1_PERIOD);
			ID_ROM.TIMER0_CTRL 	= swapshort(ID_ROM.TIMER0_CTRL);
			ID_ROM.TIMER1_CTRL 	= swapshort(ID_ROM.TIMER1_CTRL);
			ID_ROM.GBCR 		= swapword(ID_ROM.GBCR);
			ID_ROM.LBCR 		= swapword(ID_ROM.LBCR);
			ID_ROM.AINIT_SIZE 	= swapword(ID_ROM.AINIT_SIZE);
		} 
			
			
		if (!xpwrint(C40_HW_Config))      /* Hardware Config word */
			timeout("Initial communication with bootstrap");

		/* send Pseudo IDROM */
		if (!xpwrdata((char *)&ID_ROM, sizeof(IDROM)))
			timeout("pseudo IDROM");
	} else {
		if (!xpwrint(C40_HW_Config))      /* Hardware Config word */
			timeout("Initial communication with bootstrap");
	}
   } /* C40 */
}
/*}}}*/
/*{{{  send nucleus */
/*{{{  send_bit() */
PRIVATE word send_bit(data)
GenData *data;
{
  poll_the_devices();
  if (Special_Exit) 
    {
/*      longjmp(exit_jmpbuf, 1); */
      longjmp_exit;
    }

  Debug(Boot_Flag, ("Sending %d bytes at %p.", data->size, &(data->data[0])));

  if (!xpwrdata(&(data->data[0]), (word) data->size))
   return(1L);
  else
   return(0L);
}
/*}}}*/

static void send_nucleus()
{
/*  printf ("@send_nucleus ()\n"); */

  if (!TstList(Image_List))
   {      /* image is too big to load in one go, so load bit, send bit, load ... */
        word l;
        int  cur;
        char *cur_data;
	word armnucbase, kernelstartaddress;
	word MaxChunk = C40MAXCHUNK;
	struct stat sbuf;

/*	printf ("send_nucleus () - send nucleus in chunks\n"); */

/*	printf ("send_nucleus () - box_type = %s\n", box_type); */

        if ((target_processor eq Processor_Arm) && (mystrcmp(box_type, "vy86pid") == 0))
	  {
	    MaxChunk = 1024 * 4;
	  }

#if SUN3
	if ((target_processor eq Processor_C40) && (mystrcmp (box_type, "VC40") == 0))
	{
		if (VC40_Type == -1)	VC40_Type = vc40_settype ();

		if (VC40_Type == 2)
		{
			MaxChunk = 1024 * 4;
		}
	}
#endif

        if (system_image[0] eq '\0')
	  { ServerDebug("No system image defined in host.con file.");
	    /*           longjmp(exit_jmpbuf, 1); */
	    longjmp_exit;
	  }

	    /* allocate mem for chunk to be read in and sent */
        if ((cur_data = (char *)malloc((uint)MaxChunk)) eq NULL)
	  { ServerDebug("Insufficient memory to store system image.");
	    /*           longjmp(exit_jmpbuf, 1); */
	    longjmp_exit;
	  }

#if (UNIX)
        infd = fopen(system_image,"r");
#else
        infd = fopen(system_image,"rb");
#endif
        if( infd eq (FILE *) NULL )  
	  { ServerDebug("Cannot open image %s for input.", system_image);
	    /*           longjmp(exit_jmpbuf, 1); */
	    longjmp_exit;
	  }

#if 0	/* Old style of finding size of nucleus image. */
	/* This requires a standardised format for the nucleus */
	/* header. This can no longer be relied on in the V2 world. */

        if (fread((byte *) &isize,1,4,infd) ne 4) 
	  { ServerDebug("Cannot read image header.");
	    /*           longjmp(exit_jmpbuf, 1); */
	    longjmp_exit;
	  }

        isize = swap(isize);                   /* extract size of system image */

        if (fseek(infd, 0L, SEEK_SET) ne 0)
	  {  /* and reset file to beginning */
	    ServerDebug("Failed to seek image.");
	    /*           longjmp(exit_jmpbuf, 1); */
	    longjmp_exit;
	  }
#else
	/* New style. */
  if (stat(system_image, &sbuf) ne 0)
   { ServerDebug("stat failed on %s.", system_image);
    longjmp_exit;
   }

  isize = sbuf.st_size;

  if (isize <= 0)
   { ServerDebug("Bad image size returned from stat %d.",isize);
    longjmp_exit;
   }
#endif

	if (target_processor eq Processor_Arm)
	  {
	    armnucbase = get_int_config("arm_nucleus_base");

	    if (armnucbase == Invalid_config)
	      armnucbase = 0xa000L;

	    /* Calculate ARM nucleus execution address: */

	    kernelstartaddress = get_int_config ("kernel_start_address");

	    if (kernelstartaddress == Invalid_config)
	      {
		/* Start address = nucbase + size word + 6 module slot words */
		/*			+ 0 word + modhdr struct */
		kernelstartaddress = ((word)(armnucbase + (8 * 4) + 60));
	      }

#if (PC || SUN3 || SUN4 || ARMBSD || SOLARIS)

#define NUM_ARM_REGS	6

	    if (mystrcmp(box_type, "vy86pid") == 0)
	      {
		word regs[NUM_ARM_REGS];
		word reqspeed, pidspeed, hwconfig;
		char ack;

#if 0
		char *	leds_type = get_config ("arm_leds");
#endif
		/* Get host.con defined comms speed. */
		reqspeed = vy86pid_get_configbaud();

		/* FIXME: The following flag bits are defined in the
		 * "include/ampp/arm.m" header only at the moment. This source
		 * should be kept in step until the header files are
		 * cleaned-up.
		 */
		hwconfig = 0; /* default state */

		if (get_config("arm_disable_cache") == NULL)
		  hwconfig |= (1 << 24) ; /* cache should be on */
		if (get_config("arm_disable_writebuffer") == NULL)
		  hwconfig |= (1 << 25) ; /* write-buffer should be on */
		if (get_config("arm_protect_nucleus") != NULL)
		  hwconfig |= (1 << 26) ; /* nucleus should be protected */
		
#if 1 /* 940524 : simple boolean LEDs control */
		if (get_config ("arm_user_leds") == NULL)
		  {
		    hwconfig |= (1 << 27); /* the Kernel uses the LEDs */
		  }
#else
		if (leds_type != NULL)
		  {
		    if (mystrcmp(leds_type,"heartbeat") == 0)
		      {
			hwconfig |= (1 << 27) ;
		      }
		    else
		      {
			if (mystrcmp(leds_type,"user") != 0)
			  {
			    ServerDebug("Error: Invalid arm_leds option \"%s\"",leds_type);
			    /* longjmp(exit_jmpbuf, 1);  */
			    longjmp_exit;
			  }
		      }
		  }
#endif

		ServerDebug("Initialising communications with DEMON monitor (%ld baud)", reqspeed);

		/* Convert host.con requested comms speed to PID values. */
		/* 1 = 9600, 2 = 19200, 3 = 38400 */
		pidspeed = (reqspeed == 38400L) ? 3L : ((reqspeed == 19200L) ? 2L : 1L);

		/* Default baud rate is 9600. The initial open must be done     */
		/* at this speed, all further comms will be at requested speed. */
		/* The second parameter controls the use of the fifo.           */
		vy86pid_set_baudrate(9600L, FALSE);
		
		/* 0x0 = Open/Init RDP request, 0x2 = cold reset cpu, reset comms */
		/* 0 = use all memory on board, speed = 1/9600, 2/19200, 3/38400 */

		if ((!xpwrbyte(0x0)) || (!xpwrbyte(0x2)) || (!xpwrint(0)) ||
		    (!xpwrbyte((byte)pidspeed)))
		  {
		    timeout("Initialising communications with DEMON monitor");
		  }

		/* Ack is returned at old speed */
		if ((ack = xprdbyte()) != 0x5f)
		  {
		    ServerDebug("Error: Unexpected ack type (0x%x) when initialising communications.", ack);
		    /*    		longjmp(exit_jmpbuf, 1); */
		    longjmp_exit;
		  }
		if ((ack = xprdbyte()) != 0)
		  {
		    ServerDebug("Error: Bad status (0x%x) when initialising communications.", ack);
		    /*    		longjmp(exit_jmpbuf, 1); */
		    longjmp_exit;
		  }

		/* Now set to host.con requested speed for further transactions */
		vy86pid_set_baudrate(reqspeed, FALSE);
		
		Debug(Boot_Flag, ("Initialising execution environment."));
		
		/* Setup register contents required for kernel startup */
		regs[0] = swap(0x02000020L);		/* r0 = comms port address */
		regs[1] = swap(armnucbase);		/* r1 = nucleus base */
		regs[2] = swap (hwconfig);		/* r2 = hardware configuration */
		regs[3] = swap (0x00000000);		/* r3 = reserved (for configuration) */
		regs[4] = swap(kernelstartaddress);	/* pc = kernel start address */
		regs[5] = swap(0xD3L);			/* cpsr = SVC32 mode FIQ/IRQ disabled */

	      
		/* 0x5 = Write CPU state RDP request, 0xff = current mode */
		/* 0x050003 = mask to write regs r0, r1, r2, r3, pc and cpsr */
		/* and the contents of the four registers. */
		if ((!xpwrbyte(0x5)) || (!xpwrbyte(0xff)) || (!xpwrint(0x05000FL)) ||
		    (!xpwrdata((byte *)regs, NUM_ARM_REGS * sizeof(word))) )
		  {
		    timeout("Initialising execution environment");
		  }

		if ((ack = xprdbyte()) != 0x5f)
		  {
		    ServerDebug("Error: Unexpected ack type (0x%x) when initialising execution environment.", ack);
		    /*    		longjmp(exit_jmpbuf, 1); */
		    longjmp_exit;
		  }
		if ((ack = xprdbyte()) != 0)
		  {
		    ServerDebug("Error: Bad status (0x%x) when initialising execution environment.", ack);
		    /*    		longjmp(exit_jmpbuf, 1); */
		    longjmp_exit;
		  }
		
		/* Do a little validation that we are talking the same language. */
		Debug(Boot_Flag, ("Checking execution environment."));
		regs[0] = regs[1] = regs[2] = regs[3] = regs[4] = regs[5] = 0;
		
		/* 0x4 = Read CPU state RDP request, 0xff = current mode */
		/* 0x05000F = mask to read regs r0, r1, r2, r3, pc, cpsr. */
		if ((!xpwrbyte(0x4)) || (!xpwrbyte(0xff)) || (!xpwrint(0x05000FL)))
		  {
		    timeout("Checking execution environment");
		  }
		
		if ((ack = xprdbyte()) != 0x5f) 
		  {
		    ServerDebug("Error: Unexpected ack type (0x%x) when checking execution environment.", ack);
		    /*    		longjmp(exit_jmpbuf, 1); */
		    longjmp_exit;
		  }
		if (!xprddata((byte *)regs, NUM_ARM_REGS * sizeof(word)) )
		  {
		    timeout("Reading execution environment");
		  }
		
		Debug(Boot_Flag, ("Execution Env: r0 (comms port addr) : %lx", swap (regs[0])));
		Debug(Boot_Flag, ("               r1 (nucleus base)    : %lx", swap (regs[1])));
		Debug(Boot_Flag, ("               r2 (hardware config) : %lx", swap (regs[2])));
		Debug(Boot_Flag, ("               r3 (config reserved) : %lx", swap (regs[3])));
		Debug(Boot_Flag, ("               pc                   : %lx", swap (regs[4])));
		Debug(Boot_Flag, ("               psr                  : %lx", swap (regs[5])));

		if ((ack = xprdbyte()) != 0)
		  {
		    ServerDebug("Error: Bad status (0x%x) when checking execution environment.", ack);
		    /*    		longjmp(exit_jmpbuf, 1); */
		    longjmp_exit;
		  }

		Debug(Boot_Flag, ("Setting the LED's."));
		/* Light all the Leds! */
		/* Send memory write request to transfer the nucleus into the PID. */
		/* 0x3 = Write PID memory RDP request, ledaddr, 1*/
		if ((!xpwrbyte(0x3)) || (!xpwrint(0x200006c))
		    || (!xpwrint(1L)) || (!xpwrbyte(0)))
		  {
		    timeout("Setting the LED's");
		  }

		if ((ack = xprdbyte()) != 0x5f)
		  {
		    ServerDebug("Error: Unexpected ack type (0x%x) when setting LED's.", ack);
		    /*    		longjmp(exit_jmpbuf, 1); */
		    longjmp_exit;
		  }
		
		if ((ack = xprdbyte()) != 0)
		  {
		    ServerDebug("Error: Bad status (0x%x) when setting LED's.", ack);
		    /*    		longjmp(exit_jmpbuf, 1); */
		    longjmp_exit;
		  }

		ServerDebug("Sending download request (@ 0x%lx, size 0x%lx).", armnucbase, isize);

		/* Send memory write request to transfer the nucleus into the PID. */
		/* 0x3 = Write PID memory RDP request, @ armnucbase, nucsize */
		if ((!xpwrbyte(0x3)) || (!xpwrint(armnucbase))
		    || (!xpwrint(isize)))
		  {
		    timeout("Sending DEMON monitor download request");
		  }
		
		/* PID demon now expects 'isize' bytes to be sent, */
		/* followed by ack byte and status. */

	      }
	    else

#endif /* (SUN3 || SUN4 || ...) */
	      {
		char buf[80];
		int i;

		/* Send download command to monitor running on Archimedes. */

		ServerDebug("Sending ARM monitor download request (t %lX %lX).", armnucbase, isize);

		/* t = transfer(startaddr,size) : binary download */
		sprintf(buf,"t %lX %lX\n", armnucbase, isize) ;

		for (i=0; i < strlen(buf); i++)
		  {
		    if (!xpwrdata(&buf[i], 1))
		      timeout("sending 't' download command to monitor");

#if (!MSWINDOWS)
		    while( xprdrdy() ) /* get rid of any echoed chars */
		      xprdbyte();
#endif
		  }
	      }
	  }
        Debug(Boot_Flag, ("Downloading system image %s, size 0x%lx", system_image, isize) );
/*	printf ("Downloading system image %s, size 0x%lx\n", system_image, isize); */

        for (l = 0L; l < isize; l += MaxChunk) 
	 {
	   cur = (int) (((isize - l) > MaxChunk) ? MaxChunk : isize - l);

			/* read chunk */
           if (fread(cur_data, 1, (size_t)cur, infd) eq 0)
	    {
	      ServerDebug("Failed to read (%x) bytes in system image (read 0x%lx out of 0x%lx).", cur, l, isize);

              fclose( infd );
              infd = (FILE *) NULL;

/*              longjmp(exit_jmpbuf, 1); */
	      longjmp_exit;
            }

	       /* check for user interaction */
           poll_the_devices();
           if (Special_Exit)
	     {
/*	       longjmp(exit_jmpbuf, 1); */
	       longjmp_exit;
	     }

	      /* send image chunk */
           if (!xpwrdata(cur_data, (word) cur))
	    {
	      ServerDebug("Failed to send %d bytes 0x%lx/0x%lx.", cur, l, isize);

              fclose( infd );
              infd = (FILE *) NULL;

              timeout("nucleus");
            }
	   if (target_processor eq Processor_Arm)
	     output(".");
         }
	if (target_processor eq Processor_Arm)
	   {
	     output("\n\r");
	   }

	if (target_processor eq Processor_Arm) {

#if (PC || SUN3 || SUN4 || ARMBSD)

	 if (mystrcmp(box_type, "vy86pid") == 0) {
	   char ack;

	   /* Check PID write memory RDP request acknowledge. */
	   if ((ack = xprdbyte()) != 0x5f) {
		if (ack != 0) {
	    	   	ServerDebug("Error: Unexpected ack type (0x%x) after download request.", ack);
/*    			longjmp(exit_jmpbuf, 1); */
			longjmp_exit;

		} else {
	    	   	ServerDebug("Warning: DEMON monitor request acknowledge was lost.");
		}
	   } else {
		   if ((ack = xprdbyte()) != 0) {
    		   	ServerDebug("Error: Bad status (0x%x) after downloading nucleus, only 0x%x bytes received.", ack, xprdbyte());
/*    			longjmp(exit_jmpbuf, 1); */
			longjmp_exit;

		   }
	   }

#if 1
	    Debug(Boot_Flag, ("Setting the LED's (again)."));
	   /* Light some of the Leds */
	   /* Send memory write request to transfer the nucleus into the PID. */
	   /* 0x3 = Write PID memory RDP request, ledaddr, 1*/
	   if ((!xpwrbyte(0x3)) || (!xpwrint(0x200006c))
		|| (!xpwrint(1L)) || (!xpwrbyte(0x30)))
		timeout("Setting LED's (1100) on");

	   if ((ack = xprdbyte()) != 0x5f) {
    	   	ServerDebug("Error: Unexpected ack type (0x%x) when setting LED's (1100).", ack);
/*    		longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	   }

	   if ((ack = xprdbyte()) != 0) {
    	   	ServerDebug("Error: Bad status (0x%x) when setting LED's.", ack);
/*    		longjmp(exit_jmpbuf, 1); */
		longjmp_exit;
	   }
#endif
	   ServerDebug("Starting Helios-ARM system (@ %lX).", kernelstartaddress);

	   /* Send execute request to ROM demon running on PID */
	   /* 0x3 = execute RDP request, 0 = no ack required */
	   if ((!xpwrbyte(0x10)) || (!xpwrbyte(0))) {
		timeout("sending execute request to DEMON monitor");
	   }

	   vy86pid_setup_handshake();
	   
	 } else

#endif
	 {
	   /* Send go command to monitor running on Archimedes. */
	   char buf[80];
	   char *b;

	   ServerDebug("Starting Helios-ARM system (g %lX).", kernelstartaddress);
    	   sprintf(buf,"g %lX\n", kernelstartaddress);
    	   for (b = buf ; *b != '\0'; ) {
		if (!xpwrbyte(*b++))
			timeout("sending 'go' command to monitor") ;
	   }
	 }
	}
      	      /* sent entire nucleus successfully */
        Debug(Boot_Flag, ("System image downloaded"));
        fclose( infd );
        infd = (FILE *) NULL;
        iofree(cur_data);
      }
     else
      { Debug(Boot_Flag, ("Sending System Image (%ld bytes) ...",isize) );
        if (Wander(Image_List, (WordNodeFnPtr)send_bit))
         timeout("system image");
      }
}
/*}}}*/
/*{{{  send configuration vector */
/**
*** The configuration vector
**/

#if !(PC)
PRIVATE Config config;
#else
PRIVATE Config far config;
#endif

/*{{{  build_config() */
PRIVATE word build_config()
{ char *procname = slashDefaultRootName, *myname = slashDefaultServerName;
  char *tempname;
  word memory_size = processor_memory;
  word bootlink    = get_int_config("bootlink");
  word numlinks	   = 4L;
  word flags = 0;
  word i;

  tempname = get_config("root_processor");
  if (tempname ne (char *) NULL) procname = tempname;
  tempname = get_config("io_processor");
  if (tempname ne (char *) NULL) myname = tempname;
  if (memory_size eq Invalid_config) memory_size = 0L;
  if (bootlink eq Invalid_config) bootlink = (word) Default_BootLink;
  if (target_processor eq Processor_C40)
   { numlinks = 6;
     if (!mystrcmp(box_type, "DSP1") ||
	 !mystrcmp(box_type, "VC40") ||
	 !mystrcmp(box_type, "SPIRIT40") )
      numlinks = 7;
   }

  config.PortTabSize = swap(1024L);
  config.Incarnation = swap(1L);
  config.loadbase    = swap(0x80001000L);	/* will be ignored by kernel */
  if (bootstrap_mode & B_Send_Image)
   config.ImageSize   = swap(isize);		/* will be ignored by kernel */
  else
   config.ImageSize  = swap(0L);
  config.Date        = swap(get_unix_time());
  config.FirstProg   = get_int_config("nucleus_firstprog");
  if (config.FirstProg == Invalid_config)
    config.FirstProg   = swap(0L);		/* will default to procman */
  else
    config.FirstProg	= swap (config.FirstProg);
  config.Memory      = swap(memory_size);

  config.Flags       = swap(Config_Flags_rootnode);
  if (get_config("c40_disable_cache") != NULL)
    config.Flags |= swap(Config_Flags_CacheOff);
#if (PC || SUN3 || SUN4 || ARMBSD)

  if ((target_processor eq Processor_Arm) && (mystrcmp(box_type, "vy86pid") == 0))
    config.Spare       = swap(vy86pid_get_configbaud());
  else

#endif
    config.Spare       = swap(0L);

  config.NLinks      = swap(numlinks);
	/* By default the links are dead (not connected). The bootlink	*/
	/* is intelligent and running and has the right flags set, 	*/
	/* parent +  debug + ioserver					*/
  for (i = 0L; i < numlinks; i++)
   config.LinkConf[(int) i] = swap(0x00060000L | (i << 24));

  config.LinkConf[(int) bootlink] &= swap(0xFF00FF00L); 
  if (!mystrcmp(box_type, "DSP1"))
   config.LinkConf[(int) bootlink] |= swap(0x00100370L);
  elif (!mystrcmp(box_type, "HYDRA"))
   config.LinkConf[(int) bootlink] |= swap(0x00110370L);
  elif( !mystrcmp(box_type, "VC40") || !mystrcmp(box_type, "SPIRIT40") )
  {
  	/* setup configuration for shared memory links */

	/*
	 * 00e00000 => SML link, 
	 * 00000300 => Special link,
	 * 00000070 => parent + ioserver + debug
	 */
  	word cfg = 0x00e00370L;
	word tmp;


	/*
 	 * Set SML type
	 */	
#if SUN3
  	if( !mystrcmp(box_type, "VC40") )
	{
		if (VC40_Type == -1)	VC40_Type = vc40_settype ();

		if (VC40_Type == 1)
		{
			/* Ariel's HYDRA I board */
		 	cfg |= 0x00000000L;
		}
		else if (VC40_Type == 2)
		{
			/* Ariel's HYDRA II board */
			cfg |= 0x00100000L;
		}
		else
		{
			ServerDebug ("Error - unknown board type %d", VC40_Type);
		}
	}
#endif
	if( !mystrcmp(box_type, "SPIRIT40") ) 	cfg |= 0x00080000L;

	/*
	 * Set SML strobe
	 */
	if( get_config("c40_sml_g1") ) 		cfg |= 0x00040000L;

	/*
	 * Set SML size
	 */
	tmp = get_int_config("c40_sml_size");

#if SUN3
	if (VC40_Type == 2)
	{
		/* Ariel's HYDRA II board */
		if (tmp == 64)
		{
			ServerDebug ("Bad shared memory size %d, setting to 8", tmp);

			tmp == 8;
		}
	}
#endif
	switch( tmp )
	{
	default:
		ServerDebug("Invalid value for c40_sml_size, set to 8");
	case Invalid_config:
	case 8:		cfg |= 0x00000000;	break;
	case 16:	cfg |= 0x00010000;	break;
	case 32:	cfg |= 0x00020000;	break;
	case 64:	cfg |= 0x00030000;	break;
	}

	config.LinkConf[(int) bootlink] |= swap(cfg);
  }
  else
  {
	config.LinkConf[(int) bootlink] |= swap(0x00030270L);
  }

  /* if half duplex protocol is not disabled, set it on the boot link */
  if (C40HalfDuplex)
	    config.LinkConf[(int) bootlink] |= swap(Link_Flags_HalfDuplex);

  config.MyName      = swap( ((word) &config.LinkConf[numlinks]) -
                             ((word) &config.MyName));
  config.ParentName  = swap( (((word) &config.LinkConf[numlinks]) +
			     strlen(procname) + 1) -
                             ((word) &config.ParentName) );

  strcpy( ((char *)&config.LinkConf[numlinks]), procname);
  strcpy( ((char *)&config.LinkConf[numlinks]) + strlen(procname) + 1, myname );

  return((word) (sizeof(Config) + ((numlinks - 1) * sizeof(word)) +
		strlen(procname) + 1 + strlen(myname) + 1 - CONFIGSPACE));
}
/*}}}*/

PRIVATE void send_conf()
{ word size = build_config();

  if (debugflags & Boot_Flag)
     ServerDebug("Sending configuration ...");

#if (PC || SUN3 || SUN4 || ARMBSD)

  /* Initial comms with VLSI PID board must be at a know rate (9600). */
  /* The config vector defines the agreed rate for all future communications */
  if ((target_processor eq Processor_Arm) && (mystrcmp(box_type, "vy86pid") == 0))
   vy86pid_set_baudrate(9600L, FALSE);

#endif

  if (!xpwrint(size))
   timeout("system configuration size");
  if (target_processor eq Processor_C40)
   size = (size + 3L) & ~3L;

  if (!xpwrdata((byte *) &config,size))
    timeout("system configuration");

#if (PC || SUN3 || SUN4 || ARMBSD)

  /* Now communicate at agreed rate defined in host.con and sent via config. */
  /* The second argument controls use of the fifo.			     */
  if ((target_processor eq Processor_Arm) && (mystrcmp(box_type, "vy86pid") == 0))
  {
    vy86pid_set_baudrate(vy86pid_get_configbaud(), TRUE);
  }

#endif
}
/*}}}*/
/*{{{  wait for ACK */
/**
*** The main job of this routine is to wait for the kernel to start up
*** sufficiently for it to need to know a port on my side of the link. This
*** involves the processor sending a byte F0, followed by some more junk,
*** and I reply with junk plus a pseudo port for the IOproc device.
***
*** If we are trying to attach to a running system we simply send a sync
*** message to it. This lets it know we are now attached, we do not then
*** need to wait for a sync back.
**/

PRIVATE void server_helios()
{ 
  long i;

  if(bootstrap_mode & B_Send_Sync)
  {
    Debug(Boot_Flag, ("Attaching to remote system") );

/*{{{  ARMBSD bodge */
#if ARMBSD
    if (get_config("abc_protobodge3") ne NULL) {
	/* *TEMPORARY FIX* */
	/* send ResyncRequest that we know will get mangled */
        /* by abc's exec. The exec erroneously */
	/* duplicates the first char it receives down the link. */
	/* Thus creating the impression that the port to talk to the IO */
        /* Server on is 100, rather than 1! */
	send_resync();
	/* @@@ This should probably become standard */
    }
#endif
/*}}}*/

    if (!send_info_request())
      timeout("attach request");
    else
      output("Attached...\r\n");

    return;
  }

  if(!(bootstrap_mode & B_Wait_Sync))
    return;

  if (target_processor == Processor_C40) 
   {	/* Due to problems with the half duplex nature of the links */
	/* The C40 never sends the IOServer Proto_Info's. Instead   */
        /* the bootstrap code acknowledges earlier on.              */

#if SUN3
	if (VC40_Type == -1)	VC40_Type = vc40_settype ();

	if (VC40_Type == 2)
	{
		/* 
		 * Need to wait for helios kernel to initialise shared mem link
		 * before enabling interupts.
		 */
		vc40_enint ();
	}
#endif
	output("TMS320C40 Booted...\r\n");
  	return; /* the processor is now ready */
   }

  Debug(Boot_Flag, ("Waiting for sync byte from kernel") );

  for(i=0L;i < 1000000L;i++)
   {
#if multi_tasking
     if (!Multiwait()) continue;
#endif

     poll_the_devices();
     if (Special_Exit)
       {
	 /* longjmp(exit_jmpbuf, 1); */
	 longjmp_exit;
       }

     if (Special_Status)                /* or for status request */
      { ServerDebug("Server alive and well.");
        Special_Status = false;
      }
     if (Special_Reboot || DebugMode)            /* or for reboot condition */
        return;
     if( xprdrdy() )
      { word b=0L;

	if (target_processor eq Processor_C40)
	 b = xprdint();
	else
         b = xprdbyte();

        switch( (int) b )
         { case Proto_Info:
                      handle_info_request();
                      output("Booted...\r\n");
                      return; /* the processor is now ready */
           default:
                      ServerDebug("Unexpected byte 0x%lx.",b);
                      break;
         }
      }
   }

   printf("Failed to receive synchronisation byte.\n Server Aborting\n");
/*   longjmp(exit_jmpbuf, 1); */
	longjmp_exit;
}

PRIVATE void handle_info_request()
{ word a, iocport;
  word *x = (word *) &(misc_buffer1[0]);

  if (target_processor == Processor_C40)
	ServerDebug("Warning: C40 requested info exchange");

  Debug(Boot_Flag, ("Received an F0 sync.") );

  if (target_processor == Processor_C40)
    xprddata(&(misc_buffer1[4]), 8L);	/* C40 sends word size prototype byte */
  else
    xprddata(&(misc_buffer1[1]), 11L);

  a = swap(x[1]);
  iocport = swap(x[2]);

  if ( (a & 0x00FF0000L) ne 0L)
   { Debug(Boot_Flag, ("Sending back info"));
     x[0] = swap(0xf0f0f0f0L);
     x[1] = swap(0x00000100L);
     x[2] = swap(1L);
     xpwrdata(&(misc_buffer1[0]), 12L);
   }
}

PRIVATE int send_info_request()
{
  word *x = (word *) &(misc_buffer1[0]);

  x[0] = swap(0xf0f0f0f0L);	/* Proto_Sync */
  x[1] = swap(0x00000100L);	/* 1st incarnation + no reply required */
  x[2] = swap(1L);		/* IO Servers port */

  return (int)(xpwrdata(&(misc_buffer1[0]), 12L));
}

PRIVATE int send_resync()
{
  word *x = (word *) &(misc_buffer1[0]);

  /* Send at least same size as a SendInfo message */

  /* Sending resyncs of at least sizeof(MshHdr) + MaxContolSize + MaxDataSize */
  /* (16 + 256 + 64k) would probably be acceptable over 10Mb links */

  x[0] = swap(0x7f7f7f7fL);
  x[1] = swap(0x7f7f7f7fL);
  x[2] = swap(0x7f7f7f7fL);

  return (int)(xpwrdata(&(misc_buffer1[0]), 12L));
}
/*}}}*/
/*{{{  message passing */

/*------------------------------------------------------------------------
-- The following routines provide an interface to message passing       --
-- with the processor.                                                 --
------------------------------------------------------------------------*/

/**
*** It is desirable that all message interaction between processor and host
*** go via a small set of routines, and for historical reasons these are the
*** ones. Messages are always received by the root coroutine inside the main
*** loop, by calls to Request_Stat() and Request_Get(). The messages are
*** buffered in mcbstruct, which is set up by init_main_message() and the
*** space obtained is freed again by free_main_message(). It is important
*** that coroutines do not do their own GetMsg's, because the local GetMsg
*** does not know anything about ports etc. and I do not want to complicate it.
*** 
*** Request_Stat() is the routine that gets hold of any messages the processor
*** is trying to send. Some of these messages may be very low-level debugging
*** ones, identified by a FnRc of 0x22222222L, and these are treated separately.
*** I have to worry a little bit to make sure that all debugging messages
*** appear on separate lines, assuming they are not too interleaved. If another
*** message is received the routine returns true, and main_loop calls
*** Request_Get() to get hold of it. In fact the value returned is always the
*** same, the address of the main message, but this call allows me to clear
*** the flags field which is desirable according to NHG.
***
*** When a reply is to be sent to the processor this goes via Request_Return(),
*** which plays about a bit with the message to make sure that each coroutine
*** does not have to worry about swapping ports around etc. There is also a
*** user debugging facility, and a test for the special case of sending a
*** message to the invalid port zero which is strongly indicative of a bug
*** in the server. If for some reason Request_Return() is unable to send its
*** message this is indicative of the processor having crashed, so I output
*** a suitable message.
***
*** GetMsg() can return three values. It can return true if the machine has
*** received a complete Helios message, false if it has not received anything
*** or 0xF0 if it has received the first byte of a raw message. GetMsg() itself
*** is responsible for handling all peek and poke requests coming from the
*** processor.
**/

PRIVATE MCB  mcbstruct;
PRIVATE word control[256];
PRIVATE byte *real_buff = (byte *) NULL;

void init_main_message()
{ uint amount = (uint) (maxdata + 1000L);
  byte *temp;

#if PC
  if (maxdata > 64000L) maxdata = 64000L;	/* segment problems */
#endif

  temp = (byte *) malloc(amount);
  if (temp eq NULL)
    { ServerDebug("Insufficient memory on the server machine for message buffer.");
/*      longjmp(exit_jmpbuf, 1); */
	longjmp_exit;
    }
  real_buff          = temp;
  mcbstruct.Data     = (byte *) (( ((word)temp) + 511L) & 0xFFFFFE00L);
  mcbstruct.Control  = &(control[0]);
  mcb = &mcbstruct;
}

#if ANSI_prototypes
void free_main_message(void)
#else
void free_main_message()
#endif
{ if (real_buff ne (byte *) NULL) iofree(real_buff);
}

int newdebug = true;

word Request_Stat()
{ word j = 0L;

  mcbstruct.Timeout = 0x0FFL;
#if AMIGA
  j = GetHeliosMsg(&mcbstructs);
#else  
  j = GetMsg(&mcbstruct);
#endif

  if (j eq Proto_Info)
   { handle_info_request(); return(0); }
  elif (j) 
     { if (mcbstruct.MsgHdr.FnRc eq 0x22222222L)
         { int x;
           for (x = 0; x < (int)(mcbstruct.MsgHdr.DataSize); x++)
            { if (newdebug)
               { ServerDebug("+++ %q");
                 newdebug = false;
               }

              if (mcbstruct.Data[x] eq '\r')	/* remove spurious CR's */
		continue;

              if (mcbstruct.Data[x] eq '\n')
               { ServerDebug("");		/* add in spurious CR's */
                 newdebug = true;
                 continue;
               }
              ServerDebug("%c%q", mcbstruct.Data[x]);
            }
           mcbstruct.MsgHdr.Flags = 0;
           mcbstruct.MsgHdr.Dest = 0;
           mcbstruct.MsgHdr.Reply = 0;
           return(false);
         }
       else
        { mcbstruct.MsgHdr.Flags = 0;        /* on NickG's orders */
          return(true);
        }
     }

   return(false);
}

void Request_Return(code, contlen, datalen)
word code, contlen, datalen;
{ word temp;

  mcb->MsgHdr.FnRc      = code;
  mcb->MsgHdr.DataSize  = (USHORT) datalen;
  mcb->MsgHdr.ContSize  = (byte) contlen;
  temp                  = mcb->MsgHdr.Dest;
  mcb->MsgHdr.Dest      = mcb->MsgHdr.Reply;
  mcb->MsgHdr.Reply     = temp;
  mcb->Timeout          = 0x07FFFL;
  
  if (debugflags)
  {
    if (debugflags & Message_Flag)
      ServerDebug("Reply (from 0x%lx) : fn 0x%lx, port 0x%lx, csize %ld, dsize %ld",
             mcb->MsgHdr.Reply, code, mcb->MsgHdr.Dest, contlen, datalen);
  
    if (debugflags & Error_Flag)
       if ((code & 0x80000000L) ne 0)
          ServerDebug("Error : fn 0x%lx", code);
  }

  if (mcb->MsgHdr.Dest eq 0L)
      return;

#if AMIGA
  if (!PutHeliosMsg(mcb))
#else  
  if (!PutMsg(mcb))
#endif
    { output("***\r\n");
      output("*** Serious : the root processor failed to receive a message.\r\n");
#if (ST || PC)
      output("***           To reboot the system, use the Control, Shift and\r\n");
      output("***           F10 keys at the same time.\r\n");
#endif
      output("***\r\n");
    }

  mcb->MsgHdr.Flags = 0;     /* Clean out these entries */
  mcb->MsgHdr.Dest  = 0L;
  mcb->MsgHdr.Reply = 0L;
}
/*}}}*/
